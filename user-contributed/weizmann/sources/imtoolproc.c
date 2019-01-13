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

extern int  ipfb;
extern struct subregion subregion;
extern struct ipfb_box  movbox;
extern int  firstime;
extern struct menuitem *mnptr;
extern struct inputevent    event;
extern struct toolsw   *tlsw;
extern struct ipfb_box  freshbox;
extern char *freshbuf,
            cursorsave[],
           *device;
extern struct desplace desplace;

valueproc () {
/*  A procedure that prints a table of coordinates & values of the pixels of
 *  the rectangle, and gives an option to change the value of any pixel.
 *  The max table dimensions are: MAX_TABLE_XLEN = (10), MAX_TABLE_YLEN = (10)
 *  If the rectangle is bigger, the table is truncated to fit the limits.
 *  If no rectangle is defined, valueproc will operate on the last one that
 *  was defined.
 */

    struct ipfb_box sgbox;
    char    sgsave[MAX_TABLESIZE],
            ch;
    int     ncols, /* The defined subregion columns, up to MAX_TABLE_XLEN */
            nrows, /* The defined subregion rows, up to MAX_TABLE_YLEN */
            c,
            r,
            index,
            i,
            arg[3];

    if (create_subregion (&subregion,cursorsave) == 1) {

/* Remove the cursor from the screen, to make sure that no pixels are
 * effected. The next appearance of the cursor should be treated as
 * the first appearance on the current screen.
*/

       ioctl (ipfb, IPFB_SIOBOX, &movbox);
       lseek (ipfb, 0L, 0);
       write (ipfb, cursorsave, CURSOR_SIZE);
       fflush (&_iob[ipfb]);
       firstime = TRUE;

/*  Set sgbox and read the relevant pixels into sgsave */

       sgbox.ipfb_pan = subregion.origin_x;
       sgbox.ipfb_scroll = subregion.origin_y;
       sgbox.ipfb_xlen = ncols =
         (subregion.lowcorner_x - subregion.origin_x < MAX_TABLE_XLEN) ?
          (subregion.lowcorner_x - subregion.origin_x + 1) : MAX_TABLE_XLEN;
       sgbox.ipfb_ylen = nrows =
         (subregion.lowcorner_y - subregion.origin_y < MAX_TABLE_YLEN) ?
          (subregion.lowcorner_y - subregion.origin_y + 1) : MAX_TABLE_YLEN;
       ioctl (ipfb, IPFB_SIOBOX, &sgbox);
       lseek (ipfb, 0L, 0);
       read (ipfb, sgsave, ncols * nrows);

/** Create and print the values table **/

       printf ("\n     |");
       for (c = 0; c < ncols; c++)
           printf (" %3d |", c + subregion.origin_x);
       printf ("\n     |");
       for (i = 0; i < ncols; i++)
           printf ("-----|");
       for (r = 0; r < nrows; r++) {
           printf ("\n %3d | ", r + subregion.origin_y);
           for (c = 0; c < ncols; c++) {
               index = r * ncols + c;

       /** Print the decimal value of the pixels **/

               printf ("%3d | ", (0x000000ff & (int) sgsave[index]));
           }

       }
       printf ("\n\nto change pixels values enter <row  column  value> ; \
to quit hit <CR>\n");

       /** Get the input and check for validity **/

       i = 0;
       while (TRUE) {
           while ((ch = getchar ()) == ' ');
           if (ch == '\n') {
               if (i == 0) /* <CR> was hit at the beginning of a line */
                   break;
               else
                   if (i < 3) {
                     printf ("3 integer arguments are expected, try again\n");
                       i = 0;
                   }
           }
           else   /* ch != \n */
               if (i < 3) {
                   ungetc (ch, stdin);
                   if ((scanf ("%d", &arg[i])) == 1)
                       i++;
                   else { /* scanf failed to scan an integer */
                     printf ("3 integer arguments are expected, try again\n");
                     while (getchar () != '\n');/*dispose rest of input line*/
                       i = 0;
                   }
               }
           if (i == 3) {
               if (arg[0] >= 0 && arg[0] <= IPFB_SIZEY && arg[1] >= 0 &&
                    arg[1] <= IPFB_SIZEX && arg[2] >= 0 && arg[2] <= 255) {
                   ioctl (ipfb, IPFB_SX, &arg[1]);
                   ioctl (ipfb, IPFB_SY, &arg[0]);
                   ioctl (ipfb, IPFB_SDATA, &arg[2]);
               }
               else
                   printf ("the data entered is out of range, try again\n");
               while (getchar () != '\n');
               i = 0;
           }
       }
    }
}



