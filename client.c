#include "my.h"

int login_flg = 0; 

int do_register(int fd, msg_t *mss)
{
	printf("Please enter username: ");
	fgets(mss->user_name, sizeof(mss->user_name), stdin);
	mss->user_name[strlen(mss->user_name)-1] = '\0';

	printf("Please enter password: ");
	fgets(mss->password, sizeof(mss->password), stdin);
	mss->password[strlen(mss->password)-1] = '\0';

	send(fd, mss, sizeof(msg_t), 0);

	recv(fd, mss, sizeof(msg_t), 0);
	printf("%s", mss->user_name);

	if(mss->type == SUCCESS)
		login_flg = 1;
	
	return mss->type;
}

int look_up(int fd, msg_t *mss)
{	
	char tmp[MAX] = {0};
	while(1)
	{
		memset(tmp, 0, sizeof(tmp));
		printf("Lookup for (or enter q to quit): ");
		fgets(tmp, sizeof(tmp), stdin);
		if(tmp[0] == 'q' || tmp[0] == '\0')
		{
			//printf("Bye!\n");
			break;
		}
		else
		{
			mss->type = QUERY;
			tmp[strlen(tmp)-1] = '\0';
			strcpy(mss->user_name, tmp);
		}

		send(fd, mss, sizeof(msg_t), 0);
		recv(fd, mss, sizeof(msg_t), 0);
		printf("%s", mss->user_name);
	}
	return 0;
}

void prompt_user()
{
	if(login_flg)
		printf("Please enter:			\
				\n 3 - to look up words	\
				\n 4 - to view history	\
				\n 5 - to log out		\
				\n 6 - to quit\n");
	else
		printf("Please enter:			\
				\n 1 - to register		\
				\n 2 - to log in		\
				\n 3 - to look up words	\
				\n 6 - to quit\n");
}

int get_history(int fd, msg_t *mss)
{
	send(fd, mss, sizeof(msg_t), 0);
	recv(fd, mss, sizeof(msg_t), 0);
	int count = mss->type;
	printf("%s", mss->user_name);
	//printf("count = %d", count);
	if(count <= 0)
		return -1;
	while(--count)
	{
		recv(fd, mss, sizeof(msg_t), 0);
		printf("%s", mss->user_name);
	}
	return 0;
}

int main(int argc, const char *argv[])
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd < 0)
	{
		perror("socket");
		return -1;
	}

	struct sockaddr_in myaddr, server_addr;
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(10072);
	myaddr.sin_addr.s_addr = htons(INADDR_ANY);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(10011);
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);

	if(bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0)
	{
		perror("bind");
		//return -1;
	}

	connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

	msg_t client_msg;
	memset(&client_msg, 0, sizeof(client_msg));

	while(1)
	{	
		prompt_user();
		scanf("%ld", &client_msg.type);
		getchar();

		if(client_msg.type == 6)
			break;
		switch(client_msg.type)
		{
		case 1: case 2:	
			do_register(sockfd, &client_msg);	
			break;
		case 3:			
			look_up(sockfd, &client_msg);
			break;
		case 4:	
			//send(sockfd, &client_msg, sizeof(msg_t), 0);
			get_history(sockfd, &client_msg);
			break;
		case 5:	
			login_flg = 0;
			send(sockfd, &client_msg, sizeof(msg_t), 0);
			break;
		default:		
			prompt_user();	
			break;
		}
	}

	close(sockfd);
	return 0;
}
