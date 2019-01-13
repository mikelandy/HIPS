/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * ahc3_r.c - decode an ahc3-coded sequence into a byte formatted sequence
 *
 * usage: ahc3_r [-d [delta [greylevel]]]  < input_tree >output_sequence
 *
 * -d is used to create a displayable picture of the decomposition into
 * sub-blocks. The boundaries are painted in gray.  If delta is
 * specified, then each lower level boundary is delta grey-levels lower
 * than the previous level's boundary.  Delta defaults to zero.
 * Greylevel sprecifies the grey level of level 0, which defaults to 168.
 *
 * The input must be in the special PFAHC3 format.  Coding assumptions:
 * Mapping: {W,B,V,H,D}-->{10,11,00,010,011}. At the lowest level the 2
 * pixels are coded in 1 bit.  When a subdivision is possible in only one
 * or two directions, less bits are used. (see ahc3.c).  The code is
 * assumed to be left shifted and packed, with last word of each
 * ahc-tree padded with 0's to the right.  The number of bits read is given on
 * "stderr".
 *
 * to load: cc -o ahc3_r ahc3_r.c -lhips -lm
 *
 * Yoav Cohen - 4/15/83
 * modified by Mike Landy - 6/21/83
 * Hips 2 - msl - 7/22/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"d",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},{PTINT,"0","delta"},
		{PTINT,"168","greylevel"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFAHC3,LASTTYPE};

#define H 1
#define V 0
#define M 4

int nbits,inbyte,readbits,nc,nrc,stackrow,stackcol,stackdepth,ncm1,ncm1sq,M1;
int delta,greyl,decode(),ddecode(),getbit(),seebit();
byte *pic;
h_boolean dflag;
FILE *fp;

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdo;
	Filename filename;
	int i,r,c,f,one=1;
	int it1,it2;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&dflag,&delta,&greyl,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_cpf(fp,&hd,types,filename);
	getparam(&hd,"stackrow",PFINT,&one,&stackrow);
	getparam(&hd,"stackcol",PFINT,&one,&stackcol);
	getparam(&hd,"stackdepth",PFINT,&one,&stackdepth);
	r=hd.orows; nc=c=hd.ocols; nrc=r*c; f=hd.num_frame;
	if (f%stackdepth != 0 || r%stackrow!=0 || c%stackcol!=0)
		perr(HE_MSG,"image dimensions inconsistent with stack size");
	dup_headern(&hd,&hdo);
	setformat(&hdo,PFBYTE);
	if (dflag) {
		setsize(&hdo,(r*M-1) * stackdepth*M,c*M-1);
			/* fake allocation of stackdepth */
		hd.num_frame = f*M;
		alloc_imagez(&hdo);
		setsize(&hdo,(r*M-1),c*M-1);
	}
	else {
		setsize(&hdo,r*stackdepth,c);
		alloc_image(&hdo);
		setsize(&hdo,r,c);
	}
	write_headeru(&hdo,argc,argv);
	if (dflag)
		setsize(&hdo,(r*M-1) * stackdepth*M,c*M-1);
	else
		setsize(&hdo,r*stackdepth,c);
	pic = hdo.image;
	nbits=0; readbits=0;
	ncm1=nc*M-1; ncm1sq=ncm1*(r*M-1); M1=M-1;

	for (i=0;i<f/stackdepth;i++) {
	    for (it1=0;it1<r/stackrow;it1++)
		for (it2=0;it2<c/stackcol;it2++) {
			nbits=0;
			if (dflag)
				ddecode(0,stackrow,stackcol,stackdepth,
				    pic+it1*stackrow*M*(c*M-1)+it2*stackcol*M);
			 else
				decode(stackrow,stackcol,stackdepth,
				    pic+it1*c*stackrow+it2*stackcol);
		}
		write_image(&hdo,i);
	}
	fprintf(stderr,"%s: read %d bits\n",Progname,readbits);
	return(0);
}

/* decode a region */

int decode(dimv,dimh,dimd,picp)

int dimv,dimh,dimd;
byte *picp;

