/*
 * Copyright by Mike S Trachtman April 1985
 */

/*
 * Modified by Achim, Leah and Mike May 1986 onwards
 */

/*
 * Imaging ip-512 consisting of ap-512, fb-512, alu-512 & hf-512.
 * called ipfb, cuz ip and fb are names that are already in use
 *
 * Log of changes: started October 1985 October 1 1985 MST     - made it
 * work with Sun Unix 2.0. October 15 1985 MST - moved it to i/o
 * space of the multi-bus, made the FB registers precede the AP
 * registers, and changed the adress. October 17 1985 MST      - in
 * ioctl, added check to the ioxpan, ioypan, ioxlen and ioylen, to
 * make sure that it is in bounds. - making user specifiable
 * rectangle to work. - have offset wrap around mod the size of the
 * working square. October 20, 1985 MST - changed it so that all the
 * rectangle parameters are set - in one ioctl call. October 23, 1985
 * MST - added minimal support for multiple frame buffers. -
 * allocated space in the reg structure for the histogramer and alu
 * unit. - did the minimum needed to the alu unit to turn it on and
 * allow data to pass - thru it transparently. December 12, 1985 MST
 * - the set which lookup tables code, was wrong, - discovered by
 * Achim ( who supplied the correct - code) , and fixed.  Also minor
 * typos corrected January 30, 1986 MST & LM - fixed the open code,
 * to deal with more than - one frame buffer May    19, 1986 AA   -
 * add another ioctl case that allows the timed presentation of four
 * patterns.  intended for visual psychophysics. July 2, 1986  LM
 * - added that minor units are coded as follows: low three bits:
 * option bits high five bits: actual subunit so far the only option
 * bit specified is the low bit, which if turned on signifies to turn
 * on the camera and start grabbing frames continuously
 *
 * July 30, 1986 MST & LM- added option to enable talking to the hibyte
 * of a frame buffer September 15, 1986 LM - added : 1.configuration
 * information in the probe. 2. option to read/write 16 bits. 3.
 * option to open device not exclusively, using another subunit. uses
 * bit 1. October 16, 1986 LM   - some major organization changes.
 * added ioctls to read/write the contents of an entire register
 * structure of a board and the contents of the software structure
 * ipfb_device. ioctl numbers were changed, so programs using the
 * driver should be recompiled. programs running Achim staff should
 * be recompiled with -DPSYCHO. May 18, 1987 AA       - modified
 * parts for psychological experiments
 *
 */
#include "ipfb.h"
#if NIPFB > 0

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/file.h"
#include "../h/ioctl.h"
#include "../h/uio.h"

#include "../sun2/pte.h"
#include "../sundev/mbvar.h"

#define PSYCHO PSYCHO
#include "../sundev/ipfbreg.h"

#define IPFBUNIT(dev)          ((minor(dev)>>3) & 0x1f)
#define IPFBSUBUNIT(dev)       (minor(dev) & 0x07)
#define START_CAMERA           (0x01)
#define SHARE_DEVICE           (0x02)
#define PIXEL                  (0x70)  /* an arbitrary non-zero pixel
                                        * value */

#ifdef PSYCHO
#define WAIT_FOR_END_OF_VB() \
   while((reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_fbcsrlo & IPFB_CSRLVB) != 0)\
          DELAY(1)
#define WAIT_FOR_VB() \
   while((reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_fbcsrlo & IPFB_CSRLVB) == 0)\
          DELAY(1)
#endif PSYCHO

struct buf rfbbuf[NIPFB];

int ipfbprobe();

struct mb_device *ipfbdinfo[NIPFB];
struct mb_driver ipfbdriver = {ipfbprobe,0,0,0, 0, 0, sizeof(struct ipfb_reg),
"ipfb", ipfbdinfo, 0, 0, MDR_OBIO, 0};
struct ipfb_device ipfb_soft[NIPFB];


