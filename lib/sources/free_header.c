/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * free_header.c - HIPS image header deallocation
 *
 * Michael Landy 1/4/91
 */

#include <hipl_format.h>

int free_header(hd)

struct header *hd;

{
	free_hdrcon(hd);
	free(hd);
	return(HIPS_OK);
}

int free_hdrcon(hd)

struct header *hd;

{
	if (hd->ondealloc)
		free(hd->orig_name);
	hd->orig_name = (char *) 0;
	hd->ondealloc = FALSE;
	if (hd->sndealloc)
		free(hd->seq_name);
	hd->seq_name = (char *) 0;
	hd->sndealloc = FALSE;
	if (hd->oddealloc)
		free(hd->orig_date);
	hd->orig_date = (char *) 0;
	hd->oddealloc = FALSE;
	if (hd->imdealloc)
		free(hd->image);
	hd->image = (byte *) 0;
	hd->imdealloc = FALSE;
	if (hd->histdealloc && hd->seq_history)
		free(hd->seq_history);
	hd->seq_history = (char *) 0;
	hd->histdealloc = FALSE;
	if (hd->seqddealloc && hd->seq_desc)
		free(hd->seq_desc);
	hd->seq_desc = (char *) 0;
	hd->seqddealloc = FALSE;
	if (hd->paramdealloc) {
		struct extpar *xp,*nextxp;

		xp = hd->params;
		while (xp != NULLPAR) {
			if (xp->dealloc && xp->val.v_pb)
				free(xp->val.v_pb);
			free(xp->name);
			nextxp = xp->nextp;
			free(xp);
			xp = nextxp;
		}
	}
	hd->params = NULLPAR;
	hd->paramdealloc = FALSE;
	hd->numparam = 0;
	return(HIPS_OK);
}
