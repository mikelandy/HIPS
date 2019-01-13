static char *SccsId = "%W%      %G%";

/*	Copyright (c) 1987 Linda Gillespie 

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
ensure its reliability.   */

/* h_grey2disp - given a grey-level coded disparity image the
 *	program will generate a random dot stereogram interpretation.
 *	Copes with fractions of a pixel disparity.
 *		 
 * usage: h_grey2disp(hdi,hdo1,hdo2,cnt,ldisp,hdisp,threshold)
 *
 * cnt:         If set, pixel disparity is required. (corresponds to
 *              the original module "grey2disp". Otherwise sub-pixel
 *              disparity is required. (corresponds to the origanal
 *              module "grey2dispcnt".)
 * ldisp,
 * hdisp: For indicating the range of disparities. 
 * threshold:   If TRUE, i.e. thresholded output is required.
 *
 * to load: cc -o h_grey2disp h_grey2disp.c -lhips -lm
 *
 * Linda Gillespie August 1987.
 * Adapted to HIPS-2 Version by Jin Zhengping - 31 August 1991
 */

#include <hipl_format.h>
#include <math.h>

void srand_gg(),addToList(),calcIntensity(),createRightcnt();
double rand_g();
int gaussb(),threshImage(),createRight();
float greyToDisp();

int h_grey2disp(hdi,hdo1,hdo2,cnt,ldisp,hdisp,threshold)
struct header	*hdi,*hdo1,*hdo2;
int		ldisp,hdisp;
h_boolean		cnt,threshold;
{
	switch(hdi->pixel_format)
	{
	case PFBYTE:
		return(h_grey2disp_b(hdi,hdo1,hdo2,
			cnt,ldisp,hdisp,threshold));
	default:
		return(perr(HE_FMTSUBR,"h_grey2disp",
			hformatname(hdi->pixel_format)));
	}
}

int h_grey2disp_b(hdi,hdo1,hdo2,cnt,ldisp,hdisp,threshold)
struct header	*hdi,*hdo1,*hdo2;
int		ldisp,hdisp;
h_boolean		cnt,threshold;
{
	return(h_grey2disp_B(hdi->image,hdo1->image,hdo2->image,
		hdi->orows,hdi->ocols,
		cnt,ldisp,hdisp,threshold));
}

int	seed=1;

int h_grey2disp_B(imagei,imageo1,imageo2,nr,nc,
	cnt,ldisp,hdisp,threshold)
byte		*imagei,*imageo1,*imageo2;
int		nr,nc;
int		ldisp,hdisp;
h_boolean		cnt,threshold;
{
	char	 *memsetl();

	memsetl((char *)imageo1,(int)128,nr*nc*sizeof(*imageo1));
	memsetl((char *)imageo2,(int)128,nr*nc*sizeof(*imageo1));

	/* Generate two different random-dot images */
	srand_gg((unsigned)seed);
	H__SRANDOM(seed);
	gaussb(imageo1,nr,nc);

	seed=seed+4;
	srand_gg((unsigned)seed);
	H__SRANDOM(seed);
	gaussb(imageo2,nr,nc);

	/* Threshold random-dot images to obtain binary versions */	
	if (threshold == TRUE)
	{
		threshImage(imageo1,nr,nc);
		threshImage(imageo2,nr,nc);
	}

	/* Generate random-dot stereogram */
	if(cnt==TRUE)
		createRight(imageo1,imageo2,imagei,nr,nc,ldisp,hdisp);
	else
		createRightcnt(imageo1,imageo2,imagei,nr,nc,ldisp,hdisp);

	return(HIPS_OK);
}

#define	PICSIZE	256		/* The maximum size of picture */
int	picsize=PICSIZE;	/* for use by routines that call this one */
#define maxIntensity	255	/* The maximum intensity allowed.*/
#define thresh	128.0		/* For thresholding random dot images */

#define tcol(c) (c<0? 0: (c>=col? col-1 : c)) /* To ameliorate */

struct node {
	float disp;		/* total disparity shift involved */
	float fract;	 /* fraction of present pixel covered by this node */
	int intensity; 		/* intensity contributed */
	struct node *next;	/* next node to be considered */
};
typedef struct node element;
typedef element *link;


struct listheads {
	link lower;
	link upper;
};
typedef struct listheads headOfList;
typedef headOfList complexCol[PICSIZE];

/* The following routines are used to generate gaussian noise */
/* images. These images will be used as the basis of the random-dot */
/* stereogram. */

double	p=30.;

/* rand_g.c - to compute random normal deviates by a
**		method based on the central limit theorem.
**
** doesn't use the routine "rand()", initialization
** should be done via srand_g(seed).
**
**
** Timing: about 500 usec per call
**
**
** Yoav Cohen 6/4/82
**
*/
static long grandx = 1;

void srand_gg(x)
unsigned x;
{
	grandx=x;
}


double rand_g()
{
	static int i, sum ;

	sum=0.0 ;
	for (i=0;i<12;i++)
	   sum += (((grandx=grandx*1103515245+12345)&0x7fffffff)>>4) ;
	return(sum/134217728.0-6.0) ;
}



