#include "header.h"

void get_hardware_address(int sockfd, struct iface *ifn, struct ifreq s)
{
    if (0 == ioctl(sockfd, SIOCGIFHWADDR, &s)) {
        memcpy(ifn->mac_addr, s.ifr_addr.sa_data, ETH_ADDR_LEN);
        ifn->sockfd = sockfd;       
    } else {
        perror("Error getting MAC address");
        exit(1);
    }
}   

void get_netmask(int sockfd, struct iface *ifn, struct ifreq s){
    struct sockaddr_in * sin;
    unsigned int netmask;

    strncpy( s.ifr_name, ifn->ifname, IFNAMSIZ-1 );
    
    if (0 == ioctl(sockfd, SIOCGIFNETMASK, &s)) {
        sin = (struct sockaddr_in*)&s.ifr_addr;
        netmask = (unsigned int)sin->sin_addr.s_addr;
        ifn->netmask_addr = netmask;
    } else {
        perror("Error getting NETMASK address");
        exit(1);
    }
}

void set_netmask(struct iface * ifc, char *newmask)
{
    struct ifreq ifr;
    struct sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;
    memset(&ifr, 0, sizeof(ifr));
    sin->sin_family = AF_INET;
    if (!inet_pton(AF_INET, newmask, &sin->sin_addr))
    {
        printf("failed to convert netmask\n");
        exit(0);
    }
    strncpy(ifr.ifr_name, ifc->ifname, IFNAMSIZ-1);
    if (ioctl(ifc->sockfd,SIOCSIFNETMASK,&ifr) == -1)
    {
        printf("could not read interface %s\n", ifc->ifname);
        exit(0);
    }
    else{
        inet_pton(AF_INET, newmask, &ifc->netmask_addr);
        //printf("Successfully updated netmask\n");
    }
}

void get_ip_addr(int sockfd, struct iface *ifn, struct ifreq s){

    struct sockaddr_in * sin;
    unsigned int ip;

    if (0 == ioctl(sockfd, SIOCGIFADDR, &s)) {
        sin = (struct sockaddr_in*)&s.ifr_addr;
        ip = (unsigned int)sin->sin_addr.s_addr;
        ifn->ip_addr = ip;
    } else {
        perror("Error getting IP address");
        exit(1);
    }
}

void set_ip_addr(struct iface * ifc, char * newAddr){
    struct ifreq ifr;
    int res;

    ifr.ifr_addr.sa_family = AF_INET;
    struct sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;
    if(!inet_pton(AF_INET, newAddr, &sin->sin_addr))
    {
        printf("failed to convert ip address\n");
        exit(0);
    }
    strncpy(ifr.ifr_name, ifc->ifname, IFNAMSIZ);
    
    if(ioctl(ifc->sockfd, SIOCSIFADDR, &ifr) < 0){
        printf("Error!! Could not update interface IP address\n");
        exit(0);
    }
    else{
        ifc->ip_addr = (unsigned int)sin->sin_addr.s_addr;
        //printf("Successfully updated IP address\n");
    }
}

void get_mtu(int sockfd, struct iface *ifn, struct ifreq s){
    if (ioctl(ifn->sockfd, SIOCGIFMTU, &s) == 0){
        ifn->mtu = s.ifr_mtu;       
    }
    else{
        perror("Error getting MPU");
        exit(1);
    }
}

void set_mtu(struct iface * ifc, int new_mtu){
    struct ifreq ifr;

    ifr.ifr_mtu = new_mtu;
    strncpy(ifr.ifr_name, ifc->ifname, IFNAMSIZ);
    
    if(ioctl(ifc->sockfd, SIOCSIFMTU, &ifr) < 0){
        printf("Error!! Could not update interface MTU\n");
        exit(0);
    }
    else{
        ifc->mtu = ifr.ifr_mtu;
        //printf("Successfully updated MTU\n");
    }
}
