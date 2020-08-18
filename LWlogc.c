/* ***********************************************************************************
 * @Function	: 								     *
 * @Version	: 1.0.0								     *
 * @Author	: lixiaogang5							     *
 * @Date	: 2020-08-12							     *
 * @Contact 	: lxiaogang5@gmail.com						     *
 * @Company	: HIKVISION                                                          *
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
#include "LWlogc.h"


#define LWLOG_STREAM LwlogcGetNowStream()
static FILE *gStream = 0;
static const char *lwlogcLeverStr[] = {"LW_ERROR", "LW_WARN", "LW_INFO", "LW_DEBUG"};
LWLogcConf logConfigure;
bool bInit = false;
static struct LwlogcInfoPerFile logFileinUse;

pthread_attr_t attr; //可采用静态初始化方式.

void LwlogcSetStream(FILE *stream)
{
	if(!stream) {
		fprintf(stderr, "[%s]lwlogc_set_stream param is null.", FUNCTION_NAME);
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
	assert(message);
	assert(funcName);
	int len = 0;
	char time_buf[TIME_BUF_LEN] = {0};
	LwlogcTimeNow(time_buf);
	#if defined(POSIX_THREAD)
	pid_t pid;
	pid = getpid();
	
	/*
	 * format: 2020-08-12 星期2 17:48:31,982 LW_INFO funcName[line]: other param.
	 * 参考log4cxx的日志打印风格, lwlogc中增加进程pid and 线程id的打印.
	 */
	len = fprintf(LWLOG_STREAM, "%s %s (%d:%lu) %s[%d]: %s\n", time_buf, lwlogcLeverStr[curLevel], 
		pid, pthread_self(), funcName, line, message);
	#else
	len = fprintf(LWLOG_STREAM, "%s %s %s[%d]: %s\n", time_buf, lwlogcLeverStr[curLevel], funcName, line, message);
	#endif 
	
	fflush(LWLOG_STREAM);
	if(message) free((void*)message), message = NULL;

	logFileinUse.fileSize += len;
	if(false == LwlogcCheckFileExceedsLimit(&logFileinUse.fileSize)) {
		fprintf(stderr, "[%s]LwlogcCheckFileExceedsLimit failed. but the process still continues.", FUNCTION_NAME);
		return;
	}
	
	return;
}


const char *LwlogcFormatLogMessage(const char * format, ...)
{
	va_list va;
	char *buf_msg = (char *)calloc(NUM_OF_ELEMENTS, TIME_NOW_BUF_SIZE);
	if(!buf_msg){
		perror("calloc");
		return "calloc memory failed.";
	}
	
	va_start(va, format);
	/*
	 * The  functions  vprintf(), vfprintf(), vsprintf(), vsnprintf() are equivalent to the functions printf(), fprintf(), sprintf(), snprintf(),
     * respectively, except that they are called with a va_list instead of a variable number of arguments.   These  functions  do  not  call  the
     * va_end macro.  Because they invoke the va_arg macro, the value of ap is undefined after the call.  See stdarg(3)
	 */
	vsnprintf(buf_msg, TIME_NOW_BUF_SIZE - 1, format, va);
	va_end(va);
	return buf_msg;
}

//初始化lwlogc日志,
int LwlogcInit(const char *configFile)
{
	if(!configFile) {
		fprintf(stderr, "[%s]lwlogc_init parameter is empty.", FUNCTION_NAME);
		return false;
	}

	LwlogcInitGlobalVarMemberList();
	
	FILE *fp = fopen(configFile, "r");
	if(!fp) {
		perror("fopen");
		LwlogcReleaseResources(fp);
		return false;
	}

	pthread_t tid;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if( 0 != pthread_create(&tid, &attr, LwlogcReadConf, (void*)fp)) {
		fprintf(stderr, "[%s] pthread_create failed.", FUNCTION_NAME);
		LwlogcReleaseResources(fp);
		return false;
	}


	//打开写日志文件.
	while(!bInit) sleep(1);
	if(false == LwlogcCouNum2SizeOfFiles(logConfigure.filePath)) {
		fprintf(stderr, "[%s]LwlogcCouNum2SizeOfFiles failed.", FUNCTION_NAME);
		LwlogcReleaseResources(fp);
		return false;
	}

	char buf[TIME_BUF_LEN] = {0};
	size_t buf_len = sizeof(buf);
	if(false == LWlogcNewestLogFile(buf, &buf_len)) {
		fprintf(stderr, "[%s]LWlogcNewestLogFile failed.", FUNCTION_NAME);
		LwlogcReleaseResources(fp);
		return false;
	}

	FILE *fpN = fopen(buf, "a+");
	if(!fpN) {
		perror("fopen");
		fclose(fpN);
		LwlogcReleaseResources(fp);
		return false;
	}
	
	LwlogcSetStream(fpN);
	return true;
}




