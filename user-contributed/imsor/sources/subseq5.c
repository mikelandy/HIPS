/*	Copyright (c) 1989 Michael Landy

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/* subseq5.c - to extract a subsequence of 5 frames
 *
 * usage:	subseq5 [-f frame1 frame2 frame3]
 *
 * defaults:	frame1=0 (first  frame)
 *		frame2=1 (second frame)
 *		frame3=2 (third  frame)
 *
 * usage:	subseq5 [-f frame1 frame2 frame3 frame4 frame5]
 *
 * defaults:	frame1=0 (first  frame)
 *		frame2=1 (second frame)
 *		frame3=2 (third  frame)
 *		frame4=3 (fourth  frame)
 *		frame5=4 (fifth  frame)
 *
 * From a disk file sequence frames can be read in any order.
 * From a piped sequence frames must be read sequentially.
 *
 * to load:	cc -o subseq5 subseq5.c -lhips
 *
 * Y. Cohen 3/1/82
 * modified for lseek on disk files by Mike Landy 7/3/84
 * modified for short/int/float/complex - msl 3/22/88
 * modified for double/double complex - msl 2/4/89
 * modified for pyramids - msl 3/7/89
 * altered to extract 3 frames (for RGB display) by
 *    Allan Aasbjerg Nielsen
 *    IMSOR - Institute of Mathetical Mathematics and Operations Research
 *    Technical University of Denmark
 *    19.06.91
 * altered to extract 5 frames by
 *    Allan Aasbjerg Nielsen
 *    IMSOR - Institute of Mathetical Mathematics and Operations Research
 *    Technical University of Denmark
 *    16.09.91
 * HIPS-2 10.12.92 /AA (pread -> fread, lseek -> fseek)
 */

#include <stdio.h>
#include <hipl_format.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

extern int errno;
long long lseek();

char usage[]="Usage: subseq5 [-f frame1 frame2 frame3 [frame4 frame5]]";

