xhnet
=====

是由c++编写的一个服务器基础库。
====

##一、简介##
###1、跨windows和linux平台###
###2、需要libevent、libiconv、log4cplus等第三方库###
###3、windows下vs2013编译通过###
###4、centos编译通过###
###5、使用了c++11特性###
 


##二、包括以下几个功能##

###1、异常处理机制，dump、异常抛出不直接崩溃程序、lua远程调试机制###
###2、文件日志###
###3、性能检查XH_ACTION_GUARD、XH_ACTION_LOGINFO###
###4、一些小工具md5、iconv、XHGUARD、XH_STACK_TRACE###
###5、集成SMART_ASSERT，例如XH_ASSERT(a>100&&b>10)(a);如果a>100&&b>10不为true，会产生ERROR日志，且打印出a、b的值###
###6、内存管理###
###7、网络io###
###8、一些时间、文件、字符串常用函数###
