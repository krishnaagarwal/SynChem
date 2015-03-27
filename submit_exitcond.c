/****************************************************************************
*  
* Copyright (C) 1997 Synchem Group at SUNY-Stony Brook, Daren Krebsbach  
*  
*  
*  Module Name:               SUBMIT_EXITCOND.C
*  
*    Creates and updates the exit conditions panel of the job submission tool.  
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
*    ExitCond_Create
*    ExitCond_Values_Get
*    ExitCond_Values_Set
*
*    ExitCond_Reset_CB
*
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
#include <Xm/PushB.h> 
#include <Xm/Form.h> 
#include <Xm/Separator.h> 
#include <Xm/Text.h> 
#include <Xm/Label.h> 
#include <Xm/LabelG.h> 
#include <Xm/ToggleB.h> 
#include <Xm/Frame.h> 
 
#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif

#include "synchem.h"
#include "debug.h"
#include "synio.h"
 
#ifndef _H_RCB_
#include "rcb.h"
#endif

#ifndef _H_APP_RESRC_
#include "app_resrc.h"
#endif

#ifndef _H_INFO_VIEW_
#include "info_view.h"
#endif

#ifndef _H_SUBMIT_EXITCOND_
#include "submit_exitcond.h" 
#endif

#ifndef _H_SUBMIT_
#include "submit.h" 
#endif

/****************************************************************************
*  
*  Function Name:                 ExitCond_Create
*  
*****************************************************************************/  

