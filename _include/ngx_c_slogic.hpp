#ifndef __NGX_C_SLOGIC_H__
#define __NGX_C_SLOGIC_H__


#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include "ngx_c_socket.hpp"
#include "ngx_logiccomm.hpp"


class CLogicSocket : public CSocket
{
private:
    /* data */
public:
    CLogicSocket(/* args */);
    virtual ~CLogicSocket();
    virtual bool initialize();

public:
    //通用收发数据相关函数
    void sendNoBodyPkgToClient(LPSTRUC_MSG_HEADER pMsgHeader, unsigned short iMsgCode);
    //各种业务逻辑相关函数
    bool _handleRegister(lpngx_connection_t pConn, LPSTRUC_MSG_HEADER pMsgHeader, char *pPkgBody, unsigned short iBodyLength);
    bool _handleLogin(lpngx_connection_t pConn, LPSTRUC_MSG_HEADER pMsgHeader, char *pPkgBody, unsigned short iBodyLength);
    bool _handlePing(lpngx_connection_t pConn, LPSTRUC_MSG_HEADER pMsgHeader, char *pPkgBody, unsigned short iBodyLength);

    //心跳包检测时间到,该去检测心跳包是否超市的事宜,本函数是把内存释放,子类应该重新实现该函数以实现具体的判断动作
    virtual void procPingTimeoutChecking(LPSTRUC_MSG_HEADER tmpMsg, time_t curTime);
public:
    virtual void threadRecvProcFunc(char* pMsgBuf);
};

#endif