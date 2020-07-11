/*************************************************************************
    > File Name: server.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Thu 09 Jul 2020 10:52:13 AM CST
 ************************************************************************/

#include "head.h"
#include "datatype.h"
#include "color.h"
#include "common.h"
#include "udp_server.h"
#include <sys/epoll.h>
#include "thread_pool.h"
#include "udp_epoll.h"
#include "sub_reactor.h"
char *conf = "./footballd.conf";
struct Map court;
struct Bpoint ball;//ball position
struct BallStatus ball_status;
struct Score score;
int repollfd, bepollfd;
struct User *rteam, *bteam;
int port = 0;
pthread_mutex_t rmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bmutex = PTHREAD_MUTEX_INITIALIZER;
int main(int argc, char **argv){
	int opt, listener, epollfd;
	pthread_t red_t, blue_t;
	while((opt = getopt(argc, argv, "p:")) != -1){
		switch(opt){
			case 'p':
				port = atoi(optarg);
				break;
			default:
				fprintf(stderr, "Usage : %s -p port\n", argv[0]);
				exit(1);
		}
	}
	//check conf's validity
	if(!port) port = atoi(get_conf_value(conf, "PORT"));
	court.width = atoi(get_conf_value(conf, "COLS"));
	court.height = atoi(get_conf_value(conf, "LINES"));
	
	ball.x = court.width / 2;
	ball.y = court.height / 2;

	bzero(&ball_status, sizeof(ball_status));
	ball_status.who = -1;
	bzero(&score, sizeof(score));
	
	if((listener = socket_create_udp(port)) < 0){
		perror("socket_create_udp()");
		exit(1);
	}

	DBG(GREEN"INFO"NONE" : Server start on port %d.\n", port);
	
	rteam = (struct User *)calloc(MAX, sizeof(struct User));
	bteam = (struct User *)calloc(MAX, sizeof(struct User));

	epollfd = epoll_create(MAX * 2);
	repollfd = epoll_create(MAX * 2);
	bepollfd = epoll_create(MAX * 2);

	if(epollfd < 0 || repollfd < 0 || bepollfd < 0){
		perror("epoll_create()");
		exit(1);
	}

	struct task_queue redQueue;
	struct task_queue blueQueue;
    
    task_queue_init(&redQueue, MAX, repollfd);
    task_queue_init(&blueQueue, MAX, bepollfd);

	pthread_create(&red_t, NULL, sub_reactor, (void *)&redQueue);
	pthread_create(&blue_t, NULL, sub_reactor, (void *)&blueQueue);

	struct epoll_event ev, events[MAX * 2];
	ev.events = EPOLLIN;
	ev.data.fd = listener;

    DBG("Into EPOLL_CTL\n");
	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, listener, &ev) < 0){
		perror("epoll_ctl");
		exit(1);
	}

	struct sockaddr_in client;
	bzero(&client, sizeof(client));
	socklen_t len = sizeof(client);

	while(1){
		DBG(YELLOW"Main Reactor"NONE" : Waiting for client.\n");
		int nfds = epoll_wait(epollfd, events, MAX * 2, -1);
		if(nfds < 0){
			perror("epoll_wait()");
			exit(1);
		}
        DBG(L_RED"epoll_wait = %d\n", nfds);
		for(int i = 0; i < nfds; i++){
			struct User user;
            bzero(&user, sizeof(user));
			char buff[512] = {0};
			if(events[i].data.fd == listener){
				//only receive data
				recvfrom(listener, buff, sizeof(buff), 0, (struct sockaddr *)&client, &len);
				DBG(RED"Recv"NONE" :<%s:%d> %s\n", inet_ntoa(client.sin_addr), client.sin_port, buff);
				int new_fd = udp_accept(listener, &user);

				if(new_fd > 0){
                    DBG("Into ADD_TO_SUB_REACTOR\n");
					add_to_sub_reactor(&user);
				}
			}
		}
	}


	return 0;
}
