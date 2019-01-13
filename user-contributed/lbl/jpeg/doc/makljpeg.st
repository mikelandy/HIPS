; Project file for Independent JPEG Group's software
;
; This project file is for Atari ST/STE/TT systems using Pure C or Turbo C.
; Thanks to Frank Moehle (Frank.Moehle@arbi.informatik.uni-oldenburg.de)
; and to Dr. B. Setzepfandt (bernd@gina.uni-muenster.de).
;
; To use this file, rename it to LIBJPEG.PRJ.
; Read installation instructions before trying to make the program!
;
;
;      * * * Output file * * *
libjpeg.lib
;
; * * * COMPILER OPTIONS * * *  
.C[-P]        ; absolute calls
.C[-M]        ; and no string merging, folks
.C[-w-cln]    ; no "constant is long" warnings
.C[-w-par]    ; no "parameter xxxx unused"
.C[-w-rch]    ; no "unreachable code"
.C[-wsig]     ; warn if significant digits may be lost
.L[-J]        ; link new Obj-format (so we get a library)
=
; * * * * List of modules * * * * 
