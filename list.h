#ifndef LIST_H
#define LIST_H

#endif
#include <sys/time.h>
#include "request.h"

typedef struct Node{
    Request request;
    struct Node* prev;
    struct Node* next;
    
} *Node;

typedef struct List_t{
    Node tail;
    Node head;
    int curr_size;
    int max_size;
    
} *List;


List create_list(int max_size);
void destroy_list(List list);
int get_list_size(List list);
void push_back(List list, Request request);
Request pop(List list);
Request pop_by_index(List list ,int index);
int find_by_sfd(List list, int sfd);
