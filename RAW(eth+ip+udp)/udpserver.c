#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char* argv[]){

	struct sockaddr_in in_addres;
	socklen_t sock_len = sizeof(in_addres);

	memset(&in_addres, 0, sizeof(in_addres));

	

	int sock = 0;
	int read_bytes=0;
	int send_bytes=0;
	int ntohs_addr = inet_addr(argv[1]);

	char buf[6]="Hello!";
	unsigned char msgbuf[50];

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0){
		perror("socket()");
		exit(1);
	}

	
	printf("\nsocket = %d\n", sock);
	in_addres.sin_family = AF_INET;
	in_addres.sin_port = 0;
	in_addres.sin_addr.s_addr = ntohs_addr;

	if((bind(sock, (struct sockaddr*) &in_addres, sizeof(in_addres))) == -1){
		perror("bind()");
		exit(1);
	} 
	if(getsockname(sock, (struct sockaddr *)&in_addres, &sock_len)<0){
		perror("getsockname");
		return 0;
	}

	printf("\nPort:= %d\n", ntohs(in_addres.sin_port));

	struct sockaddr_in client_addr;
	socklen_t socklen = sizeof(client_addr);

	while(1){
	read_bytes = recvfrom(sock, &msgbuf, sizeof(msgbuf), 0, (struct sockaddr*)&client_addr, &socklen);
	if(read_bytes==-1){
		perror("recvfrom()");
		close(sock);
		exit(1);
	}
	printf("\nBytes reads:= %d\n", read_bytes);
		for(int i=0; i<read_bytes; i++)
			printf("%c", msgbuf[i]);

	
	send_bytes = sendto(sock, buf, sizeof(buf), 0, (struct sockaddr*)&client_addr, socklen);
	if(send_bytes==-1){
		perror("sendto()");
		close(sock);
		exit(1);
	}
	printf("\n\n");
	}
	
	close(sock);
	return 0;
}