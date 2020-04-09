#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h> //sock_addr_in
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include "ngx_c_crc32.h"
#define PORT 80
#define MAX_LINE 2048

//结构定义------------------------------------
#pragma pack (1) //对齐方式,1字节对齐 
//一些和网络通讯相关的结构放在这里
typedef struct _COMM_PKG_HEADER
{
	unsigned short pkgLen;    //报文总长度【包头+包体】--2字节，2字节可以表示的最大数字为6万多，我们定义_PKG_MAX_LENGTH 30000，所以用pkgLen足够保存下
	unsigned short msgCode;   //消息类型代码--2字节，用于区别每个不同的命令【不同的消息】
	int            crc32;     //CRC32效验--4字节，为了防止收发数据中出现收到内容和发送内容不一致的情况，引入这个字段做一个基本的校验用	
}COMM_PKG_HEADER, *LPCOMM_PKG_HEADER;

typedef struct _STRUCT_REGISTER
{
	int           iType;          //类型
	char          username[56];   //用户名 
	char          password[40];   //密码

}STRUCT_REGISTER, *LPSTRUCT_REGISTER;


typedef struct _STRUCT_LOGIN
{
	char          username[56];   //用户名 
	char          password[40];   //密码

}STRUCT_LOGIN, *LPSTRUCT_LOGIN;
#pragma pack()

int  g_iLenPkgHeader = sizeof(COMM_PKG_HEADER);
void ReceiveData(int sockFd);
int recvCommonData(int sockFd, char *pRecvBuf);

int recvCommonData(int sockFd, char *pRecvBuf)
{
    int bytes;
    char *pTmpBuf;
    int rbytes = 0; //收到的总字节数
    int allowbytes; //总共允许收的数据

    pTmpBuf = pRecvBuf;
    allowbytes = g_iLenPkgHeader;

    while (allowbytes >0)
    {

        bytes = recv(sockFd, pTmpBuf, allowbytes, 0);
        if (bytes == -1 || bytes == 0)
        {
            printf("receve data failed!\n");
            return -1;
        }
        //没收完全
        allowbytes -= bytes;
        pTmpBuf += bytes;
        rbytes += bytes;
    }
    //包头收完毕
    LPCOMM_PKG_HEADER pPkgHeader;
    pPkgHeader = (LPCOMM_PKG_HEADER)pRecvBuf;
    unsigned short iLen = ntohs(pPkgHeader->pkgLen);
    if (iLen == g_iLenPkgHeader)
    {
        printf("数据包只有包头,接收完毕\n");
        return iLen;
    }
    allowbytes = iLen - g_iLenPkgHeader;
    pTmpBuf = pRecvBuf + g_iLenPkgHeader;
    while (allowbytes > 0)
    {
        bytes = recv(sockFd, pTmpBuf, allowbytes, 0);
        if (bytes == -1 || bytes == 0)
        {
            printf("receve data2 failed!\n");
            return -1;
        }
        //没收完全
        allowbytes -= bytes;
        pTmpBuf += bytes;
        rbytes += bytes;
    }
    return rbytes;
    
    
}

int sendData(int sockfd, char *p_sendBuf, int bufLen)
{
    int usend = bufLen; //要发送的数据
    int uwrote = 0; //已发送的数据
    int tmp_sret;
    while (uwrote < usend)
    {
        /*
            ssize_t send (int s,const void *msg,size_t len,int flags);
            参数说明
            第一个参数指定发送端套接字描述符；
            第二个参数指明一个存放应用程式要发送数据的缓冲区；
            第三个参数指明实际要发送的数据的字符数；
            第四个参数一般置0。
            返回值:
            成功则返回实际传送出去的字符数，失败返回－1，错误原因存于errno 中
            错误代码:
            EBADF 参数 s 非法的 socket 处理代码。
            EFAULT 参数中有一指针指向无法存取的内存空间。
            WNOTSOCK 参数 s 为一文件描述词，非 socket。
            EINTR 被信号所中断。
            EAGAIN 此动作会令进程阻断，但参数 s 的 socket 为不可阻断的。
            ENOBUFS 系统的缓冲内存不足。
            EINVAL 传给系统调用的参数不正确。
        */
        tmp_sret = send(sockfd, p_sendBuf + uwrote, usend - uwrote, 0);
        if(tmp_sret == -1)
        {
            printf("send error! errno is %s\n", strerror(errno));
            return -1;
        }
        uwrote += tmp_sret;
    }
    return uwrote;
    
}

