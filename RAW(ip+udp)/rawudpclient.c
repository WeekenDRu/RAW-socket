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

int main(int argc, char* argv[]){

	struct sockaddr_in in_addres;
	struct UDPRAW header_udp;
	struct IPRAW header_ip;
	memset(&in_addres, 0, sizeof(in_addres));
	memset(&header_udp, 0, sizeof(header_udp));
	memset(&header_ip, 0, sizeof(header_ip));

    int sock = 0;
	int ntohs_addr = inet_addr(argv[1]);
	int ntohs_addr_2 = inet_addr(argv[3]);
	int read_bytes=0;
	int send_bytes=0;
	int one = 1;
	const int *val = &one;

	unsigned char buf[3]="Hi!";
	unsigned char msgbuf[31];
	unsigned char msgbuf2[50];
	unsigned short int sourceport=0;
    unsigned short int destinationport=0;

    memset(&msgbuf, 0, sizeof(msgbuf));

	sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	if(sock < 0){
		perror("socket()");
		exit(1);
	}

	if(setsockopt(sock, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0){
		perror("setsockopt() error");
		exit(-1);
	}

	in_addres.sin_family = AF_INET;
	in_addres.sin_addr.s_addr = ntohs_addr;

	header_ip.VARIHL=0x45;
	header_ip.DS=0x00;
	//header_ip.TL=0;
	header_ip.TL = htons(sizeof(struct IPRAW) + sizeof(struct UDPRAW) + sizeof(buf));
	header_ip.Ident=0;
	header_ip.FLAGSOFFSET=0;
	header_ip.TTL=64;
	header_ip.PROTO=0x11;
	header_ip.CSC=0;
	header_ip.IPS=ntohs_addr_2;
	header_ip.IPD=ntohs_addr;

	header_udp.SP=htons(63000);
	header_udp.DP=htons(atoi(argv[2]));
	header_udp.LEN=htons(sizeof(header_udp)+sizeof(buf));
	header_udp.CHECKSUM=0;

	memcpy((void *)msgbuf, (void *)&header_ip, sizeof(header_ip));
    memcpy((void *)(msgbuf + sizeof(header_ip)), (void *)&header_udp, sizeof(header_udp));
    memcpy((void *)(msgbuf + sizeof(header_ip) + sizeof(header_udp)), (void *)buf, sizeof(buf));

    send_bytes = sendto(sock, msgbuf, sizeof(msgbuf), 0, (struct sockaddr*)&in_addres, sizeof(in_addres));

    if(send_bytes==-1){
		perror("sendto()");
		close(sock);
		exit(1);
	}

    while(1){
    	read_bytes = recvfrom(sock, msgbuf2, 1024, 0, NULL, NULL);	
    	
    	if(read_bytes==-1){
			perror("recvfrom()");
			close(sock);
			exit(1);
		}

		destinationport=msgbuf2[22]<<8|msgbuf2[23];

		if(destinationport==ntohs(header_udp.SP)){
			printf("\nByte reads:= %d\n", read_bytes);
			for(int i=28; i<read_bytes; i++){
				printf("%c", msgbuf2[i]);
			}
			printf("\n");
		}
   	}

   	close(sock);

   	return 0;
}
