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
 *    filename:			ipc.c
 *    author(s):			yanyongmeng
 *    version:			0.1
 *    date:				2011/2/12
 * History
 */
/*-----------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "ipc.h"
#include "hitTime.h"


/*  ipc data to be transfered through fifo */
static uint32 s_ipc_strlen(char *str)
{
  return str? (strlen(str)+1) : 0;
}

static int s_ipc_set_element(t_IPC_DATA *ipc, void* value, uint32 len)
{
  if( !ipc )
    return -1;

  int x = ipc->index;
  if( x < IPC_DATA_NUM )
  {
    ipc->xxbuf[x]		= (char*)value;
    ipc->xxbuf_len[x]	= len;
    ipc->index++;
    return 0;
  }

  return -2;
}

void Ipc_Data_SetInt(t_IPC_DATA *ipc, int* value)
{
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_MYRIAD, 0, 0);
  int ret = s_ipc_set_element(ipc, value, sizeof(int));
  HT_DBG_FUNC_END(ret, 0);
  ret = 0;
}

void Ipc_Data_SetString(t_IPC_DATA *ipc, char *str)
{
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_MYRIAD, 0, 0);
  int ret = s_ipc_set_element(ipc, str, s_ipc_strlen(str));
  HT_DBG_FUNC_END(ret, 0);
  ret = 0;
}

void Ipc_Data_SetStruct(t_IPC_DATA *ipc, void *s, int size)
{
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_MYRIAD, size, 0);
  int ret = s_ipc_set_element(ipc, s, size);
  HT_DBG_FUNC_END(ret, 0);
  ret = 0;
}

char* Ipc_Data_GetString(t_IPC_DATA *ipc, uint32 index)
{
  if( !ipc )
    return NULL;

  if( (index < 1) || (index>IPC_DATA_NUM) )
    return NULL;

  return ipc->xxbuf_len[index-1]? ipc->xxbuf[index-1] : NULL;
}
char* Ipc_Data_GetFirstString(t_IPC_DATA *ipc)
{
  return Ipc_Data_GetString(ipc, 1);
}
int Ipc_Data_GetInt(t_IPC_DATA *ipc)
{
  int *x = (int*)Ipc_Data_GetString(ipc, 1);
  return x? *x : 0;
}

uint32 Ipc_Data_GetStringLength(t_IPC_DATA *ipc, uint32 index)
{
  if( !ipc )
    return 0;

  if( (index < 1) || (index>IPC_DATA_NUM) )
    return 0;

  return ipc->xxbuf_len[index-1];
}


/*  ipc modules through fifo */
typedef enum _enum_ipc_packet_header_
{
  enum_IPH_SYNC = 0,
  enum_IPH_TOTAL_LEN,
  enum_IPH_MODULE_TYPE,
  enum_IPH_FUNC_INDEX,
  enum_IPH_DATA_START,
}enum_IPC_HEADER;

#define IPC_HEADER_SYNC_MAGIC 	0x23476591
#define IPC_HEADER_SIZE			(enum_IPH_DATA_START+IPC_DATA_NUM)
#define IPC_ONCE_SEND_SIZE		32
#define BUFFER_LEN_MAX			1024*4

#define PIPE_CALL				"fifo_call"
#define PIPE_CALL_ACK			"fifo_call_ack"
#define PIPE_CALLBACK			"fifo_callback"
#define PIPE_CB_ACK				"fifo_cb_ack"

typedef struct _ipc_dlna_s_
{
  char				dir[64];

  int					pipe_call;
  int					pipe_call_ack;
  int					pipe_callback;
  int					pipe_cb_ack;

  int					maxModuleNum;
  t_IPC_RCV_FUNC		moduleHandler[16];

  pthread_mutex_t		mutex_send;
  //	pthread_mutex_t		mutex_callback;

  int					acked;		
  int					checksum;
  int					ClientFlag;
}t_IPC_MOD;

#define FIFO_IN_DLNA_FOLDER 1

