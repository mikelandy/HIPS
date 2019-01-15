/* scaletbl.c - Scales a HIPS table from short, int, or float to byte
 *		for use in the XV Map Apply window.
 *
 * usage: scaletbl [-l lower_bound] [-u upper_bound] [-fill value]
		< source_tablename
		> destination_tablename
 *
 * lower_bound and upper_bound mark off the respective boundaries over the
 * range 0..255 to which the scaling functions are applied.  Gray values
 * outside this range may be mapped to a value or left unchanged, according
 * to the fill flag.
 *
 * written by Bryan Skene (skene@george.lbl.gov)
 *		February 20, 1991
 *
 *  to load: cc -o scaletbl scaletbl.c -lhips -lm
 */

#include <stdio.h>

#include <hipl_format.h>
#include <math.h>

#if _STDC_
void      usage(void);
void      DoScaling_byte(void);
void      DoScaling_short(void);
void      DoScaling_int(void);
void      DoScaling_float(void);
#else
void      usage();
void      DoScaling_byte();
void      DoScaling_short();
void      DoScaling_int();
void      DoScaling_float();
#endif

#define MAXBYTE  256

unsigned char lower_bound = 0, upper_bound = 255;
int       numentries, form, fill = 0, fill_value;

unsigned char *blut;
short    *slut;
int      *ilut;
float    *flut;

main(argc, argv)
    int       argc;
    char    **argv;
{

    int       i = 0, entry;

    Progname = strsave(*argv);
    fprintf(stderr, "argc = %d\n", argc);

    if (argc > 1 && argv[1][0] == '-') {
	fprintf(stderr, "We're on the first arg ...\n");
	if (argv[1][1] == 'l') {
	    lower_bound = (unsigned char) atoi(argv[2]);
	} else if (argv[1][1] == 'u') {
	    upper_bound = (unsigned char) atoi(argv[2]);
	} else if (argv[1][1] == 'f') {
	    fill = 1;
	    fill_value = (unsigned char) atoi(argv[2]);
	} else
	    usage();
	if (argc > 3 && argv[3][0] == '-') {
	    fprintf(stderr, "We're on the second arg ...\n");
	    if (argv[3][1] == 'l') {
		lower_bound = (unsigned char) atoi(argv[4]);
	    } else if (argv[3][1] == 'u') {
		upper_bound = (unsigned char) atoi(argv[4]);
	    } else if (argv[3][1] == 'f') {
		fill = 1;
		fill_value = (unsigned char) atoi(argv[4]);
	    } else
		usage();
	    if (argc > 5 && argv[5][0] == '-') {
		fprintf(stderr, "We're on the third arg ...\n");
		if (argv[5][1] == 'l') {
		    lower_bound = (unsigned char) atoi(argv[6]);
		} else if (argv[5][1] == 'u') {
		    upper_bound = (unsigned char) atoi(argv[6]);
		} else if (argv[5][1] == 'f') {
		    fill = 1;
		    fill_value = (unsigned char) atoi(argv[6]);
		} else
		    usage();
	    }
	}
    }
    fprintf(stderr, "parsed the args ...\n");
    fscanf(stdin, "%d%d", &numentries, &form);
    fprintf(stderr, "Form: %d\tNumber of Entries: %d\n", form, numentries);
    fprintf(stdout, "%d\n%d\n", numentries, PFBYTE);

    switch (form) {
    case PFBYTE:
	blut = (unsigned char *) halloc(numentries,
					sizeof(unsigned char));
	for (i = 0; i < numentries; i++) {
	    fscanf(stdin, "%d", &entry);
	    blut[i] = (unsigned char) entry;
	}
	DoScaling_byte();
	break;
    case PFSHORT:
	slut = (short *) halloc(numentries, sizeof(int));
	for (i = 0; i < numentries; i++) {
	    fscanf(stdin, "%d", &entry);
	    slut[i] = (short) entry;
	}
	DoScaling_short();
	break;
    case PFINT:
	ilut = (int *) halloc(numentries, sizeof(int));
	for (i = 0; i < numentries; i++)
	    fscanf(stdin, "%d", &ilut[i]);
	DoScaling_int();
	break;
    case PFFLOAT:
	flut = (float *) halloc(numentries, sizeof(float));
	for (i = 0; i < numentries; i++)
	    fscanf(stdin, "%f", &flut[i]);
	DoScaling_float();
	break;
    default:
	perr(HE_MSG, "table is of unkown format");
    }
    exit(0);
}

