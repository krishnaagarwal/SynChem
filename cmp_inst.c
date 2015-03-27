/******************************************************************************
*
*  Copyright (C) 1998 SmithKline Beecham, Daren Krebsbach
*  Based upon PST.C and PST_VIEW.C, Copyright (C) The Synchem Project
*
*  Module Name:                     CMP_INST.C
*
*    This module defines the routines needed to find all instances of the
*    same compound in a given status file and display it in the PST
*    viewer.
*
*  Creation Date:
*
*    24-Jul-1998
*
*  Authors:
*
*    Daren Krebsbach
*
*  Routines:
*
*    CmpInst_Create
*    CmpInst_Insts_Set
*    CmpInst_Init
*    CmpInst_Reset
*    CmpInst_Setup
*    CmpInst_Update
*
*  Callback Routines:
*
*    CmpInst_Clear_CB
*    CmpInst_CmpSel_CB
*    CmpInst_CmpText_CB
*    CmpInst_InstNext_CB
*    CmpInst_InstPrev_CB
*    CmpInst_InstText_CB
*    CmpInst_Dismiss_CB
*    CmpInst_NumAll_CB
*    CmpInst_STInxSpecify_CB
*    CmpInst_SlgSpecify_CB
*
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 29-Mar-01  Miller     Modified sling search to use pipe, so as not to create
*                       a conflict between valcomp and submit - only one or the
*                       other may be used safely in a single executable (unless
*                       drastic modifications are made to replicate all the
*                       components of the submission frame)!
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include <Xm/DrawingA.h>
#include <Xm/ArrowB.h>
#include <Xm/DialogS.h>
#include <Xm/Text.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
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

#ifndef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_SLING2XTR_
#include "sling2xtr.h"
#endif

#ifndef _H_PST_
#include "pst.h"
#endif

#ifndef _H_APP_RESRC_
#include "app_resrc.h"
#endif

#ifndef _H_INFO_VIEW_
#include "info_view.h"
#endif

#ifndef _H_MOL_VIEW_
#include "mol_view.h"
#endif

#ifndef _H_PST_VIEW_
#include "pst_view.h"
#endif

#ifndef _H_CMP_INST_
#include "cmp_inst.h"
#endif

#ifndef _H_SYN_VIEW_
#include "syn_view.h"
#endif

#ifndef _H_EXTERN_
#include "extern.h"
#endif

static PstView_t *glob_pvp;
static CmpInst_t *glob_cinst_p;
static Widget stispec_pb, slgspec_pb, tl;
static char *glob_path;

/*  Static Routines  */
static   void    SMolDA_RszExp_CB (Widget, XtPointer, XtPointer);

void CmpInst_SlgSpecify_CB (Widget, XtPointer, XtPointer);
void CmpInst_STInxSpecify_CB (Widget, XtPointer, XtPointer);

void CmpInst_Path_Put (char *path)
{
  glob_path = path;
/*
printf("%s\n",path);
*/
}

/****************************************************************************
*
*  Function Name:                 CmpInst_Create
*
*    This routine creates the Compound Dialog.
*
*
*  Implicit Inputs:
*
*    GAppDim, GSynAppR, SynView application resources.
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
void CmpInst_Create
  (
  Widget         parent,
  PstView_t     *pv_p
  )
{
  ScreenAttr_t  *scra_p;                  /* Screen Attritbutes */
  CmpInst_t     *cinst_p;
  XmString       lbl_str;
  XmString       title;
  Widget         formdg, cmpnf, instf, pbf, separator;
  Dimension      da_width, da_height;
  Dimension      fm_width, fm_height;

