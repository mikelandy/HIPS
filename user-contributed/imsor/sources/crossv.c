/*      iopyright (c) 1992  Karsten Hartelius, IMSOR

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   

 crossv - calculates cross-variance or cross-semivariance matrices for
	  a dataset of irregular distributed points. It is also possible
	  to calculate cross-covariance between variables from two
	  different point-sets (=cova-functions).
	  Input is a HIPS-file of the IMSOR-defined Irregular format.
	  Output is the variance-matrices calculated for lags defined
	  by the user.

 usage: see manual

 to load: 
	cc  -o crossv crossv.c -lhips -lm

 to run:
	crossv parameters < HIPS-file   > HIPS-file  

 include-files: crossv.h , util.h . 

 Karsten Hartelius, august 1992.
*/

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <hipl_format.h>
#include <string.h>
#include <fcntl.h>
#include "crossv.h"

#ifndef CLOCKS_PER_SEC
#define  CLOCKS_PER_SEC 60
#endif

#define Byte  unsigned char
#define minval(x,y) ((x<y) ? x : y)
#define maxval(x,y) ((x>y) ? x : y)

enum interval_form{ A, B }; 
enum interval_form lag_form, dir_form;
Boolean 	not_missing();
int find_2Dlag(),getpid();

int main(argc,argv)
int	argc;
char	*argv[];
{
	void		rparam(), calc_lags(), calc_2Dlags(), calc_dirs(), wparam(), 
			block_dim(), write1_sigma(), write2_sigma();
	Dpt		input_data();
	Barray	calcblock();
	Spt		calcsigma();
	
	Progname=strsave(*argv);
	time1=clock()/(float)CLOCKS_PER_SEC;
	rparam(argc,argv);
	if (mode_2D)
		calc_2Dlags();
	else{
		calc_lags();
		if (Ndir > 1)
			calc_dirs();
		Ncov= Ndir*Nintv+1;
		}
	dhead = input_data();
	block_dim();
	bhead = calcblock(dhead,Ndata,blockstep,Nx,Ny,grid);	
	fprintf(stderr,"block-structure created \n");
	time2=clock()/(double)CLOCKS_PER_SEC;
	s = calcsigma(dhead,bhead);
	fprintf(stderr,"Matrices calculated \n");
	if (mode_2D)
		write2_sigma(argc,argv,s,out_format);
	else
		write1_sigma(argc,argv,s,out_format);
	time3=clock()/(float)CLOCKS_PER_SEC;
	wparam(argc,argv);
	return(0);
}


void rparam(argc,argv)
int  argc;
char  *argv[];
{
	int  	i, j ;
	char		*get_logname();

	Progname = 		strsave(*argv);
	variogram =	 	TRUE;
	covariance =		FALSE;
	covaflag =		FALSE;	
	mode_2D =			FALSE;
	mode_table =		FALSE;
	sigma0 =			FALSE;
	srivastava =		FALSE;
	trace = 			FALSE;
	write_drift = 		FALSE;
	write_spec =		TRUE;
	compas_angles =	FALSE;
	logdata =			FALSE;
	indicator = 		FALSE;
	lowlimit =		FALSE;
	undersampled = 	FALSE;
	leaveout =		FALSE;
	write_num =		FALSE;
	lag_cent =		FALSE;
	dir_cent = 		FALSE;
	lag_cov = 		0;
	dir_cov = 		0;
	blockdensity = 	0.2;
	out_format =		6;
	neutralval =		99;
	maxdist = 		-1.0; 
	subsmp = 			0.0; 
	Ndir = 			1;
	Nintv = 			1;
	logfname = 		get_logname(); 
	inclu.lx =		-BIG;
	inclu.ux =		 BIG;
	inclu.ly =		-BIG;
	inclu.uy =		 BIG;

	i=1;
	while (i<argc) if (argv[i][0]=='-') {
	switch (argv[i][1]) {
		case 'r': if (strcmp(argv[i],"-r")) perr(HE_MSG,"wrong argument");	
				maxdist=atof(argv[++i]); i++; 
				break;

		case 'b': if (strcmp(argv[i],"-b")) perr(HE_MSG,"wrong argument");	
				blockdensity=atof(argv[++i]); i++; 
				break;

		case 'a': if (strcmp(argv[i],"-a")) perr(HE_MSG,"wrong argument");	
				variogram=FALSE; 
				covariance=TRUE;
				i++; 
				break;

		case 't':	if (strcmp(argv[i],"-t")) perr(HE_MSG,"wrong argument");	
				trace=TRUE; 
				i++; 
				break;

		case '2': if (strcmp(argv[i],"-2D")) perr(HE_MSG,"wrong argument");
				mode_2D=TRUE;
				out_format=PFFLOAT;
				leaveout=TRUE;
				Nlag2Dx=atoi(argv[++i]);
				if (i<argc-1 && number(argv[i+1]))
					Nlag2Dy=atoi(argv[++i]);
				else Nlag2Dy=Nlag2Dx;
				i++;
				break;

		case '0':	if (strcmp(argv[i],"-0")) perr(HE_MSG,"wrong argument");
				sigma0=TRUE;
				i++;
				break;

		case 'c':	if (strcmp(argv[i],"-cova")) perr(HE_MSG,"wrong argument");
				covaflag=TRUE;
				variogram=FALSE;
				i++;
				covafname=argv[i++];
				break;

		case 's': if (strcmp(argv[i],"-sri")) perr(HE_MSG,"wrong argument");
				srivastava=TRUE;
				covariance=TRUE;
				variogram=FALSE;
				i++;
				break ;

		case 'l':	if (strcmp(argv[i],"-l") && strcmp(argv[i],"-ln"))
					perr(HE_MSG,"wrong argument");
				if (!strcmp(argv[i],"-l")){
					mode_table=TRUE;
					Nintv=atoi(argv[++i]);  
					lag_vec=dvector(Nintv+1);
					lag_form=A;
					i++;
					if ((i<argc)&&(number(argv[i]))){
						lag_form=B;
						for (j=1;number(argv[i]);j++)
							lag_vec[j]=(double)atof(argv[i++]);
						if (j!=Nintv+1) 
							perr(HE_MSG, "wrong number of lag-arguments");
						maxdist = lag_vec[Nintv];
						}
					if ((i<argc)&&(strcmp(argv[i],"c")==0)){
						lag_cent=TRUE;
						i++;
						if (i<argc && number(argv[i]))
							lag_cov=(double)atof(argv[i++]);
						}
					}
				else {
					logdata=TRUE; 
					i++;	
				}	
				break;

		case 'd':	if (strcmp(argv[i],"-d")) perr(HE_MSG,"wrong argument");
				mode_table=TRUE;
				Ndir=atoi(argv[++i]);  
				dir_vec=dvector(Ndir+1);
				dir_form=A;
				i++;
				if ((i<argc)&&(number(argv[i]))){
					dir_form=B;
					for (j=1;number(argv[i]);j++)
						dir_vec[j]=(double)atof(argv[i++]);
					if (j!=Ndir+1)  
						perr(HE_MSG, "wrong number of direction-arguments");
					}
				if ((i<argc)&&(strcmp(argv[i],"c")==0)){
					dir_cent=TRUE;
					i++;
					if (i<argc && number(argv[i]))
						dir_cov=(double)atof(argv[i++]);
					}
				break;

		case 'D': if (strcmp(argv[i],"-D")) perr(HE_MSG,"wrong argument");
				compas_angles=TRUE;
				i++;
				break;


		case 'i':	if (strcmp(argv[i],"-i")) perr(HE_MSG,"wrong argument");	
				i++;	
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
				break;

		case 'I':	if (strcmp(argv[i],"-I")) perr(HE_MSG,"wrong argument");	
				indicator=TRUE;
				i++;
				for (j=0;(i<argc)&&(number(argv[i]));i++,j++)
					detect_level[j]=(double)atof(argv[i]);
				Ndetect=j;
				break;

		case 'L':	if (strcmp(argv[i],"-L")) perr(HE_MSG,"wrong argument");	
				lowlimit=TRUE;
				i++;
 				for (j=0;(i<argc)&&(number(argv[i]));i++,j++)
					detect_level[j]=(double)atof(argv[i]);
				Ndetect=j;
				break;
			

		case 'e':	subsmp=(double)atof(argv[++i]); 
				i++; break;

		case 'n':	neutralval=(double)atof(argv[++i]);
				i++; break;

		case 'm':	undersampled=TRUE; 
				missingval=(double)atof(argv[++i]); 
				i++;
                	if ((i<argc)&&(!strcmp(argv[i],"skip"))){
					leaveout=TRUE;
					i++;
					}
				break;

		case 'f': logfname=argv[++i]; 
				i++; break;

		case 'w': switch (argv[i][2]){
					case 's': write_spec=FALSE; break;
					case 'd': write_drift=TRUE; break;
					case 'n': write_num=TRUE; break;
					case '4': out_format=PFFLOAT; break;
					}
				i++; break;

		default:
			perr(HE_MSG,"crossv -options < inseq  > outseq"); break;
		}
	}
	else perr(HE_MSG,"crossv -options < inseq >outseq"); 

	if (maxdist==-1)
		perr(HE_MSG,"no specification of neighborhood range.");
	if ((covaflag)&&(trace))
		perr(HE_MSG,"options -cova and -t are not possible at the same time"); 
	if (mode_2D && mode_table)
		perr(HE_MSG,"options -2D and -l,-d not possible at the same time");
	if (mode_2D && covaflag)
		perr(HE_MSG,"options -cova and -2D not possible at the same time");
	if (sigma0 && mode_2D && ((Nlag2Dx % 2 == 0) || (Nlag2Dy % 2 == 0)))
		perr(HE_MSG,"option -0 not valid when 2D-grid has even number of rows or cols"); 
	if (Nintv > MAXLAG) 
		perr(HE_MSG,"No of lags exceeds program upper limit\n");
	if (Ndir > MAXDIR) 
		perr(HE_MSG,"No of directions exceeds program upper limit\n");
} 


