#ifndef _SERVER_
#define _SERVER_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <wchar.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include <unistd.h>
#include <pthread.h>
#include <vector>
#include <string>
#include <ext/hash_map>
#include <ext/hash_set>

#include "locationAnalyzer.hpp"

class Server {
public:
    void init( const char* locPath, const char* casePath );
    void start( int port );
    void process( int mysockfd );

private:
    LocationAnalyzer analyzer;
};

#endif
