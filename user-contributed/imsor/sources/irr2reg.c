/*      Copyright (c) 1992  Karsten Hartelius, IMSOR.

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   

 irr2reg - assigns the irregular distributed points in a HIPS-file of  
 irregular type to pixels in a regular HIPS-grid. 

 usage: see manual

 to load:
	cc -o irr2reg irr2reg.c -L /gsnk1/hips2/lib -lm -lhips

 to run:
     irr2reg  parameters  <IRR-file  > HIPS-file 

 include-files: util.h
*/

#include "util.h"

#define minval(x,y) ((x<y) ? x : y)
#define maxval(x,y) ((x>y) ? x : y)
#define epsilon 1E-9
#define MAXCOLS 10000
#define MAXROWS 10000

struct Limits {
	double	lx, ux, ly, uy;
};

enum pixel_evaluation { MINI, MEAN, MAXI, NUMB, INDIC };
enum neutral_evaluation { UNDER, OVER, MIDD, MEDI, SPEC };

enum pixel_evaluation  pixel_ev;
enum neutral_evaluation	neutral_ev;

Dvector	X,			/* x-coordinates of data-points  */
		Y,			/* y-coordinates " */ 
		V;			/* feature " */ 
Ivector	RowI,  		/* row index " */
		ColJ;		/* coloumn index " */
    
Dmatrix 	net;			/* value of image.  */
Imatrix 	num;			/* numbers of points in grid-pixel domains */

struct	Limits	
		data,		/* x,y limits of data-points */ 
		inclu,		/* x,y limits of data to be included in grid */
		grid;		/* x,y limits of grid */ 

double	sidelx,		/* horisontal sidelength of pixels */ 
		sidely,		/* vertical sidelength of pixels */
		neutral,		/* value of unused pixels */
		missing;		/* value of missing data  */    

Dvector 	neutrals; 	/* list of neutral values */

int  	nrows,		/* number of rows in grid */ 
		ncols,		/* number of coloumns in grid */ 
		Nframes,		/* number of frames */ 
		Ndata,		/* number of data-points  */
		NdataU,		/* number of data-points included in grid */ 
		Npix,		/* number of pixels */ 
		NpixU,		/* number of pixels containing data-points */
		in_form,		/* format of input */ 
		out_form,		/* format of output */
		Nextend ;		/* repititions of extension */ 

Boolean	extendflag,	/* pixels are extended to (unused) neighbors */ 
		extendknowns,  /* known pixels are also used in extension */
		square_pixels,	/* pixels are forced to bee square */
		mirror_y,		/* image is written upside-down */ 
		undersampled,	/* there are missing values in dataset */ 
		irregular,	/* data-points are really irregular distributed */
		defaultgridx,	/* x-limits of grid equal to limits of data */
		defaultgridy;	/* y-limits of grid equal to limits of data */

char*	flushfile;
FILE		*fp;			/* flush-file */

/* Functions */
void 	rparam(),
		wparam(),
 		inputcoord(),
		nextframe(),
		limits(),
		limits_update(),
		create_grid(),
		allocate_points(),
		calc_net(),
  		extend(),
 		scale(),
 		writehead();
int		limits_inside(),getpid();


int main(argc,argv)
int	argc;
char*	argv[];
{
	int	i;
	inputcoord();
	rparam(argc,argv);
	limits();
	create_grid();
	allocate_points();
	fprintf(stderr,"pixel-net created \n");
	writehead(argc,argv);
	for (i=0;i<Nframes;i++){
		nextframe();
  	  	calc_net();
	  	if (extendflag) extend();
	  	scale(i);
		fwrite_from_dmat(stdout,net,nrows,ncols,out_form);
		fprintf(stderr," frame nr %d written to output \n",i);
	  	}
	wparam(argc,argv);
	fprintf(stderr,"program parameters written to %s \n",flushfile);
	return(0);
}


void inputcoord() 
{
	byte val=0; 
	int	count=1;
	struct header hd;
	read_header(&hd);

	if (findparam(&hd,"Irregular") != NULLPAR)
		getparam(&hd,"Irregular",PFBYTE,&count,&val);
	if (val == 0)
		perr(HE_MSG,"File is not of type: Irregular");
     	
	Ndata = hd.ocols*hd.orows;
	Nframes = hd.num_frame-2; 
	in_form = out_form = hd.pixel_format;

	/* read coordinates */
	X=dvector(Ndata);
	fread_to_dvec(stdin,X,Ndata,in_form);
	Y=dvector(Ndata);
	fread_to_dvec(stdin,Y,Ndata,in_form);
	V=dvector(Ndata);
	neutrals=dvector(Nframes);
}  /* inputcoord */