void calc_dirs()
{
	int		i;
	double	dir_step, wrap;
	void		shift_angles();

	dir_full= TRUE;
	switch (dir_form){
	case A:	dir_step= pi/Ndir;
			if (dir_cov>0.5)
				perr(HE_MSG,"coverage must not exceed 0.5");
			if (dir_cov==0)
				dir_size=dir_step;
			else {
				dir_size=dir_step*2*dir_cov;
				dir_full=FALSE;
				}
			if (!dir_cent)
				for (i=0;i<Ndir;i++)
					dir_vec[i]=(i+1)*dir_step;
			else{
				for (i=0;i<Ndir;i++)
					dir_vec[i]=i*dir_step + dir_size/2;
				}
			break;

	case B:	for (i=1;i<=Ndir;i++) 
				if (dir_vec[i]<0 || dir_vec[i]>180)
			perr(HE_MSG,"direction-intervals must lie within: [0;180]");
			for (i=1;i<=Ndir;i++) dir_vec[i]/=rad;
			if (!dir_cent)
				for (i=0;i<Ndir;i++) dir_vec[i]=dir_vec[i+1];
			else {
				if (!dir_cov){
					wrap=(dir_vec[1]+dir_vec[Ndir]-pi)/2;
					if (wrap>0){
						dir_vec[0]=wrap;
						for (i=1;i<Ndir;i++)
						dir_vec[i]=(dir_vec[i]+dir_vec[i+1])/2;
						}
					else{
						for (i=0;i<Ndir-1;i++)
						dir_vec[i]=(dir_vec[i+1]+dir_vec[i+2])/2;
						dir_vec[Ndir-1]=wrap+pi;
						}
					}
				else {
					dir_full=FALSE;
					dir_size=dir_cov/rad*2;
					wrap=dir_vec[Ndir]+dir_size/2-pi;
					if (wrap>0){
						dir_vec[0]=wrap;
						for (i=1;i<Ndir;i++)
							dir_vec[i]=dir_vec[i]+dir_size/2;
						}
					else {
						for (i=0;i<Ndir-1;i++)
							dir_vec[i]=dir_vec[i+1]+dir_size/2;
						dir_vec[Ndir-1]=wrap+pi;
						}
					}
				}
			break;
			}
}


void calc_2Dlags()
{
	lag2D_sizex=maxdist*2/Nlag2Dx;
	lag2D_sizey=maxdist*2/Nlag2Dy;
	centerblock=(Nlag2Dy/2)*Nlag2Dx + Nlag2Dx/2;
	Ncov=Nlag2Dx*Nlag2Dy;
	return;
}


void calc_lags()
{
	int	i;

	double lag_step;
	lag_full= TRUE;
	switch (lag_form){
	case A:	lag_step= maxdist/Nintv;
			if (lag_cov>0.5)
				perr(HE_MSG,"coverage must not exceed 0.5");
			lag_size = (lag_cov==0) ? lag_step : lag_step*2*lag_cov;
			if (!lag_cent)
				for (i=0;i<Nintv;i++)
					lag_vec[i]=(i+1)*lag_step;
			else{
				Nintv++;
				for (i=0;i<Nintv;i++)
					lag_vec[i]=i*lag_step + lag_size/2;
				maxdist+= lag_size/2;
				lag_full=FALSE;
				}
			break;

	case B:	if (!lag_cent)
				for (i=0;i<Nintv;i++) lag_vec[i]=lag_vec[i+1];
			else {
				lag_vec[0]=0;
				if (!lag_cov){
					for (i=0;i<Nintv;i++)
						lag_vec[i]=(lag_vec[i]+lag_vec[i+1])/2;
					lag_vec[Nintv]+=lag_vec[Nintv]-lag_vec[Nintv-1];
					maxdist=lag_vec[Nintv];
					Nintv++;
					}
				else {
					lag_size=lag_cov*2;
					Nintv++;
					for (i=0;i<Nintv;i++)
						lag_vec[i]+=lag_size/2;
					maxdist+=lag_size/2;
					lag_full=FALSE;
					}
				}
			break;
			}				
}


