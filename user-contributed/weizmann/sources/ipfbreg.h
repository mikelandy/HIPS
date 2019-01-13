/*
 * ipfbreg.h M Trachtman - April 85
 *
 * modified for visual psychophysics by Achim.
 *
 * Registers and constants for the Imaging IP-512. Called ipfb rather
 * than ip, since there is already somthing called ip. Frame buffer
 * and image aquisition system. Includes support for the ALU-512
 * unit, HF-512 unit, AP-512 and multiple FB512s. The registers are
 * layed out, first the 4 FB512, followed by the AP512, ALU512 &
 * HF512, in this order. There is also a software structure to
 * support the driver. The user should try to use the constants as
 * defined here. If some new IOCTLs are needed, please consult with
 * Leah Mory . IOCTLs numbers are the 2 low bytes of the IOCTL
 * command. The IOCTLs numbers are chosen according to the following
 * guidelines: 1. The LUT commands are 0x----0---. 2. The AP commands
 * are 0x----1---. 3. The FB  commands are 0x----2---. 4. The ALU
 * commands are 0x----3---. 5. The HF  commands are 0x----4---. 6.
 * The SOFTware structure commands are 0x----5---. 7. The TRIAL
 * commands are 0x----7---. 8. When dealing with hardware registers,
 * the right most digit is the register offset. Thus, 0x00041003
 * hints that this is an AP command, which handles the third
 * register, which is the AP controll register. If the command
 * handles only a part of the register, the second and third nibbles
 * from the right are the 8-bit mask which defines the relevant
 * field. Thus, 0x00041C03 hints that this command touches only the
 * two left most bits of the AP control registers. The 2 high bytes
 * are set accordind to the system rules. For more details refer to
 * the system calls manual or type "man ioctl". Some valuable
 * information can be found in /usr/sys/h/ioctl.h . The IOCTLs which
 * handles an entire structure are recognize only if the calling
 * program is compiled with -DKERNEL option.
 *
 * To really understand this you must have the Imaging manual for the
 * hardware.
 *
 * Modified: October,16,1986 by Leah Mory.
 *
 * Part for psychophysics again modified by Achim, December 16, 87
 *
 */

#ifdef KERNEL
struct ipfb_fbreg
{
   /* fb-512 registers */
   union
   {
      char ipfb_fbxhl[2];
      u_short ipfb_fbx;        /* 01-00: hi-lo byte of x coordinate */
   }  ipfb_xu;
#define ipfb_fbxhi ipfb_xu.ipfb_fbxhl[0]       /* 01: hi byte of x
                                                * coordinate */
#define ipfb_fbxlo ipfb_xu.ipfb_fbxhl[1]       /* 00: lo byte of x
                                                * coordinate */

   union
   {
      char ipfb_fbyhl[2];
      u_short ipfb_fby;        /* 03-02: hi-lo byte of y coordinate */
   }  ipfb_yu;
#define ipfb_fbyhi ipfb_yu.ipfb_fbyhl[0]       /* 03: hi byte of y
                                                * coordinate */
#define ipfb_fbylo ipfb_yu.ipfb_fbyhl[1]       /* 02: lo byte of y
                                                * coordinate */

   union
   {
      char ipfb_fpanhl[2];
      u_short ipfb_fbpan;      /* 05-04: hi-lo byte of pan (x)
                                * coordinate */
   }  ipfb_panu;
#define ipfb_fbpanhi ipfb_panu.ipfb_fbpanhl[0] /* 05: hi byte of pan
                                                * coordinate */
#define ipfb_fbpanlo ipfb_panu.ipfb_fbpanhl[1] /* 04: lo byte of pan
                                                * coordinate */

   union
   {
      char ipfb_fbscrollhl[2];
      u_short ipfb_fbscroll;   /* 07-06: hi-lo byte of scroll (y)
                                * coordinate */
   }  ipfb_scrollu;
#define ipfb_fbscrollhi ipfb_scrollu.ipfb_fbscrollhl[0]
                                                /* 07: hi byte of scroll
                                                        * coordinate */
#define ipfb_fbscrolllo ipfb_scrollu.ipfb_fbscrollhl[1]
                                                /* 06: lo byte of scroll
                                                        * coordinate */

   union
   {
      char ipfb_fbcsrhl[2];
      u_short ipfb_fbcsr;      /* 09-08: hi-lo byte of csr */
   }  ipfb_csru;
#define ipfb_fbcsrhi ipfb_csru.ipfb_fbcsrhl[0] /* 09: hi byte of csr */
#define ipfb_fbcsrlo ipfb_csru.ipfb_fbcsrhl[1] /* 08: lo byte of csr */

#define FBPAD 2
   char ipfb_fb_padding[FBPAD];        /* 0B-0A unused */

