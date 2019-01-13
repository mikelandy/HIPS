/*
 * Copyright (c) 1995 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * getmin - calculate the minimum value in a sequence
 *
 * usage:	getmin [-c] [-s] [-i] [-p] < iseq
 *
 * Switches:
 *		-c	Print out coordinates of minima
 *		-s	Compute minimum of entire sequence (input must be
 *				seq-able)
 *		-i	Print minimum as an integer (with rounding)
 *		-n	Ignore zero-valued pixels
 *		-p	Pipe the input to the output
 *
 *		-n	Ignore zero-valued pixels
 *
 * For a completely zero frame, without -c the minimum will print out as zero.
 *
 * For complex types, the minimum magnitude is computed.
 *
 * pixel formats handled directly: BYTE, SBYTE, SHORT, USHORT, INT, UINT,
 *	FLOAT, DBL, COMPLEX, DBLCOM
 *
 * to load:	cc -o getmin getmin.c -lhipsh -lhips -lm
 *
 * Mike Landy - 4/13/95
 */

#include <stdio.h>
#include <hipl_format.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>

static Flag_Format flagfmt[] = {
	{"c",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"s",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"i",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"n",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"p",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,PFSBYTE,PFSHORT,PFUSHORT,PFINT,PFFLOAT,PFDOUBLE,
	PFCOMPLEX,PFDBLCOM,LASTTYPE};

h_boolean cflag,sflag,iflag,nflag,pflag,piped;
struct header hd,hdp,dummyhd;
void init_min(),update_min(),print_coords(),print_min(),pfloat();

int main(argc,argv)

int argc;
char **argv;

{
	int method,fr,f;
	Filename filename;
	FILE *fp;
	Pixelval min,max,smin;
	struct stat buf;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&cflag,&sflag,&iflag,&nflag,&pflag,
		FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	if (fseek(fp,0L,1) < 0)
		piped = TRUE;
#ifdef S_IFIFO
	if (!piped) {
		fstat(fileno(fp),&buf);
		if ((buf.st_mode & S_IFMT) == S_IFIFO)
			piped = TRUE;
	}
#endif
	if (sflag && (hd.num_frame == 1))
		sflag = FALSE;
	if (sflag && piped)
		perr(HE_MSG,"input sequence must be rewindable with -s");
	method = fset_conversion(&hd,&hdp,types,filename);
	if (pflag)
		write_headeru(&hd,argc,argv);
	fr = hdp.num_frame;
	if (sflag)
		init_min(&smin);
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		if (pflag)
			write_image(&hd,f);
		h_minmax(&hdp,&min,&max,nflag);
		if (sflag) {
			update_min(&min,&smin);
		}
		else {
			if (cflag)
				print_coords(&min,f);
			else
				print_min(&min,f);
		}
	}
	if (sflag && cflag) {
		fseek(fp,0L,0);
		fread_hdr_a(fp,&dummyhd,filename);
		for (f=0;f<fr;f++) {
			fread_imagec(fp,&hd,&hdp,method,f,filename);
			print_coords(&smin,f);
		}
	}
	else if (sflag)
		print_min(&smin,f);
	return(0);
}

void init_min(smin)

Pixelval *smin;

{
	switch(hdp.pixel_format) {
	case PFBYTE:	smin->v_byte = 0; return;
	case PFSBYTE:	smin->v_sbyte = 0; return;
	case PFSHORT:	smin->v_short = 0; return;
	case PFUSHORT:	smin->v_ushort = 0; return;
	case PFINT:	smin->v_int = 0; return;
	case PFUINT:	smin->v_uint = 0; return;
	case PFFLOAT:
	case PFCOMPLEX:	smin->v_float = 0; return;
	case PFDOUBLE:
	case PFDBLCOM:	smin->v_double = 0; return;
	default:	perr(HE_MSG,"unreasonable format in init_min");
	}
}

void update_min(min,smin)

Pixelval *min,*smin;

