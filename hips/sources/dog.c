/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * dog.c - filter an image by applying difference of Gaussians mask
 *
 * usage:	dog [-w esigma [masksize [ratio]]] [-p precision] [-i] [-m]
 *				[-g] <input_seq >out_seq
 *
 * dog convolves an image with a difference of Gaussians mask by performing
 * two separable convolutions in x and y with each mask.  Esigma is the
 * standard deviation of the excitatory Gaussian (in pixels), and defaults to
 * one. Masksize is the size of the mask used for the convolution (default: 7).
 * Ratio is the ratio between the standard deviations of the excitatory and
 * inhibitory Gaussian (default: 1.6).  If ratio is identically zero, then
 * only the excitatory Gaussian is applied.
 *
 * The Gaussian mask is computed by computing the values of the continuous
 * 1D Gaussian function at equally spaced points, and averaging the values
 * that fall within the region covered by each pixel.  The number of points
 * that contribute to each mask value's average is specified using -p, and
 * defaults to 1 (thus, the midpoint of the interval's Gaussian value is
 * used).  By default, dog operates on floating point images and produces
 * floating point output.  With -i, dog operates on integer images, produces
 * an approximate integer Gaussian mask (multiplying the floating point mask
 * values by 1000), and produces integer output.  If -m is specified, the 1D
 * gaussians are printed and the program exits.
 * If -g is specified, no input is read.  Instead,
 * the program generates an image of a single pixel impulse, and
 * convolves with that.  The image has rows and columns of a size that is
 * a power of 2 and sufficient to hold the entire impulse response.
 *
 * pixel formats handled directly: INT (with -i), FLOAT (otherwise)
 * output pixel format: same as input
 *
 * to load: cc -o dog dog.c -lhipsh -lhips -lm
 *
 * Yoav Cohen - 12/12/82
 * Hips 2 - msl - 7/16/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"w",{LASTFLAG},1,{{PTDOUBLE,"1","esigma"},{PTINT,"7","masksize"},
		{PTDOUBLE,"1.6","ratio"},LASTPARAMETER}},
	{"p",{LASTFLAG},1,{{PTINT,"1","precision"},LASTPARAMETER}},
	{"i",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"m",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"g",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int typesi[] = {PFINT,LASTTYPE};
int typesf[] = {PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdt1,hdt2;
	int method,fr,f,masksize,precision,*p,*imaske,*imaski,i;
	Filename filename;
	FILE *fp;
	h_boolean iflag,mflag,gflag,r0flag;
	double esigma,ratio,scalei,scalee,h_gaussmask();
	float *pf,*fmaske,*fmaski;
	Pixelval val;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&esigma,&masksize,&ratio,&precision,
		&iflag,&mflag,&gflag,FFONE,&filename);
	if (esigma<=0.0)
		perr(HE_MSG,"sigma must be positive");
	if (masksize<1)
		perr(HE_MSG,"masksize must be positive");
	if (ratio<0.0)
		perr(HE_MSG,"arg ratio must be zero or positive");
	if (ratio == 0.0)
		fprintf(stderr,"%s: blurring by a Gaussian mask\n",Progname);
	r0flag = (ratio == 0.);
	fmaske = (float *) memalloc(masksize,sizeof(float));
	if (!r0flag)
		fmaski = (float *) memalloc(masksize,sizeof(float));
	scalee = h_gaussmask(esigma,masksize,fmaske,precision);
	if (!r0flag)
		scalei = h_gaussmask(esigma*ratio,masksize,fmaski,precision);
	if (mflag) {
		fprintf(stderr,"%s:   size=%d, precision=%d\n",Progname,
			masksize,precision);
		fprintf(stderr,"Excitatory mask, esigma=%f, scalefactor=%f\n",
			(float) esigma,(float) scalee);
		for(i=0;i<masksize;i++)
			printf("    %f\n",fmaske[i]);
		if (!r0flag) {
			fprintf(stderr,
				"Inhibitory mask, esigma=%f, scalefactor=%f\n",
				(float) (esigma*ratio),(float) scalei);
			for(i=0;i<masksize;i++)
				printf("    %f\n",fmaski[i]);
		}
		return(0);
	}
	if (gflag) {
		i = 1;
		while (i < masksize)
			i <<= 1;
		fprintf(stderr,"%s: generating %dx%d image\n",Progname,i,i);
		if (iflag) {
			init_header(&hdp,"","dog output",1,"",i,i,PFINT,
				1,"");
			alloc_image(&hdp);
			val.v_int = 0;
			h_setimage(&hdp,&val);
			p = (int *) hdp.image;
			p[i*(i/2) + i/2] = 1;
		}
		else {
			init_header(&hdp,"","dog output",1,"",i,i,PFFLOAT,
				1,"");
			alloc_image(&hdp);
			val.v_float = 0;
			h_setimage(&hdp,&val);
			pf = (float *) hdp.image;
			pf[i*(i/2) + i/2] = 1;
		}
		dup_header(&hdp,&hd);
	}
	else {
		fp = hfopenr(filename);
		fread_hdr_a(fp,&hd,filename);
		method = fset_conversion(&hd,&hdp,iflag ? typesi : typesf,
			filename);
	}
	dup_headern(&hdp,&hdt1);
	setsize(&hdt1,hdp.rows,hdp.cols);
	alloc_image(&hdt1);
	if (!r0flag) {
		dup_headern(&hdt1,&hdt2);
		alloc_image(&hdt2);
	}
	if (iflag) {
		if (sizeof(int) == sizeof(float)) {
			imaske = (int *) fmaske;
			if (!r0flag)
				imaski = (int *) fmaski;
		}
		else {
			imaske = (int *) memalloc(masksize,sizeof(int));
			if (!r0flag)
				imaski = (int *) memalloc(masksize,sizeof(int));
		}
		for (i=0;i<masksize;i++) {
			imaske[i] = fmaske[i]*1000 + .5;
			if (!r0flag)
				imaski[i] = fmaski[i]*1000 + .5;
		}
	}
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		if (fr > 1)
			fprintf(stderr,"%s: starting frame #%d\n",Progname,f);
		if (!gflag)
			fread_imagec(fp,&hd,&hdp,method,f,filename);
		if (!r0flag) {
			if (iflag)
				h_sepconv(&hdp,&hdt1,&hdt2,imaski,masksize,
					masksize/2,imaski,masksize,masksize/2);
			else
				h_sepconv(&hdp,&hdt1,&hdt2,fmaski,masksize,
					masksize/2,fmaski,masksize,masksize/2);
		}
		if (iflag)
			h_sepconv(&hdp,&hdt1,&hdp,imaske,masksize,masksize/2,
				imaske,masksize,masksize/2);
		else
			h_sepconv(&hdp,&hdt1,&hdp,fmaske,masksize,masksize/2,
				fmaske,masksize,masksize/2);
		if (!r0flag)
			h_diff(&hdp,&hdt2,&hdp);
		write_imagec(&hd,&hdp,method,hips_convback,f);
	}
	return(0);
}
