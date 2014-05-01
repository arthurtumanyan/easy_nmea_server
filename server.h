#ifndef SERVER_H
#define	SERVER_H

#ifdef	__cplusplus
extern "C" {
#endif
#include <netdb.h>

#define EPOLL_RUN_TIMEOUT -1
#define DEF_CON_COUNTER 0
#define EPOLL_TIMEOUT 1

    const char reject_msg[] = "Rejected\n";
    const char format_gpgll_chk[] = "$%s*%02X\r\n";
    const char format_gpgll[] = "$%s\r\n";   
    
    struct _nms {
        char fake_string[52];
        char string[64];
        size_t sz;
    } **nmea_struct;
    //
    int listenfd;
    int s;

    int efd;
    int connfd;
    int init_descr = 0;

    struct epoll_event event;
    struct epoll_event *events;

    struct sockaddr in_addr;
    socklen_t in_len;
    socklen_t clilen;
    struct sockaddr_in cliaddr;
    struct sockaddr_in servaddr;
    char ip[NI_MAXHOST], tmp_ip[NI_MAXHOST], port[NI_MAXSERV];
    int yes = 1;

    typedef struct {
        char ip[16];
        int listenfd;
        int connfd;
        int port;
        int counter;
        int con_counter;
        int epoll;
        struct epoll_event event;
        struct epoll_event * events;
    } PROC_THREAD_PARAMS;

#ifdef	__cplusplus
}
#endif

#endif	/* SERVER_H */

