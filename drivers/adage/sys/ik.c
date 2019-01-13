/*
 *				I K . C
 *
 *	device driver for IKONAS RDS-3000 digital frame buffer.
 *
 * Merged version.  Michael Landy -- NYU/Psychology -- 6/30/85
 *			("xp!msl"@nyu, ...!cmcl2!xp!msl)
 *
 *	This version, merges the feature of four Berkeley 4.2 drivers for the 
 * Adage/Ikonas.  These are:  Bell version (Ron Gordon, 1/15/84), University
 * of Waterloo version (Michael Herman, 3/11/85), BRL version (Mike Muuss,
 * 9/10/84), and Rockefeller University version (Kaare Christian, 4/3/84).
 * All of these are based on the BELL driver (4.1 version, 5/9/83).
 *	The idea is that if the users of the Adage don't diverge too severely
 * in our use of the Adage at the lowest levels, this will make it more
 * possible for us to share our discoveries and bug fixes.  I have tried to 
 * merge all of the features of all of the above drivers, yielding the 
 * following "family tree":
 *
 *                             Bell 4.1 version (5/9/83)
 *                                         |
 *                                         |
 *                                         |
 *                           |=========================|
 *                           |                         |
 *                           |                         |
 *           Bell 4.1C version - 6/20/83      BRL rev. 1.7 - 1/7/84
 *                       |                             |
 *                       |                             |
 *                       |                             |
 *                       |                 |=======================|
 *                       |                 |                       |
 *                       |                 |                       |
 *                       |     BRL rev. 1.14 - 9/11/84  Rockefeller - 4/3/84
 *                       |                 |                       |
 *                       |                 |                       |
 *         |==========================|    ==========|             |
 *         |                          |              |             |
 *         |                          |              |             |
 * Bell 4.2 version - 1/15/84         |              |             |
 *         |                          |              |             |
 *         |            Waterloo version - 3/11/85   |             |
 *         |                          |              |             |    
 *         |                          |              |             |    
 *         =========================================================    
 *                                      |
 *                                      |
 *                       Merged version - 6/30/85
 *
 * In performing this merge, I have tried to keep all of the features of all
 * of the drivers.  I have kept any minor bug fixes of all of the drivers, 
 * where this was possible.  I have kept all additions to the ioctl call (such
 * as IKIOGETADDR, IKTESTPROCESSOR, IKCLEARPROCESSOR, and IKSETSENDERID).  I
 * have incorporated the Waterloo version of the device timer (since every
 * driver had some version of a timeout procedure).  I have kept the separate
 * processor interrupt routine from Waterloo, which requires a slightly
 * different line in the configuration file.  I have kept the STYLE of open
 * from BRL and Rockefeller, merging it where necessary with Waterloo's
 * improvements of ikstrategy (splitting out ikstart) and ikintr (which 
 * handles the WINDOWDMA).  Note that /dev should therefore have three entries
 * for each adage:  minor device 0 (ik or ik0 - the standard style), minor
 * device 1 (ik0l - which assumes LORES pixel mode for all reads and writes to
 * image data addresses, and word mode for non-image data addresses), and minor
 * device 2 (ik0h - which assumes HIRES pixel mode for all reads and writes to
 * image data addresses, and word mode for non-image data addresses).  A second
 * adage becomes minor devices 8, 9, and 10, etc.
 *	There is at least one feature which I was not able to keep in the
 * merge - BRL's "unix-style" interface for the lores and hires style 
 * interfaces.  In that driver, the unix calls to seek on the adage are
 * effective, whereas in all other drivers, ioctl's to IKSETADDR are used.
 * In order to run software which ran under the BRL driver, any calls to 
 * seek will need to be changed to "ikseek" (in the BRL adage library), or
 * equivalently, to ioctl's.
 *	Finally, note that this is a 4.2 driver, and all references to 
 * intended compatibility to 4.1 have been removed.
 *
 *				Mike Landy - 6/30/85
 *
 * msl - 8/12/85 - allow windowdma in word mode (for 1-bit per pixel images).
 * 	Note that this only is intended for display in HIRES.  If LORES is
 * 	intended, a WINDOWDMA mode will be needed since the next address
 *	calculation in ikintr would then be wrong.  Also, it is now coded
 *	for DR/GM256's, overflows on the DR/GM64 are not handled properly.
 *
 * What follows are the merged comments from all four drivers.
 *
 *	device ik0 at uba? csr 0172460 vector ikintr ikfintr ikpintr
 *
 *	Ronald D. Gordon, Bell Telephone Laboratories, Room 3D454,
 *	Murray Hill, New Jersey 07974.  (201) 582-4099
 *	Please forward suggestions to allegra!rdg
 *
 *	Copyright 1982 Bell Telephone Laboratories, Incorporated.
 *	Bell Laboratories makes no warranties, express or implied,
 *	with regard to this software and any accompanying materials.
 *	In particular, but without limitation, Bell Laboratories makes
 *	no warranty of merchantability, fitness for a particular use,
 *	freedom from infringement of any patent, copyright or trademark,
 *	nor any warranty as to accuracy.  Accordingly, Bell Laboratories
 *	assumes no obligation to furnish any assistance of any kind
 *	whatsoever, or to furnish any additional information or documentation.
 *
 *	This software is being provided for experimental purposes only.
 *	Should it become available under a standard licensing arrangement,
 *	its continued use will be contingent upon the negotiation of a 
 *	licensing arrangement between you and the Western Electric Company.
 *
 *	Error codes returned by the driver:
 *		EIO	(open/read/write/ioctl) device failed to respond after 
 *			several seconds or error bit set.  Also returned by
 *                        IKTESTPROCESSOR when internal errors are detected.
 *		EFAULT	(read/write) invalid unibus address,
 *			or (ioctl) unable to address argp.
 *		ENXIO	(open) no device present at boot time.
 *		EBUSY	(open) device already opened.
 *		EINVAL	(read/write) byte count not appropriate for i/o
 *			mode selected, or device address out of range,
 *			or buffer address not even, or byte count too large,
 *			(ioctl) improper request or bad ikonas address.
 *		EACCES	(read/write) device did not set READY within
 *			a reasonable amount of time, or (ioctl) field or
 *			processor interrupt did not occur within a reasonable
 *			amount of time.
 *
 *	Recent changes:
 *	10/20/82	Sender id is set using bits 14-12 during ioctl
 *			calls which set all control bits.
 *	10/26/82	IKSETADDR corrected by Dave Martindale.
 * 	10/26/82	Added sc_byteno field to hold byte select bits
 *			rather than storing them in sc_control by Dave
 *			Martindale.
 *	11/26/82	Added window dma option.
 *	12/10/82	BPS processor interrupt enabled only during
 *			IKWAITPROCESSOR ioctl call.
 *	12/10/82	BPS processor is not halted at open/close time.
 *	1/27/83		Corrected overflow of u.u_offset.
 *	1/27/83		NYIT: ikopen and ikclose no longer halt bps,
 *			Paul Heckbert and Mike Chou.
 *	2/8/83		NYIT: clean-ups for lint, RTA.
 *	3/16/83		ikopen no longer attempts to reset the ik11b 
 *			since this inadvertently clears the bps processor
 *			ready bit.  considered a hardware design error!
 *	4/28/83		if open fails we reset the ik11b then return an
 *			error indication to the user.  next open usually
 *			works.
 *	4/29/83		window dma setting is not cleared when the device
 *			is opened.  it is just left alone!
 *	5/9/83		IKGETCONTROL returns all readable bits from ikonas
 *			status/command register.
 ********* at this point, the BRL and ROCKEFELLER drivers diverged********
 *	6/18/83		conversion to 4.1c using #ifdef BERK4.2 option.
 *			This has not been tested!
 *	6/19/83		added maptouser ``feature''; it is dangerous!
 *			This feature has not been tested!
 *	6/20/83		ikprobe/ikopen made more robust about clearing
 *			the ik11b interface.
 ********* at this point, the BELL and WATERLOO drivers diverged********
 ********* Comments from the WATERLOO driver: **********
 *	1/21/84		Changed to run under 4.2BSD, really.  Now
 *			incompatible with 4.1.  Uses uio structure rather
 *			than U area for parameters to read/write,
 *			is set up for a multi-entry iovec (though it
 *			won't calculate the Ikonas addresses for each
 *			segment correctly), uses timeout() rather than
 *			the defunct tsleep() for timeouts.
 *				-DMM, U of Waterloo
 *	3/5/84		Changed to update the Ikonas address after each
 *			transfer, allowing writes of more than 128Kb to
 *			work properly.  Moved the work of restarting the
 *			interface for each line of a window DMA operation
 *			from ikstrategy into ikintr, eliminating the two
 *			context switches per line transferred.  There is
 *			now a separate interrupt routine for processor
 *			interrupts, necessitating changing the system's
 *			config file.  IKBLOCK changed to 2^17 bytes.
 *				-DMM, U of Waterloo
 *	7/11/84		Now retains only the state of the RUNPROCESSOR
 *			bit in sc_control from the previous use.
 *			Formerly, leaving the BYTE bit set would hang
 *			the driver.  -DMM, U of Waterloo
 *	Feb 6/85	Added DMA retries. Paul Breslin, Omnibus Computer
 *			Graphics.
 *			#define DR64 removed by PHB (noted by mwherman)
 *	85-02-13        - added IKTESTPROCESSOR and IKCLEARPROCESSOR
 *			  iotctl calls (small corresponding change to
 *			  IKSTARTPROCESSOR) and manifests to <sys/ikio.h>
 *			- fixed bug in undocumented IKSETADDR ioctl.
 *			  ikloaddr now masks out low 10 bits
 *			- #define DR64 replaced for use at CGL
 *			- changes marked with #ifdef/endif MWH850213
 *			- #define MWH850213 added to <sys/ikio.h>
 *			- Michael Herman, Computer Graphics Laboratory,
 *			  University of Waterloo
 *	85-03-11	- fixed IKTESTPROCESSOR ioctl
 *			- DMA retries only produces a console msg if
 *			  attempting the MAX_TRIES'th attempt
 *			- changes marked with #ifdef/endif MWH850311
 *			- #define MWH850311 added to <sys/ikio.h>
 *			- Michael Herman, Computer Graphics Laboratory,
 *			  University of Waterloo
 ********* Comments from the BELL driver: **********
 *	8/31/83		IKWAITPROCESSOR returns now very fast if processor
 *			is already done when we start to check.
 *	9/11/83		IKSETSENDERID implemented.  was left out of earlier
 *			versions by mistake.
 *	12/31/83	incorporated several changes to correct 4.1c
 *			implementation.  (provided by Liudvikas Bukys
 *			rochester!bukys:  noticed that GETF returns
 *			improperly in 4.1C; U_COUNT macro added;
 *			IKMAPTOUSER correction.)
 *	1/6/84		put in timeout code for 4.2 version, this is
 *			enabled by #define IKTIMER. the code will be
 *			cleaned up to remove all reference to tsleep().
 *			it is my belief that it will work on 4.1.
 *			(vax135!martin)
 *	1/9/84		WAITFIELD now only times out once, ie with flaky
 *			hardware you only have to wait a few seconds for
 *			a user error return from ioctl().
 *			Fixed buffer management, it now seems to keep the
 *			buffers under control. (martin levy)
 *	1/15/84		fixed ikreset for use with 4.1c and 4.2.
 *			(martin levy, vax135!martin)
 ********* Comments from the BRL driver: **********
 *	10/13/83 	Adapted for 4.2 BSD operation
 *			Mike Muuss (BRL) and Terry Slattery (USNA).
 *
 *	10/16/83	Added IKIOGETADDR and mapping of UNIBUS registers
 *			to user space, for fast pixel I/O.
 *
 *	10/20/83	Muuss, Gwyn.  Made sleep uninterruptable.  Important.
 *
 * $Revision: 1.14 $
 *
 * $Log:	ik.c,v $
 * Revision 1.14  84/09/11  00:26:43  mike
 * Improved error message, and added uprintf() on DMA error.
 * 
 * Revision 1.13  84/09/10  23:18:41  mike
 * Fixed bug where "vertical interval" bit in ikcomreg was
 * being written back as "sender ID" thanks to |= and oddball
 * register layout.
 * 
 * Revision 1.12  84/08/28  23:25:44  phil
 * Misc. Trash.
 * 
 * Revision 1.11  84/08/13  17:23:24  phil
 * Added 4.2 code to prevent "overflow" in read and write routines.
 * Fixed bug in uiomove parameters.
 * 
 * Revision 1.10  84/08/09  01:37:09  phil
 * Changed old iomove's in programmed I/O to 4.2 uiomove
 * 
 * Revision 1.9  84/08/02  17:15:12  phil
 * Fixed bug in PIO and redefined it.
 * 
 * Revision 1.8  84/04/04  07:19:04  mike
 * Fixed HIRES address check, cleared sc_senderid, etc, on open.
 * 
 * Revision 1.7  84/01/07  04:57:02  mike
 * Fixed standard address list.
 * 
 * Revision 1.6  84/01/06  13:44:17  mike
 * Working driver with both old (ioctl) and seek interfaces working.
 * 
 * Revision 1.5  84/01/06  10:01:33  mike
 * Add BRL style seek interface in addition to older ioctl interface.
 * 
 * Revision 1.4  83/12/02  06:10:33  mike
 * 
 * 
 * Revision 1.3  83/12/02  04:54:29  mike
 * Adjusted #includes for 4.2
 * 
 * Revision 1.2  83/12/02  03:01:10  mike
 * BRL device driver for Ikonas RDS-3000 display system
 ********* Comments from the ROCKEFELLER driver: **********
 *
 * Apr 3, 1984
 * Removed dependency on BRL's weird b_offset.
 * lseek no longer seeks on the ikonas, use ikseek in iklib.c
 * Modified to work with DR256 - 11 bit x addr instead of 10
 * Changed BRL csleep back to sleep
 * Kaare Christian, RU
 */

