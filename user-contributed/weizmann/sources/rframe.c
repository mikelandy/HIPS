/*     Copyright (c) 1982 Michael Landy, Yoav Cohen, and George Sperling

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/* rframe.c - read a frame(using the camera) from the IP-512, display it
 *            on the monitor and store it in a file (the standard output)
 *
 * Usage:
 *   rframe [-r rows] [-c cols] [-y initialrow] [-x initialcol] [-d dev]
 *   [-p pan] [-s scroll] [-b fbnum] [-h] [-m mask] [-G gain] [-L level]
 *   [-I]>frame
 *
 * Defaults:   rows: IPFB_SIZEY (=512), cols: IPFB_SIXEX (=512),
 *              initialrow: 0, initialcol: 0, dev=/dev/ipfb0a, pan=0,scroll=0,
 *              fbnum=MASTER_FB (=0), low byte(no h option), mask=0xff00.
 *              if dev==/dev/ipfb0 , then I,G,L  are not valid.
 *              if dev==/dev/ipfb1 , then  Gain=0x80, Level=0x08, not
 *              itarative (no I option);
 *
 * Load:       cc -O -I/horef/image/sys -o rframe rframe.c -lhipl
 *
 * Michael Landy/Yoav Cohen - 2/4/82
 * Modified: YC 6/9/82
 * Modified: Leah Mory for imaging Nov 1985; Aug 1986; October 1986
 * Modified: LM , February 1987. Added -L, -G, -I options. PLL for ipfb1.
 *
 * Reads a frame from the IP-512 starting at (initialrow,initialcol)
 * with size rows * cols.  There is a wraparound.
 * If using an ALU, the program increases pan and initialcol by ALU_DELAY(=11)
 * in order to correct the ALU pipeline delay.
 * Use mask in order to take a picture only into specific bits of the pixels.
 * That includes protecting the high or low byte of a 16-bits frame buffer.
 * Bits that are set in mask are not changed by the input ("protected").
 * -m is followed by hexadecimal value.
 * Pan and scroll are set before the picture is taken, and thus determine the
 * location of the taken picture in the frame buffer. x, y, c & r determine
 * the output picture origin and dimensions.
 * When taking a picture by device /dev/ipfb1, the camera supplies the clock
 * (PLL). When taking a picture by device /dev/ipfb0, XTAL supplies the clock
 * (internal). In any case, the user has to make sure that the camera is
 * connected properly. Another difference between the two devices is the
 * ability to set the GAIN and LEVEL registers. The AP-512 of /dev/ipfb0 is
 * such that the user cannot set these registers. /dev/ipfb1 allows the user
 * do that, using either the -G and -L options, or the -I option. The data for
 * -L and -G is hexadecimal.
 * -I allows the user tune the AP by setting the GAIN and LEVEL registers
 * while the picture is being acquired. If the user does not want to set them,
 * they default to GAIN=0x80; LEVEL=0x08 ;
 * Device /dev/ipfb0a does not initiate the camera for continuous grabbing
 * frames. Because of that the contents of frame buffers, other than fbnum,
 * are preserved. One can also
 * use other IP-512 devices (look at /dev/ipfb* to find all of them).
 * If their START_CAMERA bit is on, the contents of all the frame buffers of
 * the device are lost.
 * Thus, do not use /dev/ipfb0b, /dev/ipfb0d if you want only to take a
 * picture into fbnum, and you want to keep the picture which already
 * resides in the other frame buffers.
 */

#include <stdio.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <hipl_format.h>
#include <sundev/ipfbreg.h>

char *device = "/dev/ipfb0a"; /* ipfb0a is an alu configuration. It does not
                                initiates the FB-512 for continuous grabbing
                                frames. */
struct header hd;

main(argc,argv)
int argc;
char **argv;

