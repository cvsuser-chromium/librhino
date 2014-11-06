/*****************************************************************************
 * decode_pat.c: PMT decoder example
 * Created by whz Yuxing CO,.LTD ,2002.10.1
 * Modified by whz for compaliant with VxWorks 2002.10.10
 *****************************************************************************/


#include "DMA_tsconfig.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
//#include <fcntl.h>
/* The libdvbpsi distribution defines DVBPSI_DIST */
#include "DMA_dvbpsi.h"
#include "DMA_psi.h"
#include "DMA_descriptor.h"
#include "DMA_pmt.h"       
#include "parser_ts.h"

static  ts_pmt_info DMA_I_PMTInfo;
static unsigned char *DMA_pdec_specific_info = NULL;

void DMA_DumpDescriptors(const char* str, dvbpsi_descriptor_t* p_descriptor);
void DMA_DumpPMT(void* p_zero, dvbpsi_pmt_t* p_pmt);
int DMA_GetTSPMTInfo(ts_pmt_info *pmt_info);
int DMA_TSCheckPMT(unsigned short i_program_number,unsigned short i_pmt_pid,
			   unsigned char *tsbuf, unsigned int tssize);

void DMA_MP4_IODParse(unsigned char *buf,int buf_len, ts_pmt_info *pmt_info);

//#define DEBUG_MP4

#ifdef DMA_DEBUG_MP4
#define DMA_MP4_dbg	printf
#else
#define DMA_MP4_dbg(f, a...) do {} while (0)
#endif


/*****************************************************************************
 * DumpDescriptors
 *****************************************************************************/
void DMA_DumpDescriptors(const char* str, dvbpsi_descriptor_t* p_descriptor)
{
#if 0
    while(p_descriptor)
    {
        int i;
        printf("%s 0x%02x : \"", str, p_descriptor->i_tag);
    
        for(i = 0; i < p_descriptor->i_length; i++)
            printf("%02x ", p_descriptor->p_data[i]);
        printf("\"\n");

        p_descriptor = p_descriptor->p_next;
    }
#endif

    while(p_descriptor)
    {
        if ((p_descriptor->i_tag == 0x1d) && (p_descriptor->i_length > 0))
        {
            DMA_MP4_IODParse(p_descriptor->p_data,p_descriptor->i_length,&DMA_I_PMTInfo);
        }
        p_descriptor = p_descriptor->p_next;
    }
};


/*****************************************************************************
 * DumpPMT
 *****************************************************************************/
void DMA_DumpPMT(void* p_zero, dvbpsi_pmt_t* p_pmt)
{
    dvbpsi_pmt_es_t* p_es = p_pmt->p_first_es;

    memset((void *)(&DMA_I_PMTInfo),0,sizeof(ts_pmt_info));
   
    DMA_I_PMTInfo.PCRID=p_pmt->i_pcr_pid;  
		printf("pmt pcr pid %x\n",DMA_I_PMTInfo.PCRID);
    while(p_es)
    {
        //printf("p_es type %x\n",p_es->i_type);
	    switch(p_es->i_type)
	    {
	        case ISO_IEC_11172_VIDEO: //MPEG1
            case ISO_IEC_13818_2_VIDEO: 
            case ISO_IEC_MPEG4_VIDEO:
            case ISO_IEC_H264:
		        		DMA_I_PMTInfo.videoType =p_es->i_type;
                DMA_I_PMTInfo.VID = p_es->i_pid;
                printf("DMA_I_PMTInfo.VID %x,,,%x\n",DMA_I_PMTInfo.VID,p_es->i_type);
		        		break;
            case ISO_IEC_11172_AUDIO:
            case ISO_IEC_13818_3_AUDIO: 
            case ISO_IEC_MPEG4_AUDIO:
			case ISO_IEC_AC3_AUDIO:
			case 0xBD:
			case ISO_IEC_PES_DATA:
			case ISO_IEC_13818_7_AUDIO:
                DMA_I_PMTInfo.audioType[DMA_I_PMTInfo.num_of_AID] =p_es->i_type;
		        DMA_I_PMTInfo.AID[DMA_I_PMTInfo.num_of_AID++] =p_es->i_pid;
		         printf("DMA_I_PMTInfo.AID[%d] %x\n",DMA_I_PMTInfo.num_of_AID,p_es->i_pid);
		        break;
			//case ISO_IEC_PES_DATA:
		        //DMA_I_PMTInfo.SUBTITLEID[DMA_I_PMTInfo.num_of_SUBTITLE++]=p_es->i_pid;
			//	break; 		  
	        default:
		        if(p_es->i_type>0x7f)
			    {
			        DMA_I_PMTInfo.audioType[DMA_I_PMTInfo.num_of_AID]=p_es->i_type;
			        DMA_I_PMTInfo.AID[DMA_I_PMTInfo.num_of_AID++] =p_es->i_pid;
			        //printf("2---DMA_I_PMTInfo.AID[%d] %x\n",DMA_I_PMTInfo.num_of_AID,p_es->i_pid);
			    }
	        break;
	    }

	//DMA_I_PMTInfo.num_of_AID++;
    //DMA_I_PMTInfo.
   // DumpDescriptors("    |  ]", p_es->p_first_descriptor);

    p_es = p_es->p_next;
  }
  DMA_dvbpsi_DeletePMT(p_pmt);
}


