/* lut and ymap handling routines */

#include <hipl_format.h>
#include <xdisp.h>

void modify_color_cube();
void draw_colorcube_bars();

/*****************************************
 * create_lut()
 *****************************************/

/* Create a lut from a template of RGB values, and form a ymap */

lut_t *create_lut(lut_template,name,nentries)
  char *lut_template;
  char *name;
  int  nentries;
{
  int i;
  float resln;
  lut_t *lut;
  int datum = mentries-ncolors;
 
    lut = (lut_t *)malloc(sizeof(lut_t));

    lut->ymap = (unsigned char *)malloc(MAXCOLOR);
    lut->name = (char *)malloc(strlen(name));
    lut->lut_colors = (XColor *)malloc(mentries*sizeof(XColor));
    lut->size = nentries;
    strcpy(lut->name,name);

    resln = (float)ncolors/(float)(nentries);
    if (resln > 1.0) resln = 1.0;

    for (i = 0; i < nentries; i++)
	lut->ymap[i] = datum+(i*resln);

    for (i = nentries; i < MAXCOLOR; i++)
	lut->ymap[i] = lut->ymap[nentries-1];

    for (i = 0; i < mentries; i++) {
	lut->lut_colors[i].pixel = i;
	lut->lut_colors[i].flags = DoRed | DoGreen | DoBlue;
	}

    XQueryColors(dpy,app_colormap,lut->lut_colors,mentries);

    for (i = 0; i < nentries; i++) {
	lut->lut_colors[lut->ymap[i]].red = lut_template[i]*256;
	lut->lut_colors[lut->ymap[i]].green = lut_template[i+nentries]*256;
	lut->lut_colors[lut->ymap[i]].blue = lut_template[i+nentries*2]*256;
	}

    return(lut);
}


/*****************************************
 * set_lut()
 *****************************************/

/* set lut values into colormap */

void set_lut(lut)
  lut_t* lut;
{
  int i,j;
  int redraw = 0;

    if (current_lut->size != lut->size) redraw = 1;

    current_lut = lut;
    ymap = lut->ymap;

    if (frame >= nframes)
	modify_color_cube(); 
    else
    	for (i = 0; i < mentries; i++) {
	  colors[ymap[i]].red = lut->lut_colors[ymap[xmap[0][i]]].red;
	  colors[ymap[i]].green = lut->lut_colors[ymap[xmap[0][i]]].green;
	  colors[ymap[i]].blue = lut->lut_colors[ymap[xmap[0][i]]].blue;
	  }

    XStoreColors(dpy,colormap,colors,mentries);

    if (redraw && changing_lut) {
	changing_lut = 0;
	clear_pix_cache();
    	image_pix_data.width = image_pix_data.height = 0;
	resize(draw,&image_pix_data,NULL);
    	sample_pix_data.width = sample_pix_data.height = 0;
	resize(sample,&sample_pix_data,NULL);
    	colorbar_pix_data[0].width = colorbar_pix_data[0].height = 0;
	resize(colorbar[0],colorbar_pix_data+0,NULL);
	}
}


/*****************************************
 * set_xmap()
 *****************************************/

/* set xmap values from limits given */

void set_xmap(n,ll,ul)
  int 	n;
  int	ll;
  int	ul;
{
    int i;
    float resln = (float)MAXCOLOR/(float)(ul-ll);

    for (i = 0; i < MAXCOLOR; i++) {
	if (i <= ll)
	    switch(extreme_type) {
		case 0:	xmap[n][i] = 0;
			break;
		case 1:	xmap[n][i] = 0;
			break;
		case 2:	xmap[n][i] = MAXCOLOR-1;
			break;
		case 3:	xmap[n][i] = MAXCOLOR-1;
			break;
		}
	else if (i >= ul)
	    switch(extreme_type) {
		case 0:	xmap[n][i] = MAXCOLOR-1;
			break;
		case 1:	xmap[n][i] = 0;
			break;
		case 2:	xmap[n][i] = MAXCOLOR-1;
			break;
		case 3:	xmap[n][i] = 0;
			break;
		}
	else
	    xmap[n][i] = (i-ll)*resln;
	}

    gamma_limits[n].ll = ll;
    gamma_limits[n].ul = ul;
}


