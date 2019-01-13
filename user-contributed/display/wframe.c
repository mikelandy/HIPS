/*
 *  wframe.c: display a hips image in a gfxtool and 
 *    optionally extract a rectangular portion of the image.
 *  Authors:  Paul W. Palumbo and Steven Tylock
 *    SUNY at Buffalo Department of Computer Science
 *    January, 1987
 *    
 *  This program contains some source from the hips 
 *    package.  Our original contributions are in the public domain.
 */

#include <hipl_format.h>
#include <usercore.h>
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <pixrect/pixrect_hs.h>

#define max(x,y) ((x)<(y) ? (y) : (x))
#define MASK8 0377
#define COLOR_TABLES "/users/grads/palumbo/lookup/"
#define XDEFAULT_VWSURF(ddname) {"","",0,ddname,0,256,"",0,0}
#define MAXLENGTH 256
#define  ZCAT "/usr/ucb/zcat "

/*int pixwindd();*/
int cgpixwindd();
/*int bw2dd();*/
struct vwsurf surface = XDEFAULT_VWSURF(cgpixwindd);

unsigned char *bp;
int power2[16] = { 32768, 16384, 8192, 4096, 2048, 1024, 512, 256, 128,
                      64, 32, 16, 8, 4, 2, 1 };
short *sp;
int Origin = FALSE;
int t=200,sr,sc,r,c,in_r,in_c;
int win_wid, win_hgt;
struct suncore_raster raster;
struct suncore_raster raster2;
char fname[256], color_table[256], table[256];
char seg_box_filename[256];
char cont_box_filename[256];
char cmd[256];

FILE *fp;
int factor, endit = TRUE, binary = FALSE;
int Color_seg = 255, Color_control = 255, Blink = 0;
FILE *color_file;
FILE *seg_box_file;
FILE *cont_box_file, *compfile;

