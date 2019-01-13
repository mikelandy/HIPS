/* Error handling routine. */

#include <stdio.h>
#include <errno.h>
#include <X11/Intrinsic.h>
#include <xdisp.h>


/***********************************
 * xdisp_error()
 ***********************************/

/* Print a message and qualifier. Qualifier, if present, is normally a 
   variable such as a file name. If extra_info is true, then there should
   be information available from Unix via the system errno.
*/

void xdisp_error(message,qualifier,extra_info)
  char *message;
  char *qualifier;
  int  extra_info;
{
    char s[256];
    int err = errno;

    sprintf(s,"%s: %s %s",PROGRAM_NAME,message,qualifier);

    XBell(dpy,0);

    if (extra_info) {
	errno = err;
	perror(s);
	}
    else 
	fprintf(stderr,"%s\n",s);
}


