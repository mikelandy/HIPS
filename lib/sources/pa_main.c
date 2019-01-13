
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
** Filename: pa_main.c
**
** Description:	
**
**    This is the main source code file for the HIPS argument parser.
**
** Author:  David Wilson, Turing Institute, 30/1/91.
**
*/


/*
** Specify include files.
*/

#ifdef HUSESTDARG
#include <stdarg.h>
#else
#include <varargs.h>		/* UNIX variable argument header file. */
#endif

#include <hipl_format.h>	/* HIPS main header file. */

extern Flag_Key	*flag_table;
extern char	*filename_usage;

/*
**
** Name: parseargs
**
** Purpose: Library routine to parse the command line of a HIPS filter.
**	    The grammar of the command line accepted by HIPS filters is
**	    defined in the user documentation. Basically, it consists of
**	    an arbitrary number of flags, each of which may take an
**	    arbitrary number of parameters, followed by a list of
**	    trailing filenames. The flag list and/or the filename
**	    list may be empty. Various groups of flag options may be
**	    deemed as mutually exclusive.
**
** Inputs: A variable argument list is expected as input. This will contain
**	   in order the command line arguments passed to the filter, a
**	   format description of trailing filenames accepted by the
**	   filter, pointers to variables which are assigned default
**	   filename values or the filename values present on the command
**	   line, a format description for the flag options accepted by 
**	   the filter and pointers to variables which are assigned the
**	   default parameter values or the parameter values present on
**	   the command line.
**
**	   An example call is:
**
**		parseargs (argc,
**			   argv,
**			   flag_format,
**			   parameter_ptr1,
**			   ...,
**			   parameter_ptrN,
**			   file_format,
**			   file_ptr)
**
**	   Note that the flag format and filename format descriptors
**	   are assumed to be valid and no checking is performed on these
**	   descriptors.
**
**	   Note that the order in which the parameter pointers are given
**	   must match the order in which the parameters are defined in
**	   the flag format description.
**
** Return Value: The routine returns 0 on success or HIPS_ERROR if
**		 an error is detected. Note that error conditions are
**		 handled via perr () the HIPS error handling mechanism
**		 which may cause the routine to exit.
**
** External Dependencies:
**    varargs.h - UNIX variable argument header file.
**    hipl_format.h - HIPS main header file.
**
*/


#ifdef HUSESTDARG

int parseargs(int argc, ...)

{

#else

int parseargs (va_alist)

va_dcl

{
    int	     argc;
#endif

    /*
    ** External functions.
    */

    extern int	 get_flag_data ();
    extern int	 set_defaults ();
    extern int	 parse_command_line ();

    extern void  get_filename_data ();
    extern int	 build_usage_message ();


    /*
    ** Local functions variables.
    */

    char     **argv;

    extern int num_flags;

    va_list  varguments;


    /*
    ** Retrieve the command line arguments for the filter and the
    ** filename and flag option format descriptions and pointers.
    */

#ifdef HUSESTDARG
    va_start (varguments,argc);
#else
    va_start (varguments);

    argc = va_arg (varguments, int);
#endif
    argv = va_arg (varguments, char **);

    if (get_flag_data (&varguments) == HIPS_ERROR)
    {
	return (HIPS_ERROR);
    }

    get_filename_data (&varguments);

    va_end (varguments);

    /*
    ** Build a usage message for the filter.
    */

    if (build_usage_message () == HIPS_ERROR)
	return(HIPS_ERROR);

    /*
    ** Set the default operating mode and then change any modes given
    ** on the command line.
    */

    set_defaults ();

    if (parse_command_line (argc, argv) == HIPS_ERROR)
    {
	return (HIPS_ERROR);
    }

    /*
    ** Tidy up.
    */

    if (num_flags)
	    free (flag_table);

    return (0);

}

/* parseargs with user-supplied file usage message */

#ifdef HUSESTDARG
int parseargsu(int argc, ...)

{
#else

int parseargsu(va_alist)

va_dcl

{
	int argc;
#endif
	extern int	get_flag_data ();
	extern int	set_defaults ();
	extern int	parse_command_line ();
	extern void	get_filename_data ();
	extern int	build_usage_message ();

	char **argv;
	extern int num_flags;
	va_list varguments;

#ifdef HUSESTDARG
	va_start(varguments,argc);
#else
	va_start(varguments);
	argc = va_arg(varguments,int);
#endif
	argv = va_arg(varguments,char **);
	if (get_flag_data(&varguments) == HIPS_ERROR)
		return(HIPS_ERROR);
	get_filename_data(&varguments);
	filename_usage = strsave(va_arg(varguments,char *));
	va_end(varguments);

	build_usage_message();
	set_defaults();
	if (parse_command_line(argc,argv) == HIPS_ERROR)
		return(HIPS_ERROR);
        if (num_flags)
		free (flag_table);
	return(HIPS_OK);
}
