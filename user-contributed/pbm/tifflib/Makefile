TOPDIR = /Users/msl/Documents/hips2/HIPS/HIPS
CONFIG = config.sonny
HERE = user-contributed/pbm/tifflib/
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
# Tag Image File Format Library
#
# Copyright (c) 1988, 1989, 1990, 1991, 1992 Sam Leffler
# Copyright (c) 1991, 1992 Silicon Graphics, Inc.
# 
# Permission to use, copy, modify, distribute, and sell this software and 
# its documentation for any purpose is hereby granted without fee, provided
# that (i) the above copyright notices and this permission notice appear in
# all copies of the software and related documentation, and (ii) the names of
# Sam Leffler and Silicon Graphics may not be used in any advertising or
# publicity relating to the software without the specific, prior written
# permission of Stanford and Silicon Graphics.
# 
# THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
# EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
# WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
# 
# IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
# ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
# LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
# OF THIS SOFTWARE.
#
IPATH=	-I.
#
# Library-wide configuration defines:
#    MMAP_SUPPORT	add support for memory mapping read-only files
#    COLORIMETRY_SUPPORT add support for 6.0 colorimetry tags
#    JPEG_SUPPORT	add support for 6.0 JPEG tags & JPEG algorithms
#    YCBCR_SUPPORT	add support for 6.0 YCbCr tags
#    CMYK_SUPPORT	add support for 6.0 CMYK tags
#
# Compression configuration defines:
#    CCITT_SUPPORT	add support for CCITT Group 3 & 4 algorithms
#    PACKBITS_SUPPORT	add support for Macintosh PackBits algorithm
#    LZW_SUPPORT	add support for LZW algorithm
#    THUNDER_SUPPORT	add support for ThunderScan 4-bit RLE algorithm
#    NEXT_SUPPORT	add support for NeXT 2-bit RLE algorithm
#    JPEG_SUPPORT	add support for JPEG DCT algorithm
#
# Note that if you change the library-wide configuration, you'll
# need to manual force a full rebuild.  Changing the configuration
# of which compression algorithms are included in the library is
# automatically handled (i.e. tif_compress.o has a dependency on
# the Makefile).
#
CONF_LIBRARY=\
	-DHAVE_IEEEFP=1 \
	-DUSE_VARARGS=0 \
	-DUSE_PROTOTYPES=1 \
	${NULL}
LOCALCFLAGS =	-O ${IPATH} ${CONF_LIBRARY} $(TIFF_FLAGS)
#
INCS=	tiff.h tiffio.h tiffconf.h tiffvers.h

OBJS=	tif_aux.o \
	tif_close.o \
	tif_codec.o \
	tif_color.o \
	tif_compress.o \
	tif_dir.o \
	tif_dirinfo.o \
	tif_dirread.o \
	tif_dirwrite.o \
	tif_dumpmode.o \
	tif_error.o \
	tif_extension.o \
	tif_fax3.o \
	tif_fax3sm.o \
	tif_flush.o \
	tif_getimage.o \
	tif_jbig.o \
	tif_jpeg.o \
	tif_jpeg_12.o \
	tif_luv.o \
	tif_lzma.o \
	tif_lzw.o \
	tif_next.o \
	tif_ojpeg.o \
	tif_open.o \
	tif_packbits.o \
	tif_pixarlog.o \
	tif_predict.o \
	tif_print.o \
	tif_read.o \
	tif_strip.o \
	tif_swab.o \
	tif_thunder.o \
	tif_tile.o \
	tif_unix.o	\
	tif_version.o \
	tif_warning.o \
	tif_webp.o \
	tif_write.o \
	tif_zip.o \
	tif_zstd.o \
	${NULL}
#	tif_win32.o \

LIBNAME =	libtiff.a

install: $(LIBNAME) installh

.c.o:
	$(COMPILEc) $< $(LOCALCFLAGS)

$(LIBNAME): $(OBJS)
	@rm -f $(LIBNAME)
	ar cr $(LIBNAME) `$(LORDER) $(OBJS) | $(TSORT)`;
	mv $(LIBNAME) $(LIBDIR)
	$(RANLIB) $(LIBDIR)/$(LIBNAME)
	@touch $(LIBNAME)

${OBJS}: tiffio.h tiff.h tiffiop.h
tif_compress.o: tif_compress.c Makefile
	${COMPILEc} ${LOCALCFLAGS} tif_compress.c
tif_fax3sm.o: tif_fax3sm.c g3states.h t4.h tif_fax3.h

g3states.h: mkg3states.c t4.h
	${LINKc} mkg3states ${LOCALCFLAGS} mkg3states.c
	./mkg3states > g3states.h


installh: ${INCS}
	-for i in ${INCS}; do \
	    h=`basename $$i`; \
	    cmp -s $$i ${INCLUDE_DIR}/$$h || \
	        install -c -m 444 $$i ${INCLUDE_DIR}/$$h; \
	done

clean:
	rm -f ${LIBNAME} ${OBJS} core a.out mkg3states g3states.h
clean-all:	clean_all
	rm -f g3states.h mkg3states

pristine:
	rm -f ${LIBNAME} ${OBJS} core a.out mkg3states g3states.h \
		$(LIBDIR)/$(LIBNAME)
 
man:

doc:

tags:	${SRCS}
	${CTAGS} ${SRCS}
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
