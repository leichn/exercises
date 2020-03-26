#include <stdio.h>

int main()
{

	for (int i=0; i<5; i++)
	{
		int bar = 0;
		bar++;
		printf("%d %d\n",bar, &bar);
	}


	return 0;
}