Dpt input_data()
{
	int  	i, j, inform1, inform2, count;
	byte		val;
	Dmatrix 	a;
	struct 	header hd;
	FILE		*covafile;
	Ivector	N;
		
	Ndata=Ndata1=Ndata2=Nvar=Nvar1=Nvar2=0;
	read_header(&hd);

	val=0; count=1;
     if (findparam(&hd,"Irregular") != NULLPAR)
		getparam(&hd,"Irregular",PFBYTE,&count,&val);
	if (val == 0)
		perr(HE_MSG,"File is not of type: Irregular");

	Ndata1=Ndata2=Ndata=hd.ocols;
	Nvar1=Nvar2=Nvar=hd.num_frame-2;
	inform1=hd.pixel_format;

	if (covaflag){
		covafile=hfopenr(covafname);
		fread_header(covafile,&hd,covafname);
		val=0; count=1;
     	if (findparam(&hd,"Irregular") != NULLPAR)
			getparam(&hd,"Irregular",PFBYTE,&count,&val);
		if (val == 0)
			perr(HE_MSG,"File is not of type: Irregular");
		Ndata2=hd.ocols;
		Nvar2=hd.num_frame-2;
		inform2=hd.pixel_format;
		Ndata=Ndata1+Ndata2;
		Nvar=Nvar1+Nvar2;
		}

	if (Nvar>MAXFEAT)
		perr(HE_MSG,"Number of features exceeds MAXFEAT=200");

	/* in case of cova: The two dataset are placed after one another
	   in dhead. "breakpoint" sets the boundary between the two datasets.*/
	breakpoint=Ndata1-0.5;

	/* declare and read data-structure */
	a=dmatrix(Nvar1+2,Ndata1);
	dhead=(Dpt)halloc(Ndata,sizeof(point));
	for (i=0;i<Ndata1;i++)
		dhead[i].v=dvector(Nvar1);
	for (i=Ndata1;i<Ndata;i++)
		dhead[i].v=dvector(Nvar2);

	fread_to_dmat(stdin,a,Nvar1+2,Ndata1,inform1);

	/* data are logaritmized if flag is set */ 
	if (logdata)
		for (i=2;i<Nvar+2;i++) for (j=0;j<Ndata;j++)
			a[i][j] = log(a[i][j]);

	/* data are transformed to indicator-variables */
	if ((indicator)&&(Nvar!=Ndetect))
		perr(HE_MSG,"Number of detection-levels in option I is not correct");
	if (indicator)
		for (i=2;i<Nvar+2;i++) for (j=0;j<Ndata;j++)
		if (not_missing(a[i][j]) )
			a[i][j] = (a[i][j]<detect_level[i-2]) ? 0.0 : 1.0;

	/* data with values below detection-limit are set missing */
	if ((lowlimit)&&(Nvar!=Ndetect))
		perr(HE_MSG,"Number of limits in option L is not correct");
	if (lowlimit)
		for (i=2;i<Nvar+2;i++) for (j=0;j<Ndata;j++)
		if (not_missing(a[i][j]))
		a[i][j]=(a[i][j]<detect_level[i-2]) ? missingval : a[i][j];


	mean=dvector(Nvar);
	for (i=0;i<Nvar;i++) mean[i]=0;
	N=ivector(Nvar);
	for (i=0;i<Nvar;i++) N[i]=0;
	mainf.lx = mainf.ly = BIG;
	mainf.ux = mainf.uy = -BIG;
	for (i=0;i<Ndata1;i++){
		/* fill in coordinates while searching lower and upper limits*/
		dhead[i].no=i;
		dhead[i].x=a[0][i];
		mainf.lx=minval(a[0][i],mainf.lx);
		mainf.ux=maxval(a[0][i],mainf.ux);
		dhead[i].y=a[1][i];
		mainf.ly=minval(a[1][i],mainf.ly);
		mainf.uy=maxval(a[1][i],mainf.uy);
		for (j=0;j<Nvar1;j++){
			dhead[i].v[j]=a[j+2][i];
			if (not_missing(a[j+2][i])){
				mean[j]+=a[j+2][i];
				N[j]++;
				}
			}
		}

	free_dmatrix(a,Nvar1+2);

	cova.lx=cova.ly=BIG;
	cova.ux=cova.uy=-BIG;
	if (covaflag){
		/* The cova-dataset is read and placed in dhead after the
		   std.input dataset. */
		a=dmatrix(Nvar2+2,Ndata2);
		fread_to_dmat(covafile,a,Nvar2+2,Ndata2,inform2);
		for (i=0;i<Ndata2;i++){
			dhead[Ndata1+i].no=Ndata1+i;
			dhead[Ndata1+i].x=a[0][i];
			cova.lx=minval(a[0][i],cova.lx);
			cova.ux=maxval(a[0][i],cova.ux);
			dhead[Ndata1+i].y=a[1][i];
			cova.ly=minval(a[1][i],cova.ly);
			cova.uy=maxval(a[1][i],cova.uy);
			for (j=0;j<Nvar2;j++){
				dhead[Ndata1+i].v[j]=a[j+2][i];
				/* updating mean-vector */
				if (not_missing(a[j+2][i])){
					mean[j+Nvar1]+=a[j+2][i];
					N[j+Nvar1]++;
					}
				}
			}
	free_dmatrix(a,Nvar2+2);
	fclose(covafile);
	}
	/* dividing the mean-vector by the number of points */
	for (j=0;j<Nvar;j++) 
		mean[j]/=N[j];

	/* Minimum and maximum limits in both datasets together are found */
	total.lx=minval(mainf.lx,cova.lx);
	total.ly=minval(mainf.ly,cova.ly);
	total.ux=maxval(mainf.ux,cova.ux);
	total.uy=maxval(mainf.uy,cova.uy);

	return(dhead);
} /* endofinput*/



void block_dim()
{
	double	blockarea;
	
	grid=dvector(4);
	blockarea=(total.ux-total.lx)*(total.uy-total.ly)/Ndata*blockdensity; 
	blockstep=maxval(sqrt(blockarea), maxdist/10);

	Nx = (total.ux-total.lx)/ blockstep + 1;
	Ny = (total.uy-total.ly)/ blockstep + 1;
	grid[0]= total.lx;
	grid[1]= total.lx + Nx*blockstep;
	grid[2]= total.ly;
	grid[3]= total.ly + Ny*blockstep;
}



