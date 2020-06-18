//NAME: Simran Dhaliwal
//EMAIL: sdhaliwal@ucla.edu
//ID: 905361068

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <zlib.h>
#include <assert.h>
#include <string.h>
#include <sys/wait.h>
#define BUFFER 256

//z streams for compressing and decompressing
z_stream com;
z_stream decom;

pid_t pid;
int sockfd;
int new_sockfd;

char helper[] = "\r\n";

int c_flag = 0; //flag is set to one when we compress

void system_call_fail(char* name)
{
  fprintf(stderr, "Program is ending, error occurred with %s.\n", name);
}

//function to set up pipes for which process calls it, straight from lab1a
void fd_set_up(pid_t pid, int* terminal_to_shell, int* shell_to_terminal){

  if(pid == 0) //in the case the process is the child process
  {
    //first need to close the unused file descriptors
    //i.e. write from terminal to shell, and the read from shell to terminal
    close(terminal_to_shell[1]); 
    close(shell_to_terminal[0]);

    //now need to set the standards with the pipes, per instructions
    dup2(terminal_to_shell[0], 0); //standard in (0) is pipe from terminal process
    dup2(shell_to_terminal[1], 1); //standard out (1) is pipe to terminal process
    dup2(shell_to_terminal[1], 2); //standard error (2) is pipe to terminal process

  }
  else if(pid > 0) //parent process
  {
    close(terminal_to_shell[0]); //need to close read from terminal to shell
    close(shell_to_terminal[1]); //need to close write from shell to terminal 
  }
} 

//same two functions as the lab1b-client
void compress_helper(char* source, int *source_size, char * compress, int * compress_size){
  int check = Z_OK;
  com.avail_in = *(source_size);
  com.next_in = (Bytef *) source;
  com.avail_out = *(compress_size);
  com.next_out = (Bytef *)compress;
  while(check == Z_OK)
    check = deflate(&com, Z_SYNC_FLUSH);
  *(compress_size) = *(compress_size) - com.avail_out;
}

void decompress_helper(char * source, int *source_size, char *decompress, int * decompress_size){
  decom.avail_in = *(source_size);
  decom.next_in = (Bytef*)source;
  decom.avail_out = *(decompress_size);
  decom.next_out = (Bytef*)decompress;
  int check = Z_OK;
  while(check == Z_OK)
    check = inflate(&decom, Z_SYNC_FLUSH);
  *(decompress_size) = *(decompress_size) - decom.avail_out;
}

//is called to finish processing what terminal(parent process) needs to do
void parent_process_routine(struct pollfd* poll_list, int* terminal_to_shell, int* shell_to_terminal, pid_t pid)
{
  int val; //val that will return value of poll comand 

  //The while-loop below is to deal with the poll() sys call
  //taken from example cited in the README
  while(1){
    int i;
    val = poll(poll_list, 2, 0);
    if(val == -1)  //if val == -1, syscall poll failed
      system_call_fail("poll()");

    //when there is input from the keyboard from client
    if((poll_list[0].revents&POLLIN) == POLLIN){
      char *buffer;            //read buffer(just like work buffer in client)
      char read_buffer[256];   //read_buffer
      char decom_buffer[256];  //holds decompressed data if necessary
      int decom_buffer_size = BUFFER;
      int chars_read = read(poll_list[0].fd, &read_buffer, BUFFER); //read from sock and hold number of characters read
      if(c_flag != 1){ //if there is no need to decompress, just set buffer to whatever you read
        buffer = read_buffer;
      }
      else{ //if compression is indicated, decompress into decom_buffer
        decompress_helper(read_buffer, &chars_read, decom_buffer, &decom_buffer_size);
        chars_read = decom_buffer_size;
        buffer = decom_buffer;
      }
      //go through buffer and do appropriate things, no modifidying of buffer
      for(i = 0; i < chars_read; i++){
        char aa = buffer[i];
        switch(aa)
        {
          case 4: //close pipe and exit
            close(shell_to_terminal[0]);
            close(terminal_to_shell[1]);
            goto EXIT;
            break;
          case 3: //send kill 
            close(sockfd);
            close(new_sockfd);
            if (kill(pid, SIGINT) == -1) //if ^C is entered, send a SIGINT
              system_call_fail("kill()");
            break;
          default: // forward to shell      
            write(terminal_to_shell[1], &aa, 1);
          }

        }
    }

    if(((poll_list[1].revents&POLLHUP) == POLLHUP)){ //if shell is done
      goto EXIT;
    }
    else if((poll_list[1].revents&POLLIN) == POLLIN){
      char buffer[BUFFER];  
      int chars_read = read(poll_list[1].fd, &buffer, BUFFER);  //read in from the shell, chars_read is number chars read
      if(c_flag != 1)      //if no compression, send from shell straight to client
        write(new_sockfd, buffer, chars_read);
      else{ //compressing
        char compress[BUFFER];
        int compress_size = BUFFER;
        compress_helper(buffer, &chars_read, compress, &compress_size);
        //write compressed buffer to client
        write(new_sockfd, compress, compress_size);
      }
    }
  }
  
  EXIT:
  return;
}

