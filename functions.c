#include "easy_nmea_server.h"
#include "server.h"

bool is_valid_opt(char *opts)
{
    if (opts == NULL)return false;
    if (opts[0] == '-' || opts[0] == ':' || opts[0] == '?')return false;

    return true;
}

void setRightOwner(const char *path)
{
    if (NULL == path)return;
    if (hasRightOwner(path))return;
    int sz = strlen(path) + 80;
    char func_msg[sz];
    int rc;
    struct passwd *pw = getpwnam(arguments.user);
    struct group *gr = getgrnam(arguments.group);
    if (NULL == pw || NULL == gr)
    {
        memset(func_msg, '\0', sz);
        snprintf(func_msg, sz, "No such user/group: %s/%s\n", arguments.user, arguments.group);
        writeToCustomLog(func_msg);
        exit(EXIT_FAILURE);
    }
    rc = chown(path, pw->pw_uid, gr->gr_gid);
    if (rc == -1)
    {
        memset(func_msg, '\0', sz);
        snprintf(func_msg, sz, "Cannot chown file/directory '%s',%s", path, strerror(errno));
        writeToCustomLog(func_msg);

    }
}

/**
 *
 * @param path
 * @return
 */
bool hasRightOwner(const char *path)
{
    if (NULL == path)return false;
    stat(path, &status);
    struct passwd *pw = getpwuid(status.st_uid);
    struct group *gr = getgrgid(status.st_gid);
    int i = 0;
    if ((pw != 0) && (0 == strcmp(pw->pw_name, arguments.user)))
    {
        i++;
    }
    if ((gr != 0) && (0 == strcmp(gr->gr_name, arguments.group)))
    {
        i++;
    }
    if (i < 2)
    {
        return false;
    }
    else
    {
        return true;
    }
}

 void print_usage()
{

    printf("%s\t%30s", "-p |--pidfile", "pidfile\n");
    printf("%s\t%29s", "-l |--logdir", "logdir\n");
    printf("%s\t%33s", "-c |--configfile", "configuration file\n");
    printf("%s\t%35s", "-u |--user", "working user\n");
    printf("%s\t%36s", "-g |--group", "working group\n");
    printf("%s\t%69s", "-H |--listen-addr", "IP address to listen for incoming client's connections\n");
    printf("%s\t%26s", "-P |--listen-port", "listen port\n");
    printf("%s\t%32s", "-W |--working-dir", "working directory\n");
    printf("%s\t%71s", "-s |--stat-listen-addr", "IP address to listen for statistic's and control queries\n");
    printf("%s\t%51s", "-e |--stat-listen-port", "listen port (statistics and control)\n");
    printf("%s\t%32s", "-h |--help", "this help\n");

    exit(EXIT_SUCCESS);
}

void * xmalloc(size_t size)
{
    void *new_mem = (void *) malloc(size);

    if (new_mem == NULL)
    {
        fprintf(stderr, "Cannot allocate memory... Dying\n");
        exit(EXIT_FAILURE);
    }

    return new_mem;
}

void * xrealloc(void *ptr, size_t size)
{
    void *new_mem = (void *) realloc(ptr, size);

    if (new_mem == NULL)
    {
        fprintf(stderr, "Cannot allocate memory... Dying\n");
        exit(EXIT_FAILURE);
    }

    return new_mem;
}

void * xcalloc(size_t nmemb, size_t size)
{
    void *new_mem = (void *) calloc(nmemb, size);

    if (new_mem == NULL)
    {
        fprintf(stderr, "Cannot allocate memory... Dying\n");
        exit(EXIT_FAILURE);
    }

    return new_mem;
}

int isValidIP(char *str)
{

    int segs = 0; /* Segment count. */
    int chcnt = 0; /* Character count within segment. */
    int accum = 0; /* Accumulator for segment. */

    /* Catch NULL pointer. */

    if (str == NULL)
        return 0;

    /* Process every character in string. */

    while (*str != '\0')
    {
        /* Segment changeover. */

        if (*str == '.')
        {
            /* Must have some digits in segment. */

            if (chcnt == 0)
                return 0;

            /* Limit number of segments. */

            if (++segs == 4)
                return 0;

            /* Reset segment values and restart loop. */

            chcnt = accum = 0;
            str++;
            continue;
        }

        /* Check numeric. */

        if ((*str < '0') || (*str > '9'))
            return 0;

        /* Accumulate and check segment. */

        if ((accum = accum * 10 + *str - '0') > 255)
            return 0;

        /* Advance other segment specific stuff and continue loop. */

        chcnt++;
        str++;
    }

    /* Check enough segments and enough characters in last segment. */

    if (segs != 3)
        return 0;

    if (chcnt == 0)
        return 0;

    /* Address okay. */

    return 1;
}