Spt calcsigma(dhead,bhead)
Dpt  dhead;
Barray  bhead;
{
	int    	i, j, l, 
			nr,			/* matrix number */
			prcount,		/* counter */ 
			a,			/* horisontal block-index */ 
			b,			/* vertical block-index */ 
		  	mina, minb, maxa, maxb, 	/* search-area in block-array */ 
		  	lag, 		/* lag-number */
			find_lag(),	/* finds lag-number */ 
			dir,			/* direction-number */ 
			find_dir();	/* findes direction-number */ 

	double 	dist,       	/* distance */
		  	angle,      	/* angle, x-axis is angle=0, y-axis is angle=pi */
		  	x0, y0,     	/* center-points */
		  	subsmp_limit; 	/* level of point-pair exclusion [0;1] */
	Spt    	s;        	/* list of "Sigma"-matricer */ 
	Dpt    	dp;			/* pointer to center-point */
	Bpt    	bp;			/* pointer to neighbor-point */
	double 	calcdist();
	void		addcrossv();
	Boolean	halfcirk(),	/* neighbor-point placed in upper halfplane */ 
			ok_feature(),	/* no missing values in feature-vector */ 
			within_incl(),	/* point placed inside include domain */ 
			same_dataset(),/* points belong to same dataset (in case of cova)*/ 
			subsamp();	/* subsampling in point-pairs */

	srand48(Seedval);
	/* initialisering af sigma-matricer */
	s = (Spt)halloc(Ncov,sizeof(sigma));
	for (l=0;l<Ncov;l++){
		s[l].Npairs=0;
		s[l].meandist=0.0;
		s[l].meandir=0.0;
		s[l].m_minus=dmatrix(Nvar,Nvar);
		for (i=0;i<Nvar;i++) for (j=0;j<Nvar;j++) s[l].m_minus[i][j]=0;
		s[l].m_plus=dmatrix(Nvar,Nvar);
		for (i=0;i<Nvar;i++) for (j=0;j<Nvar;j++) s[l].m_plus[i][j]=0;
    		s[l].drift=dvector(Nvar);
		for (i=0;i<Nvar;i++) s[l].drift[i]=0;
		s[l].nfeat=ivector(Nvar);
		for (i=0;i<Nvar;i++) s[l].nfeat[i]=0;
		s[l].num=dmatrix(Nvar,Nvar);
		for (i=0;i<Nvar;i++) for (j=0;j<Nvar;j++) s[l].num[i][j]=0;
    		if (l==0) 
			s[l].dist=s[l].dir=0;
	     else {
    		 	s[l].dist=lag_vec[(l-1)/Ndir];
       		s[l].dir=dir_vec[(l-1)%Ndir];
       		}
		if (trace){
			s[l].S=dmatrix(1,Nvar);
			for (j=0;j<Nvar;j++) 
				s[l].S[0][j]=0;
			}
		else{ 
			s[l].S=dmatrix(Nvar,Nvar);
			for (i=0;i<Nvar;i++) for (j=0;j<Nvar;j++)
				s[l].S[i][j]=0;
			}
    		}

/*	subsmp_limit=maxdist-sqrt(subsmp*maxdist*maxdist); */
	subsmp_limit=(1-subsmp)*maxdist;

	Nsearch=Nused=prcount=0;
	for (i=0;i<Ndata;i++) if ((ok_feature(&dhead[i])) && (within_incl(i))) {
		if (i*100/Ndata > prcount){
			fprintf(stderr,"crossv: %2d%% done \r",(i+1)*100/Ndata);
			prcount++;
			}
		dp=&dhead[i];
		x0=dp->x; 
		y0=dp->y;
    		/* Search is done in blocks which contains the "upper" half of the 
    		   	neighborhood-circle (radius = maxdist) */ 
		mina = maxval( ((x0 - maxdist - total.lx) / blockstep), 0);
		minb = (y0 - total.ly) / blockstep;
		maxa = minval( ((x0 + maxdist - total.lx) / blockstep), Nx-1); 
    		maxb = minval( ((y0 + maxdist - total.ly) / blockstep), Ny-1); 
     
    		for (a=mina;a<=maxa;a++) for (b=minb;b<=maxb;b++){
		     bp=bhead[a][b];
		     while (bp != NULL){ 
				Nsearch++;
				dist=calcdist(dp, bp->point);
				if ((dist<=maxdist || mode_2D) && 
				(halfcirk(dp,bp->point)) && 
				(subsamp(dist,subsmp_limit)) && 
				(ok_feature(bp->point)) &&
				((!covaflag) || (!same_dataset(i,bp->point->no))) &&
				(within_incl(bp->point->no))){
					if (mode_2D)
						nr=find_2Dlag(bp->point->x - dp->x, bp->point->y - dp->y);
					else {
	 					lag=find_lag(dist);
				    		dir=find_dir(dist,&angle,dp,bp->point);
				    		if ((lag>-1) && (dir>-1))
							nr= (lag==0) ? 0 : Ndir*(lag-1) + (dir-1)+1;
						else nr=-1;
						}
					if (nr>=0)
						addcrossv(s,nr,dist,angle,dp,bp->point);
					}
				bp=bp->next;
		          }
			}
		} /* for i=0 to Ndata */

	for (l=0;l<Ncov;l++) {
		if (s[l].Npairs>0)
			s[l].meandist= s[l].meandist/s[l].Npairs;
		else 
			s[l].meandist=neutralval;
		if (s[l].Npairs>0)
			s[l].meandir= s[l].meandir/s[l].Npairs;
		else s[l].meandir=neutralval;

		if ((variogram) && (l>0 || mode_2D)){
			for (i=0;i<Nvar;i++)
				if (s[l].num[i][i]>0)
					s[l].drift[i] /= s[l].num[i][i];
				else 
					s[l].drift[i]=neutralval;
			if (!trace)
				for (i=0;i<Nvar;i++) for (j=(i+1);j<Nvar;j++){
					s[l].num[i][j] = s[l].num[j][i];
					s[l].S[i][j] = s[l].S[j][i];
					}
			}

	
		if ((covariance) || (covaflag) || (l==0 && !mode_2D)) /* covariance */
			for (i=0;i<Nvar;i++){
				if (s[l].nfeat[i]>0)
					s[l].drift[i] /= s[l].nfeat[i];
				else 
					s[l].drift[i]=neutralval;
				if (srivastava){
					for (j=0;j<Nvar;j++)
						if (s[l].num[i][j] > 0) {
							s[l].m_minus[i][j] /= s[l].num[i][j];
							s[l].m_plus[i][j] /= s[l].num[i][j];
							}
					}
				}

		if ((variogram) && (l>0 || mode_2D)){
			/* cross-semivariogram is symmetric */
			if (!trace){
				for (i=0;i<Nvar;i++) for (j=0;j<Nvar;j++)
					if (s[l].num[i][j]>0)
						s[l].S[i][j] /= s[l].num[i][j];
					else
						s[l].S[i][j] = neutralval;	
				}
			else {			
				for (i=0;i<Nvar;i++)
				if (s[l].num[i][i]>0)
					s[l].S[0][i] /= s[l].num[i][i];
				else
					s[l].S[0][i] = neutralval;	
				}
			}
				
		
		if ((covariance) || (covaflag) || (l==0 && !mode_2D)){
			if (!trace){
			for (i=0;i<Nvar;i++) for (j=0;j<Nvar;j++)
				if (s[l].num[i][j]>0)
					s[l].S[i][j] = s[l].S[i][j]/s[l].num[i][j] -
								s[l].m_minus[i][j]*s[l].m_plus[i][j];
				else  
					s[l].S[i][j] = neutralval;	
				}
			else {			
				for (i=0;i<Nvar;i++)
				if (s[l].num[i][i]>0)
					s[l].S[0][i] = s[l].S[0][i]/s[l].num[i][i] -
								s[l].m_minus[i][i]*s[l].m_plus[i][i];
				else
					s[l].S[0][i] = neutralval;	
				}
			}

		if (covaflag)
			/* make average of i,j and j,i values and put sub-matrix 
			   i nrows: 0..Nvar1-1 and coloumns: 0..Nvar2-1. */
			for (i=0;i<Nvar1;i++) for (j=0;j<Nvar2;j++)
				if (s[l].num[i][j]>0)
					s[l].S[i][j] = 
							(s[l].S[i][Nvar1+j]+s[l].S[Nvar1+j][i])/2; 
	

    		} /* for l = 1,..,Ncov */
	return(s);
}


int find_2Dlag(dx,dy)
double	dx, dy;
{
	int	row, col, blocknr, move_to_cross();

	if (dx==0 && dy==0) {
		if (!sigma0) 
			return(-1);
		else return(centerblock);
	}

	if (dx<-maxdist || dx>maxdist || dy<0 || dy>maxdist)
		return(-1);
	dx+=maxdist-DOUBLE_EPS;
	dy+=maxdist- DOUBLE_EPS;
	row=Nlag2Dy - 1 - (int)(dy/lag2D_sizey);
	col=(int)(dx/lag2D_sizex);
	blocknr= row*Nlag2Dx + col;
	if (blocknr==centerblock && sigma0)
		blocknr=move_to_cross((dx-maxdist)/lag2D_sizex,(dy-maxdist)/lag2D_sizey,centerblock,Nlag2Dx);
	return blocknr;
}


