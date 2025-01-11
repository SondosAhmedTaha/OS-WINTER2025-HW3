//
// Created by student on 6/9/23.
//

#ifndef WEBSERVER_FILES_LIST_H
#define WEBSERVER_FILES_LIST_H

#endif //WEBSERVER_FILES_LIST_H
#include <sys/time.h>
#include "request.h"
//todo add edit
typedef struct Node_t{
    Request request;
    struct Node_t* previous;
    struct Node_t* next;
} *Node;

typedef struct List_t{
    Node tail;
    Node head;
    int curr_size;
    int max_size;
} *List;

//push with Request instead of ...
List listCreate(int max_size);
void listDestroy(List list);
int getListSize(List list);
//adding to the tail
//todo add edit
void pushBack(List list, Request request);
Request pop(List list);
Request popByIndex(List list ,int index);
int findThreadBySfd(List list, int sfd);
