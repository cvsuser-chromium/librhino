/*
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
*/

#include "DtcpPacketizer.h"

#include <stdio.h>
#include <memory.h>

#include "StatusCodes.h"
#include "DtcpConstants.h"
#include "DtcpApi.h"

#ifndef MIN
#   define MIN(a, b) ((a)<(b))?(a):(b)
#endif

int DtcpPacketizer_CreateContentSource(struct dtcp_stream_t *dtcp_stream, int content_emi,
                             size_t content_length, size_t max_packet_size)

{
	int res = LOGIC_ERROR;

    DEBUG_MSG(MSG_DEBUG, ("Creating new content source stream: 0x%08X - EMI=%d CL=%d PCPH=%d\r\n", 
        &dtcp_stream, content_emi, content_length, max_packet_size));

	// Validate arguments
	if (NULL == dtcp_stream ||                          // Caller required allocation
        0 == content_length ||                          // Ensure there's something to send
        0 == max_packet_size || 0 != max_packet_size%16 // DTCP PCPCL constraints
        || DTCP_MAX_PACKET_SIZE < max_packet_size)                
		return res = INVALID_ARGUMENT;

	memset(dtcp_stream, 0, sizeof(struct dtcp_stream_t));

	res = DtcpApi_OpenStream(streamTransportHttp, &dtcp_stream->dtcp_stream_handle);

	if (IS_FAILURE(res))
		return res;

    dtcp_stream->max_packet_size = max_packet_size;
	dtcp_stream->dtcp_stream_content_length = content_length;
	dtcp_stream->dtcp_stream_content_remaining = content_length;
    dtcp_stream->dtcp_emi = content_emi;
    dtcp_stream->source = 1;

	return res = SUCCESS;
}

int DtcpPacketizer_CreateContentSink(struct dtcp_stream_t *dtcp_stream, void *ake_handle)
{
	int res = LOGIC_ERROR;

    DEBUG_MSG(MSG_DEBUG, ("Creating new content sink stream: 0x%08X - AKE=0x%08X\r\n", dtcp_stream, ake_handle) );

	if (NULL == dtcp_stream || NULL == ake_handle)
        return res = INVALID_ARGUMENT;

	memset(dtcp_stream, 0, sizeof(struct dtcp_stream_t));
    dtcp_stream->dtcp_emi = -1; // INVALID EMI
    dtcp_stream->ake_handle = ake_handle;
    dtcp_stream->source = 0;

	return res = SUCCESS;
}

int DtcpPacketizer_DestroyContentStream(struct dtcp_stream_t *dtcp_stream)
{
	int res = LOGIC_ERROR;

	DEBUG_MSG(MSG_DEBUG, ("Destroyig content stream 0x%08x\r\n", dtcp_stream) );

	if (NULL == dtcp_stream)
		return INVALID_ARGUMENT;

	if (NULL != dtcp_stream->dtcp_packet_handle)
	{
		/* ignore = */ res = DtcpApi_ClosePacket(dtcp_stream->dtcp_packet_handle);
		dtcp_stream->dtcp_packet_handle = NULL;
	}

	if (NULL != dtcp_stream->dtcp_stream_handle)
	{
		/* ignore = */ res = DtcpApi_CloseStream(dtcp_stream->dtcp_stream_handle);
		dtcp_stream->dtcp_stream_handle = NULL;
	}

    // Error_OK if content send/receive is not complete for source/sink
    if ((1 == dtcp_stream->source && 0 != dtcp_stream->dtcp_stream_content_remaining) ||
        0 != dtcp_stream->dtcp_packet_content_remaining)
    {
        res = SUCCESS_FALSE;
    } else {
        res = SUCCESS;
    }

	return res;
}

