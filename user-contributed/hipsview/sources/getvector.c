#include <hipl_format.h>
#include "hipsview.h"
#include <stdio.h>
#include <sunwindow/window_hs.h>

#define V_LT -1
#define V_RT  1
#define V_UP -nx
#define V_DN  nx

int
getvector(func, x1, y1, x2, y2, nx, ny)
void (*func)();
{
	register int index, adrinc, ulim;
	register int D, A, B, u, v, vlim, nxXny;
	int diagonal_move, parallel_move;
	int dxdir, dydir, npts;

	/* East-West   boarder clip limits set by ulim & vlim  */ 
	/* North-South boarder clip limits set by   0  & nx*ny */
	nxXny = nx*ny;
	index = x1+y1*nx;
	if(index>=nxXny || index<0)
		return(0);

	if((u=x2-x1) < 0){
		u = -u;
		dxdir = V_LT;
		ulim = (x2<0) ? x1 : u;
	}else{
		dxdir = V_RT;
		ulim = (x2>nx) ? nx-x1 : u;
	}
	if((v=y2-y1) < 0){
		v = -v;
		dydir = V_UP;
		vlim = (y2<0) ? y1 : v;
	}else{
		dydir = V_DN;
		vlim = (y2>ny) ? ny-y1 : v;
	}

	if(u==0){		/* Vertical line */
		if(v==0)
			return;
		adrinc = dydir;
		u = v;
		ulim = vlim;
		goto L_fastmove;

	}else if(v==0){		/* Horizontal line */
		adrinc = dxdir;
		goto L_fastmove;

	}else if(u==v){		/* 45 deg line */
		adrinc = dxdir + dydir;
		if(vlim<ulim)
			ulim = vlim;
L_fastmove:
		npts = ulim;
		do{
			if(index>=nxXny || index<0)
				break;
			(*func)(index);		/* *vecP++ = pixP[index]; */
			index += adrinc;
		}while(--ulim > 0);

	}else{
		/* Bresenham line algorithm */

		if(u>v){		/* line in octant 1, 4, 5 or 8 */
			parallel_move = dxdir;
		}else{			/* line in octant 2, 3, 6 or 7 */
			D = v; v = u; u = D;
			parallel_move = dydir;
			ulim = vlim;
		}
		diagonal_move = dxdir + dydir;
		B = v<<1;
		D = B - u;
		A = D - u;
		npts = ulim;
		do{
			if(D>=0){
				adrinc = diagonal_move;
				D += A;
			}else{
				adrinc = parallel_move;
				D += B;
			}
			if(index>=nxXny || index<0)
				break;
			(*func)(index);		/* *vecP++ = pixP[index]; */
			index += adrinc;
		}while(--ulim > 0);
	}

	return(npts+ulim);
}

static unsigned char *L_vecP, *L_pixP;
static L_vecfunc(index)
{
	*L_vecP++ = L_pixP[index];
}

int
getpixvector(vecbuf, x1, y1, x2, y2, pixbuf, nx, ny)
unsigned char vecbuf[], pixbuf[];
{
	L_vecP = &vecbuf[0];
	L_pixP = &pixbuf[0];
	return(getvector(L_vecfunc, x1, y1, x2, y2, nx, ny));
}

/******/
extern char picfname[];
extern int hdtype;

unsigned char	*L_ucharP;
char		*L_charP;
unsigned short	*L_ushortP;
short		*L_shortP;
long		*L_longP;
float		*L_floatP;
int L_cnt;
FILE *L_wfP, *fopen();

L_PixWrite(index)
{
	int t;
	float f;

	switch(hdtype){
	case PFBYTE:  t = L_ucharP[index];	break;
/*	case BYTE_HD:   t = L_ucharP[index]-128;break; */
/*	case USHORT_HD: t = L_ushortP[index];	break; */
	case PFSHORT:  t = L_shortP[index];	break;
	case PFINT:   t = L_longP[index];	break;
	case PFFLOAT:  f = L_floatP[index];
			fprintf(L_wfP, " %f", f);
			goto L_Pix;
	default:	return;
	}
	fprintf(L_wfP, " %d", t);
   L_Pix:
	if(++L_cnt>=10){
		L_cnt = 0;
		fprintf(L_wfP, "\n");
	}
}

