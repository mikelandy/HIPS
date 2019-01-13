/*
 * Copyright (c) 1982 Michael Landy, Yoav Cohen, and George Sperling
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All
 * the software has been tested extensively and every effort has been
 * made to insure its reliability.
 */

/*
 * wframe.c - write a frame on the IP-512
 *
 * Usage: wframe [-y initialrow] [-x initialcol] [-C] [-d device] [-z]
 * [-p pan] [-s scroll] [-b fbnum] [-h] [-t] [-m mask] < frame
 *
 * Defaults: C is off and origin is [0,0]; if C is on, picture is
 * centerred. device=/dev/ipfb0a, not zoomed, pan=0,scroll=0,
 * fbnum=MASTER_FB (=0), low byte (no h option), 8 bits (no t
 * option), mask=0
 *
 * Load:       cc -O -I/horef/image/sys -o wframe wframe.c -lhipl
 *
 * wframe displays on the monitor a digital picture, input from the
 * standard input file, and stores it in the frame buffer fbnum. The
 * dimensions of the picture are known from it's header.
 * (initialrow, initialcol) specifies the frame buffer position of
 * the picture. If the picture does not fit the buffer due to a
 * non-zero origin , there will be wraparound. If the picture does
 * not fit the buffer due to it's dimensions, it will be trimmed. If
 * C is off, the upper left corner will be displayed. If C is on, the
 * central rectangle will be displayed. If the picture is smaller
 * than the buffer, then if C is off, it will be written into the
 * upper left corner of the buffer. If C is on, it will be centerred.
 * pan and scroll are only for the display. z, which sets the zoom,
 * holds only if the picture is not more than 256 x 256 pixels. h and
 * t are relevant only if fbnum is a 16-bits buffer. If t is on, the
 * input file must be made of 16-bits pixels. The monitor will
 * display the low byte, unless h is on. In this case the high byte
 * will be displayed. Only the bits that their correspondent bits in
 * mask are 0 will be modified by the input file. The others are
 * protected. Thus, if wframe uses a 16-bits picture and
 * mask==0xff00, only the low byte buffer will be changed, while the
 * high will remain untouched.
 *
 * Michael Landy/Yoav Cohen - 2/4/82 Modified by M. Trachtman - 20/11/85
 * Modified by L. Mory 24/11/85; Aug,1986, 15/09/86 ;
 */

#include <stdio.h>
#include <sys/file.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <hipl_format.h>
#include <sundev/ipfbreg.h>
#define FBCSR  IPFB_CSRLI | IPFB_CSRLSC | IPFB_CSRLHR

char *device = "/dev/ipfb0a";  /* ipfb0a is an alu configuration.  */

main(argc, argv)
   int argc;
   char **argv;

{
   int getopt();
   extern char *optarg;
   extern int optind;

   int r, newr, c, newc, y, ir, x, ic, opt;
   struct header hd;
   int zoom = 0;
   int center = 0;
   int pan = 0;
   int scroll = 0;
   int fbnum = MASTER_FB;
   int high = 0;
   int twobytes = 0;
   int mask = 0x0000;
   char cmdptr[100];
   int status;
   int ac;
   char params[100];
   char **av;

   Progname = strsave(*argv);
   alarm(60);
   read_header(&hd);
   alarm(0);
   ir = 0;
   y = 0;
   ic = 0;
   x = 0;
   r = hd.rows;
   c = hd.cols;
   newr = r <= IPFB_SIZEY ? r : IPFB_SIZEY;
   newc = c <= IPFB_SIZEX ? c : IPFB_SIZEX;

   alarm(60);
   while ((opt = getopt(argc, argv, "Cx:y:d:p:s:b:zhtm:")) != EOF)
      switch (opt)
      {
      case 'C':
        center++;
        break;
      case 'x':
        ic = atoi(optarg);
        x++;
        break;
      case 'y':
        ir = atoi(optarg);
        y++;
        break;
      case 'p':
        pan = atoi(optarg);
        break;
      case 's':
        scroll = atoi(optarg);
        break;
      case 'z':
        zoom++;
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
      case 'm':
        sscanf(optarg, "%x", &mask);
        break;
      case '?':
      default:
        fprintf(stderr,
        "Usage: wframe -C -x initialcol -y initialrow -z -d device -p \
pan -s scroll -b fbnum -h -t -m mask\n");
        exit(2);
      }
   alarm(0);
   if ((twobytes == 0) && (hd.pixel_format != PFBYTE))
   {
      fprintf(stderr, "wframe: frame must be in byte format.\n");
      exit(1);
   }
   if ((twobytes == 1) && (hd.pixel_format != PFSHORT))
   {
      fprintf(stderr, "wframe: frame must be in short format.\n");
      exit(1);
   }
   if (center)
   {
      if (!y)
        ir = (512 - newr) / 2;
      if (!x)
        ic = (512 - newc) / 2;
   }
   if ((newr != r) || (newc != c))
   {
      ac = argc;
      av = argv;
      while (ac--)
      {
        strcat(params, *(av++));
        strcat(params, " ");
      }
      strcat(params, "\0");

      if (center)
        sprintf(cmdptr, "extract %d %d | wframe %s", newr, newc, params);
      else
        sprintf(cmdptr, "extract %d %d 0 0 | wframe %s", newr, newc, params);
      lseek(0, 0, 0);
      if (fork() == 0)
      {
        execl("/bin/sh", "sh", "-c", cmdptr, 0);
        fprintf(stderr, "wframe: failed while trying to extract\n");
        exit(1);
      }
      wait(&status);
   } else
      fprintf(stderr, "wframe: r= %d,c=%d,ir=%d,ic=%d,fbnum=%x\n", newr, newc,
             ir, ic, fbnum);
   wframe(newr, newc, ir, ic, zoom, pan, scroll, fbnum, high, twobytes, mask);
   return (0);
}

wframe(r, c, ir, ic, zoom, pan, scroll, fbnum, high, twobytes, mask)
   int r, c, ir, ic, zoom, pan, scroll, fbnum, high, twobytes, mask;
{
   int i;
   char *buf;
   int fbcsr, fbopt, apcsr;
   struct ipfb_box box;
   int error;
   extern errno;
   register picsize, cread, tread, ipfb;

   picsize = c * r * (twobytes + 1);
   if ((buf = (char *) calloc(picsize, sizeof(char))) == 0)
   {
      fprintf(stderr, "wframe: frame core allocation failed\n");
      exit(1);
   }
   if ((ipfb = open(device, O_WRONLY)) < 0)
   {
      fprintf(stderr, "wframe: Couldn't open %s\n", device);
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
   ioctl(ipfb, IPFB_SPAN, &pan);
   ioctl(ipfb, IPFB_SSCROLL, &scroll);
   ioctl(ipfb, IPFB_SMASK, &mask);

   if ((zoom) && (r <= 256) && (c <= 256))
   {
      ioctl(ipfb, IPFB_GFBCSR, &fbcsr);        /* get the fb csr */
      fbcsr |= FBCSR;
      ioctl(ipfb, IPFB_SFBCSR, &fbcsr);        /* set the fb csr to zoom */
   }
   /*
    * it is written like this, because the input may come from a
    * socket, which has limited buffering
    */

   tread = 0;
   alarm(60);
   fread(buf,picsize,1,stdin);
   alarm(0);
   write(ipfb, buf, tread);
   lseek(ipfb, 0, 0);  /* kludge . to reposition device at 0,0 */
   close(ipfb);
}
