//#############################################################################
//## Copyright (c) 2004 Intel Corporation All Rights Reserved. 
//## 
//## The source code contained or described herein and all documents related to
//## the source code ("Material") are owned by Intel Corporation or its 
//## suppliers or licensors. Title to the Material remains with Intel 
//## Corporation or its suppliers and licensors. The Material contains trade 
//## secrets and proprietary and confidential information of Intel or its
//## suppliers and licensors. The Material is protected by worldwide copyright
//## and trade secret laws and treaty provisions. No part of the Material may 
//## be used, copied, reproduced, modified, published, uploaded, posted, 
//## transmitted, distributed, or disclosed in any way without Intel's prior 
//## express written permission.
//## 
//## No license under any patent, copyright, trade secret or other 
//## intellectual property right is granted to or conferred upon you by 
//## disclosure or delivery of the Materials, either expressly, by 
//## implication, inducement, estoppel or otherwise. Any license under such 
//## intellectual property rights must be express and approved by Intel in 
//## writing.
//#############################################################################

/// \file
/// Implementation file for OsWrapper.



#include "OsWrapper.h"
#include "StatusCodes.h"

#ifdef WINDOWS_BUILD
#define gettimeofday(x,y)	(x)->tv_sec = GetTickCount()/1000;(x)->tv_usec = 1000*(GetTickCount()%1000)

#include <windows.h>

#else
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <semaphore.h>

#endif

#include <stdio.h>

#include <time.h>


/* Add by Gaocn 2005_6_21 µ÷ÊÔÓÃ */
void  PrintTime( void )
{
	int		 ms, s, m, h;
	struct timeval time_x;

//	memset(&time_x, 0, sizeof(struct timeval));
     gettimeofday( &time_x, NULL );
	ms = time_x.tv_usec / 1000;
	s  = time_x.tv_sec % 60;
	m  = ( time_x.tv_sec % 3600 ) / 60;
	h  = time_x.tv_sec / 3600;
	printf("\n--[%2d:%2d:%2d:%3d]--", h, m, s, ms );
}

///////////////////////////////////////////////////////////////
//// Macros and typedefs
///////////////////////////////////////////////////////////////

/// \brief defines a non-zero value for "true"
#define TRUE   1

/// \brief defines a zero value for "false"
#define FALSE  0

typedef struct __ThreadData__
{
    void                      * ThreadParameter;
    OsWrap_ThreadFunction_Ptr   Function;
} ThreadData;


///////////////////////////////////////////////////////////////
//// Global Variables
///////////////////////////////////////////////////////////////

#if 0
static int GlobalDisplayLevel             = -1;
static int GlobalDisplayLevelMatchExactly = 0;
static int GlobalLogToFile                = 0;
#endif


static int
WindowsSocketsInitialized = FALSE;



#ifdef WINDOWS_BUILD
static DWORD WINAPI 
Win32ThreadProc(LPVOID lpParameter)
{
    int retVal;

    ThreadData   threadData;

    memcpy(&threadData, (ThreadData *) lpParameter, sizeof(ThreadData));

    free(lpParameter);

    retVal = threadData.Function(threadData.ThreadParameter);

    return ((DWORD) retVal);
}
#else
typedef void *
(*LinuxThread_Ptr)(void * aParameter);
#endif


/// \brief Spawns a new thread of execution in the specified function.
///
/// Will spawn a new thread of execution using the input parameters.
/// User must provide a function pointer.  
/// aThreadID and aThreadHandle pointers are optional but must be
/// provided if the user wants external control over the child thread.
/// aCreationFlags is only used on Win32.
int
OsWrap_NewThread(OsWrap_ThreadFunction_Ptr   aFunction, 
                 void                      * aParameter, 
                 unsigned long               aCreationFlags, 
                 void                     ** aThreadID,
                 void                     ** aThreadHandle)
{
    int returnValue = SUCCESS;

#ifdef WINDOWS_BUILD
    ThreadData * threadData;

    HANDLE handle; 

    threadData = (ThreadData *) malloc(sizeof(ThreadData));

    threadData->ThreadParameter = aParameter;
    threadData->Function        = aFunction;

    handle = CreateThread(NULL,
                          0,
                          Win32ThreadProc,
                          (LPVOID) threadData,
                          (DWORD) aCreationFlags,
                          (LPDWORD) aThreadID);

    if (aThreadHandle)
    {
        *aThreadHandle = (void *) handle;
    }

    if (!handle)
    {
        returnValue = FAILURE;
        free(threadData);
    }
#else
    if (aThreadID)
    {
        *aThreadID = NULL;
    }

    if (pthread_create((pthread_t *) aThreadHandle,
                       NULL,
                       (LinuxThread_Ptr) aFunction,
                       aParameter))
    {
        returnValue = FAILURE;
    }

#endif

    return (returnValue);
}

