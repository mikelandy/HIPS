/*
 * testconv.c - conversational program to perform a convolution on an image
 *              using the ALU-512.
 *
 * Usage:
 * testconv [-d device] [-I source_frame_buffer] [-T target_frame_buffer]
 *
 * Defaults: device=/dev/ipfb0a, I=frame buffer 0, T=frame buffer 3
 *
 * Load:       cc -O aluop.o kernel.o fb_maxmin.o
 *                 -I/horef/image/sys -o testconv testconv.c -lhipl
 *
 * testconv is a conversational version of convolve. It gives the user the
 * option to change the input source, handle the filter and make some other
 * decisions interactively.
 * In general, after an 8-bits pixels image is in the source_frame_buffer,
 * it filters the image by applying a convolution mask,
 * using the ALU-512. The 16-bits output of the convolution is stored in a
 * 16-bits target_frame_buffer. Device must be one which has an ALU-512 and
 * at least two frame buffers, one of which must be a 16-bits frame buffer.
 * Thus do not choose /dev/ipfb1. It is also recommended to use the device
 * exclusively while executing a convolution. That is why you had better use
 * /dev/ipfb0a.
 * In order to understand the logic of the program, read the manual for
 * convolve first, and try testconv out.
 * Leah Mory - April 1987
 *
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <hipl_format.h>

#define KERNEL KERNEL
#include <sundev/ipfbreg.h>
#include "convolve.h"

extern char *valloc();
struct ipfb_reg  *reg;
struct header hd;
struct ipfb_box   box;
struct mask mask;
int source_fb=0,target_fb=3;
char   *device = "/dev/ipfb0a";
int ipfb;
unsigned char buf[IPFB_SIZEX*IPFB_SIZEY*2];

/*********************************************************************/
main (argc,argv)
int argc;
char **argv;
{
    register    ps;
    short int   max,
                min;
    char    ch;
    int     r, c, x, y,
            i;
    int     opt,
            again = 1,
            scale = 0,
            showlow = 0,
            leave = 0,
            const = 0;
    int     getopt ();
    extern char *optarg;
    extern int  optind;
    int fd;

    Progname = strsave(*argv);
    while ((opt = getopt (argc, argv, "d:I:T:")) != EOF)
       switch (opt) {
           case 'd':
               device = optarg;
               break;
           case 'I':
               source_fb = atoi (optarg);
               break;
           case 'T':
               target_fb = atoi (optarg);
               break;
           default:
               printf (
                "Usage:testconv [-I source_fb] [-T target_fb] [-d device] \n");
               exit (1);

       }

    if ((ipfb = open (device, O_RDWR)) < 0) {
                               /* open the device ipfb to read & write */
       printf ("testconv:couldn't open device: %s .\n", device);
       exit (1);
    }

    ps = getpagesize ();

    if ((reg = (struct ipfb_reg *) valloc (ps)) == 0) {
       printf ("testconv: Can't allocate memory. \n");
       exit (1);
    }
    mmap (reg, ps, 7, MAP_SHARED, ipfb, 0);

    hd.orows=IPFB_SIZEY;
    hd.ocols=IPFB_SIZEX;
    x=y=0;
    while (again) {
       again=0;
       ask_for_frame (&x, &y);
       r = hd.orows;
       c = hd.ocols;
       show_buf (&showlow);
       printf ("\n\n Find minimum and maximum values of target FB? (y/[n])");
       while ((ch=getchar ()) == ' ');
       if (ch == 'y')
          fb_maxmin (target_fb, r, c,y, x, &max, &min);
       if (ch != '\n')
          while (getchar () != '\n');
       back_ground (&leave,&const);
        scale=0;
       printf ("\n\n Scale result? Hit <CR> if not. Type value if yes: ");
       while ((ch=getchar ()) == ' ');
       if (ch != '\n')
       {  ungetc(ch,stdin) ;
          scanf ("%d", &scale);
          while (getchar () != '\n');
          shift (target_fb, scale);
       }
       if (ask_for_kernel (&mask)){
           printf ("\n\n Convolve? (y/[n]) ");
           while ((ch=getchar ()) == ' ');
           if (ch == 'y')
               conv (&mask, showlow, leave, const);
           if (ch != '\n' )
               while (getchar () != '\n');
           save_result(x,y,showlow,argc,argv);
       }
       printf ("\n\n Another iteration? ([y]/n) ");
       while ((ch=getchar ()) == ' ');
       if (ch != 'n')
               again = 1;
       if (ch != '\n' )
               while (getchar () != '\n');

    }                          /* while again */

    close (ipfb);
}

