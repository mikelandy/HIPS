
/* applylut.c  : read a lookup table and apply it to a hips images
 *
 * usage: applylut lut_file < inseq > outseq 
 *
 *
 * to load: cc -o applylut applylut.c -lhips 
 *
 * Brian Tierney, LBL  10/90
 *
 * format of the look-up table file:
 *    NNNN: number of entries in the lookup table 
 *           (usually will be 256 or 65536, but could be anything
 *    list of lut values  (integers less than 65536)
 */

#include <hipl_format.h>
#include <stdio.h>
#include <sys/types.h>

u_char   *pic;
u_short  *spic;

#define MAXSHORT 65536

int    *lut;
int apply_byte_lut(),apply_short_lut();

int main(argc, argv)
    int       argc;
    char     *argv[];
{
    struct header hd;
    int       form, ival, lut_size, i;
    FILE     *fp;
    char *lut_file;
    void usageterm();

    Progname = strsave(*argv);
    if (argc < 2 || (strcmp(argv[1],"-h") == 0))
	usageterm();

    lut_file = argv[1];

    if ((fp = fopen(lut_file, "r")) == NULL) {
        fprintf(stderr, "\nError opening look-up table image file: %s \n\n",
		lut_file);
        exit(-1);
    }
    
    if (fscanf(fp, "%d", &lut_size) < 1) {
	fprintf(stderr, "Error: invalid look-up table file format \n\n");
	exit(0);
    }
    lut = (int *) halloc(lut_size, sizeof(int));

    for (i=0; i< lut_size; i++) {
	if (fscanf(fp,"%d", &ival) < 1) {
	    fprintf(stderr, "Error: invalid look-up table file format \n\n");
	    exit(0);
	}
	lut[i] = ival;
    }
    
    read_header(&hd);
    form = hd.pixel_format;
    if (form == PFBYTE) {
	pic = (u_char *) halloc(hd.orows * hd.ocols, sizeof(char));
    } else if (form == PFSHORT) {
	spic = (u_short *) halloc(hd.orows * hd.ocols, sizeof(short));
    } else
	perr(HE_MSG,"input format must be byte or short");

    update_header(&hd, argc, argv);
    write_header(&hd);

    if (form == PFBYTE)
	return(apply_byte_lut(hd.num_frame, hd.orows, hd.ocols));
    if (form == PFSHORT)
	return(apply_short_lut(hd.num_frame, hd.orows, hd.ocols));

    return (0);
}

/*******************************************************************/
int apply_byte_lut(fr, r, c)
    int       fr, r, c;
{
    int       j, i, rc;

    rc = r * c;
    for (j = 0; j < fr; j++) {
	if (fread(pic, rc * sizeof(u_char),1,stdin) != 1)
	    perr(HE_MSG,"error during read");

	for (i = 0; i < rc; i++)
	    pic[i] = (u_char)lut[pic[i]];

	if (fwrite(pic, rc * sizeof(char),1,stdout) != 1)
	    perr(HE_MSG,"error during write");
    }
    return (0);
}

/*******************************************************************/
int apply_short_lut(fr, r, c)
    int       fr, r, c;
{
    int       j, i, rc;

    rc = r * c;
    for (j = 0; j < fr; j++) {
	if (fread(spic, rc * sizeof(u_short),1,stdin) != 1)
	    perr(HE_MSG,"error during read");

	for (i = 0; i < rc; i++) {
	    spic[i] = (u_short)lut[spic[i]];
	}

	if (fwrite(pic, rc * sizeof(short),1,stdout) != 1)
	    perr(HE_MSG,"error during write");

    }
    return (0);
}

/******************************************************/
void usageterm()
{
    fprintf(stderr, "Usage: applylut lut_file < inseq > outseq \n\n");
    exit(0);
}