/// \brief Forcibly terminate a spawned thread of execution.
///
/// Will forcibly terminate a thread of execution.  Any resources
/// that had been allocated by the thread will be lost so use only
/// in extreme circumstances.
extern int
OsWrap_KillThread(void            * aThreadID,
                  void            * aThreadHandle)
{
    int returnValue = SUCCESS;

#ifdef WINDOWS_BUILD
    if (0 == TerminateThread((HANDLE) aThreadHandle, 0))
#else
    if (pthread_kill((pthread_t) aThreadHandle, SIGTERM))
#endif
    {
        returnValue = FAILURE;
    }

    return (returnValue);
}

/// \brief Sleeps current thread for specified milliseconds
///
/// Sleeps current thread for specified milliseconds.  There is no
/// specified accuracy for this function so don't use for precise timing
/// analysis.  Anything less than 2 seconds will probably not be very
/// accurate.
/// \todo Should probably run some experiments so we can specify gross
/// margin of error on OsWrap_Sleep.
void
OsWrap_Sleep(int aSleepTime)
{
#ifdef WINDOWS_BUILD
    Sleep(aSleepTime);
#else
    usleep(aSleepTime * 1000);
#endif
}

/// \brief Converts ASCII string to an integer value
///
/// Used for processing input parameters that are in ASCII
/// format but need to be used in binary.
int
OsWrap_GetIntegerFromString(char * aString)
{
    int integer = 0;

    // If the user passes in a mal-formed string
    // he should get a value of zero

    sscanf(aString, "%d", &integer);

    return (integer);
}

int
OsWrap_SemaphoreInit(
    void * aSemHandle,
    long   aSemMaxValue)
{

#ifndef WINDOWS_BUILD
    sem_t * sem = malloc(sizeof(sem_t));
    int sysRet;

    if (sem_init(sem,
                 0,
                 (int)aSemMaxValue))
    {
        return (FAILURE);
    }
    else
    {
        *((sem_t **) aSemHandle) = sem;
        return (SUCCESS);
    }
    
#else  //windows
    *(HANDLE *)aSemHandle = CreateSemaphore(
                   NULL,
                   aSemMaxValue,     //inital count
                   aSemMaxValue,     //semaphore maximum count
                   NULL);
    
    if(!(*(HANDLE *)aSemHandle))
    {
        return FAILURE;
    }
    else
    {
        return SUCCESS;
    }
#endif //ifndef WINDOWS_BUILD 

}

int
OsWrap_SemaphoreClose( void * aSemHandle )
{
#ifndef WINDOWS_BUILD

    if (aSemHandle)
    {
        sem_destroy((sem_t *) aSemHandle);
        free(aSemHandle);
    }

    return (SUCCESS);
    
#else  //windows

    return ((CloseHandle((HANDLE) aSemHandle)) ? SUCCESS : FAILURE);

#endif //ifndef WINDOWS_BUILD 
}

int
OsWrap_SemaphoreWait(  void * aSemHandle )
{
#ifndef WINDOWS_BUILD
    sem_wait((sem_t *)aSemHandle); 
    return SUCCESS;
#else  //windows

    WaitForSingleObject(
        (HANDLE *)aSemHandle,
        INFINITE);
    return 0;             
#endif //ifndef WINDOWS_BUILD
}

int
OsWrap_SemaphorePost( void * aSemHandle )
{
#ifndef WINDOWS_BUILD
    if ( sem_post( (sem_t *)aSemHandle))
    {
        return FAILURE;
    }

#else  //windows
    if( !ReleaseSemaphore(
        (HANDLE *)aSemHandle,
        1,
        NULL))
    {
        return FAILURE;  
    }

#endif //ifndef WINDOWS_BUILD

    return SUCCESS;
}

#ifdef LINUX_BUILD
static sem_t ProcessEventSemaphore;
static int   ProcessEventSemaphoreInitialized = FALSE;
#endif


