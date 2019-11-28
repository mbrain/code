#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#ifdef _WIN32 
#include <winsock.h>
#include <io.h>
#else 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#define RCVBUFSIZE 1024

struct process_thread_info {
    struct sockaddr_in client;
    #ifdef _WIN32
    SOCKET fd;
    #else
    int fd;
    #endif
};

#ifdef _WIN32
DWORD WINAPI process_thread(LPVOID lpParam) {
#else
void process_thread(void* lpParam) {
#endif
    struct process_thread_info *threadinfo = (struct process_thread_info *) lpParam;
    printf("[***] new connection established from: %s:%d\n", inet_ntoa(threadinfo->client.sin_addr), (int)ntohs(threadinfo->client.sin_port));    
    char buf[1024];
    int res;       
    while(1) {       
        res = recv(threadinfo->fd, buf, 1024, 0);          
        if(res>1) {
            buf[res] = '\0';
            send(threadinfo->fd, buf, res, 0);
        }       
    }
    free(threadinfo); 
}                                                                                      

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
        if(fd<0) continue;        
        struct process_thread_info *threadinfo = malloc(sizeof(struct process_thread_info));
        threadinfo->fd = fd;
        threadinfo->client = client;        
        #ifdef _WIN32
        if(!CreateThread(NULL, 0, process_thread, threadinfo, 0, &thread)) free(threadinfo);
        #else
        if(!pthread_create( &thread , NULL , process_thread , threadinfo)) free(threadinfo);
        #endif
    }
    #ifdef _WIN32
    closesocket(fd);
    #else
    close(fd);
    #endif
    return 1;
}
