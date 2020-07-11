/*************************************************************************
    > File Name: server.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Tue 07 Jul 2020 03:37:05 PM CST
 ************************************************************************/
#include "head.h"
#include "color.h"
#include "common.h"

#define MAX_CLIENT 250
#define BUF_SIZE 4096

struct Buffer{
	int fd;
	char buff[BUF_SIZE];
	int flag;
	int sendindex;
	int recvindex;
};

struct Buffer *AllocBuffer(){
	struct Buffer *buffer = (struct Buffer *)malloc(sizeof(struct Buffer));
	if(buffer == NULL){
		return NULL;
	}
	buffer->fd = -1;
	buffer->flag = buffer->recvindex = buffer->sendindex = 0;
	memset(buffer->buff, 0, BUF_SIZE);
	return buffer;
}

void FreeBuffer(struct Buffer *buffer){
	free(buffer);
}

char ch_char(char c){
	if(c >= 'a' && c <= 'z')
		return c - 32;
	return c;
}

int RecvToBuffer(int fd, struct Buffer *buffer){
	char ip[20] = {0};
	struct sockaddr_in client;
	strcpy(ip, inet_ntoa(client.sin_addr));
	socklen_t len = sizeof(client);
	getpeername(fd, (struct sockaddr *)&client, &len);
	int nrecv = -1;
	while(1){
		char buff[BUF_SIZE] = {0};
		nrecv = recv(fd, buff, sizeof(buff), 0);
		if(nrecv <= 0)
			break;
		DBG("<"GREEN"Recv Data"NONE"> : %s --> %s\n", ip, buff);
		for(int i = 0; i < nrecv; i++){
			if(buffer->recvindex < sizeof(buffer->buff)){
				//printf("buff:%c, recv:%d\n", buff[i], buffer->recvindex);
				buffer->buff[buffer->recvindex++] = ch_char(buff[i]);
				
			}
			//condition?
			buffer->flag = 1;
		}
	}
	if(nrecv < 0){
		if(errno == EAGAIN)
			return 0;
		return -1;
	}else if(nrecv == 0){
		return 1;
	}
}

int SendFromBuffer(int fd, struct Buffer *buffer){
	char ip[20] = {0};
	struct sockaddr_in client;
	socklen_t len = sizeof(client);
	getpeername(fd, (struct sockaddr *)&client, &len);
	strcpy(ip, inet_ntoa(client.sin_addr));
	int nsend = -1;
	while(buffer->sendindex < buffer->recvindex){
		DBG("<"L_GREEN"Send Data"NONE"> : send data to %s.\n", ip);
		nsend = send(fd, buffer->buff + buffer->sendindex, buffer->recvindex - buffer->sendindex, 0);
		
		if(nsend < 0){
			if(errno < 0){
				return 0;
			}
			buffer->fd = -1;
			return -1;
		}
		buffer->sendindex += nsend;
		if(buffer->sendindex == buffer->recvindex)
			buffer->sendindex = buffer->recvindex = 0;
		buffer->flag = 0;
		return 0;
	}
}

int main(int argc, char **argv){
	if(argc != 2){
		fprintf(stderr, "Usage: %s port!\n", argv[0]);
		exit(1);
	}

	int server_listen, fd, max_fd, port;
	port = atoi(argv[1]);
	if((server_listen = socket_create(port)) < 0){
		perror("socket_create()");
	}

	struct Buffer *buffer[MAX_CLIENT];

	for(int i = 0; i < MAX_CLIENT; i++){
		buffer[i] = AllocBuffer();
	}

	make_non_block(server_listen);

	fd_set rfds, wfds, efds;
	max_fd = server_listen;

	while(1){
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		FD_ZERO(&efds);
		FD_SET(server_listen, &rfds);
		DBG("<"PINK"Add rfds"NONE"> : set %d in rfds.\n", server_listen);
		
		for(int i = 0; i < MAX_CLIENT; i++){
			if(buffer[i]->fd == server_listen) continue;
			if(buffer[i]->fd > 0){
				if(max_fd < buffer[i]->fd) max_fd = buffer[i]->fd;
				FD_SET(buffer[i]->fd, &rfds);
				DBG("<"PINK"Add rfds"NONE"> : set %d in rfds.\n", buffer[i]->fd);
				if(buffer[i]->flag == 1){
					FD_SET(buffer[i]->fd, &wfds);
					DBG("<"YELLOW"Add wfds"NONE"> : set %d in rfds.\n", buffer[i]->fd);
				}
			}
		}
		int ret_val;
		if((ret_val = select(max_fd + 1, &rfds, &wfds, NULL, NULL)) < 0){
			perror("select");
		}
		DBG("<"BLUE"Select Return"NONE"> : select return success %d fds.\n", ret_val);

		if(FD_ISSET(server_listen, &rfds)){
			DBG("<"L_RED"Connect Request"NONE"> : new client request for login.\n");
			struct sockaddr_in client;
			socklen_t len = sizeof(client);
			if((fd = accept(server_listen, (struct sockaddr *)&client, &len)) < 0){
				perror("accept()");
				exit(1);
			}
			DBG("<"RED"Login Success"NONE"> : %s Login success\n", inet_ntoa(client.sin_addr));
			if(fd > MAX_CLIENT){
				fprintf(stderr, "too many clients!\n");
				close(fd);
			}else{
				make_non_block(fd);
				if(buffer[fd]->fd == -1)
					buffer[fd]->fd = fd;
			}
		}
		for(int i = 0; i <= max_fd; i++){
			int retval = 0;
			if(buffer[i]->fd == server_listen || buffer[i]->fd < 0) continue;
			if(FD_ISSET(buffer[i]->fd, &rfds)){
				retval = RecvToBuffer(buffer[i]->fd, buffer[i]);
				
			}
			if(retval == 0 && FD_ISSET(buffer[i]->fd, &wfds)){
				retval = SendFromBuffer(buffer[i]->fd, buffer[i]);
			}

			if(retval){
				struct sockaddr_in client;
				socklen_t len = sizeof(client);
				getpeername(buffer[i]->fd, (struct sockaddr *)&client, &len);
				DBG("<"RED"Logout"NONE"> : %s Logout\n", inet_ntoa(client.sin_addr));
				buffer[i]->fd = -1;
				close(buffer[i]->fd);
			}
		}
	}

	return 0;
}