{
   int getopt();
   extern char *optarg;
   extern int optind;

   int opt;
   int ir = 0, ic = 0, r = IPFB_SIZEX, c = IPFB_SIZEY, pan=0, scroll=0;
   int fbnum=MASTER_FB, high=0, mask=0xff00, rmask=0, gain=0x80, level= 0x08;
   int iterative=0;

   Progname = strsave(*argv);
   while ((opt = getopt (argc, argv, "r:c:x:y:d:p:s:b:hm:G:L:I")) != EOF)
      switch (opt)
      {
        case 'c':
           c = atoi (optarg);
           break;
        case 'r':
           r = atoi (optarg);
           break;
        case 'x':
           ic = atoi (optarg);
           break;
        case 'y':
           ir = atoi (optarg);
           break;
        case 'p':
           pan = atoi (optarg);
           break;
        case 's':
           scroll = atoi (optarg);
           break;
        case 'd':
           device=optarg;
           break;
        case 'b':
           fbnum = atoi (optarg);
           break;
        case 'h':
           high=1 ;
           break;
        case 'm':
            rmask=sscanf(optarg,"%x",&mask);
           break;
        case 'L':
            sscanf(optarg,"%x",&level);
           break;
        case 'G':
            sscanf(optarg,"%x",&gain);
           break;
        case 'I':
           iterative=1 ;
           break;
        case '?':
        default:
           fprintf (stderr,
 "Usage: %s -r rows -c cols -y initialrow -x initialcol -d dev -p pan \
-s scroll -b fbnum -h -m mask -G gain -L level -I\n",argv[0]);
              exit (2);
      }

       fprintf (stderr,"r= %d,c=%d,ir=%d,ic=%d\n",r,c,ir,ic);
       if((r<1)||(c<1)||(r>IPFB_SIZEY)||(c>IPFB_SIZEX))
        {
           fprintf(stderr,"%s: wrong dimensions.\n",argv[0]);
           exit(1) ;
       }
       init_header(&hd,"","",1,"",r,c,PFBYTE,1,"");
       update_header(&hd,argc,argv);
       rframe(r,c,ir,ic,pan,scroll,fbnum,high,mask,rmask,gain,level,iterative);
       return(0);
}


