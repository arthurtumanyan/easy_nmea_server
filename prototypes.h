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
#ifndef PROTOTYPES_H
#define	PROTOTYPES_H

#ifdef	__cplusplus
extern "C"
{
#endif

extern void * xmalloc(size_t size);
extern void * xrealloc(void *ptr, size_t size);
extern void * xcalloc(size_t nmemb, size_t size);
extern bool isValidPort(const char * p);
extern int isValidIP(char *str);
extern bool FileExists(char *path);
extern bool DirectoryExists(const char* pzPath);
extern void savePid();
extern int checkPid();
extern void removePid();
extern void low_string(char *str);
extern void upper_string(char *str);
extern void trim(const char *str);
extern char * nslookup(char *hostname);
extern bool is_valid_opt(char *opts);
extern void print_usage();
extern int setnonblocking(int sockfd);
extern ssize_t Writeline(int sockd, const void *vptr, size_t n);
extern ssize_t Readline(int sockd, void *vptr, size_t maxlen);
extern void setRightOwner(const char *path);
extern bool hasRightOwner(const char *path);
extern bool isIpRange(const char * range);
extern bool isSubnet(const char * subnet);
extern bool IsIPInRange(char * ipaddr, char * network, char * mask);
extern bool isValidHostname(const char * hostname);
extern void readConfig(char *cfile);
extern void halt();
extern FILE * openCustomLog();
extern void writeToCustomLog(char *);
extern void closeCustomLog();
extern void * spawn_child_processor(void * ptr);
extern int is_allowed_client(char *ip);
extern int is_client_disabled(int rc);
extern int find_proper_client_counter(char *ip);
extern void set_thread_signalmask(sigset_t SignalSet);
extern void init_active_c_table();
extern void save_active_connection(char *ipaddr, int portno, int cfd, int i);
extern void deactivate_connection(int rc, int i);
extern void do_listen();
extern void set_sig_handler();
extern int checksum(char *ss);
extern char * ValidateLongitude(const char *longitude);
extern char * ValidateLatitude(const char *latitude);

extern FILE * openDebugLog();
extern void writeToDebugLog(char *);
extern void closeDebugLog();

extern FILE * openAccessLog();
extern void writeToAccessLog(char *ip, char *ident, char *authuser, char *request, char * response, long bytes, char *backend, char *cookies);
extern void closeAccessLog();

extern FILE * openConLog();
extern void writeToConLog();
extern void closeConLog();

extern void write_syslog(char *);

#ifdef	__cplusplus
}
#endif

#endif	/* PROTOTYPES_H */

