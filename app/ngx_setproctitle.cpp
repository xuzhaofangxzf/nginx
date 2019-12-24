#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //environ
#include <string.h>

#include "ngx_global.hpp"

//设置可执行程序标题相关函数：分配内存，并且把环境变量拷贝到新内存中来
void ngxInitSetProcTitle()
{
    int i;
    //统计环境变量所占的内存长度。注意判断方法是environ[i]是否为空作为环境变量结束标记
    for (i = 0; environ[i] != NULL; i++)
    {
        g_environlen += strlen(environ[i]) + 1; //srrlen求的是没有'\0'的长度

    }
    //申请同样大小的内存，以便存放环境变量的内容
    gp_envmen = new char[g_environlen];
    memset(gp_envmen, 0, g_environlen);
    char *ptmp = gp_envmen;

    //把原来的内存内容搬到新地方来
    for (int i = 0; environ[i] != NULL; i++)
    {
        size_t size = strlen(environ[i]) + 1;
        strcpy(ptmp, environ[i]);
        environ[i] = ptmp;
        ptmp += size;
    }
    return;
    
    
}

//设置可执行程序标题
void ngxSetProcTitle(const char* title)
{
    size_t titlelen = strlen(title) + 1;
    //计算总的原始argv那块内存的长度【包括各种参数】
    size_t e_environlen = 0;    //e表示局部变量
    for (int i = 0; g_os_argv[i] != NULL; i++)
    {
        e_environlen += strlen(g_os_argv[i]) + 1;

    }
    size_t totallen = e_environlen + g_environlen; //argv and environ length summary
    if (totallen < titlelen)
    {
        printf("the title of the process is too long, there is no space enough to store it!\n");
        return;
    
    }

    g_os_argv[1] = NULL;
    
    char* ptmp = g_os_argv[0];
    strcpy(ptmp, title);
    ptmp += titlelen;
    
    size_t leftstorage = totallen - titlelen;
    memset(ptmp, 0, leftstorage);
    return;
}
