#ifndef __REQUEST_H__
#define __REQUEST_H__


typedef struct Threads_stats{
	int id;
	int stat_req;
	int dynm_req;
	int total_req;
	
} * threads_stats;

typedef struct Request{
    int conn_fd;
    struct timeval arrival_t;
    struct timeval dispatch_int;
    struct timeval pick_up_t;
    
}*Request;

void check_and_remove_skip(char *filename, int *is_skip);
Request create_request(int fd);
threads_stats create_threads_stats(int thread_id);

// handle a request
void requestHandle(int fd, struct timeval arrival, struct timeval dispatch, threads_stats t_stats,int* is_skip);

//  Returns True/False if realtime event
int getRequestMetaData(int fd);

#endif