/// \brief Open a new event to use for thread synchronization
///
/// \todo implement manual reset for CreatEvent on Linux.
EVENT_HANDLE
OsWrap_CreateEvent(unsigned int aManualReset, unsigned int aInitialState)
{
#ifdef WINDOWS_BUILD
    return ((EVENT_HANDLE) CreateEvent(NULL, aManualReset, aInitialState, NULL));
#else
    sem_t * sem = malloc(sizeof(sem_t));
    int sysRet;

    if (!ProcessEventSemaphoreInitialized)
    {
        sem_init(&ProcessEventSemaphore, 0, 1);
        ProcessEventSemaphoreInitialized = TRUE;
    }

    // We don't have auto reset implemented for linux
    if (aManualReset && ProcessEventSemaphoreInitialized)
    {
        sysRet = sem_init(sem, 0, (aInitialState) ? 1 :0);
    }

    return ((EVENT_HANDLE) sem);
#endif
}

/// \brief Close an event
///
int
OsWrap_CloseEvent(EVENT_HANDLE aHandle)
{
#ifdef WINDOWS_BUILD
    CloseHandle(aHandle);
#else
    if (aHandle)
    {
        sem_destroy((sem_t *) aHandle);
        free(aHandle);
    }
#endif
    return (SUCCESS);
}

/// \brief Set an events state to signaled
///
int
OsWrap_SetEvent(EVENT_HANDLE aHandle)
{
#ifdef WINDOWS_BUILD
    return (SetEvent((HANDLE) aHandle));
#else
    int returnValue = FAILURE;
    unsigned int value = 0;

    if (aHandle)
    {
        returnValue = SUCCESS;

        sem_wait(&ProcessEventSemaphore);

        sem_getvalue((sem_t *) aHandle, &value);

        if (!value)
        {
            sem_post((sem_t *) aHandle);
        }

        sem_post(&ProcessEventSemaphore);
    }

    return (returnValue);
#endif
}

/// \brief Set an events state to not-signaled
///
int
OsWrap_ResetEvent(EVENT_HANDLE aHandle)
{
#ifdef WINDOWS_BUILD
    return (ResetEvent((HANDLE) aHandle));
#else
    int returnValue = FAILURE;
    unsigned int value = 0;

    if (aHandle)
    {
        returnValue = SUCCESS;

        sem_wait(&ProcessEventSemaphore);

        sem_getvalue((sem_t *) aHandle, &value);

        if (value)
        {
            sem_wait((sem_t *) aHandle);
        }

        sem_post(&ProcessEventSemaphore);
    }

    return (returnValue);
#endif
}

/// \brief Block until an event becomes signaled
///
/// aTimeOut is specified in milliseconds.
int
OsWrap_WaitForEvent(EVENT_HANDLE aHandle, int aTimeOut)
{
    int retVal = OSWRAP_WAIT_UNKNOWN;

#ifdef WINDOWS_BUILD
    DWORD timeout;
    DWORD sysRetVal;
#else
    unsigned int value     = 0;
    int          startTime = OsWrap_GetCurrentTime();
#endif

    if (!aHandle)
    {
        return (retVal);
    }

#ifdef WINDOWS_BUILD
    timeout = (OSWRAP_WAIT_INFINITE == aTimeOut) ? INFINITE : (DWORD) aTimeOut;

    sysRetVal = WaitForSingleObject((HANDLE) aHandle, timeout);

    switch (sysRetVal)
    {
    case WAIT_OBJECT_0:
        retVal = OSWRAP_WAIT_SIGNALED;
        break;
    case WAIT_TIMEOUT:
        retVal = OSWRAP_WAIT_TIMEOUT;
        break;
    case WAIT_ABANDONED:
        retVal = OSWRAP_WAIT_ABANDONED;
        break;
    default:
        retVal = OSWRAP_WAIT_UNKNOWN;
    }

#else

    while (1)
    {
        sem_getvalue((sem_t *) aHandle, &value);
        if (value)
        {
            retVal = OSWRAP_WAIT_SIGNALED;
            break;
        }

        OsWrap_Sleep(1);

        if (OSWRAP_WAIT_INFINITE != aTimeOut)
        {
            if (((float) aTimeOut / 1000) < OsWrap_GetElapsedTime(startTime))
            {
                retVal = OSWRAP_WAIT_TIMEOUT;
                break;
            }
        }
    }

#endif

    return (retVal);
}

