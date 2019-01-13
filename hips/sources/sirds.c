/*
 * sirds - create a single image random dot stereogram from a depth image
 *
 * usage:	sirds [-s stripwidth] [-d] [-f [size]]
 *				<depthimage >stereogramimage
 *
 * stripwidth defaults to 20.  The largest value in depthimage must be less
 * than the value of stripwidth.  -d specifies that the stereogram is set for
 * diverged eyes (otherwise it is for converged eyes).  In the convergent case
 * an extra strip of zero disparity is generated at the left of the SIRDS.
 * If the input image is in float format then output pixels are interpolated
 * resulting in more continuous depth values.  However, this also results in
 * a successive blurring of the image from left to right.  The -f flag
 * specifies that fixation marks should be provided above and below the image
 * (the size defaults to 10 pixels).
 *
 * pixel formats handled directly: BYTE, FLOAT
 *
 * to load:	cc -o sirds sirds.c -lhipsh -lhips -lm
 *
 * Mike Landy - 8/12/93
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"s",{LASTFLAG},1,{{PTINT,"20","stripwidth"},LASTPARAMETER}},
	{"d",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"f",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},{PTINT,"10","size"},
		LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo;
	int method,fr,f,sw,nr,nc,r,c,p1,p2,delta,fsize,i;
	byte *ip,*op;
	float *ipf,alpha;
	Filename filename;
	FILE *fp;
	h_boolean dflag,fflag;
	Pixelval val;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&sw,&dflag,&fflag,&fsize,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	nr = hdp.orows;
	nc = hdp.ocols;
	setsize(&hdo,fflag ? (nr + 6*fsize) : nr,nc + (dflag ? sw : (2*sw)));
	setformat(&hdo,PFBYTE);
	alloc_image(&hdo);
	write_headeru(&hdo,argc,argv);
	if (fflag) {
		val.v_byte = 255;
		setroi(&hdo,0,0,3*fsize,hdo.ocols);
		h_setimage(&hdo,&val);
		setroi(&hdo,nr+3*fsize,0,3*fsize,hdo.ocols);
		h_setimage(&hdo,&val);
		i = (hdo.ocols - sw)/2;
		val.v_byte = 0;
		setroi(&hdo,fsize,i,fsize,fsize);
		h_setimage(&hdo,&val);
		setroi(&hdo,fsize,i+sw,fsize,fsize);
		h_setimage(&hdo,&val);
		setroi(&hdo,nr+4*fsize,i,fsize,fsize);
		h_setimage(&hdo,&val);
		setroi(&hdo,nr+4*fsize,i+sw,fsize,fsize);
		h_setimage(&hdo,&val);
	}
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		if (hdp.pixel_format == PFBYTE) {
			ip = hdp.image;
			op = hdo.image;
			if (fflag)
				op += 3*fsize*hdo.ocols;
			for (r=0;r<nr;r++) {
				for (c=0;c<sw;c++)
					*op++ = H__RANDOM() & 255;
				if (!dflag) {
					for (c=0;c<sw;c++) {
						*op = *(op-sw);
						op++;
					}
				}
				if (dflag) {
					for (c=0;c<nc;c++) {
						*op = *(op - sw + *ip++); 
						op++;
					}
				}
				else {
					for (c=0;c<nc;c++) {
						*op = *(op - sw - *ip++); 
						op++;
					}
				}
			}
		}
		else {
			ipf = (float *) hdp.image;
			op = hdo.image;
			if (fflag)
				op += 3*fsize*hdo.ocols;
			for (r=0;r<nr;r++) {
				for (c=0;c<sw;c++)
					*op++ = H__RANDOM() & 255;
				if (!dflag) {
					for (c=0;c<sw;c++) {
						*op = *(op-sw);
						op++;
					}
				}
				if (dflag) {
					for (c=0;c<nc;c++) {
						delta = *ipf;
						alpha = *ipf++ - delta;
						p1 = *(op - sw + delta);
						p2 = *(op - sw + delta + 1);
						*op++ = (1-alpha)*p1 + alpha*p2;
					}
				}
				else {
					for (c=0;c<nc;c++) {
						delta = *ipf;
						alpha = *ipf++ - delta;
						p1 = *(op - sw - delta);
						p2 = *(op - sw - delta - 1);
						*op++ = (1-alpha)*p1 + alpha*p2;
					}
				}
			}
		}
		write_image(&hdo,f);
	}
	return(0);
}
