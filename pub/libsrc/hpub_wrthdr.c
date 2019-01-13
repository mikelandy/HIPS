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
 * hpub_wrthdr.c - HIPS image header write (public domain version)
 *
 * Michael Landy - 9/30/92
 */

#include <hipspub.h>

void hpub_fwrthdr(),hpub_fwrthdrc(),hpub_fwrthdrx(),hpub_fwrthdrxr();
void hpub_perr(),hpub_setparamb2(),hpub_fwrthdr1();
int hpub_wnocr(),hpub_dfprintf();

void hpub_wrthdr(format,rows,cols,frames,colors)

int format,rows,cols,frames,colors;

{
	hpub_fwrthdr(stdout,format,rows,cols,frames,colors);
}

void hpub_wrthdrc(format,rows,cols,frames,colors,ncmap,r,g,b)

int format,rows,cols,frames,colors,ncmap;
unsigned char *r,*g,*b;

{
	hpub_fwrthdrc(stdout,format,rows,cols,frames,colors,ncmap,r,g,b);
}

void hpub_wrthdrx(format,rows,cols,frames,colors,xpar)

int format,rows,cols,frames,colors;
struct hpub_xparlist *xpar;

{
	hpub_fwrthdrx(stdout,format,rows,cols,frames,colors,xpar);
}

void hpub_wrthdrxr(format,rows,cols,frames,colors,roir,roic,frow,fcol,xpar)

int format,rows,cols,frames,colors,roir,roic,frow,fcol;
struct hpub_xparlist *xpar;

{
	hpub_fwrthdrxr(stdout,format,rows,cols,frames,colors,roir,roic,frow,
		fcol,xpar);
}

void hpub_fwrthdr(fp,format,rows,cols,frames,colors)

FILE *fp;
int format,rows,cols,frames,colors;

{
	struct hpub_xparlist xpar;

	xpar.numparam = 0;
	xpar.params = HP_NULLPAR;
	hpub_fwrthdrx(fp,format,rows,cols,frames,colors,&xpar);
}

void hpub_fwrthdrc(fp,format,rows,cols,frames,colors,ncmap,r,g,b)

FILE *fp;
int format,rows,cols,frames,colors,ncmap;
unsigned char *r,*g,*b;

{
	struct hpub_xparlist xpar;
	unsigned char *cmap;
	int i;

	xpar.numparam = 0;
	xpar.params = HP_NULLPAR;
	if ((cmap = (unsigned char *) malloc(3*ncmap)) == (unsigned char *) 0)
		hpub_perr("can't allocate cmap memory");
	for (i=0;i<ncmap;i++) {
		cmap[i] = r[i];
		cmap[ncmap+i] = g[i];
		cmap[2*ncmap+i] = b[i];
	}
	hpub_setparamb2(&xpar,"cmap",ncmap*3,cmap);
	hpub_fwrthdrx(fp,format,rows,cols,frames,colors,&xpar);
}

void hpub_fwrthdrx(fp,format,rows,cols,frames,colors,xpar)

FILE *fp;
int format,rows,cols,frames,colors;
struct hpub_xparlist *xpar;

{
	hpub_fwrthdrxr(fp,format,rows,cols,frames,colors,rows,cols,0,0,xpar);
}

void hpub_fwrthdrxr(fp,format,rows,cols,frames,colors,roir,roic,frow,fcol,xpar)

FILE *fp;
int format,rows,cols,frames,colors,roir,roic,frow,fcol;
struct hpub_xparlist *xpar;

{
	int i,j,offset;
	char s[LINELENGTH];
	struct hpub_extpar *xp;