/* create & release fifo*/
static int	s_ipc_mkfifo_one(char *dir, char *fifo_name)
{
  int ret = -1;
  char path[1024];

#ifdef FIFO_IN_DLNA_FOLDER	
  sprintf(path, "/var/%s/%s", dir, fifo_name);
#else
  sprintf(path, "/var/%s", fifo_name);
#endif
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, path);
  //	if((ret = mkfifo(path, 0777)) < 0) 
  ret = mknod(path, S_IFIFO | 0666, 0);

  HT_DBG_FUNC_END( ret, 0);
  return ret;
}
static int	s_ipc_mkfifo_all(char *dir)
{
  int ret = 0;
  char path[1024];

  sprintf(path, "/var/%s", dir);

  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, path);

  umask(0);
  //    usleep(20*1000);
#ifdef FIFO_IN_DLNA_FOLDER	
  ret = mkdir(path, 777);
#endif
  if( !ret )
  {
    // ret += chmod(path, 777);
    ret += s_ipc_mkfifo_one(dir, PIPE_CALL);
    ret += s_ipc_mkfifo_one(dir, PIPE_CALL_ACK);
    ret += s_ipc_mkfifo_one(dir, PIPE_CALLBACK);
    ret += s_ipc_mkfifo_one(dir, PIPE_CB_ACK);
  }
  //    usleep(20*1000);

  HT_DBG_FUNC_END( ret, 0);
  return ret;
}

static int	s_ipc_open_fifo_one(char *dir, char *fifo_name, int mode)
{
  char path[1024];
#ifdef FIFO_IN_DLNA_FOLDER	
  sprintf(path, "/var/%s/%s", dir, fifo_name);
#else
  sprintf(path, "/var/%s", fifo_name);
#endif
  return open(path, mode);
}
static int	s_ipc_open_fifo_all(t_IPC_MOD *me, int IsClient)
{
  char *dir = me->dir;
  int x1,x2,x3,x4;

  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, IsClient, 0);
  if( IsClient )
  {	x1 = O_WRONLY;		x2 = O_RDONLY;		x3 = O_RDONLY;		x4 = O_WRONLY;}
  else
  {	x1 = O_RDONLY;		x2 = O_WRONLY;		x3 = O_WRONLY;		x4 = O_RDONLY;}

  me->pipe_call		= s_ipc_open_fifo_one(dir, PIPE_CALL,		x1);
  HT_DBG_FUNC(me->pipe_call, "me->pipe_call = ");

  me->pipe_call_ack	= s_ipc_open_fifo_one(dir, PIPE_CALL_ACK,	x2);
  HT_DBG_FUNC(me->pipe_call_ack, "me->pipe_call_ack = ");

  me->pipe_callback	= s_ipc_open_fifo_one(dir, PIPE_CALLBACK,	x3);
  HT_DBG_FUNC(me->pipe_callback, "me->pipe_callback = ");

  me->pipe_cb_ack 	= s_ipc_open_fifo_one(dir, PIPE_CB_ACK, 	x4);
  HT_DBG_FUNC(me->pipe_cb_ack, "me->pipe_cb_ack = ");

  me->checksum = me->pipe_call + me->pipe_call_ack + me->pipe_callback +  me->pipe_cb_ack;
  HT_DBG_FUNC(me->checksum, "me->checksum = ");
  me->checksum = ~(me->checksum);
  HT_DBG_FUNC_END(me->checksum, "~(me->checksum) = ");
  return 0;
}

/* send or recieve data*/
static void ipc_data_from_buffer(t_IPC_DATA *ipc, int *buffer)
{
  int i, len, offset=0;

  if( !ipc || !buffer )
    return;

  memset(ipc, 0, sizeof(t_IPC_DATA));
  memcpy(ipc->xxbuf_len, &(buffer[enum_IPH_DATA_START]), IPC_DATA_NUM*4);

  char *p = (char*)(&(buffer[IPC_HEADER_SIZE]));
  for(i=0; i<IPC_DATA_NUM; i++)
  {
    len = ipc->xxbuf_len[i];
    if( len )
    {
      ipc->xxbuf[i] = p + offset;
      offset += len;
    }
  }
}

