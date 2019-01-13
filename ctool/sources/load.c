#ifndef lint
static char loadSID[] = "@(#)load.c	1.2 7/14/89";
#endif
/*
	Copyright 1989 Alan Shaw and Eric Schwartz.
	No part of this software may be distributed or sold without the prior
	agreement of Prof. Eric Schwartz, Dept. of Psychiatry, NYU School of
	Medicine, 550 1st Ave., New York, New York, 10016.
load.c
*/

#include	<stdio.h>
#include	<sys/types.h>
#include	<errno.h>
#include	<values.h>
  
#include	<hipl_format.h>

#define	MAXBYTE	255
#define	MINBYTE	0
#define	MININT	(MAXINT + 1)

#define L_SET	0
#define L_INCR	1
#define L_XTND	2


static char *funcname = "load";

byte_load(fp, hd, top)
  /* int */ FILE *fp;
  struct header	*hd;
  u_char		*top;
{
  int	value;
  
  funcname = "byte_load";
  value = load(fp, hd->orows, hd->ocols, hd->pixel_format, hd->sizepix,
	       (char **)top, PFBYTE, 8);
  hd->pixel_format = PFBYTE;
  return(value);
}

/* //////////////////////////////////////////////////////////////////////// */

#ifdef	GRLE

grle_load(fp, hd, top)
  /* int */ FILE *fp;
  struct header	*hd;
  struct grlerun **top;
{
  int	value;
  
  funcname = "grle_load";
  value = load(fp, hd->orows, hd->ocols, hd->pixel_format, hd->sizepix,
	       (char **)top, PFGRLE, 8);
  hd->pixel_format = PFGRLE;
  return(value);
}

/* //////////////////////////////////////////////////////////////////////// */

srle_load(fp, hd, top)
  /* int */ FILE *fp;
  struct header	*hd;
  struct grlerun **top;
{
  int	value;
  
  funcname = "srle_load";
  value = load(fp, hd->orows, hd->ocols, hd->pixel_format, hd->sizepix,
	       (char **)top, PFSRLE, 1);
  hd->pixel_format = PFSRLE;
  return(value);
}

#endif	GRLE

static int	C, Coreformat, Corebits, Informat, Inbits;
static char	**Top;

#ifdef	GRLE

static int	INPOS;
static short	temp[3];
static u_char	*outbuf;
static short	lasty;
static struct grlerun	*grleptr;
static struct srlerun	*srleptr;

#endif	GRLE

/* //////////////////////////////////////////////////////////////////////// */

