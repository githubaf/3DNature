#!/bin/bash
# Erzeugt Aminet lha-Archiv
# Alexander Fritsch, 29.11.2022

ARCHIVE=wcs.lha;

function red_msg() {
    echo -e "\\033[31;1m${@}\033[0m"
}

function green_msg() {
    echo -e "\\033[32;1m${@}\033[0m"
}

rm -rf temp temp_aminet_upload;
mkdir temp temp_aminet_upload;
for CPU in 68020 68020-60 68040 68060 i386-aros x86_64-aros; do 
   cp -f $CPU/WCS_$CPU $CPU/WCS_$CPU.info temp/; 
done;
cp wcs.readme temp/

# Jetzt liegen alle Dateien in ./temp

# Testen, dass nicht "dirty" im Hash steht.
for FILE in $(find temp -type f -name "WCS_680*"); do
   m68k-amigaos-strings $FILE | grep -i "dirty"
   if [ $? -eq 0 ]; then 
      red_msg "git dirty found in $FILE"; false; 
   fi
done

#Testen, dass der Short: Text nicht länger als 40 Zeichen ist.
LANG=de_DE.ISO-8859-1 awk 'BEGIN{ret=0} /^Short:/{gsub(/Short:[[:space:]]*/,""); if(length >40){printf("Short-Text \"%s\" longer than 40 characters!!!",$0); ret=1;}} END{exit (ret);}' wcs.readme \
&& green_msg "\nwcs.readme Short: text length OK\n" || red_msg "\nERROR: Short: text to long\n"

#Testen, dass readme-Zeilenlaenge nicht ueberschritten wird
LANG=de_DE.ISO-8859-1 awk 'BEGIN{ret=0} END{exit ret} {if(length > 78){ret=1; printf("Line %d: %s  --> (%u Zeichen)\n",FNR,$0,length)}}' wcs.readme \
&& green_msg "\nwcs.readme line length OK\n" || red_msg "\nERROR: Lines too long\n"

#Test auf Zeilenende-Zeichen:
cat -v wcs.readme | grep "\^M"; if [ $? -eq 0 ]; then red_msg "wcs.readme 0a found"; false; else green_msg "wcs.readme line endings OK"; true; fi


#jetzt LHApacken

cd temp;
   jlha a ../temp_aminet_upload/$ARCHIVE *;
cd ..;

cp CanyonSet000.jpg wcs.readme temp_aminet_upload

echo
echo "Naechste Schritte:"
echo "------------------"
echo "* Hochladen ins Aminet"
echo "* Sichern des Uploads in wcs_aminet_upoloads/Emerald-xxx"
echo "* Taggen der hochgeladenen Version"
echo

