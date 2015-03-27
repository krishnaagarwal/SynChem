/****************************************************************************
*  
* Copyright (C) 1997 Synchem Group at SUNY-Stony Brook, Daren Krebsbach  
*  
*  
*  Module Name:               SUBMIT.C
*  
*    Creates and manages the job submission tool.  
*      
*  Creation Date:  
*  
*     26-Aug-1997  
*  
*  Authors: 
*      Daren Krebsbach 
*        based on version by Nader Abdelsadek
*  
*  Routines:
*
*    Submission_Create
*    Submit_Values_Get
*    Submit_Values_Set
*
*    Submit_CancelPB_CB
*    Submit_SubmitPB_CB
*    Submit_ResetPB_CB
*    Submit_NumVerify_CB
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 26-Feb-01  Miller     Forced use of current available compounds library,
*                       rather than potentially obsolete version stored in
*                       status file.
*
*******************************************************************************/  
#include <stddef.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <ctype.h>
#include <string.h> 
#include <Xm/PushB.h> 
#include <Xm/Form.h> 
#include <Xm/RowColumn.h> 
#include <Xm/MainW.h> 
#include <Xm/MessageB.h> 
#include <Xm/Separator.h> 
#include <Xm/Text.h> 
#include <Xm/LabelG.h> 
#include <Xm/ToggleB.h> 
#include <Xm/Frame.h>
#include <Xm/FileSB.h> 

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

#ifndef _H_RCB_
#include "rcb.h"
#endif

#ifndef _H_STATUS_
#include "status.h"
#endif

#ifndef _H_APP_RESRC_ 
#include "app_resrc.h" 
#endif 

#ifndef _H_PST_VIEW_
#include "pst_view.h"
#endif

#ifndef _H_SV_FILES_
#include "sv_files.h"
#endif

#ifndef _H_SSHOT_VIEW_
#include "sshot_view.h"
#endif

#ifndef _H_INFO_VIEW_
#include "info_view.h"
#endif

#ifndef _H_SEARCH_GUI_
#include "search_gui.h"
#endif

#ifndef _H_SUBMIT_MENU_ 
#include "submit_menu.h" 
#endif

#ifndef _H_SUBMIT_ 
#include "submit.h" 
#endif

#ifndef _H_SUBMIT_SAVEPLAN_
#include "submit_saveplan.h"
#endif

#ifndef _H_SYN_VIEW_
#include "syn_view.h"
#endif

#ifndef _H_PERSIST_
#include "persist.h"
#endif

#ifndef _H_FUNCGROUPS_
#include "funcgroups.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_EXTERN_
#include "extern.h"
#endif

void SS_WalkPath (U32_t, String_t);

static SavePlanCB_t saveplan_cb;

/****************************************************************************
*  
*  Function Name:                 Submission_Create
*  
*****************************************************************************/  