int DMA_GetTSPMTInfo(ts_pmt_info *pmt_info)
{
int	i; 	
 	    
    if(pmt_info)
    {
    		printf("xxx pmt_info is ok xxx\n");
        memcpy(pmt_info,&DMA_I_PMTInfo,sizeof(ts_pmt_info));
	 	for(i=0;i<pmt_info->num_of_AID;i++)
		{
			if(pmt_info->AID[i]==0)
				{
				pmt_info->num_of_AID=i;
				break;
				}
				
		}
        return 1;
    }
    else 
        return 0;
}
/*****************************************************************************
 * TSCheckPMT: Check PMT in current stream ,return with 1 if new PMT found,user 
 *  can Check PSI info by calling GetTSPMTInfo();
 *  input:
 *  i_program_number: program number in the stream
 *	i_pmt_pid:      : PMT ID 
 *  tsbuf:  this parametre is the ts data pointer
 *  tssize: this parametre is the ts data size of tsbuf
 *  output:  1 ,if PMT is found else 0
 *****************************************************************************/
int DMA_TSCheckPMT(unsigned short i_program_number,
			   unsigned short i_pmt_pid,
			   unsigned char *tsbuf, unsigned int tssize)
{
    unsigned char *ptr=tsbuf;
    dvbpsi_handle h_dvbpsi;
    unsigned int len=0;
    unsigned short i_pid;
    memset(&DMA_I_PMTInfo,0,sizeof(ts_pmt_info));

    //printf("ts i_program_number %x pmt pid %x\n",i_program_number,i_pmt_pid);
    h_dvbpsi = DMA_dvbpsi_AttachPMT(i_program_number, DMA_DumpPMT, NULL);

    while(len<tssize)
    {
	    while((*ptr!=0x47)&&(len<tssize))
		{
	        ptr++;
		    len++;
		}

	    i_pid = ((unsigned short)(ptr[1] & 0x1f) << 8) + ptr[2];
        if(i_pid == i_pmt_pid)
	    {
		    DMA_dvbpsi_PushPacket(h_dvbpsi, ptr);		
	    }
	 
	    ptr+=188;
        len+=188;
    
        if((DMA_I_PMTInfo.AID[0])||(DMA_I_PMTInfo.VID))  
    	    break;
    }

    DMA_dvbpsi_DetachPMT(h_dvbpsi);

    if((DMA_I_PMTInfo.AID[0]==0)&&(DMA_I_PMTInfo.VID==0))  
        return 0;
    else 
	    return 1;
}

static inline void DMA_IODGetBytes(unsigned char **pi_data, int cnt, unsigned char *buf)
{
    int i;
	unsigned char *p = *pi_data;
    for (i = 0;i<cnt;i++)
    {
	    buf[i] = *p;
	    p++;
	}
	(*pi_data) = p;
}

static int DMA_IODDescriptorLength(unsigned char **pi_data)
{
	unsigned char i_b;
	unsigned int i_len=0;
	unsigned char *p = *pi_data;

	do {
		i_b = *p;
		i_len = (i_len << 7) + (i_b&0x7f);
		p++;
	} while (i_b&0x80);
    *pi_data = p;
	return (i_len);
}