int gaussb ( pic, row, col )
/* This routine generates a noisy (gaussian noise) image in pic. */

byte	*pic;
int	row,col;

{
	short		int *noise , *pnoise , *noisei;
	int		*pt, *pr, *pc;
	byte		*pici;
	int		i, j, m;
	unsigned	nrc=row*col;

	/* allocate core */
	if((noise=(short int *)calloc(nrc,sizeof(short int)))==0
	 ||(pr=(int *)calloc((unsigned)row,sizeof(int)))==0
	 ||(pc=(int *)calloc((unsigned)col,sizeof(int)))==0)
		perr(HE_ALLOC);

	pt=pr;
	for(i=0;i<row;i++)
		*pt++ = i;
	pt=pc;
	for(i=0;i<col;i++)
		*pt++ = i;

	pnoise=noise;
	for(i=0;i<nrc;i++)
		*pnoise++ = rand_g()*p;

	for(i=0;i<row;i++) {
		noisei=noise+(*(pr+i))*col;
		pici=pic+i*col;
		for(j=0;j<col;j++) {
			m= *(noisei+(*(pc+j)))+(*(pici+j));
			if(m<0)m=0;
			else if(m>255)m=255;
			*(pici+j)=m;
		}
	}
	free((char *)noise);
	free((char *)pr);
	free((char *)pc);
	return(HIPS_OK);
}

/* The following routine performs the thresholding stage of the program. */
unsigned char	hchar = 255,lchar = 0;

int threshImage ( bframe, row, col )
/* This routine thresholds the image, bframe, at thresh ( see define    */
/*  statements at the top of the program ).				*/

byte	*bframe;
int	row, col;

{
	int	nframe, r, c, i;
	int	fr, bthresh;
	byte	*bp;

	nframe = 1;
	bthresh = (int) (thresh + .9999999);
	
	for (fr=0;fr<nframe;fr++) {
	   bp = bframe;
	   for (r=0;r<row;r++) {
	      for (c=0;c<col;c++) {
		 i = (*bp >= bthresh) ? hchar : lchar;
		 *bp++=i;
	      }
	   }
	}
    return(HIPS_OK);
}


/* This routine performs the grey-level to disparity decoding. */

float greyToDisp ( greyLevel, binsize, ldisp, hdisp )
/* This routine converts a grey-level coded value into
   a disparity value. */

int	greyLevel;
int	binsize;
int	ldisp, hdisp;

{	float	disp;

	disp=(float) greyLevel/(float) binsize+ldisp;
	if (disp>hdisp)
	  disp=(float) hdisp;

	return(disp);
}

link insertFirst( intensity, disp, frac )

int	intensity;
float	disp, frac;

{	
	link	 current;

	if (frac!=0) {
	  /* List is empty so far */
	    current=(link) calloc(1,sizeof(element));
	    current->intensity=intensity;
	    current->disp=disp;
	    current->fract=frac;
	    current->next=NULL;
	}
	return(current);
}

void addToList(intensity, disp, frac, head )

int	intensity;
float	disp, frac;
link	head;

{	
	link	last, current;

	if ((head==NULL)&&(frac!=0)) {
	  /* List is empty so far */
	    current=(link) calloc(1,sizeof(element));
	    current->intensity=intensity;
	    current->disp=disp;
	    current->fract=frac;
	    current->next=NULL;
	    head=current;
	}
	else if (frac!=0) {
	    current = head; last=current;
	    while ((current!=NULL)&&(disp<current->disp)&&(frac>current->fract)){
		 last=current;
		 current=current->next;
	    }

	    if ( current==NULL) { 
	      last->next=(link) calloc(1,sizeof(element));
	      last=last->next;
	      last->intensity=intensity;
	      last->disp=disp;
	      last->fract=frac;
	      last->next=NULL;
	    }


	    else {
		if (disp >current->disp){
		  if (current==head){
	            head->next=(link) calloc(1,sizeof(element));
	            last=head->next;
	            last->intensity=intensity;
	            last->disp=disp;
	            last->fract=frac;
	            last->next=current;
		  }
		  else{
	            last->next=(link) calloc(1,sizeof(element));
	            last=last->next;
	            last->intensity=intensity;
	            last->disp=disp;
	            last->fract=frac;
	            last->next=current;
		  }
		  /* remove all old values which this fraction will cover */
		  while ((frac>current->fract) && (current!=NULL))
		       {
		       last->next=current->next;
		       free((char *)current);
		       current = last->next;
		       }
	        }
	    }
	}	    
}

void calcIntensity (head, RightPicLoc)

headOfList	head;
byte		*RightPicLoc;

