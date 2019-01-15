/*
 * vector.c -- routines for arrows which annotate an image
 *
 */

#include "display.h"
#include "ui.h"
#include "log.h"
#include "reg.h"

vector_init()
{
  setrtype(AN_VEC);

  clear_info();
  lab_info("Select tail of vector and then select head",1);
  lab_info("Hit <eval> when finished",2);

  return 0;
}

vector_eval()
{
  clear_info();

  return 0;
}

vector_clear()
{
  clear_info();

  return 0;
}
