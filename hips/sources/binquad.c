/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * binquad.c - 3D binary (temporal) and quadtree (spatial) adaptive hierarchical
 *		coding of binary images
 *
 * usage: binquad [-t stackspatial] [-f stackdepth]
 *			[-d] [-s] [-v] [-g or -a > outseq] < inseq
 *
 * stackspatial and stackdepth determine the size of the area
 * which is binquad-encoded as a tree in the forest.  Both must be powers of
 * 2, stackspatial must be a divisor of both spatial dimensions.  Stackdepth
 * defaults to the largest power of 2 divisor of the number of frames, and
 * stackspatial defaults to the largest power of 2 which is a divisor of both
 * the number of rows and the number of columns.
 *
 * -g indicates that actual code should be generated.  The full code is:
 * {W,B,S,T}-->{10,11,00,01}.  (If only one cut is possible it is encoded by 0.)
 * The output is in a special format: PFBQ, each tree is bit-packed, with the
 * last word of the tree padded with zeroes to the right.  The number of
 * output bytes is reported on "stderr".
 * -a specifies that actual code be generated in ASCII.  Only one of -g and -a
 * may be specified.  if -a is specified the output is not in HIPS format
 * (there is no header).  Specification of -s produces statistics for each
 * frame in addition to totals.  -v (verbose) is useful only for debugging
 * purposes, and prints messages about the program's progress.  -d (debug) is
 * another debugging flag, which outputs the entire stored tree structure.
 *
 * The input must be in byte-unpacked-format with 1 bit per pixel (in other
 * words, binquad only codes nonzero vs zero pixels).  The coding
 * assumptions are 2 bits per black or white node, 2 or 1 bits per meta
 * symbol. Except at the lowest level where the two pixels are coded as a
 * "nibble" in 1, 3 or 4 bits.  Also if an area is divided into two homogenous
 * areas, the second area is coded in 1 bit (0) , since it must be of
 * color different from that of the first area.  (If -g is specified, the actual
 * code is generated.  It is left shifted and packed, with the last word of
 * each tree padded with 1's to the right.)  Compression statistics are
 * given on "stderr".  The program computes the number of input bits,
 * output bits and compression ratio.
 *
 * to load:	cc -o binquad binquad.c -lhips
 *
 * Yoav Cohen - 4/24/83
 * modified for non-cubic pictures/two stack dimensions - Mike Landy - 6/20/83
 * HIPS 2 - msl - 7/22/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"t",{LASTFLAG},1,{{PTINT,"-1","stackspatial"},LASTPARAMETER}},
	{"f",{LASTFLAG},1,{{PTINT,"-1","stackdepth"},LASTPARAMETER}},
	{"s",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"d",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"v",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"g",{"a",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"a",{"g",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};

#define BQNULL 0
#define B 1
#define W 2
#define S 3
#define T 4

byte *inpic;
struct node 	{
		int len;
		char sym;
		union { struct node *pt;
			char ch;
		      } lch;
		union { struct node *pt;
			char ch;
		      } rch;
		union { struct node *pt;
			char ch;
		      } lcht;
		union { struct node *pt;
			char ch;
		      } rcht;
		};
struct node *****p;
int nns,nnd;

char *obuf;
int bufindex,bitindex,bytesout=0;
h_boolean gsw,asw,ssw,vsw,dsw;
int nr,nc,nrc,stackspa,stackdepth,enc2(),encode(),outbit(),flushbuf();
int acode(),asons(),aprint(),lenprint();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo;
	Filename filename;
	FILE *fp;
	byte *in;
	int method;
	struct node ****p1,***p2,**p3,*p4,****tp1,***tp2,**tp3,*tp4,*pt;
	int f,ifr,i,j,k,l,m,sumlen,len,v,d,s,ofr,it1,it2,r,low;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&stackspa,&stackdepth,&ssw,
		&dsw,&vsw,&gsw,&asw,FFONE,&filename);
	fp = hfopenr(filename);
	fread_header(fp,&hd,filename);
	nr = hd.orows;
	nc = hd.ocols;
	f = hd.num_frame;
	if (nr<=0 || nc<=0 || f<=0)
		perr(HE_MSG,"input image dimensions must be greater than zero");
	if (stackspa < 0) {
		for (i=1;nr%(1<<i)==0;i++);
		stackspa = 1 << (i-1);
		for (i=1;nc%(1<<i)==0;i++);
		if (1<<(i-1) < stackspa)
			stackspa = 1 << (i-1);
	}
	if (stackdepth < 0) {
		for (i=1;f%(1<<i)==0;i++);
		stackdepth = 1 << (i-1);
	}
	if (stackspa<1 || stackdepth<1)
		perr(HE_MSG,"stack args must be >= 1");
	for (nns=0;(1<<nns)<stackspa;nns++);
	for (nnd=0;(1<<nnd)<stackdepth;nnd++);
	fprintf(stderr,"%s: stacksize is %d x %d x %d\n",Progname,stackspa,
		stackspa,stackdepth);
	if ((1<<nns)!=stackspa || (1<<nnd)!=stackdepth)
		perr(HE_MSG,"stack dimensions must be powers of 2");
	if ((nr % stackspa) != 0 || (nc % stackspa) != 0)
		perr(HE_MSG,
		    "spatial stack dimensions must divide image dimensions");
	if (f < stackdepth)
		perr(HE_MSG,"stack depth more than the number of image frames");
	if (nns==0 && nnd==0)
		perr(HE_MSG,"stack size must be bigger than 1 x 1 x 1");
	else if (nns==1 && nnd==0)
		perr(HE_MSG,"stack size must be bigger than 2 x 2 x 1");
	else if (nns==0 && nnd==1)
		perr(HE_MSG,"stack size must be bigger than 1 x 1 x 2");
	nrc = nr*nc;
	ofr = f - (f%stackdepth);
	if (ofr!=f)
		fprintf(stderr,"%s: only %d frames are encoded\n",Progname,ofr);

	/* alloc several frames (stackdepth, to be precise) */

	setsize(&hd,nr*stackdepth,nc);
	alloc_image(&hd);
	method = fset_conversion(&hd,&hdp,types,filename);
	if (hdp.numcolor > 1 && stackdepth > 1)
		perr(HE_MSG,"can't handle color images with stackdepth > 1");
	if (hdp.numcolor > 1)
		ofr = f = hdp.num_frame;
	inpic = hdp.image;
	if (gsw)
		obuf=(char *)halloc(512,sizeof(char));
	p = (struct node *****) halloc(nns+1,sizeof(struct node ****));
	p1 = (struct node ****)
		halloc((nns+1)*(nnd+1),sizeof(struct node ***));
	p2 = (struct node ***)
		halloc(((nnd+1)*((1<<(nns+1))-1)) - (1<<nns),
		sizeof(struct node **));
	p3 = (struct node **)
		halloc(((nnd+1)*(((1<<(nns+nns+2))-1)/3)) - (1<<(nns+nns)),
		sizeof(struct node *));
	p4 = (struct node *)
		halloc((((1<<(nnd+1))-1)*(((1<<(nns+nns+2))-1)/3)) -
			(1<<(nns+nns+nnd)),sizeof(struct node));

	/* internal allocation */

	tp1=p1; tp2=p2; tp3=p3; tp4=p4;
	for (i=0;i<=nns;i++) {
		p[i]=tp1; tp1 += (nnd+1);
		r=1<<(nns-i);
		for (j=0;j<=nnd;j++) {
			if (i==0 && j==0)continue;
			p[i][j]=tp2; tp2 += r;
			s=1<<(nnd-j);
			for (k=0;k<r;k++) {
				p[i][j][k]=tp3; tp3 += r;
				for (l=0;l<r;l++) {
					p[i][j][k][l]=tp4; tp4 += s;
				}
			}
		}
	}

	if (vsw)
		fprintf(stderr,"%s: done allocation\n",Progname);
	if (gsw) {
		setsize(&hdp,nr,nc);
		dup_headern(&hdp,&hdo);
		hdo.pixel_format=PFBQ;
		hdo.num_frame=ofr;
		setparam(&hdo,"stackspa",PFINT,1,stackspa);
		setparam(&hdo,"stackdepth",PFINT,1,stackdepth);
		write_headeru(&hdo,argc,argv);
		setsize(&hdp,nr*stackdepth,nc);
	}
	sumlen=0;

	for (ifr=0;ifr<ofr/stackdepth;ifr++) {
		fread_imagec(fp,&hd,&hdp,method,ifr,filename);
		in = inpic;
		for (i=0;i<nrc*stackdepth;i++,in++)
			*in = (*in==0) ? B : W;
		if (vsw)
			fprintf(stderr,"%s: done initialization\n",Progname);

		for (it1=0;it1<nr/stackspa;it1++)
		    for (it2=0;it2<nc/stackspa;it2++) {
			if (vsw)
				fprintf(stderr,"%s: it1=%d, it2=%d\n",
					Progname,it1,it2);
			enc2(inpic+it1*nc*stackspa+it2*stackspa);
			if (vsw)
				fprintf(stderr,"%s: done enc2()\n",Progname);
			for (m=2;m<=nns+nnd;m++)
			    for (v=0;v<=nns && v<=m;v++) {
				d=m-v;
				if (d<0||d>nnd)continue;
			if (vsw)
				fprintf(stderr,"%s: calling encode(%d,%d)  ",
					Progname,v,d);
				encode(v,d);
			if (vsw)
				fprintf(stderr,"...done\n");
			    }
		len=p[nns][nnd][0][0][0].len;
		sumlen+=len;
		if (ssw&&f>1)
			fprintf(stderr,"%s: frame #%d, CR=%5.3f\n",Progname,
				ifr,(double)len/(stackspa*stackspa*stackdepth));
		if (dsw) {
			for (i=0;i<=nns;i++) {
				r=1<<(nns-i);
				for (j=0;j<=nnd;j++) {
					if (i==0 && j==0) continue;
					if (i+j == 1)
						low=1;
					else
						low=0;
					s=1<<(nnd-j);
					for (k=0;k<r;k++)
					for (l=0;l<r;l++)
					for (m=0;m<s;m++) {
						pt= &p[i][j][k][l][m];
						printf("%p ",(void *) pt);
						printf("p[%d][%d][%d][%d][%d]  ",i,j,k,l,m);
						printf("sym=%d, len=%d, ",pt->sym,pt->len);
						if (!low)
							printf("pointers: %p %p %p %p",pt->lch.pt,pt->rch.pt,pt->lcht.pt,pt->rcht.pt);
						printf("\n");
					}
				}
			}
		}
		if (asw||gsw)
			acode(ifr);
		}
	}

	i=ofr*nrc;
	j=sumlen;
	fprintf(stderr,"%s: Total Compression Ratio = %5.4f\n",Progname,
		(double) sumlen/(ofr*nrc));
	if (gsw)
		fprintf(stderr,"     %d bytes written out\n",bytesout);
	return(0);
}

