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


ngx_connection_s::ngx_connection_s()
{
    iCurrsequence = 0;
    pthread_mutex_init(&logicProcMutex, NULL); //互斥量初始化
}
ngx_connection_s::~ngx_connection_s() //析构函数
{
    pthread_mutex_destroy(&logicProcMutex); //互斥量释放
}

/**********************************************************
 * 函数名称: ngx_connection_s::getOneToUse
 * 函数描述: 获取新的连接的时候,进行的初始化操作
 * 函数参数:
 *      void
 * 返回值:
 *    void
***********************************************************/
void ngx_connection_s::getOneToUse()
{
    //初始化信息
    iCurrsequence++;
    fd = -1;
    curState = _PKG_HD_INIT;
    pRecvBuf = dataHeadInfo;
    iRecvLen = sizeof(COMM_PKG_HEADER);
    precvMemPointer = NULL;
    iThrowSendCount = 0;
    pSendMemHeader = NULL;
    events = 0;
    lastPingTime = time(NULL);
    

}
/**********************************************************
 * 函数名称: ngx_connection_s::putOneToFree
 * 函数描述: 回收连接的一些释放内存的操作
 * 函数参数:
 *      空
 * 返回值:
 *      空
***********************************************************/
void ngx_connection_s::putOneToFree()
{
    iCurrsequence++;
    CMemory& Cmem = CMemory::GetMemory();
    if (precvMemPointer != NULL)
    {
        Cmem.FreeMemory(precvMemPointer);
        precvMemPointer = NULL;
    }
    if (pSendMemHeader != NULL)
    {
        Cmem.FreeMemory(pSendMemHeader);
        pSendMemHeader = NULL;
    }
    iThrowSendCount = 0;
    
    
}

/**********************************************************
 * 函数名称: CSocket::initConnection
 * 函数描述: 初始化连接池
 * 函数参数:
 *      空
 * 返回值:
 *      空
***********************************************************/
void CSocket::initConnection()
{
    lpngx_connection_t p_Conn;
    CMemory& Cmem = CMemory::GetMemory();
    int iLenConnPool = sizeof(ngx_connection_t);
    for (int i = 0; i < m_worker_connections; i++)
    {
        //清理内存,因为这里分配内存new char,无法执行构造函数
        p_Conn = (lpngx_connection_t)Cmem.AllocMemory(iLenConnPool, true);
        //手动调用构造函数,因为AllocMemory无法调用构造函数
        //定位new表达式，可以在指定地址区域(栈区、堆区、静态区)构造对象,在特定地址调用构造函数
        p_Conn = new(p_Conn) ngx_connection_s(); //定位new,释放时需显示调用p_Conn->~ngx_connection_s()
        p_Conn->getOneToUse();
        m_connectionList.push_back(p_Conn); //总的连接池(包括空闲和非空闲)
        m_freeConnectionList.push_back(p_Conn); //空闲连接池

    }
    m_free_connection_n = m_total_connection_n = m_connectionList.size();
    return;
    
}
/**********************************************************
 * 函数名称: CSocket::initConnection
 * 函数描述: 释放连接池及内存
 * 函数参数:
 *      空
 * 返回值:
 *      空
***********************************************************/
void CSocket::clearConnection()
{
    lpngx_connection_t p_Conn;
    CMemory& Cmem = CMemory::GetMemory();
    while (!m_connectionList.empty())
    {
        p_Conn = m_connectionList.front();
        m_connectionList.pop_front();
        p_Conn->~ngx_connection_s();
        Cmem.FreeMemory(p_Conn);
    }
    
}
/**********************************************************
 * 函数名称: CSocket::ngx_get_connection
 * 函数描述: 从连接池中获取一个空闲连接【当一个客户端连接TCP进入，
 *          把这个TCP连接和连接池中的一个连接【对象】绑到一起，
 *          后续 我可以通过这个连接，把这个对象拿到，因为对象里边可以记录各种信息】
 * 函数参数:
 *      int isock:socket描述符
 * 返回值:
 *    lpngx_connection_t: 返回一个空闲连接
***********************************************************/


