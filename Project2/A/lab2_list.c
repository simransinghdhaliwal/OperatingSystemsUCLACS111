//NAME: Simran Dhaliwal
//EMAIL: sdhaliwal@ucla.edu
//ID: 905361068

#include <stdio.h>
#include <stdlib.h>
#include "SortedList.h"
#include <getopt.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

long iteration_num = 1; //number of iterations
long  thread_num = 1;   //number of threads

//array that will hold all the elements of the list
SortedListElement_t* list_elements;
//list will be the list that will be populated
SortedList_t list;

int opt_yield = 0;

int element_num = 1;
int i = 0;
char  nl = '\n';

//locks
pthread_mutex_t mutex;
int spin_lock = 0;
//lock flags
int flag_mutex = 0;
int flag_spin = 0;

char letter_pool[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";


void system_call_fail(char* name)
{
  fprintf(stderr, "Program is ending, error occurred with %s.\n", name);
}

//will generate a random string based on the size passed in 
char * random_key_generator(int size){
  int rand_num = 0; //allocate a key
  char * key = malloc((size+1) * sizeof(char));
  if(key == NULL)
    system_call_fail("malloc");
  int j = 0;
  for(; j < size; j++){
    rand_num = rand() % 52; //random letter a-z
    key[j] = letter_pool[rand_num];
  }
  key[size] = '\0';
  return key; //return random string
}

//set up the elements in list elements
void set_up_elements(void){
  //allocate enough space
  list_elements = malloc(element_num * sizeof(SortedListElement_t));
  if(list_elements == NULL)
    system_call_fail("malloc");
  //populate each element with a random key
  for(i = 0; i < element_num; i++){
    char * rand_key = random_key_generator(5);
    list_elements[i].key = rand_key;
    list_elements[i].next = NULL;
    list_elements[i].prev = NULL;
  }
  
}

//helper function that will output message is list becomes corrupted.
void test_fail(char* failure){
  fprintf(stderr, "Failure in %s. \n", failure);
  exit(2);
}

//return time in nanoseconds
long long get_time(struct timespec  a){
  long long bill = 1000000000;
  return ((bill* a.tv_sec) + a.tv_nsec);
}

//helper function that calls the proper locking mechanism depending on the synchronizing option
void lock(void){
  if(flag_mutex)
    pthread_mutex_lock(&mutex);
  else if(flag_spin)
    while(__sync_lock_test_and_set(&spin_lock,1));
}

//helper function that calls the proper unlocking mechanism depending on the synchronizing option
void unlock(void){
  if(flag_mutex)
    pthread_mutex_unlock(&mutex);
  else if(flag_spin)
    __sync_lock_release(&spin_lock);
}


//function that will be used with the threads
void* list_helper(void *a){  
  //a is the set of elements size of iterations that is pased in
  SortedListElement_t* b = a; //need to be converted from void* to SortedListElement_t*
  //critical section in sort into list
  lock();
  int count1 = 0;
  for(; count1 < iteration_num; count1++){
    SortedList_insert(&list, &b[count1]);
  }
  unlock();

  //critical section is to get length
  lock();
  int length = SortedList_length(&list);
  unlock();

  if(length < iteration_num)
    test_fail("SortListed_insert");

  //critical section is to search for element and delete it 
  lock();
  int count2 = 0;
  for(count2 = 0; count2 < iteration_num; count2++){
    const char* temp_key = b[count2].key;
    SortedListElement_t* search_element = SortedList_lookup(&list, temp_key);
    if(search_element == NULL)
      test_fail("SortedList lookup");
    int test_delete = SortedList_delete(search_element);
    if(test_delete == 1)
      test_fail("SortedList delete");
  }  
  unlock();
  
  return NULL;
}

//handle the segfaults
void seg_handle(){
  fprintf(stderr, "Failure caused by segfault. \n");
  exit(2);
}

int main(int argc, char ** argv){
  //seed the random number, and sigal in case of segfault
  srand(time(0));
  signal(SIGSEGV, seg_handle);
  struct timespec start, end;

  char* yield_options = "none"; //hold idl
  char* sync_opts = "none"; //will hold the none 
  char* test_name = "list-";
  char* hyphen = "-";
  char name[25];
  strcpy(name, test_name);

  //array of options 
  static struct option long_options[] = {
    {"threads", required_argument, 0, 't'},
    {"iterations", required_argument, 0, 'i'},
    {"yield", required_argument, 0, 'y'},
    {"sync", required_argument, 0, 's'},
    {0,0,0,0}
  };


  //dealing with parameters
  char ch;
  while((ch = getopt_long(argc, argv, "", long_options, NULL)) != -1){
    switch(ch){
    case 't':
      thread_num = atoi(optarg); //getting number of threads
      break;
    case 'i':
      iteration_num = atoi(optarg);
      break;
    case 'y':
      //opt_yield = 1;
      yield_options = optarg;
      //make sure that yeild options are three characters at most
      if(strlen(yield_options) > 3)
      {
        fprintf(stderr, "Invald use of yeild option. Proper Usage --yield=[idl]. \n");
        exit(1);
      }
      unsigned int i = 0;
      //go through the yield_options letter by better each time searching for another a 
      //a different character
      for(; i < strlen(yield_options); i++){
        if(yield_options[i] == 'i'){
          opt_yield |= INSERT_YIELD;
          strcat(name, "i");
        }
      }
      for(i = 0; i < strlen(yield_options); i++){
        if(yield_options[i] == 'd'){
          opt_yield |= DELETE_YIELD;
          strcat(name, "d");
        }
      }
      for(i = 0; i < strlen(yield_options); i++){
        if(yield_options[i] == 'l'){
          opt_yield |= LOOKUP_YIELD;
          strcat(name, "l");
        }
      }
      strcat(name, hyphen);
      for(i = 0; i < strlen(yield_options); i++){
        if(yield_options[i] != 'l' && yield_options[i] != 'd' && yield_options[i] != 'i'){
          fprintf(stderr, "Invald use of yeild option. Proper Usage --yield=[idl]. \n");
          exit(1);
        }
      } 
    
      break;
    case 's':
      sync_opts = optarg; //get the parameters from opt_long
      if(strlen(sync_opts) >= 2){
        fprintf(stderr, "Invalid use of --sync, Proper Usage: --sync [s|m]. \n");
        exit(1);
      }
      //go through the sync optiosn to see if there needs ot be mutex or spin
      for(i = 0; i < strlen(sync_opts); i++){
        if(sync_opts[i] == 'm')
          flag_mutex = 1;
        else if(sync_opts[i] == 's')
          flag_spin = 1;
        else{
          fprintf(stderr, "Invald use of sync. Proper Usage --yield=[s|m]. \n");
          exit(1);
        }
      }
      break;
    default:
      fprintf(stderr, "Proper Usage: lab2a_list [--threads NumberOfThreads --iterations NumberofIterations --sync=[m|s] --yield=[idl]]\n");
      exit(1);
    };
  }

  //how many elements should there be
  element_num = iteration_num * thread_num;
  pthread_t * threads = malloc(thread_num * sizeof(pthread_t)); //array of dynamically created threads
  if(threads == NULL)
    system_call_fail("malloc()");
  set_up_elements();
  
  clock_gettime(CLOCK_REALTIME, &start);

  int k = 0;
  for(; k < thread_num; k++){ //each thread is given an equal amount of elements to iterate through
    SortedListElement_t* set = (list_elements + (k * iteration_num)); //to give each thread a set of iteration_num eles.
    //ex. 2 threads, 3 iterations, list = a b c d e f, thread 1 gets a b c and thread 2 gets d e f
    pthread_create(&threads[k], NULL, list_helper, set);
  }
  int h = 0;
  for( h = 0; h < thread_num; h++)
    pthread_join(threads[h], NULL);

  clock_gettime(CLOCK_REALTIME, &end);  //getting time after all of the operations have been completed
  
  //getting the final length of the list after all of the operations
  int final_length = SortedList_length(&list);
  if(final_length != 0)
    test_fail("Final length of List");

  //output processing
  long number_of_operations = thread_num * iteration_num * 3; //getting number of operations
  long total_run_time = get_time(end) - get_time(start); //getting the total run-time 
  long avg_time = total_run_time / number_of_operations;   //finding the avg time per operations (not corrected)
  if(strcmp(yield_options, "none") == 0)
  {
    strcat(name, yield_options);
    strcat(name, hyphen);
  }
  strcat(name, sync_opts);
  int num_of_list = 1;
  fprintf(stdout, "%s,%ld,%ld,%d,%ld,%ld,%ld\n",name, thread_num, iteration_num,num_of_list,number_of_operations, total_run_time, avg_time);

  if(flag_mutex) //destroying mutex lock
    pthread_mutex_destroy(&mutex);
  free(threads); //free array of dynamically allocated threads
  exit(0);
}
