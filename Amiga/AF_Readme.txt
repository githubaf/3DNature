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

17.6.2021
---------
Jede Menge Wpointer-sign Warnungen:
PATH=$PATH:~/opt/m68k-amigaos_27Apr21/bin make clean all 2>&1 | grep "Wpointer-sign" | wc -l
4506

19.6.2021
---------
Die original Version von WCS2.04 ist viel schneller als meine gcc-Version, trotz -m68020 -m68881 -flto
Canyon Synset mit Hires Interlace, standart-Einstellungen (aber 2 Segmenten) braucht 5 Stunden 11 Minuten im Original und 8 Stunden 12 Minuten mit der GCC-Version auf dem A4000T (68040/25)
Test mit gcc -m68020 -flto -O2, aber ohne -m68881 dauert 11H 26Minuten
Test mit smake_optimize 5h 3 Minuten
Test mit smake 5h 30 Minuten

20.6.2021
Speicherschmierer!!! EM_Win->ValTxt[5] hat nur Eintraege 0...4, wird in der Schleife aber bis index 5 befuellt!
../EdMoGUI.c:274:27: warning: iteration 5 invokes undefined behavior [-Waggressive-loop-optimizations]
    DoMethod(EM_Win->ValTxt[i], MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
                           ^
../EdMoGUI.c:272:3: note: within this loop
   for (i=0; i<6; i++)

Juli 2021
---------
cppcheck *.c --force | grep "[0-9]+:[0-9]:" 
hat vor allem fehlende printf()-Parameter gefunden, resourceleaks (fehlendes fclose() im Fehlerfall) und einige Bufferoverlows!

- Einzelne Defines koennen mit -U wegdefiniert werden, die werdenn nicht untersucht)
cppcheck EdPar.c --force -U C_PLUS_PLUS | grep "[0-9]+:[0-9]:" 

8.Juli 2021
-----------
Smakefile/SMakeIt geaendert. c++ Comment detected warning (51) ist jetzt unterdrueckt.

cppcheck sollte alle Includes kennen:
~/opt/m68k-amigaos_15Jun21/bin/m68k-amigaos-gcc -E -Wp,-v -

Und wir sollten alle Compiler-Definitionen ubergeben.
~/opt/m68k-amigaos_27Apr21/bin/m68k-amigaos-gcc -dM -E - < /dev/null

12.July2021
-----------
cppcheck mit exclude unbenutzter Files/Directories. Start mit "." und dann -i

time cppcheck . -v --enable=all -I/home/developer/opt/m68k-amigaos_15Jun21/include -I/home/developer/opt/m68k-amigaos_15Jun21/lib/gcc/m68k-amigaos/6.5.0b/include -I/home/developer/opt/m68k-amigaos_15Jun21/m68k-amigaos/ndk-include -I/home/developer/opt/m68k-amigaos_15Jun21/m68k-amigaos/sys-include -I/home/developer/opt/m68k-amigaos_15Jun21/m68k-amigaos/include $(cat aaa)  -i disttools -i hyper -i info -i markov -i nngridr -i af.c -i af_test.c -i chartest.c -i GUI.c -i MapSfc.c -i MapWorld.c -i MarkovTable.c -i muitest.c -i nngridrextra.c -i OldMapTopo.c -i USDEMExtract.c 2>&1 | egrep "[0-9]+:[0-9]+:" | grep "error:" | wc -l

error     9
warning  72
style   729
note    274

15.July.2021
------------
304 Bytes can be saved by commentig 2 unused functions. Found with my find_unused_funcs.sh script
vgl_cmyk_to_rgb()
vgl_cmy_to_rgb()

another 40 Bytes by declaring setfaceone() static.

16.7.2021
---------
Current Size:
      -O2                          : 1492520 Bytes
-flto -O2                          : 1492392 Bytes   -->    128 Bytes saved due to flto. That is not much!
-flto -O2 -fommit-frame-pointer    : 1478696 Bytes   --> 13.696 Bytes saved die to fommit-frame-pointer

static setfaceone(), setfacetwo()  : 1478632 Bytes   -->     64 Bytes saved. -flto seems not to have optimized that...

   text	   data	    bss	    dec	    hex	filename
1240672	  93420	 140884	1474976	 1681a0	WCS

i18.July 2021
-------------
21 unused functions ifdefed
                                 :  1469528 Bytes   -->    9.104 Bytes saved

20.July 2021
------------
cd no_flto/                  # BuildCOnfiguration ohne -flto
../find_unused_funcs_nm.sh   # findet unused/potential static Funktionen


nach no_flto gehen. (nm geht nicht mit lto-Objekten)
neues Script findet nicht extern referenzierteObjekte, also unused oder Static-Kandidaten
../find_unused_funcs_nm.sh

Aktuell 200 solche Funktionen (schon sehr viele behoben)
Aktuelle Groesse von WCS (gcc Relase)      1466164 
                          scs/c optimize    984292   <-- warum soviel kleiner als die gcc Version???

Die sas/c Version ist *viel* schneller als die GCC-Version. Warum???

21.07.2021
----------
SAS/C Map-File erzeugen: Beim Linken
MAP wcs.map h,f

Der komplette slink Aufruf ist damit

slink LIB:c.o WCS.o WITH WCSObjs.lnk lib:utillib.with LIB LIB:scm881.lib libvgl.a LIB :sc.lib LIB:amiga.lib MAP wcs.map h,f TO WCS

Der SAS/C benutzt eine Datei SCOPTIONS mit folgendem Inhalt:
MATH=68881
CPU=68040
PARAMETERS=REGISTERS
ANSI
NOSTACKCHECK
STRINGMERGE
UNSIGNEDCHARS
COMMENTNEST
ERRORREXX
NOMULTIPLEINCLUDES
STRUCTUREEQUIVALENCE
OPTIMIZERSCHEDULER
NOVERSION
UTILITYLIBRARY
NOICONS
OPTIMIZERTIME
MULTIPLECHARACTERCONSTANTS
DEFINE AMIGA
IGNORE=147
GLOBALSYMBOLTABLE=WCSGST
PUBSCREEN=TURBOTEXT

gcc profiler:
CFLAGS="-pg" LIBS1="-lgcov" 
-fomit-frame-pointer muss entfernt werden.

Auf WinUAE oder Amiga laufen lassen. Dabei SnoopDos milaufen lasse.
Das erzeugte Programm moechte nach seinem Lauf (auf dem Amiga) auf die gcda-Files zugreifen, z.B. auf
/home/developer/Desktop/SelcoGit/3DNature/Amiga/Profiling/WCS.gcda

Deshalb muessen wir beim Compiler noch
-fprofile-generate=VBox:Desktop/SelcoGit/3DNature/Amiga/Profiling
angeben. Dann klappt das.

