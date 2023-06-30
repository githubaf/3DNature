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
--> erst mal libnix mit -pg bauen. Dann ist der Profiler hoffentlich aussagekräftiger.

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
Eineige Files waren UTF-8 kodiert. Das fuehrt zu seltsamer Darstellung auf dem Amiga. mit iconv nach ISO 8859-1 konvertiert.


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
FractPoint_Sort() macht 5,8% aus, hier könnte man optimieren?

Diesen Profiler-Lauf mal mit Amiga oder cycle-exact laufen lassen...

10.August 2021
--------------
WCS 2.04 auf dem C=A4000T (040/25) 2MBytes Chip, 16Meg Fast)
Canyon Sunset, Pal-Hires, Groeße/4 05:08:28

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
Canyon Sunset, Pal-Hires, Groeße/4 05:08:28

Neu mit gcc:
WCS_gcc_Release_Static 68020/68881 auf dem C=A4000T (040/25) 2MBytes Chip, 16Meg Fast)
Canyon Sunset, Pal-Hires, Groeße/4 07:36:02    !!! Warum so langsam? Trotz static  

Statt -m68020 -m66881 jetzt nur -m68040
WCS_gcc_Release_Static_68040 auf dem C=A4000T (040/25) 2MBytes Chip, 16Meg Fast)
Canyon Sunset, Pal-Hires, Groeße/4 10:29:07   !!!!!!!!!


WCS_gcc_Release_Static_68040_mhard-float auf dem C=A4000T (040/25) 2MBytes Chip, 16Meg Fast)
Canyon Sunset, Pal-Hires, Groeße/4 7:35:17, also auch nicht viel besser als 68020/68881


Auf dem Amiga muss zum SAS/C compilieren noch http://aminet.net/dev/c/SDI_headers.lha installiert werden und http://aminet.net/dev/mui/mui38dev.lha

Compilieren auf dem C=A4000T mit Smbfs 2.2 ist extrem langsam, geht aber. -> Besser unter WinUAE machen.

WCS_smake_optimize auf dem C=A4000T (040/25) 2MBytes Chip, 16Meg Fast)
Canyon Sunset, Pal-Hires, Groeße/4 4:53:40    --> bisher schnellste Variante

(15.Sep.21)
WCS_GCC_baserel_O2Os (1048264 Bytes) auf dem C=A4000T (040/25) 2MBytes Chip, 16Meg Fast)
GUI-Files mit -Os, die anderen mit -O2 
Canyon Sunset, Pal-Hires, Groeße/4 7:24:29. In WinUAE braucht es 1:04 statt 1:30, auf dem Amiga aber keine wesentliche Verbesserung!


26.8.2021
---------
mit -mfast-math übersetzt -> 7:31:54, also praktisch keine Verbesserung.


// https://stackoverflow.com/questions/24348227/how-to-disable-double-precision-math-while-compiling-with-gcc-or-and-iar
gcc has an optimization option -fsingle-precision-constant that treats ordinary floating point constants as single precision

-Wdouble-promotion does exactly what you want, see the doc, under Warning Options. The example in the doc is quite similar to yours by the way.

Here is basically your example:

float f(int int_var, float float_var_2) {
  return 3.0 * int_var / float_var_2;
}


Coverage:
gcovr --object-directory=. -r . --html --html-details -o coverage.html

-> Beim Compilieren UND linken -noixemul nicht vergessen. Stack erhöhen!


19.9.2021
---------
Jetzt mit -m68020 -m68881 -noixemul  -fomit-frame-pointer -DSTATIC_FCN=static -fbaserel -flto -D__inline="inline static" compiliert. (Direkte fpu-Instructions durch math-68881.h)
WCS_gcc_Rel_Stat_br68881 auf dem C=A4000T (040/25) 2MBytes Chip, 16Meg Fast)
Canyon Sunset, Pal-Hires, Groeße/4  7:24:17  also trotz durchgehend fpu-Instructions nicht schneller!!!

- Mit -m68040 compiliert und gelinkt.( -m68040 -noixemul  -fomit-frame-pointer -DSTATIC_FCN=static -fbaserel -flto -D__inline="inline static")
WCS_gcc_Rel_Stat_br68881 auf dem C=A4000T (040/25) 2MBytes Chip, 16Meg Fast)
Canyon Sunset, Pal-Hires, Groeße/4  4:49:14, schnellstes Ergebnis!

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
Canyon Sunset, Pal-Hires, Groeße/4  4:42:22, schnellstes Ergebnis!

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
Canyon Sunset, Pal-Hires, Groeße/4  4:35:17, schnellstes Ergebnis!

8.Oktober 2021
--------------
-ffast-math funktioniert mit -m68040, wenn zusaetzlich -mregparm angegeben wird, sonst nicht!

Die Variante mit -fbaserel (-m68040 -ffast-math -mreparm -fbaserel -flto) ist 120k kleiner als die ohne -fbaserel.

Weiter (STRPTR) eingefügt:
find . -name "*.[c\|h]" -exec  sed -i "s/\(nm_Label *= *\)\(".*"\)/\1 (STRPTR)\2/g" {} \;

886 Warnings

- Mit -m68040 und -mregparm ohne -fbaserel compiliert und gelinkt.( -m68040 -noixemul  -fomit-frame-pointer -DSTATIC_FCN=static -flto -mregparm -D__inline="inline static")
WCS_040_basr_mregparm_lto auf dem C=A4000T (040/25) 2MBytes Chip, 16Meg Fast)
Canyon Sunset, Pal-Hires, Groeße/4  4:35:56, minimal langsamer, 10% groesser

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

Etwa 190 Stellen, an den swmem aufegrufen wird. Scheinen ALLE eine Konstante für die Anzahl zu haben.
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
Die lassen sich nicht ändern!? Mit den kleinen Pfeilen kommt Blödsinn raus und manuell wird es nicht übernommen? (Ist beim nächsten Öffnen des Fensters wieder auf dem alten Wert.)
-> Mit den Pfeilen wird +- 1000 gemacht, das ist bei der Sonne und dem Mond viel zu viel.
-> Um die Werte zu übernehmen, muss man nach einer Änderung *jeweils* einen KeyFrame erzeugen, dann das Fenster mit Keep schließen.
Wenn man Sonne und Mond auf 1/4 der Zahl setzt, stimmt die Größe wieder.
-> Es gibt bei Motion einen Punk Scale. Der ändert sichm wenn man die Größe des Bilder mit Halv oder double andert.
-> Anscheinend ändert der sich falschrum. Also bei Halve wird Scale verdoppelt, wahrscheinlich wäre halbieren richtig. Nein. Zumindest bei der Landschaft ist Scale richtig rum
   Wenn man den Manuell ändert, ändert sich Größe der Berge richtig rum. Vielleicht nur Mond und Sonne falsch rum?
-> Meine WCS-Varianten stuerzen ab, wenn im Database-Editor auf Edit gedrueckt wird. -> Liegt an MUI. Wenn mui38usr.lha (muimaster.library 19) installiert ist, geht es. Ich erfordere im Moment muimaster.library 10 wie das originale WCS.

29.11.21
--------
Neuere Compiler vom 27.11.21 -> 24 und 26 funktionieren jetzt.

4.Dez.2021
----------
Version 26_2 ist die schnellste bisher. Komisch, auf meinem C=A4000T mit 68040 ist die 68020-60 schneller als die 68040 Version? Mehrfach nachtesten!

- Test Canyon Original-Groesse:
Bisher imer mit 2x Halbe Größe wegen zuwenig Ram. Jetzt in Startup-Sequence das ... raugenommen. Dann habe ich nur den PAL-Bildschirmmodus, dafür aber etwas mehr RAM. (14 MBytes Fast) Damit kann ich das Canyon Sunset-Bild in Originalgröße berechnen.

26_2, Canyon Sunset Originalgröße 02:59:36 !!! Viel Schneller als 2x Halbe Größe (04:28:27) !!!
--> Buttons werden falsch angezeigt (Grafikmüll) WCS funktioniert aber. (Die Original-Versionen von WCS zeigen alles richtig an) Evtl.Grafik im Fast-Ram???

 G) Github: Version 2.031 (Emerald)                                03:46:44
 0) original WCS binary from orginal Discs,  Version 2.04 (Ruby)   03:47:23

26_2 mit 1/4 Groesse über Prferences                               01:16:12 
G mit 1/4 Groesse über Prferences                                  01:33:20
0 mit 1/4 Groesse über Prferences                                  01:31:06

7.Dez.2021
----------
Ich erfordere jetzt "F:ORCE_MUIMASTER_VMIN=19" in den Eclipse/settings, also dem Makefile. Das ist MUI 3.8. Mit Version10 (also MUI 2.3) stuerzt das Editor-Window ab. (Siehe 26.11.21)
* Zum schnellen Vorschau-Berechnen Preferences 1/4 Size nehmen. Nicht die Groesse im Render-Dialog veraendern, sonst stimmt die Groesse von Sonne und Mond nicht und es dauert viel laenger als bei voller Groesse.

* Parameter Module -> Motion -> Cam View stürzt in meinen Versionen immer ab.
 
8.Dez.2021
----------
Neuer gcc von Bebbo. __chip funktioniert jetzt, damit sind die Bilder und Buttons jetzt immer in Ordnung.

11.01.2022
----------
- Neuer gcc vom 9.Jan
Text Bebbo:
"der Switch -m68881 wird ignoriert, wenn -m68040 oder -m68060 (oder -m68080) verwendet wird, das beinhaltet -m68881.

Der 68040 und der 68060 haben ja keine vollwertige FPU, sondern emulieren einige Befehle mittels F-Line Exception (oder so), was Zeit kostet. Deswegen war -ffast-math für diese Targets nicht so toll, denn da wurden die Befehle, wie fsin, fcos usw. verwendet. Nun erzeugt -ffast-math für m68040/60 direkte MathIeee Aufrufe: https://franke.ms/cex/z/75YoWe
"

17.05.2022
----------
neuer gcc unterstuetzt garbage collection -> entfernt unbenutzte Funktionen und Daten. (executable stuerzt aber noch ab!)
--> trotzdem hat der gcc noch einige weitere unbenutzte Funkionen gefunden, die ich bisher nicht als solche identifiziert hatte.
Wegdefinieren dieser Funktionen spart 2460 Bytes. (UNUSED_FUNCTIONS_GC)
Das muss auch mit unbenutzten Daten noch ausprobiert werden.

18.Mai 2022
-----------
Release funktioniert nicht mehr richtig? Alte WCS-Verisonen/Alte Compiler testen!

WCS                        gcc
-----------------------------------------------------------
HEAD detached at 8903a34   gcc 08Dec21 ok
current                    gcc 08Dec21 ok

current                    gcc 17May22 HALT3
current                    gcc 28Apr22     kein MUI.h
current                    gcc 20Apr22 HALT3
current                    gcc 28Feb22 HALT3
-------------------------------------------------
current                    gcc 24Dec21 ok   # letzter gcc 2021
current                    gcc 08Jan22 ok
current                    gcc 09Jan22 ok

current                    gcc 20Jan22 ---  -->09Jan22
current                    gcc 25Jan22 ---  -->09Jan22
current                    gcc 26Jan22 ---  -->09Jan22
current                    gcc 27Jan22 HALT3   Also ab hier HALT3
current                    gcc 28Jan22 ---  -->27Jan22
current                    gcc 29Jan22 HALT3
current                    gcc 30Jan22 HALT3

Stack ueberpruefen!

current                    gcc 18May22 HALT3
--> libnix zuruck auf Stand vom 9.Jan  ->  adc11af  

cd ~/amiga-gcc/projects/libnix/
git checkout adc11af
cd cd ~/amiga-gcc/
make clean-libnix
time make libnix PREFIX=/home/developer/opt/m68k-amigaos_18May22

18. Juni 2022
-------------
Bebbos gcc kann jetzt garbage collection! (compilieren mit -ffunction-sections -fdata-sections und linken mit --gc-sections)
Seit dem 16.6.2022 funktioniert das auch mit WCS. (vorher wurde amiga.lib nicht richtig behandelt, Bebbo hat das jetzt korrigiert)
Mit -Wl,-Map=wcs_gc.map,--trace,--gc-sections,--print-gc-sections sieht man jetzt, was entfernt wurde. Das kann man also auch manuell aus dem Code rausnehmen.

Eclipse Targets Release unf gc-sections ueberarbeitet. Das Executable heisst jetzt WCS_Release oder WCS_gc-sections. 
Die Map-Files haben auch den erweiterten Namen.
Jetzt wird auch das Original-WCS-Icon mit dem neuen Namen kopiert.

WCS funktioniert mit Release und gc-section Buldconfiguration mit dem Compiler m68k-amigaos_16Jun22.

-> -gc-collection hat keinen Effekt, wenn mit -flto compiliert / gelinkt wird. Ist das richtiG? Testen!

20.Juni 2022
------------
Arexx:
WCS hat Arexx-Support vorbereitet. Das steckt in Markow

# Fuer die AREXX-Kommandos: Nachschauen in info/GrammarOut
#---------------------------------------------------------
Diese Kommandos erzeugen eine Ausgabe im "Status Log" Fenster
Rx "ADDRESS WCS.1 project load 'Work:WCS_204/WCSProjects/CanyonSunset.proj' " 
Rx "ADDRESS WCS.1 parameters load 'WCSProjects/Arizona/SunsetAnim.object/SunsetAnim.par' "
Rx "ADDRESS WCS.1 PROject Project PATh 'work:' "
Rx "ADDRESS WCS.1 PARameters RENder STArt FRAMes 1"
Rx "ADDRESS WCS.1 PROject Mapview open" 
Rx "ADDRESS WCS.1 PROject Mapview set size 1"

Rx "ADDRESS WCS.1 parameters render start frames 1 " 
Rx "ADDRESS WCS.1 project quit" 

Rx "ADDRESS WCS.1 status inquire" 
Rx "ADDRESS WCS.1 status notifyme" 

Die Befehle sind nicht im Quelltext! Die stecken in zwei automatische generierten Tabellen. VocabTable.c und GrammarTable.c. Diese wurden aus Markow.c generiert.
Das Source-File ist wohl info/Grammartest.

Ein kleiner (Original-)Test ist wcstest.rexx. Ich habe da Nummern ringeschrieben, um zu sehen, welche Ausgabe wozu gehört.


22.Juni 2022
------------
gcc vom 18Jun22
Vergleich zur 26_2 weiter oben

WCS       Size     text    data     bss    dec     hex    Warnings   A4000T/040/25/16     Comment
Alt26_2 1038916   918904  121496  114580 1154980  119fa4      155         04:28:27   <--- fastest version on my 68040!!! gcc from 27.11.21 links correct libs
   27   1044812   934368  105376  114580 1154324  119d14      154                    <--- now fastes version!!! saved 16 Minutes! gcc from 18Jun22