glob_pvp = pv_p;
tl=parent;
while (XtParent (tl) != NULL) tl=XtParent(tl);
sshot_comp[0] = 0;
  scra_p = SynAppR_ScrnAtrb_Get (&GSynAppR);
  cinst_p = PstView_CmpInsts_Get (pv_p);
  CmpInst_Init (cinst_p);

  fm_width = (Dimension)((Screen_Width_Get (scra_p) - SWS_APPSHELL_OFFY) 
    * CDI_DA_WIDTH_SCALE);
  fm_width = (fm_width > CDI_DA_WIDTH_MIN) ? fm_width : CDI_DA_WIDTH_MIN;
  da_width = fm_width - (4 * AppDim_MargFmFr_Get (&GAppDim) 
    + 2 * AppDim_SepSmall_Get (&GAppDim));
  da_height = da_width;
  fm_height = da_height + 3 * AppDim_HtLblPB_Get (&GAppDim)
    + 2 * AppDim_HtTxt_Get (&GAppDim) 
    + 12 * AppDim_MargLblPB_Get (&GAppDim)
    + 6 * AppDim_SepSmall_Get (&GAppDim)
    + 6 * AppDim_MargFmFr_Get (&GAppDim);

  /*  Create the dialog form.  */
  title = XmStringCreateLtoR (CDI_TITLE, XmFONTLIST_DEFAULT_TAG);
  formdg = XmCreateFormDialog (parent, "CmpInstFmDg", NULL, 0);
  XtVaSetValues (formdg,
    XmNdialogStyle,    XmDIALOG_MODELESS, 
    XmNlabelFontList,  SynAppR_FontList_Get (&GSynAppR),
    XmNbuttonFontList, SynAppR_FontList_Get (&GSynAppR),
    XmNtextFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNresizePolicy,   XmRESIZE_GROW,
    XmNresizable,      True,
    XmNautoUnmanage,   False, 
    XmNdialogTitle,    title, 
    XmNmarginHeight,   AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,    AppDim_MargFmFr_Get (&GAppDim),
    XmNheight,         fm_height,
    XmNwidth,          fm_width,
    NULL);
  XmStringFree (title);
  CmpInst_FormDlg_Put (cinst_p, formdg);

  CmpInst_MolDAHt_Put (cinst_p, da_height);
  CmpInst_MolDAWd_Put (cinst_p, da_width);
  CmpInst_MolDA_Put (cinst_p, XtVaCreateManagedWidget ("CmpInstDA", 
    xmDrawingAreaWidgetClass,  formdg, 
    XmNbackground,   SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground,   SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNmarginHeight, 0,
    XmNmarginWidth,  0,
    NULL));
  XtAddCallback (CmpInst_MolDA_Get (cinst_p), XmNresizeCallback, 
    SMolDA_RszExp_CB, (XtPointer) cinst_p);
  XtAddCallback (CmpInst_MolDA_Get (cinst_p), XmNexposeCallback, 
    SMolDA_RszExp_CB, (XtPointer) cinst_p);

  instf = XtVaCreateWidget ("CmpInstFm", 
    xmFormWidgetClass,  formdg, 
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       True,
    NULL);

  CmpInst_InstPrevAB_Put (cinst_p, XtVaCreateManagedWidget ("CmpInstAB", 
    xmArrowButtonWidgetClass, instf,
    XmNarrowDirection,        XmARROW_LEFT,
    XmNmultiClick,            XmMULTICLICK_DISCARD, 
    XmNshowAsDefault,         False,
    XmNwidth,                 CDI_ARROW_DIM, 
    NULL));
  XtAddCallback (CmpInst_InstPrevAB_Get (cinst_p), XmNactivateCallback, 
    CmpInst_InstPrev_CB, (XtPointer) pv_p);


  CmpInst_InstNextAB_Put (cinst_p, XtVaCreateManagedWidget ("CmpInstAB", 
    xmArrowButtonWidgetClass, instf,
    XmNarrowDirection,        XmARROW_RIGHT,
    XmNmultiClick,            XmMULTICLICK_DISCARD, 
    XmNshowAsDefault,         False,
    XmNwidth,                 CDI_ARROW_DIM, 
    NULL));
  XtAddCallback (CmpInst_InstNextAB_Get (cinst_p), XmNactivateCallback, 
    CmpInst_InstNext_CB, (XtPointer) pv_p);
  sprintf (CmpInst_InstBuf_Get (cinst_p), "%hu", 0);
  CmpInst_InstText_Put (cinst_p, XtVaCreateManagedWidget ("CmpInstT", 
    xmTextWidgetClass,         instf,
    XmNeditMode,               XmSINGLE_LINE_EDIT,
    XmNeditable,               True,
    XmNautoShowCursorPosition, True,
    XmNcursorPositionVisible,  True,
    XmNmaxLength,              CDI_NUM_BUFLEN - 1,
    XmNcolumns,                CDI_NUM_BUFLEN - 1,
    XmNvalue,                  CmpInst_InstBuf_Get (cinst_p),
    XmNmarginWidth,            AppDim_MargLblPB_Get (&GAppDim),
    XmNmarginHeight,           AppDim_MargLblPB_Get (&GAppDim),
    XmNbackground,             SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNshowAsDefault,          False,
    XmNheight,                 AppDim_HtTxt_Get (&GAppDim),
    NULL));
  XtAddCallback (CmpInst_InstText_Get (cinst_p), XmNactivateCallback, 
    CmpInst_InstText_CB, (XtPointer) pv_p);

  lbl_str = XmStringCreateLtoR (CDI_LABEL_INSTOUTOF, 
    XmFONTLIST_DEFAULT_TAG);
  CmpInst_TotalLbl_Put (cinst_p, XtVaCreateManagedWidget ("CmpInstL", 
    xmLabelWidgetClass,  instf, 
    XmNlabelType,        XmSTRING,
    XmNlabelString,      lbl_str, 
    XmNalignment,        XmALIGNMENT_BEGINNING,
    XmNrecomputeSize,    False,
    XmNmarginHeight,     0,
    XmNmarginWidth,      0,
      NULL));
  XmStringFree (lbl_str);

  sprintf (CmpInst_TotalBuf_Get (cinst_p), "%hu", 0);
  CmpInst_TotalText_Put (cinst_p, XtVaCreateManagedWidget ("CmpInstT", 
    xmTextWidgetClass,         instf,
    XmNeditMode,               XmSINGLE_LINE_EDIT,
    XmNeditable,               False,
    XmNautoShowCursorPosition, False,
    XmNcursorPositionVisible,  False,
    XmNmaxLength,              CDI_NUM_BUFLEN - 1,
    XmNcolumns,                CDI_NUM_BUFLEN - 1,
    XmNvalue,                  CmpInst_TotalBuf_Get (cinst_p),
    XmNmarginWidth,            AppDim_MargLblPB_Get (&GAppDim),
    XmNmarginHeight,           AppDim_MargLblPB_Get (&GAppDim),
    XmNbackground,             SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNheight,                 AppDim_HtTxt_Get (&GAppDim), 
    XmNshowAsDefault,          False,
    NULL));

  XtVaSetValues (CmpInst_InstText_Get (cinst_p),
    XmNtopOffset,        AppDim_SepSmall_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       2 * CDI_ARROW_DIM 
                           + 3 * AppDim_SepSmall_Get (&GAppDim),
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_NONE,
    NULL);

  XtVaSetValues (CmpInst_InstPrevAB_Get (cinst_p),
    XmNtopOffset,        AppDim_SepSmall_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       AppDim_SepSmall_Get (&GAppDim),
    XmNleftAttachment,    XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_NONE,
    NULL);

  XtVaSetValues (CmpInst_InstNextAB_Get (cinst_p),
    XmNtopOffset,        AppDim_SepSmall_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       CDI_ARROW_DIM + 2 * AppDim_SepSmall_Get (&GAppDim),
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_NONE,
    NULL);

  XtVaSetValues (CmpInst_TotalLbl_Get (cinst_p),
    XmNtopOffset,        AppDim_SepSmall_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       CmpInst_InstText_Get (cinst_p),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_NONE,
    NULL);

  XtVaSetValues (CmpInst_TotalText_Get (cinst_p),
    XmNtopOffset,        AppDim_SepSmall_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_NONE,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);


  cmpnf = XtVaCreateWidget ("CmpInstFm", 
    xmFormWidgetClass,  formdg, 
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       True,
    NULL);

  lbl_str = XmStringCreateLtoR (CDI_LABEL_CMP, XmFONTLIST_DEFAULT_TAG);
  CmpInst_CmpLbl_Put (cinst_p, XtVaCreateManagedWidget ("CmpInstL", 
    xmLabelWidgetClass,  cmpnf, 
    XmNlabelType,        XmSTRING,
    XmNlabelString,      lbl_str, 
    XmNalignment,        XmALIGNMENT_BEGINNING,
    XmNrecomputeSize,    False,
    XmNmarginHeight,     0,
    XmNmarginWidth,      0,
    NULL));
  XmStringFree (lbl_str);

  sprintf (CmpInst_CmpBuf_Get (cinst_p), "%lu", 0);
  CmpInst_CmpText_Put (cinst_p, XtVaCreateManagedWidget ("CmpInstT", 
    xmTextWidgetClass,         cmpnf,
    XmNeditMode,               XmSINGLE_LINE_EDIT,
    XmNeditable,               True,
    XmNautoShowCursorPosition, False,
    XmNcursorPositionVisible,  False,
    XmNmaxLength,              CDI_NUM_BUFLEN - 1,
    XmNcolumns,                CDI_NUM_BUFLEN - 1,
    XmNvalue,                  CmpInst_CmpBuf_Get (cinst_p),
    XmNmarginWidth,            AppDim_MargLblPB_Get (&GAppDim),
    XmNmarginHeight,           AppDim_MargLblPB_Get (&GAppDim),
    XmNbackground,             SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNheight,                 AppDim_HtTxt_Get (&GAppDim), 
    XmNshowAsDefault,          False,
    NULL));
  XtAddCallback (CmpInst_CmpText_Get (cinst_p), XmNactivateCallback, 
    CmpInst_CmpText_CB, (XtPointer) pv_p);

  XtVaSetValues (CmpInst_CmpText_Get (cinst_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_NONE,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (CmpInst_CmpLbl_Get (cinst_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      AppDim_SepLarge_Get (&GAppDim),
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      CmpInst_CmpText_Get (cinst_p),
    NULL);

  /*  Create the select and dismiss pushbuttons.  */
  pbf = XtVaCreateWidget ("CmpInstPBFm", 
    xmFormWidgetClass,  formdg, 
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       True,
    NULL);

  separator = XtVaCreateManagedWidget ("CmpInstSep",  
    xmSeparatorWidgetClass, pbf,
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmSHADOW_ETCHED_IN,
    XmNheight,              AppDim_SepSmall_Get (&GAppDim),
    NULL);

  lbl_str = XmStringCreateLtoR (" Find Compound Number ", XmFONTLIST_DEFAULT_TAG);
  stispec_pb = XtVaCreateManagedWidget ("CmpInstPB", 
    xmPushButtonWidgetClass,  pbf, 
    XmNlabelType,             XmSTRING,
    XmNlabelString,           lbl_str, 
    XmNshowAsDefault,         False,
    XmNheight,                AppDim_HtLblPB_Get (&GAppDim), 
    XmNmarginWidth,           AppDim_MargLblPB_Get (&GAppDim),
    XmNmarginHeight,          AppDim_MargLblPB_Get (&GAppDim),
    NULL);
  XmStringFree (lbl_str);
  XtAddCallback (stispec_pb, XmNactivateCallback, 
    CmpInst_STInxSpecify_CB, (XtPointer) cinst_p);

  lbl_str = XmStringCreateLtoR (" Find Sling or Structure ", XmFONTLIST_DEFAULT_TAG);
  slgspec_pb = XtVaCreateManagedWidget ("CmpInstPB", 
    xmPushButtonWidgetClass,  pbf, 
    XmNlabelType,             XmSTRING,
    XmNlabelString,           lbl_str, 
    XmNshowAsDefault,         False,
    XmNheight,                AppDim_HtLblPB_Get (&GAppDim), 
    XmNmarginWidth,           AppDim_MargLblPB_Get (&GAppDim),
    XmNmarginHeight,          AppDim_MargLblPB_Get (&GAppDim),
    NULL);
  XmStringFree (lbl_str);
  XtAddCallback (slgspec_pb, XmNactivateCallback, 
    CmpInst_SlgSpecify_CB, (XtPointer) cinst_p);

  lbl_str = XmStringCreateLtoR (CDI_PUSHB_DISMISS, XmFONTLIST_DEFAULT_TAG);
  CmpInst_DismissPB_Put (cinst_p, XtVaCreateManagedWidget ("CmpInstPB", 
    xmPushButtonWidgetClass,  pbf, 
    XmNlabelType,             XmSTRING,
    XmNlabelString,           lbl_str, 
    XmNshowAsDefault,         False,
    XmNheight,                AppDim_HtLblPB_Get (&GAppDim), 
    XmNmarginWidth,           AppDim_MargLblPB_Get (&GAppDim),
    XmNmarginHeight,          AppDim_MargLblPB_Get (&GAppDim),
    NULL));
  XmStringFree (lbl_str);
  XtAddCallback (CmpInst_DismissPB_Get (cinst_p), XmNactivateCallback, 
    CmpInst_Dismiss_CB, (XtPointer) cinst_p);

  lbl_str = XmStringCreateLtoR (CDI_PUSHB_SELECT, XmFONTLIST_DEFAULT_TAG);
  CmpInst_CmpSelPB_Put (cinst_p, XtVaCreateManagedWidget ("CmpInstPB", 
    xmPushButtonWidgetClass, pbf, 
    XmNlabelType,            XmSTRING,
    XmNlabelString,          lbl_str, 
    XmNshowAsDefault,        False,
    XmNheight,               AppDim_HtLblPB_Get (&GAppDim), 
    XmNmarginWidth,          AppDim_MargLblPB_Get (&GAppDim),
    XmNmarginHeight,         AppDim_MargLblPB_Get (&GAppDim),
    NULL));
  XmStringFree (lbl_str);
  XtAddCallback (CmpInst_CmpSelPB_Get (cinst_p), XmNactivateCallback, 
    CmpInst_CmpSel_CB, (XtPointer) pv_p);


  XtVaSetValues (separator,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (stispec_pb,
    XmNtopOffset,        AppDim_SepSmall_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        separator,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftPosition,     15,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNrightOffset,      0,
    XmNrightPosition,    85,
    XmNrightAttachment,  XmATTACH_POSITION,
    NULL);

  XtVaSetValues (slgspec_pb,
    XmNtopOffset,        AppDim_SepSmall_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        stispec_pb,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftPosition,     15,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNrightOffset,      0,
    XmNrightPosition,    85,
    XmNrightAttachment,  XmATTACH_POSITION,
    NULL);

  XtVaSetValues (CmpInst_CmpSelPB_Get (cinst_p),
    XmNtopOffset,        AppDim_SepSmall_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        slgspec_pb,
    XmNbottomOffset,     AppDim_SepSmall_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftPosition,     15,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNrightOffset,      0,
    XmNrightPosition,    43,
    XmNrightAttachment,  XmATTACH_POSITION,
    NULL);

  XtVaSetValues (CmpInst_DismissPB_Get (cinst_p),
    XmNtopOffset,        AppDim_SepSmall_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        slgspec_pb,
    XmNbottomOffset,     AppDim_SepSmall_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftPosition,     57,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNrightOffset,      0,
    XmNrightPosition,    85,
    XmNrightAttachment,  XmATTACH_POSITION,
    NULL);

  XtVaSetValues (instf,
    XmNtopOffset,        AppDim_SepSmall_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     -(AppDim_HtTxt_Get (&GAppDim) 
                           + 2 * AppDim_MargLblPB_Get (&GAppDim)
                           + AppDim_SepSmall_Get (&GAppDim)),
    XmNbottomAttachment, XmATTACH_OPPOSITE_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (pbf,
    XmNtopOffset,        -(3 * AppDim_SepSmall_Get (&GAppDim) 
                           + 6 * AppDim_MargLblPB_Get (&GAppDim)
                           + 3 * AppDim_HtLblPB_Get (&GAppDim)),
    XmNtopAttachment,    XmATTACH_OPPOSITE_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (cmpnf,
    XmNtopOffset,        -(4 * AppDim_SepSmall_Get (&GAppDim) 
                           + 2 * AppDim_MargLblPB_Get (&GAppDim)
                           + 6 * AppDim_MargLblPB_Get (&GAppDim)
                           + 3 * AppDim_HtLblPB_Get (&GAppDim)
                           + AppDim_HtTxt_Get (&GAppDim)),
    XmNtopAttachment,    XmATTACH_OPPOSITE_FORM,
    XmNbottomOffset,     4 * AppDim_SepSmall_Get (&GAppDim)
                           + 6 * AppDim_MargLblPB_Get (&GAppDim)
                           + 3 * AppDim_HtLblPB_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (CmpInst_MolDA_Get (cinst_p),
    XmNtopOffset,        AppDim_SepSmall_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        instf,
    XmNbottomOffset,     AppDim_SepSmall_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget,     cmpnf,
    XmNleftOffset,       AppDim_SepSmall_Get (&GAppDim),
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      AppDim_SepSmall_Get (&GAppDim),
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtManageChild (cmpnf);
  XtManageChild (pbf);
  XtManageChild (instf);

  XtSetSensitive (CmpInst_InstNextAB_Get (cinst_p), False); 
  XtSetSensitive (CmpInst_InstPrevAB_Get (cinst_p), False); 
  CmpInst_IsCreated_Put (cinst_p, TRUE);

  return ;
}
/*  End of CmpInst_Create  */


/****************************************************************************
*
*  Function Name:                 CmpInst_Init
*
*    This routine initializes the Compound Instances data structure fields not
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
void CmpInst_Init
  (
  CmpInst_t    *cinst_p
  )
{
  CmpInst_CurrCmp_Put (cinst_p, NULL);
  CmpInst_CurrInst_Put (cinst_p, CDI_CMPINST_NONE);
  CmpInst_NumInsts_Put (cinst_p, 0);
  CmpInst_CmpInsts_Put (cinst_p, NULL);
  CmpInst_Molecule_Put (cinst_p, NULL);

  return ;
}
/*  End of CmpInst_Init  */


/****************************************************************************
*
*  Function Name:                 CmpInst_Reset
*
*    This routine resets the Compound Instances by freeing the memory for 
*    the array of compound pointers and reinitializing variables.
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
void CmpInst_Reset
  (
  CmpInst_t    *cinst_p
  )
{

  if (CmpInst_CmpInsts_Get (cinst_p) != NULL)
    free (CmpInst_CmpInsts_Get (cinst_p));

  if (CmpInst_Molecule_Get (cinst_p) != NULL)
    free_Molecule (CmpInst_Molecule_Get (cinst_p));

  CmpInst_Init (cinst_p);

  sprintf (CmpInst_CmpBuf_Get (cinst_p), "%lu", 0);
  XmTextSetString (CmpInst_CmpText_Get (cinst_p), 
    CmpInst_CmpBuf_Get (cinst_p));

  sprintf (CmpInst_InstBuf_Get (cinst_p), "%hu", 0);
  XmTextSetString (CmpInst_InstText_Get (cinst_p), 
    CmpInst_InstBuf_Get (cinst_p));

  sprintf (CmpInst_TotalBuf_Get (cinst_p), "%hu", 0);
  XmTextSetString (CmpInst_TotalText_Get (cinst_p), 
    CmpInst_TotalBuf_Get (cinst_p));

  XtSetSensitive (CmpInst_InstNextAB_Get (cinst_p), False); 
  XtSetSensitive (CmpInst_InstPrevAB_Get (cinst_p), False); 

  XClearArea (XtDisplay (CmpInst_MolDA_Get (cinst_p)), 
    XtWindow (CmpInst_MolDA_Get (cinst_p)), 0, 0, 0, 0, True);


  return ;
}
/*  End of CmpInst_Reset  */


/****************************************************************************
*
*  Function Name:                 CmpInst_Insts_Set
*
*    This routine creates and initializes the array of compound instances
*    for the selected compound which satisfy the tree type.  
*    It also clears all traced compound flags in the compound infos.*
*
*    Only noncyclic instances are included.  This code has been commented
*    out so that all instances satisfying the tree type are added.
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
Boolean_t CmpInst_Insts_Set
  (
  PstView_t     *pv_p
  )
{
  CmpInst_t     *cinst_p;
  PstCB_t       *pstcb_p;
  PsvCmpInfo_t  *cmp_infos;
  Compound_t   **cmpinsts;
  Compound_t    *cmp_p;
  Compound_t    *brother_p;
  Compound_t    *grandpa_p;
  Subgoal_t     *father_p;
  SymTab_t      *symtab;
  U32_t          comp_id;
  U16_t          inst_i;
  U16_t          num_noncyclic;
  Boolean_t      treetype_ok;
  Boolean_t      cyclic;

  pstcb_p = PstView_PstCB_Get (pv_p);
  if (pstcb_p == NULL)
    {
    InfoWarn_Show (CDI_WARN_NOPST);
    return (FALSE);
    }

  cmp_infos = PstView_CmpInfo_Get (pv_p);
  if (cmp_infos != NULL)
    {
    U32_t        cmp_i;

    for (cmp_i = 0; cmp_i < PstView_NumCmpI_Get (pv_p); cmp_i++)
      {
      PsvCmpI_Traced_Unset (cmp_infos[cmp_i]);
      PsvCmpI_TracElse_Unset (cmp_infos[cmp_i]);
      }
    }

  cinst_p = PstView_CmpInsts_Get (pv_p);
  CmpInst_Reset (cinst_p);

  if (PstView_LastSelCmp_Get (pv_p) != NULL)
    {
    CmpInst_CurrCmp_Put (cinst_p, PstView_LastSelCmp_Get (pv_p));
    }
  else
    {
    InfoWarn_Show (CDI_WARN_NOCURCMP);
    return (FALSE);
    }

  /*  First find out how many noncyclic compound instances there are.  
      If the current tree is all solved or developed only, make sure
      these test are also satisfied.
  */
  symtab = PstComp_SymbolTable_Get (CmpInst_CurrCmp_Get (cinst_p));
  comp_id = SymTab_Index_Get (symtab);
  num_noncyclic = 0;

  /*  For each instance of the compound, climb up the tree checking to see
      if the path is cyclic, that each subgoal satisfies the tree type.
  */

  cmp_p = SymTab_FirstComp_Get (symtab);
  while (cmp_p != NULL)
    {
    cyclic = FALSE;
    treetype_ok = TRUE;
    father_p = PstComp_Father_Get (cmp_p);
    if (father_p != NULL)
      {
      brother_p = PstSubg_Son_Get (father_p);
      grandpa_p = PstSubg_Father_Get (father_p);
      }
    else 
      {
      brother_p = NULL;
      grandpa_p = NULL;
      }

    /*  First check that all brothers are developed or all solved.  */
    if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_DEV)
      {
      while (treetype_ok && brother_p != NULL)
        {
        if (SymTab_DevelopedComp_Get (PstComp_SymbolTable_Get (brother_p)) 
            == NULL)
          treetype_ok = FALSE;

        brother_p = PstComp_Brother_Get (brother_p);
        }

      treetype_ok = treetype_ok || Pst_All_Solved (father_p);
      }

    else if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_SOLV)
      {
      treetype_ok = Pst_All_Solved (father_p);
      }

    while (treetype_ok && !cyclic && grandpa_p != NULL)
      {
      if (SymTab_Index_Get (PstComp_SymbolTable_Get (grandpa_p)) == comp_id)
        {
        cyclic = FALSE;  /*  cyclic = TRUE;  */
        }
/*      else
        {  */
        father_p = PstComp_Father_Get (grandpa_p);
        if (father_p != NULL)
          {
          brother_p = PstSubg_Son_Get (father_p);
          grandpa_p = PstSubg_Father_Get (father_p);
          }
        else 
          {
          brother_p = NULL;
          grandpa_p = NULL;
          }

        if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_DEV)
          {
          while (treetype_ok && brother_p != NULL)
            {
            if (SymTab_DevelopedComp_Get (PstComp_SymbolTable_Get (brother_p)) 
                == NULL)
              treetype_ok = FALSE;

            brother_p = PstComp_Brother_Get (brother_p);
            }

          treetype_ok = treetype_ok || Pst_All_Solved (father_p);
          }

        else if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_SOLV)
          {
          treetype_ok = Pst_All_Solved (father_p);
          }
/*        }  */
      }  /* End of while heading up path */

    /*  If we made it up to the root of the tree (grandpa is NULL) with
        no cycles found and treetype still okay, then count this compound.
    */
    if (!cyclic && treetype_ok)
      {
      num_noncyclic++;
      }

    cmp_p = PstComp_Prev_Get (cmp_p);
    }

  /*  Allocate the array of compound instances (compound node ptrs).  */
  CmpInst_NumInsts_Put (cinst_p, num_noncyclic);

  cmpinsts = (Compound_t **) malloc (CmpInst_NumInsts_Get (cinst_p) 
    * sizeof (Compound_t *));
  if (cmpinsts == NULL)
    {
    fprintf (stderr, 
      "\nCmpInst_Reset:  unable to allocate memory for comp inst array.\n");
    exit (-1);
    }

  CmpInst_CmpInsts_Put (cinst_p, cmpinsts);

  /*  Now that we know how many noncyclic instances there are, we can go
      through the instances again, this time saving them.  Since the first 
      instance is really the last, place the compounds in the array in 
      reverse order.
  */
  cmp_p = SymTab_FirstComp_Get (symtab);
  inst_i = CmpInst_NumInsts_Get (cinst_p);
  while (inst_i > 0 && cmp_p != NULL)
    {
    cyclic = FALSE;
    treetype_ok = TRUE;
    father_p = PstComp_Father_Get (cmp_p);
    if (father_p != NULL)
      {
      brother_p = PstSubg_Son_Get (father_p);
      grandpa_p = PstSubg_Father_Get (father_p);
      }
    else 
      {
      brother_p = NULL;
      grandpa_p = NULL;
      }

    /*  First check that all brothers are developed or all solved.  */
    if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_DEV)
      {
      while (treetype_ok && brother_p != NULL)
        {
        if (SymTab_DevelopedComp_Get (PstComp_SymbolTable_Get (brother_p)) 
            == NULL)
          treetype_ok = FALSE;

        brother_p = PstComp_Brother_Get (brother_p);
        }

      treetype_ok = treetype_ok || Pst_All_Solved (father_p);
      }

    else if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_SOLV)
      {
      treetype_ok = Pst_All_Solved (father_p);
      }

    while (treetype_ok && !cyclic && grandpa_p != NULL)
      {
      if (SymTab_Index_Get (PstComp_SymbolTable_Get (grandpa_p)) == comp_id)
        {
        cyclic = FALSE;  /*  cyclic = TRUE;  */
        }
/*      else
        {  */
        father_p = PstComp_Father_Get (grandpa_p);
        if (father_p != NULL)
          {
          brother_p = PstSubg_Son_Get (father_p);
          grandpa_p = PstSubg_Father_Get (father_p);
          }
        else 
          {
          brother_p = NULL;
          grandpa_p = NULL;
          }

        if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_DEV)
          {
          while (treetype_ok && brother_p != NULL)
            {
            if (SymTab_DevelopedComp_Get (PstComp_SymbolTable_Get (brother_p)) 
                == NULL)
              treetype_ok = FALSE;

            brother_p = PstComp_Brother_Get (brother_p);
            }

          treetype_ok = treetype_ok || Pst_All_Solved (father_p);
          }

        else if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_SOLV)
          {
          treetype_ok = Pst_All_Solved (father_p);
          }
/*        }  */
      }  /* End of while heading up path */

    if (!cyclic && treetype_ok)
      {
      cmpinsts[inst_i - 1] = cmp_p;
      inst_i--;
      }

    cmp_p = PstComp_Prev_Get (cmp_p);
    }

  if (inst_i != 0)
    {
    fprintf (stderr, 
      "\nCmpInst_Insts_Set:  missed an instance.\n");
    return (FALSE);
    }

  CmpInst_CurrInst_Put (cinst_p, 0);

  return (TRUE);
}
/*  End of CmpInst_Insts_Set  */


