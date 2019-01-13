/*      Copyright (c) 1993 N. C. Krieger Lassen  and Rasmus Larsen, IMSOR

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/* Cubic: Enlarges image using cubic convolution as interpolation  
 *        method.                                                 
 * Usage: cubic [-m mfactor] [-s new_rows new_cols]              
 *        Enlarges by mfactor in both horisontal and vertical   
 *        direction, or Enlarges image to any given size
 *	  new_rows x new_cols 			
 *                                             
 * NCKL 20-11-1991 IMSOR,DTH.
 * Modified for short,int,float,complex  Rasmus Larsen, IMSOR/DTH october 1993
 */

#include <math.h>
#include <hipl_format.h>
#include <stdio.h>

float	cc();
void	fread_im();
void	syntax();

int main(argc,argv)
int argc;
char **argv;
{

	struct header	hd;
	float		*ipic,*opic;
	float		c,d,k,l,t1,t2,t3,t4;
	int		nbands,nsize,pfmt,i,j,m,K,L,Nr,Nc,NNr,NNc;
	int		lokal,xk,yk,xkk,ykk;
	float		t[16],s[16];
	int		mflag = 1,sflag = 0;
	float		Mr = 2.0;
	float		Mc = 2.0;

	Progname = strsave(*argv);

	for (i=1;i<argc;i++) {
		if (strncmp(argv[i], "-m", 2) == 0) {
			mflag = 1;
			if (strncmp(argv[i+1], "-", 1) == 0) {
				syntax();
				exit(1);
			}
			else {
				Mr = atof(argv[++i]);
				Mc = Mr;
			}
			continue;
		}
		if (strncmp(argv[i], "-s", 2) == 0) {
			sflag = 1;
			mflag = 0;
			if (strncmp(argv[i+1], "-", 1) == 0) {
				syntax();
				exit(1);
			}
			else NNr = atoi(argv[++i]);
			if (strncmp(argv[i+1], "-", 1) == 0) {
				syntax();
				exit(1);
			}
			else NNc = atoi(argv[++i]);
			continue;
		}
		if (strncmp(argv[i],"-U",2) == 0) {
			syntax();
			exit(1);
		}
		if (strncmp(argv[i],"-",1) == 0) {
			fprintf(stderr,"%s: unknown option '%s'\n",Progname,argv[i]);
			syntax();
			exit(1);
		}
	}

	if (mflag+sflag != 1) {
		syntax();
		exit(1);
	}
	read_header(&hd);
	pfmt = hd.pixel_format;
	if (pfmt != PFBYTE && pfmt != PFSHORT && pfmt != PFINT && pfmt != PFFLOAT && pfmt != PFCOMPLEX) {
		fprintf(stderr,"%s: Bad format\n",Progname);
		exit(1);
	}

	Nr=hd.orows;
	Nc=hd.ocols;
	nbands = hd.num_frame;
	nsize = Nr * Nc;
	if (mflag) {
		fprintf(stderr,"%s: horizontal enlargement: %f\n",Progname,Mr);
		fprintf(stderr,"%s: vertical enlargement: %f\n",Progname,Mc);
		NNr = (int) (Mr*Nr);
		NNc = (int) (Mc*Nc);
		if (NNr<=NNc) NNc = NNr * Nc / Nr;
		else NNr = NNc * Nr / Nc;
	}
	if (sflag) {
		Mr = NNr/((float)Nr);
		Mc = NNc/((float)Nc);
		fprintf(stderr,"%s: horizontal enlargement: %f\n",Progname,Mr);
		fprintf(stderr,"%s: vertical enlargement: %f\n",Progname,Mc);
	}
	hd.orows=NNr;
	hd.rows=NNr;
	hd.ocols=NNc;
	hd.cols=NNc;

	if (pfmt != PFCOMPLEX) {
		hd.pixel_format = PFFLOAT;
		ipic = (float *) malloc(nsize*sizeof(float));
		opic = (float *) calloc(NNr*NNc,sizeof(float));
	}
	else {
		hd.pixel_format = PFCOMPLEX;
		ipic = (float *) malloc(2*nsize*sizeof(float));
		opic = (float *) calloc(2*NNr*NNc,sizeof(float));
	}

	update_header(&hd,argc,argv);
	write_header(&hd);

	for (m=0;m<nbands;m++) {
		if (pfmt != PFCOMPLEX) {
		fread_im(pfmt,nsize,ipic,stdin);
		for (i=0;i<NNr;i++) for (j=0;j<NNc;j++) {
			k = ((float) i)/Mr;
			l = ((float) j)/Mc;
			K = (int) floor(k);                       
			L = (int) floor(l);                      
			c = k - K;
			d = l - L;
			lokal = 0;
			for (xk=K-1;xk<=K+2;xk++) for (yk=L-1;yk<=L+2;yk++) {
				if (xk<0) xkk=0;
				else {
					if (xk>=Nr) xkk=Nr-1;
					else xkk = xk;
				}
				if (yk<0) ykk=0;
				else {
					if (yk>=Nc) ykk=Nc-1;
					else ykk = yk;
				}
				t[lokal] = ipic[xkk*Nc+ykk];
				lokal++;
			}
			t1 = cc(t[0],t[1],t[2],t[3],d);
			t2 = cc(t[4],t[5],t[6],t[7],d);
			t3 = cc(t[8],t[9],t[10],t[11],d);
			t4 = cc(t[12],t[13],t[14],t[15],d);
			opic[i*NNc+j] = cc(t1,t2,t3,t4,c); 
		}
		if (fwrite(opic,sizeof(float),NNc*NNr,stdout) != NNc*NNr) {
			fprintf(stderr,"%s: Write error\n",Progname);
			exit(1);
		}
		}
		else {
		fread_im(pfmt,2*nsize,ipic,stdin);
		for (i=0;i<NNr;i++) for (j=0;j<NNc;j++) {
			k = ((float) i)/Mr;
			l = ((float) j)/Mc;
			K = (int) floor(k);                       
			L = (int) floor(l);                      
			c = k - K;
			d = l - L;
			lokal = 0;
			for (xk=K-1;xk<=K+2;xk++) for (yk=L-1;yk<=L+2;yk++) {
				if (xk<0) xkk=0;
				else {
					if (xk>=Nr) xkk=Nr-1;
					else xkk = xk;
				}
				if (yk<0) ykk=0;
				else {
					if (yk>=Nc) ykk=Nc-1;
					else ykk = yk;
				}
				t[lokal] = ipic[2*(xkk*Nc+ykk)];
				s[lokal] = ipic[2*(xkk*Nc+ykk)+1];
				lokal++;
			}
			t1 = cc(t[0],t[1],t[2],t[3],d);
			t2 = cc(t[4],t[5],t[6],t[7],d);
			t3 = cc(t[8],t[9],t[10],t[11],d);
			t4 = cc(t[12],t[13],t[14],t[15],d);
			opic[2*(i*NNc+j)] = cc(t1,t2,t3,t4,c); 
			t1 = cc(s[0],s[1],s[2],s[3],d);
			t2 = cc(s[4],s[5],s[6],s[7],d);
			t3 = cc(s[8],s[9],s[10],s[11],d);
			t4 = cc(s[12],s[13],s[14],s[15],d);
			opic[2*(i*NNc+j)+1] = cc(t1,t2,t3,t4,c); 
		}
		if (fwrite(opic,sizeof(float),2*NNc*NNr,stdout) != 2*NNc*NNr) {
			fprintf(stderr,"%s: Write error\n",Progname);
			exit(1);
		}
		}
	}
	return(0);
}