/*LINTLIBRARY*/

#define DEBUG850311

/* definitions used by the driver */
#define MAXWAIT	10	/* seconds to wait for processor done or field intr */
#define MAXIOWAIT 3	/* seconds to wait for io completion */
#define MAX_TRIES	4	/* number of tries before giving up on I/O */
#define MAXPIOWAIT 2500	/* max times to check ready for PIO completion */
#define TICK 2		/* resolution of iktimeout */
#define	TICKS(s) ((s)/TICK+1)	/* convert seconds to iktimeout ticks */
#define ISWORDMODE(sc)	(((sc->sc_control)&(HALFWORD|BYTE))==0)
#define	ISLORESSC(sc)	((sc->sc_style==STYLE_LORES) && (sc->sc_ikhiaddr < 020000))
#define	ISHIRESSC(sc)	((sc->sc_style==STYLE_HIRES) && (sc->sc_ikhiaddr < 020000))
#define	NOTPIXSC(sc)	((sc->sc_style==STYLE_STD) || (sc->sc_ikhiaddr >= 020000))
#define ISLORESFC(sc)	((((sc->sc_fc)&03)==2) || (ISLORESSC(sc)))
#define ISHIRESFC(sc)	((((sc->sc_fc)&03)==1) || (ISHIRESSC(sc)))
#define ISWRESFC(sc)	((((sc->sc_fc)&03)==0) && (NOTPIXSC(sc)))
#define ISPWINDOWDMA(sc) (((sc->sc_ikloaddr)==(sc->sc_left)) && !ISWRESFC(sc))
#ifdef DR64
#define	ISWWINDOWDMA(sc) (((sc->sc_ikloaddr & 037)==(sc->sc_left)) && \
	(sc->sc_style==STYLE_STD) && (((sc->sc_fc)&03)==0) && \
	(sc->sc_ikhiaddr<020000))
