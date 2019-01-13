#include <stdio.h>
FILE *hfP = stdout;
FILE *fopen();

op_help(mbuf)			/* help */
char *mbuf;
{
	static helpsw = '4';

	if(mbuf[0]=='\0')
		mbuf[0] = helpsw;
	if(mbuf[0]=='1'){
		fprintf(hfP, "\n?2|3|4 - prints more help.\n");
		helpsw = '1'; help1();
	}else if(mbuf[0]=='2'){
		fprintf(hfP, "\n?1|3|4 - prints more help.\n");
		help2(); helpsw = '2';
	}else if(mbuf[0]=='3'){
		fprintf(hfP, "\n?1|2|4 - prints more help.\n");
		help3(); helpsw = '3';
	}else if(mbuf[0]=='4'){
		fprintf(hfP, "\n?1|2|3 - prints more help.\n");
		help4(); helpsw = '4';
	}
}

#define P(m) fprintf(hfP, m)
help1()
{
fprintf(hfP, "\n			view.help  (V1.0)\n");
fprintf(hfP, "         HIPSVIEW: A gray Scale Display & Analysis Tool\n\n");
 help_mouse();
}

help2()
{
help_slice();
help_file();
help_show();
help_hist();
}

help3()
{
help_draw();
help_reset();
help_render();
help_param();
}

help4()
{
help_misc();
}

write_helpfile()
{
	if((hfP=fopen("view.help", "w"))!=NULL){
		help1(); help2(); help3(); help4();
		printf("\nWrote view.help to pwd\n");
		fclose(hfP);
	} else {
		printf("\nCan't open view.help\n");
	}

	hfP = stdout;
}

help_mouse()
{
P("		**** Help => Mouse & Toggle Cmds ****\n");
P("  mouse sw\n");
P("  LEFT   - Drags box or slice cursors when in image area.\n");
P("         - Plot graphs on down sw transitions & when mouse\n");
P("             becomes still or leaves window.\n");
P("  MIDDLE - Prints values/average of boxed pixels when IN sp/sa MODE.\n");
P("         - Moves plot cursor & updates picture cursor\n");
P("             to match it when IN sx, sy, sv MODES.\n");
P("         - Draws threshold mapped binary image when IN st MODE,\n");
P("             lo & hi thresholds are mouse selected from histogram plot.\n");
P("         - Draws magnified image when IN sm MODE, pixels within\n");
P("             boxed region are expanded to fit hipsview window.\n");
P("  LT&MID - Rubberband box IN sa, shw, sm, sp, sw MODES or vector IN sv MODE.\n");
P("  RIGHT  - Show menues.\n");
P("  MID&RT - Toggle cursor arrow between picture & plot.\n");
P("  MID&RT - Toggle image via mouse keys. Same as period key.\n");
P("    .    - Toggle between magnified & non-mag image IN sm MODE.\n");
P("    .    - Toggle between Threshold Mapped Binary & non-TMB image.\n");
P("    /    - Toggle cursor arrow between image & plot IN sx, sy, sv MODES.\n");
P("    ,    - Toggle cursor arrow color to enhance visibility via comma key.\n");
P("\n");
}

help_slice()
{
P("		**** Slice => cmds ****\n");
P("  sx - set Slice X plot mode.\n");
P("  sy - set Slice Y plot mode.\n");
P("  sv - set Slice Vector plot mode.\n");
P("  a  - print Average, Sigma, Range, Max & Min values of boxed pixels. \n");
P("  p  - Print boxed pixels values & above statistics.\n");
P("             Boxed pixel values may be appended with the following\n");
P("                   L:low  H:high  c:clipped &  z:zone-filled codes,\n");
P("                   and > marks the current x y cursor position.\n");
P("             Pixel values in slice plot text are marked by c & z codes.\n");
P("  H  - select boxed Hi pixel.\n");
P("  L  - select boxed Lo pixel.\n");
P("  s0 - iterate Select HL off.\n");
P("  s1 - iterate Select HL on. Enable peak/valley hill climb search algorithm.\n");
P("\n");
}

help_file()
{
P("		**** File => Cmds ****\n");
P("  l name - Load file \"name\".\n");
P("  l\"     - reLoad previously named file.\n");
P("  sw     - set Show Write boxed pixel mode & mouse drag the boxed area.\n");
P("  w name - Write boxed image if in sw mode. Same format as input image.\n");
P("         - Write sliced pixels to file name if in sx, sy, sv mode.\n");
P("         - Write boxed pixels if in sp mode. In p format, see slice=>cmds.\n");
P("  w\"     - overWrite previously named file.\n");
P("  q #    - Quit\n");
P("\n");
}

