TOPDIR = /Users/msl/Documents/hips2/hips2
CONFIG = config.sonny
HERE = hips/sources/
CCS_ROOT = /Users/msl/Documents/hips2/hips2/user-contributed/lbl

# Mac OSX 10.4 configuration file.
# The file "makefile.hdr" contains code common to all makefiles.  It
# should not be necessary to edit that file.

# The installation root for HIPS
INSTALLROOT = /Users/msl/HIPS


# DRIVERDIRS = Iris NeXT SunView X XView ctool sunanim xvanim xanim datacube adage
DRIVERDIRS = X
# CONTRIBDIRS = canny cc compress fsun imsor label lbl matlab pbm surrey tamu usl uva verdth
CONTRIBDIRS = canny cc compress fsun imsor matlab pbm surrey tamu usl uva
MATINCLUDE = /Applications/MATLAB.app/extern/include
MEXPATH = /Applications/MATLAB.app/bin/mex
MATLABDEST = $(INSTALLROOT)/matlab
# SUNVDIRS = hipstool hipsview lblsun sunv
SUNVDIRS = hipstool hipsview
# HTDIRS = sunos3.4 ****or**** sunos4.0
HTDIRS = sunos4.0
# XDIRS = lblx11 imsorx11 xhips
XDIRS = xhips imsorx11
# XVDIRS = lblxview xvhips sheffield
XVDIRS = 
# DESTDIR = /usr/local/hips/bin
DESTDIR = $(INSTALLROOT)/bin
# LIBDIR = /usr/local/hips/lib
LIBDIR = $(INSTALLROOT)/lib
# INCLUDE_DIR = /usr/local/hips/include
INCLUDE_DIR = $(INSTALLROOT)/include
# MANDIR = /usr/man/mann
MANDIR = $(INSTALLROOT)/manl
MANEXT = l
# LIBMANDIR = /usr/man/mann
LIBMANDIR = $(INSTALLROOT)/manl
LIBMANEXT = l
TMPDIR = /tmp
MASKDIR = $(DESTDIR)/masks
MASK3DIR = $(DESTDIR)/3dmasks
LIB = hips
HLIB = hipsh
HALIB = hipsa

FLOAT = 
TARGET_ARCH = 
FASTCOPY = 
USESTDARG = -DHUSESTDARG
HNOVALUESH = -DHNOVALUESH

CC=gcc
CPPFLAGS = -I$(INCLUDE_DIR)
LIBRARYH = -l$(HLIB) -l$(LIB)
LIBRARY = -l$(HALIB) -l$(HLIB) -l$(LIB)
LIBRARY1 = -l$(HLIB)
LIBRARY2 = -l$(LIB)
HALIBRARY = -l$(HALIB)
LIBNAME = lib$(LIB).a
HLIBNAME = lib$(HLIB).a
HALIBNAME = lib$(HALIB).a
OTHERLIBS = -lm
OTHERLIB2 =
LDFLAGS = -L$(LIBDIR) '-Wl,-syslibroot,/Developer/SDKs/MacOSX10.6.sdk' -arch x86_64
CFLAGS = -O -g -mmacosx-version-min=10.6 -isysroot /Developer/SDKs/MacOSX10.6.sdk -arch x86_64
LDLIBFLAGS = -r -arch x86_64

ORIG = ULORIG
BITORDER = MSBFVERSION

MANPRINT = psroff -Pps -man
MEPRINT = psroff -Pps -me
TBL = tbl
EQN = eqn
RANLIB = ranlib
XVIEWHOME = 
LIBXVIEW = -lolgx -lX11
XINCLUDE = /usr/include
XLIB = /usr/X11R6/lib
D3HOME =
LORDER = lorder
TSORT = tsort
MEXEXT = mexmac

# Stuff you like to have removed from directories to "make clean"
OTHERJUNK = core *~ #* *BAK
################ Beginning of makefile.hdr ################
# Stuff that goes at the beginning of all the makefiles, but is not
# configuration parameters, should be in this file.  It is included
# after the configuration file, and before makefile.src.

DIRS =
PGMS =

SHELL = /bin/sh

COMPILEc = $(CC) -D$(ORIG) -D$(BITORDER) $(CFLAGS) -DHIPS_IMAGE \
	$(CPPFLAGS) $(TARGET_ARCH) $(FASTCOPY) $(USESTDARG) $(FLOAT) -c
LINKc = $(CC) -D$(ORIG) -D$(BITORDER) $(CFLAGS) -DHIPS_IMAGE \
	$(CPPFLAGS) $(FLOAT) $(USESTDARG) $(TARGET_ARCH) $(FASTCOPY) -o
