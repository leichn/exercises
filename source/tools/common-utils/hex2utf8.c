#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

//input:  E6 9D 8E E7 8E 89 E5 88 9A 2D E5 88 9A E5 A5 BD E9 81 87 E8 A7 81 E4 BD A0 2E 6D 70 33
//output: 李玉刚-刚好遇见你.mp3

int main(int argc, char *argv[])
{
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
		
		for (i=0; i<sizeof(data); i++)
		{
			if ((p_temp=strchr(p_rest, ' ')) != NULL ||
			    (p_temp=strchr(p_rest, ',')) != NULL)
			{
				valid = true;
				exit = false;
			}
			else if	((p_temp=strchr(p_rest, '\n')) != NULL)
			{
				valid = true;
				exit = true;
			}
			else
			{
				valid = false;
			}

			if (valid)
			{
				*p_temp = '\0';
				data[i] = strtoul(p_rest, NULL, 16);
				//printf("data[%d]=%d\n", i, data[i]);
				//snprintf(output, sizeof(output), "%s,%d", output, data[i]);
				printf("%c",data[i]);
				if (exit)
				{
					break;
				}
				else
				{
					p_rest = p_temp + 1;
				}

			}
			else
			{
				break;
			}

		}
		
		printf("\n");
	}

	return 0;
}
