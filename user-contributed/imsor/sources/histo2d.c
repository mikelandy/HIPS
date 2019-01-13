/************************************************************************/
/*                        2-D Histogram                                 */
/*                                                                      */
/*  Compile:    cc -o 2dhisto 2dhisto.c -lhips                          */
/************************************************************************/

/*****************************************************************************
*  File       :                                     Date: 20/1-93
*  Author     : Bent Foss Pedersen.
*  Purpose    : To show a 2D-histogram
*  Description:
*  Features   :
*  Options    : -f frame#1 frame#2 -t traeningssaet.hips -c scatter_cut_vaerdi
*               -z -e -comic -nomem
******************************************************************************/

#include	<hipl_format.h>
#include	<stdio.h>
#include        <stdlib.h>

#define TRUE 1
#define FALSE 0
#define NOELMHIST (256*256)

static		Flag_Format flagfmt[] = {
		
		{"f",{LASTFLAG},2,
		 {  {PTBOOLEAN,"FALSE"},
		    {PTINT,"0","framenr1"},
		    {PTINT,"1","framenr2"},
		   LASTPARAMETER
		  }
		 },
		{"T",{LASTFLAG},1,
		 {{PTBOOLEAN,"FALSE"}, {PTFILENAME,"","trainsetfile"},
		   LASTPARAMETER}
		 },
		{"c",{LASTFLAG},1,
		 {{PTBOOLEAN,"FALSE"}, {PTINT,"0","cut_value"},     
		   LASTPARAMETER}
		 }, 
		 {"e",{LASTFLAG},0,
		 {{PTBOOLEAN,"FALSE"},     
		   LASTPARAMETER}
		 },
		 {"z",{LASTFLAG},0,
		 {{PTBOOLEAN,"FALSE"},     
		   LASTPARAMETER}
		 },	  
                 {"comic",{LASTFLAG},0,
		 {{PTBOOLEAN,"FALSE"},     
		   LASTPARAMETER}
		 }, 
		 {"nomem",{LASTFLAG},0,
		 {{PTBOOLEAN,"FALSE"},     
		   LASTPARAMETER}
		 },	  
		LASTFLAG
		
		};	/* Declaration of commandline parameters	*/

int		types[] = {PFBYTE,LASTTYPE};	/* Input formats	*/


byte get_antalseg();
long fact();
long komb();
void mulhist2d();
void scatter();
void hist2d();
void twoframes();

h_boolean fflag, tflag, cflag, eflag, zflag, comicflag, nomemflag;
int cut_value;
long imagesfileposition;
byte **pointframes;

/* Hovedprogram, kalder mulhist2d for at generere histogrammer for to      */
/* frames ud fra traeningssaettet, hvis dette er angivet. Finder ud af     */
/* om -f option er benyttet, hvis den er kaldes kun en gang med de to      */
/* frames, ellers kaldes med alle kombinationer af alle frames.            */

