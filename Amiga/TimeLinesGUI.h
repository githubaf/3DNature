/* TimeLinesGUI.h
** World Construction Set GUI for Time Line Editing modules.
** By Gary R. Huber, 1994.
*/

#ifndef TIMELINESGUI_H
#define TIMELINESGUI_H

/*extern*/ struct Data
{
 short x, y, sx, sy;
 short left, right, top, bottom, textbottom, textzero, lowframe, highframe, 
	 textwidthtop, textwidthbottom, textwidthzero, framegrid,
	 framegridfirst, framegridlg, drawgrid;
 float texthighval, textlowval, valgrid, valgridfirst, framepixgrid,
	 valpixgrid, valpixpt, framepixpt;
 struct KeyTable *SKT;
 short group, activekey, activeitem, dataitems, baseitem;
 long inputflags;
 struct TimeLineWindow *win;
};


#define KEYFRAME_SELECT	(0<<0)
#define KEYFRAME_NEW 	(1<<0)
#define KEYFRAME_MOVE	(1<<1)
#define POINT_SELECTED	(1<<2)
#define QUICK_DRAW	(1<<3)
#define NO_CLEAR	(1<<4)

#include <SDI_compiler.h>

extern SAVEDS ULONG TL_AskMinMax(struct IClass *cl, Object *obj,
     struct MUIP_AskMinMax *msg);
extern SAVEDS ULONG TL_Draw(struct IClass *cl, Object *obj, struct MUIP_Draw *msg);
extern SAVEDS ULONG TL_Setup(struct IClass *cl, Object *obj,
     struct MUIP_HandleInput *msg);
extern SAVEDS ULONG TL_Cleanup(struct IClass *cl, Object *obj,
     struct MUIP_HandleInput *msg);
/*extern SAVEDS ULONG TL_HandleInput(struct IClass *cl, Object *obj,
     struct MUIP_HandleInput *msg);*/ // used locally only -> static, AF 19.7.2021

extern SAVEDS ASM ULONG TL_Dispatcher(REG(a0, struct IClass *cl),
                                      REG(a2, Object *obj),
                                      REG (a1, Msg msg));

//extern SAVEDS ULONG GNTL_HandleInput(struct IClass *cl, Object *obj, struct MUIP_HandleInput *msg); // used locally only -> static, AF 19.7.2021
extern SAVEDS ASM ULONG GNTL_Dispatcher(REG(a0, struct IClass *cl), REG(a2, Object *obj), REG(a1, Msg msg));


#endif /* TIMELINESGUI_H */
