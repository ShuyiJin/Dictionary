#ifndef _MY_H
#define _MY_H 1

#include <stdio.h>
#include <sqlite3.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define REGISTER	1
#define LOGIN		2
#define QUERY		3
#define HISTORY		4
#define LOGOUT		5
#define EXIT		6

#define SUCCESS	0
#define FAIL	-1

#define MAX 512

#define DB "./my.db"

typedef struct
{
	long type;
	char user_name[MAX];
	char password[32];
}msg_t;

#endif /* my.h */