rfreshproc () {
/*  A procedure that refreshes the screen or a part of it, from a user's
 *  picture file, or the internal buffer.
 *  It uses the function designate to determine the place for the picture.
 *  If no origin is defined for the picture the defaults are:
 *  (0,0) for a user's picture file, and the same position on the screen,
 *  from where the buffer was saved, for the internal buffer.
*/

    int     status,
            pid,
           pid1,
           desflag,
           pan = 0,
           scroll = 0,
           mask = 0x0000;
    char    cmdptr[MAX_COMMANDLEN],
            filenamep[MAX_FILENAMELEN];
    static struct menuitem  freshmenu_items[] = {
       {
           MENU_IMAGESTRING, "buffer", BUFFER
       },
       {
           MENU_IMAGESTRING, "image/picture", IMAGE_PIC
       },
       {
           MENU_IMAGESTRING, "other", OTHER
       },
    };
    static struct menu  freshmenu_body =
    {
       MENU_IMAGESTRING, "source", FRESHMENU_SIZE, freshmenu_items, NULL, NULL
    };
    struct menu *freshmenu = &freshmenu_body;
    static struct ipfb_box desbox = {0,0,0,0};

if ((desflag = designate (&desplace,cursorsave)) != -1 && desflag != 0) {

/* designation was successful and execution command was given */

    while ((mnptr = menu_display(&freshmenu, &event, tlsw -> ts_windowfd)) ==
            NULL);
    switch (mnptr -> mi_data) {
       case IMAGE_PIC:
           printf ("/horef/image/pictures:\n");
           fflush (stdout);
           if ((pid = fork ()) == 0) {
               execl ("/bin/sh", "sh", "-c", "ls /horef/image/pictures", 0);
           }
           while (wait (&status) != pid);
           fflush (stdout);
           printf ("the picture file is /horef/image/pictures/");
           scanf ("%s", filenamep);
           if (desflag == 1)         /** use default origin (0,0) **/
               sprintf (cmdptr, "wframe -d %s < /horef/image/pictures/%s",
                        device, filenamep);
           else
               if (desflag == 2)      /** use defined origin **/
                   sprintf (cmdptr,
                         "wframe -d %s -y %d -x %d < /horef/image/pictures/%s",
                           device, desplace.loc_y, desplace.loc_x, filenamep);
           if ((pid1 = fork ()) == 0) {
               execl ("/bin/sh", "sh", "-c", cmdptr, 0);
           }
           while (wait (&status) != pid1);
           break;
       case OTHER:
           printf ("enter full picture file name\n");
           scanf ("%s", filenamep);

           if (desflag == 1)
               sprintf (cmdptr, "wframe -d %s < %s", device, filenamep);
           else
               sprintf (cmdptr, "wframe -d %s -y %d -x %d < %s",
                        device, desplace.loc_y, desplace.loc_x, filenamep);
           if ((pid1 = fork ()) == 0) {
               execl ("/bin/sh", "sh", "-c", cmdptr, 0);
           }
           while (wait (&status) != pid1);
           break;
       case BUFFER:
           if (desflag == 2) {    /** use defined origin **/
               desbox.ipfb_pan = desplace.loc_x;
               desbox.ipfb_scroll = desplace.loc_y;
               desbox.ipfb_xlen = freshbox.ipfb_xlen;
               desbox.ipfb_ylen = freshbox.ipfb_ylen;
               ioctl (ipfb, IPFB_SIOBOX, &desbox);
           }
           else
               if (desflag == 1)  /** use default origin  **/
                   ioctl (ipfb, IPFB_SIOBOX, &freshbox);

/* Before writing from the buffer,set some parameters that may have been
 * changed, if PHOTO was used.
 * (wframe sets them for itself)
*/

           ioctl (ipfb, IPFB_SPAN, &pan);
           ioctl (ipfb, IPFB_SSCROLL, &scroll);
           ioctl (ipfb, IPFB_SMASK, &mask);
           lseek (ipfb, 0L, 0);
           write (ipfb, freshbuf, freshbox.ipfb_xlen * freshbox.ipfb_ylen);
           break;
    }
}
if (desflag == -1) exit (1);
}



