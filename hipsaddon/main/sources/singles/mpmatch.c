static char *SccsId = "%W%      %G%";

/*	Copyright (c) 1987 Linda Gillespie - courtesy of Marr and Poggio

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
ensure its reliability.   */

/* mpmatch - perform point based stereo matching, a la Marr and Poggio
 *		 
 * usage: mpmatch [-l] [-n its] [-t threshold] [-v] 
 *
 * -l:        For specifying that a larger excitatory neighbourhood is
 *            required i.e. 9 pixels instead of 5.
 * its:       For  indicating the number of iterations (default of 6).
 * threshold: For thresholding the excitation and inhibition sum
 *            (default of 1).
 * -v:        Verbose option.
 *
 * to load: cc -o mpmatch mpmatch.c -lhips
 *
 * Linda Gillespie June 1987.
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <hipl_format.h>
#include <stdio.h>

#define	picsize		128
#define maxIntensity	255
#define dispLevels	5 
#define intensityIncr	maxIntensity/dispLevels

int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] =
{
	{"l",
		{LASTFLAG},
		0,
		{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"n",
		{LASTFLAG},
		1,
		{{PTINT,"6","its"},LASTPARAMETER}},
	{"t",
		{LASTFLAG},
		1,
		{{PTINT,"1","threshold"},LASTPARAMETER}},
	{"v",
		{LASTFLAG},
		0,
		{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG
};
void initC0(),copyC(),oneStep();
int constrainCalc1(),constrainCalc2();

int main(argc,argv)

int	argc;
char	**argv;

{	
	struct          header hd,hdp1,hdp2,hdo;
	int             method;
	Filename        filename;
	FILE            *fp;

	int		numOfIters,cThresh;
	h_boolean		verbose,largeEN;

	byte		*tempOfr;
	int 		r, c, d;
	char		C0[picsize][picsize][dispLevels];
	char		Cn[picsize][picsize][dispLevels];
	char		Cnp1[picsize][picsize][dispLevels];
	int		i,num, outInt;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&largeEN,&numOfIters,&cThresh,&verbose,FFONE,&filename);
	fp=hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	if((hd.orows>picsize)||(hd.ocols>picsize))
		perr(HE_MSG,"max image size = 128") ;
	method=fset_conversion(&hd,&hdp1,types,filename);
	if(hdp1.num_frame!=2)
		perr(HE_MSG,"#frames must be 2.") ;
	dup_headern(&hdp1,&hdp2);
	alloc_image(&hdp2);
	dup_headern(&hdp1,&hdo);
	hdo.num_frame = numOfIters;
	alloc_image(&hdo);
	write_headeru2(&hd,&hdo,argc,argv,hips_convback);
	fread_imagec(fp,&hd,&hdp1,method,0,filename);
	fread_imagec(fp,&hd,&hdp2,method,1,filename);

	initC0(hdp1.image,hdp2.image,C0,hd.orows,hd.ocols);
	copyC(C0,Cn,hd.orows,hd.ocols);

	for(i=0;i<numOfIters;i++)
	{
		if(verbose)
			fprintf(stderr,"%s: Iteration number %d\n",Progname,i);
		tempOfr = hdo.image;
		oneStep(C0,Cn,Cnp1,hd.orows,hd.ocols,cThresh,largeEN);
		copyC(Cnp1,Cn,hd.orows,hd.ocols);
		for(r=0;r<hd.orows;r++)
			for(c=0;c<hd.ocols;c++)
			{
				num = 0; outInt = 0;
				for(d=0;d<dispLevels;d++)
				{
					if(Cn[r][c][d])
					{
						num++;
						outInt = (d+1) * intensityIncr;
					}
				}
				if(num>1) *tempOfr++ = 0;
				else *tempOfr++ = outInt; 
			}
		write_imagec(&hd,&hdo,method,hips_convback,i);
	}

	return(0);
}

/* To change the disparity range change the lowestDisp, highestDisp and 
   dispLevels accordingly. Note that lowestDisp can be smaller than zero. */
#define lowestDisp 0
#define highestDisp 4

#define	tcol(c) (c<0? 0 : (c>=col? col-1 : c))	/* To ameliorate */
#define trow(c) (c<0? 0 : (c>=row? row-1 : c))  /* boundary effects */

void initC0( picL, picR, C0, row, col )

char	C0[picsize][picsize][dispLevels];
byte	*picL, *picR;
int	row, col;

/* This routine initialises 3D array C0. It has values of one
   where there is a match in picL and picR, zeroes elsewhere. */

{	int r, c, d;

	for(r=0;r<row;r++) {
	   for(c=0;c<col;c++) {
	     for(d=0;d<dispLevels;d++) {
		if(picL[r * col + c] == picR[r * col + tcol(c + d + lowestDisp)])
		        C0[r][c][d] = 1;
		   else C0[r][c][d] = 0;
	     }
	   }
	}
}

void copyC( Cnp1, Cn, row, col )

char	Cnp1[picsize][picsize][dispLevels];
char	Cn[picsize][picsize][dispLevels];
int	row, col;

/* This routine copies 3D array Cnp1 into Cn */

