#include <stdio.h>
#include <sunwindow/window_hs.h>
#include <suntool/tool_hs.h>
#include <pixrect/pixrect_hs.h>
#include <signal.h>
#include <suntool/menu.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sundev/ipfbreg.h>
#include "imagetool.h"

char    hlsave[2 * IPFB_SIZEX],
        vlsave[IPFB_SIZEY],
        vrsave[IPFB_SIZEY],
        hhsave[2 * IPFB_SIZEX],
        save_hcorner[CURSOR_SIZE],
        save_lcorner[CURSOR_SIZE],
        corner,
        cursmoved;
char    glob_hcorner[] = {
    0, 0, 0, 0, 0, 0, 150, 150, 150, 150, 0, 150, 0, 0, 0, 0,
    150, 0, 0, 0, 0, 150, 0, 0, 0
}      ,
/* The values in glob_hcorner written on the screen as a 5x5 square will
 * make a two coloured shape of "north west" corner.
*/

        glob_lcorner[] = {
    0, 0, 0, 150, 0, 0, 0, 0, 150, 0, 0, 0, 0, 150, 0, 150, 150,
    150, 150, 0, 0, 0, 0, 0, 0
};
/* The values in glob_lcorner written on the screen as a 5x5 square will
 * make a two coloured shape of "south east" corner.                        */

struct ipfb_box hhbox,
                hlbox,
                vlbox,
                vrbox,
                cornerbox;
int     hicorner_x,
        hicorner_y;

create_subregion (sregion,undercursor)
/* A procedure that gives the user the possibility to define a rectangle on
 * the screen, in order to operate on it.
 * The rectangle must be at least 2 pixels off the borders of the screen.
 * For reasons of complexibility the minimum size of the rectangle is 3x3.
 * If no new rectangle is defined, the last one defined will hold.
 * A full screen rectangle is not displayed on the screen.
*/
struct subregion   *sregion;

/* A structure that holds the two opposite corners of the last defined
 * rectangle, and two flags indicating if the rectangle is on the screen,
 * and if it's dimensions are legal.
*/

char *undercursor;

