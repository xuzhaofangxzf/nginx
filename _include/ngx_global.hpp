#ifndef __NGX_GLOBAL_H__
#define __NGX_GLOBAL_H__

//#include <sig_atomic_t.h> //sig_atomic_t


#include <signal.h> 

#include "ngx_c_socket.hpp"
//类型定义--------------

typedef struct 
{
    char ItemName[50];
    char ItemContent[500];
}CConfItem, *LPCConfItem;


//和运行日志相关 
typedef struct
{
	int    log_level;   //日志级别 或者日志类型，ngx_macro.h里分0-8共9个级别
	int    fd;          //日志文件描述符

}ngx_log_t;



//外部全局变量的声明
extern size_t       g_argvneedmem;    //argv参数所需要的内存大小
extern size_t       g_envneedmem;         //环境变量所占内存大小
extern int          g_os_argc;              //参数个数
extern char         **g_os_argv;
extern char         *gp_envmen;
extern int          g_environlen;
extern char         **environ;

extern ngx_log_t    ngx_log;

extern pid_t        ngx_pid;
extern pid_t        ngx_parent;
extern int          ngx_process;
extern sig_atomic_t ngx_reap;

extern CSocket       g_socket; 
#endif