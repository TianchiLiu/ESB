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

	tU4 head : 3; //帧头扩展长度，0~7：扩展的8字节长字个数;
	tU4 page : 1; //长度按页计数，0:按8字节，1:按4KB页面计
	tU4 size : 12; //帧总长度，以8字节或4KB页面为单位计数;
    pid_t src_port;  /*replace the first parameter in send:send to which pid*/
    pid_t dst_port; /*which pid send,default is current in the hv6 OS*/
    tU2 service;
    tU4 primitive : 2;
    uint64_t val;   /*the value that current sends*/
    tU8 body[K5_MAX_BODY]; //帧体，扩展头部及所带数据，8字节数组；
};  

/*事件服务总线头部扩展结构：eh1, 仅扩展8字节（推荐）*/
struct tk5_eh1{
	tU2 snd_seq; //发送帧序列号,与IPv4兼容；
	tU2 ack_seq; //确认或否认收到的帧序列号，或错误码；
	tU4 hops : 8; //网络路由跳数，与IPv4和IPv6兼容；
	tU4 qos : 8; //服务质量标识，与IPv4和IPv6兼容；
	tU4 protocol : 8; //上层协议标识，与IPv4和IPv6兼容；
	tU4 endian : 1; //端点标识，0:小端点,1:大端点;仅用于网络;
	tU4 spare : 7; //留作扩展;
}; //ESB帧头扩展结构，用于cast映射body[1];

/*事件服务总线头部网络地址扩展：ehn，仅地址8字节（推荐）*/
struct tk5_ehn{
	tU4 dst_addr; //接收侧目的网络地址;
	tU4 src_addr; //发送侧源端网络地址;
} ; //扩展网络地址，用于cast映射body[2~7];


/**
 * -----------------------------------------------------------------
 * 服务原语相关参数描述，建议本次测试以此为基础。
 *------------------------------------------------------------------
 */
#define K5_MAX_NET_LEVEL 6 //最大网络级数
#define K5_MAX_NET_NAME 64 //最大网络名称字符数
/*事件服务总线网络地址描述 net*/
struct tk5_net{
	tU1 net_level; //网络级数，1为本处理器，直到7，共6级；
	tU1 cvt_level; //已将名称转换为二进制的网络级数（按位）
	tU4 name_len; //网络名称字符串总长度
	tU4 dst_port; //目的端口号，可为：pid、fd、sock；
	tU4 src_port; //源侧端口号，可为：pid、fd、sock；
	//应该写16个，不影响验证的情况下，为了方便，先写2个
	tU4 dst_addr; //接收侧目的网络地址;
	tU4 src_addr; //发送侧源端网络地址;
	//struct tk5_ehn hn[K5_MAX_NET_LEVEL]; //ESB头部网络地址扩展结构，6*8字节
	//tU1 net_name[K5_MAX_NET_NAME]; //字符串描述的网络名称，首次输入
}; //ESB网络地址描述  

/*服务编码展开结构：svc, 仅14比特；（可选）*/
struct tk5_svc{
	tU4 svc_func : 4; //服务功能：每类服务可有16个功能，需优化；
	tU4 svc_type : 4; //服务类型：目前定义11类，需优化；
	tU4 svc_pnum : 3; //服务参数个数： 在服务向量中定义；
	tU4 svc_inout : 1; //服务参数方向：0输入、1输出；2018-10-03增加；
	tU4 svc_space : 2; //服务领域空间：0系统、1及以上用户；
	tU4 primitive : 2; //服务原语，0:请求，1:确认，2:否认，3：等待
	tU4 spare : 16; //备用
} ; //服务码展开，服务原语用；