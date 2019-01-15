
/*
 * parse.c
 */


#include <hipl_format.h>
#include <stdio.h>
#include <strings.h>

#include "reg.h"
#include "ui.h"

char     *parse_logrec(), *parse_op(), *parse_id(), *parse_fnum(),
         *parse_regrec(), *parse_plist(), *parse_auxd(), *parse_text();
char     *fstring(), *my_formatheader();
u_char    readhex();

/* global command line args */
extern int trace_flg, histo_flg, ang_flg, anot_flg, dist_flg, zoom_flg;

/* other globals */
int opcode, reg_type, log_id, frame, r_flag, skip_log_entry;

/****************************************************************************/
/* routine to parse a log given in a particular string */
parse_log(lstr)
    char     *lstr;
{
    char     *rhead;		/* a pointer to the current position in the
				 * reading frame */

    rhead = fstring(lstr, "GENLOG B\n");
    if (rhead == NULL) {
	return;
    }
    /*
     * basic algorithm is to look for matching "LOGREC B" and "LOGREC E"
     * tokens. When we find one, move one level through the call stack
     */
    rhead = fstring(rhead, "LOGREC B\n");
    while (rhead != NULL) {
	rhead = parse_logrec((char *) rhead + strlen("LOGREC B\n"));
	rhead = fstring(rhead, "LOGREC B\n");
    }
}

/****************************************************************************/
/*routine to parse individual log records and advance the read head */
char     *
parse_logrec(rhead)
    char     *rhead;
{
    int       i;
    static char *tokens[] =
    {
	"OPCODE", "ID", "FNUM", "REGREC", "AUXDSIZE", "LOGREC", NULL
    };

    while (1) {
	for (i = 0; tokens[i] != NULL; i++) {
	    if (strncmp(rhead, tokens[i], strlen(tokens[i])) == 0) {
		break;
	    }
	}
	switch (i) {
	case 0:
	    rhead = parse_op(rhead);
	    break;
	case 1:
	    rhead = parse_id(rhead);
	    break;
	case 2:
	    rhead = parse_fnum(rhead);
	    break;
	case 3:
	    rhead = parse_regrec(rhead);
	    break;
	case 4:
	    rhead = parse_auxd(rhead);
	    break;
	case 5:
	    rhead += strlen("LOGREC E\n");
	    return rhead;
	}
    }
}

/****************************************************************************/
char     *
parse_op(rhead)
    char     *rhead;
{
    char     *tmp;
    char      msg[80];

    rhead += strlen("OPCODE");
    opcode = atoi(rhead);
    tmp = index(rhead, '\n');
    rhead = tmp + 1;

    return rhead;
}

char     *
parse_id(rhead)
    char     *rhead;
{
    char     *tmp;

    rhead += strlen("ID");
    log_id = atoi(rhead);

    tmp = index(rhead, '\n');
    rhead = tmp + 1;
    return rhead;
}

/****************************************************************************/
char     *
parse_fnum(rhead)
    char     *rhead;
{
    char     *tmp;

    rhead += strlen("FNUM");
    frame = atoi(rhead);
    tmp = index(rhead, '\n');
    rhead = tmp + 1;

    return rhead;
}

/****************************************************************************/
char     *
parse_regrec(rhead)
    char     *rhead;
{
    char     *tmp;
    char      msg[80];
    int       i, r_plen;

    rhead += strlen("REGREC B\n");
    rhead += strlen("REGTYPE");
    reg_type = atoi(rhead);


    tmp = index(rhead, '\n');
    rhead = tmp + 1;
    rhead += strlen("REGFLAGS");
    r_flag = atoi(rhead);

    /* ouput header line at this point */
    print_header_line();

    tmp = index(rhead, '\n');
    rhead = tmp + 1;
    if (reg_type != NOREG) {
	rhead += strlen("PLEN");
	r_plen = atoi(rhead);

	tmp = index(rhead, '\n');
	rhead = tmp + 1;
	rhead = parse_plist(rhead, reg_type);
    }
    if (reg_type == AN_TEXT) {
	rhead = parse_text(rhead);
    }
    rhead += strlen("REGREC E\n");
    return rhead;
}

