/*	Copyright (c) 1987 Pierre Landau

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * quantile.c - apply a quantile filter to an image
 *
 * usage:	quantile [-s size] [-q quantile] <iseq >oseq
 *
 * where size is the length of the side of the neighborhood in which the
 * quantile is computed. Size defaults to 3,
 *
 * and quantile is the quantile of the filter. Quantile defaults to .5.
 *
 * to load:	cc -o quantile quantile.c -lhips
 *
 * Mike Landy - 5/28/82
 * median algorithm replaced <Pierre Landau 1/6/87>
 * modified for short, int and float <Rasmus Larsen 7/5/1992>
 * modified for general quantilefilter <Lars Roenved 8/2/1993>
 */

#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFBYTE,PFSHORT,PFINT,PFFLOAT,LASTTYPE};
static Flag_Format flagfmt[] = {
	{"s",{LASTFLAG},1, {{PTINT,"3","size"}, LASTPARAMETER}}, 
	{"q",{LASTFLAG},1, {{PTDOUBLE,"0.5","quantile"}, LASTPARAMETER}},
	LASTFLAG };

float  sselect();

int main(argc,argv)
int argc;
char *argv[];
{
	void	fread_im();
	void	write_im();

	Filename filename;
	FILE	*fp;
	int	size;			/* kernel size */
	int	f,fr;			/* frame number*/
	int	r;			/* row number */
	int	c;			/* column number */
	int	sizesq;			/* kernel size (n**2) */
	int	halfsz;			/* sizesq / 2 */
	int	ir,ic;			/* row and column counters */
	int	minus,plus,top,bot,left,right;	/* offsets of
						 * kernel boundaries */
	double	quan;			/* quantile */
	float	*nb;
	float	*np;			/* pointer to fastmedian array */
	int	*col;			/* vector into ifr */
	int	nextrow;		/* offset to next row in kernel */
	struct	header hd;		/* HIPS header */
	float	*ifr;			/* input frame */
	float	*ofr;			/* output frame */
	float	*ip;			/* pointer to input */
	float	*op;			/* pointer to output */
	float	*nnp;			/* pointer to input kernel */
	register int i,j,ii,jj;		/* loop counters */

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&size,&quan,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_cpf(fp,&hd,types,filename);
	clearroi(&hd);

	sizesq = size*size;
	halfsz = (sizesq * quan) + .999999 ;
	plus = size / 2;
	minus = plus - size + 1;
	r = hd.orows; c = hd.ocols;
	fr = hd.num_frame;
	top = -minus;
	bot = r - plus;
	left = -minus;
	right = c - plus;
	nextrow = c*minus + minus;
	write_headeru(&hd,argc,argv);
	ifr = (float *) malloc(r*c*sizeof (float));
	ofr = (float *) malloc(r*c*sizeof (float));
	nb = (float *) malloc(sizesq*sizeof (float));
	col = (int *) malloc(r* sizeof (int));
	for (ic = -c,i=0;i<r;i++) col[i] = (ic += c); /* vector array */
	for (f=0;f<fr;f++) {
		fread_im(hd,ifr,fp);
		ip = ifr;
		op = ofr;
		for (i=0;i<r;i++) {
			for (j=0;j<c;j++) {
				if (i<top || i>=bot || j<left || j>=right) {
					np = nb;
					for (ii=minus;ii<=plus;ii++)
						for (jj=minus;jj<=plus;jj++) {
						    ir = i + ii;
						    ic = j + jj;
						    ir = ir<0?0:(ir>=r)?r-1:ir;
						    ic = ic<0?0:(ic>=c)?c-1:ic;
						    *np++ = ifr[col[ir]+ic];
						}
				}
				else {
					nnp = ip + nextrow;
					np = nb;
					for (ii=minus;ii<=plus;ii++) {
						for (jj=minus;jj<=plus;jj++)
							*np++ = *nnp++ ;
						nnp += c - size;
					}
				}
				ip++;
				*op++ = sselect(halfsz,nb,nb+sizesq-1);
			}
		}
		write_im(hd,ofr);
	}
	return(0);
}

#define exchf(a,b) {tmpf = a; a = b; b = tmpf;}	/* exchange floats */
#define exchp(a,b) {tmpp = a; a = b; b = tmpp;}	/* exchange pointers */

float sselect(k,lo,hi)
int k;
float *lo,*hi;

/* select the k'th element from the list between lo and hi
 * this is a implementation of R.W.Floyd's improvement to Hoare's
 * original algorithm, as published in CACM, vol 18 no 3 (march 1975) 
 */

