/******************************************************************************
*
*  Copyright (C) 1995, Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     SV_FILES.C
*
*    This manages the GUI interface to files. 
*
*  Creation Date:
*
*    10-Jun-1995
*
*  Authors:
*
*    Daren Krebsbach
*
*  Routines:
*
*    SVF_FileDg_Create
*    SVF_FileDg_Update
*    SVF_QueryDg_Create
*    SVF_QueryDg_Update
*
*    SVF_StatusFile_Cancel_CB
*    SVF_StatusFile_Read_CB
*    SVF_StatusPeek_Cancel_CB
*    SVF_StatusPeek_Read_CB
*    SVF_SubmitFile_Cancel_CB
*    SVF_SubmitFile_Read_CB
*
*    SFileD_Cancel_CB
*    SFileD_Okay_CB
*    SQueryExec_Cancel_CB
*    SQueryExec_Okay_CB
*    SQueryExit_Cancel_CB
*    SQueryExit_Okay_CB
*    SQueryNone_Cancel_CB
*    SQueryNone_Okay_CB
*
*
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 3-May-01   Miller     Added functions for deletion of run files and in the
*                       process, discovered and fixed callbacks that were
*                       removing inappropriate callbacks and leaving themselves
*                       around to be invoked at inappropriate times.
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdarg.h>
#include <sys/types.h>
#include <dirent.h>

#include <Xm/FileSB.h>
#include <Xm/MessageB.h>
#include <Xm/List.h>
#include <Xm/Text.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>

#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_SLING2XTR_
#include "sling2xtr.h"
#endif

#ifndef _H_AVLCOMP_
#include "avlcomp.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_FUNCGROUPS_
#include "funcgroups.h"
#endif

#ifndef _H_PST_
#include "pst.h"
#endif

#ifndef _H_STRATEGY_
#include "strategy.h"
#endif

#ifndef _H_RCB_
#include "rcb.h"
#endif

#ifndef _H_STATUS_
#include "status.h"
#endif

#ifndef _H_SUBMIT_MENU_
#include "submit_menu.h"
#endif

#ifdef SMU_F_SAVE_PB_LBL
#undef SMU_F_SAVE_PB_LBL
#endif

#ifndef _H_SYN_MENU_
#include "syn_menu.h"
#endif

#ifndef _H_APP_RESRC_  
#include "app_resrc.h"  
#endif  
 
#ifndef _H_SEARCH_GUI_
#include "search_gui.h"
#endif

#ifndef _H_SV_FILES_
#include "sv_files.h"
#endif

#ifndef _H_INFO_VIEW_
#include "info_view.h"
#endif

#ifndef _H_PST_VIEW_
#include "pst_view.h"
#endif

#ifndef _H_SYN_VIEW_
#include "syn_view.h"
#endif

#ifndef _H_EXTERN_
#include "extern.h"
#endif

static Widget      SQuerySaveDg;         /* Query save dialog widget */
static Widget      SFileDg;              /* File management dialog widget */
static Widget      SQuerySaveRunLbl;
static Widget      SQSSep;
static Widget      SQuerySaveColLbl[4][2];
static Widget      SQuerySaveRadio[4];
static Widget      SQSPB[2];
static int         init_button_state[2][4] = {{0, 0, 0, 1}, {1, 1, 0, 1}}, button_state[4];
static Boolean_t   delete_flag = FALSE, button_enabled[4] = {TRUE, TRUE, TRUE, TRUE}, autosave_path = FALSE;
static Rcb_t       delrun_rcb, *which_rcb;

/*
static void  SFileD_Cancel_CB (Widget, XtPointer, XtPointer);
static void  SFileD_Okay_CB   (Widget, XtPointer, XtPointer);
*/

static void  SQueryExec_Cancel_CB  (Widget, XtPointer, XtPointer);
static void  SQueryExec_Okay_CB    (Widget, XtPointer, XtPointer);
static void  SQueryExec_Del_CB    (Widget, XtPointer, XtPointer);
static void  SQueryExit_Cancel_CB  (Widget, XtPointer, XtPointer);
static void  SQueryExit_Okay_CB    (Widget, XtPointer, XtPointer);
static void  SQueryExit_Del_CB    (Widget, XtPointer, XtPointer);
static void  SQueryNone_Cancel_CB  (Widget, XtPointer, XtPointer);
static void  SQueryNone_Okay_CB    (Widget, XtPointer, XtPointer);
static void  SQueryNone_Del_CB    (Widget, XtPointer, XtPointer);
static void  SQueryOpenS_Cancel_CB (Widget, XtPointer, XtPointer);
static void  SQueryOpenS_Okay_CB   (Widget, XtPointer, XtPointer);
static void  SQueryOpenS_Del_CB   (Widget, XtPointer, XtPointer);
static void  SQueryOpenP_Cancel_CB (Widget, XtPointer, XtPointer);
static void  SQueryOpenP_Okay_CB   (Widget, XtPointer, XtPointer);
static void  SQueryOpenP_Del_CB   (Widget, XtPointer, XtPointer);
static void  SQueryDelRun_Cancel_CB (Widget, XtPointer, XtPointer);
static void  SQueryDelRun_Okay_CB  (Widget, XtPointer, XtPointer);
static void  SQueryDelRun_Okay2_CB (Widget, XtPointer, XtPointer);
static void  SVF_DeleteFile_Cancel_CB (Widget, XtPointer, XtPointer);
static void  SVF_DeleteFile_Exec_CB (Widget, XtPointer, XtPointer);
static void  SVF_ButtonState_CB (Widget, XtPointer, XtPointer);
static void  SQuerySave_Commit_CB (Widget, XtPointer, XtPointer);
static void  SDelRun_Files (char *, char *, char *, ...);
static char *SFileName_RunName_Extract (char **);
static void  DelSubTrc ();
static void  delfile (int);
static Boolean_t  save_ret_file (int);
static void reconcile_2files (char [3][128]);
static void reconcile_4files (char *);

/****************************************************************************
*
*  Function Name:                 SVF_FileDg_Create
*
*    This routine creates the file manager dialog.
*
*
*  Implicit Inputs:
*
*    SFileDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SVF_FileDg_Create
  (
  Widget    parent 
  )
{

  SFileDg = XmCreateFileSelectionDialog (parent, "SynStatusFSD", NULL, 0);
  XtVaSetValues (SFileDg,
    XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL, 
    NULL);

  return ;
}
/*  End of SVF_FileDg_Create  */

/****************************************************************************
*
*  Function Name:                 SVF_FileDg_Update
*
*    Manage the file dialog with the proper directory and callbacks 
*    based on tag. 
*
*
*  Implicit Inputs:
*
*    SFileDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SVF_FileDg_Update
  (
  char       *tag,
  XtPointer  data,
  Boolean_t  parallel
  )
{
Widget flist,ftext;
  XmString       title;
  XmString       dir;
  XmString       ext;
  XmString       files;
  XmString       selection;
  char           pattern[128], tfilename[50], commmsg[128], *slash, *dot, *ftstr, selstr[128], dirch[128], extch[128], *dirst,
                 pathfilename[100];
  FILE          *f_p, *pf_p;
  int            c, *pos_list, pos_count, pos;
   
  if (strcmp (tag, SVF_FILED_TYPE_OPENSBMT) == 0)
    {
    if (parallel)
      dir = XmStringCreateLtoR (FCB_DISDIR_SBMT (""), XmFONTLIST_DEFAULT_TAG);
    else
      dir = XmStringCreateLtoR (FCB_SEQDIR_SBMT (""), XmFONTLIST_DEFAULT_TAG);

    title = XmStringCreateLtoR (SVF_FILED_TTLE_OPENSBMT, 
      XmFONTLIST_DEFAULT_TAG);
    ext = XmStringCreateLtoR ("*" FCB_EXT_SBMT, XmFONTLIST_DEFAULT_TAG);
    files = XmStringCreateLtoR ("Files", XmFONTLIST_DEFAULT_TAG);
    selection = XmStringCreateLtoR ("Selection", XmFONTLIST_DEFAULT_TAG);

    XtVaSetValues (SFileDg,
      XmNdialogTitle, title, 
      XmNdirectory,   dir, 
      XmNpattern,     ext, 
      XmNfileListLabelString, files,
      XmNselectionLabelString, selection,
      NULL);
    XmStringFree (title);
    XmStringFree (dir);
    XmStringFree (ext);
    XmStringFree (files);
    XmStringFree (selection);
  
    XtAddCallback (SFileDg, XmNcancelCallback, 
      SVF_SubmitFile_Cancel_CB, data);

    XtAddCallback (SFileDg, XmNokCallback, 
      SVF_SubmitFile_Read_CB, data);

    XtManageChild (SFileDg);
    XDefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg), 
      SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_PLUS));
    }

  else if (strcmp (tag, "DeleteRun") == 0)
    {
    if (parallel) strcpy (dirch, FCB_DISDIR_STAT (""));
    else strcpy (dirch, FCB_SEQDIR_STAT (""));
    dirst = strstr (dirch, "status");
    strcpy (dirst, "[p,s,t][f,r,t,u][a,b,i][c,l,m,t][e,i,u]*");
    dir = XmStringCreateLtoR (dirch, XmFONTLIST_DEFAULT_TAG);

    title = XmStringCreateLtoR ("Delete Selected Run", 
      XmFONTLIST_DEFAULT_TAG);
    strcpy (extch, "*.*[p,s,t][a,r,t,u][a,b,t][c,h,m,t]*");
    ext = XmStringCreateLtoR (extch, XmFONTLIST_DEFAULT_TAG);
    files = XmStringCreateLtoR ("Choose any file for the run to delete", XmFONTLIST_DEFAULT_TAG);
    selection = XmStringCreateLtoR ("Status/path/submit/trace file representing run to be deleted", XmFONTLIST_DEFAULT_TAG);

    XtVaSetValues (SFileDg,
      XmNdialogTitle, title, 
      XmNdirectory,   dir, 
      XmNpattern,     ext, 
      XmNfileListLabelString, files,
      XmNselectionLabelString, selection,
      NULL);
    XmStringFree (title);
    XmStringFree (dir);
    XmStringFree (ext);
    XmStringFree (files);
    XmStringFree (selection);
  
    XtAddCallback (SFileDg, XmNcancelCallback, 
      SVF_DeleteFile_Cancel_CB, NULL);

    XtAddCallback (SFileDg, XmNokCallback, 
      SVF_DeleteFile_Exec_CB, NULL);

    XtManageChild (SFileDg);
    XDefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg), 
      SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_PLUS));
    }

  else if (strncmp (tag, SVF_FILED_TYPE_OPENSTAT, strlen(SVF_FILED_TYPE_OPENSTAT)) == 0)
    {
    /*  Check to make sure that the status and pfiles have been saved.
        If not, warn user and ignore exit.
    */
    if (strcmp (tag, SVF_FILED_TYPE_OPENSTAT) == 0 && Rcb_Flags_SaveStatus_Get (&GGoalRcb)
      && RunStats_Flags_PstChanged_Get (&GRunStats))
      {
      if (parallel) SVF_QueryDg_Update (SVF_QUERY_TYPE_SAVESTAT, SVF_COMMAND_TYPE_OPENPAR, data);
      else SVF_QueryDg_Update (SVF_QUERY_TYPE_SAVESTAT, SVF_COMMAND_TYPE_OPENSEQ, data);
      return;
      }

    if (parallel)
      dir = XmStringCreateLtoR (FCB_DISDIR_STAT (""), XmFONTLIST_DEFAULT_TAG);
    else
      dir = XmStringCreateLtoR (FCB_SEQDIR_STAT (""), XmFONTLIST_DEFAULT_TAG);

    title = XmStringCreateLtoR (SVF_FILED_TTLE_OPENSTAT, 
      XmFONTLIST_DEFAULT_TAG);
    ext = XmStringCreateLtoR ("*" FCB_EXT_STAT, XmFONTLIST_DEFAULT_TAG);
    files = XmStringCreateLtoR ("Files", XmFONTLIST_DEFAULT_TAG);
    selection = XmStringCreateLtoR ("Selection", XmFONTLIST_DEFAULT_TAG);

    XtVaSetValues (SFileDg,
      XmNdialogTitle, title, 
      XmNdirectory, dir, 
      XmNpattern, ext, 
      XmNfileListLabelString, files,
      XmNselectionLabelString, selection,
      NULL);
    XmStringFree (title);
    XmStringFree (dir);
    XmStringFree (ext);
    XmStringFree (files);
    XmStringFree (selection);
  
    XtAddCallback (SFileDg, XmNcancelCallback, 
      SVF_StatusFile_Cancel_CB, (XtPointer) NULL);

    XtAddCallback (SFileDg, XmNokCallback, 
      SVF_StatusFile_Read_CB, (XtPointer) NULL);

    XtManageChild (SFileDg);
    XDefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg), 
      SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_PLUS));
    }

  else if (strcmp (tag, SVF_FILED_TYPE_PEEKSTAT) == 0)
    {
    if (parallel)
      dir = XmStringCreateLtoR (FCB_DISDIR_STAT (""), XmFONTLIST_DEFAULT_TAG);
    else
      dir = XmStringCreateLtoR (FCB_SEQDIR_STAT (""), XmFONTLIST_DEFAULT_TAG);

    title = XmStringCreateLtoR (SVF_FILED_TTLE_PEEKSTAT, 
      XmFONTLIST_DEFAULT_TAG);
    ext = XmStringCreateLtoR ("*" FCB_EXT_STAT, XmFONTLIST_DEFAULT_TAG);
    files = XmStringCreateLtoR ("Files", XmFONTLIST_DEFAULT_TAG);
    selection = XmStringCreateLtoR ("Selection", XmFONTLIST_DEFAULT_TAG);

    XtVaSetValues (SFileDg,
      XmNdialogTitle, title, 
      XmNdirectory, dir, 
      XmNpattern, ext, 
      XmNfileListLabelString, files,
      XmNselectionLabelString, selection,
      NULL);
    XmStringFree (title);
    XmStringFree (dir);
    XmStringFree (ext);
    XmStringFree (files);
    XmStringFree (selection);
  
    XtAddCallback (SFileDg, XmNcancelCallback, 
      SVF_StatusPeek_Cancel_CB, data);

    XtAddCallback (SFileDg, XmNokCallback, 
      SVF_StatusPeek_Read_CB, data);

    XtManageChild (SFileDg);
    XDefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg), 
      SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_PLUS));
    }

  else if (strcmp (tag, SVF_FILED_TYPE_OPENDST) == 0)
    {
    if (parallel)
      dir = XmStringCreateLtoR (FCB_DISDIR_USR ("/draw_sling_template"), XmFONTLIST_DEFAULT_TAG);
    else
      dir = XmStringCreateLtoR (FCB_SEQDIR_USR ("/draw_sling_template"), XmFONTLIST_DEFAULT_TAG);

    title = XmStringCreateLtoR (SVF_FILED_TTLE_OPENDST, 
      XmFONTLIST_DEFAULT_TAG);
    ext = XmStringCreateLtoR ("*.txt", XmFONTLIST_DEFAULT_TAG);
    files = XmStringCreateLtoR ("Files", XmFONTLIST_DEFAULT_TAG);
    selection = XmStringCreateLtoR ("Selection", XmFONTLIST_DEFAULT_TAG);

    XtVaSetValues (SFileDg,
      XmNdialogTitle, title, 
      XmNdirectory, dir, 
      XmNpattern, ext, 
      XmNfileListLabelString, files,
      XmNselectionLabelString, selection,
      NULL);
    XmStringFree (title);
    XmStringFree (dir);
    XmStringFree (ext);
    XmStringFree (files);
    XmStringFree (selection);
  
    XtAddCallback (SFileDg, XmNcancelCallback, 
      SVF_DSTFile_Cancel_CB, data);

    XtAddCallback (SFileDg, XmNokCallback, 
      SVF_DSTFile_Read_CB, data);

    XtManageChild (SFileDg);
    XDefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg), 
      SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_PLUS));
    }

  else if (strcmp (tag, SVF_FILED_TYPE_PATHTRC) == 0)
    {
    if (parallel)
      dir = XmStringCreateLtoR (FCB_DISDIR_USR ("/pfile"), XmFONTLIST_DEFAULT_TAG);
    else
      dir = XmStringCreateLtoR (FCB_SEQDIR_USR ("/pfile"), XmFONTLIST_DEFAULT_TAG);

    title = XmStringCreateLtoR (SVF_FILED_TTLE_PATHTRC, 
      XmFONTLIST_DEFAULT_TAG);

/*
    ext = XmStringCreateLtoR ("*.path", XmFONTLIST_DEFAULT_TAG);
*/
    strcpy (pattern,
      (char *) String_Value_Get (FileCB_FileStr_Get (
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_PATH))));
    while ((slash = strstr (pattern, "/")) != NULL) strcpy (pattern, slash + 1);
