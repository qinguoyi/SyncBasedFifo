#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
 
#define PORT 9000
 
struct Package
{
    int length;
    char data[1024];
    int fin;
} picture;


/*初始化接收图像，并获取当前时间，以此存储命名图片*/
void initImage(FILE**fp)
{
    time_t t = time(0);
    char imgpath[128];

    strcpy(imgpath, "./imageRecv/");    

    char imgname[64];
    strftime(imgname, sizeof(imgname), "%Y%m%s%H%M%S.jpg", localtime(&t));
 
    if(!(*fp = fopen(strcat(imgpath, imgname), "wb+")))
    {
        printf("Open image Failed!\n");
        exit(1);
    }
}


int main(int argc, char* argv[])
{
    struct sockaddr_in addr;
    socklen_t sockfd, addr_len = sizeof(struct sockaddr_in);
 
    /* 建立socket，注意必须是SOCK_DGRAM */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror ("socket failed!");
        exit(1);
    }
 
    /* 填写sockaddr_in 结构 */
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY) ;
 
    /* 绑定socket */
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr))<0)
    {
        perror("connect failed!");
        exit(1);
    }

    //打开管道文件
    int fifo_fd = open(argv[1], O_WRONLY);

    while(1)
    {
        FILE* fp;
        int flag = 1;
        picture.fin = 0;
 
        while(!picture.fin)
        {
            memset(picture.data, 0, sizeof(picture.data));
            int recvbytes = recvfrom(sockfd, (char*)&picture, sizeof(struct Package),
                                     0, (struct sockaddr *)&addr, &addr_len);
 
            if(flag)
            {
                initImage(&fp);
                flag = 0;
            }
 
            if(recvbytes == 0)
                break;
 
            fwrite(picture.data, picture.length, 1, fp);
        }
 	 //当图像端收到信息后，向gps端发送可接收命令
	char buf[2];
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "1");
	write(fifo_fd, buf, strlen(buf));

        /* 显示client端的网络地址和收到的字符串消息 */
        //printf("Socket Server: IP: %s Connected!\n",  inet_ntoa(addr.sin_addr));
        printf("Receive a picture. ip address: %s\n", inet_ntoa(addr.sin_addr));
 
        char sendbuffer[256] = "Server Response";
        int sendbytes = sendto(sockfd, sendbuffer, sizeof(sendbuffer), 0, (struct sockaddr *)&addr, addr_len);
 
        fclose(fp); /*关闭fp，刷新缓冲区*/
    }
    close(sockfd);
    return 0;
}