void end_helper(){
    deflateEnd(&com);
    inflateEnd(&decom);
}


//taken from lab1a to handle SIGNIT
void sig_handler(){
  exit(0);
}

int main(int argc, char*  argv []){
  //pipes taken from lab1a
  int terminal_to_shell[2]; 
  int shell_to_terminal[2];
  char * program_name;
  int port_number;

  //options in order to parse the arguments
  static struct option long_options[] = {
    {"port", required_argument, 0, 'p'},
    {"compress", no_argument, 0, 'c'},
    {"shell", required_argument, 0, 's'},
    {0,0,0,0}
  };
  //go through the arguments for the function.
  char ch;
  while((ch = getopt_long(argc, argv, "", long_options, NULL)) != -1){
    switch(ch){
    case 'p': //if shell option is selected set, sh to 1
      port_number = atoi(optarg); //port number into an int
      break;
    case 'c': 
      c_flag = 1;  //indicate if they compression flag was selected
      break; 
    case 's':
      program_name = optarg; //save the name of the program we are going to run
      break;
    default: //unrecognized arguments will prompt a proper usage messgae and along with exit
      fprintf(stderr, "Proper Usage: lab1b-server [--shell ProgramName]\n");
      exit(1);
    }; 
  }

  if(c_flag == 1){
    com.zalloc = decom.zalloc = Z_NULL;
    com.zfree = decom.zfree = Z_NULL;
    com.opaque = decom.opaque = Z_NULL;
    if ((deflateInit(&com, Z_DEFAULT_COMPRESSION) != Z_OK) || (inflateInit(&decom) != Z_OK )){
        fprintf(stderr, "Something went wrong with the deflateInit or infalteInit.\n");
        exit(1);
    }
  }


//From the tutorial, cited in the README
  struct sockaddr_in serv_addr, cli_addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0)
    system_call_fail("sock()");

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port_number);
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
    system_call_fail("bind()");
    exit(1);
  }
  listen(sockfd, 5);
  socklen_t clilen = sizeof(cli_addr);
  new_sockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
  if(new_sockfd < 0)
    system_call_fail("accept()");


//if we make it down here, we forsure doing shell commands
  if(pipe(terminal_to_shell) == -1 || pipe(shell_to_terminal) == -1)
    system_call_fail("pipe()");
  
  //we are doing the --shell option
  //terminal is the parent process and the shell is the child process
  pid = fork();
  if(pid > 0){ //In the case of pid > 0 meaning we are dealing with parent process
    fd_set_up(pid, terminal_to_shell, shell_to_terminal);

    //create array of two pollfd structures 
    struct pollfd poll_list[2];
    poll_list[0].fd = new_sockfd;        //make one describe keyboard of stdin
    poll_list[1].fd = shell_to_terminal[0];//make the other one read from shell to terminal
    //have both structs wait for POLLIN or error (POLLHUP, POLLERR)
    poll_list[0].events = POLLIN | POLLHUP | POLLERR;
    poll_list[1].events = POLLIN | POLLHUP | POLLERR;

    //call to function that takes poll_list and pipes to deal with the rest of what the parent process
    //has to do
    parent_process_routine(poll_list, terminal_to_shell, shell_to_terminal, pid);

    close(terminal_to_shell[1]);
    //collecting shell's exit status and report to stderr
    int status = 0;
    if (waitpid(0, &status, 0) == -1)
      system_call_fail("waitpid()");
    int return_status = WEXITSTATUS(status);
    int return_signal = WTERMSIG(status);
    fprintf(stderr,"SHELL EXIT SIGNAL=%d STATUS=%d\n", return_signal, return_status);
  }
  else if(pid == 0){
    fd_set_up(pid, terminal_to_shell, shell_to_terminal);
    char *args[] = {program_name, NULL};
    
    if(execvp(program_name, args) == -1){
      fprintf(stderr, "Could not launch program %s. ", program_name);
      system_call_fail("Execvp");
    }
  }
  else //if pid is any other value, we know it failed so we call appropriate function
    system_call_fail("fork()");

  if(c_flag == 1)
    end_helper();
  close(sockfd);
  close(new_sockfd);
  exit(0);
}

