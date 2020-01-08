#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h> //waitpid

#include "ngx_macro.hpp"
#include "ngx_func.hpp"
#include "ngx_global.hpp"

typedef struct 
{
    int           signo;      //信号对应的数字编号 ，每个信号都有对应的#define
    const char    *signame;   //信号对应的名字，如SIGHUP
    void (*handler)(int signo, siginfo_t *siginfo, void *ucontext);
}ngx_signal_t;

static void ngx_signal_handler(int signo, siginfo_t *siginfo, void *ucontext);
static void ngx_process_get_status();
//数组 ，定义本系统处理的各种信号，我们取一小部分nginx中的信号，并没有全部搬移到这里，日后若有需要根据具体情况再增加
//在实际商业代码中，你能想到的要处理的信号，都弄进来
ngx_signal_t signals[] = {
    //signo         signame                 handler
    {SIGHUP,        "SIGHUP",                ngx_signal_handler},       //终端断开信号，对于守护进程常用于reload重载配置文件通知--标识1
    {SIGINT,        "SIGINT",                ngx_signal_handler},       //标识2   
    {SIGTERM,       "SIGTERM",               ngx_signal_handler},       //标识15
    {SIGCHLD,       "SIGCHLD",               ngx_signal_handler},       //子进程退出时，父进程会收到这个信号--标识17
    {SIGQUIT,       "SIGQUIT",               ngx_signal_handler},       //标识3
    {SIGIO,         "SIGIO",                 ngx_signal_handler},       //指示一个异步I/O事件【通用异步I/O信号】
    {SIGUSR2,       "SIGUSR2",               ngx_signal_handler},
    {SIGSYS,        "SIGSYS, SYS_IGN",       NULL},                     //我们想忽略这个信号，SIGSYS表示收到了一个无效系统调用，如果我们不忽略，进程会被操作系统杀死，--标识31
    //...
    {0,              NULL,                   NULL}                      //信号对应的数字至少是1，所以可以用0作为一个特殊标记
};

//初始化信号的函数，用于注册信号处理程序
//返回值：0成功  ，-1失败
int ngx_init_signals()
{
    ngx_signal_t     *sig;
    struct sigaction sa;
    for (sig = signals; sig->signo != 0; sig++)
    {
        memset(&sa, 0, sizeof(struct sigaction));

        if (sig->handler)
        {
            sa.sa_sigaction = sig->handler;    //sa_sigaction：指定信号处理程序(函数)，注意sa_sigaction也是函数指针，是这个系统定义的结构sigaction中的一个成员（函数指针成员）；
            sa.sa_flags = SA_SIGINFO;          //sa_flags：int型，指定信号的一些选项，设置了该标记(SA_SIGINFO)，就表示信号附带的参数可以被传递到信号处理函数中
            //说白了就是你要想让sa.sa_sigaction指定的信号处理程序(函数)生效，你就把sa_flags设定为SA_SIGINFO
        }
        else
        {
            //sa_handler:这个标记SIG_IGN给到sa_handler成员，表示忽略信号的处理程序，否则操作系统的缺省信号处理程序很可能把这个进程杀掉；
            //其实sa_handler和sa_sigaction都是一个函数指针用来表示信号处理程序。只不过这两个函数指针他们参数不一样， sa_sigaction带的参数多，信息量大，
            //而sa_handler带的参数少，信息量少；如果你想用sa_sigaction，那么你就需要把sa_flags设置为SA_SIGINFO；
            sa.sa_handler = SIG_IGN;
        }
        //比如咱们处理某个信号比如SIGUSR1信号时不希望收到SIGUSR2信号，那咱们就可以用诸如sigaddset(&sa.sa_mask,SIGUSR2);这样的语句针对信号为SIGUSR1时做处理，这个sigaddset三章五节讲过；
        //这里.sa_mask是个信号集（描述信号的集合），用于表示要阻塞的信号，sigemptyset()这个函数咱们在第三章第五节讲过：把信号集中的所有信号清0，本意就是不准备阻塞任何信号；
        sigemptyset(&sa.sa_mask);
        //设置信号处理动作(信号处理函数)，说白了这里就是让这个信号来了后调用我的处理程序，有个老的同类函数叫signal，不过signal这个函数被认为是不可靠信号语义，不建议使用，大家统一用sigaction
        //int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
        //signum参数指出要捕获的信号类型，act参数指定新的信号处理方式，oldact参数输出先前信号的处理方式（如果不为NULL的话）
        if (sigaction(sig->signo, &sa, NULL) == -1)
        {
            ngx_log_error_core(NGX_LOG_EMERG, errno, "sigaction(%s) failed", sig->signame);
            return -1;
        }
        else
        {
            ngx_log_stderr(0, "sigaction(%s) success", sig->signame);
        }

    }
    return 0;
    
}

