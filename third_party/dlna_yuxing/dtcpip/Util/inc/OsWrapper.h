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
/// \brief Definition file for OS Abstraction interface.
///
/// This header file defines a set of API's that provide an OS agnostic interface
/// that provides a variety of capabilities that are used in the DHome SDK.
/// The following capabilities are provided: Threading, Events, Semaphores,
/// Clock information, A simple list interface and a few other misc API's that were needed.

#ifndef __OS_WRAPPER_H
#define __OS_WRAPPER_H

#ifdef __cplusplus
extern "C"
{
#endif

/// \defgroup OsWrapper OsWrapper
/// \brief OS abstraction library
///
/// Provides the following capabilities: Threading, Events, Semaphores,
/// Clock information, A simple list interface and a few other misc API's that were needed.
/// @{

/// \brief Structure used for list management
typedef struct __linkedlist__
{
    void * Memory;   ///< pointer to memory allocated for list entry
    int    Length;   ///< length of allocated memory
    void * Next;     ///< pointer to next item in the list
} LinkedList;


/// \brief typedef for event handles
///
/// Used on event API's and internally maps to the appropriate
/// handle for each supported OS
typedef void * EVENT_HANDLE;

/// \brief typedef for semaphore handles
///
/// Used on semaphore API's and internally maps to the appropriate
/// handle for each supported OS
typedef void * SEM_HANDLE;

// OS agnostic return codes for OsWrap_WaitForEvent

/// \brief wait event has signalled
#define  OSWRAP_WAIT_SIGNALED     0

/// \brief wait event has timed out before being signalled
#define  OSWRAP_WAIT_TIMEOUT     -1

/// \brief wait event has been abandoned
#define  OSWRAP_WAIT_ABANDONED   -2

/// \brief wait event has returned with an unknown error
#define  OSWRAP_WAIT_UNKNOWN     -3

/// \brief OS agnostic definition for wait indefinitely
#define  OSWRAP_WAIT_INFINITE    -1

/// \brief thread function prototype
///
/// OS agnostic prototype for thread functions.
/// User must define a function with this prototype and pass
/// it in to OSWrap_NewThread.
typedef int (*OsWrap_ThreadFunction_Ptr)(void * aParameter);

extern int
OsWrap_NewThread(OsWrap_ThreadFunction_Ptr   aFunction, 
                 void                      * aParameter, 
                 unsigned long               aCreationFlags, 
                 void                     ** aThreadID,
                 void                     ** aThreadHandle);

extern int
OsWrap_KillThread(void                     * aThreadID,
                  void                     * aThreadHandle);

extern void
OsWrap_Sleep(int aSleepTime);

extern int
OsWrap_GetIntegerFromString(char * aString);

extern int
OsWrap_SemaphoreInit(
    void * aSemHandle,
    long   aSemMaxValue);

extern int
OsWrap_SemaphoreClose(
    void * aSemHandle);

extern int
OsWrap_SemaphoreWait(
    void * aSemHandle);

extern int
OsWrap_SemaphorePost(
    void * aSemHande);


extern EVENT_HANDLE
OsWrap_CreateEvent(unsigned int aManualReset, unsigned int aInitialState);

extern int
OsWrap_CloseEvent(EVENT_HANDLE aHandle);

extern int
OsWrap_SetEvent(EVENT_HANDLE aHandle);

extern int
OsWrap_ResetEvent(EVENT_HANDLE aHandle);

extern int
OsWrap_WaitForEvent(EVENT_HANDLE aHandle, int aTimeOut);

extern int
OsWrap_GetCurrentTime();

extern float
OsWrap_GetElapsedTime(int aStartTime);

extern int
OsWrap_AddToList(
    LinkedList  ** aList,
    void        ** aEntry,
    int            aEntryLength);

extern int
OsWrap_EntryIsInList(
    LinkedList  ** aList,
    void         * aEntry);

extern int
OsWrap_RemoveFromList(
    LinkedList  ** aList,
    void         * aEntry);

extern int
OsWrap_DeleteList(
    LinkedList  ** aList);

extern int
OsWrap_LogMessageToFile(
    char * fmt, ...);

/// @}

#ifdef __cplusplus
}
#endif

#endif
