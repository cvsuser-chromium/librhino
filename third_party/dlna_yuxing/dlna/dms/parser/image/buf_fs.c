/*
    文件:                   buf_fs.c
    作者(们):               liupeng
    盟友:
    版本:                   Revision: 0.0.0.1
    建立日期:               2004/7/14
    最后修改日期:           被谁修改:           说明：
*/

#include "gfx_paper.h"
#include "buf_fs.h"

/* ======================================================== */
void buf_fs_binit( buffer_t *buffer, void *startdata, int size )
{
    buffer->start = startdata;
    buffer->size = size;
    buffer->offset = 0;
}
/* ======================================================== */
long buf_fs_bseek( buffer_t *buffer, long offset, int whence )
{
    long new;

    switch( whence )
    {
        case SEEK_SET:
            if( offset >= buffer->size || offset < 0 )
                return ERROR;
            buffer->offset = offset;
            break;

        case SEEK_CUR:
            new = buffer->offset + offset;
            if( new >= buffer->size || new < 0 )
                return ERROR;
            buffer->offset = new;
            break;

        case SEEK_END:
            new = buffer->size - 1 + offset;
            if( new >= buffer->size || new < 0 )
                return ERROR;
            buffer->offset = new;
            break;

        default:
            return ERROR;
    }
    return buffer->offset;
}
/* ======================================================== */
int buf_fs_bread( buffer_t *buffer, void *dest, unsigned long size )
{
    unsigned long copysize;

    if( buffer->offset == buffer->size )
        return 0;

    if( (buffer->offset + size) > buffer->size )
        copysize = buffer->size - buffer->offset;
    else
        copysize = size;

    memcpy( dest, buffer->start + buffer->offset, copysize );

    buffer->offset += copysize;
    return copysize;
}
/* ======================================================== */
int buf_fs_bgetc(buffer_t *buffer)
{
    if( buffer->offset == buffer->size )
        return EOF;
    return buffer->start[buffer->offset++];
}
/* ======================================================== */
char *buf_fs_bgets( buffer_t *buffer, char *dest, unsigned int size )
{
    int i,o;
    unsigned int copysize = size - 1;

    if( buffer->offset == buffer->size )
        return 0;

    if( buffer->offset + copysize > buffer->size )
        copysize = buffer->size - buffer->offset;

    for( o=0, i=buffer->offset; i < buffer->offset + copysize; i++, o++ )
    {
        if( (dest[o] = buffer->start[i]) == '\n' )
            break;
    }

    buffer->offset = i + 1;
    dest[o + 1] = 0;

    return dest;
}
/* ======================================================== */
int buf_fs_beof( buffer_t *buffer )
{
    return( buffer->offset == buffer->size );
}