/*
    if ((dot = strstr (pattern, ".path")) != NULL) strcpy (dot, "*.path");
*/
    if ((dot = strstr (pattern, ".path")) != NULL) strcpy (dot, ".*path"); /* force '.' after runid */
    else strcat (pattern, "*");
    ext = XmStringCreateLtoR (pattern, XmFONTLIST_DEFAULT_TAG);
    files = XmStringCreateLtoR ("Files", XmFONTLIST_DEFAULT_TAG);
    selection = XmStringCreateLtoR ("Selection", XmFONTLIST_DEFAULT_TAG);

    XtVaSetValues (SFileDg,
      XmNdialogTitle, title, 
      XmNdirectory, dir, 
      XmNpattern, ext, 
      XmNfileListLabelString, files,
      XmNselectionLabelString, selection,
      NULL);
    XmStringFree (title);
    XmStringFree (dir);
    XmStringFree (ext);
    XmStringFree (files);
    XmStringFree (selection);
  
    XtAddCallback (SFileDg, XmNcancelCallback, 
      SVF_PathTrc_Cancel_CB, data);

    XtAddCallback (SFileDg, XmNokCallback, 
      SVF_PathTrc_CB, data);

    XtManageChild (SFileDg);
flist=XmFileSelectionBoxGetChild(SFileDg,XmDIALOG_LIST);
XmListSelectPos(flist,0,True);
if (!XmListGetSelectedPos(flist, &pos_list, &pos_count) || pos_count != 1) pos = 0;
else pos = pos_list[0];
XtFree ((char *) pos_list);
if (pos != 1)
{
  ftext=XmFileSelectionBoxGetChild(SFileDg,XmDIALOG_TEXT);
  ftstr=XmTextGetString(ftext);
  XmListDeselectPos(flist,0);
  XmTextReplace(ftext,0,strlen(ftstr),parallel?FCB_DISDIR_USR ("/pfile/"):FCB_SEQDIR_USR ("/pfile/"));
  XtFree(ftstr);
}
    XDefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg), 
      SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_PLUS));
    }

  else if (strcmp (tag, SVF_FILED_TYPE_OPENPTH) == 0)
    {
    if (parallel)
      dir = XmStringCreateLtoR (FCB_DISDIR_USR ("/pfile"), XmFONTLIST_DEFAULT_TAG);
    else
      dir = XmStringCreateLtoR (FCB_SEQDIR_USR ("/pfile"), XmFONTLIST_DEFAULT_TAG);

    title = XmStringCreateLtoR (SVF_FILED_TTLE_OPENPTH, 
      XmFONTLIST_DEFAULT_TAG);
/*
    ext = XmStringCreateLtoR ("*.path", XmFONTLIST_DEFAULT_TAG);
*/
    strcpy (pattern,
      (char *) String_Value_Get (FileCB_FileStr_Get (
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_PATH))));
    while ((slash = strstr (pattern, "/")) != NULL) strcpy (pattern, slash + 1);
/*
    if ((dot = strstr (pattern, ".path")) != NULL) strcpy (dot, "*.path");
*/
    if ((dot = strstr (pattern, ".path")) != NULL) strcpy (dot, ".*path"); /* force '.' after runid */
    else strcat (pattern, "*");
    ext = XmStringCreateLtoR (pattern, XmFONTLIST_DEFAULT_TAG);
    files = XmStringCreateLtoR ("Files", XmFONTLIST_DEFAULT_TAG);
    selection = XmStringCreateLtoR ("Selection", XmFONTLIST_DEFAULT_TAG);

    XtVaSetValues (SFileDg,
      XmNdialogTitle, title, 
      XmNdirectory, dir, 
      XmNpattern, ext, 
      XmNfileListLabelString, files,
      XmNselectionLabelString, selection,
      NULL);
    XmStringFree (title);
    XmStringFree (dir);
    XmStringFree (ext);
    XmStringFree (files);
    XmStringFree (selection);
  
    XtAddCallback (SFileDg, XmNcancelCallback, 
      SVF_PathFile_RdCancel_CB, data);

    XtAddCallback (SFileDg, XmNokCallback, 
      SVF_PathFile_Read_CB, data);

    XtManageChild (SFileDg);
flist=XmFileSelectionBoxGetChild(SFileDg,XmDIALOG_LIST);
XmListSelectPos(flist,0,True);
if (!XmListGetSelectedPos(flist, &pos_list, &pos_count) || pos_count != 1) pos = 0;
else pos = pos_list[0];
XtFree ((char *) pos_list);
if (pos != 1)
{
  ftext=XmFileSelectionBoxGetChild(SFileDg,XmDIALOG_TEXT);
  ftstr=XmTextGetString(ftext);
  XmListDeselectPos(flist,0);
  XmTextReplace(ftext,0,strlen(ftstr),parallel?FCB_DISDIR_USR ("/pfile/"):FCB_SEQDIR_USR ("/pfile/"));
  XtFree(ftstr);
}
    XDefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg), 
      SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_PLUS));
    }

  else if (strcmp (tag, SVF_FILED_TYPE_SAVEPTH) == 0)
    {
#ifdef _WIN32
    sprintf (tfilename, "temp.pth.%s", session_code);
#else
#ifdef _INTERIX_
    sprintf (tfilename, "temp.pth.%s", session_code);
#endif
    sprintf (tfilename, "/tmp/temp.pth.%s", session_code);
#endif
    if ((f_p = fopen (tfilename, "r")) == NULL)
    {
      sprintf (commmsg, "Unable to open %s!", tfilename);
      InfoWarn_Show (commmsg);
      return;
    }

    c = getc (f_p);
    fclose (f_p);

    if (c == EOF)
    {
      sprintf (commmsg, "rm %s", tfilename);
/*
      system (commmsg);
*/
      remove(commmsg + 3);
      InfoWarn_Show ("No path marked - unable to save!");
      return;
    }

    if (autosave_path)
    {
      if (parallel) strcpy (pathfilename, FCB_DISDIR_USR ("/pfile"));
      else strcpy (pathfilename, FCB_SEQDIR_USR ("/pfile"));
      strcat (pathfilename, "/");
      strcpy (pattern, (char *) String_Value_Get (FileCB_FileStr_Get (Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_PATH))));
      while ((slash = strstr (pattern, "/")) != NULL) strcpy (pattern, slash + 1);
      if ((dot = strstr (pattern, ".path")) != NULL) sprintf (dot, ".%s.path", session_code);
      else
      {
        InfoWarn_Show ("Unable to construct a valid path filename.");
        return;
      }
      strcat (pathfilename, pattern);
      pf_p = fopen (pathfilename, "w");
      if (pf_p == NULL)
      {
        sprintf (commmsg, "Unable to open file %s.", pathfilename);
        InfoWarn_Show (commmsg);
        return;
      }
      f_p = fopen (tfilename, "r");
      while ((c = getc (f_p)) != EOF) putc (c, pf_p);
      fclose (f_p);
      fclose (pf_p);
      return;
    }

    if (parallel)
      dir = XmStringCreateLtoR (FCB_DISDIR_USR ("/pfile"), XmFONTLIST_DEFAULT_TAG);
    else
      dir = XmStringCreateLtoR (FCB_SEQDIR_USR ("/pfile"), XmFONTLIST_DEFAULT_TAG);

    title = XmStringCreateLtoR (SVF_FILED_TTLE_SAVEPTH, 
      XmFONTLIST_DEFAULT_TAG);
/*
    ext = XmStringCreateLtoR ("*.path", XmFONTLIST_DEFAULT_TAG);
*/
    strcpy (pattern,
      (char *) String_Value_Get (FileCB_FileStr_Get (
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_PATH))));
    while ((slash = strstr (pattern, "/")) != NULL) strcpy (pattern, slash + 1);
/*
    if ((dot = strstr (pattern, ".path")) != NULL) strcpy (dot, "*.path");
*/
    if ((dot = strstr (pattern, ".path")) != NULL) strcpy (dot, ".*path"); /* force '.' after runid */
    else strcat (pattern, "*");
    ext = XmStringCreateLtoR (pattern, XmFONTLIST_DEFAULT_TAG);
    files = XmStringCreateLtoR ("Files", XmFONTLIST_DEFAULT_TAG);
    selection = XmStringCreateLtoR ("Selection", XmFONTLIST_DEFAULT_TAG);

    XtVaSetValues (SFileDg,
      XmNdialogTitle, title, 
      XmNdirectory, dir, 
      XmNpattern, ext, 
      XmNfileListLabelString, files,
      XmNselectionLabelString, selection,
      NULL);
    XmStringFree (title);
    XmStringFree (dir);
    XmStringFree (ext);
    XmStringFree (files);
    XmStringFree (selection);
  
    XtAddCallback (SFileDg, XmNcancelCallback, 
      SVF_PathFile_WrCancel_CB, data);

    XtAddCallback (SFileDg, XmNokCallback, 
      SVF_PathFile_Write_CB, data);

    XtManageChild (SFileDg);
flist=XmFileSelectionBoxGetChild(SFileDg,XmDIALOG_LIST);
XmListSelectPos(flist,1,True);
ftext=XmFileSelectionBoxGetChild(SFileDg,XmDIALOG_TEXT);
ftstr=XmTextGetString(ftext);
XmListDeselectPos(flist,1);
if (strcmp(ftstr,parallel?FCB_DISDIR_USR ("/pfile/"):FCB_SEQDIR_USR ("/pfile/")) == 0)
{
  strcpy (pattern,
    (char *) String_Value_Get (FileCB_FileStr_Get (
    Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_PATH))));
  while ((slash = strstr (pattern, "/")) != NULL) strcpy (pattern, slash + 1);
  sprintf (selstr, "%s%s", parallel?FCB_DISDIR_USR ("/pfile/"):FCB_SEQDIR_USR ("/pfile/"), pattern);
  XmTextReplace(ftext,0,strlen(ftstr),selstr);
}
else XmTextReplace(ftext,0,strlen(ftstr),parallel?FCB_DISDIR_USR ("/pfile/"):FCB_SEQDIR_USR ("/pfile/"));
XtFree(ftstr);
    XDefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg), 
      SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_PLUS));
    }

  return ;
}
/*  End of SVF_FileDg_Update  */

/****************************************************************************
*
*  Function Name:                 SVF_QueryDg_Create
*
*    This routine creates the general query dialog. 
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SVF_QueryDg_Create
  (
  Widget    parent
  )
{
  Widget    child;
  char wname[24];
  XmString title, savelbl, dellbl, nulllbl, collbl[4][2], runname;
  int i, pos, pixel;

/*
  SQuerySaveDg = XmCreateQuestionDialog (parent, "SynQuerySaveD", NULL, 0);
*/
  SQuerySaveDg = XmCreateFormDialog (parent, "SynQuerySaveD", NULL, 0);
  title = XmStringCreateLocalized("File Dispostion");
  XtVaSetValues (SQuerySaveDg,
    XmNdialogTitle, title,
    XmNwidth, 360,
    XmNallowResize, False,
    NULL);
  XtVaGetValues (SQuerySaveDg,
    XmNbackground, &pixel,
    NULL);
  XmStringFree (title);

  runname = XmStringCreateLocalized("<No Run Specified>");
  SQuerySaveRunLbl = XmCreateLabelGadget (SQuerySaveDg, "SQSDRunLbl", NULL, 0);
  XtVaSetValues (SQuerySaveRunLbl,
    XmNlabelString, runname,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    NULL);
  XmStringFree (runname);
  XtManageChild (SQuerySaveRunLbl);

  SQSSep = XtVaCreateManagedWidget ("SQSSep",
    xmSeparatorGadgetClass, SQuerySaveDg,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, SQuerySaveRunLbl,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget, SQuerySaveRunLbl,
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNrightWidget, SQuerySaveRunLbl,
    NULL);

  savelbl = XmStringCreateLocalized("     save/retain");
  dellbl = XmStringCreateLocalized("     delete/no save");
  nulllbl = XmStringCreateLocalized("\210");
  collbl[0][0] = XmStringCreateLocalized("status    ");
  collbl[0][1] = XmStringCreateLocalized("------");
  collbl[1][0] = XmStringCreateLocalized("path      ");
  collbl[1][1] = XmStringCreateLocalized("(all)");
  collbl[2][0] = XmStringCreateLocalized("submit    ");
  collbl[2][1] = XmStringCreateLocalized("------");
  collbl[3][0] = XmStringCreateLocalized("trace");
  collbl[3][1] = XmStringCreateLocalized("-----");
  for (i=0; i<4; i++)
  {
    sprintf (wname, "SQSDCLbl0_%d",i);
    SQuerySaveColLbl[i][0] = XmCreateLabelGadget (SQuerySaveDg, wname, NULL, 0);
    XtVaSetValues (SQuerySaveColLbl[i][0],
      XmNlabelString, collbl[i][0],
      XmNtopOffset, 0,
      XmNtopAttachment, i == 0 ? XmATTACH_WIDGET : XmATTACH_OPPOSITE_WIDGET,
      XmNtopWidget, i == 0 ? SQSSep : SQuerySaveColLbl[i - 1][0],
      XmNleftOffset, 0,
      XmNleftAttachment, i == 0 ? XmATTACH_FORM : XmATTACH_WIDGET,
      XmNleftWidget, i == 0 ? NULL : SQuerySaveColLbl[i - 1][0],
      XmNbackground, pixel,
      NULL);
    XmStringFree (collbl[i][0]);
    XtManageChild(SQuerySaveColLbl[i][0]);

    sprintf (wname, "SQSDCLbl1_%d",i);
    SQuerySaveColLbl[i][1] = XmCreateLabelGadget (SQuerySaveDg, wname, NULL, 0);
    XtVaSetValues (SQuerySaveColLbl[i][1],
      XmNlabelString, collbl[i][1],
      XmNtopOffset, 0,
      XmNtopAttachment, XmATTACH_WIDGET,
      XmNtopWidget, SQuerySaveColLbl[i][0],
      XmNleftOffset, 0,
      XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
      XmNleftWidget, SQuerySaveColLbl[i][0],
      XmNbackground, pixel,
      NULL);
    XmStringFree (collbl[i][1]);
    XtManageChild(SQuerySaveColLbl[i][1]);

    sprintf (wname, "SQSDRadio_%d",i);
/*
    SQuerySaveRadio[i] = XmVaCreateSimpleRadioBox (SQuerySaveDg, wname, init_button_state[0][i], SVF_ButtonState_CB,
      XmVaRADIOBUTTON, i == 3 ? savelbl : nulllbl, NULL, NULL, NULL,
      XmVaRADIOBUTTON, i == 3 ? dellbl : nulllbl, NULL, NULL, NULL,
*/
    SQuerySaveRadio[i] = XmVaCreateSimpleCheckBox (SQuerySaveDg, wname, SVF_ButtonState_CB,
      XmVaCHECKBUTTON, i == 3 ? savelbl : nulllbl, NULL, NULL, NULL,
      XmVaCHECKBUTTON, i == 3 ? dellbl : nulllbl, NULL, NULL, NULL,
      XmNtopOffset, 0,
      XmNtopAttachment, XmATTACH_WIDGET,
      XmNtopWidget, SQuerySaveColLbl[i][1],
      XmNleftOffset, 0,
      XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
      XmNleftWidget, SQuerySaveColLbl[i][1],
      XmNbackground, pixel,
      NULL);
    XtManageChild(SQuerySaveRadio[i]);
  }
  XmStringFree(nulllbl);
  XmStringFree(savelbl);
  XmStringFree(dellbl);