/***********************************************************************/

ask_for_frame(x,y)
int *x, *y;
{  int again=1,picsize,nbytes;
	FILE *fp;
   char fn[40],ch;

   while ( again) {
       printf ("\n\n Input picture? (y/[n]) ");
       while ((ch=getchar ()) == ' ');
       if (ch == 'y')
       {  printf(" Picture file:  ");
          scanf("%s",fn);
          while ( getchar() != '\n' );
          if ((fp = fopen (fn, "r")) == NULL){
              printf ( "*** testconv:Can't open picture file %s\n", fn);
              continue;
          }
          fread_header (fp,&hd,fn);
          if (hd.pixel_format != PFBYTE) {
               printf ( "*** testconv: frame must be in byte format.\n");
               close(fd);
               continue;
          }
          picsize = hd.orows * hd.ocols ;

          /* transfer image from file into the source frame buffer. */
          if ((nbytes=fread (buf, picsize,1,fp)) == 1) {
              printf(" Hit <CR> if picture origin is at <0,0>.\n");
              printf(" Otherwise type the coordinates of the origin. \n ");
              printf(" x,y= ");
              while ((ch=getchar ()) == ' ');
              if (ch == '\n')
                 *x = *y = 0;
              else
              {  ungetc(ch,stdin);
                 scanf("%d  %d",x,y);
                 while (getchar () != '\n');
              }
              ioctl (ipfb, IPFB_SFBUNIT, &source_fb);
              /* clear source_fb */
              DATA (source_fb) = 0;
              CSR_HI (source_fb) = CSR_HI (source_fb) | (IPFB_CSRHCCLR >> 8);
              WAIT (source_fb);
              box.ipfb_pan = *x;
              box.ipfb_scroll = *y;
              box.ipfb_xlen = hd.ocols;
              box.ipfb_ylen = hd.orows;
              ioctl (ipfb, IPFB_SIOBOX, &box);
              if ( (nbytes = write (ipfb, buf, picsize)) < picsize)
                  printf(
                    "*** testconv:Couldn't write to device %s full picture.\n",
                         device);
              lseek (ipfb,0,0); /* kludge . to reposition device at 0,0 */
          }
          else
             printf (
               "*** testconv:Couldn't read from input-file %s full picture.\n",
                     fn);
          fclose(fp);
       }
       else {
          if (ch != '\n' )
               while (getchar () != '\n');
          again=0;
       }
   }
}

/************************************************************************/

