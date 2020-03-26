#include <stdio.h>      /*标准输入输出定义*/  
#include <stdlib.h>     /*标准函数库定义*/  
#include <unistd.h>     /*Unix 标准函数定义*/  
#include <sys/types.h>    
#include <sys/stat.h>     
#include <fcntl.h>      /*文件控制定义*/  
#include <termios.h>    /*PPSIX 终端控制定义*/  
#include <errno.h>      /*错误号定义*/  
#include <stdint.h>

/** 
*@brief  设置串口通信速率 
*@param  fd     类型 int  打开串口的文件句柄 
*@param  speed  类型 int  串口速度 
*@return  void 
*/
int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,  
    B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300, };
int name_arr[] = {115200, 38400, 19200, 9600, 4800, 2400, 1200,  300,
    115200, 38400, 19200, 9600, 4800, 2400, 1200,  300, };

void set_speed(int fd, int speed)
{
    unsigned int i = 0;   
    int status = 0;   
    struct termios opt;  
    tcgetattr(fd, &opt);   
    for (i= 0;  i<sizeof(speed_arr)/sizeof(int);  i++)
    {   
        if (speed == name_arr[i])
        {       
            tcflush(fd, TCIOFLUSH);       
            cfsetispeed(&opt, speed_arr[i]);    
            cfsetospeed(&opt, speed_arr[i]);     
            status = tcsetattr(fd, TCSANOW, &opt);    
            if (status != 0)
            {          
                perror("tcsetattr fd1");    
                return;       
            }
            tcflush(fd, TCIOFLUSH);     
        }    
    }  
}

/** 
*@brief   设置串口数据位，停止位和效验位 
*@param  databits 类型  int 数据位   取值 为 7 或者8 
*@param  stopbits 类型  int 停止位   取值为 1 或者2 
*@param  parity  类型  int  效验类型 取值为N,E,O,,S 
*/  
int uart_init(char *path, int speed, int databits, int stopbits, int parity)
{
    unsigned int i = 0;   
    int status = 0;   

    int fd = -1;

	if (path == NULL)
	{
		printf("path null pointer\n");
		return -1;
	}
	
	fd = open(path, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        printf("open %s failed, return %d\n", path, fd);
        return -2;
    }
    
    struct termios options;   
    if  (tcgetattr(fd,&options) != 0)
    {   
        printf("uart tcgetattr() failed\n");       
        return -3;    
    }

	// 波特率
    for (i= 0;  i<sizeof(speed_arr)/sizeof(int);  i++)
    {   
        if (speed == name_arr[i])
        {       
            tcflush(fd, TCIOFLUSH);       
            cfsetispeed(&options, speed_arr[i]);    
            cfsetospeed(&options, speed_arr[i]);     
            status = tcsetattr(fd, TCSANOW, &options);    
            if (status != 0)
            {          
                perror("tcsetattr fd1");    
                return -4;       
            }
            tcflush(fd, TCIOFLUSH);     
        }    
    }  

    
	// 数据位
    options.c_cflag &= ~CSIZE;   
    switch (databits) /*设置数据位数*/  
    {     
        case 7:       
            options.c_cflag |= CS7;   
            break;  
        case 8:       
            options.c_cflag |= CS8;  
            break;     
        default:      
            printf("uart unsupported data bits\n");
            return -5;
    }
    
	// 奇偶校验位
    switch (parity)   
    {     
        case 'n':  
        case 'N':      
            options.c_cflag &= ~PARENB;             /* Clear parity enable */  
            options.c_iflag &= ~INPCK;              /* Disable parity checking */   
            break;
            
        case 'o':     
        case 'O':       
            options.c_cflag |= (PARODD | PARENB);   /* 设置为奇效验*/    
            options.c_iflag |= INPCK;               /* Enable parity checking */   
            break;
            
        case 'e':    
        case 'E':     
            options.c_cflag |= PARENB;              /* Enable parity */      
            options.c_cflag &= ~PARODD;             /* 转换为偶效验*/       
            options.c_iflag |= INPCK;               /* Enable parity checking */  
            break;
            
        case 'S':   
        case 's':  /*as no parity*/     
            options.c_cflag &= ~PARENB;  
            options.c_cflag &= ~CSTOPB;
            break;
            
        default:     
            printf("uart unsupported parity\n");      
            return -6;    
    }    
    
    /* 设置停止位*/    
    switch (stopbits)  
    {     
        case 1:      
            options.c_cflag &= ~CSTOPB;    
            break;
        
        case 2:      
            options.c_cflag |= CSTOPB;    
           break;
        
        default:      
             printf("uart unsupported stop bits\n");    
             return -7;   
    }

    //驱动程序启动接收字符装置，同时忽略串口信号线的状态
    options.c_cflag |= (CLOCAL | CREAD); //一般必设置的标志

    // 关闭软件流控，避免XON/XOFF字符无法传输的问题
    options.c_iflag &= ~ (IXON | IXOFF | IXANY);

    // 使用原始输入，屏蔽NL-CR和CR-NL的映射，忽略接收到回车符，不剥离输入字节的第8位
    options.c_iflag &= ~ (INLCR | ICRNL | IGNCR | ISTRIP);
    
    // 屏蔽NL-CR和CR-NL的映射，避免'\r'和'\n'被当成同个字符
    options.c_oflag &= ~(ONLCR | OCRNL);

    // 数据直接发送，避免未输入回车不发送的问题
    options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);

    // 使用原始输出，输出数据不经任何处理
    options.c_oflag  &= ~OPOST;  

#if 0
    struct serial_struct serial; 
 
    ioctl(fd, TIOCGSERIAL, &serial); 
#if TEST_LOW_LATENCY 
    serial.flags |= ASYNC_LOW_LATENCY; 
#else 
    serial.flags &= ~ASYNC_LOW_LATENCY; 
#endif 
    printf("xmit_fifo_size = %d \r\n", serial.xmit_fifo_size);
    serial.xmit_fifo_size = 8; // what is "xmit" ?? 
    ioctl(fd, TIOCSSERIAL, &serial); 

#endif

    tcflush(fd, TCIFLUSH);
    
    options.c_cc[VTIME] = 1;        // 设置超时0.1秒，单位0.1秒
    options.c_cc[VMIN]  = 240;      // 设置接收缓冲区为240个字符
    
    if (tcsetattr(fd, TCSANOW, &options) != 0)     
    {   
        printf("uart tcsetattr() failed\n");     
        return -8;    
    }    
    
    return fd;
}

int main(int argc, char *argv[])  
{ 
	char *p_dev;
	int bps;
	int datab;
	int stopb;
	int parityb;
	uint8_t buf[1024];  
	int i;
	int fd;
	int flag_close;
	int retv;  

	if (argc != 6)
	{
		printf("Usage  : ./uart_test DEV BPS DATAB STOPB PARITYB\n");
		printf("Such as: ./uart_test /dev/ttyS1 9600 8 1 N\n");
		return -1;
	}

	p_dev	= argv[1];
	bps		= atoi(argv[2]);
	datab	= atoi(argv[3]);
	stopb	= atoi(argv[4]);
	parityb = argv[5][0];
	
	fd = uart_init(p_dev, bps, datab, stopb, parityb);

	if (fd < 0 )
	{
		printf("uart init failed\n");
		return -1;
	}

	printf("wait data\n");  

	write(fd, "test", 4);

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
		usleep(10000);  

		write(fd, buf, retv);
	}          

	flag_close = close(fd);  

	if(flag_close == -1)     
	{
		printf("Close uart failed\n");  
	}
	else
	{
		printf("close uart success\n");
	}

	return 0;  
}  
