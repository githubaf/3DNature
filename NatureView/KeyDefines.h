// KeyDefines.h

#ifndef NVW_KEYDEFINES_H
#define NVW_KEYDEFINES_H

#define NV_KEY_HIDEWINTOGGLE		osgGA::GUIEventAdapter::KEY_Tab
#define NV_KEY_HIDEWINTOGGLEDESC	"TAB"
#define NV_KEY_DISABLETOGGLE		'`'
#define NV_KEY_DISABLETOGGLEDESC	"` (backquote key)"
#define NV_KEY_OCEANTOGGLE			'0'
#define NV_KEY_OCEANTOGGLEDESC		"0"
#define NV_KEY_TERRAINTOGGLE		'1'
#define NV_KEY_TERRAINTOGGLEDESC	"1"
#define NV_KEY_VECTOGGLE			'2'
#define NV_KEY_VECTOGGLEDESC		"2"
#define NV_KEY_ALTDRAPETOGGLE		'5'
#define NV_KEY_ALTDRAPETOGGLEDESC	"5"
#define NV_KEY_FOLIAGETOGGLE		'6'
#define NV_KEY_FOLIAGETOGGLEDESC	"6"
#define NV_KEY_OBJECTTOGGLE			'7'
#define NV_KEY_OBJECTTOGGLEDESC		"7"
#define NV_KEY_OVERLAYTOGGLE		'8'
#define NV_KEY_OVERLAYTOGGLEDESC	"8"
#define NV_KEY_LABELTOGGLE			'9'
#define NV_KEY_LABELTOGGLEDESC		"9"
#define NV_KEY_STATSTOGGLE			'*'
#define NV_KEY_STATSTOGGLE2			osgGA::GUIEventAdapter::KEY_KP_Multiply
#define NV_KEY_STATSTOGGLEDESC		"* (Asterisk)"
#define NV_KEY_FULLTOGGLE			'f'
#define NV_KEY_FULLTOGGLEDESC		"F"
#define NV_KEY_RETURNHOME			' '
#define NV_KEY_RETURNHOME2			osgGA::GUIEventAdapter::KEY_Home
#define NV_KEY_RETURNHOMEDESC		"Spacebar or Home"
#define NV_KEY_STOPMOVING			'.'
#define NV_KEY_STOPMOVINGDESC		"\'.\' (Period key)"
#define NV_KEY_THROTTLEUP			']'
#define NV_KEY_THROTTLEUPDESC		"]"
#define NV_KEY_THROTTLEDN			'['
#define NV_KEY_THROTTLEDNDESC		"["
//#define NV_KEY_EXIT				'' // ESC
#define NV_KEY_EXITDESC				"Escape"
#define NV_KEY_HELPTOGGLE			'h'
#define NV_KEY_HELPDESC				"\'H\' or F1"
#define NV_KEY_HELPTOGGLE2			osgGA::GUIEventAdapter::KEY_F1
#define NV_KEY_ZOOMIN				'='
#define NV_KEY_ZOOMIN2				'+'
#define NV_KEY_ZOOMIN3				osgGA::GUIEventAdapter::KEY_KP_Add
#define NV_KEY_ZOOMINDESC			"+"
#define NV_KEY_ZOOMOUT				'-'
#define NV_KEY_ZOOMOUT2				'_'
#define NV_KEY_ZOOMOUT3				osgGA::GUIEventAdapter::KEY_KP_Subtract
#define NV_KEY_ZOOMOUTDESC			"-"
#define NV_KEY_TERRAINFOLLOW		't'
#define NV_KEY_TERRAINFOLLOWDESC	"T"
#define NV_KEY_NEXTCAM				'n'
#define NV_KEY_NEXTCAMDESC			"N"
#define NV_KEY_PREVCAM				'p'
#define NV_KEY_PREVCAMDESC			"P"
#define NV_KEY_SELCAM				'/'
#define NV_KEY_SELCAMDESC			"/"
#define NV_KEY_SELCAT				'\\'
#define NV_KEY_SELCATDESC			"\\"
#define NV_KEY_TESTONE				'z'
#define NV_KEY_VIEWTOUR			osgGA::GUIEventAdapter::KEY_End // viewpoint tour toggle
#define NV_KEY_VIEWTOURDESC		"End"
#define NV_KEY_MOVEOPT				','
#define NV_KEY_MOVEOPTDESC			"[Comma]"

