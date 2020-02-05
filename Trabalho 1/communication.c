//This is the part of the code that will deal with the communication between
//daemon arp and secondary auxiliar programs. It listens for connection
//requirements and reponds to it accordingly
#include "header.h"
/*Note: look for the implementation of each function not implemented here
    at the xarp_ops.c, xif_ops.c files*/

#define TRUE 1 
#define FALSE 0 
#define PORT 4950
    
struct request_hdr{
    unsigned char   command;
    unsigned short  ttl;
    unsigned char   ether_addr[6];
    struct in_addr  ip_addr;
};

void * communication(void * input) 
{ 
    struct iface * my_ifaces = (struct iface*) input;
    int opt = TRUE; 
    int master_socket, addrlen, newsockfd, i; 
    int clilen;
    struct sockaddr_in serv_address, cli_addr; 

    char buffer[1025]; //data buffer of 1K        
   
    //create a master socket 
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    }
    //set master socket to allow multiple connections , 
    //this is just a good habit, it will work without this 
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    
    //type of socket created 
    serv_address.sin_family = AF_INET; 
    serv_address.sin_addr.s_addr = INADDR_ANY; 
    serv_address.sin_port = htons( PORT ); 

    //bind the socket to localhost port 8888 
    if (bind(master_socket, (struct sockaddr *)&serv_address, sizeof(serv_address))<0) { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    //try to specify maximum of 3 pending connections for the master socket 
    if (listen(master_socket, 3) < 0) { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 

    while(1){        
        //accept the incoming connection 
        addrlen = sizeof(serv_address); 
        //puts("Waiting for connections ..."); 
            
        clilen = sizeof(cli_addr);
        memset(&cli_addr, 0, clilen);
        //man accept
        newsockfd = accept(master_socket, (struct sockaddr*) &cli_addr, (unsigned int*) &clilen);
        if(newsockfd < 0) {
            fprintf(stderr, "ERROR: %s\n", strerror(errno));
            exit(1);
        }
        //inform user of socket number - used in send and receive commands 
        //printf("\nNew connection , socket fd is %d , ip is : %s , port : %d\n" , newsockfd ,
          //  inet_ntoa(cli_addr.sin_addr) , ntohs(cli_addr.sin_port));

        //define operation to be done
        int tmp, operation;
        read(newsockfd, &tmp, sizeof(tmp));
        operation = ntohl(tmp);

        memset(buffer, 0, sizeof(buffer));

        //for each case, deal with it accordingly
        switch(operation){
            case SHOW:
                sendTable(newsockfd);
                break;
            case ADD:
            //receive <ip address> <mac> <ttl> and dispatch it to treatment
                recv(newsockfd, buffer, sizeof(buffer), 0);
                add_item(tab, buffer, newsockfd);
                break;
            case DEL:
            //receive <ip address> and dispatch it to treatment
                recv(newsockfd, buffer, sizeof(buffer), 0);
                delete_item(tab, buffer, newsockfd);
                break;
            case RES:
            //receive <ip address> and dispatch it to treatment
                recv(newsockfd, buffer, sizeof(buffer), 0);
                resolve_arp(buffer, newsockfd, my_ifaces);
            case TTL:
            //receive <ttl> and treat it
                recv(newsockfd, buffer, sizeof(buffer), 0);
                update_ttl(newsockfd, buffer);
                break;
            //
            case IFACESDATA:
                send_interfaces_metadata(newsockfd, my_ifaces);
                break;
            //    
            case CONFIGIFACES:
                config_iface(newsockfd, my_ifaces);
                break;
            //        
            case MTUCONFIG:
                mtu_config(newsockfd, my_ifaces);
                break;
        }
    }
    return 0; 
}
