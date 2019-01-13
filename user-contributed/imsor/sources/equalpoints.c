/*      Copyright (c) 1992  Karsten Hartelius, IMSOR.

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   

 equalpoints - checks if irregular points have same x and/or y coordinates.
	       and excludes these points from output if they appear.	

 usage: see manual

 to load: 
	cc -o equalpoints equalpoints.c  -L /gsnk1/hips2/lib -lm  -lhips

 to run:
	equalpoints parameters < Irregular-file  > Irregular-file

 include-files: util.h

 Karsten Hartelius 10/4 1992. 


*/
#include <hipl_format.h>
#include <stdio.h>
#include  "util.h"
#define 	EPSILON 0.000000001

FILE	*fp;

enum evaluation { X, Y, XORY, XANDY };
enum evaluation  eval;

int main(argc,argv)
int	argc;
char	*argv[];
{
	struct header hd;
	byte		val;
	int		i, j, k, count, N, frames, format, print, Nout, equal();
	Dmatrix	in, out;
	Bvector	drop;

	print=0;
	eval=XANDY;
	for (i=1;i<argc;i++){
		if (!(strcmp(argv[i],"xy"))){
			eval=XANDY; continue; } 
		if (!(strcmp(argv[i],"x"))){ 
			eval=X; continue; } 
		if (!(strcmp(argv[i],"y"))){
			eval=Y; continue; } 
		if (!(strcmp(argv[i],"x,y"))){ 
			eval=XORY; continue; } 
		if (!(strcmp(argv[i],"-p"))){
			print=1; continue; }
		perr(HE_MSG,"wrong argument");
		}

	read_header(&hd);
     val=0; 
	count=1;
	if (findparam(&hd,"Irregular") != NULLPAR)
		getparam(&hd,"Irregular",PFBYTE,&count,&val);
	if (val == 0)
		perr(HE_MSG,"File is not of type: Irregular");

	N=hd.ocols;
	frames=hd.num_frame; 
	format=hd.pixel_format;

	in=dmatrix(frames,N);
	fread_to_dmat(stdin,in,frames,N,format);
	drop=bvector(N);

	fp=fopen("equal.out","w");
	for (i=0;i<N;i++) for (j=i+1;j<N;j++) {
		if (equal(in[0][i],in[0][j]) && equal(in[1][i],in[1][j])){
			fprintf(fp,"points %d and %d have same x- and y-coordinate : (%8.4f,%8.4f)\n",i,j,in[0][i],in[1][i]);
			if (!drop[i])
				drop[j]=1;
			}
		else
		if ((eval==X || eval==XORY) && equal(in[0][i],in[0][j])){ 
			fprintf(fp,"points %d and %d have same x-coordinate : %8.4f\n",i,j,in[0][i]);
			if (!drop[i])
				drop[j]=1;
			}
		else
		if ((eval==Y || eval==XORY) && equal(in[1][i],in[1][j])){ 
			fprintf(fp,"points %d and %d have same y-coordinate : %8.4f\n",i,j,in[1][i]);
			if (!drop[i])
				drop[j]=1;
			}
		}
	fclose(fp);

	if (print){
		Nout=0;	
		for (i=0;i<N;i++)
			if (!drop[i]) Nout++; 
		out=dmatrix(frames,Nout);
		k=0;
		for (i=0;i<N;i++) if (!drop[i]){
			out[0][k]=in[0][i];
			out[1][k]=in[1][i];
			for (j=2;j<frames;j++)
				out[j][k]=in[j][i];
			k++;
			}
		hd.ocols=hd.cols=Nout;
		update_header(&hd,argc,argv);
		write_header(&hd);
		fwrite_from_dmat(stdout,out,frames,Nout,format);
		}
	return(0);
}

int equal(v1,v2)
double	v1, v2;
{
	double	d;
	d=(v1 > v2)?v1-v2:v2-v1;
	return( d<EPSILON );
}
