%!PS-Magicprint
%%magicprint prolog -- illustrations & labels
% UC - upper corner
% if igp, RS=3.3, but 3.75
% Jin Guojun	1/18/91
/vshowdict 4 dict def
/vshow {vshowdict begin /thestring exch def
	/lineskip vt def
	8 inch 10 inch moveto
 thestring
 {/charcode exch def
 /thechar () dup 0 charcode put def
 0 lineskip rmoveto
 gsave
	thechar stringwidth pop 2 div neg 0 rmoveto
	thechar show
 grestore
 }forall
 end}def

save 50 dict begin /magicprint exch def
/inch {72 mul}def
/chsign {-1. mul}def
/setimstr {/cols exch def /imstr cols string def}def
/placeim {sx sy translate}def
/scaleim {iw 2 mul cvi ih 2 mul cvi scale}def
/setcirc {/ang exch def /freq exch def freq ang
	{dup mul exch dup mul add 1 exch sub}
	setscreen}def
/setline {/ang exch def /freq exch def freq ang
	{pop abs 1 exch sub}
	setscreen}def
/setrc {/r exch def /c exch def}def
/setnb {/nb exch def}def
/setpos {/ih exch inch def /iw exch inch def
	/sy exch inch def /sx exch inch iw sub def}def
/setform {/FontW exch def /vt exch def /igp exch vt mul def
	/LeftBd exch inch def /Margin exch inch def
	/UC exch igp add def /Vpos UC def /RS 3.75 def}def
/nl	{/Vpos {Vpos vt sub}def LeftBd Vpos moveto}def
/newline {currentpoint pop LeftBd exch sub vt rmoveto} def
/Box {Margin 0 rmoveto	currentpoint exch pop dup UC exch sub 0 exch rlineto
	8.5 inch LeftBd 2 mul Margin 2 mul add sub 0 rlineto
	0 exch UC sub rlineto closepath stroke} def
/Hput {4.25 inch exch FontW mul sub igp add exch translate 0 0 moveto}def
/Vput {inch exch FontW mul sub exch 4.5 add inch exch moveto 90 rotate}def
/Rotate {/rangle exch def /sita exch def /radius exch inch def
	/x sita sin radius mul def /y sita cos radius mul def
	iw x sub sx add ih y sub sy add translate rangle rotate}def
/imgo	{c r nb [c 0 0 r chsign 0 r]
	{currentfile imstr readhexstring pop} image}def
/imusd	{c r nb [c 0 0 r 0 0]
	{currentfile imstr readhexstring pop} image}def
/restorestate {clear magicprint end restore}def
%end prolog -- begin data
