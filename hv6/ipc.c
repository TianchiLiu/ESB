#include <string.h>
#include "ipc.h"
#include "proc.h"
#include "stdio.h"
int send_proc(pid_t pid, uint64_t val, pn_t pn, size_t size, int fd)
{
    struct proc *sender, *receiver;

    if (!is_pid_valid(pid))
        return -ESRCH;
    if (!is_proc_state(pid, PROC_SLEEPING))
        return -EAGAIN;
    if (!is_pn_valid(pn))
        return -EINVAL;
    /* pn doesn't have to be a frame */
    if (!is_page_pid(pn, current))
        return -EACCES;
    if (size > PAGE_SIZE)
        return -EINVAL;
    if (is_fd_valid(fd)) {
        if (!is_fn_valid(get_fd(current, fd)))
            return -EINVAL;
    }

    assert(pid != current, "current is running and pid is sleeping");

    sender = get_proc(current);
    receiver = get_proc(pid);
    //printf("ipc.c: %s\n", (char)pid);
    receiver->ipc_from = current;
    receiver->ipc_val = val;
    receiver->ipc_size = size;
    /* invariant: proc->ipc_page is a valid page owned by pid */
    memcpy(get_page(receiver->ipc_page), get_page(pn), size);
    /* invariant: ipc_fd is empty if it's valid */
    if (is_fd_valid(fd) && is_fd_valid(receiver->ipc_fd))
        set_fd(pid, receiver->ipc_fd, get_fd(current, fd));

    sender->state = PROC_RUNNABLE;
    receiver->state = PROC_RUNNING;
    /* receiver: sleeping -> running */
    proc_ready_add(receiver);
    current = pid;
    return 0;
}

int k5_send(pid_t pid, tU2 service,tU4 s_len)
{
    struct tk5_net *to=to;
    //struct proc *sender, *receiver;
    struct tk5_esb *current_esb=get_esb(pid);

    struct tk5_ehn* ehn; /*ESB扩展网络地址，8字节*/
    tU1 i, j; /*循环临时变量*/

    // if (current_esb == NULL)
    //     return K5_NO_ACCESS;

    if (!is_pid_valid(pid))
        return -ESRCH;
    
    if (!is_service_valid(service))
       return -1; 
    if(!is_s_len_valid(s_len))
        return -1; 

    
    //assert(current_esb!=NULL,);
    //current_esb=NULL;
    /*设置扩展头body[0],已清零*/

    if (to->net_level >= K5_N1 && to->net_level <= K5_N6) {
        for (i = 0, j = 0; i < 6; i++, j++) {
            ehn->dst_addr = to->dst_addr; /*第i级网络目的地址*/
            ehn->src_addr = to->src_addr; /*第i级网络源地址*/
            current_esb->body[j] = (tU8)ehn;   //error trans
            //memcpy(&esb->body[j], &ehn,
            //       sizeof(tU8)); /*cast映射到body[j]*/
        }
    }



    if (is_s_len_valid(s_len)) /*若有数据要发送,拷贝到ESB*/
    {
            tU4 head=current_esb->head;
            memcpy(body+head, s_buf+head, s_len);
            // for(i=head; i<s_len;i++){
            //     current_esb->body[i]=body[head][0];
            // }
            //current_esb->body=body;
            //memcpy(current_esb->body[head],body+head,s_len);
            //current_esb->body[current_esb->head] = *s_buf;
            current_esb->size = (current_esb->head + 1) * 8 + s_len; /*帧总长=头长+体长*/

        
         
        
    }
    //assert(pid > 0 && pid < 128,"test");
    //assert(pid != current, "current is running and pid is sleeping");
    // assert(current<1000,"test");
    // assert(1>0,"aa");
    //assert(global_esb!=NULL,"global_esb is NULL");
    //  esb=current_esb;
    //memset(esb, 0, sizeof(struct tk5_esb)); /*清零ESB帧结构,整页*/
    
    

    current_esb->primitive = K5_SEND;
    current_esb->service=service;
    current_esb->head=to->net_level;
    current_esb->src_port=to->src_port;
    current_esb->dst_port=to->dst_port;
    
    // val=current_esb->val;
    // current=current_esb->ipc_from;
    // pid=current_esb->ipc_to;

    // receiver = get_proc(global_esb->ipc_to);
    // receiver->ipc_from=global_esb->ipc_from;
    // receiver->ipc_val=global_esb->val;
    


    return 0;
}