/*
  child = XmMessageBoxGetChild (SQuerySaveDg, XmDIALOG_HELP_BUTTON);
  XtUnmanageChild (child);
*/
  SQSPB[0] = XmCreatePushButtonGadget (SQuerySaveDg, "CommitPB", NULL, 0);
  title = XmStringCreateLocalized(" Dispose Files ");
  XtVaSetValues (SQSPB[0],
    XmNlabelString, title,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, SQuerySaveRadio[0],
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightPosition, 40,
    XmNrightAttachment, XmATTACH_POSITION,
    XmNbackground, pixel,
    XmNallowResize, False,
    NULL);
  XmStringFree (title);
/*
  XtAddCallback (SQuerySaveDg, XmNactivateCallback, SQuerySave_Commit_CB, NULL);
*/
  XtManageChild(SQSPB[0]);
  SQSPB[1] = XmCreatePushButtonGadget (SQuerySaveDg, "CancelPB", NULL, 0);
  title = XmStringCreateLocalized(" Bypass File Disposition ");
  XtVaSetValues (SQSPB[1],
    XmNlabelString, title,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, SQuerySaveRadio[3],
    XmNleftPosition, 50,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    XmNbackground, pixel,
    XmNallowResize, False,
    NULL);
  XmStringFree (title);
  XtManageChild(SQSPB[1]);

  return ;
}
/*  End of SVF_QueryDg_Create  */

/****************************************************************************
*
*  Function Name:                 SVF_QueryDg_Update
*
*    Display the given query and add the proper callbacks based on tag. 
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SVF_QueryDg_Update
  (
  char       *tag,
  U8_t        command,
  XtPointer   data
  )
{
  XmString       mess_str;                  /* Message string */
  XmString       ok_str;                    /* Okay pb string */
  XmString       ok2_str;                   /* Okay pb string for retaining submit file */
  XmString       can_str;                   /* More meaningful action than default "Cancel" */
  XmString       title_str;                 /* Dialog title string */
  XmString       retdellbl[2], collbl[4][2];
  XmString       runname;
  Widget         child;                     /* Need a 3-way dialog for deletion */
  char           message[256], bname[12], *filespec, fspec[128], filename[32];
  int i, j;
  DIR           *dir;
  struct dirent *dir_entry;
 
  delete_flag = FALSE;

  strcpy (fspec, String_Value_Get (FileCB_FileStr_Get (Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_STAT))));
  filespec = fspec;
  strcpy (filename, SFileName_RunName_Extract (&filespec));
  runname = XmStringCreateLocalized (filename);
  XtVaSetValues (SQuerySaveRunLbl,
    XmNlabelString, runname,
    NULL);
  XmStringFree (runname);

  collbl[0][0] = XmStringCreateLocalized("status    ");
  collbl[0][1] = XmStringCreateLocalized("------");
  collbl[1][0] = XmStringCreateLocalized("path      ");
  collbl[1][1] = XmStringCreateLocalized("(all)");
  collbl[2][0] = XmStringCreateLocalized("submit    ");
  collbl[2][1] = XmStringCreateLocalized("------");
  collbl[3][0] = XmStringCreateLocalized("trace");
  collbl[3][1] = XmStringCreateLocalized("-----");
  for (i = 0; i < 4; i++)
    {
    for (j = 0; j < 2; j++)
      {
      XtVaSetValues (SQuerySaveColLbl[i][j],
        XmNlabelString, collbl[i][j],
        NULL);
      XmStringFree (collbl[i][j]);
      }
    button_enabled[i] = TRUE;
    XtVaSetValues (SQuerySaveRadio[i],
      XmNsensitive, True,
      NULL);
    }

  if (strcmp (tag, SVF_QUERY_TYPE_SAVESTAT) == 0)
    {
    retdellbl[0] = XmStringCreateLocalized("     save/retain");
    retdellbl[1] = XmStringCreateLocalized("     delete/no save");
    for (i = 0; i < 4; i++)
      {
      button_state[i] = init_button_state[0][i];
      sprintf (bname, "button_%d", 1 - button_state[i]);
      child = XtNameToWidget (SQuerySaveRadio[i], bname);
      XtVaSetValues (child,
        XmNset, False,
        NULL);
      sprintf (bname, "button_%d", button_state[i]);
      child = XtNameToWidget (SQuerySaveRadio[i], bname);
      XtVaSetValues (child,
        XmNset, True,
        NULL);
      }
    for (i=0; i<2; i++)
      {
      sprintf (bname, "button_%d", i);
      child = XtNameToWidget (SQuerySaveRadio[3], bname);
      XtVaSetValues (child,
        XmNlabelString, retdellbl[i],
        NULL);
      XmStringFree (retdellbl[i]);
      }
/*
    mess_str = XmStringCreateLtoR (SVF_QUERY_QERY_SAVESTAT, 
      XmFONTLIST_DEFAULT_TAG);
    ok_str = XmStringCreateLtoR (SVF_QUERY_OKAY_SAVESTAT, 
      XmFONTLIST_DEFAULT_TAG);
    ok2_str = XmStringCreateLtoR ("No save; delete submit and/or trace",
      XmFONTLIST_DEFAULT_TAG);
    can_str = XmStringCreateLtoR ("No save, but retain submit and/or trace",
      XmFONTLIST_DEFAULT_TAG);
    title_str = XmStringCreateLtoR (SVF_QUERY_TTLE_SAVESTAT,  
      XmFONTLIST_DEFAULT_TAG);
    XtVaSetValues (SQuerySaveDg,
      XmNdialogTitle, title_str, 
      XmNdefaultPosition, True,
      XmNokLabelString, ok_str,
      XmNhelpLabelString, ok2_str,
      XmNcancelLabelString, can_str,
      XmNmessageString, mess_str,
      XmNdefaultButtonType, XmDIALOG_OK_BUTTON,
      NULL);
    XmStringFree (mess_str);
    XmStringFree (ok_str);
    XmStringFree (ok2_str);
    XmStringFree (can_str);
    XmStringFree (title_str);
*/
    if (command == SVF_COMMAND_TYPE_NONE)
      {
      XtAddCallback (SQSPB[0], XmNactivateCallback, 
        SQueryNone_Okay_CB, NULL);
      XtAddCallback (SQSPB[1], XmNactivateCallback, 
        SQueryNone_Cancel_CB, NULL);
      }
    else if (command == SVF_COMMAND_TYPE_EXEC)
      {
      XtAddCallback (SQSPB[0], XmNactivateCallback, 
        SQueryExec_Okay_CB, NULL);
      XtAddCallback (SQSPB[1], XmNactivateCallback, 
        SQueryExec_Cancel_CB, NULL);
      }
    else if (command == SVF_COMMAND_TYPE_EXIT)
      {
      XtAddCallback (SQSPB[0], XmNactivateCallback, 
        SQueryExit_Okay_CB, NULL);
      XtAddCallback (SQSPB[1], XmNactivateCallback, 
        SQueryExit_Cancel_CB, NULL);
      }
    else if (command == SVF_COMMAND_TYPE_OPENSEQ)
      {
      XtAddCallback (SQSPB[0], XmNactivateCallback, 
        SQueryOpenS_Okay_CB, NULL);
      XtAddCallback (SQSPB[1], XmNactivateCallback, 
        SQueryOpenS_Cancel_CB, NULL);
      }
    else if (command == SVF_COMMAND_TYPE_OPENPAR)
      {
      XtAddCallback (SQSPB[0], XmNactivateCallback, 
        SQueryOpenP_Okay_CB, NULL);
      XtAddCallback (SQSPB[1], XmNactivateCallback, 
        SQueryOpenP_Cancel_CB, NULL);
      }
    XtManageChild (SQuerySaveDg);
    }
/*
    if (command == SVF_COMMAND_TYPE_NONE)
      {
      XtAddCallback (SQuerySaveDg, XmNcancelCallback, 
        SQueryNone_Cancel_CB, NULL);

      XtAddCallback (SQuerySaveDg, XmNhelpCallback, 
        SQueryNone_Del_CB, NULL);

      XtAddCallback (SQuerySaveDg, XmNokCallback, 
        SQueryNone_Okay_CB, NULL);
      }
    else if (command == SVF_COMMAND_TYPE_EXEC)
      {
      XtAddCallback (SQuerySaveDg, XmNcancelCallback, 
        SQueryExec_Cancel_CB, data);

      XtAddCallback (SQuerySaveDg, XmNhelpCallback, 
        SQueryExec_Del_CB, NULL);

      XtAddCallback (SQuerySaveDg, XmNokCallback, 
        SQueryExec_Okay_CB, data);
      }
    else if (command == SVF_COMMAND_TYPE_EXIT)
      {
      XtAddCallback (SQuerySaveDg, XmNcancelCallback, 
        SQueryExit_Cancel_CB, NULL);

      XtAddCallback (SQuerySaveDg, XmNhelpCallback, 
        SQueryExit_Del_CB, NULL);

      XtAddCallback (SQuerySaveDg, XmNokCallback, 
        SQueryExit_Okay_CB, NULL);
      }
    else if (command == SVF_COMMAND_TYPE_OPENSEQ)
      {
      XtAddCallback (SQuerySaveDg, XmNcancelCallback, 
        SQueryOpenS_Cancel_CB, data);

      XtAddCallback (SQuerySaveDg, XmNhelpCallback, 
        SQueryOpenS_Del_CB, NULL);

      XtAddCallback (SQuerySaveDg, XmNokCallback, 
        SQueryOpenS_Okay_CB, data);
      }
    else if (command == SVF_COMMAND_TYPE_OPENPAR)
      {
      XtAddCallback (SQuerySaveDg, XmNcancelCallback, 
        SQueryOpenP_Cancel_CB, data);

      XtAddCallback (SQuerySaveDg, XmNhelpCallback, 
        SQueryOpenP_Del_CB, NULL);

      XtAddCallback (SQuerySaveDg, XmNokCallback, 
        SQueryOpenP_Okay_CB, data);
      }

    XtManageChild (SQuerySaveDg);
    }
*/

  else if (strcmp (tag, "DeleteRun") == 0)
    {
    delete_flag = TRUE;
    reconcile_4files (*(char **) data);
    retdellbl[0] = XmStringCreateLocalized("     retain");
    retdellbl[1] = XmStringCreateLocalized("     delete");
    strcpy (filename, SFileName_RunName_Extract ((char **) data));
    runname = XmStringCreateLocalized (filename);
    XtVaSetValues (SQuerySaveRunLbl,
      XmNlabelString, runname,
      NULL);
    XmStringFree (runname);

    strcat (filename, ".");

    dir = opendir (String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (&delrun_rcb, FCB_IND_STAT))));
    while ((dir_entry = readdir (dir)) != NULL && strncmp (dir_entry->d_name, filename, strlen (filename)));
    closedir (dir);
    if (dir_entry == NULL)
    {
      XtVaSetValues (SQuerySaveRadio[0],
        XmNsensitive, False,
        NULL);
      button_enabled[0] = FALSE;
    }

    dir = opendir (String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (&delrun_rcb, FCB_IND_PATH))));
    while ((dir_entry = readdir (dir)) != NULL && strncmp (dir_entry->d_name, filename, strlen (filename)));
    closedir (dir);
    if (dir_entry == NULL)
    {
      XtVaSetValues (SQuerySaveRadio[1],
        XmNsensitive, False,
        NULL);
      button_enabled[1] = FALSE;
    }

    dir = opendir (String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (&delrun_rcb, FCB_IND_SBMT))));
    while ((dir_entry = readdir (dir)) != NULL && strncmp (dir_entry->d_name, filename, strlen (filename)));
    closedir (dir);
    if (dir_entry == NULL)
    {
      XtVaSetValues (SQuerySaveRadio[2],
        XmNsensitive, False,
        NULL);
      button_enabled[2] = FALSE;
    }

    dir = opendir (String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (&delrun_rcb, FCB_IND_TRCE))));
    while ((dir_entry = readdir (dir)) != NULL && strncmp (dir_entry->d_name, filename, strlen (filename)));
    closedir (dir);
    if (dir_entry == NULL)
    {
      XtVaSetValues (SQuerySaveRadio[3],
        XmNsensitive, False,
        NULL);
      button_enabled[3] = FALSE;
    }

    collbl[0][0] = XmStringCreateLocalized("[status]  ");
    collbl[0][1] = XmStringCreateLocalized(" ------");
    collbl[1][0] = XmStringCreateLocalized("[path]    ");
    collbl[1][1] = XmStringCreateLocalized(" ----");
    collbl[2][0] = XmStringCreateLocalized("[submit]  ");
    collbl[2][1] = XmStringCreateLocalized(" ------");
    collbl[3][0] = XmStringCreateLocalized("[trace]");
    collbl[3][1] = XmStringCreateLocalized(" -----");
    for (i = 0; i < 4; i++)
      {
      for (j = 0; j < 2; j++)
        {
        if (!button_enabled[i])
          {
          XtVaSetValues (SQuerySaveColLbl[i][j],
            XmNlabelString, collbl[i][j],
            NULL);
          }
        XmStringFree (collbl[i][j]);
        }
      button_state[i] = init_button_state[1][i];
      sprintf (bname, "button_%d", 1 - button_state[i]);
      XtVaSetValues (XtNameToWidget (SQuerySaveRadio[i], bname),
        XmNset, False,
        NULL);
      sprintf (bname, "button_%d", button_state[i]);
      XtVaSetValues (XtNameToWidget (SQuerySaveRadio[i], bname),
        XmNset, button_enabled[i] ? True : False,
        NULL);
      }
    for (i=0; i<2; i++)
      {
      sprintf (bname, "button_%d", i);
      child = XtNameToWidget (SQuerySaveRadio[3], bname);
      XtVaSetValues (child,
        XmNlabelString, retdellbl[i],
        NULL);
      XmStringFree (retdellbl[i]);
      }