{
	int dimh2,dimv2,dimd2,c1,c2;
	int i,j,k,bit,dirs;
	byte *tp1,*tp2;

	bit=getbit();
	if (dimv==1 && dimh==1 && dimd==1) {
		*picp= bit ? hips_lchar : hips_hchar;
		return(bit);
	}
	if (bit == 1) {
		bit=getbit();
		for (i=0;i<dimd;i++,picp+=nrc)
		for (j=0,tp1=picp;j<dimv;j++,tp1+=nc)
		for (k=0,tp2=tp1;k<dimh;k++)
			*tp2++ = bit ? hips_lchar : hips_hchar;
		return(bit);
	}

	/* code for non-uniform area */
	dimv2=dimv;dimh2=dimh;dimd2=dimd;
	dirs=0; if (dimv>1)dirs++;if (dimh>1)dirs++;if (dimd>1)dirs++;
	/* all three directions are possible: */
	if (dirs==3) {
		bit=getbit();
		if (bit==0) {/* vertical cut */
			dimh2>>=1; tp1=picp+dimh2;
		}
		else {
			bit=getbit();
			if (bit==0) {/*horizontal cut */
				dimv2>>=1; tp1=picp+dimv2*nc;
			}
			else {/* cut in depth */
				dimd2>>=1; tp1=picp+dimd2*nrc;
			}
		}
	}	
	/* One direction is possible */
	else if (dirs==1) {
		if (dimd>1) {/* cut in depth */
			dimd2>>=1; tp1=picp+dimd2*nrc;
		}
		else if (dimh>1) {/* vertical cut */
			dimh2>>=1; tp1=picp+dimh2;
		}
		else if (dimv>1) {/*horizontal cut */
			dimv2>>=1; tp1=picp+dimv2*nc;
		}
		else perr(HE_MSG,"error 2 in encode()");
	}
	/* Two directions are posssible */
	else if (dirs==2) {
		bit=getbit();
		if (dimd==1) {
			if (bit==0) {/* vertical cut */
				dimh2>>=1; tp1=picp+dimh2;
			}
			else {/*horizontal cut */
				dimv2>>=1; tp1=picp+dimv2*nc;
			}
		}
		if (dimv==1) {
			if (bit==0) {/* vertical cut */
				dimh2>>=1; tp1=picp+dimh2;
			}
			else {/* cut in depth */
				dimd2>>=1; tp1=picp+dimd2*nrc;
			}
		}
		if (dimh==1) {
			if (bit==0) {/*horizontal cut */
				dimv2>>=1; tp1=picp+dimv2*nc;
			}
			else {/* cut in depth */
				dimd2>>=1; tp1=picp+dimd2*nrc;
			}
		}
	}
	else perr(HE_MSG,"error 3 in decode()");

	c1=decode(dimv2,dimh2,dimd2,picp);

	if (dimv2==1 && dimh2==1 && dimd2==1) *tp1=c1 ? hips_hchar : hips_lchar;
	else if (c1>=0) {
		c2=seebit();
		if (c2) {
			getbit();
			picp=tp1;
			for (i=0;i<dimd2;i++,picp+=nrc)
			for (j=0,tp1=picp;j<dimv2;j++,tp1+=nc)
			for (k=0,tp2=tp1;k<dimh2;k++)
				*tp2++ = c1 ? hips_hchar : hips_lchar;
		}
		else decode(dimv2,dimh2,dimd2,tp1);
	}
	else decode(dimv2,dimh2,dimd2,tp1);
	return(-1);
}

int ddecode(level,dimv,dimh,dimd,picp)

int level,dimv,dimh,dimd;
byte *picp;

