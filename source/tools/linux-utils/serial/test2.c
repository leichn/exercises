#include <stdio.h>      /*标准输入输出定义*/  
#include <stdlib.h>     /*标准函数库定义*/  
#include <unistd.h>     /*Unix 标准函数定义*/  
#include <sys/types.h>    
#include <sys/stat.h>     
#include <fcntl.h>      /*文件控制定义*/  
#include <termios.h>    /*PPSIX 终端控制定义*/  
#include <errno.h>      /*错误号定义*/  
#include <stdint.h>
#include <sys/ioctl.h>
#include <termios.h>

#define FALSE -1  
#define TRUE 0  

int set_Parity(int fd,int databits,int stopbits,int parity);  

int serial_init(void)
{
	struct  termios opt;  

	int fd;

	fd = open("/dev/ttyS0",O_RDWR|O_NOCTTY);  
	if(fd==-1)  
	{  
		printf("open failed\n");  
		exit (0);  
	}  
	else  
		printf("open success\n");  

	tcgetattr(fd,&opt);  

	cfsetispeed(&opt,B57600);   
	cfsetospeed(&opt,B57600);

	opt.c_cflag |= (CLOCAL | CREAD);                     



	// 无校验 8位数据位1位停止位  

	opt.c_cflag &= ~PARENB;                           

	opt.c_cflag &= ~CSTOPB;  

	opt.c_cflag &= ~CSIZE;  



	// 8个数据位  

	opt.c_cflag |= CS8;  



	// 原始数据输入  

	opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);   

	opt.c_oflag &= ~(OPOST);  



	// 设置等待时间和最小接收字符数  

	opt.c_cc[VTIME]    = 2;                            

	opt.c_cc[VMIN]     = 10;     



	// 处理未接收的字符  

	tcflush(fd, TCIFLUSH);    
	tcsetattr(fd,TCSANOW,&opt);                           

	return fd;
}

int main()  
{  
	int i = 0;
	int ret = 0;
	uint8_t buf[1024];  
	int fd,flag_close,retv;  

	fd = open("/dev/ttyS2",O_RDONLY); // 改变console
//	fd = open("/dev/NULL",O_RDONLY); // 改变console
	ret = ioctl(fd,TIOCCONS);    //TIOCCONS 的作用是使成为虚拟控制台
	close(fd);
	printf("change console, ret=%d\n", ret);

#if 1
	while (1);
#else
	fd = serial_init();

	while(1)  
	{         
		retv=read(fd,buf,1024);     

		if(retv==-1)  
		{
			printf("read failed\n");  
		}

		if(retv>0)  
		{  
			for (i=0; i<retv; i++)
			{
				printf("%02X ", buf[i]);
			}
			printf("\n");
		}  
		
		write(fd, buf, retv);

		usleep(10000);  
	}          
#endif


	flag_close = close(fd);  

	if(flag_close == -1)     
		printf("Close the Device failed\n");  
	else
		printf("close console success\n");

	return 0;  

}  
