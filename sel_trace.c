/******************************************************************************
*
*  Copyright (C) 1996, Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     SEL_TRACE.C
*
*    This module defines the routines needed to trace the selection of
*    compounds to expand in a given status file and display it in the PST
*    viewer.
*
*  Creation Date:
*
*    10-Jan-1996
*
*  Authors:
*
*    Daren Krebsbach
*
*  Routines:
*
*    SelTrace_Create
*    SelTrace_Cycles_Set
*    SelTrace_Init
*    SelTrace_Reset
*    SelTrace_Setup
*    SelTrace_Update
*
*  Callback Routines:
*
*    SelTrace_Clear_CB
*    SelTrace_CmpSel_CB
*    SelTrace_CmpText_CB
*    SelTrace_CycleNext_CB
*    SelTrace_CyclePrev_CB
*    SelTrace_CycleText_CB
*    SelTrace_Dismiss_CB
*    SelTrace_NumAll_CB
*
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
*
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include <Xm/ArrowB.h>
#include <Xm/DialogS.h>
#include <Xm/Text.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/Form.h>

#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_PST_
#include "pst.h"
#endif

#ifndef _H_APP_RESRC_
#include "app_resrc.h"
#endif

#ifndef _H_INFO_VIEW_
#include "info_view.h"
#endif

#ifndef _H_PST_VIEW_
#include "pst_view.h"
#endif

#ifndef _H_SEL_TRACE_
#include "sel_trace.h"
#endif

#ifndef _H_SYN_VIEW_
#include "syn_view.h"
#endif



/****************************************************************************
*
*  Function Name:                 SelTrace_Create
*
*    This routine creates the Select Trace Dialog.
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
*    N/A
*
*  Side Effects:
*
*    None.
*
******************************************************************************/
void SelTrace_Create
  (
  Widget         parent,
  PstView_t     *pv_p
  )
{
  SelTrace_t    *selt_p;
  XmString       lbl_str;
  XmString       title;
  Widget         formdg, cmpf, cycf, lvlf;

  selt_p = PstView_SelTrace_Get (pv_p);

  /*  Create the dialog form.  */
  title = XmStringCreateLtoR (SLT_TITLE, XmFONTLIST_DEFAULT_TAG);
  formdg = XmCreateFormDialog (parent, "SelTraceFD", NULL, 0);
  XtVaSetValues (formdg,
    XmNdialogStyle, XmDIALOG_MODELESS, 
    XmNlabelFontList, SynAppR_FontList_Get (&GSynAppR),
    XmNbuttonFontList, SynAppR_FontList_Get (&GSynAppR),
    XmNtextFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNresizePolicy, XmRESIZE_GROW,
    XmNresizable, True,
    XmNautoUnmanage, False, 
    XmNdialogTitle, title, 
    NULL);
  XmStringFree (title);
  SelTrace_FormDlg_Put (selt_p, formdg);

  /*  Create the compound form, label, text window and select pushbutton.  */
  cmpf = XtVaCreateWidget ("SelTraceFm", 
    xmFormWidgetClass,  formdg, 
    XmNlabelFontList, SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight, AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth, AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable, True,
    NULL);
  SelTrace_CmpForm_Put (selt_p, cmpf);

  lbl_str = XmStringCreateLtoR (SLT_LABEL_CMP, 
    AppDim_RxnValTag_Get (&GAppDim));
  SelTrace_CmpLbl_Put (selt_p, XtVaCreateWidget ("SelTraceL", 
    xmLabelWidgetClass,  cmpf, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNalignment, XmALIGNMENT_BEGINNING,
    XmNrecomputeSize, False,
    XmNmarginHeight, 0,
    XmNmarginWidth, 0,
      NULL));
  XmStringFree (lbl_str);

  *SelTrace_CmpBuf_Get (selt_p) = '\0';
  SelTrace_CmpText_Put (selt_p, XtVaCreateWidget ("SelTraceT", 
    xmTextWidgetClass, cmpf,
    XmNeditMode, XmSINGLE_LINE_EDIT,
    XmNfontList, SynAppR_FontList_Get (&GSynAppR),
    XmNeditable, True,
    XmNautoShowCursorPosition, False,
    XmNcursorPositionVisible, False,
    XmNmaxLength, SLT_TEXT_BUFLEN - 1,
    XmNcolumns, SLT_TEXT_BUFLEN - 1,
    XmNvalue, SelTrace_CmpBuf_Get (selt_p),
    XmNmarginHeight, AppDim_SepSmall_Get (&GAppDim),
    XmNmarginWidth, AppDim_SepSmall_Get (&GAppDim),
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNshowAsDefault, False,
     NULL));
  XtAddCallback (SelTrace_CmpText_Get (selt_p), XmNactivateCallback, 
    SelTrace_CmpText_CB, (XtPointer) pv_p);

  lbl_str = XmStringCreateLtoR (SLT_LABEL_SELECT, XmFONTLIST_DEFAULT_TAG);
  SelTrace_CmpSelPB_Put (selt_p, XtVaCreateWidget ("SelTracePB", 
    xmPushButtonWidgetClass,  cmpf, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNshowAsDefault, False,
      NULL));
  XmStringFree (lbl_str);
  XtAddCallback (SelTrace_CmpSelPB_Get (selt_p), XmNactivateCallback, 
    SelTrace_CmpSel_CB, (XtPointer) pv_p);

  /*  Create the cycle form, label, text window, select pushbutton 
      and arrow buttons. 
  */
  cycf = XtVaCreateWidget ("SelTraceFm", 
    xmFormWidgetClass,  formdg, 
    XmNlabelFontList, SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight, AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth, AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable, True,
    NULL);
  SelTrace_CycleForm_Put (selt_p, cycf);

  lbl_str = XmStringCreateLtoR (SLT_LABEL_CYCLE, 
    AppDim_RxnValTag_Get (&GAppDim));
  SelTrace_CycleLbl_Put (selt_p, XtVaCreateWidget ("SelTraceL", 
    xmLabelWidgetClass,  cycf, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNalignment, XmALIGNMENT_BEGINNING,
    XmNrecomputeSize, False,
    XmNmarginHeight, 0,
    XmNmarginWidth, 0,
      NULL));
  XmStringFree (lbl_str);

  *SelTrace_CycleBuf_Get (selt_p) = '\0';
  SelTrace_CycleText_Put (selt_p, XtVaCreateWidget ("SelTraceT", 
    xmTextWidgetClass, cycf,
    XmNeditMode, XmSINGLE_LINE_EDIT,
    XmNfontList, SynAppR_FontList_Get (&GSynAppR),
    XmNeditable, True,
    XmNautoShowCursorPosition, True,
    XmNcursorPositionVisible, True,
    XmNmaxLength, SLT_TEXT_BUFLEN - 1,
    XmNcolumns, SLT_TEXT_BUFLEN - 1,
    XmNvalue, SelTrace_CycleBuf_Get (selt_p),
    XmNmarginHeight, AppDim_SepSmall_Get (&GAppDim),
    XmNmarginWidth, AppDim_SepSmall_Get (&GAppDim),
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNshowAsDefault, True,
     NULL));
  XtAddCallback (SelTrace_CycleText_Get (selt_p), XmNactivateCallback, 
    SelTrace_CycleText_CB, (XtPointer) pv_p);

  SelTrace_CycleNextAB_Put (selt_p, XtVaCreateWidget ("SelTraceAB", 
    xmArrowButtonWidgetClass, cycf,
    XmNarrowDirection, XmARROW_RIGHT,
    XmNmultiClick, XmMULTICLICK_DISCARD, 
    XmNshowAsDefault, False,
    XmNwidth, SLT_ARROW_DIM, 
    NULL));
  XtAddCallback (SelTrace_CycleNextAB_Get (selt_p), XmNactivateCallback, 
    SelTrace_CycleNext_CB, (XtPointer) pv_p);

  SelTrace_CyclePrevAB_Put (selt_p, XtVaCreateWidget ("SelTraceAB", 
    xmArrowButtonWidgetClass, cycf,
    XmNarrowDirection, XmARROW_LEFT,
    XmNmultiClick, XmMULTICLICK_DISCARD, 
    XmNshowAsDefault, False,
    XmNwidth, SLT_ARROW_DIM, 
      NULL));
  XtAddCallback (SelTrace_CyclePrevAB_Get (selt_p), XmNactivateCallback, 
    SelTrace_CyclePrev_CB, (XtPointer) pv_p);

  /*  Create the level form and labels.  */
  lvlf = XtVaCreateWidget ("SelTraceFm", 
    xmFormWidgetClass,  formdg, 
    XmNlabelFontList, SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight, AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth, AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable, False,
    NULL);
  SelTrace_LvlForm_Put (selt_p, lvlf);

  lbl_str = XmStringCreateLtoR (SLT_LABEL_LVL, 
    AppDim_RxnValTag_Get (&GAppDim));
  SelTrace_LvlLblL_Put (selt_p, XtVaCreateWidget ("SelTraceL", 
    xmLabelWidgetClass,  lvlf, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNalignment, XmALIGNMENT_BEGINNING,
    XmNrecomputeSize, False,
    XmNmarginHeight, AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth, AppDim_MargFmFr_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLtoR (SLT_LABEL_NUM, 
    AppDim_PstNSGTag_Get (&GAppDim));
  SelTrace_LvlNumL_Put (selt_p, XtVaCreateWidget ("SelTraceL", 
    xmLabelWidgetClass,  lvlf, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNalignment, XmALIGNMENT_CENTER,
    XmNrecomputeSize, False,
    XmNmarginHeight, AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth, AppDim_MargFmFr_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  /*  Create the clear pushbutton.  */
  lbl_str = XmStringCreateLtoR (SLT_PUSHB_CLEAR, XmFONTLIST_DEFAULT_TAG);
  SelTrace_ClearPB_Put (selt_p, XtVaCreateWidget ("SelTraceCPB", 
    xmPushButtonWidgetClass,  formdg, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNshowAsDefault, False,
      NULL));
  XmStringFree (lbl_str);
  XtAddCallback (SelTrace_ClearPB_Get (selt_p), XmNactivateCallback, 
    SelTrace_Clear_CB, (XtPointer) pv_p);

  /*  Create the dismiss pushbutton.  */
  lbl_str = XmStringCreateLtoR (SLT_PUSHB_DISMISS, XmFONTLIST_DEFAULT_TAG);
  SelTrace_DismissPB_Put (selt_p, XtVaCreateWidget ("SelTraceDPB", 
    xmPushButtonWidgetClass,  formdg, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNshowAsDefault, False,
      NULL));
  XmStringFree (lbl_str);
  XtAddCallback (SelTrace_DismissPB_Get (selt_p), XmNactivateCallback, 
    SelTrace_Dismiss_CB, (XtPointer) selt_p);

  /*  Create the number all pushbutton.  */
  lbl_str = XmStringCreateLtoR (SLT_PUSHB_NUMALL, XmFONTLIST_DEFAULT_TAG);
  SelTrace_NumAllPB_Put (selt_p, XtVaCreateWidget ("SelTraceNPB", 
    xmPushButtonWidgetClass,  formdg, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNshowAsDefault, False,
      NULL));
  XmStringFree (lbl_str);
  XtAddCallback (SelTrace_NumAllPB_Get (selt_p), XmNactivateCallback, 
    SelTrace_NumAll_CB, (XtPointer) pv_p);

  /*  Set the proper attachments.  */

  XtVaSetValues (SelTrace_CmpLbl_Get (selt_p),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNrightWidget, SelTrace_CmpText_Get (selt_p),
    XmNrightAttachment, XmATTACH_WIDGET,
    NULL);

  XtVaSetValues (SelTrace_CmpText_Get (selt_p),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_NONE,
    XmNrightOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNrightWidget, SelTrace_CmpSelPB_Get (selt_p),
    XmNrightAttachment, XmATTACH_WIDGET,
    NULL);

  XtVaSetValues (SelTrace_CmpSelPB_Get (selt_p),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_NONE,
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  XtVaSetValues (SelTrace_CycleLbl_Get (selt_p),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNrightWidget, SelTrace_CycleText_Get (selt_p),
    XmNrightAttachment, XmATTACH_WIDGET,
    NULL);

  XtVaSetValues (SelTrace_CycleText_Get (selt_p),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_NONE,
    XmNrightOffset, 2 * SLT_ARROW_DIM + 3 * AppDim_SepSmall_Get (&GAppDim),
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  XtVaSetValues (SelTrace_CyclePrevAB_Get (selt_p),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_NONE,
    XmNrightOffset, SLT_ARROW_DIM + 2 * AppDim_SepSmall_Get (&GAppDim),
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  XtVaSetValues (SelTrace_CycleNextAB_Get (selt_p),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_NONE,
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  XtVaSetValues (SelTrace_LvlLblL_Get (selt_p),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNrightWidget, SelTrace_LvlNumL_Get (selt_p),
    XmNrightAttachment, XmATTACH_WIDGET,
    NULL);

  XtVaSetValues (SelTrace_LvlNumL_Get (selt_p),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_NONE,
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  XtVaSetValues (SelTrace_LvlForm_Get (selt_p),
    XmNtopOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  XtVaSetValues (SelTrace_CmpForm_Get (selt_p),
    XmNtopOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNtopWidget, SelTrace_LvlForm_Get (selt_p),
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  XtVaSetValues (SelTrace_CycleForm_Get (selt_p),
    XmNtopOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNtopWidget, SelTrace_CmpForm_Get (selt_p),
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  XtVaSetValues (SelTrace_ClearPB_Get (selt_p),
    XmNtopOffset, AppDim_SepLarge_Get (&GAppDim),
    XmNtopWidget, SelTrace_CycleForm_Get (selt_p),
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNbottomOffset, AppDim_SepLarge_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftPosition, 10,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 30,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  XtVaSetValues (SelTrace_DismissPB_Get (selt_p),
    XmNtopOffset, 0,
    XmNtopWidget, SelTrace_ClearPB_Get (selt_p),
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomOffset, 0,
    XmNbottomWidget, SelTrace_ClearPB_Get (selt_p),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNleftPosition, 40,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 60,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  XtVaSetValues (SelTrace_NumAllPB_Get (selt_p),
    XmNtopOffset, 0,
    XmNtopWidget, SelTrace_ClearPB_Get (selt_p),
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomOffset, 0,
    XmNbottomWidget, SelTrace_ClearPB_Get (selt_p),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNleftPosition, 70,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 90,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  XtManageChild (SelTrace_CmpLbl_Get (selt_p));
  XtManageChild (SelTrace_CmpText_Get (selt_p));
  XtManageChild (SelTrace_CmpSelPB_Get (selt_p));
  XtManageChild (SelTrace_CmpForm_Get (selt_p));
  XtManageChild (SelTrace_CycleLbl_Get (selt_p));
  XtManageChild (SelTrace_CyclePrevAB_Get (selt_p));
  XtManageChild (SelTrace_CycleText_Get (selt_p));
  XtManageChild (SelTrace_CycleNextAB_Get (selt_p));
  XtManageChild (SelTrace_CycleForm_Get (selt_p));
  XtManageChild (SelTrace_LvlLblL_Get (selt_p));
  XtManageChild (SelTrace_LvlNumL_Get (selt_p));
  XtManageChild (SelTrace_LvlForm_Get (selt_p));
  XtManageChild (SelTrace_ClearPB_Get (selt_p));
  XtManageChild (SelTrace_DismissPB_Get (selt_p));
  XtManageChild (SelTrace_NumAllPB_Get (selt_p));
  SelTrace_IsCreated_Put (selt_p, TRUE);

  return ;
}
/*  End of SelTrace_Create  */


/****************************************************************************
*
*  Function Name:                 SelTrace_Init
*
*    This routine initializes the Select Trace data structure fields not
*    associate with the widgets.   
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
*    N/A
*
*  Side Effects:
*
*    Frees memory.
*
******************************************************************************/
void SelTrace_Init
  (
  SelTrace_t   *selt_p
  )
{

  SelTrace_CurrCmp_Put (selt_p, NULL);
  SelTrace_CurrCycle_Put (selt_p, SLT_CYCLE_NONE);
  SelTrace_NumCycles_Put (selt_p, 0);
  SelTrace_SelCmps_Put (selt_p, NULL);

  return ;
}
/*  End of SelTrace_Init  */


/****************************************************************************
*
*  Function Name:                 SelTrace_Reset
*
*    This routine resets the Select Trace by freeing the memory for the
*    array of compound pointers and reinitializing variables.
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
*    N/A
*
*  Side Effects:
*
*    Frees memory.
*
******************************************************************************/
void SelTrace_Reset
  (
  SelTrace_t   *selt_p
  )
{
  if (SelTrace_SelCmps_Get (selt_p) != NULL)
    free (SelTrace_SelCmps_Get (selt_p));

  SelTrace_Init (selt_p);

  return ;
}
/*  End of SelTrace_Reset  */


/****************************************************************************
*
*  Function Name:                 SelTrace_Cycles_Set
*
*    This routine creates and initializes the array of selected compounds
*    for each cycle.
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
*    N/A
*
*  Side Effects:
*
*    Allocates memory.
*
******************************************************************************/
Boolean_t SelTrace_Cycles_Set
  (
  PstView_t     *pv_p
  )
{
  SelTrace_t    *selt_p;
  PstCB_t       *pstcb_p;
  Compound_t   **selcmps;
  Compound_t    *cmp_p;
  U32_t          cycle_i, num_cycles;

  selt_p = PstView_SelTrace_Get (pv_p);
  pstcb_p = PstView_PstCB_Get (pv_p);
  if (pstcb_p == NULL)
    {
    InfoWarn_Show (SLT_WARN_NOPST);
    return (FALSE);
    }

  /*  Get the array of selected compound nodes.  */
  num_cycles = PstCB_TotalExpandedCompounds_Get (pstcb_p);
  if (num_cycles == 0)
    {
    InfoWarn_Show (SLT_WARN_NOCYCLES);
    return (FALSE);
    }

  selcmps = (Compound_t **) malloc (num_cycles * sizeof (Compound_t *));
  if (selcmps == NULL)
    {
    fprintf (stderr, 
      "\nSelTrace_Reset:  unable to allocate memory for sel comp array.\n");
    exit (-1);
    }

  SelTrace_SelCmps_Put (selt_p, selcmps);
  SelTrace_NumCycles_Put (selt_p, num_cycles);
  cmp_p = PstSubg_Son_Get (PstCB_Root_Get (pstcb_p));
  for (cycle_i = 0; cycle_i < num_cycles && cmp_p != NULL; cycle_i++)
    {
    selcmps[cycle_i] = cmp_p;
    cmp_p = PstComp_Next_Get (cmp_p);
    }

  if (cycle_i != num_cycles)
    {
    fprintf (stderr, 
      "\nSelTrace_Cycles_Set:  didn't get all of the selected compounds.\n");
    SelTrace_NumCycles_Put (selt_p, cycle_i);
    }

  SelTrace_CurrCycle_Put (selt_p, 0);

  return (TRUE);
}
/*  End of SelTrace_Cycles_Set  */


/****************************************************************************
*
*  Function Name:                 SelTrace_Setup
*
*    This routine sets up Select Trace:  it stores the trace of
*    selected compounds for the given PST, and displays sets the
*    current selected trace cycle in the Select Trace viewer.
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
*    N/A
*
*  Side Effects:
*
*    None.
*
******************************************************************************/
Boolean_t SelTrace_Setup
  (
  PstView_t     *pv_p
  )
{
  PsvTreeLvl_t  *pthtlvl_p;             /* Path tree level */
  SelTrace_t    *selt_p;
  PstCB_t       *pstcb_p;
  Compound_t    *curcmp_p;
  XmString       num_lbl;

  pstcb_p = PstView_PstCB_Get (pv_p);
  selt_p = PstView_SelTrace_Get (pv_p);

  if (!SelTrace_Cycles_Set (pv_p))
    return (FALSE);

  PstView_Mouse_Remove (pv_p);

  /*  Display the tree with the root as the current selected trace 
      compound.  This involves a bit of a hack:  destroy the current
      top tree level and replace it with a dummy treelevel.
  */
  SelTrace_CurrCycle_Put (selt_p, 0);
  sprintf (SelTrace_CycleBuf_Get (selt_p), "%lu", 
    SelTrace_CurrCycle_Get (selt_p) + 1);
  XmTextSetString (SelTrace_CycleText_Get (selt_p), 
    SelTrace_CycleBuf_Get (selt_p));

  curcmp_p = PstSubg_Son_Get (PstCB_Root_Get (pstcb_p));
  SelTrace_CurrCmp_Put (selt_p, curcmp_p);
  sprintf (SelTrace_CmpBuf_Get (selt_p), "%lu", 
    SymTab_Index_Get (PstComp_SymbolTable_Get (curcmp_p)));
  XmTextSetString (SelTrace_CmpText_Get (selt_p), 
    SelTrace_CmpBuf_Get (selt_p));
  PstView_Trace_Store (PstView_CmpInfo_Get (pv_p), curcmp_p, 1, TRUE);

  num_lbl = XmStringCreateLtoR (" 0", AppDim_PstNSGTag_Get (&GAppDim));
  XtVaSetValues (SelTrace_LvlNumL_Get (selt_p),
    XmNlabelString, num_lbl, 
    NULL);
  XmStringFree (num_lbl);

  if (SelTrace_NumCycles_Get (selt_p) > 1)
    XtManageChild (SelTrace_CycleNextAB_Get (selt_p));
  else
    XtUnmanageChild (SelTrace_CycleNextAB_Get (selt_p));

  if (XtIsManaged (SelTrace_CyclePrevAB_Get (selt_p)))
    XtUnmanageChild (SelTrace_CyclePrevAB_Get (selt_p));

  ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP), 
    PstView_IthPTLvl_Get (pv_p, 0));
  PstView_PathLen_Put (pv_p, 1);
  PstVLvls_SetTwo (pv_p);
  pthtlvl_p = ViewLvl_TreeLvl_Get (PstView_PathVLvl_Get (pv_p));
  TreeLvl_NCmps_Put (pthtlvl_p, 0);
  TreeLvl_NSGs_Put (pthtlvl_p, 0);
  TreeLvl_SelNd_Put (pthtlvl_p, PVW_NODE_NONE);
  TreeLvl_SelSG_Put (pthtlvl_p, 0);
  TreeLvl_FocusNd_Put (pthtlvl_p, PVW_NODE_NONE);
  TreeLvl_FocusSG_Put (pthtlvl_p, 0);
  PstView_Display (pv_p);

  return (TRUE);
}
/*  End of SelTrace_Setup  */



/****************************************************************************
*
*  Function Name:                 SelTrace_Update
*
*      
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
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SelTrace_Update
  (
  PstView_t       *pv_p, 
  U32_t            cycle
  )
{
  SelTrace_t      *selt_p;
  Compound_t      *selcmp_p;
  PsvTreeLvl_t    *tlvl_p;                  /* Pst tree level */
  PsvTreeLvl_t    *pthtlvl_p;               /* Path tree level */
  XmString         num_lbl;
  U16_t            cmp_i;
  U16_t            node_i;
  U16_t            tlvl_num;
  Boolean_t        found;
  char             num_buff[16];

  /*  Store the current cycle number, set up the previous and next buttons.
  */
  PstView_Mouse_Remove (pv_p);
  selt_p = PstView_SelTrace_Get (pv_p);
  SelTrace_CurrCycle_Put (selt_p, cycle);
  sprintf (SelTrace_CycleBuf_Get (selt_p), "%lu", 
    SelTrace_CurrCycle_Get (selt_p) + 1);
  XmTextSetString (SelTrace_CycleText_Get (selt_p), 
    SelTrace_CycleBuf_Get (selt_p));

  if (cycle > 0)
    XtManageChild (SelTrace_CyclePrevAB_Get (selt_p));
  else
    XtUnmanageChild (SelTrace_CyclePrevAB_Get (selt_p));

  if (cycle == SelTrace_NumCycles_Get (selt_p) - 1)
    XtUnmanageChild (SelTrace_CycleNextAB_Get (selt_p));
  else
    XtManageChild (SelTrace_CycleNextAB_Get (selt_p));

  selcmp_p = SelTrace_IthSelCmp_Get (selt_p, cycle);
  SelTrace_CurrCmp_Put (selt_p, selcmp_p);
  sprintf (SelTrace_CmpBuf_Get (selt_p), "%1lu", 
    SymTab_Index_Get (PstComp_SymbolTable_Get (selcmp_p)));
  XmTextSetString (SelTrace_CmpText_Get (selt_p), 
      SelTrace_CmpBuf_Get (selt_p));
  PstView_Trace_Store (PstView_CmpInfo_Get (pv_p), selcmp_p, 
    cycle + 1, TRUE);

  /*  Check to see if the selected compound is somewhere along the 
      current path.  If it is, then move the appropriate level to the
      top view level, set up the bottom two view levels and call the 
      pst display routine.  Otherwise, we need to back up the tree
      levels along the path from the given compound to the root, 
      and then set up the last two view levels and then call the pst 
      display routine.
  */
  found = FALSE;
  for (tlvl_num = 0; tlvl_num < PstView_PathLen_Get (pv_p) && !found; 
      tlvl_num++)
    {
    tlvl_p = PstView_IthPTLvl_Get (pv_p, tlvl_num);
    for (node_i = 0; node_i < TreeLvl_NCmps_Get (tlvl_p) && !found; node_i++)
      found = selcmp_p == TreeLvl_IthCmpNd_Get (tlvl_p, node_i);
    }

  if (found)
    {
    /*  Update level and compound number labels.  */
    sprintf (num_buff, "%1u", tlvl_num - 1);
    num_lbl = XmStringCreateLtoR (num_buff, AppDim_PstNSGTag_Get (&GAppDim));
      XtVaSetValues (SelTrace_LvlNumL_Get (selt_p),
      XmNlabelString, num_lbl, 
      NULL);
    XmStringFree (num_lbl);

    PstView_PathLen_Put (pv_p, tlvl_num);
    TreeLvl_SelNd_Put (tlvl_p, node_i - 1);
    TreeLvl_SelSG_Put (tlvl_p, 
        PstSubg_Index_Get (PstComp_Father_Get (selcmp_p)));
    TreeLvl_FocusNd_Put (tlvl_p, TreeLvl_SelNd_Get (tlvl_p));
    TreeLvl_FocusSG_Put (tlvl_p, TreeLvl_SelSG_Get (tlvl_p));
    ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP), tlvl_p);
    PstView_Visit_Store (PstView_CmpInfo_Get (pv_p), selcmp_p);

    /*  Update the path by moving left past the compounds with 
        a higher level.  
    */
    pthtlvl_p = ViewLvl_TreeLvl_Get (PstView_PathVLvl_Get (pv_p));
    tlvl_num--;
    if (tlvl_num < TreeLvl_NSGs_Get (pthtlvl_p))
      {
      /*  Move left past the compounds with a higher level.  */ 
      cmp_i = TreeLvl_NCmps_Get (pthtlvl_p) - 1;
      while (cmp_i > 0 && PstView_IthPthLN_Get (pv_p, cmp_i) >= tlvl_num)
      cmp_i--;

      if (PstView_IthPthLN_Get (pv_p, cmp_i) >= tlvl_num)
        TreeLvl_NCmps_Put (pthtlvl_p, cmp_i);
      else
        TreeLvl_NCmps_Put (pthtlvl_p, cmp_i + 1);

      TreeLvl_NSGs_Put (pthtlvl_p, tlvl_num);
      }
  
    else if (TreeLvl_NSGs_Get (pthtlvl_p) < tlvl_num)
      {
      DrawCxt_t       *pthdc_p;               
      Compound_t      *cmp_p;                   /* Selected compound */
      U16_t            tlvl_i;
      U16_t            num_bros;

      /*  Add subgoals to path.  */
      pthdc_p = ViewLvl_DrawCxt_Get (PstView_PathVLvl_Get (pv_p));
      tlvl_i = TreeLvl_NSGs_Get (pthtlvl_p);
      tlvl_p = PstView_IthPTLvl_Get (pv_p, tlvl_i);
      selcmp_p = TreeLvl_IthCmpNd_Get (tlvl_p, TreeLvl_SelNd_Get (tlvl_p));
      num_bros = PstSubg_NumSons_Get (PstComp_Father_Get (selcmp_p));
      cmp_i = TreeLvl_NCmps_Get (pthtlvl_p);
      while (tlvl_i < tlvl_num && num_bros + cmp_i 
          < DrawCxt_NumVsbNd_Get (pthdc_p))
        {
        cmp_p = PstSubg_Son_Get (PstComp_Father_Get (selcmp_p));
        while (cmp_p != NULL)
          {
          TreeLvl_IthCmpNd_Put (pthtlvl_p, cmp_i, cmp_p);
          PstView_IthPthLN_Put (pv_p, cmp_i, tlvl_i);
          cmp_p = PstComp_Brother_Get (cmp_p);
          cmp_i++;
          }

        tlvl_i++;
        tlvl_p = PstView_IthPTLvl_Get (pv_p, tlvl_i);
        selcmp_p = TreeLvl_IthCmpNd_Get (tlvl_p, TreeLvl_SelNd_Get (tlvl_p));
        num_bros = PstSubg_NumSons_Get (PstComp_Father_Get (selcmp_p));
        }

      TreeLvl_NSGs_Put (pthtlvl_p, tlvl_i);
      TreeLvl_NCmps_Put (pthtlvl_p, cmp_i);
      }
    }  /*  End of if found */

  else
    {
    /*  Otherwise (the compound is not along the current path), we have to 
        first load in the tree levels along the path from the root to the 
        selected compound, and then set up the path level, and call
        the pst display routine with the path view level and proper node 
        number.
    */
    tlvl_num = TreeLvls_Backup (pv_p, selcmp_p); 

    /*  Update level and compound number labels.  */
    sprintf (num_buff, "%1u", tlvl_num);
    num_lbl = XmStringCreateLtoR (num_buff, AppDim_PstNSGTag_Get (&GAppDim));
      XtVaSetValues (SelTrace_LvlNumL_Get (selt_p),
      XmNlabelString, num_lbl, 
      NULL);
    XmStringFree (num_lbl);

    PstView_PathLen_Put (pv_p, tlvl_num + 1);
    }

  PstView_ActVLvl_Put (pv_p, PVW_LEVEL_TOP);
  PstVLvls_SetTwo (pv_p);

  if (TreeLvl_NCmps_Get (ViewLvl_TreeLvl_Get (PstView_IthVLvl_Get (
      pv_p, PVW_LEVEL_MID))) == 0)
    {
    PsvTreeLvl_t  *switchtl_p;

    /*  Move up one level.  */
    PstView_ActVLvl_Put (pv_p, PVW_LEVEL_MID);
    tlvl_p = ViewLvl_TreeLvl_Get (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP));

    switchtl_p = PstView_IthPTLvl_Get (pv_p, TreeLvl_LvlNum_Get (tlvl_p) - 1);
    TreeLvl_FocusNd_Put (switchtl_p, TreeLvl_SelNd_Get (switchtl_p));
    TreeLvl_FocusSG_Put (switchtl_p, TreeLvl_SelSG_Get (switchtl_p));
    ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP), 
      switchtl_p);
    switchtl_p = PstView_IthPTLvl_Get (pv_p, TreeLvl_LvlNum_Get (tlvl_p));
    TreeLvl_FocusNd_Put (switchtl_p, TreeLvl_SelNd_Get (switchtl_p));
    TreeLvl_FocusSG_Put (switchtl_p, TreeLvl_SelSG_Get (switchtl_p));
    ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID),
      switchtl_p);
    switchtl_p = PstView_IthPTLvl_Get (pv_p, TreeLvl_LvlNum_Get (tlvl_p) + 1);
    ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM),
      switchtl_p);
    TreeLvl_SelNd_Put (switchtl_p, PVW_NODE_NONE);
    TreeLvl_SelSG_Put (switchtl_p, 0);
    if (TreeLvl_NCmps_Get (switchtl_p) > 0)
      {
      PstView_PathLen_Put (pv_p, PstView_PathLen_Get (pv_p) - 1);
      TreeLvl_FocusNd_Put (switchtl_p, 0);
      TreeLvl_FocusSG_Put (switchtl_p, PstSubg_Index_Get (
        PstComp_Father_Get (TreeLvl_IthCmpNd_Get (switchtl_p, 0))));
      }
    else
      {
      TreeLvl_FocusNd_Put (switchtl_p, PVW_NODE_NONE);
      TreeLvl_FocusSG_Put (switchtl_p, 0);
      }

    pthtlvl_p = ViewLvl_TreeLvl_Get (PstView_PathVLvl_Get (pv_p));
    tlvl_num = TreeLvl_LvlNum_Get (ViewLvl_TreeLvl_Get (PstView_IthVLvl_Get (
      pv_p, PVW_LEVEL_TOP)));
    if (tlvl_num < TreeLvl_NSGs_Get (pthtlvl_p))
      {
      /*  Move left past the compounds with a higher level.  */ 
      cmp_i = TreeLvl_NCmps_Get (pthtlvl_p) - 1;
      while (cmp_i > 0 && PstView_IthPthLN_Get (pv_p, cmp_i) >= tlvl_num)
        cmp_i--;

      if (PstView_IthPthLN_Get (pv_p, cmp_i) >= tlvl_num)
        TreeLvl_NCmps_Put (pthtlvl_p, cmp_i);
      else
        TreeLvl_NCmps_Put (pthtlvl_p, cmp_i + 1);

      TreeLvl_NSGs_Put (pthtlvl_p, tlvl_num);
      }
    }  /* End of else not on path */

    pthtlvl_p = ViewLvl_TreeLvl_Get (PstView_PathVLvl_Get (pv_p));
    if (TreeLvl_NCmps_Get (pthtlvl_p) > 0)
      {
      TreeLvl_FocusNd_Put (pthtlvl_p, TreeLvl_NCmps_Get (pthtlvl_p) - 1);
      TreeLvl_FocusSG_Put (pthtlvl_p, PstSubg_Index_Get (
        PstComp_Father_Get (TreeLvl_IthCmpNd_Get (pthtlvl_p, 
        TreeLvl_NCmps_Get (pthtlvl_p) - 1))));
      }
    else
      {
      TreeLvl_FocusNd_Put (pthtlvl_p, PVW_NODE_NONE);
      TreeLvl_FocusSG_Put (pthtlvl_p, 0);
      }

  PstView_Display (pv_p);
  return ;
}
/*  End of SelTrace_Update  */


/****************************************************************************
*
*  Function Name:                 SelTrace_Clear_CB
*
*    This routine clears all of the select numbers.
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
*    N/A
*
*  Side Effects:
*
*    None.
*
******************************************************************************/
void SelTrace_Clear_CB
  (
  Widget         w,
  XtPointer      client_p, 
  XtPointer      call_p
  )
{
  PstView_t     *pv_p;
  SelTrace_t    *selt_p;
  Compound_t    *curcmp_p;
  PsvCmpInfo_t  *cmp_infos;
  U32_t          cmp_i;

  pv_p = (PstView_t *) client_p;
  if (pv_p == NULL)
    return;

  /*  Unset all compounds in tree.  */
  PstView_Mouse_Remove (pv_p);
  selt_p = PstView_SelTrace_Get (pv_p);
  cmp_infos = PstView_CmpInfo_Get (pv_p);
  for (cmp_i = 0; cmp_i < PstView_NumCmpI_Get (pv_p); cmp_i++)
    {
    PsvCmpI_Traced_Unset (cmp_infos[cmp_i]);
    PsvCmpI_TracElse_Unset (cmp_infos[cmp_i]);
    }

  SelTrace_CurrCycle_Put (selt_p, 0);
  sprintf (SelTrace_CycleBuf_Get (selt_p), "%lu", 
    SelTrace_CurrCycle_Get (selt_p) + 1);
  XmTextSetString (SelTrace_CycleText_Get (selt_p), 
    SelTrace_CycleBuf_Get (selt_p));

  curcmp_p = PstSubg_Son_Get (PstCB_Root_Get (PstView_PstCB_Get (pv_p)));
  SelTrace_CurrCmp_Put (selt_p, curcmp_p);
  sprintf (SelTrace_CmpBuf_Get (selt_p), "%lu", 
    SymTab_Index_Get (PstComp_SymbolTable_Get (curcmp_p)));
  XmTextSetString (SelTrace_CmpText_Get (selt_p), 
    SelTrace_CmpBuf_Get (selt_p));
  PstView_Trace_Store (PstView_CmpInfo_Get (pv_p), curcmp_p, 1, TRUE);

  if (SelTrace_NumCycles_Get (selt_p) > 1)
    XtManageChild (SelTrace_CycleNextAB_Get (selt_p));
  else
    XtUnmanageChild (SelTrace_CycleNextAB_Get (selt_p));

  if (XtIsManaged (SelTrace_CyclePrevAB_Get (selt_p)))
    XtUnmanageChild (SelTrace_CyclePrevAB_Get (selt_p));

  PstView_Display (pv_p);
  return;
}
/*  End of SelTrace_Clear_CB  */


/****************************************************************************
*
*  Function Name:                 SelTrace_CmpSel_CB
*
*    This routine allows the user to specify the compound by selecting
*    a node in the pst view area.  If the compound has been selected for
*    expansion in some cycle, then the pst view and select trace are 
*    updated for that cycle.
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
*    N/A
*
*  Side Effects:
*
*    None.
*
******************************************************************************/
void SelTrace_CmpSel_CB
  (
  Widget         w,
  XtPointer      client_p, 
  XtPointer      call_p
  )
{
  PstView_t     *pv_p;
  SelTrace_t    *selt_p;
  U32_t          comp_ind;
  U32_t          cycle;

  pv_p = (PstView_t *) client_p;
  if (pv_p == NULL)
    return;

  /*  Get the selected compound.  */
  PstView_Compound_Sel (pv_p);
  if (PstView_LastSelCmp_Get (pv_p) == NULL)
    {
    InfoWarn_Show (SLT_WARN_BADSEL);
    return ;
    }

  selt_p = PstView_SelTrace_Get (pv_p);
  comp_ind = SymTab_Index_Get (PstComp_SymbolTable_Get (
    PstView_LastSelCmp_Get (pv_p)));
  cycle = 0;
  while (cycle < SelTrace_NumCycles_Get (selt_p) 
      && comp_ind != SymTab_Index_Get (PstComp_SymbolTable_Get (
      SelTrace_IthSelCmp_Get (selt_p, cycle))))
    cycle++;

  if (cycle >= SelTrace_NumCycles_Get (selt_p))
    {
    InfoWarn_Show (SLT_WARN_NOTSEL);
    return ;
    }

  if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_SOLV)
    {
    Subgoal_t     *sg_p;
    Boolean_t      path_allsolved;

    /*  We have to check to make sure the subgoals along the path
        from the the selected compound to the root are ``all solved''.
        This ain't a pretty looping structure, but it works.
    */
    sg_p = PstComp_Father_Get (SelTrace_IthSelCmp_Get (selt_p, cycle));
    path_allsolved = TRUE;
    while (sg_p != NULL && path_allsolved) 
      {
      path_allsolved = path_allsolved && Pst_All_Solved (sg_p);
      if (PstSubg_Father_Get (sg_p) != NULL)
        sg_p = PstComp_Father_Get (PstSubg_Father_Get (sg_p));
      else
        sg_p = NULL;
      }

    if (!path_allsolved)
      {
      InfoWarn_Show (SLT_WARN_NOTSOLVED);
      return ;
      }
    }

  SelTrace_Update (pv_p, cycle);
  return;
}
/*  End of SelTrace_CmpSel_CB  */


/****************************************************************************
*
*  Function Name:                 SelTrace_CmpText_CB
*
*    This routine updates the SelTrace data structure and the PST viewer
*    with the selected compound for the compound index in the text window.
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
*    N/A
*
*  Side Effects:
*
*    None.
*
******************************************************************************/
void SelTrace_CmpText_CB
  (
  Widget         w,
  XtPointer      client_p, 
  XtPointer      call_p
  )
{
  PstView_t     *pv_p;
  SelTrace_t    *selt_p;
  char          *str_p;                      /* returned number string */
  U32_t          comp_ind;
  U32_t          cycle;

  pv_p = (PstView_t *) client_p;
  if (pv_p == NULL)
    return;

  /*  Get text string and convert it into the compound index.  Check to see
      if the compound index is among the those in the list of selected 
      compounds.
  */
  str_p = XmTextGetString (w);
  if (str_p == NULL)
    {
    InfoWarn_Show (SLT_ERROR_NOSTRING);
    return;
    }
  if (sscanf (str_p, "%lu", &comp_ind) != 1)
    {
    InfoWarn_Show (SLT_WARN_CONVERT);
    XtFree (str_p);
    return;
    }

  XtFree (str_p);

  selt_p = PstView_SelTrace_Get (pv_p);
  cycle = 0;
  while (cycle < SelTrace_NumCycles_Get (selt_p) 
      && comp_ind != SymTab_Index_Get (PstComp_SymbolTable_Get (
      SelTrace_IthSelCmp_Get (selt_p, cycle))))
    cycle++;

  if (cycle >= SelTrace_NumCycles_Get (selt_p))
    {
    InfoWarn_Show (SLT_WARN_NOTSEL);
    return;
    }

  if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_SOLV)
    {
    Subgoal_t     *sg_p;
    Boolean_t      path_allsolved;

    /*  We have to check to make sure the subgoals along the path
        from the the selected compound to the root are ``all solved''.
        This ain't a pretty looping structure, but it works.
    */
    sg_p = PstComp_Father_Get (SelTrace_IthSelCmp_Get (selt_p, cycle));
    path_allsolved = TRUE;
    while (sg_p != NULL && path_allsolved) 
      {
      path_allsolved = path_allsolved && Pst_All_Solved (sg_p);
      if (PstSubg_Father_Get (sg_p) != NULL)
        sg_p = PstComp_Father_Get (PstSubg_Father_Get (sg_p));
      else
        sg_p = NULL;
      }

    if (!path_allsolved)
      {
      InfoWarn_Show (SLT_WARN_NOTSOLVED);
      return ;
      }
    }

  SelTrace_Update (pv_p, cycle);
  return;
}
/*  End of SelTrace_CmpText_CB  */


/****************************************************************************
*
*  Function Name:                 SelTrace_CycleNext_CB
*
*    This routine updates the SelTrace data structure and the PST viewer
*    with the next selected compound.  If the current tree type is solved,
*    then find the next selected compounds that has been solved.
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
*    N/A
*
*  Side Effects:
*
*    None.
*
******************************************************************************/
void SelTrace_CycleNext_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p
  )
{
  PstView_t     *pv_p;

  pv_p = (PstView_t *) client_p;
  if (pv_p == NULL)
    return;

  if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_SOLV)
    {
    SelTrace_t    *selt_p;
    Subgoal_t     *sg_p;
    U32_t          cycle;
    Boolean_t      path_allsolved;

    /*  We have to check to make sure the subgoals along the path
        from the the selected compound to the root are ``all solved''.
        This ain't a pretty looping structure, but it works.
    */
    selt_p = PstView_SelTrace_Get (pv_p);
    cycle = SelTrace_CurrCycle_Get (selt_p) + 1;
    path_allsolved = FALSE;
    while (cycle < SelTrace_NumCycles_Get (selt_p) && !path_allsolved)
      {
      sg_p = PstComp_Father_Get (SelTrace_IthSelCmp_Get (selt_p, cycle));
      path_allsolved = TRUE;
      while (sg_p != NULL && path_allsolved) 
        {
        path_allsolved = path_allsolved && Pst_All_Solved (sg_p);
        if (PstSubg_Father_Get (sg_p) != NULL)
          sg_p = PstComp_Father_Get (PstSubg_Father_Get (sg_p));
        else
          sg_p = NULL;
        }

      if (!path_allsolved)
        cycle++;
      }

    if (cycle < SelTrace_NumCycles_Get (selt_p))
          SelTrace_Update (pv_p, cycle);
    else
      {
      InfoWarn_Show (SLT_WARN_NOMORESOL);
      return ;
      }
    }

  else
    SelTrace_Update (pv_p, 
      SelTrace_CurrCycle_Get (PstView_SelTrace_Get (pv_p)) + 1);

  return ;
}
/*  End of SelTrace_CycleNext_CB  */


