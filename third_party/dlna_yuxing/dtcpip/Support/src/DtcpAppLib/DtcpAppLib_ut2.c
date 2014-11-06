#if 0
#include "DtcpAppLib.h"
#include "DtcpPacketizer.h"
#include "DtcpApi.h"
#include "StatusCodes.h"
#include "DtcpStatusCodes.h"

#include <stdio.h>
//#include <memory.h>

#if defined( EFENCE )
#   include <efence.h>
#endif

#include "ILibParsers.h"
#include "ILibWebServer.h"
#include "ILibWebClient.h"
#include "ILibAsyncSocket.h"

//
// UNIT TESTS for DTCP Helper Functions
//
#define BUF_SZ (4096)

#define SETBUFS(csz, esz) buf = buffer; enc_buf = buf2; memset(buf, 0x99, csz); buf_sz=csz; memset(enc_buf, 0x88, esz); enc_buf_sz=esz;

#define PACKETIZE1(pktsz) { do { \
        if (IS_FAILURE(res = DtcpPacketizer_CreateContentSource(&DtcpPacketizer_stream, 12, buf_sz, pktsz))) break; \
        if (IS_FAILURE(res = DtcpPacketizer_PacketizeData(&DtcpPacketizer_stream, buf, buf_sz, &enc_buf, &enc_buf_sz, &content_processed))) break; \
        if (IS_FAILURE(res = DtcpPacketizer_DestroyContentStream(&DtcpPacketizer_stream))) break; \
    } while (0); }

#define PASS_PASS(t, res) \
    if (IS_FAILURE(res)) { \
        DEBUG_MSG(3, ("TEST FAILED: #%d res=%d\r\n", t, res)); \
    } else { \
        DEBUG_MSG(3, ("TEST PASSED: #%d\r\n", t, res)); \
    }

#define PASS_FAIL(t, res) \
    if (IS_SUCCESS(res)) { \
        DEBUG_MSG(3, ("TEST FAILED: #%d res=%d\r\n", t, res)); \
    } else { \
        DEBUG_MSG(3, ("TEST PASSED: #%d\r\n", t, res)); \
    }

#define PACKETIZE2a(pktsz) \
        { int isInit = 0; do { \
            if (IS_FAILURE(res = DtcpPacketizer_CreateContentSource(&DtcpPacketizer_stream, 12, buf_sz, pktsz))) break; \
            isInit = 1; \
            if (IS_FAILURE(res = DtcpPacketizer_PacketizeData(&DtcpPacketizer_stream, buf, buf_sz, &enc_buf, &enc_buf_sz, &content_processed))) break;

#define PACKETIZE2b(esz) \
            buf += content_processed; buf_sz-=content_processed; enc_buf_sz=esz; \
            if (IS_FAILURE(res = DtcpPacketizer_PacketizeData(&DtcpPacketizer_stream, buf, buf_sz, &enc_buf, &enc_buf_sz, &content_processed))) break;

#define PACKETIZE2b2(sz, esz) \
            buf += content_processed; buf_sz=sz; enc_buf_sz=esz; \
            if (IS_FAILURE(res = DtcpPacketizer_PacketizeData(&DtcpPacketizer_stream, buf, buf_sz, &enc_buf, &enc_buf_sz, &content_processed))) break;

#define PACKETIZE2c \
            } while (0); \
        if (1==isInit) res = DtcpPacketizer_DestroyContentStream(&DtcpPacketizer_stream); \
        }

#define TEST(s, e) if ((s)) { res = e; break; }

#define TEST_HEADER(ti, tn) printf("----------------------------------------\r\n\t\tTEST %d -- %s\r\n", ti, tn);

