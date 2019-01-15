/*
 * logio.c -- routines for reading and writing logs on top of the image
 *
 */

#include <stdio.h>
#include <strings.h>

#include "common.h"
#include "llist.h"
#include "display.h"
#include "ui.h"
#include "reg.h"
#include "log.h"

#define HEADER_LOG_DATA_NAME "genial-log"
#define LOG_SIZE 1000		/* allocate log buffer in increments of this
				 * size */

int       load_log = 0;

char     *parse_logrec(), *parse_op(), *parse_id(), *parse_fnum(), *parse_regrec(),
         *parse_plist(), *parse_auxd(), *parse_text();

/***********************************************************************/
add_log(head, loglist)
    struct header *head;
    struct logent *loglist;
{
    struct logent *tmp;
    char      tstr[256];
    char     *buffer;
    int       log_size;

    if (loglist == NULL)
	return;

    buffer = (char *)calloc(LOG_SIZE, 1);
    log_size = LOG_SIZE;

    strcpy(buffer, "GENLOG B\n");
    for (tmp = loglist; tmp != NULL && tmp->reg != NULL; tmp = tmp->next) {
	strcat(buffer, "LOGREC B\n");
	sprintf(tstr, "OPCODE %d\n", tmp->opcode);
	strcat(buffer, tstr);
	sprintf(tstr, "ID %d\n", tmp->id);
	strcat(buffer, tstr);
	sprintf(tstr, "FNUM %d\n", tmp->fnum);
	strcat(buffer, tstr);
	add_reg(buffer, tmp->reg);
	if (tmp->auxdsize != 0) {
	    sprintf(tstr, "AUXDSIZE %u\n", tmp->auxdsize);
	    strcat(buffer, tstr);
	    add_auxd(buffer, tmp->auxdsize, tmp->auxdata);
	}
	strcat(buffer, "LOGREC E\n");

	if (strlen(buffer) > log_size - 300) {
	    log_size += LOG_SIZE;
	    buffer = (char *)realloc(buffer, log_size);
	}
    }
    strcat(buffer, "GENLOG E\n");

    setparam(head, HEADER_LOG_DATA_NAME, PFASCII, strlen(buffer) + 1, buffer);

#ifdef DEBUG
    printf("log: %s", buffer);
#endif
}

/***********************************************************************/
/* add a region to a header */
add_reg(buffer, reg)
    char     *buffer;
    struct region *reg;
{
    char      tstr[256];

    strcat(buffer, "REGREC B\n");
    sprintf(tstr, "REGTYPE %d\n", reg->r_type);
    strcat(buffer, tstr);
    sprintf(tstr, "REGFLAGS %d\n", reg->r_flags);
    strcat(buffer, tstr);
    if (reg->r_type != NOREG) {
	add_plen(buffer, reg->r_plen);
	add_plist(buffer, reg->r_plist);
	if (reg->r_type == AN_TEXT)
	    add_text(buffer, reg->r_sbs);
    }
    strcat(buffer, "REGREC E\n");
}

/***********************************************************************/
/* add length of plist vector to log buffer */
add_plen(buffer, len)
    char     *buffer;
    int       len;
{
    char      tstr[256];

    sprintf(tstr, "PLEN %d\n", len);
    strcat(buffer, tstr);
}


/***********************************************************************/
/* add a point list (plist) to log buffer */
add_plist(buffer, ptlist)
    char     *buffer;
    struct plist *ptlist;
{
    struct plist *trav;
    char      tstr[256];

    strcat(buffer, "PLIST B\n");
    for (trav = ptlist; trav != NULL; trav = trav->next) {
	strcat(buffer, "PLREC B\n");
	sprintf(tstr, "%4d %4d\n", trav->pt.x, trav->pt.y);
	strcat(buffer, tstr);
	strcat(buffer, "PLREC E\n");
    }
    strcat(buffer, "PLIST E\n");
}

/***********************************************************************/
/* add a text string to log buffer */
add_text(buffer, sbs)
    char     *buffer;
    struct strbs *sbs;
{
    char      tstr[256];

    sprintf(tstr, "ATEXT %s\n", sbs->string);
    strcat(buffer, tstr);
}

/***********************************************************************/
/* add a auxiliary data to a bufferer */
add_auxd(buffer, len, data)
    char     *buffer;
    u_long    len;
    u_char   *data;
{
    char      tstr[256], hexstr[8];
    u_long    count = 0, index = 0;

    while (count < len) {
	bzero(tstr, 256);
	for (index = 0; (index <= 23) && (count < len); index++, count++) {
	    sprintf(hexstr, "%2x ", data[count]);
	    (void) strcat(tstr, hexstr);
	}
	(void) strcat(tstr, "\n");
	strcat(buffer, tstr);
    }
}

/****************************************************************************/
/*
 * The following routines are for parsing / reading logs
 *
 */

read_log(head)
    struct header *head;
{
    int       count = 1000;  /* any large value */
    char     *log;