/****************************************************************************
*
*  Function Name:                 SelTrace_CyclePrev_CB
*
*    This routine updates the SelTrace data structure and the PST viewer
*    with the previous selected compound.
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
*    N/A
*
*  Side Effects:
*
*    None.
*
******************************************************************************/
void SelTrace_CyclePrev_CB
  (
  Widget         w, 
  XtPointer      client_p, 
  XtPointer      call_p
  )
{
  PstView_t     *pv_p;

  pv_p = (PstView_t *) client_p;
  if (pv_p == NULL)
    return;

  if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_SOLV)
    {
    SelTrace_t    *selt_p;
    Subgoal_t     *sg_p;
    U32_t          cycle;
    Boolean_t      path_allsolved;

    /*  We have to check to make sure the subgoals along the path
        from the the selected compound to the root are ``all solved''.
        This ain't a pretty looping structure, but it works.
    */
    selt_p = PstView_SelTrace_Get (pv_p);
    cycle = SelTrace_CurrCycle_Get (selt_p);
    path_allsolved = FALSE;
    while (cycle > 0 && !path_allsolved)
      {
      sg_p = PstComp_Father_Get (SelTrace_IthSelCmp_Get (selt_p, cycle - 1));
      path_allsolved = TRUE;
      while (sg_p != NULL && path_allsolved) 
        {
        path_allsolved = path_allsolved && Pst_All_Solved (sg_p);
        if (PstSubg_Father_Get (sg_p) != NULL)
          sg_p = PstComp_Father_Get (PstSubg_Father_Get (sg_p));
        else
          sg_p = NULL;
        }

      if (!path_allsolved)
        cycle--;
      }

    if (cycle > 0)
          SelTrace_Update (pv_p, cycle - 1);
    else
      {
      InfoWarn_Show (SLT_WARN_NOMORESOL);
      return ;
      }
    }

  else
    SelTrace_Update (pv_p, 
      SelTrace_CurrCycle_Get (PstView_SelTrace_Get (pv_p)) - 1);

  return ;
}
/*  End of SelTrace_CyclePrev_CB  */


