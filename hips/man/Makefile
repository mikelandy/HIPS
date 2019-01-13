TOPDIR = /Users/msl/Documents/hips2/hips2
CONFIG = config.sonny
HERE = hips/man/
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
# makefile for hips/man
#

MANPAGES = \
	abdou.n absdiffseq.n abspix.n addcmap.n adddesc.n addparam.n \
	addseq.n affine.n ahc3.n ahc3_r.n andseq.n applycmap.n atop.n \
	autodiff.n bandpass.n bandreject.n bclean.n binentropy.n \
	binquad.n binquad_r.n bnoise.n bpack.n btc.n btcsmooth.n \
	btorle.n calccomb.n calcpix.n calcwarp.n \
	catframes.n checkers.n clearroi.n \
	collage.n colorkey.n coltransf.n combine.n comicstrip.n \
	conjpix.n convertxhd.n \
	convolve.n correl.n cosum.n cutf.n cylindrate.n dct.n diffseq.n \
	discedge.n discedge2.n disphist.n dispwbasis.n dither.n \
	divseq.n dog.n dotdiff.n dpcm_r.n dpcm_t.n drift.n \
	dump_plot3d.n enlarge.n exppix.n extract.n extractroi.n \
	extremum.n fastcosum.n fcalccomb.n fcalcpix.n flipquad.n fourtr.n \
	fourtr3d.n framevar.n gabor.n gcube.n genframe.n genheader.n \
	getmax.n getmin.n \
	gmag.n gnoise.n gpoly.n grabheader.n grating.n greybar.n \
	gridwarp.n grot.n \
	gshift.n gsync.n halftone.n hc_bin.n hc_bin_r.n highpass.n \
	hipstosun.n hips2tohips1.n histo.n histoeq.n histostretch.n \
	htob.n htobgr.n htobgrz.n htoc.n htod.n htodc.n htof.n htoi.n \
	htolp.n htomp.n htorgb.n htorgbz.n htos.n htosb.n htoui.n \
	htous.n htozbgr.n htozrgb.n imgtopyr.n intercolor.n interdepth.n \
	interlace.n inv.dct.n inv.fourtr.n inv.fourtr3d.n \
	inv.walshtr.n joinframes.n logimg.n lowpass.n luminance.n \
	mask.n maskseq.n maxabsseq.n maxseq.n mean0.n median.n \
	minabsseq.n minseq.n morphdilate.n morpherode.n mulseq.n neg.n \
	noise.n nonisot.n oct.n oct_r.n orseq.n pad.n picinvert.n \
	pictranspose.n pixentropy.n pixto3d.n plot3topix.n plot3tov.n \
	powerpix.n prtdth.n prteps.n prthlf.n pshalftone.n pstrobe.n \
	psubseq.n ptoa.n pyrdisplay.n pyrexpand.n pyrextract.n \
	pyrmerge.n pyrreduce.n pyrtoimg.n reduce.n reflect.n repframe.n \
	rle.n rle_r.n rletob.n rmparam.n rotate180.n rotate90.n scale.n \
	scalemeansd.n seeheader.n \
	seehist.n seeplot.n setroi.n shiftpix.n sirds.n slice.n stretch.n \
	stretchpix.n stripheader.n strobe.n subsample.n subseq.n \
	suntohips.n thicken.n thin.n \
	thresh.n tmag.n transf.n trot.n tshift.n vectcode.n vectgen.n \
	view.n vmag.n vrot.n vshift.n walshtr.n wgauss.n wrapimg.n \
	xorseq.n zc.n zoneplate.n

install:	man-pgm

clean:	clean-pgm

man:
	$(MANPRINT) $(MANPAGES)

doc:

pristine:	pristineman-pgm
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
