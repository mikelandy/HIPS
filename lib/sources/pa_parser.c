
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
** Filename: pa_parser.c
**
** Description:	
**
**    This file contains the command line parsing source code for
**    the HIPS argument parser.
**
** Author:  David Wilson, Turing Institute, 30/1/91.
**
*/


/*
** Specify include files.
*/


#include <stdio.h>		/* UNIX standard I/O header file. */
#include <hipl_format.h>	/* HIPS main header file. */




/*
** External data for usage message.
*/

extern char  *usage;
int accept_flag(),accept_filter_flag(),accept_filter_parameters();
int accept_parameter_value(),accept_parameter_list(),accept_standard_flag();
int accept_filenames(),accept_file_value();
void disable_mutex();
h_boolean ischaracter(),isstring(),isinteger(),isreal(),isfilename(),isflag();

/*
**
** Name: parse_command_line
**
** Purpose: To parse the command line arguments and set the operating
**	    mode for the filter according to the flags which are
**	    present.
**
** Inputs: The routine expects the following:
**
**		argc - int, the number of command line arguments.
**
**		argv - char **, the list of command line arguments.
**
** Return Value: The routine returns 0 on success or HIPS_ERROR if
**		 the parser detects a syntax error in the command line.
**
** External Dependencies:
**    hipsl_parser.h - HIPS argument parser header file.
**
*/


int parse_command_line(argc,argv)

int   argc;
char  **argv;

{

    /*
    ** External  functions.
    */

    extern void  unlock_all_flags ();


    /*
    ** Functions called and local variables.
    */

    char  **arg_list;

    int   arg_count;
    int   accept_flag ();
    int   accept_filenames ();


    /*
    ** Clear the mutex locks to begin with.
    */

    unlock_all_flags ();

    /*
    ** Skip over the program name, then cycle through each of the
    ** command line arguments in turn. The command line should
    ** consist of flags followed by trailing filenames.
    */

    arg_list = argv + 1;
    arg_count = argc - 1;

    while (arg_count > 0 &&
	   isflag (*arg_list) == TRUE)
    {
	if (accept_flag (&arg_count, &arg_list) == HIPS_ERROR)
	{
	    return (HIPS_ERROR);
	}
    }

    if (accept_filenames (&arg_count, &arg_list) == HIPS_ERROR)
    {
	return (HIPS_ERROR);
    }

    return (0);

}




/*
**
** Name: accept_flag
**
** Purpose: To parse and verify a flag and its associated parameters from
**	    the command line arguments.
**
** Inputs: The routine expects the following:
**
**		arg_count - int *, a pointer to the number of remaining
**			    command line arguments.
**
**		arg_list - char ***, a pointer to the list of remaining
**			   command line arguments.
**
** Return Value: The routine returns 0 on success or HIPS_ERROR if
**		 the parser does not accept the flag.
**
** External Dependencies:
**    hipsl_parser.h - HIPS argument parser header file.
**
*/


int accept_flag(arg_count,arg_list)

int   *arg_count;
char  ***arg_list;

{

    /*
    ** Functions called and local variables.
    */

    int  accept_filter_flag ();
    int  accept_standard_flag ();

    h_boolean  is_filter_flag, is_standard_flag;


    /*
    ** Test if it is a filter-specific flag first of all.
    */

    if (accept_filter_flag (arg_count,
			    arg_list,
			    &is_filter_flag)
			    == HIPS_ERROR)
    {
	return (HIPS_ERROR);
    }

    /*
    ** If it has not been accepted as a filter-specific flag, test if it
    ** is a standard flag.
    */

    if (is_filter_flag == FALSE)
    {
	if (accept_standard_flag (arg_count,
				  arg_list,
				  &is_standard_flag)
				  == HIPS_ERROR)
	{
	    return (HIPS_ERROR);
	}
    }

    if (is_filter_flag == FALSE &&
	is_standard_flag == FALSE)
    {
	return (perr (HE_UNKFLG, **arg_list, usage));
    }

    return (0);

}




