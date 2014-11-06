#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include "DtcpApi.h"
#include "DtcpAppLib.h"
#include "DtcpPacketizer.h"
#include "StatusCodes.h"
//#include "render_dbg.h"
#include "dtcp_client.h"

#define DTCP_BUF_SIZE 64 * 1024
struct cache_buf{
	char buf[DTCP_BUF_SIZE];
	int data_len;
};
static struct dtcp_stream_t dtcp_stream;
static struct cache_buf* dtcp_buf = NULL;
static void *ake_handle = NULL;
static char dtcp_inited = 0;

int GlobalDisplayLevel = 0;//MSG_DEBUG;
int GlobalLogToFile = 0;
int GlobalDisplayLevelMatchExactly = 0;


int dtcp_client_init(void)
{
	int ret;

	if(dtcp_inited)
		return 0;
	ret = DtcpAppLib_Startup();
	if(ret > 0){
		//render_msg("error to start dtcp client\n");
		return -1;
	}
	dtcp_inited = 1;
	return 0;
}

int dtcp_client_close(void)
{
	if(dtcp_inited)
		DtcpAppLib_Shutdown();
	dtcp_inited = 0;
	return 0;
}

void * dtcp_client_do_ake(char *remote_ip, unsigned  short remote_dtcp_port)
{
	int ret;
	void *handle;
	
	ret = DtcpAppLib_DoAke(remote_ip, remote_dtcp_port, &handle);
	if(ret > 0){
		//render_msg("error to do ake with:add:%s port:%d\n", remote_ip, remote_dtcp_port);
		return NULL;
	}
	return handle;
}

int dtcp_client_sink_create(char *remote_ip, unsigned  short remote_dtcp_port)
{
	int ret;

	ake_handle = dtcp_client_do_ake(remote_ip, remote_dtcp_port);
	if(ake_handle){
		ret = DtcpPacketizer_CreateContentSink(&dtcp_stream, ake_handle);
		if(ret > 0){
			//render_msg("error to call DtcpPacketizer_CreateContentSink\n");
			DtcpAppLib_CloseAke(ake_handle);
			return -1;
		}
		if(dtcp_buf){
			free(dtcp_buf);
		}
		dtcp_buf = (struct cache_buf *)malloc(sizeof(struct cache_buf));
		dtcp_buf->data_len = 0;
		return 0;
	}
	return -1;
}

int dtcp_client_sink_destroy(void)
{
	DtcpPacketizer_DestroyContentStream(&dtcp_stream);
	if(ake_handle)
		DtcpAppLib_CloseAke(ake_handle);
	ake_handle = NULL;
	if(dtcp_buf)
		free(dtcp_buf);
	dtcp_buf = NULL;
	return 0;
}

void dtcp_client_buf_get(char **addr, int *buf_len)
{
	assert(dtcp_buf);
	*addr = &dtcp_buf->buf[dtcp_buf->data_len];
	*buf_len = DTCP_BUF_SIZE - dtcp_buf->data_len;
	return ;
}

void dtcp_client_buf_reset(void)
{
	assert(dtcp_buf);
	dtcp_buf->data_len = 0;
	return;
}
#if 0
int dtcp_client_clear_data(char *encrypted_data, unsigned int encrypted_data_size, char *clear_content, unsigned int *clear_content_size)
{
//	int ret;
	int processed = 0;
	char *ptr_in = dtcp_buf->buf;
	int len_in = encrypted_data_size + dtcp_buf->data_len;
	char *ptr_out = clear_content;
	int len_out = *clear_content_size;
	
	while(ptr_in < (dtcp_buf->buf +  encrypted_data_size + dtcp_buf->data_len)) {
		DtcpPacketizer_DepacketizeData(&dtcp_stream, ptr_in, len_in, ptr_out,&len_out, &processed);
		//printf("total is %d, len in is %d processed is %d\n", encrypted_data_size, len_in, processed);
		if(processed > 0) {
			ptr_in += processed;
			len_in -= processed;
			ptr_out += len_out;
			len_out = *clear_content_size - (ptr_out - clear_content);		
		} else break;
	}
	*clear_content_size =  ptr_out - clear_content;
	dtcp_buf->data_len = (int)(dtcp_buf->buf +  encrypted_data_size + dtcp_buf->data_len - ptr_in);
	if(dtcp_buf->data_len > 0){
		memmove(dtcp_buf, ptr_in, dtcp_buf->data_len);
	}
	//printf("gotted data len is %d\n", *clear_content_size);
	return 0;
}

void dtcp_test()
{

	char *url = "http://172.16.81.2:80/web/B-MP2PS_N-1.mpg?CONTENTPROTECTIONTYPE=DTCP1&DTCP1HOST=172.16.81.2&DTCP1PORT=8000";;
	char *content_type = NULL;
	int content_len = 0;
	int http_status = 0;
	int http_ret, ret = 0;
	void *http_handle;
	char *encrypt_data;
	int data_len;
	char clear_data[64 * 1024];
	int clear_len = 64 * 1024;
	FILE *fp;
	
	dtcp_client_init();
	render_track();
	if(dtcp_client_sink_create("172.16.81.2", 8000) == 0){
	render_track();
		http_ret = http_OpenHttpGet(url, &http_handle, &content_type, &content_len, &http_status, 3);
	render_track();
		if (http_ret < 0) {
			render_msg("error to connect url:%d\n", http_ret);
			return -1;
		}
		fp = fopen("dtcp.dump", "w");
		while(1){
			dtcp_client_buf_get(&encrypt_data, &data_len);
			http_ret = http_ReadHttpGet(http_handle, encrypt_data, &data_len, 1);
			if(http_ret != 0 && http_ret != -207){
				printf("return err:%d\n", http_ret);
				return -1;
			}
			if(data_len > 0){
				clear_len = 64 * 1024;
				dtcp_client_clear_data(encrypt_data, data_len, clear_data, &clear_len);
				fwrite(clear_data, clear_len, 1, fp);
			} else {
				fclose(fp);
				break;
			}			
		}
		dtcp_client_sink_destroy();
	render_track();
	}		
	exit(0);
	return -1;
}
#endif
#endif

