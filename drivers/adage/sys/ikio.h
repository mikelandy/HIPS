#ifndef _SYS_IKIO_INCLUDED_
#define _SYS_IKIO_INCLUDED_
/*
 *				I K I O . H
 *
 *			Definitions for Ikonas I/O
 *
 * Merged version.  Michael Landy -- NYU/Psychology -- 6/30/85
 *			("xp!msl"@nyu, ...!cmcl2!xp!msl)
 *
 * This version, like the accompanying ik.c, merges the feature of four
 * Berkeley 4.2 drivers for the Adage/Ikonas.  These are:  Bell version
 * (Ron Gordon, 1/15/84), University of Waterloo version (Michael Herman, 
 * 3/11/85), BRL version (Mike Muuss, 9/10/84), and Rockefeller University
 * version (Kaare Christian, 4/3/84).  All of these are based on the BELL
 * driver (4.1 version, 5/9/83).  For further comments on this merging, see
 * the beginning comments of ik.c.
 */

/*
 * Define DR64 if the adage has DR64's, undef it if it has DR256's,
 * This define affects the driver and user code (see XY defn's below).
 */

#undef	DR64

/* XXX - clean these up when trustworthy: */
#define MWH850213	/* toggle to add 85-02-13 changes by mwherman */
#define MWH850311	/* toggle to add 85-03-11 changes by mwherman */

/* basic data types for the Ikonas */

typedef long	ikword;		/* 32 bits */
typedef short	ikhalfword;	/* 16 bits */
typedef char	ikbyte;		/* 8 bits */

#define	PIXEL	ikword

struct ikfcaddr {	/* structure used with IKSETFCADDR */
	short lo;	/* low 10 address bits or 11 bit X address */
	short hi;	/* high 14 address bits or 12 bit Y address */
	short fc;	/* ikonas function code, or entire control word */
};

#ifndef	KERNEL
extern int ikfd;	/* file descriptor to ikonas after ikopen */
#endif

#ifdef KERNEL
#include "../h/ioctl.h"
#else
#include <sys/ioctl.h>
#endif

#define IKSETXY		_IOW(I,0,long)	/* set pixel address, D1616(y,x) */
#define IKSETADDR	_IOW(I,1,long)	/* set address, D1410(hi,lo) */
#define IKSETFC		_IOW(I,2,long)	/* set function code 4 bits */
#define IKSETCONTROL	_IOW(I,3,long)	/* set command register bits */
#define IKSETSENDERID	_IOW(I,4,long)	/* set sender id for writes */
#define IKRUNPROCESSOR	_IO(I,5)	/* start bps32 processor */
#define IKWAITPROCESSOR	_IO(I,6)	/* wait for bps32 completion */
#define IKHALTPROCESSOR	_IO(I,7)	/* halt bps32 processor */
#define IKGETCONTROL	_IOR(I,8,long)	/* return command register bits */
					/* set fc+addr using struct ikfcaddr */
#define IKSETFCADDR	_IOW(I,9,struct ikfcaddr)
					/* set control+addr using ikfcaddr */
#define IKSETCADDR	_IOW(I,10,struct ikfcaddr)
#define IKWAITFIELD	_IOW(I,11,long)/* wait for n fields */
#define IKWINDOWDMA	_IOW(I,12,long)/* window dma to dr memory */
#define IKIOGETADDR	_IOR(I,13,caddr_t) /* get address of mapped UNIBUS
						registers */

/* XXX -- clean this up once it works */
#ifdef MWH850311
#    define IKCLEARPROCESSOR	_IO(I,14) /* clear bps-has-interrupted flag */
#    define IKTESTPROCESSOR	_IOWR(I,15,struct ikfcaddr)
					/* return bps-has-interrupted flag */
#else /* original MWH850213 changes */
#    ifdef MWH850213
#        define IKCLEARPROCESSOR _IO(I,14) /* clear bps-has-interrupted flag */
#        define IKTESTPROCESSOR	_IOW(I,15,int) /* return bps-has-interrupted flag */
#    endif MWH850213
#endif MWH850311

/*
 *	definitions of legitimate command register bits
 *	which may be set, tested, or cleared by the user.
 */

/* Only one of these modes may be selected! */
#define WORD		0000000		/* word mode */
#define HALFWORD	0000040		/* halfword mode */
#define BYTE		0004000		/* byte mode */