/****************************************************************************/
print_header_line()
{
    char msg[80];

    skip_log_entry = 0;

    switch (opcode) {
    case LINE_TRACE:
	if (!trace_flg)
	    skip_log_entry++;
	sprintf(msg, "%s", "Line Trace");
	break;
    case HISTOGRAM:
	if (!histo_flg)
	    skip_log_entry++;
	sprintf(msg, "%s", "Histogram");
	break;
    case ZOOM:
	if (!zoom_flg)
	    skip_log_entry++;
	sprintf(msg, "%s", "Zoom");
	break;
    case DISTANCE:
	if (!dist_flg)
	    skip_log_entry++;
	sprintf(msg, "%s", "Distance");
	break;
    case ANGLE_MES:
	if (!ang_flg)
	    skip_log_entry++;
	sprintf(msg, "%s", "Angle Measure");
	break;
    case ANNOTATE:
	if (!anot_flg)
	    skip_log_entry++;
	sprintf(msg, "%s", "Annotate");
	break;
    }
    
    if (skip_log_entry)
	return;

    fprintf(stderr, "Frame: %d;  Log id: %d;  ", frame,log_id);
    fprintf(stderr, "Function: %s;  ", msg);

    switch (reg_type) {
    case NOREG:
	sprintf(msg, "%s", "None");
	break;
    case LINE:
	sprintf(msg, "%s", "Line");
	break;
    case SPLINE:
	sprintf(msg, "%s", "Spline");
	break;
    case CLSPLINE:
	sprintf(msg, "%s", "Closed Spline");
	break;
    case POLYGON:
	sprintf(msg, "%s", "Polygon");
	break;
    case BOX:
	sprintf(msg, "%s", "Box");
	break;
    case DUBLIN:
	sprintf(msg, "%s", "Angle Measure");
	break;
    case AN_TEXT:
	sprintf(msg, "%s", "Annotation Text");
	break;
    case AN_VEC:
	sprintf(msg, "%s", "Annotation Vector");
	break;
    default:
	sprintf(msg, "%s", "Unknown");
	break;
    };

    fprintf(stderr, "Region Type: %s; ", msg);

    if (r_flag == LSQ)
	fprintf(stderr, "(Least Square Mode); ");

    fprintf(stderr, "\n");
}

/****************************************************************************/
char     *
parse_plist(rhead)
    char     *rhead;
{
    XPoint    pt, plist[4];
    double    angle, dist;
    double compute_angle(), distance();
    int       i = 0;

    rhead += strlen("PLIST B\n");
    while (strncmp(rhead, "PLREC B\n", strlen("PLREC B\n")) == 0) {
	rhead += strlen("PLREC B\n");
	pt.x = (short) atoi(rhead);
	rhead += 4;		/* format is %4d */
	pt.y = (short) atoi(rhead);
	rhead += 4;		/* format is %4d */

	if (!skip_log_entry)
	    fprintf(stderr, " (%d,%d) ", pt.x, pt.y);

	rhead = fstring(rhead, "PLREC E\n");
	rhead += strlen("PLREC E\n");

	if (reg_type == DUBLIN || reg_type == LINE)
	    plist[i++] = pt;
    }
    if (!skip_log_entry) {
	fprintf(stderr, ";  ");

	if (opcode == ANGLE_MES) {
	    angle = compute_angle(plist[0], plist[1], plist[2], plist[3]);
	    fprintf(stderr, "angle =  %.3f radians", (float) angle);
	}
	if (opcode == LINE_TRACE) {
	    dist = distance(plist[0], plist[1]);
	    fprintf(stderr, "dist =  %.3f ", (float) dist);
	}
	fprintf(stderr, ";\n");
    }

    rhead += strlen("PLIST E\n");
    return rhead;
}

/****************************************************************************/
char     *
parse_text(rhead)
    char     *rhead;
{
    char     *tmp;
    char      tstr[256];
    int       len;

    rhead += strlen("ATEXT") + 1;
    tmp = index(rhead, '\n');

    len = (int) (tmp - rhead);
    bcopy(rhead, tstr, (unsigned) len);
    tstr[len++] = '\0';
    /* tstr now contains the string */
    /*
     * add string is a custom tailored version of add_char() for efficient
     * loading of strings.  see regions/text.c
     */

    fprintf(stderr, "Text: %s;\n", tstr);

    rhead = tmp + 1;
    return rhead;
}

char     *
parse_auxd(rhead)
    char     *rhead;
{
    char     *tmp;
    unsigned char *data, *auxdata;
    int       count = 0, idx, auxdsize;

    rhead += strlen("AUXDSIZE");
    auxdsize = atoi(rhead);
    tmp = index(rhead, '\n');
    rhead = tmp + 1;
    if ((auxdata = (unsigned char *) malloc(auxdsize)) == NULL) {
	perror("malloc");
	exit(0);
    }
    data = (unsigned char *) auxdata;
    while (count < auxdsize) {
	for (idx = 0; (idx <= 23) && (count < auxdsize); idx++, count++) {
	    *data++ = readhex(rhead);
	    rhead += 3;
	}
	tmp = index(rhead, '\n');
	rhead = tmp + 1;
    }
    if (strlen(data) > 0) {
	fprintf(stderr, "Aux trace data: grad: %d, min: %d, max: %d; \n",
		(int) data[0], (int) data[1], (int) data[2]);
    }
    return rhead;
}
