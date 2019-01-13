#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

int TestTime()
{
  FILE *stream;
  char NOH[32],filename[256],o_Code[255],Limit[100];
  int kb_Faux, kb_Vrai;
  char MP[200];
  char MP1[32];
  char MP2[32];
  char MP3[32];
  char Tab[3][9];
  char PW[4];
  int l[3];
  char *source[3];
  char c;
  char D[13];
  long time1;
  struct tm  *time2;
  int i, j, k;

  kb_Faux = -1;
  kb_Vrai = 0;
  
  gethostname(NOH,31);
  if ((char *) getenv("ALLEGORY") == (char *) NULL) {
    fprintf(stderr,"Sorry, you should set the ALLEGORY environment variable first.\n");
    exit(kb_Faux);}
  sprintf(filename,"%s/CONFIG/%s.config",getenv("ALLEGORY"),NOH);
  stream = fopen(filename,"r");
  if (stream == (FILE *) NULL) {
    fprintf(stderr,"Unable to open file %s\n",filename);
    exit(kb_Faux);}
  fscanf(stream,"*Magic.Clef : \"%s\n",o_Code);
  if (fscanf(stream,"*Magic.Limite : \"") != EOF)
    fscanf(stream,"%s\"\n",Limit);
  else sprintf(Limit,"nolimit");
  
  Tab[0][8] = 0;
  Tab[1][8] = 0;
  Tab[2][8] = 0;

  PW[0] = '-';
  PW[1] = 'p';
  PW[2] = 'h';
  PW[3] = 0;

  l[0] = strlen(NOH);
  l[1] = strlen(PW);
  l[2] = strlen(Limit);
  
  source[0] = NOH;
  source[1] = PW;
  source[2] = Limit;
  
  for (i = 0; i < 3; i++)
    {
      for (j = 0; j < 8; j++)
	{
	  if (j >= l[i]) {c = source[i][l[i]-1];} else {c = source[i][j];}
	  k = 3 * j + i;
	  Tab[k / 8][k % 8] = c;
	}
    }
  
  sprintf(MP1, "%s", crypt(Tab[0], "PH"));
  sprintf(MP2, "%s", crypt(Tab[1], "FR"));
  sprintf(MP3, "%s", crypt(Tab[2], "XC"));
  
  sprintf(MP, "%s%s%s", MP1+2, MP2+2, MP3+2);

  if ( strncmp(o_Code,MP,33) == 0)
    {
      if ( strcmp(Limit,"nolimit") == 0)
	{
	  return kb_Vrai;     
	}
      time1 = time(NULL);
      time2 = localtime(&time1);
      i = strftime(D, 13, "%Y%m%d", time2);
      free((char *)time2);
      
      if (strncmp(D,Limit,8) > 0)
	{
	  return kb_Faux;     
	}
      return kb_Vrai;
    }
  return kb_Faux;
}
