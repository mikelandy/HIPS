/*
NAME
     imagetool - interactive tool for image processing, using the
     mouse and the IP-512


SYNOPSIS
     imagetool [device_number]

      device_number  is  either  0  for  /dev/ipfb0c  or  1   for
     /dev/ipfb1c.

     Load:
     cc -O movecursor.o cre_sub.o imtoolproc.o -o imagetool imagetool.c
         -lsuntool -lsunwindow -lpixrect


DESCRIPTION
     imagetool is an interactive tool for  image  processing.  It
     puts  a  cursor that follows the mouse on the image monitor,
     and helps the user define rectangular part  of  that  image,
     and  process  it.   imagetool  can be invoked only from sun-
     tools.  It opens a window of 512x512 pixels,  at  the  upper
     left  corner  of the SUN console. The location of the cursor
     within this window, is translated to the cursor location  on
     the  monitor.   At  the  moment,  this  window  can  not  be
     recovered after being hidden by other windows, and  can  not
     be  moved.  Therefore, it is advised to move the window from
     which you want to call imagetool, e.g. the  shell-tool  win-
     dow,  to the right half of the console.  imagetool processes
     images by using the filters in /horef/image/bin.   Thus  the
     image  file  it  uses  must  be in the HIPL format. It opens
     parallel processes that use  the  IP-512  device  simultane-
     ously.  That  is  why  it  opens /dev/ipfb in the share mode
     (/dev/ipfb0c or /dev/ipfb1c).


HOW TO USE IMAGETOOL:
     imagetool uses suntools, therefore  it  must  be  used  from
     within  suntools.  When the imagetool window appears, enter-
     ing it with the mouse cursor will cause  another  cursor  to
     appear  on  the  image monitor. Generally, while inside this
     window, clicking the RIGHT mouse  button  will  display  the
     relevant  menu. Also choosing a menu entry is done by click-
     ing the RIGHT mouse button. If the menu disappears after the
     first  click,  hold  the  RIGHT  mouse button depressed, and
     release it when the wanted menu entry is  black. Some opera-
     tions require input from the shell-tool window. When this is
     the case, a message will appear in  the  shell-tool  window.
     One  must  move the cursor to that window in order to submit
     this input, and move it back to the imagetool window when no
     more  input  is  expected. It is recommwended to look at the
     shell-tool window for other messages  too.  Some  operations
     take longer than others. The user can know that imagetool is
     ready again if the the cursor in the image monitor  reflects
     the  movement of the mouse.  The functions VALUE, HISTOGRAM,
     ZOOM and SAVE can operate  on  a  rectangular  part  of  the
     picture.  After  choosing one of them, the last defined rec-
     tangle is displayed on the screen (unless it  is  the  whole
     screen), and the cursor looks like an upper-left corner. The
     first click of MIDDLE mouse button defines  the  upper  left
     corner  of  the  new  rectangle.  As  a  result,  the cursor
     changes, and looks like a lower right corner.The next  click
     of the MIDDLE mouse button defines the lower right corner of
     the  rectangle.  Following MIDDLE mouse button  clicks  will
     change the lower right corner.  Clicks at illegal locations,
     i.e. locations which are higher and/or more to the left than
     the  upper  left  corner  already  chosen, are ignored.  The
     smallest rectangle allowed is of 3x3 pixels. The borders  of
     any  defined  rectangle  must  be  at least 2 pixels off the
     borders of the picture.  If  the  user  wishes  to  use  the
     current  default rectangle, he can skip the above process by
     clicking the RIGHT mouse  button  immediately.  The  default
     rectangle  is  the  last  defined one, or the full screen if
     none was defined yet. After a rectangle is defined, clicking
     the RIGHT mouse button will display a secondary menu- region
     :

          rectangle - execute command on the last defined rectan-
          gle.

          full screen - execute command on the whole screen  (the
          rectangle is not changed).

          redefine - clear screen, and allow a  definition  of  a
          new rectangle.

          exit - do not execute. Go back to imagetool window  and
          click again to get the imagetool menu.

     IMAGETOOL Menu entries:
     VALUE prints in the shell-tool window a table of the coordi-
     nates  and  values of the pixels of the rectangle, and gives
     an option to change the value of any pixel  on  the  screen.
     The  maximum  size of the table is 10x10.  If the dimensions
     of the rectangle are bigger then 10x10, the pixels that  are
     displayed  are  taken  from it's upper left corner. Choosing
     the rectangle is explaind above.

     HISTOGRAM makes a histogram of the pixels of the  rectangle.
     The histogram is displayed in the imagetool window. Choosing
     the rectangle is explaind above

     ZOOM enlarges the chosen rectangle, and  places  it  at  the
     center  of  the  screen.  After  choosing  the  rectangle as
     explaind above, a  menu  of  factors:  2,  4,  8  or  other,
     appears.  If the user chooses "other", he will have to input
     a factor of his own in the  shell-tool  window,  not  before
     moving  the  mouse  cursor  to that window.  If the enlarged
     picture is bigger then the screen, the center section of  it
     is  displayed.  In these cases the process takes longer, and
     sometimes problems of memory allocation occur. If  so,  mas-
     sages are given.

     PHOTO is for taking a picture and saving it in  a  file.  At
     the same time, a histogram of the picture currently seen, is
     displayed in the imagetool window repeatedly.  This takes  a
     lot  of  memory  which sometimes cannot be allocated; if so,
     only the histograms are effected and  a  massage  is  given.
     PHOTO  also  makes  the  current  rectangle  to be the whole
     screen.

     REFRESH refreshes the screen, or a  part  of  it,  from  the
     internal  buffer  or from an image file. The internal buffer
     holds either the last rectangle  of  the  picture  that  was
     SAVEd  in  it  before, or the picture that was on the screen
     when imagetool was invoked.  After choosing  REFRESH,  click
     the  MIDDLE  mouse  button  to designate the location on the
     screen where you want the origin of  the  picture  to  be  .
     Following  MIDDLE  mouse  button  clicks will define another
     location.  You can skip this process if you want the default
     location.The default location is (0,0) for REFRESHing from a
     user's file, or the  place  from  which  it  was  SAVEd  for
     REFRESHing  from the internal buffer. Following click on the
     RIGHT mouse button will display a secondary menu- desmenu:

          default org - execute with default location.

          defined org - execute with the newly defined  location.
          This option cannot be chosen if no definition was done.

          exit - do not execute. Go back to imagetool window.


     Another secondary menu- source  will  be  displayed  once  a
     location is chosen, providing:

          buffer - refresh from internal buffer.

          image/pictures - refresh  from  pictures  directory.  A
          list     of     the    files    in    this    directory
          /horef/image/pictures will be printed. Enter  the  file
          name only.

          other - refresh from any image file. Enter full name of
          the file.

     SAVE saves a rectangle in the internal buffer or in a  file.
     Choosing  the  rectangle  is explaind above. Following that,
     another secondary menu - target will appear:

          buffer - save in the internal buffer.

          image/pictures -
           save in  /horef/image/pictures  directory.  Enter  the
          file name only.

          other - save in  another  directory.  Enter  full  file
          name, including directory.


     CLEAR WIN clears the histogram from the imagetool window.

     SHELL executes one shell command  given  in  the  shell-tool
     window, and returns control to imagetool.

     HELP prints in the shell-tool window some  useful  operating
     instructions for imagetool.

     QUIT clears the screen from the cursor, closes the open win-
     dows and device, and terminates imagetool.


DEFAULTS:
     device_number = 0


AUTHOR
     Gal Hasson - Oct. 1987
     Modified: Leah Mory - Nov. 1987


SEE ALSO:
     rframe(1HIPL),        wframe(1HIPL),         rbuffer(1HIPL),
     enlarge(1HIPL), histotool(1HIPL)

*/

