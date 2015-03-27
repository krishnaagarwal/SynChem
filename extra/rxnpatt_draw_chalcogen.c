/****************************************************************************
*  
* Copyright (C) 1999 Synchem Group at SUNY-Stony Brook, Daren Krebsbach  
*  
*  
*  Module Name:               RXNPATT_DRAW.C
*  
*    Creates and manages the menu bar of the job submission tool.  
*      
*  Creation Date:  
*  
*     25-Feb-1999  
*  
*  Authors: 
*      Jerry Miller
*        based on submit_draw by Daren Krebsbach 
*        based on version by Nader Abdelsadek
*  
*  Routines:
*
*    PDraw_Tool_Create
*    PDraw_Mode_Reset
*    PDraw_Tool_Initialize
*    PPixmap_Install
*    PCreateOptionMenu
*    PPeriodic_TblDlg_Create
*    PMolDraw_Sling_Draw
*    PAtom_Copy
*    PDrawTool_Destroy
*
*    pdraw_Double_Bond
*    pdraw_Stereo_Bond
*    pdraw_Bond
*    pdraw_Atom
*    perase_Bond
*    pdraw_Molecule
*
*    PNew_Dsp2Xtr
*    PDsp2Tsd
*    PSling2CanonSling
*    PMolecule_Double
*    pstore_BondNew
*    pstore_AtomNew
*
*    predraw
*    pclear_it
*    PDone_CB
*    PIsolate_CB
*    PNumTogl_CB
*    PPixMap_Display_CB
*    PPixMap_Resize_CB
*    PAtom_Mark_CB
*    POption_Choose_CB
*    PMenu_Choice_CB
*    PPeriodic_TblDlg_Show_CB
*    PSelected_Bond_Draw_CB
*    PMOLDRAW_StereoChem_CB
*
*    RxnInfo_SlingTxt_Update
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
*
*******************************************************************************/  
#include <math.h>
#include <string.h>
#include <Xm/Xm.h>
#include <X11/Xutil.h>

#include <Xm/MainW.h>
#include <Xm/DrawingA.h>
#include <Xm/DrawnB.h>
#include <Xm/MessageB.h>
#include <Xm/PushBG.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/Separator.h>
#include <Xm/ToggleB.h> 
#include <Xm/List.h>

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

#ifndef _H_ATOMSYM_
#include "atomsym_chalcogen.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_SLING2XTR_
#include "sling2xtr.h"
#endif

#ifndef _H_APP_RESRC_  
#include "app_resrc.h"  
#endif 

#ifndef _H_SYNHELP_
#include "synhelp.h"
#endif

#ifndef _H_ISAM_
#include "isam.h"
#endif

#ifndef _H_FUNCGROUP_FILE_
#include "funcgroup_file.h"
#endif

#define PDRAW_GLOBALS
#ifndef _H_RXNPATT_DRW_
#include "rxnpatt_draw_chalcogen.h"
#include "rxnpatt_edit.h"
#endif
#undef PDRAW_GLOBALS

#ifndef _H_EXTERN_
#include "extern.h"
#endif

Boolean_t Sch_Tsds_Store (Tsd_t *, Tsd_t *, U32_t *, U32_t *, int, char *);
void Ist_Info_Send (Tsd_t *, Tsd_t *, int *, int *, int *, int *);
void Get_Selected_Syntheme (int);
void IsTFG_Form_Create (Widget, int *, int *, int *, int *);
Boolean_t Root_Atom_OK (char *);

extern Boolean_t glob_rxlform;

  /* Redefining button events for translation table */
static String  translations = 
  "<Btn1Down>:    PDraw_Draw(down1)   \n\
   <Btn2Down>:    PDraw_Draw(down2)   \n\
   <Btn3Down>:    PDraw_Draw(down3)   \n\
   <Motion>:      PDraw_Draw(motion)";


static int gsg = GOAL;
/*
static int selgsg = GOAL;
*/
static Boolean_t first_time = TRUE;
static Boolean_t gsgsame = TRUE;
static Boolean_t nodenums_on = TRUE;
static int Goal = GOAL;
static int Subgoal = SUBG;
static char sling_roots[100][5];
static char root_atom[3] = {0};
/*
static int lastatomgsg = NEITHER;
static int lastbondgsg = NEITHER;
*/

static Boolean_t synlist_managed = FALSE;
static Widget glob_parent;
static U32_t glob_syntheme;

/****************************************************************************
*  
*  Function Name:                 PDraw_Tool_Create
*  
*****************************************************************************/  

