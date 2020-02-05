//
// OVERVIEW: Header.h
// ========
// Declaration of libraries used 
// in the implementation of the
// proxy HTTP.
//
// Author(s): Dimitre Ortt and Jorge Emerson
// Last revision: 09/04/2019


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <netdb.h>
#include <iostream>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <utility>
#include <cinttypes>
#include <list>

#define namespace std;

void *dialogue(void *args);
char * get_url(char * http_content);

