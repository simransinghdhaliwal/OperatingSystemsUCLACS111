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
#define BUFFER 256

struct termios saved_attributes;
struct pollfd poll_list[2];

//z_stream to compress and decompress
z_stream decom;
z_stream com;

//flags for logging, compressings, and the port number
int l_flag = 0, c_flag = 0;
int fd_log_file = 0;


//c-strings to help with the logging :P
char nl = '\n'; //used when i need a new line
char helper[] = "\r\n";

//logging help, op = 1 sending, op = 0 rec, used both for input and output
void logging_help(int op, char * buffer, int size){
  char SENT[] = "SENT ";  
  char bytes[] = " bytes: ";
  char RECEIVED[] = "RECEIVED ";
  char result[20];
  sprintf(result, "%d", size);    //turnng size passed in into a c-string

  if(op == 1) //does the "SENT x bytes: "
    write(fd_log_file, SENT, strlen(SENT));                               
  else if(op == 0)//does the "RECEIVED x bytes: "
    write(fd_log_file, RECEIVED, strlen(RECEIVED));
  
  write(fd_log_file, result, strlen(result));
  write(fd_log_file, bytes, strlen(bytes));
  write(fd_log_file, buffer, size);
  write(fd_log_file, &nl, 1);
}


//taken from TA example, and cited material
void reset_input_mode(void)
{
  tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);
}

//simple function that will print CR and LF to terminal
void stdout_crlf(void){
  write(STDOUT_FILENO, helper, 2);
}


void set_input_mode(void)
{
  struct termios new_attributes;
  
  //saves terminal attributes so we can restore them at later point
  tcgetattr(STDIN_FILENO, &saved_attributes);
  atexit(reset_input_mode);
  
  //setting terminal modes
  tcgetattr(STDIN_FILENO, &new_attributes);
  new_attributes.c_cflag &= ~(ICANON|ECHO);
  new_attributes.c_cc[VMIN] = 1;
  new_attributes.c_cc[VTIME] = 0;
  new_attributes.c_iflag = ISTRIP;
  new_attributes.c_oflag = 0;
  new_attributes.c_lflag = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &new_attributes);
}

void end_helper(){
    deflateEnd(&com);
    inflateEnd(&decom);
}

//function to handle the failures of system calls 
//just pass in the name of the system call that failed
void system_call_fail(char* name)
{
  fprintf(stderr, "Program is ending, error occurred with %s.\n", name);
}

//buffer you are compressings and its size to compress buffer and its smaller size
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

//buffer you are decompressing and its size to decompress buffer to larger size
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

//function that deal with taking in input and outputting to proper locations
void client_process(struct pollfd* poll_list){
  int val = 0;

  while(1){
    val = poll(poll_list, 2, 0);
    if(val == -1)
      system_call_fail("poll()");
    
    int i = 0;
    if((poll_list[0].revents&POLLIN) == POLLIN){   //input from keyboard
      char buffer[BUFFER];
      int chars_read = read(poll_list[0].fd, &buffer, BUFFER);  //read in from keybard
      for(i = 0; i < chars_read; i++){   //for every char read
      	char aa = buffer[i];
	      switch(aa){
	      case '\r':    //FROM TA, if return carriage, then set it to \n
          buffer[i] = '\n';    
	      case '\n':
	        stdout_crlf(); //output <CR><LF>
	        break;
	      default:
	      write(STDOUT_FILENO, &aa, 1); //write character back to terminal
	    }//switch
      }//for

      if(c_flag != 1){ //if we are not compressing data
        //take the buffer we modified and directly send it to the server and log if necessary
        write(poll_list[1].fd, buffer, chars_read);
        if(l_flag == 1)
          logging_help(1, buffer, chars_read);
      }
      else{ //in the case we are compressing
        //compress first 
        char compress[BUFFER];
        int compress_size = BUFFER;
        compress_helper(buffer, &chars_read, compress, &compress_size);
        //send compressed to the server and log if necessary
        write(poll_list[1].fd, compress, compress_size);
        if(l_flag == 1)
          logging_help(1, compress , compress_size);
      }
  }//if poll_list[0].fd POLLIN

    if(((poll_list[1].revents&POLLHUP) == POLLHUP)){ 
      goto EXIT;
    }
    else if((poll_list[1].revents&POLLIN) == POLLIN){
      char buffer[BUFFER];
      char *working_buffer;
      char decompression[BUFFER];
      int working_size = 0;
      int chars_read = read(poll_list[1].fd, &buffer, BUFFER);

      if(chars_read == 0) //if socket is closed, nothing to read so exit
        goto EXIT;
      
      if(c_flag != 1){   //if we are not decompressing
        working_buffer = buffer;  //working buffer and size are the read in buffer and chars read
        working_size = chars_read;
       
        if(l_flag == 1)
          logging_help(0, buffer, chars_read);
      }
      else{ //in the case the we need to decode
        if(l_flag == 1)
          logging_help(0, buffer, chars_read);
        
        //decompress
        working_size = BUFFER;
        decompress_helper(buffer, &chars_read, decompression, &working_size);

       //set working buffer to decompressed buffer and the working size has alreay been assigned
        working_buffer = decompression;
      }
      //output the working buffer, and ma- <lf> to <cr><lf> per TA
      for(i = 0; i < working_size; i++){
        char aa = working_buffer[i];
        switch(aa){
          case '\n': //will map <lf>  to <cr><lf>
            write(STDOUT_FILENO, helper, 2);
          break;
          default:
            write(STDOUT_FILENO, &aa, 1);
        }
      }
    }    
  }
 
EXIT:
  return;
}