#include <stdio.h>
#include <sunwindow/window_hs.h>
#include <suntool/tool_hs.h>
#include <signal.h>
#include <pixrect/pixrect_hs.h>
#include <suntool/menu.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sundev/ipfbreg.h>
#include "imagetool.h"

struct tool *tool;
struct toolsw  *tlsw;
struct pixwin  *pixwinptr;
struct inputevent   event;
struct rect rect = {
    5, 5, IPFB_SIZEX, IPFB_SIZEY
};
/* rect determines the size and position of the window on the console */

int     ipfb,
        firstime;
char    sqr_cursor[] = {
    0, 0, 0, 0, 0, 0, 150, 150, 150, 0, 0, 150, 0, 150, 0, 0, 150, 150,
    150, 0, 0, 0, 0, 0, 0
}      ,
/* sqr_cursor[] containes the values of the pixels of the square shaped
 * cursor. It containes 0 & 150 so it can be seen on any background.
*/

        line[IPFB_SIZEX],
       cursorsave[CURSOR_SIZE],
       *freshbuf,
       *device = "/dev/ipfb0c";
struct menuitem exitmenuitems[] = {
    {
       MENU_IMAGESTRING, "rectangle", EXECUTE
    },
    {
       MENU_IMAGESTRING, "full screen", FULL
    },
    {
       MENU_IMAGESTRING, "redefine", CLEAR
    },
    {
       MENU_IMAGESTRING, "exit", EXIT
    },
};
struct menu exitmenu_body =
{
    MENU_IMAGESTRING, "region", EXITMENU_SIZE, exitmenuitems, NULL, NULL
};
struct menu *exitmenu = &exitmenu_body;
struct menuitem *mnptr;
struct subregion    subregion = {
    FALSE, TRUE, 0, 0, IPFB_SIZEX - 1, IPFB_SIZEY - 1
};
/* subregion is initialized to the full screen, (0,0) and (511,511) */

struct ipfb_box movbox = {
    0, 0, CURSOR_XLEN, CURSOR_YLEN
}              ,
                freshbox = {
    0, 0, IPFB_SIZEX, IPFB_SIZEY
};
/* freshbox is initialized to the full screen */

struct desplace desplace = {
    0, 0
};