void DMA_MP4_IODParse(unsigned char *buf,int buf_len, ts_pmt_info *pmt_info)
{
	unsigned char bBuffer[8];
	int i_iod_len;
	unsigned int i_flags;
	unsigned char b_url;
    unsigned char *des_ptr = buf;


	DMA_IODGetBytes(&des_ptr,2,bBuffer);
	if (bBuffer[1] != 0x02) {
		DMA_MP4_dbg("ERR: tag != 0x02\n");
	}
	i_iod_len = DMA_IODDescriptorLength(&des_ptr);
	DMA_MP4_dbg(" * length:%d\n", i_iod_len);

	DMA_IODGetBytes(&des_ptr,2,bBuffer);
	i_flags = bBuffer[1];
	b_url = (i_flags >> 5)&0x01;
	DMA_MP4_dbg(" * url flag:%d\n", b_url);

	if (b_url) {
	}

	DMA_IODGetBytes(&des_ptr,5,bBuffer);

	while ((des_ptr - buf) < buf_len) {
		unsigned char i_tag=0, i_len;

		DMA_IODGetBytes(&des_ptr,1,&i_tag);
		i_len = DMA_IODDescriptorLength(&des_ptr);

		switch (i_tag) {
			case 0x03:
			{
				int i_decoderConfigDescr_length;

				DMA_IODGetBytes(&des_ptr,2,bBuffer);
				DMA_IODGetBytes(&des_ptr,1,&i_flags);
				b_url = (i_flags >> 6)&0x01;
				if ((i_flags>>7)&0x01) {
					DMA_IODGetBytes(&des_ptr,2,bBuffer);
				}

				//if ((i_flags>>6)&0x01) {
				if (b_url)
					DMA_MP4_dbg(" * url flag:%d\n", b_url);

				if ((i_flags>>5)&0x01) {
					DMA_IODGetBytes(&des_ptr,2,bBuffer);
				}

				DMA_IODGetBytes(&des_ptr,1,bBuffer);
				if (bBuffer[0] != 0x04) {
					DMA_MP4_dbg(" * ERR missing DecoderConfigDescr\n");
				}
				i_decoderConfigDescr_length = DMA_IODDescriptorLength(&des_ptr);
				DMA_MP4_dbg(" - DecoderConfigDesc length:%d\n", i_decoderConfigDescr_length);

				DMA_IODGetBytes(&des_ptr,1,bBuffer);		//objectTypeIndication
				DMA_IODGetBytes(&des_ptr,1,&i_flags);	    //streamType
				DMA_IODGetBytes(&des_ptr,3,bBuffer);		//bufferSizeDB
				DMA_IODGetBytes(&des_ptr,4,bBuffer);		//maxBitrate
				DMA_IODGetBytes(&des_ptr,4,bBuffer);		//avgBitrate

				DMA_IODGetBytes(&des_ptr,1,bBuffer);
				if (i_decoderConfigDescr_length>13 && bBuffer[0]==0x05) {
					int i;
					pmt_info->dec_specific_info_len = DMA_IODDescriptorLength(&des_ptr);
					if (pmt_info->dec_specific_info_len > 0) {
					    if (DMA_pdec_specific_info)
					        free(DMA_pdec_specific_info);
					    DMA_pdec_specific_info = malloc(pmt_info->dec_specific_info_len);
						pmt_info->p_dec_specific_info = DMA_pdec_specific_info;
					}
					for (i=0; i<pmt_info->dec_specific_info_len; i++) {
						DMA_IODGetBytes(&des_ptr,1,&pmt_info->p_dec_specific_info[i]);
						DMA_MP4_dbg("%02x ",pmt_info->p_dec_specific_info[i]);
					}
					DMA_MP4_dbg("\n");
				}
				else {
					pmt_info->dec_specific_info_len = 0;
					pmt_info->dec_specific_info_len = NULL;
				}

				{
					int i_SLConfigDescr_length;

					DMA_IODGetBytes(&des_ptr,1,bBuffer);
					if (bBuffer[0] != 0x06) {
						DMA_MP4_dbg(" * ERR missing SLConfigDescr\n");
					}
					i_SLConfigDescr_length = DMA_IODDescriptorLength(&des_ptr);
					{
						int i=0;
						for (i=0; i<i_SLConfigDescr_length; i++)
							DMA_IODGetBytes(&des_ptr,1,bBuffer);
					}
				}
				break;
			}
			default:
				DMA_MP4_dbg(" * - OD tag:0x%x length:%d (Unsupported)\n",i_tag, i_len);
				break;
		}//switch(i_tag)
	}//while
}
