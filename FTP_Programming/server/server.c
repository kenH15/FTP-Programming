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
#include <sys/select.h>
int clientStatus[100] = { 0 };//客户端状态
int PortNumber[100] = { 0 };//端口数字
char PortIP[100][50];
//char fileName[100][100];//文件名
char PATH[100] = "/tmp";
int clientfd2[100] = { 0 };
int filefd[100] = { 0 };
int pasvfd[100] = { 0 };
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
int ConnectSever(int PortNum)
{
	int listenfd;
	struct sockaddr_in addr;
	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
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
int getPort(char *temp, int clientfd)//获取端口信息
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
					PortIP[clientfd][i] = '\0';
					flag = 1;
				}
				else
				{
					PortIP[clientfd][i] = '.';
					count++;
				}

			}
			else
			{
				PortIP[clientfd][i] = temp[i];
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
	PortNumber[clientfd] = result;
	//sprintf(PortNumber[clientfd],"%d",result);//PORT NUMBER
	return 0;
}
void SentenceModify(char *sentence)//去掉末尾换行
{
	int len = strlen(sentence);
	char temp[8192];
	int j = 0;
	for (int i = 0; i < len; i++)
	{
		if (sentence[i] == '\r' || sentence[i] == '\n')
			continue;
		temp[j] = sentence[i];
		j++;
	}
	temp[j] = '\0';
	strcpy(sentence, temp);
}
void ClientHandle(int clientfd, char *sentence)
{
	//printf("STATUS\r\n");
	//printf("%d\r\n", clientStatus[clientfd]);
	//printf("%s", sentence);
	char prefix[10] = "\0";//sentence前缀
	char prefix2[10] = "\0";
	strncpy(prefix2, sentence, 4);
	if (strcmp(prefix2, "QUIT") == 0 || strcmp(prefix2, "ABOR") == 0)
	{
		char sendMsg[] = "221 Goodbye.\r\n";
		send(clientfd, sendMsg, strlen(sendMsg), 0);
		close(clientfd);
		return;
	}
	if (clientStatus[clientfd] == 0)//未登录
	{
		if (strcmp(sentence, "USER anonymous") == 0)
		{
			char sendMsg[] = "331 Guest login ok, send your complete e - mail address as password.\r\n";
			clientStatus[clientfd] = 1;
			send(clientfd, sendMsg, strlen(sendMsg), 0);
		}
		else
		{
			char sendMsg[] = "503 Your first request must be 'USER anonymous'.\r\n";
			send(clientfd, sendMsg, strlen(sendMsg), 0);
		}
		return;
	}
	if (clientStatus[clientfd] == 1)
	{
		strncpy(prefix, sentence, 5);
		if (strcmp(prefix, "PASS ") == 0) {
			char sendMsg[] = "230 Guest login ok, access restrictions apply.\r\n";
			send(clientfd, sendMsg, strlen(sendMsg), 0);
			clientStatus[clientfd] = 2;
		}
		else
		{
			char sendMsg[] = "503 You should login by inputing 'PASS password'.\r\n";
			send(clientfd, sendMsg, strlen(sendMsg), 0);
		}
		return;
	}
	if (clientStatus[clientfd] == 2)
	{
		strncpy(prefix, sentence, 4);
		strncpy(prefix2, sentence, 5);
		if (strcmp(sentence, "SYST") == 0)
		{
			char sendMsg[] = "215 UNIX Type: L8\r\n";
			send(clientfd, sendMsg, strlen(sendMsg), 0);
		}
		else if (strcmp(prefix, "TYPE") == 0)
		{
			if (strcmp(sentence, "TYPE I") == 0)
			{
				char sendMsg[] = "200 Type set to I.\r\n";
				send(clientfd, sendMsg, strlen(sendMsg), 0);
			}
			else
			{
				char sendMsg[] = "503 Error Type.\r\n";
				send(clientfd, sendMsg, strlen(sendMsg), 0);
			}
		}
		else if (strcmp(prefix, "MKD ") == 0)
		{
			//printf("MKD\r\n");
			char temp[100];
			strncpy(temp, sentence + 4, strlen(sentence) - 4);
			temp[strlen(sentence) - 4] = '\0';
			int n = mkdir(temp, S_IRWXU);
			int len = strlen(temp);
			temp[len] = '\r';
			temp[len + 1] = '\n';
			temp[len + 2] = '\0';
			send(clientfd, temp, strlen(temp), 0);
		}
		else if (strcmp(prefix, "CWD ") == 0)
		{
			//printf("CWD\r\n");
			char temp[100];
			strncpy(temp, sentence + 4, strlen(sentence) - 4);
			temp[strlen(sentence) - 4] = '\0';
			chdir(temp);
			int len = strlen(temp);
			temp[len] = '\r';
			temp[len + 1] = '\n';
			temp[len + 2] = '\0';
			//printf("%s", temp);
			send(clientfd, temp, strlen(temp), 0);
		}
		else if (strcmp(prefix, "RMD ") == 0)
		{
			char temp[100];
			strncpy(temp, sentence + 4, strlen(sentence) - 4);
			temp[strlen(sentence) - 4] = '\0';
			rmdir(temp);
			int len = strlen(temp);
			temp[len] = '\r';
			temp[len + 1] = '\n';
			temp[len + 2] = '\0';
			send(clientfd, temp, strlen(temp), 0);
		}
		else if (strcmp(prefix, "LIST") == 0)
		{
			//printf("LIST");
			DIR *dir;
			struct dirent *dp;
			dir = opendir(PATH);
			char msg[8912] = "\0";
			strcat(msg, "\r\n");
			strcat(msg, "----------------------\r\n");

			while ((dp = readdir(dir)) != NULL)
			{
				if (dp->d_name[0] == '.')
				{
					continue;
				}
				if (dp->d_type & DT_DIR)
				{
					strcat(msg, dp->d_name);
					strcat(msg, "/");
				}
				else if (dp->d_type & DT_REG)
				{
					strcat(msg, dp->d_name);
				}
				strcat(msg, "\r\n");
			}
			strcat(msg, "\r\n");
			strcat(msg, "----------------------\r\n");
			send(clientfd, msg, strlen(msg), 0);
			closedir(dir);
		}
		//memset(prefix, 0, sizeof(prefix));
		//strncpy(prefix, sentence,5);
		else if (strcmp(prefix2, "PORT ") == 0)//PORT模式
		{

			//clientPort[clientfd]
			char temp[100];
			strncpy(temp, sentence + 5, strlen(sentence) - 5);
			temp[strlen(sentence) - 5] = '\0';
			if (getPort(temp, clientfd) != 0)
			{
				char sendMsg[] = "503 ERROR PORT.\r\n";
				send(clientfd, sendMsg, strlen(sendMsg), 0);
			}
			else
			{
				char sendMsg[] = "200 PORT command successful.\r\n";
				send(clientfd, sendMsg, strlen(sendMsg), 0);
				clientStatus[clientfd] = 3;//PORT mode
			}

		}
		else if (strcmp(prefix, "PASV") == 0)
		{
			char str[100];
			char sendMsg[] = "227 Entering PassiveMode (127,0,0,1";
			int port;
			srand(time(NULL));
			port = rand() % 45535 + 20000;
			if (pasvfd[clientfd] != 0)
			{
				close(pasvfd[clientfd]);
			}

			if ((pasvfd[clientfd] = ConnectSever(port)) != 1)
			{
				clientStatus[pasvfd[clientfd]] = 5;
				strcpy(PortIP[clientfd], "127.0.0.1");
				PortNumber[clientfd] = port;
				sprintf(str, "%d", port / 256);
				printf("%s\n", str);
				strcat(sendMsg, ",");
				strcat(sendMsg, str);
				strcat(sendMsg, ",");
				sprintf(str, "%d", port % 256);
				printf("%s\n", str);
				strcat(sendMsg, str);
				strcat(sendMsg, ")\r\n");
				clientStatus[clientfd] = 4;//PASV mode
				send(clientfd, sendMsg, strlen(sendMsg), 0);
			}
			else
			{
				printf("PASV ERROR\n");
			}

		}
		return;
	}
	strncpy(prefix, sentence, 5);
	if (strcmp(prefix, "RETR ") == 0) //RETR为前缀但是不是PORT或PASV模式
	{
		if (clientStatus[clientfd] != 3 && clientStatus[clientfd] != 4)
		{
			char sendMsg[] = "425 use PORT or PASV first.\r\n";
			send(clientfd, sendMsg, strlen(sendMsg), 0);
			return;
		}
	}
	if (strcmp(prefix, "STOR ") == 0) //STOR为前缀但是不是PORT或PASV模式
	{
		if (clientStatus[clientfd] != 3 && clientStatus[clientfd] != 4)
		{
			char sendMsg[] = "425 use PORT or PASV first.\r\n";
			send(clientfd, sendMsg, strlen(sendMsg), 0);
			return;
		}
	}
	if (clientStatus[clientfd] == 3)//PORT模式
	{
		strncpy(prefix, sentence, 5);
		if (strcmp(prefix, "STOR ") == 0)//STOR
		{
			int sockfd;
			//printf("%d\n", PortNumber[clientfd]);
			SentenceModify(PortIP[clientfd]);
			//printf("%s\n", PortIP[clientfd]);


			char fileName[100];
			char tempFileName[100];
			strncpy(tempFileName, sentence + 5, strlen(sentence) - 5);
			tempFileName[strlen(sentence) - 5] = '\0';//文件名
													  //printf("%s\n", tempFileName);
			char sendMsg[] = "150 Opening BINARY mode data connection for ";
			strcat(sendMsg, tempFileName);
			strcat(sendMsg, "\r\n");
			send(clientfd, sendMsg, strlen(sendMsg), 0);

			strcpy(fileName, PATH);
			strcat(fileName, "/");
			int PathLen = strlen(PATH) + 1;
			strncat(fileName, sentence + 5, strlen(sentence) - 5);
			fileName[strlen(sentence) - 5 + PathLen] = '\0';//文件名
															//printf("%s\n", fileName);
			char fileMsg[8192] = "\0";
			if ((sockfd = ConnectClient(PortIP[clientfd], PortNumber[clientfd])) != 1)
			{
				//printf("aaaa\n");
				FILE *fp;
				//printf("make file\n");
				//printf("%s\n", fileName);
				fp = fopen(fileName, "wb");
				if (fp == NULL)
				{
					char sendMsg[] = "550 file dose not exist\r\n";
					send(clientfd, sendMsg, strlen(sendMsg), 0);
					return;
				}
				else
				{
					int n = 0;
					while (1)
					{
						n = read(sockfd, fileMsg, 8192);
						if (n <= 0)
						{
							break;
						}
						else
						{
							fwrite(fileMsg, sizeof(char), n, fp);
							char sendMsg2[] = "226 Transfer complete.\r\n";
							send(clientfd, sendMsg2, strlen(sendMsg2), 0);
						}

					}
					fclose(fp);
				}
			}
			else
			{
				char sendMsg[] = "425 Connection attempt fails\r\n";
				send(clientfd, sendMsg, strlen(sendMsg), 0);
				return;
			}
			close(sockfd);
			clientStatus[clientfd] = 2;
		}
		else if (strcmp(prefix, "RETR ") == 0)
		{
			int sockfd;
			//printf("%d\n", PortNumber[clientfd]);
			SentenceModify(PortIP[clientfd]);
			//printf("%s\n", PortIP[clientfd]);
			char fileName[100];
			char tempFileName[100];
			strncpy(tempFileName, sentence + 5, strlen(sentence) - 5);
			tempFileName[strlen(sentence) - 5] = '\0';//文件名
													  //printf("%s\n", tempFileName);
			char sendMsg[] = "150 Opening BINARY mode data connection for ";
			strcat(sendMsg, tempFileName);
			strcat(sendMsg, "\r\n");
			send(clientfd, sendMsg, strlen(sendMsg), 0);

			char fileMsg[8192] = "\0";
			strcpy(fileName, PATH);
			strcat(fileName, "/");
			int PathLen = strlen(PATH) + 1;
			strncat(fileName, sentence + 5, strlen(sentence) - 5);
			fileName[strlen(sentence) - 5 + PathLen] = '\0';//文件名
			if ((sockfd = ConnectClient(PortIP[clientfd], PortNumber[clientfd])) != 1)
			{
				FILE *fp;
				//printf("make file\n");
				printf("%s\n", fileName);
				fp = fopen(fileName, "rb");
				if (fp == NULL)
				{
					char sendMsg[] = "550 file dose not exist\r\n";
					send(clientfd, sendMsg, strlen(sendMsg), 0);
					return;
				}
				else
				{
					int n = 0;
					while (!feof(fp))
					{
						n = fread(fileMsg, sizeof(char), sizeof(fileMsg), fp);
						if (send(sockfd, fileMsg, n, 0) < 0)
						{
							//printf("failed\n");
							break;
						}
					}
					fclose(fp);
					char sendMsg2[] = "226 Transfer complete.\r\n";
					printf("226 Transfer complete.\r\n");
					send(clientfd, sendMsg2, strlen(sendMsg2), 0);
				}

			}
			else
			{
				char sendMsg[] = "425 Connection attempt fails\r\n";
				send(clientfd, sendMsg, strlen(sendMsg), 0);
				return;
			}
			close(sockfd);
			clientStatus[clientfd] = 2;
		}
	}
	if (clientStatus[clientfd] == 4)//PASV mode
	{
		strncpy(prefix, sentence, 5);
		if (strcmp(prefix, "STOR ") == 0)//STOR
		{
			int sockfd;
			char fileName[100];
			char tempFileName[100];
			strncpy(tempFileName, sentence + 5, strlen(sentence) - 5);
			tempFileName[strlen(sentence) - 5] = '\0';//文件名
													  //printf("%s\n", tempFileName);
			char sendMsg[] = "150 Opening BINARY mode data connection for ";
			strcat(sendMsg, tempFileName);
			strcat(sendMsg, "\r\n");
			send(clientfd, sendMsg, strlen(sendMsg), 0);

			strcpy(fileName, PATH);
			strcat(fileName, "/");
			int PathLen = strlen(PATH) + 1;
			strncat(fileName, sentence + 5, strlen(sentence) - 5);
			fileName[strlen(sentence) - 5 + PathLen] = '\0';//文件名
															//printf("%s\n", fileName);
			char fileMsg[8192] = "\0";
			if ((sockfd = accept(pasvfd[clientfd], NULL, NULL)) != -1)
			{
				//printf("pasvbb\n");
				FILE *fp;
				//printf("make file\n");
				//printf("%s\n", fileName);
				fp = fopen(fileName, "wb");
				if (fp == NULL)
				{
					char sendMsg[] = "550 file dose not exist\r\n";
					send(clientfd, sendMsg, strlen(sendMsg), 0);
					return;
				}
				else
				{
					int n = 0;
					while (1)
					{
						n = read(sockfd, fileMsg, 8192);
						if (n <= 0)
						{
							break;
						}
						else
						{
							fwrite(fileMsg, sizeof(char), n, fp);
							char sendMsg2[] = "226 Transfer complete.\r\n";
							send(clientfd, sendMsg2, strlen(sendMsg2), 0);
						}
					}
					fclose(fp);
				}
			}
			else
			{
				char sendMsg[] = "425 Connection attempt fails\r\n";
				send(clientfd, sendMsg, strlen(sendMsg), 0);
				return;
			}
			close(sockfd);
			close(pasvfd[clientfd]);
			pasvfd[clientfd] = 0;
			clientStatus[clientfd] = 2;
		}
		else if (strcmp(prefix, "RETR ") == 0)
		{
			//printf("aaaaaa\n");
			int sockfd;
			char fileName[100];
			char tempFileName[100];
			strncpy(tempFileName, sentence + 5, strlen(sentence) - 5);
			tempFileName[strlen(sentence) - 5] = '\0';//文件名
													  //printf("%s\n", tempFileName);

			char sendMsg[] = "150 Opening BINARY mode data connection for ";
			strcat(sendMsg, tempFileName);
			strcat(sendMsg, "\r\n");
			send(clientfd, sendMsg, strlen(sendMsg), 0);

			strcpy(fileName, PATH);
			strcat(fileName, "/");
			int PathLen = strlen(PATH) + 1;
			strncat(fileName, sentence + 5, strlen(sentence) - 5);
			fileName[strlen(sentence) - 5 + PathLen] = '\0';//文件名
			char fileMsg[8192] = "\0";
			if ((sockfd = accept(pasvfd[clientfd], NULL, NULL)) != -1)
			{
				FILE *fp;
				//printf("make file\n");
				//printf("%s\n", fileName);
				fp = fopen(fileName, "rb");
				if (fp == NULL)
				{
					printf("file dose not exist\r\n");
					char sendMsg[] = "550 file dose not exist\r\n";
					send(clientfd, sendMsg, strlen(sendMsg), 0);
					return;
				}
				else
				{
					int n = 0;
					while (!feof(fp))
					{
						n = fread(fileMsg, sizeof(char), sizeof(fileMsg), fp);
						if (send(sockfd, fileMsg, n, 0) < 0)
						{
							//printf("failed\n");
							break;
						}
					}
					fclose(fp);
					char sendMsg2[] = "226 Transfer complete.\r\n";
					send(clientfd, sendMsg2, strlen(sendMsg2), 0);
				}

			}
			else
			{
				char sendMsg[] = "425 Connection attempt fails\r\n";
				send(clientfd, sendMsg, strlen(sendMsg), 0);
				return;
			}
			close(sockfd);
			close(pasvfd[clientfd]);
			pasvfd[clientfd] = 0;
			clientStatus[clientfd] = 2;
		}
	}
}
int main(int argc, char **argv)
{
	int DEFAULT_PORT = 21;
	int listenfd, connfd;
	struct sockaddr_in addr;
	char sentence[8192];
	int p;
	int len;
	int MaxClient;
	int i;
	int addrlen;
	int sockfd;

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-port") == 0)
		{
			if (i < (argc - 1))
			{
				DEFAULT_PORT = atoi(argv[i + 1]);
			}
		}
		else if (strcmp(argv[i], "-root") == 0)
		{
			if (i<(argc - 1))
			{
				strcpy(PATH, argv[i + 1]);
			}
		}
	}
	printf("The port now is:");
	printf("%d\n", DEFAULT_PORT);
	printf("The root for all requests is:");
	printf("%s\n", PATH);

	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
	if (listenfd == -1) {
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(DEFAULT_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	if (listen(listenfd, 10) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
	fd_set fds, ready_fds;
	FD_ZERO(&fds);
	FD_ZERO(&ready_fds);
	FD_SET(listenfd, &fds);
	MaxClient = listenfd;
	while (1)
	{
		memcpy(&ready_fds, &fds, sizeof(fds));
		if (select(MaxClient + 1, &ready_fds, NULL, NULL, NULL) == -1) {
			printf("Error select(): %s(%d)\n", strerror(errno), errno);
			return 1;
		}
		for (i = 0; i <= MaxClient; i++)
		{
			if (FD_ISSET(i, &ready_fds))
			{
				if (i == listenfd)
				{
					addrlen = sizeof(addr);
					if ((connfd = accept(listenfd, (struct sockaddr*)&addr, (socklen_t *)&addrlen)) == -1)
					{
						printf("Error accept(): %s(%d)\n", strerror(errno), errno);
						continue;
					}
					else
					{
						FD_SET(connfd, &fds);
						if (connfd > MaxClient)
							MaxClient = connfd;
						char str[] = "220 Anonymous FTP server ready.\r\n";
						send(connfd, str, strlen(str), 0);
					}
				}
				else
				{
					p = 0;
					while (1) {
						int n = read(i, sentence + p, 8191 - p);
						if (n < 0) {
							printf("Error read(): %s(%d)\n", strerror(errno), errno);
							close(i);
							continue;
						}
						else if (n == 0) {
							break;
						}
						else {
							p += n;
							if (sentence[p - 1] == '\n') {
								break;
							}
						}
					}

					sentence[p - 2] = '\0';
					ClientHandle(i, sentence);
				}
			}
		}
	}

	close(listenfd);
}

