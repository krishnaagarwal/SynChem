/****************************************************************************
*  
* Copyright (C) 1999 Synchem Group at SUNY-Stony Brook, Jerry Miller
*  
*  
*  Module Name:               VALCOMP.C
*  
*    Creates and manages the job submission tool.  
*      
*  Creation Date:  
*  
*     28-Oct-1999  
*  
*  Authors: 
*      Jerry Miller
*        based on version by Daren Krebsbach 
*  
*  Routines:
*
*    ValCompEntry_Create
*
*    ValComp_CancelPB_CB
*    ValComp_SubmitPB_CB
*    ValComp_ResetPB_CB
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
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

#ifndef _H_SYNHELP_
#include "synhelp.h"
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

#ifndef _H_SYN_VIEW_
#include "syn_view.h"
#endif

#ifndef _H_EXTERN_
#include "extern.h"
#endif

#ifndef _H_AVLFORM_
#include "avlform.h"
#endif

static int         schema_num;
static SShotView_t sview;
static Widget      helppb;
static SubmitCB_t *glob_submitcb_p;

void ValComp_CancelPB_CB (Widget, XtPointer, XtPointer);
void ValComp_ResetPB_CB (Widget, XtPointer, XtPointer);
void ValComp_SubmitPB_CB (Widget, XtPointer, XtPointer);
void ValComp_DSTempPB_CB (Widget, XtPointer, XtPointer);
Widget ValCompMenu_Create (SubmitCB_t *, Widget); 
void Schema_Test_Incomplete_Flag ();
void modify_drawtool (SubmitCB_t *, Boolean_t);

void Modify_Done_Lbl (char *); /* in submit_draw2.c */
/* void CmpInst_Drawn_Sling (char *); */ /* in cmp_inst2.c - now obsolete, due to submit conflict */
void DrawPad_Drawn_Sling (char *); /* in drawpad.c, which is a separate process from syn_view, thus avoiding the conflict! */

/****************************************************************************
*  
*  Function Name:                 ValCompEntry_Create
*  
*****************************************************************************/  

