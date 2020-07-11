/*************************************************************************
    > File Name: server.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Mon 06 Jul 2020 04:38:25 PM CST
 ************************************************************************/

#include "head.h"
#include "common.h"

void *work(void *arg)
{
	int fd;
	char buff[512] = {0};
	fd = *(int *)arg;
	recv(fd, buff, sizeof(buff), 0);
	printf("Recv Msg: %s\n", buff);
	close(fd);
	return NULL;
}
int main()
{

	int server_listen, sockfd;
	if((server_listen = socket_create(8888)) < 0)
	{
		perror("socket_create()");
	}

	while(1)
	{
		pthread_t tid;
		if((sockfd = accept(server_listen, NULL, NULL)) > 0)
		{
			pthread_create(&tid, NULL, work, (void *)&sockfd);
		}
	}

}
