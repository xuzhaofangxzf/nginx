
#include <string.h>

#include "ngx_c_memory.hpp"

#include <iostream> //exception
#include <exception>
//用法
//CMemory& Cmem = CMemory::GetMemory();

/**********************************************************
 * 函数名称: CMemory::AllocMemory
 * 函数描述: 申请内存
 * 函数参数:
 *      int memCount 分配的内存字节大小
 *      bool ifMemset 是否需要将申请的内存初始化为0, 为了提高效率,有时候不需要初始化
 * 返回值:
 *    void* 返回的地址
***********************************************************/
void * CMemory::AllocMemory(int memCount, bool ifMemset)
{
    // try
    // {
    //     void *tmpData = (void*)new char[memCount];

    // }
    // catch(const std::exception& e)
    // {
    //     std::cerr << e.what() << '\n';
    // }
    void *tmpData = (void*)new char[memCount];
    if (ifMemset)
    {
        memset(tmpData, 0, memCount);
    }
    return tmpData;
    
    
}


/**********************************************************
 * 函数名称: CMemory::FreeMemory
 * 函数描述: 释放内存
 * 函数参数:
 *      void* pMem 需要释放的内存
 * 返回值:
 *    void
***********************************************************/
void CMemory::FreeMemory(void * pMem)
{
    //delete [] point;  //这么删除编译会出现警告：warning: deleting ‘void*’ is undefined [-Wdelete-incomplete]

    delete[] ((char*)pMem); //new的时候是char *，这里弄回char *，以免出警告
}