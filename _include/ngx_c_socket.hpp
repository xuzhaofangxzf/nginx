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
typedef struct ngx_connection_s ngx_connection_t, *lpngx_connection_t;
typedef class CSocket CSocket;
/*
    (1)指向类的非静态成员函数的指针需要在指针前面加上类的类型，格式为：
        typedef 返回值 (类名::*指针类型名)(参数列表);
    (2)赋值： 需要用类的成员函数地址赋值，格式为：
        指针类型名  指针名 = &类名::成员函数名;
        注意：这里的这个&符号是比较重要的：不加&，编译器会认为是在这里调用成员函数，
        所以需要给出参数列表，否则会报错；加了&，才认为是要获取函数指针。这是C++专门做了区别对待。
    (3)调用： 针对调用的对象是对象还是指针，分别用.*和->*进行调用，格式为：
        (a): (类对象.*指针名)(参数列表);
        (b): (类指针->*指针名)(参数列表);
        注意：这里的前面一对括号是很重要的，因为()的优先级高于成员操作符指针的优先级。
    (4)指向类的静态函数的指针
        类的静态成员函数和普通函数的函数指针的区别在于，他们是不依赖于具体对象的，所有实例化的对象都共享同一个静态成员，所以静态成员也没有this指针的概念。
        所以，指向类的静态成员的指针就是普通的指针。 即typedef void (*ngx_event_handler_pt)(lpngx_connection_t c);
    总结:
       如果是类的静态成员函数，那么使用函数指针和普通函数指针没区别，使用方法一样,
       如果是类的非静态成员函数，那么使用函数指针需要加一个类限制一下
*/
typedef void (CSocket::*ngx_event_handler_pt)(lpngx_connection_t c);//定义成员函数指针

struct ngx_listening_s
{
    int port;   //监听的端口
    int fd;     //套接字句柄socket
    lpngx_connection_t connection; //连接池中的一个连接，指针类型
};


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

    //char addr_text[100]; //地址的文本信息，100足够，一般其实如果是ipv4地址，255.255.255.255，其实只需要20字节就够

    //和读有关的标志---------------
    //uint8_t                   r_ready;        //读准备好标记【暂时没闹明白官方要怎么用，所以先注释掉】
    uint8_t w_ready; //写准备好标记
    ngx_event_handler_pt rhandler; //读事件的相关处理方法
    ngx_event_handler_pt whandler; //写事件的相关处理方法
    lpngx_connection_t data; //这是个指针【等价于传统链表里的next成员：后继指针】，指向下一个本类型对象，用于把空闲的连接池对象串起来构成一个单向链表，方便取用

};

//socket类
class CSocket
{
public:
    CSocket();
    virtual ~CSocket();

public:
    virtual bool Initialize();   //初始化函数

public:
    int ngx_epoll_init();   //epoll功能初始化
    //epoll增加事件
    int ngx_epoll_add_event(int fd, int readEvent, int writeEvent, uint32_t otherFlag, uint32_t eventType, lpngx_connection_t c);


    int ngx_epoll_process_events(int timer);  //epoll等待接收和处理事件

private:
    bool ngx_open_listening_sockets();  //监听必须的端口(支持多个端口)
    void ngx_close_listening_sockets(); //关闭监听套接字
    bool setnonblocking(int sockfd);    //设置非阻塞套接字
    void readConf();                    //读取各种配置项

    //一些业务处理函数handler
    void ngx_event_accept(lpngx_connection_t oldc); //建立新链接
    void ngx_wait_request_handler(lpngx_connection_t c); //设置数据来时的读处理函数
    void ngx_close_accepted_connection(lpngx_connection_t c); //用户连入，我们accept4()时，得到的socket在处理中产生失败，则资源用这个函数释放【因为这里涉及到好几个要释放的资源，所以写成函数】

    //获取对端信息相关

    size_t ngx_sock_ntop(struct sockaddr *sa, int port, u_char *text, size_t len); ////根据参数1给定的信息，获取地址端口字符串，返回这个字符串的长度

    //连接池或者连接相关
    lpngx_connection_t ngx_get_connection(int isock); //从连接池中获取一个空闲连接
    void ngx_free_connection(lpngx_connection_t c); //归还参数c所代表的连接到到连接池中

private:
    int m_worker_connections; //epoll连接的最大项数
    int m_listenPortCount; //所监听的端口数量
    int m_epollHandle; //epoll_create返回的句柄

    //和连接池有关的
    lpngx_connection_t m_pConnections;  //注意这里可是个指针，其实这是个连接池的首地址
    lpngx_connection_t m_pFreeConnections; //空闲连接链表头，连接池中总是有某些连接被占用，为了快速在池中找到一个空闲的连接，我把空闲的连接专门用该成员记录
    int m_connection_n; //当前进程中所有连接对象的总数【连接池大小】
    int m_free_connection_n; //连接池中可用的连接总数

    std::vector<lpngx_listening_t> m_ListenSocketList;  //监听套接字队列

    struct epoll_event m_events[NGX_MAX_EVENTS]; //用于在epoll_wait()中承载返回的所发生的事件

};


#endif //CPPTEMPLATE_CSOCKET_HPP
