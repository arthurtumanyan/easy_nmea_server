#include "easy_nmea_server.h"

//

int main(int argc, char** argv)
{

    /* Initialize arguments before we get to work. */
    char * argument = NULL;
    int opt = 0;
    int longIndex = 0;
    char *host = NULL;

    argument = "/var/run/enmead.pid";
    arguments.pidfile = xmalloc(sizeof (char) * strlen(argument) + 1);
    strcpy(arguments.pidfile, argument);

    argument = "/var/log/enmead";
    arguments.logdir = xmalloc(sizeof (char) * strlen(argument) + 1);
    strcpy(arguments.logdir, argument);

    argument = "/etc/enmead/enmea.conf";
    arguments.configfile = xmalloc(sizeof (char) * strlen(argument) + 1);
    strcpy(arguments.configfile, argument);

    argument = "enmead";
    arguments.user = xmalloc(sizeof (char) * strlen(argument) + 1);
    strcpy(arguments.user, argument);

    argument = "enmead";
    arguments.group = xmalloc(sizeof (char) * strlen(argument) + 1);
    strcpy(arguments.group, argument);

    argument = "/tmp";
    arguments.wdir = xmalloc(sizeof (char) * strlen(argument) + 1);
    strcpy(arguments.wdir, argument);

    argument = DEFAULT_LISTEN_IP;
    arguments.listen_addr = xmalloc(sizeof (char) * strlen(argument) + 1);
    strcpy(arguments.listen_addr, argument);

    arguments.listen_port = DEFAULT_LISTEN_PORT;

    argument = DEFAULT_LISTEN_IP;
    arguments.stat_listen_addr = xmalloc(sizeof (char) * strlen(argument) + 1);
    strcpy(arguments.stat_listen_addr, argument);

    arguments.stat_listen_port = 8088;

    argument = "custom.log";
    globals.custom_logfile_name = xmalloc(sizeof (char) * strlen(argument) + 1);
    strcpy(globals.custom_logfile_name, argument);

    argument = "debug.log";
    globals.debug_logfile_name = xmalloc(sizeof (char) * strlen(argument) + 1);
    strcpy(globals.debug_logfile_name, argument);


    argument = "connections.log";
    globals.connections_logfile_name = xmalloc(sizeof (char) * strlen(argument) + 1);
    strcpy(globals.connections_logfile_name, argument);

    argument = "access.log";
    globals.access_logfile_name = xmalloc(sizeof (char) * strlen(argument) + 1);
    strcpy(globals.access_logfile_name, argument);

    return_latitude = xmalloc(sizeof (char) * 13);
    return_longitude = xmalloc(sizeof (char) * 14);

    globals.use_resolver = 0;
    globals.use_syslog = 0;
    globals.maxcon = 4096;
    globals.sections_cnt = 1;
    snprintf(globals.identline, 12, "%s", "NMEA server");

    set_sig_handler();
    static const char *optString = "p:l:c:u:g:W:H:P:s:e:h?";
    opt = getopt_long(argc, argv, optString, longOpts, &longIndex);

    while (opt != -1)
    {
        switch (opt)
        {
        case 'p':
            if (!is_valid_opt(optarg))
            {
                print_usage();
            }
            arguments.pidfile = xrealloc(arguments.pidfile, (sizeof (char) * strlen(optarg) + 1));
            strcpy(arguments.pidfile, optarg);

            break;

        case 'l':
            if (!is_valid_opt(optarg))
            {
                print_usage();
            }
            arguments.logdir = xrealloc(arguments.pidfile, (sizeof (char) * strlen(optarg) + 1));
            strcpy(arguments.logdir, optarg);
            break;

        case 'c':
            if (!is_valid_opt(optarg))
            {
                print_usage();
            }
            arguments.configfile = xrealloc(arguments.configfile, (sizeof (char) * strlen(optarg) + 1));
            strcpy(arguments.configfile, optarg);
            break;
        case 'u':
            if (!is_valid_opt(optarg))
            {
                print_usage();
            }
            arguments.user = xrealloc(arguments.user, (sizeof (char) * strlen(optarg) + 1));
            strcpy(arguments.user, optarg);
            break;
            
        case 'g':
            arguments.group = xrealloc(arguments.group, (sizeof (char) * strlen(optarg) + 1));
            strcpy(arguments.group, optarg);
            break;
            
        case 'H':
            if (!isValidIP(optarg))
            {
                print_usage();
            }
            if (NULL != (host = nslookup(optarg)))
            {
                arguments.listen_addr = xrealloc(arguments.listen_addr, (sizeof (char) * strlen(optarg) + 1));
                strcpy(arguments.listen_addr, host);
            }
            else
            {
                printf("%s\n", hstrerror(h_errno));
                exit(EXIT_FAILURE);
            }
            break;
        case 'P':
            if (!is_valid_opt(optarg))
            {
                print_usage();
            }
            if (isValidPort(optarg))
            {
                arguments.listen_port = atoi(optarg);
            }
            else
            {
                print_usage();
            }
            break;

        case 'W':
            if (!is_valid_opt(optarg))
            {
                print_usage();
            }
            arguments.wdir = xrealloc(arguments.wdir, (sizeof (char) * strlen(optarg) + 1));
            strcpy(arguments.wdir, optarg);
            break;
        case 's':
            if (!isValidIP(optarg))
            {
                print_usage();
            }
            if (NULL != (host = nslookup(optarg)))
            {
                arguments.stat_listen_addr = xrealloc(arguments.stat_listen_addr, (sizeof (char) * strlen(optarg) + 1));
                strcpy(arguments.stat_listen_addr, host);
            }
            else
            {
                printf("%s\n", hstrerror(h_errno));
                exit(EXIT_FAILURE);
            }
            break;
        case 'e':
            if (!is_valid_opt(optarg))
            {
                print_usage();
            }
            if (isValidPort(optarg))
            {
                arguments.stat_listen_port = atoi(optarg);
            }
            else
            {
                print_usage();
            }
            break;

        default:
            if (!is_valid_opt(optarg))
            {
                print_usage();
            }
            break;
        }

        opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
    }

    if (false == FileExists(arguments.configfile))
    {
        memset(TMP_MSG, '\0', MAXLINE);
        snprintf(TMP_MSG, MAXLINE, "Configuration file '%s' does not exists\n", arguments.configfile);
        writeToCustomLog(TMP_MSG);
        exit(EXIT_FAILURE);
    }
    else if (true == DirectoryExists(arguments.configfile))
    {
        memset(TMP_MSG, '\0', MAXLINE);
        snprintf(TMP_MSG, MAXLINE, "Specified file '%s' is a directory\n", arguments.configfile);
        writeToCustomLog(TMP_MSG);
        exit(EXIT_FAILURE);
    }
    else
    {
        setRightOwner(arguments.configfile);
        if (false == hasRightOwner(arguments.configfile))
        {
            memset(TMP_MSG, '\0', MAXLINE);
            snprintf(TMP_MSG, MAXLINE, "The file '%s' has invalid owner/group", arguments.configfile);
            writeToCustomLog(TMP_MSG);
            memset(TMP_MSG, '\0', MAXLINE);
            snprintf(TMP_MSG, MAXLINE, "Should be %s:%s", arguments.user, arguments.group);
            writeToCustomLog(TMP_MSG);
            exit(EXIT_FAILURE);
        }
    }

    if (true == DirectoryExists(arguments.pidfile))
    {
        memset(TMP_MSG, '\0', MAXLINE);
        snprintf(TMP_MSG, MAXLINE, "Specified pid file '%s' is a directory", arguments.pidfile);
        writeToCustomLog(TMP_MSG);
        exit(EXIT_FAILURE);
    }

    if (false == DirectoryExists(arguments.wdir))
    {
        memset(TMP_MSG, '\0', MAXLINE);
        snprintf(TMP_MSG, MAXLINE, "Specified working directory '%s' does not exists or not a directory", arguments.wdir);
        writeToCustomLog(TMP_MSG);
        exit(EXIT_FAILURE);
    }

    if (false == DirectoryExists(arguments.logdir))
    {
        memset(TMP_MSG, '\0', MAXLINE);
        snprintf(TMP_MSG, MAXLINE, "Specified log directory '%s' does not exists", arguments.logdir);
        writeToCustomLog(TMP_MSG);
        memset(TMP_MSG, '\0', MAXLINE);
        snprintf(TMP_MSG, MAXLINE, "Creating new one\n");
        writeToCustomLog(TMP_MSG);
        if (0 > mkdir(arguments.logdir, 0755))
        {
            memset(TMP_MSG, '\0', MAXLINE);
            snprintf(TMP_MSG, MAXLINE, "Can not create log directory - %s", strerror(errno));
            writeToCustomLog(TMP_MSG);
            exit(EXIT_FAILURE);
        }
        else
        {
            setRightOwner(arguments.logdir);
        }
    }
    setRightOwner(arguments.logdir);

    if (true != hasRightOwner(arguments.logdir))
    {
        memset(TMP_MSG, '\0', MAXLINE);
        snprintf(TMP_MSG, MAXLINE, "The file '%s' has invalid owner/group", arguments.logdir);
        writeToCustomLog(TMP_MSG);
        memset(TMP_MSG, '\0', MAXLINE);
        snprintf(TMP_MSG, MAXLINE, "Should be %s:%s", arguments.user, arguments.group);
        writeToCustomLog(TMP_MSG);
        exit(EXIT_FAILURE);
    }
    //
    if (!(pwd = getpwnam(arguments.user)))
    {
        memset(TMP_MSG, '\0', MAXLINE);
        snprintf(TMP_MSG, MAXLINE, "No such user: %s. Quitting!", arguments.user);
        writeToCustomLog(TMP_MSG);

        exit(EXIT_FAILURE);
    }
    else my_uid = pwd->pw_uid;

    if (!(grp = getgrnam(arguments.group)))
    {
        memset(TMP_MSG, '\0', MAXLINE);
        snprintf(TMP_MSG, MAXLINE, "No such group: %s. Quitting!", arguments.group);
        writeToCustomLog(TMP_MSG);
        exit(EXIT_FAILURE);
    }
    else my_gid = grp->gr_gid;

    readConfig(arguments.configfile);
    checkPid();
    openCustomLog();
    openDebugLog();
    openConLog();
    openAccessLog();

    //    pid = fork();
    //
    //    if (pid < 0)
    //    {
    //        snprintf(TMP_MSG, MAXLINE, "%s", "Can not fork! [Invalid PID]");
    //        writeToCustomLog(TMP_MSG);
    //        removePid();
    //        exit(EXIT_FAILURE);
    //    }
    //    /* If we got a good PID, then
    //       we can exit the parent process. */
    //    if (pid > 0)
    //    {
    //        exit(EXIT_SUCCESS);
    //    }
    //    //
    //    snprintf(TMP_MSG, MAXLINE, "%s", "Forked successfully");
    //    writeToCustomLog(TMP_MSG);
    //    /* Change the file mode mask */
    //    umask(027);
    //    /* Create a new SID for the child process */
    //    sid = setsid();
    //    if (sid < 0)
    //    {
    //        snprintf(TMP_MSG, MAXLINE, "%s", "Can not create child process");
    //        writeToCustomLog(TMP_MSG);
    //        /* Log the failure */
    //        removePid();
    //        exit(EXIT_FAILURE);
    //    }
    //    snprintf(TMP_MSG, MAXLINE, "%s", "Daemonizing");
    //    writeToCustomLog(TMP_MSG);
    //
    //    /* Change the current working directory */
    //    if ((chdir(arguments.wdir)) < 0)
    //    {
    //        snprintf(TMP_MSG, MAXLINE, "%s", "Can not change current directory");
    //        writeToCustomLog(TMP_MSG);
    //        /* Log the failure */
    //        exit(EXIT_FAILURE);
    //    }
    //    snprintf(TMP_MSG, MAXLINE, "%s", "Working directory changed");
    //    writeToCustomLog(TMP_MSG);
    //
    //    savePid();
    //
    //    if ((setgid(my_gid)) < 0)
    //    {
    //        snprintf(TMP_MSG, MAXLINE, "ERROR: setgid(%d) failed: %s", my_gid, strerror(errno));
    //        writeToCustomLog(TMP_MSG);
    //        halt();
    //    }
    //    snprintf(TMP_MSG, MAXLINE, "Group ID changed to %d", my_gid);
    //    writeToCustomLog(TMP_MSG);
    //
    //    if ((setuid(my_uid)) < 0)
    //    {
    //        snprintf(TMP_MSG, MAXLINE, "ERROR: setuid(%d) failed: %s", my_uid, strerror(errno));
    //        writeToCustomLog(TMP_MSG);
    //        halt();
    //    }
    //    snprintf(TMP_MSG, MAXLINE, "User ID changed to %d", my_gid);
    //    writeToCustomLog(TMP_MSG);
    //
    //
    //    /* Close out the standard file descriptors */
    //    close(STDIN_FILENO);
    //    close(STDOUT_FILENO);
    //    close(STDERR_FILENO);
    //
    init_active_c_table();
    do_listen();
    halt();
    return (EXIT_SUCCESS);
}

