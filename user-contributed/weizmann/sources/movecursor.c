
/*     Movecursor is the routine that handles the cursor movements.        *
 * The movements are handled by reading 5x5 square, from where the cursor   *
 * is to be set, and writing the cursor on the same spot.                  *
 * The following movements begin with clearing the cursor from the screen.  */

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

mov_cursor (curs_shape, undercurs)
char   *curs_shape, /* points to array with the values of the cursor pixels */
       *undercurs
  /* points to array that holds the pixels that are under the cursor */;
{
    extern struct tool *tool;
    extern struct inputevent    event;
    extern struct ipfb_box  movbox;
    extern int  firstime,
                ipfb;
    int     x,
            y,
            moverr;

    if (firstime) {

/** The first appearance of the cursor on the current screen **/

       firstime = FALSE;
       x = event.ie_locx;
       y = event.ie_locy;
       movbox.ipfb_pan = x - 2;
       movbox.ipfb_scroll = y - 2;

/** loc_x and loc_y refer to the center of the 5x5 cursor **/

       ioctl (ipfb, IPFB_SIOBOX, &movbox);
       lseek (ipfb, 0L, 0); /* To make sure read & write refer
                               to the same spot */
       read (ipfb, undercurs, CURSOR_SIZE);
       lseek (ipfb, 0L, 0);
       write (ipfb, curs_shape, CURSOR_SIZE);
    }
    while ((moverr = input_readevent (tool -> tl_windowfd, &event)) != -1
     && (event.ie_code == LOC_MOVE || event.ie_code == LOC_MOVEWHILEBUTDOWN)) {

/** The next appearances of the cursor on the current screen **/

       ioctl (ipfb, IPFB_SIOBOX, &movbox);
       lseek (ipfb, 0L, 0);
       write (ipfb, undercurs, CURSOR_SIZE);
       x = event.ie_locx;
       y = event.ie_locy;
       movbox.ipfb_pan = x - 2;
       movbox.ipfb_scroll = y - 2;
       ioctl (ipfb, IPFB_SIOBOX, &movbox);
       lseek (ipfb, 0L, 0);
       read (ipfb, undercurs, CURSOR_SIZE);
       lseek (ipfb, 0L, 0);
       write (ipfb, curs_shape, CURSOR_SIZE);
    }
    if (moverr == -1) {
       fprintf (stderr, "bad input event to movecursor");
       exit (1);
    }
}
