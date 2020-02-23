//和开启子进程相关

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>   //信号相关头文件 
#include <errno.h>    //errno
#include <unistd.h>

#include "ngx_func.hpp"
#include "ngx_macro.hpp"
#include "ngx_c_conf.hpp"
//#include "ngx_string.hpp"

static u_char  master_process[] = "master process";
/*=====================函数声明=======================*/
static void ngx_start_worker_processes(int threadnums);
static int ngx_spawn_process(int inum, const char *pprocname);
static void ngx_worker_process_cycle(int inum, const char *pprocname);
static void ngx_worker_process_init(int inum);
/*
* 函数名称: ngx_master_process_cycle
* 函数描述: 创建worker子进程
* 函数参数:
*     空
* 返回值:
*    void
*/
void ngx_master_process_cycle()
{
    sigset_t set;  //信号集
    sigemptyset(&set);   //empty signal set
    //下列这些信号在执行本函数期间不希望收到【考虑到官方nginx中有这些信号，老师就都搬过来了】（保护不希望由信号中断的代码临界区）
    //建议fork()子进程时学习这种写法，防止信号的干扰;
    sigaddset(&set, SIGCHLD);       //子进程状态改变
    sigaddset(&set, SIGALRM);        //定时器超时
    sigaddset(&set, SIGIO);         //异步I/O
    sigaddset(&set, SIGINT);        //终端中断符
    sigaddset(&set, SIGHUP);        //终端中断符
    sigaddset(&set, SIGUSR1);       //终端中断符
    sigaddset(&set, SIGUSR2);       //用户定义信号
    sigaddset(&set, SIGWINCH);      //终端窗口大小改变
    sigaddset(&set, SIGTERM);       //终止
    sigaddset(&set, SIGQUIT);       //终端退出符
    //.........可以根据开发的实际需要往其中添加其他要屏蔽的信号......
    //设置，此时无法接受的信号；阻塞期间，你发过来的上述信号，多个会被合并为一个，暂存着，等你放开信号屏蔽后才能收到这些信号。。。
    //sigprocmask()在第三章第五节详细讲解过
    /*
        int sigprocmask(int how, const sigset_t *restrict set, sigset_t *restrict oldset);
        一个进程的信号屏蔽字规定了当前阻塞而不能递送给该进程的信号集。
        sigprocmask()可以用来检测或改变信号屏蔽字，其操作依参数how来决定，如果参数oldset不是NULL指针，那么信号屏蔽字会由此指针返回。
        如果set是一个非空指针，则参数how指示如何修改当前信号屏蔽字。每个进程都有一个用来描述哪些信号递送到进程时将被阻塞的信号集，该信号集中的所有信号在递送到进程后都将被阻塞。
        参数how的取值不同，带来的操作行为也不同，该参数可选值如下：
        1．SIG_BLOCK:　该值代表的功能是将newset所指向的信号集中所包含的信号加到当前的信号掩码中，作为新的信号屏蔽字。
        2．SIG_UNBLOCK:将参数newset所指向的信号集中的信号从当前的信号掩码中移除。
        3．SIG_SETMASK:设置当前信号掩码为参数newset所指向的信号集中所包含的信号。
        注意事项：sigprocmask()函数只为单线程的进程定义的，在多线程中要使用pthread_sigmask变量，在使用之前需要声明和初始化。
        返回值
        执行成功返回0，失败返回-1。
    */
    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1)
    {
        ngx_log_error_core(NGX_LOG_ALERT, errno, "ngx_master_process_cycle: sigprocmassk failed!");
        //不影响后续流程
    }
    //设置主进程标题
    size_t size;
    size = sizeof(master_process); //注意我这里用的是sizeof，所以字符串末尾的\0是被计算进来了的
    size += g_argvneedmem;    //参数长度加进来
    if (size < 1000) //标题的长度小于1000
    {
        char title[1000] = {};
        strcpy(title, (const char*)master_process);
        strcat(title, " ");
        for (int i = 0; i < g_os_argc; i++)
        {
            strcat(title, g_os_argv[i]);
        }
        ngxSetProcTitle(title);
        
    }
    //从配置文件中读取要创建的worker进程数量
    CConfig *p_config = CConfig::getInstance();
    int workerprocessnums = p_config->getIntDefault("WorkerProcesses", 1);
    ngx_start_worker_processes(workerprocessnums);//创建worker子进程
    //创建子进程后，父进程的执行流程会返回到这里，子进程不会走进来
    sigemptyset(&set); //信号屏蔽字为空，表示不屏蔽任何信号
    //sigaddset(&set, SIGINT);
    for (; ;)
    {
        /*
        sigsuspend的原子操作是：
        （1）设置新的mask并阻塞当前进程（此时进程是挂起的，不占用cpu时间，上面set被置空，即不阻塞任何信号)
        （2）假如阻塞的是SIGINT信号，收到了SIGINIT信号，阻塞，程序继续挂起；收到除SIGINT外的其他信号，程序被唤醒，同时恢复原先的mask(此处是上面注册的一堆信号)。
        （3）调用该进程设置的信号处理函数(程序中如果先来SIGINT信号，然后过来SIGUSR2信号，则信号处理函数会调用两次，打印不同的内容。第一次打印SIGUSR2,第二次打印SIGINT，因为SIGINT是前面阻塞的)
        （4）待信号处理函数返回，sigsuspend返回了。(sigsuspend将捕捉信号和信号处理函数集成到一起了)
        注：sigsuspend实际是将sigprocmask和pause结合起来原子操作。
        */
        sigsuspend(&set); //阻塞在这里，等待一个信号，此时进程是挂起的，不占用cpu时间，只有收到信号才会被唤醒（返回）；
        //此时master进程完全靠信号驱动干活
        ngx_log_error_core(NGX_LOG_NOTICE, 0, "This is father process, pid = %d", ngx_pid);
        sleep(10);
    }
    
    return;

}

