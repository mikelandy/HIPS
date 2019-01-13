/*
 * scaleseq - scales a sequence (float format).  Output is in byte format.
 *
 * usage:	scaleseq [min max] < iseq > oseq
 *
 * to load:	cc -o scaleseq scaleseq.c -lhips
 *
 * Charlie Chubb 12/6/85
 */

#include <stdio.h>
#include <ctype.h>
#include <hipl_format.h>

int getpid();

int main(argc,argv)

char *argv[] ;

{
	h_boolean	FLOAT = FALSE;
	int	pixsz;
	int	min, max, f,fr,r,c,i,k, dif;
	struct	header hd;
	char	*bfr,*bp;
	float	fmin, fmax, *ffr,*fp, incr, fdif, ftmp;
	int	*ifr,*ip;
	char	STARTFLAG;
	char	tmpfile[50];
	char	command[55];
	long	pid;
	FILE	*Fp;

	Progname = strsave(*argv);
	read_header (&hd) ;
	if(hd.pixel_format != PFFLOAT && hd.pixel_format != PFINT) {
		fprintf(stderr,"scaleseq: pixel format must be real or int\n");
		exit(1) ;
	}
	if (argc == 1) {
		min = 0;
		max = 255;
	}
	else if (argc == 3) {
		min = atoi(argv[1]);
		max = atoi(argv[2]);
	}
	else {
		fprintf(stderr, "scaleseq: number of args must be 0 or 2.\n");
		exit(1);
	}
	if (hd.pixel_format == PFFLOAT)
		FLOAT = TRUE;
	update_header (&hd,argc,argv) ;
	hd.pixel_format = PFBYTE;
	write_header (&hd) ;

	pid = getpid();
	sprintf(tmpfile,"tmp%ld",pid);
	if ((Fp = fopen(tmpfile,"w")) == NULL) {
		fprintf(stderr,"scaleseq: can't open %s for writing\n",
								tmpfile);
		exit(1);
	}
	r = hd.orows ; c = hd.ocols ;
	fr = hd.num_frame;

/* find floating maximum and minimum */

	if (FLOAT) {
		if ((ffr = (float *) calloc(r*c,sizeof (float))) == 0) {
			fprintf(stderr,"scaleseq: can't allocate core\n");
			exit(1);
		}
		pixsz = sizeof(float);
	}
	else {
		if ((ifr = (int *) calloc(r*c,sizeof (int))) == 0) {
			fprintf(stderr,"scaleseq: can't allocate core\n");
			exit(1);
		}
		pixsz = sizeof(int);
	}
	for (STARTFLAG = 1, f=0;f<fr;f++) {
		if (FLOAT) {
			if (fread(ffr,pixsz*r*c,1,stdin)!=1) {
				fprintf(stderr,"scaleseq: error during read\n");
				exit(1);
			}

			if (STARTFLAG) {
				fmax = fmin = *ffr;
				STARTFLAG = 0;
			}
			for (fp = ffr, i = 0, k = c*r; i < k; i++, fp++) {
				if (*fp > fmax)
					fmax = *fp;
				else if (*fp < fmin)
					fmin = *fp;
			}
			if (fwrite(ffr,r*c*pixsz,1,Fp)!=1) {
				fprintf(stderr,"scaleseq: write error\n");
				exit(1);
			}
		}
		else /*INT*/ {
			if (fread(ifr,pixsz*r*c,1,stdin)!=1) {
				fprintf(stderr,"scaleseq: error during read\n");
				exit(1);
			}

			if (STARTFLAG) {
				fmax = fmin = (float)*ifr;
				STARTFLAG = 0;
			}
			for (ip = ifr, i = 0, k = c*r; i < k; i++, ip++) {
				if (*ip > fmax)
					fmax = (float)*ip;
				else if (*fp < fmin)
					fmin = (float)*ip;
			}
			if (fwrite(ifr,r*c*pixsz,1,Fp)!=1) {
				fprintf(stderr,"scaleseq: write error\n");
				exit(1);
			}
		}
	}
	fclose(Fp);
	if ((Fp = fopen(tmpfile,"r")) == NULL) {
		fprintf(stderr,"scaleseq: can't open %s for reading\n",
								tmpfile);
		exit(1);
	}
	fprintf(stderr,"\n	scaleseq: min = %f, max = %f\n",fmin, fmax);
	fflush(stderr);
/* create byte formatted, scaled sequence */

	fdif = (fmax - fmin > 0? fmax - fmin: 2);
	dif =  max - min;
	incr = ((float)(dif))/fdif;
	if ((bfr = (char *) calloc(r*c,sizeof (char))) == 0) {
		fprintf(stderr,"scaleseq: can't allocate core\n");
		exit(1);
	}
	for (f=0;f<fr;f++) {
		if (FLOAT) {
			if (fread(ffr,r*c*pixsz,1,Fp)!=1) {
				fprintf(stderr,"scaleseq: error during read\n");
				exit(1);
			}
			for (fp = ffr, bp = bfr, i=0, k=c*r; i < k;
							i++, fp++, bp++) {
				ftmp = min + (*fp - fmin)*incr;
				*bp = (char)(ftmp - (int)ftmp > .5 ?
							ftmp+1. : ftmp);
			}
		}
		else /* INT */ {
			if (fread(ifr,r*c*pixsz,1,Fp)!=1) {
				fprintf(stderr,"scaleseq: error during read\n");
				exit(1);
			}
			for (ip = ifr, bp = bfr, i=0, k=c*r; i < k;
							i++, ip++, bp++) {
				ftmp = min + ((float)(*ip) - fmin)*incr;
				*bp = (char)(ftmp - (int)ftmp > .5 ?
							ftmp+1. : ftmp);
			}
		}
		if (fwrite(bfr,r*c*sizeof(char),1,stdout)!=1) {
			fprintf(stderr,"scaleseq: error during write\n");
			exit(1);
		}
	}
	fclose(Fp);
	sprintf(command,"rm %s",tmpfile);
	system(command);
	return(0) ;
}
