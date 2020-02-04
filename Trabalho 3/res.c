#include "header.h"

/*check arp cache entries to discover if mac address is stored in the arp cache
	if so, return mac correspondent*/
unsigned char * find_mac(struct in_addr ip, struct iface * my_ifaces){
	struct item * arpp = next_entry(tab);
	while(arpp){
		if(arpp->ip_addr.s_addr == ip.s_addr){
			return arpp->MAC_addr;
		}
		arpp = next_entry(tab);
	}
	return NULL;
}

/*resolve ip to mac searching mac on the arp table cache*/
void resolve_arp(char * buffer, int communication_sockfd, struct iface * my_ifaces){
	
	struct in_addr ip;
	inet_aton(buffer, &ip);
	
	unsigned char * Mac;

	Mac = find_mac(ip, my_ifaces);

	if(Mac){
		send_feedback(communication_sockfd, eth_to_str(Mac));
	}
	else{
		//in this section we should generate an arp request header, send it to the network
		//and wait for response, then add to the table. But we could not manage to do that in time

		//drop packet. Entry not found, do nothing
		//printf("No correspondent MAC found for IP given\n");
		send_feedback(communication_sockfd, "This entry is not in the ARP cache, and it was not"
			" appropriatedely added. Functionality not implemented!\t");
	}
}

//this is only an auxiliar method to help visualize info from the received packets
void resolve_ar(struct arp_hdr * ar_hdr){
	char sip[16];
	char tip[16];
	char sha[18];
	char tha[18];

	sprintf(sip, "%d.%d.%d.%d", ar_hdr->ar_spa[0], ar_hdr->ar_spa[1], ar_hdr->ar_spa[2], ar_hdr->ar_spa[3]);
	sprintf(tip, "%d.%d.%d.%d", ar_hdr->ar_tpa[0], ar_hdr->ar_tpa[1], ar_hdr->ar_tpa[2], ar_hdr->ar_tpa[3]);

	strcpy(sha, eth_to_str(ar_hdr->ar_sha));
	strcpy(tha, eth_to_str(ar_hdr->ar_tha));

	printf("senderIP: %s, senderMAC: %s\ntargetIP: %s, targetMAC: %s\noptype: %hu\n", sip, sha,
				tip, tha, ntohs(ar_hdr->ar_op));
}