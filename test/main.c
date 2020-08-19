/* ***********************************************************************************
 * @Function	: LWlogc log library usage example.									 *
 * @Version		: 1.0.0																 *
 * @Author		: lixiaogang5														 *
 * @Date		: 2020-08-12														 *
 * @Contact 	: lxiaogang5@gmail.com												 *
 * @Company		: HIKVISION                                                          *
 * ***********************************************************************************
 */

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "LWlogc.h"

int LwlogcSumFunc(const int *a, const int *b)
{
	LWLOGC_INFO("a[%d], b[%d]", *a, *b);
	return *a + *b;
}



int main(int argc, char **argv)
{

	if(true != LwlogcInit("lwlog.properties"))
	{
		LWLOGC_ERROR("LwlogcSumFunc init failed.");
		sleep(1);
	}

	LWLOGC_DEBUG("LwlogcSumFunc init succeed.");

	int a = 10, b = 20, c = 0;
	c = LwlogcSumFunc(&a, &b);
	LWLOGC_WARN("c[%d]", c);
	
	
	return 0;
}


