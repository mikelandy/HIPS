static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * translate - translate each image in the input sequence
 *
 * usage:	translate [-d rows cols] [-o srow scol]
 *
 *    rows,cols   (rows,cols) is a pair of integers specifying the size
 *                of the output image. It defaults to the size of the
 *                input image.
 *
 *    srow,scol   (srow,scol) is a pair of floating point values for
 *                the offset to be translated. It default to half of
 *                the input image size.
 *
 *  Note: any part of the translated image that lie outside the output
 *  image will be clipped.
 *
 * to load:	cc -o translate translate.c -lhipsa -lhips
 *
 * Jin Zhengping -8/3/89 
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] =
{
        {"d",
                {LASTFLAG},
                2,
                {{PTINT,"-1","rows"},{PTINT,"-1","cols"},LASTPARAMETER}},
        {"o",
                {LASTFLAG},
                2,
                {{PTDOUBLE,"-1.0","srow"},{PTDOUBLE,"-1.0","scol"},LASTPARAMETER}},
        LASTFLAG
};

int main(argc,argv)

int     argc;
char    **argv ;
 
{
        struct          header hd,hdp,hdo;
        int             method,f,fr;
        Filename        filename;
        FILE            *fp;
        double          srow,scol;
        int             rowsn,colsn;
        byte            interpolation();

	Progname = strsave(*argv);
        parseargs(argc,argv,flagfmt,&rowsn,&colsn,&srow,&scol,FFONE,&filename);
        fp=hfopenr(filename);
        fread_hdr_a(fp,&hd,filename);
        if (rowsn==-1)
        {
                rowsn = hd.orows ;
                colsn = hd.ocols ;
        }
        if (rowsn <= 0 || colsn <= 0)
                perr(HE_MSG,"reasonable image size specified?");
        if (srow==-1.0)
        {
                srow = 0.5*hd.orows ;
                scol = 0.5*hd.ocols ;
        }
        method=fset_conversion(&hd,&hdp,types,filename);
        dup_headern(&hdp,&hdo);
        setsize(&hdo,rowsn,colsn);
        alloc_image(&hdo);
        write_headeru2(&hd,&hdo,argc,argv,hips_convback);
        fr = hdp.num_frame;
        for (f=0;f<fr;f++)
        {
                fread_imagec(fp,&hd,&hdp,method,f,filename);
                h_translatei(&hdp,&hdo,srow,scol);
                write_imagec(&hd,&hdo,method,hips_convback,f);
        }
        return(0);
}