LIBRARIES = $(LDFLAGS) $(LIBRARY) $(OTHERLIBS) $(OTHERLIB2)

.SUFFIXES:	.out

.c.o:
	$(COMPILEc) $<

.c.out:
	$(LINKc) $* $< $(LIBRARIES)
	strip $*
	mv -f $* $(DESTDIR)
	@touch $@
################ End of makefile.hdr ################
#
# makefile for hips/sources
#

PGMS = \
	calc_header \
	abdou.out absdiffseq.out abspix.out addcmap.out adddesc.out \
	addparam.out addseq.out affine.out ahc3.out ahc3_r.out \
	andseq.out applycmap.out atop.out autodiff.out bandpass.out \
	bandreject.out bclean.out binentropy.out binquad.out \
	binquad_r.out bnoise.out bpack.out btc.out btcsmooth.out \
	btorle.out calccomb.out calccombp.out \
	calcpix.out calcpixp.out calcwarp.out calcwarpp.out \
	catframes.out checkers.out clearroi.out collage.out \
	colorkey.out coltransf.out combine.out comicstrip.out \
	conjpix.out convertxhd.out \
	convolve.out correl.out cosum.out cutf.out cylindrate.out \
	dct.out diffseq.out discedge.out discedge2.out disphist.out \
	dispwbasis.out dither.out divseq.out dog.out dotdiff.out \
	dpcm_r.out dpcm_t.out drift.out dump_plot3d.out enlarge.out \
	exppix.out extract.out extractroi.out extremum.out \
	fastcosum.out fcalccomb.out fcalccombp.out \
	fcalcpix.out fcalcpixp.out flipquad.out \
	fourtr.out fourtr3d.out framevar.out gabor.out gcube.out \
	genframe.out genheader.out getmax.out getmin.out \
	gmag.out gnoise.out gpoly.out \
	grabheader.out grating.out greybar.out gridwarp.out \
	grot.out gshift.out \
	gsync.out halftone.out hc_bin.out hc_bin_r.out highpass.out \
	hipstosun.out hips2tohips1.out histo.out histoeq.out histostretch.out \
	htob.out htobgr.out htobgrz.out htoc.out htod.out htodc.out \
	htof.out htoi.out htolp.out htomp.out htorgb.out htorgbz.out \
	htos.out htosb.out htoui.out htous.out htozbgr.out \
	htozrgb.out imgtopyr.out \
	intercolor.out interdepth.out interlace.out inv.dct.out inv.fourtr.out \
	inv.fourtr3d.out inv.walshtr.out \
	joinframes.out logimg.out lowpass.out luminance.out mask.out \
	maskseq.out maxabsseq.out maxseq.out mean0.out median.out \
	minabsseq.out minseq.out morphdilate.out morpherode.out \
	mulseq.out neg.out noise.out nonisot.out oct.out oct_r.out \
	orseq.out pad.out picinvert.out pictranspose.out pixentropy.out \
	pixto3d.out plot3topix.out plot3tov.out powerpix.out prtdth.out \
	prteps.out prthlf.out pshalftone.out pstrobe.out psubseq.out \
	ptoa.out pyrdisplay.out pyrexpand.out pyrextract.out \
	pyrmerge.out pyrreduce.out pyrtoimg.out reduce.out reflect.out \
	repframe.out rle.out rle_r.out rletob.out \
	rmparam.out rotate180.out rotate90.out \
	subsample.out scale.out scalemeansd.out seeheader.out \
	seehist.out seeplot.out setroi.out \
	shiftpix.out sirds.out slice.out stretch.out stretchpix.out \
	stripheader.out strobe.out subseq.out \
	suntohips.out thicken.out thin.out \
	thresh.out tmag.out transf.out trot.out tshift.out vectcode.out \
	vectgen.out view.out vmag.out vrot.out vshift.out walshtr.out \
	wgauss.out wrapimg.out xorseq.out zc.out zoneplate.out

LIBRARY = $(LIBRARYH)

install:	$(PGMS)

clean:	clean-pgm

man:

doc:

pristine:	pristine-pgm

# Routines which require special parameters

