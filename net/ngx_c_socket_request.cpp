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

//来数据时候的处理，当连接上有数据来的时候，本函数会被ngx_epoll_process_events()所调用  ,官方的类似函数为ngx_http_wait_request_handler();
void CSocket::ngx_read_request_handler(lpngx_connection_t c)
{  
    ngx_log_stderr(errno,"22222222222222222222222.");
    ssize_t recLen = recvProc(c, c->pRecvBuf, c->iRecvLen);
    if (recLen <= 0)
    {
        ngx_log_stderr(0, "receive message less than 0");
        return;
    }
    //收到了一些字节(>0),开始判断收了多少数据
    switch (c->curState)
    {
    case _PKG_HD_INIT:
        if (recLen == m_iLenPkgHeader)//正好收到完整的包头,然后进行拆包
        {
            ngx_log_stderr(0, "_PKG_HD_INIT:receive head ok");

            ngx_wait_request_handler_proc_phase1(c);
        }
        else
        {

            ngx_log_stderr(0, "_PKG_HD_INIT:receive part head Len = %d", recLen);
			//收到的包头不完整--我们不能预料每个包的长度，也不能预料各种拆包/粘包情况，所以收到不完整包头【也算是缺包】是很可能的；
            c->curState = _PKG_HD_RECVING; //接收包头中
            c->pRecvBuf = c->pRecvBuf + recLen;
            c->iRecvLen = c->iRecvLen - recLen;

        }
        break;
    case _PKG_HD_RECVING:
        if (c->iRecvLen == recLen) //接收完剩下没有接收完整的包头
        {
            ngx_log_stderr(0, "_PKG_HD_RECVING: receive head ok");
            //再次接收后,包头已经接收完整了
            ngx_wait_request_handler_proc_phase1(c);
        }
        else
        {
            ngx_log_stderr(0, "_PKG_HD_RECVING:receive part head Len = %d", recLen);
            //包头还是没有接收完整
            c->pRecvBuf = c->pRecvBuf + recLen;
            c->iRecvLen = c->iRecvLen - recLen;
        }
        break;
    case _PKG_BD_INIT:
        if (recLen == c->iRecvLen)
        {
            //包头刚收完,准备接收包体
            ngx_wait_request_handler_proc_phaselast(c);
        }
        else
        {
            c->curState = _PKG_BD_RECVING;
            c->pRecvBuf = c->pRecvBuf + recLen;
            c->iRecvLen = c->iRecvLen - recLen;
        }
        break;
    case _PKG_BD_RECVING:
        if (c->iRecvLen == recLen)
        {
            //包体接收完整
            ngx_wait_request_handler_proc_phaselast(c);
        }
        else
        {
            c->pRecvBuf = c->pRecvBuf + recLen;
            c->iRecvLen = c->iRecvLen - recLen;
        }
        break;
  
    default:
        break;
    }

   return; 
    
}
/**********************************************************
 * 函数名称: CSocket::recvProc
 * 函数描述: 接收数据专用函数--引入这个函数是为了方便，
 *          如果断线，错误之类的，这里直接释放连接池中连接，然后直接关闭socket，
 *          以免在其他函数中还要重复的干这些事
 * 函数参数:
 *      lpngx_connection_t c 连接
 *      char *buff 接收数据的缓冲区
 *      ssize_t bufLen 要接收的数据大小
 * 返回值:
 *    ssize_t 返回-1,表示有问题发生且在这把问题处理完了
 *            返回>0,表示收到的实际字节数
***********************************************************/
ssize_t CSocket::recvProc(lpngx_connection_t c, char *buff, ssize_t bufLen)
{
    ssize_t n;
    /*
        int recv( int fd, void *buf, int len, int flags )
        不论是客户还是服务器应用程序都用recv函数从TCP连接的另一端接收数据。
        参数:
            int fd: 指定接收端套接字描述符；
            void* buff: 数指明一个缓冲区，该缓冲区用来存放recv函数接收到的数据；
            int len: buf的长度；
            int flags: 一般置0。
    */
    n = recv(c->fd, buff, bufLen, 0);
    if (n ==0)
    {
        //客户端关闭(应该是正常完成了4次挥手),直接回收连接,关闭socket
        ngx_close_connection(c);
        return -1;
    }

    if (n < 0) //有错误发生
    {
        //EAGAIN和EWOULDBLOCK[【这个应该常用在hp上】应该是一样的值，表示没收到数据.
        //一般来讲，在ET模式下会出现这个错误，因为ET模式下是不停的recv肯定有一个时刻收到这个errno，但LT模式下一般是来事件才收，所以不该出现这个返回值
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            ngx_log_stderr(errno, "CSocket:: recvproc: errno == EAGAIN or EWOULDBLOCK");
            return -1;
        }
        //EINTR错误的产生：当阻塞于某个慢系统调用的一个进程捕获某个信号且相应信号处理函数返回时，该系统调用可能返回一个EINTR错误。
        //例如：在socket服务器端，设置了信号捕获机制，有子进程，当在父进程阻塞于慢系统调用时由父进程捕获到了一个有效信号时，内核会致使accept返回一个EINTR错误(被中断的系统调用)。

        if (errno == EINTR)
        {
            ngx_log_stderr(errno, "CSocket:: recvproc: errno == EINTR");
            return -1;
        }

        //所有以下错误都认为是异常,需要关闭套接字,回收连接
        if (errno == ECONNRESET)
        {
            /* 
                //如果客户端没有正常关闭socket连接，却关闭了整个运行程序【真是够粗暴无理的，应该是直接给服务器发送rst包而不是4次挥手包完成连接断开】，那么会产生这个错误            
                //10054(WSAECONNRESET)--远程程序正在连接的时候关闭会产生这个错误--远程主机强迫关闭了一个现有的连接
                //算常规错误吧【普通信息型】，日志都不用打印，没啥意思，太普通的错误
                //do nothing
                //....一些大家遇到的很普通的错误信息，也可以往这里增加各种，代码要慢慢完善，一步到位，不可能，很多服务器程序经过很多年的完善才比较圆满；
            */
           ngx_log_stderr(errno, "CSocket::recv error:%s", strerror(errno));
        }
        else
        {
            ngx_log_stderr(errno, "CSocket::recv error,connection closed!");
        }
        ngx_close_connection(c);
        return -1;
        
    }
    
    //能走到这里的，就认为收到了有效数据
    return n; //返回收到的字节数 
}

