#ifndef lint
static char SccSID[] = "@(#)grle_expand.c	1.2 6/29/87 NYU Brain Research";
#endif
  
#include <sys/types.h>
#include <hipl_format.h>

#ifdef GRLE
  
  int
    grle_expand(grle, image, rows, cols)
  struct grlerun	**grle;
  u_char		*image;
  int		rows, cols;
{
  register		i;
  register u_char		*datptr, *outptr;
  register struct grlerun	*grleptr, **ptr;
  short int		y;
  
  i = rows * cols; 
  outptr = image; 
  while (i--) 
    *outptr++ = 0; 
  
  for (y = 0, ptr = grle; y < rows; y++, ptr++)
    if (*ptr)
      for (grleptr = *ptr; grleptr; grleptr = grleptr->next) {
	  outptr = image + cols * y + grleptr->x;
	  datptr = grleptr->data;
	  i = grleptr->length;
	  while (i--)
	    *outptr++ = *datptr++;
	}
  
  return(0);
}
#endif
