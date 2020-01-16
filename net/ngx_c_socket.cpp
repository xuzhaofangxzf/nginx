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
#include <iostream>

#include "ngx_c_socket.hpp"
#include "ngx_c_conf.hpp"
#include "ngx_global.hpp"
#include "ngx_func.hpp"
#include "ngx_macro.hpp"


CSocket::CSocket()
{
    //配置相关
    m_worker_connections = 1; //epoll连接最大项数
    m_listenPortCount = 1; //监听一个端口

    //epoll相关
    m_epollHandle = -1; //epoll返回的句柄
    m_pConnections = NULL; //连接池【连接数组】先给空
    m_pFreeConnections = NULL; //连接池中的空闲连接
    m_iLenPkg_Header = sizeof(COMM_PKG_HEADER);
    m_iLenMsgHeader = sizeof(STRUC_MSG_HEADER);
    

    return;
}

CSocket::~CSocket()
{
    std::vector<lpngx_listening_t>::iterator pos;
    for (pos = m_ListenSocketList.begin(); pos != m_ListenSocketList.end(); pos++)
    {
        delete (*pos);
    }
    m_ListenSocketList.clear();

    if (m_pConnections != NULL)
    {
        delete [] m_pConnections;
    }
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
/**********************************************************
 * 函数名称: CSocket::setnonblocking
 * 函数描述: 设置阻塞/非阻塞
 * 函数参数:
 *      int sockfd: socket文件描述符
 * 返回值:
 *    失败： false
 *    成功:  true
***********************************************************/
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
    #if 0
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
   #endif

}

