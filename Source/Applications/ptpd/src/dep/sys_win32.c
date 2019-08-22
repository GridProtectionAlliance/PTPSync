/*-
 * Copyright (c) 2010-2011 Jan Breuer
 * Copyright (c) 2009-2011 George V. Neville-Neil, Steven Kreuzer,
 *                         Martin Burnicki, Gael Mace, Alexandre Van Kempen
 * Copyright (c) 2005-2008 Kendall Correll, Aidan Williams
 *
 * All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file   sys_win32.c
 * @date   Tue Jul 6 16:19:46 2011
 *
 * @brief  Code to call kernel time routines and also display server statistics.
 *
 *
 */

#include "../ptpd.h"

HANDLE TimerHandle;
HANDLE TimerSelectInterrupt = NULL;

void
getTimeWin32(TimeInternal * time)
{
    FILETIME           WindowsUTCTime;
    uint64_t            U64WindowsTime;

    GetSystemTimeAsFileTime(&WindowsUTCTime);
    U64WindowsTime = WindowsUTCTime.dwHighDateTime;
    U64WindowsTime <<= 32;
    U64WindowsTime += WindowsUTCTime.dwLowDateTime;

    time->nanoseconds = (Integer32)(U64WindowsTime % 10000000ULL);
    time->nanoseconds *= 100;

    U64WindowsTime /= 10000000ULL;
    U64WindowsTime -= 11644473600ULL;

    time->seconds = (Integer32)U64WindowsTime;
}

void
setTimeWin32(TimeInternal * time)
{
    uint64_t            U64WindowsTime;
    FILETIME           WindowsUTCTime;
    SYSTEMTIME         WindowsSystemTime;

    U64WindowsTime = time->seconds;

    /* Change epoch in seconds from January 1, 1970 to Windows January 1, 1901 */
    U64WindowsTime += 11644473600ULL;

    /* Convert seconds to 100 ns increments since January 1, 1601 */
    U64WindowsTime *= 10000000ULL;

    /* Add in nanoseconds converted to 100 ns increments */
    U64WindowsTime += time->nanoseconds / 100;

    /* Convert unsigned long long 64 bit variable to File time structure time */
    WindowsUTCTime.dwLowDateTime  = (unsigned long)(U64WindowsTime & 0xFFFFFFFFULL);
    WindowsUTCTime.dwHighDateTime = (unsigned long)(U64WindowsTime >> 32);

    /* Convert Windows UTC "file time" to Windows UTC "system time" */
    FileTimeToSystemTime(&WindowsUTCTime, &WindowsSystemTime);

    /* Now finally we can set the windows system time */
    SetSystemTime(&WindowsSystemTime);
}

Boolean
adjFreqWin32(Integer32 adj)
{
    if(adj > ADJ_FREQ_MAX)
        adj = ADJ_FREQ_MAX;
    else if(adj < -ADJ_FREQ_MAX)
        adj = -ADJ_FREQ_MAX;

    /* 100 ns units */
    return SetSystemTimeAdjustment(adj/100/CLOCK_INTERRUPTS_PER_SECOND, FALSE);
}

int inet_aton(const char* cp, struct in_addr* inp)
{
    struct in_addr a;
    a.s_addr = inet_addr(cp);

    if(a.s_addr == INADDR_NONE)
        return 0;

    *inp = a;
    return a.s_addr;
}

Boolean
EnableSystemTimeAdjustment(void)
{
    HANDLE TokenHandle;
    TOKEN_PRIVILEGES NewState;
    Boolean result = TRUE;
    /* Check windows >= XP */

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TokenHandle))
    {
        PERROR("OpenProcessToken");
        result = FALSE;
        goto exit;
    }

    NewState.PrivilegeCount = 1;
    NewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if(!LookupPrivilegeValue(NULL, SE_SYSTEMTIME_NAME, &NewState.Privileges[0].Luid))
    {
        PERROR("LookupPrivilegeValue");
        result = FALSE;
        goto exit;
    }

    if (!AdjustTokenPrivileges(TokenHandle, FALSE, &NewState, 0, NULL, NULL))
    {
        PERROR("AdjustTokenPrivileges");
        result = FALSE;
        goto exit;
    }

exit:
    if(TokenHandle)
    {
        CloseHandle(TokenHandle);
    }

    return result;
}

VOID CALLBACK
SetITimerCalback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    void (*callback)(int) = lpParameter;
    /* call catch_alarm */
    callback(0);
    /* interrupt netSelect as SIGALRM do */
    if (TimerSelectInterrupt) SetEvent(TimerSelectInterrupt);
}

int
setitimerWin32(const struct timeval * tv, void * callback)
{
    DWORD Period = (tv->tv_sec * 1000) + (tv->tv_usec / 1000);
    BOOL Succes;

    if (Period == 0) {
        if (TimerSelectInterrupt)
            CloseHandle(TimerSelectInterrupt);
        DeleteTimerQueueTimer(NULL, TimerHandle, NULL);
        return 0;
    }

    if (!TimerSelectInterrupt) {
        TimerSelectInterrupt = CreateEvent(NULL, TRUE, FALSE, TEXT("IntervalTimer"));
    }

    Succes = CreateTimerQueueTimer(&TimerHandle, NULL, SetITimerCalback,
            callback, 0, Period,  WT_EXECUTEINIOTHREAD);

    if(Succes == TRUE) {
        return 0;
    } else {
        errno = EFAULT;
        return -1;
    }
}