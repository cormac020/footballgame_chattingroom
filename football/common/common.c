#include "head.h"
extern char *conf_ans;
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

	//int reuse = 1;
	//setsockopt(sockfd, SOL_SOCKET, SO_RESUEADDR, (char *)&reuse, sizeof(int));

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

char *get_conf_value(const char *path, const char *key){
	FILE *fp = NULL;
	ssize_t nchar;
	char *line = NULL;
	size_t len = 0;

	if(path == NULL || key == NULL){
		fprintf(stderr, "Invalid argument!\n");
		return NULL;
	}
	if((fp = fopen(path, "r")) == NULL){
		perror("fopen()");
		return NULL;
	}
	char *ptr = NULL;
	/*while(1){
		if((nchar = getline(&line, &len, fp)) < 0)
		{
			perror("getline()");
			return NULL;
		}
		if(strstr(line, key) == NULL) continue;
		if((ptr = strstr(line, "=")) == NULL){
			perror("strstr()");
			return NULL;
		}
		ptr++;
		static char ret_val[512];
		strncpy(ret_val, ptr, nchar - strlen(key) - 1);
		return ret_val;
	}*/
	static char ans[50];
	while((nchar = getline(&line, &len, fp)) != -1){
		if((ptr = strstr(line, key)) == NULL) continue;

		if(line[strlen(key)] == '='){
			//printf("before: %d\n", nchar - (int)strlen(key) - 2);
			strncpy(ans, ptr + strlen(key) + 1, nchar - strlen(key)- 2);
			//printf("after\n");
			break;
		}
	}
	free(line);
	fclose(fp);
	//memset(ans, '\0', sizeof(ans));
	ans[nchar - strlen(key) - 2] = '\0';
	if(ptr == NULL)
		return NULL;
	return ans;
}
