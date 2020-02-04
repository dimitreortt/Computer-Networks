#include "header.h"

char * receive_data(int sockfd){
	static char buffer[128];

	memset(buffer, 0, sizeof(buffer));

	if(recv(sockfd, buffer, sizeof(buffer), 0) < 0){
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		exit(1);
	}

	return buffer;
}

void send_feedback(int sockfd, char * msg){
	//snprintf(buffer, sizeof(buffer), success_msg, myifaces[0].ifname);
	if(send(sockfd, msg, strlen(msg), 0) < 0){
		printf("Error! Impossible to send feedback\n");
	}
}