calc_header:
	echo > calc_header.h
	echo "#define CC		\"$(CC)\"" >> calc_header.h
	echo "#define CFLAGS		\"$(CFLAGS)\"" >> calc_header.h
	echo "#define LDFLGS1		\"$(FLOAT)\"" >> calc_header.h
	echo "#define LDFLGS2		\"$(LDFLAGS)\"" >> calc_header.h
	echo "#define LDFLGS3		\"$(LIBRARY1)\"" >> calc_header.h
	echo "#define LDFLGS4		\"$(LIBRARY2)\"" >> calc_header.h
	echo "#define LDFLGS5		\"-I$(INCLUDE_DIR)\"" >> calc_header.h
	echo "#define LDFLGS6		\"$(OTHERLIB2)\"" >> calc_header.h
	echo "#define LDLIB		\"$(LIBDIR)\"" >> calc_header.h
	echo "#define TMPLIB		\"$(TMPDIR)\"" >> calc_header.h
	echo "#define ORIGFLAG		\"-D$(ORIG)\"" >> calc_header.h
	mv calc_header.h $(INCLUDE_DIR)

# Routines which must be treated specially
calcpixp.out: calcpixp.c
	$(COMPILEc) calcpixp.c
	mv calcpixp.o $(LIBDIR)
	@touch $@

fcalcpixp.out: fcalcpixp.c
	$(COMPILEc) fcalcpixp.c
	mv fcalcpixp.o $(LIBDIR)
	@touch $@

calccombp.out: calccombp.c
	$(COMPILEc) calccombp.c
	mv calccombp.o $(LIBDIR)
	@touch $@

fcalccombp.out: fcalccombp.c
	$(COMPILEc) fcalccombp.c
	mv fcalccombp.o $(LIBDIR)
	@touch $@

calcwarpp.out: calcwarpp.c
	$(COMPILEc) calcwarpp.c
	mv calcwarpp.o $(LIBDIR)
	@touch $@

FRC:
################ Beginning of makefile.tlr ################
# Stuff that goes at the end of all the makefiles, but is not
# configuration parameters should be in this file.  It is included
# after makefile.src.

#config reads the configuration file and munches the appropriate make files
config: doit
	${TOPDIR}/Configure "${CONFIG}" "${DEPTH}" "${HERE}" "${MFLAGS}"

config-subdirs: doit
	@-if test 'x$(DIRS)' != x ; then eval \
	    'for d in $(DIRS) ; do \
		    ( cd $$d ; \
		      echo Configuring ${HERE}$$d ; \
		      ${TOPDIR}/Configure "${CONFIG}" "${TOPDIR}" \
				"${HERE}$$d/" "${MFLAGS}" ) \
	    done' ; \
	else \
	    true ; \
	fi

doit:


# Clean up installed stuff and binaries
# pristine-pgm target invoked by pristine target in dirs that make programs
# 
pristine-pgm: clean-pgm
	@-if test 'x$(PGMS)' != x ; then eval \
		'for pgm in $(PGMS); do \
			dpgm=`basename $$pgm .out`; \
			echo rm -f $(DESTDIR)/$$dpgm ; \
			rm -f $(DESTDIR)/$$dpgm ; \
		done' ; \
	else \
	    true ; \
	fi

# Clean up binaries on program directories.
clean-pgm:
	-rm -f $(PGMS) *.out install* *.o $(OTHERJUNK) man-pgm libman-pgm
	-rm -f -r *.dSYM

# Install program manual pages
man-pgm: $(MANPAGES)
	@-if test ! -r $(MANDIR) ; then mkdir -p $(MANDIR) ; fi
	chmod u+w $(MANPAGES)
	for i in $(MANPAGES) ; \
	do \
		cp $$i $(MANDIR)/`basename $$i .n`.$(MANEXT) ; \
	done
	chmod u-w $(MANPAGES)
	touch man-pgm

# Install library manual pages
libman-pgm: $(MANPAGES)
	@-if test ! -r $(MANDIR) ; then mkdir -p $(MANDIR) ; fi
	chmod u+w $(MANPAGES)
	for i in $(MANPAGES) ; \
	do \
		cp $$i $(LIBMANDIR)/`basename $$i .n`.$(LIBMANEXT) ; \
	done
	chmod u-w $(MANPAGES)
	touch libman-pgm

# Uninstall program manual pages
pristineman-pgm:
	for i in $(MANPAGES) ; \
	do \
		rm -f $(MANDIR)/`basename $$i .n`.$(MANEXT) ; \
	done
	rm -f man-pgm

# Uninstall library manual pages
pristinelibman-pgm:
	for i in $(MANPAGES) ; \
	do \
		rm -f $(LIBMANDIR)/`basename $$i .n`.$(LIBMANEXT) ; \
	done
	rm -f libman-pgm

makefile:: makefile.src
	${DEPTH}Configure "${CONFIG}" "${DEPTH}" "${HERE}" "${MFLAGS}"

################ End of makefile.tlr ################
