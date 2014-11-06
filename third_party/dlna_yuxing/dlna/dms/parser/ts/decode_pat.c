/*****************************************************************************
 * decode_pat.c: PAT decoder example
 * Created by whz Yuxing CO,.LTD ,2002.10.1
 * Modified by whz for compaliant with VxWorks 2002.10.10
 *****************************************************************************/


#include "DMA_tsconfig.h"

#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "DMA_dvbpsi.h"
#include "DMA_psi.h"
#include "DMA_pat.h"
#include "parser_ts.h"

/*****************************************************************************
 * ReadPacket
 *****************************************************************************/
static unsigned long DMA_ParseLen=0;
static  ts_psi_info   DMA_l_PSIInfo;
static  int DMA_bPSIFound=0;

/*********************º¯ÊýÁÐ±í********************************/
unsigned long  DMA_GetParseLen(void);
void DMA_DumpPAT(void* p_zero, dvbpsi_pat_t* p_pat);
int DMA_TSCheckPAT(unsigned char* tsbuf,unsigned int tssize);
void  DMA_TsResetPSI(void);

unsigned long  DMA_GetParseLen(void)
{ 
	return DMA_ParseLen;
}
/*****************************************************************************
 * DumpPAT
 *****************************************************************************/
void DMA_DumpPAT(void* p_zero, dvbpsi_pat_t* p_pat)
{
  int i;
  dvbpsi_pat_program_t* p_program = p_pat->p_first_program;
  
#if 0
  printf(  "\n");
  printf(  "New PAT\n");
  printf(  "  transport_stream_id : %d\n", p_pat->i_ts_id);
  printf(  "  version_number      : %d\n", p_pat->i_version);
  printf(  "    | program_number @ [NIT|PMT]_PID\n");
#endif  
  
  memset(&DMA_l_PSIInfo,0,sizeof(ts_psi_info));
  DMA_l_PSIInfo.i_ts_id=p_pat->i_ts_id;
  DMA_l_PSIInfo.i_version=p_pat->i_version;
  i=0;
  DMA_bPSIFound=0;
  while(p_program)
  {
  DMA_l_PSIInfo.i_number[i]=p_program->i_number;
  DMA_l_PSIInfo.i_pid[i]=p_program->i_pid;
  
  if(p_program->i_number)DMA_bPSIFound=1;
  p_program = p_program->p_next;
  i++;
  if(i>=32)
	{
		printf("Too many programs in current Stream\n");
		break;
	}
  
  }
  
/*  printf(  "  active              : %d\n", p_pat->b_current_next); */
  
  DMA_dvbpsi_DeletePAT(p_pat);
}

int DMA_GetTSPSIInfo(ts_psi_info *psi_info)
{
    if(!DMA_bPSIFound) return 0;
 
    if(psi_info)
    {
        memcpy(psi_info,&DMA_l_PSIInfo,sizeof(ts_psi_info));
        return 1;
    }
    else 
        return 0;
}
/*****************************************************************************
 * TSCheckPAT: Check PAT in current stream ,return with 1 if new PAT found,user 
 *  can Check PSI info by calling GetTSPSIInfo();
 *  input:
 *      tsbuf:  this parametre is the ts data pointer
 *      tssize: this parametre is the ts data size of tsbuf
 *****************************************************************************/
int DMA_TSCheckPAT(unsigned char* tsbuf,unsigned int tssize)
{
  dvbpsi_handle h_dvbpsi;
  unsigned int len=0;
 
  unsigned short i_pid;
  unsigned char* ptr=tsbuf;
  
  DMA_bPSIFound=0;
  
  memset(&DMA_l_PSIInfo,0,sizeof(ts_psi_info)); 

  h_dvbpsi = DMA_dvbpsi_AttachPAT(DMA_DumpPAT, NULL);
  
  while(len<tssize)
  {

	  while((*ptr!=0x47)&&(len<tssize))
		{
	     ptr++;
		 len++;
		}
  
    i_pid = ((unsigned short)(ptr[1] & 0x1f) << 8) + ptr[2];
    if(i_pid == 0x0)
	{
		DMA_dvbpsi_PushPacket(h_dvbpsi, ptr);
	  
	  if(DMA_bPSIFound)	break;
	}
	 
	ptr+=188;
    len+=188;

  }

  DMA_dvbpsi_DetachPAT(h_dvbpsi);
  
  return DMA_bPSIFound;
  }

void  DMA_TsResetPSI(void)
{      
	DMA_bPSIFound=0;
}
