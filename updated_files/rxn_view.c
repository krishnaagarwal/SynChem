/******************************************************************************
*
*  Copyright (C) 1995, Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     RXN_VIEW.C
*
*    This module defines the routines needed to display the information for 
*    the selected reaction during perusal of the PST.
*    
*
*  Creation Date:
*
*    10-Aug-1995
*
*  Authors:
*
*    Daren Krebsbach
*
*  Routines:
*
*    RxnView_ChapNames_Grab
*    RxnView_Create
*    RxnView_Destroy
*    RxnView_Mols_Scale
*    RxnView_Update
*
*
*  Callback Routines:
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
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <ctype.h>

#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/Text.h>

#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif

#include "synchem.h"
#include "debug.h"
#include "synio.h"

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

#ifndef _H_AVLCOMP_
#include "avlcomp.h"
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

#ifndef _H_INFO_VIEW_
#include "info_view.h"
#endif

#ifndef _H_MOL_VIEW_
#include "mol_view.h"
#endif

#ifndef _H_RXN_VIEW_
#include "rxn_view.h"
#endif

#ifndef _H_EXTERN_
#include "extern.h"
#endif

/* Static Variables */

static char  *chap_names[RVW_NUM_CHAPTERS] =
  {"", "Alcohol", "Carbonyl", "CC double bond", "Ether", "Carboxylic acid",
   "CC triple bond", "Carboxylic ester", "Halogen", "Peroxide", 
   "CN single bond", "CN double bond", "CN triple bond", 
   "NN double/triple bond", "Pyridine", "Carboxylic amide", "Organosulfur",
   "NO single/double bond", "Pyrylium", "Aryne", "Organomagnesium", 
   "Organolithium", "Organophosphorus", "Pyrazole", "Oxazole", "Furan", "", 
   "Hydrocarbons", "Carbocycle (3)", "Carbocycle (4)", "Carbocycle (5)", 
   "Carbocycle (6)", "Carbocycle (7)", "Carbocycle (8)", "Methylenes",
   "O esters", "", "", "", "Carbene/Nitrene", "Aromatic ring"
  };


/* Static Routine Prototypes */



/****************************************************************************
*
*  Function Name:                 RxnView_ChapNames_Grab
*
*    This routine returns a handle to the Array of chapter names.
*
*
*  Implicit Inputs:
*
*    Array of chapter names.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    Array of chapter names.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
char **RxnView_ChapNames_Grab
  (
  void
  )
{
  return (chap_names);
}
/*  End of RxnView_ChapNames_Grab  */




