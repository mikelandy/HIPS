TOPDIR = /Users/msl/Documents/hips2/HIPS/HIPS
CONFIG = config.sonny
HERE = user-contributed/pbm/pnm/
CCS_ROOT = /Users/msl/Documents/hips2/HIPS/HIPS/user-contributed/lbl

# Mac OSX 10.4 configuration file.
# The file "makefile.hdr" contains code common to all makefiles.  It
# should not be necessary to edit that file.

# The installation root for HIPS
INSTALLROOT = /Users/msl/HIPS


# DRIVERDIRS = Iris NeXT SunView X XView ctool sunanim xvanim xanim datacube adage
DRIVERDIRS = X
# CONTRIBDIRS = canny cc compress fsun imsor label lbl matlab pbm surrey tamu usl uva verdth
CONTRIBDIRS = canny cc compress fsun imsor lbl matlab pbm surrey tamu usl uva
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
# makefile for user-contributed/pbm/pnm
#

LIBNAME =	libpnm.a

OBJECTS =	libpnm1.o libpnm2.o libpnm3.o libpnm4.o

PGMS =		pnmarith.out pnmcat.out pnmconvol.out pnmcrop.out pnmcut.out \
		pnmdepth.out pnmenlarge.out pnmfile.out pnmflip.out \
		pnminvert.out \
		pnmnoraw.out pnmpaste.out pnmscale.out pnmtile.out pnmtops.out \
		pnmtorast.out pnmtoxwd.out rasttopnm.out xwdtopnm.out \
		pnmgamma.out pnmrotate.out pnmshear.out \
		tifftopnm.out pnmtotiff.out

MANPAGES =	pnmarith.n pnmcat.n pnmconvol.n pnmcrop.n pnmcut.n \
		pnmdepth.n pnmenlarge.n pnmfile.n pnmflip.n pnminvert.n \
		pnmnoraw.n pnmpaste.n pnmscale.n pnmtile.n pnmtops.n \
		pnmtorast.n pnmtoxwd.n rasttopnm.n xwdtopnm.n \
		pnmgamma.n pnmrotate.n pnmshear.n \
		anytopnm.n pnmindex.n pnmmargin.n pnmsmooth.n \
		tifftopnm.n pnmtotiff.n libpnm.n pnm.n

SCRIPTS =	anytopnm pnmindex pnmmargin pnmsmooth

TIFFDEF =	-DLIBTIFF
TIFFINC =	-I../tifflib
TIFFLIB =	-ltiff

PPMDIR =	../ppm
INCLUDEPPM =	-I$(PPMDIR)
LIBPPM =	$(PPMDIR)/libppm.a

PGMDIR =	../pgm
INCLUDEPGM =	-I$(PGMDIR)
LIBPGM =	$(PGMDIR)/libpgm.a

PBMDIR =	../pbm
INCLUDEPBM =	-I$(PBMDIR)
LIBPBM =	$(PBMDIR)/libpbm.a

CPPFLAGSL = -I.. $(INCLUDEPPM) $(INCLUDEPGM) $(INCLUDEPBM) $(TIFFDEF) $(TIFFINC)
COMPILEc = $(CC) -D$(ORIG) -D$(BITORDER) $(CFLAGS) -DHIPS_IMAGE \
        $(CPPFLAGS) $(CPPFLAGSL) $(TARGET_ARCH) $(FASTCOPY) $(FLOAT) -c
LINKc = $(CC) -D$(ORIG) -D$(BITORDER) $(CFLAGS) -DHIPS_IMAGE \
        $(CPPFLAGS) $(CPPFLAGSL) $(FLOAT) $(TARGET_ARCH) $(FASTCOPY) -o

OTHERLIBS = $(LIBNAME) $(LIBPPM) $(LIBPGM) $(LIBPBM) $(TIFFLIB) -lm -lz

#.c.o:
#	$(COMPILEc) $*.c
#	@-ld -s -r $*.o
#	@mv a.out $*.o

install:	$(LIBNAME) $(PGMS) man-pgm
	cp $(SCRIPTS) $(DESTDIR)
	cd $(DESTDIR) ; chmod +x $(SCRIPTS)

$(LIBNAME):	$(OBJECTS)
	@rm -f $(LIBNAME)
	ar cr $(LIBNAME) `$(LORDER) $(OBJECTS) | $(TSORT)`;
	$(RANLIB) $(LIBNAME)

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
