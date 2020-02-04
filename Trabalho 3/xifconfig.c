#include "header.h"

#include <netinet/tcp.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define MAX_PACKET_SIZE 65536
#define MIN_PACKET_SIZE 64

#define MAX_IFACES	64
#define MAX_IFNAME_LEN	22
#define ETH_ADDR_LEN	6
#define MAX_TABLE_SIZE 100

#define IFACESDATA 6
#define CONFIGIFACES 7
#define MTUCONFIG 8

void print_usages(char * msg){
	fprintf(stderr, "Program expects at most 3 arguments\n");
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
	char buffer[64];
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

void wait( int seconds )
{   // Pretty crossplatform, both ALL POSIX compliant systems AND Windows
    #ifdef _WIN32
        Sleep( 1000 * seconds );
    #else
        sleep( seconds );
    #endif
}

//read the table received from daemon and print it
void read_ifaces_metadata(int sockfd){
	char buffer[100000];
	int i;
	send_op_type(IFACESDATA, sockfd); 
	/*this is a trick to put the proccess to sleep on second, while
		daemon proccess writes data into the socket*/
	wait(1);
	read(sockfd, buffer, sizeof(buffer));
	printf("%s\n", buffer);
}

//print number of arguments error message
void print_error(char ** argv){
	printf("Error: Imcompatible number of arguments\n");
	printf("Usage: %s |\n", argv[0]);
	printf("\t%s <interface> <IP address> <IP Netmask> |\n", argv[0]);
	printf("\t%s <interface> mtu size\n", argv[0]);
	exit(1);
}

/*configures the interface determined in the input with ip address and 
	netmask given in the input */
void config_iface(int sockfd, char ** argv){
	char buffer[64];
	snprintf(buffer, sizeof(buffer), "%s\t%s\t%s\t", argv[1], argv[2], argv[3]);
	send_op_type(CONFIGIFACES, sockfd);
	sendm(buffer, sockfd);
	get_feedback(sockfd);
}

/*configures the mtu of the determined interface*/
void mtu_config(int sockfd, char ** argv){
	char buffer[64];
	send_op_type(MTUCONFIG, sockfd);
	snprintf(buffer, sizeof(buffer), "%s\t%s\t", argv[1], argv[2]);
	sendm(buffer, sockfd);
	get_feedback(sockfd);
}

//verify what is the command and dispatch it to the
//correct interpreter
void treat_request(int argc, char ** argv, int sockfd){
	switch(argc){
		case 1:
			read_ifaces_metadata(sockfd);
			break;
		case 3:
			mtu_config(sockfd, argv);
			break;
		case 4:
			config_iface(sockfd, argv);
			break;
		default:
			print_error(argv);
			break;		
	}
}

int main(int argc, char** argv) {
	int sockfd;

	//man 7 ip
	//man unix
	struct sockaddr_in serv_addr;

	if(argc > 4) {
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