/*
    child = XmMessageBoxGetChild (SQuerySaveDg, XmDIALOG_HELP_BUTTON);
    XtManageChild (child);
*/
/*
    sprintf (message, "Are you sure you want to delete all files for \"%s\"?",
      SFileName_RunName_Extract ((char **) data));
    mess_str = XmStringCreateLtoR (message,
      XmFONTLIST_DEFAULT_TAG);
    ok_str = XmStringCreateLtoR ("Yes, delete all", 
      XmFONTLIST_DEFAULT_TAG);
    ok2_str = XmStringCreateLtoR ("Yes, but retain submit file (if any)", 
      XmFONTLIST_DEFAULT_TAG);
    can_str = XmStringCreateLtoR ("No, cancel deletion",
      XmFONTLIST_DEFAULT_TAG);
    title_str = XmStringCreateLtoR (SVF_QUERY_TTLE_SAVESTAT,  
      XmFONTLIST_DEFAULT_TAG);
    XtVaSetValues (SQuerySaveDg,
      XmNdialogTitle, title_str, 
      XmNdefaultPosition, True,
      XmNokLabelString, ok_str,
      XmNhelpLabelString, ok2_str,
      XmNcancelLabelString, can_str,
      XmNmessageString, mess_str,
      XmNdefaultButtonType, XmDIALOG_OK_BUTTON,
      NULL);
    XmStringFree (mess_str);
    XmStringFree (ok_str);
    XmStringFree (ok2_str);
    XmStringFree (title_str);

    XtAddCallback (SQuerySaveDg, XmNcancelCallback, 
      SQueryDelRun_Cancel_CB, data);

    XtAddCallback (SQuerySaveDg, XmNokCallback, 
      SQueryDelRun_Okay_CB, data);

    XtAddCallback (SQuerySaveDg, XmNhelpCallback, 
      SQueryDelRun_Okay2_CB, data);
*/
    XtAddCallback (SQSPB[0], XmNactivateCallback, 
      SQueryDelRun_Okay_CB, NULL);
    XtAddCallback (SQSPB[1], XmNactivateCallback, 
      SQueryDelRun_Cancel_CB, NULL);

    XtManageChild (SQuerySaveDg);
    }

  return ;
}
/*  End of SVF_QueryDg_Update  */

/****************************************************************************
*
*  Function Name:                 SVF_StatusFile_Cancel_CB
*
*    This routine removes the callbacks for the file manager after 
*    unmanaging it.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SVF_StatusFile_Cancel_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
 
  XtUnmanageChild (SFileDg);
  XUndefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg));
  XtRemoveCallback (SFileDg, XmNcancelCallback, SVF_StatusFile_Cancel_CB, 
    (XtPointer) NULL);
  XtRemoveCallback (SFileDg, XmNokCallback, SVF_StatusFile_Read_CB, 
    (XtPointer) NULL);


  return ;
}
/*  End of SVF_StatusFile_Cancel_CB  */

/****************************************************************************
*
*  Function Name:                 SVF_StatusFile_Read_CB
*
*    This routine fetches the status file name from the file manager,
*    reads the file, and intializes the pst viewer.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SVF_StatusFile_Read_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  Rcb_t          stat_rcb;                   /* Temp Rcb from status file */
  RunStats_t     stat_rstat;                 /* Temp RunStats from status */
  XmFileSelectionBoxCallbackStruct *filecb;
  Display     *dsp;
  char        *filename;
  SynViewCB_t *svcb_p;
  PstView_t   *pv_p;
  PstCB_t     *pcb_p;
  char         title[128];

#ifdef _DEBUG_
printf("SVF_SF_R_CB 1\n");
#endif
  filecb = (XmFileSelectionBoxCallbackStruct *) call_p;
  if (!XmStringGetLtoR (filecb->value, XmFONTLIST_DEFAULT_TAG, &filename))  
    return ;

#ifdef _DEBUG_
printf("SVF_SF_R_CB 2\n");
#endif
  dsp = XtDisplay (SFileDg);
  XDefineCursor (dsp, XtWindow (SFileDg), SynAppR_IthCursor_Get (&GSynAppR,
    SAR_CRSRI_WAIT_W));
  XFlush (dsp);

#ifdef _DEBUG_
printf("SVF_SF_R_CB 3\n");
#endif
  /*  If the libraries have already been loaded, we need to make sure
      that the libraries specified in status file are the same ones.
      WE ALSO NEED TO DO THIS BEFORE ANYTHING ELSE TO SIMPLIFY MATTERS
      AND NOT HAVE THE PROGRAM CRASH BECAUSE WE DID TOO MANY UNNECESSARY
      THINGS TO KEEP TRACK OF BEFORE WE FLASHED THE WARNING AND RETURNED!
  */
  if (Rcb_Flags_LibsLoaded_Get (&GGoalRcb))
    {
#ifdef _DEBUG_
printf("SVF_SF_R_CB 3.1\n");
#endif
/*********************************************
UNCONDITIONALLY close and reopen ALL KB FILES!
*********************************************/
    Rcb_Flags_LibsLoaded_Put (&GGoalRcb, FALSE);
#ifdef _DEBUG_
printf("SVF_SF_R_CB 3.11\n");
#endif
    AvcLib_Control_Destroy ();
#ifdef _DEBUG_
printf("SVF_SF_R_CB 3.12\n");
#endif
    FuncGroups_Reset ();
#ifdef _DEBUG_
printf("SVF_SF_R_CB 3.2\n");
#endif
/* Defer React_Force_Initialize, which automatically destroys existing data,
   until after Status_File_Read, by which time the persistent context is known. */

/*
    Status_File_Peek (filename, &stat_rcb, &stat_rstat);
*/

    /* If the libraries specified in the status file differ from the 
       ones already loaded, ignore request to restart run.
    */
/*
    if (String_Compare (FileCB_DirStr_Get (Rcb_IthFileCB_Get (&GGoalRcb,
        FCB_IND_AVLC)), FileCB_DirStr_Get (Rcb_IthFileCB_Get (&stat_rcb,
        FCB_IND_AVLC))) != 0 ||
        String_Compare (FileCB_DirStr_Get (Rcb_IthFileCB_Get (&GGoalRcb,
        FCB_IND_FNGP)), FileCB_DirStr_Get (Rcb_IthFileCB_Get (&stat_rcb,
        FCB_IND_FNGP))) != 0 ||
        String_Compare (FileCB_DirStr_Get (Rcb_IthFileCB_Get (&GGoalRcb,
        FCB_IND_RXNS)), FileCB_DirStr_Get (Rcb_IthFileCB_Get (&stat_rcb,
        FCB_IND_RXNS))) != 0)
      {
      XtFree (filename);
      Rcb_Destroy (&stat_rcb);
      RunStats_Destroy (&stat_rstat);
      XUndefineCursor (dsp, XtWindow (SFileDg));
      XtUnmanageChild (SFileDg);
      XtRemoveCallback (SFileDg, XmNcancelCallback, SVF_StatusFile_Cancel_CB, 
        (XtPointer) NULL);
      XtRemoveCallback (SFileDg, XmNokCallback, SVF_StatusFile_Read_CB, 
        (XtPointer) NULL);
      InfoWarn_Show ("Current libraries differ from those specified in\n"
                     "the status file.  Restart program to examine file.\n");
      return ;
      }
*/
    }

#ifdef _DEBUG_
printf("SVF_SF_R_CB 4\n");
#endif
  pcb_p = Pst_ControlHandle_Get ();
  svcb_p = SynView_Handle_Grab ();
  pv_p = SynVCB_PstView_Get (svcb_p);

  if (PstCB_TotalExpandedCompounds_Get (pcb_p) > 0)
    {
    PstNode_t     node;

#ifdef _DEBUG_
printf("SVF_SF_R_CB 5\n");
#endif
    PstView_Mouse_Remove (pv_p);
    PstNode_Subgoal_Put (&node, PstCB_Root_Get (pcb_p));
    Pst_Destroy (node);
    SymbolTable_Destroy ();
    PstView_Reset (pv_p);
#ifdef _DEBUG_
printf("SVF_SF_R_CB 6\n");
#endif
    }

  /*  Temporary hack to make sure symbol table is initialized to NULL
      in case a status file with a smaller symbol table is attempted
      to be destroyed.
  */

  else 
    SymbolTable_Init ();

#ifdef _DEBUG_
printf("SVF_SF_R_CB 7\n");
#endif
  /*  If the libraries have already been loaded, we need to make sure
      that the libraries specified in status file are the same ones.
  */
  if (Rcb_Flags_LibsLoaded_Get (&GGoalRcb))
    {
/* This entire block is now superfluous, since the flag is reset on entry. */
    Status_File_Peek (filename, &stat_rcb, &stat_rstat);

    /* If the libraries specified in the status file differ from the 
       ones already loaded, ignore request to restart run.
       THIS CHECK HAS BEEN MOVED AHEAD TO A LESS DANGEROUS PLACE,
       I.E., AS CLOSE TO THE BEGINNING OF THE FUNCTION AS POSSIBLE.
    */

#ifdef _DEBUG_
printf("SVF_SF_R_CB 8\n");
#endif
    PstView_Mouse_Remove (pv_p);
    Status_File_Read (filename, &GGoalRcb, &GRunStats); 
    Rcb_Flags_LibsLoaded_Put (&GGoalRcb, TRUE);
    Rcb_Flags_LibsChanged_Put (&GGoalRcb, FALSE);
    Rcb_Destroy (&stat_rcb);
    RunStats_Destroy (&stat_rstat);
#ifdef _DEBUG_
printf("SVF_SF_R_CB 9\n");
#endif

    }  /* End of if libraries loaded. */

  else
    {
#ifdef _DEBUG_
printf("SVF_SF_R_CB 10\n");
#endif
    PstView_Mouse_Remove (pv_p);
    Status_File_Read (filename, &GGoalRcb, &GRunStats); 
/*
    AvcLib_Init ((char *) String_Value_Get (FileCB_DirStr_Get ( 
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_AVLC))), AVC_INIT_EXISTS);
    AvcLib_Init ((char *) String_Value_Get (FileCB_DirStr_Get ( 
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_AVLC))), AVC_INIT_INFO);
*/
    AvcLib_Init (FCB_SEQDIR_AVLC (""), AVC_INIT_EXISTS);
    AvcLib_Init (FCB_SEQDIR_AVLC (""), AVC_INIT_INFO);
/*
neither of these is necessary, because Status_File_Read makes the call
    React_Initialize (String_Value_Get (FileCB_DirStr_Get ( 
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_RXNS))), TRUE);

    React_Force_Initialize (String_Value_Get (FileCB_DirStr_Get ( 
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_RXNS))), TRUE);
*/
    FuncGroups_Init (String_Value_Get (FileCB_DirStr_Get ( 
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_FNGP))));
    Rcb_Flags_LibsLoaded_Put (&GGoalRcb, TRUE);
    Rcb_Flags_LibsChanged_Put (&GGoalRcb, FALSE);
#ifdef _DEBUG_
printf("SVF_SF_R_CB 11\n");
#endif
    }


  if (!PstView_Tree_Init (pv_p, Pst_ControlHandle_Get ()))
    {
#ifdef _DEBUG_
printf("SVF_SF_R_CB 12\n");
#endif
    InfoWarn_Show (PVW_NOTINIT_WARN);
#ifdef _DEBUG_
printf("SVF_SF_R_CB 13\n");
#endif
    }

  else
    {
#ifdef _DEBUG_
printf("SVF_SF_R_CB 14\n");
#endif
    if (PstCB_Root_Get (pcb_p) != NULL 
        && PstSubg_Son_Get (PstCB_Root_Get (pcb_p)) != NULL)
      {
#ifdef _DEBUG_
printf("SVF_SF_R_CB 15\n");
#endif
      PstVLvls_SetAll (pv_p, PstSubg_Son_Get (PstCB_Root_Get (pcb_p)), 0);
      PstView_Display (pv_p);

      if (XtIsManaged (RxnPreV_FormDlg_Get (ViewLvl_RxnPV_Get (
          PstView_PathVLvl_Get (pv_p)))) && !PstView_PthEHAct_Get (pv_p))
        {
#ifdef _DEBUG_
printf("SVF_SF_R_CB 16\n");
#endif
        XtAddEventHandler (DrawCxt_DA_Get (PstView_PathDC_Get (pv_p)),  
          PointerMotionMask, False, PathView_Preview_EH, (XtPointer) pv_p);
        PstView_PthEHAct_Put (pv_p, TRUE);
#ifdef _DEBUG_
printf("SVF_SF_R_CB 17\n");
#endif
        }

      if (PstView_NumActPV_Get (pv_p) > 0 && !PstView_PstEHAct_Get (pv_p))
        {
#ifdef _DEBUG_
printf("SVF_SF_R_CB 18\n");
#endif
        XtAddEventHandler (DrawCxt_DA_Get (PstView_PstDC_Get (pv_p)),  
          PointerMotionMask, False, PstView_Preview_EH, (XtPointer) pv_p);
        PstView_PstEHAct_Put (pv_p, TRUE);
#ifdef _DEBUG_
printf("SVF_SF_R_CB 19\n");
#endif
        }
#ifdef _DEBUG_
printf("SVF_SF_R_CB 20\n");
#endif
      }

    else
      {
#ifdef _DEBUG_
printf("SVF_SF_R_CB 21\n");
#endif
      InfoWarn_Show (PVW_NOROOT_WARN);
#ifdef _DEBUG_
printf("SVF_SF_R_CB 22\n");
#endif
      }
#ifdef _DEBUG_
printf("SVF_SF_R_CB 23\n");
#endif
    }
#ifdef _DEBUG_
printf("SVF_SF_R_CB 24\n");
#endif

  RunStats_Flags_StatusSaved_Put (&GRunStats, TRUE);
  RunStats_Flags_PstChanged_Put (&GRunStats, FALSE);

  XUndefineCursor (dsp, XtWindow (SFileDg));
  XtUnmanageChild (SFileDg);
  XtRemoveCallback (SFileDg, XmNcancelCallback, SVF_StatusFile_Cancel_CB, 
    (XtPointer) NULL);
  XtRemoveCallback (SFileDg, XmNokCallback, SVF_StatusFile_Read_CB, 
    (XtPointer) NULL);

#ifdef _DEBUG_
printf("SVF_SF_R_CB 25\n");
#endif
  sprintf (title, "SYNCHEM --- %s", 
    (char *) String_Value_Get (Rcb_Name_Get (&GGoalRcb)));
  XtVaSetValues (SynVCB_TopApp_Get (svcb_p), XmNtitle, title, NULL);
  XtFree (filename);

#ifdef _DEBUG_
printf("SVF_SF_R_CB 26\n");
#endif
  return ;
}
/*  End of SVF_StatusFile_Read_CB  */

/****************************************************************************
*
*  Function Name:                 SVF_StatusPeek_Cancel_CB
*
*    This routine removes the callbacks for the file manager after 
*    unmanaging it.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SVF_StatusPeek_Cancel_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
 
  XtUnmanageChild (SFileDg);
  XUndefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg));
  XtRemoveCallback (SFileDg, XmNcancelCallback, SVF_StatusPeek_Cancel_CB, 
    client_p);
  XtRemoveCallback (SFileDg, XmNokCallback, SVF_StatusPeek_Read_CB, 
    client_p);

  return ;
}
/*  End of SVF_StatusPeek_Cancel_CB  */