int move_to_cross(dx,dy,center,Nx)
double	dx, dy;
int		center, Nx;
{
	int pixel;
	if (dx<0)
		pixel=((0-dx)>dy) ? center-1 : center-Nx;
	else
		pixel=(dx>dy) ? center+1 : center-Nx;
	return pixel;
}


int find_lag(dist)
double dist;
{
	int  i; 
	if (dist==0) return(0);    
	for (i=0;dist>lag_vec[i];i++);
	if (lag_full || dist>=lag_vec[i]-lag_size) 
		return(i+1);
	else
		return(-1);
}


int find_dir(dist,angle,p,q)
double  	dist, *angle;
Dpt		p, q;
{
	int	i;
	if (dist==0){
		*angle=0;
		return(0);
		}
	*angle=atan2(q->y-p->y, q->x-p->x);
	if (compas_angles)
		*angle= (*angle<pi/2) ? 0.5*pi - (*angle) : 1.5*pi - (*angle); 
	if (Ndir==1) return(1);
	for (i=0;(i<Ndir) && (*angle>dir_vec[i]);i++);
	if (i==Ndir){
		i=0; *angle-=pi; }
	if (dir_full || *angle>=dir_vec[i]-dir_size)
		return(i+1);
	else
		return(-1);
}
	

void addcrossv(s,nr,dist,angle,p,q)
Spt  	s;		/* variance-structure */
double  	dist,	/* distance between points */ 
		angle;	/* angle between points */
int 		nr;		/* variance-matrix number */ 
Dpt  	p,		/* pointer to center */ 
		q;		/* pointer to neighbor */
{
	int  	i, j;
	Boolean	complete();

	Nused++;
	s[nr].Npairs++;
	s[nr].meandist += dist;
	s[nr].meandir += angle;

	if (covaflag){
		/* p belongs to the first dataset (0..Nvar-1) */
		if (p->no<breakpoint){
			for (i=0;i<Nvar1;i++) if (not_missing(p->v[i])){
				s[nr].nfeat[i]++;
				s[nr].drift[i] += p->v[i];
				}
			for (i=0;i<Nvar1;i++) for (j=0;j<Nvar2;j++) 
				if (complete(i,j,p,q)){
					s[nr].num[i][Nvar1+j] += 1.0;
					s[nr].S[i][Nvar1+j]+= (p->v[i]-mean[i])*(q->v[j]-mean[Nvar1+j]);
					}
			}
		else { 
		/* p belongs to the second dataset (Nvar1..Nvar-1) */
			for (i=0;i<Nvar2;i++) if (not_missing(p->v[i])){
				s[nr].nfeat[Nvar1+i]++;
				s[nr].drift[Nvar1+i] += p->v[i];
				}
			for (i=0;i<Nvar1;i++) for (j=0;j<Nvar2;j++) 
				if (complete(i,j,q,p)){
					s[nr].num[Nvar1+j][i] += 1.0;
					s[nr].S[Nvar1+j][i]+= (p->v[j]-mean[Nvar1+j])*(q->v[i]-mean[i]);
					}
			}
	}
	
	else if ((variogram) && (nr>0 || mode_2D))  {   
	/* semivariogram is calculated, except in lag 0, if mode_2D is not chosen*/
		for (i=0;i<Nvar;i++) if (complete(i,i,p,q))
			s[nr].drift[i] += (p->v[i] - q->v[i]);
		if (trace) 
			for (i=0;i<Nvar;i++) if (complete(i,i,p,q)){
				s[nr].num[i][i] += 1.0;
				s[nr].S[0][i]+= (p->v[i]-q->v[i])*(p->v[i]-q->v[i])/2;
				}
		if (!trace)
			for (i=0;i<Nvar;i++) for (j=0;j<=i;j++) 
			if (complete(i,j,p,q)){
				s[nr].num[i][j] += 1.0;
            	    	s[nr].S[i][j]+= (p->v[i]-q->v[i])*(p->v[j]-q->v[j])/2;
				}
	}  /* end calculation of cross-semivariogram */

	else  {  /* Cross-covariance is calculated */ 
		for (i=0;i<Nvar;i++) if (not_missing(p->v[i])){
			s[nr].nfeat[i]++;
			s[nr].drift[i] += p->v[i];
			}
		if (trace)
			for (i=0;i<Nvar;i++) if (complete(i,i,p,q)){
				s[nr].num[i][i] += 1.0;
				if (srivastava){
					s[nr].S[0][i]+= p->v[i]*q->v[i];
					s[nr].m_minus[i][i] += p->v[i];
					s[nr].m_plus[i][i] += q->v[i];
					}
				else
					s[nr].S[0][i]+= (p->v[i]-mean[i])*(q->v[i]-mean[i]);
				}
		if (!trace)
			for (i=0;i<Nvar;i++) for (j=0;j<Nvar;j++) 
			if (complete(i,j,p,q)){
				s[nr].num[i][j] += 1.0;
				if (srivastava){
					s[nr].S[i][j]+= p->v[i]*q->v[j];
					s[nr].m_minus[i][j] += p->v[i];
					s[nr].m_plus[i][j] += q->v[j];
					}
				else
					s[nr].S[i][j]+= (p->v[i]-mean[i])*(q->v[j]-mean[j]);
					
				}
	} /*end calculation of crossv-covariance */

} /* end addcrossv */



void addcrossv2(s,lag,dir,dist,angle,p,q)
Spt  s;
double  dist, angle;
int  lag, dir;
Dpt  p, q;
{
	int  	nr, i, j;
	Boolean	complete();

	Nused++;
	nr= (lag==0) ? 0 : Ndir*(lag-1) + (dir-1) + 1;

	s[nr].Npairs++;
	s[nr].meandist += dist;
	s[nr].meandir += angle;
	for (i=0;i<Nvar1;i++) for (j=0;j<Nvar2;j++)
		if (complete(i,j,p,q)) 
			s[nr].num[i][j] += 1.0;

	if (covaflag){
		/* p belongs to the first dataset (0..Nvar-1) */
		if (p->no<breakpoint){
			for (i=0;i<Nvar1;i++) for (j=0;j<Nvar2;j++) 
				if (complete(i,j,p,q))
					s[nr].S[i][j]+= (p->v[i]-mean[i])*(q->v[j]-mean[Nvar1+j]);
			}
		else 
		/* p belongs to the second dataset (Nvar1..Nvar-1) */
			for (i=0;i<Nvar1;i++) for (j=0;j<Nvar2;j++) 
				if (complete(i,j,q,p))
					s[nr].S[i][j]+= (p->v[j]-mean[Nvar1+j])*(q->v[i] - mean[i]);
	}
	
	else if ((variogram) && (nr>0)){   
	/* semivariogram is calculated, except in lag 0 */
		for (i=0;i<Nvar;i++) if (complete(i,i,p,q))
			s[nr].drift[i] += (p->v[i] - q->v[i]);
		if (trace) 
			for (i=0;i<Nvar;i++) if (complete(i,i,p,q))
				s[nr].S[0][i]+= (p->v[i]-q->v[i])*(p->v[i]-q->v[i])/2;
		if (!trace)
			for (i=0;i<Nvar;i++) for (j=0;j<Nvar;j++) 
			if (complete(i,j,p,q))
            	    	s[nr].S[i][j]+= (p->v[i]-q->v[i])*(p->v[j]-q->v[j])/2;
	}  /* end calculation of cross-semivariogram */

	else  {  /* Cross-covariance is calculated */ 
		for (i=0;i<Nvar;i++) if (not_missing(p->v[i]))
			s[nr].nfeat[i]++;
			s[nr].drift[i] += p->v[i];
		if (trace)
			for (i=0;i<Nvar;i++) if (complete(i,i,p,q))
				s[nr].S[0][i]+= (p->v[i]-mean[i])*(q->v[i]-mean[i]);
		if (!trace)
			for (i=0;i<Nvar;i++) for (j=0;j<Nvar;j++) 
			if (complete(i,j,p,q))
				s[nr].S[i][j]+= (p->v[i]-mean[i])*(q->v[j]-mean[j]);
	} /*end calculation of crossv-covariance */

} /* end addcrossv2 */