/**
 *
 * @param p
 * @return
 */
bool isValidPort(const char * p)
{
    int iport = 0, z = 0;
    if (p == NULL)
    {
        return false;
    }
    if (0 != (z = sscanf(p, "%d", &iport)))
    {
        if (iport > 0 && iport <= 65535)
        {
            return true;
        }
    }
    return false;
}

bool FileExists(char *path)
{
    FILE *fp = fopen(path, "r");
    if (fp)
    {
        fclose(fp);
        return true;
    }
    else
    {
        return false;
    }
}

/**
 *
 * @param pzPath
 * @return
 */
bool DirectoryExists(const char* pzPath)
{
    if (pzPath == NULL) return false;

    DIR *pDir;
    bool bExists = false;

    pDir = opendir(pzPath);

    if (pDir != NULL)
    {
        bExists = true;
        (void) closedir(pDir);
    }

    return bExists;
}

/**
 *
 */
void savePid()
{
    int fsz = sizeof (arguments.pidfile);
    int sz = fsz + 50;
    char func_msg[sz];

    if ((pid_fd = fopen(arguments.pidfile, "w")) == NULL)
    {
        memset(func_msg, '\0', sz);
        snprintf(func_msg, sz, "Cannot create PID file '%s'.Exiting nicely! - %s", arguments.pidfile, strerror(errno));
        writeToCustomLog(func_msg);
        exit(EXIT_FAILURE);
    }
    else
    {
        setRightOwner(arguments.pidfile);
        memset(func_msg, '\0', sz);
        snprintf(func_msg, sz, "Process ID stored in: '%s',%d", arguments.pidfile, errno);
        writeToCustomLog(func_msg);

    }
    fprintf(pid_fd, "%d", getpid());

    if (pid_fd)fclose(pid_fd);

    if (0 != chown(arguments.pidfile, my_uid, my_gid))
    {
        memset(func_msg, '\0', sz);
        snprintf(func_msg, sz, "Cannot chown file '%s',%s", arguments.pidfile, strerror(errno));
        writeToCustomLog(func_msg);

    }
}

/**
 *
 * @return
 */
