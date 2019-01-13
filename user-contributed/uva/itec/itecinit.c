/*	PROCEDURE
 *		itecinit
 *
 *	PURPOSE
 *		-  to map the multibus register addresses to 
 *			the programs virtual memory address space
 *		-  to initialize the frame buffer registers
 *		-  to quickly initialize the alu registers
 *		-  to initialize the rgb control registers
 *
 *	SYNTAX
 *		itecinit (itecboard)
 *
 *	AUTHOR 
 *		Chuck Carmen
 *		for
 *		Merickel Imaging Lab
 *		Biomedical Engineering
 *		University of Virginia
 *		Charlottesville, VA 22903
 */
#include <stdio.h>
#include "image.sh"

itecinit(reg)
int reg;
{
	/* for the COSMOS use the phys call  */
	/* phys (0, 0xe00000, 0x40, 0xe00000);  */
	/* for the MASSCOMP use allocate memory and use the pmapm call */
	char *p, *valloc();
	int retval, pmapm();

	if ((p = valloc(4096)) != NULL)  {
		FB = FBS = FBB = FBG = (struct fb *)p;
		FBW = FBSW = FBBW = FBGW = (struct fbw *) FB;
		p = p + 0x10;
		FBC = FBR = (struct fb *) p;
		FBCW = FBRW = (struct fbw *) p;
		p = p + 0x10;
		RGB = (struct rgb *) p;
		RGBW = (struct rgbw *) p;
		p = p + 0x10;
		ALU = (struct alu *) p;
		/* now make the mapping to the actual registers */
		if ((retval = pmapm(FB, 4096, 0xf60000)) == -1)  {
			printf("itecinit: error mapping to itec boards\n");
			exit(2);
		}
	}
	else  {
		printf("itecinit: error setting up access to itec boards\n");
		exit(3);
	}

	switch(reg) {
		case INITALL:
			aluinit();
			rgbinit();
			fbinit(COLOR);
			break;;
		case FBINIT:
			fbinit(BW);
			break;;
		case ALUINIT:
			aluinit();
			break;;
		case RGBINIT:
			rgbinit();
			break;;
	}
}

/*
 *	PROCEDURES
 *		aluinit
 *		fbcinit
 *		fbinit
 *		fbsinit
 *		rgbinit
 *
 *	SYNOPSIS
 *		aluinit
 *			initializes the alu
 *		fbcinit
 *			initializes the character frame buffer (RED fb).
 *		fbinit
 *			initializes any of the three frame buffers
 *		fbcinit
 *			initializes the short frame buffers (BLUE and GREEN fb).
 *		rgbinit 
 *			initializes the rgb control.
 */

aluinit()			/* INITIALIZE ALU		*/
{
	/* set up the ALU registers for displaying FB set A */
	ALU->CONST1 = 0;
	ALU->CONST2 = 0;
	ALU->CONST3 = 0;
	ALU->ALUCTRL = 0x05;
	ALU->SHFCTRL = 0;
	ALU->MLTCTRL = 0;
	ALU->INCTRL1 = 0x44;
	ALU->INCTRL2 = 0x44;
	ALU->INCTRL3 = 0x00;
	ALU->OUTCTRL = 0;
}

fbcinit()			/* Initialize FBC 	 */
{
	FBCW->PAN = 0;
	FBCW->SCROLL = 0;
	FBCW->X = 0;
	FBCW->Y = 0;
	FBC->MASKLO = 0; 
	FBCW->FBCTRL = 0;
}

fbinit(color) 			/* INITIALIZE FRAME BUFFERS	*/
char color;
{
	switch (color) {
	default:
	case RED:
	case CHAR:
		fbcinit();
		if (color == RED || color == CHAR)
			break;
	case BLUE:
	case GREEN:
	case SHORT:
		fbsinit(color);
		break;
	}
}

fbsinit(color)	 		/* Initialize FBS 	 */
char color;
{
	FBSW->X = 0;
	FBSW->Y = 0;
	FBSW->FBCTRL = 0;
	switch(color)
	{
	case BLUE:
		FBS->PANLO = 0;
		FBS->SCRLO = 0;
		FBS->MASKLO = 0; 
		break;
	case GREEN:
		FBS->PANHI = 0;
		FBS->SCRHI = 0;
		FBS->MASKHI = 0; 
		break;
	default:
	case SHORT:
		FBSW->PAN = 0;
		FBSW->SCROLL = 0;
		FBSW->MASK = 0; 
		break;
	}
}

rgbinit()			/* INITIALIZE RGB		*/
{
	/* use the XTAL mode for all of the following initializations */
	RGB->RGBCTRL = 0;
}