/**********************************************************
 * 函数名称: CSocket::ngx_wait_request_handler_proc_phase1
 * 函数描述: 包头接收完整后的处理函数,phase1
 * 函数参数:
 *      lpngx_connection_t c 连接
 * 返回值:
 *    void
***********************************************************/
void CSocket::ngx_wait_request_handler_proc_phase1(lpngx_connection_t c)
{
    CMemory& Cmem = CMemory::GetMemory();
    LPCOMM_PKG_HEADER pPkgHeader;
    pPkgHeader = (LPCOMM_PKG_HEADER)c->dataHeadInfo;
    unsigned short e_pkgLen;
    e_pkgLen = ntohs(pPkgHeader->pkgLen); //注意这里网络字节序转本机序
    ngx_log_stderr(0, "Packege len is %d", e_pkgLen);

    //恶意包或者错误包的判断
    if (e_pkgLen < m_iLenPkgHeader)
    {
        //伪造包/或者包错误，否则整个包长怎么可能比包头还小？（整个包长是包头+包体，就算包体为0字节，那么至少e_pkgLen == m_iLenPkgHeader）
        //报文总长度 < 包头长度，认定非法用户，废包
        //状态和接收位置都复原，这些值都有必要，因为有可能在其他状态比如_PKG_HD_RECVING状态调用这个函数；
        c->curState = _PKG_HD_INIT;      
        c->pRecvBuf = c->dataHeadInfo;
        c->iRecvLen = m_iLenPkgHeader;
        ngx_log_stderr(0, "all the length is less than header!");
    }
    else if (e_pkgLen > (_PKG_MAX_LENGTH - 1000))
    {
        //恶意包,数据包太大,认定非法用户
        //复原状态
        c->curState = _PKG_HD_INIT;      
        c->pRecvBuf = c->dataHeadInfo;
        c->iRecvLen = m_iLenPkgHeader;
        ngx_log_stderr(0, "package is too large!");
    }
    //合法的数据包,正常处理
    else
    {
        //分配内存,开始接收包体,长度是 消息头长度  + 包头长度 + 包体长度
        char *pTmpBuffer = (char*)Cmem.AllocMemory(m_iLenMsgHeader + e_pkgLen, false); //不需要memset
        //c->ifNewRecvMem = true; //标记动态申请了内存,以便后续释放
        c->precvMemPointer = pTmpBuffer; //内存开始指针
        ngx_log_stderr(0, "help me!");


        //a)先填写消息头内容
        LPSTRUC_MSG_HEADER pTmpMsgHeader = (LPSTRUC_MSG_HEADER)pTmpBuffer;
        pTmpMsgHeader->pConn = c;
        pTmpMsgHeader->iCurrsequence = c->iCurrsequence; //收到包时的连接池中连接序号记录到消息头里来，以备将来用；

        //b)包头内容
        pTmpBuffer += m_iLenMsgHeader;
        memcpy(pTmpBuffer, pPkgHeader, m_iLenPkgHeader); //把收到的包头拷贝进去
        if (e_pkgLen == m_iLenPkgHeader)
        {
            //该报文只有包头无包体(允许只有一个包头)
            //相当于接收完整了,直接进入消息队列,进行后续业务逻辑处理
            ngx_log_stderr(0, "the message has only head!");

            ngx_wait_request_handler_proc_phaselast(c);
        }
        else
        {
            //开始接收包体
            c->curState = _PKG_BD_INIT; //当前状态发生改变，包头刚好收完，准备接收包体
            c->pRecvBuf = pTmpBuffer + m_iLenPkgHeader; //pTmpBuffer指向包体的开始位置
            c->iRecvLen = e_pkgLen - m_iLenPkgHeader; //e_pkgLen是整个数据包(包头+包体)的大小,求出包体的长度

        }
        
    }
    
   return; 
    
}


