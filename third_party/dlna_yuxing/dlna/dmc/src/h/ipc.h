/*-----------------------------------------------------------------------------------------------*/
/*
* Yuxing Software CONFIDENTIAL
* Copyright (c) 2003, 2011 Yuxing Corporation.  All rights reserved.
* 
* The computer program contained herein contains proprietary information
* which is the property of Yuxing Software Co., Ltd.  The program may be used
* and/or copied only with the written permission of Yuxing Software Co., Ltd.
* or in accordance with the terms and conditions stipulated in the
* agreement/contract under which the programs have been supplied.
*
*    filename:			ipc.h
*    author(s):			yanyongmeng
*    version:			0.1
*    date:				2011/2/12
* History
*/
/*-----------------------------------------------------------------------------------------------*/
#ifndef __IPC_H__
#define __IPC_H__

/*  ipc data to be transfered through fifo */
#define IPC_DATA_NUM	4

typedef unsigned int uint32;
typedef struct _ipc_data_s_
{
	int 	index;

	uint32	xxbuf_len[IPC_DATA_NUM];
	char*	xxbuf[IPC_DATA_NUM];
}t_IPC_DATA;

void Ipc_Data_SetInt(t_IPC_DATA *ipc, int* value);// return 0 means success, otherwise fail
void Ipc_Data_SetString(t_IPC_DATA *ipc, char *str);// return 0 means success, otherwise fail
void Ipc_Data_SetStruct(t_IPC_DATA *ipc, void *s, int size);// return 0 means success, otherwise fail
char* Ipc_Data_GetString(t_IPC_DATA *ipc, uint32 index); //index should > 0
char* Ipc_Data_GetFirstString(t_IPC_DATA *ipc); // same as Ipc_Data_GetString, just index =1
int Ipc_Data_GetInt(t_IPC_DATA *ipc); // only first buf can be treated as pointer of int


/*  ipc modules through fifo */
typedef int (*t_IPC_RCV_FUNC)(int hnd, int func_index, t_IPC_DATA *data);
int	Ipc_MakeFifo(char *name);
int	Ipc_Server_Create(char *name, int maxModuleNum);
int	Ipc_Client_Create(char *name, int maxModuleNum);
void Ipc_Register_Module(int hnd, int module, t_IPC_RCV_FUNC func);
void Ipc_CallAndReturnEx(int hnd, int module, int func_index, t_IPC_DATA *para, void *buf, void **pp);
void Ipc_CallAndReturn(int hnd, int module, int func_index, t_IPC_DATA *send);
void Ipc_Ack_Value(int hnd, t_IPC_DATA *ret);
void Ipc_Release(int hnd);


#endif /*__IPC_H__ */


