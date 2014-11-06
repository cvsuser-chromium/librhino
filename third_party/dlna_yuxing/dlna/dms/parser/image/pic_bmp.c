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


/* BMP stuff*/
#define BI_RGB          0L
#define BI_RLE8         1L
#define BI_RLE4         2L
#define BI_BITFIELDS    3L

typedef struct          __attribute__((__packed__))
{   /* BITMAPFILEHEADER*/
    uchar   bfType[2];
    ulong   bfSize;
    ushort  bfReserved1;
    ushort  bfReserved2;
    ulong   bfOffBits;
    ulong   BiBcSize;
}BMPFILEHEAD;

/* windows style*/
typedef struct          __attribute__((__packed__))
{   /* BITMAPINFOHEADER*/
    ulong   BiWidth;
    ulong   BiHeight;
    ushort  BiPlanes;
    ushort  BiBitCount;
    ulong   BiCompression;
    ulong   BiSizeImage;
    ulong   BiXpelsPerMeter;
    ulong   BiYpelsPerMeter;
    ulong   BiClrUsed;
    ulong   BiClrImportant;
}BMPINFOHEAD;

/* os/2 style*/
typedef struct         __attribute__((__packed__))
{   /* BITMAPCOREHEADER*/
    ushort  bcWidth;
    ushort  bcHeight;
    ushort  bcPlanes;
    ushort  bcBitCount;
}BMPCOREHEAD;

#define FILEHEADSIZE    sizeof( BMPFILEHEAD )
#define INFOHEADSIZE    sizeof( BMPINFOHEAD )
#define COREHEADSIZE    sizeof( BMPCOREHEAD )

static int  DecodeRLE8( buffer_t *pic, uchar *buf );
static int  DecodeRLE4( buffer_t *pic, uchar *buf );



/* ======================================================== */
int picInfoBMP( buffer_t *pic, int *width, int *height, int *mode )
{
    BMPFILEHEAD bmpf;
    BMPINFOHEAD bmpi;
    BMPCOREHEAD bmpc;
    int         bits;
//printf("**picInfoBMP**\n");
    buf_fs_bseek( pic, 0, SEEK_SET );

    /* file head */
    if( buf_fs_bread( pic, &bmpf, FILEHEADSIZE ) != FILEHEADSIZE )
        return ERROR;

    if( (bmpf.bfType[0] != 'B') || (bmpf.bfType[1] != 'M') )
        return ERROR;

    bmpf.BiBcSize -= 4;
    if( bmpf.BiBcSize == INFOHEADSIZE )
    {
        if( buf_fs_bread( pic, &bmpi, INFOHEADSIZE ) != INFOHEADSIZE )
            return ERROR;

        *width = bmpi.BiWidth;
        *height = bmpi.BiHeight;
        bits = bmpi.BiBitCount;
    }
    else if( bmpf.BiBcSize == COREHEADSIZE )
    {
        if( buf_fs_bread( pic, &bmpc, COREHEADSIZE ) != COREHEADSIZE )
            return ERROR;

        *width = bmpc.bcWidth;
        *height = bmpc.bcHeight;
        bits = bmpc.bcBitCount;
    }
    else
    {
        return ERROR;
    }

    switch( bits )
    {
        case 24:
            *mode = PAPER_MODE_5551;
            break;
        case 8:
        case 4:
        case 1:
            *mode = PAPER_MODE_CLUT8;
            break;
        default:
            return ERROR;
    }
    return OK;
}

