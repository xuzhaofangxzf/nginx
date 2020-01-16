#ifndef __NGX_LOCKMUTEX_HPP__
#define __NGX_LOCKMUTEX_HPP__
#include <pthread.h>

class CMutexLock
{
private:
    pthread_mutex_t *m_pMutex;
public:
    CMutexLock(pthread_mutex_t *pMutex)
    {
        m_pMutex = pMutex;
        pthread_mutex_lock(m_pMutex); //加锁互斥量
    }
    ~CMutexLock()
    {
        pthread_mutex_unlock(m_pMutex);//解锁
    }
};

// CMutexLock::CMutexLock(pthread_mutex_t *pMutex)
// {
//     m_pMutex = pMutex;
//     pthread_mutex_lock(m_pMutex); //加锁互斥量
// }

// CMutexLock::~CMutexLock()
// {
//     pthread_mutex_unlock(m_pMutex);//解锁
// }


#endif