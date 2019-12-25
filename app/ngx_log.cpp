#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>    //va_start...
#include <unistd.h>    //STDERRO_FILENO et
#include <sys/time.h>  //gettimeofday
#include <time.h>      //localtime_r
#include <fcntl.h>     //open
#include <errno.h>     //errno

#include "ngx_global.hpp"
#include "ngx_func.hpp"
#include "ngx_c_conf.hpp"
#include "ngx_macro.hpp"
#include "ngx_string.hpp"


//全局量---------------------
//错误等级，和ngx_macro.h里定义的日志等级宏是一一对应关系
static u_char err_levels[][20]  = 
{
    {"stderr"},    //0：控制台错误
    {"emerg"},     //1：紧急
    {"alert"},     //2：警戒
    {"crit"},      //3：严重
    {"error"},     //4：错误
    {"warn"},      //5：警告
    {"notice"},    //6：注意
    {"info"},      //7：信息
    {"debug"}      //8：调试
};

ngx_log_t   ngx_log;

void ngx_log_stderr(int err, const char *fmt, ...)
{
    va_list args;
    u_char errstr[NGX_MAX_ERROR_STR + 1];
    u_char *p, *last;

    memset(errstr, 0, sizeof(errstr));

    last = errstr + NGX_MAX_ERROR_STR;    //last指向整个buffer最后去了【指向最后一个有效位置的后面也就是非有效位】，作为一个标记，防止输出内容超过这么长,

    p = ngx_cpymem(errstr, "nginx: ", 7);
    va_start(args, fmt); //使args指向第一个变参的参数
    /*
        int vsnprintf(char *buffer, size_t count, const char *format, va_list argptr);
        buffer: Storage location for output.

        count: Maximum number of characters to write.

        format: Format specification.

        argptr: Pointer to list of arguments.
    The vsnprintf function returns the number of characters written, not counting the terminating null character.
    */
    int cnt = vsnprintf((char*)p, NGX_MAX_ERROR_STR - 7, fmt, args);
    va_end(args);
    if (cnt < 0)
    {
        perror("ngx_log_stderr: string too large to print!\n");
        return;
    }
    p = p + cnt;
    if (err != 0)
    {
        p = ngx_log_errno(p, last, err);
    }
    *(p++) = '\n';
    write(STDERR_FILENO, errstr,NGX_MAX_ERROR_STR);
    return;
}

//----------------------------------------------------------------------------------------------------------------------
//描述：给一段内存，一个错误编号，我要组合出一个字符串，形如：   (错误编号: 错误原因)，放到给的这段内存中去
//     这个函数我改造的比较多，和原始的nginx代码多有不同
//buf：是个内存，要往这里保存数据
//last：放的数据不要超过这里
//err：错误编号，我们是要取得这个错误编号对应的错误字符串，保存到buffer中
u_char *ngx_log_errno(u_char *buf, u_char *last, int err)
{
    char *perrorinfo = strerror(err);
    size_t errorlen = strlen(perrorinfo); //error信息的长度

    //前后插入一些字符串(%d:)
    char leftstr[20] = {0};//字符不超过10的长度
    sprintf(leftstr, " (ErrorCode: %d: ", err);
    size_t leftlen = strlen(leftstr);

    char rightstr[] = ") ";
    size_t rightlen = strlen(rightstr);

    size_t extralen = leftlen + rightlen; //left extral length and right extral length

    if ((buf + errorlen + extralen) < last)
    {
        buf = ngx_cpymem(buf, leftstr, leftlen);
        buf = ngx_cpymem(buf, perrorinfo, errorlen);
        buf = ngx_cpymem(buf, rightstr, rightlen);

    }
    else
    {
        perror("ngx_log_errno: string too large to print\n");
    }
    return buf;   
}

//----------------------------------------------------------------------------------------------------------------------
//往日志文件中写日志，代码中有自动加换行符，所以调用时字符串不用刻意加\n；
//    日过定向为标准错误，则直接往屏幕上写日志【比如日志文件打不开，则会直接定位到标准错误，此时日志就打印到屏幕上，参考ngx_log_init()】
//level:一个等级数字，我们把日志分成一些等级，以方便管理、显示、过滤等等，如果这个等级数字比配置文件中的等级数字"LogLevel"大，那么该条信息不被写到日志文件中
//err：是个错误代码，如果不是0，就应该转换成显示对应的错误信息,一起写到日志文件中，
//ngx_log_error_core(5,8,"这个XXX工作的有问题,显示的结果是=%s","YYYY");