int main(argc,argv)
int	argc;
char **argv;
{
  struct header hd, trainhead;
  Filename	filename, trainsetfile;
  FILE		*fp, *tfp;
  int           *hist;
  byte          i,j, *frame1,*frame2, *trainimg, maxt;
  int           framenr1, framenr2;
  long		nsize;
  int           nbands,nrows,ncols;

  Progname = strsave(*argv);
  parseargs(argc,argv,flagfmt,
  &fflag, &framenr1, &framenr2, 
  &tflag, &trainsetfile,
  &cflag, &cut_value,
  &eflag, &zflag, 
  &comicflag,
  &nomemflag,
  FFONE,&filename);	/* Reading of commandline	*/

  fp = hfopenr(filename);		/* Opening of inputfile	*/
  fread_hdr_cpf(fp,&hd,types,filename);	/* Reading of header */
  imagesfileposition=ftell(fp);
  nrows = hd.orows;
  ncols = hd.ocols; 
  nsize = (long)(nrows * ncols);
  nbands = hd.num_frame;
 
  fprintf(stderr,"\n");
  
  if (nbands==1)
    perr(HE_MSG,"There must be more than one frame in file");

  hist = (int *) hmalloc(256*256*sizeof(int));
  trainimg = (byte *) hmalloc(nsize*sizeof(byte));  	
  
  if (nomemflag)
  {
    frame1 = (byte *) hmalloc(nsize*sizeof(byte));
    frame2 = (byte *) hmalloc(nsize*sizeof(byte));
   }
   else
   {
     pointframes = (byte**) hmalloc(nsize*sizeof(byte*));
     for (i=0; i< nbands; i++)
     {
       pointframes[i] = (byte*) hmalloc(nsize*sizeof(byte));
       if (fread(pointframes[i], sizeof(byte), nsize, fp) != nsize)
         perr(HE_MSG,"Error during read");
      }
    }
       
  if (tflag) 
  {  
    tfp = hfopenr(trainsetfile);		/* Opening of trainsetfil */
    fread_hdr_cpf(tfp,&trainhead,types,filename);  /* Reading of header */

    if (trainhead.orows != nrows)
      perr(HE_MSG,"Not the same number of rows in trainset as in image");

    if (trainhead.ocols != ncols)
       perr(HE_MSG,"Not the same number of collumns in trainset as in image");

    if (trainhead.num_frame != 1)
       perr(HE_MSG,"More than one frame in trainset, using frame 0"); 

    if (trainhead.pixel_format != PFBYTE)
       perr(HE_MSG,"Trainingset must be of the type BYTE");
  
    if (fread(trainimg, sizeof(byte), (int)nsize, tfp) != nsize)
      perr(HE_MSG,"Error during read of trainset");

    fclose(tfp);

    maxt=get_antalseg(trainimg, nsize);
  }  
  else
  { 
    eflag = TRUE;
    zflag = FALSE;
    maxt = 0;
   }

  hd.orows = 256;      /* Stoerrelse af histogram = 256 	*/
  hd.ocols = 256;      
  hd.pixel_format = PFINT;
  hd.cols = 256;
  hd.rows = 256;
  hd.frow = 0;
  hd.fcol = 0; 
  
  if (fflag)
    hd.num_frame=( int ) (maxt+eflag+zflag);    /* antal histogramer */
  else
    hd.num_frame=( int ) komb(nbands,2)*(maxt+eflag+zflag);  /* antal histogramer */

  write_headeru(&hd,argc,argv);	/* Write histo2D header	*/
 
  if (fflag)
  {
    fprintf(stderr,"Combines frame#%d and frame#%d:\n", framenr1, framenr2);
    twoframes((byte)framenr1, (byte)framenr2, &frame1, &frame2, nsize, fp);
    mulhist2d(frame1, frame2, trainimg, maxt, nsize, hist);
  }  
  else
  {
    if (comicflag) /* Histogrammerne skal beregnes i baglaens raekkefoelge. */
    {
      for(i=(byte)nbands-2; i!=255; i--)
        for(j=((byte)nbands-1); j>i; j--)
        {
           fprintf(stderr,"Combines frame#%d and frame#%d:\n", i,j);
           twoframes(i, j, &frame1, &frame2, nsize, fp);
           mulhist2d(frame1, frame2, trainimg, maxt, nsize, hist);
         }
  fprintf(stderr, 
    "The comicstip command should be: ... | comicstrip -s %ld | rotate90 | ...\n",
                                                             komb(nbands,2));
    }
    else
    {
      for (i=0; i<((byte)nbands-1); i++)
        for(j=i+1; j<=((byte)nbands-1); j++)
        {
           fprintf(stderr,"Combines frame#%d and frame#%d:\n", i,j);
           twoframes(i, j, &frame1, &frame2, nsize, fp);
           mulhist2d(frame1, frame2, trainimg, maxt, nsize, hist);
         }
       } /* (else comicflag) */
   } /* (else fflag) */
  fclose(fp);
}

/* Funktion der finder antallet af klasser i traeningssaettet. */
byte get_antalseg(trset, noelm)
byte *trset;
long noelm;
{       
  byte maxt = 0;
  long i;
  for(i=0; i<noelm; i++, trset++)
    if ((*trset)>maxt) maxt=(*trset);
  return(maxt);
}

/* Funktion der udregner fakulteten til et tal */
long fact(n)
int n;
{
  long i;
  for (i=1; n>1; i*=(long)n, n--);
  return(i);
}

/* Funktion der udregner alle kombinationer af et vis antal ud af en andet  */
/* vis antal. Benytter a!/(b!*(a-b)!)                                       */
long komb(a,b)
long a,b;
{
  return (fact((long)a)/(fact((long)b)*fact((long)(a-b))));
}

/* Subroutine der danner flere 2D-histogrammer ud fra to frames             */
/* (ligger som array), samt et 'traeningssaet'. Danner et ud fra hele       */
/* billedet, samt ud fra to frames. Desuden benyttes en evt. CUT-vaerdi     */
/* til at danne scatter-plot.                                               */
/* Returnere ikke noget, skriver histogrammer til standart output           */
/* maxt angiver stoerste vaerdi i tset (= antal segmenter)                  */
/* (kan godt vaere 0, som indikation om at intet traeningssaet er angivet)  */

void mulhist2d(frame1, frame2, tset, maxt, noelm, hist)