lpngx_connection_t CSocket::ngx_get_connection(int isock)
{
    //因为可能有其他线程访问m_freeConnectionList, m_Connectionlist
    CMutexLock lock(&m_connectionMutex);
    if (!m_freeConnectionList.empty())
    {
        lpngx_connection_t p_Conn = m_freeConnectionList.front();
        m_freeConnectionList.pop_front();
        p_Conn->getOneToUse();
        --m_free_connection_n;
        p_Conn->fd = isock;
        return p_Conn;
    }
    //走到这里表示没有空闲连接了,重新创建连接
    CMemory & Cmem = CMemory::GetMemory();
    lpngx_connection_t p_Conn = (lpngx_connection_t)Cmem.AllocMemory(sizeof(ngx_connection_s), true);
    p_Conn = new(p_Conn)ngx_connection_s();
    p_Conn->getOneToUse();
    m_connectionList.push_back(p_Conn);
    ++m_total_connection_n;
    p_Conn->fd = isock;
    return p_Conn;

#if 0
    lpngx_connection_t connectHead = m_pFreeConnections; //空闲连接链表头
    
    if (connectHead == NULL)
    {
        //系统应该控制连接数量，防止空闲连接被耗尽
        ngx_log_stderr(0, "CSocket::ngx_get_connection: free connection array is empty!");
        return NULL;
    }
    
    m_pFreeConnections = connectHead->next; //指向连接池中下一个未用的节点
    m_free_connection_n -= 1; //空闲连接减一

    //(1) 保存该空闲连接中的一些信息
    uintptr_t instance = connectHead->instance; //初始情况下是失效的1；
    uint64_t iCurrsequence = connectHead->iCurrsequence;
    //其他内容后续添加...

    //(2)清空connectHead中的不需要的其他内容
    memset(connectHead, 0, sizeof(ngx_connection_t));
    connectHead->fd = isock; //套接字要保存起来，这东西具有唯一性
    connectHead->curState = _PKG_HD_INIT; //收报状态处于初始化状态,准备接收数据
    connectHead->pRecvBuf = connectHead->dataHeadInfo; //收包我要先收到这里来，因为我要先收包头，所以收数据的buff直接就是dataHeadInfo
    connectHead->iRecvLen = sizeof(COMM_PKG_HEADER); //这里指定收数据的长度，这里先要求收包头这么长字节的数据
    connectHead->ifNewRecvMem = false; //标记我们并没有new内存，所以不用释放
    connectHead->pNewMemPointer = NULL;
    //(3)把保存的数据重新赋值给connectHead
    connectHead->instance = !instance; //取反，之前是失效，现在是有效的
    connectHead->iCurrsequence =iCurrsequence;
    connectHead->iCurrsequence++;
    return connectHead;
#endif
}


/**********************************************************
 * 函数名称: CSocket::ngx_free_connection
 * 函数描述: 释放连接
 * 函数参数:
 *      lpngx_connection_t c 连接
 * 返回值:
 *    void
***********************************************************/
void CSocket::ngx_free_connection(lpngx_connection_t c)
{
    CMutexLock lock(&m_connectionMutex);
    c->putOneToFree();
    m_freeConnectionList.push_back(c);
    m_free_connection_n++;
    return;
#if 0
    //将释放的连接插入的连接表的表头
    c->next = m_pFreeConnections;
    m_pFreeConnections = c;
    //回收后，该值就增加1,以用于判断某些网络事件是否过期【一被释放就立即+1也是有必要的】
    c->iCurrsequence++;
    m_free_connection_n++;
    return;
#endif
}

/**********************************************************
 * 函数名称: CSocket::inRecyConnectionQueue
 * 函数描述: 将要回收的连接放到一个队列中来,后续用专门的线程来回收
 *          有些连接,可能有线程正在访问,不能立即回收,所以隔一段时间再回收,做一个缓冲
 * 函数参数:
 *      lpngx_connection_t c 连接
 * 返回值:
 *    void
***********************************************************/
void CSocket::inRecyConnectionQueue(lpngx_connection_t pConn)
{
    std::list<lpngx_connection_t>::iterator pos;
    CMutexLock lock(&m_recyConnQueueMutex);
    //判断是否在回收站中已经有了该连接,避免重复放入
    for (pos = m_recyConnectionList.begin(); pos != m_recyConnectionList.end(); ++pos)
    {
        if ((*pos) == pConn)
        {
            break;
        }
        
    }
    if (pos != m_recyConnectionList.end())
    {
        //找到了
        return;
    }
    pConn->inRecyTime = time(NULL); //记录回时间
    ++pConn->iCurrsequence;
    m_recyConnectionList.push_back(pConn);
    ++m_totol_recyConnection_n;
    return;
}

