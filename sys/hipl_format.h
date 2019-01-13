/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * hipl_format.h - standard definitions for HIPS images
 *
 * Michael Landy - 12/28/90
 * modified added stdlib, string for malloc and strlen
 */

#ifndef HIPS2_HF
#define HIPS2_HF
#ifndef FILE
#include <stdio.h>
#endif
#include <hips_basic.h>
#include <hips_header.h>
#include <hips_error.h>
#include <hips_parser.h>

/* include local additions */
#ifdef GRLE
#include <hips_local.h>
#endif
#ifdef	STREAM_IMAGE_LIB
#include <stream.h>
#endif
#endif

/* STD Library Additions */
#include <stdlib.h>
#include <string.h>