static void ngx_signal_handler(int signo, siginfo_t *siginfo, void *ucontext)
{


    ngx_signal_t    *sig;
    char            *action; //
    for ( sig = signals; sig->signo != 0; sig++)
    {
        //找到对应的信号，进行处理
        if (sig->signo == signo)
        {
            break;
        }
        
    }

    action = (char *)"";
    if(ngx_process == NGX_PROCESS_MASTER) //master进程，管理进程，处理信号比较多
    {
        //master进程的处理
        switch (signo)
        {
        case SIGCHLD: //一般子进程退出，就会收到该信号
            ngx_reap = 1; //标记子进程状态变化，日后master主进程的for(;;)循环中可能会用到这个变量[如重新创建子进程]
            break;
        //.....其他信号处理以后待增加
        default:
            break;
        }
    }
    else if (ngx_process == NGX_PROCESS_WORKER)
    {
        //worker子进程的处理
    }
    else
    {
        //非maste与worker进程，暂时不做处理
    }
    if (siginfo && siginfo->si_pid)
    {
        ngx_log_error_core(NGX_LOG_NOTICE, 0, "signal %d (%s) received from %d%s", signo, sig->signame, siginfo->si_pid, action);
    }
    else
    {

        //没有发送该信号的进程id，所以不显示发送该信号的进程id
        ngx_log_error_core(NGX_LOG_NOTICE, 0, "signal %d (%s) received %s", signo, sig->signame, action);
    }
    //ps -eo pid,ppid,pgid,tty,stat,uid,time,tpgid,comm | grep nginx
    //子进程状态有变化，通常是意外退出【既然官方是在这里处理，我们也学习官方在这里处理】
    if (signo == SIGCHLD)
    {
        ngx_process_get_status();
    }

    return;
    
}
    
/*
* 函数名称: ngx_process_get_status
* 函数描述: 获取子进程的结束状态，防止单独kill子进程时子进程变成僵尸进程
* 函数参数: 
*    none
* 返回值:
*    void
*/
void ngx_process_get_status()
{
    pid_t pid;
    int status;
    int err;
    int one = 0; ////抄自官方nginx，应该是标记信号正常处理过一次
    //当杀死一个子进程的时候，父进程应该会收到这个SIGCHLD信号
    for (; ;)
    {
        /*
     waitpid函数: 作用同于wait，但可指定pid进程清理，可以不阻塞。
    pid_t waitpid(pid_t pid,int *status,int options);
    成功：返回清理掉的子进程ID；失败：-1（无子进程）
    特殊参数和返回情况：
    参数pid：
       >0 等待任何子进程 ID 为 pid 的子进程，只要指定的子进程还没有结束，waitpid() 就会一直等下去。
       -1 回收任意子进程（相当于wait）
       0 等待进程组 ID 与当前进程相同的任何子进程（也就是等待同一个进程组中的任何子进程）
       < -1 等待进程组ID为pid绝对值的进程组中的任何子进程；
    返回0：参数3为WNOHANG，且子进程正在运行。
    注意：一次wait或waitpid调用只能清理一个子进程，清理多个子进程需要用到循环
        */
        pid = waitpid(-1, &status, WNOHANG);
        if (pid == 0)    //子进程没结束，会立即返回这个数字，但这里应该不是这个数字【因为一般是子进程退出时会执行到这个函数】
        {
            return;
        }
        //error
        if (pid == -1)
        {
            err = errno;
            if (err == EINTR) //调用被某个信号中断
            {
                continue;
            }
            if(err == ECHILD && one)  //没有子进程
            {
                return;
            }
            if (err == ECHILD)
            {
                ngx_log_error_core(NGX_LOG_INFO, err, "waitpid() failed!");
                return;
            }
            ngx_log_error_core(NGX_LOG_ALERT, err, "waitpid() failed!");
            return;
        }//end if
        //走到这里，表示  成功【返回进程id】 ，这里根据官方写法，打印一些日志来记录子进程的退出
        one = 1; //标记waitpid()返回了正常的返回值
        if (WTERMSIG(status))   //获取使子进程终止的信号编号
        {
            ngx_log_error_core(NGX_LOG_ALERT,0,"pid = %P exited on signal %d!",pid,WTERMSIG(status)); //获取使子进程终止的信号编号
        }
        else
        {
            ngx_log_error_core(NGX_LOG_NOTICE,0,"pid = %P exited with code %d!",pid,WEXITSTATUS(status)); //WEXITSTATUS()获取子进程传递给exit或者_exit参数的低八位
        }
    }
    return;
}