//add:k5_reply
int k5_reply(pid_t pid,tI2 ack_err,tU4 s_len){
    struct tk5_esb  *esb;
    struct tk5_eh1 *eh1;
    if (!is_pid_valid(pid))
        return -ESRCH;

    if (esb == NULL)
        return K5_NO_ACCESS;

    esb=get_esb(pid);
    esb->primitive = K5_REPLY; /*设置服务原语*/
    esb->head = K5_H1; /*设置扩展头部H1*/
    eh1 = (struct tk5_eh1 *)&esb->body[0]; /*获取并展开扩展头部*/
    eh1->ack_seq == ack_err; /*确认已收到的帧序号*/

    memcpy(body, s_buf, sizeof(tU8));
    //设置扩展头部
    if (is_s_len_valid(s_len))//若有服务结果数据送回*/
    {
        tU4 head=esb->head;
        memcpy(body+head, s_buf+head, s_len);
        esb->size = (esb->head + 1) * 8 + s_len; /*设置帧长度 = 头长+体长*/
    };
    return 0;
}

//add:k5_wait
int k5_wait(pid_t pid,tU4 w_len){
    struct tk5_esb  *esb;
    struct tk5_net *from=from;
    struct tk5_ehn* ehn;
    tU1 i, j;

    if (!is_pid_valid(pid))
        return -ESRCH;

    if (esb == NULL)
        return K5_NO_ACCESS;
    // if (from == NULL)
    //     return K5_NO_ACCESS;
    //memset(esb, 0, K5_MAX_BUF); /*清零ESB帧结构,整页*/

    if (from->net_level >= K5_N1 && from->net_level <= K5_N6) {
        for (i = 0, j = 0; i < 6; i++, j++) {
            ehn->src_addr = from->src_addr; /*设置等待第i级网络源地址*/
            current_esb->body[j] = (tU8)ehn;
                //memcpy(&esb->body[j], &ehn,sizeof(tU8)); /*cast映射到body[j]*/
        }
    }
    esb=get_esb(pid);
    esb->primitive = K5_WAIT; /*设置服务原语*/
    esb->dst_port=current;

    
    esb->head = from->net_level; /*设置帧扩展长度*/
    esb->src_port = from->src_port; /*设置等待特定源端口*/
    
    // if (w_len >= esb->size) /*若指定缓冲区且足够大*/
    // {
    //     memcpy(w_buf, esb, esb->size); /*则连头拷贝到用户缓冲区*/
    //     return (esb->size); /*给用户返回接收到帧长*/
    // } else
    //     return K5_COMPLETE; /*否则返回1，内容在ESB中*/

    return 0;
}

//add:k5_call
int k5_call(pid_t pid,tU2 service,tU4 c_len){
    struct tk5_esb *esb;
    struct tk5_net *to=to;
    struct tk5_svc *svc; /*服务编码的展开结构，14比特展开4字节*/
    struct tk5_ehn* ehn; /*ESB扩展网络地址，8字节*/
    tU1 i, j; /*循环临时变量，2018-10-03增加*/
    
    svc = _svc; /*展开14比特的服务编码*/
    if (!is_pid_valid(pid))
        return -ESRCH;
    
    if (!is_service_valid(service))
       return -1;  
    
    esb=get_esb(pid);
    //if ((esb) == NULL)
    //    return K5_NO_ACCESS;
    
    //esb=get_esb(pid);
    esb->primitive = K5_CALL;

    //tU4 src_port=to->src_port;
    //tU4 dst_port=to->dst_port;
    esb->src_port=to->src_port;
    esb->dst_port=to->dst_port;

    esb->service=service;

    return 0;
}

