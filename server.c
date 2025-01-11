#include "segel.h"
#include "request.h"
#include "list.h"
#include <stdbool.h>
#define BLOCK 1
#define DROP_TAIL 2
#define DROP_HEAD 3
#define BLOCK_FLUSH 4
#define RANDOM 5
List waiting_requests;
List currently_handled;
//List vip_requests;
pthread_cond_t queue_empty, queue_full;
pthread_mutex_t thread_lock;
// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// HW3: Parse the new arguments too
//The C Program nust be invoked by
//./server [portnum][threads][queue_size][schedlag]

void * workerFunction (void* thread_stats){
    //struct timeval arrival_time; //todo when to initialzie or delete
    threads_stats th_stat= (threads_stats) thread_stats;//todo debug
    while(true){
        pthread_mutex_lock(&thread_lock);
        while(getListSize(waiting_requests)==0){
            pthread_cond_wait(&queue_empty, &thread_lock);
            //waiting for "Event" to get a waiting request...
        }
        //if we get here atleast 1 request waiting.
        Request curr_request= pop(waiting_requests);
        pushBack(currently_handled,  curr_request);
        pthread_mutex_unlock(&thread_lock);
        //the previous code is exactly like the tutorial!
        gettimeofday(&(curr_request->pickUp_time),NULL);
        timersub(&(curr_request->pickUp_time), &(curr_request->arrival_time), &(curr_request->dispatch_interval));
        requestHandle(curr_request->connfd, curr_request->arrival_time, curr_request->dispatch_interval, th_stat);

        //void requestHandle(int fd, struct timeval arrival, struct timeval dispatch, threads_stats t_stats)
        Close(curr_request->connfd);
        pthread_mutex_lock(&thread_lock);
        int thread_to_pop= findThreadBySfd(currently_handled,curr_request->connfd);
        popByIndex(currently_handled, thread_to_pop);
        pthread_cond_signal(&queue_full);//if the queue was full
        // it will release one of the waiting threads.
        //todo check this line
        pthread_mutex_unlock(&thread_lock);
    }
    //return NULL;
}