{	int r, c, d;

	for(r=0;r<row;r++) {
	   for(c=0;c<col;c++) {
	     for(d=0;d<dispLevels;d++) {
		Cn[r][c][d] = Cnp1[r][c][d];
	     }
	   }
	}
}

int constrainCalc1( r, c, d, Cn, col )

int	r, c, d;
char	Cn[picsize][picsize][dispLevels];
int	col;

/* This routine calculates the constraint terms for the Marr and */
/* Poggio pointwise stereo matching stereo algorithm. It returns */
/* a value corresponding to the constraints: */
/*	Each point in the image may have only one depth value    */
/* 	A point is likely to have a depth value near its neighbours */
/* for the point(r,c,d) in the 3D array. */
/* Excitatory neighbourhood is 5 pixels in size. */

{	int rp, cp, dp;
	int countS, countT;	/* Constraint counts */

	countS = (countT = 0);

	/* For Excitatory Neighbourhood */
	dp = d;
	for(rp=r-1;rp<r+2;rp++) {	/* For all local neighbourhood */
	   if(rp==r){
	      for(cp=c-1;cp<c+2;cp++){

	         if(d==dp)
		   if(Cn[rp][cp][dp]) countS++;

                 if((d-dp)*(d-dp)>=1)
	           if(Cn[rp][cp][dp]) countT++;
    	      }
   	   }
   	    
	   else {
		  cp =c;

		  if(d==dp)
		    if(Cn[rp][cp][dp]) countS++;

        	  if((d-dp)*(d-dp)>=1)
	  	    if(Cn[rp][cp][dp]) countT++;
   	    
	   } /* end of else */
	} /* end of for rp */


	/* Inhibitory Neighbourhood */
	rp = r;
	for(dp=0;dp<dispLevels;dp++){

	   cp = tcol( c + dp - d );
	   if( cp != c )
             if((d-dp)*(d-dp)>=1)
	       if(Cn[rp][cp][dp]) countT++;
    	}

	cp =c;
	for(dp=0;dp<dispLevels;dp++){
             if((d-dp)*(d-dp)>=1)
	       if(Cn[rp][cp][dp]) countT++;
	}
	return(countS - countT);
}

int constrainCalc2( r, c, d, Cn, col )

int	r, c, d;
char	Cn[picsize][picsize][dispLevels];
int	col;

/* This routine calculates the constraint terms for the Marr and */
/* Poggio pointwise stereo matching stereo algorithm. It returns */
/* a value corresponding to the constraints: */
/*	Each point in the image may have only one depth value */
/* 	A point is likely to have a depth value near its neighbours */
/* for the point (r,c,d) in the 3D array. */
/* Excitatory neighbourhood is 9 pixels in size. */

{	int rp, cp, dp;
	int countS, countT;	/* Constraint counts */

	countS = (countT = 0);

	/* Excitatory Neighbourhood */
	dp = d;
	for(rp=r-1;rp<r+2;rp++) {	/* For all local neighbourhood */
	   for(cp=c-1;cp<c+2;cp++){

	         if(d==dp)
		   if(Cn[rp][cp][dp]) countS++;

                 if((d-dp)*(d-dp)>=1)
	           if(Cn[rp][cp][dp]) countT++;
    	   }
	}


	/* Inhibitory Neighbourhood */
	rp = r;
	for(dp=0;dp<dispLevels;dp++){

	   cp = tcol( c + dp - d );
	   if( cp != c )
             if((d-dp)*(d-dp)>=1)
	       if(Cn[rp][cp][dp]) countT++;
    	}

	cp =c;
	for(dp=0;dp<dispLevels;dp++){
             if((d-dp)*(d-dp)>=1)
	       if(Cn[rp][cp][dp]) countT++;
	}
	return(countS - countT);
}


void oneStep( C0, Cn, Cnp1, row, col, cThresh, largeEN )

char	C0[picsize][picsize][dispLevels];
char	Cn[picsize][picsize][dispLevels];
char	Cnp1[picsize][picsize][dispLevels];
int	row, col;
int	cThresh;
char	largeEN;

/* This routine performs one step of the Marr and Poggio iterative */
/* point matching stereo algorithm. C0 is the initial 3D array, */
/* Cn is the present and Cnp1 the next array */

{	int r, c, d;
	int count=0;

	if( largeEN )
	  for(r=0;r<row;r++) {
	     for(c=0;c<col;c++) {
	        for(d=0;d<dispLevels;d++) {
		  count = constrainCalc2(r,c,d,Cn,col) + C0[r][c][d];

		  if(count>cThresh) Cnp1[r][c][d] = 1;
		   else Cnp1[r][c][d] = 0;
	        }
	      }
	   }
	else
	  for(r=0;r<row;r++) {
	     for(c=0;c<col;c++) {
	        for(d=0;d<dispLevels;d++) {
		  count = constrainCalc1(r,c,d,Cn,col) + C0[r][c][d];

		  if(count>cThresh) Cnp1[r][c][d] = 1;
		   else Cnp1[r][c][d] = 0;
	        }
	      }
	   }
}
