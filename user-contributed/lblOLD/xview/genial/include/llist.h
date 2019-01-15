/*
 * llist.h --  definitions for linked-list manipulation routines provided by
 *             arch/base/common.c
 *
 */

typedef struct lltype {
  struct lltype *next, *prev;
} llist;

extern list_add(), list_del(), llist_depth();

extern llist *llist_tail();
