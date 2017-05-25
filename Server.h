#include "Protocol.c"
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <limits.h>
#include <signal.h>

#define LARGE_MLC 4096
#define SMALL_MLC 255

void  break_loop(void);
void  command_check(int, int, char*, char*);
char* fix_return_string(char*, int);
int   ls(char*);
int   pwd(char*);
int   cd(char*, char*, char*);
int   file_info(char*, char*);
char* cat(char*);
