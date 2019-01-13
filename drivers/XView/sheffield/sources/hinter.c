/* Hips interface functions */

#include <stdlib.h>
#include <hipl_format.h>
#include <xdisp.h>

extern Flag_Format flagfmt[];
extern Filename filename;
extern int types[];

/***********************************
 * hips_init()
 ***********************************/

/* initialize hips subsystem */

hips_init(argc,argv)
  int argc;
  char *argv[];
{
    Progname = strsave(*argv);
    parseargs(argc,argv,flagfmt,&option_e,&ncolors,&maxrows,&maxcols,
			FFONE,&filename);
}


/**************************************
 * read_hips_image()
 **************************************/

/* Read in a Hips image from file filename */

void read_hips_image(file)
  char *file;
{
    polygon_t *p;
    vertex_t *v;
    int *pars;
    int ct;
    struct extpar *xp;

    FILE		*fp;
    int			method;
    int			i;
    float		lb,lc,fmin,fmax;
    Pixelval		minval,maxval;
    struct header	hdf,hdfo;

    /*
     * read header
     */ 

    filename = file;
    fp = hfopenr(filename);
    fread_header(fp,&hdr,filename);

    pixel_format = hdr.pixel_format;
    nrows = hdr.orows;
    ncols = hdr.ocols;
    nframes = hdr.num_frame;

    images = (unsigned char **)malloc((nframes+1)*sizeof(unsigned char *));
    samples = (unsigned char **)malloc((nframes+1)*sizeof(unsigned char *));
    data = (unsigned char **)malloc(nframes*sizeof(unsigned char *));
    hists = (int **)malloc(nframes*sizeof(int *));
    cdfs = (int **)malloc(nframes*sizeof(int *));

    /* 
     * read in the image
     */

    for (i = 0; i < nframes; i++) {
	fprintf(stderr,"reading frame %d\n",i);
	alloc_image(&hdr);

	if (pixel_format == PFBYTE) {
	    dup_header(&hdr,&hdp);
	    fread_image(fp,&hdr,i,filename);
	    }
	else {
	    method = fset_conversion(&hdr,&hdf,types,filename);
	    fread_imagec(fp,&hdr,&hdf,method,i,filename);
	    if (option_e || i == 0) {
	    	h_minmax(&hdf,&minval,&maxval,FALSE);
	    	fmin = minval.v_float;
	    	fmax = maxval.v_float;
	    	fprintf(stderr,"min = %.2f, max = %.2f\n",fmin,fmax);
	    	lb = (fmax > fmin) ? 255/(fmax-fmin) : 0.1;
	    	lc = -fmin*lb;
		}
	    if (pixel_format == PFFLOAT) {
		dup_headern(&hdf,&hdfo);
		alloc_image(&hdfo);
		}
	    else
		dup_header(&hdf,&hdfo);
	    h_linscale(&hdf,&hdfo,lb,lc);
	    dup_headern(&hdr,&hdp);
	    setformat(&hdp,PFBYTE);
	    alloc_image(&hdp);
	    h_tob(&hdfo,&hdp);
	    free_image(&hdfo);
	    }

	data[i] = (unsigned char *)hdr.firstpix;
	images[i] = (unsigned char *)hdp.firstpix;
	hists[i] = (int *)calloc(HIST_WIDTH,sizeof(int));
	cdfs[i] = (int *)calloc(HIST_WIDTH,sizeof(int));
	}

    images[nframes] = NULL;

    /* check for hips lut */

    if (findparam(&hdr,"cmap") != (struct extpar *)NULLPAR) {
	hips_lut_exists = 1;
    	getparam(&hdr,"cmap",PFBYTE,&hips_lut_n,&hips_lut);
	hips_lut_n/= 3;
	}
    else
	hips_lut_exists = 0;

    /* check for polygons */

    while((xp = grepparam(&hdr,".poi")) != (struct extpar *)NULLPAR) {
	getparam(&hdr,xp->name,PFINT,&ct,&pars);
	for (i = 0; i < ct; i+= 2)
	    add_poly_vertex(pars[i],pars[i+1]);
	if (xp->name[0] != '.') {
	    current_polygon->name = (char *)malloc(strlen(xp->name));
	    strtok(xp->name,".");
	    strcpy(current_polygon->name,xp->name);
	    }
	add_final_vertex();
	clearparam(&hdr,xp->name);
	}

}
