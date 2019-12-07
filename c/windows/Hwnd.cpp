#include <Windows.h>
#include <winsock.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "commctrl.h"
                    
#define MAINICON 101

#define WM_MOD (WM_USER+0x0001)
#define WM_MOD2 (WM_USER+0x0002)

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK f(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static HWND textBoxInput;
static HWND button;
static HWND myButton;
static HWND textBoxOutput;
static HWND listBox;
static HWND comboBox;

SOCKET serversock;

char *timestamp() {
    char *timestamp = (char *)malloc(sizeof(char) * 16);
    time_t ltime;
    ltime=time(NULL);
    struct tm *tm;
    tm=localtime(&ltime);    
    sprintf(timestamp,"%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
    return timestamp;  
}

int callbackCounter = 0;
void CALLBACK f(HWND hwnd, UINT uMsg, UINT timerId, DWORD dwTime) {
  char *ts = timestamp();
  char str[512];
  sprintf(str, "[%s] LOG No %d\n", ts, callbackCounter);
  if(callbackCounter%5==0 || callbackCounter == 0) { 
    SendMessage(hwnd, WM_MOD, 0, (LPARAM)str); 
    //Beep(523,50); 
    /*
    char t_buf[1024];
    sprintf(t_buf, "broadcast:%s", str);            
    send(serversock, t_buf, strlen(t_buf), 0);
    */
  }
  callbackCounter++;
  free(ts);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_COMMAND:
            if((HWND)lParam == myButton) {               
                TCHAR* buffer = new TCHAR[150];    
                GetWindowText(textBoxInput,buffer,150); 
                SetWindowText(textBoxOutput,buffer);
                SetWindowText(textBoxInput,"");
                SendMessage(textBoxInput,EM_SETSEL, 0, -1);
                SetFocus(textBoxInput);
                char t_buf[1024];
                sprintf(t_buf, "broadcast:%s", buffer);            
                send(serversock, t_buf, strlen(t_buf), 0);
                delete [] buffer;      
            }
            if((HWND)lParam == button) {               
                // get input field
                int len = GetWindowTextLength(textBoxInput) + 1;
                char *text = new char[len];
                GetWindowText(textBoxInput, &text[0], len);
                char myButtonCmd[1024];
                sprintf(myButtonCmd, "[YOU WROTE] %s\n", &text[0]);
                char t_buf[1024];
                sprintf(t_buf, "broadcast:%s", &text[0]);            
                send(serversock, t_buf, strlen(t_buf), 0);
                //Append the text to box 2.
                SendMessage(textBoxOutput, EM_SETSEL, GetWindowTextLength(textBoxOutput), GetWindowTextLength(textBoxOutput)+len);
                SendMessage(textBoxOutput, EM_REPLACESEL, GetWindowTextLength(textBoxOutput)+len, (LPARAM)myButtonCmd);
                delete[] text;
                SetWindowText(textBoxInput, "");
                SetFocus(textBoxInput);
                //SendMessage(hwnd, WM_MOD, 0, 0);
                free(text);   
            } 
            
            if((HWND)lParam == listBox) {
                if(wParam == 262144) printf("Listbox (%d) clicked (%d)\n", lParam, wParam);   
                else if(wParam == 327680) printf("Listbox (%d) blurred (%d)\n", lParam, wParam);
                else printf("Listbox (%d) ??? (%d)\n", lParam, wParam);
                //SetFocus(textBoxInput);
                //int index_of_selection = SendMessage (hwnd, CB_SETCURSEL, 0, 0);
                //printf("ios: %d\n", index_of_selection);         
            }      
            break;
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
                if(1==1) {
                long x = LOWORD(lParam), y = HIWORD(lParam);
                POINT p = { x, y };
                ClientToScreen(hwnd, &p); //add this line
                //printf("x=%d\n", (int)x);
                InvalidateRect (hwnd, NULL, FALSE) ;
                return 0;
                }
                break;
        case WM_KEYUP:
        case WM_KEYDOWN:
                break;
        case WM_DESTROY:
            PostQuitMessage(0);
            exit(0);
            return 0;
        case WM_MOVE:
            //printf("window moved to %d:%d\n", LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_SIZE:
            //printf("window resized to %dx%d\n", LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_MOUSEMOVE:
            //printf("mouse moved to %d:%d\n", LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_MOD:           
            //SendMessage(hwnd, WM_PAINT, 0, 0);
            //InvalidateRect(hwnd, NULL, TRUE);
            //SendMessage(textBoxOutput, EM_REPLACESEL, sizeof(lParam), lParam);
            
            //Append the text to box 2.
            SendMessage(textBoxOutput, EM_SETSEL, GetWindowTextLength(textBoxOutput), GetWindowTextLength(textBoxOutput)+sizeof(lParam));
            SendMessage(textBoxOutput, EM_REPLACESEL, GetWindowTextLength(textBoxOutput)+sizeof(lParam), lParam);
            //SetWindowText(textBoxInput, "");
            //SetFocus(textBoxInput); 
            
            //test();  
            break;
        case WM_MOD2:           
            //SendMessage(hwnd, WM_PAINT, 0, 0);
            //InvalidateRect(hwnd, NULL, TRUE);
            //SendMessage(textBoxOutput, EM_REPLACESEL, sizeof(lParam), lParam);
            
            //Append the text to box 2.
            SendMessage(textBoxOutput, EM_SETSEL, GetWindowTextLength(textBoxOutput), GetWindowTextLength(textBoxOutput)+sizeof(lParam));
            SendMessage(textBoxOutput, EM_REPLACESEL, GetWindowTextLength(textBoxOutput)+sizeof(lParam), lParam);
            SetWindowText(textBoxInput, "");
            SetFocus(textBoxInput);
 
            break;
        case WM_CLOSE:
              if(MessageBox(hwnd,"Wirklich beenden?","Beenden",MB_OKCANCEL)==IDOK) {
      			DestroyWindow(hwnd);
      			break;
      		}
		    else return 0;            
        default:
            // see https://wiki.winehq.org/List_Of_Windows_Messages
            //if( lParam ) printf("%d\n", lParam);
            break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int CALLBACK WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR cmdLine,int nCmdShow)
{

    // SOCKET serversock;
    WSADATA wsaData;
    WSAStartup(0x102,&wsaData);    
    struct sockaddr_in server;    
    memset( &server, 0, sizeof (server));
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(13000);
    serversock = socket( AF_INET, SOCK_STREAM, 0 );
        
    u_long iMode = 0;
    ioctlsocket(serversock, FIONBIO, &iMode);
                  
    connect(serversock, (struct sockaddr*)&server, sizeof(server));
    
    char tmp[128];
    sprintf(tmp, "broadcast:hello world");
    send(serversock, tmp, strlen(tmp), 0);
    
    // Hide default windows console
    //ShowWindow(GetConsoleWindow(), SW_HIDE);
    HWND hMainWindow;
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = "MainClass";
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_QUESTION);
    //wc.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(MAINICON));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); // IDC_ARROW, IDC_CROSS, IDC_HAND, IDC_HELP, IDC_WAIT
    wc.hbrBackground = static_cast<HBRUSH>((HBRUSH)25);
    RegisterClass(&wc);

    hMainWindow = CreateWindow(wc.lpszClassName,"HWND Demo v0.1",WS_OVERLAPPED | DS_3DLOOK | CS_DBLCLKS | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,CW_USEDEFAULT,CW_USEDEFAULT,525,370,NULL,NULL,hInstance,NULL);
    
    int error = GetLastError();
    if(hMainWindow == NULL) return 1;
    
    RECT rc;
    GetClientRect(hMainWindow, &rc);
    //InflateRect(&rc,-120,-120);
    //InvalidateRect(hMainWindow, &rc, TRUE);

    textBoxInput = CreateWindowEx(
        WS_EX_CLIENTEDGE, // style
        "Edit", // name of class
        "", // text when first created
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_WANTRETURN, // options
        10, 10, 500, 21, // size and position
        hMainWindow, // handle to parent window
        NULL, // handle to child window identifier
        NULL, // handle to application instance
        NULL); // no window creation data    
    button = CreateWindowEx(WS_EX_CLIENTEDGE,"Button","Broadcast", WS_CHILD | WS_VISIBLE , rc.right - rc.left - 85, 140,75,30,hMainWindow,NULL,NULL,NULL);             
    textBoxOutput = CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL |  ES_MULTILINE | ES_READONLY ,10,40,500,90,hMainWindow,NULL,NULL,NULL);
    //MessageBox(hMainWindow, "Could not create edit box.", "Error", MB_OK | MB_ICONERROR);
    listBox = CreateWindowEx(WS_EX_CLIENTEDGE, "listbox", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL, 10, 140, 100, rc.top + (rc.bottom - 150), hMainWindow, NULL, NULL, NULL);
    SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM)TEXT("Item1") );
    SendMessage(listBox, LB_ADDSTRING, 1, (LPARAM)TEXT("Item2"));
    SendMessage(listBox, LB_ADDSTRING, 2, (LPARAM)TEXT("Item3"));
    SendMessage(listBox, LB_ADDSTRING, 3, (LPARAM)TEXT("Item4"));    
    SendMessage(listBox, LB_ADDSTRING, 4, (LPARAM)TEXT("Item5"));
    
    comboBox = CreateWindowEx(WS_EX_CLIENTEDGE, "COMBOBOX", "", CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_VISIBLE, 120, 140, 100, 200, hMainWindow, NULL, NULL, NULL);
    SendMessage(comboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("Item1"));
    SendMessage(comboBox, CB_ADDSTRING, 1, (LPARAM)TEXT("Item2"));
    SendMessage(comboBox, CB_ADDSTRING, 2, (LPARAM)TEXT("Item3"));
    SendMessage(comboBox, CB_ADDSTRING, 3, (LPARAM)TEXT("Item4"));
    SendMessage(comboBox, CB_ADDSTRING, 4, (LPARAM)TEXT("Item5"));


    ShowWindow(hMainWindow,SW_SHOW);
    UpdateWindow(hMainWindow);

    MSG msg = { };

    SetTimer(hMainWindow, 0, 1000,(TIMERPROC) &f);
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        // data from msg
        if (msg.message == WM_KEYDOWN) {
            if(msg.wParam == 13) {
                //SendMessage (hMainWindow, WM_MOD2, 0, (LPARAM)"Message sent!\n");
                printf("key down %d\n", msg.wParam);
                SendMessage(button, BM_CLICK, 0, 0);
            }
        }
    }

    return 0;
}  