int LoadBMP2RealFB(THE_PAPER paper,void *buf,int size,int x_shift,int y_shift)
{
	
	BMPFILEHEAD bmpf;
	BMPINFOHEAD bmpi;
	BMPCOREHEAD bmpc;
	
	int 		width, height, compression, pitch, bits;
	THE_PALETTE	clut;
	ulong		color;
	
	uchar		r, g, b, a, c;
	int 		i, n, x, y;
	
	uchar		*line;

	//
	buffer_t pic_r;
	buffer_t *pic;
	pic_r.start=buf;
	pic_r.size=size;
	pic_r.offset=0;
	pic=&pic_r;
	
	buf_fs_bseek( pic, 0, SEEK_SET );
	
	/* file head */
	if( buf_fs_bread( pic, &bmpf, FILEHEADSIZE ) != FILEHEADSIZE )
		return ERROR;
	
	if( (bmpf.bfType[0] != 'B') || (bmpf.bfType[1] != 'M') )
		return ERROR;
	
	bmpf.BiBcSize -= 4;
	if( bmpf.BiBcSize == INFOHEADSIZE )
	{
		if( buf_fs_bread( pic, &bmpi, INFOHEADSIZE ) != INFOHEADSIZE )
			return ERROR;
	
		width = bmpi.BiWidth;
		height = bmpi.BiHeight;
		bits = bmpi.BiBitCount;
		compression = bmpi.BiCompression;
	}
	else if( bmpf.BiBcSize == COREHEADSIZE )
	{
		if( buf_fs_bread( pic, &bmpc, COREHEADSIZE ) != COREHEADSIZE )
			return ERROR;
	
		width = bmpc.bcWidth;
		height = bmpc.bcHeight;
		bits = bmpc.bcBitCount;
		compression = BI_RGB;
	}
	else
	{
		return ERROR;
	}
	if((bits!=24)||(paper->mode!=PAPER_MODE_8888))
		return ERROR;
	
	clut = (THE_PAPER)paper->clut;
	switch( bits )
	{
		case 24:
			pitch = width * 3;
			break;
	
		case 8:
		case 4:
		case 1:
			n = 1 << bits;
			if( bmpf.BiBcSize == INFOHEADSIZE )
		{
				for( i = 0; i < n; i ++ )
				{
					b = buf_fs_bgetc( pic );
					g = buf_fs_bgetc( pic );
					r = buf_fs_bgetc( pic );
					a = buf_fs_bgetc( pic );
					clut->colors[i] = paper->function->color( paper, r, g, b, COLOR_ALPHA_000 );
				}
		}
		else
			{
				for( i = 0; i < n; i ++ )
				{
					b = buf_fs_bgetc( pic );
					g = buf_fs_bgetc( pic );
					r = buf_fs_bgetc( pic );
					clut->colors[i] = paper->function->color( paper, r, g, b, COLOR_ALPHA_000 );
				}
		}
			break;
	
		default:
			return ERROR;
	}
	
	/* line size pitch */
	switch( bits )
	{
		case 24:
			pitch = width * 3;
			break;
		case 8:
			pitch = width;
			break;
		case 4:
			pitch = (width + 1) >> 1;
			break;
		case 1:
			pitch = (width + 7) >> 3;
		break;
		default:
			return ERROR;
	}
	pitch = (pitch + 3) & ~3;
	
	/* image data */
	/* line buffer */
	if( (line = MALLOC( pitch )) == NULL )
		return ERROR;
	
	buf_fs_bseek( pic, bmpf.bfOffBits, SEEK_SET );
	y = height - 1;
	while( height -- )
	{
		switch( compression )
		{
			case BI_RGB:
				buf_fs_bread( pic, line, pitch );
				break;
	
			case BI_RLE8:
				DecodeRLE8( pic, line );
				break;
	
			case BI_RLE4:
				DecodeRLE4( pic, line );
				break;
		}
		switch( bits )
		{
			case 24:
				for( x = 0; x < width; x++ )
				{
					b = line[x *3 + 0];
					g = line[x *3 + 1];
					r = line[x *3 + 2];
					color = paper->function->color( paper, r, g, b, COLOR_ALPHA_000 );
					paper->function->putpixel( paper, x+x_shift, y+y_shift, color );
				}
				break;
	
			case 8:
				for( x = 0; x < width; x++ )
				{
					c = line[x];
					paper->function->putpixel( paper, x+x_shift, y+y_shift, c );
				}
				break;
	
			case 4:
				for( x = 0; x < width; x++ )
				{
					if( !(x % 0x1) )
					{
						c = line[x >> 1];
					}
					paper->function->putpixel( paper, x+x_shift, y+y_shift, c&0xf );
					c = c >> 4;
				}
				break;
	
			case 1:
				for( x = 0; x < width; x++ )
				{
					if( !(x % 0x7) )
					{
						c = line[x >> 3];
					}
					paper->function->putpixel( paper, x+x_shift, y+y_shift, c&0x1 );
					c = c >> 1;
				}
				break;
		}
		y --;
	}
	
	/* free line buffer */
	FREE( line );
	
	return OK;
	
	#if 0
	//printf("1\n");	

	/* Step 6: while (scan lines remain to be read) */
	y = 0;
	while( cinfo.output_scanline < cinfo.output_height )
	{
		JSAMPROW buffer[1];
		buffer[0] = (JSAMPROW)line;
	jpeg_read_scanlines( &cinfo, buffer, 1 );
//printf("333333333333333333333333\n");
		for( x = 0; x < width; x++ )
		{
			r = line[x *3 + 0];
			g = line[x *3 + 1];
			b = line[x *3 + 2];
	#ifdef _OPTIMIZE_	
	color= ((ulong)COLOR_ALPHA_000 << 24) | ((ulong)r << 16) | ((ulong)g << 8) | (ulong)b;
	 *((ulong*)(paper->framebuffer + paper->xinc * (x+x_shift)+ paper->yinc * (y+y_shift)))=color;
	#else
			color = paper->function->color( paper, r, g, b, COLOR_ALPHA_000 );
			paper->function->putpixel( paper, x, y, color );
	#endif
		}
		y++;
	}

	#endif	
	return OK;
	
}