/*
**
** Name: accept_filter_flag
**
** Purpose: To parse and verify a filter-specific flag and its associated
**	    parameters from the command line arguments.
**
** Inputs: The routine expects the following:
**
**		arg_count - int *, a pointer to the number of remaining
**			    command line arguments.
**
**		arg_list - char ***, a pointer to the list of remaining
**			   command line arguments.
**
**		is_filter_flag - h_boolean *, a pointer to a flag set to
**			   indicate whether or not the flag has been
**			   accepted as a filter-specific flag.
**
** Return Value: The routine returns 0 on success or HIPS_ERROR (-1) if
**		 an error is detected when parsing the flag.
**
** External Dependencies:
**    hipsl_parser.h - HIPS argument parser header file.
**
*/


int accept_filter_flag(arg_count,arg_list,is_filter_flag)

int	 *arg_count;
char	 ***arg_list;
h_boolean	 *is_filter_flag;

{

    /*
    ** External functions.
    */

    extern void	     lock_flags ();

    extern Flag_Key  *find_flag ();


    /*
    ** Local functions and variables.
    */

    int	      accept_filter_parameters ();

    void      disable_mutex_flags ();

    Flag      flag;
    Flag_Key  *flag_ptr;


    /*
    ** Remove the leading '-'.
    */

    flag = (**arg_list) + 1;

    /*
    ** Search for the flag in the parser flag table and signal whether
    ** or not it is recognised by this filter.
    */

    if ((flag_ptr = find_flag (flag)) == (Flag_Key *) NULL)
    {
	*is_filter_flag = FALSE;
    }
    else
    {
	*is_filter_flag = TRUE;

	/*
	** Signal an error if this flag has been locked out through
	** mutual exclusion or if it has already been specified.
	*/

	if (flag_ptr->locked == TRUE)
	{
	    return (perr (HE_MUTEX, usage));
	}
	if (flag_ptr->specified) {
	    return (perr (HE_PTWICE,flag));
	}
	flag_ptr->specified = TRUE;

	/*
	** Retrieve any parameters associated with the flag.
	*/

	(*arg_list)++;
	(*arg_count)--;
	
	if (accept_filter_parameters (flag_ptr,
				      arg_count,
				      arg_list)
				      == HIPS_ERROR)
	{
	    return (HIPS_ERROR);
	}
	
	/*
	** Lock out and disable any flags with which this flag is
	** mutually exclusive.
	*/

	lock_flags (flag_ptr->format->mutex_flags);
	disable_mutex_flags (flag_ptr->format->mutex_flags);
    }

    return (0);

}




/*
**
** Name: accept_filter_parameters
**
** Purpose: To parse and verify the parameters associated with a 
**	    filter-specific flag from the command line arguments.
**
** Inputs: The routine expects the following:
**
**		flag_ptr - Flag_Key *, a pointer to a flag whose
**			   parameters are to be parsed from the
**			   command line.
**
**		arg_count - int *, a pointer to the number of remaining
**			    command line arguments.
**
**		arg_list - char ***, a pointer to the list of remaining
**			   command line arguments.
**
** Return Value: The routine returns 0 on success or HIPS_ERROR (-1) if
**		 an error is detected when parsing the flag parameters.
**
** External Dependencies:
**    hipl_format.h - HIPS argument parser header file.
**
*/


int accept_filter_parameters(flag_ptr,arg_count,arg_list)

Flag_Key  *flag_ptr;
int	  *arg_count;
char	  ***arg_list;

