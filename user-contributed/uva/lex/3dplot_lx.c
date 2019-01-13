/*
 *	3dplot_lx
 *
 *	routine to display a hips 3d-plot on the Lexidata
 *
 *	Charles Carman   7/29/87
 */

plot3d_lx(buf,nb)
	char *buf;
	int nb;
{
	double br, x1, y1, z1, x2, y2, z2;
	int in, op;
	short chan, err;

	dsopn_(&err,&chan);

	for(in=0; in<nb; ) {
		in = getplot(buf,in,&op,&br,&x1,&y1,&z1,&x2,&y2,&z2);
		switch (op) {
		case 'p':
			l_point_3d(br,x1,y1);
			break;
		case 'v':
			l_vec_3d(br,x1,y1,x2,y2);
			break;
		case 'n':
			l_endp_3d(x2,y2);
			break;
		default:
			perror("unknown op-code");
			break;
		}
	}
	dscls_();
}

l_point_3d(b,x,y)
	double b,x,y ;
{
	short npix = 1;
	short ci, xydata[2];

	ci = (short)(b + 0.5);
	xydata[0] = (short)(x + 0.5);
	xydata[1] = (short)(y + 0.5);
	dspnt_(&npix,&ci,xydata);
	dsowt_();
}

l_vec_3d(b,x1,y1,x2,y2)
	double b,x1,y1,x2,y2 ;
{
	extern double cur_b, cur_x, cur_y;
	short ci, xstr, ystr, xend, yend;

	ci = (short)(b + 0.5);
	xstr = (short)(x1 + 0.5);
	ystr = (short)(y1 + 0.5);
	xend = (short)(x2 + 0.5);
	yend = (short)(y2 + 0.5);
	dsvec_(&xstr,&ystr,&xend,&yend,&ci);
	dsowt_();
	cur_b = b; cur_x = x2; cur_y = y2;
}

l_endp_3d(x,y)
	double x,y ;
{
	extern double cur_b, cur_x, cur_y;
	short ci, xstr, ystr, xend, yend;

	ci = (short)(cur_b + 0.5);
	xstr = (short)(cur_x + 0.5);
	ystr = (short)(cur_y + 0.5);
	xend = (short)(x + 0.5);
	yend = (short)(y + 0.5);
	dsvec_(&xstr,&ystr,&xend,&yend,&ci);
	dsowt_();
	cur_x = x; cur_y = y;
}