#define OVERWRITESW_OFF 0
#define OVERWRITESW_ON  1
#define OVERWRITESW_ENABLE 2
extern int overwritesw;

write_slice_to_file(filenm, x1, y1, x2, y2, imP, nx, ny)
char *filenm;
IMAGEptr *imP;
{
	int n;

	x1 = xScaleIFm(x1); x2 = xScaleIFm(x2);
	y1 = yScaleIFm(y1); y2 = yScaleIFm(y2);
	fprintf(stderr,"\nslice range xy0(%3d,%3d) to dxy(%3d,%3d)\n",
					x1, y1, x2-x1, y2-y1);
	if(x1<0||y1<0||x2<0||y2<0 || x1>nx||x2>nx || y1>ny||y2>ny){
		fprintf(stderr, "write Err: slice out of range\n");
		overwritesw = OVERWRITESW_OFF;
		return;
	}
	if(access(filenm, 0)==0 && overwritesw!=OVERWRITESW_ON){
		overwritesw = OVERWRITESW_ENABLE;
		fprintf(stderr,
"\nFile <%s> exists - type '\"' to overwrite ? \n ***  ",filenm);
		fflush(stderr);
		return;
	}

	if(filenm==0 || filenm[0]<='"'){
		fprintf(stderr,"Illegal filename\n");
		return;
	}

	fprintf(stderr, "\n");
	overwritesw = OVERWRITESW_OFF;

	if((L_wfP=fopen(filenm, "w"))==NULL) {
		perror("write_slice_to_file, ");
		fprintf(stderr, "Can't creat file: %s\n", filenm);
		return;
	}
 
	fprintf(stderr, "Writing image slice to file<%s>...", filenm);

	fprintf(L_wfP, "%s xy0(%3d,%3d) to dxy(%3d,%3d)\n",
					&picfname[0], x1, y1, x2-x1, y2-y1);
	switch(hdtype){
	case PFBYTE:  L_ucharP  = (unsigned char  *)imP;	break;
/*	case BYTE_HD:   L_charP   = (char  *)imP;		break; */
/*	case BYTE_HD:   L_ucharP  = (unsigned char  *)imP;	break; */
/*	case USHORT_HD: L_ushortP = (unsigned short *)imP;	break; */
	case PFSHORT:  L_shortP  = (short *)imP;		break;
	case PFINT:   L_longP   = (long  *)imP;			break;
	case PFFLOAT:  L_floatP  = (float *)imP;		break;
	}
	L_cnt = 0;

	n = getvector(L_PixWrite, x1, y1, x2, y2, nx, ny);
	fprintf(L_wfP, "\n");
	fprintf(stderr, "Wrote %d pix.\n", n);
	fclose(L_wfP);
	}

	write_pbox_to_file(filenm)
	char *filenm;
	{
		if(access(filenm, 0)==0 && overwritesw!=OVERWRITESW_ON){
			overwritesw = OVERWRITESW_ENABLE;
		fprintf(stderr,
"\nFile <%s> exists - type '\"' to overwrite ? \n ***  ",filenm);
		fflush(stderr);
		return;
	}

	if(filenm==0 || filenm[0]<='"'){
		fprintf(stderr,"Illegal filename\n");
		return;
	}

	fprintf(stderr, "\n");
	overwritesw = OVERWRITESW_OFF;

	if((L_wfP=fopen(filenm, "w"))==NULL) {
		perror("write_pbox_to_file, ");
		fprintf(stderr, "Can't creat file: %s\n", filenm);
		return;
	}
 
	fprintf(stderr, "Writing pbox to file<%s>...", filenm);

	op_pabox(L_wfP, 'p');
	fprintf(stderr, "Done\n");
	fclose(L_wfP);
}

extern int mh_wd, mh_ht, pa_wd, pa_ht;
extern IMAGEptr *image1b, *image2b, *image4b;

printPaAvg(ulcsw, mx, my)
{
	printavg_std(ulcsw, mx, my, pa_wd, pa_ht);
}
printHwAvg(ulcsw, mx, my)
{
	printavg_std(ulcsw, mx, my, mh_wd, mh_ht);
}
printHfAvg(wd, ht)
{
	printavg_std('u', 0, 0, wd, ht);
}
	
