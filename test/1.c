#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "LWlogc.h"



int main()
{
	char *loc = "/home/lixiaogang5/Code/LWLogc/test/666.log";
	int len = 0;
	len = LwlogcCouNum2SizeOfFiles(loc);
	printf("@len:%d\n", len);

	LwlogcGetLogsNum(logConfigure.fp);

	return 0;
}
