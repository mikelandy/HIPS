/*      Copyright (c) 1987, 1988 UCLA Machine Perception Laboratory
Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.  
*/

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sundev/vipreg.h>

#define DISPLAY_WIDTH  512
#define TRUE	1
#define FALSE	0
int true = TRUE, false = FALSE, arg;

int vip_fd[3];
unsigned char *vip_base[3];

char *dev_name[] = {"/dev/vip0", "/dev/vip1", "/dev/vip2"};



/*
 * General setup routine to open up a VIP device. 
 * Parameters are given below:
 *
 *        board_number: 0  (/dev/vip0 ==> GREEN MASTER)
 *                      1  (/dev/vip1 ==> RED         )
 *                      2  (/dev/vip2 ==> BLUE        )
 *
 *        reset_registers: 0 (leave the current vip register values alone)
 *                         1 (set registers into a user_friendly state)
 *
 *        clear_memory: 0  (do not erase the board's frame memory)
 *                      1  (clear frame memory to value of 0     )
 *
 *        define_luts: 0  (leave the current lut values alone)
 *                     1  (set up all luts in a linear scale )
 *
*/

mpl_vip_setup(board_number,reset_registers,clear_memory,define_luts)
int board_number;
int reset_registers;
int clear_memory;
int define_luts;
{

  if ((vip_fd[board_number] = open(dev_name[board_number], O_RDWR, 0)) == -1) 
     {
      fprintf(stderr, "Can't open %s\n", dev_name[board_number]);
      return(FALSE);
     }

  if (ioctl(vip_fd[board_number], VIPNIOCGETDRIVERSTATE, &arg) == -1) 
     {
      fprintf(stderr, "ioctl failed for %s\n", dev_name[board_number]);
      return(FALSE);
     }
  else if (arg & VIP_NOTALONE) 
          {
	    fprintf(stderr, "%s: we are not alone!\n", dev_name[board_number]);
	    return(FALSE);
          }


  if (reset_registers)
     {
      if (ioctl(vip_fd[board_number], VIPXIOCRESETREGISTERS, &arg) == -1)
         {
	   fprintf(stderr, "ioctl VIPXIOCRESETREGISTERS failed for %s\n", 
                           dev_name[board_number]);
	   return(FALSE);
         }
     }

  if ((vip_base[board_number] = (unsigned char *)valloc(VIP_SIZE)) == NULL) 
     {
       fprintf(stderr, "Unable to allocate %d bytes for %s\n", VIP_SIZE, 
                       dev_name[board_number]);
       return(FALSE);
     }

  if (mmap(vip_base[board_number], VIP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
		 vip_fd[board_number], 0) == -1) 
     {
       fprintf(stderr,"Unable to mmap memory for %s\n", dev_name[board_number]);
       return(FALSE);
     }

  if (ioctl(vip_fd[board_number], VIPBIOCNOREGSONCLOSE, &arg) == -1) 
     {
      fprintf(stderr, "ioctl VIPBIOCNOREGSONCLOSE failed for %s\n", 
                      dev_name[board_number]);
      return(FALSE);
     }

  if (define_luts)
     mpl_vip_def_luts(board_number);
 
  if (clear_memory)
     mpl_vip_clear(board_number,0);

 return(TRUE);
}



mpl_vip_close_device(board_number)
int board_number;
{
   close(vip_fd[board_number]);
}

mpl_vip_clear(board, color)
int board;
unsigned char color;
{
    register long cl;
    register int cnt;
    register long *ptr;

    cl = (color << 24) | (color << 16) | (color << 8) | color;
    ptr = (long *)vip_base[board];
    for (cnt = 512 * 512; cnt > 0; cnt -= 4) *ptr++ = cl;
}


mpl_vip_quadbuf()
{
    int i;

    for (i = 0; i < 3; i++) {
	ioctl(vip_fd[i], VIPBIOCQUADBUF, (arg = 1, &arg));
    }
}


mpl_vip_quadrant(q)
int q;
{
    int i;

    for (i = 0; i < 3; i++) {
	ioctl(vip_fd[i], VIPNIOCQUADRANT, &q);
    }
}


mpl_vip_def_luts(board)
int board;
{
    unsigned char olut[256], ilut[256];
    int i;
    long l;
int w;

    for (i = 0; i < 256; i++) {
	olut[i] = i; ilut[i] = 255 - i;
    }

    l = lseek(vip_fd[board], VIP_RED_LUT, L_SET);
    w = write(vip_fd[board], olut, 256);
    l = lseek(vip_fd[board], VIP_GREEN_LUT, L_SET);
    w = write(vip_fd[board], olut, 256);
    l = lseek(vip_fd[board], VIP_BLUE_LUT, L_SET);
    w = write(vip_fd[board], olut, 256);
    l = lseek(vip_fd[board], VIP_INPUT_LUT, L_SET);
    w = write(vip_fd[board], ilut, 256);
}

