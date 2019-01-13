/*
 *	PROGRAM
 *		lutmap
 *
 *	PURPOSE
 *		to map an image file with a lut file
 *
 *	SYNTAX
 *		lutmap <lut file> < in seq   > out seq
 *	the default directory for lut file is /usr/spool/images/lut
 *	but it is only added when "lut file" does not begin with
 *	directory information
 */
#include <hipl_format.h>
#include <fcntl.h>
#include <stdio.h>

#define COLMAX 512

int main(argc, argv)
	int argc;
	char **argv;
{
	struct header hd;
	int rows, cols, nframs, lutsiz, imgsiz;
	FILE *mfp;
	unsigned char lutbuf[3][256];
	unsigned char *inbuf, *outbuf;
	char lt_name[80],tmp[100];
	int n, k;
	register unsigned char *pin, *pout;
	register int i, j;


	Progname = strsave(*argv);
	if (argc != 2)
		perr(HE_MSG,"Syntax: lutmap <lutfile>");

	switch (*argv[1]) {
	case '.':
	case '/':
		strcpy(lt_name,argv[1]);
		break;
	default:
		strcpy(lt_name,"/usr/spool/images/lut/");
		strcat(lt_name,argv[1]);
		break;
	}

	if ((mfp = fopen(lt_name,"r")) == NULL) {
		sprintf(tmp,"could not open lut file %s",argv[1]);
		perr(HE_MSG,tmp);
	}
	fread_header(mfp,&hd,lt_name);
	if (hd.pixel_format != PFLUT) {
		sprintf(tmp,"file %s not a lut data file",argv[1]);
		perr(HE_MSG,tmp);
	}
	lutsiz = hd.orows;
	if (fread(lutbuf,lutsiz*256,1,mfp) != 1)
		perr(HE_MSG,"error reading lut data");
	fclose(mfp);

	read_header(&hd);
	if (hd.pixel_format != PFBYTE)
		perr(HE_MSG,"input image must be in byte format");
	rows = hd.orows;
	cols = hd.ocols;
	nframs = hd.num_frame;
	imgsiz = rows*cols;

	hd.num_frame *= lutsiz;
	update_header(&hd, argc, argv);
	write_header(&hd);

	inbuf = (unsigned char *) halloc(imgsiz,1);
	outbuf = (unsigned char *) halloc(imgsiz,1);

	for (n=0; n<nframs; n++) {
		if (fread(inbuf,imgsiz,1,stdin) != 1)
			perr(HE_MSG,"read error");
		for (k=0; k<lutsiz; k++) {
	/* This switch of index is because the Itec luts are in the order
		blue - green - red, and wframe expects red - green - blue */
			i = (lutsiz - 1) - k;
			pin = inbuf; pout = outbuf;
			for (j=0; j<imgsiz; j++, pin++, pout++) {
				*pout = lutbuf[i][*pin];
			}
			if (fwrite(outbuf,imgsiz,1,stdout) != 1)
				perr(HE_MSG,"write error");
		}
	}
}
