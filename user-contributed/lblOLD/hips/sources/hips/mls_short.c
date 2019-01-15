static char *SccsId = "%W%      %G%";

/*	Copyright (c) 1985 Peter Mowforth & Jin Zhengping

/*
   version for short data
*/

/*
Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * mls.c - apply a maximum likelihood smoothing filter to an image
 *
 * usage:	mls [bigsize [smallsize]] [-pN] <iseq >oseq
 *
 * where bigsize is the length of the side of the window in which mls is
 * computed. Bigsize defaults to 3. Smallsize is the length of the side
 * of the windows which make up the original window, to find the window
 * with the greatest concentration of the nearest neighbours. Smallsize
 * defaults to (bigsize + 1) / 2. N is the percentage(x 100) of the nearest
 * neighbours in the original window which have the closest grey levels
 * to that of the central pixel. N defaults to 62.
 *
 * to load:	cc -o mls_short mls_short.c -lhips
 *
 * Peter Mowforth & Jin Zhengping - 8/5/85
 */

#include <stdio.h>
#include <hipl_format.h>

#define MAXVAL 2000		/* maximum allowed input value, (algorithm
				 * become terribly slow for larger values ) */
#define  N MAXVAL + 1		/* maximum number of histogram buckets */
#define  M 11

main(argc, argv)
    char     *argv[];

