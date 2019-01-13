/*      Copyright (c) 1990 Carsten Kruse Olsson

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * framecut.c - cuts a image into subimages (succecive frames)
 *
 * usage - framecut [-l <side lenght> | 
 *                      -lh <horisontal side length> | 
 *                      -lv <vertical side length>]
 *                [-c <center spacing> | 
 *                      -ch <horisontal center spacing> |
 *                      -cv <vertical center spacing>] 
 *                <insequence >outsequence
 *
 * The input sequence can be in byte, short int, int,
 * float, complex, double, or double complex format and the output has the
 * same format.
 * The input image is cut into a number of new images each with side length
 * as specified by the -l parameter (16 default) and sampled -c pixels
 * apart (default: same as -l parameter) in the input image. 
 *
 * To load: cc -o framecut framecut.c  -lhipl
 *
 * 25/5-90 cko
 */

#include <hipl_format.h>
#include <stdio.h>
#include <string.h>

char usage[255]={"Usage: framecut [-l <side length> | -lr <row sl> | -lc <col sl>] \
                [-c <center spacing> | -cr <row cs> | -cc <col cs>"};



int main(argc,argv)

int argc;
char *argv[];

{
        struct header hd;
        int rside = 16, cside = 16, rspce = -1, cspce = -1;
        int     i, j, r, c;
        int     frame, nf, rfr, cfr;
        hsize_t  nrows, ncols, nbyte;
        char    *inim, *outim, *op;

        Progname = strsave(*argv);
        read_header(&hd);
        for (i=1;i<argc;i++) {
                if (argv[i][0]=='-') {
                        switch(argv[i][1]) {
                        case 'l':
                                switch (argv[i][2]){
                                case 'h':       rside = atoi(argv[++i]); break;
                                case 'v':       cside = atoi(argv[++i]); break;
                                case '\0':      rside = atoi(argv[++i]);
                                                cside = rside;
                                                break;
                                default:        perr(HE_MSG,usage);
                                }
                                break;
                        case 'c':
                                switch (argv[i][2]){
                                case 'h':       rspce = atoi(argv[++i]); break;
                                case 'v':       cspce = atoi(argv[++i]); break;
                                case '\0':      rspce = atoi(argv[++i]);
                                                cspce = rspce;
                                                break;
                                default:        perr(HE_MSG,usage);
                                }
                                break;
                        default:        perr(HE_MSG,usage);
                        }
                }
                else
                        perr(HE_MSG,usage);
        }
        if (rspce<0)
		rspce = rside;
        if (cspce<0)
		cspce = cside;
        update_header(&hd,argc,argv);
        
        nrows=hd.orows; ncols=hd.ocols; nf=hd.num_frame;

        switch (hd.pixel_format){
        case PFBYTE:    nbyte = 1; break;
        case PFSHORT:   nbyte = 2; break;
        case PFINT:
        case PFFLOAT:   nbyte = 4; break;
        case PFCOMPLEX:
        case PFDOUBLE:  nbyte = 8; break;
        case PFDBLCOM:  nbyte = 16; break;
        default: perr(HE_MSG,"illegal input image format");
        }

        inim = (char *) malloc(nrows*ncols*nbyte);

        if (nrows%rspce==0)     rfr = nrows/rspce;
        else                    rfr = nrows/rspce+1;
        if (ncols%cspce==0)     cfr = ncols/cspce;
        else                    cfr = ncols/cspce+1;

        hd.num_frame = nf*rfr*cfr;
	setsize(&hd,rside,cside);

        write_header(&hd);

        if ((outim=(char *)malloc(rfr*cfr*rside*cside*nbyte))==NULL)
                perr(HE_MSG,"Not enough core");

        for (frame=0; frame<nf; frame++) {
                if (fread(inim,ncols*nrows*nbyte,1,stdin) != 1)
                        perr(HE_MSG,"Error during read");
                for (r=0, op=outim; r<nrows; r+=rspce)
                        for (c=0; c<ncols; c+=cspce)
                                for (i=0; i<rside; i++){
                                        if (r+i<nrows){
                                                for (j=0; j<cside; j++, op+=nbyte){
                                                        if (c+j<ncols)
                                                                memcpy((void *)op, (void *)&(inim[((r+i)*ncols+c+j)*nbyte]),nbyte);
                                                        else
                                                                memset((void *)op, 0, nbyte);
                                                }
                                        }
                                        else{
                                                memset((void *)op, 0, cside*nbyte);
                                                op += cside*nbyte;
                                        }
                                }
                if (fwrite(outim,rfr*cfr*rside*cside*nbyte,1,stdout) != 1)
                        perr(HE_MSG,"error during write");
        }
        return(0);
}
