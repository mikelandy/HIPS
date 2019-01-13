#include <hipl_format.h>
#include <stdio.h>
#include <sys/types.h>
#include <pixrect/pixrect.h>
#include <pixrect/pr_io.h>
#include "hipsview.h" 
#include <rasterfile.h>

/*typedef struct {
	int type;
	int length;
	unsigned char *map[3];
} colormap_t; */

readim_file(filenm, buf1P, buf2P, buf4P)
char	 *filenm;
IMAGEptr *buf1P, *buf2P, *buf4P;
{
	int	 nbytes, nxXny, tag;
	FILE *fp;
	IMAGEptr *bufP, new_imP;
	struct rasterfile ras_hd;

	if((fp = fopen(filenm, "r")) == NULL) {
		perror("\nreadim_file ");
		fprintf(stderr, " Filename: %s\n", filenm);
		return(0);
	}
/*fprintf(stderr, "(RASMAGIC>>16) = %x", RAS_MAGIC>>16);
*/
	fread_header(fp,&hd,filenm);
	update_header(&hd,argcc,argvv);
	hdtype = hd.pixel_format;
	nu = hd.ocols;
	nv = hd.orows;
	nxXny = nu * nv;
	if (hdtype != PFBYTE && hdtype != PFSHORT && hdtype != PFINT
	    && hdtype != PFFLOAT) {
		fprintf(stderr,
			"Error: format must by byte, short, int, or float\n");
		fclose(fp);
		return(0);
	}

	switch(hdtype) {
	case PFBYTE:
		bufP = buf1P;
		nbytes = nxXny;
		tag = 1;
		break;
	case PFSHORT:
		bufP = buf2P;
		nbytes = 2 * nxXny;
		tag = 2;
		break;
	case PFINT:
	case PFFLOAT:
		bufP = buf4P;
		nbytes = 4 * nxXny;
		tag = 4;
		break;
	}

	if(bufP==0)
		return(-1);

	if(*bufP==0){
		new_imP=(unsigned char *)malloc(nbytes);
	}else{
		new_imP=(unsigned char *)realloc(*bufP, nbytes);
	}

	if(new_imP!=0){
		fread(new_imP, nbytes,1,fp);
	}else{
		perror("\nreadim_file ");
		tag = 0;
	}

	*bufP = new_imP;
	fclose(fp);
	return(tag);
}

IMAGEptr *
im_alloc(imageP, size)
IMAGEptr *imageP;
{
	if(imageP==0)
		imageP = (unsigned char **)malloc(size);
	else		/* reallocate in case size changes */
		imageP = (unsigned char **)realloc(imageP, size);
	
	if(imageP==0){
		perror("\nrealloc ");
		printf("ERROR: can't allocate imageP\n");
		return;
	}
	return(imageP);
}
