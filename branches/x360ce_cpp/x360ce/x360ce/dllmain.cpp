/*  x360ce - XBOX360 Controler Emulator
 *  Copyright (C) 2002-2010 Racer_S
 *  Copyright (C) 2010-2011 Robert Krawczyk
 *
 *  x360ce is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  x360ce is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with x360ce.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.h"
#include "globals.h"
#include "version.h"
#include "x360ce.h"
#include "Utilities\Log.h"
#include "Utilities\Misc.h"
#include "Config.h"
#include "DirectInput.h"
#include "InputHook\InputHook.h"

CRITICAL_SECTION cs;

HINSTANCE g_hX360ceInstance = NULL;
HINSTANCE g_hNativeInstance = NULL;

HWND g_hWnd = NULL;

HINSTANCE g_hDInput8Instance = NULL;
DirectInput8CreateType g_DirectInput8Create = NULL;

iHook g_iHook;

void LoadSystemDInput8DLL()
{
	WCHAR sysdir[MAX_PATH];
	WCHAR buffer[MAX_PATH];

	// Getting path to system dir and to xinput1_3.dll
	GetSystemDirectory(sysdir,MAX_PATH);

	// Append dll name
	//wcscat_s(buffer,MAX_PATH,L"\\xinput1_3.dll");
	swprintf_s(buffer,L"%s\\%s",sysdir,L"dinput8.dll");

	// try to load the system's xinput dll, if pointer empty
	WriteLog(LOG_CORE,L"Loading %s",buffer);

	if (!g_hDInput8Instance)
		g_hDInput8Instance = LoadLibrary(buffer);

	//Debug
	if (!g_hDInput8Instance)
	{
		WriteLog(LOG_CORE,L"Cannot load %s error: 0x%x", buffer, GetLastError());
		WriteLog(LOG_CORE,L"x360ce will exit now!!!");
		ExitProcess(1); // exit the hard way
	}
	else
	{
		g_DirectInput8Create = (DirectInput8CreateType) GetProcAddress( g_hDInput8Instance, "DirectInput8Create");
	}
}

void LoadSystemXInputDLL()
{
    WCHAR sysdir[MAX_PATH];
    WCHAR buffer[MAX_PATH];

    // Getting path to system dir and to xinput1_3.dll
    GetSystemDirectory(sysdir,MAX_PATH);

    // Append dll name
    //wcscat_s(buffer,MAX_PATH,L"\\xinput1_3.dll");
    swprintf_s(buffer,L"%s\\%s",sysdir,DLLFileName(g_hX360ceInstance));

    // try to load the system's xinput dll, if pointer empty
    WriteLog(LOG_CORE,L"Loading %s",buffer);

    if (!g_hNativeInstance) g_hNativeInstance = LoadLibrary(buffer);

    //Debug
    if (!g_hNativeInstance)
    {
        WriteLog(LOG_CORE,L"Cannot load %s error: 0x%x", buffer, GetLastError());
        WriteLog(LOG_CORE,L"x360ce will exit now!!!");
        ExitProcess(1); // exit the hard way
    }
    else if(bInitBeep) MessageBeep(MB_ICONASTERISK);
}

SHORT ConfiguredPadCount()
{
    SHORT configuredpads = 0;

    for(int i = 0; i < 4; i++)
    {
        if(g_Gamepad[i].configured) ++configuredpads;
    }

    return configuredpads;
}

VOID InstallInputHooks()
{
    //x360ce_InputHookConfig.sConfiguredPads = ConfiguredPadCount();

	iHookPadConfig padconf[4];
    if(g_iHook.GetState())
    {
        for(WORD i = 0; i < ConfiguredPadCount(); i++)
        {
            if(g_Gamepad[i].configured) padconf[i].Enable();
            padconf[i].SetProductGUID(g_Gamepad[i].productGUID);
            padconf[i].SetInstanceGUID(g_Gamepad[i].instanceGUID);
			g_iHook.AddHook(padconf[i]);
        }
    }

	g_iHook.ExecuteHooks();
}

VOID InitInstance(HINSTANCE hinstDLL)
{

#ifdef _DEBUG
    int CurrentFlags;
    CurrentFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    CurrentFlags |= _CRTDBG_DELAY_FREE_MEM_DF;
	CurrentFlags |= _CRTDBG_ALLOC_MEM_DF;
    CurrentFlags |= _CRTDBG_LEAK_CHECK_DF;
    //CurrentFlags |= _CRTDBG_CHECK_ALWAYS_DF;
    _CrtSetDbgFlag(CurrentFlags);
#endif

	InitializeCriticalSection(&cs);
	EnterCriticalSection(&cs);

    g_hX360ceInstance =  hinstDLL;

    DWORD dwAppPID = GetCurrentProcessId();
    ReadConfig("x360ce.ini");
    Console();
    LogEnable(CreateLog());
    WriteStamp();

#if SVN_MODS != 0
    WriteLog(LOG_CORE,L"x360ce %d.%d.%d.%dM [%s - %d]",VERSION_MAJOR,VERSION_MINOR,VERSION_PATCH,SVN_REV,HostFileName(),dwAppPID);
#else
    WriteLog(LOG_CORE,L"x360ce %d.%d.%d.%d [%s - %d]",VERSION_MAJOR,VERSION_MINOR,VERSION_PATCH,SVN_REV,HostFileName(),dwAppPID);
#endif

    WriteLog(LOG_CORE,L"http://code.google.com/p/x360ce");

	LoadSystemDInput8DLL();
	LoadSystemXInputDLL();

    InstallInputHooks();
	LeaveCriticalSection(&cs);
}

VOID ExitInstance()
{
	EnterCriticalSection(&cs);

    if (g_hNativeInstance)
    {
        FreeLibrary(g_hNativeInstance);
        g_hNativeInstance = NULL;
    }

    if(IsWindow(g_hWnd))
		DestroyWindow(g_hWnd);

    g_hWnd = NULL;
    UnregisterClass(L"x360ceWClass",g_hX360ceInstance);

    WriteLog(LOG_CORE,L"x360ce terminating, bye");

    LogCleanup();

	LeaveCriticalSection(&cs);
	DeleteCriticalSection(&cs);
}

extern "C" BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved )
{
    UNREFERENCED_PARAMETER(lpReserved);

    switch( fdwReason )
    {
    case DLL_PROCESS_ATTACH:
        InitInstance(hinstDLL);
        break;

    case DLL_PROCESS_DETACH:
        ExitInstance();
        break;
    }

    return TRUE;
}