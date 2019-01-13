
/*      Copyright (c) 1992  Karsten Hartelius, IMSOR.

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   

 reg2irr - transforms a regular HIPS-image to a file of the IMSOR defined
	Irregular format.  

 usage: see manual

 to load:
	cc -o reg2irr reg2irr.c -L /gsnk1/hips2/lib -lm -lhips

 to run:
     reg2irr  parameters  < HIPS-file  > HIPS-file 

 include-files:  util.h
*/



#include "util.h"
#include <stdio.h>
#include <hipl_format.h>

struct Limits {
	double 	lx, ux, ly, uy, dx, dy;
	int		nx, ny;
	};

Dvector	regptr,	/* frame of regular dataset */
		irrptr,	/* frame of irregular dataset */
		xptr,	/* x-coordinate of irregular dataset */
		yptr;	/* y-coordinate of irregular dataset */

Ivector	indxptr;	/* index of irregular points in regptr */

int		npix,	/* no.of pixels in input dataset */
		nirr,	/* no.of points in output dataset */
		nframes,	/* no.of frames in input dataset */
		in_form,	/* format of input-data */
		out_form;	/* format of output-data */

double	tolerance2,
				/* maximum distance of irr.point to corresponding pixel */
		missing,	/* value of missing input-data */
		neutralval;
				/* value of output-data not assigned a value */

struct Limits
		reg,		/* coordinate-limits of input-data */
		irr;		/* coordinate-limits of input-data */

Boolean	irrgrid,	/* output-data is a grid */
		defaultx,	/* x-limits of output-grid equal to input-grid */	
		defaulty,	/* y-limits of output-grid equal to input-grid */	
		defaultstep,
				/* pixel-size of output-grid equal to input-grid */
		defaultsize,	
				/* no.of rows and cols equal to input-grid */	
		undersampled,
				/* missing value in dataset */
		gridspec,	/* coordinate-limits specified in input-header */
		tolflag;	/* irr.point must lie within certain distance from pixel */
		
char		*irrfilename;
				/* name of file containing irregular point-set */


int main(argc,argv)
int	argc;
char	*argv[];
{
	void read_irreg(), create_irr(), create_indx(), calculate(), 
		inputreg(), rparam();

	inputreg();
	rparam(argc,argv);
	if (irrgrid)
		create_irr();
	else
		read_irreg(irrfilename);
	create_indx();
	calculate(argc,argv);
	return(0);
}



void read_irreg(irrfname)
char		*irrfname;
{
	struct header hd;
	byte		val=0;
	int		count=1;
	FILE		*irrfile;

	irrfile=fopen(irrfname,"r");
	fread_header(irrfile,&hd,irrfname);
	if (findparam(&hd,"Irregular") != NULLPAR)
		getparam(&hd,"Irregular",PFBYTE,&count,&val);
	if (val == 0)
		perr(HE_MSG,"File is not of type Irregular");

	nirr=hd.ocols;

	irrptr=dvector(nirr);
	indxptr=ivector(nirr);
	xptr=dvector(nirr);
	yptr=dvector(nirr);
	fread_to_dvec(irrfile,xptr,nirr,hd.pixel_format);
	fread_to_dvec(irrfile,yptr,nirr,hd.pixel_format);
	return;
}

void create_irr()
{
	int	i, j;

	if (defaultx){
		irr.lx = reg.lx;
		irr.ux = reg.ux;
		}
	if (defaulty){
		irr.ly = reg.ly;
		irr.uy = reg.uy;
		}
	if (!defaultstep){
		irr.nx = (irr.ux-irr.lx)/irr.dx + 1;
		irr.ny = (irr.uy-irr.ly)/irr.dy + 1;
		}
	if (!defaultsize){
		irr.dx = (irr.ux-irr.lx)/irr.nx;
		irr.dy = (irr.uy-irr.ly)/irr.ny;
		}
	if (defaultsize && defaultstep){
		irr.dx = reg.dx;
		irr.dy = reg.dy;
		irr.nx = (irr.ux-irr.lx)/irr.dx + 1;
		irr.ny = (irr.uy-irr.ly)/irr.dy + 1;
		}
	nirr = irr.nx*irr.ny;

	irrptr=dvector(nirr);
	indxptr=ivector(nirr);
	xptr=dvector(nirr);
	yptr=dvector(nirr);

	/* calculate x,y coordinates of irr.points. */ 
	for (i=0;i<irr.ny;i++)
		for (j=0;j<irr.nx;j++){
			xptr[i*irr.nx + j] = irr.lx + irr.dx*j;
			yptr[i*irr.nx + j] = irr.uy - irr.dy*i;
			}
	return;
}
			
			
void create_indx()
{
	int		i, row, col, include;
	double	dist2, distance2();

	include=TRUE;
	for (i=0;i<nirr;i++){
		/* find row and column of irr.point in grid */
		col = (xptr[i]-reg.lx+0.5*reg.dx)/reg.dx; 
		row = reg.ny - 1 - (int)((yptr[i]-reg.ly+0.5*reg.dy)/reg.dy); 
		if (tolflag){
			dist2=distance2(row,col,yptr[i],xptr[i]);
			include=(dist2<=tolerance2) ? 1 : 0;
			}
		/* calculate index from row,column if irr.point is inside grid */ 
		if ((col>=0)&&(col<reg.nx)&&(row>=0)&&(row<reg.ny)&&(include))
			indxptr[i] = row*reg.nx + col;
		else
			indxptr[i] = -1;
		}
	return;
}

