#ifndef lint
static char SccSID[] = "@(#)srle_expand.c	1.2 6/29/87 NYU Brain Research";
#endif
  
#include <sys/types.h>
#include <hipl_format.h>

#ifdef GLRE
  
  int
    srle_expand(srle, image, rows, cols)
  struct srlerun	**srle;
  u_char		*image;
  int		rows, cols;
{
  register		i;
  register u_char		*outptr;
  register struct srlerun	*srleptr, **ptr;
  short int		y;
  
  i = rows * cols;
  outptr = image;
  while (i--)
    *outptr++ = 0;
  
  for (y = 0, ptr = srle; y < rows; y++, ptr++)
    if (*ptr)
      for (srleptr = *ptr; srleptr; srleptr = srleptr->next) {
	  outptr = image + cols * y + srleptr->x;
	  i = srleptr->length;
	  while (i--)
	    *outptr++ = 0xff;
	}
  
  return(0);
}
#endif
