#include <stdio.h>
#include "convolve.h"

/* read a kernel from a file */
read_mask(mask,fp,mode)
struct mask *mask;
FILE *fp;
int mode;
{
    int     i,
            j,
            n,
            num,
            func;
    char   *s, *ker_ptr;

    s = mask -> name;
    while ((*s++ = getc (fp)) != '\n');

 /* read the dimension of the of kernel */
    if (mode == HIPL) /* the mask is as in /horef/image/masks */
       { fscanf (fp, "%d %d %d",&(mask->dx), &num, &func);
         mask->dy = mask->dx;
       }
    else /* mode= IP512 */
       fscanf (fp, "%d %d",&(mask->dx),&(mask->dy));
    mask->kernel = (char *) malloc (mask->dx * mask->dy);
    ker_ptr = mask->kernel;
    for (i = 0; i < mask->dy; i++) {/* read the kernel  from the file */
       for (j = 0; j < mask->dx; j++) {
           fscanf (fp, "%d", &n);
           *ker_ptr++ = (char) (n);
       }
    }
    close (fp);
}

/* read a kernel from screen */
input_mask(mask)
struct mask *mask;
{
    int     i,
            j,
            n,
            getnum;
    char   *ker_ptr, *s;

    printf ("mask name (enclosed in double quotes):\n ");
    s = mask -> name;
    while ((*s++ = getchar ()) != '\n');
    printf ("X dimension= ");
    scanf ("%d", &(mask -> dx));
    while (getchar () != '\n');
    printf ("Y dimension= ");
    scanf ("%d", &(mask -> dy));
    while (getchar () != '\n');
    mask -> kernel = (char *) malloc ((mask -> dx) * (mask -> dy));
    ker_ptr = mask -> kernel;
    for (i = 0; i < mask -> dy; i++)
       for (j = 0; j < mask -> dx; j++) {
           getnum = 1;
           while (getnum) {
               printf ("    KERNEL[%d][%d]= ", i, j);
               scanf ("%d", &n);
               while (getchar () != '\n');
               if ((n > 127) || (n < -128))
                 printf ("*** testconv: kernel value overflow. try again.\n");
               else
                   getnum = 0;
           }
           *ker_ptr++ = (char) (n);
       }
}

/* display the kernel */
display_mask(mask)
struct mask *mask;
{
    int     n,
           i,
            j;
    char   *ker_ptr, *s;
    printf(" current mask is: \n");
    s = mask -> name;
    while (putchar (*s++) != '\n');
    printf(" columns= %d, rows= %d\n",mask -> dx,mask -> dy);
    ker_ptr = mask -> kernel;
    for (i = 0; i < mask -> dy; i++) {
       printf("     ");
       for (j = 0; j < mask -> dx; j++)
          printf ("%4d%c",(int)(*ker_ptr++),(j == (mask->dx -1))? '\n' :' ');
    }
}

/* output kernel to a file*/
write_mask(mask,fp,mode)
struct mask *mask;
FILE *fp;
int mode;
{
    int     n,
           i,
            j,
            num = 1,
            func = 1;
    char   *s,
           *ker_ptr;

    s = mask -> name;
    while (putc (*s++, fp) != '\n');

 /* write the dimensions of the kernel */
    if (mode == HIPL)          /* the mask is as in /horef/image/masks */
       fprintf (fp, "%d %d %d\n", mask -> dx, num, func);
    else                       /* mode= IP512 */
       fprintf (fp, "%d %d\n", mask -> dx, mask -> dy);
    ker_ptr = mask -> kernel;
    /* write the kernel to the file */
    for (i = 0; i < mask -> dy; i++)
       for (j = 0; j < mask -> dx; j++)
          fprintf (fp,"%4d%c",(int)(*ker_ptr++),(j==(mask->dx -1))?'\n':' ');
    close (fp);
}

modify_mask(mask)
struct mask *mask;
{
    int     quit=0,
            i,
            j,
            n,
            getnum;
    char   c,*ker_ptr, *s;

    printf (
     " Type a new value, if you want to modify entry. Otherwise hit <CR>.\n ");
    printf (" mask name :  ");
    s = mask -> name;
    while (putchar (*s++) != '\n');
    printf (" new mask name :  ");
    while ((c=getchar ()) == ' ');
    if (c != '\n')
    {   ungetc(c,stdin) ;
        s= mask -> name ;
        while ((*s++ = getchar ()) != '\n');
    }
    printf ("X dimension= %-4d\b\b\b\b",mask->dx);
    while ((c=getchar ()) == ' ');
    if (c != '\n')
    {  ungetc(c,stdin) ;
       scanf ("%d", &(mask -> dx));
       while (getchar () != '\n');
    }
    printf ("Y dimension= %-4d\b\b\b\b",mask->dy);
    while ((c=getchar ()) == ' ');
    if (c != '\n')
    {  ungetc(c,stdin) ;
       scanf ("%d", &(mask -> dy));
       while (getchar () != '\n');
    }

    printf (
    " Type new value to modify element.\n Hit <CR> to leave it the same and \
to continue to next element. \n Type 'q' to quit modifying all together \
.\n");
    ker_ptr = mask -> kernel;
    for (i = 0; !quit && i < mask -> dy; i++)
       for (j = 0; !quit && j < mask -> dx; j++)
           {   printf (
                    "    KERNEL[%d][%d]=%-4d\b\b\b\b", i, j, (int)*ker_ptr++);
               while ((c=getchar ()) == ' ');
               if (c == 'q')
               {   quit=1;
                   while (getchar () != '\n');
                   break;
               }
               if ( c != '\n')
               {   ker_ptr--;
                   ungetc(c,stdin) ;
                   getnum = 1;
                   while (getnum)
                   {
                      scanf ("%d", &n);
                      while (getchar () != '\n');
                      if ((n > 127) || (n < -128))
                      {     printf ("*** testconv: overflow. try again.\n");
                            printf ("    KERNEL[%d][%d]= ", i, j);
                      }
                      else
                            getnum = 0;
                   }
                   *ker_ptr++ = (char) (n);
               }
           }
}

/* change one element */
modify_element(mask,row,col)
struct mask *mask;
int row,col;
{
    int     n,
            getnum;
    char   *ker_ptr;
    ker_ptr = mask -> kernel + row * (mask->dx) + col;
    printf ("    KERNEL[%d][%d]=%-4d\b\b\b\b", row, col, (int)*ker_ptr);
    getnum = 1;
    while (getnum)
    {   scanf ("%d", &n);
       while (getchar () != '\n');
       if ((n > 127) || (n < -128))
       {  printf ("*** testconv: overflow. try again.\n");
          printf ("    KERNEL[%d][%d]= ", row, col);
       }
       else
          getnum = 0;
    }
    *ker_ptr = (char) (n);
}


