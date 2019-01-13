/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_noise.c - simulate a noisy communication channel
 *
 * This program simulates a noisy communication channel by flipping bits in
 * a byte-formatted image with probability p.  The program works by computing
 * the number of bits until the next bit to be flipped.  In order to have this
 * work properly across multiple frames, the routine returns the number of
 * bits until the next flip after the end of the input image, which should
 * be supplied as the count argument for the subsequent frame.  The user also
 * supplies the number of effective bits per pixel.  Only this number of
 * low-order bits are subject to flipping.
 *
 * pixel formats: BYTE
 *
 * Yoav Cohen 3/15/82
 * sped-up: Mike Landy 9/18/88
 * HIPS 2 - msl - 8/5/91
 */

#include <hipl_format.h>
#include <math.h>

#define DEN 2147483648.0

int h_noise(hdi,hdo,p,counter,bpp)

struct header *hdi,*hdo;
double p;
int *counter,bpp;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_noise_b(hdi,hdo,p,counter,bpp));
	default:	return(perr(HE_FMTSUBR,"h_noise",
				hformatname(hdi->pixel_format)));
	}
}

int h_noise_b(hdi,hdo,p,counter,bpp)

struct header *hdi,*hdo;
double p;
int *counter,bpp;

{
	return(h_noise_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
	    hdi->ocols,hdo->ocols,p,counter,bpp));
}

int h_noise_B(imagei,imageo,nr,nc,nlpi,nlpo,p,counter,bpp)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo,*counter,bpp;
double p;

{
	register byte *ip,*op;
	register int i,j,count;
	int nexi,nexo,bit;
	double q,den;

	q = log(1.0-p); 
	den = log(DEN);
	count = *counter;
	if (count < 0) {
		do bit = H__RANDOM(); while (bit == 0);
		count = ceil((log((double)bit)-den)/q);
	}
	ip = imagei;
	op = imageo;
	nexi = nlpi-nc;
	nexo = nlpo-nc;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*op = *ip;
			while (count<=bpp) {
				bit = (1<<(count-1));
				*op ^= bit;
				do bit = H__RANDOM(); while (bit == 0);
				count=count+ceil((log((double)bit)-den)/q);
			}
			count = count - bpp;
			op++; ip++;
		}
		ip += nexi;
		op += nexo;
	}
	*counter = count;
	return(HIPS_OK);
}