/****************************************************************************
*
*  Function Name:                 CmpInst_Setup
*
*    This routine sets up Compound Instances after a compound has been
*    selected.  It stores the set of instances for the current compound,
*    displays the molecule and information, and then updates the PST viewer
*    with the first instance of the compound.
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
Boolean_t CmpInst_Setup
  (
  PstView_t     *pv_p
  )
{
  CmpInst_t     *cinst_p;
  PstCB_t       *pstcb_p;
  SymTab_t      *symtab;
  Xtr_t         *temp_xtr_p;
  Sling_t        temp_sling;

  pstcb_p = PstView_PstCB_Get (pv_p);
  cinst_p = PstView_CmpInsts_Get (pv_p);

  if (!CmpInst_Insts_Set (pv_p))
    return (FALSE);

  symtab = PstComp_SymbolTable_Get (CmpInst_CurrCmp_Get (cinst_p));

  sprintf (CmpInst_CmpBuf_Get (cinst_p), "%lu", SymTab_Index_Get (symtab));
  XmTextSetString (CmpInst_CmpText_Get (cinst_p), 
    CmpInst_CmpBuf_Get (cinst_p));

  sprintf (CmpInst_InstBuf_Get (cinst_p), "%hu", 
    CmpInst_CurrInst_Get (cinst_p) + 1);
  XmTextSetString (CmpInst_InstText_Get (cinst_p), 
    CmpInst_InstBuf_Get (cinst_p));

  sprintf (CmpInst_TotalBuf_Get (cinst_p), "%hu",
    CmpInst_NumInsts_Get (cinst_p));
  XmTextSetString (CmpInst_TotalText_Get (cinst_p), 
    CmpInst_TotalBuf_Get (cinst_p));


  if (CmpInst_NumInsts_Get (cinst_p) > 1)
    XtSetSensitive (CmpInst_InstNextAB_Get (cinst_p), True); 
  else
    XtSetSensitive (CmpInst_InstNextAB_Get (cinst_p), False); 

  XtSetSensitive (CmpInst_InstPrevAB_Get (cinst_p), False); 

  /*  Display selected molecule in window.  */
  temp_sling = Sling_CopyTrunc (SymTab_Sling_Get (symtab));
  temp_xtr_p = Sling2Xtr (temp_sling);
  CmpInst_Molecule_Put (cinst_p, Xtr2Dsp (temp_xtr_p));
  Sling_Destroy (temp_sling);
  Xtr_Destroy (temp_xtr_p);
  if (!dsp_Shelley (CmpInst_Molecule_Get (cinst_p)))
    {
    free_Molecule (CmpInst_Molecule_Get (cinst_p));
    RxnPreV_Mol_Put (cinst_p, NULL);
    InfoWarn_Show (CDI_WARN_SHELLEY_ABORT);
    }

  Mol_Scale (CmpInst_Molecule_Get (cinst_p), CmpInst_MolDAHt_Get (cinst_p),
    CmpInst_MolDAWd_Get (cinst_p));
  XClearArea (XtDisplay (CmpInst_MolDA_Get (cinst_p)), 
    XtWindow (CmpInst_MolDA_Get (cinst_p)), 0, 0, 0, 0, True);

  /*  Update the PST with the first instance of compound.  */
  CmpInst_Update (pv_p, 0);

  return (TRUE);
}
/*  End of CmpInst_Setup  */



