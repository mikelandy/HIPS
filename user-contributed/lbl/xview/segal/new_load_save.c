#include "header.def"
#include "imagedef.h"

U_IMAGE	img;

main(argc, argv)
int argc;
char **argv;
{
	format_init(&img, IMAGE_INIT_TYPE, HIPS, *argv, "January 18, 1992");

	in_fp = fopen("bird.hips", "r");

	if((*img.header_handle)(HEADER_READ, &img, 0, 0, 0))
		prgmerr(1, "Unknown Image Format");
	
	(*img.friend) (FI_LOAD_FILE, &img, 0, 0);

	fprintf(stderr, "Rows : %d\n", img.height);
	fprintf(stderr, "Cols : %d\n", img.width);
	fprintf(stderr, "Frame: %d\n", img.frames);
	message("buffer size : %u\n", pointer_buffer_size(img.src));
}