void nextframe(){
	fread_to_dvec(stdin,V,Ndata,in_form);
}

void limits()
{
	int	i;
	/* domain of included data = domain of points inside specified
	   include-domain. */ 
	data.lx=data.ly=BIG;
	data.ux=data.uy=-BIG; 
	for (i=0;i<Ndata;i++)
	if ((X[i]>=inclu.lx) && (X[i]<=inclu.ux) && (Y[i]>=inclu.ly) && (Y[i]<=inclu.uy) )
		limits_update(&data,X[i],Y[i]);

	/* domain of grid  */
	if (defaultgridx){
		grid.lx = data.lx;
		grid.ux = data.ux;
		}
   	else 
		if ((grid.lx > data.lx) || (grid.ux < data.ux)) 
			perr(HE_MSG,"data exceed grid-domain"); 
	if (defaultgridy){
		grid.ly = data.ly;
		grid.uy = data.uy;
		}
   	else 
		if ((grid.ly > data.ly) || (grid.uy < data.uy)) 
			perr(HE_MSG,"data exceed grid-domain"); 
}


void  create_grid()
{
	if (irregular){
		if (sidelx == 0.0){
			if (square_pixels)
				sidelx=sidely=maxval( (grid.uy-grid.ly)/nrows,(grid.ux-grid.lx)/ncols ); 
			else{
				sidelx= (grid.ux-grid.lx)/ncols;
				sidely= (grid.uy-grid.ly)/nrows;
				}
			sidelx+= epsilon;
			sidely+= epsilon;
			}
		nrows = (int)((grid.uy-grid.ly)/sidely + 1);
		ncols = (int)((grid.ux-grid.lx)/sidelx + 1);
		grid.ux = grid.lx + ncols*sidelx;
		grid.uy = grid.ly + nrows*sidely;
		}
	else {
		if (sidelx == 0.0) {
			if (square_pixels)
				sidelx=sidely=maxval((grid.uy-grid.ly)/(nrows-1),(grid.ux-grid.lx)/(ncols-1));
			else{
				sidelx= (grid.ux-grid.lx)/(ncols-1);
				sidely= (grid.uy-grid.ly)/(nrows-1);
				}
		}
     	nrows = round( (grid.uy-grid.ly)/sidely + 1);
     	ncols = round( (grid.ux-grid.lx)/sidelx + 1);
		grid.lx = grid.lx - sidelx/2.0;
		grid.ly = grid.ly - sidely/2.0;
		grid.ux = grid.lx + ncols*sidelx;
		grid.uy = grid.ly + nrows*sidely;
		}

   	Npix = nrows*ncols;

	net=dmatrix(nrows,ncols);
	num=imatrix(nrows,ncols);
} 


void allocate_points()
{
	int	i, row;
	RowI=ivector(Ndata);
	ColJ=ivector(Ndata);
	for (i=0;i<Ndata;i++) RowI[i]=-1;
	for (i=0;i<Ndata;i++) ColJ[i]=-1;
	NdataU=0;
	for (i=0;i<Ndata;i++) if (limits_inside(inclu,X[i],Y[i])){
		NdataU++;
		row = (int)((Y[i] - grid.ly)/sidely);
		if (!mirror_y)
			row = nrows - 1 - row;
		RowI[i] = row;
		ColJ[i] = (int)((X[i] - grid.lx)/sidelx);
		}
}		


void calc_net()
{
	int 	i, j, r, c;
	double	val;

	switch (pixel_ev){
		case MINI	:	val=BIG; break;
		case MEAN	:	val=0; break;
		case MAXI	:	val=-BIG; break;
		case NUMB	:	val=0; break;
		case INDIC:	val=0; break;
		}
	for (i=0;i<nrows;i++) for (j=0;j<ncols;j++){
		net[i][j]=val;
		num[i][j]=0;
		}

	for (i=0;i<Ndata;i++) 
	if ((RowI[i] != -1) && (!undersampled || V[i] != missing)){
		r = RowI[i];
		c = ColJ[i];
		num[r][c]++;
		switch (pixel_ev){
			case MINI : net[r][c] = minval( net[r][c], V[i] ); break;
			case MEAN: net[r][c] += (V[i] - net[r][c])/num[r][c]; break;
			case MAXI : net[r][c] = maxval( net[r][c], V[i] ); break;
			case NUMB: net[r][c]++; break;
			case INDIC: net[r][c] = 1; break;
			}
		}
}


