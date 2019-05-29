#pragma once
/**
 *******************************************************************
 * k5_esb.h
 *
 * Micro Kernel OS K5 微内核操作系统公用数据结构定义 
 *******************************************************************
 */

/**
 * ---------------------------------------------------------------
 *  事件服务总线基础常数定义
 *-----------------------------------------------------------------
 */

/*常用数据类型定义*/
typedef char tI1; //有符号1字节整型数
typedef unsigned char tU1; //无符号1字节整型数
typedef short tI2; //有符号2字节整型数
typedef unsigned short tU2; //无符号2字节整型数
typedef int tI4; //有符号4字节整型数
typedef unsigned int tU4; //无符号4字节整型数
typedef long long tI8; //有符号8字节整型数
typedef unsigned long long tU8; //无符号8字节整型数
typedef float tF4; //4字节浮点数
typedef double tF8; //8字节浮点数

/*事件服务总线帧头类型和6级网络定义（3比特）*/
#define K5_H0 1 //0型帧，头部2个八字节；
#define K5_H1 2 //1型帧，头部3个八字节；
#define K5_N1 3 //1级网，头部4个八字节；
#define K5_N2 4 //2级网，头部5个八字节；
#define K5_N3 5 //3级网，头部6个八字节；
#define K5_N4 6 //4级网，头部7个八字节；
#define K5_N5 7 //5级网，头部8个八字节；
#define K5_N6 8 //6级网，头部9个八字节；

/*服务原语primitive代码定义（2比特）*/
#define K5_CALL 0 //服务原语：00：同步请求调用；
#define K5_REPLY 1 //服务原语：01：异步应答；
#define K5_SEND 2 //服务原语：10：异步发送；
#define K5_WAIT 3 //服务原语：11：同步等待接收；

/*内存页面典型尺寸定义 （可以增加）*/
#define PAGE_4KB 0X00000fff + 1 //最小页面字节数(4KB,12bits)
#define PAGE_64KB 0X0000ffff + 1 //小页面字节数(64KB,16bits)
#define PAGE_1MB 0X000fffff + 1 //中页面字节数(1MB,20bits)
#define PAGE_16MB 0X00ffffff + 1 //大页面字节数(16MB,24bits)

#define K5_BPN 1 //发送缓冲区页面数
#define BUF_SIZE K5_BPN *PAGE_4KB //缺省缓冲区长度
#define DAT_SIZE BUF_SIZE - 8 * 8 //缺省数据区长度

#define K5_EH1 0 //扩展头起始索引号
#define K5_EHN 1 //扩展网络地址起始索引号

#define K5_MAX_BUF 4096 //最大缓冲区长度，1页
#define K5_MAX_BODY 511 //最大数据体长度，1页
#define K5_ESB_PAGE 512 //最大数据体长度，1页

#define K5_KERNEL 0 //微内核服务（虚拟）线程标识
#define K5_PROXY 1 //网络代理服务线程标识
#define K5_SYSTEM 2 //操作系统（虚拟）服务线程标识
#define K5_USER 30 //用户服务代理进程（暂定30）

#define K5_READY 1
#define K5_RUNNING 2
#define K5_BLOCKED 3
#define K5_SUSPENDED 4

/*返回参数定义*/
#define K5_COMPLETE 1
#define KK_COMPLETE 1

#define K5_NO_ACCESS -1
#define K5_NO_SERVER -2
#define K5_ERR_INIT -3
#define K5_ERR_WAIT -4

#define KK_NO_ACCESS -1
#define KK_NO_SERVER -2
#define KK_ERR_INIT -3
#define KK_ERR_WAIT -4
#define KK_NO_PRIMITIVE -5
#define KK_NO_SRC -6
#define KK_NO_DST -7    

struct tk5_esb{

    pid_t src_port;  /*replace the first parameter in send:send to which pid*/
    pid_t dst_port; /*which pid send,default is current in the hv6 OS*/
    uint64_t val;   /*the value that current sends*/
    tU4 primitive;
    tU2 service;
};