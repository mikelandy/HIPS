/*	Copyright (c) 1990 Jens Michael Carstensen
Disclaimer:  No guarantees of performance accompany this software,
nor is any responsility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliality.   */

/*
 * hips2pcl.c - converts a binary image for printing on
 *              the HP LaserJet printer
 *
 * usage:       hips2pcl [-1234] <iseq >pclseq
 *
 * where the input sequence is in byte format with 1 or 8 bit per pixel.
 * The output sequence can be piped directly to an HP LaserJet printer
 * through the lp command.
 *
 * to load:    cc -o hips2pcl hips2pcl.c -lhipl
 *
 * Jens Michael Carstensen - 23/1/90
 */

#include <hipl_format.h>
#include <stdio.h>
#include <string.h>

#define Byte         unsigned char
#define LJreset      sprintf(out,"\33E");fwrite(out,1,strlen(out),stdout);
#define LJdpi(dpi)   sprintf(out,"\33*t%dR",dpi);fwrite(out,1,strlen(out),stdout);
#define LJposx(xp)   sprintf(out,"\33*p%dX",xp);fwrite(out,1,strlen(out),stdout);
#define LJposy(yp)   sprintf(out,"\33*p%dY",yp);fwrite(out,1,strlen(out),stdout);
#define LJstrt       sprintf(out,"\33*r1A");fwrite(out,1,strlen(out),stdout);
#define LJsend(bpr)  sprintf(out,"\33*b%dW",bpr);fwrite(out,1,strlen(out),stdout);
#define LJend        sprintf(out,"\33*rB");fwrite(out,1,strlen(out),stdout);
#define LJlandscape  sprintf(out,"\33&l1O");fwrite(out,1,strlen(out),stdout);
#define LJpitch(cpi) sprintf(out,"\33(s%dH",cpi);fwrite(out,1,strlen(out),stdout);
#define LJheight(hh) sprintf(out,"\33(s%dV",hh);fwrite(out,1,strlen(out),stdout);
#define LJitalic     sprintf(out,"\33(s1S");fwrite(out,1,strlen(out),stdout);
#define LJhelv       sprintf(out,"\33(s4T");fwrite(out,1,strlen(out),stdout);

int  vdots=3507,hdots=2338;

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	char *frame,*charp,*buf,out[20];
	int i,j,k;
	int nframe,nrows,ncols,readcol,fr;
        int rdpp,bpp,mag,mult,dpi,xp,yp,bpr,sum,count;

	Progname = strsave(*argv);

	read_header(&hd);
	if (hd.pixel_format != PFBYTE)
		perr(HE_MSG,"input sequence must be bytes");
	bpp = 8;
	nrows = hd.orows;
	ncols = hd.ocols;
	nframe = hd.num_frame;
	
        rdpp=(hdots-250)/ncols;
	if (rdpp == 0) 
          perr(HE_MSG,"image too large - max 3200x2048");
	if (bpp == 1) {
	  if (rdpp > 4) mag = 4; else mag = rdpp;
	  mult = 1;
	  readcol = (ncols+7)/8;
        }
	else {
	  switch (rdpp) {
	    case 1: mag=1; break;
	    case 2: case 10: case 11: case 14: mag=2; break;
            case 3: case 6: case 7: case 9: case 15: case 18: case 19:
              mag=3; break;
            default: mag=4;
          }
          mult = rdpp/mag;
	  readcol = ncols;
        }
	for (i=1;i<argc;i++) {
           if (argv[i][0]=='-') {
		switch (argv[i][1]) {
			case '1': mag=1; mult=1; break;
			case '2': mag=2; mult=1; break;
			case '3': mag=3; mult=1; break;
			case '4': mag=4; mult=1; break;
			default:
				perr(HE_MSG,"hips2pcl [-n] < inseq > outseq");
		}
           }
	}
        dpi = 300/mag; /* dots per inch */
        xp = hdots/2-mag*mult*ncols/2;
        yp = vdots/2-mag*mult*nrows/2;
        bpr = (ncols*mult+7)/8;

	frame = (char *) halloc(nrows*readcol,sizeof (char));
	charp=frame;
	buf = (char *) halloc(bpr,sizeof (char));

  
	for (fr=0;fr<nframe;fr++) {
                LJreset;
                LJposx(xp);
                LJposy(yp);
                LJdpi(dpi);
                LJstrt;
  
		if (fread(frame,1,nrows*readcol,stdin) != nrows*readcol)
			perr(HE_MSG,"unexpected end-of-file");

                for (k=0;k<nrows;k++) {
		  if (bpp == 1) for (i=0;i<bpr;i++) *(buf+i) = *charp++;
		  else
		  {
		    count=0;
                    for (i=0;i<bpr;i++) {
                      sum=0;
                      for (j=0;j<8 && i*8+j<ncols*mult;j++)
		      {
			sum += ((*charp!=0) ? 1 : 0) << (7-j);
			if (++count == mult) {count=0;charp++;}
                      }
                      *(buf+i)=sum;
                    }
		  }
                  for (i=0;i<mult;i++) {
                    LJsend(bpr);
		    if (fwrite(buf,1,bpr,stdout) != bpr)
		      perr(HE_MSG,"write error");
                  }
                } /* for */
                LJend;
                LJreset;
	}
	return(0);
}
