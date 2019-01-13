/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * write_histo.c - write a histogram frame
 *
 * Michael Landy - 6/30/91
 */

#include <stdio.h>
#include <hipl_format.h>

int write_histo(histo,fr)

struct hips_histo *histo;
int fr;

{
	return(fwrite_histo(stdout,histo,fr,"<stdout>"));
}

int fwrite_histo(fp,histo,fr,fname)

FILE *fp;
struct hips_histo *histo;
int fr;
Filename fname;

{
	if (fwrite(histo->histo,histo->sizehist,1,fp) != 1)
		return(perr(HE_WRITEFRFILE,fr,fname));
	return(HIPS_OK);
}

int histo_to_hdr(hd,histo)

struct header *hd;
struct hips_histo *histo;

{
	if (setparam(hd,"numbin",PFINT,1,histo->nbins) == HIPS_ERROR)
		return(HIPS_ERROR);
	if (setparam(hd,"imagepixfmt",PFINT,1,histo->pixel_format) ==
	    HIPS_ERROR)
		return(HIPS_ERROR);
	switch(histo->pixel_format) {
	case PFBYTE:	if (setparam(hd,"binleft",PFINT,1,
			    (int) (histo->minbin.v_byte)) == HIPS_ERROR)
				return(HIPS_ERROR);
			if (setparam(hd,"binwidth",PFINT,1,
			    (int) (histo->binwidth.v_byte)) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	case PFSBYTE:	if (setparam(hd,"binleft",PFINT,1,
			    (int) (histo->minbin.v_sbyte)) == HIPS_ERROR)
				return(HIPS_ERROR);
			if (setparam(hd,"binwidth",PFINT,1,
			    (int) (histo->binwidth.v_sbyte)) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	case PFSHORT:	if (setparam(hd,"binleft",PFINT,1,
			    (int) (histo->minbin.v_short)) == HIPS_ERROR)
				return(HIPS_ERROR);
			if (setparam(hd,"binwidth",PFINT,1,
			    (int) (histo->binwidth.v_short)) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	case PFUSHORT:	if (setparam(hd,"binleft",PFINT,1,
			    (int) (histo->minbin.v_ushort)) == HIPS_ERROR)
				return(HIPS_ERROR);
			if (setparam(hd,"binwidth",PFINT,1,
			    (int) (histo->binwidth.v_ushort)) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	case PFINT:	if (setparam(hd,"binleft",PFINT,1,histo->minbin.v_int)
			    == HIPS_ERROR)
				return(HIPS_ERROR);
			if (setparam(hd,"binwidth",PFINT,1,
			    histo->binwidth.v_int) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	case PFUINT:	if (setparam(hd,"binleft",PFINT,1,
			    (int) (histo->minbin.v_uint)) == HIPS_ERROR)
				return(HIPS_ERROR);
			if (setparam(hd,"binwidth",PFINT,1,
			    (int) (histo->binwidth.v_uint)) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	case PFFLOAT:
	case PFCOMPLEX:	if (setparam(hd,"binleft",PFFLOAT,1,
			    histo->minbin.v_float) == HIPS_ERROR)
				return(HIPS_ERROR);
			if (setparam(hd,"binwidth",PFFLOAT,1,
			    histo->binwidth.v_float) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	case PFDOUBLE:
	case PFDBLCOM:	if (setparam(hd,"binleft",PFFLOAT,1,
			    (float) (histo->minbin.v_double)) == HIPS_ERROR)
				return(HIPS_ERROR);
			if (setparam(hd,"binwidth",PFFLOAT,1,
			    (float) (histo->binwidth.v_double)) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	default:	return(perr(HE_FMTSUBR,"histo_to_hdr",
				hformatname(histo->pixel_format)));
	}
	return(HIPS_OK);
}
