#include "../include/clibook.h"
int errnum=0;

int main(int argc, char const *argv[])
{
	int ret;
	if (3 != argc)
	{
		printf("Usage : %s <IP> <PORT>\n", argv[0]);
		exit(-1);
	}
	// 1. 创建套接字
	int sockfd = socket_connect(argv);

	MSG msg;
	memset(&msg, 0, sizeof(MSG));
	int n;
	while (1)
	{
		printf("=====================================\n");
		printf("=            1.管理员登录            =\n");
		printf("=            2.用户登录              =\n");
		printf("=            3.用户注册              =\n");
		printf("=            4.退出                  =\n");
		printf("======================================\n");
		printf("请输入选项: ");
		scanf("%d", &n);
		switch (n)
		{
		case 1:
			do_login(sockfd, &msg);
			if(msg.id==1)
			{
				goto next;
			}
			else{
				printf("该账号非管理员账号\n");
			}
			break;
		case 2:
			do_login(sockfd, &msg);
			if(msg.id!=1)
			{
				goto next2;
			}
			else{
				printf("该账号为管理员账号\n");
			}
			break;
		case 3:
			do_register(sockfd, &msg);
			break;
		case 4:
			exit(0);
			break;
		}
	}
next:
	while (1)
	{
		printf("=====================================\n");
		printf("=           1.添加图书               =\n");
		printf("=           2.修改图书               =\n");
		printf("=           3.查看所有图书           =\n");
		printf("=           4.退出             =\n");
		printf("=====================================\n");
		printf("请输入选项: ");
		scanf("%d", &n);
		switch (n)
		{
		case 1: //添加图书
			do_add(sockfd, &msg);
			break;
		case 2: //修改图书
			do_modify(sockfd, &msg);
			break;
		case 3: //查看所有图书
			all_query(sockfd, &msg);
			break;
		case 4:
			exit(-1);
			break;
		}
	}
next2:
	while (1)
	{
		printf("=====================================\n");
		printf("=           1.借阅图书               =\n");
		printf("=           2.查看所有图书           =\n");
		printf("=           3.退出             =\n");
		printf("=====================================\n");
		printf("请输入选项: ");
		scanf("%d", &n);
		switch (n)
		{
		case 1: //借阅图书（删除）
			do_delete(sockfd, &msg);
			break;
		case 2: //查看所有图书
			all_query(sockfd, &msg);
			break;
		case 3:
			exit(-1);
			break;
		}
	}
	close(sockfd);
	return 0;
}
// 创建套接字
int socket_connect(const char *argv[])
{

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd)
		ERRLOG("创建套接字失败");
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr)); 
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	socklen_t server_addr_len = sizeof(server_addr);
	//与服务器建立连接
	if (-1 == connect(sockfd, (struct sockaddr *)&server_addr, server_addr_len))
	{
		ERRLOG("listen error");
		exit(-1);
	}

	printf("---连接服务器成功---\n");
	return sockfd;
}

// 1.注册
void do_register(int sockfd, MSG *msg)
{
	memset(msg, 0, sizeof(MSG));
	msg->type = R;
	printf("输入ID: ");
	scanf("%d", &msg->id);
	printf("输入密码: ");
	scanf("%s", msg->pass);
	//发送数据
	if (0 >= send(sockfd, msg, sizeof(MSG), 0))
		ERRLOG("send error");
	//接收数据并输出
	if (0 >= recv(sockfd, msg, sizeof(MSG), 0))
		ERRLOG("send error");

	printf("%s\n", msg->data);
}

// 2.登录
int do_login(int sockfd, MSG *msg)
{
	char NO[32] = "n";
	char unlogin[128]={0};
	msg->type = L;
	printf("输入ID: ");
	scanf("%d", &msg->id);
	printf("输入密码: ");
	scanf("%s", msg->pass);
	//发送数据
	if (0 >= send(sockfd, msg, sizeof(MSG), 0))
		ERRLOG("send error ?");
	//接收数据并输出
	if (0 >= recv(sockfd, msg, sizeof(MSG), 0))
		ERRLOG("recv error");
	printf("%s\n", msg->data);
	sprintf(unlogin,"ID[%d]与密码[%s]不一致", msg->id,msg->pass);
	if (strcmp(msg->data,unlogin)== 0)
	{
		errnum++;
		if (3==errnum)
		{
			errnum=0;
			exit(0);
		}
	}

	if (strcmp(msg->data,"登陆成功") == 0)
	{
		printf("登陆成功");
	}
	return 0;
}