static void ipc_check_self(t_IPC_MOD *me)
{
  int sum = me->pipe_call + me->pipe_call_ack + me->pipe_callback +  me->pipe_cb_ack;
  sum = ~sum;
  if( (sum!= me->checksum) || me->pipe_call<1 || me->pipe_call_ack<1 || me->pipe_callback<1 || me->pipe_cb_ack<1)
  {
    while(1)
    {
      HT_DBG_PRINTF(HT_MOD_IPC, HT_BIT_FEW, "!!!!!!!!!!!!!!!!!!!! ipc_check_self error\n");
    }
  }
}
static int ipc_send_action(t_IPC_MOD *me, int pipe, int module, int func_index, t_IPC_DATA *data)
{
  int 		i, len, x, padding, ret=0;
  int			crc = ~IPC_HEADER_SYNC_MAGIC;
  int			temp[IPC_HEADER_SIZE + 1 + IPC_ONCE_SEND_SIZE];

  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_MYRIAD, func_index, 0);
  ipc_check_self(me);

  len = 0;
  if( data )
  {
    for( i=0; i<IPC_DATA_NUM; i++ )
    {
      if( !(data->xxbuf[i]) )
        data->xxbuf_len[i] = 0;
      len += data->xxbuf_len[i];
    }
  }

  padding = 0;
  x = len%4;
  if( x )
  {
    padding = 4 - x;
    len += padding;
  }

  if( len > (BUFFER_LEN_MAX*4-200) )
  {
    while(1)
    {
      HT_DBG_PRINTF(HT_MOD_IPC, HT_BIT_KEY, "!!!!!!!!!!!!!!!!!!!!!!!!!!: data length = %d > %d", len, BUFFER_LEN_MAX*4-200);
      sleep(2);
    }
  }

  temp[enum_IPH_SYNC]			= IPC_HEADER_SYNC_MAGIC;
  temp[enum_IPH_TOTAL_LEN]	= IPC_HEADER_SIZE*4/*header*/  + len/*str + padding*/ + 4/*crc*/;
  temp[enum_IPH_MODULE_TYPE]	= module;
  temp[enum_IPH_FUNC_INDEX]	= func_index;

  for( i=0; i<IPC_DATA_NUM; i++ )
  {
    if( data )
      temp[enum_IPH_DATA_START+i] = data->xxbuf_len[i];
    else
      temp[enum_IPH_DATA_START+i] = 0;
  }

  if( len == 0 )
  {
    temp[IPC_HEADER_SIZE] = crc;
    ret += write(pipe, temp, IPC_HEADER_SIZE*4 + 4);
  }
  else if( len < IPC_ONCE_SEND_SIZE*4 )
  {
    x = IPC_HEADER_SIZE + len/4;
    temp[x-1] = 0;
    temp[x] = crc;

    char *dest = (char*)(&(temp[IPC_HEADER_SIZE]));
    for( i=0; i < IPC_DATA_NUM; i++ )
    {
      x = data->xxbuf_len[i]; 
      if( x ) 
      { 
        memcpy(dest, data->xxbuf[i], x); 
        dest += x; 
      }
    }

    ret += write(pipe, temp, IPC_HEADER_SIZE*4 + len + 4);
  }
  else
  {
    ret = write(pipe, temp, IPC_HEADER_SIZE*4);

    for( i=0; i < IPC_DATA_NUM; i++ )
    {
      x = data->xxbuf_len[i]; 
      if( x ) 
      { 
        ret += write(pipe, data->xxbuf[i], x); 
      }
    }

    temp[0] = 0;
    temp[1] = crc;
    ret += write(pipe, (char*)temp + (4-padding), padding + 4);
  }

  HT_DBG_FUNC_END(ret, 0);
  return ret;
}

static int ipc_recieve_asign(int *buffer, int cmd_end, int offset)
{
  memcpy(buffer, buffer+cmd_end, (offset-cmd_end)*4);
  return offset - cmd_end;
}

