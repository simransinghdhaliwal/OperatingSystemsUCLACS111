//NAME: Simran Dhaliwal
//EMAIL: sdhaliwal@ucla.edu
//ID: 905361068

#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "SortedList.h"

void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
	if(list == NULL) //if list is nullptr
		return;


	if(opt_yield & INSERT_YIELD)
		sched_yield();
	
	if(list->next == NULL){//means that the list is empty
		list->next = element;
		element->next = list;
		element->prev = list;
		list->prev = element;
		return;
	}

	//list already have entires
	SortedListElement_t* temp = list->next;
	while(temp != list){
		if(strcmp(element->key, temp->key) < 0){ //come across place to enter
			temp->prev->next = element;
			element->prev = temp->prev;
			temp->prev = element;
			element->next = temp;
			return;	
		}
		temp = temp->next;
	}

	//in the case temp = list
	temp->prev->next = element;
	element->prev = temp->prev;
	temp->prev = element;
	element->next = temp;

}

int SortedList_delete(SortedListElement_t *element) {
	if(element == NULL || element->key == NULL) 
		return 1;
	if(element->next->prev != element || element->prev->next != element)
		return 1;
	
	if(opt_yield & DELETE_YIELD)
		sched_yield();

    
	element->prev->next = element->next;
	element->next->prev = element->prev;
		
	return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
	if(list == NULL || key == NULL)
		return NULL;
	if(opt_yield & LOOKUP_YIELD)
		sched_yield();
	SortedListElement_t* temp = list->next;
	while(temp != list){
		if(strcmp(temp->key, key) == 0)
			return temp;
		temp = temp->next;
	}

	return NULL;
}

int SortedList_length(SortedList_t *list) {
	if(list == NULL)
		return -1;
	if(opt_yield & LOOKUP_YIELD)
		sched_yield();
	if(list->next == NULL)
		return 0;
	int count = 0;
	SortedListElement_t * temp = list->next;
	while(temp != list)
	{
	  if(temp->next->prev != temp || temp->prev->next != temp)
	    return 1;
	  temp = temp->next;
	  count++;
	}
	return count;
}
