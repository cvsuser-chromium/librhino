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

#ifndef __STATUS_CODES_H__
#define __STATUS_CODES_H__

/// \file
/// \brief Defines status codes and helper macros for checking status codes
///
/// 
#include "OsWrapper.h"

#ifdef __cplusplus
extern  "C"
{
#endif

/* Add by Gaocn 2005_6_21 µ˜ ‘”√ */
extern void  PrintTime( void );

/// \brief Checks if a status code was successful
#define IS_SUCCESS(ReturnCode) (0 <= (ReturnCode))

/// \brief Checks if a status code was a failure
#define IS_FAILURE(ReturnCode) (0 >  (ReturnCode))

/// \brief Checks if a status code was a particular success code
#define IS_SPECIFIC_SUCCESS(ReturnCode, SuccessCode) \
            ((SuccessCode) == (ReturnCode))

/// \brief Checks if a status code was a particular failure code
#define IS_SPECIFIC_FAILURE(ReturnCode, ErrorCode) \
            ((ErrorCode) == (ReturnCode))


#define SUCCESS                             0   ///< Function succeeded
#define SUCCESS_FALSE                       1

#define FAILURE                            -1   ///< General failure
#define INVALID_ARGUMENT                   -2   ///< Invalid function argument
#define LOGIC_ERROR                        -3   ///< Functional logic error
#define FILE_NOT_FOUND                     -4   ///< File not found

/// \brief Defines which error/debug messages to display
///
/// This variable needs to be instantiated somewhere in your module and then
/// set as you see fit.
/// A value of zero will cause all messages to be displayed.
/// A value of 2 will cause all messages with DisplayLevel 0 to 2 to be displayed.
extern int GlobalDisplayLevel;

/// \brief Defines the specific error/debug messages to display
///
/// This variable needs to be instantiated somewhere in your module and then
/// set as you see fit.
/// A value of one will cause only messages which match the error level
/// exactly to be displayed
extern int GlobalDisplayLevelMatchExactly;

/// \brief Defines whether error/debug message should be saved to a file.
///
/// This variable needs to be instantiated somewhere in your module and then
/// set as you see fit.
/// A non-zero value will cause messages to be logged to a file.
/// 1: Will log error messages to the file
/// 2: Will log debug messages to the file
/// 3: Will log both error and debug messages to the file
extern int GlobalLogToFile;

#define MSG_ALWAYS (0)
#define MSG_INFO   (1)
#define MSG_ERR    (10)
#define MSG_WARN   (20)
#define MSG_DEBUG  (30)

/// \brief Outputs an in moroy buffer to the debug display.
/// The DEBUG_BUF macro wraps this funtion for conditional compilation.
/// \param DisplayLevel - Input; The log display level to use
/// \param aBuffer - Input; Pointer to the memory region to display
/// \param aBufferSize - Input; Size (in bytes) to display from aBuffer
void PrintBuffer(int DisplayLevel, unsigned char *aBuffer, unsigned int aBufferSize);

#define DISABLE_MESSAGES
#ifdef DISABLE_MESSAGES

#define DEBUG_MSG_TIME(DisplayLevel, FormatString_AND_VariableParameterList)                \
    if ((0 == GlobalDisplayLevel                                                ) ||        \
        (GlobalDisplayLevelMatchExactly  && (DisplayLevel) == GlobalDisplayLevel) ||        \
        (!GlobalDisplayLevelMatchExactly && (DisplayLevel) <= GlobalDisplayLevel))          \
    {                                                                                       \
	   PrintTime( );                                                                       \
        printf FormatString_AND_VariableParameterList;                                      \
    }

#define DEBUG_MSG(DisplayLevel, FormatString_AND_VariableParameterList)
#define DEBUG_BUF(DisplayLevel, Buffer, BufferSize)
#define ERROR_MSG(DisplayLevel, ErrorCode, FormatString_AND_VariableParameterList)
#define DEBUG_STATEMENT(DisplayLevel, statement)

#else


/// \brief Prints a debug message
#define DEBUG_MSG_TIME(DisplayLevel, FormatString_AND_VariableParameterList)                \
    if ((0 == GlobalDisplayLevel                                                ) ||        \
        (GlobalDisplayLevelMatchExactly  && (DisplayLevel) == GlobalDisplayLevel) ||        \
        (!GlobalDisplayLevelMatchExactly && (DisplayLevel) <= GlobalDisplayLevel))          \
    {                                                                                       \
	   PrintTime( );                                                                       \
        /* printf ("[ %04d %s ]\r\n", __LINE__, __FILE__); */                               \
        printf FormatString_AND_VariableParameterList;                                      \
        if (1 < GlobalLogToFile)                                                            \
        {                                                                                   \
            OsWrap_LogMessageToFile ("%04d %s: ", __LINE__, __FILE__);                      \
            OsWrap_LogMessageToFile FormatString_AND_VariableParameterList;                 \
        }                                                                                   \
    }

/// \brief Prints a debug message
#define DEBUG_MSG(DisplayLevel, FormatString_AND_VariableParameterList)                     \
    if ((0 == GlobalDisplayLevel                                                ) ||        \
        (GlobalDisplayLevelMatchExactly  && (DisplayLevel) == GlobalDisplayLevel) ||        \
        (!GlobalDisplayLevelMatchExactly && (DisplayLevel) <= GlobalDisplayLevel))          \
    {                                                                                       \
        /* printf ("[ %04d %s ]\r\n", __LINE__, __FILE__); */                               \
        printf FormatString_AND_VariableParameterList;                                      \
        if (1 < GlobalLogToFile)                                                            \
        {                                                                                   \
            OsWrap_LogMessageToFile ("%04d %s: ", __LINE__, __FILE__);                      \
            OsWrap_LogMessageToFile FormatString_AND_VariableParameterList;                 \
        }                                                                                   \
    }

/// \brief Prints an error message
#define ERROR_MSG(DisplayLevel, ErrorCode, FormatString_AND_VariableParameterList)          \
    if ((0 == GlobalDisplayLevel                                                ) ||        \
        (GlobalDisplayLevelMatchExactly  && (DisplayLevel) == GlobalDisplayLevel) ||        \
        (!GlobalDisplayLevelMatchExactly && (DisplayLevel) <= GlobalDisplayLevel))          \
    {                                                                                       \
        printf ("%04d %s: Error:%d ", __LINE__, __FILE__, ErrorCode);                       \
        printf FormatString_AND_VariableParameterList;                                      \
        if (1 == GlobalLogToFile || 3 == GlobalLogToFile)                                   \
        {                                                                                   \
            OsWrap_LogMessageToFile ("%04d %s: Error:%d ", __LINE__, __FILE__, ErrorCode);  \
            OsWrap_LogMessageToFile FormatString_AND_VariableParameterList;                 \
        }                                                                                   \
    }

/// \brief Conditionally compiles/includes a code statement
#define DEBUG_STATEMENT(DisplayLevel, statement)                                            \
    if ((0 == GlobalDisplayLevel                                                ) ||        \
        (GlobalDisplayLevelMatchExactly  && (DisplayLevel) == GlobalDisplayLevel) ||        \
        (!GlobalDisplayLevelMatchExactly && (DisplayLevel) <= GlobalDisplayLevel))          \
    {                                                                                       \
        statement ;                                                                         \
    }

/// \brief Prints a memroy buffer
#define DEBUG_BUF(DisplayLevel, Buffer, BufferSize)                                         \
    if ((0 == GlobalDisplayLevel                                                ) ||        \
        (GlobalDisplayLevelMatchExactly  && (DisplayLevel) == GlobalDisplayLevel) ||        \
        (!GlobalDisplayLevelMatchExactly && (DisplayLevel) <= GlobalDisplayLevel))          \
    {                                                                                       \
        PrintBuffer(DisplayLevel, Buffer, BufferSize);                                      \
    }

#endif

#ifdef __cplusplus
}
#endif
#endif