ipfbprobe(regpnt, unit)
   caddr_t regpnt;
   int unit;
{
   register struct ipfb_reg *reg;
   register int c;
   register int i, j;
   register struct ipfb_device *soft = &ipfb_soft[unit];

   reg = (struct ipfb_reg *) regpnt;

   /* first let us find out if we have an alu unit */
   if (peekc((char *) &reg->ipfb_alureg.ipfb_aluk1) != -1)
   {
      printf("alu ");
      soft->ipfb_flags = IPFB_ALU;
   }
   /* now find out which frame buffer units (fb512) are on the system */
   soft->ipfb_fbavail = 0;
   for (i = 0; i < 4; i++)
   {
      soft->ipfb_aluch(i) = 0x44;
      /* initially, no channel is associated with any frame buffer */
      if (peekc((char *) &reg->ipfb_fbreg[i].ipfb_fbdatalo) != -1)
      {         /* low is there; reset & clear FB. */
        printf("fb%d ", i);
        soft->ipfb_fbavail |= 0x01 << i;
        if (peekc((char *) &reg->ipfb_fbreg[i].ipfb_fbdatahi) != -1)
        {      /* high is there too. impossible to have high without
                * low. */
           printf("16-bits ");
           soft->ipfb_fbavail |= 0x10 << i;
        }
        reg->ipfb_fbreg[i].ipfb_xu.ipfb_fbx = 0;
        reg->ipfb_fbreg[i].ipfb_yu.ipfb_fby = 0;
        reg->ipfb_fbreg[i].ipfb_panu.ipfb_fbpan = 0;
        reg->ipfb_fbreg[i].ipfb_scrollu.ipfb_fbscroll = 0;
        reg->ipfb_fbreg[i].ipfb_csru.ipfb_fbcsr = 0;
        reg->ipfb_fbreg[i].ipfb_masku.ipfb_fbmask = 0;
        reg->ipfb_fbreg[i].ipfb_datau.ipfb_fbdata = 0;
        reg->ipfb_fbreg[i].ipfb_fbcsrhi = IPFB_CSRHCCLR >> 8;
        while ((reg->ipfb_fbreg[i].ipfb_csru.ipfb_fbcsr & IPFB_CSRHCS)
               != IPFB_CSRHCNOP)
           DELAY(1);
      }
   }
   if (!soft->ipfb_fbavail)
   {
      printf("ipfb%d: device does not exist\n", unit);
      return 0;
   }
   if (soft->ipfb_flags & IPFB_ALU)
      /* check on which alu channel each frame buffer is */
   {    /* program alu to transfer nonezero high and zero low */
      soft->ipfb_aluout = 0;
      reg->ipfb_alureg.ipfb_aluk1 = PIXEL;     /* a non-zero pixel */
      reg->ipfb_alureg.ipfb_aluk2 = PIXEL;     /* a non-zero pixel */
      reg->ipfb_alureg.ipfb_aluk3 = 0;
      reg->ipfb_alureg.ipfb_alucsr = 0x06;     /* logical AND. result:
                                                * high=PIXEL, low=0 */
      reg->ipfb_alureg.ipfb_shift = 0;
      reg->ipfb_alureg.ipfb_mult = 0;
      reg->ipfb_alureg.ipfb_input1 = 0x44;
      reg->ipfb_alureg.ipfb_input2 = 0x44;
      reg->ipfb_alureg.ipfb_input3 = 0;
      reg->ipfb_alureg.ipfb_output = 0;

      /*
       * in order to find out which frame buffer is connected to each
       * alu channel, if any, we set each channel, on it's turn, to
       * be fed by the high byte. only the frame buffer which is
       * connected to this channel will have , in it's data register,
       * the value PIXEL. all the others will have 0. The selection
       * is done by setting the appropriate bit in the output
       * register.
       */

      for (j = 0; j < 4; j++)
      {         /* set alu output register to transfer high byte through j */
        reg->ipfb_alureg.ipfb_output = 0x01 << j;
        DELAY(1);
        for (i = 0; i < 4; i++)
           /* search for data through all available fbs */
           if ((0x01 << i) & soft->ipfb_fbavail)
           {   /* set fb to grab one frame and wait till it is done */
              /*
               * printf("fb%d data before= %x\n",
               * i,(int)(reg->ipfb_fbreg[i].ipfb_datau.ipfb_fbdata));
               * */
              reg->ipfb_fbreg[i].ipfb_fbcsrhi = IPFB_CSRHCSF >> 8;
              while ((reg->ipfb_fbreg[i].ipfb_csru.ipfb_fbcsr &
                      IPFB_CSRHCS) != IPFB_CSRHCNOP)
                 DELAY(1);
              /*
               * printf("fb%d data after= %x\n",
               * i,(int)(reg->ipfb_fbreg[i].ipfb_datau.ipfb_fbdata));
               * */
              if (reg->ipfb_fbreg[i].ipfb_fbdatalo == PIXEL)
              {
                 soft->ipfb_aluch(i) =
                    (soft->ipfb_aluch(i) & 0xf0) | (char) j;
                 printf("fb%d low on alu%d. ", i, j);
                 break;
              }
              if ((0x10 << i) & soft->ipfb_fbavail)
                 if (reg->ipfb_fbreg[i].ipfb_fbdatahi == PIXEL)
                 {
                    soft->ipfb_aluch(i) =
                       (soft->ipfb_aluch(i) & 0x0f) | (((char) j) << 4);
                    soft->ipfb_aluout |=
                       reg->ipfb_alureg.ipfb_output & 0x0f;
                    printf("fb%d high on alu%d. ", i, j);
                    break;
                 }
           }
      }
      printf("\n");
   }
   return (sizeof(struct ipfb_reg));
}

