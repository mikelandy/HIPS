/*
 * Copyright (c) 1992
 *
 *	Rasmus Larsen
 *	IMSOR/TUD
 *	The Institute of Mathematical Statististics and Operations Research
 *	The Technical University of Denmark
 * 
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 * 
 */
/*
 *  ihs2rgb.c - IHS  to RGB transformation
 *
 *
 *  to load: cc ihs2rgb.c -o ihs2rgb -lhips -lm
 *
 */


#include <hipl_format.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void	pread_im();
void	norm_im();


#define pi            3.14159265358979323846

double	mat[9]  = {1,-0.25,0.433012702,1,-0.25,-0.433012702,1,0.5,0};
byte	cflag = 0;
byte	sflag = 0;
byte	Tflag = 0;

int types[] = {PFBYTE, PFSHORT, PFINT, PFFLOAT,LASTTYPE};

int main(argc,argv)
int argc;
char *argv[];
{
struct	header	hd;
Filename	filename = "<stdin>";
FILE		*fp;
int		nr,nc,nf,np,i,p;
float		*ihs,*in,*hue,*sat;
float		*rgb,*red,*green,*blue;
float		*l,*u,*v;
byte		twoflag = 0;


Progname = strsave(*argv);
for (i=1;i<argc;i++) {
	if (strncmp(argv[i], "-c", 2) == 0) {
		cflag = 1;
		continue;
	}
	if (strncmp(argv[i], "-s", 2) == 0) {
		sflag = 1;
		continue;
	}
	if (strncmp(argv[i], "-T", 2) == 0) {
		Tflag = 1;
		continue;
	}
	if (strncmp(argv[i], "-U", 2) == 0) {
		fprintf(stderr,"Usage: ihs2rgb [-c] [-s] [-T]\n");
		exit(1);
	}
	if (strncmp(argv[i],"-",1) == 0) {
		fprintf(stderr,"%s: unknown option '%s'\n",Progname,argv[i]);
		fprintf(stderr,"Usage: ihs2rgb [-c] [-s] [-T]\n");
		exit(1);
	}
}

fp = hfopenr(filename);
fread_hdr_cpf(fp,&hd,types,filename);

nr = hd.rows;
nc = hd.cols;
np = nr * nc;
nf = hd.num_frame;

if (!(nf == 3 || nf == 2) ) {
	fprintf(stderr,"%s: Implemented for 2-frame and 3-frame sequences only",Progname);
	exit(1);
}
if (nf == 2) {
	fprintf(stderr,"%s: Setting saturation to maximum\n",Progname);
	twoflag = 1;
}

ihs = (float *) halloc(np*3,sizeof(float));
rgb = (float *) halloc(np*3,sizeof(float));

/* read frame */
pread_im(hd,ihs,fp);
norm_im(ihs,np,twoflag); 

hd.pixel_format = PFFLOAT;
hd.num_frame = 3;
hd.numcolor = 3;
write_headeru(&hd,argc,argv);


if (Tflag) {

	l	= ihs;
	u	= ihs + np;
	v	= ihs + np * 2;

	red	= rgb;
	green	= rgb + np;
	blue	= rgb + np * 2;

	/* In order to obtain that v is greenness,			*/
	/* the sign of v has been changed				*/
	for (p=0;p<np;p++,red++,green++,blue++,l++,u++,v++) {
		*red	= *l + *u * mat[1] - *v * mat[2];
		*green	= *l + *u * mat[4] - *v * mat[5];
		*blue	= *l + *u * mat[7];
	}
}
else {
	in	= ihs;
	hue	= ihs + np;
	sat	= ihs + np * 2;

	red	= rgb;
	green	= rgb + np;
	blue	= rgb + np * 2;

	for (p=0;p<np;p++,red++,green++,blue++,in++,hue++,sat++) {
		*red	= *in + *sat * (mat[1]*cos(*hue) + mat[2]*sin(*hue));
		*green	= *in + *sat * (mat[4]*cos(*hue) + mat[5]*sin(*hue));
		*blue	= *in + *sat *  mat[7]*cos(*hue);
	}
}
if (sflag == 0) {
	red	= rgb;
	green	= rgb + np;
	blue	= rgb + np * 2;

	for (p=0;p<np;p++,red++,green++,blue++) {
		if (*red < 0 ) *red = 0;
		if (*red > 1 ) *red = 1;
		if (*green < 0 ) *green = 0;
		if (*green > 1 ) *green = 1;
		if (*blue < 0 ) *blue = 0;
		if (*blue > 1 ) *blue = 1;
	}
}


/* write new pixels */
if(fwrite(rgb,sizeof(float),3*np,stdout) != 3*np) {
	fprintf(stderr,"%s: error during write",Progname);
	exit(1);
}
return(0);
}
/* fread_im: reads image from stdin into a float format */
void pread_im(hd,image,fp)

