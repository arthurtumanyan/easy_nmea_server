/*
Copyright [2013] [Arthur Tumanyan <arthurtumanyan@gmail.com]

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
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