ipfbopen(dev, flags)
   dev_t dev;
   int flags;
{
   register struct mb_device *md;
   register struct ipfb_reg *reg;
   register char fbunit = IPFBUNIT(dev);
   register char fbsubunit = IPFBSUBUNIT(dev);
   register struct ipfb_device *soft = &ipfb_soft[fbunit];
   register i;
   int j;
   static char which[] =
   {IPFB_APCSRII, IPFB_APCSRIG, IPFB_APCSRIB, IPFB_APCSRIR};

   if (fbunit >= NIPFB ||
       (soft->ipfb_open & IPFB_OPENFIELD) == IPFB_OPEN
        /* device is already open but without IPFB_SHR */ ||
       (!(fbsubunit & SHARE_DEVICE) && (soft->ipfb_open))
       || (md = ipfbdinfo[fbunit]) == 0 || md->md_alive == 0)
      return (ENXIO);

   if (!(soft->ipfb_open & IPFB_OPEN))
      soft->ipfb_open |= IPFB_OPEN;
   if (!(fbsubunit & SHARE_DEVICE))
   {    /* set device up only if it is not a shared device */

      reg = (struct ipfb_reg *) md->md_addr;

      /* set up soft registers */
      soft->ipfb_fbuse = MASTER_FB;    /* use mater fb */
      soft->ipfb_options = 0;  /* all special options are off */
      /* setup the rectangle being used, to be the whole screen */
      soft->ipfb_box.ipfb_pan = 0;
      soft->ipfb_box.ipfb_scroll = 0;
      soft->ipfb_box.ipfb_xlen = IPFB_SIZEX;
      soft->ipfb_box.ipfb_ylen = IPFB_SIZEY;

      /* set up fb */
      for (i = 0; i <= 3; i++)
      {         /* initialize all of the available frame buffers */
        if ((0x01 << i) & soft->ipfb_fbavail)
        {      /* set all registers, except data. */
           reg->ipfb_fbreg[i].ipfb_xu.ipfb_fbx = 0;
           reg->ipfb_fbreg[i].ipfb_yu.ipfb_fby = 0;
           reg->ipfb_fbreg[i].ipfb_panu.ipfb_fbpan = 0;
           reg->ipfb_fbreg[i].ipfb_scrollu.ipfb_fbscroll = 0;
           reg->ipfb_fbreg[i].ipfb_fbcsrlo = 0;
           reg->ipfb_fbreg[i].ipfb_masku.ipfb_fbmask = 0;
           /* no bit plane should be protected */
           if ((flags & FREAD) && (fbsubunit & START_CAMERA))
              /* read - acquire continuously */
              reg->ipfb_fbreg[i].ipfb_fbcsrhi =
                 (IPFB_CSRHXD | IPFB_CSRHXC | IPFB_CSRHRC | IPFB_CSRHWC |
                  IPFB_CSRHCCO) >> 8;
           else
              reg->ipfb_fbreg[i].ipfb_fbcsrhi =
                 (IPFB_CSRHXD | IPFB_CSRHXC | IPFB_CSRHRC | IPFB_CSRHWC) >> 8;
        }
      }

      /* set up ap */
      /* set up ap csr */
      reg->ipfb_apreg.ipfb_apcsr =
        IPFB_APCSRIS | IPFB_APCSRGL | IPFB_APCSRII;
      /* do all the luts */
      /* pick lut group 0 for all 4 lut groups */
      reg->ipfb_apreg.ipfb_lutsel =
        IPFB_LUTSELI0 | IPFB_LUTSELR0 | IPFB_LUTSELG0 | IPFB_LUTSELB0;
      /* load the input, green, blue and red luts */
      for (j = 0; j < 4; j++)
      {
        reg->ipfb_apreg.ipfb_apcsr =
           (reg->ipfb_apreg.ipfb_apcsr & ~IPFB_APCSRI) | which[j];
        /* first wait for vertical blanking */
        for (i = 256; i--;)
        {
           while ((reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_fbcsrlo &
                   IPFB_CSRLVB) == 0)
              DELAY(1);        /* just wait */
           reg->ipfb_apreg.ipfb_lutaddr = i;
           reg->ipfb_apreg.ipfb_lutdata = i;
        }
      }

      /* set up the alu unit, if it exists, as a passthru */
      if (soft->ipfb_flags & IPFB_ALU)
      {
        reg->ipfb_alureg.ipfb_aluk1 = 0;
        reg->ipfb_alureg.ipfb_aluk2 = 0;
        reg->ipfb_alureg.ipfb_aluk3 = 0;
        reg->ipfb_alureg.ipfb_alucsr = 0x05;
        reg->ipfb_alureg.ipfb_shift = 0;
        reg->ipfb_alureg.ipfb_mult = 0;
        reg->ipfb_alureg.ipfb_input1 = 0x44;
        reg->ipfb_alureg.ipfb_input2 = 0x44;
        reg->ipfb_alureg.ipfb_input3 =
           (soft->ipfb_aluch(soft->ipfb_fbuse)) & 0x0f;    /* display low */
        if (flags & FREAD)
           reg->ipfb_alureg.ipfb_output = 0;   /* grab low for all
                                                * channels */
        else
           reg->ipfb_alureg.ipfb_output = soft->ipfb_aluout;
      }
   } else
      soft->ipfb_open |= IPFB_SHR;

   return 0;
}