static int ipc_recieve_action(t_IPC_MOD *me, int pipe, int *buffer, int buf_len, int *last)
{
  fd_set			rset;
  struct timeval	tv;
  int 			i, ret,len, type, slen, crc, err, offset = *last;
  int readlen = *last*4;

  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_MYRIAD, *last, "*last = ");//HT_BIT_MYRIAD
  ipc_check_self(me);
  while(1)
  {
    // read more data
    if( readlen < buf_len*4 )	// read data
    {
read_again: 	
      tv.tv_sec  = 60;// CHECK_SUBSCRIPTIONS_TIMEOUT;
      tv.tv_usec = 0;

      HT_DBG_FUNC(readlen, "readlen = ");

      FD_ZERO(&rset);
      FD_SET( pipe, &rset);

      ret = select(pipe + 1, &rset, NULL, NULL, &tv);
      ret = read(pipe, (char*)buffer+readlen, buf_len*4 - readlen);
      if( ret < 0 )
      {
        // fatal error!!!!
        HT_DBG_PRINTF(HT_MOD_IPC, HT_BIT_KEY, "!!!!!!!!!!!!!!!!! ret < 0");
        ret = -1;
        break;
      }
      if( ret == 0 )
      {
        // forced to quit
        ret = 0;
        break;
      }

      readlen += ret;
      if( readlen%4 != 0 )
        goto read_again;

      offset = readlen/4;
    }

    err = 0;
re_align:		
    if( err )
    {
      HT_DBG_PRINTF(HT_MOD_IPC, HT_BIT_KEY, "!!!!!!!!!!!!!!!!!!! err = ", err);
    }
    // find and align header
    if( buffer[0] != IPC_HEADER_SYNC_MAGIC )
    {
      ret = 0;
      for( i = 0; i < offset; i++ )
      {
        if( buffer[i] == IPC_HEADER_SYNC_MAGIC )
        {
          offset = ipc_recieve_asign(buffer, i, offset);
          readlen = offset*4;
          ret = 1;
          break;
        }
      }

      if( ret == 0 )
      {
        buffer[0] = 0;
        offset = 0;
        readlen = 0;
        continue;
      }
    }

    // enough to be a cmplete packet
    if( offset < IPC_HEADER_SIZE  )// needed to read more
      continue;
    len = buffer[enum_IPH_TOTAL_LEN];
    if( len%4 )
    {
      buffer[0] = 0;
      err = 1;
      goto re_align;
    }
    len = len/4;
    if( len > buf_len )
    {
      buffer[0] = 0;
      err = 2;
      goto re_align;
    }
    if( len > offset )
      continue;

    // check packet
    //1. crc
    crc = buffer[len-1];
    if( crc != ~IPC_HEADER_SYNC_MAGIC )
    {
      buffer[0] = 0;
      err = 3;
      goto re_align;
    }
    //2. type 
    type = buffer[enum_IPH_MODULE_TYPE];
    if( type < 0 || type > me->maxModuleNum )
    {
      buffer[0] = 0;
      err = 6;
      goto re_align;
    }
    //3. slen 
    slen = 0;
    for(i=0; i<IPC_DATA_NUM; i++)
    {
      int x = buffer[enum_IPH_DATA_START+i];
      if( x < 0 )
      {
        buffer[0] = 0;
        err = 8;
        goto re_align;
      }
      slen += x;
    }

    if( slen > (len - IPC_HEADER_SIZE - 1)*4 )
    {
      buffer[0] = 0;
      err = 9;
      goto re_align;
    }
    ret = len;
    break;
  }

  *last = offset;
  HT_DBG_FUNC_END(ret*4, 0);
  return ret;
}

/*-------------------------------------*/
static void* ipc_client_monitor_loop(void *hnd)
{
  t_IPC_MOD*		me = (t_IPC_MOD*)hnd;
  int				buffer[BUFFER_LEN_MAX] = {0};
  int				ret, ack_len=0, cmd_len, offset = 0;
  t_IPC_DATA 		data;
  int				module,func;

  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_KEY, 0, NULL);
  while(1)
  {
    cmd_len = ipc_recieve_action(me, me->pipe_callback, buffer, BUFFER_LEN_MAX, &offset);

    module = func = 0;
    if( cmd_len > 0 )
    {
      module	= buffer[enum_IPH_MODULE_TYPE];
      func	= buffer[enum_IPH_FUNC_INDEX];
    }
    HT_DBG_PRINTF(HT_MOD_IPC, HT_BIT_MANY, "ipc_client_monitor_loop: module=%d, func=%d, cmd_len = %d\n", module, func, cmd_len);

    if( cmd_len > 0 )
    {
      ipc_data_from_buffer(&data, buffer);
      me->acked = 0;
      if( me->moduleHandler[module] )
        ret = me->moduleHandler[module]((int)hnd, func, &data);
      if( me->acked==0 )
        ack_len = ipc_send_action(me, me->pipe_cb_ack, module, func, NULL);

      HT_DBG_PRINTF(HT_MOD_IPC, HT_BIT_MANY, "ipc_client_monitor_loop: type = %d\n", func);

      offset = ipc_recieve_asign(buffer, cmd_len, offset);
    }
    else if( cmd_len == 0 )
    {
    }
    else
      break;
  }
  HT_DBG_FUNC_END((int)me, 0);
  return NULL;
}

