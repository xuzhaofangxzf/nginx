#include "nginx.hpp"

//本文件用的函数声明
//static void freeresource();
int main(int argc, char *const *argv)
{
    /*（1）一些全局变量的初始化等*/
    int exitcode = 0;
    ngx_pid = getpid();
    ngx_parent = getppid();
        //全局量有必要初始化的
    ngx_log.fd = -1;                  //-1：表示日志文件尚未打开；因为后边ngx_log_stderr要用所以这里先给-1
    ngx_process = NGX_PROCESS_MASTER; //先标记本进程是master进程
    ngx_reap = 0;                     //标记子进程没有发生变化
    for(int i = 0; i < argc; i++)  //argv =  ./nginx -a -b -c asdfas
    {
        g_argvneedmem += strlen(argv[i]) + 1; //+1是给\0留空间。
    }
    //统计环境变量所占的内存。注意判断方法是environ[i]是否为空作为环境变量结束标记
    for(int i = 0; environ[i] != NULL; i++) 
    {
        g_envneedmem += strlen(environ[i]) + 1; //+1是因为末尾有\0,是占实际内存位置的，要算进来
    } //end for
    g_os_argc = argc;               //保存参数个数
    g_os_argv = (char **)argv;     //保存参数指针
    ngxInitSetProcTitle(); //备份环境变量


#if 0
    //在参数列表紧接着的位置，是环境变量的地址，存放在一个叫environ的全局变量中
    for (int i = 0; environ[i]; i++)
    {
        printf("environ[%d]内容=%s\n", i, environ[i]);
        printf("environ[%d]地址=%p\n", i, &environ[i]);

    }
    printf("-------------------------------------------");
#endif
    
/*（2）配置文件的加载*/
    //读取配置文件
    CConfig *p_config = CConfig::getInstance();
    if (p_config->loadConf("../nginx.conf") == false)
    {
        printf("load config file failed!, exit!\n");
        freeresource();
        exit(1);
    }
    //（3）日志初始化
    ngx_log_init();    //日志初始化(创建/打开日志文件)，这个需要配置项，所以必须放配置文件载入的后边；
    //（4）信号初始化
    if (ngx_init_signals() != 0)
    {
        freeresource();
        exit(1);
    }
    #if 0
    //(5)socket初始化,为了测试惊群问题,需要将socket初始化放入子进程中执行,看看是否能解决
    if (!g_socket.Initialize())
    {
        ngx_log_stderr(0, "socket init failed");
        exitcode = 1;
        freeresource();
        return exitcode;
    }
    #endif
    //（5）创建守护进程
    if (p_config->getIntDefault("Daemon", 0) == 1)
    {
        int cdaemonresult = ngx_daemon();
        if (cdaemonresult == -1)
        {
            exitcode = 1;
            freeresource();
            return exitcode;
        }
        if (cdaemonresult == 1)
        {
            //原来的父进程
            freeresource();
            exitcode = 0;
            return exitcode;
        }
        //走到这里，成功创建了守护进程并且这里已经是fork()出来的进程，现在这个进程做了master进程
        g_daemonized = 1;    //守护进程标记，标记是否启用了守护进程模式，0：未启用，1：启用了

    }
    //(6)开始正式的主工作流程，主流程一致在下边这个函数里循环，暂时不会走下来，资源释放啥的日后再慢慢完善和考虑
    ngx_master_process_cycle(); //不管父进程还是子进程，正常工作期间都在这个函数里循环；

#if 0
    //打印配置文件信息
    p_config->printItem();
    int ListenPort = p_config->getIntDefault("ListenPort");
    printf("listen port is %d\n", ListenPort);
    //日志测试
    ngx_log_error_core(0,0,"nginx start working=%s","correct");
    for (;;)
    {
        sleep(3);
        ngx_log_error_core(5,8,"nginx sleeping...");
    }
#endif
    freeresource();
    printf("process exit, bye!\n");

    return 0;
}



void freeresource()
{
    //(1)对于因为设置可执行程序标题导致的环境变量分配的内存，我们应该释放
    if(gp_envmen)
    {
        delete []gp_envmen;
        gp_envmen = NULL;
    }

    //(2)关闭日志文件
    if(ngx_log.fd != STDERR_FILENO && ngx_log.fd != -1)  
    {        
        close(ngx_log.fd); //不用判断结果了
        ngx_log.fd = -1; //标记下，防止被再次close吧
    }
}
