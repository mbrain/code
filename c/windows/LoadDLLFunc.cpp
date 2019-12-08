#include <windows.h>
#include <stdio.h>

const DWORD MAXINJECTSIZE = 8192;

typedef HMODULE (__stdcall *PLoadLibraryW)(wchar_t*);
typedef HMODULE (__stdcall *PGetModuleHandleW)(wchar_t*);
typedef BOOL    (__stdcall *PFreeLibrary)(HMODULE);
typedef FARPROC (__stdcall *PGetProcAddress)(HMODULE, char*);

struct RemoteThreadBlock {
	DWORD				ErrorLoad; 
	DWORD				ErrorFunction; 
	DWORD				ReturnCodeForFunction; 
	DWORD				ErrorFree; 
	HMODULE				hModule;
	BOOL				bLoadLibrary;
	BOOL				bFreeLibrary;
	PLoadLibraryW		fnLoadLibrary;
	PGetModuleHandleW	fnGetModuleHandle;
	PFreeLibrary		fnFreeLibrary;
	PGetProcAddress		fnGetProcAddress;
	wchar_t				lpModulePath[_MAX_PATH];
	char				lpFunctionName[256];	
};

DWORD ConvertWCharToChar( wchar_t*, char*, DWORD );
void EnableDebugPriv( void );
bool ExecuteRemoteThread( HANDLE, BOOL, BOOL, wchar_t*, wchar_t* );
DWORD __stdcall RemoteThread( RemoteThreadBlock* );
DWORD LoadDllForRemoteThread( DWORD, BOOL, BOOL, wchar_t*, wchar_t* );
BOOL IsWindowsNT();

int main( int argc, wchar_t *argv[] ) {
	BOOL bUsage = TRUE;
	DWORD pID = 0;
	wchar_t* lpModulePath = NULL;
	wchar_t* lpFunctionName = NULL;
	BOOL bLoad = FALSE;
	BOOL bFree = FALSE;
	for ( int i = 1, j = 0 ; i < argc; i++ ) {
		if ( wcsicmp( argv[i], L"/?" ) == 0 || wcsicmp( argv[i], L"-?" ) == 0 || wcsicmp( argv[i], L"/h" ) == 0 || wcsicmp( argv[i], L"-h" ) == 0 || wcsicmp( argv[i], L"/help" ) == 0 || wcsicmp( argv[i], L"-help" ) == 0 ) {
			bUsage = TRUE;
			break;
		} else if ( wcsicmp( argv[i], L"/l" ) == 0 || wcsicmp( argv[i], L"-l" ) == 0 ) {
			bLoad = TRUE;
		} else if ( wcsicmp( argv[i], L"/u" ) == 0 || wcsicmp( argv[i], L"-u" ) == 0 ) {
			bFree = TRUE;
		} else {
			switch ( j ) {
    			case 0:
    				char strPId[10];
    				ConvertWCharToChar( argv[i], strPId, sizeof(strPId) );
    				pID = atoi( strPId );    
    				if ( pID == -1 ) pID = GetCurrentProcessId();    				
    				break;
    			case 1:
    				lpModulePath = argv[i];
    				bUsage = FALSE;
    				break;
    			case 2:
    				lpFunctionName = argv[i];
    				break;
    			default:
    				break;
			};
			j++;
		}
	};
    wprintf( L"[info] Dller v1.0 by Manuel Zarat (manuel.zarat@gmail.com)\n" );
	if ( bUsage ) {
		wprintf( L"Usage: LOADDLL [-L] [-U] processID dllPath [functionName]\n" );		
		return 1;
	}
	if ( !IsWindowsNT() ) {
		wprintf( L"This executable needs WindowsNT/Windows2000\n" );
		return 2;
	}
	EnableDebugPriv();
	LoadDllForRemoteThread( pID, bLoad, bFree, lpModulePath, lpFunctionName );	
	return 0;
}

BOOL IsWindowsNT() {
   OSVERSIONINFOEX osvi;
   BOOL bOsVersionInfoEx;
   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
   bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi);
   if( bOsVersionInfoEx == 0 ) {
      osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
      if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
         return FALSE;
   }
   return osvi.dwPlatformId == VER_PLATFORM_WIN32_NT;
}

DWORD ConvertWCharToChar( wchar_t* wStr, char* cStr, DWORD size ) {
	ULONG i = 0;
	wchar_t* wAct = wStr;
	char* cAct = cStr;
	*cStr = 0;
	while ( *wAct != 0 && i < size ) {
		*cAct++ = (char)*wAct++;
		i++;
	}
	return i;
}

