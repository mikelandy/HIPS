/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * rle_r.c - decode an rle-coded sequence into a byte formatted sequence.
 *
 * usage: rle_r  < input_code >output_sequence
 *
 * The input must be in the special PFRLED/PFRLEW/PFRLEB format.
 * The code is assumed to be left shifted and packed, with last word of each
 * frame padded with 0's to the right.
 * The number of bits read is given on "stderr".
 *
 * to load: cc -o rle_r rle_r.c -lhips -lm
 *
 * Yoav Cohen - 5/24/83
 * Hips 2 - msl - 7/23/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PFRLED,PFRLEB,PFRLEW,LASTTYPE};

byte *pic;
int nbits,inbyte,readbits,nc,nrc,getnib(),getbit();
h_boolean wsw=FALSE,bsw=FALSE;
FILE *fp;
void decode();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdo;
	Filename filename;
	int i,r,c,f;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_cpf(fp,&hd,types,filename);
	r=hd.orows; nc=c=hd.ocols;
	if (hd.pixel_format==PFRLEW)
		wsw = TRUE;
	if (hd.pixel_format==PFRLEB)
		bsw = TRUE;
	if (r<=0 || c<0)
		perr(HE_MSG,"dimensions must be >0");
	f=hd.num_frame;
	dup_headern(&hd,&hdo);
	setformat(&hdo,PFBYTE);
	alloc_image(&hdo);
	write_headeru(&hdo,argc,argv);
	pic = hdo.image;
	nbits=0; readbits=0;
	for (i=0;i<f;i++) {
		nbits=0;
		decode(r,c,pic);
		write_image(&hdo,i);
	}
	fprintf(stderr,"%s: read %d bits\n",Progname,readbits);
	return(0);
}

/* decode a region */

void decode(r,c,picp)

int r,c;
byte *picp;

{
	int i,tail,color,j,n;
	byte *tp,*pix;

	for (i=0,tp=picp;i<r;i++,tp+=c) {
		if (bsw||wsw)
			color=wsw;
		else
			color=getbit();
		tail=c; pix=tp;
		while(tail>0) {
			n=getnib(tail);
			for(j=0;j<n;j++)
				*pix++ = color ? hips_hchar : hips_lchar;
			color=1-color;
			tail -= n;
		}
	}
}

int getnib(tail)

int tail;

{
	int logtail,run,i;

	--tail; logtail=1;
	while((tail>>logtail)>0)
		logtail++;
	run=0;
	for (i=0;i<logtail;i++)
		run=(run<<1)+getbit();
	return(run+1);
}

int getbit()

{
	int bit;

	if (nbits==0) {
		inbyte=getc(fp);
		if (inbyte==EOF) {
			fprintf(stderr,"%s: readbits=%d\n",Progname,readbits);
			perr(HE_MSG,"unexpected EOF in middle of frame");
		}
		nbits=8;
	}
	nbits--; readbits++;
	bit=(inbyte>>7)&01;
	inbyte<<= 1;
	return(bit);
}
