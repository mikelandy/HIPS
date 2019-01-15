/*	Copyright (c) 1982 Michael Landy, Yoav Cohen, and George Sperling

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * disphist_short.c - display disphistgram files as a bar graph
 *
 * usage:	disphist_short [-b -m maxcnt] <hist >graphimage
 *
 * -m   specifies an initial maximum bincount for use in scaling the
 *      displays.  Otherwise, the maximum in the first histogram is used,
 *      and increased if when a later histogram exceeds it.
 * -b   specifies an output image size of 512x512. Default is 256x256.
 * -l   take the log of the counts before plotting
 *
 * This is a hack. disphist and disphist_short should be combined.
 *
 * to load:	cc -o disphist_short disphist_short.c -lhipl
 *
 * Michael Landy - 12/15/82
 */

#include <hipl_format.h>
#include <stdio.h>

int mflag = 0;
int bflag = 0;
int lflag = 0;
int maxcnt = 1;
int image_width;
unsigned char border_color = 127;
unsigned char background_color = 0;
unsigned char bar_color = 255;

main(argc,argv)

int argc;
char **argv;

{
    int i,j,k,r,c,f,fr,binleft,numbin,binwidth,*hist,*hp,one=1;
    unsigned char out256[256*256],*op;
    unsigned char out512[512*512];
    struct header hd;
    char tmp[100];

    Progname = strsave(*argv);
    read_header(&hd);
    r=hd.orows;
    c=hd.ocols;
    f=hd.num_frame;
    update_header(&hd,argc,argv);

    while (--argc > 0)
        {
        argv++;
        if (argv[0][0] == '-')
    	{
    	    switch(argv[0][1])
    		{
    		case 'D':
    			continue;
    		case 'm':
    			mflag++;
    			maxcnt = atoi(*(++argv));
    			argc--;
    			continue;
    		case 'b':
    			bflag++;
    			argc--;
    			continue;
    		case 'l':
    			lflag++;
    			argc--;
    			continue;
    		default:
    			sprintf(tmp,"unknown switch %s\n",argv[0]);
			perr(HE_MSG,tmp);
    		}
    	}
    }

    if (hd.pixel_format != PFHIST)
    	perr(HE_MSG,"image must be in histogram format");
    hd.pixel_format = PFBYTE;
    
    if (!bflag)
	image_width = 256;
    else
	image_width = 512;
    hd.orows = hd.rows = image_width;
    hd.ocols = hd.cols = image_width;

    getparam(&hd,"imagepixfmt",PFINT,&one,&i);
    if (i != PFBYTE)
	    perr(HE_MSG,"image format has to have been byte");
    getparam(&hd,"numbin",PFINT,&one,&numbin);
    getparam(&hd,"binleft",PFINT,&one,&binleft);
/*  getparam(&hd,"binwidth",PFINT,&one,&i); */
    binwidth = 65535/numbin;
/*
    if (binwidth*numbin != 65535)
    	perr(HE_MSG,"strange number of bins!!?");
*/

    write_header(&hd);
    hist = (int *) halloc(numbin+2,sizeof(int));

    fprintf(stderr,"disphist: image size was %d x %d\n",r,c);
    fprintf(stderr,"disphist: first bin starts at %d, and there are %d bins\n",binleft,numbin);

    for(fr=0; fr<f; fr++)
	{  /* loop over sequence */ 
    	if (!bflag )
	    op = out256;
	else
	    op = out512;
		/* put on a border  */
   	for (i=0; i<image_width; i++)
    	    for (j=0; j<image_width; j++)
		{
		if (j==0 || j==(image_width -1) || i==0 || i==(image_width -1))
			*op++ = border_color;
    		else
    			*op++ = background_color;
    		}

    	if (fread(hist,numbin*sizeof(int),1,stdin) != 1)
    		perr(HE_MSG,"error during read");

    	/* print underflow/overflow here, when add this */
    	hp = hist+1;
    	j = 0;
    	for (i=0; i<numbin; i++)
	    {
    	    if (*hp > maxcnt)
		{
    		j++;
    		maxcnt = *hp++;
    		}
    	    else
    		hp++;
    	    }
    	if (j)
    		fprintf(stderr,"disphist: frame %d maxcnt increased to %d\n",
    			fr,maxcnt);

    	for (i=0; i<=image_width; i++)
	    {
		/* get height of bar */
    	    j = hist[(i/binwidth)+1]*(image_width-2)/maxcnt;
    	    if (j>(image_width -2)) j = (image_width -2);
#ifdef ULORIG
	    if(!bflag)
    	        op = out256 + i + (image_width-1)*image_width;
	    else
    	        op = out512 + i + (image_width-1)*image_width;
#else
	    if(!bflag)
    	        op = out256 + i + image_width;
	    else
    	        op = out512 + i + image_width;
#endif
    	    for (k=0; k<j ;k++)
		{
    		*op = bar_color;
#ifdef ULORIG
    		op -= image_width;
#else
    		op += image_width;
#endif
    		}
    	}

	if(!bflag)
    	if (fwrite(out256,image_width*image_width*sizeof(char),1,stdout) != 1)
    		perr(HE_MSG,"error during write");
	else
    	if (fwrite(out512,image_width*image_width*sizeof(char),1,stdout) != 1)
    		perr(HE_MSG,"error during write");
    }
}
