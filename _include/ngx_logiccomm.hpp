#ifndef __NGX_LOGICCOMM_HPP__
#define __NGX_LOGICCOMM_HPP__

//收发命令宏定义
#define _CMD_START                      0
#define _CMD_PING                       _CMD_START + 0 //ping心跳包
#define _CMD_REGISTER                   _CMD_START + 5
#define _CMD_LOGIN                      _CMD_START + 6

//结构定义
#pragma pack(1)

typedef struct _STRUCT_REGISTER
{
    int iType;
    char username[56];
    char password[40];
}STRUCT_REGISTER, *LPSTRUCT_REGISTER;

typedef struct _STRUCT_LOGIN
{
    char username[56];
    char password[40];
}STRUCT_LOGIN, *LPSTRUCT_LOGIN;

#pragma pack()



#endif