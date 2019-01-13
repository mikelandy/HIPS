/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * binquad_r.c - decode a binquad-coded sequence into a byte formatted sequence
 *
 * usage: binquad_r [-d [delta [greylevel]]]  < input_tree >output_sequence
 *
 * -d is used to create a displayable picture of the decomposition into
 * sub-blocks. The boundaries are painted in gray.  If delta is
 * specified, then each lower level boundary is delta grey-levels lower
 * than the previous level's boundary.  Delta defaults to zero.
 * Greylevel sprecifies the grey level of level 0, which defaults to 168.
 *
 * The input must be in the special PFBQ format.  Coding assumptions: 
 * Mapping: {W,B,S,T}-->{10,11,00,01}.  At the lowest level the pixels are
 * coded in 1 bit.  When a subdivision is possible in only one direction,
 * less bits are used. (see binquad.c).  The code is assumed to be left
 * shifted and packed, with last word of each ahc-tree padded with 0's to the
 * right.  The number of bits read is given on "stderr".
 *
 * to load: cc -o binquad_r binquad_r.c -lhips -lm
 *
 * Yoav Cohen - 4/25/83
 * modified by Mike Landy - 6/20/83
 * Hips 2 - msl - 7/22/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"d",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},{PTINT,"0","delta"},
		{PTINT,"168","greylevel"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBQ,LASTTYPE};

#define H 1
#define V 0
#define M 4

int nbits,inbyte,readbits,nc,nrc,stackspa,stackdepth,ncm1,ncm1sq,M1;
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
	int i,r,c,f,it1,it2,one=1;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&dflag,&delta,&greyl,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_cpf(fp,&hd,types,filename);
	getparam(&hd,"stackspa",PFINT,&one,&stackspa);
	getparam(&hd,"stackdepth",PFINT,&one,&stackdepth);
	r=hd.orows; nc=c=hd.ocols; nrc=r*c; f=hd.num_frame;
	if (f%stackdepth != 0 || r%stackspa!=0 || c%stackspa!=0)
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
	    for (it1=0;it1<r/stackspa;it1++)
		for (it2=0;it2<c/stackspa;it2++) {
			nbits=0;
			if (dflag)
				ddecode(0,stackspa,stackdepth,
				    pic+it1*stackspa*M*(c*M-1)+it2*stackspa*M);
			 else
				decode(stackspa,stackdepth,
				    pic+it1*c*stackspa+it2*stackspa);
		}
		write_image(&hdo,i);
	}
	fprintf(stderr,"%s: read %d bits\n",Progname,readbits);
	return(0);
}

/* decode a region */

int decode(dimh,dimd,picp)

int dimd,dimh;
byte *picp;

{
	int dimh2,dimd2,c1,c2,c3,c4; 
	int i,j,k,bit,dirs;
	byte *tp1,*tp2,*tp3;
	int spatial;

	bit=getbit();
	if (dimh==1 && dimd==1) {
		*picp = bit ? hips_lchar : hips_hchar;
		return(bit);
	}
	if (bit == 1) {
		bit=getbit();
		for (i=0;i<dimd;i++,picp+=nrc)
		for (j=0,tp1=picp;j<dimh;j++,tp1+=nc)
		for (k=0,tp2=tp1;k<dimh;k++)
			*tp2++  = bit ? hips_lchar : hips_hchar;
		return(bit);
	}

	/* code for non-uniform area */
	dimh2=dimh;dimd2=dimd;
	dirs=0;if(dimh>1)dirs++;if(dimd>1)dirs++;
	/* all directions are possible: */
	if(dirs==2) {
		bit=getbit();
		if(bit==0) {/* spatial cut */
			dimh2>>=1; tp1=picp+dimh2;
			spatial=1; tp2=picp+dimh2*nc;
			tp3=tp2+dimh2;
		}
		else	{ 
			/* temporal cut */
			dimd2>>=1; tp1=picp+dimd2*nrc;
			spatial=0;
		}

	}	
	/* One direction is possible */
	else if(dirs==1) {
		if(dimd>1) {/* temporal cut */
			dimd2>>=1; tp1=picp+dimd2*nrc;
			spatial=0;
		}
		else if(dimh>1) {/* spatial cut */
			dimh2>>=1; tp1=picp+dimh2;
			spatial=1; tp2=picp+dimh2*nc;
			tp3=tp2+dimh2;
		}
		else perr(HE_MSG,"error 2 in encode()");
	}
	else perr(HE_MSG,"error 3 in decode()");

	c1=decode(dimh2,dimd2,picp);
	if(spatial) {	c2=decode(dimh2,dimd2,tp1);
			c3=decode(dimh2,dimd2,tp2);
	}
	else	{ tp3=tp1; }

	if((dimh2==1 && dimd2==1) && 
		(!spatial||(spatial&&c1>=0&&c1==c2&&c2==c3)))
				 *tp3=c1 ? hips_hchar : hips_lchar;
	else if((!spatial&&c1>=0)||(spatial&&c1>=0&&c1==c2&&c2==c3)) {
		c4=seebit();
		if (c4) {
			getbit();
			picp=tp3;
			for (i=0;i<dimd2;i++,picp+=nrc)
			for (j=0,tp1=picp;j<dimh2;j++,tp1+=nc)
			for (k=0,tp2=tp1;k<dimh2;k++)
				*tp2++ = c1 ? hips_lchar : hips_hchar;
		}
		else decode(dimh2,dimd2,tp3);
	}
	else decode(dimh2,dimd2,tp3);
	return(-1);
}

