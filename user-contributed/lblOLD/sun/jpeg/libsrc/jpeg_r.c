/* JPEG_R . C
#
% decompress entries. GIF needs too many space, and we don't want it here.
*/

#include "cdjpeg.h"
#include "jpegint.h"	/* jpeg_init_decompress + DSTATE_START	*/


djpeg_dest_ptr	dest_mgr, jinit_write_default();
#ifndef DEFAULT_FMT	/* djpeg.c has its own	*/
#define DEFAULT_FMT	FMT_DEFAULT
#endif
IMAGE_FORMATS requested_fmt = DEFAULT_FMT;
struct jpeg_decompress_struct	dinfo;
extern struct jpeg_error_mgr	jerr;

jpeg_init_decompress(j_decompress_ptr cinfo)
{
static	int	inited;
	if (!inited++)	{
/* Initialize the JPEG decompression object with default error handling. */
		cinfo->err = jpeg_std_error(&jerr);
		jpeg_create_decompress(cinfo);
	} else	cinfo->global_state = DSTATE_START;
return	inited;
}

  /* Initialize the output module now to let it override any crucial
   * option settings (for instance, GIF wants to force color quantization).
   */
djpeg_dest_ptr
setup_output_mgr(IMAGE_FORMATS requested_fmt)
{
djpeg_dest_ptr	dst_mgr;
  switch (requested_fmt) {
#ifdef BMP_SUPPORTED
  case FMT_BMP:
    dst_mgr = jinit_write_bmp(&dinfo, FALSE);
    break;
  case FMT_OS2:
    dst_mgr = jinit_write_bmp(&dinfo, TRUE);
    break;
#endif
#ifdef GIF_SUPPORTED
  case FMT_GIF:
    dst_mgr = jinit_write_gif(&dinfo);
    break;
#endif
#ifdef PPM_SUPPORTED
  case FMT_PPM:
    dst_mgr = jinit_write_ppm(&dinfo);
    break;
#endif
#ifdef RLE_SUPPORTED
  case FMT_RLE:
    dst_mgr = jinit_write_rle(&dinfo);
    break;
#endif
#ifdef TARGA_SUPPORTED
  case FMT_TARGA:
    dst_mgr = jinit_write_targa(&dinfo);
    break;
#endif
  default:
    dst_mgr = jinit_write_default(&dinfo);
    break;
  }
  dst_mgr->output_file = dinfo.img->OUT_FP;

return	dst_mgr;
}


/*
 * Signal catcher to ensure that temporary files are removed before aborting.
 * NB: for Amiga Manx C this is actually a global routine named _abort();
 * we put "#define signal_catcher _abort" in jconfig.h.  Talk about bogus...
 */

#ifdef NEED_SIGNAL_CATCHER
#include <signal.h>

static j_common_ptr sig_cinfo;

GLOBAL void
signal_catcher (int signum)
{
  if (sig_cinfo != NULL) {
    if (sig_cinfo->err != NULL) /* turn off trace output */
      sig_cinfo->err->trace_level = 0;
    jpeg_destroy(sig_cinfo);	/* clean up memory allocation & temp files */
  }
  exit(EXIT_FAILURE);
}

#endif



METHODDEF void
output_init (j_decompress_ptr cinfo, djpeg_dest_ptr sinfo)
{
register U_IMAGE* img = cinfo->img;
/*	Do header transform. We suppose no mono_color images in JPEG	*/
	img->width = cinfo->image_width;
	img->height = cinfo->image_height;
	img->in_color = CFM_ILC;
	img->color_form = img->mid_type==HIPS ? CFM_ILC : img->in_color;
#ifndef	STREAM_IMAGE_LIB
	img->in_type = JPEG;
#endif
	if ((img->channels = cinfo->num_components) == 1)
		img->mono_img = True,
		img->color_form = img->in_color = CFM_SGF;
	else if (!img->color_dpy)
		img->color_form = CFM_SGF;
	img->dpy_channels = img->color_dpy ? img->channels : 1;
	img->frames += !img->frames;
	img->pxl_in = img->pxl_out = 1;
}


static	int	row_pos;	/* output buffer line ptr, not real row # */

