#include "my.h"


struct sockaddr_in myaddr, client_addr;
int login_flg = 0;

void deal_register(int fd, msg_t *mss, char *usr)
{
	// printf("register received: %ld, %s, %s\n", mss->type, mss->user_name, mss->password);
	char respond[MAX] = {0};

	sqlite3 *db;
	if(sqlite3_open(DB, &db) != SQLITE_OK)
	{
		memset(mss, 0, sizeof(msg_t));
		strcpy(mss->user_name, "Error open database\n");
		mss->type = FAIL;
		send(fd, mss, sizeof(msg_t), 0);
		return;
	}

	char sql[MAX] = {0};
	char **resultp, *errmsg;
	int nrow, ncol;
	sprintf(sql,"select * from user_info where username = '%s';", mss->user_name);
	sqlite3_get_table(db, sql, &resultp, &nrow, &ncol, &errmsg );
	if(nrow > 0)
	{
		// user already exits
		memset(mss, 0, sizeof(msg_t));
		strcpy(mss->user_name, "Username already exits!\n");
		mss->type = FAIL;
		send(fd, mss, sizeof(msg_t), 0);
	}
	else
	{
		// insert user
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "insert into user_info values('%s', '%s');", \
				mss->user_name, mss->password);
		//printf("sql: %s\n", sql);
		if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
		{
			printf("error: %s\n", errmsg);
			return;
		}

		strcpy(usr, mss->user_name);
		login_flg = 1;
		sprintf(respond, "Registered successfully\nHello %s!\n", mss->user_name);
		memset(mss, 0, sizeof(msg_t));
		strcpy(mss->user_name, respond);
		//printf("register respond: %s\n", respond);
		send(fd, mss, sizeof(msg_t), 0);
	}

	sqlite3_close(db);
	return;
}

void deal_login(int fd, msg_t *mss, char *usr)
{
	printf("login received: %ld, %s, %s\n", mss->type, mss->user_name, mss->password);
	char respond[MAX] = {0};

	sqlite3 *db;
	if(sqlite3_open(DB, &db) != SQLITE_OK)
	{
		memset(mss, 0, sizeof(msg_t));
		strcpy(mss->user_name, "Error open database\n");
		mss->type = FAIL;
		send(fd, mss, sizeof(msg_t), 0);
		return;
	}

	char sql[MAX] = {0};
	char **resultp, *errmsg;
	int nrow, ncol;
	sprintf(sql,"select * from user_info where username = '%s';", mss->user_name);
	sqlite3_get_table(db, sql, &resultp, &nrow, &ncol, &errmsg );
	if (nrow == 0)
	{
		// user not exist
		memset(mss, 0, sizeof(msg_t));
		strcpy(mss->user_name, "User does not exits!\n");
		mss->type = FAIL;
		send(fd, mss, sizeof(msg_t), 0);
	}
	else if(nrow == 1)
	{
		//printf("%s | %s\n", resultp[0], resultp[1]);
		//printf("%s | %s\n", resultp[2], resultp[3]);
		if(strcmp(resultp[3], mss->password))
		{
			// password does not match	
			memset(mss, 0, sizeof(msg_t));
			strcpy(mss->user_name, "Wrong password!\n");
			mss->type = FAIL;
			send(fd, mss, sizeof(msg_t), 0);
		}
		else
		{
			//login successfully
			login_flg = 1;
			strcpy(usr, mss->user_name);
			sprintf(respond, "Hello %s!\n", mss->user_name);
			memset(mss, 0, sizeof(msg_t));
			strcpy(mss->user_name, respond);
			send(fd, mss, sizeof(msg_t), 0);
		}
	}

	sqlite3_close(db);
	return;
}

char *get_time(char *timestamp, int size)
{
	time_t t;
	int sec = time(&t);
	struct tm *lt = localtime(&t);
	memset(timestamp, 0, size);
	strftime(timestamp, 24, "%Y-%m-%d %H:%M:%S", lt);
	printf("%s\n", timestamp);
	return timestamp;
}

void add_history(char *buf, char *usr)
{
	printf("add_history\n");
	sqlite3 *db;
	if(sqlite3_open(DB, &db) != SQLITE_OK)
	{
		printf("error open db\n");
		return;
	}
	
	printf("success open db\n");

	char sql[MAX];
	char *errmsg;
	char timestamp[24];
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "insert into history values('%s', '%s', '%s');", \
			usr, buf, get_time(timestamp, 24));
	printf("sql: %s\n", sql);
	if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		printf("error: %s\n", errmsg);
		return;
	}
	sqlite3_close(db);
}