Widget ExitCond_Create
  (
  ExitCondCB_t   *exitcb_p,
  Widget          parent
  ) 
{ 
  Widget          form;
  Widget          sep1, sep2;
  XmString        lbl_str;

  ExitCondCB_Frame_Put (exitcb_p, XtVaCreateWidget ("ExitCondFr",
    xmFrameWidgetClass, parent,
    XmNshadowType,      XmSHADOW_IN,
    XmNshadowThickness, 3,
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    NULL));

  form = XtVaCreateWidget ("ExitCondFm", 
    xmFormWidgetClass,  ExitCondCB_Frame_Get (exitcb_p), 
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       True,
    NULL);
  ExitCondCB_Form_Put (exitcb_p, form);

  lbl_str = XmStringCreateLocalized (SEC_EXITCOND_TITLE);  
  ExitCondCB_FrameTitle_Put (exitcb_p, 
    XtVaCreateManagedWidget ("ExitCondLbl",
      xmLabelGadgetClass,        ExitCondCB_Frame_Get (exitcb_p),
      XmNchildType,              XmFRAME_TITLE_CHILD,
      XmNchildVerticalAlignment, XmALIGNMENT_WIDGET_TOP,
      XmNchildHorizontalSpacing, XmALIGNMENT_CENTER, 
      XmNlabelType,              XmSTRING,  
      XmNlabelString,            lbl_str,
      NULL));
  XmStringFree (lbl_str);  
         
  lbl_str = XmStringCreateLocalized (SEC_MAXCYCLES_LBL);  
  ExitCondCB_MaxCyclesLbl_Put (exitcb_p,
    XtVaCreateManagedWidget ("ExitCondLbl",  
      xmLabelWidgetClass,  form, 
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,    False,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str); 
 
  ExitCondCB_MaxCycles_Get (exitcb_p)[0] = '\0';
  ExitCondCB_MaxCyclesTxt_Put (exitcb_p,
    XtVaCreateManagedWidget ("ExitCondTxt", 
      xmTextWidgetClass, form,
      XmNeditable,       True,
      XmNeditMode,       XmSINGLE_LINE_EDIT,
      XmNmaxLength,      SEC_MAX_FIELD_SIZE,
      XmNcolumns,        SEC_MAX_FIELD_SIZE, 
      XmNvalue,          ExitCondCB_MaxCycles_Get (exitcb_p),
      XmNmarginHeight,   AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,    AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XtAddCallback (ExitCondCB_MaxCyclesTxt_Get (exitcb_p),
    XmNmodifyVerifyCallback, Submit_NumVerify_CB,
    NULL); 

  lbl_str = XmStringCreateLocalized (SEC_MAXTIME_LBL);  
  ExitCondCB_MaxTimeLbl_Put (exitcb_p, 
    XtVaCreateManagedWidget ("ExitCondLbl",  
      xmLabelWidgetClass,  form,  
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,    False,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL)); 
  XmStringFree (lbl_str);  

  ExitCondCB_MaxTime_Get (exitcb_p)[0] = '\0';
  ExitCondCB_MaxTimeTxt_Put (exitcb_p, 
    XtVaCreateManagedWidget ("ExitCondTxt", 
      xmTextWidgetClass,  form,
      XmNeditable,        True,
      XmNeditMode,        XmSINGLE_LINE_EDIT,
      XmNmaxLength,       SEC_MAX_FIELD_SIZE,
      XmNcolumns,         SEC_MAX_FIELD_SIZE, 
      XmNvalue,           ExitCondCB_MaxTime_Get (exitcb_p),
      XmNmarginHeight,    AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,     AppDim_MargLblPB_Get (&GAppDim),
      NULL)); 
  XtAddCallback (ExitCondCB_MaxTimeTxt_Get (exitcb_p),
    XmNmodifyVerifyCallback, Submit_NumVerify_CB,
    NULL); 


  lbl_str = XmStringCreateLocalized (SEC_FIRSTSOL_RB_LBL);  
  ExitCondCB_FirstSolTB_Put (exitcb_p,
    XtVaCreateManagedWidget ("ExitCondTB", 
      xmToggleButtonWidgetClass, form, 
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);  

  lbl_str = XmStringCreateLocalized (SEC_ADDCYCLES_LBL);  
  ExitCondCB_AddCyclesLbl_Put (exitcb_p, 
    XtVaCreateManagedWidget ("ExitCondLbl",  
      xmLabelWidgetClass,  form,  
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,    False,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);  

  ExitCondCB_AddCycles_Get (exitcb_p)[0] = '\0';
  ExitCondCB_AddCyclesTxt_Put (exitcb_p,
    XtVaCreateManagedWidget("ExitCondTxt", 
      xmTextWidgetClass, form,
      XmNeditable,       True,
      XmNeditMode,       XmSINGLE_LINE_EDIT,
      XmNmaxLength,      SEC_MAX_FIELD_SIZE,
      XmNcolumns,        SEC_MAX_FIELD_SIZE, 
      XmNvalue,          ExitCondCB_AddCycles_Get (exitcb_p),
      XmNmarginHeight,   AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,    AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XtAddCallback (ExitCondCB_AddCyclesTxt_Get (exitcb_p),
    XmNmodifyVerifyCallback, Submit_NumVerify_CB,
    NULL); 

  lbl_str = XmStringCreateLocalized (SEC_ADDTIME_LBL);  
  ExitCondCB_AddTimeLbl_Put (exitcb_p,
    XtVaCreateManagedWidget ("ExitCondLbl",  
      xmLabelWidgetClass,  form,  
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,    False,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);  

  ExitCondCB_AddTime_Get (exitcb_p)[0] = '\0';
  ExitCondCB_AddTimeTxt_Put (exitcb_p,
    XtVaCreateManagedWidget ("ExitCondTxt", 
      xmTextWidgetClass, form,
      XmNeditable,       True,
      XmNeditMode,       XmSINGLE_LINE_EDIT,
      XmNmaxLength,      SEC_MAX_FIELD_SIZE,
      XmNcolumns,        SEC_MAX_FIELD_SIZE, 
      XmNvalue,          ExitCondCB_AddTime_Get (exitcb_p),
      XmNmarginHeight,   AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,    AppDim_MargLblPB_Get (&GAppDim),
     NULL));
  XtAddCallback (ExitCondCB_AddTimeTxt_Get (exitcb_p),
    XmNmodifyVerifyCallback, Submit_NumVerify_CB,
    NULL); 

  sep1 = XtVaCreateManagedWidget ("ExitCondSep",  
    xmSeparatorWidgetClass, form,
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmSHADOW_ETCHED_IN,
    XmNshadowThickness,     AppDim_SepSmall_Get (&GAppDim),
    NULL);

  sep2 = XtVaCreateManagedWidget ("ExitCondSep",  
    xmSeparatorWidgetClass, form,
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmDOUBLE_LINE,
    XmNshadowThickness,     AppDim_SepLarge_Get (&GAppDim),
    NULL);

  lbl_str = XmStringCreateLocalized ("reset");  
  ExitCondCB_ResetPB_Put (exitcb_p, 
    XtVaCreateManagedWidget ("ExitCondPB",  
      xmPushButtonWidgetClass, form, 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));
  XmStringFree (lbl_str);  

  XtAddCallback (ExitCondCB_ResetPB_Get (exitcb_p), XmNactivateCallback, 
    ExitCond_Reset_CB, (XtPointer) exitcb_p);

  XtVaSetValues (ExitCondCB_MaxCyclesTxt_Get (exitcb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_NONE,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (ExitCondCB_MaxCyclesLbl_Get (exitcb_p),
    XmNtopOffset,        AppDim_MargLblPB_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     AppDim_MargLblPB_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     ExitCondCB_MaxCyclesTxt_Get (exitcb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      ExitCondCB_MaxCyclesTxt_Get (exitcb_p),
    NULL);

  XtVaSetValues (ExitCondCB_MaxTimeTxt_Get (exitcb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        ExitCondCB_MaxCyclesTxt_Get (exitcb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       ExitCondCB_MaxCyclesTxt_Get (exitcb_p),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (ExitCondCB_MaxTimeLbl_Get (exitcb_p),
    XmNtopOffset,        AppDim_MargLblPB_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        ExitCondCB_MaxCyclesLbl_Get (exitcb_p),
    XmNbottomOffset,     AppDim_MargLblPB_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     ExitCondCB_MaxTimeTxt_Get (exitcb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      ExitCondCB_MaxTimeTxt_Get (exitcb_p),
    NULL);

  XtVaSetValues (sep1,
    XmNtopOffset,        AppDim_MargFmFr_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        ExitCondCB_MaxTimeTxt_Get (exitcb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (ExitCondCB_FirstSolTB_Get (exitcb_p),
    XmNtopOffset,        AppDim_MargFmFr_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        sep1,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (ExitCondCB_AddCyclesTxt_Get (exitcb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        ExitCondCB_FirstSolTB_Get (exitcb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_NONE,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (ExitCondCB_AddCyclesLbl_Get (exitcb_p),
    XmNtopOffset,        AppDim_MargLblPB_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        ExitCondCB_FirstSolTB_Get (exitcb_p),
    XmNbottomOffset,     AppDim_MargLblPB_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     ExitCondCB_AddCyclesTxt_Get (exitcb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      ExitCondCB_AddCyclesTxt_Get (exitcb_p),
    NULL);

  XtVaSetValues (ExitCondCB_AddTimeTxt_Get (exitcb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        ExitCondCB_AddCyclesTxt_Get (exitcb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_NONE,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (ExitCondCB_AddTimeLbl_Get (exitcb_p),
    XmNtopOffset,        AppDim_MargLblPB_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        ExitCondCB_AddCyclesLbl_Get (exitcb_p),
    XmNbottomOffset,     AppDim_MargLblPB_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     ExitCondCB_AddTimeTxt_Get (exitcb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      ExitCondCB_AddTimeTxt_Get (exitcb_p),
    NULL);

  XtVaSetValues (sep2,
    XmNtopOffset,        AppDim_MargFmFr_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        ExitCondCB_AddTimeTxt_Get (exitcb_p),
    XmNbottomOffset,     AppDim_MargFmFr_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (ExitCondCB_ResetPB_Get (exitcb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        sep2,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     30,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    70,
    NULL);


  XtManageChild (form);

  return (ExitCondCB_Frame_Get (exitcb_p)); 
}  
/*  End of ExitCond_Create  */

/****************************************************************************
*  
*  Function Name:                 ExitCond_Values_Get
*  
*****************************************************************************/  

void ExitCond_Values_Get
  (
  ExitCondCB_t    *exitcb_p,
   Rcb_t          *rcb_p
  )
{
  U32_t            number;
  char            *str_p;
  
  /** Get number of max cycles **/
  str_p = XmTextGetString (ExitCondCB_MaxCyclesTxt_Get (exitcb_p));
  if (str_p == NULL || sscanf (str_p, "%lu", &number) != 1)
    {
    InfoWarn_Show ("Invalid number for maximum cycles.");
    XtFree (str_p);
    return;
    }

  Rcb_MaxCycles_Put (rcb_p, number);
  XtFree (str_p);

  /** Get number of max run time **/
  str_p = XmTextGetString (ExitCondCB_MaxTimeTxt_Get (exitcb_p));
  if (str_p == NULL || sscanf (str_p, "%lu", &number) != 1)
    {
    InfoWarn_Show ("Invalid number for maximum run time (minutes).");
    XtFree (str_p);
    return;
    }

  Rcb_MaxRuntime_Put (rcb_p, number);
  XtFree (str_p);

  /** Get first solution to TRUE or FALSE **/
  if (XmToggleButtonGetState (ExitCondCB_FirstSolTB_Get (exitcb_p)))
    {
    Rcb_Flags_FirstSol_Put (rcb_p, TRUE);
    }
  else
    {
    Rcb_Flags_FirstSol_Put (rcb_p, FALSE);
    }

  /** Get Additional Cycles value **/
  str_p = XmTextGetString (ExitCondCB_AddCyclesTxt_Get (exitcb_p));
  if (str_p == NULL || sscanf (str_p, "%lu", &number) != 1)
    {
    InfoWarn_Show ("Invalid number for additional cycles.");
    XtFree (str_p);
    return;
    }

  Rcb_AddCycles_Put (rcb_p, number);
  XtFree (str_p);

  /** Get Additional Time value **/
  str_p = XmTextGetString (ExitCondCB_AddTimeTxt_Get (exitcb_p));
  if (str_p == NULL || sscanf (str_p, "%lu", &number) != 1)
    {
    InfoWarn_Show ("Invalid number for additional run time (minutes).");
    XtFree (str_p);
    return;
    }

  Rcb_AddTime_Put (rcb_p, number);
  XtFree (str_p);

  return;
}
/*  End of ExitCond_Values_Get  */

/****************************************************************************
*  
*  Function Name:                 ExitCond_Values_Set
*  
*****************************************************************************/  

void ExitCond_Values_Set
  (
  ExitCondCB_t   *exitcb_p,
  Rcb_t          *rcb_p
  )
{  
  /** Set number of max cycles **/
  sprintf (ExitCondCB_MaxCycles_Get (exitcb_p), "%lu",  
    Rcb_MaxCycles_Get (rcb_p));
#ifdef _CYGWIN_
  XmTextSetString (ExitCondCB_MaxCyclesTxt_Get (exitcb_p),
    ExitCondCB_MaxCycles_Get (exitcb_p));
#else
  XtVaSetValues (ExitCondCB_MaxCyclesTxt_Get (exitcb_p),
    XmNvalue, ExitCondCB_MaxCycles_Get (exitcb_p),
    NULL);
#endif

  /** Set number of max run time **/
  sprintf (ExitCondCB_MaxTime_Get (exitcb_p), "%lu",  
    Rcb_MaxRuntime_Get (rcb_p));
#ifdef _CYGWIN_
  XmTextSetString (ExitCondCB_MaxTimeTxt_Get (exitcb_p),
    ExitCondCB_MaxTime_Get (exitcb_p));
#else
  XtVaSetValues (ExitCondCB_MaxTimeTxt_Get (exitcb_p),
    XmNvalue, ExitCondCB_MaxTime_Get (exitcb_p),
    NULL);
#endif

  /** Set first solution to TRUE or FALSE **/
  XtVaSetValues (ExitCondCB_FirstSolTB_Get (exitcb_p),
    XmNset, Rcb_Flags_FirstSol_Get (rcb_p),
    NULL);

  /** Set Additional Cycles value **/
  sprintf (ExitCondCB_AddCycles_Get (exitcb_p), "%lu",  
    Rcb_AddCycles_Get (rcb_p));
#ifdef _CYGWIN_
  XmTextSetString (ExitCondCB_AddCyclesTxt_Get (exitcb_p),
    ExitCondCB_AddCycles_Get (exitcb_p));
#else
  XtVaSetValues (ExitCondCB_AddCyclesTxt_Get (exitcb_p),
    XmNvalue, ExitCondCB_AddCycles_Get (exitcb_p),
    NULL);
#endif

  /** Set Additional Time value **/
  sprintf (ExitCondCB_AddTime_Get (exitcb_p), "%lu",  
    Rcb_AddTime_Get (rcb_p));
#ifdef _CYGWIN_
  XmTextSetString (ExitCondCB_AddTimeTxt_Get (exitcb_p),
    ExitCondCB_AddTime_Get (exitcb_p));
#else
  XtVaSetValues (ExitCondCB_AddTimeTxt_Get (exitcb_p),
    XmNvalue, ExitCondCB_AddTime_Get (exitcb_p),
    NULL);
#endif

  return;
}
/*  End of ExitCond_Values_Set  */

/****************************************************************************
*  
*  Function Name:                 ExitCond_Reset_CB
*  
*****************************************************************************/  

void ExitCond_Reset_CB
(
  Widget    w,   
  XtPointer client_data,   
  XtPointer call_data
)
{
  ExitCondCB_t   *exitcb_p;
  Rcb_t           temp_rcb;

  exitcb_p = (ExitCondCB_t *) client_data;
  Rcb_Init (&temp_rcb, FALSE);
  ExitCond_Values_Set (exitcb_p, &temp_rcb);
  Rcb_Destroy (&temp_rcb);

   return;
}
/*  End of ExitCond_Reset_CB  */

/*  End of SUBMIT_EXITCOND.C  */

