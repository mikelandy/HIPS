
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
** Filename: pa_usage.c
**
** Description:	
**
**    This file contains the usage message source code for the
**    HIPS argument parser.
**
** Author:  David Wilson, Turing Institute, 30/1/91.
** Changed to dynamic memory allocation for the usage message - msl - Jan 6 1995
**
*/


/*
** Specify include files.
*/


#include <stdio.h>		/* UNIX standard I/O header file. */
#include <hipl_format.h>	/* HIPS main header file. */

/*
** Define macros for the usage message.
*/

#define MAX_LINE_SIZE   75	/* Max characters per line in usage message */
#define USAGE_INCR  1000	/* Allocation increment of usage message */
#define	GRPUSAGE_INCR	200	/* Allocation incr for one flag group */

/*
** Define a store for the usage message.
*/

char  *usage,*grpusage;
static h_boolean usagealloc = FALSE,groupalloc = FALSE;
static int usageleft,usagelen,groupleft,grouplen;
static int currlinelen;
int usagecat(),groupcat();

/*
**
** Name: build_usage_message
**
** Purpose: To build a usage message describing the flags in the flag
**	    table.
**
** Inputs: None.
**
** Return Value: Error indication.
**
** External Dependencies:
**    hipl_format.h - HIPS argument parser header file.
**
*/


int build_usage_message ()

{

    /*
    ** External data.
    */

    extern int	     num_flags;
    extern Flag_Key  *flag_table;


    /*
    ** Local functions and variables.
    */

    int		 index,add_group_to_usage(),add_filenames_to_usage();

    void	 unlock_all_flags ();

    Flag_Key	 *flag_ptr;


    /*
    ** Clear the mutex locks to start with.
    */

    unlock_all_flags ();

    /*
    ** Initialize usage memory
    */

    if (usagealloc) {
	usageleft = usagelen - 1;
	usage[0] = 0;
    }
    else {
	if ((usage = memalloc(USAGE_INCR,sizeof(char))) == (char *) HIPS_ERROR)
		return(HIPS_ERROR);
	usage[0] = 0;
	usageleft = USAGE_INCR - 1;
	usagelen = USAGE_INCR;
    }

    /*
    ** Add the filter name.
    */

    (void) sprintf (usage, "usage: %s", Progname);
    currlinelen = strlen(usage);
    usageleft -= currlinelen;

    /*
    ** Cycle through the flag table and if the mutually exclusive group
    ** of flags to which this flag belongs has not already been added,
    ** add it now.
    */

    flag_ptr = flag_table;

    for (index = 0; index < num_flags;  index++)
    {
	if (flag_ptr->locked == FALSE)
	{
	    if (add_group_to_usage (flag_ptr) == HIPS_ERROR)
		return(HIPS_ERROR);
	}

	flag_ptr++;
    }

    /*
    ** Finally add trailing file names if any are allowed.
    */

    if (add_filenames_to_usage () == HIPS_ERROR)
	return(HIPS_ERROR);

    if (usagecat("\n") == HIPS_ERROR)
	return(HIPS_ERROR);
    return(HIPS_OK);

}




/*
**
** Name: add_group_to_usage
**
** Purpose: To add a group of mutually exclusive flags and to the usage
**	    message. If the flag has no mutually exclusive flags, then 
**	    it is simply added on its own.
**
**	    Note that if the mutex "locked" signal for a flag is set to
**	    TRUE, the mutex group to which that flag belongs has already
**	    been added. Otherwise, it has still to be inserted into the
**	    usage message.
**
** Inputs: The following inputs are expected:
**
**		flag_ptr - Flag_Key *, a pointer to a flag whose mutex
**			   group is to be aded to the usage message.
**
** Return Value: Error indication.
**
** External Dependencies:
**    hipsl_parser.h - HIPS argument parser header file.
**
*/

int add_group_to_usage (flag_ptr)

Flag_Key  *flag_ptr;