#define	BYTENO(i)	((i)<<14)	/* selects byte number in byte mode */
#define BYTEFIELD	BYTENO(3)
#define	SENDERID(i)	((i)<<12)	/* sender id for writes */
#define SENDERFIELD	SENDERID(7)

#define INVISIBLEIO	0000200
#define RUNPROCESSOR	0001000
#define RESET		0002000
#define FRAMEINT	0010000
#define PROCINT		0020000

/*
 *	definition of function codes.
 *	WORDIO (0) is most often used for
 *	accesses to other than frame buffer memories.
 */

#define WORDIO		000	/* normal word i/o function code */

/* frame buffer memory (DR64/DR256/GM64/GM256) function codes */

#define WORDRES		000	/* DR/GM word read and DR write */
#define LORES		002	/* DR/GM low res. read/write */
#define HIRES		003	/* DR/GM high res. read/write */
#define LORESMASK	004	/* GM low res. mask-mode write */
#define HIRESMASK	000	/* GM high res. mask-mode write */
#define LORESCOND	006	/* GM low res. conditional write */
#define HIRESCOND	007	/* GM high res. conditional write */
#define SETWRMASK	010	/* DR/GM word res. write mask write */
#define SETLRMASK	012	/* DR/GM low res. write mask write */
#define SETHRMASK	013	/* DR/GM high res. write mask write */
#define SETLRSHADE	016	/* GM low res. shade register write */
#define SETHRSHADE	017	/* GM high res. shade register write */

/* matrix multiplier function codes */
#define	MACONT		001	/* start matrix multiplier */
#define	MASTOP		002	/* stop matrix multiplier */
#define MARUN		005	/* start matrix multiplier at zero */

/* Aliases for Ron Gordon's choice of manifest names */
#define DRWORDRES	WORDRES
#define DRLORES		LORES
#define DRHIRES		HIRES
#define DRLORESMASK	SETLRMASK
#define DRHIRESMASK	SETHRMASK
#define GMLORESSHADE	SETLRSHADE
#define GMHIRESSHADE	SETHRSHADE
#define GMLORESMASK	LORESMASK
#define GMHIRESMASK	HIRESMASK
#define GMLORES		LORES
#define GMHIRES		HIRES
#define GMLORESCOND	LORESCOND
#define GMHIRESCOND	HIRESCOND

/* useful macros for forming multibit values from smaller operands */

#define D1616(a,b) ((ikword)((((a)&0177777)<<16)|((b)&0177777)))
#define D1410(a,b) ((ikword)((((a)&037777)<<10)|((b)&01777)))
#define D101010(a,b,c) ((ikword)(((((a)&01777)<<10)|((b)&01777))<<10|((c)&01777)))
#define D888(a,b,c) ((ikword)(((((a)&0377)<<8)|((b)&0377))<<8|((c)&0377)))
#define RGB10(r,g,b) D101010(b,g,r)
#define RGB8(r,g,b) D888(b,g,r)
#define RGBA8(r,g,b,a) ((ikword)((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)))
#define IK(h,l) D1410(h,l)
#define XY16(x,y) D1616(y,x)

/*
 * The following defines constuct image memory addresses.  For any given
 * write mode (PIXEL or WORD) and display mode (low resolution or high 
 * resolution - denoted with a final 'H'), there are three macros.  The first
 * constructs a full 24 bit address. The next, LO, constructs the bottom 10
 * bits of the address.  And the third, HI, constructs the top 14 bits of the
 * address. All macros mask off the appropriate number of bits for each field,
 * so no surprises should occur.  Finally, for word mode, the user supplies
 * the memory card number, the bit plane, y, and the index of the group of 8
 * (in LORES) or 32 (in HIRES) x pixels, which is the actual x value divided
 * by 8 or 32, respectively.  Also, note that the card value is 4 bits, and
 * is not exactly as described in the manual, which gives the card number in
 * octal AS IT ALIGNS IN THE REGISTER.  For example, with these macros, the
 * first red card should be referred to as "02".
 */

#ifdef DR64
#define	PIXELXY(x,y) ((ikword) (((x&0x1ff)<<1) | ((y&0x1ff)<<11)))
#define LO_PIXELXY(x,y) ((ikword) ((x&0x1ff)<<1))
#define HI_PIXELXY(x,y) ((ikword) ((y&0x1ff)<<1))
#define	PIXELXYH(x,y) ((ikword) ((x&0x3ff) | ((y&0x3ff)<<10)))
#define LO_PIXELXYH(x,y) ((ikword) (x&0x3ff))
#define HI_PIXELXYH(x,y) ((ikword) (y&0x3ff))
#define	WORDXY(card,plane,x8,y) ((ikword) (((card&0xf)<<16) | ((plane&1)<<15) \
	| (x8&0x3f) | ((y&0x1ff)<<6)))
