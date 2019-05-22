#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <memory.h> 
#include <stdlib.h> 
#include <fcntl.h>
 
#define BUFFER_SIZE 8
#define GPS_SIZE 1000
int main(int argc, char *argv[])
{
    int sockfd,n;
    struct sockaddr_in server,client;
	int addrlen=sizeof(struct sockaddr);
    char filename[100];
    char filepath[100];
 
    char *buffer;//file buffer
    int fileTrans;
 
    buffer = (char *)malloc(sizeof(char)*BUFFER_SIZE);
    printf("init bufferSize=%d  SIZE=%d\n",sizeof(buffer),BUFFER_SIZE);
    //bzero(buffer,BUFFER_SIZE); 
    int lenfilepath;
    FILE *fp;
    int writelength;
 
    if((sockfd = socket(AF_INET,SOCK_DGRAM,0))<0)
    {
        printf("socket build error!\n");
    }
    else
    {
        printf("socket build success!\n");
    }
    memset(&server,0,sizeof(server));  //清空server结构体
    server.sin_family= AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(8888);

    //int nRecvBuf = 2048;//设置为32K
    //int optlen = sizeof(int);
    //setsockopt(sockfd, SOL_SOCKET,SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));

    //int lengthBuf;
    //getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &lengthBuf, &optlen); 

    //printf("缓冲区大小为：%d\n", lengthBuf);

    struct timeval tv_out;
    tv_out.tv_sec = 1;
    tv_out.tv_usec = 0;
    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv_out,sizeof(tv_out));

    if((bind(sockfd,(struct sockaddr*)&server,sizeof(server)))==-1)
    {
        printf("bind error!\n");
    }
    else
    {
        printf("bind success!\n");
    }
    int fifo_fd = open(argv[1], O_RDONLY);
    while(1)
    {
	    printf("waiting....\n");
            memset(filename,'\0',sizeof(filename));
            memset(filepath,'\0',sizeof(filepath));
            lenfilepath = recvfrom(sockfd,filepath,100,0,(struct sockaddr *)&client,&addrlen);
            printf("filepath :%s\n",filepath);
            if(lenfilepath<0)
            {
                printf("recv error!\n");
		return -1;
            }
            else
            {
                int i=0,k=0;  
                for(i=strlen(filepath);i>=0;i--)  
                {  
                    if(filepath[i]!='/')      
                    {  
                        k++;  
                    }  
                    else   
                        break;    
                }  
                strcpy(filename,filepath+(strlen(filepath)-k)+1);   
            }
            printf("filename :%s\n",filename);
            //fp = fopen(filename,"w");
            //if(fp!=NULL)
            //{
		int times = 1;
		int flag = fcntl(fifo_fd, F_GETFL);
		flag |= O_NONBLOCK;
		fcntl(fifo_fd, F_SETFL, flag);
                while(1)
                {
		    //gps端接收到图像端的命令后才能开始接收,清空服务器端的读缓冲区
		    while(1){
	                    bzero(buffer,BUFFER_SIZE);
			    fileTrans = recvfrom(sockfd,buffer,BUFFER_SIZE,0,(struct sockaddr *)&client,&addrlen);
	 		    char buf[2];
			    int ret;
			    ret = read(fifo_fd, buf, sizeof(buf));
			    if(ret > 0){
				break;
			    }
		    }

		    bzero(buffer,BUFFER_SIZE);
	            fileTrans = recvfrom(sockfd,buffer,BUFFER_SIZE,0,(struct sockaddr *)&client,&addrlen);

		    fp = fopen(filename,"ab+");

		    printf("times = %d   ",times);
 		    times++;
		    printf("filetrans size : %d\n", fileTrans);

                    if(fileTrans<0)
                    {
                        printf("recv2 error!\n");
                        break;
                    }
		    printf("gps recv : %s\n", buffer);
                    writelength = fwrite(buffer,sizeof(char),fileTrans,fp);
 
                    if(fileTrans < BUFFER_SIZE)
                    {
                        printf("finish writing!\n");
                        break;
                    }else{
			//printf("write succ! %d fileTrans=%d\n",writelength,fileTrans);
			printf("write successful!\n");
			//break;
		    }
		   
                    bzero(buffer,BUFFER_SIZE);
		    if(times == GPS_SIZE + 1)
                    {
                        printf("finish writing!\n");
                        break;
		    }
		    else
			 printf("continue\n");

		    fclose(fp);
                }
                printf("recv finished!\n");
                
		close(fifo_fd);
    }
    close(sockfd);
    return 0;
}
