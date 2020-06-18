//NAME: Simran Dhaliwal
//EMAIL: sdhaliwal@ucla.edu
//ID: 905361068

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>

long long counter;
long iteration_num = 1;
char* name_of_test = "add-";


//mutex lock
pthread_mutex_t lock;
int lock_s = 0; //for spin lock

//flags
int opt_yield = 0; // 1 means that yield was selected
char opt_sync = 'k'; //m is mutex, s is spinlock, and c is compare and swap
char* sync_str = "test";

//function pointer, for easy transitions
void (*add_ptr)(long long *pointer, long long value) = NULL;

//vanilla add 
void add(long long *pointer, long long value){
  long long sum = *pointer + value;
  if (opt_yield)
    sched_yield();
  *pointer = sum;
}

//add with mutex lock
void add_mutex(long long *pointer, long long value){
  pthread_mutex_lock(&lock);  //lock mutex
  long long sum = *pointer + value;
  if (opt_yield)
    sched_yield();
  *pointer = sum;
  pthread_mutex_unlock(&lock); //unlock mutex
}

//add with spin
void add_spin(long long *pointer, long long value){
  while(__sync_lock_test_and_set(&lock_s,1)); //starting spinning if need be
  long long sum = *pointer + value;
  if (opt_yield)
    sched_yield();
  *pointer = sum;
  __sync_lock_release(&lock_s); //release the lock
}

//add with compare and swap
void add_cas(long long *pointer, long long value){
  long long old_value;
  long long new_value;
  do{
    old_value = *pointer;    //old value of count
    new_value = old_value + value; //new value of the count
    if(opt_yield)
      sched_yield();

  }while(__sync_val_compare_and_swap(pointer, old_value, new_value) != old_value);
}


//function that helps with threads, enables thread the use the proper add function
void*  add_helper(void* a){
  long j = 0;
  for (j = 0; j < iteration_num; j++){
    (*add_ptr)(&counter, 1);
    (*add_ptr)(&counter, -1);
  }
  return a;
}

//return time in nanoseconds
long long get_time(struct timespec  a){
  long long bill = 1000000000;
  return ((bill* a.tv_sec) + a.tv_nsec);
}

int main(int argc, char ** argv){
  struct timespec start, end;

  //one thread default
  long  thread_num = 1;
  counter = 0; //initialize long long counter to zero


  //array for options
  static struct option long_options[] = {
    {"threads", required_argument, 0, 't'},
    {"iterations", required_argument, 0, 'i'},
    {"yield", no_argument, 0, 'y'},
    {"sync", required_argument, 0, 's'},
    {0,0,0,0}
  };

  char ch;
  while((ch = getopt_long(argc, argv, "", long_options, NULL)) != -1){
    switch(ch){
    case 't':
      thread_num = atoi(optarg); //getting number of threads
      break;
    case 'i':
      iteration_num = atoi(optarg); //getting number of iterations 
      break;
    case 'y':
      opt_yield = 1; //if we are using the yield option
      break;
    case 's':
      opt_sync = *optarg;
      if((opt_sync == 'm' || opt_sync == 's' || opt_sync == 'c') && strlen(optarg) == 1) //make sure only m,s or c and size of 1
      {
        if(opt_sync == 'm'){ //initialize the mutex lock, if m is selected
          pthread_mutex_init(&lock, NULL);
        }
        break;
      }
    default:
      fprintf(stderr, "Proper Usage: lab2a_add [--threads NumberOfThreads --iterations NumberofIterations] \n");
      exit(1);
    };
  }

    //calculating output
  if(opt_yield && opt_sync == 'k')
    name_of_test = "add-yield-none";
  else if(!opt_yield && opt_sync != 'k'){ //if not yielding but synchronizing
    if(opt_sync == 's')
      name_of_test = "add-s";
    else if(opt_sync == 'c')
      name_of_test = "add-c";
    else if(opt_sync == 'm')
      name_of_test = "add-m";
  }
  else if(opt_yield && opt_sync != 'k'){ //if yielding and synchronizing
    if(opt_sync == 's')
      name_of_test = "add-yield-s";
    else if(opt_sync == 'c')
      name_of_test = "add-yield-c";
    else if(opt_sync == 'm')
      name_of_test = "add-yield-m";
  }
  else
    name_of_test = "add-none";


  //make add_ptr point to the proper implementation of add
  if(opt_sync == 'k')
    add_ptr = &add;        //vanilla add
  else if(opt_sync == 'm') //mutex add
    add_ptr = &add_mutex;
  else if(opt_sync == 'c')
    add_ptr = &add_cas;    //compare and swap 
  else if(opt_sync == 's')
    add_ptr = &add_spin;   //spin-lock
  
  //starting specificed number of threads
  pthread_t* threads = malloc(thread_num * sizeof(pthread_t));

  clock_gettime(CLOCK_REALTIME, &start); //starting time for the clock ==> check if this is right
  
  //starting each thread
  int i = 0;
  for(i = 0; i < thread_num; i++) //each function will run the add-helper
    pthread_create(&threads[i], NULL, add_helper, NULL); 

  //threads join each other when they are done
  for(i = 0; i < thread_num; i++)
    pthread_join(threads[i], NULL);


  //get the ending time 
  clock_gettime(CLOCK_REALTIME, &end);

  //calculating output
  long total_operations = 2 * thread_num * iteration_num; //finding total amount of oeprations
  long total_run_time = get_time(end)  - get_time(start); //finding total runtime of the program
  long avg_time = total_run_time / total_operations;      //average time per operation
  
  fprintf(stdout, "%s,%ld,%ld,%ld,%ld,%ld,%lld\n",name_of_test, thread_num, iteration_num, total_operations, total_run_time, avg_time, counter);


  if(opt_sync == 'm') //destroying mutex lock
    pthread_mutex_destroy(&lock);
  free(threads); //free dynamically allocated threads
  exit(0);
};
