/*
 * txtur.c - calculate textural features on a HIPS-2 image
 *
 * Author: James Darrell McCauley
 *         Texas Agricultural Experiment Station
 *         Department of Agricultural Engineering
 *         Texas A&M University
 *         College Station, Texas 77843-2117 USA
 *
 * HIPS-2 version: Michael Landy - 10/7/92
 *
 * usage:	txtur [-d distance] < iseq
 *
 * pixel formats handled directly: BYTE
 *
 * to load:	cc -o txtur txtur.c -lhips
 *
 * Code written partially taken from pgmtofs.c in the PBMPLUS package
 * by Jef Poskanser (Copyright (C) 1991 by Jef Poskanser), whose original
 * copyright notice follows:
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  This software is provided "as is" without express or
 * implied warranty.
 *
 * Algorithms for calculating features (and some explanatory comments) are
 * taken from:
 *
 *   Haralick, R.M., K. Shanmugam, and I. Dinstein. 1973. Textural features
 *   for image classification.  IEEE Transactions on Systems, Man, and
 *   Cybernetics, SMC-3(6):610-621.
 *
 * However, unlike the Haralick paper, rather than implementing a histogram
 * equalization prior to calculating the matrices, instead, the greyscale is
 * simply compressed toward zero to eliminate unused greylevels.  Also, grey
 * tones are labeled 0 through tones-1 (rather than 1 through tones), so some
 * of the descriptor values change, although their usefulness for
 * discriminations does not.
 *
 * Copyright (C) 1991 Texas Agricultural Experiment Station, employer for
 * hire of James Darrell McCauley
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  This software is provided "as is" without express or
 * implied warranty.
 *
 * THE TEXAS AGRICULTURAL EXPERIMENT STATION (TAES) AND THE TEXAS A&M
 * UNIVERSITY SYSTEM (TAMUS) MAKE NO EXPRESS OR IMPLIED WARRANTIES
 * (INCLUDING BY WAY OF EXAMPLE, MERCHANTABILITY) WITH RESPECT TO ANY
 * ITEM, AND SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL
 * OR CONSEQUENTAL DAMAGES ARISING OUT OF THE POSESSION OR USE OF
 * ANY SUCH ITEM. LICENSEE AND/OR USER AGREES TO INDEMNIFY AND HOLD
 * TAES AND TAMUS HARMLESS FROM ANY CLAIMS ARISING OUT OF THE USE OR
 * POSSESSION OF SUCH ITEMS.
 *
 * Modification History:
 * 24 Jun 91 - J. Michael Carstensen <jmc@imsor.dth.dk> supplied fix for
 *             correlation function.
 */

#include <stdio.h>
#include <hipl_format.h>
#include<math.h>