/* The array that holds the pixels that are under the cursor */
{
    extern struct ipfb_box  movbox;
    extern struct tool *tool;
    extern struct toolsw   *tlsw;
    extern struct menuitem *mnptr;
    extern int  ipfb,
                firstime;
    extern struct menu *exitmenu;
    extern struct inputevent    event;
    extern char cursorsave[];
    int     i,
            suberr;
    char   *cursor,
            hcorner[CURSOR_SIZE],
            lcorner[CURSOR_SIZE],
            buff[9];
    static char curs_hcorner[] = {
       0, 0, 0, 0, 0, 0, 150, 150, 150, 150, 0, 150, 0, 0, 0, 0,
        150, 0, 0, 0, 0, 150, 0, 0, 0
    }          ,
/* Cursor pixels, shaped as a two coloured "north west" corner. */

                curs_lcorner[] = {
       0, 0, 0, 150, 0, 0, 0, 0, 150, 0, 0, 0, 0, 150, 0,
        150, 150, 150, 150, 0, 0, 0, 0, 0, 0
    };
/* Cursor pixels, shaped as a two coloured "south east" corner. */

/* Display the last defined rectangle on the screen, and change the cursor
 * shape to a "north west" corner.
*/
    draw (sregion, undercursor);
    corner = FIRST_CORNER;
    cursor = curs_hcorner;
    while ((suberr = input_readevent (tool -> tl_windowfd, &event)) != -1) {
       if (event.ie_code == LOC_MOVE || event.ie_code == LOC_MOVEWHILEBUTDOWN)
       {
           cursmoved = TRUE;
           mov_cursor (cursor, undercursor);
       }
       else
           if (event.ie_code == MS_MIDDLE) {
               switch (corner) {
                   case FIRST_CORNER:
/* The horizontal borders of the rectangle are 2 pixels wide, so the inside
 * of it must be at least 2 pixels away from the borders of the screen.
 * If the designation is not legal ignore it.
*/

                       if (event.ie_locx > 1 && event.ie_locy > 1) {
                           hicorner_x = event.ie_locx;
                           hicorner_y = event.ie_locy;
                           cornerbox.ipfb_pan = hicorner_x - 2;
                           cornerbox.ipfb_scroll = hicorner_y - 2;
                           cornerbox.ipfb_xlen = 5;
                           cornerbox.ipfb_ylen = 5;
                           ioctl (ipfb, IPFB_SIOBOX, &cornerbox);
                           lseek (ipfb, 0L, 0);
                           write (ipfb, undercursor, CURSOR_SIZE);
                           fflush (&_iob[ipfb]);
               /* Remove the last rectangle from the screen. */
                           clear (sregion, undercursor);

/* The screen is cleared from anything but the picture, in order to make
 * sure that undercursor will not hold any unwanted values
*/

                           ioctl (ipfb, IPFB_SIOBOX, &cornerbox);
                           lseek (ipfb, 0L, 0);
                           read (ipfb, undercursor, CURSOR_SIZE);
                           ioctl (ipfb, IPFB_SIOBOX, &cornerbox);
                           lseek (ipfb, 0L, 0);
                           write (ipfb, cursor, CURSOR_SIZE);
                           corner = SECOND_CORNER;
                           cursor = curs_lcorner;

               /* Leave a "corner" at the designated spot */

                           for (i = 0; i < 3; i++) {
                               buff[i] = undercursor[12 + i];
                               buff[3 + i] = undercursor[17 + i];
                               buff[6 + i] = undercursor[22 + i];
                           }
                           cornerbox.ipfb_pan = hicorner_x;
                           cornerbox.ipfb_scroll = hicorner_y;
                           cornerbox.ipfb_xlen = 3;
                           cornerbox.ipfb_ylen = 3;
                           ioctl (ipfb, IPFB_SIOBOX, &cornerbox);
                           lseek (ipfb, 0L, 0);
                           write (ipfb, buff, 9);
                           for (i = 0; i < CURSOR_SIZE; i++)
                               save_hcorner[i] = undercursor[i];
                           firstime = TRUE;
                       }
                       break;
                   case SECOND_CORNER:
/* The rectangle is limited to minimum size of 3x3, for reasons of
 * complexibility in writing and manipulating the cursor, and the drawing of
 * the rectangle.
*/

                       if (event.ie_locx - hicorner_x > 2 &&
                           event.ie_locy - hicorner_y > 2) {

/* Remove the "corner",load sregion, and draw the new defined rectangle. */

                           cornerbox.ipfb_pan = hicorner_x - 2;
                           cornerbox.ipfb_scroll = hicorner_y - 2;
                           cornerbox.ipfb_xlen = 5;
                           cornerbox.ipfb_ylen = 5;
                           ioctl (ipfb, IPFB_SIOBOX, &cornerbox);
                           lseek (ipfb, 0L, 0);
                           write (ipfb, save_hcorner, CURSOR_SIZE);
                           sregion -> dimension_ok = TRUE;
                           sregion -> origin_x = hicorner_x;
                           sregion -> origin_y = hicorner_y;
                           sregion -> lowcorner_x = event.ie_locx;
                           sregion -> lowcorner_y = event.ie_locy;
                           draw (sregion, undercursor);
                           corner = CORRECTION;
                           for (i = 0; i < CURSOR_SIZE; i++)
                               save_lcorner[i] = undercursor[i];
                           firstime = TRUE;
                       }
                       break;
                   case CORRECTION:
                       if (event.ie_locx - hicorner_x > 2 &&
                           event.ie_locy - hicorner_y > 2) {

       /* Remove the old rectangle and draw the corrected one */

                           clear (sregion, undercursor);
                           sregion -> dimension_ok = TRUE;
                           sregion -> lowcorner_x = event.ie_locx;
                           sregion -> lowcorner_y = event.ie_locy;
                           draw (sregion, undercursor);
                           for (i = 0; i < CURSOR_SIZE; i++)
                               save_lcorner[i] = undercursor[i];
                           firstime = TRUE;
                       }
                       break;
               }               /* switch (corner) */
               cursmoved = FALSE;
           }                   /* ms_middle */
           else
               if (event.ie_code == MS_RIGHT) { /* The menu button */
                   while ((mnptr =
                      menu_display (&exitmenu, &event, tlsw -> ts_windowfd))
                            == NULL);
       /* A menu entry mast be pickeded at this stage */
                   switch (mnptr -> mi_data) {
                       case EXECUTE:
               /* execute on the rectangle that was last defined */
                           clear (sregion,undercursor);
                           return (1);
                           break;
                       case FULL:
               /* Set the rectangle and execute on the whole screen */
                           clear (sregion, undercursor);
                           sregion -> dimension_ok = TRUE;
                           sregion -> origin_x = 0;
                           sregion -> origin_y = 0;
                           sregion -> lowcorner_x = IPFB_SIZEX - 1;
                           sregion -> lowcorner_y = IPFB_SIZEY - 1;
                           return (1);
                           break;
                       case EXIT:
                           clear (sregion, undercursor);
                           return (0);
                           break;
                       case CLEAR:
               /* Allow definition of a new rectangle */
                           clear (sregion, undercursor);
                           cursor = curs_hcorner;
                           corner = FIRST_CORNER;
                           break;
                   }           /* switch (mnptr) */
               }               /* if MS_RIGHT */
    }                          /* while */
    return (-1);
}                              /* create_subregion */



