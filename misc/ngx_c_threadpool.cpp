#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "ngx_global.hpp"
#include "ngx_func.hpp"
#include "ngx_c_threadpool.hpp"
#include "ngx_c_memory.hpp"
#include "ngx_macro.hpp"

//静态成员初始化
pthread_mutex_t ngx_c_threadpool::m_pthreadMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ngx_c_threadpool::m_pthreadCond = PTHREAD_COND_INITIALIZER;
bool ngx_c_threadpool::m_shutdown = false;

ngx_c_threadpool::ngx_c_threadpool()
{
    m_iRunningThreadNum = 0;
    m_iLastEmgTime = 0;
}
ngx_c_threadpool::~ngx_c_threadpool()
{
 //资源释放在StopAll()里统一进行
}
/**********************************************************
 * 函数名称: ngx_c_threadpool::createThread
 * 函数描述: 创建线程
 * 函数参数:
 *      int threadNum 线程个数
 * 返回值:
 *    bool true 成功, false失败
***********************************************************/
bool ngx_c_threadpool::createThread(int threadNum)
{
    ThreadItem *pNew;
    int err;
    m_iThreadNum = threadNum;
    for (int i = 0; i < m_iThreadNum; i++)
    {
        //创建一个新县城,并放入到容器中
        m_threadVector.push_back(pNew = new ThreadItem(this));
        /*
            int pthread_create(pthread_t *tidp,const pthread_attr_t *attr,(void*)(*start_rtn)(void*),void *arg);
            创建线程（实际上就是确定调用该线程函数的入口点），在线程创建以后，就开始运行相关的线程函数
            参数: 
                pthread_t *tidp 指向线程标识符的指针
                const pthread_attr_t *attr 设置线程属性
                (void*)(*start_rtn)(void*) 线程运行函数的起始地址
                void *arg 运行函数的参数
            返回值:
                若线程创建成功，则返回0。若线程创建失败，则返回出错编号，并且*thread中的内容是未定义的
            注意:
                因为pthread并非Linux系统的默认库，而是POSIX线程库。在Linux中将其作为一个库来使用，因此加上 -lpthread（或-pthread）以显式链接该库。
            函数在执行错误时的错误信息将作为返回值返回，并不修改系统全局变量errno，当然也无法使用perror()打印错误信息
        */
        err = pthread_create(&pNew->_handle, NULL, ThreadFunc, pNew); 
        if (err != 0)
        {
            ngx_log_stderr(err, "ngx_c_threadpool: createPool %d failed, errno is %d", i, err);
            return false;
        }
        else
        {
            ngx_log_stderr(err, "ngx_c_threadpool: createPool %d success, errno is %d", i, err);
        }
    }
    ngx_log_stderr(0, "create threadpoll success");
    //我们必须保证每个线程都启动并运行到pthread_cond_wait()，本函数才返回，只有这样，这几个线程才能进行后续的正常工作
    std::vector<ThreadItem*>::iterator iter;
    iter = m_threadVector.begin();
    while (iter != m_threadVector.end())
    {
        if ((*iter)->isRunning == false)
        {
            usleep(100 * 1000);
            iter = m_threadVector.begin();
        }
        iter++;
    }
    ngx_log_stderr(0, "create threadpoll all success");
    return true;
    
}

