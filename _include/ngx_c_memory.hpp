#ifndef __NGX_MEMORY_H__
#define __NGX_MEMORY_H__

#include <stddef.h> //NULL
//局部静态变量的懒汉单例模式
class CMemory
{
private:

    CMemory(/* args */){}
public:
    ~CMemory(){}


public:
    static CMemory& GetMemory()
    {
        static CMemory m_Memery;
        return m_Memery;
    }
    
private:
    static CMemory m_Memery;


public:
    void * AllocMemory(int memCount, bool ifMemset);
    void FreeMemory(void * pMem);
};

#endif