#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //environ
#include <string.h>
/*因为environ是一个全局的外部变量，所以切记使用前要用extern关键字进行声明，然后在使用。
unistd.h头文件中声明了这个变量，所以也可以将unist.h进行include，这个时候就不用再对environ进行extern声明了（应为unistd.h中已经声明了）
*/
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
        strncpy(ptmp, size, environ[i]);
        #if 0
        strcpy(ptmp, environ[i]);
        environ[i] = ptmp; //这行代码可以删除?
        #endif
        ptmp += size;
    }
    return;
    
    
}

//设置可执行程序标题
void ngxSetProcTitle(const char* title)
{
    size_t titlelen = strlen(title) + 1;
    //计算总的内存长度

    size_t totallen = g_envneedmem + g_argvneedmem; //argv and environ length summary
    if (totallen < titlelen)
    {
        printf("the title of the process is too long, there is no space enough to store it!\n");
        return;
    
    }
    //(3)设置后续的命令行参数为空，表示只有argv[]中只有一个元素了，这是好习惯；防止后续argv被滥用，因为很多判断是用argv[] == NULL来做结束标记判断的;
    g_os_argv[1] = NULL;
    
    char* ptmp = g_os_argv[0]; //指向开始的位置
    strncpy(ptmp, titlelen, title);
    ptmp += titlelen;
    
    size_t leftstorage = totallen - titlelen; //把剩下的内存清零，不然有可能会显示乱码
    memset(ptmp, 0, leftstorage);
    return;
}