Widget PDraw_Tool_Create 
  (
  RxnInfoCB_t   *rxninfo_p,
  Widget         parent,
  XtAppContext  *appcon,
  Xtr_t         *goal_xtr,
  Xtr_t         *subg_xtr,
  U32_t         *roots_p,
  U32_t         *synthemes_p,
  U32_t          chap_syntheme
  )
{
  GC                gc;
  Pixmap            pixmap;
  ScreenAttr_t     *sca_p;
  Display          *display;
  Widget            form;
  XmString          lbl_drw, lbl_sel, lbl_srt, lbl_del, lbl_drt, lbl_done;
  XmString          lbl_str, lbl_strs[2]; 
  Widget            separator[2],
                    separator1, separator2, separator3, separator4, separator5;
  XtActionsRec      draw_actions;
  char              bitmap_file[256];
  int               pattern, i;
  Dimension         sw_w, sw_h;
  Boolean_t         must_clear;
  XmFontList        flhv12;
  XmFontListEntry   helv12;

  if (!first_time)
    {
    if (PDrawMol_p != NULL)
      free_Reaction (PDrawMol_p);
    PDrawMol_p = NULL;
    }

  gsg = GOAL;

  if (IsThere_Draw_Flag && glob_rxlform) nodenums_on = FALSE;
  else nodenums_on = TRUE;

  glob_syntheme = chap_syntheme;

  if (goal_xtr == NULL || subg_xtr == NULL || roots_p == NULL ||
    synthemes_p == NULL)
    {
printf("main entry: IsThere_Draw_Flag=%d glob_rxlform=%d\n",IsThere_Draw_Flag,glob_rxlform);
    gsgsame = !(IsThere_Draw_Flag && glob_rxlform);
    PDrawMol_p = NULL;
    must_clear = TRUE;
    }
  else
    {
    gsgsame = FALSE;
    PDrawMol_p = Xtr2RxnDsp (goal_xtr, subg_xtr, roots_p, synthemes_p);
    for (i = 0; i < PDrawMol_p->both_dm.natms; i++)
      for (pattern = 0; pattern < 2; pattern++)
      RxnDsp_AtomCharge_Put (RxnDsp_AtomPtr_Get (PDrawMol_p, i), pattern,
        nodenums_on ? i + 1 : 0);
    must_clear = FALSE;
    }

printf("* * * gsgsame=%d* * *\n",gsgsame);

  /* Allocate necessary memory for global drawing structures.  UGH! - DK  */
  PDraw_Tool_Initialize ();

  if (!first_time)
    {
    XtManageChild (PDrawTool_Frame_Get (&GPDrawToolCB));
    if (IsThere_Draw_Flag && glob_rxlform)
      {
lbl_drt = XmStringCreateLocalized ("");
      XtManageChild (PDrawTool_DonePB_Get (&GPDrawToolCB));
      XtManageChild (PDrawTool_ClearPB_Get (&GPDrawToolCB));
      XtUnmanageChild (PDrawTool_IsolatePB_Get (&GPDrawToolCB));
/**/
      XtUnmanageChild (PDrawTool_NumToglPB_Get (&GPDrawToolCB));
/**/
/*
      XtUnmanageChild (XtNameToWidget (PDrawTool_EditModeRB_Get (&GPDrawToolCB), "button_4"));
*/
XtVaSetValues(XtNameToWidget (PDrawTool_EditModeRB_Get (&GPDrawToolCB), "button_4"),
XmNvisibleWhenOff, False,
XmNlabelString, lbl_drt,
NULL);
      XtUnmanageChild (PDrawTool_RootSynForm_Get (&GPDrawToolCB));
      lbl_srt = XmStringCreateLocalized (CS_PADDFGS); 
      lbl_str = XmStringCreateLocalized ("IsThere Fragment Editor");
      lbl_done = XmStringCreateLocalized ("Exit; start search");
      }
    else if (glob_rxlform)
      {
lbl_drt = XmStringCreateLocalized (CS_PDELETEROOT);
      XtManageChild (PDrawTool_DonePB_Get (&GPDrawToolCB));
      XtManageChild (PDrawTool_ClearPB_Get (&GPDrawToolCB));
      XtManageChild (PDrawTool_IsolatePB_Get (&GPDrawToolCB));
/**/
      XtManageChild (PDrawTool_NumToglPB_Get (&GPDrawToolCB));
/**/
/*
      XtManageChild (XtNameToWidget (PDrawTool_EditModeRB_Get (&GPDrawToolCB), "button_4"));
*/
XtVaSetValues(XtNameToWidget (PDrawTool_EditModeRB_Get (&GPDrawToolCB), "button_4"),
XmNvisibleWhenOff, True,
XmNlabelString, lbl_drt,
NULL);
      XtManageChild (PDrawTool_RootSynForm_Get (&GPDrawToolCB));
      lbl_srt = XmStringCreateLocalized (CS_PSELECTROOT); 
      lbl_str = XmStringCreateLocalized ("Transform Pattern Editor");
      lbl_done = XmStringCreateLocalized (CS_PDONE);  
      }
    else
      {
lbl_drt = XmStringCreateLocalized (CS_PDELETEROOT);
      XtUnmanageChild (PDrawTool_DonePB_Get (&GPDrawToolCB));
      XtUnmanageChild (PDrawTool_ClearPB_Get (&GPDrawToolCB));
      XtUnmanageChild (PDrawTool_IsolatePB_Get (&GPDrawToolCB));
/**/
      XtManageChild (PDrawTool_NumToglPB_Get (&GPDrawToolCB));
/**/
/*
      XtManageChild (XtNameToWidget (PDrawTool_EditModeRB_Get (&GPDrawToolCB), "button_4"));
*/
XtVaSetValues(XtNameToWidget (PDrawTool_EditModeRB_Get (&GPDrawToolCB), "button_4"),
XmNvisibleWhenOff, True,
XmNlabelString, lbl_drt,
NULL);
      XtManageChild (PDrawTool_RootSynForm_Get (&GPDrawToolCB));
      lbl_srt = XmStringCreateLocalized (CS_PSELECTROOT); 
      lbl_str = XmStringCreateLocalized ("Transform Pattern Viewer");
      lbl_done = XmStringCreateLocalized (CS_PDONE);  
      }

    XmStringFree (lbl_drt);

    XtVaSetValues (XtNameToWidget (PDrawTool_EditModeRB_Get (&GPDrawToolCB), "button_2"),
      XmNlabelString, lbl_srt,
      NULL);
    XmStringFree (lbl_srt);

    XtVaSetValues (PDrawTool_Frame_Get (&GPDrawToolCB),
      XmNdialogTitle, lbl_str,
      NULL);
    XmStringFree (lbl_str);

    XtVaSetValues (PDrawTool_DonePB_Get (&GPDrawToolCB),
      XmNlabelString, lbl_done,
      NULL);
    XmStringFree (lbl_done);


    if (must_clear)
      PMenu_Choice_CB (PDrawTool_ClearPB_Get (&GPDrawToolCB), (XtPointer) PDRW_MENU_DELETE_ALL, (XtPointer) NULL);
    else
      POption_Choose_CB (PDrawTool_RedrawPB_Get (&GPDrawToolCB), (XtPointer) PDRW_REDRAW_FLAG, (XtPointer) NULL);
    return (PDrawTool_Frame_Get (&GPDrawToolCB));
    }

  first_time = FALSE;

  helv12 = XmFontListEntryLoad (XtDisplay (parent), "-*-helvetica-bold-r-normal-*-12-*-75-75-*-*-iso8859-1",
    XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);
  flhv12 = XmFontListAppendEntry (NULL, helv12);

  glob_parent = parent;

  /*  Construct the Drawing tool dialog.  */
/*
  PDrawTool_Frame_Put (&GPDrawToolCB, XtVaCreateWidget ("PDrawToolFr",
    xmFrameWidgetClass,  parent,
    XmNshadowType,       XmSHADOW_IN,
    XmNshadowThickness,  3,
    XmNmarginWidth,      AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginHeight,     AppDim_MargFmFr_Get (&GAppDim),
    NULL));
*/
  PDrawTool_ExitErrorMsg_Put (&GPDrawToolCB, XmCreateMessageDialog (parent, "PDrawToolEEMsg", NULL, 0));

  XtUnmanageChild (XmMessageBoxGetChild (PDrawTool_ExitErrorMsg_Get (&GPDrawToolCB), XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild (XmMessageBoxGetChild (PDrawTool_ExitErrorMsg_Get (&GPDrawToolCB), XmDIALOG_HELP_BUTTON));
  XtAddCallback (XmMessageBoxGetChild (PDrawTool_ExitErrorMsg_Get (&GPDrawToolCB), XmDIALOG_OK_BUTTON),
    XmNactivateCallback, PDismissErr_CB, (XtPointer) NULL);

  lbl_str = XmStringCreateLocalized ("Error: Attempt to Save Invalid Transform");
  XtVaSetValues (PDrawTool_ExitErrorMsg_Get (&GPDrawToolCB),
    XmNdialogTitle, lbl_str,
    XmNmessageString, lbl_str,
    NULL);
  XmStringFree (lbl_str);

  PDrawTool_Frame_Put (&GPDrawToolCB, XmCreateFormDialog (parent, "PDrawToolFr", NULL, 0));

  lbl_str = XmStringCreateLocalized ("Transform Pattern Editor");

  XtVaSetValues (PDrawTool_Frame_Get (&GPDrawToolCB),
    XmNdialogTitle, lbl_str,
    XmNwidth,1100,
    XmNheight,900,
    XmNshadowType,       XmSHADOW_IN,
    XmNshadowThickness,  3,
    XmNmarginWidth,      AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginHeight,     AppDim_MargFmFr_Get (&GAppDim),
    NULL);

  XmStringFree (lbl_str);

  form = XtVaCreateWidget ("PDrawToolFm", 
    xmFormWidgetClass,  PDrawTool_Frame_Get (&GPDrawToolCB), 
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    2 * AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     2 * AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       False,
    XmNfractionBase,    200,
    XmNtopOffset,0,
    XmNtopAttachment,XmATTACH_FORM,
    XmNbottomOffset,0,
    XmNbottomAttachment,XmATTACH_FORM,
    XmNleftOffset,0,
    XmNleftAttachment,XmATTACH_FORM,
    XmNrightOffset,0,
    XmNrightAttachment,XmATTACH_FORM,
    NULL);
  PDrawTool_Form_Put (&GPDrawToolCB, form);

  /*  Construct the edit options subform.  */
  PDrawTool_EditForm_Put (&GPDrawToolCB, XtVaCreateWidget ("PDrawEditFm",
    xmFormWidgetClass,  form, 
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       False,
    NULL));

  lbl_str = XmStringCreateLocalized (SBD_PMODE_LBL);  
  PDrawTool_EditModeLbl_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawModeLbl",  
      xmLabelWidgetClass,  PDrawTool_EditForm_Get (&GPDrawToolCB),  
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,    False,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  lbl_drw = XmStringCreateLocalized (CS_PDRAW_BOND); 
  lbl_sel = XmStringCreateLocalized (CS_PSELECT); 
  lbl_srt = XmStringCreateLocalized (CS_PSELECTROOT); 
  lbl_del = XmStringCreateLocalized (CS_PCLEAR); 
  lbl_drt = XmStringCreateLocalized (CS_PDELETEROOT); 
  PDrawTool_EditModeRB_Put (&GPDrawToolCB, 
     XmVaCreateSimpleRadioBox (PDrawTool_EditForm_Get (&GPDrawToolCB), 
       "PDrawModeRB",    0,        POption_Choose_CB,
       XmVaRADIOBUTTON, lbl_drw,  NULL, NULL, NULL, 
       XmVaRADIOBUTTON, lbl_sel,  NULL, NULL, NULL, 
       XmVaRADIOBUTTON, lbl_srt,  NULL, NULL, NULL, 
       XmVaRADIOBUTTON, lbl_del,  NULL, NULL, NULL, 
       XmVaRADIOBUTTON, lbl_drt,  NULL, NULL, NULL, 
       NULL)); 
  XmStringFree(lbl_drw); 
  XmStringFree(lbl_sel); 
  XmStringFree(lbl_srt); 
  XmStringFree(lbl_del); 
  XmStringFree(lbl_drt); 
 
  lbl_str = XmStringCreateLocalized (SBD_PEDIT_LBL);  
  PDrawTool_EditOptLbl_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawEditLbl",  
      xmLabelWidgetClass,  PDrawTool_EditForm_Get (&GPDrawToolCB),  
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,    False,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLocalized (CS_PSELECTALL);  
  PDrawTool_EditSelAllPB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawEditPB",  
      xmPushButtonWidgetClass, PDrawTool_EditForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_EditSelAllPB_Get (&GPDrawToolCB),
     XmNactivateCallback, PMenu_Choice_CB, (XtPointer) PDRW_MENU_SELECT_ALL);

  lbl_str = XmStringCreateLocalized (CS_PCLEARSEL);  
  PDrawTool_EditDelSelPB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawEditPB",  
      xmPushButtonWidgetClass, PDrawTool_EditForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_EditDelSelPB_Get (&GPDrawToolCB),
     XmNactivateCallback, PMenu_Choice_CB, (XtPointer) PDRW_MENU_DELETE_SEL);

   XtVaSetValues (PDrawTool_EditModeLbl_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (PDrawTool_EditModeRB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_EditModeLbl_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (PDrawTool_EditOptLbl_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_EditModeRB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (PDrawTool_EditSelAllPB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_EditOptLbl_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (PDrawTool_EditDelSelPB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_EditSelAllPB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);


  /*  Construct the atom symbol subform.  */
  PDrawTool_AtomForm_Put (&GPDrawToolCB, XtVaCreateWidget ("PDrawAtomFm",
    xmFormWidgetClass,  form, 
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    0,
    XmNmarginWidth,     0,
    XmNresizable,       False,
    NULL));

  lbl_str = XmStringCreateLocalized (SBD_PATOM_LBL);  
  PDrawTool_AtomLbl_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawAtomLbl",  
      xmLabelWidgetClass,  PDrawTool_AtomForm_Get (&GPDrawToolCB),  
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,    False,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLocalized (CS_PELM_C);  
  PDrawTool_Atm_C_PB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawAtomPB",  
      xmPushButtonWidgetClass, PDrawTool_AtomForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_Atm_C_PB_Get (&GPDrawToolCB),
     XmNactivateCallback, PAtom_Mark_CB, (XtPointer) CS_PELM_C);

  lbl_str = XmStringCreateLocalized (CS_PELM_H);  
  PDrawTool_Atm_H_PB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawAtomPB",  
      xmPushButtonWidgetClass, PDrawTool_AtomForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_Atm_H_PB_Get (&GPDrawToolCB),
     XmNactivateCallback, PAtom_Mark_CB, (XtPointer) CS_PELM_H);

  lbl_str = XmStringCreateLocalized (CS_PELM_N);  
  PDrawTool_Atm_N_PB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawAtomPB",  
      xmPushButtonWidgetClass, PDrawTool_AtomForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_Atm_N_PB_Get (&GPDrawToolCB),
     XmNactivateCallback, PAtom_Mark_CB, (XtPointer) CS_PELM_N);

  lbl_str = XmStringCreateLocalized (CS_PELM_O);  
  PDrawTool_Atm_O_PB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawAtomPB",  
      xmPushButtonWidgetClass, PDrawTool_AtomForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_Atm_O_PB_Get (&GPDrawToolCB),
     XmNactivateCallback, PAtom_Mark_CB, (XtPointer) CS_PELM_O);

  lbl_str = XmStringCreateLocalized (CS_PELM_S);  
  PDrawTool_Atm_S_PB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawAtomPB",  
      xmPushButtonWidgetClass, PDrawTool_AtomForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_Atm_S_PB_Get (&GPDrawToolCB),
     XmNactivateCallback, PAtom_Mark_CB, (XtPointer) CS_PELM_S);

  lbl_str = XmStringCreateLocalized (CS_PELM_P);  
  PDrawTool_Atm_P_PB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawAtomPB",  
      xmPushButtonWidgetClass, PDrawTool_AtomForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_Atm_P_PB_Get (&GPDrawToolCB),
     XmNactivateCallback, PAtom_Mark_CB, (XtPointer) CS_PELM_P);

  lbl_str = XmStringCreateLocalized (CS_PELM_Cl);  
  PDrawTool_Atm_Cl_PB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawAtomPB",  
      xmPushButtonWidgetClass, PDrawTool_AtomForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_Atm_Cl_PB_Get (&GPDrawToolCB),
     XmNactivateCallback, PAtom_Mark_CB, (XtPointer) CS_PELM_Cl);

  lbl_str = XmStringCreateLocalized (CS_PELM_F);  
  PDrawTool_Atm_F_PB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawAtomPB",  
      xmPushButtonWidgetClass, PDrawTool_AtomForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_Atm_F_PB_Get (&GPDrawToolCB),
     XmNactivateCallback, PAtom_Mark_CB, (XtPointer) CS_PELM_F);

  lbl_str = XmStringCreateLocalized (CS_PELM_Br);  
  PDrawTool_Atm_Br_PB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawAtomPB",  
      xmPushButtonWidgetClass, PDrawTool_AtomForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_Atm_Br_PB_Get (&GPDrawToolCB),
     XmNactivateCallback, PAtom_Mark_CB, (XtPointer) CS_PELM_Br);

  lbl_str = XmStringCreateLocalized (CS_PELM_I);  
  PDrawTool_Atm_I_PB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawAtomPB",  
      xmPushButtonWidgetClass, PDrawTool_AtomForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_Atm_I_PB_Get (&GPDrawToolCB),
     XmNactivateCallback, PAtom_Mark_CB, (XtPointer) CS_PELM_I);

  lbl_str = XmStringCreateLocalized (CS_PELM_R);  
  PDrawTool_Atm_R_PB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawAtomPB",  
      xmPushButtonWidgetClass, PDrawTool_AtomForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_Atm_R_PB_Get (&GPDrawToolCB),
     XmNactivateCallback, PAtom_Mark_CB, (XtPointer) CS_PELM_R);

  lbl_str = XmStringCreateLocalized (CS_PELM_R_P);  
  PDrawTool_Atm_Rp_PB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawAtomPB",  
      xmPushButtonWidgetClass, PDrawTool_AtomForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_Atm_Rp_PB_Get (&GPDrawToolCB),
     XmNactivateCallback, PAtom_Mark_CB, (XtPointer) CS_PELM_R_P);

  lbl_str = XmStringCreateLocalized (CS_PELM_DOTLBL);  
  PDrawTool_Atm_Dot_PB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawAtomPB",  
      xmPushButtonWidgetClass, PDrawTool_AtomForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_Atm_Dot_PB_Get (&GPDrawToolCB),
     XmNactivateCallback, PAtom_Mark_CB, (XtPointer) CS_PELM_DOT);

  lbl_str = XmStringCreateLocalized (CS_PELM_CLNLBL);  
  PDrawTool_Atm_Cln_PB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawAtomPB",  
      xmPushButtonWidgetClass, PDrawTool_AtomForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_Atm_Cln_PB_Get (&GPDrawToolCB),
     XmNactivateCallback, PAtom_Mark_CB, (XtPointer) CS_PELM_CLN);

  lbl_str = XmStringCreateLocalized (CS_PELM_X);  
  PDrawTool_Atm_X_PB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawAtomPB",  
      xmPushButtonWidgetClass, PDrawTool_AtomForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_Atm_X_PB_Get (&GPDrawToolCB),
     XmNactivateCallback, PAtom_Mark_CB, (XtPointer) CS_PELM_X);

  lbl_str = XmStringCreateLocalized (CS_PELM_CHALCOGEN);  
  PDrawTool_Atm_Ch_PB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawAtomPB",  
      xmPushButtonWidgetClass, PDrawTool_AtomForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_Atm_Ch_PB_Get (&GPDrawToolCB),
     XmNactivateCallback, PAtom_Mark_CB, (XtPointer) CS_PELM_CH);

  lbl_str = XmStringCreateLocalized (CS_PELM_OTHER);  
  PDrawTool_Atm_Other_PB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawAtomPB",  
      xmPushButtonWidgetClass, PDrawTool_AtomForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_Atm_Other_PB_Get (&GPDrawToolCB),
     XmNactivateCallback, PPeriodic_TblDlg_Show_CB, (XtPointer) NULL);

  XtVaSetValues (PDrawTool_AtomLbl_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (PDrawTool_Atm_C_PB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_AtomLbl_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     0,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    50,
    NULL);

  XtVaSetValues (PDrawTool_Atm_H_PB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        PDrawTool_Atm_C_PB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     PDrawTool_Atm_C_PB_Get (&GPDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     50,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    100,
    NULL);

  XtVaSetValues (PDrawTool_Atm_N_PB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_Atm_C_PB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     0,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    50,
    NULL);

  XtVaSetValues (PDrawTool_Atm_O_PB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        PDrawTool_Atm_N_PB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     PDrawTool_Atm_N_PB_Get (&GPDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     50,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    100,
    NULL);

  XtVaSetValues (PDrawTool_Atm_S_PB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_Atm_N_PB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     0,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    50,
    NULL);

  XtVaSetValues (PDrawTool_Atm_P_PB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        PDrawTool_Atm_S_PB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     PDrawTool_Atm_S_PB_Get (&GPDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     50,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    100,
    NULL);

  XtVaSetValues (PDrawTool_Atm_Cl_PB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_Atm_S_PB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     0,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    50,
    NULL);

  XtVaSetValues (PDrawTool_Atm_F_PB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        PDrawTool_Atm_Cl_PB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     PDrawTool_Atm_Cl_PB_Get (&GPDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     50,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    100,
    NULL);

  XtVaSetValues (PDrawTool_Atm_Br_PB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_Atm_Cl_PB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     0,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    50,
    NULL);

  XtVaSetValues (PDrawTool_Atm_I_PB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        PDrawTool_Atm_Br_PB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     PDrawTool_Atm_Br_PB_Get (&GPDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     50,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    100,
    NULL);

  XtVaSetValues (PDrawTool_Atm_R_PB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_Atm_Br_PB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     0,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    50,
    NULL);

  XtVaSetValues (PDrawTool_Atm_Rp_PB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        PDrawTool_Atm_R_PB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     PDrawTool_Atm_R_PB_Get (&GPDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     50,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    100,
    NULL);

  XtVaSetValues (PDrawTool_Atm_Dot_PB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_Atm_R_PB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     0,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    50,
    NULL);

  XtVaSetValues (PDrawTool_Atm_Cln_PB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        PDrawTool_Atm_Dot_PB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     PDrawTool_Atm_Dot_PB_Get (&GPDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     50,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    100,
    NULL);

  XtVaSetValues (PDrawTool_Atm_X_PB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_Atm_Dot_PB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     0,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    50,
    NULL);

  XtVaSetValues (PDrawTool_Atm_Other_PB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_Atm_Cln_PB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     PDrawTool_Atm_X_PB_Get (&GPDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       PDrawTool_Atm_X_PB_Get (&GPDrawToolCB),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (PDrawTool_Atm_Ch_PB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_Atm_X_PB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  /*  Construct the bond symbol subform.  */
  PDrawTool_BondForm_Put (&GPDrawToolCB, XtVaCreateWidget ("PDrawBondFm",
    xmFormWidgetClass,  form, 
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       False,
    NULL));

  lbl_str = XmStringCreateLocalized (SBD_PBOND_LBL);  
  PDrawTool_BndLftLbl_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawBondLbl",  
      xmLabelWidgetClass,  PDrawTool_BondForm_Get (&GPDrawToolCB),  
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,    False,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  sprintf (bitmap_file, "%s/%s", SAR_DIR_BITMAPS (""), CS_PSNGL_BOND);
  pixmap = XmGetPixmap (Screen_TScreen_Get (SynAppR_ScrnAtrb_Get (&GSynAppR)), 
    bitmap_file, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK), 
    SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
  PDrawTool_BndLSglPB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawBondPB",  
      xmPushButtonWidgetClass, PDrawTool_BondForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmPIXMAP,  
      XmNlabelPixmap,          pixmap,
      NULL));  
  XtAddCallback (PDrawTool_BndLSglPB_Get (&GPDrawToolCB),
     XmNactivateCallback, PSelected_Bond_Draw_CB, (XtPointer) DSP_BOND_SINGLE);
  /*
  XmDestroyPixmap (Screen_TScreen_Get (SynAppR_ScrnAtrb_Get (&GSynAppR)), 
    pixmap);
   */
  sprintf (bitmap_file, "%s/%s", SAR_DIR_BITMAPS (""), CS_PDUBL_BOND);
  pixmap = XmGetPixmap (Screen_TScreen_Get (SynAppR_ScrnAtrb_Get (&GSynAppR)), 
    bitmap_file, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK), 
    SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
  PDrawTool_BndLDblPB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawBondPB",  
      xmPushButtonWidgetClass, PDrawTool_BondForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmPIXMAP,  
      XmNlabelPixmap,          pixmap,
      NULL));  
  XtAddCallback (PDrawTool_BndLDblPB_Get (&GPDrawToolCB),
     XmNactivateCallback, PSelected_Bond_Draw_CB, (XtPointer) DSP_BOND_DOUBLE);
  /*
  XmDestroyPixmap (Screen_TScreen_Get (SynAppR_ScrnAtrb_Get (&GSynAppR)), 
    pixmap);
   */
 
  sprintf (bitmap_file, "%s/%s", SAR_DIR_BITMAPS (""), CS_PTRPL_BOND);
  pixmap = XmGetPixmap (Screen_TScreen_Get (SynAppR_ScrnAtrb_Get (&GSynAppR)), 
    bitmap_file, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK), 
    SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
  PDrawTool_BndLTplPB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawBondPB",  
      xmPushButtonWidgetClass, PDrawTool_BondForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmPIXMAP,  
      XmNlabelPixmap,          pixmap,
      NULL));  
  XtAddCallback (PDrawTool_BndLTplPB_Get (&GPDrawToolCB),
     XmNactivateCallback, PSelected_Bond_Draw_CB, (XtPointer) DSP_BOND_TRIPLE);
  /*
  XmDestroyPixmap (Screen_TScreen_Get (SynAppR_ScrnAtrb_Get (&GSynAppR)), 
    pixmap);
   */

  sprintf (bitmap_file, "%s/%s", SAR_DIR_BITMAPS (""), CS_PVARBL_BOND);
  pixmap = XmGetPixmap (Screen_TScreen_Get (SynAppR_ScrnAtrb_Get (&GSynAppR)), 
    bitmap_file, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK), 
    SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
  PDrawTool_BndLVblPB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawBondPB",  
      xmPushButtonWidgetClass, PDrawTool_BondForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmPIXMAP,  
      XmNlabelPixmap,          pixmap,
      NULL));  
  XtAddCallback (PDrawTool_BndLVblPB_Get (&GPDrawToolCB),
     XmNactivateCallback, PSelected_Bond_Draw_CB, (XtPointer)DSP_BOND_VARIABLE);

  sprintf (bitmap_file, "%s/%s", SAR_DIR_BITMAPS (""), CS_PRSDNT_BOND);
  pixmap = XmGetPixmap (Screen_TScreen_Get (SynAppR_ScrnAtrb_Get (&GSynAppR)), 
    bitmap_file, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK), 
    SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
  PDrawTool_BndLResPB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawBondPB",  
      xmPushButtonWidgetClass, PDrawTool_BondForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmPIXMAP,  
      XmNlabelPixmap,          pixmap,
      NULL));  
  XtAddCallback (PDrawTool_BndLResPB_Get (&GPDrawToolCB),
     XmNactivateCallback, PSelected_Bond_Draw_CB, (XtPointer)DSP_BOND_RESONANT);

  /*  Construct the root-syntheme subform.  */
  PDrawTool_RootSynForm_Put (&GPDrawToolCB, XtVaCreateWidget ("PDrawRootSynFm",
    xmFormWidgetClass,  form, 
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       False,
    NULL));

  lbl_str = XmStringCreateLocalized (SBD_PROOTSYN_LBL);  
  PDrawTool_RootSynLbl_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawRootSynLbl",  
      xmLabelWidgetClass,  PDrawTool_RootSynForm_Get (&GPDrawToolCB),  
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,    False,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  PDrawTool_RootSynArea_Put (&GPDrawToolCB,
    XtVaCreateManagedWidget ("PDrawToolRSDA",
      xmDrawingAreaWidgetClass,
                            PDrawTool_RootSynForm_Get (&GPDrawToolCB),
      XmNbackground,        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
      XmNforeground,        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
      XmNheight,            175,
      XmNwidth,             150,
      NULL));
  XtAddCallback (PDrawTool_RootSynArea_Get (&GPDrawToolCB),
    XmNexposeCallback, redisplay_roots, NULL);

  XtVaSetValues (PDrawTool_BndLftLbl_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (PDrawTool_BndLSglPB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_BndLftLbl_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     1,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    20,
    NULL);

  XtVaSetValues (PDrawTool_BndLDblPB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        PDrawTool_BndLSglPB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     PDrawTool_BndLSglPB_Get (&GPDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     20,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    40,
    NULL);

  XtVaSetValues (PDrawTool_BndLTplPB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        PDrawTool_BndLDblPB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     PDrawTool_BndLDblPB_Get (&GPDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     40,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    60,
    NULL);

  XtVaSetValues (PDrawTool_BndLVblPB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        PDrawTool_BndLTplPB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     PDrawTool_BndLTplPB_Get (&GPDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     60,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    80,
    NULL);

  XtVaSetValues (PDrawTool_BndLResPB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        PDrawTool_BndLVblPB_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     PDrawTool_BndLVblPB_Get (&GPDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     80,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    100,
    NULL);

  separator1 = XtVaCreateManagedWidget("PDrawToolSep",  
    xmSeparatorWidgetClass, form, 
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmSHADOW_ETCHED_IN,
    XmNheight,              AppDim_SepSmall_Get (&GAppDim),
    NULL);

  separator2 = XtVaCreateManagedWidget("PDrawToolSep",  
    xmSeparatorWidgetClass, form, 
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmSHADOW_ETCHED_IN,
    XmNheight,              AppDim_SepSmall_Get (&GAppDim),
    NULL);

  separator3 = XtVaCreateManagedWidget("PDrawToolSep",  
    xmSeparatorWidgetClass, form, 
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmSHADOW_ETCHED_IN,
    XmNheight,              AppDim_SepSmall_Get (&GAppDim),
    NULL);

  separator4 = XtVaCreateManagedWidget("PDrawToolSep",  
    xmSeparatorWidgetClass, form, 
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmSHADOW_ETCHED_IN,
    XmNheight,              AppDim_SepSmall_Get (&GAppDim),
    NULL);

/*
  separator5 = XtVaCreateManagedWidget("PDrawToolSep",  
    xmSeparatorWidgetClass, form, 
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmSHADOW_ETCHED_IN,
    XmNheight,              AppDim_SepSmall_Get (&GAppDim),
    NULL);
*/

  /*  Construct the drawing area subform.  */
  PDrawTool_DrawForm_Put (&GPDrawToolCB, XtVaCreateWidget ("PDrawDrawFm",
    xmFormWidgetClass,  form, 
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       False,
    XmNfractionBase,    200,
    NULL));

  lbl_strs[GOAL] = XmStringCreateLocalized (SBD_PGOAL_LBL);  
  lbl_strs[SUBG] = XmStringCreateLocalized (SBD_PSGOAL_LBL);  
  for (pattern = 0; pattern < 2; pattern++)
    {
    PDrawTool_PattLbl_Put (&GPDrawToolCB, pattern,
      XtVaCreateManagedWidget ("PDrawModeLbl",  
        xmLabelWidgetClass,  PDrawTool_DrawForm_Get (&GPDrawToolCB),
        XmNlabelType,        XmSTRING,  
        XmNlabelString,      lbl_strs[pattern],   
        XmNalignment,        XmALIGNMENT_BEGINNING,
        XmNrecomputeSize,    False,
        XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
        XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
        NULL));
    XmStringFree (lbl_strs[pattern]);
    }

  separator[GOAL] = XtVaCreateManagedWidget("PDrawToolSep",  
    xmSeparatorWidgetClass, PDrawTool_DrawForm_Get (&GPDrawToolCB),
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmSHADOW_ETCHED_IN,
    XmNheight,              AppDim_SepSmall_Get (&GAppDim),
    NULL);

  separator[SUBG] = XtVaCreateManagedWidget("PDrawToolSep",  
    xmSeparatorWidgetClass, form,
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmDOUBLE_LINE,
    XmNheight,              AppDim_SepLarge_Get (&GAppDim),
    NULL);

  PDrawTool_PBForm_Put (&GPDrawToolCB, XtVaCreateWidget ("PDrawEditFm",
    xmFormWidgetClass,  form, 
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       False,
    NULL));

  lbl_str = XmStringCreateLocalized (CS_PHELP);  
  PDrawTool_HelpPB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawToolPB",  
      xmPushButtonWidgetClass, PDrawTool_PBForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      XmNfontList,             flhv12,
      XmNheight,               AppDim_HtLblPB_Get (&GAppDim),
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_HelpPB_Get (&GPDrawToolCB),
     XmNactivateCallback, Help_CB, (XtPointer) "rxnpatt_draw:Reaction Transform Editor\\rxnpatt_draw2:IsThere Fragment Editor");

  lbl_str = XmStringCreateLocalized (CS_PDONE);  
  PDrawTool_DonePB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawToolPB",  
      xmPushButtonWidgetClass, PDrawTool_PBForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      XmNfontList,             flhv12,
      XmNheight,               AppDim_HtLblPB_Get (&GAppDim),
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_DonePB_Get (&GPDrawToolCB),
     XmNactivateCallback, PDone_CB, (XtPointer) rxninfo_p);

  lbl_str = XmStringCreateLocalized (CS_PQUIT);  
  PDrawTool_QuitPB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawToolPB",  
      xmPushButtonWidgetClass, PDrawTool_PBForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      XmNfontList,             flhv12,
      XmNheight,               AppDim_HtLblPB_Get (&GAppDim),
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_QuitPB_Get (&GPDrawToolCB),
     XmNactivateCallback, PQuit_CB, (XtPointer) rxninfo_p);

  lbl_str = XmStringCreateLocalized (CS_PISOLATE);  
  PDrawTool_IsolatePB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawToolPB",  
      xmPushButtonWidgetClass, PDrawTool_PBForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_IsolatePB_Get (&GPDrawToolCB),
     XmNactivateCallback, PIsolate_CB, NULL);

  lbl_str = XmStringCreateLocalized (CS_PNUMTOGL);  
  PDrawTool_NumToglPB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawToolPB",  
      xmPushButtonWidgetClass, PDrawTool_PBForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_NumToglPB_Get (&GPDrawToolCB),
     XmNactivateCallback, PNumTogl_CB, NULL);

if (IsThere_Draw_Flag && glob_rxlform) XtUnmanageChild (PDrawTool_NumToglPB_Get (&GPDrawToolCB));

  lbl_str = XmStringCreateLocalized (CS_PREDRAW);  
  PDrawTool_RedrawPB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawToolPB",  
      xmPushButtonWidgetClass, PDrawTool_PBForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_RedrawPB_Get (&GPDrawToolCB),
     XmNactivateCallback, POption_Choose_CB, (XtPointer) PDRW_REDRAW_FLAG);

  lbl_str = XmStringCreateLocalized (CS_PCLEARALL);  
  PDrawTool_ClearPB_Put (&GPDrawToolCB, 
    XtVaCreateManagedWidget ("PDrawToolPB",  
      xmPushButtonWidgetClass, PDrawTool_PBForm_Get (&GPDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (PDrawTool_ClearPB_Get (&GPDrawToolCB),
     XmNactivateCallback, PMenu_Choice_CB, (XtPointer) PDRW_MENU_DELETE_ALL);

  XtVaSetValues (PDrawTool_HelpPB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     2,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    13,
    NULL);

  XtVaSetValues (PDrawTool_DonePB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     16,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    27,
    NULL);

  XtVaSetValues (PDrawTool_QuitPB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     30,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    41,
    NULL);

  XtVaSetValues (PDrawTool_IsolatePB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     44,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    55,
    NULL);

  XtVaSetValues (PDrawTool_NumToglPB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     58,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    69,
    NULL);

  XtVaSetValues (PDrawTool_RedrawPB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     72,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    83,
    NULL);

  XtVaSetValues (PDrawTool_ClearPB_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     86,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    97,
    NULL);

  for (pattern=0; pattern<2; pattern++)
    PDrawTool_ScrlldWin_Put (&GPDrawToolCB, pattern,
      XtVaCreateManagedWidget
       (pattern==GOAL?"PDrawToolGPSW":"PDrawToolSPSW",
        xmScrolledWindowWidgetClass, PDrawTool_DrawForm_Get (&GPDrawToolCB),
        XmNscrollingPolicy,          XmAUTOMATIC,
        XmNscrollBarDisplayPolicy,   XmAS_NEEDED,
        NULL));

  /*  Add the "draw" action/function used by the translation table
   *  parsed by the translations resource below.
   */
  draw_actions.string = "PDraw_Draw";
  draw_actions.proc = (XtActionProc) PDraw_Draw;
  XtAppAddActions (*appcon, &draw_actions, 1);

  for (pattern=0; pattern<2; pattern++)
    {
    PDrawTool_DrawArea_Put (&GPDrawToolCB, pattern, XtVaCreateManagedWidget
     (pattern==GOAL?"PDrawToolGPDA":"PDrawToolSPDA",
      xmDrawingAreaWidgetClass, PDrawTool_ScrlldWin_Get (&GPDrawToolCB,pattern),
      XmNtranslations,      XtParseTranslationTable (translations),
      XmNbackground,        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
      XmNforeground,        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
      XmNheight,            PDRW_DA_PXMP_HT,
      XmNwidth,             PDRW_DA_PXMP_WD,
      NULL));
    XtAddCallback (PDrawTool_DrawArea_Get (&GPDrawToolCB, pattern),
      XmNexposeCallback, 
      predraw, (XtPointer)(pattern==GOAL?&Goal:&Subgoal));
    XtAddEventHandler (PDrawTool_DrawArea_Get (&GPDrawToolCB, pattern),
      LeaveWindowMask, FALSE, pleave_drawarea,
      (XtPointer)(pattern==GOAL?&Goal:&Subgoal));
    XtAddEventHandler (PDrawTool_DrawArea_Get (&GPDrawToolCB, pattern),
      EnterWindowMask, FALSE, penter_drawarea,
      (XtPointer)(pattern==GOAL?&Goal:&Subgoal));
    }

  XtVaSetValues (separator1,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget,     PDrawTool_EditForm_Get (&GPDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       PDrawTool_EditForm_Get (&GPDrawToolCB),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
    XmNrightWidget,      PDrawTool_EditForm_Get (&GPDrawToolCB),
    NULL);

  XtVaSetValues (PDrawTool_EditForm_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_NONE,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      -(AppDim_SbtPanel_Wd_Get (&GAppDim)),
    XmNrightAttachment,  XmATTACH_OPPOSITE_FORM,
    NULL);

  XtVaSetValues (separator2,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_EditForm_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       PDrawTool_EditForm_Get (&GPDrawToolCB),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
    XmNrightWidget,      PDrawTool_EditForm_Get (&GPDrawToolCB),
    NULL);

  XtVaSetValues (PDrawTool_AtomForm_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        separator2,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       PDrawTool_EditForm_Get (&GPDrawToolCB),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
    XmNrightWidget,      PDrawTool_EditForm_Get (&GPDrawToolCB),
    NULL);

  XtVaSetValues (separator3,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_AtomForm_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       PDrawTool_EditForm_Get (&GPDrawToolCB),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
    XmNrightWidget,      PDrawTool_EditForm_Get (&GPDrawToolCB),
    NULL);

  XtVaSetValues (PDrawTool_BondForm_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        separator3,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       PDrawTool_EditForm_Get (&GPDrawToolCB),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
    XmNrightWidget,      PDrawTool_EditForm_Get (&GPDrawToolCB),
    NULL);

  XtVaSetValues (separator4,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_BondForm_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       PDrawTool_EditForm_Get (&GPDrawToolCB),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
    XmNrightWidget,      PDrawTool_EditForm_Get (&GPDrawToolCB),
    NULL);

  XtVaSetValues (PDrawTool_RootSynForm_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        separator4,
    XmNbottomOffset,     0,
/*
    XmNbottomAttachment, XmATTACH_NONE,
*/
    XmNbottomAttachment, XmATTACH_WIDGET,
/*
    XmNbottomWidget,     PDrawTool_PBForm_Get (&GPDrawToolCB),
*/
    XmNbottomWidget,     separator[SUBG],
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       PDrawTool_BondForm_Get (&GPDrawToolCB),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
    XmNrightWidget,      PDrawTool_BondForm_Get (&GPDrawToolCB),
    NULL);

/*
  XtVaSetValues (separator5,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_RootSynForm_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget,     PDrawTool_PBForm_Get (&GPDrawToolCB),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     separator[SUBG],
    XmNbottomWidget,     PDrawTool_DrawForm_Get (&GPDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       PDrawTool_BondForm_Get (&GPDrawToolCB),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
    XmNrightWidget,      PDrawTool_BondForm_Get (&GPDrawToolCB),
    NULL);
*/

  XtVaSetValues (PDrawTool_RootSynLbl_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (PDrawTool_RootSynArea_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_RootSynLbl_Get (&GPDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (PDrawTool_PBForm_Get (&GPDrawToolCB),
/*
Get rid of obscure complicating stuff ...
    XmNtopOffset,        -(AppDim_HtLblPB_Get (&GAppDim) 
                           + 2 * AppDim_MargFmFr_Get (&GAppDim)),
    XmNtopAttachment,    XmATTACH_OPPOSITE_FORM,
*/
    XmNtopPosition,      190,
    XmNtopAttachment,    XmATTACH_POSITION,
    XmNbottomOffset,     AppDim_MargFmFr_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
/*
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       PDrawTool_RootSynForm_Get (&GPDrawToolCB),
    XmNleftWidget,       PDrawTool_DrawForm_Get (&GPDrawToolCB),
*/
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (PDrawTool_DrawForm_Get (&GPDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget,     PDrawTool_PBForm_Get (&GPDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       PDrawTool_EditForm_Get (&GPDrawToolCB),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (separator[SUBG],
/*
    XmNtopOffset,        -(AppDim_HtLblPB_Get (&GAppDim) 
                           + 3 * AppDim_MargFmFr_Get (&GAppDim)
                           + AppDim_SepLarge_Get (&GAppDim)),
    XmNtopAttachment,    XmATTACH_OPPOSITE_FORM,
*/
/*
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_DrawForm_Get (&GPDrawToolCB),
*/
    XmNtopPosition,      185,
    XmNtopAttachment,    XmATTACH_POSITION,
    XmNbottomOffset,     AppDim_MargFmFr_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget,     PDrawTool_PBForm_Get (&GPDrawToolCB),
/*
    XmNbottomAttachment, XmATTACH_FORM,
*/
    XmNleftOffset,       0,
/*
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       PDrawTool_BondForm_Get (&GPDrawToolCB),
*/
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (PDrawTool_PattLbl_Get (&GPDrawToolCB, GOAL),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
/*
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       PDrawTool_EditForm_Get (&GPDrawToolCB),
*/
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (separator[GOAL],
    XmNtopPosition,      /* 95 */ 100,
    XmNtopAttachment,    XmATTACH_POSITION,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
/*
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       PDrawTool_BondForm_Get (&GPDrawToolCB),
*/
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (PDrawTool_ScrlldWin_Get (&GPDrawToolCB, GOAL),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_PattLbl_Get (&GPDrawToolCB, GOAL),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget,     separator[GOAL],
    XmNleftOffset,       0,
/*
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       PDrawTool_EditForm_Get (&GPDrawToolCB),
*/
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (PDrawTool_PattLbl_Get (&GPDrawToolCB, SUBG),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        separator[GOAL],
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
/*
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       PDrawTool_BondForm_Get (&GPDrawToolCB),
*/
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (PDrawTool_ScrlldWin_Get (&GPDrawToolCB, SUBG),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        PDrawTool_PattLbl_Get (&GPDrawToolCB, SUBG),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
/*
    XmNbottomWidget,     separator[SUBG],
*/
    XmNleftOffset,       0,
/*
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       PDrawTool_BondForm_Get (&GPDrawToolCB),
*/
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  /* Manage Unmanaged children */    
  XtManageChild (PDrawTool_EditModeRB_Get (&GPDrawToolCB));
  XtManageChild (PDrawTool_EditForm_Get (&GPDrawToolCB));
  XtManageChild (PDrawTool_AtomForm_Get (&GPDrawToolCB));
  XtManageChild (PDrawTool_BondForm_Get (&GPDrawToolCB));
  XtManageChild (PDrawTool_PBForm_Get (&GPDrawToolCB));
  XtManageChild (PDrawTool_RootSynForm_Get (&GPDrawToolCB));
  XtManageChild (PDrawTool_DrawForm_Get (&GPDrawToolCB));

  XtManageChild (form);

  /* Create the periodic table dialog */ 
  PPeriodic_TblDlg_Create (parent);

/* Create syntheme list */
  SynList_Create (parent);

/* Create syntheme list - no longer needed
  Slings_Create (parent);
*/

  /*  Create a GC and Pixmap for the drawing area.  */
  sca_p = SynAppR_ScrnAtrb_Get (&GSynAppR);
  display = Screen_TDisplay_Get (sca_p);
  gc = XCreateGC (display, Screen_RootWin_Get (sca_p), 0, NULL);
  XSetBackground (display, gc, SynAppR_IthClrPx_Get (&GSynAppR, 
    SAR_CLRI_WHITE));
  XSetForeground (display, gc, SynAppR_IthClrPx_Get (&GSynAppR, 
    SAR_CLRI_WHITE));
  XSetFillStyle (display, gc, FillSolid);
  XSetLineAttributes (display, gc, 1, LineSolid, CapButt, JoinMiter);
  XSetFont (display, gc, SynAppR_IthFont_Get (&GSynAppR, 
    SAR_FONTSTRCTS_NRML)->fid);
  PDrawTool_GC_Put (&GPDrawToolCB, gc);

  for (pattern=0; pattern<2; pattern++)
    {
    PDrawTool_Pixmap_Put (&GPDrawToolCB, pattern, XCreatePixmap (display,
      Screen_RootWin_Get (sca_p), PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 
      Screen_Depth_Get (sca_p)));

    XFillRectangle (display, PDrawTool_Pixmap_Get (&GPDrawToolCB, pattern),
      gc, 0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT);
    }

  XSetForeground (display, gc, SynAppR_IthClrPx_Get (&GSynAppR, 
    SAR_CLRI_BLACK));
 
  XtManageChild(PDrawTool_Frame_Get (&GPDrawToolCB));

  if (IsThere_Draw_Flag && glob_rxlform)
    {
lbl_drt = XmStringCreateLocalized ("");
    XtManageChild (PDrawTool_DonePB_Get (&GPDrawToolCB));
    XtManageChild (PDrawTool_ClearPB_Get (&GPDrawToolCB));
    XtUnmanageChild (PDrawTool_IsolatePB_Get (&GPDrawToolCB));
/**/
    XtUnmanageChild (PDrawTool_NumToglPB_Get (&GPDrawToolCB));
/**/
/*
    XtUnmanageChild (XtNameToWidget (PDrawTool_EditModeRB_Get (&GPDrawToolCB), "button_4"));
*/
XtVaSetValues(XtNameToWidget (PDrawTool_EditModeRB_Get (&GPDrawToolCB), "button_4"),
XmNvisibleWhenOff, False,
XmNlabelString, lbl_drt,
NULL);
    XtUnmanageChild (PDrawTool_RootSynForm_Get (&GPDrawToolCB));
    lbl_srt = XmStringCreateLocalized (CS_PADDFGS); 
    lbl_str = XmStringCreateLocalized ("IsThere Fragment Editor");
    lbl_done = XmStringCreateLocalized ("Exit; start search");
    }
  else if (glob_rxlform)
    {
lbl_drt = XmStringCreateLocalized (CS_PDELETEROOT);
    XtManageChild (PDrawTool_DonePB_Get (&GPDrawToolCB));
    XtManageChild (PDrawTool_ClearPB_Get (&GPDrawToolCB));
    XtManageChild (PDrawTool_IsolatePB_Get (&GPDrawToolCB));
/**/
    XtManageChild (PDrawTool_NumToglPB_Get (&GPDrawToolCB));
/**/
/*
    XtManageChild (XtNameToWidget (PDrawTool_EditModeRB_Get (&GPDrawToolCB), "button_4"));
*/
XtVaSetValues(XtNameToWidget (PDrawTool_EditModeRB_Get (&GPDrawToolCB), "button_4"),
XmNvisibleWhenOff, True,
XmNlabelString, lbl_drt,
NULL);
    XtManageChild (PDrawTool_RootSynForm_Get (&GPDrawToolCB));
    lbl_srt = XmStringCreateLocalized (CS_PSELECTROOT); 
    lbl_str = XmStringCreateLocalized ("Transform Pattern Editor");
    lbl_done = XmStringCreateLocalized (CS_PDONE);
    }
  else
    {
lbl_drt = XmStringCreateLocalized (CS_PDELETEROOT);
    XtUnmanageChild (PDrawTool_DonePB_Get (&GPDrawToolCB));
    XtUnmanageChild (PDrawTool_ClearPB_Get (&GPDrawToolCB));
    XtUnmanageChild (PDrawTool_IsolatePB_Get (&GPDrawToolCB));
/**/
    XtManageChild (PDrawTool_NumToglPB_Get (&GPDrawToolCB));
/**/
/*
    XtManageChild (XtNameToWidget (PDrawTool_EditModeRB_Get (&GPDrawToolCB), "button_4"));
*/
XtVaSetValues(XtNameToWidget (PDrawTool_EditModeRB_Get (&GPDrawToolCB), "button_4"),
XmNvisibleWhenOff, True,
XmNlabelString, lbl_drt,
NULL);
    XtManageChild (PDrawTool_RootSynForm_Get (&GPDrawToolCB));
    lbl_srt = XmStringCreateLocalized (CS_PSELECTROOT); 
    lbl_str = XmStringCreateLocalized ("Transform Pattern Viewer");
    lbl_done = XmStringCreateLocalized (CS_PDONE);
    }

  XmStringFree (lbl_drt);

  XtVaSetValues (XtNameToWidget (PDrawTool_EditModeRB_Get (&GPDrawToolCB), "button_2"),
    XmNlabelString, lbl_srt,
    NULL);
  XmStringFree (lbl_srt);

  XtVaSetValues (PDrawTool_Frame_Get (&GPDrawToolCB),
    XmNdialogTitle, lbl_str,
    NULL);
  XmStringFree (lbl_str);

  XtVaSetValues (PDrawTool_DonePB_Get (&GPDrawToolCB),
    XmNlabelString, lbl_done,
    NULL);
  XmStringFree (lbl_done);

  if (!gsgsame)
    {
    for (pattern = 0; pattern < 2; pattern++)
      {
      XtVaGetValues (PDrawTool_ScrlldWin_Get (&GPDrawToolCB, pattern),
        XmNwidth, &sw_w,
        XmNheight, &sw_h,
        NULL);

      pdraw_Molecule (display, 
        XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pattern)), gc,
        PDrawMol_p, AppDim_MolScale_Get (&GAppDim), sw_h, sw_w, pattern,
        pattern == GOAL);
      pdraw_Molecule (display, PDrawTool_Pixmap_Get (&GPDrawToolCB, pattern),
        gc, PDrawMol_p, 1.0, 0, 0, pattern, FALSE);
      }
    draw_roots (display,
      XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, GOAL)),
      PDrawTool_Pixmap_Get (&GPDrawToolCB, GOAL), gc, PDrawMol_p);
    }

  return (PDrawTool_Frame_Get (&GPDrawToolCB));
}
/*  End of PDraw_Tool_Create  */


/****************************************************************************
*  
*  Function Name:                 PDraw_Mode_Reset
*  
*****************************************************************************/  

void PDraw_Mode_Reset
  (
  int            which_mode
  )
{
  WidgetList     toggles;

  /** Get the run type distributed or sequential **/
  XtVaGetValues (PDrawTool_EditModeRB_Get (&GPDrawToolCB),
    XmNchildren,    &toggles,
    NULL);

  XmToggleButtonSetState (toggles[which_mode], True, True);

  return;

}
/*  End of PDraw_Mode_Reset  */


/****************************************************************************
*  
*  Function Name:                 Slings_Create
*  
*****************************************************************************/  

void Slings_Create
  (
  Widget      parent
  )
{
  XmString          lbl_str;
  XFontStruct      *fs8x12;
  XmFontList        fl8x12;
 
  PDrawTool_SlingListForm_Put (&GPDrawToolCB, XmCreateFormDialog 
    (parent, "SlingsFmDg", NULL, 0));

  lbl_str = XmStringCreateLocalized ("Syntheme Slings");
    XtVaSetValues (PDrawTool_SlingListForm_Get (&GPDrawToolCB),
      XmNdialogTitle,  lbl_str,
      XmNdialogStyle,  XmDIALOG_MODELESS,
      XmNautoUnmanage, False,
      NULL);
  XmStringFree (lbl_str);

  fs8x12 = XLoadQueryFont (PDrawDisplay, "8x12");
  fl8x12 = XmFontListCreate (fs8x12, XmSTRING_DEFAULT_CHARSET);

  PDrawTool_SlingList_Put (&GPDrawToolCB, XmCreateScrolledList 
    (PDrawTool_SlingListForm_Get (&GPDrawToolCB), "SlingList", NULL, 0));

  XtVaSetValues (PDrawTool_SlingList_Get (&GPDrawToolCB),
      XmNbackground,               SynAppR_IthClrPx_Get(&GSynAppR,
                                                        SAR_CLRI_WHITE),
      XmNforeground,               SynAppR_IthClrPx_Get(&GSynAppR,
                                                        SAR_CLRI_BLACK),
      XmNfontList,                 fl8x12,
      XmNselectionPolicy,          XmSINGLE_SELECT,
      XmNwidth,                    200,
      XmNheight,                   600,
      XmNx,                        100,
      XmNy,                        100,
      NULL);

  XtAddCallback(PDrawTool_SlingList_Get (&GPDrawToolCB),
    XmNsingleSelectionCallback,Sling_Select_CB,NULL);

  XtManageChild (PDrawTool_SlingList_Get (&GPDrawToolCB));
}

/* end of Slings_Create */

/****************************************************************************
*  
*  Function Name:                 SynList_Create
*  
*****************************************************************************/  

void SynList_Create 
  (
  Widget      parent
  )
{
  XFontStruct      *fs6x10;
  XmFontList        fl6x10;
  char              bitmap_file[256], fg_name[256], *sling;
  int               pattern, fg_num, fgrec_num, fglist_pos, name_inx, sl_inx;
  FuncGrp_Record_t *fgrec;
  Isam_Control_t   *fgfile;
  XmString          lbl_str;
 
/*
  PDrawTool_SynListForm_Put (&GPDrawToolCB, XmCreateFormDialog 
    (parent, "SynListFmDg", NULL, 0));

  lbl_str = XmStringCreateLocalized ("Select Syntheme");
    XtVaSetValues (PDrawTool_SynListForm_Get (&GPDrawToolCB),
      XmNdialogTitle,  lbl_str,
      XmNdialogStyle,  XmDIALOG_MODELESS,
      XmNautoUnmanage, False,
      NULL);
  XmStringFree (lbl_str);

  fs6x10 = XLoadQueryFont (PDrawDisplay, "6x10");
  fl6x10 = XmFontListCreate (fs6x10, XmSTRING_DEFAULT_CHARSET);

  PDrawTool_SynList_Put (&GPDrawToolCB, XmCreateScrolledList 
    (PDrawTool_SynListForm_Get (&GPDrawToolCB), "SynList", NULL, 0));

  XtVaSetValues (PDrawTool_SynList_Get (&GPDrawToolCB),
      XmNbackground,               SynAppR_IthClrPx_Get(&GSynAppR,
                                                        SAR_CLRI_WHITE),
      XmNforeground,               SynAppR_IthClrPx_Get(&GSynAppR,
                                                        SAR_CLRI_BLACK),
      XmNfontList,                 fl6x10,
      XmNselectionPolicy,          XmSINGLE_SELECT,
      XmNwidth,                    200,
      XmNheight,                   600,
      XmNx,                        100,
      XmNy,                        100,
      NULL);

  XtAddCallback(PDrawTool_SynList_Get (&GPDrawToolCB),
    XmNsingleSelectionCallback,Syntheme_Store_CB,NULL);
*/

  /* Fill in the syntheme list */

/*
  XmListSetAddMode (PDrawTool_SynList_Get (&GPDrawToolCB), TRUE);
*/

  fgfile = (Isam_Control_t *) malloc (ISAMCONTROLSIZE);
  if (fgfile == NULL) IO_Exit_Error (R_AVL, X_SYSCALL, 
    "Unable to allocate memory for Isam Control Block.");
  strcpy (IO_FileName_Get (Isam_File_Get (fgfile)), FGFILE);
  Isam_Open (fgfile, ISAM_TYPE_FGINFO, ISAM_OPEN_READ);

  fgrec_num = fglist_pos = 1;
  while((fgrec = FuncGrp_Rec_Read (fgrec_num, fgfile)) != NULL
    && (fg_num = FuncGrp_Head_FGNum_Get (FuncGrp_Rec_Head_Get (fgrec))) <= 40)
    {
    for (name_inx = 0;
         name_inx < FuncGrp_Head_NumNames_Get (FuncGrp_Rec_Head_Get (fgrec));
         name_inx++, fglist_pos++)
      {
/*
      PDrawTool_SynListNum_Put (&GPDrawToolCB, fglist_pos, fg_num);
      if (name_inx == 0)
        sprintf(fg_name, "%2d: %s", fg_num,
          String_Value_Get (FuncGrp_Rec_Name_Get (fgrec, name_inx)));
      else
        sprintf(fg_name, "    %s",
          String_Value_Get (FuncGrp_Rec_Name_Get (fgrec, name_inx)));
      lbl_str = XmStringCreate (fg_name, XmSTRING_DEFAULT_CHARSET);
      XmListAddItem (PDrawTool_SynList_Get (&GPDrawToolCB), lbl_str, 0);
      XmStringFree (lbl_str);
*/

      if (name_inx == 0)
        {
        sling =
          (char *) malloc
          (100 * FuncGrp_Head_NumSlings_Get (FuncGrp_Rec_Head_Get(fgrec)));
        sling[0]=0;
        for (sl_inx = 0;
             sl_inx < FuncGrp_Head_NumSlings_Get (FuncGrp_Rec_Head_Get(fgrec));
             sl_inx++)
          {
          strcat(sling, Sling_Name_Get (FuncGrp_Rec_Sling_Get (fgrec, sl_inx)));
          strcat(sling,"\\");
          }
/*
        GPDrawToolCB.slings[fgrec_num] = sling;
*/
        GPDrawToolCB.slings[FuncGrp_Head_FGNum_Get (FuncGrp_Rec_Head_Get (fgrec))] = sling;
        }
/*
      GPDrawToolCB.slings[fglist_pos] = sling;
*/
      }
    fgrec_num++;
    }

  Isam_Close (fgfile);
  free (fgfile);

/*
  XmListSetAddMode (PDrawTool_SynList_Get (&GPDrawToolCB), FALSE);

  XmListSetBottomPos (PDrawTool_SynList_Get (&GPDrawToolCB), fglist_pos - 1);

  XmListSetPos (PDrawTool_SynList_Get (&GPDrawToolCB), 1);
*/

  /* This should not be necessary, because XmListSetBottomPos should be
     setting the value automatically, but it isn't working here - go figure! */

/*
  XtVaSetValues(PDrawTool_SynList_Get (&GPDrawToolCB),
    XmNvisibleItemCount,    fglist_pos - 1,
    NULL);

  XtManageChild (PDrawTool_SynList_Get (&GPDrawToolCB));
*/
}

/* end of SynList_Create */

/****************************************************************************
*  
*  Function Name:                 PPeriodic_TblDlg_Create
*  
*****************************************************************************/  

void PPeriodic_TblDlg_Create 
  (
  Widget      parent
  )
{
  Widget    frame, separator, pb;
  Widget    H_elm_pb, He_elm_pb, Ac_elm_pb;
  Widget    Li_Ra_rc, B_Ar_rc, Sc_Rn_rc, Ce_Lw_rc;
  XmString  str;
  int       i;
 
  PDrawTool_PeriodicTbl_Put (&GPDrawToolCB, XmCreateFormDialog 
    (parent, "DrawPTblFmDg", NULL, 0));

  str = XmStringCreateLocalized ("Periodic Table");
    XtVaSetValues (PDrawTool_PeriodicTbl_Get (&GPDrawToolCB),
      XmNdialogTitle,  str,
      XmNdialogStyle,  XmDIALOG_MODELESS,
      XmNautoUnmanage, False,
      NULL);
  XmStringFree (str);

  frame = XtVaCreateManagedWidget ("PDrawPTblFr", 
    xmFrameWidgetClass, PDrawTool_PeriodicTbl_Get (&GPDrawToolCB),
    XmNtopAttachment, XmATTACH_FORM,
    XmNtopOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    NULL);

  str = XmStringCreateLocalized (CS_PELM_H);
  H_elm_pb =  XtVaCreateManagedWidget ("PDrawPTblPB", 
    xmPushButtonGadgetClass, 
    PDrawTool_PeriodicTbl_Get (&GPDrawToolCB),
    XmNlabelString, str,
    XmNtopAttachment, XmATTACH_FORM,
    XmNtopOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    NULL);
  XmStringFree (str);
  XtAddCallback (H_elm_pb, XmNactivateCallback, PAtom_Mark_CB, 
    (XtPointer) CS_PELM_H);

  str = XmStringCreateLocalized (CS_PELM_He);
  He_elm_pb =  XtVaCreateManagedWidget ("PDrawPTblPB", xmPushButtonGadgetClass, 
    PDrawTool_PeriodicTbl_Get (&GPDrawToolCB),
    XmNlabelString, str,
    XmNtopAttachment, XmATTACH_FORM,
    XmNtopOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    XmNrightOffset, 0,
    NULL);
  XmStringFree (str);
  XtAddCallback (He_elm_pb, XmNactivateCallback, PAtom_Mark_CB, 
    (XtPointer) CS_PELM_He);

  Li_Ra_rc =  XtVaCreateManagedWidget ("PDrawPTblRC", xmRowColumnWidgetClass, 
    PDrawTool_PeriodicTbl_Get (&GPDrawToolCB),
    XmNpacking, XmPACK_COLUMN,
    XmNnumColumns, 2,
    XmNmarginHeight, 0,
    XmNmarginWidth, 0,
    XmNspacing, 0, 
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopOffset, 0,
    XmNtopWidget, H_elm_pb,
    XmNleftAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    NULL);
  for(i = 0; i < XtNumber(Li_Ra_PElms); i++)
    {
   
    pb = XtVaCreateManagedWidget (Li_Ra_PElms[i], xmPushButtonGadgetClass,     
      Li_Ra_rc, NULL);

    /** callback for an element button from Li to Ra **/  
    XtAddCallback (pb, XmNactivateCallback, PAtom_Mark_CB, Li_Ra_PElms[i]);
    }

  B_Ar_rc =  XtVaCreateManagedWidget ("PDrawPTblRC", xmRowColumnWidgetClass, 
    PDrawTool_PeriodicTbl_Get (&GPDrawToolCB),
    XmNpacking, XmPACK_COLUMN,
    XmNnumColumns, 6,
    XmNmarginHeight, 0,
    XmNmarginWidth, 0,
    XmNspacing, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopOffset, 0,
    XmNtopWidget, He_elm_pb,
    XmNrightAttachment, XmATTACH_FORM,
    XmNrightOffset, 0,
    NULL);
  for(i = 0; i < XtNumber(B_Ar_PElms); i++)
    {
   
    pb = XtVaCreateManagedWidget (B_Ar_PElms[i], xmPushButtonGadgetClass,     
      B_Ar_rc, NULL);

    /** callback for an element button from B to Ar **/  
    XtAddCallback (pb, XmNactivateCallback, PAtom_Mark_CB, B_Ar_PElms[i]);
    }

  Sc_Rn_rc =  XtVaCreateManagedWidget ("PDrawPTblRC", xmRowColumnWidgetClass, 
    PDrawTool_PeriodicTbl_Get (&GPDrawToolCB),
    XmNpacking, XmPACK_COLUMN,
    XmNnumColumns, 16,
    XmNmarginHeight, 0,
    XmNmarginWidth, 0,
    XmNspacing, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopOffset, 0,
    XmNtopWidget, B_Ar_rc,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftOffset, 0,
    XmNleftWidget, Li_Ra_rc,
    NULL);
  for(i = 0; i < XtNumber(Sc_Rn_PElms); i++)
    {
   
    pb = XtVaCreateManagedWidget (Sc_Rn_PElms[i], xmPushButtonGadgetClass,     
      Sc_Rn_rc, NULL);

    /** callback for an element button from Sc to Rn **/  
    XtAddCallback (pb, XmNactivateCallback, PAtom_Mark_CB, Sc_Rn_PElms[i]);
    }

  Ce_Lw_rc =  XtVaCreateManagedWidget ("PDrawPTblRC", xmRowColumnWidgetClass, 
    PDrawTool_PeriodicTbl_Get (&GPDrawToolCB),
    XmNpacking, XmPACK_COLUMN,
    XmNnumColumns, 14,
    XmNmarginHeight, 0,
    XmNmarginWidth, 0,
    XmNspacing, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopOffset, 0,
    XmNtopWidget, Li_Ra_rc,
    XmNleftAttachment, XmATTACH_FORM,
    XmNleftOffset, 20,
    NULL);
  for(i = 0; i < XtNumber(Ce_Lw_PElms); i++)
    {
   
    pb = XtVaCreateManagedWidget (Ce_Lw_PElms[i], xmPushButtonGadgetClass,     
      Ce_Lw_rc, NULL);

    /** callback for an element button from Ce to Lw **/  
    XtAddCallback (pb, XmNactivateCallback, PAtom_Mark_CB, Ce_Lw_PElms[i]);
    }

  separator = XtVaCreateManagedWidget("PDrawPTblSep",
    xmSeparatorWidgetClass, PDrawTool_PeriodicTbl_Get (&GPDrawToolCB),
    XmNorientation, XmHORIZONTAL,
    XmNseparatorType, XmSINGLE_LINE,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopOffset, 5,
    XmNtopWidget, Ce_Lw_rc,
    XmNleftAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    XmNrightOffset, 0,
    NULL);

  str = XmStringCreateLocalized (CS_PMULTIPLE);
  pb = XtVaCreateManagedWidget ("PDrawPTblPB",
    xmPushButtonWidgetClass, PDrawTool_PeriodicTbl_Get (&GPDrawToolCB), 
    XmNlabelString, str,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopOffset, 5,
    XmNtopWidget, separator,
    XmNleftAttachment, XmATTACH_FORM,
    XmNleftOffset, 100,
    NULL);
  XmStringFree (str);
  XtAddCallback (pb, XmNactivateCallback, PAtom_Mark_CB, CS_PMULTIPLE);

  str = XmStringCreateLocalized (CS_PDISMISS);
  pb = XtVaCreateManagedWidget ("PDrawPTblPB", xmPushButtonWidgetClass, 
    PDrawTool_PeriodicTbl_Get (&GPDrawToolCB),
    XmNlabelString, str,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopOffset, 5,
    XmNtopWidget, separator,
    XmNrightAttachment, XmATTACH_FORM,
    XmNrightOffset, 100,
    NULL);
  XmStringFree (str);
  XtAddCallback (pb, XmNactivateCallback, PAtom_Mark_CB, CS_PDISMISS);
  
  str = XmStringCreateLocalized (CS_PELM_Ac);
  Ac_elm_pb =  XtVaCreateManagedWidget ("Ac_elm_pb", xmPushButtonGadgetClass, 
    PDrawTool_PeriodicTbl_Get (&GPDrawToolCB),
    XmNlabelString, str,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopOffset, 0,
    XmNtopWidget, Sc_Rn_rc,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftOffset, 0,
    XmNleftWidget, Li_Ra_rc,
    NULL);
  XtAddCallback (Ac_elm_pb, XmNactivateCallback, PAtom_Mark_CB, CS_PELM_Ac);

  return;
}
/*  End of PPeriodic_TblDlg_Create  */


/****************************************************************************
*  
*  Function Name:                 PPixMap_Display_CB
*  
*****************************************************************************/  

void PPixMap_Display_CB
  (
  Widget    w,
  XtPointer client_data,
  XtPointer call_data
  )
{
  Pixmap  pixmap = (Pixmap ) client_data;
  XmDrawnButtonCallbackStruct *cbs = (XmDrawnButtonCallbackStruct *) call_data;
  int          dummy1, dummy2;
  unsigned int dummy3, dummy4;
  unsigned int srcy, srcx, destx, desty, pix_w, pix_h;
  unsigned int drawsize, border;
  Dimension bdr_w, w_width, w_height;
  short hlthick, shthick;
  Window root;

  /*** Get width and height of the pixmap.  Don't use srcx and root ***/
  XGetGeometry (XtDisplay (w), pixmap, &root, &dummy1, &dummy2,
    &pix_w, &pix_h, &dummy3, &dummy4);

  /*  Get the values of all the resources that affect the entire 
   *  geometry of the button.
   */

   XtVaGetValues (w,
     XmNwidth,              &w_width,
     XmNheight,             &w_height,
     XmNborderWidth,        &bdr_w,
     XmNhighlightThickness, &hlthick,
     XmNshadowThickness,    &shthick,
     NULL);

  /*** Calculate available drawing area, width 1st ***/
  border = bdr_w + hlthick + shthick;
  
  /*** if window is bigger than pixmap, center it, else clip pixmap ***/
  drawsize = w_width - 2 * border;
  if (drawsize > pix_w)
    {
    srcx = 0;
    destx = (drawsize - pix_w) / 2 + border;    
    }
  else
    {
    srcx = (pix_w - drawsize) / 2;
    pix_w = drawsize;
    destx = border;
    }
 
  drawsize = w_height - 2 * border;
  if (drawsize > pix_h)
    {
    srcy = 0;
    desty = (drawsize - pix_h) / 2 + border;
    }
  else
    {
    srcy = (pix_h - drawsize) / 2;
    pix_h = drawsize; 
    desty = border;
    }
 
   XCopyArea (XtDisplay (w), pixmap, cbs->window,
     PDrawTool_GC_Get (&GPDrawToolCB), srcx, srcy, pix_w, pix_h, destx, desty);
  
   return;

}
/*  End of PPixMap_Display_CB  */

/****************************************************************************
*  
*  Function Name:                 PPixMap_Resize_CB
*  
*****************************************************************************/  

void PPixMap_Resize_CB
  (
  Widget    w,
  XtPointer client_data,
  XtPointer call_data
  )
{ 
  if (XtIsRealized (w))
    XClearArea (XtDisplay(w), XtWindow(w), 0, 0, 0, 0, TRUE);
 
 return;

}
/*  End of PPixMap_Resize_CB  */

/****************************************************************************
*  
*  Function Name:                 PCreateOptionMenu
*  
*****************************************************************************/  

void PCreateOptionMenu 
  (
  Widget parent,
  char *buttn1_name,
  char *buttn2_name,
  int  buttn1_tag,
  int  buttn2_tag,
  int  x, 
  int  y
  )
{
  Widget menu, pane, button1, button2;

  /*
   *  Create an option menu 
   */

  menu = XmCreateOptionMenu (parent, "menu", NULL, 0);
   
  /*
   *  Create a pulldown pane and attach it to the option menu
   */

  pane = XmCreatePulldownMenu (parent, "pane", NULL, 0);
  XtVaSetValues (menu, XmNsubMenuId, pane,
    NULL);

  XtManageChild (menu);

  /*
   *  Add buttons to the pane and register callbacks
   *  to define the action associated with each menu entry.
   */

  button1 = XtCreateManagedWidget (buttn1_name, xmPushButtonWidgetClass,
    pane, NULL, 0);
  XtAddCallback (button1, XmNactivateCallback, PMenu_Choice_CB, 
    (XtPointer) buttn1_tag);
  button2 = XtCreateManagedWidget (buttn2_name, xmPushButtonWidgetClass,
    pane, NULL, 0);
  XtAddCallback (button2, XmNactivateCallback, PMenu_Choice_CB, 
    (XtPointer) buttn2_tag);

  return;
} 
/*  End of PCreateOptionMenu  */

/****************************************************************************
*  
*  Function Name:                 pclear_it
*  
*****************************************************************************/  

void pclear_it
  (
  Widget     pb,
  XtPointer  client_data,
  XtPointer  call_data
  )
{
  Widget drawing_a = (Widget) client_data;
  XmPushButtonCallbackStruct *cbs = (XmPushButtonCallbackStruct *) call_data;

  /*  Clear pixmap with white */
  XSetForeground (XtDisplay (drawing_a), PDrawTool_GC_Get (&GPDrawToolCB), 
    SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));  

  /* This clears the the pixmap */
  XFillRectangle (XtDisplay (drawing_a),
    PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), 
    PDrawTool_GC_Get (&GPDrawToolCB), 0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT);

  /*  Drawing is now done using black, chage the gc */
  XSetForeground (XtDisplay (drawing_a), PDrawTool_GC_Get (&GPDrawToolCB), 
    SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));

  /*  render the newly cleared pixmap onto the window */
  XCopyArea (cbs->event->xbutton.display,
    PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), 
    XtWindow (drawing_a), PDrawTool_GC_Get (&GPDrawToolCB), 0, 0,
    PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);

  PDraw_Mode_Reset (PDRW_BOND_FLAG);

  gsg = GOAL;
  gsgsame = TRUE;
}
/*  End of pclear_it  */

/****************************************************************************
*  
*  Function Name:                 predraw
*  
*****************************************************************************/  

void predraw
  (
  Widget      drawing_a,
  XtPointer   client_data,
  XtPointer   call_data
  )

{
  XmDrawingAreaCallbackStruct *cbs =
    (XmDrawingAreaCallbackStruct *) call_data;
  
  XCopyArea (cbs->event->xexpose.display,
    PDrawTool_Pixmap_Get (&GPDrawToolCB, *(int *)client_data), 
    XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, *(int *)client_data)), 
    PDrawTool_GC_Get (&GPDrawToolCB), 0, 0, PDRW_DA_PXMP_WD, 
    PDRW_DA_PXMP_HT, 0, 0);

/* Why is this here???  It only screws things up!!! */
/*
  PDraw_Mode_Reset (PDRW_BOND_FLAG);
*/

  return;
}
/*  End of predraw  */

/****************************************************************************
*  
*  Function Name:                 PDraw_Tool_Initialize
*  
*****************************************************************************/  

void PDraw_Tool_Initialize  
  (
  void
  )
{
  ScreenAttr_t  *sca_p;
  int i;
         
  if (first_time)
    {
    PDrawBondsInfo = (PDrw_bonds_info_t *) malloc (PDRW_BONDS_INFO_SIZE);
    PDrawSelInfo = (PDrw_select_info_t *) malloc (PDRW_SELECT_INFO_SIZE);
    }
  
  /* Allocate memory for molecule data structures */
  if (PDrawMol_p == NULL)
    {
    PDrawMol_p = (RxnDsp_Molecule_t *) malloc (RXNDSP_MOLECULE_SIZE);
    PDrawMol_p->goal_dm_p = PDrawMol_p->subgoal_dm_p = NULL;
    RxnDsp_RootCnt_Put (PDrawMol_p, 0);
  
    /* Allocate memory for first 128 atoms and bonds */
    RxnDsp_AtomPtr_Put (PDrawMol_p, 0,
      (RxnDsp_Atom_t *) malloc (RXNDSP_ATOM_SIZE * DRW_MEM_UNIT));
    RxnDsp_BondPtr_Put (PDrawMol_p, 0,
      (RxnDsp_Bond_t *) malloc (RXNDSP_BOND_SIZE * DRW_MEM_UNIT));

    PDrawMol_p->both_dm.nallocatms = DRW_MEM_UNIT;
    PDrawMol_p->both_dm.nallocbnds = DRW_MEM_UNIT;
    PDrawMol_p->both_dm.natms = 0;
    PDrawMol_p->both_dm.nbnds = 0;
    PDrawMol_p->both_dm.rxncnr_p = (RxnDsp_Atom_t *) NULL;
    }

  PDrawMol_p->both_dm.scale = AppDim_MolScale_Get (&GAppDim);
  PDrawTool_Molecule_Put (&GPDrawToolCB, PDrawMol_p); 

  /* Initialize mode of drawing */
  PDrawFlags.drawing_mode = PDRW_BOND_FLAG;
  PDrawFlags.retrace_mode = TRUE;
  PDrawFlags.switch_by_click = TRUE;
  PDrawFlags.dflt_bond = DSP_BOND_DOUBLE;

  PDrawBondsInfo->in_atom_area = FALSE;
  PDrawBondsInfo->in_bond_area = FALSE;
  PDrawBondsInfo->cur_bnd_p = (RxnDsp_Bond_t *) NULL;
  PDrawBondsInfo->esc_mode = TRUE;
  PDrawBondsInfo->clicked_gsg = NEITHER;

  sca_p = SynAppR_ScrnAtrb_Get (&GSynAppR);
  PDrawDisplay = Screen_TDisplay_Get (sca_p);
  PDrawScreen = Screen_TScreen_Get (sca_p);

  return ;
}
/*  End of Draw_Tool_Initialize  */

/****************************************************************************
*  
*  Function Name:                 PPeriodic_TblDlg_Show_CB
*  
*****************************************************************************/  

void PPeriodic_TblDlg_Show_CB
  ( 
  Widget    w, 
  XtPointer client_data,
  XtPointer call_data
  )
{
  if ( w != NULL)
    XtManageChild (PDrawTool_PeriodicTbl_Get (&GPDrawToolCB));
  else
    printf ("ERROR:  Unable to display the periodic table dialog");

  return;
} 
/*  End of PPeriodic_TblDlg_Show_CB  */

/****************************************************************************
*  
*  Function Name:                 PDraw_Draw
*  
*****************************************************************************/  

void PDraw_Draw 
  (
  Widget           wid,
  XButtonEvent    *event,
  String          *args,
  int             *num_args
)
{
  RxnDsp_Atom_t   *dratm_p, *pratm_p, *atom_p;
  RxnDsp_Bond_t   *drbnd_p;
  GC               gc;
  int              adj_i, adj_j, adj_k, jj, pat, k;
  Boolean_t        root_there;

  gc = PDrawTool_GC_Get (&GPDrawToolCB);

  if (*num_args != 1) 
    {
    fprintf (stderr, "PDraw_Draw : Wrong number of args\n");
    exit (-1);
    }

  if (XtWindow (wid) != XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)))
    return;

  switch (PDrawFlags.drawing_mode) 
    {
    case PDRW_BOND_FLAG:
      XSetForeground (PDrawDisplay, gc, 
        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
      if (strcmp (args[0], "down1") == 0) 
        {  
        /* If "escape mode" is set, the current point is the
	   beginning of a bond.
         */
        if (PDrawBondsInfo->esc_mode)
          {
	  PDrawBondsInfo->esc_mode = FALSE;
/*
          lastatomgsg = lastbondgsg = NEITHER;
*/

	  /* Check whether there is already an atom at this point.
	     If there is, just store its coords as the point the bond 
	     starts from.
	     If not, put it into Mol data structure.
	   */
          dratm_p = RxnDsp_AtomThere_Get (PDrawMol_p, gsg, event->x, event->y);
	  if (dratm_p != NULL) 
            {
	    PDrawBondsInfo->clicked_x = RxnDsp_AtomXCoord_Get (dratm_p, gsg);
	    PDrawBondsInfo->clicked_y = RxnDsp_AtomYCoord_Get (dratm_p, gsg);
            PDrawBondsInfo->clicked_gsg = gsg;
	    PDrawBondsInfo->new_atom = FALSE;
	    }
	  else 
            {
	    dratm_p = pstore_AtomNew (PDrawMol_p, "C", "\0", "\0",
              event->x, event->y);
            pdraw_Atom (PDrawDisplay,
              PDrawTool_Pixmap_Get (&GPDrawToolCB, GOAL),
              PDrawTool_Pixmap_Get (&GPDrawToolCB, SUBG), gc,
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK),
              dratm_p, PDRW_ATOM_SIZE_LRG);
            pdraw_Atom (PDrawDisplay,
              XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, GOAL)),
              XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, SUBG)), gc,
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK),
              dratm_p, PDRW_ATOM_SIZE_LRG);

            /* store "null bond" to keep TSD relationships for sling */
            dratm_p = RxnDsp_AtomThere_Get (PDrawMol_p,
              PDrawBondsInfo->clicked_gsg,
              PDrawBondsInfo->clicked_x, PDrawBondsInfo->clicked_y);
            if (dratm_p)
              {
              drbnd_p = RxnDsp_BondThere_Get (PDrawMol_p, gsg,
                event->x, event->y, PDrawBondsInfo->clicked_gsg,
                PDrawBondsInfo->clicked_x, PDrawBondsInfo->clicked_y);
              if (!drbnd_p)
                drbnd_p = pstore_BondNew (PDrawMol_p,
                  PDrawBondsInfo->clicked_gsg,
                  PDrawBondsInfo->clicked_x, PDrawBondsInfo->clicked_y,
                  event->x, event->y, DSP_BOND_ERASE);
              }

	    /* Store the point the bond starts from */
	    PDrawBondsInfo->clicked_x = event->x;
	    PDrawBondsInfo->clicked_y = event->y;
            PDrawBondsInfo->clicked_gsg = gsg;
	    PDrawBondsInfo->new_atom = TRUE;
	    }
          } /* End of if in "escape mode" */

        else
          {
          /* Copy the Pixmap in order to erase the previous line
             and draw a line from (xy) stored to current (xy) in both
             Drawing Area and in Pixmap
          */
/*
          lastbondgsg = gsg;
*/
          if (gsgsame) for (pat=0; pat<2; pat++)
	    XCopyArea (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, pat), 
              XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)), 
              gc, 0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);
          else
	    XCopyArea (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), 
              XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), 
              gc, 0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);

          /* Check whether the bond specified by drawing already exists. */
          drbnd_p = RxnDsp_BondThere_Get (PDrawMol_p, gsg, event->x, event->y,
            PDrawBondsInfo->clicked_gsg,
            PDrawBondsInfo->clicked_x, PDrawBondsInfo->clicked_y);
          if (drbnd_p != NULL) 
            {
            dratm_p = RxnDsp_AtomThere_Get (PDrawMol_p, gsg,
              event->x, event->y);
	    /* If we are in bond++ mode, increment its num of lines */
/* This is not the place to check this!!!
   It causes valid bond order changes to be skipped!!!
	    if (!PDrawFlags.retrace_mode)
*/
            if (gsgsame) for (pat=0; pat<2; pat++)
              {
              /* Erase bond on both Drawing Area and Pixmap. */
	      pdraw_Bond (PDrawDisplay, 
                XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)), gc,
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), PDrawMol_p, 
                drbnd_p, pat, FALSE, FALSE);
	      pdraw_Bond (PDrawDisplay,
                PDrawTool_Pixmap_Get (&GPDrawToolCB, pat), gc,
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), PDrawMol_p,
                drbnd_p, pat, FALSE, FALSE);

	      if (RxnDsp_BondOrder_Get (drbnd_p, pat) < DSP_BOND_TRIPLE
                && !PDrawFlags.retrace_mode)
                RxnDsp_BondOrder_Put (drbnd_p, pat,
                RxnDsp_BondOrder_Get (drbnd_p, pat) + 1);
	      else
                RxnDsp_BondOrder_Put (drbnd_p, pat, DSP_BOND_SINGLE);

	      pdraw_Bond (PDrawDisplay, 
                XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)), gc, 
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), PDrawMol_p, 
                drbnd_p, pat, TRUE, TRUE);
	      pdraw_Bond (PDrawDisplay,
                PDrawTool_Pixmap_Get (&GPDrawToolCB, pat), gc,
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), 
                PDrawMol_p, drbnd_p, pat, TRUE, TRUE);
	      }
            else
              {
              /* Erase bond on both Drawing Area and Pixmap. */
	      pdraw_Bond (PDrawDisplay, 
                XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc,
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), PDrawMol_p, 
                drbnd_p, gsg, FALSE, FALSE);
	      pdraw_Bond (PDrawDisplay,
                PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), gc,
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), PDrawMol_p,
                drbnd_p, gsg, FALSE, FALSE);

	      if (RxnDsp_BondOrder_Get (drbnd_p, gsg) < DSP_BOND_TRIPLE
                && !PDrawFlags.retrace_mode)
                RxnDsp_BondOrder_Put (drbnd_p, gsg,
                RxnDsp_BondOrder_Get (drbnd_p, gsg) + 1);
	      else
                RxnDsp_BondOrder_Put (drbnd_p, gsg, DSP_BOND_SINGLE);

	      pdraw_Bond (PDrawDisplay, 
                XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc, 
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), PDrawMol_p, 
                drbnd_p, gsg, TRUE, TRUE);
	      pdraw_Bond (PDrawDisplay,
                PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), gc,
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), 
                PDrawMol_p, drbnd_p, gsg, TRUE, TRUE);
	      }

	    /* Store the point the bond ends at */
	    PDrawBondsInfo->clicked_x = RxnDsp_AtomXCoord_Get (dratm_p, gsg);
	    PDrawBondsInfo->clicked_y = RxnDsp_AtomYCoord_Get (dratm_p, gsg);
            PDrawBondsInfo->clicked_gsg = gsg;

	    /* Store this bond as the current bond */
	    PDrawBondsInfo->cur_bnd_p = drbnd_p;
            } /* End if get_Bond_There */

          else 
            {
	    /* If bond starts from and ends at the same point and
	       the atom it starts from is new, discard this drawings.
	     */
	    if ((dratm_p = RxnDsp_AtomThere_Get (PDrawMol_p, gsg,
                   event->x, event->y)) 
	        && (RxnDsp_AtomXCoord_Get (dratm_p, PDrawBondsInfo->clicked_gsg)
                   == PDrawBondsInfo->clicked_x) 
	        && (RxnDsp_AtomYCoord_Get (dratm_p, PDrawBondsInfo->clicked_gsg)
                   == PDrawBondsInfo->clicked_y) 
	        && (PDrawBondsInfo->new_atom)) 
	      return;

	    /* Check whether there is an atom at the end point of the bond 
	       If yes, simply draw a line and store the coords of this atom
	       as the end coords of the bond. 
	       Otherwise, alloc memory for a new atom, draw a line and make
	       appropriate storage.
            */
	    if (dratm_p != NULL) 
              {
              drbnd_p = pstore_BondNew (PDrawMol_p, PDrawBondsInfo->clicked_gsg,
                PDrawBondsInfo->clicked_x, 
                PDrawBondsInfo->clicked_y, RxnDsp_AtomXCoord_Get (dratm_p, gsg),
                RxnDsp_AtomYCoord_Get (dratm_p, gsg), DSP_BOND_SINGLE);
	      if (drbnd_p != NULL)
                {
                if (gsgsame) for (pat=0; pat<2; pat++)
                  {
	          pdraw_Bond (PDrawDisplay, 
                    XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)), gc, 
                    SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK),
                    PDrawMol_p, drbnd_p, pat, TRUE, TRUE);
	          pdraw_Bond (PDrawDisplay,
                    PDrawTool_Pixmap_Get (&GPDrawToolCB, pat), 
                    gc, SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), 
                    PDrawMol_p, drbnd_p, pat, TRUE, TRUE);
                  }
                else
                  {
	          pdraw_Bond (PDrawDisplay, 
                    XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc, 
                    SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK),
                    PDrawMol_p, drbnd_p, gsg, TRUE, TRUE);
	          pdraw_Bond (PDrawDisplay,
                    PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), 
                    gc, SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), 
                    PDrawMol_p, drbnd_p, gsg, TRUE, TRUE);
                  }

	        /* Store the point the bond ends at */
	        PDrawBondsInfo->clicked_x =
                  RxnDsp_AtomXCoord_Get (dratm_p, gsg);
	        PDrawBondsInfo->clicked_y =
                  RxnDsp_AtomYCoord_Get (dratm_p, gsg);
                PDrawBondsInfo->clicked_gsg = gsg;

	        /* Store this bond as the current bond */
	        PDrawBondsInfo->cur_bnd_p = drbnd_p;
	        }

	      else  /* pstore_BondNew retured NULL - too many bonds attached */
                {   
	        /* Manage appropriate Error Dialog and discard this bond */
	        PDrawBondsInfo->esc_mode = TRUE;
	        PDrawBondsInfo->cur_bnd_p = (RxnDsp_Bond_t *) NULL;
/*
	        InfoWarn_Show ("many_bonds_errdg"); 
*/
	        }
	      } /* End of if atom is at the end point */

	    else 
             {
	      dratm_p = pstore_AtomNew (PDrawMol_p, "C", "\0", "\0", event->x, 
                event->y);
              drbnd_p = pstore_BondNew (PDrawMol_p, PDrawBondsInfo->clicked_gsg,
                PDrawBondsInfo->clicked_x, 
                PDrawBondsInfo->clicked_y, event->x, event->y,
                DSP_BOND_SINGLE);
	      if (drbnd_p != NULL) 
                {
                if (gsgsame) for (pat=0; pat<2; pat++)
                  {
	          pdraw_Bond (PDrawDisplay, 
                    XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)), gc,
                    SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK),
                    PDrawMol_p, drbnd_p, pat, TRUE, TRUE);
	          pdraw_Bond (PDrawDisplay,
                    PDrawTool_Pixmap_Get (&GPDrawToolCB, pat), 
                    gc, SynAppR_IthColor_Get (
                    &GSynAppR, SAR_CLRI_BLACK), PDrawMol_p, drbnd_p, pat,
                    TRUE, TRUE);
                  }
                else
                  {
	          pdraw_Bond (PDrawDisplay, 
                    XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc,
                    SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK),
                    PDrawMol_p, drbnd_p, gsg, TRUE, TRUE);
	          pdraw_Bond (PDrawDisplay,
                    PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), 
                    gc, SynAppR_IthColor_Get (
                    &GSynAppR, SAR_CLRI_BLACK), PDrawMol_p, drbnd_p, gsg,
                    TRUE, TRUE);
                  pat = 1 - gsg;

                  pdraw_Atom (PDrawDisplay,
                    PDrawTool_Pixmap_Get (&GPDrawToolCB, pat),
                    PDrawTool_Pixmap_Get (&GPDrawToolCB, pat), gc,
                    SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK),
                    dratm_p, PDRW_ATOM_SIZE_LRG);
                  pdraw_Atom (PDrawDisplay,
                    XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)),
                    XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)), gc,
                    SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK),
                    dratm_p, PDRW_ATOM_SIZE_LRG);
                  }

	        /* Store the point the bond ends at */
	        PDrawBondsInfo->clicked_x = event->x;
	        PDrawBondsInfo->clicked_y = event->y;
                PDrawBondsInfo->clicked_gsg = gsg;

	        /* Store this bond as the current bond */
	        PDrawBondsInfo->cur_bnd_p = drbnd_p;
	        }

	      else 
                {  
                /* pstore_BondNew retured NULL - too many bonds attached.
	           Manage appropriate Error Dialog and discard this bond.
                */
	        PDrawBondsInfo->esc_mode = TRUE;
	        PDrawBondsInfo->cur_bnd_p = (RxnDsp_Bond_t *) NULL;
/*
	        InfoWarn_Show ("many_bonds_errdg");
*/
	        rxn_delete_Atom (PDrawMol_p, dratm_p, gsgsame ? BOTH : gsg);
	        }
	      }
	    } /* End else get_Bond_There */
          } /*End not in the "ecsape mode" */
        } /* End of "down1" */
    
      else if ((strcmp (args[0], "down2") == 0) && (!PDrawBondsInfo->esc_mode)) 
        {
        /* If the bond specified by drawing already exists. */
        drbnd_p = RxnDsp_BondThere_Get (PDrawMol_p, gsg, event->x, event->y,
          PDrawBondsInfo->clicked_gsg,
          PDrawBondsInfo->clicked_x, PDrawBondsInfo->clicked_y);
        if (drbnd_p != NULL) 
          {
	  dratm_p = RxnDsp_AtomThere_Get (PDrawMol_p, gsg, event->x, event->y);

	  /* Erase bond on both Drawing Area and Pixmap. */
          if (gsgsame) for (pat=0; pat<2; pat++)
            {
	    pdraw_Bond (PDrawDisplay, 
              XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)), gc,
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), PDrawMol_p, 
              drbnd_p, pat, FALSE, FALSE);
	    pdraw_Bond (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, pat),
              gc, SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), PDrawMol_p, 
              drbnd_p, pat, FALSE, FALSE);

	    RxnDsp_BondOrder_Put (drbnd_p, pat, PDrawFlags.dflt_bond);

	    /* Draw bond again */
	    pdraw_Bond (PDrawDisplay, 
              XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)), gc, 
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), PDrawMol_p,
              drbnd_p, pat, TRUE, TRUE);
	    pdraw_Bond (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, pat),
              gc, SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), PDrawMol_p, 
              drbnd_p, pat, TRUE, TRUE);
            }
          else
            {
	    pdraw_Bond (PDrawDisplay, 
              XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc,
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), PDrawMol_p, 
              drbnd_p, gsg, FALSE, FALSE);
	    pdraw_Bond (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg),
              gc, SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), PDrawMol_p, 
              drbnd_p, gsg, FALSE, FALSE);

	    RxnDsp_BondOrder_Put (drbnd_p, gsg, PDrawFlags.dflt_bond);

	    /* Draw bond again */
	    pdraw_Bond (PDrawDisplay, 
              XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc, 
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), PDrawMol_p,
              drbnd_p, gsg, TRUE, TRUE);
	    pdraw_Bond (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg),
              gc, SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), PDrawMol_p, 
              drbnd_p, gsg, TRUE, TRUE);
            }

          } /* End of if there is a bond specified by drawings */

        else 
          {
	  dratm_p = pstore_AtomNew (PDrawMol_p, "C", "\0", "\0", event->x, 
            event->y);
          drbnd_p = pstore_BondNew (PDrawMol_p, PDrawBondsInfo->clicked_gsg,
            PDrawBondsInfo->clicked_x, 
            PDrawBondsInfo->clicked_y, RxnDsp_AtomXCoord_Get (dratm_p, gsg),
            RxnDsp_AtomYCoord_Get (dratm_p, gsg), PDrawFlags.dflt_bond);
	  if (drbnd_p != NULL)
            {
            if (gsgsame) for (pat=0; pat<2; pat++)
              {
	      pdraw_Bond (PDrawDisplay, 
                XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)), gc, 
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), PDrawMol_p, 
                drbnd_p, pat, TRUE, TRUE);
              pdraw_Bond (PDrawDisplay,
                PDrawTool_Pixmap_Get (&GPDrawToolCB, pat), gc,
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), 
                PDrawMol_p, drbnd_p, pat, TRUE, TRUE);
              }
            else
              {
	      pdraw_Bond (PDrawDisplay, 
                XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc, 
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), PDrawMol_p, 
                drbnd_p, gsg, TRUE, TRUE);
              pdraw_Bond (PDrawDisplay,
                PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), gc,
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), 
                PDrawMol_p, drbnd_p, gsg, TRUE, TRUE);
              pat = 1 - gsg;

              pdraw_Atom (PDrawDisplay,
                PDrawTool_Pixmap_Get (&GPDrawToolCB, pat),
                PDrawTool_Pixmap_Get (&GPDrawToolCB, pat), gc,
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK),
                dratm_p, PDRW_ATOM_SIZE_LRG);
              pdraw_Atom (PDrawDisplay,
                XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)),
                XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)), gc,
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK),
                dratm_p, PDRW_ATOM_SIZE_LRG);
              }

	    /* Store this bond as the current bond */
	    PDrawBondsInfo->cur_bnd_p = drbnd_p;
	    }
	  else  /* pstore_BondNew retured NULL - too many bonds attached */
            { 
	    /* Manage appropriate Error Dialog and discard this bond */
	    PDrawBondsInfo->esc_mode = TRUE;
	    PDrawBondsInfo->cur_bnd_p = (RxnDsp_Bond_t *) NULL;
/*
	    InfoWarn_Show ("many_bonds_errdg");
*/
	    if (dratm_p->adj_info.num_neighbs == 0)
	      rxn_delete_Atom (PDrawMol_p, dratm_p, gsgsame ? BOTH : gsg);
	    }
          } /* End of there is no bond specified by drawings */

        /* Store the point the bond ends at */
        PDrawBondsInfo->clicked_x = RxnDsp_AtomXCoord_Get (dratm_p, gsg);
        PDrawBondsInfo->clicked_y = RxnDsp_AtomYCoord_Get (dratm_p, gsg);
        PDrawBondsInfo->clicked_gsg = gsg;
        if (gsgsame) for (pat=0; pat<2; pat++)
          XCopyArea (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, pat), 
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)), 
            gc, 0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);
        else
          XCopyArea (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), 
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), 
            gc, 0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);
        } /* End of "down2" */

      else if (strcmp (args[0], "motion") == 0) 
        {
        if (!PDrawBondsInfo->esc_mode) 
          {
          dratm_p = RxnDsp_AtomThere_Get (PDrawMol_p, gsg, event->x, event->y);
          pratm_p = RxnDsp_AtomThere_Get (PDrawMol_p,
            PDrawBondsInfo->clicked_gsg, PDrawBondsInfo->clicked_x,
            PDrawBondsInfo->clicked_y);
          if (pratm_p != NULL && IsThere_Draw_Flag && glob_rxlform && RxnDsp_AtomXCoord_Get (pratm_p, gsg) > 0xff00 &&
            RxnDsp_AtomYCoord_Get (pratm_p, gsg) > 0xff00)
            {
            PDrawBondsInfo->esc_mode = TRUE;
/*
            PDrawBondsInfo->cur_bnd_p = (RxnDsp_Bond_t *) NULL;
*/
            dratm_p = pratm_p = NULL;
            }
	  if (dratm_p != NULL)
            { 
	    if (!PDrawBondsInfo->in_atom_area && pratm_p != NULL)
              {
	      XCopyArea (PDrawDisplay,
                PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), 
                XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc,
                0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);
	      XDrawLine (event->display, event->window, gc, 
                RxnDsp_AtomXCoord_Get (dratm_p, gsg),
                RxnDsp_AtomYCoord_Get (dratm_p, gsg),
                RxnDsp_AtomXCoord_Get (pratm_p, gsg),
                RxnDsp_AtomYCoord_Get (pratm_p, gsg));
/*
                PDrawBondsInfo->clicked_x, PDrawBondsInfo->clicked_y);
*/
	      }
	    }

	  else if (pratm_p != NULL)
            {
	    XCopyArea (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg),
              XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc, 0, 0, 
              PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);
	    XDrawLine (event->display, event->window, 
              gc, event->x, event->y,
              RxnDsp_AtomXCoord_Get (pratm_p, gsg),
              RxnDsp_AtomYCoord_Get (pratm_p, gsg));
/*
              PDrawBondsInfo->clicked_x, PDrawBondsInfo->clicked_y);
*/
	    }
          }
        } /* End of "motion" */

      else if (strcmp (args[0], "down3") == 0) 
        {
        PDrawBondsInfo->esc_mode = TRUE;
        PDrawBondsInfo->cur_bnd_p = (RxnDsp_Bond_t *) NULL;

        /* Copy the Pixmap in order to erase the previous line
	   and hihglighted areas.
         */
        XCopyArea (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), 
          XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc, 0, 0, 
          PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);

        /* Check whether to discard the atom that was made by the last
           drawing.
         */
        if ((dratm_p = RxnDsp_AtomThere_Get (PDrawMol_p,
            PDrawBondsInfo->clicked_gsg,
            PDrawBondsInfo->clicked_x, PDrawBondsInfo->clicked_y))
            && (dratm_p->adj_info.num_neighbs == 0) 
            && (dratm_p->isC)
            && RxnDsp_AtomCharge_Get (dratm_p, gsg) == '\0')
	  PDrawMol_p->both_dm.natms--;     
        } /* End of "down3" */

      else if (strcmp (args[0], "leave") == 0)
        {
/*
        if (gsgsame) for (pat=0; pat<2; pat++)
	  XCopyArea (PDrawDisplay,
            PDrawTool_Pixmap_Get (&GPDrawToolCB, pat), 
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)), gc,
            0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);
        else
*/
	  XCopyArea (PDrawDisplay,
            PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), 
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc,
            0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);
        }

      break; /* End of PDRW_BOND_FLAG */

    case PDRW_SELECTROOT_FLAG:
      if (strcmp (args[0], "down1") == 0)
        {
        dratm_p = RxnDsp_AtomThere_Get (PDrawMol_p, gsg, event->x, event->y);
        if (dratm_p != NULL) 
          {
/*
          if (strcasecmp (dratm_p->sym, root_atom))
*/
          if (!Root_Atom_OK (dratm_p->sym))
            {
            printf("Invalid atom selected\n");
            XBell (PDrawDisplay, 10);
            break;
            }

          /* Reset for next time */
          root_atom[0] = 0;

          for (k = 0, root_there = FALSE;
               !root_there && k < RxnDsp_RootCnt_Get (PDrawMol_p); k++)
            if (RxnDsp_RootPtr_Get (PDrawMol_p, k)->x ==
                RxnDsp_AtomXCoord_Get (dratm_p, GOAL) &&
                RxnDsp_RootPtr_Get (PDrawMol_p, k)->y ==
                RxnDsp_AtomYCoord_Get (dratm_p, GOAL))
            {
            root_there = TRUE;
            RxnDsp_RootPtr_Get (PDrawMol_p, k)->syntheme = Selected_Syntheme;
            }

          if (!root_there)
            {
            RxnDsp_RootPtr_Get (PDrawMol_p, RxnDsp_RootCnt_Get (PDrawMol_p))->x
              = RxnDsp_AtomXCoord_Get (dratm_p, GOAL);
            RxnDsp_RootPtr_Get (PDrawMol_p, RxnDsp_RootCnt_Get (PDrawMol_p))->y
              = RxnDsp_AtomYCoord_Get (dratm_p, GOAL);
            RxnDsp_RootPtr_Get (PDrawMol_p, RxnDsp_RootCnt_Get (PDrawMol_p))->
              syntheme = Selected_Syntheme;
            RxnDsp_RootCnt_Put (PDrawMol_p,
              RxnDsp_RootCnt_Get (PDrawMol_p) + 1);
            }
/*
          PDrawFlags.drawing_mode = PDRW_SYNWAIT_FLAG;
*/
          /* Copy the Pixmap in order to erase the previously hihglighted areas
          */
          XCopyArea (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), 
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc,
            0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);
          PDrawBondsInfo->in_atom_area = FALSE;
/*
      root_atoms[nroots++] = dratm_p - RxnDsp_AtomPtr_Get (PDrawMol_p, 0);
*/
          draw_roots(PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, GOAL),
           XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, GOAL)), gc,
           PDrawMol_p);
          PDrawFlags.drawing_mode = PDRW_BOND_FLAG;
          PDraw_Mode_Reset (PDRW_BOND_FLAG);
          }
        }
      break;

    case PDRW_DELETEROOT_FLAG:
      if (strcmp (args[0], "down1") == 0)
        {
        dratm_p = RxnDsp_AtomThere_Get (PDrawMol_p, gsg, event->x, event->y);
        if (dratm_p != NULL) 
          {
          for (jj = 0; jj < RxnDsp_RootCnt_Get (PDrawMol_p); jj++)
            if (RxnDsp_AtomXCoord_Get (dratm_p, gsg) ==
                RxnDsp_RootPtr_Get (PDrawMol_p, jj)->x &&
                RxnDsp_AtomYCoord_Get (dratm_p, gsg) ==
                RxnDsp_RootPtr_Get (PDrawMol_p, jj)->y)
            {
            RxnDsp_RootCnt_Put (PDrawMol_p,
              RxnDsp_RootCnt_Get (PDrawMol_p) - 1);
            for (k = jj; k < RxnDsp_RootCnt_Get (PDrawMol_p); k++)
              {
                RxnDsp_RootPtr_Get (PDrawMol_p, k)->x =
                  RxnDsp_RootPtr_Get (PDrawMol_p, k + 1)->x;
                RxnDsp_RootPtr_Get (PDrawMol_p, k)->y =
                  RxnDsp_RootPtr_Get (PDrawMol_p, k + 1)->y;
                RxnDsp_RootPtr_Get (PDrawMol_p, k)->syntheme =
                  RxnDsp_RootPtr_Get (PDrawMol_p, k + 1)->syntheme;
              }
/* fudge/hack to avoid rewriting callback code into another routine */
            PNumTogl_CB (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg),
              (XtPointer) &k, NULL);
            PDrawFlags.drawing_mode = PDRW_BOND_FLAG;
            PDraw_Mode_Reset (PDRW_BOND_FLAG);
            break;
            }
          }
        }
      break;

    case PDRW_CLEAR_FLAG:
      if (strcmp (args[0], "down1") == 0) 
        {
        /* If there is a click on an atom, delete it with all bonds
           adjacent to it.
         */
        dratm_p = RxnDsp_AtomThere_Get (PDrawMol_p, gsg, event->x, event->y);
        if (dratm_p != NULL) 
          {
          /* Delete all the adjacent bonds first */
	  jj = dratm_p->adj_info.num_neighbs;
	  for (adj_i = adj_k = 0; adj_i < jj; adj_i++) 
            {
	    adj_j = (int) dratm_p->adj_info.adj_atms[adj_k];
            atom_p = RxnDsp_AtomPtr_Get (PDrawMol_p, adj_j);
	    drbnd_p = RxnDsp_BondThere_Get (PDrawMol_p, gsg,
              RxnDsp_AtomXCoord_Get (dratm_p, gsg),
              RxnDsp_AtomYCoord_Get (dratm_p, gsg),
              gsg,
              RxnDsp_AtomXCoord_Get (atom_p, gsg),
              RxnDsp_AtomYCoord_Get (atom_p, gsg));
            if (gsgsame) for (pat=0; pat<2; pat++)
              {
	      pdraw_Bond (PDrawDisplay, 
                XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)), gc, 
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), PDrawMol_p, 
                drbnd_p, pat, TRUE, TRUE);	  
	      pdraw_Bond (PDrawDisplay,
                PDrawTool_Pixmap_Get (&GPDrawToolCB, pat), gc,
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), 
                PDrawMol_p, drbnd_p, pat, TRUE, TRUE);	  
              }
            else
              {
	      pdraw_Bond (PDrawDisplay, 
                XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc, 
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), PDrawMol_p, 
                drbnd_p, gsg, TRUE, TRUE);	  
	      pdraw_Bond (PDrawDisplay,
                PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), gc,
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), 
                PDrawMol_p, drbnd_p, gsg, TRUE, TRUE);	  
              }
	    if (!rxn_delete_Bond (PDrawMol_p, drbnd_p, gsgsame ? BOTH : gsg))
              adj_k++;
	    else if ((atom_p->adj_info.num_neighbs == 0) && (atom_p->isC)
                && RxnDsp_AtomCharge_Get (atom_p, gsg) == '\0')
              {
	      pdraw_Atom (PDrawDisplay, 
                XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, GOAL)),
                XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, SUBG)), gc, 
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), atom_p, 
                PDRW_ATOM_SIZE_LRG);
	      pdraw_Atom (PDrawDisplay,
                PDrawTool_Pixmap_Get (&GPDrawToolCB, GOAL),
                PDrawTool_Pixmap_Get (&GPDrawToolCB, SUBG), gc,
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), atom_p,
                PDRW_ATOM_SIZE_LRG);
              check_roots (PDrawMol_p, atom_p);
	      if (!rxn_delete_Atom (PDrawMol_p, atom_p, gsgsame ? BOTH : gsg))
                {
	        pdraw_Atom (PDrawDisplay, 
                  XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, GOAL)),
                  XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, SUBG)), gc, 
                  SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), atom_p, 
                  PDRW_ATOM_SIZE_LRG);
	        pdraw_Atom (PDrawDisplay,
                  PDrawTool_Pixmap_Get (&GPDrawToolCB, GOAL),
                  PDrawTool_Pixmap_Get (&GPDrawToolCB, SUBG), gc,
                  SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), atom_p, 
                  PDRW_ATOM_SIZE_LRG);
                }
              }
	    /* dratm_p could point at the last atom in the molecule.
	       Then, delete_Atom(DrawMol_p, atom_p) copied it to the
	       place of atom_p. Since we want to point at the last version
	       of dratm_p, redirect dratm_p
	     */
	    if (RxnDsp_AtomPtr_Get (PDrawMol_p,
              PDrawMol_p->both_dm.natms + 1) == dratm_p)
              dratm_p = atom_p;
	    } /* End of for all adjacent bonds */

	  pdraw_Atom (PDrawDisplay, 
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, GOAL)),
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, SUBG)), gc, 
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), dratm_p, 
            PDRW_ATOM_SIZE_LRG);
	  pdraw_Atom (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, GOAL),
            PDrawTool_Pixmap_Get (&GPDrawToolCB, SUBG), gc,
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), dratm_p, 
            PDRW_ATOM_SIZE_LRG);
          check_roots (PDrawMol_p, dratm_p);
	  if (!rxn_delete_Atom (PDrawMol_p, dratm_p, gsgsame ? BOTH : gsg))
            {
	    pdraw_Atom (PDrawDisplay, 
              XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, GOAL)),
              XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, SUBG)), gc, 
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), dratm_p, 
              PDRW_ATOM_SIZE_LRG);
	    pdraw_Atom (PDrawDisplay,
              PDrawTool_Pixmap_Get (&GPDrawToolCB, GOAL),
              PDrawTool_Pixmap_Get (&GPDrawToolCB, SUBG), gc,
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), dratm_p, 
              PDRW_ATOM_SIZE_LRG);
            }
          }
      
        /* If there is a click on a bond, delete it */
        drbnd_p = pin_Bond_Scope (PDrawMol_p, event->x, event->y);
        if (drbnd_p != NULL) if (gsgsame)
          {
          for (pat=0; pat<2; pat++)
	    pdraw_Bond (PDrawDisplay, 
              XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)), gc, 
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), PDrawMol_p, 
              drbnd_p, pat, TRUE, TRUE);
	  perase_Bond (PDrawDisplay,
            PDrawTool_Pixmap_Get (&GPDrawToolCB, GOAL),
            PDrawTool_Pixmap_Get (&GPDrawToolCB, SUBG),
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, GOAL)),
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, SUBG)), gc,
            PDrawMol_p, drbnd_p, BOTH);
          }
        else
          {
	  pdraw_Bond (PDrawDisplay, 
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc, 
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), PDrawMol_p, 
            drbnd_p, gsg, TRUE, TRUE);
	  perase_Bond (PDrawDisplay,
            PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg),
            PDrawTool_Pixmap_Get (&GPDrawToolCB, 1 - gsg),
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)),
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, 1 - gsg)), gc,
            PDrawMol_p, drbnd_p, gsg);
          }
        } /* End of "down1" */
      break; /* End of PDRW_CLEAR_FLAG */

    case PDRW_SELECT_FLAG:
      if (strcmp (args[0], "down1") == 0) 
        {
        /* If this is the first click in selection mode 
           or outside the selected area, start selection.
        */
        if (!PDrawSelInfo->move_it && (!PDrawSelInfo->isSelected 
            || (!gsgsame && gsg != PDrawSelInfo->selected_gsg)
	    || event->x < PDrawSelInfo->selected_x 
	    || event->y < PDrawSelInfo->selected_y 
	    || event->x > PDrawSelInfo->selected_x + PDrawSelInfo->sel_width
	    || event->y > PDrawSelInfo->selected_y + PDrawSelInfo->sel_height))
          {
	  /* If smth was selected prior, unselect it */
          if (gsgsame || gsg == PDrawSelInfo->selected_gsg)
	    punmark_selected (PDrawMol_p, event->display, 
              PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), event->window, gc);
          if (gsgsame || gsg != PDrawSelInfo->selected_gsg)
            {
            gsg = 1 - gsg;
	    punmark_selected (PDrawMol_p, event->display, 
              PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg),
              XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc);
            gsg = 1 - gsg;
            PDrawSelInfo->selected_gsg = gsg;
            }
	  PDrawBondsInfo->clicked_x = event->x;
	  PDrawBondsInfo->clicked_y = event->y;
          PDrawBondsInfo->clicked_gsg = gsg;
	  PDrawBondsInfo->prev_x = event->x;
	  PDrawBondsInfo->prev_y = event->y;
          PDrawBondsInfo->prev_gsg = gsg;

	  /* Initialize information about position of atom 
	     reletively to position of pointer.
	  */
	  dratm_p = RxnDsp_AtomPtr_Get (PDrawMol_p, 0);
	  for (jj = 0; jj < PDrawMol_p->both_dm.natms; jj++) 
            {
	    dratm_p->kvadr = patom_pos_to_pointer_pos (
              RxnDsp_AtomXCoord_Get (dratm_p, gsg),
              RxnDsp_AtomYCoord_Get (dratm_p, gsg),
              event->x, event->y);
	    ++dratm_p;
	    }
          } /* End of the first click in selection mode */

        /* If this is the second (completing) click in selection mode */
        else if  (PDrawSelInfo->move_it && !PDrawSelInfo->isSelected) 
          {
	  PDrawSelInfo->move_it = FALSE;

	  /* Draw a line that complete selecting loop */
	  XSetForeground (PDrawDisplay, gc, 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLUE));
	  XDrawLine (event->display, event->window,
            gc, event->x, event->y,
            PDrawBondsInfo->clicked_x, PDrawBondsInfo->clicked_y);

	  /* Check selection status of atoms, if someone dragged mouse
	     directly to the point of the first click.
	  */
	  pcheck_kvadr (PDrawMol_p, PDrawBondsInfo->clicked_x, 
            PDrawBondsInfo->clicked_y, event->x, event->y);

	  /* Copy the Pixmap in order to erase the selecting curve */
	  XCopyArea (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), 
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc, 0, 0, 
            PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);

	  /* Highlight atoms that have been selected */
