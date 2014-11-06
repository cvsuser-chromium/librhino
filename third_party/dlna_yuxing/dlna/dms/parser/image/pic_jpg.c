/*
    文件:                   jpg.c
    作者(们):               liupeng
    盟友:
    版本:                   Revision: 0.0.0.1
    建立日期:               2004/4/12
    说明：
        BMP decoding routine

    最后修改日期:           被谁修改:           说明：
*/
#include <setjmp.h>//add by wjl.

#include "gfx_paper.h"
#include "constant.h"
#include "buf_fs.h"
#include "pic.h"

#include "jpeglib.h"

static buffer_t *inptr;

static void init_source(j_decompress_ptr cinfo);
static void fill_input_buffer(j_decompress_ptr cinfo);
static void skip_input_data(j_decompress_ptr cinfo, long num_bytes);
static boolean resync_to_restart(j_decompress_ptr cinfo, int desired);
static void term_source(j_decompress_ptr cinfo);

//add by wjl.
struct my_error_mgr 
{  
	struct jpeg_error_mgr pub;	/* "public" fields */  
	jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)my_error_exit (j_common_ptr cinfo)
{  
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */  
	my_error_ptr myerr = (my_error_ptr) cinfo->err;  
	/* Always display the message. */  
	/* We could postpone this until after returning, if we chose. */  
	(*cinfo->err->output_message) (cinfo);  
	/* Return control to the setjmp point */  
	longjmp(myerr->setjmp_buffer, 1);
}
//end wjl.

/* ======================================================== */
int picInfoJPG( buffer_t *pic, int *width, int *height, int *mode )
{
    unsigned char magic[8];
    struct jpeg_source_mgr smgr;
    struct jpeg_decompress_struct cinfo;
//    struct jpeg_error_mgr jerr;
    struct my_error_mgr jerr;
    /* first determine if JPEG file since decoder will error if not */
    buf_fs_bseek( pic, 0, SEEK_SET );
    if( buf_fs_bread( pic, magic, 2) != 2 )
        return ERROR;
    if (magic[0] != 0xFF || magic[1] != 0xD8)
        return ERROR;   /* not JPEG image */

    buf_fs_bread(pic, magic, 8);
#if 0
    if( strncmp( magic + 4, "JFIF", 4 ) != 0 )
        return ERROR;   /* not JPEG image */
#endif


    buf_fs_bread(pic, 0, SEEK_SET);

    /* Step 1: allocate and initialize JPEG decompression object */
    /* We set up the normal JPEG error routines. */
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    if (setjmp(jerr.setjmp_buffer)) 
    {    
		/* If we get here, the JPEG code has signaled an error.     
		* We need to clean up the JPEG object, close the input file, and return.     */    
		jpeg_destroy_decompress(&cinfo);        
		return ERROR;  
    }

//printf(" ***********  picInfoJPG  222222 **********\n");
  
    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);
//printf(" ***********  picInfoJPG  333333  **********\n");
    /* Step 2:  Setup the source manager */
    smgr.init_source = (void *) init_source;
    smgr.fill_input_buffer = (void *) fill_input_buffer;
    smgr.skip_input_data = (void *) skip_input_data;
    smgr.resync_to_restart = (void *) resync_to_restart;
    smgr.term_source = (void *) term_source;
    cinfo.src = &smgr;
    inptr = pic;

    /* Step 3: read file parameters with jpeg_read_header() */
    jpeg_read_header(&cinfo, TRUE);

    /* Step 4: set parameters for decompression */
    jpeg_calc_output_dimensions( &cinfo );

    /* 取得图片信息 */
    *width = cinfo.output_width;
    *height = cinfo.output_height;
    *mode = PAPER_MODE_5551;


    /* Step 8: Release JPEG decompression object */
    jpeg_destroy_decompress (&cinfo);

    return OK;
}

