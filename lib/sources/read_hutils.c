/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * read_hutils.c - HIPS Picture Format Header read utilities
 *
 * Michael Landy - 1/4/91
 */

#include <hipl_format.h>
#include <stdio.h>

int nulllist[] = {LASTTYPE};

int read_hdr_a(hd)

struct header *hd;

{
	return(fread_hdr_cpfac(stdin,hd,nulllist,"<stdin>",0,1));
}

int fread_hdr_a(fp,hd,fname)

FILE *fp;
struct header *hd;
Filename fname;

{
	return(fread_hdr_cpfac(fp,hd,nulllist,fname,0,1));
}

int read_hdr_cpf(hd,typelist)

struct header *hd;
int *typelist;

{
	return(fread_hdr_cpfac(stdin,hd,typelist,"<stdin>",1,0));
}

int fread_hdr_cpf(fp,hd,typelist,fname)

FILE *fp;
struct header *hd;
int *typelist;
Filename fname;

{
	return(fread_hdr_cpfac(fp,hd,typelist,fname,1,0));
}

int read_hdr_cpfa(hd,typelist)

struct header *hd;
int *typelist;

{
	return(fread_hdr_cpfac(stdin,hd,typelist,"<stdin>",1,1));
}

int fread_hdr_cpfa(fp,hd,typelist,fname)

FILE *fp;
struct header *hd;
int *typelist;
Filename fname;

{
	return(fread_hdr_cpfac(fp,hd,typelist,fname,1,1));
}

int fread_hdr_cpfac(fp,hd,typelist,fname,flagc,flaga)

FILE *fp;
struct header *hd;
int *typelist,flagc,flaga;
Filename fname;

{
	int *tp;

	if (fread_header(fp,hd,fname) == HIPS_ERROR)
		return(HIPS_ERROR);
	if (flagc) {
		tp = typelist;
		while (*tp != LASTTYPE) {
			if (hd->pixel_format == *tp)
				break;
			tp++;
		}
		if (*tp == LASTTYPE)
			return(perr(HE_FMTFILE,hformatname(hd->pixel_format),
				fname));
	}
	if (flaga) {
		if (alloc_image(hd) == HIPS_ERROR)
			return(HIPS_ERROR);
	}
	return(HIPS_OK);
}

int read_hdr_cc(hd,chd,mask)

struct header *hd,*chd;
int mask;

{
	return(fread_hdr_ccac(stdin,hd,chd,mask,"<stdin>",0));
}

int fread_hdr_cc(fp,hd,chd,mask,fname)

FILE *fp;
struct header *hd,*chd;
int mask;
Filename fname;

{
	return(fread_hdr_ccac(fp,hd,chd,mask,fname,0));
}

int read_hdr_cca(hd,chd,mask)

struct header *hd,*chd;
int mask;

{
	return(fread_hdr_ccac(stdin,hd,chd,mask,"<stdin>",1));
}

int fread_hdr_cca(fp,hd,chd,mask,fname)

FILE *fp;
struct header *hd,*chd;
int mask;
Filename fname;

{
	return(fread_hdr_ccac(fp,hd,chd,mask,fname,1));
}

int fread_hdr_ccac(fp,hd,chd,mask,fname,flaga)

FILE *fp;
struct header *hd,*chd;
int mask,flaga;
Filename fname;

{
	int one=1;
	int nc1,nc2,nf1,nf2,nd1,nd2;

	if (fread_header(fp,hd,fname) == HIPS_ERROR)
		return(HIPS_ERROR);
	if (mask & CM_ROWS) {
		if (hd->rows != chd->rows)
			return(perr(HE_C_ROW,fname));
	}
	if (mask & CM_COLS) {
		if (hd->cols != chd->cols)
			return(perr(HE_C_COL,fname));
	}
	if (mask & CM_FRAMES) {
		if ((hd->num_frame/hd->numcolor) !=
		    (chd->num_frame/chd->numcolor))
			return(perr(HE_C_FRM,fname));
		if (hgetdepth(hd) != hgetdepth(chd))
			return(perr(HE_C_DEPTH,fname));
	}
	if (mask & CM_FRAMESC) {
		nf1 = hd->num_frame;
		if (type_is_col3(hd))
			nf1 *= 3;
		nf2 = chd->num_frame;
		if ((nd1 = hgetdepth(hd)) == HIPS_ERROR)
			return(HIPS_ERROR);
		if ((nd2 = hgetdepth(chd)) == HIPS_ERROR)
			return(HIPS_ERROR);
		if (type_is_col3(chd))
			nf2 *= 3;
		if ((hd->numcolor != 1 || chd->numcolor != 1 ||
		     type_is_col3(hd) || type_is_col3(chd) ||
		     nd1 != 1 || nd2 != 1)) {
			if (nf1 != nf2)
				return(perr(HE_C_FRMC,fname));
			if (nd1 != nd2)
				return(perr(HE_C_DPTHC,fname));
		}
	}
	if (mask & CM_DEPTH) {
		if (hgetdepth(hd) != hgetdepth(chd))
			return(perr(HE_C_DEPTH,fname));
	}
	if (mask & CM_FORMAT) {
		if (hd->pixel_format != chd->pixel_format)
			return(perr(HE_C_FMT,hformatname(hd->pixel_format),
				fname));
	}
	if (mask & CM_NUMCOLOR) {
		if (hd->numcolor != chd->numcolor)
			return(perr(HE_C_NCL,fname));
	}
	if (mask & CM_NUMCOLOR3) {
		nc1 = hd->numcolor;
		nc2 = chd->numcolor;
		if (type_is_col3(hd)) {
			if (nc1 != 1)
				return(perr(HE_COL1,fname));
			nc1 = 3;
		}
		if (type_is_col3(chd)) {
			if (nc2 != 1)
				return(perr(HE_COL1,"??comparison file"));
			nc2 = 3;
		}
		if (nc1 != nc2)
			return(perr(HE_C_NCL,fname));
	}
	if (mask & CM_NUMLEV) {
		if ((hd->pixel_format == PFINTPYR ||
		     hd->pixel_format == PFFLOATPYR) &&
		    (chd->pixel_format == PFINTPYR ||
		     chd->pixel_format == PFFLOATPYR)) {
			int nl,nlc;
			if (getparam(hd,"toplev",PFINT,&one,&nl) == HIPS_ERROR)
				return(HIPS_ERROR);
			if (getparam(chd,"toplev",PFINT,&one,&nlc)
			    == HIPS_ERROR)
				return(HIPS_ERROR);
			if (nl != nlc)
				return(perr(HE_C_NLV,fname));
		}
	}
	if (mask & CM_OROWS) {
		if (hd->orows != chd->orows)
			return(perr(HE_C_OROW,fname));
	}
	if (mask & CM_OCOLS) {
		if (hd->ocols != chd->ocols)
			return(perr(HE_C_OCOL,fname));
	}
	if (flaga) {
		if (alloc_image(hd) == HIPS_ERROR)
			return(HIPS_ERROR);
	}
	return(HIPS_OK);
}