int DtcpPacketizer_PacketizeData(/* IN */ struct dtcp_stream_t *dtcp_stream, 
				 /* IN */ char *clear_data, /* IN */ size_t clear_data_size, 
				 /* IN/OUT */ char **encrypted_data, /* IN/OUT */ size_t *encrypted_data_size,
				 /* OUT */ size_t *total_content_processed)
{
    char *buf_ptr = NULL;
    int res = LOGIC_ERROR;

    size_t data_to_send = 0;    // Total ammount of data processed during this call
                                // = (content + overhead[header + padding])
    size_t content_to_send = 0; // Total ammount of content to processed (each iteration)
    size_t content_padding = 0; // Bytes of padding needed (each iteration)

    DEBUG_MSG(MSG_DEBUG+2, ("DtcpPacketizer_PacketizeData(0x%08X, 0x%08X, %d, 0x%08x, 0x%08x, 0x%08x\r\n", dtcp_stream,
		  clear_data, clear_data_size, encrypted_data, encrypted_data_size, total_content_processed ));

    // Validate Params
    if (NULL == dtcp_stream	|| 0 == dtcp_stream->source || // Source only function
        NULL == clear_data || 0 == clear_data_size || NULL == total_content_processed || // Passed data?
        0 == dtcp_stream->dtcp_stream_content_length ) // Done sending?
      return res = INVALID_ARGUMENT;

    // Clear OUT param 
    *total_content_processed = 0;
    // Set our work buffer to the IN/OUT param
    buf_ptr = *encrypted_data;

    //
    // Work Loop
    //

    // while there is data to send & there is room left in the OUT buffer
    while (0 < dtcp_stream->dtcp_stream_content_remaining && *encrypted_data_size > data_to_send)
    {
        content_to_send = 0; // Reset content bytes processed
        content_padding = 0; // Reset padding bytes needed

        //
        // PCP Header Section
        //

        // Create a packet header if needed
        if (NULL == dtcp_stream->dtcp_packet_handle)
        {
            // Ensure the OUT buffer is large enough for the header
            if ( *encrypted_data_size < data_to_send + DTCP_CONTENT_PACKET_HEADER_SIZE)
            {
                DEBUG_MSG(MSG_DEBUG+2, ("DtcpPacketizer_PacketizeData: SUCCESS_FALSE - Not enough room in OUT buffer for PCPH.\r\n"));
                // This is not an error state.  We may have already processed data successfully.
                // This state is signified to the caller by normally returning the encrypted data
                // and size.  However, total_content_processed != clear_data_size (SUCCESS_FALSE)
                res = SUCCESS_FALSE;
                break;
            }

            // Compute PCPCL (content length) = MIN of clear data left AND specified max packet size
            dtcp_stream->dtcp_packet_content_length =
                MIN(dtcp_stream->dtcp_stream_content_remaining, dtcp_stream->max_packet_size);

            // 3C'

            // Create packet header and write into OUT buffer
            res = DtcpApi_CreatePacketHeader(
                dtcp_stream->dtcp_stream_handle,
                dtcp_stream->dtcp_emi & 0xFF,
                dtcp_stream->dtcp_packet_content_length,
                buf_ptr,
                &dtcp_stream->dtcp_packet_handle);

            if (IS_FAILURE(res)) {
                DEBUG_MSG(MSG_ERR, ("DtcpApi_CreatePacketHeader(): FAILED = %d\r\n", res) );
                // Propagate error to caller
                break;
            }

            // Set content remaining for PCP
            dtcp_stream->dtcp_packet_content_remaining = dtcp_stream->dtcp_packet_content_length;
            // Advance OUT buffer pointer by header size
            buf_ptr += DTCP_CONTENT_PACKET_HEADER_SIZE;
            // Advance count of data to send
            data_to_send += DTCP_CONTENT_PACKET_HEADER_SIZE;

            DEBUG_MSG(MSG_DEBUG+2, ("DtcpPacketizer_PacketizeData: Created PCPH - 0x%08X CL=%d\r\n\r\n", 
                dtcp_stream->dtcp_packet_handle, dtcp_stream->dtcp_packet_content_length));
        } // Create PCP Header


        //
        // PCP Body Section
        //

        // Compute ammount of content to encrypt to send
        // = MIN of total content left              // Caller may not have passed in a full packet worth of data
        //      AND content left the current PCP    // Caller may have paseed in more than a full packet of data
        //      AND ammount of OUT buffer left (16-byte aligned) // Caller may have passed an OUT buffer smaller than needed
        // Assumption: clear_dat_size MUST 16-byte aligned
        content_to_send = MIN(clear_data_size, dtcp_stream->dtcp_packet_content_remaining);
        content_to_send = MIN(content_to_send, ((*encrypted_data_size - data_to_send)/16)*16 );

        // Check to see if the OUT buffer has enought space to send anything, at least 16-bytes of data (AES block size)
        if ( 16 > (*encrypted_data_size - data_to_send))
        {
            DEBUG_MSG(MSG_DEBUG+2, ("DtcpPacketizer_PacketizeData: SUCCESS_FALSE - Not enough room in OUT buffer for PCP.\r\n"));
            // This is not an error state.  We may have already processed data successfully.
            // This state is signified to the caller by normally returning the encrypted data
            // and size.  However, total_content_processed != clear_data_size (SUCCESS_FALSE)
            res = SUCCESS_FALSE;
            break;
        }

        //
        // Add padding bytes if necessary (16-byte align)
		//
        // add padding
        if (0 == dtcp_stream->dtcp_packet_content_remaining - content_to_send)
        {        
            content_padding = (16 - (dtcp_stream->dtcp_packet_content_remaining % 16) ) % 16;
            // Ensure our buffer is large enough
            if (*encrypted_data_size < content_to_send   // to process now
                + content_padding                        // padding padding now
                + data_to_send        // already processed (incs header)
                )
            {
                DEBUG_MSG(MSG_DEBUG+2, ("Not enuf room for padding and next block: CTS=%d PAD=%d DTS=%d EDS=%d\r\n",
                    content_to_send, content_padding, data_to_send, *encrypted_data_size));
                break;
            }

            // Set the padding bits
            // Do not include in data_to_send until after encryption has completed
            memset(buf_ptr + content_to_send, content_padding, content_padding);            

            DEBUG_MSG(MSG_DEBUG+2, ("Added padding: PAD=%d\r\n", content_padding) );
        } else {
            // Ensure our buffer is large enough
            if (*encrypted_data_size < 
                    content_to_send // to process now             
                    + data_to_send  // already processed (incs header)
                )
            {
                DEBUG_MSG(MSG_DEBUG+2, ("Not enuf room for PCP Body: CTS=%d DTS=%d EDS=%d\r\n",
                    content_to_send, data_to_send, *encrypted_data_size));
                break;
            }
        }

        // Encrypt data
        res = DtcpApi_EncryptData(
            dtcp_stream->dtcp_packet_handle,
            clear_data,
            buf_ptr,
            content_to_send + content_padding );

        if (IS_FAILURE(res)) {
            DEBUG_MSG(MSG_ERR, ("FAILED: DtcpApi_EncryptData() = %d\r\n", res) );            
            break; // Return what we've done thus far
        }

        buf_ptr += content_to_send + content_padding;
        data_to_send += content_to_send + content_padding;
        *total_content_processed += content_to_send;

        // Decrement byte counters
        dtcp_stream->dtcp_packet_content_remaining -= content_to_send;
        dtcp_stream->dtcp_stream_content_remaining -= content_to_send;

        // Increment clear data pointers
        clear_data += content_to_send;
        clear_data_size -= content_to_send;

        DEBUG_MSG(MSG_DEBUG+2, ("Encrypted content: CTS=%d / SCR=%d PCR=%d\r\n",
            content_to_send,
            dtcp_stream->dtcp_stream_content_remaining, dtcp_stream->dtcp_packet_content_remaining));

        // Close header if needed
        if (0 == dtcp_stream->dtcp_packet_content_remaining)
        {
            res = DtcpApi_ClosePacket(dtcp_stream->dtcp_packet_handle); 
            dtcp_stream->dtcp_packet_handle = NULL;

            if (IS_FAILURE(res)) {
                DEBUG_MSG(MSG_ERR, ("FAILED: DtcpApi_ClosePacket() = %d\r\n", res) );                
                break;
            }

            DEBUG_MSG(MSG_DEBUG+2, ("Closed PCPH: 0x%08X SCR=%d\r\n", 
                &dtcp_stream->dtcp_packet_handle, 
                dtcp_stream->dtcp_stream_content_remaining));
        }

        // Is there more data to process?
        if (0 == clear_data_size)
        {
            DEBUG_MSG(MSG_DEBUG+2, ("Finished processing input data...\r\n"));
            break;
        }
    }

    *encrypted_data_size = data_to_send;
    
    DEBUG_MSG(MSG_DEBUG+2, ("Returning: EDS=%d\r\n", *encrypted_data_size));

    return res;
}

