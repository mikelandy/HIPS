/**************************************************************************
 *                                                                        * 
 * Function: polyfit_line.c()					                        *
 *                                                                        *
 * Usage:	polyfit_line [-d] [-v] [-w] [-c]  < inseq > outseq              *
 * Returns:  none							                        *
 * Defaults:row input: order 3 :no weighting: no collapse of output       *
 * Loads: cc -o -DDG polyfit_line polyfit_line.c -lhipl                   *
 * Modified:TK 2-II-88                                                    *
 *                                                                        *
 * Description:compute a polynomial curve to fit a set of data points from*
 *             a grey-level histogram column or row                       *
 *             -d specifies the order of polynomial fit of the curve      *
 *             -w specifies that data points are to be weighted ( not yet)*
 *             -v specifies that the histogram be made from a column input*
 *             -c causes multiple row/column sequences to collapse into a *
 *             single normalised curve                                    *
 **************************************************************************
 *                    Copyright (c) 1987                                  *
 *                    Captain Chaos                                       *
 **************************************************************************
 */

#include <hipl_format.h>
#include <stdio.h>
#include <math.h>

int cflag = 0; 
int vflag = 0; 
int mflag = 0; 
int wflag = 0; 

/* a function to return maxmin values */
void maxmin(a,b)
int *a, *b;
{
	if (*a < *b) {
		int t= *a ; *a = *b ; *b = t ;
	}
}