// NavMode switching
#define NV_KEY_MOVEMODE				'm'
#define NV_KEY_MOVEMODEDESC			"M"
#define NV_KEY_ROTMODE				'r'
#define NV_KEY_ROTMODEDESC			"R or [CTRL]"
#define NV_KEY_SLIDEMODE			'x'
#define NV_KEY_SLIDEMODEDESC		"X or [ALT]"
#define NV_KEY_CLIMBMODE			0 // no hotkey for it directly
#define NV_KEY_CLIMBMODEDESC		"[SHIFT]"
#define NV_KEY_QUERYMODE			'q'
#define NV_KEY_QUERYMODEDESC		"Q"


//#define NV_KEY_UNDO					osgGA::GUIEventAdapter::KEY_Undo
#define NV_KEY_UNDO					'z' // plus CTRL
#define NV_KEY_UNDODESC				"CRTL+Z"

#define NV_KEY_QUERY				osgGA::GUIEventAdapter::KEY_Menu
#define NV_KEY_QUERYDESC			"[Context Menu Key]"


// Quake-style navigation keys
#define NV_KEY_MOVELEFT				'a'
#define NV_KEY_MOVELEFTDESC			"A"

#define NV_KEY_MOVERIGHT			'd'
#define NV_KEY_MOVERIGHTDESC		"D"

#define NV_KEY_MOVEFWD				'w'
#define NV_KEY_MOVEFWDDESC			"W"
#define NV_KEY_MOVEFWD2				osgGA::GUIEventAdapter::KEY_Up
#define NV_KEY_MOVEFWD2DESC			"Arrow Up"

#define NV_KEY_MOVEBACK				's'
#define NV_KEY_MOVEBACKDESC			"S"
#define NV_KEY_MOVEBACK2			osgGA::GUIEventAdapter::KEY_Down
#define NV_KEY_MOVEBACK2DESC		"Arrow Down"

#define NV_KEY_MOVEUP				'g'
#define NV_KEY_MOVEUPDESC			"G"

#define NV_KEY_MOVEDOWN				'v'
#define NV_KEY_MOVEDOWNDESC			"V"

#define NV_KEY_TURNLEFT				osgGA::GUIEventAdapter::KEY_Left
#define NV_KEY_TURNLEFTDESC			"Arrow Left"
#define NV_KEY_TURNLEFT2			'j'
#define NV_KEY_TURNLEFT2DESC		"J"
#define NV_KEY_TURNRIGHT			osgGA::GUIEventAdapter::KEY_Right
#define NV_KEY_TURNRIGHTDESC		"Arrow Right"
#define NV_KEY_TURNRIGHT2			'l'
#define NV_KEY_TURNRIGHT2DESC		"L"
#define NV_KEY_TURNUP				'i'
#define NV_KEY_TURNUPDESC			"I"
#define NV_KEY_TURNDOWN				'k'
#define NV_KEY_TURNDOWNDESC			"K"
#define NV_KEY_TURNCLOCK			'o'
#define NV_KEY_TURNCLOCKDESC		"O"
#define NV_KEY_TURNCCLOCK			'u'
#define NV_KEY_TURNCCLOCKDESC		"U"

// Misc keys
#define NV_KEY_SOUNDSTOP			'y'
#define NV_KEY_SOUNDSTOPDESC		"Y"

// LOD
#define NV_KEY_LODMORE				'}'
#define NV_KEY_LODLESS				'{'
#define NV_KEY_LODMOREB				osgGA::GUIEventAdapter::KEY_F12
#define NV_KEY_LODLESSB				osgGA::GUIEventAdapter::KEY_F11

// Test
#define NV_KEY_TESTTWO				osgGA::GUIEventAdapter::KEY_F2
#define NV_KEY_TESTTHREE			osgGA::GUIEventAdapter::KEY_F3
#define NV_KEY_TESTFOUR				osgGA::GUIEventAdapter::KEY_F4
#define NV_KEY_TESTFIVE				osgGA::GUIEventAdapter::KEY_F5
#define NV_KEY_TESTSIX					osgGA::GUIEventAdapter::KEY_F6
#define NV_KEY_TESTSEVEN			osgGA::GUIEventAdapter::KEY_F7
#define NV_KEY_TESTEIGHT			osgGA::GUIEventAdapter::KEY_F8
#define NV_KEY_TESTNINE				osgGA::GUIEventAdapter::KEY_F9
#define NV_KEY_TESTTEN					osgGA::GUIEventAdapter::KEY_F10
//#define NV_KEY_TESTELEVEN			osgGA::GUIEventAdapter::KEY_F11
//#define NV_KEY_TESTTWELVE			osgGA::GUIEventAdapter::KEY_F12

#endif // NVW_KEYDEFINES_H
