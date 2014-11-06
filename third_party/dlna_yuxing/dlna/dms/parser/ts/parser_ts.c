#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AvMediaInfo.h"
#include "parser_ts.h"

extern int DMA_TSCheckPAT(unsigned char* tsbuf,unsigned int tssize);
extern int DMA_TSCheckPMT(unsigned short i_program_number,
			   unsigned short i_pmt_pid,
			   unsigned char *tsbuf, unsigned int tssize);
			   
static ts_psi_info DMA_psi_info;
static int DMA_bTSParsed = 0;
static int DMA_bTsParseDbg = 0;
static int DMA_bPATFound;


void DMA_TSParseInit(void)
{
	DMA_bPATFound = 0; 
	memset((void *)(&DMA_psi_info), 0, sizeof(ts_psi_info));
}

int DMA_TSParsePSI(unsigned char* buf, unsigned int ts_size)
{
	int rt;   
	int	i;
	ts_pmt_info pmt_info;
   
	if(!buf)return 0;
  
	DMA_bTSParsed=0;

	memset((void *)&pmt_info,0,sizeof(ts_pmt_info));
  
	while(1){
		if(!DMA_bPATFound){
			if(DMA_bTsParseDbg)
        			printf("TSCheckPAT!\n");
   	       	rt=DMA_TSCheckPAT(buf,ts_size);
   	        	if(!rt){
		      		if(DMA_bTsParseDbg)
     			    	printf("Cannot parse PAT!\n");  
	 	        	break; 
   	        	}
   	
			DMA_GetTSPSIInfo(&DMA_psi_info);
			for(i=0;i<32;i++){
				if(DMA_psi_info.i_number[i]!=0){
	     				DMA_bPATFound=1;
	     				break;
				}
			}
			if(i>=32) return DMA_bTSParsed;
		}	
		for(i=0;i<32;i++){
            		if (DMA_psi_info.i_number[i]==0)
                		continue;
			if(DMA_bTsParseDbg)
				printf("TSCheckPMT!\n");
            		rt=DMA_TSCheckPMT(DMA_psi_info.i_number[i],DMA_psi_info.i_pid[i],buf, ts_size);
		    	if  (!rt)
		       	continue;
		    	printf("xxx get ts pmt info xxx\n");
		    	DMA_GetTSPMTInfo(&pmt_info);
		    	if((pmt_info.VID==0)&&(pmt_info.AID[0]==0))
		       	continue;
            		else 
                		break;
		}
		if(i<32){
			if(DMA_bTsParseDbg){
				printf("PMT video ID     is: 0x%02x\n",pmt_info.VID );
   				printf("PMT audio nubmer is: 0x%02x\n",pmt_info.num_of_AID);
   		        	for(i=0;i<pmt_info.num_of_AID;i++)
   		            	printf("PMT Audio_%02d     is: 0x%02x\n",i,pmt_info.AID[i]);
#ifdef SUBTITLE_SUPPORT	   
                		printf("subtitle nubmer is: 0x%02x\n",pmt_info.num_of_SUBTITLE);
		        	for(i=0;i<pmt_info.num_of_SUBTITLE;i++)
		            		printf("Subtitle_%02d     is: 0x%02x\n",i,pmt_info.SUBTITLEID[i]);
#endif
			}
			DMA_bTSParsed=1;
		}
		break;
	}   //end while
	return DMA_bTSParsed;
}

void DMA_TsParseSetDbg(int s)
{
	DMA_bTsParseDbg=s;
}

int DMA_TSParseGetInfo(ts_pmt_info *p)
{
	return DMA_GetTSPMTInfo(p);
} 

/*全局变量，用来存储TS的进一步分析信息*/
ts_pmt_info *g_ts_pmt_info_p = NULL;
int parser_ts(char *buffer, int len,MEDIA_INFO* the_media_info)
{
	char *buff = buffer;

	if(g_ts_pmt_info_p != NULL){
		printf("xxx g_ts_pmt_info_p is not NULL xxx\n");
		free(g_ts_pmt_info_p);
		g_ts_pmt_info_p = NULL;
	}
	g_ts_pmt_info_p = (ts_pmt_info *)malloc(sizeof(ts_pmt_info));
	memset(g_ts_pmt_info_p, 0, sizeof(ts_pmt_info));
	if( !g_ts_pmt_info_p ){
		return -1;
	}
	/**设置打印信息**/	
	DMA_TsParseSetDbg(1);
	
	DMA_TSParseInit();
	
	if(DMA_TSParsePSI(buff,len) == 1){
		DMA_TSParseGetInfo(g_ts_pmt_info_p);
	}else{
		/*现在分析H264还存在准确度问题，
		*所以默认解析不出来的TS为H264******/
		g_ts_pmt_info_p->videoType = 0x1B;
		g_ts_pmt_info_p->audioType[0] = 0x04;
	}
	
	the_media_info->info.addInfo = g_ts_pmt_info_p;
	
	buff = NULL;
	return 0;
}


int format_is_ts(unsigned char *buffer, unsigned long bytes, MEDIA_INFO* the_media_info)
{
	unsigned char* p_buffer	= buffer;
	unsigned long i	= 0;

	while (i < 10240) {
		while ( (*p_buffer != 0x47) && (i < bytes) ) {
			p_buffer++;
			i++;
		}

		if ( (*(p_buffer + 188) == 0x47) && (*(p_buffer + 188*2) == 0x47) && (*(p_buffer + 188*3) == 0x47) &&
			(*(p_buffer + 188*4) == 0x47) && (*(p_buffer + 188*5) == 0x47) && (*(p_buffer + 188*6) == 0x47) &&
			(*(p_buffer + 188*7) == 0x47) && (*(p_buffer + 188*8) == 0x47) && (*(p_buffer + 188*9) == 0x47) &&
			(*(p_buffer + 188*10) == 0x47) && (*(p_buffer + 188*11) == 0x47) && (*(p_buffer + 188*12) == 0x47) 
			) {
			printf("===========detect format is TS\n");
			return 1;
		} else {
			p_buffer++;
			i++;
		}
	}	
	return 0;
}