Boolean subsamp(dist,limit)
double  dist, limit;
{ 
	double  drand48();	
	return((drand48() < limit/dist)); 
}


Boolean same_dataset(i,j)
/* This function returns TRUE if the two indices belong to the same dataset */
int	i, j;
{
  return( ((i<breakpoint)&&(j<breakpoint)) || ((i>breakpoint)&&(j>breakpoint)));
}


Boolean ok_feature(p)
Dpt  p;
{
	int  i;
	if (!undersampled) 
		return TRUE; 
	if (!leaveout){
		for (i=0;i<Nvar;i++)
			if (not_missing(p->v[i])) 
				return TRUE;
	return FALSE;
	}
	/* skip whole observation if missing value is present */
	for (i=0;i<Nvar;i++)
		if (!not_missing(p->v[i])) 
			return FALSE;
	return TRUE;
}

Boolean not_missing(val)
double	val;
{
	return((!undersampled)||(val!=missingval));
}


Boolean within_incl(i)
int	i;
{
	return ((dhead[i].x >= inclu.lx) && (dhead[i].x <= inclu.ux) &&
			(dhead[i].y >= inclu.ly) && (dhead[i].y <= inclu.uy));
}


Boolean complete(i,j,p,q)
int	i, j;
Dpt	p, q;
{   
	return( (not_missing(p->v[i]))&&(not_missing(q->v[j])) ); 
}

	
		
double calcdist(p,q)
Dpt   p, q;
{
	double  dist;
	dist = sqrt((p->x - q->x)*(p->x - q->x) + 
             (p->y - q->y)*(p->y - q->y)) ;  
	if (dist<EPSILON) dist=0;
	return(dist);
}
	

Boolean halfcirk(p,q)
Dpt  p, q;
{  
	if (q->y > p->y) 
		return(1); 
	else
		if ((p->y == q->y) && (q->x <= p->x)) 
			return(1);
     return(0);
}


void write2_sigma(argc,argv,s,out_format)
int  argc, out_format;
char *argv[];
Spt  s;
{  
	int		i, j, r, c, r2, c2, f;
	struct 	header outhd;
	Fmatrix	frame;		

	Ncov= (trace) ? Nvar2 : (float)sqr(Nvar2)/2+(float)Nvar2/2;
	if (write_num)
		Ncov+=1;
	if (write_drift)
		Ncov+=Nvar2;

   	init_header(&outhd,"","",Ncov,"",Nlag2Dy,Nlag2Dx,out_format,1,"");
	if (variogram) 
		setparam(&outhd,"Semivariance",PFBYTE,1,1);
	if (covaflag) 
   		setparam(&outhd,"Cova",PFBYTE,1,1);
	if (covariance)	
   		setparam(&outhd,"Covariance",PFBYTE,1,1);
	outhd.seq_desc="Each frame contains a 2D cross-variance image. frame 0: variable 0 vs. 0, frame 1: variable 1 vs. 0, frame 2: variable 1 vs. 1, frame 3: variable 2 vs. 0 , etc"; 
	update_header(&outhd,argc,argv);
	write_header(&outhd);

	frame=fmatrix(Nlag2Dy,Nlag2Dx);

	/* write number-frame */
	if (write_num){
		for (r=0;r<Nlag2Dy/2;r++) for (c=0;c<Nlag2Dx;c++)
			frame[r][c]=(float)s[r*Nlag2Dx+c].num[0][0];
		if (Nlag2Dy % 2 ==1){
			r2=Nlag2Dy/2;
			for (c=0;c<Nlag2Dx;c++)
				frame[r2][c]= (float)(s[r2*Nlag2Dx+c].num[0][0]+
						s[(r2+1)*Nlag2Dx-c-1].num[0][0]);
			if ((Nlag2Dx % 2 ==1) && (sigma0))
				frame[r2][Nlag2Dx/2]/=2;
			}
		for (r=(Nlag2Dy+1)/2;r<Nlag2Dy;r++) for (c=0;c<Nlag2Dx;c++){
			r2=Nlag2Dy-1-r;
			c2=Nlag2Dx-1-c;
			frame[r][c]=(float)s[r2*Nlag2Dx+c2].num[0][0];
			}
		fwrite_from_fmat(stdout,frame,Nlag2Dy,Nlag2Dx,out_format);
		}
	if (trace)
		for (i=0;i<Nvar2;i++){
			for (r=0;r<Nlag2Dy/2;r++) for (c=0;c<Nlag2Dx;c++)
				frame[r][c]=(float)s[r*Nlag2Dx+c].S[0][i];
			if (Nlag2Dy % 2==1){
				r2=Nlag2Dy/2;
				for (c=0;c<(Nlag2Dx+1)/2;c++)
				frame[r2][c]=frame[r2][Nlag2Dx-1-c]=
					(float)(s[r2*Nlag2Dx+c].S[0][i]);
				}
			for (r=(Nlag2Dy+1)/2;r<Nlag2Dy;r++) for (c=0;c<Nlag2Dx;c++){
				r2=Nlag2Dy-1-r;
				c2=Nlag2Dx-1-c;
				frame[r][c]=(float)s[r2*Nlag2Dx+c2].S[0][i];
				}
			fwrite_from_fmat(stdout,frame,Nlag2Dy,Nlag2Dx,out_format);
			}
	else	{ 
		for (i=0;i<Nvar2;i++){
			for (r=0;r<Nlag2Dy/2;r++) for (c=0;c<Nlag2Dx;c++)
				frame[r][c]=(float)s[r*Nlag2Dx+c].S[i][i];
			if (Nlag2Dy % 2==1){
				r2=Nlag2Dy/2;
				for (c=0;c<Nlag2Dx;c++)
					frame[r2][c]= (float)((s[r2*Nlag2Dx+c].S[i][i]+
						s[(r2+1)*Nlag2Dx-c-1].S[i][i])/2.0);
				}
			for (r=(Nlag2Dy+1)/2;r<Nlag2Dy;r++) for (c=0;c<Nlag2Dx;c++){
				r2=Nlag2Dy-1-r;
				c2=Nlag2Dx-1-c;
				frame[r][c]=(float)s[r2*Nlag2Dx+c2].S[i][i];
				}
			fwrite_from_fmat(stdout,frame,Nlag2Dy,Nlag2Dx,out_format);
			}
		for (i=0;i<Nvar2;i++) for (j=0;j<i;j++){
			for (r=0;r<Nlag2Dy/2;r++) for (c=0;c<Nlag2Dx;c++)
				frame[r][c]=(float)s[r*Nlag2Dx+c].S[i][j];
			if (Nlag2Dy % 2==1){
				r2=Nlag2Dy/2;
				for (c=0;c<Nlag2Dx;c++)
					frame[r2][c]= (float)((s[r2*Nlag2Dx+c].S[i][j]+
						s[(r2+1)*Nlag2Dx-c-1].S[j][i])/2.0);
				}
			for (r=(Nlag2Dy+1)/2;r<Nlag2Dy;r++) for (c=0;c<Nlag2Dx;c++){
				r2=Nlag2Dy-1-r;
				c2=Nlag2Dx-1-c;
				frame[r][c]=(float)s[r2*Nlag2Dx+c2].S[j][i];
				}
			fwrite_from_fmat(stdout,frame,Nlag2Dy,Nlag2Dx,out_format);
			}
		}
	if (write_drift)
		for (i=0;i<Nvar2;i++){
			for (r=0;r<Nlag2Dy/2;r++) for (c=0;c<Nlag2Dx;c++)
				frame[r][c]=(float)s[r*Nlag2Dx+c].drift[i];
			if (Nlag2Dy % 2 ==1){
				r2=Nlag2Dy/2;
				for (c=0;c<Nlag2Dx;c++)
					frame[r2][c]= (float)((s[r2*Nlag2Dx+c].drift[i]-s[(r2+1)*Nlag2Dx-c-1].drift[i])/2.0);
				}
			for (r=(Nlag2Dy+1)/2;r<Nlag2Dy;r++) for (c=0;c<Nlag2Dx;c++){
				r2=Nlag2Dy-1-r;
				c2=Nlag2Dx-1-c;
				frame[r][c]=-(float)s[r2*Nlag2Dx+c2].drift[i];
				}
			fwrite_from_fmat(stdout,frame,Nlag2Dy,Nlag2Dx,out_format);
			}
}
		
		
void write1_sigma(argc,argv,s,out_format)
int  argc, out_format;
char *argv[];
Spt  s;
{  
	int		i, f, j, rows, cols, mean, spec, points, size;
	struct header outhd;
	char		*typestr;
	Dmatrix  	specmat=dmatrix(3,Nvar2);
	Dvector	dum=dvector(Nvar1);

	for (i=0;i<3;i++) for (j=0;j<Nvar2;j++) specmat[i][j]=0;
	rows = (trace) ? 1 : Nvar1;
	cols = Nvar2;
	mean = (write_drift) ? 1 : 0;
	spec = (write_spec) ? 3 : 0;
	points = (write_num) ? rows : 0;
	size = spec+mean+points+rows;

   	init_header(&outhd,"","",Ncov,"",size,cols,out_format,1,"");

	if (variogram) 
		setparam(&outhd,"Semivariance",PFBYTE,1,1);
	if (covaflag) 
   		setparam(&outhd,"Cova",PFBYTE,1,1);
	if (covariance)	
   		setparam(&outhd,"Covariance",PFBYTE,1,1);

	outhd.seq_desc="sequence: frame 0 is covariance-matrix, following frames contains cross-covariances or cross-semivariograms for lag 0,1,2, etc. If more than one direction is chosen, direction is nested within lags in the sequence.";
	update_header(&outhd,argc,argv);

	write_header(&outhd);

	for (f=0;f<Ncov;f++){
		if (write_spec) {
			specmat[0][0]=s[f].meandist;
			specmat[1][0]=s[f].meandir;
			specmat[2][0]=(double)s[f].Npairs;
 			fwrite_from_dmat(stdout,specmat,3,cols,out_format);
			}
		if ((write_num) && (trace)){
			for (j=0;j<cols;j++)
				dum[j]=s[f].num[j][j];
			fwrite_from_dvec(stdout,dum,cols,out_format);
			}
		if ((write_num) && (!trace))
			fwrite_from_dmat(stdout,s[f].num,rows,cols,out_format);
		if (write_drift) 
			fwrite_from_dvec(stdout,s[f].drift,cols,out_format);
		fwrite_from_dmat(stdout,s[f].S,rows,cols,out_format);
      	}
}



