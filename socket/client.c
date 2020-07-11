/*************************************************************************
    > File Name: client.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Mon 06 Jul 2020 04:38:25 PM CST
 ************************************************************************/

#include "head.h"
#include "common.h"

int main(int argc, char *argv[]){

	char ip[20] = {0};
	int port = 0, sockfd;
	struct sockaddr_in server;
	char info[512];
	if(argc != 3){
		fprintf(stderr, "Usage: %s ip port!\n", argv[0]);
		exit(0);
	}

	strcpy(ip, argv[1]);
	port = atoi(argv[2]);

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket()");
		exit(1);
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = inet_addr(ip);

	if(connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0){
		perror("connect");
		exit(1);
	}
	
	int buff;
	
	recv(sockfd, (void *)&buff, sizeof(buff), 0);
	while(buff != 3){
		if(buff == 1){
			sprintf(info, "%s", "2019302180191");
			send(sockfd, info, strlen(info), 0);
		}	
		else if(buff == 2){
			sprintf(info, "%s", "黄丞");
			send(sockfd, info, strlen(info), 0);
		}
		recv(sockfd, (void *)&buff, sizeof(buff), 0);
	}
	close(sockfd);
	return 0;
}
