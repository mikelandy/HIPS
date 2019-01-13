/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * hc_bin.c - binary-tree coding
 *
 * usage: hc_bin [-t height [width]] [-s] [-v] [-g > outseq] < inseq 
 *
 * height and width determine the size of the area which is hc_bin-encoded as a
 * tree in the forest.  Each must be a power of 2 and evenly divide the number
 * of rows and columns in the input images, respectively.
 *
 * -g indicates that actual code should be generated.  -s reprts statistics
 * on each individual frame.  -v specifies vertical dominance (otherwise
 * horizontal dominance is the default).
 *
 * The input must be in byte-unpacked-format with 1 bit per pixel.  Also: block
 * dimensions must be powers of 2.  Coding assumptions: 2 bits per black or
 * white pixel, 1 bit per meta symbol. Except at the lowest level where the
 * two pixels are coded as a "nibble" in 1 bit.  Also: if an area is divided
 * into two homogeneous areas, the second area is coded in 1 bit (0), since it
 * must be of color different from that of the first area.  If -g is specified
 * the actual code is generated.  It is left shifted and packed, with last
 * word of each binary-tree padded with 1's to the right.  Compression
 * statistics are given on "stderr".  The program computes the number of input
 * bits; it also counts the number of output symbols, and multiplies it by the
 * number of bits for the output.
 *
 * to load:	cc -o hc_bin hc_bin.c -lhips -lm
 *
 * Yoav Cohen - 10/4/82
 * HIPS 2 - msl - 7/23/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"t",{LASTFLAG},1,{{PTINT,"-1","height"},{PTINT,"-1","width"},
		LASTPARAMETER}},
	{"g",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"s",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"v",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};

byte *pic;
char *symbol,*nbits,*code;
int countp,countw,countb,countn,countm,nc,nrc,nsym;
h_boolean generate,vsw;
int encode(),packsym();
void pushb(),pushw(),pushparen(),pop1(),pushnib();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo;
	Filename filename;
	FILE *fp;
	int method,i,j,r,c,f,len,sumlen,sumbytes,nbytes,height,width,ic,ir;
	int rr,cc,tp,tw,tb,tn,tm;
	h_boolean ssw;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&height,&width,&generate,&ssw,&vsw,
		FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	if (height < 0)
		height=hd.orows;
	if (width < 0)
		width=hd.ocols;
	if (height<=0 || width<=0)
		perr(HE_MSG,"unreasonable block dimensions");
	r=hd.orows; nc=c=hd.ocols;
	nrc=r*c;
	if (r<=0 || c<=0)
		perr(HE_MSG,"frame dimensions must be >0");
	i=1;
	while (i<height) {
		i<<= 1;
		if (i>height) {
			height= i>>1;
			fprintf(stderr,"%s: block height set to %d\n",
				Progname,height);
		}
	}
	i=1;
	while (i<width) {
		i <<=1;
		if (i>width) {
			width= i>>1;
			fprintf(stderr,"%s: block width set to %d\n",
				Progname,width);
		}
	}
	method = fset_conversion(&hd,&hdp,types,filename);
	f=hdp.num_frame;
	pic = hdp.image;
	countp=countw=countb=countn=countm=0;
	sumlen=sumbytes=0;
	if (generate) {
		dup_headern(&hdp,&hdo);
		hdo.pixel_format=PFBT;
		setparam(&hdo,"height",PFINT,1,height);
		setparam(&hdo,"width",PFINT,1,width);
		setparam(&hdo,"vdom",PFINT,1,vsw);
		write_headeru(&hdo,argc,argv);
		symbol = (char *) halloc(nrc,sizeof(char));
		nbits = (char *) halloc(nrc+nrc,sizeof(char));
		code = (char *) halloc(nrc,sizeof(char));
	}

	for(i=0;i<f;i++) {
		fread_imagec(fp,&hd,&hdp,method,i,filename);
		rr=height;
		for(ir=0;ir<r;ir+=rr) {
			cc=width;
			if (ir+rr>r)
				rr >>= 1;
			for (ic=0;ic<c;ic+=cc) {
				if(ic+cc>c)cc >>= 1;
				nsym=0;
				tp=countp;tw=countw;tb=countb;
				tn=countn;tm=countm;
				len=encode(rr,cc,pic+ir*nc+ic);
				if(generate) {
					nbytes=packsym();
					sumbytes+=nbytes;
					fwrite(code,nbytes,1,stdout);
				}
				if (len>=0)
					len=1;
				if (len<0)
					len= (-len);
				sumlen+=len;
				if(sumlen != countp+countw+countb+countn+countm)
				    perr(HE_MSG,
					"inconsistency in counting symbols");
				if (ssw)
					j=(countw-tw+countb-tb)*2+countp-
						tp+countn-tn+countm-tm;
				if (ssw)
					fprintf(stderr,
						"%s: frame %d, CR: %5.3f\n",
						Progname,i,(double)j/(rr*cc));
			}
		}
	}

	i=f*r*c;
	j=(countw+countb)*2+countp+countn+countm;
	fprintf(stderr,
		"%s: in: %d bits, out: %d bits, Total compression: %5.3f \n \
		 parens %d, wpixels %d, bpixels %d, uniform %d, nibbles %d\n",
		Progname,i,j,(double)j/i,countp,countw,countb,countm,countn);

	if (generate)
		fprintf(stderr,"         bytes written %d\n",sumbytes);
	return(0);
}

