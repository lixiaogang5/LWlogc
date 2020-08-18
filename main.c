/* ***********************************************************************************
 * @Function	: 																	 *
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

//extern LWLogcConf logConfigure;
#define LWLOG_INFO(x) LwlogcMessage(logConfigure.logLevel, __LINE__, __FUNCTION__, LwlogcFormatLogMessage x)


int main(int argc, char **argv)
{

	if(1 != LwlogcInit("lwlog.properties"))
	{
		fputs("lwlog_init failed.", stderr);
		sleep(1);
	}

	
	for(;;) LWLOG_INFO(("lwlogc_message start TEST ...")), sleep(1);
	
	return 0;
}


