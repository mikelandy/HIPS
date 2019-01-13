#include <stdio.h>
#include <gl.h>
#include <hipl_format.h>
#include <device.h>

#define CMAPOFFSET 256

long wsizx, wsizy;
int xstart, ystart;
long row,col;
char *buf;
Icoord ix, iy;
Colorindex *pixval;
h_boolean offsetgiven = FALSE;

main(argc,argv)
int argc;
char **argv;
{
	struct header hd;
	int i,j;
	short dev, val;


	wsizx = wsizy = xstart = ystart = 0;

	if (argc > 1) wsizx = atoi(argv[1]);
	if (argc > 2) wsizy = atoi(argv[2]);
	if (argc > 3) {
		xstart = atoi(argv[3]);
		offsetgiven = TRUE;
	}
	if (argc > 4) ystart = atoi(argv[4]);

	read_header(&hd);
	row = hd.orows;
	col = hd.ocols;
	if (hd.pixel_format != PFBYTE)
		perr("must be byte format");
	buf = (char *)halloc((unsigned) (row * col), sizeof(char));
	pixval = (Colorindex *) halloc((unsigned) col, sizeof(Colorindex));
	fread(buf, row*col,1,stdin);
	if (argc == 1) {
		prefsize(col, row);
		wsizx = col;
		wsizy = row;
	}
	else prefsize(wsizx, wsizy);
	winopen("wframe");
	qdevice(ESCKEY);
	qdevice(REDRAW);
	for(i = 0; i < 256; i++)
		mapcolor((Colorindex) (i + CMAPOFFSET) ,(short)i,(short)i,(short)i);
	drawimage();
	while (TRUE)
		while (qtest()) {
			dev = qread(&val);
			switch(dev) {
			case ESCKEY:
				free(pixval);
				free(buf);
				exit(0);
				break;
			case REDRAW:
				drawimage();
				break;
			default:
				break;
			}
		}
}

drawimage() {

	int i, j;

	color(127+CMAPOFFSET);
	clear();
	if (!offsetgiven) {
		iy = wsizy - (wsizy-row)/2 - 1;
		ix = (wsizx - col)/2;
	}
	else {
		iy = wsizy - ystart;
		ix = xstart;
	}
	for(j = 0; j<row; j++) {
		for(i=0; i<col; i++)
			pixval[i] = (buf[j*col+i] & 0377) + CMAPOFFSET;
		cmov2i(ix, iy);
		writepixels(col, pixval);
		iy--;
	}
}
