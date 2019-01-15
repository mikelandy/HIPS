#include "cdjpeg.h"	/* Common decls for cjpeg/djpeg applications */
#include "jversion.h"

GLOBAL	struct	jpeg_error_mgr	jerr;

#define	JMESSAGE(code,string)	string ,

static const char * const cdjpeg_message_table[] = {
#include "cderror.h"	/* create message string table */
	NULL
};

/* Add some application-specific error messages (from cderror.h) */

error_init()
{
	jerr.addon_message_table = cdjpeg_message_table;
	jerr.first_addon_message = JMSG_FIRSTADDONCODE;
	jerr.last_addon_message = JMSG_LASTADDONCODE;
}

char*
get_error_msg(struct jpeg_error_mgr *err, int msg_code)
{
char*	msgtext = NULL;
  /* Look up message string in proper table */
  if (msg_code > 0 && msg_code <= err->last_jpeg_message) {
    msgtext = err->jpeg_message_table[msg_code];
  } else if (err->addon_message_table != NULL &&
	     msg_code >= err->first_addon_message &&
	     msg_code <= err->last_addon_message) {
    msgtext = err->addon_message_table[msg_code - err->first_addon_message];
  }

  /* Defend against bogus message number */
  if (msgtext == NULL) {
    err->msg_parm.i[0] = msg_code;
    msgtext = err->jpeg_message_table[0];
  }
return	msgtext;
}