void extend()
{
	int 		i, j, r, c, rr, cc, n = Nextend;
	double	cval;
	Bmatrix  	used_pix;
	
	used_pix=(byte**)halloc(nrows+20,sizeof(byte *));
	used_pix += 10;
	for (i=-10;i<nrows+10;i++){
		used_pix[i]=(byte *)halloc(ncols+20,sizeof(byte));
		used_pix[i] += 10;
		for (j=-10;j<ncols+10;j++) used_pix[i][j]=9;
		}

	for (r=0;r<nrows;r++) for (c=0;c<ncols;c++)
		used_pix[r][c] = (num[r][c] == 0) ? 0 : 1; 

	for (r=0;r<nrows;r++) for (c=0;c<ncols;c++)
	if (used_pix[r][c]==1){
		cval = net[r][c];
		for (rr=r-n; rr<=r+n; rr++) for (cc=c-n; cc<=c+n;cc++)
		if ((used_pix[rr][cc]==0) || (extendknowns && used_pix[rr][cc]!=9)){
			num[rr][cc]++;
      		switch (pixel_ev){
		  	case MINI :	net[rr][cc] = minval( net[rr][cc], cval ); 
						break;
		  	case MEAN:	net[rr][cc]+= (cval - net[rr][cc])/num[rr][cc]; 
						break;
		  	case MAXI :	net[rr][cc] = maxval( net[rr][cc], cval ); 
						break;
			case NUMB:	net[rr][cc]++; break;
		  	case INDIC: 	net[rr][cc] = 1; break;
			}
		}
	}
}
		

void scale(frame) 
int	frame;
{
	int 	i, j, k;
	double	min=BIG, max=-BIG, meanval, median, sum=0, *list;
	void quicksort_doub();
  	NpixU=0;

	for (i=0;i<nrows;i++) for (j=0;j<ncols;j++) 
	if (num[i][j] > 0) {
		min = minval(min, net[i][j]);
		max = maxval(max, net[i][j]);
		sum+= net[i][j];
		NpixU++;
		}
  	meanval = sum/NpixU;
	/* if constant value of irregular points then let neutral=-1 */
	if (min==max)
		neutral=-1;
	else
		/* Pixels not used are assigned the neutral value  */
		switch (neutral_ev){
			case UNDER : neutral = min-(max-min)*0.005; break;
			case OVER : neutral = max+(max-min)*0.005; break;
			case MIDD : neutral = meanval; break;
			case MEDI : 
					{
						list=dvector(NpixU);
						k=0;
						for (i=0;i<nrows;i++) for (j=0;j<ncols;j++)
						if (num[i][j] > 0)
							list[k++] = net[i][j] - min + 1;
						quicksort_doub(list,0,NpixU-1);
						median = list[NpixU/2] + min - 1;
						neutral = min + (max-median);
					} break;
			case SPEC : neutral = neutral; break;
			}

	neutrals[frame] = neutral;
	for (i=0;i<nrows;i++) for (j=0;j<ncols;j++)
	if (num[i][j] == 0) 
		net[i][j] = neutral; 
}


void writehead(argc,argv)
int	argc;
char*	argv[];
{
	struct header hd;
	float val[10];
	init_header(&hd,"","",Nframes,"",nrows,ncols,out_form,1);
	update_header(&hd,argc,argv);
	val[0] = (float)grid.lx;
	val[1] = (float)grid.ux;
	val[2] = (float)grid.ly;
	val[3] = (float)grid.uy;
	val[4] = (float)data.lx;
	val[5] = (float)data.ux;
	val[6] = (float)data.ly;
	val[7] = (float)data.uy;
	val[8] = (float)sidelx;
	val[9] = (float)sidely;
	setparam(&hd,"Gridspec",PFFLOAT,10,val);
	write_header(&hd);
}


