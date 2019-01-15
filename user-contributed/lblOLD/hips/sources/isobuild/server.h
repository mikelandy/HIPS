
/* server.h   : definitions for server part of isobuild */

/* David Robertson/Brian Tierney   LBL  */

/* $Id: server.h,v 1.3 1992/01/31 02:06:50 tierney Exp $ */

/* $Log: server.h,v $
 * Revision 1.3  1992/01/31  02:06:50  tierney
 * *** empty log message ***
 *
 * Revision 1.2  1991/12/19  01:42:20  davidr
 * added RCS identification markers
 * */

#define SERV_DEBUG

#define FILEPROC 1
#define PARAMPROC 2
#define MOVIEPROC 3
#define HIGHRESPROC 4

#define SUNINT 4
#define FILEPROC_READ_LENGTH 44
#define FILEPROC_WRITE_LENGTH 4

#define PARAM_READ_LENGTH 18
#define MOVIE_READ_LENGTH 36

#define HIGHRES_WRITE_LENGTH 4
#define HIGHRES_READ_LENGTH 32

