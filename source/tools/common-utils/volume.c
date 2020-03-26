#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

//input:  E6 9D 8E E7 8E 89 E5 88 9A 2D E5 88 9A E5 A5 BD E9 81 87 E8 A7 81 E4 BD A0 2E 6D 70 33
//output: 李玉刚-刚好遇见你.mp3
//
//

#define L_LIMIT		15
#define B_LIMIT		100

// [0-15] <--> [0-100]
int l_to_b(int x, int x_limit, int y_limit)
{
	int y;

	int quotient = x*y_limit/x_limit;
	int remainder = x*y_limit%x_limit;

	y = quotient + ((remainder>0)?1:0);

	// printf("%d -> %d, [%d,%d]\n", x, y, quotient, remainder);

	return y;
}

int b_to_l(int x, int x_limit, int y_limit)
{
	int y;

	int quotient = x*y_limit/x_limit;
	int remainder = x*y_limit%x_limit;

	//	y = quotient + ((remainder>0)?1:0);
	y = quotient;

	// printf("%d -> %d, [%d,%d]\n", x, y, quotient, remainder);

	return y;
}

int main(int argc, char *argv[])
{

#if 0
	int i;
	char input[1024];
//	char output[1024];
	int data[255];
	char *p_rest;
	char *p_temp;
	bool valid;
	bool exit;

	printf("please input:\n");
	while (1)
	{
		printf("input:\t");
		fgets(input, sizeof(input), stdin);
		printf("output:\t");

		//memset(output, '\0', sizeof(output));
		p_rest = input;

		if (strstr(p_rest, "big to small: ") != 0)
		{
		}
		else if (strstr(p_rest, "small to big: ") != 0)
		{
		}
		
	}
#endif

	int i;
	int x;
	int y;

	for (i=0; i<=L_LIMIT; i++)
	{
		x = i;
		y = l_to_b(x, L_LIMIT, B_LIMIT);
		printf("%d -> %d, ", x, y);

		x = y;
		y = b_to_l(x, B_LIMIT, L_LIMIT);
		printf("%d -> %d\n", x, y);
	}

	printf("\n\n");

	for (i=0; i<=B_LIMIT; i++)
	{
		x = i;
		y = b_to_l(x, B_LIMIT, L_LIMIT);
		printf("%d -> %d, ", x, y);

		x = y;
		y = l_to_b(x, L_LIMIT, B_LIMIT);
		printf("%d -> %d\n", x, y);
	}

	return 0;
}
