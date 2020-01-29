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

/**********************************************************
 * 函数名称: CSocket::ngx_event_accept
 * 函数描述: 建立新连接专用函数，当新连接进入时，本函数会被ngx_epoll_process_events()所调用
 * 函数参数:
 *      lpngx_connection_t oldc: 从epoll_wait中返回过来的连接
 * 返回值:
 *    void
***********************************************************/
void CSocket::ngx_event_accept(lpngx_connection_t oldc)
{
    //因为listen套接字上用的不是ET【边缘触发】，而是LT【水平触发】，意味着客户端连入如果我要不处理，这个函数会被多次调用，所以，我这里这里可以不必多次accept()，可以只执行一次accept()
    //这也可以避免本函数被卡太久，注意，本函数应该尽快返回，以免阻塞程序运行；
    struct sockaddr servSockAddr; //远端服务器的socket地址
    socklen_t sockLen;
    int err;
    int level;
    int s;
    static int use_accept4 = 1; //使用accept4标记
    lpngx_connection_t newc; //代表连接池中的一个新连接

    sockLen = sizeof(servSockAddr);
    do
    {
        /*
            int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
            accept()系统调用应用于可连接套接口类型 (SOCK_STREAM,SOCK_SEQPACKET)。
            它取出在监听套接口sockfd请求队列里的第一个连接，新建一个已连接的套接口，并且返回一个引用该套接口新的文件描述符。
            参数:
            sockfd: 由socket创建的套接口，通过bind绑定到一个本地地址，并且在调用listen之后正处于监听之中
            addr: 指向一个 sockaddr结构的指针.
            addrlen: addrlen 是一个“值-返回”型参数，调用者必须把它初始化为addr指向的结构的大小(字节数)，返回时，它指出端地址的实际大小
                   如果提供的缓冲区太小，返回的地址将被截断，此时，addrlen 将返回一个比传入更大的值。
            如果队列里没有未处理的连接，并且套接口没有标记为不阻塞，accept()会阻塞当前调用进程直到有一个连接出现。
            如果没有未处理的连接，同时套接口被标记为不阻塞，accept() 返回EAGAIN 或 EWOULDBLOCK 错误。
            
            int accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags);
            flags 是 0，那么 accept4() 与 accept() 功能一样。下面flags 的值可能通过位求或运算来得到不同的行为：
            SOCK_NONBLOCK
                在新打开的文件描述符设置 O_NONBLOCK 标记。在 fcntl(2) 中保存这个标记可以得到相同的效果。
            SOCK_CLOEXEC
                在新打开的文件描述符里设置 close-on-exec (FD_CLOEXEC) 标记。参看在open(2)里关于 O_CLOEXEC标记的描述来了解这为什么有用
            返回值:
                成功时，这个系统调用返回一个非负整数的文件描述符来代表接受的套接口。错误时，返回 -1，并把 errno设置为合适的值
            错误:
            EAGAIN 或 EWOULDBLOCK
                套接口被标记为非阻塞并且没有连接等待接受。POSIX.1-2001允许在此时返回这两种错误，但没有要求两个常量必须具有相同的值，所以可移植的程序应该同时检查两者。
            EBADF
                描述符无效。
            ECONNABORTED
                一个连接已经中止了。
            EFAULT
                参数 addr 不在可写的用户地址空间里。
            EINTR
                在一个有效的连接到达之前，本系统调用被信号中断，参看 signal(7)。
            EINVAL
                套接口不在监听连接，或 addrlen 无效(如是负数)。
            EINVAL
                (accept4()) 在 flags 中有无效的值。
            EMFILE
                达到单个进程打开的文件描述上限。
            ENFILE
                达到系统允许打开文件个数的全局上限。
            ENOBUFS, ENOMEM
                没有足够的自由内存。这通常是指套接口内存分配被限制，而不是指系统内存不足。
            ENOTSOCK
                描述符是一个文件，不是一个套接字。
            EOPNOTSUPP
                引用的套接口不是 SOCK_STREAM 类型的。
            EPROTO
                协议错误。


        */

        if(use_accept4 == 1)
        {
            //从内核获取一个用户端连接，最后一个参数SOCK_NONBLOCK表示返回一个非阻塞的socket，节省一次ioctl【设置为非阻塞】调用
            s = accept4(oldc->fd, &servSockAddr, &sockLen, SOCK_NONBLOCK);
        }
        else
        {
            s = accept(oldc->fd, &servSockAddr, &sockLen);
        }

         //惊群，有时候不一定完全惊动所有4个worker进程，可能只惊动其中2个等等，其中一个成功其余的accept4()都会返回-1；错误 (11: Resource temporarily unavailable【资源暂时不可用】) 
        //所以参考资料：https://blog.csdn.net/russell_tao/article/details/7204260
        //其实，在linux2.6内核上，accept系统调用已经不存在惊群了（至少我在2.6.18内核版本上已经不存在）。大家可以写个简单的程序试下，在父进程中bind,listen，然后fork出子进程，
        //所有的子进程都accept这个监听句柄。这样，当新连接过来时，大家会发现，仅有一个子进程返回新建的连接，其他子进程继续休眠在accept调用上，没有被唤醒。
        ngx_log_stderr(0,"测试惊群问题，看惊动几个worker进程%d\n",s); //【我的结论是：accept4可以认为基本解决惊群问题，但似乎并没有完全解决，有时候还会惊动其他的worker进程】
        
        if (s == -1)
        {
            err = errno;
            //对accept、send和recv而言，事件未发生时errno通常被设置成EAGAIN（意为“再来一次”）或者EWOULDBLOCK（意为“期待阻塞”）
            if (err == EAGAIN || err == EWOULDBLOCK) //accept()没准备好，这个EAGAIN错误EWOULDBLOCK是一样的
            {
                return;
            }
            level = NGX_LOG_ALERT;
            if (err == ECONNABORTED) //ECONNRESET错误则发生在对方意外关闭套接字后【您的主机中的软件放弃了一个已建立的连接--由于超时或者其它失败而中止接连(用户插拔网线就可能有这个错误出现)】
            {
                //该错误被描述为“software caused connection abort”，即“软件引起的连接中止”。原因在于当服务和客户进程在完成用于 TCP 连接的“三次握手”后，
                //客户 TCP 却发送了一个 RST （复位）分节，在服务进程看来，就在该连接已由 TCP 排队，等着服务进程调用 accept 的时候 RST 却到达了。
                //POSIX 规定此时的 errno 值必须 ECONNABORTED。源自 Berkeley 的实现完全在内核中处理中止的连接，服务进程将永远不知道该中止的发生。
                //服务器进程一般可以忽略该错误，直接再次调用accept。
                level = NGX_LOG_ERR;
            }
            else if (err == EMFILE || err == ENFILE)
            {
                //EMFILE:进程的fd已用尽【已达到系统所允许单一进程所能打开的文件/套接字总数】。可参考：https://blog.csdn.net/sdn_prc/article/details/28661661   以及 https://bbs.csdn.net/topics/390592927
                //ulimit -n ,看看文件描述符限制,如果是1024的话，需要改大;  打开的文件句柄数过多 ,把系统的fd软限制和硬限制都抬高.
                //ENFILE这个errno的存在，表明一定存在system-wide的resource limits，而不仅仅有process-specific的resource limits。按照常识，process-specific的resource limits，一定受限于system-wide的resource limits。
                level = NGX_LOG_CRIT;
            }
            ngx_log_error_core(level,errno,"CSocekt::ngx_event_accept: accept error!");

            if (use_accept4 && err == ENOSYS) //调用函数错误,accept4函数不支持
            {
                use_accept4 = 0; //下次采用accept函数
                continue;
            }

            if (err == ECONNABORTED) //对方关闭套接字
            {
                /* do nothing */
            }
            if (err == EMFILE || err == ENFILE)
            {
                //do nothing，这个官方做法是先把读事件从listen socket上移除，然后再弄个定时器，定时器到了则继续执行该函数，但是定时器到了有个标记，会把读事件增加到listen socket上去；
                //我这里目前先不处理吧【因为上边已经写这个日志了】；
            }
            
            return;
        }//end if (s == -1)
        
    //走到这里 accept成功
    ngx_log_stderr(errno, "accept success accept sock = %d", s);
    newc = ngx_get_connection(s); //从连接池中获取空闲连接
    if (newc == NULL)
    {
        //连接池中的连接不可用,关闭socket并返回
        if (close(s) == -1)
        {
            ngx_log_error_core(NGX_LOG_ALERT, errno, "CSocket::ngx_event_accept: close(%d) failed", s);
        }
        return;
        
    }
    //...........将来这里会判断是否连接超过最大允许连接数，现在，这里可以不处理

    //成功地拿到了连接池中的一个连接
    memcpy(&newc->s_sockaddr, &servSockAddr, sockLen); //拷贝客户端地址到连接对象【要转成字符串ip地址参考函数ngx_sock_ntop()】
    //{
       //测试将收到的地址弄成字符串，格式形如"192.168.1.126:40904"或者"192.168.1.126"
       char ipaddr[50]; 
       memset(ipaddr,0,sizeof(ipaddr));
       ngx_sock_ntop(&newc->s_sockaddr,1,ipaddr,sizeof(ipaddr)); //宽度给小点
       ngx_log_stderr(0,"ip信息为%s\n",ipaddr);
    //}

    if (!use_accept4)
    {
        if (setnonblocking(s) == false)
        {
            ngx_log_error_core(NGX_LOG_WARN, errno, "accept setnonblocking failed socket (%d)", s);
            //释放连接
            ngx_close_connection(newc);
            return;
        }
    }
    newc->listening = oldc->listening; //连接对象 和监听对象关联，方便通过连接对象找监听对象【关联到监听端口】
    //newc->w_ready = 1; //标记可以写，新连接写事件肯定是ready的；【从连接池拿出一个连接时这个连接的所有成员都是0】
    //设置数据来时的读处理函数，其实官方nginx中是ngx_http_wait_request_handler()
    newc->rhandler = &CSocket::ngx_read_request_handler; 
    newc->whandler = &CSocket::ngx_write_request_handler;
    //客户端应该主动发送第一次的数据，这里将读事件加入epoll监控
    if (ngx_epoll_add_event(s,              //socket句柄
                            1, 0,           //读写事件
                            0,        //其他补充标记,EPOLLET(高速模式,边沿出发)
                            EPOLL_CTL_ADD,  //事件类型(增加,其他还有删除/修改)
                            newc) == -1)    //连接池中的连接
    {
        ngx_close_connection(newc);
        return;
    }
    break;
    /*
        这种do while的形式比较特别,如果去掉的话,因为前面某些条件达不到,后面的就无法执行到,所以该
        形式的循环能保证后面的执行到才结束.
    */
    } while (1); 

    return;
}

/**********************************************************
 * 函数名称: CSocket::ngx_close_accepted_connection
 * 函数描述: 释放连接,并关闭socket
 * 函数参数:
 *      lpngx_connection_t c: 传入要关闭的连接
 * 返回值:
 *    void
***********************************************************/
void CSocket::ngx_close_connection(lpngx_connection_t c)
{
    ngx_free_connection(c);
    if (close(c->fd) == -1)
    {
        ngx_log_error_core(NGX_LOG_ALERT, errno, "CSockt::ngx_close_accepted_connection: close fd = %d failed", c->fd);
    }
    c->fd = -1;
    ngx_free_connection(c);
    return;
    
}