int ConnectServer()
{
    int sockFd;
    struct sockaddr_in clientaddr;
    char IpAddr[20] = "192.168.0.112";
    //(1) create socket
    if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket error!\n");
        exit(1);
    }
    //(2)设置连接服务器地址结构
    memset(&clientaddr, 0, sizeof(clientaddr));
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_port = htons(PORT);
    //clientaddr.sin_addr.s_addr = inet_addr("192.168.0.112");
    /*
        int inet_pton(int af, const char *src, void *dst);
        描述:将"点分十进制"->"二进制整数". 这个函数转换字符串到网络地址
        参数:
        int af:地址族(AF_INT,AF_INET6)
        char *src:src为指向字符型的地址，即ASCII的地址的首地址（ddd.ddd.ddd.ddd格式的），函数将该地址转换为in_addr的结构体，并复制在*dst中。
        void *dst:转换后的数据
    */
    if(inet_pton(AF_INET, IpAddr, &clientaddr.sin_addr) < 0)
    {
        printf("inet_pton error\n");
        exit(1);
    }
    //(3)发送连接服务器请求
    if (connect(sockFd, (struct sockaddr *)&clientaddr, sizeof(clientaddr)) < 0)
    {
        perror("connect error\n");
        exit(1);
    }
    printf("Connected the server\n");
    //(4) 设置收发数据超时时间
    //注意:以下方法在Linux中无效,试验过程中,总是返回错误,经查阅资料,需要由后面的方式才可以
    /*
        在send(),recv()过程中有时由于网络状况等原因，发收不能预期进行,而设置收发时限：
        int nNetTimeout=1000;//1秒
        //发送时限
        setsockopt(socket，SOL_S0CKET, SO_SNDTIMEO, (char *)&nNetTimeout,sizeof(int));
        //接收时限
        setsockopt(socket，SOL_S0CKET, SO_RCVTIMEO, (char *)&nNetTimeout,sizeof(int));
        这样做在Linux环境下是不会产生效果的，须如下定义：struct timeval timeout = {3,0}; 
        //设置发送超时
        setsockopt(socket，SOL_SOCKET,SO_SNDTIMEO，(char *)&timeout,sizeof(struct timeval));
        //设置接收超时
        setsockopt(socket，SOL_SOCKET,SO_RCVTIMEO，(char *)&timeout,sizeof(struct timeval));
        有两点注意就是：
        1）recv ()的第四个参数需为MSG_WAITALL，在阻塞模式下不等到指定数目的数据不会返回，除非超时时间到。还要注意的是只要设置了接收超时，在没有MSG_WAITALL时也是有效的。说到底超时就是不让你的程序老在那儿等，到一定时间进行一次返回而已。
        2）即使等待超时时间值未到，但对方已经关闭了socket， 则此时recv()会立即返回，并收到多少数据返回多少数据。
        补充：
        同样可以设置连接(connect)超时：即通过SO_SNDTIMO套节字参数让超时操作跳过select。
        原因是：Linux内核源码中connect的超时参数和SO_SNDTIMO操作的参数一致。
        因此，在linux平台下，可以通过connect之前设置SO_SNDTIMO来达到控制连接超时的目的。
    */
    #if 0
    int iSendRecvTimeout = 5000;
    if (setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&iSendRecvTimeout, sizeof(iSendRecvTimeout)) == -1)
    {
        printf("set receive timeout failed!\n");
        close(sockFd);
        return -1;
    }
    if (setsockopt(sockFd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&iSendRecvTimeout, sizeof(iSendRecvTimeout)) == -1)
    {
        printf("set send timeout failed!\n");
        close(sockFd);
        return -1;
    }
    #endif

    struct timeval timeout = {5,0}; 
    if (setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(struct timeval)) == -1)
    {
        printf("set receive timeout failed!\n");
        close(sockFd);
        return -1;
    }
    if (setsockopt(sockFd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(struct timeval)) == -1)

    {
        printf("set send timeout failed!\n");
        close(sockFd);
        return -1;
    }
    return sockFd;

}

