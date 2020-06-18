//NAME: Simran Dhaliwal
//EMAIL: sdhaliwal@ucla.edu
//ID: 905361068

#include <stdio.h>
#include <stdlib.h>
#include <mraa.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <getopt.h>
#include <string.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

//devices tools
mraa_aio_context temp_sensor;     //context for the temp_sensor


//time helper object
struct tm* loc_time;
time_t t;


//flag for C (0) of F (1)  temp_sensor
int temp_flag = 1;
int log_flag = 0; //is 1 if we are using a file
char *file_name; //file name in case we are writing to one
FILE* file_des = NULL; //pointer that will help us write to log file

//how fast samples are reported
int period = 1;

//flags that indicate if end_flag and paused_flag
int end_flag = 0;
int paused_flag = 0;

//the function below is called when the button is pressed, sets end_flag to 1 to indicate
//we want to program to end


//lab4c specific variables
int sockfd = 0;
char * host_name = NULL;
int id = 0;
int port = -1;

void test(){
	end_flag = 1;
}

void argument_testing(){
	int id_num = 0, test_id = id;
	do{
		id_num++;
		test_id /= 10;
	}while(test_id != 0);

	if (port == -1 || id_num != 9 || host_name == NULL || file_des == NULL){
		fprintf(stderr, "Invalid Arguments.\n");
		exit(1);
	}
}


//function that is called to shut down the program properly
void end_program(){
        // close the contexts for temp_sensor and button
        mraa_aio_close(temp_sensor);
    
	//get final time for the program
	time(&t);
        loc_time = localtime(&t);
	
	//Print (and log) shutdown message and exit with 0
	char end[25];
	int x = sprintf(end, "%02d:%02d:%02d SHUTDOWN\n",loc_time->tm_hour, loc_time->tm_min, loc_time->tm_sec);
	write(sockfd, end, x);
	if (log_flag)
		fprintf(file_des, "%02d:%02d:%02d SHUTDOWN\n",loc_time->tm_hour, loc_time->tm_min, loc_time->tm_sec);
	exit(0);
}

//this function will return the temp, code mainly gotten from sensor website
float get_temperature(void){
	float  val = mraa_aio_read(temp_sensor); //read the temp_sensor
	float R = (1023.0/val - 1.0) * 100000.0; //calculate R	
	float temp  = 1.0/(log(R/100000)/4275+1/298.15); //calculate temp_sensor in K
	if(temp_flag == 0)
		return temp - 273.15;
	else
		return (temp -273.15) * (9.0/5.0)  + 32;
}


//Function below just reports the time and temp_sensorerature 
void report(){
	if(paused_flag)
		return;
	else{
		time(&t);
        	loc_time = localtime(&t);
        	float temp = get_temperature();
		char report[30];
		int x = sprintf(report, "%02d:%02d:%02d %.1f\n",loc_time->tm_hour, loc_time->tm_min, loc_time->tm_sec,  temp);
 		write(sockfd, report, x);
		if(log_flag)
			fprintf(file_des, "%02d:%02d:%02d %.1f\n",loc_time->tm_hour, loc_time->tm_min, loc_time->tm_sec,  temp);
		//what makes the program wait a certain amount of time
		sleep(period);
	}
}


//deals with processing input from stdin
void process(char * str){
	int command_valid = 0; //if 1 str will be logged
	char aa[256]; //this is a buffer that will be used to copy command from str
	char peri[] = "PERIOD=";
	memset(aa, 0, 256);
	unsigned int index = 0;
	//only want upto \n
	while(index < strlen(str)){
		if(str[index] == '\n')
			break;
		//aa[index] = str[index];
		index++;
	}
	memcpy(aa, str, index);
	//log to log file if necessary
	
	if(strcmp(aa, "OFF") == 0){
		if(log_flag){
			fprintf(file_des, "%s\n", aa);
		}
		end_flag = 1;
		end_program();
	}
	else if(strcmp(aa, "SCALE=F") == 0){
		temp_flag = 1;
		command_valid = 1;
	}
	else if(strcmp(aa, "SCALE=C") == 0){
		temp_flag = 0;
		command_valid = 1;	
	}
	else if(strcmp(aa, "STOP") == 0){
		paused_flag = 1;
		command_valid = 1;
	}
	else if(strcmp(aa, "START") == 0){
		paused_flag = 0;
		command_valid = 1;
	}	
	else if(aa[0] == 'L' && aa[1] == 'O' && aa[2] == 'G'){
		//just print that this happened
		command_valid = 1;
	}
	else if (strlen(aa) > (strlen(peri))){//check for period
		unsigned int i = 0;	
		for(; i < strlen(peri); i++){
			if(aa[i] != peri[i])
				return;
		}
		int j = atoi(aa+7);
		if(j > 0)
			period = j;
		command_valid = 1;
	}

	if(log_flag){
		if(command_valid)
			fprintf(file_des, "%s\n", aa);
	}
		
}


