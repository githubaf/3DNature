cd vlg
smake COPTIONS="MATH=68881"
copy libvgl.a /
cd /
setenv BUILDID=AF_BuildID
copy env:BUILDID envarc:   ; envarc:BUILDID is needed during smake. What is its purpose?

; fix WCSObjs.lnk
; this text file ends with many 0x00-Bytes which confise slink
; remove them (I used vim)

smake

or for compilation with optimizer:
smake optimize

emufan wrote: SelectBuild.rexx, it deals with the BuildID/env thing.


Issues:
DEM.c checks for extension "elev" or "relel", my Files are uppercase ??? uppercase. Change code to accept case independent

3.Mar.2021
----------
* fixed WCSObjs.lnk (removed superflous zeros at the end)
* replaced the fractal files in Tools directory with that from WCS 2.04 disks as the files from Github lead to wired looking pictures,
  the newer fractal files lead to proper pictures now

4.Mar2021
---------
Gross/Kleinschreibung bei einem Include korrigiert.
Assigns fuer Amiga SAS/C 

assign include: Work:MUI/Developer/C/Include/ add
assign lib: sc:LIB/ add
assign lib: work: add

vgl had several missing prototypes -> added stdlib.h to Amiga-ifdef-section in vgl/vgl.h

18.May.2021
-----------
Schrittweise die Includes aufraeumen. Durch die GST Geschichte vom SAS/C war das wohl bisher relativ egal. Wie compilieren erst mal nur WCS.h...
~/opt/m68k-amigaos_14May21/bin/m68k-amigaos-gcc -noixemul -O0 -g3 -Wall -c WCS.h -DMAIN -DEXT= -Wno-pointer-sign -Wno-missing-braces

01.06.2021
----------
MUI-Include-File geaendert. Jetzt kann AGUI.c compiliert werden.

cd Debug
PATH=$PATH:~/opt/m68k-amigaos_27Apr21/bin make all

compiliert schon eine ganze Menge.

4.6.21
------
Komische Zeichen in GlobeMapSupport.c zu "Alpha" und "Beta" geaendert.
HyperKhorner4M-1.asm voruebergehend vom Compilieren ausgeschlossen, da andere ASM-Syntax?
MapSfc.c excluded. Array dort ist nirgend definiert?
MapWorld.c excluded for now
OldMapTopo.c excluded for now

6.6.21
------
Anzeige der Funktionen, die dem Linker noch fehlen:
cd Debug
PATH=$PATH:~/opt/m68k-amigaos_27Apr21/bin make all 2>&1 | grep -v "/m68k-amigaos/bin/ld" | awk '/undefined reference to/{print $5}' | sort --unique

8.6.2021
--------
SAS/C specific functions in sasc_functions.c including test. Test should pass on Amiga(!) and linux command line.

sc sasc_functions.c LINK IGNORE=51 DEFINE=TESTING_SASC_FUNCTIONS
gcc sasc_functions.c -DTESTING_SASC_FUNCTIONS -lm -Wall -pedantic && ./a.out

14.Juni 2021
------------
WCS kann jetzt komplett mit Bebbos's gcc gebaut werden. Weiterhin compilierbar mit SAS/C

15.6.2021
---------
Warnungen gezielt bearbeiten. Zuerst printf-Format
PATH=$PATH:~/opt/m68k-amigaos_27Apr21/bin make all # clean all 2>&1 | grep "warning: format '%d' expects argument of type 'int', but"

- Alle %d-Warnungen ausser denen, die ein double-Argument haben. Erst mit SAS/C ausprobieren, ob wir das %d zu %f aendern oder den Parameter
  zu int casten muessen.

--> double DoubleVariable=3.14159;
--> printf("%d\n",DoubleVariable);

zeigt bei gcc und auch bei SAS/C Unfug an. Hier haben wir also richtige schwere(?) Fehler.
8x in LineSupport.c und 1x in TLSupportGUI.c

fixed TLSupportGUI.c
Timelines Editor Water Elev initial value wurde auch im Original-Executable falsch angezeigt. Fehler ist nur beim 1. Mal sichtbar, anderer Tab und wieder zurueck, dann verschwindet der Fehler.

Original Fehler reproduzieren (mit aktuellem Stand korrigiert)
-------------------------------------------------------------

(Project-> Open->CanyonSunset.proj
SunsetAnim.par laden
Parameter nicht fuer neues Format speichern.

SunsetAnim.par laden
Parameter nicht fuer neues Format speichern.

SunsetAnim.par laden
Parameter nicht fuer neues Format speichern.

SunsetAnim.par laden
Parameter nicht fuer neues Format speichern.

SunsetAnim.par laden
Parameter nicht fuer neues Format speichern.
4. Icon klicken, Ecosystem Editor auswaehlen
3x Make Key (z.B. 0,1,2)
Timelines -> Oben steht "Water" und rechts daneben eine falsche Zahl. Tab Elev ist aktiv. Klickt man andere Tabs und dann wieder zurueck auf Elev, ist der Wert berichtigt.

16.6.2021
---------
Noch einen kleinen Fehler behoben:
ScreenMode im Info-Fenster war immer falsch, wenn im Screenmode-Selector NICHT auf Save sondern auf Use gedrueckt wurde.
Behoben, der Mode wird jetzt aus dem Screen ausgelesen. (Agui.c Zeile 3060)