load(fp, r, c, informat, inbits, top, coreformat, corebits)
  FILE *fp;
  int	r,c, informat, inbits, coreformat, corebits;
  char	**top;
{
  register 	i;
  register u_char	*inbuf;
  int		imglngth;
  int		lseek();
  int		maxinsize, typesize;
  int		preadval;
  
#ifdef	GRLE
  
  int		begin, end;
  u_char	*read_line(), *line;
  short		X, Y, Length;
  short		*x, *y, *length;
  int		readval;
  u_char	*grledata;
  
#endif	GRLE
  
  C = c, Coreformat = coreformat, Informat = informat, Top = top,
  Inbits = inbits;
  Corebits = corebits;
  
  if ((informat != PFBYTE
#ifdef	GRLE
       &&   informat != PFGRLE
#endif	GRLE
       &&   informat != PFINT
       &&   informat != PFFLOAT
       &&   informat != PFCOMPLEX)
      ||  (coreformat != PFBYTE
#ifdef	GRLE
	   &&   coreformat != PFGRLE
#endif	GRLE
	   &&   coreformat != PFINT
	   &&   coreformat != PFFLOAT
	   &&   coreformat != PFCOMPLEX)) {
      fprintf(stderr, "%s: invalid format\n", funcname);
      return(-1);
    }
  
  if (informat == PFCOMPLEX && coreformat != PFCOMPLEX)
    {
      fprintf(stderr, "%s: conversion from complex format is undefined\n",
	      funcname);
      return(-1);
    }

  switch(informat)
    {
    case PFBYTE:
      typesize = sizeof(u_char);
      break;
      
    case PFINT:
      typesize = sizeof(int);
      break;
      
    case PFFLOAT:
      typesize = sizeof(float);
      break;
      
    case PFCOMPLEX:
      typesize = 2 * sizeof(float);
      break;
    }

  switch(informat)
    {
    case PFBYTE:
      maxinsize = r * c * typesize;
      break;

    case PFINT:
    case PFFLOAT:
    case PFCOMPLEX:
      if (inbits != 8 * typesize)
	{
	  fprintf(stderr,
		  "%s: bits_per_pixel incompatible with input format\n",
		  funcname);
	  return(-1);
	}
      maxinsize = r * c * typesize;
      break;
      
#ifdef	GRLE
    case PFGRLE:
      maxinsize = (int)(3.5 * r * c * sizeof(char));
      break;
#endif	GRLE
    }

  if (informat == PFBYTE
      ||  informat == PFINT
      ||  informat == PFFLOAT
      ||  informat == PFCOMPLEX) {
      
      imglngth = r * c * typesize;
      
      if (coreformat	== informat
	  &&  corebits	== inbits) {
	  if (informat == PFBYTE && inbits == 1)
	    imglngth = r * (((c - 1) >> 3) + 1);
	  preadval = fread((char *)top, imglngth, 1, fp);
	  if (preadval * imglngth != imglngth) {
	      fprintf(stderr, "%s: tried to read %d bytes from file; got %d\n",
		      funcname, imglngth, preadval);
	      return(-1);
	    }
	  return(0);
	}
      
      if (informat	== PFBYTE
	  &&  inbits	== 1) {
	  u_char	*tempbuf;
	  int	templength = r * (((c - 1) >> 3) + 1);
	  
	  if ((tempbuf = (u_char *) malloc(templength)) == NULL) {
	      fprintf(stderr, "%s: can't allocate core\n", funcname);
	      return(-1);
	    }
	  
	  preadval = fread((char *)tempbuf, templength, 1, fp);
	  if (preadval * templength != templength) {
	      fprintf(stderr, "%s: tried to read %d bytes from file; got %d\n",
		      funcname, templength, preadval);
	      free(tempbuf);
	      return(-1);
	    }
	  
	  if (coreformat == PFBYTE && corebits == 8) {
	      unpack(tempbuf, (u_char *) top, r, c);
	      free(tempbuf);
	      return(0);
	    }
	  else {
	      if ((inbuf = (u_char *) malloc(imglngth))
		  == NULL) {
		  fprintf(stderr, "%s: can't allocate core\n", funcname);
		  return(-1);
		}
	      unpack(tempbuf, inbuf, r, c);
	      free(tempbuf);
	    }
	}
      
      else {
	  if ((inbuf = (u_char *) malloc(imglngth)) == NULL) {
	      fprintf(stderr, "%s: can't allocate core\n", funcname);
	      return(-1);
	    }
	  
	  preadval = fread((char *)inbuf, imglngth, 1, fp);
	  if (preadval * imglngth != imglngth) {
	      fprintf(stderr, "%s: tried to read %d bytes from file; got %d\n",
		      funcname, imglngth, preadval);
	      free(inbuf);
	      return(-1);
	    }
	}
      
      if (informat == PFBYTE && inbits == 8
	  &&  coreformat == PFBYTE && corebits == 1) {
	  pack(inbuf, (u_char *) top, r, c);
	  free(inbuf);
	  return(0);
	}
      
      if (informat == PFBYTE && coreformat == PFINT) {
	  register int	*intptr = (int *) top;
	  register u_char	*charptr = inbuf;
	  
	  i = r * c;
	  while (i--)
	    *intptr++ = *charptr++;
	  free(inbuf);
	  return(0);
	}
      
      if (informat == PFBYTE
	  &&  (coreformat == PFFLOAT || coreformat == PFCOMPLEX)) {
	  register float	*floatptr = (float *) top;
	  register u_char	*charptr = inbuf;
	  
	  i = r * c;
	  while (i--)
	    *floatptr++ = *charptr++;
	  if (coreformat == PFCOMPLEX) {
	      i = r * c;
	      while (i--)
		*floatptr++ = 0;
	    }
	  free(inbuf);
	  return(0);
	}
      
      if (informat == PFFLOAT && coreformat == PFINT) {
	  register int	*intptr = (int *) top;
	  register float	*floatptr = (float *) inbuf;
	  int		underflows = 0, overflows = 0;
	  
	  i = r * c;
	  while (i--) {
	      if (*floatptr > MAXINT)
		overflows++;
	      else if (*floatptr < MININT)
		underflows++;
	      *intptr++ = *floatptr++;
	    }
	  free(inbuf);
	  showflows(underflows, overflows);
	  return(0);
	}
      
      if (informat == PFFLOAT && coreformat == PFCOMPLEX) {
	  register float	*complexptr = (float *) top;
	  register float	*floatptr = (float *) inbuf;
	  
	  i = r * c;
	  while (i--)
	    *complexptr++ = *floatptr++;
	  i = r * c;
	  while (i--)
	    *complexptr++ = 0;
	  free(inbuf);
	  return(0);
	}
      
      if (informat == PFFLOAT && coreformat == PFBYTE) {
	  register float	*floatptr = (float *) inbuf;
	  register u_char	*byteptr;
	  
	  i = r * c;
	  
	  if (corebits == 8) {
	      int underflows = 0, overflows = 0;
	      
	      byteptr = (u_char *) top;
	      while (i--) {
		  if (*floatptr > MAXBYTE)
		    overflows++;
		  else if (*floatptr < MINBYTE)
		    underflows++;
		  *byteptr++ = *floatptr++;
		}
	      free(inbuf);
	      showflows(underflows, overflows);
	      return(0);
	    }
	  
	  else if (corebits == 1) {
	      u_char *bytebuf = (u_char *) malloc(r * c *
						  sizeof(u_char));
	      if (bytebuf == NULL) {
		  fprintf(stderr,
			  "%s: can't allocate core\n", funcname);
		  free(inbuf);
		  return(-1);
		}
	      byteptr = bytebuf;
	      while (i--)
		*byteptr++ = *floatptr++;
	      free(inbuf);
	      
	      pack(bytebuf, (u_char *) top, r, c);
	      free(bytebuf);
	      
	      return(0);
	    }
	}
      
      if (informat == PFINT && coreformat == PFBYTE) {
	  if (corebits == 1)
	    intpack((int *) inbuf, (u_char *) top, r, c);
	  else {
	      register u_char	*charptr = (u_char *) top;
	      register int	*intptr = (int *) inbuf;
	      int		underflows = 0, overflows = 0;
	      
	      i = r * c;
	      while (i--) {
		  if (*intptr > MAXBYTE)
		    overflows++;
		  else if (*intptr < MINBYTE)
		    underflows++;
		  *charptr++ = *intptr++;
		}
	      showflows(underflows, overflows);
	    }
	  free(inbuf);
	  return(0);
	}
      
      if (informat == PFINT
	  &&  (coreformat == PFFLOAT || coreformat == PFCOMPLEX)) {
	  register float *floatptr = (float *) top;
	  register int	*intptr = (int *) inbuf;
	  
	  i = r * c;
	  while (i--)
	    *floatptr++ = *intptr++;
	  if (coreformat == PFCOMPLEX) {
	      i = r * c;
	      while (i--)
		*floatptr++ = 0;
	    }
	  free(inbuf);
	  return(0);
	}
      
#ifdef	GRLE
      
      /*
	coreformat == PFGRLE:
	*/
      
      if (informat == PFINT) {
	  register u_char	*byteptr;
	  register int	*intptr;
	  int underflows = 0, overflows = 0;
	  u_char *bytebuf = (u_char *) malloc(r * c * sizeof(u_char));
	  if (bytebuf == NULL) {
	      fprintf(stderr, "%s: can't allocate core\n", funcname);
	      return(-1);
	    }
	  i = r * c;
	  byteptr = bytebuf;
	  intptr = (int *) inbuf;
	  while (i--) {
	      if (*intptr > MAXBYTE)
		overflows++;
	      else if (*intptr < MINBYTE)
		underflows++;
	      *byteptr++ = *intptr++;
	    }
	  free(inbuf);
	  showflows(underflows, overflows);
	  inbuf = bytebuf;
	}
      
      else if (informat == PFFLOAT) {
	  register u_char	*byteptr;
	  register float	*floatptr;
	  int		underflows = 0, overflows = 0;
	  u_char *bytebuf = (u_char *) malloc(r * c *
					      sizeof(u_char));
	  if (bytebuf == NULL) {
	      fprintf(stderr, "%s: can't allocate core\n",
		      funcname);
	      return(-1);
	    }
	  i = r * c;
	  byteptr = bytebuf;
	  floatptr = (float *) inbuf;
	  while (i--) {
	      if (*floatptr > MAXBYTE)
		overflows++;
	      else if (*floatptr < MINBYTE)
		underflows++;
	      *byteptr++ = *floatptr++;
	    }
	  free(inbuf);
	  showflows(underflows, overflows);
	  inbuf = bytebuf;
	}
      
      Y	= 0;
      INPOS	= 0;
      lasty	= -1;
      while (Y < r) {
	  X = 0;
	  line = inbuf + INPOS;
	  INPOS += c;
	  while (X < c) {
	      for (i = X; i < c; i++) {
		  if (*(line + i) == 0)
		    continue;
		  break;
		}
	      X = i;
	      if (i == c)
		continue;
	      for (end = X; end < c; end++) {
		  if (*(line + end) != 0)
		    continue;
		  break;
		}
	      Length = (end - X);
	      
	      if (write_line(X, Y, Length, line + X) == -1) {
		  free(inbuf);
		  return(-1);
		}
	      
	      X = end;
	    }
	  Y++;
	}
      free(inbuf);
      return(0);
      
#endif	GRLE
      
    }
  
#ifdef	GRLE
  
  /*
    informat == PFGRLE:
    */
  
  if (coreformat == PFBYTE) {
      outbuf = (u_char *)top;
    }
  else if (coreformat == PFINT || coreformat == PFFLOAT) {
      outbuf = (u_char *) calloc(r * c, sizeof(u_char));
      if (outbuf == NULL) {
	  fprintf(stderr, "%s: can't allocate core\n", funcname);
	  return(-1);
	}
    }
  
/*  if ((begin = lseek(fileno(fp), 0, L_INCR)) < 0) { */
  if ((begin = ftell(fp)) <= 0) {
      if (errno == ESPIPE) {
	  lasty = -1;
	  while ((readval = fread((char *)temp, 6, 1, fp)) == 1) {
	      x	= temp;
	      y	= temp + 1;
	      length	= temp + 2;
	      if ((*x < 0) || (*y < 0)    || (*length < 0)
		  ||  (*x > c-1)|| (*y > r-1)  || (*length > c)) {
		  fprintf(stderr, "%s: invalid grle data\n", funcname);
		  return(-1);
		}
	      if (coreformat == PFBYTE) {
		  
		  if (inbits == 8) {
		      preadval = fread((char *)(outbuf + *y * c + *x),
				       *length, 1, fp);
		      if (preadval * *length != *length) {
			  fprintf(stderr,
				  "%s: tried to read %d bytes from file; got %d\n",
				  funcname, *length, preadval);
			  return(-1);
			}
		    }
		  else if (inbits == 1) {
		      for (i = 0; i < *length; i++)
			*(outbuf + *y * c + *x + i) = 0xff;
		    }
		}
	      else if (coreformat == PFGRLE) {
		  if (corebits == 8) {
		      
		      if (*y != lasty) {
			  lasty = *y;
			  (struct grlerun *)top[lasty] = grleptr = (struct grlerun *)
			    malloc(sizeof(struct grlerun));
			}
		      else {
			  grleptr->next = (struct grlerun *)
			    malloc(sizeof(struct grlerun));
			  grleptr = grleptr->next;
			}
		      if (!grleptr) {
			  fprintf(stderr, "%s: can't allocate core\n", funcname);
			  return(-1);
			}
		      
		      grleptr->data = (u_char *) malloc((*length + 1) * sizeof(u_char));
		      if (!(grleptr->data)) {
			  fprintf(stderr, "%s: can't allocate core\n", funcname);
			  return(-1);
			}
		      if (inbits == 8) {
			  preadval = fread((char *)(grleptr->data), 
					   *length, 1, fp);
			  if (preadval * *length != *length) {
			      fprintf(stderr,
				      "%s: tried to read %d bytes from file; got %d\n",
				      funcname, *length, preadval);
			      return(-1);
			    }
			}
		      else
			for (i = 0; i < *length; i++)
			  grleptr->data[i] = 0xff;
		      
		      grleptr->data[*length] = '\0';
		      
		      grleptr->x	= *x;
		      grleptr->length	= *length;
		      grleptr->next	= NULL;
		      
		    }
		  else if (corebits = 1) {
		      
		      if (*y != lasty) {
			  lasty = *y;
			  (struct srlerun *)top[lasty] = srleptr = (struct srlerun *)
			    malloc(sizeof(struct srlerun));
			}
		      else {
			  srleptr->next = (struct srlerun *)
			    malloc(sizeof(struct srlerun));
			  srleptr = srleptr->next;
			}
		      if (!srleptr) {
			  fprintf(stderr, "%s: can't allocate core\n", funcname);
			  return(-1);
			}
		      if (inbits == 8) {
			  grledata = (u_char *) malloc((*length + 1) * sizeof(u_char));
			  if (!grledata) {
			      fprintf(stderr, "%s: can't allocate core\n", funcname);
			      return(-1);
			    }
			  preadval = fread((char *)grledata, *length, 1, fp);
			  if (preadval * *length != *length) {
			      fprintf(stderr,
				      "%s: tried to read %d bytes from file; got %d\n",
				      funcname, *length, preadval);
			      return(-1);
			    }
			  free(grledata);
			}
		      srleptr->x	= *x;
		      srleptr->length	= *length;
		      srleptr->next	= NULL;
		      
		    }
		}
	    }
	  if (readval != 0) {
	      fprintf(stderr, "%s: invalid grle data\n",
		      funcname);
	      return(-1);
	    }
	}
      else {
	  perror(funcname);
	  return(-1);
	}
    }
  
  else {	/* lseek to begin succeeded */
      
      if ((end = lseek(fileno(fp), 0, L_XTND)) < 0) {
	  perror(funcname);
	  return(-1);
	}
      imglngth = end - begin;
      
      if (lseek(fileno(fp), begin, L_SET) < 0) {
	  perror(funcname);
	  return(-1);
	}
      
      if ((inbuf = (u_char *) malloc(imglngth)) == NULL) {
	  fprintf(stderr, "%s: can't allocate core\n", funcname);
	  return(-1);
	}
      
/*      preadval = fread((char *)inbuf, imglngth, 1, fp);
      if (preadval * imglngth != imglngth) { */
      preadval = read(fileno(fp),(char *)inbuf, imglngth);
      if (preadval != imglngth) {
	  fprintf(stderr,
		  "%s: tried to read %d bytes from file; got %d\n",
		  funcname, imglngth, preadval);
	  free(inbuf);
	  return(-1);
	}
      
      INPOS = 0;
      
      lasty = -1;
      while (INPOS <  imglngth) {
	  line = read_line(inbuf, &x, &y, &length);
	  if ((*x < 0)	|| (*y < 0)	|| (*length < 0)
	      ||  (*x > c-1)	|| (*y > r-1)	|| (*length > c)) {
	      fprintf(stderr, "%s: invalid grle data\n",
		      funcname);
	      free(inbuf);
	      return(-1);
	    }
	  
	  if (write_line(*x, *y, *length, line) == -1) {
	      free(inbuf);
	      return(-1);
	    }
	}
      
      free(inbuf);
    }
  
  switch(coreformat)
    {
    case PFBYTE:
      if (Corebits == 1)
	pack(outbuf, outbuf, r, c);
      break;
      
    case PFINT:
    {
      register int	*intptr = (int *) top;
      register u_char	*charptr = outbuf;
      
      i = r * c;
      while (i--)
	*intptr++ = *charptr++;
      free(outbuf);
    }
      break;
      
    case PFFLOAT:
    {
      register float	*floatptr = (float *) top;
      register u_char	*charptr = outbuf;
      
      i = r * c;
      while (i--)
	*floatptr++ = *charptr++;
      free(outbuf);
    }
      break;
    }
  
#endif	GRLE
  
  return(0);
}


