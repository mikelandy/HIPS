/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * ptoa - converts  images from pixel-format  to ASCII
 *
 * usage:	ptoa <iseq >oseq
 *
 * to load:	cc -o ptoa ptoa.c -lhips
 *
 * Yoav Cohen - 6/26/83
 * Mike Landy - 10/9/83
 * HIPS 2 - msl - 1/8/91
 * RGB/RGBZ/etc. - msl - 5/24/93
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PFBYTE,PFSHORT,PFINT,PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,
	PFINTPYR,PFFLOATPYR,PFMIXED,PFRGB,PFRGBZ,PFZRGB,PFBGR,PFBGRZ,PFZBGR,
	LASTTYPE};
void btoa(),itoa(),stoa(),dtoa(),iptoa(),fptoa(),ftoa(),rgbtoa(),rgbztoa();
void zrgbtoa(),bgrtoa(),bgrztoa(),zbgrtoa();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp;
	int method,fr,f,r,c,toplev,form,one=1,*fmts,fmtssize;
	hsize_t currsize;
	Filename filename;
	h_boolean mflag=FALSE;
	FILE *fp;

	Progname = strsave(*argv);
	hips_hchar = 1;
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	if (hd.pixel_format == PFMIXED)
		mflag = TRUE;
	method = fset_conversion(&hd,&hdp,types,filename);
	form=hdp.pixel_format;
	r = hdp.orows; c = hdp.ocols; fr = hdp.num_frame;
	if (form==PFCOMPLEX) {
		hdp.cols *= 2;
		hdp.fcol *= 2;
		c = hdp.ocols *= 2;
		form=PFFLOAT;
	}
	else if (form==PFDBLCOM) {
		hdp.cols *= 2;
		hdp.fcol *= 2;
		c = hdp.ocols *= 2;
		form=PFDOUBLE;
	}
	else if (ptype_is_col3(form)) {
		hdp.cols *= 3;
		hdp.fcol *= 3;
		hdp.ocols *= 3;
	}
	if (form==PFINTPYR || form==PFFLOATPYR)
		getparam(&hdp,"toplev",PFINT,&one,&toplev);
	hdp.pixel_format = PFASCII;
	write_headeru(&hdp,argc,argv);
	hdp.pixel_format = form;
	if (mflag) {
		fmtssize = hd.num_frame;
		getparam(&hd,"formats",PFINT,&fmtssize,&fmts);
		if (fmtssize != hd.num_frame)
			perr(HE_FMTSLEN,filename);
		setformat(&hd,fmts[0]);
		alloc_image(&hd);
		currsize = hd.sizeimage;
		for (f=0;f<fr;f++) {
			setformat(&hd,fmts[f]);
			if (hd.sizeimage > currsize) {
				free(hd.image);
				alloc_image(&hd);
				currsize = hd.sizeimage;
			}
			fread_image(fp,&hd,f,filename);
			form = fmts[f];
			c = (form == PFCOMPLEX || form == PFDBLCOM)
				? (hd.ocols * 2) : hd.ocols;
			if (form == PFCOMPLEX)
				form = PFFLOAT;
			else if (form == PFDBLCOM)
				form = PFDOUBLE;
			if (form == PFBYTE)
				btoa(&hd,r,c);
			else if (form == PFINT)
				itoa(&hd,r,c);
			else if (form == PFSHORT)
				stoa(&hd,r,c);
			else if (form == PFDOUBLE)
				dtoa(&hd,r,c);
			else if (form == PFINTPYR)
				iptoa(&hd,r,c,toplev);
			else if (form == PFFLOATPYR)
				fptoa(&hd,r,c,toplev);
			else if (form == PFFLOAT)
				ftoa(&hd,r,c);
			else if (form == PFRGB)
				rgbtoa(&hd,r,c);
			else if (form == PFRGBZ)
				rgbztoa(&hd,r,c);
			else if (form == PFZRGB)
				zrgbtoa(&hd,r,c);
			else if (form == PFBGR)
				bgrtoa(&hd,r,c);
			else if (form == PFBGRZ)
				bgrztoa(&hd,r,c);
			else if (form == PFZBGR)
				zbgrtoa(&hd,r,c);
			else
				perr(HE_FMTFILE,hformatname(form),filename);
		}
	}
	else {
		for (f=0;f<fr;f++) {
			fread_imagec(fp,&hd,&hdp,method,f,filename);
			if (form == PFBYTE)
				btoa(&hdp,r,c);
			else if (form == PFINT)
				itoa(&hdp,r,c);
			else if (form == PFSHORT)
				stoa(&hdp,r,c);
			else if (form == PFDOUBLE)
				dtoa(&hdp,r,c);
			else if (form == PFINTPYR)
				iptoa(&hdp,r,c,toplev);
			else if (form == PFFLOATPYR)
				fptoa(&hdp,r,c,toplev);
			else if (form == PFRGB)
				rgbtoa(&hdp,r,c);
			else if (form == PFRGBZ)
				rgbztoa(&hdp,r,c);
			else if (form == PFZRGB)
				zrgbtoa(&hdp,r,c);
			else if (form == PFBGR)
				bgrtoa(&hdp,r,c);
			else if (form == PFBGRZ)
				bgrztoa(&hdp,r,c);
			else if (form == PFZBGR)
				zbgrtoa(&hdp,r,c);
			else
				ftoa(&hdp,r,c);
		}
	}
	return(0);
}