void* LwlogcReadConf(void *pFd)
{
	if(!pFd) {
		fprintf(stderr, "[%s]lwlogc_read_conf param is null.", FUNCTION_NAME);
		return (void*)false;
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
		for(; fgets(buf_conf, TIME_NOW_BUF_SIZE, fp) != NULL; ) //really read the data: size - 1
		{
			char *p = NULL;
			if('#' == buf_conf[0] || isspace(buf_conf[0]) || isdigit(buf_conf[0])) continue;
			
			/* strcasestr() function is a nonstandard extension.*/
			if((p = strstr(buf_conf, "lwlog.File"))) {
				//while(*(++p) != '/');
				char buf[512] = {0};
				int len = 0;
				len = sscanf(p, "%[^/]", buf);
				if(EOF == len 
				   || 0 == len
				   || -1 == len) {
					fprintf(stderr, "[%s]sscanf conf file failed.", FUNCTION_NAME);
					LwlogcReleaseResources(fp);
					return (void*)false;
				}

				len = strlen(buf_conf) - strlen(p); //忽略的字节数.
				logConfigure.filePath = (char*)calloc(NUM_OF_ELEMENTS, (strlen(buf_conf) - len - strlen(buf)) + 1);
				if(!logConfigure.filePath) {
					perror("realloc");
					return (void*)false;
				}

				strncpy(logConfigure.filePath, buf_conf + len + strlen(buf), (strlen(buf_conf) - len - strlen(buf)));
				
			} else if((p = strstr(buf_conf, "MaxBackupIndex"))) {
				while(0 == isdigit(*++p));
				logConfigure.maxBackupIndex = atoi(p);
				
			} else if((p = strstr(buf_conf, "lwlog.MaxFileSize"))) {
				while(0 == isdigit(*++p));
				logConfigure.maxFileSize = atoi(p);
				
			} else if((p = strstr(buf_conf, "Threshold"))) {
				while(*p++ != '=');
				while(isspace(*(++p)));
				if(!p) {
					fprintf(stdout, "[%s]lwlog.Threshold Use default print level(stdout).", FUNCTION_NAME);
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
					fprintf(stderr, "[%s]lwlog.Threshold Parameter error.", FUNCTION_NAME);
					LwlogcReleaseResources(fp);
					return (void*)false;
				}
				}
			}
		}

		bInit = true;
		rewind(fp);
		sleep(5);
	}

	LwlogcReleaseResources(fp);
	return (void*)true;
}


