/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * read_header.c - HIPS Picture Format Header read
 *
 * Michael Landy - 2/1/82
 * modified to use read/write 4/26/82
 * modified for HIPS2 1/3/91
 */

#include <stdio.h>
#include <hipl_format.h>

int read_header(hd)

struct header *hd;

{
	return(fread_header(stdin,hd,"<stdin>"));
}

int fread_header(fp,hd,fname)

FILE *fp;
struct header *hd;
Filename fname;

{
	char inp[LINELENGTH],ptypes[20];
	byte *binarea;
	int i,v,sizebin,toplev,curroffset,one=1;
	struct extpar *xp,*lastxp;

	hips_oldhdr = FALSE;
	if (hfgets(inp,LINELENGTH,fp)) return(perr(HE_HDRREAD,fname));
	if (strcmp(inp,"HIPS\n")!=0)
		return(fread_oldhdr(fp,hd,inp,fname));
	if (hfgets(inp,LINELENGTH,fp)) return(perr(HE_HDRREAD,fname));
	hd->orig_name = strsave(inp);
	hd->ondealloc = TRUE;
	if (hfgets(inp,LINELENGTH,fp)) return(perr(HE_HDRREAD,fname));
	hd->seq_name = strsave(inp);
	hd->sndealloc = TRUE;
	if (fscanf(fp,"%d",&(hd->num_frame)) != 1)
		return(perr(HE_HDRREAD,fname));
	if (swallownl(fp))
		return(perr(HE_HDRREAD,fname));
	if (hfgets(inp,LINELENGTH,fp)) return(perr(HE_HDRREAD,fname));
	hd->orig_date = strsave(inp);
	hd->oddealloc = TRUE;
	if (fscanf(fp,"%d",&(hd->orows)) != 1) return(perr(HE_HDRREAD,fname));
	if (fscanf(fp,"%d",&(hd->ocols)) != 1) return(perr(HE_HDRREAD,fname));
	if (fscanf(fp,"%d",&(hd->rows)) != 1) return(perr(HE_HDRREAD,fname));
	if (fscanf(fp,"%d",&(hd->cols)) != 1) return(perr(HE_HDRREAD,fname));
	if (fscanf(fp,"%d",&(hd->frow)) != 1) return(perr(HE_HDRREAD,fname));
	if (fscanf(fp,"%d",&(hd->fcol)) != 1) return(perr(HE_HDRREAD,fname));
	if (fscanf(fp,"%d",&(hd->pixel_format)) != 1)
		return(perr(HE_HDRREAD,fname));
	if ((hd->pixel_format == PFMSBF || hd->pixel_format == PFLSBF) &&
		(hd->fcol % 8 != 0))
			return(perr(HE_ROI8F,"fread_header",fname));
	if (fscanf(fp,"%d",&(hd->numcolor)) != 1)
		return(perr(HE_HDRREAD,fname));
	hd->numpix = hd->orows * hd->ocols;
	hd->sizepix = hsizepix(hd->pixel_format);
	hd->sizeimage = hd->sizepix * hd->numpix;
	if (hd->pixel_format == PFMSBF || hd->pixel_format == PFLSBF)
		hd->sizeimage = hd->orows * ((hd->ocols+7) / 8) * sizeof(byte);
	hd->imdealloc = FALSE;
	if (fscanf(fp,"%d",&(hd->sizehist)) != 1)
		return(perr(HE_HDRREAD,fname));
	if (swallownl(fp))
		return(perr(HE_HDRREAD,fname));
	if ((hd->seq_history = (char *) malloc(1 + (hd->sizehist)))
		== (char *) 0)
			return(perr(HE_ALLOCSUBR,"fread_header"));
	if (hd->sizehist) {
		if (fread(hd->seq_history,hd->sizehist,1,fp) != 1)
			return(perr(HE_HDRREAD,fname));
	}
	hd->seq_history[hd->sizehist] = '\0';
	hd->histdealloc = TRUE;
	if (fscanf(fp,"%d",&(hd->sizedesc)) != 1)
		return(perr(HE_HDRREAD,fname));
	if (swallownl(fp))
		return(perr(HE_HDRREAD,fname));
	if ((hd->seq_desc = (char *) malloc(1 + (hd->sizedesc))) == (char *) 0)
		return(perr(HE_ALLOCSUBR,"fread_header"));
	if (hd->sizedesc) {
		if (fread(hd->seq_desc,hd->sizedesc,1,fp) != 1)
			return(perr(HE_HDRREAD,fname));
	}
	hd->seq_desc[hd->sizedesc] = '\0';
	hd->seqddealloc = TRUE;
	if (fscanf(fp,"%d",&(hd->numparam)) != 1)
		return(perr(HE_HDRREAD,fname));
	if (swallownl(fp))
		return(perr(HE_HDRREAD,fname));
	hd->paramdealloc = TRUE;
	hd->params = NULLPAR;
	for (i=0;i<hd->numparam;i++) {
		if ((xp = (struct extpar *) malloc(sizeof(struct extpar)))
			== NULLPAR)
				return(perr(HE_ALLOCSUBR,"fread_header"));
		if (i==0)
			lastxp = hd->params = xp;
		else {
			lastxp->nextp = xp;
			lastxp = xp;
		}
		xp->nextp = NULLPAR;
		if (fscanf(fp,"%s %s %d",inp,ptypes,&(xp->count)) != 3)
			return(perr(HE_HDRPREAD,fname));
		xp->name = strsave(inp);
		switch(ptypes[0]) {
		case 'c': xp->format = PFASCII; break;
		case 'b': xp->format = PFBYTE; break;
		case 'i': xp->format = PFINT; break;
		case 'f': xp->format = PFFLOAT; break;
		case 's': xp->format = PFSHORT; break;
		default: return(perr(HE_HDRPTYPES,ptypes,fname));
		}
		if (xp->count == 1) {
			switch(xp->format) {
			case PFASCII:
			case PFBYTE:	if (fscanf(fp,"%d",&v) != 1)
						return(perr(HE_HDRPREAD,fname));
					xp->val.v_b = v;
					break;
			case PFSHORT:	if (fscanf(fp,"%d",&v) != 1)
						return(perr(HE_HDRPREAD,fname));
					xp->val.v_s = v;
					break;
			case PFINT:	if (fscanf(fp,"%d",&(xp->val.v_i))
					    != 1)
						return(perr(HE_HDRPREAD,fname));
					break;
			case PFFLOAT:	if (fscanf(fp,"%f",&(xp->val.v_f))
					    != 1)
						return(perr(HE_HDRPREAD,fname));
					break;
			}
		}
		else {	/*  *** temporarily store offset in dealloc *** */
			if (fscanf(fp,"%d",&(xp->dealloc)) != 1)
				return(perr(HE_HDRPREAD,fname));
		}
		if (swallownl(fp))
			return(perr(HE_HDRREAD,fname));
	}
	if (fscanf(fp,"%d",&sizebin) != 1) return(perr(HE_HDRREAD,fname));
	if (swallownl(fp))
		return(perr(HE_HDRREAD,fname));
	curroffset = 0;
	xp = hd->params;
	while (xp != NULLPAR) {
		if (xp->count > 1) {
			if (xp->dealloc != curroffset)
				return(perr(HE_XINC,xp->name,xp->format,
				    xp->count,xp->dealloc,curroffset,fname));
			i = xp->count *
				((xp->format == PFASCII) ? sizeof(char) :
				hsizepix(xp->format));
			i = (i+3) & ~03;
			if ((xp->val.v_pb = (byte *) malloc(i)) == (byte *) 0)
				return(perr(HE_ALLOCSUBR,"fread_header"));
			xp->dealloc = TRUE;
			if (fread(xp->val.v_pb,i,1,fp) != 1)
				return(perr(HE_HDRBREAD,fname));
			curroffset += i;
		}
		else
			xp->dealloc = FALSE;
		xp = xp->nextp;
	}
	if (curroffset != sizebin)
		return(perr(HE_HDRXOV,fname,sizebin,curroffset));
	if (hd->pixel_format == PFINTPYR || hd->pixel_format == PFFLOATPYR) {
		if (getparam(hd,"toplev",PFINT,&one,&toplev) == HIPS_ERROR)
			return(HIPS_ERROR);
		hd->numpix = pyrnumpix(toplev,hd->rows,hd->cols);
		hd->sizeimage = hd->numpix * hd->sizepix;
	}
	return(HIPS_OK);
}
