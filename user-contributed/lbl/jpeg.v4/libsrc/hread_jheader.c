/*	Hips_READ_JpegHEADER . C
%
%
*/

#include "jinclude.h"

U_IMAGE   uimg;
struct Decompress_info_struct dinfo;
struct Decompress_methods_struct dc_methods;
struct External_methods_struct e_methods;

METHODDEF void
d_ui_method_selection(decompress_info_ptr cinfo)
{
    /* if grayscale or CMYK input, force similar output; */
    /* else leave the output colorspace as set by options. */
    if (cinfo->jpeg_color_space == CS_GRAYSCALE)
	cinfo->out_color_space = CS_GRAYSCALE;
    else if (cinfo->jpeg_color_space == CS_CMYK)
	cinfo->out_color_space = CS_CMYK;

/*
	jselwdefault(cinfo);
*/
}

/* this routine reads the main JFIF header and the scan header for the
   first frame */
hread_jmain_header(FILE * fp)
{

    dinfo.img = &uimg;
    dinfo.methods = &dc_methods;
    dinfo.emethods = &e_methods;
    jselerror(&e_methods);	/* error/trace message routines */
    jselmemmgr(&e_methods);	/* memory allocation routines */
    dc_methods.d_ui_method_selection = d_ui_method_selection;
/*
	emethods = &e_methods;
*/
    j_d_defaults(&dinfo, TRUE);

    /* main set up	 */
    jpeg_uimg_init(fp, HIPS, 0, True);
    dinfo.input_file = fp;
    dinfo.output_file = stdout;
    jselrjfif(&dinfo);

    if ((*dinfo.methods->read_file_header) (&dinfo) < 0
		|| ! ((*dinfo.methods->read_scan_header) (&dinfo)))
	return EOF;		/* error	 */
    return 0;
}

hread_jscan_header(FILE * fp)
{
    return !(*dinfo.methods->read_scan_header) (&dinfo);
}
