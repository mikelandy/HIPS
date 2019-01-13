/* daccess.c - functions for accessing data from frames. */

#include <hipl_format.h>
#include <math.h>
#include <xdisp.h>

void convert_complex();
char *get_data_by_name_f();


/*********************************************
 * f()
 *********************************************/

/* f returns the floating point representation of the data item at a given
   row and column position in a given frame. If the pixel format is complex,
   the real part is returned.
*/

float f(fr,r,c)
  int	fr;
  int	r;
  int	c;
{
  int offset = r*ncols+c;
  float z;

    switch (pixel_format) {
	case PFBYTE:	z = (float)((unsigned char *)data[fr])[offset];
			break;
	case PFSHORT:	z = (float)((short *)data[fr])[offset];
			break;
	case PFINT:	z = (float)((int *)data[fr])[offset];
			break;
	case PFFLOAT:	z = ((float *)data[fr])[offset];
			break;
	case PFCOMPLEX:	z = ((float *)data[fr])[offset*2];
			break;
	default:	z = 0.0;
	}
    return(z);
}



/*********************************************
 * f_i()
 *********************************************/

/* f_i returns the floating point representation of the imaginary part of the
   data item at a given row and column position in a given frame. If the pixel
   format is not complex, zero is returned.
*/

float f_i(fr,r,c)
  int	fr;
  int	r;
  int	c;
{
  int offset = r*ncols+c;
  float z;

    switch (pixel_format) {
	case PFCOMPLEX:	z = ((float *)data[fr])[offset*2+1];
			break;
	default:	z = 0.0;
	}
    return(z);
}



/*********************************************
 * get_data_by_name()
 *********************************************/

/* returns the string representation of the data item at at given row and
   column of a given frame. If frame is virtusl (overlay), obtain data
   for 3 chosen frames.
*/

char *get_data_by_name(fr,r,c,s)
  int	fr;
  int	r;
  int	c;
  char  *s;
{
  char s1[32],s2[32],s3[32];

    if (fr >= nframes) {
	get_data_by_name_f(red_frame,r,c,s1);
	get_data_by_name_f(green_frame,r,c,s2);
	get_data_by_name_f(blue_frame,r,c,s3);
	sprintf(s,"(%s,%s,%s)",s1,s2,s3);
	}
    else
	get_data_by_name_f(fr,r,c,s);
}



/*********************************************
 * get_data_by_name_f()
 *********************************************/

/* returns the string representation of the data item at at given row and
   column of a given frame.
*/

char *get_data_by_name_f(fr,r,c,s)
  int	fr;
  int	r;
  int	c;
  char  *s;
{
  int offset = r*ncols+c;

    switch (pixel_format) {
	case PFBYTE:	sprintf(s,"%d",
				(int)((unsigned char *)data[fr])[offset]);
			break;
	case PFSHORT:	sprintf(s,"%d",(int)((short *)data[fr])[offset]);
			break;
	case PFINT:	sprintf(s,"%d",((int *)data[fr])[offset]);
			break;
	case PFFLOAT:	sprintf(s,"%.*f",precision,
				(double)((float *)data[fr])[offset]);
			break;
	case PFDOUBLE:	sprintf(s,"%.*f",precision,
				((double *)data[fr])[offset]);
			break;
	case PFCOMPLEX:	convert_complex(
				((float *)data[fr])[offset*2],
				((float *)data[fr])[offset*2+1],
				s);
			break;
	case PFSBYTE:	sprintf(s,"%d",(int)((char *)data[fr])[offset]);
			break;
	case PFUSHORT:	sprintf(s,"%d",
				(int)((unsigned short *)data[fr])[offset]);
			break;
	case PFUINT:	sprintf(s,"%d",((unsigned int *)data[fr])[offset]);
			break;
	default:	sprintf(s,"?");
	}
}



/*********************************************
 * convert_complex()
 *********************************************/

/* Convert a complex number to string representation depending on type
   of representation (RECTangular/POLAR), and precision which pertain.
*/

void convert_complex(r,i,s)
  float	r;
  float	i;
  char 	*s;
{
  double dr = (double)r;
  double di = (double)i;

    switch(cdconv) {
	case RECT:	sprintf(s,"%.*f%+.*fi",precision,r,precision,i);
			break;
	case POLAR:	sprintf(s,"(%.*f,%.*f)",
				precision,
				sqrt(dr*dr+di*di),
				precision,
				atan2(di,dr));
			break;
	default:
			sprintf(s,"?");
	}
}



