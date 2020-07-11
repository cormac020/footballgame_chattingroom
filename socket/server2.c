/*************************************************************************
    > File Name: server2.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Wed 08 Jul 2020 11:13:07 AM CST
 ************************************************************************/

#include "head.h"
#include "common.h"
#define MAX_EVENTS 25
#define MAX_CLIENTS 250
#define BUF_SIZE 4096

int main(int argc, char **argv){
	int epollfd, server_listen, fd, port;
	char ip[20] = {0}, buff[BUF_SIZE] = {0};
	struct epoll_event ev, events[MAX_EVENTS];
	//
	if(argc != 2){
		fprintf(stderr, "Usage : %s port!\n", argv[0]);
		exit(1);
	}

	port = atoi(argv[1]);

	if((server_listen = socket_create(port)) < 0){
		perror("socket_create()");
		exit(1);
	}
	
	make_non_block(server_listen);

	if((epollfd = epoll_create(MAX_CLIENTS))){
		perror("epoll_create()");
		exit(1);
	}
	
	ev.events = EPOLLIN;
	ev.data.fd = server_listen;

	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, server_listen, &ev) < 0){
		perror("epoll_ctl");
		exit(1);
	}

	while(1){
		int nfds;
		if((nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1)) < 0){
			perror("epoll_wait()");
			exit(1);
		}

		for(int i = 0; i < nfds; i++){
			if(events[i].data.fd == server_listen){
				struct sockadd_in client;
				socklen_t len = sizeof(client);
				if((fd = accept(server_listen, (struct sockaddr *)&client, &len)) < 0){
					perror("accept()");
					exit(1);
				}
				make_non_block(fd);
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = fd;

				if(epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) < 0){
					perror("epoll_ctl()");
					exit(1);
				}
			} else{
				if(events[i].events & (EPOLLIN | EPOLLHUP | EPOLLERR)){
					memset(buff, 0, sizeof(buff));
					if(recv(events[i].data.fd, buff, BUF_SIZE, 0) > 0){
						printf("Recv: %s\n", buff);
						for(int i = 0; i < strlen(buff); i++){
							if(buff[i] >= 'a' && buff[i] <= 'z') buff[i] -= 32;
						}
						send(event[i].data.fd, buff, strlen(buff), 0);
					} else{
						if(epoll_ctl(epollfd, EPOLL_CTL_DEL, event[i].data.fd, NULL) < 0){
							perror("epoll_ctl()");
							exit(1);
						}
						close(events[i].data.fd);
					}
				}
			}
		}
	}

	return 0;
}
