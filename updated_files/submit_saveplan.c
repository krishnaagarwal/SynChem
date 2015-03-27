/****************************************************************************
*  
* Copyright (C) 2001 Synchem Group at SUNY-Stony Brook, Gerald A. Miller  
*  
*  
*  Module Name:               SUBMIT_SAVEPLAN.C
*  
*    Creates and updates the save plan panel of the job submission tool.  
*      
*  Creation Date:  
*  
*     04-Sep-2001  
*  
*  Authors: 
*      Gerald A. Miller
*        based on submit_exitcond by Daren Krebsbach
*  
*  Routines:
*
*    SavePlan_Create
*    SavePlan_Values_Get
*    SavePlan_Values_Set
*
*    SavePlan_Reset_CB
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

#ifndef _H_SUBMIT_SAVEPLAN_
#define _GLOBAL_DEF_
#include "submit_saveplan.h" 
#undef _GLOBAL_DEF_
#endif

#ifndef _H_SUBMIT_
#include "submit.h" 
#endif

/****************************************************************************
*  
*  Function Name:                 SavePlan_Create
*  
*****************************************************************************/  

Widget SavePlan_Create
  (
  SavePlanCB_t   *savecb_p,
  Widget          parent
  ) 
{ 
  Widget          form;
  Widget          sep1, sep2, sep3;
  XmString        lbl_str, null_lbl_str;

  SavePlanCB_Frame_Put (savecb_p, XtVaCreateWidget ("SavePlanFr",
    xmFrameWidgetClass, parent,
    XmNshadowType,      XmSHADOW_IN,
    XmNshadowThickness, 3,
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    NULL));

  form = XtVaCreateWidget ("SavePlanFm", 
    xmFormWidgetClass,  SavePlanCB_Frame_Get (savecb_p), 
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       True,
    NULL);
  SavePlanCB_Form_Put (savecb_p, form);

  lbl_str = XmStringCreateLocalized (SSP_SAVEPLAN_TITLE);  
  SavePlanCB_FrameTitle_Put (savecb_p, 
    XtVaCreateManagedWidget ("SavePlanLbl",
      xmLabelGadgetClass,        SavePlanCB_Frame_Get (savecb_p),
      XmNchildType,              XmFRAME_TITLE_CHILD,
      XmNchildVerticalAlignment, XmALIGNMENT_WIDGET_TOP,
      XmNchildHorizontalSpacing, XmALIGNMENT_CENTER, 
      XmNlabelType,              XmSTRING,  
      XmNlabelString,            lbl_str,
      NULL));
  XmStringFree (lbl_str);  
         
  null_lbl_str = XmStringCreateLocalized ("");
  SavePlanCB_CyclesTB_Put (savecb_p,
    XtVaCreateManagedWidget ("SavePlanTB_Cyc", 
      xmToggleButtonWidgetClass, form, 
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      null_lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));

  lbl_str = XmStringCreateLocalized (SSP_PRECYCLES_LBL);  
  SavePlanCB_PreCyclesLbl_Put (savecb_p,
    XtVaCreateManagedWidget ("SavePlanLbL_PreCyc", 
      xmLabelWidgetClass, form, 
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,    False,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);  

  SavePlanCB_Cycles_Get (savecb_p)[0] = '\0';
  SavePlanCB_CyclesTxt_Put (savecb_p,
    XtVaCreateManagedWidget ("SavePlanTxt", 
      xmTextWidgetClass, form,
      XmNeditable,       True,
      XmNeditMode,       XmSINGLE_LINE_EDIT,
      XmNmaxLength,      SSP_MAX_FIELD_SIZE,
      XmNcolumns,        SSP_MAX_FIELD_SIZE, 
      XmNvalue,          SavePlanCB_Cycles_Get (savecb_p),
      XmNmarginHeight,   AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,    AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XtAddCallback (SavePlanCB_CyclesTxt_Get (savecb_p),
    XmNmodifyVerifyCallback, Submit_NumVerify_CB,
    NULL); 

  lbl_str = XmStringCreateLocalized (SSP_POSTCYCLES_LBL);  
  SavePlanCB_PostCyclesLbl_Put (savecb_p,
    XtVaCreateManagedWidget ("SavePlanLbl_PostCyc",  
      xmLabelWidgetClass,  form, 
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,    False,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str); 
 
  lbl_str = XmStringCreateLocalized (SSP_BEFOREAFTER_LBL);  
  SavePlanCB_BeforeAfterLbl_Put (savecb_p,
    XtVaCreateManagedWidget ("SavePlanLbL_BefAft", 
      xmLabelWidgetClass, form, 
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,    False,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);  

  SavePlanCB_AfterCycles_Get (savecb_p)[0] = '\0';
  SavePlanCB_AfterCyclesTxt_Put (savecb_p,
    XtVaCreateManagedWidget ("SavePlanTxt_After", 
      xmTextWidgetClass, form,
      XmNeditable,       True,
      XmNeditMode,       XmSINGLE_LINE_EDIT,
      XmNmaxLength,      SSP_MAX_FIELD_SIZE,
      XmNcolumns,        SSP_MAX_FIELD_SIZE, 
      XmNvalue,          SavePlanCB_Cycles_Get (savecb_p),
      XmNmarginHeight,   AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,    AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XtAddCallback (SavePlanCB_AfterCyclesTxt_Get (savecb_p),
    XmNmodifyVerifyCallback, Submit_NumVerify_CB,
    NULL); 

  lbl_str = XmStringCreateLocalized (SSP_AFTERAFTER_LBL);  
  SavePlanCB_AfterAfterLbl_Put (savecb_p,
    XtVaCreateManagedWidget ("SavePlanLbl_AftAft",  
      xmLabelWidgetClass,  form, 
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,    False,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str); 
 
  SavePlanCB_MaxMaxTB_Put (savecb_p,
    XtVaCreateManagedWidget ("SavePlanTB_Max", 
      xmToggleButtonWidgetClass, form, 
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      null_lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));

  lbl_str = XmStringCreateLocalized (SSP_MAXTIME_MAXCYC_LBL);  
  SavePlanCB_MaxMaxLbl_Put (savecb_p, 
    XtVaCreateManagedWidget ("SavePlanLbl_Max",  
      xmLabelWidgetClass,  form,  
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,    False,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL)); 
  XmStringFree (lbl_str);  

  SavePlanCB_FirstSolTB_Put (savecb_p,
    XtVaCreateManagedWidget ("SavePlanTB_1st", 
      xmToggleButtonWidgetClass, form, 
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      null_lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (null_lbl_str);

  lbl_str = XmStringCreateLocalized (SSP_FIRSTSOL_SAVE_LBL);  
  SavePlanCB_FirstSolLbl_Put (savecb_p,
    XtVaCreateManagedWidget ("SavePlanLbl_1st", 
      xmLabelWidgetClass,  form,  
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,    False,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);  

  sep1 = XtVaCreateManagedWidget ("SavePlanSep",  
    xmSeparatorWidgetClass, form,
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmSHADOW_ETCHED_IN,
    XmNshadowThickness,     AppDim_SepSmall_Get (&GAppDim),
    NULL);

  sep2 = XtVaCreateManagedWidget ("SavePlanSep2",  
    xmSeparatorWidgetClass, form,
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmSHADOW_ETCHED_IN,
    XmNshadowThickness,     AppDim_SepSmall_Get (&GAppDim),
    NULL);

  sep3 = XtVaCreateManagedWidget ("SavePlanSep3",  
    xmSeparatorWidgetClass, form,
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmDOUBLE_LINE,
    XmNshadowThickness,     AppDim_SepLarge_Get (&GAppDim),
    NULL);

  lbl_str = XmStringCreateLocalized ("reset");  
  SavePlanCB_ResetPB_Put (savecb_p, 
    XtVaCreateManagedWidget ("SavePlanPB",  
      xmPushButtonWidgetClass, form, 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));
  XmStringFree (lbl_str);  

  XtAddCallback (SavePlanCB_ResetPB_Get (savecb_p), XmNactivateCallback, 
    SavePlan_Reset_CB, (XtPointer) savecb_p);

  XtVaSetValues (SavePlanCB_CyclesTB_Get (savecb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_NONE,
    NULL);

  XtVaSetValues (SavePlanCB_PreCyclesLbl_Get (savecb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        SavePlanCB_CyclesTB_Get (savecb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     SavePlanCB_CyclesTB_Get (savecb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       SavePlanCB_CyclesTB_Get (savecb_p),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_NONE,
    NULL);

  XtVaSetValues (SavePlanCB_CyclesTxt_Get (savecb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_NONE,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     SavePlanCB_PreCyclesLbl_Get (savecb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       SavePlanCB_PreCyclesLbl_Get (savecb_p),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_NONE,
    NULL);

  XtVaSetValues (SavePlanCB_PostCyclesLbl_Get (savecb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        SavePlanCB_CyclesTB_Get (savecb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     SavePlanCB_CyclesTB_Get (savecb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       SavePlanCB_CyclesTxt_Get (savecb_p),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_NONE,
    NULL);

  XtVaSetValues (SavePlanCB_AfterCyclesTxt_Get (savecb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        SavePlanCB_CyclesTxt_Get (savecb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       SavePlanCB_CyclesTxt_Get (savecb_p),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
    XmNrightWidget,      SavePlanCB_CyclesTxt_Get (savecb_p),
    NULL);

  XtVaSetValues (SavePlanCB_BeforeAfterLbl_Get (savecb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_NONE,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     SavePlanCB_AfterCyclesTxt_Get (savecb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_NONE,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      SavePlanCB_AfterCyclesTxt_Get (savecb_p),
    NULL);

  XtVaSetValues (SavePlanCB_AfterAfterLbl_Get (savecb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_NONE,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     SavePlanCB_AfterCyclesTxt_Get (savecb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       SavePlanCB_AfterCyclesTxt_Get (savecb_p),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_NONE,
    NULL);

  XtVaSetValues (sep1,
    XmNtopOffset,        AppDim_MargFmFr_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        SavePlanCB_AfterCyclesTxt_Get (savecb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (SavePlanCB_MaxMaxTB_Get (savecb_p),
    XmNtopOffset,        AppDim_MargFmFr_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        sep1,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_NONE,
    NULL);

  XtVaSetValues (SavePlanCB_MaxMaxLbl_Get (savecb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        SavePlanCB_MaxMaxTB_Get (savecb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     SavePlanCB_MaxMaxTB_Get (savecb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       SavePlanCB_MaxMaxTB_Get (savecb_p),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_NONE,
    NULL);

  XtVaSetValues (sep2,
    XmNtopOffset,        AppDim_MargFmFr_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        SavePlanCB_MaxMaxTB_Get (savecb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (SavePlanCB_FirstSolTB_Get (savecb_p),
    XmNtopOffset,        AppDim_MargFmFr_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        sep2,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_NONE,
    NULL);

  XtVaSetValues (SavePlanCB_FirstSolLbl_Get (savecb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        SavePlanCB_FirstSolTB_Get (savecb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     SavePlanCB_FirstSolTB_Get (savecb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       SavePlanCB_FirstSolTB_Get (savecb_p),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_NONE,
    NULL);

  XtVaSetValues (sep3,
    XmNtopOffset,        AppDim_MargFmFr_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        SavePlanCB_FirstSolTB_Get (savecb_p),
    XmNbottomOffset,     AppDim_MargFmFr_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (SavePlanCB_ResetPB_Get (savecb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        sep3,
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

  SavePlan_Reset_CB (NULL, (XtPointer) savecb_p, NULL);

  return (SavePlanCB_Frame_Get (savecb_p)); 
}  
/*  End of SavePlan_Create  */

/****************************************************************************
*  
*  Function Name:                 SavePlan_Values_Get
*  
*****************************************************************************/  

void SavePlan_Values_Get
  (
  SavePlanCB_t    *savecb_p
  )
{
  U32_t            number;
  char            *str_p;
  
  if (XmToggleButtonGetState (SavePlanCB_CyclesTB_Get (savecb_p)))
    {
    RTRcb_CycleSave_Flag_Put (&GRTRcb, TRUE);
    /** Get number of save cycle increment **/
    str_p = XmTextGetString (SavePlanCB_CyclesTxt_Get (savecb_p));
    if (str_p == NULL || sscanf (str_p, "%lu", &number) != 1 || number == 0)
      {
      RTRcb_CycleSave_Flag_Put (&GRTRcb, FALSE);
      InfoWarn_Show ("Invalid number for save cycle increment.");
      XtFree (str_p);
      return;
      }

    RTRcb_CycleIncr_Put (&GRTRcb, number);
    XtFree (str_p);

    /** Get number of start cycle for save **/
    str_p = XmTextGetString (SavePlanCB_AfterCyclesTxt_Get (savecb_p));
    if (str_p == NULL || sscanf (str_p, "%lu", &number) != 1)
      {
      RTRcb_CycleSave_Flag_Put (&GRTRcb, FALSE);
      InfoWarn_Show ("Invalid number for save start cycle number.");
      XtFree (str_p);
      return;
      }

    RTRcb_CycleStart_Put (&GRTRcb, number);
    XtFree (str_p);

  /** Get cycle save to TRUE or FALSE **/
    }
  else
    {
    RTRcb_CycleSave_Flag_Put (&GRTRcb, FALSE);
    }

  /** Get final save to TRUE or FALSE **/
  if (XmToggleButtonGetState (SavePlanCB_MaxMaxTB_Get (savecb_p)))
    {
    RTRcb_FinalSave_Flag_Put (&GRTRcb, TRUE);
    }
  else
    {
    RTRcb_FinalSave_Flag_Put (&GRTRcb, FALSE);
    }

  /** Get first solution to TRUE or FALSE **/
  if (XmToggleButtonGetState (SavePlanCB_FirstSolTB_Get (savecb_p)))
    {
    RTRcb_1stSolSave_Flag_Put (&GRTRcb, TRUE);
    }
  else
    {
    RTRcb_1stSolSave_Flag_Put (&GRTRcb, FALSE);
    }

  return;
}
/*  End of SavePlan_Values_Get  */

/****************************************************************************
*  
*  Function Name:                 SavePlan_Values_Set
*  
*****************************************************************************/  

void SavePlan_Values_Set
  (
  SavePlanCB_t   *savecb_p
  )
{  
  /** Set number of cycle increment **/
  sprintf (SavePlanCB_Cycles_Get (savecb_p), "%lu",  
    RTRcb_CycleIncr_Get (&GRTRcb));
#ifdef _CYGWIN_
  XmTextSetString (SavePlanCB_CyclesTxt_Get (savecb_p),
    SavePlanCB_Cycles_Get (savecb_p));
#else
  XtVaSetValues (SavePlanCB_CyclesTxt_Get (savecb_p),
    XmNvalue, SavePlanCB_Cycles_Get (savecb_p),
    NULL);
#endif

  /** Set number of start cycle **/
  sprintf (SavePlanCB_AfterCycles_Get (savecb_p), "%lu",  
    RTRcb_CycleStart_Get (&GRTRcb));
#ifdef _CYGWIN_
  XmTextSetString (SavePlanCB_AfterCyclesTxt_Get (savecb_p),
    SavePlanCB_AfterCycles_Get (savecb_p));
#else
  XtVaSetValues (SavePlanCB_AfterCyclesTxt_Get (savecb_p),
    XmNvalue, SavePlanCB_AfterCycles_Get (savecb_p),
    NULL);
#endif

  /** Set cycle save to TRUE or FALSE **/
  XtVaSetValues (SavePlanCB_CyclesTB_Get (savecb_p),
    XmNset, RTRcb_CycleSave_Flag_Get (&GRTRcb),
    NULL);

  /** Set final save to TRUE or FALSE **/
  XtVaSetValues (SavePlanCB_MaxMaxTB_Get (savecb_p),
    XmNset, RTRcb_FinalSave_Flag_Get (&GRTRcb),
    NULL);

  /** Set first solution save to TRUE or FALSE **/
  XtVaSetValues (SavePlanCB_FirstSolTB_Get (savecb_p),
    XmNset, RTRcb_1stSolSave_Flag_Get (&GRTRcb),
    NULL);

  return;
}
/*  End of SavePlan_Values_Set  */

/****************************************************************************
*  
*  Function Name:                 SavePlan_Reset_CB
*  
*****************************************************************************/  

void SavePlan_Reset_CB
(
  Widget    w,   
  XtPointer client_data,   
  XtPointer call_data
)
{
  SavePlanCB_t   *savecb_p;

  savecb_p = (SavePlanCB_t *) client_data;
  RTRcb_CycleSave_Flag_Put (&GRTRcb, FALSE);
  RTRcb_FinalSave_Flag_Put (&GRTRcb, TRUE);
  RTRcb_1stSolSave_Flag_Put (&GRTRcb, FALSE);
  RTRcb_CycleIncr_Put (&GRTRcb, 5000);
  RTRcb_CycleStart_Put (&GRTRcb, 5000);
  SavePlan_Values_Set (savecb_p);

   return;
}
/*  End of SavePlan_Reset_CB  */

/*  End of SUBMIT_EXITCOND.C  */

