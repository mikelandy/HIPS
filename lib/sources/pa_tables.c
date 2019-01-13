
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
** Filename: pa_tables.c
**
** Description:	
**
**    This file contains the table internal handling source code for
**    the HIPS argument parser.
**
** Author:  David Wilson, Turing Institute, 30/1/91.
**
*/


/*
** Specify include files.
*/


#include <stdio.h>		/* UNIX standard I/O header file. */

#ifdef HUSESTDARG
#include <stdarg.h>
#else
#include <varargs.h>		/* UNIX variable argument header file. */
#endif

#include <hipl_format.h>	/* HIPS main header file. */




/*
**
** Name: get_flag_data
**
** Purpose: To retrieve the format and parameter pointer data
**	    describing the flag options accepted by the parser.
**
** Inputs: The routine expects the following input:
**
**		varguments - va_list*, a pointer to a variable argument
**			     list.
**
**	   The next argument in the list is expected to be the flag
**	   format descriptor. The arguments immediately following this
**	   are expected to be pointers to each of the parameters
**	   associated with the flag options. Note that the order in which
**	   these are given should be identical to the order in which
**	   the parameters are defined in the flag format description.
**	   Flag options with no associated parameters are deemed to be
**	   binary flags and a pointer to a boolean variable should be
**	   present in the argument list for these flags.
**
**	   The results are undefined if not enough any of the parameter
**	   pointers are missing from the argument list. If too many
**	   parameter pointers are given, the remainder will be ignored.
**
** Return Value: The routine returns 0 on success or HIPS_ERROR if
**		 an error is detected.
**
** External Dependencies:
**    hipl_format.h - HIPS argument parser header file.
**
*/

int get_flag_data (varguments)

va_list	 *varguments;

{
    Flag_Format	 *flag_list;

    /*
    ** External data.
    */

    extern int	     num_flags;

    extern Flag_Key  *flag_table;


    /*
    ** Local functions and variables.
    */

    int		 index,count_flags();

    void	 add_flag_to_table ();

    Flag_Key	 *table_ptr;
    Flag_Format	 *flag_ptr;


    /*
    ** Retrieve the list of flag format descriptors, allocate core for
    ** the parser flag table and then add an entry for each flag in the
    ** flag list.
    */

    flag_list = va_arg (*varguments, Flag_Format *);
    num_flags = count_flags (flag_list);

    if (num_flags &&
	(flag_table = (Flag_Key *) memalloc (num_flags, sizeof (Flag_Key)))
		    == (Flag_Key *) HIPS_ERROR)
    {
	return (HIPS_ERROR);
    }

    flag_ptr = flag_list;
    table_ptr = flag_table;

    for (index = 0; index < num_flags; index++)
    {
	add_flag_to_table (flag_ptr,
			   table_ptr,
			   varguments);

	flag_ptr++;
	table_ptr++;
    }

    return (0);

}




/*
**
** Name: count_flags
**
** Purpose: To count the number of flags in a flag list.
**
** Inputs: The routine expects the following:
**
**		flag_list - Flag_Format *, a pointer to a list of format
**			    descriptors for each flag to be accepted by
**			    the parser.
**
** Return Value: The routine returns a count of the number of flags in
**		 the flag list.
**
** External Dependencies:
**    hipl_format.h - HIPS argument parser header file.
**
*/

int count_flags(flag_list)

Flag_Format  *flag_list;

{

    int  count = 0;


    while ((flag_list++)->value != LASTFLAG)
    {
	count++;
    }

    return (count);

}




/*
**
** Name: add_flag_to_table
**
** Purpose: To add an entry to the parser flag table.
**
** Inputs: The following inputs are expected:
**
**		flag_ptr - Flag_Format *, a pointer to the format
**			   descriptor for a flag.
**
**		table_ptr - Flag_Key *, a pointer to the position in
**			    the flag table at which the format descriptor
**			    is added.
**
**		varguments - va_list *, a pointer to the variable
**			     argument list.
**
**	   The next arguments in the list are expected to be pointers
**	   to each of the parameters associated with the flag options.
**	   Note that the order in which these are given should be
**	   identical to the order in which the parameters are defined
**	   in the flag format description. Flag options with no
**	   associated parameters are deemed to be binary flags and a
**	   pointer to a boolean variable should be present in the
**	   argument list for these flags.
**
**	   The results are undefined if not enough any of the parameter
**	   pointers are missing from the argument list. If too many
**	   parameter pointers are given, the remainder will be ignored.
**
** Return Value: None
**
** External Dependencies:
**    varargs.h - UNIX variable argument header file.
**    hipl_format.h - HIPS argument parser header file.
**
*/

void add_flag_to_table(flag_ptr,table_ptr,varguments)

Flag_Format  *flag_ptr;
Flag_Key     *table_ptr;
va_list	     *varguments;

