
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
** Filename: pa_defaults.c
**
** Description:	
**
**    This file contains the default setting source code for the
**    HIPS argument parser.
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
**
** Name: set_defaults
**
** Purpose: To set the default operating mode for the filter.
**
** Inputs: None.
**
** Return Value: None.
**
** External Dependencies:
**    hipl_format.h - HIPS argument parser header file.
**
*/


int set_defaults()

{

    /*
    ** Functions called.
    */

    int	  set_flag_defaults ();
    int	  set_filename_defaults ();


    if (set_flag_defaults () == HIPS_ERROR)
    {
	return (HIPS_ERROR);
    }

    if (set_filename_defaults () == HIPS_ERROR)
    {
	return (HIPS_ERROR);
    }

    return (0);

}



/*
**
** Name: set_flag_defaults
**
** Purpose: To set the default operating mode for the filter-specific
**	    flag options.
**
** Inputs: None.
**
** Return Value: None.
**
** External Dependencies:
**    hipl_format.h - HIPS argument parser header file.
**
*/


int set_flag_defaults()

{

    /*
    ** External functions and data.
    */

    extern int	     atoi ();
    extern int	     num_flags;

    extern double    atof ();

    extern Flag_Key  *flag_table;


    /*
    ** Local functions and variables.
    */

    int		 index;

    Parameter	 *parameter;
    Generic_Ptr	 *parameter_ptr;
    Flag_Key	 *flag_ptr;


    /*
    ** Cycle thro' each flag in turn and set the default value for each
    ** parameter that is associated with it.
    */

    flag_ptr = flag_table;

    for (index = 0; index < num_flags; index++)
    {
	parameter = flag_ptr->format->parameters;
	parameter_ptr = flag_ptr->parameter_ptrs;

	while (parameter->type != LASTPARAMETER)
	{
	    switch (parameter->type)
	    {
		case PTBOOLEAN: if (strcmp (parameter->pdefault, "TRUE") == 0)
			       {
				  *(parameter_ptr->boolean_ptr) = TRUE;
			       }
			       else
			       {
				  *(parameter_ptr->boolean_ptr) = FALSE;
			       }

			       break;

		case PTCHAR:   *(parameter_ptr->char_ptr) = *(parameter->pdefault);
			       break;

		case PTSTRING: if (((*parameter_ptr->string_ptr) = (char *) memalloc (strlen (parameter->pdefault) + 1, sizeof(char)))
								 == (char*) HIPS_ERROR)
			       {
				   return (HIPS_ERROR);
			       }
			       strcpy (*parameter_ptr->string_ptr, parameter->pdefault);
			       break;

		case PTINT:    *(parameter_ptr->int_ptr) = atoi (parameter->pdefault);
			       break;

		case PTDOUBLE: *(parameter_ptr->double_ptr) = atof (parameter->pdefault);
			       break;

		case PTFILENAME: if (((*parameter_ptr->filename_ptr) = (char *) memalloc (strlen (parameter->pdefault) + 1, sizeof(char)))
								 == (char*) HIPS_ERROR)
			       {
				   return (HIPS_ERROR);
			       }
			       strcpy (*parameter_ptr->filename_ptr, parameter->pdefault);
			       break;

		case PTLIST:   parameter_ptr->listarg_ptr->argcount = 0;
			       break;

	    }

	    parameter++;
	    parameter_ptr++;
	}

	flag_ptr++;
    }
    return(HIPS_OK);

}



/*
**
** Name: set_filename_defaults
**
** Purpose: To set default values for any trailing filename parameters.
**
** Inputs: None.
**
** Return Value: None.
**
** External Dependencies:
**    hipl_format.h - HIPS argument parser header file.
**
*/


int set_filename_defaults()

{

    /*
    ** External data.
    */

    extern Filename_Ptr	    filename_ptr;
    extern Filename_Format  filename_format;


    /*
    ** Local data.
    */

    static int  stdin_length = 8;	/* Length of string "<stdin>" */


    /*
    ** Expect image input to come from stdin by default.
    ** Note that the filename pointer in the case of a list
    ** of filenames has not been preallocated so memory must
    ** be found.
    */

    switch (filename_format)
    {

	case FFNONE:  break;

	case FFONE:   if (((*filename_ptr.filename) = (char *) memalloc (stdin_length, sizeof (char)))
						    == (char *) HIPS_ERROR)
		      {
			  return (HIPS_ERROR);
		      }

		      strcpy (*filename_ptr.filename, "<stdin>");
		      break;


	case FFTWO:   if (((*filename_ptr.filepair[0]) = (char *) memalloc (1, sizeof (char)))
						       == (char *) HIPS_ERROR)
		      {
			  return (HIPS_ERROR);
		      }

		      strcpy (*filename_ptr.filepair[0], "");

		      if (((*filename_ptr.filepair[1]) = (char *) memalloc (stdin_length, sizeof (char)))
						       == (char *) HIPS_ERROR)
		      {
			  return (HIPS_ERROR);
		      }

		      strcpy (*filename_ptr.filepair[1], "<stdin>");
		      break;

	case FFLIST:  *(filename_ptr.filenames.count) = 1;

		      if (((*filename_ptr.filenames.list) = (char **) memalloc (1, sizeof (char *)))
							  == (char **) HIPS_ERROR)
		      {
			  return (HIPS_ERROR);
		      }

		      if (((**filename_ptr.filenames.list) = memalloc (stdin_length, sizeof (char)))
							   == (char *) HIPS_ERROR)
		      {
			  return (HIPS_ERROR);
		      }

		      strcpy (**(filename_ptr.filenames.list), "<stdin>");
		      break;

    }
    return(HIPS_OK);

}