/home/developer/opt/m68k-amigaos_15Jun21/bin/m68k-amigaos-gprof  WCS # (ungestipptes Executable)

der profiler-output zeigt ganz oben, welche Funktion wieviel beigtragen hat.

Oder grafisch:

/home/developer/opt/m68k-amigaos_15Jun21/bin/m68k-amigaos-gprof  WCS | gprof2dot -n0 -e0 | dot -Tsvg -o output.svg && mirage output.svg


--> Der Profiler-Lauf war sehr schnell!? Sonst ist mein gcc-Executable ja ziemlich langsam im Vergleich zur SAS/C-Version.
Compiler-Optionen ueberpruefen. Ist LTO vielleicht doch schaedlich? --> Wohl nicht

--> Es liegt am -m68881 beim Compiler und Linker. Damit ist es unter WinUAE drastisch schneller, sogaer etwas schneller als die SAS/C-Version.
--> Auf dem Amiga muss das noch ausprobiert werden.

Der Profiler zeigt, dass meine swmem()-Funktion  extrem oft aufgerufen wird und 8% der Zeit braucht. Kann man die inline machen?
--> erst mal libnix mit -pg bauen. Dann ist der Profiler hoffentlich aussagekr�ftiger.

22.July2021
-----------
Man kann in Eclipse die Compiler-Flag pro File setzen! Einfach rechte Maustaste Preferences. Damit koennte ich alle GUI-Files mit -Os compilieren und den Rest mit -O2 oder -O3.


23.July2021
-----------
mor functions static. 150 left.

26.July 2021
------------
Alle moegliche  Funktion static oder ganz wegdefiniert.

Release:
1427452  STATIC_FUNCTIONS=static -flto  -fomit-frame-pointer    -> nochmal 3.488 Bytes haben die ganzen static functions gespart 
1430940  STATIC_FUNCTIONS=       -flto  -fomit-frame-pointer

Alle Files mit GUI im Namen mit -Os compiliert, den Rest mit -O2
Release:
1151504 STATIC_FUNCTIONS=static -flto  -fomit-frame-pointer


28.Juli 2021
------------
pre-commit Script erzeugt. Muss manuell nach .git/hooks kopiert werden. Verhindert das Einchecken von UTF-8 Dateien.

29.Juli 2021
------------
Eineige Files wraen UTF-8 kodiert. Das fuehrt zu seltsamer Darstellung auf dem Amiga. mit iconv nach ISO 8859-1 konvertiert.


2.August 2021
-------------

#!/bin/bash
# rejects commits of UTF-8 text files. 
# copy this file to .git/hooks/
# AF, 29.Juli 2021

FILE_LIST="$(git diff --cached --name-only)"

for FILE in $FILE_LIST; do
if [ $(file "$FILE" | grep -c "UTF-8 Unicode text") -ne 0 ]; then
echo "Local pre-commit hook"
echo "Error: File $FILE is UTF-8 encoded. Change that to ISO 8859-1 for Amiga and try again!"
echo "Commit refused."
exit 1
fi
done

--------------------------------

# copy this file to hooks/
# AF, 29.Juli 2021

EXIT_CODE=0

# check each branch being pushed
while read old_sha new_sha refname
do
if [ $old_sha == "0000000000000000000000000000000000000000" ]; then
old_sha=$(git hash-object -t tree /dev/null) # erstes Mal? Dann Hash vom "Empty Tree" nehmen
fi

FILE_LIST=$(git diff --name-only $old_sha $new_sha)
for FILE in $FILE_LIST; do
if [ $(git show "$new_sha":$FILE | file - | grep -c "UTF-8 Unicode text") -ne 0 ]; then
echo "Error: $refname: $FILE is UTF-8 encoded. Change that to ISO 8859-1 for Amiga and try again!"
EXIT_CODE=1
fi
done

done

exit "$EXIT_CODE"

3.August 2021
-------------
Bebbo hat an seinem gcc fuer mich gearbeitet. Jetzt kann man libnix mit -pg uerbsetzen und sollte dann ein komplettes Profiling machen koennen.
PATH=$PATH:~/opt/m68k-amigaos_26Jul21/bin/ make clean all 2>&1

PATH=$PATH:~/opt/m68k-amigaos_26Jul21/bin/ m68k-amigaos-gprof  WCS # | gprof2dot -n0 -e0 | dot -Tsvg -o output.svg && mirage output.svg

     # map-File fuer file ordering erzeugen
     PATH=$PATH:~/opt/m68k-amigaos_26Jul21/bin/  m68k-amigaos-nm --extern-only --defined-only -v --print-file-name WCS.unstripped >map_file

    # jetzt Objekt File Reihenfolge vorschlagen lassen -> geht nicht! gprof behauptet, da sind keine Symbole !?
    PATH=$PATH:~/opt/m68k-amigaos_26Jul21/bin/ m68k-amigaos-gprof WCS.unstripped -a --file-ordering map_file 

Der Profiler-Aufruf zeigt swmem() als einen Zeitfresser an! 8% nur fuer diese Funktion. Also jetzt Baum nur mit swmem() als Ziel anzeigen:
PATH=$PATH:~/opt/m68k-amigaos_26Jul21/bin/ m68k-amigaos-gprof  WCS.unstripped | gprof2dot -n0 -e0 --leaf=swmem | dot -Tsvg -o output.svg && mirage output.svg

--> OK. Aufruf von 
FractPoint_Sort() macht 5,8% aus, hier k�nnte man optimieren?

Diesen Profiler-Lauf mal mit Amiga oder cycle-exact laufen lassen...

10.August 2021
--------------
WCS 2.04 auf dem C=A4000T (040/25) 2MBytes Chip, 16Meg Fast)
Canyon Sunset, Pal-Hires, Groe�e/4 05:08:28

Profiling-Lauf:
--> Der Speicher reicht nicht, um die Wolken zu berechnen!
--> Nicht mit Samba-Laufwerk arbeiten sondern assign VBOX: RAM: gemacht. Sonst ist das Schreiben der gcda-Files EXTREM langsam!
--> Vorher ein 
makedir ram:Desktop/SelcoGit/3DNature/Amiga/Profiling ALL
damit die Unterverzeichnisse da sind. Nach Programmende dann das Verzeichnis RAM:Desktop nach Linux: kopieren.
--> gmon.out nocht kopieren. (Liegt im Verzeichnis, in dem Das Programm gestartet wurde)

--gstabs beim Assembler mit angeben, damit Debuginformationen geschrieben werden.

16.8.2021
----------
Alter Lauf:
WCS 2.04 auf dem C=A4000T (040/25) 2MBytes Chip, 16Meg Fast)
Canyon Sunset, Pal-Hires, Groe�e/4 05:08:28