int main(int argc, char ** argv){
	temp_sensor = mraa_aio_init(1);

	static struct option long_options[] = {
		{"scale", required_argument, 0, 's'},
		{"period", required_argument, 0, 'p'},
		{"log", required_argument, 0, 'l'},
		{"host", required_argument, 0, 'h'},
		{"id", required_argument, 0, 'i'},
		{0,0,0,0}
	};

	char * type;
	int x; //holds value of passed in period
	signed char ch; 
	while((ch = getopt_long(argc, argv ,"", long_options, NULL)) != -1){
		switch(ch){
		case 's':
		type = optarg;
		if(strcmp(type, "C") == 0)
			temp_flag = 0;
		else if(strcmp(type, "F") == 0)
			temp_flag = 1;	
		else{
			fprintf(stderr, "Error with Scale Arg. \n");
			exit(1);
		}
		break;
		case 'p':
		x = atoi(optarg);
		if(x < 1){
			fprintf(stderr, "Error with passed in period. \n");
			exit(1);
		}
		period = x;
		break;
		
		case 'l':
		log_flag = 1;
		file_name = optarg;
		file_des = fopen(file_name, "w");
		if(file_des == NULL){ //check if the system call failed
			fprintf(stderr, "Bad log file. \n");
			exit(1);
		}
		break;
		case 'h':
		host_name = optarg;
		break;
		case 'i':
		id = atoi(optarg);	
		break;
		default:
		fprintf(stderr, "Proper Usage: lab4b [--scale=[C|F]] [--period=X] [--log=file_name]");
		exit(1);
		};	
	}

	//getting port number
	port = atoi(argv[optind]);
	//checking we have all mandatory arguments
	argument_testing();

	//connecting the the server
	struct sockaddr_in serv_addr;
	struct hostent *server;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){
		fprintf(stderr,"Error opening socket.\n");
		exit(2);
	}

	server = gethostbyname(host_name);
	if (server == NULL){
		fprintf(stderr, "Error, no such host.\n");
		exit(1);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
    	serv_addr.sin_family = AF_INET;
    	bcopy((char *)server->h_addr, 
              (char *)&serv_addr.sin_addr.s_addr,
              server->h_length);
    	serv_addr.sin_port = htons(port);

	if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		fprintf(stderr, "Error Connecting.\n");
		exit(2);
	}

	//sending ID to server
	char id_to_send[15];
	int id_size = sprintf(id_to_send, "ID=%d\n", id);
	write(sockfd, id_to_send, id_size);
	
	//set up poll for input
	struct pollfd poll_list[1];
	poll_list[0].fd = sockfd;
	poll_list[0].events = POLLIN;
	char buffer[128]; //will get info from stdin read into it
	while(1){
		
		int val = poll(poll_list, 1,0);
                if(val ==  -1){
                        fprintf(stderr, "Error with Poll.\n");
                }
               
		//in the case that there is input from stdin
		if((poll_list[0].revents & POLLIN) == POLLIN){
			//FILE* sock_in = fdopen(sockfd, "r");			
			//fgets(buffer, 256, sock_in); //look in readme                      		 
			int chars_read = read(sockfd, buffer, 128);
			int start_pos = 0;
			int index = 0;
			while(index < chars_read){
				if(buffer[index] != '\n'){
					index++;
				}
				else{
					char command[50];
					memset(command, 0, 50);
					memcpy(command, &buffer[start_pos], (index+1) - start_pos);
					index++;
					start_pos = index;
					process(command);
				}
			}
                }
		

		if(end_flag){
			end_program();
		}

		report();	
		
	}



	// program should never go down here, just included this in case of something goes wrong
	mraa_aio_close(temp_sensor);
	
	exit(0);
}
