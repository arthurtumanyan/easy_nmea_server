#include "easy_nmea_server.h"
#include "prototypes.h"

ssize_t Readline(int sockd, void *vptr, size_t maxlen)
{
    ssize_t n, rc;
    char c, *buffer;

    buffer = vptr;

    for (n = 1; n < maxlen; n++)
    {

        if ((rc = read(sockd, &c, 1)) == 1)
        {
            *buffer++ = c;
            if (c == '\n')
                break;
        }
        else if (rc == 0)
        {
            if (n == 1)
                return 0;
            else
                break;
        }
        else
        {
            if (errno == EINTR)
                continue;
            return -1;
        }
    }

    *buffer = 0;
    return n;
}

/*  Write a line to a socket  */

ssize_t Writeline(int sockd, const void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char *buffer;

    buffer = vptr;
    nleft = n;

    while (nleft > 0)
    {
        if ((nwritten = write(sockd, buffer, nleft)) <= 0)
        {
            if (errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }
        nleft -= nwritten;
        buffer += nwritten;
    }

    return n;
}

char * nslookup(char *hostname)
{

    int sz = 128;
    char func_msg[sz];
    static char ipaddr[16];

    if (!globals.use_resolver)
    {
        return hostname;
    }

    if (isValidIP(hostname))
    {
        return hostname;
    }
    struct addrinfo hints, *res;
    struct in_addr addr;
    int err;

    memset(&hints, 0, sizeof (hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    if ((err = getaddrinfo(hostname, NULL, &hints, &res)) != 0)
    {
        memset(func_msg, '\0', sz);
        snprintf(func_msg, sz, "GETADDRINFO error for host: %s: %d %s", hostname, err, strerror(errno));
        pthread_mutex_lock(&t_error);
        writeToCustomLog(func_msg);
        pthread_mutex_unlock(&t_error);
        return NULL;
    }

    addr.s_addr = ((struct sockaddr_in *) (res->ai_addr))->sin_addr.s_addr;
    if (res)freeaddrinfo(res);
    snprintf(ipaddr, sizeof (ipaddr), "%s", inet_ntoa(addr));
    return ipaddr;
}

int setnonblocking(int sockfd)
{
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK | O_ASYNC);
    return 0;
}

uint32_t IPToUInt(char * ipaddr)
{
    int a, b, c, d;
    uint32_t addr = 0;

    if (sscanf(ipaddr, "%d.%d.%d.%d", &a, &b, &c, &d) != 4)
        return 0;

    addr = a << 24;
    addr |= b << 16;
    addr |= c << 8;
    addr |= d;

    return addr;
}

bool isIpRange(const char * range)
{
    int a, b, c, d, e;
    if (sscanf(range, "%d.%d.%d.%d-%d", &a, &b, &c, &d, &e) == 5)
    {

        return true;
    }
    return false;
}

bool isSubnet(const char * subnet)
{
    int a1, b1, c1, d1, a2, b2, c2, d2;
    if (sscanf(subnet, "%d.%d.%d.%d/%d.%d.%d.%d", &a1, &b1, &c1, &d1, &a2, &b2, &c2, &d2) == 8)
    {

        return true;
    }
    return false;
}

bool IsIPInRange(char * ipaddr, char * network, char * mask)
{
    uint32_t ip_addr = IPToUInt(ipaddr);
    uint32_t network_addr = IPToUInt(network);
    uint32_t mask_addr = IPToUInt(mask);

    uint32_t net_lower = (network_addr & mask_addr);
    uint32_t net_upper = (net_lower | (~mask_addr));

    if (ip_addr >= net_lower && ip_addr <= net_upper)
    {
        return true;
    }
    return false;
}

bool isValidHostname(const char * hostname)
{
    regex_t regex;
    int reti;
    bool retval = false;
    char * ValidHostnameRegex = "^([[:digit:]a-zA-Z]([-[:digit:]a-zA-Z]{0,61}[[:digit:]a-zA-Z]){0,1})$";

    reti = regcomp(&regex, ValidHostnameRegex, 0);
    if (reti)
    {
        retval = false;
    }
    reti = regexec(&regex, hostname, 0, NULL, 0);
    if (!reti)
    {
        retval = true;
    }
    else if (reti == REG_NOMATCH)
    {
        retval = false;
    }
    regfree(&regex);

    return retval;
}

void init_active_c_table()
{
    int f = 0;
    active_connections = xmalloc(globals.sections_cnt * sizeof (struct _active_connections *));
    for (f = 0; f < globals.sections_cnt; f++)
    {
        active_connections[f] = xmalloc(globals.maxcon * sizeof (struct _active_connections));
    }
    int i, j;

    for (i = 0; i < globals.sections_cnt; i++)
    {
        for (j = 0; j < globals.maxcon; j++)
        {
            memset(active_connections[i][j].ip, '\0', 16);
            active_connections[i][j].port = -1;
            active_connections[i][j].connfd = -1;
            active_connections[i][j].conno = -1;
        }
    }
}

void save_active_connection(char *ipaddr, int portno, int rc, int i)
{

    int sz = 128;
    char network_msg[sz];

    memset(network_msg, '\0', sz);
    snprintf(network_msg, sz, "Saving connection state for [%d][%d]", rc, i);
    writeToCustomLog(network_msg);

    strcpy(active_connections[rc][i].ip, ipaddr);
    active_connections[rc][i].connfd = i;
    active_connections[rc][i].port = portno;
    active_connections[rc][i].conno = i;
}

void deactivate_connection(int rc, int i)
{

    int sz = 128;
    char network_msg[sz];

    memset(network_msg, '\0', sz);
    snprintf(network_msg, sz, "Deactivating connection state for [%d][%d]", rc, i);
    writeToCustomLog(network_msg);

    bzero(active_connections[rc][i].ip, 16);
    active_connections[rc][i].connfd = -1;
    active_connections[rc][i].port = -1;
    active_connections[rc][i].conno = -1;

}