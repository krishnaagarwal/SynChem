/****************************************************************************
*  
* Copyright (C) 1997 Synchem Group at SUNY-Stony Brook, Daren Krebsbach  
*  
*  
*  Module Name:               SUBMIT_RUNTYPE.C
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
*    RunType_Create
*    RunType_Values_Get
*    RunType_Values_Set
*
*    RunType_Reset_CB
*
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
*
*******************************************************************************/  
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/ToggleB.h> 
#include <Xm/Frame.h> 
#include <Xm/Form.h>
#include <Xm/Separator.h>

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
 
#ifndef _H_SUBMIT_RUNTYPE_
#include "submit_runtype.h" 
#endif

#ifndef _H_SUBMIT_
#include "submit.h" 
#endif

/****************************************************************************
*  
*  Function Name:                 RunType_Create
*  
*****************************************************************************/  

Widget RunType_Create
  (
  RunTypeCB_t    *runtypecb_p,
  Widget          parent
  ) 
{ 
  Widget          ssep, sep;
  Widget          title;
  XmString        lbl_str; 
  XmString        lbl_seq, lbl_dis; 

  RunTypeCB_Frame_Put (runtypecb_p, XtVaCreateWidget ("RunTypeFr",
    xmFrameWidgetClass, parent,
    XmNshadowType,      XmSHADOW_IN,
    XmNshadowThickness, 3,
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    NULL));

  RunTypeCB_Form_Put (runtypecb_p, XtVaCreateWidget ("RunTypeFm", 
    xmFormWidgetClass, RunTypeCB_Frame_Get (runtypecb_p), 
    XmNmarginHeight,   AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,    AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,      False,
    NULL)); 

  lbl_str = XmStringCreateLocalized (SMU_RUNTYPE_TITLE);  
  title = XtVaCreateManagedWidget ("RunTypeLbl",
    xmLabelGadgetClass,        RunTypeCB_Frame_Get (runtypecb_p),
    XmNchildType,              XmFRAME_TITLE_CHILD,
    XmNchildVerticalAlignment, XmALIGNMENT_WIDGET_TOP,
    XmNchildHorizontalSpacing, XmALIGNMENT_CENTER, 
    XmNlabelType,              XmSTRING,  
    XmNlabelString,            lbl_str,   
    NULL);
  XmStringFree (lbl_str);  

  RunTypeCB_TypeFrame_Put (runtypecb_p, XtVaCreateWidget ("RunTypeFr", 
      xmFrameWidgetClass, RunTypeCB_Form_Get (runtypecb_p), 
      XmNshadowType,      XmSHADOW_ETCHED_OUT, 
      XmNshadowThickness, 3,
      XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
      XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
      NULL));

   RunTypeCB_TypeForm_Put (runtypecb_p, XtVaCreateWidget ("RunTypeFm", 
      xmFormWidgetClass,  RunTypeCB_TypeFrame_Get (runtypecb_p), 
      XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
      XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
      XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
      XmNresizable,       True,
      NULL));

  lbl_seq = XmStringCreateLocalized (SMU_SEQUENTIAL_RB_LBL); 
  lbl_dis = XmStringCreateLocalized (SMU_DISTRIBUTED_RB_LBL); 
 
  RunTypeCB_SeqDisRB_Put (runtypecb_p, 
     XmVaCreateSimpleRadioBox (RunTypeCB_TypeForm_Get (runtypecb_p), 
       "RunTypeRB",     0,        NULL,
       XmVaRADIOBUTTON, lbl_seq,  NULL, NULL, NULL, 
       XmVaRADIOBUTTON, lbl_dis,  NULL, NULL, NULL, 
       NULL)); 
  XmStringFree(lbl_seq); 
  XmStringFree(lbl_dis); 
 
  ssep = XtVaCreateManagedWidget ("RunTypeSep",  
    xmSeparatorWidgetClass, RunTypeCB_TypeForm_Get (runtypecb_p), 
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmSHADOW_ETCHED_IN,
    XmNshadowThickness,     AppDim_SepSmall_Get (&GAppDim),
    NULL);

  lbl_str = XmStringCreateLocalized (SMU_CONTINUE_TB_LBL);  
  RunTypeCB_ContinueTB_Put (runtypecb_p,    
    XtVaCreateManagedWidget ("RunTypeTB",  
      xmToggleButtonWidgetClass, RunTypeCB_TypeForm_Get (runtypecb_p), 
      XmNlabelType,              XmSTRING,  
      XmNlabelString,            lbl_str,   
      XmNalignment,              XmALIGNMENT_BEGINNING,
      XmNmarginHeight,           AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,            AppDim_MargLblPB_Get (&GAppDim),
      NULL));  
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLocalized (SMU_BACKGROUND_TB_LBL);  
  RunTypeCB_BackGroundTB_Put (runtypecb_p,    
    XtVaCreateManagedWidget ("RunTypeTB",  
      xmToggleButtonWidgetClass, RunTypeCB_TypeForm_Get (runtypecb_p), 
      XmNlabelType,              XmSTRING,  
      XmNlabelString,            lbl_str,   
      XmNalignment,              XmALIGNMENT_BEGINNING,
      XmNmarginHeight,           AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,            AppDim_MargLblPB_Get (&GAppDim),
      NULL));  
  XmStringFree (lbl_str);

  XtVaSetValues (RunTypeCB_SeqDisRB_Get (runtypecb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (ssep,
    XmNtopOffset,        AppDim_MargFmFr_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        RunTypeCB_SeqDisRB_Get (runtypecb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);
 
  XtVaSetValues (RunTypeCB_ContinueTB_Get (runtypecb_p),
    XmNtopOffset,        AppDim_MargFmFr_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        ssep,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);
 
  XtVaSetValues (RunTypeCB_BackGroundTB_Get (runtypecb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        RunTypeCB_ContinueTB_Get (runtypecb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);
 
  XtManageChild (RunTypeCB_SeqDisRB_Get (runtypecb_p)); 
  XtManageChild (RunTypeCB_TypeForm_Get (runtypecb_p)); 
  XtManageChild (RunTypeCB_TypeFrame_Get (runtypecb_p)); 


  RunTypeCB_SearchFrame_Put (runtypecb_p, XtVaCreateWidget ("RunTypeFr", 
    xmFrameWidgetClass, RunTypeCB_Form_Get (runtypecb_p), 
    XmNshadowType,      XmSHADOW_ETCHED_OUT, 
    XmNshadowThickness, 3,
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    NULL));

  RunTypeCB_SearchForm_Put (runtypecb_p, XtVaCreateWidget ("RunTypeFm", 
    xmFormWidgetClass,  RunTypeCB_SearchFrame_Get (runtypecb_p), 
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       False,
    NULL));

  lbl_str = XmStringCreateLocalized (SMU_EFFORT_DIST_TB_LBL);  
  RunTypeCB_EffDistTB_Put (runtypecb_p, 
    XtVaCreateManagedWidget ("RunTypeTB",  
      xmToggleButtonWidgetClass, RunTypeCB_SearchForm_Get (runtypecb_p), 
      XmNlabelType,              XmSTRING,  
      XmNlabelString,            lbl_str,   
      XmNalignment,              XmALIGNMENT_BEGINNING,
      XmNmarginHeight,           AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,            AppDim_MargLblPB_Get (&GAppDim),
      NULL));  
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLocalized (SMU_NTCL_LBL);  
  RunTypeCB_NtclLbl_Put (runtypecb_p, 
    XtVaCreateManagedWidget ("RunTypeLbl",
      xmLabelWidgetClass, RunTypeCB_SearchForm_Get (runtypecb_p), 
      XmNlabelType,       XmSTRING,  
      XmNlabelString,     lbl_str,   
      XmNalignment,       XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,   False,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));  
  XmStringFree (lbl_str);

  RunTypeCB_NtclBuf_Get (runtypecb_p)[0] = '\0';
  RunTypeCB_NtclTxt_Put (runtypecb_p,
    XtVaCreateManagedWidget ("RunTypeTxt", 
      xmTextWidgetClass, RunTypeCB_SearchForm_Get (runtypecb_p),
      XmNeditable,       True,
      XmNeditMode,       XmSINGLE_LINE_EDIT,
      XmNmaxLength,      SMU_NTCL_BUFF_MAXLEN,
      XmNcolumns,        SMU_NTCL_BUFF_MAXLEN, 
      XmNvalue,          RunTypeCB_NtclBuf_Get (runtypecb_p),
      XmNmarginHeight,   AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,    AppDim_MargLblPB_Get (&GAppDim),
     NULL));
  XtAddCallback (RunTypeCB_NtclTxt_Get (runtypecb_p),
    XmNmodifyVerifyCallback, Submit_NumVerify_CB,
    NULL); 

  XtVaSetValues (RunTypeCB_EffDistTB_Get (runtypecb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (RunTypeCB_NtclTxt_Get (runtypecb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        RunTypeCB_EffDistTB_Get (runtypecb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_NONE,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (RunTypeCB_NtclLbl_Get (runtypecb_p),
    XmNtopOffset,        AppDim_MargLblPB_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        RunTypeCB_NtclTxt_Get (runtypecb_p),
    XmNbottomOffset,     AppDim_MargLblPB_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     RunTypeCB_NtclTxt_Get (runtypecb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      RunTypeCB_NtclTxt_Get (runtypecb_p),
    NULL);

  XtManageChild (RunTypeCB_SearchForm_Get (runtypecb_p)); 
  XtManageChild (RunTypeCB_SearchFrame_Get (runtypecb_p)); 

  sep = XtVaCreateManagedWidget ("RunTypeSep",  
    xmSeparatorWidgetClass, RunTypeCB_Form_Get (runtypecb_p), 
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmDOUBLE_LINE,
    XmNshadowThickness,     AppDim_SepLarge_Get (&GAppDim),
    NULL);

  lbl_str = XmStringCreateLocalized (SMU_RESET_PB_LBL);  
  RunTypeCB_ResetPB_Put (runtypecb_p, 
    XtVaCreateManagedWidget("RunTypePB",  
      xmPushButtonWidgetClass, RunTypeCB_Form_Get (runtypecb_p), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);

  XtAddCallback (RunTypeCB_ResetPB_Get (runtypecb_p),
     XmNactivateCallback, RunType_Reset_CB, (XtPointer) runtypecb_p);


  XtVaSetValues (RunTypeCB_TypeFrame_Get (runtypecb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (RunTypeCB_SearchFrame_Get (runtypecb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        RunTypeCB_TypeFrame_Get (runtypecb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (sep,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        RunTypeCB_SearchFrame_Get (runtypecb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (RunTypeCB_ResetPB_Get (runtypecb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        sep,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     30,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    70,
    NULL);

  RunTypeCB_ShowDistPar_Put (runtypecb_p, FALSE);
  XtManageChild (RunTypeCB_Form_Get (runtypecb_p)); 
 
  return (RunTypeCB_Frame_Get (runtypecb_p)); 
} 
/*  End of RunType_Create  */


/****************************************************************************
*  
*  Function Name:                 RunType_Values_Get
*  
*****************************************************************************/  

void RunType_Values_Get
  (
  RunTypeCB_t   *runtypecb_p,
  Rcb_t         *rcb_p
  )

{
  char          *str_p;
  U32_t          number;
  WidgetList     toggles;

  /** Get the run type distributed or sequential **/
  XtVaGetValues (RunTypeCB_SeqDisRB_Get (runtypecb_p),
    XmNchildren,    &toggles,
    NULL);

  if (XmToggleButtonGetState (toggles[SMU_SEQUENTIAL_RB_IND]))
    Rcb_RunType_Put (rcb_p, RCB_TYPE_SEQ);
  else if (XmToggleButtonGetState (toggles[SMU_DISTRIBUTED_RB_IND]))
    Rcb_RunType_Put (rcb_p, RCB_TYPE_DIS_LIN);
  else
    Rcb_RunType_Put (rcb_p, RCB_TYPE_NONE);
    
  /** Get the Background Process Flag **/
  if (XmToggleButtonGetState (RunTypeCB_BackGroundTB_Get (runtypecb_p)))
    {
    Rcb_Flags_Background_Put (rcb_p, TRUE);
    }
  else
    {
    Rcb_Flags_Background_Put (rcb_p, FALSE);
    }

  /** Get the Continue Search Flag **/
  if (XmToggleButtonGetState (RunTypeCB_ContinueTB_Get (runtypecb_p)))
    {
    Rcb_Flags_Restart_Put (rcb_p, TRUE);
    }
  else
    {
    Rcb_Flags_Restart_Put (rcb_p, FALSE);
    }

 /** Get the Effort Distribution Flag **/
  if (XmToggleButtonGetState (RunTypeCB_EffDistTB_Get (runtypecb_p)))
    {
    Rcb_Flags_EffortDis_Put (rcb_p, TRUE);
    }
  else
    {
    Rcb_Flags_EffortDis_Put (rcb_p, FALSE);
    }

  /** Get number of temp closed cycles (NTCL) **/
  str_p = XmTextGetString (RunTypeCB_NtclTxt_Get (runtypecb_p));
  if (str_p == NULL || sscanf (str_p, "%lu", &number) != 1)
    {
    InfoWarn_Show ("Temporary closure must be an integer between 0 and 255.");
    XtFree (str_p);
    return;
    }

  if (number > 0xff)
    Rcb_NTCL_Put (rcb_p, 0xff);
  else
    Rcb_NTCL_Put (rcb_p, (U8_t) number);

  XtFree (str_p);
  return;
}
/*  End of RunType_Values_Get  */

/****************************************************************************
*  
*  Function Name:                 RunType_Values_Set
*  
*****************************************************************************/  

void RunType_Values_Set
  (
  RunTypeCB_t   *runtypecb_p,
  Rcb_t         *rcb_p
  )

{
  WidgetList     toggles;

  /** Set the run type distributed or sequential **/
  XtVaGetValues (RunTypeCB_SeqDisRB_Get (runtypecb_p),
    XmNchildren,    &toggles,
    NULL);

  if (Rcb_RunType_Get (rcb_p) == RCB_TYPE_DIS_LIN)
    {
    XmToggleButtonSetState (toggles[SMU_DISTRIBUTED_RB_IND], True, True);
    RunTypeCB_ShowDistPar_Put (runtypecb_p, TRUE);
    }
  else
    {
    XmToggleButtonSetState (toggles[SMU_SEQUENTIAL_RB_IND], True, True);
    RunTypeCB_ShowDistPar_Put (runtypecb_p, FALSE);
    }

  /** Set the Background Process Flag **/
  XtVaSetValues (RunTypeCB_BackGroundTB_Get (runtypecb_p),
    XmNset, Rcb_Flags_Background_Get (rcb_p),
    NULL);

  /** Set the Continue Search Flag **/
  XtVaSetValues (RunTypeCB_ContinueTB_Get (runtypecb_p),
    XmNset, Rcb_Flags_Restart_Get (rcb_p),
    NULL);

  /** Set the Effort Distribution Flag **/
  XtVaSetValues (RunTypeCB_EffDistTB_Get (runtypecb_p),
    XmNset, Rcb_Flags_EffortDis_Get (rcb_p),
    NULL);
 
  /** Set number of temp closed cycles (NTCL) **/
  sprintf (RunTypeCB_NtclBuf_Get (runtypecb_p), "%hu", Rcb_NTCL_Get (rcb_p));
  XmTextSetString (RunTypeCB_NtclTxt_Get (runtypecb_p),
    RunTypeCB_NtclBuf_Get (runtypecb_p));

  return;
}
/*  End of RunType_Values_Set  */


/****************************************************************************
*  
*  Function Name:                 RunType_Reset_CB
*  
*****************************************************************************/  

void RunType_Reset_CB
(
  Widget    w,   
  XtPointer client_data,   
  XtPointer call_data
)
{
  Rcb_t          rcb;
  RunTypeCB_t   *runtypecb_p;

  runtypecb_p = (RunTypeCB_t *) client_data;
  Rcb_Init (&rcb, FALSE);
  RunType_Values_Set (runtypecb_p, &rcb);
  Rcb_Destroy (&rcb);

  return;
}
/*  End of RunType_Reset_CB  */
 
/*  End of SUBMIT_RUNTYPE.C  */