/*
          selgsg = gsg;
*/
          PDrawSelInfo->selected_gsg = gsg;
	  pmark_selected (PDrawMol_p, event->display, event->window, 
            gc);
          if (gsgsame)
            {
            gsg = 1 - gsg;
	    pmark_selected (PDrawMol_p, event->display,
              XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)),
              gc);
            gsg = 1 - gsg;
            }
          } /* End of the second click in selection mode */
  
        /* If there is click inside of the selected area, 
	   make preparation to move it.
         */
        else if (PDrawSelInfo->isSelected && !PDrawSelInfo->move_it 
            && (gsgsame || gsg == PDrawSelInfo->selected_gsg)
	    && event->x > PDrawSelInfo->selected_x 
            && event->y > PDrawSelInfo->selected_y 
            && event->x < PDrawSelInfo->selected_x + PDrawSelInfo->sel_width 
            && event->y < PDrawSelInfo->selected_y + PDrawSelInfo->sel_height) 
          {
	  PDrawSelInfo->move_it = TRUE;

	  /* Store click coords to calculate deltas when the area will be 
             moved.
          */
	  PDrawBondsInfo->clicked_x = event->x;
	  PDrawBondsInfo->clicked_y = event->y;
          PDrawBondsInfo->clicked_gsg = gsg;
	  PDrawBondsInfo->prev_x = event->x;
	  PDrawBondsInfo->prev_y = event->y;
          PDrawBondsInfo->prev_gsg = gsg;

	  /* Erase the selected area on the pixmap */
	  pdraw_selected (PDrawMol_p, event->display, 
            PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), gc, 
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), 0, 0, FALSE);
          if (gsgsame)
            {
            gsg = 1 - gsg;
	    pdraw_selected (PDrawMol_p, event->display, 
              PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), gc, 
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), 0, 0, FALSE);
            gsg = 1 - gsg;
            }
          } /* End of initializing for movement of selected area */

        /* If there is a click during the movement of selected area,
	   stop moving it.
         */
        else if ((PDrawSelInfo->isSelected) && (PDrawSelInfo->move_it)) 
          {
	  PDrawSelInfo->move_it = FALSE;
	  pdraw_selected (PDrawMol_p, event->display, 
            PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), 
            gc, SynAppR_IthColor_Get (&GSynAppR, 
            SAR_CLRI_BLUE), event->x - PDrawBondsInfo->prev_x, 
            event->y - PDrawBondsInfo->prev_y, TRUE);
	  XCopyArea (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), 
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc, 0, 0, 
            PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);
          }
        } /* End of "down1" */

      if (strcmp (args[0], "motion") == 0) 
        {
        /* If smth is already selected, this motion event
	   should move selected area.
         */
        if ((PDrawSelInfo->isSelected) && (PDrawSelInfo->move_it)) 
          {
	  pdraw_selected (PDrawMol_p, event->display, event->window, 
            gc, SynAppR_IthColor_Get (&GSynAppR, 
            SAR_CLRI_BLUE), event->x - PDrawBondsInfo->prev_x, 
            event->y - PDrawBondsInfo->prev_y,TRUE);
          PDrawBondsInfo->prev_x = event->x;
	  PDrawBondsInfo->prev_y = event->y;
          PDrawBondsInfo->prev_gsg = gsg;
          }

        else if (PDrawSelInfo->move_it && !PDrawSelInfo->isSelected) 
          {
	  XSetForeground (PDrawDisplay, gc, 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLUE));
	  XDrawLine (event->display, event->window, gc,
            PDrawBondsInfo->prev_x, PDrawBondsInfo->prev_y,
            event->x, event->y);

	  /* Check selection status of atoms after this motion event */
	  pcheck_kvadr (PDrawMol_p, event->x, event->y,
            PDrawBondsInfo->prev_x, PDrawBondsInfo->prev_y);
	  PDrawBondsInfo->prev_x = event->x;
	  PDrawBondsInfo->prev_y = event->y;
          PDrawBondsInfo->prev_gsg = gsg;
          }
        } /* End of "motion" */
      break;  /* End of PDRW_SELECT_FLAG */

    case PDRW_ATOM_FLAG:
      if (strcmp (args[0], "down1") == 0) 
        {
        dratm_p = RxnDsp_AtomThere_Get (PDrawMol_p, gsg, event->x, event->y);
        if (dratm_p != NULL) 
          {
	  PDrawBondsInfo->clicked_x = event->x;
	  PDrawBondsInfo->clicked_y = event->y;
          PDrawBondsInfo->clicked_gsg = gsg;

	  /* Assign current atom symbol (specified by pushbuttons in
	     periodic table) to the clicked atom by invoking Atom_Mark_CB.
	   */
	  PAtom_Mark_CB ((Widget) NULL, (XtPointer) PDrawFlags.cur_atmsym,
            (XtPointer) NULL);
          }
        }
      break;  /* End of PDRW_ATOM_FLAG */

    case PDRW_RXNCNTR_FLAG:
      dratm_p = RxnDsp_AtomThere_Get (PDrawMol_p, gsg, event->x, event->y);
      if ((strcmp (args[0], "down1") == 0) && dratm_p != NULL) 
        {
        /* If rxn center has been specified, clear the previous one */
        if (RxnDsp_RxncPtr_Get (PDrawMol_p)) for (pat=0; pat<2; pat++)
          {
	  XSetForeground (PDrawDisplay, gc, 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
	  XDrawRectangle (PDrawDisplay,
            PDrawTool_Pixmap_Get (&GPDrawToolCB, pat), gc, 
            RxnDsp_AtomXCoord_Get (RxnDsp_RxncPtr_Get (PDrawMol_p), gsg)
              - (PDRW_HILIT_AREA << 1),
            RxnDsp_AtomYCoord_Get (RxnDsp_RxncPtr_Get (PDrawMol_p), gsg)
              - (PDRW_HILIT_AREA << 1),
            PDRW_HILIT_AREA << 2, PDRW_HILIT_AREA << 2);
          }

        /* Mark clicked atom as a reaction center */
        XSetForeground (PDrawDisplay, gc, 
          SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_GOLD));
        for (pat=0; pat<2; pat++)
          {
          XDrawRectangle (PDrawDisplay,
            PDrawTool_Pixmap_Get (&GPDrawToolCB, pat), gc,
            RxnDsp_AtomXCoord_Get (dratm_p, gsg) - (PDRW_HILIT_AREA << 1), 
            RxnDsp_AtomYCoord_Get (dratm_p, gsg) - (PDRW_HILIT_AREA << 1),
            PDRW_HILIT_AREA << 2, PDRW_HILIT_AREA << 2);
          XCopyArea (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, pat), 
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)), gc, 0, 0, 
            PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);
          }

       /* da_wid = retrieve_widget_id ("working_da", 1);
        XUndefineCursor (XtDisplay (da_wid), XtWindow (da_wid));*/
        
        RxnDsp_RxncPtr_Put (PDrawMol_p, dratm_p);
        PDrawFlags.drawing_mode = PDRW_BOND_FLAG;
        PDraw_Mode_Reset (PDRW_BOND_FLAG);
        }
      break;  /* End of PDRW_RXNCNTR_FLAG */
    }/*End of switch drawing mode */

  /* General actions need to be done regardless drawing mode 
     (except selecting drawing mode), if motion event occurs.
   */
  if ((strcmp (args[0], "motion") == 0) 
      && (PDrawFlags.drawing_mode != PDRW_SELECT_FLAG)) 
    {
    /* If there is an atom at this point, highlight it.
     */    
    if ((dratm_p = RxnDsp_AtomThere_Get (PDrawMol_p, gsg, event->x, event->y)) 
	&& !(PDrawBondsInfo->in_atom_area)) 
      {
      switch (PDrawFlags.drawing_mode) 
        {
        case PDRW_BOND_FLAG:
	  XSetForeground (PDrawDisplay, gc, 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_GREEN));
	  break;
        case PDRW_ATOM_FLAG:
	  XSetForeground (PDrawDisplay, gc, 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_VIOLET));
	  break;
        case PDRW_SELECTROOT_FLAG:
	  XSetForeground (PDrawDisplay, gc, 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLUE));
          break;
        case PDRW_DELETEROOT_FLAG:
	  XSetForeground (PDrawDisplay, gc, 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
          break;
        case PDRW_CLEAR_FLAG:
	  XSetForeground (PDrawDisplay, gc, 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_RED));
	  break;
        case PDRW_RXNCNTR_FLAG:
	  XSetForeground (PDrawDisplay, gc, 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_GOLD));
	  break;
        }

      XFillRectangle (PDrawDisplay, 
        XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc,
        RxnDsp_AtomXCoord_Get (dratm_p, gsg) - PDRW_HILIT_AREA,
        RxnDsp_AtomYCoord_Get (dratm_p, gsg) - PDRW_HILIT_AREA,     
        (PDRW_HILIT_AREA << 1), (PDRW_HILIT_AREA << 1));

      PDrawBondsInfo->in_atom_area = TRUE;
      PDrawBondsInfo->in_bond_area = FALSE;

      /* If there is "clear" drawing mode, highlight all bonds adjacent
	 to this atom.
       */
      if (PDrawFlags.drawing_mode == PDRW_CLEAR_FLAG) 
        {
	/* Highlight bonds */
	for (adj_i = 0; adj_i < dratm_p->adj_info.num_neighbs; adj_i++) 
          {
	  adj_j = (int) dratm_p->adj_info.adj_atms[adj_i];
	  drbnd_p = RxnDsp_BondThere_Get (PDrawMol_p, gsg,
            RxnDsp_AtomXCoord_Get (dratm_p, gsg),
            RxnDsp_AtomYCoord_Get (dratm_p, gsg), gsg,
            RxnDsp_AtomXCoord_Get (RxnDsp_AtomPtr_Get (PDrawMol_p, adj_j),
              gsg),
            RxnDsp_AtomYCoord_Get (RxnDsp_AtomPtr_Get (PDrawMol_p, adj_j),
              gsg));
	  pdraw_Bond (PDrawDisplay, 
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc, 
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_RED), PDrawMol_p, 
            drbnd_p, gsg, TRUE, TRUE);	  
          if (gsgsame)
	    pdraw_Bond (PDrawDisplay, 
              XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, 1 - gsg)), gc, 
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_RED), PDrawMol_p, 
              drbnd_p, 1 - gsg, TRUE, TRUE);	  
	  }
        } /* End hilite bonds in "clear" mode */

      XSetForeground (PDrawDisplay, gc, 
        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
      }

    /* If we get out of the scope of atom, erase highlighted area 
     */
    if (!(dratm_p = RxnDsp_AtomThere_Get (PDrawMol_p, gsg, event->x, event->y)) 
	&& (PDrawBondsInfo->in_atom_area)) 
      {
      /* Copy the Pixmap in order to erase the previously hihglighted areas */
      XCopyArea (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), 
        XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc,
        0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);
      if (gsgsame)
        XCopyArea (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, 1 - gsg), 
          XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, 1 - gsg)), gc,
          0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);
      PDrawBondsInfo->in_atom_area = FALSE;
      }

    if (PDrawFlags.drawing_mode == PDRW_CLEAR_FLAG) 
      {
      /* If there is a bond at this point, highlight it.
       */    
      if ((drbnd_p = pin_Bond_Scope (PDrawMol_p, event->x, event->y))
	  && !PDrawBondsInfo->in_bond_area && !PDrawBondsInfo->in_atom_area) 
        {
	pdraw_Bond (PDrawDisplay, 
          XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc,
          SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_RED), PDrawMol_p, 
          drbnd_p, gsg, TRUE, TRUE);
        if (gsgsame)
	  pdraw_Bond (PDrawDisplay, 
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, 1 - gsg)), gc,
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_RED), PDrawMol_p, 
            drbnd_p, 1 - gsg, TRUE, TRUE);
	PDrawBondsInfo->in_bond_area = TRUE;
        }

      /* If we get out of the scope of bond, erase highlighted area 
       */
      if (!(drbnd_p = pin_Bond_Scope (PDrawMol_p, event->x, event->y)) 
	  && (PDrawBondsInfo->in_bond_area)) 
        {
	/* Copy Pixmap in order to erase the previously highlighted areas */
	XCopyArea (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), 
          XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), gc,
          0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);
        if (gsgsame)
	  XCopyArea (PDrawDisplay,
            PDrawTool_Pixmap_Get (&GPDrawToolCB, 1 - gsg),
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, 1 - gsg)), gc,
            0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);
	PDrawBondsInfo->in_bond_area = FALSE;
        }
      } /* End of not in the PDRW_BOND mode */
    } /* End of "general motion" */

  return ; 
}
/*  End of PDraw_Draw  */