static void* ipc_server_monitor_loop(void *hnd)
{
  t_IPC_MOD 		*me = (t_IPC_MOD *)hnd;
  int				buffer[BUFFER_LEN_MAX] = {0};
  int				ret=0, ack_len=0, cmd_len, offset = 0;
  t_IPC_DATA 		data;
  int				module,func;

  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_KEY, 0, NULL);
  s_ipc_open_fifo_all(me, 0);
  while(1)
  {
    cmd_len = ipc_recieve_action(me, me->pipe_call, buffer, BUFFER_LEN_MAX, &offset);

    module = func = 0;
    if( cmd_len > 0 )
    {
      module	= buffer[enum_IPH_MODULE_TYPE];
      func	= buffer[enum_IPH_FUNC_INDEX];
    }
    HT_DBG_PRINTF(HT_MOD_IPC, HT_BIT_MANY, "ipc_server_monitor_loop: module=%d, func=%d, cmd_len = %d\n", module, func, cmd_len);

    if( cmd_len > 0 )
    {
      ipc_data_from_buffer(&data, buffer);
      me->acked = 0;
      if( me->moduleHandler[module] )
        ret = me->moduleHandler[module]((int)hnd, func, &data);
      if( me->acked==0 )
        ack_len = ipc_send_action(me, me->pipe_call_ack, module, func, NULL);
      HT_DBG_PRINTF(HT_MOD_IPC, HT_BIT_MANY, "ipc_server_monitor_loop: func = %d, ret =%d, ack_len = %d\n", func, ret, ack_len);

      offset = ipc_recieve_asign(buffer, cmd_len, offset);
    }
    else if( cmd_len == 0 )
    {
    }
    else
      break;
  }

  HT_DBG_FUNC_END((int)me, 0);
  return NULL;
}

static void ipc_wait_ack(t_IPC_MOD *me, int pipe, int module, int func_index, t_IPC_DATA *ret)
{
  int				buffer[BUFFER_LEN_MAX] = {0};
  int				cmd_len, offset = 0;

  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_MANY, pipe, 0);
  while(1)
  {
    cmd_len = ipc_recieve_action(me, pipe, buffer, BUFFER_LEN_MAX, &offset);

    HT_DBG_PRINTF(HT_MOD_IPC, HT_BIT_MANY, "ipc_wait_ack: cmd_len = %d, offset = %d\n", cmd_len, offset);
    if( cmd_len != offset )
    {
      HT_DBG_FUNC( 0, "!!!!!!!!!!!!!!!!!!!!!!!!!!  cmd_len != offset");
    }

    if( cmd_len > 0 )
    {
      if( ret )
        ipc_data_from_buffer(ret, buffer);

      offset = ipc_recieve_asign(buffer, cmd_len, offset);
      break;
    }
    else if( cmd_len == 0 )
    {
    }
    else
      break;
  }
}
int	Ipc_MakeFifo(char *name)
{
  int ret=0;
#if 0
  char temp[1024];
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, name);
  sprintf(temp, "/var/%s/%s", name, PIPE_CALL);
  ret += remove(temp);
  HT_DBG_FUNC(ret,temp);

  sprintf(temp, "/var/%s/%s", name, PIPE_CALL_ACK);
  ret += remove(temp);
  HT_DBG_FUNC(ret,temp);

  sprintf(temp, "/var/%s/%s", name, PIPE_CALLBACK);
  ret += remove(temp);
  HT_DBG_FUNC(ret,temp);

  sprintf(temp, "/var/%s/%s", name, PIPE_CB_ACK);
  ret += remove(temp);
  HT_DBG_FUNC(ret,temp);

  sprintf(temp, "/var/%s", name);
  ret += remove(temp);
  HT_DBG_FUNC(ret,temp);
#endif    
  ret += s_ipc_mkfifo_all(name);

  return ret;
}