void  ValCompEntry_Create 
  ( 
  SubmitCB_t    *submitcb_p, 
  Widget         parent, 
  XtAppContext  *appcon,
  int            schema
  ) 
{ 
  ScreenAttr_t  *scra_p;                  /* Screen Attritbutes */
  Widget         menubar, runtype, exitcond, jobinfo, drawtool; 
  Widget         formdg, separator, appshell; 
  XmString       lbl_str;
  Dimension      job_info_ht;
int i,j;
String_t temp_str;
Sling_t canon_sling;
   
  schema_num = schema;
  glob_submitcb_p = submitcb_p;
  if (Submit_Created_Get (submitcb_p))
    {
    modify_drawtool (submitcb_p, schema >= 0);
    SShotView_Reset (&sview);
if (schema < 0)
{
printf("Submit_SubmitPB_Get(submitcb_p)=%d\n", Submit_SubmitPB_Get(submitcb_p));
  XtUnmanageChild (Submit_SubmitPB_Get (submitcb_p));
  glob_special = TRUE;
  done_pb_function = ValComp_SubmitPB_CB;
/*
  if (sshot_comp[0]!='\0')
  {
    temp_str = String_Create (sshot_comp, 0);
    canon_sling = String2Sling (temp_str);
    String_Destroy (temp_str);

    XmTextSetString (JobInfo_SlingTxt_Get (Submit_JobInfoCB_Get (submitcb_p)),
      (char *) Sling_Name_Get (canon_sling));

printf("before MolDraw_Sling_Draw\n");
    MolDraw_Sling_Draw (canon_sling);
    Sling_Destroy (canon_sling);
  }
else printf("no sling to draw\n");
*/
}
else
{
  XtManageChild (Submit_SubmitPB_Get (submitcb_p));
  glob_special = FALSE;
  done_pb_function = NULL;
}
  if (sshot_comp[0]!='\0')
  {
    temp_str = String_Create (sshot_comp, 0);
    canon_sling = String2Sling (temp_str);
    String_Destroy (temp_str);

    XmTextSetString (JobInfo_SlingTxt_Get (Submit_JobInfoCB_Get (submitcb_p)),
      (char *) Sling_Name_Get (canon_sling));

printf("before MolDraw_Sling_Draw\n");
    MolDraw_Sling_Draw (canon_sling);
    Sling_Destroy (canon_sling);
  }
else printf("no sling to draw\n");

    return; 
    }

  SShotView_Create (parent, &sview);

  scra_p = SynAppR_ScrnAtrb_Get (&GSynAppR);

  lbl_str = XmStringCreateLtoR ("Validation Compound Entry", XmFONTLIST_DEFAULT_TAG);
  formdg = XmCreateFormDialog (parent, "ValCompDlg", NULL, 0);

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
    XmNheight,         Screen_Height_Get (scra_p) - 3 * SWS_APPSHELL_OFFY,
    XmNwidth,          Screen_Width_Get (scra_p) - SWS_APPSHELL_OFFX,
   NULL);
  XmStringFree (lbl_str);
  Submit_FormDlg_Put (submitcb_p, formdg);

  separator = XtVaCreateManagedWidget ("ValCompSep", 
    xmSeparatorWidgetClass, formdg, 
    XmNseparatorType,       XmDOUBLE_LINE, 
    XmNshadowThickness,     AppDim_SepLarge_Get (&GAppDim),
    NULL);   

  /* Create the "reset", "cancel" and "submit" pushbuttons. */ 

  lbl_str = XmStringCreateLocalized ("Help");
  helppb = XtVaCreateManagedWidget ("ValCompHelp",
    xmPushButtonWidgetClass, formdg,
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str,
    XmNheight, AppDim_HtLblPB_Get (&GAppDim),
    NULL);
  XmStringFree (lbl_str);

  XtAddCallback (helppb, XmNactivateCallback, Help_CB, (XtPointer) "valcomp:Validation Compound Entry");
   
  lbl_str = XmStringCreateLocalized (SBT_RESET_PB_LBL);  
  Submit_ResetPB_Put (submitcb_p, 
    XtVaCreateManagedWidget ("ValCompPB",  
      xmPushButtonWidgetClass, formdg,  
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,
      XmNheight,               AppDim_HtLblPB_Get (&GAppDim),
      NULL)); 
  XmStringFree (lbl_str);

  XtAddCallback(Submit_ResetPB_Get (submitcb_p), XmNactivateCallback,
    ValComp_ResetPB_CB, (XtPointer) submitcb_p);
   
  lbl_str = XmStringCreateLocalized (SBT_CANCEL_PB_LBL);  
  Submit_CancelPB_Put (submitcb_p, 
    XtVaCreateManagedWidget ("ValCompPB",  
      xmPushButtonWidgetClass, formdg,  
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,
      XmNheight,               AppDim_HtLblPB_Get (&GAppDim),
      NULL)); 
  XmStringFree (lbl_str);
 
  XtAddCallback(Submit_CancelPB_Get (submitcb_p), XmNactivateCallback,  
    ValComp_CancelPB_CB, (XtPointer) submitcb_p);
 
  lbl_str = XmStringCreateLocalized (SBT_SUBMIT_PB_LBL);  
  Submit_SubmitPB_Put (submitcb_p, 
    XtVaCreateManagedWidget ("ValCompPB",  
      xmPushButtonWidgetClass, formdg,  
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str, 
      XmNheight,               AppDim_HtLblPB_Get (&GAppDim),
      NULL));  
  XmStringFree (lbl_str);
  
  XtAddCallback(Submit_SubmitPB_Get (submitcb_p), XmNactivateCallback,  
   ValComp_SubmitPB_CB, (XtPointer) submitcb_p);

  menubar = ValCompMenu_Create (submitcb_p, formdg); 
  jobinfo = JobInfo_Create (Submit_JobInfoCB_Get (submitcb_p), formdg); 
  lbl_str = XmStringCreateLocalized ("draw_sling_templates");  
  XtVaSetValues (JobInfo_CompsPB_Get (Submit_JobInfoCB_Get (submitcb_p)),
    XmNlabelString,          lbl_str,
    NULL);
  XmStringFree (lbl_str);
  XtRemoveAllCallbacks (JobInfo_CompsPB_Get (Submit_JobInfoCB_Get (submitcb_p)), XmNactivateCallback);
  XtAddCallback (JobInfo_CompsPB_Get (Submit_JobInfoCB_Get (submitcb_p)), XmNactivateCallback, ValComp_DSTempPB_CB,
    (XtPointer) submitcb_p);
  XtSetSensitive (JobInfo_CompsPB_Get (Submit_JobInfoCB_Get (submitcb_p)), True);

  drawtool = Draw_Tool_Create (Submit_JobInfoCB_Get (submitcb_p), formdg,
    appcon);

  Rcb_Init (Submit_TempRcb_Get (submitcb_p), FALSE); 
  Rcb_RunType_Put (Submit_TempRcb_Get (submitcb_p), RCB_TYPE_NONE);
  JobInfo_Values_Set (Submit_JobInfoCB_Get (submitcb_p), Submit_TempRcb_Get (submitcb_p));

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

  XtVaSetValues (helppb,
    XmNtopOffset,        -(AppDim_HtLblPB_Get (&GAppDim)
                           + 2 * AppDim_MargFmFr_Get (&GAppDim)),
    XmNtopAttachment,    XmATTACH_OPPOSITE_FORM,
    XmNbottomOffset,     AppDim_MargFmFr_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftPosition,     3,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNrightPosition,     11,
    XmNrightAttachment,   XmATTACH_POSITION,
    NULL);

  XtVaSetValues (Submit_SubmitPB_Get (submitcb_p),
/*
    XmNtopOffset,        -(AppDim_HtLblPB_Get (&GAppDim) 
                           + 2 * AppDim_MargFmFr_Get (&GAppDim)),
    XmNtopAttachment,    XmATTACH_OPPOSITE_FORM,
    XmNbottomOffset,     AppDim_MargFmFr_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_FORM,
*/
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        helppb,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     helppb,
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
/*
    XmNtopWidget,        Submit_SubmitPB_Get (submitcb_p),
*/
    XmNtopWidget,        helppb,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
/*
    XmNbottomWidget,     Submit_SubmitPB_Get (submitcb_p),
*/
    XmNbottomWidget,     helppb,
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
/*
    XmNtopWidget,        Submit_SubmitPB_Get (submitcb_p),
*/
    XmNtopWidget,        helppb,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
/*
    XmNbottomWidget,     Submit_SubmitPB_Get (submitcb_p),
*/
    XmNbottomWidget,     helppb,
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

  XtVaSetValues (drawtool,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        jobinfo,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget,     separator,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  /* Manage Unmanaged children */    
  XtManageChild (menubar); 
  XtManageChild (separator); 
  XtManageChild (jobinfo); 
  XtManageChild (drawtool); 
  
  XtManageChild (formdg);

  modify_drawtool (submitcb_p, schema >= 0);

  Submit_Created_Put (submitcb_p, TRUE); 
printf("before conditions\n");

if (schema < 0)
{
printf("Submit_SubmitPB_Get(submitcb_p)=%d\n", Submit_SubmitPB_Get(submitcb_p));
  XtUnmanageChild (Submit_SubmitPB_Get (submitcb_p));
  glob_special = TRUE;
  done_pb_function = ValComp_SubmitPB_CB;
/*
  if (sshot_comp[0]!='\0')
  {
    temp_str = String_Create (sshot_comp, 0);
    canon_sling = String2Sling (temp_str);
    String_Destroy (temp_str);

    XmTextSetString (JobInfo_SlingTxt_Get (Submit_JobInfoCB_Get (submitcb_p)),
      (char *) Sling_Name_Get (canon_sling));

    MolDraw_Sling_Draw (canon_sling);
    Sling_Destroy (canon_sling);
  }
*/
}
else
{
  XtManageChild (Submit_SubmitPB_Get (submitcb_p));
  glob_special = FALSE;
  done_pb_function = NULL;
}
printf("after conditions\n");
  if (sshot_comp[0]!='\0')
  {
    temp_str = String_Create (sshot_comp, 0);
    canon_sling = String2Sling (temp_str);
    String_Destroy (temp_str);

    XmTextSetString (JobInfo_SlingTxt_Get (Submit_JobInfoCB_Get (submitcb_p)),
      (char *) Sling_Name_Get (canon_sling));

printf("before MolDraw_Sling_Draw\n");
    MolDraw_Sling_Draw (canon_sling);
    Sling_Destroy (canon_sling);
  }
else printf("no sling to draw\n");

  return; 
} 
/* End of ValCompEntry_Create

/****************************************************************************
*  
*  Function Name:                 ValComp_CancelPB_CB
*  
*****************************************************************************/  

void ValComp_CancelPB_CB
  (  
  Widget          button, 
  XtPointer       client_data, 
  XtPointer       call_data 
  ) 
{
  SubmitCB_t     *submitcb_p;

  submitcb_p = (SubmitCB_t *) client_data;

  Menu_Choice_CB (button, (XtPointer) DRW_MENU_DELETE_ALL, (XtPointer) NULL);

  XtUnmanageChild (Submit_FormDlg_Get (submitcb_p));

  if (schema_num == -2)
  {
    printf("canceled\n\n");
    exit (0);
  }

  return;
}
/* End of ValComp_CancelPB_CB */ 

/****************************************************************************
*  
*  Function Name:                 ValComp_ResetPB_CB
*  
*****************************************************************************/  

void ValComp_ResetPB_CB
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
  Rcb_RunType_Put (Submit_TempRcb_Get (sbmtcb_p), RCB_TYPE_NONE);
  JobInfo_Values_Set (Submit_JobInfoCB_Get (sbmtcb_p), Submit_TempRcb_Get (sbmtcb_p));
  Menu_Choice_CB (button, (XtPointer) DRW_MENU_DELETE_ALL, (XtPointer) NULL);

  return;
}
/* End of ValComp_ResetPB_CB */ 