/****************************************************************************
*
*  Function Name:                 CmpInst_Update
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
void CmpInst_Update
  (
  PstView_t       *pv_p, 
  U16_t            instance
  )
{
  CmpInst_t       *cinst_p;
  Compound_t      *selcmp_p;
  PsvTreeLvl_t    *tlvl_p;                  /* Pst tree level */
  PsvTreeLvl_t    *pthtlvl_p;               /* Path tree level */
  PsvTreeLvl_t    *switchtl_p;
  U16_t            cmp_i;
  U16_t            node_i;
  U16_t            tlvl_num;
  Boolean_t        found;

  /*  Store the current instance number, set up the previous and next buttons.
  */
  PstView_Mouse_Remove (pv_p);
  cinst_p = PstView_CmpInsts_Get (pv_p);
  CmpInst_CurrInst_Put (cinst_p, instance);
  sprintf (CmpInst_InstBuf_Get (cinst_p), "%hu", 
    CmpInst_CurrInst_Get (cinst_p) + 1);
  XmTextSetString (CmpInst_InstText_Get (cinst_p), 
    CmpInst_InstBuf_Get (cinst_p));

  if (instance > 0)
    XtSetSensitive (CmpInst_InstPrevAB_Get (cinst_p), True); 
  else
    XtSetSensitive (CmpInst_InstPrevAB_Get (cinst_p), False); 

  if (instance == CmpInst_NumInsts_Get (cinst_p) - 1)
    XtSetSensitive (CmpInst_InstNextAB_Get (cinst_p), False); 
  else
    XtSetSensitive (CmpInst_InstNextAB_Get (cinst_p), True); 

  selcmp_p = CmpInst_IthCmpInst_Get (cinst_p, instance);
  CmpInst_CurrCmp_Put (cinst_p, selcmp_p);
  sprintf (CmpInst_CmpBuf_Get (cinst_p), "%1lu", 
    SymTab_Index_Get (PstComp_SymbolTable_Get (selcmp_p)));
  XmTextSetString (CmpInst_CmpText_Get (cinst_p), 
      CmpInst_CmpBuf_Get (cinst_p));

  PstView_Trace_Store (PstView_CmpInfo_Get (pv_p), selcmp_p, 
    instance + 1, FALSE);

  /*  Check to see if the selected compound is somewhere along the 
      current path.  If it is, then move the appropriate level to the
      top view level, set up the bottom two view levels and call the 
      PST display routine.  Otherwise, we need to back up the tree
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
      Compound_t      *cmp_p;
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
    PstView_PathLen_Put (pv_p, tlvl_num + 1);
    }

  PstView_ActVLvl_Put (pv_p, PVW_LEVEL_TOP);
  PstVLvls_SetTwo (pv_p);

  /*  Move up one level, if possible.  */
    tlvl_p = ViewLvl_TreeLvl_Get (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP));
  if (TreeLvl_LvlNum_Get (tlvl_p) > 0)
    {
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
    }  /* End of move up one level.  */

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
/*  End of CmpInst_Update  */

