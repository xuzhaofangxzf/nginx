#ifndef __NGX_GLOBAL_H__
#define __NGX_GLOBAL_H__


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

extern char **g_os_argv;
extern char *gp_envmen;
extern int g_environlen;
extern char **environ;


extern pid_t       ngx_pid;
extern ngx_log_t   ngx_log;
#endif