#define LO_WORDXY(card,plane,x8,y) ((ikword) ((x8&0x3f) | ((y&0xf)<<6)))
#define HI_WORDXY(card,plane,x8,y) ((ikword) (((card&0xf)<<6) \
	| ((plane&1)<<5) | ((y&0x1ff)>>4)))
#define	WORDXYH(card,plane,x32,y) ((ikword) (((card&0xf)<<16) \
	| ((plane&1)<<15) | (x32&0x1f) | ((y&0x3ff)<<5)))
#define LO_WORDXYH(card,plane,x32,y) ((ikword) ((x32&0x1f) | ((y&0x1f)<<5)))
#define HI_WORDXYH(card,plane,x32,y) ((ikword) (((card&0xf)<<6) \
	| ((plane&1)<<5) | ((y&0x3ff)>>5)))
#else
#define	PIXELXY(x,y) ((ikword) (((x&0x3ff)<<1) | ((y&0x3ff)<<11)))
#define LO_PIXELXY(x,y) ((ikword) ((x&0x3ff)<<1))
#define HI_PIXELXY(x,y) ((ikword) ((y&0x3ff)<<1))
#define	PIXELXYH(x,y) ((ikword) ((x&0x7ff) | ((y&0x7ff)<<10)))
#define LO_PIXELXYH(x,y) ((ikword) (x&0x7ff))
#define HI_PIXELXYH(x,y) ((ikword) (y&0x7ff))
#define	WORDXY(card,plane,x8,y) ((ikword) (((card&0xf)<<18) | ((plane&1)<<15) \
	| (x8&0x3f) | ((x8&0x40)<<11) | ((y&0x1ff)<<6) | ((y&0x200)<<7)))
#define LO_WORDXY(card,plane,x8,y) ((ikword) ((x8&0x3f) | ((y&0xf)<<6)))
#define HI_WORDXY(card,plane,x8,y) ((ikword) (((card&0xf)<<8) | ((plane&1)<<5) \
	| ((x8&0x40)<<1) | ((y&0x1ff)>>4) | ((y&0x200)>>3)))
#define	WORDXYH(card,plane,x32,y) ((ikword) (((card&0xf)<<18) \
	| ((plane&1)<<15) | (x32&0x1f) | ((x32&0x20)<<12) | ((y&0x3ff)<<5) \
	| ((y&0x400)<<6)))
#define LO_WORDXYH(card,plane,x32,y) ((ikword) ((x32&0x1f) | ((y&0x1f)<<5)))
#define HI_WORDXYH(card,plane,x32,y) ((ikword) (((card&0xf)<<8) \
	| ((plane&1)<<5) | ((x32&0x20)<<2) | ((y&0x3ff)>>5) | ((y&0x400)>>4)))
#endif DR64

/* definitions of important addresses in RDS-3000 system */