histoproc () {
/* A procedure that draws a histogram.
 * It calls histotool with the windowfd of the subwindow opened by imagetool,
 * so the histogram is drawn inside the window opened by imagetool.
 * If no rectangle is defined, histoproc will operate on the last one that
 * was defined.
*/

    char    cmdptr[MAX_COMMANDLEN];
    int     status,
           pid;

    if (create_subregion (&subregion,cursorsave) == 1) {
       ioctl (ipfb, IPFB_SIOBOX, &movbox);
       lseek (ipfb, 0L, 0);
       write (ipfb, cursorsave, CURSOR_SIZE);
       fflush (&_iob[ipfb]);
       if ((pid = fork ()) == 0) {
           sprintf (cmdptr,
                    "rbuffer -d %s -x %d -y %d -r %d -c %d|histo|histotool %d",
                     device, subregion.origin_x, subregion.origin_y,
                     (subregion.lowcorner_y - subregion.origin_y + 1),
                     (subregion.lowcorner_x - subregion.origin_x + 1),
                     tlsw -> ts_windowfd);
           execl ("/bin/sh", "sh", "-c", cmdptr, 0);
       }
       while (wait (&status) != pid);
       firstime = TRUE;
    }

}



zoomproc () {
/* A procedure that takes the rectangle defined on the screen, enlarges it
 * in both dimensions by the factor that was chosen, and writes it back
 * at the center of the screen.
 * If no rectangle is defined, zoomproc will operate on the last one that
 * was defined.
 * If the enlarged rectangle is bigger then the screen, the middle is taken
 * and displayed.
*/

    int     status,
            pid,
            factor;
    char    cmdptr[MAX_COMMANDLEN];
    static struct menuitem  zoomenu_items[] = {
       {
           MENU_IMAGESTRING, "2", TWO
       },
       {
           MENU_IMAGESTRING, "4", FOUR
       },
       {
           MENU_IMAGESTRING, "8", EIGHT
       },
       {
           MENU_IMAGESTRING, "other", OTHER
       },
    };
    static struct menu  zoomenu_body =
    {
       MENU_IMAGESTRING, "factor", ZOOMENU_SIZE, zoomenu_items, NULL, NULL
    };
    struct menu *zoomenu = &zoomenu_body;

    if (create_subregion (&subregion,cursorsave) == 1) {
       ioctl (ipfb, IPFB_SIOBOX, &movbox);
       lseek (ipfb, 0L, 0);
       write (ipfb, cursorsave, CURSOR_SIZE);
       fflush (&_iob[ipfb]);
       while (
       (mnptr = menu_display (&zoomenu, &event, tlsw -> ts_windowfd)) == NULL);
           switch (mnptr -> mi_data) {
               case TWO:
                   factor = 2;
                   break;
               case FOUR:
                   factor = 4;
                   break;
               case EIGHT:
                   factor = 8;
                   break;
               case OTHER:
                   printf ("enter other factor\n");
                   scanf ("%d", &factor);
                   break;
           }
           sprintf (cmdptr,
            "rbuffer -d %s -x %d -y %d -r %d -c %d|enlarge %d|wframe -d %s -C",
             device, subregion.origin_x, subregion.origin_y,
             (subregion.lowcorner_y - subregion.origin_y + 1),
             (subregion.lowcorner_x - subregion.origin_x + 1), factor, device);
           if ((pid = fork ()) == 0) {
               execl ("/bin/sh", "sh", "-c", cmdptr, 0);
       }
       while (wait (&status) != pid);
       firstime = TRUE;
    }
}


saveproc () {
/* A procedure that saves either the whole screen or a rectangle.
 * The picture can be saved as a ruster file in the pictures directory,
 * or any other directory, or as an array of values in the internal buffer.
 * If a subregion of the screen is saved in the buffer, it 's location is
 * saved as the default for refresh.
 * If no rectangle is defined, saveproc will operate on the last one that
 * was defined.
*/

       int     status,
            pid;
char    cmdptr[MAX_COMMANDLEN],
        filenamep[MAX_FILENAMELEN];
static struct menuitem  savemenu_items[] = {
    {
       MENU_IMAGESTRING, "buffer", BUFFER
    },
    {
       MENU_IMAGESTRING, "image/picture", IMAGE_PIC
    },
    {
       MENU_IMAGESTRING, "other", OTHER
    },
};
static struct menu  savemenu_body =
{
    MENU_IMAGESTRING, "target", SAVEMENU_SIZE, savemenu_items, NULL, NULL
};
struct menu *savemenu = &savemenu_body;

if (create_subregion (&subregion,cursorsave) == 1) {
    ioctl (ipfb, IPFB_SIOBOX, &movbox);
    lseek (ipfb, 0L, 0);
    write (ipfb, cursorsave, CURSOR_SIZE);
    fflush (&_iob[ipfb]);
    while ((mnptr = menu_display (&savemenu, &event, tlsw -> ts_windowfd)) ==
          NULL);
    switch (mnptr -> mi_data) {
       case IMAGE_PIC:
           printf ("the picture file is /horef/image/pictures/");
           scanf ("%s", filenamep);
           sprintf (cmdptr,
            "rbuffer -d %s -x %d -y %d -r %d -c %d > /horef/image/pictures/%s",
            device, subregion.origin_x, subregion.origin_y,
             (subregion.lowcorner_y - subregion.origin_y + 1),
             (subregion.lowcorner_x - subregion.origin_x + 1), filenamep);
           if ((pid = fork ()) == 0) {
               execl ("/bin/sh", "sh", "-c", cmdptr, 0);
           }
           while (wait (&status) != pid);
           break;
       case OTHER:
           printf ("enter full dest file name\n");
           scanf ("%s", filenamep);
           sprintf (cmdptr, "rbuffer -d %s -x %d -y %d -r %d -c %d > %s",
                    device, subregion.origin_x, subregion.origin_y,
                    (subregion.lowcorner_y - subregion.origin_y + 1),
                    (subregion.lowcorner_x - subregion.origin_x + 1),
                    filenamep);

           if ((pid = fork ()) == 0) {
               execl ("/bin/sh", "sh", "-c", cmdptr, 0);
           }
           while (wait (&status) != pid);
           break;
       case BUFFER:
/*Set freshbox for reading the relevant pixels, and as a default for Refresh*/

           freshbox.ipfb_pan = subregion.origin_x;
           freshbox.ipfb_scroll = subregion.origin_y;
           freshbox.ipfb_xlen =
             (subregion.lowcorner_x - subregion.origin_x + 1);
           freshbox.ipfb_ylen =
             (subregion.lowcorner_y - subregion.origin_y + 1);
           ioctl (ipfb, IPFB_SIOBOX, &freshbox);
           lseek (ipfb, 0L, 0);
           read (ipfb, freshbuf, freshbox.ipfb_xlen * freshbox.ipfb_ylen);
           break;
    }
    firstime = TRUE;
}
}