/* ======================================================== */
int picLoadBMP( buffer_t *pic, THE_PAPER paper )
{
    BMPFILEHEAD bmpf;
    BMPINFOHEAD bmpi;
    BMPCOREHEAD bmpc;

    int         width, height, compression, pitch, bits;
    THE_PALETTE    clut;
    uchar       r, g, b, a, c;
    int         i, n, x, y;
    uchar       *line;
    void *addr;
   // printf("mode is %d\n",paper->mode);

    buf_fs_bseek( pic, 0, SEEK_SET );

    /* file head */
    if( buf_fs_bread( pic, &bmpf, FILEHEADSIZE ) != FILEHEADSIZE )
        return ERROR;

    if( (bmpf.bfType[0] != 'B') || (bmpf.bfType[1] != 'M') )
        return ERROR;

    bmpf.BiBcSize -= 4;
    if( bmpf.BiBcSize == INFOHEADSIZE )
    {
        if( buf_fs_bread( pic, &bmpi, INFOHEADSIZE ) != INFOHEADSIZE )
            return ERROR;

        width = bmpi.BiWidth;
        height = bmpi.BiHeight;
        bits = bmpi.BiBitCount;
        compression = bmpi.BiCompression;
    }
    else if( bmpf.BiBcSize == COREHEADSIZE )
    {
        if( buf_fs_bread( pic, &bmpc, COREHEADSIZE ) != COREHEADSIZE )
            return ERROR;

        width = bmpc.bcWidth;
        height = bmpc.bcHeight;
        bits = bmpc.bcBitCount;
        compression = BI_RGB;
    }
    else
    {
        return ERROR;
    }
//printf("Here------->0    bits = %d\n",bits);

    /* paper 有效性判断 */
    switch( bits )
    {
        case 24:
            if( (paper->mode == PAPER_MODE_5551 )||(paper->mode == PAPER_MODE_5650)||(paper->mode == PAPER_MODE_0888 )||(paper->mode == PAPER_MODE_8888 ))
                break;
        case 8:
        case 4:
        case 1:
            if( paper->mode == PAPER_MODE_CLUT8 )
                break;
        default:
            return ERROR;
    }
//printf("Here------->1 \n");

    if( paper->w != width )
        return ERROR;
    if( paper->h != height )
        return ERROR;

    /* palette pitch*/
    clut = (THE_PAPER)paper->clut;
    switch( bits )
    {
        case 24:
            pitch = width * 3;
            break;

        case 8:
        case 4:
        case 1:
            n = 1 << bits;
            if( bmpf.BiBcSize == INFOHEADSIZE )
        {
                for( i = 0; i < n; i ++ )
                {
                    b = buf_fs_bgetc( pic );
                    g = buf_fs_bgetc( pic );
                    r = buf_fs_bgetc( pic );
                    a = buf_fs_bgetc( pic );
                    clut->colors[i] = paper->function->color( paper, r, g, b, COLOR_ALPHA_000 );
                }
        }
        else
            {
                for( i = 0; i < n; i ++ )
                {
                    b = buf_fs_bgetc( pic );
                    g = buf_fs_bgetc( pic );
                    r = buf_fs_bgetc( pic );
                    clut->colors[i] = paper->function->color( paper, r, g, b, COLOR_ALPHA_000 );
                }
        }
            break;

        default:
            return ERROR;
    }
//printf("Here------->2\n");

    /* line size pitch */
    switch( bits )
    {
        case 24:
            pitch = width * 3;
            break;
        case 8:
            pitch = width;
            break;
        case 4:
            pitch = (width + 1) >> 1;
            break;
        case 1:
            pitch = (width + 7) >> 3;
        break;
        default:
            return ERROR;
    }
    pitch = (pitch + 3) & ~3;

    /* image data */
    /* line buffer */
    if( (line = MALLOC( pitch )) == NULL )
        return ERROR;
	

    buf_fs_bseek( pic, bmpf.bfOffBits, SEEK_SET );
    y = height - 1;
    while( height -- )
    {
        switch( compression )
        {
            case BI_RGB:
                buf_fs_bread( pic, line, pitch );
                break;

            case BI_RLE8:
                DecodeRLE8( pic, line );
                break;

            case BI_RLE4:
                DecodeRLE4( pic, line );
                break;
        }
        switch( bits )
        {
            case 24:
		
		
                for( x = 0; x < width; x++ )
                {
                    b = line[x *3 + 0];
                    g = line[x *3 + 1];
                    r = line[x *3 + 2];	
                    //color = paper->function->color( paper, r, g, b, COLOR_ALPHA_000 );
                   // paper->function->putpixel( paper, x, y, color );
		addr = (void*)(paper->framebuffer + paper->yinc * y + paper->xinc * x);
                   
			switch(paper->mode)
				{
					case PAPER_MODE_0888:
						*((char *)addr) = r;
						*((char *)addr + 1) = g;
						*((char *)addr + 2) = b;
						break;
					case PAPER_MODE_5551:
						*((unsigned short *)addr)=( (((ulong)COLOR_ALPHA_000 << 8) & 0x00008000) | (((ulong)r<< 7) & 0x00007c00) |
						 (((ulong)g << 2) & 0x000003e0) | (((ulong)b>> 3) & 0x0000001f)   );
						break;
					case PAPER_MODE_5650:
						*((unsigned short *)addr)=(  (((ulong)r<< 8) & 0x0000f800) |(0x00000020|((ulong) g<< 3) & 0x000007e0) |(((ulong)b>> 3) & 0x0000001f)   );
						break;
					case PAPER_MODE_8888:
						*((unsigned int *)addr)=((ulong)COLOR_ALPHA_000 << 24)|(r << 16) | (g << 8) | b;
						break;
				}
					
                }
                break;

            case 8:
                for( x = 0; x < width; x++ )
                {
                    c = line[x];
                    paper->function->putpixel( paper, x, y, c );
                }
                break;

            case 4:
                for( x = 0; x < width; x++ )
                {
                    if( !(x % 0x1) )
                    {
                        c = line[x >> 1];
                    }
                    paper->function->putpixel( paper, x, y, c&0xf );
                    c = c >> 4;
                }
                break;

            case 1:
                for( x = 0; x < width; x++ )
                {
                    if( !(x % 0x7) )
                    {
                        c = line[x >> 3];
                    }
                    paper->function->putpixel( paper, x, y, c&0x1 );
                    c = c >> 1;
                }
                break;
        }
        y --;
    }
//printf("Here-------> 3\n");

    /* free line buffer */
    FREE( line );

    return OK;
}

