# LWlogc
1. LWlogc简介
   LWlogc是一款轻量级的log文件库。它使用纯C编写，主要由两个文件组成，分别是：LWlogc.c和LWlogc.h；代码总行数不到1000行，非常轻量和简洁。
占用CPU和内存少。

1.1 LWlogc日志打印级别
   

   等号（"="）后面可以有一个或多个空格，或是无空格。打印等级不区分大小写。 即: ERROR和error书写是等效的。

1.1 LWlogc特性
    （1） 它支持用户指定的log日志存储目录，若该目录下无用户指定log文件， 则创建并写入log信息到该文件；配置log文件存储目录:
    				lwlog.File = /home/000/666.log

    （2） 支持用户动态修改log存储文件数量
    	    		lwlog.MaxBackupIndex = 66

    （3） 支持用户动态修改每个log文件大小（MB)
    				lwlog.MaxFileSize = 1024

    （4） 支持用户动态修改log打印级别 
          LWlogc当前支持ERROR、WARN、INFO和DEBUG共四种打印级别。它支持动态修改，即在你的进程服务运行时候，动态调整等级。 
					lwlog.Threshold = ERROR 
   
     以上选项都是在当前运行进程服务的同级目录下的配置文件lwlog.properties中。 对于该配置文件中的各选项的等号（“=”）后面，支持有一个
     或多个空格，同时对于log打印等级，是不区分大小写的， 即ERROR等效于error。
    