/* //////////////////////////////////////////////////////////////////////// */

#ifdef	GRLE

static u_char *read_line(inbuf, x, y, length)
  u_char	*inbuf;
  short	**x, **y, **length;
{
  register u_char	*utemp		= (u_char *)temp;
  register u_char	*position	= inbuf + INPOS;
  u_char 		*line;
  int i;
  
  for (i = 0; i < 6; i++)
    *(utemp + i) = *(position + i);
  
  *x	= temp;
  *y	= temp + 1;
  *length	= temp + 2;
  
  line	= inbuf + INPOS + 6;
  if (Inbits == 8)
    INPOS	+= **length + 6;
  else if (Inbits == 1)
    INPOS	+= 6;
  return(line);
}

/* //////////////////////////////////////////////////////////////////////// */

static write_line(x, y, length, line)
  short	x, y, length;
  u_char	*line;
{
  register	i;
  u_char	*grledata, *maxdata;
  
  switch (Coreformat)
    {
    case PFBYTE:
    case PFINT:
    case PFFLOAT:
	for (i = 0; i < length; i++)
	  *(outbuf + y * C + x + i) = ((Inbits == 1) ? 0xff : *line++);
	break;
      
    case PFGRLE:
      switch (Corebits)
	{
	case 8:
	  if (y != lasty) {
	      lasty = y;
	      (struct grlerun *)Top[lasty] =
		grleptr = (struct grlerun *)malloc(sizeof(struct grlerun));
	    }
	  else {
	      grleptr->next = (struct grlerun *)malloc(sizeof(struct grlerun));
	      grleptr = grleptr->next;
	    }
	  if (!grleptr) {
	      fprintf(stderr, "%s: can't allocate core\n", funcname);
	      return(-1);
	    }
	  grleptr->data = (u_char *)malloc((length + 1) * sizeof(u_char));
	  if (!(grleptr->data)) {
	      fprintf(stderr, "%s: can't allocate core\n", funcname);
	      return(-1);
	    }
	  grledata = grleptr->data;
	  if (Informat == PFGRLE && Inbits == 8
	      ||  Informat == PFBYTE
	      ||  Informat == PFINT
	      ||  Informat == PFFLOAT) {
	      maxdata = grledata + length;
	      while (grledata < maxdata)
		*grledata++ = *line++;
	    }
	  else if (Inbits == 1) {
	      for (i = 0; i < length; i++)
		*grledata++ = 0xff;
	    }
	  *grledata = '\0';
	  grleptr->x = x;
	  grleptr->length = length;
	  grleptr->next	= NULL;
	  break;
  
	case 1:
	  if (y != lasty) {
	      lasty = y;
	      (struct srlerun *)Top[lasty] = srleptr
		= (struct srlerun *)malloc(sizeof(struct srlerun));
	    }
	  else {
	      srleptr->next = (struct srlerun *)malloc(sizeof(struct srlerun));
	      srleptr = srleptr->next;
	    }
	  if (!srleptr) {
	      fprintf(stderr, "%s: can't allocate core\n", funcname);
	      return(-1);
	    }
	  srleptr->x = x;
	  srleptr->length = length;
	  srleptr->next	= NULL;
	  break;
	}
      }
  return(0);
}