/****************************************************************************
*  
*  Function Name:                 pmark_selected
*  
*****************************************************************************/  

void pmark_selected 
  (
  RxnDsp_Molecule_t  *mol_p,
  Display         *disp, 
  Drawable         drbl, 
  GC               gc
  )
{
  RxnDsp_Atom_t   *slatm1_p, *slatm2_p, *atom_p;
  RxnDsp_Bond_t   *bond_p;
  int              max_x, max_y;
  int              atm_i, adj_i, ii;
  int              neighb_count;

  PDrawSelInfo->isSelected = FALSE;
  PDrawSelInfo->selected_x = PDrawSelInfo->selected_y = 0;
  max_x = max_y = 0;

  slatm1_p = RxnDsp_AtomPtr_Get (mol_p, 0);
  for (atm_i = 0; atm_i < mol_p->both_dm.natms; atm_i++) 
    {
    if (slatm1_p->xp && slatm1_p->xn && slatm1_p->yp && slatm1_p->yn) 
      {
      neighb_count = 0;

      /* If atom is selected and it doesn't have neighbors,
	 include it in selected atoms.
       */
      if (slatm1_p->adj_info.num_neighbs == 0) 
        {
/* NOTE: Marked here */
	PDrawSelInfo->isSelected = TRUE;
	RxnDsp_AtomSelect_Put (slatm1_p, gsg, PDRW_SELECT_TOTAL);

	pdraw_Atom (disp, drbl, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
          SAR_CLRI_BLUE), slatm1_p, PDRW_ATOM_SIZE_LRG);

	/* Update info about left upper corner of selected area */
	if ((RxnDsp_AtomXCoord_Get (slatm1_p, gsg) < PDrawSelInfo->selected_x) 
	    || (PDrawSelInfo->selected_x == 0))
	  PDrawSelInfo->selected_x =
            RxnDsp_AtomXCoord_Get (slatm1_p, gsg);

	if ((RxnDsp_AtomYCoord_Get (slatm1_p, gsg) < PDrawSelInfo->selected_y) 
	    || (PDrawSelInfo->selected_y == 0))
	  PDrawSelInfo->selected_y = RxnDsp_AtomYCoord_Get (slatm1_p, gsg);

	/* Update info about right buttom corner of selected area */
	if (RxnDsp_AtomXCoord_Get (slatm1_p, gsg) > max_x)
	  max_x = RxnDsp_AtomXCoord_Get (slatm1_p, gsg);

	if (RxnDsp_AtomYCoord_Get (slatm1_p, gsg) > max_y)
	  max_y = RxnDsp_AtomYCoord_Get (slatm1_p, gsg);

        } /* End if atom has no neighbors */

      else 
        {
	/* If atom is selected and has neighbors, 
	   check whether its neighbors are selected 
	 */
	for (adj_i = 0; adj_i < slatm1_p->adj_info.num_neighbs; adj_i++) 
          {
	  slatm2_p =
            RxnDsp_AtomPtr_Get (mol_p, slatm1_p->adj_info.adj_atms[adj_i]);
	  if (slatm2_p->xp && slatm2_p->xn && slatm2_p->yp && slatm2_p->yn) 
            {
	    /* If neighbor atom is selected too, increment neighbor counter
	       and draw bond that connects selected neighbors
	     */
	    PDrawSelInfo->isSelected = TRUE;
	    RxnDsp_AtomSelect_Put (slatm1_p, gsg, DRW_SELECT_MOVE);
	    ++neighb_count;
	    bond_p = RxnDsp_BondThere_Get (mol_p, gsg,
              RxnDsp_AtomXCoord_Get (slatm1_p, gsg),
              RxnDsp_AtomYCoord_Get (slatm1_p, gsg),
              gsg,
              RxnDsp_AtomXCoord_Get (slatm2_p, gsg),
              RxnDsp_AtomYCoord_Get (slatm2_p, gsg));
	    pdraw_Bond (disp, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
              SAR_CLRI_BLUE), mol_p, bond_p, gsg, FALSE, FALSE);

	    /* Update info about left upper corner of selected area */
	    if ((RxnDsp_AtomXCoord_Get (slatm1_p, gsg) <
                 PDrawSelInfo->selected_x) 
		|| (PDrawSelInfo->selected_x == 0))
	      PDrawSelInfo->selected_x = RxnDsp_AtomXCoord_Get (slatm1_p, gsg);

	    if ((RxnDsp_AtomYCoord_Get (slatm1_p, gsg) <
                   PDrawSelInfo->selected_y) 
		|| (PDrawSelInfo->selected_y == 0))
	      PDrawSelInfo->selected_y = RxnDsp_AtomYCoord_Get (slatm1_p, gsg);

	    if ((RxnDsp_AtomXCoord_Get (slatm2_p, gsg) <
                   PDrawSelInfo->selected_x) 
		|| (PDrawSelInfo->selected_x == 0))
	      PDrawSelInfo->selected_x = RxnDsp_AtomXCoord_Get (slatm2_p, gsg);

	    if ((RxnDsp_AtomYCoord_Get (slatm2_p, gsg) <
                   PDrawSelInfo->selected_y) 
		|| (PDrawSelInfo->selected_y == 0))
	      PDrawSelInfo->selected_y = RxnDsp_AtomYCoord_Get (slatm2_p, gsg);

	    /* Update info about right buttom corner of selected area */
	    if (RxnDsp_AtomXCoord_Get (slatm1_p, gsg) > max_x)
	      max_x = RxnDsp_AtomXCoord_Get (slatm1_p, gsg);

	    if (RxnDsp_AtomYCoord_Get (slatm1_p, gsg) > max_y)
	      max_y = RxnDsp_AtomYCoord_Get (slatm1_p, gsg);

	    if (RxnDsp_AtomXCoord_Get (slatm2_p, gsg) > max_x)
	      max_x = RxnDsp_AtomXCoord_Get (slatm2_p, gsg);

	    if (RxnDsp_AtomYCoord_Get (slatm2_p, gsg) > max_y)
	      max_y = RxnDsp_AtomYCoord_Get (slatm2_p, gsg);
            } /* End if neighbor is selected */
          else if (!gsgsame)
            {
            /* if null-bonded to this neighbor, simply increment count */
	    bond_p = RxnDsp_BondThere_Get (mol_p, gsg,
              RxnDsp_AtomXCoord_Get (slatm1_p, gsg),
              RxnDsp_AtomYCoord_Get (slatm1_p, gsg),
              gsg,
              RxnDsp_AtomXCoord_Get (slatm2_p, gsg),
              RxnDsp_AtomYCoord_Get (slatm2_p, gsg));
            if (RxnDsp_BondOrder_Get (bond_p, gsg) == 0)
              neighb_count++;
            }
          } /* End for all neighbors */

	/* Increment isSelected field of all bonds that adjacent to 
	   slatm1_p. Since we scan all atoms, bond that coonects
	   selected atoms will be incremented twice. "Move" bonds
	   will be incremented only once.
	 */
	if (RxnDsp_AtomSelect_Get (slatm1_p,gsg) == PDRW_SELECT_MOVE)
	  for (ii = 0; ii < slatm1_p->adj_info.num_neighbs; ii++) 
            {
	    atom_p =
              RxnDsp_AtomPtr_Get (mol_p, slatm1_p->adj_info.adj_atms[ii]);
	    bond_p = RxnDsp_BondThere_Get (mol_p, gsg,
              RxnDsp_AtomXCoord_Get (slatm1_p, gsg),
              RxnDsp_AtomYCoord_Get (slatm1_p, gsg), gsg,
              RxnDsp_AtomXCoord_Get (atom_p, gsg),
              RxnDsp_AtomYCoord_Get (atom_p, gsg));
            RxnDsp_BondSelect_Put (bond_p, gsg,
              RxnDsp_BondSelect_Get (bond_p, gsg) + 1);
	    }

	/* If its all neighbors are selected, mark it and draw it. */
	if (neighb_count == slatm1_p->adj_info.num_neighbs) 
          {
/* NOTE: marked here too */
	  RxnDsp_AtomSelect_Put (slatm1_p, gsg, PDRW_SELECT_TOTAL);

	  pdraw_Atom (disp, drbl, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
            SAR_CLRI_BLUE), slatm1_p, PDRW_ATOM_SIZE_LRG);
	  }
        } /* End if atom has neighbors (else) */
      } /* End if atom is selected */

    ++slatm1_p;
    }/* End for all atoms */

  /* If bond(s) was selected, mark that area */
  if (PDrawSelInfo->isSelected) 
    {
    PDrawSelInfo->sel_width = max_x - PDrawSelInfo->selected_x 
      + (PDRW_HILIT_AREA << 1) - 1;
    PDrawSelInfo->sel_height = max_y - PDrawSelInfo->selected_y
      + (PDRW_HILIT_AREA << 1) - 1;

    /* Draw a rectangle around selected area */
    XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLUE));
    XDrawRectangle (disp, drbl, gc, PDrawSelInfo->selected_x - PDRW_HILIT_AREA,
      PDrawSelInfo->selected_y - PDRW_HILIT_AREA, PDrawSelInfo->sel_width,    
      PDrawSelInfo->sel_height);
    XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
    }
  return ;
}
/*  End of pmark_selected  */

