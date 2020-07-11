/*************************************************************************
    > File Name: datatype.h
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Thu 09 Jul 2020 10:09:45 AM CST
 ************************************************************************/
#ifndef _DATATYPE_H
#define _DATATYPE_H
#define MAX 300//half of number of user
#define NTHREAD 3

//Ball concerned
struct Bpoint{
	double x;
	double y;
};

struct Speed{
	double x;
	double y;
};

struct Accel{
	double x;
	double y;
};

struct BallStatus{
	struct Speed v;
	struct Accel a;
	int who;//which team
	char name[20];//who controls ball
};

struct Score{
	int blue;
	int red;
};

//Player concerned
struct Point{
	int x;
	int y;
};
struct User{
	int team;//0 red, 1 blue
	int fd;//link of this player
	char name[20];
	int online;
	int flag;//times of non_response
    struct Point loc;
};

//data interact concerned
struct LogRequest{
	char name[20];
	int team;
	char msg[512];
};

struct LogResponse{
	int type;// 0 ok, 1 not ok
	char msg[512];
};

//interaction concerned during game
#define MAX_MSG 4096

#define ACTION_KICK 0x01
#define ACTION_CARRY 0x02
#define ACTION_STOP 0x04
struct Ctl{
	int action;
	int dirx;
	int diry;
	int strength;//power of kick
};

#define FT_HEART 0x01//pulse
#define FT_WALL 0x02//note
#define FT_MSG 0x04//chat
#define FT_ACK 0x08//ack
#define FT_CTL 0x10//ctrl info
#define FT_GAME 0x20//game environ
#define FT_SCORE 0x40//score
#define FT_GAMEOVER 0x80
#define FT_FIN 0x100//leave
struct FootBallMsg{
	int type;
	int size;
	int team;
	char name[20];
	char msg[MAX_MSG];
	struct Ctl ctl;
};

#define CHAT_FIN 0x01
#define CHAT_HEART 0x02
#define CHAT_ACK 0x04
#define CHAT_WALL 0x08
#define CHAT_MSG 0x10
#define CHAT_FUNC 0x20
#define CHAT_SYS 0x40
struct ChatMsg{
    int type;
    char name[20];
    char msg[1024];
};
//court concerned
struct Map{
	int width;
	int height;
	struct Point start;
	int gate_width;
	int gate_height;
};
#endif
