/*  IMSOR 1993 ,  Rasmus Larsen                            
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

void syntax();

int main(argc,argv)
int argc;
char *argv[];
{
FILE		*imfp;
int		i,k;
int		nrows,ncols,nbands,nsize,sizepix;
int		rows,cols,frow,fcol;
long		pos;
struct header	hd;
long int	imagestart;
byte		*line;
char		*filename = "<stdin>";
int		sflag = 0;
int		ssflag = 0;
int		pflag = 0;
int		ppflag = 0;



Progname = strsave(*argv);

for (i=1;i<argc;i++) {
	if (strncmp(argv[i], "-s", 2) == 0) {
		sflag = 1;
		if (i+1>=argc && strncmp(argv[i+1], "-", 1) == 0) ssflag = 1;
		else {
			rows = atoi(argv[++i]);
			if (i+1<argc && strncmp(argv[i+1], "-", 1) != 0)
				cols = atoi(argv[++i]);
			else cols = rows;
		}
		continue;
	}
	if (strncmp(argv[i], "-p", 2) == 0) {
		pflag = 1;
		if (i+1>=argc && strncmp(argv[i+1], "-", 1) == 0) ppflag = 1;
		else {
			frow = atoi(argv[++i]);
			if (i+1<argc && strncmp(argv[i+1], "-", 1) != 0)
			fcol = atoi(argv[++i]);
			else fcol = frow;
		}
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

imfp = stdin;
fread_header(imfp,&hd,filename);
nrows   = hd.orows;
ncols   = hd.ocols;
nbands  = hd.num_frame;
sizepix = hd.sizepix;
nsize   = nrows * ncols;

if (ssflag || !sflag) { rows = nrows/2; cols = ncols/2; }
if (ppflag || !pflag) { frow = (nrows - rows)/2; fcol = (ncols - cols)/2; }
if (frow+rows > nrows || fcol+cols > ncols) {
	fprintf(stderr,"%s: Error in specification of region of interest\n",Progname);
	exit(1);
}

clearroi(&hd);
hd.orows = rows;
hd.ocols = cols;
write_headeru(&hd,argc,argv);

imagestart = ftell(imfp);
line = (byte *) malloc(cols*sizepix);

for (k=0;k<nbands;k++) {
	for (i=frow;i<frow+rows;i++) {
		pos = imagestart+(k*nsize+i*ncols+fcol)*sizepix;
		fseek(imfp,(long) pos,SEEK_SET);
		if (fread(line,sizepix,cols,imfp) != cols) {
			fprintf(stderr,"%s: Error during read\n",Progname);
			exit(1);
		}
		if (fwrite(line,sizepix,cols,stdout) != cols) {
			fprintf(stderr,"%s: Error during write\n",Progname);
			exit(1);
		}
	}
}


return 0;
}

void syntax()
{
fprintf(stderr,"usage: %s\n",Progname);
fprintf(stderr,"\t\t[-s [rows<int> [cols<int>]]] [-p [frow<int> [fcol<int>]]]\n");
fprintf(stderr,"\t\t[-U]\n");
}
