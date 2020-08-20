@[toc]

# 1. LWlogc简介

​        <font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;LWlogc是一款基于纯C开发的轻量级日志(log)记录框架。主要由两个文件组成，分别是：LWlogc.c和LWlogc.h；代码总行数在千行左右，非常轻量和简洁， API通俗易懂。LWlogc日志记录框架能非常友好地嵌入到任何你所要开发的项目中，它具有良好的移植性；并且它占用的资源（包括但不限于内存、磁盘空间、CPU等）非常之低。LWlogc具有高稳定性、强可靠性、可扩展性等特点。</font>


## 1.1 LWlogc特性

### 1.1  lwlog.properties 配置文件

​      <font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;LWlogc进程启动时候，会先读取该进程目录下的lwlog.properties配置文件，配置文件中各参数的配置正确与否对该日志记录服务能否正常工作起到了决定性的作用。因此，下面先对该配置文件中的参数作一介绍。</font>

​	  <font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;以下是该配置文件中的所有参数:</font>

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
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;该参数指定log文件存储于磁盘上面的具体位置。如: </font> 
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;/data1/lixiaogang5/work/OpenSource/LWlogc/test/<font face="Lucida Console" size=4 color = blue>hik.log</font></font> 
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;值得注意的是， log文件名（lxg.log）前面的目录部分需保证是已存在的，若不存在，LWlogc日志记录库初始化时候会报错提示目录不存在；并在失败时候将日志打印默认输出到终端(标准输出)。 log文件不存在， LWlogc会自动创建，比如第一次启动你的服务时候，目录下面肯定是不存在log文件的，此时LWlogc日志记录库会检测，是否需要创建。</font> 

<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;(1) 若用户指定目录下, 没有log文件, 则创建第一个log文件，命名为: 用户指定log名字.log。如上例中的: hik.log。</font> 

<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;(2) 若用户指定目录下, 存在log文件，则读取最新的一个log文件名以及序号(即上次进程关闭时候在写的log文件)；若该文件大小已经超过用户指定的大小（1.1.3中MaxFileSize），则创建一个新的log文件，且该文件名叫上一个log文件名序号递增（比如上一个log文件名是: hik.log10, 那么本次创建的log文件名是: hik.log11）。如果此时目录下log文件数量已经超过了用户指定的数量（1.1.2节MaxBackupIndex），则删除最早的log文件。</font> 

<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;(3) lwlog.File配置，因考虑到实际情况，暂不支持动态修改。</font> 
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;</font> 
     
#### 1.1.2 lwlog.MaxBackupIndex
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;MaxBackupIndex参数指明用户希望保存在指定目录下的log文件数量。当log文件数量超过该限制之后，会删除最早(最旧)的一个log文件。该参数支持动态修改。即第一次指定10个，然后立刻修改为20个，是立刻生效的。即LWlogc日志记录库会统计，满足20个之后才删除最早文件。</font> 
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;</font> 

#### 1.1.3 lwlog.MaxFileSize
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;MaxFileSize参数指定每个log文件的大小。该参数默认单位是MB。 因此在统计日志磁盘空间是否充裕时候，你的计算方式是这样的：</font> 

<font face="Lucida Console" size=4 color = blue>&ensp;&ensp; &ensp;&ensp;需要磁盘空间 = MaxBackupIndex（文件数量） * MaxFileSize(每个文件大小)</font> 

<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;MaxFileSize是支持动态修改的，且立刻生效。</font> 
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;</font> 
#### 1.1.4 lwlog.Threshold
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;Threshold参数用于限制或是控制log文件的打印等级。当前LWlogc日志记录库框架支持四种打印级别，分别是：<font face="Lucida Console" size=4 color = red>ERROR、WARN、INFO和DEBUG（值从低到高）</font>。 若不想DEBUG的日志信息打印并输出到log文件中，那么该参数可以设置为INFO， 这样高于INFO级别的所有日志都不会被打印；同理，若不想输出INFO级别的日志打印，则该参数填写为WARN，依次类推。如果因业务需求，当前四种打印等级不满足实际情况，也可以代码中增加，比如TRACE、FATAL等，也是非常方便的，它具有良好的可拓展性。</font> 

<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;同理，该参数是支持动态修改的。并且在配置文件中，其值不关心大小写，即INFO等价于info/Info/iNFO等。</font> 


# 2. LWlogc代码架构
## 2.1 LWlogc日志记录库文件目录结构图
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;在第1节中提高过LWlogc日志记录框架库非常轻量，仅两个文件。下图是LWlogc项目的文件目录结构图。</font>

