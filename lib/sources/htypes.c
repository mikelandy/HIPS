/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * htypes.c - standard definitions for HIPS pixel formats
 *
 * Michael Landy - 1/3/91
 */

#include <hipl_format.h>

struct h_types h_typenames[] = {

{PFBYTE,"unsigned bytes"},
{PFSHORT,"short integers"},
{PFINT,"integers"},
{PFFLOAT,"floats"},
{PFCOMPLEX,"complex"},
{PFASCII,"ascii"},
{PFDOUBLE,"double"},
{PFDBLCOM,"double complex"},
{PFQUAD,"quadtree encoding (Mimaging)"},
{PFQUAD1,"quadtree encoding"},
{PFHIST,"histogram"},
{PFSPAN,"spanning tree format"},
{PLOT3D,"plot-3d format"},
{PFMSBF,"packed, most-significant-bit first"},
{PFLSBF,"packed, least-significant-bit first"},
{PFSBYTE,"signed bytes"},
{PFUSHORT,"unsigned shorts"},
{PFUINT,"unsigned integers"},
{PFRGB,"RGBRGBRGB bytes"},
{PFRGBZ,"RGB0RGB0RGB0 bytes"},
{PFZRGB,"0RGB0RGB0RGB bytes"},
{PFMIXED,"mixed raster formats"},
{PFBGR,"BGRBGRBGR bytes"},
{PFBGRZ,"BGR0BGR0BGR0 bytes"},
{PFZBGR,"0BGR0BGR0BGR bytes"},
{PFINTPYR,"integer pyramid"},
{PFFLOATPYR,"float pyramid"},
{PFPOLYLINE,"2D points"},
{PFCOLVEC,"Set of RGB triplets defining colours"},
{PFUKOOA,"Data in standard UKOOA format"},
{PFTRAINING,"Set of colour vector training examples"},
{PFTOSPACE,"TOspace world model data structure"},
{PFSTEREO,"Stereo sequence"},
{PFRGPLINE,"2D points with regions"},
{PFRGISPLINE,"2D points with regions and interfaces"},
{PFCHAIN,"Chain code encoding (Mimaging)"},
{PFLUT,"LUT format (Mimaging)"},
{PFAHC,"adaptive hierarchical encoding"},
{PFOCT,"oct-tree encoding"},
{PFBT,"binary tree encoding"},
{PFAHC3,"3-d adaptive hierarchical encoding"},
{PFBQ,"binquad encoding"},
{PFRLE,"grey-scale run-length encoding"},
{PFRLED,"run-length encoding"},
{PFRLEB,"run-length encoding, line begins black"},
{PFRLEW,"run-length encoding, line begins white"},
{PFPOLAR,"rho-theta format (Mimaging)"},
{PFVFFT3D,"float complex 3D virtual-very fast FT"},
{PFVFFT2D,"float complex 2D virtual-very fast FT"},
{PFDVFFT3D,"double complex 3D VFFT"},
{PFDVFFT2D,"double complex 2D VFFT"},
{PFVVFFT3D,"float 3D VFFT in separated planes"},
{PFDVVFFT3D,"double 3D VVFFT in separated planes"},
{-1,"unknown pixel format"}};

struct h_convs h_ctors[] = {

{CPLX_MAG,"complex (magnitude)"},
{CPLX_PHASE,"complex (phase)"},
{CPLX_REAL,"complex (real part only)"},
{CPLX_IMAG,"complex (imaginary part only)"},
{-1,"unknown format conversion???"}};

struct h_convs h_dctors[] = {

{CPLX_MAG,"double complex (magnitude)"},
{CPLX_PHASE,"double complex (phase)"},
{CPLX_REAL,"double complex (real part only)"},
{CPLX_IMAG,"double complex (imaginary part only)"},
{-1,"unknown format conversion???"}};

struct h_convs h_rtocs[] = {

{CPLX_RVI0,"complex (set real part only)"},
{CPLX_R0IV,"complex (set imaginary part only)"},
{CPLX_RVIV,"complex (set real and imaginary parts)"},
{-1,"unknown format conversion???"}};

struct h_convs h_rtodcs[] = {

{CPLX_RVI0,"double complex (set real part only)"},
{CPLX_R0IV,"double complex (set imaginary part only)"},
{CPLX_RVIV,"double complex (set real and imaginary parts)"},
{-1,"unknown format conversion???"}};