METHODDEF void	/* convert JPEG buf to U_IMAGE buf	*/
put_pixel_rows(j_decompress_ptr cinfo, djpeg_dest_ptr sinfo, JDIMENSION rows)
{
register U_IMAGE* img = cinfo->img;
register int	c=img->dpy_channels, r, w=cinfo->image_width;
	if (img->in_color != img->color_form &&
		img->color_form != CFM_SEPLANE)	{
	register byte	*pr = (byte*)img->cnvt + row_pos * w;
	    for (r=0; r<rows; r++)
		if (img->color_form == CFM_SGF)
		    ilc_to_gray(pr, sinfo->buffer[0], w, No, NULL, No),	pr+=w;
		else	{
		register byte	*p=sinfo->buffer[0], *pg=pr+w, *pb=pg+w;
		register int	cnt=w;
			while (cnt--)	{
				*pr++ = *p++;	*pg++ = *p++;	*pb++ = *p++;
			}
			pr = pb;
		}
	} else	sinfo->buffer[0] += rows * w * c;	/* = pr; */
	row_pos += r*c;
}

METHODDEF void	/* JPEG claims that row_supplied is always 1	*/
put_demapped_rgb (j_decompress_ptr cinfo, djpeg_dest_ptr sinfo,
			JDIMENSION rows_supplied)
{
register JDIMENSION	w = cinfo->output_width;	/* * row_supplied */
register char *p = (char*)cinfo->img->cnvt + row_pos * w;
register JSAMPROW ptr = sinfo->buffer[0];
register JSAMPROW color_map0 = cinfo->colormap[0],
		color_map1 = cinfo->colormap[1],
		color_map2 = cinfo->colormap[2];

    while (w--)	{
	register int	v = GETJSAMPLE(*ptr++);
	*p++ = (char) GETJSAMPLE(color_map0[v]);
	*p++ = (char) GETJSAMPLE(color_map1[v]);
	*p++ = (char) GETJSAMPLE(color_map2[v]);
    }
row_pos += 3;	/* * row_supplied */
}


METHODDEF void
put_demapped_gray (j_decompress_ptr cinfo, djpeg_dest_ptr sinfo,
			JDIMENSION rows_supplied)
{
register JDIMENSION	w = cinfo->output_width;	/* * row_supplied */
register char *p = (char*)cinfo->img->cnvt + row_pos * w;
register JSAMPROW ptr = sinfo->buffer[0];
register JSAMPROW color_map = cinfo->colormap[0];

    while (w--)
	*p++ = (char) GETJSAMPLE(color_map[GETJSAMPLE(*ptr++)]);
row_pos++;	/* += row_supplied */
}



METHODDEF void
output_term (j_decompress_ptr cinfo, djpeg_dest_ptr sinfo)
{
	free(sinfo);
}

/*
 * This routine gets control after the input file header has been read.
 * It must determine what output file format is to be written,
 * and make any other decompression parameter changes that are desirable.
 */

static djpeg_dest_ptr
jinit_write_default(j_decompress_ptr cinfo)
{
/*
	cinfo->methods->output_init = output_init;
	cinfo->methods->put_color_map = put_color_map;
	cinfo->methods->put_pixel_rows = put_pixel_rows;
	cinfo->methods->output_term = output_term;
	put_pixel_rows(cinfo, 0, 0);
*/
djpeg_dest_ptr	dst = (djpeg_dest_ptr) (*cinfo->mem->alloc_small)
		((j_common_ptr) cinfo, JPOOL_IMAGE, SIZEOF(*dst));
	dst->start_output = output_init;
	dst->finish_output = output_term;
	jpeg_calc_output_dimensions(cinfo);
	dst->buffer_height = 1;
	if (cinfo->quantize_colors) {
		dst->buffer = (*cinfo->mem->alloc_sarray)
			((j_common_ptr) cinfo, JPOOL_IMAGE,
			cinfo->output_width, (JDIMENSION) 1);
		if (cinfo->out_color_space == JCS_GRAYSCALE)
			dst->put_pixel_rows = put_demapped_gray;
		else
			dst->put_pixel_rows = put_demapped_rgb;
	} else	{
		if (cinfo->img->in_color != cinfo->img->color_form &&
			cinfo->img->color_form != CFM_SEPLANE)
			dst->buffer = (*cinfo->mem->alloc_sarray)
				((j_common_ptr) cinfo, JPOOL_IMAGE,
				cinfo->output_width, (JDIMENSION) 1);
		else	dst->buffer = &cinfo->img->cnvt;
		dst->put_pixel_rows = put_pixel_rows;
	}
return	dst;
}



/*
 * The main entry.
 */