main(argc,argv)
int argc;
char *argv[];
{
  struct header hd;
  int i, LUT=FALSE, seg_box = FALSE, cont_box = FALSE, Eight = FALSE;
  int Sleep = FALSE, shift_lut = 0, Low_resolution = FALSE;
  unsigned char *outpic;
  unsigned char *reduce();

  Progname = strsave(*argv);
  color_file = NULL;

  if (getenv("LOOKUP") != NULL) {
	strcpy(color_table,getenv("LOOKUP"));
	LUT = TRUE;
  }

  for (i=1; i<argc; ++i) {
   if (argv[i][0] == '-') {
     switch (argv[i][1]) {
       case 't': t = atoi(argv[++i]);
		 binary = TRUE;
                 break;
       case 'S': Color_seg = atoi(argv[++i]);
		 if (Color_seg == 0) Color_seg = 255;
		 else if (Color_seg == 255) Color_seg = 0;
                 break;
       case 'C': Color_control = atoi(argv[++i]);
		 if (Color_control == 0) Color_control = 255;
		 else if (Color_control == 255) Color_control = 0;
                 break;
       case 'c':
		strcpy(color_table,argv[++i]);
		LUT = TRUE;
		break;
       case 'b':
		strcpy(seg_box_filename,argv[++i]);
		seg_box = TRUE;
		break;
       case 'B':
		strcpy(cont_box_filename,argv[++i]);
		cont_box = TRUE;
		break;
       case '8': Eight = TRUE;
		  break;
       case 'l': Low_resolution = TRUE;
		 break;
       case 'L': shift_lut = atoi(argv[++i]);
		 fprintf(stderr,"Shift = %d\n",shift_lut);
		 break;
       case 's': Sleep = TRUE;
		  break;
       case 'n':
       case 'N': Blink = atoi(argv[++i]);
		 break;
       case 'O':
       case 'o': Origin = TRUE;
		  break;
			
       default:  (void)fprintf(stderr,"bad option: -%c\n",argv[i][1]);
		help();
                 exit(4);
     }
   }
   else { (void)strcpy (fname,argv[i]);}
  }
  if (LUT == TRUE){
  	strcat(color_table,".lu");
  	if ((color_file = fopen(color_table,"r")) == NULL) {
  		strcpy(table,COLOR_TABLES);
		strcat(table,color_table);
		if ((color_file = fopen(table,"r")) == NULL) {
			fprintf(stderr,"Not able to open color lookup table %s or %s\n",color_table,table);
			exit(1);
		}
  	}
  }
  if (seg_box == TRUE) {
	if ((seg_box_file = fopen(seg_box_filename,"r")) == NULL) {
		fprintf(stderr,"Not able to open segmentation box file %s\n",seg_box_filename);
		exit(1);
	}
  }
  if (cont_box == TRUE) {
	if ((cont_box_file = fopen(cont_box_filename,"r")) == NULL) {
		fprintf(stderr,"Not able to open control box file %s\n",cont_box_filename);
		exit(1);
	}
  }
	
  if ( fname[0] == '\0') { fp = stdin; Sleep = FALSE;}
  else if ((fp=fopen(fname,"r")) == NULL) 
	strcat(fname,".Z");


  if (Compressed(fname) == TRUE){
                strcpy(cmd,ZCAT);
                strcat(cmd,fname);
                if ((compfile  = popen(cmd,"r")) == NULL) {
                        fprintf(stderr,"extl: Can not open file: %s\n",fname);                        exit(1);
                }
                fp = compfile;
		fname[strlen(fname)-2] = '\0';
  } 
  fread_header(fp,&hd,fname);
  (void)strcat(fname,".extract");
  in_r = hd.orows;
  in_c = hd.ocols;

  if (hd.pixel_format != PFBYTE) {
	(void) fprintf(stderr,"Pixel format not BYTE!!\n");
	exit(1);
  }
  if ((bp = (unsigned char *) calloc(in_r,in_c)) == NULL) { 
    (void)fprintf(stderr,"bad calloc\n"); exit(1);
  }
  if (fread(bp,in_r*in_c,1,fp) != 1) {
        (void)fprintf(stderr,"bad read\n"); exit(2); }
  init_graphics(shift_lut);

  size_raster(&surface,0.0,1.0,0.0,1.0,&raster2);
  win_wid = raster2.width;
  win_hgt = raster2.height;
  outpic = reduce(bp,in_r,in_c,win_wid, &r, &c);

  sr = r;
  if (binary == TRUE) sc = (c+15) / 16;
  else sc = (c+1)/2;
  if ((sp = (short *) calloc(sr*sc,sizeof(short))) == NULL) {
    (void)fprintf(stderr,"bad calloc\n"); exit(1);
  }

   drawraster(sp,outpic,r,c);
   if (seg_box == TRUE) draw_seg_rects(seg_box_file,FALSE,Color_seg,Low_resolution);
   if (cont_box == TRUE) draw_cont_rects(cont_box_file,Eight,Color_control,Sleep,Low_resolution);
   for (;(endit ==TRUE);) {
  	drawbox(bp,outpic,fname,hd);
  	delete_retained_segment(1);
   }

  deselect_view_surface(&surface);
  terminate_core();
}

drawraster(sp,outpic,sr,sc)
register int sr,sc;
unsigned char *outpic;
short *sp;
{
  register int ic, itc, isc, i, j;
  unsigned char *bptemp;
  short *sptemp;
  ic = (sc);
  if (binary == TRUE) sc = (sc+15) / 16;
  else sc = (sc+1)/2;
  sptemp = sp;
  for (i=0; i< sr; ++i) {
    for (j=0; j< sc; ++j) { 
       *sptemp++ = 0;
    }
  }
  for (i=0; i< sr; ++i) {
    isc = i * sc;
    itc = i * ic;
    bptemp = outpic + itc;
    sptemp = sp + isc - 1;
    for (j=0; j< ic; ++j) { 
	if (binary == TRUE) {
      		if ((0377 & (*(bptemp + j))) < t) 
                   		*(sptemp + j / 16) |= power2[ j % 16 ];
	} else {

    	  	/* correct lookup table problem */
      		if (*bptemp == 0377) *bptemp = 0;
      		else if (*bptemp == 0) *bptemp = 0377;
      		if (j&01) {
        		*sptemp |= *bptemp;
      		}
      		else { 
        		++sptemp;
        		*sptemp = (*bptemp << 8);
      		}
      		++bptemp;
    	}
     }
  }

    create_retained_segment(1);
    raster.width = ic; 
    raster.height = sr; 
    raster.depth = (binary == TRUE?1:8); 
    raster.bits = sp;
    put_raster(&raster);
    close_retained_segment(1);
}

