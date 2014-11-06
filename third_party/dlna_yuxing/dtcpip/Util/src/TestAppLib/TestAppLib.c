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

#include "TestAppLib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef WINCE_BUILD
#include <memory.h>
#endif


static COMMAND *gCommands;
static int      gCommandCount;


int ProcessParameters(int argc, char * argv[]);
char *getCmd(char*, int*);



//defualt Commands for use with the menu
DEFAULT_COMMAND defaultCommands[DEFAULT_COMMAND_COUNT] = 
{
    { 'l', "loop: loop commands ( l:<loop_label>:<loop_count> )\n" },
    { 'e', "end loop: end the innermost loop\n" },
    { 'p', "list previous commands:\n   p lists all previous commands, p:n lists the n most recent commands\n" },
    { 'x', "execute previous command:\n   x:n executes command n in list of previous commands\n" },
    { 'h', "help: explanation of default commands\n" },
    { 'q', "quit\n" },
};



void DisplayUsage()
{
    int i = 0;

    printf(
        "Usage:\n"
        "Valid parameters are:\n"
        "-file:<name>     Retrieve commands from file <name>\n"
        "-menu            Use the app in menu mode instead of cmd line mode\n"
        "-loop:<loop_label>:<loop_count> <command:args> -endloop:<loop_label>    Loop the commands\n");
    for (i = 0; i < gCommandCount; i++)
    {
        printf("%-17s%s", 
               gCommands[i].CommandName, 
               gCommands[i].CommandDescription);
    }
}

// This routine is used to simplify getting the extra level of pointer
// indirection needed by the ProcessParameters function.
void
ProcessMenuCommand(char * aCommand)
{
    ProcessParameters(1, &aCommand);
}