#else
#define	ISWWINDOWDMA(sc) (((sc->sc_ikloaddr & 037)==(sc->sc_left & 037)) && \
	(((sc->sc_ikhiaddr & 0200) >> 2) == (sc->sc_left & 040)) && \
	(sc->sc_style==STYLE_STD) && (((sc->sc_fc)&03)==0) && \
	(sc->sc_ikhiaddr<020000))
#endif DR64
#define	ISWINDOWDMA(sc)	((sc->sc_width>0) && \
	(ISPWINDOWDMA(sc) || ISWWINDOWDMA(sc)))
#define IKFC 017
#define IKCONTROL	(HALFWORD|INVISIBLEIO|RUNPROCESSOR|RESET|BYTE)

/*
 *	#define ENABLEPIO if you want to use PIO for short transfers.
 *	some (extended distance) interfaces for the ikonas don't
 *	work with PIO! (11/21/82)
 *	#define DR64 if you have DR64 memories, which have only
 *	10 bits of X addressibility. Note that this particular define does NOT
 *	appear here, but rather in ikio.h.
 *	you may also #define IKMAPTOUSER which will map the
 *	device registers into user address space.  Very risky, but for
 *	those who must have it, here it is!
 *	Comment from BRL:
 *	Re-enabled - PCD - 2 Aug 84.  It didn't work because of a program
 *	bug (now corrected)!  But PIO still does a busy wait (yuck!).
 */

#define IKBITPRINT		/* for nice %b output */
#define	IKENABLEPIO		/* for programmed I/O for 4 byte WORD xfers */
#define	IKMAPTOUSER		/* for IKIOGETADDR call */
#undef	IKDEBUG			/* for debugging output of all calls */
#define POLICE			/* for function code checking */


/* used with unibus status/command register */

#define GO		0000001
#define EXTBUSADDR	0000060
#define IOINTERENABLE	0000100
#define READY		0000200
#define PROCINTRENABLE	0001000
#define FLDINTRENABLE	0002000
#define IKUBRESET	0010000
#define NEX		0040000
#define ERROR		0100000

#ifdef IKBITPRINT
char	ubcom_bits[] = "\10\20ERR\17NEX\13FIE\12PIE\10RDY\7IIE\1GO";
#endif

/* used with ikonas status/command register */

#define FUNCTION	0000017
#define WRITE		0000020
#define READ		0000000
#define DMAENABLE	0000100
#define INCREMENT	0000400		/* always set in ikcomreg */

#include "ik.h"
#if NIK > 0
#include "../h/param.h"
#include "../h/buf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/file.h"
#include "../h/ioctl.h"
#include "../h/kernel.h"
#include "../h/uio.h"
#include "../h/ikio.h"
#include "../machine/pte.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"

#define IKPRI (PZERO-1)		/* make sleep uninterruptible (until timeout) */

struct ikdevice {
	short ubwcount;		/* unibus word count (two's complement) */
	short ubaddr;		/* unibus address register (must be even) */
	short ubcomreg;		/* unibus status & command register */
	short datareg;		/* data i/o register */
	short ikloaddr;		/* ikonas lower address register */
	short ikhiaddr;		/* ikonas upper address register */
	short ikcomreg;		/* ikonas status & command register */
};

int	ikprobe(), ikattach();
struct	uba_device *ikdinfo[NIK];
u_short ikstd[] = {0172460,0171000,0171200,0};
struct	uba_driver ikdriver =
    { ikprobe, 0, ikattach, 0, ikstd, "ik", ikdinfo };
struct a {	 	/* used to get file descriptor from read/write args */
	int fdes;
	off_t off;
	int sbase;
};

/*
 * The minor device code is now allocated as follows:
 *
 *		+---------------------+--------------+
 *		|  7   6   5   4   3  |  2   1   0   |
 *		+---------------------+--------------+
 *		    |				|
 *		    |				|
 *		    |__ Controller #		|__ Interface Operation Mode
 *
 * Several styles of operation are defined:
 *	STYLE_STD	ioctl + read/write style interface
 *	STYLE_LORES	seek + read/write, "unix style", LORES mode.
 *	STYLE_HIRES	seek + read/write, "unix style", HIRES mode.
 */
#define	IKUNIT(dev)	(minor(dev)>>3)
#define IKSTYLE(dev)	(minor(dev)&07)
#define STYLE_STD	0
#define STYLE_LORES	1
#define STYLE_HIRES	2
#define IKHIRES		(1<<0)	/* Hi-res transfer mode */
#define IKPIXEL		(1<<1)	/* Pixel transfer mode */

#define	IKADDR		((struct ikdevice *)(ikdinfo[IKUNIT(dev)]->ui_addr))

struct	ik_softc {
	struct buf *sc_bp;	/* buffer header */
	int	sc_width;	/* width of window dma */
	int	sc_length;	/* total length of this transfer */
	int	sc_count;	/* number of bytes to transfer in this DMA */
	int	sc_ubaddr;	/* current UBA addr for window DMA */
	int	sc_fdelay;	/* number of fields to delay */
	int	sc_ubinfo;
	short	sc_state;
#define	IKSC_OPEN	01	/* someone has the device open */
/* #define	IKSC_BUSY 02	i/o operation in progress, now subsumed
					under nonzero sc_bp */
	short	sc_fc;		/* 4 bit function code */
	short	sc_control;	/* ikonas interface control bits */
	short	sc_ikloaddr;	/* lo 10 address bits or 11 bit x address */
	short	sc_ikhiaddr;	/* hi 14 address bits or 12 bit y address */
	short	sc_senderid;	/* sender ID to use during writes */
	short	sc_byteno;	/* byte number to use during reads */
	short	sc_left;	/* left edge x address for window dma */
	short	sc_ikcom;	/* temp holder for ikcomreg, used so that
					ikintr knows the actual function code
					that was used (given STYLE and addr) */
	short	sc_timer;	/* timeout clock */
	short	sc_retries;	/* number of times this operation attempted */
#ifdef MWH850311
	short   sc_procint_enabled; /* bps processor interrupts enabled ? */
	short   sc_procint_waiting; /* user waiting in IKTESTPROCESSOR */
	short   sc_procint_count;   /* BPS interrupt counter */
#else /* original 850213 */
#ifdef MWH850213
	int     sc_procint;     /* bps has interrupted ? */
#endif MWH850213
#endif MWH850311
	int	sc_style;	/* remember style for use in ikintr and 
					iktimeout */
	char	sc_event;	/* event we are awaiting (for timeout) */
#define EV_IO	0		/* I/O completion */
#define	EV_FLD	1		/* field interrupt */
#define EV_PROC	2		/* processor interrupt */
	char	sc_wdma;	/* window DMA in progress */
} ik_softc[NIK];

char ik_timing;			/* timeout clock running */
struct buf rikbuf[NIK];

#ifdef POLICE
    int    ik_trapfc = IKFC+1;