/****************************************************************************
*
*  Function Name:                 SVF_StatusPeek_Read_CB
*
*    This routine fetches the status file name from the file manager, peeks
*    into the file to get the rcb, and intializes the job submission form.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SVF_StatusPeek_Read_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  RunStats_t     stat_rstat;                 /* Temp RunStats from status */
  XmFileSelectionBoxCallbackStruct *filecb;
  char          *filename;
  SubmitCB_t    *sbmtcb_p;

  sbmtcb_p = (SubmitCB_t *) client_p;

  filecb = (XmFileSelectionBoxCallbackStruct *) call_p;
  if (!XmStringGetLtoR (filecb->value, XmFONTLIST_DEFAULT_TAG, &filename))  
    return ;
  if (filename[strlen (filename) - 1] == '/')
    {
    InfoWarn_Show ("No status file selected.");
    return;
    }

  if (Rcb_RunType_Get (Submit_TempRcb_Get (sbmtcb_p)) == RCB_TYPE_NONE)
    {
    Rcb_Destroy (Submit_TempRcb_Get (sbmtcb_p));
    Status_File_Peek (filename, Submit_TempRcb_Get (sbmtcb_p), &stat_rstat);
    JobInfo_Values_Set (Submit_JobInfoCB_Get (sbmtcb_p), Submit_TempRcb_Get (sbmtcb_p));
    Rcb_RunType_Put (Submit_TempRcb_Get (sbmtcb_p), RCB_TYPE_NONE);
    }
  else
    {
    Rcb_Destroy (Submit_TempRcb_Get (sbmtcb_p));
    Status_File_Peek (filename, Submit_TempRcb_Get (sbmtcb_p), &stat_rstat);
    Submit_Values_Set (sbmtcb_p, Submit_TempRcb_Get (sbmtcb_p));
    }

  JobInfo_Draw_CB (w, (XtPointer) Submit_JobInfoCB_Get (sbmtcb_p), 
    (XtPointer) NULL);
  RunStats_Destroy (&stat_rstat);

  XUndefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg));
  XtUnmanageChild (SFileDg);
  XtRemoveCallback (SFileDg, XmNcancelCallback, SVF_StatusPeek_Cancel_CB, 
    client_p);
  XtRemoveCallback (SFileDg, XmNokCallback, SVF_StatusPeek_Read_CB, 
    client_p);

  XtFree (filename);

  return ;
}
/*  End of SVF_StatusPeek_Read_CB  */

/****************************************************************************
*
*  Function Name:                 SVF_SubmitFile_Cancel_CB
*
*    This routine removes the callbacks for the file manager before 
*    unmanaging it.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SVF_SubmitFile_Cancel_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
 
  XtUnmanageChild (SFileDg);
  XUndefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg));
  XtRemoveCallback (SFileDg, XmNcancelCallback, SVF_SubmitFile_Cancel_CB, 
    client_p);
  XtRemoveCallback (SFileDg, XmNokCallback, SVF_SubmitFile_Read_CB, 
    client_p);

  return ;
}
/*  End of SVF_SubmitFile_Cancel_CB  */

/****************************************************************************
*
*  Function Name:                 SVF_SubmitFile_Read_CB
*
*    This routine fetches the submission file name from the file manager,
*    reads the file, and intializes the job submission form.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SVF_SubmitFile_Read_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{
  XmFileSelectionBoxCallbackStruct *filecb;
  char          *filename;
  SubmitCB_t    *sbmtcb_p;

  sbmtcb_p = (SubmitCB_t *) client_p;

  /*  Get the file name from the file selection box.  */
  filecb = (XmFileSelectionBoxCallbackStruct *) call_p;
  if (!XmStringGetLtoR (filecb->value, XmFONTLIST_DEFAULT_TAG, &filename))  
    return ;
  if (filename[strlen (filename) - 1] == '/')
    {
    InfoWarn_Show ("No submit file selected.");
    return;
    }

  if (Rcb_RunType_Get (Submit_TempRcb_Get (sbmtcb_p)) == RCB_TYPE_NONE)
    {
    Rcb_Destroy (Submit_TempRcb_Get (sbmtcb_p));
    Rcb_Init (Submit_TempRcb_Get (sbmtcb_p), FALSE);
    Rcb_Load (Submit_TempRcb_Get (sbmtcb_p), filename, TRUE);
    JobInfo_Values_Set (Submit_JobInfoCB_Get (sbmtcb_p), Submit_TempRcb_Get (sbmtcb_p));
    Rcb_RunType_Put (Submit_TempRcb_Get (sbmtcb_p), RCB_TYPE_NONE);
    }
  else
    {
    Rcb_Destroy (Submit_TempRcb_Get (sbmtcb_p));
    Rcb_Init (Submit_TempRcb_Get (sbmtcb_p), FALSE);
    Rcb_Load (Submit_TempRcb_Get (sbmtcb_p), filename, TRUE);
    Submit_Values_Set (sbmtcb_p, Submit_TempRcb_Get (sbmtcb_p));
    }

  JobInfo_Draw_CB (w, (XtPointer) Submit_JobInfoCB_Get (sbmtcb_p), 
    (XtPointer) NULL);

  XUndefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg));
  XtUnmanageChild (SFileDg);
  XtRemoveCallback (SFileDg, XmNcancelCallback, SVF_SubmitFile_Cancel_CB, 
    client_p);
  XtRemoveCallback (SFileDg, XmNokCallback, SVF_SubmitFile_Read_CB, 
    client_p);

  XtFree (filename);
  return ;
}
/*  End of SVF_SubmitFile_Read_CB  */

/****************************************************************************
*
*  Function Name:                 SVF_DeleteFile_Cancel_CB
*
*    This routine removes the callbacks for the file manager before 
*    unmanaging it.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SVF_DeleteFile_Cancel_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
 
  XtUnmanageChild (SFileDg);
  XUndefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg));
  XtRemoveCallback (SFileDg, XmNcancelCallback, SVF_DeleteFile_Cancel_CB, 
    client_p);
  XtRemoveCallback (SFileDg, XmNokCallback, SVF_DeleteFile_Exec_CB, 
    client_p);

  return ;
}
/*  End of SVF_DeleteFile_Cancel_CB  */

/****************************************************************************
*
*  Function Name:                 SVF_DeleteFile_Exec_CB
*
*    This routine deletes the status file whose name is obtained from the
*    file manager, along with all attendant files.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SVF_DeleteFile_Exec_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{
  XmFileSelectionBoxCallbackStruct *filecb;
  static char                      *filename; /* need to retain for deletion callbacks */

  /*  Get the file name from the file selection box.  */
  filecb = (XmFileSelectionBoxCallbackStruct *) call_p;
  if (!XmStringGetLtoR (filecb->value, XmFONTLIST_DEFAULT_TAG, &filename))  
    return ;
  if (filename[strlen (filename) - 1] == '/')
    {
    InfoWarn_Show ("No status file selected.");
    return;
    }

  XUndefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg));
  XtUnmanageChild (SFileDg);
  XtRemoveCallback (SFileDg, XmNcancelCallback, SVF_DeleteFile_Cancel_CB, 
    client_p);
  XtRemoveCallback (SFileDg, XmNokCallback, SVF_DeleteFile_Exec_CB, 
    client_p);

  SVF_QueryDg_Update ("DeleteRun", 8, (XtPointer) &filename); /* a level of indirection seems to be
                                                                 necessary to insure an unchanging
                                                                 value, although documentation of
                                                                 any such requirement is either
                                                                 obscure or missing - don't trust
                                                                 opaque functions any more than
                                                                 necessary! */
  return ;
}
/*  End of SVF_DeleteFile_Exec_CB  */

/****************************************************************************
*
*  Function Name:                 SVF_DSTFile_Cancel_CB
*
*    This routine removes the callbacks for the file manager before 
*    unmanaging it.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SVF_DSTFile_Cancel_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
 
  XtUnmanageChild (SFileDg);
  XUndefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg));
  XtRemoveCallback (SFileDg, XmNcancelCallback, SVF_DSTFile_Cancel_CB, 
    client_p);
  XtRemoveCallback (SFileDg, XmNokCallback, SVF_DSTFile_Read_CB, 
    client_p);

  return ;
}
/*  End of SVF_DSTFile_Cancel_CB  */

/****************************************************************************
*
*  Function Name:                 SVF_DSTFile_Read_CB
*
*    This routine fetches the draw_sling_template file name from the file manager,
*    reads the file, and initializes the job submission form.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SVF_DSTFile_Read_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{
  XmFileSelectionBoxCallbackStruct *filecb;
  char          *filename,
                 line[256],
                *eol;
  SubmitCB_t    *sbmtcb_p;
  FILE          *f_p;

  sbmtcb_p = (SubmitCB_t *) client_p;

  /*  Get the file name from the file selection box.  */
  filecb = (XmFileSelectionBoxCallbackStruct *) call_p;
  if (!XmStringGetLtoR (filecb->value, XmFONTLIST_DEFAULT_TAG, &filename))  
    return ;
  if (filename[strlen (filename) - 1] == '/')
    {
    InfoWarn_Show ("No drawing template file selected.");
    return;
    }

#ifdef _INTERIX_
    f_p = fopen (gccfix (filename), "r");
#else
#ifdef _CYGWIN_
    f_p = fopen (filename, "r");
#else
#ifdef _WIN32
    f_p = fopen (gccfix (filename), "r");
#else
    f_p = fopen (filename, "r");
#endif
#endif
#endif

  /** Set value for the user name.  **/
  fgets (line, 254, f_p);
  eol = strstr (line, "\r\n");
  if (eol == NULL) eol = strstr (line, "\n");
  *eol = '\0';
  strncpy (JobInfo_User_Get (Submit_JobInfoCB_Get (sbmtcb_p)), line, SJI_USERBUF_MAXLEN);
  JobInfo_User_Get (Submit_JobInfoCB_Get (sbmtcb_p))[SJI_USERBUF_MAXLEN] = '\0';
  XmTextSetString (JobInfo_UserTxt_Get (Submit_JobInfoCB_Get (sbmtcb_p)),
    JobInfo_User_Get (Submit_JobInfoCB_Get (sbmtcb_p)));

  /** Set value for date.  **/
  fgets (line, 254, f_p);
  eol = strstr (line, "\r\n");
  if (eol == NULL) eol = strstr (line, "\n");
  *eol = '\0';
  sprintf (JobInfo_Date_Get (Submit_JobInfoCB_Get (sbmtcb_p)), "%s", line);
  XmTextSetString (JobInfo_DateTxt_Get (Submit_JobInfoCB_Get (sbmtcb_p)),
    JobInfo_Date_Get (Submit_JobInfoCB_Get (sbmtcb_p)));

  /** Set value for comment.  **/
  fgets (line, 254, f_p);
  eol = strstr (line, "\r\n");
  if (eol == NULL) eol = strstr (line, "\n");
  *eol = '\0';
  strncpy (JobInfo_Comment_Get (Submit_JobInfoCB_Get (sbmtcb_p)), line, SJI_COMNTBUF_MAXLEN);
  JobInfo_Comment_Get (Submit_JobInfoCB_Get (sbmtcb_p))[SJI_COMNTBUF_MAXLEN] = '\0';
  XmTextSetString (JobInfo_CommentTxt_Get (Submit_JobInfoCB_Get (sbmtcb_p)),
    JobInfo_Comment_Get (Submit_JobInfoCB_Get (sbmtcb_p)));

  /** Set value for runid.  **/
  fgets (line, 254, f_p);
  eol = strstr (line, "\r\n");
  if (eol == NULL) eol = strstr (line, "\n");
  *eol = '\0';
  strncpy (JobInfo_RunId_Get (Submit_JobInfoCB_Get (sbmtcb_p)), line, SJI_RUNIDBUF_MAXLEN);
  JobInfo_RunId_Get (Submit_JobInfoCB_Get (sbmtcb_p))[SJI_RUNIDBUF_MAXLEN] = '\0';
  XmTextSetString (JobInfo_RunIdTxt_Get (Submit_JobInfoCB_Get (sbmtcb_p)),
    JobInfo_RunId_Get (Submit_JobInfoCB_Get (sbmtcb_p)));

  /** Set value for target compound (sling).  **/
  fgets (line, 254, f_p);
  eol = strstr (line, "\r\n");
  if (eol == NULL) eol = strstr (line, "\n");
  *eol = '\0';
  strncpy (JobInfo_Sling_Get (Submit_JobInfoCB_Get (sbmtcb_p)), line, SJI_SLINGBUF_MAXLEN);
  JobInfo_Sling_Get (Submit_JobInfoCB_Get (sbmtcb_p))[SJI_SLINGBUF_MAXLEN] = '\0';
  XmTextSetString (JobInfo_SlingTxt_Get (Submit_JobInfoCB_Get (sbmtcb_p)),
    JobInfo_Sling_Get (Submit_JobInfoCB_Get (sbmtcb_p)));

  fclose (f_p);

  JobInfo_Draw_CB (w, (XtPointer) Submit_JobInfoCB_Get (sbmtcb_p), 
    (XtPointer) NULL);

  XUndefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg));
  XtUnmanageChild (SFileDg);
  XtRemoveCallback (SFileDg, XmNcancelCallback, SVF_DSTFile_Cancel_CB, 
    client_p);
  XtRemoveCallback (SFileDg, XmNokCallback, SVF_DSTFile_Read_CB, 
    client_p);

  XtFree (filename);
  return ;
}
/*  End of SVF_DSTFile_Read_CB  */

/****************************************************************************
*
*  Function Name:                 SVF_PathTrc_Cancel_CB
*
*    This routine removes the callbacks for the file manager before 
*    unmanaging it.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SVF_PathTrc_Cancel_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
 
  XtUnmanageChild (SFileDg);
  XUndefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg));
  XtRemoveCallback (SFileDg, XmNcancelCallback, SVF_PathTrc_Cancel_CB, 
    client_p);
  XtRemoveCallback (SFileDg, XmNokCallback, SVF_PathTrc_CB, 
    client_p);
  SynMenu_PathTrc_CB (NULL, NULL, NULL);

  return ;
}
/*  End of SVF_PathTrc_Cancel_CB  */

/****************************************************************************
*
*  Function Name:                 SVF_PathTrc_CB
*
*    This routine fetches the name of the path file from the file manager
*    and sends it pack to the SynMenu_PathTrc_CB () function.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SVF_PathTrc_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{
  XmFileSelectionBoxCallbackStruct *filecb;
  char          *filename,
                 fname_copy[256],
                *eol,
                *tag;
  FILE          *f_p, *tf_p;
  int            c;

  tag = (char *) client_p;

  /*  Get the file name from the file selection box.  */
  filecb = (XmFileSelectionBoxCallbackStruct *) call_p;
  if (!XmStringGetLtoR (filecb->value, XmFONTLIST_DEFAULT_TAG, &filename))  
    return ;
  XUndefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg));
  XtUnmanageChild (SFileDg);
  XtRemoveCallback (SFileDg, XmNcancelCallback, SVF_PathTrc_Cancel_CB, 
    client_p);
  XtRemoveCallback (SFileDg, XmNokCallback, SVF_PathTrc_CB, 
    client_p);

  if (filename[strlen (filename) - 1] == '/')
    {
    InfoWarn_Show ("No path file selected.");
    XtFree (filename);
    return;
    }

  strcpy (fname_copy, filename);
  XtFree (filename);
  SynMenu_PathTrc_CB (NULL, (XtPointer) fname_copy, NULL);
  return ;
}
/*  End of SVF_PathTrc_CB  */