drawbox(bp,outpic,filename,hd)
unsigned char *bp, *outpic;
char filename[];
struct header hd;
{
  float X1,Y1,X2,Y2,x1,x2,y1,y2,x3,y3;
  float box_wid, box_hgt;
  float box_offsetx;
  int cut_offsetx, cut_offsety, point;
  int cut_wid, cut_hgt;

  int button = 2;

  set_echo(LOCATOR,1,1);
  while (button == 2) {
  	set_line_index(255);
  	set_linestyle(SOLID);
  	(void)printf("Left for pixel value, middle for box corner, right for Cancel\n");
  	await_any_button_get_locator_2(999999999,1,&button,&x1,&y1);
  	while (button == 1) {
		int X, Y, offset;
    		size_raster(&surface,0.0,1.0,0.0,1.0,&raster2);
    		win_wid = raster2.width;
    		win_hgt = raster2.height;
    		map_ndc_to_world_2(x1,y1,&X1,&Y1);
    		X = X1*win_wid;
		Y = r - Y1*win_hgt;
    		offset = X + Y*c;
		if (Origin == FALSE) Y = Y1*win_hgt;
    		if ((Y >= 0) && (Y < r) && (X >= 0) && (X < c)){
      			point = *(outpic + offset);
			if (point == 0) point = 255;
			else if (point == 255) point = 0;
      			(void)printf("coordinate (%d, %d) has value: %d\n",X,Y,point);
    		} else
      			(void)printf("coordinate (%d, %d) has value: <undefined>\n",X,Y);
    		await_any_button_get_locator_2(999999999,1,&button,&x1,&y1);
   	} 
   	if (button == 3) {
		 endit = FALSE;
		 return;
	}

  	set_echo_position(LOCATOR,1,x1,y1);
  	set_echo(LOCATOR,1,6);
  	(void)printf("Any button for second box corner\n");
  	await_any_button_get_locator_2(999999999,1,&button,&x2,&y2);
	
  	if (x2 < x1) {x3=x1; x1=x2; x2=x3;}
  	if (y2 < y1) {y3=y1; y1=y2; y2=y3;}
  	map_ndc_to_world_2(x1,y1,&X1,&Y1);
  	map_ndc_to_world_2(x2,y2,&X2,&Y2);
	
  	create_retained_segment(2);
	
    	set_rasterop(XORROP);
    	set_linestyle(DOTTED);
    	move_abs_2(X1,Y1);
    	line_abs_2(X2,Y1);
    	line_abs_2(X2,Y2);
    	line_abs_2(X1,Y2);
    	line_abs_2(X1,Y1);
    	move_abs_2(0.0,0.0);
    	line_abs_2(0.0,1.0);
    	line_abs_2(1.0,1.0);
    	line_abs_2(1.0,0.0);
    	line_abs_2(0.0,0.0);
  	close_retained_segment(2);
  	set_echo(LOCATOR,1,1);
	
    	size_raster(&surface,0.0,1.0,0.0,1.0,&raster2);
    	win_wid = raster2.width;
    	win_hgt = raster2.height;
  	box_wid = (X2 - X1) * win_wid;
  	box_hgt = (Y2 - Y1) * win_hgt;
  	cut_wid = (int) (box_wid + 0.5);
  	cut_hgt = (int) (box_hgt + 0.5);
  	box_offsetx = X1 * win_wid;
  	cut_offsetx = (int)(box_offsetx + 0.5);
  	cut_offsety = (int)(r - Y2*win_hgt + 0.5);
	  (void)fprintf(stderr,"box width = %d box height = %d\n",
					cut_wid*factor,cut_hgt*factor);
  	(void)fprintf(stderr,"offset x = %d  offset y = %d\n",
					cut_offsetx*factor,cut_offsety*factor);
  	if ((cut_offsety <0) || (cut_offsetx > c) || ((cut_offsety + box_hgt) > r)
     		|| ((cut_offsetx + box_wid) > c))
     	{
         	(void)fprintf (stderr,"block is not contained in picture, please try again\n");
	      	delete_retained_segment(2);
      		set_echo(LOCATOR,1,1);
      		button = 2;
     	} else {
		if (binary == TRUE) 
    			(void)printf("Use left button to cancel frame (or change threshhold), middle to redraw box, and right to extract\n");
		else 
    			(void)printf("Use left button to cancel frame, middle to redraw box, and right to extract\n");
  		await_any_button(999999999,&button);
  		if (button == 1) {
      			delete_retained_segment(2);
      			set_echo(LOCATOR,1,1);
      			endit = FALSE;
			if (binary == TRUE) {
      				(void)printf("Use left or middle button to cancel frame, and right to change threshold\n");
      				await_any_button(999999999,&button);
      				if (button == 3) {
        				int t1; char ans[100];
					endit = TRUE;
					(void)printf ("old thresh = %d, new thresh = [%d] ",t,t);
        				(void) gets(ans);
        				if (ans[0] != '\0') {
              					(void) sscanf(ans,"%d",&t);
        				}
					button = 4;
      				}
			}
    		}
  		if (button == 2) {
		      delete_retained_segment(2);
      			set_echo(LOCATOR,1,1);
     		}
  		if (button == 3) {
        		extract(cut_hgt*factor,cut_wid*factor,cut_offsety*factor,cut_offsetx*factor,bp,hd,filename);
        		delete_retained_segment(2);
			button = 4;
        		endit = FALSE;
   		}
  	}
    }
}