/**************************************************************************/

int enc2(pic)

byte *pic;

{
	struct node *pt;
	byte *in1,*in2,*in11,*in22,*in111,*in222,p1,p2,p3,p4;
	int ir,ic,il;

/* To encode the m=1 diagonal */
	/* spatial quads */
	in1=pic; in2=pic+nc;
	for (il=0;il<stackdepth;il++,in1+=nrc,in2+=nrc)
	for (ir=0,in11=in1,in22=in2;ir<stackspa/2;
		ir++,in11+=(nc+nc),in22+=(nc+nc))
	for (ic=0,in111=in11,in222=in22;ic<stackspa/2;ic++) {
		p1= *in111++; p3= *in222++;
		p2= *in111++; p4= *in222++;
		pt= &p[1][0][ir][ic][il];
		pt->len = 2;
		if (p1==p2 && p2==p3 && p3==p4)
			pt->sym = p1;
		else {
			pt->sym = S;
			if (p1==p2 && p2==p3) pt->len = 4; else pt->len = 5;
			pt->lch.ch = p1; pt->rch.ch = p2;
			pt->lcht.ch = p3; pt->rcht.ch = p4;
		}
	}
	/* temporal pairs */
	in1=pic; in2=pic+nrc;
	for (il=0;il<stackdepth/2;il++,in1+=(nrc+nrc),in2+=(nrc+nrc))
	for (ir=0,in11=in1,in22=in2;ir<stackspa;ir++,in11+=nc,in22+=nc)
	for (ic=0,in111=in11,in222=in22;ic<stackspa;ic++) {
		p1= *in111++; p2= *in222++; 
		pt= &p[0][1][ir][ic][il];
		pt->len = 2;
		if (p1==p2)
			pt->sym = p1;
		else {
			pt->sym = T;
			pt->lch.ch = p1; pt->rch.ch = p2;
		}
	}
	return(0);
}

