SyncBasedFifo
===============
有些无人机厂商的图像和飞控数据分开传输，由于两者传输速率的不同，容易造成图像帧和GPS数据的不同步，从而导致GPS融合全景图后定位不准或无法融合。通过UDP协议模拟图像端和飞控端收发数据，设置图像端传输速率为两秒一帧，飞控端传输速率为一秒一帧，以图像端接收到数据的时间戳为准，飞控端接收当前时刻的飞控数据，基于FIFO实现两者的数据同步。   

关键思路
------------
* fifo通知
	* 图像端写，飞控端读
	```C++
	//图像端接收到数据后，通知飞控端
	write(fifo_fd, buf, strlen(buf));

	//飞控端接收通知
	ret = read(fifo_fd, buf, sizeof(buf));
	```

* 清空飞控服务器端读缓冲区
	* fifo文件描述符设置非阻塞
	```C++
	int flag = fcntl(fifo_fd, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(fifo_fd, F_SETFL, flag);
	```
		
	* 清空读缓冲区
	```C++
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

	//读取当前时刻的飞控数据
	bzero(buffer,BUFFER_SIZE);
	fileTrans = recvfrom(sockfd,buffer,BUFFER_SIZE,0,(struct sockaddr *)&client,&addrlen);
	```
* 测试
	* 创建fifo文件
	```C++
	mkfifo imageGPSFifo
	```

	* 编译生成可执行文件
	```C++
	gcc imageSer -o imageser
	gcc imageCli -o imagecli
	gcc gpsSer -o gpsser
	gcc gpsCli -o gpscli
	```
	
	* 启动图像端和飞控端
	```C++
	//清空图像接收文件夹内的文件
	cd ./imageRecv
	rm -f *

	//先启动飞控端
	./gpsser imageGPSfifo
	./gpscli	//输入测试数据 ./gps/gps.txt

	//启动图像端
	./imageser imageGPSfifo
	./imagecli
	```

	* 结果文件
	```C++
	//飞控端接收文件为当前文件夹gps.txt
	//图像端接收的图片在imageRecv
	```