extract(height,width,y_off,x_off,ifr,hd,filename)
int height,width,y_off,x_off;
unsigned char *ifr;
struct header hd;
char filename[];
{
        int r,c,nr,nc,ir,ic,or,oc;
        int lc,rc,br,tr,i,j;
        unsigned char *ip,*op;
	unsigned char *ofr;
        FILE *fp;
	char Error[256];
 
        (void)fprintf(stderr,"Height = %d, Width = %d, y_off = %d , x_off = %d\n",height,width,y_off,x_off);
        r = hd.orows;
        c = hd.ocols;
        nr=r/2; nc=c/2;
        nr=height; ir=(r - ((nr<0) ? -nr : nr))/2;
        nc=width; ic=(c - ((nc<0) ? -nc : nc))/2;
        ir=y_off;
        ic=x_off;
 
        lc=ic; br=ir;
        if(nr<0) {
                br=ir+nr+1;nr=(-nr);
        }
        if(nc<0) {
                lc=ic+nc+1;nc=(-nc);
        }
 
        rc=lc+nc-1;
        tr=br+nr-1;
        
        if(lc<0)lc=0; if(br<0)br=0;
        if(rc>=c)rc=c-1; if(tr>=r)tr=r-1;
 
        or=tr-br+1;
        oc=rc-lc+1;
 
        if(or<=0 || oc<=0 ) {
                 (void) sprintf(Error,"extract: zero or negative dimension (or,oc) = (%d,%d).\n",or,oc);
                 error(Error,0);
        }
 
        hd.orows=or; hd.ocols=oc;
        if ((ofr = (unsigned char *) calloc((unsigned)or*oc,sizeof(unsigned char))) == 0) {
                error("extract: can't allocate core\n",0);
        }
                 
        (void)fprintf(stderr,"filename = %s\n",filename);
        if ((fp=fopen(filename,"w")) == NULL ) {
              (void) sprintf (Error,"cannot create %s\n",filename);
                error(Error,0);
        }
        fwrite_header(fp,&hd,filename);
        
        if(hd.pixel_format==PFBYTE) {
                ip = ifr + lc + br*c;
                op = ofr;
                for(i=0;i<or;i++) {
                        for(j=0;j<oc;j++)
                                *op++ = *ip++;
                        ip += c-oc;
                }
                if (fwrite(ofr,or*oc*sizeof(unsigned char),1,fp) != 1) {
                        error("extract: error during write\n",0);
                }
        }
        (void)fclose(fp);
        free(ofr);
}

/*
 *  reduce:  according to Landy et al. as in the Hips package
 **   usage: reduce [factor] < bigframe > smallframe
 **   default "factor": 2
 **   if "rows" or "cols" are not divisible by "factor", they are
 **   reduced to the highest number which is divisible by "factor".
  bp = reduce(bp,in_r,in_c,win_wid, &r, &c);
 */