void EncryptionTest()
{
    char buffer[BUF_SZ*4];
    char buf2[BUF_SZ*4];

    char *buf = buffer;
    size_t buf_sz = sizeof(buffer);

    char *enc_buf = buf2;
    size_t enc_buf_sz = 4096;

    int res = -999;

    int test_i = 0;
    
    size_t content_processed = 0;

    struct dtcp_stream_t DtcpPacketizer_stream;

    // -- --
    TEST_HEADER(test_i, "A");
    SETBUFS(16, 14+16);
    PACKETIZE1(32);    
    PASS_PASS(test_i, res);
    test_i++;

#if 0
    // pad 0, 1 pcp, 1 call, can't fit anything
    TEST_HEADER(test_i, "B");
    SETBUFS(17, 77);
    PACKETIZE1(16);
    if (res!=-3) res = -997;
    PASS_PASS(test_i, -3==res);
    test_i++;
#endif

    // pad 0, 1 pcp, 1 call, 2 extra bytes in enc
    TEST_HEADER(test_i, "B");
    SETBUFS(4080, BUF_SZ); // 14 + 2; // align to 16bytes
    PACKETIZE1(buf_sz);
    if (enc_buf_sz!=4096-2) res = -999;
    if (content_processed!=4080) res = -998;
    PASS_PASS(test_i, res);
    test_i++;

    // pad 0, 1 pcp, 1 call, perfect fit
    TEST_HEADER(test_i, "D");
    SETBUFS(4080, BUF_SZ-2); // 14 + 2; // align to 16bytes    
    PACKETIZE1(buf_sz);
    if (enc_buf_sz!=4096-2) res = -999;
    if (content_processed!=4080) res = -998;
    PASS_PASS(test_i, res);
    test_i++;

    // pad 0, 1 pcp, 1 call, perfect fit
    TEST_HEADER(test_i, "E");
    SETBUFS(4080, 4078);    
    PACKETIZE1(buf_sz);
    if (res!=1) res = -997;
    if (enc_buf_sz!=4078) res = -999;
    if (content_processed!=4080-16) res = -998;
    PASS_PASS(test_i, res);
    test_i++;

    // pad 0, 1 pcp, 1 call, can't fit pcph
    TEST_HEADER(test_i, "F");
    SETBUFS(4080, 12);
    PACKETIZE1(buf_sz);
    if (res!=1) res = -997;
    if (enc_buf_sz!=0) res = -999;
    if (content_processed!=0) res = -998;
    PASS_PASS(test_i, res);
    test_i++;

    // pad 0, 1 pcp, 1 call, can't fit body
    TEST_HEADER(test_i, "G");
    SETBUFS(4080, 14);
    PACKETIZE1(buf_sz);
    if (res!=1) res = -997;
    if (enc_buf_sz!=14) res = -999;
    if (content_processed!=0) res = -998;
    PASS_PASS(test_i, res);
    test_i++;

    // pad 0, 1 pcp, 1 call, can't fit body 16-byte aligned
    TEST_HEADER(test_i, "H");
    SETBUFS(4080, 14+8);
    PACKETIZE1(buf_sz);
    if (res!=1) res = -997;
    if (enc_buf_sz!=14) res = -999;
    if (content_processed!=0) res = -998;
    PASS_PASS(test_i, res);
    test_i++;

    // pad 0, 1 pcp, 1 call, can't fit anything
    TEST_HEADER(test_i, "I");
    SETBUFS(4080, 0);
    PACKETIZE1(buf_sz);
    if (res!=-3) res = -997;
    PASS_PASS(test_i, -3==res);
    test_i++;

//--
    // pad 0, 2 pcp, 1 call, perfect fit
    TEST_HEADER(test_i, "J");
    SETBUFS(1024*2, (14+1024)*2);
    PACKETIZE1(1024);    
    if (enc_buf_sz!=(14+1024)*2) res = -999;
    if (content_processed!=1024*2) res = -998;
    PASS_PASS(test_i, res);
    test_i++;

    // pad 0, 16 pcp, 1 call, perfect fit
    TEST_HEADER(test_i, "U");
    SETBUFS(16*16, (14+16)*16);
    PACKETIZE1(16);
    if (enc_buf_sz!=(14+16)*16) res = -999;
    if (content_processed!=16*16) res = -998;
    PASS_PASS(test_i, res);
    test_i++;

    // pad 15, 1 pcp, 2 call, 16 bytes extra
    TEST_HEADER(test_i, "V");
    SETBUFS(16+1, 32);
    PACKETIZE2a(32);
    if (enc_buf_sz!=30) { res = -999; break; }
    if (content_processed!=16) { res = -998; break; }
    PACKETIZE2b(32);
    if (enc_buf_sz!=16) { res = -997; break; }
    if (content_processed!=1) { res = -996; break; }
    PACKETIZE2c;
    PASS_PASS(test_i, res);
    test_i++;

    // pad 15, 1 pcp, 3 call, 16 bytes extra
    TEST_HEADER(test_i, "W");
    SETBUFS(2*16+1, 16);
    PACKETIZE2a(48);
    if (enc_buf_sz!=14) { res = -999; break; }
    if (content_processed!=0) { res = -998; break; }
    PACKETIZE2b(16);
    if (enc_buf_sz!=16) { res = -997; break; }
    if (content_processed!=16) { res = -996; break; }
    PACKETIZE2b(16);
    if (enc_buf_sz!=16) { res = -995; break; }
    if (content_processed!=16) { res = -994; break; }
    PACKETIZE2b(16);
    if (enc_buf_sz!=16) { res = -993; break; }
    if (content_processed!=1) { res = -992; break; }
    PACKETIZE2c;
    PASS_PASS(test_i, res);
    test_i++;
//--

    // pad 0, 1 pcp, 2 call, perfect fit
    TEST_HEADER(test_i, "K");
    SETBUFS(4080, 14+4080/2-8);
    PACKETIZE2a(4080);
    if (enc_buf_sz!=4080/2+14-8) { res = -999; break; }
    if (content_processed!=4080/2-8) { res = -998; break; }
    PACKETIZE2b(4080/2+8);
    if (enc_buf_sz!=4080/2+8) { res = -999; break; }
    if (content_processed!=4080/2+8) { res = -998; break; }
    PACKETIZE2c;
    PASS_PASS(test_i, res);
    test_i++;

    // pad 15, 1 pcp, 2 call, perfect fit
    TEST_HEADER(test_i, "L");
    SETBUFS(4080-15, 14+4080/2-8);
    PACKETIZE2a(4080);
    if (enc_buf_sz!=4080/2+14-8) { res = -999; break; }
    if (content_processed!=4080/2-8) { res = -998; break; }
    PACKETIZE2b(4080/2+8);
    if (enc_buf_sz!=4080/2+8) { res = -999; break; }
    if (content_processed!=4080/2+8-15) { res = -998; break; }
    PACKETIZE2c;
    PASS_PASS(test_i, res);
    test_i++;

//--
    // fringe else padding case
    // pad 15, 1 pcp, 1 call, perfect fit
    TEST_HEADER(test_i, "M");
    SETBUFS(17, 16+14);
    PACKETIZE2a(32);
    if (enc_buf_sz!=14+16) { res = -999; }
    if (content_processed!=16) { res = -998; }
    PACKETIZE2b(16);
    if (enc_buf_sz!=16) { res = -999; }
    if (content_processed!=1) { res = -998; }
    PACKETIZE2c;
    PASS_PASS(test_i, res);
    test_i++;

    // pad 1, 1 pcp, 1 call, perfect fit
    TEST_HEADER(test_i, "N");
    SETBUFS(16+15, 16+14);
    PACKETIZE2a(32);
    if (enc_buf_sz!=14+16) { res = -999; }
    if (content_processed!=16) { res = -998; }
    PACKETIZE2b(16);
    if (enc_buf_sz!=16) { res = -999; }
    if (content_processed!=15) { res = -998; }
    PACKETIZE2c;
    PASS_PASS(test_i, res);
    test_i++;

//----------

#if 0
    TEST_HEADER(test_i, "O");
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

    TEST_HEADER(test_i, "P");
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

    TEST_HEADER(test_i, "Q");
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

    TEST_HEADER(test_i, "R");
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

    TEST_HEADER(test_i, "S");
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

    // Invalid PCPH packet size
    TEST_HEADER(test_i, "T");
    SETBUFS(2048, 28+2048);
    PACKETIZE1(1024-1);
    //if (res!=INVALID_ARGUMENT) res = -997; else res = SUCCESS;
    PASS_PASS(test_i, res==INVALID_ARGUMENT);
    test_i++;    
#endif

    // pad 8, 1 pcp, 2 call, second call not aligned
    TEST_HEADER(test_i, "X");
    SETBUFS(1016, 512+14);
    PACKETIZE2a(2048);
    if (enc_buf_sz!=512+14) { res = -999; break; }
    if (content_processed!=512) { res = -998; break; }
    PACKETIZE2b(512);
    if (enc_buf_sz!=512) { res = -999; break; }
    if (content_processed!=504) { res = -998; break; }
    PACKETIZE2c;
    PASS_PASS(test_i, res);
    test_i++;

    // targeted - second call unaligned
    TEST_HEADER(test_i, "Y");
    SETBUFS(9003+16, 14);
    PACKETIZE2a(128*1024*1024);
    if (enc_buf_sz!=14) { res = -999; break; }
    if (content_processed!=0) { res = -998; break; }
    PACKETIZE2b(32*1024);
    if (enc_buf_sz!=9008) { res = -999; break; }
    if (content_processed!=9003) { res = -998; break; }
    PACKETIZE2c;
    PASS_PASS(test_i, res);
    test_i++;


    DEBUG_MSG(3, ("**** FINISHED ***\r\n"));
}

