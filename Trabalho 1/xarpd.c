#include "header.h"
/* */
struct iface my_ifaces[MAX_IFACES];
/* */

/* Find first empty entry of the arp cache*/
short find_entry(struct item ** tab){
	short i = 0;
	for(i = 0; i < MAX_TABLE_SIZE && tab[i] != NULL; i++){}
	return (i < 100) ? i : -1;
}
/* */
// Print an Ethernet address
void print_eth_address(char *s, unsigned char *eth_addr)
{
	printf("%s %02X:%02X:%02X:%02X:%02X:%02X", s,
	       eth_addr[0], eth_addr[1], eth_addr[2],
	       eth_addr[3], eth_addr[4], eth_addr[5]);
}
/* */
// Bind a socket to an interface
int bind_iface_name(int fd, char *iface_name)
{
	return setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, iface_name, strlen(iface_name));
}
/* */

/*get interface information analysing the devices*/
void get_iface_info(int sockfd, char *ifname, struct iface *ifn)
{
	struct ifreq s;
	struct sockaddr_in * sin;
	unsigned int ip;

	strcpy(ifn->ifname, ifname);
	strncpy(s.ifr_name, ifname, IFNAMSIZ);

	get_hardware_address(sockfd, ifn, s);
	get_ip_addr(sockfd, ifn, s);
	get_mtu(sockfd, ifn, s);
	get_netmask(sockfd, ifn, s);
	
}
// Print the expected command line for the program
void print_usage()
{
	printf("\xa arpd <interface> [<interfaces>]\n");
	exit(1);
}
/* */
// Break this function to implement the ARP functionalities.
void doProcess(unsigned char* packet, int len, char * ifacename) {
	if(!len || len < MIN_PACKET_SIZE)
		return;

	struct ether_hdr* eth = (struct ether_hdr*) packet;

	if(htons(0x0806) == eth->ether_type) {
		// ARP

		//functionalities were almost not implemented at all
		struct arp_hdr* arp_hr = (struct arp_hdr*) (packet + sizeof(struct ether_hdr));
		//resolve_ar(arp_hr);
		//...
	}
	// Ignore if it is not an ARP packet
}
/* */
// This function should be one thread for each interface.
/*void read_iface(struct iface *ifn)*/
void *read_iface(void *vifn)
{
	struct iface *ifn = (struct iface*)vifn;

	socklen_t	saddr_len;
	struct sockaddr	saddr;
	unsigned char	*packet_buffer;
	int		n;
	
	saddr_len = sizeof(saddr);	
	packet_buffer = malloc(MAX_PACKET_SIZE);
	if (!packet_buffer) {
		printf("\nCould not allocate a packet buffer\n");		
		exit(1);
	}
	
	while(1) {
		n = recvfrom(ifn->sockfd, packet_buffer, MAX_PACKET_SIZE, 0, &saddr, &saddr_len);
		if(n < 0) {
			fprintf(stderr, "ERROR: %s\n", strerror(errno));
			exit(1);
		}
		sem_wait(&sem);
		doProcess(packet_buffer, n, ifn->ifname);
		sem_post(&sem);
	}
}
/*wait for determined seconds*/
void wait(int seconds)
{   // Pretty crossplatform, both ALL POSIX compliant systems AND Windows
    #ifdef _WIN32
        Sleep(1000 * seconds);
    #else
        sleep(seconds);
    #endif
}
/*every second decrement ttl of each of the ARP cache entries*/
void* decrement_ttl(void * uselessinput){
	while(1){
		//printf("%d\n", tab[102]->ttl);
		wait(1);
		struct item * arpp;
		int i = 0;
		sem_wait(&sem);
		for(i = 0; i < MAX_TABLE_SIZE; i++){
			arpp = tab[i];
			if(arpp){
				arpp->ttl -= 1;
				if(!arpp->ttl){
					tab[i] = NULL;
				}
			}
		}
		sem_post(&sem);
	}
}
/* */
// main function
int main(int argc, char** argv) {
	int			i, sockfd;
	pthread_t 	tid[argc - 1];
	pthread_t 	t_com;
	pthread_t 	timer;

	dynamicTtl = 60;

	if (argc < 2)
		print_usage();
	
	/*initialize semaphore*/	
	sem_init(&sem, 0, 1);

	for (i = 1; i < argc; i++) {
		sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));  
		if(sockfd < 0) {
			fprintf(stderr, "ERROR: %s\n", strerror(errno));
			exit(1);
		}
		
		if (bind_iface_name(sockfd, argv[i]) < 0) {
			perror("Server-setsockopt() error for SO_BINDTODEVICE");
			printf("%s\n", strerror(errno));
			close(sockfd);
			exit(1);
		}
		get_iface_info(sockfd, argv[i], &my_ifaces[i-1]);
		//printf("netmask was: %s\n", get_netmask(my_ifaces[i]));
	}
	memset(my_ifaces[i].ifname, 0, sizeof(my_ifaces[i].ifname));

	for (i = 0; i < argc-1; i++) {
		print_eth_address(my_ifaces[i].ifname, my_ifaces[i].mac_addr);
		// Create one thread for each interface. Each thread should run the function read_iface.
		if(pthread_create(&tid[i], NULL, read_iface, (void*)&my_ifaces[i])){
			fprintf(stderr, "Error creating thread\n");
			return 1;
		}
		printf("\n");		
	}

	//Create one thread to deal with requests from programs
	if(pthread_create(&t_com, NULL, communication, (void *)my_ifaces)){
		fprintf(stderr, "Error creating thread\n");
		return 1;
	}

	/*create on thread to decrement ttl in every entry of the arp_cache
		every second*/
	if(pthread_create(&timer, NULL, decrement_ttl, (void *)&i)){
		fprintf(stderr, "Error creating thread\n");
		return 1;
	}

	struct item it;

	for(i = 0; i < MAX_TABLE_SIZE; i++){
		tab[i] = NULL;
	}

	void ** arg;
	pthread_join(t_com, arg);
	return 0;
}
/* */
