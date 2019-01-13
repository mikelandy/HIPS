#include <stdio.h>
#include <hipl_format.h>
#include "util.h"
#define maxval(x,y) ((x>y) ? x : y)
#define minval(x,y) ((x<y) ? x : y)

long long lseek();

int main(argc,argv)
int argc;
char *argv[];
{
	struct header hd;
	int		i, f, ndata, frames, informat, load;
	float	smallest, greatest, step, zero;
	Fvector	data;
	Bvector	out;

	if (argc==2 && (!strcmp(argv[1],"-load")))
		load=1;
	else
		load=0;
	read_header(&hd);
	ndata=(load) ? hd.ocols*hd.orows*hd.num_frame : hd.ocols*hd.orows;
	frames=(load) ? 1 : hd.num_frame;
	informat=hd.pixel_format;
	data=fvector(ndata);
	out=bvector(ndata);

	greatest=-BIG;
	smallest=BIG;
	for (f=0;f<frames;f++){
		fread_to_fvec(stdin,data,ndata,informat);
		for (i=0;i<ndata;i++){
			greatest=maxval(greatest,data[i]);
			smallest=minval(smallest,data[i]);
			}
		}
	step=(greatest-smallest)/255;
	zero=smallest-step/2;
	hd.pixel_format=PFBYTE;
	update_header(&hd,argc,argv);
	write_header(&hd);

	if (!load){
		lseek(0,0L,0);
		read_header(&hd);
		for (f=0;f<hd.num_frame;f++){
			fread_to_fvec(stdin,data,ndata,informat);
			for (i=0;i<ndata;i++)
				out[i]=(data[i]-zero)/step;
			fwrite_vec(stdout,out,ndata,sizeof(byte));
			}
		}
	else{
		for (i=0;i<ndata;i++)
			out[i]=(data[i]-zero)/step;
		fwrite_vec(stdout,out,ndata,sizeof(byte));
		}
	return(0);
}