rframe(r,c,ir,ic,pan,scroll,fbnum,high,mask,rmask,gain,level,iterative)
int    r,c,ir,ic,pan,scroll,fbnum,high,mask,rmask,gain,level,iterative;
{
    char   *buf,
            ch;
    int     apcsr,
            fbcsr,
            fbopt,
            xtal,
            more;
    struct ipfb_box box;
    register    picsize,
                cwrite,
                twrite,
                ipfb;
    int     error;
    int     i,
            irr,
            icc;

    if ((ipfb = open (device, O_RDONLY)) < 0) {
       fprintf (stderr, "rframe: Couldn't open %s\n", device);
       exit (1);
    }
    picsize = c * r;
    if ((buf = (char *) calloc (picsize, sizeof (char))) == 0) {
       fprintf (stderr, "rframe:frame core allocation failed\n");
       close (ipfb);
       exit (1);
    }

    if ((error = ioctl (ipfb, IPFB_SFBUNIT, &fbnum)) == -1) {
       perror ("wrong frame buffer number");
       close (ipfb);
       exit (1);
    }

    if (high) {
       ioctl (ipfb, IPFB_GETOPT, &fbopt);
       fbopt |= IPFB_DATAHI;
       if ((error = ioctl (ipfb, IPFB_SETOPT, &fbopt)) == -1) {
           perror ("no HIGH on frame buffer");
           close (ipfb);
           exit (1);
       }
       if (!rmask)
           mask = 0xff;        /* do not read into low */

    }

    ioctl (ipfb, IPFB_SPAN, &pan);
    ioctl (ipfb, IPFB_SSCROLL, &scroll);
    ioctl (ipfb, IPFB_GETFLAGS, &fbopt);
    if (fbopt & IPFB_ALU) {
       ic = ic + ALU_DELAY;    /* correct ALU delay */
       pan = pan + ALU_DELAY;
    }
    box.ipfb_pan = ic;
    box.ipfb_scroll = ir;
    box.ipfb_xlen = c;
    box.ipfb_ylen = r;

    ioctl (ipfb, IPFB_SIOBOX, &box);
    ioctl (ipfb, IPFB_SMASK, &mask);

    if (!(xtal = strncmp ("/dev/ipfb1", device, sizeof "/dev/ipfb1" - 1))) {
       fprintf (stderr,
                "\nrframe NOTE: Device %s is used.\n             The sync \
for the camera is PLL (external).\n", device);
       ioctl (ipfb, IPFB_SLEVEL, &level);
       ioctl (ipfb, IPFB_SGAIN, &gain);
    /* set AP to PLL */
       ioctl (ipfb, IPFB_GAPCSR, &apcsr);
       apcsr &= ~IPFB_APCSRGL;
       ioctl (ipfb, IPFB_SAPCSR, &apcsr);
    }
    else
       fprintf (stderr,
                "\nrframe NOTE: Device %s is used.\n             The sync \
for the camera is XTAL (internal).\n             Options -G, -L, -I \
do not hold .\n", device);

    ioctl (ipfb, IPFB_GFBCSR, &fbcsr);
    if ((fbcsr & IPFB_CSRHCS) != IPFB_CSRHCCO) {
       fbcsr = (fbcsr & ~IPFB_CSRHCS) | IPFB_CSRHCCO;
       ioctl (ipfb, IPFB_SFBCSR, &fbcsr);
    }

    if (!xtal) {
       if (iterative) {
           fprintf (stderr, "\nTune the AP GAIN/LEVEL registers. \
Hit <CR> if you like current value.\n");
           while (more) {
               more = 0;
               ioctl (ipfb, IPFB_GGAIN, &gain);
               gain &= 0x000000ff;
               fprintf (stderr, "AP-GAIN (hex):  %02x\b\b", gain);
               while ((ch = getchar ()) == ' ');
               if (ch != '\n') {
                   ungetc (ch, stdin);
                   scanf ("%x", &gain);
                   while (getchar () != '\n');
                   ioctl (ipfb, IPFB_SGAIN, &gain);
                   more = 1;
               }
               ioctl (ipfb, IPFB_GLEVEL, &level);
               level &= 0x000000ff;
               fprintf (stderr, "AP-LEVEL (hex): %02x\b\b", level);
               while ((ch = getchar ()) == ' ');
               if (ch != '\n') {
                   ungetc (ch, stdin);
                   scanf ("%x", &level);
                   while (getchar () != '\n');
                   ioctl (ipfb, IPFB_SLEVEL, &level);
                   more = 1;
               }
           }
       }
       ioctl (ipfb, IPFB_GGAIN, &gain);
       gain &= 0x000000ff;
       ioctl (ipfb, IPFB_GLEVEL, &level);
       level &= 0x000000ff;
       fprintf (stderr, "\nAP-GAIN (hex)= %x, AP-LEVEL (hex)= %x\n",
                gain, level);
    }
    fprintf (stderr, "\nHit <CR> when the picture is focused");
    getchar ();
    ioctl (ipfb, IPFB_GFBCSR, &fbcsr);
    fbcsr = (fbcsr & ~IPFB_CSRHCS) | IPFB_CSRHCSF;
    ioctl (ipfb, IPFB_SFBCSR, &fbcsr);

    if (!xtal) {
    /* reset AP to XTAL */
       ioctl (ipfb, IPFB_GAPCSR, &apcsr);
       apcsr |= IPFB_APCSRGL;
       ioctl (ipfb, IPFB_SAPCSR, &apcsr);
    }

    ioctl (ipfb, IPFB_SPAN, &pan);
    read (ipfb, buf, picsize);
    close (ipfb);
    write_header (&hd);
    fwrite(buf,picsize,1,stdout);

}
