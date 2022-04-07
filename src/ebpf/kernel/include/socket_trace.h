#ifndef __BPF_SOCKET_TRACE_H__
#define __BPF_SOCKET_TRACE_H__

#include "bpf_base.h"
#include "common.h"
#include "kernel.h"
#include "bpf_endian.h"

#ifndef unlikely
#define unlikely(x)             __builtin_expect(!!(x), 0)
#endif

#ifndef likely
#define likely(x)               __builtin_expect(!!(x), 1)
#endif

#ifndef BPF_USE_CORE
#include <sys/socket.h>
#include <stddef.h>
#include <linux/in.h>

typedef long unsigned int __kernel_size_t;

enum {
	TCPF_ESTABLISHED = (1 << 1),
	TCPF_SYN_SENT = (1 << 2),
	TCPF_SYN_RECV = (1 << 3),
	TCPF_FIN_WAIT1 = (1 << 4),
	TCPF_FIN_WAIT2 = (1 << 5),
	TCPF_TIME_WAIT = (1 << 6),
	TCPF_CLOSE = (1 << 7),
	TCPF_CLOSE_WAIT = (1 << 8),
	TCPF_LAST_ACK = (1 << 9),
	TCPF_LISTEN = (1 << 10),
	TCPF_CLOSING = (1 << 11)
};

struct user_msghdr {
	void *msg_name;
	int msg_namelen;
	struct iovec *msg_iov;
	__kernel_size_t msg_iovlen;
	void *msg_control;
	__kernel_size_t msg_controllen;
	unsigned int msg_flags;
};

struct mmsghdr {
	struct user_msghdr msg_hdr;
	unsigned int msg_len;
};
#else
#define NULL ((void*)0)
#define AF_INET         2	/* Internet IP Protocol         */
#define PF_INET         AF_INET
#define AF_INET6        10	/* IP version 6                 */
#define PF_INET6        AF_INET6
#endif

#define CONN_ADD		    0
#define CONN_DEL		    1

#define SOCK_DIR_SND_REQ            0
#define SOCK_DIR_SND_RES            1
#define SOCK_DIR_RCV_REQ            2
#define SOCK_DIR_RCV_RES            3
#define SOCK_ADD_EVENT	            4
#define SOCK_INFO_EVENT             5

#define HTTP_REQUEST_MIN_LEN 7

#define HTTP_CODE_MSG_LEN 16
#define AF_UNKNOWN 0xff

#define SOCK_CHECK_TYPE_ERROR           0
#define SOCK_CHECK_TYPE_UDP             1
#define SOCK_CHECK_TYPE_TCP_ES          2

#include "socket_trace_common.h"

struct member_fields_offset {
	__u8 ready;
	__u32 task__files_offset;
	__u32 sock__flags_offset;
	__u8  socket__has_wq_ptr;
	__u32 tcp_sock__copied_seq_offset;
	__u32 tcp_sock__write_seq_offset;
};

/********************************************************/
// socket trace struct
/********************************************************/
#define socklen_t size_t

union sockaddr_t {
	struct sockaddr sa;
	struct sockaddr_in in4;
	struct sockaddr_in6 in6;
};

struct conn_info_t {
#ifdef PROBE_CONN
	__u64 id;
#endif
	struct __tuple_t tuple;
	__u16 skc_family;	/* PF_INET, PF_INET6... */
	__u16 sk_type;		/* socket type (SOCK_STREAM, etc) */
	__u8 skc_ipv6only;
	void *sk;

	// The protocol of traffic on the connection (HTTP, MySQL, etc.).
	enum traffic_protocol protocol;
	// MSG_UNKNOWN, MSG_REQUEST, MSG_RESPONSE
	enum message_type message_type;

	enum traffic_direction direction; /* T_INGRESS T_EGRESS ... */
	size_t prev_count;
	char prev_buf[4];
	enum traffic_direction prev_direction;
	struct socket_info_t *socket_info_ptr; /* lookup __socket_info_map */
};

enum syscall_src_func {
	SYSCALL_FUNC_UNKNOWN,
	SYSCALL_FUNC_WRITE,
	SYSCALL_FUNC_READ,
	SYSCALL_FUNC_SEND,
	SYSCALL_FUNC_RECV,
	SYSCALL_FUNC_SENDTO,
	SYSCALL_FUNC_RECVFROM,
	SYSCALL_FUNC_SENDMSG,
	SYSCALL_FUNC_RECVMSG,
	SYSCALL_FUNC_SENDMMSG,
	SYSCALL_FUNC_RECVMMSG,
	SYSCALL_FUNC_WRITEV,
	SYSCALL_FUNC_READV,
	SYSCALL_FUNC_SENDFILE
};

struct data_args_t {
	// Represents the function from which this argument group originates.
	enum syscall_src_func source_fn;
	__u32 fd;
	// For send()/recv()/write()/read().
	const char *buf;
	// For sendmsg()/recvmsg()/writev()/readv().
	const struct iovec *iov;
	size_t iovlen;
	// For sendmmsg()
	unsigned int *msg_len;
};

#define TPPROG(F) SEC("tracepoint/syscalls/"__stringify(F)) int bpf_func_##F

struct syscall_comm_enter_ctx {
	__u64 __pad_0;		/*     0     8 */
	int __syscall_nr;	/*    offset:8     4 */
	__u32 __pad_1;		/*    12     4 */
	__u64 fd;		/*    offset:16    8 */
	char *buf;		/*    24     8 */
	size_t count;		/*    32     8 */
};

struct syscall_comm_exit_ctx {
	__u64 __pad_0;		/*     0     8 */
	int __syscall_nr;	/*    offset:8     4 */
	__u32 __pad_1;		/*    12     4 */
	__u64 ret;		/*    offset:16    8 */
};

static __inline __u64 gen_conn_key_id(__u64 param_1, __u64 param_2)
{
	/*
	 * key:
	 *  - param_1 28bits as high bits.
	 *  - param_2 36bits as low bits.
	 *
	 * 如果用进程号作为param_1, 那么这里囊括的进程号的范围是 [0 ~ 2^28 - 1]
	 */
	return ((param_1 >> 4) << 36) | (param_2 & 0xfffffffffULL);
}

#endif /* __BPF_SOCKET_TRACE_H__ */
