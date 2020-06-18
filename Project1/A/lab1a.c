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
#include <sys/wait.h>

#define BUFFER 256

//to remember the original attributes
struct termios saved_attributes;


//simple function that will print CR and LF to terminal
void stdout_crlf(void){
  char helper[] = {'\r', '\n'};
  write(STDOUT_FILENO, helper, 2);
}

void sig_handler(){
  exit(0);
}

//function to handle the failures of system calls 
//just pass in the name of the system call that failed
void system_call_fail(char* name)
{
  fprintf(stderr, "Program is ending, error occurred with %s.\n", name);
}

//function to all for simple non-canonical output
void terminalProcess(void)
{
  char c;
  while(1){
    if(read(STDIN_FILENO, &c, 1) == -1) //read a char
      system_call_fail("read()");
    if(c == '\r' || c == '\n'){ //if CR or Lf call stdout_crlif
      stdout_crlf();
      continue; //do not want to write repeat
    }
    if(c == 4) //escape if it is end of transmission
      break;
    if(write(STDOUT_FILENO, &c, 1) == -1) //else just output to screen
      system_call_fail("write()");
  }
}

//function to set up pipes for which process calls it
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

//taken from TA example, and cited material
void reset_input_mode(void)
{
  tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);
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

//is called to finish processing what terminal(parent process) needs to do
void parent_process_routine(struct pollfd* poll_list, int* terminal_to_shell, int* shell_to_terminal, pid_t pid)
{
  int val; //val that will return value of poll comand 
  char control_c[] = {'^', 'C', '\n'};  //c-string to print ^C
  char control_d[] = {'^', 'D', '\n'};  //c-string to print ^D

  //The while-loop below is to deal with the poll() sys call
  //taken from example cited in the README
  while(1){
    int i;
    val = poll(poll_list, 2, 0);
    char buffer[BUFFER];   //will hold the content read from either keyboard or shell
    if(val == -1)         
      system_call_fail("poll()");

    //when there is input from teh keyboard
    if((poll_list[0].revents&POLLIN) == POLLIN){
      int chars_read = read(poll_list[0].fd, &buffer, BUFFER); //read from keyboard and hold number of characters read
      for(i = 0; i < chars_read; i++){
        char aa = buffer[i];
        switch(aa)
        {
          case '\r': //if keyboard reads <cr> or <lf>
          case '\n':
            stdout_crlf(); //stdout gets <cr><lf>
            aa = '\n';
            write(terminal_to_shell[1], &aa, 1); //shell gets <lf>
            break;
          case 4:
            write(STDOUT_FILENO,control_d,3); //print ^D
            close(shell_to_terminal[0]);
            close(terminal_to_shell[1]);
            goto EXIT;
            break;
          case 3:
            write(STDOUT_FILENO,control_c,3); //print ^C
            if (kill(pid, SIGINT) == -1) //if ^C is entered, send a SIGINT
              system_call_fail("kill()");
            break;
          default: //print character to stdout and forward to shell
            write(STDOUT_FILENO, &aa, 1);       
            write(terminal_to_shell[1], &aa, 1);
        }

      }
    }

    if(((poll_list[1].revents&POLLHUP) == POLLHUP)){ //if shell is done
      goto EXIT;
    }
    else if((poll_list[1].revents&POLLIN) == POLLIN){
      int chars_read = read(poll_list[1].fd, &buffer, BUFFER);  //read in from the shell, chars_read is number chars read
      for(i = 0; i < chars_read; i++){ //go through everything that is being read
        char aa = buffer[i];
        switch(aa){
          case '\n': //if <lf> make stdout print out <cr><lf>
            stdout_crlf();
          break;
          case 4: //need to close write from terminal to shell[1]
            close(terminal_to_shell[1]);
            break;
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


int main(int argc, char*  argv []){
  //indicates if shell option is selected 
  int terminal_to_shell[2]; 
  int shell_to_terminal[2];
  int ch, sh = 0; //initall = 0, no shell
  pid_t pid;      
  signal(SIGPIPE, sig_handler);
  //no matter what we need to set up the pseudo terminal
  set_input_mode();
  char* program_name;
  //array of long options, as done in project 0. 
  //The only required option is shell, so only one argument
  static struct option long_options[] = {
    {"shell", required_argument, 0, 's'},
    {0,0,0,0}
  };

  //go through the arguments for the function.
  while((ch = getopt_long(argc, argv, "", long_options, NULL)) != -1){
    switch(ch){
    case 's': //if shell option is selected set, sh to 1
      sh = 1;
      program_name = optarg; //holds the name of the program will are trying to run
      break;

    default: //unrecognized arguments will prompt a proper usage messgae and along with exit
      fprintf(stderr, "Proper Usage: lab1a [--shell ProgramName]\n");
      exit(1);
    };
    
  }

  //if we are not going --shell option
  if(!sh){
    terminalProcess();  //go into to non-canonical read/write function
    exit(0); //exit the function properly.
  }  

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
    poll_list[0].fd = STDIN_FILENO;        //make one describe keyboard of stdin
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
    if ( waitpid(0, &status, 0) == -1)
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

  exit(0);
}