draw (sregion,save)
/* A procedure that displays the last defined rectangle on the screen.
 * It takes in consideration the location of the cursor, in relation to the
 * borders of the rectangle.
 * The horizontal borders are 2 pixels wide because it blinks a lot less.
*/

struct subregion   *sregion;
char   *save;
{
    extern char line[];
    char    curson;

    if (sregion -> dimension_ok && sregion -> origin_x > 1 &&
       sregion -> origin_y > 1) {
       if (curson = cursor_onreg (&movbox, sregion)) {
/* If the cursor is located where the borders of the rectangle are to be put,
 * the cursor is removed before drawing the rectangle. That is done, to make
 * sure that no "dirt" will be left on the picture when the rectangle is
 * removed.
*/

           ioctl (ipfb, IPFB_SIOBOX, &movbox);
           lseek (ipfb, 0L, 0);
           write (ipfb, save, CURSOR_SIZE);
           fflush (&_iob[ipfb]);
           firstime = TRUE;
       }
/* The horizontal lines are written in two stages, in order to match the
 * colours between them.
*/

       hhbox.ipfb_pan = sregion -> origin_x;
       hhbox.ipfb_scroll = sregion -> origin_y - 2;
       hhbox.ipfb_xlen = sregion -> lowcorner_x - sregion -> origin_x + 1;
       hhbox.ipfb_ylen = 1;
       ioctl (ipfb, IPFB_SIOBOX, &hhbox);
       lseek (ipfb, 0L, 0);
       read (ipfb, hhsave, hhbox.ipfb_xlen);
       lseek (ipfb, 0L, 0);
       write (ipfb, line, hhbox.ipfb_xlen);
       hhbox.ipfb_scroll += 1;
       ioctl (ipfb, IPFB_SIOBOX, &hhbox);
       lseek (ipfb, 0L, 0);
       read (ipfb, &hhsave[hhbox.ipfb_xlen], hhbox.ipfb_xlen);
       lseek (ipfb, 0L, 0);
       write (ipfb, line, hhbox.ipfb_xlen);

       hlbox.ipfb_pan = sregion -> origin_x;
       hlbox.ipfb_scroll = sregion -> lowcorner_y + 1;
       hlbox.ipfb_xlen = sregion -> lowcorner_x - sregion -> origin_x + 1;
       hlbox.ipfb_ylen = 1;
       ioctl (ipfb, IPFB_SIOBOX, &hlbox);
       lseek (ipfb, 0L, 0);
       read (ipfb, hlsave, hlbox.ipfb_xlen);
       lseek (ipfb, 0L, 0);
       write (ipfb, line, hlbox.ipfb_xlen);
       hlbox.ipfb_scroll += 1;
       ioctl (ipfb, IPFB_SIOBOX, &hlbox);
       lseek (ipfb, 0L, 0);
       read (ipfb, &hlsave[hlbox.ipfb_xlen], hlbox.ipfb_xlen);
       lseek (ipfb, 0L, 0);
       write (ipfb, line, hlbox.ipfb_xlen);

       vlbox.ipfb_pan = sregion -> origin_x - 1;
       vlbox.ipfb_scroll = sregion -> origin_y - 2;
       vlbox.ipfb_xlen = 1;
       vlbox.ipfb_ylen = sregion -> lowcorner_y - sregion -> origin_y + 4;
       ioctl (ipfb, IPFB_SIOBOX, &vlbox);
       lseek (ipfb, 0L, 0);
       read (ipfb, vlsave, vlbox.ipfb_ylen);
       lseek (ipfb, 0L, 0);
       write (ipfb, line, vlbox.ipfb_ylen);

       vrbox.ipfb_pan = sregion -> lowcorner_x + 1;
       vrbox.ipfb_scroll = sregion -> origin_y - 2;
       vrbox.ipfb_xlen = 1;
       vrbox.ipfb_ylen = sregion -> lowcorner_y - sregion -> origin_y + 4;
       ioctl (ipfb, IPFB_SIOBOX, &vrbox);
       lseek (ipfb, 0L, 0);
       read (ipfb, vrsave, vrbox.ipfb_ylen);
       lseek (ipfb, 0L, 0);
       write (ipfb, line, vrbox.ipfb_ylen);

       sregion -> onscreen = TRUE;
    }
}