//进程起来第一次时候检测log文件数量和对应name. 以及最后一个log文件的大小.
int LwlogcCouNum2SizeOfFiles(const char *logFilesPath)
{
	if(!logFilesPath) {
		fprintf(stderr, "[%s]LwlogcCouNum2SizeOfFiles parameter is empty.", FUNCTION_NAME);
		return false;
	}

	//获取log文件的路径name
	const char *p = logFilesPath;
	const char *q = p + strlen(p);
	while(*--q != '/');
	char buf[512] = {0};
	strncpy(buf, p, strlen(p) - strlen(q) + 1/*最后一个'/'*/);

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
	logConfigure.fp = (struct LwlogcInfoPerFile *)calloc(NUM_OF_ELEMENTS, sizeof(struct LwlogcInfoPerFile));
	if(!logConfigure.fp) {
		perror("calloc");
		return false;
	}

	logConfigure.fp->pre = logConfigure.fp->next = NULL;
	logConfigure.fp->fileName = NULL;
	logConfigure.fp->fileIndex = -1;
	logConfigure.fp->fileSize = 0;
	struct LwlogcInfoPerFile *pHead = NULL;
	
	while(rddir = readdir(dir)) {
		if(0 == strncmp(rddir->d_name, ".", 1) 
		  || 0 == strncmp(rddir->d_name, "..", 2)) continue;

		if(DT_REG != rddir->d_type) continue;

		//可能当前目录下有其他文件.
		char *str = NULL;
		str = strstr(q, ".");
		char fn[50] = {0};
		strncpy(fn, q + NUM_OF_ELEMENTS, strlen(q) - strlen(str) - NUM_OF_ELEMENTS);
		if(DT_REG == rddir->d_type 
		   && (!strstr(rddir->d_name, "log")
		   || !strstr(rddir->d_name, fn))) continue;
		/*
		 * 如果文件属性是目录, 则也忽略. 只检测文件(仅d_name和d_info是POSIX标准)
		 * 其他为某些Linux所特有, 会降低代码移植性.
		 * DT_BLK:  block device; 		 DT_LNK:     symbolic link.
         * DT_CHR:  character device.	 DT_REG:     regular file.
         * DT_DIR:  directory			 DT_SOCK:    UNIX domain socket.
         * DT_FIFO: named pipe (FIFO).   DT_UNKNOWN: file type is unknown.
		 */

		//char *f = q;
		//f++;
		//if(NULL == strstr(rddir->d_name, f)) continue;
		
		struct stat st;
		memset(&st, 0x00, sizeof(st));
		if(0 == stat(rddir->d_name, &st)) {
			struct LwlogcInfoPerFile  *pFile = (struct LwlogcInfoPerFile*)calloc(NUM_OF_ELEMENTS, sizeof(struct LwlogcInfoPerFile));
			if(!pFile) {
				perror("calloc");
				return false;
			}
			
			pFile->fileName = (char*)calloc(NUM_OF_ELEMENTS, strlen(rddir->d_name) + NUM_OF_ELEMENTS);
			if(!pFile->fileName) {
				perror("calloc");
				return false;
			}

			strncpy(pFile->fileName, rddir->d_name, strlen(rddir->d_name));
			pFile->fileSize = st.st_size;

			char *t = NULL;
			t = pFile->fileName;
			while(0 == isdigit(*++t) && *t != '\0');
			*t == '\0' ? pFile->fileIndex = 0 : (pFile->fileIndex = atoi(t));
	
			LwlogcAddNodeToLinked(pHead, pFile);
		}
	}

	closedir(dir);
	return true;
}


int LwlogcGetLogsNum(const struct LwlogcInfoPerFile *pHead)
{
	if(!pHead) {
		fprintf(stdout, "[%s]LwlogcGetLogsNum null pointer.", FUNCTION_NAME);
		return -1;
	}

	int num = 0;
	const struct LwlogcInfoPerFile *p = pHead;
	while(p->next) num++, p = p->next;

	return num;
}



int LwlogcDeleteOldLogFile()
{
	pthread_mutex_lock(&logConfigure.mutex);
	int minIndex = 0;
	struct LwlogcInfoPerFile *pHead = logConfigure.fp;
	struct LwlogcInfoPerFile *pT = NULL;
	
	while(pHead->next) pHead = pHead->next, (pHead->fileIndex < minIndex) ? minIndex = pHead->fileIndex : minIndex;
	while(pHead->pre) {
		if(minIndex == pHead->fileIndex) {
			//删除旧log文件.
			char buf[128] = {0};
			strncpy(buf, pHead->fileName, sizeof(buf));
			if(0 != unlink(buf)) {
				perror("unlink");
				return false;
			}

			if(!pHead->next) {
				//last node.
				pHead->pre->next = NULL;
			}else {
				//pT = pHead->pre;
				//pT->next = pHead->next;
				//pHead->next->pre = pT;

				pT = pHead->pre;
				pT->next = pHead->next;
				pHead->next->pre = pT;
			}

			assert(pHead->fileName);
			free(pHead->fileName);
			pHead->fileName = NULL;
			assert(pHead);
			free(pHead);
			pHead = NULL;

			return true;
		}

		pHead = pHead->pre;
	}

	pthread_mutex_unlock(&logConfigure.mutex);
	return true;
}