{

    /*
    ** Functions called.
    */

    int		 index, min_parameters;

    int    	 accept_parameter_value ();
    h_boolean	 is_valid_parameter;
    h_boolean	 continuing = TRUE;
    Parameter	 *parameter;
    Generic_Ptr	 *parameter_ptr;


    /*
    ** Retrieve values from the command line for any parameters
    ** associated with this flag.
    */

    parameter = flag_ptr->format->parameters;
    parameter_ptr = flag_ptr->parameter_ptrs;
    min_parameters = flag_ptr->format->min_parameters;

    if (*arg_count < min_parameters)
    {
	return (perr (HE_MISSFPAR, flag_ptr->format->value, usage));
    }

    /*
    ** Some flags which change the operating mode of the filter will
    ** have a boolean parameter as the first parameter. Set this to be
    ** TRUE, showing that the operating mode associated with this flag
    ** has been enabled.
    */

    if (parameter->type == PTBOOLEAN)
    {
	*(parameter_ptr->boolean_ptr) = TRUE;
	parameter++;
	parameter_ptr++;
    }

    /*
    ** Make sure that the mandatory parameters are present.
    */

    for (index = 0; index < min_parameters; index++)
    {
	if (parameter->type == PTLIST) {
		if (accept_parameter_list  (parameter_ptr,
					    arg_list,
					    arg_count,
					    &is_valid_parameter)
					    == HIPS_ERROR)
		{
		    return (HIPS_ERROR);
		}
		else if (is_valid_parameter == FALSE)
		{
		    return (perr (HE_INVFPAR, **arg_list, flag_ptr->format->value, usage));
		}
	}
	else {
		if (accept_parameter_value (parameter_ptr,
					    parameter->type,
					    **arg_list,
					    &is_valid_parameter)
					    == HIPS_ERROR)
		{
		    return (HIPS_ERROR);
		}
		else if (is_valid_parameter == FALSE)
		{
		    return (perr (HE_INVFPAR, **arg_list, flag_ptr->format->value, usage));
		}

		(*arg_list)++;
		(*arg_count)--;
	}
	parameter++;
	parameter_ptr++;
    }

    /*
    ** Greedy parsing to retrieve the optional parameters.
    */

    while (*arg_count > 0 &&
	   parameter->type != LASTPARAMETER &&
	   continuing == TRUE)
    {
	if (parameter->type == PTLIST) {
		if (accept_parameter_list  (parameter_ptr,
					    arg_list,
					    arg_count,
					    &is_valid_parameter)
					    == HIPS_ERROR)
		{
		    return (HIPS_ERROR);
		}
		else if (is_valid_parameter == TRUE)
		{
		    parameter++;
		    parameter_ptr++;
		}
		else
		{
		    continuing = FALSE;
		}
	}
	else {
		if (accept_parameter_value (parameter_ptr,
					    parameter->type,
					    **arg_list,
					    &is_valid_parameter)
					    == HIPS_ERROR)
		{
		    return (HIPS_ERROR);
		}
		else if (is_valid_parameter == TRUE)
		{
		    parameter++;
		    parameter_ptr++;
		    (*arg_list)++;
		    (*arg_count)--;
		}
		else
		{
		    continuing = FALSE;
		}
	}
    }

    return (0);

}




/*
**
** Name: accept_parameter_value
**
** Purpose: To assign a value given as a string literal to a parameter.
**	    Note that the value is checked to verify that it is a literal
**	    of the correct type. If it is not of the correct type, the
**	    parameter value is not accepted.
**
** Inputs:
**
** Return Value:
**
** External Dependencies:
**    hipl_format.h - HIPS argument parser header file.
**
*/


int accept_parameter_value(parameter_ptr,parameter_type,value,is_valid)

Generic_Ptr	*parameter_ptr;
Parameter_Type	parameter_type;
char		*value;
h_boolean		*is_valid;

