/* ***********************************************************************************
 * @Function	: 																	 *
 * @Version		: 1.0.0																 *
 * @Author		: lixiaogang5														 *
 * @Date		: 2020-08-12														 *
 * @Contact 	: lxiaogang5@gmail.com												 *
 * @Company		: HIKVISION                                                          *
 * ***********************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#include <stdarg.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "LWLogc.h"


#define LWLOG_STREAM LwlogcGetNowStream()
static FILE *gStream = 0;

typedef enum {
	false,
	true,
}bool;

static const char *lwlogcLeverStr[] = {"LW_ERROR", "LW_WARN", "LW_INFO", "LW_DEBUG"};
LWLogcConf logConfigure;
bool bInit = false;
pthread_attr_t attr; //可采用静态初始化方式.


void LwlogcSetStream(const FILE *stream)
{
	if(!stream) {
		fputs("lwlogc_set_stream param is null.", stderr);
		gStream = stdout;
	}
	
	gStream = stream;
	return;
}


/**
*  @name LWlogc obtain i/o stream
*/
FILE *LwlogcGetNowStream()
{
	if(!gStream) gStream = stderr;
	return gStream;
}


/**
*  @name LWlogc now time
*/
const char *LwlogcTimeNow(char *time_buf)
{
    struct timeval tv;
    struct tm lt;
    time_t time_now_sec = 0;
    size_t len = 0;

	memset(&tv, 0x00, sizeof(tv));
//	memset(&lt, 00x0, sizeof(lt));
    
    gettimeofday(&tv,0);
    time_now_sec = tv.tv_sec;
    assert(localtime_r(&time_now_sec, &lt));

    // clone the format used by log4j ISO8601DateFormat
    // specifically: "yyyy-MM-dd HH:mm:ss,SSS"
    len = strftime(time_buf, TIME_NOW_BUF_SIZE, "%Y-%m-%d "WEEK"%u %H:%M:%S", &lt);
    len += snprintf(time_buf + len, TIME_NOW_BUF_SIZE - len, ",%03d", (int)(tv.tv_usec/1000));

    return time_buf;
}



/*
 * Get a version of the information. 
 * suc: returns a pointer to the memory; failed: abort
 */
void LwlogcVersion(char *dst)
{
	if(!dst) {
		fputs("LwlogcVersion parameter is empty.", stderr);
		return;
	}

	memset(dst, 0x00, *dst);
	snprintf(dst, LWLOGC_VERSION_LEN, "%d.%d.%d", LWLOG_MAJOR_VERISON, LWLOG_MINOR_VERSION, LWLOG_PATCH_VERSION);
	return;
}


void LwlogcMessage(LWLogcLevel curLevel, int line, const char *funcName, const char *message)
{
	#if defined(NDEBUG)
	#else
	assert(funcName && message);
	#endif 

	char time_buf[TIME_NOW_BUF_SIZE] = {0};
	LwlogcTimeNow(time_buf);
	#if defined(POSIX_THREAD)
	pid_t pid;
	pid = getpid();
	
	/*
	 * format: 2020-08-12 星期2 17:48:31,982 LW_INFO funcName[line]: other param.
	 * 参考log4cxx的日志打印风格, lwlogc中增加进程pid and 线程id的打印.
	 */
	fprintf(LWLOG_STREAM, "%s %s (%d:%d) %s[%d]: %s\n", time_buf, lwlogcLeverStr[curLevel], 
		pid, pthread_self(), funcName, line, message);
	#else
	fprintf(LWLOG_STREAM, "%s %s %s[%d]: %s\n", time_buf, lwlogcLeverStr[curLevel], funcName, line, message);
	#endif 
	
	fflush(LWLOG_STREAM);
	return;
}


const char *LwlogcFormatLogMessage(const char * format, ...)
{
	int len = 0;
	va_list va;
	//char *buf_msg = (char *)realloc(1, (TIME_NOW_BUF_SIZE * 2));
	//if(!buf_msg) perror("realloc"), return "realloc failed.";
	char buf_msg[TIME_NOW_BUF_SIZE] = {0};
	
	va_start(va, format);
	/*
	 * The  functions  vprintf(), vfprintf(), vsprintf(), vsnprintf() are equivalent to the functions printf(), fprintf(), sprintf(), snprintf(),
     * respectively, except that they are called with a va_list instead of a variable number of arguments.   These  functions  do  not  call  the
     * va_end macro.  Because they invoke the va_arg macro, the value of ap is undefined after the call.  See stdarg(3)
	 */
	len = vsnprintf(buf_msg, TIME_NOW_BUF_SIZE - 1, format, va);
	printf("len: %d\n", len);
	va_end(va);
	return buf_msg;
}

