


/*
    文件:                   pic.h
    作者(们):               liupeng
    盟友:
    版本:                   Revision: 0.0.0.1
    建立日期:               2004/7/14
    最后修改日期:           被谁修改:           说明：
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