bool CSocket::Initialize() 
{
    readConf();
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
/**********************************************************
 * 函数名称: CSocket::readConf
 * 函数描述: 读取配置文件相关信息，将值付给相关变量
 * 函数参数:
 *      空
 * 返回值:
 *    void
***********************************************************/
void CSocket::readConf()
{
    CConfig *pConfig = CConfig::getInstance();
    m_worker_connections = pConfig->getIntDefault("worker_connections", m_worker_connections);
    m_listenPortCount = pConfig->getIntDefault("ListenPortCount",m_listenPortCount);

    return;
}

/**********************************************************
 * 函数名称: CSocket::ngx_epoll_init
 * 函数描述: epoll功能初始化，子进程中进程，本函数被ngx_worker_process_init()调用
 * 函数参数:
 *      空
 * 返回值:
 *    int：
***********************************************************/
int CSocket::ngx_epoll_init()
{
    /*
    int epoll_create(int size)
    该 函数生成一个epoll专用的文件描述符。
    它其实是在内核申请一空间，用来存放你想关注的socket fd上是否发生以及发生了什么事件。
    size就是你在这个epoll fd上能关注的最大socket fd数。
    */
    //(1)很多内核版本不处理epoll_create的参数，只要该参数>0即可
    //创建一个epoll对象，创建了一个红黑树，还创建了一个双向链表
    m_epollHandle = epoll_create(m_worker_connections); //直接以epoll连接的最大项数为参数，肯定是>0的
    if (m_epollHandle == -1)
    {
        ngx_log_stderr(errno, "CSocket::ngx_epoll_init(), epoll_create() failed!");
        exit(2); //直接退出，由系统帮我们回收资源
    }

    //(2)创建连接池，用于存储客户端的连接信息
    m_connection_n = m_worker_connections; //记录当前连接池的总连接数
    try
    {
        m_pConnections = new ngx_connection_t[m_connection_n];
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    int i = m_connection_n; //连接池中连接数
    lpngx_connection_t next = NULL;
    lpngx_connection_t connectHead = m_pConnections; //连接池数组首地址
    do
    {
        //从数组的末尾开始，将连接池串联起来，表示空的连接池
        i--;
        connectHead[i].data = next; //此处的data相当于链表中的next指针，数组的最后一个元素的data（即next）指向NULL
        connectHead[i].fd = -1; //初始化连接，无socket和该连接池中的连接【对象】绑定
        connectHead[i].instance = 1; //失效标志位置位1【失效】
        connectHead[i].iCurrsequence = 0; //当前序号统一从0开始
        //------------------

        next = &connectHead[i]; //next指向当前元素的地址，其前一个元素的data（next）会指向现在的next，串联起来

    } while (i);
    
    m_pFreeConnections = next; //此时next指向c[0]，将空闲连接表头指向首地址
    m_free_connection_n = m_connection_n; //当前链表都是空闲的
    //(3)遍历所有监听socket【监听端口】，我们为每个监听socket增加一个 连接池中的连接【说白了就是让一个socket和一个内存绑定，以方便记录该sokcet相关的数据、状态等等】
    std::vector<lpngx_listening_t>::iterator pos;
    for (pos = m_ListenSocketList.begin(); pos != m_ListenSocketList.end(); pos++)
    {
        connectHead = ngx_get_connection((*pos)->fd);
        if (connectHead == NULL)
        {
            ngx_log_stderr(errno, "CSocket::ngx_epoll_init(), ngx_get_connection() failed!");
            exit(2);
        }
        connectHead->listening = (*pos); //连接对象 和监听对象关联，方便通过连接对象找监听对象
        (*pos)->connection = connectHead; //监听对象 和连接对象关联，方便通过监听对象找连接对象
        //对监听端口的读事件设置处理方法，因为监听端口是用来等对方连接的发送三路握手的，所以监听端口关心的就是读事件
        connectHead->rhandler = &CSocket::ngx_event_accept;
        //在监听socket上增加监听事件，从而开始让监听端口履行其职责【如果不加这行，虽然端口能连上，但不会触发ngx_epoll_process_events()里边的epoll_wait()往下走】          
        if (ngx_epoll_add_event((*pos)->fd, //socket句柄
                                    1, 0,   //读写事件（只关系读事件）
                                    0,
                                    EPOLL_CTL_ADD, //事件类型，（增加，其他还有删除/修改）
                                    connectHead    //连接池中的连接
                                    ) == - 1)
        {
            exit(2);
        }
        
    }

    return 1;
}

/**********************************************************
 * 函数名称: CSocket::ngx_epoll_add_event
 * 函数描述: epoll增加事件
 * 函数参数:
 *      int fd: 句柄
 *      int readEvent:是否读事件，0不是，1是
 *      int writeEvent:是否写事件，0不是，1是
 *      uint32_t otherFlag: 其他需要额外补充的标记
 *      uint32_t eventType: 事件类型，EPOLL_CTL_ADD,EPOLL_CTL_MOD, EPOLL_CTL_DEL 
 *      lpngx_connection_t c: 对应的连接池中的连接的指针
 * 返回值:
 *    int: 成功1，失败返回-1
***********************************************************/
int CSocket::ngx_epoll_add_event(int fd, 
                                int readEvent, 
                                int writeEvent, 
                                uint32_t otherFlag, 
                                uint32_t eventType, 
                                lpngx_connection_t c)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));

    if (readEvent == 1)
    {
        //读事件，这里发现官方nginx没有使用EPOLLERR，因此我们也不用【有些范例中是使用EPOLLERR的】
        ev.events = EPOLLIN | EPOLLRDHUP; //EPOLLIN读事件，也就是read ready
        //【客户端三次握手连接进来，也属于一种可读事件】 EPOLLRDHUP 客户端关闭连接，断连
        //似乎不用加EPOLLERR，只用EPOLLRDHUP即可，EPOLLERR/EPOLLRDHUP 实际上是通过触发读写事件进行读写操作recv write来检测连接异常
        //ev.events |= (ev.events | EPOLLET);  //只支持非阻塞socket的高速模式【ET：边缘触发】，就拿accetp来说，如果加这个EPOLLET，则客户端连入时，epoll_wait()只会返回一次该事件，
                                                //如果用的是EPOLLLT【水平触发：低速模式】，则客户端连入时，epoll_wait()会被触发多次，一直到用accept()来处理；
        //https://blog.csdn.net/q576709166/article/details/8649911
        //找下EPOLLERR的一些说法：
        //a)对端正常关闭（程序里close()，shell下kill或ctr+c），触发EPOLLIN和EPOLLRDHUP，但是不触发EPOLLERR 和EPOLLHUP。
        //b)EPOLLRDHUP    这个好像有些系统检测不到，可以使用EPOLLIN，read返回0，删除掉事件，关闭close(fd);如果有EPOLLRDHUP，检测它就可以直到是对方关闭；否则就用上面方法。
        //c)client 端close()联接,server 会报某个sockfd可读，即epollin来临,然后recv一下 ， 如果返回0再掉用epoll_ctl 中的EPOLL_CTL_DEL , 同时close(sockfd)。
                //有些系统会收到一个EPOLLRDHUP，当然检测这个是最好不过了。只可惜是有些系统，上面的方法最保险；如果能加上对EPOLLRDHUP的处理那就是万能的了。
        //d)EPOLLERR      只有采取动作时，才能知道是否对方异常。即对方突然断掉，是不可能有此事件发生的。只有自己采取动作（当然自己此刻也不知道），read，write时，出EPOLLERR错，说明对方已经异常断开。
        //e)EPOLLERR 是服务器这边出错（自己出错当然能检测到，对方出错你咋能知道啊）
        //f)给已经关闭的socket写时，会发生EPOLLERR，也就是说，只有在采取行动（比如读一个已经关闭的socket，或者写一个已经关闭的socket）时候，才知道对方是否关闭了。
        //这个时候，如果对方异常关闭了，则会出现EPOLLERR，出现Error把对方DEL掉，close就可以了。
    }
    else
    {
        /* code */
    }

    if (otherFlag != 0)
    {
        ev.events |= otherFlag;
    }
     //以下这段代码抄自nginx官方,因为指针的最后一位【二进制位】肯定不是1，所以 和 c->instance做 |运算；
     //到时候通过一些编码，既可以取得c的真实地址，又可以把此时此刻的c->instance值取到
    //比如c是个地址，可能的值是 0x00af0578，对应的二进制是‭101011110000010101111000‬，而 | 1后是0x00af0579
    
    ev.data.ptr = (void*)((uintptr_t)c | c->instance); //把对象弄进去，后续来事件时，用epoll_wait()后，这个对象能取出来用 
                                                        //但同时把一个 标志位【不是0就是1】弄进去

    /*
    int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
    该函数用于控制某个epoll文件描述符上的事件，可以注册事件，修改事件，删除事件。
    args:
        epfd: 由epoll_create生成的epoll专用的文件描述符
        op:   要进行操作的的事件，可能的取值为EPOLL_CTL_ADD添加, EPOLL_CTL_MOD修改, EPOLL_CTL_DEL删除
        fd:   关联的文件描述符
        event:指向epoll_event的指针
            常用的事件类型：
            EPOLLIN: 表示对应的文件描述符有可读事件
            EPOLLOUT:表示对应的文件描述符有可写事件
            EPOLLPRI:表示对应的文件描述符有紧急的数据可读事件
            EPOLLERR:表示对应的文件描述符发生错误
            EPOLLHUP:表示对应的文件描述符被挂断
            EPOLLET:工作模式为ET(edge-triggered)边沿触发模式
    return：
        0，成功；-1，失败
    */                                                
    if (epoll_ctl(m_epollHandle, eventType, fd, &ev) == -1)
    {
        ngx_log_stderr(errno, 
        "CSocket::ngx_epoll_add_event: epoll_ctrl failed! eventType = %d, fd = %d, readEvent = %d, writeEvent = %d, otherflag = %d", eventType, fd, readEvent, writeEvent, otherFlag);
        return -1;
    }
    
    return 1;
                        
}