int
UseMenuToGetCommands()
{
    int returnValue = 0;

    int     command = -1;
    char    ccommand;
    char  * inputBuffer;

    int     i;

    //used for history feature
    char *hist[MAX_MENU_HIST];
    int numHist = 0;
    int histOverflowCount = 0; // this is to handle the ideosyncracies when hist gets larger then MAX_MENU_HIST

    //this is used to decide how to process the commands
    int loopMode = 0;
    //this is used to process the execute previous command
    int xMode = 0;

    char* argList[MAX_MENU_LOOP_ARGS];
    int argNum = 0;

    LABEL_NODE * head = NULL;

    while (command != 0)
    {
        if (!xMode)
        {
            inputBuffer = malloc(512 * sizeof(char));

            if (!loopMode)
            {
                for (i = 0; i < gCommandCount; i++)
                {
                    printf("%d: %s", 
                           gCommands[i].CommandNumber, 
                           gCommands[i].CommandDescription);
                }
            }

            printf("Enter Command --> ");
        }

        // For some reason, reading integers directly from the command line causes
        // major problems when an invalid digit is entered.  The stack seems to become
        // corrupted and we get into an infinite loop and have to use Crtl-C to get out.
        // This behaviour has been observed on both Windows and Linux and we have been
        // unable to determine if it is our problem or a problem with the underlying
        // implementation of scanf.  The solution was to read a string from the
        // command line and then scan that string for the desired integers.
        if (!xMode)
        {
            int i = 0;
            while ('\n' != (inputBuffer[i++] = getchar()));
            inputBuffer[i-1] = '\0';
        }


        if (xMode || 0 < strlen(inputBuffer))
        {
            int commandFound = 0;
            int i;
            
            xMode = 0;
            
            if (1 == sscanf(inputBuffer, "%d", &command))
            {
                for (i = 0; i < gCommandCount; i++)
                {
                    if (gCommands[i].CommandNumber == command)
                    {
                        char *colon = strchr(inputBuffer, ':');
                        if (colon)
                        {
                            char *command;
                                
                            command = malloc(1000);
                            
                            memset(command, 0, 1000);
                            strcpy(command, gCommands[i].CommandName);
                            strcat(command + strlen(command), colon);
                            
                            if (loopMode)
                            {
                                argList[argNum] = malloc(MAX_ARG_SIZE);
                                strcpy(argList[argNum], command);
                                argNum++;

                                free(command);
                            }
                            else
                            {
                                ProcessParameters(1, &command);
                                free(command);
                            }
                        }
                        else
                        {
                            if (loopMode)
                            {
                                argList[argNum] = malloc(MAX_ARG_SIZE);
                                strcpy(argList[argNum], gCommands[i].CommandName);
                                argNum++;
                            }
                            else
                            {
                                ProcessMenuCommand(gCommands[i].CommandName);
                            }
                        }
                        commandFound = 1;
                    }
                }
            }
            else
            {
                ccommand = inputBuffer[0];

                for (i = 0; i < DEFAULT_COMMAND_COUNT; i++)
                {
                    if (defaultCommands[i].CommandID == ccommand)
                    {
                        char *colon = strchr(inputBuffer, ':');

                        if (ccommand == 'l')
                        {
                            char *command;
                            char *label;
                            int temp, lblLen;

                            command = malloc(1000);

                            memset(command, 0, 1000);

                            if(!colon)
                            {
                                printf("Please make sure there is a label and count\n");
                                free (command);
                                break;
                            }

                            strcpy(command, "-loop");
                            strcat(command + strlen(command), colon);

                            if (!loopMode)
                            {
                                loopMode = 1;
                            }

                            argList[argNum] = malloc(MAX_ARG_SIZE);
                            strcpy(argList[argNum], command);
                            argNum++;

                            getCmd(command, &temp);
                            label = getCmd(command + temp, &lblLen);

                            Push(&head, label, lblLen);
                                                        
                            free(command);
                            commandFound = 1;
                        }
                        else if (ccommand == 'e' && loopMode)
                        {
                            int lblLen;
                            char* lbl;
                            char* command;

                            command = malloc(1000);

                            memset(command, 0, 1000);

                            strcpy(command, "-endloop:");

                            lbl = Pop(&head, &lblLen);

                            strncat (command, lbl, lblLen);


                            argList[argNum] = malloc(MAX_ARG_SIZE);
                            strcpy(argList[argNum], command);
                            argNum++;


                            if (!head)
                            {
                                int i;

                                ProcessParameters(argNum, argList);

                                for (i=0; i < argNum; i++)
                                    free(argList[i]);

                                argNum = 0;

                                loopMode = 0;
                            }

                            free(command);
                            commandFound = 1;
                        }
                                                
                        else if (ccommand == 'q')
                        {
                            command = 0;
                            commandFound = 1;
                        }
                        else if (ccommand == 'h' && !loopMode)
                        {
                            int i;

                            printf("\nDefualt Commands:\n");
                            for (i = 0; i < DEFAULT_COMMAND_COUNT; i++)
                            {
                                printf("%c: %s", 
                                       defaultCommands[i].CommandID, 
                                       defaultCommands[i].CommandDescription);
                            }
                            printf("\n");
                            commandFound = 1;

                        }
                        else if (ccommand == 'p' && !loopMode)
                        {
                            int num, x;

                            if(colon)
                            {
                                colon++;
                                num = atoi(colon);

                                if (num < numHist && num)
                                {
                                    for (x = numHist - num; x < numHist; x++)
                                    {
                                        printf( "<%d> %s\n", x + 1 + histOverflowCount, hist[x]);
                                    }
                                    printf("\n");
                                    commandFound = 1;
                                    break;
                                }
                            }

                            for (x = 0; x < numHist; x++)
                            {
                                printf( "<%d> %s\n", x + 1 + histOverflowCount, hist[x]);
                            }

                            printf("\n");
                            commandFound = 1;
                        }
                        else if (ccommand == 'x')
                        {
                            int num;

                            if(colon)
                            {
                                colon++;
                                num = atoi(colon);

                                num--;

                                if (num >= numHist + histOverflowCount || num - histOverflowCount < 0)
                                    break;

                                strcpy(inputBuffer, hist[num - histOverflowCount]);

                                commandFound = 1;
                                xMode = 1;
                            }
                        }

                    }

                }
            }
           
            if (!commandFound)
            {
                                 
                printf("You entered: '%s'\nwhich is an invalid command.  Try again.\n", inputBuffer);
                    
            }
            else
            {
                if (!xMode)
                {
                    if (numHist == MAX_MENU_HIST)
                    {
                        int i;

                        for (i = 1; i < MAX_MENU_HIST; i++)
                        {
                            strcpy(hist[i - 1], hist[i]);
                        }

                        strcpy(hist[MAX_MENU_HIST - 1], inputBuffer);

                        histOverflowCount++;
                    }
                    else
                    {
                        hist[numHist] = malloc(MAX_ARG_SIZE);

                        strcpy(hist[numHist], inputBuffer);
                        numHist++;
                    }
                                        
                }
            }
        }
        else
        {
            printf("Error reading string from command line.  Try again.\n");
        }

        if (!xMode)
            free (inputBuffer);
    }

    for(i =0; i < numHist; i++)
        free (hist[i]);

    return (returnValue);
}