/****************************************************************************
*
*  Function Name:                 SelTrace_CycleText_CB
*
*    This routine updates the SelTrace data structure and the PST viewer
*    with the selected compound for the cycle entered in the text window.
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
*    N/A
*
*  Side Effects:
*
*    None.
*
******************************************************************************/
void SelTrace_CycleText_CB
  (
  Widget         w,
  XtPointer      client_p, 
  XtPointer      call_p
  )
{
  PstView_t     *pv_p;
  SelTrace_t    *selt_p;
  char          *str_p;                      /* returned number string */
  U32_t          cycle;

  pv_p = (PstView_t *) client_p;
  if (pv_p == NULL)
    return;

  /*  Get text string and convert it into the cycle number.  Check to see
      if the cycle number os valid.
  */
  str_p = XmTextGetString (w);
  if (str_p == NULL)
    {
    InfoWarn_Show (SLT_ERROR_NOSTRING);
    return;
    }
  if (sscanf (str_p, "%lu", &cycle) != 1)
    {
    InfoWarn_Show (SLT_WARN_CONVERT);
    XtFree (str_p);
    return;
    }

  XtFree (str_p);

  /*  If a valid number was entered, update the select trace viewer 
      and display the pst tree.  
  */
  selt_p = PstView_SelTrace_Get (pv_p);
  cycle--;
  if (cycle >= SelTrace_NumCycles_Get (selt_p))
    {
    InfoWarn_Show (SLT_WARN_INVALID);
    return;
    }

  if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_SOLV)
    {
    Subgoal_t     *sg_p;
    Boolean_t      path_allsolved;

    /*  We have to check to make sure the subgoals along the path
        from the the selected compound to the root are ``all solved''.
        This ain't a pretty looping structure, but it works.
    */
    sg_p = PstComp_Father_Get (SelTrace_IthSelCmp_Get (selt_p, cycle));
    path_allsolved = TRUE;
    while (sg_p != NULL && path_allsolved) 
      {
      path_allsolved = path_allsolved && Pst_All_Solved (sg_p);
      if (PstSubg_Father_Get (sg_p) != NULL)
        sg_p = PstComp_Father_Get (PstSubg_Father_Get (sg_p));
      else
        sg_p = NULL;
      }

    if (!path_allsolved)
      {
      InfoWarn_Show (SLT_WARN_NOTSOLVED);
      return ;
      }
    }

  SelTrace_Update (pv_p, cycle);
  return;
}
/*  End of SelTrace_CycleText_CB  */