unsigned char *
reduce (inpic,in_r,in_c,win_wid, rowsout, colsout)
unsigned char *inpic;
int in_r, in_c, win_wid;
int *rowsout, *colsout;
{
        register int i_fact_cols, f2, hf, sum, colsmf, factor_cols;
        register int i,j,k,mf;
        register unsigned char  *pin, *pout, *outpic ;

        factor = (max(in_r,in_c) + win_wid - 1) / win_wid;
        (*rowsout)=in_r/factor ;
        (*colsout)=in_c/factor ;
(void)fprintf(stderr,"in_r=%d in_c=%d win_wid=%d\n",in_r,in_c,win_wid);
        (void)printf("Factor = %d\n",factor);
        if (factor == 1) return(inpic);
        colsmf = (*colsout)*factor;
        factor_cols = factor* in_c;
        if (factor < 1 || factor > 512) {
                error("reduce: unreasonable factor\n",0);
        }

      if(!(outpic=(unsigned char *)calloc((unsigned)((*rowsout)*(*colsout)),sizeof(char)))){
                error("reduce: not enough core\n",0);
        }
        f2=factor*factor ;
        hf=f2/2 ;
        
        pout=outpic ;
        if (factor != 1) {
          for (i=0 ; i< (*rowsout) ; i++){
            i_fact_cols = i*factor_cols;
            for (mf=0 ; mf<colsmf ; mf += factor) {
                sum=hf ;
                pin=inpic + i_fact_cols + mf;
                for(j=0;j<factor;j++,pin+= in_c)
                  for (k=0 ; k<factor ; k++) sum+= ((*(pin+k)) & MASK8 ) ;
                *pout++ =(sum/f2) ;
            }     
          }
        }   
          
  return(outpic);
}

draw_seg_rects(box_file,Eight,Color,Low_resolution)
FILE *box_file;
int Eight,Color,Low_resolution;
{
	float x1, y1, x2, y2, x3,y3,x4,y4;
	int X1, Y1, X2, Y2, X3,Y3,X4,Y4;
	float FactorX, FactorY, inrange();
	
  	size_raster(&surface,0.0,1.0,0.0,1.0,&raster2);
  	win_wid = raster2.width;
  	win_hgt = raster2.height;

  	create_retained_segment(6);
	set_rasterop(NORMAL);
  	set_line_index(Color);
  	set_linestyle(SOLID);

	FactorX = factor*win_wid;
	FactorY = factor*win_hgt;

	if (Low_resolution == TRUE) {
		printf("Adjusting seg boxes for Low resolution\n");
		FactorX = FactorX * 4;
		FactorY = FactorY * 4;
	}

	while (1){
       	 	if (fscanf (box_file,"%d %d %d %d",&X1,&Y1,&X2,&Y2) != 4) break;
        	if (Eight == TRUE) {
                	if (fscanf (box_file,"%d %d %d %d",&X3,&Y3,&X4,&Y4) != 4) break;
                        x3 = X3/FactorX; x3 = inrange(x3);
                	x4 = X4/FactorX; x4 = inrange(x4);
                	y3 = Y3/FactorY; y3 = inrange(y3); /*y3 = 511 - y3;*/
                	y4 = Y4/FactorY; y4 = inrange(y4); /*y4 = 511 - y4;*/
        	}
	
  		x1 = X1 / FactorX; x1 = inrange(x1);
  		x2 = X2 / FactorX; x2 = inrange(x2);
  		y1 = Y1 / FactorY; y1 = inrange(y1); /*y1 = 511 - y1;*/
  		y2 = Y2 / FactorY; y2 = inrange(y2); /*y2 = 511 - y2;*/

	
		if (Eight == FALSE) {
  			/*fsLineDraw(fsfd,fb,x1,y1,x1,y2,Color);*/
  			/*fsLineDraw(fsfd,fb,x1,y1,x2,y1,Color);*/
  			/*fsLineDraw(fsfd,fb,x2,y2,x2,y1,Color);*/
  			/*fsLineDraw(fsfd,fb,x2,y2,x1,y2,Color);*/

    			move_abs_2(x1,y1);
    			line_abs_2(x1,y2);
    			line_abs_2(x2,y2);
	    		line_abs_2(x2,y1);
    			line_abs_2(x1,y1);

		} else {
  			/*fsLineDraw(fsfd,fb,x1,y1,x2,y2,Color);*/
  			/*fsLineDraw(fsfd,fb,x2,y2,x3,y3,Color);*/
  			/*fsLineDraw(fsfd,fb,x3,y3,x4,y4,Color);*/
  			/*fsLineDraw(fsfd,fb,x4,y4,x1,y1,Color);*/

	    		move_abs_2(x1,y1);
	    		line_abs_2(x2,y2);
	    		line_abs_2(x3,y3);
	    		line_abs_2(x4,y4);
    			line_abs_2(x1,y1);
		}	
	}
	close_retained_segment(6);
}

