/*	PROGRAM
 *		home (RGB configuration)
 *
 *	PURPOSE
 *		to undo the zoom and put the pan and scroll registers 
 *		at 0,0 which will cause the original, unzoomed display
 *		to be centered in the frame buffer
 *
 *	SYNTAX
 *		home
 *	
 *	AUTHOR
 *		Charles Carman
 *		for
 *		Merickel Imaging Lab
 *		Biomedical Engineering
 *		University of Virginia
 *		Charlottesville, VA   22903
 *
 */
#include <image.h>
#include <lexioctl.h>

main()
{
	int lexflg, devc;
	unsigned short homex;
	short err, chan, x, y, pwr;

	if ((devc = getdev()) == 0) 
		exit(1);
	lexflg = (devc == 'L') ? 1 : 0;

	if (lexflg) {
		x = y = 0;
		pwr = 1;
		dsopn_(&err, &chan);
		dszom_(&x, &y, &pwr);
		dscls_();
	} else {
		itecinit(STD);

		swab(&FBSW->PAN, &homex, 2);
		if (homex >= COLCENTER) homex -= COLCENTER;
		if (homex >= COLQUARTER) homex -= COLQUARTER;

		dohome(homex);
	}
}
