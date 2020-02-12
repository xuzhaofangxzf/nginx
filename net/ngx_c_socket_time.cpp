//和时间先关的函数
#include <stdio.h>
#include <unistd.h>
#include "ngx_c_socket.hpp"
#include "ngx_func.hpp"
/**********************************************************
 * 函数名称: CSocket::addToTimeQueue
 * 函数描述: 设置剔除时钟(向map表中增加内容)
 * 函数参数:
 *      lpngx_connection_t pConn
 * 返回值:
 *    void
***********************************************************/
void CSocket::addToTimeQueue(lpngx_connection_t pConn)
{
    CMemory &CMem = CMemory::GetMemory();
    time_t futureTime = time(NULL);
    futureTime += m_iWaitTime;
    CMutexLock lock(&m_timeQueueMutex);
    LPSTRUC_MSG_HEADER tmpMsgHeader = (LPSTRUC_MSG_HEADER)CMem.AllocMemory(m_iLenMsgHeader, false);
    tmpMsgHeader->pConn = pConn;
    tmpMsgHeader->iCurrsequence = pConn->iCurrsequence;
    m_timerQueueMap.insert(std::make_pair(futureTime, tmpMsgHeader));
    m_cur_size++;
    m_timer_value = getEarliestTime(); //计时队列头部时间即最早的时间
    return;

}
/**********************************************************
 * 函数名称: CSocket::removeFirstTimer
 * 函数描述: 从时间队列中获取队列中最早时间的元素
 * 函数参数:
 *      空
 * 返回值:
 *    time_t: 时间队列中最早时间的元素
***********************************************************/
time_t CSocket::getEarliestTime()
{
    std::multimap<time_t, LPSTRUC_MSG_HEADER>::iterator pos;
    pos = m_timerQueueMap.begin();
    return pos->first;
}
/**********************************************************
 * 函数名称: CSocket::removeFirstTimer
 * 函数描述: 从m_timeQueuemap移除最早的时间，并把最早这个时间所在的项的值所对应的指针返回，
 *          调用者负责互斥，所以本函数不用互斥，
 * 函数参数:
 *      空
 * 返回值:
 *    LPSTRUC_MSG_HEADER: 时间队列中最早的元素中对应的消息指针
***********************************************************/
LPSTRUC_MSG_HEADER CSocket::removeFirstTimer()
{
    std::multimap<time_t, LPSTRUC_MSG_HEADER>::iterator pos;
    LPSTRUC_MSG_HEADER pTmp;
    if (m_cur_size <= 0)
    {
        return NULL;
    }
    pos = m_timerQueueMap.begin();
    pTmp = pos->second;
    m_timerQueueMap.erase(pos);
    --m_cur_size;
    return pTmp;
    
}
/**********************************************************
 * 函数名称: CSocket::getOverTimeTimer
 * 函数描述: 根据当前的时间,从m_timeQueuemap找到比这个时间更老(更早)的1个节点，也就是超时的节点
 *          调用者负责互斥，所以本函数不用互斥，
 * 函数参数:
 *      空
 * 返回值:
 *    LPSTRUC_MSG_HEADER: 时间队列中最早的元素中对应的消息指针
***********************************************************/
LPSTRUC_MSG_HEADER CSocket::getOverTimeTimer(time_t curTime)
{
    if (m_cur_size <= 0)
    {
        return NULL;
    }
    CMemory &Cmem = CMemory::GetMemory();
    LPSTRUC_MSG_HEADER pTmp;
    time_t earliestTime = getEarliestTime();
    if (earliestTime <= curTime)
    {
        if (m_ifTimeOutKick == 1)
        {
            pTmp = removeFirstTimer(); //把最早的节点从时间队列中删除，并把元素中的第二项取出来
        }
        // //不要求超时就剔除，什么都不需要做，节点的时间也不需要更新，反正已经超时了，更新也没有意义
        // time_t newInQueueTime = curTime + m_iWaitTime;
        if (m_cur_size > 0)
        {
            m_timer_value = getEarliestTime();
        }
        return pTmp;
    }
    return NULL;
}
/**********************************************************
 * 函数名称: CSocket::getOverTimeTimer
 * 函数描述: 把指定用户TCP连接从timer表中删除
 * 函数参数:
 *      lpngx_connection_t pConn
 * 返回值:
 *    void
***********************************************************/
void CSocket::deletFromTimerQueue(lpngx_connection_t pConn)
{
    std::multimap<time_t, LPSTRUC_MSG_HEADER>::iterator pos;
    CMemory &Cmem = CMemory::GetMemory();
    CMutexLock lock(&m_timeQueueMutex);
    for (pos = m_timerQueueMap.begin(); pos != m_timerQueueMap.end();)
    {
        if (pos->second->pConn == pConn)
        {
            Cmem.FreeMemory(pos->second);
            pos = m_timerQueueMap.erase(pos);
            --m_cur_size;
        }
        else
        {
            pos++;
        }
        
    }
    if (m_cur_size > 0)
    {
        m_timer_value = getEarliestTime();
    }
    return;
}
/**********************************************************
 * 函数名称: CSocket::clearAllFromTimerQueue
 * 函数描述: 清理事件队列中所有的内容
 * 函数参数:
 *      lpngx_connection_t pConn
 * 返回值:
 *    void
***********************************************************/
void CSocket::clearAllFromTimerQueue()
{
    std::multimap<time_t, LPSTRUC_MSG_HEADER>::iterator pos;
    CMemory &Cmem = CMemory::GetMemory();
    for (pos = m_timerQueueMap.begin(); pos != m_timerQueueMap.end(); pos++)
    {
        Cmem.FreeMemory(pos->second);
        --m_cur_size;
    }
    m_timerQueueMap.clear();
    return;
    
}
/**********************************************************
 * 函数名称: CSocket::clearAllFromTimerQueue
 * 函数描述: 时间队列监视和处理函数，处理到期剔除不发心跳包的用户线程入口函数
 * 函数参数:
 *      void *threadData：线程函数的参数
 * 返回值:
 *    void *
***********************************************************/
void * CSocket::serverTimerQueueMonitorThread(void *threadData)
{
    ThreadItem *pThread = static_cast<ThreadItem*>(threadData);
    CSocket *pSocketObj = pThread->_pThis;
    time_t absoluteTime, curTime;
    int err;
    while (g_stopEvent == 0)
    {
        //这里先做一个初步的判断，看是否满足条件，避免无条件加锁造成系统损耗
        if (pSocketObj->m_cur_size > 0)
        {
            //时间队列中最近发生的时间放到absoluteTime里边
            absoluteTime = pSocketObj->m_timer_value;
            curTime = time(NULL);
            if (absoluteTime < curTime)
            {
                std::list<LPSTRUC_MSG_HEADER> m_lsIdleList; //保存要处理的内容
                LPSTRUC_MSG_HEADER result;
                err = pthread_mutex_lock(&pSocketObj->m_timeQueueMutex);
                if (err != 0)
                {
                    ngx_log_stderr(err, "CSocket::serverTimerQueueMonitorThread: pthread_mutex_lock failed");
                }
                while ((result = pSocketObj->getOverTimeTimer(curTime)) != NULL)
                {
                    m_lsIdleList.push_back(result); //把需要处理的超时链接放入队列
                }
                err = pthread_mutex_unlock(&pSocketObj->m_timeQueueMutex);
                if (err != 0)
                {
                    ngx_log_stderr(err, "CSocket::serverTimerQueueMonitorThread: pthread_mutex_unlock failed");
                }
                LPSTRUC_MSG_HEADER tmpMsg;
                while (!m_lsIdleList.empty())
                {
                    tmpMsg = m_lsIdleList.front();
                    m_lsIdleList.pop_front();
                    pSocketObj->procPingTimeoutCheking(tmpMsg, curTime); //这里需要检查心跳超时问题
                }
                
            }
            
        }
    }
    usleep(500 * 1000); //休息500ms
    
    return (void *)0;
}

void CSocket::procPingTimeoutCheking(LPSTRUC_MSG_HEADER tmpmsg, time_t curTime)
{
    CMemory &Cmem = CMemory::GetMemory();
    Cmem.FreeMemory(tmpmsg);
    return;
}