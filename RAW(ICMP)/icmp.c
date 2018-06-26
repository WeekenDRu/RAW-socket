#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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

struct ICMPHDR{
		unsigned char Type;
		unsigned char Code;
		unsigned short CheckSum;
		unsigned short Ident;
		unsigned short Seqf;
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

	struct sockaddr_in in_addres;
	struct IPRAW header_ip;
	struct ICMPHDR header_icmp;
	struct IPRAW *p_header_ip;
	struct ICMPHDR *p_header_icmp;
	struct in_addr C;

	memset(&in_addres, 0, sizeof(in_addres));
	memset(&header_ip, 0, sizeof(header_ip));
	memset(&header_icmp, 0, sizeof(header_icmp));

    int sock = 0;
	int addr_dst = inet_addr(argv[1]);
	int addr_src = inet_addr(argv[2]);
	int read_bytes=0;
	int send_bytes=0;
	int one = 1;
	const int *val = &one;

	char buf_send[28];
	char buf_recv[62];
	char* icmp_csc;
	char* ip_csc;
	char* b;

	sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(sock < 0){
		perror("socket()");
		exit(1);
	}

	if(setsockopt(sock, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0){
		perror("setsockopt() error");
		exit(-1);
	}

	in_addres.sin_family = AF_INET;
	in_addres.sin_addr.s_addr = addr_dst;

	header_ip.VARIHL=0x45;
	header_ip.DS=0x00;
	header_ip.TL = sizeof(header_ip) + sizeof(header_icmp);
	header_ip.Ident=0;
	header_ip.FLAGSOFFSET=0;
	header_ip.TTL=64;
	header_ip.PROTO=IPPROTO_ICMP;
	header_ip.CSC=0;
	header_ip.IPS=addr_src;
	header_ip.IPD=addr_dst;

	header_icmp.Type = 8;
	header_icmp.Code = 0;

	icmp_csc = (char*)malloc(sizeof(header_icmp));
	memcpy(icmp_csc, &header_icmp, sizeof(header_icmp));

	ip_csc = (char*)malloc(sizeof(header_ip));
	memcpy(ip_csc, &header_ip, sizeof(header_ip));

	header_icmp.CheckSum = csum((unsigned short*)icmp_csc,sizeof(header_icmp));
	header_ip.CSC = csum((unsigned short*)ip_csc, sizeof(header_ip));

	memcpy((void *)buf_send, (void *)&header_ip, sizeof(header_ip));
    memcpy((void *)(buf_send + sizeof(header_ip)), (void *)&header_icmp, sizeof(header_icmp));
    

    send_bytes = sendto(sock, buf_send, sizeof(buf_send), 0, (struct sockaddr*)&in_addres, sizeof(in_addres));

    if(send_bytes==-1){
		perror("sendto()");
		close(sock);
		exit(1);
	}

    while(1){
    	read_bytes = recvfrom(sock, buf_recv, 62, 0, NULL, NULL);	

 		p_header_ip = (struct IPRAW*)(buf_recv);
 		p_header_icmp = (struct ICMPHDR*)(buf_recv+sizeof(header_ip));

 		printf("\nType ICMP: %d\n",p_header_icmp->Type);
 		printf("\nCode ICMP: %d\n",p_header_icmp->Code);
 		C.s_addr = p_header_ip->IPS;
 		b = inet_ntoa(C);
 		printf("\nIP source: %s\n",b);
 		C.s_addr = p_header_ip->IPD;
 		b = inet_ntoa(C);
 		printf("\nIP destination: %s\n",b);
    	
    	if(read_bytes==-1){
			perror("recvfrom()");
			close(sock);
			exit(1);
		}
   	}

   	close(sock);

   	return 0;
}
