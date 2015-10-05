/* CmdCallProtos.h
** Function prototypes (all the same, actually) for the functions that
** get called from the command parser.
** Copyright 1995 by Questar Productions
*/


EXT int DataBase(struct CmdContext *Call);
EXT int ParamIO(struct CmdContext *Call);
EXT int KeyOps(struct CmdContext *Call);
EXT int MotionKey(struct CmdContext *Call);
EXT int RenderSet(struct CmdContext *Call);
EXT int ColorKey(struct CmdContext *Call);
EXT int EcoKey(struct CmdContext *Call);
EXT int ViewOps(struct CmdContext *Call);
EXT int ProjectOps(struct CmdContext *Call);
EXT int MapOps(struct CmdContext *Call);
EXT int Quit(struct CmdContext *Call);
EXT int RenderOps(struct CmdContext *Call);
EXT int Status(struct CmdContext *Call);

