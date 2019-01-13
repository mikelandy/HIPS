/*
 * This file is considered to be public domain software.  We hereby give
 * permission for anyone to make any use of this code, including copying the
 * code, including it with freely distributed software, including it with
 * commercially available software, and including it in ftp-able code.
 * We do not assert that this software is completely bug-free (although we hope
 * it is), and we do not support the software (officially) in any way.  The
 * intention is to make it possible for people to read and write standard HIPS
 * formatted image sequences, and write conversion programs to other formats,
 * without owning a license for HIPS-proper.  However, we do require that all
 * distributed copies of these source files include the following copyright
 * notice.
 *
 ******************************************************************************
 *
 * Copyright (c) 1992 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 *
 ******************************************************************************
 */

/*
 * hpub_rdhdr.c - HIPS Picture Format Header read (public domain version)
 *
 * Michael Landy - 9/30/92
 * added cmap support - msl - 5/26/95
 * added general xparam support - msl - 10/31/95
 */

#include <hipspub.h>

void hpub_frdhdr(),hpub_frdhdrc(),hpub_frdhdrx(),hpub_frdhdrxr(),hpub_perr();
void hpub_fgets(),hpub_frdoldhdr(),hpub_swallownl();
int hpub_checkparam();

void hpub_rdhdr(format,rows,cols,frames,colors)

int *format,*rows,*cols,*frames,*colors;

{
	hpub_frdhdr(stdin,format,rows,cols,frames,colors);
}

void hpub_rdhdrc(format,rows,cols,frames,colors,ncmap,r,g,b)

int *format,*rows,*cols,*frames,*colors,*ncmap;
unsigned char **r,**g,**b;

{
	hpub_frdhdrc(stdin,format,rows,cols,frames,colors,ncmap,r,g,b);
}

void hpub_rdhdrx(format,rows,cols,frames,colors,xpar)

int *format,*rows,*cols,*frames,*colors;
struct hpub_xparlist *xpar;

{
	hpub_frdhdrx(stdin,format,rows,cols,frames,colors,xpar);
}

void hpub_rdhdrxr(format,rows,cols,frames,colors,roir,roic,frow,fcol,xpar)

int *format,*rows,*cols,*frames,*colors,*roir,*roic,*frow,*fcol;
struct hpub_xparlist *xpar;

{
	hpub_frdhdrxr(stdin,format,rows,cols,frames,colors,roir,roic,frow,fcol,
		xpar);
}

void hpub_frdhdr(fp,format,rows,cols,frames,colors)

FILE *fp;
int *format,*rows,*cols,*frames,*colors;

{
	int ncmap;
	unsigned char *r,*g,*b;

	hpub_frdhdrx(fp,format,rows,cols,frames,colors,
		(struct hpub_xparlist *) 0);
}

void hpub_frdhdrc(fp,format,rows,cols,frames,colors,ncmap,r,g,b)

FILE *fp;
int *format,*rows,*cols,*frames,*colors,*ncmap;
unsigned char **r,**g,**b;

{
	struct hpub_xparlist xpar;
	unsigned char *cmap;
	int count;

	hpub_frdhdrx(fp,format,rows,cols,frames,colors,&xpar);
	if (!hpub_checkparam(&xpar,"cmap")) {
		*ncmap = 0;
		return;
	}
	cmap = hpub_getparamb2(&xpar,"cmap",&count);
	if (count%3)
		hpub_perr("cmap count not a multiple of 3");
	*ncmap = count/3;
	*r = cmap;
	*g = cmap + *ncmap;
	*b = cmap + 2*(*ncmap);
}

void hpub_frdhdrx(fp,format,rows,cols,frames,colors,xpar)

FILE *fp;
int *format,*rows,*cols,*frames,*colors;
struct hpub_xparlist *xpar;

{
	int i;

	hpub_frdhdrxr(fp,format,rows,cols,frames,colors,&i,&i,&i,&i,xpar);
}

void hpub_frdhdrxr(fp,format,rows,cols,frames,colors,roir,roic,frow,fcol,xpar)

FILE *fp;
int *format,*rows,*cols,*frames,*colors,*roir,*roic,*frow,*fcol;
struct hpub_xparlist *xpar;

{
	char inp[LINELENGTH],ptypes[20];
	unsigned char *buf,*pr,*pg,*pb,*pbuf;
	int i,j,sz,np,count,fmt,offset,curroffset,sizebin;
	float f;
	struct hpub_extpar *xp,*lastxp,tmpxp;