/* Many thanks to R.Stevens */

typedef void Sigfunc(int);

Sigfunc * signal(int signo, Sigfunc *func)
{
    struct sigaction act, oact;
    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (signo == SIGALRM)
    {

        act.sa_flags |= SA_INTERRUPT; /* SunOS 4.x */

    }
    else
    {

        act.sa_flags |= SA_RESTART; /* SVR4, 44BSD */

    }
    if (sigaction(signo, &act, &oact) < 0)
        return (SIG_ERR);
    return (oact.sa_handler);
}

/* end signal */

Sigfunc * Signal(int signo, Sigfunc *func) /* for our signal() function */
{
    Sigfunc *sigfunc;

    if ((sigfunc = signal(signo, func)) == SIG_ERR)
    {
        // todo
    }
    return (sigfunc);
}

void signal_hup(int sig)
{
    char msg[64];
    memset(msg, '\0', 64);
    snprintf(msg, 64, "Caught signal %d", sig);
    writeToCustomLog(msg);
    snprintf(msg, 64, "Reading config file");
    writeToCustomLog(msg);
    readConfig(arguments.configfile);
}

void signal_term(int sig)
{
    char msg[64];
    memset(msg, '\0', 64);
    snprintf(msg, 64, "Caught signal %d", sig);
    writeToCustomLog(msg);
    halt();
}

void set_sig_handler()
{

    Signal(SIGINT, signal_term);
    Signal(SIGHUP, signal_hup);
    Signal(SIGUSR1, SIG_IGN);
    Signal(SIGUSR2, SIG_IGN);
    Signal(SIGTRAP, SIG_IGN);
    Signal(SIGCHLD, SIG_IGN);
    Signal(SIGTSTP, SIG_IGN);
    Signal(SIGTTOU, SIG_IGN);
    Signal(SIGTTIN, SIG_IGN);
    Signal(SIGABRT, SIG_IGN);
    Signal(SIGPIPE, SIG_IGN);
    Signal(SIGALRM, SIG_IGN);
    Signal(SIGSEGV, SIG_IGN);
    Signal(SIGBUS, SIG_IGN);
    Signal(SIGWINCH, SIG_IGN);
    Signal(SIGTERM, signal_term);
}
