/*
 * spline.c -- routines for splines for labelling images
 *
 */

#include "display.h"
#include "ui.h"
#include "log.h"
#include "reg.h"

spline_init()
{
  setrtype(SPLINE);

  clear_info();
  lab_info("Select points for spline",1);
  lab_info("Hit <eval> when finished",2);

  return 0;
}

spline_eval()
{
  clear_info();

  return 0;
}

spline_clear()
{
  clear_info();

  return 0;
}