draw_cont_rects(box_file,Eight,Color,Sleep,Low_resolution)
FILE *box_file;
int Eight,Color,Sleep,Low_resolution;
{
	float x1, y1, x2, y2, x3,y3,x4,y4;
	int X1, Y1, X2, Y2, X3,Y3,X4,Y4;
	int First = 1, b, o, len, sync,i;
	float FactorX, FactorY, inrange();
	float conf, skew;
	char input_string[MAXLENGTH],string[MAXLENGTH];
	
  	size_raster(&surface,0.0,1.0,0.0,1.0,&raster2);
  	win_wid = raster2.width;
  	win_hgt = raster2.height;

	set_rasterop(NORMAL);
  	set_line_index(Color);
  	set_linestyle(SOLID);

	FactorX = factor*win_wid;
	FactorY = factor*win_hgt;
	
	printf("Low_resolution = %d\n",Low_resolution);

	if (Low_resolution == TRUE) {
		printf("Adjusting control boxes for Low resolution\n");
		FactorX = FactorX * 4;
		FactorY = FactorY * 4;
	}

	sync = 7;

	while (1){
	   if (fgets(input_string,MAXLENGTH,box_file) == NULL) break;
	   if (First != 1 && (Sleep == TRUE)) sleeping();
           First = 0;
	   if (Eight == FALSE) {
        	len = sscanf (input_string,"%d %d %d %d %d %f %d %f %s",&b,&X1,&Y1,&X2,&Y2,&conf,&o, &skew ,string);
        	if ((len != 8) && (len != 9)) break;
   	}
   	else {
        	len = sscanf (input_string,"%d %d %d %d %d %d %d %d %d %f %d %f %s",&b,&X1,&Y1,&X2,&Y2,&X3,&Y3,&X4,&Y4,&conf,&o,&skew,string);
        	if ((len != 12) && (len != 13)) break;
   	}


        	if (Eight == TRUE) {
                        x3 = X3/FactorX; x3 = inrange(x3);
                	x4 = X4/FactorX; x4 = inrange(x4);
                	y3 = Y3/FactorY; y3 = inrange(y3); /*y3 = 511 - y3;*/
                	y4 = Y4/FactorY; y4 = inrange(y4); /*y4 = 511 - y4;*/
        	}
	
  		x1 = X1 / FactorX; x1 = inrange(x1);
  		x2 = X2 / FactorX; x2 = inrange(x2);
  		y1 = Y1 / FactorY; y1 = inrange(y1); /*y1 = 511 - y1;*/
  		y2 = Y2 / FactorY; y2 = inrange(y2); /*y2 = 511 - y2;*/

  	        create_retained_segment(sync);
	
		if (Eight == FALSE) {
  			/*fsLineDraw(fsfd,fb,x1,y1,x1,y2,Color);*/
  			/*fsLineDraw(fsfd,fb,x1,y1,x2,y1,Color);*/
  			/*fsLineDraw(fsfd,fb,x2,y2,x2,y1,Color);*/
  			/*fsLineDraw(fsfd,fb,x2,y2,x1,y2,Color);*/

    			move_abs_2(x1,y1);
    			line_abs_2(x1,y2);
    			line_abs_2(x2,y2);
	    		line_abs_2(x2,y1);
    			line_abs_2(x1,y1);

		} else {
  			/*fsLineDraw(fsfd,fb,x1,y1,x2,y2,Color);*/
  			/*fsLineDraw(fsfd,fb,x2,y2,x3,y3,Color);*/
  			/*fsLineDraw(fsfd,fb,x3,y3,x4,y4,Color);*/
  			/*fsLineDraw(fsfd,fb,x4,y4,x1,y1,Color);*/

	    		move_abs_2(x1,y1);
	    		line_abs_2(x2,y2);
	    		line_abs_2(x3,y3);
	    		line_abs_2(x4,y4);
    			line_abs_2(x1,y1);
		}	
		printf("Block %d has confidence value %6.2f, orientation %d, skew %6.2f and Status %s\n",
          		b, conf, o, skew, string );
		close_retained_segment(sync);
		for (i=0; i<Blink; ++i) {
			set_segment_visibility(sync,FALSE);
			set_segment_visibility(sync,TRUE);
		}
		++sync;
	}
}