int LWlogcNewestLogFile(char *pLogBuf, const size_t *bufSize)
{
	int maxIndex = 0;

	if(0 == LwlogcGetLogsNum(logConfigure.fp)) {
		if(false == LwlogcCreateFirstLogFile(pLogBuf, bufSize)) {
			fprintf(stderr, "[%s]LwlogcCreateFirstLogFile failed.", FUNCTION_NAME);
			return false;
		}
	
		return true;
	}
	
	struct LwlogcInfoPerFile *pHead = NULL;
	pHead = logConfigure.fp;
	
	while(pHead->next)  pHead = pHead->next, (pHead->fileIndex > maxIndex) ? maxIndex = pHead->fileIndex : maxIndex;
	while(pHead->pre) {
		if(maxIndex == pHead->fileIndex) {
			
			//若该log文件已经超过用户指定预期的大小, 则新创建log文件.
			if((pHead->fileSize / CONVERT_BYTES_TO_MEGA) >= logConfigure.maxFileSize) {

				char buf[TIME_BUF_LEN] = {0};
				char arr[TIME_BUF_LEN/2] = {0};
				const char *t = pHead->fileName;
				while('.' != *(++t));
				strncpy(arr, pHead->fileName, strlen(pHead->fileName) - strlen(t));
				snprintf(buf, sizeof(buf), "%s.log%d", arr, ++maxIndex);
				memcpy(pLogBuf, buf, *bufSize);

				//new node and add to linked list.
				int logIndex = pHead->fileIndex;
				logIndex ++;
				
				struct LwlogcInfoPerFile *pFile = NULL;
				pFile = LwlogcCreateNewLogs(pLogBuf, &logIndex);
				if(!pFile) {
					fprintf(stderr, "[%s]LwlogcCreateNewLogs failed.", FUNCTION_NAME);
					return false;
				}

				struct LwlogcInfoPerFile *pHNode = NULL;
				LwlogcAddNodeToLinked(pHNode, pFile);
				
				//同时判断log文件数量是否超过用户指定数量.
				if(LwlogcGetLogsNum(logConfigure.fp) > logConfigure.maxBackupIndex) {
					//删除最旧的log文件.
					if(false == LwlogcDeleteOldLogFile()) {
						fprintf(stderr, "[%s]LwlogcDeleteOldLog failed.", FUNCTION_NAME);
						return false;
					}
				}

				return true;
			}

			strncpy(pLogBuf, pHead->fileName, *bufSize);
			pthread_rwlock_wrlock(&logFileinUse.rwlock);
			logFileinUse.fileName = pHead->fileName;
			logFileinUse.fileIndex = pHead->fileIndex;
			logFileinUse.fileSize = pHead->fileSize;
			pthread_rwlock_unlock(&logFileinUse.rwlock);
				
			return true;
		}

		pHead = pHead->pre;
	}
	
	if(!pHead->pre && 0 == strlen(pLogBuf)) return false;
	return true;
}

//0-false 1-true
const struct LwlogcInfoPerFile *LwlogcCreateNewLogs(const char *pFileName, const int *pFileIndex)
{
	assert(pFileName);
	struct LwlogcInfoPerFile *pFile  = (struct LwlogcInfoPerFile *)calloc(NUM_OF_ELEMENTS, sizeof(struct LwlogcInfoPerFile));
	if(!pFile) {
		perror("calloc");
		return false;
	}

	pFile->fileIndex = *pFileIndex;
	
	pFile->fileSize = 0;
	pFile->pre = pFile->next = NULL;
	pFile->fileName = (char *)calloc(NUM_OF_ELEMENTS, strlen(pFileName) + NUM_OF_ELEMENTS);
	assert(pFile->fileName);
	memcpy(pFile->fileName, pFileName, strlen(pFileName));

	//log file in use ...
	pthread_rwlock_wrlock(&logFileinUse.rwlock);
	logFileinUse.fileName = pFile->fileName;
	logFileinUse.fileSize = pFile->fileSize;
	logFileinUse.fileIndex = pFile->fileIndex;
	pthread_rwlock_unlock(&logFileinUse.rwlock);
	
	return pFile;
}


void LwlogcAddNodeToLinked(struct LwlogcInfoPerFile *pHead, struct LwlogcInfoPerFile *pFileNode)
{
	assert(pFileNode);
	pthread_mutex_lock(&logConfigure.mutex);
	pHead = logConfigure.fp;
	while(pHead->next) pHead = pHead->next;
	#if 0
	pFileNode->next = pHead->next;
	pFileNode->pre = pHead;
	if(pHead->next) pHead->next->pre = pFileNode;
	pHead->next = pFileNode;
	#else
	pHead->next = pFileNode;
	pFileNode->pre = pHead;
	pFileNode->next = NULL;
	#endif 
	pthread_mutex_unlock(&logConfigure.mutex);

	return;
}


