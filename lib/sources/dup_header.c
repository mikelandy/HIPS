/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * dup_header.c - copy an image header
 *
 * It is assumed that the second image header will be primary, and so it
 * gains all deallocation rights.
 *
 * Michael Landy - 1/4/91
 */

#include <stdio.h>
#include <hipl_format.h>

int dup_header(hd1,hd2)

struct header *hd1,*hd2;

{
	hd2->orig_name = hd1->orig_name;
	hd2->ondealloc = hd1->ondealloc;
	hd1->ondealloc = FALSE;
	hd2->seq_name = hd1->seq_name;
	hd2->sndealloc = hd1->sndealloc;
	hd1->sndealloc = FALSE;
	hd2->num_frame = hd1->num_frame;
	hd2->orig_date = hd1->orig_date;
	hd2->oddealloc = hd1->oddealloc;
	hd1->oddealloc = FALSE;
	hd2->orows = hd1->orows;
	hd2->ocols = hd1->ocols;
	hd2->rows = hd1->rows;
	hd2->cols = hd1->cols;
	hd2->frow = hd1->frow;
	hd2->fcol = hd1->fcol;
	hd2->pixel_format = hd1->pixel_format;
	hd2->numcolor = hd1->numcolor;
	hd2->numpix = hd1->numpix;
	hd2->sizepix = hd1->sizepix;
	hd2->sizeimage = hd1->sizeimage;
	hd2->image = hd1->image;
	hd2->imdealloc = hd1->imdealloc;
	hd1->imdealloc = FALSE;
	hd2->firstpix = hd1->firstpix;
	hd2->sizehist = hd1->sizehist;
	hd2->seq_history = hd1->seq_history;
	hd2->histdealloc = hd1->histdealloc;
	hd1->histdealloc = FALSE;
	hd2->sizedesc = hd1->sizedesc;
	hd2->seq_desc = hd1->seq_desc;
	hd2->seqddealloc = hd1->seqddealloc;
	hd1->seqddealloc = FALSE;
	hd2->numparam = hd1->numparam;
	hd2->paramdealloc = hd1->paramdealloc;
	hd1->paramdealloc = FALSE;
	hd2->params = hd1->params;
	return(HIPS_OK);
}

int dup_headern(hd1,hd2)

struct header *hd1,*hd2;

{
	hd2->orig_name = hd1->orig_name;
	hd2->ondealloc = hd1->ondealloc;
	hd1->ondealloc = FALSE;
	hd2->seq_name = hd1->seq_name;
	hd2->sndealloc = hd1->sndealloc;
	hd1->sndealloc = FALSE;
	hd2->num_frame = hd1->num_frame;
	hd2->orig_date = hd1->orig_date;
	hd2->oddealloc = hd1->oddealloc;
	hd1->oddealloc = FALSE;
	hd2->orows = hd1->orows;
	hd2->ocols = hd1->ocols;
	hd2->rows = hd1->rows;
	hd2->cols = hd1->cols;
	hd2->frow = hd1->frow;
	hd2->fcol = hd1->fcol;
	hd2->pixel_format = hd1->pixel_format;
	hd2->numcolor = hd1->numcolor;
	hd2->numpix = hd1->numpix;
	hd2->sizepix = hd1->sizepix;
	hd2->sizeimage = hd1->sizeimage;
	hd2->image = 0;
	hd2->imdealloc = FALSE;
	hd2->firstpix = 0;
	hd2->sizehist = hd1->sizehist;
	hd2->seq_history = hd1->seq_history;
	hd2->histdealloc = hd1->histdealloc;
	hd1->histdealloc = FALSE;
	hd2->sizedesc = hd1->sizedesc;
	hd2->seq_desc = hd1->seq_desc;
	hd2->seqddealloc = hd1->seqddealloc;
	hd1->seqddealloc = FALSE;
	hd2->numparam = hd1->numparam;
	hd2->paramdealloc = hd1->paramdealloc;
	hd1->paramdealloc = FALSE;
	hd2->params = hd1->params;
	return(HIPS_OK);
}
