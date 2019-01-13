/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * ahc3.c - 3D adaptive hierarchical coding into binary trees of binary images
 *
 * usage: ahc3 [-t stackrow stackcol] [-f stackdepth]
 *			[-d] [-s] [-v] [-g | -a > outseq] < inseq 
 *
 * stackrow, stackcol and stackdepth determine the size of the area
 * which is ahc3-encoded as a tree in the forest.  All must be powers of
 * 2, stackrow and stackcol must be divisors of their corresponding
 * spatial dimensions.  Stackdepth defaults to the largest power of 2
 * divisor of the number of frames, and stackrow and stackcol default to
 * the largest power of 2 which is a divisor of the number of rows and
 * the number of columns, respectively.
 *
 * -g indicates that actual code should be generated.  The full code
 * is: {W,B,V,H,D}-->{10,11,00,010,011}.  (If only two cuts are possible
 * they are encoded by {00,01}; if one only, by {0}.)  The output is in a
 * special format: PFAHC3, each frame is bit-packed, with the last word
 * of the tree padded with zeroes to the right.  The number of output
 * bytes is reported on "stderr".  -a specifies that actual code be generated
 * in ASCII.  Only one of -g and -a may be specified.  if -a is specified the
 * output is not in HIPS format (there is no header).  Specification of -s 
 * produces statistics for each frame in addition to totals.  -v (verbose)
 * is useful only for debugging purposes, and prints messages about the 
 * program's progress.  -d (debug) is another debugging flag, which outputs
 * the entire stored tree structure.
 *
 * The input must be in byte-unpacked-format with 1 bit per pixel (in other
 * words, ahc3 only codes nonzero vs zero pixels).  The coding
 * assumptions are 2 bits per black or white node, 2 or 3 bits per meta
 * symbol. Except at the lowest level where the two pixels are coded as a
 * "nibble" in 1 bit.  Also if an area is divided into two homogenous
 * areas, the second area is coded in 1 bit (0) , since it must be of
 * color different from that of the first area.  (If -g is specified, the actual
 * code is generated.  It is left shifted and packed, with the last word of
 * each tree padded with 1's to the right.)  Compression statistics are
 * given on "stderr".  The program computes the number of input bits,
 * output bits and compression ratio.
 *
 * to load:	cc -o ahc3 ahc3.c -lhips
 *
 * Yoav Cohen - 4/10/83
 * modified by Mike Landy - 6/21/83
 * HIPS 2 - msl - 7/22/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"t",{LASTFLAG},1,{{PTINT,"-1","stackrow"},{PTINT,"-1","stackcol"},
		LASTPARAMETER}},
	{"f",{LASTFLAG},1,{{PTINT,"-1","stackdepth"},LASTPARAMETER}},
	{"s",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"d",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"v",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"g",{"a",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"a",{"g",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};

#define AHCNULL 0
#define B 1
#define W 2
#define V 3
#define H 4
#define D 5

byte *inpic;
struct node  {
	int len;
	char sym;
	union {
		struct node *pt;
		char ch;
	} lch;
	union {
		struct node *pt;
		char ch;
	} rch;
};
struct node ******p;
int nnr,nnc,nnd;

char *obuf;
int bufindex,bitindex,bytesout=0;
h_boolean gsw,asw,vsw,dsw;
int nr,nc,nrc,stackrow,stackcol,stackdepth,enc2(),encode();
int outbit(),flushbuf(),acode(),asons(),aprint(),lenprint();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo;
	Filename filename;
	FILE *fp;
	h_boolean ssw;
	byte *in;
	struct node *****p1,****p2,***p3,**p4,*p5;
	struct node *****tp1,****tp2,***tp3,**tp4,*tp5,*pt;
	int f,ifr,i,j,k,l,m,n,sumlen,len,v,h,d,s,ofr,it1,it2,r,c,low;
	int method;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&stackrow,&stackcol,&stackdepth,&ssw,
		&dsw,&vsw,&gsw,&asw,FFONE,&filename);
	fp = hfopenr(filename);
	fread_header(fp,&hd,filename);
	nr = hd.orows;
	nc = hd.ocols;
	f = hd.num_frame;
	if (nr<=0 || nc<=0 || f<=0)
		perr(HE_MSG,"input image dimensions must be greater than zero");
	if (stackrow < 0) {
		for (i=1;nr%(1<<i)==0;i++);
		stackrow = 1 << (i-1);
	}
	if (stackcol < 0) {
		for (i=1;nc%(1<<i)==0;i++);
		stackcol = 1 << (i-1);
	}
	if (stackdepth < 0) {
		for (i=1;f%(1<<i)==0;i++);
		stackdepth = 1 << (i-1);
	}
	if (stackrow<1 || stackcol<1 || stackdepth<1)
		perr(HE_MSG,"stack args must be >= 1");
	for (nnr=0;(1<<nnr)<stackrow;nnr++);
	for (nnc=0;(1<<nnc)<stackcol;nnc++);
	for (nnd=0;(1<<nnd)<stackdepth;nnd++);
	fprintf(stderr,"%s: stacksize is %d x %d x %d\n",Progname,stackrow,
		stackcol,stackdepth);
	if ((1<<nnr)!=stackrow || (1<<nnc)!=stackcol || (1<<nnd)!=stackdepth)
		perr(HE_MSG,"stack dimensions must be powers of 2");
	if ((nr % stackrow) != 0 || (nc % stackcol) != 0)
		perr(HE_MSG,
		    "stack sizes must divide the number of rows and columns");
	if (f < stackdepth)
		perr(HE_MSG,"stackdepth is too large");
	if (nnr==0 && nnc==0 && nnd==0)
		perr(HE_MSG,"stack size must be bigger than 1 x 1 x 1");
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
		f = ofr = hdp.num_frame;
	inpic = hdp.image;
	if (gsw)
		obuf=(char *)halloc(512,sizeof(char));
	p = (struct node ******) halloc(nnr+1,sizeof(struct node *****));
	p1 = (struct node *****)
		halloc((nnr+1)*(nnc+1),sizeof(struct node ****));
	p2 = (struct node ****)
		halloc((nnr+1)*(nnc+1)*(nnd+1),sizeof(struct node ***));
	p3 = (struct node ***)
		halloc(((nnc+1)*(nnd+1)*((1<<(nnr+1))-1)) - (1<<nnr),
			sizeof(struct node **));
	p4 = (struct node **)
		halloc(((nnd+1)*((1<<(nnr+1))-1)*((1<<(nnc+1))-1)) -
			((1<<nnr)*(1<<nnc)),
			sizeof(struct node *));
	p5 = (struct node *)
		halloc((((1<<(nnr+1))-1)*((1<<(nnc+1))-1)*((1<<(nnd+1))-1)) -
			((1<<nnr)*(1<<nnc)*(1<<nnd)),
			sizeof(struct node));

	/* internal allocation */

	tp1=p1; tp2=p2; tp3=p3; tp4=p4; tp5=p5;
	for (i=0;i<=nnr;i++) {
		p[i]=tp1; tp1 += (nnc+1);
		r=1<<(nnr-i);
		for (j=0;j<=nnc;j++) {
			p[i][j]=tp2; tp2 += (nnd+1);
			c=1<<(nnc-j);
			for (k=0;k<=nnd;k++) {
				if (i==0 && j==0 && k==0) continue;
				p[i][j][k]=tp3; tp3 += r;
				s=1<<(nnd-k);
				for (l=0;l<r;l++) {
					p[i][j][k][l]=tp4; tp4 += c;
					for (m=0;m<c;m++) {
						p[i][j][k][l][m]=tp5; tp5 += s;
					}
				}
			}
		}
	}
	if (vsw)
		fprintf(stderr,"%s: done allocation\n",Progname);
	if (gsw) {
		setsize(&hdp,nr,nc);
		dup_headern(&hdp,&hdo);
		hdo.pixel_format=PFAHC3;
		hdo.num_frame=ofr;
		setparam(&hdo,"stackrow",PFINT,1,stackrow);
		setparam(&hdo,"stackcol",PFINT,1,stackcol);
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

		for (it1=0;it1<nr/stackrow;it1++)
		    for (it2=0;it2<nc/stackcol;it2++) {
			if (vsw)
				fprintf(stderr,"%s: it1=%d, it2=%d\n",
					Progname,it1,it2);
			enc2(inpic+it1*nc*stackrow+it2*stackcol);
			if (vsw)
				fprintf(stderr,"%s: done enc2()\n",Progname);
			for (m=2;m<=nnr+nnc+nnd;m++)
			  for (v=0;v<=nnr && v<=m;v++)
			    for (h=0;h<=nnc && h<=m;h++) {
				d=m-v-h;
				if (d<0||d>nnd)continue;
			if (vsw)
				fprintf(stderr,"%s: calling encode(%d,%d,%d)  ",
					Progname,v,h,d);
				encode(v,h,d);
			if (vsw)
				fprintf(stderr,"...done\n");
			    }
		len=p[nnr][nnc][nnd][0][0][0].len;
		sumlen+=len;
		if (ssw&&f>1)
			fprintf(stderr,"%s: frame #%d, CR=%5.3f\n",Progname,
				ifr,(double)len/(stackrow*stackcol*stackdepth));
		if (dsw)	{
			for (i=0;i<=nnr;i++) {
				r=1<<(nnr-i);
				for (j=0;j<=nnc;j++) {
					c=1<<(nnc-j);
					for (k=0;k<=nnd;k++) {
						if (i==0 && j==0 && k==0)
							continue;
						if ((i+j+k)==1)
							low=1;
						else
							low=0;
						s=1<<(nnd-k);
						for (l=0;l<r;l++)
						for (m=0;m<c;m++)
						for (n=0;n<s;n++) {
							pt= &p[i][j][k][l][m][n];
							printf("%p ",(void *) pt);
							printf("p[%d][%d][%d][%d][%d][%d]  ",i,j,k,l,m,n);
							printf("sym=%d, len=%d, ",pt->sym,pt->len);
							if (!low)
								printf("pointers: %p %p",pt->lch.pt,pt->rch.pt);
							printf("\n");
						}
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

char *pic;

{
	struct node *pt;
	char *in1,*in2,*in11,*in22,*in111,*in222,p1,p2;
	int ir,ic,il;

/* To encode the m=1 diagonal */
	/* vertical pairs */
	in1=pic; in2=pic+nc;
	for (il=0;il<stackdepth;il++,in1+=nrc,in2+=nrc)
	for (ir=0,in11=in1,in22=in2;ir<stackrow/2;
		ir++,in11+=(nc+nc),in22+=(nc+nc))
	for (ic=0,in111=in11,in222=in22;ic<stackcol;ic++) {
		p1= *in111++; p2= *in222++;
		pt= &p[1][0][0][ir][ic][il];
		pt->len = 2;
		if (p1==p2)
			pt->sym = p1;
		else {
			pt->sym = H;
			pt->lch.ch = p1; pt->rch.ch = p2;
		}
	}
	/* horizontal pairs */
	in1=pic; in2=pic+1;
	for (il=0;il<stackdepth;il++,in1+=nrc,in2+=nrc)
	for (ir=0,in11=in1,in22=in2;ir<stackrow;ir++,in11+=nc,in22+=nc) 
	for (ic=0,in111=in11,in222=in22;ic<stackcol/2;ic++,in111+=2,in222+=2) {
		p1= *in111; p2= *in222; 
		pt= &p[0][1][0][ir][ic][il];
		pt->len = 2;
		if (p1==p2)
			pt->sym = p1;
		else {
			pt->sym = V;
			pt->lch.ch = p1; pt->rch.ch = p2;
		}
	}
	/* pairs across layers */
	in1=pic; in2=pic+nrc;
	for (il=0;il<stackdepth/2;il++,in1+=(nrc+nrc),in2+=(nrc+nrc))
	for (ir=0,in11=in1,in22=in2;ir<stackrow;ir++,in11+=nc,in22+=nc)
	for (ic=0,in111=in11,in222=in22;ic<stackcol;ic++) {
		p1= *in111++; p2= *in222++; 
		pt= &p[0][0][1][ir][ic][il];
		pt->len = 2;
		if (p1==p2)
			pt->sym = p1;
		else {
			pt->sym = D;
			pt->lch.ch = p1; pt->rch.ch = p2;
		}
	}
	return(0);
}

int encode(v,h,d)

int v,h,d;

{
	struct node *pt,*hch1,*hch2,*vch1,*vch2,*dch1,*dch2;
	char hsym,vsym,dsym,hsym1,hsym2,vsym1,vsym2,dsym1,dsym2;
	int ir,ic,il,hlen,vlen,dlen;
	int nbits,max;

	nbits=0; if (v>0) nbits++; if (h>0) nbits++; if (d>0) nbits++;
	if (nbits<=0||nbits>3)
		perr(HE_MSG,"error1 in encode()");
	for (il=0;il<(1<<(nnd-d));il++)
	for (ir=0;ir<(1<<(nnr-v));ir++)
	for (ic=0;ic<(1<<(nnc-h));ic++) {
		pt= &p[v][h][d][ir][ic][il];
		/* is a h-cut possible ? */
		if (v==0) hlen=0; /* no */
		else {		 /* yes */
			hch1 = &p[v-1][h][d][ir+ir][ic][il];
			hch2 = &p[v-1][h][d][ir+ir+1][ic][il];
			hsym1 = hch1->sym;
			hsym2 = hch2->sym;
			/* is it uniform ? */
			if ((hsym1==B && hsym2==B) || (hsym1==W && hsym2==W)) {
						/* yes */
				hsym=hsym1;
				hlen=2;
			}
			else	{ /* non-uniform */
				hlen = hch1->len + hch2->len + nbits;
				hsym = H;
				if ((hsym1==B||hsym1==W) &&
					(hsym2==B||hsym2==W))
						--hlen;
			}
		}
		/* is a v-cut possible ? */
		if (h==0)
			vlen=0; /* no */
		else {		 /* yes */
			vch1 = &p[v][h-1][d][ir][ic+ic][il];
			vch2 = &p[v][h-1][d][ir][ic+ic+1][il];
			vsym1 = vch1->sym;
			vsym2 = vch2->sym;
			/* is it uniform ? */
			if ((vsym1==B && vsym2==B)
			  || (vsym1==W && vsym2==W)) {  /* yes */
				vsym=vsym1;
				vlen=2;
			}
			else	{ /* non-uniform */
				vlen = vch1->len + vch2->len +
					((nbits==3)?2:nbits);
				vsym = V;
				if ((vsym1==B||vsym1==W) &&
					(vsym2==B||vsym2==W))
						--vlen;
			}
		}
		/* is a d-cut possible ? */
		if (d==0) dlen=0; /* no */
		else {		 /* yes */
			dch1 = &p[v][h][d-1][ir][ic][il+il];
			dch2 = &p[v][h][d-1][ir][ic][il+il+1];
			dsym1 = dch1->sym;
			dsym2 = dch2->sym;
			/* is it uniform ? */
			if ((dsym1==B && dsym2==B)
			  ||(dsym1==W && dsym2==W)) {  /* yes */
				dsym=dsym1;
				dlen=2;
			}
			else	{ /* non-uniform */
				dlen = dch1->len + dch2->len + nbits;
				dsym = D;
				if ((dsym1==B||dsym1==W) &&
					(dsym2==B||dsym2==W))
						--dlen;
			}
		}
		max=hlen; if (vlen>max)max=vlen; if (dlen>max)max=dlen; max++;
		if (hlen==0)hlen=max;
		if (vlen==0)vlen=max;
		if (dlen==0)dlen=max;
		if (hlen<= vlen && hlen<= dlen) {
			pt->sym = hsym; pt->len = hlen;
			if (hsym==H)
				{ pt->rch.pt = hch2; pt->lch.pt = hch1; }
			else
				{ pt->rch.pt = AHCNULL; pt->lch.pt = AHCNULL; }
		}
		else if (vlen <= dlen) {
			pt->sym = vsym;
			pt->len = vlen;
			if (vsym==V)
				{ pt->rch.pt = vch2; pt->lch.pt = vch1; }
			else
				{ pt->rch.pt = AHCNULL; pt->lch.pt = AHCNULL; }
		}
		else {
			pt->sym = dsym;
			pt->len = dlen;
			if (dsym==D)
				{ pt->rch.pt = dch2; pt->lch.pt = dch1; }
			else	
				{ pt->rch.pt = AHCNULL; pt->lch.pt = AHCNULL; }
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

	if (gsw)
		{bufindex=bitindex=0;obuf[0]=0;}
	else
		printf("\n\nAHC: Code for batch # %d\n\n",frame);
	pt = &p[nnr][nnc][nnd][0][0][0];
	sym=pt->sym;
	switch(sym) {
		case	W:	aprint(0,'W','2');outbit(1);outbit(0);
				break;
		case	B:	aprint(0,'B','2');outbit(1);outbit(1);
				break;
		case	V:	if (nnr==0&&nnd==0)
					{ aprint(0,'V','1');outbit(0); }
				else	{ aprint(0,'V','2');outbit(0);outbit(0); }
				break;
		case	H:	if (nnc==0&&nnd==0)
					{ aprint(0,'H','1');outbit(0); }
				else if (nnd==0)
					{ aprint(0,'H','2');outbit(0);outbit(1); }
				else if (nnc==0)
					{ aprint(0,'H','2');outbit(0);outbit(0); }
				else	{ aprint(0,'H','3');outbit(0);outbit(1);outbit(0); }
				break;
		case	D:	if (nnr==0&&nnc==0)
					{ aprint(0,'D','1');outbit(0); }
				else if (nnr==0||nnc==0)
					{ aprint(0,'D','2');outbit(0);outbit(1); }
				else	{ aprint(0,'D','3');outbit(0);outbit(1);outbit(1); }
				break;
		default: perr(HE_MSG,"error in acode()");
	}
	lenprint(pt->len);
	if (sym==H||sym==V||sym==D)
		 asons(pt,nnc,nnr,nnd,0);
	if (gsw)
		flushbuf();
	else
		putchar('\n');
	return(0);
}

int asons(pt,h,v,d,level) 

struct node *pt;
int h,v,d,level;

{
	char sym,sym1,sym2;
	int level1,hh,vv,dd;
	
	sym=pt->sym;
	if (sym==W||sym==B)
		perr(HE_MSG,"error 0 in asons()");
	level1=level+1;
/* sons are single pixels: */
	if (h+v+d==1) {
		if (pt->lch.ch==W)
			{ aprint(level1,'W','1');outbit(0); }
		else
			{ aprint(level1,'B','1');outbit(1); }
		return(0);
	}
/* other cases */
	
	hh=h;vv=v;dd=d;
	if (sym==D)
		dd--;
	else if (sym==H)
		vv--;
	else if (sym==V)
		hh--;
	sym1=pt->lch.pt->sym; sym2=pt->rch.pt->sym;

	switch(sym1)	{
		case	W:	aprint(level1,'W','2');outbit(1);outbit(0);
				lenprint(pt->lch.pt->len);
				break;
		case	B:	aprint(level1,'B','2');outbit(1);outbit(1);
				lenprint(pt->lch.pt->len);
				break;
		case	V:	if (vv==0&&dd==0)
					{ aprint(level1,'V','1');outbit(0); }
				else	{ aprint(level1,'V','2');outbit(0);outbit(0); }
				lenprint(pt->lch.pt->len);
				asons(pt->lch.pt,hh,vv,dd,level1);
				break;
		case	H:	if (hh==0&&dd==0)
					{ aprint(level1,'H','1');outbit(0); }
				else if (dd==0)
					{ aprint(level1,'H','2');outbit(0);outbit(1); }
				else if (hh==0)
					{ aprint(level1,'H','2');outbit(0);outbit(0); }
				else	{ aprint(level1,'H','3');outbit(0);outbit(1);outbit(0); }
				lenprint(pt->lch.pt->len);
				asons(pt->lch.pt,hh,vv,dd,level1);
				break;
		case	D:	if (vv==0&&hh==0)
					{ aprint(level1,'D','1');outbit(0); }
				else if (vv==0||hh==0)
					{ aprint(level1,'D','2');outbit(0);outbit(1); }
				else	{ aprint(level1,'D','3');outbit(0);outbit(1);outbit(1); }
				lenprint(pt->lch.pt->len);
				asons(pt->lch.pt,hh,vv,dd,level1);
				break;
		default:	fprintf(stderr,"%s: funny character: %d-%c-\n",
					Progname,(unsigned int)sym1,sym1);
				perr(HE_MSG,"error 1 in asons");
		}
	switch(sym2)	{
		case	W:	if (sym1==B || sym1==W)
					{aprint(level1,'U','1');outbit(1);
						(pt->rch.pt->len)--;}
				else
					{aprint(level1,'W','2');outbit(1);outbit(0);}
				lenprint(pt->rch.pt->len);
				break;
		case	B:	if (sym1==B || sym1==W)
					{aprint(level1,'U','1');outbit(1);
						(pt->rch.pt->len)--;}
				else
					{aprint(level1,'B','2');outbit(1);outbit(1);}
				lenprint(pt->rch.pt->len);
				break;
		case	V:	if (vv==0&&dd==0)
					{ aprint(level1,'V','1');outbit(0); }
				else	{ aprint(level1,'V','2');outbit(0);outbit(0); }
				lenprint(pt->rch.pt->len);
				asons(pt->rch.pt,hh,vv,dd,level1);
				break;
		case	H:	if (hh==0&&dd==0)
					{ aprint(level1,'H','1');outbit(0); }
				else if (dd==0)
					{ aprint(level1,'H','2');outbit(0);outbit(1); }
				else if (hh==0)
					{ aprint(level1,'H','2');outbit(0);outbit(0); }
				else	{ aprint(level1,'H','3');outbit(0);outbit(1);outbit(0); }
				lenprint(pt->rch.pt->len);
				asons(pt->rch.pt,hh,vv,dd,level1);
				break;
		case	D:	if (vv==0&&hh==0)
					{ aprint(level1,'D','1');outbit(0); }
				else if (vv==0||hh==0)
					{ aprint(level1,'D','2');outbit(0);outbit(1); }
				else	{ aprint(level1,'D','3');outbit(0);outbit(1);outbit(1); }
				lenprint(pt->rch.pt->len);
				asons(pt->rch.pt,hh,vv,dd,level1);
				break;
		default:	fprintf(stderr,"%s: funny character: %d -%c-\n",
					Progname,(unsigned int) sym2,sym2);
		        	perr(HE_MSG,"error 2 in asons");
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