![在这里插入图片描述](https://img-blog.csdnimg.cn/20200820124035547.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2xpeGlhb2dhbmdfdGhlYW5zd2Vy,size_16,color_FFFFFF,t_70#pic_center)
![image](https://img-blog.csdnimg.cn/20200820124035547.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2xpeGlhb2dhbmdfdGhlYW5zd2Vy,size_16,color_FFFFFF,t_70#pic_center)
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;该图详细的说明了LWlogc日志库中各文件的分布以及其功能。这也是你Download该目录之后的文件结构布局图。</font> 
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;</font> 

## 2.2 LWlogc服务启动流程
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;对于LWlogc日志记录框架库，其服务的启动流程逻辑结构图如下所示。该图较为详细的阐述了服务启动的细节和核心。以及对于各环节的出错处理。阅读源码时候，结合本图，会更加高效。</font>

![在这里插入图片描述](https://img-blog.csdnimg.cn/20200820165425615.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2xpeGlhb2dhbmdfdGhlYW5zd2Vy,size_16,color_FFFFFF,t_70#pic_center)
![image](https://img-blog.csdnimg.cn/20200820165425615.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2xpeGlhb2dhbmdfdGhlYW5zd2Vy,size_16,color_FFFFFF,t_70#pic_center)
## 2.3 使用LWlogc
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;若将该LWlogc应用于你项目中，可以有两种方式：</font>
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;方式一: 直接将LWlogc.c和LWlogc.h两个文件嵌入到你的项目中，然后使用.h文件中定义的四个宏。LWLOGC_ERROR、LWLOGC_WARN、LWLOGC_INFO、LWLOGC_DEBUG。当然你可也完全可以根据实际项目所要对其进行宏重新定义。</font>

<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;方式二: 使用LWlog.*同级目录下的Makefile编译出LWlogc日志记录库的动态库 libLWlogc.so， 然后将该动态库放在你的项目中，编译成果物物时候链接进去。对于该方式的具体使用可以参考同级目录test/下的main.c测试文件的所有。</font>

## 2.4 LWlogc内存、cpu占用
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;测试环境(VMware虚拟机环境)</font>

<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;(1) 系统平台: VMware Virtual Platform</font>
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;(2) CPU:  Intel(R) Core(TM) i7-6500U CPU @ 2.50GHz</font>
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;(3) 内存: MemTotal:        2006924 kB(2GB)</font>
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;(4) 操作系统版本: Ubuntu 20.04</font>
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;(5) 磁盘型号: HDD(机械硬盘) 20GB</font>

<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;该数据使用test/目录下的main.c文件中的数据测试所得，但是条件修改了，死循永不停息地重复打印日志、写磁盘、读磁盘、删log文件、创建新log文件等等。因此对CPU的占用稍微偏高，这是在压测环境下所的的数据，仅供参考。</font>

![在这里插入图片描述](https://img-blog.csdnimg.cn/20200820235837651.png#pic_center)
### 2.4.1 LWLogc使用效果
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;当前lwlog.properties配置文件其中几个配置的参数如下所示：</font>

<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;即log文件名是hik.logxx； 每个log文件大小是800MB，保留的文件数量3个。当文件数量超过3个时候，会删除最早的log文件(hik.log50), 然后新建log文件(hik.log52)。 </font>
```c
# log文件路径
lwlog.File = /home/lixiaogang5/Code/LWlogc-master/test/hik.log

# log文件数量
lwlog.MaxBackupIndex = 3

# 每个log文件大小，MB
lwlog.MaxFileSize = 800
```

![在这里插入图片描述](https://img-blog.csdnimg.cn/20200821001102685.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2xpeGlhb2dhbmdfdGhlYW5zd2Vy,size_16,color_FFFFFF,t_70#pic_center)

<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;节省篇幅，更多的配置结果现象就不一一阐述了。</font>


# 3. LWlogc依赖库
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;LWlogc当前仅依赖POSIX线程库，即libpthread.so。</font> 

# 4. LWlogc位于gitbuh具体位置
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;该日志记录框架库是由我本人利用业余时间单独开发，其具体位置在: [<font face="Lucida Console" size=4 color = red><u>LWlogc</u><font>](https://github.com/lixiaogang5/LWlogc)。 欢迎使用，也非常欢迎大家的更好建议。</font>

# 5. LWlogc待完成任务
<font face="Lucida Console" size=4 color = black>&ensp;&ensp; &ensp;&ensp;当前的LWlogc能够很好的应用。但是它还存在着一些局限性。比如一个项目里面，有很多个模块划分；协议处理模块、数据库数据模块、图片存储模块等等。然后希望每个不同模块用不同的log日志文件去分别记录，分门别类，无论是排查问题，还是检索数据，目标都会显得更加清晰、高效。</font>
