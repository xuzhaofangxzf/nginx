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
    lpngx_connection_t connectHead = m_pFreeConnections; //空闲连接链表头
    
    if (connectHead == NULL)
    {
        //系统应该控制连接数量，防止空闲连接被耗尽
        ngx_log_stderr(0, "CSocket::ngx_get_connection: free connection array is empty!");
        return NULL;
    }
    
    m_pFreeConnections = connectHead->data; //指向连接池中下一个未用的节点
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
    //将释放的连接插入的连接表的表头
    c->data = m_pFreeConnections;
    m_pFreeConnections = c;
    //回收后，该值就增加1,以用于判断某些网络事件是否过期【一被释放就立即+1也是有必要的】
    c->iCurrsequence++;
    m_free_connection_n++;
    return;
}