int main(int argc, char** argv){
  
  int port_number;    //will hold the port number
  char* log_filename; //will hold the name of the log file

  //options for the program
  static struct option long_options[] = {     
    {"port", required_argument, 0, 'p'},
    {"log", required_argument, 0, 'l'},
    {"compress", no_argument, 0, 'c'},
    {0,0,0,0}
  };

  //parse through the arguments
  char ch;
  while((ch = getopt_long(argc, argv, "", long_options, NULL)) != -1){
    switch(ch){
    case 'p':
      port_number = atoi(optarg); //set port number to number passed in
      break;
    case 'l':
      l_flag = 1;
      log_filename = optarg;      //file name arg to log_filename
      break;
    
    case 'c':
      c_flag = 1;    //info will compress
      break;

    default: //unrecognized arguments will prompt a proper usage messgae and along with exit
      fprintf(stderr, "Proper Usage: lab1b-client [--port PortNumber] [--log filename] [--compress]\n");
      exit(1);
    };
  }

  //from tutorial
  int sockfd;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  if(c_flag == 1){ //need to initialize z_streams 
    com.zalloc  = decom.zalloc = Z_NULL;
    com.zfree = decom.zfree = Z_NULL;
    com.opaque = decom.opaque = Z_NULL;
    if (deflateInit(&com, Z_DEFAULT_COMPRESSION) != Z_OK || inflateInit(&decom) != Z_OK ){
        fprintf(stderr, "Something went wrong \n");
        exit(1);
    }
  }
  if(l_flag == 1){ //incase of logging, we need ot create/open file
    fd_log_file = creat(log_filename, 0666);
    if(fd_log_file == -1){ //incase creat failed
      fprintf(stderr, "Error with log file %s.\n", log_filename);
      system_call_fail("creat()");
    }   
  }

  set_input_mode(); //set terminal modes

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0){
    fprintf(stderr, "Error in opening socket, program ending.");
    exit(1);
  }

  server = gethostbyname("localhost");
  if(server == NULL){
    fprintf(stderr, "Host of that name does not exist, program exiting");
    exit(1);
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,
	(char *)&serv_addr.sin_addr.s_addr,
	server->h_length);
  serv_addr.sin_port = htons(port_number);

  if(connect(sockfd, (struct sockaddr* )&serv_addr, sizeof(serv_addr)) < 0){
    fprintf(stderr, "Error connecting, program ending");
    exit(1);
  }

  poll_list[0].fd = STDIN_FILENO;
  poll_list[1].fd = sockfd;      //instead of input from shell, we need to take in input from the sock file descriptor
  poll_list[0].events = POLLIN | POLLHUP | POLLERR;
  poll_list[1].events = POLLIN | POLLHUP | POLLERR;

  //call client_process, this will deal with keyboard entry and entry from the server.
  client_process(poll_list);

  //close the socket that was opened
  close(sockfd);
  
  //if we are compressing, we need to end decom and com and case the fd for the log file
  if(c_flag == 1)
    end_helper();
  if(l_flag == 1)
    close(fd_log_file);
  exit(0);
}