void  Submission_Create 
  ( 
  SubmitCB_t    *submitcb_p, 
  Widget         parent, 
  XtAppContext  *appcon
  ) 
{ 
  ScreenAttr_t  *scra_p;                  /* Screen Attritbutes */
  Widget         menubar, runtype, exitcond, jobinfo, drawtool, saveplan; 
  Widget         formdg, separator, appshell; 
  XmString       lbl_str;
  Dimension      job_info_ht;
   
  scra_p = SynAppR_ScrnAtrb_Get (&GSynAppR);

  lbl_str = XmStringCreateLtoR (SBT_SUBMIT_TITLE, XmFONTLIST_DEFAULT_TAG);
  formdg = XmCreateFormDialog (parent, "SubmitDlg", NULL, 0);

  XtVaSetValues (formdg,
    XmNdialogStyle,    XmDIALOG_FULL_APPLICATION_MODAL,
    XmNlabelFontList,  SynAppR_FontList_Get (&GSynAppR),
    XmNbuttonFontList, SynAppR_FontList_Get (&GSynAppR),
    XmNtextFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNresizePolicy,   XmRESIZE_GROW,
    XmNresizable,      True,
    XmNautoUnmanage,   False, 
    XmNdialogTitle,    lbl_str,
    XmNx,              0,
    XmNy,              0,
    XmNheight,         Screen_Height_Get (scra_p) - 5 * SWS_APPSHELL_OFFY / 2,
    XmNwidth,          Screen_Width_Get (scra_p) - SWS_APPSHELL_OFFX,
   NULL);
  XmStringFree (lbl_str);
  Submit_FormDlg_Put (submitcb_p, formdg);

  separator = XtVaCreateManagedWidget ("SubmitSep", 
    xmSeparatorWidgetClass, formdg, 
    XmNseparatorType,       XmDOUBLE_LINE, 
    XmNshadowThickness,     AppDim_SepLarge_Get (&GAppDim),
    NULL);   

  /* Create the "reset", "cancel" and "submit" pushbuttons. */ 
   
  lbl_str = XmStringCreateLocalized (SBT_RESET_PB_LBL);  
  Submit_ResetPB_Put (submitcb_p, 
    XtVaCreateManagedWidget ("SubmitPB",  
      xmPushButtonWidgetClass, formdg,  
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,
      XmNheight,               AppDim_HtLblPB_Get (&GAppDim),
      NULL)); 
  XmStringFree (lbl_str);

  XtAddCallback(Submit_ResetPB_Get (submitcb_p), XmNactivateCallback,
    Submit_ResetPB_CB, (XtPointer) submitcb_p);
   
  lbl_str = XmStringCreateLocalized (SBT_CANCEL_PB_LBL);  
  Submit_CancelPB_Put (submitcb_p, 
    XtVaCreateManagedWidget ("SubmitPB",  
      xmPushButtonWidgetClass, formdg,  
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,
      XmNheight,               AppDim_HtLblPB_Get (&GAppDim),
      NULL)); 
  XmStringFree (lbl_str);
 
  XtAddCallback(Submit_CancelPB_Get (submitcb_p), XmNactivateCallback,  
    Submit_CancelPB_CB, (XtPointer) submitcb_p);
 
  lbl_str = XmStringCreateLocalized (SBT_SUBMIT_PB_LBL);  
  Submit_SubmitPB_Put (submitcb_p, 
    XtVaCreateManagedWidget ("SubmitPB",  
      xmPushButtonWidgetClass, formdg,  
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str, 
      XmNheight,               AppDim_HtLblPB_Get (&GAppDim),
      NULL));  
  XmStringFree (lbl_str);
  
  XtAddCallback(Submit_SubmitPB_Get (submitcb_p), XmNactivateCallback,  
   Submit_SubmitPB_CB, (XtPointer) submitcb_p);

  menubar = SubmitMenu_Create (submitcb_p, formdg); 
  runtype = RunType_Create (Submit_RunTypeCB_Get (submitcb_p), formdg);   
  exitcond = ExitCond_Create (Submit_ExitCondCB_Get (submitcb_p), formdg);   
  saveplan = SavePlan_Create (&saveplan_cb, formdg);   
  jobinfo = JobInfo_Create (Submit_JobInfoCB_Get (submitcb_p), formdg); 


  drawtool = Draw_Tool_Create (Submit_JobInfoCB_Get (submitcb_p), formdg,
    appcon);

  Rcb_Init (Submit_TempRcb_Get (submitcb_p), FALSE); 
  Submit_Values_Set (submitcb_p, Submit_TempRcb_Get (submitcb_p));

  /* Create the rest of the job submission tool ***/ 
    
  XtVaSetValues (menubar, 
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM, 
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM, 
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM, 
   NULL); 

  XtVaSetValues (Submit_SubmitPB_Get (submitcb_p),
    XmNtopOffset,        -(AppDim_HtLblPB_Get (&GAppDim) 
                           + 2 * AppDim_MargFmFr_Get (&GAppDim)),
    XmNtopAttachment,    XmATTACH_OPPOSITE_FORM,
    XmNbottomOffset,     AppDim_MargFmFr_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     15,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    29,
    NULL);

  XtVaSetValues (Submit_CancelPB_Get (submitcb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        Submit_SubmitPB_Get (submitcb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     Submit_SubmitPB_Get (submitcb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     43,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    57,
    NULL);

  XtVaSetValues (Submit_ResetPB_Get (submitcb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        Submit_SubmitPB_Get (submitcb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     Submit_SubmitPB_Get (submitcb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     71,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    85,
    NULL);

  XtVaSetValues (separator,
    XmNtopOffset,        -(AppDim_HtLblPB_Get (&GAppDim) 
                           + 3 * AppDim_MargFmFr_Get (&GAppDim)
                           + AppDim_SepLarge_Get (&GAppDim)),
    XmNtopAttachment,    XmATTACH_OPPOSITE_FORM,
    XmNbottomOffset,     AppDim_MargFmFr_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget,     Submit_SubmitPB_Get (submitcb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  job_info_ht = 2 * AppDim_HtLblPB_Get (&GAppDim) 
    + AppDim_SepSmall_Get (&GAppDim) 
    + 9 * AppDim_MargLblPB_Get (&GAppDim)
    + 4 * AppDim_HtTxt_Get (&GAppDim)
    + 2 * AppDim_MargFmFr_Get (&GAppDim);

  XtVaSetValues (jobinfo, 
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        menubar,
    XmNbottomOffset,     -(job_info_ht + AppDim_HtLblPB_Get (&GAppDim) 
                           + 2 * AppDim_MargFmFr_Get (&GAppDim)),
    XmNbottomAttachment, XmATTACH_OPPOSITE_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (exitcond,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_NONE,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget,     separator,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      -AppDim_SbtExitWd_Get (&GAppDim),
    XmNrightAttachment,  XmATTACH_OPPOSITE_FORM,
    NULL);

  XtVaSetValues (saveplan,
    XmNtopOffset,        -200,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        exitcond,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget,     exitcond,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      -AppDim_SbtExitWd_Get (&GAppDim),
    XmNrightAttachment,  XmATTACH_OPPOSITE_FORM,
    NULL);

  XtVaSetValues (runtype,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        jobinfo,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
    XmNrightWidget,      exitcond,
    NULL);

  XtVaSetValues (drawtool,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        jobinfo,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget,     separator,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       exitcond,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  /* Manage Unmanaged children */    
  XtManageChild (menubar); 
  XtManageChild (separator); 
  XtManageChild (runtype); 
  XtManageChild (exitcond); 
  XtManageChild (saveplan); 
  XtManageChild (jobinfo); 
  XtManageChild (drawtool); 
  
  XtManageChild (formdg);

  Submit_Created_Put (submitcb_p, TRUE); 
  Submit_ContLoaded_Put (submitcb_p, FALSE); 

  return; 
} 
/* End of Submission_Create
*    N/A
*
*  Side Effects:
*
*    N/A */ 

/****************************************************************************
*  
*  Function Name:                 Submission_Destroy
*  
*****************************************************************************/  

void Submission_Destroy 
  (
  SubmitCB_t     *submitcb_p
  )
{
  Submit_Created_Put (submitcb_p, FALSE); 
  return;
}
/* End of Submission_Destroy */ 
  
/****************************************************************************
*  
*  Function Name:                 Submission_Dump
*  
*****************************************************************************/  

Boolean_t Submission_Dump 
  (
  Rcb_t         *rcb_p
  )
{
  FILE          *f;                          /* Temporary */
  char          *fn;                         /* Temporary */


  fn = (char *) String_Value_Get (FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_SBMT)));

#ifdef _WIN32
  f = fopen (gccfix (fn), "w");
#else
  f = fopen (fn, "w");
#endif
  if (f == NULL)
    {
    char   mess[256];

    sprintf (mess, "Unable to open (write) job submission file:\n  %s\n", fn);
    InfoWarn_Show (mess);
    return (FALSE);
    }

  fprintf (f, "!  SYNCHEM job submission file.\n");

  fprintf (f, "!  %s  %s      %s  %s!\n", 
    "User:", String_Value_Get (Rcb_User_Get (rcb_p)),
    "Date:", ctime (&((Rcb_Date_Get (rcb_p)).tv_sec)));

  fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_NAME, 
    (char *) String_Value_Get (Rcb_Name_Get (rcb_p)));
  fprintf (f, "%-8s  %s\n", RCB_FILE_TAG_COMMENT, 
    (char *) String_Value_Get (Rcb_Comment_Get (rcb_p)));
  fprintf (f, "%-15s  %s\n", RCB_FILE_TAG_GOAL, 
    (char *) Sling_Name_Get (Rcb_Goal_Get (rcb_p)));


  fprintf (f, "%-20s  %lu\n", RCB_FILE_TAG_MAX_CYCLES, 
    Rcb_MaxCycles_Get (rcb_p));
  fprintf (f, "%-20s  %lu\n", RCB_FILE_TAG_MAX_TIME, 
    Rcb_MaxRuntime_Get (rcb_p));
  fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_RUNRESTART,
    (Rcb_Flags_Restart_Get (rcb_p)) ? "True" : "False");
  fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_BACKGROUND,
    (Rcb_Flags_Background_Get (rcb_p)) ? "True" : "False");

  fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_EFFORTDIS,
    (Rcb_Flags_EffortDis_Get (rcb_p)) ? "True" : "False");
  fprintf (f, "%-20s  %hu\n", RCB_FILE_TAG_NTCL, 
    Rcb_NTCL_Get (rcb_p));
 
  fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_FIRSTSOL,
    (Rcb_Flags_FirstSol_Get (rcb_p)) ? "True" : "False");
  fprintf (f, "%-20s  %lu\n", RCB_FILE_TAG_ADD_CYCLES, 
    Rcb_AddCycles_Get (rcb_p));
  fprintf (f, "%-20s  %lu\n", RCB_FILE_TAG_ADD_TIME, 
    Rcb_AddTime_Get (rcb_p));

  fprintf (f, "%-20s  %hu\n", RCB_FILE_TAG_RUNTYPE, 
    Rcb_RunType_Get (rcb_p));

  if (Rcb_RunType_Get (rcb_p) >= RCB_TYPE_DIS_START)
    {
    fprintf (f, "%-20s  %hu\n", RCB_FILE_TAG_MAX_NODES, 
      Rcb_MaxNodes_Get (rcb_p));
    fprintf (f, "%-20s  %u\n", RCB_FILE_TAG_MAX_WORKERS, 
      Rcb_MaxWorkers_Get (rcb_p));
    fprintf (f, "%-20s  %u\n", RCB_FILE_TAG_MIN_WORKERS, 
      Rcb_MinWorkers_Get (rcb_p));
    fprintf (f, "%-20s  %hu per node\n", RCB_FILE_TAG_PROC_NODE, 
      Rcb_ProcPerNode_Get (rcb_p));

    fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_DIS_SHARED_SELS,
      (Rcb_Flags_DisSharedSels_Get (rcb_p)) ? "True" : "False");
    fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_AND_PARALLELISM,
      (Rcb_Flags_AndParallel_Get (rcb_p)) ? "True" : "False");
    fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_MASTER_SEL_NB,
      (Rcb_Flags_MasterSelNB_Get (rcb_p)) ? "True" : "False");

    fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_WORKER_LOCAL_SEL,
      (Rcb_Flags_WorkerLocalSel_Get (rcb_p)) ? "True" : "False");
    fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_WORKER_PREF_GLOBAL,
      (Rcb_Flags_WorkerPreferGlobal_Get (rcb_p)) ? "True" : "False");
    fprintf (f, "%-20s  %u\n",RCB_FILE_TAG_WORKER_MAX_CYCLES,
      Rcb_WorkerMaxCycles_Get (rcb_p));
    fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_WORKER_CURMERIT,
      (Rcb_Flags_WorkerCurMerit_Get (rcb_p)) ? "True" : "False");
    fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_WORKER_NBMERIT,
      (Rcb_Flags_WorkerNBMerit_Get (rcb_p)) ? "True" : "False");
    fprintf (f, "%-20s  %hu\n", RCB_FILE_TAG_WORKER_MERIT_PRCT,
      Rcb_WorkerMeritPrct_Get (rcb_p));
   }

  fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_STEREO,
    (Rcb_Flags_StereoChemistry_Get (rcb_p)) ? "True" : "False");
  fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_PRESERVE,
    (Rcb_Flags_PreserveStructures_Get (rcb_p)) ? "True" : "False");
  fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_STRATBONDS,
    (Rcb_Flags_StrategicBonds_Get (rcb_p)) ? "True" : "False");

  fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_SAVE_STATUS,
    (Rcb_Flags_SaveStatus_Get (rcb_p)) ? "True" : "False");
  fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_SAVE_TRACE,
    (Rcb_Flags_SaveTrace_Get (rcb_p)) ? "True" : "False");
  fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_SAVE_MERITS,
    (Rcb_Flags_SaveMerits_Get (rcb_p)) ? "True" : "False");
  fprintf (f, "%-20s  %hu\n", RCB_FILE_TAG_CHEM_TRACE, 
    Rcb_ChemistryTrace_Get (rcb_p));

  fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_RUNCONTINUE,
    (Rcb_Flags_Continue_Get (rcb_p)) ? "True" : "False");
  fprintf (f, "%-20s  %lu\n", RCB_FILE_TAG_LEAP, 
    Rcb_LeapSize_Get (rcb_p));
 
  fprintf (f, "%-25s  %s\n", RCB_FILE_TAG_DIR_AVL, 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_AVLC))));
  fprintf (f, "%-25s  %s\n", RCB_FILE_TAG_DIR_FG, 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_FNGP))));
  fprintf (f, "%-25s  %s\n", RCB_FILE_TAG_DIR_TEMPL, 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_TMPL))));
  fprintf (f, "%-25s  %s\n", RCB_FILE_TAG_DIR_RXN, 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p,
    FCB_IND_RXNS))));
  fprintf (f, "%-25s  %s\n", RCB_FILE_TAG_DIR_PATH, 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p,
    FCB_IND_PATH))));
  fprintf (f, "%-25s  %s\n", RCB_FILE_TAG_DIR_STAT, 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_STAT))));
  fprintf (f, "%-25s  %s\n", RCB_FILE_TAG_DIR_SUBMIT, 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_SBMT))));
  fprintf (f, "%-25s  %s\n", RCB_FILE_TAG_DIR_TRACE, 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_TRCE))));
  fprintf (f, "%-25s  %s\n", RCB_FILE_TAG_DIR_COVER, 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_COVR))));
  fprintf (f, "%-25s  %s\n", RCB_FILE_TAG_DIR_FORCESEL, 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_FSEL))));

  fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_FILE_COVER,
    (char *) String_Value_Get (Rcb_Name_Get (rcb_p)));
  fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_COVER_DEV,
    (Rcb_Flags_SearchCoverDev_Get (rcb_p)) ? "True" : "False");
  fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_COVER_SLV,
    (Rcb_Flags_SearchCoverSlv_Get (rcb_p)) ? "True" : "False");
  fprintf (f, "%-20s  %u\n", RCB_FILE_TAG_COVER_PRCT, 
    Rcb_CoverPrct_Get (rcb_p));

  fprintf (f, "%-20s  %s\n", RCB_FILE_TAG_FILE_FORCESEL,
    (char *) String_Value_Get (Rcb_Name_Get (rcb_p)));

  fclose (f);

  return (TRUE);
}
/* End of Submission_Dump */ 
  