/**********************************************************
 * 函数名称: ngx_c_threadpool::ThreadFunc
 * 函数描述: 线程入口函数
 * 函数参数:
 *      void * threadData 
 * 返回值:
 *    void* 
***********************************************************/
void * ngx_c_threadpool::ThreadFunc(void *threadData)
{
    /*
        
    */
    ThreadItem *pThread = static_cast<ThreadItem*>(threadData);
    ngx_c_threadpool *pThreadPoolObj = pThread->_pThis;
    char *jobbuf = NULL;
    CMemory& Cmem = CMemory::GetMemory();
    int err;

    pthread_t tid = pthread_self(); //获取自身线程ID

    while (true)
    {
        //线程用pthread_mutex_lock()函数去锁定指定的mutex变量，若该mutex已经被另外一个线程锁定了，该调用将会阻塞线程直到mutex被解锁。  
        err = pthread_mutex_lock(&m_pthreadMutex);
        if (err != 0)
        {
            ngx_log_stderr(err, "ThreadFunc: pthread_mutex_lock failed: %s", strerror(err));

        }
        //以下这行程序写法技巧十分重要，必须要用while这种写法，
        //因为：pthread_cond_wait()是个值得注意的函数，调用一次pthread_cond_signal()可能会唤醒多个【惊群】【官方描述是 至少一个/pthread_cond_signal 在多处理器上可能同时唤醒多个线程】
        //老师也在《c++入门到精通 c++ 98/11/14/17》里第六章第十三节谈过虚假唤醒，实际上是一个意思；
        //老师也在《c++入门到精通 c++ 98/11/14/17》里第六章第八节谈过条件变量、wait()、notify_one()、notify_all()，其实跟这里的pthread_cond_wait、pthread_cond_signal、pthread_cond_broadcast非常类似
        //pthread_cond_wait()函数，如果只有一条消息 唤醒了两个线程干活，那么其中有一个线程拿不到消息，那如果不用while写，就会出问题，所以被惊醒后必须再次用while拿消息，拿到才走下来；
        while ((pThreadPoolObj->m_MsgRecvQueue.size() == 0) == NULL && m_shutdown == false)
        {
            ngx_log_stderr(err, "ThreadFunc: pthread_cond_wait before tid = %u", tid);
            /*
            对该段代码的解析:
                1. 创建线程的时候,会自动调用ThreadFunc函数,由于创建线程(早于socket)的时候,消息队列中并没有消息,所以所有的线程都会走这里
                2. 执行pthread_cond_wait的时候,线程会阻塞在这里,并自动释放m_pthreadMutex互斥量
                3. 在某个地方调用了pthread_cond_signal(&m_pthreadCond),会唤醒线程,在多处理器上可能会唤醒多个线程(可以测试下)
                4. 如果这个pthread_cond_wait被唤醒(被唤醒后程序执行前提是拿到了锁), pthread_cond_wait()返回时,互斥量自动被锁住
                5. 唤醒后,线程会从pthread_cond_wait执行,如果此时消息队列中有消息,则会往下执行,先释放互斥锁,然后执行业务逻辑
            */
           if (pThread->isRunning == false)
           {
               pThread->isRunning = true;
           }
           //刚开始执行pthread_cond_wait()的时候，会卡在这里，而且m_pthreadMutex会被释放掉；
           pthread_cond_wait(&m_pthreadCond, &m_pthreadMutex);
           ngx_log_stderr(0, "wake up the thread id = %u", tid);
           
        }
        
        if (m_shutdown)
        {
            err = pthread_mutex_unlock(&m_pthreadMutex); //解锁mutex
            break;
        }
        //处理接收到的消息
        jobbuf = pThreadPoolObj->m_MsgRecvQueue.front();
        pThreadPoolObj->m_MsgRecvQueue.pop_front();
        --pThreadPoolObj->m_iRecvMsgQueueCount;

        err = pthread_mutex_unlock(&m_pthreadMutex); //解锁mutex        
        if (err != 0)
        {
            ngx_log_stderr(err, "ThreadFunc: pthread_mutex_unlock failed: %s", strerror(err));
        }
        ++pThreadPoolObj->m_iRunningThreadNum;
        
        g_socket.threadRecvProcFunc(jobbuf);
        
        //先打印处理
        
        ngx_log_stderr(0, "excute start: tid = %ui", tid);
        sleep(5);
        ngx_log_stderr(0, "excute end: tid = %ui", tid);
        Cmem.FreeMemory(jobbuf);
        --pThreadPoolObj->m_iRunningThreadNum;

    }//end while(true)

    return (void*)0;

}

