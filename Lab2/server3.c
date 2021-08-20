#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>

#define MAXLINE 256
#define LISTENQ 3
#define TRUE   1  
#define FALSE  0  

#define COLOR_RED 	  "\x1B[31m"
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
	int listenfd, connfd, n, PORT_NUM, opt = TRUE;
	int client_socket[10], max_clients = 10, activity;   
    int i, sd, max_sd;
	float answer;
	socklen_t cli_len;
	char buf[MAXLINE];
	struct sockaddr_in cli_addr, serv_addr;
	fd_set readfds;

	if (argc < 2) {
		fprintf(stderr,"usage %s <port>\n",argv[0]);
		exit(0);
	}
	PORT_NUM = atoi(argv[1]);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port =  htons(PORT_NUM);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	/*initialize all client_socket[] to 0 so not checked */ 
    for (i = 0; i < max_clients; i++)   {   
    	client_socket[i] = 0;   
    }

	listenfd = create_listen(serv_addr);

	printf("%s\n","Server running...waiting for connections.");
	cli_len = sizeof(cli_addr);

	while(1) {
        FD_ZERO(&readfds);   
        FD_SET(listenfd, &readfds);   
        max_sd = listenfd;

        for(i = 0; i < max_clients; ++i)   {   
            sd = client_socket[i];     
            if(sd > 0) FD_SET( sd , &readfds);                    
            if(sd > max_sd) max_sd = sd;   
        }

		/*wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely */
        activity = select(max_sd + 1 , &readfds , NULL , NULL , NULL);
        if ((activity < 0) && (errno!=EINTR))   
        {   
            printf("select error");   
        }  

		/*If something happened on the master socket ,then its an incoming connection*/
		if (FD_ISSET(listenfd, &readfds))   
        {     
            if ((connfd = accept (listenfd, (struct sockaddr *) &cli_addr, &cli_len)) < 0) { 
				perror("server acccept failed"); 
				exit(0); 
			}  
            else printf("%s%s%d%s\n",COLOR_GREEN,"Connected with client socket number ",connfd,COLOR_RESET);
                 
            for (i = 0; i < max_clients; i++)   {   
                if( client_socket[i] == 0 )   {   
                    client_socket[i] = connfd;   
                    printf("Adding to list of sockets as %d\n" , i);          
                    break;   
                }   
            }   
        }
		
        for (i = 0; i < max_clients; i++) {   
            sd = client_socket[i];   
            if (FD_ISSET( sd , &readfds)){   
                if ((n = recv(sd, buf, MAXLINE, 0)) == 0){   
                    getpeername(sd, (struct sockaddr*)&cli_addr, &cli_len);   
					printf("%sDisconnected client %d : socket no. %d, ip %s, port %d%s\n",COLOR_RED,i,sd, inet_ntoa(cli_addr.sin_addr) , ntohs(cli_addr.sin_port),COLOR_RESET);   
                    close(sd);   
                    client_socket[i] = 0;   
                }
                else {   
                    printf("Client socket %d sent message: %s",sd,buf);
					answer = calculate(buf);
					bzero(buf,MAXLINE);
					if(answer == -999.99f)
						strcpy(buf,"INVALID");
					else{	
						if(answer == (int)answer) sprintf(buf,"%d",(int)answer);
						else sprintf(buf,"%f",answer);
					}
					if ((n = send(sd,buf,MAXLINE,0)) < 0){
						fprintf(stderr, "ERROR reading from socket\n");
						exit(1);
					}
					printf("Sending reply: %s\n",buf);
                }   
            }   
        }
	}
	exit(0);
}
