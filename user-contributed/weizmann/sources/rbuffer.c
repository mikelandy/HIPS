/*
 * Copyright (c) 1982 Michael Landy, Yoav Cohen, and George Sperling
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All
 * the software has been tested extensively and every effort has been
 * made to insure its reliability.
 */

/*
 * rbuffer.c - read a buffer from the IP-512
 *
 * Usage: rbuffer [-r rows] [-c cols] [-y initialrow] [-x initialcol]
 * [-d dev] [-b fbnum] [-h] [-t]>frame
 *
 * Defaults:   rows: IPFB_SIXEY (=512), cols:IPFB_SIZEX (=512),
 * initialrow: 0, initialcol: 0, dev=/dev/ipfb0a, fbnum=MASTER_FB
 * (=0), low byte(no h option) 8-bits (no t option)
 *
 * Load:       cc -O -I/horef/image/bin -o rbuffer rbuffer.c -lhipl
 *
 * Michael Landy/Yoav Cohen - 2/4/82 Modified: YC 6/9/82 Leah Mory for
 * imaging July 1986;aug 1986;November 1986
 *
 * rbuffer reads buffer fbnum from the IP-512,  starting at
 * (initialrow,initialcol) with size rows * cols, and stores it in
 * the standard output file. (initialrow, initialcol) specifies the
 * position in the frame buffer where the read starts from. If the
 * picture does not fit the buffer due to a non-zero origin, there
 * will be wraparound.  Thus, if a picture happens to reside not in
 * the upper left corner of the frame buffer, e.g. it is right
 * shifted as a result of an ALU operation, setting
 * (initialrow,initialcol) to the picture's origin will produce a
 * file with no offset. h and t are relevant only if fbnum is a
 * 16-bits buffer. If t is on, the output file will be made of
 * 16-bits pixels. If t is off, the low byte is read. if t is off and
 * h is on,  the high byte is read. device /dev/ipfb0a does not
 * initiate the camera for continuous grabbing frames. That is why
 * the frame buffer's contents are preserved. One can also use other
 * devices, as long as their START_CAMERA bit is not on. Thus, do not
 * use /dev/ipfb0b, /dev/ipfb0d, /dev/ipfb1b, /dev/ipfb1d.
 */

#include <stdio.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <hipl_format.h>
#include <sundev/ipfbreg.h>

char *device = "/dev/ipfb0a";  /* ipfb0a is an alu configuration. It
                                * does not initiate the camera for
                                * continuous grabbing frames. */
struct header hd;

main(argc, argv)
   int argc;
   char **argv;

{
   int getopt();
   extern char *optarg;
   extern int optind;
   int r, c, ir, ic, or, oc, opt;

   int pan = 0;
   int fbnum = MASTER_FB;
   int high = 0;
   int twobytes = 0;

   Progname = strsave(*argv);
   ir = 0;
   ic = 0;
   r = IPFB_SIZEY;
   c = IPFB_SIZEX;
   while ((opt = getopt(argc, argv, "r:c:x:y:d:b:ht")) != EOF)
      switch (opt)
      {
      case 'c':
        c = atoi(optarg);
        break;
      case 'r':
        r = atoi(optarg);
        break;
      case 'x':
        ic = atoi(optarg);
        break;
      case 'y':
        ir = atoi(optarg);
        break;
      case 'd':
        device = optarg;
        break;
      case 'b':
        fbnum = atoi(optarg);
        break;
      case 'h':
        high = 1;
        break;
      case 't':
        twobytes = 1;
        break;
      case '?':
      default:
        fprintf(stderr,
                "Usage: rbuffer -r rows -c cols -y initialrow -x initialcol \
-d dev -b fbnum -h -t\n");
        exit(2);
      }

   fprintf(stderr, "rbuffer: r= %d,c=%d,ir=%d,ic=%d\n", r, c, ir, ic);
   if ((r < 1) || (c < 1) || (r > IPFB_SIZEY) || (c > IPFB_SIZEX))
   {
      fprintf(stderr, "rbuffer: wrong dimensions.\n");
      exit(1);
   }
   if (!twobytes)
      init_header(&hd, "", "", 1, "", r, c, PFBYTE,1, "");
   else
      init_header(&hd, "", "", 1, "", r, c, PFSHORT,1, "");
   update_header(&hd, argc, argv);
   rbuffer(r, c, ir, ic, fbnum, high, twobytes);
   return (0);
}


rbuffer(r, c, ir, ic, fbnum, high, twobytes)
   int r, c, ir, ic, fbnum, high, twobytes;
{
   char *buf;
   int fbcsr, fbopt;
   struct ipfb_box box;
   register picsize, cwrite, twrite, ipfb;
   int error;
   extern errno;
   char *devl;
   int i, irr, icc;

   picsize = c * r * (twobytes + 1);
   if ((buf = (char *) calloc(picsize, sizeof(char))) == 0)
   {
      fprintf(stderr, "rbuffer:  core allocation failed\n");
      exit(1);
   }
   if ((ipfb = open(device, O_RDONLY)) < 0)
   {
      fprintf(stderr, "rbuffer: Couldn't open %s\n", device);
      exit(1);
   }
   if ((error = ioctl(ipfb, IPFB_SFBUNIT, &fbnum)) == -1)
   {
      perror("wrong frame buffer number");
      close(ipfb);
      exit(1);
   }
   if (high || twobytes)
   {
      ioctl(ipfb, IPFB_GETOPT, &fbopt);
      if (twobytes)
        fbopt |= IPFB_DATA16;
      if (high)
        fbopt |= IPFB_DATAHI;
      if ((error = ioctl(ipfb, IPFB_SETOPT, &fbopt)) == -1)
      {
        perror("no HIGH on frame buffer");
        close(ipfb);
        exit(1);
      }
   }
   box.ipfb_pan = ic;
   box.ipfb_scroll = ir;
   box.ipfb_xlen = c;
   box.ipfb_ylen = r;
   ioctl(ipfb, IPFB_SIOBOX, &box);

   read(ipfb, buf, picsize);
   close(ipfb);

   write_header(&hd);
   fwrite(buf,picsize,1,stdout);

}