save_result(x,y,showlow,argc,argv)
int argc;
char **argv;
int x, y, showlow;
{  int again=1, fbopt, r, c,picsize;
FILE *fp;
   char fn[40],ch;

   while ( again) {
       printf ("\n\n Save result? (y/[n]) ");
       while ((ch=getchar ()) == ' ');
       if (ch == 'y')
       {  printf(" Result file:  ");
          scanf("%s",fn);
          while ( getchar() != '\n' );
          if ((fp = fopen (fn, "w" )) == NULL){
              printf ( "*** testconv:Can't open result file %s\n", fn);
              continue;
          }
          /* transfer convolution result from target frame buffer to file.*/
          r = hd.orows;
          c = hd.ocols;
          init_header (&hd, "", "", 1, "", r, c, PFSHORT,1, "");
          update_header (&hd, argc, argv);
          ioctl (ipfb, IPFB_SFBUNIT, &target_fb);
          ioctl (ipfb, IPFB_GETOPT, &fbopt);
          fbopt |= IPFB_DATA16;
          if (!showlow)
             fbopt |= IPFB_DATAHI;
          ioctl (ipfb, IPFB_SETOPT, &fbopt);
          box.ipfb_pan = PAN (target_fb) + x;
          box.ipfb_scroll = SCROLL (target_fb) + y;
          box.ipfb_xlen = c;
          box.ipfb_ylen = r;
          ioctl (ipfb, IPFB_SIOBOX, &box);
          picsize = r * c * 2;
          if (read (ipfb, buf, picsize) < picsize)
             printf (
                   "*** testconv:Couldn't read from device %s full picture.\n",
                     device);
          else
          {    fwrite_header (fp,&hd,fn);
               if ( fwrite (buf, picsize,1,fp) != 1)
                  printf(
                     "*** testconv:Couldn't write to file %s full picture.\n",
                         fn);
               else
                  again=0;
               fclose(fp);
          }
       }
       else
       {  if (ch != '\n' )
               while (getchar () != '\n');
          again = 0;
       }
   }
}

/************************************************************************/

 int ask_for_kernel(mask)
 struct mask *mask;
 {  FILE *fp, *fopen();
    int op,n,again = 1,more=1,row,col,mode;
    char fn[40],c;
    static int exist = 0;
  while ( again)
  { printf("\n\n OPTIONS FOR THE KERNEL:");
    printf("\n     1. Input kernel from file in current directory.");
    printf("\n     2. Input kernel from /horef/image/masks/ directory.");
    printf("\n     3. Input kernel from screen.");
    printf("\n     4. Use previous kernel with no change.");
    printf("\n     5. Modify kernel .");
    printf("\n     6. Display kernel.");
    printf("\n     7. Save kernel.");
    printf("\n Choose one of the options or hit <CR> to continue-->");
    while ((c=getchar ()) == ' ');
    if (c == '\n')
       break ;
    ungetc(c,stdin);
    scanf("%d",&op);
    while ( getchar() != '\n' );
    switch (op)
    {  case 1:
              printf("  Name of the file: ");
              scanf("%s",fn);
              while ( getchar() != '\n' );
              if ((fp = fopen (fn, "r")) == NULL)
                  printf ( "testconv:Can't open kernel file %s\n", fn);
               else {
                 read_mask(mask,fp,IP512);
                 exist=1;
               }
               break;
       case 2:
              printf("  Mask number: ");
              scanf("%d",&n);
              while ( getchar() != '\n' );
               sprintf(fn,"/horef/image/masks/mask%d",n);
              if ((fp = fopen (fn, "r")) == NULL)
                  printf ("\n*** testconv:Can't open kernel file %s\n", fn);
              else {
                 read_mask(mask,fp,HIPL);
                 exist=1;
              }
              break;
       case 3:
              input_mask(mask);
              exist=1;
              break;
       case 4:
              if (exist)
                 again=0;
              else
                 printf("\n*** testconv: no previuos mask.\n");
              break;
       case 5:
              if (!exist)
                 printf("\n*** testconv: no previuos mask.\n");
              else
              {   printf(" Hit <CR> if you want to scan the kernel.\n");
                  printf(
                    " Otherwise type the coordinates of the element you \
want to modify.\n First element is <0,0>.\n");
                  printf("  row  col= ");
                  while ((c=getchar ()) == ' ');
                  if (c == '\n')
                     modify_mask(mask);
                  else
                  {  more=1;
                     ungetc(c,stdin);
                     while (more)
                     {  more=0;
                        scanf("%d %d",&row,&col);
                        while (getchar () != '\n');
                        if (row < mask->dy && col < mask->dx)
                           modify_element(mask,row,col);
                        else
                        {   printf(" row or col out of range.\n row must \
be <%d  . col must be <%d\n",mask->dy,mask->dx);
                            printf("  row  col= ");
                            more=1;
                        }
                     }
                  }
              }
              break;

       case 6:
              if (exist)
                 display_mask(mask);
              else
                 printf("\n*** testconv: no previuos mask.\n");
              break;
       case 7:
              if (!exist)
                 printf("\n*** testconv: no previuos mask.\n");
              else
              {   printf("  Name of the file: ");
                  scanf("%s",fn);
                  while (getchar () != '\n');
                  if ((fp = fopen (fn, "w")) == NULL)
                  {  printf (
                           "\n*** testconv:Can't open output kernel file %s\n",
                             fn);
                     break;
                  }
                  printf(
              " Hit <CR> if kernel mode=IP512. Anything else if mode=HIPL.\n");
                  while ((c=getchar ()) == ' ');
                  if (c == '\n')
                     mode=IP512;
                  else {
                      mode=HIPL;
                      while (getchar () != '\n');
                  }
                  write_mask(mask,fp,mode);
              }
              break;
       default:
              printf ("\n*** testconv:Error in option,try again.\n");
              break;
       }
    } /* while again */
    if (!exist)
       printf ("\n*** testconv: no mask was chosen\n");
    return(exist);
 }