/****************************************************************************
*
*  Function Name:                 RxnView_Create
*
*    This routine creates the widgets needed to display the information
*    about the current reaction.
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
*    Initializes static pointer to application resources.
*
******************************************************************************/
Widget RxnView_Create
  (
  Widget         parent,
  RxnView_t     *rxnv_p
  )
{
  Pixmap         plus;
  Pixmap         dblarw;

  ScreenAttr_t  *scra_p;                  /* Screen Attritbutes */
  XmString       lbl_str;
  Widget         form;
  U8_t           mol_i;
  char          *plus_file;
  char          *dblarw_file;

  plus_file = SAR_DIR_BITMAPS (SAR_PIXMAP_FNAME_PLUS);
  dblarw_file = SAR_DIR_BITMAPS (SAR_PIXMAP_FNAME_DBLARW);

  /*  Upon creation, set the value of the static pointer to the 
      application resources so that call backs have use of it later.
  */
  scra_p = SynAppR_ScrnAtrb_Get (&GSynAppR);

  /* Create the primary form that contains the molecule form, label form, 
     and scroll window.  
  */
  RxnView_Frame_Put (rxnv_p, XtVaCreateWidget ("RxnVFr",
    xmFrameWidgetClass,  parent,
    XmNshadowType, XmSHADOW_IN,
    XmNshadowThickness, 3,
    XmNmarginWidth, AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginHeight, AppDim_MargFmFr_Get (&GAppDim),
    NULL));

  form = XtVaCreateWidget ("RxnVFm", 
    xmFormWidgetClass,  RxnView_Frame_Get (rxnv_p), 
    XmNlabelFontList, SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight, AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth, AppDim_MargFmFr_Get (&GAppDim),
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNresizable, True,
    NULL);
  RxnView_Form_Put (rxnv_p, form);


  /* Create the reaction bulletin board that contains the molecule drawings
     with labels, and signs (arrows and pluses).  
  */
  RxnView_MolForm_Put (rxnv_p, XtVaCreateWidget ("RxnVMolForm", 
    xmFormWidgetClass,  form, 
    XmNlabelFontList, SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight, 0,
    XmNmarginWidth, 0,
    XmNresizable, True,
    XmNresizePolicy, XmRESIZE_NONE,
    XmNrubberPositioning, False,
    XmNallowOverlap, False,
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    NULL));

  plus = XmGetPixmap (XtScreen (RxnView_MolForm_Get (rxnv_p)), 
    plus_file, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK), 
    SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
  if (plus == XmUNSPECIFIED_PIXMAP)
    fprintf (stderr, "RxnView_Create:  couldn't load pixmap %s.\n",
      plus_file);

  dblarw = XmGetPixmap (XtScreen (RxnView_MolForm_Get (rxnv_p)), 
    dblarw_file, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK), 
    SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
  if (dblarw == XmUNSPECIFIED_PIXMAP)
    fprintf (stderr, "RxnView_Create:  couldn't load pixmap %s.\n",
      dblarw_file);

  for (mol_i = 0; mol_i < RVW_MAXNUM_MOLS; mol_i++)
    {
    MolForm_Create (&RxnView_IthMol_Get (rxnv_p, mol_i), 
      RxnView_MolForm_Get (rxnv_p));

    if (mol_i < RVW_MAXNUM_MOLS - 1)
      RxnV_IthM_Sign_Put (rxnv_p, mol_i, XtVaCreateWidget ("RxnVSL", 
      xmLabelWidgetClass,  RxnView_MolForm_Get (rxnv_p), 
      XmNlabelType, XmPIXMAP,
      XmNlabelPixmap, (mol_i > 0) ? plus : dblarw, 
      XmNalignment, XmALIGNMENT_CENTER,
      XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
      XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
      XmNmarginHeight, 0,
      XmNmarginWidth, 0,
      XmNheight, RVW_SIGN_HT,
      XmNwidth, RVW_SIGN_WD,
      XmNrecomputeSize, False,
      XmNleftAttachment, XmATTACH_FORM,
      XmNleftOffset, 0,
      XmNrightAttachment, XmATTACH_NONE,
      XmNrightOffset, 0,
      XmNbottomAttachment, XmATTACH_FORM,
      XmNbottomOffset, 0,
      XmNtopAttachment, XmATTACH_FORM,
      XmNtopOffset, 0,
      NULL));
    else
      RxnV_IthM_Sign_Put (rxnv_p, mol_i, NULL);
    }

  /* Create the labels for the reaction name, reaction library, chapter
     and schema number, and the easy, yield, confidence and subgoal merit
     values.  
  */
  lbl_str = XmStringCreateLtoR (RVW_REACTION_NONE, 
    AppDim_RxnNamTag_Get (&GAppDim));
  RxnView_NameLbl_Put (rxnv_p, XtVaCreateWidget ("RxnVNameL", 
    xmLabelWidgetClass, form, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNalignment, XmALIGNMENT_BEGINNING,
    XmNheight, AppDim_RxnNameHt_Get (&GAppDim),
    NULL));
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLtoR (RVW_LIB_LBL, AppDim_RxnLblTag_Get (&GAppDim));
  RxnView_LibLbl_Put (rxnv_p, XtVaCreateWidget ("RxnVLibL", 
    xmLabelWidgetClass, form, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNalignment, XmALIGNMENT_BEGINNING,
    XmNheight, AppDim_HtLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLtoR (RVW_VALUE_LIB, 
    AppDim_RxnValTag_Get (&GAppDim));
  RxnView_LibVal_Put (rxnv_p, XtVaCreateWidget ("RxnVLibVL", 
    xmLabelWidgetClass, form, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNalignment, XmALIGNMENT_BEGINNING,
    XmNheight, AppDim_HtLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);
//printf("kka11:in rxnview\n");
  lbl_str = XmStringCreateLtoR (RVW_CHAP_LBL, 
    AppDim_RxnLblTag_Get (&GAppDim));
  RxnView_ChapLbl_Put (rxnv_p, XtVaCreateWidget ("RxnVChapL", 
    xmLabelWidgetClass, form, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNalignment, XmALIGNMENT_BEGINNING,
    XmNheight, AppDim_HtLblPB_Get (&GAppDim),
     NULL));
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLtoR (RVW_VALUE_CHAP, 
    AppDim_RxnValTag_Get (&GAppDim));
  RxnView_ChapVal_Put (rxnv_p, XtVaCreateWidget ("RxnVChapVL", 
    xmLabelWidgetClass, form, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNalignment, XmALIGNMENT_BEGINNING,
    XmNheight, AppDim_HtLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLtoR (RVW_SCHEMA_LBL, 
    AppDim_RxnLblTag_Get (&GAppDim));
  RxnView_SchemaLbl_Put (rxnv_p, XtVaCreateWidget ("RxnVScmaL", 
    xmLabelWidgetClass, form, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNalignment, XmALIGNMENT_BEGINNING,
    XmNheight, AppDim_HtLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLtoR (RVW_VALUE_NUM, 
    AppDim_RxnValTag_Get (&GAppDim));
  RxnView_SchemaVal_Put (rxnv_p, XtVaCreateWidget ("RxnVScmaVL", 
    xmLabelWidgetClass, form, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNalignment, XmALIGNMENT_BEGINNING,
    XmNheight, AppDim_HtLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLtoR (RVW_EASE_LBL, 
    AppDim_RxnLblTag_Get (&GAppDim));
  RxnView_EaseLbl_Put (rxnv_p, XtVaCreateWidget ("RxnVEaseL", 
    xmLabelWidgetClass, form, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNalignment, XmALIGNMENT_BEGINNING,
    XmNheight, AppDim_HtLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLtoR (RVW_VALUE_PARAM, 
    AppDim_RxnValTag_Get (&GAppDim));
  RxnView_EaseVal_Put (rxnv_p, XtVaCreateWidget ("RxnVEaseVL", 
    xmLabelWidgetClass, form, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNalignment, XmALIGNMENT_BEGINNING,
    XmNheight, AppDim_HtLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLtoR (RVW_YIELD_LBL, 
    AppDim_RxnLblTag_Get (&GAppDim));
  RxnView_YieldLbl_Put (rxnv_p, XtVaCreateWidget ("RxnVYldL", 
    xmLabelWidgetClass, form, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNalignment, XmALIGNMENT_BEGINNING,
    XmNheight, AppDim_HtLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLtoR (RVW_VALUE_PARAM, 
    AppDim_RxnValTag_Get (&GAppDim));
  RxnView_YieldVal_Put (rxnv_p, XtVaCreateWidget ("RxnVYldVL", 
    xmLabelWidgetClass, form, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNalignment, XmALIGNMENT_BEGINNING,
    XmNheight, AppDim_HtLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLtoR (RVW_CONF_LBL, 
    AppDim_RxnLblTag_Get (&GAppDim));
  RxnView_ConfLbl_Put (rxnv_p, XtVaCreateWidget ("RxnVConfL", 
    xmLabelWidgetClass, form, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNalignment, XmALIGNMENT_BEGINNING,
    XmNheight, AppDim_HtLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLtoR (RVW_VALUE_PARAM, 
    AppDim_RxnValTag_Get (&GAppDim));
  RxnView_ConfVal_Put (rxnv_p, XtVaCreateWidget ("RxnVConfVL", 
    xmLabelWidgetClass, form, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNalignment, XmALIGNMENT_BEGINNING,
    XmNheight, AppDim_HtLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLtoR (RVW_SOLVED_LBL, 
    AppDim_RxnValTag_Get (&GAppDim));
  RxnView_SolvedLbl_Put (rxnv_p, XtVaCreateWidget ("RxnVSolvedL", 
    xmLabelWidgetClass, form, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNalignment, XmALIGNMENT_BEGINNING,
    XmNheight, AppDim_HtLblPB_Get (&GAppDim),
    XmNrecomputeSize, False,
      NULL));
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLtoR (RVW_SGMRT_LBL, 
    AppDim_RxnLblTag_Get (&GAppDim));
  RxnView_SGMeritLbl_Put (rxnv_p, XtVaCreateWidget ("RxnVSGMrtL", 
    xmLabelWidgetClass, form, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNalignment, XmALIGNMENT_BEGINNING,
    XmNheight, AppDim_HtLblPB_Get (&GAppDim),
    XmNrecomputeSize, False,
      NULL));
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLtoR (RVW_VALUE_MERIT, 
    AppDim_RxnValTag_Get (&GAppDim));
  RxnView_SGMeritVal_Put (rxnv_p, XtVaCreateWidget ("RxnVSGMrtVL", 
    xmLabelWidgetClass, form, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    XmNalignment, XmALIGNMENT_BEGINNING,
    XmNheight, AppDim_HtLblPB_Get (&GAppDim),
    XmNrecomputeSize, False,
      NULL));
  XmStringFree (lbl_str);


  /*  Set the proper attachments.  */
  XtVaSetValues (RxnView_EaseLbl_Get (rxnv_p),
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNtopOffset, -(AppDim_HtLblPB_Get (&GAppDim) 
      + AppDim_SepSmall_Get (&GAppDim)),
    XmNtopAttachment, XmATTACH_OPPOSITE_FORM,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset, AppDim_SepLarge_Get (&GAppDim),
    XmNleftAttachment, XmATTACH_POSITION,
    XmNleftPosition, 0,
    NULL);

  XtVaSetValues (RxnView_EaseVal_Get (rxnv_p),
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget, RxnView_EaseLbl_Get (rxnv_p),
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget, RxnView_EaseLbl_Get (rxnv_p),
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, RxnView_EaseLbl_Get (rxnv_p),
XmNrightOffset, 0,
XmNrightAttachment, XmATTACH_POSITION,
XmNrightPosition, 19,
    NULL);

  XtVaSetValues (RxnView_YieldLbl_Get (rxnv_p),
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget, RxnView_EaseLbl_Get (rxnv_p),
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget, RxnView_EaseLbl_Get (rxnv_p),
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNleftPosition, 20,
    NULL);

  XtVaSetValues (RxnView_YieldVal_Get (rxnv_p),
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget, RxnView_EaseLbl_Get (rxnv_p),
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget, RxnView_EaseLbl_Get (rxnv_p),
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, RxnView_YieldLbl_Get (rxnv_p),
XmNrightOffset, 0,
XmNrightAttachment, XmATTACH_POSITION,
XmNrightPosition, 39,
    NULL);

  XtVaSetValues (RxnView_ConfLbl_Get (rxnv_p),
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget, RxnView_EaseLbl_Get (rxnv_p),
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget, RxnView_EaseLbl_Get (rxnv_p),
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNleftPosition, 40,
    NULL);

  XtVaSetValues (RxnView_ConfVal_Get (rxnv_p),
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget, RxnView_EaseLbl_Get (rxnv_p),
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget, RxnView_EaseLbl_Get (rxnv_p),
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, RxnView_ConfLbl_Get (rxnv_p),
    XmNrightOffset, 0,
/*
    XmNrightAttachment, XmATTACH_NONE,
*/
XmNrightAttachment, XmATTACH_POSITION,
XmNrightPosition, 59,
    NULL);

  XtVaSetValues (RxnView_SGMeritVal_Get (rxnv_p),
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget, RxnView_EaseLbl_Get (rxnv_p),
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget, RxnView_EaseLbl_Get (rxnv_p),
    XmNrightOffset, AppDim_SepLarge_Get (&GAppDim),
    XmNrightAttachment, XmATTACH_FORM,
    NULL);
//printf("kka12:in rxnview\n");
  XtVaSetValues (RxnView_SGMeritLbl_Get (rxnv_p),
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget, RxnView_EaseLbl_Get (rxnv_p),
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget, RxnView_EaseLbl_Get (rxnv_p),
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_WIDGET,
    XmNrightWidget, RxnView_SGMeritVal_Get (rxnv_p),
    NULL);

  XtVaSetValues (RxnView_SolvedLbl_Get (rxnv_p),
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget, RxnView_EaseLbl_Get (rxnv_p),
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget, RxnView_EaseLbl_Get (rxnv_p),
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_NONE,
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_WIDGET,
    XmNrightWidget, RxnView_SGMeritLbl_Get (rxnv_p),
    NULL);

  XtVaSetValues (RxnView_LibLbl_Get (rxnv_p),
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNtopOffset, -(2 * (AppDim_HtLblPB_Get (&GAppDim) 
      + AppDim_SepSmall_Get (&GAppDim))),
    XmNtopAttachment, XmATTACH_OPPOSITE_FORM,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget, RxnView_EaseLbl_Get (rxnv_p),
    XmNleftOffset, AppDim_SepLarge_Get (&GAppDim),
    XmNleftAttachment, XmATTACH_POSITION,
    XmNleftPosition, 0,
    NULL);

  XtVaSetValues (RxnView_LibVal_Get (rxnv_p),
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget, RxnView_LibLbl_Get (rxnv_p),
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget, RxnView_LibLbl_Get (rxnv_p),
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, RxnView_LibLbl_Get (rxnv_p),
    NULL);

  XtVaSetValues (RxnView_ChapLbl_Get (rxnv_p),
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget, RxnView_LibLbl_Get (rxnv_p),
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget, RxnView_LibLbl_Get (rxnv_p),
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNleftPosition, 20,
    NULL);

  XtVaSetValues (RxnView_ChapVal_Get (rxnv_p),
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget, RxnView_LibLbl_Get (rxnv_p),
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget, RxnView_LibLbl_Get (rxnv_p),
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, RxnView_ChapLbl_Get (rxnv_p),
    NULL);

  XtVaSetValues (RxnView_SchemaLbl_Get (rxnv_p),
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget, RxnView_LibLbl_Get (rxnv_p),
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget, RxnView_LibLbl_Get (rxnv_p),
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNleftPosition, 65,
    NULL);

  XtVaSetValues (RxnView_SchemaVal_Get (rxnv_p),
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget, RxnView_LibLbl_Get (rxnv_p),
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget, RxnView_LibLbl_Get (rxnv_p),
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, RxnView_SchemaLbl_Get (rxnv_p),
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_NONE,
    NULL);
//printf("kka12:in rxnview\n");
  XtVaSetValues (RxnView_NameLbl_Get (rxnv_p),
    XmNtopOffset, -(2 * (AppDim_HtLblPB_Get (&GAppDim)  
      + AppDim_SepSmall_Get (&GAppDim)) 
      + AppDim_RxnNameHt_Get (&GAppDim)),
    XmNtopAttachment, XmATTACH_OPPOSITE_FORM,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget, RxnView_LibLbl_Get (rxnv_p),
    XmNleftOffset, AppDim_SepLarge_Get (&GAppDim),
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, AppDim_SepLarge_Get (&GAppDim),
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  XtVaSetValues (RxnView_MolForm_Get (rxnv_p),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget, RxnView_NameLbl_Get (rxnv_p),
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  XtManageChild (RxnView_EaseLbl_Get (rxnv_p));
  XtManageChild (RxnView_EaseVal_Get (rxnv_p));
  XtManageChild (RxnView_YieldLbl_Get (rxnv_p));
  XtManageChild (RxnView_YieldVal_Get (rxnv_p));
  XtManageChild (RxnView_ConfLbl_Get (rxnv_p));
  XtManageChild (RxnView_ConfVal_Get (rxnv_p));
  XtManageChild (RxnView_SGMeritVal_Get (rxnv_p));
  XtManageChild (RxnView_SGMeritLbl_Get (rxnv_p));
  XtManageChild (RxnView_SolvedLbl_Get (rxnv_p));
  XtManageChild (RxnView_ChapLbl_Get (rxnv_p));
  XtManageChild (RxnView_ChapVal_Get (rxnv_p));
  XtManageChild (RxnView_LibLbl_Get (rxnv_p));
  XtManageChild (RxnView_LibVal_Get (rxnv_p));
  XtManageChild (RxnView_SchemaLbl_Get (rxnv_p));
  XtManageChild (RxnView_SchemaVal_Get (rxnv_p));
  XtManageChild (RxnView_NameLbl_Get (rxnv_p));
  XtManageChild (RxnView_MolForm_Get (rxnv_p));
  XtManageChild (form);

  RxnView_CurSG_Put (rxnv_p, NULL);
  RxnView_NumSGs_Put (rxnv_p, 0);
  return (RxnView_Frame_Get (rxnv_p));
}
/*  End of RxnView_Create  */


/****************************************************************************
*
*  Function Name:                 RxnView_Destroy
*
*    This routine.
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
void RxnView_Destroy
  (
  RxnView_t    *rxnv_p,
  Display      *display
  )
{
/*
  if (String_Alloc_Get (RxnView_RxnName_Get (rxnv_p)))
    String_Destroy (RxnView_RxnName_Get (rxnv_p));

  if (RxnView_RxnCntr_Get (rxnv_p) != NULL)
    free (RxnView_RxnCntr_Get (rxnv_p));
  
  if (RxnView_TgtMol_Get (rxnv_p) != NULL)
    free (RxnView_TgtMol_Get (rxnv_p));

  XFreeGC (display, RxnView_GC_Get (rxnv_p));

  XmDestroyPixmap (XtScreen (RxnView_MolForm_Get (rxnv_p)), plus);
  XmDestroyPixmap (XtScreen (RxnView_MolForm_Get (rxnv_p)), dblarw);

*/
  return ;
}
/*  End of RxnView_Destroy  */


/****************************************************************************
*
*  Function Name:                 RxnView_Mols_Scale
*
*    Scale the molecules in the reaction so that they fit nicely
*    in the reaction display window.
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
void RxnView_Mols_Scale
  (
  RxnView_t      *rxnv_p
  )
{
  Dsp_Molecule_t *mol_p;
  Dsp_Atom_t     *atom_p;
  Dimension       avail_daw;
  Dimension       total_daw;
  Dimension       x_off, y_off;
  U16_t           atm_i;
  U8_t            mol_i;
  U8_t            num_mols;

  num_mols = RxnView_NumSGs_Get (rxnv_p) + 1;

  avail_daw = RxnView_MolFormW_Get (rxnv_p) - (num_mols - 1)
    * RVW_SIGN_WD - num_mols * (2 * AppDim_AtmMaxW_Get (&GAppDim) 
    + AppDim_MargMol_Get (&GAppDim));

  total_daw = 0;
  for (mol_i = 0; mol_i < num_mols; mol_i++)
    {
    mol_p = RxnV_IthM_Mol_Get (rxnv_p, mol_i);
    if (mol_p->molh == 0)
      mol_p->scale = AppDim_MolScale_Get (&GAppDim);
    else
      {
      mol_p->scale = (RxnView_MolDAH_Get (rxnv_p) 
        - (2 * AppDim_AtmMaxH_Get (&GAppDim) + AppDim_MargMol_Get (&GAppDim))) 
        / (float) mol_p->molh;

      if (mol_p->scale > AppDim_MolScale_Get (&GAppDim))
        mol_p->scale = AppDim_MolScale_Get (&GAppDim);
      }
    
    total_daw += mol_p->scale * mol_p->molw;
    }

  if (total_daw > avail_daw)
    {
    float    delta;

    delta = avail_daw / (float) total_daw;
    for (mol_i = 0; mol_i < num_mols; mol_i++)
      {
      mol_p = RxnV_IthM_Mol_Get (rxnv_p, mol_i);
      mol_p->scale *= delta;

      RxnV_IthM_DAW_Put (rxnv_p, mol_i, (Dimension) (mol_p->scale
        * mol_p->molw + (2 * AppDim_AtmMaxW_Get (&GAppDim) 
        + AppDim_MargMol_Get (&GAppDim))));
      }
    }
  else
   {
   Dimension      daw_diff;
   Dimension      lda_diff;

    daw_diff = avail_daw - total_daw;
    for (mol_i = 0; mol_i < num_mols; mol_i++)
      {
      mol_p = RxnV_IthM_Mol_Get (rxnv_p, mol_i);

      RxnV_IthM_DAW_Put (rxnv_p, mol_i, (Dimension) (mol_p->scale
        * mol_p->molw + (2 * AppDim_AtmMaxW_Get (&GAppDim) 
        + AppDim_MargMol_Get (&GAppDim))));

     if (daw_diff > 0 &&  RxnV_IthM_LblW_Get (rxnv_p, mol_i)
          > RxnV_IthM_DAW_Get (rxnv_p, mol_i))
        {
        lda_diff = RxnV_IthM_LblW_Get (rxnv_p, mol_i)
          - RxnV_IthM_DAW_Get (rxnv_p, mol_i);
        if (daw_diff > lda_diff)
          {
          RxnV_IthM_DAW_Put (rxnv_p, mol_i, 
            RxnV_IthM_DAW_Get (rxnv_p, mol_i) + lda_diff);
          daw_diff -= lda_diff;
          }
        else
          {
          RxnV_IthM_DAW_Put (rxnv_p, mol_i, 
            RxnV_IthM_DAW_Get (rxnv_p, mol_i) + daw_diff);
          daw_diff = 0;
          }
        }
      }
   }

  /*  Calculate the coordinates of the molecules.  */
  for (mol_i = 0; mol_i < num_mols; mol_i++)
    {
    mol_p = RxnV_IthM_Mol_Get (rxnv_p, mol_i);
    x_off = ((Dimension) (RxnV_IthM_DAW_Get (rxnv_p, mol_i) 
      - mol_p->scale * mol_p->molw)) >> 1;
    y_off = ((Dimension) (RxnView_MolDAH_Get (rxnv_p) 
      - mol_p->scale * mol_p->molh)) >> 1;

    atom_p = mol_p->atoms;

    for (atm_i = 0; atm_i < mol_p->natms; atm_i++)
      {
      atom_p->x = (int) (atom_p->x * mol_p->scale) + x_off;
      atom_p->y = (int) (atom_p->y * mol_p->scale) + y_off;
      atom_p++;
      }
    }

  return ;
}
/*  End of RxnView_Mols_Scale  */


/****************************************************************************
*
*  Function Name:                 RxnView_Update
*
*    Update the reaction view window.
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
void RxnView_Update
  (
  RxnView_t     *rxnv_p, 
  Subgoal_t     *sg_p
  )
{
  char             mlbl_buf[RVW_LBL_BUFLEN];       /* Cmp merit label */
  char             nlbl_buf[RVW_LBL_BUFLEN];       /* Cmp name label */
  char             val_buf[RVW_VAL_BUFLEN]; 
  Compound_t      *pop_p;
  Compound_t      *son_p;
  InfoWin_t       *iw_p;
  SymTab_t        *symb_p;
  Xtr_t           *xtr_p;
  React_Head_t    *rxn_head_p;
  React_Record_t  *rxn_rec_p;
  React_TextRec_t *txt_rec_p;
  XmString         val_lbl;
  XmString         solved_lbl;
  XmString         mrt_lbl;
  XmString         name_lbl;
  Sling_t          temp_sling;
  char            *rxn_name;
  Pixel            lbl_fg;
  Dimension        x_pos;
  Dimension        nlw, nlh, mlw, mlh;
  U16_t            kb_param;
  U16_t            sg_param;
  U8_t             chap_num;
  U8_t             mol_i;
  U8_t             num_sgs;

  if (rxnv_p == NULL || sg_p == NULL)
    return;

  rxn_rec_p = React_Schema_Handle_Get (PstSubg_Reaction_Schema_Get (sg_p));
  rxn_head_p = React_Head_Get (rxn_rec_p);
  txt_rec_p = React_Text_Get (rxn_rec_p);

  /*  Unmanage molecule form and sign widgets.  */
  if (XtIsManaged (RxnV_IthM_Form_Get (rxnv_p, 0)))
    {
    XtUnmanageChild (RxnV_IthM_DA_Get (rxnv_p, 0));
    XtUnmanageChild (RxnV_IthM_Form_Get (rxnv_p, 0));
    }

  for (mol_i = 0; mol_i < RxnView_NumSGs_Get (rxnv_p); mol_i++)
    {
    XtUnmanageChild (RxnV_IthM_Sign_Get (rxnv_p, mol_i));
    XtUnmanageChild (RxnV_IthM_DA_Get (rxnv_p, mol_i + 1));
    XtUnmanageChild (RxnV_IthM_Form_Get (rxnv_p, mol_i + 1));
    }


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
    val_lbl = XmStringCreateLtoR ("Unknown", 
      AppDim_RxnValTag_Get (&GAppDim));

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

  /*  Update ease label.  */
  kb_param = React_Head_Ease_Get (rxn_head_p);
  sg_param = PstSubg_Reaction_Ease_Get (sg_p);
  if (kb_param == sg_param)
    sprintf (val_buf, "%u", kb_param);
  else
    sprintf (val_buf, "%u <-- %u", sg_param, kb_param);
  val_lbl = XmStringCreateLtoR (val_buf, AppDim_RxnValTag_Get (&GAppDim));
  XtVaSetValues (RxnView_EaseVal_Get (rxnv_p),
    XmNlabelString, val_lbl, 
    NULL);
  XmStringFree (val_lbl);

  /*  Update yield label.  */
  kb_param = React_Head_Yield_Get (rxn_head_p);
  sg_param = PstSubg_Reaction_Yield_Get (sg_p);
  if (kb_param == sg_param)
    sprintf (val_buf, "%u", kb_param);
  else
    sprintf (val_buf, "%u <-- %u", sg_param, kb_param);
  val_lbl = XmStringCreateLtoR (val_buf, AppDim_RxnValTag_Get (&GAppDim));
  XtVaSetValues (RxnView_YieldVal_Get (rxnv_p),
    XmNlabelString, val_lbl, 
    NULL);
  XmStringFree (val_lbl);

  /*  Update confidence label.  */
  kb_param = React_Head_Confidence_Get (rxn_head_p);
  sg_param = PstSubg_Reaction_Confidence_Get (sg_p);
  if (kb_param == sg_param)
    sprintf (val_buf, "%u", kb_param);
  else
    sprintf (val_buf, "%u <-- %u", sg_param, kb_param);

  val_lbl = XmStringCreateLtoR (val_buf, AppDim_RxnValTag_Get (&GAppDim));
  XtVaSetValues (RxnView_ConfVal_Get (rxnv_p),
    XmNlabelString, val_lbl, 
    NULL);
  XmStringFree (val_lbl);

  /*  Update (solved) subgoal merit label.  */
  if (PstSubg_Merit_Solved_Get (sg_p) != SUBG_MERIT_INIT)
    {
    solved_lbl = XmStringCreateLtoR (RVW_SOLVED_LBL, 
      AppDim_RxnValTag_Get (&GAppDim));
    sprintf (val_buf, "%d", PstSubg_Merit_Solved_Get (sg_p));
    }
  else
    {
    solved_lbl = XmStringCreateLtoR ("", AppDim_RxnValTag_Get (&GAppDim));
    sprintf (val_buf, "%d", PstSubg_Merit_Main_Get (sg_p));
    }
  XtVaSetValues (RxnView_SolvedLbl_Get (rxnv_p),
    XmNlabelString, solved_lbl, 
    NULL);
  XmStringFree (solved_lbl);

  val_lbl = XmStringCreateLtoR (val_buf, AppDim_RxnValTag_Get (&GAppDim));
  XtVaSetValues (RxnView_SGMeritVal_Get (rxnv_p),
    XmNlabelString, val_lbl, 
    NULL);
  XmStringFree (val_lbl);


  /*  Update reaction drawings.  */

  /*  Update goal compound first.  */
  pop_p = PstSubg_Father_Get (sg_p);
  symb_p = PstComp_SymbolTable_Get (pop_p);
  temp_sling = Sling_CopyTrunc (SymTab_Sling_Get (symb_p));

  /*  Change labels and fetch molecule.  */
  if (SymTab_Flags_Available_Get (symb_p))
    lbl_fg = SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_AVL_VD);
  else if (SymTab_Flags_Solved_Get (symb_p))
    lbl_fg = SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_SOL_VD);
  else if (SymTab_Flags_Stuck_Get (symb_p) 
        || SymTab_Flags_Unsolveable_Get (symb_p))
    lbl_fg = SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_UNS_VD);
  else if (SymTab_Flags_Open_Get (symb_p))
    {
    if (SymTab_DevelopedComp_Get (symb_p) != NULL)
      lbl_fg = SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_DEV_VD);
    else 
    lbl_fg = SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_UND_VD);
    }
  else
    lbl_fg = SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK);

  if (SymTab_Flags_Available_Get (symb_p))
    {
    Avi_CmpInfo_t  *cmp_info_p;

    cmp_info_p = AvcLib_Info (AvcLib_Key ((char *) 
      Sling_Name_Get (temp_sling)));

    /*  Don't include catalog number in compound number label.
    sprintf (mlbl_buf, "cmp %1lu     %s", SymTab_Index_Get (symb_p),
      Avi_CmpInfo_Catalog_Get (cmp_info_p));
    */
    sprintf (mlbl_buf, "cmp %1lu", SymTab_Index_Get (symb_p));
    if (Avi_CmpInfo_NameLen_Get (cmp_info_p) > 0)
       sprintf (nlbl_buf, "%s", Avi_CmpInfo_Name_Get (cmp_info_p));
    else
       sprintf (nlbl_buf, "%s", "available");

    AviCmpInfo_Destroy (cmp_info_p);
    }
  else
    {
    sprintf (mlbl_buf, "cmp %1lu     merit:  %1u", SymTab_Index_Get (symb_p),
      SymTab_Flags_Solved_Get (symb_p) ? SymTab_Merit_Solved_Get (symb_p) 
      : SymTab_Merit_Main_Get (symb_p));
//kka     sprintf (nlbl_buf," ");
    sprintf (nlbl_buf,"%s\n[Caution: May be a partial drawing.\n  Draw manually using above SLING if needed]", temp_sling); //kka 
    }

  mrt_lbl = XmStringCreateLtoR (mlbl_buf, AppDim_RxnValTag_Get (&GAppDim));
  XtVaSetValues (RxnV_IthM_MrtLbl_Get (rxnv_p, 0),
    XmNlabelString, mrt_lbl, 
    XmNforeground, lbl_fg,
    NULL);
  XmStringExtent (SynAppR_FontList_Get (&GSynAppR), mrt_lbl, &mlw, &mlh);
  XmStringFree (mrt_lbl);

  name_lbl = XmStringCreateLtoR (nlbl_buf, AppDim_RxnMrtTag_Get (&GAppDim));
  XtVaSetValues (RxnV_IthM_NameLbl_Get (rxnv_p, 0),
    XmNlabelString, name_lbl, 
    NULL);
  XmStringExtent (SynAppR_FontList_Get (&GSynAppR), name_lbl, &nlw, &nlh);
  XmStringFree (name_lbl);
  nlw += 2 * AppDim_MargLblPB_Get (&GAppDim);
  RxnV_IthM_LblW_Put (rxnv_p, 0, (mlw > nlw) 
      ? mlw + 3 * AppDim_MargLblPB_Get (&GAppDim)
      : nlw + 3 * AppDim_MargLblPB_Get (&GAppDim));

  xtr_p = Sling2Xtr (temp_sling);
  if (RxnV_IthM_Mol_Get (rxnv_p, 0) != NULL)
    free_Molecule (RxnV_IthM_Mol_Get (rxnv_p, 0));
 
  RxnV_IthM_Mol_Put (rxnv_p, 0, Xtr2Dsp (xtr_p));

  Xtr_Destroy (xtr_p);
//int i;//kka
//printf("Draw first molecule\n");//kka
//scanf("%d",&i);//kka
  if (!dsp_Shelley (RxnV_IthM_Mol_Get (rxnv_p, 0)))
    {
    fprintf (stderr, 
      "\nkka:RxnView_Update:  unable to calculate coords for product molecule.\n"); //kka
    fprintf(stderr, "Sling:%s\n",Sling2String(temp_sling));//kka
    
    //free_Molecule (RxnV_IthM_Mol_Get (rxnv_p, 0));//kka
    //RxnV_IthM_Mol_Put (rxnv_p, 0, NULL);//kka
    }
  Sling_Destroy (temp_sling);
  /*  Update subgoal compounds.  */
  num_sgs = (U8_t) PstSubg_NumSons_Get (sg_p); 
  RxnView_NumSGs_Put (rxnv_p, num_sgs);
  son_p = PstSubg_Son_Get (sg_p);
  for (mol_i = 1; mol_i < RVW_MAXNUM_MOLS && mol_i <= num_sgs; mol_i++)
    {
    symb_p = PstComp_SymbolTable_Get (son_p);
    temp_sling = Sling_CopyTrunc (SymTab_Sling_Get (symb_p));

    /*  Change labels, get molecule and then draw it.  */
    if (SymTab_Flags_Available_Get (symb_p))
      lbl_fg = SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_AVL_VD);
    else if (SymTab_Flags_Solved_Get (symb_p))
      lbl_fg = SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_SOL_VD);
    else if (SymTab_Flags_Stuck_Get (symb_p) 
          || SymTab_Flags_Unsolveable_Get (symb_p))
      lbl_fg = SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_UNS_VD);
    else if (SymTab_Flags_Open_Get (symb_p))
      {
      if (SymTab_DevelopedComp_Get (symb_p) != NULL)
        lbl_fg = SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_DEV_VD);
      else 
      lbl_fg = SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_UND_VD);
      }
    else
      lbl_fg = SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK);

    if (SymTab_Flags_Available_Get (symb_p))
      {
      Avi_CmpInfo_t  *cmp_info_p;

      cmp_info_p = AvcLib_Info (AvcLib_Key ((char *) 
        Sling_Name_Get (temp_sling)));

    /*  Don't include catalog number in compound number label.
      sprintf (mlbl_buf, "cmp %1lu     %s", SymTab_Index_Get (symb_p),
        Avi_CmpInfo_Catalog_Get (cmp_info_p));
    */
      sprintf (mlbl_buf, "cmp %1lu", SymTab_Index_Get (symb_p));
      if (Avi_CmpInfo_NameLen_Get (cmp_info_p) > 0)
         sprintf (nlbl_buf, "%s", Avi_CmpInfo_Name_Get (cmp_info_p));
      else
         sprintf (nlbl_buf, "%s", "available");

      AviCmpInfo_Destroy (cmp_info_p);
      }
    else
      {
      sprintf (mlbl_buf, "cmp %1lu     merit:  %1u", 
        SymTab_Index_Get (symb_p),SymTab_Flags_Solved_Get (symb_p) 
        ? SymTab_Merit_Solved_Get (symb_p) : SymTab_Merit_Main_Get (symb_p));
//kka     sprintf (nlbl_buf," ");
    sprintf (nlbl_buf,"%s\n[Caution: May be a partial drawing.\n  Draw manually using above SLING if needed]", temp_sling); //kka 
      }


    mrt_lbl = XmStringCreateLtoR (mlbl_buf, AppDim_RxnValTag_Get (&GAppDim));
    XtVaSetValues (RxnV_IthM_MrtLbl_Get (rxnv_p, mol_i),
      XmNlabelString, mrt_lbl, 
      XmNforeground, lbl_fg,
      NULL);
    XmStringExtent (SynAppR_FontList_Get (&GSynAppR), mrt_lbl, &mlw, &mlh);
    XmStringFree (mrt_lbl);

    name_lbl = XmStringCreateLtoR (nlbl_buf, AppDim_RxnMrtTag_Get (&GAppDim));
    XtVaSetValues (RxnV_IthM_NameLbl_Get (rxnv_p, mol_i),
      XmNlabelString, name_lbl, 
      NULL);
    XmStringExtent (SynAppR_FontList_Get (&GSynAppR), name_lbl, &nlw, &nlh);
    XmStringFree (name_lbl);
    nlw += 2 * AppDim_MargLblPB_Get (&GAppDim);
    RxnV_IthM_LblW_Put (rxnv_p, mol_i, (mlw > nlw) 
        ? mlw + 3 * AppDim_MargLblPB_Get (&GAppDim)
        : nlw + 3 * AppDim_MargLblPB_Get (&GAppDim));

    xtr_p = Sling2Xtr (temp_sling);
    if (RxnV_IthM_Mol_Get (rxnv_p, mol_i) != NULL)
      free_Molecule (RxnV_IthM_Mol_Get (rxnv_p, mol_i));

    RxnV_IthM_Mol_Put (rxnv_p, mol_i, Xtr2Dsp (xtr_p));

    Xtr_Destroy (xtr_p);
    if (!dsp_Shelley (RxnV_IthM_Mol_Get (rxnv_p, mol_i)))
      {
      fprintf (stderr, 
        "\nkka:RxnView_Update:  unable to calculate coords for %1u react mol.\n",mol_i); //kka
      fprintf(stderr, "Sling:%s\n",Sling2String(temp_sling));//kka
      //free_Molecule (RxnV_IthM_Mol_Get (rxnv_p, mol_i));//kka
      //RxnV_IthM_Mol_Put (rxnv_p, mol_i, NULL);//kka
      }
    Sling_Destroy (temp_sling);
    son_p = PstComp_Brother_Get (son_p);
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
  for (mol_i = 0; mol_i < num_sgs; mol_i++)
    {
//int i;//kka
//printf("Draw next molecule\n");//kka
//scanf("%d",&i);//kka
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
  
  RxnView_CurSG_Put (rxnv_p, sg_p);
  iw_p = InfoWin_Handle_Get (SVI_INFO_RXN);
  if (XtIsManaged (InfoWin_InfoDlg_Get (*iw_p)))
    InfoWin_Reaction_Update ();

  return ;
}
/*  End of RxnView_Update  */


/*  End of RXN_VIEW.C  */