/**********************************************************
 * 函数名称: CSocket::ngx_epoll_process_events
 * 函数描述: 获取发生的事件消息的处理
 * 函数参数:
 *      int timer:阻塞时长，单位是毫秒
 * 返回值:
 *    int: 1撑场返回，0：有问题的返回，不影响流程的继续进行
***********************************************************/
//本函数被ngx_process_events_and_timers()调用，而ngx_process_events_and_timers()是在子进程的死循环中被反复调用

int CSocket::ngx_epoll_process_events(int timer)
{
    //等待事件，事件会返回到m_events里，最多返回NGX_MAX_EVENTS个事件【因为我只提供了这些内存】；
    //阻塞timer这么长时间除非：a)阻塞时间到达 b)阻塞期间收到事件会立刻返回c)调用时有事件也会立刻返回d)如果来个信号，比如你用kill -1 pid测试
    //如果timer为-1则一直阻塞，如果timer为0则立即返回，即便没有任何事件
    //返回值：有错误发生返回-1，错误在errno中，比如你发个信号过来，就返回-1，错误信息是(4: Interrupted system call)
    //       如果你等待的是一段时间，并且超时了，则返回0；
    //       如果返回>0则表示成功捕获到这么多个事件【返回值里】
    int events = epoll_wait(m_epollHandle, m_events, NGX_MAX_EVENTS, timer);

    if (events == -1)
    {
        //有错误发生，发送某个信号给本进程就可以导致这个条件成立，
        //#define EINTR  4，EINTR错误的产生：当阻塞于某个慢系统调用的一个进程捕获某个信号且相应信号处理函数返回时，该系统调用可能返回一个EINTR错误。
        //例如：在socket服务器端，设置了信号捕获机制，有子进程，当在父进程阻塞于慢系统调用时由父进程捕获到了一个有效信号时，内核会致使accept返回一个EINTR错误(被中断的系统调用)。
        if (errno == EINTR)
        {
            //信号所致，直接返回，一般认为不是什么问题，记录下日志即可，因为一般也不会人为给worker进程发送消息
            ngx_log_error_core(NGX_LOG_INFO, errno, "CSocket::ngx_epoll_process_events: epoll_wait failed");
            return 1;
        }
        else
        {
            ngx_log_error_core(NGX_LOG_INFO, errno, "CSocket::ngx_epoll_process_events: epoll_wait failed");
            return 0;

        }
        
    }
    
    if (events == 0) //超时，没有事件
    {
        if (timer != -1)
        {
            //阻塞事件到了，超时了，正常返回
            return 1;
        }
        //持续阻塞，但是却没有事件，是有问题的存在
        ngx_log_error_core(NGX_LOG_INFO, errno, "CSocket::ngx_epoll_process_events: epoll_wait persistently blocking, but no events back");
        return 0;
    }
    //会惊群，一个telnet上来，4个worker进程都会被惊动，都执行下边这个
    //ngx_log_stderr(errno,"惊群测试1:%d",events);
    
    //正常收到事件
    lpngx_connection_t c;
    uintptr_t instance;
    uint32_t revents;
    for (int i = 0; i < events; i++) //events为返回的实际事件的数量
    {
        c = (lpngx_connection_t)(m_events[i].data.ptr); //ngx_epll_add_event()给进去的，这里再取回来
        instance = (uintptr_t) c & 1; //将地址的最后一位取出来
        c = (lpngx_connection_t)((uintptr_t)c & (uintptr_t) ~1); //去掉最后一位，得到C的地址
        
        //一个套接字，当关联一个连接池中的连接(对象)时，套接字是要赋值给c->fd
        //关闭连接时，在函数ngx_close_accepted_connection会把c->fd置为-1，
        //假如epoll_wait返回三个事件，处理第一个事件的时候，由于业务需要，关闭了这个连接，此时c->fd被置为-1；
        //第二个事件正常处理
        //假如这第三个事件，也跟第一个事件对应的是同一个连接，那这个条件就会成立；那么这种事件，属于过期事件，不该处理
        if (c->fd == -1)
        {
            ngx_log_error_core(NGX_LOG_DEBUG, 0, "CSocket::ngx_epoll_process_events:stale event:%p", c);
            continue;
        }

        if (c->instance != instance)
        {
            /* 假如epoll_wait一次放回3个事件，第一个与第三个对应的是同一个连接的数据写入事件(即可读事件)，第二个是建立连接的新事件
            (1) 假如第一个事件处理的是数据的写入等操作，即采用的是accept返回的socket，由于业务的需要，关闭了这个连接.
                调用ngx_free_connection将该连接归还给连接池,同时释放了socket(假如是50)，c->fd被置为了-1。
            (2) 在ngx_epoll_process_events方法的循环中开始处理第二个事件的时候，建立新的连接，由于第一个连接的socket刚刚被释放，很可能将刚释放的50复用给了该连接，
                而此时从连接池中取出的连接也刚好是第一个连接释放后的空闲连接。在函数ngx_get_connection中,重新将50的socket描述符赋值给了c->fd.
            (3) 因此，在循环中处理第三个事件时，第三个连接会仍然认为该连接c是自己的连接,仅仅判断c->fd == -1是不能过滤该过期事件了,这个事件就是过期了。
                如何判断该过期的事件，依靠instance标志位能够解决这个问题，当调用ngx_get_connection从连接池中获取一个新连接时，我们把instance标志位置反，所以这个条件如果不成立，说明这个连接已经被挪作他用了；
             */
            ngx_log_error_core(NGX_LOG_DEBUG, 0, "CSocket::ngx_epoll_process_events:stale event:%p", c);
            continue;
        }

        //到这里都是正常事件了
        revents = m_events[i].events; //取出事件类型
        if (revents & (EPOLLERR | EPOLLHUP)) //例如对方close掉套接字，这里会感应到【换句话说：如果发生了错误或者客户端断连】
        {
            revents |= EPOLLIN | EPOLLOUT; //EPOLLIN:表示对应的连接上的数据可以读取（TCP链接的远端主动关闭连接，也相当于可读事件，因为本服务器小处理发送来的FIN包）
                                            //EPOLLOUT：表示对应的连接上可以写入数据发送【写准备好】
        }

        if (revents & EPOLLIN)
        {
            //客户端新连入,这个会成立
            //注意括号的运用来正确设置优先级，防止编译出错；【如果是个新客户连入
            //如果新连接进入，这里执行的应该是CSocekt::ngx_event_accept(c)】            
            //如果是已经连入，发送数据到这里，则这里执行的应该是 CSocekt::ngx_wait_request_handler
            (this->*(c->rhandler))(c);

        }
        if (revents & EPOLLOUT)
        {
            //代扩展
            ngx_log_stderr(errno, "aaaaaaaa");
        }
        
        
    }

    return 1;
    
}