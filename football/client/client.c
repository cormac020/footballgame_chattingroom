/*************************************************************************
    > File Name: client.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Wed 08 Jul 2020 03:08:13 PM CST
 ************************************************************************/

#include "head.h"
#include "common.h"
#include "color.h"
#include "datatype.h"
#include "udp_server.h"
#include "client_recv.h"
int sockfd = -1;
int server_port = 0;
char server_ip[20] = {0};
char *conf = "./football.conf";

void logout(int signum){
    struct ChatMsg msg;
    msg.type = CHAT_FIN;
    send(sockfd, (void *)&msg, sizeof(msg), 0);
    close(sockfd);
    DBG(L_GREEN"BYE!\n");
    exit(0);
}

int main(int argc, char **argv){
    struct LogRequest request;
    struct LogResponse response;
    bzero(&request, sizeof(request));
    bzero(&response, sizeof(response));
	//deal pharams
	//h:p:n:t:m
	//Host_ip_of_server, Port_of_server, Name_of_player, Team_num(1:blue, 0:red), Message_for_login
	//ifndef scanf from conf
	int opt;
	//printf("error0");
	while((opt = getopt(argc, argv, "h:p:t:m:n:")) != -1){
		switch(opt){
			case 't':
				request.team = atoi(optarg);
				break;
			case 'h':
				strcpy(server_ip, optarg);
				break;
			case 'p':
				server_port = atoi(optarg);
				break;
			case 'm':
				strcpy(request.msg, optarg);
				break;
			case 'n':
				strcpy(request.name, optarg);
				break;
			default:
				fprintf(stderr, "Usage : %s [-hptmn]!\n", argv[0]);
				exit(1);
		}
	}
	//printf("error1");
	if(!server_port) server_port = atoi(get_conf_value(conf, "SERVERPORT"));
	if(!request.team) request.team = atoi(get_conf_value(conf, "TEAM"));
	if(!strlen(request.name)) strcpy(request.name, get_conf_value(conf, "NAME"));
	if(!strlen(server_ip)) strcpy(server_ip, get_conf_value(conf, "SERVERIP"));
	if(!strlen(request.msg)) strcpy(request.msg, get_conf_value(conf, "LOGMSG"));

	DBG("<"GREEN"Conf Display"NONE"> : server_ip = %s, port = %d, team = %s, name = %s\n%s"\
			, server_ip, server_port, request.team? "BLUE" : "RED", request.name, request.msg);

    
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(server_port);
	server.sin_addr.s_addr = inet_addr(server_ip);

	socklen_t len = sizeof(server);

	if((sockfd = socket_create_udp(server_port)) < 0){
		perror("socket_udp()");
		exit(1);
	}

	if(sendto(sockfd, (void *)&request, sizeof(request), 0, (struct sockaddr *)&server, len) < 0){
        perror("sendto()");
        exit(1);
    }
    
    
    DBG("\n<"YELLOW"Send data"NONE"> : send %s to %s\n", request.msg, server_ip);
    fd_set set;
    FD_ZERO(&set);
    FD_SET(sockfd, &set);
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    
    int retval;
    if((retval = select(sockfd + 1, &set, NULL, NULL, &tv)) < 0){
        perror("select()");
        exit(1);
    } else if(retval){
        int ret = recvfrom(sockfd, (void *)&response, sizeof(response), 0, (struct sockaddr *)&server, &len);
        if(ret != sizeof(response) || response.type){
            DBG(RED"Error"NONE"The Game Server refused your login.\n\t This May Be Helpful : %s\n", response.msg);
        }
    } else{
        DBG(RED"Error"NONE"The Game Server is out of service!\n");
        exit(1);
    }
    
    DBG(GREEN"Server"NONE" : %s\n", response.msg);

    connect(sockfd, (struct sockaddr *)&server, len);
    //DBG(YELLOW"Connect Success!\n");
    //char buff[512] = {0};
    //sprintf(buff, "A small step.");
	//sendto(sockfd, (void *)&request, sizeof(request), 0, (struct sockaddr *)&server, len);
    //send(sockfd, buff, strlen(buff), 0);
    //DBG(GREEN"Send: %s to server.\n", buff);
    //recvfrom(sockfd, (void *)&response, sizeof(response), 0, (struct sockaddr *)&server, &len);
    //recv(sockfd, buff, sizeof(buff), 0);
    //DBG(RED"Server Info"NONE" : %s\n", buff);
    signal(SIGINT, logout);
    pthread_t recv_t;
    pthread_create(&recv_t, NULL, do_recv, NULL);
    struct ChatMsg msg;
    while(1){
        bzero(&msg, sizeof(msg));
        msg.type = CHAT_WALL;
        printf(RED"Please Input: \n"NONE);
        scanf("%[^\n]s", msg.msg);
        getchar();
        if(strlen(msg.msg)){
            if(msg.msg[0] == '@') msg.type = CHAT_MSG;
            if(msg.msg[0] == '#') msg.type = CHAT_FUNC;
            send(sockfd, (void *)&msg, sizeof(msg), 0);
        }
    }
    /*DBG("<"YELLOW"Server Response"NONE"> : from %s\n", server_ip);

    while(recvfrom(sockfd, response.msg, sizeof(response.msg), 0, (struct sockaddr *)&server, &len) < 0){
        fprintf(stderr, "Invalid Input: %s\n", response.msg);
        //exit(1);        
    }
    if(response.type == 1){
        fprintf(stderr, "Invalid type: %s\n", response.msg);
        exit(1);
    }
    DBG("<"PINK"Recv msg"NONE"> : %s\n", response.msg);
    if(connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0){
        perror("connect()");
        exit(1);
    }
    DBG("<"RED"Connect Success"NONE"> : connect to %s:%d\n", server_ip, server_port);
    char buff[512] = "A small step";
	sendto(sockfd, (void *)&buff, strlen(buff), 0, (struct sockaddr *)&server, len);
    if(send(sockfd, buff, strlen(buff), 0) < 0){
        perror("send()");
        exit(1);
    }
    DBG("<"L_GREEN"Resend msg"NONE"> : send \"%s\" to %s\n", buff, server_ip);
    bzero(buff, sizeof(buff));
    while(recv(sockfd, buff, sizeof(buff), 0) > 0){
        DBG(RED"Server Info"NONE" : %s\n", buff);
    }*/
    return 0;

}
    
    
/*
Usage: ./a.out -h serverip -p port -t team -n name
*/