Neu mit gcc:
WCS_gcc_Release_Static 68020/68881 auf dem C=A4000T (040/25) 2MBytes Chip, 16Meg Fast)
Canyon Sunset, Pal-Hires, Groe�e/4 07:36:02    !!! Warum so langsam? Trotz static  

Statt -m68020 -m66881 jetzt nur -m68040
WCS_gcc_Release_Static_68040 auf dem C=A4000T (040/25) 2MBytes Chip, 16Meg Fast)
Canyon Sunset, Pal-Hires, Groe�e/4 10:29:07   !!!!!!!!!


WCS_gcc_Release_Static_68040_mhard-float auf dem C=A4000T (040/25) 2MBytes Chip, 16Meg Fast)
Canyon Sunset, Pal-Hires, Groe�e/4 7:35:17, also auch nicht viel besser als 68020/68881


Auf dem Amiga muss zum SAS/C compilieren noch http://aminet.net/dev/c/SDI_headers.lha installiert werden und http://aminet.net/dev/mui/mui38dev.lha

Compilieren auf dem C=A4000T mit Smbfs 2.2 ist extrem langsam, geht aber. -> Besser unter WinUAE machen.

WCS_smake_optimize auf dem C=A4000T (040/25) 2MBytes Chip, 16Meg Fast)
Canyon Sunset, Pal-Hires, Groe�e/4 4:53:40    --> bisher schnellste Variante

(15.Sep.21)
WCS_GCC_baserel_O2Os (1048264 Bytes) auf dem C=A4000T (040/25) 2MBytes Chip, 16Meg Fast)
GUI-Files mit -Os, die anderen mit -O2 
Canyon Sunset, Pal-Hires, Groe�e/4 7:24:29. In WinUAE braucht es 1:04 statt 1:30, auf dem Amiga aber keine wesentliche Verbesserung!


26.8.2021
---------
mit -mfast-math �bersetzt -> 7:31:54, also praktisch keine Verbesserung.


// https://stackoverflow.com/questions/24348227/how-to-disable-double-precision-math-while-compiling-with-gcc-or-and-iar
gcc has an optimization option -fsingle-precision-constant that treats ordinary floating point constants as single precision

-Wdouble-promotion does exactly what you want, see the doc, under Warning Options. The example in the doc is quite similar to yours by the way.

Here is basically your example:

float f(int int_var, float float_var_2) {
  return 3.0 * int_var / float_var_2;
}


Coverage:
gcovr --object-directory=. -r . --html --html-details -o coverage.html

-> Beim Compilieren UND linken -noixemul nicht vergessen. Stack erh�hen!


19.9.2021
---------
Jetzt mit -m68020 -m68881 -noixemul  -fomit-frame-pointer -DSTATIC_FCN=static -fbaserel -flto -D__inline="inline static" compiliert. (Direkte fpu-Instructions durch math-68881.h)
WCS_gcc_Rel_Stat_br68881 auf dem C=A4000T (040/25) 2MBytes Chip, 16Meg Fast)
Canyon Sunset, Pal-Hires, Groe�e/4  7:24:17  also trotz durchgehend fpu-Instructions nicht schneller!!!

- Mit -m68040 compiliert und gelinkt.( -m68040 -noixemul  -fomit-frame-pointer -DSTATIC_FCN=static -fbaserel -flto -D__inline="inline static")
WCS_gcc_Rel_Stat_br68881 auf dem C=A4000T (040/25) 2MBytes Chip, 16Meg Fast)
Canyon Sunset, Pal-Hires, Groe�e/4  4:49:14, schnellstes Ergebnis!

24.Sep.21
---------
Experimente mit -ffast-math. Zusammen mit -O2 und -m68040 oder -m68020 -m68881 werden dann FPU-Instruktionen in den Code eingebaut. Bebbo hat noch Fehler rausgebaut. Jetzt erzeugt WCS ein Bild, das ist aber um 90 Grad gedreht!? Ohne -ffast-math ist alles OK.

-> Das Problem liegt im MapTopo.c  -> wohl doch nicht!
-> Mit #pragma GCC optimize("no-fast-math") Teile der Datei ohne fast-math compiliert. Komisch, bringt nichts!

--> Es ist MapUtil.c (alles ohne -ffast-math gebaut, dann einzelne File mit -ffast-math compiliert bis der 90 Grad-Fehler auftrat.

--> Bebbo hat das behoben am 25.9.21 morgens.

Test, ob -ffast-math wirkt:
m68k-amigaos-objdump -D WCS | grep "fsin"
Er muss fsin finden. 
Ich muss die Unterschiede -m68020, -m68020 -m68881, -m68020 -m68881 -ffast-math mal untersuchen.
Was ist bei -m68040 anders als bei -m68020 -m68881?

26.9.2021
---------
-mregparm fuehrt immer noch zum Crash. Es liegt nicht an Bebbos -fbbb Optimierungen.

28.9.2021
---------
Nochmal Speed-Test. Roadshow und smbfs2.2 waren aktiv, aber Netzwerkkabel gezogen, um konstante Bedingungnen zu erzeugen.
Mit -m68040 compiliert und gelinkt.( -g -O2 -m68040 -noixemul  -fomit-frame-pointer -DSTATIC_FCN=static -fbaserel -flto -D__inline="inline static")
WCS_040_basrel_lto auf dem C=A4000T (040/25) 2MBytes Chip, 16Meg Fast)
Canyon Sunset, Pal-Hires, Groe�e/4  4:42:22, schnellstes Ergebnis!

29.9.2021
---------
WCS 2.04 auf frischem WinUAE A4000T (3.2) installiert. Dabei wird MUI version 10 installiert. Version 10 ist also genug und wir brauchen nicht Version 19 (MUI 3.8) fordern.


5.Oktober 2021
--------------
alt   5273 warnings --> CONST_STRPTR Casts fuer Aufrufe von User_Message()
jetzt 4657 warnings

6.Oktober 2021
--------------
Die Ausdruecke 'FORM' durch MakeID('F','O','R','M') ersetzt, um Warnungen zu beseitigen.

for FILE in $(find -type f -name "*.[c\|h]"); do sed  -i "s/'\([A-Z]\)\([A-Z]\)\([A-Z]\)\([A-Z]\)'/MakeID(\'\1\',\'\2\',\'\3\',\'\4\')/g" $FILE; done

4553 Warnings

7-Oktober 2021
--------------
In MUI.h muss (CONST_STRPTR) rein, um Warnungen zu beseitigen:
#echo \#define MUIC_Register \"Register.mui\" | sed -e "s/\(#define MUIC_[A-Z,a-z]* \)\(.\)/\1 (CONST_STRPTR)\2/g"
Also:
sed -i "s/\(#define MUIC_[A-Z,a-z]* \)\(.\)/\1 (CONST_STRPTR)\2/g" ~/amiga_gcc_link/../m68k-amigaos/include/libraries/mui.h

