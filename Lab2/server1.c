#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#define MAXLINE 256
#define LISTENQ 1

#define COLOR_RED	  "\x1B[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RESET   "\x1b[0m"

float calculate(char *buffer){
	int i=1,first = 0;
	float result;
	char *str = (char*)malloc((strlen(buffer)-1)*sizeof(char));
	strncpy(str,buffer,strlen(buffer)-1);
	
	if(str[0] == '-'){
		first = 1;
		i=2;
	}
	if((str[first] < '0' || str[first] > '9'))
		return -999.99f;
	else result = (first == 0) ? (float)(str[first]-'0') : -(float)(str[first]-'0');
	for ( ; str[i]; i += 2) { 
		char operator = str[i], operand = str[i+1]; 
		if((operand < '0' || operand > '9')) return -999.99f; 
		if (operator == '+') result += (float)(operand - '0'); 
		else if (operator == '-')  result -= (float)(operand - '0'); 
		else if (operator == '*')  result *= (float)(operand - '0'); 
		else if (operator == '/')  result /= (float)(operand - '0'); 
		else return -999.99f; 
	} 
	return result;
}

int create_listen(struct sockaddr_in serv_addr){
	int listenfd;

	if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
		fprintf(stderr, "ERROR opening socket\n");
		exit(1);
	}

	if (bind (listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) <0) {
		fprintf(stderr, "ERROR binding\n");
		exit(1);
	}

	if (listen (listenfd, LISTENQ) <0) {
		fprintf(stderr, "ERROR listening\n");
		exit(1);
	}

	return listenfd;
}

int main (int argc, char *argv[])
{
	int listenfd, connfd, n, PORT_NUM;
	float answer;
	socklen_t cli_len;
	char buf[MAXLINE];
	struct sockaddr_in cli_addr, serv_addr;

	if (argc < 2) {
		fprintf(stderr,"usage %s <port>\n",argv[0]);
		exit(0);
	}
	PORT_NUM = atoi(argv[1]);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port =  htons(PORT_NUM);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	listenfd = create_listen(serv_addr);

	printf("%s\n","Server running...waiting for connections.");

	while(1) {
		cli_len = sizeof(cli_addr);
		if ((connfd = accept (listenfd, (struct sockaddr *) &cli_addr, &cli_len)) < 0) { 
			printf("server acccept failed...\n"); 
			exit(0); 
		} 
		else printf("%s%s%d%s\n",COLOR_GREEN,"Connected with client socket number ",connfd,COLOR_RESET);
		
		close(listenfd); /*Close the listening socket to avoid more than one connection*/
		
		while ((n = recv(connfd, buf, MAXLINE,0)) > 0)  {
			printf("Client socket %d sent message: %s",connfd,buf);
			if(strncmp(buf,"exit",4)==0){
				if (send(connfd,buf,n,0) < 0){
					fprintf(stderr, "ERROR writing to socket\n");
					exit(1);
				}
				printf("Sending reply: exit\n");
				continue;
			}

			answer = calculate(buf);
			bzero(buf,MAXLINE);
			if(answer == -999.99f)
				strcpy(buf,"INVALID");
			else{	
				if(answer == (int)answer) sprintf(buf,"%d",(int)answer);
				else sprintf(buf,"%f",answer);
			}
			if ((n = send(connfd,buf,MAXLINE,0)) < 0){
				fprintf(stderr, "ERROR reading from socket\n");
				exit(1);
			}
			printf("Sending reply: %s\n",buf);
		}
		if (n < 0) {
			perror("Read error");
			exit(1);
		}
		if (n == 0){   
			getpeername(connfd, (struct sockaddr*)&cli_addr, &cli_len);   
			printf("%sDisconnected socket no. %d, ip %s, port %d%s\n",COLOR_RED,connfd, inet_ntoa(cli_addr.sin_addr) , ntohs(cli_addr.sin_port),COLOR_RESET);   
		}
		close(connfd); 
		listenfd = create_listen(serv_addr); /*Again create and listen to socket*/
	}
	exit(0);
}
