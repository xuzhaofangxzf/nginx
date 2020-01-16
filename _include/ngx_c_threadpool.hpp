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
    void Call(int irmqc); //来任务了,调用一个线程池中的线程工作

private:
    //在C++的类中，普通成员函数不能作为pthread_create的线程函数，如果要作为pthread_create中的线程函数，必须是static
    static void *ThreadFunc(void *threadData); //新线程的线程回调函数
    
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
    
};

#endif