#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ngx_func.hpp"
#include "ngx_c_conf.hpp"

//初始化
CConfig *CConfig::m_pInstance = new CConfig();
CConfig::destroy CConfig::dtroy;

CConfig::~CConfig()
{
    std::vector<LPCConfItem>::iterator pos;
    for (pos = m_configItemList.begin(); pos != m_configItemList.end(); pos++)
    {
        delete (*pos);
    }
    m_configItemList.clear();
}

//装载配置文件
bool CConfig::loadConf(const char *pconfName)
{
    FILE *fp;
    fp = fopen(pconfName, "r");
    if (fp == NULL)
    {
        return false;
    }

    char linebuf[501];

    while (!feof(fp))
    {
        if (fgets(linebuf, 500, fp) == NULL) //从文件中读数据，每次读一行，一行最多不要超过500个字符
        {
            continue;
        }
        if (linebuf[0] == 0)
        {
            continue;
        }

        if (*linebuf == ';' || *linebuf == ' ' || *linebuf == '#' || *linebuf == '\t' || *linebuf == '\n')
        {
            continue;
        }
        //屁股后边若有换行"10"，回车"13"，空格"32"等都截取掉
        while (strlen(linebuf) > 0)
        {
            if ((linebuf[strlen(linebuf) - 1] == 10) || (linebuf[strlen(linebuf) - 1] == 13) || (linebuf[strlen(linebuf) - 1] == 32))
            {
                linebuf[strlen(linebuf) - 1] = 0;
            }
            else
            {
                break;
            }
        }
        //相当与这一行是空行，仅仅一个换行符
        if (linebuf[0] == 0)
        {
            continue;
        }
        if (*linebuf == '[') //开头是[也不作处理
        {
            continue;
        }
        //strchr函数功能为在一个串中查找给定字符的第一个匹配之处，返回第一次找到字符的位置
        //listenPort = 5678可以通过
        char *ptmp = strchr(linebuf, '='); //返回“=”第一次出现的位置
        if (ptmp != NULL)
        {
            LPCConfItem p_confitem = new CConfItem();//记的要释放资源
            memset(p_confitem, 0, sizeof(CConfItem));
            strncpy(p_confitem->ItemName, linebuf, (int)(ptmp - linebuf)); //拷贝等号前面的
            //后面的可能有两种情况，一种是有注释#，另一种是没有注释的
            char *ptmplast = strchr(linebuf, '#');
            if (ptmplast != NULL) //有#注释项
            {

                strncpy(p_confitem->ItemContent, ptmp + 1, ptmplast - ptmp - 1);
            }
            else
            {
                strcpy(p_confitem->ItemContent, ptmp + 1);  //将“=”右侧拷贝
            }


            printf("before: ItemName [%s]= p_confitem->ItemContent [%s]\n", p_confitem->ItemName, p_confitem->ItemContent);
            //去除两边的空格
            Ltrim(p_confitem->ItemName);
            Rtrim(p_confitem->ItemName);
            Ltrim(p_confitem->ItemContent);
            Rtrim(p_confitem->ItemContent);
            //先打印下配置的结果

            printf("after: ItemName [%s]= p_confitem->ItemContent [%s]\n", p_confitem->ItemName, p_confitem->ItemContent);
            m_configItemList.push_back(p_confitem);
        }
    }
    fclose(fp);
    return true;
}

//根据ItemName获取配置信息字符串，不修改不用互斥.字符川类型的版本
const char *CConfig::getString(const char *p_itemname)
{
    std::vector<LPCConfItem>::iterator pos;
    for (pos = m_configItemList.begin(); pos != m_configItemList.end(); pos++)
    {
        
        if (strcasecmp((*pos)->ItemName, p_itemname) == 0)
        {
            return (*pos)->ItemContent;
        }
    }
    return NULL; //没有找到对应的条目
}
void CConfig::printItem()
{
    std::vector<LPCConfItem>::iterator pos;
    for (pos = m_configItemList.begin(); pos != m_configItemList.end(); pos++)
    {
        printf("item%s = %s\n", (*pos)->ItemName, (*pos)->ItemContent);
    }
    return;
}
//根据itemName获取配置信息，int类型的版本。如果没找到，则def为默认值
int CConfig::getIntDefault(const char *p_itemName, const int def)
{
    std::vector<LPCConfItem>::iterator pos;
    for (pos = m_configItemList.begin(); pos != m_configItemList.end(); pos++)
    {
        if (strcasecmp((*pos)->ItemName, p_itemName) == 0)
        {
            return atoi((*pos)->ItemContent);
        }
    }
    return def;
}