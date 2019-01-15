/*
 * jdmaster.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains the main control for the JPEG decompressor.
 * The system-dependent (user interface) code should call jpeg_decompress()
 * after doing appropriate setup of the decompress_info_struct parameter.
%
% Modified:     Jin Guojun - LBL, Image Technology Group
%       Date:   April 14, 1992
%       Goal:   To be easily handled by conversion library - CCS (c)
%		HIPS, FITS, GIF, RLE, SUN-raster, PNM, TIFF, PICT ...
%		can be compressed by cjpeg now, directly displayed by tuner,
%		decompressed to other image type by torle, torast, and color_ps.
%		These type of images can be determined by program `headers'.
*/

#include "jinclude.h"


METHODDEF void
d_per_scan_method_selection (decompress_info_ptr cinfo)
/* Central point for per-scan method selection */
{
  /* MCU disassembly */
  jseldmcu(cinfo);
  /* Upsampling of pixels */
  jselupsample(cinfo);
}


void
d_initial_method_selection (decompress_info_ptr cinfo)
/* Central point for initial method selection (after reading file header) */
{
  /* JPEG file scanning method selection is already done. */
  /* So is output file format selection (both are done by user interface). */

  /* Entropy decoding: either Huffman or arithmetic coding. */
#ifdef C_ARITH_CODING_SUPPORTED
  jseldarithmetic(cinfo);
#else
  if (cinfo->arith_code) {
    ERREXIT(cinfo->emethods, "Arithmetic coding not supported");
  }
#endif
  jseldhuffman(cinfo);
  /* Cross-block smoothing */
#ifdef BLOCK_SMOOTHING_SUPPORTED
  jselbsmooth(cinfo);
#else
  cinfo->do_block_smoothing = FALSE;
#endif
  /* Gamma and color space conversion */
  jseldcolor(cinfo);

  /* Color quantization selection rules */
#ifdef QUANT_1PASS_SUPPORTED
#ifdef QUANT_2PASS_SUPPORTED
  /* We have both, check for conditions in which 1-pass should be used */
  if (cinfo->num_components != 3 || cinfo->jpeg_color_space != CS_YCbCr)
    cinfo->two_pass_quantize = FALSE; /* 2-pass only handles YCbCr input */
  if (cinfo->out_color_space == CS_GRAYSCALE)
    cinfo->two_pass_quantize = FALSE; /* Should use 1-pass for grayscale out */
#else /* not QUANT_2PASS_SUPPORTED */
  cinfo->two_pass_quantize = FALSE; /* only have 1-pass */
#endif
#else /* not QUANT_1PASS_SUPPORTED */
#ifdef QUANT_2PASS_SUPPORTED
  cinfo->two_pass_quantize = TRUE; /* only have 2-pass */
#else /* not QUANT_2PASS_SUPPORTED */
  if (cinfo->quantize_colors) {
    ERREXIT(cinfo->emethods, "Color quantization was not compiled");
  }
#endif
#endif

#ifdef QUANT_1PASS_SUPPORTED
  jsel1quantize(cinfo);
#endif
#ifdef QUANT_2PASS_SUPPORTED
  jsel2quantize(cinfo);
#endif

  /* Pipeline control */
  jseldpipeline(cinfo);
  /* Overall control (that's me!) */
  cinfo->methods->d_per_scan_method_selection = d_per_scan_method_selection;
}


void
initial_setup (decompress_info_ptr cinfo)
/* Do computations that are needed before initial method selection */
{
  short ci;
  jpeg_component_info *compptr;

  /* Compute maximum sampling factors; check factor validity */
  cinfo->max_h_samp_factor = 1;
  cinfo->max_v_samp_factor = 1;
  for (ci = 0; ci < cinfo->num_components; ci++) {
    compptr = &cinfo->comp_info[ci];
    if (compptr->h_samp_factor<=0 || compptr->h_samp_factor>MAX_SAMP_FACTOR ||
	compptr->v_samp_factor<=0 || compptr->v_samp_factor>MAX_SAMP_FACTOR)
      ERREXIT(cinfo->emethods, "Bogus sampling factors");
    cinfo->max_h_samp_factor = MAX(cinfo->max_h_samp_factor,
				   compptr->h_samp_factor);
    cinfo->max_v_samp_factor = MAX(cinfo->max_v_samp_factor,
				   compptr->v_samp_factor);

  }

  /* Compute logical downsampled dimensions of components */
  for (ci = 0; ci < cinfo->num_components; ci++) {
    compptr = &cinfo->comp_info[ci];
    compptr->true_comp_width = (cinfo->image_width * compptr->h_samp_factor
				+ cinfo->max_h_samp_factor - 1)
				/ cinfo->max_h_samp_factor;
    compptr->true_comp_height = (cinfo->image_height * compptr->v_samp_factor
				 + cinfo->max_v_samp_factor - 1)
				 / cinfo->max_v_samp_factor;
  }
}


/*
 * This is the main entry point to the JPEG decompressor.
 */


jpeg_decompress (decompress_info_ptr cinfo)
{
int	i = cinfo->img->frames;
    do	{
	read_next_jpeg_frame(cinfo);
	if (cinfo->img->o_type == HIPS)
		fwrite(cinfo->img->src, cinfo->img->dpy_channels,
		    cinfo->img->width*cinfo->img->height, cinfo->img->OUT_FP);
	} while (--i > 0);
    close_jpeg_read(cinfo);
}

read_next_jpeg_frame(decompress_info_ptr cinfo)
{
int	orgi_passes = cinfo->total_passes;
	if (feof(cinfo->input_file))	return	EOF;
  /* And let the pipeline controller do the rest. */
	(*cinfo->methods->d_pipeline_controller) (cinfo);
	(*cinfo->methods->read_file_trailer) (cinfo);
	cinfo->total_passes = orgi_passes;
	cinfo->completed_passes = 0;
	cinfo->methods->put_pixel_rows(cinfo, 0, 0);
return	cinfo->image_height * cinfo->image_width;
}

close_jpeg_read(decompress_info_ptr cinfo)
{
  /* Finish output file, release working storage, etc */
  if (cinfo->quantize_colors)
    (*cinfo->methods->color_quant_term) (cinfo);
  (*cinfo->methods->colorout_term) (cinfo);
  (*cinfo->methods->output_term) (cinfo);

  (*cinfo->emethods->free_all) ();

  /* My, that was easy, wasn't it? */
}
