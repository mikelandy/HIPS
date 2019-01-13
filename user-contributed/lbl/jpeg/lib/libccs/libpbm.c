/*
%	libpbm . c
*/

#ifdef	PGM_READ_ALL_OR_MEM

#include "pgm.h"

static bit*	bitrow;


pgm_readpgminit(fileP, colsP, rowsP, maxvalP, formatP)
FILE*	fileP;
int	*colsP, *rowsP, *formatP;
gray*	maxvalP;
{
register int	status;
	/* Check magic number */
	*formatP = pbm_readmagicnumber(fileP);
	switch (status=PGM_FORMAT_TYPE(*formatP))	{
        case PGM_TYPE:
		status = pgm_readpgminitrest(fileP, colsP, rowsP, maxvalP);
		break;
        case PBM_TYPE:
		status = 0;
		pbm_readpbminitrest(fileP, colsP, rowsP);
		*maxvalP = 1;
		bitrow = pbm_allocrow(*colsP);
		break;
	default:
		DEBUGMESSAGE("magic number %d - not a pgm file.\n", status);
	}
return	status;
}

gray**	pgm_readpgm(fileP, colsP, rowsP, maxvalP)
FILE*	fileP;
int*	colsP;
int*	rowsP;
gray*	maxvalP;
{
gray**	grays=NULL;
int	row, format;

    pgm_readpgminit(fileP, colsP, rowsP, maxvalP, &format);
    grays = pgm_allocarray(*colsP, *rowsP);
    for (row=0; row < *rowsP; ++row)
	pgm_readpgmrow(fileP, grays[row], *colsP, *maxvalP, format);
return grays;
}


char**
pm_allocarray(cols, rows, size)
int	cols, rows, size;
{
register char**	its;
register int	i;

    its = (char**) NZALLOC(rows, sizeof(char*), "pm-alloc**");
    its[0] = (char*) nzalloc(rows * cols, size, "pm_alloc*");
    for (i=1; i < rows; ++i)
	its[i] = &(its[0][i * cols * size]);
return	its;
}

void
pm_freearray(its, rows)
char**	its;
int	rows;
{
	CFREE(its[0]);
	CFREE(its);
}

#endif
