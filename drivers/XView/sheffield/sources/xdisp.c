/* xdisp itself */

#include <hipl_format.h>
#include <xdisp.h>

extern Filename filename;

main(argc,argv)
  int argc;
  char *argv[];
{
  int i;

    x_init(&argc,argv);

    hips_init(argc,argv);
    read_hips_image(filename);

    sample_image();

    create_widgets();

    graphics_init();

    create_hists();

    create_cursor_list();
    create_lut_list();

    frame = 0;

    for (i = 0; i < 4; i++) {
	xmap[i] = (unsigned char *)malloc(MAXCOLOR);
        set_xmap(i,0,MAXCOLOR-1);
	gamma_limits[i].llx = 0;
	gamma_limits[i].ulx = HIST_WIDTH-1;
	}

    set_lut(current_lut);

    create_colorbar();

    create_image();

    resize(draw,&image_pix_data,NULL);

    add_events();
    xloop();

}
