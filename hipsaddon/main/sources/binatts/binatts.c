static char *SccsId = "%W%      %G%";

/* Compute binary shape-attributes from a hips format picture.
 *
 * B.A.Shepherd, March,85
 */

#include <stdio.h>
#include <hipl_format.h>
#include "satts.h"

#define TRUE	1
#define FALSE	0
#define MAXATTS 40
#define NAMSIZE	30

int numatts ;
char attnam[MAXATTS][NAMSIZE] ;
char dstdir[]=DSTDIR;

static Flag_Format flagfmt[] = {
	{"f",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTFILENAME,"","attribute-file"},LASTPARAMETER}},
	{"v",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG } ;

int types[] = {PFBYTE,LASTTYPE} ;
void readattfile(),pre_getattval(),free_blobs();
int next_blob(),getattval(),delete_blob();

int main(argc,argv)
int argc ;
char *argv[] ;
{
	int i,picno,val,method;
	h_boolean vector,attlist = FALSE,blobsfound = FALSE,fflag;
	char classnam[30],fname[200];
	Filename afname,ifname;
	struct header hd,hdp ;
	struct blobstruct *bp,*make_blob(), *blistptr = NULL;
	FILE *afp,*ifp ;

	Progname = strsave(*argv);
	strcpy(fname,dstdir);
	strcat(fname,"/atts.35");
	strcpy(classnam,"unknown");

	parseargs(argc,argv,flagfmt,&fflag,&afname,&vector,FFONE,&ifname) ;
	if (!fflag)
		afname = fname;

	if (!attlist) {
		if ((afp=fopen(afname,"r"))==NULL) {
			printf("can't open %s\n",afname) ;
			exit(-1) ;
		}
		readattfile(afp);
	} 

	ifp = hfopenr(ifname) ;
	fread_hdr_a(ifp,&hd,ifname);
	method = fset_conversion(&hd,&hdp,types,ifname) ;

	pre_getattval(hd.ocols,hd.orows) ;  /* no cleaning now!*/ 

	for (picno=0; picno < hdp.num_frame; picno++) {
		fread_imagec(ifp,&hd,&hdp,method,picno,ifname) ;

		bp = make_blob(hdp.image,hdp.ocols,hdp.orows,&blistptr) ;
		bp->xst = 0 ; bp->yst = 0 ;
		while (next_blob(hdp.image,hdp.ocols,hdp.orows,
					&bp->xst,&bp->yst)) {
			blobsfound = TRUE ;
			for (i=0;i<numatts;i++) {
				val=getattval(attnam[i],bp);
				if (vector)
					printf("%d ",val);
				else
					printf("%s= %d\n",attnam[i],val);
			}
			if (vector)
				printf("%s\n",classnam);
			else
				printf("\n");
			delete_blob(hdp.image,hdp.ocols,hdp.orows,
						bp->xst,bp->yst,FALSE) ;
			free_blobs(&blistptr);
			bp = make_blob(hdp.image,hdp.ocols,hdp.orows,&blistptr) ;
			bp->xst = 0 ; bp->yst = 0 ;
		}
		if (!blobsfound)
			perr(HE_MSG,"frame is empty");
	}
	return(0);
}

void readattfile(fp)
FILE *fp ;
{
	int i,j,k ;
	int endofline,instring ;
	char temp[20] ;

	fscanf(fp,"%d",&numatts) ;
	for (i=0;i<numatts;i++)
	{	fscanf(fp,"%s %s",attnam[i],temp) ;
		if (strcmp(temp,"integer"))
		{	endofline = FALSE ;
			instring = FALSE ;
			j=k=0 ;
			do
			{	fscanf(fp,"%c",temp);
				if (temp[0] == '\n')
					endofline = TRUE ;
				else
					if ((temp[0]==' ')||(temp[0]=='\t'))
					{	if (instring)
						{	instring = FALSE ;
							k = 0 ;
							j++  ;
						}
					}
					else
						instring = TRUE ;
			}
			while (!endofline) ;
		}
	}
#ifdef CLASSES
	fscanf(fp,"%d",&numclasses) ;
	for (i=0;i<numclasses;i++)
	{	fscanf(fp,"%s",classnam[i]) ;
	}
#endif
	fclose(fp) ;
}
