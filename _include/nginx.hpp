#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "ngx_c_conf.hpp"
#include "ngx_func.hpp"
#include "ngx_signal.hpp"
#include "ngx_global.hpp"
#include "ngx_c_socket.hpp"
#include "ngx_c_memory.hpp"
/*globle variate for settings*/

//extern char **environ;
size_t g_argvneedmem = 0;    //argv参数所需要的内存大小
size_t  g_envneedmem=0;         //环境变量所占内存大小
int g_os_argc;              //参数个数
char **g_os_argv;   //原始命令行参数数组，在main中会被赋值
char *gp_envmen = NULL; //自己重新分配env环境变量的内存
int g_environlen = 0;   //the memery length of the environ
//和进程本身有关的全局量
pid_t ngx_pid;               //当前进程的pid
pid_t ngx_parent;            //父进程的pid
int ngx_process;            //进程类型，比如master，worker等
int g_daemonized = 0;         //守护进程标记，标记是否启用了守护进程模式，0：未启用，1：启用了
/*sig_atomic_t:系统定义的类型：访问或改变这些变量需要在计算机的一条指令内完成
一般等价于int【通常情况下，int类型的变量通常是原子访问的，也可以认为 sig_atomic_t就是int类型的数据】*/
sig_atomic_t ngx_reap;      //标记子进程状态变化[一般是子进程发来SIGCHLD信号表示退出]
CSocket g_socket;
ngx_c_threadpool  g_threadpool;      //线程池全局对象
int g_stopEvent; //标志程序退出, 0:不退出 1:退出