/* encode a region */

int encode(dimv,dimh,picp)

int dimv,dimh;
byte *picp;

{
	int dimv2,dimh2,c1,c2;
	byte *picp2;

	if (dimv==1 && dimh==1) {
		if (generate) {
			if (*picp==0) {
				countb++;
				pushb();
			}
			else {
				countw++;
				pushw();
			}
		}
		else {
			if (*picp==0)
				countb++;
			else
				countw++;
		}
		return(*picp==0?0:1);
	}
	else {
		if (dimv < dimh || (dimv==dimh && vsw==1)) {
			dimv2 = dimv;
			dimh2 = dimh >> 1;
			picp2 = picp + dimh2;
		}
		else {
			dimv2 = dimv >> 1;
			dimh2 = dimh;
			picp2 = picp + dimv2*nc;
		}
		c2 = encode (dimv2,dimh2,picp2);
		c1 = encode (dimv2,dimh2,picp);
		
		if(c1==c2 && c1>=0) {
			if (c1==0)
				countb--;
			else
				countw--;
			if (generate)
				pop1();
			return(c1);
		}
		if (dimv+dimh==3 ) {
			countw--;countb--; countn++;
			if (generate)
				pushnib();
			countp++;
			return(-2);
		}
		if(c1>=0 && c2>=0) {
			if (c2==0)
				countb--;
			else
				countw--;
			countm++;
			countp++;
			if (generate) {
				symbol[nsym-2]=1;
				nbits[nsym-2]=1;
				pushparen();
			}
			return(-3);
		}
		else {
			countp++;
			if (generate)
				pushparen();
			return( ((c1>=0)?(-1):c1)
				+((c2>=0)?(-1):c2)
				-1 );
		}
	}
}

/* symbols pushing&popping routines.
** coding: 	( 0
**		w 10
**		b 11
*/

void pushb()
{
	symbol[nsym]=3; nbits[nsym++]=2;
}

void pushw()
{
	symbol[nsym]=2; nbits[nsym++]=2;
}

void pushparen()
{
	symbol[nsym]=0; nbits[nsym++]=1;
}

void pop1()
{
	nsym--; 
}

void pushnib()
{
	symbol[nsym-2] = symbol[nsym-1] & 01;
	nbits[nsym-2]=1;
	pop1();
	pushparen();
}

/* packing the symbols */

int packsym() 

{
	int nbytes,obits,ibits,i;
	char bbyte,inbyte;

	nbytes=0; obits=8; nsym--;
	bbyte=0;

	while(nsym>=0) {
		inbyte=symbol[nsym]; ibits=nbits[nsym];
		if (obits>=ibits) {
			bbyte=(bbyte<<ibits)+inbyte;
			obits -= ibits; nsym--;
			if (obits==0) {
				code[nbytes++]=bbyte;
				obits=8;
				bbyte=0;
			}
		}
		else {
			bbyte = (bbyte<<obits) + (inbyte>>(ibits-obits));
			ibits = nbits[nsym] -= obits;
			symbol[nsym] = (inbyte << (8-ibits))>>(8-ibits);
			code[nbytes++]=bbyte; obits=8; bbyte=0;
		}
	}
	if (obits==8)
		return(nbytes);
	else
		for(i=0;i<obits;i++)
			bbyte=(bbyte<<1)+1;
	code[nbytes++]=bbyte;
	return(nbytes);
}
