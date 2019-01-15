/*
 * boxfunc.c -- routines for drawing boxes
 *
 */

#include "ui.h"
#include "display.h"
#include "log.h"
#include "reg.h"


box_init()
{
  clear_info();
  lab_info("Please select a box",1);
  lab_info("Hit <eval> when finished",2);

  setrtype(BOX);

  return 0;
}

box_eval()
{
  XPoint p1,p2;
  static char l[80];

  p1.x=curfunc->reg->r_plist->pt.x;
  p1.y=curfunc->reg->r_plist->pt.y;
  p2.x=curfunc->reg->r_plist->next->pt.x;
  p2.y=curfunc->reg->r_plist->next->pt.y;
  sprintf(l,"Box selected from: (%d,%d) to (%d,%d)", p1.x,p1.y,p2.x,p2.y);
  clear_info();
  lab_info(l,1);
  printf(l);
  return 0;
}

box_clear()
{

  clear_info();

  return 0;
}
  
