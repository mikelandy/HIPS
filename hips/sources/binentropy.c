/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * binentropy - compute the entropy of pixel blocks over a binary image
 *
 * usage:	binentropy [-b depth [height [width]]] [-h] [-v] <iseq
 *
 * Binentropy computes the entropy of a binary sequence considering 3D
 * subblocks of the sequence as the `alphabet' of the entropy computation.
 * Depth, height and width all default to 2.  If -h is specified, the Huffman
 * code for the blocks is generated and statistics for it are given.  Two
 * statistics are given: for code which is based on all combinations, and for
 * code which is based on all non-uniform blocks.  If the sub-block dimensions
 * do not evenly divide the sequence dimensions, excess pixels are ignored in
 * the computation.  The -v switch is used to display the state of the program
 * as it progresses; it reports the frame number that is processed
 * and the "pass" number when computing the Huffman code.
 *
 * pixel formats handled directly: BYTE
 *
 * to load:	cc -o binentropy binentropy.c -lhipsh -lhips
 *
 * Yoav Cohen - 9/9/82
 * HIPS 2 - Michael Landy - 7/5/91
 *
 * bugs: computation of the statistics of the Huffman code is very inefficient.
 * It should be used only for small blocks. For large blocks use the -v
 * switch to get some idea about the time it takes, or change the code...
 */

#include <stdio.h>
#include <hipl_format.h>
#include <math.h>

static Flag_Format flagfmt[] = {
	{"b",{LASTFLAG},1,{{PTINT,"2","depth"},{PTINT,"2","height"},
		{PTINT,"2","width"},LASTPARAMETER}},
	{"h",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"v",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};
h_boolean vflag;
double h_pathlen();
int put_entry(),zero_min();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp;
	int method,depth,height,width,nr,nc,nf,ntable,*table,*vec;
	int count,ibr,ibc,i,k,l,m,word,nex,nexb,nexd,nbc;
	int f;
	byte *p,*p2;
	Filename filename;
	FILE *fp;
	h_boolean hflag;
	double entropy,prob,dn,h_pathlen(),pl;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&depth,&height,&width,&hflag,&vflag,
		FFONE,&filename);
	if (depth<1 || height<1 || width<1)
		perr(HE_MSG,"unreasonable depth, height or width");
	if(depth*height*width >= 32)
		perr(HE_MSG,"sub-blocks are too big");
	fp = hfopenr(filename);
	fread_header(fp,&hd,filename);
	nr = hd.rows;
	nc = hd.cols;
	nf = hd.num_frame;
	hd.rows *= depth;	/* trick to read `depth' frames and still
					have firstpix be correct */
	hd.num_frame /= depth;
	alloc_image(&hd);
	ntable = 1<<(depth*height*width);
	table = (int *) halloc(ntable,sizeof(int));
	if (hflag)
		vec = (int *) memalloc(ntable,sizeof(int));
	method = fset_conversion(&hd,&hdp,types,filename);
	if (hdp.numcolor > 1)
		perr(HE_MSG,"can't handle color images");
	fprintf(stderr,"%s: depth,height,width: %d %d %d\n",Progname,
		depth,height,width);
	count=0;
	nbc = hdp.cols/width;
	nexb = hdp.ocols - nbc*width;
	nex = hdp.ocols - width;
	nexd = (hdp.orows - height) * hdp.ocols;
	for(f=0;f<nf;f+=depth) {
		if (vflag)
			fprintf(stderr,"%s: reading frame block %d\n",
				Progname,f);
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		p = hdp.firstpix;
		for(ibr=0;ibr<nr;ibr+=height) {
		    for(ibc=0;ibc<nc;ibc+=width) {
			p2 = p;
			word=0;
			count++;
			for(k=0;k<depth;k++) {
			  for(l=0;l<height;l++) {
			    for(m=0;m<width;m++) {
				word <<= 1;
				if (*p2++)
					word++;
			    }
			    p2 += nex;
			  }
			  p2 += nexd;
			}
			table[word]++;
			p += width;
		    }
		    p += nexb;
		}
	}
	dn=count;
	for(i=0;i<ntable;i++) {
		prob = table[i]/dn;
		if (prob)
			entropy += prob*log(prob);
	}
	entropy /= (-log((double) 2.));
	fprintf(stderr,"%s: in %d blocks, entropy= %f\n",Progname,count,
		(float) entropy);
	fprintf(stderr,"%s: entropy per pixel = %f\n",Progname,
		(float) (entropy/(depth*height*width)));
	if (!hflag)
		return(0);
	for (i=0;i<ntable;i++)
		vec[i]=table[i];
	pl = h_pathlen(vec,ntable);
	fprintf(stderr,"%s: average path-length= %f, per pixel= %f\n",
			Progname,(float) pl,(float) (pl/(depth*width*height)));
	pl = h_pathlen(table+1,ntable-2);
	fprintf(stderr,"%s:    for non-uniform blocks= %f, per pixel= %f\n",
			Progname,(float) pl,(float) (pl/(depth*width*height)));
	return(0);
}

/*
 * h_pathlen - find average length of Huffman code word
 */

double h_pathlen(vec,nvals)

int *vec,nvals;

{
	double sumlength;
	int i;
	int min1,min2;

	sumlength=0.0;
	for(i=0;i<nvals-1;i++) {
		if (vflag && (i+1)%10 == 0)
			fprintf(stderr,
				"%s: Huffman, iteration %d out of %d\n",
				Progname,i+1,nvals);
		min1 = zero_min(vec,nvals);
		min2 = zero_min(vec,nvals);
		put_entry(vec,nvals,min1+min2);
		sumlength += min1+min2;
	}
	return(sumlength/(min1+min2));
}

int zero_min(vec,nvals)

int *vec,nvals;

{
	int i,imin;
	int v,min;

	imin = -1;
	for (i=0;i<nvals;i++) {
		v=vec[i];
		if (v == -1)
			continue;
		if (v<min || imin<0) {
			imin=i;
			min=v;
		}
	}
	if (imin<0 || imin>=nvals)
		perr(HE_MSG,"error in zero_min");
	vec[imin] = -1;
	return(min);
}

int put_entry(vec,length,ent)

int *vec,length,ent;

{
	int i;

	for(i=0;i<length;i++)
		if(vec[i] == -1) {
			vec[i] = ent;
			return(0);
		}
	perr(HE_MSG,"error in put_entry");
	return(0);
}
