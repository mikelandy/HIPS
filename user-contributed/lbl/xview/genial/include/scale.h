/*
 * scale.h -- definitions for lables on a graph with "clean" divisions
 *
 */

#define LSIZE 8 /* max # of characters in a label */

struct graph_lab {
  int val; /* the value at this position */
  char lab[LSIZE]; /* the string to display as the label */
  int p_off; /* the pixel offset into the graph */
};

extern int build_glab();

/* definitions for xscale.c */
#define VERTICAL 1
#define HORIZONTAL 2