	hpub_fgets(inp,LINELENGTH,fp);	/* magic */
	if (strcmp(inp,"HIPS\n") != 0) {
		if (xpar != (struct hpub_xparlist *) 0)
			xpar->numparam = 0;
		hpub_frdoldhdr(fp,format,rows,cols,frames,colors);
		*roir = *rows;
		*roic = *cols;
		*frow = 0;
		*fcol = 0;
		return;
	}
	hpub_fgets(inp,LINELENGTH,fp);	/* onm */
	hpub_fgets(inp,LINELENGTH,fp);	/* snm */
	if (fscanf(fp,"%d",frames) != 1)
		hpub_perr("error reading number of frames");
	hpub_swallownl(fp);
	hpub_fgets(inp,LINELENGTH,fp);	/* odt */
	if (fscanf(fp,"%d",rows) != 1)
		hpub_perr("error reading number of rows");
	if (fscanf(fp,"%d",cols) != 1)
		hpub_perr("error reading number of cols");
	if (fscanf(fp,"%d",roir) != 1)	/* roirows */
		hpub_perr("error reading header");
	if (fscanf(fp,"%d",roic) != 1)	/* roicols */
		hpub_perr("error reading header");
	if (fscanf(fp,"%d",frow) != 1)	/* frow */
		hpub_perr("error reading header");
	if (fscanf(fp,"%d",fcol) != 1)	/* fcol */
		hpub_perr("error reading header");
	if (fscanf(fp,"%d",format) != 1)
		hpub_perr("error reading pixel format");
	if (fscanf(fp,"%d",colors) != 1)
		hpub_perr("error reading number of colors");
	if (fscanf(fp,"%d",&sz) != 1)	/* szhist */
		hpub_perr("error reading header");
	hpub_swallownl(fp);
	if (sz) {
		if ((buf = (unsigned char *) malloc(sz)) == (unsigned char *) 0)
			hpub_perr("error allocating memory to read header");
		if (fread(buf,sz,1,fp) != 1)
			hpub_perr("error reading header");
		free(buf);
	}
	if (fscanf(fp,"%d",&sz) != 1)	/* szdesc */
		hpub_perr("error reading header");
	hpub_swallownl(fp);
	if (sz) {
		if ((buf = (unsigned char *) malloc(sz)) == (unsigned char *) 0)
			hpub_perr("error allocating memory to read header");
		if (fread(buf,sz,1,fp) != 1)
			hpub_perr("error reading header");
		free(buf);
	}
	if (fscanf(fp,"%d",&np) != 1)	/* nparam */
		hpub_perr("error reading header");
	hpub_swallownl(fp);
	if (xpar != (struct hpub_xparlist *) 0) {
		xpar->numparam = np;
		xpar->params = HP_NULLPAR;
	}
	for (i=0;i<np;i++) {
		if (xpar != (struct hpub_xparlist *) 0) {
			if ((xp = (struct hpub_extpar *)
			  malloc(sizeof(struct hpub_extpar)))
			  == HP_NULLPAR)
				hpub_perr("error allocating parameter memory");
			if (i == 0)
				lastxp = xpar->params = xp;
			else {
				lastxp->nextp = xp;
				lastxp = xp;
			}
			xp->nextp = HP_NULLPAR;
		}
		else
			xp = &tmpxp;
		if (fscanf(fp,"%s %s %d",inp,ptypes,&(xp->count)) != 3)
			hpub_perr("header format error");
		xp->name = hpub_strsave(inp);
		switch(ptypes[0]) {
		case 'c': xp->format = PFASCII; break;
		case 'b': xp->format = PFBYTE; break;
		case 'i': xp->format = PFINT; break;
		case 'f': xp->format = PFFLOAT; break;
		case 's': xp->format = PFSHORT; break;
		default: hpub_perr("header format error");
		}
		if (xp->count == 1) {
			switch(xp->format) {
			case PFASCII:
			case PFBYTE:	if (fscanf(fp,"%d",&j) != 1)
						hpub_perr(
							"error reading header");
					xp->val.v_b = j;
					break;
			case PFSHORT:	if (fscanf(fp,"%d",&j) != 1)
						hpub_perr(
							"error reading header");
					xp->val.v_s = j;
					break;
			case PFINT:	if (fscanf(fp,"%d",&j) != 1)
						hpub_perr(
							"error reading header");
					xp->val.v_i = j;
					break;
			case PFFLOAT:	if (fscanf(fp,"%f",&f) != 1)
						hpub_perr(
							"error reading header");
					xp->val.v_f = f;
					break;
			}
		}
		else {
			if (fscanf(fp,"%d",&(xp->offset)) != 1)
				hpub_perr("error reading header");
		}
		hpub_swallownl(fp);
	}
	if (fscanf(fp,"%d",&sizebin) != 1)
		hpub_perr("error reading header");
	hpub_swallownl(fp);
	if (xpar == (struct hpub_xparlist *) 0) {
		if (sizebin == 0)
			return;
		if ((buf = (unsigned char *) malloc(sizebin))
		    == (unsigned char *) 0)
			hpub_perr("error allocating memory to read header");
		if (fread(buf,sizebin,1,fp) != 1)
			hpub_perr("error reading header");
		free(buf);
		return;
	}
	curroffset = 0;
	xp = xpar->params;
	while (xp != HP_NULLPAR) {
		if (xp->count > 1) {
			if (xp->offset != curroffset)
				hpub_perr("bad extended parameter offset");
			switch(xp->format) {
			case PFASCII:
			case PFBYTE:	sz = sizeof(unsigned char); break;
			case PFSHORT:	sz = sizeof(short); break;
			case PFINT:	sz = sizeof(int); break;
			case PFFLOAT:	sz = sizeof(float); break;
			default:	hpub_perr("error - bad format type?");
			}
			i = xp->count * sz;
			i = (i+3) & ~03;
			if ((xp->val.v_pb = (unsigned char *) malloc(i)) ==
			    (unsigned char *) 0)
				hpub_perr("can't allocate parameter memory");
			if (fread(xp->val.v_pb,i,1,fp) != 1)
				hpub_perr("error reading parameter array");
			curroffset += i;
		}
		xp = xp->nextp;
	}
	if (curroffset != sizebin)
		hpub_perr("error with parameter offsets");
}

