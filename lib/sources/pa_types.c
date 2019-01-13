
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
** Filename: pa_types.c
**
** Description:	
**
**    This file contains the token classification source code for the
**    HIPS argument parser.
**
** Author:  David Wilson, Turing Institute, 30/1/91.
**
*/


/*
** Specify include files.
*/


#include <stdio.h>		/* UNIX standard I/O header file. */
#include <ctype.h>		/* UNIX character classification header file. */
#include <hipl_format.h>	/* HIPS main header file. */




/*
**
** Name: isboolean
**
** Purpose: To verify if string contains a valid boolean literal.
**	    Valid boolean literals are either "TRUE"  or "FALSE".
**
** Inputs: The routine expects the following:
**
**		string - char *, the string to be validated.
**
** Return Value: The routine returns TRUE if the string represents a
**		 valid character and FALSE otherwise.
**
** External Dependencies:
**    hipl_format.h - HIPS argument parser header file.
**
*/


h_boolean
isboolean (string)

char  *string;

{

    if (string == (char *) NULL ||
	(strcmp (string, "TRUE") != 0 &&
	strcmp (string, "FALSE") != 0))
    {
	return (FALSE);
    }

    return (TRUE);

}




/*
**
** Name: ischaracter
**
** Purpose: To verify if string contains a valid character literal.
**	    Valid character strings are 1 character in length and contain
**	    no control keys.
**
** Inputs: The routine expects the following:
**
**		string - char *, the string to be validated.
**
** Return Value: The routine returns TRUE if the string represents a
**		 valid character and FALSE otherwise.
**
** External Dependencies:
**    hipl_format.h - HIPS argument parser header file.
**
*/


h_boolean
ischaracter (string)

char  *string;

{

    if (string == (char *) NULL ||
	iscntrl (TOascii (*string)) != 0 ||
	strlen (string) !=  1)
    {
	return (FALSE);
    }

    return (TRUE);

}




/*
**
** Name: isstring
**
** Purpose: To verify if string contains a valid character string
**	    literal. Valid character string literals contain no
**	    control keys.
**
** Inputs: The routine expects the following:
**
**		string - char *, the string to be validated.
**
** Return Value: The routine returns TRUE if the string represents a
**		 valid character string and FALSE otherwise.
**
** External Dependencies:
**    hipl_format.h - HIPS argument parser header file.
**
*/


h_boolean
isstring (string)

char  *string;

{

    if (string == (char *) NULL)
    {
	return (FALSE);
    }

    while (*string != '\0')
    {
	if (iscntrl (TOascii (*string)) != 0)
	{
	    return (FALSE);
	}
	string++;
    }

    return (TRUE);

}




/*
**
** Name: isinteger
**
** Purpose: To verify if string contains a valid integer literal.
**	    Valid integer literals comprise an optional leading sign
**	    character followed by one or more digits.
**
** Inputs: The routine expects the following:
**
**		string - char *, the string to be validated.
**
** Return Value: The routine returns TRUE if the string represents a
**		 valid integer and FALSE otherwise.
**
** External Dependencies:
**    hipl_format.h - HIPS argument parser header file.
**
*/


h_boolean
isinteger (string)

char  *string;

{

    if (string == (char *) NULL)
    {
	return (FALSE);
    }

    if (*string == '+' || *string == '-')
    {
	string++;
    }

    do
    {
	if (isdigit (TOascii (*string)) == 0)
	{
	    return (FALSE);
	}
	string++;

    } while (*string != '\0');

    return (TRUE);

}




/*
**
** Name: isreal
**
** Purpose: To verify if string contains a valid real number literal.
**	    Valid real numbers comprise an optional leading sign followed
**	    by a real number in either exponent or decimal point
**	    notation.
**
** Inputs: The routine expects the following:
**
**		string - char *, the string to be validated.
**
** Return Value: The routine returns TRUE if the string represents a
**		 valid real number and FALSE otherwise.
**
** External Dependencies:
**    hipl_format.h - HIPS argument parser header file.
**
*/


h_boolean
isreal (string)

char  *string;

{

    h_boolean  leading_digits = FALSE;


    if (string == (char *) NULL)
    {
	return (FALSE);
    }

    if (*string == '+' || *string == '-')
    {
	string++;
    }

    if (isdigit (TOascii (*string)) != 0)
    {
	leading_digits = TRUE;

	do
	{
	    string++;
	} while (isdigit (TOascii (*string)) != 0);
    }

    if (*string == '\0' &&
	leading_digits == TRUE)
    {
	return (TRUE);
    }
    if (*string ==  '.')
    {
	string++;
    }
    if (!leading_digits && !isdigit(TOascii(*string)))
	return(FALSE);
    while (isdigit(TOascii(*string)))
	string++;
    if (*string == '\0')
	return(TRUE);
    if (*string == 'e' || *string == 'E')
    {
	string++;

	if (*string ==  '+' || *string == '-')
	{
	    string++;
	}
    }
    else
	return (FALSE);

    do
    {
	if (isdigit (TOascii (*string)) == 0)
	{
	    return (FALSE);
	}
	string++;

    }while (*string != '\0');

    return (TRUE);

}




/*
**
** Name: isfilename
**
** Purpose: To verify if string is a valid filename literal.
**
** Inputs: The routine expects the following:
**
**		string - char *, the string to be validated.
**
** Return Value: The routine returns TRUE if the string represents a
**		 valid filename and FALSE otherwise.
**
** External Dependencies:
**    hipl_format.h - HIPS argument parser header file.
**
*/


h_boolean
isfilename (string)

char  *string;

{
    if (string == (char *) NULL)
    {
	return (FALSE);
    }

    return(isstring(string));
}




/*
**
** Name: isflag
**
** Purpose: To verify if string contains a valid flag literal.
**	    A valid flag must begin with a '-' followed by a
**	    character string.
**
** Inputs: The routine expects the following:
**
**		string - char *, the string to be validated.
**
** Return Value: The routine returns TRUE if the string represents a
**		 valid flag and FALSE otherwise.
**
** External Dependencies:
**    hipl_format.h - HIPS argument parser header file.
**
*/


h_boolean
isflag (string)

char  *string;

{

    if (isstring (string) == FALSE ||
	*string != '-' ||
	strlen (string) < 2)
    {
	return (FALSE);
    }

    return (TRUE);

}