/****************************************************************************
*
*  Function Name:                 CmpInst_CmpSel_CB
*
*    This routine allows the user to specify the compound by selecting
*    a node in the pst view area.  The instances of that compound are loaded,
*    and the first instance found and displayed in pst viewer.
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
void CmpInst_CmpSel_CB
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

  /*  Get the selected compound.  */
  PstView_Compound_Sel (pv_p);
  if (PstView_LastSelCmp_Get (pv_p) == NULL)
    {
    InfoWarn_Show (CDI_WARN_BADSEL);
    return ;
    }

  CmpInst_Setup (pv_p);

  return;
}
/*  End of CmpInst_CmpSel_CB  */


/****************************************************************************
*
*  Function Name:                 CmpInst_CmpText_CB
*
*    This routine allows the user to specify the compound by typing the unique
*    compound index in the text window.  The instances of that compound 
*    are loaded, and the first instance found and displayed in pst viewer.
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
void CmpInst_CmpText_CB
  (
  Widget         w,
  XtPointer      client_p, 
  XtPointer      call_p
  )
{
  PstView_t     *pv_p;
  char          *str_p;                      /* returned number string */
  U32_t          symtab_ind;

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
    InfoWarn_Show (CDI_ERROR_NOSTRING);
    return;
    }
  if (sscanf (str_p, "%lu", &symtab_ind) != 1)
    {
    InfoWarn_Show (CDI_WARN_CONVERT);
    XtFree (str_p);
    return;
    }

  XtFree (str_p);

  /*  Find compound with given unique compound number in symbol table. */

  /*
  PstView_LastSelCmp_Put (pv_p, );
  CmpInst_Setup (pv_p);
  */

  return;
}
/*  End of CmpInst_CmpText_CB  */


