/*
 * convolve.c - perform a convolution on a 512X512 image using the ALU-512.
 *
 * Usage:
 * convolve -f filter_number [-d device] [-l] [-x initialcol] [-y initialrow]
 * [-I source_frame_buffer] [-T target_frame_buffer] [-M] [-L] [-C constant]
 * [-S scaling] [-A] [-F maskmode]< in-frame > out-frame
 * or
 * convolve -m mask_descriptor_name  [-d device] [-l] [-x initialcol]
 * [-y initialrow] [-I source_frame_buffer] [-T target_frame_buffer] [-M] [-L]
 * [-C constant] [-S scaling] [-A] [-F maskmode] <in-frame > out-frame
 *
 * Defaults: device=/dev/ipfb0a, display high, x=0, y=0, I=frame buffer 0,
 *           T=frame buffer 3, M off, L off, C=0, no scaling, A off.
 *           if -f, F=HIPL; if -m, F=IP512.
 *
 * Load:       cc -O aluop.o kernel.o fb_maxmin.o
 *                 -I/horef/image/sys -o convolve convolve.c -lhipl
 * Convolve transfers an 8-bits pixels image, from the standard input
 * to the source_frame_buffer. Then, it filters the image by applying a
 * convolution mask, using the ALU-512. The 16-bits output of the convolution
 * is stored in a 16-bits target_frame_buffer and copied to the standard
 * output. Usually the high byte of the result is displayed. The low byte is
 * displayed if, and only if, the -l option is set. The -f option specifies a
 * filter from a system library (kept in /horef/image/masks). The definition
 * for each of these filters is to be found in /horef/image/masks/maskn,
 * where n is the filter number. The -m switch allows a new filter to be
 * supplied by the user.  The format of the filter definition file may be
 * either one of the following formats:
 * 1.  "filter name"
 *     masksize 1 1
 *     mask
 *
 *   where the masksize is the length of a side of the mask (which must be
 *   square). "1" means that we apply only one mask, and the last "1" was
 *   chosen as an arbitrary function number. mask is given as a sequence of
 *   integers in column-fastest order. This format of the mask file is such in
 *   order to comply with the format for the commands mask and fmask, and a
 *   mask with this format is said to have mode=HIPL.
 * 2.  "filter name"
 *     masksizex masksizey
 *     mask
 *
 *   where masksizex is the number of columns of the mask, and masksizey is
 *   number of its rows. Mask can be non-square.
 *   Mask with this format is said to have mode=IP512.
 * When using the -f option, mode defaults to HIPL. When using the -m option,
 * mode defaults to IP512. One can use -F followed by 1 or 2 in order to
 * change it. HIPL is defined as 1; IP512 is defined as 2.
 *
 * x and y determine the origin of the source image. The result will be output
 * from this point as well.
 * Device must be one which has an ALU-512 and at least two frame buffers,
 * one of which must be a 16-bits frame buffer. Thus do not choose /dev/ipfb1.
 * It is also recommended to use the device exclusively while executing a
 * convolution. That is why you had better use /dev/ipfb0a. If you want to run
 * a sequence of convolutions without destroying the previous result, set the
 * -L option. Otherwise the target_frame_buffer will be cleared before the
 * operation begins. It either will be cleared to 0, or to the value of the -C
 * argument (if used). This is very useful when you want to get rid of
 * negative values in the result in order to have a meaningful display. The
 * other argument which is important for the display is -S, followed by a
 * positive integer which is less than 16. The resultant convolution will be
 * left-shifted by this value. This is done in order to get the 8 most
 * significant bits of the result in the high byte of the target_frame_buffer.
 * In order to find out what values of -C and -S to choose for the best
 * result, use the -M option first. Applying this option, the programm
 * calculates the minimal and the maximal pixels in the result. If the
 * minimum is negative, execute convolve againe, setting -C to the absolute
 * value of the minimum. That will make all the pixels non-negative. From the
 * maximum, printed out as hexadecimal, you can know how many unused bits are
 * left on the left of the high byte. Set -S to that number. No need to use
 * -M any longer, as it slows down the program.
 * An alternative way to achieve the same result, is to use the option A.
 * This option makes the program perform all the above steps automatically.
 * It prints out the const and scale values for future use.
 * Overflow is not taken care of.
 *
 * Hedva S.Hess - Sep. 1986
 * Modified: Leah Mory - Nov. 1986
 * Modified: Leah Mory - dec. 1986
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
unsigned char buf[IPFB_SIZEX*IPFB_SIZEY*2];
int  source_fb=0, target_fb=3;
int ipfb;

/**********************************************************/

