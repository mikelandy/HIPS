/*	jpeg_compress-CCS interface
#
%	JPEG_W . C
%
*/

#include "cdjpeg.h"

LOCAL int	input_row_pos;

#ifdef	STREAM_IMAGE_LIB

LOCAL	int	stream_is_linked;
void
config_stream_link(int yn)
{	stream_is_linked = yn;	}

#endif

re_init_scan(j_compress_ptr cinfo)
{	input_row_pos = cinfo->img->tmp_offset = 0;
	if (cinfo->next_scanline)
		re_init_scan_more(cinfo);
}

get_input_row(j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
register U_IMAGE*	img = cinfo->img;
register int	col = img->width;
register JSAMPROW	pr = sinfo->buffer[0];
register byte	*p = (byte*) img->src + input_row_pos * img->dpy_channels * col;
	switch(img->color_form)	{
	case CFM_SCF:
		while (col--)	{
		register int	c = *p++;
			*pr++ = reg_cmap[0][c];
			*pr++ = reg_cmap[1][c];
			*pr++ = reg_cmap[2][c];
		}
		break;
	case CFM_SEPLANE:
		mesg("warning -- not ready\n");
	case CFM_ILC:
	case CFM_SGF:
		sinfo->buffer[0] = p;
		break;
	case CFM_ILL:
		line_to_cell_color(pr, p, col, img->height);
		break;
	default:	prgmerr('f', "strange color form %d", img->color_form);
	}
	input_row_pos++;
return	1;
}

void
input_init(j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
register U_IMAGE*	img = cinfo->img;
register int	fmt;
#ifdef	STREAM_IMAGE_LIB
	if (!stream_is_linked)
#endif
	(*img->header_handle)(HEADER_READ, img, 0, 0, Yes /* OsameI */);
	if (img->in_color == CFM_SCF && img->dpy_channels == 3)
		img->dpy_channels = img->channels,
		img->color_form = img->in_color;
	cinfo->image_width = (JDIMENSION) img->width;
	cinfo->image_height = (JDIMENSION) img->height;
/*
	cinfo->data_precision = 8;
*/
	sinfo->get_pixel_rows = get_input_row;
	switch(fmt = img->color_form)	{
	case CFM_SGF:
		 cinfo->in_color_space = JCS_GRAYSCALE;
		cinfo->input_components = img->dpy_channels;
		break;
	case CFM_ILL:
	default:
		cinfo->in_color_space = JCS_RGB;
		cinfo->input_components = 3;
	}
	sinfo->buffer_height = 1;
/*
	sinfo->buffer = (fmt != CFM_ILL && fmt != CFM_SCF) ?	(void*)
		(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
			cinfo->input_components * SIZEOF(JSAMPROW))	:
	(void*)	(*cinfo->mem->alloc_sarray)((j_common_ptr) cinfo, JPOOL_IMAGE,
			cinfo->image_width, 3);
*/
	sinfo->buffer = NZALLOC(cinfo->input_components, SIZEOF(JSAMPROW),
		"sbuf");
	if (fmt == CFM_ILL || fmt == CFM_SCF)
		sinfo->buffer[0] = NZALLOC(cinfo->image_width, 3, "sbuf0");
}

LOCAL void
input_term(j_compress_ptr cinfo, cjpeg_source_ptr term)
{
	if (term)	{
		free(cinfo->img->src);
		free(term->buffer);
		free(term);
	} else	input_row_pos = 0;
}



cjpeg_source_ptr
select_file_type (j_compress_ptr cinfo, FILE * infile, int is_targa)
{
int	c;
cjpeg_source_ptr	src;
	if (is_targa) {
#ifdef TARGA_SUPPORTED
		src = jinit_read_targa(cinfo);
#else
		ERREXIT(cinfo, JERR_TGA_NOTCOMP);
#endif
	} else	{
#ifdef	BMP_SUPPORTED
		c = getc(infile);
		ungetc(c, infile);
	    if (c == 'B')
		src = jinit_read_bmp(cinfo);
	    else
#endif
		src = ZALLOC(SIZEOF(*src), 1, "src_mgr"),
		src->start_input = input_init,
		src->finish_input = input_term;
	}
	src->input_file = infile;
return	src;
}

/*
jpeg_compress (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
int     i = cinfo->img->frames;
	do {
		if ((*cinfo->img->std_swif)(FI_LOAD_FILE, cinfo->img, 0, 0) < 0)
			break;
		while (cinfo->next_scanline < cinfo->image_height)
			jpeg_write_scanlines(cinfo, sinfo->buffer,
				(*sinfo->get_pixel_rows) (cinfo, sinfo));
		fprintf(stderr, "%d frames left (%u)\n",
			i, cinfo->img->tmp_offset);
	} while (--i > 0);
}
*/

write_next_jpeg_frame(j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
JDIMENSION	num_scanlines;
	/* Process data */
	while (cinfo->next_scanline < cinfo->image_height) {
		num_scanlines = (*sinfo->get_pixel_rows) (cinfo, sinfo);
		jpeg_write_scanlines(cinfo, sinfo->buffer, num_scanlines);
	}
#ifdef	STREAM_IMAGE_LIB
	cinfo->img->tmp_offset = stream_jpeg_dst_size(cinfo);
#elif	!defined	SOI_SOF_SOS_EOI
	cinfo->img->tmp_offset += 4096 - cinfo->dest->free_in_buffer;
#endif
}

init_jpeg_header(j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
#ifdef	STREAM_IMAGE_LIB
	/* Read the input file header to initialize file size & colorspace. */
	(*sinfo->start_input)(cinfo, sinfo);
	/* after V6, this must be executed after start_input()	*/
	jpeg_default_colorspace(cinfo);	/* really needed here ?	*/
#endif
	if (!cinfo->img->tmp_offset)
		jpeg_set_defaults(cinfo);
	cinfo->img->tmp_offset = 0;	/* remove progressive flag */
	/* Specify data destination for compression */
#ifdef	STREAM_IMAGE_LIB
	stream_stdio_dest(cinfo, cinfo->img->OUT_FP);
#else
	jpeg_stdio_dest(cinfo, cinfo->img->OUT_FP);
#endif
#ifndef	SOI_SOF_SOS_EOI
	/* write header once; (TRUE for each frame)	*/
	jpeg_start_compress(cinfo, FALSE);
#endif
}

close_jpeg_write(j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
/* Finish compression and release memory */
	(*sinfo->finish_input) (cinfo, sinfo);
#ifndef	SOI_SOF_SOS_EOI
	jpeg_finish_compress(cinfo);
#endif
	jpeg_destroy_compress(cinfo);
}

