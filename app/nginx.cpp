#include "nginx.hpp"

//本文件用的函数声明
static void freeresource();
int main(int argc, char *const *argv)
{

#if 0
    //在参数列表紧接着的位置，是环境变量的地址，存放在一个叫environ的全局变量中
    for (int i = 0; environ[i]; i++)
    {
        printf("environ[%d]内容=%s\n", i, environ[i]);
        printf("environ[%d]地址=%p\n", i, &environ[i]);

    }
    printf("-------------------------------------------");
#endif
    g_os_argv = (char **)argv;
    ngxInitSetProcTitle(); //备份环境变量
    //修改程序运行时候后台的名字
    ngxSetProcTitle("nginx: master process");
    //读取配置文件
    CConfig *p_config = CConfig::getInstance();
    if (p_config->loadConf("../nginx.conf") == false)
    {
        printf("load config file failed!, exit!");
        exit(1);
    }
    p_config->printItem();
    int ListenPort = p_config->getIntDefault("ListenPort");
    printf("listen port is %d\n", ListenPort);
    ngx_log_init();
    ngx_log_error_core(5,0,"nginx start working=%s","correct");
    for (;;)
    {
        sleep(3);
        ngx_log_error_core(5,8,"nginx sleeping...");
    }

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
