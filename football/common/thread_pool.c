/*************************************************************************
    > File Name: thread_pool.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Thu 09 Jul 2020 02:49:14 PM CST
 ************************************************************************/

#include "head.h"
#include "color.h"
#include "common.h"
#include "datatype.h"
#include "sub_reactor.h"
#include "udp_server.h"
#include "udp_client.h"
#include "udp_epoll.h"
#include "thread_pool.h"
extern int repollfd, bepollfd;
extern struct User *bteam, *rteam;
extern pthread_mutex_t bmutex, rmutex;

void send_all(struct ChatMsg *msg){
    for(int i = 0; i < MAX; i++){
        if(bteam[i].online) send(bteam[i].fd, (void *)msg, sizeof(struct ChatMsg), 0);
        if(rteam[i].online) send(rteam[i].fd, (void *)msg, sizeof(struct ChatMsg), 0);
        
    }
}

void send_to(char *to, struct ChatMsg *msg, int fd){
    int flag = 0;
    for(int i = 0; i < MAX; i++){
        if(rteam[i].online && (!strcmp(to, rteam[i].name) || !strcmp(to, rteam[i].name))){
            send(rteam[i].fd, msg, sizeof(struct ChatMsg), 0);
            flag = 1;
            break;
        }
        if(bteam[i].online && (!strcmp(to, bteam[i].name) || !strcmp(to, bteam[i].name))){
            send(bteam[i].fd, msg, sizeof(struct ChatMsg), 0);
            flag = 1;
            break;
        }
    }
    if(!flag){
        memset(msg->msg, 0, sizeof(msg->msg));
        sprintf(msg->msg, "用户 %s 不在线，或用户名错误！", to);
        msg->type = CHAT_SYS;
        send(fd, msg, sizeof(struct ChatMsg), 0);
    }
}
void do_work(struct User *user){
	//receive a message, print
    struct ChatMsg msg, r_msg;
    bzero(&msg, sizeof(msg));
    bzero(&r_msg, sizeof(r_msg));
    recv(user->fd, (void *)&msg, sizeof(msg), 0);
    //****************
    //
    if(msg.type & CHAT_FUNC){
        char list[512][20] = {0};
        int nuser = list_online(list);
        for(int i = 0; i < nuser; i++)
            send(user->fd, list[i], strlen(list[i]), 0);
        

    //***********************
    }else if(msg.type & CHAT_WALL){
        printf("<%s> ~ %s \n", user->name, msg.msg);
        send_all(&msg);
    }else if(msg.type & CHAT_MSG){
        char to[20] = {0};
        int i = 1;
        for(; i < 20; i++){
            if(msg.msg[i] == ' ') break;
        }

        if(msg.msg[i] != ' ' ||msg.msg[0] != '@'){
            memset(&r_msg, 0, sizeof(r_msg));
            r_msg.type = CHAT_SYS;
            sprintf(r_msg.msg, "私聊格式错误！");
            send(user->fd, (void *)&r_msg, sizeof(r_msg), 0);
        }else{
            msg.type = CHAT_MSG;
            strncpy(to, msg.msg + 1, i - 1);
            send_to(to, &msg, user->fd);
        }
        /*if(msg.msg[0] != '@'){
            memset(msg.msg, 0, sizeof(msg.msg));
            sprintf(msg.msg, "Choose a user!");
            msg.type = CHAT_SYS;
            send(user->fd, &msg, sizeof(struct ChatMsg), 0);
        }
        int i;
        for(i = 1; i < strlen(msg.msg); i++){
            if(msg.msg[i] == ' ') break;
        }
        char name[20] = {0};
        strncpy(name, msg.msg + 1, i - 1);
        send_to(name, &msg, user->fd);*/
    }else if(msg.type & CHAT_FIN){
        bzero(msg.msg, sizeof(msg.msg));
        msg.type = CHAT_SYS;
        sprintf(msg.msg, " %s 下线了", user->name);
        send_all(&msg);
        user->online = 0;
        if(user->team)
            pthread_mutex_lock(&bmutex);
        else
            pthread_mutex_lock(&rmutex);
        int epollfd = user->team ? bepollfd : repollfd;
        del_event(epollfd, user->fd);
        if(user->team)
            pthread_mutex_unlock(&bmutex);
        else
            pthread_mutex_unlock(&rmutex);
        printf(GREEN"Server Info"NONE" : %s logout", user->name);
        close(user->fd);
    }
}

void task_queue_init(struct task_queue *taskQueue, int sum, int epollfd){
	taskQueue->sum = sum;
	taskQueue->epollfd = epollfd;
	taskQueue->team = calloc(sum, sizeof(void *));
	taskQueue->head = taskQueue->tail = 0;
	pthread_mutex_init(&taskQueue->mutex, NULL);
	pthread_cond_init(&taskQueue->cond, NULL);
}

void task_queue_push(struct task_queue *taskQueue, struct User *user){
	pthread_mutex_lock(&taskQueue->mutex);
	taskQueue->team[taskQueue->tail] = user;
	DBG(L_GREEN"Thread Pool"NONE" : Task push %s\n", user->name);
	if(++taskQueue->tail == taskQueue->sum){
		DBG(L_GREEN"Thread Pool"NONE" : Task Queue End\n");
		taskQueue->tail = 0;
	}
	pthread_cond_signal(&taskQueue->cond);
	pthread_mutex_unlock(&taskQueue->mutex);
}

struct User *task_queue_pop(struct task_queue *taskQueue){
	pthread_mutex_lock(&taskQueue->mutex);
	while(taskQueue->tail == taskQueue->head){
		DBG(L_GREEN"Thread Pool"NONE" : Task Queue Empty, Waiting For Task\n");
		pthread_cond_wait(&taskQueue->cond, &taskQueue->mutex);
	}
	struct User *user = taskQueue->team[taskQueue->head];
	DBG(L_GREEN"Thread Pool"NONE" : Task Pop %s\n", user->name);
	if(++taskQueue->head == taskQueue->sum){
		DBG(L_GREEN"Thread Pool"NONE" : Task Queue End\n");
		taskQueue->head = 0;
	}
	pthread_mutex_unlock(&taskQueue->mutex);
	return user;
}

void *thread_run(void *arg){
	pthread_detach(pthread_self());
	struct task_queue *taskQueue = (struct task_queue *)arg;
	while(1){
		struct User *user = task_queue_pop(taskQueue);
		do_work(user);
	}
}
