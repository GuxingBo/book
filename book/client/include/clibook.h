#ifndef __CLIBOOK_H__
#define __CLIBOOK_H__
#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>     // memset
#include <sys/types.h>  
#include <sys/socket.h>
#include <netinet/in.h> // 信息结构体struct sockaddr_in
#include <signal.h>
#include <netinet/in.h>	//inet_aton
#include <arpa/inet.h>
#include <unistd.h>	
#define ERRLOG(errmsg)                                       \
	do                                                       \
	{                                                        \
		printf("%s--%s(%d):", __FILE__, __func__, __LINE__); \
		perror(errmsg);                                      \
	} while (0)

#define R 1 //  注册
#define L 2 //  登录
#define M 4 //  修改
#define A 5 //  增加
#define D 6 //  删除
#define Qall 7 // 查询所有

#define N 32


//信息结构体
typedef struct
{
	int type;		//选择
	int id; //用户ID
	char name[64];	//图书名字
	char pass[64]; // password密码
	char data[256]; //emark返回数据
	char author[64]; //图书作者
	char note[256]; //图书备注
}MSG;
int socket_connect(const char *argv[]);
void do_register(int sockfd, MSG *msg);
int do_login(int sockfd, MSG *msg);
void all_query(int sockfd, MSG *msg);
void do_add(int sockfd, MSG *msg);
void do_delete(int sockfd, MSG *msg);
void do_modify(int sockfd, MSG *msg);
#endif