char *getCmd(char *aCommandString, int *aCommandLength)
{
    
    if (aCommandString && (0 != strcmp("", aCommandString)))
    {
        unsigned int index;

        while (':' == *aCommandString)
        {
            aCommandString++;
        }

        // Start with 1 so the [index - 1] never violates the string
        // memory boundaries
        for (index = 1; index < strlen(aCommandString); index++)
        {
            // Make sure the colon is all by itself.
            // If not, this is a sub command and needs to be left
            // in the string.  There should never be a colon at
            // the beginning of the string so the [index - 1]
            // should be safe.
            if (':' == aCommandString[index]     &&
                ':' != aCommandString[index - 1] &&
                ':' != aCommandString[index + 1])
            {
                break;
            }
        }
        if (aCommandLength)
        {
            *aCommandLength = index;
        }
#if 0
        colon = strchr(temp, ':');
        
        if (colon)
        {
            if (aCommandLength)
            {
                *aCommandLength = colon - temp;
            }
        }
        else
        {
            if (aCommandLength)
            {
                *aCommandLength = strlen(temp);
            }
        }
#endif
    }
    else
    {
        aCommandString = NULL;
        if (aCommandLength)
        {
            *aCommandLength = 0;
        }
    }

    return (aCommandString);
} // getCmd

void
ConvertSubCommands(char * Command)
{
    unsigned int oldIndex = 0;
    unsigned int newIndex = 0;

    char * buffer = malloc(strlen(Command) + 1);

    memset(buffer, 0, strlen(Command) + 1);

    // Remove 1 colon from each set of multiple colon sub strings

    if (buffer)
    {
        buffer[newIndex] = Command[oldIndex];
        newIndex++;
        for (oldIndex = 1; oldIndex < strlen(Command); oldIndex++)
        {
            if (!(':' != Command[oldIndex - 1] &&
                  ':' == Command[oldIndex]))
            {
                buffer[newIndex] = Command[oldIndex];
                newIndex++;
            }
        }

        strcpy(Command, buffer);
    }
}