int LwlogcCheckFileExceedsLimit(const int *pCurLength)
{
	assert(pCurLength);
	
	if((logFileinUse.fileSize / CONVERT_BYTES_TO_MEGA) >= logConfigure.maxFileSize) {
		
		//关闭旧文件句柄.
		FILE *fp = NULL;
		fp = LwlogcGetNowStream();
		fclose(fp);

		char buf[TIME_BUF_LEN] = {0}; //hik.log10 ~ hik.log2
		char name[TIME_BUF_LEN] = {0};
		int logIndex = logFileinUse.fileIndex;
		
		const char *p = logFileinUse.fileName + strlen(logFileinUse.fileName);
		while(isdigit(*--p)) ;
		strncpy(buf, logFileinUse.fileName, strlen(logFileinUse.fileName) - strlen(p) + NUM_OF_ELEMENTS);
		snprintf(name, TIME_BUF_LEN, "%s%d", buf, ++logIndex);

		struct LwlogcInfoPerFile *pHead = NULL;
		struct LwlogcInfoPerFile *pFile = NULL;
		pFile =	LwlogcCreateNewLogs((const char*)name, &logIndex);
		if(!pFile) {
			fprintf(stderr, "[%s]LwlogcCreateNewLogs failed.", FUNCTION_NAME);
			return false;

		}

		LwlogcAddNodeToLinked(pHead, pFile);

		if(LwlogcGetLogsNum(logConfigure.fp) > logConfigure.maxBackupIndex) {
			//删除最早log文件. -修改为sleep, 等待log删除线程继续处理.  ---待做
			if(false == LwlogcDeleteOldLogFile()) {
				fprintf(stderr, "%s[LwlogcDeleteOldLogFile] failed.", FUNCTION_NAME);
				return false;
			}
		}

		//获取最新的文件句柄
		fp = fopen(name, "a+");
		if(!fp) {
			perror("fopen");
			LwlogcSetStream(stdout);
			return false;
		}

		LwlogcSetStream(fp);
	}
	
	return true;
}



int LwlogcCreateFirstLogFile(char *pLogBuf, const size_t *bufSize)
{
	//表示当前目录下还未有log文件.
	const char *tail = logConfigure.filePath + strlen(logConfigure.filePath);
	while('/' != *--tail) ;

	//Ignore special characters such \r\n  \r, Repair log file name garbled caused by special characters
	const char *spec = tail + strlen(tail);
	while(0 == isalnum(*--spec)) ;
	int len = strlen(tail) - strlen(spec);
	strncpy(pLogBuf, tail + NUM_OF_ELEMENTS, *bufSize > len ? len : *bufSize);
	pLogBuf[strlen(pLogBuf)] = '\0';

	//将该节点加入fp链表中进行管理.
	int logIndex = 0;
	struct LwlogcInfoPerFile *pFile = NULL;
	pFile = LwlogcCreateNewLogs((const char*)pLogBuf, &logIndex);
	if(!pFile) {
		fprintf(stderr,"[%s]LwlogcCreateNewLogs failed.", FUNCTION_NAME);
		return false;
	}

	struct LwlogcInfoPerFile *pHead = NULL;
	LwlogcAddNodeToLinked(pHead, pFile);
	return true;
}


void LwlogcReleaseResources(FILE *pNowStream)
{
	if(pNowStream) fclose(pNowStream);

	pthread_rwlock_destroy(&logFileinUse.rwlock);
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&logConfigure.mutex);
	return;
}


void LwlogcInitGlobalVarMemberList()
{
	logConfigure.filePath = NULL;
	logConfigure.convPattern = NULL;
	logConfigure.maxBackupIndex = 0;
	logConfigure.append = 0;
	logConfigure.maxFileSize = 0;
	logConfigure.logLevel = LW_INFO;
	logConfigure.fp = NULL;
	pthread_mutex_init(&logConfigure.mutex, NULL);

	logFileinUse.fileName = NULL;
	logFileinUse.fileIndex = -1;
	logFileinUse.fileSize = 0;
	logFileinUse.pre = logFileinUse.next = NULL;
	pthread_rwlock_init(&logFileinUse.rwlock, NULL);

	return;
}