{
	switch(hdp.pixel_format) {
	case PFBYTE:	
			if (nflag) {
				if (min->v_byte != 0 &&
				    min->v_byte > smin->v_byte)
					smin->v_byte = min->v_byte;
			}
			else {
				if (min->v_byte > smin->v_byte)
					smin->v_byte = min->v_byte;
			}
			return;
	case PFSBYTE:	
			if (nflag) {
				if (min->v_sbyte != 0 &&
				    min->v_sbyte > smin->v_sbyte)
					smin->v_sbyte = min->v_sbyte;
			}
			else {
				if (min->v_sbyte > smin->v_sbyte)
					smin->v_sbyte = min->v_sbyte;
			}
			return;
	case PFSHORT:	
			if (nflag) {
				if (min->v_short != 0 &&
				    min->v_short > smin->v_short)
					smin->v_short = min->v_short;
			}
			else {
				if (min->v_short > smin->v_short)
					smin->v_short = min->v_short;
			}
			return;
	case PFUSHORT:	
			if (nflag) {
				if (min->v_ushort != 0 &&
				    min->v_ushort > smin->v_ushort)
					smin->v_ushort = min->v_ushort;
			}
			else {
				if (min->v_ushort > smin->v_ushort)
					smin->v_ushort = min->v_ushort;
			}
			return;
	case PFINT:	
			if (nflag) {
				if (min->v_int != 0 &&
				    min->v_int > smin->v_int)
					smin->v_int = min->v_int;
			}
			else {
				if (min->v_int > smin->v_int)
					smin->v_int = min->v_int;
			}
			return;
	case PFUINT:	
			if (nflag) {
				if (min->v_uint != 0 &&
				    min->v_uint > smin->v_uint)
					smin->v_uint = min->v_uint;
			}
			else {
				if (min->v_uint > smin->v_uint)
					smin->v_uint = min->v_uint;
			}
			return;
	case PFFLOAT:	
	case PFCOMPLEX:	
			if (nflag) {
				if (min->v_float != 0 &&
				    min->v_float > smin->v_float)
					smin->v_float = min->v_float;
			}
			else {
				if (min->v_float > smin->v_float)
					smin->v_float = min->v_float;
			}
			return;
	case PFDOUBLE:	
	case PFDBLCOM:	
			if (nflag) {
				if (min->v_double != 0 &&
				    min->v_double > smin->v_double)
					smin->v_double = min->v_double;
			}
			else {
				if (min->v_double > smin->v_double)
					smin->v_double = min->v_double;
			}
			return;
	default:	perr(HE_MSG,"unreasonable format in update_min");
	}
}

void print_coords(min,f)

Pixelval *min;
int f;