{

    /*
    ** Functions called and local variables..
    */

    Flag      *mutex_list;
    Flag_Key  *mutex_flag, *find_flag ();
    int		len,add_flag_to_usage();

    /*
    ** allocate memory if needed
    */

    if (groupalloc) {
	groupleft = grouplen - 1;
	grpusage[0] = 0;
    }
    else {
	if ((grpusage = memalloc(GRPUSAGE_INCR,sizeof(char)))
	    == (char *) HIPS_ERROR)
		return(HIPS_ERROR);
	grpusage[0] = 0;
	groupleft = GRPUSAGE_INCR - 1;
	grouplen = GRPUSAGE_INCR;
    }

    /*
    ** Add the flag and then each of its mutually exclusive flags in
    ** turn. Signal that each flag in this group has now been locked out.
    */

    if (groupcat(" [") == HIPS_ERROR)
	return(HIPS_ERROR);

    if (add_flag_to_usage (flag_ptr->format) == HIPS_ERROR)
	return(HIPS_ERROR);
    flag_ptr->locked = TRUE;

    mutex_list = flag_ptr->format->mutex_flags;
    while (*mutex_list != LASTFLAG)
    {
	if (groupcat(" | ") == HIPS_ERROR)
		return(HIPS_ERROR);

	mutex_flag = find_flag (*mutex_list);
	if (add_flag_to_usage (mutex_flag->format) == HIPS_ERROR)
		return(HIPS_ERROR);
	mutex_flag->locked = TRUE;

	mutex_list++;
    }
    if (groupcat ("]") == HIPS_ERROR)
	return(HIPS_ERROR);
    len = strlen(grpusage);
    if (len+currlinelen > MAX_LINE_SIZE) {
	if (usagecat("\n\t\t") == HIPS_ERROR)
		return(HIPS_ERROR);
	currlinelen = 16;
    }
    if (usagecat(grpusage) == HIPS_ERROR)
	return(HIPS_ERROR);
    currlinelen += len;
    return(HIPS_OK);
}




/*
**
** Name: add_flag_to_usage
**
** Purpose: To add a flag and its associated parameters to the
**	    usage message.
**
** Inputs: The following inputs are expected:
**
**		flag_ptr - Flag_Format *, a pointer to the format
**			   descriptor for the flag which is to be
**			   added to the usage messgae.
**
** Return Value: Error indication.
**
** External Dependencies:
**    hipl_format.h - HIPS argument parser header file.
**
*/

int add_flag_to_usage (flag_ptr)

Flag_Format  *flag_ptr;

{

    /*
    ** Functions called and local variables.
    */

    char	 *get_ptype_text();

    int		 parameter_num, num_optional_parameters;
    int		 get_ptype_index ();

    void	 zero_ptype_counts();

    Parameter	 *parameter;

    /*
    ** First add the flag and then each of its  parameters.
    */

    if (groupcat("-") == HIPS_ERROR)
	return(HIPS_ERROR);
    if (groupcat(flag_ptr->value) == HIPS_ERROR)
	return(HIPS_ERROR);

    /*
    ** Ignore any leading boolean parameters.
    */

    parameter = flag_ptr->parameters;

    if (parameter->type == PTBOOLEAN)
    {
	parameter++;
    }

    /*
    ** First add the mandatory parameters.
    */

    for (parameter_num = 1; parameter_num <= flag_ptr->min_parameters; parameter_num++)
    {
	if (groupcat(" ") == HIPS_ERROR)
		return(HIPS_ERROR);
	if (groupcat(parameter->par_usage) == HIPS_ERROR)
		return(HIPS_ERROR);
	if (groupcat(get_ptype_text(parameter->type)) == HIPS_ERROR)
		return(HIPS_ERROR);
	parameter++;
    }

    /*
    ** Then add the optional parameters.
    */

    num_optional_parameters = 0;

    while (parameter->type != LASTPARAMETER)
    {
	if (groupcat(" [") == HIPS_ERROR)
		return(HIPS_ERROR);
	if (groupcat(parameter->par_usage) == HIPS_ERROR)
		return(HIPS_ERROR);
	if (groupcat(get_ptype_text(parameter->type)) == HIPS_ERROR)
		return(HIPS_ERROR);
	parameter++;
	num_optional_parameters++;
    }

    while (num_optional_parameters > 0)
    {
	if (groupcat("]") == HIPS_ERROR)
		return(HIPS_ERROR);
	num_optional_parameters--;
    }
    return(HIPS_OK);
}

