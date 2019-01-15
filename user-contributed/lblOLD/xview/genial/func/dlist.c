/*
 * dlist.c -- routines for managing display lists
 *
 */

#include <stdio.h>
#include "ui.h"
#include "display.h"
#include "plist.h"

/* draw_dlist() -- draw a linked list of segments */

draw_dlist(win, phead)
Window win;
struct dlist *phead;
{
  struct dlist *pstore;
  XGCValues gcval;
  int i;
  XPoint pt;

  gcval.foreground=standout;
  XChangeGC(display, gc, GCForeground, &gcval);
  for (pstore=phead; pstore!=NULL; pstore=pstore->next) 
    if (pstore->flags!=NODRAW) {
      for (i=0; i < pstore->len; i++) {
	itod_pt(&pstore->points[i].pt, &pt);
	XDrawPoint(display, win, gc, (int) pt.x,
		   (int) pt.y);
      }
    }
}

/* ref_dlist() -- refresh the backing store underneath a dlist */
ref_dlist(win, phead)
Window win;
struct dlist *phead;
{
  struct dlist *pstore;
  XGCValues gcval;
  XPoint pt;
  int i;

  for (pstore=phead; pstore!=NULL; pstore=pstore->next) {
    for (i=0; i < pstore->len; i++) {
      gcval.foreground=pstore->points[i].val;
      XChangeGC(display, gc, GCForeground, &gcval);
      itod_pt(&pstore->points[i].pt, &pt);
      XDrawPoint(display, win, gc, (int) pt.x,
		 (int) pt.y);
    }
  }
}

/* draw_pvec() -- draw a single point buffer  */
draw_pvec(xid, pbuf, len)
     XID xid;
     struct pval *pbuf;
     int len;
{
  register int i;
  XGCValues gcval;
  XPoint pt;

  gcval.foreground=standout;
  XChangeGC(display, gc, GCForeground, &gcval);
  for (i=0; i < len; i++) {
    itod_pt(&pbuf[i].pt,&pt);
    XDrawPoint(display, xid, gc, pt.x, pt.y);
  }
}

struct dlist
*dlist_new()
{
  struct dlist *dl;

  if ((dl=(struct dlist *) malloc(sizeof(struct dlist)))==NULL) {
    perror("malloc");
    exit(0);
  }
  dl->next=dl->prev=NULL;
  dl->points=NULL;
  dl->len=0;
  dl->flags=0;
  return dl;
}
