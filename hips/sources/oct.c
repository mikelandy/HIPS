/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * oct.c - Hierarchical oct-tree coding of binary image sequences
 *
 * usage:	oct [-s stacksize] [-g > out_sequence] < input_sequence 
 *
 * stacksize determines the size of the area which is oct-encoded as a tree in
 * the forest.  It must be a power of 2 and evenly divide the number of rows
 * and columns in the input images.  It can be no greater than 32.
 *
 * -g indicates that actual code should be generated.  The full code
 * is: {W,B,G}-->{00,01,1}.  The output is in a special format: PFOCT, each
 * frame is bit-packed, with the last word of the tree padded with zeroes to
 * the right.  The number of output bytes is reported on "stderr".
 * arguments and defaults -- -g to generate actual code.
 *
 * The input must be in byte-unpacked-format with 1 bit per pixel (in other
 * words, oct only codes nonzero vs zero pixels).  The coding
 * assumptions are 2 bits per black or white node, 1 bit per meta
 * symbol. Except at the lowest level where the eight pixels are coded as a
 * "nibble" in 8 bits.  If -g is specified, the actual code is generated.  It
 * is left shifted and packed, with the last word of each tree padded with 1's
 * to the right.  Compression statistics are given on "stderr".  The program
 * computes the number of input bits, output bits and compression ratio.
 *
 * to load:	cc -o oct oct.c -lhips
 *
 * Yoav Cohen - 4/6/83
 * modified by Mike Landy - 6/21/83
 * HIPS 2 - msl - 7/23/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"s",{LASTFLAG},1,{{PTINT,"-1","stacksize"},LASTPARAMETER}},
	{"g",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};

#define b 0
#define w 1
#define B 2
#define W 3
#define G 4

char buffer[512],**tree,*ttree;
int byteindex,bitindex,nc,nrc,encode(),outree();
h_boolean generate;
void outsym(),outbit(),flushbuf();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo;
	Filename filename;
	FILE *fp;
	byte *pic;
	int method,i,j,r,c,f,len,stacks,ofr,logs,it1,it2;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&stacks,&generate,FFONE,&filename);
	fp = hfopenr(filename);
	fread_header(fp,&hd,filename);
	r=hd.orows; nc=c=hd.ocols;
	nrc=r*c;
	f=hd.num_frame;
	len = 0;
	if (r<=0 || c<=0 || f<=0)
		perr(HE_MSG,"input image dimensions must be greater than zero");
	if (stacks < 0) {
		for (i=1;r%(1<<i)==0;i++);
		stacks = 1<<(i-1);
		for (i=1;c%(1<<i)==0;i++);
		if ((1<<(i-1)) < stacks)
			stacks = 1<<(i-1);
		for (i=1;f>=(1<<i);i++);
		if ((1<<(i-1)) < stacks)
			stacks = 1<<(i-1);
		if (stacks > 32)
			stacks = 32;
	}
	if (stacks < 1)
		perr(HE_MSG,"stacksize argument must be >= 1");
	for (logs=0;(1<<logs)<stacks;logs++);
	fprintf(stderr,"%s: stacksize is %d x %d x %d\n",Progname,stacks,
		stacks,stacks);
	if ((1<<logs)!=stacks)
		perr(HE_MSG,"stack dimension must be a power of 2");
	if (r%stacks!=0 || c%stacks!=0)
		perr(HE_MSG,
		    "stack size must divide the number of rows and columns");
	if (f < stacks)
		perr(HE_MSG,"stacks arg greater than the number of frames");
	if (stacks<2)
		perr(HE_MSG,"stacks arg must be > 1");
	ofr=f-(f % stacks);
	if (ofr!=f)
		fprintf(stderr,"%s: only %d frames are encoded\n",Progname,ofr);

	/* alloc several frames (stacks, to be precise) */

	setsize(&hd,r*stacks,c);
	alloc_image(&hd);
	method = fset_conversion(&hd,&hdp,types,filename);
	if (hdp.numcolor > 1)
		perr(HE_MSG,"can't handle color images");
	pic = hdp.image;
	ttree = (char *)
		halloc((unsigned)(stacks*stacks*stacks*8/7+1),sizeof(char));
	tree = (char **) halloc(logs+1,sizeof(char *));

	/* internal allocation */

	tree[0]=ttree; j=stacks*stacks*stacks;
	for(i=1;i<=logs;i++,j>>=3)
		tree[i]=tree[i-1]+j;

	if (generate) {
		setsize(&hdp,r,c);
		dup_headern(&hdp,&hdo);
		hdo.pixel_format=PFOCT;
		hdo.num_frame=ofr;
		setparam(&hdo,"stacks",PFINT,1,stacks);
		write_headeru(&hdo,argc,argv);
		setsize(&hdp,r*stacks,c);
	}

	for(j=0;j<ofr/stacks;j++) {
		fread_imagec(fp,&hd,&hdp,method,j,filename);
		for (it1=0;it1<r/stacks;it1++)
		    for (it2=0;it2<c/stacks;it2++) {
			encode(logs,pic+it1*c*stacks+it2*stacks,0);
			byteindex=bitindex=0; buffer[0]=0;
			len += outree(logs,0);
			if (generate)
				flushbuf();
		    }
	}

	fprintf(stderr,"%s: in: %d bits, out: %d bits, compression: %5.3f\n",
		Progname,r*c*ofr,len,(double)len/(r*c*ofr));
	return(0);
}