--> 2712 Warnings

Es gibt viele Log()-Aufrufe, die auch (CONST_STRPTR) brauchen:
#echo "Log(ERR_OPEN_FAIL, \"Wave File\");" | sed "s/\(Log(.*,\) *\(.*\"\)/\1 (CONST_STR_PTR)\2/g"

Also
for FILE in $(find -type f -name "*.[c\|h]"); do sed -i "s/\(Log(.*,\) *\(.*\"\)/\1 (CONST_STRPTR)\2/g" $FILE; done

--> 2602 Warnings

Weiter mit Warnungs-Casting
--> 2500 Warnings

und weiter
--> 1202 Warnings

- Mit -m68040 und -mregparm compiliert und gelinkt.( -m68040 -noixemul  -fomit-frame-pointer -DSTATIC_FCN=static -fbaserel -flto -mregparm -D__inline="inline static")
WCS_040_basr_mregparm_lto auf dem C=A4000T (040/25) 2MBytes Chip, 16Meg Fast)
Canyon Sunset, Pal-Hires, Groe�e/4  4:35:17, schnellstes Ergebnis!

8.Oktober 2021
--------------
-ffast-math funktioniert mit -m68040, wenn zusaetzlich -mregparm angegeben wird, sonst nicht!

Die Variante mit -fbaserel (-m68040 -ffast-math -mreparm -fbaserel -flto) ist 120k kleiner als die ohne -fbaserel.

Weiter (STRPTR) eingef�gt:
find . -name "*.[c\|h]" -exec  sed -i "s/\(nm_Label *= *\)\(".*"\)/\1 (STRPTR)\2/g" {} \;

886 Warnings

- Mit -m68040 und -mregparm ohne -fbaserel compiliert und gelinkt.( -m68040 -noixemul  -fomit-frame-pointer -DSTATIC_FCN=static -flto -mregparm -D__inline="inline static")
WCS_040_basr_mregparm_lto auf dem C=A4000T (040/25) 2MBytes Chip, 16Meg Fast)
Canyon Sunset, Pal-Hires, Groe�e/4  4:35:56, minimal langsamer, 10% groesser

11.Oktober 2021
---------------
Mehr casts und forward Declaration
580 Warnings
jetzt sind alle Wint-conversion Warnungen raus.
114 Warnings uebrig.

12.Oktober 2021
---------------
fixed fscanf in Support.c  Added lengt limit. removed errournious "&" in front of char arrays
--> 65 warnings left. Compileable with SAS/C  (smake optimize)

mixed more fscanf.
-->56 warnings left.

14.10.2021
----------
Erzeugen der Prototypen aus den c-Files
cproto *.c -E m68k-amigaos-cpp  -I. -DSTATIC_FCN= > aaa.txt


15.10.21
--------
Fehler tritt bei STATIC_FCN = static auf, und irgendwo in 
./LWSupport.c ./MakeFaces.c ./Map.c ./MapExtra.c ./MapGUI.c ./MapSupport.c ./MapTopoObject.c ./MapUtil.c ./MoreGUI.c ./MUIFloatInt.c

--> Das Problem ist in MapUtil.c

17.10.21
--------
und dort in der einfachen Funktion ZeroMatrix3x3()!
Wenn die static ist, funktioniert das Programm nicht. (Keine Landschaft). Wenn sie nicht static ist, klappt alles.

19.10.2021
----------
Bebbo hat das Problem im gcc behoben. Mit dem Compiler vom 18.10.2021 funktioniert WCS jetzt mit und ohne .mregparm, mit und ohne static functions und mit und ohne ffastmath.
-> Testcode wieder aus den WCS-Quellen rausgenommen. 

Ohne -lto habe ich noch etwa 100 [-Wmaybe-uninitialized] warnings. Mit -lto verschwinden die!?

Mit
m68k-amigaos-objdump -t WCS.unstripped  | awk '/^[0-9]+.**\.data/{print $6}' | sed 's/^_//g'
bekomme ich alle globalen Variablen. Der SED-Aufruf entfernt den Unterstrich am Anfang. Ich habe ohne -lto kompiliert.

So kann ich ich nach einer Variablen suchen und azeigenn in wievielen Dateien diese Variable benutzt wird.
for FILE in $(find .. -name "*.c"); do grep -nH "sinxrot" $FILE; done | awk -F: '{print $1}' | sort --unique

Also als Script:

cat ./find_static_candidate_variables.sh

#!/bin/bash
# 19.Oktober 2021, AF
# sucht Variablen, dies nue in einer Datei verwendet werden, also static gemacht werden koennen

for VAR in $(m68k-amigaos-objdump -t WCS.unstripped  | awk '/^[0-9]+.**\.data/{print $6}' | sed 's/^_//g'); do
        FILELISTE="$(for FILE in $(find .. -name "*.c"); do
        grep -nH $VAR $FILE | awk -F: '{print $1}';done | sort --unique)" # Alle Files, die die Variable enthalten

	if [ ! -z "$FILELISTE" ]   # wenn Liste nicht leer. (Leere Liste liefert auch Anz=1 ???)
	then
	        ANZ=$(echo "$FILELISTE" | wc -l)   # Anzahl Elemente in der FILELISTE
        	if [ "$ANZ" -eq 1 ]
	        then
        	        echo "$FILELISTE $VAR"
		fi
        fi
done


