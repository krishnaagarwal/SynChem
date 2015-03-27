/******************************************************************************
*
*  Copyright (C) 1995, Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     SSHOT_VIEW.C
*
*    This module defines the routines needed to view an application of 
*    SingleShot to the selected reaction in the PST viewer.
*
*  Creation Date:
*
*    26-Dec-1995
*
*  Authors:
*
*    Daren Krebsbach
*
*  Routines:
*
*    SShotView_Create
*    SShotView_Reset
*    SShotView_Update
*
*  Callback Routines:
*
*    SShotView_DismissPB_CB
*    SShotView_NextPB_CB
*    SShotView_PrevPB_CB
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

#include <Xm/DialogS.h>
#include <Xm/Text.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>

#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifdef _H_UTL_
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

#ifndef _H_SGLSHOT_
#include "sglshot.h"
#endif

#ifndef _H_PST_
#include "pst.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_APP_RESRC_
#include "app_resrc.h"
#endif

#ifndef _H_DSP_
#include "dsp.h"
#endif

#ifndef _H_MOL_VIEW_
#include "mol_view.h"
#endif

#ifndef _H_RXN_VIEW_
#include "rxn_view.h"
#endif

#ifndef _H_SSHOT_VIEW_
#define _GLOBAL_SSV_DEF_
#include "sshot_view.h"
#undef _GLOBAL_SSV_DEF_
#endif

#ifndef _CYGHAT_
#ifdef _CYGWIN_
#define _CYGHAT_
#else
#ifdef _REDHAT_
#define _CYGHAT_
#endif
#endif
#endif

void ptread_no_init (int, int *, Condition_t **, int *, Posttest_t **, String_t **, String_t **);
void ptwrite (int, Condition_t *, char ****, int, Posttest_t *, char ****, String_t *, String_t *, int, Boolean_t *);


/****************************************************************************
*
*  Function Name:                 SShotView_Create
*
*    This routine creates the SingleShot Viewing Dialog.
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
void SShotView_Create
  (
  Widget         parent,
  SShotView_t   *ssv_p
  )
{
  ScreenAttr_t  *scra_p;                  /* Screen Attritbutes */
  RxnView_t     *rxnv_p;                  /* Reaction Viewer */
  XmString       lbl_str;
  XmString       title;
  Widget         formdg[2];
  Widget         rxn_frame;
  U8_t           gensg_i;
int i,j;
#ifdef _CYGHAT_
  Widget box[2];
Arg al[50];
int ac;
#endif

  scra_p = SynAppR_ScrnAtrb_Get (&GSynAppR);

  /* Create the dialog form, reaction view, scrolled text window, label 
     and push buttons.  */
  title = XmStringCreateLtoR (SSV_TITLE, XmFONTLIST_DEFAULT_TAG);
  formdg[0] = XmCreateFormDialog (parent, "SShotV", NULL, 0);
  XtVaSetValues (formdg[0],
    XmNdialogStyle,     XmDIALOG_MODELESS, 
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNbuttonFontList,  SynAppR_FontList_Get (&GSynAppR),
    XmNtextFontList,    SynAppR_FontList_Get (&GSynAppR),
    XmNresizePolicy,    XmRESIZE_NONE,
    XmNresizable,       True,
    XmNautoUnmanage,    False, 
    XmNdialogTitle,     title, 
    XmNdefaultPosition, False,
#ifdef _CYGHAT_
    XmNheight,          17 * AppDim_AppHeight_Get (&GAppDim)/ 20,
    XmNwidth,           19 * AppDim_AppWidth_Get (&GAppDim) / 20,
    XmNx,               Screen_Width_Get (scra_p)
                          - AppDim_AppWidth_Get (&GAppDim) - SSV_RXNMOL_BRDR,
    XmNy,               25,
#else
    XmNheight,          AppDim_AppHeight_Get (&GAppDim),
    XmNwidth,           AppDim_AppWidth_Get (&GAppDim),
    XmNx,               Screen_Width_Get (scra_p)
                          - AppDim_AppWidth_Get (&GAppDim) - SSV_RXNMOL_BRDR,
    XmNy,               0,
#endif
    NULL);
  XmStringFree (title);
  SShotV_FormDlg_Put (ssv_p, 0, formdg[0]);

  title = XmStringCreateLtoR ("Selected PostTransform Test", XmFONTLIST_DEFAULT_TAG);
  formdg[1] = XmCreateFormDialog (parent, "SShotV", NULL, 0);
  XtVaSetValues (formdg[1],
    XmNdialogStyle,     XmDIALOG_MODELESS, 
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNbuttonFontList,  SynAppR_FontList_Get (&GSynAppR),
    XmNtextFontList,    SynAppR_FontList_Get (&GSynAppR),
    XmNresizePolicy,    XmRESIZE_NONE,
    XmNresizable,       True,
    XmNautoUnmanage,    False, 
    XmNdialogTitle,     title, 
    XmNdefaultPosition, False,
#ifdef _CYGHAT_
    XmNheight,          17 * AppDim_AppHeight_Get (&GAppDim) / 20,
    XmNwidth,           AppDim_AppWidth_Get (&GAppDim) / 2,
    XmNx,               Screen_Width_Get (scra_p)
                          - AppDim_AppWidth_Get (&GAppDim) / 2 - SSV_RXNMOL_BRDR,
    XmNy,               25,
#else
    XmNheight,          AppDim_AppHeight_Get (&GAppDim),
    XmNwidth,           AppDim_AppWidth_Get (&GAppDim) / 2,
    XmNx,               Screen_Width_Get (scra_p)
                          - AppDim_AppWidth_Get (&GAppDim) / 2 - SSV_RXNMOL_BRDR,
    XmNy,               0,
#endif
    NULL);
  XmStringFree (title);
  SShotV_FormDlg_Put (ssv_p, 1, formdg[1]);

  /*  Create a reaction viewer, but unmanage and destroy the subgoal 
      merit labels.
  */
  rxnv_p = SShotV_RxnView_Get (ssv_p);
  rxn_frame = RxnView_Create (formdg[0], rxnv_p);  

  XtVaSetValues (RxnView_MolForm_Get (rxnv_p),
    XmNheight, (AppDim_AppWidth_Get (&GAppDim) >> 1),
    XmNwidth, AppDim_AppWidth_Get (&GAppDim) - SSV_DLGSHELL_OFFX,
    NULL);
  RxnView_MolFormW_Put (rxnv_p, AppDim_AppWidth_Get (&GAppDim)
    - SSV_DLGSHELL_OFFX);
  RxnView_MolDAH_Put (rxnv_p, (AppDim_AppWidth_Get (&GAppDim) >> 1)
    - RxnV_IthM_LblsH_Get (rxnv_p, 0));

  /*  Try just setting these unused labels to blanks instead of 
      destroying them.
  XtUnmanageChild (RxnView_SolvedLbl_Get (rxnv_p));
  XtUnmanageChild (RxnView_SGMeritVal_Get (rxnv_p));
  XtUnmanageChild (RxnView_SGMeritLbl_Get (rxnv_p));
  XtDestroyWidget (RxnView_SGMeritVal_Get (rxnv_p));
  XtDestroyWidget (RxnView_SGMeritLbl_Get (rxnv_p));
  XtDestroyWidget (RxnView_SolvedLbl_Get (rxnv_p));
  */
  lbl_str = XmStringCreateLtoR (" ", AppDim_RxnValTag_Get (&GAppDim));
  XtVaSetValues (RxnView_SolvedLbl_Get (rxnv_p),
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    NULL);
  XtVaSetValues (RxnView_SGMeritVal_Get (rxnv_p),
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    NULL);
  XmStringFree (lbl_str);

  XtVaSetValues (RxnView_NameLbl_Get (rxnv_p),
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_MENUBG),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    NULL);

  lbl_str = XmStringCreateLtoR (" ", AppDim_RxnLblTag_Get (&GAppDim));
  XtVaSetValues (RxnView_SGMeritLbl_Get (rxnv_p),
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    NULL);
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLtoR (SSV_LABEL_TESTS, 
    AppDim_PstLblTag_Get (&GAppDim));
  SShotV_TestLbl_Put (ssv_p, XtVaCreateWidget ("SShotVPTL", 
    xmLabelWidgetClass,  formdg[0], 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNalignment, XmALIGNMENT_CENTER,
    XmNrecomputeSize, False,
    XmNmarginHeight, 0,
    XmNmarginWidth, 0,
    XmNheight, AppDim_RxnNameHt_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  *SShotV_TextBuf_Get (ssv_p, 0) = '\0';
  *SShotV_TextBuf_Get (ssv_p, 1) = '\0';
#ifdef _CYGHAT_
ac=0;
XtSetArg(al[ac],
    XmNscrollingPolicy, XmAUTOMATIC); ac++;
XtSetArg(al[ac],
    XmNscrollBarDisplayPolicy, XmAS_NEEDED); ac++;
XtSetArg(al[ac],
    XmNscrollVertical, True); ac++;
XtSetArg(al[ac],
    XmNeditMode, XmMULTI_LINE_EDIT); ac++;
XtSetArg(al[ac],
    XmNeditable, False); ac++;
XtSetArg(al[ac],
    XmNautoShowCursorPosition, False); ac++;
XtSetArg(al[ac],
    XmNcursorPositionVisible, False); ac++;
XtSetArg(al[ac],
    XmNvalue, SShotV_TextBuf_Get (ssv_p, 0)); ac++;
XtSetArg(al[ac],
    XmNmarginHeight, 0); ac++;
XtSetArg(al[ac],
    XmNmarginWidth, 0); ac++;
XtSetArg(al[ac],
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE)); ac++;
XtSetArg(al[ac],
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK)); ac++;

  SShotV_ScrlText_Put (ssv_p, 0, XmCreateScrolledText (formdg[0], "SShotVST", 
    al, ac));
  XtAddCallback (SShotV_ScrlText_Get (ssv_p, 0), XmNmotionVerifyCallback, 
    SShotView_ScrlText_CB, (XtPointer) ssv_p);

