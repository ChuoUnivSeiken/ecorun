/*
 * integer_test.c
 *
 *  Created on: 2015/03/13
 *      Author: Yoshio
 */

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../src/integer.h"

int main(void)
{
	char str[10];
	memset(str, 0, sizeof(str));

	uint32_t succ = 1;
	uint32_t i;
	uint32_t one_percent = INT_MAX / 100;
	for (i = 0; i <= INT_MAX; i++)
	{
		int32_t src = i, dest;
		int32_to_str(src, str);
		dest = str_to_uint32(str);
		//sscanf(str, "%d", &dest); //OK

		succ &= (dest == src);

		if (i % one_percent == 0)
		{
			printf("%d %\n", i / one_percent);
		}

		if (!succ)
		{
			printf("FAILED\n");
			return 0;
		}
	}

	printf(succ ? "SUCCEEDED\n" : "FAILED\n");

	return 0;
}
