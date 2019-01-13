/*
 * Copyright (c) 1987 Leah Mory
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All
 * the software has been tested extensively and every effort has been
 * made to insure its reliability.
 */

/*
 * controlfb.c - display a specific frame buffer as it is, or clear
 * it beforehand.
 *
 * Usage: controlfb [-g greylevel] [-p pan] [-s scroll] [-z] [-d dev]
 * [-b fbnum] [-h]
 *
 * Defaults:  device=/dev/ipfb0a, not zoomed, pan=0,scroll=0,
 * fbnum=MASTER_FB (=0), low byte (no h option), no change (no g
 * option)
 *
 * Load:       cc -O -I/horef/image/sys -o controlfb controlfb.c -lhipl
 *
 * controlfb displays on the monitor the contents of the frame buffer
 * fbnum. If the -g option is used, the frame buffer is cleared to
 * the value greylevel beforehand, and then displayed. pan and scroll
 * are only for the display : the contents of the frame buffer stays
 * intact. z sets the zoom. h is relevant only if fbnum is a 16-bits
 * buffer. The monitor will display the low byte, unless h is on. In
 * this case the high byte will be displayed.
 *
 * Leah Mory , April 1987.
 */

#include <stdio.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sundev/ipfbreg.h>
#include <hipl_format.h>

#define FBCSR  IPFB_CSRLI | IPFB_CSRLSC | IPFB_CSRLHR

char *device = "/dev/ipfb0a";  /* ipfb0a is an alu configuration.  */

main(argc, argv)
   int argc;
   char **argv;
{
   int error;
   extern errno;
   extern int optind;
   extern char *optarg;
   int mask = 0xff00, fbcsr, fbopt, op, ipfb;
   int zoom = 0;
   int pan = 0;
   int scroll = 0;
   int clear = 0, greylevel;
   int fbnum = MASTER_FB;
   int high = 0;

   Progname = strsave(*argv);
   while ((op = getopt(argc, argv, "g:p:s:zd:b:h")) != EOF)
      switch (op)
      {
      case 'g':
        clear = 1;
        greylevel = atoi(optarg);
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
      default:
        fprintf(stderr,
                "Usage:\ncontrolfb [-g greylevel] [-p pan] [-s scroll] [-z]\
 [-d device] [-b fbnum ] [-h]\n");
        exit(2);
        break;
      }
   if ((ipfb = open(device, O_WRONLY)) < 0)
   {
      fprintf(stderr, "Couldn't open %s\n", device);
      exit(1);
   }
   if ((error = ioctl(ipfb, IPFB_SFBUNIT, &fbnum)) == -1)
   {
      perror("wrong frame buffer number");
      close(ipfb);
      exit(1);
   }
   if (high)
   {
      ioctl(ipfb, IPFB_GETOPT, &fbopt);
      fbopt |= IPFB_DATAHI;
      if ((error = ioctl(ipfb, IPFB_SETOPT, &fbopt)) == -1)
      {
        perror("no HIGH on frame buffer");
        close(ipfb);
        exit(1);
      }
      greylevel <<= 8;
      mask = 0xff;     /* do not clear low */
   }
   ioctl(ipfb, IPFB_SPAN, &pan);
   ioctl(ipfb, IPFB_SSCROLL, &scroll);
   if (zoom)
   {
      ioctl(ipfb, IPFB_GFBCSR, &fbcsr);        /* get the fb csr */
      fbcsr |= FBCSR;
      ioctl(ipfb, IPFB_SFBCSR, &fbcsr);        /* set the fb csr to zoom */
   }
   if (clear)
   {
      ioctl(ipfb, IPFB_SMASK, &mask);
      ioctl(ipfb, IPFB_SDATA, &greylevel);
      ioctl(ipfb, IPFB_GFBCSR, &fbcsr);
      fbcsr = (fbcsr & ~IPFB_CSRHCS) | IPFB_CSRHCCLR;  /* clear */
      ioctl(ipfb, IPFB_SFBCSR, &fbcsr);
   }
   close(ipfb);
}
