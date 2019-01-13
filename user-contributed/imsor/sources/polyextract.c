/*      Copyright (c) 1992  Karsten Hartelius, IMSOR.

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   

 polyextract - extract a polygon shape from a sequence, by setting all 
 image-elements outside the polygon equal to 0.

 usage: see manual

 to load:
	cc -o polyextract polyextract.c -L /gsnk1/hips2/lib -lm -lhips

 to run:
     polyextract  parameters  <HIPS-file  > HIPS-file 

 include-files: util.h
	
*/

#include <hipl_format.h>
#include "util.h"

#define Nmax 100
#define   minval(x,y) ((x<y) ? x : y)
#define   maxval(x,y) ((x>y) ? x : y)

struct point {
	double  x, y;
};

struct line {
	struct point p1, p2;
	};

struct line  lt, lp;
struct point polygon[Nmax];

int		N, npoints, cols, frames, format, irregular, inside(),on_opp_side(), 
		intersect(), ccw(), find_inside, startx, stopx, starty, stopy;

void		read_polygon(), read_data(), polygon_limits(), calc_include(), 
		write_out();
Bvector	include;
Fvector	data;
Dvector	X, Y;
float	skip_value;
double	largex, pol_lx, pol_ux, pol_ly, pol_uy;
struct 	header hd;
int between();

int main(argc,argv)
int argc;
char *argv[];
{
	int	i;
	float dum;
	find_inside=TRUE;
	irregular=FALSE;
	skip_value=-1;
	for (i=1;i<argc;i++) if (argv[i][0]=='-'){
		switch (argv[i][1]){
			case 'f': read_polygon(argv[++i]); break;
			case 'p': N=0; 
					i++;
					while ((i<argc) && (argv[i][0]!='-')){
						N++;
						polygon[N].x=atof(argv[i++]);
						polygon[N].y=atof(argv[i++]);
						}
					i--;
					break;
			case 'i': find_inside=FALSE; break;
			case 's': i++;
					skip_value=atof(argv[i]); break;
			default : perr(HE_MSG,"wrong argument");
			}
		}
	read_data();
	polygon_limits();
	calc_include();
	write_out(argc,argv);
}

void read_polygon(fname)
char *fname;
{
	FILE *fp;
	float  x, y;

	if (!(fp=fopen(fname,"r")))
		perr(HE_MSG,"can not open polygon-file"); 
	N=0; /* notice N=1. */ 
	while (fscanf(fp," %f %f ",&x, &y)!=EOF) {
		N++;
		polygon[N].x=(double)x;
		polygon[N].y=(double)y;
		}
}


void polygon_limits()
{
	int	i;
	pol_lx = pol_ly = BIG; 
	pol_ux = pol_uy = -BIG; 
	for (i=1;i<=N;i++){
		pol_lx = minval(pol_lx,polygon[i].x);
		pol_ux = maxval(pol_ux,polygon[i].x);
		pol_ly = minval(pol_ly,polygon[i].y);
		pol_uy = maxval(pol_uy,polygon[i].y);
		}
}

		
void read_data()
{
   	int  	i, count ;
	byte		val;

	read_header(&hd);
	count=1; val=0;
	if (findparam(&hd,"Irregular") != NULLPAR)
		getparam(&hd,"Irregular",PFBYTE,&count,&val);
	if (val == 1)
		irregular=TRUE;

	npoints=hd.ocols*hd.orows;
	cols=hd.ocols;
	frames=hd.num_frame;
	format=hd.pixel_format;

	if (irregular){
		X=dvector(npoints);
		Y=dvector(npoints);
		fread_to_dvec(stdin,X,npoints,format);	
		fread_to_dvec(stdin,Y,npoints,format);	
		largex=-BIG;
		for (i=0;i<npoints;i++)
			largex = maxval(X[i],largex);
		frames-=2;
		}
	else 
		largex = cols+1;
	include=bvector(npoints);
	for (i=0;i<npoints;i++)
		include[i]=0;
}  /* input_data */