#   define checkfc(fc) if ((fc) == ik_trapfc) psignal(u.u_procp, SIGIOT)
#endif POLICE

/*
 *	IKBLOCK describes the maximum number of bytes which may
 *	be transfered in one operation.  IKBLOCK is a hardware
 *	limitation of the Ikonas device.  it must be divisible by
 *	4 because of the funny word mode of the ikonas.
 *	the twos complement of IKBLOCK/2 must be representable in a
 *	16 bit register.
 */

#define IKBLOCK ( ((int)0200000)*2 )

/*
 *			I K P R O B E
 */
ikprobe(reg)
	caddr_t reg;
{
	register br, cvec;		/* value-result */
	register struct ikdevice *ikaddr = (struct ikdevice *)reg;

#ifdef lint
	br = 0; cvec = br; br = cvec;
#endif

#ifdef IKDEBUG
	printf("ikprobe(0%o)\n", reg);
#endif
	/* reset the interface */
	ikaddr->ubcomreg = 0;
	ikaddr->ubcomreg = IKUBRESET;
	ikaddr->ubcomreg = 0;

	/* reset programmable elements of the ikonas: bps and mpc */
	ikaddr->ikcomreg = RESET | RUNPROCESSOR;
	ikaddr->ubcomreg = 0;

	/* make it perform an I/O completion interrupt */

	ikaddr->ikcomreg = HALFWORD | READ | INCREMENT;
	ikaddr->ikhiaddr = ikaddr->ikloaddr = 0;
	ikaddr->ubcomreg = GO | IOINTERENABLE;
	DELAY(MAXPIOWAIT);
	ikaddr->ubcomreg = 0;
	return sizeof(struct ikdevice);
}

/*
 *			I K A T T A C H
 */
ikattach(ui)
	struct uba_device *ui;
{
#ifdef IKDEBUG
	printf("ikattach(0%o)\n",ui);
#endif
}

/*
 *			I K O P E N
 */
ikopen(dev,flag)
	dev_t dev;
	int flag;
{
	register struct ik_softc *sc = &ik_softc[IKUNIT(dev)];
	register struct uba_device *ui;
	register struct ikdevice *ikaddr = IKADDR;
	int iktimeout();
	int error_code;
	int pass;

#ifdef IKDEBUG
	printf("ikopen(0%o)\n",dev);
#endif
	if (IKUNIT(dev) >= NIK || (ui = ikdinfo[IKUNIT(dev)]) == 0 ||
	    ui->ui_alive == 0)
		return ENXIO;
	if (((sc = &ik_softc[IKUNIT(dev)]) -> sc_state) & IKSC_OPEN)
		return EBUSY;

	/* initialize softc structure */
	sc->sc_state = IKSC_OPEN;
	sc->sc_fc = 0;
	sc->sc_byteno = 0;
	sc->sc_senderid = 0;
	/* retain only the state of run-processor bit from previous use */
	sc->sc_control &= RUNPROCESSOR;

	/* Comment from BRL:  
	 * WE NO LONGER DO AN IKUBRESET AT IKOPEN TIME.  THAT WOULD CLEAR
 	 * THE BPS PROC READY BIT AND THERE IS NO WAY TO RESTORE THAT BIT'S
	 * STATE.  CONSIDERED TO BE A HARDWARE DESIGN ERROR!
	 */

	for (pass=1; ; pass++) {

		/* perform a programmed i/o read to verify that the device is ok */
		/* sc_control contains the state of the run-proc bit */
	
		ikaddr->ubcomreg = 0;	/* in case IKUBRESET was set earlier */
		ikaddr->ikcomreg = sc->sc_control | HALFWORD | READ | INCREMENT;
		ikaddr->ikhiaddr = ikaddr->ikloaddr = 0;
		ikaddr->ubcomreg = GO;	/* clears READY, should be set again soon */
		/* The Bell 4.2 driver sets IOINTRENABLE and waits for the
			device to interrupt, even though PIO still does a
			busy/wait.  I have left this as is, since I have
			eliminated the ikwait routine of 4.2. Someone should
			check to see how slow this really is!  msl - 6/30/85 */
	
		/*
		Check to see that the ready bit is set.
		If not, ikbwait returns an error.
		We try once to do a full device reset.
		However, this losses a lot of "state" info
		which cannot be recovered, thus this is done only as a
		last ditch effort to recover.
		*/
		if ((error_code = ikbwait(dev)) == 0) {
			if (pass > 1)
				uprintf("ik%d: reset\n", IKUNIT(dev));
			break;
		}
		if (pass > 1) {
#ifdef IKBITPRINT
			printf("ik%d: ik11b reset does not raise READY! ubcomreg=%b\n",
				minor(dev),(ikaddr->ubcomreg)&0xffff,ubcom_bits);
#endif
			sc->sc_state = 0;
			return error_code;
		}
		ikaddr->ubcomreg = 0;
		ikaddr->ubcomreg = IKUBRESET;	/* zap it */
		ikaddr->ubcomreg = 0;
		ikaddr->ikcomreg = RUNPROCESSOR | RESET;
		ikaddr->ikcomreg = 0;
	}

	sc->sc_timer = 0;
	sc->sc_left = sc->sc_width = 0;
#ifdef MWH850311
	sc->sc_procint_enabled = 0;
	sc->sc_procint_waiting = 0;
#endif MWH850311
	if (!ik_timing) {
		timeout(iktimeout, (caddr_t)0, hz*TICK);
		++ik_timing;
	}
#ifdef IKMAPTOUSER
	maptouser(ikaddr);
#endif
	return 0;
}

/*
 *			I K C L O S E
 */
ikclose(dev,flag)
	dev_t dev;
	int flag;
{
	register struct ik_softc *sc = &ik_softc[IKUNIT(dev)];
	register struct ikdevice *ikaddr = IKADDR;

#ifdef IKDEBUG
	printf("ikclose(0%o)\n",dev);
#endif
	/* the BELL driver had a check for sc->sc_buf.b_flags & B_BUSY,
		which printed a message and reset it.  My experience was that
		this hung the driver when it occurred, so I have left it out */
	sc->sc_state = 0;
#ifdef MWH850311
	sc->sc_procint_enabled = 0;
	ikaddr->ubcomreg = sc->sc_procint_enabled;
#else
	ikaddr->ubcomreg = 0;	/* make sure all intrpt enable bits are off */
#endif
#ifdef IKMAPTOUSER
	unmaptouser(ikaddr);
#endif
}

/*
 *			I K S T R A T E G Y
 */
ikstrategy(bp)
	register struct buf *bp;
{
	register dev_t	dev;
	register struct ik_softc   *sc;
	register struct uba_device *ui;

	dev = bp->b_dev;
	ui  = ikdinfo[IKUNIT(dev)];
	if (ui == 0 || ui->ui_alive == 0) {	/* suggested by bill reeves */
		bp->b_error = ENXIO;
		goto bad;
	}
	if (bp->b_bcount == 0)
		return;
	/*
	 * Check that the length is appropriate.
	 */
	sc = &ik_softc[IKUNIT(dev)];
	if ((ISWORDMODE(sc) && (bp->b_bcount&3)) /* wordmode requires %4 bytes */
	    || (bp->b_bcount&1)) {		/* others require %2 bytes */ 
		bp->b_error = EINVAL;
		goto bad;
	}
	if (bp->b_bcount > IKBLOCK) {
		bp->b_error = EINVAL;
		goto bad;
	}
	if ((sc->sc_ubinfo = ubasetup(ui->ui_ubanum, bp, UBA_NEEDBDP)) == 0) {
		bp->b_error = EFAULT;
		goto bad;
	}
	sc->sc_bp = bp;
	sc->sc_length = (int)(bp->b_bcount);	/* max count < int size */
	sc->sc_ubaddr = sc->sc_ubinfo&0x3ffff;
	sc->sc_count  = sc->sc_length;
	if( sc->sc_wdma  &&  sc->sc_count > sc->sc_width )
		sc->sc_count = sc->sc_width;
	sc->sc_retries = 0;

	sc->sc_style = IKSTYLE(bp->b_dev);	/* save style for ikstart,
							ikintr, and iktimeout */
#ifdef MSLDEBUG
	printf("ikstrategy bp=%x len=%d ubad=%x\ncnt=%d wdma=%d wid=%d\n",
		sc->sc_bp,sc->sc_length,sc->sc_ubaddr,sc->sc_count,
		sc->sc_wdma,sc->sc_width);
	printf("style=%d lo=%o hi=%o left=%d fc=%o\n",
		sc->sc_style,sc->sc_ikloaddr,sc->sc_ikhiaddr,sc->sc_left,
		sc->sc_fc);
#endif
	ikstart(sc, IKADDR, (int)(bp->b_flags & B_READ));
	return;

bad:
	bp->b_flags |= B_ERROR;
	iodone(bp);
}


