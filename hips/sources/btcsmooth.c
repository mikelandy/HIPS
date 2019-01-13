/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * btcsmooth - smooth a btc image by taking averages over nonoverlapping
 * 		4 x 4 blocks, and smoothing those averages, leaving the
 *		deviations from those averages alone.
 *
 * usage:	btcsmooth <iseq >oseq
 *
 * to load:	cc -o btcsmooth btcsmooth.c -lhips -lm
 *
 * Mike Landy - 1/14/83
 * HIPS 2 - msl - 7/24/91
 */

#include <stdio.h>
#include <hipl_format.h>

#define	apict(ar,ac)	avfr[(ar)*c4+(ac)]
#define	gpict(pr,pc)	(bfr[(pr)*c+(pc)])
#define	pict(pr,pc)	bfr[(pr)*c+(pc)]

int r,c,fr,r4,c4,rc16,*avfr,aneighbor();
byte *bfr;

static Flag_Format flagfmt[] = {
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	int	f,i,j,k,l,n0,n1,n2,n3,n4,n5,n6,n7,n8,dev,sum[16],newpict;
	byte	*bp;
	int	*avp,method;
	struct header hd,hdp;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	r = hd.orows; c = hd.ocols;
	if ((r % 4)!=0 || (c % 4)!=0)
		perr(HE_MSG,"row and column sizes must be a multiple of four");
	r4 = r/4;
	c4 = c/4;
	rc16 = r*c/16;
	fr = hdp.num_frame;
	write_headeru(&hdp,argc,argv);
	bfr = hdp.image;
	avfr = (int *) halloc(rc16,sizeof (int));
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		avp = avfr;
		for (i=0;i<rc16;i++)
			*avp++ = 0;
		bp = bfr;
		for (i=0;i<r;i++)
			for (j=0;j<c;j++)
				*(avfr + (i/4)*c4 + (j/4)) += *bp++;
		for (i=0;i<r4;i++) {
		    for (j=0;j<c4;j++) {
			n0 = apict(i,j);
			n1 = aneighbor(i  ,j+1);
			n2 = aneighbor(i+1,j+1);
			n3 = aneighbor(i+1,j  );
			n4 = aneighbor(i+1,j-1);
			n5 = aneighbor(i  ,j-1);
			n6 = aneighbor(i-1,j-1);
			n7 = aneighbor(i-1,j  );
			n8 = aneighbor(i-1,j+1);
			sum[0] = 6*n0 + 6*n5 + 2*n6 + 2*n7;
			sum[1] = 9*n0 + 3*n5 + 3*n7 + n6;
			sum[2] = 12*n0 + 4*n7;
			sum[3] = 9*n0 + 3*n1 + 3*n7 + n8;
			sum[4] = 8*n0 + 8*n5;
			sum[5] = 12*n0 + 4*n5;
			sum[6] = 16*n0;
			sum[7] = 12*n0 + 4*n1;
			sum[8] = 6*n0 + 6*n5 + 2*n3 + 2*n4;
			sum[9] = 9*n0 + 3*n5 + 3*n3 + n4;
			sum[10] = 12*n0 + 4*n3;
			sum[11] = 9*n0 + 3*n1 + 3*n3 + n2;
			sum[12] = 4*n0 + 4*n3 + 4*n4 + 4*n5;
			sum[13] = 6*n0 + 6*n3 + 2*n4 + 2*n5;
			sum[14] = 8*n0 + 8*n3;
			sum[15] = 6*n0 + 6*n3 + 2*n1 + 2*n2;
			for (k=0;k<4;k++) {
			    for (l=0;l<4;l++) {
				dev = 16*gpict(i*4+k,j*4+l) - n0;
				newpict = ((sum[k*4+l]/16) + dev)/16;
				pict(i*4+k,j*4+l) = newpict < 0 ? 0 :
					(newpict > 255 ? 255 : newpict);
			    }
			}
		    }
		}
		write_image(&hdp,f);
	}
	return(0);
}

int aneighbor(i,j)

int i,j;

{
	if (i < 0)
		i = 0;
	else if (i >= r4)
		i = r4-1;
	if (j < 0)
		j = 0;
	else if (j >= c4)
		j = c4-1;
	return(avfr[i*c4+j]);
}
