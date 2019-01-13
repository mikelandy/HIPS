/*
 *	linemeans calculates the mean and variance of each scan line
 *	in one frame of a 3150 pixel TM quart scene.
 *	Leading and trailing bytes are unchanged.
 *	Output is a one-column HIPS file with one or two frames.
 *	If variance is output, output is bandinterleaved (mean,var,mean,var,..).
 *
 *	Header-lines are detected in a naive fashion.
 *
 *	Allan Aasbjerg Nielsen, IMSOR
*/
#include <hipl_format.h>
#define MAX_LS 1024

char *usage="usage - linemeans [-s] [-v] [-tm]";

int main(argc,argv)
int argc;
char **argv;
{
struct header hd;
int	i,j,fr;
int	nr,nc,nf,ncr;
byte	*frame;
int	*linestart;
int	ls,linesum,linesum2;
char	text[128];
int	sumflag=0,varflag=0,tmflag=0;
float	aux;

Progname = strsave(*argv); 

for (i=1;i<argc;i++) {
    if (argv[i][0]=='-') {
       switch(argv[i][1]) {
       case 's': sumflag=1;
		 break;
       case 't': tmflag=1;
		 break;
       case 'v': varflag=1;
		 break;
       default:  fprintf(stderr,"%s: %s\n",Progname,usage);
		 exit(1);
		 break;
       }
    }
}

read_header(&hd);
if (hd.pixel_format != PFBYTE) {
   fprintf(stderr,"%s: image must be byte format\n",Progname);
   exit(1);
}
if ((nf=hd.num_frame) != 1) {
   fprintf(stderr,"%s: one frame at a time\n",Progname);
   exit(1);
}
if (findparam(&hd,"Interleaving") != NULLPAR) {
   fprintf(stderr,"%s: bandinterleaved images not allowed\n",Progname);
   exit(1);
}
if (findparam(&hd,"Irregular") != NULLPAR) {
   fprintf(stderr,"%s: irregular images not allowed\n",Progname);
   exit(1);
}
nr=hd.orows;
if ((nc=hd.cols) < 3150 && tmflag==1) {
   fprintf(stderr,"%s: with -tm image must have at least 3150 columns\n",Progname);
   exit(1);
}
hd.pixel_format = PFFLOAT;
hd.orows=nr;
hd.rows=nr;
hd.ocols=1;
hd.cols=1;
/*setparam(&hd,"Irregular",PFBYTE,1,1);*/
if (varflag==1) {
   setparam(&hd,"Interleaving",PFBYTE,1,1);
   hd.num_frame=2;
}
update_header(&hd,argc,argv);
write_header(&hd);

/* frame holds one row at a time only */
frame = (byte *) malloc(nr*sizeof(byte));

for (fr=0;fr<nf;fr++) {
    for (i=0;i<nr;i++) {
	if (fread(frame,sizeof(byte),nc,stdin) != nc) {
	   fprintf(stderr,"%s: error reading row %d in frame %d\n",Progname,i,fr);
	   exit(1);
	}
	if (tmflag==1) {
	   ncr=3150;
	   linestart=(int *)(frame+24);
	   ls=(*linestart)+=32;
	   /*fprintf(stderr,"%d\n",ls);*/
	}
	else {
	   ls=0;
	   ncr=nc;
	}
	/*Check if line is header*/
	if (ls<MAX_LS) {
	   for (j=ls,linesum=0;j<ls+ncr;j++) linesum+=(int)frame[j];
	   /*
	   fprintf(stderr,"%s: frame %d line %d\n",Progname,fr,i);
	   if (j==ls+ncr-1) fprintf(stderr,"\n");
	   */
	   if (sumflag==1) {
	      aux=(float)linesum;
	      if ((fwrite(&aux,sizeof(float),1,stdout))!=1) {
		 fprintf(stderr,"%s: error writing sum of row %d\n",Progname,i);
		 exit(1);
	      }
	   }
	   else {
	      aux=(float)linesum/ncr;
	      if ((fwrite(&aux,sizeof(float),1,stdout))!=1) {
		 fprintf(stderr,"%s: error writing mean of row %d\n",Progname,i);
		 exit(1);
	      }
	   }
	   if (varflag==1) {
	      for (j=ls,linesum2=0;j<ls+ncr;j++)
	          linesum2+=((int)frame[j])*((int)frame[j]);
	      aux=(linesum2-(float)linesum*linesum/ncr)/(ncr-1);
	      if ((fwrite(&aux,sizeof(float),1,stdout))!=1) {
		 fprintf(stderr,"%s: error writing variance of row %d\n",Progname,i);
		 exit(1);
	      }
	   }
	}
	else {
	   for (j=0;j<128;j++) text[j]=(char)frame[j];
	   fprintf(stderr,"%s: header in line %d  %s\n",Progname,i,text);
	}
    }
}
exit(0);
}