void EncryptionTest_Random(unsigned int iterations)
{    
    unsigned int max_buf = 64*1024;

    unsigned int failures = 0;
    unsigned int passes = 0;
    int res = 0;
    int count = 0;

    struct dtcp_stream_t DtcpPacketizer_stream;

    srand(time(NULL)+iterations);

    for (count = 0 ;count<iterations; count++)
    {
        // generate random buffer sizes
        unsigned int buf_sz = rand()%(max_buf-16)+16;        
        unsigned int ebuf_sz = rand()%(max_buf-16)+16;
        unsigned int ebuf_len = ebuf_sz;
        char *buf = (char *) malloc(buf_sz);
        char *ebuf = (char *) malloc(ebuf_sz);
        // generate a random 16byte aligned pcp sz < 128MB        
        unsigned int pcpsz = ((rand()%(128*1024*1024-16))/16)*16+16;

        unsigned int buf_processed = 0;
        unsigned int work_loops = 0;
        int isInit = 0;
        do {            
            if (IS_FAILURE(res = DtcpPacketizer_CreateContentSource(&DtcpPacketizer_stream, 12, buf_sz, pcpsz))) break;
            isInit = 1;
            
            work_loops = 0;
            while (buf_processed<buf_sz) {
                int content_processed = 0;
                if (IS_FAILURE(
                        res = DtcpPacketizer_PacketizeData(&DtcpPacketizer_stream, 
                                buf+buf_processed, buf_sz-buf_processed, 
                                &ebuf, &ebuf_len, &content_processed)
                                )
                    ) break;

                buf_processed += content_processed;
                work_loops++;
            }
        } while (0); // work loop

        if (1==isInit)
            res = DtcpPacketizer_DestroyContentStream(&DtcpPacketizer_stream);

        if (IS_FAILURE(res)) {
            failures++;
            printf("FAILURE: iteration=%d bufsz=%d ebufsz=%d pcpsz=%d workloop=%d\r\n", 
                iterations, buf_sz, ebuf_sz, pcpsz, work_loops);
        } else {
            passes++;
        }
        
        free(buf);
        free(ebuf);

        printf("[%12d / %12d]                            \r", count, iterations);
    } // iterations

    printf("\r\nResults\r\n\tFailures\t%d\r\n\tPasses\t%d\r\n",
        failures, passes);
}