/****************************************************************************
*  
*  Function Name:                 Submission_Execute
*  
*****************************************************************************/  

void Submission_Execute
  (  
  SubmitCB_t    *submitcb_p
  ) 
{
  Compound_t    *cur_compound_p;             /* Goal compound */
  char          *tracefn;
  Display       *dsp;
  PstCB_t       *pcb_p;
  PstView_t     *pv_p;
  SynViewCB_t   *svcb_p;
  Rcb_t          dflt_rcb;                   /* Temp Rcb from defaults */
  Rcb_t          sbmt_rcb;                   /* Temp Rcb from submit form */
  Rcb_t          stat_rcb;                   /* Temp Rcb from status file */
  RunStats_t     stat_rstat;                 /* Temp RunStats from status */
  Time_t         start_time;
  char           title[128];                 /*  New title of PstViewer */
U32_t cmpinx;

  svcb_p = SynView_Handle_Grab ();
  pcb_p = Pst_ControlHandle_Get ();
  pv_p = SynVCB_PstView_Get (svcb_p);
  start_time = Syn_Time_Get (FALSE);

  /*  Since we are starting a new run, or restarting a saved run,
      we need to destroy the current PST, SymbolTable and PstView,
      if they exist.
  */

if (glob_sel_cmp != NULL) /* set up run and skip all the graphical stuff! */
{
  PstView_Mouse_Remove (pv_p); /* error-prone way to keep track of callbacks - have to keep track of every call to
                                  PstView_Display and precede it with this! */
  Rcb_Destroy (&GGoalRcb);
  Rcb_Copy (&GGoalRcb, Submit_TempRcb_Get (submitcb_p));
  Rcb_Destroy (Submit_TempRcb_Get (submitcb_p));
/**/
  PstView_Reset (pv_p);
/**/
}

else
{

  if (PstCB_TotalExpandedCompounds_Get (pcb_p) > 0)
    {
    PstNode_t     node;

    PstNode_Subgoal_Put (&node, PstCB_Root_Get (pcb_p));
    Pst_Destroy (node);
    SymbolTable_Destroy ();
    PstView_Reset (pv_p);
    PstCB_TotalExpandedCompounds_Put (pcb_p, 0); /* This is NECESSARY for the NEXT time! */
    }

  /*  Change the cursor to the watch.  Ideally, it would be nice to
      restrict any user inputs at this point, but perhaps some other
      time.
  */
  dsp = XtDisplay (Submit_FormDlg_Get (submitcb_p));
  XDefineCursor (dsp, XtWindow (Submit_FormDlg_Get (submitcb_p)), 
    SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_WAIT_W));
  XFlush (dsp);

  /*  Get a temporary copy of the parameters in the submission form.
      We have to do some checking before altering the system variables.
  */
  Rcb_Init (&sbmt_rcb, FALSE);
  Submit_Values_Get (submitcb_p, &sbmt_rcb, FALSE); 
  FileNames_Complete (Rcb_FileCBs_Get (&sbmt_rcb), 
    Rcb_Name_Get (&sbmt_rcb));

  /*  If the libraries have already been loaded, we need to make sure
      that the libraries specified in status file if a restart(continue) run
      or the submission file if a new run are the same ones.
  */
  if (Rcb_Flags_LibsLoaded_Get (&GGoalRcb))
    {
  /*  We have to destroy the KB data and reinitialize with the
      appropriate data (current for new run/legacy for restart).
  */
    AvcLib_Control_Destroy ();
    FuncGroups_Reset ();
/* The use of React_Force_Initialize is deferred until after Status_File_Read (restart)
   or after setting glob_run_date and glob_run_time to PER_TIME_ANY (new run). */
    Rcb_Flags_LibsLoaded_Put (&GGoalRcb, FALSE); /* This is NECESSARY for the NEXT time! */

    if (Rcb_Flags_Restart_Get (&sbmt_rcb) == TRUE)
      {
      Status_File_Peek ((char *) String_Value_Get (FileCB_FileStr_Get (
        Rcb_IthFileCB_Get (&sbmt_rcb, FCB_IND_STAT))), &stat_rcb,
        &stat_rstat);

      /* If the libraries specified in the status file differ from the 
         ones already loaded, ignore request to restart run.
      */
/***** No longer used *****
      if (String_Compare (FileCB_DirStr_Get (Rcb_IthFileCB_Get (&GGoalRcb,
          FCB_IND_AVLC)), FileCB_DirStr_Get (Rcb_IthFileCB_Get (&stat_rcb,
          FCB_IND_AVLC))) != 0 ||
          String_Compare (FileCB_DirStr_Get (Rcb_IthFileCB_Get (&GGoalRcb,
          FCB_IND_FNGP)), FileCB_DirStr_Get (Rcb_IthFileCB_Get (&stat_rcb,
          FCB_IND_FNGP))) != 0 ||
          String_Compare (FileCB_DirStr_Get (Rcb_IthFileCB_Get (&GGoalRcb,
          FCB_IND_RXNS)), FileCB_DirStr_Get (Rcb_IthFileCB_Get (&stat_rcb,
          FCB_IND_RXNS))) != 0)
***** No longer used *****/

if (FALSE)
        {
        Rcb_Destroy (&sbmt_rcb);
        Rcb_Destroy (&stat_rcb);
        RunStats_Destroy (&stat_rstat);

        XUndefineCursor (dsp, XtWindow (Submit_FormDlg_Get (submitcb_p)));
        InfoWarn_Show ("Current libraries differ from those specified\n"
                       "in the specified status file.  Unable to \n"
                       "execute run.");

        return;
        }

      else
        {
        /*  Reload submission forms after reading the status file to reflect 
            changes to the stored run parameters.  Save the username and date.
        */
        PstView_Mouse_Remove (pv_p);
        Rcb_Destroy (&GGoalRcb);
        RunStats_Destroy (&GRunStats);

        Status_File_Read ((char *) String_Value_Get (FileCB_FileStr_Get (
          Rcb_IthFileCB_Get (&sbmt_rcb, FCB_IND_STAT))), &GGoalRcb,
          &GRunStats);
        Submit_Values_Get (submitcb_p, &GGoalRcb, TRUE); 

/***** Now reopen KB *****/
/*
        AvcLib_Init ((char *) String_Value_Get (FileCB_DirStr_Get ( 
          Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_AVLC))), AVC_INIT_EXISTS);
        AvcLib_Init ((char *) String_Value_Get (FileCB_DirStr_Get ( 
          Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_AVLC))), AVC_INIT_INFO);
*/
        AvcLib_Init (FCB_SEQDIR_AVLC (""), AVC_INIT_EXISTS);
        AvcLib_Init (FCB_SEQDIR_AVLC (""), AVC_INIT_INFO);

/* STATUS FILE READ opens rxnlib
        React_Force_Initialize (String_Value_Get (FileCB_DirStr_Get ( 
          Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_RXNS))), TRUE);
*/

        FuncGroups_Init (String_Value_Get (FileCB_DirStr_Get ( 
          Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_FNGP))));
/***** KB reopened *****/

        Rcb_Flags_LibsLoaded_Put (&GGoalRcb, TRUE);
        Rcb_Flags_LibsChanged_Put (&GGoalRcb, FALSE);
        Rcb_Date_Put (&GGoalRcb, Rcb_Date_Get (&sbmt_rcb));
        String_Destroy (Rcb_User_Get (&GGoalRcb));
        Rcb_User_Put (&GGoalRcb, String_Copy (Rcb_User_Get (&sbmt_rcb)));
        Rcb_Destroy (&stat_rcb);
        RunStats_Destroy (&stat_rstat);
        }
      }  /* End of if restart run. */
    else
      {
/***** No longer used *****
      if (String_Compare (FileCB_DirStr_Get (Rcb_IthFileCB_Get (&GGoalRcb,
          FCB_IND_AVLC)), FileCB_DirStr_Get (Rcb_IthFileCB_Get (&sbmt_rcb,
          FCB_IND_AVLC))) != 0 ||
          String_Compare (FileCB_DirStr_Get (Rcb_IthFileCB_Get (&GGoalRcb,
          FCB_IND_FNGP)), FileCB_DirStr_Get (Rcb_IthFileCB_Get (&sbmt_rcb,
          FCB_IND_FNGP))) != 0 ||
          String_Compare (FileCB_DirStr_Get (Rcb_IthFileCB_Get (&GGoalRcb,
          FCB_IND_RXNS)), FileCB_DirStr_Get (Rcb_IthFileCB_Get (&sbmt_rcb,
          FCB_IND_RXNS))) != 0)
        {
        Rcb_Destroy (&sbmt_rcb);
        XUndefineCursor (dsp, XtWindow (Submit_FormDlg_Get (submitcb_p)));
        InfoWarn_Show ("Current libraries differ from those specified\n"
                       "in the submission directory form.  Unable to \n"
                       "execute run.");

        return;
        }
***** No longer used *****/

/* but we need to check directories to prevent a persistence bomb due to use of a non-persistent library! */
      Rcb_Init (&dflt_rcb, FALSE);

      if (String_Compare (FileCB_DirStr_Get (Rcb_IthFileCB_Get (&dflt_rcb,
          FCB_IND_AVLC)), FileCB_DirStr_Get (Rcb_IthFileCB_Get (&sbmt_rcb,
          FCB_IND_AVLC))) != 0 ||
          String_Compare (FileCB_DirStr_Get (Rcb_IthFileCB_Get (&dflt_rcb,
          FCB_IND_FNGP)), FileCB_DirStr_Get (Rcb_IthFileCB_Get (&sbmt_rcb,
          FCB_IND_FNGP))) != 0 ||
          String_Compare (FileCB_DirStr_Get (Rcb_IthFileCB_Get (&dflt_rcb,
          FCB_IND_RXNS)), FileCB_DirStr_Get (Rcb_IthFileCB_Get (&sbmt_rcb,
          FCB_IND_RXNS))) != 0)
        {
        Rcb_Destroy (&dflt_rcb);
        Rcb_Destroy (&sbmt_rcb);
        XUndefineCursor (dsp, XtWindow (Submit_FormDlg_Get (submitcb_p)));
        InfoWarn_Show ("Libraries specified in the submission directory form\n"
                       "are not those expected for a new run.  Unable to \n"
                       "execute run due to likelihood of incompatibility\n"
                       "with persistence.\n\n"
                       "The submit Options menu allows you to reset these\n"
                       "to the expected defaults.");

        return;
        }
      else
        {
        Rcb_Destroy (&dflt_rcb);

        /*  Reload submission forms after initializing the Rcb to reflect 
            changes to the stored run parameters.
        */
        PstView_Mouse_Remove (pv_p);
        Rcb_Init (&GGoalRcb, FALSE);

        Submit_Values_Get (submitcb_p, &GGoalRcb, FALSE); 
        FileNames_Complete (Rcb_FileCBs_Get (&GGoalRcb), 
          Rcb_Name_Get (&GGoalRcb));

/***** Now reset glob_run_date and _time; reopen KB *****/
        glob_run_date = glob_run_time = PER_TIME_ANY;

/*
        AvcLib_Init ((char *) String_Value_Get (FileCB_DirStr_Get ( 
          Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_AVLC))), AVC_INIT_EXISTS);
        AvcLib_Init ((char *) String_Value_Get (FileCB_DirStr_Get ( 
          Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_AVLC))), AVC_INIT_INFO);
*/
        AvcLib_Init (FCB_SEQDIR_AVLC (""), AVC_INIT_EXISTS);
        AvcLib_Init (FCB_SEQDIR_AVLC (""), AVC_INIT_INFO);

        React_Force_Initialize (String_Value_Get (FileCB_DirStr_Get ( 
          Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_RXNS))), TRUE);

        FuncGroups_Init (String_Value_Get (FileCB_DirStr_Get ( 
          Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_FNGP))));
/***** glob_run_date and _time reset; KB reopened *****/

        Rcb_Flags_LibsLoaded_Put (&GGoalRcb, TRUE);
        Rcb_Flags_LibsChanged_Put (&GGoalRcb, FALSE);
        RunStats_Init (&GRunStats);
        }
      }
    }  /* End of if libraries already loaded. */
  else
    {
    PstView_Mouse_Remove (pv_p);

    if (Rcb_Flags_Restart_Get (&sbmt_rcb) == TRUE)
      {
      /*  Reload submission file after reading the status file to reflect 
          changes to the stored run parameters.  Save the username and date.
      */
      Status_File_Read ((char *) String_Value_Get (FileCB_FileStr_Get (
        Rcb_IthFileCB_Get (&sbmt_rcb, FCB_IND_STAT))), &GGoalRcb,
        &GRunStats);
      Submit_Values_Get (submitcb_p, &GGoalRcb, TRUE); 

      Rcb_Date_Put (&GGoalRcb, Rcb_Date_Get (&sbmt_rcb));
      String_Destroy (Rcb_User_Get (&GGoalRcb));
      Rcb_User_Put (&GGoalRcb, String_Copy (Rcb_User_Get (&sbmt_rcb)));
      }
    else
      {
      Rcb_Init (&GGoalRcb, FALSE);
      Submit_Values_Get (submitcb_p, &GGoalRcb, FALSE); 

/* We need to check directories to prevent a persistence bomb due to use of a non-persistent library! */
      Rcb_Init (&dflt_rcb, FALSE);

      if (String_Compare (FileCB_DirStr_Get (Rcb_IthFileCB_Get (&dflt_rcb,
          FCB_IND_AVLC)), FileCB_DirStr_Get (Rcb_IthFileCB_Get (&GGoalRcb,
          FCB_IND_AVLC))) != 0 ||
          String_Compare (FileCB_DirStr_Get (Rcb_IthFileCB_Get (&dflt_rcb,
          FCB_IND_FNGP)), FileCB_DirStr_Get (Rcb_IthFileCB_Get (&GGoalRcb,
          FCB_IND_FNGP))) != 0 ||
          String_Compare (FileCB_DirStr_Get (Rcb_IthFileCB_Get (&dflt_rcb,
          FCB_IND_RXNS)), FileCB_DirStr_Get (Rcb_IthFileCB_Get (&GGoalRcb,
          FCB_IND_RXNS))) != 0)
        {
        Rcb_Destroy (&dflt_rcb);
        XUndefineCursor (dsp, XtWindow (Submit_FormDlg_Get (submitcb_p)));
        InfoWarn_Show ("Libraries specified in the submission directory form\n"
                       "are not those expected for a new run.  Unable to \n"
                       "execute run due to likelihood of incompatibility\n"
                       "with persistence.\n\n"
                       "The submit Options menu allows you to reset these\n"
                       "to the expected defaults.");

        return;
        }
      else
        {
        Rcb_Destroy (&dflt_rcb);

        FileNames_Complete (Rcb_FileCBs_Get (&GGoalRcb), 
          Rcb_Name_Get (&GGoalRcb));
        RunStats_Init (&GRunStats);

        React_Force_Initialize (String_Value_Get (FileCB_DirStr_Get ( 
          Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_RXNS))), TRUE);
        }
      }

