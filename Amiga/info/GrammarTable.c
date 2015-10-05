/* GrammarTable.c
**
** C-Source for binary Markov word-sentance table.
** Machine generated code, do NOT hand-alter.
** Created from file "info/GrammarTest".
*/

#define EXT extern
#include "GrammarTable.h"


struct MWS_Entry SentLookUp[] =
{
{0x000000a0, 0x00000004, 0},             /*   0: DATAbase goes to 4. */
{0x00000212, 0x00000009, 0},             /*   1: PARameters goes to 9. */
{0x0000022b, 0x000000c2, 0},             /*   2: PROject goes to 194. */
{0x100002ac, 0x00000120, 0},             /*   3: STATus goes to 288. */

{0x000000e0, 0x00000007, 0},             /*   4: ENabled goes to 7. */
{0x00000189, 0,          DataBase},      /*   5: LOAd (DataBase) */
{0x10000278, 0,          DataBase},      /*   6: SAVe (DataBase) */

{0x000001e8, 0,          DataBase},      /*   7: OFF (DataBase) */
{0x100001ec, 0,          DataBase},      /*   8: ON (DataBase) */

{0x00000176, 0x00000010, 0},             /*   9: Keyframe goes to 16. */
{0x00000189, 0,          ParamIO},       /*  10: LOAd (ParamIO) */
{0x000001c1, 0x00000012, 0},             /*  11: MOTion goes to 18. */
{0x00000258, 0x0000003c, 0},             /*  12: RENder goes to 60. */
{0x00000278, 0,          ParamIO},       /*  13: SAVe (ParamIO) */
{0x2000007f, 0x0000009d, 0},             /*  14: COLor [arg] goes to 157. */
{0x300000da, 0x000000a9, 0},             /*  15: ECosystem [arg] goes to 169. */

{0x000000ae, 0,          KeyOps},        /*  16: DELete (KeyOps) */
{0x100001a1, 0,          KeyOps},        /*  17: MAKe (KeyOps) */

{0x00000027, 0,          MotionKey},     /*  18: ALL (MotionKey) */
{0x00000051, 0,          MotionKey},     /*  19: BANk (MotionKey) */
{0x00000054, 0,          MotionKey},     /*  20: BIas (MotionKey) */
{0x00000067, 0x00000026, 0},             /*  21: CAMEra goes to 38. */
{0x00000070, 0x00000029, 0},             /*  22: CEnter goes to 41. */
{0x0000008e, 0,          MotionKey},     /*  23: CONtinuity (MotionKey) */
{0x000000a2, 0,          MotionKey},     /*  24: DATUm (MotionKey) */
{0x000000d0, 0x0000002b, 0},             /*  25: EARth goes to 43. */
{0x00000116, 0,          MotionKey},     /*  26: FLATTENIng (MotionKey) */
{0x0000011c, 0x0000002c, 0},             /*  27: FOCus goes to 44. */
{0x0000011d, 0x0000002f, 0},             /*  28: FOG goes to 47. */
{0x00000152, 0x00000031, 0},             /*  29: HAZe goes to 49. */
{0x00000160, 0x00000033, 0},             /*  30: HOrizon goes to 51. */
{0x0000027d, 0,          MotionKey},     /*  31: SCAle (MotionKey) */
{0x00000293, 0x00000036, 0},             /*  32: SHade goes to 54. */
{0x000002b6, 0x00000037, 0},             /*  33: SUN goes to 55. */
{0x000002d1, 0,          MotionKey},     /*  34: TENsion (MotionKey) */
{0x00000300, 0x00000039, 0},             /*  35: VERtical goes to 57. */
{0x00000303, 0x0000003a, 0},             /*  36: VIew goes to 58. */
{0x10000310, 0x0000003b, 0},             /*  37: Z goes to 59. */

{0x0000002d, 0,          MotionKey},     /*  38: ALTitude (MotionKey) */
{0x00000180, 0,          MotionKey},     /*  39: LAtitude (MotionKey) */
{0x10000190, 0,          MotionKey},     /*  40: LONgitude (MotionKey) */

{0x0000030e, 0,          MotionKey},     /*  41: X (MotionKey) */
{0x1000030f, 0,          MotionKey},     /*  42: Y (MotionKey) */

{0x10000261, 0,          MotionKey},     /*  43: ROtation (MotionKey) */

{0x0000002d, 0,          MotionKey},     /*  44: ALTitude (MotionKey) */
{0x00000180, 0,          MotionKey},     /*  45: LAtitude (MotionKey) */
{0x10000190, 0,          MotionKey},     /*  46: LONgitude (MotionKey) */

{0x0000012e, 0,          MotionKey},     /*  47: FUll (MotionKey) */
{0x100001d7, 0,          MotionKey},     /*  48: NONe (MotionKey) */

{0x00000241, 0,          MotionKey},     /*  49: RANGe (MotionKey) */
{0x100002a9, 0,          MotionKey},     /*  50: STARt (MotionKey) */

{0x00000183, 0,          MotionKey},     /*  51: LIne (MotionKey) */
{0x0000021e, 0,          MotionKey},     /*  52: POint (MotionKey) */
{0x100002b3, 0,          MotionKey},     /*  53: STRetch (MotionKey) */

{0x100000fc, 0,          MotionKey},     /*  54: FACtor (MotionKey) */

{0x00000180, 0,          MotionKey},     /*  55: LAtitude (MotionKey) */
{0x10000190, 0,          MotionKey},     /*  56: LONgitude (MotionKey) */

{0x100000ec, 0,          MotionKey},     /*  57: EXAggeration (MotionKey) */

{0x10000037, 0,          MotionKey},     /*  58: ARc (MotionKey) */

{0x100001b4, 0,          MotionKey},     /*  59: MInimum (MotionKey) */

{0x00000026, 0x00000067, 0},             /*  60: ALIas goes to 103. */
{0x00000035, 0,          RenderSet},     /*  61: ANtialias (RenderSet) */
{0x0000004f, 0,          RenderSet},     /*  62: BACkground (RenderSet) */
{0x00000051, 0x00000068, 0},             /*  63: BANk goes to 104. */
{0x0000005c, 0x0000006a, 0},             /*  64: BOrder goes to 106. */
{0x00000079, 0,          RenderSet},     /*  65: CLOuds (RenderSet) */
{0x0000007f, 0x0000006b, 0},             /*  66: COLor goes to 107. */
{0x00000086, 0,          RenderSet},     /*  67: COMposite (RenderSet) */
{0x000000aa, 0x0000006d, 0},             /*  68: DEFault goes to 109. */
{0x000000c7, 0x0000006e, 0},             /*  69: DRaw goes to 110. */
{0x000000d2, 0x0000006f, 0},             /*  70: EASe goes to 111. */
{0x000000da, 0x00000071, 0},             /*  71: ECosystem goes to 113. */
{0x000000f0, 0x00000072, 0},             /*  72: EXPort goes to 114. */
{0x00000104, 0x00000073, 0},             /*  73: FIEld goes to 115. */
{0x0000010d, 0x00000074, 0},             /*  74: FIXEd goes to 116. */
{0x00000113, 0x00000075, 0},             /*  75: FLATTEN goes to 117. */
{0x00000128, 0,          RenderSet},     /*  76: FRACtal (RenderSet) */
{0x00000135, 0x00000076, 0},             /*  77: GLobal goes to 118. */
{0x00000146, 0x0000007c, 0},             /*  78: GRId goes to 124. */
{0x00000160, 0x0000007d, 0},             /*  79: HOrizon goes to 125. */
{0x00000183, 0x0000007e, 0},             /*  80: LIne goes to 126. */
{0x00000197, 0x00000082, RenderSet},     /*  81: LOOkahead goes to 130. (RenderSet) */
{0x000001a2, 0x00000083, 0},             /*  82: MAP goes to 131. */
{0x000001ae, 0x00000085, 0},             /*  83: MAXimum goes to 133. */
{0x000001fe, 0,          RenderSet},     /*  84: OVerscan (RenderSet) */
{0x0000021a, 0x00000086, 0},             /*  85: PIcture goes to 134. */
{0x00000226, 0x00000087, 0},             /*  86: PRIority goes to 135. */
{0x00000254, 0x0000008a, 0},             /*  87: RELIef goes to 138. */
{0x00000258, 0x0000008b, 0},             /*  88: RENder goes to 139. */
{0x00000278, 0x0000008d, 0},             /*  89: SAVe goes to 141. */
{0x00000281, 0x0000008e, 0},             /*  90: SCReen goes to 142. */
{0x00000296, 0x00000090, 0},             /*  91: SIze goes to 144. */
{0x0000029b, 0x00000093, 0},             /*  92: SKY goes to 147. */
{0x000002a9, 0x00000094, 0},             /*  93: STARt goes to 148. */
{0x000002ae, 0x00000095, 0},             /*  94: STEp goes to 149. */
{0x000002bb, 0,          RenderSet},     /*  95: SURface (RenderSet) */
{0x000002d6, 0x00000096, 0},             /*  96: TRees goes to 150. */
{0x000002f4, 0x00000097, 0},             /*  97: VECtor goes to 151. */
{0x000002fa, 0x00000098, 0},             /*  98: VELocity goes to 152. */
{0x0000030d, 0x00000099, 0},             /*  99: WOrld goes to 153. */
{0x00000310, 0x0000009a, 0},             /*  100: Z goes to 154. */
{0x00000317, 0x0000009b, RenderSet},     /*  101: ZBuffer goes to 155. (RenderSet) */
{0x1000031c, 0,          RenderSet},     /*  102: ZEnith (RenderSet) */

{0x100000fc, 0,          RenderSet},     /*  103: FACtor (RenderSet) */

{0x000000fc, 0,          RenderSet},     /*  104: FACtor (RenderSet) */
{0x100002d9, 0,          RenderSet},     /*  105: TUrn (RenderSet) */

{0x1000023f, 0,          RenderSet},     /*  106: RANDom (RenderSet) */

{0x100001a2, 0x0000006c, RenderSet},     /*  107: MAP goes to 108. (RenderSet) */

{0x100002d6, 0,          RenderSet},     /*  108: TRees (RenderSet) */

{0x100000da, 0,          RenderSet},     /*  109: ECosystem (RenderSet) */

{0x10000146, 0,          RenderSet},     /*  110: GRId (RenderSet) */

{0x00000169, 0,          RenderSet},     /*  111: IN (RenderSet) */
{0x100001f7, 0,          RenderSet},     /*  112: OUt (RenderSet) */

{0x100001a9, 0,          RenderSet},     /*  113: MATch (RenderSet) */

{0x10000317, 0,          RenderSet},     /*  114: ZBuffer (RenderSet) */

{0x10000258, 0,          RenderSet},     /*  115: RENder (RenderSet) */

{0x10000128, 0,          RenderSet},     /*  116: FRACtal (RenderSet) */

{0x100000da, 0,          RenderSet},     /*  117: ECosystem (RenderSet) */

{0x000000da, 0x00000079, 0},             /*  118: ECosystem goes to 121. */
{0x0000024d, 0x0000007a, 0},             /*  119: REFerence goes to 122. */
{0x100002a2, 0x0000007b, 0},             /*  120: SNow goes to 123. */

{0x1000013f, 0,          RenderSet},     /*  121: GRADient (RenderSet) */

{0x10000180, 0,          RenderSet},     /*  122: LAtitude (RenderSet) */

{0x1000013f, 0,          RenderSet},     /*  123: GRADient (RenderSet) */

{0x10000296, 0,          RenderSet},     /*  124: SIze (RenderSet) */

{0x1000010b, 0,          RenderSet},     /*  125: FIX (RenderSet) */

{0x000000fe, 0,          RenderSet},     /*  126: FADe (RenderSet) */
{0x000001eb, 0,          RenderSet},     /*  127: OFFSet (RenderSet) */
{0x100002d2, 0x00000081, 0},             /*  128: TO goes to 129. */

{0x10000281, 0,          RenderSet},     /*  129: SCReen (RenderSet) */

{0x1000012b, 0,          RenderSet},     /*  130: FRAMes (RenderSet) */

{0x10000038, 0x00000084, 0},             /*  131: AS goes to 132. */

{0x100002bb, 0,          RenderSet},     /*  132: SURface (RenderSet) */

{0x1000012b, 0,          RenderSet},     /*  133: FRAMes (RenderSet) */

{0x1000003d, 0,          RenderSet},     /*  134: ASPect (RenderSet) */

{0x0000015a, 0,          RenderSet},     /*  135: HIgh (RenderSet) */
{0x00000198, 0,          RenderSet},     /*  136: LOW (RenderSet) */
{0x100001db, 0,          RenderSet},     /*  137: NORmal (RenderSet) */

{0x10000293, 0,          RenderSet},     /*  138: SHade (RenderSet) */

{0x000001f5, 0,          RenderSet},     /*  139: OPTions (RenderSet) */
{0x10000289, 0,          RenderSet},     /*  140: SEGments (RenderSet) */

{0x10000168, 0,          RenderSet},     /*  141: IFf (RenderSet) */

{0x00000157, 0,          RenderSet},     /*  142: HEight (RenderSet) */
{0x10000309, 0,          RenderSet},     /*  143: WIdth (RenderSet) */

{0x0000012e, 0,          RenderSet},     /*  144: FUll (RenderSet) */
{0x00000150, 0,          RenderSet},     /*  145: HALf (RenderSet) */
{0x10000233, 0,          RenderSet},     /*  146: QUArter (RenderSet) */

{0x10000026, 0,          RenderSet},     /*  147: ALIas (RenderSet) */

{0x1000012b, 0,          RenderSet},     /*  148: FRAMes (RenderSet) */

{0x1000012b, 0,          RenderSet},     /*  149: FRAMes (RenderSet) */

{0x100000fc, 0,          RenderSet},     /*  150: FACtor (RenderSet) */

{0x10000289, 0,          RenderSet},     /*  151: SEGments (RenderSet) */

{0x100000c4, 0,          RenderSet},     /*  152: DIStribution (RenderSet) */

{0x100001a2, 0,          RenderSet},     /*  153: MAP (RenderSet) */

{0x10000026, 0,          RenderSet},     /*  154: ALIas (RenderSet) */

{0x00000026, 0,          RenderSet},     /*  155: ALIas (RenderSet) */
{0x10000121, 0,          RenderSet},     /*  156: FORmat (RenderSet) */

{0x00000054, 0,          ColorKey},      /*  157: BIas (ColorKey) */
{0x00000057, 0,          ColorKey},      /*  158: BLue (ColorKey) */
{0x0000008e, 0,          ColorKey},      /*  159: CONtinuity (ColorKey) */
{0x00000144, 0,          ColorKey},      /*  160: GREen (ColorKey) */
{0x00000162, 0,          ColorKey},      /*  161: HSv (ColorKey) */
{0x00000164, 0,          ColorKey},      /*  162: HUe (ColorKey) */
{0x000001c6, 0,          ColorKey},      /*  163: NAme (ColorKey) */
{0x00000246, 0,          ColorKey},      /*  164: RED (ColorKey) */
{0x0000025a, 0,          ColorKey},      /*  165: RGb (ColorKey) */
{0x00000276, 0,          ColorKey},      /*  166: SATuration (ColorKey) */
{0x000002d1, 0,          ColorKey},      /*  167: TENsion (ColorKey) */
{0x100002ed, 0,          ColorKey},      /*  168: VAlue (ColorKey) */

{0x0000007f, 0,          EcoKey},        /*  169: COLor (EcoKey) */
{0x00000183, 0,          EcoKey},        /*  170: LIne (EcoKey) */
{0x000001a9, 0x000000b5, 0},             /*  171: MATch goes to 181. */
{0x000001ae, 0x000000b9, 0},             /*  172: MAXimum goes to 185. */
{0x000001b4, 0x000000bb, 0},             /*  173: MInimum goes to 187. */
{0x000001ba, 0x000000bd, 0},             /*  174: MODEL goes to 189. */
{0x000001c6, 0,          EcoKey},        /*  175: NAme (EcoKey) */
{0x00000251, 0,          EcoKey},        /*  176: RELEl (EcoKey) */
{0x0000029a, 0x000000be, EcoKey},        /*  177: SKEw goes to 190. (EcoKey) */
{0x000002d6, 0x000000bf, 0},             /*  178: TRees goes to 191. */
{0x000002dc, 0,          EcoKey},        /*  179: TYpe (EcoKey) */
{0x100002e6, 0x000000c1, 0},             /*  180: Understory goes to 193. */

{0x1000007f, 0x000000b6, 0},             /*  181: COLor goes to 182. */

{0x00000057, 0,          EcoKey},        /*  182: BLue (EcoKey) */
{0x00000144, 0,          EcoKey},        /*  183: GREen (EcoKey) */
{0x10000246, 0,          EcoKey},        /*  184: RED (EcoKey) */

{0x00000251, 0,          EcoKey},        /*  185: RELEl (EcoKey) */
{0x1000029f, 0,          EcoKey},        /*  186: SLope (EcoKey) */

{0x00000251, 0,          EcoKey},        /*  187: RELEl (EcoKey) */
{0x1000029f, 0,          EcoKey},        /*  188: SLope (EcoKey) */

{0x1000010a, 0,          EcoKey},        /*  189: FILename (EcoKey) */

{0x10000041, 0,          EcoKey},        /*  190: AZ (EcoKey) */

{0x000000b3, 0,          EcoKey},        /*  191: DENsity (EcoKey) */
{0x10000157, 0,          EcoKey},        /*  192: HEight (EcoKey) */

{0x100000da, 0,          EcoKey},        /*  193: ECosystem (EcoKey) */

{0x0000004f, 0x000000d6, 0},             /*  194: BACkground goes to 214. */
{0x0000006b, 0x000000d8, ViewOps},       /*  195: CAMView goes to 216. (ViewOps) */
{0x0000007f, 0x000000dc, 0},             /*  196: COLor goes to 220. */
{0x000000a0, 0x000000de, 0},             /*  197: DATAbase goes to 222. */
{0x000000b6, 0x000000e0, 0},             /*  198: DIR goes to 224. */
{0x000000ba, 0x000000e1, 0},             /*  199: DIRList goes to 225. */
{0x0000012b, 0x000000e5, 0},             /*  200: FRAMes goes to 229. */
{0x00000141, 0x000000e7, 0},             /*  201: GRAPh goes to 231. */
{0x00000183, 0x000000e9, 0},             /*  202: LIne goes to 233. */
{0x00000189, 0,          ProjectOps},    /*  203: LOAd (ProjectOps) */
{0x000001a6, 0x000000eb, MapOps},        /*  204: MAPView goes to 235. (MapOps) */
{0x000001ba, 0x000000f7, 0},             /*  205: MODEL goes to 247. */
{0x000001bd, 0x000000f8, 0},             /*  206: MODUle goes to 248. */
{0x00000212, 0x00000111, 0},             /*  207: PARameters goes to 273. */
{0x0000022b, 0x00000113, 0},             /*  208: PROject goes to 275. */
{0x00000235, 0,          Quit},          /*  209: QUIt (Quit) */
{0x00000258, 0x00000115, 0},             /*  210: RENder goes to 277. */
{0x00000278, 0x0000011b, ProjectOps},    /*  211: SAVe goes to 283. (ProjectOps) */
{0x000002cc, 0x0000011c, 0},             /*  212: TEMporary goes to 284. */
{0x10000317, 0x0000011e, 0},             /*  213: ZBuffer goes to 286. */

{0x0000010a, 0,          ProjectOps},    /*  214: FILename (ProjectOps) */
{0x10000214, 0,          ProjectOps},    /*  215: PATh (ProjectOps) */

{0x00000094, 0x000000da, 0},             /*  216: CUrrent goes to 218. */
{0x100000c7, 0x000000db, ViewOps},       /*  217: DRaw goes to 219. (ViewOps) */

{0x1000012b, 0,          ViewOps},       /*  218: FRAMes (ViewOps) */

{0x100001b9, 0,          ViewOps},       /*  219: MODE (ViewOps) */

{0x100001a2, 0x000000dd, 0},             /*  220: MAP goes to 221. */

{0x10000214, 0,          ProjectOps},    /*  221: PATh (ProjectOps) */

{0x0000010a, 0,          ProjectOps},    /*  222: FILename (ProjectOps) */
{0x10000214, 0,          ProjectOps},    /*  223: PATh (ProjectOps) */

{0x100001c6, 0,          ProjectOps},    /*  224: NAme (ProjectOps) */

{0x00000075, 0,          ProjectOps},    /*  225: CLEar (ProjectOps) */
{0x000000aa, 0,          ProjectOps},    /*  226: DEFault (ProjectOps) */
{0x10000214, 0x000000e4, 0},             /*  227: PATh goes to 228. */

{0x10000020, 0,          ProjectOps},    /*  228: ADd (ProjectOps) */

{0x0000010a, 0,          ProjectOps},    /*  229: FILename (ProjectOps) */
{0x10000214, 0,          ProjectOps},    /*  230: PATh (ProjectOps) */

{0x000001c6, 0,          ProjectOps},    /*  231: NAme (ProjectOps) */
{0x10000214, 0,          ProjectOps},    /*  232: PATh (ProjectOps) */

{0x0000010a, 0,          ProjectOps},    /*  233: FILename (ProjectOps) */
{0x10000214, 0,          ProjectOps},    /*  234: PATh (ProjectOps) */

{0x000000c7, 0x000000f2, 0},             /*  235: DRaw goes to 242. */
{0x000000ec, 0,          MapOps},        /*  236: EXAggeration (MapOps) */
{0x00000180, 0,          MapOps},        /*  237: LAtitude (MapOps) */
{0x00000190, 0,          MapOps},        /*  238: LONgitude (MapOps) */
{0x000001f0, 0,          MapOps},        /*  239: OPEn (MapOps) */
{0x0000020a, 0,          MapOps},        /*  240: PALette (MapOps) */
{0x1000028a, 0x000000f6, 0},             /*  241: SET goes to 246. */

{0x00000040, 0,          MapOps},        /*  242: ASYnc (MapOps) */
{0x000001b9, 0,          MapOps},        /*  243: MODE (MapOps) */
{0x100002be, 0x000000f5, 0},             /*  244: SYnc goes to 245. */

{0x10000040, 0,          MapOps},        /*  245: ASYnc (MapOps) */

{0x10000296, 0,          MapOps},        /*  246: SIze (MapOps) */

{0x10000214, 0,          ProjectOps},    /*  247: PATh (ProjectOps) */

{0x0000006b, 0x000000fb, ViewOps},       /*  248: CAMView goes to 251. (ViewOps) */
{0x000001a6, 0x000000ff, MapOps},        /*  249: MAPView goes to 255. (MapOps) */
{0x10000258, 0x0000010b, 0},             /*  250: RENder goes to 267. */

{0x00000094, 0x000000fd, 0},             /*  251: CUrrent goes to 253. */
{0x100000c7, 0x000000fe, ViewOps},       /*  252: DRaw goes to 254. (ViewOps) */

{0x1000012b, 0,          ViewOps},       /*  253: FRAMes (ViewOps) */

{0x100001b9, 0,          ViewOps},       /*  254: MODE (ViewOps) */

{0x000000c7, 0x00000106, 0},             /*  255: DRaw goes to 262. */
{0x000000ec, 0,          MapOps},        /*  256: EXAggeration (MapOps) */
{0x00000180, 0,          MapOps},        /*  257: LAtitude (MapOps) */
{0x00000190, 0,          MapOps},        /*  258: LONgitude (MapOps) */
{0x000001f0, 0,          MapOps},        /*  259: OPEn (MapOps) */
{0x0000020a, 0,          MapOps},        /*  260: PALette (MapOps) */
{0x1000028a, 0x0000010a, 0},             /*  261: SET goes to 266. */

{0x00000040, 0,          MapOps},        /*  262: ASYnc (MapOps) */
{0x000001b9, 0,          MapOps},        /*  263: MODE (MapOps) */
{0x100002be, 0x00000109, 0},             /*  264: SYnc goes to 265. */

{0x10000040, 0,          MapOps},        /*  265: ASYnc (MapOps) */

{0x10000296, 0,          MapOps},        /*  266: SIze (MapOps) */

{0x000001d5, 0x0000010d, 0},             /*  267: NOGuiwarnings goes to 269. */
{0x100002be, 0x00000110, 0},             /*  268: SYnc goes to 272. */

{0x00000040, 0,          RenderOps},     /*  269: ASYnc (RenderOps) */
{0x100002be, 0x0000010f, 0},             /*  270: SYnc goes to 271. */

{0x10000040, 0,          RenderOps},     /*  271: ASYnc (RenderOps) */

{0x10000040, 0,          RenderOps},     /*  272: ASYnc (RenderOps) */

{0x0000010a, 0,          ProjectOps},    /*  273: FILename (ProjectOps) */
{0x10000214, 0,          ProjectOps},    /*  274: PATh (ProjectOps) */

{0x0000010a, 0,          ProjectOps},    /*  275: FILename (ProjectOps) */
{0x10000214, 0,          ProjectOps},    /*  276: PATh (ProjectOps) */

{0x000001d5, 0x00000117, 0},             /*  277: NOGuiwarnings goes to 279. */
{0x100002be, 0x0000011a, 0},             /*  278: SYnc goes to 282. */

{0x00000040, 0,          RenderOps},     /*  279: ASYnc (RenderOps) */
{0x100002be, 0x00000119, 0},             /*  280: SYnc goes to 281. */

{0x10000040, 0,          RenderOps},     /*  281: ASYnc (RenderOps) */

{0x10000040, 0,          RenderOps},     /*  282: ASYnc (RenderOps) */

{0x10000281, 0,          ProjectOps},    /*  283: SCReen (ProjectOps) */

{0x0000010a, 0,          ProjectOps},    /*  284: FILename (ProjectOps) */
{0x10000214, 0,          ProjectOps},    /*  285: PATh (ProjectOps) */

{0x0000010a, 0,          ProjectOps},    /*  286: FILename (ProjectOps) */
{0x10000214, 0,          ProjectOps},    /*  287: PATh (ProjectOps) */

{0x0000016e, 0,          Status},        /*  288: INQuire (Status) */
{0x100001e1, 0,          Status},        /*  289: NOTifyme (Status) */

{0, 0, 0} /* End of table dummy marker. */
};

/* EOF */