Das kam raus:
--------------
./find_static_candidate_variables.sh | sort
../ColorBlends.c colavg
../DataBase.c fieldname
../DataBase.c NoOfFields
../DataBase.c RecordLength
../EdPar.c CoShift                 // geht nicht, per Macro ueberall benutzt
../EdPar.c UndoKeyFrames
../Fractal.c polyct                // 1048236
../GlobeMap.c ACosTable
../GlobeMap.c ASinTable
../GlobeMap.c CosTable
../GlobeMap.c ILBMnum
../GlobeMap.c lastfacect
../GlobeMap.c RenderWind0_Sig
../GlobeMap.c SinTable
../GlobeMap.c statfile            // 1048204
../GlobeMap.c TrigTableEntries    // geht nicht ??? Aerger mit WCS.c ????
../GlobeMapSupport.c bluesky      ok bis hier, (Canyon Bild ok)
../GlobeMapSupport.c flblue
../GlobeMapSupport.c flgreen
../GlobeMapSupport.c flred
../GlobeMapSupport.c greensky
../GlobeMapSupport.c redsky       ok bis hier
../InteractiveDraw.c BinarySerialPlaceHolder  unused
../InteractiveDraw.c Itchy
../InteractiveDraw.c Pixie
../InteractiveView.c AltRenderListSize
../InteractiveView.c InterWind0_Sig
../InteractiveView.c WindowNumber   ok bis hier (Canyon Bild ok) 1048236 bigger
../LineSupport.c ptblue
../LineSupport.c ptgreen
../LineSupport.c ptred              <--- 1048412 bigger
../LWSupport.c LWNullObj            ok bis hier (Canyon Bild ok)
../MakeFaces.c elface               <--
../Map.c MapWind3_Sig
../Map.c MP_DigLatScale
../Map.c MP_DigLonScale
../Map.c MP_Nlat
../Map.c MP_ORx
../Map.c MP_ORy
../Map.c MP_Rotate
../Map.c MP_Wlon                    <-- ok bis hier (Canyon Bild ok) 1048412
../MapExtra.c frontpen
../MapExtra.c graphtype
../MapExtra.c ptstore
../MapGUI.c MapNewMenus             <-- ok bis hier (Canyon Bild ok) 1048412
../MapGUI.c MapWind0_Sig
../MapGUI.c PrimaryColors
../MapGUI.c PrintColors
../MapGUI.c UnderConstOK            <-- ok bis hier (Canyon Bild ok) 1048412
../MapSupport.c lat_y
../MapSupport.c lon_x
../MapTopoObject.c ptqq              
../MapWorld.c faceel                <--- ok bis hier (Canyon Bild ok) 1048412
../MoreGUI.c SaveAscii               
../MUIFloatInt.c DOSBase
../MUIFloatInt.c FloatIntClassPointer 
../MUIFloatInt.c FWT
../MUIFloatInt.c UtilityBase
../ParamsGUI.c IA_AnimStep
../RequesterGUI.c frbase
../RequesterGUI.c frfile
../RequesterGUI.c frparam
../RequesterGUI.c PocketWatch
../RequesterGUI.c ProjDate
../RequesterGUI.c PWDate
../RequesterGUI.c Today            <<-- ok bis hier (Canyon Bild ok) 1048408
../RexxSupport.c RexxSysBase
../RexxSupport.c SysBase
../Tree.c HorSunAngle
../Tree.c HorSunFact
../Tree.c VertSunFact
../vgl/clib.c _vgl_rand_last       <---
../WCS.c AslBase
../WCS.c GadToolsBase
../WCS.c GfxBase
../WCS.c MUIMasterBase
../WCS.c NewAltColors
../WCS.c PenSpec

-g -m68040 -noixemul  -fomit-frame-pointer -DSTATIC_FCN=static -ffast-math -mregparm -fbaserel -flto --> 1054764 Bytes   (groesser als ohne -flto!)
-g -m68040 -noixemul  -fomit-frame-pointer -DSTATIC_FCN=static -ffast-math -mregparm -fbaserel       --> 1048612 Bytes

Static-Test jetzt wegen schnelleren Linken ohne flto.

21.10.2021
jetzt noch einmal
./find_static_candidate_variables.sh | sort
../EdPar.c CoShift                 // not static. Is used via macro from everywhere, AF
../GlobeMap.c TrigTableEntries     // static AF geht nicht, Aerger mit WCS.c Assembler?????  Hat bebbo mit neuem gcc 25Sep21 behoben!
../MapGUI.c MapNewMenus            //ok, nachgeholt
../MUIFloatInt.c DOSBase
../MUIFloatInt.c UtilityBase
../RexxSupport.c RexxSysBase
../RexxSupport.c SysBase
../vgl/clib.c _vgl_rand_last
../WCS.c AslBase
../WCS.c GadToolsBase
../WCS.c GfxBase
../WCS.c MUIMasterBase
../WCS.c NewAltColors              // ok, nachgeholt
../WCS.c PenSpec                   // ok, nachgeholt

Jetzt immernoch 1048408 Bytes. Alles OK. (Canyon Bild ok)


27.10.2021
----------
swmem()-Statistik von CanyonSunset 752x480 (default) 1 Frame. Fast 38 Mio Aufrufe.

swmem(total)= 37822678
swmem(1)    =  7894144 (20.9%)
swmem(2)    =   962530 ( 2.5%)
swmem(4)    =        2 ( 0.0%)
swmem(8)    = 28966002 (76.6%)
swmem(other)=        0 ( 0.0%)

Etwa 190 Stellen, an den swmem aufegrufen wird. Scheinen ALLE eine Konstante f�r die Anzahl zu haben.
inline mit Compiletime-Macro machen!


4.Now.2021
----------
#define swmem(a,b,n) \
    (__builtin_constant_p(n) && n==1) ? swmem_1(a,b) :  \
        (__builtin_constant_p(n) && n==8) ? swmem_8(a,b) : swmem_other(a,b,n);

macht die compiletime-Entscheidung, welches swmem() eingebaut wird. per #define SWMEM_FAST_INLINE einschaltbar.

Compilierbar, aber WCS funktioniert damit nicht mehr richtig. Die neuen Funktionen koennen ohne inline genutzt werden mit -fno-inline.

Wo wird swmem() genutzt?
find . -name "*.c" -exec grep -nH "swmem" {} \; | awk -F: '{print $1}' | sort -u

Dann dateiweise mit -fno-inline compilieren, bis es wieder funktioniert.

5.11.2021
---------
Suche nach problematischem File:

../Cloud.c              -fno-inline
../DataBase.c           - 
../DataOps.c            -
../DEM.c                -
../EdDBaseGUI.c         -
../EdEcoGUI.c           -
../EditGui.c            -
../EdPar.c              -
../Fractal.c            -

../InteractiveView.c
../LineSupport.c
../Map.c
../MapExtra.c
../MapSfc.c
../MapSupport.c
../MapTopoObject.c
../MapUtil.c
../sasc_functions.c
../USDEMExtract.c

Funktioniert wieder. Problem also im ersten Block.

../Cloud.c              -  ohne -fno-inline
../DataBase.c           - 
../DataOps.c            - 
../DEM.c                - 
../EdDBaseGUI.c         - 

noch ok bis hier

../EdEcoGUI.c           - 
../EditGui.c            - 

noch ok bis hier. Vermutung: Fractal.c ist es? 

../EdPar.c              
../Fractal.c            - geht nicht mehr. Also Fehler hier! Wenn Fractal.c mit inline uebersetzt wird, geht es nicht mehr.


Fuer Compiler-Explorer: Preprocessed File erzeugen

m68k-amigaos-gcc -DFORCE_MUIMASTER_VMIN=10 -DAMIGA_GUI -DSTATIC_FCN=static -DSTATIC_VAR=static -I"/home/developer/Desktop/SelcoGit/3DNature/Amiga" -O2 -Wall -fmessage-length=0 -funsigned-char -MMD -MP "../Fractal.c" -g -m68040 -noixemul  -fomit-frame-pointer -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -fbaserel -Winline -DSWMEM_FAST_INLINE -mregparm -fno-inline -E >Fractal_prepro.c