/*
    AvcLib_Init ((char *) String_Value_Get (FileCB_DirStr_Get ( 
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_AVLC))), AVC_INIT_EXISTS);
    AvcLib_Init ((char *) String_Value_Get (FileCB_DirStr_Get ( 
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_AVLC))), AVC_INIT_INFO);
*/
    AvcLib_Init (FCB_SEQDIR_AVLC (""), AVC_INIT_EXISTS);
    AvcLib_Init (FCB_SEQDIR_AVLC (""), AVC_INIT_INFO);

/* Conditional: only if a new run, because restart calls Status_File_Read, which in turn calls React_Force_Initialize
    React_Initialize (String_Value_Get (FileCB_DirStr_Get ( 
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_RXNS))), TRUE);
*/

/* "Spiff" this up later, if necessary */
    Persist_Open (FCB_SEQDIR_RXNS ("/rkbstd.inx"), NULL, NULL, FALSE);

    FuncGroups_Init (String_Value_Get (FileCB_DirStr_Get ( 
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_FNGP))));

    Rcb_Flags_LibsLoaded_Put (&GGoalRcb, TRUE);
    Rcb_Flags_LibsChanged_Put (&GGoalRcb, FALSE);
    }  /* End of else libraries not loaded. */

  Rcb_Destroy (&sbmt_rcb);
}