    if (findparam(head, HEADER_LOG_DATA_NAME) == NULLPAR)
	return;

    getparam(head, HEADER_LOG_DATA_NAME, PFASCII, &count, &log);
    load_log = 1;
    if (log != NULL) {
	parse_log(log);
    }
    load_log = 0;
}

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
    fxn_select(0);
}

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
	    rhead = parse_op(rhead, curfunc);
	    break;
	case 1:
	    rhead = parse_id(rhead, curfunc);
	    break;
	case 2:
	    rhead = parse_fnum(rhead, curfunc);
	    break;
	case 3:
	    rhead = parse_regrec(rhead, curfunc);
	    break;
	case 4:
	    rhead = parse_auxd(rhead, curfunc);
	    break;
	case 5:
	    rhead += strlen("LOGREC E\n");
	    if (curfunc->fnum == curframe()) {
	        build_dreg(curfunc->reg);
		fxn_select(curfunc->opcode);
		fxn_init();
		fxn_eval();
	    }
	    log_perm();
	    return rhead;
	}
    }
}

char     *
parse_op(rhead, log)
    char     *rhead;
    struct logent *log;
{
    char     *tmp;

    rhead += strlen("OPCODE");
    log->opcode = atoi(rhead);
    tmp = index(rhead, '\n');
    rhead = tmp + 1;

    return rhead;
}

char     *
parse_id(rhead, log)
    char     *rhead;
    struct logent *log;
{
    char     *tmp;

    rhead += strlen("ID");
    log->id = atoi(rhead);
    tmp = index(rhead, '\n');
    rhead = tmp + 1;
    return rhead;
}

char     *
parse_fnum(rhead, log)
    char     *rhead;
    struct logent *log;
{
    char     *tmp;

    rhead += strlen("FNUM");
    log->fnum = atoi(rhead);
    tmp = index(rhead, '\n');
    rhead = tmp + 1;
    return rhead;
}

char     *
parse_regrec(rhead, log)
    char     *rhead;
    struct logent *log;
{
    struct region *reg;
    char     *tmp;
    int       type;

    rhead += strlen("REGREC B\n");
    rhead += strlen("REGTYPE");
    type = atoi(rhead);
    setplim(MAXPOINTS);
    reg = newreg(type);
    tmp = index(rhead, '\n');
    rhead = tmp + 1;
    rhead += strlen("REGFLAGS");
    reg->r_flags = atoi(rhead);
    tmp = index(rhead, '\n');
    rhead = tmp + 1;
    if (type != NOREG) {
	free(reg->r_plist);
	rhead += strlen("PLEN");
	reg->r_plen = atoi(rhead);
	tmp = index(rhead, '\n');
	rhead = tmp + 1;
	if ((reg->r_plist = (struct plist *)
	     malloc(sizeof(struct plist *))) == NULL) {
	    perror("malloc");
	    exit(1);
	}
	rhead = parse_plist(rhead,log);
	flush_cpl(reg);
    }
    if (type == AN_TEXT) {
	rhead = parse_text(rhead);
    }
    rhead += strlen("REGREC E\n");
    log->reg = reg;
    return rhead;
}

char     *
parse_plist(rhead,log)
    char     *rhead;
    struct logent *log;
{
    XPoint    pt;

    rhead += strlen("PLIST B\n");
    while (strncmp(rhead, "PLREC B\n", strlen("PLREC B\n")) == 0) {
	rhead += strlen("PLREC B\n");
	pt.x = (short) atoi(rhead);
	rhead += 4;		/* format is %4d */
	pt.y = (short) atoi(rhead);
	rhead += 4;		/* format is %4d */
	if (log->fnum == curframe())
	    add_point(pt.x, pt.y);
	else
	    add_log_point(pt.x, pt.y);
	rhead = fstring(rhead, "PLREC E\n");
	rhead += strlen("PLREC E\n");
    }
    rhead += strlen("PLIST E\n");
    return rhead;
}

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
    add_string(tstr);
    rhead = tmp + 1;
    return rhead;
}

char     *
parse_auxd(rhead, log)
    char     *rhead;
    struct logent *log;
{
    char     *tmp;
    unsigned char *data;
    int       count = 0, idx;

    rhead += strlen("AUXDSIZE");
    log->auxdsize = atoi(rhead);
    tmp = index(rhead, '\n');
    rhead = tmp + 1;
    if ((log->auxdata = (unsigned char *) malloc(log->auxdsize)) == NULL) {
	perror("malloc");
	exit(0);
    }
    data = (unsigned char *) log->auxdata;
    while (count < log->auxdsize) {
	for (idx = 0; (idx <= 23) && (count < log->auxdsize); idx++, count++) {
	    *data++ = readhex(rhead);
	    rhead += 3;
	}
	tmp = index(rhead, '\n');
	rhead = tmp + 1;
    }
    return rhead;
}