main (argc, argv)
int     argc;
char   *argv[];
{
    struct inputmask    inputmask;
    static struct menuitem  procmenu_items[] = {
       {
           MENU_IMAGESTRING, "value", VALUE
       },
       {
           MENU_IMAGESTRING, "histogram", HISTOGRAM
       },
       {
           MENU_IMAGESTRING, "zoom", ZOOM
       },
       {
           MENU_IMAGESTRING, "photo", PHOTO
       },
       {
           MENU_IMAGESTRING, "refresh", REFRESH
       },
       {
           MENU_IMAGESTRING, "save", SAVE
       },
       {
           MENU_IMAGESTRING, "clear win", CLEAR
       },
       {
           MENU_IMAGESTRING, "shell", KEY_BOARD
       },
       {
           MENU_IMAGESTRING, "help", HELP
       },
       {
           MENU_IMAGESTRING, "quit", QUIT
       },
    };
    static struct menu  procmenu_body =
    {
       MENU_IMAGESTRING, "imagetool", PROCMENU_SIZE, procmenu_items, NULL, NULL
    };
    struct menu *procmenu = &procmenu_body;
    int     i,
            err,
            designee,
           devop;

    Progname = strsave(*argv);
    firstime = TRUE;

/* line is the array from which the borders of the rectangle are taken, when
 * it's drawn on the screen. line is set to 150,150,0,0,150,150,...
 */

    for (i = 0; i < IPFB_SIZEX; i += 4) {
       line[i] = 150;
       line[i + 1] = 150;
    }
    tool = tool_create ("imagetool", OPEN, &rect, NULL);

/* Create the toolsubwindow to fit exectly inside the borders of tool  */

    tlsw = tool_createsubwindow (tool, "image_sw",
           IPFB_SIZEX - 2 * tool_borderwidth (tool),
           IPFB_SIZEY - tool_borderwidth (tool) - tool_stripeheight (tool));

/* Set the inputmask to accept mouse movements with or without depressed
 * buttons.
*/

    win_getinputmask (tool -> tl_windowfd, &inputmask, designee);
    win_setinputcodebit (&inputmask, LOC_MOVE);
    win_setinputcodebit (&inputmask, LOC_MOVEWHILEBUTDOWN);
    win_setinputmask (tool -> tl_windowfd, &inputmask, NULL, designee);
    tool_install (tool);
    tool_display (tool);
    pixwinptr = pw_open (tlsw -> ts_windowfd);
    pw_writebackground (
              pixwinptr, 0, 0, tlsw -> ts_width, tlsw -> ts_height, PIX_CLR);
    pw_close (pixwinptr);
    if (argc == 2)
       if ((devop = atoi (argv[1])) == 1)
           device = "/dev/ipfb1c";
       else if (devop != 0){
           fprintf(stderr,"Usage imagetool [0] [1]\n");
           exit(2);
       }
    if ((ipfb = open (device, 2)) < 0) {
       fprintf (stderr, "cannot open %s", device);
       exit (1);
    }
/* Allocate memory for the internal buffer,and save the current screen in it */

    freshbuf = (char *) alloca (IPFB_SIZEX * IPFB_SIZEY);
    ioctl (ipfb, IPFB_SIOBOX, &freshbox);
    lseek (ipfb, 0L, 0);
    read (ipfb, freshbuf, IPFB_SIZEX * IPFB_SIZEY);
    while ((err = input_readevent (tool -> tl_windowfd, &event)) != -1) {
       if (event.ie_code == LOC_MOVE || event.ie_code == LOC_MOVEWHILEBUTDOWN)
           mov_cursor (sqr_cursor, cursorsave);
       else
           if (event.ie_code == MS_RIGHT)
               if ((mnptr = menu_display (&procmenu, &event,
                     tlsw -> ts_windowfd)) != NULL)
                   switch (mnptr -> mi_data) {
                       case VALUE:
                           valueproc ();
                           break;
                       case HISTOGRAM:
                           histoproc ();
                           break;
                       case ZOOM:
                           zoomproc ();
                           break;
                       case REFRESH:
                           rfreshproc ();
                           break;
                       case SAVE:
                           saveproc ();
                           break;
                       case CLEAR:
                           pixwinptr = pw_open (tlsw -> ts_windowfd);
                           pw_writebackground (pixwinptr, 0, 0,
                               tlsw -> ts_width, tlsw -> ts_height, PIX_CLR);
                           pw_close (pixwinptr);
                           break;
                       case PHOTO:
                           photoproc ();
                           break;
                       case KEY_BOARD:
                           keyproc ();
                           break;
                       case HELP:
                           helpproc ();
                           break;
                       case QUIT:
                           ioctl (ipfb, IPFB_SIOBOX, &movbox);
                           lseek (ipfb, 0L, 0);
                           write (ipfb, cursorsave, CURSOR_SIZE);
                           close (ipfb);
                           tool_destroy (tool);
                           exit (0);
                           break;
                   }
    }                          /* while */
    if (err == -1) {
       fprintf (stderr, "bad input");
       exit (1);
    }
}                              /* main */
