#ifndef __NGX_FUNC_H__
#define __NGX_FUNC_H__
void Rtrim(char *str);
void Ltrim(char *str);
void ngxInitSetProcTitle();
void ngxSetProcTitle(const char* title);
#endif