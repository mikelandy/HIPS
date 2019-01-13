/*
+ *************************************************************************** +
Program Name:  hips2imP.c
Date:          Aug, 1989.
Purpose:       
       To print an hips image on an imagen printer using, imPress code.


Intro:  
       First, a set of (up to 64) gray-level glyphs are downloaded
       to the Imagen imPress printer, and given a font name. Assuming,
       that the image (input) arriving at the stdin is of 0..255 graylevels,
       each pixel is rounded to  0..63 levels, and translated to to one of
       64 characters in this font. 


       Since each glyph is of  8 x 8 dots, the largest size of the image
       is governed by several factors.

       D = Dots Per Inch resolution of the printer.
       g = (2,4,8)  ==>  (0..3, 0..15, 0..63) gray levels. 
       H = Height,  W = Width of the paper in inches.

       N = min (H, W) * D /g 

       Correspondingly, the imagen printer's floating origin needs to be
       chosen as well.


NOTE:
       This is a quick and dirty implementation.  Variable size input has
       not been implemented yet.

+ ************************************************************************* +
*/

#include <stdio.h>
#include <math.h>
#include <hipl_format.h>


#define imp_SET_BOL     209
#define imp_PAGE        213
#define imp_SET_IL      208
#define imp_CRLF        197
#define imp_ENDPAGE	219
#define imp_EOF		255
#define imp_SET_FAMILY  207
#define imp_BGLY	199

FILE *ifp, *ofp;
unsigned char *pic;  /* [128][128]; */
struct header  hd;
unsigned char  buffer[512];
void CreateGlyphs(),putcode(),putbyte(),putword(),o_endpage(),o_close();
void LoadGlyph();

int main(argc, argv)
    int argc;
    char *argv[];
{
int i, j, size, xlen, ylen;

Progname=strsave(*argv);
ifp=stdin;
ofp=stdout;
read_header(&hd);
fprintf(stderr,"Original name: %s\n", hd.orig_name);
fprintf(stderr,"Sequence name: %s\n",hd.seq_name);
fprintf(stderr,"num_frame:     %d\n",hd.num_frame);
fprintf(stderr,"orig_date:     %s\n",hd.orig_date);
fprintf(stderr,"rows           %d\n",hd.orows);
fprintf(stderr,"cols           %d\n",hd.ocols); 
ylen=hd.orows;
xlen=hd.ocols;
size = hd.orows * hd.ocols;
fprintf(ofp,"@document(language impress,messagedetail on,jamresistance on,prerasterization on,jobheader off)");
CreateGlyphs();
putcode(imp_PAGE);
putcode(imp_SET_FAMILY); putbyte(1);
putcode(imp_SET_BOL); putword(750);
putcode(imp_SET_IL); putword(800);
putcode(imp_CRLF);
putcode(imp_SET_IL); putword(8);

/* Convert image to 64 grey scales, character values 32-96         */
/* The values were mapped onto [32, 96] because all the characters */
/* are printable, and this makes debugging easier.                 */
pic = (unsigned char *) malloc(size);
fread(pic, 1, size, stdin);
for(j=0; j<ylen; j++)  {
   for(i=0; i<xlen; i++) 
       buffer[i]=(pic[(j*xlen + i)]>>2)+32;
   fwrite(buffer, 1, xlen, ofp);
   putcode(imp_CRLF);
   }

if (argc>1) fclose(ifp);
if (argc>2) fclose(ofp);

o_endpage();
o_close();
}

void CreateGlyphs()
{
/* This function sets up 64  8x8 character glyphs, which represent
   gray scales 0 to 63.  The array pos[] tells which bit to set
   in the 8 byte array glyph[]. The ordering attempts to generate a
   decaying dot.
*/
unsigned char *glyph;
unsigned int i, j, k;
static short pos[]=
       {27, 36, 35, 28, 29, 44, 34, 19, 
        37, 43, 26, 20, 45, 42, 18, 21, 
        52, 33, 11, 30, 51, 25, 12, 38,
        53, 41, 10, 22, 50, 13, 60,  3,
        46, 17, 31, 32,  9, 14, 54, 49,
        59, 24,  4, 39, 58, 40,  2, 23,
        61, 16,  5, 47, 62,  1, 15, 57,
         8, 55, 48,  6, 63, 56,  0,  7 };

glyph=(unsigned char*)malloc(8);

for(i=0; i<64; i++){
   j= pos[i]>>3;          /* jth byte in glyph */
   k= 7 - ( pos[i] % 8) ; /*  set kth bit in glyph[j] */
   glyph[j] |= (0x1<<k);
   LoadGlyph(63-i, glyph, 8);
   }

free(glyph);
}

void LoadGlyph( name, glyph, size)
          unsigned int name, size;
          unsigned char *glyph;
/****************************************************************
  This function stores the passed glyph with its rotation, family
  and name + 32 (to map its name to a printable character).
*****************************************************************/
{
int j;

putcode(imp_BGLY);
putword((1<<7)+name+32); /* rotation, family, name + 32 */
putword(size); putword(size); putword(0); putword(size); putword(0);
for (j=0;j<size;j++) 
    putbyte(glyph[j]);
}


void o_close()
{
putcode(imp_EOF);
fclose(ofp);
}

void o_endpage()
{
putcode(imp_ENDPAGE);
}


void putcode(c)
int c;
{
putc(c,ofp);
}

void putbyte(b)
int b;
{
putc(b,ofp);
}

void putword(w)
int w;
{
putc( w>>8 ,ofp); 
putc( w&0xff ,ofp);
}