XtSetArg(al[7],
    XmNvalue, SShotV_TextBuf_Get (ssv_p, 1));

  SShotV_ScrlText_Put (ssv_p, 1, XmCreateScrolledText (formdg[1], "SSPTVST", 
    al, ac));
#else
  SShotV_ScrlText_Put (ssv_p, 0, XmCreateScrolledText (formdg[0], "SShotVST", 
    NULL, 0));
  XtVaSetValues (SShotV_ScrlText_Get (ssv_p, 0),
    XmNscrollingPolicy, XmAUTOMATIC,
    XmNscrollBarDisplayPolicy, XmAS_NEEDED,
#ifdef _CYGHAT_
        XmNscrollVertical, True,
#endif
    XmNeditMode, XmMULTI_LINE_EDIT,
    XmNeditable, False,
    XmNautoShowCursorPosition, False,
    XmNcursorPositionVisible, False,
    XmNvalue, SShotV_TextBuf_Get (ssv_p, 0),
    XmNmarginHeight, 0,
    XmNmarginWidth, 0,
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
     NULL);
  XtAddCallback (SShotV_ScrlText_Get (ssv_p, 0), XmNmotionVerifyCallback, 
    SShotView_ScrlText_CB, (XtPointer) ssv_p);

  SShotV_ScrlText_Put (ssv_p, 1, XmCreateScrolledText (formdg[1], "SSPTVST", 
    NULL, 0));
  XtVaSetValues (SShotV_ScrlText_Get (ssv_p, 1),
    XmNscrollingPolicy, XmAUTOMATIC,
    XmNscrollBarDisplayPolicy, XmAS_NEEDED,
#ifdef _CYGHAT_
        XmNscrollVertical, True,
#endif
    XmNeditMode, XmMULTI_LINE_EDIT,
    XmNeditable, False,
    XmNautoShowCursorPosition, False,
    XmNcursorPositionVisible, False,
    XmNvalue, SShotV_TextBuf_Get (ssv_p, 1),
    XmNmarginHeight, 0,
    XmNmarginWidth, 0,
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
     NULL);
#endif

  lbl_str = XmStringCreateLtoR (SSV_PUSHB_PREVIOUS, XmFONTLIST_DEFAULT_TAG);
  SShotV_PrevPB_Put (ssv_p, XtVaCreateWidget ("SShotVPPB", 
    xmPushButtonWidgetClass,  formdg[0], 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNshowAsDefault, False,
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_MENUBG),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
      NULL));
  XmStringFree (lbl_str);
  XtAddCallback (SShotV_PrevPB_Get (ssv_p), XmNactivateCallback, 
    SShotView_PrevPB_CB, (XtPointer) ssv_p);

  lbl_str = XmStringCreateLtoR (SSV_PUSHB_DISMISS, XmFONTLIST_DEFAULT_TAG);
  SShotV_DismissPB_Put (ssv_p, 0, XtVaCreateWidget ("SShotVDPB", 
    xmPushButtonWidgetClass,  formdg[0], 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNshowAsDefault, False,
      NULL));
  XtAddCallback (SShotV_DismissPB_Get (ssv_p, 0), XmNactivateCallback, 
    SShotView_DismissPB_CB, (XtPointer) ssv_p);

  SShotV_DismissPB_Put (ssv_p, 1, XtVaCreateWidget ("SSPTVDPB", 
    xmPushButtonWidgetClass,  formdg[1], 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNshowAsDefault, True,
      NULL));
  XmStringFree (lbl_str);
  XtAddCallback (SShotV_DismissPB_Get (ssv_p, 1), XmNactivateCallback, 
    SShotView_DismissPTPB_CB, (XtPointer) ssv_p);

  lbl_str = XmStringCreateLtoR (SSV_PUSHB_NUMON, XmFONTLIST_DEFAULT_TAG);
  SShotV_NumToglPB_Put (ssv_p, XtVaCreateWidget ("SShotVNTPB", 
    xmPushButtonWidgetClass,  formdg[0], 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNshowAsDefault, False,
      NULL));
  XtAddCallback (SShotV_NumToglPB_Get (ssv_p), XmNactivateCallback, 
    SShotView_NumToglPB_CB, (XtPointer) ssv_p);

  lbl_str = XmStringCreateLtoR (SSV_PUSHB_NEXT, XmFONTLIST_DEFAULT_TAG);
  SShotV_NextPB_Put (ssv_p, XtVaCreateWidget ("SShotVNPB", 
    xmPushButtonWidgetClass, formdg[0],
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNshowAsDefault, False,
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_MENUBG),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    NULL));
  XmStringFree (lbl_str);
  XtAddCallback (SShotV_NextPB_Get (ssv_p), XmNactivateCallback, 
    SShotView_NextPB_CB, (XtPointer) ssv_p);

#ifdef _CYGHAT_
  for (i=0; i<2; i++)
  {
    box[i]=XtVaCreateManagedWidget ("box",
      xmRowColumnWidgetClass, formdg[i],
      XmNleftAttachment,      XmATTACH_FORM,
      XmNrightAttachment,     XmATTACH_FORM,
      XmNbottomAttachment,    XmATTACH_WIDGET,
      XmNbottomWidget,        SShotV_DismissPB_Get (ssv_p, i),
      XmNorientation,         XmHORIZONTAL,
      XmNheight,              1,
      NULL);
  }
