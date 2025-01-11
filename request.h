#ifndef __REQUEST_H__
#define __REQUEST_H__


typedef struct Threads_stats{
	int id;
	int stat_req;
	int dynm_req;
	int total_req;
} * threads_stats;
typedef struct request{
    int connfd;
    struct timeval arrival_time;
    struct timeval dispatch_interval;
    struct timeval pickUp_time;
    //thread statics?
}*Request;

Request createRequest(int connfd);
//destroyReq?
threads_stats createThreadStats(int thread_id);
// handle a request
void requestHandle(int fd, struct timeval arrival, struct timeval dispatch, threads_stats t_stats);

//  Returns True/False if realtime event
int getRequestMetaData(int fd);

#endif