int encode(v,d)

int v,d;

{
	struct node *pt,*hch1,*hch2,*hch3,*hch4,*dch1,*dch2;
	char hsym,dsym,hsym1,hsym2,hsym3,hsym4,dsym1,dsym2;
	int ir,ic,il,hlen,dlen,nbits,max;

	nbits=0; if (v>0) nbits++; if (d>0) nbits++;
	if (nbits<=0||nbits>2)perr(HE_MSG,"error1 in encode()");
	for (il=0;il<(1<<(nnd-d));il++)
	for (ir=0;ir<(1<<(nns-v));ir++)
	for (ic=0;ic<(1<<(nns-v));ic++) {
		pt= &p[v][d][ir][ic][il];
		/* is a S-cut possible ? */
		if (v==0)hlen=0; /* no */
		else {		 /* yes */
			hch1 = &p[v-1][d][ir+ir][ic+ic][il];
			hch2 = &p[v-1][d][ir+ir][ic+ic+1][il];
			hch3 = &p[v-1][d][ir+ir+1][ic+ic][il];
			hch4 = &p[v-1][d][ir+ir+1][ic+ic+1][il];
			hsym1 = hch1->sym;
			hsym2 = hch2->sym;
			hsym3 = hch3->sym;
			hsym4 = hch4->sym;
			/* is it uniform ? */
			if ( (hsym1==B && hsym2==B && hsym3==B && hsym4==B)
			  ||(hsym1==W && hsym2==W && hsym3==W && hsym4==W) ) {
						/* yes */
				hsym=hsym1;
				hlen=2;
			}
			else	{ /* non-uniform */
			
				hlen = hch1->len + hch2->len +hch3->len + hch4->len + nbits;
				hsym = S;
				if ((hsym1==B||hsym1==W) && (hsym2==hsym1) &&
				    (hsym3==hsym2) && (hsym4==B || hsym4==W))
					--hlen;
			}
		}
		/* is a T-cut possible ? */
		if (d==0)
			dlen=0; /* no */
		else {		 /* yes */
			dch1 = &p[v][d-1][ir][ic][il+il];
			dch2 = &p[v][d-1][ir][ic][il+il+1];
			dsym1 = dch1->sym;
			dsym2 = dch2->sym;
			/* is it uniform ? */
			if ( (dsym1==B && dsym2==B)
			  ||(dsym1==W && dsym2==W) ) {  /* yes */
				dsym=dsym1;
				dlen=2;
			}
			else	{ /* non-uniform */
				dlen = dch1->len + dch2->len + nbits;
				dsym = T;
				if (  (dsym1==B||dsym1==W) && (dsym2==B||dsym2==W))
					--dlen;
			}
		}
		max=hlen; if (dlen>max)max=dlen; max++;
		if (hlen==0)hlen=max; if (dlen==0)dlen=max;
		if (hlen<= dlen) {
			pt->sym = hsym; pt->len = hlen;
			if (hsym==S) {
				pt->rch.pt = hch2; pt->lch.pt = hch1; 
				pt->lcht.pt= hch3; pt->rcht.pt= hch4;
			}
			else 	{
				pt->rch.pt = BQNULL; pt->lch.pt = BQNULL; 
				pt->lcht.pt= BQNULL; pt->rcht.pt= BQNULL;
			}
				  
		}
		else {
			pt->sym = dsym;
			pt->len = dlen;
			if (dsym==T) {
				pt->rch.pt = dch2; pt->lch.pt = dch1;
			}
			else	{
				pt->rch.pt = BQNULL; pt->lch.pt = BQNULL;
			}
			pt->lcht.pt= BQNULL; pt->rcht.pt= BQNULL;
		}
	}
	return(0);
}