{
	float	frac;
	int	intensity;
 	
	float	fracCount=0;
	float	newfrac=0;
	float	lastFR=0; 
	float	lastFL=0;
	float	overlap=0;
	float	intensSoFar=0;

	link	current;

	while ((fracCount<1)&&((head.upper!=NULL)||(head.lower!=NULL))){
	     if ((head.lower!=NULL)&&((head.upper==NULL)||
		 (head.lower->disp >= head.upper->disp))){
	       current = head.lower;
	       frac=current->fract;
	       intensity=current->intensity;
	       head.lower = current->next;
	       free((char *)current);
	       overlap = (frac+lastFR)-1;
	       if (overlap>0)
		 newfrac=(frac-lastFL-overlap);	/*Overlap with top half of bit*/
	       else 
		 newfrac=(frac-lastFL);		/* No Overlap */
	       lastFL=frac;

	     }
	     else  if ((head.upper!=NULL)&&((head.lower==NULL)||
		        (head.upper->disp >= head.lower->disp))){
	       current = head.upper;
	       frac=current->fract;
	       intensity=current->intensity;
	       head.upper = current->next;
	       free((char *)current);
	       overlap = (frac+lastFL)-1;
	       if (overlap>0)
		 newfrac=(frac-lastFR-overlap);	/*Overlap with top half of bit*/
	       else 
		 newfrac=(frac-lastFR);		/* No Overlap */
	       lastFR=frac;

	     }
	     intensSoFar += (newfrac/frac)*intensity;
	     fracCount += newfrac;
	}
	if (fracCount<1){
	  intensSoFar += (1-fracCount) * *RightPicLoc;
	}
	*RightPicLoc = (int) intensSoFar;
}

void createRightcnt( LP, RP, dispImage, row, col, ldisp, hdisp )

byte	*LP, *RP;
byte	*dispImage;
int	row, col;
int	ldisp, hdisp;

/* Generate the second half of the random-dot stereogram by using */
/* the grey-level disparity map to change the left stereo half 	  */
/* to form the right stereo half. Note that occlusions will be    */
/* automatically dealt with because the right stereo half origin- */
/* contains noise ( different noise to that of the left stereo half.) */

{	int	r, c;
	int	dispLevels, greyLevel, binsize;
	float	disp;
	float	greyToDisp();
	link	insertFirst();

	byte	*RLoc;

	complexCol colStruct;
	int	smallPixel, bigPixel;
	float	frac;
	int	intensity;
 	
	dispLevels=hdisp-ldisp+1;
	binsize=(maxIntensity+1)/dispLevels;

	for (r=0;r<row;r++){
	   for (c=0;c<col;c++){
	      colStruct[c].lower=NULL;
	      colStruct[c].upper=NULL;
	   }

   	   for (c=0;c<col;c++){
	      greyLevel=(int)(*dispImage++);
	      disp=greyToDisp(greyLevel,binsize,ldisp,hdisp);
	      frac=disp-(int)disp;

	      if (frac>=0){
		smallPixel = c + (int)disp;
		bigPixel = c + (int)disp + 1;
	      }
	      else {
		smallPixel= c + (int)disp - 1;
		bigPixel= c + (int)disp;
	      }

	      intensity = *LP++;

	      if (frac>=0){
		if (( colStruct[tcol(smallPixel)].upper ==NULL)&&((1-frac)!=0))
	          colStruct[tcol(smallPixel)].upper=
			insertFirst ((int)((1-frac)*intensity), disp, (1-frac));
		else
		  addToList((int)((1-frac)*intensity), disp, (1-frac), 
					     colStruct[tcol(smallPixel)].upper);

		if (( colStruct[tcol(bigPixel)].lower==NULL)&&(frac!=0))
	          colStruct[tcol(bigPixel)].lower =
			insertFirst((int)(frac*intensity), disp, frac);
		else
	   	  addToList((int)(frac*intensity), disp, frac, 
					       colStruct[tcol(bigPixel)].lower);
	      }
	      else {
		frac= 0.0 - frac;
		if (( colStruct[tcol(smallPixel)].upper==NULL)&&(frac!=0))
	          colStruct[tcol(smallPixel)].upper=
			insertFirst((int)(frac*intensity), disp, frac);
		else
	          addToList((int)(frac*intensity), disp, frac, 
					     colStruct[tcol(smallPixel)].upper);

		if ((colStruct[tcol(bigPixel)].lower==NULL)&&((1-frac)!=0))
	          colStruct[tcol(bigPixel)].lower=
		   	insertFirst((int)((1-frac)*intensity), disp, (1-frac));
		else
	          addToList((int)((1-frac)*intensity), disp, (1-frac), 
			                       colStruct[tcol(bigPixel)].lower);
	      }
	   }

	   for (c=0;c<col;c++){
	      RLoc = RP+(r*col)+c;
	      calcIntensity(colStruct[c], RLoc);
	   }
	}
}

int createRight(imageol,imageor,imagei,nr,nc,ldisp,hdisp)
byte	*imageol,*imageor,*imagei;
int	nr,nc,ldisp,hdisp;
{
	int		r,c,disp;
	byte		*lp=imageol;
	byte		*rp=imageor;
	byte		*dispImage=imagei;
	int		binsize=256/(hdisp-ldisp+1);
		for(r=0;r<nr;r++,rp+=nc)
		   for(c=0;c<nc;c++,dispImage++,lp++)
		   {
			disp=c+*(dispImage)/binsize+ldisp;
			if (disp>=0 && disp<nc)
				*(rp + disp) = *lp;
		   }
	return(HIPS_OK);
}