/* this interface mainly used for RUI */
static int DecodeJpeg2Paper8888(struct jpeg_decompress_struct  *cinfo, THE_PAPER paper, int x_shift, int y_shift, int alpha)
{
	unsigned char		*line=NULL;	//注意这里要定义为无符号型指针，不然移位时会出问题!!!!
	int 		index=0;
	ulong		r=0, g=0, b=0, color=0;
	ulong		*addr=0;
	int 		x=0, y=0;
	JSAMPROW	buffer[1];
	
	int width = cinfo->output_width;

	if( (paper->framebuffer & 0x3) != 0 )
		return ERROR;
	line = (char *)yxMalloc(cinfo->output_width * cinfo->output_components);
	if( line == NULL )
		return ERROR;
	color = (ulong)COLOR_ALPHA_000 << 24;
	y = 0;
	while( cinfo->output_scanline < cinfo->output_height )
	{
		#if 0
		jpeg_read_scanlines( cinfo, &line, 1 );
		#else
		
		buffer[0] = (JSAMPROW)line;
		jpeg_read_scanlines( cinfo, buffer, 1 );
		addr = (void *)(paper->framebuffer + paper->yinc * (y+y_shift) + paper->xinc * x_shift);
		addr = (ulong *)(paper->framebuffer + paper->yinc * (y+y_shift) + paper->xinc * x_shift);
		#endif
		
		for(index = 0, x = 0; x < width; x++ )
		{
		#if 0	
		      r = line[x *3 + 0];
		      g = line[x *3 + 1];
		      b = line[x *3 + 2];
		      color = paper->function->color( paper, r, g, b, COLOR_ALPHA_000 );
		      paper->function->putpixel( paper, x, y, color );
		#else
			r = line[index];
			index++;

			g = line[index];
			index++;

			b = line[index];
			index++;

			*addr = color | (r << 16) | (g << 8) | b;
			addr++;
		 #endif
		}
		y++;
	}

	yxFree(line);

	return OK;
}

/* this interface commonly used for all purpose, it is very fast and efficint */
static int DecodeJpeg2Paper888(struct jpeg_decompress_struct *cinfo, THE_PAPER paper, int x_shift, int y_shift, int alpha)
{
	char		*addr;
	int 		y=0;
	int cnt=0;
	JSAMPROW	buffer[1];
	PRINT_CUR_FUNC();
	addr =(void * )(paper->framebuffer + paper->yinc * y_shift + paper->xinc * x_shift);
	while( cinfo->output_scanline < cinfo->output_height )
	{
		buffer[0] = (JSAMPROW)addr;
		jpeg_read_scanlines( cinfo, buffer, 1 );	
		addr += paper->yinc;
		y++;
		cnt++;
	}
	return OK;
}

#if 0
int LoadJPG2RealFB(THE_PAPER paper,void *buf,int size,int x_shift,int y_shift)
{
	
	unsigned char magic[8];
	struct jpeg_source_mgr smgr;
	struct jpeg_decompress_struct cinfo;
	//struct jpeg_error_mgr jerr;
	struct my_error_mgr jerr;
	int jpg_width,jpg_height,jpg_mode;
	
	int 	width, height, pitch;
	uchar	*line, r, g, b;
	int 	x, y;
	ulong	color;
	buffer_t pic_r;
	buffer_t *pic;
	pic_r.start=buf;
	pic_r.size=size;
	pic_r.offset=0;
	pic=&pic_r;
	if(picInfoJPG(pic, &jpg_width, &jpg_height, &jpg_mode)!=OK)
		{
			return ERROR;
		}
	/* first determine if JPEG file since decoder will error if not */
	buf_fs_bseek( pic, 0, SEEK_SET );
	if( buf_fs_bread( pic, magic, 2) != 2 )
		return ERROR;
	if (magic[0] != 0xFF || magic[1] != 0xD8)
		return ERROR;	/* not JPEG image */
	buf_fs_bread(pic, magic, 8);
		
	//
	buf_fs_bread(pic, 0, SEEK_SET);
	
	/* Step 1: allocate and initialize JPEG decompression object */
	/* We set up the normal JPEG error routines. */
	cinfo.err = jpeg_std_error(&jerr);
	
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer)) 
	{	 
		/* If we get here, the JPEG code has signaled an error. 	
		* We need to clean up the JPEG object, close the input file, and return.	 */    
		jpeg_destroy_decompress(&cinfo);		
		return ERROR;  
	}
		/* Now we can initialize the JPEG decompression object. */
		jpeg_create_decompress(&cinfo);

		/* Step 2:	Setup the source manager */
		smgr.init_source = (void *) init_source;
		smgr.fill_input_buffer = (void *) fill_input_buffer;
		smgr.skip_input_data = (void *) skip_input_data;
		smgr.resync_to_restart = (void *) resync_to_restart;
		smgr.term_source = (void *) term_source;
		cinfo.src = &smgr;
		inptr = pic;

		/* Step 3: read file parameters with jpeg_read_header() */
		jpeg_read_header(&cinfo, TRUE);
	//printf("1111111111111111\n");
		/* Step 4: set parameters for decompression */

		/* Step 5: Start decompressor */
		jpeg_start_decompress (&cinfo);
	//printf("2222222222222222\n");

		/* 取得图片信息 */
		width = cinfo.output_width;
		height = cinfo.output_height;

		if( paper->mode != PAPER_MODE_8888 )
			goto err;
	 DecodeJpeg2Paper8888(&cinfo, paper,  x_shift,  y_shift, COLOR_ALPHA_000);
	err:
		/* Step 7: Finish decompression */
		jpeg_finish_decompress (&cinfo);
	//printf("44444444444444\n");
		/* Step 8: Release JPEG decompression object */
		jpeg_destroy_decompress (&cinfo);
	
		return OK;
	
}
#endif