   union
   {
      char ipfb_fbmaskhl[2];
      u_short ipfb_fbmask;     /* 0D-0C: hi-lo byte of memory plane
                                * protect mask */
   }  ipfb_masku;
#define ipfb_fbmaskhi ipfb_masku.ipfb_fbmaskhl[0]      /* 0D: hi byte of memory
                                                       * plane protect mask */
#define ipfb_fbmasklo ipfb_masku.ipfb_fbmaskhl[1]      /* 0C: lo byte of memory
                                                       * plane protect mask */
   union
   {
      char ipfb_fbdatahl[2];
      u_short ipfb_fbdata;     /* 0F-0E: hi-lo byte of data */
   }  ipfb_datau;
#define ipfb_fbdatahi ipfb_datau.ipfb_fbdatahl[0]     /* 0F: hi byte of data */
#define ipfb_fbdatalo ipfb_datau.ipfb_fbdatahl[1]     /* 0E: lo byte of data */
#define ipfb_fbdata16 ipfb_datau.ipfb_fbdata   /* two  bytes of data */
};

struct ipfb_apreg
{
   /* ap-512 registers */
   char ipfb_lutaddr;  /* 01: lut load address register */
   char ipfb_lutdata;  /* 00: lut load data register */
   char ipfb_apcsr;    /* 03: ap control register */
   char ipfb_lutsel;   /* 02: select lut group */
   char ipfb_gain;     /* 05: ap programmable gain register */
   char ipfb_level;    /* 04: programmable level register */
#define APPAD 2
   char ipfb_ap_pad[APPAD];    /* 06-0F: finish this block */
};

struct ipfb_alureg
{
   /* the ALU (Arithmetic Logic Unit) */
   char ipfb_aluk2;    /* 01: constant k2 */
   char ipfb_aluk1;    /* 00: constant k1 */
   char ipfb_alux;     /* 03: unused byte */
   char ipfb_aluk3;    /* 02: constant k3 */
#define ALUPAD 4
   char ipfb_aluxx[ALUPAD];    /* 04-07: unused bytes */
   char ipfb_shift;    /* 09: shift control */
   char ipfb_alucsr;   /* 08: alu control register */
   char ipfb_aluxxx;   /* 0B: unused */
   char ipfb_mult;     /* 0A: multiplier control */
   char ipfb_input2;   /* 0D: input control register two */
   char ipfb_input1;   /* 0C: input control register one */
   char ipfb_output;   /* 0F: output control register */
   char ipfb_input3;   /* 0E: input control register three */
};

struct ipfb_hfreg
{
   /* the histogrammer */
   short ipfb_hfcsr;   /* 00-01: control status register */
   short ipfb_hfstatus;        /* 02-03: status register */
   short ipfb_feature; /* 04-05: feature counter */
   short ipfb_memaddr; /* 06-07: memory address */
   short ipfb_memdata; /* 08-09: memory data */
#define  HFPAD 6
   char ipfb_hfx[HFPAD];       /* 0A-0F: unused */
};

struct ipfb_reg
{
   struct ipfb_fbreg ipfb_fbreg[4];    /* 0x00-0x3F: four copies of
                                        * the frame buffer */
   struct ipfb_apreg ipfb_apreg;       /* 0x40-0x47: the ap processor */
   char ipfb_pad2alu[8];       /* 0x48-0x4f: padding untill the alu */
   struct ipfb_alureg ipfb_alureg;     /* 0x50-0x5F: the alu processor */
   char ipfb_pad2hf[0x200 - 0x60];     /* padding until the
                                        * histogrammer */
   struct ipfb_hfreg ipfb_hfreg;       /* 0x200-0x20f: the
                                        * histogrammer */
};

#endif KERNEL

struct ipfb_box
{
   int ipfb_pan;       /* starting x coordinate of rectangle */
   int ipfb_scroll;    /* starting y coordinate of rectangle */
   int ipfb_xlen;      /* length in x dimension of the rectangle */
   int ipfb_ylen;      /* length in y dimension of the rectangle */
};

#ifdef KERNEL
struct ipfb_device
{
   char ipfb_open;     /* open flags */

#define IPFB_OPENFIELD  0x03   /* bit that indicate open situation */
#define IPFB_OPEN       0x01   /* device is already open */
#define IPFB_SHR        0x02   /* device is open as SHARED_DEVICE */

   char ipfb_options;  /* special options controlled from ioctls */

   char ipfb_fbavail;  /* which frame buffers (of 0,1,2,3) are
                        * available */
   char ipfb_aluout;   /* default contents for alu output register .
                        * set by ipfbprobe. */
   int ipfb_flags;     /* misc flags */