ipfbclose(dev, flags)
   dev_t dev;
   int flags;
{
   /* mark it unused */
   ipfb_soft[IPFBUNIT(dev)].ipfb_open = 0;
   return 0;
}

#define MAX_IPFB_BSIZE         (64*512)        /* 1/8 of an entire frame at a
                                        * time ?? */
ipfbminphys(bp)
   struct buf *bp;
{
   if (bp->b_bcount > MAX_IPFB_BSIZE)
      bp->b_bcount = MAX_IPFB_BSIZE;
}

ipfbstrategy(bp)
   register struct buf *bp;
{
   register count;     /* keep count of howmany bytes to do */
   register char *from, *to;
   register unsigned short *from16, *to16;
   register struct mb_device *md;
   register struct ipfb_reg *reg;
   register fbunit = IPFBUNIT(bp->b_dev);
   register struct ipfb_device *soft;

   md = ipfbdinfo[fbunit];
   reg = (struct ipfb_reg *) md->md_addr;
   soft = &ipfb_soft[fbunit];

   /* we don't need a start routine, as we are not interrupt driven */
   if (bp->b_flags & B_READ)
      if (soft->ipfb_options & IPFB_DATA16)
      {
        from16 = &(reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_fbdata16);
        to16 = (unsigned short *) bp->b_un.b_addr;
      } else if (soft->ipfb_options & IPFB_DATAHI)
      {
        from = &(reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_fbdatahi);
        to = bp->b_un.b_addr;
      } else   /* data low */
      {
        from = &(reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_fbdatalo);
        to = bp->b_un.b_addr;
      }
   else
    /* Writing */ if (soft->ipfb_options & IPFB_DATA16)
   {
      to16 = &(reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_fbdata16);
      from16 = (unsigned short *) bp->b_un.b_addr;
   } else if (soft->ipfb_options & IPFB_DATAHI)
   {
      to = &(reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_fbdatahi);
      from = bp->b_un.b_addr;
   } else/* data low */
   {
      to = &(reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_fbdatalo);
      from = bp->b_un.b_addr;
   }

   soft->ipfb_count = bp->b_bcount;
   if (soft->ipfb_options & IPFB_DATA16)
      soft->ipfb_count /= 2;
   while (soft->ipfb_count > 0)
   {
      /* put it at the right place */
      soft->ipfb_yu.ipfb_y =
        soft->ipfb_offset / soft->ipfb_box.ipfb_xlen +
        soft->ipfb_box.ipfb_scroll;
      soft->ipfb_xu.ipfb_x =
        soft->ipfb_offset % soft->ipfb_box.ipfb_xlen +
        soft->ipfb_box.ipfb_pan;

      /* load the x and y addresses, only during H or V blanking */
      /*
       * while
       * ((reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_csru.ipfb_fbcsr &
       * (IPFB_CSRLHB | IPFB_CSRLVB)) == 0) DELAY(1);
       */
      reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_fbyhi = soft->ipfb_yhi;
      reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_fbylo = soft->ipfb_ylo;
      reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_fbxhi = soft->ipfb_xhi;
      reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_fbxlo = soft->ipfb_xlo;
      count = min(soft->ipfb_count,
                 soft->ipfb_box.ipfb_xlen + soft->ipfb_box.ipfb_pan -
                 soft->ipfb_xu.ipfb_x);
      /* since we are being fed by phsio */
      soft->ipfb_offset = (soft->ipfb_offset + count) %
        (soft->ipfb_box.ipfb_xlen * soft->ipfb_box.ipfb_ylen);
      soft->ipfb_count -= count;

      /*
       * the loop 'while(count--) *to++=*from' should execute as fast
       * as possible thus this code is written in this long unraveled
       * form to facilitate this
       */
      if (bp->b_flags & B_READ)
        if (soft->ipfb_options & IPFB_DATA16)
           while (count--)
              /* the actual INPUT */
              *to16++ = *from16;
        else
           while (count--)
              *to++ = *from;
      else
       /* Writing */ if (soft->ipfb_options & IPFB_DATA16)
        while (count--)
           /* the actual OUTPUT */
           *to16 = *from16++;
      else
        while (count--)
           *to = *from++;
   }
   iodone(bp);
}

