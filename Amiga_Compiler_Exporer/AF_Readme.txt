16.Sep.2021
-----------

Fuer den Compiler-Explorer von Bebbo https://franke.ms/cex

Die SAS/C-Optionen sind 

MATH=68881 CPU=68040 PARAMETERS=REGISTERS ANSI NOSTACKCHECK STRINGMERGE UNSIGNEDCHARS COMMENTNEST ERRORREXX NOMULTIPLEINCLUDES STRUCTUREEQUIVALENCE OPTIMIZERSCHEDULER NOVERSION UTILITYLIBRARY NOICONS OPTIMIZERTIME MULTIPLECHARACTERCONSTANTS DEFINE AMIGA IGNORE=147 PUBSCREEN=TURBOTEXT IGNORE=51 DEFINE=STATIC_FCN=static

Zusaetzlich ist im Amiga Smakefile noch GLOBALSYMBOLTABLE=WCSGST

Fuer den Bebbo gcc 6.5.0b (-Wall weggelassen, sonst STPRT-Warnungen)

-O2       -c -fmessage-length=0 -funsigned-char -m68020 -m68881 -noixemul  -fomit-frame-pointer -DSTATIC_FCN=static -fbaserel

Im Moment zeigt der beim SAS/C nur 133 Zeilen Assembler an. VIEL zu wenig. Der gcc zeigt 2802 Zeilen.