void rparam(argc,argv)
int	argc;
char* argv[];
{
	int		i;
	char		*get_logname();

	Progname =	strsave(*argv);
	neutral_ev =	MIDD; 
	pixel_ev =	MEAN; 
	mirror_y =	NO;
	undersampled =	NO;
	square_pixels= NO;
	irregular =	YES;
	extendflag =	NO;
	extendknowns = NO;
	Nextend =		1;
	nrows =		0;
	ncols =		0;
	sidelx =		0.0;
	sidely =		0.0;
	defaultgridx=	YES;
	defaultgridy=	YES;
	flushfile =	get_logname();
	out_form =	PFDOUBLE;	
	inclu.lx = 	-BIG;
	inclu.ux = 	 BIG;
	inclu.ly = 	-BIG;
	inclu.uy = 	 BIG;

	for (i=1;i<argc;i++) if (argv[i][0]=='-') 
	switch (argv[i][1]) {
		case 'r':	nrows=atoi(argv[++i]); break;

	    	case 'c': ncols=atoi(argv[++i]); break;

		case 'S':	square_pixels=TRUE;	break;

		case 's': sidelx=sidely=(double)atof(argv[++i]); break; 

		case 'a': irregular = NO; break;

		case 'i':	i++; 
				if ((i<argc) && (!strcmp(argv[i],"x"))){
					inclu.lx = (double)atof(argv[++i]);  
					inclu.ux = (double)atof(argv[++i]);  
					i++;
					}
				if ((i<argc) && (!strcmp(argv[i],"y"))){
					inclu.ly = (double)atof(argv[++i]);  
					inclu.uy = (double)atof(argv[++i]);  
					i++;
					}
				i--;
				break;

		case 'g':	i++; 
				if ((i<argc) && (!strcmp(argv[i],"x"))){
					defaultgridx = FALSE;
					grid.lx = (double)atof(argv[++i]);  
					grid.ux = (double)atof(argv[++i]);  
					i++;
					}
				if ((i<argc) && (!strcmp(argv[i],"y"))){
					defaultgridy = FALSE;
					grid.ly = (double)atof(argv[++i]);  
					grid.uy = (double)atof(argv[++i]);  
					i++;
					}
				i--;
				break;

		case 'm':	undersampled=YES;
				missing=(double)atof(argv[++i]); 
		    		break;

		case 'e':	extendflag=YES; 
				if (i<argc-1 && number(argv[i+1]))
					Nextend=atoi(argv[++i]); 
				break;

		case 'E':	extendflag=YES; 
				extendknowns=YES;	
				if (i<argc-1 && number(argv[i+1]))
					Nextend=atoi(argv[++i]); 
				break;

		case 'p':	pixel_ev=atoi(argv[++i]); 
				break;

		case 'n': i++;
				/* by default neutral_ev = MIDD  */
				switch (argv[i][0]){
					case 'u':	neutral_ev=UNDER; break;
					case 'o':	neutral_ev=OVER; break;
					case 'm':	neutral_ev=MEDI; break;
					default : neutral_ev=SPEC;
							neutral=(double)atof(argv[i]);
							break;
			 		} 
				break;

		case 'f':	flushfile = argv[++i];
				break;

		case 'y':	mirror_y = TRUE;
				break;

		case 'w': switch (argv[i][2]){
					case '4':	out_form=PFFLOAT; break;
					case '1':	out_form=PFBYTE; break;
					case '8':	out_form=PFDOUBLE; break;
					default : perr(HE_MSG,"wrong argument");
					}
				break;

		default :	perr(HE_MSG,"wrong parameter"); 
				break;
		}

	if ((sidelx == 0.0) && (nrows == 0) && (ncols == 0))
	perr(HE_MSG,"Number of rows and coloumns in grid not specified");

	if ((sidelx != 0.0) && (nrows > 0) && (ncols > 0))
	perr(HE_MSG,"double specification of grid");

	if ((pixel_ev==NUMB || pixel_ev==INDIC)){ 
		Nframes = 1;	
		neutral_ev = SPEC;
		neutral = 0; 
		}

	if ((sidelx==0.0)&&(ncols==0))
	ncols = nrows; 
	
	if ((sidely==0.0)&&(nrows==0))
	nrows = ncols;
} 




void quicksort_doub(a, l, r)
double	*a;
int		l, r;
{
	int i, j;
	double	v, t;
	if (r>l){
		v=a[r]; i=l-1; j=r;
		for (;;){
			while (a[++i] < v);
			while (a[--j] > v);
			if (i >= j) break;
			t=a[i]; a[i]=a[j]; a[j]=t;
			}
		t=a[i]; a[i]=a[r]; a[r]=t;
		quicksort_doub(a,l,i-1);
		quicksort_doub(a,i+1,r);
		}
}