{

    /*
    ** External functions.
    */

    extern int	    atoi ();

    extern double   atof ();



    switch (parameter_type)
    {
	case PTCHAR:    if ((*is_valid = ischaracter (value)) == TRUE)
			{
			    *(parameter_ptr->char_ptr) = *value;
			}
			break;

	case PTSTRING:  if ((*is_valid = isstring (value)) == TRUE)
			{
			    free (*parameter_ptr->string_ptr);

			    if (((*parameter_ptr->string_ptr) = (char *) memalloc (strlen (value) + 1, sizeof (char)))
			    				      == (char *) HIPS_ERROR)
			    {
				return (HIPS_ERROR);
			    }

			    strcpy (*parameter_ptr->string_ptr, value);
			}
			break;

	case PTINT:     if ((*is_valid = isinteger (value)) == TRUE)
			{
			    *(parameter_ptr->int_ptr) = atoi (value);
			}
			break;

	case PTDOUBLE:  if ((*is_valid = isreal (value)) == TRUE)
			{
			    *(parameter_ptr->double_ptr) = atof (value);
			}
			break;

	case PTFILENAME:  if ((*is_valid = isfilename (value)) == TRUE)
			{
			    free (*parameter_ptr->filename_ptr);

			    if (((*parameter_ptr->filename_ptr) = (char *) memalloc (strlen (value) + 1, sizeof (char)))
			    				      == (char *) HIPS_ERROR)
			    {
				return (HIPS_ERROR);
			    }

			    strcpy (*parameter_ptr->filename_ptr, value);
			}
			break;

    }

    return (0);

}




/*
**
** Name: accept_parameter_list
**
** Purpose: To assign a series of values given as string literals to a PTLIST
**	    type of parameter.
**	    Note that the values are checked to verify that they are literals
**	    of the correct type. If not, the parameter value is not accepted.
**
** Inputs:
**
** Return Value:
**
** External Dependencies:
**    hipl_format.h - HIPS argument parser header file.
**
*/


int accept_parameter_list(parameter_ptr,arglist,argcount,is_valid)

Generic_Ptr	*parameter_ptr;
char		***arglist;
int		*argcount;
h_boolean		*is_valid;

{
    int count,i;

    /*
    ** External functions.
    */

    extern int	    atoi ();

    extern h_boolean  isstring ();
    extern h_boolean  isinteger ();


	if (isinteger(**arglist)) {
		count = atoi(**arglist);
		if (count < 0 || count > *argcount - 1) {
			*is_valid = FALSE;
			return(HIPS_OK);
		}
		parameter_ptr->listarg_ptr->argcount = count;
		if ((parameter_ptr->listarg_ptr->args =
			(char **) memalloc(count,sizeof(char *))) ==
			(char **) HIPS_ERROR)
				return(HIPS_ERROR);
		*is_valid = TRUE;
		(*arglist)++; (*argcount)--;
		for (i=0;i<count;i++) {
			parameter_ptr->listarg_ptr->args[i] = **arglist;
			(*arglist)++; (*argcount)--;
		}
	}
	else
		*is_valid = FALSE;
	return(HIPS_OK);

}




/*
**
** Name: disable_mutex_flags
**
** Purpose: To disable a list of flags in a mutex group. Each of these
**	    flags may have an associated boolean parameter which is set to
**	    FALSE to indicate that this mutex option has been disabled.
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


void disable_mutex_flags(flag_list)

Flag  *flag_list;

{

    Flag_Key  *flag_ptr;
    extern Flag_Key  *find_flag ();


    while (*flag_list != LASTFLAG)
    {
	flag_ptr = find_flag (*flag_list);
	if (flag_ptr->format->parameters->type == PTBOOLEAN)
		*(flag_ptr->parameter_ptrs->boolean_ptr) = FALSE;

	flag_list++;
    }

}




/*
**
** Name: accept_standard_flag
**
** Purpose: To parse and verify a standard HIPS flag and its associated
**	    parameters from the command line arguments.
**
** Inputs: The routine expects the following:
**
**		arg_count - int *, a pointer to the number of remaining
**			    command line arguments.
**
**		arg_list - char ***, a pointer to the list of remaining
**			   command line arguments.
**
**		is_standard_flag - h_boolean *, a pointer to a flag set to
**			   indicate whether or not the flag has been
**			   accepted.
**
** Return Value: The routine returns TRUE if the parser accepts the flag
**		 or FALSE otherwise.
**
** External Dependencies:
**    hipl_format.h - HIPS argument parser header file.
**
*/


