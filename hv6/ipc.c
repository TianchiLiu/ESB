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
    //struct proc *sender, *receiver;
    struct tk5_esb  *esb;

    if (!is_pid_valid(pid))
        return -ESRCH;
    
    if (!is_service_valid(service))
       return -1;  
    if (esb == NULL)
        return K5_NO_ACCESS;
    //assert(esb!=NULL,);
    //assert(pid > 0 && pid < 128,"test");
    //assert(pid != current, "current is running and pid is sleeping");
    // assert(current<1000,"test");
    // assert(1>0,"aa");
    //assert(global_esb!=NULL,"global_esb is NULL");
    esb=get_esb(pid);
    //memset(esb, 0, sizeof(struct tk5_esb)); /*清零ESB帧结构,整页*/
    
    

    esb->primitive = K5_SEND;
    esb->src_port=current;
    esb->dst_port=pid;
    esb->service=service;
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

    if (!is_pid_valid(pid))
        return -ESRCH;

    if (esb == NULL)
        return K5_NO_ACCESS;

    esb=get_esb(pid);
    esb->primitive = K5_REPLY; /*设置服务原语*/
    esb->head = K5_H1; /*设置扩展头部H1*/
    return 0;
}

//add:k5_wait
int k5_wait(pid_t pid,tU4 w_len){
    struct tk5_esb  *esb;

    if (!is_pid_valid(pid))
        return -ESRCH;

    if (esb == NULL)
        return K5_NO_ACCESS;

    esb=get_esb(pid);
    esb->primitive = K5_WAIT; /*设置服务原语*/
    esb->src_port=pid;
    esb->dst_port=current;

    return 0;
}

//add:k5_call
int k5_call(pid_t pid,tU2 service,tU4 c_len){
    struct tk5_esb  *esb;
    if (!is_pid_valid(pid))
        return -ESRCH;
    
    if (!is_service_valid(service))
       return -1;  
    if (esb == NULL)
        return K5_NO_ACCESS;

    esb=get_esb(pid);
    tU4 src_port=to->src_port;
    tU4 dst_port=to->dst_port;
    esb->primitive = K5_CALL;
    esb->src_port=src_port;
    esb->dst_port=dst_port;
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
