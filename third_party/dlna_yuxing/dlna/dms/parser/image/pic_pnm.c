/*
    文件:                   pnm.c
    作者(们):               liupeng
    盟友:
    版本:                   Revision: 0.0.0.1
    建立日期:               2004/4/12
    说明：
        BMP decoding routine

    最后修改日期:           被谁修改:           说明：
*/

#include "gfx_paper.h"
#include "constant.h"
#include "buf_fs.h"
#include "pic.h"


#define PNM_TYPE_P1         1
#define PNM_TYPE_P2         2
#define PNM_TYPE_P3         3
#define PNM_TYPE_P4         4
#define PNM_TYPE_P5         5
#define PNM_TYPE_P6         6

static int common( buffer_t *pic );
static int whitespace( buffer_t *pic );
static int magic( buffer_t *pic, int *type );
static int width_height_max( buffer_t *pic, int type, int *witch, int *height, int *max );
/* ======================================================== */
int picInfoPNM( buffer_t *pic, int *width, int *height, int *mode )
{
    int max, type;

    buf_fs_bseek( pic, 0, SEEK_SET );

    /* ---- header info ---- */
    /* magic */
    if( magic( pic, &type ) == ERROR )
        return ERROR;

    /* width  height*/
    if( width_height_max( pic, type, width, height, &max ) == ERROR )
        return ERROR;

    switch( type )
    {
        case PNM_TYPE_P1:
        case PNM_TYPE_P2:
        case PNM_TYPE_P3:
            return ERROR;

        case PNM_TYPE_P4:
        case PNM_TYPE_P5:
            *mode = PAPER_MODE_5650;
            break;

        case PNM_TYPE_P6:
            *mode = PAPER_MODE_5650;
            break;
    }
    return OK;
}

/* ======================================================== */
int picLoadPNM( buffer_t *pic, THE_PAPER paper )
{
    int         type, width, height, max;
    PALETTE  clut;

    uchar       c, r,g,b;
    ulong       color;
    int         x, y;
    int         ix, iy, ic, step, inc;

    buf_fs_bseek( pic, 0, SEEK_SET );
    //printf("pnm mode is %d\n",paper->mode);

    /* ---- header info ---- */
    /* magic */
    if( magic( pic, &type ) == ERROR )
        return ERROR;

    /* width  height*/
    if( width_height_max( pic, type, &width, &height, &max ) == ERROR )
        return ERROR;

    /* ---- data ---- */
	if( (paper->mode != PAPER_MODE_5551 )&&(paper->mode != PAPER_MODE_5650 )&&(paper->mode != PAPER_MODE_0888)&&(paper->mode != PAPER_MODE_8888 ))
        return ERROR;
    if( paper->w != width )
        return ERROR;
    if( paper->h != height )
        return ERROR;


    switch( type )
    {
        /* 1/0  */
        case PNM_TYPE_P4:
            clut.colors[0] = paper->function->color( paper, 0, 0, 0,COLOR_ALPHA_000 );
            clut.colors[1] = paper->function->color( paper, 255, 255, 255, COLOR_ALPHA_000 );

            c = buf_fs_bgetc( pic );
            ic = 7;
            iy = height;
            y = 0;
            while( iy-- )
            {
                ix = width;
                x = 0;
                while( ix-- )
                {
                    if( c & bit8[ic] )
                        paper->function->putpixel( paper, x++, y, clut.colors[1] );
                    else
                        paper->function->putpixel( paper, x++, y, clut.colors[0] );
                    if( ic-- == 0 )
                    {
                        c = buf_fs_bgetc( pic );
                        ic = 7;
                    }
                }
                y++;
            }
            break;
            /* gray */
        case PNM_TYPE_P5:
            inc = 255 / max;
            step = 0;
            for( ix = 0; ix < max; ix++ )
            {
                clut.colors[ix] = paper->function->color( paper, step, step, step, COLOR_ALPHA_000 );
                step += inc;
            }
            clut.colors[max] = paper->function->color( paper, 255, 255, 255, COLOR_ALPHA_000 );

            iy = height;
            y = 0;
            while( iy-- )
            {
                ix = width;
                x = 0;
                while( ix-- )
                {
                    c = buf_fs_bgetc( pic );
                    paper->function->putpixel( paper, x++, y, clut.colors[c] );
                }
                y++;
            }
            break;
        /* true */
        case PNM_TYPE_P6:
            inc =  0;
            for( ix = 0; ix < 7; ix++ )
            {
                if( max & bit8[7-ix] )
                    break;
                inc++;
            }
            iy = height;
            y = 0;
            while( iy-- )
            {
                ix = width;
                x = 0;
                while( ix-- )
                {
                    r = buf_fs_bgetc( pic ) << inc;
                    g = buf_fs_bgetc( pic ) << inc;
                    b = buf_fs_bgetc( pic ) << inc;
                    color = paper->function->color( paper, r, g, b, COLOR_ALPHA_000 );
                    paper->function->putpixel( paper, x++, y, color );
                }
                y++;
            }
            break;
        /* no! */
        default:
            return ERROR;
    }
    return OK;
}