int outbit(bit)

int bit;

{
/* on entry bufindex and bitindex point to the new available location */
	if (gsw) {
		if (bit<0 || bit>1)
			perr(HE_MSG,"error in outbit()");
		obuf[bufindex] += bit;
		bitindex++;
		if (bitindex<8)
			obuf[bufindex] = obuf[bufindex]<<1;
		else { 
			bufindex++; bitindex=0; 
			if (bufindex>511) {
				fwrite(obuf,512,1,stdout);
				bytesout+=512; bufindex=0;
			}
			obuf[bufindex]=0;
		}
	}
	return(0);
}

int flushbuf()

{
	if (bufindex==0 && bitindex==0)
		return(0);
	if (bitindex>0) {
		obuf[bufindex]=obuf[bufindex]<<(7-bitindex);
		bufindex++;
	}
	fwrite(obuf,bufindex,1,stdout);
	bytesout += (bufindex);
	return(0);
}

int acode(frame)

int frame;

{
	struct node *pt;
	char sym;

	if (gsw) {
		bufindex=bitindex=0;
		obuf[0]=0;
	}
	else
		printf("\n\n%s: Code for batch # %d\n\n",Progname,frame);
	pt = &p[nns][nnd][0][0][0];
	sym=pt->sym;
	switch(sym) {
		case	W:	aprint(0,'W','2');outbit(1);outbit(0);
				break;
		case	B:	aprint(0,'B','2');outbit(1);outbit(1);
				break;
		case	S:	if (nnd==0)
					{ aprint(0,'S','1');outbit(0); }
				else	{ aprint(0,'S','2');outbit(0);outbit(0); }
				break;
		case	T:	if (nns==0)
					{ aprint(0,'T','1');outbit(0); }
				else	{ aprint(0,'T','2');outbit(0);outbit(1); }
				break;
		default: 	perr(HE_MSG,"error in acode()");
	}
	lenprint(pt->len);
	if (sym==S||sym==T)
		 asons(pt,nns,nnd,0);
	if (gsw)
		flushbuf();
	return(0);
}

