TOPDIR = /Users/msl/Documents/hips2/hips2
CONFIG = config.sonny
HERE = user-contributed/lbl/jpeg/lib/libscs/
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
# makefile for user-contributed/lbl/libscs
#

OBJECTS1 = convertu.o sysenv.o
OBJECTS2 = convertu2.o sysenv2.o
OBJECTS3 = convertu3.o sysenv3.o
OBJECTS4 = convertu4.o sysenv4.o
OBJECTS5 = convertu5.o sysenv5.o
OBJECTS6 = convertu6.o sysenv6.o
OBJECTS= $(OBJECTS1) $(OBJECTS2) $(OBJECTS3) $(OBJECTS4) $(OBJECTS5) $(OBJECTS6)

LOCALINC = $(CCS_ROOT)/include
LIBNAME = libscs
LIB1 = $(LIBNAME)1.a
LIB2 = $(LIBNAME)2.a
LIB3 = $(LIBNAME)3.a
LIB4 = $(LIBNAME)4.a
LIB5 = $(LIBNAME)5.a
LIB6 = $(LIBNAME)6.a
LIBS = $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6)

COMMON_FLAGS = -DFITS_IMAGE
COLOR_FLAGS = -DRLE_IMAGE -DEXTENDED_U_IMAGE
LIBFLAG1 = $(COMMON_FLAGS) -D_DEBUG_
LIBFLAG2 = $(COMMON_FLAGS) $(COLOR_FLAGS) -D_DEBUG_
LIBFLAG3 = $(COMMON_FLAGS) $(COLOR_FLAGS) -DCOMMON_TOOL
LIBFLAG4 = $(LIBFLAG3) -DPICT_IMAGE
LIBFLAG5 = $(LIBFLAG4) -DJPEG_IMAGE
LIBFLAG6 = $(LIBFLAG5) -DWRITE_ANY_IMAGE

default:	$(OBJECTS) $(LIBS)
install:	$(OBJECTS) $(LIBS)

$(OBJECTS1):	$(LOCALINC)/function.h
	$(COMPILEc) $(LIBFLAG1) -I$(LOCALINC) $<
	@-ld $(LDLIBFLAGS) $*.o
	@mv a.out $*.o

$(OBJECTS2):	$(LOCALINC)/function.h
	$(COMPILEc) $(LIBFLAG2) -I$(LOCALINC) $<
	@-ld $(LDLIBFLAGS) $*.o
	@mv a.out $*.o

$(OBJECTS3):	$(LOCALINC)/function.h
	$(COMPILEc) $(LIBFLAG3) -I$(LOCALINC) $<
	@-ld $(LDLIBFLAGS) $*.o
	@mv a.out $*.o

$(OBJECTS4):	$(LOCALINC)/function.h
	$(COMPILEc) $(LIBFLAG4) -I$(LOCALINC) $<
	@-ld $(LDLIBFLAGS) $*.o
	@mv a.out $*.o

$(OBJECTS5):	$(LOCALINC)/function.h
	$(COMPILEc) $(LIBFLAG5) -I$(LOCALINC) $<
	@-ld $(LDLIBFLAGS) $*.o
	@mv a.out $*.o

$(OBJECTS6):	$(LOCALINC)/function.h
	$(COMPILEc) $(LIBFLAG6) -I$(LOCALINC) -I$(RLEDIR)/include $<
	@-ld $(LDLIBFLAGS) $*.o
	@mv a.out $*.o

$(LIB1):        $(OBJECTS1)
	@rm -f $(LIB1)
	ar cr $(LIB1) `$(LORDER) $(OBJECTS1) | $(TSORT)`;
	mv $(LIB1) $(LIBDIR)
	$(RANLIB) $(LIBDIR)/$(LIB1)
	@touch $(LIB1)

$(LIB2):        $(OBJECTS2)
	@rm -f $(LIB2)
	ar cr $(LIB2) `$(LORDER) $(OBJECTS2) | $(TSORT)`;
	mv $(LIB2) $(LIBDIR)
	$(RANLIB) $(LIBDIR)/$(LIB2)
	@touch $(LIB2)

$(LIB3):        $(OBJECTS3)
	@rm -f $(LIB3)
	ar cr $(LIB3) `$(LORDER) $(OBJECTS3) | $(TSORT)`;
	mv $(LIB3) $(LIBDIR)
	$(RANLIB) $(LIBDIR)/$(LIB3)
	@touch $(LIB3)

$(LIB4):        $(OBJECTS4)
	@rm -f $(LIB4)
	ar cr $(LIB4) `$(LORDER) $(OBJECTS4) | $(TSORT)`;
	mv $(LIB4) $(LIBDIR)
	$(RANLIB) $(LIBDIR)/$(LIB4)
	@touch $(LIB4)

$(LIB5):        $(OBJECTS5)
	@rm -f $(LIB5)
	ar cr $(LIB5) `$(LORDER) $(OBJECTS5) | $(TSORT)`;
	mv $(LIB5) $(LIBDIR)
	$(RANLIB) $(LIBDIR)/$(LIB5)
	@touch $(LIB5)

$(LIB6):        $(OBJECTS6)
	@rm -f $(LIB6)
	ar cr $(LIB6) `$(LORDER) $(OBJECTS6) | $(TSORT)`;
	mv $(LIB6) $(LIBDIR)
	$(RANLIB) $(LIBDIR)/$(LIB6)
	@touch $(LIB6)

clean:
	rm -f *.o $(LIBNAME)*.a
clean-all:	clean_all

pristine:
	rm -f *.o $(LIBNAME)*.a $(LIBDIR)/$(LIBNAME)*.a

man:

doc:
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
