/*	jpeg_r.c
%
% AUTHOR:	Jin Guojun - LBL, Image Technology Group
% Date:		April 14, 1992
% Goal:		This file is derived from jdmain.c and jdmaster.c as part of
%		CCS library file to handle JPEG image.
*/

#include "jinclude.h"
#ifdef INCLUDES_ARE_ANSI
#include <stdlib.h>		/* to declare exit() */
#endif
#include <ctype.h>		/* to declare tolower() */
#ifdef NEED_SIGNAL_CATCHER
#include <signal.h>		/* to declare signal() */
#endif

#ifdef THINK_C
#include <console.h>		/* command-line reader for Macintosh */
#endif

#ifdef DONT_USE_B_MODE		/* define mode parameters for fopen() */
#define READ_BINARY	"r"
#define WRITE_BINARY	"w"
#else
#define READ_BINARY	"rb"
#define WRITE_BINARY	"wb"
#endif

#ifndef EXIT_FAILURE		/* define exit() codes if not provided */
#define EXIT_FAILURE  1
#endif
#ifndef EXIT_SUCCESS
#ifdef VMS
#define EXIT_SUCCESS  1		/* VMS is very nonstandard */
#else
#define EXIT_SUCCESS  0
#endif
#endif


#include "jversion.h"		/* for version message */


#ifndef DEFAULT_FMT		/* so can override from CFLAGS in Makefile */
#define DEFAULT_FMT	FMT_DEFAULT
#endif

IMAGE_FORMATS requested_fmt = DEFAULT_FMT;	/* set default output file format */

void	jselwdefault();

/*
 * This routine gets control after the input file header has been read.
 * It must determine what output file format is to be written,
 * and make any other decompression parameter changes that are desirable.
 */

METHODDEF void
d_ui_method_selection (decompress_info_ptr cinfo)
{
  /* if grayscale or CMYK input, force similar output; */
  /* else leave the output colorspace as set by options. */
  if (cinfo->jpeg_color_space == CS_GRAYSCALE)
    cinfo->out_color_space = CS_GRAYSCALE;
  else if (cinfo->jpeg_color_space == CS_CMYK)
    cinfo->out_color_space = CS_CMYK;

  /* select output file format */
  /* Note: jselwxxx routine may make additional parameter changes,
   * such as forcing color quantization if it's a colormapped format.
   */
  switch (requested_fmt) {
#ifdef GIF_SUPPORTED
  case FMT_GIF:
    jselwgif(cinfo);
    break;
#endif
#ifdef PPM_SUPPORTED
  case FMT_PPM:
    jselwppm(cinfo);
    break;
#endif
#ifdef RLE_SUPPORTED
  case FMT_RLE:
    jselwrle(cinfo);
    break;
#endif
#ifdef TARGA_SUPPORTED
  case FMT_TARGA:
    jselwtarga(cinfo);
    break;
#endif
  default:
	jselwdefault(cinfo);
    break;
  }
}


/*
 * Signal catcher to ensure that temporary files are removed before aborting.
 * NB: for Amiga Manx C this is actually a global routine named _abort();
 * see -Dsignal_catcher=_abort in CFLAGS.  Talk about bogus...
 */

#ifdef NEED_SIGNAL_CATCHER

static external_methods_ptr emethods; /* for access to free_all */

GLOBAL void
signal_catcher (int signum)
{
  if (emethods != NULL) {
    emethods->trace_level = 0;	/* turn off trace output */
    (*emethods->free_all) ();	/* clean up memory allocation & temp files */
  }
  exit(EXIT_FAILURE);
}

#endif


/*
 * Optional routine to display a percent-done figure on stderr.
 * See jddeflts.c for explanation of the information used.
 */

#ifdef PROGRESS_REPORT

METHODDEF void
progress_monitor (decompress_info_ptr cinfo, long loopcounter, long looplimit)
{
  if (cinfo->total_passes > 1) {
    fprintf(stderr, "\rPass %d/%d: %3d%% ",
	    cinfo->completed_passes+1, cinfo->total_passes,
	    (int) (loopcounter*100L/looplimit));
  } else {
    fprintf(stderr, "\r %3d%% ",
	    (int) (loopcounter*100L/looplimit));
  }
  fflush(stderr);
}

#endif


METHODDEF void
output_init (decompress_info_ptr cinfo)
{
register U_IMAGE* img = cinfo->img;
/*	Do header transform. We suppose no mono_color images in JPEG	*/
	img->width = cinfo->image_width;
	img->height = cinfo->image_height;
	img->in_color = CFM_ILL;
	img->color_form = img->mid_type==HIPS ? CFM_ILC : img->in_color;
	img->in_type = JPEG;
	if ((img->channels = cinfo->num_components) == 1)
		img->mono_img = True,
		img->color_form = img->in_color = CFM_SGF;
	else if (!img->color_dpy)
		img->color_form = CFM_SGF;
	img->dpy_channels = img->color_dpy ? img->channels : 1;
	img->frames += !img->frames;
	img->pxl_in = img->pxl_out = 1;
}

METHODDEF void
put_color_map (decompress_info_ptr cinfo, int num_colors, JSAMPARRAY colormap)
{
/*	do color map transform	*/
}

