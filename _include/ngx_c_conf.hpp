#ifndef __NGX_C_CONF_H__
#define __NGX_C_CONF_H__

#include <vector>
#include "ngx_global.hpp"


class CConfig
{
private:
    CConfig(){}
    class destroy
    {
        public:
        ~destroy()
        {
            destroyInstance();
        }
        
    };
    static CConfig* m_pInstance;
    static destroy dtroy;
public:
    static CConfig* getInstance()
    {
        return m_pInstance;
    }
    static void destroyInstance()
    {
        if(m_pInstance != nullptr)
        {
            delete m_pInstance;
            m_pInstance = nullptr;

        }
        return;
    }
public:
    ~CConfig();
    bool loadConf(const char* pconfName);
    const char* getString(const char* p_itemName);
    int getIntDefault(const char* p_itemName, const int def = 0);
    //for test
    void printItem();
public:
    std::vector<LPCConfItem> m_configItemList; //存储配置信息列表
};


#endif