//初始化lwlogc日志,
int LwlogcInit(const char *configFile)
{
	if(!configFile) {
		fputs("lwlogc_init null.", stderr);
		return false;
	}

	pthread_mutex_init(&logConfigure.mutex, NULL);
	
	logConfigure.filePath = NULL;
	logConfigure.convPattern = NULL;
	logConfigure.maxBackupIndex = 0;
	logConfigure.append = 0;
	logConfigure.maxFileSize = 0;
	logConfigure.logLevel = LW_INFO;
	//logConfigure.fp = NULL;

	FILE *fp = fopen(configFile, "r");
	if(!fp) {
		fputs("fopen configFile failed.", stderr);
		fclose(fp);
		return false;
	}
	
	pthread_t tid;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	
	if( 0 != pthread_create(&tid, &attr, LwlogcReadConf, (void*)fp)) {
		fputs("pthread_create failed.", stderr);
		pthread_attr_destroy(&attr);
		return false;
	}


	//打开写日志文件.
	while(!bInit) sleep(1);

	//{"LW_ERROR", "LW_WARN", "LW_INFO", "LW_DEBUG"};
	printf("filePath: %s, %d, %d, %d\n", logConfigure.filePath, logConfigure.logLevel, logConfigure.maxFileSize, logConfigure.maxBackupIndex);

	pthread_mutex_lock(&logConfigure.mutex);
	FILE *fpW = fopen(/*"/home/000/666.log"*/logConfigure.filePath, "w+");
	if(!fpW) {
		perror("fopen");
		return false;
	}
	pthread_mutex_unlock(&logConfigure.mutex);

	LwlogcSetStream(fpW);
	return true;
}




void* LwlogcReadConf(void *pFd)
{
	if(!pFd) {
		fputs("lwlogc_read_conf param is null.", stderr);
		return NULL;
	}

	/*************************************************************************************************
	 * r	: 只读模式打开文件, 流指针指向文件开始																	 *
	 * r+	: 可读写模式打开, 流指针指向文件开始																	 *
	 * w	: 只写模式打开, 文件存在, 则清空; 不存在, 则创建, 且流指针指向文件开始	                                             * 
	 * w+	: 可读写模式打开, 若文件存在, 清空; 不存在, 则创建, 且流指针指向文件开始                                               *
	 * a	: 以追加模式打开, 文件不存在, 会创建; 且流指针指向文件末尾. 所有写入都是追加到文件末尾.  *
	 * a+	: 以追加读写模式打开文件, 文件不存在, 会创建. 流指针指向文件末尾, 所有写入追加到文件末尾.*
	 *************************************************************************************************
	 */
	FILE *fp = (FILE*)pFd;
	char buf_conf[TIME_NOW_BUF_SIZE] = {0};
	
	for(; ; )
	{
		memset(buf_conf, 0x00, TIME_NOW_BUF_SIZE);
		pthread_mutex_lock(&logConfigure.mutex);
		for(; fgets(buf_conf, TIME_NOW_BUF_SIZE, fp) != NULL; memset(buf_conf, 0x00, sizeof(buf_conf))) //really read the data: size - 1
		{
			char *p = NULL;
			if('#' == buf_conf[0] || isspace(buf_conf[0])) continue;
			
			/* #if defined(_GNU_SOURCE) strcasestr
			 * The strstr() function conforms to C89 and C99.  The strcasestr() function is a nonstandard extension.
			 * 当在Getting SMBIOS data from sysfs.
			 * 	SMBIOS 2.7 present
			 * 该函数的使用出现问题.
			 */
			 
			if((p = strstr(buf_conf, "File"))) {
				//while(*(++p) != '/');
				char buf[512] = {0};
				int len = 0;
				len = sscanf(p, "%[^/]", buf);
				if(EOF == len 
				   || 0 == len
				   || -1 == len) {
					fputs("sscanf conf file failed.", stderr);
					return false;
				}

				len = strlen(buf_conf) - strlen(p); //忽略的字节数.
				
				logConfigure.filePath = (char*)calloc(1, (strlen(buf_conf) - len - strlen(buf)) + 1);
				if(!logConfigure.filePath) {
					perror("realloc");
					return false;
				}
				
				strncpy(logConfigure.filePath, buf_conf + len + strlen(buf), (strlen(buf_conf) - len - strlen(buf)));
				
			} else if((p = strstr(buf_conf, "MaxBackupIndex"))) {
				while(0 == isdigit(*++p));
				logConfigure.maxBackupIndex = atoi(p);
				
			} else if((p = strstr(buf_conf, "MaxFileSize"))) {
				while(0 == isdigit(*++p));
				logConfigure.maxFileSize = atoi(p);
				
			} else if((p = strstr(buf_conf, "Threshold"))) {
				while(*p++ != '=');
				while(isspace(*(++p)));
				if(!p) {
					logConfigure.logLevel = LW_INFO; //如果不填写, 则默认INFO
				}
				
				switch(p[0]) {
				case 'i':
				case 'I': {
					logConfigure.logLevel = LW_INFO;
					break;
				}

				case 'd': //0-error 1-warn 2-info 3-debug
				case 'D': {
					logConfigure.logLevel = LW_DEBUG;
					break;
				}

				case 'e':
				case 'E': {
					logConfigure.logLevel = LW_ERROR;
					break;
				}

				case 'w':
				case 'W': {
					logConfigure.logLevel = LW_WARN;
					break;
				}

				default: {
					fputs("lwlog.Threshold params err.", stderr);
					fclose(fp);
					return false;
				}
				}
			}
		}

		bInit = true;
		// file			level	maxsize maxindex
		// "",			 0, 	0, 		66
		// 666.log		 0, 	1024, 	66
		printf("@@@@file:%s, level:%d, maxsize:%d, maxindex:%d\n", logConfigure.filePath, 
			logConfigure.logLevel, logConfigure.maxFileSize, logConfigure.maxBackupIndex);
		pthread_mutex_unlock(&logConfigure.mutex);
		sleep(3);
	}

	pthread_attr_destroy(&attr);
	fclose(fp);
	return true;
}


