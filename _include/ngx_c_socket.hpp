//
// Created by Xufang on 2020/1/2.
//

#ifndef CPPTEMPLATE_CSOCKET_HPP
#define CPPTEMPLATE_CSOCKET_HPP


#include <vector>
#include <sys/epoll.h> //epoll
#include <sys/socket.h>

#define NGX_LISTEN_BACKLOG 511  //已完成连接队列，nginx给511，我们也先按照这个来：不懂这个数字的同学参考第五章第四节，
#define NGX_MAX_EVENTS     512  //epoll_wait一次最多接收这么多个事件，nginx中缺省是512，我们这里固定给成512就行，没太大必要修改
typedef struct ngx_listening_s ngx_listening_t, *lpngx_listening_t;
typedef class CSocket CSocket;
typedef void (CSocket::*ngx_event_handler_pt)(lpngx_connection_t c);

typedef struct ngx_listening_s
{
    int port;   //监听的端口
    int fd;     //套接字句柄socket
} ngx_listening_t, *lpngx_listening_t;


//以下三个结构是非常重要的三个结构，我们遵从官方nginx的写法；
//(1)该结构表示一个TCP连接【客户端主动发起的、Nginx服务器被动接受的TCP连接】
struct ngx_connection_s
{
    int fd; //套接字句柄socket
    lpngx_listening_t listening; //如果这个链接被分配给了一个监听套接字，那么这个里边就指向监听套接字对应的那个lpngx_listening_t的内存首地址
    //------------------------------------
    unsigned instance:1; //【位域】失效标志位：0：有效，1：失效【这个是官方nginx提供，到底有什么用，ngx_epoll_process_events()中详解
    uint64_t iCurrsequence;  //我引入的一个序号，每次分配出去时+1，此法也有可能在一定程度上检测错包废包，具体怎么用，用到了再说
    struct sockaddr s_sockaddr;  //保存对方地址信息



    //和读有关的标志---------------
    //uint8_t                   r_ready;        //读准备好标记【暂时没闹明白官方要怎么用，所以先注释掉】
    uint8_t w_ready; //写准备好标记
    ngx_event_handler_pt

};
class CSocket
{
public:
    CSocket();
    virtual ~CSocket();

public:
    virtual bool Initialize();   //初始化函数

public:
    int ngx_epoll_init();   //epoll功能初始化
    int ngx_epoll_add_event(int fd, int readEvent, int writeEvent, uint32_t otherFlag, uint32_t eventType, lpngx_conne)

    int ngx_epoll_process_events(int timer);  //epoll等待接收和处理事件

private:
    bool ngx_open_listening_sockets();  //监听必须的端口(支持多个端口)
    void ngx_close_listening_sockets(); //关闭监听套接字
    bool setnonblocking(int sockfd);    //设置非阻塞套接字
    void readConf();                    //读取各种配置项

    //一些业务处理函数handler
    void ng

private:
    int m_listenPortCount;  //所监听的端口数量
    std::vector<lpngx_listening_t> m_ListenSocketList;  //监听套接字队列

};


#endif //CPPTEMPLATE_CSOCKET_HPP
