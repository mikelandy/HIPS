/*
 * common.c -- routines for common operations, i.e. getting a pixel value,
 *             that are used repeatedly
 *
 */

#include "ui.h"
#include "display.h"
#include "llist.h"
#include "plist.h"
#include "common.h"
#include "scale.h"
#include <ctype.h>
#include <strings.h>

static struct pval prune_buf[4000];	/* buffer of pruned point list. */
static int nprune;

/* makepbuf() -- makes a continous pbuf from a linked list of pbstores. This
   is useful for things like traces which have to have interactive access to
   the points which the region is made of */
/* makepbuf() calls prune().  The two of these could very easily be
   combined into one functions and the whole thing would be much more
   efficient if we did that.  FIX THIS!! */

int
makepbuf(src, dst)
    struct dlist *src;
    struct pval **dst;
{
    struct dlist *trav;
    struct pval *buf = NULL;
    int       npts = 0;

    for (trav = src; trav != NULL; trav = trav->next) {
	if (buf == NULL) {
	    buf = (struct pval *) malloc(trav->len * sizeof(struct pval));
	} else {
	    buf = (struct pval *) realloc((char *) buf,
				  (npts + trav->len) * sizeof(struct pval));
	}
	bcopy((char *) trav->points, (char *) (buf + npts),
	      trav->len * sizeof(struct pval));
	npts += trav->len;
    }
    prune(buf, npts);
    /* now copy the pruned point buffer to dst */
    free(buf);
    buf = (struct pval *) malloc(nprune * sizeof(struct pval));
    bcopy((char *) prune_buf, (char *) buf, nprune * sizeof(struct pval));
    *dst = buf;
    npts = nprune;
    nprune = 0;
    return npts;
}

/* prune removes duplicate points which could show up from a spline trace and
   should also get the pixel value off the ORIGINAL image as opposed to the
   image which has been scaled in the appropriate manner for the colormap. */

int
prune(plst, npts)
    struct pval *plst;
    int       npts;
{
    struct pval *tr;
    int       s, t, found;

    tr = prune_buf + 1;
    bcopy((char *) plst, (char *) prune_buf, sizeof(struct pval));
    for (t = 0; t <= (npts - 4); t++) {
	found = 0;
	for (s = 1; s <= 4; s++) {
	    if (plst[t].pt.x == plst[t + s].pt.x && plst[t].pt.y == plst[t + s].pt.y)
		found = 1;
	}
	if (found == 0) {
	    bcopy((char *) (&plst[t]), (char *) tr, sizeof(struct pval));
	    nprune++;
	    tr++;
	}
    }
}

/* box_vec() -- build a vector which is the average across of the pixel
 * values in a rectangular region
 */
box_vec(pl, obuf)
    struct plist *pl;
    struct pval **obuf;
{
    int       len, width, orient, x, y, val;
    struct pval *pbuf, *ptmp;

    /* compute the direction of the rectangle */
    orient = box_direction(pl);

    switch (orient) {
    case HORIZONTAL:
	len = pl->next->pt.x - pl->pt.x;
	width = pl->next->pt.y - pl->pt.y;
	pbuf = (struct pval *) malloc(len * sizeof(struct pval));
	ptmp = pbuf;
	for (x = pl->pt.x; x < pl->next->pt.x; x++) {
	    val = 0;
	    for (y = pl->pt.y; y < pl->next->pt.y; y++) {
		val += getpix(orig_img, x, y);
	    };
	    val = val / width;
	    /*
	     * okay we have computed an average value, lets add it to the
	     * point buffer.
	     */
	    ptmp->pt.x = (short) x;
	    ptmp->pt.y = (short) pl->pt.y + width / 2;
	    ptmp->val = XGetPixel(orig_ximg, ptmp->pt.x, ptmp->pt.y);
	    ptmp->oval = val;
	    ptmp++;
	}
	break;
    case VERTICAL:
	len = pl->next->pt.y - pl->pt.y;
	width = pl->next->pt.x - pl->pt.x;
	pbuf = (struct pval *) malloc(len * sizeof(struct pval));
	ptmp = pbuf;
	for (y = pl->pt.y; y < pl->next->pt.y; y++) {
	    val = 0;
	    for (x = pl->pt.x; x < pl->next->pt.x; x++) {
		val += getpix(orig_img, x, y);
	    };
	    val = val / width;
	    /*
	     * okay we have computed an average value, lets add it to the
	     * point buffer.
	     */
	    ptmp->pt.x = (short) pl->pt.x + width / 2;;
	    ptmp->pt.y = (short) y;
	    ptmp->val = XGetPixel(orig_ximg, ptmp->pt.x, ptmp->pt.y);
#ifdef DEBUG
	    printf("(%d,%d): Local Value: %d, Averaged Value: %d\n", (int) ptmp->pt.x,
		 (int) ptmp->pt.y, getpix(orig_img, ptmp->pt.x, ptmp->pt.y),
		   val);
#endif
	    ptmp->oval = val;
	    ptmp++;
	};
	break;
	/* no default case */
    }
    *obuf = pbuf;
    return len;
}

/* getpix() -- return the gray value for a pixel given an original image */
unsigned long
getpix(img, x, y)
    struct img_data *img;
    unsigned long x, y;
{
    return dval(x, y, img, 0);
}


int
slope(dl)
    struct dlist *dl;
{
    double    x1, y1, x2, y2;

    x1 = (double) dl->points[0].pt.x;
    y1 = (double) dl->points[0].pt.y;
    x2 = (double) dl->points[dl->len - 1].pt.x;
    y2 = (double) dl->points[dl->len - 1].pt.y;
    return irint((y2 - y1) / (x2 - x1));
}