printavg_std(ulcsw, mx, my, wd, ht)
{
	if(ulcsw=='c'){
		mx -= wd;	my -= ht;
		wd *= 2;	ht *= 2;
	}

	getimstatistics(&ap_imst, mx, my, wd, ht, nu, nv);
	get_avg_sigma(&ap_imst);
	if(ulcsw=='c') fprintf(stdout,"sac ");
	fprintf(stdout,
		"xyUL(%3d,%3d) xyC(%5.1f,%5.1f) wd:ht(%3d,%3d)\n",
			mx, my,
			mx+(wd/2.0-0.5), my+(ht/2.0-0.5),
			wd, ht);
	fprintf(stdout, "pix avg = %f  sigma = %f  range = %f\n",
			ap_imst.average, ap_imst.sigma,
			ap_imst.fmax-ap_imst.fmin);
	fprintf(stdout, " %d min @xy(%d %d)=%f",
			ap_imst.mincnt, ap_imst.min_ix,
			ap_imst.min_iy, ap_imst.fmin);
	fprintf(stdout, "  %d max @xy(%d %d)=%f\n",
			ap_imst.maxcnt, ap_imst.max_ix,
			ap_imst.max_iy, ap_imst.fmax);
}

getimstatistics(imstP, x1, y1, wd, ht, nx, ny)
struct imstatistics *imstP;
{
	IMAGEptr *imP;
	register int ix, iy, ylim, xlim;
	int y1Xnx;
	int Idata, imax, imin;
	int max_ix, max_iy, min_ix, min_iy, maxcnt, mincnt;
	register float fmin, fmax, ft, fsum, fsumsq;

	fmin = imin =  INFINITY;
	fmax = imax = -INFINITY;
	maxcnt = mincnt = 0;
	max_ix = max_iy = min_ix = min_iy = 0;
	fsum = fsumsq = 0;

	xlim = x1+wd-1; ylim = y1+ht-1;
	if(x1<0 || y1<0 || xlim>=nx || ylim>=ny){
		fprintf(stderr, "avg win out of range\n");
		imstP->n = fmin = imin = fmax = imax = 0;
		goto L_rtn;
	}

	y1Xnx = y1*nx;

