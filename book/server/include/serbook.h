#ifndef __SERBOOK_H__
#define __SERBOOK_H__
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
#include <unistd.h>	// fork
#include <sys/wait.h>

/*数据库文件名字*/
#define FILEname "ser.db"

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

sqlite3 *proc_init(void);
int socket_bind_listen(const char *argv[]);
void child_do(int accept_fd, sqlite3 *sql_db);
void deal_signal(int s);
void do_register_s(int accept_fd, MSG *msg, sqlite3 *sql_db);
void do_login_s(int accept_fd, MSG *msg, sqlite3 *sql_db);
void all_query_s(int accept_fd,sqlite3 *sql_db);
void do_add_s(int accept_fd, MSG *msg, sqlite3 *sql_db);
void do_delete_s(int accept_fd, MSG *msg, sqlite3 *sql_db);
void do_modify_s(int accept_fd, MSG *msg, sqlite3 *sql_db);
#endif