int DtcpPacketizer_DepacketizeData(struct dtcp_stream_t *dtcp_stream, 
				  /* IN */ char *encrypted_data, /* IN */ size_t encrypted_data_size, 
				  /* IN/OUT */ char *clear_content, /* IN/OUT */ size_t *clear_content_size,
                  /* OUT */ size_t *total_data_processed)
{
    char *buf_ptr = NULL;
    int res = LOGIC_ERROR;
    
    int data_to_process = 0;
    
    int content_processed = 0;
    int data_processed = 0;

    *total_data_processed = 0;

    // Validate Params
    if (NULL == dtcp_stream || NULL == encrypted_data || 0 == encrypted_data_size
        || NULL == clear_content || 0 == clear_content_size || 0 == *clear_content_size
        || NULL == total_data_processed)
    {
        return res = INVALID_ARGUMENT;
    }
    
    // Set our work buffer
    buf_ptr = clear_content;
    
    //
    // Decode Data
    //

    do {
        
        if ( ((NULL == dtcp_stream->dtcp_packet_handle) && ((encrypted_data_size - *total_data_processed) < DTCP_CONTENT_PACKET_HEADER_SIZE))
			 || ((encrypted_data_size - *total_data_processed) < 16) )
		{
            res = SUCCESS_FALSE;
            break;
        }

        //
        // Consume DTCP PCPH
        //
        if (NULL == dtcp_stream->dtcp_packet_handle) {

            res = DtcpApi_ConsumePacketHeader(
                dtcp_stream->ake_handle,
                encrypted_data,
                &dtcp_stream->dtcp_emi,
                &dtcp_stream->dtcp_packet_content_length,
                &dtcp_stream->dtcp_packet_handle);

            if (0 != res) {
                DEBUG_MSG(MSG_ERR, ("FAILED: DtcpApi_ConsumePacketHeader() == %d\r\n", res) );
                break;
            }

            DEBUG_MSG(MSG_DEBUG+2, ("Consumed header: EMI=%d CL=%u\r\n", dtcp_stream->dtcp_emi, dtcp_stream->dtcp_packet_content_length) );

            // Determine number of data bytes (content + padded bytes to read)
            dtcp_stream->dtcp_packet_bytes_remaining = 
                (dtcp_stream->dtcp_packet_content_length%16 != 0) ? 
                (dtcp_stream->dtcp_packet_content_length + (16 - (dtcp_stream->dtcp_packet_content_length % 16))) 
                : (dtcp_stream->dtcp_packet_content_length);

            // Determine if there are padding bytes
            dtcp_stream->dtcp_packet_padding_bytes = dtcp_stream->dtcp_packet_bytes_remaining - dtcp_stream->dtcp_packet_content_length;
           
            // Increment data pointers
            encrypted_data += DTCP_CONTENT_PACKET_HEADER_SIZE;
            // Increment byte counters
            *total_data_processed += DTCP_CONTENT_PACKET_HEADER_SIZE;
            data_processed += DTCP_CONTENT_PACKET_HEADER_SIZE;
        } // Consume PCPH        

        // Ensure we do not process more that PCPH content size
        data_to_process = MIN(dtcp_stream->dtcp_packet_bytes_remaining, encrypted_data_size - data_processed);
        data_to_process = MIN(data_to_process, *clear_content_size - content_processed);

        // Can only process non-header information in 16 bytes blocks
        data_to_process = (data_to_process%16 != 0) ? 
            (data_to_process - (data_to_process % 16)) : (data_to_process);     

        // Fall through if we have no data blocks to process
        if (data_to_process == 0)
		{
			// Done for now, however maybe not all of the data was consumed
			res = SUCCESS_FALSE;
            break;
		}

        // Decrypt data in place
        res = DtcpApi_DecryptData(dtcp_stream->dtcp_packet_handle,
            encrypted_data,
            buf_ptr,
            data_to_process);

        if (0 != res) {
            DEBUG_MSG(MSG_ERR, ("FAILED: DtcpApi_DecryptData() == %d\r\n", res) );
            break;
        }          

        // Increment data pointes byte counters
        encrypted_data += data_to_process;
        dtcp_stream->dtcp_packet_bytes_remaining -= data_to_process;        
        data_processed += data_to_process;
        *total_data_processed += data_to_process;
        dtcp_stream->dtcp_stream_content_length += data_to_process;
        
        // Account for padding if necessary
        if (0 == dtcp_stream->dtcp_packet_bytes_remaining)
        {
            buf_ptr += data_to_process - dtcp_stream->dtcp_packet_padding_bytes;
            content_processed += data_to_process - dtcp_stream->dtcp_packet_padding_bytes;
            // Finished with packet data, close pakcet
            res = DtcpApi_ClosePacket(dtcp_stream->dtcp_packet_handle);
            dtcp_stream->dtcp_packet_handle = NULL;
        } else {
            buf_ptr += data_to_process;
            content_processed += data_to_process;
        }
        
        // determine if there is more data that can be processed
        data_to_process = encrypted_data_size - data_processed;

     } while (0 != data_to_process);

    *clear_content_size = content_processed;    

    return res;
}