DWORD LoadDllForRemoteThread( DWORD processID, BOOL bLoad, BOOL bFree, wchar_t* lpModuleName, wchar_t* lpFunctionName ) {
	HANDLE hProcess = OpenProcess( PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, processID );	
	if ( hProcess != NULL ) {
		ExecuteRemoteThread( hProcess, bLoad, bFree, lpModuleName, lpFunctionName );
		CloseHandle( hProcess );
	}
	else wprintf( L"Open remote process failed! Error = %d\n", GetLastError() );		
	return 0;
}

void EnableDebugPriv( void ) {
	HANDLE hToken;
	LUID sedebugnameValue;
	TOKEN_PRIVILEGES tkp;
	if ( ! OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken ) ) {
		wprintf( L"OpenProcessToken() failed, Error = %d SeDebugPrivilege is not available.\n", GetLastError() );
		return;
	}
	if ( ! LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &sedebugnameValue ) ) {
		wprintf( L"LookupPrivilegeValue() failed, Error = %d SeDebugPrivilege is not available.\n", GetLastError() );
		CloseHandle( hToken );
		return;
	}
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = sedebugnameValue;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if ( ! AdjustTokenPrivileges( hToken, FALSE, &tkp, sizeof tkp, NULL, NULL ) ) wprintf( L"AdjustTokenPrivileges() failed, Error = %d SeDebugPrivilege is not available.\n", GetLastError() );
	CloseHandle( hToken );
}

