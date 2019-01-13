/*	Copyright (c) 1990 Jens Michael Carstensen, IMSOR 

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 *  tga2hips.c - converts an image file from tga-format to hips-format 
 *
 *  Usage: tga2hips <tga-file >outseq
 *
 *  to load: cc -o tga2hips tga2hips.c -lhips 
 *
 */

#define  Byte     unsigned char

#include <hipl_format.h>
#include <stdio.h>

long read();

int main(argc,argv)
int argc;
char **argv;
{
        struct header hd;
        int    	i,j,k,nf,npix;
  	char  	header[256];
 	Byte   	*ifr,*dfr,*ofr,*cmap;
	int	id_len,cmap_type,image_type;
	int	cmap_index,cmap_len,cmap_depth,cmap_n;
	int	fcol,frow,nrows,ncols,pix_depth,pixlen;
	int	desc_bits,desc_order,desc_zero;

	Progname = strsave(*argv);

  	if ((read(0,header,18)) != 18)
		perr(HE_MSG,"error during read");

	id_len         = header[0];
	cmap_type      = header[1];
	image_type     = header[2];
  	cmap_index     = 256*(Byte)header[4]+(Byte)header[3];
  	cmap_len       = 256*(Byte)header[6]+(Byte)header[5];
	cmap_depth     = header[7];
  	fcol           = 256*(Byte)header[9]+(Byte)header[8];
 	frow           = 256*(Byte)header[11]+(Byte)header[10];
  	ncols          = 256*(Byte)header[13]+(Byte)header[12];
 	nrows          = 256*(Byte)header[15]+(Byte)header[14];
	pix_depth      = header[16];    
	desc_bits      = header[17] & 0x0f;
	desc_order     = header[17] & 0x30;
	desc_zero      = header[17] & 0xc0;

	switch (cmap_type) {
	case 0:
		fprintf(stderr,"No colormap included\n");
		break;
	case 1:
		fprintf(stderr,"Colormap included\n");
		break;
	default:
		perr(HE_MSG,"Bad cmap_type\n");
	}

	switch (image_type) {
	case 0:
		fprintf(stderr,"No image data included\n");
		break;
	case 1:
		fprintf(stderr,"Uncompressed, colormapped image\n");
		break;
	case 2:
		fprintf(stderr,"Uncompressed, true-color image\n");
		break;
	case 3:
		fprintf(stderr,"Uncompressed, black/white true-color image\n");
		break;
	case 9:
		fprintf(stderr,"Run-length encoded, colormapped image\n");
		break;
	case 10:
		fprintf(stderr,"Run-length encoded, true-color image\n");
		break;
	case 11:
		fprintf(stderr,"Run-length encoded, black/white true-color image\n");
		break;
	default:
		perr(HE_MSG,"Bad image_type\n");
	}

	fprintf(stderr,"Cmap start  : %d\n",cmap_index);
	fprintf(stderr,"Cmap length : %d\n",cmap_len);
	fprintf(stderr,"Cmap depth  : %d\n",cmap_depth);

	fprintf(stderr,"X-origin    : %d\n",fcol);
	fprintf(stderr,"Y-origin    : %d\n",frow);
	fprintf(stderr,"X-total     : %d\n",ncols);
	fprintf(stderr,"Y-total     : %d\n",nrows);
	fprintf(stderr,"Pixel depth : %d\n",pix_depth);

  	if ((read(0,header,id_len)) != id_len)
		perr(HE_MSG,"error during read");

	nf = pixlen = pix_depth/8;
	if (nf == 4) nf--;
	init_header(&hd,"","",nf,"",nrows,ncols,PFBYTE,nf,"");
	update_header(&hd,argc,argv);
	write_header(&hd);
	npix = nrows * ncols;
	
	cmap_n = cmap_depth/8;
	cmap = (Byte *) halloc(cmap_len*cmap_n,sizeof(Byte));
 	if ((read(0,cmap,cmap_len*cmap_n)) != cmap_len*cmap_n)
		perr(HE_MSG,"error during read");
	/*
	for (i=0;i<cmap_len;i++) 
		fprintf(stderr,"%3d	%3d	%3d\n",
			cmap[3*i],cmap[3*i+1],cmap[3*i+2]);
			*/

	ifr = (Byte *) halloc(npix*pixlen,sizeof(Byte));
	if (image_type==2 || image_type==3) 
		dfr = (Byte *) halloc(npix,sizeof(Byte));
	ofr = (Byte *) halloc(npix,sizeof(Byte));
  	if ((read(0,ifr,npix*pixlen)) != npix*pixlen)
		perr(HE_MSG,"error during read");


	switch (image_type) {
	case 1:
		for (i=0;i<nrows;i++) for (j=0;j<ncols;j++) {
			ofr[i*ncols+j]=ifr[(nrows-i-1)*ncols+j];
		}
	    	if (fwrite(ofr,1,npix,stdout) != npix ) 
			perr(HE_MSG,"error during write");
		break;
	case 2:
		for (k=2;k>=0;k--) {
			j=k;
			for (i=0;i<npix;i++,j+=pixlen) dfr[i]=ifr[j]; 
			for (i=0;i<nrows;i++) for (j=0;j<ncols;j++) {
				ofr[i*ncols+j]=dfr[(nrows-i-1)*ncols+j];
			}
	    		if (fwrite(ofr,1,npix,stdout) != npix ) 
				perr(HE_MSG,"error during write");
		}
	    	break;
	case 3:
		for (i=0;i<nrows;i++) for (j=0;j<ncols;j++) {
			ofr[i*ncols+j]=ifr[(nrows-i-1)*ncols+j];
		}
	    	if (fwrite(ofr,1,npix,stdout) != npix ) 
			perr(HE_MSG,"error during write");
		break;
	default:
		perr(HE_MSG,"bad image type");
	    	break;
	}

        return(0);
}