void 
DoScaling_byte()
{

/* search table, find min and max, build a new blut based on min and max */
    unsigned char min, max, scaled_val, delta, offset;
    int       i;

    offset = 0;
    min = max = blut[1];
    for (i = 0; i < numentries; i++) {
	if (blut[i] < min)
	    min = blut[i];
	if (blut[i] > max)
	    max = blut[i];
    }
    if (min < 0)
	offset = (-min);
    delta = max - min;
    fprintf(stderr, "min = %d, max = %d, delta = %d\n", min, max, delta);

    for (i = 0; i < MAXBYTE; i++) {
	if (i >= lower_bound && i <= upper_bound)
	    scaled_val = (unsigned char) ((float) (blut[i] + offset)
					  / (float) delta * (float) MAXBYTE);
	else if (fill)
	    scaled_val = fill_value;
	else
	    scaled_val = i;
	printf("%d\n", scaled_val);
    }
}

void 
DoScaling_short()
{

/* search table, find min and max, build a new blut based on min and max */
    short     min, max, delta, offset;
    unsigned char scaled_val;
    int       i;

    offset = 0;
    min = max = slut[1];
    for (i = 0; i < numentries; i++) {
	if (slut[i] < min)
	    min = slut[i];
	if (slut[i] > max)
	    max = slut[i];
    }
    if (min < 0)
	offset = -min;
    delta = max - min;
    fprintf(stderr, "min = %d, max = %d, delta = %d\n", min, max, delta);

    for (i = 0; i < MAXBYTE; i++) {
	if (i >= lower_bound && i <= upper_bound)
	    scaled_val = (unsigned char) ((float) (slut[i] + offset)
					  / (float) delta * (float) MAXBYTE);
	else if (fill)
	    scaled_val = fill_value;
	else
	    scaled_val = i;
	fprintf(stdout, "%d\n", scaled_val);
    }
}

void 
DoScaling_int()
{

/* search table, find min and max, build a new blut based on min and max */
    int       min, max, delta, offset;
    unsigned char scaled_val;
    int       i;

    offset = 0;
    min = max = ilut[1];
    for (i = 0; i < numentries; i++) {
	if (ilut[i] < min)
	    min = ilut[i];
	if (ilut[i] > max)
	    max = ilut[i];
    }
    if (min < 0)
	offset = -min;
    delta = max - min;
    fprintf(stderr, "min = %d, max = %d, delta = %d\n", min, max, delta);

    for (i = 0; i < MAXBYTE; i++) {
	if (i >= lower_bound && i <= upper_bound)
	    scaled_val = (unsigned char) ((float) (ilut[i] + offset)
					  / (float) delta * (float) MAXBYTE);
	else if (fill)
	    scaled_val = fill_value;
	else
	    scaled_val = i;
	fprintf(stdout, "%d\n", scaled_val);
    }
}

void 
DoScaling_float()
{

/* search table, find min and max, build a new blut based on min and max */
    float     min, max, delta, offset;
    unsigned char scaled_val;
    int       i;

    offset = 0.0;
    min = max = flut[1];
    for (i = 0; i < numentries; i++) {
	if (flut[i] < min)
	    min = flut[i];
	if (flut[i] > max)
	    max = flut[i];
    }
    if (min < 0.0)
	offset = -min;
    delta = max - min;
    fprintf(stderr, "min = %f, max = %f, delta = %f\n", min, max, delta);

    for (i = 0; i < MAXBYTE; i++) {
	if (i >= lower_bound && i <= upper_bound)
	    scaled_val = (unsigned char) ((float) (flut[i] + offset)
					  / (float) delta * (float) MAXBYTE);
	else if (fill)
	    scaled_val = fill_value;
	else
	    scaled_val = i;
	fprintf(stdout, "%d\n", scaled_val);
    }
}

void 
usage()
{
    fprintf(stderr, "usage: scaletbl [-l lower_bound] [-u upper_bound] < source_table > dest_table\n");
    exit(-1);
}
