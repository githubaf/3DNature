# AF, 12.Feb.25
# usage:
#   gnuplot coverage.gnuplot
#   Format der History-Datei
#   2025-02-12_17:09:15 21%
#-----------------------------


set xdata time
set timefmt "%Y-%m-%d %H:%M:%S"
set format x "%Y-%m-%d\n%H:%M:%S"
set xlabel "Date"
set ylabel "Total Coverage (%)"
set title "Coverage History"
set grid
#plot "coverage_history.csv" using 1:2 with linespoints title "Total Coverage"
plot "coverage_history.txt" using 1:3 with linespoints title "Total Coverage"

# pause ist wichtig. Sonst geht das Diagramm sofort wieder zu.
# Wenn man das Script mit -p startet, bleibt das Fenster auf, aber das Script ist
# zu Ende und damit ist die mouseformat.Einstellung wieder weg. 
# (Koordinatenanzeige Zeit und Wert unten links)

#pause -1  "<Enter> zum beenden\n"  -1 == Warte auf Enter
pause 10