#define DR256	IK(0,0)		/* beginning of lo/hi res dr memory */
#define DR256W	IK(01000,0)	/* beginning of word res dr memory */
#define FBC	IK(030000,0)	/* beginning of FBC register block */
#define FBCVPL	IK(030000,0)	/* fbc viewport location (y,x) */
#define FBCVPS	IK(030000,1)	/* fbc viewport size (y,x) */
#define FBCWL	IK(030000,2)	/* fbc window location (y,x) */
#define FBCZOOM	IK(030000,3)	/* fbc zoom factor (y,x) */
#define FBCDRC	IK(030000,4)	/* fbc display rate control */
#define FBCVC	IK(030000,5)	/* fbc video control */
#define FBCCL	IK(030000,6)	/* cursor (y,x) location registers */
#define FBCCD	IK(030000,0400)	/* cursor bit map definition */
#define XBS	IK(030200,0)	/* beginning of 34 xbs registers */
#define LUVO	IK(020300,0)	/* beginning of luvo color tables */
#define LUVOXBS	IK(020301,0)	/* luvo channel crossbar switch */
#define BPS	IK(020500,0)	/* beginning of bps registers */
#define BPSPC	IK(020500,0)	/* bps program counter (read only) */
#define BPSSTAT	IK(020500,0)	/* bps status register (write only) */
#define SR	IK(020200,0)	/* beginnig of scratch pad registers */
#define MCM	IK(020000,0)	/* beginning of microcode memory */
#define MA	IK(020400,0)	/* beginning of multiplier/accumulator */
#define MACM	IK(020400,0)	/* ma coefficient memory */
#define MAMPM	IK(020401,0)	/* ma microprogram memory */
#define MAPA	IK(020402,0)	/* ma program address registers */
#define MAIOLIST IK(020402,1)	/* ma input/output list offset */
#define MALPCNT	IK(020402,2)	/* ma loop 0 counter */
#define MAZRANGE IK(020402,3)	/* ma zrange for clipping */
#define MARESULT IK(020403,0)	/* ma result registers */
#define MAXYREG	IK(020403,0)	/* ma x/y register */
#define MAZSHADE IK(020403,1)	/* ma z/shade register */
#define MAWREG	IK(020403,2)	/* ma w register */
#define MAPC	IK(020403,3)	/* pc readback register */
#define MASTART	IK(020402,7)	/* ma program start trigger */
#define MPC	IK(034000,0)	/* microprocessor controller */
#define MPCRAM	IK(034000,0)	/* microprocessor ram */
#define MPCIO	IK(034040,0)	/* microprocessor i/o space */
#define VI	IK(030100,0)	/* video input */
#define VIVPL	IK(030100,0)	/* vi viewport location (y,x) */
#define VIMF	IK(030100,1)	/* vi minification factor (y,x) */
#define VIWL	IK(030100,2)	/* vi window location (y,x) */
#define VIVPS	IK(030100,3)	/* vi viewport size (y,x) */
#define VICR	IK(030100,4)	/* vi control register */
#define VISFREQ	IK(030100,5)	/* vi sampling frequency */
#define CGM	IK(020100,0)	/* character generator module */
#define CGMFT	IK(020100,0)	/* cgm font table */
#define CGMS	IK(020200,0200)	/* cgm string */
#define CGMBCB	IK(020600,0)	/* cgm base control block */

#define IKMAXBYTES	32768	/* max bytes transferable in 1 read/write */

/*
	the following are site dependent ikonas timing values.
*/

#define PCRVALUE	39		/* pcr value for fbi 640 per line */
#define VIVPVALX	24		/* horizontal delay */
#define VIVPVALY	16		/* vertical delay */

/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * BRL Extentions, for programming registers
 *
 * Definitions relating to the Ikonas FBC (frame buffer controller).
 * This group of registers is write-only, and resides at
 * Ikonas address 30,000$0.
 *
 * Mike Muuss, BRL, 10/26/83. 
 */
struct ik_fbc  {
	long	fbc_xviewport:16, fbc_yviewport:16;
	long	fbc_xsizeview:16, fbc_ysizeview:16;
	long	fbc_xwindow:16, fbc_ywindow:16;
	long	fbc_xzoom:16, fbc_yzoom:16;
	long	fbc_horiztime:16, fbc_nlines:16;
	long	fbc_control;
	long	fbc_xcursor:16, fbc_ycursor:16;
};

/* Definitions for features in fbc_control */
#define FBC_CURSORON	(1<<2)		/* Set to turn cursor ON */
#define FBC_HIRES	(1<<3)		/* Set gives HIRES, else LORES */
#define FBC_AUTOCLEAR	(1<<5)		/* write only -- clear image mem */
#define FBC_EXTERNSYNC	(1<<6)		/* set=EXTERN, else horiztime+nlines */
#define FBC_CMAP2	(1<<7)		/* select 2nd color map */
#define FBC_RS343	(1<<9)		/* set for RS343, else RS170 */
#define FBC_NOINTERLACE	(1<<10)		/* set=repeat field, else interlace */
#define FBC_PIXELCLOCK(r) ((r)<<16)	/* nanoseconds/pixel, 16-128 */
					/* poorly documented Adage features
					    not available on all systems (see
					    Installation Guide pg. 4-7): */
#define FBC_EXTPIXELCLK	(1<<22)		/* external pixel clock, if set */
#define FBC_PROGSYN	(1<<23)		/* programmed horiz sync if set */
#define FBC_DRIVEBPCK	(1<<24)		/* drive bit pixel clock if 0,
					   default should be 1 */

/* Seemingly, some small omissions from the interface command list */
#define IKWRITE 020
#define IKREAD	000

#endif /* _SYS_IKIO_INCLUDED_ */