/****************************************************************************
*  
*  Function Name:                 ValComp_SubmitPB_CB
*  
*****************************************************************************/  

void ValComp_SubmitPB_CB
  (  
  Widget         w, 
  XtPointer      client_data, 
  XtPointer      call_data 
  ) 
{
  SubmitCB_t    *submitcb_p;
  String_t       sling_str;
  Sling_t        temp_sling;
  Xtr_t         *xtr;
  int            len;
  char           variable2jump_thru_hoops_with[1000];
  void         (*save_done_pb_function)(Widget, XtPointer, XtPointer);

printf("VC_SPB_CB1\n");
  submitcb_p = (SubmitCB_t *) client_data;
  if (submitcb_p == NULL) submitcb_p = glob_submitcb_p; /* kludge to make Done_CB interchangeable with ValComp_SubmitPB_CB */

  /*  Make sure the sling has been generated for the current drawing.  */
  save_done_pb_function = done_pb_function;
  done_pb_function = NULL;
printf("VC_SPB_CB2\n");
  Done_CB (w, (XtPointer) Submit_JobInfoCB_Get (submitcb_p), (XtPointer) NULL);
printf("VC_SPB_CB3\n");
  done_pb_function = save_done_pb_function;

printf("VC_SPB_CB4\n");
  Menu_Choice_CB (w, (XtPointer) DRW_MENU_DELETE_ALL, (XtPointer) NULL);

printf("VC_SPB_CB5\n");
  XtUnmanageChild (Submit_FormDlg_Get (submitcb_p));

printf("VC_SPB_CB6\n");
  strcpy (variable2jump_thru_hoops_with,
    JobInfo_Sling_Get (Submit_JobInfoCB_Get (submitcb_p)));
  if (schema_num < 0)
    {
printf("VC_SPB_CB7\n");
    if (schema_num == -1) Avl_Drawn_Sling (variable2jump_thru_hoops_with);
/*
    else CmpInst_Drawn_Sling (variable2jump_thru_hoops_with);
*/
    else DrawPad_Drawn_Sling (variable2jump_thru_hoops_with);
printf("VC_SPB_CB8\n");
    return;
    }
  len = strlen (variable2jump_thru_hoops_with);
  if (len==0) return;
  sling_str = String_Create (NULL, len + 1);
  strcpy (String_Value_Get (sling_str), variable2jump_thru_hoops_with);
  String_Length_Put (sling_str, len);
  temp_sling=String2Sling(sling_str);

  xtr = Sling2Xtr /* _PlusHydrogen */ (temp_sling);
  String_Destroy (sling_str);
  if (xtr==NULL) return;
  Xtr_Name_Set (xtr, NULL);
  SShotV_TgtXtr_Put (&sview, xtr);
  SShotV_RxnRec_Put (&sview, React_Schema_Handle_Get (schema_num));

  SingleShot_Apply (SShotV_GenSGs_Get (&sview), temp_sling, schema_num, FALSE, TRUE);

  Schema_Test_Incomplete_Flag ();

  SShotView_Setup (&sview);
  SShotView_Update (&sview, 0);

  XtManageChild (SShotV_FormDlg_Get (&sview, 0));

  return;
}
/* End of ValComp_SubmitPB_CB */ 