int main(argc,argv)
int	argc;
char	*argv[];
{
	struct header hd;
	int i,j,fr[5],dfr,sizepix,numbytes,oldfn,piped,toplev;
	char *frame;
	struct stat buf;

	Progname = strsave(*argv);

	fr[0]=0;
	fr[1]=1;
	fr[2]=2;
	fr[3]=3;
	fr[4]=4;

	for (i=1;i<argc;i++) {
		if (argv[i][0] == '-')
		switch (argv[i][1]) {
		case 'f': if (argc>5) {
				fr[0]=atoi(argv[++i]);
				fr[1]=atoi(argv[++i]);
				fr[2]=atoi(argv[++i]);
				fr[3]=atoi(argv[++i]);
				fr[4]=atoi(argv[++i]);
			  }
			  else {
				fr[2]=atoi(argv[++i]);
				fr[3]=atoi(argv[++i]);
				fr[4]=atoi(argv[++i]);
			  }
			  break;
		case 'U':
		default:  perr(HE_MSG,usage);
			  break;
		}
	}

	fprintf(stderr,"subseq5: get frames %d %d %d %d %d\n",fr[0],fr[1],fr[2],fr[3],fr[4]);

	/* inseq piped? */
	lseek(0,0L,1);
	piped = errno!=0;
	if (!piped) fstat(0,&buf);

	read_header(&hd);
	if (hd.pixel_format == PFBYTE || hd.pixel_format == PFLUT)
		sizepix = sizeof(char);
	else if (hd.pixel_format == PFSHORT)
		sizepix = sizeof(short);
	else if (hd.pixel_format == PFINT)
		sizepix = sizeof(int);
	else if (hd.pixel_format == PFFLOAT)
		sizepix = sizeof(float);
	else if (hd.pixel_format == PFCOMPLEX)
		sizepix = 2*sizeof(float);
	else if (hd.pixel_format == PFDOUBLE)
		sizepix = sizeof(double);
	else if (hd.pixel_format == PFDBLCOM)
		sizepix = 2*sizeof(double);
	else if (hd.pixel_format == PFINTPYR)
		sizepix = sizeof(int);
	else if (hd.pixel_format == PFFLOATPYR)
		sizepix = sizeof(float);
	else
		perr(HE_MSG,"format must be byte, short, int, float, complex,\n\
double, double complex, integer pyramid, or floating pyramid");
	if (hd.pixel_format == PFINTPYR || hd.pixel_format == PFFLOATPYR) {
		perr(HE_MSG,"PFINTPYR and PFFLOATPYR not supported");}
	/*
	{
		if (pread(0,&toplev,sizeof(int)) != sizeof(int))
			perr(HE_MSG,"error reading number of pyramid levels");
		i = pyrnumpix(toplev,hd.orows,hd.ocols);
		numbytes = sizepix*i;
	}
	else if (hd.bits_per_pixel==1 && hd.bit_packing)
		numbytes = ((hd.ocols+7)/8)*hd.orows*sizeof(char);
	else
	*/
		numbytes = sizepix*hd.ocols*hd.orows;
	frame = (char *) halloc(numbytes,sizeof(char));
	oldfn=hd.num_frame;
	hd.num_frame=5;
	for (i=0;i<hd.num_frame;i++)
		if (oldfn<fr[i]) perr(HE_MSG,"frame %d not in sequence",fr[i]);
	update_header(&hd,argc,argv);
	write_header(&hd);
	/*
	if (hd.pixel_format == PFINTPYR || hd.pixel_format == PFFLOATPYR)
		if (write(1,&toplev,sizeof(int)) != sizeof(int))
			perr(HE_MSG,"error writing number of pyramid levels");
	*/

	/* inseq is piped or disk file S_IFCHR: sequential access */
	if (piped || (buf.st_mode&S_IFMT)==S_IFCHR) {
		/* fprintf(stderr,"piped or disk file S_IFCHR: sequential access\n"); */
		for (i=0;i<hd.num_frame;i++)
			if (i>0 && fr[i]<fr[i-1])
				perr(HE_MSG,"sequence must be read sequentially when piped or disk file S_IFCHR");
		for (i=0;i<oldfn;i++) {
			if(i>fr[4])
				perr(HE_MSG,"not that many frames in input sequence");
			if (i==fr[0] || i==fr[1] || i==fr[2] || i==fr[3] || i==fr[4]) {
				/* fprintf(stderr,"read frame %d\n",i); */
				if (fread(frame,sizeof(char),numbytes,stdin)
				    != numbytes)
				       perr(HE_MSG,"error during read of frame %d",i);
				if (fwrite(frame,sizeof(char),numbytes,stdout)
				    != numbytes)
				      perr(HE_MSG,"error during write of frame %d",i);
			}
			else {
				if (fread(frame,sizeof(char),numbytes,stdin)
				    != numbytes)
				       perr(HE_MSG,"error during read of frame %d",i);
			}
		}
	}
	/* inseq is disk file, no S_IFCHR: direct access */
	else {
		/* fprintf(stderr,"disk file, no S_IFCHR: direct access\n"); */
		if (fr[0]>0) fseek(stdin,fr[0]*numbytes*sizeof(char),1);
		for (i=0;i<hd.num_frame;i++) {
			if (i > 0) {
				if ((dfr = (fr[i]-fr[i-1])) > 0) {
					/* fprintf(stderr,"dfr = %d\n",dfr); */
					if (dfr != 1) fseek(stdin,(dfr-1)*numbytes*sizeof(char),1);
				}
				else {
					/* fprintf(stderr,"dfr = %d\n",dfr); */
					fseek(stdin,(dfr-1)*numbytes*sizeof(char),1);
				}
			}
			if (fread(frame,sizeof(char),numbytes,stdin)
			    != numbytes)
				perr(HE_MSG,"error during read of frame %d",i);
			if (fwrite(frame,sizeof(char),numbytes,stdout)
			    != numbytes)
				perr(HE_MSG,"error during write of frame %d",i);
		}
	}
	return(0);
}
