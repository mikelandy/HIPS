/*  IMSOR 1991 ,  Rasmus Larsen                            
 *                                                                  
 *
 * compile:  cc -o bil bil.c  -lhips 
 *
 *
 *                                                                        */
			      
#include <stdio.h>
#include <errno.h>
#include <hipl_format.h>
 
#ifndef SEEK_SET
#define	SEEK_SET 0
#endif

int types[] = {PFMSBF,PFLSBF,PFSBYTE,PFBYTE,PFSHORT,PFUSHORT,PFINT,PFUINT,
		PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,LASTTYPE};
static Flag_Format flagfmt[] = { LASTFLAG };
long long lseek();


int main(argc,argv)
int argc;
char *argv[];
{
Filename	filename;
FILE		*fp;
int		i,j;
int		piped;
int		rw,cl,nfr,rwsize,frsize;
struct header	hd;
long int	imagestart;
byte		*image;
int		nsize;

Progname = strsave(*argv);
parseargs(argc,argv,flagfmt,FFONE,&filename);
fp = hfopenr(filename);
fread_hdr_cpf(fp,&hd,types,filename);
clearroi(&hd);

if(findparam(&hd,"Interleaving") == NULLPAR && findparam(&hd,"bil") == NULLPAR){
	setparam(&hd,"bil",PFBYTE,1,1); 
	lseek(0,0L,1);
	piped = errno != 0;
	if (piped) {
		fprintf(stderr,"%s: Piping is not allowed\n",Progname);
		exit(1);
	}
	write_headeru(&hd,argc,argv);
	rw = hd.orows;
	cl = hd.ocols;
	nfr = hd.num_frame;
	nsize = rw * cl;

	imagestart = ftell(fp);
	rwsize = cl * hd.sizepix;
	frsize = nsize * hd.sizepix;
	image = hmalloc(rwsize);
	for (i=0;i<rw;i++) for (j=0;j<nfr;j++) {
		fseek(fp,(long) imagestart + j*frsize + i*rwsize,SEEK_SET);
		if (fread(image,rwsize,1,fp) != 1) {
			fprintf(stderr,"%s: Error during read\n",Progname);
			exit(1);
		}
		if (fwrite(image,rwsize,1,stdout) != 1) {
			fprintf(stderr,"%s: Error during write\n",Progname);
			exit(1);
		}
	}
}
else {
	if(findparam(&hd,"Interleaving") != NULLPAR)
		clearparam(&hd,"Interleaving");
	if(findparam(&hd,"bil") != NULLPAR) clearparam(&hd,"bil");
	lseek(0,0L,1);
	piped = errno != 0;
	if (piped) {
		fprintf(stderr,"%s: Piping is not allowed\n",Progname);
		exit(1);
	}
	write_headeru(&hd,argc,argv);
	rw = hd.orows;
	cl = hd.ocols;
	nfr = hd.num_frame;

	imagestart = ftell(fp);
	rwsize = cl * hd.sizepix;
	image = hmalloc(rwsize);
	for (j=0;j<nfr;j++) for (i=0;i<rw;i++) {
		fseek(fp,(long) imagestart + (j + i*nfr)*rwsize,SEEK_SET);
		if (fread(image,rwsize,1,fp) != 1) {
			fprintf(stderr,"%s: Error during read\n",Progname);
			exit(1);
		}
		if (fwrite(image,rwsize,1,stdout) != 1) {
			fprintf(stderr,"%s: Error during write\n",Progname);
			exit(1);
		}
	}
}
}