/****************************************************************************
*
*  Function Name:                 SVF_PathFile_RdCancel_CB
*
*    This routine removes the callbacks for the file manager before 
*    unmanaging it.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SVF_PathFile_RdCancel_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
 
  XtUnmanageChild (SFileDg);
  XUndefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg));
  XtRemoveCallback (SFileDg, XmNcancelCallback, SVF_PathFile_RdCancel_CB, 
    client_p);
  XtRemoveCallback (SFileDg, XmNokCallback, SVF_PathFile_Read_CB, 
    client_p);
  SynMenu_Path_CB (NULL, NULL, NULL);

  return ;
}
/*  End of SVF_PathFile_RdCancel_CB  */

/****************************************************************************
*
*  Function Name:                 SVF_PathFile_Read_CB
*
*    This routine fetches the path file name from the file manager, reads the
*    file into temp.pth, and reinvokes the SynMenu_Path_CB () function.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SVF_PathFile_Read_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{
  XmFileSelectionBoxCallbackStruct *filecb;
  char          *filename,
                 tfilename[50],
                 line[256],
                *eol,
                *tag;
  FILE          *f_p, *tf_p;
  int            c;

  tag = (char *) client_p;

  /*  Get the file name from the file selection box.  */
  filecb = (XmFileSelectionBoxCallbackStruct *) call_p;
  if (!XmStringGetLtoR (filecb->value, XmFONTLIST_DEFAULT_TAG, &filename))  
    return ;

  XUndefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg));
  XtUnmanageChild (SFileDg);
  XtRemoveCallback (SFileDg, XmNcancelCallback, SVF_PathFile_RdCancel_CB, 
    client_p);
  XtRemoveCallback (SFileDg, XmNokCallback, SVF_PathFile_Read_CB, 
    client_p);

  if (filename[strlen (filename) - 1] == '/')
    {
    InfoWarn_Show ("No path file selected.");
    XtFree (filename);
    return;
    }

#ifdef _INTERIX_
    f_p = fopen (filename, "r");
    sprintf (tfilename, "/tmp/temp.pth.%s", session_code);
#else
#ifdef _WIN32
    f_p = fopen (gccfix (filename), "r");
    sprintf (tfilename, "temp.pth.%s", session_code);
#else
#ifdef _CYGWIN_
    f_p = fopen (filename, "r");
    sprintf (tfilename, "temp.pth.%s", session_code);
#else
    f_p = fopen (filename, "r");
    sprintf (tfilename, "/tmp/temp.pth.%s", session_code);
#endif
#endif
#endif

  tf_p = fopen (tfilename, "w");

  while ((c = getc (f_p)) != EOF) putc (c, tf_p);
  
  fclose (f_p);
  fclose (tf_p);

  XtFree (filename);
  SynMenu_Path_CB (NULL, (XtPointer) tag, NULL);
  return ;
}
/*  End of SVF_PathFile_Read_CB  */

/****************************************************************************
*
*  Function Name:                 SVF_PathFile_WrCancel_CB
*
*    This routine removes the callbacks for the file manager before 
*    unmanaging it.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SVF_PathFile_WrCancel_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  char command[128];
 
  sprintf (command,
#ifdef _INTERIX_
    "rm temp.pth.%s",
#else
#ifdef _WIN32
    "rm temp.pth.%s",
#else
    "rm /tmp/temp.pth.%s",
#endif
#endif
    session_code);
/*
  system (command);
*/
  remove(command+3);

  XtUnmanageChild (SFileDg);
  XUndefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg));
  XtRemoveCallback (SFileDg, XmNcancelCallback, SVF_PathFile_WrCancel_CB, 
    client_p);
  XtRemoveCallback (SFileDg, XmNokCallback, SVF_PathFile_Write_CB, 
    client_p);

  return ;
}
/*  End of SVF_PathFile_WrCancel_CB  */

/****************************************************************************
*
*  Function Name:                 SVF_PathFile_Write_CB
*
*    This routine fetches the path file name from the file manager and copies
*    temp.pth into it.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SVF_PathFile_Write_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{
  XmFileSelectionBoxCallbackStruct *filecb;
  char          *filename,
                 tfilename[50],
                 command[128],
                 line[256],
                *message,
                *eol;
  FILE          *f_p, *tf_p;
  int            c;
  static char    overwrite_attempt[128] = {'\0'};

  message = (char *) client_p;

  /*  Get the file name from the file selection box.  */
  filecb = (XmFileSelectionBoxCallbackStruct *) call_p;
  if (!XmStringGetLtoR (filecb->value, XmFONTLIST_DEFAULT_TAG, &filename))  
    return ;
  if (filename[strlen (filename) - 1] == '/')
    {
    InfoWarn_Show ("No path file selected.");
    return;
    }

#ifdef _WIN32
    f_p = fopen (gccfix (filename), "r");
#else
    f_p = fopen (filename, "r");
#endif

  if (f_p != NULL)
  {
    fclose (f_p);
    if (strcmp (filename, overwrite_attempt) == 0) overwrite_attempt[0] = '\0';
    else
    {
      strcpy (overwrite_attempt, filename);
      InfoWarn_Show ("File exists - not overwritten.\n\nIf your intent IS to overwrite the file,\nyou must click OK again; "
        "otherwise, please\nmodify the name in the selection field\nbefore doing so.");
      return;
    }
  }
  else overwrite_attempt[0] = '\0';

#ifdef _INTERIX_
    f_p = fopen (gccfix (filename), "w");
    sprintf (tfilename, "temp.pth.%s", session_code);
#else
#ifdef _WIN32
    f_p = fopen (gccfix (filename), "w");
    sprintf (tfilename, "temp.pth.%s", session_code);
#else
    f_p = fopen (filename, "w");
    sprintf (tfilename, "/tmp/temp.pth.%s", session_code);
#endif
#endif

  if (f_p == NULL)
  {
    InfoWarn_Show ("File could not be written.");
    fclose (f_p);
    return;
  }

  tf_p = fopen (tfilename, "r");
  sprintf (command, "rm %s", tfilename);

  while ((c = getc (tf_p)) != EOF) if (putc (c, f_p) == EOF)
  {
    InfoWarn_Show ("File could not be written completely - check free disk space.");
    fclose (f_p);
    fclose (tf_p);
/*
    system (command);
*/
    remove (command + 3);
    return;
  }

  fclose (f_p);
  fclose (tf_p);
/*
  system (command);
*/
  remove (command + 3);

  XUndefineCursor (XtDisplay (SFileDg), XtWindow (SFileDg));
  XtUnmanageChild (SFileDg);
  XtRemoveCallback (SFileDg, XmNcancelCallback, SVF_PathFile_WrCancel_CB, 
    client_p);
  XtRemoveCallback (SFileDg, XmNokCallback, SVF_PathFile_Write_CB, 
    client_p);

  XtFree (filename);

  switch (message[0])
  {
  case 'M':
    InfoMess_Show (message + 1);
    break;
  case 'W':
    InfoWarn_Show (message + 1);
    break;
  default:
    break;
  }
  return ;
}
/*  End of SVF_PathFile_Write_CB  */

/****************************************************************************
*
*  Function Name:                 SQueryExec_Cancel_CB
*
*    Remove the callbacks, unmanage dialog and execute submission. 
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SQueryExec_Cancel_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{

  SubmitCB_t     *scb_p;

  scb_p = (SubmitCB_t *) client_p;

  XtUnmanageChild (SQuerySaveDg);

/*
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryExec_Cancel_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryExec_Okay_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryExec_Del_CB, client_p);
*/
  XtRemoveCallback (SQSPB[0], XmNactivateCallback, 
    SQueryExec_Okay_CB, client_p);
  XtRemoveCallback (SQSPB[1], XmNactivateCallback, 
    SQueryExec_Cancel_CB, client_p);

  Submission_Execute (scb_p);

  return ;
}
/*  End of SQueryExec_Cancel_CB  */

/****************************************************************************
*
*  Function Name:                 SQueryExec_Del_CB
*
*    Remove the callbacks, unmanage dialog and delete trace, submit;
*    execute submission
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SQueryExec_Del_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{

  SubmitCB_t     *scb_p;

  scb_p = (SubmitCB_t *) client_p;

  XtUnmanageChild (SQuerySaveDg);

  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryExec_Cancel_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryExec_Okay_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryExec_Del_CB, client_p);

  DelSubTrc ();

  Submission_Execute (scb_p);

  return ;
}
/*  End of SQueryExec_Del_CB  */

/****************************************************************************
*
*  Function Name:                 SQueryExec_Okay_CB
*
*    Remove the proper callbacks based on tag and unmanage dialog. 
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SQueryExec_Okay_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{
  Display       *dsp;
  SubmitCB_t    *scb_p;
  Boolean_t      ok2submit;

  scb_p = (SubmitCB_t *) client_p;

  dsp = XtDisplay (SQuerySaveDg);
  XDefineCursor (dsp, XtWindow (SQuerySaveDg), 
  SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_WAIT_W));
  XFlush (dsp);

/*
  write_fail_fatal = FALSE;
  write_fail_str[0] = '\0';
  Status_File_Write ((char *)String_Value_Get (FileCB_FileStr_Get (
    Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_STAT))), &GGoalRcb,
    &GRunStats);
  if (ok2submit = write_fail_str[0] == '\0')
    {
    RunStats_Flags_StatusSaved_Put (&GRunStats, TRUE);
    RunStats_Flags_PstChanged_Put (&GRunStats, FALSE);
    }

  XUndefineCursor (dsp, XtWindow (SQuerySaveDg));
  XtUnmanageChild (SQuerySaveDg);

  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryExec_Cancel_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryExec_Okay_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryExec_Del_CB, client_p);
*/
  SQuerySave_Commit_CB (NULL, &ok2submit, NULL);
  
  XUndefineCursor (dsp, XtWindow (SQuerySaveDg));
  XtUnmanageChild (SQuerySaveDg);

  XtRemoveCallback (SQSPB[0], XmNactivateCallback, 
    SQueryExec_Okay_CB, client_p);
  XtRemoveCallback (SQSPB[1], XmNactivateCallback, 
    SQueryExec_Cancel_CB, client_p);

  if (ok2submit) Submission_Execute (scb_p);
  else
    {
    InfoWarn_Show (write_fail_str);
    write_fail_str[0] = '\0';
    write_fail_fatal = TRUE;
    }

  return ;
}
/*  End of SQueryExec_Okay_CB  */

/****************************************************************************
*
*  Function Name:                 SQueryExit_Cancel_CB
*
*    Remove the proper callbacks based on tag and unmanage dialog. 
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SQueryExit_Cancel_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{

#ifdef _DEBUG_
printf("SQueryExit_Cancel entered\n");
#endif
  XtUnmanageChild (SQuerySaveDg);
/*
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryExit_Cancel_CB, NULL);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryExit_Okay_CB, NULL);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryExit_Del_CB, NULL);
*/
#ifdef _DEBUG_
printf("SQueryExit_Cancel removing callbacks\n");
#endif
  XtRemoveCallback (SQSPB[0], XmNactivateCallback, 
    SQueryExit_Okay_CB, client_p);
  XtRemoveCallback (SQSPB[1], XmNactivateCallback, 
    SQueryExit_Cancel_CB, client_p);

#ifdef _DEBUG_
printf("SQueryExit_Cancel calling SynView_Exit\n");
#endif
  SynView_Exit ();
  return ;
}
/*  End of SQueryExit_Cancel_CB  */

/****************************************************************************
*
*  Function Name:                 SQueryExit_Del_CB
*
*    Remove the proper callbacks based on tag and unmanage dialog; delete
*    trace, submit.
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SQueryExit_Del_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{

  XtUnmanageChild (SQuerySaveDg);
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryExit_Cancel_CB, NULL);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryExit_Okay_CB, NULL);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryExit_Del_CB, NULL);

  DelSubTrc ();

  SynView_Exit ();
  return ;
}
/*  End of SQueryExit_Del_CB  */

/****************************************************************************
*
*  Function Name:                 SQueryExit_Okay_CB
*
*    Remove the proper callbacks based on tag and unmanage dialog. 
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SQueryExit_Okay_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{
  Display       *dsp;
  Boolean_t      ok2exit;

  dsp = XtDisplay (SQuerySaveDg);
  XDefineCursor (dsp, XtWindow (SQuerySaveDg), 
  SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_WAIT_W));
  XFlush (dsp);

/*
  write_fail_fatal = FALSE;
  write_fail_str[0] = '\0';
  Status_File_Write ((char *)String_Value_Get (FileCB_FileStr_Get (
    Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_STAT))), &GGoalRcb,
    &GRunStats);

  if (ok2exit = write_fail_str[0] == '\0')
    {
    RunStats_Flags_StatusSaved_Put (&GRunStats, TRUE);
    RunStats_Flags_PstChanged_Put (&GRunStats, FALSE);
    }

  XUndefineCursor (dsp, XtWindow (SQuerySaveDg));
  XtUnmanageChild (SQuerySaveDg);
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryExit_Cancel_CB, NULL);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryExit_Okay_CB, NULL);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryExit_Del_CB, NULL);
*/

  SQuerySave_Commit_CB (NULL, &ok2exit, NULL);
  
  XUndefineCursor (dsp, XtWindow (SQuerySaveDg));
  XtUnmanageChild (SQuerySaveDg);

  XtRemoveCallback (SQSPB[0], XmNactivateCallback, 
    SQueryExit_Okay_CB, client_p);
  XtRemoveCallback (SQSPB[1], XmNactivateCallback, 
    SQueryExit_Cancel_CB, client_p);

  if (ok2exit) SynView_Exit ();
  else
    {
    InfoWarn_Show (write_fail_str);
    write_fail_str[0] = '\0';
    write_fail_fatal = TRUE;
    }

  return ;
}
/*  End of SQueryExit_Okay_CB  */

/****************************************************************************
*
*  Function Name:                 SQueryNone_Cancel_CB
*
*    Remove the proper callbacks based on tag and unmanage dialog. 
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SQueryNone_Cancel_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{

  XtUnmanageChild (SQuerySaveDg);
/*
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryNone_Cancel_CB, *//* NULL unsafe *//* client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryNone_Okay_CB, *//* NULL unsafe *//* client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryNone_Del_CB, *//* NULL unsafe *//* client_p);
*/
  XtRemoveCallback (SQSPB[0], XmNactivateCallback, 
    SQueryNone_Okay_CB, client_p);
  XtRemoveCallback (SQSPB[1], XmNactivateCallback, 
    SQueryNone_Cancel_CB, client_p);

  return ;
}
/*  End of SQueryExit_Cancel_CB  */

/****************************************************************************
*
*  Function Name:                 SQueryNone_Del_CB
*
*    Remove the proper callbacks based on tag and unmanage dialog; delete
*    trace, submit.
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SQueryNone_Del_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{

  XtUnmanageChild (SQuerySaveDg);
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryNone_Cancel_CB, /* NULL unsafe */ client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryNone_Okay_CB, /* NULL unsafe */ client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryNone_Del_CB, /* NULL unsafe */ client_p);

  DelSubTrc ();

  return ;
}
/*  End of SQueryExit_Del_CB  */

