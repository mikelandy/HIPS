TOPDIR = /Users/msl/Documents/hips2/hips2
CONFIG = config.sonny
HERE = user-contributed/pbm/ppm/
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
# makefile for user-contributed/pbm/ppm
#

LIBNAME =	libppm.a

OBJECTS =	libppm1.o libppm2.o libppm3.o libppm4.o libppm5.o

PGMS =		giftoppm.out gouldtoppm.out ilbmtoppm.out imgtoppm.out \
		mtvtoppm.out \
		pcxtoppm.out pgmtoppm.out pi1toppm.out picttoppm.out \
		pjtoppm.out ppmdither.out ppmhist.out ppmmake.out ppmquant.out \
		ppmrelief.out ppmtogif.out ppmtoicr.out ppmtoilbm.out \
		ppmtopcx.out ppmtopgm.out ppmtopi1.out ppmtopict.out \
		ppmtopj.out ppmtopuzz.out ppmtorgb3.out ppmtosixel.out \
		ppmtotga.out ppmtouil.out ppmtoxpm.out ppmtoyuv.out \
		qrttoppm.out \
		rawtoppm.out rgb3toppm.out spctoppm.out sputoppm.out \
		tgatoppm.out ximtoppm.out xpmtoppm.out yuvtoppm.out ppmpat.out

SCRIPTS =	ppmquantall

MANPAGES =	giftoppm.n gouldtoppm.n ilbmtoppm.n imgtoppm.n mtvtoppm.n \
		pcxtoppm.n pgmtoppm.n pi1toppm.n picttoppm.n \
		pjtoppm.n ppmdither.n ppmhist.n ppmmake.n ppmquant.n \
		ppmrelief.n  ppmtogif.n ppmtoicr.n ppmtoilbm.n \
		ppmtopcx.n ppmtopgm.n ppmtopi1.n ppmtopict.n \
		ppmtopj.n ppmtopuzz.n ppmtorgb3.n ppmtosixel.n \
		ppmtotga.n ppmtouil.n ppmtoxpm.n ppmtoyuv.n qrttoppm.n \
		rawtoppm.n rgb3toppm.n spctoppm.n sputoppm.n \
		tgatoppm.n ximtoppm.n xpmtoppm.n yuvtoppm.n \
		ppmpat.n ppmquantall.n libppm.n ppm.n

PGMDIR =	../pgm
INCLUDEPGM =	-I$(PGMDIR)
LIBPGM =	$(PGMDIR)/libpgm.a

PBMDIR =	../pbm
INCLUDEPBM =	-I$(PBMDIR)
LIBPBM =	$(PBMDIR)/libpbm.a

RGBDEF =	-DRGB_DB=\"$(XLIB)/X11/rgb.txt\"

CPPFLAGSL = -I.. $(INCLUDEPGM) $(INCLUDEPBM) $(RGBDEF)
COMPILEc = $(CC) -D$(ORIG) -D$(BITORDER) $(CFLAGS) -DHIPS_IMAGE \
        $(CPPFLAGS) $(CPPFLAGSL) $(TARGET_ARCH) $(FASTCOPY) $(FLOAT) -c
LINKc = $(CC) -D$(ORIG) -D$(BITORDER) $(CFLAGS) -DHIPS_IMAGE \
        $(CPPFLAGS) $(CPPFLAGSL) $(FLOAT) $(TARGET_ARCH) $(FASTCOPY) -o

OTHERLIBS = $(LIBNAME) $(LIBPGM) $(LIBPBM) -lm

.c.o:
	$(COMPILEc) $*.c
	@-ld -s -r $*.o
	@mv a.out $*.o

$(LIBNAME):	$(OBJECTS)
	@rm -f $(LIBNAME)
	ar cr $(LIBNAME) `$(LORDER) $(OBJECTS) | $(TSORT)`;
	$(RANLIB) $(LIBNAME)

install:	$(LIBNAME) $(PGMS) man-pgm
	cp $(SCRIPTS) $(DESTDIR)
	cd $(DESTDIR) ; chmod +x $(SCRIPTS)

clean:	clean-pgm

man:
	$(MANPRINT) $(MANPAGES)

doc:

pristine:	pristine-pgm

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
