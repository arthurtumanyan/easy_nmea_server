#include "easy_nmea_server.h"
#include "server.h"

static int raw_connection_count = 0;
static int raw_workers_cnt = 0;
static int incoming_con[SECTIONS_MAX_COUNT];

int create_and_bind(char *bip, int bport)
{

    int sz = 128;
    char network_msg[sz];
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof (servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(bip);
    servaddr.sin_port = htons(bport);

    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (yes)) < 0)
    {
        memset(network_msg, '\0', sz);
        snprintf(network_msg, sz, "SETSOCKOPT(): %s", strerror(errno));
        writeToCustomLog(network_msg);
        halt();
    }

    if (0 != bind(listenfd, (struct sockaddr *) &servaddr, sizeof (servaddr)))
    {
        memset(network_msg, '\0', sz);
        snprintf(network_msg, sz, "Can not bind on %s: %s", arguments.listen_addr, strerror(errno));
        writeToCustomLog(network_msg);
        halt();
    }
    return listenfd;
}

void do_listen()
{

    int sz = 128;
    int rc = 0, f = 0;
    char network_msg[sz];
    int pth_ret = 0;

    nmea_struct = xmalloc(globals.sections_cnt * sizeof (struct _nms *));
    for (f = 0; f < globals.sections_cnt; f++)
    {
        nmea_struct[f] = xmalloc(globals.maxcon * sizeof (struct _nms));
        incoming_con[f] = 0;
    }

    /* Buffer where events are returned */
    events = calloc(globals.maxcon, sizeof event);

    listenfd = create_and_bind(arguments.listen_addr, arguments.listen_port);

    if (0 != setnonblocking(listenfd))
    {
        memset(network_msg, '\0', sz);
        snprintf(network_msg, sz, "Can not make non-blocking socket: - %s", strerror(errno));
        writeToCustomLog(network_msg);
        halt();
    }

    if (-1 == listen(listenfd, globals.maxcon))
    {
        memset(network_msg, '\0', sz);
        snprintf(network_msg, sz, "Can not listen on %s: %s", arguments.listen_addr, strerror(errno));
        writeToCustomLog(network_msg);
        halt();
    }
    memset(network_msg, '\0', sz);
    snprintf(network_msg, sz, "Binding on %s:%d", arguments.listen_addr, arguments.listen_port);
    writeToCustomLog(network_msg);
    memset(network_msg, '\0', sz);
    snprintf(network_msg, sz, "Configured to serve maximum %d client%s", globals.maxcon, globals.maxcon > 1 ? "s" : "");
    writeToCustomLog(network_msg);

    efd = epoll_create(globals.maxcon);
    if (-1 == efd)
    {
        memset(network_msg, '\0', sz);
        snprintf(network_msg, sz, "Error with epoll initialization: - %s", strerror(errno));
        writeToCustomLog(network_msg);
        halt();
    }
    //
    event.data.fd = listenfd;
    event.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP;
    //
    if (-1 == epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &event))
    {
        memset(network_msg, '\0', sz);
        snprintf(network_msg, sz, "Error with epoll_ctl: - %s", strerror(errno));
        writeToCustomLog(network_msg);
        halt();
    }

    //
    while (!listen_stop_flag)
    {
        int n, i;
        n = epoll_wait(efd, events, globals.maxcon, EPOLL_TIMEOUT);
        for (i = 0; i < n; i++)
        {
            if (listenfd == events[i].data.fd)
            {

                in_len = sizeof in_addr;
                connfd = accept(listenfd, &in_addr, &in_len);
                if (connfd == -1)
                {
                    if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
                    {
                        /* We have processed all incoming
                           connections. */
                        continue;
                    }
                    else
                    {
                        memset(network_msg, '\0', sz);
                        snprintf(network_msg, sz, "Can not accept: %s", strerror(errno));
                        writeToCustomLog(network_msg);
                        continue;
                    }
                }

                if (raw_connection_count == globals.maxcon)
                {
                    memset(network_msg, '\0', sz);
                    snprintf(network_msg, sz, "Connections count limit exceeded [%d]", raw_connection_count);
                    writeToCustomLog(network_msg);
                    close(connfd);
                    continue;
                }

                if (0 == getnameinfo(&in_addr, in_len, ip, sizeof ip, port, sizeof port, NI_NUMERICHOST | NI_NUMERICSERV))
                {
                    memset(network_msg, '\0', sz);
                    snprintf(network_msg, sz, "Connection attempt from %s,port %d", ip, atoi(port));
                    writeToConLog(network_msg);
                }

                if (-1 == (rc = is_allowed_client(ip)))
                {
                    Writeline(connfd, reject_msg, strlen(reject_msg));
                    if (0 > close(connfd))
                    {
                        memset(network_msg, '\0', sz);
                        snprintf(network_msg, sz, "Error %s", strerror(errno));
                        writeToCustomLog(network_msg);
                    }
                    continue;
                }
                //
                if (incoming_con[rc] == MAX_CON_PER_IP)
                {
                    memset(network_msg, '\0', sz);
                    snprintf(network_msg, sz, "Connection's count limit per client reached [%d]: dropping connection", MAX_CON_PER_IP);
                    writeToCustomLog(network_msg);
                    close(connfd);
                    continue;
                }
                //
                if (raw_workers_cnt == globals.maxcon)
                {
                    memset(network_msg, '\0', sz);
                    snprintf(network_msg, sz, "You are about to reach worker's count limit [%d]: dropping connection", globals.maxcon);
                    writeToCustomLog(network_msg);
                    close(connfd);
                    continue;
                }

                if (0 != setnonblocking(connfd))
                {
                    memset(network_msg, '\0', sz);
                    snprintf(network_msg, sz, "Can not make non-blocking socket: - %s", strerror(errno));
                    writeToCustomLog(network_msg);
                    halt();
                }
                event.data.fd = connfd;
                event.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP;

                if (0 > epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &event))
                {
                    memset(network_msg, '\0', sz);
                    snprintf(network_msg, sz, "Error with epoll_ctl: - %s", strerror(errno));
                    writeToCustomLog(network_msg);
                    continue;
                }
                //
                raw_connection_count++;
                incoming_con[rc]++;
                memset(network_msg, '\0', sz);
                snprintf(network_msg, sz, "Connection %d established from %s,port %d", raw_connection_count, ip, atoi(port));
                writeToConLog(network_msg);

                memset(network_msg, '\0', sz);
                snprintf(network_msg, sz, "Connections count = %d, %d connection(s) for client id %d", raw_connection_count, incoming_con[rc], globals.sections[rc].id);
                writeToConLog(network_msg);

                PROC_THREAD_PARAMS param;
                snprintf(param.ip, 16, "%s", ip);
                param.port = atoi(port);
                param.connfd = connfd;
                param.counter = rc;
                param.con_counter = DEF_CON_COUNTER;
                param.epoll = efd;
                param.events = &event;

                pth_ret = pthread_create(&p_child, NULL, spawn_child_processor, &param);
                if (0 == pth_ret)
                {
                    raw_workers_cnt++;
                    memset(network_msg, '\0', sz);
                    snprintf(network_msg, sz, "Workers count: %d", raw_workers_cnt);
                    writeToCustomLog(network_msg);
                }
                else
                {
                    memset(network_msg, '\0', sz);
                    snprintf(network_msg, sz, "Cannot create a thread: %s", strerror(errno));
                    writeToCustomLog(network_msg);
                }

                //
            } /* if listenfd */
        } /* for */
        usleep(10);
    } /* while */

    free(events);
    close(listenfd);
}

