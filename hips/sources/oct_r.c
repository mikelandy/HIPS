/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * oct_r.c - decode an oct-tree into a byte formatted sequence.
 *
 * usage: oct_r [-d] < input_tree >output_sequence
 *
 * -d is used to create a displayable picture of the decomposition into
 * sub-blocks. The boundaries are painted in gray.
 *
 * The input must be in the special PFOCT format.  Coding assumptions:
 * Mapping: {W,B,G}-->{00,01,1}. At the lowest level the eight pixels are
 * coded as a "nibble" in 8 bits.  The code is assumed to be left shifted and
 * packed, with last word of each oct-tree padded with 0's to the right.
 * The Number of bits read is given on "stderr".
 *
 * to load: cc -o oct_r oct_r.c -lhips -lm
 *
 * Yoav Cohen - 4/7/83
 * modified by Mike Landy - 6/21/83
 * Hips 2 - msl - 7/23/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"d",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFOCT,LASTTYPE};

#define M 4

int nbits,inbyte,readbits,nr,nc,nrc,stacks,ncm1,getbit();
byte *pic;
FILE *fp;
void decode(),ddecode();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdo;
	Filename filename;
	h_boolean dflag;
	byte *tp;
	int i,j,r,c,f,it1,it2,one=1;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&dflag,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_cpf(fp,&hd,types,filename);
	getparam(&hd,"stacks",PFINT,&one,&stacks);
	nr=r=hd.orows; nc=c=hd.ocols; nrc=r*c; ncm1=nc*M-1; f=hd.num_frame;
	if (f%stacks != 0 || nr%stacks!=0 || nc%stacks!=0)
		perr(HE_MSG,"image dimensions inconsistent with stack size");
	dup_headern(&hd,&hdo);
	setformat(&hdo,PFBYTE);
	if (dflag) {
		setsize(&hdo,(r*M-1) * stacks*M,c*M-1);
			/* fake allocation of stacks */
		hd.num_frame = f*M;
		alloc_imagez(&hdo);
		setsize(&hdo,(r*M-1),c*M-1);
	}
	else {
		setsize(&hdo,r*stacks,c);
		alloc_image(&hdo);
		setsize(&hdo,r,c);
	}
	write_headeru(&hdo,argc,argv);
	if (dflag)
		setsize(&hdo,(r*M-1) * stacks*M,c*M-1);
	else
		setsize(&hdo,r*stacks,c);
	pic = hdo.image;
	nbits=0; readbits=0;

	for(j=0;j<f/stacks;j++) {
		if(dflag) { /* paint boundaries between trees */
			tp=pic;
			for (i=0;i<ncm1*(r*M-1)*stacks*M;i++)
				*tp++ = 128;
		}
		for(it1=0;it1<r/stacks;it1++)
		    for(it2=0;it2<c/stacks;it2++) {
			if (dflag)
				ddecode(stacks,
					pic+it1*(c*M-1)*M*stacks+it2*stacks*M);
			else
				decode(stacks,pic+it1*c*stacks+it2*stacks);
			nbits=0;
		    }
		write_image(&hdo,j);
	}
	fprintf(stderr,"%s: read %d bits\n",Progname,readbits);
	return(0);
}

/* decode a region */

void decode(dim,pic)

int dim;
byte *pic;

{
	int dim2,dim2nc,dim2nrc,k,i,j,bit;
	byte *picp,*tp;

	bit=getbit();
	if (dim==1) {
		*pic = bit ? hips_hchar : hips_lchar;
		return;
	}
	if (bit == 0) {
		bit=getbit();
		for(k=0;k<dim;k++,pic+=nrc)
		  for(i=0,picp=pic;i<dim;i++,picp+=nc)
		    for(j=0,tp=picp;j<dim;j++)
			*tp++  = bit ? hips_hchar : hips_lchar;
		return;
	}
	dim2=dim>>1; dim2nc=dim2*nc; dim2nrc=dim2*nrc;
	decode(dim2,pic);
	decode(dim2,pic+dim2);
	decode(dim2,pic+dim2nc);
	decode(dim2,pic+dim2nc+dim2);
	decode(dim2,pic+dim2nrc);
	decode(dim2,pic+dim2nrc+dim2);
	decode(dim2,pic+dim2nrc+dim2nc);
	decode(dim2,pic+dim2nrc+dim2nc+dim2);
}

void ddecode(dim,pic)	/* inefficient; should be optimized. */

int dim;
byte *pic;

{
	int dim2,dim2m,dimm1,k,i,j,bit;
	byte *tp,*picp;

	dimm1=dim*M-1;
	bit=getbit();
	if(dim==1 || bit == 0) {
		if(bit==0 && dim != 1)bit=getbit();
		for(k=0;k<dimm1;k++,pic+=((ncm1)*(nr*M-1)))
		for(i=0,picp=pic;i<dimm1;i++,picp+=(ncm1))
		for(j=0,tp=picp;j<dimm1;j++)
			*tp++ = (bit)? hips_hchar : hips_lchar;
		return;
	}
	dim2=dim>>1;
	dim2m=dim2*M;
	ddecode(dim2,pic);
	ddecode(dim2,pic+dim2m);
	ddecode(dim2,pic+dim2m*(ncm1));
	ddecode(dim2,pic+dim2m*(ncm1)+dim2m);
	pic+=(dim2m*(ncm1)*(nr*M-1));
	ddecode(dim2,pic);
	ddecode(dim2,pic+dim2m);
	ddecode(dim2,pic+=(dim2m*(ncm1)));
	ddecode(dim2,pic+dim2m);
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
