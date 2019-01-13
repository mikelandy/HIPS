/*
 *	3dplot_itc
 *
 *	put a hips 3d-plot on the itec display
 *
 *	Charles Carman  7/29/87
 */
#include "image.sh"

plot3d_itc(buf,nb)
	char *buf;
	int nb;
{
	double br, x1, y1, z1, x2, y2, z2;
	int in, op;

	itecinit(STD);
	unmask(BW);

	for(in=0; in<nb; ) {
		in = getplot(buf,in,&op,&br,&x1,&y1,&z1,&x2,&y2,&z2);
		switch (op) {
		case 'p':
			i_point_3d(br,x1,y1);
			break;
		case 'v':
			i_vec_3d(br,x1,y1,x2,y2);
			break;
		case 'n':
			i_endp_3d(x2,y2);
			break;
		default:
			perror("unknown op-code");
			break;
		}
	}
	mask(BW);
}

i_point_3d(b,x,y)
	double b,x,y ;
{
	unmask(BW);
	bwwrpt((int)(b+0.5), (int)(x+0.5), (int)(y+0.5));
}

i_vec_3d(b,x1,y1,x2,y2)
	double b,x1,y1,x2,y2 ;
{
	extern double cur_b, cur_x, cur_y;

	drln((short)(x1+0.5), (short)(y1+0.5), (short)(x2+0.5),
		(short)(y2+0.5), BW, (unsigned char)(b+0.5));
	cur_b = b; cur_x = x2; cur_y = y2;
}

i_endp_3d(x,y)
	double x,y ;
{
	extern double cur_b, cur_x, cur_y;

	drln((short)(cur_x+0.5), (short)(cur_y+0.5), (short)(x+0.5),
		(short)(y+0.5), BW, (unsigned char)(cur_b+0.5));
	cur_x = x; cur_y = y;
}