/**********************************************************
 * 函数名称: ngx_c_threadpool::StopAll
 * 函数描述: 停止线程池中的所有线程
 * 函数参数:
 *      空
 * 返回值:
 *    void 
***********************************************************/
void ngx_c_threadpool::stopAll()
{
    //避免重复调用
    if (m_shutdown == true)
    {
        return;
    }
    m_shutdown = true;
    //(1) 唤醒所有线程,一定要在改变条件状态以后再给线程发信号
    int err = pthread_cond_broadcast(&m_pthreadCond);
    if (err != 0)
    {
        ngx_log_stderr(err, "ngx_c_threadpool:stopAll call pthread_cond_broardcast failed! err is %s", strerror(err));
        return;
    }
    // (2)等待线程返回
    std::vector<ThreadItem*>::iterator iter;
    for (iter = m_threadVector.begin(); iter != m_threadVector.end(); iter++)
    {
        pthread_join((*iter)->_handle, NULL);
    }

    //(3) 注销互斥量与条件变量
    pthread_mutex_destroy(&m_pthreadMutex);
    pthread_cond_destroy(&m_pthreadCond);
    
    //(4)释放线程池中的线程
    
    for (iter = m_threadVector.begin(); iter != m_threadVector.end(); iter++)
    {
        if (*iter)
        {
            delete *iter;
        }
        
    }
    m_threadVector.clear();
    ngx_log_stderr(0, "ngx_c_threadpool:stopAll release all the threads success!");
    return;
    
}

/**********************************************************
 * 函数名称: ngx_c_threadpool::Call
 * 函数描述: 调用线程池中的线程执行任务
 * 函数参数:
 *      int irmqc 消息队列中消息的个数
 * 返回值:
 *    void 
***********************************************************/
void ngx_c_threadpool::Call()
{
    int err = pthread_cond_signal(&m_pthreadCond); //唤醒一个等待的线程
    if(err != 0 )
    {
        //这是有问题啊，要打印日志啊
        ngx_log_stderr(err,"CThreadPool::Call()中pthread_cond_signal()失败，返回的错误码为%d!",err);
    }
    //(1) 如果当前的工作线程全部繁忙,则告警
    if (m_iThreadNum == m_iRunningThreadNum)
    {
        time_t curTime = time(NULL);
        if (curTime - m_iLastEmgTime > 10) //10s上报一次
        {
            m_iLastEmgTime = curTime; //更新事件
            ngx_log_stderr(0, "ngx_c_threadpool: all threads are busy, please do something!");
        }
        
    }
    return;
}

/**********************************************************
 * 函数名称: ngx_c_threadpool::inMsgRecvQueueAndSignal
 * 函数描述: 收到一个完整消息后，入消息队列，并触发线程池中线程来处理该消息
 * 函数参数:
 *      char *buf:指向消息的指针
 * 返回值:
 *    void 
***********************************************************/
void ngx_c_threadpool::inMsgRecvQueueAndSignal(char *buf)
{
    int err = pthread_mutex_lock(&m_pthreadMutex);
    if (err != 0)
    {
        ngx_log_stderr(err, "ngx_c_threadpool::inMsgRecvQueueAndSignal pthread_mutex_lock(&m_pthreadMutex) failed");

    }
    m_MsgRecvQueue.push_back(buf);
    ++m_iRecvMsgQueueCount;
    if (err !=0 )
    {
        ngx_log_stderr(err, "ngx_c_threadpool::inMsgRecvQueueAndSignal pthread_mutex_unlock(&m_pthreadMutex) failed");
    }
    
    Call();
    return;
    
}
/**********************************************************
 * 函数名称: ngx_c_threadpool::clearMsgRecvQueue
 * 函数描述: 清理接收消息队列
 * 函数参数:
 *      空
 * 返回值:
 *    void 
***********************************************************/
void ngx_c_threadpool::clearMsgRecvQueue()
{
    char *sTmpMemPoint;
    CMemory& Cmem = CMemory::GetMemory();
    //不需要再互斥了把?
    while (!m_MsgRecvQueue.empty())
    {
        sTmpMemPoint = m_MsgRecvQueue.front();
        m_MsgRecvQueue.pop_front();
        Cmem.FreeMemory(sTmpMemPoint);
    }
}
