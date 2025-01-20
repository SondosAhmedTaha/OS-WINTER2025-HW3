#include "segel.h"
#include "request.h"
#include "list.h"
#include <stdbool.h>


#define BLOCK 1
#define DROP_TAIL 2
#define DROP_HEAD 3
#define BLOCK_FLUSH 4
#define RANDOM 5


int VIP_inside;
List wait_requests;
List curr_handled;
List vip_requests;
pthread_cond_t  queue_full, queue_empty_VIP, regular_allowed;
pthread_mutex_t lock;


void* worker_function (void* thread_stats){
    
    threads_stats th_stat = (threads_stats) thread_stats;
    while(true){
        pthread_mutex_lock(&lock);

        while(VIP_inside > 0 || get_list_size(vip_requests) >0 ||
				get_list_size(wait_requests)==0){
            pthread_cond_wait(&regular_allowed, &lock);
        }

        Request curr_request = pop(wait_requests);
        push_back(curr_handled, curr_request);

        //if we get here atleast 1 request waiting.
        pthread_mutex_unlock(&lock);
        //the previous code is exactly like the tutorial!
        int is_skip = 0;
        gettimeofday(&(curr_request->pick_up_t),NULL);
        timersub(&(curr_request->pick_up_t), &(curr_request->arrival_t), &(curr_request->dispatch_int));
        requestHandle(curr_request->conn_fd, curr_request->arrival_t, curr_request->dispatch_int, th_stat, &is_skip);
        Close(curr_request->conn_fd);

        //if is_skip =1 , then we have a skip request
        if(is_skip == 1){
            //REVIEW THIS AFTERWARDS
            pthread_mutex_lock(&lock);
            int thread_to_pop = find_by_sfd(curr_handled, curr_request->conn_fd);
            pop_by_index(curr_handled, thread_to_pop);
            int index = get_list_size(wait_requests);
            curr_request = pop_by_index(wait_requests, index);
            push_back(curr_handled, curr_request);
            pthread_mutex_unlock(&lock);
            is_skip = 0;
            gettimeofday(&(curr_request->pick_up_t),NULL);
            timersub(&(curr_request->pick_up_t), &(curr_request->arrival_t), &(curr_request->dispatch_int));
            requestHandle(curr_request->conn_fd, curr_request->arrival_t, curr_request->dispatch_int, th_stat, &is_skip);
            Close(curr_request->conn_fd);
        }

        pthread_mutex_lock(&lock);
        int thread_to_pop = find_by_sfd(curr_handled, curr_request->conn_fd);
        pop_by_index(curr_handled, thread_to_pop);
        pthread_cond_signal(&queue_full);
        pthread_mutex_unlock(&lock);
    }
}

void * VIP_worker_function (void* thread_stats){
    
    threads_stats th_stat= (threads_stats) thread_stats;
    while(true){
        pthread_mutex_lock(&lock);
        while(get_list_size(vip_requests)==0){
            pthread_cond_wait(&queue_empty_VIP, &lock);
        }
        Request curr_request = pop(vip_requests);
        VIP_inside++;
        //if we get here atleast 1 request waiting.
        pthread_mutex_unlock(&lock);
        //the previous code is exactly like the tutorial!
        gettimeofday(&(curr_request->pick_up_t),NULL);
        timersub(&(curr_request->pick_up_t), &(curr_request->arrival_t), &(curr_request->dispatch_int));
        int num=0;
        requestHandle(curr_request->conn_fd, curr_request->arrival_t, curr_request->dispatch_int, th_stat, &num);

        Close(curr_request->conn_fd);
        pthread_mutex_lock(&lock);
        VIP_inside--;
        pthread_cond_signal(&queue_full);
        // it will release one of the waiting threads.
        pthread_mutex_unlock(&lock);
    }
}

//fill the input in the params
void getargs(int *port, int* threads_num, int* queue_size,char** algo,int* max_size, int argc, char *argv[]) {
	
    if (argc != 5 ) { exit(1); }
    *port = atoi(argv[1]);
    *threads_num= atoi(argv[2]);
    *queue_size= atoi(argv[3]);
    if ((*port)<1023 || (*port)>65535 || (*threads_num)==0 || (*queue_size)==0){
		exit(1);
	}
    *algo = (char*) malloc(sizeof(char) * strlen(argv[4]) + 1);
    strcpy(*algo, argv[4]);
}

int translate_algo(char* algo){
	
    if(strcmp(algo, "block") == 0) { return BLOCK; }
    else if(strcmp(algo, "dt") == 0) { return DROP_TAIL; }
    else if(strcmp(algo, "dh") == 0) { return DROP_HEAD; }
    else if(strcmp(algo, "bf") == 0) { return BLOCK_FLUSH; }
    else if(strcmp(algo, "random") == 0) { return RANDOM; }
    return -1;
}

//Algorithms------------------------------------------------------------