int accept_standard_flag(arg_count,arg_list,is_standard_flag)

int	 *arg_count;
char	 ***arg_list;
h_boolean	 *is_standard_flag;

{

    /*
    ** Extrenal function and variable.
    */

    extern void  print_usage ();


    /*
    ** Local variable.
    */

    int	  value;
    static h_boolean Pspec=FALSE,CRspec=FALSE,RCspec=FALSE,UHspec=FALSE;
    static h_boolean ULspec=FALSE;


    *is_standard_flag = TRUE;

    if (strcmp (**arg_list, "-U") == 0)
    {
	print_usage ();
    }
    else if (strcmp (**arg_list, "-CB") == 0)
    {
	hips_convback = TRUE;
	(*arg_list)++; (*arg_count)--;
    }
    else if (strcmp (**arg_list, "-NFH") == 0)
    {
	hips_fullhist = FALSE;
	(*arg_list)++; (*arg_count)--;
    }
    else if (strcmp (**arg_list, "-NFD") == 0)
    {
	hips_fulldesc = FALSE;
	(*arg_list)++; (*arg_count)--;
    }
    else if (strcmp (**arg_list, "-FXP") == 0)
    {
	hips_fullxpar = TRUE;
	(*arg_list)++; (*arg_count)--;
    }
    else if (strcmp (**arg_list, "-D") == 0)
    {
	(*arg_list)++; (*arg_count)--;
    }
    else if (strcmp (**arg_list, "-P") == 0)
    {
	(*arg_list)++; (*arg_count)--;
	if (Pspec)
	    return (perr (HE_PTWICE,"P"));
	Pspec = TRUE;

	if (*arg_count < 1)
	{
	    return (perr (HE_MISSFPAR, "P", usage));
	}

	if (strcmp (**arg_list, "M") == 0)
	{
	    hips_packing = MSBF_PACKING;
	}
	else if (strcmp (**arg_list, "L") == 0)
	{
	    hips_packing = LSBF_PACKING;
	}
	else
	{
	    return (perr (HE_INVFPAR, **arg_list, "P", usage));
	}

	(*arg_list)++; (*arg_count)--;
    }
    else if (strcmp (**arg_list, "-CR") == 0)
    {
	(*arg_list)++; (*arg_count)--;
	if (CRspec)
	    return (perr (HE_PTWICE,"CR"));
	CRspec = TRUE;

	if (*arg_count < 1)
	{
	    return (perr (HE_MISSFPAR, "CR", usage));
	}

	if (strcmp (**arg_list, "M") == 0)
	{
	    hips_cplxtor = CPLX_MAG;
	}
	else if (strcmp (**arg_list, "P") == 0)
	{
	    hips_cplxtor = CPLX_PHASE;
	}
	else if (strcmp (**arg_list, "R") == 0)
	{
	    hips_cplxtor = CPLX_REAL;
	}
	else if (strcmp (**arg_list, "I") == 0)
	{
	    hips_cplxtor = CPLX_IMAG;
	}
	else
	{
	    return (perr (HE_INVFPAR, **arg_list, "CR", usage));
	}

	(*arg_list)++; (*arg_count)--;
    }
    else if (strcmp (**arg_list, "-RC") == 0)
    {
	(*arg_list)++; (*arg_count)--;
	if (RCspec)
	    return (perr (HE_PTWICE,"RC"));
	RCspec = TRUE;

	if (*arg_count < 1)
	{
	    return (perr (HE_MISSFPAR, "RC", usage));
	}

	if (strcmp (**arg_list, "I") == 0)
	{
	    hips_rtocplx = CPLX_R0IV;
	}
	else if (strcmp (**arg_list, "R") == 0)
	{
	    hips_rtocplx = CPLX_RVI0;
	}
	else if (strcmp (**arg_list, "B") == 0)
	{
	    hips_rtocplx = CPLX_RVIV;
	}
	else
	{
	    return (perr (HE_INVFPAR, **arg_list, "RC", usage));
	}

	(*arg_list)++; (*arg_count)--;
    }
    else if (strcmp (**arg_list, "-UH") == 0)
    {
	(*arg_list)++; (*arg_count)--;
	if (UHspec)
	    return (perr (HE_PTWICE,"UH"));
	UHspec = TRUE;

	if (*arg_count < 1)
	{
	    return (perr (HE_MISSFPAR, "UH", usage));
	}

	if (isinteger (**arg_list) == FALSE)
	{
	    return (perr (HE_INVFPAR, **arg_list, "UH", usage));
	}

	value = atoi (**arg_list);
	if (value < 0 || value > 255)
	{
	    return (perr (HE_INVFPAR, **arg_list, "UH", usage));
	}

	hips_hchar = (byte) value;

	(*arg_list)++; (*arg_count)--;
    }
    else if (strcmp (**arg_list, "-UL") == 0)
    {
	(*arg_list)++; (*arg_count)--;
	if (ULspec)
	    return (perr (HE_PTWICE,"UL"));
	ULspec = TRUE;

	if (*arg_count < 1)
	{
	    return (perr (HE_MISSFPAR, "UL", usage));
	}

	if (isinteger (**arg_list) == FALSE)
	{
	    return (perr (HE_INVFPAR, **arg_list, "UL", usage));
	}

	value = atoi (**arg_list);
	if (value < 0 || value > 255)
	{
	    return (perr (HE_INVFPAR, **arg_list, "UH", usage));
	}

	hips_lchar = (byte) value;

	(*arg_list)++; (*arg_count)--;
    }
    else
    {
	*is_standard_flag = FALSE;
    }

    return (0);

}