void ngx_log_error_core(int level, int err, const char *fmt, ...)
{
    u_char *last;
    u_char errstr[NGX_MAX_ERROR_STR + 1];

    memset(errstr, 0, sizeof(errstr));
    last = errstr + NGX_MAX_ERROR_STR;

    struct timeval tv;
    struct tm      tm;
    //time_t         sec;    //seconds
    u_char         *p;     //指向当前要拷贝数据到其中的内存位置
    va_list        args;

    memset(&tv, 0, sizeof(struct timeval));
    memset(&tm, 0, sizeof(tm));

    /*计算时间的另一种方法*/

    time_t time_seconds = time(0);
    localtime_r(&time_seconds, &tm);

    #if 0
    gettimeofday(&tv, NULL);    //获取当前时间，返回自1970-01-01 00:00:00到现在经历的秒数【第二个参数是时区，一般不关心】
    sec = tv.tv_sec;
    localtime_r(&sec, &tm);    //把参数1的time_t转换为本地时间，保存到参数2中去，带_r的是线程安全的版本，尽量使用
    #endif
    tm.tm_mon++;
    tm.tm_year +=1900;
    char strCurTime[40] = {0};   //先组合出一个当前时间字符串，格式形如：2019/01/08 19:57:11
    snprintf(strCurTime, 40, "%4d/%02d/%02d %02d:%02d:%02d", 
                    tm.tm_year, tm.tm_mon,
                    tm.tm_mday, tm.tm_hour,
                    tm.tm_min, tm.tm_sec);
    p = ngx_cpymem(errstr, strCurTime, strlen((const char*)strCurTime)); ////日期增加进来，得到形如：     2019/01/08 20:26:07
    int levellen = sprintf((char *)p, " [%s] ", (const char *)err_levels[level]);
    p += levellen;
    int pidlen = sprintf((char *)p, "%d:", ngx_pid);
    p += pidlen;
    va_start(args, fmt);
    int msglen = vsnprintf((char *)p, last - p, fmt, args);
    p += msglen;
    va_end(args);
    if (err != 0)
    {
        p = ngx_log_errno(p, last, err);
    }
    *(p++) = '\n';
    
    ssize_t fd; //写文件描述符

    for (;;)
    {
        if (level > ngx_log.log_level)
        {
            //数字越大，等级越低
            break;
        }
        fd = write(ngx_log.fd, errstr, p - errstr);
        if (fd == -1)
        {
            //写文件失败

            if(errno == ENOSPC) //写失败，且原因是磁盘没空间了
            {
                //磁盘没空间了
                //没空间还写个毛线啊
                //先do nothing吧；
            }
            else
            {
                //这是有其他错误，那么我考虑把这个错误显示到标准错误设备吧；
                if(ngx_log.fd != STDERR_FILENO) //当前是定位到文件的，则条件成立
                {
                    fd = write(STDERR_FILENO,errstr,p - errstr);
                }
            }
        }
        break;
        
    }
    return;
}


//----------------------------------------------------------------------------------------------------------------------
//描述：日志初始化，就是把日志文件打开 ，注意这里边涉及到释放的问题，如何解决？

void ngx_log_init()
{
    u_char *plogname = NULL;
    //size_t nlen;
    //从配置文件中读取和日志相关的配置信息
    CConfig *p_config = CConfig::getInstance();
    plogname = (u_char*)p_config->getString("Log");
    if(plogname == NULL)
    {
       //没读到，就要给个缺省的路径文件名了
        plogname = (u_char *) NGX_ERROR_LOG_PATH; //"logs/error.log" ,logs目录需要提前建立出来    
    }
    ngx_log.log_level = p_config->getIntDefault("LogLevel", NGX_LOG_NOTICE);
    //只写打开|追加到末尾|文件不存在则创建【这个需要跟第三参数指定文件访问权限】
    //mode = 0644：文件访问权限， 6: 110    , 4: 100：     【用户：读写， 用户所在组：读，其他：读】 老师在第三章第一节介绍过
    ngx_log.fd = open((const char *)plogname, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (ngx_log.fd == -1)
    {
        ngx_log_stderr(errno, "alert: could not open log file : open() \"%s\" failed", plogname);
        ngx_log.fd = STDERR_FILENO; //直接定位到标准错误去了
    }
    return;
    
}