/* Finally!  We're done with the graphics and can get down to business! */

  Strategy_Init (Rcb_Strategy_Get (&GGoalRcb), Rcb_NTCL_Get (&GGoalRcb));

  /*  Open the trace file, if necessary.  */
  if (Rcb_Flags_SaveTrace_Get (&GGoalRcb))
    {
    tracefn = (char *) String_Value_Get (FileCB_FileStr_Get (
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_TRCE)));
    if (Rcb_ChemistryTrace_Get (&GGoalRcb) < TL_RUNSTATS)
      Rcb_ChemistryTrace_Put (&GGoalRcb, TL_RUNSTATS);
    }
  else
    {
#ifdef _CYGWIN_
    tracefn = "NUL:";
#else
    tracefn = "/dev/null";
#endif
    Rcb_ChemistryTrace_Put (&GGoalRcb, TL_NONE);
    }
  
  GTrace[DB_CHEMISTRY].options = Rcb_ChemistryTrace_Get (&GGoalRcb);
  IO_Init_Files (tracefn, Rcb_Flags_Restart_Get (&GGoalRcb));
  RunStats_Flags_TraceSaved_Put (&GRunStats, TRUE);

  if (Rcb_Flags_SaveTrace_Get (&GGoalRcb))
    {
    Rcb_Dump (&GGoalRcb, &GTraceFile);
    fflush (IO_FileHandle_Get (&GTraceFile));
    }
  /*  Temporary hack to make sure symbol table is initialized to NULL
      in case a status file with a smaller symbol table is attempted
      to be destroyed.
  */

  else 
    SymbolTable_Init ();

  /*  If this is a new run, initialize the problem solving tree
      (must be done after the functional groups are initialized.
  */
  if (Rcb_Flags_Restart_Get (&GGoalRcb) == FALSE)
    {
    Sling_t        target_sling;               /* Goal sling */
    SymTab_t      *symtab_p;                   /* Goal symbol table */
    Xtr_t         *xtr_p;                      /* Goal XTR */

    /* Form the canonical Sling_t and Xtr_t for the goal compound and 
       add the hydrogen atoms.
       - Do not _Destroy target_sling, it is preserved in the RCB
       - Make sure it gets into the symbol table too
       - Load the goal compound into the PST (initialize PST and symbol table)
    */
    target_sling = Rcb_Goal_Get (&GGoalRcb);

    TRACE (R_MAIN, DB_MAIN, TL_SELECT, (outbuf,
       "Goal sling : %s", Sling_Name_Get (target_sling)));

    xtr_p = Sling2Xtr_PlusHydrogen (target_sling);
    Xtr_Attr_ResonantBonds_Set (xtr_p);
    Xtr_Name_Set (xtr_p, NULL);
    target_sling = Name_Sling_Get (xtr_p, 
       Rcb_Flags_StereoChemistry_Get (&GGoalRcb) == TRUE ? FALSE : TRUE);   

    Xtr_Destroy (xtr_p);
    xtr_p = Sling_CanonicalName2Xtr (target_sling);
    cur_compound_p = Pst_Root_Set (xtr_p, target_sling);
    symtab_p = PstComp_SymbolTable_Get (cur_compound_p);
    SymTab_Merit_Initial_Put (symtab_p, SymTab_Merit_Main_Get (symtab_p));
    Xtr_Destroy (xtr_p);
    }

  else
    {
    cur_compound_p = PstSubg_Son_Get (PstCB_Root_Get (pcb_p));
    PstCB_MainTarget_Put (pcb_p, 
      SymTab_Sling_Get (PstComp_SymbolTable_Get (cur_compound_p)));
    }

  Time_Add (&start_time, Syn_Time_Get (TRUE));
  Timer_Init_Put (RunStats_CumTimes_Get (&GRunStats), start_time, SET);

