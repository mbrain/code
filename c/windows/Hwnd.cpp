#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define WM_MOD (WM_USER+0x0001)
#define WM_MOD2 (WM_USER+0x0002)

static HWND textBoxInput;
static HWND button;
static HWND myButton;
static HWND textBoxOutput;
static HWND listBox;
static HWND comboBox;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND CreateHwnd (HWND hwndParent) {
    HWND item = CreateWindowEx( WS_EX_CLIENTEDGE, "Button", "Test", WS_CHILD | WS_VISIBLE, 10, 41, 75, 30, hwndParent, NULL, NULL, NULL); 
    return (item);
}

void test() {
    for(int i = 0; i < 100000; i++) printf("%d\n", i);
} 

char *timestamp(){
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
  strcat(ts, " LOG ENTRY\n");
  if(callbackCounter%11==0 || callbackCounter == 0) SendMessage(hwnd, WM_MOD, 0, (LPARAM)ts);
  free(ts);
  callbackCounter++;
}

int CALLBACK WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR cmdLine,int nCmdShow)
{
    
    // Hide default windows console
    //ShowWindow(GetConsoleWindow(), SW_HIDE);
    HWND hMainWindow;
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = "MainClass";
    wc.hInstance = hInstance;
    //wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); // IDC_ARROW, IDC_CROSS, IDC_HAND, IDC_HELP, IDC_WAIT
    wc.hbrBackground = static_cast<HBRUSH>((HBRUSH)25);
    RegisterClass(&wc);

    hMainWindow = CreateWindow(wc.lpszClassName,"Client GUI (Demo)",WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,CW_USEDEFAULT,CW_USEDEFAULT,525,370,NULL,NULL,hInstance,NULL);
    
    int error = GetLastError();
    if(hMainWindow == NULL) return 1;

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
    //button = CreateWindowEx(WS_EX_CLIENTEDGE,"Button","Replace", WS_CHILD | WS_VISIBLE , 10, 41,75,30,hMainWindow,NULL,NULL,NULL);             
    textBoxOutput = CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL |  ES_MULTILINE | ES_READONLY ,10,40,500,90,hMainWindow,NULL,NULL,NULL);
    //MessageBox(hMainWindow, "Could not create edit box.", "Error", MB_OK | MB_ICONERROR);
    listBox = CreateWindowEx(WS_EX_CLIENTEDGE, "listbox", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL, 10, 140, 100, 60, hMainWindow, NULL, NULL, NULL);
    SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM)"name");
    SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM)"alter");
    SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM)"rang");    
    comboBox = CreateWindowEx(WS_EX_CLIENTEDGE, "COMBOBOX", "", CBS_DROPDOWN | WS_CHILD | WS_VISIBLE, 120, 140, 50, 50, hMainWindow, NULL, NULL, NULL);
    SendMessage(comboBox, CB_ADDSTRING, 0, (LPARAM)"Hiii");

    // HWND listView = null;
        
    RECT rc;
    GetClientRect(hMainWindow, &rc);
    //InflateRect(&rc,-120,-120);
    //InvalidateRect(hMainWindow, &rc, TRUE);
    
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
                SendMessage (hMainWindow, WM_MOD2, 0, (LPARAM)"Message sent!\n");
                printf("key down %d\n", msg.wParam);
            }
        }
    }

    return 0;
}  

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_COMMAND:
            if((HWND)lParam == button) {               
                TCHAR* buffer = new TCHAR[150];    
                GetWindowText(textBoxInput,buffer,150);
                //printf("WM_COMMAND::button: %s\n", buffer);    
                SetWindowText(textBoxOutput,buffer);
                SetWindowText(textBoxInput,"");
                SendMessage(textBoxInput,EM_SETSEL, 0, -1);
                SetFocus(textBoxInput);           
                delete [] buffer;       
            }
            if((HWND)lParam == myButton) {               
                // get input field
                int len = GetWindowTextLength(textBoxInput) + 1;
                char *text = new char[len];
                GetWindowText(textBoxInput, &text[0], len);
                //Append the text to box 2.
                SendMessage(textBoxOutput, EM_SETSEL, GetWindowTextLength(textBoxOutput), GetWindowTextLength(textBoxOutput)+len);
                SendMessage(textBoxOutput, EM_REPLACESEL, GetWindowTextLength(textBoxOutput)+len, (LPARAM)&text[0]);
                delete[] text;
                SetWindowText(textBoxInput, "");
                SetFocus(textBoxInput);
                //SendMessage(hwnd, WM_MOD, 0, 0);     
            } 
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            exit(0);
            return 0;
        case WM_MOVE:
            printf("moved to %d:%d\n", LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_SIZE:
            printf("resized to %dx%d\n", LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_MOUSEMOVE:
            //printf("mouse moved");
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
            return 0; //break;
        case WM_MOD2:           
            //SendMessage(hwnd, WM_PAINT, 0, 0);
            //InvalidateRect(hwnd, NULL, TRUE);
            //SendMessage(textBoxOutput, EM_REPLACESEL, sizeof(lParam), lParam);
            
            //Append the text to box 2.
            SendMessage(textBoxOutput, EM_SETSEL, GetWindowTextLength(textBoxOutput), GetWindowTextLength(textBoxOutput)+sizeof(lParam));
            SendMessage(textBoxOutput, EM_REPLACESEL, GetWindowTextLength(textBoxOutput)+sizeof(lParam), lParam);
            SetWindowText(textBoxInput, "");
            SetFocus(textBoxInput);
 
            return 0; //break;            
        default:
            // see https://wiki.winehq.org/List_Of_Windows_Messages
            //if( lParam ) printf("%d\n", lParam);
            break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
