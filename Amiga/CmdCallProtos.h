/* CmdCallProtos.h
** Function prototypes (all the same, actually) for the functions that
** get called from the command parser.
** Copyright 1995 by Questar Productions
*/


EXTERN int DataBase(struct CmdContext *Call);
EXTERN int ParamIO(struct CmdContext *Call);
EXTERN int KeyOps(struct CmdContext *Call);
EXTERN int MotionKey(struct CmdContext *Call);
EXTERN int RenderSet(struct CmdContext *Call);
EXTERN int ColorKey(struct CmdContext *Call);
EXTERN int EcoKey(struct CmdContext *Call);
EXTERN int ViewOps(struct CmdContext *Call);
EXTERN int ProjectOps(struct CmdContext *Call);
EXTERN int MapOps(struct CmdContext *Call);
EXTERN int Quit(struct CmdContext *Call);
EXTERN int RenderOps(struct CmdContext *Call);
EXTERN int Status(struct CmdContext *Call);
EXTERN int ImportDEM(struct CmdContext *Call);