//进程起来第一次时候检测log文件数量和对应name. 以及最后一个log文件的大小.
int LwlogcCouNum2SizeOfFiles(const char *logFilesPath)
{
	if(!logFilesPath) {
		fputs("LwlogcCouNum2SizeOfFiles parameter is empty.", stderr);
		return false;
	}

	//获取log文件的路径name
	char *p = logFilesPath;
	char *q = p + strlen(p);
	while(*--q != '/');
	char buf[512] = {0};
	strncpy(buf, q, strlen(p) - strlen(q) + 1/*最后一个'/'*/);

	//log4cxx的log文件格式规则: fileName.log, fileName.log1, fileName.log2, 也采取此方式.

	DIR *dir = NULL;
	struct dirent *rddir = NULL;
	
	dir = opendir((const char *)buf);
	if(!dir) {
		perror("opendir");
		//closedir(dir);
		return false;	
	}

	/* readdir读取完整个目录,返回NULL; 失败也返回NULL. 因此采取方法是:
	 * 每次调用readdir之前重置errno为0. 下一次调用判断返回值和errno.
	 * readdir仅有一个ERRORS, 即EBADF(Invalid directory stream descriptor dirp.)
	 * 因此, 可省去重置errno操作.
	 */

	struct LwlogcInfoPerFile perFileInfo;
	
	while(rddir = readdir(dir)) {
		if(0 == strncmp(rddir->d_name, ".", 1) 
		  || 0 == strncmp(rddir->d_name, "..", 2)) continue;

		if(DT_REG != rddir->d_type) continue;
		/*
		 * 如果文件属性是目录, 则也忽略. 只检测文件(仅d_name和d_info是POSIX标准)
		 * 其他为某些Linux所特有, 会降低代码移植性.
		 * DT_BLK:  block device; 		 DT_LNK:     symbolic link.
         * DT_CHR:  character device.	 DT_REG:     regular file.
         * DT_DIR:  directory			 DT_SOCK:    UNIX domain socket.
         * DT_FIFO: named pipe (FIFO).   DT_UNKNOWN: file type is unknown.
		 */

		char *f = q;
		f++;
		if(NULL == strstr(rddir->d_name, f)) continue;
		
		struct stat st;
		memset(&st, 0x00, sizeof(st));
		//LwlogcInfoPerFile 

		if(0 == stat(rddir->d_name, &st)) {
			//st.st_size;
		
		}
	}

	closedir(dir);
	return true;
}


