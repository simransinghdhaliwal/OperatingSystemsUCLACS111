//NAME: Simran Dhaliwal
//EMAIL: sdhaliwal@ucla.edu
//ID: 905361068

#include "SortedList.h"
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <string.h>


void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
    //if the list is empty insert the new element and let list
    //point to it
    if(list->next == NULL){ //empty list
    //critical section
        if(opt_yield & INSERT_YIELD)
            sched_yield();
        list->next = element;
        list->prev = element;
        element->prev = list;
        element->next = list; 
        return;
    }

    //If the list is not empty, need to traverse to where we should insert the
    //element
    SortedListElement_t *temp = list->next;
    while(temp != list){//while we do not reach end the of the list
        if(strcmp(element->key, temp->key) < 0) //if the elment's key is less than the temp->key insert here
            break; 
        temp = temp->next; //if the element key is greater than the temp key, keep traversing
    }

    //critical section below
    if(opt_yield & INSERT_YIELD) //yield if need be
        sched_yield();

    //if the element is greater than everything inside of the list, then make it new end of list
    if(temp == list){ 
        list->prev->next = element; //new prev of list is element
        element->prev = list->prev;
        element->next = list;
        list->prev = element;
        return;
    }
    //only in case if we need to insert between two elements
    temp->prev->next = element;
    element->next = temp;
    element->prev = temp->prev;
    temp->prev = element;
}

int SortedList_delete(SortedListElement_t *element){
    if(element == NULL || element->key == NULL) //if the element is null or key, invalid delete
        return 1;
    if(element->next != NULL && element->prev != NULL) //check connections between node were are deleting
       if(element->prev->next != element || element->next->prev != element)
            return 1;
    //critical section below, because of swapping
    if(opt_yield & DELETE_YIELD) //yield if we need to 
        sched_yield();

    //swapping to get remvove element from the list
    element->prev->next = element->next;
    element->next->prev = element->prev;
    return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
    if(key == NULL || list == NULL) //making sure that both the key and list are non-corrupted
        return NULL;
    SortedListElement_t* temp = list->next;
    //need to traverse the list until we reach where key is equal to current nodes key
    while(temp != list){
        //critical section
        if(opt_yield & LOOKUP_YIELD)
            sched_yield();
        if(strcmp(key, temp->key) == 0)
            return temp; //in the case we find the node, return it
        temp = temp->next;
    }
    return NULL;
}

int SortedList_length(SortedList_t *list){
    int length = 0; //inital length of list is zero
    if(list == NULL) //if list is corrupted return -1
        return -1;
    SortedListElement_t* temp = list->next;
    while(temp != list){ //go through list until you make it back to the list node
        //critical section
        if(opt_yield & LOOKUP_YIELD)
            sched_yield();
        temp = temp->next;
        length++; //increase the length of the list every iteration
    }
    return length;   
}