   int ipfb_fbuse;     /* which frame buffer (of 0,1,2,3) are we using */
   union
   {
      int ipfb_alus;
      char ipfb_aluc[4];       /* to which alu channel each available
                                * frame buffer is connected */
   }  ipfb_alucu;

#define ipfb_aluch(FB)   ipfb_alucu.ipfb_aluc[FB]
#define ipfb_aluchs      ipfb_alucu.ipfb_alus

   int ipfb_offset;    /* where we are in the device */
   long ipfb_count;    /* how many bytes we are transferring */
   /* the soft copy of the device registers */
   union
   {
      short ipfb_x;    /* where we are in the x coordinate */
      char ipfb_xc[2];
   }  ipfb_xu;

#define ipfb_xhi       ipfb_xu.ipfb_xc[0]
#define ipfb_xlo       ipfb_xu.ipfb_xc[1]

   union
   {
      short ipfb_y;    /* where we are in the y coordinate */
      char ipfb_yc[2];
   }  ipfb_yu;

#define ipfb_yhi       ipfb_yu.ipfb_yc[0]
#define ipfb_ylo       ipfb_yu.ipfb_yc[1]
   struct ipfb_box ipfb_box;

};

#define SIZEOFSOFT ((sizeof(struct ipfb_device) & 0x7F)<< 16 )

/* ioctls to read/write all the AP registers by one command. */

#define IPFB_SAP       IOC_IN   | 0x00081010   /* set all registers of
                                                * AP */
#define IPFB_GAP       IOC_OUT  | 0x00081010   /* get all registers of
                                                * AP */

/* ioctls to read/write all the FB registers by one command. */

#define IPFB_SFB0      IOC_IN   | 0x00102010   /* set all registers of
                                                * FB0 */
#define IPFB_GFB0      IOC_OUT  | 0x00102010   /* get all registers of
                                                * FB0 */
#define IPFB_SFB1      IOC_IN   | 0x00102011   /* set all registers of
                                                * FB1 */
#define IPFB_GFB1      IOC_OUT  | 0x00102011   /* get all registers of
                                                * FB1 */
#define IPFB_SFB2      IOC_IN   | 0x00102012   /* set all registers of
                                                * FB2 */
#define IPFB_GFB2      IOC_OUT  | 0x00102012   /* get all registers of
                                                * FB2 */
#define IPFB_SFB3      IOC_IN   | 0x00102013   /* set all registers of
                                                * FB3 */
#define IPFB_GFB3      IOC_OUT  | 0x00102013   /* get all registers of
                                                * FB3 */

/* ioctls to read/write all the ALU registers by one command. */

#define IPFB_SALU      IOC_IN   | 0x00103010   /* set all registers of
                                                * ALU */
#define IPFB_GALU      IOC_OUT  | 0x00103010   /* get all registers of
                                                * ALU */

/* ioctls to read/write all the HF registers by one command. */

#define IPFB_SHF       IOC_IN  | 0x00104010    /* set all registers of
                                                * HF */
#define IPFB_GHF       IOC_OUT | 0x00104010    /* get all registers of
                                                * HF */

/* ioctls to read/write all the software registers by one command. */
/* set all software  registers */
#define IPFB_SSOFT     IOC_IN  | 0x00005010 | SIZEOFSOFT
/* get all software registers */
#define IPFB_GSOFT     IOC_OUT | 0x00005010 | SIZEOFSOFT
#endif KERNEL

/* ipfb_options flags */
#define IPFB_DATA      0x03    /* bits that handle data requirements */
#define IPFB_DATAHI    0x01    /* use hi byte instead of lo byte for
                                * I/O */
#define IPFB_DATA16     0x02   /* use 16 bits in I/O */

/* ipfb_flags flags */
#define IPFB_ALU       0x01    /* does this system have an alu */

/* hardware constants */

#define MASTER_FB      0
/*
 * this is the frame size of a picture in full resolution, half of
 * this in each dimension for half resolution or non interlaced.
 */
#define IPFB_SIZEX     512     /* pixels/line */
#define IPFB_SIZEY     512     /* lines/frame */
#define ALU_DELAY       11     /* picture is right shifted by
                                * ALU_DELAY pixels after each ALU
                                * operation.  */


/* AP-512 */

/* lut data register - needs nothing */

/* lut address register - needs nothing */

/* lut (Look Up Table) select register */

#define IPFB_LUTSELI   (0x03)  /* the bits that are significant for
                                * input lut */
