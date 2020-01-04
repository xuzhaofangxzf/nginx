//和守护进程相关

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>     //errno
#include <sys/stat.h>
#include <fcntl.h>


#include "ngx_func.hpp"
#include "ngx_macro.hpp"
#include "ngx_c_conf.hpp"

/*
* 函数名称: ngx_daemon
* 函数描述: 守护进程初始化
* 函数参数:
*     空
* 返回值:
*    失败： -1
*    子进程：0，
*    父进程: 1
*/
int ngx_daemon()
{
    //（1）创建守护进程的第一步，fork()一个子进程
    switch (fork())
    {
    case -1:
        /* failed */
        ngx_log_error_core(NGX_LOG_EMERG, errno,"ngx_daemon:fork() failed!");
        break;
    case 0:
    //子进程，直接break，执行下面的操作
        break;
    default:
        //父进程以往 直接退出exit(0);现在希望回到主流程去释放一些资源
        return 1;  //父进程直接返回1；
    }
    //只有子进程才能走到这个流程
    ngx_parent = ngx_pid;   //ngx_pid是原来父进程的id，因为这里是子进程，所以子进程的ngx_parent设置为原来父进程的pid
    ngx_pid = getpid();     //当前进程的id
    
    //（2）脱离终端，终端关闭，将跟此子进程无关
    if (setsid() == -1)
    {
        ngx_log_error_core(NGX_LOG_EMERG, errno,"ngx_daemon()中setsid()失败!");
        return -1;
    }
    //(3)设置文件mask为0，不要让它限制文件的权限。
    umask(0);
    //(4)将输出重定向到设备黑洞中去
    int fd = open("/dev/null", O_RDWR);
    if(fd == -1)
    {
        ngx_log_error_core(NGX_LOG_EMERG, errno, "ngx_daemon: open(\"/dev/null\") failed!");
        return -1;
    }
    if (dup2(fd, STDIN_FILENO) == -1)
    {
       ngx_log_error_core(NGX_LOG_EMERG,errno,"ngx_daemon()中dup2(STDIN)failed!");
       return -1; 
    }
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
       ngx_log_error_core(NGX_LOG_EMERG,errno,"ngx_daemon()中dup2(STDOUT)failed!");
       return -1; 
    }
    if (fd > STDERR_FILENO)  //fd应该是3，这个应该成立
     {
        if (close(fd) == -1)  //释放资源这样这个文件描述符就可以被复用；不然这个数字【文件描述符】会被一直占着；
        {
            ngx_log_error_core(NGX_LOG_EMERG,errno, "ngx_daemon()中close(fd)失败!");
            return -1;
        }
    }
    return 0;
}