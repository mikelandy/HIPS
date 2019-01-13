/*
%	STReaM_JPEG_READ_WRITE . C
%
%	Copyright (c)	Lawrence Berkeley Labroatory
%
% AUTHOR:     Jin Guojun - LBL, Image Technology Group	1992
*/

#ifdef	STREAM_IMAGE
#include "jinclude.h"
#ifdef INCLUDES_ARE_ANSI
#include <stdlib.h>		/* to declare exit() */
#endif
#include <ctype.h>		/* to declare tolower() */
#ifdef NEED_SIGNAL_CATCHER
#include <signal.h>		/* to declare signal() */
#endif

#ifndef EXIT_FAILURE		/* define exit() codes if not provided */
#define EXIT_FAILURE  1
#endif

#include "jversion.h"		/* for version message */
#include "hipl_format.h"


/*
 * This routine determines what format the input file is,
 * and selects the appropriate input-reading module.
 *
 * To determine which family of input formats the file belongs to,
 * we may look only at the first byte of the file, since C does not
 * guarantee that more than one character can be pushed back with ungetc.
 * Looking at additional bytes would require one of these approaches:
 */

int	should_be;
U_IMAGE	uimg;	/* must be global for ld overplacing it.
		If any program, which doesn't use a global uimg, has to use
		get_jpeg_uimg() and set_jpeg_uimg() to exchange its own img
		context with this uimg structure.	*/

void
get_jpeg_uimg(U_IMAGE *img)
{
	*img = uimg;
}

void
set_jpeg_uimg(U_IMAGE *img)
{
	uimg = *img;
}


/*
 * This routine gets control after the input file header has been read.
 * It must determine what output JPEG file format is to be written,
 * and make any other compression parameter changes that are desirable.
 */

METHODDEF void
c_ui_method_selection(compress_info_ptr cinfo)
{
    /* If the input is gray scale, generate a monochrome JPEG file. */
    if (cinfo->in_color_space == CS_GRAYSCALE) {
	j_monochrome_default(cinfo);
    }
    /* For now, always select JFIF output format. */
#ifdef JFIF_SUPPORTED
    jselwjfif(cinfo);
#else
    You shoulda defined JFIF_SUPPORTED.	/* deliberate syntax error */
#endif
}


/*
 * Signal catcher to ensure that temporary files are removed before aborting.
 * NB: for Amiga Manx C this is actually a global routine named _abort();
 * see -Dsignal_catcher=_abort in CFLAGS.  Talk about bogus...
 */

#ifdef NEED_SIGNAL_CATCHER

static external_methods_ptr emethods;	/* for access to free_all */

GLOBAL void
signal_catcher(int signum)
{
    if (emethods != NULL) {
	emethods->trace_level = 0;	/* turn off trace output */
	(*emethods->free_all) ();	/* clean up memory allocation & temp
					 * files */
    }
    exit(EXIT_FAILURE);
}

#endif


/*
 * The entry program.
 */

LOCAL struct Compress_info_struct cinfo;
LOCAL struct Compress_methods_struct c_methods;

jpeg_uimg_init(FILE *fp, int itype, int rw, int color)
{	/* only initial once!	*/
	if (!rw && uimg.IN_FP == fp || rw && uimg.OUT_FP == fp)	return;
	uimg.color_dpy = color;
	format_init(&uimg, IMAGE_INIT_TYPE, itype, JPEG, "sjpeg", "N25-2");
	if (fp)	if (rw)	uimg.OUT_FP = fp;
		else	uimg.IN_FP = fp;
	init_pipe_read(&uimg);
}

hjpeg_color(struct header *hd)
{
	memcpy(&hhd, hd, sizeof(*hd));
#ifdef	_DEBUG_
	hips_header_handle(HEADER_FROM, &uimg, hd);
#else
	hips_header_to_jpeg(HEADER_FROM, &uimg, hd);
#endif
	return	isColorImage(uimg.in_color);
}