/****************************************************************************
*
*  Function Name:                 CmpInst_InstNext_CB
*
*    This routine updates the CmpInst data structure and the PST viewer
*    with the next selected compound.
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
void CmpInst_InstNext_CB
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

  CmpInst_Update (pv_p, 
    CmpInst_CurrInst_Get (PstView_CmpInsts_Get (pv_p)) + 1);

  return ;
}
/*  End of CmpInst_InstNext_CB  */


/****************************************************************************
*
*  Function Name:                 CmpInst_InstPrev_CB
*
*    This routine updates the CmpInst data structure and the PST viewer
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
void CmpInst_InstPrev_CB
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

  CmpInst_Update (pv_p, 
    CmpInst_CurrInst_Get (PstView_CmpInsts_Get (pv_p)) - 1);

  return ;
}
/*  End of CmpInst_InstPrev_CB  */


/****************************************************************************
*
*  Function Name:                 CmpInst_InstText_CB
*
*    This routine updates the CmpInst data structure and the PST viewer
*    with the selected compound for the instance entered in the text window.
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
void CmpInst_InstText_CB
  (
  Widget         w,
  XtPointer      client_p, 
  XtPointer      call_p
  )
{
  PstView_t     *pv_p;
  CmpInst_t     *cinst_p;
  char          *str_p;                      /* returned number string */
  U16_t          instance;

  pv_p = (PstView_t *) client_p;
  if (pv_p == NULL)
    return;

  /*  Get text string and convert it into the instance number.  Check to see
      if the instance number os valid.
  */
  str_p = XmTextGetString (w);
  if (str_p == NULL)
    {
    InfoWarn_Show (CDI_ERROR_NOSTRING);
    return;
    }
  if (sscanf (str_p, "%hu", &instance) != 1)
    {
    InfoWarn_Show (CDI_WARN_CONVERT);
    XtFree (str_p);
    return;
    }

  XtFree (str_p);

  /*  If a valid number was entered, update the select trace viewer 
      and display the pst tree.  
  */
  cinst_p = PstView_CmpInsts_Get (pv_p);
  instance--;
  if (instance >= CmpInst_NumInsts_Get (cinst_p))
    {
    InfoWarn_Show (CDI_WARN_INVALID);
    return;
    }

  CmpInst_Update (pv_p, instance);
  return;
}
/*  End of CmpInst_InstText_CB  */

