/*	Copyright (c) 1982 Michael Landy, Yoav Cohen, and George Sperling

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * avefilt.c - apply a averaging filter to an image
 *
 * usage:	avefilt [size] <iseq >oseq
 *
 * where size is the length of the side of the neighborhood in which the
 * average is computed. Size defaults to 3.
 *
 * to load:	cc -o avefilt avefilt.c -lhips
 *
 * Mike Landy - 5/28/82
 * Chuck Carman - 4/2/87 modified from median
 */

#include <stdio.h>
#include <hipl_format.h>

int main(argc,argv)

char *argv[] ;

{
	int	f,size,fr,r,c,i,j,sizesq,ir,ic;
	int	minus,plus,ii,jj,top,bot,left,right;
	int	count, sum;
	struct	header hd;
	char	*ifr,*ofr,*ip,*op,*nnp;

	Progname = strsave(*argv);
	if (argv[argc-1][0] == '-')
		argc--;
	if (argc > 1)
		size = atoi(argv[1]);
	else
		size = 3;
	if (size < 1 || size > 10)
		perr(HE_MSG,"unreasonable size specified");
	sizesq = size*size;
	plus = size / 2;
	minus = plus - size + 1;
	read_header (&hd) ;
	if(hd.pixel_format != PFBYTE)
		perr(HE_MSG,"avefilt: pixel format must be byte");
	r = hd.orows ; c = hd.ocols ;
	fr = hd.num_frame;
	top = -minus;
	bot = r - plus;
	left = -minus;
	right = c - plus;
	update_header (&hd,argc,argv) ;
	write_header (&hd) ;
	if ((ifr = (char *) calloc(r*c,sizeof (char))) == 0 ||
	    (ofr = (char *) calloc(r*c,sizeof (char))) == 0)
		perr(HE_MSG,"can't allocate core");
	for (f=0;f<fr;f++) {
		if (fread(ifr,r*c*sizeof(char),1,stdin) != 1)
			perr(HE_MSG,"error during read");
		ip = ifr;
		op = ofr;
		for (i=0;i<r;i++) {
			for (j=0;j<c;j++) {
				sum = 0; count = 0;
				if (i<top || i>=bot || j<left || j>=right) {
					for (ii=minus;ii<=plus;ii++)
						for (jj=minus;jj<=plus;jj++) {
						    ir = i + ii;
						    ic = j + jj;
						    ir = ir<0?0:(ir>=r)?r-1:ir;
						    ic = ic<0?0:(ic>=c)?c-1:ic;
						    sum += ifr[ir*c+ic] & 0377;
						    count++;
						}
				}
				else {
					nnp = ip + minus*c + minus;
					for (ii=minus;ii<=plus;ii++) {
						for (jj=minus;jj<=plus;jj++) {
							sum += *nnp++ & 0377;
							count++;
						}
						nnp += c - size;
					}
				}
				ip++;
				*op++ = sum / count;
			}
		}
		if (fwrite(ofr,r*c*sizeof(char),1,stdout) != 1)
			perr(HE_MSG,"error during write");
	}
	return(0);
}
