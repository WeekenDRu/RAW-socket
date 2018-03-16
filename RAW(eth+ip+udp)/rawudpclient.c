#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <net/if.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h> 
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>

struct UDPRAW{
	unsigned short int SP;
	unsigned short int DP;
	unsigned short int LEN;
	unsigned short int CHECKSUM;
};

struct IPRAW{
	unsigned char VARIHL;
	unsigned char DS;
	unsigned short int TL;
	unsigned short int Ident;
	unsigned short int FLAGSOFFSET;
	unsigned char TTL;
	unsigned char PROTO;
	unsigned short int CSC;
	u_int32_t IPS;
	u_int32_t IPD; 
};

struct ETHRAW{
	unsigned char DA[6];
	unsigned char SA[6];
	unsigned short Type;
};

unsigned short csum(unsigned short *ptr, int nbytes) 
{
    register long sum;
    unsigned short oddbyte;
    unsigned short answer;
    sum=0;
    while(nbytes>1) {
        sum+=*ptr++;
        nbytes-=2;
    }
    if(nbytes==1) {
        oddbyte=0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }
    sum = (sum>>16)+(sum & 0xffff);
    sum = sum + (sum>>16);
    answer=(short)~sum;
    return(answer);
}

int main(int argc, char* argv[]){

	struct sockaddr_ll out_addres;
	struct UDPRAW header_udp;
	struct IPRAW header_ip;
	struct ETHRAW header_eth;
	struct ifreq ifr;

    int sock = 0;
    int read_bytes=0;
	int send_bytes=0;
	uint8_t mac_dst[6];
	unsigned int ind_if=0; 
	unsigned short int destinationport=0;

	char* ip_csc;
	unsigned char buf[3]="Hi!";
	unsigned char buf_send[45];
	unsigned char buf_recv[60];

    char*    if_name        = 	    	argv[2];
	int      htons_ip_dst   = inet_addr(argv[3]);
	uint16_t htons_port_dst =      atoi(argv[4]);
	int 	 htons_ip_src   = inet_addr(argv[5]);

    memset(&buf_send, 0, sizeof(buf_send));
    memset(&ifr, 0, sizeof(ifr));
	memset(&header_udp, 0, sizeof(header_udp));
	memset(&header_ip, 0, sizeof(header_ip));
	memset(&header_eth, 0, sizeof(header_eth));

    size_t if_name_len=strlen(if_name);

    if (if_name_len<sizeof(ifr.ifr_name)) {
    	memcpy(ifr.ifr_name,if_name,if_name_len);
    	ifr.ifr_name[if_name_len]=0;
	} else {
    	printf("interface name is too long");
	}

	sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(sock < 0){
		perror("socket()");
		exit(1);
	}

	ind_if = if_nametoindex(if_name);
	ifr.ifr_ifindex = ind_if;

	if(ioctl(sock, SIOCGIFHWADDR, &ifr)<0){
		perror("ioctl()");
		exit(1);
	}

	if (ifr.ifr_hwaddr.sa_family!=ARPHRD_ETHER) {
    printf("not an Ethernet interface");
	}

	const unsigned char* mac=(unsigned char*)ifr.ifr_hwaddr.sa_data;
	printf("My MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
    mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

	if(6 == sscanf(argv[1], "%02x:%02x:%02x:%02x:%02x:%02x", &mac_dst[0], &mac_dst[1], &mac_dst[2], &mac_dst[3], &mac_dst[4], &mac_dst[5])){
		printf("Send to MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", mac_dst[0], mac_dst[1], mac_dst[2], mac_dst[3], mac_dst[4], mac_dst[5]);
	}
	else printf("Not correct MAC address");
	

	out_addres.sll_ifindex=ind_if;
	out_addres.sll_halen=ETH_ALEN;
	out_addres.sll_addr[0] = mac_dst[0];
	out_addres.sll_addr[1] = mac_dst[1];
	out_addres.sll_addr[2] = mac_dst[2];
	out_addres.sll_addr[3] = mac_dst[3];
	out_addres.sll_addr[4] = mac_dst[4];
	out_addres.sll_addr[5] = mac_dst[5];
	
	header_eth.DA[0] = out_addres.sll_addr[0];
	header_eth.DA[1] = out_addres.sll_addr[1];
	header_eth.DA[2] = out_addres.sll_addr[2];
	header_eth.DA[3] = out_addres.sll_addr[3];
	header_eth.DA[4] = out_addres.sll_addr[4];
	header_eth.DA[5] = out_addres.sll_addr[5];
	header_eth.SA[0] = mac[0];
	header_eth.SA[1] = mac[1];
	header_eth.SA[2] = mac[2];
	header_eth.SA[3] = mac[3];
	header_eth.SA[4] = mac[4];
	header_eth.SA[5] = mac[5];
	header_eth.Type=htons(ETH_P_IP);

	header_ip.VARIHL=0x45;
	header_ip.DS=0x00;
	header_ip.TL=htons(sizeof(header_ip)+sizeof(header_udp) + sizeof(buf));
	header_ip.Ident=htons(5432);
	header_ip.FLAGSOFFSET=0;
	header_ip.TTL=64;
	header_ip.PROTO=0x11;
	header_ip.CSC=0;
	header_ip.IPS=htons_ip_src;
	header_ip.IPD=htons_ip_dst;

	ip_csc = (char *)malloc(sizeof(header_ip));
	memcpy(ip_csc, &header_ip, sizeof(header_ip));
	header_ip.CSC = csum((unsigned short*)ip_csc,sizeof(header_ip));

	header_udp.SP=htons(63000);
	header_udp.DP=htons(htons_port_dst);
	header_udp.LEN=htons(sizeof(header_udp)+sizeof(buf));
	header_udp.CHECKSUM=0;

	memcpy((void *)buf_send, (void *)&header_eth, sizeof(header_eth));
	memcpy((void *)(buf_send + sizeof(header_eth)), (void *)&header_ip, sizeof(header_ip));
    memcpy((void *)(buf_send + sizeof(header_eth) + sizeof(header_ip)), (void *)&header_udp, sizeof(header_udp));
    memcpy((void *)(buf_send + sizeof(header_eth) + sizeof(header_ip) + sizeof(header_udp)), (void *)buf, sizeof(buf));

    send_bytes = sendto(sock, buf_send, sizeof(buf_send), 0, (struct sockaddr*)&out_addres, sizeof(out_addres));

    if(send_bytes==-1){
		perror("sendto()");
		close(sock);
		exit(1);
	}

    while(1){
    	read_bytes = recvfrom(sock, buf_recv, 1024, 0, NULL, NULL);	

    	if(read_bytes==-1){
			perror("recvfrom()");
			close(sock);
			exit(1);
		}

		destinationport=buf_recv[36]<<8|buf_recv[37];

		if(destinationport==ntohs(header_udp.SP)){
			printf("\nByte reads:= %d\n", read_bytes);
			for(int i=42; i<read_bytes; i++){
				printf("%c", buf_recv[i]);
			}
			printf("\n");
		}
   	}

   	close(sock);

   	return 0;
}