clear (sregion,save)
/* A procedure that clears the rectangle from the screen.
 * It takes in consideration the location of the cursor, in relation to the
 * borders of the rectangle.
*/

struct subregion   *sregion;
char *save;
{
    extern char line[];
    int     i,
            curson;

    if (curson = cursor_onreg (&movbox, sregion)) {
/* If the cursor is located on the borders of the rectangle, the cursor is
 * removed before drawing the rectangle. That is done to make sure that
 * no "dirt" will be left on the picture when the cursor will be moved.
*/

       ioctl (ipfb, IPFB_SIOBOX, &movbox);
       lseek (ipfb, 0L, 0);
       write (ipfb, save, CURSOR_SIZE);
       fflush (&_iob[ipfb]);
       firstime = TRUE;
    }
    if (sregion -> onscreen) {
       hhbox.ipfb_pan = sregion -> origin_x;
       hhbox.ipfb_scroll = sregion -> origin_y - 2;
       hhbox.ipfb_xlen = sregion -> lowcorner_x - sregion -> origin_x + 1;
       hhbox.ipfb_ylen = 2;

       hlbox.ipfb_pan = sregion -> origin_x;
       hlbox.ipfb_scroll = sregion -> lowcorner_y + 1;
       hlbox.ipfb_xlen = sregion -> lowcorner_x - sregion -> origin_x + 1;
       hlbox.ipfb_ylen = 2;

       vlbox.ipfb_pan = sregion -> origin_x - 1;
       vlbox.ipfb_scroll = sregion -> origin_y - 2;
       vlbox.ipfb_xlen = 1;
       vlbox.ipfb_ylen = sregion -> lowcorner_y - sregion -> origin_y + 4;

       vrbox.ipfb_pan = sregion -> lowcorner_x + 1;
       vrbox.ipfb_scroll = sregion -> origin_y - 2;
       vrbox.ipfb_xlen = 1;
       vrbox.ipfb_ylen = sregion -> lowcorner_y - sregion -> origin_y + 4;

       ioctl (ipfb, IPFB_SIOBOX, &hhbox);
       lseek (ipfb, 0L, 0);
       write (ipfb, hhsave, 2 * hhbox.ipfb_xlen);

       ioctl (ipfb, IPFB_SIOBOX, &hlbox);
       lseek (ipfb, 0L, 0);
       write (ipfb, hlsave, 2 * hlbox.ipfb_xlen);

       ioctl (ipfb, IPFB_SIOBOX, &vlbox);
       lseek (ipfb, 0L, 0);
       write (ipfb, vlsave, vlbox.ipfb_ylen);

       ioctl (ipfb, IPFB_SIOBOX, &vrbox);
       lseek (ipfb, 0L, 0);
       write (ipfb, vrsave, vrbox.ipfb_ylen);

       if (curson) {
           ioctl (ipfb, IPFB_SIOBOX, &movbox);
           lseek (ipfb, 0L, 0);
           read (ipfb, save, CURSOR_SIZE);
       }

       sregion -> onscreen = FALSE;
    }
    else
       if (corner == SECOND_CORNER) {
           if (!cursmoved) /* The cursor is on the "north west corner. */
               for (i = 0; i < CURSOR_SIZE; i++)
                   save[i] = save_hcorner[i];
           else {
/* The cursor must be cleared from the screen before the "corner".  *
 * Otherwise "dirt" might be left on the screen.                   */

               ioctl (ipfb, IPFB_SIOBOX, &movbox);
               lseek (ipfb, 0L, 0);
               write (ipfb, save, CURSOR_SIZE);
               fflush (&_iob[ipfb]);
               firstime = TRUE;
           }
           cornerbox.ipfb_pan = hicorner_x - 2;
           cornerbox.ipfb_scroll = hicorner_y - 2;
           cornerbox.ipfb_xlen = 5;
           cornerbox.ipfb_ylen = 5;
           ioctl (ipfb, IPFB_SIOBOX, &cornerbox);
           lseek (ipfb, 0L, 0);
           write (ipfb, save_hcorner, CURSOR_SIZE);
       }
}