int
ProcessParameters(int argc, char * argv[])
{
    int returnValue = 0;

    int index;
    int i;
    int commandFound = 0;

    for (index=0; index < argc; index++)
    {
        unsigned int   cmdLength;
        char *cmd = getCmd(argv[index], &cmdLength);
                
        if (!cmd)
        {
            break;
        }

        if (0 == memcmp("-file", cmd, cmdLength))
        {
            FILE *fileHandle;
            char * fileName = getCmd(cmd + cmdLength, NULL);
            int argNum = 0;
            char** argList;
            
            commandFound = 1;

            fileHandle = fopen(fileName, "rb");
            if (fileHandle)
            {
                char *command = malloc(MAX_ARG_SIZE);
                if (command)
                {
                    int x =0;

                    memset(command, 0, MAX_ARG_SIZE);

                    while(NULL != fgets(command, MAX_ARG_SIZE, fileHandle))
                    {
                        //If line begins with a # then skip in counting args.
                        //This is OK, because bottom loop does not depend on this number
                        //only the size of the array depends on this number and we adjust
                        if (command[0] != '#')
                        {
                            argNum++;
                        }
                    }

                    fseek(fileHandle, 0, SEEK_SET);
                                        
                    argList = (char **)malloc(argNum*sizeof(char*));
                    memset(command, 0, MAX_ARG_SIZE);

                    while(NULL != fgets(command, MAX_ARG_SIZE, fileHandle))
                    {
                        char *end;

                        end = strchr(command, '\n');

                        if (end)
                        {
                            *end = '\0';
                        }
                        end = strchr(command, '\r');
                        if (end)
                        {
                            *end = '\0';
                        }

                        //Skip adding lines with a '#' as first symbol
                        if (command[0] != '#')
                        {
                            argList[x] = command;
                            x++;
                        }
                        command = malloc(MAX_ARG_SIZE);
                    }
                    free(command);

                    returnValue = ProcessParameters(argNum, argList);
                    for (x=0; x < argNum; x++) 
                    {
                        free(argList[x]);
                    }
                    free(argList);
                }
            }
            else
            {
                printf("Invalid filename cmd:  -file %s\n", argv[index]);
                DisplayUsage();
                returnValue = -1;
            }
        }
        else if ((0 == memcmp("-menu", cmd, cmdLength)) &&
                 (strlen("-menu") == cmdLength))
        {
            commandFound = 1;
            returnValue = UseMenuToGetCommands();
        }
        else  if (0 == memcmp("-loop", cmd, cmdLength))
        {
            int lblLength, cntLength;
            char * count;
            char * label;
            int loopCount;
            int endIndex = -1;
            int j, y; 

            label = getCmd(cmd + cmdLength, &lblLength);
            count = getCmd(cmd + cmdLength + lblLength + 1, &cntLength);
                        

            loopCount = 0;
                        
            loopCount = atoi(count);
                        

            // checks validity of the loop count
            if (!loopCount)
            {
                printf ("Inappropriate count\n");
                DisplayUsage();
                return -1;
            }

            //this finds the index of where the endloop with the same label is
                        
                
            for (j = index; j < argc; j++)
            {
                cmd = getCmd(argv[j], &cmdLength);
                if (0 == memcmp("-endloop", cmd, cmdLength))
                {
                    char * eLabel = getCmd(cmd + cmdLength, NULL);
                    int eLblLen = strlen(eLabel);

                    if (0 == memcmp(eLabel, label, lblLength) && eLblLen == lblLength)
                    { 
                        endIndex = j;
                        break;
                    }
                }
            }
  
            if (endIndex == -1)
            {
                printf ("There is no matching endloop\n");
                DisplayUsage();
                return -1;
            }

            //run whats in the loop loopCount times
                        
            for (y = 0; y < loopCount; y++)
            {
                returnValue = ProcessParameters ((endIndex - 1) - index, &argv[index + 1]);
            }


            //move past the endIndex
            index = endIndex;
            commandFound = 1;
        }
		//ALL FUNCTION
		else if ((0 == memcmp("-all", cmd, cmdLength)) &&
                 (strlen("-all") == cmdLength))
        {
            commandFound = 1;
            for (i = 0; i < gCommandCount; i++)
			{
                if (0 != gCommands[i].CommandNumber)
                {
		            returnValue = gCommands[i].CommandFunc(argc, argv);

			        if(0 == returnValue)
                    {
                        printf("%-25s - PASSED\n", gCommands[i].CommandName + 1);
                    }
			        else
                    {
                        printf("%-25s - FAILED on line %d\n", gCommands[i].CommandName + 1, returnValue);
                    }				  
                }
			}
        }

        else
        {
            for (i = 0; i < gCommandCount; i++)
            {
                if ((0 == memcmp(gCommands[i].CommandName, cmd, cmdLength)) &&
                    (strlen(gCommands[i].CommandName) == cmdLength) &&
                    (gCommands[i].CommandFunc))
                {
                    char *args[MAX_ARG_SIZE];
                    int   argc = 0;
                    char *nextArg = cmd;
                    int   nextArgLength = cmdLength;
                    int   j;
                    
                    commandFound = 1;

                    while (0 != (nextArg = getCmd(nextArg + nextArgLength, &nextArgLength)))
                    {
                        args[argc] = malloc(nextArgLength + 1);
                        if (args[argc])
                        {
                            memset(args[argc], 0, nextArgLength + 1);
                            memcpy(args[argc], nextArg, nextArgLength);
                            // Remove multiple instances of colons which
                            // denote sub menu commands
                            ConvertSubCommands(args[argc]);
                        }
                        argc++;
                    }
                    returnValue = gCommands[i].CommandFunc(argc, args);
                    for (j = 0; j < argc; j++)
                    {
                        free(args[j]);
                    }
                }
            }
        }

        if (0 == commandFound)
        {
            printf("Invalid command %s\n", cmd);
        }

    }

    return (returnValue);
}


