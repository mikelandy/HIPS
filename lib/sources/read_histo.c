/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * read_histo.c - read a histogram frame
 *
 * Michael Landy - 6/30/91
 */

#include <stdio.h>
#include <hipl_format.h>

int read_histo(histo,fr)

struct hips_histo *histo;
int fr;

{
	return(fread_histo(stdin,histo,fr,"<stdin>"));
}

int fread_histo(fp,histo,fr,fname)

FILE *fp;
struct hips_histo *histo;
int fr;
Filename fname;

{
	if (hips_oldhdr) {
		histo->histo[0] = histo->histo[1+histo->nbins] = 0;
		if (fread(&(histo->histo[1]),(histo->nbins)*sizeof(int),1,fp)
		    != 1)
			return(perr(HE_READFRFILE,fr,fname));
	}
	else {
		if (fread(histo->histo,histo->sizehist,1,fp) != 1)
			return(perr(HE_READFRFILE,fr,fname));
	}
	return(HIPS_OK);
}

int hdr_to_histo(hd,histo)

struct header *hd;
struct hips_histo *histo;

{
	int i,one=1;
	float f;

	if (getparam(hd,"numbin",PFINT,&one,&(histo->nbins)) == HIPS_ERROR)
		return(HIPS_ERROR);
	if (getparam(hd,"imagepixfmt",PFINT,&one,&(histo->pixel_format)) ==
	    HIPS_ERROR)
		return(HIPS_ERROR);
	histo->histodealloc = FALSE;
	switch(histo->pixel_format) {
	case PFBYTE:	if (getparam(hd,"binleft",PFINT,&one,&i) == HIPS_ERROR)
				return(HIPS_ERROR);
			histo->minbin.v_byte = i;
			if (getparam(hd,"binwidth",PFINT,&one,&i) == HIPS_ERROR)
				return(HIPS_ERROR);
			histo->binwidth.v_byte = i;
			break;
	case PFSBYTE:	if (getparam(hd,"binleft",PFINT,&one,&i) == HIPS_ERROR)
				return(HIPS_ERROR);
			histo->minbin.v_sbyte = i;
			if (getparam(hd,"binwidth",PFINT,&one,&i) == HIPS_ERROR)
				return(HIPS_ERROR);
			histo->binwidth.v_sbyte = i;
			break;
	case PFSHORT:	if (getparam(hd,"binleft",PFINT,&one,&i) == HIPS_ERROR)
				return(HIPS_ERROR);
			histo->minbin.v_short = i;
			if (getparam(hd,"binwidth",PFINT,&one,&i) == HIPS_ERROR)
				return(HIPS_ERROR);
			histo->binwidth.v_short = i;
			break;
	case PFUSHORT:	if (getparam(hd,"binleft",PFINT,&one,&i) == HIPS_ERROR)
				return(HIPS_ERROR);
			histo->minbin.v_ushort = i;
			if (getparam(hd,"binwidth",PFINT,&one,&i) == HIPS_ERROR)
				return(HIPS_ERROR);
			histo->binwidth.v_ushort = i;
			break;
	case PFINT:	if (getparam(hd,"binleft",PFINT,&one,
			    &(histo->minbin.v_int)) == HIPS_ERROR)
				return(HIPS_ERROR);
			if (getparam(hd,"binwidth",PFINT,&one,
			    &(histo->binwidth.v_int)) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	case PFUINT:	if (getparam(hd,"binleft",PFINT,&one,&i) == HIPS_ERROR)
				return(HIPS_ERROR);
			histo->minbin.v_uint = i;
			if (getparam(hd,"binwidth",PFINT,&one,&i) == HIPS_ERROR)
				return(HIPS_ERROR);
			histo->binwidth.v_uint = i;
			break;
	case PFFLOAT:
	case PFCOMPLEX:	if (getparam(hd,"binleft",PFFLOAT,&one,
			    &(histo->minbin.v_float)) == HIPS_ERROR)
				return(HIPS_ERROR);
			if (getparam(hd,"binwidth",PFFLOAT,&one,
			    &(histo->binwidth.v_float)) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	case PFDOUBLE:
	case PFDBLCOM:	if (getparam(hd,"binleft",PFFLOAT,&one,&f)
			    == HIPS_ERROR)
				return(HIPS_ERROR);
			histo->minbin.v_double = f;
			if (getparam(hd,"binleft",PFFLOAT,&one,&f)
			    == HIPS_ERROR)
				return(HIPS_ERROR);
			histo->binwidth.v_double = f;
			break;
	default:	return(perr(HE_FMTSUBR,"histo_to_hdr",
				hformatname(histo->pixel_format)));
	}
	return(HIPS_OK);
}