{
	int dimh2,dimv2,dimd2,c1,c2;
	int i,j,k,bit,dirs;
	int color,gcolor;
	byte *tp1,*tp2;

	gcolor = greyl-level*delta;
	bit=getbit();
	if (dimv==1 && dimh==1 && dimd==1) {
		color = bit ? hips_lchar : hips_hchar;
		for (i=0;i<M1;i++,picp+=ncm1sq)
		for (j=0,tp1=picp;j<M1;j++,tp1+=ncm1)
		for (k=0,tp2=tp1;k<M1;k++)
			*tp2++=color;
		return(bit);
	}
	if (bit == 1) {
		bit=getbit();
		color = bit ? hips_lchar : hips_hchar;
		for (i=0;i<dimd*M-1;i++,picp+=ncm1sq)
		for (j=0,tp1=picp;j<dimv*M-1;j++,tp1+=ncm1)
		for (k=0,tp2=tp1;k<dimh*M-1;k++)
			*tp2++ = color;
		return(bit);
	}

	/* code for non-uniform area */
	dimv2=dimv;dimh2=dimh;dimd2=dimd;
	dirs=0; if (dimv>1)dirs++;if (dimh>1)dirs++;if (dimd>1)dirs++;
	/* all three directions are possible: */
	if (dirs==3) {
		bit=getbit();
		if (bit==0) {/* vertical cut */
			goto vcut;
		}
		else {
			bit=getbit();
			if (bit==0) {/*horizontal cut */
				goto hcut;
			}
			else {/* cut in depth */
				goto dcut;
			}
		}
	}	
	/* One direction is possible */
	else if (dirs==1) {
		if (dimd>1) {/* cut in depth */
			goto dcut;
		}
		else if (dimh>1) {/* vertical cut */
			goto vcut;
		}
		else if (dimv>1) {/*horizontal cut */
			goto hcut;
		}
		else perr(HE_MSG,"error 2 in encode()");
	}
	/* Two directions are posssible */
	else if (dirs==2) {
		bit=getbit();
		if (dimd==1) {
			if (bit==0) {/* vertical cut */
				goto vcut;
			}
			else {/*horizontal cut */
				goto hcut;
			}
		}
		if (dimv==1) {
			if (bit==0) {/* vertical cut */
				goto vcut;
			}
			else {/* cut in depth */
				goto dcut;
			}
		}
		if (dimh==1) {
			if (bit==0) {/*horizontal cut */
				goto hcut;
			}
			else {/* cut in depth */
				goto dcut;
			}
		}
	}
	else perr(HE_MSG,"error 3 in ddecode()");

hcut:
	dimv2>>=1;
	for (i=0,tp1=picp+(dimv2*M-1)*ncm1;i<dimd*M-1;i++,tp1+=ncm1sq)
		for (j=0,tp2=tp1;j<dimh*M-1;j++)
			*tp2++ = gcolor;
	tp1 = picp+dimv2*M*ncm1;
	goto donecut;
vcut:
	dimh2>>=1;
	for (i=0,tp1=picp+(dimh2*M-1);i<dimd*M-1;i++,tp1+=ncm1sq)
		for (j=0,tp2=tp1;j<dimv*M-1;j++,tp2+=ncm1)
			*tp2 = gcolor;
	tp1 = picp+dimh2*M;
	goto donecut;
dcut:
	dimd2>>=1;
	for (i=0,tp1=picp+(dimd2*M-1)*ncm1sq;i<dimv*M-1;i++,tp1+=ncm1)
		for (j=0,tp2=tp1;j<dimh*M-1;j++)
			*tp2++ = gcolor;
	tp1 = picp+dimd2*M*ncm1sq;
donecut:
	c1=ddecode(level+1,dimv2,dimh2,dimd2,picp);

	if (dimv2==1 && dimh2==1 && dimd2==1) {
		picp = tp1;
		for (i=0;i<M-1;i++,picp+=ncm1sq)
			for (j=0,tp1=picp;j<M-1;j++,tp1+=ncm1)
				for (k=0,tp2=tp1;k<M-1;k++)
					*tp2++ = c1 ? hips_hchar : hips_lchar;
	}
	else if (c1>=0) {
		c2=seebit();
		if (c2) {
			getbit();
			picp=tp1;
			for (i=0;i<dimd2*M-1;i++,picp+=ncm1sq)
			for (j=0,tp1=picp;j<dimv2*M-1;j++,tp1+=ncm1)
			for (k=0,tp2=tp1;k<dimh2*M-1;k++)
				*tp2++ = c1 ? hips_hchar : hips_lchar;
		}
		else ddecode(level+1,dimv2,dimh2,dimd2,tp1);
	}
	else ddecode(level+1,dimv2,dimh2,dimd2,tp1);
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