/****************************************************************************
*  
*  Function Name:                 punmark_selected
*  
*****************************************************************************/  

void punmark_selected 
  (
  RxnDsp_Molecule_t *mol_p,
  Display           *disp, 
  Drawable           drbl1,
  Drawable           drbl2,
  GC                 gc)
{
  RxnDsp_Atom_t     *atom_p;
  RxnDsp_Bond_t     *bond_p;
  int                ii;

  if (PDrawSelInfo->isSelected)
    pdraw_selected (mol_p, disp, drbl1, gc,
      SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), 0, 0, FALSE);

  XCopyArea (PDrawDisplay, drbl1, drbl2, gc, 0, 0, PDRW_DA_PXMP_WD,
    PDRW_DA_PXMP_HT, 0, 0);
  PDrawSelInfo->isSelected = FALSE;
  PDrawSelInfo->move_it = TRUE;

  /* Make all atoms and bonds unselected */
  atom_p = RxnDsp_AtomPtr_Get (mol_p, 0);
  for (ii = 0; ii < mol_p->both_dm.natms; ii++) 
    {
    atom_p->xp = FALSE;
    atom_p->xn = FALSE;
    atom_p->yp = FALSE;
    atom_p->yn = FALSE;
    RxnDsp_AtomSelect_Put (atom_p, gsg, PDRW_SELECT_NONE);
    ++atom_p;
    }

  bond_p = RxnDsp_BondPtr_Get (mol_p, 0);
  for (ii = 0; ii < mol_p->both_dm.nbnds; ii++) 
    {
    RxnDsp_BondSelect_Put (bond_p, gsg, PDRW_SELECT_NONE);
    ++bond_p;
    }

  return ;
}
/*  End of punmark_selected  */

/****************************************************************************
*  
*  Function Name:                 pdraw_selected
*  
*****************************************************************************/  

void pdraw_selected 
  (
  RxnDsp_Molecule_t *mol_p,
  Display           *disp, 
  Drawable           drbl, 
  GC                 gc,
  XColor             color,
  int                delta_x,
  int                delta_y,
  Boolean_t          draw_rect)
{
  RxnDsp_Atom_t     *dratm_p;
  RxnDsp_Bond_t     *drbnd_p;
  int                ii;

  /* Erase previous selected area, first */
  drbnd_p = RxnDsp_BondPtr_Get (mol_p, 0);
  for (ii = 0; ii < mol_p->both_dm.nbnds; ii++) 
    {
    if (RxnDsp_BondSelect_Get (drbnd_p, gsg) != PDRW_SELECT_NONE)
      pdraw_Bond (disp, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
        SAR_CLRI_WHITE), mol_p, drbnd_p, gsg, FALSE, FALSE);

    ++drbnd_p;
    }

  dratm_p = RxnDsp_AtomPtr_Get (mol_p, 0);
  for (ii = 0; ii < mol_p->both_dm.natms; ii++) 
    {
    if (RxnDsp_AtomSelect_Get (dratm_p, gsg) != DRW_SELECT_NONE)
      pdraw_Atom (disp, drbl, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
        SAR_CLRI_WHITE), dratm_p, PDRW_ATOM_SIZE_LRG);

    ++dratm_p;
    }

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
  XDrawRectangle (disp, drbl, gc, PDrawSelInfo->selected_x - PDRW_HILIT_AREA,
    PDrawSelInfo->selected_y - PDRW_HILIT_AREA, PDrawSelInfo->sel_width,    
    PDrawSelInfo->sel_height);

  /* Now redraw selected area in the new place specified by deltas */
  dratm_p = RxnDsp_AtomPtr_Get (mol_p, 0);
  for (ii = 0; ii < mol_p->both_dm.natms; ii++) 
    {
    if (RxnDsp_AtomSelect_Get (dratm_p, gsg) != PDRW_SELECT_NONE) 
      {
      RxnDsp_AtomXCoord_Put (dratm_p, gsg,
        RxnDsp_AtomXCoord_Get (dratm_p, gsg) + delta_x); 
      RxnDsp_AtomYCoord_Put (dratm_p, gsg,
        RxnDsp_AtomYCoord_Get (dratm_p, gsg) + delta_y); 
      } 

    ++dratm_p;
    }

  drbnd_p = RxnDsp_BondPtr_Get (mol_p, 0);
  for (ii = 0; ii < mol_p->both_dm.nbnds; ii++) 
    {
    if (RxnDsp_BondSelect_Get (drbnd_p, gsg) == PDRW_SELECT_TOTAL)
      pdraw_Bond (disp, drbl, gc, color, mol_p, drbnd_p, gsg, FALSE, FALSE);      

    if (RxnDsp_BondSelect_Get (drbnd_p, gsg) == PDRW_SELECT_MOVE) 
      {
      if (RxnDsp_AtomSelect_Get (drbnd_p->latm_p, gsg) == PDRW_SELECT_MOVE) 
        {
	if (draw_rect)
	  pdraw_Bond (disp, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
            SAR_CLRI_BLACK), mol_p, drbnd_p, gsg, FALSE, TRUE);
	else
	  pdraw_Bond (disp, drbl, gc, color, mol_p, drbnd_p, gsg, FALSE, TRUE);
        }

      if (RxnDsp_AtomSelect_Get (drbnd_p->ratm_p, gsg) == PDRW_SELECT_MOVE) 
        {
	if (draw_rect)
	  pdraw_Bond (disp, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
            SAR_CLRI_BLACK), mol_p, drbnd_p, gsg, TRUE, FALSE);
	else
	  pdraw_Bond (disp, drbl, gc, color, mol_p, drbnd_p, gsg, TRUE, FALSE);
        }
      }

    ++drbnd_p;
    } /* End for all bonds */

  dratm_p = RxnDsp_AtomPtr_Get (mol_p, 0);
  for (ii = 0; ii < mol_p->both_dm.natms; ii++) 
    {
    if (RxnDsp_AtomSelect_Get (dratm_p, gsg) != PDRW_SELECT_NONE)
      pdraw_Atom (disp, drbl, drbl, gc, color, dratm_p, PDRW_ATOM_SIZE_LRG);

    ++dratm_p;
    }

  if (draw_rect) 
    {
    PDrawSelInfo->selected_x += delta_x; 
    PDrawSelInfo->selected_y += delta_y; 
    XDrawRectangle (disp, drbl, gc, PDrawSelInfo->selected_x - PDRW_HILIT_AREA,
      PDrawSelInfo->selected_y - PDRW_HILIT_AREA, PDrawSelInfo->sel_width,    
      PDrawSelInfo->sel_height);
    }

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
  return ;
}
/*  End of pdraw_selected  */

/****************************************************************************
*  
*  Function Name:                 pin_Bond_Scope
*  
*****************************************************************************/  

RxnDsp_Bond_t *pin_Bond_Scope 
  (
  RxnDsp_Molecule_t *mol_p,
  int                x, 
  int                y
  )
{
  float          k, l;
  int            x0, y0, yy;
  int            x1, x2, y1, y2;
  int            bnd_i;
  RxnDsp_Bond_t *bnd_p;

  bnd_p = RxnDsp_BondPtr_Get (mol_p, 0);
  for (bnd_i = 0; bnd_i < mol_p->both_dm.nbnds; bnd_i++)
    {
    if ((((x < RxnDsp_AtomXCoord_Get (bnd_p->latm_p, gsg) + PDRW_HILIT_AREA) && 
	  (x > RxnDsp_AtomXCoord_Get (bnd_p->ratm_p, gsg) - PDRW_HILIT_AREA)) ||
	 ((x < RxnDsp_AtomXCoord_Get (bnd_p->ratm_p, gsg) + PDRW_HILIT_AREA) && 
	  (x > RxnDsp_AtomXCoord_Get (bnd_p->latm_p, gsg) - PDRW_HILIT_AREA)))
      &&
	(((y < RxnDsp_AtomYCoord_Get (bnd_p->latm_p, gsg) + PDRW_HILIT_AREA) && 
	  (y > RxnDsp_AtomYCoord_Get (bnd_p->ratm_p, gsg) - PDRW_HILIT_AREA)) ||
	 ((y < RxnDsp_AtomYCoord_Get (bnd_p->ratm_p, gsg) + PDRW_HILIT_AREA) && 
	  (y > RxnDsp_AtomYCoord_Get (bnd_p->latm_p, gsg) - PDRW_HILIT_AREA)))) 
      {
      if (abs(RxnDsp_AtomYCoord_Get (bnd_p->ratm_p, gsg) -
            RxnDsp_AtomYCoord_Get (bnd_p->latm_p, gsg)) < 
	  abs(RxnDsp_AtomXCoord_Get (bnd_p->ratm_p, gsg) -
            RxnDsp_AtomXCoord_Get (bnd_p->latm_p, gsg))) 
        {
	x0 = x;
	y0 = y;
	x1 = RxnDsp_AtomXCoord_Get (bnd_p->latm_p, gsg);
	x2 = RxnDsp_AtomXCoord_Get (bnd_p->ratm_p, gsg);
	y1 = RxnDsp_AtomYCoord_Get (bnd_p->latm_p, gsg);
	y2 = RxnDsp_AtomYCoord_Get (bnd_p->ratm_p, gsg);
        }
      else 
        {
	x0 = y;
	y0 = x;
	x1 = RxnDsp_AtomYCoord_Get (bnd_p->latm_p, gsg);
	x2 = RxnDsp_AtomYCoord_Get (bnd_p->ratm_p, gsg);
	y1 = RxnDsp_AtomXCoord_Get (bnd_p->latm_p, gsg);
	y2 = RxnDsp_AtomXCoord_Get (bnd_p->ratm_p, gsg);
        }

      k = (float) (y2 - y1) / (float) (x2 - x1);
      l = y1 - (k * x1);
      yy = (int) ( (float) k * x0 + l);
      if ((y0 < yy + PDRW_HILIT_AREA) && (y0 > yy - PDRW_HILIT_AREA))
	return (bnd_p);
      }

    ++bnd_p;
    }

  return ((RxnDsp_Bond_t *) NULL);
}
/*  End of pin_Bond_Scope  */

/****************************************************************************
*  
*  Function Name:                 PAtom_Mark_CB
*  
*****************************************************************************/  

void PAtom_Mark_CB 
  (
  Widget    w,
  XtPointer client_data,
  XtPointer call_data
  )
{
  RxnDsp_Atom_t  *dratm_p, *atom_p;
  RxnDsp_Bond_t  *bond_p;
  String          tag_string; 
  int             adj_i, pat;

  tag_string = (String) client_data;

  if (strcmp ((char *) tag_string, CS_PMULTIPLE) == 0) 
    {
    PDrawFlags.drawing_mode = PDRW_ATOM_FLAG;
    PDrawBondsInfo->esc_mode = TRUE;
    strcpy (PDrawFlags.cur_atmsym, "C");
    return;
    }

  if (strcmp ((char *) tag_string, CS_PDISMISS) == 0) 
    {
    PDrawFlags.drawing_mode = PDRW_BOND_FLAG;
    PDrawBondsInfo->esc_mode = TRUE;
    strcpy (PDrawFlags.cur_atmsym, "C");
    PDraw_Mode_Reset (PDRW_BOND_FLAG);
    XtUnmanageChild (PDrawTool_PeriodicTbl_Get (&GPDrawToolCB));
    return;
    } 

printf("PAtom_Mark_CB: IsThere_Draw_Flag=%d glob_rxlform=%d\n",IsThere_Draw_Flag,glob_rxlform);
  /* If callback was activated by a pushbutton in "Draw Atom" mode,
     store cur_atmsym.
   */
  if ((PDrawFlags.drawing_mode == PDRW_ATOM_FLAG) && (w != (Widget) NULL)) 
    {
    strcpy (PDrawFlags.cur_atmsym, tag_string);
    return;
    }

  if (!(dratm_p = RxnDsp_AtomThere_Get (PDrawMol_p, PDrawBondsInfo->clicked_gsg,
       PDrawBondsInfo->clicked_x, PDrawBondsInfo->clicked_y)))
    return;

  /* Erase previous atom symbols */
  pdraw_Atom (PDrawDisplay,
    XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, GOAL)), 
    XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, SUBG)), 
    PDrawTool_GC_Get (&GPDrawToolCB), 
    SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), dratm_p, 
    PDRW_ATOM_SIZE_LRG);

  pdraw_Atom (PDrawDisplay,
    PDrawTool_Pixmap_Get (&GPDrawToolCB, GOAL), 
    PDrawTool_Pixmap_Get (&GPDrawToolCB, SUBG), 
    PDrawTool_GC_Get (&GPDrawToolCB), 
    SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), dratm_p, 
    PDRW_ATOM_SIZE_LRG);

  if (strcmp ((char *) tag_string, "+") == 0)
    if (gsgsame) for (pat=0; pat<2; pat++)
      RxnDsp_AtomCharge_Put (dratm_p, pat, tag_string[0]);
    else
      RxnDsp_AtomCharge_Put (dratm_p, PDrawBondsInfo->clicked_gsg,
        tag_string[0]);

  else
    {
    strcpy (dratm_p->sym, (char *) tag_string);
    if (strcmp ((char *) tag_string, "C") == 0) 
      {
      dratm_p->isC = TRUE;

      /* Redraw all bonds adjacent to this atom 
	 in order to fill space left from atom symbol
       */
      for (adj_i = 0; adj_i < dratm_p->adj_info.num_neighbs; adj_i++) 
        {
	atom_p = RxnDsp_AtomPtr_Get (PDrawMol_p,
          (int) dratm_p->adj_info.adj_atms[adj_i]);
	for (pat=0; pat<2; pat++)
          {
          bond_p = RxnDsp_BondThere_Get (PDrawMol_p, pat,
            RxnDsp_AtomXCoord_Get (dratm_p, pat),
            RxnDsp_AtomYCoord_Get (dratm_p, pat), pat,
            RxnDsp_AtomXCoord_Get (atom_p, pat),
            RxnDsp_AtomYCoord_Get (atom_p, pat));
	  if (bond_p != NULL)
            {
	    pdraw_Bond (PDrawDisplay, 
              XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)), 
              PDrawTool_GC_Get (&GPDrawToolCB),
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), PDrawMol_p, 
              bond_p, pat, TRUE, TRUE);
	    pdraw_Bond (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, pat),
              PDrawTool_GC_Get (&GPDrawToolCB), 
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), PDrawMol_p, 
              bond_p, pat, TRUE, TRUE);
            }
	  }
        }
      }

    else
      dratm_p->isC = FALSE;
    }

  pdraw_Atom (PDrawDisplay, 
    XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, GOAL)), 
    XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, SUBG)), 
    PDrawTool_GC_Get (&GPDrawToolCB), 
    SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), dratm_p, 
    PDRW_ATOM_SIZE_LRG);

  pdraw_Atom (PDrawDisplay,
    PDrawTool_Pixmap_Get (&GPDrawToolCB, GOAL), 
    PDrawTool_Pixmap_Get (&GPDrawToolCB, SUBG), 
    PDrawTool_GC_Get (&GPDrawToolCB), 
    SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), dratm_p,
    PDRW_ATOM_SIZE_LRG);
}
/*  End of PAtom_Mark_CB  */

/****************************************************************************
*  
*  Function Name:                 patom_pos_to_pointer_pos
*  
*****************************************************************************/  

int patom_pos_to_pointer_pos 
  (
  int atom_x, 
  int atom_y,
  int point_x, 
  int point_y
  )
{
  if ((atom_x <= point_x) && (atom_y >= point_y))
    return (1);

  if ((atom_x > point_x) && (atom_y > point_y))
    return (2);

  if ((atom_x >  point_x) && (atom_y <= point_y))
    return (3);

  if ((atom_x <= point_x) && (atom_y <  point_y))
    return (4);

  return (0);
}
/*  End of patom_pos_to_pointer_pos  */

/****************************************************************************
*  
*  Function Name:                 pcheck_kvadr
*  
*****************************************************************************/  

void pcheck_kvadr 
  (
  RxnDsp_Molecule_t *mol_p,
  int                point_x, 
  int                point_y,
  int                old_x, 
  int                old_y
  )
{
  RxnDsp_Atom_t     *atom_p;
  int                new_kvadr;
  int                ii;

  atom_p = RxnDsp_AtomPtr_Get (mol_p, 0);
  for (ii = 0; ii < mol_p->both_dm.natms; ii++) 
    {
    /* If kvadrant of pointer has been changed */
    if (atom_p->kvadr != (new_kvadr = patom_pos_to_pointer_pos (
         RxnDsp_AtomXCoord_Get (atom_p, PDrawSelInfo->selected_gsg),
         RxnDsp_AtomYCoord_Get (atom_p, PDrawSelInfo->selected_gsg), point_x, point_y))) 
      {
      switch (atom_p->kvadr) 
        {
        case 1:
	  switch (new_kvadr) 
            {
	    case 2:
	      atom_p->yn = pnot(atom_p->yn);
	      break;
	    case 3:
	      if (PDRW_Slope (RxnDsp_AtomXCoord_Get (atom_p,
                    PDrawSelInfo->selected_gsg),
                    point_x, point_y,old_x, old_y) 
                  < RxnDsp_AtomYCoord_Get (atom_p, PDrawSelInfo->selected_gsg)) 
                {
	        atom_p->yn = pnot(atom_p->yn);
	        atom_p->xn = pnot(atom_p->xn);
	        } 
	      else 
                {
	        atom_p->yp = pnot(atom_p->yp);
	        atom_p->xp = pnot(atom_p->xp);
	        }
	      break;
	    case 4:
	      atom_p->xp = pnot(atom_p->xp);
	      break;
	    }
	  break;

        case 2:
	  switch (new_kvadr) 
            {
	    case 1:
	      atom_p->yn = pnot(atom_p->yn);
	      break;
	    case 3:
	      atom_p->xn = pnot(atom_p->xn);
	      break;
	    case 4:
	      if (PDRW_Slope(RxnDsp_AtomXCoord_Get (atom_p,
                    PDrawSelInfo->selected_gsg),
                    point_x, point_y, old_x, old_y) 
                  < RxnDsp_AtomYCoord_Get (atom_p, PDrawSelInfo->selected_gsg)) 
                {
	        atom_p->yn = pnot(atom_p->yn);
	        atom_p->xp = pnot(atom_p->xp);
	        }
	      else 
                {
	        atom_p->yp = pnot(atom_p->yp);
	        atom_p->xn = pnot(atom_p->xn);
	        }
	      break;
	     }
	  break;

        case 3:
	  switch (new_kvadr) 
            {
	    case 1:
	      if (PDRW_Slope(RxnDsp_AtomXCoord_Get (atom_p,
                    PDrawSelInfo->selected_gsg),
                    point_x, point_y, old_x, old_y) 
                  < RxnDsp_AtomYCoord_Get (atom_p, PDrawSelInfo->selected_gsg)) 
                {
	        atom_p->yn = pnot(atom_p->yn);
	        atom_p->xn = pnot(atom_p->xn);
	        } 
	      else 
                {
	        atom_p->yp = pnot(atom_p->yp);
	        atom_p->xp = pnot(atom_p->xp);
	        }
	      break;
	    case 2:
	      atom_p->xn = pnot(atom_p->xn);
	      break;
	    case 4:
	      atom_p->yp = pnot(atom_p->yp);
	      break;
	    }
	  break;

        case 4:
	  switch (new_kvadr) 
            {
	    case 1:
	      atom_p->xp = pnot(atom_p->xp);
	      break;
	    case 2:
	      if (PDRW_Slope(RxnDsp_AtomXCoord_Get (atom_p,
                    PDrawSelInfo->selected_gsg),
                    point_x, point_y, old_x, old_y) 
                  < RxnDsp_AtomYCoord_Get (atom_p, PDrawSelInfo->selected_gsg)) 
                {
	        atom_p->yn = pnot(atom_p->yn);
	        atom_p->xp = pnot(atom_p->xp);
	        }
	      else 
                {
	        atom_p->yp = pnot(atom_p->yp);
	        atom_p->xn = pnot(atom_p->xn);
	        }
	      break;
            case 3:
	      atom_p->yp = pnot(atom_p->yp);
	      break;
	    }
	  break;
        } /* End of main switch (atom_p->kvadr) */

      atom_p->kvadr = new_kvadr;
      } /* End of if */

    ++atom_p;
    } /* End of for all atoms */

  return ;
}
/*  End of pcheck_kvadr  */

/****************************************************************************
*  
*  Function Name:                 POption_Choose_CB
*  
*****************************************************************************/  

void POption_Choose_CB  
  (
  Widget         w,
  XtPointer      client_data,
  XtPointer      call_data
  )
{
/* to prevent phantom button press when resetting or interruption of root
   selection */
static Boolean_t selecting_root = FALSE;
static Boolean_t deleting_root = FALSE;
  int            tag, pat, i, savegsg;
  Dimension      sw_h, sw_w;
  Widget         da_wid[2];
  Boolean_t      save_sel;

  tag = (int) client_data;

  PDrawFlags.drawing_mode = tag;
  PDrawBondsInfo->esc_mode = TRUE;

  /* If smth was selected prior, unselect it */

  save_sel = PDrawSelInfo->isSelected;
  for (savegsg=gsg, gsg=0; gsg<2; gsg++)
    {
    da_wid[gsg] = PDrawTool_DrawArea_Get(&GPDrawToolCB, gsg);
    PDrawSelInfo->isSelected = save_sel;
    punmark_selected (PDrawMol_p, PDrawDisplay,
      PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), 
      XtWindow (da_wid[gsg]), PDrawTool_GC_Get (&GPDrawToolCB));
    }
  gsg = savegsg;

  if (selecting_root && tag != PDRW_SELECTROOT_FLAG)
    {
/*
    if (XtIsManaged (PDrawTool_SynListForm_Get (&GPDrawToolCB)))
      XtUnmanageChild (PDrawTool_SynListForm_Get (&GPDrawToolCB));
    if (XtIsManaged (PDrawTool_SlingListForm_Get (&GPDrawToolCB)))
      XtUnmanageChild (PDrawTool_SlingListForm_Get (&GPDrawToolCB));
*/
    Sy_Form_Close ();
    selecting_root = synlist_managed = FALSE;
    }

  if (deleting_root && tag != PDRW_DELETEROOT_FLAG)
    deleting_root = FALSE;

  switch (tag) 
    {
    case PDRW_BOND_FLAG:
        for (pat=0; pat<2; pat++)
        XUndefineCursor (XtDisplay (da_wid[pat]), XtWindow (da_wid[pat]));

      XSetForeground (PDrawDisplay, PDrawTool_GC_Get(&GPDrawToolCB), 
        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
      PDrawBondsInfo->cur_bnd_p = (RxnDsp_Bond_t *) NULL;
      PDrawBondsInfo->in_atom_area = FALSE;
      PDrawBondsInfo->in_bond_area = FALSE;
      break;

    case PDRW_ATOM_FLAG:
      for (pat=0; pat<2; pat++)
        XUndefineCursor (XtDisplay (da_wid[pat]), XtWindow (da_wid[pat]));
      
/*
      InfoWarn_Show ("periodic_table_dialog");
*/
      PDrawFlags.drawing_mode = PDRW_BOND_FLAG;
      PDraw_Mode_Reset (PDRW_BOND_FLAG);
      break;

    case PDRW_CLEAR_FLAG:
      for (pat=0; pat<2; pat++)
        XDefineCursor (XtDisplay (da_wid[pat]), XtWindow (da_wid[pat]), 
          SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_ERASE));
      break;

    case PDRW_SELECT_FLAG:
      for (pat=0; pat<2; pat++)
        XDefineCursor (XtDisplay (da_wid[pat]), XtWindow (da_wid[pat]), 
          SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_WRITE));
     
      PDrawSelInfo->isSelected = FALSE;
      PDrawSelInfo->move_it = FALSE;
      break;

    case PDRW_REDRAW_FLAG:
      if (rxndsp_Shelley (PDrawMol_p)) for (pat=0; pat<2; pat++)
        {
        for (i = 0; i < PDrawMol_p->both_dm.natms; i++)
          RxnDsp_AtomCharge_Put (RxnDsp_AtomPtr_Get (PDrawMol_p, i), pat,
            nodenums_on ? i + 1 : 0);

        XUndefineCursor (XtDisplay (da_wid[pat]), XtWindow (da_wid[pat]));

        XSetForeground (PDrawDisplay, PDrawTool_GC_Get(&GPDrawToolCB), 
          SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));

        XFillRectangle (PDrawDisplay, 
          XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)), 
          PDrawTool_GC_Get(&GPDrawToolCB), 
          0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT);
        XFillRectangle (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, pat),
           PDrawTool_GC_Get(&GPDrawToolCB), 0, 0,
           PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT);

        XSetForeground (PDrawDisplay, PDrawTool_GC_Get(&GPDrawToolCB), 
          SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));

        XtVaGetValues (PDrawTool_ScrlldWin_Get (&GPDrawToolCB, pat),
          XmNwidth, &sw_w,
          XmNheight, &sw_h,
          NULL);

        pdraw_Molecule (PDrawDisplay, 
          XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)), 
          PDrawTool_GC_Get(&GPDrawToolCB), 
          PDrawMol_p, AppDim_MolScale_Get (&GAppDim), sw_h, sw_w, pat,
            pat == GOAL);
        pdraw_Molecule (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, pat),
          PDrawTool_GC_Get(&GPDrawToolCB), PDrawMol_p, 1.0, 0, 0, pat, FALSE);
        }

      /* If Shelley code could not handle this transformation,
          popup error dialog.
      */
/*
      else
        InfoWarn_Show ("shelley_code_errdg");
*/
 
      draw_roots (PDrawDisplay,
        XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, GOAL)),
        PDrawTool_Pixmap_Get (&GPDrawToolCB, GOAL),
        PDrawTool_GC_Get(&GPDrawToolCB), PDrawMol_p);

      PDrawFlags.drawing_mode = PDRW_BOND_FLAG;
      PDraw_Mode_Reset (PDRW_BOND_FLAG);
      PDrawBondsInfo->cur_bnd_p = (RxnDsp_Bond_t *) NULL;
      XmProcessTraversal (w, XmTRAVERSE_HOME);
      break;

    case PDRW_SELECTROOT_FLAG:
      if (IsThere_Draw_Flag && glob_rxlform)
        {
        IsTFG_Form_Create (glob_parent, goalfg, subgfg, gminin, sminin);
        break;
        }
      if (selecting_root)
        {
/*
        if (!XtIsManaged (PDrawTool_SynListForm_Get (&GPDrawToolCB)) &&
            !XtIsManaged (PDrawTool_SlingListForm_Get (&GPDrawToolCB)))
*/
        if (!synlist_managed)
          selecting_root = FALSE;
        break;
        }
      if (RxnDsp_RootCnt_Get (PDrawMol_p) == MAXROOTS)
        {
        XBell (PDrawDisplay, 10);
        PDraw_Mode_Reset (PDRW_BOND_FLAG);
        }
      else
        {
        selecting_root = TRUE;
/*
        XtManageChild (PDrawTool_SynListForm_Get (&GPDrawToolCB));
*/
/* Sy_Form_Create resides in syntheme_list.c */
        Sy_Form_Create (glob_parent, glob_syntheme, Get_Selected_Syntheme);
        synlist_managed = TRUE;
        XDefineCursor (XtDisplay (da_wid[GOAL]), XtWindow (da_wid[GOAL]), 
          SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_PLUS));
        }
      break;

    case PDRW_DELETEROOT_FLAG:
      if (RxnDsp_RootCnt_Get (PDrawMol_p) == 0)
        {
        if (deleting_root)
          deleting_root = FALSE;
        else
          XBell (PDrawDisplay, 10);
        PDraw_Mode_Reset (PDRW_BOND_FLAG);
        }
      else
        {
        deleting_root = TRUE;
        XDefineCursor (XtDisplay (da_wid[GOAL]), XtWindow (da_wid[GOAL]), 
          SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_RXNVW));
        }
      break;

    case PDRW_RXNCNTR_FLAG:
      for (pat=0; pat<2; pat++)
        XDefineCursor (XtDisplay (da_wid[pat]), XtWindow (da_wid[pat]), 
          SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_TARGET));
      
      XmProcessTraversal (w, XmTRAVERSE_HOME);
      break;
    }

  return ;
}
/*  End of POption_Choose_CB  */

/****************************************************************************
*  
*  Function Name:                 PMenu_Choice_CB
*  
*****************************************************************************/  

void PMenu_Choice_CB 
  (
  Widget w,
  XtPointer client_data,
  XtPointer call_data
  )
				  
{
  RxnDsp_Atom_t   *atom_p;
  RxnDsp_Bond_t   *bond_p;
  int              ii, start, finish, savegsg;
  Boolean_t        buble;
  Widget           da_wid[2];
  int              tag, pat;
  Boolean_t        save_sel;

  tag = (int) client_data;
  for (pat=0; pat<2; pat++)
    da_wid[pat] = PDrawTool_DrawArea_Get(&GPDrawToolCB, pat);

  switch (tag) 
    {
    
    case PDRW_MENU_DELETE_ALL:
      if (IsThere_Draw_Flag && glob_rxlform) /* avoid crash - move atoms into visible area */
        POption_Choose_CB (PDrawTool_RedrawPB_Get (&GPDrawToolCB), (XtPointer) PDRW_REDRAW_FLAG, (XtPointer) NULL);
      tag = SMU_PYES_RESPONSE;
      PDraw_Mode_Reset (PDRW_BOND_FLAG);
      gsgsame = TRUE;
/*
      nroots = 0;
*/
      RxnDsp_RootCnt_Put (PDrawMol_p, 0);

    case PDRW_MENU_SELECT_ALL:
      PDrawBondsInfo->esc_mode = TRUE;
      PDrawSelInfo->move_it = FALSE;
      for (pat=0; pat<2; pat++)
        XDefineCursor (XtDisplay (da_wid[pat]), XtWindow (da_wid[pat]), 
          SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_WRITE));
      
      PDrawFlags.drawing_mode = PDRW_SELECT_FLAG;
      PDraw_Mode_Reset (PDRW_SELECT_FLAG);

      for (savegsg=gsg, gsg=0; gsg<2; gsg++)
        {
        /* Mark all atoms as selected */
        atom_p = RxnDsp_AtomPtr_Get (PDrawMol_p, 0);
        for (ii = 0; ii < PDrawMol_p->both_dm.natms; ii++) 
          {
          atom_p->xp = atom_p->xn = atom_p->yp = atom_p->yn = TRUE;
          ++atom_p;
          }

        pmark_selected (PDrawMol_p, PDrawDisplay, 
          PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg),
          PDrawTool_GC_Get (&GPDrawToolCB));
        XCopyArea (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), 
          XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), 
          PDrawTool_GC_Get (&GPDrawToolCB), 0, 0,
          PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);
        }
      gsg=savegsg;

      PDrawSelInfo->selected_gsg = BOTH;

      if ((tag == SMU_PNO_RESPONSE) || (tag == PDRW_MENU_SELECT_ALL))
        break;

    case PDRW_MENU_DELETE_SEL:
      if (PDrawSelInfo->selected_gsg == BOTH)
        {
        gsgsame = TRUE;
        PDrawSelInfo->selected_gsg = GOAL;
        }
      PDrawBondsInfo->esc_mode = TRUE;

      /* Delete all selected bonds */
      if (gsgsame)
      {
        start = GOAL;
        finish = SUBG;
      }
      else
        start = finish = PDrawSelInfo->selected_gsg;
/*
      for (pat=start; pat<= finish; pat++)
*/
        do 
        {
        buble = FALSE;
        bond_p = RxnDsp_BondPtr_Get (PDrawMol_p, 0);
        for (ii = 0; ii < PDrawMol_p->both_dm.nbnds; ii++) 
          {
	  if (RxnDsp_BondSelect_Get (bond_p, /* pat */
            PDrawSelInfo->selected_gsg) == PDRW_SELECT_TOTAL) 
            {
	    pdraw_Bond (PDrawDisplay, 
              XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, /* pat */
              PDrawSelInfo->selected_gsg)), 
              PDrawTool_GC_Get (&GPDrawToolCB), 
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), PDrawMol_p, 
              bond_p, /* pat */ PDrawSelInfo->selected_gsg, TRUE, TRUE);
	    pdraw_Bond (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB,
              /* pat */ PDrawSelInfo->selected_gsg),
              PDrawTool_GC_Get (&GPDrawToolCB), 
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), 
              PDrawMol_p, bond_p, /* pat */ PDrawSelInfo->selected_gsg,
              TRUE, TRUE);
            RxnDsp_BondSelect_Put (bond_p, PDrawSelInfo->selected_gsg, 0);
            if (gsgsame)
              {
	      pdraw_Bond (PDrawDisplay, 
                XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, /* pat */
                1 - PDrawSelInfo->selected_gsg)), 
                PDrawTool_GC_Get (&GPDrawToolCB), 
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), PDrawMol_p, 
                bond_p, /* pat */ 1 - PDrawSelInfo->selected_gsg, TRUE, TRUE);
	      pdraw_Bond (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB,
                /* pat */ 1 - PDrawSelInfo->selected_gsg),
                PDrawTool_GC_Get (&GPDrawToolCB), 
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), 
                PDrawMol_p, bond_p, /* pat */ 1 - PDrawSelInfo->selected_gsg,
                TRUE, TRUE);
              RxnDsp_BondSelect_Put (bond_p, 1 - PDrawSelInfo->selected_gsg,
                PDRW_SELECT_FORMERLY);
              }
	    rxn_delete_Bond (PDrawMol_p, bond_p, /* pat */
              gsgsame ? BOTH : PDrawSelInfo->selected_gsg);
	    buble = TRUE;
	    }

	  ++bond_p;
          }
        } while (buble);

