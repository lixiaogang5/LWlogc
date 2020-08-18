/* ***********************************************************************************
 * @Function	: 								     								 *
 * @Version		: 1.0.0								     							 *
 * @Author		: lixiaogang5							     						 *
 * @Date		: 2020-08-12							     						 *
 * @Contact 	: lxiaogang5@gmail.com						     					 *
 * @Company		: HIKVISION                                                          *
 * ***********************************************************************************
 */

#ifndef LWLOGC_H
#define LWLOGC_H

#include <stdio.h>
#include <pthread.h>


#ifdef __cplusplus
extern "C"{
#endif 

#if !defined(PRINTF_LIKE)
#if defined(__GNUC__) || defined(__clang__) || defined(__TI_COMPILER_VERSION__)
#define PRINTF_LIKE(f, a) __attribute__((format(printf, f, a)))
#else
#define PRINTF_LIKE(f, a)
#endif
#endif

#if !defined(WEAK)
#if (defined(__GNUC__) || defined(__clang__) || \
     defined(__TI_COMPILER_VERSION__)) &&       \
    !defined(_WIN32)
#define WEAK __attribute__((weak))
#else
#define WEAK
#endif
#endif

#ifdef __GNUC__
#define NORETURN __attribute__((noreturn))
#define NOINLINE __attribute__((noinline))
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#define NOINSTR __attribute__((no_instrument_function))
#define DO_NOT_WARN_UNUSED __attribute__((unused))
#else
#define NORETURN
#define NOINLINE
#define WARN_UNUSED_RESULT
#define NOINSTR
#define DO_NOT_WARN_UNUSED
#endif /* __GNUC__ */



#define P_UNIX 		0 //类UNIX平台
#define P_WINCE 	1 //Windows CE 
#define P_WINDOWS 	2 //Windows
#define P_OS2		3 //
#define P_OPENBSD	4 //
//更多预定义编译器宏参考: https://sourceforge.net/p/predef/wiki/OperatingSystems/

#ifndef PLATFORM_ARCH
#if defined(__unix__) || defined(__APPLE__)
#define PLATFORM_ARCH P_UNIX
#elif defined(WINCE)
#define PLATFORM_ARCH P_WINCE
#elif defined(_WIN32)
#define PLATFORM_ARCH P_WINDOWS
#endif 
#endif //PLATFORM_ARCH

#if !defined(PLATFORM_ARCH)
#error "The corresponding platform type was not found."
#endif 


#if defined(PLATFORM_ARCH)
#ifndef POSIX_THREAD
#define POSIX_THREAD
#endif 
#endif 


#define LWLOG_MAJOR_VERISON 1
#define LWLOG_MINOR_VERSION 0
#define LWLOG_PATCH_VERSION 0

#define LWLOGC_VERSION_LEN	20
#define TIME_NOW_BUF_SIZE 1024
#define WEEK "星期"

#define NUM_OF_ELEMENTS	1
#define CONVERT_BYTES_TO_MEGA	(1024 * 1024)
#define TIME_BUF_LEN	128
#define FUNCTION_NAME	__FUNCTION__

typedef enum {
	false,
	true,
}bool;


/**
*  @name Debug levels
*/
typedef enum
{
	LW_ERROR = 0,
	LW_WARN	 = 1, 
	LW_INFO  = 2,
	LW_DEBUG = 3, 
	//next id: 5
}LWLogcLevel;


struct LwlogcInfoPerFile{
	struct LwlogcInfoPerFile *pre, *next;
	pthread_rwlock_t  rwlock;
	char *fileName;
	int fileSize;
	int fileIndex; //当前目录下log文件的最大索引号. eg: test.log.10, 则fineIndex = 10
};


/**
*  @name LWlogc properties
*/
typedef struct
{
	char *filePath;			//文件路径(用户必填)
	char *convPattern;		//匹配模式(暂时不使用)
	int maxBackupIndex;		//log文件数量, 默认1个
   	unsigned short append;	//追加(暂时不使用)
	long long maxFileSize;	//每个log文件的大小(MB)
	LWLogcLevel logLevel;   //日志等级, 默认LW_INFO
	pthread_mutex_t mutex;
	struct LwlogcInfoPerFile *fp;  //目录下列表文件信息.
}LWLogcConf;



extern LWLogcConf logConfigure;

/**
*  @name LWlogc now time
*/
static const char *LwlogcTimeNow(char *time_buf);


/**
*  @name LWlogc version
*/
/** @fn 	
 *	@brief	
 *	@param	hHandle    [IN] 	- 
 *	@param	pstModel   [IN] 	- 
 *	@param	stMdlType  [IN] 	- 
 *	@param	pModelList [OUT]	- 
 *	@return 错误码
 */

extern void LwlogcVersion(char *dst);

/**
*  @name LWlogc debug message(ingore file name)
*/
extern void LwlogcMessage(LWLogcLevel curLevel, int line, const char *funcName, const char *message);


/**
*  @name LWlogc set i/o steam
*/
extern void LwlogcSetStream(FILE *stream);


/**
*  @name LWlogc obtain i/o stream
*/
extern FILE *LwlogcGetNowStream();


/**
*  @name LWlogc format log message
*/
extern const char *LwlogcFormatLogMessage(const char * format, ...);

extern int LwlogcInit(const char *filePath);

void* LwlogcReadConf(void *pFd);

extern int LwlogcCouNum2SizeOfFiles();

extern int LwlogcGetLogsNum(const struct LwlogcInfoPerFile *pHead);

extern int LwlogcDeleteOldLogFile();

extern int LWlogcNewestLogFile(char *pLogBuf, const size_t *bufSize);

extern const struct LwlogcInfoPerFile *LwlogcCreateNewLogs(const char *pFileName, const int *pFileIndex);

extern void LwlogcAddNodeToLinked(struct LwlogcInfoPerFile *pHead, struct LwlogcInfoPerFile *pFileNode);

extern int LwlogcCheckFileExceedsLimit(const int *pCurLength);

extern int LwlogcCreateFirstLogFile(char *pLogBuf, const size_t *bufSize);

extern void LwlogcReleaseResources(FILE *pNowStream);

extern void LwlogcInitGlobalVarMemberList();
#ifdef __cplusplus
}
#endif 

#endif /*LWLOG_H*/



