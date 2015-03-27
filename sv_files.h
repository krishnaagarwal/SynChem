#ifndef _H_SV_FILES_
#define _H_SV_FILES_  1
/******************************************************************************
*
*  Copyright (C) 1996 Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     SV_FILES.H
*
*    This is a header contains function prototypes used by the GUI for
*    the managing files. 
*
*
*  Creation Date:
*
*    26-Nov-1996
*
*  Authors:
*
*    Daren Krebsbach
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
*
******************************************************************************/

#include <X11/Intrinsic.h>


#define SVF_QUERY_TYPE_SAVESTAT  "SaveStatus"
#define SVF_QUERY_QERY_SAVESTAT  "Do you wish to save current synthesis space?"
#define SVF_QUERY_OKAY_SAVESTAT  "save"
#define SVF_QUERY_TTLE_SAVESTAT  "Save Status File?"

#define SVF_COMMAND_TYPE_NONE    0
#define SVF_COMMAND_TYPE_EXIT    1
#define SVF_COMMAND_TYPE_EXEC    2
#define SVF_COMMAND_TYPE_OPENSEQ 3
#define SVF_COMMAND_TYPE_OPENPAR 4

#define SVF_FILED_TYPE_OPENSTAT  "OpenStatus"
#define SVF_FILED_TTLE_OPENSTAT  "Open Status File"

#define SVF_FILED_TYPE_PEEKSTAT  "PeekStatus"
#define SVF_FILED_TTLE_PEEKSTAT  "Peek into Status File"

#define SVF_FILED_TYPE_OPENSBMT  "OpenSubmit"
#define SVF_FILED_TTLE_OPENSBMT  "Open Submission File"

#define SVF_FILED_TYPE_OPENDST   "OpenDSTemp"
#define SVF_FILED_TTLE_OPENDST   "Open Draw_Sling_Template File"

#define SVF_FILED_TYPE_OPENPTH   "OpenPathFile"
#define SVF_FILED_TTLE_OPENPTH   "Open Path File"

#define SVF_FILED_TYPE_SAVEPTH   "SavePathFile"
#define SVF_FILED_TTLE_SAVEPTH   "Save Path File"

#define SVF_FILED_TYPE_PATHTRC   "TracePathFile"
#define SVF_FILED_TTLE_PATHTRC   "Trace Path File"

/*** Routine Prototypes ***/

void  SVF_FileDg_Create        (Widget);
void  SVF_FileDg_Update        (char *, XtPointer, Boolean_t);
void  SVF_QueryDg_Create       (Widget);
void  SVF_QueryDg_Update       (char *, U8_t, XtPointer);

void  SVF_StatusFile_Cancel_CB (Widget, XtPointer, XtPointer);
void  SVF_StatusFile_Read_CB   (Widget, XtPointer, XtPointer);
void  SVF_StatusPeek_Cancel_CB (Widget, XtPointer, XtPointer);
void  SVF_StatusPeek_Read_CB   (Widget, XtPointer, XtPointer);
void  SVF_SubmitFile_Cancel_CB (Widget, XtPointer, XtPointer);
void  SVF_SubmitFile_Read_CB   (Widget, XtPointer, XtPointer);
void  SVF_DSTFile_Cancel_CB    (Widget, XtPointer, XtPointer);
void  SVF_DSTFile_Read_CB      (Widget, XtPointer, XtPointer);
void  SVF_PathFile_RdCancel_CB (Widget, XtPointer, XtPointer);
void  SVF_PathFile_Read_CB     (Widget, XtPointer, XtPointer);
void  SVF_PathFile_WrCancel_CB (Widget, XtPointer, XtPointer);
void  SVF_PathFile_Write_CB    (Widget, XtPointer, XtPointer);
void  SVF_PathTrc_Cancel_CB    (Widget, XtPointer, XtPointer);
void  SVF_PathTrc_CB           (Widget, XtPointer, XtPointer);

#endif
/*  End of SV_FILES.H  */