int main(argc,argv)
int argc;
char *argv[] ;
{
	int i,j,j1,k;					/* counters*/
	int r,c,fr,f;					/* hips format */
	int ld,ln,m,mm,n;			     /* poly function */
	double p,p1,p2,r2,s1,s2,s3,s4,vr,wt ;
	int *x,*y,*w,*outp;	                         /* poly arrays */
	double *coeff,*d1,*d2,*d3,*d4,*d5,*d6;
	char *iny,*ty;
	struct header hd;

	Progname = strsave(*argv);
	/* initialise polynomial parameters */
	m = mm = n = j1 = 0 ;
	p = p1 = p2 = s1 = s3 = s4 = r2 = s2 = vr = wt = 0.0 ;
	
	/* initialise parameters from input header */
	read_header(&hd);

	if (hd.pixel_format != PFBYTE)
		perr(HE_MSG,"pixel format must be byte");
	r=hd.orows;
	c=hd.ocols;
	f=hd.num_frame;
	hd.pixel_format = PFHIST;
	
	update_header(&hd,argc,argv);

	/* assign input arguments */
	while (--argc > 0) {                /* while arguments remain */
		if ((*++argv)[0] == '-') {  /* check first char of first argv*/
			switch((*argv)[1]) {
			case 'D':
				continue;
			case 'd':
				mflag++;        /* set order flag to 1 (on) */
				(*argv)[0] = (*argv)[1] = '0';
				m = atoi(*argv);     /* order of polynomial */
				continue;
			case 'v':
				vflag++;          /* set col. flag to 1 (on) */
				continue;
			case 'w':
				wflag++;     /* set weighting flag to 1 (on) */
				continue;
			case 'c':
				cflag++;      /* set compact flag to 1 (on) */
				continue;
			default:
				fprintf(stderr,
					"polyfit_line: unknown switch %s\n",
					argv[0]);
				exit(1);
			}
		}
	}

	/* check for single line input */
	if (!vflag) {
		if (r != 1)
			perr(HE_MSG,"input must contain one row");
   		(n = c);		/* if row input n = no. of cols */
   	}
   	else if (vflag) {
   		if (c != 1)
   			perr(HE_MSG,"input must contain one column");
   		(n = r);		/* if column input n = no. of rows*/
   	}

	if (cflag)
		hd.num_frame = 1;

/* Polynomial Function */
	ln = 1000;			/* max no. of data points 1000*/
	ld = 11;			/* max order of polynomial 10*/
	if ( n < 2 || n > ln )		/* check no of data points*/
			perr(HE_MSG,"no. of data points error");
	if (!mflag) m = 3 ;
	else { 
		i = (ld -1); j = (n-1); maxmin ( &i, &j);
		if ( m < 1 || m > j )		/* check order of polynomial */
			perr(HE_MSG,"order of polynomial error");
   	}
	setparam(&hd,"numbin",PFINT,1,n);
	setparam(&hd,"imagepixfmt",PFINT,1,PFBYTE);
	setparam(&hd,"binleft",PFINT,1,0);
	setparam(&hd,"binwidth",PFINT,1,1);
	write_header(&hd);

	if ( (iny = (char *) calloc(n,sizeof(char))) == 0 ||
		(outp = (int *) calloc(n+2,sizeof(int))) == 0 ||
		(y = (int *) calloc(n,sizeof(int))) == 0 ||
		(x = (int *) calloc(n,sizeof(int))) == 0 ||
		(w = (int *) calloc(n,sizeof(int))) == 0 ||
		(coeff = (double *) calloc(ld,sizeof(double))) == 0 ||
		(d1 = (double *) calloc(ld,sizeof(double))) == 0 ||
		(d2 = (double *) calloc(ld,sizeof(double))) == 0 ||
		(d3 = (double *) calloc(ld,sizeof(double))) == 0 ||
		(d4 = (double *) calloc(ld,sizeof(double))) == 0 ||
		(d5 = (double *) calloc(ld,sizeof(double))) == 0 ||
		(d6 = (double *) calloc(ld,sizeof(double))) == 0 )
			perr(HE_MSG,"can't allocate core");

	fprintf(stderr,"order of polynomial: %d\n",m);
	fprintf(stderr,"number of data points: %d\n",n);
	for (i=0;i<n;i++) outp[i+1] = 0 ;

	/* for each frame in the sequence */
	for(fr=0;fr<f;fr++) {		/* read in data from stdin (0) */
		if (fread(iny,n*sizeof(char),1,stdin) != 1)
			perr(HE_MSG,"error during read");
		ty = iny;
		for (i=0;i<n;i++) {	
			j = *ty++ & 0377;	/* convert y data to int */
			y[i] = j ;
			x[i] = i ;			/* assign x data */
			w[i] = 1 ;
		}
		
		/* may include here old data new order*/
		/* if (mf > 0 && m > mm) { j1 = mm+1; mm=m } else { */

		s1 = s2 = s3 = s4 = 0.0 ; mm = m ; j1 = 1 ;
		for (i=1;i<=n;i++) {
			wt = (double)w[i] ;
			s1 = s1 + wt * (double)x[i] ; s2 = s2 + wt ;
			s3 = s3 + wt * (double)y[i] ; 
			s4 = s4+wt * (double)y[i] * (double)y[i] ;
			}
		d4[1] = s1/s2 ; d5[1] = 0 ; d6[1] = s3/s2 ; d1[1] = 0 ;
		d2[1] = 1 ; vr = s4 - s3 * d6[1] ;

		for (j=j1;j<=mm;j++) {
			s1 = s2 = s3 = s4 = 0.0;
			for (i=1;i<=n;i++) {
				p1 = 0 ; p2 = 1 ;
				for (k=1;k<=j;k++) {
					p = p2 ;
					p2 = ((double)x[i] - d4[k]) * p2 -
						d5[k] * p1;
					p1 = p ;
				}
				wt = (double)w[i] ; p = wt * p2 * p2 ;
				s1 = s1 + p * (double)x[i]; s2 = s2 + p ; 
				s3 = s3 + wt * p1 * p1 ;
				s4 = s4 + wt * (double)y[i] *p2 ;
			}
			d4[j+1] = s1/s2 ; d5[j+1] = s2/s3 ; d6[j+1] = s4/s2  ;
			d3[1] = -d4[j] * d2[1] - d5[j] * d1[1];
			if (j >= 4) {
				for (k=2;k<=j-2;k++) {
					d3[k] = d2[k-1] - d4[j] * d2[k] -
						d5[j] * d1[k] ;
				}
			}
			if (j > 2)
				d3[j-1] = d2[j-2] - d4[j] * d2[j-1] - d5[j] ;
			if (j > 1)
				d3[j] = d2[j-1] - d4[j] ;	
			for (k=1;k<=j;k++) {
				d1[k] = d2[k]; d2[k] = d3[k] ;
				d6[k] = d6[k] + d3[k] * d6[j+1];
			}
		}
		for (j=1;j<=m+1;j++)
			coeff[j] = d6[m+2-j];
		p2 = 0.0;
		for (i=0;i<=n;i++) {
			p = coeff[1];
			for (j=1;j<=m;j++) {
				p = p * (double)x[i] + coeff[j+1];
				k = outp[i+1];
				outp[i+1] = k + (int)p;
						/* significant figures ??*/
			}
			p = p - (double)y[i]; p2 = p2 + (double)w[i] * p * p ;
		}
		s2 = 0.0;
		if ( n > m+1) s2 = p2/(n-m-1) ;
		r2 = 1.0 ;
		if ( vr != 0) {
			r2 = 1 - p2/vr ;
			if (r2 < 0) r2 = 0 ;
		}
		/* results of polyfit for each frame */
		fprintf(stderr,"coefficients (c%d constant term last)\n",m+1);
		for (j=1;j<=m+1;j++) {
			fprintf(stderr,"c%d:  %f\n",j,coeff[j]);
		}
		fprintf(stderr,"residual variance: %f\n", s2);
		r2 = (int)(1000000.0 * r2 + 0.5) / 1000000.0;
		fprintf(stderr,"coefficient of determination: %f\n", r2);
	
		/*for (i=0;i<n;i++) {
			fprintf(stderr,"X= %d    Y= %d   P(X)= %d\n",
				x[i],y[i],outp[i+1]);
		}	print of the approximated points    */
		if (!cflag) {
			if (fwrite(outp,(n+2)*sizeof(int),1,stdout) != 1)
				perr(HE_MSG,"error during write");
			for (i=0;i<n;i++) outp[i+1] = 0;
		}
	}
	if (cflag) {
		for (i=0;i<n;i++) outp[i+1] = outp[i+1]/fr;
					/*normalise collapsed  */
		if (fwrite(outp,(n+2)*sizeof(int),1,stdout) != 1)
			perr(HE_MSG,"error during write");
	}
	return(0);
}
