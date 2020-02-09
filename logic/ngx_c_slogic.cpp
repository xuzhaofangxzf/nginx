#include "ngx_c_slogic.hpp"
#include "ngx_c_crc32.hpp"
#include "ngx_func.hpp"

typedef bool (CLogicSocket::*handler)(  lpngx_connection_t pConn,       //连接池中的连接
                                        LPSTRUC_MSG_HEADER pMsgHeader,  //消息头指针
                                        char *pPkgBody,                 //包体指针
                                        unsigned short iBodyLength);    //包体长度

static const handler statusHandler[] = 
{
    &CLogicSocket::_handlePing,     //(0):心跳包的实现
    NULL,   //保留
    NULL,
    NULL,
    NULL,

    //开始处理具体的业务逻辑
    &CLogicSocket::_handleRegister, //注册功能
    &CLogicSocket::_handleLogin     //登录功能
};

#define AUTH_TOTAL_COMMANDS sizeof(statusHandler)/sizeof(handler) //整个命令有多少个

//构造函数
CLogicSocket::CLogicSocket()
{

}
//析构函数
CLogicSocket::~CLogicSocket()
{

}

/**********************************************************
 * 函数名称: CLogicSocket::initialize
 * 函数描述: 初始化函数
 * 函数参数:
 *      空
 * 返回值:
 *    bool
***********************************************************/
bool CLogicSocket::initialize()
{
    return CSocket::Initialize();
}
/**********************************************************
 * 函数名称: CLogicSocket::threadRecvProcFunc
 * 函数描述: 处理收到的数据包,由线程池来调用本函数,本函数是一个单独的线程
 * 函数参数:
 *      char * pMsgBuf, 消息头+包头+包体
 * 返回值:
 *    void
***********************************************************/
void CLogicSocket::threadRecvProcFunc(char* pMsgBuf)
{
    LPSTRUC_MSG_HEADER pMsgHeader = (LPSTRUC_MSG_HEADER)pMsgBuf; //消息头
    LPCOMM_PKG_HEADER pPkgHeader = (LPCOMM_PKG_HEADER)(pMsgBuf + m_iLenMsgHeader);
    void *pPkgBody; //指向包体的指针
    unsigned short pkgLen = ntohs(pPkgHeader->pkgLen);
    if (m_iLenPkgHeader == pkgLen) //只有包头
    {
        if (pPkgHeader->crc32 != 0)
        {
            return; //crc错,直接丢弃
        }
        pPkgBody = NULL;
    }
    else
    {
        pPkgHeader->crc32 = ntohl(pPkgHeader->crc32);
        pPkgBody = (void *)(pMsgBuf + m_iLenMsgHeader + m_iLenPkgHeader);
        //计算CRC值判断包的完整性
        int calcCrc = CCRC32::GetInstance()->Get_CRC((unsigned char *)pPkgBody, pkgLen - m_iLenPkgHeader); //计算纯包体的CRC值
        if (calcCrc != pPkgHeader->crc32)
        {
            ngx_log_stderr(0, "CLogicSocket::threadRecvProcFunc: CRC not equal, drop the data");
            return;
        }
        
    }
    unsigned short iMsgCode = ntohs(pPkgHeader->mgsCode);
    lpngx_connection_t pConn = pMsgHeader->pConn;
    //(1)如果从收到客户端发送来的包,到服务器调用一个线程池中的线程处理该包的过程中,客户端断开了,这种收到的数据包就算是过期了,丢弃不处理
    if (pConn->iCurrsequence != pMsgHeader->iCurrsequence)
    {
        return;
    }
    //(2)判断消息码是正确的,防止客户端恶意侵害服务器,发送一个不在我们服务器处理范围内的消息码
    if (iMsgCode >= AUTH_TOTAL_COMMANDS)
    {
        ngx_log_stderr(0, "CLogicSocket::threadRecvProcFunc: unkown message code(%d)", iMsgCode);
        return;
    }
    //(3)正常处理
    if (statusHandler[iMsgCode] == NULL)
    {
        ngx_log_stderr(0, "CLogicSocket::threadRecvProcFunc: cannot find corresponding function of the message code");
        return;
    }
    (this->*statusHandler[iMsgCode])(pConn, pMsgHeader, (char *)pPkgBody, pkgLen - m_iLenPkgHeader);
    return;
    
    
    
    
}
//通用收发数据相关函数
/**********************************************************
 * 函数名称: CLogicSocket::sendNoBodyPkgToClient
 * 函数描述: 发送没有包体的数据包给客户端, 发送心跳包的时候使用
 * 函数参数:
 *      LPSTRUC_MSG_HEADER pMsgHeader:消息头
 *      unsigned short iMsgCode: 消息类型
 * 返回值:
 *    void
***********************************************************/
void CLogicSocket::sendNoBodyPkgToClient(LPSTRUC_MSG_HEADER pMsgHeader, unsigned short iMsgCode)
{
    CMemory& Cmem = CMemory::GetMemory();
    char *pSendBuf = (char *)Cmem.AllocMemory(m_iLenMsgHeader + m_iLenPkgHeader, false); //消息头+包头的长度
    char *ptmpBuf = pSendBuf;

    memcpy(ptmpBuf, pMsgHeader, m_iLenMsgHeader);
    ptmpBuf += m_iLenMsgHeader;

    LPCOMM_PKG_HEADER pPkgHeader = (LPCOMM_PKG_HEADER)ptmpBuf;
    pPkgHeader->mgsCode = htons(iMsgCode);
    pPkgHeader->pkgLen = htons(m_iLenPkgHeader);
    pPkgHeader->crc32 = 0;
    msgSend(pSendBuf);
    return;


}
//各种业务逻辑相关函数
bool CLogicSocket::_handleRegister(lpngx_connection_t pConn, LPSTRUC_MSG_HEADER pMsgHeader, char *pPkgBody, unsigned short iBodyLength)
{
    ngx_log_stderr(0, "excute the function CLogicSocket::_handleRegister!");
    //(1)判断包体的合法性
    if (pPkgBody == NULL) //具体看客户端服务器约定，如果约定这个命令[msgCode]必须带包体，那么如果不带包体，就认为是恶意包，直接不处理
    {
        return false;
    }
    int iRecvLen = sizeof()
}
bool _handleLogin(lpngx_connection_t pConn, LPSTRUC_MSG_HEADER pMsgHeader, char *pPkgBody, unsigned short iBodyLength);
bool _handlePing(lpngx_connection_t pConn, LPSTRUC_MSG_HEADER pMsgHeader, char *pPkgBody, unsigned short iBodyLength);

//心跳包检测时间到,该去检测心跳包是否超市的事宜,本函数是把内存释放,子类应该重新实现该函数以实现具体的判断动作
virtual void procPingTimeoutChecking(LPSTRUC_MSG_HEADER tmpMsg, time_t curTime);