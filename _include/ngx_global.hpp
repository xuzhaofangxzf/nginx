#ifndef __NGX_GLOBAL_H__
#define __NGX_GLOBAL_H__



typedef struct 
{
    char ItemName[50];
    char ItemContent[500];
}CConfItem, *LPCConfItem;

//外部全局变量的声明

extern char **g_os_argv;
extern char *gp_envmen;
extern int g_environlen;
extern char **environ;

#endif