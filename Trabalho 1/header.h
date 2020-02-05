#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define MAX_PACKET_SIZE 65536
#define MIN_PACKET_SIZE 20

#define MAX_IFACES	64
#define MAX_IFNAME_LEN	22
#define ETH_ADDR_LEN	6
#define MAX_TABLE_SIZE 128
#define LISTEN_ENQ 5

#define SHOW 1	
#define RES 2
#define ADD 3
#define DEL 4
#define TTL 5
#define IFACESDATA 6
#define CONFIGIFACES 7
#define MTUCONFIG 8

struct iface {
	int		sockfd;
	int		ttl;
	int		mtu;
	char		ifname[MAX_IFNAME_LEN];
	unsigned char	mac_addr[6];
	unsigned int	netmask_addr;
	unsigned int	ip_addr;
	unsigned int	rx_pkts;
	unsigned int	rx_bytes;
	unsigned int	tx_pkts;
	unsigned int	tx_bytes;
};

struct ether_hdr {
	unsigned char	ether_dhost[6];	// Destination address
	unsigned char	ether_shost[6];	// Source address
	unsigned short	ether_type;	// Type of the payload
};

struct ip_hdr {

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	unsigned char	ip_ihl:4,
			ip_v:4;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	unsigned char	ip_ihl:4,
			ip_v:4;
#endif	
	unsigned char	ip_tos;		// Type of service
	unsigned short	ip_len;		// Datagram Length
	unsigned short	ip_id;		// Datagram identifier
	unsigned short	ip_offset;	// Fragment offset
	unsigned char	ip_ttl;		// Time To Live
	unsigned char	ip_proto;	// Protocol
	unsigned short	ip_csum;	// Header checksum
	unsigned int	ip_src;		// Source IP address
	unsigned int	ip_dst;		// Destination IP address
};

// Read RFC 826 to define the ARP structure
struct arp_hdr{
	/* all the fields taken from include/linux/if_arp.h */
   unsigned short int 	ar_hrd;		/* Format of hardware address.  */
   unsigned short int 	ar_pro;		/* Format of protocol address.  */
   unsigned char 		ar_hln;		/* Length of hardware address.  */
   unsigned char 		ar_pln;		/* Length of protocol address.  */
   unsigned short int 	ar_op;		/* ARP opcode (command).  */
   unsigned char       	ar_sha[ETH_ADDR_LEN]; 	/*Sender Hardware Address*/
   unsigned char       	ar_spa[4];				/*Sender Protocol Protocol*/
   unsigned char       	ar_tha[ETH_ADDR_LEN];	/*Target Hardware Address*/
   unsigned char       	ar_tpa[4];				/*Target Protocol Address*/
};      

/* Structure holding the data of one entry of the ARP cache*/
struct item{
	unsigned short 	entry;
	struct in_addr 	ip_addr;
	unsigned char 	MAC_addr[6];
	unsigned short 	ttl;
};

/* Table holding the ARP cache*/
struct Table{
	struct item* tab[MAX_TABLE_SIZE];
	ushort tabsize;
};

/* Global Variables */

/* Semaphore*/
sem_t sem;

/* Arp Cache Table*/
struct item* tab[MAX_TABLE_SIZE];


//##########################################################
//xarpd.c
//operations used by xarpd.c
void print_eth_address(char *s, unsigned char *eth_addr);
void get_hardware_address(int sockfd, struct iface *ifn, struct ifreq s);
void get_ip_addr(int sockfd, struct iface *ifn, struct ifreq s);
void get_mtu(int sockfd, struct iface *ifn, struct ifreq s);
void get_netmask(int sockfd, struct iface *ifn, struct ifreq s);
void* communication(void* input);/* Structure holding the data of one entry of the ARP cache*/
void resolve_ar(struct arp_hdr * ar_hdr);
void resolve_arp(char * buffer, int sockets, struct iface * my_ifaces);
short find_entry(struct item ** tab);



//##########################################################
//xarp_ops.c
//operations to respond to xarp.c requests
struct item * create_item(int entry, int ttl, char * eth, char * ip);
void sendTable(int newsockfd);
void delete_item(struct item ** tab, char * buffer, int newsockfd);
void add_item(struct item ** tab, char * addresses, int newsockfd);
void update_ttl(int sockfd, char * buffer);
char* eth_to_str(unsigned char * eth_addr);
void str_to_eth(char * macStr, char * mac);
struct item* next_entry(struct item ** tab);
void str_to_eth(char * macStr, char * mac);


//###########################################################
//xif_ops.c
//operations to respond to xifconfig.c requests
void send_interfaces_metadata(int newsockfd, struct iface * myifaces);
void config_iface(int newsockfd, struct iface * myifaces);
void mtu_config(int sockfd, struct iface * myifaces);
char * receive_data(int sockfd);
void send_feedback(int sockfd, char * msg);


//###########################################################
//io_control.c
//operations to deal with devices settings
void set_ip_addr(struct iface * ifc, char * new_ip_addr);
void set_netmask(struct iface * ifc, char *newmask);
void set_mtu(struct iface * ifc, int new_mtu);


/*dynamic ttl to be given to arp cache entries found 
	dynamically*/
unsigned short dynamicTtl;