METHODDEF void	/* convert JPEG buf to U_IMAGE buf	*/
put_pixel_rows(decompress_info_ptr cinfo, int rows, JSAMPIMAGE dataptr)
{
static	int	row_pos;

if (!dataptr)
	row_pos = 0;
else	{
register U_IMAGE* img = cinfo->img;
register int	c=img->dpy_channels, r, w=cinfo->image_width;
register byte	*p = (byte*)img->src + row_pos * w;
	for (r=0; r<rows; r++)
	    if (img->in_color != img->color_form &&
		img->color_form != CFM_SEPLANE)
		if (img->color_form == CFM_SGF)
		    ill_to_gray(p, dataptr[0][r], dataptr[1][r],
			dataptr[2][r], w),	p+=w;
		else	{
		register byte	*pr=dataptr[0][r], *pg=dataptr[1][r],
			*pb = dataptr[2][r];
		register int	cnt=w;
			while (cnt--)	{
				*p++ = *pr++;	*p++ = *pg++;	*p++ = *pb++;
			}
		}
	    else for (c=0; c<img->channels; c++, p+=w)
		memcpy(p, dataptr[c][r], w);
	row_pos += r*c;
}
}

METHODDEF void
output_term (decompress_info_ptr cinfo)
{
/*	do nothing	*/
}


/*
 * This routine gets control after the input file header has been read.
 * It must determine what output file format is to be written,
 * and make any other decompression parameter changes that are desirable.
 */

METHODDEF void
jselwdefault (decompress_info_ptr cinfo)
{
	cinfo->methods->output_init = output_init;
	cinfo->methods->put_color_map = put_color_map;
	cinfo->methods->put_pixel_rows = put_pixel_rows;
	cinfo->methods->output_term = output_term;
	put_pixel_rows(cinfo, 0, 0);
}



/*
 * The main entry.
 */

struct Decompress_info_struct dinfo;
struct Decompress_methods_struct dc_methods;
struct External_methods_struct e_methods;

GLOBAL
jpeg_header_handle(int job, U_IMAGE *img, int ac, char **av, VType *assist)
{
int	i;
    switch (job)	{
    case HEADER_READ:
    case HEADER_FREAD:
	if (!av)
		fseek(img->IN_FP, 0, 0);
	dinfo.img = img;
	/* Initialize the system-dependent method pointers. */
	dinfo.methods = &dc_methods;
	dinfo.emethods = &e_methods;
	jselerror(&e_methods);	/* error/trace message routines */
	jselmemmgr(&e_methods);	/* memory allocation routines */
	dc_methods.d_ui_method_selection = d_ui_method_selection;

	/* Now OK to enable signal catcher. */
#ifdef NEED_SIGNAL_CATCHER
	emethods = &e_methods;
	signal(SIGINT, signal_catcher);
#ifdef SIGTERM			/* not all systems have SIGTERM */
	signal(SIGTERM, signal_catcher);
#endif
#endif

	/* Set up default JPEG parameters. */
	j_d_defaults(&dinfo, TRUE);

	dinfo.input_file = img->IN_FP;
	dinfo.output_file = img->OUT_FP;

  /* Set up to read a JFIF or baseline-JPEG file. */
  /* A smarter UI would inspect the first few bytes of the input file */
  /* to determine its type. */
#ifdef JFIF_SUPPORTED
	jselrjfif(&dinfo);
#else
	You shoulda defined JFIF_SUPPORTED.   /* deliberate syntax error */
#endif

	/* From jdmaster.c --- Init pass counts to 0 ---
		total_passes is adjusted in method selection */
	dinfo.total_passes = 0;
	dinfo.completed_passes = 0;

  /* Read the JPEG file header markers; everything up through the first SOS
   * marker is read now.  NOTE: the user interface must have initialized the
   * read_file_header method pointer (eg, by calling jselrjfif or jselrtiff).
   * The other file reading methods (read_scan_header etc.) were probably
   * set at the same time, but could be set up by read_file_header itself.
   */
	if ((*dinfo.methods->read_file_header) (&dinfo) < 0
		|| ! ((*dinfo.methods->read_scan_header) (&dinfo)))
	    if (requested_fmt == FMT_DEFAULT)
		return	EOF;
	    else	return	prgmerr(0, "not a JPEG file");

  /* Give UI a chance to adjust decompression parameters and select */
  /* output file format based on info from file header. */
	(*dinfo.methods->d_ui_method_selection) (&dinfo);

  /* Now select methods for decompression steps. */
	initial_setup(&dinfo);
	d_initial_method_selection(&dinfo);

  /* Initialize the output file & other modules as needed */
  /* (modules needing per-scan init are called by pipeline controller) */

	(*dinfo.methods->output_init) (&dinfo);
	(*dinfo.methods->colorout_init) (&dinfo);
	if (dinfo.quantize_colors)
		(*dinfo.methods->color_quant_init) (&dinfo);
	break;
    case HEADER_WRITE:
	break;
    default:	return	prgmerr(0, "no other JPEG handler");
    }
return	0;
}

read_jpeg_image(U_IMAGE *img, char *fmt, bool reload)
{
int	ret;
if (!reload)
verify_buffer_size(&img->src, img->width*img->height, img->dpy_channels, "jpeg");
ret = read_next_jpeg_frame(&dinfo);

if (!img->color_dpy || img->dpy_depth <= 8)
	img->channels = img->dpy_channels;
return	ret;
}
