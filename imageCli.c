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
		//rb+以可读写的方式打开一个二进制文件
		fp = fopen(img, "rb+");
		
		//fp文件流从文件结尾偏移0
		fseek(fp, 0, SEEK_END);
		
		//用于得到文件位置指针当前位置相对于文件首的偏移字节数。
		//先将指针移到文件尾，判断出文件的字节总长度
		int fend = ftell(fp);
		
		//fp文件流从文件开始偏移0
		fseek(fp, 0, 0);
		int sendbytes, recvbytes;

		while(fend > 0)
		{
			memset(picture.data, 0, sizeof(picture.data));
			
			//fread从流fp中读取1项二进制数据，存储到picture.data，每项数据1024个字节
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
				//每次发送成功则将总长度减1024
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