/*
 *			I K S T A R T
 */
ikstart(sc, ikaddr, rw)
register struct ik_softc *sc;
register struct ikdevice *ikaddr;
int rw;		/* non-zero => read, 0 => write */
{
	short comreg;

	comreg = sc->sc_fc | DMAENABLE | INCREMENT | (sc->sc_control)
		| (rw ? (READ | sc->sc_byteno) : (WRITE | sc->sc_senderid));
	if (sc->sc_style != STYLE_STD) {
		comreg &= ~(IKHIRES|IKPIXEL);	/* assume word mode */
		if (sc->sc_ikhiaddr < 020000) {	/* in image date area */
			comreg |= IKPIXEL;	/* pixel mode */
			if (sc->sc_style == STYLE_HIRES)
				comreg |= IKHIRES; /* hires mode */
		}
	}
	ikaddr->ikcomreg =  sc->sc_ikcom = comreg; /* set and save for ikintr */
#ifdef MSLDEBUGINT
	printf("ikstart comreg=%o\n",comreg);
#endif

	ikaddr->ubwcount = (short)( - (sc->sc_count >> 1));
	ikaddr->ubaddr   = (short)sc->sc_ubaddr;
	ikaddr->ikloaddr = sc->sc_ikloaddr;
	ikaddr->ikhiaddr = sc->sc_ikhiaddr;

	sc->sc_event  = EV_IO;
	sc->sc_timer  = TICKS(MAXIOWAIT);

	/* Note that the Waterloo driver has eliminated the ikwait routine
		and implements the timeout in iktimeout, along with retry
		code, so no call to ikwait is needed here */

#ifdef MWH850311
	ikaddr->ubcomreg = (short)(((sc->sc_ubaddr>>12) & EXTBUSADDR)
			   | GO | IOINTERENABLE | sc->sc_procint_enabled);
#else /* original code */
	ikaddr->ubcomreg = (short)(((sc->sc_ubaddr>>12) & EXTBUSADDR)
			   | GO | IOINTERENABLE);
#endif
}

/*
 *			M I N I K P H
 *
 * SPECIAL NOTE:
 * the ikonas device has 3 r/w modes.
 * WORD - 4 unibus bytes = 1 ikonas word (4 byte words in ikonas)
 * HALFWORD - 2 unibus bytes = 1 ikonas word (2 phantom bytes provided)
 * BYTE - 1 unibus byte = 1 ikonas word (3 phantom bytes provided)
 *
 * because only an integer number of unibus words can be transfered,
 * read/write must be of an even number of bytes.  because word mode
 * won't interrupt until the entire ikonas word is filled, read/write
 * must be a multiple of 4 bytes if word mode is set.
 *
 * Berkeley UNIX will make multiple calls to the strategy routine if
 * necessary to handle a long I/O.  Thus the limit set by minikph
 * is really just controlling the maximum number of pages which
 * need be locked into memory for one call of the strategy routine
 * and the amount of UBA map space that is needed.
 */

	u_int
minikph(bp)
	struct buf *bp;
{
	register struct ik_softc *sc = &ik_softc[IKUNIT(bp->b_dev)];

	if (bp->b_bcount > IKBLOCK) {
		if (sc->sc_wdma)
			bp->b_bcount = (IKBLOCK / sc->sc_width) * sc->sc_width;
		else	bp->b_bcount = IKBLOCK;
	}
}

/*
 *			I K W R I T E
 */
ikwrite(dev, uio)
	dev_t dev;
	register struct uio *uio;
{
	register struct ik_softc *sc = &ik_softc[IKUNIT(dev)];
	register struct ikdevice *ikaddr = IKADDR;
	short value[2];
	int err;

#ifdef IKDEBUG
	printf("ikwrite(0%o)\n",dev);
#endif
	if (IKUNIT(dev) >= NIK)
		return ENXIO;
	/*
	 * Prevent eventual overflow of f_offset.
	 */
	u.u_ofile[((struct a *)u.u_ap)->fdes] -> f_offset = 0;
/*
 *	window dma transfers may be performed only if we are
 *	at the left edge and, if using a word function code, only if this
 *	operation is to image memory.
 *	window dma is enabled when sc->sc_width > 0.
 */
	sc->sc_wdma = ISWINDOWDMA(sc);
#ifdef IKENABLEPIO
	if ((ISWORDMODE(sc) && uio->uio_resid==4) && IKSTYLE(dev)==STYLE_STD
		&& sc->sc_wdma==0) {
			/* then use programmed I/O rather than DMA */
		if (err = uiomove((char *)value,4,UIO_WRITE,uio))
			return err;
		ikaddr->ikcomreg = sc->sc_control | sc->sc_fc | sc->sc_senderid
					| WRITE | INCREMENT;
		ikaddr->ikloaddr = sc->sc_ikloaddr;
		ikaddr->ikhiaddr = sc->sc_ikhiaddr;
#ifdef MWH850311
		ikaddr->ubcomreg = GO | sc->sc_procint_enabled;
#else
		ikaddr->ubcomreg = GO;
#endif
		ikaddr->datareg = value[0];
		ikaddr->datareg = value[1];
		/* assume READY && !(NEX|ERROR) set */
		return 0;
	}
#endif
	/* else, use DMA transfer */
	return physio(ikstrategy, &rikbuf[IKUNIT(dev)], dev, B_WRITE, minikph, uio);
}

/*
 *			I K R E A D
 */
ikread(dev, uio)
	dev_t dev;
	register struct uio *uio;
{
	register struct ik_softc *sc = &ik_softc[IKUNIT(dev)];
	register struct ikdevice *ikaddr = IKADDR;
	short value[2];
	int error_code;
	int err;

#ifdef IKDEBUG
	printf("ikread(0%o)\n",dev);
#endif
	if (IKUNIT(dev) >= NIK)
		return ENXIO;
	/*
	 * This (rather dirty) piece of code prevents eventual
	 * overflow of the f_offset.  Note that the file descriptor
	 * number has already been checked by the time we reach here.
	 */
	u.u_ofile[((struct a *)u.u_ap)->fdes] -> f_offset = 0;
/*
 *	window dma transfers may be performed only if we are
 *	at the left edge and, if using a word function code, only if this
 *	operation is to image memory.
 *	window dma is enabled when sc->sc_width > 0.
 */
	sc->sc_wdma = ISWINDOWDMA(sc);
#ifdef IKENABLEPIO
	if ((ISWORDMODE(sc) && uio->uio_resid==4) && IKSTYLE(dev)==STYLE_STD
		&& sc->sc_wdma==0) {
			/* then use programmed I/O rather than DMA */
		ikaddr->ikcomreg = sc->sc_control | sc->sc_fc | sc->sc_byteno
					| READ | INCREMENT;
		ikaddr->ikloaddr = sc->sc_ikloaddr;
		ikaddr->ikhiaddr = sc->sc_ikhiaddr;
#ifdef MWH850311
		ikaddr->ubcomreg = GO | sc->sc_procint_enabled;
#else
		ikaddr->ubcomreg = GO;
#endif
		if ((error_code = ikbwait(dev)) != 0)
			return error_code;
		value[0] = ikaddr->datareg;
		value[1] = ikaddr->datareg;
		if (err = uiomove((char *)value,4,UIO_READ,uio))
			return err;
		return 0;
	}
#endif

	/* else, use DMA transfer */
	return physio(ikstrategy, &rikbuf[IKUNIT(dev)], dev, B_READ, minikph, uio);
}