struct header	hd;
float		*image;
FILE		*fp;
{
	int		pfmt;
	byte		*bp,*b;
	short		*sp,*s;
	int		*ip,*r;
	float		*f;
	int		i,k;
	int		npix;


	pfmt = hd.pixel_format;
	npix = hd.rows*hd.cols;
	f = image;

	if (pfmt == PFBYTE) {
		bp = (byte *) malloc(npix*sizeof(byte));
		for (k=0;k<hd.num_frame;k++) {
			b = bp;
			if(fread(bp,sizeof(byte),npix,fp) != npix) {
				fprintf(stderr,"%s: read error",Progname);
				exit(1);
			}
			for (i=0;i<npix;i++) *f++ = (float) *b++;
		}
		free(bp);
	}
	if (pfmt == PFSHORT) {
		sp = (short *) malloc(sizeof(short)*npix);
		for (k=0;k<hd.num_frame;k++) {
			s = sp;
			if(fread(sp,sizeof(short),npix,fp) != npix) {
				fprintf(stderr,"%s: read error",Progname);
				exit(1);
			}
			for (i=0;i<npix;i++) *f++ = (float) *s++;
		}
		free(sp);
	}
	if (pfmt == PFINT) {
		ip = (int *) malloc(sizeof(int)*npix);
		for (k=0;k<hd.num_frame;k++) {
			r = ip;
			if(fread(ip,sizeof(int),npix,fp) != npix) {
				fprintf(stderr,"%s: read error",Progname);
				exit(1);
			}
			for (i=0;i<npix;i++) *f++ = (float) *r++;
		}
		free(ip);
	}
	if (pfmt == PFFLOAT) {
		if(fread(image,sizeof(float),npix*hd.num_frame,fp) 
			!= npix*hd.num_frame) {
				fprintf(stderr,"%s: read error",Progname);
				exit(1);
		}
	}
}

/*norm_im: norms input image for transformation */
void	norm_im(ihs,np,twoflag)
float	*ihs;
int	np;
byte	twoflag;

{
	int	i;
	float	min,max;
	float	*r,*p,*q,temp;
	double	magenta;
	double	blue;
	double	colour;
	
	blue	= - 2*pi;
	magenta	= - 5*pi/3;
	if (cflag) colour = blue;
	else colour = magenta;

	/* Intensity */

	min = *ihs;
	max = *ihs;
	p = ihs;
	for (i=0;i<np;i++,p++)
		if (*p < min) min = *p; else {if (*p > max) max = *p;}

	p = ihs;
	for (i=0;i<np;i++,p++) {
		temp  = (*p - min)/(max - min);
		*p = temp;
	}

	if (!Tflag) {

		/* Hue */

		min = *(ihs + np);
		max = *(ihs + np);
		p = ihs + np;
		for (i=0;i<np;i++,p++)
			if (*p < min) min = *p; else {if (*p > max) max = *p;}
		p = ihs + np;
		for (i=0;i<np;i++,p++) {
			temp = colour * (*p - min)/(max - min);
			*p = temp;
		}

		/* Saturation */

		if (!twoflag) {
			min = *(ihs + 2*np);
			max = *(ihs + 2*np);
			p = ihs + 2*np;
			for (i=0;i<np;i++,p++)
				if (*p < min) min = *p; else {if (*p > max) max = *p;}
			p = ihs + 2*np;
			q = ihs;
			for (i=0;i<np;i++,p++,q++) {
				temp = (*p - min)/(max - min);
				if (sflag == 0) *p =temp;
				else {
					if (*q < 0.5)	*p = temp * *q * 2;
					else		*p = temp * (1 - *q) * 2;
				}
			}
		}
		else {
			q = ihs;
			p = ihs + 2*np;
			for (i=0;i<np;i++,p++,q++) {
				if (sflag == 0) *p = 1;
				else {
					if (*q < 0.5)	*p =  *q * 2;
					else		*p =  (1 - *q) * 2;
				}
			}
		}
	}
	else {
		/* blueness */

		min = *(ihs + np);
		max = *(ihs + np);
		p = ihs + np;
		for (i=0;i<np;i++,p++)
			if (*p < min) min = *p; else {if (*p > max) max = *p;}
		p = ihs + np;
		for (i=0;i<np;i++,p++) {
			temp = 2 * (*p - min)/(max - min) - 1;
			*p = temp;
		}

		/* greenness */

		min = *(ihs + 2 * np);
		max = *(ihs + 2 * np);
		p = ihs + 2 * np;
		for (i=0;i<np;i++,p++)
			if (*p < min) min = *p; else {if (*p > max) max = *p;}
		p = ihs + 2 * np;
		for (i=0;i<np;i++,p++) {
			temp = 2 * (*p - min)/(max - min) - 1;
			*p = temp;
		}

		if (sflag) {

			/* blueness and greenness */

			max = 0;
			p = ihs + np;
			q = ihs + 2*np;
			for (i=0;i<np;i++,p++,q++) {
				temp = *p * *p + *q * *q;
				if (temp > max) max = temp;
			}
			temp = sqrt(max);
			max = temp;
			p = ihs + np;
			q = ihs + 2*np;
			r = ihs;
			for (i=0;i<np;i++,p++,q++,r++) {
				if (*r < 0.5) {
					*p *= *r * 2 / max;
					*q *= *r * 2 / max;
				}
				else {
					*p *= (1 - *r) * 2 / max;
					*q *= (1 - *r) * 2 / max;
				}
			}
		}
	}
}
