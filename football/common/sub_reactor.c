/*************************************************************************
    > File Name: sub_reactor.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Thu 09 Jul 2020 03:44:22 PM CST
 ************************************************************************/

#include "head.h"
#include "color.h"
#include "datatype.h"
#include "common.h"
#include <sys/epoll.h>
#include "thread_pool.h"
void *sub_reactor(void *arg){
	struct task_queue *taskQueue = (struct task_queue *)arg;
	pthread_t *tid = (pthread_t *)calloc(NTHREAD, sizeof(pthread_t));
	for(int i = 0; i < NTHREAD; i++){
        DBG(YELLOW"Create Thread %d\n", i);
		pthread_create(&tid[i], NULL, thread_run, (void *)taskQueue);
	}
	struct epoll_event ev, events[MAX];
    DBG(L_GREEN"Into While(1)\n");
	while(1){
		DBG(L_RED"Sub Reactor"NONE" : Epoll Waiting...\n");
		int nfds = epoll_wait(taskQueue->epollfd, events, MAX, -1);
        DBG(L_RED"nfd = %d\n", nfds);
		if(nfds < 0){
			perror("epoll_wait");
			exit(1);
		}
        DBG("Now Client number: %d\n", nfds);
		for(int i = 0; i < nfds; i++){
			struct User *user = (struct User *)events[i].data.ptr;
			DBG(L_RED"Sub Reactor"NONE" : %s Ready\n", user->name);
			if(events[i].events & EPOLLIN){
                task_queue_push(taskQueue, user);
			}
		}
	}
	return NULL;
}
