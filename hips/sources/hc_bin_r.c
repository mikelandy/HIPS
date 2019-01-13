/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * hc_bin_r.c - decode an hc_bin-coded sequence into a byte formatted sequence
 *
 * usage: hc_bin_r [-d [delta [greylevel]]]  < input_tree >output_sequence
 *
 * -d is used to create a displayable picture of the decomposition into
 * sub-blocks. The boundaries are painted in gray.
 * -d is used to create a displayable picture of the decomposition into
 * sub-blocks. The boundaries are painted in gray.  If delta is specified, then
 * each lower level boundary is delta grey-levels lower than the previous
 * level's boundary. Delta defaults to zero. Greylevel specifies the grey level
 * of level 0, which defaults to 168.
 *
 * The input must be in the special PFBT format.  Coding assumptions: 
 * Mapping: {W,B,G}-->{10,11,0}. At the lowest level the pixels are coded in 1
 * bit per pair.  The code is assumed to be left shifted and packed, with last
 * word of each binary-tree padded with 0's to the right.  The number of bits
 * read is given on "stderr".
 *
 * to load: cc -o hc_bin_r hc_bin_r.c -lhips -lm
 *
 * Mike Landy - 10/1/83
 * Hips 2 - msl - 7/23/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"d",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},{PTINT,"0","delta"},
		{PTINT,"168","greylevel"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBT,LASTTYPE};

#define M 4

int nbits,inbyte,readbits,nc,nrc,height,width,delta,greyl,ncm1,ncm1sq,M1;
byte *pic;
h_boolean dflag,vsw;
FILE *fp;
int decode(),ddecode(),getbit(),seebit();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdo;
	Filename filename;
	h_boolean dflag;
	int i,r,c,f,it1,it2,one=1;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&dflag,&delta,&greyl,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_cpf(fp,&hd,types,filename);
	getparam(&hd,"height",PFINT,&one,&height);
	getparam(&hd,"width",PFINT,&one,&width);
	getparam(&hd,"vdom",PFINT,&one,&vsw);
	r=hd.orows; nc=c=hd.ocols; nrc=r*c; f=hd.num_frame;
	if (r%height!=0 || c%width!=0)
		perr(HE_MSG,"image size inconsistent with stack size");
	dup_headern(&hd,&hdo);
	setformat(&hdo,PFBYTE);
	if (dflag) {
		setsize(&hdo,r*M-1,c*M-1);
		alloc_imagez(&hdo);
	}
	else
		alloc_image(&hdo);
	write_headeru(&hdo,argc,argv);
	pic = hdo.image;
	nbits=0; readbits=0;
	ncm1=nc*M-1; ncm1sq=ncm1*(r*M-1); M1=M-1;

	for (i=0;i<f;i++) {
		for (it1=0;it1<r/height;it1++)
		    for (it2=0;it2<c/width;it2++) {
			nbits=0;
			if (dflag)
				ddecode(0,height,width,
					pic+it1*height*M*(c*M-1)+it2*width*M);
			 else
				decode(height,width,pic+it1*c*height+it2*width);
		    }
		write_image(&hdo,i);
	}
	fprintf(stderr,"%s: read %d bits\n",Progname,readbits);
	return(0);
}

/* decode a region */

int decode(dimh,dimw,picp)

int dimh,dimw;
byte *picp;

{
	int dimh2,dimw2,c1,c2,j,k,bit;
	byte *tp1,*tp2;

	bit=getbit();
	if (dimh==1 && dimw==1) {
		*picp = bit ? hips_lchar : hips_hchar;
		return(bit);
	}
	if (bit == 1) {
		bit=getbit();
		for (j=0,tp1=picp;j<dimh;j++,tp1+=nc)
			for (k=0,tp2=tp1;k<dimw;k++)
				*tp2++  = bit ? hips_lchar : hips_hchar;
		return(bit);
	}

	/* code for non-uniform area */

	dimh2=dimh;dimw2=dimw;
	
	if (dimh<dimw || (dimh==dimw && vsw==1)) {	/* vertical cut */
		dimw2>>=1; tp1=picp+dimw2;
	}
	else {						/* horizontal cut */
		dimh2>>=1; tp1=picp+dimh2*nc;
	}

	c1=decode(dimh2,dimw2,picp);

	if (dimh2==1 && dimw2==1)
		*tp1=c1;
	else if (c1>=0) {
		c2=seebit();
		if (c2) {
			getbit();
			for (j=0;j<dimh2;j++,tp1+=nc)
				for (k=0,tp2=tp1;k<dimw2;k++)
					*tp2++ = c1 ? hips_hchar : hips_lchar;
		}
		else
			decode(dimh2,dimw2,tp1);
	}
	else
		decode(dimh2,dimw2,tp1);
	return(-1);
}