void block(int max_size, Request request){
	
    while (get_list_size(wait_requests) + get_list_size(curr_handled)
			+ get_list_size(vip_requests) >= max_size){
        pthread_cond_wait(&queue_full, &lock);
    }
    push_back(wait_requests, request);
    pthread_cond_signal(&regular_allowed);
}

void drop_tail(int conn_fd){
    Close(conn_fd);
}

void drop_head(Request curr_request){
    if(get_list_size(wait_requests) != 0){
        Request head = pop(wait_requests);
        Close(head->conn_fd);
    }
        push_back(wait_requests, curr_request);
        pthread_cond_signal(&regular_allowed);
        return;
}

void block_flush(Request request){
	
    while (get_list_size(wait_requests) + get_list_size(curr_handled)
			+ get_list_size(vip_requests) > 0){
        pthread_cond_wait(&queue_full, &lock);
    }
    Close(request->conn_fd);
    free(request);
}

void drop_random(List list, int connfd, Request request, int max_size){
	
    int num_to_delete = (list->curr_size+1)/2;
    for (int i = 0; i < num_to_delete; i++){
        if(get_list_size(list) <= 0) { break; }
        int index = rand() % (list->curr_size);
        Request to_delete = pop_by_index(list, index);
        Close(to_delete->conn_fd);
        free(to_delete);
    }
    while (get_list_size(wait_requests)+ get_list_size(curr_handled)+
			get_list_size(vip_requests) >= max_size){
        pthread_cond_wait(&queue_full, &lock);
    }
    push_back(wait_requests, request);
    pthread_cond_signal(&regular_allowed);
}

//end of algorithms-----------------------------------------------------

int main(int argc, char *argv[]) {

    int port, clientlen, threads_num, queue_size, max_size, listenfd, connfd;
    struct sockaddr_in clientaddr;
    char* algo;
    
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&queue_empty_VIP,NULL);
    pthread_cond_init(&queue_full, NULL);
    pthread_cond_init(&regular_allowed, NULL);
    
    max_size = 0;
    VIP_inside = 0;

	//setting the input
    getargs(&port, &threads_num, &queue_size , &algo, &max_size, argc, argv);
    int algorithm = translate_algo(algo);
    if (algorithm == -1) { exit(1); }
    
    //create 3 lists
    int size_list = queue_size;
    wait_requests = create_list(size_list);
    curr_handled = create_list(size_list);
    vip_requests = create_list(size_list);
    
    //creating arrays for the threads and the stats
    pthread_t* worker_threads = (pthread_t*) malloc(sizeof(pthread_t)*(threads_num+1));
    threads_stats* thread_stats_array = malloc(sizeof(threads_stats)*(threads_num+1));

	//create the threads
    for (int i = 0; i < threads_num; i++) {
		
        threads_stats thr_stat = create_threads_stats(i);
        thread_stats_array[i] = thr_stat;
        pthread_create(&worker_threads[i], NULL, worker_function, (void*)(thread_stats_array[i]));
    }
    
    threads_stats thr_stat = create_threads_stats(threads_num);
    thread_stats_array[threads_num] = thr_stat;
    pthread_create(&worker_threads[threads_num], NULL, VIP_worker_function, (void*)(thread_stats_array[threads_num]));
    
    //the main event
    listenfd = Open_listenfd(port);
    while (1) {
		
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        Request current_request = create_request(connfd);
        pthread_mutex_lock(&lock);
        
        if(get_list_size(wait_requests)+ get_list_size(curr_handled)
			+ get_list_size(vip_requests) < queue_size){

            if(getRequestMetaData(connfd)){
                push_back(vip_requests, current_request);
                pthread_cond_signal(&queue_empty_VIP);
            }
            else{
                push_back(wait_requests, current_request);
                pthread_cond_signal(&regular_allowed);
            }
            pthread_mutex_unlock(&lock);
            continue;
        }

        //IF THE SERVER IS FULL OF VIP REQUESTS, BLOCKING SHOULD BE APPLIED
        else {
			
			if(getRequestMetaData(connfd)){
                push_back(vip_requests, current_request);
                pthread_cond_signal(&queue_empty_VIP);
            }
            
            if(get_list_size(vip_requests) >= queue_size){
                while(get_list_size(vip_requests) > queue_size){
                    pthread_cond_wait(&queue_full, &lock);
                }
            }
            
            else if(algorithm == BLOCK) {
				block(queue_size, current_request);
			}
            else if(algorithm == DROP_TAIL) {
                drop_tail(connfd);
                free(current_request);
            }
            else if(algorithm == DROP_HEAD) {
                drop_head(current_request);
            }
            else if(algorithm == BLOCK_FLUSH) {
                block_flush(current_request);
            }
            else if(algorithm == RANDOM) {
                drop_random(wait_requests, connfd, current_request, queue_size);
            }
            pthread_mutex_unlock(&lock);
        }
    }
}



    


 
