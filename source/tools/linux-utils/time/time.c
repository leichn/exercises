#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>


int main()
{
    struct timeval tv;

	while (1)
	{
		gettimeofday(&tv,NULL);

		printf("%ld.%ld\n", tv.tv_sec, tv.tv_usec);

		sleep(1); // 为方便观看，让程序睡三秒后对比
	}

    return 0;
}