static int DecodeJpeg2Paper1555(struct jpeg_decompress_struct *cinfo, THE_PAPER paper, int x_shift, int y_shift, int alpha)
{
	char		*line=NULL;
	int 		index = 0;
	ulong value;
	unsigned short		r=0, g=0, b=0, color=0;
	unsigned short		*addr=0;
	int 		x=0, y=0, width=0;
	JSAMPROW	buffer[1];
	PRINT_CUR_FUNC();
	width = cinfo->output_width;
	line = (char *)yxMalloc(width * cinfo->output_components);
	if( line == NULL )
		return ERROR;

	if( alpha > 0 )
		color = 0x8000;
	else
		color = 0;

	y = 0;
	while( cinfo->output_scanline < cinfo->output_height )
	{
		buffer[0] = (JSAMPROW)line;
		jpeg_read_scanlines( cinfo, buffer, 1 );

		addr = (unsigned short *)(paper->framebuffer + paper->yinc * (y+y_shift) + paper->xinc * x_shift);
		for(index = 0, x = 0; x < width; x++ )
		{
			r = line[index];
			index++;

			g = line[index];
			index++;

			b = line[index];
			index++;
			value=( (((ulong)COLOR_ALPHA_000 << 8) & 0x00008000) |
				 (((ulong)r<< 7) & 0x00007c00) |
				 (((ulong)g << 2) & 0x000003e0) |
				 (((ulong)b>> 3) & 0x0000001f)   );
			*addr =value;
			addr++;
		}
		y++;
	}

	yxFree(line);
	return OK;
}



static int DecodeJpeg2Paper5650(struct jpeg_decompress_struct *cinfo, THE_PAPER paper, int x_shift, int y_shift, int alpha)
{
	char		*line=NULL;
	int 		index = 0;
	unsigned short		r=0, g=0, b=0, color=0;
	unsigned short		*addr=0;
	int 		x=0, y=0, width=0;
	JSAMPROW	buffer[1];
	PRINT_CUR_FUNC();
	width = cinfo->output_width;
	line = (char *)yxMalloc(width * cinfo->output_components);
	if( line == NULL )
		return ERROR;

	if( alpha > 0 )
		color = 0x8000;
	else
		color = 0;

	y = 0;
	
	int m;
	int n;
	m = width%16;
	n = width>>4;
	while( cinfo->output_scanline < cinfo->output_height )
	{
		buffer[0] = (JSAMPROW)line;
		jpeg_read_scanlines( cinfo, buffer, 1 );

		addr = (unsigned short *)(paper->framebuffer + paper->yinc * (y+y_shift) + paper->xinc * x_shift);
		for(index = 0, x = 0; x < width; x++ )
		{
	//		*(addr++)=(  (((ulong)(line[++index])<< 8) & 0x0000f800) |(0x00000020|((ulong) (line[++index])<< 3) & 0x000007e0) | (((ulong)(line[++index])>> 3) & 0x0000001f)   );
			r = line[index++];
			g = line[index++];
			b = line[index++];
			*(addr++)=(  (((ulong)r<< 8) & 0x0000f800) |(0x00000020|((ulong) g<< 3) & 0x000007e0) |(((ulong)b>> 3) & 0x0000001f)   );
		}
		y++;
	}

	yxFree(line);
	return OK;
}