#define IPFB_LUTSELI0  (0x00)  /* select input lut 0 */
#define IPFB_LUTSELI1  (0x01)  /* select input lut 1 */
#define IPFB_LUTSELI2  (0x02)  /* select input lut 2 */
#define IPFB_LUTSELI3  (0x03)  /* select input lut 3 */

#define IPFB_LUTSELR   (0x03 <<2)      /* the bits that are
                                        * significant for red lut */
#define IPFB_LUTSELR0  (0x00 <<2)      /* select red lut 0 */
#define IPFB_LUTSELR1  (0x01 <<2)      /* select red lut 1 */
#define IPFB_LUTSELR2  (0x02 <<2)      /* select red lut 2 */
#define IPFB_LUTSELR3  (0x03 <<2)      /* select red lut 3 */

#define IPFB_LUTSELG   (0x03 <<4)      /* the bits that are
                                        * significant for green lut */
#define IPFB_LUTSELG0  (0x00 <<4)      /* select green lut 0 */
#define IPFB_LUTSELG1  (0x01 <<4)      /* select green lut 1 */
#define IPFB_LUTSELG2  (0x02 <<4)      /* select green lut 2 */
#define IPFB_LUTSELG3  (0x03 <<4)      /* select green lut 3 */

#define IPFB_LUTSELB   (0x03 << 6)     /* the bits that are
                                        * significant for blue lut */
#define IPFB_LUTSELB0  (0x00 <<6)      /* select blue lut 0 */
#define IPFB_LUTSELB1  (0x01 <<6)      /* select blue lut 1 */
#define IPFB_LUTSELB2  (0x02 <<6)      /* select blue lut 2 */
#define IPFB_LUTSELB3  (0x03 <<6)      /* select blue lut 3 */

/* ap csr control register */
#define IPFB_APCSRHL   (0x01)  /* select high or low byte for display:
                                * low - 0, hi - 1 */
#define IPFB_APCSRVBE  (0x02)  /* video bus output: enable - 0,
                                * disable - 1 */
#define IPFB_APCSRVE   (0x08)  /* video output: enable - 0, disable -
                                * 1 (unused in this version of the
                                * board) */
#define IPFB_APCSRGL   (0x10)  /* Gen lock mode: PLL - 0, XTAL - 1  */
#define IPFB_APCSRIS   (0x20)  /* Video (camera) input select: channel
                                * 0 - 0, channel 1 - 1 */
/* which lut set to use */
#define IPFB_APCSRI    (0x03 << 6)     /* Input Lut set for CPU Access */
#define IPFB_APCSRII   (0x00 << 6)     /* Input Lut set for CPU
                                        * Access: input */
#define IPFB_APCSRIG   (0x01 << 6)     /* Input Lut set for CPU
                                        * Access: green */
#define IPFB_APCSRIR   (0x02 << 6)     /* Input Lut set for CPU
                                        * Access: red */
#define IPFB_APCSRIB   (0x03 << 6)     /* Input Lut set for CPU
                                        * Access: blue */

/* gain and level registers - nothing */

/* FB registers */

/* all the x-y register pairs - nothing */

/* fb csr - lo register */
#define IPFB_CSRLI     (0x01)  /* sync signal control: interlaced - 0,
                                * noninterlaced - 1 */
#define IPFB_CSRLSC    (0x02)  /* Memory adress sequencing control:
                                * Interlaced (480/512 lines) - 0,
                                * Noninterlaced (240/256 lines) - 1 */
#define IPFB_CSRLHR    (0x04)  /* Horizontal resolution control: 512
                                * pixels/line - 0, 256 pixels/line - 1 */
#define IPFB_CSRLPP    (0x08)  /* Pixel Protection Selection: Not
                                * active - 0, Active - 1 */
/* the next three are read registers only */
#define IPFB_CSRLHB    (0x10)  /* Horizontal Blanking: no - 0, yes - 1 */
#define IPFB_CSRLVB    (0x20)  /* Vertical Blanking: no - 0, yes - 1 */
#define IPFB_CSRLEO    (0x40)  /* Field Status: even field active - 0,
                                * odd field active - 1 */

/* fb csr - hi register */
#define IPFB_CSRHXD    (0x0100)        /* auto x step direction:
                                        * decrement - 0, increment - 1 */
#define IPFB_CSRHYD    (0x0200)        /* auto y step direction:
                                        * decrement - 0, increment - 1 */
#define IPFB_CSRHXC    (0x0400)        /* enable x step control:
                                        * disable - 0, enable - 1 */
#define IPFB_CSRHYC    (0x0800)        /* enable y step control:
                                        * disable - 0, enable - 1 */
#define IPFB_CSRHRC    (0x1000)        /* enable step after read
                                        * control: disable - 0, enable
                                        * - 1 */