/**********************************************************
 * 函数名称: CSocket::serverRecyConnectionThread
 * 函数描述: 将要回收的连接放到一个队列中来,后续用专门的线程来回收
 *          有些连接,可能有线程正在访问,不能立即回收,所以隔一段时间再回收,做一个缓冲
 * 函数参数:
 *      lpngx_connection_t c 连接
 * 返回值:
 *    void
***********************************************************/
void* CSocket::serverRecyConnectionThread(void *threadData)
{
    ThreadItem *pThread = static_cast<ThreadItem*>(threadData);
    CSocket *pSockObj = pThread->_pThis;
    time_t currtime;
    int err;
    std::list<lpngx_connection_t>::iterator pos, posEnd;
    lpngx_connection_t pConn;
    for (; ;)
    {
        usleep(200 * 1000); //200ms
        if ((pSockObj->m_totol_recyConnection_n > 0)  && (g_stopEvent == 0))
        {
            currtime = time(NULL);
            err = pthread_mutex_lock(&pSockObj->m_recyConnQueueMutex);
            if (err != 0)
            {
                ngx_log_stderr(err, "CSocket::serverRecyConnectionThread pthread_mutex_lock failed, errno = %d", err);
            }
            pos = pSockObj->m_recyConnectionList.begin();
            posEnd = pSockObj->m_recyConnectionList.end();
            for (; pos != posEnd; pos++)
            {
                pConn = (*pos);
                if ((pConn->inRecyTime + pSockObj->m_recyConnectionWaitTime) > currtime)
                {
                    continue; //没到释放时间
                }
                //到释放时间了,应该进行释放
                /*==================code========================*/
                if (pConn->iThrowSendCount > 0)
                {
                    ngx_log_stderr(0, "CSocket::serverRecyConnectionThread, iThrowSendCount greater than 0 when free Connection");
                }

                //开始释放
                --pSockObj->m_totol_recyConnection_n;
                pos = pSockObj->m_recyConnectionList.erase(pos);
                pSockObj->ngx_free_connection(pConn); //释放连接到空闲连接中去
                
            }
            err = pthread_mutex_unlock(&pSockObj->m_recyConnQueueMutex);
            if (err != 0)
            {
                ngx_log_stderr(err, "CSocket::serverRecyConnectionThread pthread_mutex_unlock failed, errno = %d", err);
            }
  
        }//end if
        if (g_stopEvent == 1)
        {
            if (pSockObj->m_totol_recyConnection_n > 0)
            {
                //程序退出,不管时间是否到,都要释放
                err = pthread_mutex_lock(&pSockObj->m_recyConnQueueMutex);
                if (err != 0)
                {
                    ngx_log_stderr(err, "CSocket::serverRecyConnectionThread pthread_mutex_lock2 failed, errno = %d", err);
                }
                pos = pSockObj->m_recyConnectionList.begin();
                posEnd = pSockObj->m_recyConnectionList.end();
                for (; pos != posEnd; pos++)
                {
                    pConn = (*pos);
                    //开始释放
                    --pSockObj->m_totol_recyConnection_n;
                    pos = pSockObj->m_recyConnectionList.erase(pos);
                    pSockObj->ngx_free_connection(pConn); //释放连接到空闲连接中去
                }
                err = pthread_mutex_unlock(&pSockObj->m_recyConnQueueMutex);
                if (err != 0)
                {
                    ngx_log_stderr(err, "CSocket::serverRecyConnectionThread pthread_mutex_unlock2 failed, errno = %d", err);
                }
            }
            break;
        }
        
    }
   return (void*)0;
}