void EncryptionDecryptionTest_Random(unsigned int iterations)
{    
    unsigned int max_buf = 64*1024;

    unsigned int failures = 0;
    unsigned int passes = 0;
    int res = 0;
    int count = 0;

    struct dtcp_stream_t source;
    struct dtcp_stream_t sink;

    srand(time(NULL)+iterations);

    for (count = 0 ;count<iterations; count++)
    {
        // generate random buffer sizes
        unsigned int buf_sz = rand()%(max_buf-16)+16;        
        unsigned int ebuf_sz = rand()%(max_buf-16)+16;        
        unsigned int ebuf_len = ebuf_sz;
        unsigned int cbuf_sz = rand()%(max_buf-16)+16;
        unsigned int cbuf_len = cbuf_sz;
        char *buf = (char *) malloc(buf_sz);
        char *ebuf = (char *) malloc(ebuf_sz);
        char *cbuf = (char *) malloc(cbuf_sz);
        // generate a random 16byte aligned pcp sz < 128MB        
        unsigned int pcpsz = ((rand()%(128*1024*1024-16))/16)*16+16;

        unsigned int buf_processed = 0;
        unsigned int cbuf_processed = 0;

        unsigned int work_loops = 0;
        int isInitSource = 0;
        int isInitSink = 0;
        int isInitAke = 0;

        void *akeHandle = NULL;

        memset(buf,  0x11, buf_sz);
        memset(ebuf, 0xFF, ebuf_sz);
        memset(cbuf, 0x22, cbuf_sz);              

        do {            
            if (IS_FAILURE( res = DtcpAppLib_DoAke("127.0.0.1", DtcpAppLib_GetSourcePort(), &akeHandle))) break;
            isInitAke = 1;

            if (IS_FAILURE(res = DtcpPacketizer_CreateContentSink(&sink, akeHandle))) break;
            isInitSink = 1;

            if (IS_FAILURE(res = DtcpPacketizer_CreateContentSource(&source, 12, buf_sz, pcpsz))) break;
            isInitSource = 1;
            
            work_loops = 0;
            while (buf_processed<buf_sz) {
                int content_processed = 0;
                ebuf_len = ebuf_sz;
                if (IS_FAILURE(
                        res = DtcpPacketizer_PacketizeData(&source, 
                                buf+buf_processed, buf_sz-buf_processed, 
                                &ebuf, &ebuf_len, &content_processed)
                                )
                    ) break;

                buf_processed += content_processed;
                
                cbuf_len = cbuf_sz;                
                if (IS_FAILURE(
                        res = DtcpPacketizer_DepacketizeData(&sink, 
                                &ebuf, content_processed,
                                cbuf, &cbuf_len, &content_processed)
                                )
                    ) break;
                cbuf_processed += content_processed;
               
                work_loops++;
            }
        } while (0); // work loop

        if (1==isInitAke)
            res = DtcpAppLib_CloseAke(akeHandle);

        if (1==isInitSink)
            res = DtcpPacketizer_DestroyContentStream(&sink);

        if (1==isInitSource)
            res = DtcpPacketizer_DestroyContentStream(&source);

        if (IS_SUCCESS(res) && cbuf_processed != buf_processed)
            res = -999;

        if (IS_FAILURE(res)) {
            failures++;
            printf("FAILURE: iteration=%d bufsz=%d ebufsz=%d pcpsz=%d workloop=%d\r\n", 
                iterations, buf_sz, ebuf_sz, pcpsz, work_loops);
        } else {
            passes++;
        }
        
        free(buf);
        free(ebuf);

        printf("[%12d / %12d]                            \r", count, iterations);
    } // iterations

    printf("\r\nResults\r\n\tFailures\t%d\r\n\tPasses\t%d\r\n",
        failures, passes);
}


