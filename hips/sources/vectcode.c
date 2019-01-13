/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * vectcode.c - compute bit rates for spanning tree images and decode
 *
 * usage:	vectcode [-n] [-h | -q] <iseq >oseq
 *
 * -n gives no image output, and just computes bit rates.
 * -h simulates the algorithm with half the resolution.
 * -q simulates the algorithm with a quarter the resolution.
 *
 * to load:	cc -o vectcode vectcode.c -lhips -lm
 *
 * Mike Landy - 1/12/83
 * HIPS 2 - msl - 7/24/91
 */

#include <stdio.h>
#include <hipl_format.h>

#define	NBIT	14
#define	STLIM	100
#define setpnt(x,y)	pict[y*c+x]=hips_hchar
#define	ab(b,n)	bcnt[b] += n

int dx[9] = {0,1,1,0,-1,-1,-1,0,1};
int dy[9] = {0,0,1,1,1,0,-1,-1,-1};
int bcnt[NBIT];
byte *pict;
int r,c,fr;
h_boolean nflag;
int resol = 1;
void fillvect();

static Flag_Format flagfmt[] = {
	{"n",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"h",{"q",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"q",{"h",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFSPAN,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	Filename filename;
	FILE *fp;
	int i,f,m1,m2,m3,m4,dx,dy,dx1,dy1,dx2,dy2,ox,oy,iso,ressave;
	short b,x,y,x1,y1,xsave,ysave;
	short nstk,stkx[STLIM],stky[STLIM];
	int ch;
	h_boolean hflag,qflag,iflag,iflag2,eflag;
	Pixelval val;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&nflag,&hflag,&qflag,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_cpf(fp,&hd,types,filename);
	for (i=0;i<NBIT;i++)
		bcnt[i] = 0;
	iso = 0;
	if (hflag)
		resol = 2;
	else if (qflag)
		resol = 4;
	setformat(&hd,PFBYTE);
	hd.pixel_format = PFBYTE;
	r = hd.orows; c = hd.ocols;
	if (r > 128 || c > 64)
		perr(HE_MSG,"image size must by 128x64 or smaller");
	fr = hd.num_frame;
	if (!nflag)
		write_headeru(&hd,argc,argv);
	alloc_image(&hd);
	pict = hd.image;
	iflag2 = FALSE;
	ressave = resol == 4 ? 4 : (resol == 2 ? 2 : 0);
	val.v_byte = hips_lchar;
	for (f=0;f<fr;f++) {
		h_setimage(&hd,&val);
		iflag = TRUE;
		eflag = FALSE;
		x = y = 0; /* assume (0,0) for previous vector end */
		while (!eflag) {
			if ((ch = getc(fp)) == EOF)
				perr(HE_READFRFILE,f,filename);
			if (iflag && ch!='I')
				perr(HE_MSG,
					"frame doesn't start with Init point");
			iflag = FALSE;
			if (iflag2 && ch=='I')
				iso++;
			iflag2 = FALSE;
			switch(ch) {
	
			case 'I':
				iflag2 = TRUE;
				ab(0,2);
				ab(1,13-ressave);
				ox = x;
				oy = y;
				if (fread(&x1,sizeof(short),1,fp) != 1)
					perr(HE_READFRFILE,f,filename);
				if (fread(&y1,sizeof(short),1,fp) != 1)
					perr(HE_READFRFILE,f,filename);
				xsave = x1;
				ysave = y1;
				x1 &= ~(resol-1);
				y1 &= ~(resol-1);
				dx = x1 - x;
				dy = y1 - y;
				if (ox < 7*resol)
					dx -= (7*resol-ox);
				if (ox > (c-9*resol))
					dx += (ox-c+9*resol);
				if (oy < 7*resol)
					dy -= (7*resol-oy);
				if (oy > (r-9*resol))
					dy += (oy-r+9*resol);
				ab(11,1);
				if (dx>=-7*resol && dx<=8*resol &&
				    dy>=-7*resol && dy<=8*resol)
					ab(12,8);
				else
					ab(13,13-ressave);
				setpnt(x,y);
				stkx[0] = xsave;
				stky[0] = ysave;
				nstk = 0;
				x = x1;
				y = y1;
				continue;
			case 'C':
				ab(0,1);
				ab(3,13-ressave);
				if (fread(&x1,sizeof(short),1,fp) != 1)
					perr(HE_READFRFILE,f,filename);
				if (fread(&y1,sizeof(short),1,fp) != 1)
					perr(HE_READFRFILE,f,filename);
				xsave = x1;
				ysave = y1;
				x1 &= ~(resol-1);
				y1 &= ~(resol-1);
				dx2 = dx1 = dx = x1 - x;
				dy2 = dy1 = dy = y1 - y;
				if (x < 7*resol)
					dx -= (7*resol-x);
				if (x > (c-9*resol))
					dx += (x-c+9*resol);
				if (y < 7*resol)
					dy -= (7*resol-y);
				if (y > (r-9*resol))
					dy += (y-r+9*resol);
				if (x < 1*resol)
					dx1 -= (1*resol-x);
				if (x > (c-3*resol))
					dx1 += (x-c+3*resol);
				if (y < 1*resol)
					dy1 -= (1*resol-y);
				if (y > (r-3*resol))
					dy1 += (y-r+3*resol);
				if (x < 8*resol)
					dx2 -= (8*resol-x);
				if (x > (c-9*resol))
					dx2 += (x-c+9*resol);
				if (y < 7*resol)
					dy2 -= (7*resol-y);
				if (y > (r-9*resol))
					dy2 += (y-r+9*resol);
				ab(4,1);
				if ((dx>=-7*resol && dx<=8*resol &&
				    dy>=-7*resol && dy<=8*resol) ||
					(dx==-8*resol && dy==0))
					ab(5,8);
				else
					ab(6,13-ressave);
				if ((dx1>=-1*resol && dx1<=2*resol &&
				    dy1>=-1*resol && dy1<=2*resol) ||
					(dx1==-2*resol && dy1==0)) {
					ab(7,2);
					ab(8,4);
				}
				else if (dx2>=-8*resol && dx2<=8*resol &&
				    dy2>=-7*resol && dy2<=8*resol) {
					ab(7,1);
					ab(9,8);
				}
				else {
					ab(7,2);
					ab(10,13-ressave);
				}
				fillvect(x,y,x1,y1);
				for (i=0;i<=nstk;i++)
					if (stkx[i]==xsave && stky[i]==ysave)
						goto stacked;
				if (++nstk >= STLIM)
					perr(HE_MSG,"stack overflow");
				stkx[nstk] = xsave;
				stky[nstk] = ysave;
			    stacked:
				x = x1;
				y = y1;
				continue;
			case 'B':
				ab(0,3);
				if (fread(&b,sizeof(short),1,fp) != 1)
					perr(HE_READFRFILE,f,filename);
				if (b<0 || b>nstk)
					perr(HE_MSG,
					  "Back-to code to nonexistent point");
				if (nstk <= 1)
					ab(2,0);
				else if (nstk == 2)
					ab(2,1);
				else if (nstk <= 4)
					ab(2,2);
				else if (nstk <= 8)
					ab(2,3);
				else
					ab(2,(((int) b/7)+1)*3);
				x = stkx[b];
				y = stky[b];
				x &= ~(resol-1);
				y &= ~(resol-1);
				continue;
			case 'E':
				ab(0,3);
				eflag = TRUE;
				continue;
			default:
				perr(HE_MSG,"unknown vector code");
			}
		}
		if (!nflag)
			write_image(&hd,f);
	}
	m1 = bcnt[0]+bcnt[1]+bcnt[2]+bcnt[3];
	m2 = bcnt[0]+bcnt[1]+bcnt[2]+bcnt[4]+bcnt[5]+bcnt[6];
	m3 = bcnt[0]+bcnt[1]+bcnt[2]+bcnt[7]+bcnt[8]+bcnt[9]+bcnt[10];
	m4 = bcnt[0]+bcnt[11]+bcnt[12]+
			bcnt[13]+bcnt[2]+bcnt[7]+bcnt[8]+bcnt[9]+bcnt[10];
	fprintf(stderr,"%s: 	bit rates\n\
method 1:	%d + %d + %d + %d\n\
				= %d bits = %f bits/frame\n\
\n\
method 2:	%d + %d + %d + %d + %d + %d\n\
				= %d bits = %f bits/frame\n\
\n\
method 3:	%d + %d + %d + %d + %d + %d + %d\n\
				= %d bits = %f bits/frame\n\
\n\
method 4:	%d + %d + %d + %d + %d + %d + %d + %d + %d\n\
				= %d bits = %f bits/frame\n\
%d isolated points = %f isolated points/frame\n",
		Progname,bcnt[0],bcnt[1],bcnt[2],bcnt[3],m1,((float) m1)/fr,
		bcnt[0],bcnt[1],bcnt[2],bcnt[4],bcnt[5],bcnt[6],m2,
			((float) m2)/fr,
		bcnt[0],bcnt[1],bcnt[2],bcnt[7],bcnt[8],bcnt[9],bcnt[10],m3,
			((float) m3)/fr,
		bcnt[0],bcnt[11],bcnt[12],bcnt[13],bcnt[2],bcnt[7],bcnt[8],
			bcnt[9],bcnt[10],m4,((float) m4)/fr,
		iso,((float) iso)/fr);
	return(0);
}

void fillvect(x0,y0,x1,y1)

short x0,y0,x1,y1;

{
	int dmv,hmv,nd,nh,dh,rem,i,x,y,mv;

	if (nflag)
		return;
	if (x0 < x1) {
		if (y0 < y1) {
			if ((x1-x0) > (y1-y0)) {
				dmv = 2;
				hmv = 1;
				nd = y1 - y0;
				nh = (x1-x0) - nd;
			}
			else {
				dmv = 2;
				hmv = 3;
				nd = x1 - x0;
				nh = (y1-y0) - nd;
			}
		}
		else {
			if ((x1-x0) > (y0-y1)) {
				dmv = 8;
				hmv = 1;
				nd = y0 - y1;
				nh = (x1-x0) - nd;
			}
			else {
				dmv = 8;
				hmv = 7;
				nd = x1 - x0;
				nh = (y0-y1) - nd;
			}
		}
	}
	else {
		if (y0 < y1) {
			if ((x0-x1) > (y1-y0)) {
				dmv = 4;
				hmv = 5;
				nd = y1 - y0;
				nh = (x0-x1) - nd;
			}
			else {
				dmv = 4;
				hmv = 3;
				nd = x0 - x1;
				nh = (y1-y0) - nd;
			}
		}
		else {
			if ((x0-x1) > (y0-y1)) {
				dmv = 6;
				hmv = 5;
				nd = y0 - y1;
				nh = (x0-x1) - nd;
			}
			else {
				dmv = 6;
				hmv = 7;
				nd = x0 - x1;
				nh = (y0-y1) - nd;
			}
		}
	}

	setpnt(x0,y0);
	x = x0;
	y = y0;
	dh = nd + nh;
	rem = dh/2;
	
	for (i=0;i<dh;i++) {
		rem += nh;
		if (rem >= dh) {
			rem -= dh;
			mv = hmv;
		}
		else
			mv = dmv;
		x += dx[mv];
		y += dy[mv];
		setpnt(x,y);
	}
}