int checkPid()
{
    pid_t S_PID;
    if ((pid_fd = fopen(arguments.pidfile, "r")) != NULL)
    {
        (void) fscanf(pid_fd, "%d", &S_PID);
        if (pid_fd)fclose(pid_fd);

        if (kill(S_PID, 18) == 0)
        {
            printf("Can be only one running daemon with the same settings!Quitting...\n");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}

/**
 *
 */
void removePid()
{
    int fsz = sizeof (arguments.pidfile);
    int sz = fsz + 64;
    char func_msg[sz];
    if (0 != unlink(arguments.pidfile))
    {
        memset(func_msg, '\0', sz);
        snprintf(func_msg, sz, "Cannot unlink file '%s',%s", arguments.pidfile, strerror(errno));
        writeToCustomLog(func_msg);
    }
    else
    {
        memset(func_msg, '\0', sz);
        snprintf(func_msg, sz, "Pid file '%s' removed", arguments.pidfile);
        writeToCustomLog(func_msg);

    }
}

/**
 *
 * @param str
 */
void low_string(char *str)
{
    unsigned int h;
    for (h = 0; h < strlen(str); h++)str[h] = tolower((int) str[h]);
}

void upper_string(char *str)
{
    unsigned int h;
    for (h = 0; h < strlen(str); h++)str[h] = toupper((int) str[h]);
}
/**
 *
 * @param str
 */
void trim(const char *str)
{
    char *p;

    if ((p = strchr(str, '\r')) != NULL)
    {
        *p = '\0';
    }
    if ((p = strchr(str, '\n')) != NULL)
    {
        *p = '\0';
    }
}

void halt()
{

    int sz = 15;
    char func_msg[sz];

    writeToCustomLog("Closing open handles");
    int i, j;
    for (i = 0; i < globals.sections_cnt; i++)
    {
        for (j = 0; j < globals.maxcon; j++)
        {
            memset(nmea_struct[i][j].fake_string, '\0', 128);
            memset(nmea_struct[i][j].string, '\0', 128);
            nmea_struct[i][j].sz = 0;
        }
    }
    writeToCustomLog("Terminating threads");
    listen_stop_flag = true;
    stat_listen_stop_flag = true;
    sleep(3);
    removePid();
    //
    memset(func_msg, '\0', sz);
    snprintf(func_msg, sz, "%s", "Shutting down");
    writeToCustomLog(func_msg);

    closeCustomLog();
    closeDebugLog();
    closeConLog();
    closeAccessLog();

    if(return_latitude){
        FREE(return_latitude);
    }
    
    if(return_longitude){
        FREE(return_longitude);
    }
    
    if (arguments.pidfile)
    {
        FREE(arguments.pidfile);
        arguments.pidfile = NULL;
    }

    if (arguments.logdir)
    {
        FREE(arguments.logdir);
        arguments.logdir = NULL;
    }

    if (arguments.configfile)
    {
        FREE(arguments.configfile);
        arguments.configfile = NULL;
    }

    if (arguments.user)
    {
        FREE(arguments.user);
        arguments.user = NULL;
    }

    if (arguments.group)
    {
        FREE(arguments.group);
        arguments.group = NULL;
    }

    if (arguments.wdir)
    {
        FREE(arguments.wdir);
        arguments.wdir = NULL;
    }

    if (arguments.listen_addr)
    {
        FREE(arguments.listen_addr);
        arguments.listen_addr = NULL;
    }

    if (arguments.stat_listen_addr)
    {
        FREE(arguments.stat_listen_addr);
        arguments.stat_listen_addr = NULL;
    }

    if (globals.custom_logfile_name)
    {
        FREE(globals.custom_logfile_name);
        globals.custom_logfile_name = NULL;
    }

    if (globals.debug_logfile_name)
    {
        FREE(globals.debug_logfile_name);
        globals.debug_logfile_name = NULL;
    }

    if (globals.connections_logfile_name)
    {
        FREE(globals.connections_logfile_name);
        globals.connections_logfile_name = NULL;
    }

    if (globals.access_logfile_name)
    {
        FREE(globals.access_logfile_name);
        globals.access_logfile_name = NULL;
    }

    exit(EXIT_SUCCESS);
}

int is_allowed_client(char *cip)
{

    int sz = 50;
    char func_msg[sz];
    static int msgc[SECTIONS_MAX_COUNT * MAX_CON_PER_IP];
    int cno = find_proper_client_counter(cip);
    if (-1 == cno)
    {
        if (msgc[cno] < 1)
        {
            memset(func_msg, '\0', sz);
            snprintf(func_msg, sz, "No configuration found for address %s", cip);
            pthread_mutex_lock(&t_error);
            writeToDebugLog(func_msg);
            pthread_mutex_unlock(&t_error);
            msgc[cno]++;
        }
        return -1;
    }

    if (is_client_disabled(cno))
    {
        if (msgc[cno] < 1)
        {
            memset(func_msg, '\0', sz);
            snprintf(func_msg, sz, "Source address %s disabled", cip);
            pthread_mutex_lock(&t_error);
            writeToDebugLog(func_msg);
            pthread_mutex_unlock(&t_error);
            msgc[cno]++;
        }
        return -1;
    }

    return cno;
}
//

int is_client_disabled(int rc)
{
    return (false == globals.sections[rc].enabled);
}
//

int find_proper_client_counter(char *cip)
{
    int i, j;
    int a, b, c, d, last_octet;
    char new_ip[16];

    for (i = 0; i < globals.sections_cnt; i++)
    {
        if (!globals.sections[i].enabled)continue;
        for (j = 0; j < SOURCE_IPNETCNT_PER_CLIENT; j++)
        {

            /* case we have a hostname */
            if (0 == strcmp(globals.sections[i].from[j].netmask, "") && globals.sections[i].from[j].lastoctet_range == 0)
            {
                if (0 == strcmp(globals.sections[i].from[j].address, cip))
                {
                    return i;
                }
            }
            /* case we have an ip range */
            if (0 == strcmp(globals.sections[i].from[j].netmask, "") && globals.sections[i].from[j].lastoctet_range != 0)
            {
                if (sscanf(globals.sections[i].from[j].address, "%d.%d.%d.%d", &a, &b, &c, &d) == 4)
                {
                    for (last_octet = d; last_octet <= globals.sections[i].from[j].lastoctet_range; last_octet++)
                    {
                        snprintf(new_ip, 16, "%d.%d.%d.%d", a, b, c, last_octet);
                        if (0 == strcmp(new_ip, cip))
                        {
                            return i;
                        }
                    }
                }
            }
            /* case we have a subnet */
            if (0 != strcmp(globals.sections[i].from[j].netmask, "") && globals.sections[i].from[j].lastoctet_range == 0)
            {
                if (IsIPInRange(cip, globals.sections[i].from[j].address, globals.sections[i].from[j].netmask))
                {
                    return i;
                }
            }
        }
    }
    return -1;
}

int checksum(char *ss)
{
    int c = 0;

    while (*ss)
        c ^= *ss++;

    return c;
}
