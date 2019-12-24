#include "nginx.hpp"
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
    
    for (;;)
    {
        sleep(3);
        printf("Sleep 3 seconds!\n");
    }
    if (gp_envmen != NULL)
    {
        delete[] gp_envmen;
        gp_envmen = NULL;
    }

    printf("process exit, bye!\n");

    return 0;
}
