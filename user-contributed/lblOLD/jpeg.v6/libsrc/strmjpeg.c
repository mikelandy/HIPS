/*
%	STReaM_JPEG_READ_WRITE . C
%
%	Copyright (c)	Lawrence Berkeley Labroatory
%
% AUTHOR:     Jin Guojun - LBL, Image Technology Group	1992
*/

#ifdef	STREAM_IMAGE_LIB
#include "cdjpeg.h"
#include "jversion.h"		/* for version message */
#include "hipl_format.h"

#ifndef EXIT_FAILURE		/* define exit() codes if not provided */
#define EXIT_FAILURE  1
#endif



/*
 * This routine determines what format the input file is,
 * and selects the appropriate input-reading module.
 *
 * To determine which family of input formats the file belongs to,
 * we may look only at the first byte of the file, since C does not
 * guarantee that more than one character can be pushed back with ungetc.
 * Looking at additional bytes would require one of these approaches:
 */


U_IMAGE	uimg;	/* must be global for ld overplacing it.
		If any program, which doesn't use a global uimg, has to use
		get_jpeg_uimg() and set_jpeg_uimg() to exchange its own img
		context with this uimg structure.	*/
extern	char*	Progname;

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

#ifdef NEED_SIGNAL_CATCHER

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

/*
 * The entry program.
 */

extern	struct jpeg_error_mgr	jerr;
LOCAL	struct jpeg_compress_struct	cinfo;
LOCAL	cjpeg_source_ptr	inp_mgr;

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

	if (hhdp->image)
		uimg.src = uimg.dest = hhdp->image;
	if (!uimg.dpy_channels)	uimg.dpy_channels = uimg.channels;

/* Initialize the JPEG compression object with default error handling. */
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	format_init(&uimg, IMAGE_INIT_TYPE, HIPS, JPEG, Progname, "M5-2");
	uimg.color_dpy = True;
	cinfo.img = &uimg;

	error_init();

  /* Now safe to enable signal catcher. */
#ifdef NEED_SIGNAL_CATCHER
  sig_cinfo = (j_common_ptr) cinfo;
  signal(SIGINT, signal_catcher);
#ifdef SIGTERM			/* not all systems have SIGTERM */
  signal(SIGTERM, signal_catcher);
#endif
#endif

	uimg.OUT_FP = hfp;

	/* Figure out the input file format, and set up to read it. */
	inp_mgr = select_file_type(&cinfo, uimg.IN_FP ? uimg.IN_FP : stdin, No);
	config_stream_link(Yes);	/* for init_jpeg_header */

#ifdef PROGRESS_REPORT
    /* Start up progress display, unless trace output is on */
    if (jerr.trace_level == 0)
	c_methods.progress_monitor = progress_monitor;
#endif
	init_jpeg_header(&cinfo, inp_mgr);	/* Must be last line here */
	cinfo.comp_info[0].h_samp_factor = 2;	/* for parallax board	*/
	cinfo.comp_info[0].v_samp_factor = 1;
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


stream_write_jpeg_subheader()	/*	who needs it ?	*/
{
register int	total = cinfo.img->tmp_offset;
	re_init_scan(&cinfo);
return	total;
}

stream_jpeg_write_frame(void *buf, sframe_header * sf)
{
#ifdef	SOI_SOF_SOS_EOI	/* it should be after re_init, but finish_comp() did */
    jpeg_start_compress(&cinfo, FALSE);
#endif
    uimg.dest = uimg.src = buf;
    uimg.o_form = IFMT_STREAM;
    re_init_scan(&cinfo);
    write_next_jpeg_frame(&cinfo, inp_mgr);
#ifdef	SOI_SOF_SOS_EOI
    cinfo.dest->free_in_buffer = -1;	/* don't flush data in buffer */
    jpeg_finish_compress(&cinfo);
#endif
    sf->size = uimg.tmp_offset;
}

stream_jpeg_write_term()
{
#ifndef	SOI_SOF_SOS_EOI
    (*cinfo.dest->init_destination)(&cinfo);
#endif
    close_jpeg_write(&cinfo, inp_mgr);
}

write_jpeg_eof(FILE *fp)
{
#ifdef	_DEBUG_
register int	i = ftell(fp);
	fprintf(stderr, "M_EOI is put at %d (%X)\n", i, i);
#endif
   putc(0xff,fp);  
   putc(0xd9,fp);
}


/*	STReaM_JPEG_READ . C
%
*/


#ifndef	JPEG_BUFFER_EDGE
#define	JPEG_BUFFER_EDGE	768
#endif
#define	JPEG_IN_BUF_SIZE	JPEG_BUFFER_EDGE * JPEG_BUFFER_EDGE

GLOBAL int
stream_jpeg_rinit(FILE * hfp, int qfact /* not used */, int color_flag)
{
/*extern IMAGE_FORMATS requested_fmt;*/

    jpeg_uimg_init(hfp, RLE, False, color_flag);
    uimg.color_dpy = color_flag;
    uimg.in_form = IFMT_STREAM;
/*    requested_fmt = FMT_DEFAULT;	*/

    /* Start up progress display, unless trace output is on */
#ifdef PROGRESS_REPORT
    if (jerr.trace_level == 0)
	dc_methods.progress_monitor = progress_monitor;
#endif

    if (jpeg_header_handle(HEADER_READ, &uimg, 0, 1, 0))
	return -1;
return 0;
}


extern	djpeg_dest_ptr	dest_mgr;
extern	struct jpeg_decompress_struct	dinfo;

stream_jpeg_read_frame(void *buf, sframe_header *sf)
{
int	ret;
	dinfo.img->tmp_offset = sf->size;
	dinfo.should_be = sf->size + ftell(dinfo.img->IN_FP) -
		dinfo.src->bytes_in_buffer;
	uimg.src = uimg.cnvt = buf;
	ret = read_next_jpeg_frame(&dinfo, dest_mgr);
	if (!feof(uimg.IN_FP))	reset_scan(&dinfo);
return	ret;
}

stream_jpeg_read_term()
{
    close_jpeg_read(&dinfo, inp_mgr);
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

