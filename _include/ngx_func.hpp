#ifndef __NGX_FUNC_H__
#define __NGX_FUNC_H__

#include "ngx_string.hpp"

void Rtrim(char *str);
void Ltrim(char *str);
void ngxInitSetProcTitle();
void ngxSetProcTitle(const char* title);
void ngx_log_stderr(int err, const char *fmt, ...);
u_char *ngx_log_errno(u_char *buf, u_char *last, int err);
void ngx_log_init();
void ngx_log_error_core(int level, int err, const char *fmt, ...);
//signals and process relavents
int ngx_init_signals();
void ngx_master_process_cycle();
int ngx_daemon();

//处理socket事件相关
void ngx_process_events_and_timers();
#endif