ohne lto
   28 1037904   928128  105244  114580 1147952  118430      263
dafuer mit -lstack
   29 1037904   928128  105244  114580 1147952  118430 ??? genau so gross???

Alt26_2 -g -noixemul -m68020-60 -mhard-float -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -mregparm
27   m68k-amigaos-gcc -DFORCE_MUIMASTER_VMIN=19 -DAMIGA_GUI -DTOOLCHAIN_VER=\"'%@@=492:?i`gyF?aa 9EEADi^^8:E9F3]4@>^3633@^2>:82\\844]8:E 2>:82\\844id4h`f67 2>:82\\?6E:?4=F56i_b6ch77 2C@D\\DEF77i7ca__gd 3:?FE:=Di5eba4c_be 4=:3ai5d`65_4 75aAC28>2i6hg7_h_ 75aD75i35f6e55 844i`e`e`fch4 :C2i333f2ge :I6>F=i7_55hhf =:3563F8ica5h5b2 =:3?:Iibfd7f53 =:3$s{`aibac7e34 ?6H=:3\\4J8H:?idfa2c3c $sxic2f7d7e D754if34`42g G2D>i26b62a4 G344iaffgb62 G=:?<ia7`6_3a'\" -I"/home/developer/Desktop/SelcoGit/3DNature/Amiga" -O2 -Wall -c -fmessage-length=0 -funsigned-char -MMD -MP -MF"vgl/color.d" -MT"vgl/color.o" -o "vgl/color.o" "../vgl/color.c" -noixemul -m68040 -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -mregparm -Winline -DSWMEM_FAST_INLINE -flto
27   m68k-amigaos-gcc  -o "WCS"_Release  ./vgl/color.o ./vgl/dumb.o ./vgl/dumbpoly.o ./vgl/pixmap.o ./vgl/wuline.o  ./esp/.metadata/.plugins/org.eclipse.cdt.make.core/specs.o  ./AGUI.o ./BitMaps.o ./Cloud.o ./CloudGUI.o ./ColorBlends.o ./Commands.o ./DEM.o ./DEMGUI.o ./DEMObject.o ./DLG.o ./DataBase.o ./DataOps.o ./DataOpsGUI.o ./DefaultParams.o ./DiagnosticGUI.o ./DispatchGUI.o ./EdDBaseGUI.o ./EdEcoGUI.o ./EdMoGUI.o ./EdPar.o ./EdSetExtrasGUI.o ./EdSetGUI.o ./EditGui.o ./EvenMoreGUI.o ./Foliage.o ./FoliageGUI.o ./Fractal.o ./GenericParams.o ./GenericTLGUI.o ./GlobeMap.o ./GlobeMapSupport.o ./GrammarTable.o ./HelpGUI.o ./HyperKhorner4M-1_gcc.o ./Images.o ./InteractiveDraw.o ./InteractiveUtils.o ./InteractiveView.o ./LWSupport.o ./LineSupport.o ./MUIFloatInt.o ./MakeFaces.o ./Map.o ./MapExtra.o ./MapGUI.o ./MapLineObject.o ./MapSupport.o ./MapTopo.o ./MapTopoObject.o ./MapUtil.o ./Memory.o ./Menu.o ./MoreGUI.o ./Params.o ./ParamsGUI.o ./PlotGUI.o ./RequesterGUI.o ./RexxSupport.o ./ScratchPad.o ./ScreenModeGUI.o ./Support.o ./TLSupportGUI.o ./TimeLinesGUI.o ./Tree.o ./Version.o ./VocabTable.o ./WCS.o ./Wave.o ./WaveGUI.o ./nncrunch.o ./nngridr.o ./sasc_functions.o    -lmui -lm -Wl,-Map="WCS"_Release.map,--trace -noixemul -m68040 -fbaserel -flto

28  m68k-amigaos-gcc -DFORCE_MUIMASTER_VMIN=19 -DAMIGA_GUI -DTOOLCHAIN_VER=\"'%@@=492:?i`gyF?aa 9EEADi^^8:E9F3]4@>^3633@^2>:82\\844]8:E 2>:82\\844id4h`f67 2>:82\\?6E:?4=F56i_b6ch77 2C@D\\DEF77i7ca__gd 3:?FE:=Di5eba4c_be 4=:3ai5d`65_4 75aAC28>2i6hg7_h_ 75aD75i35f6e55 844i`e`e`fch4 :C2i333f2ge :I6>F=i7_55hhf =:3563F8ica5h5b2 =:3?:Iibfd7f53 =:3$s{`aibac7e34 ?6H=:3\\4J8H:?idfa2c3c $sxic2f7d7e D754if34`42g G2D>i26b62a4 G344iaffgb62 G=:?<ia7`6_3a'\" -I"/home/developer/Desktop/SelcoGit/3DNature/Amiga" -O2 -Wall -c -fmessage-length=0 -funsigned-char -MMD -MP -MF"vgl/color.d" -MT"vgl/color.o" -o "vgl/color.o" "../vgl/color.c" -noixemul -m68040 -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -mregparm -Winline -DSWMEM_FAST_INLINE
28  m68k-amigaos-gcc  -o "WCS"_Release  ./vgl/color.o ./vgl/dumb.o ./vgl/dumbpoly.o ./vgl/pixmap.o ./vgl/wuline.o  ./esp/.metadata/.plugins/org.eclipse.cdt.make.core/specs.o  ./AGUI.o ./BitMaps.o ./Cloud.o ./CloudGUI.o ./ColorBlends.o ./Commands.o ./DEM.o ./DEMGUI.o ./DEMObject.o ./DLG.o ./DataBase.o ./DataOps.o ./DataOpsGUI.o ./DefaultParams.o ./DiagnosticGUI.o ./DispatchGUI.o ./EdDBaseGUI.o ./EdEcoGUI.o ./EdMoGUI.o ./EdPar.o ./EdSetExtrasGUI.o ./EdSetGUI.o ./EditGui.o ./EvenMoreGUI.o ./Foliage.o ./FoliageGUI.o ./Fractal.o ./GenericParams.o ./GenericTLGUI.o ./GlobeMap.o ./GlobeMapSupport.o ./GrammarTable.o ./HelpGUI.o ./HyperKhorner4M-1_gcc.o ./Images.o ./InteractiveDraw.o ./InteractiveUtils.o ./InteractiveView.o ./LWSupport.o ./LineSupport.o ./MUIFloatInt.o ./MakeFaces.o ./Map.o ./MapExtra.o ./MapGUI.o ./MapLineObject.o ./MapSupport.o ./MapTopo.o ./MapTopoObject.o ./MapUtil.o ./Memory.o ./Menu.o ./MoreGUI.o ./Params.o ./ParamsGUI.o ./PlotGUI.o ./RequesterGUI.o ./RexxSupport.o ./ScratchPad.o ./ScreenModeGUI.o ./Support.o ./TLSupportGUI.o ./TimeLinesGUI.o ./Tree.o ./Version.o ./VocabTable.o ./WCS.o ./Wave.o ./WaveGUI.o ./nncrunch.o ./nngridr.o ./sasc_functions.o    -lmui -lm -Wl,-Map="WCS"_Release.map,--trace -noixemul -m68040 -fbaserel

mit -lstack linken, damit __stack zum seten einer Stacksize funktioniert.
29 m68k-amigaos-gcc -DFORCE_MUIMASTER_VMIN=19 -DAMIGA_GUI -DTOOLCHAIN_VER=\"'%@@=492:?i`gyF?aa 9EEADi^^8:E9F3]4@>^3633@^2>:82\\844]8:E 2>:82\\844id4h`f67 2>:82\\?6E:?4=F56i_b6ch77 2C     @D\\DEF77i7ca__gd 3:?FE:=Di5eba4c_be 4=:3ai5d`65_4 75aAC28>2i6hg7_h_ 75aD75i35f6e55 844i`e`e`fch4 :C2i333f2ge :I6>F=i7_55hhf =:3563F8ica5h5b2 =:3?:Iibfd7f53 =:3$s{`aibac7e34 ?6H=:3\\4     J8H:?idfa2c3c $sxic2f7d7e D754if34`42g G2D>i26b62a4 G344iaffgb62 G=:?<ia7`6_3a'\" -I"/home/developer/Desktop/SelcoGit/3DNature/Amiga" -O2 -Wall -c -fmessage-length=0 -funsigned-char -     MMD -MP -MF"vgl/color.d" -MT"vgl/color.o" -o "vgl/color.o" "../vgl/color.c" -noixemul -m68040 -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static -ffast-math -mregp     arm -Winline -DSWMEM_FAST_INLINE 
29 m68k-amigaos-gcc  -o "WCS"_Release  ./vgl/color.o ./vgl/dumb.o ./vgl/dumbpoly.o ./vgl/pixmap.o ./vgl/wuline.o  ./esp/.metadata/.plugins/org.eclipse.cdt.make.core/specs.o  ./AGUI.o      ./BitMaps.o ./Cloud.o ./CloudGUI.o ./ColorBlends.o ./Commands.o ./DEM.o ./DEMGUI.o ./DEMObject.o ./DLG.o ./DataBase.o ./DataOps.o ./DataOpsGUI.o ./DefaultParams.o ./DiagnosticGUI.o .     /DispatchGUI.o ./EdDBaseGUI.o ./EdEcoGUI.o ./EdMoGUI.o ./EdPar.o ./EdSetExtrasGUI.o ./EdSetGUI.o ./EditGui.o ./EvenMoreGUI.o ./Foliage.o ./FoliageGUI.o ./Fractal.o ./GenericParams.o .     /GenericTLGUI.o ./GlobeMap.o ./GlobeMapSupport.o ./GrammarTable.o ./HelpGUI.o ./HyperKhorner4M-1_gcc.o ./Images.o ./InteractiveDraw.o ./InteractiveUtils.o ./InteractiveView.o ./LWSupp     ort.o ./LineSupport.o ./MUIFloatInt.o ./MakeFaces.o ./Map.o ./MapExtra.o ./MapGUI.o ./MapLineObject.o ./MapSupport.o ./MapTopo.o ./MapTopoObject.o ./MapUtil.o ./Memory.o ./Menu.o ./Mo     reGUI.o ./Params.o ./ParamsGUI.o ./PlotGUI.o ./RequesterGUI.o ./RexxSupport.o ./ScratchPad.o ./ScreenModeGUI.o ./Support.o ./TLSupportGUI.o ./TimeLinesGUI.o ./Tree.o ./Version.o ./Voc     abTable.o ./WCS.o ./Wave.o ./WaveGUI.o ./nncrunch.o ./nngridr.o ./sasc_functions.o    -lmui -lm -Wl,-Map="WCS"_Release.map,--trace -noixemul -m68040 -fbaserel -lstack
 

8.Juli 2022
-----------
Mal wieder mit SAS/C compilieren:
#in die user-startup
assign include: Work:MUI/Developer/C/Include/ add
assign include: work:SDI/includes/ add

setenv BUILDID=AF_BuildID
copy env:BUILDID envarc:   ; envarc:BUILDID is needed during smake. What is its purpose?

smake

14.Juli 2022
------------
Die Bilder hatten oft ein Byte zuviel im Body. (Auch im original WCS204) Das habe ich inzwischen korriguert.
Bei ungefaehr die Haelfte aller Bilder enthielt der Body-Chunk 1 Bytes zuviel.
Ich habe iff-Tools von Thomas Rapp bekommen und fuer Linux angepasst. Damit kann man die Bilder schnell unter Linux auf IFF-Korrektheit pruefen. Ich habe ein Script geschrieben:

~/Desktop/SelcoGit/iff_tests$ cat test_wcs_bilder.sh 
set -e  # Abort on error

for BILD in $(find ~/Desktop/WCSFrames/ -type f | grep -v "\.info" | sort); do 
	echo $BILD
	./CheckPack_Linux $BILD
	./iff1_Linux $BILD
	./iff2_Linux $BILD DUMP NOBODY
	iff/testfile_linux $BILD
	echo ---------------------
	
done


Das gcc-WCS erzeugt anscheinend andere Bilder als das original WCS. Der sichtbare Ausschnitt stimmt nicht ganz überein, Wolken sind anders und einige Schaumkoepfe auf dem Wasser???

compare -compose src CanyonSet000 ~/Desktop/CanyonSet000_WCS204 DiffImage
display DiffImage

Das zeigt Unterschiede rot an. Es ist fast alles rot!


20.Juli 2022
------------
export LW getestet. (Ram:/Objects korrigiert, Strich muss weg wenn : davor ist)
Exportierte Szene in LW geladen. Dort ist der gleiche Unteschied SAS/C <-> GCC Versionen zu sehen wie im berechneten Bild, also leicht unteschiedlicher Blickwinkel. Analysieren!
(lws-Files unterscheiden sich auch, da ansetzten. Fehler passiert also vor der Bildberechnung)

- Beim Compilieren sollte ein Test rein, ob in Git ausgecheckte Files vornaden waren. Eine Warnung oder so. Sonst meinen letzten Git-Hash mit aufnhmen. Vielleicht an der Seriennumern-Stelle?

21.07.2022
----------
Wo kommen die Unterschiede her? LWS-Scenefiles verglichen.
Printausgaben eingefuegt in LWScene_Export() . Dort unterscheiden sich PP.x * 1000.0, PP.y * 1000.0, PP.z * 1000.0 recht deutlich. 
PP wird von LWOB_Export() gesetzt. Das PP kommt aus MapUtil.c convertpt(&DP); 
In MapUtil.c convertpt() wird  
PT->x = -radius * sin(PT->lon) * cos(PT->lat);
PT->y = radius * sin(PT->lat);
berechnet. 
PT->lat und lon sind gleich zur SAS/C Version, PT->lat unterscheidet sich deutlich. Wo wird das gesetzt?

Alle Aufrufe mit printf versehen. Unser Aufruf kommt von
./LWSupport.c:LWOB_Export() Zeile 444: calling convertpt()
Gleich beim ersten Aufruf ist  gcc:ptelev=2316.000000 SAS:ptelev=1742.000000. Damit bekommt DP.alt natuerlich auch einen anderen Wert.

Wo Kommt ptelev her?
wird ein paar Zeilen Vorher berechnet.
../LWSupport.c:LWOB_Export() Zeile 437: gcc: map.map[0]=2316 sas:map.map[0]=1742

Wo kommt map.map[0]=2316 her?

Dem.c
short readDEM(char *filename, struct elmapheaderV101 *map)
DEM.c:readDEM() Zeile 79: Version=1.020000

