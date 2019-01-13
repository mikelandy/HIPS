#include "util.h"
#define MAXLAG 100 
#define MAXDIR 50
#define MAXFEAT 200
#define EPSILON 0.0000001
#define Seedval 7145219

typedef struct Point *Dpt;

typedef struct Point {
	int  no;
	double x, y, *v; 
	} point;

typedef struct Blokpoint *Bpt;

typedef struct Blokpoint {
   Dpt  point;
   Bpt  next;
   } bpoint;

typedef Bpt  **Barray;

struct Limit {
	double  lx, ux, ly, uy;
	};	

typedef struct Sigma *Spt;

typedef struct Sigma {
	int 		Npairs, 	/* number of point-pairs used  */
			*nfeat;	/* number of feature i, i=0,..,n-1 in lag. */ 
	double  	**num,	/* number of (i,j) feature-comb. in point-pairs. */ 
		 	dist,	/* specified distance of lag */ 
			meandist,	/* mean distance in point-pairs */ 
			dir,		/* specified direction of lag. */ 
			meandir,	/* mean angle in point-pairs. */ 
			*drift,	/* mean drift in features in point-pairs. */
			**S,		/* cross-variance */
			**m_minus,
			**m_plus;
	} sigma;

typedef Spt *Slistpt;


int  	Nintv,		/* #lag-intervals */ 
		Ndir,		/* #direction intervals */ 
		Nlag2Dx,		/* #lags in x-direction of 2D-grid */
		Nlag2Dy,		/* #lags in y-direction of 2D-grid */
		centerblock,	/* number of center-lag in 2D-grid */
		Nvar,		/* #features */ 
		Nvar1,		/* in case of cova: #features in std.input file */
		Nvar2,		/* in case of cova: #features in cova-file */
		Ndata,		/* #data-points */ 
		Ndata1,		/* #datapoints in data-file */ 
		Ndata2,		/* #datapoints in cova-file */ 
		Nx,			/* #blocks in x-direction */ 
		Ny,			/* #blocks in y-direction */ 
		Ncov,		/* #Covariance/Semivarince-matrices */ 
		Ndetect,		/* #detection-limits given in parameterlist */
     	Nsearch,		/* #point-pairs met during neighbor-search */ 
		Nused,		/* #point-pairs used in calculations */ 
		out_format;	/* #pixel-format of output-data */

struct	Limit
		mainf, 		/* limits of data read from std.input */ 
		cova,		/* limits of data given by option "-cova" */ 
		total,		/* limits of the total data-set */
		inclu;		/* limits of data-set used in calculations */

Boolean 	variogram,  	/* Semivariogram */
		covariance,	/* Covariance */ 
		covaflag,	 	/* cova-variance */
		mode_2D,		/* 2D-grid contains displacement vectors */	
		mode_table,	/* variance written to matrices */
		sigma0,		/* Covariance included in cross-variance */
        	trace,      	/* Only diagonal-elements are calculated */
		compas_angles,	/* North=0, East=90, South=180, West=270 */
		lag_cent,		/* lags are centered */
		dir_cent,		/* direction intervals are centered */
		lag_full,		/* lags cover the whole interval: [0;maxrange] */
		dir_full,		/* dirs cover the whole interval: [0;pi] */
        	write_drift, 	/* mean-vector is written for each output-matrice */
		write_spec, 	/* Mean distance and angle of points in interval is
			   			written to output. */
		srivastava,  	/* variances calculated by deterministic method ? */
		indicator,	/* Indicator variables? */ 
		lowlimit,		/* values < limit = missing */ 
		logdata,		/* Transform features by ln before calculations */
		undersampled,	/* missing point, value = missing.*/
		leaveout,		/* Observations containing missing values are 
					   skipped totally. */
		write_num;	/* Number of point-pairs written to output*/

double 	maxdist,  	/* maximum lag-distance */
		subsmp,   	/* level of sub-sampling */
		lag_size, 	/* size of lags */  
		dir_size, 	/* size of direction-intervals */
		lag_cov,  	/* coverage of lags */
		dir_cov,  	/* coverage of directions-intervals */
		lag2D_sizex,	/* horisontal size of lags in 2D-grid */
		lag2D_sizey,	/* horisontal size of lags in 2D-grid */
		blockstep, 	/* side-length of blocks in block-structure */
		blockdensity, 	/* Average no. of points in a block */
		breakpoint,  	/* border between main-data and cova-data in dhead*/
		detect_level[MAXFEAT],	
					/* detection-limits of variables in case of  
					   indicator-variables */
		missingval, 	/* value of missing features */
		neutralval,	/* value of non-defined variance-elements. */
		time1,		/* variables containing computation-time */
		time2,
		time3;

Dvector 	lag_vec,     	/* list of lags */
		dir_vec,		/* list of directions */ 
		grid,		/* vector of block=net limits */
		mean;		/* vector of variable-means */

Dpt 		dhead;

Barray	bhead;

Spt     	s;

char		*logfname, 
		*covafname;

FILE    	*fp;

