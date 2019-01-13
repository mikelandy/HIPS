/* fvaxtosun.c - converts single precision numbers from vax format to sun
 * format. It should work on either a sun or a vax, since it just uses byte
 * manipulations.
 *
 * usage: fvaxtosun <floatin.hpl > floatout.hpl
 *
 * to load: cc -o fvaxtosun fvaxtosun.c -O -lhips
 *
 * Ramin Samadani - 24 June 88
 */

#include <stdio.h>
#include <hipl_format.h>
char *prog;

int main(argc,argv)
	int argc;
	char *argv[];
{
/* VARIABLES */
    float *ifr,*ofr;
    unsigned char *bifr,*bofr;
    int i,npix;
    struct header hd;

    Progname = prog = strsave(*argv);

/*
 * check passed parameters 
 */

    if (argc > 1) {
	fprintf(stderr,"use: %s <filein  >fileout\n",prog);
	exit(1);
    }
    read_header(&hd);
    if (hd.pixel_format != PFFLOAT) {
	fprintf(stderr,"%s: input file must be float\n",prog);
	exit(1);
    }
    if ((ifr = (float *) calloc(hd.orows*hd.ocols,sizeof(float))) == NULL){
	fprintf(stderr,"%s: can't allocate input frame\n",prog);
	exit(1);
    }
    if ((ofr = (float *) calloc(hd.orows*hd.ocols,sizeof(float))) == NULL){
	fprintf(stderr,"%s: can't allocate output frame\n",prog);
	exit(1);
    }
    if (fread(ifr,hd.orows*hd.ocols*sizeof(float),1,stdin) != 1) {
	fprintf(stderr,"%s: can't read frame\n",prog);
	exit(1);
    }
    update_header(&hd,argc,argv);
    write_header(&hd);

/* MAIN PROCESSING */
    npix = hd.orows*hd.ocols;
    bifr = (unsigned char *) ifr;
    bofr = (unsigned char *) ofr;
    for (i = 0;i < npix; i++) {
	bofr[1] = (bifr[0])&0377;
	bofr[3] = (bifr[2])&0377;
	bofr[0] = ((bifr[1])&0377) - 1;
	bofr[2] = (bifr[3])&0377;

	bifr = bifr + 4; bofr = bofr + 4;
    }
/* 
 * write the frame 
 */
    if (fwrite(ofr,hd.orows*hd.ocols*sizeof(float),1,stdout) != 1) {
	fprintf(stderr,"%s: can't write frame\n",prog);
	exit(1);
    }
}