void btoa(hd,r,c)

struct header *hd;
int r,c;

{
	int i,j;
	byte *ip;

	ip = hd->image;
	for (i=0;i<r;i++) {
		for (j=0;j<c;j++)
			printf("%d ",(int) *ip++);
		printf("\n");
	}
	printf("\n");
}

void itoa(hd,r,c)

struct header *hd;
int r,c;

{
	int i,j;
	int *ip;

	ip = (int *) hd->image;
	for (i=0;i<r;i++) {
		for (j=0;j<c;j++)
			printf("%d ",*ip++);
		printf("\n");
	}
	printf("\n");
}

void stoa(hd,r,c)

struct header *hd;
int r,c;

{
	int i,j;
	short *spp;

	spp = (short *) hd->image;
	for (i=0;i<r;i++) {
		for (j=0;j<c;j++)
			printf("%d ",(int) *spp++);
		printf("\n");
	}
	printf("\n");
}

void ftoa(hd,r,c)

struct header *hd;
int r,c;

{
	int i,j;
	float *ip;

	ip = (float *) hd->image;
	for (i=0;i<r;i++) {
		for (j=0;j<c;j++)
			printf("%f ",*ip++);
		printf("\n");
	}
	printf("\n");
}

void dtoa(hd,r,c)

struct header *hd;
int r,c;

{
	int i,j;
	double *ip;

	ip = (double *) hd->image;
	for (i=0;i<r;i++) {
		for (j=0;j<c;j++)
			printf("%f ",(float) *ip++);
		printf("\n");
	}
	printf("\n");
}

void iptoa(hd,r,c,toplev)

struct header *hd;
int r,c,toplev;

{
	int i,j,lev,nr,nc;
	int *ip;

	ip = (int *) hd->image;
	nr = r; nc = c;
	for (lev=0;lev<=toplev;lev++) {
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++)
				printf("%d ",*ip++);
			printf("\n");
		}
		nr = (nr+1)/2; nc = (nc+1)/2;
	}
	printf("\n");
}

void fptoa(hd,r,c,toplev)

struct header *hd;
int r,c,toplev;

{
	int i,j,lev,nr,nc;
	float *ip;

	ip = (float *) hd->image;
	nr = r; nc = c;
	for (lev=0;lev<=toplev;lev++) {
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++)
				printf("%f ",*ip++);
			printf("\n");
		}
		nr = (nr+1)/2; nc = (nc+1)/2;
	}
	printf("\n");
}

void rgbtoa(hd,r,c)

struct header *hd;
int r,c;

{
	int i,j;
	byte *ip;

	ip = hd->image;
	for (i=0;i<r;i++) {
		for (j=0;j<c;j++) {
			if (j)
				printf("   ");
			printf("%d %d %d",(int) ip[0],(int) ip[1], (int) ip[2]);
			ip += 3;
		}
		printf("\n");
	}
	printf("\n");
}

void rgbztoa(hd,r,c)

struct header *hd;
int r,c;

{
	int i,j;
	byte *ip;

	ip = hd->image;
	for (i=0;i<r;i++) {
		for (j=0;j<c;j++) {
			if (j)
				printf("   ");
			printf("%d %d %d",(int) ip[0],(int) ip[1], (int) ip[2]);
			ip += 4;
		}
		printf("\n");
	}
	printf("\n");
}

void zrgbtoa(hd,r,c)

struct header *hd;
int r,c;

{
	int i,j;
	byte *ip;

	ip = hd->image;
	ip++;
	for (i=0;i<r;i++) {
		for (j=0;j<c;j++) {
			if (j)
				printf("   ");
			printf("%d %d %d",(int) ip[0],(int) ip[1], (int) ip[2]);
			ip += 4;
		}
		printf("\n");
	}
	printf("\n");
}

void bgrtoa(hd,r,c)

struct header *hd;
int r,c;

{
	int i,j;
	byte *ip;

	ip = hd->image;
	for (i=0;i<r;i++) {
		for (j=0;j<c;j++) {
			if (j)
				printf("   ");
			printf("%d %d %d",(int) ip[2],(int) ip[1], (int) ip[0]);
			ip += 3;
		}
		printf("\n");
	}
	printf("\n");
}

void bgrztoa(hd,r,c)

struct header *hd;
int r,c;

{
	int i,j;
	byte *ip;

	ip = hd->image;
	for (i=0;i<r;i++) {
		for (j=0;j<c;j++) {
			if (j)
				printf("   ");
			printf("%d %d %d",(int) ip[2],(int) ip[1], (int) ip[0]);
			ip += 4;
		}
		printf("\n");
	}
	printf("\n");
}

void zbgrtoa(hd,r,c)

struct header *hd;
int r,c;

{
	int i,j;
	byte *ip;

	ip = hd->image;
	ip++;
	for (i=0;i<r;i++) {
		for (j=0;j<c;j++) {
			if (j)
				printf("   ");
			printf("%d %d %d",(int) ip[2],(int) ip[1], (int) ip[0]);
			ip += 4;
		}
		printf("\n");
	}
	printf("\n");
}
