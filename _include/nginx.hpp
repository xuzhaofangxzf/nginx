#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "ngx_c_conf.hpp"
#include "ngx_func.hpp"
#include "ngx_signal.hpp"
#include "ngx_global.hpp"
/*globle variate for settings*/

//extern char **environ;

char **g_os_argv;   //原始命令行参数数组，在main中会被赋值
char *gp_envmen = NULL; //自己重新分配env环境变量的内存
int g_environlen = 0;   //the memery length of the environ
//和进程本身有关的全局量
pid_t ngx_pid;               //当前进程的pid