int ddecode(level,dimh,dimd,picp)

int level,dimh,dimd;
byte *picp;

{
	int dimh2,dimd2,c1,c2,c3,c4; 
	int i,j,k,bit,dirs;
	byte *tp1,*tp2,*tp3;
	int spatial,color,gcolor;

	gcolor = greyl-level*delta;
	bit=getbit();
	if(dimh==1 && dimd==1) {
		color=(bit)? hips_lchar : hips_hchar;
		for(i=0;i<M1;i++,picp+=ncm1sq)
		for(j=0,tp1=picp;j<M1;j++,tp1+=ncm1)
		for(k=0,tp2=tp1;k<M1;k++)
			*tp2++ = color;
		return(bit);
	}
	if(bit == 1) {
		bit=getbit();
		color=(bit)?hips_lchar:hips_hchar;
		for(i=0;i<dimd*M-1;i++,picp+=ncm1sq)
		for(j=0,tp1=picp;j<dimh*M-1;j++,tp1+=ncm1)
		for(k=0,tp2=tp1;k<dimh*M-1;k++)
			*tp2++  = color;
		return(bit);
	}

	/* code for non-uniform area */
	dimh2=dimh;dimd2=dimd;
	dirs=0;if(dimh>1)dirs++;if(dimd>1)dirs++;
	/* all directions are possible: */
	if(dirs==2) {
		bit=getbit();
		if(bit==0) {/* spatial cut */
			goto scut;
		}
		else	{ 
			/* temporal cut */
			goto tcut;
		}
			
	}	
	/* One direction is possible */
	else if(dirs==1) {
		if(dimd>1) {/* temporal cut */
			goto tcut;
		}
		else if(dimh>1) {/* spatial cut */
			goto scut;
		}
		else perr(HE_MSG,"error 2 in encode()");
	}
	else perr(HE_MSG,"error 3 in decode()");
tcut:
	dimd2>>=1;
	for (i=0,tp1=picp+(dimd2*M-1)*ncm1sq;i<dimh*M-1;i++,tp1+=ncm1)
		for (j=0,tp2=tp1;j<dimh*M-1;j++)
			*tp2++ = gcolor;
	tp1=picp+dimd2*ncm1sq*M;
	spatial=0;
	goto donecut;
scut:
	dimh2>>=1;
	for (i=0,tp1=picp+(dimh2*M-1);i<dimd*M-1;i++,tp1+=ncm1sq)
		for (j=0,tp2=tp1;j<dimh*M-1;j++,tp2+=ncm1)
			*tp2 = gcolor;
	for (i=0,tp1=picp+(dimh2*M-1)*ncm1;i<dimd*M-1;i++,tp1+=ncm1sq)
		for (j=0,tp2=tp1;j<dimh*M-1;j++)
			*tp2++ = gcolor;
	tp1=picp+dimh2*M;
	spatial=1; tp2=picp+dimh2*M*ncm1;
	tp3=tp2+dimh2*M;

donecut:
	c1=ddecode(level+1,dimh2,dimd2,picp);
	if (spatial) {
		c2=ddecode(level+1,dimh2,dimd2,tp1);
		c3=ddecode(level+1,dimh2,dimd2,tp2);
	}
	else	{ tp3=tp1; }

	if((dimh2==1 && dimd2==1) && 
		(!spatial||(spatial&&c1>=0&&c1==c2&&c2==c3))) {
			color=(c1)?hips_hchar:hips_lchar;
			picp = tp3;
			for(i=0;i<M1;i++,picp+=ncm1sq)
			for(j=0,tp1=picp;j<M1;j++,tp1+=ncm1)
			for(k=0,tp2=tp1;k<M1;k++)
				*tp2++  = color;
	}
	else if((!spatial&&c1>=0)||(spatial&&c1>=0&&c1==c2&&c2==c3)) {
		c4=seebit();
		if (c4) {
			getbit();
			color=(c1)?hips_hchar:hips_lchar;
			picp=tp3;
			for(i=0;i<dimd2*M-1;i++,picp+=ncm1sq)
			for(j=0,tp1=picp;j<dimh2*M-1;j++,tp1+=ncm1)
			for(k=0,tp2=tp1;k<dimh2*M-1;k++)
				*tp2++ = color;
		}
		else ddecode(level+1,dimh2,dimd2,tp3);
	}
	else ddecode(level+1,dimh2,dimd2,tp3);
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