static Flag_Format flagfmt[] = {
	{"d",{LASTFLAG},1,{{PTINT,"1","distance"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};

#define RADIX 2.0
#define EPSILON 0.000000001
#define BL  "Angle                 "
#define F1  "Angular Second Moment "
#define F2  "Contrast              "
#define F3  "Correlation           "
#define F4  "Variance              "
#define F5  "Inverse Diff Moment   "
#define F6  "Sum Average           "
#define F7  "Sum Variance          "
#define F8  "Sum Entropy           "
#define F9  "Entropy               "
#define F10 "Difference Variance   "
#define F11 "Difference Entropy    "
#define F12 "Meas of Correlation-1 "
#define F13 "Meas of Correlation-2 "
#define F14 "Max Correlation Coeff "

#define SIGN2(x,y) ((y)<0 ? -fabs(x) : fabs(x))
#define SWAP(a,b) {y=(a);(a)=(b);(b)=y;}

void results(),hessenberg(),mkbalanced(),reduction(),simplesrt();
void freematrix(),freevector(),comp_px(),comp_pxmy(),comp_meansd();
float f1_asm(),f2_contrast(),f3_corr(),f4_var(),f5_idm(),
	f6_savg(),f7_svar(),f8_sentropy(),f9_entropy(),f10_dvar(),
	f11_dentropy(),f12_icorr(),f13_icorr(),f14_maxcorr(),*vector(),
	**matrix();

float *px[4],*pxmy[4],**Q[4],*x[4],*iy[4],mean[4],stddev[4],entropy[4];
static int maxtones = 0;
float Log2;


int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp;
	int method,fr,f;
	Filename filename;
	FILE *fp;
	register byte *grays,*pg,*pgr,*pgd,*pgdr,*pgdl;
	int tone[256],invtone[256],R0,R45,R90,d,xx,yy,i;
	int rows,cols,ocols,row,col,nexi;
	int itone,jtone,tones;
	float **P_matrix[4];
	float ASM[4],contrast[4],corr[4],var[4],idm[4],savg[4];
	float sentropy[4],svar[4],dvar[4],dentropy[4];
	float icorr[4],maxcorr[4];

	Progname = strsave(*argv);
	Log2 = log(2.);
	parseargs(argc,argv,flagfmt,&d,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	fr = hdp.num_frame;
	grays = hdp.firstpix;
	rows = hd.rows;
	cols = hd.cols;
	ocols = hd.ocols;
	nexi = ocols - cols;
	for (f=0;f<fr;f++) {
	    fread_imagec(fp,&hd,&hdp,method,f,filename);
	    printf("\nFrame %d results:\n",f);

/* Determine the number of different gray scales */

	    for (i=0;i<256;i++)
		tone[i] = 0;
	    pg = grays;
	    for (row=0;row<rows;row++) {
		for (col=0;col<cols;col++)
			tone[*pg++] = 1;
		pg += nexi;
	    }

/* Collapse array, taking out all zero values */

	    for (i=0,tones=0;i<256;i++)
		if (tone[i])
			invtone[i] = tones++;
	    printf("(Image has %d graylevels.)\n",tones);

/* Now array contains only the gray levels present (in ascending order) */

/* Allocate memory for gray-tone spatial dependence matrix */

	    if (maxtones > 0 && tones > maxtones) {
		for (i=0;i<4;i++) {
			freematrix(P_matrix[i],0,maxtones-1,0,maxtones-1);
			freematrix(Q[i],1,maxtones,1,maxtones);
			freevector(px[i],0,maxtones-1);
			freevector(pxmy[i],0,maxtones-1);
			freevector(x[i],1,maxtones);
			freevector(iy[i],1,maxtones);
		}
	    }
	    if (tones > maxtones) {
		maxtones = tones;
		for (i=0;i<4;i++) {
			P_matrix[i] = matrix(0,tones-1,0,tones-1);
			Q[i] = matrix(1,tones,1,tones);
			px[i] = vector(0,tones-1);
			pxmy[i] = vector(0,tones-1);
			x[i] = vector(1,tones);
			iy[i] = vector(1,tones);
		}
	    }
	    for (itone=0;itone<tones;++itone)
		for (jtone=0;jtone<tones;++jtone)
			for (i=0;i<4;i++)
				P_matrix[i][itone][jtone] = 0;

/* Find gray-tone spatial dependence matrix */

	    fprintf(stderr,"Frame %d, Computing spatial dependence matrix...",
		f);
	    pg = grays;
	    pgr = grays + d;
	    pgd = grays + d*ocols;
	    pgdr = grays + d*ocols + d;
	    pgdl = grays + d*ocols - d;
	    for (row=0;row<rows;++row) {
		for (col=0;col<cols;++col) {
			xx = invtone[*pg++];
			if (col + d < cols) {
				yy = invtone[*pgr++];
				P_matrix[0][xx][yy]++;
				P_matrix[0][yy][xx]++;
				if (row + d < rows) {
					yy = invtone[*pgdr++];
					P_matrix[1][xx][yy]++;
					P_matrix[1][yy][xx]++;
				}
				else
					pgdr++;
			}
			else {
				pgr++;
				pgdr++;
			}
			if (row + d < rows) {
				yy = invtone[*pgd++];
				P_matrix[2][xx][yy]++;
				P_matrix[2][yy][xx]++;
				if (col - d >= 0) {
					yy = invtone[*pgdl++];
					P_matrix[3][xx][yy]++;
					P_matrix[3][yy][xx]++;
				}
				else
					pgdl++;
			}
			else {
				pgd++;
				pgdl++;
			}
		}
		pg += nexi;
		pgr += nexi;
		pgd += nexi;
		pgdr += nexi;
		pgdl += nexi;
	    }

/* Gray-tone spatial dependence matrices are complete */

/* Find normalizing constants */

	    R0 = 2 * rows * (cols - d);
	    R45 = 2 * (rows - d) * (cols - d);
	    R90 = 2 * (rows - d) * cols;

/* Normalize gray-tone spatial dependence matrix */

	    for (itone=0;itone<tones;++itone)
		for (jtone=0;jtone<tones;++jtone) {
			P_matrix[0][itone][jtone] /= R0;
			P_matrix[1][itone][jtone] /= R45;
			P_matrix[2][itone][jtone] /= R90;
			P_matrix[3][itone][jtone] /= R45;
		}

	    fprintf(stderr,"\n");
	    printf("%s         0         45         90        135        Avg\n",
		BL);

	    for (i=0;i<4;i++)
		    comp_px(P_matrix[i],tones,px[i]);

	    for (i=0;i<4;i++)
		    ASM[i] = f1_asm(P_matrix[i],tones);
	    results(F1,ASM);

	    for (i=0;i<4;i++)
		    contrast[i] = f2_contrast(P_matrix[i],tones);
	    results(F2,contrast);

	    for (i=0;i<4;i++) {
		    comp_meansd(tones,i);
		    corr[i] = f3_corr(P_matrix[i],tones,i);
	    }
	    results(F3,corr);

	    for (i=0;i<4;i++)
		    var[i] = stddev[i]*stddev[i];
	    results(F4,var);

	    for (i=0;i<4;i++)
		    idm[i] = f5_idm(P_matrix[i],tones);
	    results(F5,idm);

	    for (i=0;i<4;i++)
		savg[i] = f6_savg(P_matrix[i],tones);
	    results(F6,savg);

	    for (i=0;i<4;i++)
		svar[i] = f7_svar(P_matrix[i],tones,savg[i]);
	    results(F7,svar);

	    for (i=0;i<4;i++)
		sentropy[i] = f8_sentropy(P_matrix[i],tones);
	    results(F8,sentropy);

	    for (i=0;i<4;i++)
		    entropy[i] = f9_entropy(P_matrix[i],tones);
	    results(F9,entropy);

	    for (i=0;i<4;i++)
		    comp_pxmy(P_matrix[i],tones,pxmy[i]);

	    for (i=0;i<4;i++)
		    dvar[i] = f10_dvar(P_matrix[i],tones,pxmy[i]);
	    results(F10,dvar);

	    for (i=0;i<4;i++)
		    dentropy[i] = f11_dentropy(P_matrix[i],tones,pxmy[i]);
	    results(F11,dentropy);

	    for (i=0;i<4;i++)
		    icorr[i] = f12_icorr(P_matrix[i],tones,i);
	    results(F12,icorr);

	    for (i=0;i<4;i++)
		    icorr[i] = f13_icorr(P_matrix[i],tones,i);
	    results(F13,icorr);

	    for (i=0;i<4;i++)
		    maxcorr[i] = f14_maxcorr(P_matrix[i],tones,i);
	    results(F14,maxcorr);
	}
	return(0);
}

/* marginals */

void comp_px(P,Ng,p)

float **P,*p;
int Ng;

{
	int i,j;

	for (i=0;i<Ng;i++)
		p[i] = 0;
/*
 * p[i] is the ith entry in the marginal probability matrix obtained by
 * summing the rows of p[i][j]
 */
	for (i=0;i<Ng;i++)
		for (j=0;j<Ng;j++)
			p[i] += P[i][j];
}

/* Angular Second Moment */

float f1_asm(P,Ng)
float **P;
int Ng;

{
	int i,j;
	float sum = 0;

	for (i=0;i<Ng;++i)
		for (j=0;j<Ng;++j)
			sum += P[i][j] * P[i][j];
	return sum;

/*
 * The angular second-moment feature (ASM) f1 is a measure of homogeneity of
 * the image. In a homogeneous image, there are very few dominant gray-tone
 * transitions. Hence the P matrix for such an image will have fewer entries
 * of large magnitude.
 */
}

/* Contrast */

float f2_contrast(P,Ng)

float **P;
int Ng;

{
	int i,j,n;
	float sum = 0;

	for (i=0;i<Ng;++i) {
		for (j=0;j<Ng;++j)
			if (i != j) {
				n = i - j;
				sum += n*n*P[i][j];
			}
	}
	return sum;

/*
 * The contrast feature is a difference moment of the P matrix and is a
 * measure of the contrast or the amount of local variations present in an
 * image.
 */
}

/* mean, stddev */

void comp_meansd(Ng,mtype)

int Ng,mtype;

{
	int i;
	float sum_sqr=0;

	mean[mtype] = 0;
	for (i=0;i<Ng;i++) {
		mean[mtype] += px[mtype][i] * i;
		sum_sqr += px[mtype][i] * i * i;
	}
	stddev[mtype] = sqrt(sum_sqr - (mean[mtype]*mean[mtype]));
}

/* Correlation */

float f3_corr(P,Ng,mtype)

float **P;
int Ng,mtype;

{
	int i,j;
	float tmp;

	for (tmp=0,i=0;i<Ng;i++)
		for (j=0;j<Ng;j++)
			tmp += i * j * P[i][j];

	return (tmp - mean[mtype]*mean[mtype]) / (stddev[mtype]*stddev[mtype]);

/*
 * This correlation feature is a measure of gray-tone linear-dependencies in
 * the image.
 */
}

/* Inverse Difference Moment */

float f5_idm(P,Ng)

float **P;
int Ng;

{
	int i,j;
	float idm = 0;

	for (i=0;i<Ng;i++)
		for (j=0;j<Ng;j++)
			idm += P[i][j] / (1 + (i - j) * (i - j));
	return idm;
}

/* Sum Average */

float f6_savg(P,Ng)

float **P;
int Ng;

{
	int i,j;
	float savg = 0;

	for (i=0;i<Ng;i++)
		for (j=0;j<Ng;j++)
			savg += (i+j)*P[i][j];
	return savg;
}


/* Sum Variance */

float f7_svar(P,Ng,S)

float **P,S;
int Ng;

{
	int i,j;
	float sum,var = 0;

	for (i=0;i<Ng;i++)
		for (j=0;j<Ng;j++) {
			sum = i + j - S;
			var += sum*sum*P[i][j];
		}
	return var;
}

/* Sum Entropy */

float f8_sentropy(P,Ng)

float **P;
int Ng;

{
	int i,j,count;
	float Pxpy[511];
	float sentropy = 0;

	count = 2*Ng-1;
	for (i=0;i<count;i++)
		Pxpy[i] = 0;
	for (i=0;i<Ng;i++)
		for (j=0;j<Ng;j++)
			Pxpy[i+j] += P[i][j];
	for (i=0;i<count;i++)
		sentropy -= Pxpy[i] * log(Pxpy[i] + EPSILON);
	return sentropy/Log2;
}


/* Entropy */

float f9_entropy(P,Ng)

float **P;
int Ng;

{
	int i,j;
	float entropy = 0;

	for (i=0;i<Ng;i++)
		for (j=0;j<Ng;j++)
			entropy += P[i][j] * log(P[i][j] + EPSILON);

	return -entropy/Log2;
}

void comp_pxmy(P,Ng,p)

float **P,*p;
int Ng;

{
	int i,j;

	for (i=0;i<Ng;i++)
		p[i] = 0;
	for (i=0;i<Ng;i++)
		for (j=0;j<Ng;j++)
			p[abs(i - j)] += P[i][j];
}

/* Difference Variance */

float f10_dvar(P,Ng,p)

float **P,*p;
int Ng;

{
	int i;
	float sum=0,sum_sqr=0;

	for (i=0;i<Ng;i++) {
		sum += i*p[i];
		sum_sqr += i*i*p[i];
	}
	return sum_sqr-(sum*sum);
}

/* Difference Entropy */

float f11_dentropy(P,Ng,p)

float **P,*p;
int Ng;

{
	int i;
	float sum=0;

	for (i=0;i<Ng;i++)
		sum += p[i] * log(p[i] + EPSILON);
	return -sum/Log2;
}

/* Information Measures of Correlation */

float f12_icorr(P,Ng,mtype)

float **P;
int Ng,mtype;

{
	int i,j;
	float hx=0,hxy1=0;

	for (i=0;i<Ng;i++)
		for (j=0;j<Ng;j++)
			hxy1 -=
			  P[i][j]*log(px[mtype][i]*px[mtype][j] + EPSILON);
	for (i=0;i<Ng;i++)
		hx -= px[mtype][i] * log(px[mtype][i] + EPSILON);
	return (entropy[mtype] - (hxy1/Log2))/(hx/Log2);
}

/* Information Measures of Correlation */

float f13_icorr(P,Ng,mtype)

float **P;
int Ng,mtype;

{
	int i,j;
	float hxy2=0;
	for (i=0;i<Ng;i++)
		for (j=0;j<Ng;j++)
			hxy2 -= px[mtype][i]*px[mtype][j]*
				log(px[mtype][i]*px[mtype][j] + EPSILON);
	return (sqrt(fabs(1 - exp(-2.0 * ((hxy2/Log2) - entropy[mtype])))));
}

/* Returns the Maximal Correlation Coefficient */

float f14_maxcorr(P,Ng,mtype)

float **P;
int Ng,mtype;

{
	int i,j,k;
	float **Qm,*pxm;

 /* Find the Q matrix */

	Qm = Q[mtype];
	pxm = px[mtype];
	for (i=0;i<Ng;i++) {
		for (j=0;j<Ng;j++) {
			Qm[i + 1][j + 1] = 0;
			for (k=0;k<Ng;k++)
				Qm[i + 1][j + 1] +=
					P[i][k] * P[j][k] /
					(pxm[i] * pxm[k]);
		}
	}

/* Balance the matrix */
	mkbalanced(Qm,Ng);
/* Reduction to Hessenberg Form */
	reduction(Qm,Ng);
/* Finding eigenvalue for nonsymetric matrix using QR algorithm */
	hessenberg(Qm,Ng,x[mtype],iy[mtype]);
/* Returns the sqrt of the second largest eigenvalue of Q */
	return sqrt(x[mtype][2]);
}

float *vector(nl,nh)

int nl,nh;

{
	float *v;

	v = (float *) malloc((unsigned) (nh - nl + 1) * sizeof (float));
	if (!v)
		fprintf(stderr,"memory allocation failure"),exit(1);
	return v - nl;
}

/* Allocates a float matrix with range [nrl..nrh][ncl..nch] */

float **matrix(nrl,nrh,ncl,nch)

int nrl,nrh,ncl,nch;

{
	int i;
	float **m;

 /* allocate pointers to rows */

	m = (float **) malloc((unsigned) (nrh - nrl + 1) * sizeof (float *));
	if (!m)
		fprintf(stderr,"memory allocation failure"),exit(1);
	m -= nrl;

/* allocate rows and set pointers to them */

	for (i=nrl;i<=nrh;i++) {
		m[i] = (float *) malloc((unsigned) (nch - ncl + 1) *
			sizeof (float));
		if (!m[i])
			fprintf(stderr,"memory allocation failure"),exit(2);
		m[i] -= ncl;
	}

/* return pointer to array of pointers to rows */

	return m;
}

/* Free a previously allocated matrix */

void freematrix(m,nrl,nrh,ncl,nch)

float **m;
int nrl,nrh,ncl,nch;

{
	int i;

	for (i=nrl;i<=nrh;i++)
		free(m[i]+ncl);
	free(m+nrl);
}

/* Free a previously allocated vector */

void freevector(v,nl,nh)

float *v;
int nl,nh;

{
	free(v+nl);
}

void results(c,a)

char *c;
float *a;

{
	int i;

	printf("%s",c);
	for (i=0;i<4;i++)
		printf("% 1.3e ",a[i]);
	printf("% 1.3e\n",(a[0] + a[1] + a[2] + a[3]) / 4);
}

void mkbalanced(a,n)

float **a;
int n;

{
	int last,j,i;
	float s,r,g,f,c,sqrdx;

	sqrdx = RADIX * RADIX;
	last = 0;
	while (last == 0) {
		last = 1;
		for (i=1;i<=n;i++) {
			r = c = 0.0;
			for (j=1;j<=n;j++)
				if (j != i) {
					c += fabs(a[j][i]);
					r += fabs(a[i][j]);
				}
			if (c && r) {
				g = r / RADIX;
				f = 1.0;
				s = c + r;
				while (c < g) {
					f *= RADIX;
					c *= sqrdx;
				}
				g = r * RADIX;
				while (c > g) {
					f /= RADIX;
					c /= sqrdx;
				}
				if ((c + r) / f < 0.95 * s) {
					last = 0;
					g = 1.0 / f;
					for (j=1;j<=n;j++)
						a[i][j] *= g;
					for (j=1;j<=n;j++)
						a[j][i] *= f;
				}
			}
		}
	}
}


void reduction(a,n)

float **a;
int n;

{
	int m,j,i;
	float y,x;

	for (m = 2;m < n;m++) {
		x = 0.0;
		i = m;
		for (j=m;j<=n;j++) {
			if (fabs(a[j][m - 1]) > fabs(x)) {
				x = a[j][m - 1];
				i = j;
			}
		}
		if (i != m) {
			for (j=m-1;j<=n;j++)
				SWAP(a[i][j],a[m][j]);
			for (j=1;j<=n;j++)
				SWAP(a[j][i],a[j][m]);
		}
		if (x) {
			for (i=m+1;i<=n;i++) {
				if ((y = a[i][m - 1])) {
					y /= x;
					a[i][m - 1] = y;
					for (j=m;j<=n;j++)
						a[i][j] -= y * a[m][j];
					for (j=1;j<=n;j++)
						a[j][m] += y * a[j][i];
				}
			}
		}
	}
}

void hessenberg(a,n,wr,wi)

float **a,wr[],wi[];
int n;

{
	int nn,m,l,k,j,its,i,mmin;
	float z,y,x,w,v,u,t,s,r,q,p,anorm;

	anorm = fabs(a[1][1]);
	for (i=2;i<=n;i++)
		for (j=(i-1);j<=n;j++)
			anorm += fabs(a[i][j]);
	nn = n;
	t = 0.0;
	while (nn >= 1) {
	    its = 0;
	    do {
		for (l=nn;l>=2;l--) {
		    s = fabs(a[l - 1][l - 1]) + fabs(a[l][l]);
		    if (s == 0.0)
			s = anorm;
		    if ((float) (fabs(a[l][l - 1]) + s) == s)
			break;
		}
		x = a[nn][nn];
		if (l == nn) {
		    wr[nn] = x + t;
		    wi[nn--] = 0.0;
		}
		else {
		    y = a[nn - 1][nn - 1];
		    w = a[nn][nn - 1] * a[nn - 1][nn];
		    if (l == (nn - 1)) {
			p = 0.5 * (y - x);
			q = p * p + w;
			z = sqrt(fabs(q));
			x += t;
			if (q >= 0.0) {
			    z = p + SIGN2(z,p);
			    wr[nn - 1] = wr[nn] = x + z;
			    if (z)
				wr[nn] = x - w / z;
			    wi[nn - 1] = wi[nn] = 0.0;
			}
			else {
			    wr[nn - 1] = wr[nn] = x + p;
			    wi[nn - 1] = -(wi[nn] = z);
			}
			nn -= 2;
		    }
		    else {
			if (its == 30)
			    fprintf(stderr,
		"Too many iterations to required to find %s\ngiving up\n",
				F14),exit(1);
			if (its == 10 || its == 20) {
			    t += x;
			    for (i=1;i<=nn;i++)
				a[i][i] -= x;
			    s = fabs(a[nn][nn - 1]) + fabs(a[nn - 1][nn - 2]);
			    y = x = 0.75 * s;
			    w = -0.4375 * s * s;
			}
			++its;
			for (m=(nn-2);m>=l;m--) {
			    z = a[m][m];
			    r = x - z;
			    s = y - z;
			    p = (r * s - w) / a[m + 1][m] + a[m][m + 1];
			    q = a[m + 1][m + 1] - z - r - s;
			    r = a[m + 2][m + 1];
			    s = fabs(p) + fabs(q) + fabs(r);
			    p /= s;
			    q /= s;
			    r /= s;
			    if (m == l)
					break;
			    u = fabs(a[m][m - 1]) * (fabs(q) + fabs(r));
			    v = fabs(p) * (fabs(a[m - 1][m - 1]) + fabs(z) +
				fabs(a[m + 1][m + 1]));
			    if ((float) (u + v) == v)
				break;
			}
			for (i=m+2;i<=nn;i++) {
			    a[i][i - 2] = 0.0;
			    if (i != (m + 2))
				a[i][i - 3] = 0.0;
			}
			for (k=m;k<=nn-1;k++) {
			    if (k != m) {
				p = a[k][k - 1];
				q = a[k + 1][k - 1];
				r = 0.0;
				if (k != (nn - 1))
				    r = a[k + 2][k - 1];
				if ((x = fabs(p) + fabs(q) + fabs(r))) {
				    p /= x;
				    q /= x;
				    r /= x;
				}
			    }
			    if ((s = SIGN2(sqrt(p * p + q * q + r * r),p))) {
				if (k == m) {
				    if (l != m)
					a[k][k - 1] = -a[k][k - 1];
				}
				else
				    a[k][k - 1] = -s * x;
				p += s;
				x = p / s;
				y = q / s;
				z = r / s;
				q /= p;
				r /= p;
				for (j=k;j<=nn;j++) {
				    p = a[k][j] + q * a[k + 1][j];
				    if (k != (nn - 1)) {
					p += r * a[k + 2][j];
					a[k + 2][j] -= p * z;
				    }
				    a[k + 1][j] -= p * y;
				    a[k][j] -= p * x;
				}
				mmin = nn < k + 3 ? nn : k + 3;
				for (i=l;i<=mmin;i++) {
				    p = x * a[i][k] + y * a[i][k + 1];
				    if (k != (nn - 1)) {
					p += z * a[i][k + 2];
					a[i][k + 2] -= p * r;
				    }
				    a[i][k + 1] -= p * q;
				    a[i][k] -= p;
				}
			    }
			}
		    }
		}
	    } while (l < nn - 1);
	}
}