/*
 *			I K I N T R
 *
 *	ikintr handles I/O completion interrupts.
 *
 *	Note that here, as well as in the later routines, the argument "dev"
 *	comes from the interrupt vector, and hence the minor device is the
 *	adage number, not the adage*8+style used in the software.  Thus, we
 *	get ik_softc[dev], not ik_softc[IKUNIT(dev)] as is used elsewhere.
 */
ikintr(dev)
	dev_t dev;
{
	register struct ik_softc *sc = &ik_softc[dev];
	register struct ikdevice *ikaddr = IKADDR;
	register struct buf *bp;
	int nwds;
	register i;

	if ((ikaddr->ubcomreg&READY) == 0) {
		printf("ik%d: spurious I/O interrupt\n", dev);
		return;
	}
	sc->sc_timer = 0;
	if ((ikaddr->ubcomreg) & (NEX | ERROR)) {	
#ifdef IKBITPRINT
#ifdef MWH850311
                if (sc->sc_retries+1 >= MAX_TRIES) {
		    printf("ik%d:error #%d, ubscr=%b\n",
		           dev, sc->sc_retries+1, 
		           ikaddr->ubcomreg&0xffff, ubcom_bits);
		}
#else /* original PHB code */
		printf("ik%d: error try = %d, ubscr=%b\n",
			minor(dev), sc->sc_retries, ikaddr->ubcomreg&0xffff,
			ubcom_bits);
#endif MWH850311
#endif
		if ((bp = sc->sc_bp) != (struct buf *)0) {
			if( ++(sc->sc_retries) < MAX_TRIES ) {
				ikstart(sc, ikaddr, (int)(bp->b_flags & B_READ));
				return;
			}
			ubarelse(ikdinfo[IKUNIT(bp->b_dev)]->ui_ubanum,
				&sc->sc_ubinfo);
			sc->sc_bp    = (struct buf *)0;
			bp->b_error  = EIO;
			bp->b_flags |= B_ERROR;
			iodone(bp);
		}
		return;
	}
	if ((bp = sc->sc_bp) == (struct buf *)0)   /* no I/O in progress */
		return;

	sc->sc_length -= sc->sc_count;
#ifdef MSLDEBUGINT
	printf("ikintr -%d-%d-",sc->sc_length,sc->sc_count);
#endif
	/*
	 * If doing window DMA, increment the Y address
	 * and continue if necessary.
	 */
	if (sc->sc_wdma) {
		if ((sc->sc_ikcom & 03) == 2)
			sc->sc_ikhiaddr += 2;
		else if ((sc->sc_ikcom & 03) == 3)
			sc->sc_ikhiaddr ++;
		else {		/* this only works for word mode-->hires */
			i = sc->sc_ikloaddr;
			i += 040;
			sc->sc_ikloaddr = i & 01777;
			if (i & 02000) {
				i = sc->sc_ikhiaddr;
				i &= 037;
				i++;
				sc->sc_ikhiaddr = (sc->sc_ikhiaddr & ~(037))
					| (i & 037);
				if (i & 040)
					sc->sc_ikhiaddr ^= 0100;
			}
		}
#ifdef MSLDEBUGINT
		printf("A-%o-%o-",sc->sc_ikcom,sc->sc_ikhiaddr);
#endif
		if (sc->sc_length > 0) {
#ifdef MSLDEBUGINT
		printf("B\n");
#endif
			sc->sc_ubaddr += sc->sc_count;
			sc->sc_count   = (sc->sc_length > sc->sc_width)
					 ? sc->sc_width : sc->sc_length;
			ikstart(sc, ikaddr, (int)(bp->b_flags & B_READ));
			return;
		}
	}
	else {
		/*
		 * Calculate the Ikonas address immediately following
		 * the end of this transfer.  The computation tries
		 * to handle all cases of word, halfword, and byte
		 * transfers and low/high/word resolution, but hasn't
		 * been tested for all of them. For word resolution, the
		 * address is simply incremented, which may not be the most
		 * sensible for word access to the image memory.
		 */
		if (sc->sc_control&BYTE)
			nwds = sc->sc_count;
		else if (sc->sc_control&HALFWORD)
			nwds = sc->sc_count / 2;
		else
			nwds = sc->sc_count / 4;
		i = sc->sc_ikloaddr;
		if ((sc->sc_ikcom & 03) == 2)
			i += 2*nwds;		/* lo res */
		else
			i += nwds;		/* hi res */
#ifdef DR64
		sc->sc_ikloaddr = i & 0x3ff;
		i >>= 10;
#else
		sc->sc_ikloaddr = i & 0x7ff;
		i >>= 11;
#endif DR64
		if (i != 0) {
			if ((sc->sc_ikcom & 03) == 2)
				sc->sc_ikhiaddr += 2 * i;	/* lo res */
			else
				sc->sc_ikhiaddr += i;		/* hi res */
		}
	}
#ifdef MSLDEBUGINT
		printf("C-%o-%o\n",sc->sc_ikhiaddr,sc->sc_ikloaddr);
#endif
	ubarelse(ikdinfo[IKUNIT(bp->b_dev)]->ui_ubanum, &sc->sc_ubinfo);
	bp->b_resid = 0;
	sc->sc_bp = (struct buf *)0;
	iodone(bp);
}

/*
 *			I K P I N T R
 *
 *	ikpintr handles processor interrupts.
 */
ikpintr(dev)
	dev_t dev;
{
	register struct ik_softc *sc = &ik_softc[dev];
	register struct ikdevice *ikaddr = IKADDR;

#ifdef MWH850311
	ikaddr->ubcomreg = 0;	/* this may act as some sort of guard */
	sc->sc_procint_count++;	/* bps has interrupted */
	ikaddr->ubcomreg = PROCINTRENABLE;
        /* assume: sc->sc_procint_enabled = PROCINTRENABLE; */
#else /* original MWH850213 code */
#ifdef MWH850213
	sc->sc_procint = 1;	/* bps has interrupted */
#endif MWH850213
#endif MWH850311

#ifdef DEBUG850311
	printf( "ik%d pi:c %d\n", dev, sc->sc_procint_count );
#endif DEBUG850311

        if (sc->sc_procint_waiting) {
            /* this is a critical section that is not being handled properly */
            sc->sc_procint_waiting = 0;
	    wakeup((caddr_t)sc);
	    }
}

/*
 *			I K F I N T R
 *
 *	ikfintr handles field interrupts.
 *	normally, field interrupts are turned off.
 *	they are enabled only for the IKWAITFIELD ioctl call.
 */
