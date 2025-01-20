#include "list.h"
#include <stdlib.h>

static Node create_node(Request request)
{
    Node new_node = (Node)malloc(sizeof(*new_node));
    if(new_node == NULL) { return NULL; }
    new_node->request = request;
    new_node->next = NULL;
    return new_node;
}

List create_list(int max_size){
    List list = (List) malloc(sizeof(*list));
    if(list == NULL) { return NULL; }
    list->head = NULL;
    list->tail = NULL;
    list->curr_size = 0;
    list->max_size = max_size;
    return list;
}

void destroy_list(List list){
    if(list == NULL) { return; }
    free(list);
}

int get_list_size(List list){
    if(list == NULL) { return -1; }
    return list->curr_size;
}

void push_back(List list, Request request){
    if(list == NULL) { return; }
    if(get_list_size(list) == list->max_size) { return; }
    Node new_node = create_node(request);
    if(get_list_size(list) == 0){
        list->head = new_node;
        list->tail = new_node;
    }
    else{
        list->tail->next = new_node;
        new_node->prev = list->tail;
        list->tail = new_node;
    }
    list->curr_size++;
    return;
}

Request pop(List list){
    if(list == NULL) { return NULL; }
    if(get_list_size(list) == 0) { return NULL; }
    Node temp = list->head->next;
    Request request = list->head->request;
    if(get_list_size(list) == 1){
        free(list->head);
        list->head=NULL;
        list->tail=NULL;
    }
    else{
        free(list->head);
        list->head=temp;
        temp->prev=NULL;
    }
    list->curr_size--;
    return request;
}

Request pop_by_index(List list ,int index){
    if (list == NULL || index < 0 || index >= get_list_size(list)
		|| get_list_size(list) == 0){
        return NULL ;
    }
    Node node = list->head;
    for(int i = 0; i < index; i++) { node = node->next; }
    
    Node temp = node->prev;
    if(!temp || index <= 0){
        return pop(list);
    }
    temp->next = node->next;
    if(temp->next){
        temp->next->prev=temp;
    }
    if(index >= get_list_size(list) -1) {
        list->tail=temp;
    }
    Request temp_req = node->request;
    free(node);
    list->curr_size --;
    return temp_req;
}

int find_by_sfd(List list, int sfd){
    if(list == NULL) { return -1; }
    if(get_list_size(list) == 0) { return -1; }
    Node node = list->head;
    for(int i = 0; i < get_list_size(list); i++){
        if(node->request->conn_fd == sfd){
            return i;
        }
        node=node->next;
    }
    return -1;
}

