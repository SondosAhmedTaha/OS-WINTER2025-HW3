//
// Created by student on 6/9/23.
//
#include "list.h"
#include <stdlib.h>
//todo add more functions and maybe fields in node...
static Node nodeCreate(Request request)
{
    Node new_node= (Node)malloc(sizeof(*new_node));
    if(new_node==NULL){
        return NULL;
    }
    new_node->request=request;
    new_node->next=NULL;
    //new_node->previous=??; //should be the last previous
    return new_node;
}

/*static void nodeDestroy(Node node){
    free(node);
}*/

List listCreate(int max_size){
    List list=(List) malloc(sizeof(*list));
    if(list==NULL){
        return NULL;
    }
    list->curr_size=0;
    list->max_size=max_size;
    list->head=NULL;
    list->tail=NULL;
    return list;
}
void listDestroy(List list){
    if(list==NULL){
        return;
    }
    free(list);
}

int getListSize(List list){
    if(list==NULL)
    {
        return -1;
    }
    return list->curr_size;
}

/* int getListMaxSize(List list){
    if(list==NULL)
    {
        return -1;
    }
    return list->max_size;
}
*/

void pushBack(List list, Request request){
    if(list==NULL)
    {
        return;
    }
    if(getListSize(list)==list->max_size){
        return;
    }
    Node new_node= nodeCreate(request);
    if(getListSize(list)==0){
        list->head=new_node;
        list->tail=new_node;
        //new_node->next=NULL;
        //new_node->previous=NULL;
    }
    else{
        list->tail->next=new_node;
        new_node->previous=list->tail;
        list->tail=new_node;
    }
    list->curr_size++;
    return;
}

Request pop(List list){
    if(list==NULL){
        return NULL;
    }
    if(getListSize(list)==0){
        return NULL;
    }
    Node temp=list->head->next;
    Request request=list->head->request;
    if(getListSize(list)==1){
        free(list->head);
        list->head=NULL;
        list->tail=NULL;
    }
    else{
        free(list->head);
        list->head=temp;
        temp->previous=NULL;
    }
    list->curr_size--;
    return request;
}


Request popByIndex(List list ,int index){ // todo changed
    if (list==NULL || index<0 || index>= getListSize(list) || getListSize(list)==0){
        return NULL ;
    }
    Node it=list->head;
    for(int i=0; i<index;i++)
    {
        it=it->next;
    }
    //int sfd=it->request->connfd; //todo check
    Node temp= it->previous;
    if(!temp || index <= 0){
        return pop(list);
    }
    temp->next = it->next;
    if(temp->next){
        temp->next->previous=temp;
    }
    if(index >= getListSize(list) -1) {
        list->tail=temp;
    }
      //list->tail=temp->previous;
    Request temp_req = it->request;
      free(it);
    list->curr_size --;
    return temp_req;
}

int findThreadBySfd(List list, int sfd){
    if(list==NULL){
        return -1;
    }
    if(getListSize(list)==0){
        return -1;
    }
    Node it=list->head;
    for(int i=0; i< getListSize(list); i++){
        if(it->request->connfd==sfd){ //todo check
            return i;
        }
        it=it->next;
    }
    return -1;
}