ipfbwrite(dev, uio)
   dev_t dev;
   struct uio *uio;
{
   if (IPFBUNIT(dev) >= NIPFB)
      return ENXIO;
   ipfb_soft[IPFBUNIT(dev)].ipfb_offset = uio->uio_offset;
   return physio(
        ipfbstrategy, &rfbbuf[IPFBUNIT(dev)], dev, B_WRITE, ipfbminphys, uio);
}

ipfbread(dev, uio)
   dev_t dev;
   struct uio *uio;
{
   if (IPFBUNIT(dev) >= NIPFB)
      return ENXIO;
   ipfb_soft[IPFBUNIT(dev)].ipfb_offset = uio->uio_offset;
   return physio(
        ipfbstrategy, &rfbbuf[IPFBUNIT(dev)], dev, B_READ, ipfbminphys, uio);
}

/* ARGSUSED */
ipfbioctl(dev, cmd, dataptr, flag)
   dev_t dev;
   register int cmd;
   caddr_t dataptr;    /* only 127 bytes are available */
   int flag;
{
   register struct ipfb_reg *reg;
   register struct ipfb_device *soft;
   register i;
   register char byte;
   register char *data;
   int t;/* a temp place for swabbing sake */
   int t0, t1, t2, t3, t4, t5, t6, t7; /* eight temp variables */

#ifdef PSYCHO
   int x[8];   /* eight more temp variables */

#endif PSYCHO

   int fbunit = IPFBUNIT(dev);
   struct mb_device *md;
   int lutset, lutwhich, offset;

   md = ipfbdinfo[fbunit];
   reg = (struct ipfb_reg *) md->md_addr;
   soft = &ipfb_soft[fbunit];
   data = (char *) dataptr;

   /*
    * setup lut tables this is not included in the switch, as there
    * are many cases to type
    */
   if ((cmd & 0xF000) == 0)
   {
      /* we have to temporarily remember which lut set we are using */
      lutset = reg->ipfb_apreg.ipfb_lutsel;
      lutwhich = reg->ipfb_apreg.ipfb_apcsr;
      /* now decide which 64 byte block we want (there are 4 of them) */
      offset = ((cmd & 0x0300) >> 8) * 64;
      /* now decide which color set we want */
      reg->ipfb_apreg.ipfb_lutsel = ((char) cmd) & 0xFF;
      reg->ipfb_apreg.ipfb_apcsr = (reg->ipfb_apreg.ipfb_apcsr &
                               ~IPFB_APCSRI) | ((cmd >> 4) & 0x00C0);
      /* wait for vertical blanking */
      while (
        (reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_fbcsrlo & IPFB_CSRLVB) == 0)
        DELAY(1);      /* just wait */

      /* do it */
      for (i = 0; i < 64; i++)
      {
        reg->ipfb_apreg.ipfb_lutaddr = i + offset;
        /* decide which way we are pumping */
        if (cmd & IOC_OUT)
        {
           *data++ = reg->ipfb_apreg.ipfb_lutdata;
        } else
        {
           reg->ipfb_apreg.ipfb_lutdata = *data++;
        }
      }
      /* restore things to the way they were */
      reg->ipfb_apreg.ipfb_lutsel = lutset;
      reg->ipfb_apreg.ipfb_apcsr = lutwhich;
   } else
      switch (cmd)
      {

      case IPFB_SLUTWHICH:
        /*
         * decide which look up table to use for each of the three
         * channels, and for input from the camera
         */
        /* wait for vertical blanking */
        while (
           (reg->ipfb_fbreg[MASTER_FB /* soft->ipfb_fbuse */ ].ipfb_fbcsrlo &
           IPFB_CSRLVB) == 0)
           DELAY(1);
        reg->ipfb_apreg.ipfb_lutsel = (char) *((int *) data);
        break;

      case IPFB_GLUTWHICH:
        *(int *) data = (int) reg->ipfb_apreg.ipfb_lutsel;
        break;

      case IPFB_SAPCSR:
        reg->ipfb_apreg.ipfb_apcsr = (char) *((int *) data);
        break;

      case IPFB_GAPCSR:
        *(int *) data = (int) reg->ipfb_apreg.ipfb_apcsr;
        break;

      case IPFB_SINPUT_LUT:
        /* which lut table to use for input to the CPU */
        reg->ipfb_apreg.ipfb_apcsr =
           (reg->ipfb_apreg.ipfb_apcsr & ~IPFB_APCSRI) &
            ((char) *((int *) data));
        break;

      case IPFB_GINPUT_LUT:
        *(int *) data = (int) (reg->ipfb_apreg.ipfb_apcsr & IPFB_APCSRI);
        break;

      case IPFB_SLEVEL:
        /* level control */
        reg->ipfb_apreg.ipfb_level = (char) *((int *) data);
        break;

      case IPFB_GLEVEL:
        *(int *) data = (int) reg->ipfb_apreg.ipfb_level;
        break;

      case IPFB_SGAIN:
        /* gain control */
        reg->ipfb_apreg.ipfb_gain = (char) *((int *) data);
        break;

      case IPFB_GGAIN:
        *(int *) data = (int) reg->ipfb_apreg.ipfb_gain;
        break;

      case IPFB_SAP:
        /* set AP structure. calling program should #define KERNEL */
        reg->ipfb_apreg = *((struct ipfb_apreg *) data);
        break;

      case IPFB_GAP:
        /* get AP structure. calling program should #define KERNEL */
        *((struct ipfb_apreg *) data) = reg->ipfb_apreg;
        break;

        /* now the fb stuff */

      case IPFB_SX:
        t = *(int *) data;
        reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_xu.ipfb_fbx = (short) t;
        break;

      case IPFB_GX:
        t = (short) reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_xu.ipfb_fbx;
        *(int *) data = (int) t;
        break;

      case IPFB_SY:
        t = *(int *) data;
        reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_yu.ipfb_fby = (short) t;
        break;

      case IPFB_GY:
        t = (short) reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_yu.ipfb_fby;
        *(int *) data = (int) t;
        break;

      case IPFB_SPAN:
        t = *(int *) data;
        reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_panu.ipfb_fbpan = (short) t;
        break;

      case IPFB_GPAN:
        t = (short) reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_panu.ipfb_fbpan;
        *(int *) data = (int) t;
        break;

      case IPFB_SSCROLL:
        t = *(int *) data;
        reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_scrollu.ipfb_fbscroll =
           (short) t;
        break;

      case IPFB_GSCROLL:
        t =
         (short) reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_scrollu.ipfb_fbscroll;
        *(int *) data = (int) t;
        break;

      case IPFB_SFBCSR:
        t = *(int *) data;
        reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_csru.ipfb_fbcsr = (short) t;
        /*
         * if it is a clear screen command, or get a single frame,
         * wait for it to terminate
         */
        if ((t & IPFB_CSRHCS) == IPFB_CSRHCCLR ||
            (t & IPFB_CSRHCS) == IPFB_CSRHCSF)
           /* wait for it to say, that nothing is going on */
           while ((reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_csru.ipfb_fbcsr &
                   IPFB_CSRHCS) != IPFB_CSRHCNOP)
              DELAY(1);
        break;

      case IPFB_GFBCSR:
        t = (short) reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_csru.ipfb_fbcsr;
        *(int *) data = (int) t;
        break;

      case IPFB_SMASK:
        t = *(int *) data;
        reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_masku.ipfb_fbmask = (short) t;
        break;

      case IPFB_GMASK:
        t = (short) reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_masku.ipfb_fbmask;
        *(int *) data = (int) t;
        break;

      case IPFB_SDATA:
        t = *(int *) data;
        reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_datau.ipfb_fbdata = (short) t;
        break;

      case IPFB_GDATA:
        t = (short) reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_datau.ipfb_fbdata;
        *(int *) data = (int) t;
        break;

      case IPFB_SFB0:
      case IPFB_SFB1:
      case IPFB_SFB2:
      case IPFB_SFB3:
        /* calling program should #define KERNEL */
        t = cmd & 0x00000003;  /* t= which fb to set */
        if (!((0x01 << t) & soft->ipfb_fbavail))
           return ENXIO;
        reg->ipfb_fbreg[t] = *((struct ipfb_fbreg *) data);
        break;

      case IPFB_GFB0:
      case IPFB_GFB1:
      case IPFB_GFB2:
      case IPFB_GFB3:
        /* calling program should #define KERNEL */
        t = cmd & 0x00000003;  /* t= which fb to set */
        if (!((0x01 << t) & soft->ipfb_fbavail))
           return ENXIO;
        *((struct ipfb_fbreg *) data) = reg->ipfb_fbreg[t];
        break;

      case IPFB_SALU:
        /* calling program should #define KERNEL */
        reg->ipfb_alureg = *((struct ipfb_alureg *) data);
        break;

      case IPFB_GALU:
        /* calling program should #define KERNEL */
        *((struct ipfb_alureg *) data) = reg->ipfb_alureg;
        break;

      case IPFB_SHF:
        /* calling program should #define KERNEL */
        /* the histogrammer */
        reg->ipfb_hfreg = *((struct ipfb_hfreg *) data);
        break;

      case IPFB_GHF:
        /* calling program should #define KERNEL */
        *((struct ipfb_hfreg *) data) = reg->ipfb_hfreg;
        break;

        /* modify software structure. */

      case IPFB_SFBUNIT:
        /* which of the four available frame buffers to use */
        t = *(int *) data;
        if (!((0x01 << t) & soft->ipfb_fbavail))
           return ENXIO;
        soft->ipfb_fbuse = t;
        soft->ipfb_options &= ~IPFB_DATA;
        /* clear previous data options */
        if (soft->ipfb_flags & IPFB_ALU)
           reg->ipfb_alureg.ipfb_input3 =
              (reg->ipfb_alureg.ipfb_input3 & 0xf0) |
              (soft->ipfb_aluch(t) & 0x0f);    /* display low */
        break;

      case IPFB_GFBUNIT:

        *(int *) data = (int) soft->ipfb_fbuse;
        break;

      case IPFB_GFBALUCH:

        *(int *) data = (int) soft->ipfb_aluchs;
        break;

      case IPFB_GFBAVAIL:

        *(int *) data = (int) soft->ipfb_fbavail;
        break;

        /* set pan scroll ylen and xlen */

      case IPFB_SIOBOX:
        /* check that the data is valid */
        t1 = ((struct ipfb_box *) data)->ipfb_pan;
        t2 = ((struct ipfb_box *) data)->ipfb_scroll;
        t3 = ((struct ipfb_box *) data)->ipfb_xlen;
        t4 = ((struct ipfb_box *) data)->ipfb_ylen;
        if (t1 < 0 || t2 < 0 || t3 <= 0 || t4 <= 0 ||
            t3 > IPFB_SIZEX || t4 > IPFB_SIZEY)
           return ENXIO;
        soft->ipfb_box.ipfb_pan = t1;
        soft->ipfb_box.ipfb_scroll = t2;
        soft->ipfb_box.ipfb_xlen = t3;
        soft->ipfb_box.ipfb_ylen = t4;
        break;

      case IPFB_GIOBOX:
        *((struct ipfb_box *) data) = soft->ipfb_box;
        break;

      case IPFB_SETOPT:
        byte = (char) *((int *) data);
        if (byte & IPFB_DATA)  /* HIGH or DATA16 are on */
           if (!((0x10 << soft->ipfb_fbuse) & soft->ipfb_fbavail))
              return ENXIO;
        if (soft->ipfb_flags & IPFB_ALU)
           if (byte & IPFB_DATAHI)     /* display HIGH */
              reg->ipfb_alureg.ipfb_input3 =
                 (reg->ipfb_alureg.ipfb_input3 & 0xf0) |
                 ((soft->ipfb_aluch(soft->ipfb_fbuse) & 0xf0) >> 4);
           else        /* display low */
              reg->ipfb_alureg.ipfb_input3 =
                 (reg->ipfb_alureg.ipfb_input3 & 0xf0) |
                 (soft->ipfb_aluch(soft->ipfb_fbuse) & 0x0f);
        soft->ipfb_options = byte;
        break;

      case IPFB_GETOPT:
        *(int *) data = (int) soft->ipfb_options;
        break;

      case IPFB_GETFLAGS:
        *(int *) data = (int) soft->ipfb_flags;
        break;

      case IPFB_SSOFT:
        /* calling program should #define KERNEL */
        *soft = *((struct ipfb_device *) data);
        break;

      case IPFB_GSOFT:
        /* calling program should #define KERNEL */
        *((struct ipfb_device *) data) = *soft;
        break;

#ifdef PSYCHO
      case IPFB_TRIAL:

        x[0] = x[1] = x[3] = x[6] = 0x0000;
        x[2] = x[4] = x[5] = x[7] = 0x0100;

        WAIT_FOR_END_OF_VB();
        WAIT_FOR_VB();
        WAIT_FOR_END_OF_VB();
        WAIT_FOR_VB();

        for (i = 0; i < 8; i += 2)
        {
           if (t0 = ((struct ipfb_trial *) data)->tme[i])
           {
              reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_panu.ipfb_fbpan = x[i];
              reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_scrollu.ipfb_fbscroll =
                 x[i + 1];
              reg->ipfb_apreg.ipfb_lutsel =
                 ((struct ipfb_trial *) data)->tbl[i];
              for (; t0 > 0; t0--)
              {
                 WAIT_FOR_END_OF_VB();
                 WAIT_FOR_VB();
              }
              if (t1 = ((struct ipfb_trial *) data)->tme[i + 1])
              {
                 reg->ipfb_apreg.ipfb_lutsel =
                    ((struct ipfb_trial *) data)->tbl[i + 1];
                 for (; t1 > 0; t1--)
                 {
                    WAIT_FOR_END_OF_VB();
                    WAIT_FOR_VB();
                 }
              }
           }
        }
        break;

      case IPFB_MOVIE:

        WAIT_FOR_END_OF_VB();
        WAIT_FOR_VB();
        WAIT_FOR_END_OF_VB();
        WAIT_FOR_VB();

        for (i = 0; i < 31; i++)
        {
           if ((t0 = ((struct ipfb_movie *) data)->pan[i]) == 0xFF
               || (t1 = ((struct ipfb_movie *) data)->srl[i]) == 0xFF)
              break;
           if (t2 = ((struct ipfb_movie *) data)->tme[i])
           {
              reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_panu.ipfb_fbpan
                 = (t0 < 0x80) ? t0 : t0 + 0x80;
              reg->ipfb_fbreg[soft->ipfb_fbuse].ipfb_scrollu.ipfb_fbscroll
                 = (t1 < 0x80) ? t1 : t1 + 0x80;
              reg->ipfb_apreg.ipfb_lutsel
                 = ((struct ipfb_movie *) data)->tbl[i];
              for (; t2; t2--)
              {
                 WAIT_FOR_END_OF_VB();
                 WAIT_FOR_VB();
              }
           }
        }
        break;


#endif PSYCHO

      default:
        return ENOTTY;
      }
   return 0;
}

/* ARGSUSED */
ipfbmmap(dev, off, prot)
   dev_t dev;
   off_t off;
   int prot;
{
   if (IPFBUNIT(dev) >= NIPFB || off)
      return -1;
   off = getkpgmap(ipfbdinfo[IPFBUNIT(dev)]->md_addr) & PG_PFNUM;
   return off;
}

#endif
