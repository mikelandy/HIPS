
/* speed test program for arrays rnts  */

#include <stdio.h>
#include <sys/types.h>

/*****************************************************/
main(argc, argv)
    int       argc;
    char    **argv;
{
    u_char ***image;
    int       rw, cl, nf;
    register int c, r, f, i;
    register u_char *bptr, *gptr;
    int       npix;

    long      tbuf[4], ut1, ut2;

    u_char ***alloc_3d_byte_array();

    rw = 50;
    cl = 50;
    nf = 50;
    npix = nf * rw * cl;

    fprintf(stderr, "\n size: rows: %d,  cols: %d,  frames: %d \n", rw, cl, nf);

    image = alloc_3d_byte_array(nf, rw, cl);

    /* speed test #1  */

    times(tbuf);
    ut1 = tbuf[0];

    bptr = **image;

    for (i = 0; i < npix; i++) {
	*(bptr++) = 100;
    }

    times(tbuf);
    ut2 = tbuf[0];
    fprintf(stderr, "result method 1: user time was =%d\n", ut2 - ut1);

    times(tbuf);
    ut1 = tbuf[0];
    bptr = **image;

    for (i = 0; i < npix; i++) {
	bptr[i] = 100;
    }

    times(tbuf);
    ut2 = tbuf[0];
    fprintf(stderr, "result method 2: user time was =%d\n", ut2 - ut1);

    times(tbuf);
    ut1 = tbuf[0];
    for (f = 0; f < nf; f++)
	for (r = 0; r < rw; r++)
	    for (c = 0; c < cl; c++)
		image[f][r][c] = 100;

    times(tbuf);
    ut2 = tbuf[0];
    fprintf(stderr, "result method 3: user time was =%d\n", ut2 - ut1);

    fprintf(stderr, "\n\n finished. \n\n");
    return (0);
}