#define IPFB_CSRHWC    (0x2000)        /* enable step after write
                                        * control: disable - 0, enable
                                        * - 1 */

#define IPFB_CSRHCS    (0x0300<<6)     /* bits that control frame
                                        * acquire */
#define IPFB_CSRHCNOP  (0x0000<<6)     /* No operation in progress */
#define IPFB_CSRHCCLR  (0x0100<<6)     /* clear screen */
#define IPFB_CSRHCSF   (0x0200<<6)     /* acquire single frame */
#define IPFB_CSRHCCO   (0x0300<<6)     /* acquire continuously */

/*
 * ioctl commands these may need explanation, I suggest you read the
 * manual for the device and/or read programs using it (the driver
 * for example)
 *
 * ioctls have to be coded in a special way, what a drag.
 */

/* first the AP stuff */

/*
 * Load the Luts, due to the way ioctl is built, only 64 (<= 127)
 * bytes at a time so we have to do the 256 bytes in four shots so
 * each set of four is the 4 sections
 */
#define IPFB_SLUTLI00  IOC_IN | 0x00400000     /* set Input Lut0 */
#define IPFB_SLUTLI01   IOC_IN | 0x00400100
#define IPFB_SLUTLI02  IOC_IN | 0x00400200
#define IPFB_SLUTLI03   IOC_IN | 0x00400300

#define IPFB_SLUTLI10  IOC_IN | 0x00400001     /* set Input Lut1 */
#define IPFB_SLUTLI11   IOC_IN | 0x00400101
#define IPFB_SLUTLI12  IOC_IN | 0x00400201
#define IPFB_SLUTLI13   IOC_IN | 0x00400301

#define IPFB_SLUTLI20  IOC_IN | 0x00400002     /* set Input Lut2 */
#define IPFB_SLUTLI21   IOC_IN | 0x00400102
#define IPFB_SLUTLI22  IOC_IN | 0x00400202
#define IPFB_SLUTLI23   IOC_IN | 0x00400302

#define IPFB_SLUTLI30  IOC_IN | 0x00400003     /* set Input Lut3 */
#define IPFB_SLUTLI31   IOC_IN | 0x00400103
#define IPFB_SLUTLI32  IOC_IN | 0x00400203
#define IPFB_SLUTLI33   IOC_IN | 0x00400303

#define IPFB_SLUTLG00  IOC_IN | 0x00400400     /* set Green Lut0 */
#define IPFB_SLUTLG01   IOC_IN | 0x00400500
#define IPFB_SLUTLG02  IOC_IN | 0x00400600
#define IPFB_SLUTLG03   IOC_IN | 0x00400700

#define IPFB_SLUTLG10  IOC_IN | 0x00400410     /* set Green Lut1 */
#define IPFB_SLUTLG11   IOC_IN | 0x00400510
#define IPFB_SLUTLG12  IOC_IN | 0x00400610
#define IPFB_SLUTLG13   IOC_IN | 0x00400710

#define IPFB_SLUTLG20  IOC_IN | 0x00400420     /* set Green Lut2 */
#define IPFB_SLUTLG21   IOC_IN | 0x00400520
#define IPFB_SLUTLG22  IOC_IN | 0x00400620
#define IPFB_SLUTLG23   IOC_IN | 0x00400720

#define IPFB_SLUTLG30  IOC_IN | 0x00400430     /* set Green Lut3 */
#define IPFB_SLUTLG31   IOC_IN | 0x00400530
#define IPFB_SLUTLG32  IOC_IN | 0x00400630
#define IPFB_SLUTLG33   IOC_IN | 0x00400730

#define IPFB_SLUTLR00  IOC_IN | 0x00400800     /* set Red Lut0 */
#define IPFB_SLUTLR01   IOC_IN | 0x00400900
#define IPFB_SLUTLR02  IOC_IN | 0x00400A00
#define IPFB_SLUTLR03   IOC_IN | 0x00400B00

#define IPFB_SLUTLR10  IOC_IN | 0x00400804     /* set Red Lut1 */
#define IPFB_SLUTLR11   IOC_IN | 0x00400904
#define IPFB_SLUTLR12  IOC_IN | 0x00400A04
#define IPFB_SLUTLR13   IOC_IN | 0x00400B04

#define IPFB_SLUTLR20  IOC_IN | 0x00400808     /* set Red Lut2 */
#define IPFB_SLUTLR21   IOC_IN | 0x00400908
#define IPFB_SLUTLR22  IOC_IN | 0x00400A08
#define IPFB_SLUTLR23   IOC_IN | 0x00400B08