/* NOTE: This is a problem - infinite loop when clearing */
      /* Delete all selected atoms */
/*
      for (pat=start; pat<= finish; pat++)
*/
        do 
        {
        buble = FALSE;
        atom_p = RxnDsp_AtomPtr_Get (PDrawMol_p, 0);
        for (ii = 0; ii < PDrawMol_p->both_dm.natms; ii++) 
          {
	  if (RxnDsp_AtomSelect_Get (atom_p, /* pat */
            PDrawSelInfo->selected_gsg) == DRW_SELECT_TOTAL) 
            {
	    pdraw_Atom (PDrawDisplay, 
              XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB,
              /* start */ PDrawSelInfo->selected_gsg)), 
              XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB,
              /* finish */ PDrawSelInfo->selected_gsg)), 
              PDrawTool_GC_Get (&GPDrawToolCB), 
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), atom_p, 
              PDRW_ATOM_SIZE_LRG);
	    pdraw_Atom (PDrawDisplay,
              PDrawTool_Pixmap_Get (&GPDrawToolCB, /* start */
              PDrawSelInfo->selected_gsg), 
              PDrawTool_Pixmap_Get (&GPDrawToolCB, /* finish */
              PDrawSelInfo->selected_gsg), 
              PDrawTool_GC_Get (&GPDrawToolCB), 
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), 
              atom_p, PDRW_ATOM_SIZE_LRG);
            RxnDsp_AtomSelect_Put (atom_p, PDrawSelInfo->selected_gsg,
              PDRW_SELECT_FORMERLY);
/*
            if (gsgsame)
*/
              {
	      pdraw_Atom (PDrawDisplay, 
                XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, /* start */
                1 - PDrawSelInfo->selected_gsg)), 
                XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, /* finish */
                1 - PDrawSelInfo->selected_gsg)), 
                PDrawTool_GC_Get (&GPDrawToolCB), 
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), atom_p, 
                PDRW_ATOM_SIZE_LRG);
	      pdraw_Atom (PDrawDisplay,
                PDrawTool_Pixmap_Get (&GPDrawToolCB, /* start */
                1 - PDrawSelInfo->selected_gsg), 
                PDrawTool_Pixmap_Get (&GPDrawToolCB, /* finish */
                1 - PDrawSelInfo->selected_gsg), 
                PDrawTool_GC_Get (&GPDrawToolCB), 
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), 
                atom_p, PDRW_ATOM_SIZE_LRG);
              RxnDsp_AtomSelect_Put (atom_p, 1 - PDrawSelInfo->selected_gsg,
                PDRW_SELECT_FORMERLY);
              }
            check_roots (PDrawMol_p, atom_p);
	    if (!rxn_delete_Atom (PDrawMol_p, atom_p,
              gsgsame ? BOTH : PDrawSelInfo->selected_gsg))
              {
	      pdraw_Atom (PDrawDisplay, 
                XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, /* start */
                PDrawSelInfo->selected_gsg)), 
                XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, /* finish */
                PDrawSelInfo->selected_gsg)), 
                PDrawTool_GC_Get (&GPDrawToolCB), 
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), atom_p, 
                PDRW_ATOM_SIZE_LRG);
	      pdraw_Atom (PDrawDisplay,
                PDrawTool_Pixmap_Get (&GPDrawToolCB, /* start */
                PDrawSelInfo->selected_gsg), 
                PDrawTool_Pixmap_Get (&GPDrawToolCB, /* finish */
                PDrawSelInfo->selected_gsg), 
                PDrawTool_GC_Get (&GPDrawToolCB), 
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), 
                atom_p, PDRW_ATOM_SIZE_LRG);
/*
              if (gsgsame)
*/
                {
	        pdraw_Atom (PDrawDisplay, 
                  XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, /* start */
                  1 - PDrawSelInfo->selected_gsg)), 
                  XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, /* finish */
                  1 - PDrawSelInfo->selected_gsg)), 
                  PDrawTool_GC_Get (&GPDrawToolCB), 
                  SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), atom_p, 
                  PDRW_ATOM_SIZE_LRG);
	        pdraw_Atom (PDrawDisplay,
                  PDrawTool_Pixmap_Get (&GPDrawToolCB, /* start */
                  1 - PDrawSelInfo->selected_gsg), 
                  PDrawTool_Pixmap_Get (&GPDrawToolCB, /* finish */
                  1 - PDrawSelInfo->selected_gsg),
                  PDrawTool_GC_Get (&GPDrawToolCB), 
                  SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), 
                  atom_p, PDRW_ATOM_SIZE_LRG);
                }
              }
	    buble = TRUE;
	    }

	  ++atom_p;
          }
        } while (buble);

      save_sel = PDrawSelInfo->isSelected;
      for (savegsg=gsg, gsg=start; gsg<=finish; gsg++)
        {
        PDrawSelInfo->isSelected = save_sel;
        punmark_selected (PDrawMol_p, PDrawDisplay, 
          PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), 
          XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, gsg)), 
          PDrawTool_GC_Get (&GPDrawToolCB));
        }
      gsg=savegsg;

      PDrawSelInfo->move_it = FALSE;

      if (tag == SMU_PYES_RESPONSE) 
        {
        /* White out the drawing area */
        XSetForeground (PDrawDisplay, PDrawTool_GC_Get (&GPDrawToolCB), 
          SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
        for (pat=start; pat<=finish; pat++)
          {
          XFillRectangle (PDrawDisplay,
            PDrawTool_Pixmap_Get (&GPDrawToolCB, pat),
            PDrawTool_GC_Get (&GPDrawToolCB), 0, 0,
            PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT);
          XCopyArea (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, pat), 
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)), 
            PDrawTool_GC_Get (&GPDrawToolCB), 0, 0,
            PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);

          XUndefineCursor (XtDisplay (da_wid[pat]), XtWindow (da_wid[pat]));
          }
        
        PDrawFlags.drawing_mode = PDRW_BOND_FLAG;
        PDraw_Mode_Reset (PDRW_BOND_FLAG);
        }
      break;
    case PDRW_MENU_RETRACE:
      PDrawFlags.retrace_mode = TRUE;
      /*wid = retrieve_widget_id ("options_menu_bondpl_mode_tb", 0);    
      XmToggleButtonSetState (wid, FALSE, FALSE);*/
      break;

    case PDRW_MENU_BOND_PLUS:
      PDrawFlags.retrace_mode = FALSE;
     /* wid = retrieve_widget_id ("options_menu_retrace_mode_tb", 0);
      XmToggleButtonSetState (wid, FALSE, FALSE);*/
      break;
    case DSP_BOND_DOUBLE:
      PDrawFlags.dflt_bond = DSP_BOND_DOUBLE;
      break;
    
    case DSP_BOND_TRIPLE:
      PDrawFlags.dflt_bond = DSP_BOND_TRIPLE;
      break;

    case PDRW_MENU_EXIT:
      exit (0);

    case PDRW_MENU_SUBMIT:
      exit (0);
    }

  return ;
}
/*  End of PMenu_Choice_CB  */

/****************************************************************************
*  
*  Function Name:                 PSelected_Bond_Draw_CB
*  
*****************************************************************************/  

void PSelected_Bond_Draw_CB 
  (
  Widget           w,
  XtPointer        client_data,
  XtPointer        call_data             
  )
{
  int             tag;
  RxnDsp_Atom_t  *atom1_p, *atom2_p;

  if (PDrawBondsInfo->clicked_gsg == NEITHER) return;

  tag = (int) client_data;

printf("PSelected_Bond_Draw_CB: IsThere_Draw_Flag=%d glob_rxlform=%d cur_bondp=%p\n",IsThere_Draw_Flag,glob_rxlform,
PDrawBondsInfo->cur_bnd_p);
  if (tag > 10)
    /* Function was invoked by middle default buttons */
    PDrawFlags.dflt_bond = tag - 10;

  else 
    {
    /* Function was invoked by "last drawn bond" buttons */
    if ((PDrawBondsInfo->cur_bnd_p) &&
	(PDrawFlags.drawing_mode == PDRW_BOND_FLAG || (IsThere_Draw_Flag && glob_rxlform))) 
      {
      if (tag != DSP_BOND_ERASE) 
        {
	pdraw_Bond (PDrawDisplay, 
          XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB,
          PDrawBondsInfo->clicked_gsg)), 
          SynAppR_MolGC_Get (&GSynAppR),
          SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), PDrawMol_p, 
          PDrawBondsInfo->cur_bnd_p, PDrawBondsInfo->clicked_gsg, FALSE, FALSE);
	pdraw_Bond (PDrawDisplay,
          PDrawTool_Pixmap_Get (&GPDrawToolCB, PDrawBondsInfo->clicked_gsg), 
          SynAppR_MolGC_Get (&GSynAppR),
          SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), PDrawMol_p, 
          PDrawBondsInfo->cur_bnd_p, PDrawBondsInfo->clicked_gsg, FALSE, FALSE);
        if (gsgsame)
          {
	  pdraw_Bond (PDrawDisplay, 
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB,
            1 - PDrawBondsInfo->clicked_gsg)), 
            SynAppR_MolGC_Get (&GSynAppR),
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), PDrawMol_p, 
            PDrawBondsInfo->cur_bnd_p, 1 - PDrawBondsInfo->clicked_gsg,
            FALSE, FALSE);
	  pdraw_Bond (PDrawDisplay,
            PDrawTool_Pixmap_Get (&GPDrawToolCB,
            1 - PDrawBondsInfo->clicked_gsg), 
            SynAppR_MolGC_Get (&GSynAppR),
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), PDrawMol_p, 
            PDrawBondsInfo->cur_bnd_p, 1 - PDrawBondsInfo->clicked_gsg,
            FALSE, FALSE);
          }
	if ((tag == DSP_BOND_STEREO_DOWN)  
            || (tag == DSP_BOND_STEREO_OPP_DOWN) 
	    || (tag == DSP_BOND_STEREO_UP) 
	    || (tag == DSP_BOND_STEREO_OPP_UP)) 
          {
	  atom1_p = RxnDsp_AtomThere_Get (PDrawMol_p,
            PDrawBondsInfo->clicked_gsg,
            PDrawBondsInfo->clicked_x, PDrawBondsInfo->clicked_y);
	  PDrawBondsInfo->cur_bnd_p->latm_p = 
	    (PDrawBondsInfo->cur_bnd_p->ratm_p == atom1_p) ?
	      PDrawBondsInfo->cur_bnd_p->latm_p :
	      PDrawBondsInfo->cur_bnd_p->ratm_p;
	  PDrawBondsInfo->cur_bnd_p->ratm_p = atom1_p;
	  RxnDsp_BondStereo_Put (PDrawBondsInfo->cur_bnd_p,
            PDrawBondsInfo->clicked_gsg, tag);
          if (gsgsame)
	    RxnDsp_BondStereo_Put (PDrawBondsInfo->cur_bnd_p,
              1 - PDrawBondsInfo->clicked_gsg, tag);
	  }
	else 
          {
	  RxnDsp_BondOrder_Put (PDrawBondsInfo->cur_bnd_p,
            PDrawBondsInfo->clicked_gsg, tag);
	  RxnDsp_BondStereo_Put (PDrawBondsInfo->cur_bnd_p,
            PDrawBondsInfo->clicked_gsg, DSP_BOND_STEREO_NONE);
          if (gsgsame)
            {
	    RxnDsp_BondOrder_Put (PDrawBondsInfo->cur_bnd_p,
              1 - PDrawBondsInfo->clicked_gsg, tag);
	    RxnDsp_BondStereo_Put (PDrawBondsInfo->cur_bnd_p,
              1 - PDrawBondsInfo->clicked_gsg, DSP_BOND_STEREO_NONE);
            }
          }

	pdraw_Bond (PDrawDisplay, 
          XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB,
          PDrawBondsInfo->clicked_gsg)), 
          SynAppR_MolGC_Get (&GSynAppR), 
          SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), PDrawMol_p, 
          PDrawBondsInfo->cur_bnd_p, PDrawBondsInfo->clicked_gsg, TRUE, TRUE);
	pdraw_Bond (PDrawDisplay,
          PDrawTool_Pixmap_Get (&GPDrawToolCB, PDrawBondsInfo->clicked_gsg), 
          SynAppR_MolGC_Get (&GSynAppR),
          SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), PDrawMol_p, 
          PDrawBondsInfo->cur_bnd_p, PDrawBondsInfo->clicked_gsg, TRUE, TRUE);
        if (gsgsame)
          {
	  pdraw_Bond (PDrawDisplay, 
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB,
            1 - PDrawBondsInfo->clicked_gsg)), 
            SynAppR_MolGC_Get (&GSynAppR), 
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), PDrawMol_p, 
            PDrawBondsInfo->cur_bnd_p,
            1 - PDrawBondsInfo->clicked_gsg, TRUE, TRUE);
	  pdraw_Bond (PDrawDisplay,
            PDrawTool_Pixmap_Get (&GPDrawToolCB,
            1 - PDrawBondsInfo->clicked_gsg), 
            SynAppR_MolGC_Get (&GSynAppR),
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), PDrawMol_p, 
            PDrawBondsInfo->cur_bnd_p,
            1 - PDrawBondsInfo->clicked_gsg, TRUE, TRUE);
          }
        }
      else 
        {
	atom1_p = RxnDsp_AtomThere_Get (PDrawMol_p, PDrawBondsInfo->clicked_gsg,
          PDrawBondsInfo->clicked_x, PDrawBondsInfo->clicked_y);
	atom2_p = (atom1_p == PDrawBondsInfo->cur_bnd_p->latm_p) ?
	  PDrawBondsInfo->cur_bnd_p->ratm_p : PDrawBondsInfo->cur_bnd_p->latm_p;

	if ((atom1_p->adj_info.num_neighbs == 1) &&
	    (atom2_p->adj_info.num_neighbs == 1)) 
          {
	  PDrawBondsInfo->esc_mode = TRUE;
	  } 
	else if (!((atom1_p->adj_info.num_neighbs >  1) &&
		   (atom2_p->adj_info.num_neighbs == 1))) 
          {
	  PDrawBondsInfo->clicked_x =
            RxnDsp_AtomXCoord_Get (atom2_p, PDrawBondsInfo->clicked_gsg);
	  PDrawBondsInfo->clicked_y =
            RxnDsp_AtomYCoord_Get (atom2_p, PDrawBondsInfo->clicked_gsg);
	  }

	pdraw_Bond (PDrawDisplay, 
          XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB,
          PDrawBondsInfo->clicked_gsg)), 
          SynAppR_MolGC_Get (&GSynAppR), 
          SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), PDrawMol_p, 
          PDrawBondsInfo->cur_bnd_p, PDrawBondsInfo->clicked_gsg, TRUE, TRUE);
        if (gsgsame)
          {
	  pdraw_Bond (PDrawDisplay, 
            XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB,
            1 - PDrawBondsInfo->clicked_gsg)), 
            SynAppR_MolGC_Get (&GSynAppR), 
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), PDrawMol_p, 
            PDrawBondsInfo->cur_bnd_p,
            1 - PDrawBondsInfo->clicked_gsg, TRUE, TRUE);
	  perase_Bond (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, GOAL),
	    PDrawTool_Pixmap_Get (&GPDrawToolCB, SUBG), 
	    XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, GOAL)), 
	    XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, SUBG)), 
            SynAppR_MolGC_Get (&GSynAppR), PDrawMol_p, 
            PDrawBondsInfo->cur_bnd_p, BOTH);
          }
        else
	  perase_Bond (PDrawDisplay,
            PDrawTool_Pixmap_Get (&GPDrawToolCB, PDrawBondsInfo->clicked_gsg), 
	    PDrawTool_Pixmap_Get (&GPDrawToolCB,
            1 - PDrawBondsInfo->clicked_gsg), 
	    XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB,
            PDrawBondsInfo->clicked_gsg)), 
	    XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB,
            1 - PDrawBondsInfo->clicked_gsg)), 
            SynAppR_MolGC_Get (&GSynAppR), PDrawMol_p, 
            PDrawBondsInfo->cur_bnd_p, PDrawBondsInfo->clicked_gsg);
	PDrawBondsInfo->cur_bnd_p = (RxnDsp_Bond_t *) NULL;

        } /* End of DSP_BOND_ERASE */
      }
    } /* End of else */

  XCopyArea (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB,
    PDrawBondsInfo->clicked_gsg), 
    XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB,
    PDrawBondsInfo->clicked_gsg)), 
    SynAppR_MolGC_Get (&GSynAppR), 0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT,
    0, 0);
  if (gsgsame)
    XCopyArea (PDrawDisplay,
      PDrawTool_Pixmap_Get (&GPDrawToolCB, 1 - PDrawBondsInfo->clicked_gsg), 
      XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB,
      1 - PDrawBondsInfo->clicked_gsg)), 
      SynAppR_MolGC_Get (&GSynAppR), 0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT,
      0, 0);

  return ;
}
/*  End of PSelected_Bond_Draw_CB  */

/****************************************************************************
*  
*  Function Name:                 PQuit_CB
*  
*****************************************************************************/  

void PQuit_CB 
  (
  Widget           w,
  XtPointer        client_data,
  XtPointer        call_data             
  )
{
  if (IsThere_Draw_Flag && glob_rxlform) Ist_Info_Send (NULL, NULL, NULL, NULL, NULL, NULL);
  else Sch_Tsds_Store (NULL, NULL, NULL, NULL, 0, NULL);
  XtUnmanageChild (PDrawTool_Frame_Get (&GPDrawToolCB));
}

/****************************************************************************
*  
*  Function Name:                 PDone_CB
*  
*****************************************************************************/  

void PDone_CB 
  (
  Widget           w,
  XtPointer        client_data,
  XtPointer        call_data             
  )
{
  RxnDsp_Molecule_t *mol_p;
  RxnInfoCB_t       *rxninfo_p[2];
  int                pat, i, j, nroots;
  Tsd_t             *tsd_p[2], *ttsd_p;
  U32_t              roots[MAXROOTS], synthemes[MAXROOTS];
  char               errmsg[256];
  XmString           msg_str;
  Boolean_t          normal_exit;

  mol_p = PDrawMol_p;
  if (mol_p == NULL)
    return;

dump_RxnDsp (mol_p, "PDone_CB");

  for (pat=0; pat<2; pat++)
    tsd_p[pat] = RxnDsp2Tsd (mol_p, pat);
printf("\nGoal (before compression)\n");
Tsd_Dump (tsd_p[GOAL], &GStdOut);
printf("\nSubgoal (before compression)\n");
Tsd_Dump (tsd_p[SUBG],&GStdOut);

  if (IsThere_Draw_Flag && glob_rxlform)
    {
    PDraw_Mode_Reset (PDRW_BOND_FLAG);

    for (i = roots[0] = 0; i < 2; i++)
      {
      ttsd_p = Tsd_Copy (tsd_p[i]);
      Tsd_MatchCompress_Fix (tsd_p[i], ttsd_p, roots);
      Tsd_Destroy (ttsd_p);
      }
printf("\nGoal\n");
Tsd_Dump (tsd_p[GOAL], &GStdOut);
printf("\nSubgoal\n");
Tsd_Dump (tsd_p[SUBG],&GStdOut);

    XtUnmanageChild (PDrawTool_Frame_Get (&GPDrawToolCB));

    Ist_Info_Send (tsd_p[GOAL], tsd_p[SUBG], goalfg, subgfg, gminin, sminin);

    Tsd_Destroy (tsd_p[GOAL]);
    Tsd_Destroy (tsd_p[SUBG]);

    return;
    }

  nroots =  RxnDsp_RootCnt_Get (mol_p);

  for (i = 0; i < mol_p->both_dm.natms; i++)
    for (j = 0; j < nroots; j++)
      if (RxnDsp_AtomIsRoot_Get(mol_p, i, j))
    {
    roots[j] = i;
    synthemes[j] = RxnDsp_RootPtr_Get (mol_p, j)->syntheme;
    }

  for (j = nroots; j < MX_ROOTS; j++) roots[j] = REACT_NODE_INVALID;
/*
    rxninfo_p[pat] = ((RxnInfoCB_t *) client_data) + pat;

  RxnInfo_SlingTxt_Update (rxninfo_p, mol_p);
*/
  PDraw_Mode_Reset (PDRW_BOND_FLAG);

  normal_exit = Sch_Tsds_Store (tsd_p[GOAL], tsd_p[SUBG], roots, synthemes, nroots, errmsg);

  Tsd_Destroy (tsd_p[GOAL]);
  Tsd_Destroy (tsd_p[SUBG]);

  if (!normal_exit || errmsg[0] != '\0')
    {
    msg_str = XmStringCreateLocalized (errmsg);
    XtVaSetValues (PDrawTool_ExitErrorMsg_Get (&GPDrawToolCB),
      XmNmessageString, msg_str,
      NULL);
    XmStringFree (msg_str);

    XtManageChild (PDrawTool_ExitErrorMsg_Get (&GPDrawToolCB));
    if (!normal_exit) return;
    }

  XtUnmanageChild (PDrawTool_Frame_Get (&GPDrawToolCB));

  return;
}
/* End of Done_CB */

/****************************************************************************
*  
*  Function Name:                 PIsolate_CB
*  
*****************************************************************************/  

void PIsolate_CB 
  (
  Widget           w,
  XtPointer        client_data,
  XtPointer        call_data             
  )
{
  gsgsame = FALSE;
}
/* End of PIsolate_CB */

/****************************************************************************
*  
*  Function Name:                 PNumTogl_CB
*  
*****************************************************************************/  

void PNumTogl_CB 
  (
  Widget           w,
  XtPointer        client_data,
  XtPointer        call_data             
  )
{
  int pat, savegsg, save_sel, i;
  Widget da_wid[2];

  if (client_data == NULL)
    nodenums_on = !nodenums_on;

  /* If smth was selected prior, unselect it */

  save_sel = PDrawSelInfo->isSelected;
  for (savegsg=gsg, gsg=0; gsg<2; gsg++)
    {
    da_wid[gsg] = PDrawTool_DrawArea_Get(&GPDrawToolCB, gsg);
    PDrawSelInfo->isSelected = save_sel;
    punmark_selected (PDrawMol_p, PDrawDisplay,
      PDrawTool_Pixmap_Get (&GPDrawToolCB, gsg), 
      XtWindow (da_wid[gsg]), PDrawTool_GC_Get (&GPDrawToolCB));
    }
  gsg = savegsg;

  for (pat = 0; pat < 2; pat++)
    {
    for (i = 0; i < PDrawMol_p->both_dm.natms; i++)
      RxnDsp_AtomCharge_Put (RxnDsp_AtomPtr_Get (PDrawMol_p, i), pat,
        nodenums_on ? i + 1 : 0);

    XUndefineCursor (XtDisplay (da_wid[pat]), XtWindow (da_wid[pat]));

    XSetForeground (PDrawDisplay, PDrawTool_GC_Get(&GPDrawToolCB), 
      SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));

    XFillRectangle (PDrawDisplay, 
      XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)), 
      PDrawTool_GC_Get(&GPDrawToolCB), 
      0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT);
    XFillRectangle (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, pat),
      PDrawTool_GC_Get(&GPDrawToolCB), 0, 0,
      PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT);

    XSetForeground (PDrawDisplay, PDrawTool_GC_Get(&GPDrawToolCB), 
      SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));

    pdraw_Molecule (PDrawDisplay, 
      XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, pat)), 
      PDrawTool_GC_Get(&GPDrawToolCB), PDrawMol_p, 1.0, 0, 0, pat, FALSE);
    pdraw_Molecule (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, pat),
      PDrawTool_GC_Get(&GPDrawToolCB), PDrawMol_p, 1.0, 0, 0, pat, FALSE);
    }

    draw_roots (PDrawDisplay,
      XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, GOAL)),
      PDrawTool_Pixmap_Get (&GPDrawToolCB, GOAL),
      PDrawTool_GC_Get(&GPDrawToolCB), PDrawMol_p);

    PDrawFlags.drawing_mode = PDRW_BOND_FLAG;
    PDraw_Mode_Reset (PDRW_BOND_FLAG);
    PDrawBondsInfo->cur_bnd_p = (RxnDsp_Bond_t *) NULL;
    PDrawBondsInfo->esc_mode = TRUE;

    XmProcessTraversal (w, XmTRAVERSE_HOME);
}
/* End of PNumTogl_CB */

/****************************************************************************
*  
*  Function Name:                 PMOLDRAW_StereoChem_CB
*  
*****************************************************************************/  

void PMOLDRAW_StereoChem_CB
 (
  Widget           w,
  XtPointer        client_data,
  XtPointer        call_data             
  )
{
  return;
}
/*  End of PMOLDRAW_StereoChem_CB  */

/****************************************************************************
*  
*  Function Name:                 PNew_RxnDsp2Xtr
*  
*****************************************************************************/  

Xtr_t *PNew_RxnDsp2Xtr (RxnDsp_Molecule_t *mol_p, int which)
{
  Xtr_t            *xtr_p;
  RxnDsp_Atom_t    *atom_p, *adjatm_p;
  RxnDsp_Bond_t    *bond_p;
  int               atm_i, ii;
  char              sym[3];
  Dsp_Molecule_t   *patt_p;

  /*xtr_p = (Xtr_t *) malloc (XTRSIZE);*/
  if (which == GOAL)
    patt_p = mol_p->goal_dm_p;
  else
    patt_p = mol_p->subgoal_dm_p;

  xtr_p = Xtr_Create (patt_p->natms);

  /* Transform atom relative info first */
  Xtr_NumAtoms_Put (xtr_p, patt_p->natms);
  xtr_p->attributes = (XtrRow_t *) malloc (XTRROWSIZE * patt_p->natms);

  atom_p = patt_p->atoms;
  for (atm_i = 0; atm_i < patt_p->natms; atm_i++) 
    {
    /* Convert atom symbol in dsp format to atom id */
    if (strcmp (atom_p->sym, " I ") == 0)
      strcpy (sym, "I");      
    else if (strcmp (atom_p->sym, " : ") == 0)
      strcpy (sym, ":`");
    else if (strcmp (atom_p->sym, " . ") == 0)
      strcpy (sym, ".`");
    Xtr_Attr_Atomid_Put (xtr_p, atm_i, Atomsymbol2Id((U8_t *)sym));	   

    Xtr_Attr_NumNeighbors_Put (xtr_p, atm_i, atom_p->adj_info.num_neighbs);

    /* Transform bond relative info */
    for (ii = 0; ii < atom_p->adj_info.num_neighbs; ii++) 
      {
      Xtr_Attr_NeighborId_Put (xtr_p, atm_i, ii, 
        (U16_t) atom_p->adj_info.adj_atms[ii]);
      adjatm_p = patt_p->atoms + (int) atom_p->adj_info.adj_atms[ii];
      bond_p = get_Bond_There (patt_p,
        RxnDsp_AtomXCoord_Get (atom_p, gsg),
        RxnDsp_AtomYCoord_Get (atom_p, gsg),
        RxnDsp_AtomXCoord_Get (adjatm_p, gsg),
        RxnDsp_AtomYCoord_Get (adjatm_p, gsg));

      /* Convert multiplicity of bond */
      switch (bond_p->nlines) 
        {
        case  DSP_BOND_ERASE:
          Xtr_Attr_NeighborBond_Put (xtr_p, atm_i, ii, BOND_NONE);
          break;
        case  DSP_BOND_SINGLE:
          Xtr_Attr_NeighborBond_Put (xtr_p, atm_i, ii, BOND_SINGLE);
          break;
        case  DSP_BOND_DOUBLE:
          Xtr_Attr_NeighborBond_Put (xtr_p, atm_i, ii, BOND_DOUBLE);
          break;
        case  DSP_BOND_TRIPLE:
          Xtr_Attr_NeighborBond_Put (xtr_p, atm_i, ii, BOND_TRIPLE);
          break;
        case  DSP_BOND_RESONANT:
          Xtr_Attr_NeighborBond_Put (xtr_p, atm_i, ii, BOND_RESONANT);
          break;
        case  DSP_BOND_VARIABLE:
          Xtr_Attr_NeighborBond_Put (xtr_p, atm_i, ii, BOND_VARIABLE);
          break;
        default:
          Xtr_Attr_NeighborBond_Put (xtr_p, atm_i, ii, BOND_SINGLE);
        }

      /* Convert stereo information */
      switch (bond_p->stereo) 
        {
        case  DSP_BOND_STEREO_UP:	
        case  DSP_BOND_STEREO_OPP_UP:
	  Xtr_Attr_NeighborStereo_Put (xtr_p, atm_i, ii, BOND_DIR_UP);
	  break;
        case  DSP_BOND_STEREO_DOWN:
        case  DSP_BOND_STEREO_OPP_DOWN:
	  Xtr_Attr_NeighborStereo_Put (xtr_p, atm_i, ii, BOND_DIR_DOWN);
	  break;
        default:
	  Xtr_Attr_NeighborStereo_Put (xtr_p, atm_i, ii, BOND_DIR_INVALID);
        }
      } /* End for all neighbors */

    atom_p++;
    } /* End for all atoms */

  return (xtr_p);
}
/*  End of PNew_RxnDsp2Xtr  */

/****************************************************************************
*  
*  Function Name:                 PMolDraw_Sling_Draw
*  
*****************************************************************************/  

void PMolDraw_Sling_Draw 
  (
  Sling_t sling,
  int     which
  )
{
  U16_t              num_atoms;                 /* Number of atoms found */
  Dimension          sw_h, sw_w;
  Widget             da_wid;
  Xtr_t             *xtr_p;
  Dsp_Molecule_t    *old_mol_p, *patt_p;

  if (!Sling_Validate (sling, &num_atoms))
    {
/*
    InfoWarn_Show ("Invalid sling.");
*/
    return;
    }

  xtr_p = Sling2Xtr ( /* PSling2CanonSling( */ sling /* ) */ );
  patt_p = Xtr2Dsp (xtr_p);
  if (which == GOAL)
    {
    old_mol_p = PDrawMol_p->goal_dm_p;
    PDrawMol_p->goal_dm_p = patt_p;
    }
  else
    {
    old_mol_p = PDrawMol_p->subgoal_dm_p;
    PDrawMol_p->subgoal_dm_p = patt_p;
    }

  if (old_mol_p != NULL)
    free_Molecule (old_mol_p);

  da_wid = PDrawTool_DrawArea_Get(&GPDrawToolCB, which);
   
  if (dsp_Shelley (patt_p)) 
    {
    XUndefineCursor (XtDisplay (da_wid), XtWindow (da_wid));
        
    XSetForeground (PDrawDisplay, PDrawTool_GC_Get (&GPDrawToolCB), 
      SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));

    XFillRectangle (PDrawDisplay, 
      XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, which)), 
      PDrawTool_GC_Get (&GPDrawToolCB), 
      0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT);
    XFillRectangle (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, which), 
      PDrawTool_GC_Get (&GPDrawToolCB), 0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT);

    XSetForeground (PDrawDisplay, PDrawTool_GC_Get (&GPDrawToolCB), 
      SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));

    XtVaGetValues (PDrawTool_ScrlldWin_Get (&GPDrawToolCB, which),
      XmNwidth, &sw_w,
      XmNheight, &sw_h,
      NULL);

    pdraw_Molecule (PDrawDisplay, 
      XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, which)), 
      PDrawTool_GC_Get (&GPDrawToolCB), 
      PDrawMol_p, AppDim_MolScale_Get (&GAppDim), sw_h, sw_w, which, TRUE);
    pdraw_Molecule (PDrawDisplay, PDrawTool_Pixmap_Get (&GPDrawToolCB, which), 
      PDrawTool_GC_Get (&GPDrawToolCB), PDrawMol_p, 1.0, 0, 0, which, FALSE);

    PDraw_Mode_Reset (PDRW_BOND_FLAG);
    }

    /* If Shelley code could not handle this transformation,
       popup error dialog.
    */
/*
    else
      InfoWarn_Show ("Unable to draw molecule (Shelley module)."); 
*/

   return;
}
/*  End of PMolDraw_Sling_Draw  */


/****************************************************************************
*  
*  Function Name:                 RxnDsp2Tsd
*  
*****************************************************************************/  