/*
* 函数名称: ngx_start_worker_processes
* 函数描述: 根据给定的参数创建指定数量的子进程，因为以后可能要扩展功能，增加参数，所以单独写成一个函数
* 函数参数: 
*     threadnums:要创建的子进程数量
* 返回值:
*    void
*/

static void ngx_start_worker_processes(int threadnums)
{
    for (int i = 0; i < threadnums; i++)
    {
        ngx_spawn_process(i, "nginx: worker process");  //master进程在走这个循环，来创建若干个子进程
    }
    return;
    
}

/*
* 函数名称: ngx_spawn_process
* 函数描述: 产生一个子进程
* 函数参数: 
*     inum：进程编号【0开始】
*     pprocname：子进程名字"worker process"
* 返回值:
*    int 进程ID
*/
static int ngx_spawn_process(int inum, const char *pprocname)
{
    pid_t pid;
    pid = fork();
    switch (pid)
    {
    case -1:
        ngx_log_error_core(NGX_LOG_ALERT, errno, "ngx_spawn_process: fork child process num = %d, procname = \"%s\"failed!", inum, pprocname);
        return -1;
        
    case 0: // child process
        ngx_parent = ngx_pid;     //因为是子进程了，所有原来的pid变成了父pid
        ngx_pid = getpid();       //重新获取pid，即本子进程的pid
        ngx_worker_process_cycle(inum, pprocname);
    
    default: //这个应该是父进程分支，直接break;，流程往switch之后走
        break;
    }
    return pid; //返回的是子进程的id

}

/*
* 函数名称: ngx_worker_process_cycle
* 函数描述: worker子进程的功能函数，每个woker子进程，就在这里循环着了
        (无限循环【处理网络事件和定时器事件以对外提供web服务】）
* 函数参数: 
*     inum：进程编号【0开始】
*     pprocname：子进程名字"worker process"
* 返回值:
*    void
*/
static void ngx_worker_process_cycle(int inum, const char *pprocname)
{
    //设置一下变量
    ngx_process = NGX_PROCESS_WORKER;  //设置进程的类型，是worker进程
    //重新为子进程设置进程名，不要与父进程重复------
    ngx_worker_process_init(inum);
    ngx_log_stderr(0, "running here 1");
    ngxSetProcTitle(pprocname);
    for (; ;)
    {
        //sleep(10);
        //ngx_log_error_core(0, 0, "This is child process num = %d, pid = %d ", inum, ngx_pid);
        ngx_process_events_and_timers(); //处理网络事件和定时器事件
    }
   g_threadpool.stopAll();
   g_socket.shutDown_subproc(); 
}
/*
* 函数名称: ngx_worker_process_init
* 函数描述: 子进程创建时调用本函数进行一些初始化工作
* 函数参数: 
*     inum：进程编号【0开始】
* 返回值:
*    void
*/
static void ngx_worker_process_init(int inum)
{
    sigset_t set;
    sigemptyset(&set);//原来是屏蔽那10个信号【防止fork()期间收到信号导致混乱】，现在不再屏蔽任何信号【接收任何信号】
    if (sigprocmask(SIG_SETMASK, &set, NULL) == -1)  
    {
        ngx_log_error_core(NGX_LOG_ALERT, errno, "ngx_worker_process_init: sigprocmask failed! proc num = %d", inum);
        //....将来再扩充代码
        //....
    }
    CConfig *p_config = CConfig::getInstance();
    int tempThreadNums = p_config->getIntDefault("ProcMsgRecvWorkThreadCount", 5);
    if (g_threadpool.createThread(tempThreadNums) == false)
    {
        ngx_log_stderr(0,"create threads pool failed!");
        exit(-2);
    }
    if (!g_socket.initialize_subproc())
    {
        ngx_log_stderr(0, "init subproc failed!");
        exit(-2);
    }
    
    sleep(1);
    //(5)socket初始化,为了测试惊群问题,需要将socket初始化放入子进程中执行,看看是否能解决
    if (!g_socket.Initialize())
    {
        ngx_log_stderr(0, "socket init failed");
        freeresource();
        exit(1);
    }
    //初始化epoll
    g_socket.ngx_epoll_init();
    return;
    
}