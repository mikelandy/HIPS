
/* make_mask.c                           -Brian Tierney,  LBL   6/90
 *
 * make_mask creates an edge detector mask of a  specified  size
 * for  detecting  edges  at a variety of angles. This file can
 * then be used as input to the HIPS  program  mask.  The  mask
 * program performs a convolution of the image and this mask.
 *
 * Usage: see usageterm at end of this file (-h for help)
 *
 *  Note: should use odd numbers for symetric masks
 */


#include <stdio.h>
#include <hipl_format.h>

#define MS 40

/* useful globals */
char      mask1[MS][MS], mask2[MS][MS];
int       gap, k, size;
int    m0, m22, m45, m67, m90, m112, m135, m157; /* flags */

void usageterm(),parse_args(),make_0_90_masks(),print_mask(),fill_gap();
void make_45_135_masks(),make_22_157_masks(),make_67_112_masks();

int main(argc, argv)
    int       argc;
    char    **argv;
{
    int num_masks;

    parse_args(argc, argv);

    num_masks = m0 + m22 + m45 + m67 + m90 + m112 + m135 + m157;

    fprintf(stdout, " %d x %d edge detector \n", size, size);
    fprintf(stdout," %d   5   2\n",num_masks);

    make_0_90_masks();
    if (m0)
	print_mask(mask1);
    if (m90)
	print_mask(mask2);

    make_45_135_masks();
    if (m45)
	print_mask(mask1);
    if (m135)
	print_mask(mask2);

    make_22_157_masks();
    if (m22)
	print_mask(mask1);
    if (m157)
	print_mask(mask2);

    make_67_112_masks();
    if (m67)
	print_mask(mask1); 
    if (m112)
	print_mask(mask2); 
    return(0);
}


/*********************************************************/
void make_0_90_masks()
{
    int       i, j, mid;

    /* horizontal and vertical masks */
    for (i = 0; i < size; i++)
	for (j = 0; j < size; j++) {

	    mid = size / 2;
	    if (i == mid) {
		mask1[i][j] = 2;

	    } else if (i == 0 || i == size - 1) {
		mask1[i][j] = -1;

	    } else {
		mask1[i][j] = 0;
	    }

	    if (j == mid) {
		mask2[i][j] = 2;

	    } else if (j == 0 || j == size - 1) {
		mask2[i][j] = -1;

	    } else {
		mask2[i][j] = 0;
	    }
	}
}

/**************************************************************/
void make_45_135_masks()
{
    int       i, j;
    int       n, mid, qurt;

    for (i = 0; i < size; i++)
	for (j = 0; j < size; j++) {

	    n = i + j;
	    mid = size - 1;
	    qurt = mid / 2;
	    if (n == mid) {
		mask1[i][j] = 2;
		mask2[size - 1 - i][j] = 2;

	    } else if (n == mid - 1 || n == mid + 1) {
		mask1[i][j] = 0;
		mask2[size - 1 - i][j] = 0;

	    } else if ((n == mid - qurt) || (n == mid + qurt) ||
		       (n == mid - qurt - 1) || (n == mid + qurt + 1)) {
		mask1[i][j] = -1;
		mask2[size - 1 - i][j] = -1;

	    } else {
		mask1[i][j] = 0;
		mask2[size - 1 - i][j] = 0;
	    }
	}
}

/**************************************************************/
void make_22_157_masks()
{
    int       i, j, i2, j2;

    i2 = j2 = size / 2;
    while (j2 >= 0) {
	mask1[i2][j2] = 2;
	mask2[size - 1 - i2][j2] = 2;

	i = i2 - 1;
	j = j2;
	gap = k = 0;
	while (i >= 0) {
	    fill_gap(i, j);
	    i--;
	}

	i = i2 + 1;
	j = j2;
	gap = k = 0;
	while (i < size) {
	    fill_gap(i, j);
	    i++;
	}

	if (((j2 / 2) * 2) == j2)	/* even number */
	    i2++;
	j2--;
    }

    i2 = j2 = size / 2;
    while (j2 < size) {
	mask1[i2][j2] = 2;
	mask2[size - 1 - i2][j2] = 2;

	i = i2 - 1;
	j = j2;
	gap = k = 0;
	while (i >= 0) {
	    fill_gap(i, j);
	    i--;
	}

	i = i2 + 1;
	j = j2;
	gap = k = 0;
	while (i < size) {
	    fill_gap(i, j);
	    i++;
	}

	if (((j2 / 2) * 2) == j2)	/* even number */
	    i2--;
	j2++;
    }

}
/**************************************************************/
void make_67_112_masks()
{
    int       i, j, i2, j2;

    i2 = j2 = size / 2;
    while (i2 >= 0) {
	mask1[i2][j2] = 2;
	mask2[size - 1 - i2][j2] = 2;

	j = j2 - 1;
	i = i2;
	gap = k = 0;
	while (j >= 0) {
	    fill_gap(i, j);
	    j--;
	}

	j = j2 + 1;
	i = i2;
	gap = k = 0;
	while (j < size) {
	    fill_gap(i, j);
	    j++;
	}

	if (((i2 / 2) * 2) == i2)	/* even number */
	    j2++;
	i2--;
    }

    i2 = j2 = size / 2;
    while (i2 < size) {
	mask1[i2][j2] = 2;
	mask2[size - 1 - i2][j2] = 2;

	j = j2 - 1;
	i = i2;
	gap = k = 0;
	while (j >= 0) {
	    fill_gap(i, j);
	    j--;
	}

	j = j2 + 1;
	i = i2;
	gap = k = 0;
	while (j < size) {
	    fill_gap(i, j);
	    j++;
	}

	if (((i2 / 2) * 2) == i2)	/* even number */
	    j2--;
	i2++;
    }

}