cat Fractal_prepro.c | sed 's/^#/\/\/\#/g' > Fractal_prepro_2.c    # Kommentare mit Doppelkreuz will der CompilerExplorer nicht

-O2 -m68040 -fomit-frame-pointer -ffast-math -mregparm
-O2 -m68040 -fomit-frame-pointer -ffast-math -mregparm -fno-inline

--> Problem liegt in Fractal.c in der Funktion 
void FractPoint_Sort(double *Elev)

11.11.2021
----------
Bebbo commented on Github:
-----------------------------------------------
...
and you should convert this

extern double polyq[10][3];
extern double polyslope[10][3];
extern double polyrelel[10][3];
extern double polydiplat[10][3];
extern double polydiplon[10][3];
extern double polylat[10][3];
extern double polylon[10][3];
extern double polyx[10][3];
extern double polyy[10][3];
extern double polyel[10][3];
extern double polycld[10][3];

into one reference to a struct.

extern struct p [10][3] _p;

but that's work^^

-----------------------------------------------

The problem is an optimization which is ok, since the data isn't marked as volatile. But the swapping modifies the data...
So the later compares are using invalid data which yields an invalid result.

so either make polyy volatile to catch all modifications
or use code like

 double p0 = polyy[b][0];
 double p1 = polyy[b][1];
 double p2 = polyy[b][2];

and swap the p0/p1 accordingly, to ensure subsequent comparisons are using the correct values.
And even better: use code like: (check for correctness please)

  if (p0 > p1) {
	  if (p0 < p2)
		  SWAP(0,1);
	  else if (p1 >= p2)
		  SWAP(0,2);
	  else 
		  ROT(2,1,0); // better than many SWAPs
  } else if (p1 > p2) {
     if (p0 < p2)
	  SWAP(1,2);
    else
        ROT(2,0,1);
 }

which avoids double/triple swaps.
-----------------------------------------------
For the moment I made polyy volatile and that indeed fixes the problem. I will try Bebbos 
optimization suggestions later.

Toolchain-Versions ins Executable: Aehnlich wie in espeak

Makefile:
TOOLCHAIN_VER=$(shell cat $(COMPILER_DIR)/toolchain_commit.txt)
--> Jetzt erzeugt mein Install-Script ein Script m68k-amigaos-toolchain_hashes.sh, das man aufrufen kann
TOOLCHAIN_VER=$(68k-amigaos-toolchain_hashes.sh)
--> TOOLCHAIN_VER=\""$(shell m68k-amigaos-toolchain_hashes.sh)"\" bei Properties->C/C++ Build -> Settings -> Cross GCC Compiler -> Preprozessor

in Version.c:
char toolchain_ver[] = "\0$TLCN: " TOOLCHAIN_VER;   /* This macro contains git hashes for Bebbos's gcc*/

Man koennte das ganze noch mit rot47 im Makefile verstecken
| tr '!-~' 'P-~!-O'

m68k-amigaos-strings WCS | grep \$TLCN


Das mit rot47 ist gar nicht so einfach:
TOOLCHAIN_VER=\"'$(shell m68k-amigaos-toolchain_hashes.sh | tr '!-~' 'P-~!-O' | sed 's/\\/\\\\/g' )'\"
-> Das define TOOLCHAIN_VER wird ja zu einem C-String. Dabei wird z.B. \8 zu einem nicht einer binaeren 8 (statt einem \ und einer 8). Beim rot47 enstehen aber "\".
   Die muessen dan mit sed noch in "\\" umgewandelt werden, damit es im C-String dann wirklich ein "\" ist.

Dekodieren mit
m68k-amigaos-strings WCS | grep \$TLCN | tr '!-~' 'P-~!-O' | sed 's/[[:space:]]/\n/g'
Der sed-Aufruf macht das ganze noch schoen zeilenweiseonst steht alles hintereinader.


13.11.2021
----------
Benchmarks, gcc vom 8.Now.21
Cynyon-sunset, 1/4 Size (low memeory on A4000T), Pal HigRes
noixemul, A4000T mit 68040/25 16MBytes Fast
CTRL-A-A before each test. No Ip-Stack running, IP-Switch turned off


DSTATIC_FCN = make functions static if possible
DSTATIC_VAR = make global variables static if possible
SWMEM_FAST_INLINE = Inline with volatile double polyy, 3 simple swmem-functions, gcc selects one at compiletime depending on size-parameter

 G) Github: Version 2.031 (Emerald) 1.134.884 Bytes) (Jul 19 1996 12:13:22 arcticus)  

 0)  original WCS binary from orginal Discs,  Version 2.04 (Ruby) (Apr 10 1996 14:15:52 Questar)
 1) -g -noixemul -m68000 -DSTATIC_FCN= -DSTATIC_VAR=
 2) -g -noixemul -m68020 -DSTATIC_FCN= -DSTATIC_VAR=
 3) -g -noixemul -m68020 -m68881                        -DSTATIC_FCN=       -DSTATIC_VAR=
 4) -g -noixemul -m68040                                -DSTATIC_FCN=       -DSTATIC_VAR=
 5) -g -noixemul -m68040 -fomit-frame-pointer           -DSTATIC_FCN=       -DSTATIC_VAR=
 6) -g -noixemul -m68040 -fomit-frame-pointer -fbaserel -DSTATIC_FCN=       -DSTATIC_VAR=
 7) -g -noixemul -m68040 -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=
 8) -g -noixemul -m68040 -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static
 9) -g -noixemul -m68040 -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math
10) -g -noixemul -m68040 -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -mregparm
11) -g -noixemul -m68040 -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -mregparm -Winline -DSWMEM_FAST_INLINE
12) -g -noixemul -m68040 -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -mregparm -Winline -DSWMEM_FAST_INLINE -flto

Slowdon by -fomit-frame-pointer about 0:02 -> ignore for now
           -fbaserel            about 2:40
           -DSTATIC_FCN=static  about 2:10 
           -DSWMEM_FAST_INLINE  about 5:20
            
13) -g -noixemul -m68040 -fomit-frame-pointer           -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -mregparm -Winline -DSWMEM_FAST_INLINE -flto
14) -g -noixemul -m68040 -fomit-frame-pointer           -DSTATIC_FCN=       -DSTATIC_VAR=static -ffast-math -mregparm -Winline -DSWMEM_FAST_INLINE -flto
15) -g -noixemul -m68040 -fomit-frame-pointer           -DSTATIC_FCN=       -DSTATIC_VAR=static -ffast-math -mregparm -Winline                     -flto

No big progress at all, repeat the last three without flto

