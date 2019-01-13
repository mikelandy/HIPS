 /*
 *	PROGRAM
 *		pixshift
 *
 *	PURPOSE
 *		to shift an image vertically and/or horizontally by a 
 *		variable number of pixels.
 *
 *	SYNTAX
 *		pixshift [<horiz> <vert>] [-background]
 */
#include <stdio.h>
#include <hipl_format.h>

#define TRUE 1
#define FALSE 0

int main(argc, argv)
	int argc;
	char **argv;
{
	struct header hd;
	register int j, rt_flg;
	int nrows, ncols, nsize, nbytes, i, n;
	int horiz, vert, up_flg, h_flg, v_flg;
	unsigned char bkgnd;
	char *ibuf, *obuf;
	
	Progname = strsave(*argv);
	read_header(&hd);
	update_header(&hd, argc, argv);
	write_header(&hd);

	if (argc == 2 || argc >= 5)
		perr(HE_MSG,"syntax: pixshift [<horiz> <vert>] [-bckgrnd]");

	if (hd.pixel_format != PFBYTE && hd.pixel_format != PFSHORT &&
		hd.pixel_format != PFINT && hd.pixel_format != PFFLOAT)
			perr(HE_MSG,
				"image must be float, int, short or byte format");

	nrows = hd.orows;
	ncols = hd.ocols;
	nsize = nrows * ncols;
	switch (hd.pixel_format) {
	case PFBYTE:
		nbytes = sizeof(char);
		break;
	case PFSHORT:
		nbytes = sizeof(short);
		break;
	case PFINT:
		nbytes = sizeof(int);
		break;
	case PFFLOAT:
		nbytes = sizeof(float);
		break;
	}
	ibuf = (char *) halloc(nsize * nbytes,1);
	obuf = (char *) halloc(nsize * nbytes,1);
	horiz = vert = 1;
	up_flg = rt_flg = h_flg = v_flg = TRUE;

	bkgnd = 0;
	if (argc == 4) bkgnd = 0 - atoi(argv[3]);
	if (argc >= 3) {
		horiz = atoi(argv[1]);
		vert = atoi(argv[2]);
		if (horiz < 0) {
			horiz = 0 - horiz;
			rt_flg = FALSE;
		}
		if (vert < 0) {
			vert = 0 - vert;
			up_flg = FALSE;
		}
		if (horiz == 0) h_flg = FALSE;
		if (vert == 0) v_flg = FALSE;
	}
	if (argc == 2) bkgnd = 0 - atoi(argv[1]);

    for (n=0; n<hd.num_frame; n++) {
	if (fread(ibuf,nsize*nbytes,1,stdin) != 1)
		perr(HE_MSG,"read error");

	switch (hd.pixel_format) {
	case PFBYTE: {
		register char *pbi, *pbo;
		pbi = ibuf; pbo = obuf;
		if (v_flg && !up_flg)
			for (j=0; j<vert*ncols; j++,pbo++) *pbo = bkgnd;
		if (v_flg && up_flg) pbi += vert * ncols;
		for (i=0; i<nrows-vert; i++) {
			if (h_flg && !rt_flg) pbi += horiz;
			if (h_flg && rt_flg)
				for (j=0; j<horiz; j++,pbo++) *pbo = bkgnd;

			for (j=0; j<ncols-horiz; j++,pbi++,pbo++) *pbo = *pbi;

			if (h_flg && rt_flg) pbi += horiz;
			if (h_flg && !rt_flg)
				for (j=0; j<horiz; j++,pbo++) *pbo = bkgnd;
		}
		if (v_flg && up_flg)
			for (j=0; j<vert*ncols; j++,pbo++) *pbo = bkgnd;
		break; }
	case PFSHORT: {
		register short *psi, *pso;
		psi = (short *)ibuf; pso = (short *)obuf;
		if (v_flg && !up_flg)
			for (j=0; j<vert*ncols; j++,pso++) *pso = bkgnd;
		if (v_flg && up_flg) psi += vert * ncols;
		for (i=0; i<nrows-vert; i++) {
			if (h_flg && !rt_flg) psi += horiz;
			if (h_flg && rt_flg)
				for (j=0; j<horiz; j++,pso++) *pso = bkgnd;

			for (j=0; j<ncols-horiz; j++,psi++,pso++) *pso = *psi;

			if (h_flg && rt_flg) psi += horiz;
			if (h_flg && !rt_flg)
				for (j=0; j<horiz; j++,pso++) *pso = bkgnd;
		}
		if (v_flg && up_flg)
			for (j=0; j<vert*ncols; j++,pso++) *pso = bkgnd;
		break; }
	case PFINT: {
		register int *pii, *pio;
		pii = (int *)ibuf; pio = (int *)obuf;
		if (v_flg && !up_flg)
			for (j=0; j<vert*ncols; j++,pio++) *pio = bkgnd;
		if (v_flg && up_flg) pii += vert * ncols;
		for (i=0; i<nrows-vert; i++) {
			if (h_flg && !rt_flg) pii += horiz;
			if (h_flg && rt_flg)
				for (j=0; j<horiz; j++,pio++) *pio = bkgnd;

			for (j=0; j<ncols-horiz; j++,pii++,pio++) *pio = *pii;

			if (h_flg && rt_flg) pii += horiz;
			if (h_flg && !rt_flg)
				for (j=0; j<horiz; j++,pio++) *pio = bkgnd;
		}
		if (v_flg && up_flg)
			for (j=0; j<vert*ncols; j++,pio++) *pio = bkgnd;
		break; }
	case PFFLOAT: {
		register float *pfi, *pfo;
		pfi = (float *)ibuf; pfo = (float *)obuf;
		if (v_flg && !up_flg)
			for (j=0; j<vert*ncols; j++,pfo++) *pfo = bkgnd;
		if (v_flg && up_flg) pfi += vert * ncols;
		for (i=0; i<nrows-vert; i++) {
			if (h_flg && !rt_flg) pfi += horiz;
			if (h_flg && rt_flg)
				for (j=0; j<horiz; j++,pfo++) *pfo = bkgnd;

			for (j=0; j<ncols-horiz; j++,pfi++,pfo++) *pfo = *pfi;

			if (h_flg && rt_flg) pfi += horiz;
			if (h_flg && !rt_flg)
				for (j=0; j<horiz; j++,pfo++) *pfo = bkgnd;
		}
		if (v_flg && up_flg)
			for (j=0; j<vert*ncols; j++,pfo++) *pfo = bkgnd;
		break; }
	}

	if (fwrite(obuf,nsize*nbytes,1,stdout) != 1)
		perr(HE_MSG,"write error");
    }
}