#endif

  /*  Set the proper attachments.  */


  XtVaSetValues (SShotV_DismissPB_Get (ssv_p, 0),
    XmNtopAttachment, XmATTACH_NONE,
    XmNbottomOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftPosition, 27,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 47,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  XtVaSetValues (SShotV_DismissPB_Get (ssv_p, 1),
    XmNtopAttachment, XmATTACH_NONE,
    XmNbottomOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftPosition, 40,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 60,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  XtVaSetValues (SShotV_NumToglPB_Get (ssv_p),
    XmNtopAttachment, XmATTACH_NONE,
    XmNbottomOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftPosition, 53,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 73,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  XtVaSetValues (SShotV_PrevPB_Get (ssv_p),
    XmNtopOffset, 0,
    XmNtopWidget, SShotV_DismissPB_Get (ssv_p, 0),
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomOffset, 0,
    XmNbottomWidget, SShotV_DismissPB_Get (ssv_p, 0),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNleftOffset, AppDim_SepLarge_Get (&GAppDim),
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightPosition, 20,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  XtVaSetValues (SShotV_NextPB_Get (ssv_p),
    XmNtopOffset, 0,
    XmNtopWidget, SShotV_DismissPB_Get (ssv_p, 0),
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomOffset, 0,
    XmNbottomWidget, SShotV_DismissPB_Get (ssv_p, 0),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNleftPosition, 80,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightOffset, AppDim_SepLarge_Get (&GAppDim),
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  XtVaSetValues (XtParent (SShotV_ScrlText_Get (ssv_p, 0)),
    XmNtopOffset, 0,
    XmNtopWidget, SShotV_TestLbl_Get (ssv_p),
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNbottomOffset, AppDim_SepLarge_Get (&GAppDim),
#ifdef _CYGHAT_
    XmNbottomWidget, box[0],
    XmNbottomAttachment, XmATTACH_WIDGET,
#else
    XmNbottomWidget, SShotV_DismissPB_Get (ssv_p, 0),
    XmNbottomAttachment, XmATTACH_WIDGET,
#endif
    XmNleftOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  XtVaSetValues (XtParent (SShotV_ScrlText_Get (ssv_p, 1)),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomOffset, AppDim_SepLarge_Get (&GAppDim),
#ifdef _CYGHAT_
    XmNbottomWidget, box[1],
    XmNbottomAttachment, XmATTACH_WIDGET,
#else
    XmNbottomWidget, SShotV_DismissPB_Get (ssv_p, 1),
    XmNbottomAttachment, XmATTACH_WIDGET,
#endif
    XmNleftOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  XtVaSetValues (SShotV_TestLbl_Get (ssv_p),
    XmNtopOffset, 0,
    XmNtopWidget, rxn_frame,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  XtVaSetValues (rxn_frame,
    XmNtopOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, AppDim_SepSmall_Get (&GAppDim),
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  XtManageChild (rxn_frame);
  XtManageChild (SShotV_DismissPB_Get (ssv_p, 0));
  XtManageChild (SShotV_DismissPB_Get (ssv_p, 1));
  XtManageChild (SShotV_PrevPB_Get (ssv_p));
  XtManageChild (SShotV_NumToglPB_Get (ssv_p));
  XtManageChild (SShotV_NextPB_Get (ssv_p));
  XtManageChild (SShotV_ScrlText_Get (ssv_p, 0));
  XtManageChild (SShotV_ScrlText_Get (ssv_p, 1));
  XtManageChild (SShotV_TestLbl_Get (ssv_p));

  for (gensg_i = 0; gensg_i < SSV_GENSG_MAXNUM; gensg_i++)
    SShotGenSG_IthInfo_Put (SShotV_GenSGs_Get (ssv_p), gensg_i, NULL);

  SShotGenSG_NumInfos_Put (SShotV_GenSGs_Get (ssv_p), 0);
  SShotGenSG_PreTrans_Put (SShotV_GenSGs_Get (ssv_p), TRUE);
  SShotV_CurrGenSG_Put (ssv_p, 0);
  SShotV_RxnRec_Put (ssv_p, NULL);
  SShotV_TgtXtr_Put (ssv_p, NULL);
  SShotV_IsCreated_Put (ssv_p, TRUE);

  return ;
}
/*  End of SShotView_Create  */


/****************************************************************************
*
*  Function Name:                 SShotView_Reset
*
*    This routine resets the SingleShot Viewing Dialog by destroying
*    the generated subgoal information structures used to display the
*    information.
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
void SShotView_Reset
  (
  SShotView_t   *ssv_p
  )
{
  SShotGenSGs_t *gensgs_p;
  U8_t           gensg_i;
  int            i, j;

  gensgs_p = SShotV_GenSGs_Get (ssv_p);

  for (gensg_i = 0; gensg_i < SShotGenSG_NumInfos_Get (gensgs_p); gensg_i++)
    {
    SShotInfo_Destroy (SShotGenSG_IthInfo_Get (gensgs_p, gensg_i));
    SShotGenSG_IthInfo_Put (gensgs_p, gensg_i, NULL);
    }

  SShotGenSG_NumInfos_Put (gensgs_p, 0);
  SShotGenSG_PreTrans_Put (gensgs_p, TRUE);
  SShotV_CurrGenSG_Put (ssv_p, 0);
  SShotV_RxnRec_Put (ssv_p, NULL);
  Xtr_Destroy (SShotV_TgtXtr_Get (ssv_p));
  SShotV_TgtXtr_Put (ssv_p, NULL);

  *SShotV_TextBuf_Get (ssv_p, 0) = '\0';
  new_view = TRUE;

  for (i = 0; i < ncond; i++)
    {
    for (j = 0; j < 3; j++)
      free (condv[0][i][j]);
    free (condv[0][i]);
    }
  if (ncond != 0) free (condv[0]);
  ncond = 0;

  for (i = 0; i < ntest; i++)
    {
    for (j = 0; j < 5; j++)
      free (testv[0][i][j]);
    free (testv[0][i]);
    }
  if (ntest != 0) free (testv[0]);
  ntest = 0;

  return ;
}
/*  End of SShotView_Reset  */


/****************************************************************************
*
*  Function Name:                 SShotView_Setup
*
*    This routine creates the target molecule, and fills in the reaction
*    name, library, chapter and schema.
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
*    N/A
*
******************************************************************************/
void SShotView_Setup
  (
  SShotView_t    *ssv_p
  )
{
  char           **chap_names;
  RxnView_t       *rxnv_p;
  React_Record_t  *rxn_rec_p;
  React_Head_t    *rxn_head_p;
  React_TextRec_t *txt_rec_p;
  char            *rxn_name;
  XmString         val_lbl;
  Dimension        mlw, mlh;
  char             val_buf[RVW_VAL_BUFLEN]; 
  U8_t             chap_num;
  U8_t             mol_i;
  Boolean_t        node_used[100];
  int              i, num_atoms;

  if (ssv_p == NULL)
    return;

  for (i = 0; i < 100; i++) node_used[i] = FALSE;

  ptread_no_init (React_Head_SchemaId_Get (React_Head_Get (SShotV_RxnRec_Get (ssv_p))) + 1,
    &ncond, &cond, &ntest, &test, &reas, &chem);
  ptwrite (ncond, cond, condv, ntest, test, testv, reas, chem, 25, node_used);

  rxnv_p = SShotV_RxnView_Get (ssv_p);
  rxn_rec_p = SShotV_RxnRec_Get (ssv_p);
  rxn_head_p = React_Head_Get (rxn_rec_p);
  txt_rec_p = React_Text_Get (rxn_rec_p);
  chap_names = RxnView_ChapNames_Grab ();

  /*  Update reaction name label.  */
  rxn_name = (char *) String_Value_Get (React_TxtRec_Name_Get (txt_rec_p));
  val_lbl = XmStringCreateLtoR (rxn_name, AppDim_RxnNamTag_Get (&GAppDim));
  XtVaSetValues (RxnView_NameLbl_Get (rxnv_p),
    XmNlabelString, val_lbl, 
    NULL);
  XmStringFree (val_lbl);

  /*  Update library name label.  */
  if (React_Head_Library_Get (rxn_head_p) == BAS_LIB_NUM)
    val_lbl = XmStringCreateLtoR ("bas", AppDim_RxnValTag_Get (&GAppDim));
  else if (React_Head_Library_Get (rxn_head_p) == DEV_LIB_NUM)
    val_lbl = XmStringCreateLtoR ("dev", AppDim_RxnValTag_Get (&GAppDim));
  else
    val_lbl = XmStringCreateLtoR ("n/a", AppDim_RxnValTag_Get (&GAppDim));

  XtVaSetValues (RxnView_LibVal_Get (rxnv_p),
    XmNlabelString, val_lbl, 
    NULL);
  XmStringFree (val_lbl);

  /*  Update chapter name label.  */
  chap_num = React_Head_SynthemeFG_Get (rxn_head_p);
  if (chap_num > 0 && chap_num < RVW_NUM_CHAPTERS)
    val_lbl = XmStringCreateLtoR (chap_names[chap_num], 
      AppDim_RxnValTag_Get (&GAppDim));
  else
    val_lbl = XmStringCreateLtoR ("Unknown", AppDim_RxnValTag_Get (&GAppDim));

  XtVaSetValues (RxnView_ChapVal_Get (rxnv_p),
    XmNlabelString, val_lbl, 
    NULL);
  XmStringFree (val_lbl);

  /*  Update schema number label.  */
  sprintf (val_buf, "%hu", React_TxtHd_OrigSchema_Get (
    React_TxtRec_Head_Get (txt_rec_p)));
  val_lbl = XmStringCreateLtoR (val_buf, AppDim_RxnValTag_Get (&GAppDim));
  XtVaSetValues (RxnView_SchemaVal_Get (rxnv_p),
    XmNlabelString, val_lbl, 
    NULL);
  XmStringFree (val_lbl);

  /*  Store target compound molecule and set up label.  */
  if (RxnV_IthM_Mol_Get (rxnv_p, 0) != NULL)
    free_Molecule (RxnV_IthM_Mol_Get (rxnv_p, 0));
 
  RxnV_IthM_Mol_Put (rxnv_p, 0, Xtr2Dsp (SShotV_TgtXtr_Get (ssv_p)));
  if (!dsp_Shelley (RxnV_IthM_Mol_Get (rxnv_p, 0)))
    {
    fprintf (stderr, 
      "\nSShot_View_Update:  unable to calculate coords for product molecule.\n");
    //free_Molecule (RxnV_IthM_Mol_Get (rxnv_p, 0));//kka
    //RxnV_IthM_Mol_Put (rxnv_p, 0, NULL);//kka
    //return;//kka
    }
  else
    SShotView_Add_Pattern_Nums (RxnV_IthM_Mol_Get (rxnv_p, 0), SShotGenSG_IthInfo_Get (SShotV_GenSGs_Get (ssv_p), 0), 0);

  val_lbl = XmStringCreateLtoR (SSV_MOL_MRT, AppDim_RxnValTag_Get (&GAppDim));
  XtVaSetValues (RxnV_IthM_MrtLbl_Get (rxnv_p, 0),
    XmNlabelString, val_lbl, 
    NULL);
  XmStringFree (val_lbl);

  val_lbl = XmStringCreateLtoR (SSV_MOL_TRGT, AppDim_RxnValTag_Get (&GAppDim));
  XtVaSetValues (RxnV_IthM_NameLbl_Get (rxnv_p, 0),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNlabelString, val_lbl, 
    NULL);
  XmStringExtent (SynAppR_FontList_Get (&GSynAppR), val_lbl, &mlw, &mlh);
  XmStringFree (val_lbl);
  mlw += 2 * AppDim_MargLblPB_Get (&GAppDim);
  RxnV_IthM_LblW_Put (rxnv_p, 0, mlw + 3 * AppDim_MargLblPB_Get (&GAppDim));
  RxnV_IthM_DAW_Put (rxnv_p, 0, RxnView_MolDAH_Get (rxnv_p));

  if (targ_x != NULL) free (targ_x);
  if (targ_y != NULL) free (targ_y);
  num_atoms = RxnV_IthM_Mol_Get (rxnv_p, 0)->natms;
  targ_x = (Dimension *) malloc (num_atoms * sizeof (Dimension));
  targ_y = (Dimension *) malloc (num_atoms * sizeof (Dimension));
  for (i=0; i<num_atoms; i++)
    {
    targ_x[i] = RxnV_IthM_Mol_Get (rxnv_p, 0)->atoms[i].x;
    targ_y[i] = RxnV_IthM_Mol_Get (rxnv_p, 0)->atoms[i].y;
    }
  Mol_Scale (RxnV_IthM_Mol_Get (rxnv_p, 0), RxnView_MolDAH_Get (rxnv_p),
    RxnV_IthM_DAW_Get (rxnv_p, 0));

  XtVaSetValues (RxnV_IthM_Form_Get (rxnv_p, 0),
    XmNwidth, RxnV_IthM_DAW_Get (rxnv_p, 0),
    XmNleftOffset, 0,
    NULL);

  /*  Set up labels for precursors.  */
  for (mol_i = 1; mol_i < RVW_MAXNUM_MOLS; mol_i++)
    {
    val_lbl = XmStringCreateLtoR (SSV_MOL_MRT, AppDim_RxnValTag_Get (&GAppDim));
    XtVaSetValues (RxnV_IthM_MrtLbl_Get (rxnv_p, mol_i),
      XmNlabelString, val_lbl, 
      NULL);
    XmStringFree (val_lbl);

    sprintf (val_buf, "%s%1hu", SSV_MOL_PREC, mol_i);
    val_lbl = XmStringCreateLtoR (val_buf, AppDim_RxnValTag_Get (&GAppDim));
    XtVaSetValues (RxnV_IthM_NameLbl_Get (rxnv_p, mol_i),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
      XmNlabelString, val_lbl, 
      NULL);
    XmStringExtent (SynAppR_FontList_Get (&GSynAppR), val_lbl, &mlw, &mlh);
    XmStringFree (val_lbl);
    mlw += 2 * AppDim_MargLblPB_Get (&GAppDim);
    RxnV_IthM_LblW_Put (rxnv_p, mol_i, 
      mlw + 3 * AppDim_MargLblPB_Get (&GAppDim));
    }

  XtManageChild (RxnV_IthM_Form_Get (rxnv_p, 0));

  return ;
}
/*  End of SShotView_Setup  */


/****************************************************************************
*
*  Function Name:                 SShotView_Update
*
*      
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
*    N/A
*
******************************************************************************/
void SShotView_Update
  (
  SShotView_t   *ssv_p, 
  U8_t           curr_sg
  )
{
  RxnView_t       *rxnv_p;
  React_Head_t    *rxn_head_p;
  React_TextRec_t *txt_rec_p;
  Posttest_t      *post_p;
  Xtr_t           *xtr_p;
  SShotInfo_t     *ssi_p;
  Dsp_Molecule_t  *mol_p;
  Dsp_Atom_t      *atom_p;
  char            *tbuf_p;
  char            *tverb_p;
  XmString         val_lbl;
  Dimension        x_off, y_off;
  Dimension        x_pos;
  U16_t            atm_i;
  S16_t            ease_adj_p;
  S16_t            yield_adj_p;
  S16_t            conf_adj_p;
  S16_t            merit_ease_adj_p;
  S16_t            merit_yield_adj_p;
  S16_t            merit_conf_adj_p;
  S16_t            kb_param;
  char             val_buf[RVW_VAL_BUFLEN]; 
  U8_t             test_i;
  U8_t             mol_i;
  Boolean_t        passed;

  if (ssv_p == NULL)
    return;

#ifdef _DEBUG_
printf("SSV_U entered\n");
#endif
  /*  Store the current generated subgoal number, set up the previous 
      and next buttons, unmanage the generated compounds from the previous
      subgoal and check if the target molecule passed the pretransform
      tests.  If so, get the current generated subgoal information
      record.  If by chance it is NULL, return with only the target 
      displayed.
  */
  tbuf_p = SShotV_TextBuf_Get (ssv_p, 0);
  new_view = TRUE;
  rxnv_p = SShotV_RxnView_Get (ssv_p);
  SShotV_CurrGenSG_Put (ssv_p, curr_sg);

  if (SShotV_CurrGenSG_Get (ssv_p) > 0)
    XtManageChild (SShotV_PrevPB_Get (ssv_p));
  else
    XtUnmanageChild (SShotV_PrevPB_Get (ssv_p));

/*
  if (SShotV_CurrGenSG_Get (ssv_p) == 
*/
/* above logic ignores the case of 0 subgoals */
  if (SShotV_CurrGenSG_Get (ssv_p) >= 
      SShotGenSG_NumInfos_Get (SShotV_GenSGs_Get (ssv_p)) - 1)
    XtUnmanageChild (SShotV_NextPB_Get (ssv_p));
  else
    XtManageChild (SShotV_NextPB_Get (ssv_p));


  for (mol_i = 0; mol_i < RxnView_NumSGs_Get (rxnv_p); mol_i++)
    {
    XtUnmanageChild (RxnV_IthM_Sign_Get (rxnv_p, mol_i));
    XtUnmanageChild (RxnV_IthM_DA_Get (rxnv_p, mol_i + 1));
    XtUnmanageChild (RxnV_IthM_Form_Get (rxnv_p, mol_i + 1));
    }

  /*  If the target failed the pretransformed tests, print the 
      appropriate message in the text window and return.  
  */
  if (SShotGenSG_PreTrans_Get (SShotV_GenSGs_Get (ssv_p)) == FALSE)
    {
    sprintf (tbuf_p, "%s\n\n%s\n", SSV_WARN_PRET_FAIL, SShotGenSG_PreTBuff_Get (SShotV_GenSGs_Get (ssv_p)));
    XmTextSetString (SShotV_ScrlText_Get (ssv_p, 0), SShotV_TextBuf_Get (ssv_p, 0));

    /*  Update ease, yield and confidence labels with fail notice.  */
    val_lbl = XmStringCreateLtoR (SSV_PARVAL_FAIL, 
      AppDim_RxnValTag_Get (&GAppDim));
    XtVaSetValues (RxnView_EaseVal_Get (rxnv_p),
      XmNlabelString, val_lbl, 
      NULL);
    XtVaSetValues (RxnView_YieldVal_Get (rxnv_p),
      XmNlabelString, val_lbl, 
      NULL);
    XtVaSetValues (RxnView_ConfVal_Get (rxnv_p),
      XmNlabelString, val_lbl, 
      NULL);
    XmStringFree (val_lbl);
    return;
    }

  ssi_p = SShotGenSG_IthInfo_Get (SShotV_GenSGs_Get (ssv_p), curr_sg);
  if (ssi_p == NULL)
    {
     /*  Print error message in posttran test window.  */
    sprintf (tbuf_p, "%s\n", SSV_WARN_NO_INFO);
    XmTextSetString (SShotV_ScrlText_Get (ssv_p, 0), SShotV_TextBuf_Get (ssv_p, 0));

     /*  Clear ease, yield and confidence labels.  */
    val_lbl = XmStringCreateLtoR (RVW_VALUE_PARAM, 
      AppDim_RxnValTag_Get (&GAppDim));
    XtVaSetValues (RxnView_EaseVal_Get (rxnv_p),
      XmNlabelString, val_lbl, 
      NULL);
    XtVaSetValues (RxnView_YieldVal_Get (rxnv_p),
      XmNlabelString, val_lbl, 
      NULL);
    XtVaSetValues (RxnView_ConfVal_Get (rxnv_p),
      XmNlabelString, val_lbl, 
      NULL);
    XmStringFree (val_lbl);
    return;
    }

  /*  If one of the generated compounds failed the CompOk tests, print the 
      appropriate message in the text window and return.  
  */
  if (SShotInfo_CompOk_Get (ssi_p) == FALSE)
    {
    sprintf (tbuf_p, "%s\n", SSV_WARN_INVALID);
    XmTextSetString (SShotV_ScrlText_Get (ssv_p, 0), SShotV_TextBuf_Get (ssv_p, 0));

    /*  Update ease, yield and confidence labels with fail notice.  */
    val_lbl = XmStringCreateLtoR (SSV_PARVAL_FAIL, 
      AppDim_RxnValTag_Get (&GAppDim));
    XtVaSetValues (RxnView_EaseVal_Get (rxnv_p),
      XmNlabelString, val_lbl, 
      NULL);
    XtVaSetValues (RxnView_YieldVal_Get (rxnv_p),
      XmNlabelString, val_lbl, 
      NULL);
    XtVaSetValues (RxnView_ConfVal_Get (rxnv_p),
      XmNlabelString, val_lbl, 
      NULL);
    XmStringFree (val_lbl);
/*
Don't return yet - we want to see the rejected molecule.
*/
/*
    return;
*/
    }

  /*  Otherwise, unscale the target molecule, store the generated
      subgoal molecules, and set up the reaction viewer.  
  */
  rxn_head_p = React_Head_Get (SShotV_RxnRec_Get (ssv_p));
  txt_rec_p = React_Text_Get (SShotV_RxnRec_Get (ssv_p));
  RxnView_NumSGs_Put (rxnv_p, SShotInfo_NumMols_Get (ssi_p));

  /*  Unscale the target molecule.  */
  XtUnmanageChild (RxnV_IthM_DA_Get (rxnv_p, 0));
  XtUnmanageChild (RxnV_IthM_Form_Get (rxnv_p, 0));

/*
This does not work (and cannot work) consistently - once you truncate a floating-point result,
you can't get back the exact original value - the result is that the molecule may shrink in
one or both dimensions each time it is reused.
*/
/*
  mol_p = RxnV_IthM_Mol_Get (rxnv_p, 0);
  x_off = ((Dimension) (RxnV_IthM_DAW_Get (rxnv_p, 0) 
    - (mol_p->scale * mol_p->molw) + 0.5)) >> 1;
  y_off = ((Dimension) (RxnView_MolDAH_Get (rxnv_p) 
    - (mol_p->scale * mol_p->molh) + 0.5)) >> 1;
  atom_p = mol_p->atoms;
  for (atm_i = 0; atm_i < mol_p->natms; atm_i++)
    {
    atom_p->x = (int) (((atom_p->x - x_off) / mol_p->scale) + 0.5);
    atom_p->y = (int) (((atom_p->y - y_off) / mol_p->scale) + 0.5);
    atom_p++;
    }
*/
  mol_p = RxnV_IthM_Mol_Get (rxnv_p, 0);
  SShotView_Add_Pattern_Nums (mol_p, SShotGenSG_IthInfo_Get (SShotV_GenSGs_Get (ssv_p), curr_sg), 0);
  atom_p = mol_p->atoms;
  for (atm_i = 0; atm_i < mol_p->natms; atm_i++)
    {
    atom_p->x = targ_x[atm_i];
    atom_p->y = targ_y[atm_i];
    atom_p++;
    }

  mol_p->scale = 1.0;

  /*  Change precursor label, convert slings into molecules and calculate
      new coordinates and positions.  */
  for (mol_i = 1; mol_i < RVW_MAXNUM_MOLS 
      && mol_i <= SShotInfo_NumMols_Get (ssi_p); mol_i++)
    {
    xtr_p = Sling2Xtr (SShotInfo_IthSling_Get (ssi_p, mol_i - 1));
    if (RxnV_IthM_Mol_Get (rxnv_p, mol_i) != NULL)
      free_Molecule (RxnV_IthM_Mol_Get (rxnv_p, mol_i));

    RxnV_IthM_Mol_Put (rxnv_p, mol_i, Xtr2Dsp (xtr_p));
    Xtr_Destroy (xtr_p);
    if (!dsp_Shelley (RxnV_IthM_Mol_Get (rxnv_p, mol_i)))
      {
      fprintf (stderr, 
        "\nSShotView_Update:  unable to calculate coords for %1u mol.\n",
        mol_i);
      //free_Molecule (RxnV_IthM_Mol_Get (rxnv_p, mol_i));//kka
      //RxnV_IthM_Mol_Put (rxnv_p, mol_i, NULL);//kka
      }
    else
      SShotView_Add_Pattern_Nums (RxnV_IthM_Mol_Get (rxnv_p, mol_i), SShotGenSG_IthInfo_Get (SShotV_GenSGs_Get (ssv_p), curr_sg),
        mol_i);
    }

  /*  Determine size of drawing areas, scale drawings and display.  */
  RxnView_Mols_Scale (rxnv_p);
  x_pos = 0;

  XtVaSetValues (RxnV_IthM_Form_Get (rxnv_p, 0),
    XmNwidth, RxnV_IthM_DAW_Get (rxnv_p, 0),
    XmNleftOffset, x_pos,
    NULL);
  x_pos += RxnV_IthM_DAW_Get (rxnv_p, 0);

  XtManageChild (RxnV_IthM_DA_Get (rxnv_p, 0));
  XtManageChild (RxnV_IthM_Form_Get (rxnv_p, 0));
  for (mol_i = 0; mol_i < SShotInfo_NumMols_Get (ssi_p); mol_i++)
    {
    XtVaSetValues (RxnV_IthM_Sign_Get (rxnv_p, mol_i),
      XmNleftOffset, x_pos,
      NULL);
    x_pos += RVW_SIGN_WD;

    XtVaSetValues (RxnV_IthM_Form_Get (rxnv_p, mol_i + 1),
      XmNwidth, RxnV_IthM_DAW_Get (rxnv_p, mol_i + 1),
      XmNleftOffset, x_pos,
      NULL);
    x_pos += RxnV_IthM_DAW_Get (rxnv_p, mol_i + 1);

    XtManageChild (RxnV_IthM_Sign_Get (rxnv_p, mol_i));
    XtManageChild (RxnV_IthM_DA_Get (rxnv_p, mol_i + 1));
    XtManageChild (RxnV_IthM_Form_Get (rxnv_p, mol_i + 1));
    }
  
  if (SShotInfo_CompOk_Get (ssi_p) == FALSE) return;

  /*  Display the post-transform test results.  */

  ease_adj_p = 0;
  yield_adj_p = 0;
  conf_adj_p = 0;
  passed = TRUE;
  if (SShotInfo_NumTests_Get (ssi_p) == 0)
    {
    sprintf (tbuf_p, "%s\n", SSV_WARN_NO_TESTS);
    XmTextSetString (SShotV_ScrlText_Get (ssv_p, 0), SShotV_TextBuf_Get (ssv_p, 0));
    }
  else
    {
    for (test_i = 0; test_i < SShotInfo_NumTests_Get (ssi_p) && passed; 
         test_i++)
      {
      post_p = React_Test_Get (SShotV_RxnRec_Get (ssv_p), test_i);
      if (SShotInfo_IthTRslt_Get (ssi_p, test_i) == SSV_PTEST_FALSE)
        {
        sprintf (tbuf_p, "          %s%1hu%s\n               %s\n\n", 
          SSV_TESTNUM, test_i + 1, SSV_TESTMESS_FALSE,
          (char *) String_Value_Get (React_TxtRec_Reason_Get (txt_rec_p, 
          test_i)));
        tbuf_p += strlen (tbuf_p); 
        }

      else if (SShotInfo_IthTRslt_Get (ssi_p, test_i) == SSV_PTEST_PASS)
        {
        sprintf (tbuf_p, "%s%1hu:  %s", SSV_TESTNUM, test_i + 1, 
          SSV_TESTMESS_PASS);
        tbuf_p += strlen (tbuf_p); 
        if (Post_EaseAdj_Get (post_p) != 0)
          {
          ease_adj_p += Post_EaseAdj_Get (post_p); 
          if (Post_EaseAdj_Get (post_p) < 0)
            {
            sprintf (tbuf_p, ", %s%1d", SSV_TESTMESS_EASEDEC, 
              -Post_EaseAdj_Get (post_p));
            tbuf_p += strlen (tbuf_p); 
            }
          else
            {
            sprintf (tbuf_p, ", %s%1d", SSV_TESTMESS_EASEINC, 
              Post_EaseAdj_Get (post_p));
            tbuf_p += strlen (tbuf_p); 
            }
          }

        if (Post_YieldAdj_Get (post_p) != 0)
          {
        yield_adj_p += Post_YieldAdj_Get (post_p); 
          if (Post_YieldAdj_Get (post_p) < 0)
            {
            sprintf (tbuf_p, ", %s%1d", SSV_TESTMESS_YLDDEC, 
              -Post_YieldAdj_Get (post_p));
            tbuf_p += strlen (tbuf_p); 
            }
          else
            {
            sprintf (tbuf_p, ", %s%1d", SSV_TESTMESS_YLDINC, 
              Post_YieldAdj_Get (post_p));
            tbuf_p += strlen (tbuf_p); 
            }
          }

        if (Post_ConfidenceAdj_Get (post_p) != 0)
          {
          conf_adj_p += Post_ConfidenceAdj_Get (post_p); 
          if (Post_ConfidenceAdj_Get (post_p) < 0)
            {
            sprintf (tbuf_p, ", %s%1d", SSV_TESTMESS_CONFDEC, 
              -Post_ConfidenceAdj_Get (post_p));
            tbuf_p += strlen (tbuf_p); 
            }
          else
            {
            sprintf (tbuf_p, ", %s%1d", SSV_TESTMESS_CONFINC, 
              Post_ConfidenceAdj_Get (post_p));
            tbuf_p += strlen (tbuf_p); 
            }
          }

        sprintf (tbuf_p, ".\n     %s\n\n", (char *) 
          String_Value_Get (React_TxtRec_Reason_Get (txt_rec_p, test_i)));
        tbuf_p += strlen (tbuf_p); 
        }

      else if (SShotInfo_IthTRslt_Get (ssi_p, test_i) == SSV_PTEST_FAIL)
        {
        sprintf (tbuf_p, "%s%1hu:  %s\n     %s\n\n", SSV_TESTNUM, 
          test_i + 1, SSV_TESTMESS_FAIL, (char *) 
          String_Value_Get (React_TxtRec_Reason_Get (txt_rec_p, test_i)));
        tbuf_p += strlen (tbuf_p); 
        passed = FALSE;
        }

      else if (SShotInfo_IthTRslt_Get (ssi_p, test_i) == SSV_PTEST_NONE)
        {
        sprintf (tbuf_p, "%s%1hu:  %s\n     %s\n\n", SSV_TESTNUM, 
          test_i + 1, SSV_TESTMESS_NONE, (char *) 
          String_Value_Get (React_TxtRec_Reason_Get (txt_rec_p, test_i)));
        tbuf_p += strlen (tbuf_p); 
        }

      else
        {
        sprintf (tbuf_p, "%s%1hu:  %s%1hu.\n\n", SSV_TESTNUM, test_i
          + 1, SSV_TESTMESS_ERROR, SShotInfo_IthTRslt_Get (ssi_p, test_i));
        tbuf_p += strlen (tbuf_p); 
        }
      }  /* End of for each test */
    }/* End of else more than zero tests */

  if (passed)
    {
    if (SShotInfo_MrtUpdate_Get (ssi_p) == FALSE)
      {
      sprintf (tbuf_p, "%s\n", SSV_WARN_MRTUP_FAIL);
      XmTextSetString (SShotV_ScrlText_Get (ssv_p, 0), 
        SShotV_TextBuf_Get (ssv_p, 0));

      /*  Update ease, yield and confidence labels with fail notice.  */
      val_lbl = XmStringCreateLtoR (SSV_PARVAL_FAIL, 
        AppDim_RxnValTag_Get (&GAppDim));
      XtVaSetValues (RxnView_EaseVal_Get (rxnv_p),
        XmNlabelString, val_lbl, 
        NULL);
      XtVaSetValues (RxnView_YieldVal_Get (rxnv_p),
        XmNlabelString, val_lbl, 
        NULL);
      XtVaSetValues (RxnView_ConfVal_Get (rxnv_p),
        XmNlabelString, val_lbl, 
        NULL);
      XmStringFree (val_lbl);
      return;
      }
    else if (SShotInfo_NumApply_Get (ssi_p) == FALSE)
      {
      sprintf (tbuf_p, "%s\n", SSV_WARN_MAXAPP_FAIL);
      XmTextSetString (SShotV_ScrlText_Get (ssv_p, 0), 
        SShotV_TextBuf_Get (ssv_p, 0));

      /*  Update ease, yield and confidence labels with fail notice.  */
      val_lbl = XmStringCreateLtoR (SSV_PARVAL_FAIL, 
        AppDim_RxnValTag_Get (&GAppDim));
      XtVaSetValues (RxnView_EaseVal_Get (rxnv_p),
        XmNlabelString, val_lbl, 
        NULL);
      XtVaSetValues (RxnView_YieldVal_Get (rxnv_p),
        XmNlabelString, val_lbl, 
        NULL);
      XtVaSetValues (RxnView_ConfVal_Get (rxnv_p),
        XmNlabelString, val_lbl, 
        NULL);
      XmStringFree (val_lbl);
      return;
      }
    else if (SShotInfo_MaxNonid_Get (ssi_p) == FALSE)
      {
      sprintf (tbuf_p, "%s\n", SSV_WARN_MAXNONID_FAIL);
      XmTextSetString (SShotV_ScrlText_Get (ssv_p, 0), 
        SShotV_TextBuf_Get (ssv_p, 0));

      /*  Update ease, yield and confidence labels with fail notice.  */
      val_lbl = XmStringCreateLtoR (SSV_PARVAL_FAIL, 
        AppDim_RxnValTag_Get (&GAppDim));
      XtVaSetValues (RxnView_EaseVal_Get (rxnv_p),
        XmNlabelString, val_lbl, 
        NULL);
      XtVaSetValues (RxnView_YieldVal_Get (rxnv_p),
        XmNlabelString, val_lbl, 
        NULL);
      XtVaSetValues (RxnView_ConfVal_Get (rxnv_p),
        XmNlabelString, val_lbl, 
        NULL);
      XmStringFree (val_lbl);
      return;
      }
    else
      {
      /* Factor in merit adjustments */
      merit_ease_adj_p = SShotInfo_MeritEA_Get (ssi_p);
      merit_yield_adj_p = SShotInfo_MeritYA_Get (ssi_p);
      merit_conf_adj_p = SShotInfo_MeritCA_Get (ssi_p);
      if (merit_ease_adj_p !=0 || merit_yield_adj_p != 0 || merit_conf_adj_p != 0)
        {
        strcpy (tbuf_p, "\n__________________________________________________\n\nMerit adjusted for");
        tbuf_p += strlen (tbuf_p);
        if (merit_ease_adj_p != 0)
          {
          sprintf (tbuf_p, " ease %+d", merit_ease_adj_p);
          tbuf_p += strlen (tbuf_p);
          }
        if (merit_yield_adj_p != 0)
          {
          sprintf (tbuf_p, " yield %+d", merit_yield_adj_p);
          tbuf_p += strlen (tbuf_p);
          }
        if (merit_conf_adj_p != 0)
          {
          sprintf (tbuf_p, " confidence %+d", merit_conf_adj_p);
          tbuf_p += strlen (tbuf_p);
          }
        }

      /* Move cursor past end of last test so click there will trigger callback */
      strcat (tbuf_p, "\n\n\n\n\n\n\n\n\n\n");
      /*  All tests passed, so display results of tests, and adjust
          merit parameters.
      */
      XmTextSetString (SShotV_ScrlText_Get (ssv_p, 0), 
      SShotV_TextBuf_Get (ssv_p, 0));
      /*  Update ease label.  */
      kb_param = (S16_t) React_Head_Ease_Get (rxn_head_p);
      if (ease_adj_p + merit_ease_adj_p == 0)
        sprintf (val_buf, "%d", kb_param);
      else
        sprintf (val_buf, "%d <-- %d", kb_param + ease_adj_p + merit_ease_adj_p, 
          kb_param);

      val_lbl = XmStringCreateLtoR (val_buf, AppDim_RxnValTag_Get (&GAppDim));
      XtVaSetValues (RxnView_EaseVal_Get (rxnv_p),
        XmNlabelString, val_lbl, 
        NULL);
      XmStringFree (val_lbl);

      /*  Update yield label.  */
      kb_param = (S16_t) React_Head_Yield_Get (rxn_head_p);
      if (yield_adj_p + merit_yield_adj_p == 0)
        sprintf (val_buf, "%d", kb_param);
      else
        sprintf (val_buf, "%d <-- %d", kb_param + yield_adj_p + merit_yield_adj_p, 
          kb_param);

      val_lbl = XmStringCreateLtoR (val_buf, AppDim_RxnValTag_Get (&GAppDim));
      XtVaSetValues (RxnView_YieldVal_Get (rxnv_p),
        XmNlabelString, val_lbl, 
        NULL);
      XmStringFree (val_lbl);

      /*  Update confidence label.  */
      kb_param = (S16_t) React_Head_Confidence_Get (rxn_head_p);
      if (conf_adj_p + merit_conf_adj_p == 0)
        sprintf (val_buf, "%d", kb_param);
      else
        sprintf (val_buf, "%d <-- %d", kb_param + conf_adj_p + merit_conf_adj_p, 
          kb_param);

      val_lbl = XmStringCreateLtoR (val_buf, AppDim_RxnValTag_Get (&GAppDim));
      XtVaSetValues (RxnView_ConfVal_Get (rxnv_p),
        XmNlabelString, val_lbl, 
        NULL);
      XmStringFree (val_lbl);
      }
    }

  else
    {
    /*  Update ease, yield and confidence labels with fail notice.  */
    XmTextSetString (SShotV_ScrlText_Get (ssv_p, 0), SShotV_TextBuf_Get (ssv_p, 0));
    val_lbl = XmStringCreateLtoR (SSV_PARVAL_FAIL, 
      AppDim_RxnValTag_Get (&GAppDim));
    XtVaSetValues (RxnView_EaseVal_Get (rxnv_p),
      XmNlabelString, val_lbl, 
      NULL);
    XtVaSetValues (RxnView_YieldVal_Get (rxnv_p),
      XmNlabelString, val_lbl, 
      NULL);
    XtVaSetValues (RxnView_ConfVal_Get (rxnv_p),
      XmNlabelString, val_lbl, 
      NULL);
    XmStringFree (val_lbl);
    }
#ifdef _DEBUG_
printf("SSV_U exiting\n");
#endif

  return ;
}
/*  End of SShotView_Update  */


/****************************************************************************
*
*  Function Name:                 SShotView_DismissPB_CB
*
*    This routine resets and unmanages (pops down) the singleshot viewer.
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
*    Unmanages singleshot viewer.
*
******************************************************************************/
void SShotView_DismissPB_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p
  )
{
  SShotView_t  *ssv_p;

  ssv_p = (SShotView_t *) client_p;

  if (ssv_p == NULL)
    return;

  SShotView_Reset (ssv_p);
  XtUnmanageChild (SShotV_FormDlg_Get (ssv_p, 0));

  if (XtIsManaged (SShotV_FormDlg_Get (ssv_p, 1)))
    XtUnmanageChild (SShotV_FormDlg_Get (ssv_p, 1));

  return ;
}
/*  End of SShotView_DismissPB_CB  */


/****************************************************************************
*
*  Function Name:                 SShotView_DismissPTPB_CB
*
*    This routine unmanages (pops down) the posttransform viewer.
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
*    Unmanages singleshot viewer.
*
******************************************************************************/
void SShotView_DismissPTPB_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p
  )
{
  SShotView_t  *ssv_p;

  ssv_p = (SShotView_t *) client_p;

  if (ssv_p == NULL)
    return;

  if (XtIsManaged (SShotV_FormDlg_Get (ssv_p, 1)))
    XtUnmanageChild (SShotV_FormDlg_Get (ssv_p, 1));

  return ;
}
/*  End of SShotView_DismissPB_CB  */


/****************************************************************************
*
*  Function Name:                 SShotView_NumToglPB_CB
*
*    This routine resets and updates the singleshot previewer with the
*    same generated subgoal, toggling numbering on or off.
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
void SShotView_NumToglPB_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p
  )
{
  SShotView_t  *ssv_p;
  XmString lbl_str;

  ssv_p = (SShotView_t *) client_p;

  if (ssv_p == NULL)
    return;

  numbering_on = !numbering_on;
  lbl_str = XmStringCreateLtoR (numbering_on ? SSV_PUSHB_NUMOFF : SSV_PUSHB_NUMON, XmFONTLIST_DEFAULT_TAG);
  XtVaSetValues (SShotV_NumToglPB_Get (ssv_p),
    XmNlabelString, lbl_str, 
      NULL);
  XmStringFree (lbl_str);
  SShotView_Update (ssv_p, SShotV_CurrGenSG_Get (ssv_p));

  return ;
}
/*  End of SShotView_NumToglPB_CB  */


/****************************************************************************
*
*  Function Name:                 SShotView_NextPB_CB
*
*    This routine resets and updates the singleshot previewer with the
*    next generated subgoal.
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
void SShotView_NextPB_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p
  )
{
  SShotView_t  *ssv_p;

  ssv_p = (SShotView_t *) client_p;

  if (ssv_p == NULL)
    return;

  SShotView_Update (ssv_p, SShotV_CurrGenSG_Get (ssv_p) + 1);

  return ;
}
/*  End of SShotView_NextPB_CB  */


/****************************************************************************
*
*  Function Name:                 SShotView_PrevPB_CB
*
*    This routine resets and updates the singleshot previewer with the
*    previous generated subgoal.
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
void SShotView_PrevPB_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p
  )
{
  SShotView_t  *ssv_p;

  ssv_p = (SShotView_t *) client_p;

  if (ssv_p == NULL)
    return;

  SShotView_Update (ssv_p, SShotV_CurrGenSG_Get (ssv_p) - 1);

  return ;
}
/*  End of SShotView_PrevPB_CB  */


/****************************************************************************
*
*  Function Name:                 SShotView_ScrlText_CB
*
*    This routine updates the posttransform viewer with the
*    selected test and associated conditions.
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
void SShotView_ScrlText_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p
  )
{
  SShotView_t     *ssv_p;
  XmTextVerifyPtr  cbs;
  char            *text, *cond_p, condname[16], *tbuf_p;
  int              pos, oldpos, testnum, i, j, k;

  ssv_p = (SShotView_t *) client_p;

  if (ssv_p == NULL)
    return;

  if (new_view)
    {
    new_view = FALSE;
    return;
    }

  cbs = (XmTextVerifyPtr) call_p;
  oldpos = cbs->currInsert;
  pos = cbs->newInsert;
  text = SShotV_TextBuf_Get (ssv_p, 0);

  if (strlen(text)==0) return;
  while(pos > 0 && strncmp (text + pos, "Test #", 6) != 0) pos--;
  while(pos < strlen (text) - 7 && strncmp (text + pos, "Test #", 6) != 0) pos++;
  if (strncmp (text + pos, "Test #", 6) != 0) return;
  sscanf (text + pos + 6, "%d", &testnum);
  testnum--;

  tbuf_p = SShotV_TextBuf_Get (ssv_p, 1);
  *tbuf_p = '\0';

  for (i = 0; i < ncond; i++)
    {
    cond_p = condv[0][i][0];
    while (*cond_p == '\t') cond_p++;
    strcpy (condname, cond_p);
    cond_p = strstr (condname, ":");
    if (cond_p != NULL)
      *cond_p = '\0';
    cond_p = strstr (testv[0][testnum][2], condname);
    if (cond_p != NULL && (cond_p[strlen (condname)] < '0' || cond_p[strlen (condname)] > '9'))
      for (j = 0; j < 3; j++)
      {
      strcpy (tbuf_p, condv[0][i][j] + (j == 1 ? 0 : 1));
      if (j != 1)
        for (k = 2; k < strlen (tbuf_p); k++)
        if (tbuf_p[k] == '\t' && tbuf_p[k - 1] == '\n')
        {
        strcpy (tbuf_p + k, tbuf_p + k + 1);
        k++;
        }
      tbuf_p += strlen (tbuf_p);
      }
    }

  for (i = 0; i < 5; i++)
    {
    strcpy (tbuf_p, testv[0][testnum][i] + (i == 1 ? 0 : 1));
    if (i != 1)
      for (j = 2; j < strlen (tbuf_p); j++)
      if (tbuf_p[j] == '\t' && tbuf_p[j - 1] == '\n')
      {
      strcpy (tbuf_p + j, tbuf_p + j + 1);
      k++;
      }
    tbuf_p += strlen (tbuf_p);
    }

  if (XtIsManaged (SShotV_FormDlg_Get (ssv_p, 1)))
    XtUnmanageChild (SShotV_FormDlg_Get (ssv_p, 1));

  XmTextSetString (SShotV_ScrlText_Get (ssv_p, 1), SShotV_TextBuf_Get (ssv_p, 1));

  XtManageChild (SShotV_FormDlg_Get (ssv_p, 1));

  return ;
}
/*  End of SShotView_PrevPB_CB  */


/****************************************************************************
*
*  Function Name:                 SShotView_Add_Pattern_Nums
*
*    This routine is a kludge to shoehorn the pattern node numbers into the
*    member of the dsp_Atom structure that was originally reserved for the
*    charge.
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
void SShotView_Add_Pattern_Nums (Dsp_Molecule_t *dsp, SShotInfo_t *ssi_p, U16_t sgnum)
{
  U32_t **tmaps;
  U16_t   rowcnt, colcnt, i, j, k, true_sgnum;

if (dsp==NULL)
{
printf("Error in dsp\n");
return;
}

  if (!numbering_on)
    {
    for (i = 0; i < dsp->natms; i++) dsp->atoms[i].chg[0] = '\0';
    return;
    }

  for (k=0; k<dsp->natms; k++) dsp->atoms[k].chg[0] = 0;

  true_sgnum = sgnum - 1;
  tmaps = SShotInfo_TargMaps_Get (ssi_p);
  rowcnt = SShotInfo_TMapRowCnt_Get (ssi_p);
  colcnt = SShotInfo_TMapColCnt_Get (ssi_p);
  for (i = 0; i < rowcnt; i++) switch (sgnum)
    {
    case 0:
      for (j = k = 0;
           j < colcnt && k < dsp->natms && SShotInfo_TMapTargAtom (tmaps, i, j) < dsp->natms;
          )
        {
        if (k < SShotInfo_TMapTargAtom (tmaps, i, j)) k++;
        else if (k > SShotInfo_TMapTargAtom (tmaps, i, j)) j++;
        else
          {
          dsp->atoms[k].chg[0] = SShotInfo_TMapPattAtom (tmaps, i, j) + 1;
          j++;
          k++;
          }
        }
      break;
    default:
      for (j = 0; j < colcnt; j++) if (true_sgnum == SShotInfo_TMapSubgNum (tmaps, i, j))
        {
        k = SShotInfo_TMapSubgAtom (tmaps, i, j);
        if (k != 255 && k < dsp->natms) dsp->atoms[k].chg[0] = SShotInfo_TMapPattAtom (tmaps, i, j) + 1;
        }
      break;
    }
}

/*  End of SSHOT_VIEW.C  */
