#pragma once

#include "types.h"

int sys_send(pid_t pid, uint64_t val, pn_t pn, size_t size, int fd);
int sys_recv(pid_t pid, pn_t pn, int fd);
int sys_reply_wait(pid_t pid, uint64_t val, pn_t inpn, size_t size, int infd, pn_t outpn);
int sys_call(pid_t pid, uint64_t val, pn_t inpn, size_t size, pn_t outpn, int outfd);
int sys_send2(pid_t pid, tU2 service,uint64_t val);