	fprintf(fp,"HIPS\n");
	i = 5;
	i += hpub_wnocr(fp,"");	/* onm */
	i += hpub_wnocr(fp,""); /* snm */
	i += hpub_dfprintf(fp,frames);
	i += hpub_wnocr(fp,"");
	i += hpub_dfprintf(fp,rows);	/* orows */
	i += hpub_dfprintf(fp,cols);	/* ocols */
	i += hpub_dfprintf(fp,roir);	/* ROI rows */
	i += hpub_dfprintf(fp,roic);	/* ROI cols */
	i += hpub_dfprintf(fp,frow);	/* frow */
	i += hpub_dfprintf(fp,fcol);	/* fcol */
	i += hpub_dfprintf(fp,format);
	i += hpub_dfprintf(fp,colors);
	i += hpub_dfprintf(fp,13);	/* szhist */
	i += 13;
	if (fwrite("Fake History\n",13,1,fp) != 1)
		hpub_perr("error writing hips header");
	i += hpub_dfprintf(fp,17);	/* szdesc */
	i += 17;
	if (fwrite("Fake Description\n",17,1,fp) != 1)
		hpub_perr("error writing hips header");
	i += hpub_dfprintf(fp,xpar->numparam);
	xp = xpar->params;
	offset = 0;
	while (xp != HP_NULLPAR) {
		if (xp->count == 1) {
			switch (xp->format) {
			case PFASCII:	sprintf(s,"%s c 1 %d\n",xp->name,
						(int) xp->val.v_b); break;
			case PFBYTE:	sprintf(s,"%s b 1 %d\n",xp->name,
						(int) xp->val.v_b); break;
			case PFSHORT:	sprintf(s,"%s s 1 %d\n",xp->name,
						(int) xp->val.v_s); break;
			case PFINT:	sprintf(s,"%s i 1 %d\n",xp->name,
						xp->val.v_i); break;
			case PFFLOAT:	sprintf(s,"%s f 1 %f\n",xp->name,
						xp->val.v_f); break;
			default:	hpub_perr(
					"hpub_wrthdr - illegal parameter type");
			}
		}
		else {
			switch (xp->format) {
			case PFASCII:	sprintf(s,"%s c %d %d\n",xp->name,
						xp->count,offset);
					offset += xp->count *
						sizeof(unsigned char);
					break;
			case PFBYTE:	sprintf(s,"%s b %d %d\n",xp->name,
						xp->count,offset);
					offset += xp->count *
						sizeof(unsigned char);
					break;
			case PFSHORT:	sprintf(s,"%s s %d %d\n",xp->name,
						xp->count,offset);
					offset += xp->count * sizeof(short);
					break;
			case PFINT:	sprintf(s,"%s i %d %d\n",xp->name,
						xp->count,offset);
					offset += xp->count * sizeof(int);
					break;
			case PFFLOAT:	sprintf(s,"%s f %d %d\n",xp->name,
						xp->count,offset);
					offset += xp->count * sizeof(float);
					break;
			default:	hpub_perr(
					"hpub_wrthdr - illegal parameter type");
			}
			offset = (offset+3) & (~03); /* round up to 4 bytes */
		}
		j = strlen(s);
		i += j;
		if (fwrite(s,j,1,fp) != 1)
			hpub_perr("error writing hips header");
		xp = xp->nextp;
	}
	sprintf(s,"%d\n",offset);	/* the size of the binary area */
	j = strlen(s);
	i += j;
	while ((i & 03) != 0) {		/* pad binary area size line with
					   blanks so that entire header
					   comes out to be an even multiple
					   of 4 bytes - the binary area itself
					   is guaranteed to be an even
					   multiple because each individual
					   entry is padded */
		putc(' ',fp);
		i++;
	}
	if (fwrite(s,j,1,fp) != 1)	/* write size of binary area */
		hpub_perr("error writing hips header");
	xp = xpar->params;
	offset = 0;
	while (xp != HP_NULLPAR) {
		if (xp->count > 1) {
			switch (xp->format) {
			case PFASCII:
			case PFBYTE:	i = xp->count * sizeof(unsigned char);
					break;
			case PFSHORT:	i = xp->count * sizeof(short); break;
			case PFINT:	i = xp->count * sizeof(int); break;
			case PFFLOAT:	i = xp->count * sizeof(float); break;
			default:	hpub_perr(
					"hpub_wrthdr - illegal parameter type");
			}
			if (fwrite(xp->val.v_pb,i,1,fp) != 1)
				hpub_perr("error writing hips header");
			offset += i;
			while ((offset & 03) != 0) {
				putc('\0',fp);
				offset++;
			}
		}
		xp = xp->nextp;
	}
}

void hpub_wrthdr1(format,rows,cols,frames)

int format,rows,cols,frames;

{
	hpub_fwrthdr1(stdout,format,rows,cols,frames);
}

void hpub_fwrthdr1(fp,format,rows,cols,frames)

FILE *fp;
int format,rows,cols,frames;

{
	hpub_wnocr(fp,"");	/* onm */
	hpub_wnocr(fp,""); /* snm */
	hpub_dfprintf(fp,frames);
	hpub_wnocr(fp,"");
	hpub_dfprintf(fp,rows);	/* orows */
	hpub_dfprintf(fp,cols);	/* ocols */
	hpub_dfprintf(fp,8);	/* fake bpp */
	if (format == PFMSBF) {
		hpub_dfprintf(fp,format);
		hpub_dfprintf(fp,MSBFIRST);
	}
	else if (format == PFLSBF) {
		hpub_dfprintf(fp,format);
		hpub_dfprintf(fp,LSBFIRST);
	}
	else {
		hpub_dfprintf(fp,format);
		hpub_dfprintf(fp,0);
	}
	fprintf(fp,"Fake History\n");
	fprintf(fp,"Fake Description\n.\n");
}

int hpub_wnocr(fp,s)

char *s;
FILE *fp;

{
	char *t;
	int i;

	t = s;
	i = 0;
	while (*t != '\n' && *t != '\0') {
		putc(*t++,fp);
		i++;
	}
	putc('\n',fp);
	return(i+1);
}

int hpub_dfprintf(fp,i)

FILE *fp;
int i;

{
	char s[30];
	int j;

	sprintf(s,"%d\n",i);
	j = strlen(s);
	if (fwrite(s,j,1,fp) != 1)
		hpub_perr("error during header write");
	return(j);
}
