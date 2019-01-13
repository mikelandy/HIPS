
/*
**             Copyright (c) 1991 The Turing Institute
**
** Disclaimer:  No guarantees of performance accompany this software,
** nor is any responsibility assumed on the part of the authors.  All the
** software has been tested extensively and every effort has been made to
** insure its reliability.
**
*/


/*
**
** Filename: pa_externs.c
**
** Description:	
**
**    External variables used by the HIPS argument parser
**
** Author:  David Wilson, Turing Institute, 30/1/91.
**
*/

#include <hipl_format.h>

Flag_Key	*flag_table;		/* All flag data. */
int		num_flags;

Filename_Ptr	filename_ptr;		/* All image file data. */
Filename_Format	filename_format;
char		*filename_usage = (char *) 0;