/***********************************************************************/

back_ground(leave,const)
int *leave, *const ;
 {
  int n,again=1;
  char c;
     printf("\n\n INITIAL BACKGROUND:");
     printf("\n      0. clear target frame buffer to 0.");
     printf("\n      1. clear target frame buffer to a constant.");
     printf("\n      2. leave frame buffer as it is.");
     while (again) {
        again = 0;
       printf("\n Choose one of the options or hit <CR> to continue-->");
       while ((c=getchar ()) == ' ');
        if (c == '\n')
               break ;
       ungetc(c,stdin);
        scanf("%d",&n);
        while ( getchar() != '\n' );
        switch (n)
        {  case 0:
            *const=0;
            *leave=0;
            break;
           case 1:
             printf(" Constant = ");
            scanf("%d",const);
            while ( getchar() != '\n' );
            *leave=0;
             break;
           case 2:
             *leave=1;
            break;
           default:
            printf ("*** testconv:Error in option,try again.\n");
            again = 1;
            break;
       }

     }
 }
/***********************************************************************/

show_buf(low)
int *low;
{
    char    c,
            fbuf[4],
           source_ch,
            target_high,
            target_low;
    int     n,
            again = 1;
 /* find out the ALU chanels of the high and low frame buffers. */
    ioctl (ipfb, IPFB_GFBALUCH, fbuf);

    target_high = (((fbuf[target_fb] & 0xf0) >> 4) & 0x0f);
    target_low = (fbuf[target_fb] & 0x0f);
    source_ch = (fbuf[source_fb] & 0x0f);

    while (again) {
       printf ("\n\n FRAME BUFFER TO VIEW :");
       printf ("\n        0. Source frame buffer.");
       printf ("\n        1. Target frame buffer low.");
       printf ("\n        2. Target frame buffer high.");
       printf ("\n Choose one of the options or hit <CR> to continue-->");
       while ((c = getchar ()) == ' ');
       if (c == '\n')
           break;
       ungetc (c, stdin);
       scanf ("%d", &n);
       while (getchar () != '\n');
       switch (n) {
           case 0:
               ALU_IN3 = (char) source_ch;
               break;
           case 1:
               ALU_IN3 = (char) target_low;
               *low = 1;
               break;
           case 2:
               ALU_IN3 = (char) target_high;
               *low = 0;
               break;
           default:
               printf ("*** testconv:Error in option,try again.\n");
               break;
       }
    }
}

