/*
    文件:                   gfx_picture.c
    作者(们):               liupeng
    盟友:
    版本:                   Revision: 0.0.0.1
    建立日期:               2004/7/14
    最后修改日期:           被谁修改:           说明：
*/
#include <stdlib.h>
#include "gfx_paper.h"
#include "buf_fs.h"
#include "pic.h"


/* ======================================================== */
int infopic( uchar *pic, ulong size, int *width, int *height, int *mode )
{
    buffer_t    buf;

	if( pic && (size>0) && width && height &&  mode)
	{
		*width = 0;
		*height = 0;
		*mode = PAPER_MODE_NULL;
		
		buf_fs_binit( &buf, (void*)pic, (int)size );
		if( picInfoPNM( &buf, width, height, mode ) != OK )
		if( picInfoBMP( &buf, width, height, mode ) != OK )
		if( picInfoJPG( &buf, width, height, mode ) != OK )
		if( picInfoPNG( &buf, width, height, mode ) != OK )
		    return ERROR;

		if( *width<= 0 || *height<= 0 || *mode == PAPER_MODE_NULL)
			return ERROR;

		return OK;
	}
	return ERROR;
}

THE_PAPER loadpic_paper( uchar *pic, ulong size, THE_PAPER paper )
{
    buffer_t    buf;

    buf_fs_binit( &buf, pic, size );
    if( picLoadPNM( &buf, paper ) != OK )
    if( picLoadBMP( &buf, paper ) != OK )
    if( picLoadJPG( &buf, paper ) != OK )
    if( picLoadPNG( &buf, paper ) != OK )
    {
        return NULL;
    }

    return paper;
}

THE_PAPER loadpic( uchar *pic, ulong size )
{
    int         	width, height, mode;
    THE_PAPER   paper, pp;

    /* 1 获得信息 */
	if( infopic( pic, size, &width, &height, &mode ) != OK )
		return NULL;

    /* 2 制作paper */
    if( (paper = makePaper( width, height )) == NULL )
		return NULL;

    /* 3 load pic */
	pp = loadpic_paper(pic, size, paper);
    if( pp == NULL  )
    {
        burnPaper( paper );
        return NULL;
    }
    /* 4 返回拒柄 */
    return paper;
}

/* ======================================================== */
THE_PAPER loadpicWithPreMalloc( uchar *pic, ulong size,void *preMallocBuf,ulong bufSize )
{
    int         	width, height, mode;
    THE_PAPER   paper, pp;

    /* 1 获得信息 */
	if( infopic( pic, size, &width, &height, &mode ) != OK )
		return NULL;

    /* 2 制作paper */
	paper=makePaperWithPreFB( width,height,preMallocBuf, bufSize);
	if( paper == NULL )
		return NULL;

    /* 3 load pic */
	pp = loadpic_paper(pic, size, paper);
    if( pp == NULL  )
    {
        burnPaper( paper );
        return NULL;
    }
    /* 4 返回拒柄 */
    return paper;
}

#if 0
/* ======================================================== */
THE_PAPER loadpic_0888( uchar *pic, ulong size )
{
	THE_PAPER	paper;
	int 		width, height, mode;
	buffer_t	buf;

	/* 1 获得信息 */
	buf_fs_binit( &buf, pic, size );
	if( picInfoPNM( &buf, &width, &height, &mode ) != OK )
	if( picInfoBMP( &buf, &width, &height, &mode ) != OK )
	if( picInfoJPG( &buf, &width, &height, &mode ) != OK )
	if( picInfoPNG( &buf, &width, &height, &mode ) != OK )
		return NULL;

	/* 2 制作paper */
	if( (paper = makePaper( PAPER_MODE_0888, width, height )) == NULL )
		return NULL;
		

	/* 3 load pic */
	buf_fs_binit( &buf, pic, size );
	if( picLoadPNM( &buf, paper ) != OK )
	if( picLoadBMP( &buf, paper ) != OK )
	if( picLoadJPG( &buf, paper ) != OK )
	if( picLoadPNG( &buf, paper ) != OK )
	{
		burnPaper( paper );
		return NULL;
	}

	/* 4 返回拒柄 */
	return paper;
}



THE_PAPER loadpic_alpha( uchar *pic, ulong size,int alpha )
{
		THE_PAPER 	paper,tmp_paper;
		int 				width, height, mode;
		buffer_t		buf;	

		/* 1 获得信息 */
		buf_fs_binit( &buf, pic, size );
		if( picInfoPNM( &buf, &width, &height, &mode ) != OK )
		if( picInfoBMP( &buf, &width, &height, &mode ) != OK )
		if( picInfoJPG( &buf, &width, &height, &mode ) != OK )
		if( picInfoPNG( &buf, &width, &height, &mode ) != OK )
				return NULL;

		/* 2 制作paper */
		if( (tmp_paper = makePaper( SELECT_MODE, width, height )) == NULL )
		return NULL;
		if( (paper = makePaper( SELECT_MODE, width, height )) == NULL )
		return NULL;
		
		bar( paper, 0,0, width, height,0);
		/* 3 load pic */
		buf_fs_binit( &buf, pic, size );
		if( picLoadPNM( &buf, tmp_paper ) != OK )
		if( picLoadBMP( &buf, tmp_paper ) != OK )
		if( picLoadJPG( &buf, tmp_paper ) != OK )
		if( picLoadPNG( &buf, tmp_paper ) != OK )
		{
				burnPaper( tmp_paper );
				return NULL;
		}
		bitblt(tmp_paper, 0,0,  paper, 0,0,paper->w,paper->h, alpha);
		burnPaper( tmp_paper );
		/* 4 返回拒柄 */
		return paper;
}

/* ======================================================== */
#endif

