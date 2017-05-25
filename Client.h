#include "Protocol.c"
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <signal.h>
#include <sys/types.h>
#include <netdb.h>

int   connect_to_server(int*, char*, struct sockaddr_in, char*);
void  exit_client(void);
void  user_input(int);
char* prepare_for_server(int, char*, char*);
void  read_from_server(int);
int   well_formed_formula(int, char*);
int   check_argument(int, char*);
int   error_check(char*);
int   missing_data(char*);
void  main_menu(void);
void  cd_menu(void);
void  dfi_menu(void);
void  cat_menu(void);	