/* public functions */
int	Ipc_Server_Create(char *name, int maxModuleNum)
{
  int len = sizeof(t_IPC_MOD);
  t_IPC_MOD *me = malloc(len);

  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_KEY, maxModuleNum, NULL);
  memset(me, 0, len);

  me->maxModuleNum = maxModuleNum;
  pthread_mutex_init(&(me->mutex_send), NULL);
  strcpy(me->dir, name);

  //	s_ipc_mkfifo_all(name);

#if 0
  Ipc_Server_MonitorLoop(me);
#else
  //	typedef void *(* VFUNCV)(void *arg);
  pthread_t	p_tid;
  pthread_create( &p_tid, NULL, ipc_server_monitor_loop, me);
#endif

  HT_DBG_FUNC_END((int)me, 0);
  return (int)me;
}

int	Ipc_Client_Create(char *name, int maxModuleNum)
{
  int len = sizeof(t_IPC_MOD);
  t_IPC_MOD *me = malloc(len);

  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_KEY, maxModuleNum, NULL);
  memset(me, 0, len);

  me->maxModuleNum = maxModuleNum;
  me->ClientFlag = 1;
  pthread_mutex_init(&(me->mutex_send), NULL);
  strcpy(me->dir, name);

  //	s_ipc_mkfifo_all(name);
  s_ipc_open_fifo_all(me, 1);

  //	typedef void *(* VFUNCV)(void *arg);
  pthread_t	p_tid;
  pthread_create( &p_tid, NULL, ipc_client_monitor_loop, me);

  HT_DBG_FUNC_END((int)me, NULL);
  return (int)me;
}

void Ipc_Register_Module(int hnd, int module, t_IPC_RCV_FUNC func)
{
  t_IPC_MOD *me = (t_IPC_MOD*)hnd;

  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, module, "module = ");
  if( me && (module < me->maxModuleNum) )
    me->moduleHandler[module] = func;
  HT_DBG_FUNC_END((int)func, "func = ");
}

void Ipc_CallAndReturnEx(int hnd, int module, int func_index, t_IPC_DATA *para, void *buf, void **pp)
{
  t_IPC_MOD *me = (t_IPC_MOD*)hnd;
  int send, ack;

  pthread_mutex_lock(&(me->mutex_send));

  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, func_index, 0);

  if( me->ClientFlag )
  {
    send = me->pipe_call;
    ack  = me->pipe_call_ack;
  }
  else
  {
    send = me->pipe_callback;
    ack  = me->pipe_cb_ack;
  }

  int cmd_len = ipc_send_action(me, send, module, func_index, para);
  HT_DBG_FUNC(cmd_len, "cmd_len = ");
  cmd_len=0;
  ipc_wait_ack(me, ack, module, func_index, para);

    char *p = Ipc_Data_GetFirstString(para);
  if(p)
  {
    HT_DBG_FUNC(para->xxbuf_len[0], 0);
    if(buf)
      memcpy(buf, p, para->xxbuf_len[0]);
    else if(pp)
    {
      *pp = malloc(para->xxbuf_len[0]);
      memcpy(*pp, p, para->xxbuf_len[0]);
    }
    else
    {
    }
  }

  HT_DBG_FUNC_END(0, 0);

  pthread_mutex_unlock (&(me->mutex_send));
}
void Ipc_CallAndReturn(int hnd, int module, int func_index, t_IPC_DATA *para)
{
  Ipc_CallAndReturnEx(hnd, module, func_index, para, NULL, NULL);
}

void Ipc_Ack_Value(int hnd, t_IPC_DATA *ret)
{
  t_IPC_MOD *me = (t_IPC_MOD*)hnd;

  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, me->acked, "me->acked = ");
  if( me->acked == 1 )
    return;

  int pipe = (me->ClientFlag)? me->pipe_cb_ack: me->pipe_call_ack;
  int cmd_len = ipc_send_action(me, pipe, 0, -1, ret);
  me->acked = 1;
  HT_DBG_FUNC_END(cmd_len, "cmd_len = ");
  cmd_len=0;
}

void Ipc_Release(int hnd)
{
  t_IPC_MOD *me = (t_IPC_MOD*)hnd;

  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_KEY, hnd, 0);
  if( me )
  {
    close(me->pipe_call);
    close(me->pipe_call_ack);
    close(me->pipe_callback);
    close(me->pipe_cb_ack);
    pthread_mutex_destroy (&(me->mutex_send));

    free(me);
  }
  HT_DBG_FUNC_END(hnd, 0);
}


