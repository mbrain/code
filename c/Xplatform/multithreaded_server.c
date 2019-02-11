#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#ifdef _WIN32 /* Windows */
#include <winsock.h>
#include <io.h>
#else /* UNIX/Linux */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#define RCVBUFSIZE 1024

/**
 * Thread Handler
 */
#ifdef _WIN32
DWORD WINAPI process_thread(LPVOID lpParam) {
    SOCKET current_client = (SOCKET)lpParam;
#else
void process_thread(int sock) {
    int current_client = sock;
#endif
    char buf[1024];
    int res;       
    while(1) {       
        res = recv(current_client, buf, 1024, 0);          
        if(res>1) {
            buf[strlen(buf)] = '\0';
            send(current_client, buf, strlen(buf), 0);
        }       
    }    
}

/**
 * Main
 */
int main( int argc, char *argv[]) {
    int port = port = atoi(argv[1]);
    struct sockaddr_in server, client;
    #ifdef _WIN32
    SOCKET sock, fd;
    #else
    int sock, fd;
    #endif
    unsigned int len;
    #ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(0x102,&wsaData);
    #endif
    memset( &server, 0, sizeof (server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);
    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    bind(sock, (struct sockaddr*)&server, sizeof( server));
    listen(sock, 5);
    #ifdef _WIN32
    DWORD thread;
    #else
    pthread_t thread;
    #endif
    while(1) {
        len = sizeof(client);
        fd = accept(sock, (struct sockaddr*)&client, &len);
        if(fd>0) {
            #ifdef _WIN32
            CreateThread(NULL, 0, process_thread, (LPVOID)fd, 0, &thread);
            #else
            pthread_create( &thread , NULL , process_thread , (int)fd);
            #endif
        }
    }
    #ifdef _WIN32
    closesocket(fd);
    #else
    close(fd);
    #endif
    return 1;
}