char *get_logname()
{
	int  number=getpid();
	char  *logname;
	logname=(char*)halloc(15,sizeof(char));
 	sprintf(logname,"crv%d.log",number);
	return logname;
}


Barray calcblock( dhead, Nirr, step, Nx, Ny, grid) 
double  	*grid, 	     /* lx, ux, ly, uy */	
		step;		/* sidelength of blocks in net. */
int		Nirr,		/* #points in irregular dataset. */ 
		Nx,			/* #coloumns in net. */ 
		Ny;			/* #rows in net. */
Dpt 		dhead; 		/* structure containing data. */
{
	int 		i, r, c;
	Dpt 		dpeger;
	Bpt 		indset();

	/* oprettelse af blokstruktur */
	bhead = (Bpt **)halloc(Nx,sizeof(Bpt *));
	for (c=0;c<Nx;c++)
    		bhead[c] = (Bpt *)halloc(Ny,sizeof(Bpt));
	for (c=0;c<Nx;c++) for (r=0;r<Ny;r++)
	    	bhead[c][r] = NULL;

	dpeger=dhead;
	for (i=0;i<Nirr;i++){
 	   	c = (dpeger->x - grid[0])/step;
    		r = (dpeger->y - grid[2])/step;
    		bhead[c][r] = indset(bhead[c][r],dpeger); 
    		dpeger=(dpeger+1);
		}
	return(bhead);
}



Bpt indset(bpeger,dpeger)
Bpt bpeger;
Dpt dpeger;
{
	Bpt  new;
	new = (Bpt)malloc(sizeof(bpoint));
	new->point = dpeger;
	new->next = bpeger;
	return(new); 
}


