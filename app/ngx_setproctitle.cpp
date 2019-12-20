#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //environ
#include <string.h>

#include "ngx_global.hpp"

extern char **environ;

//设置可执行程序标题相关函数：分配内存，并且把环境变量拷贝到新内存中来
void ngxInitSetProcTitle()
{
    //int i;
    //统计环境变量所占的内存长度。注意判断方法是environ[i]是否为空作为环境变量结束标记
    // for (i = 0; environ[i]; i++)
    // {
    //     g_environlen += strlen(environ[i]) + 1; //srrlen求的是没有'\0'的长度

    // }


    
    
}