int kk_send(pid_t pid, tU2 service,tU4 s_len)
{
    struct tk5_net *to=to;
    //struct proc *sender, *receiver;
    struct tk5_esb *current_esb=get_esb(pid);

    struct tk5_ehn* ehn; /*ESB扩展网络地址，8字节*/
    tU1 i, j; /*循环临时变量*/

    // if (current_esb == NULL)
    //     return K5_NO_ACCESS;

    if (!is_pid_valid(pid))
        return -ESRCH;
    
    if (!is_service_valid(service))
       return -1; 
    if(!is_s_len_valid(s_len))
        return -1; 

    
    //assert(current_esb!=NULL,);
    //current_esb=NULL;
    /*设置扩展头body[0],已清零*/

    if (to->net_level >= K5_N1 && to->net_level <= K5_N6) {
        for (i = 0, j = 0; i < 6; i++, j++) {
            ehn->dst_addr = to->dst_addr; /*第i级网络目的地址*/
            ehn->src_addr = to->src_addr; /*第i级网络源地址*/
            current_esb->body[j] = (tU8)ehn;   //error trans
            //memcpy(&esb->body[j], &ehn,
            //       sizeof(tU8)); /*cast映射到body[j]*/
        }
    }



    if (is_s_len_valid(s_len)) /*若有数据要发送,拷贝到ESB*/
    {
            tU4 head=current_esb->head;
            memcpy(body+head, s_buf+head, s_len);
            // for(i=head; i<s_len;i++){
            //     current_esb->body[i]=body[head][0];
            // }
            //current_esb->body=body;
            //memcpy(current_esb->body[head],body+head,s_len);
            //current_esb->body[current_esb->head] = *s_buf;
            current_esb->size = (current_esb->head + 1) * 8 + s_len; /*帧总长=头长+体长*/

        
         
        
    }
    //assert(pid > 0 && pid < 128,"test");
    //assert(pid != current, "current is running and pid is sleeping");
    // assert(current<1000,"test");
    // assert(1>0,"aa");
    //assert(global_esb!=NULL,"global_esb is NULL");
    //  esb=current_esb;
    //memset(esb, 0, sizeof(struct tk5_esb)); /*清零ESB帧结构,整页*/
    
    

    current_esb->primitive = K5_SEND;
    current_esb->service=service;
    current_esb->head=to->net_level;
    current_esb->src_port=to->src_port;
    current_esb->dst_port=to->dst_port;
    
    // val=current_esb->val;
    // current=current_esb->ipc_from;
    // pid=current_esb->ipc_to;

    // receiver = get_proc(global_esb->ipc_to);
    // receiver->ipc_from=global_esb->ipc_from;
    // receiver->ipc_val=global_esb->val;
    


    return 0;
}

//add:kk_reply
int kk_reply(pid_t pid,tI2 ack_err,tU4 s_len){
    struct tk5_esb  *esb;
    struct tk5_eh1 *eh1;
    if (!is_pid_valid(pid))
        return -ESRCH;

    if (esb == NULL)
        return K5_NO_ACCESS;

    esb=get_esb(pid);
    esb->primitive = K5_REPLY; /*设置服务原语*/
    esb->head = K5_H1; /*设置扩展头部H1*/
    eh1 = (struct tk5_eh1 *)&esb->body[0]; /*获取并展开扩展头部*/
    eh1->ack_seq == ack_err; /*确认已收到的帧序号*/

    memcpy(body, s_buf, sizeof(tU8));
    //设置扩展头部
    if (is_s_len_valid(s_len))//若有服务结果数据送回*/
    {
        tU4 head=esb->head;
        memcpy(body+head, s_buf+head, s_len);
        esb->size = (esb->head + 1) * 8 + s_len; /*设置帧长度 = 头长+体长*/
    };
    return 0;
}

//add:k5_wait
int kk_wait(pid_t pid,tU4 w_len){
    struct tk5_esb  *esb;
    struct tk5_net *from=from;
    struct tk5_ehn* ehn;
    tU1 i, j;

    if (!is_pid_valid(pid))
        return -ESRCH;

    if (esb == NULL)
        return K5_NO_ACCESS;
    // if (from == NULL)
    //     return K5_NO_ACCESS;
    //memset(esb, 0, K5_MAX_BUF); /*清零ESB帧结构,整页*/

    if (from->net_level >= K5_N1 && from->net_level <= K5_N6) {
        for (i = 0, j = 0; i < 6; i++, j++) {
            ehn->src_addr = from->src_addr; /*设置等待第i级网络源地址*/
            current_esb->body[j] = (tU8)ehn;
                //memcpy(&esb->body[j], &ehn,sizeof(tU8)); /*cast映射到body[j]*/
        }
    }
    esb=get_esb(pid);
    esb->primitive = K5_WAIT; /*设置服务原语*/
    esb->dst_port=current;

    
    esb->head = from->net_level; /*设置帧扩展长度*/
    esb->src_port = from->src_port; /*设置等待特定源端口*/
    
    // if (w_len >= esb->size) /*若指定缓冲区且足够大*/
    // {
    //     memcpy(w_buf, esb, esb->size); /*则连头拷贝到用户缓冲区*/
    //     return (esb->size); /*给用户返回接收到帧长*/
    // } else
    //     return K5_COMPLETE; /*否则返回1，内容在ESB中*/

    return 0;
}

//add:k5_call
int kk_call(pid_t pid,tU2 service,tU4 c_len){
    struct tk5_esb *esb;
    struct tk5_net *to=to;
    struct tk5_svc *svc; /*服务编码的展开结构，14比特展开4字节*/
    struct tk5_ehn* ehn; /*ESB扩展网络地址，8字节*/
    tU1 i, j; /*循环临时变量，2018-10-03增加*/
    
    svc = _svc; /*展开14比特的服务编码*/
    if (!is_pid_valid(pid))
        return -ESRCH;
    
    if (!is_service_valid(service))
       return -1;  
    
    esb=get_esb(pid);
    //if ((esb) == NULL)
    //    return K5_NO_ACCESS;
    
    //esb=get_esb(pid);
    esb->primitive = K5_CALL;

    //tU4 src_port=to->src_port;
    //tU4 dst_port=to->dst_port;
    esb->src_port=to->src_port;
    esb->dst_port=to->dst_port;

    esb->service=service;

    return 0;
}
/*
 * This is called by sys_recv in entry.S.
 * - Upon entry, current's hvm is already flushed.
 * - Upon exit, run_current() is called to return to user space.
 */