{

    Parameter	 *parameter;
    Generic_Ptr	 *parameter_ptr;


    /*
    ** Add the flag format descriptor.  Note that it hasn't been used yet.
    */

    table_ptr->format = flag_ptr;
    table_ptr->specified = FALSE;

    /*
    ** Retrieve pointers for each of the parameters associated with
    ** this flag. Note that only the first parameter may be a boolean.
    */

    parameter = flag_ptr->parameters;
    parameter_ptr = table_ptr->parameter_ptrs;

    if (parameter->type == PTBOOLEAN)
    {
	parameter_ptr->boolean_ptr = va_arg (*varguments, h_boolean *);
	parameter++;
	parameter_ptr++;
    }

    while (parameter->type != LASTPARAMETER)
    {
        switch (parameter->type)
        {
	    case PTCHAR:   parameter_ptr->char_ptr = va_arg (*varguments, char *);
		           break;

	    case PTSTRING: parameter_ptr->string_ptr = va_arg (*varguments, char **);
		           break;

	    case PTINT:    parameter_ptr->int_ptr = va_arg (*varguments, int *);
		           break;

	    case PTDOUBLE: parameter_ptr->double_ptr = va_arg (*varguments, double *);
		           break;

	    case PTFILENAME: parameter_ptr->filename_ptr = va_arg (*varguments, char **);
		           break;

	    case PTLIST:   parameter_ptr->listarg_ptr = va_arg (*varguments, Listarg *);
		           break;

        }

        parameter++;
        parameter_ptr++;
    }

}




/*
**
** Name: find_flag
**
** Purpose: To locate the named flag in the flag table.
**
** Inputs: The routine expects the following:
**
**		value - the flag value to be located.
**
** Return Value: If the named flag is found, a pointer to the entry in
**		 the parser flag table is returned. Otherwise, a NULL
**		 pointer is returned.
**
** External Dependencies:
**    hipsl_parser.h - HIPS argument parser header file.
**
*/


Flag_Key *find_flag(value)

Flag  value;

{

    /*
    ** External data.
    */

    extern int	     num_flags;

    extern Flag_Key  *flag_table;


    /*
    ** Local data.
    */

    int	      found, index;

    Flag_Key  *flag_ptr;


    index = 0;
    found = FALSE;
    flag_ptr = flag_table;

    while (index < num_flags &&
	   found == FALSE)
    {
	if (strcmp (flag_ptr->format->value, value) == 0)
	{
	    return (flag_ptr);
	}

	flag_ptr++;
	index++;
    }

    return ((Flag_Key *) NULL);

}



/*
**
** Name: unlock_all_flags
**
** Purpose: To release the mutex lock on all the flags in the flag table.
**
** Inputs: None.
**
** Return Value: None.
**
** External Dependencies:
**    hipsl_parser.h - HIPS argument parser header file.
**
*/


void unlock_all_flags()

{

    /*
    ** External data.
    */

    extern int	     num_flags;

    extern Flag_Key  *flag_table;


    /*
    ** Local data.
    */

    int	      index;

    Flag_Key  *flag_ptr;


    flag_ptr = flag_table;

    for (index = 0; index < num_flags; index++)
    {
	flag_ptr->locked = FALSE;
	flag_ptr++;
    }

}




/*
**
** Name: lock_flags
**
** Purpose: To set the mutex lock on a specified list of flags in the
**	    flag table.
**
** Inputs: The routine expects the following:
**
**		flag_list - Flag *, a pointer to a list of the flags to
**			    be locked.
**
** Return Value: None.
**
** External Dependencies:
**    hipsl_parser.h - HIPS argument parser header file.
**
*/


void lock_flags(flag_list)

Flag  *flag_list;

{

    Flag_Key  *flag_ptr;


    while (*flag_list != LASTFLAG)
    {
	flag_ptr = find_flag (*flag_list);
	flag_ptr->locked = TRUE;
	flag_list++;
    }

}




/*
**
** Name: get_filename_data
**
** Purpose: To retrieve the format and parameter pointer data
**	    describing the trailing filenames accepted by the parser.
**
** Inputs: The routine expects the following input:
**
**		varguments - va_list*, a pointer to a variable argument
**			     list.
**
**	   Note that the next argument in the list is expected to be
**	   the filename format descriptor immediately followed by
**	   pointers to the filename variables. The results are undefined
**	   any of the parameters missing from the argument list. If too many
**	   parameter are given, the remainder will be ignored.
**
** Return Value: The routine returns 0 on success or HIPS_ERROR (-1) if
**		 an error in the filename format is detected.
**
** External Dependencies:
**    hipl_format.h - HIPS argument parser header file.
**
*/

void get_filename_data(varguments)

va_list	 *varguments;

{

    /*
    ** External data.
    */

    extern Filename_Ptr	    filename_ptr;
    extern Filename_Format  filename_format;


    filename_format = va_arg (*varguments, int);

    switch (filename_format)
    {

	case FFNONE:  break;

	case FFONE:   filename_ptr.filename = va_arg (*varguments, char **);
		      break;

	case FFTWO:   filename_ptr.filepair[0] = va_arg (*varguments, char **);
		      filename_ptr.filepair[1] = va_arg (*varguments, char **);
		      break;

	case FFLIST:  filename_ptr.filenames.count = va_arg (*varguments, int *);
		      filename_ptr.filenames.list = va_arg (*varguments, char ***);
		      break;

    }

}