/* encode a region */

int encode(level,pic,node)

int level,node;
byte *pic;

{
	int i,c[8],color,level1,node8,width;
	
	if (level==0)
		return(tree[level][node]=(*pic)?w:b);
	level1=level-1; node8=node<<3;
	width=1<<level1;
	c[0]=encode(level1,pic,node8++);
	c[1]=encode(level1,pic+width,node8++);
	c[2]=encode(level1,pic+width*nc,node8++);
	c[3]=encode(level1,pic+width*(nc+1),node8++);
	c[4]=encode(level1,pic+=(width*nrc),node8++);
	c[5]=encode(level1,pic+width,node8++);
	c[6]=encode(level1,pic+=(width*nc),node8++);
	c[7]=encode(level1,pic+width,node8);

	color=c[0];
	if(color==G)
		return(tree[level][node]=G);
	for(i=1;i<8;i++)
		if(c[i]==G || c[i]!=color)
			return(tree[level][node]=G);
	if (color==b)
		color=B;
	else if (color==w)
		color=W;
	return(tree[level][node]=color);
}

int outree(level,node)

int level,node;

{
	int sym;
	int back,i,subnode;

	sym=tree[level][node];
	if(sym!=G) {
		switch (sym) {
		case B:		back=2;break;
		case W:		back=2;break;
		case b:		back=1;break;
		case w:		back=1;break;
		default:	perr(HE_MSG,"error 1 in outree");
		}
		if (generate)
			outsym(sym);
		return(back);
	}
	else {
		if (generate)
			outsym(sym);
		back=1; subnode=node*8;
		for(i=0;i<8;i++,subnode++)
			back+=outree(level-1,subnode);
		return(back);
	}
}

void outsym(sym)

int sym;

{
	switch(sym) {
	case B:		outbit(0);outbit(0);break;
	case W:		outbit(0);outbit(1);break;
	case b:		outbit(0);break;
	case w:		outbit(1);break;
	case G:		outbit(1);break;
	default:	perr(HE_MSG,"error 1 in outsym");
	}
}

void outbit(bit)

int bit;

{
	buffer[byteindex]=(buffer[byteindex]+(bit&1));
	bitindex++;
	if (bitindex>=8) {
		bitindex=0; byteindex++;
		if (byteindex>=512) {
			fwrite(buffer,512,1,stdout); byteindex=0;
		}
		buffer[byteindex]=0;
	}
	else
		buffer[byteindex]<<=1;
}

void flushbuf()

{
	if (bitindex>0) {
		while(bitindex<7)outbit(0);
		fwrite(buffer,byteindex+1,1,stdout);
	}
	else
		fwrite(buffer,byteindex,1,stdout);
}