/*
**
** Name: accept_filenames
**
** Purpose: To parse and verify trailing filenames in the command line
**	    arguments.
**
** Inputs: The routine expects the following:
**
**		arg_count - int *, a pointer to the number of remaining
**			    command line arguments.
**
**		arg_list - char ***, a pointer to the list of remaining
**			   command line arguments.
**
** Return Value: The routine returns 0 on success or HIPS_ERROR (-1) if
**		 the parser does not accept the flag.
**
** External Dependencies:
**    hipsl_parser.h - HIPS argument parser header file.
**
*/


int accept_filenames(arg_count,arg_list)

int   *arg_count;
char  ***arg_list;

{

    /*
    ** External data.
    */

    extern Filename_Ptr	    filename_ptr;
    extern Filename_Format  filename_format;

    /*
    ** Local data.
    */

    char     **list_ptr;

    h_boolean  accept_stdin = TRUE;
    h_boolean  is_valid_filename;


    switch (filename_format)
    {

	case FFNONE: if (*arg_count > 0)
		     {
			 return (perr (HE_SYNTAX, usage));
		     }

		     break;

	case FFONE:  if (*arg_count > 1)
		     {
		         return (perr (HE_FILECNT, usage));
		     }

		     if (*arg_count > 0)
		     {
			 free (*filename_ptr.filename);

			 if (accept_file_value (filename_ptr.filename,
						**arg_list,
						&accept_stdin,
						&is_valid_filename)
						== HIPS_ERROR)
			 {
		             return (HIPS_ERROR);
			 }
			 else if (is_valid_filename == FALSE)
			 {
		             return (perr (HE_INVFILE, **arg_list));
			 }
			 (*arg_list)++; (*arg_count)--;
		     }

		     break;

	case FFTWO:  if (*arg_count > 2)
		     {
		         return (perr (HE_FILECNT, usage));
		     }
		     else if (*arg_count < 1)
		     {
		         return (perr (HE_MISSFILE, usage));
		     }
		     else if (*arg_count == 1)
		     {
			 accept_stdin = FALSE;		/* Default is file 2 is stdin. */
		     }

		     free (*filename_ptr.filepair[0]);

		     if (accept_file_value (filename_ptr.filepair[0],
					    **arg_list,
					    &accept_stdin,
					    &is_valid_filename)
					    == HIPS_ERROR)
		     {
		         return (HIPS_ERROR);
		     }
		     else if (is_valid_filename == FALSE)
		     {
		         return (perr (HE_INVFILE, **arg_list));
		     }
		     (*arg_list)++; (*arg_count)--;

		      if (*arg_count > 0)
		      {
		     	  free (*filename_ptr.filepair[1]);

		          if (accept_file_value (filename_ptr.filepair[1],
						 **arg_list,
						 &accept_stdin,
						 &is_valid_filename)
						 == HIPS_ERROR)
			 {
		             return (HIPS_ERROR);
			 }
			 else if (is_valid_filename == FALSE)
		          {
		              return (perr (HE_INVFILE, **arg_list));
		          }
		          (*arg_list)++; (*arg_count)--;
		      }

		      break;

	case FFLIST:  if (*arg_count > 0)
		      {
		          free (**filename_ptr.filenames.list);
		          free (*filename_ptr.filenames.list);

		          *(filename_ptr.filenames.count) = *arg_count;

		          if (((*filename_ptr.filenames.list) = (char **) memalloc (*arg_count, sizeof (char *)))
							  == (char **) HIPS_ERROR)
		          {
			      return (HIPS_ERROR);
		          }

		          list_ptr = *filename_ptr.filenames.list;

		          while (*arg_count > 0)
		          {

		              if (accept_file_value (list_ptr,
						     **arg_list,
						     &accept_stdin,
						     &is_valid_filename)
						     == HIPS_ERROR)
		              {
		       	        return (HIPS_ERROR);
		              }
			      else if (is_valid_filename == FALSE)
		              {
		                  return (perr (HE_INVFILE, **arg_list));
		              }

		              (*arg_list)++; (*arg_count)--;
		              list_ptr++;
		          }

		      }
		      break;

    }

    return (0);

}

