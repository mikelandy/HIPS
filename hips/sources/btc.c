/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * btc.c - 2-d block truncation coding/encoding
 *
 * usage:	btc -m -v [-p jap [nbloc [filtered-file]]] [-s] <seq >oseq
 * 
 * where -m uses "morrin threshold" instead of mean threshold,
 *       -v uses variable bit planes instead of regular bit planes,
 *       jap is the threshold for use of half bit planes (default=4.0),
 *	 nbloc is the size of a block,
 *	 and the optional "filtered-file" is used to compute thresholds instead
 *	 of the actual image (this hasn't been converted to our format yet*****)
 *	 The -s switch causes outputs of variance histograms, etc.
 *
 * to load:	cc -o btc btc.c -lhips -lm
 *
 * Michael Landy 4/15/82 - copied almost verbatim from programs supplied
 * by O. Robert Mitchell of Purdue.  This program simulates the 2-d encoding/
 * decoding algorithm described in the paper "Multilevel Graphics Representation
 * Using Block Truncation Coding" (Proc. IEEE, 68(7), pp. 868-873, July 1980),
 * and uses only 7 bits to code mu/sigma.  It assumes pixels are 0-255, but
 * actually quantizes assuming 32 grey levels at most.
 *
 * Hips 2 - msl - 7/24/91
 */

#include <stdio.h>
#include <hipl_format.h>
#include <math.h>

byte linegrp[16][512],high,low;
byte line[16][512];
float avg,var,thres;
float avgt,vart;
int mode2,mode1,threscount,bitcount=0;
float blsq,abcount;
float x2,sca,junk[16],delp[16];
h_boolean flags,flag,flag2,flag3;
int npic,ncol,nbloc,ncgroup,ngroup,offset,above,below,bitpl[16],mit[32];
float jap;

static Flag_Format flagfmt[] = {
	{"p",{LASTFLAG},1,{{PTDOUBLE,"32.","jap"},{PTINT,"4","nbloc"},
		{PTFILENAME,"","filtered-file"},LASTPARAMETER}},
	{"m",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"v",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"s",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};
void stats(),val(),quant();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp;
	Filename filename,filtfile;
	FILE *fp,*fp3;
	int method,row,count,ccount,jj,ii,offset1,rcount,frame;
	float bpp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&jap,&nbloc,&filtfile,&flag,&flag2,&flags,
		FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	write_headeru(&hdp,argc,argv);
	flag3 = (strcmp(filtfile,"") != 0);
	sca=1.0;
	thres = 0;
	npic = hd.orows;
	ncol = hd.ocols;
	if ((npic % nbloc != 0) || (ncol % nbloc != 0))
		perr(HE_MSG,"block size must divide picture size");
	if (flag3)
		fp3 = ffopen(filtfile,"r");
	if (flags) {
		fprintf(stderr,
			"BLOCK TRUNCATION CODING STATS(graphics method):\n");
		if (flag) fprintf(stderr,"morrin threshold ");
		else fprintf(stderr,"mean threshold ");
		if (flag2) fprintf(stderr,"var bit planes\n");
		else fprintf(stderr,"regular bit planes\n");
		fprintf(stderr,"picture size: %d x %d\n",npic,ncol);
		fprintf(stderr,"block size: %10d\n",nbloc);
		fprintf(stderr,"morrin constant is: %10.3f\n",sca);
		fprintf(stderr,"break point is: %10.0f\n",jap);
		if (flag3) {
			fprintf(stderr,"filtering used\n");
			fprintf(stderr,"input file for bit plane: %s\n",
				filtfile);
		}
	}
	blsq=nbloc;
	blsq = blsq * blsq;
	ngroup = npic / nbloc;
	ncgroup = ncol / nbloc;

/* for each frame in the sequence */

for (frame=0;frame<hdp.num_frame;frame++) {

	fprintf(stderr,"btc: beginning frame #%d\n",frame);
	abcount = threscount = 0;
	mode1 = mode2 = 0;
	fread_imagec(fp,&hd,&hdp,method,frame,filename);
	for (ccount=0;ccount<ngroup;ccount++) {      /* for each line group */
		for (row=0;row<nbloc;row++) {       /* read in image and fltrd*/
			for (jj=0;jj<ncol;jj++)
				linegrp[row][jj] =
					hdp.image[(ccount*nbloc+row)*ncol + jj];
			if (flag3 != 0)
				fread(&(line[row][0]),ncol,sizeof(byte),fp3);
		}
		offset = 0;
		for (rcount=0;rcount<ncgroup;rcount++) { /* for each block */
			stats(); /* compute bit planes and quant mu/sigma */
			val(); /* compute high/low values in block */
			for (ii=0;ii<nbloc;ii++) {  /* simulate decoder */
				for (jj=0;jj<nbloc;jj++) {
					offset1 = jj + offset;
					count = linegrp[ii][offset1];
					if (bitpl[jj+ii*nbloc] == 1)
						linegrp[ii][offset1] = high;
					else
						linegrp[ii][offset1] = low;
				}
			}
			offset += nbloc;
		}
		for (row=0;row<nbloc;row++) /* output line group */
			fwrite(&(linegrp[row][0]),ncol,sizeof(byte),stdout);
	}
	if (flags) {
		fprintf(stderr,"btc: above count is %10.0f\n",abcount);
		fprintf(stderr,"var thres count is %d\n",threscount);
		fprintf(stderr,"mode1 count(full) is %d\n",mode1);
		fprintf(stderr,"mode2 count(half) is %d\n",mode2);
		fprintf(stderr,"var histo!!!!!!\n");
		for (ii=0;ii<32;ii++) fprintf(stderr,"  %5d %5d \n",ii,mit[ii]);
	}
	bitcount += 7*ngroup*ncgroup+mode1*blsq+mode2*(blsq/2);
}
ii = bitcount/hdp.num_frame;
bpp = ii;
bpp /= npic*ncol;
fprintf(stderr,"btc: total bitcount = %d, bits per frame = %d\n",bitcount,ii);
fprintf(stderr,"btc: bits per pixel = %10.5f\n",bpp);
return(0);
}



void stats()

{
	float sum1,sum2,dum,cum;
	int jrow,jcol,lcol,count,ith;

	sum1 = 0;
	sum2 = 0;

	for (jrow=0;jrow<nbloc;jrow++) {
		for (jcol=0;jcol<nbloc;jcol++) {
			lcol = jcol + offset;
			if (jrow == 0 && lcol != 0) {
				count = linegrp[jrow][lcol-1];
				if (flag3 != 0)
					count = line[jrow][lcol-1];
				ith = count;
			}
			else if (jrow != 0 && lcol == 0) {
				ith = linegrp[jrow-1][lcol];
				if (flag3 != 0)
					ith = line[jrow-1][lcol];
				count = ith;
			}
			else if (jrow == 0 && lcol == 0) {
				count = ith = 0;
			}
			else {
				count = linegrp[jrow-1][lcol];
				ith = linegrp[jrow][lcol-1];
				if (flag3 != 0) {
					count = line[jrow-1][lcol];
					ith = line[jrow][lcol-1];
				}
			}
			cum = count;
			dum = ith;
			cum = (cum + dum)/2.0;
			count = linegrp[jrow][lcol];
			dum = count;
			sum1 = sum1 + dum;
			sum2 = sum2 + (dum * dum);
			if (flag3 != 0) dum = line[jrow][lcol];
			junk[jcol+jrow*nbloc] = dum;
			if (flag == 0) dum = cum = 0;
			delp[jcol+jrow*nbloc] = (dum-cum)/sca;
		}
	}

	/* at this point, 
		sum1 = sum of pixels in the image block
		sum2 = sum of squares of pixels in the image block
		junk = image or filtered image block
		delp = 0 or (pixel - (left + upper neighbor)/2) from image
			or filtered image block (for morrin threshold)
	 */
	lcol = blsq;
	avg = sum1 / blsq;
	x2 = sum2 / blsq;
	var = x2 - (avg*avg);
	var = sqrt(var);
	avgt = avg;
	vart = var;
	quant();

	/* avgt,vart are actual average and variance, avg,var are 
	   quantized so that combined it takes 7 bits to encode them */

	if (var == 0.0) { /* if variance is zero, high=low, so don't bother
				computing bit plane */
		above = 5;
		thres = avgt;
		return;
	}
	thres = avgt;
	above = 0;
	for (jrow=0;jrow<lcol;jrow++) { /* compute bit plane */
		if (junk[jrow] > thres - delp[jrow]) {
			above++;
			bitpl[jrow] = 1;
		}
		else bitpl[jrow] = 0;
	}
	if (flag2 == 0) return;
	if (var >= jap) {
		mode1 += 1;
		return;
	}
	if (var < 8.0) {
		var = 0.0;
		above = 5;
		thres = avgt;
		return;
	}

	mode2 += 1; /* mode2 - transmit half the bits */
	if ((bitpl[0] == 1 && bitpl[3] == 1) ||
		(bitpl[5] == 1 && bitpl[9] == 1)) bitpl[1] = 1;
					else	bitpl[1] = 0;
	if ((bitpl[0] == 1 && bitpl[3] == 1) ||
		(bitpl[6] == 1 && bitpl[10] == 1)) bitpl[2] = 1;
					else	bitpl[2] = 0;
	if ((bitpl[0] == 1 && bitpl[12] == 1) ||
		(bitpl[5] == 1 && bitpl[6] == 1)) bitpl[4] = 1;
					else	bitpl[4] = 0;
	if ((bitpl[15] == 1 && bitpl[3] == 1) ||
		(bitpl[5] == 1 && bitpl[6] == 1)) bitpl[7] = 1;
					else	bitpl[7] = 0;
	if ((bitpl[0] == 1 && bitpl[12] == 1) ||
		(bitpl[10] == 1 && bitpl[9] == 1)) bitpl[8] = 1;
					else	bitpl[8] = 0;
	if ((bitpl[15] == 1 && bitpl[3] == 1) ||
		(bitpl[10] == 1 && bitpl[9] == 1)) bitpl[11] = 1;
					else	bitpl[11] = 0;
	if ((bitpl[12] == 1 && bitpl[15] == 1) ||
		(bitpl[5] == 1 && bitpl[9] == 1)) bitpl[13] = 1;
					else	bitpl[13] = 0;
	if ((bitpl[12] == 1 && bitpl[15] == 1) ||
		(bitpl[6] == 1 && bitpl[10] == 1)) bitpl[14] = 1;
					else	bitpl[14] = 0;
}



void val() /* compute high and low */

{
	float dum,cum,fact;

	if (var == 0) {
		high = avg;
		low = avg;
		threscount++;
		return;
	}
	if (above == blsq || above == 0) {
		high = low = avg;
		abcount += 1.;
		return;
	}
	dum = above;
	fact = (blsq - dum) / dum;
	dum = sqrt(fact);
	cum = sqrt(1./fact);
	dum = avg + var * dum;
	cum = avg - var * cum;
	if (vart == 0) dum = cum = avg;
	if (dum < 0.) dum = 0.;
	if (dum > 255.) dum = 255.;
	if (cum < 0.) cum = 0.;
	if (cum > 255.) cum = 255.;
	high = dum;
	low = cum;
}



void quant()

{
	int jj,kk;
	float z;

/*	PUT THE MEAN AND DEV. QUANT IN HERE !!!!!!*/
	z = (var/8.0)+.5;
	kk = z;
	mit[kk]++;
	z = (avg/8.0)+.5;
	jj = z;

	if (kk == 0) {
		if (jj<=0) jj = 0;
		else if (jj>=31) jj = 31;
	}
	else if (kk == 1) {
		if (jj <= 1) jj = 1;
		else if (jj <= 22);
		else if (jj <= 24) jj = 24;
		else if (jj <= 26) jj = 26;
		else if (jj <= 28) jj = 28;
		else jj = 30;
	}
	else if (kk == 2) {
		if (jj<=2) jj = 1;
		else if (jj <= 4) jj = 3;
		else if (jj <= 6) jj = 5;
		else if (jj <= 8) jj = 7;
		else if (jj <= 10) jj = 9;
		else if (jj <= 12) jj = 11;
		else if (jj <= 14) jj = 13;
		else if (jj <= 16) jj = 15;
		else if (jj <= 18) jj = 17;
		else if (jj <= 20) jj = 19;
		else if (jj <= 22) jj = 21;
		else if (jj <= 24) jj = 23;
		else if (jj <= 26) jj = 25;
		else if (jj <= 28) jj = 27;
		else if (jj <= 30) jj = 29;
		else jj = 31;
	}
	else if (kk == 3) {
		if (jj<=3) jj = 2;
		else if (jj<=6) jj = 5;
		else if (jj<=9) jj = 8;
		else if (jj<=12) jj = 11;
		else if (jj<=15) jj = 14;
		else if (jj<=18) jj = 17;
		else if (jj<=21) jj = 20;
		else if (jj<=24) jj = 23;
		else if (jj<=27) jj = 26;
		else jj = 29;
	}
	else if (kk == 4) {
		if (jj<=3) jj = 2;
		else if (jj<=6) jj = 5;
		else if (jj<=9) jj = 8;
		else if (jj<=12) jj = 11;
		else if (jj<=15) jj = 14;
		else if (jj<=18) jj = 17;
		else if (jj<=21) jj = 20;
		else if (jj<=24) jj = 23;
		else if (jj<=27) jj = 26;
		else jj = 29;
	}
	else if (kk <= 6) {
		kk = 5;
		if (jj<=4) jj = 2;
		else if (jj<=8) jj = 6;
		else if (jj<=12) jj = 10;
		else if (jj<=16) jj = 14;
		else if (jj<=20) jj = 18;
		else if (jj<=24) jj = 22;
		else if (jj<=28) jj = 26;
		else jj = 30;
	}
	else if (kk <= 8) {
		kk = 7;
		if (jj<=4) jj = 2;
		else if (jj<=8) jj = 6;
		else if (jj<=12) jj = 10;
		else if (jj<=16) jj = 14;
		else if (jj<=20) jj = 18;
		else if (jj<=24) jj = 22;
		else if (jj<=28) jj = 26;
		else jj = 30;
	}
	else if (kk<=10) {
		kk = 9;
		if (jj<=5) jj = 3;
		else if (jj<=10) jj = 8;
		else if (jj<=15) jj = 13;
		else if (jj<=20) jj = 18;
		else if (jj<=25) jj = 23;
		else jj = 28;
	}
	else if (kk<=12) {
		kk = 11;
		if (jj<=7) jj = 4;
		else if (jj<=13) jj = 10;
		else if (jj<=19) jj = 16;
		else if (jj<=25) jj = 22;
		else jj = 28;
	}
	else if (kk<= 14) {
		kk = 13;
		if (jj<=9) jj = 6;
		else if (jj<=15) jj = 12;
		else if (jj<=21) jj = 19;
		else jj = 26;
	}
	else {
		kk = 15;
		if (jj<=12) jj = 10;
		else if (jj<=18) jj = 16;
		else jj = 22;
	}
	avg = jj << 3;
	var = kk << 3;
}