void set_thread_signalmask(sigset_t SignalSet)
{

    sigemptyset(&SignalSet);
    sigaddset(&SignalSet, SIGINT);
    sigaddset(&SignalSet, SIGHUP);
    sigaddset(&SignalSet, SIGUSR1);
    sigaddset(&SignalSet, SIGUSR2);
    sigaddset(&SignalSet, SIGTRAP);
    sigaddset(&SignalSet, SIGCHLD);
    sigaddset(&SignalSet, SIGTSTP);
    sigaddset(&SignalSet, SIGTTOU);
    sigaddset(&SignalSet, SIGTTIN);
    sigaddset(&SignalSet, SIGABRT);
    sigaddset(&SignalSet, SIGPIPE);
    sigaddset(&SignalSet, SIGALRM);
    sigaddset(&SignalSet, SIGSEGV);
    sigaddset(&SignalSet, SIGBUS);

    pthread_sigmask(SIG_BLOCK, &SignalSet, NULL);
}

void * spawn_child_processor(void * ptr)
{
    sigset_t epoll_sigset;
    int sz = 128;
    char network_msg[sz];
    pid_t tid = syscall(__NR_gettid);
    PROC_THREAD_PARAMS params = *((PROC_THREAD_PARAMS*) ptr);
    int fd = params.connfd;
    int rc = params.counter;
    int epollfd = params.epoll;
    int done = 0;
    unsigned int usec = 0;

    struct timeval timeout, tv;

    timeout.tv_sec = EPOLL_TIMEOUT;
    timeout.tv_usec = 0;

    if (rc == SECTIONS_MAX_COUNT && fd == MAX_CON_PER_IP)
    {
        pthread_mutex_lock(&t_error);
        memset(network_msg, '\0', sz);
        snprintf(network_msg, sz, "Handle [%d][%d] is reserved for internal use", rc, fd);
        writeToCustomLog(network_msg);
        pthread_mutex_unlock(&t_error);
        pthread_exit((void *) 0);
    }

    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof (timeout)) < 0)
    {
        pthread_mutex_lock(&t_error);
        memset(network_msg, '\0', sz);
        snprintf(network_msg, sz, "SETSOCKOPT(): %s", strerror(errno));
        writeToCustomLog(network_msg);
        pthread_mutex_unlock(&t_error);
    }

    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeout, sizeof (timeout)) < 0)
    {
        pthread_mutex_lock(&t_error);
        memset(network_msg, '\0', sz);
        snprintf(network_msg, sz, "SETSOCKOPT(): %s", strerror(errno));
        writeToCustomLog(network_msg);
        pthread_mutex_unlock(&t_error);
    }
    pthread_mutex_lock(&t_error);
    memset(network_msg, '\0', sz);
    snprintf(network_msg, sz, "Starting thread ID [%d] to serve client [id: %d]", tid, globals.sections[params.counter].id);
    writeToDebugLog(network_msg);
    pthread_mutex_unlock(&t_error);

    set_thread_signalmask(epoll_sigset);

    if (0 == pthread_detach(pthread_self()))
    {
        pthread_mutex_lock(&t_error);
        memset(network_msg, '\0', sz);
        snprintf(network_msg, sz, "Thread ID [%d] detached", tid);
        writeToDebugLog(network_msg);
        pthread_mutex_unlock(&t_error);
    }
    //
    pthread_mutex_lock(&t_error);
    save_active_connection(params.ip, params.port, rc, fd);
    pthread_mutex_unlock(&t_error);

    while (1)
    {

        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            continue;
        }
        else if (errno == ECONNRESET || errno != 0)
        {
            pthread_mutex_lock(&t_error);
            memset(network_msg, '\0', sz);
            snprintf(network_msg, sz, "Connection on descriptor %d [from %s,port %d] aborted", fd, params.ip, params.port);
            writeToConLog(network_msg);
            done = 1;
            pthread_mutex_unlock(&t_error);
            break;
        }
        //

        memset(nmea_struct[rc][fd].fake_string, '\0', 128);
        memset(nmea_struct[rc][fd].string, '\0', 128);
        gettimeofday(&tv, NULL);

        usec = (tv.tv_usec / 1000);
        if ((usec * 10) > 1000)
        {
            usec = usec / 10;
        }
        snprintf(nmea_struct[rc][fd].fake_string, sizeof(nmea_struct[rc][fd].fake_string), "%s%s,%s,%s,%ld.%u,%s,%s",
                 navigator_sat_tbl[globals.sections[rc].navigator_sat],
                 sentences_tbl[globals.sections[rc].sentence],
                 globals.sections[rc].latitude,
                 globals.sections[rc].longitude,
                 tv.tv_sec,
                 usec,
                 data_status_tbl[globals.sections[rc].data_status],
                 faa_modes_indicator_tbl[globals.sections[rc].faa_modes_indicator]);

        snprintf(nmea_struct[rc][fd].string, strlen(nmea_struct[rc][fd].fake_string) + ((1 == globals.sections[rc].checksum)?strlen(format_gpgll_chk):strlen(format_gpgll) +1),
                 (1 == globals.sections[rc].checksum) ? format_gpgll_chk : format_gpgll,
                 nmea_struct[rc][fd].fake_string,
                 (1 == globals.sections[rc].checksum) ? checksum(nmea_struct[rc][fd].fake_string) : '\0');

        nmea_struct[rc][fd].sz = strlen(nmea_struct[rc][fd].string);
        Writeline(fd, nmea_struct[rc][fd].string, nmea_struct[rc][fd].sz);
        //
        usleep(1000000 * globals.sections[rc].speed);

    } /* while */

    if (done)
    {
        if (0 > epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &event))
        {
            pthread_mutex_lock(&t_error);
            memset(network_msg, '\0', sz);
            snprintf(network_msg, sz, "Error with epoll_ctl: - %s", strerror(errno));
            writeToCustomLog(network_msg);
            pthread_mutex_unlock(&t_error);
        }
        close(fd);

    }

    pthread_mutex_lock(&t_error);
    deactivate_connection(rc, fd);

    raw_connection_count--;
    incoming_con[rc]--;
    memset(network_msg, '\0', sz);
    snprintf(network_msg, sz, "Connections count = %d, %d connection(s) for client id %d", raw_connection_count, incoming_con[rc], globals.sections[rc].id);
    writeToConLog(network_msg);

    memset(network_msg, '\0', sz);
    snprintf(network_msg, sz, "Terminating thread ID [%d]", tid);
    writeToDebugLog(network_msg);

    raw_workers_cnt--;

    memset(network_msg, '\0', sz);
    snprintf(network_msg, sz, "Workers count: %d", raw_workers_cnt);
    writeToCustomLog(network_msg);
    pthread_mutex_unlock(&t_error);
    //
    pthread_exit((void *) 0);

}