GLOBAL
hhjpeg_winit(FILE * hfp, struct header * hhdp, int qfactor, int color_flag)
{
	jpeg_uimg_init(hfp, RLE, True, color_flag);
	uimg.color_dpy = color_flag;
	cinfo.img = &uimg;
#ifdef	_DEBUG_
	hips_header_handle(HEADER_FROM, &uimg, hhdp);
#else
	hips_header_to_jpeg(HEADER_FROM, &uimg, hhdp);
#endif
	if (color_flag | isColorImage(uimg.in_color))	{
		uimg.dpy_channels = 3;
		uimg.color_form = CFM_ILL;
	}

    uimg.src = uimg.dest = hhdp->image;
    if (!uimg.dpy_channels)	uimg.dpy_channels = uimg.channels;

    /* Set up links to method structures. */
    cinfo.methods = &c_methods;
    cinfo.emethods = &e_methods;

    /* Install, but don't yet enable signal catcher. */
#ifdef NEED_SIGNAL_CATCHER
    emethods = NULL;
    signal(SIGINT, signal_catcher);
#ifdef SIGTERM			/* not all systems have SIGTERM */
    signal(SIGTERM, signal_catcher);
#endif
#endif

    /* (Re-)initialize the system-dependent error and memory managers. */
    jselerror(cinfo.emethods);	/* error/trace message routines */
    jselmemmgr(cinfo.emethods);	/* memory allocation routines */
    cinfo.methods->c_ui_method_selection = c_ui_method_selection;

    /* Now OK to enable signal catcher. */
#ifdef NEED_SIGNAL_CATCHER
    emethods = cinfo.emethods;
#endif

    /* Set up default JPEG parameters. */
    j_c_defaults(&cinfo, qfactor, FALSE);	/* default quality level = 75 */

    cinfo.output_file = uimg.OUT_FP = hfp;

    cinfo.input_file = uimg.IN_FP ? uimg.IN_FP : stdin;

    /* Figure out the input file format, and set up to read it. */
    select_file_type(&cinfo, No);
	config_stream_link(Yes);	/* for init_jpeg_header	*/

#ifdef PROGRESS_REPORT
    /* Start up progress display, unless trace output is on */
    if (e_methods.trace_level == 0)
	c_methods.progress_monitor = progress_monitor;
#endif
	cinfo.comp_info[0].h_samp_factor = 2;
	cinfo.comp_info[0].v_samp_factor = 1;
	init_jpeg_header(&cinfo);	/* Must be last line here	*/
}

stream_jpeg_read_image(void *buf, struct header *hdp, int fn)
{
	uimg.src = buf;
	if (get_sinfo(1, 0) == 1)	{
	sframe_header	sf;
		read_stream_header(uimg.IN_FP, hdp, &sf, fn);
		return	stream_jpeg_read_frame(buf, &sf);
	}
	return	(*uimg.std_swif)(FI_LOAD_FILE, &uimg, NULL, 0);
}


/*	STReaM_JPEG_READ . C
%
*/

#ifndef DEFAULT_FMT		/* so can override from CFLAGS in Makefile */
#define DEFAULT_FMT	FMT_PPM
#endif



/*
 * The entry program.
 */

#ifndef	JPEG_BUFFER_EDGE
#define	JPEG_BUFFER_EDGE	512
#endif
#define	JPEG_IN_BUF_SIZE	JPEG_BUFFER_EDGE * JPEG_BUFFER_EDGE

GLOBAL int
stream_jpeg_rinit(FILE * hfp, int qfact /* not used */, int color_flag)
{
extern IMAGE_FORMATS requested_fmt;

    jpeg_uimg_init(hfp, RLE, False, color_flag);
    uimg.color_dpy = color_flag;
    uimg.in_form = IFMT_STREAM;
    requested_fmt = FMT_DEFAULT;

    /* Start up progress display, unless trace output is on */
#ifdef PROGRESS_REPORT
    if        (e_methods.trace_level == 0)
	dc_methods.progress_monitor = progress_monitor;
#endif

    if (jpeg_header_handle(HEADER_READ, &uimg, 0, 1, 0)) {
	return -1;
    } else {
	register int w = uimg.width, h = uimg.height;
	if (w * h > JPEG_IN_BUF_SIZE)
		w = h = JPEG_BUFFER_EDGE;
	verify_buffer_size(&dinfo.input_buffer, w, h, "jsrc");
    }
return 0;
}


int
stream_write_jpeg_subheader()
{
	(*cinfo.methods->write_scan_header) (&cinfo);
	return(cinfo.img->tmp_offset);
}

stream_jpeg_read_frame(void *buf, sframe_header *sf)
{
	dinfo.img->tmp_offset = sf->size;
	should_be = sf->size + ftell(dinfo.input_file);
	uimg.src = buf;
return	read_next_jpeg_frame(&dinfo);
}

