/*
NAME
     histotool - display a histogram file as a  bar  graph  in  a
     suntools window

SYNOPSIS
     histotool [subwindow_fd] < infile

LOAD:
  cc -o histotool histotool.c -lsuntool -lsunwindow -lpixrect

DESCRIPTION
     histotool converts a histogram  file  infile  (created  with
     histo  )  to a displayable format, and displays it on a sun-
     tools window. It must be invoked from within suntools.   The
     display  window  must be big enough - 300x300, to accomodate
     the graph. If histotool is activated from  a  program  which
     has  a  subwindow  to display the histogram, like imagetool,
     the calling program gives that subwindow_fd as  an  argument
     to histotool. Otherwise, histotool opens an output subwindow
     on it's own.

AUTHOR
     Gal Hasson - Oct. 1987
     Modified: Leah Mory - Nov. 1987

SEE ALSO:
     histo(1HIPL), disphist(1HIPL), imagetool(1HIPL)
*/

#include <hipl_format.h>
#include <stdio.h>
#include <sunwindow/window_hs.h>
#include <suntool/tool_hs.h>
#include <signal.h>
#include <pixrect/pixrect_hs.h>
#include <suntool/menu.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sundev/ipfbreg.h>
#define WINDOW_XLEN 300
#define WINDOW_YLEN 300
#define OPEN 3
#define GRAYLEVELS  256
#define INPUTLEN    (sizeof(int)*(GRAYLEVELS+2))


main (argc, argv)
int     argc;
char   *argv[];
{
    char    cbuf[INPUTLEN];
    int    *buf,
           swfd,
            i,
            maxcount,
           originx,
           originy,
           highy,
           lowx,
           flag,
           tread,
           cread;
    struct tool *tool;
    struct toolsw  *tlsw;
    struct pixwin  *pixptr;
    static struct rect rect = {
       5, 5, WINDOW_XLEN, WINDOW_YLEN
    };

       struct header hd;

       Progname = strsave(*argv);
       read_header(&hd);
       if (hd.pixel_format != PFHIST){
               fprintf(stderr,"image must be in byte histogram format\n");
               exit (1);
       }
 /* It is written like this, because the input may come from a socket, which
    has a limited buffering
*/
    buf =(int *)cbuf;
    if (fread(buf,INPUTLEN,1,stdin) != 1) {
       fprintf(stderr,"histotool: could not get enough input\n");
       exit (1);
    }
    flag = 0;
    originx = 10;
    highy = 10;

       /* The histogram is 10 pixels off the window's upper left corner */

    lowx = originx + GRAYLEVELS;
    originy = highy + GRAYLEVELS;
    if (argc == 2){ /* windowfd was given */
       swfd = atoi (argv[1]);
    }
    else if (argc == 1){ /* windowfd was not given */
       tool = tool_create ("histotool_tool", OPEN, &rect, NULL);
        tlsw = tool_createsubwindow (tool, "histotool_sw",
              WINDOW_XLEN - 2 * tool_borderwidth (tool),
               WINDOW_YLEN - tool_borderwidth (tool) -
                                    tool_stripeheight(tool));
       tool_install (tool);
        tool_display (tool);
       swfd = tlsw->ts_windowfd;
       flag = 1;
    }
    else{
       fprintf(stderr,"Usage: histotool [fd]");
       exit (1);
    }
       pixptr = pw_open (swfd);
       if (flag == 1) /* A special window was opened, clear it */
          pw_writebackground (pixptr, 0, 0, tlsw -> ts_width,
           tlsw -> ts_height, PIX_CLR);

               /* Draw axis for the histogram */

       pw_vector (pixptr,originx,originy,originx,highy, 0x1E, 1);
       pw_vector (pixptr,originx,originy,lowx,originy, 0x1E, 1);
       maxcount = 0;
       for (i = 1; i < (GRAYLEVELS+1); i++)
           if (buf[i] > maxcount)
               maxcount = buf[i];

               /* Draw the histogram */

       if (maxcount > 0)
           for (i = 2; i < (GRAYLEVELS+2); i++) {
               pw_vector (pixptr, (i+originx-1),originy, (i+originx-1),highy,
                           PIX_CLR, 1);
               pw_vector (pixptr, (i+originx-1),originy, (i+originx-1),
                    (int) (originy - buf[i-1] * GRAYLEVELS / maxcount), 0x1E, 1);
           }
       pw_close (pixptr);
       if (flag == 1){
           printf("hit <^C> for termination\n");
           pause();

/*
           getchar(); /* Wait before terminating */
/*
           tool_destroy(tool);
*/
       }
}


