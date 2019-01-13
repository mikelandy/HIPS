#ifndef lint
static char SccSID[] = "%W% %G%";
#endif
/*
	Copyright 1988 Alan Shaw and Eric Schwartz.
	No part of this software may be distributed or sold without the prior
	agreement of Prof. Eric Schwartz, Dept. of Psychiatry, NYU School of
	Medicine, 550 1st Ave., New York, New York, 10016.
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<hipl_format.h>
#include	"cscan.h"

void peanocurve();

int main(argc, argv)
  int argc;
  char *argv[];
{
  FILE		*fp;
  extern u_char	*curve;
  extern short	xyz[LEVELS][LEVELS][LEVELS];
  
  Progname = strsave(*argv);
  
  peanocurve();
  
  fp = ffopen(SCANFILE, "w");
  fwrite(curve, 3 * COLORS * sizeof(u_char),1,fp);
  fclose(fp);
  
  fp = ffopen(XYZFILE, "w");
  fwrite(xyz, COLORS * sizeof(short),1,fp);
  fclose(fp);
  
  exit(0);
}