/****************************************************************************
*
*  Function Name:                 SelTrace_Dismiss_CB
*
*    This routine resets and unmanages (pops down) the SelTrace selector.
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
*    N/A
*
*  Side Effects:
*
*    Unmanages SelTrace selector.
*
******************************************************************************/
void SelTrace_Dismiss_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p
  )
{
  SelTrace_t  *selt_p;

  selt_p = (SelTrace_t *) client_p;
  if (selt_p == NULL)
    return;

  SelTrace_Reset (selt_p);
  XtUnmanageChild (SelTrace_FormDlg_Get (selt_p));

  return ;
}
/*  End of SelTrace_Dismiss_CB  */


/****************************************************************************
*
*  Function Name:                 SelTrace_NumAll_CB
*
*    This routine sets all of the selected compound cycle numbers.
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
*    N/A
*
*  Side Effects:
*
*    None.
*
******************************************************************************/
void SelTrace_NumAll_CB
  (
  Widget         w,
  XtPointer      client_p, 
  XtPointer      call_p
  )
{
  PstView_t     *pv_p;
  SelTrace_t    *selt_p;
  Compound_t    *selcmp_p;
  U32_t          cycle;

  pv_p = (PstView_t *) client_p;
  if (pv_p == NULL)
    return;

  /*  Set the trace and cycle number of all selected compounds in PST.  */
  selt_p = PstView_SelTrace_Get (pv_p);
  if (SelTrace_SelCmps_Get (selt_p) == NULL)
    if (!SelTrace_Cycles_Set (pv_p))
      return;

  PstView_Mouse_Remove (pv_p);

  for (cycle = 0; cycle < SelTrace_NumCycles_Get (selt_p); cycle++)
    {
    selcmp_p = SelTrace_IthSelCmp_Get (selt_p, cycle);
    PstView_Trace_Store (PstView_CmpInfo_Get (pv_p), selcmp_p, 
      cycle + 1, TRUE);
    }

  PstView_Display (pv_p);
  return;
}
/*  End of SelTrace_NumAll_CB  */

/*  End of SEL_TRACE.C  */
