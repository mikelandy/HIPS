/* macros.h
 * Max Rible
 *
 * Macro definitions used in HIPStool.
 * This file is included in "hipstool.h".
 */

/* Shut lint up */
#define Calloc(a,b) (b *) calloc((unsigned)(a), sizeof(b))
#define Realloc(a,b,c) (c *) realloc((char *)(a), (unsigned)(b)*sizeof(c))
#define Cfree(a,b,c) cfree((char *)(a), (unsigned)(b), sizeof(c))
#define Fread(a,b,c,d) fread((char *)(a), sizeof(b), (int)(c), d)
#define Fwrite(a,b,c,d) fwrite((char *)(a), sizeof(b), (int)(c), d)
#define Memcpy(a,b,c) memcpy((char *)(a), (char *)(b), (int)(c))
#define Strdup(s) (char *) strdup((char *)(s))

/* Simplifies the line drawing business. */
#define Draw2D(a,b,c,d,i) doline2d(pairarr[a][0], pairarr[b][1], \
	pairarr[c][0], pairarr[d][1], tmplines+i, 0, 1)
#define Connect2D(a,b,i) Draw2D(a,a,b,b,i)

#define Win_width(foo) (((foo)->width > MAX_WINX) ? MAX_WINX : (foo)->width)
#define Win_height(foo) (((foo)->height > MAX_WINY) ? MAX_WINY : (foo)->height)

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

/* SCALE scales a quantity between 0 and 1 to chars. */
#define SCALE(x) (fabs(x) >= 1.0 ? 255 : (int)(255.0*fabs(x)))

/* Cursor position reporting update stuff. */
#define CURS_LEN 100
#define Update_cursor(a, b, g) sprintf(cursortext, \
        "Row (y): %4d  Col (x): %4d  Gray: %4u", (b), (a), (g)); \
        panel_set(io.messages[MESSAGE_CURSOR], PANEL_LABEL_STRING, \
	cursortext, 0)
#define Update_gray(left, gray0, right, gray1) sprintf(cursortext, \
	"Left (%3d):  %3d  Right (%3d):  %3d.", (left), (gray0), \
	(right), (gray1)); panel_set(io.messages[MESSAGE_CURSOR], \
	PANEL_LABEL_STRING, cursortext, 0)

/* Single-line information bar. */
#define INFO_LEN 100
#define Update_info(string) strcpy(windowtext, string); panel_set( \
	  io.messages[MESSAGE_SELECTION], PANEL_LABEL_STRING, windowtext, 0)

#define Prinfo1(string)  strcpy(infotext[0], string); panel_set( \
          io.messages[MESSAGE_INFO_1], PANEL_LABEL_STRING, infotext[0], 0)
#define Prinfo2(string) strcpy(infotext[1], string); panel_set( \
          io.messages[MESSAGE_INFO_2], PANEL_LABEL_STRING, infotext[1], 0)

#if defined(SUNTOOLS) && defined(MACROIZED)
#define colormap256(w, r, g, b) pw_setcmsname((w)->pw, Progname), \
    pw_putcolormap((w)->pw, 0, 256, (r), (g), (b))
#define put_pix(w, x, y, c) pw_put((w)->pw, (x), (y), (c))
#define get_pix(w, x, y) pw_get((w)->pw, (x), (y))
#define line(w, x1, y1, x2, y2, c) ((c == XOR_COLOR) ? pw_vector((w)->pw, \
	x1, y1, x2, y2, PIX_SRC ^ PIX_DST, 0xFF) : pw_vector((w)->pw, \
	x1, y1, x2, y2, PIX_SRC, c))
#define text(w, x, y, s, c) pw_ttext((w)->pw, x, y, PIX_SRC | PIX_COLOR(c), \
				     (Pixfont *) NULL, s)
#define wipe(w, c) pw_writebackground((w)->pw, 0, 0, (w)->width, (w)->height, \
				      PIX_SRC | PIX_COLOR(c))
#define getstring(foo) (char *) panel_get_value(foo)
#define putstring(foo, bar) panel_set_value(foo, bar)
#endif /* SUNTOOLS && MACROIZED */
