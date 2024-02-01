#include <cstdio>

#include"../../share/ShareFunction.h"
#include"unistd.h"

int main()
{
    printf("%s 向你问好!\n", "TestLinux");
	share::InitData();
    usleep(10000*1000);
    return 0;
}