/*
 *
 * Name: accept_file_value
 *
 * Purpose: To assign a value given as a string literal to a filename.
 *	    Note that the value is checked to verify that it is a literal
 *	    of the correct type. If it is not of the correct type, the
 *	    filename value is not accepted.
 *
 * Inputs:
 *
 * Return Value:
 *
 * External Dependencies:
 *    hipl_format.h - HIPS argument parser header file.
 *
 */


int accept_file_value(filename_ptr,value,accept_stdin,is_valid_filename)

char	 **filename_ptr;
char	 *value;
h_boolean  *accept_stdin;
h_boolean  *is_valid_filename;

{
	if (!isfilename(value)) {
		*is_valid_filename = FALSE;
		return(HIPS_OK);
	}
	if (strcmp(value,"-") == 0 || strcmp(value,"<stdin>") == 0) {
		if (!(*accept_stdin)) {
			*is_valid_filename = FALSE;
			return(HIPS_OK);
		}
		*accept_stdin = FALSE;
		*is_valid_filename = TRUE;
		if ((*filename_ptr =
			(char *) memalloc(strlen("<stdin>")+1,sizeof(char)))
				== (char *) HIPS_ERROR)
					return (HIPS_ERROR);
		strcpy(*filename_ptr,"<stdin>");
		return(HIPS_OK);
	}
	*is_valid_filename = TRUE;
	if ((*filename_ptr = (char *) memalloc(strlen(value)+1,sizeof(char)))
		== (char *) HIPS_ERROR)
			return (HIPS_ERROR);
	strcpy(*filename_ptr,value);
	return(HIPS_OK);
}
