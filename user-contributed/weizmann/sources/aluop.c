
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#define KERNEL KERNEL
#include <sundev/ipfbreg.h>
#include "convolve.h"
extern struct ipfb_reg  *reg;
extern int  ipfb,source_fb, target_fb;


/* Performs a hardware convolution, using the imaging ALU-512 board .
 * ipfb is the device fd. source_fb is the number of the frame buffer where
 * the original image resides. target_fb is the number of a 16-bits frame
 * buffer, where the result of the convolution will be found. In our system,
 * there is only one device with an ALU-512 : /dev/ipfb0. You must open the
 * device and perform a mmap before calling to conv. You also must have the
 * filter ready. mask is a pointer to the filter structure: kernel points to
 * the filter array; dx and dy are it's dimensions. The source_fb is
 * panned and scrolled to the pan and scroll values before the convolution
 * begins. If leave is on, target_fb is not cleared before the convolution
 * takes place. Otherwise it is cleared to the value const. This helps us
 * forcing the result to be all non-negative. The result is a 16-bits signed
 * integer. In order to get the 8 most significant bits to the high byte of
 * the result, one has to find out how many free bits are on the left and
 * call the procedure shift. This is done after conv.
 * Overflow is not taken care of.
 *
 * Usually the high byte of the result is displayed on the monitor. If low is
 * on, the low byte is displayed.
*/

conv (mask,showlow,leave,const)
struct mask *mask;
int  showlow,leave,const;
{   char    source_ch,
           target_high,
            target_low,
            fb_aluch[4];
    char   *ker_ptr;
    int     i,
            j,
            error;
    extern  errno;

    fprintf (stderr, "convolution..\n");
    ioctl (ipfb, IPFB_GFBALUCH, fb_aluch);
    source_ch = (fb_aluch[source_fb] & 0x0f);
    target_high = (((fb_aluch[target_fb] & 0xf0) >> 4) & 0x0f);
    target_low = (fb_aluch[target_fb] & 0x0f);
    if ( (target_high > 3) || (target_low > 3))
    {
     fprintf (stderr,"***conv :No ALU channels for target FB %d\n",target_fb);
       close(ipfb);
       exit(1);
    }
    if (!leave) {
       WAITVB (target_fb);
       PAN (target_fb) = 0;
       SCROLL (target_fb) = 0;
       DATA (target_fb) = (short) const;
    /* clear target_fb according to the constant value  */
       CSR_HI (target_fb) = CSR_HI (target_fb) | (IPFB_CSRHCCLR >> 8);
    }
    WAIT (target_fb);

 /* program the ALU for convolutuon */
    ALU_K1 = 0;
    ALU_K3 = 0;
    ALU_CSR = 0x03; /* add result of mult to target fb */
    ALU_SHIFT = 0;
    ALU_MULT = 0x11; /* B Bus 2's complement. multiplier enabled;no adj*/
    ALU_IN1 = 0x40 | source_ch; /* A bus = source_fb,
                                  B bus = k2 (kernel element) */
    ALU_IN2 = fb_aluch[target_fb]; /* bus C = fb low, bus D = fb high */
    ALU_IN3 = showlow ? (char) (target_low) : (char) (target_high);
    ALU_OUT = 0x01 << target_high;

    /* position target to center of mask */
    WAITVB (target_fb);
    PAN (target_fb) += (mask -> dx / 2);
    SCROLL (target_fb) += (mask -> dy / 2);

    ker_ptr = mask -> kernel;
    for (i = 0; i < mask -> dy; i++) {
       for (j = 0; j < mask -> dx; j++) {
           ALU_K2 = *ker_ptr++;/* update the ALU-register K2 from the
                                  kernel */
           CSR_HI (target_fb) = CSR_HI (target_fb) | (IPFB_CSRHCSF >> 8);
                               /* single acquire */
           WAIT (target_fb);
           WAITVB (target_fb);
           PAN (target_fb) += 11 - 1;  /* fix 11 pixels delay of the ALU-512,
                                          then move the target image right
                                          for next kernel element*/
       }
       SCROLL (target_fb)--;   /* move the target down for next kernel row*/
       WAITVB (target_fb);
       PAN (target_fb) += mask -> dx; /*re-position target after end of row*/
    }
 /* after all the rows, move target up again*/
    SCROLL (target_fb) += mask -> dy;
    SCROLL (target_fb) -= (mask -> dy / 2);
    WAITVB (target_fb);
    PAN (target_fb) -= (mask -> dx / 2);
}

/********************************************************************/

shift(fb,scale)
int fb,scale;
{   char  fb_high, fb_low,fb_aluch[4];

    fprintf(stderr,"shift. scale= %d\n",scale);
    ioctl (ipfb, IPFB_GFBALUCH, fb_aluch);

    fb_high = (((fb_aluch[fb] & 0xf0) >> 4) & 0x0f);
    fb_low = (fb_aluch[fb] & 0x0f);
    if ( (fb_high > 3) || (fb_low > 3))
    {  fprintf (stderr,"***shift : No ALU channels for target FB %d\n",fb);
       close(ipfb);
       exit(1);
    }
    WAIT (target_fb);

    /* program the ALU for shift  */
    ALU_K1 = 0;
    ALU_K2 = 0;
    ALU_K3 = 0;
    ALU_CSR = 5;  /* or */
    ALU_SHIFT = ( 0x10) | (scale & 0x0f); /* 0x10 = logical left shift */
    ALU_MULT = 0;
    ALU_IN1 = 0x44; /* bus A = k3, bus B = k2 */
    ALU_IN2 = fb_aluch[fb]; /* bus C = fb low, bus D = fb high */
    ALU_OUT = 0x01 << fb_high;

    CSR_HI(fb) = CSR_HI(fb) | (IPFB_CSRHCSF>>8);/* single acquire */
    WAIT (fb);
    WAITVB (fb);
    PAN (fb) += 11;
}
