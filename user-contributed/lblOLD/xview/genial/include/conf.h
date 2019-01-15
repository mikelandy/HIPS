/*
 * conf.h -- function configuration definitions 
 *
 */

struct fxnsw {
  int (*f_init)();
  int (*f_eval)();
  int (*f_clear)();
  int (*f_reset)();
  int (*f_change)();
};

