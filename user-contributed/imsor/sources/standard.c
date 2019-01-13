/*      Copyright (c) 1992  Karsten Hartelius, IMSOR.

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   

 standard - transforms a sequence so that mean = 0 and variance = 1 in 
	every frame.

 usage: see manual

 to load: 
    cc -o standard standard.c -L /gsnk1/hips2/lib  -lhips -lm

 to run:
    standard  parameters < HIPS-file > HIPS-file

 include-files: util.h

 Karsten Hartelius, IMSOR  10/4 1992. 

*/

#include <math.h>
#include <stdio.h>
#include <hipl_format.h>
#include "util.h"

int main(argc,argv)
int argc;
char *argv[];
{

	double 	mean,
			spred,
			missingval,
			var;
	byte		val;
	Dvector 	v;	
	struct 	header hd;
	int     	i, f, frames, format, Ndata, existpoints, count,  
			irregular=FALSE, missingflag=FALSE;

	Progname=strsave(*argv);
   	/* read parameters */		
   	for (i=1;i<argc;i++) if (argv[i][0]=='-') 
		switch (argv[i][1]) {
			case 'm': missingflag=TRUE;
					missingval=(double)atof(argv[++i]);
			break;
    			}	

	read_header(&hd);
     val=0;
	count=1;
	if (findparam(&hd,"Irregular") != NULLPAR)
		getparam(&hd,"Irregular",PFBYTE,&count,&val);
	if (val == 1)
		irregular = TRUE;
			
	frames=hd.num_frame;
	format=hd.pixel_format;
	Ndata=hd.cols*hd.rows;
	write_header(&hd);

	v=dvector(Ndata);
	for (f=0;f<frames;f++){
		fread_to_dvec(stdin,v,Ndata,format);
		if ((!irregular)||(f>1)){	
			mean=var=0.0;
			existpoints=0;
			for (i=0;i<Ndata;i++)
			if ((!missingflag)||(v[i]!=missingval)){ 
				mean+=v[i];
				existpoints++;
				}
			mean=mean/existpoints;

			for (i=0;i<Ndata;i++) 
			if ((!missingflag)||(v[i]!=missingval))
				var+=(v[i]-mean)*(v[i]-mean);
/*			var=var/(existpoints-1); */
			var=var/existpoints; 
			spred=sqrt(var);
			for (i=0;i<Ndata;i++) 
			if ((!missingflag)||(v[i]!=missingval))
				v[i]=(v[i]-mean)/spred;	
			}
		fwrite_from_dvec(stdout,v,Ndata,format);
		}
}
