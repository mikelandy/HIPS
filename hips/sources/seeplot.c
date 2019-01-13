/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/* seeplot - to see a sequence of PLOT3D format in ascii.
 *
 * usage: seeplot < plotfile
 *
 * To load: cc -o seeplot seeplot.c -lhips
 *
 * Yoav Cohen 11/16/82
 * HIPS 2 - msl - 7/24/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PLOT3D,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	char *buf[FBUFLIMIT];
	struct header hd;
	int i,j,inbytes,flags,op,vflag,mflag;
	double sh_v[3],rot_m[3][3],x1,y1,z1,x2,y2,z2,b;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_cpf(fp,&hd,types,filename);
	for(i=0;i<hd.num_frame;i++) {
		inbytes=read_frame(fp,buf,FBUFLIMIT,&flags,sh_v,rot_m,i,
			filename);
		vflag=flags&2; mflag=flags&1;
		if (vflag)
			printf("vector: yes, ");
		else
			printf("vector: no, ");
		if (mflag)
			printf("matrix: yes,\n");
		else
			printf("matrix: no.\n");
		if (vflag)
			printf("vector:  %f %f %f\n",sh_v[0],sh_v[1],sh_v[2]);
		if (mflag) {
			printf("matrix:\n");
			for(j=0;j<3;j++)
				printf("%f %f %f\n",rot_m[j][0],rot_m[j][1],
					rot_m[j][2]);
		}
		printf("bytes read = %d\n",inbytes);
		for (j=0;j<inbytes;) {
			j=getplot(buf,j,&op,&b,&x1,&y1,&z1,&x2,&y2,&z2);
			switch(op) {
				case 'p': printf("p %f %f %f %f\n",b,x1,y1,z1);
					  break;
				case 'v': printf("v %f %f %f %f %f %f %f\n",
						b,x1,y1,z1,x2,y2,z2);
					  break;
				case 'n': printf("n %f %f %f\n",x2,y2,z2);
					  break;
				default:  printf("unrecognized op %d\n",op);
					  break;
			}
		}
		printf("end of frame no. %d\n\n",i);
	}
	return(0);
}
