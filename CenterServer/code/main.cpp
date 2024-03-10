#include <cstdio>

#include"../../share/ShareFunction.h"
#include"unistd.h"

#include "AppManager.h"
#include <signal.h>

int main()
{
    //SIGPIPE 是一个 POSIX 信号，表示在写到一个已关闭的管道（或类似的文件描述符）时发生了错误。默认情况下，如果程序向已关闭的管道写入数据，操作系统会向该程序发送 SIGPIPE 信号，以通知它发生了错误。

    //    通过调用 signal(SIGPIPE, SIG_IGN); ，程序告诉操作系统在发生 SIGPIPE 信号时不要终止程序，而是简单地忽略这个信号。这通常在程序不希望因为向已关闭的管道写入而终止时使用，而希望通过返回错误来处理这种情况。
    signal(SIGPIPE, SIG_IGN);//
    printf("%s 向你问好!\n", "TestLinux");
    app::run();
	//share::InitData();
 //   usleep(10000*1000);
    return 0;
}