designate(desplaceptr,save)
/* A procedure that gives the user the possibility to define a spot on the
 * screen, that will be the origin for displaying a picture.
 * The coordinates of the defined origin are returned in desplaceptr.
 * After the definition the procedure removes the cursor from the screen,
 * and clears the scren from any unwanted "dirt".
 * If 'defined org' is chosen although no spot was defined, the procedure
 * will terminate with an error massage.
 * When a picture is displayed, one or two lines of pixels from it's borders
 * are not seen on the screen.
 * For that reason, and other problems at the edges, the defined spot must
 * be at least 2 pixels away from the borders of the picture.
 * Illegal definitions will be ignored.
*/

struct desplace  *desplaceptr;
char *save;
{
    int     i,
            deserr,
            onscreen,
            wasdes, /* was a definition of an origin made */
            descorner_x,
            descorner_y;
    char    buff[9],
           *cursor;
    static struct menuitem  desmenuitems[] = {
       {
           MENU_IMAGESTRING, "default org", PREV
       },
       {
           MENU_IMAGESTRING, "defined org", DES
       },
       {
           MENU_IMAGESTRING, "exit", EXIT
       },
    };
    static struct menu  desmenu_body =
    {
       MENU_IMAGESTRING, "desmenu", DESMENU_SIZE, desmenuitems, NULL, NULL
    };
    static struct menu *desmenu = &desmenu_body;
    struct menuitem *mnptr;

    onscreen = FALSE;
    wasdes = FALSE;
    cursor = glob_hcorner;
    corner = FIRST_CORNER;
    while ((deserr = input_readevent (tool -> tl_windowfd, &event)) != -1) {
       if (event.ie_code == LOC_MOVE || event.ie_code == LOC_MOVEWHILEBUTDOWN)
       {
           mov_cursor (cursor, save);
       }
       else
           if (event.ie_code == MS_MIDDLE) {
               switch (corner) {
                   case FIRST_CORNER:
                       if (event.ie_locx > 1 && event.ie_locy > 1) {

                           /* Ignore illegal definitions */

                           descorner_x = event.ie_locx;
                           descorner_y = event.ie_locy;
                           corner = CORRECTION;
                           wasdes = TRUE;

                       /* leave a "corner" on the screen */

                           for (i = 0; i < 3; i++) {
                               buff[i] = save[12 + i];
                               buff[3 + i] = save[17 + i];
                               buff[6 + i] = save[22 + i];
                           }
                           cornerbox.ipfb_pan = descorner_x;
                           cornerbox.ipfb_scroll = descorner_y;
                           cornerbox.ipfb_xlen = 3;
                           cornerbox.ipfb_ylen = 3;
                           ioctl (ipfb, IPFB_SIOBOX, &cornerbox);
                           lseek (ipfb, 0L, 0);
                           write (ipfb, buff, 9);
                           for (i = 0; i < CURSOR_SIZE; i++)
                               save_hcorner[i] = save[i];
                           firstime = TRUE;
                           onscreen = TRUE;
                       }
                       break;
                   case CORRECTION:
                       if (event.ie_locx > 1 && event.ie_locy > 1) {

/* If the new definition is leagal, clear the previous defined "corner",and
 * display the new one.
*/

                           cornerbox.ipfb_pan = event.ie_locx - 2;
                           cornerbox.ipfb_scroll = event.ie_locy - 2;
                           cornerbox.ipfb_xlen = 5;
                           cornerbox.ipfb_ylen = 5;
                           ioctl (ipfb, IPFB_SIOBOX, &cornerbox);
                           lseek (ipfb, 0L, 0);
                           write (ipfb, save, CURSOR_SIZE);
                           cornerbox.ipfb_pan = descorner_x - 2;
                           cornerbox.ipfb_scroll = descorner_y - 2;
                           ioctl (ipfb, IPFB_SIOBOX, &cornerbox);
                           lseek (ipfb, 0L, 0);
                           write (ipfb, save_hcorner, CURSOR_SIZE);
                           cornerbox.ipfb_pan = event.ie_locx - 2;
                           cornerbox.ipfb_scroll = event.ie_locy - 2;
                           ioctl (ipfb, IPFB_SIOBOX, &cornerbox);
                           lseek (ipfb, 0L, 0);
                           read (ipfb, save, CURSOR_SIZE);
                           for (i = 0; i < 3; i++) {
                               buff[i] = save[12 + i];
                               buff[3 + i] = save[17 + i];
                               buff[6 + i] = save[22 + i];
                           }
                           descorner_x = event.ie_locx;
                           descorner_y = event.ie_locy;
                           cornerbox.ipfb_pan = event.ie_locx - 2;
                           cornerbox.ipfb_scroll = event.ie_locy - 2;
                           cornerbox.ipfb_xlen = 5;
                           cornerbox.ipfb_ylen = 5;
                           ioctl (ipfb, IPFB_SIOBOX, &cornerbox);
                           lseek (ipfb, 0L, 0);
                           write (ipfb, glob_hcorner, CURSOR_SIZE);
                           cornerbox.ipfb_pan = descorner_x;
                           cornerbox.ipfb_scroll = descorner_y;
                           cornerbox.ipfb_xlen = 3;
                           cornerbox.ipfb_ylen = 3;
                           ioctl (ipfb, IPFB_SIOBOX, &cornerbox);
                           lseek (ipfb, 0L, 0);
                           write (ipfb, buff, 9);
                           for (i = 0; i < CURSOR_SIZE; i++)
                               save_hcorner[i] = save[i];
                           firstime = TRUE;
                       }
                       break;
               }
           }
           else
               if (event.ie_code == MS_RIGHT) {

       /* clear the screen before exiting the procedure */

                   ioctl (ipfb, IPFB_SIOBOX, &movbox);
                   lseek (ipfb, 0L, 0);
                   write (ipfb, save, CURSOR_SIZE);
                   fflush (&_iob[ipfb]);
                   if (onscreen) {
                       cornerbox.ipfb_pan = descorner_x - 2;
                       cornerbox.ipfb_scroll = descorner_y - 2;
                       cornerbox.ipfb_xlen = 5;
                       cornerbox.ipfb_ylen = 5;
                       ioctl (ipfb, IPFB_SIOBOX, &cornerbox);
                       lseek (ipfb, 0L, 0);
                       write (ipfb, save_hcorner, CURSOR_SIZE);
                       onscreen = FALSE;
                   }
                   firstime = TRUE;
                   while ((mnptr = menu_display (&desmenu, &event,
                          tlsw -> ts_windowfd)) == NULL);
                   switch (mnptr -> mi_data) {
                       case PREV:
                           return (1);
                           break;
                       case DES:
                           if (wasdes) {
                               desplaceptr -> loc_x = descorner_x;
                               desplaceptr -> loc_y = descorner_y;
                               return (2);
                           }
                           else {
                               printf (
                     "you did not define the origin for refresh, try again\n");
                               return (0);
                           }
                           break;
                       case EXIT:
                           return (0);
                           break;
                   }
               }
    }
    return (-1);
}