/*
**
** Name: get_ptype_text
**
** Purpose: To retrieve the text descriptor to be included in the usage
**	    message for a given parameter type.
**
** Inputs: The following inputs are expected:
**
**		ptype - Parameter_Type, the parameter type of interest.
**
** Return Value: The routine returns a character string for the
**		 descriptor associated with the parameter type.
**
** External Dependencies:
**    hipsl_parser.h - HIPS argument parser header file.
**
*/


char *
get_ptype_text (ptype)

Parameter_Type	ptype;

{

    char  *string;


    switch (ptype)
    {

	case PTBOOLEAN:	  string = "<boolean>";
			  break;

	case PTCHAR:	  string = "<character>";
			  break;

	case PTSTRING:	  string = "<string>";
			  break;

	case PTINT:	  string = "<integer>";
			  break;

	case PTDOUBLE:	  string = "<real>";
			  break;

	case PTFILENAME:  string = "<filename>";
			  break;

	case PTLIST:      string = "";
			  break;

	default:	  string = "";
			  break;
    }

    return (string);

}

/*
**
** Name: add_filenames_to_usage
**
** Purpose: To add trailing filenames the given format to the usage
**	    message.
**
** Inputs: None.
**
** Return Value: Error indication.
**
** External Dependencies:
**    hipsl_parser.h - HIPS argument parser header file.
**
*/

int add_filenames_to_usage ()

{
    /*
    ** External data.
    */

    extern Filename_Ptr	    filename_ptr;
    extern Filename_Format  filename_format;
    extern char *filename_usage;
    char *filestring;
    int len;

    if (filename_usage) {
	filestring = filename_usage;
	len = strlen(filestring)+1;
    }
    else {
	switch (filename_format) {

	case FFNONE:  filestring = "";
		      break;

	case FFONE:   filestring = " [imagefile]";
		      break;

	case FFTWO:   filestring = " imagefile1 [<] imagefile2";
		      break;

	case FFLIST:  filestring = " [imagefile1 imagefile2 ... imagefileN]";
		      break;

	default:      filestring = "";
		      break;
	}
	len = strlen(filestring);
    }
    if (len+currlinelen > MAX_LINE_SIZE) {
	if (usagecat("\n\t\t") == HIPS_ERROR)
		return(HIPS_ERROR);
    }
    if (filename_usage) {
	if (usagecat(" ") == HIPS_ERROR)
		return(HIPS_ERROR);
    }
    if (usagecat(filestring) == HIPS_ERROR)
	return(HIPS_ERROR);
    return(HIPS_OK);
}

/*
** Name: usagecat
**
** Purpose: To append usage text to the message and allocate memory if needed.
**
** Inputs: string to append
**
** Return Value: error indication
*/

int usagecat(str)

char *str;

{
    int len;
    char *newusage;

    len = strlen(str);
    if (len > usageleft) {
	if ((newusage = memalloc(usagelen+USAGE_INCR,sizeof(char)))
	    == (char *) HIPS_ERROR)
		return(HIPS_ERROR);
	strcpy(newusage,usage);
	free(usage);
	usage = newusage;
	usageleft += USAGE_INCR;
	usagelen += USAGE_INCR;
    }
    strcat(usage,str);
    usageleft -= len;
    return(HIPS_OK);
}

/*
** Name: groupcat
**
** Purpose: To append group text to the message and allocate memory if needed.
**
** Inputs: string to append
**
** Return Value: error indication
*/

int groupcat(str)

char *str;

{
    int len;
    char *newgroup;

    len = strlen(str);
    if (len > groupleft) {
	if ((newgroup = memalloc(grouplen+GRPUSAGE_INCR,sizeof(char)))
	    == (char *) HIPS_ERROR)
		return(HIPS_ERROR);
	strcpy(newgroup,grpusage);
	free(grpusage);
	grpusage = newgroup;
	groupleft += GRPUSAGE_INCR;
	grouplen += GRPUSAGE_INCR;
    }
    strcat(grpusage,str);
    groupleft -= len;
    return(HIPS_OK);
}

/*
**
** Name: print_usage
**
** Purpose: To print the usage message for the filter and exit.
**
** Inputs: None.
**
** Return Value: None.
**
** External Dependencies:
**    stdio.h - UNIX standard I/O header file.
**
*/

void
print_usage ()

{
    fprintf (stderr, "%s", usage);
    exit (0);
}
