#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <memory.h> 
#include <stdlib.h>
 
#define BUFFER_SIZE 8
 
int main()
{
    int sockcd;
    struct sockaddr_in server;
    char filepath[100];//file to translate
    int addrlen=sizeof(struct sockaddr);
 
    FILE *fp;
    int lenpath; //filepath length
    char *buffer;//file buffer
    int fileTrans;
    buffer = (char *)malloc(sizeof(char)*BUFFER_SIZE);
    bzero(buffer,BUFFER_SIZE); 
 
    if((sockcd = socket(AF_INET,SOCK_DGRAM,0))<0)
    {
        printf("socket build error!\n");
    }
    memset(&server,0,sizeof(server));
    server.sin_family= AF_INET;
    server.sin_port = htons(8888);
    if(inet_pton(AF_INET,"127.0.0.1",&server.sin_addr)<0)
    {
        printf("inet_pton error!\n");
    }
 
    printf("file path:\n");
    scanf("%s",filepath);//get filepath
 
    fp = fopen(filepath,"r");//opne file
    if(fp==NULL)
    {
        printf("filepath not found!\n");
        return 0;
 
    }
    printf("filepath : %s\n",filepath);
    lenpath = sendto(sockcd,filepath,strlen(filepath),0,(struct sockaddr *)&server,addrlen);// put file path to sever 
    if(lenpath<0)
    {
        printf("filepath send error!\n");
    }
    else
    {
        printf("filepath send success!\n");
    }
    //sleep(1);
    printf("begin send data...\n");
    int times = 1;
    /*********************************************
    *   从文件fp里读取100个字节,可用以下语句  
    *	fread(buffer,100,1,fp)  
    *	fread(buffer,50,2,fp)  
    *	fread(buffer,1,100,fp)   
    *********************************************/
    //从fp中读取1024个字节，每次读一个，存入buf
    while((fileTrans = fread(buffer,sizeof(char),BUFFER_SIZE,fp))>0)
    {
	sleep(1);  //注意这里
	printf("times = %d   ",times);
	times++;
        //printf("fileTrans =%d\n",fileTrans);
        if(sendto(sockcd,buffer,fileTrans,0,(struct sockaddr *)&server,addrlen)<0)
        {
            printf("send failed!\n");
            break;      
        }
	else{
	    printf("send successful!\n");
	    printf("gps is : %s\n", buffer);
	}
	if(fileTrans < BUFFER_SIZE) break;
        bzero(buffer,BUFFER_SIZE); 
    }
    //printf("fileTrans =%d\n",fileTrans);
    fclose(fp);
    close(sockcd);
    return 0;
}
