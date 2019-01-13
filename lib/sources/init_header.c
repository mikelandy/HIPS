/*
 *	Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * init_header.c - HIPL image header initialization
 *
 * Michael Landy - 2/1/82
 * HIPS 2 version - 1/3/91
 */

#include <hipl_format.h>

int init_header(hd,onm,snm,nfr,odt,rw,cl,pfmt,nc,desc)

struct header *hd;
char *onm,*snm,*odt,*desc;
int nfr,rw,cl,pfmt,nc;

{
	hd->orig_name = strsave(onm);
	hd->ondealloc = TRUE;
	hd->seq_name = strsave(snm);
	hd->sndealloc = TRUE;
	hd->num_frame = nfr;
	hd->orig_date = strsave(odt);
	hd->oddealloc = TRUE;
	hd->orows = hd->rows = rw;
	hd->ocols = hd->cols = cl;
	hd->frow = hd->fcol = 0;
	hd->pixel_format = pfmt;
	hd->numcolor = nc;
	hd->numpix = rw*cl;
	hd->sizepix = hsizepix(pfmt);
	hd->sizeimage = hd->sizepix * hd->numpix;
	if (pfmt == PFMSBF || pfmt == PFLSBF)
		hd->sizeimage = rw * ((cl+7) / 8) * sizeof(byte);
	hd->imdealloc = FALSE;
	hd->sizehist = 1;
	hd->seq_history = " ";
	hd->histdealloc = FALSE;
	hd->sizedesc = strlen(desc);
	hd->seq_desc = strsave(desc);
	hd->seqddealloc = TRUE;
	hd->numparam = 0;
	hd->paramdealloc = TRUE;
	hd->params = NULLPAR;
	return(HIPS_OK);
}

int init_hdr_alloc(hd,onm,snm,nfr,odt,rw,cl,pfmt,nc,desc)

struct header *hd;
char *onm,*snm,*odt,*desc;
int nfr,rw,cl,pfmt,nc;

{
	init_header(hd,onm,snm,nfr,odt,rw,cl,pfmt,nc,desc);
	return(alloc_image(hd));
}

int init_header_d(hd,onm,onmd,snm,snmd,nfr,odt,odtd,rw,cl,pfmt,nc,desc,descd)

struct header *hd;
char *onm,*snm,*odt,*desc;
int nfr,rw,cl,pfmt,nc;
h_boolean onmd,snmd,odtd,descd;

{
	hd->orig_name = onm;
	hd->ondealloc = onmd;
	hd->seq_name = snm;
	hd->sndealloc = snmd;
	hd->num_frame = nfr;
	hd->orig_date = odt;
	hd->oddealloc = odtd;
	hd->orows = hd->rows = rw;
	hd->ocols = hd->cols = cl;
	hd->frow = hd->fcol = 0;
	hd->pixel_format = pfmt;
	hd->numcolor = nc;
	hd->numpix = rw*cl;
	hd->sizepix = hsizepix(pfmt);
	hd->sizeimage = hd->sizepix * hd->numpix;
	if (pfmt == PFMSBF || pfmt == PFLSBF)
		hd->sizeimage = rw * ((cl+7) / 8) * sizeof(byte);
	hd->imdealloc = FALSE;
	hd->sizehist = 1;
	hd->seq_history = " ";
	hd->histdealloc = FALSE;
	hd->sizedesc = strlen(desc);
	hd->seq_desc = desc;
	hd->seqddealloc = descd;
	hd->numparam = 0;
	hd->paramdealloc = TRUE;
	hd->params = NULLPAR;
	return(HIPS_OK);
}

int init_hdr_alloc_d(hd,onm,onmd,snm,snmd,nfr,odt,odtd,rw,cl,pfmt,nc,desc,descd)

struct header *hd;
char *onm,*snm,*odt,*desc;
int nfr,rw,cl,pfmt,nc;
h_boolean onmd,snmd,odtd,descd;

{
	init_header_d(hd,onm,onmd,snm,snmd,nfr,odt,odtd,rw,cl,pfmt,nc,desc,
		descd);
	return(alloc_image(hd));
}
