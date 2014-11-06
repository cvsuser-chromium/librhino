/*
    文件:                   png.c
    作者(们):               liupeng
    盟友:
    版本:                   Revision: 0.0.0.1
    建立日期:               2004/4/12
    说明：
        PNG decoding routine

    最后修改日期:           被谁修改:           说明：
*/

#include "gfx_paper.h"
#include "constant.h"
#include "buf_fs.h"
#include "pic.h"

#include "png.h"



#include <png.h>

#ifndef png_jmpbuf
#define png_jmpbuf(png_ptr)((png_ptr)->jmpbuf)
#endif

static void png_read_buffer(png_structp pstruct, png_bytep pointer, png_size_t size)
{
  buf_fs_bread( pstruct->io_ptr, pointer, size );
}


/* ======================================================== */
int picInfoPNG( buffer_t *pic, int *width, int *height, int *mode )
{
  unsigned char hdr[8];
  png_structp state;
  png_infop pnginfo;
  png_uint_32 w, h;
  int bit_depth, colourtype;

  buf_fs_bseek( pic, 0L, SEEK_SET );

  if( buf_fs_bread( pic, hdr, 8 ) != 8) 
    return ERROR;

  if( png_sig_cmp( hdr, 0, 8) ) 
    return ERROR;

  if(!(state = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) 
    goto err;

  if(!(pnginfo = png_create_info_struct(state))) {
    png_destroy_read_struct(&state, NULL, NULL);
    goto err;
  }

  if( setjmp( png_jmpbuf( state ) ) ) {
    png_destroy_read_struct( &state, &pnginfo, NULL);
    return ERROR;
  }

  /* Set up the input function */
  png_set_read_fn( state, pic, png_read_buffer );
  /* png_init_io(state, pic); */

  png_set_sig_bytes( state, 8 );
  png_read_info( state, pnginfo );
  png_get_IHDR(state, pnginfo, &w, &h, &bit_depth, &colourtype, NULL, NULL, NULL);

  *width = w;
  *height = h;
  *mode =  PAPER_MODE_5551;

  png_destroy_read_struct( &state, &pnginfo, NULL );

  return OK;

err:
  return ERROR;
}



int LoadPNG2RealFB(THE_PAPER paper,void *buf,int size,int x_shift,int y_shift)
{
	
	unsigned char hdr[8];
	png_structp state;
	png_infop pnginfo;
	png_uint_32 width, height;
	int bit_depth, colourtype;
	int x, y, pitch;
	char *line, r, g, b;
	ulong color;
	ulong *addr=NULL;
	int index=0;
	//
	buffer_t pic_r;
	buffer_t *pic;
	pic_r.start=buf;
	pic_r.size=size;
	pic_r.offset=0;
	pic=&pic_r;
	
	color = (ulong)COLOR_ALPHA_000 << 24;
		buf_fs_bseek( pic, 0L, SEEK_SET );
	//printf("png,mode is %d\n",paper->mode);
		if( buf_fs_bread( pic, hdr, 8 ) != 8) 
			return ERROR;
	
		if( png_sig_cmp( hdr, 0, 8) ) 
			return ERROR;
		//printf("这是一个png文件\n");
		if(!(state = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) 
			goto err;
	
		if(!(pnginfo = png_create_info_struct(state))) {
			png_destroy_read_struct(&state, NULL, NULL);
			goto err;
		}
	
		if( setjmp( png_jmpbuf( state ) ) ) {
			png_destroy_read_struct( &state, &pnginfo, NULL);
			return ERROR;
		}
	
		/* Set up the input function */
		png_set_read_fn( state, pic, png_read_buffer );
		/* png_init_io(state, pic; */
	
		png_set_sig_bytes( state, 8 );
	
		png_read_info( state, pnginfo );
		png_get_IHDR(state, pnginfo, &width, &height, &bit_depth, &colourtype, NULL, NULL, NULL);
	
	
	
		if( paper->mode != PAPER_MODE_8888 )
			goto err;
		png_set_expand(state);
		if(bit_depth == 16)
			png_set_strip_16(state);
		if(colourtype & PNG_COLOR_MASK_ALPHA)
			png_set_strip_alpha(state);
		if(colourtype == PNG_COLOR_TYPE_GRAY ||
			 colourtype == PNG_COLOR_TYPE_GRAY_ALPHA)
			png_set_gray_to_rgb(state);
	
		pitch = width * 4;
		if( (line = MALLOC( pitch )) == NULL )
			goto err;
	
		for( y = 0; y < height; y++ ){
			
			png_bytep row_pointer = line;
			png_read_row( state, row_pointer, NULL);	
			addr = (ulong *)(paper->framebuffer + paper->yinc * (y+y_shift) + paper->xinc * x_shift);
			for(index = 0, x = 0; x < width; x++ )
			{
				r = line[index];
				index++;
			
				g = line[index];
				index++;
			
				b = line[index];
				index++;
			
				*addr = color | (r << 16) | (g << 8) | b;
				addr++;
			}
		}
	
		png_read_end(state, NULL);
		yxFree( line );
		png_destroy_read_struct(&state, &pnginfo, NULL);
	
		return OK;
	
	err:
		return ERROR;
	
}


/* ======================================================== */
int picLoadPNG( buffer_t *pic, THE_PAPER paper )
{
  unsigned char hdr[8];
  png_structp state;
  png_infop pnginfo;
  png_uint_32 width, height;
  int bit_depth, colourtype;
  int x, y, pitch;
  char *line, r, g, b;
  void *addr;
	int color;
  buf_fs_bseek( pic, 0L, SEEK_SET );
//printf("png,mode is %d\n",paper->mode);
  if( buf_fs_bread( pic, hdr, 8 ) != 8) 
    return ERROR;

  if( png_sig_cmp( hdr, 0, 8) ) 
    return ERROR;

  if(!(state = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) 
    goto err;

  if(!(pnginfo = png_create_info_struct(state))) {
    png_destroy_read_struct(&state, NULL, NULL);
    goto err;
  }

  if( setjmp( png_jmpbuf( state ) ) ) {
    png_destroy_read_struct( &state, &pnginfo, NULL);
    return ERROR;
  }

  /* Set up the input function */
  png_set_read_fn( state, pic, png_read_buffer );
  /* png_init_io(state, pic; */

  png_set_sig_bytes( state, 8 );

  png_read_info( state, pnginfo );
  png_get_IHDR(state, pnginfo, &width, &height, &bit_depth, &colourtype, NULL, NULL, NULL);



  if( (paper->mode != PAPER_MODE_5551 )&&(paper->mode != PAPER_MODE_5650 )&&(paper->mode != PAPER_MODE_0888)&&(paper->mode != PAPER_MODE_8888 ))
    goto err;
  if( paper->w != width )
    goto err;
  if( paper->h != height )
    goto err;

  png_set_expand(state);
  if(bit_depth == 16)
    png_set_strip_16(state);
  if(colourtype & PNG_COLOR_MASK_ALPHA)
    png_set_strip_alpha(state);
  if(colourtype == PNG_COLOR_TYPE_GRAY ||
     colourtype == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(state);


	pitch = width * 4;
	if( (line = MALLOC( pitch )) == NULL )
		goto err;
	png_bytep row_pointer = line;
 	 for( y = 0; y < height; y++ )
	{
   		 png_read_row( state, row_pointer, NULL);

	    for( x = 0; x < width; x++ )
	   {
	      r = line[x *3 + 0];
	      g = line[x *3 + 1];
	      b = line[x *3 + 2];
	    #if 1  
	      color = paper->function->color( paper, r, g, b, COLOR_ALPHA_000 );
	      paper->function->putpixel( paper, x, y, color );
			#else
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
			#endif	  
           }
        }

  png_read_end(state, NULL);
  yxFree( line );
  png_destroy_read_struct(&state, &pnginfo, NULL);

  return OK;

err:
  return ERROR;
}