stream_jpeg_read_term()
{
	close_jpeg_read(&dinfo);
}

stream_jpeg_write_frame(void *buf, sframe_header * sf)
{
    uimg.dest = uimg.src = buf;
    uimg.o_form = IFMT_STREAM;
    write_next_jpeg_frame(&cinfo);
    sf->size = uimg.tmp_offset;
}

stream_jpeg_write_term()
{
	close_jpeg_write(&cinfo);
	write_jpeg_eof(cinfo.output_file);
}

write_jpeg_eof(FILE *fp)
{
#ifdef	_DEBUG_
register int	i = ftell(fp);
	fprintf(stderr, "M_EOI is put at %d (%X)\n", i, i);
#endif
	putc(0xff,fp);  
	putc(0xd9,fp);	/* end of image marker (from jrdjiff.c) */
}

#ifndef	_DEBUG_
hips_header_to_jpeg(int job, U_IMAGE *img, struct header *hhd)
{
int	hform = hhd->pixel_format;
	img->in_type = HIPS;
	img->cmaplen = 0;
	img->color_form = CFM_SGF;	/* default to gray scale	*/
	img->channels = hhd->numcolor;
	if (findparam(hhd, "cmap") != NULLPAR)	{
	VType*	cmapp;
	int	nc;
		getparam(hhd, "cmap", PFBYTE, &nc, &cmapp);
		if (nc % 3 || nc > MaxColors*3)
			prgmerr(0, "strange colormap = %d", nc);
		else	{
			if (reg_cmap[0])
				free(reg_cmap[0]);
			reg_cmap[0] = cmapp;
			img->cmaplen = (nc /= 3);
			reg_cmap[1] = reg_cmap[0] + nc;
			reg_cmap[2] = reg_cmap[1] + nc;
			img->in_color = img->color_form = CFM_SCF;
		}
	}
	else	if (img->channels==3)	{	/* kludge handle SEPLANE */
			if (hform==IFMT_ILC)	/* PFRGB */
				img->in_color = CFM_ILC;
			else	img->in_color = CFM_SEPLANE;
		}
		else if (hform==IFMT_ALPHA)
			img->in_color = CFM_ALPHA,
			img->channels = 4;
		else if (hform == IFMT_ILC)
			img->in_color = CFM_ILC,
			img->channels = 3;
		else	img->in_color = CFM_SGF;

	/*	should select_color_form here	*/
	if ((img->in_color == CFM_SEPLANE) & !img->color_dpy) {
		img->dpy_channels = 1;
		img->color_form = CFM_SGF;
	}

#if	defined COMMON_TOOL | defined RLE_IMAGE
	/* not force to stick at 1 channel	*/
	if (img->mid_type==RLE && img->cmaplen && !assist)	{
		rle_dflt_hdr.ncmap = img->mono_img = 0;
		if (img->color_form != CFM_SGF)
			img->dpy_channels = img->channels = 3;
	}
#endif
	img->frames = hhd->num_frame / hhd->numcolor;
	img->in_form = hhd->pixel_format;
	img->width = hhd->ocols;
	img->height = hhd->orows;
	img->pxl_in = pxl_size((*hhd));
	if (img->pxl_in == 3)
		img->pxl_in = 1;	/* for PFRGB	*/
	if (!img->pxl_in) {
	register int	vsize;
		switch (img->in_form)	{
		case IFMT_VFFT3D:
		case IFMT_VFFT2D:
		case IFMT_VVFFT3D:
			vsize = sizeof(float)<<1;
			break;
		case IFMT_DVFFT3D:
		case IFMT_DVFFT2D:
		case IFMT_DVVFFT3D:
			vsize = sizeof(double)<<1;
			break;
		case IFMT_HIST:
			vsize  = sizeof(int);
			break;
		default:vsize = 1;
			msg("%d: undetermined size", img->in_form);
		}
		img->pxl_in = vsize;
	}
	img->history = hhd->seq_history;
	img->desc = hhd->seq_desc;
	img->sub_img_w = hhd->cols;
	img->sub_img_h = hhd->rows;
	img->sub_img_x = hhd->fcol;
	img->sub_img_y = hhd->frow;
	select_color_form(img, True);
}
#endif
#endif

