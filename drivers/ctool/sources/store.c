#ifndef lint
static char SccSID[] = "@(#)store.c	1.1 7/13/89";
#endif
/*
	Copyright 1989 Alan Shaw and Eric Schwartz.
	No part of this software may be distributed or sold without the prior
	agreement of Prof. Eric Schwartz, Dept. of Psychiatry, NYU School of
	Medicine, 550 1st Ave., New York, New York, 10016.
store.c
*/

#include <stdio.h>
#include <sys/types.h>
#include <values.h>

#include <hipl_format.h>

#define MAXBYTE	255
#define	MINBYTE	0
#define	MININT	(MAXINT + 1)

static char *funcname = "store";

byte_store(fp, hd, top)
  /* int */ FILE *fp;
  struct header	*hd;
  char		*top;
{
  funcname = "byte_store";
  return store(fp, hd->orows, hd->ocols,
	       hd->pixel_format, hd->sizepix, top, PFBYTE, 8);
}

/* //////////////////////////////////////////////////////////////////////// */

#ifdef	GRLE

grle_store(fp, hd, top)
  /*int*/ FILE *fp;
  struct header	*hd;
  char		*top;
{
  funcname = "grle_store";
  return store(fp, hd->orows, hd->ocols,
	       hd->pixel_format, hd->sizepix, top, PFGRLE, 8);
}

/* //////////////////////////////////////////////////////////////////////// */

srle_store(fp, hd, top)
  /*int*/ FILE *fp;
  struct header	*hd;
  char		*top;
{
  funcname = "srle_store";
  return store(fp, hd->orows, hd->ocols, 
	       hd->pixel_format, hd->sizepix, top, PFGRLE, 1);
}

static int OUTPOS;

#endif	GRLE

/* //////////////////////////////////////////////////////////////////////// */