/* Oops!  Here's more graphical stuff! */
if (glob_sel_cmp == NULL)
{
  SubmitMenu_Dialogs_PopDown ();

  XtUnmanageChild (Submit_FormDlg_Get (submitcb_p));
  XUndefineCursor (dsp, XtWindow (Submit_FormDlg_Get (submitcb_p)));
  XtUnmanageChild (Submit_FormDlg_Get (submitcb_p));
}

  /* ****>   the synthesis search begins here   <*****/

  Synchem_Search (cur_compound_p, &GGoalRcb, &GRunStats, 
    SynVCB_CycleDg_Get (svcb_p),  SynVCB_TopApp_Get (svcb_p));

  /*  Save the status file.  Can't save the status file--alters pst.
  if (Rcb_Flags_SaveStatus_Get (&GGoalRcb))
    {
    RunStats_Flags_StatusSaved_Put (&GRunStats, TRUE);
    Status_File_Write ((char *)String_Value_Get (FileCB_FileStr_Get (
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_STAT))), &GGoalRcb,
      &GRunStats);
    }
  */

  /*  Dump run status to trace file.  */
  if (Rcb_Flags_SaveTrace_Get (&GGoalRcb))
    {
    RunStats_Dump (&GRunStats, &GTraceFile, 
      (Rcb_RunType_Get (&GGoalRcb) >= RCB_TYPE_DIS_START));
    fflush (IO_FileHandle_Get (&GTraceFile));
    }

