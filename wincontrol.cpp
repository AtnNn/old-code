// Written by Etienne Laurin

#include <windows.h>
#include "resource.h"
#include <Tlhelp32.h>
HWND BhWnd;
HWND B2hWnd;
HWND LhWnd;

BOOL ListProcess();
BOOL GetProcessModule (DWORD dwPID, DWORD dwModuleID, LPMODULEENTRY32 lpMe32, DWORD cbMe32);
BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LPRECT rcParent;
	int i;
	DWORD hp;
	unsigned long *ec;

	switch (message)
	{
	case WM_COMMAND:
		if(HIWORD(wParam)==BN_CLICKED)
		{
			if(lParam==(LPARAM) BhWnd)
			{
				if(!ListProcess())MessageBox(BhWnd,"ListProcess error, please contact AtnNn","WinControl",NULL);
			}
			else if(lParam==(LPARAM) B2hWnd)
			{
				HANDLE hProcess; 
				i = SendMessage(LhWnd, LB_GETCURSEL, 0, 0); 
				hp = SendMessage(LhWnd, LB_GETITEMDATA, i, 0);
				hProcess = OpenProcess (PROCESS_ALL_ACCESS, FALSE, hp); 
				GetExitCodeProcess(hProcess,ec);
				TerminateProcess(hProcess,(UINT)ec);
				SendMessage(hwnd,WM_COMMAND,BN_CLICKED,(LPARAM)BhWnd);
			}
		return 0;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage (0);
		return 0;
	case WM_SIZE:
		GetClientRect(hwnd, rcParent);
		MoveWindow(BhWnd,0,0, rcParent->right/2, 20, TRUE);
		MoveWindow(B2hWnd,rcParent->right/2,0, rcParent->right/2, 20, TRUE);
		MoveWindow(LhWnd,0,20, rcParent->right, rcParent->bottom-20, TRUE);
		return 0; 
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,PSTR szCmdLine, int iCmdShow)
{
	MSG msg;
	WNDCLASS wndclass;
	HWND hWnd;

	wndclass.style         = NULL;//CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON1));
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = "WinControl";

	if (!RegisterClass (&wndclass))
	{    
		MessageBox(NULL,"Error registering the window class. Please contact AtnNn","WinControl",NULL);
		return 0;
	}

	hWnd = CreateWindow("WinControl",
						TEXT ("WinControl"),
						WS_TILED|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_THICKFRAME,
						100,
						100,
						800,
						600,
						NULL,
						NULL,
						hInstance,
						NULL);
	BhWnd=CreateWindow("Button",TEXT("Update Processes"),WS_CHILD|WS_VISIBLE,1,1,198,19,hWnd,NULL,hInstance,NULL);
	B2hWnd=CreateWindow("Button",TEXT("Kill Process"),WS_CHILD|WS_VISIBLE,101,1,198,19,hWnd,NULL,hInstance,NULL);
	LhWnd=CreateWindow("ListBox",TEXT("Processes"),WS_CHILD|WS_VISIBLE,1,21,197,365,hWnd,NULL,hInstance,NULL);
	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);
	ListProcess();
	while(GetMessage (&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam ;
}

BOOL ListProcess()
{
	HANDLE         hProcessSnap = NULL; 
	PROCESSENTRY32 pe32      = {0}; 

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
	
	if (hProcessSnap == INVALID_HANDLE_VALUE) 
		return (FALSE); 

	pe32.dwSize = sizeof(PROCESSENTRY32); 
 
	if (Process32First(hProcessSnap, &pe32)) 
	{ 
		DWORD         dwPriorityClass; 
		BOOL          bGotModule = FALSE; 
		MODULEENTRY32 me32       = {0}; 
		int i = 0;
		SendMessage(LhWnd,LB_RESETCONTENT,0,0);
		do 
		{ 
			bGotModule = GetProcessModule(pe32.th32ProcessID, pe32.th32ModuleID, &me32, sizeof(MODULEENTRY32)); 

			if (bGotModule) 
			{ 
				HANDLE hProcess; 
				hProcess = OpenProcess (PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID); 
				dwPriorityClass = GetPriorityClass (hProcess); 
				CloseHandle (hProcess); 
				//printf( "\nPriority Class Base\t%d\n", pe32.pcPriClassBase); 
				//printf( "PID\t\t\t%d\n", pe32.th32ProcessID);
				//printf( "Thread Count\t\t%d\n", pe32.cntThreads);
				//printf( "Module Name\t\t%s\n", me32.szModule);
				//printf( "Full Path\t\t%s\n\n", me32.szExePath);
                SendMessage(LhWnd, LB_ADDSTRING, 0, (LPARAM) me32.szExePath);
                //char Buf[16];
				//ultoa(pe32.th32ProcessID,Buf,10);
				//SendMessage(LhWnd, LB_ADDSTRING, 0, (LPARAM) Buf);
				SendMessage(LhWnd, LB_SETITEMDATA, i, pe32.th32ProcessID); 
				SetFocus(LhWnd);
				i++;
			}
		} 
		while (Process32Next(hProcessSnap, &pe32)); 
	} 
	else 
	{
		CloseHandle (hProcessSnap); 
		return FALSE;
	}
	CloseHandle (hProcessSnap);
	return TRUE;
}

BOOL GetProcessModule (DWORD dwPID, DWORD dwModuleID, 
     LPMODULEENTRY32 lpMe32, DWORD cbMe32) //this function is copy/paste from mswinsdk
{ 
    BOOL          bRet        = FALSE; 
    BOOL          bFound      = FALSE; 
    HANDLE        hModuleSnap = NULL; 
    MODULEENTRY32 me32        = {0}; 
 
    // Take a snapshot of all modules in the specified process. 

    hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID); 
    if (hModuleSnap == INVALID_HANDLE_VALUE) 
        return (FALSE); 
 
    // Fill the size of the structure before using it. 

    me32.dwSize = sizeof(MODULEENTRY32); 
 
    // Walk the module list of the process, and find the module of 
    // interest. Then copy the information to the buffer pointed 
    // to by lpMe32 so that it can be returned to the caller. 

    if (Module32First(hModuleSnap, &me32)) 
    { 
        do 
        { 
            if (me32.th32ModuleID == dwModuleID) 
            { 
                CopyMemory (lpMe32, &me32, cbMe32); 
                bFound = TRUE; 
            } 
        } 
        while (!bFound && Module32Next(hModuleSnap, &me32)); 
 
        bRet = bFound;   // if this sets bRet to FALSE, dwModuleID 
                         // no longer exists in specified process 
    } 
    else 
        bRet = FALSE;           // could not walk module list 
 
    // Do not forget to clean up the snapshot object. 

    CloseHandle (hModuleSnap); 
 
    return (bRet); 
} 