/// \brief Return the value of the ANSI C clock function
///
int OsWrap_GetCurrentTime()
{
    return ((int) clock());
}

/// \brief Return elapsed time in seconds as a float value
///
float OsWrap_GetElapsedTime(int aStartTime)
{
    return (((float) (OsWrap_GetCurrentTime() - aStartTime)) / (float) CLOCKS_PER_SEC);
}

/// \brief Add an entry to a list
///
int
OsWrap_AddToList(
    LinkedList  ** aList,
    void        ** aEntry,
    int            aEntryLength)
{
    int returnValue = FAILURE;

    if (aList && aEntry)
    {
        LinkedList * newListMember;

        newListMember = malloc(sizeof(LinkedList));
        if (newListMember)
        {
            newListMember->Memory = malloc(aEntryLength);
            newListMember->Next   = NULL;
            if (newListMember->Memory)
            {
                memset(newListMember->Memory, 0, aEntryLength);
                newListMember->Length = aEntryLength;
                returnValue = SUCCESS;
            }
            else
            {
                free(newListMember);
            }
        }

        if (IS_SUCCESS(returnValue))
        {
            // Insert new list member at the beginning of the array
            if (*aList)
            {
                newListMember->Next = (void *) *aList;
            }
            *aList = newListMember;

            // Return pointer to allocated memory
            *aEntry = newListMember->Memory;
        }
    }

    return (returnValue);
}

/// \brief Check to see if an entry is in a list.  Return value is boolean
///
int
OsWrap_EntryIsInList(
    LinkedList  ** aList,
    void         * aEntry)
{
    int returnValue = 0;

    if (aList && aEntry)
    {
        LinkedList * currentEntry = *aList;
        while (currentEntry)
        {
            if (aEntry == currentEntry->Memory)
            {
                returnValue = 1;
                break;
            }
            currentEntry = (LinkedList *) currentEntry->Next;
        }
    }

    return (returnValue);
}

/// \brief Remove an entry from a list
///
int
OsWrap_RemoveFromList(
    LinkedList  ** aList,
    void         * aEntry)
{
    int returnValue = SUCCESS;

    if (aList && aEntry)
    {
        LinkedList * currentEntry = *aList;
        LinkedList * lastEntry    = NULL;;
        while (currentEntry)
        {
            if (aEntry == currentEntry->Memory)
            {
                if (lastEntry)
                {
                    lastEntry->Next = currentEntry->Next;
                }
                else
                {
                    *aList = (LinkedList *) currentEntry->Next;
                }
                memset(currentEntry->Memory, 0, currentEntry->Length);
                free(currentEntry->Memory);
                free(currentEntry);
                break;
            }
            lastEntry    = currentEntry;
            currentEntry = (LinkedList *) currentEntry->Next;
        }
    }
    else
    {
        returnValue = FAILURE;
    }

    return (returnValue);
}

/// \brief Release all resources allocated for a list
///
int
OsWrap_DeleteList(
    LinkedList ** aList)
{
    int returnValue = SUCCESS;

    if (aList)
    {
        LinkedList * currentEntry = *aList;
        LinkedList * nextEntry;
        while (currentEntry)
        {
            if (currentEntry->Memory)
            {
                memset(currentEntry->Memory, 0, currentEntry->Length);
                free(currentEntry->Memory);
            }
            nextEntry = (LinkedList *) currentEntry->Next;
            free(currentEntry);
            currentEntry = nextEntry;
        }
    }
    else
    {
        returnValue = FAILURE;
    }

    return (returnValue);
}

#ifndef DISABLE_MESSAGES

static unsigned char
StringFormatBuffer[2048];

/// \brief Provide a function that simplifies logging error and debug messages to a file
///
int
OsWrap_LogMessageToFile(
    char * fmt, ...)
{
    int returnValue = SUCCESS;

    FILE * file;

    va_list args;

    va_start(args, fmt);

    file = fopen("DtcpSdk.log", "a+");

    if (file)
    {
        vsprintf(StringFormatBuffer, fmt, args);
        if (strlen(StringFormatBuffer) != 
            fwrite(StringFormatBuffer, 
                   1, 
                   strlen(StringFormatBuffer), 
                   file)
           )
        {
            returnValue = FAILURE;
        }

        fclose(file);
    }
    else
    {
        returnValue = FAILURE;
    }

    va_end(args);

    return (returnValue);
}
#endif