/*
 * hpub_frdoldhdr - read old (HIPS-1) format header (public domain version)
 *
 * Michael Landy - 9/30/92
 */

void hpub_frdoldhdr(fp,format,rows,cols,frames,colors)

FILE *fp;
int *format,*rows,*cols,*frames,*colors;

{
	char inp[LINELENGTH];
	int i,bpck;

	/* onm already read */
	hpub_fgets(inp,LINELENGTH,fp);	/* snm */
	if (fscanf(fp,"%d",frames) != 1)
		hpub_perr("error reading number of frames");
	hpub_swallownl(fp);
	hpub_fgets(inp,LINELENGTH,fp);	/* odt */
	if (fscanf(fp,"%d",rows) != 1)
		hpub_perr("error reading number of rows");
	if (fscanf(fp,"%d",cols) != 1)
		hpub_perr("error reading number of cols");
	if (fscanf(fp,"%d",&i) != 1)	/* bpp */
		hpub_perr("error reading header");
	if (fscanf(fp,"%d",&bpck) != 1)	/* bpck */
		hpub_perr("error reading header");
	if (fscanf(fp,"%d",format) != 1)
		hpub_perr("error reading pixel format");
	hpub_swallownl(fp);
	if (*format == PFBYTE && bpck == MSBFIRST)
		*format = PFMSBF;
	else if (*format == PFBYTE && bpck == LSBFIRST)
		*format = PFLSBF;
	*colors = 1;
	hpub_fgets(inp,LINELENGTH,fp);	/* hist */
	while(inp[strlen(inp)-3] == '|')
		hpub_fgets(inp,LINELENGTH,fp);
	while (1) {	/* desc */
		hpub_fgets(inp,LINELENGTH,fp);
		if (strcmp(inp,".\n") == 0)
			return;
	}
}

void hpub_swallownl(fp)

FILE *fp;

{
	int i;

	while ((i = getc(fp)) != '\n') {
		if (i == EOF)
			hpub_perr("error reading input end-of-line");
	}
}

void hpub_fgets(s,n,fp)

char *s;
int n;
FILE *fp;

{
	int i;

	fgets(s,n,fp);
	i=strlen(s);
	if (i==0 || s[i-1]!='\n')
		hpub_perr("error reading input line");
}

char *hpub_strsave(s)

char *s;

{
	char *news;

	if ((news = (char *) malloc(strlen(s)+1)) == (char *) 0)
		hpub_perr("hpub_strsave: error allocating memory");
	return(strcpy(news,s));
}