/****************************************************************************
*
*  Function Name:                 SQueryNone_Okay_CB
*
*    Remove the proper callbacks based on tag and unmanage dialog. 
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SQueryNone_Okay_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{
  Display       *dsp;
  Boolean_t      ok;

  dsp = XtDisplay (SQuerySaveDg);
  XDefineCursor (dsp, XtWindow (SQuerySaveDg), 
  SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_WAIT_W));
  XFlush (dsp);

/*
  write_fail_fatal = FALSE;
  write_fail_str[0] = '\0';
  Status_File_Write ((char *)String_Value_Get (FileCB_FileStr_Get (
    Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_STAT))), &GGoalRcb,
    &GRunStats);

  if (ok = write_fail_str[0] == '\0')
    {
    RunStats_Flags_StatusSaved_Put (&GRunStats, TRUE);
    RunStats_Flags_PstChanged_Put (&GRunStats, FALSE);
    }

  XUndefineCursor (dsp, XtWindow (SQuerySaveDg));
  XtUnmanageChild (SQuerySaveDg);
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryNone_Cancel_CB, *//* NULL unsafe *//* client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryNone_Okay_CB, *//* NULL unsafe *//* client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryNone_Del_CB, *//* NULL unsafe *//* client_p);

  if (!ok)
    {
    InfoWarn_Show (write_fail_str);
    write_fail_str[0] = '\0';
    write_fail_fatal = TRUE;
    }
*/

  SQuerySave_Commit_CB (NULL, NULL, NULL);
  
  XUndefineCursor (dsp, XtWindow (SQuerySaveDg));
  XtUnmanageChild (SQuerySaveDg);

  XtRemoveCallback (SQSPB[0], XmNactivateCallback, 
    SQueryNone_Okay_CB, client_p);
  XtRemoveCallback (SQSPB[1], XmNactivateCallback, 
    SQueryNone_Cancel_CB, client_p);


  return ;
}
/*  End of SQueryNone_Okay_CB  */

/****************************************************************************
*
*  Function Name:                 SQueryOpenS_Cancel_CB
*
*    Remove the proper callbacks based on tag and unmanage dialog;
*    open sequential status file.
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SQueryOpenS_Cancel_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{

  XtUnmanageChild (SQuerySaveDg);
/* remove CORRECT callbacks!
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryNone_Cancel_CB, NULL);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryNone_Okay_CB, NULL);
*/
/*
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryOpenS_Cancel_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryOpenS_Okay_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryOpenS_Del_CB, client_p);
*/
  XtRemoveCallback (SQSPB[0], XmNactivateCallback, 
    SQueryOpenS_Okay_CB, client_p);
  XtRemoveCallback (SQSPB[1], XmNactivateCallback, 
    SQueryOpenS_Cancel_CB, client_p);

  SVF_FileDg_Update (SVF_FILED_TYPE_OPENSTAT " No Save", client_p, FALSE);

  return ;
}
/*  End of SQueryOpenS_Cancel_CB  */

/****************************************************************************
*
*  Function Name:                 SQueryOpenS_Del_CB
*
*    Remove the proper callbacks based on tag and unmanage dialog;
*    delete submit, trace; open sequential status file.
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SQueryOpenS_Del_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{

  XtUnmanageChild (SQuerySaveDg);
/* remove CORRECT callbacks!
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryNone_Cancel_CB, NULL);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryNone_Okay_CB, NULL);
*/
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryOpenS_Cancel_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryOpenS_Okay_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryOpenS_Del_CB, client_p);

  DelSubTrc ();

  SVF_FileDg_Update (SVF_FILED_TYPE_OPENSTAT " No Save", client_p, FALSE);

  return ;
}
/*  End of SQueryOpenS_Del_CB  */

/****************************************************************************
*
*  Function Name:                 SQueryOpenS_Okay_CB
*
*    Remove the proper callbacks based on tag and unmanage dialog;
*    open sequential status file.
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SQueryOpenS_Okay_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{
  Display       *dsp;
  Boolean_t      ok2open;

  dsp = XtDisplay (SQuerySaveDg);
  XDefineCursor (dsp, XtWindow (SQuerySaveDg), 
  SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_WAIT_W));
  XFlush (dsp);

/*
  write_fail_fatal = FALSE;
  write_fail_str[0] = '\0';
  Status_File_Write ((char *)String_Value_Get (FileCB_FileStr_Get (
    Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_STAT))), &GGoalRcb,
    &GRunStats);

  if (ok2open = write_fail_str[0] == '\0')
    {
    RunStats_Flags_StatusSaved_Put (&GRunStats, TRUE);
    RunStats_Flags_PstChanged_Put (&GRunStats, FALSE);
    }

  XUndefineCursor (dsp, XtWindow (SQuerySaveDg));
  XtUnmanageChild (SQuerySaveDg);
*/
/* remove CORRECT callbacks!
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryNone_Cancel_CB, NULL);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryNone_Okay_CB, NULL);
*/
/*
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryOpenS_Cancel_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryOpenS_Okay_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryOpenS_Del_CB, client_p);
*/

  SQuerySave_Commit_CB (NULL, &ok2open, NULL);
  
  XUndefineCursor (dsp, XtWindow (SQuerySaveDg));
  XtUnmanageChild (SQuerySaveDg);

  XtRemoveCallback (SQSPB[0], XmNactivateCallback, 
    SQueryOpenS_Okay_CB, client_p);
  XtRemoveCallback (SQSPB[1], XmNactivateCallback, 
    SQueryOpenS_Cancel_CB, client_p);

  if (ok2open) SVF_FileDg_Update (SVF_FILED_TYPE_OPENSTAT " No Save", client_p, FALSE);
  else
    {
    InfoWarn_Show (write_fail_str);
    write_fail_str[0] = '\0';
    write_fail_fatal = TRUE;
    }

  return ;
}
/*  End of SQueryOpenS_Okay_CB  */

/****************************************************************************
*
*  Function Name:                 SQueryOpenP_Cancel_CB
*
*    Remove the proper callbacks based on tag and unmanage dialog;
*    open parallel status file.
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SQueryOpenP_Cancel_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{

  XtUnmanageChild (SQuerySaveDg);
/* remove CORRECT callbacks!
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryNone_Cancel_CB, NULL);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryNone_Okay_CB, NULL);
*/
/*
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryOpenP_Cancel_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryOpenP_Okay_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryOpenP_Del_CB, client_p);
*/
  XtRemoveCallback (SQSPB[0], XmNactivateCallback, 
    SQueryOpenP_Okay_CB, client_p);
  XtRemoveCallback (SQSPB[1], XmNactivateCallback, 
    SQueryOpenP_Cancel_CB, client_p);

  SVF_FileDg_Update (SVF_FILED_TYPE_OPENSTAT " No Save", client_p, TRUE);

  return ;
}
/*  End of SQueryOpenP_Cancel_CB  */

/****************************************************************************
*
*  Function Name:                 SQueryOpenP_Del_CB
*
*    Remove the proper callbacks based on tag and unmanage dialog;
*    delete submit, trace; open parallel status file.
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SQueryOpenP_Del_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{

  XtUnmanageChild (SQuerySaveDg);
/* remove CORRECT callbacks!
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryNone_Cancel_CB, NULL);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryNone_Okay_CB, NULL);
*/
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryOpenP_Cancel_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryOpenP_Okay_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryOpenP_Del_CB, client_p);

  DelSubTrc ();

  SVF_FileDg_Update (SVF_FILED_TYPE_OPENSTAT " No Save", client_p, TRUE);

  return ;
}
/*  End of SQueryOpenP_Del_CB  */

/****************************************************************************
*
*  Function Name:                 SQueryOpenP_Okay_CB
*
*    Remove the proper callbacks based on tag and unmanage dialog;
*    open parallel status file.
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SQueryOpenP_Okay_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{
  Display       *dsp;
  Boolean_t      ok2open;

  dsp = XtDisplay (SQuerySaveDg);
  XDefineCursor (dsp, XtWindow (SQuerySaveDg), 
  SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_WAIT_W));
  XFlush (dsp);

/*
  write_fail_fatal = FALSE;
  write_fail_str[0] = '\0';
  Status_File_Write ((char *)String_Value_Get (FileCB_FileStr_Get (
    Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_STAT))), &GGoalRcb,
    &GRunStats);

  if (ok2open = write_fail_str[0] == '\0')
    {
    RunStats_Flags_StatusSaved_Put (&GRunStats, TRUE);
    RunStats_Flags_PstChanged_Put (&GRunStats, FALSE);
    }

  XUndefineCursor (dsp, XtWindow (SQuerySaveDg));
  XtUnmanageChild (SQuerySaveDg);
*/
/* remove CORRECT callbacks!
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryNone_Cancel_CB, NULL);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryNone_Okay_CB, NULL);
*/
/*
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryOpenP_Cancel_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryOpenP_Okay_CB, client_p);
*/

  SQuerySave_Commit_CB (NULL, &ok2open, NULL);
  
  XUndefineCursor (dsp, XtWindow (SQuerySaveDg));
  XtUnmanageChild (SQuerySaveDg);

  XtRemoveCallback (SQSPB[0], XmNactivateCallback, 
    SQueryOpenP_Okay_CB, client_p);
  XtRemoveCallback (SQSPB[1], XmNactivateCallback, 
    SQueryOpenP_Cancel_CB, client_p);

  if (ok2open) SVF_FileDg_Update (SVF_FILED_TYPE_OPENSTAT " No Save", client_p, TRUE);
  else
    {
    InfoWarn_Show (write_fail_str);
    write_fail_str[0] = '\0';
    write_fail_fatal = TRUE;
    }

  return ;
}
/*  End of SQueryOpenP_Okay_CB  */

/****************************************************************************
*
*  Function Name:                 SQueryDelRun_Cancel_CB
*
*    Remove the proper callbacks based on tag and unmanage dialog;
*    open parallel status file.
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SQueryDelRun_Cancel_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{
  Widget   child;
  char   **filename;

  XtUnmanageChild (SQuerySaveDg);
/*
  child = XmMessageBoxGetChild (SQuerySaveDg, XmDIALOG_HELP_BUTTON);
  XtUnmanageChild (child);
*/
/*
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryDelRun_Cancel_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryDelRun_Okay_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNhelpCallback, 
    SQueryDelRun_Okay2_CB, client_p);

  filename = (char **) client_p;
  XtFree (*filename);
*/
  XtRemoveCallback (SQSPB[0], XmNactivateCallback, 
    SQueryDelRun_Okay_CB, client_p);
  XtRemoveCallback (SQSPB[1], XmNactivateCallback, 
    SQueryDelRun_Cancel_CB, client_p);

  return ;
}
/*  End of SQueryDelRun_Cancel_CB  */

/****************************************************************************
*
*  Function Name:                 SQueryDelRun_Okay_CB
*
*    Remove the proper callbacks based on tag and unmanage dialog;
*    delete all run files.
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SQueryDelRun_Okay_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{
  Widget   child;
  char   **filename;

  XtUnmanageChild (SQuerySaveDg);
/*
  child = XmMessageBoxGetChild (SQuerySaveDg, XmDIALOG_HELP_BUTTON);
  XtUnmanageChild (child);
*/
/*
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryDelRun_Cancel_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryDelRun_Okay_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNhelpCallback, 
    SQueryDelRun_Okay2_CB, client_p);

  filename = (char **) client_p;
  SDelRun_Files (*filename, "/trace/", ".trace", "/submit/", ".submit", "/pfile/", ".*path", "/status/", ".status", NULL);
  XtFree (*filename);
*/

  SQuerySave_Commit_CB (NULL, NULL, NULL);
  
  XtRemoveCallback (SQSPB[0], XmNactivateCallback, 
    SQueryDelRun_Okay_CB, client_p);
  XtRemoveCallback (SQSPB[1], XmNactivateCallback, 
    SQueryDelRun_Cancel_CB, client_p);

  return ;
}
/*  End of SQueryDelRun_Okay_CB  */

/****************************************************************************
*
*  Function Name:                 SQueryDelRun_Okay2_CB
*
*    Remove the proper callbacks based on tag and unmanage dialog;
*    delete all run files except for submit file.
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SQueryDelRun_Okay2_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{
  Widget   child;
  char   **filename;

  XtUnmanageChild (SQuerySaveDg);
/*
  child = XmMessageBoxGetChild (SQuerySaveDg, XmDIALOG_HELP_BUTTON);
  XtUnmanageChild (child);
*/
  XtRemoveCallback (SQuerySaveDg, XmNcancelCallback, 
    SQueryDelRun_Cancel_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNokCallback, 
    SQueryDelRun_Okay_CB, client_p);
  XtRemoveCallback (SQuerySaveDg, XmNhelpCallback, 
    SQueryDelRun_Okay2_CB, client_p);

  filename = (char **) client_p;
  SDelRun_Files (*filename, "/trace/", ".trace", "/pfile/", ".*path", "/status/", ".status", NULL);
  XtFree (*filename);

  return ;
}

/****************************************************************************
*
*  Function Name:                 SVF_ButtonState_CB
*
*    SVF_ButtonState_CB (sets the state according to the name of the
*                        button and its parent)
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SVF_ButtonState_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{
  Widget child;
  int    buttonn, which;
  char   bname[12];

  sscanf (XtName(XtParent(w)), "SQSDRadio_%d", &buttonn);
/*
  sscanf (XtName(w), "button_%d", button_state + buttonn);
*/
  sscanf (XtName(w), "button_%d", &which);
/*
  if (button_state[buttonn] == which) button_state[buttonn] = 1 - which;
  else button_state[buttonn] = which;
*/
  button_state[buttonn] = which; /* toggling is too confusing - just select the one clicked, whether or not already selected */
  XtVaSetValues (w,
    XmNset, True, /* must explicitly select this one ... */
    NULL);
  sprintf (bname, "button_%d", 1 - which);
  child = XtNameToWidget (SQuerySaveRadio[buttonn], bname);
  XtVaSetValues (child,
/*
    XmNset, button_state[buttonn] == which ? False : True,
*/
    XmNset, False, /* and unselect the other one */
    NULL);

  return ;
}
/*  End of SVF_ButtonState_CB  */

static void  SDelRun_Files (char *filename, char *trcdir, char *trcext, ...)
{
  va_list  ap;
  char     command[512], *next[2], *prev[2], temp[256], *pos;
  int      i;

  sprintf (command, "rm %s", filename);
/*
  system (command);
printf ("%s\n",command);
*/
  remove(command+3);
  prev[0] = trcdir;
  prev[1] = trcext;

  va_start (ap, trcext);
  do
  {
    next[0] = va_arg (ap, char *);
    if (next[0] != NULL)
    {
      next[1] = va_arg (ap, char *);
      if (next[1] == NULL)
      {
        fprintf (stderr, "Mismatched arglist passed to SDelRun_Files!\n");
        exit (1);
      }
      for (i = 0; i < 2; i++)
      {
        pos = strstr (command, prev[i]);
        if (pos == NULL)
        {
          fprintf (stderr, "Error seeking \"%s\" within \"%s\"!\n", prev[i], command);
          exit (1);
        }
        strcpy (temp, pos + strlen (prev[i]));
        sprintf (pos, "%s%s", next[i], temp);
        prev[i] = next[i];
      }
/*
  system (command);
printf ("%s\n",command);
*/
  remove(command+3);
    }
  }
  while (next[0] != NULL);
}

