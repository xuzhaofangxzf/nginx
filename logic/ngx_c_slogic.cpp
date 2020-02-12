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
/**********************************************************
 * 函数名称: CLogicSocket::_handleRegister
 * 函数描述: 用户注册入口函数
 * 函数参数:
 *      lpngx_connection_t pConn:消息头
 *      LPSTRUC_MSG_HEADER pMsgHeader: 消息类型
 *      char *pPkgBody
 *      unsigned short iBodyLength
 * 返回值:
 *    bool
***********************************************************/
bool CLogicSocket::_handleRegister(lpngx_connection_t pConn, LPSTRUC_MSG_HEADER pMsgHeader, char *pPkgBody, unsigned short iBodyLength)
{
    ngx_log_stderr(0, "excute the function CLogicSocket::_handleRegister!");
    //(1)判断包体的合法性
    if (pPkgBody == NULL) //具体看客户端服务器约定，如果约定这个命令[msgCode]必须带包体，那么如果不带包体，就认为是恶意包，直接不处理
    {
        return false;
    }
    int iRecvLen = sizeof(STRUCT_REGISTER);
    if (iRecvLen != iBodyLength)
    {
        return false;
    }
    /*
        (2)对于同一个用户，可能同时会发送过来多个请求，造成多个线程同时为该用户服务，比如以网游为例，用户要在商店中买A物品，又要买B物品
        而用户的钱只够买A或者B，不够同时买A和B。
        如果用户发送购买命令过来买了一次A，又买了一次B，如果是两个线程执行同一个用户的这两次不同的购买命令，很可能造成这个用户同时购买成功了A和B。
        所以，为了稳妥起见，针对某个用户的命令，我们一般都要互斥
    */
   CMutexLock lock(&pConn->logicProcMutex); //凡是和本用户有关的访问都要互斥
   //(3)获取发送过来的整个数据
   LPSTRUCT_REGISTER pRecvinfo = (LPSTRUCT_REGISTER)pPkgBody;
   pRecvinfo->iType = ntohl(pRecvinfo->iType);
   //防止客户端发送活来畸形包，导致服务器直接使用这个数据出现错误
   pRecvinfo->username[sizeof(pRecvinfo->username) - 1] = '\0'; 
   pRecvinfo->password[sizeof(pRecvinfo->password) - 1] = '\0';
   //(4)这里可能还要考虑，根据业务逻辑，进一步判断数据的合法性
   //当前该玩家的状态是否适合收到这个数据(比如，用户没有登陆，就不能购买物品等)。。。。
   //(5)给客户端返回数据时，一般也是返回一个结构，这个结构内容需要由客户端/服务器协商，这里返回同样的结构
   LPCOMM_PKG_HEADER pPkgHeader;
   CMemory &Cmem = CMemory::GetMemory();
   CCRC32 *pCRC32 = CCRC32::GetInstance();
   int iSendLen = sizeof(STRUCT_REGISTER);
   //(a)分配内存
   char *pSendBuf = (char *)Cmem.AllocMemory(m_iLenMsgHeader + m_iLenPkgHeader + iSendLen, false);
   //(b)填充消息头
   memcpy(pSendBuf, pMsgHeader, m_iLenMsgHeader);
   //(c)填充包头
   pPkgHeader = (LPCOMM_PKG_HEADER)(pSendBuf + m_iLenMsgHeader);
   pPkgHeader->mgsCode = _CMD_REGISTER;
   pPkgHeader->mgsCode = htons(pPkgHeader->mgsCode);
   pPkgHeader->pkgLen = htons(m_iLenPkgHeader + iSendLen);
   //(d)填充包体
   LPSTRUCT_REGISTER pSendInfo = (LPSTRUCT_REGISTER)(pSendBuf + m_iLenMsgHeader + m_iLenPkgHeader);

   //(e)包体内容确定好后，计算包体的CRC32的值
   pPkgHeader->crc32 = pCRC32->Get_CRC((unsigned char *)pSendInfo, iSendLen);
   pPkgHeader->crc32 = htonl(pPkgHeader->crc32);
   //(f)发送数据包
   msgSend(pSendBuf);
   return true;

}
/**********************************************************
 * 函数名称: CLogicSocket::_handleLogin
 * 函数描述: 用户登陆入口函数
 * 函数参数:
 *      lpngx_connection_t pConn:消息头
 *      LPSTRUC_MSG_HEADER pMsgHeader: 消息类型
 *      char *pPkgBody
 *      unsigned short iBodyLength
 * 返回值:
 *    bool
***********************************************************/
bool CLogicSocket::_handleLogin(lpngx_connection_t pConn, LPSTRUC_MSG_HEADER pMsgHeader, char *pPkgBody, unsigned short iBodyLength)
{
    ngx_log_stderr(0, "excute function CLogicSocket::_handleLogin()");
    return true;
}
/**********************************************************
 * 函数名称: CLogicSocket::_handlePing
 * 函数描述: 接收并处理客户端发送过来的ping包
 * 函数参数:
 *      lpngx_connection_t pConn:消息头
 *      LPSTRUC_MSG_HEADER pMsgHeader: 消息类型
 *      char *pPkgBody
 *      unsigned short iBodyLength
 * 返回值:
 *    bool
***********************************************************/
bool CLogicSocket::_handlePing(lpngx_connection_t pConn, LPSTRUC_MSG_HEADER pMsgHeader, char *pPkgBody, unsigned short iBodyLength)
{
    //心跳包没有包体
    if (iBodyLength != 0)
    {
        return false;
    }
    CMutexLock lock(&pConn->logicProcMutex);
    pConn->lastPingTime = time(NULL);
    sendNoBodyPkgToClient(pMsgHeader, _CMD_PING);
    ngx_log_stderr(0, "send back ping package success!");
    return true;
}
/**********************************************************
 * 函数名称: CLogicSocket::procPingTimeoutChecking
 * 函数描述: 心跳包检测时间到,该去检测心跳包是否超时的事宜,本函数是类函数，实现具体的判断动作
 * 函数参数:
 *      lpngx_connection_t pConn:消息头
 *      LPSTRUC_MSG_HEADER pMsgHeader: 消息类型
 *      char *pPkgBody
 *      unsigned short iBodyLength
 * 返回值:
 *    bool
***********************************************************/
void CLogicSocket::procPingTimeoutChecking(LPSTRUC_MSG_HEADER tmpMsg, time_t curTime)
{
    CMemory &Cmem = CMemory::GetMemory();
    //连接没有断开
    if (tmpMsg->iCurrsequence ==  tmpMsg->pConn->iCurrsequence)
    {
        lpngx_connection_t pConn = tmpMsg->pConn;
        if (m_ifTimeOutKick == 1)
        {
            zdCloseSocketProc(pConn);
        }
        //超时剔除的判断标准:每次检查的时间间隔*3，超过这个时间没发送心跳包
        else if((curTime - pConn->lastPingTime) > (m_iWaitTime * 3 + 10))
        {
            //如果此时该用户正好断线，切这个socket文件描述符正好立即被后续的连接复用，此时可能会无端关闭正常的socket
            //这种情况也直接关闭连接
            zdCloseSocketProc(pConn);
        }
        Cmem.FreeMemory(tmpMsg);
    }
    else
    {
        //连接已经断了
        Cmem.FreeMemory(tmpMsg);
    }
    return;
    
}