int asons(pt,h,d,level) 

int h,d,level;
struct node *pt;

{
	char sym,sym1,sym2,sym3,sym4;
	int level1,hh,dd;
	
	sym=pt->sym;
	if (sym==W||sym==B)
		perr(HE_MSG,"error 0 in asons()");
	level1=level+1;
/* sons are single pixels: */
	if (h+d==1) {
		if (h==1) {
			sym1=pt->lch.ch; sym2=pt->rch.ch; sym3=pt->lcht.ch; sym4=pt->rcht.ch;
			if (sym1==W)
				{aprint(level1,'W','1');outbit(0); }
			else	{aprint(level1,'B','1');outbit(1); }
			if (sym2==W)
				{aprint(level1,'W','1');outbit(0); }
			else	{aprint(level1,'B','1');outbit(1); }
			if (sym3==W)
				{aprint(level1,'W','1');outbit(0); }
			else	{aprint(level1,'B','1');outbit(1); }
			if (sym1==sym2 && sym2==sym3) return(0);
			else
			if (sym4==W)
				{aprint(level1,'W','1');outbit(0); }
			else	{aprint(level1,'B','1');outbit(1); }
			return(0);
		}
		else {
			sym1=pt->lch.ch;
			if (sym1==W)
				{aprint(level1,'W','1');outbit(0); }
			else	{aprint(level1,'B','1');outbit(1); }
			return(0);
		}
	}
/* other cases */
	
	hh=h;dd=d;
	if (sym==T)
		dd--;
	else if (sym==S)
		hh--;
	sym1=pt->lch.pt->sym; sym2=pt->rch.pt->sym;