ikfintr(dev)
	dev_t dev;
{
	register struct ik_softc *sc = &ik_softc[dev];

	if (--(sc->sc_fdelay) > 0)
		return;
	wakeup((caddr_t)&(sc->sc_fdelay));
}

/*
 *			I K I O C T L
 */
ikioctl(dev, cmd, addr, flag)
	dev_t dev;
	register int cmd;
	register caddr_t addr;	/* 4.2 BSD: in kernel address of args */
	int flag;
{
	register m;
	register struct ik_softc *sc = &ik_softc[IKUNIT(dev)];
	struct ikdevice *ikaddr = IKADDR;
	struct ikfcaddr ikfcaddr;
	int s;

#ifdef IKDEBUG
	printf("ikioctl(0%o,0%o,0%o,0%o)\n",dev,cmd,addr,flag);
#endif
	switch (cmd) {
	case IKSETXY:
		/* for speed, no error checks are performed */
		m = *(int *)addr;
		sc->sc_ikloaddr = (short)m;		/* 10 or 11 bit X */
		sc->sc_ikhiaddr = (short)(m>>16);	/* 10 or 11 bit Y */
		return 0;
	case IKWINDOWDMA:
		m = *(int *)addr;
#ifdef MSLDEBUG
		printf("windowdma(%x)\n",m);
#endif
		if ((sc->sc_left = (short)(m>>16)) > 03777) {
			sc->sc_left = sc->sc_width = 0;
			return EINVAL;
		}
#ifdef MSLDEBUG
		printf("val1\n");
#endif
		if ((sc->sc_width = (m & 0xffff)) >
#ifdef DR64
			1024*4) {	/* allow for HIRES windowdma */
#else
			2048*4) {	/* allow for HIRES windowdma */
#endif
			sc->sc_left = sc->sc_width = 0;
			return EINVAL;
		}
#ifdef MSLDEBUG
		printf("val2\n");
#endif
		return 0;
	case IKSETADDR:
		m = *(int *)addr;
		if (m & ~0xffffff)
			return EINVAL;
		sc->sc_ikloaddr = (short) (m & 0x3ff);
		sc->sc_ikhiaddr = (short)(m>>10);
		return 0;
	case IKSETFC:
		sc->sc_fc = (*(int *)addr) & IKFC;  /* 4 bit function code */
#		ifdef POLICE
		    checkfc(sc->sc_fc);
#		endif POLICE
		return 0;
	case IKSETFCADDR:
		ikfcaddr = *((struct ikfcaddr *)addr);
		sc->sc_ikloaddr = ikfcaddr.lo;
		sc->sc_ikhiaddr = ikfcaddr.hi;
		sc->sc_fc = ikfcaddr.fc & IKFC;
#		ifdef POLICE
		    checkfc(sc->sc_fc);
#		endif POLICE
		return 0;
	case IKSETCADDR:
		ikfcaddr = *((struct ikfcaddr *)addr);
		sc->sc_ikloaddr = ikfcaddr.lo;
		sc->sc_ikhiaddr = ikfcaddr.hi;
		sc->sc_fc = ikfcaddr.fc & IKFC;
#		ifdef POLICE
		    checkfc(sc->sc_fc);
#		endif POLICE
		sc->sc_control = ikfcaddr.fc & IKCONTROL;
		sc->sc_byteno = ikfcaddr.fc & BYTEFIELD;
		sc->sc_senderid = ikfcaddr.fc & SENDERFIELD;
		ikaddr->ikcomreg = sc->sc_control | INCREMENT;
		return 0;
	case IKSETCONTROL:	/* NOTE: IKSETCONTROL changes fc too! */
		m = *(int *)addr;
		sc->sc_fc = m & IKFC;
#		ifdef POLICE
		    checkfc(sc->sc_fc);
#		endif POLICE
		sc->sc_control = m & IKCONTROL;
		sc->sc_byteno = m & BYTEFIELD;
		sc->sc_senderid = m & SENDERFIELD;
		ikaddr->ikcomreg = sc->sc_control | INCREMENT;
		return 0;
	case IKSETSENDERID:
		sc->sc_senderid = *(int *) addr & SENDERFIELD;
		return 0;
	case IKRUNPROCESSOR:
		ikaddr->ikcomreg = RUNPROCESSOR | RESET | INCREMENT;
		sc->sc_control |= RUNPROCESSOR;
		sc->sc_control &= ~RESET;
		ikaddr->ikcomreg = sc->sc_control | INCREMENT;
#ifdef MWH850213
	case IKCLEARPROCESSOR:
		sc->sc_procint_count = 0; /* reset bps-has-interrupted flag */
#endif MWH850213
#ifdef MWH850311
		sc->sc_procint_enabled = PROCINTRENABLE;
		ikaddr->ubcomreg = sc->sc_procint_enabled;
#endif MWH850311
		return 0;
	case IKWAITFIELD:
		m = *(int *)addr;
		if (m < 1 || m > MAXWAIT*30)
			return EINVAL;
		/*
			we must wait 1 additional field time since the
			first is only a partial field delay.
		*/
		sc->sc_fdelay = m+1;
		sc->sc_event = EV_FLD;
		sc->sc_timer = TICKS(MAXWAIT);
		s = spl5();
#ifdef MWH850311
		ikaddr->ubcomreg = FLDINTRENABLE | sc->sc_procint_enabled;
#else
		ikaddr->ubcomreg = FLDINTRENABLE;
#endif
		/* delay for several secs */
		while (sc->sc_fdelay > 0) {
			sleep((caddr_t)&sc->sc_fdelay, IKPRI);
			if (sc->sc_timer == 0) {
				splx(s);
				sc->sc_fdelay = 0;
				printf("ik%d: field timeout\n",IKUNIT(dev));
				return EACCES;
			}
		}
		sc->sc_timer = 0;
		splx(s);
		return 0;
	case IKWAITPROCESSOR:
		sc->sc_event = EV_PROC;
		sc->sc_timer = TICKS(MAXWAIT);
		s = spl5();
#ifdef MWH850311
		sc->sc_procint_enabled = PROCINTRENABLE;
		ikaddr->ubcomreg = sc->sc_procint_enabled;
#else
		ikaddr->ubcomreg = PROCINTRENABLE;
#endif
		for(;;) {
			if ((ikaddr->ikcomreg) & PROCINT)
				break;
			/* delay for several secs */
			sleep((caddr_t)sc, IKPRI);
			if (sc->sc_timer == 0) {
				splx(s);
#ifdef MWH850311
				/* do nothing */
#else
				ikaddr->ubcomreg = 0;
#endif
				return EACCES;
			}
		}
		sc->sc_timer = 0;
		splx(s);
#ifdef MWH850311
		/* do nothing */
#else
		ikaddr->ubcomreg = 0;
#endif
		return 0;
#ifdef MWH850311
	case IKTESTPROCESSOR:
		ikfcaddr = *((struct ikfcaddr *)addr);
		/*
		 * if (ikfcaddr.hi && processor hasn't interrupted)
		 *     - wait for bps interrupt as in IKWAITPROCESSOR.
		 *     - return sc_procint_count or error.  
		 *	 if sc_procint_count is returned, it should be 1.
		 * else
		 *     - don't wait.
		 *     - just return sc_procint_count.
		 */
		if (ikfcaddr.hi && (sc->sc_procint_count == 0)) {  /* wait */
		    sc->sc_event = EV_PROC;
		    sc->sc_timer = TICKS(MAXWAIT);
		    s = spl5();
		    if (sc->sc_procint_enabled != PROCINTRENABLE) {
 		        /* 
                         * error - this should always be enabled
                         * between IKSTARTPROCESSOR and IKHALTPROCESSOR
                         * ioctl calls
                         */
#ifdef DEBUG850311
			printf( "ik%d TP:sc_procint_enabled not set\n",
				IKUNIT(dev) );
#endif DEBUG850311
                        return( EIO );
		    }
		    sc->sc_procint_waiting = 1;
		    for(;;) {
			    if ((ikaddr->ikcomreg) & PROCINT)
				    break;
			    /* delay for several secs */
			    sleep((caddr_t)sc, IKPRI);
			    if (sc->sc_timer == 0) {
				    splx(s);
				    return EACCES;
			    }
		    }
		    sc->sc_timer = 0;
		    splx(s);
		}

		ikfcaddr.lo = sc->sc_procint_count;
		sc->sc_procint_count -= ikfcaddr.lo;
#ifdef DEBUG850311
		printf( "ik%d TP:c,h,l %d %d %d\n",
                        IKUNIT(dev), sc->sc_procint_count, 
                        ikfcaddr.hi, ikfcaddr.lo );
#endif DEBUG850311
		*((struct ikfcaddr *)addr) = ikfcaddr;	/* copy out */
		return( 0 );
#else /* original MWH850213 changes */
#ifdef MWH850213
	case IKTESTPROCESSOR:
		m = *(int *)addr;
		/*
		 * if (m)
		 *     - wait for bps interrupt as in IKWAITPROCESSOR.
		 *     - return sc_procint or error.  if sc_procint is
		 *       returned, it should be 1.
		 * else
		 *     - don't wait.
		 *     - just return sc_procint.
		 */
		if (m) { 
		    sc->sc_event = EV_PROC;
		    sc->sc_timer = TICKS(MAXWAIT);
		    s = spl5();
		    ikaddr->ubcomreg = PROCINTRENABLE;
		    for(;;) {
			    if ((ikaddr->ikcomreg) & PROCINT)
				    break;
			    /* delay for several secs */
			    sleep((caddr_t)sc, IKPRI);
			    if (sc->sc_timer == 0) {
				    splx(s);
				    ikaddr->ubcomreg = 0;
				    return EACCES;
			    }
		    }
		    sc->sc_timer = 0;
		    splx(s);
		}
		return( sc->sc_procint );
#endif MWH850213
#endif MWH850311
	case IKHALTPROCESSOR:
		sc->sc_control &= ~RUNPROCESSOR;
		ikaddr->ikcomreg = sc->sc_control | INCREMENT;
#ifdef MWH850311
		sc->sc_procint_enabled = 0;
		ikaddr->ubcomreg = sc->sc_procint_enabled;
		sc->sc_procint_waiting = 0;
#endif MWH850311
		return 0;
	case IKGETCONTROL:
		/*
			to return current ikonas status/command register
			value to user, we return relavent driver-local values
			plus some readable bits directly from the device
			register itself.
		*/
		*((int *)addr) = sc->sc_fc | sc->sc_control | sc->sc_byteno
			| ((ikaddr->ikcomreg)
			       & (WRITE|DMAENABLE|FRAMEINT|PROCINT|INCREMENT));
		return 0;

#ifdef IKMAPTOUSER
	case IKIOGETADDR:	
		*(caddr_t *)addr = (caddr_t) ikaddr;
		return 0;
#endif

	default:
		return EINVAL;
	}
}