00000030  d2 f1 a9 fc 09 0c 02 8b  00 01 5f 90 49 d3 65 f8  |.........._.I.e.|
00000040  4d 06 e8 cd 06 ce 06 ca  06 c9 06 c7 06 c6 06 c3  |M...............|

Wir lesen also von verschiedenen Stellen:
if ((read (fhelev, map->map, map->size)) != map->size)  // vorher lseek(fh,0,SEEK_CUR) gemacht fuer die Anzeige
gcc:  ../DEM.c:readDEM() Zeile 105: FilePosition ist 52
sas:                                FilePosition ist 68

Ein Versionsvergleich geht wohl schief:
DEM.c:readDEM() Zeile 35: 
if (abs(Version - 1.00) < .0001)
gcc (abs(Version - 1.00)=0.000000
sas (abs(Version - 1.00)=0.020000   <--- SAS ist richtig

--> abs liefert int zurueck, zumindest laut linux manpage. SAS/C liefert das in diesem Fall gewuenschte Ergebnis...
Es muesste fabs() fuer Fliesskommazahlen benutzt werden.
--> Im Code gibt es 195 abs()-Aufrufe... Viel zu ueberpruefen. 

fabs() in Dem.c fuer die Versionstests benutzt. Die Landschaft scheint jetzt aus dem korrekten Blicjwinkel berechnet zu werden. Wolken usw sind aber noch unterschiedlich zwischen allen Versionen!

22.Juli 2022
-------------
find . -name "*.c" -exec grep -nHi "abs.*(" {} \; | grep -v fabs

MapTopo.c abs() durch fabs() ersetzt - alle haben double Parameter
InteractiveDraw.c abs() bis auf eins durch fabs() ersetzt
Fractal.c abs() durch fabs() ersetzt (etliche waren schon fabs()
DataOpsGUI.c abs() durch fabs() ersetzt
EdPar.c abs() durch fabs() ersetzt
MapUtil.c durch fabs() ersetzt
Database.c  durch fabs() ersetzt, hier war es auch ein Versionstest
ColorBlends.c abs() durch fabs() ersetzt
DEM.c einige abs() durch fabs() ersetzt
Maps.c einige abs() durch fabs() ersetzt
MapExtra.c einige abs() durch fabs() ersetzt
InteractiveUtils.c
MapSupport.c einige abs() durch fabs() ersetzt
EditGui.c alles so gelassen
EdSetGUI.c alles so gelassen


-> Bilder unterscheiden sich immer noch stark in den Wolken.
Zufallsfubktionen checken:
   srand48()  <- Die wird oft in WCS benutzt. Werte ausgeben, vergleichen.
   seed48()
   lcong48()

Die ersten 500 hseed-Aufrufe in MapTopoObject.c sind identisch.
dir ist auch identisch.

Bilder berechnet ohne Wolken, Wasser, Reflecionen, Fractal-Tiefe=0 -> Bilder unterscheiden sich zwischen den Versionen.
Mit gleicher Version 2 Bilder berechnet -> Bilder sind gleich, also kein zufaelliges Dithern.


---> Ursache gefunden. Die Noisemap wird unterschiedlich initialisiert.
GaussRand() liefert unterschiedliche Ergebnisee bei SAS/C und gcc.
Das liegt daran, dass drand48() nur nach dem ersten Aufruf von srand48() gleiche Resultate liefert. Bei weiteren Aufrufen unterscheiden sich die Ergebnisse.

26.07.2022
Hier wurde mal ein Fehler in der Mathe-Library von Lattice C gesucht...
https://obligement-free-fr.translate.goog/articles/c_correction_bogue_drand48.php?_x_tr_sch=http&_x_tr_sl=fr&_x_tr_tl=de&_x_tr_hl=de&_x_tr_pto=sc

Ein Besipielquelltext fue drand48() ist in af_drand48_test2.c. Die Ergebnisse entsprechen der gcc Routine, sowohl auf gcc als auch auf SAS/C.
erand48(seed) funktioniert mit SAS/C richtig, wie unter gcc.

26.07.2022
----------
Eigene srand48/drand48 Funktionen. Drand47() addiert beim SAS/C die letzen beiden Shorts des Buffers und haben damit ein anderes letzts Short im Puffer. Damit ist drand48() gleich.
In Globemap.c wird ein 64k Array von Gauss-Werten erzeugt. Damit wird die float-Berechnung nach UBYTE gecastet. Bei negativen Werten kommt beim SAS/C 0 raus, beim gcc nicht. Entsprechende if()-Aenderung eingebaut.
--> Bild ohne Wolken, Sonne, Mond und Fraktaltife=0 hat jetzt identischen Himmel in der SAS/C und der gcc-Version.
Wasser und Berge unterscheiden sich aber noch.

27.07.2022
----------
Bilder vergleichen:
compare -fuzz 1% -channel blue -compose src ~/Desktop/CanyonSet_gcc_no_000 ~/Desktop/CanyonSet_sas_no_000 DiffImage
#channel und fuzz kann man weglassen.

28.7.2022
---------
Weitere Suche nach unterschieden. In colorblends.c gibt es Unterschiede bei
- liegt an -ffast-math. Ohne das fast-math probieren.


Test-File geschrieben. af_fast_math_test.c -> Zeigt das Problem.

CC[0].Red ist 200
sunshade ist 0.95 bzw 0.9500000000000001

 CC[0].Red = PARC_SCOL_ECO(PAR_UNDER_ECO(i), 0) + redrand;   /* understory color */
  AF_DEBUG_hd("CC[0].Red",CC[0].Red);
  AF_DEBUG_f("sunshade",sunshade);
  AF_DEBUG_double_hex("sunshade hex:",sunshade);

  CC[0].Red -= sunshade * CC[0].Red;  // <- 9 bei gcc und fast-math, 10 (richtig) bei SAS/C

- ohne -ffast-math ist die Differenz der Bilder viel kleiner.
compare  -compose src ~/Desktop/CanyonSet_gcc_no_000 ~/Desktop/CanyonSet_sas_no_000 DiffImage

- Sonne/Mond mit Halo bringen keine Verschlechterung.

Anzeige der Anzahl unterschiedlicher (roter Pixel:)
compare  -compose src ~/Desktop/CanyonSet_gcc_no_000 ~/Desktop/CanyonSet_sas_no_000 DiffImage  -format %c histogram:info:

Anzeige der Unterschiede mit unterliegendem "Schattenbild"
compare  ~/Desktop/CanyonSet_gcc_no_000 ~/Desktop/CanyonSet_sas_no_000 DiffImage2

29.07.2022
----------
gcc und SAS/C Bilder (FractalDepth=0, No 3dClouds, No reflection, no Shadow, no waves) sind jetzt identisch.
Reflektionen sind auch identisch.
Zusaetzlich 3D-Clouds:
-> 3d-Clouds sind ziemlich identisch, haben aber viele unterschiedliche Punkte die auch noch wie Linien aussehen.

->renderclouds() Berechnet die Wolken, bringt sie auf den Bildschirm und in die fertige Datei. Ohne Renderclouds kommen auch die Clouds-Busy-Fenster und es dauert genauso lange. Aber eben keine Wolken zu sehen.


31.07.2022
----------
Die Logfiles sind inzwischen viel zu gross fuer meld-
Kopieren Startzeile bis Endzeile aus einem riesigen Fle:
sed -n '1415252,1415452 p' ~/Desktop/wcs_gcc.txt > wcs_gcc_1.txt

1.Augut 2022
------------
Bei den Files von Github was das Binary WCS 2.031 dabei.
Wenn ich auf den damaligen Stand zuruckgehe und es nachbaue, dann erzeugt das mit SCS/C nachgebaute WCS identische Bilder.
Das WCS 204 erzeugt zumindest andere Wolken.

4.Aug.2022
----------
Unterschiede in Z-Buffer Files. Sind sehr klein.
meld <(xxd ~/Desktop/CanyonSet_gcc_000GZB_gray_array) <(xxd ~/Desktop/CanyonSet_sas_000GZB_gray_array)

5.August 2022
-------------
Git-Hash als BUILDID hinzugefuegt bei den Eclipse Compilersettings. "G/" fuer gcc-Version.  -DBUILDID=\"g/'$(shell git describe --always --dirty)'\" 

Bei SAS manuell ENVARC-Variable setzen.
# unter Linux:
git describe --always --dirty  # zeigt z.B. a048f82-dirty
; in WinUAE:
setenv BuildID s/a048f82-dirty  ; s fuer SAS/C
copy env:BuildID  envarc:
delete version.o 
smake optimize


6.August 2022
-------------
Reworked build-targets. New Targets are 68040, 68020-60, Coverage, Profiling.
Latest toolchain version. (31.Juli 2022) -> Profiling does not work, no gmon.out file produced.
Makefiles need to be added to git. A simple "make all" should build the targets from the command line.
--> in den Buildverzeichnissen (68040, 68020-60, Coverage etc)
alles loeschen. Ueberflüssige Verzeichnisse und Dateien Loeschen. Mit Eclipse "Clean Project", dann "Build Project" um die Makefiles neu zu erzeugen. 
Makefiles einchecken:
git add -f makefile sources.mk objects.mk subdir.mk vgl/subdir.mk

8.8.2022
--------
Version ac5b912789dbc5431a83ee86618feeb13b231bfe als Testversion verschickt. Ohne fast-math!

gcc vom 31Jul22

WCS       Size     text    data     bss    dec     hex    Warnings   A4000T/040/25/16     Comment
   30   1034100   930076  102628  114580 1147284  118194    151           3:57:40          schnellstes (68040-Version)
                                                                          4:39:35          68020-60

30 m68k-amigaos-gcc -DFORCE_MUIMASTER_VMIN=19 -DAMIGA_GUI -DTOOLCHAIN_VER=\"'%@@=492:?ib`yF=aa 9EEADi^^8:E9F3]4@>^3633@^2>:82\\844]8:E 2>:82\\844i5_d`_cd 2>:82\\?6E:?4=F56i_b6ch77 2C@D\\DEF77ig4f`2gb 3:?FE:=Di5eba4c_be 4=:3ai6e4_4d_ 75aAC28>2i6hg7_h_ 75aD75i35f6e55 844i5657`c_ab :C2i333f2ge :I6>F=i7_55hhf =:3563F8ica5h5b2 =:3?:Ii26hgafb =:3$s{`aibac7e34 ?6H=:3\\4J8H:?idfa2c3c $sxic2f7d7e D754if34`42g G2D>i26b62a4 G344iaffgb62 G=:?<ia7`6_3a'\" 
   -I"/home/developer/Desktop/SelcoGit/3DNature/Amiga" -Os -Wall -c -fmessage-length=0 -funsigned-char -MMD -MP -MF"AGUI.d" -MT"AGUI.o" -o "AGUI.o" "../AGUI.c" 
   -DBUILDID=\"g/'ac5b912'\" -noixemul -m68040 -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static -mregparm -Winline -DSWMEM_FAST_INLINE -g -flto

30 m68k-amigaos-gcc  -o "WCS"  ./vgl/color.o ./vgl/dumb.o ./vgl/dumbpoly.o ./vgl/pixmap.o ./vgl/wuline.o  ./AGUI.o ./BitMaps.o ./Cloud.o ./CloudGUI.o 
   ./ColorBlends.o ./Commands.o ./DEM.o ./DEMGUI.o ./DEMObject.o ./DLG.o ./DataBase.o ./DataOps.o ./DataOpsGUI.o ./DefaultParams.o ./DiagnosticGUI.o 
   ./DispatchGUI.o ./EdDBaseGUI.o ./EdEcoGUI.o ./EdMoGUI.o ./EdPar.o ./EdSetExtrasGUI.o ./EdSetGUI.o ./EditGui.o ./EvenMoreGUI.o ./Foliage.o ./FoliageGUI.o 
   ./Fractal.o ./GenericParams.o ./GenericTLGUI.o ./GlobeMap.o ./GlobeMapSupport.o ./GrammarTable.o ./HelpGUI.o ./HyperKhorner4M-1_gcc.o ./Images.o ./InteractiveDraw.o 
   ./InteractiveUtils.o ./InteractiveView.o ./LWSupport.o ./LineSupport.o ./MUIFloatInt.o ./MakeFaces.o ./Map.o ./MapExtra.o ./MapGUI.o ./MapLineObject.o ./MapSupport.o 
   ./MapTopo.o ./MapTopoObject.o ./MapUtil.o ./Memory.o ./Menu.o ./MoreGUI.o ./Params.o ./ParamsGUI.o ./PlotGUI.o ./RequesterGUI.o ./RexxSupport.o ./ScratchPad.o ./ScreenModeGUI.o 
   ./Support.o ./TLSupportGUI.o ./TimeLinesGUI.o ./Tree.o ./Version.o ./VocabTable.o ./WCS.o ./Wave.o ./WaveGUI.o ./nncrunch.o ./nngridr.o ./sasc_functions.o    
   -noixemul -m68040 -fomit-frame-pointer -fbaserel -lm -lmui -DSTATIC_FCN=static -DSTATIC_VAR=static -mregparm -Winline -DSWMEM_FAST_INLINE -g -ldebug  -Wl,-Map=WCS_68040.map -flto

Die 68040 noch einmal mit ffast-math compiliert. -> 3:49:50, also 8 Minuten schneller. (Aber das Ergebnis hatte ja mehr Unterschiede)

14.8.2022
---------
Neues Target 68060.
68020-60, 68040 und 68060 als Testversion fuer A1K gedacht.

29.8.2022
---------
Added Check for 68020/68881 to linker-Settings. (Call check_wcs.sh)  -> Was not possible as post-build-step, as errors are ignored there!???

Absturz, wenn "rexxsyslib.library failed to load"

-> Priority von __cpucheck.c muss die hoechste sein! Hat sich wohl verschoben, Pullrequest gestellt.

1.Sep.2022
----------
Nur die 68040 Version funktioniert richtig. 68020, 68020-60 und 68060 erzeugen falsche Bilder. (Compiler vom 31.7.22)
Compiler 27.11.2021 OK. (Andere Fehler, Icons sehen komplett falsch aus)
15May22    falsch. z.B. Berge sehr seltsam
20Apr22    falsch. z.B. Berge sehr seltsam
13Feb22    falsch. z.B. Berge sehr seltsam


2.Sep.2022
Wenn Toolchain nicht mit meinem Script gebaut:

touch ~/opt/m68k-amigaos/m68k-amigaos/bin/m68k-amigaos-toolchain_hashes.sh
chmod +x ~/opt/m68k-amigaos/m68k-amigaos/bin/m68k-amigaos-toolchain_hashes.sh

Problem hat nichst mit -flto zu tun.

-> Wenn ich *ohne* -m68881 linke, dann ist der Fehler weg.
Mit -m68881 wird beim Linken
LOAD /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a
gemacht (mapfile)
ohne -m68881
LOAD /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libgcc.a


5.Sep.2022
----------
Libraryfiles testweise ersetzen:
mkdir lib-files
cd lib-files/
cp /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libgcc.a .
m68k-amigaos-ar xv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libgcc.a
cp /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a  libb_libm020_libm881_libgcc.a
ls *.o

_absvdi2.o   _clear_cache.o  _ctzdi2.o  enable-execute-stack.o  _fixunsdfsi.o  _floatdisf.o    gmon.o      _mulvdi3.o    _popcountdi2.o   _subvsi3.o      unwind-dw2-fde.o          xfpgnulib__floatsisf.o
_absvsi2.o   _clrsbdi2.o     _ctzsi2.o  _eprintf.o              _fixunssfdi.o  _floatditf.o    _lshrdi3.o  _mulvsi3.o    _popcountsi2.o   _trampoline.o   unwind-dw2.o              xfpgnulib__floatunsidf.o
_addvdi3.o   _clrsbsi2.o     _divdc3.o  _ffsdi2.o               _fixunssfsi.o  _floatdixf.o    __main.o    _mulxc3.o     _popcount_tab.o  _ucmpdi2.o      unwind-sjlj.o             xfpgnulib__floatunsisf.o
_addvsi3.o   _clzdi2.o       _divdi3.o  _ffssi2.o               _fixunstfdi.o  _floatundidf.o  _moddi3.o   _negdi2.o     _powidf2.o       _udivdi3.o      xfpgnulib__cmpxf2.o       xfpgnulib.o
_ashldi3.o   _clz.o          _divsc3.o  _fixdfdi.o              _fixunsxfdi.o  _floatundisf.o  _muldc3.o   _negvdi2.o    _powisf2.o       _udivmoddi4.o   xfpgnulib__extendsfdf2.o  xfpgnulib__truncdfsf2.o
_ashrdi3.o   _clzsi2.o       _divtc3.o  _fixsfdi.o              _fixunsxfsi.o  _floatunditf.o  _muldi3.o   _negvsi2.o    _powitf2.o       _udiv_w_sdiv.o  xfpgnulib__fixdfsi.o      xfpgnulib__unorddf2.o
_bswapdi2.o  _cmpdi2.o       _divxc3.o  _fixtfdi.o              _fixxfdi.o     _floatundixf.o  _mulsc3.o   _paritydi2.o  _powixf2.o       _umoddi3.o      xfpgnulib__fixsfsi.o      xfpgnulib__unordsf2.o
_bswapsi2.o  _ctors.o        emutls.o   _fixunsdfdi.o           _floatdidf.o   __gcc_bcmp.o    _multc3.o   _paritysi2.o  _subvdi3.o       unwind-c.o      xfpgnulib__floatsidf.o

m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _absvdi2.o _absvsi2.o  _addvdi3.o _addvsi3.o _ashldi3.o _ashrdi3.o _bswapdi2.o _bswapsi2.o
-> Fehler noch da

m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _clear_cache.o _clrsbdi2.o _clrsbsi2.o _clzdi2.o _clz.o _clzsi2.o _cmpdi2.o _ctors.o
-> Fehler noch da

m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a enable-execute-stack.o _eprintf.o _ffsdi2.o _ffssi2.o _fixdfdi.o _fixsfdi.o _fixtfdi.o _fixunsdfdi.o
-> Fehler noch da

m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _fixunsdfsi.o _fixunssfdi.o _fixunssfsi.o _fixunstfdi.o _fixunsxfdi.o _fixunsxfsi.o _fixxfdi.o _floatdidf.o
-> Fehler noch da

m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _floatdisf.o _floatditf.o _floatdixf.o _floatundidf.o _floatundisf.o _floatunditf.o _floatundixf.o __gcc_bcmp.o
-> Crash

m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a gmon.o _lshrdi3.o __main.o _moddi3.o _muldc3.o _muldi3.o _mulsc3.o _multc3.o
-> Crash

m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _mulvdi3.o _mulvsi3.o _mulxc3.o _negdi2.o _negvdi2.o _negvsi2.o _paritydi2.o _paritysi2.o
-> Crash.

Mit frischem Archiv:
-> kein Crash, Fehler wieder da -> wie erwartet.

m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a xfpgnulib__unordsf2.o xfpgnulib__unorddf2.o xfpgnulib__truncdfsf2.o xfpgnulib.o xfpgnulib__floatunsisf.o xfpgnulib__floatunsidf.o xfpgnulib__floatsisf.o xfpgnulib__floatsidf.o xfpgnulib__fixsfsi.o xfpgnulib__fixdfsi.o xfpgnulib__extendsfdf2.o xfpgnulib__cmpxf2.o unwind-sjlj.o unwind-dw2.o unwind-dw2-fde.o
-> Fehler noch da

m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a unwind-c.o _umoddi3.o _udiv_w_sdiv.o _udivmoddi4.o _udivdi3.o _ucmpdi2.o _trampoline.o _subvsi3.o
-> Fehler noch da

m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _subvdi3.o _powixf2.o _powitf2.o _powisf2.o _powidf2.o _popcount_tab.o _popcountsi2.o _popcountdi2.o
-> Fehler noch da

m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _paritysi2.o _paritydi2.o _negvsi2.o _negvdi2.o _negdi2.o _mulxc3.o _mulvsi3.o _mulvdi3.o
-> Fehler noch da

m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _multc3.o _mulsc3.o _muldi3.o _muldc3.o _moddi3.o __main.o _lshrdi3.o gmon.o
-> Fehler noch da

m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _floatdisf.o
-> Fehler noch da

m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _floatditf.o
r - _floatditf.o
-> Fehler noch da

m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _floatdixf.o
r - _floatdixf.o
-> Fehler noch da

m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _floatundidf.o
r - _floatundidf.o
-> FEHLER WEG! Geht wieder!  -> _floatundidf.o hat Probleme!??? "<Convert a 64bit unsigned integer to IEEE double"

Mit frischem Archiv:
-> Fehler wieder da -> wie erwartet.

Nochmal nur _floatundidf.o ersetzen:
m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _floatundidf.o
->Jetzt hängt er wieder.

OK, also mehr ersetzen: ???


m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _floatundidf.o _floatundisf.o _floatunditf.o _floatundixf.o __gcc_bcmp.o gmon.o _lshrdi3.o __main.o _moddi3.o _muldc3.o _muldi3.o _mulsc3.o _multc3.o _mulvdi3.o _mulvsi3.o _mulxc3.o _negdi2.o _negvdi2.o _negvsi2.o _paritydi2.o _paritysi2.o
-> haengt noch

m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _popcountdi2.o _popcountsi2.o _popcount_tab.o _powidf2.o _powisf2.o _powitf2.o _powixf2.o _subvdi3.o
-> haengt noch

m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _subvsi3.o _trampoline.o _ucmpdi2.o _udivdi3.o _udivmoddi4.o _udiv_w_sdiv.o _umoddi3.o unwind-c.o
-> hangt noch

m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a unwind-dw2-fde.o unwind-dw2.o unwind-sjlj.o xfpgnulib__cmpxf2.o xfpgnulib__extendsfdf2.o xfpgnulib__fixdfsi.o xfpgnulib__fixsfsi.o xfpgnulib__floatsidf.o
-> haengt noch

m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a xfpgnulib__floatsisf.o xfpgnulib__floatunsidf.o xfpgnulib__floatunsisf.o xfpgnulib.o xfpgnulib__truncdfsf2.o xfpgnulib__unorddf2.o xfpgnulib__unordsf2.o
-> Jetzt haben wir wieder die ganze 2. Haelfte und es funktioniert wieder.

Mit frischem Archiv:
-> Fehler wieder da -> wie erwartet.
m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _floatundidf.o
m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a xfpgnulib__cmpxf2.o xfpgnulib__extendsfdf2.o xfpgnulib__fixdfsi.o xfpgnulib__fixsfsi.o xfpgnulib__floatsidf.o xfpgnulib__floatsisf.o xfpgnulib__floatunsidf.o xfpgnulib__floatunsisf.o xfpgnulib.o xfpgnulib__truncdfsf2.o xfpgnulib__unorddf2.o xfpgnulib__unordsf2.o
also _floatundidf.o und alle xfpgnulib*  -> geht.

Mit frischem Archiv:
-> Fehler wieder da -> wie erwartet.
m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _floatundidf.o
m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a  xfpgnulib.o
r - xfpgnulib.o
-> haengt

Mit frischem Archiv:
-> Fehler wieder da -> wie erwartet.
m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _floatundidf.o
m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a xfpgnulib__cmpxf2.o xfpgnulib__extendsfdf2.o xfpgnulib__fixdfsi.o xfpgnulib__fixsfsi.o xfpgnulib__floatsidf.o
erster Teil der xfpgnulib* > haengt.

Mit frischem Archiv:
-> Fehler wieder da -> wie erwartet.
m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _floatundidf.o
m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a xfpgnulib__floatsisf.o xfpgnulib__floatunsidf.o xfpgnulib__floatunsisf.o xfpgnulib.o xfpgnulib__truncdfsf2.o xfpgnulib__unorddf2.o xfpgnulib__unordsf2.o
nur 2. Teil der xfpgnulib* -> geht!

Mit frischem Archiv:
-> Fehler wieder da -> wie erwartet.
m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _floatundidf.o
m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a xfpgnulib__floatsisf.o xfpgnulib__floatunsidf.o xfpgnulib__floatunsisf.o
- geht. Also wenigstens einen von xfpgnulib__floatsisf.o xfpgnulib__floatunsidf.o xfpgnulib__floatunsisf.o brauchen wir.

Mit frischem Archiv:
-> Fehler wieder da -> wie erwartet.
m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a _floatundidf.o
m68k-amigaos-ar rv /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a xfpgnulib__floatunsidf.o
--> JA; Damit geht es wieder!

rw-r--r-- 0/0    188 Jan  1 00:00 1978 xfpgnulib__floatunsidf.o     OK /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libgcc.a
rw-r--r-- 0/0    180 Jan  1 00:00 1978 xfpgnulib__floatunsidf.o    BAD /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a

rw-r--r-- 0/0    236 Jan  1 00:00 1978 _floatundidf.o               OK /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libgcc.a 
rw-r--r-- 0/0    144 Jan  1 00:00 1978 _floatundidf.o              BAD /home/developer/opt/m68k-amigaos_31Jul22/lib/gcc/m68k-amigaos/6.5.0b/libb/libm020/libm881/libgcc.a

16.9.2022
---------
-> Problem scheint mit Toolchain vom 15.Sep.22 geloest zu sein. Berge sehen in allen Varianten gut aus!
-> CPU.Check ist noch nicht behoben (68060 gar nicht, 20 und 20-60 falsche Prio)

WinUAE:
schnellste Variante ist 68020-60 und CPU auf 68020 stellen

18.10.2022
----------
CPU/FPU-Check in Bebbos Toolchain:

Prio 79 und 80 tauschen. cpucheck soll hoechste Priorität haben. Pullrequest am 31.Aug22 gestellt.
sources/nix/misc/__cpucheck.c
sources/nix/misc/__initlibraries.c

CPU-Checkcode fehlt, wenn mit -m68060 copiliert wird.

sources/nix/misc/__cpucheck.c
#if defined(mc68020) || defined(mc68030) || defined(mc68040) || defined(mc68060) || defined(mc68080)
Das Define mc68020 ist NICHT gesetzt, wenn fuer eine hoehere CPU comüpiliert wird. Deshalb || defined(mc68030) || defined(mc68040) || defined(mc68060) || defined(mc68080)
Pullrequest am 17.Oct22 gestellt.

cd ~/amiga-gcc
make clean-libnix
make libnix PREFIX=/home/developer/opt/m68k-amigaos_02Oct22

2.Nov.22
--------
* CPU/FPU-Check works again with Bebbos's toolchain from 31.Oct.22
* Profiling works again with Bebbos's toolchain from 1.Nov.22

22.Nov.22
---------
#Aminet preparations:

# TODO: Testen, dass nicht "dirty" im Hash steht.
# TODO: Testen, dass nicht "beta" in $VER steht.

ARCHIVE=wcs.lha; 
\rm -rf temp temp_aminet_upload; 
mkdir temp temp_aminet_upload; 
for CPU in 68020 68020-60 68040 68060; do cp -f $CPU/WCS_$CPU $CPU/WCS_$CPU.info temp/; done; 
cd temp; 
jlha a ../temp_aminet_upload/$ARCHIVE *; 
cd ..; 
cp CanyonSet000.jpg wcs.readme temp_aminet_upload

ftp -p   # -p passive mode, falls ftp durch NAT
open main.aminet.net
Name (main.aminet.net:developer): anonymous
Password: selco@t-online.de
cd new
pwd
put wcs.lha
put wcs.readme    # keine Verzeichnisse! also nicht z.B. amiga/espeak.readme!
put CanyonSet000.jpg
quit

Anzeige der max Zeilenlänge für Aminet-readme-File in vim (78 Zeichen/Zeile erlaubt):
:set colorcolumn=79

Kontrolle:

function red_msg() {
    echo -e "\\033[31;1m${@}\033[0m"
}

function green_msg() {
    echo -e "\\033[32;1m${@}\033[0m"
}

awk 'BEGIN{ret=0} END{exit ret} {if(length > 78){ret=1; printf("%s  --> (%u Zeichen)\n",$0,length)}}' wcs.readme \ 
&& green_msg "\nOK\n" || red_msg "\nERROR: Lines too long\n"

#Test auf Zeilenende-Zeichen:
cat -v wcs.readme | grep "\^M"; if [ $? -eq 0 ]; then red_msg "0a gefunden"; false; else green_msg "OK"; true; fi

25.Nov.22
---------
Sichern des Aminet-Uploads:

cd ~/Desktop/SelcoGit/wcs_aminet_upoloads/Emerald-Anton   # Git-Verzeichnis
for DIR in $(find /home/developer/Desktop/SelcoGit/3DNature/Amiga -type d -name "680*"); do 
   FILEBASE=WCS_$(basename $DIR); 
   cp -v $DIR/$FILEBASE .; 
   cp -v $DIR/$FILEBASE.info .; 
   cp -v $DIR/$FILEBASE.unstripped .; 
   done; 
cp -v /home/developer/Desktop/SelcoGit/3DNature/Amiga/CanyonSet000.jpg .

#readlink ~/amiga_gcc_link in readme-File nicht vergessen

git add *


28.Now.22
---------
Taggen der Aminet-Versionen:

m68k-amigaos-strings 68020/WCS_68020 | grep "Serial: "  # Anzeige des Git-Hashes (008e576)
git tag -a Emerald-Anton 008e576 -m "uploaded to Aminet 25.Nov.2022"

git show Emerald-Anton
git log

git push origin Emerald-Anton
git show origin/master Emerald-Anton
git log origin/master


29.Now.22
---------
Packen/Testen des Aminet-Archivs in sh-File aminet_archive.sh gepackt.


Aminet-Upload Emerald-Anton am 25.Now.2022


08.Dez.2022
-----------
-O2 fuer 68020  (war versehentlich -O0)
-i386-aros begonnen. Einige Quellen compilierbar
-#include <graphics/display.h> feht beim aros-gcc?
- SHORT und USHORT gibt es beim aros-gcc auch nicht. 

9.Dez.2022
----------
* Icaros Dektop VirtualBox Linux Hosted installiert in /home/developer/Desktop/IcarosDesktop  -> Da nehme ich die Include-Files her. (https://vmwaros.blogspot.com/p/download.html # Current version: 2.3)
* gcc nach /usr/local/amiga installiert. (download AROS_GCC_v6.5.0.tar.xz (169MB) was created from snapshot AROS-20190416-source.tar.bz2, uses GCC 6.5.0) from http://cshandley.co.uk/crosscompilers/
* neues Target i386-aros fuer Eclipse
* Buildsteps angepasst, also z.B. kein CPU/FPU-Check bei AROS
* Code jetzt bis auf das Assembler-File compilierbar.
* Alte Flags obsolete Flags ACTIVATE | SMART_REFRESH | WINDOWDEPTH usw. durch WFLG_ACTIVATE | WFLG_SMART_REFRESH | WFLG_DEPTHGADGET ersetzt -> noetig fuer AROS, OK fuer 68k
* Genauso Flags wir VANILLAKEY -> IDCMP_VANILLAKEY
* #include <graphics/display.h> entfernt. Fehlt bei AROS, nicht noetig bei 68k.
* IA_Width usw umbenannt in -> ia_width, sonst Namenskonflikt in AROS mit imageclass.h
* einige extra Klammern gegen Warnungen "Suggest Paranthes...)
* Testweise struct RxsLib *RexxSysBase; in RexxSupport.c eingefuegt, sonst nicht linkbar. Muss wieder raus!
* ScratchPad.c -> HK4M(regs) mit ifdef __AROS__, ist Assembler, muss nach C konvertiert werden.
* Dementsprechend HyperKhorner4M-1_gcc.asm aus der Buildconfiguration fuer AROS rausgenommen.
* seterrno(); in WCS.c wegdefiniert. Gibt es das bei AROS nicht?
* in WCS.h eingefuert mit ifdef __AROS__   #include <proto/muimaster.h> und #include <libraries/mui.h> 
* In vgl_internals.h typedef unsigned int size_t auskommentiert. Konflikt mit AROS
* WCS ist damit fuer AROS i386 compilierbar/linkbar. Starseite erscheint, Bilder nicht OK, kein Titelmenu?, stuerzt dann spaeter ab.

* i386-aros-strip mach das Executable kaputt. Die Exe, die vorher einigermassen ging, stuerzt dann sofort beim Start ab.
* Images mussten endian-gedreht werden. In WCS.c eine Funktion FlipImageWords() eingebaut und fuer alle Images aufgerufen. Die Bildchen und Buttons sind damit richtig.
* Das Programm hat unter AROS kein Menu???
  -> MUIA_Application_Menu wird anscheinen nicht von AROS unterstuetzt (ist auch bei MUI als obsolete gekennzeichnet) Man muss statt 
     "MUIA_Application_Menu			, WCSNewMenus," 
     wohl
     "MUIA_Application_Menustrip,  MUI_MakeObject(MUIO_MenustripNM,WCSNewMenus,0)," nehmen. 
     -> Damit ist das Menu jetzt da. Erst mal pit ifdef __AROS__ in AGUI.c gemacht, um 68k Version nicht zugefaehrden.

* Die Seriennummer sieht seltsam aus. Da steht Emerald-Anton mit drin!? (Wir sind jetzt Berta und in der Seriennummer sollte das garnicht drin sein)
  -> liegt an "git describe". Da wird das letzte Tag mit ausgegeben und die Anzahl der Commits danach. --exclude "*" beseitigt das. In Eclipse korrigiert.

10.Dec.2022
-----------
* Endian correction in EdPar.c Funktion short loadparams(USHORT loadcode, short loaditem)
  -> Projectfile colodemo.proj kann geladen werden. Das zugehoerige Parameterfile Demo1.par schlaegt fehl.

12.Dec.2022
-----------
* Packed eingebaut. Struktur war zu gross bei Aros i386 
  -> struct __attribute__((__packed__))ParHeader in WCS.h
* Endian-Korrektur in Database.c
* Programm stuerz beim Rendern oder Motion Editor -> CamView ab.
* Compilieren mit -fpack-struct fuehrt zum sofortigen Crash von WCS

13.Dec.2022
-----------
* Fixed Nullptr-Zugriff in CloudGUI.c GUICloud_SetGads() wenn Neuer Start ohne Projekt laden, Parameter Module -> Clouds
  -> CloudData_GetShort(CD, CLOUDDATA_NUMWAVES)
* DEM.c readDEM() muss endian korrigiert werden.

15.Dec.2002
-----------
* Fixed Nullptr-Zugriff in MapSupport.c void latlon_XY(long i) wenn Neuer Start ohne Projekt laden, Parameter Module -> Map view, Databse File Loader -> Cancel
* Viele KPrintF() engebaut. Starten mit "AF: ", damit man spaeter die eigenen Zeilen aus dem Log rausfiltern kann. (AROS/Zune  schreibt viele eigene Ausgaben)
* Icaros starten mit ./Arch/linux/AROSBootstrap &> aaa.txt; Dann hat man ein Textfile mit den KPrintFs, auch wenn es einen Crash gibt-
* Ausgaben mit WinUAE vergleichen.
* z.B. in EdPar.c wird der SPiecher im Fehlerfall nicht wieder komplett freigegeben. if GetMemory() || GetMemory() || .. goto error
  in Dlg.c und MapToptObject auch


#../EvenMoreGUI.c:36:	get_Memory(sizeof (struct TimeSetWindow), MEMF_CLEAR)) == NULL)
#../ScreenModeGUI.c:38:     if((ThisMode = get_Memory(sizeof(struct WCSScreenMode), MEMF_CLEAR)))
#../GenericTLGUI.c:393:	get_Memory(sizeof (struct TimeLineWindow), MEMF_CLEAR)) == NULL)
#../MapExtra.c:2881: return ((struct Branch *)get_Memory(sizeof (struct Branch), MEMF_CLEAR));
#../DiagnosticGUI.c:15:	get_Memory(sizeof (struct DiagnosticWindow), MEMF_CLEAR)) == NULL)
#../DEMGUI.c:1772: if ((SaveMap = (short *)get_Memory(SaveSize, MEMF_ANY)) != NULL)
#../EdEcoGUI.c:55: EE_Win->ECList = (char **)get_Memory(EE_Win->ECListSize, MEMF_CLEAR);
#../DataBase.c:1142:    if ((DLItem->Next = get_Memory(sizeof (struct DirList), MEMF_CLEAR)) != NULL)
#../EdMoGUI.c:2041:	get_Memory(sizeof (struct ParListWindow), MEMF_CLEAR)) == NULL)
#../nncrunch.c:1317:   if ((NNG->Grid = (float *)get_Memory(NNG->GridSize, MEMF_ANY)) != NULL)
#../nngridr.c:221: return ((struct NNGrid *)get_Memory(sizeof (struct NNGrid), MEMF_CLEAR));
#../DataOps.c:3160:   if ((RowZip = (long *)get_Memory(rows * sizeof (long), MEMF_ANY)))
#../Params.c:1620:	get_Memory((KT_MaxFrames + 1) * sizeof (double), MEMF_CLEAR)) == NULL)
#../Tree.c:1417: return ((struct BitmapImage *)get_Memory(sizeof (struct BitmapImage), MEMF_CLEAR));
#../DEM.c:218: map->map = (short *)get_Memory (map->size, MEMF_ANY);
#../MapSupport.c:457: if (((mapelmap = (struct elmapheaderV101 *)get_Memory(MapElmapSize, MEMF_CLEAR))
#../LineSupport.c:547:	get_Memory(sizeof (struct MotionWindow), MEMF_CLEAR)) == NULL)
#../EditGui.c:43:	get_Memory(sizeof (struct EcoPalWindow), MEMF_CLEAR)) == NULL)
#../AGUI.c:2060:	get_Memory(sizeof (struct StatusLogWindow), MEMF_CLEAR)) == NULL)
#../EdDBaseGUI.c:1629: if ((DL_Win->DLName = (char **)get_Memory(DL_Win->DLNameSize, MEMF_CLEAR)) == NULL)
#../CloudGUI.c:32:	get_Memory(sizeof (struct CloudWindow), MEMF_CLEAR)) == NULL)
#../WaveGUI.c:46:	get_Memory(sizeof (struct WaveWindow), MEMF_CLEAR)) == NULL)
#../MapUtil.c:580: if ((Qavg = (float *)get_Memory(Qsize, MEMF_CLEAR)) == NULL)
#../EdSetGUI.c:91:	get_Memory(sizeof (struct SettingsWindow), MEMF_CLEAR)) == NULL)
#../Wave.c:159:	get_Memory(sizeof (struct WaveData), MEMF_CLEAR));

compiler ohne flto, -O0 -g, ohne -fomit-frame-ptr zum debuggen fuer AROS

17.Dec.22
---------
Save Screen stuezt ab, weil CD->Data ein Null-Ptr ist. Das liegt wahrscheinlich daran, dass das Programm in 24 Bit laeuft. WCS geht
von einem nicht-24bit Bildschirm aus. Auf dem Amiga nachtesten! -> crashed dirt auch!
* Bei RTG ist WCSScrn->RastPort.BitMap->Planes NULL. In dem Fall ReadPixelArray() Kopie der Zeile holen?

20.Dec.2022
-----------
* Ueberprueft: Die Farbtabelle wird richtig angespeichert. Trotzdem sind die Farben falsch bei SAve Screen.

21.Dec.2022
-----------
Die Fenster haben alle einen weißen oder hellgrauen Hintergrund. Damit sie die gleiche Farbe haben wie die Amiga-Version, muss unter
      WindowContents, VGroup,
ein
      MUIA_Background, MUII_BACKGROUND,  //ALEXANDER
eingefügt werden.

Dann sind die Buttons immer noch weiß.
In RequesterGUI.c

APTR KeyButtonFunc(char ControlChar, char *Contents)
{
return(KeyButtonObject(ControlChar), MUIA_Text_Contents, Contents,
        MUIA_Background  , MUII_BACKGROUND,  // Alexander
        End);
} /* KeyButtonFunc() */

und dann sehen auch die Buttons so aus wie unter AmigaOS.

22.12.2022
----------
Mit ReadPixelArray8() bekomme ich auf dem Amiga bei P96-Screens mit Non-Standard Bitmaps ein fehlerhaftes IFF-File. 
Auf Aros ist eine Farbe falsch (gelb)

25.12.2022
-----------
Lesen in DEM.c readDEM() ist OK. (Alles mit KPrintF Verglichen. 

29.12.2022
----------
Ich will die Groesse aller Strukturen ausgeben, um herauszufinden, ob ich noch irgendwo packed hinschreiben muss:

ctags ../WCS.h   # ezeugt eine Datei "tags".  Die ist Tab-separiert. Das 4. Wort zeigt den Typ: s=struct, u=union
cat tags  | awk -F'\t' '{ if ($4=="u") {printf(" KPrintF(\"AF: sizeof(union %s)=%%ld\\n\",sizeof(union %s));\n", $1,$1);} }'
cat tags  | awk -F'\t' '{ if ($4=="s") {printf(" KPrintF(\"AF: sizeof(struct %s)=%%ld\\n\",sizeof(struct %s));\n", $1,$1);} }'
#damit C_File bauen.


ifdef __AROS__
#define USHORT unsigned short
#define SHORT short
//#define EXTERN
#define __far
#define __chip
#endif

#include "WCS.h"

int main(void)
{
 KPrintF...
 retuen 0;
}

m68k-amigaos-gcc -I.. af_size.c -ldebug -o af_size_68k
i386-aros-gcc -I.. -I/usr/local/amiga/i386-aros/include/SDI/ af_size.c -ldebug -o  af_size_i386-aros
cp af_size_i386-aros ~/Desktop/aros_build/alt-abiv0-linux-i386-d/bin/linux-i386/AROS/Alexander/WCS_204/

mit Meld Diffs anschauen.

Raussuchen, wo in Strukturen riengelesen wird:

find .. -name "*.c" -exec grep -nH "read.*sizeof" {} \; | awk -F 'struct ' 'NF>1{ sub(/ .*/,"",$NF); print $NF }' | awk -F\) '{print $1}' | sort --unique

In meld nachschauen, ob es eine Struktur ist, deren Groesse sich unterscheidet.

30.12.2022
----------
Die unions 
KeyFrame
KeyFrameV1
NoLinearKeyFrame
haben unterschiedliche Groessen bei 68k und i386-aros

-> Alle Strukturen in den unions haben unterschiedliche Groessen -> pragma packed einfuegen

/*EXTERN*/ union KeyFrameV1 {
 struct MotionKey MoKey;          <-
 struct ColorKey CoKey;           <-
 struct EcosystemKeyV1 EcoKey;    <-
 struct EcosystemKey2V1 EcoKey2;  <-
};

/*EXTERN*/ union KeyFrame {
 struct MotionKey MoKey;          <-
 struct MotionKey2 MoKey2;        <-
 struct ColorKey CoKey;           <-
 struct EcosystemKey EcoKey;      <-
 struct EcosystemKey2 EcoKey2;    <-
 struct CloudKey CldKey;          <-
 struct WaveKey WvKey;            <-
};


2.Jan2023
---------
68k-Original
Der Farbverlaufsbalken im "Color Editor" funktioniert nicht in NON-Standard Screenmodes.

AROS:
Das Lesen und Endian-Korrigieren der Union KeyFrame in EdPar.c musste Spezialbehandlungen fuer die Faelle 
MotionKey, ColorKey und den Rest bekommen.

CanyonSunset kann gerendert werden. Farben sind OK. Wasserwellen fehlen, es ist spiegelglatt.

3.Jan23
-------
C-Ersatz fuer HyperKHorner.asm. Motion-Window Cam-View funktioniert damit komplett.

4.Jan23
--------
Water waves are correct now. In fact it was the reflection that killed the waves. The file WCSSlMap%1d.Temp was endian-corrected after reading but
written in host-byteorder. There is no need to endian-correct this file at all.

13.Jan23
---------
Beim Abspeichern darf in EdPar.c fwriteKeyFrames() nicht der endian-gedrehte Wert fuer das switch (KeyFrames[i].MoKey.Group) benutzt werden!

* Project Save As / Load new Project funktioniert jetzt.
* 'The Parameter File format has been changed slightly since this file was saved. Would you like to re-save it in the new format now?' Speichern und dann neue laden funktioniert jetzt.
(Beides mit CanyonSunset getestet)

Der Screenmode kann auch gespeichert werden und wird dann wieder geladen. Das ist ein ASCII-File, keine Endian-Probleme.

Lightwave Export:
* "Scene" Das exportierte LWS File von CanyonSunset stimmt fast mit der Amiga-Version ueberein. (ASCII-File) Scheinen Rundung/Genauigkeits-Unterschiede zu sein.
* "Scene + DEMs"  ???
* "DEM only" ??? Error loading DEM object ???
* "Motion only" stimmt fast mit der Amiga-Version ueberein. (ASCII-File) Scheinen Rundung/Genauigkeits-Unterschiede zu sein.

Bilder speichern, Render Settings, 2. Tab
* "IFF"    -> geht ja schon
* "Sculpt" -> CynyonSet000.blu .grn .red  -> OK, getestet 14.Jan23

# erst mal Breite und Hoehe des Bildes anzeigen
identify AROS_CanyonSet000.iff
   ilbmtoppm: warning - non-square pixels; to fix do a 'pnmscale -yscale 1.1'
   ilbmtoppm: input is a deep (24-bit) ILBM
   AROS_CanyonSet000.iff PPM 752x480 752x480+0+0 8-bit sRGB 1.03273MiB 0.000u 0:00.009

# zusammenfuegen. die red grn und blue Files sind einfach Arrays von 8Bit Werten. In der reihenfolge R G B angeben.
convert -depth 8 -size 752x480 gray:AROS_CanyonSet000.red  gray:AROS_CanyonSet000.grn gray:AROS_CanyonSet000.blu -combine rgb_combined.jpg

# anzeigen. OK.
display rgb_combined.jpg

- Auch getestet mit ADPRO 2.5.0 unter WinUAE mit dem SCULPT Loader 14.Jan23

* "Raw intrlvd RGB" -> CynyonSet000.RAW
    AROS_CynyonSet000.RAW  3Bytes grouping "xxd -g3 AROS_CanyonSet000.RAW | more"  

    .RAW 00000000: f59f3d f49d3b f39d3b f39c3b f39b3b f2  ..=..;..;..;..;.  jeweils RGB RGB RGB

    .red 00000000: f5f4f3 f3f3f2 f2f2f2 f2f2f1 f1f2f2 f2  ................  nur die Rot-Werte. Anfang passt mit .RAW zusammen
    .grn 00000000: 9f9d9d 9c9b9b 9a9a9b 9b9b99 999999 99  ................  nur die Gruen-Werte. Anfang passt mit .RAW zusammen
    .blu 00000000: 3d3b3b 3b3b3b 3a3a3a 3a3938 383838 38  =;;;;;::::988888  nur die Blau-Werte. Anfang passt mit .RAW zusammen

warum geht "display -depth 8 -size 752x480 rgb:CanyonSet000.RAW  &" dann nicht richtig?

#eigenes Programm zum Zerlegen des RAW-Files geschrieben.
./af_split_RAW_image AROS_CanyonSet000.RAW 752 480
convert -depth 8 -size 752x480 gray:AROS_CanyonSet000.RAW.red  gray:AROS_CanyonSet000.RAW.grn gray:AROS_CanyonSet000.RAW.blu -combine AROS_RAW_recombined.jpg
display AROS_RAW_recombined.jpg  # -> genauso falsch wie "display -depth 8 -size 752x480 rgb:CanyonSet000.RAW  &"
---> also Fehler im RAW-File
-> Behoben in BitMaps.c savebitmaps(), width*3 
Kann jetzt mit "display -depth 8 -size 752x480 rgb:AROS_CanyonSet000.RAW  &" angezeigt werden.

*Export Z-Buffer testen  (Alle haben default sehr aenhliche Namen)
 * Z As Floating Pt IFF     CanyonSet000ZB  -> falsch, Zeilen/Spalten werden in WCS-Amiga Convert DEM Format ZBuf falsch angezeigt.
 * Z As Gray Scale IFF      CanyonSet000ZB  OK.  unterschiedlich gross, unterschiedlich -> Linux "display" zeigt zwei praktisch gleiche Bilder. Beide keinen ColorMap. Kann auch mit ADPRO angezeigt werden. 14.Jan23
 * Z As Floating Pt Array   CanyonSet000FZB 4 Bytes/Pixel "display -endian MSB -depth 32 -size 752x480 -define quantum:format=floating-point -define quantum:scale=4e+03 gray:Amiga_CanyonSet000FZB"
                                            (quantum:scale=4e+03 soll die FloatWerte in den Bereich von 0...1 normieren durch Division)
                            - muss bei AROS noch Endian gedreht werden!
                            19.Jan23: Done! OK.
 * Z As Gray Scale Array    CanyonSet000GZB 4 Bytes/Pixel, gleich gross, kleine Unterschiede -> OK: "display -depth 8 -size 752x480 gray:AROS_CanyonSet000GZB_gray_array"

Der Himmel scheint unterschiedlich zu sein.

19.Jan23
--------
"Z As Floating Pt Array" fertig und getestet fuer AROS.
todo: "Z As Floating Pt IFF" fuer AROS

# Anzeige der Floating Pt IFF-Files:
# zeigt Offset der Chunks im IFF-File an
~/Desktop/SelcoGit/amiga_buch/sources/fuzzer_test/src/iff_structure_linux CanyonSet000ZB_fl.iff

Pos=12 (0xc)
Chunk name: ZBUF
ChunkLen: 36 (0x24)
---------------------------------
Pos=56 (0x38)
Chunk name: ZBOD
ChunkLen: 1443840 (0x160800)
---------------------------------

tail --bytes 1443840 CanyonSet000ZB_fl.iff | display -endian MSB -depth 32 -size 752x480 -define quantum:format=floating-point -define quantum:offset=100 -define quantum:scale=4e+03 gray:
# oder
identify CanyonSet000FZB  # fuer die Abmessungen
tail --bytes $((752*480*4)) CanyonSet000ZB | display -endian MSB -depth 32 -size 752x480 -define quantum:format=floating-point -define quantum:offset=100 -define quantum:scale=4e+03 gray:

* der ZBOD Chunk ist ok, Big-Endian
* der ZBUF Chunk muss endian.korrigiert werden. Neue Variante von iff_structure mit ZBUF Infos geschrieben. 
~/Desktop/SelcoGit/af_iff_structure/iff_structure_linux CanyonSet000ZB_fl.iff


~/Desktop/SelcoGit/af_iff_structure/iff_structure_linux CanyonSet000ZB_fl.iff 
Filesize is 1443904
FORM
FORM-ChunkLen: 1443888 (0x160830)
---------------------------------
Chunk name: ILBM
---------------------------------
Pos=12 (0xc)
Chunk name: ZBUF
ChunkLen: 36 (0x24)
  additional info available:
     Width:       752
     Height:      480
     VarType:     6
     Compression: 0
     Sorting:     0
     Units:       3            <km>
     Min:         1.461988
     Max:         144.360977
     Bkgrnd:      340282346638528859811704183484516925440.000000
     ScaleFactor: 1.000000
     ScaleBase    0.000000
---------------------------------
Pos=56 (0x38)
Chunk name: ZBOD
ChunkLen: 1443840 (0x160800)
---------------------------------

ZBuffer-Float IFF save ist jetzt OK. Kann vom Amiga in WCS DEM-Convert eingelesen werden.
AROS DEM-Convert kommt mit den Files noch nicht klar.

20.Jan2023
----------
FLoat Pt IFF ZBuffer-File kann jetzt im DEM-Converter eingelesen werden.

23.Jan.2023
-----------
Beta-Timeout (62 Tage) in WCS.c eingebaut. Defines in Version.h
Ablaufdatum wird in Beta-Versionen mit printf als "epoch" (Sekunden seit 1970) ausgegeben. (Bei AROS von der Shell starten)
Anzeige des Klartext-Datums mit
date -d@1674518400

2.Feb.23
--------
Added x86_64-aros als neue Konfiguration. Benutze deadw00ds toolchain/Aros von https://github.com/deadw00d/AROS/blob/master/INSTALL.md

7.Feb.2023
----------
* Fuer AROS copilieren zwei Scripte in ~/bin angelegt. Die enthalten den gcc-Aufruf mit --sysroot und -I fuer das SDI-Include-Dir.

# cat i386-aros-gcc

#!/bin/sh

#Wrapper fuer AROS-gcc, damit man nicht manuell sysroot uebergeben muss

exec /home/developer/Desktop/SelcoGit/aros_deadw00d_32bit/toolchain-alt-abiv0-i386/i386-aros-gcc --sysroot=/home/developer/Desktop/SelcoGit/aros_deadw00d_32bit/alt-abiv0-linux-i386-d/bin/linux-i386/AROS/Development -I/home/developer/Desktop/SelcoGit/aros_deadw00d_32bit/alt-abiv0-linux-i386-d/bin/linux-i386/AROS/Development/include/SDI/ "$@"

#cat x86_64-aros-gcc 

#!/bin/sh

#Wrapper fuer AROS-gcc, damit man nicht manuell sysroot uebergeben muss
exec /home/developer/Desktop/SelcoGit/aros_deadw00d_32bit/toolchain-alt-abiv0-i386/i386-aros-gcc --sysroot=/home/developer/Desktop/SelcoGit/aros_deadw00d_32bit/alt-abiv0-linux-i386-d/bin/linux-i386/AROS/Development -I/home/developer/Desktop/SelcoGit/aros_deadw00d/core-linux-x86_64-d/bin/linux-x86_64/gen/include/SDI "$@"

* Laut Deadwood muss Strip für AROS muss Strip extra Parameter bekommen.
Krzysztof Smiechowicz <deadwood@onet.pl> schrieb mir am Mo 06.02.2023 19:44
AROS executables are not full executables in ELF sense, they are more relocable objects. Due to this, default use of strip strips too much. Here is the command line which should be ok:

x86_64-aros-strip --strip-unneeded -R.comment


* Die 32bit-Versionen von WCS (unstripped unf stripped) funktionieren unter 32Bit-Aros.
* Die 64bit-Versionen von WCS sind angeblich nicht ausfuehrbar!?

-> Laut Deadw00d soll ich -fno-common beim Compilieren mit angeben. -> Ja, dann geht es. Warum? app wird nur einmal in main anelegt. Wenn man app in WCS.c anlegt UND dabei auf NULL setzt, ist der Fehler weg und er meckert wegen dem naechsten globalen Ding. -> ich mache bei AROS -fno-common und lassen den Quellcode unveraendert.

-> 32 Bit Version funktioniert, 64 Bit Version stuerzt an, sobald man etwas mit MUI macht.
Krzysztof sagt:
BTW, if your software has never been compiled for 64-bit and it is a MUI software, you need to change your ULONG variables used for getting attributes to IPTRs. IPTR is "integer large enough to hold a pointer" and this is what AROS MUI expects (on both 32 and 64 platforms. On 32-bit IPTR = ULONG, on 64-bit IPTR = UQUAD)

Also bei den set() Funktionen nicht auf (ULONG) casen sonder auf (IPTR)

Das ist ein Job fuer sed
#im Source-Verzeichnis
sed -i -E "s/(set.+)ULONG(.+)/\1IPTR\2/" *.c

9.Feb23
-------
Alle Strukturen, die mit fread oder read geladen werden, muessen LONG/ULONG statt long/unsigned long verwenden, weil long bei AROS64 8 Bytes gross ist! Auch beim Einlesen beachten, dass die Zielstruktur/Array dann vom Typ LONG sein muessen.
--> AROS64 kann jetzt alle 3 Demo-Projekte rendern. Problem: MUI reagiert nicht auf Mausklicks oder stuerzt ab, z.B. Cancel-Button beim Rendern oder EingabeZeilen bei Parametern.

15.Feb23
--------
Nochmal AROS Lightwave Export:

Lightwave Export:
* "Scene" Das exportierte LWS File von CanyonSunset stimmt fast mit der Amiga-Version ueberein. (ASCII-File) Scheinen Rundung/Genauigkeits-Unterschiede zu sein.
* "Scene + DEMs"  ???
* "DEM only" ??? Error loading DEM object ???
* "Motion only" stimmt fast mit der Amiga-Version ueberein. (ASCII-File) Scheinen Rundung/Genauigkeits-Unterschiede zu sein.


Frische Installation: Ein Verzeichnis Objects gibt es nicht!
* "Scene" 
  "Scene Path/File" "WCSProjects:" and "CanyonSunset.LWS"
  "LW DEM Object Path" "WCSProjects:/Objects"  Mit "/" im Original. Datei leer lassen.
  es entsteht: "WCSProjects:CanyonSunset.LWS"   (ASCII-File)

LWSC
1

FirstFrame 0
LastFrame 399
FrameStep 1

LoadObject Objects/WCSNull.LWO
ObjectMotion (unnamed)
9
1
0.000000 0.000000 0.000000 0.0 0.0 0.0 1.0 1.0 1.0
0 0 0.0 0.0 0.0
EndBehavior 1
ShadowOptions 7


* "Scene + DEMs"
  "Scene Path/File" "WCSProjects:" and "CanyonSunset.LWS"
  "LW DEM Object Path" "WCSProjects:/Objects"  Mit "/" im Original. Datei leer lassen.
  Neue Schublade WCSProjects:/Objects wird angelegt, also in Wirklichkeit wegen des "\" WCS:Objetcs.
  Es entsteht "WCSProjects:CanyonSunset.LWS" und WCSProjects:/Objects/ "36112.I   .LWO", "36112.M   .LWO" und "WCSNull.LWO". (also in Wirklichkeit wegen des "\" in WCS:)
AMIGA OK.
  
Das "WCSProjects:CanyonSunset.LWS" ist jetzt viel groesser und weitere "LoadObject Objects/36112.I   .LWO" und "LoadObject Objects/36112.M   .LWO"
Lightwave findes die LWO-Files nicht, weil LW in "Objects" sucht.

Ich hatte in meiner WCS-Version den "/" schon entfernt. Damit entsteht Objects jetzt IN "WCSProjects:" und liegt damit auf gleicher Hoehe wie "CanyonSunset.LWS" und die
"LoadObject Objects/36112.I   .LWO" Kommandos in CanyonSunset.LWS passen.
LW-Options Panel Content Directory auf WCSProjects: setzen.
Szene kann in LW geladen werden.
AMIGA OK.
AROS nur wenn WCSProjects:Objects schon vorhanden ist. Amiga-LW laedt die von AROS generierte Scene ewig (busy)
-> Amiga-LW laedt die von AROS generierte Scene jetzt. Die Lightwave-Ansicht ist aber nicht ganz identisch. Es gibt eine diagonale Kante in der Ansicht.
-> In beiden erueugten LWP-Files sind die letzten 17 Koordinaten unterschiedlich 
   ~/Desktop/SelcoGit/af_iff_structure/iff_structure_linux  '36112.M   .LWO' >'36112.M.LWO.decoded'
   ~/Desktop/SelcoGit/af_iff_structure/iff_structure_linux  '36112.M_Amiga.LWO' >36112.M_Amiga.LWO.decoded
   meld 36112.I_Amiga.LWO.decoded 36112.I.LWO.decoded
   -> In meiner 68020er Version ist die gleiche Kante? Nur im Original nicht? Nochmal mit SAS/C kompilieren!

* "DEM only" Es muss ein existierendes *.elev File angegeben werden. Daraus wird ein *.LWO File erzeugt.
* In LW geladen: https://ftp2.grandis.nu/turran/FTP/~Uploads/emu/Lightwave/Lightwave.3D_v5.0r-CDSetup/LW50r_Incl.CDContent.lha
AMIGA OK.

* "Motion only" Es entsteht "WCSProjects:CanyonSunset.LWM"
Amiga OK.

22.Feb.2023
-----------
Mal wieder SAS/C compilieren:
sc-Ordener vom Amiga geholt. User-Startup:
;---------------------------------------------------
;BEGIN SAS/C
assign netinclude: Work:AmiTCP-SDK-4.3/netinclude/
assign netlib: Work:AmiTCP-SDK-4.3/netlib
assign sc: Work:sc
assign lib: sc:lib
assign include: sc:include
assign cxxinclude: sc:cxxinclude
path sc:c add

assign include: Work:MUI/Developer/C/Include/ add
assign include: Work:SDI/includes/ add

;END SAS/C

setenv BUILDID=AF_BuildID  ; for WCS compiling
copy env:BUILDID envarc:   ; envarc:BUILDID is needed during smake. What is its purpose?

;---------------------------------------------------

http://aminet.net/dev/mui/mui38dev.lha installieren
http://aminet.net/dev/c/SDI_headers.lha installieren

* Amiga/WinUAE neu booten.
cd vgl
smake
cd /
smake all ; no optimization or
smake optimize

23.Feb.2023
-----------
Das WCSGST File macht bei der SAS/C Version Aerger. Da darf u.A. kein inline drinstehen. Das GST-File wird aus WCS.c gebaut. Welche includes benutzt WCS.c?
Die Includes sieht man beim gcc mit -H. Kann man mit 
gcc ....     -c -H 2>&1 | grep "^\.\+ " | grep -v "m68k-amigaos_08Nov22"
 (ein oder mehrere Punkte am Zeilenanfang, dann ein Leerzeichen. Blende die Systemheader aus.)
rausfiltern, also

m68k-amigaos-gcc -DFORCE_MUIMASTER_VMIN=19 -DAMIGA_GUI -DTOOLCHAIN_VER=\"'%@@=492:?i_g}@Gaa 9EEADi^^8:E9F3]4@>^3633@^2>:82\\844]8:E 2>:82\\844ig5d556a 2>:82\\?6E:?4=F56i_b6ch77 2C@D\\DEF77ig4f`2gb 3:?FE:=Di_d6df`3b5 4=:3ai6e4_4d_ 75aAC28>2i6hg7_h_ 75aD75i35f6e55 844i5524b4aa4 :C2i333f2ge :I6>F=i7_55hhf =:3$s{`aibac7e34 =:3563F8ica5h5b2 =:3?:Iib5d42b2 ?6H=:3\\4J8H:?i32be_6b D754i2f3ce_7 G2D>i26b62a4 G344iaffgb62 G=:?<ia7`6_3a'\" -I"/home/developer/Desktop/SelcoGit/3DNature/Amiga" -O2 -Wall  -fmessage-length=0 -funsigned-char -MMD -MP -MF"WCS.d" -MT"WCS.o"  "../WCS.c" -DBUILDID=\"g/'daa6ae6-dirty'\" -noixemul -m68020 -m68881 -fomit-frame-pointer -fbaserel -DSTATIC_FCN=static -DSTATIC_VAR=static -mregparm -Winline -DSWMEM_FAST_INLINE -c -H 2>&1 | grep "^\.\+ " | grep -v "m68k-amigaos_08Nov22"

Ohne die Greps zeigt er auch noch an, wo Include-Ifdefs hin sollten:

Multiple include guards may be useful for:
../CmdCallProtos.h
../GUIDefines.h
../GUIExtras.h
../Headers.h
../Proto.h
../Version.h
../WCS.h

--> Erledigt.

smake clean
sc MAKEGST=WCSGST WCS.c "IGNORE=51" "DEFINE=STATIC_FCN=static" "DEFINE=STATIC_VAR=static" "DEFINE=__BYTE_ORDER__=1" "DEFINE=__ORDER_BIG_ENDIAN__=1"
smake AGUI.o

Mit Emerald_Anton verglichen. Dort laesst sich alles mit smake bauen. Altes WCS.c vom Emerald_Anton genommen, Header-Files aktuell -> geht auch!
mit gitk die History vom WCS.c angeschaut und ann einzelne Versione mit git checkout geholt:
- WCS.c 22.12.2022 geht noch
        19.1.2023 geht noch
        24.1.2023 geht nicht mehr (AGUI.c kann nicht mehr compiliert werdenm wenn das WCSGST mit WCS.c neu gebaut wurde)

-> in WCS.c durfte kein #include <time.h> stehen. Sonst kann man AGUI.c nicht mehr kompilieren, wenn das WCSGST schon da ist (!!???)

Korrigiert, Kann wieder mit SAS/C gebaut werden.

27.Feb.23
---------
SAS/C-Version kann mit Vamos 0.7 gebaut werden. Ohne smake, aufruf aller Compiler/Linker-Kommandos. Folgende ~/.vamosrc benutzt. SC, MUI, SDI auf den Linux-Rechner kopiert.

[vamos]
#quiet=True

# 8Meg Ram for 68000er Computer
#ram_size=8192

# more RAM for bigger CPUs, disable HW access for so much RAM
cpu=68020
hw_access=disable
ram_size=32768

#16 KBytes of Stack
stack=16

[volumes]
# wb310=~/amiga/wb310
sc=~/Desktop/AmigaFiles/sc
C=~/Desktop/AmigaFiles/C
L=~/Desktop/AmigaFiles/L
MUI=~/Desktop/AmigaFiles/MUI
SDI=~/Desktop/AmigaFiles/SDI
Libs=~/Desktop/AmigaFiles/Libs

[assigns]
include=sc:include,MUI:Developer/C/Include,SDI:includes
lib=sc:lib
t=root:tmp
ENV=t:ENV

[path]
path=sc:c,L:
#,wb310:c

#wichtig, sonst meckert smake "Can't open version 0 of icon.library"
[icon.library]
mode=fake


##############################################

Script zum bauen mit SAS/C und vamos
build_wcs_sasc.sh

Script-Aufruf in die pre_commit Datei uebernommen. Nicht vergessen, die Datei nach .git/hooks zu kopieren!

#!/bin/bash
# rejects commits of UTF-8 text files. 
# copy this file to .git/hooks

FILE_LIST="$(git diff --cached --name-only)"

for FILE in $FILE_LIST; do
    if [ $(file "$FILE" | grep -c "UTF-8 Unicode text") -ne 0 ]; then
        echo "Local pre-commit hook"
        echo "Error: File $FILE is UTF-8 encoded. Change that to ISO 8859-1 for Amiga and try again!"
        echo "Commit refused."
        exit 1
    fi
done

#check, if WCS is still compileable with SAS/C
cd /home/developer/Desktop/SelcoGit/3DNature/Amiga/
./build_wcs_sasc.sh

7.Mar.23
--------
neuer Amiga-Gcc (4.Mar23) produces crashing executable and many messages like
WCS: classface reloc for _DoMethod is out of range: 00000000
WCS: text reloc for _CheckRexxMsg is out of range: 00000000
WCS: text reloc for _SetRexxVar is out of range: 00000000
WCS: text reloc for _DeletePort is out of range: 00000000
WCS: text reloc for _DeletePort is out of range: 00000000

Compiler vom 08.Nov22 funktioniert.

13.Mar.23
---------
Bebbo hat libamiga.a zwischenzeitlich von NDK 3.2 genommen. Das hatte nicht funktioniert und zu den reloc-Fehlern gefuert.
Compiler vom 08.Mar23 funktioniert.

16.Mar.23
---------
Export LW (Scene+LWO) mit dem ColoDemo-Projekt bringt eine Fehlermeldung. Das LWS-File ist OK, (minimale FloatingPoint-Abweichungen), alle LWO-Files sind unterschiedlich, nur das NULL-Object ist gleich.

--> Dummer Fehler in meinen 
writeFloatArray_BigEndian()
fwriteFloatArray_BigEndian()
fwriteSHORTArray_BigEndian()
Die letzten Bytes wurden falsch geschrieben. (Falsche Variable, i statt k, Argh!)
Jetzt sind die LWO Files identisch. (Bis auf einzelne FloatingPoint Bytes)

-> Damit sieht jetzt auch CanyonSunset in LW richtig aus!
-> ColoDemo bringt beim LW-Export aber eine Fehlermeldung. (Auch im Amiga Original WCS 2.04!)

17.Mar.23
---------
LightWave Export und Colodemo
Scene:       Problem-Message (fscene ist gesetzt, Meldung nur, wenn nicht gesetzt)
Scene+DEM:   Problem-Message (fscene ist gesetzt, Meldung nur, wenn nicht gesetzt)
DEM only:    es muss ein *.elev File ausgewaehlt werden. Das winr dann als LWO-File gespeichert.
Motion only: ExportWave(NULL) aufgerufen, also fscene nicht gesetzt, richtige "keine Keyframes Meldung kommt)

-> Also: Colodemo hat keine Keyframes, deshalb geht Lightwave Export nicht. Einfach Keyframe erzeugen, dann geht es (Amiga) Aros 64 haengt wegen Zune-Notification-Loop?
-> In ExportWave() die vedingung if(Supplied) rausgenommen. Damit werden die Fehlermeldung jetzt immer angezeigt, nicht nur beim reinen Motion-only Export.

Convert DEM
-----------
Bsp. (Files auf Amiga (WinUAE) konvertiert)
Input  :  Vista Pro DEM, alps.dem
Output :  Bin Array -> 258x258 Werten (signed oder unsigned int 1,2 oder 4 Bytes, 8 nicht unterstuetzt)
                       Warum stoert -endian MSB hier? gibt schwarze Bilder   
                       display -depth  8 -size 258x258 gray:alps_s8.bin   <-- nur weisses Bild, alle Hoehen zu gross? (253...1385 laut Test-Button)
                       display -depth 16 -size 258x258 gray:/home/developer/Desktop/SelcoGit/aros_deadw00d/core-linux-x86_64-d/bin/linux-x86_64/AROS/WCS/alps_s16.bin
                       display -depth 32 -size 258x258 gray:/home/developer/Desktop/SelcoGit/aros_deadw00d/core-linux-x86_64-d/bin/linux-x86_64/AROS/WCS/alps_s32.bin
                    ->                (floating point 4 Bytes oder 8 Bytes)
                       display -depth 32 -size 258x258 -endian MSB -define quantum:format=floating-point -define quantum:scale=5e+01  gray:alps_f32.bin
                       display -depth 64 -size 258x258 -endian MSB -define quantum:format=floating-point -define quantum:scale=5e+01  gray:alps_f64.bin
           -----------
                    
          ColorMap ??????? -> Array von 258x258 Bytes, int/unsigned/fload und die Anzahl ValueBytes sind egal

AROS kann das Vista-DEM-File im Moment nicht einlesen.

20Mar23
-------
Unterschiede in DEM-Convert Vista->bin  Files.

FILE=alps_f64.bin; meld <(xxd ~/Desktop/SelcoGit/aros_deadw00d/core-linux-x86_64-d/bin/linux-x86_64/AROS/WCS/aros_"$FILE") <(xxd ~/Desktop/SelcoGit/aros_deadw00d/core-linux-x86_64-d/bin/linux-x86_64/AROS/WCS/"$FILE")

oder cmp fuer alle aros_ Dateien Vergleich mit den Files vom Amiga

for FILE in $(find /home/developer/Desktop/SelcoGit/aros_deadw00d/core-linux-x86_64-d/bin/linux-x86_64/AROS/WCS/ -name "aros_*.bin"); do echo $(echo "$FILE" | sed 's/aros_alps/alps/g') "$FILE"; done

* Vista -> bin OK

* Vista -> ColorMap OK
- 3 color component files ???
- Format (int, float )und Laenge 1,2,4,8 sind egal, werden nicht beachtet.

* Vista -> Zbuffer OK
- erzeugt ein float iff File
- Format (int, float )und Laenge 1,2,4,8 sind egal, werden nicht beachtet.
- An den Namen wird ZB angehaengt

* Vista -> Gray IFF Falsch  ???
* erzeugt 8 Planes IFF File, soll kleiner als 24 Bit sein, ist es aber hier nicht. (Alps-DEM konvertiert

* Vista -> Color IFF OK
* erzeugt 24 Planes IFF (GRAU!!!) File

* WCS DEM sind die .elev-Files zu sein
* WCS DEM -> ZBuffer OK

* Vista ->WCS DEM:  *.elev-Files OK. (kleine Unterschiede im elemaphdr durch floating point bei steplat und steplon)
                    *.Obj-File noch nicht ok. Aenderungen in MapSupport.c Funktion saveobject() noetig.
- Vorgehensweise:
Open Project -> CanyonSunset.project
                WCSProjects:Arizona/SunsetAnim.object/SunsetAnim.par

Convert DEM
  Vista DEM Alps.dem   -> WCS DEM
  * Wir muessen Low/High Lon and Alt eingeben. 
    Woher wissen wir die? 
	Einfach aehnlich werte wie bei den anderen CanyonSunset Sachen eingeben.
	Dazu Database Edit einen von den mit * nehmen und z.B. 36 (+Offset) und 112 (+Offset) eintragen
    
-> WCSProjects:Arizona/SunsetAnim.object
  Endungen elev und Obj werden automatisch erzeugt. Mit Leerzeichen vor dem Punkt.
     AF_ALPS.elev  (133196 Bytes)  // Die Hoehendaten
     AF_ALPS.Obj   (   173 Bytes)  // Da steht WCSVector drin

Das kann man dann in MapView auch anschauen.

23.Mar.2023
-----------
* af_cmpObjFiles.c Tool zum Vergleichen der Obj-Files (WCS-Vector Object, entsteht beim Konvertieren zu WCS DEM)
* Obj-Files vom Vista -> WCS DEM-Konvertieren sind jetzt auch sind jetzt OK.
* Im Obj-File Header war das MaxEl und MinEl-Feld im vectorheaderV100 nicht gesetzt. Jetzt auf 0 initialisiert.


28.Maerz 2023
-------------
Falsche Hintergrund in der Zune-GUI:
Da werden die Zune(MUI) Prefs genommen. Ist beim Amiga auch so. Bei dem Deadw00d (?) AROS sind die Prefs aber "verstellt".
* Es sieht ziemlich so aus wie auf dem Amiga, wenn man die Prefs-Datei einfach löscht.
delete ENVARC:Zune/global.prefs

*Wiederherstellen der AROS Zune-Prefs-Datei:
cp /home/developer/Desktop/SelcoGit/aros_deadw00d/AROS/workbench/prefs/env-archive/default/Zune/global.prefs ~/Desktop/SelcoGit/aros_deadw00d/core-linux-x86_64-d/bin/linux-x86_64/AROS/Prefs/Env-Archive/Zune/

(Es gibt dort Prefs für classic, default und showcase) (classic hat auch flasche Hintergundfarben)

-> Lösung:
In der Shell
zune WCS.1  ; öffnet den Zune(MUI) Prefs Editor. Da alles wie gewünscht einstellen (sieht man aber nicht leider nicht sofort) und dann speichern. Wird in ENVARC: und ENV: gespeichert.
            ; Bei Start wird dann nach "WCS.1.prefs" gesucht.


Convert ZBuffer -> WCS DEM  (ZBuffer-File vorher mit WCS in WinUAE aus ALPS.DEM erstellt) Floating Point, 4 Bytes, HiLo
Die Max/Min-Anzeige ist nach dem einlesen des ZBuffer-Files korrekt. Nach Test springt sie auf 0.0000 und 0.0000
elev-File enthaelt fast nur Nullen???

Convert ZBuffer -> WCS DEM fixed. Resulting Files are identical to Amiga generated ones. (29.3.23)


29.3.23
-------
AROS 64bit
* Eventloop im Render-Window hat deadw00d mit muimaster.library 19.64 behoben
* Komische Zeichen in Tabs, die die Farbe aendern sollen, hat deadw00d in muimaster.library 19.65 behoben

* "Map View" hat noch eine event loop, es flackert und ist nicht bedienbar -> Beispiel an deadw00d geschickt.

30.3.2023
---------
Convert Ascii -> WCS DEM
* Auf dem Amiga ein float-Bin 4 Bytes erzeugen aus dem Vista Alps.dem. Als AF_ALPS_flt4Bin speichern und auf den Linux-Rechner bringen
* od ist ein Standard-Tool zum ausgeben von Hex-Files in allen möglichen Formaten
COLS=258; MYPATH=~/Desktop/SelcoGit/aros_deadw00d/core-linux-x86_64-d/bin/linux-x86_64/AROS/WCS/WCSProjects; od --format=fF --width=$(( $COLS*4 )) --endian=big --address-radix=none --output-duplicates "$MYPATH/AF_ALPS_flt4Bin" > "$MYPATH/AF_ALPS_Float.txt";
* Jetzt haben wir ein ASCII Array mit 258x258 Einträgen. 

*Min und Max aus dem ASCII-File kann man mit datamash (installieren) anzeigen/kontrollieren:
* Dab bestimmt min und max pro Spalte(!) und gibt als Ergebnis eine Zeile mit 258 Minima und Maxima aus. Mit transform machen wir daraus eine(!) Spalte und suchen darin dann nochmal nach min und max
COLUMNS=258; datamash --whitespace min 1-$COLUMNS max 1-$COLUMNS < ~/Desktop/SelcoGit/aros_deadw00d/core-linux-x86_64-d/bin/linux-x86_64/AROS/WCS/WCSProjects/AF_ALPS_Float.txt | datamash transpose | datamash min 1 max 1

* Beim ASCII->WCS DEM Konvertieren fragt er dann unter AROS, ob er invertieren soll!? Das macht er beim Amiga Original nicht. 
* In meiner 68k-Version haengt er sich dann auf - nein, dauert nur EWIG. fgetc() zu langsam bei bebbos gcc?

31.3.2023
---------
* AROS: minimum muimaster.library 19.67 wird jetzt vorrausgesetzt. Da sind alle notwndigen notification-Loop Fixes von deadw00d drin.
* 102030405 abziehen, um Beta-Timeout zu bekommen. (ein wenig verschleiert)

* Kann man Images.c CompRoseImgData irgendwie transparent machen? Dort passt die Hintergrundfarbe nicht zu de AROS Default-Einstellungen.
  in Agui.c  wird es benutzt: Child, ImageObject, MUIA_Image_OldImage, &CompRose,

3.April 2023
------------
Motion-Window Cam-View funktioniert nicht richtig in der 64 Bit-Version. In der 32Bit-Version ost alles OK.
-> 64Bit Problem. "unsigned long" Pointer to BitmapData. Muss fuer Ptr++ aber auf 32Bit Werte zeigen, also ULONG  Korrigiert in dump.c
-> Motion-Window Cam-View geht jetzt auch unter AROS 64Bit.
Das DIAG-Fenster crashed beim Schließen.

4.April23
---------
Es gibt eine Variable "scrnrowzip[2000];" Was ist da auf 2000 beschraenkt?
QCmap[] typ auf von long auf LONG geaendert. Das DIAG Window stuerzt jetzt beim Schließen nicht mehr ab.

4.April23
---------
* Test auf mindest-Revision fuer muimaster.library in der i386 Version eingebaut.
* viele IPTR statt LONG in EdMoGUI.c (wo get()-Aufrufe sind

// x86_64
Motion Editor -> Parameters List -> Center X

EdMoGUI.c
void Set_EM_Item() Zeile 1071

 if (incr[2] == 0.0)
  {
  set(EM_Win->ParTxt[2], MUIA_Text_Contents, (IPTR)"\0");
  sprintf(str, "%f", PAR_FIRST_MOTION(item[2]));  // <-- Crash in sprintf. String ist OK!
                                                  // ein printf (ohne s) crashed auch, wenn es "%f",3.14159 enthaelt
												  // also weder str noch PAR_FIRST_MOTION(item[2]) Problem

18.April23
----------
Test fuer Convert DEM begonnen. (68020)
test_files Schublade nach WinUAE kopieren. Vista DEM -> andere Formate funktioniert weitgehend (ausser WCS DEM Onj-Files und Colormap Files)
- In DataOps.c habe ich ein #define  PRINT_CONVERTDEM_PARAMS  eingebaut. Dann werden die Aufrufparameter der Funktion ConvertDEM() fuer den Test ausgegeben.

- In DataOPs.c #ifdef OLD_COLORMAP eingebaut und Code geaendert. Jetzt werden bei "ColorMap" als Ziel wirklich 3 red, grn und blue Files erzeugt. 
  (mit Leerzeichen wie schon bei Obj und elev). Der Alte Code hat immer nur ein File ohne Endung erzeugt, weil er auf die Endung des INPUT-Files geschaut hat, die
  natuerlich im Normalfall nie "red" ist. Das war ein Fehler im Originalcode.

22.Apr.23
- Wenn ich mit -mregparms compilie ist argc=0. Damit argc/argv klappt, muss die main()-Funktion ein __stdargs vorangestellt bekommen!

AROS: Nur die ersten 64 Bytes von elMapHeaderV101 werden gelesen/geschrieben, also auch diese muessen/duerfen Endian-gedreht werden!


2.Mai.23
--------
Convert-Test mit ASCII als Source. Siehe 30.3.
ASCII-File erzeugen:
* erst ein Bin-Array erzeugen. z.B. Float 4 Bytes. aus BigSur.DEM, weil dort kleine (<128) und grosse Hoehen drin sind. Wie nehmen das BSur.DEMF4 aus test_files/sources, das wir vorher schon mit WCS2.04 erzeugt hatten
COLS=258; MYPATH=~/Desktop/SelcoGit/3DNature/Amiga/test_files/source; od --format=fF --width=$(( $COLS*4 )) --endian=big --address-radix=none --output-duplicates "$MYPATH/BSur.DEMF4" > "$MYPATH/BSur.DEMAS"

4.Mai.23
--------
156 Convert DEM Tests, 3 failed, (gcc for Amiga)

-Floating-Point to hex online converter (braucht man manchmal):  https://gregstoll.com/~gregstoll/floattohex/

5.Mai.23
--------
Fixed Data-Units and one reference file for test. Now only one failed test left. (SumElDifSq wrong. Bug on Amiga?)

- Wenn das Source-Format IFF ist, dann gibt es nur 256 Höhenwerte. 8Bit-IFF Bilder = 256 Werte. 24Bit IFF Bilder werden intern (Rot + Grün + Blau ) /3 gerechnet.
- Wahrscheinlich kann man mir Floor und Ceiling den Wertebereich runterskalieren.
- Ich nehme zum Test einfach BigSur.DEM als Input je einmal Grayiff und einmal ColorIFF als Output. Die Sind dann Source für die IFF-Tests.A

6.Mai.23
--------
IFF Tests fertig. insgesamt 182 Tests, 2 Fehler (SumElDifSq)


24.Mai 2023
-----------
DTED-Files:
Rügen von Earthexplorer.usgs.gov/ geladen. (Account erforderlich).
File: n54_e013_3arc_v2.dt1

Koordinaten:

Resolution 	3-ARC
Date Updated 	2013-04-17 12:17:06-05
NW Corner Lat 	55°00'00"N
NW Corner Long 	13°00'00"E
NE Corner Lat 	55°00'00"N
NE Corner Long 	14°00'00"E
SE Corner Lat 	54°00'00"N
SE Corner Long 	14°00'00"E
SW Corner Lat 	54°00'00"N
SW Corner Long 	13°00'00"E

Die Datei kann mit WCS convertiert werden. Allerdings ist in der Mapansicht nur Unfug zu sehen und das Programm stürzt dann auch später ab.

Das Format ist 1201x601 Pixel. Kann WCS das? Mal als iff oder ASCII-Array convertieren.

Zuerst mit gdal das dt1-File in ein Bildformat umwandeln:
docker run -it -v /home/developer/Desktop/SelcoGit/3DNature/Amiga/test_usgs_dted:/tmp  ghcr.io/osgeo/gdal:alpine-small-3.7.0
/ # gdal_translate tmp/n54_e013_3arc_v2.dt1 -of gif /tmp/n54_e013_3arc_v2.gif

Das gif-Bild in ein iff-Bild umwandeln:
convert n54_e013_3arc_v2.gif  n54_e013_3arc_v2.ilbm
identify n54_e013_3arc_v2.ilbm
display n54_e013_3arc_v2.ilbm

* Das IFF-Bild kann WCS 2.04 nach WCS-DEM konvertien. Es kann in der Mapview richtig angezeigt werden. (Scale 25, Lat 54.5, Lon -13.5, Exag 48)

Man kann auch ein ASCII-Array erzeugen:
gdal_translate -of AAIGrid /tmp/n54_e013_3arc_v2.dt1 /tmp/n54_e013_3arc_v2.asc

Von dem n54_e013_3arc_v2.asc müssen dann die ersten Zeilen entfernt werden, sie sind Beschreibung.
tail -n +8 n54_e013_3arc_v2.asc > n54_e013_3arc_v2.ascarr

Kann konvertiert werden. Bei Value-Bytes 2 oder 4 einstellen, sonst wird bei 8Bit abgeschnitten!

26.Mai.2023
-----------
Bild auf 301x301 verkleinern:
Das ! sorgt dafuer, das der Aspect ignoriert wird. Sonst macht er 151x301 draus.

convert n54_e013_3arc_v2.ilbm -resize 301x301! n54_e013_3arc_v2_small.ilbm
identify n54_e013_3arc_v2_small.ilbm


5.Juni.2023
----------
DTED File Rügen (601x1201) ist total verzerrt.
DTED-File Teneriffa (1201x1201) is ok!
-> Problem scheint zu sein, wenn DTED-File nicht quadratisch ist.
Die DTED-Lesefunktion LoadDTED() ist ok. Der Fehler muss später sein.

9.Juni2023
----------
Wenn in Convert DEM ein WCS-DEM File geladen wird, werden jetzt im DEM Registrat TAB die Lo und Hi Koordinaten angezeigt, genauso wir beim Laden von DTED-Dateien.

13.6.2023
---------
Die Targets Profiling und Coveriage ließen sich nicht mehr linken. Lag ev. am neuem Eclipse? -lm, -lmui tauchte in der Linker-Zeile ganz am Ende auf, also hinter dem Test-Script, nicht am Ende des Linker-Aufrufes. -> Korrigiert.

Coverage:
Auf dem Amiga in der Linux-Verzeichnis gehen, wo WCS_Coverage liegt. WCS_Coverage starten.
Auf Linux:
cd ~/Desktop/SelcoGit/3DNature/Amiga/Coverage
gcovr --gcov-executable=m68k-amigaos-gcov --object-directory=. -r .. --html --html-details -o coverage.html

14.6.2023
---------
Coverage geht wieder. -mregparm musste weg, sonst wurden keine gcda-Files erzeugt.

--> coverage als ASCII, damit man verschiedene Laeufe vergleichen kann:
for FILE in $(ls *.gcno); do m68k-amigaos-gcov $(basename $FILE .gcno); done
mkdir no_scale
mv *.gcov no_scale/
rm *.cgda  # neue Coverage, nicht aufaddieren

DTED-File convertieren und diesemal mit Skalierung, also neue Rows/Columns Werte

und dann gcno-Dateien mit meld vergleichen

16.6.2023
---------
Skalieren geht nicht bei ILBM auch nicht. (Ruegen 601x1201 -> 301x601) zeigt nur 1. Viertel

19.6.2023
---------
Skalieren bei ILBM geht jetzt.

22.6.2023
---------
Damit das Skalieren auch bei DTED geht, muessen einige Werte vertauscht werden. LastInRow/Col, INPUT_ROWS/COLS, Row/ColStep in ConvertDEM().

- DTED -> IFF geht nur, wenn Cols==Rows, sonst nicht.
- IFF  -> IFF geht.

30.6.2023
---------
- IFF -> IFF geht nicht, wenn Targed x!=y, 
also Ruegen 601x1201 -> 301x601 geht nicht, 
     Ruegen 601x1201 -> 301x301 geht aber