16) -g -noixemul -m68040 -fomit-frame-pointer           -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -mregparm -Winline -DSWMEM_FAST_INLINE
17) -g -noixemul -m68040 -fomit-frame-pointer           -DSTATIC_FCN=       -DSTATIC_VAR=static -ffast-math -mregparm -Winline -DSWMEM_FAST_INLINE
18) -g -noixemul -m68040 -fomit-frame-pointer           -DSTATIC_FCN=       -DSTATIC_VAR=static -ffast-math -mregparm -Winline

Not better. Lets try the best with -flto

19) -g -noixemul -m68040 -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -mregparm                              -flto 

slower than without -flto.

Lets try the best wit -Os for all sources. (so far all without "GUI" in the name were compiled with -O2) (the final -Os overwrites the previous -O2)
Maybe that improves chache usage?  --> unfortunatelly no
20) -g -noixemul -m68040 -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -mregparm -Os  # -Clouds become distorted. (MapTopo.c-Problem) -> fixed with gcc from 23.Nov.21
21  -g -noixemul -m68040 -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -mregparm -Os  (MapTopo.c manually compiled with -O2)

fastest version so far (19) but with other CPU-options
22) -g -noixemul -m68060 -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -mregparm
23) -g -noixemul -m68020-40              -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -mregparm
24  -g -noixemul -m68020-40 -mhard-float -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -mregparm
25) -g -noixemul -m68020-60              -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -mregparm
26) -g -noixemul -m68020-60 -mhard-float -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -mregparm



WCS     Size     text	   data	    bss	    dec	    hex    Warnings   A4000T/040/25/16     Comment
G     1134884                                                            05:13:34
00    1068664                                                            04:59:44
01    1442044  1184016	  95032	 141048	1420096	 15ab40	     186         10:48:52
02    1410776  1170108	  95040	 141048	1406196	 1574f4      186         10:44:25
03    1180092   970148	  95028	 141048	1206224	 1267d0	     186         07:44:49
04    1209540   995488	  95028	 141048	1231564	 12cacc      186         04:48:11 
05    1203388   990124	  95028	 141048	1226200	 12b5d8      186         04:48:13
06    1080612   959964	 121500	 114580	1196044	 12400c      149         04:50:51           smaller and faster than with static functions?           
07    1081756   961088	 121500	 114580	1197168	 124470      152         04:53:02
08    1081848   961180	 121500	 114580	1197260	 1244cc      155         04:49:53
09    1070364   950232	 121496	 114580	1186308	 121a04      155         04:39:02
10    1048648   928660	 121496	 114580	1164736	 11c5c0	     155         04:30:26
11    1049728   929848	 121496	 114580	1165924	 11ca64      276         04:35:49           FAST_INLINE slower than simple function call!?
12    1056588   936048	 121624	 114580	1172252	 11e31c      177         04:33:46


13    1172828   958196	  95080	 141032	1194308	 123944      214         04:35:51
14    1181428   965528	  95216	 141032	1201776	 125670      211         04:34:21
15    1181168   964764	  95216	 141032	1201012	 125374       90         04:37:29

16    1165288   951576	  95024	 141048	1187648	 121f40      313         04:31:14
17    1173772   958924	  95024	 141048	1194996	 123bf4      310         04:37:53
18    1173164   957872	  95024	 141048	1193944	 1237d8      189         04:32:18

19    1054712   934228	 121524	 114580	1170332	 11db9c       56         04:35:39

20     949492   831448	 121496	 114580	1067524	 104a04      159         04:35:54   distorted clouds !!!???? -> MapTopo.c causes the problem. Try Maptopo manually with -O2 -> fixed with gcc from 23.Nov.21
21     952888   834816	 121496	 114580	1070892	 10572c      ???         04:36:11

22    1014920   894884   121496  114580 1130960  1141d0      155         06:08:57   compiled for 68060 is much slower on the 68040!!!
23    1253540   1123420  121512  114580 1359512  14be98      155         10:22:46   no floatingpoint commands inside? no fmove!!
24    1050664   930544   121496  114580 1166620  11cd1c      155         04:59:09   floatingpoint is back. fmove. fastmath fsin.  Wrong Picture! Rock colors distoreted!
24_02 1048276   928320	 121496	 114580	1164396	 11c46c      155         04:31:25   gcc from 27.11.21 links correct libs
25    1238852  1108712   121512  114580 1344804  148524      155         10:17:46   no floatingpoint commands inside? no fmove!! Faster than 23 !?
26    1037968   917792   121496  114580 1153868  119b4c      155         05:10:50   floatingpoint is back. fmove. fastmath fsin.  Wrong Picture! Rock colors distoreted!
26_02 1038916   918904	 121496	 114580	1154980	 119fa4      155         04:28:27   <--- fastest version on my 68040!!! gcc from 27.11.21 links correct libs

The problem with the distoreted clouds is in in MapTopo.c function MapCloud() when compiled with -Os. (added #pragma GCC push_options #pragma GCC optimize ("Os") #pragma GCC pop_options around that functions an compiled everything with -Os then.


22.11.21
--------
Not enough memory for Cayon-sunset picture in original size. (Not enough mem for cloud map), even if started with no ip-stack. 16 MBytes seem to be to little.

For reference tests with -m68060 and -m68020-60 should be done. How fast are they on my 48040?

Compiler: ${COMMAND} ${FLAGS} ${OUTPUT_FLAG} ${OUTPUT_PREFIX}${OUTPUT} ${INPUTS}                                     -g -noixemul -m68040 -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -mregparm
Linker  : ${COMMAND} ${FLAGS} ${OUTPUT_FLAG} ${OUTPUT_PREFIX}${OUTPUT} ${INPUTS} -Wl,-Map=wcs.map,--trace -ldebug    -g -noixemul -m68040 -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -mregparm

The longer linker-commandline (including -Wl,-Map=wcs.map,--trace -ldebug)  can be given for both, Compiler and Linker in eclipse.



23.11.21
--------
The new gcc from 23.11.21 fixes the -Os issue found in 20) "distorted clouds !!!????"
The other issues 24) and 26) "Wrong Picture! Rock colors distoreted!" is still there

25.11.21
--------
Suche nach Problem 24/26
Projekt komplett mit Fehler bauen (-m68020-40)
ls -1 *.o
Haelfte der Files nach files.txt kopieren. (das Assembler-File nicht)
ein Objekt loeschen, dann make all, -> dann haben wir die Kommandozeile zum bauen eines Objekts, geaenderte Optionen verwenden (hier statt -m68020-40 wieder -m68040)
File durch Variable ersetzen, folgendes Script (mit der gefundenen Compiler-Zeile) aufrufen