{
    int       argcp, f, fr, r, c, b, i, j, k = -1;
    int       l, ii, jj, kk, ll, iii, jjj, lll, ir, ic, sum;
    int       minus, plus, top, bot, left, right;
    int       bigsize, smallsize = -1, ss1, size, smallsq;
    int       low, up, work, max, count;
    struct header hd;
    short    *ifr, *ofr, *ip, *op, *nnp;
    int       his[N], win[M][M], winl[M][M];



    Progname = strsave(*argv);
    argcp = argc;
    if (--argc > 0) {
	if (argv[argc][1] == 'p') {
	    argv[argc][0] = argv[argc][1] = '0';
	    k = atoi(argv[argc]);
	} else if (argc > 1 & argv[argc - 1][1] != 'p') {
	    smallsize = atoi(argv[argc--]);
	    bigsize = atoi(argv[argc]);
	} else
	    bigsize = atoi(argv[argc]);
	if (--argc > 0)
	    if (argv[argc][1] == 'p') {
		argv[argc][0] = argv[argc][1] = '0';
		k = atoi(argv[argc]);
	    } else if (argc > 1 & argv[argc - 1][1] != 'p') {
		smallsize = atoi(argv[argc--]);
		bigsize = atoi(argv[argc]);
	    } else
		bigsize = atoi(argv[argc]);
	else if (k == -1)
	    k = 62;
	else
	    bigsize = 3;
	if (argc > 1) {
	    fprintf(stderr, "command usage: mls [bigsize [smallsize]] [-pN]\n");
	    exit(1);
	}
    } else {
	k = 62;
	bigsize = 3;
    }

    if (bigsize < 1 || bigsize > 11) {
	fprintf(stderr, "mls: unreasonable bigsize specified\n");
	exit(1);
    }
    if (k < 0 || k > 100) {
	fprintf(stderr, "mls: unreasonable N specified\n");
	exit(1);
    }
    if (smallsize == -1)
	smallsize = (bigsize + 1) / 2;
    if (smallsize < 1 || smallsize > bigsize) {
	fprintf(stderr, "mls: unreasonable smallsize specified\n");
	exit(1);
    }
    smallsq = smallsize * smallsize;
    ss1 = smallsize - 1;
    k = k * bigsize * bigsize / 100;

    plus = bigsize / 2;
    minus = plus - bigsize + 1;

    read_header(&hd);
    if (hd.pixel_format != PFSHORT) {
	fprintf(stderr, "mls: pixel format must be short\n");
	exit(1);
    }
    r = hd.orows;
    c = hd.ocols;
    fr = hd.num_frame;

    top = -minus;
    bot = r - plus;
    left = -minus;
    right = c - plus;

    update_header(&hd, argcp, argv);
    write_header(&hd);

    if ((ifr = (short *) calloc(r * c, sizeof(short))) == 0 ||
	(ofr = (short *) calloc(r * c, sizeof(short))) == 0) {
	fprintf(stderr, "mls: can't allocate core\n");
	exit(1);
    }
/* processing begin */

    for (f = 0; f < fr; f++) {
        fprintf(stderr,"\n processing frame %d...",f);
	if (fread(ifr, r * c * sizeof(short),1,stdin) != 1) {
	    fprintf(stderr, "mls: error during read\n");
	    exit(1);
	}
	for (i = 0; i < r * c; i++)
	    if (ifr[i] > MAXVAL) {
		fprintf(stderr,"\n Error: value in input file is too large!");
		fprintf(stderr,"\n All values must be less than %d.\n\n", MAXVAL);
		exit(-1);
	    }
	ip = ifr;
	op = ofr;

/* compute histogram and initialize window */

	for (i = 0; i < r; i++)
	    for (j = 0; j < c; j++) {
		for (l = 0; l < N; l++)
		    his[l] = 0;
		if (i < top || i >= bot || j < left || j >= right) {
		    for (iii = 0, ii = minus; ii <= plus; ii++, iii++)
			for (jjj = 0, jj = minus; jj <= plus; jj++, jjj++) {
			    ir = i + ii;
			    ic = j + jj;
			    ir = ir < 0 ? 0 : (ir >= r) ? r - 1 : ir;
			    ic = ic < 0 ? 0 : (ic >= c) ? c - 1 : ic;
			    work = ifr[ir * c + ic] & 0xffff;
			    ++his[work];
			    win[iii][jjj] = work;
			    winl[iii][jjj] = 0;
			}
		} else {
		    nnp = ip + minus * c + minus;
		    for (iii = 0, ii = minus; ii <= plus; ii++, iii++) {
			for (jjj = 0, jj = minus; jj <= plus; jj++, jjj++) {
			    work = *nnp++ & 0xffff;
			    ++his[work];
			    win[iii][jjj] = work;
			    winl[iii][jjj] = 0;
			}
			nnp += c - bigsize;
		    }
		}

/* find boundaries of grey levels of nearest neighbours */

		l = 0;
		ll = lll = *ip & 0xffff;
		kk = his[ll] - 1;
		while (kk < k) {
		    l = (l > 0) ? -l : (-l + 1);
		    lll = ll + l;
		    if (lll > MAXVAL)
			lll = MAXVAL;
		    else if (lll < 0)
			lll = 0;
		    kk += his[lll];
		}
		if (l > 0) {
		    low = ll - l + 1;
		    up = ll + l;
		} else {
		    low = ll + l;
		    up = ll - l;
		}

/* find maximum number of nearest neighbours that a small wimdow contains */

		for (iii = 0; iii < bigsize; iii++)
		    for (jjj = 0; jjj < bigsize; jjj++)
			if (win[iii][jjj] <= up)
			    if (win[iii][jjj] >= low)
				winl[iii][jjj] = 1;

		max = 0;
		for (ii = ss1; ii < bigsize; ii++)
		    for (jj = ss1; jj < bigsize; jj++) {
			count = 0;
			for (iii = ii - ss1; iii <= ii; iii++)
			    for (jjj = jj - ss1; jjj <= jj; jjj++)
				if (winl[iii][jjj])
				    count++;
			if (count > max)
			    max = count;
		    }

/* average the central pixel */

		sum = size = 0;
		for (ii = ss1; ii < bigsize; ii++)
		    for (jj = ss1; jj < bigsize; jj++) {
			count = 0;
			for (iii = ii - ss1; iii <= ii; iii++)
			    for (jjj = jj - ss1; jjj <= jj; jjj++)
				if (winl[iii][jjj])
				    count++;
			if (count == max) {
			    for (iii = ii - ss1; iii <= ii; iii++)
				for (jjj = jj - ss1; jjj <= jj; jjj++)
				    sum += win[iii][jjj];
			    size += smallsq;
			}
		    }

		ip++;
		*op++ = sum / size;
	    }
	if (fwrite(ofr, r * c * sizeof(short),1,stdout) != 1) {
	    fprintf(stderr, "mls: error during write\n");
	    exit(1);
	}
    }
    return (0);
}