struct send_stream_t
{
    struct dtcp_stream_t DtcpPacketizer_stream;
    int isInit;

    unsigned int buf_sz;
    char *buf;

    unsigned int ebuf_sz;
    unsigned int ebuf_len;
    char *ebuf;

    unsigned int pcpsz;

    unsigned int buf_processed;
    unsigned int work_loops;
    unsigned int count;
    unsigned int iterations;

    unsigned int failures;
    unsigned int passes;

    HANDLE sem;
};

void EncryptionServer_OnSendOK(struct ILibWebServer_Session *sender)
{
    //SetEvent(((struct send_stream_t *)sender->User)->sem);
}

void EncryptionServer_SendStream(struct ILibWebServer_Session *session, unsigned int iterations)
{
    unsigned int max_buf = 64*1024;

    unsigned int failures = 0;
    unsigned int passes = 0;
    int res = 0;

    struct send_stream_t *stream = malloc(sizeof(struct send_stream_t));
    memset(stream, 0, sizeof(struct send_stream_t));
    
    session->User = stream;

    srand(time(NULL)+iterations);
    
    stream->iterations = iterations;
    stream->sem=CreateEvent(NULL, TRUE, FALSE, NULL);

    // Send HTTP header
    {
        char buf[1024];
        int bufLen = sprintf(buf, "\r\nServer: Intel CEL / MicroMediaServer\r\nAccept-Range: bytes\r\nContent-Type: application/x-dtcp1;CONTENTFORMAT=application/octet-stream");
        /*ignore =*/ ILibWebServer_StreamHeader_Raw(session,200,"OK",buf,ILibAsyncSocket_MemoryOwnership_USER);
    }

    for (stream->count = 0 ;stream->count<stream->iterations; stream->count++)
    {
        // generate random buffer sizes
        stream->buf_sz = rand()%(max_buf-16)+16;
        stream->ebuf_sz = rand()%(max_buf-16)+16;
        stream->ebuf_len = stream->ebuf_sz;
        stream->buf = (char *) malloc(stream->buf_sz);
        stream->ebuf = (char *) malloc(stream->ebuf_sz);
        // generate a random 16byte aligned pcp sz < 128MB
        stream->pcpsz = ((rand()%(128*1024*1024-16))/16)*16+16;

        stream->buf_processed = 0;
        stream->work_loops = 0;
        stream->isInit = 0;
        
        do {            
            if (IS_FAILURE(res = DtcpPacketizer_CreateContentSource(&stream->DtcpPacketizer_stream, 12, stream->buf_sz, stream->pcpsz))) break;
            stream->isInit = 1;
            
            stream->work_loops = 0;
            while (stream->buf_processed<stream->buf_sz) {
                int content_processed = 0;
                if (IS_FAILURE(
                        res = DtcpPacketizer_PacketizeData(&stream->DtcpPacketizer_stream, 
                                stream->buf+stream->buf_processed, stream->buf_sz-stream->buf_processed, 
                                &stream->ebuf, &stream->ebuf_len, &content_processed)
                                )
                    )
                    break;

                { // new scope
                    int send_flag =  ILibWebServer_StreamBody(session,
                                        stream->ebuf,
                                        content_processed,
                                        ILibAsyncSocket_MemoryOwnership_USER,
                                        (stream->buf_processed==stream->buf_sz && stream->count==stream->iterations)?1:0);
                    if (0!=send_flag) {
                        //WaitForSingleObject(stream->sem,INFINITE);
                        Sleep(100);
                    }
                }

                stream->buf_processed += content_processed;
                stream->work_loops++;
            }
        } while (0); // work loop

        if (1==stream->isInit)
            res = DtcpPacketizer_DestroyContentStream(&stream->DtcpPacketizer_stream);

        if (IS_FAILURE(res)) {
            stream->failures++;
            printf("FAILURE: iteration=%d bufsz=%d ebufsz=%d pcpsz=%d workloop=%d\r\n", 
                stream->iterations, stream->buf_sz, stream->ebuf_sz, stream->pcpsz, stream->work_loops);
        } else {
            stream->passes++;
        }
        
        free(stream->buf);
        free(stream->ebuf);

        printf("[%12d / %12d]                            \r", stream->count, stream->iterations);

    } // iterations

    printf("\r\nResults\r\n\tFailures\t%d\r\n\tPasses\t%d\r\n",
        stream->failures, stream->passes);
    
    CloseHandle(stream->sem);
    free(stream);
}

void EncryptionServer_Random_OnReveive(struct ILibWebServer_Session *session,
					int InterruptFlag,
					struct packetheader *header,
					char *bodyBuffer, int *beginPointer,
					int endPointer, int done)
{
    // consume the data
    *beginPointer = endPointer;

    EncryptionServer_SendStream(session, rand());
        
}

void EncryptionServer_Random_OnSession(struct ILibWebServer_Session *SessionToken, void *User)
{
    SessionToken->OnReceive = &EncryptionServer_Random_OnReveive;
    SessionToken->OnSendOK = &EncryptionServer_OnSendOK;
}

void EncryptionServer_Random()
{
    void *chain = ILibCreateChain();
    void *web_server = ILibWebServer_Create(chain, 1, 8008, &EncryptionServer_Random_OnSession, NULL);
    ILibStartChain(chain);
    getchar();
} 
#endif