#define IPFB_SLUTLR30  IOC_IN | 0x0040080C     /* set Red Lut3 */
#define IPFB_SLUTLR31   IOC_IN | 0x0040090C
#define IPFB_SLUTLR32  IOC_IN | 0x00400A0C
#define IPFB_SLUTLR33   IOC_IN | 0x00400B0C

#define IPFB_SLUTLB00  IOC_IN | 0x00400C00     /* set Blue Lut0 */
#define IPFB_SLUTLB01   IOC_IN | 0x00400D00
#define IPFB_SLUTLB02  IOC_IN | 0x00400E00
#define IPFB_SLUTLB03   IOC_IN | 0x00400F00

#define IPFB_SLUTLB10  IOC_IN | 0x00400C40     /* set Blue Lut1 */
#define IPFB_SLUTLB11   IOC_IN | 0x00400D40
#define IPFB_SLUTLB12  IOC_IN | 0x00400E40
#define IPFB_SLUTLB13   IOC_IN | 0x00400F40

#define IPFB_SLUTLB20  IOC_IN | 0x00400C80     /* set Blue Lut2 */
#define IPFB_SLUTLB21   IOC_IN | 0x00400D80
#define IPFB_SLUTLB22  IOC_IN | 0x00400E80
#define IPFB_SLUTLB23   IOC_IN | 0x00400F80

#define IPFB_SLUTLB30  IOC_IN | 0x00400CC0     /* set Blue Lut3 */
#define IPFB_SLUTLB31   IOC_IN | 0x00400DC0
#define IPFB_SLUTLB32  IOC_IN | 0x00400EC0
#define IPFB_SLUTLB33   IOC_IN | 0x00400FC0

/* These are to get the LUT Tables */

#define IPFB_GLUTLI00  IOC_OUT | 0x00400000    /* get Input Lut0 */
#define IPFB_GLUTLI01   IOC_OUT | 0x00400100
#define IPFB_GLUTLI02  IOC_OUT | 0x00400200
#define IPFB_GLUTLI03   IOC_OUT | 0x00400300

#define IPFB_GLUTLI10  IOC_OUT | 0x00400001    /* get Input Lut1 */
#define IPFB_GLUTLI11   IOC_OUT | 0x00400101
#define IPFB_GLUTLI12  IOC_OUT | 0x00400201
#define IPFB_GLUTLI13   IOC_OUT | 0x00400301

#define IPFB_GLUTLI20  IOC_OUT | 0x00400002    /* get Input Lut2 */
#define IPFB_GLUTLI21   IOC_OUT | 0x00400102
#define IPFB_GLUTLI22  IOC_OUT | 0x00400202
#define IPFB_GLUTLI23   IOC_OUT | 0x00400302

#define IPFB_GLUTLI30  IOC_OUT | 0x00400003    /* get Input Lut3 */
#define IPFB_GLUTLI31   IOC_OUT | 0x00400103
#define IPFB_GLUTLI32  IOC_OUT | 0x00400203
#define IPFB_GLUTLI33   IOC_OUT | 0x00400303

#define IPFB_GLUTLG00  IOC_OUT | 0x00400400    /* get Green Lut0 */
#define IPFB_GLUTLG01   IOC_OUT | 0x00400500
#define IPFB_GLUTLG02  IOC_OUT | 0x00400600
#define IPFB_GLUTLG03   IOC_OUT | 0x00400700

#define IPFB_GLUTLG10  IOC_OUT | 0x00400410    /* get Green Lut1 */
#define IPFB_GLUTLG11   IOC_OUT | 0x00400510
#define IPFB_GLUTLG12  IOC_OUT | 0x00400610
#define IPFB_GLUTLG13   IOC_OUT | 0x00400710

#define IPFB_GLUTLG20  IOC_OUT | 0x00400420    /* get Green Lut2 */
#define IPFB_GLUTLG21   IOC_OUT | 0x00400520
#define IPFB_GLUTLG22  IOC_OUT | 0x00400620
#define IPFB_GLUTLG23   IOC_OUT | 0x00400720

#define IPFB_GLUTLG30  IOC_OUT | 0x00400430    /* get Green Lut3 */
#define IPFB_GLUTLG31   IOC_OUT | 0x00400530
#define IPFB_GLUTLG32  IOC_OUT | 0x00400630
#define IPFB_GLUTLG33   IOC_OUT | 0x00400730

#define IPFB_GLUTLR00  IOC_OUT | 0x00400800    /* get Red Lut0 */
#define IPFB_GLUTLR01   IOC_OUT | 0x00400900
#define IPFB_GLUTLR02  IOC_OUT | 0x00400A00
#define IPFB_GLUTLR03   IOC_OUT | 0x00400B00

