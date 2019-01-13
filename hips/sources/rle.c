/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * rle.c - run length encoding of binary sequences
 *
 * usage: rle [-b | -w] [-s] [-g | -a > out_sequence] < input_sequence 
 *
 * -g indicates that actual code should be generated.  -a generates code in
 * ASCII (with no header output).  -s reports statistics on each individual
 * frame.  -b denotes that each line
 * must begin with a black pixel.  If this is not the case, the first pixel is
 * set to black (0).  -w is the same as -b, but for white.  If neither -b nor
 * -w is specified, the first output bit for each line specifies the color.
 *
 * The input must be in byte-unpacked-format with 1 bit per pixel.  If -g is
 * specified the actual code is generated.  It is left shifted and packed,
 * with last word of each frame padded with 0's to the right.  Compression
 * statistics are given on "stderr".  The program computes the number of input
 * bits, output bits and compression ratio.
 *
 * to load:	cc -o rle rle.c -lhips -lm
 *
 * Yoav Cohen - 5/23/83
 * HIPS 2 - msl - 7/23/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"g",{"a",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"a",{"g",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"s",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"b",{"w",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"w",{"b",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};

byte *obuf,*inpic;
int bufindex,bitindex,bytesout=0,encode(),outrun();
h_boolean gsw,asw,bsw,wsw;
void outnib(),outbit(),flushbuf();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo;
	Filename filename;
	FILE *fp;
	int method,r,c,nrc,f,ifr,sumlen,len;
	h_boolean ssw;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&gsw,&asw,&ssw,&bsw,&wsw,
		FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	r=hd.orows; c=hd.ocols; nrc=r*c;
	if (r<=0 || c<=0)
		perr(HE_MSG,"dimensions must be >0");
	if (gsw)
		obuf = (byte *) halloc(512,sizeof(byte));
	method = fset_conversion(&hd,&hdp,types,filename);
	f=hdp.num_frame;
	inpic = hdp.image;
	if (gsw) {
		dup_headern(&hdp,&hdo);
		hdo.pixel_format = wsw ? PFRLEW : (bsw ? PFRLEB : PFRLED);
		write_headeru(&hdo,argc,argv);
	}
	sumlen=0;
	for (ifr=0;ifr<f;ifr++) {
		fread_imagec(fp,&hd,&hdp,method,ifr,filename);
		len = encode(r,c);
		sumlen += len;
		if (ssw&&f>1)
			fprintf(stderr,"%s: frame #%d, CR: %5.3f\n",
				Progname,ifr,(double) len/nrc);
	}

	fprintf(stderr,"%s: Total Compression Ratio = %5.3f\n",
		Progname, (double) sumlen/(f*nrc));
	if (gsw)
		fprintf(stderr,"     %d bytes written out.\n",bytesout);
	return(0);
}

int encode(r,c)

int r,c;

{
	byte *inp,*tinp;
	int len,tail,runlen,ir,i,color;
	
	inp=inpic;
	bufindex=bitindex=0;
	if (gsw)
		obuf[0]=0;
	if (asw)
		printf("\n\n New Frame\n");
	len=0;
	for (ir=0; ir<r; ir++,inp += c) {
		if (asw)
			printf("\nline %3d : ",ir);
		if (bsw||wsw)
			*inp=wsw;
		else {
			outnib((*inp ? 1 : 0),1);
			len++;
		}
		tail=c; tinp=inp; color= *tinp ? 1 : 0; runlen=0;
		for (i=0;i<c;i++) {
			if (*tinp)
				*tinp = 1;
			if ((*tinp++)==color)
				runlen++;
			else {
				len+=outrun(runlen,tail);
				color= 1-color;
				tail -= runlen;
				runlen=1;
			}
		}
		len+=outrun(runlen,tail);
	}
	if (gsw)
		flushbuf();
	return(len);
}

int outrun(runlen,tail)

int runlen,tail;

{
	int logtail;

	if (runlen>tail)
		perr(HE_MSG,"error in outrun()");
	logtail=1; --tail;
	while((tail>>logtail)>0)
		logtail++;
	outnib(runlen-1,logtail);
	return(logtail);
}

void outnib(word,nbits)

int word,nbits;

{
	static int bits[32];
	int i;

	if (nbits>32)
		perr(HE_MSG,"error1 in outnib");
	if (asw) {
		printf("%d(%d) ",word,nbits);
		return;
	}
	if (!gsw)
		return;
	for (i=0;i<nbits;i++) {
		bits[nbits-i-1]=word&01;
		word>>=1;
	}
	for (i=0;i<nbits;i++)
		outbit(bits[i]);
}

void outbit(bit)

int bit;

{
/* on entry bufindex and bitindex point to the new available location */
	if (bit<0 || bit>1)
		perr(HE_MSG,"error in outbit()");
	obuf[bufindex] += bit; bitindex++;
	if (bitindex<8)
		obuf[bufindex] = obuf[bufindex]<<1;
	else  { 
		bufindex++; bitindex=0; 
		if (bufindex>511) {
			fwrite(obuf,512,1,stdout);
		  	bytesout+=512; bufindex=0;
		}
		obuf[bufindex]=0;
	}
}

void flushbuf()

{
	if (bufindex==0 && bitindex==0)
		return;
	if (bitindex>0) {
		obuf[bufindex]=obuf[bufindex]<<(7-bitindex);
		bufindex++;
	}
	fwrite(obuf,bufindex,1,stdout);
	bytesout += (bufindex);
}
