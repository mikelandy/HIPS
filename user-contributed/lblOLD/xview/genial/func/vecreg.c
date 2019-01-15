/*
 * vecreg.c -- arrow-shaped regions 
 *
 */

#include "reg.h"
#include "display.h"
#include "ui.h"
#include "llist.h"
#include "common.h"
#include <X11/Xlib.h>
#include <stdio.h>
#include <math.h>

draw_vec(reg)
struct region *reg;
{
  int ht=20, wid=15;
  float   x, y, xb, yb, dx, dy, l, sina, cosa;
  int     xc, yc, xd, yd,x1,y1,x2,y2;
  XPoint pc,pd;
  struct dlist *vec1, *vec2;

  
  ras_line(reg->r_plist->pt, reg->r_plist->next->pt,
	   reg->r_dlist, orig_ximg, orig_img);
  vec1=(struct dlist *) malloc(sizeof(struct dlist));
  vec2=(struct dlist *) malloc(sizeof(struct dlist));
  x1=reg->r_plist->pt.x;
  y1=reg->r_plist->pt.y;
  x2=reg->r_plist->next->pt.x;
  y2=reg->r_plist->next->pt.y;
  dx = x2 - x1;  dy = y1 - y2;
  l = sqrt((double)(dx*dx + dy*dy));
  if(l == 0)
    return;
  sina = dy / l;  cosa = dx / l;
  xb = x2*cosa - y2*sina;
  yb = x2*sina + y2*cosa;
  x = xb - ht;
  y = yb - wid / 2;
  xc = x*cosa + y*sina + .5;
  yc = -x*sina + y*cosa + .5;
  y = yb + wid / 2;
  xd = x*cosa + y*sina + .5;
  yd = -x*sina + y*cosa + .5;
  pc.x=xc;
  pc.y=yc;
  pd.x=xd;
  pd.y=yd;
  ras_line(pc,reg->r_plist->next->pt, vec1, orig_ximg, orig_img);
  ras_line(pd,reg->r_plist->next->pt, vec2, orig_ximg, orig_img);
  llist_add((llist *) vec1, (llist **) &reg->r_dlist, (llist **) NULL);
  llist_add((llist *) vec2, (llist **) &reg->r_dlist, (llist **) NULL);

  draw_dlist(img_win->d_xid, reg->r_dlist);
}