set -x
for FILE in $(cat files.txt); do
FILE="${FILE%.*}"; m68k-amigaos-gcc -DFORCE_MUIMASTER_VMIN=10 -DAMIGA_GUI -DTOOLCHAIN_VER=\"'%@@=492:?iab}@Ga` 9EEADi^^8:E9F3]4@>^3633@^2>:82\\844]8:E 2>:82\\844i3bc3c3` 2>:82\\?6E:?4=F56i_b6ch77 2C@D\\DEF77idfgedd4 3:?FE:=Di_d`7675g5 4=:3ai7hh7a6d 75aAC28>2i_hc67c7 75aD75i35f6e55 844ide_4fddhe :C2i333f2ge :I6>F=i7_55hhf =:3563F8ica5h5b2 =:3?:Ii6h35276 =:3$s{`aibac7e34 ?6H=:3\\4J8H:?i526`4`_ $sxic2f7d7e D754ia4_dag` G2D>i26b62a4 G344i757fg2h G=:?<ia7`6_3a'\" -I"/home/developer/Desktop/SelcoGit/3DNature/Amiga" -Os -Wall -c -fmessage-length=0 -funsigned-char -MMD -MP -MF"$FILE.d" -MT"$FILE.o" -o "$FILE.o" "../$FILE.c" -g -noixemul -m68040 -mhard-float -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -mregparm
done
set +x
make all # Linken

A--> Geht es jetzt wieder? Fehler in der ersten Haelfte usw.

26.11.21
--------
-> Es werden die falschen libraries gelinkt. (keine m68881). mit -m68881 statt -mhard-float im Linker-Aufruf geht es wieder. 

- Wenn wir 2xhalbe Groesse machen, sind Sonne und Mond viel zu gross. (Canyon-sunset)
- Wenn ich auf auf Edit im Database-Module klicke, stuerzt WCS ab, zumindest in meiner (letzten??) Version. Im Original scheint es zu gehen.

Die Groesse von Mond und Sonne muesste wohl in STATIC_FCN void ApplyImageScale(void) in MoreGui.c angepasst werden. ??
Im MotionEditor gibt es Sun Size und Moon Size.
Die lassen sich nicht �ndern!? Mit den kleinen Pfeilen kommt Bl�dsinn raus und manuell wird es nicht �bernommen? (Ist beim n�chsten �ffnen des Fensters wieder auf dem alten Wert.)
-> Mit den Pfeilen wird +- 1000 gemacht, das ist bei der Sonne und dem Mond viel zu viel.
-> Um die Werte zu �bernehmen, muss man nach einer �nderung *jeweils* einen KeyFrame erzeugen, dann das Fenster mit Keep schlie�en.
Wenn man Sonne und Mond auf 1/4 der Zahl setzt, stimmt die Gr��e wieder.
-> Es gibt bei Motion einen Punk Scale. Der �ndert sichm wenn man die Gr��e des Bilder mit Halv oder double andert.
-> Anscheinend �ndert der sich falschrum. Also bei Halve wird Scale verdoppelt, wahrscheinlich w�re halbieren richtig. Nein. Zumindest bei der Landschaft ist Scale richtig rum
   Wenn man den Manuell �ndert, �ndert sich Gr��e der Berge richtig rum. Vielleicht nur Mond und Sonne falsch rum?
-> Meine WCS-Varianten stuerzen ab, wenn im Database-Editor auf Edit gedrueckt wird. -> Liegt an MUI. Wenn mui38usr.lha (muimaster.library 19) installiert ist, geht es. Ich erfordere im Moment muimaster.library 10 wie das originale WCS.

29.11.21
--------
Neuere Compiler vom 27.11.21 -> 24 und 26 funktionieren jetzt.

4.Dez.2021
----------
Version 26_2 ist die schnellste bisher. Komisch, auf meinem C=A4000T mit 68040 ist die 68020-60 schneller als die 68040 Version? Mehrfach nachtesten!

- Test Canyon Origina�-Groesse:
Bisher imer mit 2x Halbe Gr��e wegen zuwenig Ram. Jetzt in Startup-Sequence das ... raugenommen. Dann habe ich nur den PAL-Bildschirmmodus, daf�r aber etwas mehr RAM. (14 MBytes Fast) Damit kann ich das Canyon Sunset-Bild in Originalgr��e berechnen.

26_2, Canyon Sunset Originalgr��e 02:59:36 !!! Viel Schneller als 2x Halbe Gr��e (04:28:27) !!!
--> Buttons werden falsch angezeigt (Grafikm�ll) WCS funktioniert aber. (Die Original-Versionen von WCS zeigen alles richtig an) Evtl.Grafik im Fast-Ram???

 G) Github: Version 2.031 (Emerald)                                03:46:44
 0) original WCS binary from orginal Discs,  Version 2.04 (Ruby)   03:47:23

26_2 mit 1/4 Groesse �ber Prferences                               01:16:12 
G mit 1/4 Groesse �ber Prferences                                  01:33:20
0 mit 1/4 Groesse �ber Prferences                                  01:31:06

7.Dez.2021
----------
Ich erfordere jetzt "F:ORCE_MUIMASTER_VMIN=19" in den Eclipse/settings, also dem Makefile. Das ist MUI 3.8. Mit Version10 (also MUI 2.3) stuerzt das Editor-Window ab. (Siehe 26.11.21)
* Zum schnellen Vorschau-Berechnen Preferences 1/4 Size nehmen. Nicht die Groesse im Render-Dialog veraendern, sonst stimmt die Groesse von Sonne und Mond nicht und es dauert viel laenger als bei voller Groesse.

* Parameter Module -> Motion -> Cam View st�rzt in meinen Versionen immer ab.
 
8.Dez.2021
----------
Neuer gcc von Bebbo. __chip funktioniert jetzt, damit sind die Bilder und Buttons jetzt immer in Ordnung.

11.01.2022
----------
- Neuer gcc vom 9.Jan
Text Bebbo:
"der Switch -m68881 wird ignoriert, wenn -m68040 oder -m68060 (oder -m68080) verwendet wird, das beinhaltet -m68881.

Der 68040 und der 68060 haben ja keine vollwertige FPU, sondern emulieren einige Befehle mittels F-Line Exception (oder so), was Zeit kostet. Deswegen war -ffast-math f�r diese Targets nicht so toll, denn da wurden die Befehle, wie fsin, fcos usw. verwendet. Nun erzeugt -ffast-math f�r m68040/60 direkte MathIeee Aufrufe: https://franke.ms/cex/z/75YoWe
"

17.05.2022
----------
neuer gcc unterstuetzt garbage collection -> entfernt unbenutzte Funktionen und Daten. (executable stuerzt aber noch ab!)
--> trotzdem hat der gcc noch einige weitere unbenutzte Funkionen gefunden, die ich bisher nicht als solche identifiziert hatte.
Wegdefinieren dieser Funktionen spart 2460 Bytes. (UNUSED_FUNCTIONS_GC)
Das muss auch mit unbenutzten Daten noch ausprobiert werden.