{
	int nr,nc,nex,i,j;
	byte *pb,m_b;
	sbyte *psb,m_sb;
	short *ps,m_s;
	h_ushort *pus,m_us;
	int *pi,m_i;
	h_uint *pui,m_ui;
	float *pf,m_f,magn_f;
	double *pd,m_d,magn_d;

	nr = hdp.rows;
	nc = hdp.cols;
	nex = hdp.ocols - hdp.cols;
	if (hdp.pixel_format == PFCOMPLEX || hdp.pixel_format == PFDBLCOM)
		nex *= 2;

	switch(hdp.pixel_format) {
	case PFBYTE:	
			if (nflag && min->v_byte == 0)
				return;
			pb = hdp.firstpix;
			m_b = min->v_byte;
			for (i=0;i<nr;i++) {
				for (j=0;j<nc;j++) {
					if (*pb++ == m_b)
						fprintf(stderr,"%d %d %d %d\n",
							f,i,j,m_b);
				}
				pb += nex;
			}
			return;
	case PFSBYTE:	
			if (nflag && min->v_sbyte == 0)
				return;
			psb = (sbyte *) hdp.firstpix;
			m_sb = min->v_sbyte;
			for (i=0;i<nr;i++) {
				for (j=0;j<nc;j++) {
					if (*psb++ == m_sb)
						fprintf(stderr,"%d %d %d %d\n",
							f,i,j,m_sb);
				}
				psb += nex;
			}
			return;
	case PFSHORT:	
			if (nflag && min->v_short == 0)
				return;
			ps = (short *) hdp.firstpix;
			m_s = min->v_short;
			for (i=0;i<nr;i++) {
				for (j=0;j<nc;j++) {
					if (*ps++ == m_s)
						fprintf(stderr,"%d %d %d %d\n",
							f,i,j,m_s);
				}
				ps += nex;
			}
			return;
	case PFUSHORT:	
			if (nflag && min->v_ushort == 0)
				return;
			pus = (h_ushort *) hdp.firstpix;
			m_us = min->v_ushort;
			for (i=0;i<nr;i++) {
				for (j=0;j<nc;j++) {
					if (*pus++ == m_us)
						fprintf(stderr,"%d %d %d %d\n",
							f,i,j,m_us);
				}
				pus += nex;
			}
			return;
	case PFINT:	
			if (nflag && min->v_int == 0)
				return;
			pi = (int *) hdp.firstpix;
			m_i = min->v_int;
			for (i=0;i<nr;i++) {
				for (j=0;j<nc;j++) {
					if (*pi++ == m_i)
						fprintf(stderr,"%d %d %d %d\n",
							f,i,j,m_i);
				}
				pi += nex;
			}
			return;
	case PFUINT:	
			if (nflag && min->v_uint == 0)
				return;
			pui = (h_uint *) hdp.firstpix;
			m_ui = min->v_uint;
			for (i=0;i<nr;i++) {
				for (j=0;j<nc;j++) {
					if (*pui++ == m_ui)
						fprintf(stderr,"%d %d %d %d\n",
							f,i,j,m_ui);
				}
				pui += nex;
			}
			return;
	case PFFLOAT:	
			if (nflag && min->v_float == 0)
				return;
			pf = (float *) hdp.firstpix;
			m_f = min->v_float;
			for (i=0;i<nr;i++) {
				for (j=0;j<nc;j++) {
					if (*pf++ == m_f) {
						fprintf(stderr,"%d %d %d ",
							f,i,j);
						pfloat(m_f);
					}
				}
				pf += nex;
			}
			return;
	case PFDOUBLE:	
			if (nflag && min->v_double == 0)
				return;
			pd = (double *) hdp.firstpix;
			m_d = min->v_double;
			for (i=0;i<nr;i++) {
				for (j=0;j<nc;j++) {
					if (*pd++ == m_d) {
						fprintf(stderr,"%d %d %d ",
							f,i,j);
						pfloat(m_d);
					}
				}
				pd += nex;
			}
			return;
	case PFCOMPLEX:	
			if (nflag && min->v_float == 0)
				return;
			pf = (float *) hdp.firstpix;
			m_f = min->v_float;
			for (i=0;i<nr;i++) {
				for (j=0;j<nc;j++) {
					magn_f = sqrt((double) (pf[0]*pf[0]
						+ pf[1]*pf[1]));
					pf += 2;
					if (magn_f == m_f) {
						fprintf(stderr,"%d %d %d ",
							f,i,j);
						pfloat(m_f);
					}
				}
				pf += nex;
			}
			return;
	case PFDBLCOM:	
			if (nflag && min->v_double == 0)
				return;
			pd = (double *) hdp.firstpix;
			m_d = min->v_double;
			for (i=0;i<nr;i++) {
				for (j=0;j<nc;j++) {
					magn_d = sqrt(pd[0]*pd[0] +
						pd[1]*pd[1]);
					pd += 2;
					if (magn_d == m_d) {
						fprintf(stderr,"%d %d %d ",
							f,i,j);
						pfloat(m_d);
					}
				}
				pd += nex;
			}
			return;
	default:	perr(HE_MSG,"unreasonable format in print_coords");
	}
}

void print_min(min,f)

Pixelval *min;
int f;

{
	if (!sflag)
		fprintf(stderr,"%d ",f);
	switch(hdp.pixel_format) {
	case PFBYTE:	fprintf(stderr,"%d\n",min->v_byte); return;
	case PFSBYTE:	fprintf(stderr,"%d\n",min->v_sbyte); return;
	case PFSHORT:	fprintf(stderr,"%d\n",min->v_short); return;
	case PFUSHORT:	fprintf(stderr,"%u\n",min->v_ushort); return;
	case PFINT:	fprintf(stderr,"%d\n",min->v_int); return;
	case PFUINT:	fprintf(stderr,"%u\n",min->v_uint); return;
	case PFFLOAT:
	case PFCOMPLEX:	pfloat(min->v_float); return;
	case PFDOUBLE:
	case PFDBLCOM:	pfloat(min->v_double); return;
	default:	perr(HE_MSG,"unreasonable format in print_min");
	}
}

void pfloat(f)

double f;

{
	if (iflag)
		fprintf(stderr,"%d\n",(int) (f < 0 ? (f - .5) : (f + .5)));
	else
		fprintf(stderr,"%f\n",f);
}