double distance2(r,c,y,x)
int		r, c;
double	y, x;
{
	return((x-(reg.lx+c*reg.dx))*(x-(reg.lx+c*reg.dx)) +
		  (y-(reg.ly+(reg.ny-1-r)*reg.dy))*(y-(reg.ly+(reg.ny-1-r)*reg.dy)));	
}

void calculate(argc,argv)
int		argc;
char		*argv[];
{
	int	i, f;
	struct header hd;
	init_header(&hd,"IMSOR","",nframes+2,"",1,nirr,out_form,1);
	setparam(&hd,"Irregular",PFBYTE,1,1);
	update_header(&hd,argc,argv);
	write_header(&hd);

	fwrite_from_dvec(stdout,xptr,nirr,out_form);
	fwrite_from_dvec(stdout,yptr,nirr,out_form);
	for (f=0;f<nframes;f++){
		fread_to_dvec(stdin,regptr,npix,in_form);
		for (i=0;i<nirr;i++)
			if ((indxptr[i]==-1)||((undersampled)&&(regptr[indxptr[i]]==missing)))
				irrptr[i]=neutralval;
			else
				irrptr[i]=regptr[indxptr[i]];
		fwrite_from_dvec(stdout,irrptr,nirr,out_form);
		}
	return;
}

void rparam(argc,argv)
int	argc;
char	*argv[];
{
	int 	i;

	irrgrid=TRUE;
	defaultx=TRUE;
	defaulty=TRUE;
	defaultstep=TRUE;
	defaultsize=TRUE;
	undersampled=FALSE;
	tolflag=FALSE;
	missing=neutralval=-1;
	for (i=1;i<argc;i++) if (argv[i][0]=='-')
	switch (argv[i][1]){
		case 'x':	reg.lx=(double)atof(argv[++i]);
				reg.ux=(double)atof(argv[++i]);
				break;
		case 'y':	reg.ly=(double)atof(argv[++i]);
				reg.uy=(double)atof(argv[++i]);
				break;
		case 'f':	irrgrid=FALSE;	
				irrfilename=argv[++i];
				break;
		case 'g':	i++;
				if ((i<argc) && (!strcmp(argv[i],"x"))){
					defaultx = FALSE;
					irr.lx = (double)atof(argv[++i]);
					irr.ux = (double)atof(argv[++i]);
					i++;
					}
				if ((i<argc) && (!strcmp(argv[i],"y"))){
					defaulty = FALSE;
					irr.ly = (double)atof(argv[++i]);
					irr.uy = (double)atof(argv[++i]);
					i++;
					}
				if ((i<argc) && (!strcmp(argv[i],"step"))){
					defaultstep = FALSE;
					irr.dx = (double)atof(argv[++i]);
					irr.dy = (double)atof(argv[++i]);
					i++;
					}
				if ((i<argc) && (!strcmp(argv[i],"size"))){
					defaultsize = FALSE;
					irr.nx = atoi(argv[++i]);
					irr.ny = atoi(argv[++i]);
					i++;
					}
				i--;
				break;
	
		case 'm':	undersampled=TRUE;
				missing=(double)atof(argv[++i]);
				break;
		case 'n':	neutralval=(double)atof(argv[++i]);
				break;
		case 't':	tolflag=TRUE;
				tolerance2=(double)atof(argv[++i]);
				tolerance2=tolerance2*tolerance2;
				break;
		case 'w':	if (!strcmp(argv[i],"w8"))
					out_form=6;
				break;
		}
	return;
}

void inputreg()
{
	struct header hd;
	int		count;
	float 	*spec; 
	double	scalefactor;

	read_header(&hd);
	reg.ny=hd.orows;
	reg.nx=hd.ocols;
	npix=reg.nx*reg.ny;
	nframes=hd.num_frame;
	in_form=hd.pixel_format;
	out_form=PFFLOAT;
	count=10;
	spec=(float*)halloc(10,sizeof(float));
	if (findparam(&hd,"Gridspec")!=NULLPAR){
		getparam(&hd,"Gridspec",PFFLOAT,&count,&spec);
		reg.dx = spec[8];
		reg.dy = spec[9];
		reg.lx = spec[0] + 0.5*reg.dx;
		reg.ux = spec[1] - 0.5*reg.dx;
		reg.ly = spec[2] + 0.5*reg.dy;
		reg.uy = spec[3] - 0.5*reg.dy;
		scalefactor=((reg.ux-reg.lx)/reg.dx+1)/reg.nx;
		reg.dx*= scalefactor;	
		reg.dy*= scalefactor;	
		}
	else {
		reg.dx=reg.dy=1.0;
		reg.lx=reg.ly=0;
		reg.ux=reg.nx-1;
		reg.uy=reg.ny-1;
		}
	regptr=dvector(npix);
	return;
}


		
