

static char usage[] = "\
Usage: scale_geom [options]\n\
-s %d%d%d%d     source box (row col #rows #cols)\n\
-dc %d	 	number of cols in destination file (default = source size) \n\
-dr %d	 	number of rows in the destination file  \n\
-d %d%d 	number of cols, rows in file dest file \n\
-filt %s[%s     filter name in x and y (default=triangle)\n\
                        \"-filt '?'\" prints a filter catalog\n\
-supp %f[%f     filter support radius\n\
-blur %f[%f     blur factor: >1 is blurry, <1 is sharp\n\
-window %s[%s   window an IIR filter (default=blackman)\n\
-debug %d       print filter coefficients\n\
-xy             filter x before y\n\
-yx             filter y before x\n\
-2              output image is 2 times the size of the input image\n\
-3              output image is 3 times the size of the input image\n\
-plain          disable filter coercion\n\
-keep0          keep zeros in xfilter\n\
Where %d denotes integer, %f denotes float, %s denotes string,\n\
and '[' marks optional following args\n\
";

