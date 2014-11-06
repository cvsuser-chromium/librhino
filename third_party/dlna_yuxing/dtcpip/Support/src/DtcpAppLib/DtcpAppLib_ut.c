#include "DtcpAppLib.h"
#include "DtcpPacketizer.h"
#include "DtcpApi.h"
#include "StatusCodes.h"
#include "DtcpStatusCodes.h"

#include <stdio.h>
#include <memory.h>

//
// UNIT TESTS for DTCP Helper Functions
//
void EncryptionTest()
{
    char buffer[4096];
    char buf2[4096];

    char *buf = buffer;
    size_t buf_sz = sizeof(buffer);

    char *enc_buf = buf2;
    size_t enc_buf_sz = 4096;

    int res = -999;

    int test_i = 0;
    
    size_t content_processed = 0;

    struct dtcp_stream_t DtcpPacketizer_stream;

    // pad 0, 1 pcp, 1 call
    memset(buf, 0x99, buf_sz);
    memset(enc_buf, 0x88, enc_buf_sz); 
    buf_sz -= 2 + 14;
    do {
        if (0 != (res = DtcpPacketizer_CreateContentSource(&DtcpPacketizer_stream, 12, buf_sz, 4096)))
            break;
        if (0 != (res = DtcpPacketizer_PacketizeData(&DtcpPacketizer_stream, buf, buf_sz, &enc_buf, &enc_buf_sz, &content_processed)))
            break;
        if ((0 != (res = DtcpPacketizer_DestroyContentStream(&DtcpPacketizer_stream))))
            break;
        if ((enc_buf_sz != (sizeof(buf2) - 2)) && (res = -9))
            break;
    } while (0);
    if (IS_FAILURE(res)) {
        DEBUG_MSG(3, ("TEST FAILED: #%d res=%d\r\n", test_i, res));
    } else {
        DEBUG_MSG(3, ("TEST PASSED: #%d\r\n", test_i, res));
    }
    test_i++;

    // pad 13, 1 pcp, 1 call
    buf_sz = sizeof(buffer);
    enc_buf_sz = sizeof(buf2);
    memset(buf, 0x99, buf_sz);
    memset(enc_buf, 0x88, enc_buf_sz); 
    buf_sz -= 15 + 14;
    do {
        if (0 != (res = DtcpPacketizer_CreateContentSource(&DtcpPacketizer_stream, 12, buf_sz, 4096)))
            break;
        if (0 != (res = DtcpPacketizer_PacketizeData(&DtcpPacketizer_stream, buf, buf_sz, &enc_buf, &enc_buf_sz, &content_processed)))
            break;
        if ((0 != (res = DtcpPacketizer_DestroyContentStream(&DtcpPacketizer_stream))))
            break;
    } while (0);
    if (IS_FAILURE(res)) {
        DEBUG_MSG(3, ("TEST FAILED: #%d res=%d\r\n", test_i, res));
    } else {
        DEBUG_MSG(3, ("TEST PASSED: #%d\r\n", test_i, res));
    }
    test_i++;

    // no room for pcph
    buf_sz = sizeof(buffer);
    enc_buf_sz = 13;
    memset(buf, 0x99, buf_sz);
    memset(enc_buf, 0x88, enc_buf_sz); 
    do {
        if (0 != (res = DtcpPacketizer_CreateContentSource(&DtcpPacketizer_stream, 12, buf_sz, 4096)))
            break;
        if (0 != (res = DtcpPacketizer_PacketizeData(&DtcpPacketizer_stream, buf, buf_sz, &enc_buf, &enc_buf_sz, &content_processed)))
            break;
        if ((0 != (res = DtcpPacketizer_DestroyContentStream(&DtcpPacketizer_stream))))
            break;
    } while (0);
    if (IS_FAILURE(res)) {
        DEBUG_MSG(3, ("TEST FAILED: #%d res=%d\r\n", test_i, res));
    } else {
        DEBUG_MSG(3, ("TEST PASSED: #%d\r\n", test_i, res));
    }
    test_i++;

    // only room for 16 bytes of data post pcph
    buf_sz = sizeof(buffer);
    enc_buf_sz = 14 + 16;
    memset(buf, 0x99, buf_sz);
    memset(enc_buf, 0x88, enc_buf_sz); 
    do {
        if (0 != (res = DtcpPacketizer_CreateContentSource(&DtcpPacketizer_stream, 12, buf_sz, 4096)))
            break;
        if (0 != (res = DtcpPacketizer_PacketizeData(&DtcpPacketizer_stream, buf, buf_sz, &enc_buf, &enc_buf_sz, &content_processed)))
            break;
        if ((0 != (res = DtcpPacketizer_DestroyContentStream(&DtcpPacketizer_stream))))
            break;
    } while (0);
    if (IS_FAILURE(res) || enc_buf_sz !=30) {
        DEBUG_MSG(3, ("TEST FAILED: #%d res=%d\r\n", test_i, res));
    } else {
        DEBUG_MSG(3, ("TEST PASSED: #%d\r\n", test_i, res));
    }
    test_i++;

    // only room for 1 byte of data post pcph
    buf_sz = sizeof(buffer);
    enc_buf_sz = 14 + 1;
    memset(buf, 0x99, buf_sz);
    memset(enc_buf, 0x88, enc_buf_sz); 
    do {
        if (0 != (res = DtcpPacketizer_CreateContentSource(&DtcpPacketizer_stream, 12, buf_sz, 4096)))
            break;
        if (0 != (res = DtcpPacketizer_PacketizeData(&DtcpPacketizer_stream, buf, buf_sz, &enc_buf, &enc_buf_sz, &content_processed)))
            break;
        if ((0 != (res = DtcpPacketizer_DestroyContentStream(&DtcpPacketizer_stream))))
            break;
    } while (0);
    if (IS_FAILURE(res)) {
        DEBUG_MSG(3, ("TEST FAILED: #%d res=%d\r\n", test_i, res));
    } else {
        DEBUG_MSG(3, ("TEST PASSED: #%d\r\n", test_i, res));
    }
    test_i++;

    // no room for data post pcph
    buf_sz = sizeof(buffer);
    enc_buf_sz = 14;
    memset(buf, 0x99, buf_sz);
    memset(enc_buf, 0x88, enc_buf_sz); 
    do {
        if (0 != (res = DtcpPacketizer_CreateContentSource(&DtcpPacketizer_stream, 12, buf_sz, 4096)))
            break;
        if (0 != (res = DtcpPacketizer_PacketizeData(&DtcpPacketizer_stream, buf, buf_sz, &enc_buf, &enc_buf_sz, &content_processed)))
            break;
        if ((0 != (res = DtcpPacketizer_DestroyContentStream(&DtcpPacketizer_stream))))
            break;
    } while (0);
    if (IS_FAILURE(res)) {
        DEBUG_MSG(3, ("TEST FAILED: #%d res=%d\r\n", test_i, res));
    } else {
        DEBUG_MSG(3, ("TEST PASSED: #%d\r\n", test_i, res));
    }
    test_i++;

    // pad 0, 2 pcp, 1 call, no room left over
    buf_sz = 1024*2;
    enc_buf_sz = 2*14 + 2*1024;
    memset(buf, 0x99, buf_sz);
    memset(enc_buf, 0x88, enc_buf_sz); 
    do {
        if (0 != (res = DtcpPacketizer_CreateContentSource(&DtcpPacketizer_stream, 12, buf_sz, 1024)))
            break;
        if (0 != (res = DtcpPacketizer_PacketizeData(&DtcpPacketizer_stream, buf, buf_sz, &enc_buf, &enc_buf_sz, &content_processed)))
            break;
        if ((0 != (res = DtcpPacketizer_DestroyContentStream(&DtcpPacketizer_stream))))
            break;
    } while (0);
    if (IS_FAILURE(res) || (2*14 + 2*1024) != enc_buf_sz ) {
        DEBUG_MSG(3, ("TEST FAILED: #%d res=%d\r\n", test_i, res));
    } else {
        DEBUG_MSG(3, ("TEST PASSED: #%d\r\n", test_i, res));
    }
    test_i++;

    // bad PCPH size
    buf_sz = 1024*2;
    enc_buf_sz = 2*14 + 2*1024;
    memset(buf, 0x99, buf_sz);
    memset(enc_buf, 0x88, enc_buf_sz); 
    do {
        if (0 != (res = DtcpPacketizer_CreateContentSource(&DtcpPacketizer_stream, 12, buf_sz, 1023)))
            break;
        if (0 != (res = DtcpPacketizer_PacketizeData(&DtcpPacketizer_stream, buf, buf_sz, &enc_buf, &enc_buf_sz, &content_processed)))
            break;
        if ((0 != (res = DtcpPacketizer_DestroyContentStream(&DtcpPacketizer_stream))))
            break;
    } while (0);
    if (IS_FAILURE(res)) {
        DEBUG_MSG(3, ("TEST FAILED: #%d res=%d\r\n", test_i, res));
    } else {
        DEBUG_MSG(3, ("TEST PASSED: #%d\r\n", test_i, res));
    }
    test_i++;

    // pad 0, 2 pcp, 1 call, no room left over
    buf_sz = 2048;
    enc_buf_sz = 256;
    memset(buf, 0x99, buf_sz);
    memset(enc_buf, 0x88, enc_buf_sz);
    do {
        size_t data_done = buf_sz;
        if (0 != (res = DtcpPacketizer_CreateContentSource(&DtcpPacketizer_stream, 12, buf_sz, 1024)))
            break;
        while (0)
        {
            if (0 != (res = DtcpPacketizer_PacketizeData(&DtcpPacketizer_stream, buf, buf_sz, &enc_buf, &enc_buf_sz, &content_processed)))
               break;
        }
        if ((0 != (res = DtcpPacketizer_DestroyContentStream(&DtcpPacketizer_stream))))
            break;
    } while (0);
    if (IS_FAILURE(res) || (2*14 + 2*1024) != enc_buf_sz ) {
        DEBUG_MSG(3, ("TEST FAILED: #%d res=%d\r\n", test_i, res));
    } else {
        DEBUG_MSG(3, ("TEST PASSED: #%d\r\n", test_i, res));
    }
    test_i++;

    DEBUG_MSG(3, ("**** FINISHED ***\r\n"));
}
