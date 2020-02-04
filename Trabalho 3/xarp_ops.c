/*Here are implementations of functions that deal with the data exchange
    between the xifconfig.c program and the daemon*/

#include "header.h"

/*receives a mac address in string format(17 bytes) and fulfils variable
    'mac' with corresponding 6 byte mac address*/
void str_to_eth(char * macStr, char * mac){
    int iMac[6];
    int i;
    sscanf(macStr, "%x:%x:%x:%x:%x:%x", &iMac[0], &iMac[1], &iMac[2], &iMac[3], &iMac[4], &iMac[5]);
    for(i=0;i<6;i++)
        mac[i] = (unsigned char)iMac[i];
}

/*create a new arp_table entry and inits it accordingly
    then returns the object or NULL in error case*/
struct item * create_item(int entry, int ttl, char * eth, char * ip){
    struct item * new = (struct item *)malloc(sizeof(struct item));
    if(new){
        inet_aton(ip, &new->ip_addr);
        str_to_eth(eth, new->MAC_addr);
        new->ttl = ttl;
        new->entry = entry;
        return new;
    }
    else{
        return NULL;
    }
}

/*add_item interprets char byte sequence from 'data' and dispatch
    it accordingly to each field of an arp_table entry struct*/
/*it also sends a message with the feedback*/
void add_item(struct item ** tab, char * data, int newsockfd){
    /*'entry' gets the first available entry in the table*/
    int entry = find_entry(tab);
    int ttl;
    char * entryError = "Error!! There is no available entry in the ARP_Table!\t";
    char * createError = "Error!! Unsuccessfull attempt to create new entry!\t";
    char * success = "Entry successfully added to the ARP_Table!\t";

    char ip[16], ether[32];
    if(entry == -1){
        write(newsockfd, entryError, strlen(entryError));
    }
    else{
        strcpy(ip, strtok(data, "\t"));
        strcpy(ether, strtok(NULL, "\t"));
        ttl = atoi(strtok(NULL, "\t"));
        tab[entry] = create_item(entry, ttl, ether, ip);
        if(tab[entry]){
            write(newsockfd, success, strlen(success));
        }
        else{
            write(newsockfd, createError, strlen(createError));
        }        
    }
}

/*this function searches for the specified IP in the table
    in success case, it deletes this entry. In fail case, it reports to
        the caller*/
void delete_item(struct item ** tab, char * ip, int newsockfd){
    int i = 0;
    struct in_addr inp;
    inet_aton(ip, (struct in_addr*)&inp.s_addr);

    char * erro = "Error!! This entry is not in the ARP_Table!\t";
    char * success = "Entry successfully deleted!\t";
    
    while(i < MAX_TABLE_SIZE){
        if(tab[i] != NULL && tab[i]->ip_addr.s_addr == inp.s_addr){
            break;
        }
        else{
            i++;
        }
    }
    if(i < MAX_TABLE_SIZE){
        //free(tab[i]);
        tab[i] = NULL;
        write(newsockfd, success, strlen(success));        
    }
    else{   
        write(newsockfd, erro, strlen(erro));
    }
}

/*updates the default dynamic ttl with the value entered*/
void update_ttl(int sockfd, char * buffer){
    dynamicTtl = atoi(buffer);
    sprintf(buffer, "Successfully updated Dinamic TTL\t");
    send(sockfd, buffer, strlen(buffer), 0);
}

/*this method implements the translation of a 6byte mac adress to
    string format mac address, and returns the result*/
char* eth_to_str(unsigned char * eth_addr){
    static unsigned char eth[18];

    memset(eth, 0, sizeof(eth));

    snprintf(eth, sizeof(eth), "%02X:%02X:%02X:%02X:%02X:%02X",
         (unsigned char)eth_addr[0], (unsigned char)eth_addr[1], (unsigned char)eth_addr[2],
          (unsigned char)eth_addr[3], (unsigned char)eth_addr[4], (unsigned char)eth_addr[5]);

    //printf("%s\n", eth);
    return eth; 
}

/*this is an auxiliar function implemented to iterate over the arp_cache table
    returning each of its entries*/
struct item* next_entry(struct item* tab[]){
    static struct item* next;
    static int j = 0;
    next = tab[j++];
    while(next == NULL){
        if(j == MAX_TABLE_SIZE){
            j = 0;
            return NULL;            
        }
        else{
            next = tab[j++];
        }
    }
    return next;
}

/*this function implements the duty of sending data to the auxiliar program
    containing the arp_cache content*/
void sendTable(int newsockfd){

    FILE *fp = fdopen(newsockfd, "w");
    struct item *arpp;

    fprintf(fp,"Entrada\tEndereço IP\tEndereço Ethernet\tTTL\n");
    arpp = next_entry(tab);
    while(arpp){
        fprintf(fp, "%d\t%-15s\t%s\t%d\t\n", arpp->entry, inet_ntoa(arpp->ip_addr),eth_to_str(arpp->MAC_addr), arpp->ttl);
        arpp = next_entry(tab);
    }
    fclose(fp);
}