/*****************************
* Push and Pop Functoins for *
* the label stack            *
*****************************/

void Push (LABEL_NODE **headRef, char* value, int length)
{
    LABEL_NODE * newNode;

    newNode = malloc( sizeof(LABEL_NODE) );

    strcpy(newNode->label, value);
    newNode->lblLength = length;
    newNode->next = *headRef;
    *headRef = newNode;
}


char* Pop (LABEL_NODE** node, int* length)
{
    char* temp;
    LABEL_NODE *tempNode = (*node);

    temp = malloc(MAX_LBL_SIZE);

    strcpy(temp,(*node)->label);

    *length = (*node)->lblLength;

    *node = (*node)->next;

    free(tempNode);

    return (temp);
}



void
CleanupQuotes(int argc, char * argv[])
{
    int argvIndex;
    int index;
    int stringLength;
    char * string;
    char * tempString;
    int tempStringIndex;

    for (argvIndex = 0; argvIndex < argc; argvIndex++)
    {
        string = argv[argvIndex];
        stringLength = strlen(string);
        tempString = malloc(stringLength+1);
        memset(tempString, 0, stringLength+1);
        tempStringIndex = 0;
        for (index = 0; index < stringLength; index++)
        {
            if ('"' != string[index])
            {
                tempString[tempStringIndex] = string[index];
                tempStringIndex++;
            }
        }
        strcpy(string, tempString);
    }
}

int RunTest(int argc, char *argv[], int aCommandCount, COMMAND *aCommands)
{
    int returnValue = 0;

    COMMAND * oldCommands;
    int       oldCommandCount;
    
    // This is a lazy way of making this routine capable of
    // recursion.  To do this right, we shouldn't be relying on
    // globals.  As long as we don't add new globals, this should
    // be sufficient.

    oldCommands     = gCommands;
    oldCommandCount = gCommandCount;
        

    gCommandCount = aCommandCount;
    gCommands     = aCommands;

    if (argc == 0)
    {
        DisplayUsage();
        returnValue = -1;
    }
    else
    {
        CleanupQuotes(argc, argv);
        returnValue = ProcessParameters(argc, &argv[0]);
    }

    // Restore previous command structure incase of recursion.
    gCommands     = oldCommands;
    gCommandCount = oldCommandCount;

    return (returnValue);
} // RunTest