float inrange(coor)
float coor;
{
	if (coor<0.0) return(0.0);
	else if (coor > 1.0) return(1.0);
	else return(coor);
}

sleeping()
{
  char c;
 
 

     printf("Hit RETURN to Continue -> ");
     for(;;) {
         c=getchar();
         if (c=='\n') break;
     }   
}


Compressed(fname)
char *fname;
{
	int len;

	len = strlen(fname);
	if (len<2) return(FALSE);
	if (fname[len-2] == '.' && fname[len-1] == 'Z') return(TRUE);
	return(FALSE);
}
init_graphics(shift_lut)
int shift_lut;
{
  float red[256],green[256],blue[256];
  int redI[256],greenI[256],blueI[256];
  int i;

  if (initialize_core(BUFFERED,SYNCHRONOUS,TWOD)) exit(1);
  if (initialize_view_surface(&surface,FALSE)) exit(2);
  if (select_view_surface(&surface)) exit(3);
  size_raster(&surface,0.0,1.0,0.0,1.0,&raster2);
  win_wid = raster2.width;
  win_hgt = raster2.height;

  if (color_file == NULL) {
	  fprintf(stderr,"Using standard bw lookup table\n");
	  for (i=0; i<256; ++i)  
		blue[i] = green[i] = red[i] = inrange(((float)(i+shift_lut))/255); 
  } else {
	fprintf(stderr,"Using created lookup table\n");
  	for (i=0;i<256;++i) 
		fscanf(color_file,"%d",&redI[i]);
  	for (i=0;i<256;++i) 
		fscanf(color_file,"%d",&greenI[i]);
  	for (i=0;i<256;++i) 
		fscanf(color_file,"%d",&blueI[i]);
  	for (i=0;i<256;++i) {
		red[i] = inrange((redI[i]+shift_lut)/255.0);
		green[i] = inrange((greenI[i]+shift_lut)/255.0);
		blue[i] = inrange((blueI[i]+shift_lut)/255.0);
  	}
  }
  blue[255] = green[255] = red[255] = 0.0;
  blue[0] = green[0] = red[0] = 0.0;
  

  set_rasterop(NORMAL);
  define_color_indices(&surface,0,255,red,green,blue);

  set_ndc_space_2(1.0,1.0);
  set_viewport_2(0.0,1.0,0.0,1.0);
  set_window(0.0,1.0,0.0,1.0);

  initialize_device(LOCATOR,1);
  initialize_device(BUTTON,1);
  initialize_device(BUTTON,2);
  initialize_device(BUTTON,3);
  set_echo_surface(LOCATOR,1,&surface);
 
}

error(string,ret)
char string[];
int ret;
{
        char ans[256];

   (void)fprintf(stderr,"%s",string);
   if (ret != 1) {
        (void)fprintf(stderr,"ERROR:  Hit return to abort!!");
        (void)gets(ans);
        exit(1);
   }
}

help()
{
	fprintf(stderr,"wframe [-t <thresh>]\n");
	fprintf(stderr,"       [-C <box-color>] \n");
	fprintf(stderr,"       [-c <lut-file>] \n");
	fprintf(stderr,"       [-b <segmentation-box-file>] \n");
	fprintf(stderr,"       [-B <control-structure-box-file>] \n");
	fprintf(stderr,"       [-8 ]   {Format of Control box file is 4 points}\n");
	fprintf(stderr,"       [-L <LUT-shift-value>] \n");
	fprintf(stderr,"       [-l ]   {Use low resolution image for Box info} \n");
	fprintf(stderr,"       [-s/S ] {Use Sleep to display Control Results}\n");
	fprintf(stderr,"       [-o/O ] {Use Top Left as Origin of display}\n");
	fprintf(stderr,"       [-n/N <Number-of-Control-blinks>]\n");
	fprintf(stderr,"\nInput file can either be in command line or redirected\n");
}