/****************************************************************************
*
*  Function Name:                 CmpInst_SlgSpecify_CB
*
*  Selects compound via sling (from drawing)
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
*    Unmanages CmpInst selector.
*
******************************************************************************/
void CmpInst_SlgSpecify_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p
  )
{
char sling[512], process[128], line[2000];
FILE *pipe;
  String_t        string;
  Sling_t         sling_st;
  SymTab_t       *symtab_p;
  SymTab_t       *last_p;
  U32_t           hash;
  int             index;
  char            msg_str[256];
  Xtr_t          *xtr;

  glob_cinst_p = (CmpInst_t *) client_p;

#ifdef _CYGWIN_
  sprintf(process,"%sdrawpad.exe \"%s\"",glob_path,sshot_comp);
#else
  sprintf(process,"%sdrawpad \"%s\"",glob_path,sshot_comp);
#endif
#ifdef _DEBUG_
debug_print(process);
#endif
  pipe = popen (process, "r");
  while (fgets (line, 1999, pipe) != NULL) /* bypass any debug output! */
  {
    line[511] = '\0';
    strcpy (sling, line);
  }
  while (sling[0]!='\0' && sling[strlen(sling)-1]<=' ') sling[strlen(sling)-1]='\0';
  if (sling[0]=='\0')
  {
    InfoWarn_Show ("No sling specified.");
    return;
  }
printf("sling=%s\n",sling);
  pclose(pipe);
  strcpy (sshot_comp, sling);
  string = String_Create (sling, 0);
  sling_st = String2Sling (string);

    xtr = Sling2Xtr_PlusHydrogen (sling_st);
/*
    Sling_Destroy (sling_st);
*/
    Xtr_Attr_ResonantBonds_Set (xtr);
    Xtr_Name_Set (xtr, NULL);
    sling_st = Name_Sling_Get (xtr, TRUE);
    Xtr_Destroy(xtr);

  hash = Pst_Hash (sling_st, MX_SYMTAB_BUCKETS);
  symtab_p = SymTab_HashBucketHead_Get (hash);
  index = 0;

  while (symtab_p != NULL && index == 0)
    {
    last_p = symtab_p;
    if (strcasecmp ((char *) Sling_Name_Get (sling_st),
      (char *) Sling_Name_Get (SymTab_Sling_Get (symtab_p))) == 0)
      index = SymTab_Index_Get (symtab_p);
    else
      symtab_p = SymTab_HashChain_Get (symtab_p);
    }

  if (index == 0)
    {
    sprintf (msg_str, "Sling %s not found in PST.", Sling_Name_Get (sling_st));
    Sling_Destroy (sling_st);
    InfoWarn_Show (msg_str);
    return;
    }

  Sling_Destroy (sling_st);

  CmpInst_STInxSpecify_CB (NULL, (XtPointer) glob_cinst_p, (XtPointer) index);
}

