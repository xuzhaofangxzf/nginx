//
// Created by Xufang on 2020/1/2.
//
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>      //ioctl
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>  //errno
#include <fcntl.h>

#include "ngx_c_socket.hpp"
#include "ngx_c_conf.hpp"
#include "ngx_global.hpp"
#include "ngx_func.hpp"
#include "ngx_macro.hpp"

CSocket::CSocket()
{
    m_listenPortCount = 1;
}

CSocket::~CSocket()
{
    std::vector<lpngx_listening_t>::iterator pos;
    for (pos = m_ListenSocketList.begin(); pos != m_ListenSocketList.end(); pos++)
    {
        delete (*pos);
    }
    m_ListenSocketList.clear();
    return;
    
}

/*
 * 函数名称: CSocket::ngx_open_listening_sockets
 * 函数描述: socket初始化函数
 * 函数参数:
 *      空
 * 返回值:
 *    失败： false
 *    成功:  true
*/
bool CSocket::ngx_open_listening_sockets()
{
    CConfig *pConfig = CConfig::getInstance();
    m_listenPortCount = pConfig->getIntDefault("ListenPortCount", m_listenPortCount);
    
    int *aSock = new int[m_listenPortCount];
    struct sockaddr_in serv_addr;       //服务器的地址结构体
    int iport;  //端口
    char strinfo[100];  //临时字符串
    //初始化相关
    memset(strinfo, 0, strlen(strinfo));
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; //IPv4
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int i = 0;
    for (i = 0; i < m_listenPortCount; i++)
    {
        /*
         * int socket(int af, int type, int protocol);
         * af 为地址族（Address Family），也就是 IP 地址类型，常用的有 AF_INET 和 AF_INET6。
         * type 为数据传输方式，常用的有 SOCK_STREAM 和 SOCK_DGRAM
         * protocol 表示传输协议，常用的有 IPPROTO_TCP 和 IPPTOTO_UDP，分别表示 TCP 传输协议和 UDP 传输协议
        */
        aSock[i] = socket(AF_INET, SOCK_STREAM, 0);
        if (aSock[i] == -1)
        {
            ngx_log_stderr(errno, "CSocket::Initialize(), socket create failed, listen number = %d", i);
            break;  //此时跳出是为了后续资源的清除
        }
        /*
         * int setsockopt(int sockfd, int level, int optname,const void *optval, socklen_t optlen);
         * sockfd：标识一个套接口的描述字。
         * level：选项定义的层次；支持SOL_SOCKET、IPPROTO_TCP、IPPROTO_IP和IPPROTO_IPV6。
         * optname：需设置的选项。
         * optval：指针，指向存放选项待设置的新值的缓冲区。
         * optlen：optval缓冲区长度。
        */
        int reuseaddr = 1;   //1:打开对应的设置选项
        if (setsockopt(aSock[i], SOL_SOCKET, SO_REUSEADDR, (const void *)&reuseaddr, sizeof(reuseaddr)) == -1)
        {
            ngx_log_stderr(errno, "CSocket::ngx_open_listening_sockets() setsockopt failed! listen number = %d", i);
            
            break;
        }
        //设置该socket为非阻塞
        if(!setnonblocking(aSock[i]))
        {
             ngx_log_stderr(errno, "CSocket::ngx_open_listening_sockets() setnobocking failed! listen number = %d", i);
             break;
        }
        //设置本服务器需要监听的地址和端口
        sprintf(strinfo, "ListenPort%d", i);
        iport = pConfig->getIntDefault(strinfo, 10000);
        serv_addr.sin_port = htons((in_port_t)iport);

        //绑定服务器地址结构体
        /*
         * int bind(int sock, struct sockaddr *addr, socklen_t addrlen);  
         * sock 为 socket 文件描述符，
         * addr 为 sockaddr 结构体变量的指针，
         * addrlen 为 addr 变量的大小，可由 sizeof() 计算得出
        */
        if (bind(aSock[i], (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        {
             ngx_log_stderr(errno, "CSocket::ngx_open_listening_sockets() bind failed! listen number = %d", i);
             break;
        }
        //开始监听
        /*
         * int listen(int sock, int backlog); 
         * sock 为需要进入监听状态的套接字，backlog 为请求队列的最大长度。
         * 请求队列
         * 当套接字正在处理客户端请求时，如果有新的请求进来，套接字是没法处理的，只能把它放进缓冲区，待当前请求处理完毕后，再从缓冲区中读取出来处理。如果不断有新的请求进来，它们就按照先后顺序在缓冲区中排队，直到缓冲区满。这个缓冲区，就称为请求队列（Request Queue）。
            缓冲区的长度（能存放多少个客户端请求）可以通过 listen() 函数的 backlog 参数指定，但究竟为多少并没有什么标准，可以根据你的需求来定，并发量小的话可以是10或者20。
            如果将 backlog 的值设置为 SOMAXCONN，就由系统来决定请求队列长度，这个值一般比较大，可能是几百，或者更多。
            当请求队列满时，就不再接收新的请求，对于 Linux，客户端会收到 ECONNREFUSED 错误
        */
        if (listen(aSock[i], NGX_LISTEN_BACKLOG) == -1)
        {
            ngx_log_stderr(errno, "CSocket::ngx_open_listening_sockets() listen failed! listen number = %d", i);
            break;
        }
        lpngx_listening_t p_listenSocketItem = new ngx_listening_t;
        memset(p_listenSocketItem, 0, sizeof(ngx_listening_t));
        p_listenSocketItem->port = iport;
        p_listenSocketItem->fd = aSock[i];
        ngx_log_error_core(NGX_LOG_INFO, 0, "listen %d port success!", iport);
        m_ListenSocketList.push_back(p_listenSocketItem);
     
    }

    if (i < m_listenPortCount) //没有初始化完毕，认为某些出错，直接清除资源，返回错误
    {
        //关闭已经建立的socket
        while (i >= 0)
        {
            close(aSock[i]);
            i--;
        }
        delete[] aSock;
        return false;
    
    }
    delete[] aSock;
    return true;
}
/*
 * 函数名称: CSocket::setnonblocking
 * 函数描述: 设置阻塞/非阻塞
 * 函数参数:
 *      int sockfd: socket文件描述符
 * 返回值:
 *    失败： false
 *    成功:  true
*/
bool CSocket::setnonblocking(int sockfd) 
{
    int nb = 1; //0: clear, 1: set
    /*
     * int ioctl(int fd, int cmd, ...)
     * fd: 文件描述符
     * cmd: 交互协议，设备驱动将根据 cmd 执行对应操作
     * ioctl() 函数执行成功时返回 0，失败则返回 -1 并设置全局变量 errorno 值
    */
    if (ioctl(sockfd, FIONBIO, &nb) == -1)  //FIONBIO：设置/清除非阻塞I/O标记：0：清除，1：设置
    {
        return false;
    }
    return true;
    /*
        另一种写法:
        int fcntl(int fd, int cmd); 
        int fcntl(int fd, int cmd, long arg); 
        int fcntl(int fd, int cmd, struct flock *lock);
        fcntl()针对(文件)描述符提供控制。参数fd是被参数cmd操作(如下面的描述)的描述符。针对cmd的值，fcntl能够接受第三个参数int arg。
        fcntl()的返回值与命令有关。如果出错，所有命令都返回－1，如果成功则返回某个其他值。
        下列三个命令有特定返回值：F_DUPFD , F_GETFD , F_GETFL以及F_GETOWN。
        F_DUPFD   返回新的文件描述符
        F_GETFD   返回相应标志
        F_GETFL , F_GETOWN   返回一个正的进程ID或负的进程组ID
        fcntl函数有5种功能:
        1. 复制一个现有的描述符(cmd=F_DUPFD). 
        2. 获得／设置文件描述符标记(cmd=F_GETFD或F_SETFD). 
        3. 获得／设置文件状态标记(cmd=F_GETFL或F_SETFL). 
        4. 获得／设置异步I/O所有权(cmd=F_GETOWN或F_SETOWN). 
        5. 获得／设置记录锁(cmd=F_GETLK , F_SETLK或F_SETLKW).
    */
   int opts = fcntl(sockfd, F_GETFL);
   if (opts < 0)
   {
       ngx_log_stderr(errno, "CSocket::setnonblocking: fcntl(F_GETFL) failed!");
       return false;
   }
   //把非阻塞标记加到原来的标记上，标记这是个非阻塞套接字【如何关闭非阻塞呢？opts &= ~O_NONBLOCK,然后再F_SETFL一下即可】
   opts |= O_NONBLOCK;
   if (fcntl(sockfd, F_SETFL, opts) < 0)
   {
       ngx_log_stderr(errno, "CSocket::setnoblocking: fcntl(F_SETFL) failed");
       return false;
   }
   return true;

}

bool CSocket::Initialize() 
{
    bool reco = ngx_open_listening_sockets();
    return reco;
}

void CSocket::ngx_close_listening_sockets() 
{
    for (int i = 0; i < m_listenPortCount; i++)
    {
        close(m_ListenSocketList[i]->fd);
        ngx_log_error_core(NGX_LOG_INFO, 0, "Close listening port %d", m_ListenSocketList[i]->port);
    }
    
}

void CSocket::readConf()
{

}
