#!/bin/bash

# Es vergleicht die beiden Dateien mit `cmp` und findet die erste unterschiedliche Zeile.
# Es berechnet die Zeilennummer, ab der die Ausgabe beginnen soll (100 Zeilen vor dem Unterschied).
# Es schneidet die Dateien und speichert die relevanten Zeilen in neuen Dateien.
# Schliesslich wird `meld` aufgerufen, um die neuen Dateien zu vergleichen.


#set -x

# Ueberpruefen, ob zwei Argumente uebergeben wurden
if [ "$#" -ne 2 ]; then
echo "Usage: $0 <datei1> <datei2>"
exit 1
fi

file1="$1"
file2="$2"

# Finde die erste unterschiedliche Zeile
diff_line=$(cmp "$file1" "$file2" | head -n 1 | awk '{print $7}')
echo ALEXANDER $diff_line
# Wenn keine Unterschiede gefunden wurden
if [ -z "$diff_line" ]; then
echo "Die Dateien sind identisch."
exit 0
fi

# Berechne die Zeilennummer fuer den Schnitt
line_number=$(($diff_line - 100))

# Erstelle die neuen Dateien
start_line=$((line_number < 1 ? 1 : line_number))
end_line=$(($diff_line + 2000000))

# Schneide die Dateien und speichere sie in neuen Dateien
sed -n "${start_line},${end_line}p" "$file1" > "neue_datei1.txt"
sed -n "${start_line},${end_line}p" "$file2" > "neue_datei2.txt"

# Diff mit Meld anzeigen
meld "neue_datei1.txt" "neue_datei2.txt"
