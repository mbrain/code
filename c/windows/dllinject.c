#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>  
                       
typedef void (*startThreadFunc)();
                       
int main(int argc, char *argv[]) {   

	BOOL bret;
	DWORD pid;
	DWORD texitcode;
	HANDLE phandle, thandle;
	LPVOID rdllname;
	FARPROC loadlibrary;
    
    printf("DLL Injector v1 (2019) - Written by Manuel Zarat, manuel.zarat@gmail.com\n\n\n\n");

	if (argc < 3 || sscanf(argv[1], "%d", &pid) < 0) {
		printf("Usage: %s PID DLLNAME\n", argv[0]);
		return 1;
	}
    
	LPSTR dllname = argv[2];
	SIZE_T size = strlen(argv[2]) + 1;

	printf("[info] inject \"%s\" to process %d\n", dllname, pid); 

	phandle = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, pid);
	printf("[info] process handle = %p\n", phandle);    
	if (phandle == NULL) {
		printf("[warn] OpenProcess failed!\n");
		return 1;
	}   

	rdllname = VirtualAllocEx(phandle, NULL, size, MEM_COMMIT, PAGE_READWRITE);
	printf("[info] alloc remote memory = %p\n", rdllname);
	if (rdllname == NULL) {
		printf("[warn] VirtualAllocEx failed!\n");
		return 1;
	}

	bret = WriteProcessMemory(phandle, rdllname, dllname, size, NULL);
	printf("[info] WriteProcessMemory returns %d\n", (int) bret);
	if (bret == FALSE) {
		printf("[warn] WriteProcessMemory failed!\n");
		return 1;
	}

	loadlibrary = GetProcAddress(GetModuleHandle("kernel32.dll"),"LoadLibraryA");
	printf("[info] address of LoadLibraryA = %p\n", loadlibrary);
	if (loadlibrary == NULL) {
		printf("[warn] GetProcAddress failed!\n");
		return 1;
	} 
 
	thandle = CreateRemoteThread(phandle, NULL, 0, (LPTHREAD_START_ROUTINE) loadlibrary, rdllname, 0, NULL);
	printf("[info] thread handle = %p\n", thandle);
	if (thandle == NULL) {
		printf("[warn] CreateRemoteThread failed!\n");
		return 1;
	}     
    
	printf("[info] wait for remote thread\n");
	WaitForSingleObject(thandle, INFINITE);

	bret = GetExitCodeThread(thandle, &texitcode);
	if (bret == FALSE) printf("[warn] GetExitCodeThread failed!\n");
	else printf("[info] remote thread's base address = %p\n", texitcode);

    /* Is loading the DLL itself, not from injected process :( */
    HINSTANCE getDLLFunc = LoadLibrary(dllname);
    printf("[info] calling remote dll func @ %d\n", GetProcAddress(getDLLFunc, "info"));
    startThreadFunc startThreadFuncVar = (startThreadFunc)GetProcAddress(getDLLFunc, "info");
    startThreadFuncVar();
    FreeLibrary(getDLLFunc);      

        
    boolean bSuccess = (texitcode != 0) ? TRUE : FALSE;

	printf("[info] free remote memory\n");
	VirtualFreeEx(phandle, rdllname, 0, MEM_RELEASE);

	printf("[info] free handle\n");
	CloseHandle(thandle);
	CloseHandle(phandle);

	if (texitcode != 0) printf("[info] finish!\n");
	else printf("[info] finish, but it seems that an error has occurred.\n");

	return 0; 
}