void limits_update(rec, x, y)
struct Limits *rec;
double	x, y;
{
	if (x < (*rec).lx) (*rec).lx = x;
	else if (x > (*rec).ux) (*rec).ux = x;
	if (y < (*rec).ly) (*rec).ly = y;
	else if (y > (*rec).uy) (*rec).uy = y;
	return;
}

int limits_inside(rec, x, y)
struct Limits rec;
double	x, y;
{
	return ((x >= rec.lx) && (x < rec.ux) && (y >= rec.ly) && (y < rec.uy)); 
}


char *get_logname()
{
	int  number=getpid();
	char  *logname;
	logname=(char*)halloc(15,sizeof(char));
 	sprintf(logname,"i2r%d.log",number);
	return logname;
}

void wparam(argc,argv)
int	argc;
char*	argv[];
{
	int 	i;

	fp=fopen(flushfile,"w");
	fprintf(fp," IRR2REG \n");
	fprintf(fp," PROGRAM-PARAMETERS: \n");
	fprintf(fp,"\n");
	fprintf(fp," program called by : \n");
	for (i=0;i<argc;i++){ 
		fprintf(fp," %s ",argv[i]);
		if (i % 12 == 11) fprintf(fp,"\n");
		}
	fprintf(fp,"\n");
	fprintf(fp,"\n");

	fprintf(fp,"\n no.of rows         %d \n",nrows);
	fprintf(fp,"\n no.of columns      %d \n",ncols);
	fprintf(fp,"\n horisontal length of pixel  %f \n",sidelx);
	fprintf(fp,"\n vertical length of pixel  %f \n",sidely);
	fprintf(fp,"\n grid-domain  ");
	fprintf(fp,"\n   x : %lf - %lf   y : %lf - %lf \n",grid.lx, grid.ux, 
			grid.ly, grid.uy);
	fprintf(fp," \n data-domain  ");
	fprintf(fp,"\n   x : %lf - %lf   y : %lf - %lf \n",data.lx, data.ux,
			data.ly, data.uy);
	fprintf(fp,"\n");

	if (undersampled)
		fprintf(fp,"\n missing value in data-set :  %lf \n",missing); 
	if (mirror_y)
		fprintf(fp,"\n output image is mirrored in horisontal axis \n"); 
	if (extendflag){
		fprintf(fp,"\n pixel-values are extended to neighborhood");
		if (extendknowns)
		fprintf(fp,"\n (including center-pixel)");
		fprintf(fp,"\n   - order of neighborhood :  %d ",Nextend);
		fprintf(fp,"\n");
		}
	fprintf(fp,"\n datapoints         %d \n",Ndata);
	fprintf(fp,"\n datapoints used    %d \n",NdataU);
	fprintf(fp,"\n pixels             %d \n",Npix);
	fprintf(fp,"\n pixels used        %d \n",NpixU);
	fprintf(fp,"\n");

	fprintf(fp,"\n calculations-method of pixels :");
	switch (pixel_ev){
		case MINI : fprintf(fp,"  minimum of point-values \n"); break;
		case MEAN: fprintf(fp,"  mean of point-values\n"); break;
		case MAXI : fprintf(fp,"  maximum of point-values \n"); break;
		case NUMB: fprintf(fp,"  number of points in pixel-domain \n"); break;
		case INDIC:fprintf(fp,"  indication of points in pixel-domain \n"); break;
	}
	fprintf(fp,"\n");

	fprintf(fp,"\n value of unused pixels (neutral pixels) :");
	switch (neutral_ev){
		case UNDER : fprintf(fp,"  0.5 %% under data-span \n"); break;
		case OVER  : fprintf(fp,"  0.5 %% over data-span \n"); break;
		case MIDD  : fprintf(fp,"  mean-value of pixels \n"); break;
		case MEDI  : fprintf(fp,"  median-value of pixels \n"); break;
		case SPEC	 : fprintf(fp,"  value given by user \n"); break;
	}
	fprintf(fp,"\n");
	for (i=0;i<Nframes;i++){
		fprintf(fp,"    frame %d :  %lf \n", i, neutrals[i]);
		if (i % 12 == 11) fprintf(fp,"\n");
		}
	fclose(fp);
}    