	switch(sym1)	{
		case	W:	aprint(level1,'W','2');outbit(1);outbit(0);
				lenprint(pt->lch.pt->len);
				break;
		case	B:	aprint(level1,'B','2');outbit(1);outbit(1);
				lenprint(pt->lch.pt->len);
				break;
		case	S:	if (dd==0)
					{ aprint(level1,'S','1');outbit(0); }
				else	{ aprint(level1,'S','2');outbit(0);outbit(0); }
				lenprint(pt->lch.pt->len);
				asons(pt->lch.pt,hh,dd,level1);
				break;
		case	T:	if (hh==0)
					{ aprint(level1,'T','1');outbit(0); }
				else	{ aprint(level1,'T','2');outbit(0);outbit(1); }
				lenprint(pt->lch.pt->len);
				asons(pt->lch.pt,hh,dd,level1);
				break;
		default:	fprintf(stderr,"%s: funny character: %d-%c-\n",
					Progname,(unsigned int)sym1,sym1);
				perr(HE_MSG,"error 1 in asons");
	}
	switch(sym2)	{
		case	W:	if (sym==T && sym1==B)
					{aprint(level1,'U','1'); outbit(1);(pt->rch.pt->len)--;}
				else
					{aprint(level1,'W','2');outbit(1);outbit(0);}
				lenprint(pt->rch.pt->len);
				break;
		case	B:	if (sym==T && sym1==W)
					{aprint(level1,'U','1'); outbit(1);(pt->rch.pt->len)--;}
				else
					{aprint(level1,'B','2');outbit(1);outbit(1);}
				lenprint(pt->rch.pt->len);
				break;
		case	S:	if (dd==0)
					{ aprint(level1,'S','1');outbit(0); }
				else	{ aprint(level1,'S','2');outbit(0);outbit(0); }
				lenprint(pt->rch.pt->len);
				asons(pt->rch.pt,hh,dd,level1);
				break;
		case	T:	if (hh==0)
					{ aprint(level1,'T','1');outbit(0); }
				else	{ aprint(level1,'T','2');outbit(0);outbit(1); }
				lenprint(pt->rch.pt->len);
				asons(pt->rch.pt,hh,dd,level1);
				break;
		default:	fprintf(stderr,"%s: funny character: %d-%c-\n",
					Progname,(unsigned int)sym2,sym2);
				perr(HE_MSG,"error 2 in asons");
	}
	if (sym==T)
		return(0);
	sym3=pt->lcht.pt->sym; sym4=pt->rcht.pt->sym;
	switch(sym3)	{
		case	W:	aprint(level1,'W','2');outbit(1);outbit(0);
				lenprint(pt->lcht.pt->len);
				break;
		case	B:	aprint(level1,'B','2');outbit(1);outbit(1);
				lenprint(pt->lcht.pt->len);
				break;
		case	S:	if (dd==0)
					{ aprint(level1,'S','1');outbit(0); }
				else	{ aprint(level1,'S','2');outbit(0);outbit(0); }
				lenprint(pt->lcht.pt->len);
				asons(pt->lcht.pt,hh,dd,level1);
				break;
		case	T:	if (hh==0)
					{ aprint(level1,'T','1');outbit(0); }
				else	{ aprint(level1,'T','2');outbit(0);outbit(1); }
				lenprint(pt->lcht.pt->len);
				asons(pt->lcht.pt,hh,dd,level1);
				break;
		default:	fprintf(stderr,"%s: funny character: %d-%c-\n",
					Progname,(unsigned int)sym3,sym3);
				perr(HE_MSG,"error 3 in asons");
	}
	switch(sym4)	{
		case	W:	if ((sym1==B) && sym1==sym2 && sym2==sym3)
					{aprint(level1,'U','1'); outbit(1);(pt->rcht.pt->len)--;}
				else {aprint(level1,'W','2');outbit(1);outbit(0);}
				lenprint(pt->rcht.pt->len);
				break;
		case	B:	if ((sym1==W) && sym1==sym2 && sym2==sym3)
					{aprint(level1,'U','1'); outbit(1);(pt->rcht.pt->len)--; }
				else {aprint(level1,'B','2');outbit(1);outbit(1);}
				lenprint(pt->rcht.pt->len);
				break;
		case	S:	if (dd==0)
					{ aprint(level1,'S','1');outbit(0); }
				else	{ aprint(level1,'S','2');outbit(0);outbit(0); }
				lenprint(pt->rcht.pt->len);
				asons(pt->rcht.pt,hh,dd,level1);
				break;
		case	T:	if (hh==0)
					{ aprint(level1,'T','1');outbit(0); }
				else	{ aprint(level1,'T','2');outbit(0);outbit(1); }
				lenprint(pt->rcht.pt->len);
				asons(pt->rcht.pt,hh,dd,level1);
				break;
		default:	fprintf(stderr,"%s: funny character: %d-%c-\n",
					Progname,(unsigned int)sym4,sym4);
				perr(HE_MSG,"error 4 in asons");
	}
	return(0);
}

int aprint(level,ch1,ch2)

int level;
char ch1,ch2;

{
	int i;

	if (asw) {
		putchar('\n');
		while(level-- >0) { for (i=0;i<7;i++)putchar(' ');}
		putchar(ch1);putchar(ch2);
	}
	return(0);
}

int lenprint(len)

int len;

{
	if (asw)
		printf(" (%d)",len);
	return(0);
}