#endif	GRLE

/* //////////////////////////////////////////////////////////////////////// */

/* lifted from bup.c: */

static u_char mask[8] = {128, 64, 32, 16, 8, 4, 2, 1};

unpack(bitbuf, bytebuf, r, c)
  u_char	*bitbuf, *bytebuf;
  int	r, c;
{
  register int	inbytecount, rowsize, insize, pad, bit;
  register u_char	*pin, *pout, *pmask;
  
  rowsize	= ((c - 1) >> 3) + 1;
  insize	= rowsize * r;
  pad	= 8 - ((rowsize * 8) - c);
  
  pin = bitbuf;
  pout = bytebuf;
  for (inbytecount = 0; inbytecount < insize; inbytecount++, pin++)
    if (pad && inbytecount % rowsize == rowsize - 1) {
	if (! *pin)
	  for (bit = 0; bit < pad; bit++)
	    *pout++ = '\0';
	else
	  for (bit = 0, pmask = mask; bit < pad; bit++)
	    *pout++ = (*pin & *pmask++) ? 255 : 0;
      }
    else {
	if (! *pin)
	  for (bit = 0; bit < 8; bit++)
	    *pout++ = '\0';
	else
	  for (bit = 0, pmask = mask; bit < 8; bit++)
	    *pout++ = (*pin & *pmask++) ? 255 : 0;
      }

}