void collectData(int sockfd)
{
    //(1)组消息内容
    CCRC32 *pCRC32 = CCRC32::GetInstance();
    char* p_sendBuf = (char*)malloc(sizeof(STRUCT_REGISTER) + g_iLenPkgHeader); //先测试发注册信息
    LPCOMM_PKG_HEADER pinfohead;
    pinfohead = (LPCOMM_PKG_HEADER)p_sendBuf;
    pinfohead->msgCode = 5;
    pinfohead->msgCode = htons(pinfohead->msgCode);
    pinfohead->pkgLen = htons(g_iLenPkgHeader + sizeof(STRUCT_REGISTER));
    LPSTRUCT_REGISTER pstruSendStruct = (LPSTRUCT_REGISTER)(p_sendBuf + g_iLenPkgHeader);
    pstruSendStruct->iType = htonl(100);
    strcpy(pstruSendStruct->username, "admin");

    //获取CRC
    pinfohead->crc32 = pCRC32->Get_CRC((unsigned char *)pstruSendStruct, sizeof(STRUCT_REGISTER));
    pinfohead->crc32 = htonl(pinfohead->crc32);

    //(2)发消息
    if (sendData(sockfd, p_sendBuf, g_iLenPkgHeader + sizeof(STRUCT_REGISTER)) < 0)
    {
        printf("send data error\n");
        free(p_sendBuf);
        return;
    }
    free(p_sendBuf);
    printf("Send data success!\n");
    ReceiveData(sockfd);
    return;
}
//接收数据函数
void ReceiveData(int sockFd)
{
    char recvBuf[100000] = {0};
    int rec = recvCommonData(sockFd, recvBuf);
    if (rec == -1)
    {
        printf("receive data failed!\n");
        //close(sockFd);
        return;
    }
    printf("received data success, data length is %d\n", rec);
    
}
//发送心跳包函数
void heartBeat(int sockFd)
{
    CCRC32 *p_crc32 = CCRC32::GetInstance();

	char *p_sendbuf = (char *)new char[g_iLenPkgHeader]; //没有包体

	LPCOMM_PKG_HEADER         pinfohead;
	pinfohead = (LPCOMM_PKG_HEADER)p_sendbuf;
	pinfohead->msgCode = 0;
	pinfohead->msgCode = htons(pinfohead->msgCode);  //这没啥 用
	pinfohead->pkgLen = htons(g_iLenPkgHeader);

	//crc值需要最后算的
	pinfohead->crc32 = 0;
	pinfohead->crc32 = htonl(pinfohead->crc32); //针对四字节数字，主机序转网络序

    if (sendData(sockFd, p_sendbuf, g_iLenPkgHeader) == -1)
    {
        printf("send heartbeat package failed\n");
        delete[] p_sendbuf;
        return;
    }
    printf("Send heartbeat package success!\n");

    char recvBuf[10000] = {0};
    int rec = recvCommonData(sockFd, recvBuf);
    if (rec == -1)
    {
        printf("receive heartbeat failed\n");
        //close(sockFd);
        delete[] p_sendbuf;
        return;
    }
    delete[] p_sendbuf;
    printf("receive heartbeat success!\n");

    return;
    
}
int main()
{

    //忽略信号
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    sigprocmask(SIG_BLOCK, &set, NULL);
    int cmd;
    int sockfd;
    printf("Please make order of the process: 1: connect server\n 2: sendData\n 3: ReceiveData\n 4: send heartBeat package\n");
    while (scanf("%d", &cmd))
    {
        //printf("Please make order of the process\n");
        switch (cmd)
        {
        case 1:
            sockfd = ConnectServer();
            break;
        case 2:
            collectData(sockfd);
            break;
        case 3:
            ReceiveData(sockfd);
            break;
        case 4:
            heartBeat(sockfd);
            break;
        // default:
        //     break;
        }
    }
    return 0;
}

