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
#define FALSE 0
#define TRUE 1

extern int vip_fd[3];
extern char *dev_name[];



mpl_vip_cont_grab_frame(video_input_channel,board)
int video_input_channel;
int board;
{
    int return_code, status_code, turn_on;
    

    if (ioctl(vip_fd[board],VIPNIOCGETDRIVERSTATE,&return_code) == -1 )
       {
         fprintf(stderr,"ioctl VIPNIOCGETDRIVERSTATE failed for %s\n",
                        dev_name[board]);
         return(FALSE);
       }
    else if (return_code & VIP_NOTALONE)
            {
             fprintf(stderr,"we are not alone on the board %s\n",
                            dev_name[board]);
             return(FALSE);
            }

                       /* set the active video input channel */
 
    if (ioctl(vip_fd[board],VIPNIOCINPUTSELECT,&video_input_channel) == -1 )
       {
         fprintf(stderr,"ioctl VIPNIOCINPUTSELECT failed for %s\n",
                        dev_name[board]);
         return(FALSE);
       }

                       /* switch to external sync */

    if (ioctl(vip_fd[board],VIPBIOCEXTERNALSYNC,&return_code) == -1 )
       {
         fprintf(stderr,"ioctl VIPBIOCEXTERNALSYNC failed for %s\n",
                        dev_name[board]);
         return(FALSE);
       }

                     /* turn on the continues grab mode */
    turn_on = 1;
    if (ioctl(vip_fd[board],VIPBIOCCONTFRAMEGRAB,&turn_on) == -1 )
       {
         fprintf(stderr,"ioctl VIPBIOCCONTFRAMEGRAB failed for %s\n",
                        dev_name[board]);
         return(FALSE);
       }
    
    return(TRUE);
    
}


mpl_vip_snap(board)
int board;
{
    int return_code, status_code;

    return_code = 0;

    if (ioctl(vip_fd[board],VIPBIOCCONTFRAMEGRAB,&return_code) == -1 )
       {
         fprintf(stderr,"ioctl VIPBIOCGETCONTFRAMEGRAB failed for %s\n",
                        dev_name[board]);
         return(FALSE);
       }
    else if (return_code != 0)
            {
              fprintf(stderr,"return code = %d\n",return_code);
              fprintf(stderr,"Nothing to snap -- no live video being digitized on %s\n", dev_name[board]);
              return(FALSE);
            }

    return_code =0;  /* turn off the continues grab mode */
    if (ioctl(vip_fd[board],VIPBIOCCONTFRAMEGRAB,&return_code) == -1 )
       {
         fprintf(stderr,"ioctl VIPBIOCCONTFRAMEGRAB failed for %s\n",
                        dev_name[board]);
         return(FALSE);
       }

                     /* grab one frame */

    if (ioctl(vip_fd[board],VIPXIOCFRAMEGRAB,&return_code) == -1 )
       {
         fprintf(stderr,"ioctl VIPXIOCFRAMEGRAB failed for %s\n",
                        dev_name[board]);
         return(FALSE);
       }

    if (ioctl(vip_fd[board],VIPNIOCGETSTATUS,&status_code) == -1 )
       {
         fprintf(stderr,"ioctl VIPNIOCGETSTATUS failed for %s\n",
                        dev_name[board]);
         return(FALSE);
       }

                     /* wait until all of the frame is captured */

    while (status_code & VIP_STATUS_FGRACT )
           if (ioctl(vip_fd[board],VIPNIOCGETSTATUS,&status_code) == -1)
              {
                fprintf(stderr,"ioctl VIPNIOCGETSTATUS failed for %s\n",
                               dev_name[board]);
                return(FALSE);
              }

    return_code =0;   /* switch to internal sync */

    if (ioctl(vip_fd[board],VIPBIOCEXTERNALSYNC,&return_code) == -1 )
       {
         fprintf(stderr,"ioctl VIPBIOCEXTERNALSYNC failed for %s\n",
                        dev_name[board]);
         return(FALSE);
       }

    return(TRUE);
}

