/*
    �ļ�:                   gfx_pulp.h
    ����(�� :               liupeng
    ����:
    �汾:                   Revision: 0.0.0.1
    ��������:               2004/7/7
    ����޸�����:           ��˭�޸�:           ˵����
*/


#ifndef _BUF_FS_H_
#define _BUF_FS_H_

typedef struct              /* structure for reading images from buffer   */
{
    unsigned char *start;   /* The pointer to the beginning of the buffer */
    unsigned long size;     /* The total size of the buffer               */
    unsigned long offset;   /* The current offset within the buffer       */
}buffer_t;

void        buf_fs_binit        ( buffer_t *buffer, void *startdata, int size );
long        buf_fs_bseek        ( buffer_t *buffer, long offset, int whence );
int         buf_fs_bread        ( buffer_t *buffer, void *dest, unsigned long size );
int         buf_fs_bgetc        ( buffer_t *buffer );
char        *buf_fs_bgets       ( buffer_t *buffer, char *dest, unsigned int size );
int         buf_fs_beof         ( buffer_t *buffer );

#endif
