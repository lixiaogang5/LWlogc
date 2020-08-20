[TOC]

# 1. LWlogc简介

​        <font face="Lucida Console" size=4 color = black>LWlogc是一款基于纯C开发的轻量级日志(log)记录框架。主要由两个文件组成，分别是：LWlogc.c和LWlogc.h；代码总行数在千行左右，非常轻量和简洁， API通俗易懂。LWlogc日志记录框架能非常友好地嵌入到任何你所要开发的项目中，它具有良好的移植性；并且它占用的资源（包括但不限于内存、磁盘空间、CPU等）非常之低。LWlogc具有高稳定性、强可靠性、可扩展性等特点。</font>


## 1.1 LWlogc特性

### 1.1  lwlog.properties 配置文件

​      <font face="Lucida Console" size=4 color = black>LWlogc进程启动时候，会先读取该进程目录下的lwlog.properties配置文件，配置文件中各参数的配置正确与否对该日志记录服务能否正常工作起到了决定性的作用。因此，下面先对该配置文件中的参数作一介绍。</font>

​	  <font face="Lucida Console" size=4 color = black>以下是该配置文件中的所有参数:</font>

```shell
# (1) log文件路径
lwlog.File = /data1/lixiaogang5/work/OpenSource/LWlogc/test/lxg.log

# (2) log文件数量(保留磁盘log文件)
lwlog.MaxBackupIndex = 66

# (3) 每个log文件大小，(单位MB)
lwlog.MaxFileSize = 1024

# (4) log文件追加(暂时不使用)
lwlog.Append = TRUE

# (5) log打印等级(ERROR、WARN、INFO、DEBUG)
lwlog.Threshold = ERROR

# (6) log文件名模式(暂时不使用)
lwlog.ConversionPattern = %Y-%M-%D
```

   

#### 1.1.1 lwlog.File

#### 1.1.2 lwlog.MaxBackupIndex

#### 1.1.3 lwlog.MaxFileSize

#### 1.1.4 lwlog.Threshold



   <font face="Lucida Console" size=4 color = black> LWlogc日志；当前支持四种打印等级，分别是：ERROR、WARN、INFO和DEBUG（值从低到高），当配置文件设置参数<u>ThreShold</u>设置为INFO级别时候。而且支持动态更改其配置信息，</font>

（1） 它支持用户指定的log日志存储目录，若该目录下无用户指定log文件， 则创建并写入log信息到该文件；配置log文件存储目录:
    		lwlog.File = /home/000/666.log

（2） 支持用户动态修改log存储文件数量
    		lwlog.MaxBackupIndex = 66

（3） 支持用户动态修改每个log文件大小（MB)
     		lwlog.MaxFileSize = 1024

（4） 支持用户动态修改log打印级别 
          LWlogc当前支持ERROR、WARN、INFO和DEBUG共四种打印级别。它支持动态修改，即在你的进程服务运行时候，动态调整等级。 
		lwlog.Threshold = ERROR 

​      以上选项都是在当前运行进程服务的同级目录下的配置文件lwlog.properties中。 对于该配置文件中的各选项的等号（“=”）后面，支持有一个或多个空格，同时对于log打印等级，是不区分大小写的， 即ERROR等效于error。

## 1.2 LWlogc使用   


### 1.2.1 编译LWlogc动态库 

 <font face="Lucida Console" size=4 color = black>LWlogc目录下有一个Makefile文件, 使用make工具编译可得到对应的libLWlogc.so动态库文件，直接将该库链接到你的项目中，编译使用即可。</font>

1.2.2 





# 2. LWlogc框架

```shell
.
├── lib
│   └── libLWlogc.so
├── LWlogc.c
├── LWlogc.h
├── lwlog.properties
├── Makefile
├── README.md
└── test
    ├── lwlog.properties
    ├── main.c
    └── Makefile
```

![在这里插入图片描述](https://img-blog.csdnimg.cn/20200820122919489.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2xpeGlhb2dhbmdfdGhlYW5zd2Vy,size_16,color_FFFFFF,t_70#pic_center)

## 2.1  LWlogc流程图