/* Oops again!  Don't initialize pst_view for expansion!
if (glob_sel_cmp == NULL)
{
*/
  if (!PstView_Tree_Init (pv_p, Pst_ControlHandle_Get ()))
    InfoWarn_Show (PVW_NOTINIT_WARN);
  else
    {
    if (PstCB_Root_Get (pcb_p) != NULL 
        && PstSubg_Son_Get (PstCB_Root_Get (pcb_p)) != NULL)
      {
      PstVLvls_SetAll (pv_p, PstSubg_Son_Get (PstCB_Root_Get (pcb_p)), 0);
      PstView_Display (pv_p);

      if (XtIsManaged (RxnPreV_FormDlg_Get (ViewLvl_RxnPV_Get (
          PstView_PathVLvl_Get (pv_p)))) && !PstView_PthEHAct_Get (pv_p))
        {
        XtAddEventHandler (DrawCxt_DA_Get (PstView_PathDC_Get (pv_p)),  
          PointerMotionMask, False, PathView_Preview_EH, (XtPointer) pv_p);
        PstView_PthEHAct_Put (pv_p, TRUE);
        }

      if (PstView_NumActPV_Get (pv_p) > 0 && !PstView_PstEHAct_Get (pv_p))
        {
        XtAddEventHandler (DrawCxt_DA_Get (PstView_PstDC_Get (pv_p)),  
          PointerMotionMask, False, PstView_Preview_EH, (XtPointer) pv_p);
        PstView_PstEHAct_Put (pv_p, TRUE);
        }
      }

    else
      {
      InfoWarn_Show (PVW_NOROOT_WARN);
      }
    }
/*
}
*/

  sprintf (title, "SYNCHEM --- %s", 
    (char *) String_Value_Get (Rcb_Name_Get (&GGoalRcb)));
  XtVaSetValues (SynVCB_TopApp_Get (svcb_p), XmNtitle, title, NULL);

if (glob_sel_cmp != NULL)
{
  cmpinx=PstComp_Index_Get (glob_sel_cmp);
  SS_WalkPath (cmpinx, FileCB_FileStr_Get(Rcb_IthFileCB_Get(&GGoalRcb, FCB_IND_PATH)));
}

  return;
}
/* End of Submission_Execute */ 

/****************************************************************************
*  
*  Function Name:                 Submit_IsDistrib
*  
*****************************************************************************/  

Boolean_t Submit_IsDistrib
  (
  SubmitCB_t     *submitcb_p
  )
{
  WidgetList      toggles;

  XtVaGetValues (RunTypeCB_SeqDisRB_Get (Submit_RunTypeCB_Get (submitcb_p)),
    XmNchildren,    &toggles,
    NULL);

  if (XmToggleButtonGetState (toggles[SMU_DISTRIBUTED_RB_IND]))
    return (TRUE);

  return (FALSE);
}
/* End of Submit_IsDistrib */ 

/****************************************************************************
*  
*  Function Name:                 Submit_Values_Get
*
*     Ignore directory, strategy and file modifications to continued runs. 
*  
*****************************************************************************/  

