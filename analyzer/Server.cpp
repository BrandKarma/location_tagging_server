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
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ext/hash_map>
#include <ext/hash_set>
#include <dlfcn.h>

#include "Server.hpp"
#include "marshal.h"

using namespace std;
using namespace __gnu_cxx;


void Server::init( const char* locPath, const char* casePath ) {
    analyzer.init( locPath, casePath );
}


void Server::start( int port ) {
    int sockfd, newsockfd, clilen;
    struct sockaddr_in cli_addr, serv_addr;
    pthread_t chld_thr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        fprintf(stderr,"server: can't open stream socket\n"), exit(0);

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);
	
    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
       fprintf(stderr,"server: can't bind local address\n"), exit(0);

    printf("Analyzer successfully bind on Port %d\n", port );

    listen(sockfd, 10);

    for(;;){

        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, 
                           (socklen_t*) &clilen);
 
        if(newsockfd < 0) {
            fprintf(stderr,"server: accept error\n");
            continue;
        }
        process( newsockfd );
    }
}

void  Server::process(int mysocfd) {

    /* read from the given socket */
//printf("child thread [%d]: socket no. = %d\n", pthread_self(), mysocfd );
    int packetSize;
    recv(mysocfd, &packetSize, sizeof(int), MSG_PEEK);
    packetSize = ntohl( packetSize );
    if (packetSize <= 0) {
        printf("Ignore error message packet, message header = %d\n", packetSize );
        close(mysocfd);
        return;    
    }

    char * data = new char[packetSize];
    int n = recv(mysocfd, data, packetSize, MSG_WAITALL);
    if ( n != packetSize ) {
        printf("Actual size of incoming packet unmatched with actual size\n");
        delete [] data;
        close(mysocfd);
        return;    
    }
    
    printf("***packet received specified size = %d, actual size = %d\n", packetSize, n);
    fflush(stdout);
  
    Unmarshaler * u = new Unmarshaler( data );
    if ( n != u->unmarshal_int() ) {
        printf("corrupted packet received! Skip ... \n");
        delete u;
        close(mysocfd);
        return;
    }

    char *locStr = u->unmarshal_string(MAX_STR_LEN);    
    printf( "%s\n", locStr );
    delete(u);

    wstring locationWstr = ctow( locStr );
    vector<string> locBreakDown = analyzer.analyze( locationWstr );

    Marshaler * m = new Marshaler();

    m->marshal_int( locBreakDown.size()-1 );
    for ( int i = 1; i < locBreakDown.size(); i++ )
    {   
        m->marshal_string( locBreakDown[i].c_str() );  
    }

    // close the socket and exit this thread 
    int inpacketsize = packetSize;
    packetSize = m->get_data(&data);
    delete m;
    n = send(mysocfd, data, packetSize, 0);
    delete data;

    close(mysocfd);
    printf("***packet sent out %d (specified %d)for incoming %d\n", 
           n, packetSize, inpacketsize);
    fflush(stdout);
}


int main( int argc, char** argv ) {
    if ( argc < 4 ) 
    {
        cout << "Wrong number of arguments: " << argv[0] << " <locPath> < casePath> <port>";
    }
    setlocale(LC_CTYPE, "en_US.UTF-8");
    Server s;
    s.init( argv[1], argv[2] );
    s.start( atoi(argv[3]) );
}