/* -------------------------------------------------------- */
static int DecodeRLE8( buffer_t *pic, uchar *buf )
{
    int     c, n;

    while( 1 )
    {
        switch( n = buf_fs_bgetc( pic ) )
        {
            case EOF:
                return OK;

            case 0:           /* 0 = escape*/
                switch( n = buf_fs_bgetc( pic ))
                {
                    case 0:     /* 0 0 = end of current scan line*/
                    case 1:     /* 0 1 = end of data*/
                        return 1;
                    case 2:     /* 0 2 xx yy delta mode NOT SUPPORTED*/
                        buf_fs_bgetc( pic );
                        buf_fs_bgetc( pic );
                        continue;

                    default:    /* 0 3..255 xx nn uncompressed data*/
                        for( c = 0; c < n; c++ )
                            *buf++ = buf_fs_bgetc( pic );
                        if( n & 1)
                            buf_fs_bgetc( pic );
                        continue;
                }
            default:
                c = buf_fs_bgetc( pic );
                while( n-- )
                    *buf++ = c;
                continue;
        }
    }
}

/* -------------------------------------------------------- */
static uchar *p;
static int  once;

static void
put4(int b)
{
    static int  last;

    last = (last << 4) | b;
    if( ++once == 2)
    {
        *p++ = last;
        once = 0;
    }
}
static int DecodeRLE4( buffer_t *pic, uchar *buf )
{
    int     c, n, c1, c2;

    p = buf;
    once = 0;
    c1 = 0;

    for( ;;)
    {
        switch( n = buf_fs_bgetc(pic))
        {
            case EOF:
                return OK;
            case 0:           /* 0 = escape*/
                switch( n = buf_fs_bgetc(pic))
                {
                    case 0:     /* 0 0 = end of current scan line*/
                        if( once )
                            put4( 0);
                        return 1;
                    case 1:     /* 0 1 = end of data*/
                        if( once )
                            put4( 0 );
                        return 1;

                    case 2:     /* 0 2 xx yy delta mode NOT SUPPORTED*/
                        buf_fs_bgetc( pic );
                        buf_fs_bgetc( pic );
                        continue;

                    default:    /* 0 3..255 xx nn uncompressed data*/
                        c2 = (n+3) & ~3;
                        for( c=0; c<c2; c++)
                        {
                            if( (c & 1) == 0)
                                c1 = buf_fs_bgetc(pic);
                            if( c < n)
                                put4( (c1 >> 4) & 0x0f);
                            c1 <<= 4;
                        }
                        continue;
                }
                default:
                    c = buf_fs_bgetc(pic);
                    c1 = (c >> 4) & 0x0f;
                    c2 = c & 0x0f;
                    for( c=0; c<n; c++)
                        put4( (c&1)? c2: c1);
                    continue;
        }
    }
}