GLOBAL
jpeg_header_handle(int job, U_IMAGE *img, int ac, char **av, VType *assist)
{
int	i;
    switch (job)	{
    case HEADER_READ:
    case HEADER_FREAD:
	if (!av)
		fseek(img->IN_FP, 0, 0);

	jpeg_init_decompress(&dinfo);
	dinfo.img = img;
	dest_mgr = NULL;

  /* Now safe to enable signal catcher. */
#ifdef NEED_SIGNAL_CATCHER
	sig_cinfo = (j_common_ptr) dinfo;
	signal(SIGINT, signal_catcher);
#ifdef SIGTERM			/* not all systems have SIGTERM */
	signal(SIGTERM, signal_catcher);
#endif
#endif

#ifdef PROGRESS_REPORT
  /* Enable progress display, unless trace output is on */
	if (jerr.trace_level == 0)
	dc_methods.progress_monitor = progress_monitor;
#endif

  /* Specify data source for decompression */
	jpeg_stdio_src(&dinfo, dinfo.img->IN_FP);

  /* Read file header, set default decompression parameters */
	if (jpeg_read_header(&dinfo, TRUE) == JPEG_SUSPENDED)
	    if (requested_fmt == FMT_DEFAULT)
		return	EOF;
	    else	return	prgmerr(0, "not a JPEG file");

	img->in_type = JPEG;
	dest_mgr = setup_output_mgr(requested_fmt);

  /* Start decompressor */
	jpeg_start_decompress(&dinfo);

#ifdef PROGRESS_REPORT
  /* Clear away progress display */
  if (jerr.trace_level == 0) {
    fprintf(stderr, "\r                \r");
    fflush(stderr);
  }
#endif
	/* setup img vars, and write out file header if needed	*/
	(*dest_mgr->start_output) (&dinfo, dest_mgr);
	break;
    case HEADER_WRITE:
	break;
    default:	return	prgmerr(0, "no other JPEG handler");
    }
return	0;
}

read_jpeg_image(U_IMAGE *img, char *fmt, bool reload)
{
int	i, f=MAX(1, img->load_all), ret, fsz = img->width*img->height;

if (!reload)
	verify_buffer_size(&img->src, fsz, img->dpy_channels * f, "jpeg");

#ifdef	JPEG_INTERNAL_MF_FORCE	/* force multi-frame processing	*/
if (img->frames == 1 && f > 1)	/* for get_soi() in jdmarker.c	*/
	img->frames = f;	/* some JPEG boards are special	*/
#endif

for (i=0; i < f; i++)	{
	dinfo.img->cnvt = 	/* for movie. Does this bother others?	*/
	img->cnvt = (char*)img->src + i * fsz;	/* V5 feature	*/
	ret = read_next_jpeg_frame(&dinfo, dest_mgr);
}

if (!img->color_dpy || img->dpy_depth <= 8)
	img->channels = img->dpy_channels;
return	ret;
}

/*	The main decompress entry;
	img->cnvt must be set to loading buffer pointer	*/
read_next_jpeg_frame(j_decompress_ptr cinfo, djpeg_dest_ptr sinfo)
{
	if (!cinfo->inputctl->has_multiple_scans	/* V6 */
		&& feof(cinfo->img->IN_FP) && row_pos	/* not for first F */
	||	reset_scan(cinfo) < 0)	return	EOF;
	row_pos = 0;
	/* Process data */
	while (cinfo->output_scanline < cinfo->output_height)
		(*sinfo->put_pixel_rows) (cinfo, sinfo, jpeg_read_scanlines
			(cinfo, sinfo->buffer, sinfo->buffer_height));
return	cinfo->image_height * cinfo->image_width;
}

/*	The routine will reuse img->src to load every frame for
	HIPS output or DEFAULT_FMT	*/
jpeg_decompress(j_decompress_ptr cinfo, djpeg_dest_ptr sinfo)
{
U_IMAGE	*img = cinfo->img;
int	i = img->frames;
/* therefore, should we comfirm for just abvoe or all o_types ?	*/
	if ((img->o_type == HIPS || requested_fmt == DEFAULT_FMT) &&
		img->cnvt != img->src)	img->cnvt = img->src;
    do	{
	read_next_jpeg_frame(cinfo, sinfo);
	if (img->o_type == HIPS || requested_fmt == DEFAULT_FMT)
	    if (fwrite(img->src, img->dpy_channels,
			img->width*img->height, img->OUT_FP) > 0)
		img->cnvt = img->src;
	    else	break;
	} while (--i > 0);
	close_jpeg_read(cinfo, sinfo);
}

close_jpeg_read(j_decompress_ptr cinfo, djpeg_dest_ptr sinfo)
{
  /* Finish decompression and release memory */
  (*sinfo->finish_output) (cinfo, sinfo);
  jpeg_finish_decompress(cinfo);
  jpeg_destroy_decompress(cinfo);
}