void wparam(argc,argv)
int argc;
char *argv[];
{
	int  	i, j;
	FILE	*fp;

	fprintf(stderr,"Program parameters written to flushfile: %s \n",logfname);
	fp=fopen(logfname,"w");
	fprintf(fp," CROSSV \n");
	fprintf(fp," PROGRAM-PARAMETERS:\n");
	fprintf(fp,"\n");
	fprintf(fp," program called by: \n"); 
	for (i=0;i<argc;i++){ 
		fprintf(fp," %s ",argv[i]);
		if (i % 12 == 11) fprintf(fp,"\n");
		}
	fprintf(fp,"\n");
	fprintf(fp,"\n");

	if (variogram) 
		fprintf(fp," Calculation of Cross-variogram matrices \n");
	if (covaflag)
		fprintf(fp," Calculation of Cova matrices \n");
	if (covariance)
		fprintf(fp," Calculation of Crossvariance matrices \n");
	if (srivastava)
		fprintf(fp," Variance calculated by use of deterministic framework \n"); 
	if (trace)
		fprintf(fp,"  - Only trace is calculated \n");
	if (mode_2D)
		fprintf(fp," 2D variance image is created \n");
	
	if ((write_drift)&&(variogram))
		fprintf(fp,"  - drift {Z(x)-Z(x+h)} in lags is calculated \n");
	if ((write_drift)&&(!variogram))
		fprintf(fp,"  - level of proces Z(x) in lags is calculated \n");

	if (covaflag) {
		fprintf(fp,"\n The output-matrices gives the cross-variance between");
		fprintf(fp," \n the variables in dataset 1 and dataset2.  Element (i,j) is the");
		fprintf(fp," \n cross-covariance between variable i in dataset 1 and variable j");
		fprintf(fp," \n in dataset 2 \n \n");
	}

	if (indicator){
		fprintf(fp,"\n Data are transformed to indicator-variables, values");
		fprintf(fp,"\n smaller than detection-limit are set equal to 0, otherwise 1.");
		fprintf(fp,"\n The limits are: \n");  
		for (j=0;j<Nvar;j++)
			fprintf(fp,"     variable %d : %8.4lf \n",j,detect_level[j]);
		fprintf(fp," \n \n ");
		}
	if (lowlimit){
		fprintf(fp,"\n Data below a specified detection-limit are skipped ");
		fprintf(fp,"\n from the calculations. The limits are: \n");  
		for (j=0;j<Nvar;j++)
			fprintf(fp,"     variable %d : %8.4lf \n",j,detect_level[j]);
		fprintf(fp," \n \n ");
		}
   
	if (logdata)
		fprintf(fp,"\n  Data are logaritmized before calculations. \n ");

	fprintf(fp,"\n max-distance               %f",maxdist);
	fprintf(fp,"\n #data-points               %d",Ndata1);
	fprintf(fp,"\n #variables                 %d",Nvar1);
	if (covaflag){
		fprintf(fp,"\n #data-points in cova-file  %d",Ndata2);
		fprintf(fp,"\n #variables in cova-file    %d",Nvar2);
		}

	if (out_format==3)
		fprintf(fp,"\n output-format              floats");
	else	fprintf(fp,"\n output-format              doubles");
	if (lag_cent)
		fprintf(fp,"\n lags (centered)            %d",Nintv); 
	else 
		fprintf(fp,"\n lags                       %d",Nintv);
	if (dir_cent)
		fprintf(fp,"\n directions (centered)      %d",Ndir); 
	else 
		fprintf(fp,"\n directions                 %d",Ndir);
	if (compas_angles)
		fprintf(fp," (compas-angles)");
	if (subsmp>0)
		fprintf(fp,"\n level of subsampling       %f",subsmp);
	if (lag_cov>0)
		fprintf(fp,"\n lag-coverage               %f",lag_cov);
	if (dir_cov>0)
		fprintf(fp,"\n direction-coverage         %f",dir_cov);
	if (undersampled)
		fprintf(fp,"\n missing-data value         %f",missingval);
	if (leaveout)
		fprintf(fp,"\n observations with missing-values are skipped ");
	if (write_num)
		fprintf(fp,"\n number of variance-elements are written to output");  
	fprintf(fp,"\n");
	fprintf(fp,"\n");
	fprintf(fp,"\n");

	fprintf(fp," Processing-time \n");
	fprintf(fp,"   Preprocessing of data  %6.2f seconds \n",time2-time1);
	fprintf(fp,"   Variance calculations  %6.2f seconds \n",time3-time2);
	fprintf(fp,"\n");
	fprintf(fp,"\n");

	if (!mode_2D){
	fprintf(fp," lag-intervals: \n");
	fprintf(fp,"\n   0 :  0.000000 - %f",lag_vec[0]);
	if (lag_full)
		for (i=1;i<Nintv;i++)
		fprintf(fp,"\n   %d :  %f - %f",i,lag_vec[i-1],lag_vec[i]);
	else
		for (i=1;i<Nintv;i++)
		fprintf(fp,"\n   %d :  %f - %f",i,maxval(lag_vec[i-1],lag_vec[i]-lag_size),lag_vec[i]);
	fprintf(fp,"\n");
	fprintf(fp,"\n");
	fprintf(fp,"\n");

	if (Ndir>1){
		fprintf(fp,"\n direction-intervals:\n");
		if (dir_full){
			fprintf(fp,"\n   0 :  %8.2f - %8.2f",(dir_vec[Ndir-1]-pi)*rad,dir_vec[0]*rad);
			for (i=1;i<Ndir;i++)
			fprintf(fp,"\n   %d :  %8.2f - %8.2f",i,dir_vec[i-1]*rad,dir_vec[i]*rad);
			}
		else { 
			fprintf(fp,"\n   0 :  %8.2f - %8.2f",maxval(dir_vec[Ndir-1]-pi,dir_vec[0]-dir_size)*rad,dir_vec[0]*rad);
			for (i=1;i<Ndir;i++)
			fprintf(fp,"\n   %d :  %8.2f - %8.2f",i,maxval(dir_vec[i-1],dir_vec[i]-dir_size)*rad,dir_vec[i]*rad);
			}
		fprintf(fp,"\n");
		fprintf(fp,"\n");
		fprintf(fp,"\n");
	}
	}
	else {
		fprintf(fp,"\n size of 2D variance grid ");
		fprintf(fp,"\n   horisontal length of pixels :  %f",lag2D_sizex); 
		fprintf(fp,"\n   vertical length of pixels :    %f",lag2D_sizey); 
		fprintf(fp,"\n");
		fprintf(fp,"\n");
		}

	fprintf(fp," limits of coordinates:\n");
	if (covaflag){
		fprintf(fp,"\n   std.input dataset: ");
		fprintf(fp,"\n   x :  %f  -  %f",mainf.lx,mainf.ux);
		fprintf(fp,"\n   y :  %f  -  %f",mainf.ly,mainf.uy);
		fprintf(fp,"\n \n  cova dataset:"); 
		fprintf(fp,"\n   x :  %f  -  %f",cova.lx,cova.ux);
		fprintf(fp,"\n   y :  %f  -  %f",cova.ly,cova.uy);
		fprintf(fp,"\n \n  both dataset:"); 
	}

	fprintf(fp,"\n   x :  %f  -  %f",total.lx,total.ux);
	fprintf(fp,"\n   y :  %f  -  %f",total.ly,total.uy);
	fprintf(fp,"\n");
	fprintf(fp,"\n");
	fprintf(fp,"\n Point-search statistics:  ");
	fprintf(fp,"\n   side-length of search-blocks  %4.4f",blockstep);
	fprintf(fp,"\n   blocks placed over dataset    %d",Nx*Ny);  
	fprintf(fp,"\n   point-pairs searched          %d",Nsearch); 
	fprintf(fp,"\n   point-pairs used              %d",Nused);
	fprintf(fp,"\n");
	fprintf(fp,"\n");
	fprintf(fp,"\n");

	if (!mode_2D){
		fprintf(fp,"                  INTERVAL-STATISTICS            \n");
		fprintf(fp,"\n                         ");
		fprintf(fp,"\n interval   #lag   #dir  average dist.  average angle  point-pairs \n");   
		for (i=1;i<Ncov;i++) 
			fprintf(fp,"\n  %4d     %4d   %4d    %10.4f     %10.4f     %8d ",i, (i-1)/Ndir, 
				(i-1)%Ndir, s[i].meandist, s[i].meandir*rad, s[i].Npairs);
		fprintf(fp,"\n");
		fprintf(fp,"\n");
		fprintf(fp,"\n");
		fprintf(fp," Sequence in matrix-file: First frame contains covariance matrix. \n The following frames contains cross-variance matrices in same \n order as listet above. \n ");
		}
	fclose(fp);

}