void getargs(int *port, int* threads, int* queue_size,char** shedlag,int* max_size, int argc, char *argv[])
{
    if (argc != 5 ) {
        //fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *threads= atoi(argv[2]);
    *queue_size= atoi(argv[3]);
    *shedlag=(char*) malloc(sizeof (char) *strlen(argv[4])+1);
    //if malloc failed
    strcpy(*shedlag, argv[4]);

}
int getSchedAlgo(char* shedlag){
    if(strcmp(shedlag, "block")==0){
        return BLOCK;
    }
    else if(strcmp(shedlag, "dt")==0){
        return DROP_TAIL;
    }
    else if(strcmp(shedlag, "dh")==0){
        return DROP_HEAD;
    }
    else if(strcmp(shedlag, "bf")==0){
        return BLOCK_FLUSH;
    }

    else if(strcmp(shedlag, "random")==0){
        return RANDOM;
    }
    return -1;
}
void Block(int queue_size_limit, Request request){
    while (getListSize(waiting_requests)+ getListSize(currently_handled)>=queue_size_limit){
        pthread_cond_wait(&queue_full,&thread_lock);
    }
    pushBack(waiting_requests, request);
    pthread_cond_signal(&queue_empty);

}
void DropTail(int connfd){
    Close(connfd);
}
void DropHead(int connfd, Request current_request){
    if(getListSize(waiting_requests)!=0){
        Request poped_req= pop(waiting_requests);
        Close(poped_req->connfd);
        //pushBack(waiting_requests,poped_req);
        pushBack(waiting_requests, current_request);
        pthread_cond_signal(&queue_empty);//waiting to be a "place" to handle
    }
    else{
        Close(connfd);
        return;
    }
}
void BlockFlush(Request request){
    while (getListSize(waiting_requests)+ getListSize(currently_handled)>0){
        pthread_cond_wait(&queue_full,&thread_lock);
    }
    Close(request->connfd); // changed day7
    free(request);
    //pushBack(waiting_requests, request);
    //pthread_cond_signal(&queue_empty);//waiting to be a "place" to handle
}


void drop_random(List list, int connfd, Request req, int queue_size_limit){ // todo changed
    int number_tobe_removed= (list->curr_size+1)/2;
    for (int i=0; i<number_tobe_removed; i++){
        if(getListSize(list)<=0){
            break;
        }
        int index= rand() % (list->curr_size);
        Request to_dump=popByIndex(list, index);
        Close(to_dump->connfd);
        free(to_dump);
    }
    while (getListSize(waiting_requests)+ getListSize(currently_handled)>=queue_size_limit){
        pthread_cond_wait(&queue_full,&thread_lock);
    }
    pushBack(waiting_requests, req);
    pthread_cond_signal(&queue_empty);
    //pushing back??
}
int main(int argc, char *argv[])
{

    int listenfd, connfd, port, clientlen, threads, queue_size, max_size=0;
    struct sockaddr_in clientaddr;
    char* shedlag;

    pthread_mutex_init(&thread_lock, NULL);//mutex init
    pthread_cond_init(&queue_empty, NULL);
    pthread_cond_init(&queue_full, NULL);

    getargs(&port, &threads, &queue_size , &shedlag,&max_size,argc, argv);
    int shedalgo= getSchedAlgo(shedlag);
    pthread_t* worker_threads=(pthread_t*)malloc(sizeof(pthread_t)*(threads)); // allocating the array of worker threads
    threads_stats * thread_stat_array= malloc(sizeof (threads_stats)*(threads));
//    pthread_t* worker_threads=(pthread_t*)malloc(sizeof(pthread_t)*(threads+1)); // allocating the array of worker threads
//    threads_stats * thread_stat_array= malloc(sizeof (threads_stats)*(threads+1));
    //create two lists.
    int list_size=queue_size;
    waiting_requests= listCreate(list_size);// todo make sure of arg
    currently_handled= listCreate(list_size);
    //
    // HW3: Create some threads...
    //

    for (int i = 0; i < threads; i++) {
        //int* index= malloc(sizeof (int));
        //*index=i;
        threads_stats t_stat=createThreadStats(i);
        thread_stat_array[i]=t_stat;
        pthread_create(&worker_threads[i], NULL, workerFunction, (void*)(thread_stat_array[i]));//todo syntax
        //todo check the last parameter's syntax.
    }
    /////////////////////////////////I WANT TO CREATE THE VIP THREAD/////////////////////////
//    threads_stats t_stat=createThreadStats(threads);
//    thread_stat_array[threads]=t_stat;
//    pthread_create(&worker_threads[threads], NULL, workerFunction, (void*)(thread_stat_array[threads]));
    ////////////////////////////////////////////////////////////////////////////////////////
    ///*******************************/*/
    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        Request current_request= createRequest(connfd);
        pthread_mutex_lock(&thread_lock);
        if(getListSize(waiting_requests)+ getListSize(currently_handled)<queue_size){

            //SHOULD CHECK VIP REQUESTS
            //if(getRequestMetaData(connfd))
            //pushBack(vip_requests, current_request);

            //there are enough buffers.
            pushBack(waiting_requests, current_request);
            pthread_cond_signal(&queue_empty);//waiting to be a "place" to handle
            // the request.
            pthread_mutex_unlock(&thread_lock);
            continue;

        }
            //todo handle cases..
        else{
            //part 2 policies
            if(shedalgo==BLOCK){
                Block(queue_size, current_request);
                //I added the following to the wrapper
                //pushBack(waiting_requests, current_request);
                //pthread_cond_signal(&queue_empty);//waiting to be a "place" to handle
            }
            else if(shedalgo==DROP_TAIL){
                DropTail(connfd);
                free(current_request);
            }
            else if(shedalgo==DROP_HEAD){
                DropHead(connfd, current_request);
                //

            }
            else if(shedalgo==BLOCK_FLUSH){
                BlockFlush(current_request);
            }
            else if(shedalgo==RANDOM){
                drop_random(waiting_requests, connfd, current_request, queue_size);
            }
            pthread_mutex_unlock(&thread_lock);
        }
    }
}



    


 
