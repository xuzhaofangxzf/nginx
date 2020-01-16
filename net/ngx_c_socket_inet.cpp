
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>    //uintptr_t
#include <stdarg.h>    //va_start....
#include <unistd.h>    //STDERR_FILENO等
#include <sys/time.h>  //gettimeofday
#include <time.h>      //localtime_r
#include <fcntl.h>     //open
#include <errno.h>     //errno
//#include <sys/socket.h>
#include <sys/ioctl.h> //ioctl
#include <arpa/inet.h>

#include "ngx_c_conf.hpp"
#include "ngx_macro.hpp"
#include "ngx_global.hpp"
#include "ngx_func.hpp"
#include "ngx_c_socket.hpp"
#include "ngx_c_socket.hpp"

/**********************************************************
 * 函数名称: CSocket::ngx_sock_ntop
 * 函数描述: 将socket绑定的地址转换为文本格式【根据参数1给定的信息，获取地址端口字符串，返回这个字符串的长度】
 * 函数参数:
 *      struct sockaddr *sar:客户端的IP地址信息
 *      int port: 1:表示把端口信息也放到组合成的字符串里,0:则不包含端口信息
 *      u_char *text: 保存文本信息
 *      size_t len: 记录文本的宽度
 * 返回值:
 *    int: 1撑场返回，0：有问题的返回，不影响流程的继续进行
***********************************************************/
size_t CSocket::ngx_sock_ntop(struct sockaddr *sa, int port, char *text, size_t len)
{
    struct sockaddr_in *sin;
    int slen;
    u_char *p;

    switch (sa->sa_family)
    {
    case AF_INET:
        sin = (struct sockaddr_in *)sa;
        p = (u_char*)&sin->sin_addr;
        if (port)
        {
            slen = snprintf(text, len, "%d.%d.%d.%d.:%d", p[0], p[1], p[2], p[3], ntohs(sin->sin_port));
        }
        else
        {
            slen = snprintf(text, len, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
        }
        return slen;  
    
    default:
        break;
    }
    return 0;

}