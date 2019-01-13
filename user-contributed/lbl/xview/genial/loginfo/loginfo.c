/*
 * loginfo.c                   -Brian Tierney, LBL
 *
 * loginfo.c  -print info from genial log
 *
 * Usage:
 *
 * Load:	cc -o loginfo loginfo.c -lhips
 *
 */

#include <hipl_format.h>
#include <stdio.h>
#include <strings.h>

#include "reg.h"
#include "ui.h"

#define HEADER_LOG_DATA_NAME "genial-log"

char     *parse_logrec(), *parse_op(), *parse_id(), *parse_fnum(),
         *parse_regrec(), *parse_plist(), *parse_auxd(), *parse_text();
char     *fstring(), *my_formatheader();
u_char    readhex();

int       trace_flg, histo_flg, ang_flg, anot_flg, dist_flg, zoom_flg;
char     *filename;

main(argc, argv)
    int       argc;
    char    **argv;

{
    struct header hd;
    int       rval;
    FILE     *fp;
    void      parse_args();

    Progname = strsave(*argv);
    parse_args(argc, argv);

    fp = hfopenr(filename);
    fread_header(fp, &hd, filename);
    fprintf(stderr, "%s", my_formatheader(&hd));

    fprintf(stderr, "\nGenial log information: \n");
    read_log(&hd);

    return (0);
}

/****************************************************************************/
/*
 * The following routines are for parsing / reading logs
 *
 */

read_log(head)
    struct header *head;
{
    int       count = 1000;	/* any large number */
    char     *log;

    if (findparam(head, HEADER_LOG_DATA_NAME) == NULLPAR)
	return;

    getparam(head, HEADER_LOG_DATA_NAME, PFASCII, &count, &log);

    if (log != NULL) {
	parse_log(log);
    }
}


/****************************************************************************/
/* read a hex string and return a single byte */
u_char
readhex(str)
    char     *str;
{
    register u_char val = 0;

    if (isdigit(*str))
	val = (*str - '0') * 16;
    else {
	if (islower(*str))
	    val = (*str - 'a' + 10) * 16;
    }
    str++;
    if (isdigit(*str))
	val += (*str - '0');
    else {
	if (islower(*str))
	    val += (*str - 'a' + 10);
    }
#ifdef DEBUG
    printf("readhex: %d\n", (int) val);
#endif
    return val;
}

/****************************************************************************/
/* routine to find the first occurence of string s2 in s1 */
char     *
fstring(s1, s2)
    char     *s1, *s2;
{
    char     *tmp;

    tmp = index(s1, *s2);
    while (tmp != NULL) {
	if (strncmp(tmp, s2, strlen(s2)) == 0) {
	    return tmp;
	} else {
	    tmp = index((char *) tmp + 1, *s2);
	}
    }
    return tmp;
}

/*********************************************************************/
char     *hstring, *hsptr;
int       msglen;
int       hstrlen = 0;
/*********************************************************************/
/* modified version of HIPS2 library routine  */
char     *
my_formatheader(h)
    struct header *h;
{
    hsptr = hstring;
    msglen = 0;
    addstr("Original name:\t\t\t");
    addstr(h->orig_name);
    addstr("Sequence name:\t\t\t");
    addstr(h->seq_name);
    addstr("Number of frames:\t\t");
    adddec(h->num_frame);
    if (h->numcolor > 1) {
	addstr("\nNumber of color frames:\t\t");
	adddec(h->num_frame / h->numcolor);
    }
    addstr("\n");
    addstr("Number of rows:\t\t");
    adddec(h->orows);
    addstr("\nNumber of columns:\t");
    adddec(h->ocols);
    if (h->pixel_format == PFTOSPACE)
	addstr("\nFormat:\t\t\t\t");
    else
	addstr("\nPixel format:\t\t\t");
    addstr(hformatname(h->pixel_format));
    addstr("\nNumber of color planes:\t\t");
    adddec(h->numcolor);
    addstr("\n");
    if (h->sizedesc > 1) {
	addstr("Sequence Description:\n\n");
	addstr(h->seq_desc);
	addstr("\n");
    } else
	addstr("No sequence description\n");
    *hsptr = 0;
    return (hstring);
}

/****************************************************************************/
addstr(s)
    char     *s;

{
    int       i, incr;
    char     *tmp;

    i = strlen(s);
    if (i + msglen + 1 > hstrlen) {
	incr = (512 > (i + 1)) ? 512 : (i + 1);
	tmp = memalloc(hstrlen + incr, sizeof(char));
	if (hstrlen) {
	    strcpy(tmp, hstring);
	    free(hstring);
	}
	hstrlen += incr;
	hstring = tmp;
	hsptr = hstring + msglen;
    }
    strcpy(hsptr, s);
    msglen += i;
    hsptr += i;
}

/****************************************************************************/
adddec(i)
    int       i;

{
    char      s[20];

    sprintf(s, "%d", i);
    addstr(s);
}

/****************************************************************/
void
parse_args(argc, argv)
    int       argc;
    char     *argv[];
{
    void      usageterm();

    filename = NULL;
    trace_flg = histo_flg = ang_flg = anot_flg = dist_flg = zoom_flg = 0;

    /* Interpret options  */
    while (--argc > 0 && (*++argv)[0] == '-') {
	char     *s;
	for (s = argv[0] + 1; *s; s++)
	    switch (*s) {
	    case 'i':
		if (argc < 2)
		    usageterm();
		filename = *++argv;
		argc--;
		break;
	    case 'T':
		trace_flg++;
		break;
	    case 'H':
		histo_flg++;
		break;
	    case 'A':
		ang_flg++;
		break;
	    case 'N':
		anot_flg++;
		break;
	    case 'Z':
		zoom_flg++;
		break;
	    case 'h':
		usageterm();
		break;
	    default:
		usageterm();
		break;
	    }
    }				/* while */

    /* if none selected, then select all */
    if (trace_flg == 0 && histo_flg == 0 && ang_flg == 0 &&
	anot_flg == 0 && dist_flg == 0 && zoom_flg == 0)
	trace_flg = histo_flg = ang_flg = anot_flg = dist_flg = zoom_flg = 1;

    if (filename == NULL) {
	filename = malloc(15);
	strcpy(filename, "<stdin>");
    }
}

/****************************************************************************/
void
usageterm()
{
    fprintf(stderr, "loginfo options: (default is all of the following) \n");
    fprintf(stderr, "   [-T]  trace info \n");
    fprintf(stderr, "   [-H]  histogram info \n");
    fprintf(stderr, "   [-A]  angle measure info \n");
    fprintf(stderr, "   [-D]  distance info \n");
    fprintf(stderr, "   [-Z]  zoom info \n");
    fprintf(stderr, "   [-N]  image annotation info \n");
    fprintf(stderr, "   [-i filename] input file default is stdin \n\n");
    exit(0);
}