void deal_query(int fd, msg_t *mss, char *usr)
{
	printf("deal_query\n");
	FILE *fp = fopen("./dict.txt", "r");
	if(NULL == fp)
	{
		memset(mss, 0, sizeof(msg_t));
		strcmp(mss->user_name, strerror(errno));
		send(fd, &mss, sizeof(mss), 0);
		return;
	}

	char buf[MAX];
	while(fgets(buf, sizeof(buf), fp))
	{
		int cmp = strncmp(buf, mss->user_name, strlen(mss->user_name));
		if(!cmp && buf[strlen(mss->user_name)] == ' ')
		{
			memset(mss, 0, sizeof(msg_t));
			strcpy(mss->user_name, buf);
			send(fd, mss, sizeof(msg_t), 0);
			//printf("send: %ld, %s, %s\n", mss->type, mss->user_name, mss->password);
			add_history(buf, usr);
			break;
		}
		if(cmp > 0 || cmp == 0 && buf[strlen(mss->user_name)] != ' ')
		{
			memset(mss, 0, sizeof(msg_t));
			strcpy(mss->user_name, "Not found\n");
			mss->type = FAIL;
			send(fd, mss, sizeof(msg_t), 0);
			//printf("send: %ld, %s, %s\n", mss->type, mss->user_name, mss->password);
			break;
		}
	}
	fclose(fp);
	return;
}

void deal_history(int fd, msg_t *mss, char *usr)
{
	printf("deal_history %s\n", usr);
	if(login_flg)
	{
		sqlite3 *db;
		if(sqlite3_open(DB, &db) != SQLITE_OK)
		{
			memset(mss, 0, sizeof(msg_t));
			strcpy(mss->user_name, "Error open database\n");
			mss->type = FAIL;
			send(fd, mss, sizeof(msg_t), 0);
			return;
		}

		printf("success open db\n");

		char sql[MAX] = {0};
		char **resultp, *errmsg;
		int nrow, ncol;
		sprintf(sql,"select * from history where username = '%s';", usr);
		printf("sql: %s\n", sql);
		sqlite3_get_table(db, sql, &resultp, &nrow, &ncol, &errmsg );
		
		memset(mss, 0, sizeof(msg_t));
		mss->type = nrow;
		if (nrow == 0)
		{
			// no history
			strcpy(mss->user_name, "History not found!\n");
			send(fd, mss, sizeof(msg_t), 0);
			return;
		}

		int i;
		int index = ncol;
		char tmp[MAX] = {0};
		printf("nrow = %d, ncol = %d\n", nrow, ncol);
		for(i = 0; i < nrow; i++)
		{
			//printf("HISTORY: %s  %s\n", resultp[index+2], resultp[index+1]);
			sprintf(tmp, "%s %s", resultp[index+2], resultp[index+1]);
			//printf("%s", tmp);
			memset(mss, 0, sizeof(msg_t));
			strcpy(mss->user_name, tmp);
			mss->type = nrow;
			send(fd, mss, sizeof(msg_t), 0);
			index += ncol;
		}
		sqlite3_close(db);
	}
	else
	{
		// not logged in
		memset(mss, 0, sizeof(msg_t));
		strcpy(mss->user_name, "Please login\n");
		mss->type = FAIL;
		send(fd, mss, sizeof(msg_t), 0);
	}
	printf("end history\n");
	return;
}

void deal_logout(char *usr)
{
	printf("%s log out\n", usr);
	login_flg = 0;
	memset(usr, 0, MAX);
}

void *deal_fun(void *p)
{
	int connfd = *(int *)p;
	
	msg_t client_msg;
	char usr[MAX] = {0};

	while(1)
	{
		if(recv(connfd, &client_msg, sizeof(client_msg), 0) <= 0)
		{
			printf("Client %d quit\n", connfd);
			close(connfd);
			pthread_exit(NULL);
		}
		printf("deal_fun: received from %s: %ld, %s, %s\n", usr, client_msg.type, client_msg.user_name, client_msg.password);
		switch(client_msg.type)
		{
		case REGISTER:	deal_register(connfd, &client_msg, usr);		break;
		case LOGIN:		deal_login(connfd, &client_msg, usr);		break;
		case QUERY:		deal_query(connfd, &client_msg, usr);		break;
		case HISTORY:	deal_history(connfd, &client_msg, usr);		break;
		case LOGOUT:	deal_logout(usr);					break;
		default:		break; 
		}
	}
	return NULL;
}

int main(int argc, const char *argv[])
{
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);

	if(listenfd < 0)
	{
		perror("socket");
		return -1;
	}

	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(10011);
	myaddr.sin_addr.s_addr = htons(INADDR_ANY);

	if(bind(listenfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0)
	{
		perror("bind");
		return -1;
	}

	listen(listenfd, 10);

	socklen_t len = sizeof(myaddr);

	while(1)
	{
		int connfd = accept(listenfd, (struct sockaddr *)&client_addr, &len);
		if(connfd < 0)
		{
			perror("accept");
			return -1;
		}

		pthread_t id;
		pthread_create(&id, NULL, deal_fun, &connfd);
	}

	close(listenfd);
	return 0;
}
