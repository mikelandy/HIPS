/* rle_config.h
 * 
 * Automatically generated by make-config-h script.
 * DO NOT EDIT THIS FILE.
 * Edit include/makefile.src and the configuration file instead.
 */
/*	modified in April, 1992
#define TAAC1 TAAC1
#define X11 X11
#define GIF GIF
*/
#define GRAYFILES GRAYFILES
#define MACPAINT MACPAINT
#define POSTSCRIPT POSTSCRIPT
#define SUNRASTER SUNRASTER
#define TARGA TARGA
#define SYS_V_SETPGRP SYS_V_SETPGRP
#define USE_L_FLAG USE_L_FLAG
#define USE_RANDOM USE_RANDOM
#if !defined MIPS & !defined	SGI
#define USE_STDLIB_H USE_STDLIB_H
#endif
#if	defined INCLUDES_ARE_ANSI && !defined	BSD4
#define	USE_STRING_H
#endif	/* modified in March, 1994	*/
#define XLIBINT_H XLIBINT_H
/***************** From rle_config.tlr *****************/

/* CONST_DECL must be defined as 'const' or nothing. */
#ifdef CONST_DECL
#undef CONST_DECL
#define CONST_DECL const

#else
#define CONST_DECL

#endif

/* A define for getx11. */
#ifndef XLIBINT_H
#define XLIBINT_H_NOT_AVAILABLE
#endif
