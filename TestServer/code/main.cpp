#include <cstdio>

#include"../../share/ShareFunction.h"
#include"unistd.h"

#include "AppManager.h"

int main()
{
    printf("%s 向你问好!\n", "TestLinux");
    app::run();
	//share::InitData();
 //   usleep(10000*1000);
    return 0;
}