bool ExecuteRemoteThread( HANDLE hProcess, BOOL bLoad, BOOL bFree, wchar_t* lpDllPath, wchar_t* lpFunctionName ) {
	HANDLE ht = 0;
	void *p = 0;
	RemoteThreadBlock *c = 0;
	bool result = false;
	DWORD rc;
	HMODULE hKernel32 = 0;
	RemoteThreadBlock localCopy;
    char funcarg[] = "hello";
	::ZeroMemory( &localCopy, sizeof(localCopy) );

	p = VirtualAllocEx( hProcess, 0, MAXINJECTSIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE );
	if ( p == 0 ) {
		wprintf( L"VirtualAllocEx() failed, Error = %d \n", GetLastError() );
		goto cleanup;
	}
	c = (RemoteThreadBlock*) VirtualAllocEx( hProcess, 0, sizeof(RemoteThreadBlock), MEM_COMMIT, PAGE_READWRITE );
	if ( c == 0 ) {
		wprintf( L"VirtualAllocEx() failed, Error = %d \n", GetLastError() );
		goto cleanup;
	}
	if ( ! WriteProcessMemory( hProcess, p, (LPCVOID)&RemoteThread, MAXINJECTSIZE, 0 ) ) {
		wprintf( L"WriteProcessMemory() failed, Error = %d \n", GetLastError() );
		goto cleanup;
	}	
	wcscpy( localCopy.lpModulePath, lpDllPath );
	if ( lpFunctionName == NULL ) localCopy.lpFunctionName[0] = 0;
	else ConvertWCharToChar( lpFunctionName, localCopy.lpFunctionName, sizeof(localCopy.lpFunctionName) );	
	localCopy.bLoadLibrary = bLoad;
	localCopy.bFreeLibrary = bFree;
	hKernel32 = LoadLibraryA( "kernel32.dll" );
	if ( hKernel32 == NULL ) {
		wprintf( L"Couldn't load kernel32.dll. That's a surprise!\n" );
		goto cleanup;
	}
	localCopy.fnLoadLibrary = (PLoadLibraryW)GetProcAddress( hKernel32, "LoadLibraryW" );
	localCopy.fnGetModuleHandle = (PGetModuleHandleW)GetProcAddress( hKernel32, "GetModuleHandleW" );
	localCopy.fnFreeLibrary = (PFreeLibrary)GetProcAddress( hKernel32, "FreeLibrary" );
	localCopy.fnGetProcAddress = (PGetProcAddress)GetProcAddress( hKernel32, "GetProcAddress" );
	if (localCopy.fnLoadLibrary == NULL || localCopy.fnGetModuleHandle == NULL || localCopy.fnFreeLibrary == NULL || localCopy.fnGetProcAddress == NULL) {
		wprintf( L"GetProcAddress() failed. Error = %d\n", GetLastError() );
		goto cleanup;
	}
	if ( ! WriteProcessMemory( hProcess, c, &localCopy, sizeof localCopy, 0 ) ) {
		wprintf( L"WriteProcessMemory() failed, Error = %d \n", GetLastError() );
		goto cleanup;
	}
	ht = CreateRemoteThread( hProcess, 0, 0, (DWORD (__stdcall *)( void *)) p, c, 0, &rc );
	if ( ht == NULL ) {
		wprintf( L"CreateRemoteThread() failed, Error = %d \n", GetLastError() );
		goto cleanup;
	}
	rc = WaitForSingleObject( ht, INFINITE );
	switch ( rc ) {
    	case WAIT_TIMEOUT:
    		wprintf( L"WaitForSingleObject() timed out. INFINITE is over!" );
    		goto cleanup;
    	case WAIT_FAILED:
    		wprintf( L"WaitForSingleObject() failed, Error = %d \n", GetLastError() );
    		goto cleanup;
    	case WAIT_OBJECT_0:
    		if ( ! ReadProcessMemory( hProcess, c, &localCopy, sizeof localCopy, 0 ) ) {
    			wprintf( L"ReadProcessMemory() failed, Error = %d\n", GetLastError() );
    			goto cleanup;
    		}
    		if ( bLoad ) {
    			wprintf( L"[info] Lade \"%s\": ", lpDllPath );
    			switch( localCopy.ErrorLoad ) {
        			case 0:
        				wprintf( L"OK (0x%08X)\n", localCopy.hModule );
        				break;
        
        			default:
        				wprintf( L"ERROR\n", localCopy.ErrorLoad );
        				break;
    			}
    		}   
    		if ( lpFunctionName != NULL ) {
    			wprintf( L"[info] Lade \"%s()\": ", lpFunctionName );    
    			switch( localCopy.ErrorFunction ) {
        			case 0:
        				wprintf( L"OK (0x%X)\n", localCopy.ReturnCodeForFunction );
        				break;        
        			default:
        				wprintf( L"ERROR\n" );
        				break;
    			}
    		}
    		if ( bFree ) {
    			wprintf( L"[info] Entlade \"%s\": ", lpDllPath );
    			switch( localCopy.ErrorFree ) {
        			case 0:
        				wprintf( L"OK (0x%08X)\n", localCopy.hModule );
        				break;
        
        			default:
        				wprintf( L"ERROR\n" );
        				break;
    			}
    		}   		
    		break;    	
    	default:
    		wprintf( L"WaitForSingleObject() failed, Error = %d \n", GetLastError() );
    		break;
	}

cleanup:
	CloseHandle( ht );
	if ( p != 0 ) VirtualFreeEx( hProcess, p, 0, MEM_RELEASE );
	if ( c != 0 ) VirtualFreeEx( hProcess, c, 0, MEM_RELEASE );
	if ( hKernel32 != NULL) FreeLibrary( hKernel32 );
	return result;
}

DWORD __stdcall RemoteThread( RemoteThreadBlock* execBlock ) {
	typedef DWORD (*PRemoteDllFunction)();
	HMODULE hModule = NULL;
	execBlock->ErrorLoad = 0;
	execBlock->ErrorFunction = 0;
	execBlock->ErrorFree = 0;
	if ( execBlock->bLoadLibrary ) {
		execBlock->hModule = (HMODULE)(*execBlock->fnLoadLibrary)( execBlock->lpModulePath );
		hModule = execBlock->hModule;
		execBlock->ErrorLoad = execBlock->hModule != NULL ? 0 : 1;
	}
	if ( hModule == NULL ) hModule = (*execBlock->fnGetModuleHandle)( execBlock->lpModulePath );
	if ( execBlock->lpFunctionName[0] != 0 ) {
		PRemoteDllFunction fnRemoteDllFunction = (PRemoteDllFunction) (*execBlock->fnGetProcAddress)( hModule, execBlock->lpFunctionName );
		if ( fnRemoteDllFunction != NULL ) {
			execBlock->ErrorFunction = 0;
			execBlock->ReturnCodeForFunction = (*fnRemoteDllFunction)();
		}
		else execBlock->ErrorFunction = 1;
	}
	if ( execBlock->bFreeLibrary ) {
		execBlock->ErrorFree = execBlock->fnFreeLibrary( hModule ) ? 0 : 1;
	}
	execBlock->hModule = hModule;	
	return 0;
}