void calc_include()
{
	int	i, x, y, last;
	struct point p;

	polygon[0]=polygon[N];
	polygon[N+1]=polygon[1];
	polygon[N+2]=polygon[2];
	if (irregular){
		for (i=0;i<npoints;i++)
			if ((X[i]>=pol_lx) && (X[i]<=pol_ux) && (Y[i]>=pol_ly) && (Y[i]<=pol_uy)){
				p.x=X[i];
				p.y=Y[i];
				if (inside(p,polygon,N,&last))
					include[i]=1;
				}
		}
	else {
		for (y=pol_ly;y<=pol_uy;y++){
			x=pol_lx;
			while (x<=pol_ux){
				p.x=(double)x;
				p.y=(double)y;
				if (inside(p,polygon,N,&last))
					for (x=x;(x<=last)&&(x<=pol_ux);x++)
						include[y*cols+x]=1;
				else
					for (x=x;x<last;x++)
						include[y*cols+x]=0;
				}
			}

		}
	return;
}			


int inside(t,p,N,last)
struct  point t, p[];
int	N, *last;
{
	/* A horisontal line which is infinite towards right is created and
	   the number of time this line crosses the polygon is counted */
	int i, count=0;
	double  limit=pol_ux+1, x;
	lt.p1=t; 
	lt.p2=t; 
	/* create infinite long horizontal line */
	lt.p2.x = largex; 
	for (i=1;i<=N;i++){
		if (p[i].y==lt.p1.y){
			if (p[i+1].y==lt.p1.y){
				/* testline runs through horisontal polygon-edge */
				if (between(lt.p1.x,p[i].x,p[i+1].x)){
					/* on line */
					*last=maxval(p[i].x,p[i+1].x);
					return 1;
					}
				/* left of line */ 
				if (p[i].x > lt.p1.x){
					if (on_opp_side(lt,p[i-1],p[i],p[i+1],p[i+2]))
						count++;
					limit=minval(limit, minval(p[i].x,p[i+1].x) );
					}
				i++;
				}
			else {
				/* testline runs through polygon-vertice */
				if (p[i].x > lt.p1.x){
					if (on_opp_side(lt,p[i-1],p[i],p[i],p[i+1]))
						count++;
					limit=minval(limit,p[i].x);
					}
				}
			}
		else {
			lp.p1=p[i]; 
			lp.p2=p[i-1];
			if (intersect(&x)){ 
				count++;
				limit=minval(limit,x);
				}
			}
		}
	*last=limit;
	return count % 2;
}

int between(v0,v1,v2)
double	v0, v1, v2;
{
	return ( ((v0<=v1)&&(v0>=v2)) || ((v0>=v1)&&(v0<=v2)) );
}

int on_opp_side(lt,p0,p1,p2,p3)
struct line lt;
struct point p0, p1, p2, p3;
{
	double	dx0, dx1, dx2, dx3, dy0, dy1, dy2, dy3;

	dx0 = lt.p2.x - p0.x;
	dx1 = lt.p2.x - p1.x;
	dx2 = lt.p2.x - p2.x;
	dx3 = lt.p2.x - p3.x;
	dy0 = lt.p2.y - p0.y;
	dy1 = lt.p2.y - p1.y;
	dy2 = lt.p2.y - p2.y;
	dy3 = lt.p2.y - p3.y;
	return ( ((-dx0*dy1+dy0*dx1)*(-dx2*dy3+dy2*dx3)) > 0 );	
}

int intersect(x)
double  *x;
{
	if ( ((lp.p1.y<lt.p1.y)&&(lp.p2.y>lt.p1.y)) ||
		((lp.p1.y>lt.p1.y)&&(lp.p2.y<lt.p1.y)) ){
		*x = lp.p1.x + (lt.p1.y-lp.p1.y)/(lp.p2.y-lp.p1.y)*(lp.p2.x-lp.p1.x);
		return ((int)(*x)>lt.p1.x);
		}
	else return 0;
}
		


void write_out(argc,argv)
int	argc;
char	*argv[];
{
	int		i, k;

	update_header(&hd,argc,argv);
	write_header(&hd);
	if (irregular){
		fwrite_from_dvec(stdout,X,npoints,format);	
		fwrite_from_dvec(stdout,Y,npoints,format);	
		}

  	data=fvector(npoints);
	for (k=0;k<frames;k++){
	   	fread_to_fvec(stdin,data,npoints,format);
		if (find_inside){
			for (i=0;i<npoints;i++) 
				if (!include[i])
					data[i]=skip_value;
			}
		else {
			for (i=0;i<npoints;i++) 
				if (include[i])
					data[i]=skip_value;
			}
	   	fwrite_from_fvec(stdout,data,npoints,format);
		}

}

