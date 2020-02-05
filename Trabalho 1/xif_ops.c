/*Here are implementations of functions that deal with the data exchange
	between the xifconfig.c program and the daemon*/
#include "header.h"

/*write interfaces metadata into socket file descriptor*/
void send_interfaces_metadata(int newsockfd, struct iface * myifaces){
	
	struct iface *ifp = myifaces; 
	FILE * fp = fdopen(newsockfd, "w");

	char netmask[16], ip[16];

	while(strcmp(ifp->ifname, "\0")){

		struct in_addr ip_addr, netmask_addr;
		ip_addr.s_addr = ifp->ip_addr;
		netmask_addr.s_addr = ifp->netmask_addr;

		strcpy(netmask,inet_ntoa(netmask_addr));
		strcpy(ip, inet_ntoa(ip_addr));
	
		fprintf(fp, "%s\tEndereco de HW %s\n"
					"\tinet end.:%-15sMasc:%s\n"
					"\tUP MTU:%d\n"
					"\n", 
					ifp->ifname, eth_to_str(ifp->mac_addr), ip, netmask, 
					ifp->mtu);
		ifp++;
	}

    fclose(fp);
}

/*return interface with name 'ifname'*/
struct iface * find_interface(char * ifname, struct iface * myifaces){
	struct iface *ifp = myifaces;
	while(strcmp(ifp->ifname, "\0")){
		if(!strcmp(ifp->ifname, ifname)){
			return ifp;
		}
		ifp++;
	}
	return NULL;
}

/*receive command telling which iface to configure, and the data necessary for that
	then sets it accordingly*/
void config_iface(int newsockfd, struct iface * myifaces){
	char * data;
	char buffer[64];

	data = receive_data(newsockfd);

	char * ifname = strtok(data, "\t");
	char * new_ip_addr = strtok(NULL, "\t");
	char * new_mask = strtok(NULL, "\t");

	//look for interface with name ifname
	struct iface *ifp = find_interface(ifname, myifaces);

	if(ifp){		
		set_netmask(ifp, new_mask);
		set_ip_addr(ifp, new_ip_addr);
		
		snprintf(buffer, sizeof(buffer), "Successfully updated IP Address and Netmask from interface %s\t", myifaces[0].ifname);
		send_feedback(newsockfd, buffer);
	}
	else{
		send_feedback(newsockfd, "Sorry! Daemon is not listening to an interface with this name.\n");
	}

}

/*receive command telling which iface to configure, and the new mtu,
	then sets it accordingly*/
void mtu_config(int sockfd, struct iface * myifaces){
	char buffer[64];
	if(recv(sockfd, buffer, sizeof(buffer), 0) < 0){
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		exit(1);
	}
	char * ifname = strtok(buffer, "\t");
	int new_mtu = atoi(strtok(NULL, "\t"));

	//look for interface with name ifname
	struct iface *ifp = find_interface(ifname, myifaces);

	if(ifp){
		set_mtu(ifp, new_mtu);

		snprintf(buffer, sizeof(buffer), "Successfully updated MTU from interface %s\t", myifaces[0].ifname);
		send_feedback(sockfd, buffer);
	}
	else{
		send_feedback(sockfd, "Sorry! Daemon is not listening to an interface with this name.\n");
	}
}

