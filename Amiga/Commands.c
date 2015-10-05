/* Commands.c
** File of (beginnings of) implemented parser commands.
** Copyright 1995 by Questar productions.
*/


/* Local includes */
#include "WCS.h"
#include	"RexxSupport.h"
#include "GrammarTable.h"
#include "Proto.h"
#include "VocabTable.h"

/* Beginning of implemented functions */

int DataBase(struct CmdContext *Call)
{

switch (Call->WordToken[1])
 {
 case VE_LOAD:
  {
  switch (Database_LoadDisp(0, 0, Call->ArgStr, 1))
   {
   case 0:
    {
    sprintf(Call->ArgStr, "Database file loaded: %s", dbasename);
    return(0);
    break;
    }
   case 1:
    {
    strcpy(Call->ArgStr, "Error loading Database");
    break;
    }
   case 2:
    {
    strcpy(Call->ArgStr, "Unsupported Database file format");
    break;
    }
   case 3:
    {
    strcpy(Call->ArgStr, "Error reading Database");
    break;
    }
   case 4:
    {
    strcpy(Call->ArgStr, "Error allocating Database memory");
    break;
    }
   } /* switch */
  return (10);
  break;
  } /* load */
 } /* switch */
DemoFunc(Call, "DataBase");
return(0);
} /* DataBase() */

int ParamIO(struct CmdContext *Call)
{

DemoFunc(Call, "ParamIO");
return(0);
} /* ParamIO */

int KeyOps(struct CmdContext *Call)
{

DemoFunc(Call, "KeyOps");
return(0);
} /* KeyOps() */

int MotionKey(struct CmdContext *Call)
{

DemoFunc(Call, "MotionKey");
strcpy(Call->ArgStr, "123");
return(0);
} /* MotionKey() */

int RenderSet(struct CmdContext *Call)
{

DemoFunc(Call, "RenderSet");
return(0);
} /* RenderSet() */

int ColorKey(struct CmdContext *Call)
{

DemoFunc(Call, "ColorKey");
return(0);
} /* ColorKey() */

int EcoKey(struct CmdContext *Call)
{

DemoFunc(Call, "EcoKey");
return(0);
} /* EcoKey() */

int ViewOps(struct CmdContext *Call)
{

DemoFunc(Call, "ViewOps");
return(0);
} /* ViewOps() */

int ProjectOps(struct CmdContext *Call)
{

DemoFunc(Call, "ProjectOps");
return(0);
} /* ProjectOps() */

int MapOps(struct CmdContext *Call)
{

DemoFunc(Call, "MapOps");
return(0);
} /* MapOps() */

int Quit(struct CmdContext *Call)
{

DemoFunc(Call, "Quit");
return(0);
} /* Quit() */

int RenderOps(struct CmdContext *Call)
{

DemoFunc(Call, "RenderOps");
return(0);
} /* RenderOps() */

int Status(struct CmdContext *Call)
{

DemoFunc(Call, "Status");
strcpy(Call->ArgStr, "Status Good.");
return(0);
} /* Status() */



void DemoFunc(struct CmdContext *Call, char *FromZone)
{
char TextMsg[250];
short i = 0;

while (Call->WordToken[i] && i < 10)
 {
 sprintf(TextMsg, "AREXX: %s -- Token=%d ", FromZone, Call->WordToken[i++]);
 Log(DTA_NULL, TextMsg);
 }

if(strlen(Call->InlineArg))
	{
	sprintf(TextMsg, "     InlineArg=%s", Call->InlineArg);
	Log(DTA_NULL, TextMsg);
	} /* if */
if(strlen(Call->ArgStr))
	{
	sprintf(TextMsg, "     Arg=%s", Call->ArgStr);
	Log(DTA_NULL, TextMsg);
	} /* if */

Call->ArgStr[0] = NULL; /* No return string */

return;
} /* DemoFunc() */

int ImportDEM(struct CmdContext *Call)
{

} /* ImportDEM() */