double
distance(pt1, pt2)
    XPoint    pt1, pt2;
{
    register double x1, y1, x2, y2, dsq;

    x1 = (double) pt1.x;
    y1 = (double) pt1.y;
    x2 = (double) pt2.x;
    y2 = (double) pt2.y;

    dsq = ((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    return (sqrt(dsq));
}

double
idist(a, b, a2, b2)
    int       a, b, a2, b2;
{
    register double x1, y1, x2, y2, dsq;

    x1 = (double) a;
    y1 = (double) b;
    x2 = (double) a2;
    y2 = (double) b2;

    dsq = ((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    return (sqrt(dsq));
}

/* routines for adding and deleting elements from a linked list */

/* llist_add -- add an element to the end of a linked list */
llist_add(ele, head, tail)
    llist    *ele, **head, **tail;
{
    llist    *trav;

    if (*head == NULL) {
	*head = ele;
	(*head)->next = NULL;
	(*head)->prev = NULL;
	if (tail != NULL) {
	    (*tail) = *head;
	}
	return;
    }
    /* there is at least one element */
    for (trav = *head; trav->next != NULL; trav = trav->next) {
    }
    trav->next = ele;
    ele->prev = trav;
    ele->next = NULL;
    if (tail != NULL) {
	*tail = ele;
    }
}

/* llist_del() -- delete a given entry from a linked list */
llist_del(ent, head, tail)
    llist    *ent, **head, **tail;
{
    llist    *trav;

    trav = ent;
    if (trav == *head) {
#ifdef WHY_DID_ANTONY_DO_THIS
	/* it could be the tail also! */
	if (tail != NULL)
	    if (trav == *tail)
		*tail = NULL;
#endif
	*head = trav->next;
	if (trav->next != NULL)
	    trav->next->prev = NULL;
	free(trav);
    } else {
	/* not the head, but is it the tail? */
	if (trav == *tail) {
	    *tail = (trav->prev);
	    if (trav->prev != NULL)
		trav->prev->next = NULL;
	    free(trav);
	} else {
	    /* an element in the middle */
	    trav->prev->next = trav->next;
	    trav->next->prev = trav->prev;
	    free(trav);
	}
    }

}

/* llist_free() -- free every element in a linked list */
llist_free(head)
    llist   **head;
{
    llist    *trav, *tmp;

    if (*head == NULL)
       return;

    if ((*head)->prev != NULL) {
	(*head)->prev->next = NULL;
    }
    /* traverse to list end */
    for (trav = tmp = *head; trav != NULL; trav = trav->next, tmp = trav) {
	free(tmp);
    };
    *head = NULL;
}

/* llist_depth() -- length of a linked list */
llist_depth(head)
    llist    *head;
{
    llist    *trav;
    int       len = 0;

    for (trav = head; trav != NULL; trav = trav->next)
	len++;

    return len;
}

/* llist_tail() -- find the tail in a linked list */
llist
* llist_tail(head)
    llist    *head;
{
    llist    *trav;

    trav = head;
    if (trav->next == NULL)
	return trav;
    else
	while (trav->next != NULL)
	    trav = trav->next;
    return trav;
}


unsigned long
dval(x, y, img, avgrad)
    int       x, y;
    struct img_data *img;
    int       avgrad;		/* number of points to average around the x,y
				 * value */
{
    register int xlen, ylen;
    register u_long sum = 0, cnt = 0;
    int       i, j;

    if (avgrad == 0) {
	switch (img->dsize) {
	case 1:
	    return (unsigned long) *((unsigned char *) paddr(x, y, img));
	case 2:
	    return (unsigned long) *((unsigned short *) paddr(x, y, img));
	case 4:
	    return (unsigned long) *((unsigned int *) paddr(x, y, img));
	}
    }
    xlen = x + avgrad;
    ylen = y + avgrad;

    for (i = (x - avgrad); i <= xlen; i++) {
	for (j = (y - avgrad); j <= ylen; j++) {
	    switch (img->dsize) {
	    case 1:
		sum += (unsigned long) *((unsigned char *) paddr(i, j, img));
		break;
	    case 2:
		sum += (unsigned long) *((unsigned short *) paddr(i, j, img));
		break;
	    case 4:
		sum += (unsigned long) *((unsigned int *) paddr(i, j, img));
		break;
	    }
	    cnt++;
	}
    }
    return sum / cnt;
}


/* routine to find the first occurence of string s2 in s1 */
char     *
fstring(s1, s2)
    char     *s1, *s2;
{
    char     *tmp;

    tmp = index(s1, *s2);
    while (tmp != NULL) {
	if (strncmp(tmp, s2, strlen(s2)) == 0) {
	    return tmp;
	} else {
	    tmp = index((char *) tmp + 1, *s2);
	}
    }
    return tmp;
}

/* read a hex string and return a single byte */
u_char
readhex(str)
    char     *str;
{
    register u_char val = 0;

    if (isdigit(*str))
	val = (*str - '0') * 16;
    else {
	if (islower(*str))
	    val = (*str - 'a' + 10) * 16;
    }
    str++;
    if (isdigit(*str))
	val += (*str - '0');
    else {
	if (islower(*str))
	    val += (*str - 'a' + 10);
    }
#ifdef DEBUG
    printf("readhex: %d\n", (int) val);
#endif
    return val;
}

/* routine to test whether or not a point is within the bounds of an XImage */
/* returna 1 if true, 0 if false */
int
pt_in_xim(x, y, xim)
    int       x, y;
    XImage   *xim;
{
    if ((x < 0) || (y < 0)) {	/* negative #s AIGH! */
	return 0;
    }
    if ((x >= xim->width) || (y >= xim->height))
	return 0;
    return 1;
}
