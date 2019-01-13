/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * plot3tov.c - convert a sequence from PLOT3D format to UNIX PLOT format
 *
 * The output is not preceded by a header!
 *
 * To load:	cc -o plot3tov plot3tov.c -lhips
 *
 * Yoav Cohen 17/11/82
 * HIPS 2 - msl - 8/1/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PLOT3D,LASTTYPE};
void convert(),cwrite();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int nf,flags,inbytes,i;
	char buf[FBUFLIMIT];
	double sh_v[3],rot_m[3][3];
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_cpf(fp,&hd,types,filename);
	if (hd.num_frame<=0)
		perr(HE_MSG,"no of images must be positive");
	nf=hd.num_frame;
	for (i=0;i<nf;i++) {
		inbytes = read_frame(fp,buf,FBUFLIMIT,&flags,sh_v,rot_m,
				i,filename);
		trans_frame(buf,inbytes,sh_v,rot_m,&flags);
		convert(stdout,buf,inbytes);
	}
	return(0);
}

void convert(fp,buf,nbuf)

FILE *fp;
char *buf;
int nbuf;

{
	int op,indx;
	double x,y,z,x2,y2,z2,b;
	short int s[2];
	char bbb;

	bbb='s'; s[0]= -256; s[1]= -256;
	cwrite(fp,&bbb,sizeof(char));
	cwrite(fp,(char *)s,2*sizeof(short int));
	s[0]=s[1]=256;
	cwrite(fp,(char *)s,2*sizeof(short int));
	indx = 0;
	while (indx<nbuf) {
		indx = getplot(buf,indx,&op,&b,&x,&y,&z,&x2,&y2,&z2);
		switch (op) {
		case 'p':	bbb='p'; s[0]= x+.5; s[1]= y+.5;
				cwrite(fp,&bbb,sizeof(char));
				cwrite(fp,(char *)s,2*sizeof(short int));
				break;
		case 'v':	bbb='l';
				cwrite(fp,&bbb,sizeof(char));
				s[0]=x+.5; s[1]=y+.5;
				cwrite(fp,(char *)s, 2*sizeof(short int));
				s[0]=x2+.5; s[1]=y2+.5;
				cwrite(fp,(char *)s, 2*sizeof(short int));
				break;
		case 'n':	bbb='n';
				s[0]=x+.5; s[1]=y+.5;
				cwrite(fp,&bbb,sizeof(char));
				cwrite(fp,(char *)s,2*sizeof(short int));
				break;
		default : 	perr(HE_MSG,"unrecognized op-code");
		}
	}
	bbb='e';
	cwrite(fp,&bbb,sizeof(char));
}

void cwrite(fp,b,nb) 

FILE *fp;
char *b;
int nb;

{
	if (fwrite(b,nb,1,fp) != 1)
		perr(HE_MSG,"error in writing");
}