main (argc,argv)
int argc;
char **argv;
{
    register int    ps;
    short int   max,
                min;
    char    fn[40],
           *device = "/dev/ipfb0a";/* the defualt device */
    int     r,
            c,
            picsize,
           tread,
           cread,
            fbopt,
            opt,
            mmode = 0,
            x = 0,
            y = 0,
            scale = 0,
            self = 0,
            maxmin = 0,
            showlow = 0,
            leave = 0,
            const = 0,
            fmode = 0;
    FILE * fp, *fopen ();

    int     getopt ();
    extern char *optarg;
    extern int  optind;

    int     error;
    extern  errno;

    Progname = strsave(*argv);
    if (argc == 1) {
       fprintf (stderr, "\nKernel file is reqired.\n");
       fprintf (stderr, "Usage:\nconvolve -f file_number | -m file_name \
[-x initialcol] [-y initialrow] [-d device] [-l low] [-S Scaling] \
[-I source_fb] [-T target_fb] [-L] [-C colour] [-M] [-A] [-F maskmode]< \
in_file > out_file \n");
       exit (1);
    }
    while ((opt = getopt (argc, argv, "x:y:S:f:m:d:I:T:C:LMlAF:")) != EOF)
       switch (opt) {
           case 'x':
               x = atoi (optarg);
               break;
           case 'y':
               y = atoi (optarg);
               break;
           case 'f':
               sprintf (fn, "/horef/image/masks/mask%s", optarg);
               mmode = HIPL;
               break;
           case 'm':
               strcpy (fn, optarg);
               mmode = IP512;
               break;
           case 'S':
               scale = atoi (optarg);
               break;
           case 'd':
               device = optarg;
               break;
           case 'l':
               showlow = 1;
               break;
           case 'I':
               source_fb = atoi (optarg);
               break;
           case 'T':
               target_fb = atoi (optarg);
               break;
           case 'C':
               const = atoi (optarg);
               break;
           case 'L':
               leave = 1;
               break;
           case 'M':
               maxmin = 1;
               break;
           case 'A':
               self = 1;
               break;
           case 'F':
               fmode = atoi (optarg);
               break;

           default:
               fprintf (stderr,"Usage:\nconvolve -f file number -m file name \
[-x initialcol] [-y initialrow ] [-d device] [-S Scaling] [-l] [-I source fb] \
[-T target fb]  [-L] [-C colour] [-M] [-A] [-F maskmode]< in_file > \
out_file \n");
               exit (1);

       }
    if (!mmode) {
       fprintf (stderr, "\nKernel file is reqired.\n");
       fprintf (stderr, "Usage:\nconvolve -f file_number | -m file_name \
[-s scroll] [-p pan] [-d device] [-l low] [-S Scaling] [-I source_fb] \
[-T target_fb] [-L] [-C colour] [-M] [-A] [-F maskmode]< in_file > \
out_file \n");
       exit (1);
    }

    if (!fmode)
       fmode = mmode;          /* mask format gets the default if not set
                                  */
    if (self)
       maxmin = leave = const = scale = 0;

/*read header before opening the device in order to enable pipe-line operation
*/
    read_header (&hd);
    r = hd.orows;
    c = hd.ocols;
    picsize = r * c;

    if (hd.pixel_format != PFBYTE) {
       fprintf (stderr, "convolve: frame must be in byte format.\n");
       exit (1);
    }

    if ((ipfb = open (device, O_RDWR)) < 0) {
       fprintf (stderr, "convolve: Couldn't open device: %s \n", device);
       exit (1);
    }

 /* execute mmap to get pointer to the device registers */
    ps = getpagesize ();
    if ((reg = (struct ipfb_reg *) valloc (ps)) == 0) {
       fprintf (stderr, "convolve: Can't allocate memory. \n");
       exit (1);
    }
    mmap (reg, ps, 7, MAP_SHARED, ipfb, 0);


 /* transfer image from standard input into the source frame buffer. */
    ioctl (ipfb, IPFB_SFBUNIT, &source_fb);
 /* clear source_fb */
    DATA (source_fb) = 0;
    CSR_HI (source_fb) = CSR_HI (source_fb) | (IPFB_CSRHCCLR >> 8);
    WAIT (source_fb);
    box.ipfb_pan = x;
    box.ipfb_scroll = y;
    box.ipfb_xlen = c;
    box.ipfb_ylen = r;
    ioctl (ipfb, IPFB_SIOBOX, &box);

 /* it is written like this, because the input may come from a socket,
    which has limited buffering */

    fread(buf,picsize,1,stdin);
    write (ipfb, buf, tread);
    lseek (ipfb, 0, 0);     /* kludge . to reposition device at 0,0 */

 /* input kernel */
    if ((fp = fopen (fn, "r")) == NULL) {
       fprintf (stderr, "convolve:Can't open kernel file %s\n", fn);
       exit (1);
    }
    read_mask (&mask, fp, fmode);

    conv (&mask, showlow, leave, const);
    sleep (3);
    if (scale)
       shift (target_fb, scale);

    if (maxmin || self)
       fb_maxmin (target_fb, r, c, y, x, &max, &min);
    if (self) {
       const = min < 0 ? -min : 0;
       conv (&mask, showlow, leave, const);
       fb_maxmin (target_fb, r, c, y, x, &max, &min);
       if (scale = chkmax (max))
           shift (target_fb, scale);
    }

/* transfer convolution result from target frame buffer to standard output.*/
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

    picsize *= 2;
    read (ipfb, buf, picsize);
    close (ipfb);
    write_header (&hd);
    write (buf, picsize,1,stdout);
    fprintf (stderr, "wrote on standard output %d bytes.\n", picsize);

    fprintf (stderr, "finished..\n");

}

/***********************************************************************/

int chkmax (max)
short int max;
{   register scale ;

    for (scale=0; (max & (0x8000 >> scale))==0 && scale <16; scale++);

    return(scale<16 ? scale : 0);
}