help_show()
{
P("		**** Set Show Mode => Cmds ****\n");
P("  sa     - set Show Average & Sigma of boxed pixels.\n");
/*P("  sac    -     As above except UL corner of box is the center of pixel area.\n");*/
P("  sp     - set Show Print boxed pixels.\n");
P("                 Print above on MID sw press, see slice=>cmds a & p.\n");
P("  sm # # - set Show Magnify boxed pixels mode. Magnify on MID sw press.\n");
P("                 #'s are magnification (wd ht) scale factors.\n");
P("  st     - set Show histogram Threshold mapped binary image,\n");
P("           MIDDLE sw selects (low hi) thresholds from histogram plot.\n");
P("           The second sw press draws the binary image.\n");
P("\n");
}

help_hist()
{
P("		**** Histogram => Cmds ****\n");
P("sh|shf - Set hist generate mode to use full  image. Plot mode is unchanged.\n");
P("   shw - Set hist generate mode to use boxed image. Plot mode is unchanged.\n");
P("   h   - Set hist plot mode to standard   & plot the histogram.\n");
P("   he  - Set hist plot mode to equalized  & plot the equalized histogram.\n");
P("   hi  - Set hist plot mode to integrated & plot the integrated histogram.\n");
P("\n");
}

help_draw()
{
P("		**** Draw image => Cmds ****\n");
P("  d      - Draw normal/raw image.\n");
P("  de     - Draw histogram Equalized image.\n");
P("  dm # # - Draw Magnified image, #'s are magnification (wd  ht) scales.\n");
P("  dt # # - Draw Threshold mapped binary image, #'s are (low hi) threshold.\n");
P("\n");
}

help_reset()
{
P("		**** Reset => Cmds ****\n");
P("  rd     - ReDisplay.\n");
P("  ri     - ReInitialize hipsview <image window set to 512X512>.\n");
P("  rs     - ReSize window to fit current raw image.\n");
P("  rw # # - Set image Window to new (wd ht) and resize window to fit.\n");
P("\n");
}

help_render()
{
P("		**** Render => Cmds ****\n");
P("  I  - Interchange foreground & background, ie. colormap bytes 0 & 255.\n");
P("  i  - Invert grayscale ramp map & load color lut with it.\n");
P("  D0 - Sun canvas Dither mode OFF. Color/GrayScale Sun default.\n");
P("  D1 - Sun canvas Dither mode ON . Monochrome Sun default.\n");
P("  D2 - Sun canvas Dither invert mode ON .\n");
P("\n");
}

help_param()
{
P("		**** Show & Set Parameter => Cmds ****\n");
P("  u # # - set box wd ht in s[ampw] & shw modes & ap box wd ht in s[xyv] modes.\n");
P("  v #   - set box    ht in s[ampw] & shw modes & ap box    ht in s[xyv] modes.\n");
P("  x # # - set upper left box or slice XY coord.\n");
P("  y #   - set upper left box or slice Y  coord.\n");
P(" du # # - set vector lengths du dv in sv mode.\n");
P(" dv #   - set vector length  dv    in sv mode.\n");
P("              The second arg in cmds u, x & du is optional.\n");
P("  c #'s - set image Clip&scale points args=(lo hi). Enter r to disable clipping.\n");
P("  z #'s - set image pix fill Zone args=(lo hi val). Enter r to disable filling.\n");
P("  s   - Status printed.\n");
P("  %% # - Change print boxed pixels format.\n");
P("  > # - Print current working directory & execute chdir system call.\n");
P("  ! # - Execute system cmd.\n");
P("          The above Cmds always print the current # arg values.\n");
P("\n");
}

help_misc()
{
P("		 **** Help => Misc (non menu) Cmds ****\n");
P("  ? # - Prints help files 1 to 4.\n");
P("  C   - Reload Color Map with previously loaded Sun Raster File color map.\n");
P("  w view.help - Writes view.help file to pwd.\n");
P("\n");
P("  image state <Bn.m Hf Cn Sp>	Code is printed by status s cmd.\n");
P("     where\n");
P("       B  is Base  pic  type [n, e], the .m means magnify mode is on.\n"); 
P("       H  is Hist  mode [f, w]  f:full image  w:windowed image.\n");
P("       C  is Current pic type [n, e, m, t].\n");
P("       S  is Show mode [a, h, m, p, t, w, x, y, v].\n");
P("\n");
P("    Image File Header:    HIPS format\n");
P("      hdtype:\n");
P("      PFBYTE\n");
P("      PFSHORT\n");
P("      PFINT\n");
P("      PFFLOAT\n");
P("\n");
}