#define IPFB_GLUTLR10  IOC_OUT | 0x00400804    /* get Red Lut1 */
#define IPFB_GLUTLR11   IOC_OUT | 0x00400904
#define IPFB_GLUTLR12  IOC_OUT | 0x00400A04
#define IPFB_GLUTLR13   IOC_OUT | 0x00400B04

#define IPFB_GLUTLR20  IOC_OUT | 0x00400808    /* get Red Lut2 */
#define IPFB_GLUTLR21   IOC_OUT | 0x00400908
#define IPFB_GLUTLR22  IOC_OUT | 0x00400A08
#define IPFB_GLUTLR23   IOC_OUT | 0x00400B08

#define IPFB_GLUTLR30  IOC_OUT | 0x0040080C    /* get Red Lut3 */
#define IPFB_GLUTLR31   IOC_OUT | 0x0040090C
#define IPFB_GLUTLR32  IOC_OUT | 0x00400A0C
#define IPFB_GLUTLR33   IOC_OUT | 0x00400B0C

#define IPFB_GLUTLB00  IOC_OUT | 0x00400C00    /* get Blue Lut0 */
#define IPFB_GLUTLB01   IOC_OUT | 0x00400D00
#define IPFB_GLUTLB02  IOC_OUT | 0x00400E00
#define IPFB_GLUTLB03   IOC_OUT | 0x00400F00

#define IPFB_GLUTLB10  IOC_OUT | 0x00400C40    /* get Blue Lut1 */
#define IPFB_GLUTLB11   IOC_OUT | 0x00400D40
#define IPFB_GLUTLB12  IOC_OUT | 0x00400E40
#define IPFB_GLUTLB13   IOC_OUT | 0x00400F40

#define IPFB_GLUTLB20  IOC_OUT | 0x00400C80    /* get Blue Lut2 */
#define IPFB_GLUTLB21   IOC_OUT | 0x00400D80
#define IPFB_GLUTLB22  IOC_OUT | 0x00400E80
#define IPFB_GLUTLB23   IOC_OUT | 0x00400F80

#define IPFB_GLUTLB30  IOC_OUT | 0x00400CC0    /* get Blue Lut3 */
#define IPFB_GLUTLB31   IOC_OUT | 0x00400DC0
#define IPFB_GLUTLB32  IOC_OUT | 0x00400EC0
#define IPFB_GLUTLB33   IOC_OUT | 0x00400FC0

/*
 * Note that we have to allow at least 4 bytes of transfer, even when
 * we only use 1 or 2
 */

/* rest of ioctls handling AP registers */

#define IPFB_SLUTWHICH IOC_IN  | 0x00041002    /* set: eight bits are
                                                * 8-BBGGRRII-0 */
#define IPFB_GLUTWHICH IOC_OUT | 0x00041002    /* get: eight bits are
                                                * 8-BBGGRRII-0 */

#define IPFB_SAPCSR    IOC_IN  | 0x00041003    /* set: apcsr */
#define IPFB_GAPCSR    IOC_OUT | 0x00041003    /* get: apcsr */
#define IPFB_SINPUT_LUT IOC_IN  | 0x00041C03   /* set: LUT group to use
                                                * by the LUT ADDRESS &
                                                * DATA rgisters */
#define IPFB_GINPUT_LUT IOC_OUT | 0x00041C03   /* get: which LUT group
                                                * is used by the LUT
                                                * ADDRESS & DATA
                                                * rgisters */
#define IPFB_SLEVEL    IOC_IN  | 0x00041004    /* set level */
#define IPFB_GLEVEL    IOC_OUT | 0x00041004    /* get level */
#define IPFB_SGAIN     IOC_IN  | 0x00041005    /* set gain */
#define IPFB_GGAIN     IOC_OUT | 0x00041005    /* get gain */

/* FB registers stuff */

#define IPFB_SX                IOC_IN  | 0x00042000    /* set X location, where
                                                * current rectangle is
                                                * for the hardware */
#define IPFB_GX                IOC_OUT | 0x00042000    /* get X location, where
                                                * current rectangle is
                                                * for the hardware */
#define IPFB_SY                IOC_IN  | 0x00042002    /* set Y location, where
                                                * current rectangle is */
#define IPFB_GY                IOC_OUT | 0x00042002    /* get Y location, where
                                                * current rectangle is */
#define IPFB_SPAN      IOC_IN  | 0x00042004    /* set pan location,
                                                * where current
                                                * rectangle is for the
                                                * hardware */
#define IPFB_GPAN      IOC_OUT | 0x00042004    /* get pan location,
                                                * where current
                                                * rectangle is for the
                                                * hardware */
