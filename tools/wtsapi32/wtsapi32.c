#include <windows.h>
#include <tlhelp32.h>
#include <stdlib.h>
#include <string.h>

typedef struct _WTS_PROCESS_INFOA {
    DWORD SessionId;
    DWORD ProcessId;
    LPSTR pProcessName;
    PSID  pUserSid;
} WTS_PROCESS_INFOA, *PWTS_PROCESS_INFOA;

typedef struct _WTS_PROCESS_INFOW {
    DWORD SessionId;
    DWORD ProcessId;
    LPWSTR pProcessName;
    PSID  pUserSid;
} WTS_PROCESS_INFOW, *PWTS_PROCESS_INFOW;

/* Single allocation: array followed by name strings, so WTSFreeMemory(ptr) frees all. */
BOOL WINAPI WTSEnumerateProcessesA(HANDLE server, DWORD reserved, DWORD version,
                                   PWTS_PROCESS_INFOA *ppProcessInfo, DWORD *pCount)
{
    HANDLE snap;
    PROCESSENTRY32 pe;
    DWORD count = 0, names = 0, i = 0;
    BYTE *buf; char *strp; PWTS_PROCESS_INFOA arr;

    if (!ppProcessInfo || !pCount) { SetLastError(ERROR_INVALID_PARAMETER); return FALSE; }
    if (reserved != 0 || version != 1) { SetLastError(ERROR_INVALID_PARAMETER); return FALSE; }

    snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) { SetLastError(ERROR_ACCESS_DENIED); return FALSE; }

    pe.dwSize = sizeof(pe);
    if (Process32First(snap, &pe)) {
        do { count++; names += (DWORD)strlen(pe.szExeFile) + 1; } while (Process32Next(snap, &pe));
    }
    if (count == 0) { CloseHandle(snap); SetLastError(ERROR_ACCESS_DENIED); return FALSE; }

    buf = (BYTE *)LocalAlloc(LPTR, count * sizeof(WTS_PROCESS_INFOA) + names);
    if (!buf) { CloseHandle(snap); SetLastError(ERROR_NOT_ENOUGH_MEMORY); return FALSE; }

    arr  = (PWTS_PROCESS_INFOA)buf;
    strp = (char *)(buf + count * sizeof(WTS_PROCESS_INFOA));

    pe.dwSize = sizeof(pe);
    if (Process32First(snap, &pe)) {
        do {
            size_t n;
            if (i >= count) break;
            n = strlen(pe.szExeFile) + 1;
            memcpy(strp, pe.szExeFile, n);
            arr[i].SessionId    = 0;
            arr[i].ProcessId    = pe.th32ProcessID;
            arr[i].pProcessName = strp;
            arr[i].pUserSid     = NULL;
            strp += n;
            i++;
        } while (Process32Next(snap, &pe));
    }
    CloseHandle(snap);

    *ppProcessInfo = arr;
    *pCount = i;
    return TRUE;
}

BOOL WINAPI WTSEnumerateProcessesW(HANDLE server, DWORD reserved, DWORD version,
                                   PWTS_PROCESS_INFOW *ppProcessInfo, DWORD *pCount)
{
    HANDLE snap; PROCESSENTRY32W pe;
    DWORD count = 0, names = 0, i = 0;
    BYTE *buf; WCHAR *strp; PWTS_PROCESS_INFOW arr;

    if (!ppProcessInfo || !pCount) { SetLastError(ERROR_INVALID_PARAMETER); return FALSE; }
    if (reserved != 0 || version != 1) { SetLastError(ERROR_INVALID_PARAMETER); return FALSE; }

    snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) { SetLastError(ERROR_ACCESS_DENIED); return FALSE; }

    pe.dwSize = sizeof(pe);
    if (Process32FirstW(snap, &pe)) {
        do { count++; names += (DWORD)(wcslen(pe.szExeFile) + 1); } while (Process32NextW(snap, &pe));
    }
    if (count == 0) { CloseHandle(snap); SetLastError(ERROR_ACCESS_DENIED); return FALSE; }

    buf = (BYTE *)LocalAlloc(LPTR, count * sizeof(WTS_PROCESS_INFOW) + names * sizeof(WCHAR));
    if (!buf) { CloseHandle(snap); SetLastError(ERROR_NOT_ENOUGH_MEMORY); return FALSE; }

    arr  = (PWTS_PROCESS_INFOW)buf;
    strp = (WCHAR *)(buf + count * sizeof(WTS_PROCESS_INFOW));

    pe.dwSize = sizeof(pe);
    if (Process32FirstW(snap, &pe)) {
        do {
            size_t n;
            if (i >= count) break;
            n = wcslen(pe.szExeFile) + 1;
            memcpy(strp, pe.szExeFile, n * sizeof(WCHAR));
            arr[i].SessionId    = 0;
            arr[i].ProcessId    = pe.th32ProcessID;
            arr[i].pProcessName = strp;
            arr[i].pUserSid     = NULL;
            strp += n;
            i++;
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);

    *ppProcessInfo = arr;
    *pCount = i;
    return TRUE;
}

VOID WINAPI WTSFreeMemory(PVOID pMemory)
{
    if (pMemory) LocalFree((HLOCAL)pMemory);
}

BOOL WINAPI WTSFreeMemoryExA(int cls, PVOID pMemory, ULONG count)
{ if (pMemory) LocalFree((HLOCAL)pMemory); return TRUE; }

BOOL WINAPI WTSFreeMemoryExW(int cls, PVOID pMemory, ULONG count)
{ if (pMemory) LocalFree((HLOCAL)pMemory); return TRUE; }

HANDLE WINAPI WTSOpenServerA(LPSTR n)  { return (HANDLE)1; }
HANDLE WINAPI WTSOpenServerW(LPWSTR n) { return (HANDLE)1; }
VOID   WINAPI WTSCloseServer(HANDLE h) { }

BOOL WINAPI DllMain(HINSTANCE h, DWORD reason, LPVOID r)
{ if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(h); return TRUE; }

/* auto stubs */
BOOL WINAPI WTSConnectSessionA(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSConnectSessionW(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSDisconnectSession(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSEnableChildSessions(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSEnumerateProcessesExA(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSEnumerateProcessesExW(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSEnumerateServersA(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSEnumerateServersW(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSEnumerateSessionsA(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSEnumerateSessionsExA(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSEnumerateSessionsExW(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSEnumerateSessionsW(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSLogoffSession(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSOpenServerExA(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSOpenServerExW(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSQuerySessionInformationA(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSQuerySessionInformationW(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSQueryUserConfigA(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSQueryUserConfigW(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSQueryUserToken(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSRegisterSessionNotification(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSRegisterSessionNotificationEx(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSSendMessageA(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSSendMessageW(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSSetSessionInformationA(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSSetSessionInformationW(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSSetUserConfigA(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSSetUserConfigW(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSShutdownSystem(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSStartRemoteControlSessionA(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSStartRemoteControlSessionW(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSStopRemoteControlSession(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSTerminateProcess(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSUnRegisterSessionNotification(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSUnRegisterSessionNotificationEx(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSVirtualChannelClose(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSVirtualChannelOpen(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSVirtualChannelOpenEx(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSVirtualChannelPurgeInput(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSVirtualChannelPurgeOutput(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSVirtualChannelQuery(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSVirtualChannelRead(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSVirtualChannelWrite(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
BOOL WINAPI WTSWaitSystemEvent(void) { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE; }