/*
 *			I K R E S E T
 */
ikreset(uban)
	int uban;
{
	register ik;
	register struct uba_device *ui;
	register struct ik_softc *sc = ik_softc;
	register struct ikdevice *ikaddr;

#ifdef IKDEBUG
	printf("ikreset(0%o)\n",uban);
#endif
	for (ik=0; ik<NIK; ik++, sc++) {
		if ((ui = ikdinfo[ik]) == 0 || ui->ui_alive == 0 ||
		    ui->ui_ubanum != uban || (sc->sc_state & IKSC_OPEN) == 0)
			continue;
		printf(" ik%d", ik);
		ikaddr = (struct ikdevice *)ui->ui_addr;
#ifdef MWH850311
                sc->sc_procint_enabled = 0;
		ikaddr->ubcomreg = sc->sc_procint_enabled;
#else
		ikaddr->ubcomreg = 0;
#endif
		if (sc->sc_ubinfo) {
			printf("<%d>", (sc->sc_ubinfo>>28)&0xf);
			sc->sc_ubinfo = 0;	/* 4.2, already deallocated */
		}
		if (sc->sc_bp) {
			sc->sc_bp->b_flags |= B_ERROR;
			iodone(sc->sc_bp);
			sc->sc_bp = (struct buf *)0;
		}
		/*** ikstart(sc->sc_bp->b_dev); ? ***/
	}
}


/*
 *			I K B W A I T
 */
ikbwait(dev)
	dev_t dev;
{
	register loop;
	register struct ikdevice *ikaddr = IKADDR;

	for (loop=0; loop < MAXPIOWAIT; loop++) {
		if ((ikaddr->ubcomreg) & READY) {
			if ((ikaddr->ubcomreg) & (ERROR|NEX)) {
#ifdef IKBITPRINT
				printf("ik%d: error ubscr=%b\n",
					minor(dev), ikaddr->ubcomreg&0xffff,
					ubcom_bits);
#endif
				return EIO;
			}
			else	return 0;
		}
	}
	printf("ik%d: pio timeout\n",IKUNIT(dev));
	return EACCES;
}

/*
 *			I K T I M E O U T
 *
 * Count down the timer for each active device.
 * If is has run out, wake up the sleeper.
 * If there are no active devices anymore,
 * quit timing altogether.
 */
iktimeout()
{
	register struct ik_softc *sc;
	register struct buf	 *bp;
	register dev_t		 dev;
	int inuse = 0;

	for (sc = &ik_softc[0]; sc < &ik_softc[NIK]; ++sc) {
		if (sc->sc_state == 0)
			continue;
		inuse = 1;
		if (sc->sc_timer == 0)
			continue;
		if (--sc->sc_timer == 0)
			switch (sc->sc_event) {
			case EV_IO:
#ifdef MWH850311
				if (sc->sc_retries+1 >= MAX_TRIES) {
				    printf("ik%d:DMA timeout #%d\n",
					sc - &ik_softc[0], sc->sc_retries+1);
				    }
#else /* original PHB code */
				printf("ik%d: DMA timeout, try = %d\n",
					sc - &ik_softc[0], sc->sc_retries);
#endif MWH850311
				/*
				 * sc_bp should always point somewhere
				 * at this point, but just in case...
				 */
				if ((bp = sc->sc_bp) == (struct buf *)0)
					break;
				dev = bp->b_dev;
				if( ++(sc->sc_retries) < MAX_TRIES ) {
					ikstart(sc, IKADDR, (int)(bp->b_flags & B_READ));
					break;
				}
				ubarelse(ikdinfo[IKUNIT(dev)]->ui_ubanum,
					&sc->sc_ubinfo);
				bp->b_error = EIO;
				bp->b_flags |= B_ERROR;
				iodone(bp);
				sc->sc_bp = (struct buf *)0;
				break;

			case EV_FLD:
				wakeup((caddr_t)&sc->sc_fdelay);
				break;

			case EV_PROC:
				wakeup((caddr_t)sc);
				break;

			default:
				printf("ik%d: bad event %d in iktimeout\n",
					sc - &ik_softc[0], sc->sc_event);
			}
	}
	if (inuse)
		timeout(iktimeout, (caddr_t)0, hz*TICK);
	else
		ik_timing = 0;
}

/*
 * Note that the routines maptouser/unmaptouser appear in uba.c in 4.2, and
 * hence need not be included here, as they were in most of the drivers
 */

#endif NIK