/****************************************************************************
*  
*  Function Name:                 ValComp_DSTempPB_CB
*  
*****************************************************************************/  

void ValComp_DSTempPB_CB
  (  
  Widget         w, 
  XtPointer      client_data, 
  XtPointer      call_data 
  )
{
  SVF_FileDg_Update (SVF_FILED_TYPE_OPENDST, client_data, FALSE);
} 
/* End of ValComp_DSTempPB_CB */ 

void modify_drawtool (SubmitCB_t *submitcb_p, Boolean_t normal)
{
  XmString lbl[2];

printf ("modify_drawtool (%p, %d)\n",submitcb_p,normal);
  if (normal)
  {
    Modify_Done_Lbl (CS_DONE);
    lbl[0] = XmStringCreateLocalized (SJI_SLING_LBL);
    lbl[1] = XmStringCreateLocalized (SJI_DRAW_PB_LBL);
    XtManageChild (JobInfo_CommentLbl_Get (Submit_JobInfoCB_Get (submitcb_p)));
    XtManageChild (JobInfo_CommentTxt_Get (Submit_JobInfoCB_Get (submitcb_p)));
    XtManageChild (JobInfo_RunIdLbl_Get (Submit_JobInfoCB_Get (submitcb_p)));
    XtManageChild (JobInfo_RunIdTxt_Get (Submit_JobInfoCB_Get (submitcb_p)));
    XtManageChild (JobInfo_ResetPB_Get (Submit_JobInfoCB_Get (submitcb_p)));
    XtManageChild (Submit_ResetPB_Get (submitcb_p));
  }
  else
  {
printf("m_d0\n");
    Modify_Done_Lbl ("Create Sling & Exit");
printf("m_d1\n");
    lbl[0] = XmStringCreateLocalized ("Draw Buffer (Last Sling Saved)");
    lbl[1] = XmStringCreateLocalized ("Draw Sling in Buffer");
printf("m_d2\n");
    XtUnmanageChild (JobInfo_CommentLbl_Get (Submit_JobInfoCB_Get (submitcb_p)));
    XtUnmanageChild (JobInfo_CommentTxt_Get (Submit_JobInfoCB_Get (submitcb_p)));
printf("m_d3\n");
    XtUnmanageChild (JobInfo_RunIdLbl_Get (Submit_JobInfoCB_Get (submitcb_p)));
    XtUnmanageChild (JobInfo_RunIdTxt_Get (Submit_JobInfoCB_Get (submitcb_p)));
printf("m_d4\n");
    XtUnmanageChild (JobInfo_ResetPB_Get (Submit_JobInfoCB_Get (submitcb_p)));
printf("m_d5\n");
    XtUnmanageChild (Submit_ResetPB_Get (submitcb_p));
printf("m_d6\n");
  }
  XtVaSetValues (JobInfo_SlingLbl_Get (Submit_JobInfoCB_Get (submitcb_p)),
    XmNlabelString, lbl[0],
    NULL);
printf("m_d7\n");
  XmStringFree (lbl[0]);
printf("m_d8\n");
  XtVaSetValues (JobInfo_DrawPB_Get (Submit_JobInfoCB_Get (submitcb_p)),
    XmNlabelString, lbl[1],
    NULL);
printf("m_d9\n");
  XmStringFree (lbl[1]);
printf("m_d10\n");
}

/* End of VALCOMP.C */ 