Tsd_t *RxnDsp2Tsd 
  (
  RxnDsp_Molecule_t *mol_p,
  int                which
  )
{
  Tsd_t            *tsd_p;
  RxnDsp_Atom_t    *atom_p, *adjatm_p;
  RxnDsp_Bond_t    *bond_p;
  int               atm_i, ii, odd_var, even_var;
  char              sym[8];

  odd_var = 1;
  even_var = 2;

  tsd_p = Tsd_Create (mol_p->both_dm.natms);

  /* Transform atom relative info first */
  atom_p = RxnDsp_AtomPtr_Get (mol_p, 0);
  for (atm_i = 0; atm_i < mol_p->both_dm.natms; atm_i++) 
    {
    /* Convert atom symbol in dsp format to atom id */
    strcpy (sym, atom_p->sym);
    if (strcmp (atom_p->sym, " I ") == 0)
      strcpy (sym, "I");      
    else if (strcmp (atom_p->sym, "X") == 0)
      strcpy (sym, "#j");
    else if (strcmp (atom_p->sym, "Ch") == 0)
      strcpy (sym, "#k");
    else if (strcmp (atom_p->sym, "R") == 0)
      {
      sprintf (sym, "$%d", even_var);
      even_var += 2;
      }
    else if (strcmp (atom_p->sym, "R'") == 0)
      {
      sprintf (sym, "$%d", odd_var);
      odd_var += 2;
      }
    else if (strcmp (atom_p->sym, " : ") == 0)
      strcpy (sym, ":`");
    else if (strcmp (atom_p->sym, " . ") == 0)
      strcpy (sym, ".`");
    Tsd_Atomid_Put (tsd_p, atm_i, Atomsymbol2Id( (U8_t *) sym));	   


    /* Transform bond relative info */
    for (ii = 0; ii < atom_p->adj_info.num_neighbs; ii++) 
      {
      Tsd_Atom_NeighborId_Put (tsd_p, atm_i, ii, 
        (U16_t) atom_p->adj_info.adj_atms[ii]);
      adjatm_p =
        RxnDsp_AtomPtr_Get (mol_p, (int) atom_p->adj_info.adj_atms[ii]);
      bond_p = RxnDsp_BondThere_Get (mol_p, which,
        RxnDsp_AtomXCoord_Get (atom_p, which),
        RxnDsp_AtomYCoord_Get (atom_p, which), which,
        RxnDsp_AtomXCoord_Get (adjatm_p, which),
        RxnDsp_AtomYCoord_Get (adjatm_p, which));

      /* Convert multiplicity of bond */
      if (bond_p == NULL)
        Tsd_Atom_NeighborBond_Put (tsd_p, atm_i, ii, BOND_NONE);
      else switch (RxnDsp_BondOrder_Get (bond_p, which))
        {
        case  DSP_BOND_ERASE:
          Tsd_Atom_NeighborBond_Put (tsd_p, atm_i, ii, BOND_NONE);
          break;
        case  DSP_BOND_SINGLE:
          Tsd_Atom_NeighborBond_Put (tsd_p, atm_i, ii, BOND_SINGLE);
          break;
        case  DSP_BOND_DOUBLE:
          Tsd_Atom_NeighborBond_Put (tsd_p, atm_i, ii, BOND_DOUBLE);
          break;
        case  DSP_BOND_TRIPLE:
          Tsd_Atom_NeighborBond_Put (tsd_p, atm_i, ii, BOND_TRIPLE);
          break;
        case  DSP_BOND_RESONANT:
          Tsd_Atom_NeighborBond_Put (tsd_p, atm_i, ii, BOND_RESONANT);
          break;
        case  DSP_BOND_VARIABLE:
          Tsd_Atom_NeighborBond_Put (tsd_p, atm_i, ii, BOND_VARIABLE);
          break;
        default:
          Tsd_Atom_NeighborBond_Put (tsd_p, atm_i, ii, BOND_SINGLE);
        }

      } /* End for all neighbors */

    atom_p++;
    } /* End for all atoms */

  return (tsd_p);
}
/*  End of RxnDsp2Tsd  */

/****************************************************************************
*  
*  Function Name:                 PSling2CanonSling
*  
*****************************************************************************/  

Sling_t PSling2CanonSling 
  (
  Sling_t sling
  )
{
  Xtr_t   *xtr_p;

  if (Sling_Length_Get (sling) == 0)
    return (Sling_Create (0));

  xtr_p = Sling2Xtr_PlusHydrogen (sling);
  Xtr_Attr_ResonantBonds_Set (xtr_p);
  Xtr_Name_Set (xtr_p, NULL);
  sling = Sling_CopyTrunc (Name_Sling_Get (xtr_p, FALSE));
  
  Xtr_Destroy (xtr_p);
  
  return sling;
}
/*  End of PSling2CanonSling  */

/****************************************************************************
*  
*  Function Name:                 PAtom_Copy
*  
*****************************************************************************/  

void PAtom_Copy 
  (
  RxnDsp_Atom_t  *src_atom,
  RxnDsp_Atom_t  *dest_atom
  )
{
  int i;

/* ???
  if (src_atom != NULL)
    return;
??? */

/* This makes more sense!!! */
  if (src_atom == NULL || dest_atom == NULL) return;

  strcpy (dest_atom->sym, src_atom->sym);
  strcpy (dest_atom->map, src_atom->map);
  for (i=0; i<3; i++)
    dest_atom->chg[i] = src_atom->chg[i];
  RxnDsp_AtomXCoord_Put (dest_atom, gsg, RxnDsp_AtomXCoord_Get (src_atom, gsg));
  RxnDsp_AtomYCoord_Put (dest_atom, gsg, RxnDsp_AtomYCoord_Get (src_atom, gsg));
  dest_atom->isC = src_atom->isC;

  dest_atom->adj_info.num_neighbs = src_atom->adj_info.num_neighbs;
  for (i=0; i < src_atom->adj_info.num_neighbs; i++)
    dest_atom->adj_info.adj_atms[i] = src_atom->adj_info.adj_atms[i];

  dest_atom->kvadr = src_atom->kvadr;
  dest_atom->xp = src_atom->xp;
  dest_atom->xn = src_atom->xn;
  dest_atom->yp = src_atom->yp;
  dest_atom->yn = src_atom->yn;
  dest_atom->isSelected = src_atom->isSelected;

  return;
}
/*  End of PAtom_Copy  */

/****************************************************************************
*  
*  Function Name:                 PMolecule_Double
*  
*****************************************************************************/  

RxnDsp_Molecule_t *PMolecule_Double 
   (
   RxnDsp_Molecule_t *mol_p
   )
{
  RxnDsp_Molecule_t  *doubled_mol_p;

  /* Allocate memory for molecule data structure */
  doubled_mol_p = (RxnDsp_Molecule_t *) malloc (RXNDSP_MOLECULE_SIZE);
 
   /* Allocate double the memory for atoms and bonds */
  RxnDsp_AtomPtr_Put (doubled_mol_p, 0, (RxnDsp_Atom_t *)
    malloc (RXNDSP_ATOM_SIZE * mol_p->both_dm.nallocatms * 2));
  RxnDsp_BondPtr_Put (doubled_mol_p, 0, (RxnDsp_Bond_t *)
    malloc (RXNDSP_BOND_SIZE * mol_p->both_dm.nallocbnds * 2));

  copy_Reaction (doubled_mol_p, mol_p);
  
  doubled_mol_p->both_dm.nallocatms = mol_p->both_dm.nallocatms * 2;
  doubled_mol_p->both_dm.nallocbnds = mol_p->both_dm.nallocbnds * 2;

  free_Reaction (mol_p);

  return doubled_mol_p;
}
/*  End of PMolecule_Double  */

/****************************************************************************
*  
*  Function Name:                 PDrawTool_Destroy
*  
*****************************************************************************/  

void PDrawTool_Destroy 
  (
  void
  )
{
  XFreeGC (XtDisplay (PDrawTool_BondForm_Get (&GPDrawToolCB)), PDrawTool_GC_Get 
    (&GPDrawToolCB));
  
  free_Reaction (PDrawMol_p);

  return;
}
/*  End of PDrawTool_Destroy  */
  
/****************************************************************************
*
*  Function Name:                 pstore_AtomNew (used to be store_Atom)
*
*    This routine stores an atom in the DSP structure. 
*    
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Arguments:
*
*    mol_p - pointer to RXNDSP data structure
*    sym   - atom symbol
*    chg   - atom charge
*    map   - not used
*    x     - atom's x-coordinate
*    y     - atom's y-coordinate
*
*  Return Values:
*
*    pstore_AtomNew returns a pointer to a new atom that has been stored.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
RxnDsp_Atom_t  *pstore_AtomNew (RxnDsp_Molecule_t  *mol_p,
			 char *sym, char *chg, char *map,
			 int x, int y)
{
  RxnDsp_Atom_t      *dratm_p, *otherpatt_atm_p;
  int                 i, otherpatt_x, otherpatt_y, dx, dy, dxm, dym;

/*
  lastatomgsg = gsg;
*/

  /* If num of atoms exceeds num of atom data structures allocated
     in memory, double the memory allocated to molecule
   */
  if (mol_p->both_dm.natms == mol_p->both_dm.nallocatms) 
    {
    mol_p = PMolecule_Double (mol_p);
    PDrawMol_p = mol_p;
    }
  /* If atom already exists, just return pointer to it 
   */
  dratm_p = RxnDsp_AtomThere_Get (mol_p, gsg, x, y);
  if (dratm_p != NULL)
    return (dratm_p);
      
  strcpy ((RxnDsp_AtomPtr_Get (mol_p, mol_p->both_dm.natms))->sym, sym);
  for (i=0; i<3; i++)
    (RxnDsp_AtomPtr_Get (mol_p, mol_p->both_dm.natms))->chg[i] =
/*
    chg[i]);
*/
    nodenums_on ? mol_p->both_dm.natms + 1 : 0;
  strcpy ((RxnDsp_AtomPtr_Get (mol_p, mol_p->both_dm.natms))->map, map);

  RxnDsp_AtomXCoord_Put (RxnDsp_AtomPtr_Get (mol_p, mol_p->both_dm.natms),
    gsg, x);
  RxnDsp_AtomYCoord_Put (RxnDsp_AtomPtr_Get (mol_p, mol_p->both_dm.natms),
    gsg, y);

  /* Initialize other pattern with out-of-range coordinates before checking
     for conflicting atom */
  RxnDsp_AtomXCoord_Put (RxnDsp_AtomPtr_Get (mol_p, mol_p->both_dm.natms),
    1 - gsg, 0xffff);
  RxnDsp_AtomYCoord_Put (RxnDsp_AtomPtr_Get (mol_p, mol_p->both_dm.natms),
    1 - gsg, 0xffff);

if (IsThere_Draw_Flag && glob_rxlform)
{
      otherpatt_x = 0xffdf;
      otherpatt_y = 0xffdf;
}
else
{
  for (dy = 0; dy <= 500; dy += 25)
    for (dym = (dy == 0 ? 1 : -1); dym < 3; dym +=2)
      for (dx = 0; dx <= 500; dx += 25)
        for (dxm = (dx == 0 ? 1 : -1); dxm < 3; dxm +=2)
    {
    if (x + dxm * dx < 10 || y + dym * dy < 10) continue;
    otherpatt_atm_p = RxnDsp_AtomIntrfr_Get (mol_p, 1 - gsg, x + dxm * dx,
      y + dym * dy, 30);
    if (otherpatt_atm_p == NULL)
      {
      otherpatt_x = x + dxm * dx;
      otherpatt_y = y + dym * dy;
      dx=dy=550;
      dxm=dym=3;
      break;
      }
    }
if (otherpatt_x == 0xffff)
{
printf("coordinate error\n");
exit(1);
}
}

  RxnDsp_AtomXCoord_Put (RxnDsp_AtomPtr_Get (mol_p, mol_p->both_dm.natms),
    1 - gsg, otherpatt_x);
  RxnDsp_AtomYCoord_Put (RxnDsp_AtomPtr_Get (mol_p, mol_p->both_dm.natms),
    1 - gsg, otherpatt_y);

  if (strcmp (sym, "C") == 0)
    (RxnDsp_AtomPtr_Get (mol_p, mol_p->both_dm.natms))->isC = TRUE;
  else
    (RxnDsp_AtomPtr_Get (mol_p, mol_p->both_dm.natms))->isC = FALSE;

  for (i=0; i<2; i++)
    RxnDsp_AtomSelect_Put (RxnDsp_AtomPtr_Get (mol_p, mol_p->both_dm.natms),
      i, PDRW_SELECT_NONE);
  (RxnDsp_AtomPtr_Get (mol_p, mol_p->both_dm.natms))->adj_info.num_neighbs = 0;

  mol_p->both_dm.natms++;
  return (RxnDsp_AtomPtr_Get (mol_p, mol_p->both_dm.natms - 1));
}
/*  End of pstore_AtomNew  */

  
/****************************************************************************
*
*  Function Name:                 pstore_BondNew (used to be store_Bond)
*
*    This routine stores a bond in the DSP structure. 
*    
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Arguments:
*
*    mol_p  - pointer to DSP data structure
*    x1     - "left" atom's x-coordinate
*    y1     - "left" atom's y-coordinate
*    x2     - "right" atom's x-coordinate
*    y2     - "right" atom's y-coordinate
*    nlines - multiplicity of bond
*
*  Return Values:
*
*    pstore_BondNew returns a pointer to a new bond that has been stored.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
 RxnDsp_Bond_t *pstore_BondNew (RxnDsp_Molecule_t  *mol_p,
                                int atom1_gsg,
			        int x1, int y1,
			        int x2, int y2,
			        int nlines)
{
  int             adj_i, adj_j, pat;
  RxnDsp_Atom_t  *statm1_p, *statm2_p;

  /* If num of bonds exceeds num of bond data structures allocated
     in memory, then double it.
   */
  if (mol_p->both_dm.nbnds == mol_p->both_dm.nallocbnds) 
    {
    mol_p = PMolecule_Double (mol_p);
    PDrawMol_p = mol_p;
    }

  statm1_p = RxnDsp_AtomThere_Get (mol_p, atom1_gsg, x1, y1);

  /* If the bond to be stored exceed max number of bonds
     that could be attached to one atom, return NULL.
   */
  if (statm1_p->adj_info.num_neighbs == 6)
    return ((RxnDsp_Bond_t *) NULL);

  statm2_p = RxnDsp_AtomThere_Get (mol_p, gsg, x2, y2);

  /* If the bond to be stored exceed max number of bonds
     that could be attached to one atom, return NULL.
   */
  if (statm2_p->adj_info.num_neighbs == 6)
    return ((RxnDsp_Bond_t *) NULL);

  /* Find number of "left" atom that is adjacent to the bond to be stored 
     in "mol_p" strusture */
  for (adj_i = 0; adj_i < mol_p->both_dm.natms; adj_i++)
    if (RxnDsp_AtomXCoord_Get (RxnDsp_AtomPtr_Get (mol_p, adj_i), gsg) ==
      RxnDsp_AtomXCoord_Get (statm1_p, gsg) &&
      RxnDsp_AtomYCoord_Get (RxnDsp_AtomPtr_Get (mol_p, adj_i), gsg) ==
      RxnDsp_AtomYCoord_Get (statm1_p, gsg))
      break;

  /* Find number of "right" atom that is adjacent to the bond to be stored
     in "mol_p" strusture */
  for (adj_j = 0; adj_j < mol_p->both_dm.natms; adj_j++)
    if (RxnDsp_AtomXCoord_Get (RxnDsp_AtomPtr_Get (mol_p, adj_j), gsg) ==
      RxnDsp_AtomXCoord_Get (statm2_p, gsg) &&
      RxnDsp_AtomYCoord_Get (RxnDsp_AtomPtr_Get (mol_p, adj_j), gsg) ==
      RxnDsp_AtomYCoord_Get (statm2_p, gsg))
      break;

  statm1_p->adj_info.adj_atms[statm1_p->adj_info.num_neighbs] = (long) adj_j;
  statm1_p->adj_info.num_neighbs++;
  statm2_p->adj_info.adj_atms[statm2_p->adj_info.num_neighbs] = (long) adj_i;
  statm2_p->adj_info.num_neighbs++;


  /* Store pointers to the adjacent atoms of the bond. */
  RxnDsp_BondPtr_Get (mol_p, mol_p->both_dm.nbnds)->latm_p = statm1_p;
  RxnDsp_BondPtr_Get (mol_p, mol_p->both_dm.nbnds)->ratm_p = statm2_p;
  for (pat=0; pat<2; pat++)
    {
    RxnDsp_BondOrder_Put (RxnDsp_BondPtr_Get (mol_p, mol_p->both_dm.nbnds),
      pat, GS_NEWBOND (pat, nlines));
    RxnDsp_BondSelect_Put (RxnDsp_BondPtr_Get (mol_p, mol_p->both_dm.nbnds),
      pat, PDRW_SELECT_NONE);
    RxnDsp_BondStereo_Put (RxnDsp_BondPtr_Get (mol_p, mol_p->both_dm.nbnds),
      pat, DSP_BOND_STEREO_NONE);
    }

  mol_p->both_dm.nbnds++;

  return (RxnDsp_BondPtr_Get (mol_p, mol_p->both_dm.nbnds - 1));
}
/*  End of pstore_BondNew  */

/****************************************************************************
*
*  Function Name:                 pdraw_Double_Bond
*
*    This routine draws 2 lines that represent a double bond.
*    It makes drawings on an arbitrary Drawable (X11 type).
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Arguments:
*
*    disp  - pointer to an X11 Display
*    drbl  - X11 Drawable where the drawings take place; could be
*            a DrawingArea or a Pixmap
*    GC    - graphical context that the routine uses for the drawings
*    x1,y1,x2,y2 - coordinates of the bond
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
void pdraw_Double_Bond 
  (
  Display    *disp, 
  Drawable    drbl, 
  GC          gc,
  int         x1, 
  int         y1, 
  int         x2, 
  int         y2
  )
{
  float       length;
  float       norm_x, norm_y;
  int         x0, y0;

  length = sqrt ((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));

  norm_x = (y2-y1)/length; 
  norm_y = (x2-x1)/length;

  norm_x *= PDRW_DBL_BOND_OFFSET;
  norm_y *= PDRW_DBL_BOND_OFFSET;

  x0 = (int) norm_x;
  y0 = (int) norm_y;

  XDrawLine (disp, drbl, gc, x1 + x0, y1 - y0, x2 + x0, y2 - y0);
  XDrawLine (disp, drbl, gc, x1 - x0, y1 + y0, x2 - x0, y2 + y0);

  return ;
}
/*  End of pdraw_Double_Bond  */

/****************************************************************************
*
*  Function Name:                 pdraw_Stereo_Bond
*
*    This routine draws a triangle-shaped area that represents a stereo bond.
*    The pallete that the triangle is filled by depends on the stereo info
*    of the bond.
*    It makes drawings on an arbitrary Drawable (X11 type).
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Arguments:
*
*    disp  - pointer to an X11 Display
*    drbl  - X11 Drawable where the drawings take place; could be
*            a DrawingArea or a Pixmap
*    GC    - graphical context that the routine uses for the drawings
*    stereo- stereo information of the bond; must have one of the values
*            defined in dsp.h
*    x1,y1,x2,y2 - coordinates of the bond
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
void pdraw_Stereo_Bond 
  (
  Display    *disp, 
  Drawable   drbl, 
  GC         gc,
  int        stereo,
  int        x1, 
  int        y1, 
  int        x2, 
  int        y2
  )
{
  float      length;
  float      norm_x, norm_y;
  int        x0, y0;
  XPoint     triangle[3];
  Pixmap     stipple;

  length = sqrt ((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));

  norm_x = (y2-y1)/length; 
  norm_y = (x2-x1)/length;

  norm_x *= PDRW_STR_BOND_OFFSET;
  norm_y *= PDRW_STR_BOND_OFFSET;

  x0 = (int) norm_x;
  y0 = (int) norm_y;

  if ((stereo == DSP_BOND_STEREO_DOWN) || (stereo == DSP_BOND_STEREO_UP)) 
    {
    triangle [0].x = x1;
    triangle [0].y = y1;
    triangle [1].x = x2 + x0;
    triangle [1].y = y2 - y0;
    triangle [2].x = x2 - x0;
    triangle [2].y = y2 + y0;
    }
  else
    {
    triangle [0].x = x2;
    triangle [0].y = y2;
    triangle [1].x = x1 + x0;
    triangle [1].y = y1 - y0;
    triangle [2].x = x1 - x0;
    triangle [2].y = y1 + y0;
    }

  if ((stereo == DSP_BOND_STEREO_DOWN) || (stereo == DSP_BOND_STEREO_OPP_DOWN)) 
    {
    XSetFillStyle (disp, gc, FillStippled);
    if (x0 == 0)
      stipple = SynAppR_IthPMap_Get (&GSynAppR, SAR_PIXMAP_STPLVER);
    else if (y0 == 0)
      stipple = SynAppR_IthPMap_Get (&GSynAppR, SAR_PIXMAP_STPLHOR);
    else if ((x0 * y0) < 0)
      stipple = SynAppR_IthPMap_Get (&GSynAppR, SAR_PIXMAP_STPLNEG);
    else if ((x0 * y0) > 0)
      stipple = SynAppR_IthPMap_Get (&GSynAppR, SAR_PIXMAP_STPLPOS);

    XSetStipple (disp, gc, stipple);
    XFillPolygon (disp, drbl, gc, triangle, 3, Convex, CoordModeOrigin);
    XSetFillStyle (disp, gc, FillSolid);
    }

  /* If stereo is up */
  else  
    XFillPolygon (disp, drbl, gc, triangle, 3, Convex, CoordModeOrigin);

  return ;
}
/*  End of pdraw_Stereo_Bond  */


/****************************************************************************
*
*  Function Name:                 pdraw_Bond
*
*    This routine draws a bond. The information about bond's charachteristics
*    (multiplicity, stereo info, etc.) is taken from the DSP bond data struct.
*    It makes drawings on an arbitrary Drawable (X11 type).
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Arguments:
*
*    disp  - pointer to an X11 Display
*    drbl  - X11 Drawable where the drawings take place; could be
*            a DrawingArea or a Pixmap
*    GC    - graphical context that the routine uses for the drawings
*    color - color of the drawings
*    mol_p - pointer to a DSP data structure
*    bond_p- pointer to the bond to be drawn
*    which - pattern
*    redraw_atom1, redraw_atom2 - variables of boolean type that
*            indicate if the adjacent atoms need to be redrawn
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    Foreground color of Graphical context (GC) is changed to Black.
*    Line attributes of the GC are canged to LineSolid, CapButt, JoinRound
*
******************************************************************************/
void pdraw_Bond 
  (
  Display           *disp, 
  Drawable           drbl, 
  GC                 gc,
  XColor             color,
  RxnDsp_Molecule_t *mol_p,
  RxnDsp_Bond_t     *bond_p,
  int                which,
  Boolean_t          redraw_atom1,
  Boolean_t          redraw_atom2
  )
{
  XSetForeground (disp, gc, color.pixel);
  XSetFillStyle (disp, gc, FillSolid);
  if (RxnDsp_BondStereo_Get (bond_p, which) != DSP_BOND_STEREO_NONE)
    pdraw_Stereo_Bond (disp, drbl, gc, RxnDsp_BondStereo_Get (bond_p, which),
      RxnDsp_AtomXCoord_Get (bond_p->latm_p, which),
      RxnDsp_AtomYCoord_Get (bond_p->latm_p, which),
      RxnDsp_AtomXCoord_Get (bond_p->ratm_p, which),
      RxnDsp_AtomYCoord_Get (bond_p->ratm_p, which));
  else 
    {
    switch (RxnDsp_BondOrder_Get (bond_p, which)) 
      {
      case DSP_BOND_SINGLE:
        XDrawLine (disp, drbl, gc,
          RxnDsp_AtomXCoord_Get (bond_p->latm_p, which),
          RxnDsp_AtomYCoord_Get (bond_p->latm_p, which),
          RxnDsp_AtomXCoord_Get (bond_p->ratm_p, which),
          RxnDsp_AtomYCoord_Get (bond_p->ratm_p, which));
        break;
      case DSP_BOND_TRIPLE:
        XDrawLine (disp, drbl, gc,
          RxnDsp_AtomXCoord_Get (bond_p->latm_p, which),
          RxnDsp_AtomYCoord_Get (bond_p->latm_p, which),
          RxnDsp_AtomXCoord_Get (bond_p->ratm_p, which),
          RxnDsp_AtomYCoord_Get (bond_p->ratm_p, which));
      case DSP_BOND_DOUBLE:	  
        pdraw_Double_Bond (disp, drbl, gc,
          RxnDsp_AtomXCoord_Get (bond_p->latm_p, which),
          RxnDsp_AtomYCoord_Get (bond_p->latm_p, which),
          RxnDsp_AtomXCoord_Get (bond_p->ratm_p, which),
          RxnDsp_AtomYCoord_Get (bond_p->ratm_p, which));
        break;
      case DSP_BOND_RESONANT:
        XSetLineAttributes (disp, gc, 0, LineOnOffDash, CapButt, JoinRound);
        pdraw_Double_Bond (disp, drbl, gc,
          RxnDsp_AtomXCoord_Get (bond_p->latm_p, which),
          RxnDsp_AtomYCoord_Get (bond_p->latm_p, which),
          RxnDsp_AtomXCoord_Get (bond_p->ratm_p, which),
          RxnDsp_AtomYCoord_Get (bond_p->ratm_p, which));
        break;
      case DSP_BOND_VARIABLE:
        XSetLineAttributes (disp, gc, 0, LineOnOffDash, CapButt, JoinRound);
        XDrawLine (disp, drbl, gc,
          RxnDsp_AtomXCoord_Get (bond_p->latm_p, which),
          RxnDsp_AtomYCoord_Get (bond_p->latm_p, which),
          RxnDsp_AtomXCoord_Get (bond_p->ratm_p, which),
          RxnDsp_AtomYCoord_Get (bond_p->ratm_p, which));
        break;
      } /* End of switch (bond num lines) */
    } /* End of else (if stereo is NONE) */
  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
  XSetLineAttributes (disp, gc, 0, LineSolid, CapButt, JoinRound);

  /* If nessesary, redraw atoms on both sides of the bond */
  if (redraw_atom1)
    pdraw_Atom (disp, drbl, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
      SAR_CLRI_BLACK), bond_p->latm_p, PDRW_ATOM_SIZE_LRG);

  if (redraw_atom2)
    pdraw_Atom (disp, drbl, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
      SAR_CLRI_BLACK), bond_p->ratm_p, PDRW_ATOM_SIZE_LRG);
  
  return ;
}
/*  End of pdraw_Bond  */


/****************************************************************************
*
*  Function Name:                 pdraw_Atom
*
*    This routine draws an atom.
*    It makes drawings on an arbitrary Drawable (X11 type).
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Arguments:
*
*    disp  - pointer to an X11 Display
*    drbl1 - X11 Drawable where the drawings take place; could be
*            a DrawingArea or a Pixmap
*    drbl2 - X11 Drawable where the drawings take place; could be
*            a DrawingArea or a Pixmap
*    GC    - graphical context that the routine uses for the drawings
*    color - color of the drawings
*    dratm_p- pointer to the atom to be drawn
*    size  - size of the atom to be drawn (must be one of the values
*            defined in drawmol.h
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    Foreground color of Graphical context (GC) is changed to Black.
*    Font of the GC is canged
*
******************************************************************************/
void pdraw_Atom 
  (
  Display     *disp, 
  Drawable     drbl1, 
  Drawable     drbl2, 
  GC           gc,
  XColor       color,
  Dsp_Atom_t  *dratm_p,
  int          size
  )
{
  XFontStruct   *symfnt_p;
  XFontStruct   *chgfnt_p;
/*  XFontStruct   *mapfnt_p;   Atom mappings not implemented yet */
  XmString       sym_cs;                /* Xm string for symbol */
  XmString       map_cs;                /* Xm string for mapping */
  XmString       chg_cs[2];             /* Xm string for charge */
  Dimension      aw, ah;                /* entire atom's width & height */
  Dimension      cw[2], ch[2],
                 cwmax, chmax;          /* charge width & height */
  Dimension      mw, mh;                /* mapping width & height */
  Dimension      sw, sh;                /* symbol width & height */
  int            ax[2], ay[2];          /* entire atom's coords */
  int            cx[2], cy[2];          /* charge coords */
  int            mx[2], my[2];          /* mapping coords */
  int            sx[2], sy[2];          /* symbol coords */
  int            Goal;
  int            i, chg_cslen[2], dx, dy;
  static char    tmpchg[2][4];
  Boolean_t      onedrbl = drbl1 == drbl2;

/*
  Goal = onedrbl ? gsg : GOAL;
*/
  if (onedrbl)
    {
    if (drbl1 == PDrawTool_Pixmap_Get (&GPDrawToolCB, GOAL) ||
        drbl1 == XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, GOAL)))
      Goal = GOAL;
    else Goal = SUBG;
    }
  else
    Goal = GOAL;

  XSetForeground (disp, gc, color.pixel);

  /* Calculate atom-sym and atom-chg sizes depending on required size
     of entire atom image
   */
  switch (size) 
    {
    case PDRW_ATOM_SIZE_LRG:
/*
      if (dratm_p->isC) 
        {
        sw = 0;
        sh = 0;
        }
      else 
*/
        {
        symfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_LRG);
/*
        sym_cs = XmStringCreateLtoR (dratm_p->sym, SAR_FONTTAG_ATM_LRG);
*/
        sym_cs = XmStringCreateLtoR (dratm_p->sym, SAR_FONTTAG_RXNNAM_NML);
        XmStringExtent (SynAppR_FontList_Get (&GSynAppR), sym_cs, &sw, &sh);
        }

      if (RxnDsp_AtomCharge_Get (dratm_p, Goal) == '\0') 
        {
        cw[Goal] = 0;
        ch[Goal] = 0;
        }
      else 
        {
        sprintf (tmpchg[Goal], "%d", RxnDsp_AtomCharge_Get (dratm_p, Goal));
/*
        if (dratm_p->isC) 
          {
	  chgfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_LRG);
	  chg_cs[Goal] = XmStringCreateLtoR (tmpchg[Goal], SAR_FONTTAG_ATM_LRG);
          }
        else 
*/
          {
/*
	  chgfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_NRML);
	  chg_cs[Goal] = XmStringCreateLtoR (tmpchg[Goal], SAR_FONTTAG_ATM_NML);
*/
	  chgfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_SML);
	  chg_cs[Goal] = XmStringCreateLtoR (tmpchg[Goal], SAR_FONTTAG_ATM_SML);
          }
        chg_cslen[Goal] = strlen (tmpchg[Goal]);

        XmStringExtent (SynAppR_FontList_Get (&GSynAppR), chg_cs[Goal],
          cw + Goal, ch + Goal);
        }

      if (onedrbl);
      else if (RxnDsp_AtomCharge_Get (dratm_p, SUBG) == '\0') 
        {
        cw[SUBG] = 0;
        ch[SUBG] = 0;
        }
      else 
        {
        sprintf (tmpchg[SUBG], "%d", RxnDsp_AtomCharge_Get (dratm_p, SUBG));
/*
        if (dratm_p->isC) 
          {
	  chgfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_LRG);
	  chg_cs[SUBG] = XmStringCreateLtoR (tmpchg[SUBG], SAR_FONTTAG_ATM_LRG);
          }
        else 
*/
          {
/*
	  chgfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_NRML);
	  chg_cs[SUBG] = XmStringCreateLtoR (tmpchg[SUBG], SAR_FONTTAG_ATM_NML);
*/
	  chgfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_SML);
	  chg_cs[SUBG] = XmStringCreateLtoR (tmpchg[SUBG], SAR_FONTTAG_ATM_SML);
          }
        chg_cslen[SUBG] = strlen (tmpchg[SUBG]);

        XmStringExtent (SynAppR_FontList_Get (&GSynAppR), chg_cs[SUBG],
          cw + SUBG, ch + SUBG);
        }

      if (dratm_p->map[0] == '\0' /* || !mol_p->map_em */ ) 
        {
        mw = 0;
        mh = 0;
        }
      else 
        {
        map_cs = XmStringCreateLtoR (dratm_p->map, SAR_FONTTAG_ATM_NML);
        XmStringExtent (SynAppR_FontList_Get (&GSynAppR), map_cs, &mw, &mh);
        }

      break; /* End of large size atom */

    case PDRW_ATOM_SIZE_NML:
      if (dratm_p->isC) 
        {
        sw = 0;
        sh = 0;
        }
      else 
        {
        if ((strcmp (dratm_p->sym, ".") == 0) ||
	    (strcmp (dratm_p->sym, ":") == 0)) 
          {
	  symfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_LRG);
/*
	  sym_cs = XmStringCreateLtoR (dratm_p->sym, SAR_FONTTAG_ATM_LRG);
*/
	  sym_cs = XmStringCreateLtoR (dratm_p->sym, SAR_FONTTAG_RXNNAM_NML);
          }
        else 
          {
	  symfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_NRML);
/*
	  sym_cs = XmStringCreateLtoR (dratm_p->sym, SAR_FONTTAG_ATM_NML);
*/
	  sym_cs = XmStringCreateLtoR (dratm_p->sym, SAR_FONTTAG_RXNNAM_NML);
          }

        XmStringExtent (SynAppR_FontList_Get (&GSynAppR), sym_cs, &sw, &sh);
        }

      if (RxnDsp_AtomCharge_Get (dratm_p, Goal) == '\0') 
        {
        cw[Goal] = 0;
        ch[Goal] = 0;
        }
      else 
        {
        sprintf (tmpchg[Goal], "%d", RxnDsp_AtomCharge_Get (dratm_p, Goal));
/*
        if (dratm_p->isC) 
          {
	  chgfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_NRML);
	  chg_cs[Goal] = XmStringCreateLtoR (tmpchg[Goal], SAR_FONTTAG_ATM_NML);
          }
        else 
*/
          {
	  chgfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_SML);
	  chg_cs[Goal] = XmStringCreateLtoR (tmpchg[Goal], SAR_FONTTAG_ATM_SML);
          }
        chg_cslen[Goal] = strlen (tmpchg[Goal]);

        XmStringExtent (SynAppR_FontList_Get (&GSynAppR), chg_cs[Goal],
          cw + Goal, ch + Goal);
        }

      if (onedrbl);
      else if (RxnDsp_AtomCharge_Get (dratm_p, SUBG) == '\0') 
        {
        cw[SUBG] = 0;
        ch[SUBG] = 0;
        }
      else 
        {
        sprintf (tmpchg[SUBG], "%d", RxnDsp_AtomCharge_Get (dratm_p, SUBG));
/*
        if (dratm_p->isC) 
          {
	  chgfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_NRML);
	  chg_cs[SUBG] = XmStringCreateLtoR (tmpchg[SUBG], SAR_FONTTAG_ATM_NML);
          }
        else 
*/
          {
	  chgfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_SML);
	  chg_cs[SUBG] = XmStringCreateLtoR (tmpchg[SUBG], SAR_FONTTAG_ATM_SML);
          }
        chg_cslen[SUBG] = strlen (tmpchg[SUBG]);

        XmStringExtent (SynAppR_FontList_Get (&GSynAppR), chg_cs[SUBG],
          cw + SUBG, ch + SUBG);
        }

      if (dratm_p->map[0] == '\0' /*|| !mol_p->map_em*/ ) 
        {
        mw = 0;
        mh = 0;
        }
      else 
        {
        map_cs = XmStringCreateLtoR (dratm_p->map, SAR_FONTTAG_ATM_SML);
        XmStringExtent (SynAppR_FontList_Get (&GSynAppR), map_cs, &mw, &mh);
        }

      break; /* End of normal size atom */

    case PDRW_ATOM_SIZE_SML:
      if (dratm_p->isC) 
        {
        sw = 0;
        sh = 0;
        }
      else 
        {
        if ((strcmp (dratm_p->sym, ".") == 0) ||
	    (strcmp (dratm_p->sym, ":") == 0)) 
          {
	  symfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_NRML);
/*
	  sym_cs = XmStringCreateLtoR (dratm_p->sym, SAR_FONTTAG_ATM_NML);
*/
	  sym_cs = XmStringCreateLtoR (dratm_p->sym, SAR_FONTTAG_RXNNAM_NML);
          }
        else 
          {
	  symfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_SML);
