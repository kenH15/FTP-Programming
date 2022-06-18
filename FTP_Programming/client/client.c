#include <sys/socket.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include<sys/types.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include<stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
int FilePortNum;
char FileIP[8912];


void PASVgetIP(char*recvmsg, char*buf)
{
	int len = strlen(recvmsg);
	int flag = 0;
	int j = 0;
	for (int i = 0; i < len; i++)
	{
		if (flag == 1 && flag != 2)
		{
			buf[j] = recvmsg[i];
			j++;
		}
		if (recvmsg[i] == '(')
		{
			flag = 1;
		}
		else if (recvmsg[i] == ')')
		{
			flag = 2;
		}

	}
	buf[j - 1] = '\0';
}
int ConnectServer(int PortNum)
{
	int listenfd;
	struct sockaddr_in addr;
	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
	if (listenfd == -1) {
		return 1;
	}
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PortNum);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	if (listen(listenfd, 10) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
	return listenfd;

}
int ConnectClient(char*IP, int PortNum) //与Client创建连接
{
	int sockfd;
	struct sockaddr_in addr;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PortNum);
	if (inet_pton(AF_INET, IP, &addr.sin_addr) <= 0) {
		printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
	sleep(1);
	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		printf("Error connect(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	//printf("%s\n", IP);
	//printf("%d", PortNum);
	return sockfd;
}
int getPort(char *temp, char*FileIP)//获取端口信息
{
	int length = strlen(temp);
	int count = 0;
	int flag = 0;//判断IP还是PORT NUMBER
	char index1[10];//存第一个PORT NUMBER
	char index2[10];//存第二个PORT NUMBER
	int flag2 = 0;//判断第几个port number
	int i = 0, j = 0;
	for (i = 0; i < length; i++)
	{
		if (temp[i] == ',')
		{
			count++;
		}
	}
	if (count != 5)
	{
		printf("Wrong Port!\r\n");
		return -1;
	}
	count = 0;
	for (i = 0; i < length; i++)
	{

		if (flag == 0)//IP
		{
			if (temp[i] == ',')
			{
				if (count == 3)
				{
					FileIP[i] = '\0';
					flag = 1;
				}
				else
				{
					FileIP[i] = '.';
					count++;
				}
			}
			else
			{
				FileIP[i] = temp[i];
			}
		}
		else if (flag == 1)//Number
		{
			if (temp[i] == ',')
			{
				index1[j] = '\0';
				flag2 = 1;
				j = 0;
			}
			else
			{
				if (flag2 == 0)
				{
					count++;
					index1[j] = temp[i];
					j++;
				}
				else
				{
					index2[j] = temp[i];
					j++;
				}
			}
		}
	}
	int num1 = strtol(index1, NULL, 10);
	int num2 = strtol(index2, NULL, 10);
	int result = num1 * 256 + num2;
	//FilePortNum = result;
	//sprintf(FilePortNum, "%d", result);
	return result;
}
int main(int argc, char **argv)
{
	int  PORT = 21;
	char DefaultIP[100] = "127.0.0.1";
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-port") == 0)
		{
			if (i < (argc - 1))
			{
				strcpy(DefaultIP, argv[i + 1]);
			}
		}
		else if (strcmp(argv[i], ":") == 0)
		{
			PORT = atoi(argv[i + 1]);
		}

	}

	int sockfd, listenfd;
	struct sockaddr_in addr;
	char sentence[8192];
	char recvMsg[8192];
	char recvMsg2[8192];
	int len;
	int p;
	int Mode = 0;
	int connfd = 0;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	if (inet_pton(AF_INET, DefaultIP, &addr.sin_addr) <= 0) {
		printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		printf("Error connect(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	//接受初始消息
	recv(sockfd, recvMsg, sizeof(recvMsg), 0);
	printf("FROM SERVER:%s", recvMsg);
	memset(recvMsg, 0, sizeof(recvMsg));

	while (1)
	{
		fgets(sentence, 4096, stdin);
		len = strlen(sentence);
		sentence[len - 1] = '\0';
		strcat(sentence, "\r\n");
		//printf("%s", sentence);

		char prefix[10] = "\0";
		char prefix2[10] = "\0";
		strncpy(prefix, sentence, 5);
		strncpy(prefix2, sentence, 4);
		char sign[10] = "\0";
		if (strcmp(prefix2, "QUIT") != 0 && strcmp(prefix2, "ABOR") != 0)
		{
			send(sockfd, sentence, len + 1, 0);
			recv(sockfd, recvMsg, sizeof(recvMsg), 0);
			printf("FROM SERVER %s\n", recvMsg);
			strncpy(sign, recvMsg, 3);

		}
		else if (strcmp(prefix2, "QUIT") == 0 || strcmp(prefix2, "ABOR") == 0)
		{
			send(sockfd, sentence, len + 1, 0);
			recv(sockfd, recvMsg, sizeof(recvMsg), 0);
			printf("FROM SERVER %s\n", recvMsg);
			strncpy(sign, recvMsg, 3);
			if (strcmp(sign, "221") == 0)
			{
				break;
			}
		}
		if (strcmp(prefix, "USER ") == 0)//输入USER anonymous
		{
			if (strcmp(sign, "331") == 0)
			{
				Mode = 1;
			}
		}
		else if (strcmp(prefix, "PASS ") == 0)
		{
			if (Mode == 1);
			{
				if (strcmp(sign, "230") == 0)
				{
					Mode = 2;
				}
			}
		}

		else if (strcmp(prefix, "PORT ") == 0)
		{
			if (Mode >= 2)
			{
				if (strcmp(sign, "200") == 0)
				{
					char temp[100];
					strncpy(temp, sentence + 5, strlen(sentence) - 5);
					temp[strlen(sentence) - 7] = '\0';
					FilePortNum = getPort(temp, FileIP);
					//printf("%s\n", FileIP);
					//printf("%d\n", FilePortNum);
					Mode = 3;//PORT
				}
			}
		}
		else if (strcmp(prefix2, "PASV") == 0)
		{
			if (Mode >= 2)
			{
				if (strcmp(sign, "227") == 0)
				{
					char temp[100];
					PASVgetIP(recvMsg, temp);
					//printf("%s\r\n", temp);
					FilePortNum = getPort(temp, FileIP);
					//printf("%s\n", FileIP);
					//printf("%d\n", FilePortNum);
					connfd = ConnectClient(FileIP, FilePortNum);
					Mode = 4;
				}
			}
		}

		else if (strcmp(prefix, "STOR ") == 0)
		{
			if (Mode >= 2)
			{
				//printf("%d\n", Mode);
				if (Mode == 3)
				{
					listenfd = ConnectServer(FilePortNum);

				}
				char fileMsg[8192] = "\0";
				if (Mode == 3 || Mode == 4)
				{

					char fileName[100];
					strncpy(fileName, sentence + 5, strlen(sentence) - 5);
					fileName[strlen(sentence) - 7] = '\0';//文件名
														  //printf("%s\r\n", fileName);
					FILE *fp = fopen(fileName, "rb");
					if (fp == NULL)
					{
						printf("File dose not exist\r\n");
					}
					else
					{
						strncpy(sign, recvMsg, 3);
						if (Mode == 3)//若为PORT模式发送的
						{
							connfd = accept(listenfd, NULL, NULL);
							if (connfd == -1)
							{
								printf("Error accept(): %s(%d)\n", strerror(errno), errno);
								continue;
							}
						}
						if (strcmp(sign, "150") == 0)
						{
							while (!feof(fp))
							{
								int len = fread(fileMsg, sizeof(char), sizeof(fileMsg), fp);
								if (len > 0)
								{
									int n = send(connfd, fileMsg, len, 0);
									if (n < 0)
									{
										printf("450 Failed to send file\r\n");
									}
									memset(fileMsg, 0, sizeof(fileMsg));
								}
							}
							fclose(fp);
							close(connfd);
							connfd = 0;
							if (Mode == 3)
							{
								close(listenfd);
								listenfd = 0;
							}
							recv(sockfd, recvMsg2, sizeof(recvMsg2), 0);
							printf("FROM SERVER %s\r\n", recvMsg2);
							memset(recvMsg2, 0, sizeof(recvMsg2));
							Mode = 2;
						}
					}
				}
			}
		}
		else if (strcmp(prefix, "RETR ") == 0)
		{

			if (Mode >= 2)
			{
				if (strcmp(sign, "150") == 0)
				{
					if (Mode == 3)
					{
						listenfd = ConnectServer(FilePortNum);
						//printf("%d\n", listenfd);
					}
					char fileMsg[8192] = "\0";
					if (Mode == 3 || Mode == 4)
					{
						if (Mode == 3)
						{
							connfd = accept(listenfd, NULL, NULL);
							if (connfd == -1)
							{
								printf("Error accept(): %s(%d)\n", strerror(errno), errno);
								continue;
							}
						}
						char fileName[100];
						strncpy(fileName, sentence + 5, strlen(sentence) - 5);
						fileName[strlen(sentence) - 7] = '\0';//文件名
						FILE *fp = fopen(fileName, "wb");
						if (fp == NULL)
						{
							printf("File dose not exist\r\n");
						}
						while (1)
						{
							int n = recv(connfd, fileMsg, sizeof(fileMsg), 0);
							if (n <= 0)
							{

								break;
							}
							fwrite(fileMsg, sizeof(char), n, fp);

						}
						fclose(fp);
						close(connfd);
						if (Mode == 3)
						{
							close(listenfd);
							connfd = 0;
						}
						memset(recvMsg2, 0, sizeof(recvMsg2));
						recv(sockfd, recvMsg2, sizeof(recvMsg2), 0);
						printf("FROM SERVER%s\r\n", recvMsg2);
						Mode = 2;
					}
				}
			}
		}

		memset(sentence, 0, sizeof(sentence));
		memset(recvMsg, 0, sizeof(recvMsg));
		memset(recvMsg2, 0, sizeof(recvMsg2));
	}
	//printf("FROM SERVER: %s", sentence);

	close(sockfd);

	return 0;
}