keyproc () {
/* A procedure that executes ONE shell command, given to it as input, and
 * returns the control to imagetool.
*/

    int     status,
            pid;
    char    cmdptr[MAX_COMMANDLEN];

    ioctl (ipfb, IPFB_SIOBOX, &movbox);
    lseek (ipfb, 0L, 0);
    write (ipfb, cursorsave, CURSOR_SIZE);
    fflush (&_iob[ipfb]);
    printf ("write shell command line (no more then %d characters)\n",
           MAX_COMMANDLEN);
    scanf("%[^\n]",cmdptr);

    /* A format that scans a string that terminates with \n */

    getchar();                  /* dispose \n */
       if ((pid=fork ()) == 0) {
           execl ("/bin/sh", "sh", "-c", cmdptr, 0);
           fprintf (stderr, "could not execute command\n");
           fflush (stderr);
       }
    while(wait (&status) != pid );
    fflush (stdout);
    firstime = TRUE;
}



photoproc () {
/* A procedure that calls rframe, and simultaneously draws a histogram of
 * the screen in the window.
 * Photoproc sometimes has problems in allocating core for the histogram,
 * in these cases it will notify the user.
 * Potoproc also changes the current default rectangle to the whole screen.
*/

    int     pid,
            rpid,
            spid,
            status,
            statusj;
    char    safile[MAX_FILENAMELEN],
            cmdptr[MAX_COMMANDLEN],
            cmdpt[MAX_COMMANDLEN],
           *shift;

    if (device[9] == '0')
      shift = "-x 11";
    else
      shift = "";
    printf ("write file name for the picture\n");
    scanf ("%s", safile);
    if ((rpid = fork ()) == 0) {
       sprintf (cmdptr, "rframe -d %s > %s", device, safile);
       execl ("/bin/sh", "sh", "-c", cmdptr, 0);
    }
    sleep (5); /* A few seconds for sincronizing */
    if ((pid = fork ()) == 0)
       while (TRUE) {
           if ((spid = fork ()) == 0) {
               sprintf (cmdpt, "rbuffer -d %s %s |histo|histotool %d",
                        device, shift , tlsw -> ts_windowfd);
               execl ("/bin/sh", "sh", "-c", cmdpt, 0);
           }
           while (wait (&statusj) != spid);
       }
     while (wait (&status) != rpid);
    kill (pid, 9); /* Terminate the histograms drawing */
    firstime = TRUE;

/* Change default rectangle to full screen */

    subregion.origin_x = 0;
    subregion.origin_y = 0;
    subregion.lowcorner_x = IPFB_SIZEX - 1;
    subregion.lowcorner_y = IPFB_SIZEY - 1;
}

helpproc()
{
/* Prints some guidance on usage of imagetool */
    int     pid,
            status;
    if ((pid = fork ()) == 0) {
       execl ("/bin/sh", "sh", "-c",
               "more /horef/image/ip512/cmd/imtoolhelp", 0);
    }
    while (wait (&status) != pid);
    fflush (stdout);
}

