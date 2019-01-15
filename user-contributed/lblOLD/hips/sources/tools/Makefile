TOPDIR = /Users/msl/Documents/hips2/hips2
CONFIG = config.sonny
HERE = user-contributed/lbl/hips/sources/tools/
CCS_ROOT = /Users/msl/Documents/hips2/hips2/user-contributed/lbl

# Mac OSX 10.4 configuration file.
# The file "makefile.hdr" contains code common to all makefiles.  It
# should not be necessary to edit that file.

# The installation root for HIPS
INSTALLROOT = /Users/msl/HIPS


# DRIVERDIRS = Iris NeXT SunView X XView ctool sunanim xvanim xanim datacube adage
DRIVERDIRS = X
# CONTRIBDIRS = canny cc compress fsun imsor label lbl matlab pbm surrey tamu usl uva verdth
CONTRIBDIRS = canny cc compress fsun imsor lbl matlab surrey tamu usl uva
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
# makefile for user-contributed/lbl/hips/sources/tools
#

PGMS =	append.out hexdump.out histo_simulate.out mdisphist.out \
	magicprint.out mem-test.out

PGM3 =	mainpeak.out mean_scale.out mhisto.out mixadd.out mixmul.out

PGM5 =	elastic.out frame.out \
	multi_mask.out powertool.out rotate90n.out

PGMC = to_stream.out to_jfif.out

# PSDIR set in config file
PSFLAGS = "-DLDLIB=\"$(LIBDIR)/magicprint.pro\""
MFONT = \"NewCenturySchlbk-Bold\"
FontFlag = -DMFont=$(MFONT)

LOCAL_INC = $(CCS_ROOT)/include
PUBLICLIB = -lccs -ltiff -lrle $(LIBRARY2) $(OTHERLIBS) $(OTHERLIB2) $(LOCALCFLAGS)
LIBRARIE1 = $(LDFLAGS) -lscs1 $(PUBLICLIB)
LIBRARIE2 = $(LDFLAGS) -lscs2 $(PUBLICLIB)
LIBRARIE3 = $(LDFLAGS) -lscs3 $(PUBLICLIB)
LIBRARIE5 = $(LDFLAGS) -lscs5 $(PUBLICLIB) -ljpeg -lpanel
LIBRARIEC = $(LDFLAGS) -ljpeg -lscs1 $(PUBLICLIB)
LOCALCFLAGS = $(CPPFLAGS) -I$(LOCAL_INC) -DHIPS_IMAGE -DCOMMON_TOOL \
	-DNO_STRUCT_AUTOINIT -DNO_SINGLE_WORD_REG -DNO_UNION_AUTOINIT
MASKLOC = -DMY_LIB=\"$(MASK3DIR)/mask\"

.c.out:
	$(LINKc) $* $< $(LIBRARIE2)
	mv $* $(DESTDIR)
	@touch $@

#install:	$(PGMS)	$(PGM3) $(PGM5) $(PGMC)
install:	$(PGMS)	$(PGM3) $(PGM5)

clean:	clean-pgm
clean-all:	clean_all

man:

doc:

pristine:	pristine-pgm


# Routines which require special parameters

$(PGMC):
	$(LINKc) $* $< $(LIBRARIEC) 
	mv $* $(DESTDIR)
	@touch $@

$(PGM3):
	$(LINKc) $* $< $(LIBRARIE3) $(CPL_FLAGS)
	mv $* $(DESTDIR)
	@touch $@

$(PGM5):
	$(LINKc) $* $< $(LIBRARIE5)
	mv $* $(DESTDIR)
	@touch $@

mdisphist.out:	mdisphist.c
	$(CC) -O -o mdisphist mdisphist.c $(LIBRARIE2)
	mv $* $(DESTDIR)
	@touch $@

# it is very important to test print magin for setting MGPOffset (0.04 - 0.6).
magicprint.out: magicprint.c
	$(LINKc) $* $< $(PSFLAGS) $(FontFlag) -DMGPOffset=0.2 $(LIBRARIE5)
	mv $* $(DESTDIR)
	cp magicprint.pro $(LIBDIR)
	chmod 664 $(LIBDIR)/magicprint.pro
	@touch $@

mkdest:
	@if [ ! -f magicprint.pro ];	then	\
		ln -s ../*.pro . ;	\
	else	true;	fi

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