/*********************************************/
void fill_gap(i, j)
    int       i, j;
{
    if (gap < size / 3) {
	mask1[i][j] = 0;
	mask2[size - 1 - i][j] = 0;
	gap++;
    } else if (k < 2) {
	mask1[i][j] = -1;
	mask2[size - 1 - i][j] = -1;
	k++;
    } else {
	mask1[i][j] = 0;
	mask2[size - 1 - i][j] = 0;
    }
}


/*********************************************/

void print_mask(mask)
    char      mask[MS][MS];
{
    int       i, j;

    fprintf(stdout,"\n%d   %d   %d   %d\n\n",size,size,size/2,size/2);
    for (i = 0; i < size; i++) {
	fprintf(stdout, " \n");
	for (j = 0; j < size; j++) {
	    fprintf(stdout, "%5d", mask[i][j]);
	}
    }
    fprintf(stdout, " \n\n");
}
/****************************************************************/

void parse_args(argc, argv)
    int       argc;
    char     *argv[];
{
    m0 = m22 = m45 = m67 = m90 = m112 = m135 = m157 = 0;
    size = 7;			/* default */

    /* Interpret options  */
    while (--argc > 0 && (*++argv)[0] == '-') {
        char     *s;
        for (s = argv[0] + 1; *s; s++)
            switch (*s) {
            case 'a':  /* all masks */
		m0 = m22 = m45 = m67 = m90 = m112 = m135 = m157 = 1; 
                break;
            case '1':
		m0++;
                break;
            case '2':
		m22++;
                break;
            case '3':
		m45++;
                break;
            case '4':
		m67++;
                break;
            case '5':
		m90++;
                break;
            case '6':
		m112++;
                break;
            case '7':
		m135++;
                break;
            case '8':
		m157++;
                break;
            case 's':
                if (argc < 2)
                    usageterm();
                size = atoi(*++argv);
                fprintf(stderr, " mask size: %d\n", size);
                argc--;
                break;
            case 'h':
                usageterm();
                break;
            default:
                usageterm();
                break;
            }
    }                           /* while */

    if ( (m0 + m22 + m45 + m67 + m90 + m112 + m135 + m157) == 0) {
	/* set default */
	m0 = m45 = m90 = m135 = 1; 
    }
    if (size >= MS) {
	fprintf(stderr, "Error, size must be less than %d \n\n",MS);
	exit(0);
    }
}

/******************************************************/
void
usageterm()
{
    fprintf(stderr, "Usage: make_mask [-s] [-1 .. -8] [-a] > outfile  \n");
    fprintf(stderr, "   [-s] size of output mask (default = 7) \n");
    fprintf(stderr, "       (NOTE: use odd #'s for symmetric masks) \n");
    fprintf(stderr, "   [-1] mask at angle of 0    degrees \n");
    fprintf(stderr, "   [-2] mask at angle of 22.5 degrees \n");
    fprintf(stderr, "   [-3] mask at angle of 45   degrees \n");
    fprintf(stderr, "   [-4] mask at angle of 67.5 degrees \n");
    fprintf(stderr, "   [-5] mask at angle of 90   degrees \n");
    fprintf(stderr, "   [-6] mask at angle of 112.5 degrees \n");
    fprintf(stderr, "   [-7] mask at angle of 135  degrees \n");
    fprintf(stderr, "   [-8] mask at angle of 157.5 degrees \n");
    fprintf(stderr, "   [-a] mask with all of the above angles \n");
    fprintf(stderr, "     ( default is -1 -3 -5 -7 ) \n\n");

    exit(0);
}
