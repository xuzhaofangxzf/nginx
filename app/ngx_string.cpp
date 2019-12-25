#include <stdio.h>
#include <string.h>


//截取字符串尾部空格以及煮食字符
void Rtrim(char *str)
{
    size_t len = 0;
    if(str == NULL)
    {
        return;
    }
    len = strlen(str);
    while (len > 0 && (str[len - 1] == ' '))
    {
        str[--len] = 0;
    }
    return;
}

//截取字符串头部空格
void Ltrim(char *str)
{
    size_t len = 0;
    len = strlen(str);
    char *p_srcstr = str;
    if (*p_srcstr != ' ') //开头不是空格
    {
        return;
    }
    //找到第一个不是空格的字符
    while (*p_srcstr == ' ' && *p_srcstr !='\0')
    {
        p_srcstr++;
    }
    if (*p_srcstr == '\0')
    {
        *str = '\0';
        return;    
    }
    //从第一个不为空格的字符开始，拷贝到以头部开始
    char * p_dststr = str;
    while (*p_srcstr != '\0')
    {
        (*p_dststr) = (*p_srcstr);

        p_srcstr++;
        p_dststr++;
    }
    *p_srcstr = '\0';
    return;
 
}
//拷贝=后面的值，自动去除#注释代码
// char *ngx_strcpy(char *__dst, const char *__src)
// {

// }
