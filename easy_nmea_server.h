#ifndef E_NMEA_H
#define	E_NMEA_H

#ifdef	__cplusplus
extern "C" {
#endif

#define FREE(ptr) do{ \
    free((ptr));      \
    (ptr) = NULL;     \
  }while(0)

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <error.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/file.h>     /*  socket definitions        */
#include <sys/types.h>        /*  socket types              */
#include <arpa/inet.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <fcntl.h>
#include <syslog.h>
#include <signal.h>
#include <time.h>
#include <sys/file.h>
#include <syslog.h>
#include <pwd.h>
#include <grp.h>
#include <getopt.h>
#include <sys/epoll.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <pthread.h>
#include <netdb.h>
#include <dirent.h>
#include <regex.h>
#include <libconfig.h>
#include "prototypes.h"

#define no_argument            0
#define required_argument      1
#define optional_argument      2

#define DEFAULT_LISTEN_IP       "127.0.0.1"
#define DEFAULT_LISTEN_PORT     1234
#define MAXLINE 1024
#define CFG_PARAM_LEN 64  
#define SECTIONS_MAX_COUNT 64  
#define MAX_CON_PER_IP  64
#define SOURCE_IPNETCNT_PER_CLIENT 64

#if (LIBCONFIG_VER_MAJOR == 1 && LIBCONFIG_VER_MINOR >= 4) 
    typedef int enmea_int;
#else
    typedef long enmea_int;
#endif

    struct {
        char *pidfile;
        char *logdir;
        char *configfile;
        char *user;
        char *group;
        char *wdir;
        char *listen_addr;
        char *stat_listen_addr;
        int stat_listen_port;
        int listen_port;
    } arguments;

    struct _active_connections {
        char ip[16];
        int port; /* for further needs */
        int connfd;
        int conno;
    } **active_connections;

    static const struct option longOpts[] = {
        { "pidfile", required_argument, NULL, 'p'},
        { "logdir", required_argument, NULL, 'l'},
        { "configfile", required_argument, NULL, 'c'},
        { "user", required_argument, NULL, 'u'},
        { "group", required_argument, NULL, 'g'},
        { "working-dir", required_argument, NULL, 'W'},
        { "listen-addr", required_argument, NULL, 'H'},
        { "listen-port", required_argument, NULL, 'P'},
        { "stat-listen-addr", required_argument, NULL, 's'},
        { "stat-listen-port", required_argument, NULL, 'e'},
        { "help", no_argument, NULL, 'h'},


        { NULL, no_argument, NULL, 0}
    };

    typedef enum {
        SHUFFLE, INCREMENT, DECREMENT
    } _sim_methods;

    typedef enum {
        GPS, GLONASS, BOTH
    } _navigator_sat;
    //
    const char * navigator_sat_tbl[3] = {"GP", "GL", "GN"};
    //

    typedef enum {
        A, D, E, M, S, N
    } _faa_modes_indicator;
    //
    const char * faa_modes_indicator_tbl[6] = {"A", "D", "E", "M", "S", "N"};
    //

    typedef enum {
        GLL, RMC, VTG, ZDA, GGA, GSA, GSV, XTE, RMB, DTM
    } _sentences;
    //
    const char * sentences_tbl[10] = {"GLL", "RMC", "VTG", "ZDA", "GGA", "GSA", "GSV", "XTE", "RMB", "DTM"};
    //

    typedef enum {
        A_VALID, V_INVALID
    } _data_status;
    //
    const char * data_status_tbl[2] = {"A", "V"};
    //

    typedef struct {
        int maxcon;
        int sections_cnt;
        int use_syslog;
        int use_resolver;
        char identline[64];
        char * custom_logfile_name;
        char * debug_logfile_name;
        char * connections_logfile_name;
        char * access_logfile_name;

        struct _sections {
            int id;
            int enabled;
            int checksum;

            struct {
                char address[16];
                char netmask[16];
                int lastoctet_range;
            } from[SOURCE_IPNETCNT_PER_CLIENT];
            char longitude[14];
            char latitude[13];
            int simulate;
            double speed;
            _navigator_sat navigator_sat;
            _faa_modes_indicator faa_modes_indicator;
            _sentences sentence;
            _data_status data_status;
            _sim_methods sim_methods;
        } sections[SECTIONS_MAX_COUNT];
    } GLOBALS;
    //
    char TMP_MSG[MAXLINE];

    struct passwd *pwd;
    struct group *grp;
    struct stat status;
    uid_t my_uid;
    gid_t my_gid;
    pid_t pid, sid;
    off_t file_size;

    pthread_t stat_listen_thread;
    pthread_t p_child;

    int childres, stat_listenres;
    int childtid = 1, stat_listentid = 2;

    pthread_mutex_t t_mutex, t_error;

    bool listen_stop_flag;
    bool stat_listen_stop_flag;

    time_t ticks;
    DIR *buf_dir;
    struct dirent *entry;
    struct stat statbuf;

    FILE * access_fd = NULL;
    FILE * custom_fd = NULL;
    FILE * debug_fd = NULL;
    FILE * conlog_fd = NULL;
    FILE * pid_fd = NULL;
    //
    GLOBALS globals;

    char * return_latitude;
    char * return_longitude;
    
#ifdef	__cplusplus
}
#endif

#endif	/* E_NMEA_H */

