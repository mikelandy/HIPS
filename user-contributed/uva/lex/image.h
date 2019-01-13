/*************************************************************
                         image.h  

     header file for programs accessing the IP-512 boards
	!!!!! This definition includes the ALU. !!!!

**************************************************************/

/* FB boards are configured in two sets.  Set S is 16 bits wide,
   and set C is only 8 bits wide.  The original designations 
   FB and FBW will refer only to set S.
   The FB and ALU boards have 16 contiguous registers for each set.
   The RGB board has only 8.
 */

/* FB-512 registers
	registers are defined as char as they are 8 bits
	68000 compiler has 16-bit short ints */

struct fb {
	char	XLO;		/* FBbase    */
	char	XHI;		/* FBbase+1  */
	char	YLO;		/* FBbase+2  */
	char	YHI;		/* FBbase+3  */
	char	PANLO;		/* FBbase+4  */
	char	PANHI;		/* FBbase+5  */
	char	SCRLO;		/* FBbase+6  */
	char	SCRHI;		/* FBbase+7  */
	char	FBCTRLO;	/* FBbase+8  */
	char	FBCTRHI;	/* FBbase+9  */
	char	SPACE1;		/* FBbase+10 reserved */
	char	SPACE2;		/* FBbase+11 reserved */
	char	MASKLO;		/* FBbase+12 */
	char	MASKHI;		/* FBbase+13 */
	char	PIXELLO;	/* FBbase+14 */
	char	PIXELHI;	/* FBbase+15 */
	};

/* remember that the lo and hi bytes in each word are switched. */

struct fbw {
	short	X;		/* FBbase    */
	short	Y;		/* FBbase+2  */
	short	PAN;		/* FBbase+4  */
	short	SCROLL;		/* FBbase+6  */
	short	FBCTRL;		/* FBbase+8  */
	short	FBSPACE;	/* FBbase+10 reserved */
	short 	MASK;		/* FBbase+12 */
	short 	PIXEL;		/* FBbase+14 */
	};

/* RGB-512 registers  */
struct rgb {
	char	LUTDATA;	/* RGBbase   */
	char	LUTADDR;	/* RGBbase+1 */
	char	LUTSEL;		/* RGBbase+2 */
	char	RGBCTRL;	/* RGBbase+3 */
	char	LEVEL;		/* RGBbase+4 */
	char	GAIN;		/* RGBbase+5 */
	char	SPACE5;		/* RGBbase+6 */
	char	SPACE6;		/* RGBbase+7 */
	};

struct rgbw {
	short	LUTDEF;		/* RGBbase   */
	short	RGBCTRL;	/* RGBbase+2 */
	short	SPACE7;		/* RGBbase+4 */
	short	SPACE8;		/* RGBbase+6 */
	};

/* alu registers */
struct alu {
	char	CONST1;		/* ALUbase    */
	char	CONST2;		/* ALUbase+1  */
	char	CONST3;		/* ALUbase+2  */
	char	SPACE1;		/* ALUbase+3  */
	char	SPACE2;		/* ALUbase+4  */
	char	SPACE3;		/* ALUbase+5  */
	char	SPACE4;		/* ALUbase+6  */
	char	SPACE5;		/* ALUbase+7  */
	char	ALUCTRL;	/* ALUbase+8  */
	char	SHFCTRL;	/* ALUbase+9  */
	char	MLTCTRL;	/* ALUbase+10 */
	char	SPACE6;		/* ALUbase+11 */
	char	INCTRL1;	/* ALUbase+12 */
	char	INCTRL2;	/* ALUbase+13 */
	char	INCTRL3;	/* ALUbase+14 */
	char	OUTCTRL;	/* ALUbase+15 */
	};

#ifndef IMAGE_H
#define IMAGE_H
struct fb *FB, *FBS, *FBC, *FBB, *FBG, *FBR;
struct fbw *FBW, *FBSW, *FBCW, *FBBW, *FBGW, *FBRW;
struct rgb *RGB;
struct rgbw *RGBW;
struct alu *ALU;
#else
extern struct fb *FB, *FBS, *FBC, *FBB, *FBG, *FBR;
extern struct fbw *FBW, *FBSW, *FBCW, *FBBW, *FBGW, *FBRW;
extern struct rgb *RGB;
extern struct rgbw *RGBW;
extern struct alu *ALU;
#endif

#define INCXWR	0x25
#define DECXWR	0x24
#define INCYWR	0x2a
#define DECYWR	0x28
#define INCXRD	0x15
#define DECXRD	0x14
#define INCYRD	0x1a
#define DECYRD	0x18

#define ROWMAX	480
#define COLMAX	512
#define ROWCENTER	ROWMAX/2
#define COLCENTER	COLMAX/2
#define ROWQUARTER	ROWMAX/4
#define COLQUARTER	COLMAX/4

#define YES  1
#define NO   0

/* ITEC INIT DEFINITIONS */
#define STD 0
#define INITALL 1
#define ALUINIT 2
#define FBINIT 3
#define RGBINIT 4

/* FRAME BUFFER "COLOR" DEFINITIONS */
#define RED 'r'
#define GREEN 'g'
#define BLUE 'b'
#define COLOR 'C'
#define BW 'w'
#define SHORT 's'
#define CHAR 'c'

/* GRAYLEVEL MAX and MIN VALUES */
#define BLACK 0
#define WHITE 255

/* LETTER SIZES */
#define SMALL 1
#define LARGE 0
