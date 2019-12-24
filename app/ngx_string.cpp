#include <stdio.h>
#include <string.h>


//截取字符串尾部空格
void Rtrim(char *str)
{
    size_t len = 0;
    if(str == NULL)
    {
        return;
    }
    len = strlen(str);
    while (len > 0 && str[len - 1] == ' ')
    {
        str[--len] = 0;
    }
    return;
}

//截取字符串尾部空格
void Ltrim(char *str)
{
    size_t len = 0;
    len = strlen(str);
    char *p_dst = str;
    if (*p_dst != ' ') //开头不是空格
    {
        return;
    }
    //找到第一个不是空格的字符
    while (*p_dst == ' ' && *p_dst !='\0')
    {
        p_dst++;
    }
    if (*p_dst == '\0')
    {
        *str = '\0';
        return;    
    }
    //从第一个不为空格的字符开始，拷贝到以头部开始
    char * p_src = str;
    while (*p_dst != '\0')
    {
        (*p_dst) = (*p_src);
        p_dst++;
        p_src++;
    }
    *p_dst = '\0';
    return;
    

}