/****************************************************************************
*
*  Function Name:                 SQuerySave_Commit_CB
*
*    SQuerySave_Commit_CB (sets the state according to the name of the
*                        button and its parent)
*
*
*  Implicit Inputs:
*
*    SQuerySaveDg
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SQuerySave_Commit_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p  
  )
{
  int i;
  Boolean_t *ok, local_ok;
#ifdef _DEBUG_
static char *str[4]={"status","path","submit","trace"};
#endif

  ok = (Boolean_t *) client_p;
  if (ok == NULL) ok = &local_ok;
  *ok = TRUE;

  for (i=0; i<4; i++)
  {
#ifdef _DEBUG_
printf("i=%d: BEFORE %s of %s\n", i, button_state[i]?"deletion":"save/retain", str[i]);
#endif
    if (button_state[i]) delfile (i);
    else *ok &= save_ret_file (i);
#ifdef _DEBUG_
printf("i=%d: AFTER %s of %s\n", i, button_state[i]?"deletion":"save/retain", str[i]);
#endif
  }
  XtUnmanageChild (SQuerySaveDg);
}

static char *SFileName_RunName_Extract (char **filename)
{
  static char  name[50];
  char        *find;

  find = strstr (*filename, "/");
  if (find == NULL) find = *filename;
  else
  {
    find = *filename + strlen (*filename);
    while (*(find - 1) != '/') find--;
  }
  strcpy (name, find);
  find = strstr (name, ".");
  if (find != NULL) *find = '\0';

  return (name);
}

void DelSubTrc ()
{
  char filename[256];

  strcpy (filename, (char *)String_Value_Get (FileCB_FileStr_Get (
    Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_TRCE))));
  SDelRun_Files (filename, "/trace/", ".trace", "/submit/", ".submit", NULL);
}

int wild_remove (char *filename)
{
  DIR           *dir;
  struct dirent *dir_entry;
  char          *asterisk,
                *slash,
                 dirname[100],
                 file[50],
                 filespec[150],
                *testext;
  Boolean_t      found;

#ifdef _DEBUG_
printf("wild_remove called for %s\n",filename);
#endif
  if ((asterisk = strstr (filename, "*")) == NULL) return (remove (filename));
  strcpy (dirname, filename);
  for (slash = dirname + strlen (dirname) - 1; *slash != '/'; slash--);
  *slash++ = '\0';
  strcpy (file, slash);
  *(strstr(file,"*")) = '\0';
  dir = opendir (dirname);
  found = FALSE;
  while ((dir_entry = readdir (dir)) != NULL)
    if (!strncmp (dir_entry->d_name, file, strlen (file)) &&
    (testext = strstr (dir_entry->d_name + strlen (file), asterisk + 1)) != NULL &&
    !strcmp (testext, asterisk + 1))
  {
    sprintf (filespec, "%s/%s", dirname, dir_entry->d_name);
#ifdef _DEBUG_
printf("removing %s\n",filespec);
#endif
    found = !remove (filespec);
  }
  closedir (dir);

  return (found ? 0 : -1);
}

void delfile (int filetype)
{
  char filename[256], msg[512];
  FILE *f;
#ifdef _DEBUG_
char *which[]={"status","path","submit","trace"};
#endif

  if (delete_flag) which_rcb = &delrun_rcb;
  else which_rcb = &GGoalRcb;
  if (!button_enabled[filetype]) return;
#ifdef _DEBUG_
printf("delfile entered for %s with delete_flag=%d\n",which[filetype],delete_flag);
#endif
  switch (filetype)
  {
  case 0:
    strcpy (filename, (char *)String_Value_Get (FileCB_FileStr_Get (
      Rcb_IthFileCB_Get (which_rcb, FCB_IND_STAT))));
    break;
  case 1:
    if (!delete_flag) return;
    strcpy (filename, (char *)String_Value_Get (FileCB_FileStr_Get (
      Rcb_IthFileCB_Get (which_rcb, FCB_IND_PATH))));
    break;
  case 2:
    strcpy (filename, (char *)String_Value_Get (FileCB_FileStr_Get (
      Rcb_IthFileCB_Get (which_rcb, FCB_IND_SBMT))));
    break;
  case 3:
    strcpy (filename, (char *)String_Value_Get (FileCB_FileStr_Get (
      Rcb_IthFileCB_Get (which_rcb, FCB_IND_TRCE))));
    break;
  }
/*
  if ((f=fopen(filename,"r"))!=NULL) fclose(f);
  else
  {
    if (delete_flag)
    {
      sprintf (msg, "Unable to delete %s - file not found", filename);
      InfoWarn_Show (msg);
    }
    return;
  }
  remove (filename);
*/
  if (wild_remove (filename))
  {
    if (delete_flag)
    {
      sprintf (msg, "Unable to delete %s - no matching file(s) found", filename);
      InfoWarn_Show (msg);
    }
  }
}

Boolean_t save_ret_file (int filetype)
{
  char filename[256], msg[512], date_str[32];
  FILE *f;
  SubmitCB_t scb;
  Boolean_t ok;
#ifdef _DEBUG_
char *which[]={"status","path","submit","trace"};
#endif

  if (delete_flag) return (TRUE);
#ifdef _DEBUG_
printf("save_ret_file entered for %s\n",which[filetype]);
#endif
  ok = TRUE;
  switch (filetype)
  {
  case 0:
    strcpy (filename, (char *)String_Value_Get (FileCB_FileStr_Get (
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_STAT))));
    if ((f=fopen(filename,"r"))!=NULL)
    {
      fclose(f);
      if (RunStats_Flags_PstChanged_Get (&GRunStats))
      {
        write_fail_fatal = FALSE;
        write_fail_str[0] = '\0';
        Status_File_Write ((char *)String_Value_Get (FileCB_FileStr_Get (
          Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_STAT))), &GGoalRcb,
          &GRunStats);

        if (ok = write_fail_str[0] == '\0')
        {
          RunStats_Flags_StatusSaved_Put (&GRunStats, TRUE);
          RunStats_Flags_PstChanged_Put (&GRunStats, FALSE);
        }
      }
    }
    break;
  case 1:
    autosave_path = TRUE;
    SynMenu_Path_CB (NULL, (XtPointer)SMU_PATH_WRITE_TAG, NULL);
    autosave_path = FALSE;
    break;
  case 2:
    strcpy (filename, (char *)String_Value_Get (FileCB_FileStr_Get (
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_SBMT))));
    if ((f=fopen(filename,"r"))!=NULL)
    {
      fclose(f);
      return (TRUE);
    }
/* For the moment, don't try to save submit file - crashes disposition!
#ifdef _DEBUG_
printf ("before saving submit file (%s)\n", filename);
#endif
    SubmitMenu_FSave_CB (NULL, (XtPointer) &scb, NULL);
#ifdef _DEBUG_
printf ("after saving submit file (%s)\n", filename);
#endif
Of course the above doesn't work!  It expects the submit window and all its subwindows to be open! */
    strcpy (date_str, ctime (&((Rcb_Date_Get (&GGoalRcb)).tv_sec)));
    f=fopen(filename,"w");
    fprintf(f,"!  SYNCHEM job submission file.\n!  User:  %s      Date:  %s!\nCompoundName          %s\nComment   %s\n\n",
      String_Value_Get (Rcb_User_Get (&GGoalRcb)), date_str, String_Value_Get (Rcb_Name_Get (&GGoalRcb)),
      String_Value_Get (Rcb_Comment_Get (&GGoalRcb)));
    fprintf (f, "TargetCompound   %s\nMaxCycles             %d\nMaxRuntime            %d\nRunRestart            %s\n",
      Sling_Name_Get (Rcb_Goal_Get (&GGoalRcb)), Rcb_MaxCycles_Get (&GGoalRcb), Rcb_MaxRuntime_Get (&GGoalRcb),
      Rcb_Flags_Restart_Get (&GGoalRcb) ? "True" : "False");
    fprintf (f, "BackgroundProcess     %s\nEffortDistribution    %s\nNTCL                  %d\nFirstSolution         %s\n",
      Rcb_Flags_Background_Get (&GGoalRcb) ? "True" : "False", Rcb_Flags_EffortDis_Get (&GGoalRcb) ? "True" : "False",
      Rcb_NTCL_Get (&GGoalRcb), Rcb_Flags_FirstSol_Get (&GGoalRcb) ? "True" : "False");
    fprintf (f, "AdditionalCycles      %d\nAdditionalTime        %d\nRunType               %d\nNoStereochemistry     %s\n",
      Rcb_AddCycles_Get (&GGoalRcb), Rcb_AddTime_Get (&GGoalRcb), Rcb_RunType_Get (&GGoalRcb), Rcb_Flags_StereoChemistry_Get
      (&GGoalRcb) ? "False" : "True");
    fprintf (f, "PreserveStructures    %s\nStrategicBonds        %s\nSaveStatusFile        %s\nSaveTraceFile         %s\n",
      Rcb_Flags_PreserveStructures_Get (&GGoalRcb) ? "True" : "False", Rcb_Flags_StrategicBonds_Get (&GGoalRcb) ? "True" : "False",
      Rcb_Flags_SaveStatus_Get (&GGoalRcb) ? "True" : "False", Rcb_Flags_SaveTrace_Get (&GGoalRcb) ? "True" : "False");
    fprintf (f, "SaveShelvedMerits     %s\nChemistryTraceLevel   %d\nRunContinuation       %s\nLeapSize              %d\n",
      Rcb_Flags_SaveMerits_Get (&GGoalRcb) ? "True" : "False", Rcb_ChemistryTrace_Get (&GGoalRcb), Rcb_Flags_Continue_Get
      (&GGoalRcb) ? "True" : "False", Rcb_LeapSize_Get (&GGoalRcb));
    fprintf (f, "AvailCompLibDir            %s\nFunctionalGroupDir         %s\nTemplateDir                %s\n",
      FCB_SEQDIR_AVLC (""), FCB_SEQDIR_FNGP (""), FCB_SEQDIR_TMPL (""));
    fprintf (f, "ReactionLibDir             %s\nPathDir                    %s\nStatusDir                  %s\n",
      FCB_SEQDIR_RXNS (""), FCB_SEQDIR_PATH (""), FCB_SEQDIR_STAT (""));
    fprintf (f, "SubmitDir                  %s\nTraceDir                   %s\nCoverDir                   %s\n",
      FCB_SEQDIR_SBMT (""), FCB_SEQDIR_TRCE (""), FCB_SEQDIR_COVR (""));
    fprintf (f, "ForceSelectDir             %s\nCoverFile             %s\nSearchSpaceCoverDev   %s\n",
      FCB_SEQDIR_FSEL (""), String_Value_Get (Rcb_Name_Get (&GGoalRcb)), Rcb_Flags_SearchCoverDev_Get (&GGoalRcb) ? "True" :
      "False");
    fprintf (f, "SearchSpaceCoverSlv   %s\nDevCoverPercent       %d\nForceSelectFile       %s\n",
      Rcb_Flags_SearchCoverSlv_Get (&GGoalRcb) ? "True" : "False", Rcb_CoverPrct_Get (&GGoalRcb), String_Value_Get (Rcb_Name_Get
      (&GGoalRcb)));
    fclose(f);
    break;
  case 3:
    strcpy (filename, (char *)String_Value_Get (FileCB_FileStr_Get (
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_TRCE))));
    if ((f=fopen(filename,"r"))!=NULL) fclose(f);
    else
    {
      sprintf (msg, "Unable to retain %s - file not found", filename);
      InfoWarn_Show (msg);
      return (TRUE);
    }
    break;
  }
  return (ok);
}

static void reconcile_2files (char mask[3][128])
{
  int len[2], i, j, n;

  len[0] = strlen (mask[0]);
  len[1] = strlen (mask[1]);
  if (len[0]<len[1]) n=0;
  else n=1;
  for (i=j=0; i<len[n]; i++) if (mask[0][i]==mask[1][i]) mask[2][j++]=mask[0][i];
  else
  {
    sprintf(mask[2]+j, "[%c,%c]", mask[0][i], mask[1][i]);
    j+=5;
  }
  if (len[0]!=len[1]) mask[2][j++]='*';
  mask[2][j]='\0';
}

static void reconcile_4files (char *filename)
{
  char format_string[128], string[128], *status, *pfile, *submit, *trace, *s;

  strcpy (format_string, filename);
  status = strstr (format_string, "status");
  pfile = strstr (format_string, "pfile");
  submit = strstr (format_string, "submit");
  trace = strstr (format_string, "trace");
  if (status != NULL)
  {
    strncpy (status, "%s", 2);
    strcpy (status + 2, status + 6);
    status = strstr (format_string, "status");
/*
    strncpy (status, "%s", 2);
    strcpy (status + 2, status + 6);
*/
    strcpy (status, "%s");
  }
  else if (pfile != NULL)
  {
    strncpy (pfile, "%s", 2);
    strcpy (pfile + 2, pfile + 5);
/*
    pfile = strstr (format_string, "path");
    while (*(pfile - 1) != '.') pfile--;
    strncpy (pfile, "%s", 2);
    strcpy (pfile + 2, pfile + 5);
*/
    pfile = strstr (format_string, ".path");
    while (*(pfile - 1) != '.' && *(pfile - 1) != '/') pfile--;
    if (*(pfile - 1) == '/') pfile = strstr (format_string, "path");
    strcpy (pfile, "%s");
  }
  else if (submit != NULL)
  {
    strncpy (submit, "%s", 2);
    strcpy (submit + 2, submit + 6);
    submit = strstr (format_string, "submit");
/*
    strncpy (submit, "%s", 2);
    strcpy (submit + 2, submit + 6);
*/
    strcpy (submit, "%s");
  }
  else if (trace != NULL)
  {
    strncpy (trace, "%s", 2);
    strcpy (trace + 2, trace + 5);
    trace = strstr (format_string, "trace");
/*
    strncpy (trace, "%s", 2);
    strcpy (trace + 2, trace + 5);
*/
    strcpy (trace, "%s");
  }
  sprintf (string, format_string, "status", "status");
  FileCB_FileStr_Put (Rcb_IthFileCB_Get (&delrun_rcb, FCB_IND_STAT), String_Create ((const char *)string, 0));
  s=string+strlen(string);
  while (*--s != '/');
  *s = '\0';
  FileCB_DirStr_Put (Rcb_IthFileCB_Get (&delrun_rcb, FCB_IND_STAT), String_Create ((const char *)string, 0));
  sprintf (string, format_string, "pfile", "*path");
  FileCB_FileStr_Put (Rcb_IthFileCB_Get (&delrun_rcb, FCB_IND_PATH), String_Create ((const char *)string, 0));
  s=string+strlen(string);
  while (*--s != '/');
  *s = '\0';
  FileCB_DirStr_Put (Rcb_IthFileCB_Get (&delrun_rcb, FCB_IND_PATH), String_Create ((const char *)string, 0));
  sprintf (string, format_string, "submit", "submit");
  FileCB_FileStr_Put (Rcb_IthFileCB_Get (&delrun_rcb, FCB_IND_SBMT), String_Create ((const char *)string, 0));
  s=string+strlen(string);
  while (*--s != '/');
  *s = '\0';
  FileCB_DirStr_Put (Rcb_IthFileCB_Get (&delrun_rcb, FCB_IND_SBMT), String_Create ((const char *)string, 0));
  sprintf (string, format_string, "trace", "trace");
  FileCB_FileStr_Put (Rcb_IthFileCB_Get (&delrun_rcb, FCB_IND_TRCE), String_Create ((const char *)string, 0));
  s=string+strlen(string);
  while (*--s != '/');
  *s = '\0';
  FileCB_DirStr_Put (Rcb_IthFileCB_Get (&delrun_rcb, FCB_IND_TRCE), String_Create ((const char *)string, 0));
}

/*  End of SYN_VIEW.C */
