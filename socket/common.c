#include "head.h"

int socket_create(int port)
{
	int sockfd;
	struct sockaddr_in server;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return -1;
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	int reuse = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_RESUEADDR, (char *)&reuse, sizeof(int));

	if(bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		return -1;
	}

	if(listen(sockfd, 10) < 0)
	{
		return -1;
	}

	return sockfd;
}

void make_non_block(int fd){
	unsigned long ul = 1;
	ioctl(fd, FIONBIO, &ul);
}

void make_block(int fd){
	unsigned long ul = 0;
	ioctl(fd, FIONBIO, &ul);
}
