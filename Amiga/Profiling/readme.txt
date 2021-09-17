17.Sep.21
---------

WCS mit -pg und -g bauen. Beide auch beim Linken angeben! WCS wird dann 14Meg statt 1,2Meg groß. Ohne -g geht line-by-line Profiling nicht.

WCS hat auf dem Amiga 9 Stunden 32 Minuten gedauert. Canyon-Sunset, 1/4 Size, Pal Hires, C=A4000T 68040/25

time ~/opt/m68k-amigaos_30Aug21/bin/m68k-amigaos-gprof -l WCS.unstripped  >profiling_line_by_line.txt

braucht auf dem Laptop 57 Minuten!

rm -rf profiling.pdf && ~/opt/m68k-amigaos_30Aug21/bin/m68k-amigaos-gprof  WCS.unstripped | gprof2dot -n0 -e0 | dot -Tpdf -o profiling.pdf &