/************************
 * create_colorbar()
 ************************/

void create_colorbar()
{
    int i,j,fr;
    unsigned char *cbi[4];

    for (i = 0; i < 4; i++) {

    	cbi[i] = (unsigned char *)malloc(HIST_WIDTH*COLORBAR_HEIGHT);

    	colorbar_pix_data[i].im = cbi[i];
    	colorbar_pix_data[i].width = colorbar_pix_data[i].height = 0;
    	colorbar_pix_data[i].max_width = HIST_WIDTH;
    	colorbar_pix_data[i].max_height = COLORBAR_HEIGHT;
    	colorbar_pix_data[i].pix = NULL;
    	colorbar_pix_data[i].origin.r = colorbar_pix_data[i].origin.c = 0;
    	colorbar_pix_data[i].mag = 1;
    	colorbar_pix_data[i].gc = gc;
    	colorbar_pix_data[i].depth = depth;
    	colorbar_pix_data[i].pix_cache = NULL;
	}

    for (i = 0; i < COLORBAR_HEIGHT; i++)
	for (j = 0; j < HIST_WIDTH ; j++)
	        cbi[0][i*HIST_WIDTH+j] = j;
}


/************************
 * draw_colorcube_bars()
 ************************/

void draw_colorcube_bars()
{
    int i,j;

    for (i = 0; i < COLORBAR_HEIGHT; i++)
	for (j = 0; j < HIST_WIDTH; j++) {
	    colorbar_pix_data[1].im[i*HIST_WIDTH+j] = 			
			xmap[1][j]*NREDS/256*NBLUES*NGREENS;
	    colorbar_pix_data[2].im[i*HIST_WIDTH+j] = 
			xmap[2][j]*NGREENS/256*NBLUES;
	    colorbar_pix_data[3].im[i*HIST_WIDTH+j] = 
			xmap[3][j]*NBLUES/256;
	    }
}


/************************
 * create_ov_lut()
 ************************/

lut_t *create_ov_lut()
{
  lut_t *lut;
  unsigned char *lt;

    lt = (unsigned char *)malloc(3*NREDS*NBLUES*NGREENS*sizeof(unsigned char));
    lut = create_lut(lt,"ov",NREDS*NBLUES*NGREENS);
    free(lt);
    return(lut);
}


/************************
 * modify_color_cube()
 ************************/

void modify_color_cube()
{
  int i,j,k;
  int p = 0;
  unsigned char reds[NREDS];
  unsigned char greens[NGREENS];
  unsigned char blues[NBLUES];

    for (i = 0; i < NREDS; i++)
	reds[i] = (cmy_state) ? xmap[RED+1][(NREDS-1-i)*255/(NREDS-1)] :
			xmap[RED+1][i*255/(NREDS-1)];
    for (i = 0; i < NGREENS; i++)
	greens[i] = (cmy_state) ? xmap[GREEN+1][(NGREENS-1-i)*255/(NGREENS-1)] :
			xmap[GREEN+1][i*255/(NGREENS-1)];
    for (i = 0; i < NBLUES; i++)
	blues[i] = (cmy_state) ? xmap[BLUE+1][(NBLUES-1-i)*255/(NBLUES-1)] :
			xmap[BLUE+1][i*255/(NBLUES-1)];

    for (i = 0; i < NREDS; i++)
    	for (j = 0; j < NGREENS; j++)
    	    for (k = 0; k < NBLUES; k++,p++) {
		colors[ymap[p]].red = reds[i]*256;
		colors[ymap[p]].green = greens[j]*256;
		colors[ymap[p]].blue = blues[k]*256;
		}
}

