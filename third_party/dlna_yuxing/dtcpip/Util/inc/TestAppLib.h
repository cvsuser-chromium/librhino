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

#ifndef __TEST_APP_LIB_H__
#define __TEST_APP_LIB_H__

#define MAX_ARG_SIZE 256
#define MAX_LBL_SIZE 64
#define DEFAULT_COMMAND_COUNT 6
#define MAX_MENU_HIST 100 //the max number of commands the history will hold
#define MAX_MENU_LOOP_ARGS 32 //the max number of Args that can be used with a loop in a menu

#define SUCCESS 0
#define ERROR __LINE__ 

#define CHECK(cond)		\
	if(!(cond))			\
	{					\
		return ERROR;	\
	}					\

#define CHECK_MSG(cond, msg)\
	if(!(cond))				\
	{						\
		printf("%s\n", msg);\
		return ERROR;		\
	}						\


typedef int (*Func_Ptr)(int argc, char *argv[]);

typedef struct _command_
{
    int       CommandNumber;
    char     *CommandName;
    char     *CommandDescription;
    Func_Ptr  CommandFunc;
} COMMAND;



//this is for the defualt commands
typedef struct _default_command_
{
    char      CommandID;
    char*	  CommandDescription;
} DEFAULT_COMMAND;

//the stack for use with loop labels, when the menu is used
typedef struct _label_node_
{
	struct LABEL_NODE* next;
	char label[MAX_LBL_SIZE];
	int lblLength;
} LABEL_NODE;

#ifdef _TRS_CPP_COMPILER
#ifndef ICL_MANGLE_SYMBOLS
#pragma public_c_interface(Push, Pop, RunTest)
#endif
#endif


void Push (LABEL_NODE **, char*, int);
char* Pop (LABEL_NODE **, int*);





int RunTest(int argc, char *argv[], int aCommandCount, COMMAND *aCommands);

#endif // __TEST_APP_LIB_H__
