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

int main(int argc, char* argv[]){

	struct sockaddr_in in_addres;
	memset(&in_addres, 0, sizeof(in_addres));

    int sock = 0;
	int ntohs_addr = inet_addr(argv[1]);
	int read_bytes=0;
	int send_bytes=0;

	unsigned char buf[3]="Hi!";
	unsigned char msgbuf[11];
	unsigned char msgbuf2[50];
	unsigned short int sourceport=0;
    unsigned short int destinationport=0;

	struct UDPRAW header_udp;
	memset(&header_udp, 0, sizeof(header_udp));

	sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	if(sock < 0){
		perror("socket()");
		exit(1);
	}

	in_addres.sin_family = AF_INET;
	in_addres.sin_addr.s_addr = ntohs_addr;

	header_udp.SP=htons(63000);
	header_udp.DP=htons(atoi(argv[2]));
	header_udp.LEN=htons(sizeof(header_udp)+sizeof(buf));
	header_udp.CHECKSUM=0;
    memcpy((void *)msgbuf, (void *)&header_udp, sizeof(header_udp));
    memcpy((void *)(msgbuf+sizeof(header_udp)), (void *)buf, sizeof(buf));

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