void Submit_Values_Get
  (
  SubmitCB_t     *submitcb_p,
  Rcb_t          *rcb_p,
  Boolean_t       continuation
  )
{
  if (!continuation)
    {

    SubmitMenu_Forms_Get (rcb_p);

    }

  RunType_Values_Get (Submit_RunTypeCB_Get (submitcb_p), rcb_p);
  ExitCond_Values_Get (Submit_ExitCondCB_Get (submitcb_p), rcb_p); 
  SavePlan_Values_Get (&saveplan_cb);
  JobInfo_Values_Get (Submit_JobInfoCB_Get (submitcb_p), rcb_p);

  return;
}
/* End of Submit_Values_Get */ 

/****************************************************************************
*  
*  Function Name:                 Submit_Values_Set
*  
*****************************************************************************/  

void Submit_Values_Set
  (
  SubmitCB_t     *submitcb_p,
  Rcb_t          *rcb_p
  )
{
  RunType_Values_Set (Submit_RunTypeCB_Get (submitcb_p), rcb_p);
  ExitCond_Values_Set (Submit_ExitCondCB_Get (submitcb_p), rcb_p); 
  JobInfo_Values_Set (Submit_JobInfoCB_Get (submitcb_p), rcb_p); 

  SubmitMenu_Forms_Set (rcb_p);

  return;
}
/* End of Submit_Values_Set */ 

/****************************************************************************
*  
*  Function Name:                 Submit_CancelPB_CB
*  
*****************************************************************************/  

void Submit_CancelPB_CB
  (  
  Widget          button, 
  XtPointer       client_data, 
  XtPointer       call_data 
  ) 
{
  SubmitCB_t     *submitcb_p;

  submitcb_p = (SubmitCB_t *) client_data;

  Menu_Choice_CB (button, (XtPointer) DRW_MENU_DELETE_ALL, (XtPointer) NULL);

  SubmitMenu_Dialogs_PopDown ();

  XtUnmanageChild (Submit_FormDlg_Get (submitcb_p));

  return;
}
/* End of Submit_CancelPB_CB */ 

/****************************************************************************
*  
*  Function Name:                 Submit_NumVerify_CB
*  
*****************************************************************************/  

void Submit_NumVerify_CB
  (
  Widget         w,
  XtPointer      client_data,
  XtPointer      call_data
  )
{
  XmTextVerifyCallbackStruct *cbs = (XmTextVerifyCallbackStruct * ) call_data;

  /* Test all characters of any new input to be sure they are 
     digits only.  Set doit to FALSE if any character fails.
  */
  if (cbs->text->ptr)
  {
    char *string = cbs->text->ptr;
    int i;

    for (i=0; i < cbs->text->length; i++)
      if (!isdigit (string[i]))
      {
      cbs->doit = FALSE;
      XBell (XtDisplay (w), 5);
      return;
      } 
   }

  return;
}
/* End of Submit_NumVerify_CB */ 

/****************************************************************************
*  
*  Function Name:                 Submit_ResetPB_CB
*  
*****************************************************************************/  

void Submit_ResetPB_CB
  (  
  Widget          button, 
  XtPointer       client_data, 
  XtPointer       call_data 
  ) 
{
  SubmitCB_t     *sbmtcb_p;

  sbmtcb_p = (SubmitCB_t *) client_data;

  Rcb_Destroy (Submit_TempRcb_Get (sbmtcb_p));
  Rcb_Init (Submit_TempRcb_Get (sbmtcb_p), FALSE);
  Submit_Values_Set (sbmtcb_p, Submit_TempRcb_Get (sbmtcb_p));
  Menu_Choice_CB (button, (XtPointer) DRW_MENU_DELETE_ALL, (XtPointer) NULL);

  return;
}
/* End of Submit_ResetPB_CB */ 

/****************************************************************************
*  
*  Function Name:                 Submit_SubmitPB_CB
*  
*****************************************************************************/  

void Submit_SubmitPB_CB
  (  
  Widget         w, 
  XtPointer      client_data, 
  XtPointer      call_data 
  ) 
{
  PstCB_t       *pcb_p;
  SubmitCB_t    *submitcb_p;
  Rcb_t          sbmt_rcb;                   /* Temp Rcb from submit form */

  submitcb_p = (SubmitCB_t *) client_data;
  pcb_p = Pst_ControlHandle_Get ();

  /*  Make sure the sling has been generated for the current drawing.  */
  Done_CB (w, (XtPointer) Submit_JobInfoCB_Get (submitcb_p), 
    (XtPointer) NULL);

  Menu_Choice_CB (w, (XtPointer) DRW_MENU_DELETE_ALL, (XtPointer) NULL);

  SubmitMenu_Dialogs_PopDown ();

  XtUnmanageChild (Submit_FormDlg_Get (submitcb_p));

  /*  If this is a distributed job submission, just save submit file.  */
  if (Submit_IsDistrib (submitcb_p))
    {
    char         mess[256];

    /*  Initialize run control block for distributed run.  */
    Rcb_Init (&sbmt_rcb, TRUE);
    Submit_Values_Get (submitcb_p, &sbmt_rcb, FALSE); 
    Submission_Dump (&sbmt_rcb);
    sprintf (mess, "Submission file for distributed run saved:\n  %s",
      (char *) String_Value_Get (FileCB_FileStr_Get (
      Rcb_IthFileCB_Get (&sbmt_rcb, FCB_IND_SBMT))));
    InfoMess_Show (mess);

    Rcb_Destroy (&sbmt_rcb);
    return;
    }

  /*  Check to see if there is an unsaved PST in the viewer.  */
  if (PstCB_TotalExpandedCompounds_Get (pcb_p) > 0
      && Rcb_Flags_SaveStatus_Get (&GGoalRcb) 
      && RunStats_Flags_PstChanged_Get (&GRunStats))
    {
    SVF_QueryDg_Update (SVF_QUERY_TYPE_SAVESTAT, SVF_COMMAND_TYPE_EXEC, 
      client_data);
    }
  
  else
    {
    RunStats_Flags_PstChanged_Put (&GRunStats, FALSE);
    Submission_Execute (submitcb_p);
    }

  return;
}
/* End of Submit_SubmitPB_CB */ 


/* End of SUBMIT.C */ 