store(fp, r, c, outformat, outbits, top, coreformat, corebits)
  FILE *fp;
  int	r, c, outformat, outbits, coreformat, corebits;
  char	*top;
{
  register u_char	*outbuf;
  u_char		*malloc();
  float			outbytes;
  u_char		*bytebuf;
  int			writeval;
  int			typesize;
  
#ifdef	GRLE
  
  register struct grlerun *grleptr, **ptr;
  register struct srlerun *srleptr, **sptr;
  u_char		*data;
  short			y, i;
  short			X, Y, Length;
  u_char		*inbuf, *line;
  int			INPOS, end;
  void			write_data();
  void			write_fields();
  
#endif	GRLE
  
  if ((outformat != PFBYTE
#ifdef	GRLE
       &&   outformat != PFGRLE
#endif	GRLE
       &&   outformat != PFINT
       &&   outformat != PFFLOAT
       &&   outformat != PFCOMPLEX)
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
  
  if (coreformat == PFCOMPLEX && outformat != PFCOMPLEX) {
      fprintf(stderr,
	      "%s: conversion from complex format is undefined\n",
	      funcname);
      return(-1);
    }
  
  switch(outformat)
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
  
  if (outformat == PFBYTE
      ||  outformat == PFINT
      ||  outformat == PFFLOAT
      ||  outformat == PFCOMPLEX) {
      outbytes = r * c * typesize;
      if (outformat == PFBYTE && outbits == 1)
	outbytes = r * ((c + 7) >> 3) * typesize;
      
      if (coreformat == outformat
	  &&  corebits == outbits) {
	  fwrite((char *)top, (int) outbytes, 1, fp);
	  return(0);
	}
      
      if ((outbuf = malloc((int) outbytes)) == NULL) {
	  fprintf(stderr,"%s: can't allocate core\n", funcname);
	  return(-1);
	}
      
#ifdef	GRLE
      
      if (coreformat == PFGRLE) {
	  if (outformat == PFBYTE && outbits == 8)
	    bytebuf = outbuf;
	  else if (outformat == PFINT
		   ||	 outformat == PFFLOAT
		   ||	 (outformat == PFBYTE && outbits == 1)) {
	      bytebuf = (u_char *)
		calloc(r * c, sizeof(u_char));
	      if (bytebuf == NULL) {
		  fprintf(stderr, "%s: can't allocate core\n", funcname);
		  return(-1);
		}
	    }
	  
	  if (corebits == 8)
	    grle_expand((struct grlerun **)top, bytebuf, r, c);
	  else if (corebits == 1)
	    srle_expand((struct srlerun **)top, bytebuf, r, c);
	}
      
#endif	GRLE
      
      if (coreformat == PFBYTE) {
	  if (outformat == PFBYTE && outbits == 8)
	    free(outbuf);
	  if (corebits == 8) {
	      bytebuf = (u_char *) top;
	      pack(bytebuf, outbuf, r, c);
	    }
	  else if (corebits == 1) {
	      bytebuf = (u_char *)
		malloc(r * c * sizeof(u_char));
	      if (bytebuf == NULL) {
		  fprintf(stderr, "%s: can't allocate core\n", funcname);
		  return(-1);
		}
	      unpack(top, bytebuf, r, c);
	    }
	  else {
	      fprintf(stderr, "%s: invalid format\n", funcname);
	      return(-1);
	    }
	  if (outformat == PFBYTE && outbits == 8)
	    outbuf = bytebuf;
	}
      
      if (outformat == PFINT) {
	  register int	*intptr = (int *) outbuf;
	  register	i = r * c;
	  
	  if  (coreformat == PFBYTE
#ifdef	GRLE
	       ||   coreformat == PFGRLE
#endif	GRLE
	       ) {
	      register u_char	*charptr = (u_char *) bytebuf;
	      
	      while (i--)
		*intptr++ = *charptr++;
	      
	      if ((coreformat == PFBYTE && corebits == 1)
#ifdef	GRLE
		  ||  (coreformat == PFGRLE)
#endif	GRLE
		  )
		free(bytebuf);
	      
	    }
	  else if (coreformat == PFFLOAT) {
	      register float	*floatptr = (float *) top;
	      int		underflows = 0, overflows = 0;
	      
	      while (i--) {
		  if (*floatptr > MAXINT)
		    overflows++;
		  else if (*floatptr < MININT)
		    underflows++;
		  *intptr++ = *floatptr++;
		}
	      showflows(underflows, overflows);
	    }
	}
      
      else if (outformat == PFFLOAT || outformat == PFCOMPLEX) {
	  register float	*floatptr = (float *) outbuf;
	  register	i;
	  
	  if (coreformat == PFINT) {
	      register int	*intptr = (int *) top;
	      
	      i = r * c;
	      while (i--)
		*floatptr++ = *intptr++;
	      if (outformat == PFCOMPLEX) {
		  i = r * c;
		  while (i--)
		    *floatptr++ = 0;
		}
	    }
	  else {
	      register u_char	*charptr = (u_char *) bytebuf;
	      
	      i = r * c;
	      while (i--)
		*floatptr++ = *charptr++;
	      if (coreformat == PFINT) {
		  i = r * c;
		  while (i--)
		    *floatptr++ = 0;
		}
	      
	      if ((coreformat == PFBYTE && corebits == 1)
#ifdef	GRLE
		  ||  (coreformat == PFGRLE)
#endif	GRLE
		  )
		free(bytebuf);
	    }
	}
      
      else if (coreformat == PFINT && outformat == PFBYTE) {
	  register int	*intptr = (int *) top;
	  register u_char	*charptr;
	  register	i = r * c;
	  int		underflows = 0, overflows = 0;
	  
	  if (outbits == 8)
	    bytebuf = outbuf;
	  else if (outbits == 1) {
	      bytebuf = (u_char *)
		malloc(r * c * sizeof(u_char));
	      if (bytebuf == NULL) {
		  fprintf(stderr, "%s: can't allocate core\n", funcname);
		  return(-1);
		}
	    }
	  charptr = (u_char *) bytebuf;
	  
	  while (i--) {
	      if (*intptr > MAXBYTE)
		overflows++;
	      else if (*intptr < MINBYTE)
		underflows++;
	      *charptr++ = *intptr++;
	    }
	  
	  showflows(underflows, overflows);
	  
	  if (outbits == 1) {
	      pack(bytebuf, outbuf, r, c);
	      free(bytebuf);
	    }
	}
      
      else if (coreformat == PFFLOAT && outformat == PFBYTE) {
	  register float	*floatptr = (float *) top;
	  register u_char	*charptr;
	  register	i = r * c;
	  int		underflows = 0, overflows = 0;
	  
	  if (outbits == 8)
	    bytebuf = outbuf;
	  else if (outbits == 1) {
	      bytebuf = (u_char *)
		malloc(r * c * sizeof(u_char));
	      if (bytebuf == NULL) {
		  fprintf(stderr, "%s: can't allocate core\n", funcname);
		  return(-1);
		}
	    }
	  charptr = (u_char *) bytebuf;
	  
	  while (i--) {
	      if (*floatptr > MAXBYTE)
		overflows++;
	      else if (*floatptr < MINBYTE)
		underflows++;
	      *charptr++ = *floatptr++;
	    }
	  
	  showflows(underflows, overflows);
	  
	  if (outbits == 1) {
	      pack(bytebuf, outbuf, r, c);
	      free(bytebuf);
	    }
	}
      
      else if (outformat == PFBYTE && outbits == 1) {
	  pack(bytebuf, outbuf, r, c);
#ifdef	GRLE
	  if (coreformat == PFGRLE)
	    free(bytebuf);
#endif	GRLE
	}
      
      fwrite(outbuf, (int) outbytes, 1, fp);
      free(outbuf);
      return(0);
    }
  
#ifdef	GRLE
  
  /*
    outformat == PFGRLE:
    */
  
  OUTPOS = 0;
  
  outbytes = r * c * 3.5;
  
  if ((outbuf = malloc((int) outbytes)) == NULL) {
      fprintf(stderr,"%s: can't allocate core\n", funcname);
      return(-1);
    }
  
  if (coreformat == PFGRLE && corebits == 8)
    for (y = 0, ptr = (struct grlerun **)top; y < r; y++, ptr++)
      for (grleptr = *ptr; grleptr; grleptr = grleptr->next) {
	  write_fields(outbuf, &grleptr->x, &y,
		       &grleptr->length);
	  if (outformat == PFGRLE && outbits == 8)
	    write_data(outbuf, grleptr->data, sizeof(char) * grleptr->length);
	  
	}
  else if (coreformat == PFGRLE && corebits == 1)
    for (y = 0, sptr = (struct srlerun **)top; y < r; y++, sptr++)
      for (srleptr = *sptr;srleptr; srleptr = srleptr->next) {
	  write_fields(outbuf, &srleptr->x, &y, 
		       &srleptr->length);
	  if (outformat == PFGRLE && outbits == 8) {
	      data = malloc(sizeof(char) *
			    srleptr->length);
	      for (i = 0; i < srleptr->length; i++)
		data[i] = 0xff;
	      write_data(outbuf, data, sizeof(char) * srleptr->length);
	      free(data);
	    }
	}
  else if (coreformat == PFBYTE
	   ||	 coreformat == PFFLOAT
	   ||	 coreformat == PFINT) {
      u_char	*unpacked;
      if (coreformat == PFBYTE && corebits == 1) {
	  if ((unpacked = malloc(r*c*sizeof(u_char))) == NULL) {
	      fprintf(stderr, "%s: can't allocate core\n", funcname);
	      return(-1);
	    }
	  unpack(top, unpacked, r, c);
	  top = (char *)unpacked;
	}
      else if (coreformat == PFFLOAT) {
	  register u_char	*charptr;
	  register float	*floatptr = (float *) top;
	  register	i = r * c;
	  int		underflows = 0, overflows = 0;
	  
	  if ((unpacked = malloc(r*c*sizeof(u_char))) == NULL) {
	      fprintf(stderr, "%s: can't allocate core\n", funcname);
	      return(-1);
	    }
	  charptr = unpacked;
	  while (i--) {
	      if (*floatptr > MAXBYTE)
		overflows++;
	      else if (*floatptr < MINBYTE)
		underflows++;
	      *charptr++ = *floatptr++;
	    }
	  showflows(underflows, overflows);
	  top = (char *)unpacked;
	}
      else if (coreformat == PFINT) {
	  register u_char	*charptr;
	  register int	*intptr = (int *) top;
	  register	i = r * c;
	  int		underflows = 0, overflows = 0;
	  
	  if ((unpacked = malloc(r*c*sizeof(u_char))) == NULL) {
	      fprintf(stderr, "%s: can't allocate core\n", funcname);
	      return(-1);
	    }
	  charptr = unpacked;
	  while (i--) {
	      if (*intptr > MAXBYTE)
		overflows++;
	      else if (*intptr < MINBYTE)
		underflows++;
	      *charptr++ = *intptr++;
	    }
	  showflows(underflows, overflows);
	  top = (char *)unpacked;
	}
      Y = 0;
      inbuf = (u_char *)top;
      INPOS = 0;
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
	      Length = end - X;
	      
	      write_fields(outbuf, &X, &Y, &Length);
	      if (outformat == PFGRLE && outbits == 8)
		write_data(outbuf, line + X, sizeof(char) * Length);
	      
	      X = end;
	    }
	  Y++;
	}
      if (corebits == 1)
	free(top);
    }
  else {
      fprintf(stderr, "%s: bad format argument\n", funcname);
      return(-1);
    }
  
  writeval = fwrite(outbuf, OUTPOS, 1, fp);
  free(outbuf);
  if (writeval != OUTPOS) {
      fprintf(stderr, "%s: write failed\n", funcname);
      return(-1);
    }
  
#endif	GRLE
  
  return(0);
}


/* //////////////////////////////////////////////////////////////////////// */

#ifdef	GRLE

static void write_data(outbuf, value, length)
  u_char		*outbuf;
  register u_char	*value;
  register int	length;
{
  register u_char *outspot;
  
  outspot = OUTPOS + outbuf;
  OUTPOS += length;
  
  while (length--)
    *outspot++ = *value++;
}

/* //////////////////////////////////////////////////////////////////////// */

static void write_fields(outbuf, x, y, length)
  u_char *outbuf, *x, *y, *length;
{
  register u_char *outspot;
  
  outspot = outbuf + OUTPOS;
  OUTPOS += 6;
  
  *outspot++ = *x++;
  *outspot++ = *x;
  *outspot++ = *y++;
  *outspot++ = *y;
  *outspot++ = *length++;
  *outspot   = *length;
}

#endif	GRLE