{
    register float *i,*j,*a;
    float *val[3],t,*tmpp,tmpf;
    int df;
    while(1) {
      if ((t = hi-lo) <= 2) /* if the sequence is short (n<3) sort
                               it directly */
        {
          val[0] = lo;
          val[1] = lo+1;
          val[2] = hi;
          if (t == 1) {
            return(*val[0] < *val[1] ? *val[k-1] : *val[2-k]);
          }
          else{
            if (*val[0] > *val[1]) exchp(val[0],val[1])
            if (*val[0] > *val[2]) exchp(val[0],val[2])
            if (*val[1] > *val[2]) exchp(val[1],val[2])
            return (*val[k-1]);
          }
        }
      else {
        t = *(a = lo);          /* take first element of list as pivot */
        i = lo;
        j = hi;
        if (*hi > t) exchf(*hi,*lo); /* set up for first exchange */
        while (i < j) 
          {
            exchf(*i,*j)
            i++;
            j--;
            while (*i < t) i++; /* scan list for pair to exchange */
            while (*j > t) j--;
          }
        if (*lo == t) exchf(*lo,*j) /* put pivot back where it belongs */
        else {
            j++;
            exchf(*j,*hi)
            }
        
      /* now adjust hi,lo so they surround the subset containing
         the k-l+1th element */

        df = j-lo+1;
        if (df < k) {
          k = k-(df);
          lo = j+1;
        }
        else {
          if (df == k) return (*j);
          else hi = j - 1;
        }
      }
    }
  }
/* fread_im: reads image from stdin into a float format */
void fread_im(hd,image,fil)

struct header	hd;
float		*image;
FILE		*fil;

{
	int		pfmt;
	byte		*bpp,*b;
	short		*spp,*s;
	int		*ipp,*r;
	float		*fpp;
	int		i;
	int		npix,size;


	pfmt = hd.pixel_format;
	npix = hd.orows*hd.ocols;
	size = npix * hd.sizepix;
	fpp = image;

	if (pfmt == PFBYTE) {
		bpp = (byte *) malloc(size);
		b = bpp;
		if(fread(bpp,sizeof(byte),npix,fil) != npix) {
			fprintf(stderr,"%s: Error during read\n",Progname);
			exit(1);
		}
		for (i=0;i<npix;i++) *fpp++ = (float) *b++;
		free(bpp);
	}
	if (pfmt == PFSHORT) {
		spp = (short *) malloc(size);
		s = spp;
		if(fread(spp,sizeof(short),npix,fil) != npix) {
			fprintf(stderr,"%s: Error during read\n",Progname);
			exit(1);
		}
		for (i=0;i<npix;i++) *fpp++ = (float) *s++;
		free(spp);
	}
	if (pfmt == PFINT) {
		ipp = (int *) malloc(size);
		r = ipp;
		if(fread(ipp,sizeof(int),npix,fil) != npix) {
			fprintf(stderr,"%s: Error during read\n",Progname);
			exit(1);
		}
		for (i=0;i<npix;i++) *fpp++ = (float) *r++;
		free(ipp);
	}
	if (pfmt == PFFLOAT) 
		if(fread(image,sizeof(float),npix,fil) != npix) {
			fprintf(stderr,"%s: Error during read\n",Progname);
			exit(1);
		}
}
/* write_im: writes image on stdout */
void write_im(hd,image)

struct header	hd;
float		*image;

{
	int		pfmt;
	byte		*bpp,*b;
	short		*spp,*s;
	int		*ipp,*r;
	float		*fpp;
	int		i;
	int		npix,size;


	pfmt = hd.pixel_format;
	npix = hd.orows*hd.ocols;
	size = npix * hd.sizepix;
	fpp = image;

	if (pfmt == PFBYTE) {
		bpp = (byte *) malloc(size);
		b = bpp;
		for (i=0;i<npix;i++) *b++ = (byte) *fpp++;
		if(fwrite(bpp,sizeof(byte),npix,stdout) != npix) {
			fprintf(stderr,"%s: Error during write\n",Progname);
			exit(1);
		}
		free(bpp);
	}
	if (pfmt == PFSHORT) {
		spp = (short *) malloc(size);
		s = spp;
		for (i=0;i<npix;i++) *s++ = (short) *fpp++;
		if(fwrite(spp,sizeof(short),npix,stdout) != npix) {
			fprintf(stderr,"%s: Error during write\n",Progname);
			exit(1);
		}
		free(spp);
	}
	if (pfmt == PFINT) {
		ipp = (int *) malloc(size);
		r = ipp;
		for (i=0;i<npix;i++) *r++ = (int) *fpp++;
		if(fwrite(ipp,sizeof(int),npix,stdout) != npix) {
			fprintf(stderr,"%s: Error during write\n",Progname);
			exit(1);
		}
		free(ipp);
	}
	if (pfmt == PFFLOAT) 
		if(fwrite(image,sizeof(float),npix,stdout) != npix) {
			fprintf(stderr,"%s: Error during write\n",Progname);
			exit(1);
		}
}