/**********************************************************
 * 函数名称: CSocket::ngx_wait_request_handler_proc_phaselast(lpngx_connection_t c)
 * 函数描述: 收到一个完整的数据包以后的处理
 * 函数参数:
 *      lpngx_connection_t c 连接
 * 返回值:
 *    void
***********************************************************/
void CSocket::ngx_wait_request_handler_proc_phaselast(lpngx_connection_t c)
{

    //inMsgRecvQueue(c->pNewMemPointer, irmqc); //返回消息队列当前的数量irmqc
    //激发线程池中的某个线程来处理业务逻辑
    g_threadpool.inMsgRecvQueueAndSignal(c->precvMemPointer);
    c->precvMemPointer = NULL;
    c->curState = _PKG_HD_INIT;
    c->pRecvBuf = c->dataHeadInfo;
    c->iRecvLen = m_iLenMsgHeader;
    return;
}


/**********************************************************
 * 函数名称: CSocket::inMsgRecvQueue
 * 函数描述: 收到一个完整的数据包后,将包放入消息队列,格式是(消息头+包头+包体)
 * 函数参数:
 *      char *buf 消息的内存
 * 返回值:
 *    void
***********************************************************/

// void CSocket::inMsgRecvQueue(char *buf, int &imsgQueCount)
// {
//     CMutexLock lock(&m_recvMsgQueueMutex); //自动加锁
//     m_MsgRecvQueue.push_back(buf); //入消息队列
//     ++m_iRecvMsgQueueCount; //消息队列长度,比m_MsgRecvQueue.size()更高效
//     imsgQueCount = m_iRecvMsgQueueCount; //接收消息队列当前信息数量
//     ngx_log_stderr(0,"received a messaage!");
// }
ssize_t sendProc(lpngx_connection_t c, char *buff, ssize_t size)
{

}
#if 0
/**********************************************************
 * 函数名称: CSocket::outMsgRecvQueue
 * 函数描述: 从消息队列中取出消息,以备后续处理
 * 函数参数:
 *      无
 * 返回值:
 *    char * 取出的消息
***********************************************************/
char *CSocket::outMsgRecvQueue()
{
    CMutexLock lock(&m_recvMsgQueueMutex);
    if (m_MsgRecvQueue.empty())
    {
        ngx_log_stderr(0, "empty msg!");
        return NULL;
    }
    char * sTmpMsgBuf = m_MsgRecvQueue.front();
    m_MsgRecvQueue.pop_front();
    --m_iRecvMsgQueueCount;
    return sTmpMsgBuf;
    
}
#endif
void CSocket::threadRecvProcFunc(char *pMsgBuf)
{

    return;
}