byte *frame1, *frame2, *tset, maxt;
long noelm;
int *hist;
{
  int i;

  /* Foerst hele skaermen, bliver beregnet hvis eflag TRUE. */
  if (eflag)
  { 
    hist2d(frame1, frame2, tset, 0, NOELMHIST, TRUE, hist);
    if (cflag) scatter(hist);
    if (fwrite(hist, sizeof(int), NOELMHIST, stdout) != NOELMHIST)
      perr(HE_MSG, "Error during write");
    } 
  /* Saa segmenterne, hvis der er nogen. */

  if (tflag)
  { 
    for(i=(int)(! zflag) ; i<=maxt; i++)
    {
      hist2d(frame1, frame2, tset, i, noelm, FALSE, hist);
      if (cflag) scatter(hist);
      if (fwrite(hist, sizeof(int), NOELMHIST, stdout) != NOELMHIST)
        perr(HE_MSG, "Error during write");
    }
  }
}

/* Subroutine der danner scatterplot ud fra en cut vaerdi.                  */
/* Alle de vaerdier der er i hist, som er over den globale variabel         */
/* cut_value's vaerdi bliver maxint, og under bliver 0                      */
void scatter(hist)
int *hist;

{
  long i=0;
  int *h;
  h=hist;

  for (i=0; i<NOELMHIST; i++, h++)
  {
    if ((*h)<=cut_value)
      *h=0;
    else
      *h=255;
  }
}

/* Subroutine der danner 2D-histogram ud fra to frames(ligger som array),   */
/* samt et 'traeningssaet' med tilhoerende variabel(tvalue) som fortaeller  */
/* hvilket segment der bruges. Hvis heleskaerm er sat er traeningssaet      */
/* irrelevant, og der beregnes for hele skaermen. Returnere histogram       */
/* Der forudsaettes, at saafremt heleskaerm ikke er sat, skal der vaere et  */
/* traeningssaet.                                                           */

void hist2d(frame1, frame2, tset, tvalue, noelm, heleskaerm, hist)

byte *frame1, *frame2, *tset, tvalue;
long noelm;
h_boolean heleskaerm;
int *hist;
{
  byte *f1, *f2, *t;
  long n;
  int *h;

  h= hist;
  fprintf(stderr, "Makes 2Dhistogram of ");
  if (heleskaerm)
    fprintf(stderr, "the entiere image\n");
  else
    fprintf(stderr, "class #%d\n", tvalue);

  for(n= 0; n<(long)(NOELMHIST); n++,h++) (*h)=0; /* Initialiserer histogrammet med 0     */

  f1=frame1; f2=frame2; t=tset;
  for(n= 0; n<noelm; n++, f1++, f2++, t++)  /* Laver histogram */
    if (((*t)==tvalue)||(heleskaerm)) {
      if (comicflag)
        hist[((255-(int)(*f1)))*256+(int)(255-(*f2))]++;
                  /* (0,0) i nederste hoejre hjoerne, til brug ved comicstrip */
      else
        hist[(255-((int)(*f2)))*256+(int)(*f1)]++;
                                        /* (0,0) i nederste venstre hjoerne */
    }
}

/* Subroutine der laeser to frames ind ud fra en fil pointer og stoerrelsen */
/* af disse. Forudsaetning: imagesfileposition skal pege paa frame 0's      */
/* start og f1 og f2 skal tilhoere {0,1,..}.                                */

void twoframes(f1, f2, frame1, frame2, noelm, fp)

byte **frame1, **frame2, f1, f2;
long noelm;
FILE *fp;

{
  byte mf;

   if (f1==f2)
     perr(HE_MSG,"The choosen frames are the same");
       
  if (f2<f1)
  {
    mf = f2; f2 = f1; f1 = mf;
   }
 
 if (nomemflag)
 { 
    if (!fflag) fseek(fp, imagesfileposition, 0); 
                 /* Laeser frem til start paa frame 0, */
                 /* hvis der er mere end to frames.    */
    fseek(fp, (long)(noelm*f1), 1);    /* soeger fra start(=fp) til 1. frame   */
    
    if (fread((*frame1), sizeof(byte), noelm, fp) != noelm)
      perr(HE_MSG,"Error during read");
  
    fseek(fp, (long)(noelm*(f2-f1-1)), 1); /* finder starten paa 2. frame      */

    if (fread((*frame2), sizeof(byte), noelm, fp) != noelm)
      perr(HE_MSG,"Error during read");
  } /* nomemflag */
  else
  {
     (*frame1) = pointframes[f1];  
     (*frame2) = pointframes[f2]; 
   } /* (else nomemflag) */  
}
