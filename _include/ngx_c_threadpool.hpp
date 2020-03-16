#ifndef __NGX_C_THREADPOOL_HPP__

#define __NGX_C_THREADPOOL_HPP__
#include <vector>
#include <pthread.h>
#include <atomic>
#include <string.h>
#include <iostream>
class ngx_c_threadpool
{
public:
    ngx_c_threadpool(/* args */);
    ~ngx_c_threadpool();
public:
    bool createThread(int threadNum); //创建该线程池中的所有线程
    void stopAll(); //使线程池中的所有线程退出
    void Call(); //来任务了,调用一个线程池中的线程工作
    void inMsgRecvQueueAndSignal(char *buf); //收到一个完整信息后,入消息队列,并触发线程池中的线程来处理该消息
private:
    /*
        在C++的类中，普通成员函数不能作为pthread_create的线程函数，如果要作为pthread_create中的线程函数，必须是static!
        why?
        当把线程函数封装在类中，this指针会作为默认的参数被传进函数中，从而和线程函数参数(void*)不能匹配，不能通过编译.
        在C语言中，我们使用pthread_create创建线程，线程函数是一个全局函数，所以在C++中，创建线程时，也应该使用一个全局函数.
        将线程函数作为静态函数,因为在C++中静态函数没有this指针,(即在内存中静态函数与普通的全局函数没有什么区别)
        当线程函数要访问私有变量的时候,需要将this指针作为参数传递给静态函数,这样可以通过该this指针访问所有的私有变量,但是假如还需要向
        静态函数中传递自己需要的参数的时候,就需要将this指针和需要的参数封装成一个结构体一起传递给静态函数.ThreadItem就是用这个方法.             
       

    */
    static void *ThreadFunc(void *threadData); //新线程的线程回调函数
    void clearMsgRecvQueue(); //清理接收消息队列
    
private:
    struct ThreadItem
    {
        pthread_t _handle; //线程句柄
        ngx_c_threadpool *_pThis; //记录线程的指针
        bool isRunning; //标记是否正式启动起来,启动起来后,才允许调用stopAll()来释放
        ThreadItem(ngx_c_threadpool *pthis):_pThis(pthis),isRunning(false){}
        ~ThreadItem(){}
    };
private:
    static pthread_mutex_t m_pthreadMutex; //县城同步互斥量
    static pthread_cond_t m_pthreadCond; //线程同步条件变量
    static bool m_shutdown; //线程退出标志,false不退出,true退出

    int m_iThreadNum; //要创建的线程数量
    std::atomic<int> m_iRunningThreadNum;//运行中的线程数
    time_t m_iLastEmgTime; ////上次发生线程不够用【紧急事件】的时间,防止日志报的太频繁
    std::vector<ThreadItem*> m_threadVector; //线程容器
    //接收消息队列
    std::list<char*> m_MsgRecvQueue; //接收数据消息队列
    int m_iRecvMsgQueueCount; //接收消息队列大小
    
};

#endif