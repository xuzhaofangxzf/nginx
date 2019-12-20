#include "nginx.hpp"
int main(int argc, char * const *argv)
{

    #if 0
    for (int i = 0; environ[i]; i++)
    {
        printf("environ[%d]内容=%s\n", i, environ[i]);
        printf("environ[%d]地址=%p\n", i, &environ[i]);

    }
    printf("-------------------------------------------");
    #endif
    g_os_argv = (char**)argv;
    return 0;
}

