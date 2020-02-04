#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
/* */
void print_usages(char * msg){
	fprintf(stderr, "Usage: %s <command> {<attributes>}\n", msg);
		exit(1);
}
//send already formatted argument(s) to daemon
void sendm(char * arg, int sockfd){
	//man send
	if(send(sockfd, arg, strlen(arg), 0) < 0) {
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		exit(1);
	}
}
//receive feedback messages from daemon, such as success or errors
//and print it
void get_feedback(int sockfd){
	char buffer[256];
	//man recv
	if(recv(sockfd, buffer, sizeof(buffer), 0) < 0) {
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		exit(1);
	}
	printf("\n%s\n\n", strtok(buffer, "\t"));
}

//initializes dialogue between xarp and daemon
//sending the op number according to specification
void send_op_type(int n, int sockfd){
	int tmp = htonl(n);
	write(sockfd, &tmp, sizeof(tmp));
}

//wait for determined number of seconds
void wait( int seconds )
{   // Pretty crossplatform, both ALL POSIX compliant systems AND Windows
    #ifdef _WIN32
        Sleep( 1000 * seconds );
    #else
        sleep( seconds );
    #endif
}

//read the table received from daemon and print it
void readTable(int sockfd){
	char buffer[16384];
	int i;

	/*this is a trick to put the proccess to sleep for 1 second, while
		daemon proccess writes data into the socket*/
	wait(1);

	/*read all table content already formatted*/
	read(sockfd, buffer, sizeof(buffer));

	/*display table*/
	printf("%s\n", buffer);
}

//treat 'show' command accordingly
void treat_show(int sockfd){
	send_op_type(1, sockfd);
	readTable(sockfd);
}

//treat 'del' command accordingly
void treat_del(int argc, int sockfd, char ** argv){
	if(argc < 3){
		printf("Error!! Too few arguments for 'Del' command\n");
		printf("Usage: %s <ip address>\n", argv[0]);
		exit(1);
	}	
	send_op_type(4, sockfd);
	//send ip address to be deleted
	sendm(argv[2], sockfd);
	//read and print feedback [error|success]
	get_feedback(sockfd);
}

//treat 'add' command accordingly
void treat_add(int argc, int sockfd, char ** argv){
	char buffer[64];
	if(argc < 5){
		printf("Error!! Too few arguments for 'Add' command\n");
		printf("Usage: %s <ip address> <MAC address> <ttl>\n", argv[0]);
		exit(1);
	}
	
	send_op_type(3, sockfd);
	//format arguments to be sent to daemon
	snprintf(buffer, sizeof(buffer), "%s\t%s\t%s\t", argv[2], argv[3], argv[4]);
	//send the 3 arguments that will be interpreted by the daemon
	sendm(buffer, sockfd);

	//read and print feedback [error|success]
	get_feedback(sockfd);
}

//treat 'ttl' command accordingly
void treat_ttl(int argc, int sockfd, char ** argv){
	if(argc < 3){
		printf("Error!! Too few arguments for 'ttl' command\n");
		printf("Usage: %s <ttl>\n", argv[0]);
		exit(1);
	}
	send_op_type(5, sockfd);
	sendm(argv[2], sockfd);

	//read and print feedback [error|success]
	get_feedback(sockfd);
}

void treat_res(int argc, int sockfd, char ** argv){
	if(argc < 3){
		printf("Error!! Too few arguments for 'res' command\n");
		printf("Usage: %s <ip address>\n", argv[0]);
		exit(1);
	}
	send_op_type(2, sockfd);
	sendm(argv[2], sockfd);

	//read and print feedback [error|success]
	get_feedback(sockfd);
}

//verify what is the command and dispatch it to the
//correct interpreter
void treat_request(int argc, char ** argv, int sockfd){
	if(!strcmp(argv[1], "show")){
		treat_show(sockfd);
	}
	else if(!strcmp(argv[1], "res")){
		treat_res(argc, sockfd, argv);
	}
	else if(!strcmp(argv[1], "del")){
		treat_del(argc, sockfd, argv);
	}
	else if(!strcmp(argv[1], "add")){
		treat_add(argc, sockfd, argv);
	}
	else if(!strcmp(argv[1], "ttl")){
		treat_ttl(argc, sockfd, argv);
	}
}

int main(int argc, char** argv) {
	int sockfd;

	//man 7 ip
	//man unix
	struct sockaddr_in serv_addr;

	if(argc < 2 || argc > 5) {
		print_usages(argv[0]);
	}

	//man socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
  
	if(sockfd < 0) {
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		exit(1);
	}
  
	memset((char*) &serv_addr, 0, sizeof(serv_addr));
  
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(4950);
  
	//man connect
	if(connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		exit(1);
	}

	//treat accordingly each of the commands
	treat_request(argc, argv, sockfd);
	
	close(sockfd);

	return 0;
}