float cc(x1,x2,x3,x4,a)
float x1,x2,x3,x4,a;
{
	float res;
   
	res = x2+a*((x3-x1)+a*((2*x1-2*x2+x3-x4)+a*(x2-x1-x3+x4)));
	return res;
}

/* fread_im: reads image from stdin into a float format */
void fread_im(pfmt,npix,image,fil)

int		pfmt,npix;
float		*image;
FILE		*fil;

{
	byte		*bpp,*b;
	short		*spp,*s;
	int		*ipp,*r;
	float		*fpp;
	int		i;


	fpp = image;

	if (pfmt == PFBYTE) {
		bpp = (byte *) malloc(npix);
		b = bpp;
		if(fread(bpp,sizeof(byte),npix,fil) != npix) {
			fprintf(stderr,"%s: Error during read\n",Progname);
			exit(1);
		}
		for (i=0;i<npix;i++) *fpp++ = (float) *b++;
		free(bpp);
	}
	if (pfmt == PFSHORT) {
		spp = (short *) malloc(2*npix);
		s = spp;
		if(fread(spp,sizeof(short),npix,fil) != npix) {
			fprintf(stderr,"%s: Error during read\n",Progname);
			exit(1);
		}
		for (i=0;i<npix;i++) *fpp++ = (float) *s++;
		free(spp);
	}
	if (pfmt == PFINT) {
		ipp = (int *) malloc(4*npix);
		r = ipp;
		if(fread(ipp,sizeof(int),npix,fil) != npix) {
			fprintf(stderr,"%s: Error during read\n",Progname);
			exit(1);
		}
		for (i=0;i<npix;i++) *fpp++ = (float) *r++;
		free(ipp);
	}
	if (pfmt == PFFLOAT) 
		if(fread(image,sizeof(float),npix,fil) != npix) {
			fprintf(stderr,"%s: Error during read\n",Progname);
			exit(1);
		}
	if (pfmt == PFCOMPLEX) 
		if(fread(image,sizeof(float),npix,fil) != npix) {
			fprintf(stderr,"%s: Error during read\n",Progname);
			exit(1);
		}
}

void    syntax()
{
        fprintf(stderr,"usage: %s\n",Progname);
        fprintf(stderr,"\t\t[-m mfactor<float>] | [-s new_rows new_cols]\n");
        fprintf(stderr,"\t\t[-U]\n");
}