// 查询所有
void all_query(int sockfd, MSG *msg)
{
	memset(msg, 0, sizeof(MSG));
	int flag=0;
	char zoor[64]={0};
	sprintf(zoor,"ID[%d]为空", 0);
	msg->type = Qall;
	//发送数据
	if (0 >= send(sockfd, msg, sizeof(MSG), 0))
		ERRLOG("send error");
	while (1)
	{
		//接收数据并输出
		if (0 >= recv(sockfd, msg, sizeof(MSG), 0))
			ERRLOG("recv error");
		if (strcmp(msg->data, zoor) == 0)
		{
			printf("%s\n", msg->data);
		}
		if (strcmp(msg->data,"**over**") == 0)
		{
			printf("输出成功\n");
			break;
		}
		else
		{
			if (0==flag)
			{
				printf("ID| NAME | AUTHOR | NOTE |\n");
				flag=1;
			}
			printf("%d   %s    %s   %s\n", msg->id, msg->name, msg->author, msg->note);
			memset(msg, 0, sizeof(MSG));
		}
	}
	flag=0;
}

// 增加图书
void do_add(int sockfd, MSG *msg)
{
	memset(msg, 0, sizeof(MSG));
	msg->type = A;
	printf("输入图书ID:");
	scanf("%d", &msg->id);
	printf("输入图书名字:");
	scanf("%s", msg->name);
	printf("输入作者名字:");
	scanf("%s", msg->author);
	printf("输入图书备注:");
	scanf("%s", msg->note);
	//发送数据
	if (0 >= send(sockfd, msg, sizeof(MSG), 0))
		ERRLOG("send error");
	//接收数据并输出
	if (0 >= recv(sockfd, msg, sizeof(MSG), 0))
		ERRLOG("send error");

	printf("%s\n", msg->data);
}

//删除
void do_delete(int sockfd, MSG *msg)
{	
	printf("输入要借阅图书的ID: ");
	scanf("%d", &msg->id);
	msg->type = D;
	//发送数据
	if (0 >= send(sockfd, msg, sizeof(MSG), 0))
		ERRLOG("send error");
	//接收数据并输出
	if (0 >= recv(sockfd, msg, sizeof(MSG), 0))
		ERRLOG("recv error");
	printf("%s\n", msg->data);
}

// 修改
void do_modify(int sockfd, MSG *msg)
{
	printf("输入修改要修改的图书ID: ");
	scanf("%d", &msg->id);
	char NO[32] = "n";
	int flag=0;
	msg->type = M;
	printf("开始修改图书ID: [%d] 信息\n", msg->id);
	printf("修改图书名称(y/n): ");
	scanf("%s", NO);
	if (strcmp(NO, "y") == 0 || strcmp(NO, "Y") == 0)
	{
		memset(msg->name, 0, sizeof(msg->name));
		printf("新名称: ");
		scanf("%s", msg->name);
		flag=1;
	}
	printf("修改图书作者(y/n): ");
	scanf("%s", NO);
	if (strcmp(NO, "y") == 0 || strcmp(NO, "Y") == 0)
	{
		memset(msg->author, 0, sizeof(msg->author));
		printf("新作者: ");
		scanf("%s", msg->author);
		flag=1;
	}
	printf("修改图书备注(y/n): ");
	scanf("%s", NO);
	if (strcmp(NO, "y") == 0 || strcmp(NO, "Y") == 0)
	{
		memset(msg->note, 0, sizeof(msg->note));
		printf("新备注: ");
		scanf("%s", msg->note);
		flag=1;
	}
	if (0==flag)
	{
		printf("没有任何修改\n");
	}
	else
	{
		//发送数据
		if (0 >= send(sockfd, msg, sizeof(MSG), 0))
			ERRLOG("send error");
		//接收数据并输出
		if (0 >= recv(sockfd, msg, sizeof(MSG), 0))
			ERRLOG("recv error");

		if (strcmp(msg->data, "修改成功") == 0)
		{
			printf("%s\n", msg->data);
		}
	}
}