/* ======================================================== */
int picLoadJPG( buffer_t *pic, THE_PAPER paper )
{
    unsigned char magic[8];
    struct jpeg_source_mgr smgr;
    struct jpeg_decompress_struct cinfo;
    //struct jpeg_error_mgr jerr;
    struct my_error_mgr jerr;
    int     width, height;

    /* first determine if JPEG file since decoder will error if not */
    buf_fs_bseek( pic, 0, SEEK_SET );
    if( buf_fs_bread( pic, magic, 2) != 2 )
        return ERROR;
    if (magic[0] != 0xFF || magic[1] != 0xD8)
        return ERROR;   /* not JPEG image */
    buf_fs_bread(pic, magic, 8);
#if 0
    if( strncmp( magic + 4, "JFIF", 4 ) != 0 )
        return ERROR;   /* not JPEG image */
#endif


    buf_fs_bread(pic, 0, SEEK_SET);

    /* Step 1: allocate and initialize JPEG decompression object */
    /* We set up the normal JPEG error routines. */
    cinfo.err = jpeg_std_error(&jerr);

    //wjl
    jerr.pub.error_exit = my_error_exit;
    if (setjmp(jerr.setjmp_buffer)) 
    {    
		/* If we get here, the JPEG code has signaled an error.     
		* We need to clean up the JPEG object, close the input file, and return.     */    
		jpeg_destroy_decompress(&cinfo);        
		return ERROR;  
    }


    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);

    /* Step 2:  Setup the source manager */
    smgr.init_source = (void *) init_source;
    smgr.fill_input_buffer = (void *) fill_input_buffer;
    smgr.skip_input_data = (void *) skip_input_data;
    smgr.resync_to_restart = (void *) resync_to_restart;
    smgr.term_source = (void *) term_source;
    cinfo.src = &smgr;
    inptr = pic;

    /* Step 3: read file parameters with jpeg_read_header() */
    jpeg_read_header(&cinfo, TRUE);
    /* Step 4: set parameters for decompression */

    /* Step 5: Start decompressor */
    jpeg_start_decompress (&cinfo);

    /* 取得图片信息 */
    width = cinfo.output_width;
    height = cinfo.output_height;
 switch(paper->mode)
 	{
 	case PAPER_MODE_5551:
		DecodeJpeg2Paper1555(&cinfo, paper,  0, 0, COLOR_ALPHA_000);
		break;
	case PAPER_MODE_8888:
		DecodeJpeg2Paper8888(&cinfo, paper,  0, 0, COLOR_ALPHA_000);
		break;
	case PAPER_MODE_0888:
		PRINT_CUR_FUNC();
		DecodeJpeg2Paper888(&cinfo, paper,  0,0, COLOR_ALPHA_000);
		break;
	case PAPER_MODE_5650:
		DecodeJpeg2Paper5650(&cinfo, paper,  0, 0, COLOR_ALPHA_000);
		break;	
		
	default:
		goto err;
		break;
 	}
err:
    /* Step 7: Finish decompression */
    jpeg_finish_decompress (&cinfo);
    /* Step 8: Release JPEG decompression object */
    jpeg_destroy_decompress (&cinfo);

#if 0
{
    FILE *fp;
    fp = fopen("./test_bmp9.bin","w+");
    if (fp == NULL)
		printf("open file error!\n");
    fwrite(paper->framebuffer,paper->size,1,fp);
    fclose(fp);
}
#endif

    return OK;
}

/* -------------------------------------------------------- */
static void init_source(j_decompress_ptr cinfo)
{
    cinfo->src->next_input_byte = inptr->start;
    cinfo->src->bytes_in_buffer = inptr->size;
}

/* -------------------------------------------------------- */
static void fill_input_buffer(j_decompress_ptr cinfo)
{
    return;
}

/* -------------------------------------------------------- */
static void skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
    if ((unsigned long)num_bytes >= inptr->size)
        return;
    cinfo->src->next_input_byte += num_bytes;
    cinfo->src->bytes_in_buffer -= num_bytes;
}

/* -------------------------------------------------------- */
static boolean resync_to_restart(j_decompress_ptr cinfo, int desired)
{
    return jpeg_resync_to_restart(cinfo, desired);
}

/* -------------------------------------------------------- */
static void term_source(j_decompress_ptr cinfo)
{
    return;
}
