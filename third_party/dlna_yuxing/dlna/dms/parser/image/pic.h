


/*
    �ļ�:                   pic.h
    ����(��):               liupeng
    ����:
    �汾:                   Revision: 0.0.0.1
    ��������:               2004/7/14
    ����޸�����:           ��˭�޸�:           ˵����
*/

#ifndef _PIC_H_
#define _PIC_H_

int picInfoPNM( buffer_t *pic, int *width, int *height, int *mode );
int picLoadPNM( buffer_t *pic, THE_PAPER paper );

int picInfoBMP( buffer_t *buf, int *width, int *height, int *bits );
int picLoadBMP( buffer_t *buf, THE_PAPER paper );

int picInfoJPG( buffer_t *buf, int *width, int *height, int *bits );
int picLoadJPG( buffer_t *buf, THE_PAPER paper );

int picInfoPNG( buffer_t *buf, int *width, int *height, int *bits );
int picLoadPNG( buffer_t *buf, THE_PAPER paper );


#endif