int recv_proc(pid_t pid, pn_t pn, int fd)
{
    struct proc *server;

    if (!is_pid_valid(pid))
        return -ESRCH;
    if (!is_proc_state(pid, PROC_RUNNABLE))
        return -EINVAL;
    if (!is_pn_valid(pn))
        return -EINVAL;
    if (!is_page_pid(pn, current))
        return -EACCES;
    if (!is_page_type(pn, PAGE_TYPE_FRAME))
        return -EINVAL;
    if (is_fd_valid(fd)) {
        if (is_fn_valid(get_fd(current, fd)))
            return -EINVAL;
    }

    assert(pid != current, "current is running and pid is runnable");
    server = get_proc(current);
    server->ipc_from = 0;
    server->ipc_page = pn;
    server->ipc_size = 0;
    server->ipc_fd = fd;

    server->state = PROC_SLEEPING;
    /* server: running -> sleeping */
    proc_ready_del(server);
    get_proc(pid)->state = PROC_RUNNING;
    current = pid;
    return 0;
}

static int send_recv(pid_t pid, uint64_t val, pn_t inpn, size_t size, int infd, pn_t outpn,
                     int outfd)
{
    struct proc *sender, *receiver;

    /* target process must be sleeping */
    if (!is_pid_valid(pid))
        return -ESRCH;
    if (!is_proc_state(pid, PROC_SLEEPING))
        return -EAGAIN;
    /* check in page */
    if (!is_pn_valid(inpn))
        return -EINVAL;
    /* inpn doesn't have to be a frame */
    if (!is_page_pid(inpn, current))
        return -EACCES;
    if (size > PAGE_SIZE)
        return -EINVAL;
    /* check in fd: either invalid or non-empty */
    if (is_fd_valid(infd)) {
        if (!is_fn_valid(get_fd(current, infd)))
            return -EINVAL;
    }
    /* check out page */
    if (!is_pn_valid(outpn))
        return -EINVAL;
    if (!is_page_pid(outpn, current))
        return -EACCES;
    if (!is_page_type(outpn, PAGE_TYPE_FRAME))
        return -EINVAL;
    /* check out fd: either invalid or empty */
    if (is_fd_valid(outfd)) {
        if (is_fn_valid(get_fd(current, outfd)))
            return -EINVAL;
    }

    assert(pid != current, "current is running and pid is sleeping");

    sender = get_proc(current);
    receiver = get_proc(pid);

    /* check if the receiver wants to accept anyone's request */
    if (receiver->ipc_from && receiver->ipc_from != current)
        return -EACCES;

    /* client: prepare for response */
    sender->ipc_page = outpn;
    sender->ipc_fd = outfd;

    /* server: transfer data from client */
    receiver->ipc_from = current;
    receiver->ipc_val = val;
    /* transfer the page */
    memcpy(get_page(receiver->ipc_page), get_page(inpn), size);
    receiver->ipc_size = size;
    /* transfer the fd */
    if (is_fd_valid(infd) && is_fd_valid(receiver->ipc_fd))
        set_fd(pid, receiver->ipc_fd, get_fd(current, infd));

    /* switch control */

    sender->state = PROC_SLEEPING;
    receiver->state = PROC_RUNNING;
    /* receiver: sleeping -> running */
    proc_ready_add(receiver);
    /* sender: running -> sleeping */
    proc_ready_del(sender);
    return 0;
}

int reply_wait_proc(pid_t pid, uint64_t val, pn_t inpn, size_t size, int infd, pn_t outpn)
{
    int r;

    r = send_recv(pid, val, inpn, size, infd, outpn, -1);
    if (r)
        return r;
    /* accept any ipc */
    get_proc(current)->ipc_from = 0;
    current = pid;
    return 0;
}

int call_proc(pid_t pid, uint64_t val, pn_t inpn, size_t size, pn_t outpn, int outfd)
{
    int r;

    r = send_recv(pid, val, inpn, size, -1, outpn, outfd);
    if (r)
        return r;
    /* accept any ipc */
    get_proc(current)->ipc_from = pid;
    current = pid;
    return 0;
}