/*
	  sym_cs = XmStringCreateLtoR (dratm_p->sym, SAR_FONTTAG_ATM_SML);
*/
	  sym_cs = XmStringCreateLtoR (dratm_p->sym, SAR_FONTTAG_RXNNAM_SML);
          }

        XmStringExtent (SynAppR_FontList_Get (&GSynAppR), sym_cs, &sw, &sh);
        }

      if (RxnDsp_AtomCharge_Get (dratm_p, Goal) == '\0') 
        {
        cw[Goal] = 0;
        ch[Goal] = 0;
        }
      else 
        {
        sprintf (tmpchg[Goal], "%d", RxnDsp_AtomCharge_Get (dratm_p, Goal));
        chgfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_SML);
        chg_cs[Goal] = XmStringCreateLtoR (tmpchg[Goal], SAR_FONTTAG_ATM_SML);
        chg_cslen[Goal] = strlen (tmpchg[Goal]);
        XmStringExtent (SynAppR_FontList_Get (&GSynAppR), chg_cs[Goal],
          cw + Goal, ch + Goal);
        }

      if (onedrbl);
      else if (RxnDsp_AtomCharge_Get (dratm_p, SUBG) == '\0') 
        {
        cw[SUBG] = 0;
        ch[SUBG] = 0;
        }
      else 
        {
        sprintf (tmpchg[SUBG], "%d", RxnDsp_AtomCharge_Get (dratm_p, SUBG));
        chgfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_SML);
        chg_cs[SUBG] = XmStringCreateLtoR (tmpchg[SUBG], SAR_FONTTAG_ATM_SML);
        chg_cslen[SUBG] = strlen (tmpchg[SUBG]);
        XmStringExtent (SynAppR_FontList_Get (&GSynAppR), chg_cs[SUBG],
          cw + SUBG, ch + SUBG);
        }

      if (dratm_p->map[0] == '\0' /*|| !mol_p->map_em*/ ) 
        {
        mw = 0;
        mh = 0;
        }
      else 
        {
        map_cs = XmStringCreateLtoR (dratm_p->map, SAR_FONTTAG_ATM_SML);
        XmStringExtent (SynAppR_FontList_Get (&GSynAppR), map_cs, &mw, &mh);
        }

      break; /* End of small size atom */
    } /* End of switch size */

  chmax = ch[onedrbl ? Goal : ch[GOAL] > ch[SUBG] ? GOAL : SUBG];
  cwmax = cw[onedrbl ? Goal : cw[GOAL] > cw[SUBG] ? GOAL : SUBG];

  ah = sh + (chmax >> 1) + (mh >> 1);
  aw = sw + ((cwmax > mw) ? cwmax : mw);
  for (i=0; i<2; i++)
    {
/*
    ax[i] = RxnDsp_AtomXCoord_Get (dratm_p, i) - (aw >> 1);
    ay[i] = RxnDsp_AtomYCoord_Get (dratm_p, i) + (ah >> 1);
*/
    ax[i] = RxnDsp_AtomXCoord_Get (dratm_p, i) - (sw >> 1);
    ay[i] = RxnDsp_AtomYCoord_Get (dratm_p, i) + (sh >> 1);
    sx[i] = ax[i];
    sy[i] = ay[i] - (mh >> 1);
    cx[i] = ax[i] + sw;
    cy[i] = ay[i] - (ah >> 1);
    mx[i] = ax[i] + sw;
    my[i] = ay[i];
    }

  if (!dratm_p->isC) 
    {
    XSetFont (disp, gc, symfnt_p->fid);
    XDrawImageString (disp, drbl1, gc, sx[Goal], sy[Goal],
      dratm_p->sym, strlen (dratm_p->sym));    
    if (!onedrbl) XDrawImageString (disp, drbl2, gc, sx[SUBG], sy[SUBG],
      dratm_p->sym, strlen (dratm_p->sym));
    for (dy = /* -1 */ 0; dy < 1; dy++)    
      for (dx = -1; dx < 1; dx++)
      {
      XDrawString (disp, drbl1, gc, sx[Goal] + dx, sy[Goal] + dy,
        dratm_p->sym, strlen (dratm_p->sym));    
      if (!onedrbl) XDrawString (disp, drbl2, gc,
        sx[SUBG] + dx, sy[SUBG] + dy,
        dratm_p->sym, strlen (dratm_p->sym));    
      }
    XmStringFree (sym_cs);
    }
  else
    {
    XFillArc (disp, drbl1, gc, RxnDsp_AtomXCoord_Get (dratm_p, Goal) - 5,
      RxnDsp_AtomYCoord_Get (dratm_p, Goal) - 5, 10, 10, 0, 360 * 64);
    if (!onedrbl)
      XFillArc (disp, drbl2, gc, RxnDsp_AtomXCoord_Get (dratm_p, SUBG) - 5,
        RxnDsp_AtomYCoord_Get (dratm_p, SUBG) - 5, 10, 10, 0, 360 * 64);
    }

  if (RxnDsp_AtomCharge_Get (dratm_p, Goal) != '\0') 
    {
    XSetFont (disp, gc, chgfnt_p->fid);
    XDrawImageString (disp, drbl1, gc, cx[Goal], cy[Goal],
/*
      dratm_p->chg + Goal, 1);
*/
      tmpchg[Goal], chg_cslen[Goal]);
    XmStringFree (chg_cs[Goal]);
    }

  if (!onedrbl && RxnDsp_AtomCharge_Get (dratm_p, SUBG) != '\0') 
    {
    XSetFont (disp, gc, chgfnt_p->fid);
    XDrawImageString (disp, drbl2, gc, cx[SUBG], cy[SUBG],
/*
      dratm_p->chg + SUBG, 1);
*/
      tmpchg[SUBG], chg_cslen[SUBG]);
    XmStringFree (chg_cs[SUBG]);
    }

  XSetFont (disp, gc, symfnt_p->fid);

  if ( /*mol_p->map_em && */ dratm_p->map[0] != '\0') 
    {
    XDrawImageString (disp, drbl1, gc, mx[Goal], my[Goal], dratm_p->sym, 
      strlen (dratm_p->map));
    if (!onedrbl) XDrawImageString (disp, drbl2, gc, mx[SUBG], my[SUBG],
      dratm_p->sym, strlen (dratm_p->map));
    XmStringFree (map_cs);
    }

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
  return ;
}
/*  End of pdraw_Atom  */


/****************************************************************************
*
*  Function Name:                 perase_Bond
*
*    This routine erases a bond. The information about the bond in the DSP
*    data structure is deleted as well using delete_Bond() function.
*    If any of the adjacent atoms is Carbon and has no bond adjacent to it
*    except the one that is being deleted, it is deleted from the DSP (no
*    drawings need to be done for this, since carbons don't have a symbol
*    on the screen).
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Arguments:
*
*    disp  - pointer to an X11 Display
*    drbl  - X11 Drawable where the drawings take place; could be
*            a DrawingArea or a Pixmap
*    GC    - graphical context that the routine uses for the drawings
*    mol_p - pointer to the DSP molecule
*    erbnd_p- pointer to the bond to be erased
*    which - which pattern
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    Foreground color of Graphical context (GC) is changed to Black,
*    since it calls draw_Bond to erase the drawings.
*
******************************************************************************/
void perase_Bond 
  (
  Display           *disp, 
  Drawable           drbl1, 
  Drawable           drbl2, 
  Drawable           drbl3,
  Drawable           drbl4,
  GC                 gc,
  RxnDsp_Molecule_t *mol_p,
  RxnDsp_Bond_t     *erbnd_p,
  int                which
  )
{
  RxnDsp_Atom_t     *left_p, *right_p;
  int                atms_to_del = 0;
  int                local_which;

  left_p = erbnd_p->latm_p;
  right_p = erbnd_p->ratm_p;
  
  if (which == BOTH)
    {
    pdraw_Bond (disp, drbl1, gc,
      SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE),
      mol_p, erbnd_p, GOAL, TRUE, TRUE);
    pdraw_Bond (disp, drbl2, gc,
      SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE),
      mol_p, erbnd_p, SUBG, TRUE, TRUE);
    local_which = GOAL;
    }
  else
    {
    pdraw_Bond (disp, drbl1, gc,
      SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE),
      mol_p, erbnd_p, which, TRUE, TRUE);
    local_which = which;
    }

  /* Delete this bond from mol data structure */
  if (!rxn_delete_Bond (mol_p, erbnd_p, which)) return;

  /* If an adjacent atoms don't have neighbors and are Carbons,
     delete them.
   */
  if (left_p->adj_info.num_neighbs == 0 && left_p->isC
      && RxnDsp_AtomCharge_Get (left_p, local_which) == '\0')
    {
    atms_to_del += 1;
    pdraw_Atom (disp, drbl1, drbl2, gc,
      SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE),
      left_p, PDRW_ATOM_SIZE_LRG);
    pdraw_Atom (disp, drbl3, drbl4, gc,
      SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE),
      left_p, PDRW_ATOM_SIZE_LRG);
    }

  if (right_p->adj_info.num_neighbs == 0 && right_p->isC 
      && RxnDsp_AtomCharge_Get (right_p, local_which) == '\0')
    {
    atms_to_del += 2;
    pdraw_Atom (disp, drbl1, drbl2, gc,
      SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE),
      right_p, PDRW_ATOM_SIZE_LRG);
    pdraw_Atom (disp, drbl3, drbl4, gc,
      SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE),
      right_p, PDRW_ATOM_SIZE_LRG);
    }

  switch (atms_to_del) 
    {
    case 1:
      check_roots (mol_p, left_p);
      rxn_delete_Atom (mol_p, left_p, which);
      break;

    case 2:
      check_roots (mol_p, right_p);
      rxn_delete_Atom (mol_p, right_p, which);
      break;

    case 3:
      check_roots (mol_p, left_p);
      check_roots (mol_p, right_p);
      if (left_p == RxnDsp_AtomPtr_Get (mol_p, mol_p->both_dm.natms)) 
        {
        if (rxn_delete_Atom (mol_p, right_p, which))
          rxn_delete_Atom (mol_p, right_p, which);
        else
          rxn_delete_Atom (mol_p, left_p, which);
        }
      else if (right_p == RxnDsp_AtomPtr_Get(mol_p, mol_p->both_dm.natms)) 
        {
        if (rxn_delete_Atom (mol_p, left_p, which))
          rxn_delete_Atom (mol_p, left_p, which);
        else
          rxn_delete_Atom (mol_p, right_p, which);
        }
      else 
        {
        rxn_delete_Atom (mol_p, right_p, which);
        rxn_delete_Atom (mol_p, left_p, which);
        }
      break;

    default:
      break;
    } /* End of switch */    

  return;
}
/*  End of perase_Bond  */

/****************************************************************************
*
*  Function Name:                 pdraw_Molecule
*
*    This routine draws a DSP molecule. It can resize
*    the molecule according to the scaling factor.
*    It also can center the molecule.
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Arguments:
*
*    disp  - pointer to an X11 Display
*    drbl  - X11 Drawable where the drawings take place; could be
*            a DrawingArea or a Pixmap
*    GC    - graphical context that the routine uses for the drawings
*    mol_p - pointer to the molecule to be drawn
*    sc_factor - scaling factor to resize the molecule. If = 1,
*            the molecule remains of the same size. If < 1, the
*            molecule is shrunk. If > 1, the molecule is enlarged
*    height, width - dimensions of the region where the molecule
*            needs to be centered in. If to_center is not set,
*            they are not used
*    to_center - indicates if the molecule needs to be centered
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    Foreground color of Graphical context (GC) is changed to Black.
*    Font of the GC is canged
*
******************************************************************************/
void pdraw_Molecule 
  (
  Display           *disp, 
  Drawable           drbl, 
  GC                 gc,
  RxnDsp_Molecule_t *mol_p,
  float              sc_factor,
  Dimension          height,
  Dimension          width,
  int                which,
  Boolean_t          to_center
  )
{
  RxnDsp_Atom_t *atom_p;
  RxnDsp_Bond_t *bond_p;
  int            maxx, maxy, minx, miny;
  int            atom_size;
  int            h_RPDeltaDAW, h_RPDeltaDAH;
  int            atm_i, bnd_i, root_i, j;

  if (sc_factor < 0) 
    {
    fprintf (stderr, "pdraw_Molecule : Negative sc factor\n");
    exit (-1);
    }

  /* Calculate actual width and height */
  width  -= AppDim_AtmMaxW_Get (&GAppDim);
  height -= AppDim_AtmMaxH_Get (&GAppDim);
  if (sc_factor != 1.00 && which != SUBG) 
    {
    /* Calculate atom xy-coordinates according to scaling factor */
    atom_p = RxnDsp_AtomPtr_Get (mol_p, 0);

    for (atm_i = 0; atm_i < mol_p->both_dm.natms; atm_i++) 
      {
      for (j = 0; j < 2; j++)
        {
        RxnDsp_AtomXCoord_Put (atom_p, j,
          (int) (RxnDsp_AtomXCoord_Get (atom_p, j) * sc_factor));
        RxnDsp_AtomYCoord_Put (atom_p, j,
          (int) (RxnDsp_AtomYCoord_Get (atom_p, j) * sc_factor));
        }
      ++atom_p;
      }
    for (root_i = 0; root_i < RxnDsp_RootCnt_Get (mol_p); root_i++)
      {
      RxnDsp_RootPtr_Get (mol_p, root_i)->x =
        (int) (RxnDsp_RootPtr_Get (mol_p, root_i)->x * sc_factor);
      RxnDsp_RootPtr_Get (mol_p, root_i)->y =
        (int) (RxnDsp_RootPtr_Get (mol_p, root_i)->y * sc_factor);
      }
    }

  /* If molecule needs to be centered, calculate max xy coordinates */
  if (to_center) 
    {
    maxx = maxy = 0;
    h_RPDeltaDAW = AppDim_AtmMaxW_Get (&GAppDim) >> 1;
    h_RPDeltaDAH = AppDim_AtmMaxH_Get (&GAppDim) >> 1;
    atom_p = RxnDsp_AtomPtr_Get (mol_p, 0);
    minx = maxx = RxnDsp_AtomXCoord_Get (atom_p, GOAL);
    miny = maxy = RxnDsp_AtomYCoord_Get (atom_p, GOAL);

    for (atm_i = 0; atm_i < mol_p->both_dm.natms; atm_i++) 
      {
      for (j = 0; j < 2; j++)
        {
        if (minx > RxnDsp_AtomXCoord_Get (atom_p, j))
            minx = RxnDsp_AtomXCoord_Get (atom_p, j);
        if (maxx < RxnDsp_AtomXCoord_Get (atom_p, j))
            maxx = RxnDsp_AtomXCoord_Get (atom_p, j);
        if (miny > RxnDsp_AtomYCoord_Get (atom_p, j))
            miny = RxnDsp_AtomYCoord_Get (atom_p, j);
        if (maxy < RxnDsp_AtomYCoord_Get (atom_p, j))
            maxy = RxnDsp_AtomYCoord_Get (atom_p, j);
        }
      ++atom_p;
      }

    maxx += h_RPDeltaDAW;
    maxy += h_RPDeltaDAH;
    maxx = ((maxx < (int) width) && ((width  - maxx) >> 1 > h_RPDeltaDAW)) 
      ? ((int) width - maxx) >> 1 : h_RPDeltaDAW;
    maxy = ((maxy < (int)height) && ((height - maxy) >> 1 > h_RPDeltaDAH)) 
      ? ((int)height - maxy) >> 1 : h_RPDeltaDAH;

    /* Recalculate atom xy-coordinates to center the molecule */
    atom_p = RxnDsp_AtomPtr_Get (mol_p, 0);
    for (atm_i = 0; atm_i < mol_p->both_dm.natms; atm_i++) 
      {
      for (j = 0; j < 2; j++)
        {
        RxnDsp_AtomXCoord_Put (atom_p, j,
          RxnDsp_AtomXCoord_Get (atom_p, j) + maxx);
        RxnDsp_AtomYCoord_Put (atom_p, j,
          RxnDsp_AtomYCoord_Get (atom_p, j) + maxy);
        }
      ++atom_p;
      }
    for (root_i = 0; root_i < RxnDsp_RootCnt_Get (mol_p); root_i++)
      {
      RxnDsp_RootPtr_Get (mol_p, root_i)->x += maxx;
      RxnDsp_RootPtr_Get (mol_p, root_i)->y += maxy;
      }
    }

  /*  Draw the bonds of the molecule first. */
  bond_p = RxnDsp_BondPtr_Get (mol_p, 0);
  for (bnd_i = 0; bnd_i < mol_p->both_dm.nbnds; bnd_i++) 
    {
    pdraw_Bond (disp, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
      SAR_CLRI_BLACK), mol_p, bond_p, which, FALSE, FALSE);
    ++bond_p;
    }

  /* Set atom size depending on a scaling factor (since he uses large
     everywhere else, use it here as well).  - DK
  if (mol_p->both_dm.scale < DSP_DELTA_NORM_MIN && mol_p->both_dm.scale != 1.0)
    atom_size = PDRW_ATOM_SIZE_LRG;
  else if (mol_p->both_dm.scale < DSP_DELTA_NORM_MAX)
    atom_size = PDRW_ATOM_SIZE_NML;
  else
    atom_size = PDRW_ATOM_SIZE_SML;
   */

  atom_size = PDRW_ATOM_SIZE_LRG;

  /*  Draw the atoms of the molecule. */
  atom_p = RxnDsp_AtomPtr_Get (mol_p, 0);

  for (atm_i = 0; atm_i < mol_p->both_dm.natms; atm_i++) 
    {
    /* If atom has no neighbors and is Carbon, draw its symbol */
    /* Unnecessary and confusing complication in this case */
/*
    if ((atom_p->adj_info.num_neighbs == 0) 
        && (strcmp (atom_p->sym, "C") == 0))
      atom_p->isC = FALSE;
*/

    pdraw_Atom (disp, drbl, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
      SAR_CLRI_BLACK), atom_p, atom_size);
    ++atom_p;
    }

  return ;
}
/*  End of pdraw_Molecule  */

/****************************************************************************
*  
*  Function Name:                 RxnInfo_SlingTxt_Update
*  
*****************************************************************************/  

void RxnInfo_SlingTxt_Update 
  ( 
  RxnInfoCB_t      **rxninfo_p, 
  RxnDsp_Molecule_t *mol_p
  )
{
  Tsd_t    *tsd_p; 
  Sling_t   sling;  
int j,k,l,nbr,bond;
/*
  Sling_t   canon_sling;  
*/
  int pat,i;

  if (mol_p == NULL)
    return; 

  for (pat=0; pat<2; pat++)
    {
    tsd_p = RxnDsp2Tsd (mol_p, pat);
    sling = Tsd2Sling (tsd_p);

    strncpy (RxnInfo_Sling_Get (rxninfo_p[pat]), 
      (char *) Sling_Name_Get (sling), RXN_SLINGBUF_MAXLEN);
    RxnInfo_Sling_Get (rxninfo_p[pat])[RXN_SLINGBUF_MAXLEN] = '\0';
/*
  XmTextSetString (RxnInfo_SlingTxt_Get (rxninfo_p[pat]), 
    RxnInfo_Sling_Get (rxninfo_p[pat]));
*/
    Sling_Destroy (sling);
/*
    Sling_Destroy (canon_sling);
*/

    Tsd_Destroy (tsd_p);
    }

  return;
}
/* End of RxnInfo_SlingTxt_Update */

/****************************************************************************
*  
*  Function Name:                 pleave_drawarea
*  
*****************************************************************************/  

void pleave_drawarea
  (
  Widget    w,
  XtPointer client_data,
  XEvent   *event,
  Boolean  *cont
  )
{
  static String  args[] = {"leave"};
  static int     num_args = 1;

  if (event->type == LeaveNotify)
    PDraw_Draw (w, &event->xbutton, args, &num_args);
}
/* End of pleave_drawarea */

/****************************************************************************
*  
*  Function Name:                 penter_drawarea
*  
*****************************************************************************/  

void penter_drawarea
  (
  Widget    w,
  XtPointer client_data,
  XEvent   *event,
  Boolean  *cont
  )
{
  if (event->type == EnterNotify)
    gsg = *(int *)client_data;
}
/* End of penter_drawarea */

/****************************************************************************
*  
*  Function Name:                 draw_roots
*  
*****************************************************************************/  

void draw_roots (Display *disp, Drawable drbl1, Drawable drbl2, GC gc,
  RxnDsp_Molecule_t *mol_p)
{
  int            i, j, x, y;
  RxnDsp_Atom_t *ap;
  Boolean_t      root_found;
  char           rs_string[10];

XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
/*
XFillRectangle (disp, drbl1, gc, 0, 0, 100, 100);
XFillRectangle (disp, drbl2, gc, 0, 0, 100, 100);
*/
XFillRectangle (disp, XtWindow(PDrawTool_RootSynArea_Get(&GPDrawToolCB)),
gc, 0, 0, 150, 175);

  for (i=0; i<RxnDsp_RootCnt_Get (mol_p); i++)
    for (j=0, root_found=FALSE; !root_found && j<mol_p->both_dm.natms; j++)
      if (RxnDsp_AtomIsRoot_Get (mol_p, j, i))
    {
    root_found = TRUE;
    ap = RxnDsp_AtomPtr_Get (mol_p, j);
    x = RxnDsp_AtomXCoord_Get (ap, GOAL);
    y = RxnDsp_AtomYCoord_Get (ap, GOAL);

    XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLUE));
    XDrawRectangle (disp, drbl1, gc, x - 12, y - 10, 30, 25);
    XDrawRectangle (disp, drbl2, gc, x - 12, y - 10, 30, 25);

/*
sprintf(rs_string, "R%d S%02d", i+1,
RxnDsp_RootPtr_Get (mol_p, i)->syntheme);

XDrawString(disp, drbl1, gc, 10, 20 * (i + 1),
rs_string, strlen(rs_string));
XDrawString(disp, drbl2, gc, 10, 20 * (i + 1),
rs_string, strlen(rs_string));
*/
    sprintf(rs_string, "Root %d (Syntheme %2d)", i+1,
      RxnDsp_RootPtr_Get (PDrawMol_p, i)->syntheme);

    XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));

    XDrawString(PDrawDisplay,
      XtWindow(PDrawTool_RootSynArea_Get(&GPDrawToolCB)),
      gc, 5, 20 * (2 * i + 1), rs_string, strlen(rs_string));

    sprintf(rs_string, "at Node %d", j+1);

    XDrawString(PDrawDisplay,
      XtWindow(PDrawTool_RootSynArea_Get(&GPDrawToolCB)),
      gc, 25, 20 * (2 * i + 2), rs_string, strlen(rs_string));

    }
}
/* End of draw_roots */

/****************************************************************************
*  
*  Function Name:                 check_roots
*  
*****************************************************************************/  

void check_roots (RxnDsp_Molecule_t *mol_p, RxnDsp_Atom_t *atom_p)
{
  int       i, j, x, y;
  Drawable  drbl1, drbl2;
  GC        gc;

  if (!gsgsame && gsg != GOAL) return; /* do NOT remove root just because it is disconnected in the SUBGOAL pattern! */

  for (i=0; i<RxnDsp_RootCnt_Get (mol_p); i++)
    if (RxnDsp_AtomIsRoot_Get
      (mol_p, atom_p - RxnDsp_AtomPtr_Get (mol_p, 0), i))
    {
    drbl1 = PDrawTool_Pixmap_Get (&GPDrawToolCB, GOAL);
    drbl2 = XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, GOAL));
    gc = PDrawTool_GC_Get (&GPDrawToolCB);
    x = RxnDsp_AtomXCoord_Get (atom_p, GOAL);
    y = RxnDsp_AtomYCoord_Get (atom_p, GOAL);
    XSetForeground (PDrawDisplay, gc,
      SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
    XDrawRectangle (PDrawDisplay, drbl1, gc, x - 12, y - 10, 30, 25);
    XDrawRectangle (PDrawDisplay, drbl2, gc, x - 12, y - 10, 30, 25);
    RxnDsp_RootCnt_Put (mol_p, RxnDsp_RootCnt_Get (mol_p) - 1);
    for (j=i; j<RxnDsp_RootCnt_Get (mol_p); j++)
      {
      RxnDsp_RootPtr_Get (mol_p, j)->x = RxnDsp_RootPtr_Get (mol_p, j + 1)->x;
      RxnDsp_RootPtr_Get (mol_p, j)->y = RxnDsp_RootPtr_Get (mol_p, j + 1)->y;
      RxnDsp_RootPtr_Get (mol_p, j)->syntheme =
        RxnDsp_RootPtr_Get (mol_p, j + 1)->syntheme;
      }
    draw_roots (PDrawDisplay, drbl1, drbl2, gc, mol_p);

    return;
    }
}
/* End of check_roots */

/****************************************************************************
*  
*  Function Name:                 Syntheme_Store_CB
*  
*****************************************************************************/  

void Syntheme_Store_CB
  (
  Widget           w,
  XtPointer        client_data,
  XtPointer        call_data             
)
{
  char     sling[500], *this, *next, list_str[200];
  XmString lbl_str;
  int      nslings, i;

  XmListCallbackStruct *cbs = (XmListCallbackStruct *) call_data;

  if (cbs->reason != XmCR_SINGLE_SELECT) return;
/*
  if (Selected_Syntheme != 0)
    {
    if (RxnDsp_RootCnt_Get (PDrawMol_p) == 0) return;
    RxnDsp_RootCnt_Put (PDrawMol_p, RxnDsp_RootCnt_Get (PDrawMol_p) - 1);
    }
*/
  Selected_Syntheme =
    PDrawTool_SynListNum_Get (&GPDrawToolCB, cbs->item_position);

/*
  PDrawFlags.drawing_mode = PDRW_SYNSELECT_FLAG;
*/
  XtUnmanageChild (PDrawTool_SynListForm_Get (&GPDrawToolCB));

  XmListDeleteAllItems (PDrawTool_SlingList_Get (&GPDrawToolCB));

  strcpy (sling, GPDrawToolCB.slings[cbs->item_position]);
  if (cbs->item_position == 8) strcat (sling, "X\\");

  XmListSetAddMode (PDrawTool_SlingList_Get (&GPDrawToolCB), TRUE);

  for (this = sling, nslings = 0; *this; this = next + 1, nslings++)
    {
    next = strstr (this, "\\");
    *next = 0;
    strncpy (sling_roots[nslings], this, 4);
    if (sling_roots[nslings][0] == '(')
      {
      sling_roots[nslings][3] = '\0';
      strcpy (sling_roots[nslings], sling_roots[nslings] + 1);
      }
    else sling_roots[nslings][1] = '\0';
    sprintf(list_str,"%s [Root atom = %s]",this,sling_roots[nslings]);
    lbl_str = XmStringCreateLocalized (list_str);
    XmListAddItem (PDrawTool_SlingList_Get (&GPDrawToolCB), lbl_str, 0);
    XmStringFree (lbl_str);
    }

  XmListSetAddMode (PDrawTool_SlingList_Get (&GPDrawToolCB), FALSE);

  XmListSetBottomPos (PDrawTool_SlingList_Get (&GPDrawToolCB), nslings);
  XmListSetPos (PDrawTool_SlingList_Get (&GPDrawToolCB), 1);

  XtVaSetValues(PDrawTool_SlingList_Get (&GPDrawToolCB),
    XmNvisibleItemCount,    nslings,
    NULL);

  XtManageChild (PDrawTool_SlingListForm_Get (&GPDrawToolCB));

  strcpy (root_atom, sling_roots[0]);
  /* If all root atoms are the same, allow bypassing of sling selection */
  for (i = 1; root_atom[0] != 0 && i < nslings; i++)
    if (strcmp (root_atom, sling_roots[i]) != 0)
      root_atom[0] = 0;
}

/****************************************************************************
*  
*  Function Name:                 Sling_Select_CB
*  
*****************************************************************************/  

void Sling_Select_CB
  (
  Widget           w,
  XtPointer        client_data,
  XtPointer        call_data             
)
{
  XmListCallbackStruct *cbs = (XmListCallbackStruct *) call_data;
  if (cbs->reason != XmCR_SINGLE_SELECT) return;

  strcpy (root_atom, sling_roots[cbs->item_position - 1]);
  
  XtUnmanageChild (PDrawTool_SlingListForm_Get (&GPDrawToolCB));
}

/****************************************************************************
*  
*  Function Name:                 redisplay_roots
*  
*****************************************************************************/  

void redisplay_roots
  (
  Widget           w,
  XtPointer        client_data,
  XtPointer        call_data             
)
{
  int            i, j, x, y;
  RxnDsp_Atom_t *ap;
  Boolean_t      root_found;
  char           rs_string[10];
  GC             gc;

  gc = PDrawTool_GC_Get (&GPDrawToolCB);

  XSetForeground (PDrawDisplay, gc,
    SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));

  XFillRectangle (PDrawDisplay,
    XtWindow(PDrawTool_RootSynArea_Get(&GPDrawToolCB)), gc, 0, 0, 150, 175);

  XSetForeground (PDrawDisplay, gc,
    SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));

  for (i=0; i<RxnDsp_RootCnt_Get (PDrawMol_p); i++)
    for (j=0, root_found=FALSE; !root_found && j<PDrawMol_p->both_dm.natms; j++)
      if (RxnDsp_AtomIsRoot_Get (PDrawMol_p, j, i))
    {
    root_found = TRUE;
    sprintf(rs_string, "Root %d (Syntheme %2d)", i+1,
      RxnDsp_RootPtr_Get (PDrawMol_p, i)->syntheme);

    XDrawString(PDrawDisplay,
      XtWindow(PDrawTool_RootSynArea_Get(&GPDrawToolCB)),
      gc, 5, 20 * (2 * i + 1), rs_string, strlen(rs_string));

    sprintf(rs_string, "at Node %d", j+1);

    XDrawString(PDrawDisplay,
      XtWindow(PDrawTool_RootSynArea_Get(&GPDrawToolCB)),
      gc, 25, 20 * (2 * i + 2), rs_string, strlen(rs_string));

    }
}

/****************************************************************************
*  
*  Function Name:                 root_syntheme
*  
*****************************************************************************/  

int root_syntheme(RxnDsp_Molecule_t *mol_p, U32_t n)
{
int i;

for (i=0; i<mol_p->num_roots; i++)
  if (mol_p->root_syn[i].x==mol_p->both_dm.atoms[n].x &&
      mol_p->root_syn[i].y==mol_p->both_dm.atoms[n].y)
  return(mol_p->root_syn[i].syntheme);
return(0);
}

/****************************************************************************
*  
*  Function Name:                 PDismissErr_CB
*  
*****************************************************************************/  

void PDismissErr_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild (PDrawTool_ExitErrorMsg_Get (&GPDrawToolCB));
}

/****************************************************************************
*  
*  Function Name:                 Get_Selected_Syntheme
*  
*****************************************************************************/  

void Get_Selected_Syntheme (int sy)
{
  char     sling[500], *this, *next, *ch;
  int      nslings, i, j, k;

  Selected_Syntheme = sy;

  strcpy (sling, GPDrawToolCB.slings[sy]);
  if (sy == 8) strcat (sling, "X\\");
  else if (sy == 25) while ((ch = strstr (sling, "(#k)")) != NULL)
    strncpy (ch, "(Ch)", 4);

  for (this = sling, nslings = 0; *this; this = next + 1, nslings++)
    {
    next = strstr (this, "\\");
    *next = 0;
    strncpy (sling_roots[nslings], this, 4);
    if (sling_roots[nslings][0] == '(')
      {
      sling_roots[nslings][3] = '\0';
      strcpy (sling_roots[nslings], sling_roots[nslings] + 1);
      }
    else sling_roots[nslings][1] = '\0';
    }

  for (i = 0; i < nslings - 1; i++)
    for (j = i + 1; j < nslings;) if (strcmp (sling_roots[i], sling_roots[j]) == 0)
    {
    for (k = j + 1; k < nslings; k++) strcpy (sling_roots[k - 1], sling_roots[k]);
    nslings--;
    }
  else j++;

  sling_roots[nslings][0] = '\0';
}

Boolean_t Root_Atom_OK (char *sym)
{
  int i;

  for (i = 0; sling_roots[i][0] != '\0'; i++)
    if (strcasecmp (sym, sling_roots[i]) == 0) return (TRUE);

  return (FALSE);
}

void IsTList_Update (Boolean_t saved)
{
  int i, j;
  GC                gc;
  Pixmap            pix[2];
  Window            win[2];
  ScreenAttr_t     *sca_p;
  Display          *display;
  char              str[16];

  PDraw_Mode_Reset (PDRW_BOND_FLAG);

  sca_p = SynAppR_ScrnAtrb_Get (&GSynAppR);
  display = Screen_TDisplay_Get (sca_p);
  gc = PDrawTool_GC_Get (&GPDrawToolCB);
  for (i=0; i<2; i++)
    {
    pix[i] = PDrawTool_Pixmap_Get (&GPDrawToolCB, i);
    win[i] = XtWindow (PDrawTool_DrawArea_Get (&GPDrawToolCB, i));
    }

  if (!saved) return;

  XSetForeground (display, gc, SynAppR_IthClrPx_Get (&GSynAppR, 
    SAR_CLRI_WHITE));
 
  XFillRectangle (display, win[0], gc, 0, 0, 50, 250);
  XFillRectangle (display, pix[0], gc, 0, 0, 50, 250);
  XFillRectangle (display, win[1], gc, 0, 0, 50, 250);
  XFillRectangle (display, pix[1], gc, 0, 0, 50, 250);

  XSetForeground (display, gc, SynAppR_IthClrPx_Get (&GSynAppR, 
    SAR_CLRI_BLACK));

  XSetFont (display, gc, SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_LRG)->fid);
 
  for (i=0; goalfg[i]; i++)
    {
    sprintf (str, "%-3d", goalfg[i]);
    if (gminin[i]<0) sprintf(str+strlen(str)," < %d",-gminin[i]);
    else if (gminin[i]>1) sprintf(str+strlen(str)," > %d",gminin[i]-1);
printf("goal: %s\n",str);
    XDrawImageString (display, win[0], gc, 10, 25 * i + 20, str, strlen (str));    
    XDrawImageString (display, pix[0], gc, 10, 25 * i + 20, str, strlen (str));    
    }

  for (i=0; subgfg[i]; i++)
    {
    sprintf (str, "%-3d", subgfg[i]);
    if (sminin[i]<0) sprintf(str+strlen(str)," < %d",-sminin[i]);
    else if (sminin[i]>1) sprintf(str+strlen(str)," > %d",sminin[i]-1);
printf("subgoal: %s\n",str);
    XDrawImageString (display, win[1], gc, 10, 25 * i + 20, str, strlen (str));    
    XDrawImageString (display, pix[1], gc, 10, 25 * i + 20, str, strlen (str));    
    }
}

/*  End of RXNPATT_DRAW.C  */