#define IPFB_SSCROLL   IOC_IN  | 0x00042006    /* set scroll location,
                                                * where current
                                                * rectangle is */
#define IPFB_GSCROLL   IOC_OUT | 0x00042006    /* get scroll location,
                                                * where current
                                                * rectangle is */
#define IPFB_SFBCSR    IOC_IN  | 0x00042008    /* set fb csr */
#define IPFB_GFBCSR    IOC_OUT | 0x00042008    /* get fb csr */
#define IPFB_SMASK     IOC_IN  | 0x0004200C    /* set pixel plane
                                                * protect mask */
#define IPFB_GMASK     IOC_OUT | 0x0004200C    /* get pixel plane
                                                * protect mask */
#define IPFB_SDATA     IOC_IN  | 0x0004200E    /* set contents of  data
                                                * register */
#define IPFB_GDATA     IOC_OUT | 0x0004200E    /* get contents of  data
                                                * register */

/*
 * now the stuff that is only in the driver, and has no (TRUE)
 * hardware counterpart
 */

/*
 * Choose which of the frame buffers in the set to use. The choice of
 * hibyte or lobyte can not be done via this choice but is done by
 * setting the appropiate bit, in the appropiate frame buffer
 */
#define IPFB_SFBUNIT   IOC_IN  | 0x00045004    /* set which frame
                                                * buffer to use */
#define IPFB_GFBUNIT   IOC_OUT | 0x00045004    /* get which frame
                                                * buffer we are using */
#define IPFB_GFBAVAIL  IOC_OUT | 0x00045005    /* get which frame
                                                * buffers are available */
#define IPFB_GFBALUCH  IOC_OUT | 0x00045006    /* get on which alu
                                                * channels frame
                                                * buffers are  */

/*
 * the next ioctl allows us to specify a rectangle where current
 * read/write should go to. One use is to make it very easy to draw
 * rectangles. It is also useful for going into zoom mode, you give
 * the coordinates of the upper left hand box { 0, 0, 256, 256} and
 * then turn on the appropiate zoom bit in fbcsr. It is also useful
 * in repositioning FB contents after an ALU operations; For that
 * use, just set ipfb_pan in ipfb_box to the shift happenned due to
 * ALU activities. It takes as an argument the ipfb_box structure.
 */

#define IPFB_SIOBOX    IOC_IN  | 0x00105007    /* set: rectangle for
                                                * i/o */
#define IPFB_GIOBOX    IOC_OUT | 0x00105007    /* get: rectangle for
                                                * i/o */


/* set/get special option bits - see description above */

#define IPFB_SETOPT    IOC_IN  | 0x00045008    /* set special options */
#define IPFB_GETOPT    IOC_OUT | 0x00045008    /* get special options */

/* get device flags - see description above */

#define IPFB_GETFLAGS  IOC_OUT | 0x00045009    /* get flags */


#ifdef PSYCHO
/*
 * additional ioctl to run a psychophysical trial consisting of the
 * timed presentation of four consecutive patterns and four
 * intermission periods.   achim.
 */

struct ipfb_trial
{
   char tbl[8];        /* eight lookuptable selections */
   int tme[8]; /* eight presentation times     */
};

#define        IPFB_TRIAL      IOC_IN | 0x00407000     /* run the trial */

/*
 * additional ioctl to run a psychophysical trial consisting of 31
 * consecutive presentations of one of four quadrants ( pan 0x00, srl
 * 0x00; pan 0x80, srl 0x00; pan 0x80, srl 0x80; pan 0x00, srl 0x80)
 * (NOTE: THESE NUMBERS ARE NOT A MISTAKE, THE DRIVER CHANGES 0x80 to
 * 0x100), through one of four lookuptables (0x00, 0x55, 0xAA, 0xFF),
 * for a number of frames, (0-255).  The sequence stops either after
 * 31 presentations, or when pan or scroll are found to equal 0xFF,
 * whichever comes first.
 */

struct ipfb_movie
{
   unsigned char pan[31];      /* thirty-one pan selections */
   unsigned char srl[31];      /* thirty-one scroll selections */
   unsigned char tbl[31];      /* thirty-one lookuptable selections */
   unsigned char tme[31];      /* thirty-one presentation times */
};

/*
 * to come up with the following number, add 0x7001 to 0x10000 times
 * the number of bytes contained in the structure ipfb_movie.  as of
 * now, ipfb_movie contains 4 times 31, or 0x7C, bytes, which results
 * in 0x7001 plus 0x007C00000 or 0x007C7001
 */

#define        IPFB_MOVIE      IOC_IN | 0x007C7001     /* run the movie */

#endif PSYCHO