/* //////////////////////////////////////////////////////////////////////// */

pack(bytebuf, bitbuf, r, c)
  u_char	*bitbuf, *bytebuf;
  int	r, c;
{
  int cnt = 0, i, j;
  u_char tbyte;
  
  i = r;
  while (i--) {
      j = c;
      tbyte = 0;
      cnt = 0;
      while (j--) {
	  if (*bytebuf++)
	    tbyte |= mask[cnt];
	  cnt++;
	  if (cnt == 8) {
	      *bitbuf++ = tbyte;
	      tbyte = 0;
	      cnt = 0;
	    }
	}
      if (cnt)
	*bitbuf++ = tbyte;
    }
}

/* //////////////////////////////////////////////////////////////////////// */

intpack(intbuf, bitbuf, r, c)
  register int	*intbuf;
  register u_char	*bitbuf;
  int		r, c;
{
  register        cnt = 0, i, j;
  /* register */ u_char tbyte;
  
  i = r;
  while (i--) {
      j = c;
      tbyte = 0;
      cnt = 0;
      while (j--) {
	  if (*intbuf++)
	    tbyte |= mask[cnt];
	  cnt++;
	  if (cnt == 8) {
	      *bitbuf++ = tbyte;
	      tbyte = 0;
	      cnt = 0;
	    }
	}
      if (cnt)
	*bitbuf++ = tbyte;
    }
}

/* //////////////////////////////////////////////////////////////////////// */

showflows(underflows, overflows)
  int	underflows, overflows;
{
  if (overflows > 0 || underflows > 0)
    fprintf(stderr, "%s: warning: ", funcname);
  if (overflows > 0)
    fprintf(stderr, "%d overflows", overflows);
  if (overflows > 0 && underflows > 0)
    fprintf(stderr, " and ");
  if (underflows > 0)
    fprintf(stderr, "%d underflows", underflows);
  if (overflows > 0 || underflows > 0)
    fprintf(stderr, " detected\n");
}