cursor_onreg(mptr,sptr)
/* A procedure that checks whether the cursor is ,or is not ,on the borders
 * of the rectangle defined by *sptr. and returns the unswer.
 * Whether the rectangle is on the screen or not,does not matter.
*/

struct ipfb_box *mptr;
struct subregion *sptr;
{
    if (/* cursor is on one of the horizontal borders */
       (((mptr -> ipfb_scroll < sptr -> origin_y &&
          mptr -> ipfb_scroll >= sptr -> origin_y - 6)  ||
         (mptr -> ipfb_scroll <= sptr -> lowcorner_y + 2 &&
          mptr -> ipfb_scroll >= sptr -> lowcorner_y - 3) )  &&
        (mptr -> ipfb_pan >= sptr -> origin_x - 5 &&
         mptr -> ipfb_pan < sptr -> lowcorner_x + 1)) ||
       /* cursor is on one of the vertical borders */
       (((sptr -> origin_x - 1 >= mptr -> ipfb_pan &&
          sptr -> origin_x - 1 <= mptr -> ipfb_pan + 4) ||
         (sptr -> lowcorner_x + 1 >= mptr -> ipfb_pan &&
          sptr -> lowcorner_x + 1 <= mptr -> ipfb_pan + 4)) &&
        (mptr -> ipfb_scroll > sptr -> origin_y - 5 &&
         mptr -> ipfb_scroll <= sptr -> lowcorner_y)))
       return (TRUE);
    else
       return (FALSE);
}