/* -------------------------------------------------------- */
static int common( buffer_t *pic )
{
    char    c;

    c = buf_fs_bgetc( pic );
    if( c == EOF )
        return ERROR;
    if( c != '#' )
    {
        buf_fs_bseek( pic, -1, SEEK_CUR );
        return OK;
    }
    do
    {
        if( (c = buf_fs_bgetc( pic )) == EOF )
            return ERROR;
    }
    while( c !=  0x0a );
    return OK;
}
/* -------------------------------------------------------- */
static int whitespace( buffer_t *pic )
{
    char c;

    c = buf_fs_bgetc( pic );
    if( c == EOF )
        return ERROR;
    if( !((c == 0x0d) || (c == 0x09) || (c == 0x0a) || (c == 0x20)) )
    {
        buf_fs_bseek( pic, -1, SEEK_CUR );
        return OK;
    }

    do
    {
        if( (c = buf_fs_bgetc( pic )) == EOF )
            return ERROR;
    }while( (c == 0x0d) || (c == 0x09) || (c == 0x0a) || (c == 0x20) );
    return OK;
}
/* -------------------------------------------------------- */
static int magic( buffer_t *pic, int *type )
{
    char buf[4];

    if( !buf_fs_bgets( pic,buf, 4) )
        return ERROR;

    if( !ys_strcmp("P1\n", buf) )
        return ERROR;               /* *type = PNM_TYPE_P1; */
    else if( !ys_strcmp("P2\n", buf) )
        return ERROR;               /* *type = PNM_TYPE_P2; */
    else if( !ys_strcmp("P3\n", buf) )
        return ERROR;               /* *type = PNM_TYPE_P3; */
    else if( !ys_strcmp("P4\n", buf) )
        *type = PNM_TYPE_P4;
    else if( !ys_strcmp("P5\n", buf) )
        *type = PNM_TYPE_P5;
    else if( !ys_strcmp("P6\n", buf) )
        *type = PNM_TYPE_P6;
    else
        return ERROR;

    return OK;
}
/* -------------------------------------------------------- */
static int width_height_max( buffer_t *pic, int type, int *width, int *height, int *max )
{
    char    buf[32];

    if( whitespace( pic ) == ERROR )
        return ERROR;
    if( common( pic ) == ERROR )
        return ERROR;

    /* width height */
    buf_fs_bgets( pic, buf, 32 );
    if( sscanf( buf, "%i %i", width, height ) != 2 )
        return ERROR;

    if( whitespace( pic ) == ERROR )
        return ERROR;
    if( common( pic ) == ERROR )
        return ERROR;

    /* max */
    if( (type == PNM_TYPE_P1) || (type == PNM_TYPE_P4) )
    {
        *max = 1;
    }
    else
    {
        buf_fs_bgets( pic, buf, 32 );
        if( sscanf( buf, "%i", max ) != 1 )
            return ERROR;
    }

    return OK;
}