	switch(hdtype){
	case PFBYTE:
		imP = image1b;
		if(imP==0) goto L_err;
		L_ucharP = (unsigned char  *)imP;
		L_ucharP+=y1Xnx;
		for(iy=y1; iy<=ylim; iy++, L_ucharP+=nx)
			for(ix=x1; ix<=xlim; ix++){
				Idata =  L_ucharP[ix];
				if(Idata >= imax){
					if(Idata==imax) maxcnt++;
					else{
						imax = Idata; maxcnt = 1;
						max_ix = ix; max_iy = iy;
					}
				}else if(Idata <= imin){
					if(Idata==imin) mincnt++;
					else{
						imin = Idata; mincnt = 1;
						min_ix = ix; min_iy = iy;
					}
				}
				ft = Idata;
				fsum += ft;   fsumsq += ft*ft;
			}
		break;
/*	case BYTE_HD:
		imP = image1b;
		if(imP==0) goto L_err;
		L_ucharP = (unsigned char  *)imP;
		L_ucharP+=y1Xnx;
		for(iy=y1; iy<=ylim; iy++, L_ucharP+=nx)
			for(ix=x1; ix<=xlim; ix++){
				Idata =  L_ucharP[ix] - 128;
				if(Idata >= imax){
					if(Idata==imax) maxcnt++;
					else{
						imax = Idata; maxcnt = 1;
						max_ix = ix; max_iy = iy;
					}
					imax = Idata; maxcnt++;
				}else if(Idata <= imin){
					if(Idata==imin) mincnt++;
					else{
						imin = Idata; mincnt = 1;
						min_ix = ix; min_iy = iy;
					}
				}
				ft = Idata;
				fsum += ft;   fsumsq += ft*ft;
			}
		break; */
/*	case USHORT_HD:
		imP = image2b;
		if(imP==0) goto L_err;
		L_ushortP = (unsigned short *)imP;
		L_ushortP+=y1Xnx;
		for(iy=y1; iy<=ylim; iy++, L_ushortP+=nx)
			for(ix=x1; ix<=xlim; ix++){
				Idata =  L_ushortP[ix];
				if(Idata >= imax){
					if(Idata==imax) maxcnt++;
					else{
						imax = Idata; maxcnt = 1;
						max_ix = ix; max_iy = iy;
					}
				}else if(Idata <= imin){
					if(Idata==imin) mincnt++;
					else{
						imin = Idata; mincnt = 1;
						min_ix = ix; min_iy = iy;
					}
				}
				ft = Idata;
				fsum += ft;   fsumsq += ft*ft;
			}
		break; */
	case PFSHORT:
		imP = image2b;
		if(imP==0) goto L_err;
		L_shortP = (short *)imP;
		L_shortP+=y1Xnx;
		for(iy=y1; iy<=ylim; iy++, L_shortP+=nx)
			for(ix=x1; ix<=xlim; ix++){
				Idata =  L_shortP[ix];
				if(Idata >= imax){
					if(Idata==imax) maxcnt++;
					else{
						imax = Idata; maxcnt = 1;
						max_ix = ix; max_iy = iy;
					}
				}else if(Idata <= imin){
					if(Idata==imin) mincnt++;
					else{
						imin = Idata; mincnt = 1;
						min_ix = ix; min_iy = iy;
					}
				}
				ft = Idata;
				fsum += ft;   fsumsq += ft*ft;
			}
		break;
	case PFINT:
		imP = image4b;
		if(imP==0) goto L_err;
		L_longP = (long  *)imP;
		L_longP+=y1Xnx;
		for(iy=y1; iy<=ylim; iy++, L_longP+=nx)
			for(ix=x1; ix<=xlim; ix++){
				Idata =  L_longP[ix];
				if(Idata >= imax){
					if(Idata==imax) maxcnt++;
					else{
						imax = Idata; maxcnt = 1;
						max_ix = ix; max_iy = iy;
					}
				}else if(Idata <= imin){
					if(Idata==imin) mincnt++;
					else{
						imin = Idata; mincnt = 1;
						min_ix = ix; min_iy = iy;
					}
				}
				ft = Idata;
				fsum += ft;   fsumsq += ft*ft;
			}
		break;
	case PFFLOAT:
		imP = image4b;
		if(imP==0) goto L_err;
		L_floatP = (float *)imP;
		L_floatP+=y1Xnx;
		for(iy=y1; iy<=ylim; iy++, L_floatP+=nx)
			for(ix=x1; ix<=xlim; ix++){
				ft = L_floatP[ix];
				if(ft >= fmax){
					if(ft==fmax) maxcnt++;
					else{
						fmax = ft; maxcnt = 1;
						max_ix = ix; max_iy = iy;
					}
				}else if(ft <= fmin){
					if(ft==fmin) mincnt++;
					else{
						fmin = ft; mincnt = 1;
						min_ix = ix; min_iy = iy;
					}
				}
				fsum += ft;   fsumsq += ft*ft;
			}
		imstP->imax   = fmax;
		imstP->imin   = fmin;
		break;
	default:
  L_err:
		printf("Illegal hdtype, getimstatistics()\n");
		return;
	}

	imstP->n = wd*ht;
   L_rtn:
	imstP->fsum   = fsum;
	imstP->fsumsq = fsumsq;
	imstP->imax   = imax;
	imstP->imin   = imin;
	imstP->fmax   = fmax;
	imstP->fmin   = fmin;
	imstP->maxcnt = maxcnt;
	imstP->mincnt = mincnt;
	imstP->max_ix = max_ix;
	imstP->max_iy = max_iy;
	imstP->min_ix = min_ix;
	imstP->min_iy = min_iy;
	if(hdtype!=PFFLOAT){
		imstP->fmax   = imax;
		imstP->fmin   = imin;
	}
}
	
get_avg_sigma(imstP)
struct imstatistics *imstP;
{
	float  n,  avg, var;
	double sqrt();

	n = imstP->n;
	avg = imstP->fsum/n;
	var = (imstP->fsumsq - n*avg*avg)/n;
	imstP->sigma = sqrt(var);		/* standard deviation */
	imstP->average = avg;
}
