/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/* grot.c -  rotate a 3-D graph
 * 
 * usage:	grot [-p x [y [z]]]
 * 
 * defaults:	x,y,z=0.0 degrees
 *
 * To load:	cc -o grot grot.c -lhips -lm
 *
 * Yoav Cohen 11/15/82
 * HIPS 2 - msl - 7/24/91
 */

#include <stdio.h>
#include <math.h>
#include <hipl_format.h>

double 	sh_v[3],rot_m[3][3],r[3][3],tv[3];
double xf,yf,zf;

static Flag_Format flagfmt[] = {
	{"p",{LASTFLAG},1,{{PTDOUBLE,"0.","x"},{PTDOUBLE,"0.","y"},
		{PTDOUBLE,"0.","z"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PLOT3D,LASTTYPE};
int isatty();
void fix_rot(),mulm3();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int nf,i,j,k,flags,inbytes;
	char buf[FBUFLIMIT];
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&zf,&yf,&xf,FFONE,&filename);
	fp = hfopenr(filename);

	/* is there a file input ? */
	if (!isatty(fileno(fp))) {
		fread_hdr_cpf(fp,&hd,types,filename);
		if (hd.num_frame <= 0)
			perr(HE_MSG,"no. of frames must be positive ");
		nf=hd.num_frame;
		write_headeru(&hd,argc,argv);
		fix_rot();
		for(i=0;i<nf;i++) {
			inbytes=read_frame(fp,buf,FBUFLIMIT,&flags,sh_v,rot_m,
				i,filename);
			mulm3(r,rot_m,rot_m);
			for(j=0;j<3;j++) {
				tv[j]=0.0;
				for(k=0;k<3;k++)
					tv[j] += sh_v[k]*r[j][k];
			}
			for(j=0;j<3;j++)
				sh_v[j]=tv[j];
			write_frame(stdout,buf,inbytes,sh_v,rot_m,i);
		}
	}
	else
		perr(HE_MSG,"no input file");

	return(0);
}

void fix_rot()

{
	double xa,ya,za,a[3][3],b[3][3],c[3][3],t[3][3];

	xa=H_PI/180.*xf;
	ya=H_PI/180.*yf;
	za=H_PI/180.*zf;

	a[0][0]=a[1][1]=cos(xa); a[0][1]= -(a[1][0] = sin(xa) );
	a[0][2]=a[1][2]=a[2][0]=a[2][1]=0.0;
	a[2][2]=1.;

	b[0][0]=b[2][2]=cos(ya);
	b[2][0]=sin(ya); b[0][2]= -b[2][0];
	b[0][1]=b[1][0]=b[1][2]=b[2][1]=0;
	b[1][1]=1.0;

	c[0][0]=1;
	c[0][1]=c[0][2]=c[1][0]=c[2][0]=0.;
	c[1][1]=c[2][2]=cos(za);
	c[2][1]=sin(za); c[1][2]= -c[2][1];

	mulm3(b,a,t); mulm3(c,t,r); 

	return;
}


void mulm3(a,b,c) 

double a[3][3],b[3][3],c[3][3];

{
	double t[3][3];
	int i,j,k;
	
	for(i=0;i<3;i++)
	for(j=0;j<3;j++) {
		t[i][j]=0.0;
		for(k=0;k<3;k++)
			t[i][j]+= a[i][k]*b[k][j];
	}
	for(i=0;i<3;i++)
	    for(j=0;j<3;j++)
		c[i][j]=t[i][j];
	return;
}
