/* hips_pic: hooking hips package into pic package */

#include "pic.h"
#include "hips.h"

static Pic_procs pic_hips_procs = {
	(char *(*) ()) hips_open, hips_close,
	hips_get_name,
	hips_clear, hips_clear_rgba,

	hips_set_nchan, hips_set_box,
	hips_write_pixel, hips_write_pixel_rgba,
	hips_write_row, hips_write_row_rgba,

	hips_get_nchan, hips_get_box,
	hips_read_pixel, hips_read_pixel_rgba,
	hips_read_row, hips_read_row_rgba,
	hips_dup
};

Pic       pic_hips = {"hips", &pic_hips_procs};