/****************************************************************************
*
*  Function Name:                 CmpInst_STInxSpecify_CB
*
*  Selects compound via ST index number
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
*    Unmanages CmpInst selector.
*
******************************************************************************/
void CmpInst_STInxSpecify_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p
  )
{
  CmpInst_t   *cinst_p;
  char        *str_p;
  int          inx, i;
  SymTab_t    *symtab_p;
  Boolean_t    found;

  cinst_p = (CmpInst_t *) client_p;
  if (cinst_p == NULL)
    return;

  if (w != NULL)
  {
    str_p = XmTextGetString (CmpInst_CmpText_Get (cinst_p));
    inx = atoi (str_p);
    XtFree (str_p);
  }
  else inx = (int) call_p;


  if (inx == 0)
  {
    InfoMess_Show ("Incorrect Symbol Table Index specification.");
    return;
  }

  /* Why aren't these indices an integral part of the global PST data structures,
     instead of obtainable only through an inefficient serialization process??? */

  for (i = 0, found = FALSE; i < SymTab_HashSize_Get () && !found; ++i)
    for (symtab_p = SymTab_HashBucketHead_Get (i); symtab_p != NULL && !found; symtab_p = SymTab_HashChain_Get (symtab_p))
    if (SymTab_Index_Get (symtab_p) == inx)
  {
    found = TRUE;
    CmpInst_CurrCmp_Put (cinst_p, SymTab_FirstComp_Get (symtab_p));
    PstView_LastSelCmp_Put (glob_pvp, SymTab_FirstComp_Get (symtab_p));
    CmpInst_Setup (glob_pvp);
  }

  if (!found)
  {
    InfoMess_Show ("No such symbol table index number.");
    return;
  }

  return ;
}
/*  End of CmpInst_STInxSpecify_CB  */

/****************************************************************************
*
*  Function Name:                 CmpInst_Dismiss_CB
*
*    This routine resets and unmanages (pops down) the CmpInst selector.
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
*    Unmanages CmpInst selector.
*
******************************************************************************/
void CmpInst_Dismiss_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p
  )
{
  CmpInst_t   *cinst_p;

  cinst_p = (CmpInst_t *) client_p;
  if (cinst_p == NULL)
    return;

  XtUnmanageChild (CmpInst_FormDlg_Get (cinst_p));

  CmpInst_Reset (cinst_p);

  return ;
}
/*  End of CmpInst_Dismiss_CB  */

/****************************************************************************
*
*  Function Name:                 SMolDA_RszExp_CB
*
*    This routine handles the resize and expose events for the drawing
*    area of the compound instances dialog.  On an expose, it calls the
*    update function with no changes to the data so that the same target
*    molecule is redrawn.  On a resize, it saves the new size of the
*    drawing area.
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
static void SMolDA_RszExp_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  XmDrawingAreaCallbackStruct *cb_p;
  CmpInst_t      *cinst_p;

  cinst_p = (CmpInst_t *) client_p;
  cb_p = (XmDrawingAreaCallbackStruct *) call_p;

  if (cb_p->reason == XmCR_RESIZE)
    {
    Dsp_Molecule_t  *mol_p;
    Dsp_Atom_t      *atom_p;
    U16_t            atm_i;
    Dimension        x_off, y_off;
    Dimension        da_ht, da_wd;

    XtVaGetValues (CmpInst_MolDA_Get (cinst_p),
      XmNheight, &da_ht,
      XmNwidth, &da_wd,
      NULL);

    /*  Unscale the molecule before scaling it with new da size.  */
    mol_p = CmpInst_Molecule_Get (cinst_p);
    if (mol_p != NULL)
      {
      x_off = (Dimension) (CmpInst_MolDAWd_Get (cinst_p) 
        - (mol_p->scale * mol_p->molw) + 0.5) >> 1;
      y_off = (Dimension) (CmpInst_MolDAHt_Get (cinst_p) 
        - (mol_p->scale * mol_p->molh) + 0.5) >> 1;
      atom_p = mol_p->atoms;
      for (atm_i = 0; atm_i < mol_p->natms; atm_i++)
        {
        atom_p->x = (int) (((atom_p->x - x_off) / mol_p->scale) + 0.5);
        atom_p->y = (int) (((atom_p->y - y_off) / mol_p->scale) + 0.5);
        atom_p++;
        }

      mol_p->scale = 1.0;
      Mol_Scale (CmpInst_Molecule_Get (cinst_p), da_ht, da_wd);
      }

    CmpInst_MolDAHt_Put (cinst_p, da_ht);
    CmpInst_MolDAWd_Put (cinst_p, da_wd);
    }

  else if (cb_p->reason == XmCR_EXPOSE)
    {
    XExposeEvent    *event;
    XEvent           skip_exposes;

    /* Since we are redrawing the entire drawing area after each expose
       event, we can remove all following expose events generated by 
       by the same user action.
    */
    event = (XExposeEvent *) cb_p->event;
    if (event->count > 0)
      {
      while (XCheckTypedWindowEvent (event->display, event->window,
             Expose, &skip_exposes));
      }

    XClearWindow (XtDisplay (CmpInst_MolDA_Get (cinst_p)), 
      XtWindow (CmpInst_MolDA_Get (cinst_p)));

    Mol_Draw (CmpInst_Molecule_Get (cinst_p), cb_p->window);
    }

  return ;
}
/*  End of SMolDA_RszExp_CB  */

/*  End of CMP_INST.C  */
