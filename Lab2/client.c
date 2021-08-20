#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#define MAXLINE 256

int main(int argc, char *argv[])
{
	int n, sockfd, PORT_NUM;
	struct sockaddr_in serv_addr;
	char sendline[MAXLINE], recvline[MAXLINE];

	if (argc < 3) {
		fprintf(stderr,"usage %s <server-ip-addr> <server-port>\n",argv[0]);
		exit(0);
	}
	PORT_NUM = atoi(argv[2]);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr= inet_addr(argv[1]);
	serv_addr.sin_port =  htons(PORT_NUM);

	if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
		fprintf(stderr, "ERROR opening socket\n");
		exit(1);
	}
	else printf("Socket successfully created..\n");

	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0) {
		if(errno == 111) perror("Line Busy : ");
		else perror("Error connectong");
		exit(1);
	}
	else printf("Connected to server..\n");

	do{
		printf("Please enter the message to the server: ");

		bzero(sendline,MAXLINE);
		fgets(sendline,sizeof(sendline),stdin);
		if ((n = send(sockfd,sendline,sizeof(sendline),0)) < 0){
			fprintf(stderr, "ERROR writing to socket\n");
			exit(1);
		}

		bzero(recvline,MAXLINE);
		if ((n = recv(sockfd,recvline,MAXLINE,0)) < 0){
			fprintf(stderr, "ERROR reading from socket\n");
			exit(1);
		}
		printf("Server replied: %s\n",recvline);
		if ((strncmp(recvline, "exit", 4)) == 0) { 
			printf("Client Exit...\n"); 
			break; 
		}
	} while(1);
	
	exit(0);
}