int ddecode(level,dimh,dimw,picp)

int level,dimh,dimw;
byte *picp;

{
	int dimh2,dimw2,c1,c4,j,k,bit,color,gcolor;
	byte *tp1,*tp2;

	gcolor = greyl-level*delta;
	bit=getbit();
	if (dimh==1 && dimw==1) {
		color=(bit)? hips_lchar : hips_hchar;
		for (j=0,tp1=picp;j<M1;j++,tp1+=ncm1)
			for (k=0,tp2=tp1;k<M1;k++)
				*tp2++ = color;
		return(bit);
	}
	if (bit == 1) {
		bit=getbit();
		color=(bit)? hips_lchar : hips_hchar;
		for (j=0,tp1=picp;j<dimh*M-1;j++,tp1+=ncm1)
			for (k=0,tp2=tp1;k<dimw*M-1;k++)
				*tp2++ = color;
		return(bit);
	}

	/* code for non-uniform area */
	dimh2=dimh;dimw2=dimw;
	
	if (dimh<dimw || (dimh==dimw && vsw==1)) {	/* vertical cut */
		dimw2>>=1;
		for (j=0,tp1=picp+dimw2*M-1;j<dimh*M-1;j++) {
			*tp1 = gcolor;
			tp1 += ncm1;
		}
		tp1=picp+dimw2*M;
	}
	else {						/* horizontal cut */
		dimh2>>=1;
		for (j=0,tp1=picp+(dimh2*M-1)*ncm1;j<dimw*M-1;j++)
			*tp1++ = gcolor;
		tp1=picp+dimh2*M*ncm1;
	}

	c1=ddecode(level+1,dimh2,dimw2,picp);

	if (dimh2==1 && dimw2==1) {
		color=(c1)? hips_hchar : hips_lchar;
		for (j=0;j<M1;j++,tp1+=ncm1)
			for (k=0,tp2=tp1;k<M1;k++)
				*tp2++ = color;
	}
	else if (c1>=0) {
		c4=seebit();
		if (c4) {
			getbit();
			color=(c1)? hips_hchar : hips_lchar;
			for (j=0;j<dimh2*M-1;j++,tp1+=ncm1)
				for (k=0,tp2=tp1;k<dimw2*M-1;k++)
					*tp2++ = color;
		}
		else
			ddecode(level+1,dimh2,dimw2,tp1);
	}
	else
		ddecode(level+1,dimh2,dimw2,tp1);
	return(-1);
}

int getbit()

{
	int bit;

	if (nbits==0) {
		inbyte=getc(fp);
		if (inbyte==EOF) {
			fprintf(stderr,"%s: readbits=%d\n",Progname,readbits);
			perr(HE_MSG,"unexpected EOF in middle of tree");
		}
		nbits=8;
	}
	nbits--; readbits++;
	bit=(inbyte>>7)&01;
	inbyte<<= 1;
	return(bit);
}

int seebit()

{
	int bit;

	if (nbits==0) {
		bit=getc(fp);
		if (bit==EOF) {
			fprintf(stderr,"%s: readbits=%d\n",Progname,readbits);
			perr(HE_MSG,"unexpected EOF in middle of tree");
		}
		if (ungetc(bit,fp)==EOF)
			perr(HE_MSG,"cannot push char back");
		bit>>=7; bit &= 01;
		return(bit);
	}
	bit=(inbyte>>7)&01;
	return(bit);
}
