#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
 
#define Port 9000 // 服务器端口地址
 
// 图片包格式
struct Package
{
	int length;
	char data[1024];
	int fin;
}picture;
 
int main(int argc, char* argv[])
{
	int clientSocket;
	struct sockaddr_in server;
	int addr_len=sizeof(struct sockaddr);

	if((clientSocket = socket(AF_INET,SOCK_DGRAM,0))<0)
	{
		printf("socket build error!\n");
	}
	memset(&server,0,sizeof(server));
	server.sin_family= AF_INET;
	server.sin_port = htons(Port);

	if(inet_pton(AF_INET,"127.0.0.1",&server.sin_addr)<0)
	{
		printf("inet_pton error!\n");
	}

	char img[20];

	int num = 89;
	while(1)
	{
		
		// 打包发送图片
		char s[5];
		sprintf(s, "%d", num);
		strcpy(img, "./imageSend/");
		strcat(img, s);
		strcat(img, ".jpg");
		printf("%s\n", img);

		FILE *fp;
		fp = fopen(img, "rb+");
		fseek(fp, 0, SEEK_END);
		int fend = ftell(fp);
		fseek(fp, 0, 0);
		int sendbytes, recvbytes;

		while(fend > 0)
		{
			memset(picture.data, 0, sizeof(picture.data));
			fread(picture.data, 1024, 1, fp);
			if(fend >= 1024) //还有剩余包未发送完
			{
				picture.length = 1024;
				picture.fin = 0;
			}
			else //最后一个包 
			{
				picture.length = fend;
				picture.fin = 1;
			}
 
			sendbytes = sendto(clientSocket, (char *)&picture, sizeof(struct Package), 0, (struct sockaddr*)&server, addr_len);
 
			if(sendbytes < 0)
			{
				printf("Send Picture Failed!\n");
				return -1;
			}
			else
			{
				fend -= 1024;  
			}
		}
 
		// 接收服务器回应
		char recvbuffer[256];
		recvbytes = recvfrom(clientSocket, recvbuffer, sizeof(recvbuffer), 0, (struct sockaddr*)&server, &addr_len);
		printf("Server: %s",recvbuffer);
		sleep(2);
		num++;
	}
	// 关闭socket
	close(clientSocket);
	return 0;
}
