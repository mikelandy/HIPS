TOPDIR = /Users/msl/Documents/hips2/hips2
CONFIG = config.sonny
HERE = user-contributed/lbl/jpeg/libsrc/
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
# makefile for jpeg/jpeg-v6/lib
#
# This makefile is suitable for Unix-like systems with any C compilers.

LIBNAME = libjpeg.a

LOCAL_INC = -I. -I$(CCS_ROOT)/include
LOCALCFLAGS = $(LOCAL_INC) $(JPEG_FLAGS) -DCCS_ECL -DHAVE_JPEG_FRAME_MARKER

# In particular:
#   Please turn on any extra-warning switches you have.
#   For example, I normally use "-Wall" with gcc.
#   Add -DBSD if on a pure BSD system (see jinclude.h).
#   Add -DMEM_STATS to enable gathering of memory usage statistics.

# Put here the object file name for the correct system-dependent memory
# manager file.  For Unix this is usually jmemnobs.o, but you may need
# to use jmemansi.o or jmemname.o if you have limited swap space.
# (For now, just use jmemnobs.o, 'cause the other options ain't ready.)
SYSDEPMEM= jmemnobs.o

# source files: JPEG library proper
LIBSOURCES= jcapimin.c jcapistd.c jccoefct.c jccolor.c jcdctmgr.c jchuff.c \
	jcinit.c jcmainct.c jcmarker.c jcmaster.c jcomapi.c jcparam.c \
	jcphuff.c jcprepct.c jcsample.c jctrans.c jdapimin.c jdapistd.c \
	jdatadst.c jdatasrc.c jdcoefct.c jdcolor.c jddctmgr.c jdhuff.c \
	jdinput.c jdmainct.c jdmarker.c jdmaster.c jdmerge.c jdphuff.c \
	jdpostct.c jdsample.c jdtrans.c jerror.c jfdctflt.c jfdctfst.c \
	jfdctint.c jidctflt.c jidctfst.c jidctint.c jidctred.c jquant1.c \
	jquant2.c jutils.c jmemmgr.c jmemansi.c jmemname.c jmemnobs.c \
	jmemdos.c
# source files: cjpeg/djpeg/jpegtran applications, also rdjpgcom/wrjpgcom
APPSOURCES= cjpeg.c djpeg.c jpegtran.c cdjpeg.c rdcolmap.c rdswitch.c \
	rdjpgcom.c wrjpgcom.c rdppm.c wrppm.c rdgif.c wrgif.c rdtarga.c \
	wrtarga.c rdbmp.c wrbmp.c rdrle.c wrrle.c
SOURCES= $(LIBSOURCES) $(APPSOURCES)
# files included by source files
INCLUDES= jchuff.h jdhuff.h jdct.h jerror.h jinclude.h jmemsys.h jmorecfg.h \
	jpegint.h jpeglib.h jversion.h cdjpeg.h cderror.h
# library object files common to compression and decompression
COMOBJECTS= jcomapi.o jutils.o jerror.o jmemmgr.o $(SYSDEPMEM)
# compression library object files
CLIBOBJECTS= jcapimin.o jcapistd.o jctrans.o jcparam.o jdatadst.o jcinit.o \
	jcmaster.o jcmarker.o jcmainct.o jcprepct.o jccoefct.o jccolor.o \
	jcsample.o jchuff.o jcphuff.o jcdctmgr.o jfdctfst.o jfdctflt.o \
	jfdctint.o
# decompression library object files
DLIBOBJECTS= jdapimin.o jdapistd.o jdtrans.o jdatasrc.o jdmaster.o \
	jdinput.o jdmarker.o jdhuff.o jdphuff.o jdmainct.o jdcoefct.o \
	jdpostct.o jddctmgr.o jidctfst.o jidctflt.o jidctint.o jidctred.o \
	jdsample.o jdcolor.o jquant1.o jquant2.o jdmerge.o

# object files for sample applications (excluding library files)
COBJECTS= rdppm.o rdtarga.o rdbmp.o rdswitch.o jpeg_w.o
DOBJECTS= wrppm.o wrtarga.o wrbmp.o rdcolmap.o jpeg_r.o
#COBJECTS= cjpeg.o rdppm.o rdgif.o rdtarga.o rdrle.o rdbmp.o rdswitch.o cdjpeg.o
#DOBJECTS= djpeg.o wrppm.o wrgif.o wrtarga.o wrrle.o wrbmp.o rdcolmap.o cdjpeg.o

IO_OBJS = $(COBJECTS) $(DOBJECTS) errorini.o strmjpeg.o sdatadst.o cdjpeg.o
# These objectfiles are included in libjpeg.a
LIBOBJECTS= $(CLIBOBJECTS) $(DLIBOBJECTS) $(COMOBJECTS) $(IO_OBJS)
ANSI_OBJ = $(LIBOBJECTS)

default install: $(ANSI2KNR) $(LIBNAME)

ANSI2KNR = cp	# make non-ANSI anyway
include ansi-knr

# libjpeg.a is useful if you are including the JPEG software in a larger
# program; you'd include it in your link, rather than the individual modules.

.c.o:
	$(COMPILEc) $(LOCALCFLAGS) $*.c

$(LIBNAME):     $(LIBOBJECTS)
	@$(RM) -f $(LIBNAME) tmpansi.c
	ar cr $(LIBNAME) `$(LORDER) $(LIBOBJECTS) | $(TSORT)`;
	mv $(LIBNAME) $(LIBDIR)
	$(RANLIB) $(LIBDIR)/$(LIBNAME)
	@touch $(LIBNAME)

$(INCLUDES):
	cp -p $(INCLUDES) $(INCLUDE_DIR)

clean:
	$(RM) -f *.o $(LIBNAME)
clean-all:	clean_all

jconfig.h: jconfig.doc
	echo You must prepare a system-dependent jconfig.h file.
	echo Please read the installation directions in install.doc.
	exit 1

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
