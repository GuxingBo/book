#include "../include/serbook.h"
int main(int argc, char const *argv[])
{
	//数据库初始化
	sqlite3 *sql_db=proc_init();
	if (3 != argc)
	{
		printf("Usage : %s <IP> <PORT>\n", argv[0]);
		exit(-1);
	}
	//创建套接字
	int sockfd = socket_bind_listen(argv);

	//客户端信息的结构体
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	socklen_t client_addr_len = sizeof(client_addr);

	pid_t pid;
	int accept_fd;

	while (1)
	{
		accept_fd=accept(sockfd,(struct sockaddr *)&client_addr, &client_addr_len);
		printf("客户端[%s : %d]已连接\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));


		if (0>(pid=fork())) ERRLOG("fork error");

		else if (0==pid)
		{
			child_do(accept_fd,sql_db);
		}
		else
		{


			if (signal(SIGCHLD, deal_signal) == SIG_ERR) 
				ERRLOG("signal error");
			close(accept_fd);
		}
	}

	//关闭监听套接字
	close(sockfd);
	return 0;
}

// 数据库初始化
sqlite3 *proc_init(void)
{
	//打开数据库文件
	sqlite3 *sql_db=NULL;
	int ret=sqlite3_open(FILEname,&sql_db);
	if (ret!=SQLITE_OK)
	{
		printf("打开数据库文件 失败");
		printf("errcode[%d]  errmsg[%s]\n", ret, sqlite3_errmsg(sql_db));
		exit(-1);
	}
	//2建表

	char sql_rbuf[256]="CREATE TABLE IF NOT EXISTS user(id INT PRIMARY KEY, pass TEXT)";
	ret=sqlite3_exec(sql_db,sql_rbuf,NULL,NULL,NULL);
	if (ret!=SQLITE_OK)
	{
		perror("建user表 失败");
		printf("errcode[%d]  errmsg[%s]\n", ret, sqlite3_errmsg(sql_db));
		exit(-1);
	}
	//图书ID-图书名字-图书作者-图书备注
	char sql_ubuf[256]="CREATE TABLE IF NOT EXISTS book(id INT PRIMARY KEY,name TEXT,author TEXT,note TEXT)";
	ret=sqlite3_exec(sql_db,sql_ubuf,NULL,NULL,NULL);
	if (ret!=SQLITE_OK)
	{
		perror("建book表 失败");
		printf("errcode[%d]  errmsg[%s]\n", ret, sqlite3_errmsg(sql_db));
		exit(-1);
	}

	return sql_db;
}

//3. 创建套接字
int socket_bind_listen(const char *argv[])
{

	int sockfd=socket(AF_INET,SOCK_STREAM,0);
	if (-1==sockfd)	ERRLOG("创建套接字 失败"); 


	struct sockaddr_in server_addr;
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family=AF_INET;			

	server_addr.sin_port=htons(atoi(argv[2]));

	server_addr.sin_addr.s_addr=inet_addr(argv[1]);
	socklen_t server_addr_len=sizeof(server_addr);

	if (-1==bind(sockfd,(struct sockaddr *)&server_addr,server_addr_len))
		ERRLOG("sockaddr error"); 


	if (-1 == listen(sockfd, 10))
		ERRLOG("listen error");

	return sockfd;
}

void child_do(int accept_fd, sqlite3 *sql_db)
{
	MSG msg;
	memset(&msg,0,sizeof(MSG));
	while (recv(accept_fd, &msg, sizeof(MSG), 0) > 0) 
	{
		printf("type = %d\n", msg.type); 
		switch (msg.type)
		{
		case A: //增加
			do_add_s(accept_fd, &msg, sql_db);
			break;
		case D: //删除
			do_delete_s(accept_fd, &msg, sql_db);
			break;
		case M: //修改
			do_modify_s(accept_fd, &msg, sql_db);
			break;
		case R: // 注册
			do_register_s(accept_fd, &msg, sql_db);
			break;
		case L: // 登录
			do_login_s(accept_fd, &msg, sql_db);
			break;
		case Qall: // 查询所有
			all_query_s(accept_fd,sql_db);
			break;
		}
	}
	printf("客户端 退出\n");
	exit(0);
}

// 信号处理函数
void deal_signal(int s)
{
	
	wait(NULL); 
}

//  注册
void do_register_s(int accept_fd, MSG *msg, sqlite3 *sql_db)
{
	char sql_buf[512];
	memset(sql_buf,0,sizeof(sql_buf));
	sprintf(sql_buf,"INSERT INTO user(id,pass) VALUES(%d,'%s')", msg->id, msg->pass);
	int ret=sqlite3_exec(sql_db,sql_buf,NULL,NULL,NULL);
	if (ret!=SQLITE_OK)
	{
		perror("注册失败");
		printf("errcode[%d]  errmsg[%s]\n", ret, sqlite3_errmsg(sql_db));
		sprintf(msg->data, "ID %d 已经存在", msg->id);
	}
	else
	{
		strcpy(msg->data, "注册成功");
	}
	//发送数据
	if (0 >= send(accept_fd,msg, sizeof(MSG), 0))
		ERRLOG("send error");
}

//登陆
void do_login_s(int accept_fd, MSG *msg, sqlite3 *sql_db)
{
	char sql_buf[256]={0};
	//使用查询函数判断是否存在
	sprintf(sql_buf,"SELECT * FROM user WHERE id=%d AND pass='%s'", msg->id, msg->pass);

	char **result;	
	int row = 0;	
	int column = 0; 
	int ret=sqlite3_get_table(sql_db,sql_buf,&result,&row,&column,NULL);
	if (ret!=SQLITE_OK)
	{
		perror("查询失败");
		printf("errcode[%d]  errmsg[%s]\n", ret, sqlite3_errmsg(sql_db));
	}
	if (0==row)
	{
		sprintf(msg->data,"ID[%d]与密码[%s]不一致", msg->id,msg->pass);
	}
	else
	{
		strcpy(msg->data, "登录成功");
	}
	//发送数据
	if (0 >= send(accept_fd, msg, sizeof(MSG), 0))
		ERRLOG("send error");
}

// 查询所有
void all_query_s(int accept_fd,sqlite3 *sql_db)
{
	MSG msg;
	int i,x;
	char sql_buf[256]={0};
	memset(&msg,0,sizeof(msg));
	//使用查询函数
	strcpy(sql_buf,"SELECT * FROM book");
	char **result;	
	int row = 0;	
	int column = 0; 
	int ret=sqlite3_get_table(sql_db,sql_buf,&result,&row,&column,NULL);
	if (ret!=SQLITE_OK)
	{
		perror("查询失败");
		printf("errcode[%d]  errmsg[%s]\n", ret, sqlite3_errmsg(sql_db));
	}
	if (0==row)
	{
		sprintf(msg.data,"ID[%d]为空", 0);
	}
	else
	{
		x=column;
		for (i= 0; i<row; i++)
		{
			if (result[x])
			{
				msg.id=atoi(result[x]);
			}
		
			x++;
			snprintf(msg.name,sizeof(msg.name),"%s",result[x++]);
			snprintf(msg.author,sizeof(msg.author),"%s",result[x++]);
			snprintf(msg.note,sizeof(msg.note),"%s",result[x++]);
			//发送数据
			if (0 >= send(accept_fd,&msg, sizeof(MSG), 0))
				ERRLOG("send error");
			memset(&msg,0,sizeof(msg));
			if (row==(i+1))
			{
				strcpy(msg.data,"**over**");
				//发送数据
				if (0 >= send(accept_fd,&msg, sizeof(MSG), 0))
					ERRLOG("send error");
			}				
		}	
	}
	sqlite3_free_table(result);
}

// 增加
void do_add_s(int accept_fd, MSG *msg, sqlite3 *sql_db)
{
	char sql_buf[512];
	memset(sql_buf,0,sizeof(sql_buf));
	sprintf(sql_buf,"INSERT INTO book(id,name,author,note) VALUES(%d,'%s','%s','%s')", msg->id, msg->name,msg->author,msg->note);
	int ret=sqlite3_exec(sql_db,sql_buf,NULL,NULL,NULL);
	if (ret!=SQLITE_OK)
	{
		perror("添加失败");
		printf("errcode[%d]  errmsg[%s]\n", ret, sqlite3_errmsg(sql_db));
		sprintf(msg->data, "ID为 %d 的图书已经存在", msg->id);
	}
	else
	{
		strcpy(msg->data, "添加成功");
	}
	//发送数据
	if (0 >= send(accept_fd,msg, sizeof(MSG), 0))
		ERRLOG("send error");
}

//删除
void do_delete_s(int accept_fd, MSG *msg, sqlite3 *sql_db)
{
	char sql_buf[256]={0};
	//填写sql语句
	sprintf(sql_buf,"DELETE FROM book WHERE id=%d",msg->id);
	//执行sql语句
	int ret = sqlite3_exec(sql_db, sql_buf, NULL, NULL, NULL);
	if (ret!=SQLITE_OK)
	{
		perror("查询失败");
		printf("errcode[%d]  errmsg[%s]\n", ret, sqlite3_errmsg(sql_db));
	}
	else
	{
		strcpy(msg->data, "借阅成功");
	}
	//发送数据
	if (0 >= send(accept_fd, msg, sizeof(MSG), 0))
		ERRLOG("send error");
	memset(msg,0,sizeof(MSG));
}

// 修改
void do_modify_s(int accept_fd, MSG *msg, sqlite3 *sql_db)
{
	char sql_buff[512] = {0};
	// pass ,name ,sex ,age,pay,job
	sprintf(sql_buff,"UPDATE book SET name='%s',author='%s',note='%s' WHERE id=%d",
			msg->name,msg->author,msg->note,msg->id);
	//执行sql语句
	int ret = sqlite3_exec(sql_db, sql_buff, NULL, NULL, NULL);
	if (ret != SQLITE_OK)
	{
		perror("修改失败");
		printf("返回值[%d]  错误信息[%s]\n", ret, sqlite3_errmsg(sql_db));
		exit(-1);
	}
	strcpy(msg->data, "修改成功");
	//发送数据
	if (0 >= send(accept_fd,msg, sizeof(MSG), 0))
		ERRLOG("send error");
	memset(msg,0,sizeof(MSG));
}
