#include <stdio.h>
#include <stdint.h>

void main(void)
{
	uint8_t arr[8] = {	0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88	};
	uint32_t *p1 = (uint32_t *)(&arr[1]);
	uint32_t *p2 = (uint32_t *)(&arr[2]);
	uint32_t *p3 = (uint32_t *)(&arr[4]);
	uint32_t *p4 = (uint32_t *)(&arr[5]);

	printf("%08X %08X %08X %08X\n", *p1, *p2, *p3, *p4);
}
