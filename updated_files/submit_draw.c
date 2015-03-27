/****************************************************************************
*  
* Copyright (C) 1997 Synchem Group at SUNY-Stony Brook, Daren Krebsbach  
*  
*  
*  Module Name:               SUBMIT_DRAW.C
*  
*    Creates and manages the menu bar of the job submission tool.  
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
*    Draw_Tool_Create
*    Draw_Mode_Reset
*    Draw_Tool_Initialize
*    Pixmap_Install
*    CreateOptionMenu
*    Periodic_TblDlg_Create
*    MolDraw_Sling_Draw
*    Atom_Copy
*    DrawTool_Destroy
*
*    draw_Double_Bond
*    draw_Stereo_Bond
*    draw_Bond
*    draw_Atom
*    erase_Bond
*    draw_Molecule
*
*    New_Dsp2Xtr
*    Dsp2Tsd
*    Sling2CanonSling
*    Molecule_Double
*    store_BondNew
*    store_AtomNew
*
*    redraw
*    clear_it
*    Done_CB
*    PixMap_Display_CB
*    PixMap_Resize_CB
*    Atom_Mark_CB
*    Option_Choose_CB
*    Menu_Choice_CB
*    Periodic_TblDlg_Show_CB
*    Selected_Bond_Draw_CB
*    MOLDRAW_StereoChem_CB
*
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 29-Aug-01  Miller     Tried to make form layout more intuitive to give lesstif
*                       a break, but to no avail.  Thus, it makes no sense to
*                       make similar changes to drawpad.c and rxnpatt_draw.c!
*
*******************************************************************************/  
#include <math.h>
#include <Xm/Xm.h>
#include <X11/Xutil.h>

#include <Xm/MainW.h>
#include <Xm/DrawingA.h>
#include <Xm/DrawnB.h>
#include <Xm/PushBG.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/Separator.h>
#include <Xm/ToggleB.h> 

#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif

#include "synchem.h"
#include "debug.h"
#include "synio.h"
 

#ifndef _H_ATOMSYM_
#include "atomsym.h"
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

#ifndef _H_INFO_VIEW_
#include "info_view.h"
#endif

#define DRAW_GLOBALS
#ifndef _H_SUBMIT_DRAW_
#include "submit_draw.h"
#include "edit.h"
#endif
#undef DRAW_GLOBALS


#ifndef _H_SUBMIT_JOBINFO_
#include "submit_jobinfo.h" 
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


  /* Redefining button events for translation table */
static String  translations = 
  "<Btn1Down>:    Draw_Draw(down1)   \n\
   <Btn2Down>:    Draw_Draw(down2)   \n\
   <Btn3Down>:    Draw_Draw(down3)   \n\
   <Motion>:      Draw_Draw(motion)";


/****************************************************************************
*  
*  Function Name:                 Draw_Tool_Create
*  
*****************************************************************************/  

Widget Draw_Tool_Create 
  (
  JobInfoCB_t   *jobinfo_p,
  Widget         parent,
  XtAppContext  *appcon
  )
{
  GC             gc;
  Pixmap         pixmap;
  ScreenAttr_t  *sca_p;
  Display       *display;
  Widget         form;
  XmString       lbl_drw, lbl_sel, lbl_del;
  XmString       lbl_str; 
  Widget         separator, separator1, separator2, separator3, separator4;
  XtActionsRec   draw_actions;
  char           bitmap_file[256];

  /* Allocate necessary memory for global drawing structures.  UGH! - DK  */
  Draw_Tool_Initialize ();

  /*  Construct the Drawing tool dialog.  */
  DrawTool_Frame_Put (&GDrawToolCB, XtVaCreateWidget ("DrawToolFr",
    xmFrameWidgetClass,  parent,
    XmNshadowType,       XmSHADOW_IN,
    XmNshadowThickness,  3,
    XmNmarginWidth,      AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginHeight,     AppDim_MargFmFr_Get (&GAppDim),
    NULL));

  form = XtVaCreateWidget ("DrawToolFm", 
    xmFormWidgetClass,  DrawTool_Frame_Get (&GDrawToolCB), 
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    2 * AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     2 * AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       False,
    NULL);
  DrawTool_Form_Put (&GDrawToolCB, form);

  /*  Construct the edit options subform.  */
  DrawTool_EditForm_Put (&GDrawToolCB, XtVaCreateWidget ("DrawEditFm",
    xmFormWidgetClass,  form, 
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       False,
    NULL));

  lbl_str = XmStringCreateLocalized (SBD_MODE_LBL);  
  DrawTool_EditModeLbl_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawModeLbl",  
      xmLabelWidgetClass,  DrawTool_EditForm_Get (&GDrawToolCB),  
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,    False,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  lbl_drw = XmStringCreateLocalized (CS_DRAW_BOND); 
  lbl_sel = XmStringCreateLocalized (CS_SELECT); 
  lbl_del = XmStringCreateLocalized (CS_CLEAR); 
  DrawTool_EditModeRB_Put (&GDrawToolCB, 
     XmVaCreateSimpleRadioBox (DrawTool_EditForm_Get (&GDrawToolCB), 
       "DrawModeRB",    0,        Option_Choose_CB,
       XmVaRADIOBUTTON, lbl_drw,  NULL, NULL, NULL, 
       XmVaRADIOBUTTON, lbl_sel,  NULL, NULL, NULL, 
       XmVaRADIOBUTTON, lbl_del,  NULL, NULL, NULL, 
       NULL)); 
  XmStringFree(lbl_drw); 
  XmStringFree(lbl_sel); 
  XmStringFree(lbl_del); 
 
  lbl_str = XmStringCreateLocalized (SBD_EDIT_LBL);  
  DrawTool_EditOptLbl_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawEditLbl",  
      xmLabelWidgetClass,  DrawTool_EditForm_Get (&GDrawToolCB),  
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,    False,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLocalized (CS_SELECTALL);  
  DrawTool_EditSelAllPB_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawEditPB",  
      xmPushButtonWidgetClass, DrawTool_EditForm_Get (&GDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (DrawTool_EditSelAllPB_Get (&GDrawToolCB),
     XmNactivateCallback, Menu_Choice_CB, (XtPointer) DRW_MENU_SELECT_ALL);

  lbl_str = XmStringCreateLocalized (CS_CLEARSEL);  
  DrawTool_EditDelSelPB_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawEditPB",  
      xmPushButtonWidgetClass, DrawTool_EditForm_Get (&GDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (DrawTool_EditDelSelPB_Get (&GDrawToolCB),
     XmNactivateCallback, Menu_Choice_CB, (XtPointer) DRW_MENU_DELETE_SEL);


  XtVaSetValues (DrawTool_EditModeLbl_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (DrawTool_EditModeRB_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        DrawTool_EditModeLbl_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (DrawTool_EditOptLbl_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        DrawTool_EditModeRB_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (DrawTool_EditSelAllPB_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        DrawTool_EditOptLbl_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (DrawTool_EditDelSelPB_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        DrawTool_EditSelAllPB_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);


  /*  Construct the atom symbol subform.  */
  DrawTool_AtomForm_Put (&GDrawToolCB, XtVaCreateWidget ("DrawAtomFm",
    xmFormWidgetClass,  form, 
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    0,
    XmNmarginWidth,     0,
    XmNresizable,       False,
    NULL));

  lbl_str = XmStringCreateLocalized (SBD_ATOM_LBL);  
  DrawTool_AtomLbl_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawAtomLbl",  
      xmLabelWidgetClass,  DrawTool_AtomForm_Get (&GDrawToolCB),  
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,    False,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLocalized (CS_ELM_C);  
  DrawTool_Atm_C_PB_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawAtomPB",  
      xmPushButtonWidgetClass, DrawTool_AtomForm_Get (&GDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (DrawTool_Atm_C_PB_Get (&GDrawToolCB),
     XmNactivateCallback, Atom_Mark_CB, (XtPointer) CS_ELM_C);

  lbl_str = XmStringCreateLocalized (CS_ELM_H);  
  DrawTool_Atm_H_PB_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawAtomPB",  
      xmPushButtonWidgetClass, DrawTool_AtomForm_Get (&GDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (DrawTool_Atm_H_PB_Get (&GDrawToolCB),
     XmNactivateCallback, Atom_Mark_CB, (XtPointer) CS_ELM_H);

  lbl_str = XmStringCreateLocalized (CS_ELM_N);  
  DrawTool_Atm_N_PB_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawAtomPB",  
      xmPushButtonWidgetClass, DrawTool_AtomForm_Get (&GDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (DrawTool_Atm_N_PB_Get (&GDrawToolCB),
     XmNactivateCallback, Atom_Mark_CB, (XtPointer) CS_ELM_N);

  lbl_str = XmStringCreateLocalized (CS_ELM_O);  
  DrawTool_Atm_O_PB_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawAtomPB",  
      xmPushButtonWidgetClass, DrawTool_AtomForm_Get (&GDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (DrawTool_Atm_O_PB_Get (&GDrawToolCB),
     XmNactivateCallback, Atom_Mark_CB, (XtPointer) CS_ELM_O);

  lbl_str = XmStringCreateLocalized (CS_ELM_S);  
  DrawTool_Atm_S_PB_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawAtomPB",  
      xmPushButtonWidgetClass, DrawTool_AtomForm_Get (&GDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (DrawTool_Atm_S_PB_Get (&GDrawToolCB),
     XmNactivateCallback, Atom_Mark_CB, (XtPointer) CS_ELM_S);

  lbl_str = XmStringCreateLocalized (CS_ELM_P);  
  DrawTool_Atm_P_PB_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawAtomPB",  
      xmPushButtonWidgetClass, DrawTool_AtomForm_Get (&GDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (DrawTool_Atm_P_PB_Get (&GDrawToolCB),
     XmNactivateCallback, Atom_Mark_CB, (XtPointer) CS_ELM_P);

  lbl_str = XmStringCreateLocalized (CS_ELM_Cl);  
  DrawTool_Atm_Cl_PB_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawAtomPB",  
      xmPushButtonWidgetClass, DrawTool_AtomForm_Get (&GDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (DrawTool_Atm_Cl_PB_Get (&GDrawToolCB),
     XmNactivateCallback, Atom_Mark_CB, (XtPointer) CS_ELM_Cl);

  lbl_str = XmStringCreateLocalized (CS_ELM_F);  
  DrawTool_Atm_F_PB_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawAtomPB",  
      xmPushButtonWidgetClass, DrawTool_AtomForm_Get (&GDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (DrawTool_Atm_F_PB_Get (&GDrawToolCB),
     XmNactivateCallback, Atom_Mark_CB, (XtPointer) CS_ELM_F);

  lbl_str = XmStringCreateLocalized (CS_ELM_Br);  
  DrawTool_Atm_Br_PB_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawAtomPB",  
      xmPushButtonWidgetClass, DrawTool_AtomForm_Get (&GDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (DrawTool_Atm_Br_PB_Get (&GDrawToolCB),
     XmNactivateCallback, Atom_Mark_CB, (XtPointer) CS_ELM_Br);

  lbl_str = XmStringCreateLocalized (CS_ELM_I);  
  DrawTool_Atm_I_PB_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawAtomPB",  
      xmPushButtonWidgetClass, DrawTool_AtomForm_Get (&GDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (DrawTool_Atm_I_PB_Get (&GDrawToolCB),
     XmNactivateCallback, Atom_Mark_CB, (XtPointer) CS_ELM_I);

  lbl_str = XmStringCreateLocalized (CS_ELM_OTHER);  
  DrawTool_Atm_Other_PB_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawAtomPB",  
      xmPushButtonWidgetClass, DrawTool_AtomForm_Get (&GDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (DrawTool_Atm_Other_PB_Get (&GDrawToolCB),
     XmNactivateCallback, Periodic_TblDlg_Show_CB, (XtPointer) NULL);

  XtVaSetValues (DrawTool_AtomLbl_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (DrawTool_Atm_C_PB_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        DrawTool_AtomLbl_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     0,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    50,
    NULL);

  XtVaSetValues (DrawTool_Atm_H_PB_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        DrawTool_Atm_C_PB_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     DrawTool_Atm_C_PB_Get (&GDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     50,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    100,
    NULL);

  XtVaSetValues (DrawTool_Atm_N_PB_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        DrawTool_Atm_C_PB_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     0,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    50,
    NULL);

  XtVaSetValues (DrawTool_Atm_O_PB_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        DrawTool_Atm_N_PB_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     DrawTool_Atm_N_PB_Get (&GDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     50,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    100,
    NULL);

  XtVaSetValues (DrawTool_Atm_S_PB_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        DrawTool_Atm_N_PB_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     0,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    50,
    NULL);

  XtVaSetValues (DrawTool_Atm_P_PB_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        DrawTool_Atm_S_PB_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     DrawTool_Atm_S_PB_Get (&GDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     50,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    100,
    NULL);

  XtVaSetValues (DrawTool_Atm_Cl_PB_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        DrawTool_Atm_S_PB_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     0,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    50,
    NULL);

  XtVaSetValues (DrawTool_Atm_F_PB_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        DrawTool_Atm_Cl_PB_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     DrawTool_Atm_Cl_PB_Get (&GDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     50,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    100,
    NULL);

  XtVaSetValues (DrawTool_Atm_Br_PB_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        DrawTool_Atm_Cl_PB_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     0,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    50,
    NULL);

  XtVaSetValues (DrawTool_Atm_I_PB_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        DrawTool_Atm_Br_PB_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     DrawTool_Atm_Br_PB_Get (&GDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     50,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    100,
    NULL);

  XtVaSetValues (DrawTool_Atm_Other_PB_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        DrawTool_Atm_Br_PB_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNleftPosition,     0,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    XmNrightPosition,    50,
    NULL);

  /*  Construct the bond symbol subform.  */
  DrawTool_BondForm_Put (&GDrawToolCB, XtVaCreateWidget ("DrawBondFm",
    xmFormWidgetClass,  form, 
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       False,
    NULL));

  lbl_str = XmStringCreateLocalized (SBD_BOND_LBL);  
  DrawTool_BndLftLbl_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawBondLbl",  
      xmLabelWidgetClass,  DrawTool_BondForm_Get (&GDrawToolCB),  
      XmNlabelType,        XmSTRING,  
      XmNlabelString,      lbl_str,   
      XmNalignment,        XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,    False,
      XmNmarginHeight,     AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  sprintf (bitmap_file, "%s/%s", SAR_DIR_BITMAPS (""), CS_SNGL_BOND);
  pixmap = XmGetPixmap (Screen_TScreen_Get (SynAppR_ScrnAtrb_Get (&GSynAppR)), 
    bitmap_file, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK), 
    SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
  DrawTool_BndLSglPB_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawBondPB",  
      xmPushButtonWidgetClass, DrawTool_BondForm_Get (&GDrawToolCB), 
      XmNlabelType,            XmPIXMAP,  
      XmNlabelPixmap,          pixmap,
      NULL));  
  XtAddCallback (DrawTool_BndLSglPB_Get (&GDrawToolCB),
     XmNactivateCallback, Selected_Bond_Draw_CB, (XtPointer) DSP_BOND_SINGLE);
  /*
  XmDestroyPixmap (Screen_TScreen_Get (SynAppR_ScrnAtrb_Get (&GSynAppR)), 
    pixmap);
   */
  sprintf (bitmap_file, "%s/%s", SAR_DIR_BITMAPS (""), CS_DUBL_BOND);
  pixmap = XmGetPixmap (Screen_TScreen_Get (SynAppR_ScrnAtrb_Get (&GSynAppR)), 
    bitmap_file, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK), 
    SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
  DrawTool_BndLDblPB_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawBondPB",  
      xmPushButtonWidgetClass, DrawTool_BondForm_Get (&GDrawToolCB), 
      XmNlabelType,            XmPIXMAP,  
      XmNlabelPixmap,          pixmap,
      NULL));  
  XtAddCallback (DrawTool_BndLDblPB_Get (&GDrawToolCB),
     XmNactivateCallback, Selected_Bond_Draw_CB, (XtPointer) DSP_BOND_DOUBLE);
  /*
  XmDestroyPixmap (Screen_TScreen_Get (SynAppR_ScrnAtrb_Get (&GSynAppR)), 
    pixmap);
   */
 
  sprintf (bitmap_file, "%s/%s", SAR_DIR_BITMAPS (""), CS_TRPL_BOND);
  pixmap = XmGetPixmap (Screen_TScreen_Get (SynAppR_ScrnAtrb_Get (&GSynAppR)), 
    bitmap_file, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK), 
    SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
  DrawTool_BndLTplPB_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawBondPB",  
      xmPushButtonWidgetClass, DrawTool_BondForm_Get (&GDrawToolCB), 
      XmNlabelType,            XmPIXMAP,  
      XmNlabelPixmap,          pixmap,
      NULL));  
  XtAddCallback (DrawTool_BndLTplPB_Get (&GDrawToolCB),
     XmNactivateCallback, Selected_Bond_Draw_CB, (XtPointer) DSP_BOND_TRIPLE);
  /*
  XmDestroyPixmap (Screen_TScreen_Get (SynAppR_ScrnAtrb_Get (&GSynAppR)), 
    pixmap);
   */

  XtVaSetValues (DrawTool_BndLftLbl_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (DrawTool_BndLSglPB_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        DrawTool_BndLftLbl_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     1,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    33,
    NULL);

  XtVaSetValues (DrawTool_BndLDblPB_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        DrawTool_BndLSglPB_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     DrawTool_BndLSglPB_Get (&GDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     34,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    66,
    NULL);

  XtVaSetValues (DrawTool_BndLTplPB_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        DrawTool_BndLSglPB_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     DrawTool_BndLSglPB_Get (&GDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     67,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    99,
    NULL);

  separator1 = XtVaCreateManagedWidget("DrawToolSep",  
    xmSeparatorWidgetClass, form, 
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmSHADOW_ETCHED_IN,
    XmNheight,              AppDim_SepSmall_Get (&GAppDim),
    NULL);

  separator2 = XtVaCreateManagedWidget("DrawToolSep",  
    xmSeparatorWidgetClass, form, 
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmSHADOW_ETCHED_IN,
    XmNheight,              AppDim_SepSmall_Get (&GAppDim),
    NULL);

  separator3 = XtVaCreateManagedWidget("DrawToolSep",  
    xmSeparatorWidgetClass, form, 
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmSHADOW_ETCHED_IN,
    XmNheight,              AppDim_SepSmall_Get (&GAppDim),
    NULL);

  separator4 = XtVaCreateManagedWidget("DrawToolSep",  
    xmSeparatorWidgetClass, form, 
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmSHADOW_ETCHED_IN,
    XmNheight,              AppDim_SepSmall_Get (&GAppDim),
    NULL);

  separator = XtVaCreateManagedWidget("DrawToolSep",  
    xmSeparatorWidgetClass, form, 
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmDOUBLE_LINE,
    XmNheight,              AppDim_SepLarge_Get (&GAppDim),
    NULL);

  DrawTool_PBForm_Put (&GDrawToolCB, XtVaCreateWidget ("DrawEditFm",
    xmFormWidgetClass,  form, 
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       False,
    NULL));

  lbl_str = XmStringCreateLocalized (CS_DONE);  
  DrawTool_DonePB_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawToolPB",  
      xmPushButtonWidgetClass, DrawTool_PBForm_Get (&GDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      XmNheight,               AppDim_HtLblPB_Get (&GAppDim),
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (DrawTool_DonePB_Get (&GDrawToolCB),
     XmNactivateCallback, Done_CB, (XtPointer) jobinfo_p);

  lbl_str = XmStringCreateLocalized (CS_REDRAW);  
  DrawTool_RedrawPB_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawToolPB",  
      xmPushButtonWidgetClass, DrawTool_PBForm_Get (&GDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (DrawTool_RedrawPB_Get (&GDrawToolCB),
     XmNactivateCallback, Option_Choose_CB, (XtPointer) DRW_REDRAW_FLAG);

  lbl_str = XmStringCreateLocalized (CS_CLEARALL);  
  DrawTool_ClearPB_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawToolPB",  
      xmPushButtonWidgetClass, DrawTool_PBForm_Get (&GDrawToolCB), 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,   
      NULL));  
  XmStringFree (lbl_str);
  XtAddCallback (DrawTool_ClearPB_Get (&GDrawToolCB),
     XmNactivateCallback, Menu_Choice_CB, (XtPointer) DRW_MENU_DELETE_ALL);


  XtVaSetValues (DrawTool_DonePB_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     15,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    29,
    NULL);

  XtVaSetValues (DrawTool_RedrawPB_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        DrawTool_DonePB_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     DrawTool_DonePB_Get (&GDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     43,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    57,
    NULL);

  XtVaSetValues (DrawTool_ClearPB_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        DrawTool_DonePB_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     DrawTool_DonePB_Get (&GDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     71,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    85,
    NULL);

  DrawTool_ScrlldWin_Put (&GDrawToolCB, 
    XtVaCreateManagedWidget ("DrawToolSW",
      xmScrolledWindowWidgetClass, form,
      XmNscrollingPolicy,          XmAUTOMATIC,
      XmNscrollBarDisplayPolicy,   XmAS_NEEDED,
      NULL));

  /*  Add the "draw" action/function used by the translation table
   *  parsed by the translations resource below.
   */
  draw_actions.string = "Draw_Draw";
  draw_actions.proc = (XtActionProc) Draw_Draw;
  XtAppAddActions (*appcon, &draw_actions, 1);

  DrawTool_DrawArea_Put (&GDrawToolCB, XtVaCreateManagedWidget ("DrawToolDA",
    xmDrawingAreaWidgetClass, DrawTool_ScrlldWin_Get (&GDrawToolCB),
    XmNtranslations,      XtParseTranslationTable (translations),
    XmNbackground,        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground,        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNheight,            DRW_DA_PXMP_HT,
    XmNwidth,             DRW_DA_PXMP_WD,
    NULL));
  XtAddCallback (DrawTool_DrawArea_Get (&GDrawToolCB), XmNexposeCallback, 
    redraw, NULL);

/* It's no wonder the layout algorithm for lesstif chokes on this convoluted layout!
   Let's try to make a more intuitive set of values that starts with specifics!!! */

  XtVaSetValues (separator1,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
/*
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget,     DrawTool_EditForm_Get (&GDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       DrawTool_EditForm_Get (&GDrawToolCB),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
    XmNrightWidget,      DrawTool_EditForm_Get (&GDrawToolCB),
*/
XmNbottomOffset,     -(AppDim_SepLarge_Get (&GAppDim)),
XmNbottomAttachment, XmATTACH_OPPOSITE_FORM,
XmNleftOffset,       0,
XmNleftAttachment,   XmATTACH_FORM,
XmNrightOffset,      -(AppDim_SbtPanel_Wd_Get (&GAppDim)),
XmNrightAttachment,  XmATTACH_OPPOSITE_FORM,
    NULL);

/* Now that we have some SPECIFICS for the FIRST widget, we can make some SENSIBLE
   relative attachments!  We can now refer only to widgets whose positional values
   have already been established, instead of playing games with Motif!  (However,
   since lesstif still balks, it's also obvious that 200 is too stingy.) */

  XtVaSetValues (DrawTool_EditForm_Get (&GDrawToolCB),
    XmNtopOffset,        0,
/*
    XmNtopAttachment,    XmATTACH_NONE,
*/
XmNtopAttachment,    XmATTACH_WIDGET,
XmNtopWidget,        separator1,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
/*
    XmNrightOffset,      -(AppDim_SbtPanel_Wd_Get (&GAppDim)),
    XmNrightAttachment,  XmATTACH_OPPOSITE_FORM,
*/
XmNrightOffset,      0,
XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
XmNrightWidget,      separator1,
    NULL);

  XtVaSetValues (separator2,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        DrawTool_EditForm_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       DrawTool_EditForm_Get (&GDrawToolCB),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
    XmNrightWidget,      DrawTool_EditForm_Get (&GDrawToolCB),
    NULL);

  XtVaSetValues (DrawTool_AtomForm_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        separator2,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       DrawTool_EditForm_Get (&GDrawToolCB),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
    XmNrightWidget,      DrawTool_EditForm_Get (&GDrawToolCB),
    NULL);

  XtVaSetValues (separator3,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        DrawTool_AtomForm_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       DrawTool_EditForm_Get (&GDrawToolCB),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
    XmNrightWidget,      DrawTool_EditForm_Get (&GDrawToolCB),
    NULL);

  XtVaSetValues (DrawTool_BondForm_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        separator3,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       DrawTool_EditForm_Get (&GDrawToolCB),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
    XmNrightWidget,      DrawTool_EditForm_Get (&GDrawToolCB),
    NULL);

  XtVaSetValues (separator4,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        DrawTool_BondForm_Get (&GDrawToolCB),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       DrawTool_EditForm_Get (&GDrawToolCB),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
    XmNrightWidget,      DrawTool_EditForm_Get (&GDrawToolCB),
    NULL);

  XtVaSetValues (DrawTool_PBForm_Get (&GDrawToolCB),
    XmNtopOffset,        -(AppDim_HtLblPB_Get (&GAppDim) 
                           + 2 * AppDim_MargFmFr_Get (&GAppDim)),
    XmNtopAttachment,    XmATTACH_OPPOSITE_FORM,
    XmNbottomOffset,     AppDim_MargFmFr_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       DrawTool_BondForm_Get (&GDrawToolCB),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (separator,
    XmNtopOffset,        -(AppDim_HtLblPB_Get (&GAppDim) 
                           + 3 * AppDim_MargFmFr_Get (&GAppDim)
                           + AppDim_SepLarge_Get (&GAppDim)),
    XmNtopAttachment,    XmATTACH_OPPOSITE_FORM,
    XmNbottomOffset,     AppDim_MargFmFr_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget,     DrawTool_PBForm_Get (&GDrawToolCB),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       DrawTool_BondForm_Get (&GDrawToolCB),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);


  XtVaSetValues (DrawTool_ScrlldWin_Get (&GDrawToolCB),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget,     separator,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       DrawTool_EditForm_Get (&GDrawToolCB),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  /* Manage Unmanaged children */    
  XtManageChild (DrawTool_EditModeRB_Get (&GDrawToolCB));
  XtManageChild (DrawTool_EditForm_Get (&GDrawToolCB));
  XtManageChild (DrawTool_AtomForm_Get (&GDrawToolCB));
  XtManageChild (DrawTool_BondForm_Get (&GDrawToolCB));
  XtManageChild (DrawTool_PBForm_Get (&GDrawToolCB));
  XtManageChild (form);

  /* Create the periodic table dialog */ 
  Periodic_TblDlg_Create (parent);

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
  DrawTool_GC_Put (&GDrawToolCB, gc);

  DrawTool_Pixmap_Put (&GDrawToolCB, XCreatePixmap (display,
    Screen_RootWin_Get (sca_p), DRW_DA_PXMP_WD, DRW_DA_PXMP_HT, 
    Screen_Depth_Get (sca_p)));

  XFillRectangle (display, DrawTool_Pixmap_Get (&GDrawToolCB),
    gc, 0, 0, DRW_DA_PXMP_WD, DRW_DA_PXMP_HT);
  XSetForeground (display, gc, SynAppR_IthClrPx_Get (&GSynAppR, 
    SAR_CLRI_BLACK));
 

  return (DrawTool_Frame_Get (&GDrawToolCB));
}
/*  End of Draw_Tool_Create  */


/****************************************************************************
*  
*  Function Name:                 Draw_Mode_Reset
*  
*****************************************************************************/  

void Draw_Mode_Reset
  (
  int            which_mode
  )
{
  WidgetList     toggles;

  /** Get the run type distributed or sequential **/
  XtVaGetValues (DrawTool_EditModeRB_Get (&GDrawToolCB),
    XmNchildren,    &toggles,
    NULL);

  XmToggleButtonSetState (toggles[which_mode], True, True);
   
  return;

}
/*  End of Draw_Mode_Reset  */


/****************************************************************************
*  
*  Function Name:                 Periodic_TblDlg_Create
*  
*****************************************************************************/  

void Periodic_TblDlg_Create 
  (
  Widget      parent
  )
{
  Widget    frame, separator, pb;
  Widget    H_elm_pb, He_elm_pb, Ac_elm_pb;
  Widget    Li_Ra_rc, B_Ar_rc, Sc_Rn_rc, Ce_Lw_rc;
  XmString  str;
  int       i;
 
  DrawTool_PeriodicTbl_Put (&GDrawToolCB, XmCreateFormDialog 
    (parent, "DrawPTblFmDg", NULL, 0));

  str = XmStringCreateLocalized ("Periodic Table");
    XtVaSetValues (DrawTool_PeriodicTbl_Get (&GDrawToolCB),
      XmNdialogTitle,  str,
      XmNdialogStyle,  XmDIALOG_MODELESS,
      XmNautoUnmanage, False,
      NULL);
  XmStringFree (str);

  frame = XtVaCreateManagedWidget ("DrawPTblFr", 
    xmFrameWidgetClass, DrawTool_PeriodicTbl_Get (&GDrawToolCB),
    XmNtopAttachment, XmATTACH_FORM,
    XmNtopOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    NULL);

  str = XmStringCreateLocalized (CS_ELM_H);
  H_elm_pb =  XtVaCreateManagedWidget ("DrawPTblPB", 
    xmPushButtonGadgetClass, 
    DrawTool_PeriodicTbl_Get (&GDrawToolCB),
    XmNlabelString, str,
    XmNtopAttachment, XmATTACH_FORM,
    XmNtopOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    NULL);
  XmStringFree (str);
  XtAddCallback (H_elm_pb, XmNactivateCallback, Atom_Mark_CB, 
    (XtPointer) CS_ELM_H);

  str = XmStringCreateLocalized (CS_ELM_He);
  He_elm_pb =  XtVaCreateManagedWidget ("DrawPTblPB", xmPushButtonGadgetClass, 
    DrawTool_PeriodicTbl_Get (&GDrawToolCB),
    XmNlabelString, str,
    XmNtopAttachment, XmATTACH_FORM,
    XmNtopOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    XmNrightOffset, 0,
    NULL);
  XmStringFree (str);
  XtAddCallback (He_elm_pb, XmNactivateCallback, Atom_Mark_CB, 
    (XtPointer) CS_ELM_He);

  Li_Ra_rc =  XtVaCreateManagedWidget ("DrawPTblRC", xmRowColumnWidgetClass, 
    DrawTool_PeriodicTbl_Get (&GDrawToolCB),
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
  for(i = 0; i < XtNumber(Li_Ra_Elms); i++)
    {
   
    pb = XtVaCreateManagedWidget (Li_Ra_Elms[i], xmPushButtonGadgetClass,     
      Li_Ra_rc, NULL);

    /** callback for an element button from Li to Ra **/  
    XtAddCallback (pb, XmNactivateCallback, Atom_Mark_CB, Li_Ra_Elms[i]);
    }

  B_Ar_rc =  XtVaCreateManagedWidget ("DrawPTblRC", xmRowColumnWidgetClass, 
    DrawTool_PeriodicTbl_Get (&GDrawToolCB),
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
  for(i = 0; i < XtNumber(B_Ar_Elms); i++)
    {
   
    pb = XtVaCreateManagedWidget (B_Ar_Elms[i], xmPushButtonGadgetClass,     
      B_Ar_rc, NULL);

    /** callback for an element button from B to Ar **/  
    XtAddCallback (pb, XmNactivateCallback, Atom_Mark_CB, B_Ar_Elms[i]);
    }

  Sc_Rn_rc =  XtVaCreateManagedWidget ("DrawPTblRC", xmRowColumnWidgetClass, 
    DrawTool_PeriodicTbl_Get (&GDrawToolCB),
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
  for(i = 0; i < XtNumber(Sc_Rn_Elms); i++)
    {
   
    pb = XtVaCreateManagedWidget (Sc_Rn_Elms[i], xmPushButtonGadgetClass,     
      Sc_Rn_rc, NULL);

    /** callback for an element button from Sc to Rn **/  
    XtAddCallback (pb, XmNactivateCallback, Atom_Mark_CB, Sc_Rn_Elms[i]);
    }

  Ce_Lw_rc =  XtVaCreateManagedWidget ("DrawPTblRC", xmRowColumnWidgetClass, 
    DrawTool_PeriodicTbl_Get (&GDrawToolCB),
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
  for(i = 0; i < XtNumber(Ce_Lw_Elms); i++)
    {
   
    pb = XtVaCreateManagedWidget (Ce_Lw_Elms[i], xmPushButtonGadgetClass,     
      Ce_Lw_rc, NULL);

    /** callback for an element button from Ce to Lw **/  
    XtAddCallback (pb, XmNactivateCallback, Atom_Mark_CB, Ce_Lw_Elms[i]);
    }

  separator = XtVaCreateManagedWidget("DrawPTblSep",
    xmSeparatorWidgetClass, DrawTool_PeriodicTbl_Get (&GDrawToolCB),
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

  str = XmStringCreateLocalized (CS_MULTIPLE);
  pb = XtVaCreateManagedWidget ("DrawPTblPB",
    xmPushButtonWidgetClass, DrawTool_PeriodicTbl_Get (&GDrawToolCB), 
    XmNlabelString, str,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopOffset, 5,
    XmNtopWidget, separator,
    XmNleftAttachment, XmATTACH_FORM,
    XmNleftOffset, 100,
    NULL);
  XmStringFree (str);
  XtAddCallback (pb, XmNactivateCallback, Atom_Mark_CB, CS_MULTIPLE);

  str = XmStringCreateLocalized (CS_DISMISS);
  pb = XtVaCreateManagedWidget ("DrawPTblPB", xmPushButtonWidgetClass, 
    DrawTool_PeriodicTbl_Get (&GDrawToolCB),
    XmNlabelString, str,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopOffset, 5,
    XmNtopWidget, separator,
    XmNrightAttachment, XmATTACH_FORM,
    XmNrightOffset, 100,
    NULL);
  XmStringFree (str);
  XtAddCallback (pb, XmNactivateCallback, Atom_Mark_CB, CS_DISMISS);
  
  str = XmStringCreateLocalized (CS_ELM_Ac);
  Ac_elm_pb =  XtVaCreateManagedWidget ("Ac_elm_pb", xmPushButtonGadgetClass, 
    DrawTool_PeriodicTbl_Get (&GDrawToolCB),
    XmNlabelString, str,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopOffset, 0,
    XmNtopWidget, Sc_Rn_rc,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftOffset, 0,
    XmNleftWidget, Li_Ra_rc,
    NULL);
  XtAddCallback (Ac_elm_pb, XmNactivateCallback, Atom_Mark_CB, CS_ELM_Ac);

  return;
}
/*  End of Periodic_TblDlg_Create  */


/****************************************************************************
*  
*  Function Name:                 PixMap_Display_CB
*  
*****************************************************************************/  

void PixMap_Display_CB
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
     DrawTool_GC_Get (&GDrawToolCB), srcx, srcy, pix_w, pix_h, destx, desty);
  
   return;

}
/*  End of PixMap_Display_CB  */

/****************************************************************************
*  
*  Function Name:                 PixMap_Resize_CB
*  
*****************************************************************************/  

void PixMap_Resize_CB
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
/*  End of PixMap_Resize_CB  */

/****************************************************************************
*  
*  Function Name:                 CreateOptionMenu
*  
*****************************************************************************/  

void CreateOptionMenu 
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
  XtAddCallback (button1, XmNactivateCallback, Menu_Choice_CB, 
    (XtPointer) buttn1_tag);
  button2 = XtCreateManagedWidget (buttn2_name, xmPushButtonWidgetClass,
    pane, NULL, 0);
  XtAddCallback (button2, XmNactivateCallback, Menu_Choice_CB, 
    (XtPointer) buttn2_tag);

  return;
} 
/*  End of CreateOptionMenu  */

/****************************************************************************
*  
*  Function Name:                 clear_it
*  
*****************************************************************************/  

void clear_it
  (
  Widget     pb,
  XtPointer  client_data,
  XtPointer  call_data
  )
{
  Widget drawing_a = (Widget) client_data;
  XmPushButtonCallbackStruct *cbs = (XmPushButtonCallbackStruct *) call_data;

  /*  Clear pixmap with white */
  XSetForeground (XtDisplay (drawing_a), DrawTool_GC_Get (&GDrawToolCB), 
    SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));  

  /* This clears the the pixmap */
  XFillRectangle (XtDisplay (drawing_a), DrawTool_Pixmap_Get (&GDrawToolCB), 
    DrawTool_GC_Get (&GDrawToolCB), 0, 0, DRW_DA_PXMP_WD, DRW_DA_PXMP_HT);

  /*  Drawing is now done using black, chage the gc */
  XSetForeground (XtDisplay (drawing_a), DrawTool_GC_Get (&GDrawToolCB), 
    SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));

  /*  render the newly cleared pixmap onto the window */
  XCopyArea (cbs->event->xbutton.display, DrawTool_Pixmap_Get (&GDrawToolCB), 
    XtWindow (drawing_a), DrawTool_GC_Get (&GDrawToolCB), 0, 0, DRW_DA_PXMP_WD, 
    DRW_DA_PXMP_HT, 0, 0);

  Draw_Mode_Reset (DRW_BOND_FLAG);

}
/*  End of clear_it  */

/****************************************************************************
*  
*  Function Name:                 redraw
*  
*****************************************************************************/  

void redraw
  (
  Widget      drawing_a,
  XtPointer   client_data,
  XtPointer   call_data
  )

{
  XmDrawingAreaCallbackStruct *cbs =
    (XmDrawingAreaCallbackStruct *) call_data;
  
  XCopyArea (cbs->event->xexpose.display, DrawTool_Pixmap_Get (&GDrawToolCB), 
    XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), 
    DrawTool_GC_Get (&GDrawToolCB), 0, 0, DRW_DA_PXMP_WD, 
    DRW_DA_PXMP_HT, 0, 0);

  Draw_Mode_Reset (DRW_BOND_FLAG);

  return;
}
/*  End of redraw  */

/****************************************************************************
*  
*  Function Name:                 Draw_Tool_Initialize
*  
*****************************************************************************/  

void Draw_Tool_Initialize  
  (
  void
  )
{
  ScreenAttr_t  *sca_p;
         
  /* Allocate memory for molecule data structures */
  DrawMol_p = (Dsp_Molecule_t *) malloc (DSP_MOLECULE_SIZE);
  
  DrawBondsInfo = (Drw_bonds_info_t *) malloc (DRW_BONDS_INFO_SIZE);;
  DrawSelInfo = (Drw_select_info_t *) malloc (DRW_SELECT_INFO_SIZE);
  
  /* Allocate memory for first 128 atoms and bonds */
  DrawMol_p->atoms = (Dsp_Atom_t *) malloc (DSP_ATOM_SIZE * DRW_MEM_UNIT);
  DrawMol_p->bonds = (Dsp_Bond_t *) malloc (DSP_BOND_SIZE * DRW_MEM_UNIT);

  DrawMol_p->nallocatms = DRW_MEM_UNIT;
  DrawMol_p->nallocbnds = DRW_MEM_UNIT;
  DrawMol_p->natms = 0;
  DrawMol_p->nbnds = 0;
  DrawMol_p->rxncnr_p = (Dsp_Atom_t *) NULL;
  DrawMol_p->scale = AppDim_MolScale_Get (&GAppDim);

  DrawTool_Molecule_Put (&GDrawToolCB, DrawMol_p); 
  
  /* Initialize mode of drawing */
  DrawFlags.drawing_mode = DRW_BOND_FLAG;
  DrawFlags.retrace_mode = TRUE;
  DrawFlags.switch_by_click = TRUE;
  DrawFlags.dflt_bond = DSP_BOND_DOUBLE;

  DrawBondsInfo->in_atom_area = FALSE;
  DrawBondsInfo->in_bond_area = FALSE;
  DrawBondsInfo->cur_bnd_p = (Dsp_Bond_t *) NULL;
  DrawBondsInfo->esc_mode = TRUE;

  sca_p = SynAppR_ScrnAtrb_Get (&GSynAppR);
  DrawDisplay = Screen_TDisplay_Get (sca_p);
  DrawScreen = Screen_TScreen_Get (sca_p);

  return ;
}
/*  End of Draw_Tool_Initialize  */

/****************************************************************************
*  
*  Function Name:                 Periodic_TblDlg_Show_CB
*  
*****************************************************************************/  

void Periodic_TblDlg_Show_CB
  ( 
  Widget    w, 
  XtPointer client_data,
  XtPointer call_data
  )
{
  if ( w != NULL)
    XtManageChild (DrawTool_PeriodicTbl_Get (&GDrawToolCB));
  else
    printf ("ERROR:  Unable to display the periodic table dialog");

  return;
} 
/*  End of Periodic_TblDlg_Show_CB  */

/****************************************************************************
*  
*  Function Name:                 Draw_Draw
*  
*****************************************************************************/  

void Draw_Draw 
  (
  Widget           wid,
  XButtonEvent    *event,
  String          *args,
  int             *num_args
)
{
  Dsp_Atom_t      *dratm_p, *atom_p;
  Dsp_Bond_t      *drbnd_p;
  GC               gc;
  int              adj_i, adj_j, jj;

  gc = DrawTool_GC_Get (&GDrawToolCB);

  if (*num_args != 1) 
    {
    fprintf (stderr, "Draw_Draw : Wrong number of args\n");
    exit (-1);
    }

  if (XtWindow (wid) != XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)))
    return;

  switch (DrawFlags.drawing_mode) 
    {
    case DRW_BOND_FLAG:
      XSetForeground (DrawDisplay, gc, 
        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
      if (strcmp (args[0], "down1") == 0) 
        {  
        /* If "escape mode" is set, the current point is the
	   beginning of a bond.
         */
        if (DrawBondsInfo->esc_mode)
          {
	  DrawBondsInfo->esc_mode = FALSE;

	  /* Check whether there is already an atom at this point.
	     If there is, just store its coords as the point the bond 
	     starts from.
	     If not, put it into Mol data structure.
	   */
          dratm_p = get_Atom_There (DrawMol_p, event->x, event->y);
	  if (dratm_p != NULL) 
            {
	    DrawBondsInfo->clicked_x = dratm_p->x;
	    DrawBondsInfo->clicked_y = dratm_p->y;
	    DrawBondsInfo->new_atom = FALSE;
	    }
	  else 
            {
	    store_AtomNew (DrawMol_p, "C", "\0", "\0", event->x, event->y);

	    /* Store the point the bond starts from */
	    DrawBondsInfo->clicked_x = event->x;
	    DrawBondsInfo->clicked_y = event->y;
	    DrawBondsInfo->new_atom = TRUE;
	    }
          } /* End of if in "escape mode" */

        else
          {
          /* Copy the Pixmap in order to erase the previous line
             and draw a line from (xy) stored to current (xy) in both
             Drawing Area and in Pixmap
          */
	  XCopyArea (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
            XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), 
            gc, 0, 0, DRW_DA_PXMP_WD, DRW_DA_PXMP_HT, 0, 0);

          /* Check whether the bond specified by drawing already exists. */
          drbnd_p = get_Bond_There (DrawMol_p, event->x, event->y,
            DrawBondsInfo->clicked_x, DrawBondsInfo->clicked_y);
          if (drbnd_p != NULL) 
            {
            dratm_p = get_Atom_There (DrawMol_p, event->x, event->y);
	    /* If we are in bond++ mode, increment its num of lines */
	    if (!DrawFlags.retrace_mode) 
              {
              /* Erase bond on both Drawing Area and Pixmap. */
	      draw_Bond (DrawDisplay, 
                XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), gc,
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), DrawMol_p, 
                drbnd_p, FALSE, FALSE);
	      draw_Bond (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
                gc, SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), 
                DrawMol_p, drbnd_p, FALSE, FALSE);

	      if (drbnd_p->nlines < DSP_BOND_TRIPLE)
	        ++drbnd_p->nlines;
	      else
	        drbnd_p->nlines = DSP_BOND_SINGLE;

	      draw_Bond (DrawDisplay, 
                XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), gc, 
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), DrawMol_p, 
                drbnd_p, TRUE, TRUE);
	      draw_Bond (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), gc,
                SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), 
                DrawMol_p, drbnd_p, TRUE, TRUE);
	      }

	    /* Store the point the bond ends at */
	    DrawBondsInfo->clicked_x = dratm_p->x;
	    DrawBondsInfo->clicked_y = dratm_p->y;

	    /* Store this bond as the current bond */
	    DrawBondsInfo->cur_bnd_p = drbnd_p;
            } /* End if get_Bond_There */

          else 
            {
	    /* If bond starts from and ends at the same point and
	       the atom it starts from is new, discard this drawings.
	     */
	    if ((dratm_p = get_Atom_There (DrawMol_p, event->x, event->y)) 
	        && (dratm_p->x == DrawBondsInfo->clicked_x) 
	        && (dratm_p->y == DrawBondsInfo->clicked_y) 
	        && (DrawBondsInfo->new_atom)) 
              {
	      return;
	      }

	    /* Check whether there is an atom at the end point of the bond 
	       If yes, simply draw a line and store the coords of this atom
	       as the end coords of the bond. 
	       Otherwise, alloc memory for a new atom, draw a line and make
	       appropriate storage.
            */
	    if (dratm_p != NULL) 
              {
              drbnd_p = store_BondNew (DrawMol_p, DrawBondsInfo->clicked_x, 
                DrawBondsInfo->clicked_y, dratm_p->x, dratm_p->y,
                DSP_BOND_SINGLE);
	      if (drbnd_p != NULL) 
                {
	        draw_Bond (DrawDisplay, 
                  XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), gc, 
                  SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), DrawMol_p, 
                  drbnd_p, TRUE, TRUE);
	        draw_Bond (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
                  gc, SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), 
                  DrawMol_p, drbnd_p, TRUE, TRUE);

	        /* Store the point the bond ends at */
	        DrawBondsInfo->clicked_x = dratm_p->x;
	        DrawBondsInfo->clicked_y = dratm_p->y;

	        /* Store this bond as the current bond */
	        DrawBondsInfo->cur_bnd_p = drbnd_p;
	        }

	      else  /* store_BondNew retured NULL - too many bonds attached */
                {   
	        /* Manage appropriate Error Dialog and discard this bond */
	        DrawBondsInfo->esc_mode = TRUE;
	        DrawBondsInfo->cur_bnd_p = (Dsp_Bond_t *) NULL;
	       InfoWarn_Show ("many_bonds_errdg"); 
	        }
	      } /* End of if atom is at the end point */

	    else 
             {
	      dratm_p = store_AtomNew (DrawMol_p, "C", "\0", "\0", event->x, 
                event->y);
              drbnd_p = store_BondNew (DrawMol_p, DrawBondsInfo->clicked_x, 
                DrawBondsInfo->clicked_y, event->x, event->y,
                DSP_BOND_SINGLE);
	      if (drbnd_p != NULL) 
                {
	        draw_Bond (DrawDisplay, 
                  XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), gc,
                  SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), DrawMol_p,
                  drbnd_p, TRUE, TRUE);
	        draw_Bond (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
                  gc, SynAppR_IthColor_Get (
                  &GSynAppR, SAR_CLRI_BLACK), DrawMol_p, drbnd_p, TRUE, TRUE);

	        /* Store the point the bond ends at */
	        DrawBondsInfo->clicked_x = event->x;
	        DrawBondsInfo->clicked_y = event->y;

	        /* Store this bond as the current bond */
	        DrawBondsInfo->cur_bnd_p = drbnd_p;
	        }

	      else 
                {  
                /* store_BondNew retured NULL - too many bonds attached.
	           Manage appropriate Error Dialog and discard this bond.
                */
	        DrawBondsInfo->esc_mode = TRUE;
	        DrawBondsInfo->cur_bnd_p = (Dsp_Bond_t *) NULL;
	        InfoWarn_Show ("many_bonds_errdg");
	        delete_Atom (DrawMol_p, dratm_p);
	        }
	      }
	    } /* End else get_Bond_There */
          } /*End not in the "ecsape mode" */
        } /* End of "down1" */
    
      else if ((strcmp (args[0], "down2") == 0) && (!DrawBondsInfo->esc_mode)) 
        {
        /* If the bond specified by drawing already exists. */
        drbnd_p = get_Bond_There (DrawMol_p, event->x, event->y,
          DrawBondsInfo->clicked_x, DrawBondsInfo->clicked_y);
        if (drbnd_p != NULL) 
          {
	  dratm_p = get_Atom_There (DrawMol_p, event->x, event->y);

	  /* Erase bond on both Drawing Area and Pixmap. */
	  draw_Bond (DrawDisplay, 
            XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), gc,
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), DrawMol_p, 
            drbnd_p, FALSE, FALSE);
	  draw_Bond (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), gc, 
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), DrawMol_p, 
            drbnd_p, FALSE, FALSE);
	  drbnd_p->nlines = DrawFlags.dflt_bond;

	  /* Draw bond again */
	  draw_Bond (DrawDisplay, 
            XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), gc, 
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), DrawMol_p,
            drbnd_p, TRUE, TRUE);
	  draw_Bond (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), gc,
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), DrawMol_p, 
            drbnd_p, TRUE, TRUE);
          } /* End of if there is a bond specified by drawings */

        else 
          {
	  dratm_p = store_AtomNew (DrawMol_p, "C", "\0", "\0", event->x, 
            event->y);
          drbnd_p = store_BondNew (DrawMol_p, DrawBondsInfo->clicked_x, 
            DrawBondsInfo->clicked_y, dratm_p->x, dratm_p->y,
            DrawFlags.dflt_bond);
	  if (drbnd_p != NULL) 
            {
	    draw_Bond (DrawDisplay, 
              XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), gc, 
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), DrawMol_p, 
              drbnd_p, TRUE, TRUE);
            draw_Bond (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), gc,
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), 
              DrawMol_p, drbnd_p, TRUE, TRUE);

	    /* Store this bond as the current bond */
	    DrawBondsInfo->cur_bnd_p = drbnd_p;
	    }
	  else  /* store_BondNew retured NULL - too many bonds attached */
            { 
	    /* Manage appropriate Error Dialog and discard this bond */
	    DrawBondsInfo->esc_mode = TRUE;
	    DrawBondsInfo->cur_bnd_p = (Dsp_Bond_t *) NULL;
	    InfoWarn_Show ("many_bonds_errdg");
	    if (dratm_p->adj_info.num_neighbs == 0)
	      delete_Atom (DrawMol_p, dratm_p);
	    }
          } /* End of there is no bond specified by drawings */

        /* Store the point the bond ends at */
        DrawBondsInfo->clicked_x = dratm_p->x;
        DrawBondsInfo->clicked_y = dratm_p->y;
        XCopyArea (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
          XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), 
          gc, 0, 0, DRW_DA_PXMP_WD, DRW_DA_PXMP_HT, 0, 0);
        } /* End of "down2" */

      else if (strcmp (args[0], "motion") == 0) 
        {
        if (!DrawBondsInfo->esc_mode) 
          {
          dratm_p = get_Atom_There (DrawMol_p, event->x, event->y);
	  if (dratm_p != NULL) 
            { 
	    if (!DrawBondsInfo->in_atom_area) 
              {
	      XCopyArea (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
                XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), gc, 0, 0, 
                DRW_DA_PXMP_WD, DRW_DA_PXMP_HT, 0, 0);
	      XDrawLine (event->display, event->window, gc, 
                dratm_p->x, dratm_p->y, DrawBondsInfo->clicked_x, 
                DrawBondsInfo->clicked_y);
	      }
	    }

	  else 
            {
	    XCopyArea (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB),
              XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), gc, 0, 0, 
              DRW_DA_PXMP_WD, DRW_DA_PXMP_HT, 0, 0);
	    XDrawLine (event->display, event->window, 
              gc, event->x, event->y,
              DrawBondsInfo->clicked_x, DrawBondsInfo->clicked_y);
	    }
          }
        } /* End of "motion" */

      else if (strcmp (args[0], "down3") == 0) 
        {
        DrawBondsInfo->esc_mode = TRUE;
        DrawBondsInfo->cur_bnd_p = (Dsp_Bond_t *) NULL;

        /* Copy the Pixmap in order to erase the previous line
	   and hihglighted areas.
         */
        XCopyArea (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
          XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), gc, 0, 0, 
          DRW_DA_PXMP_WD, DRW_DA_PXMP_HT, 0, 0);

        /* Check whether to discard the atom that was made by the last
           drawing.
         */
        if ((dratm_p = get_Atom_There (DrawMol_p, DrawBondsInfo->clicked_x,  
            DrawBondsInfo->clicked_y)) && (dratm_p->adj_info.num_neighbs == 0) 
            && (dratm_p->isC) && (strcmp (dratm_p->chg, "\0") == 0))
	  DrawMol_p->natms--;     
        } /* End of "down3" */

      break; /* End of DRW_BOND_FLAG */

    case DRW_CLEAR_FLAG:
      if (strcmp (args[0], "down1") == 0) 
        {
        /* If there is a click on an atom, delete it with all bonds
           adjacent to it.
         */
        dratm_p = get_Atom_There (DrawMol_p, event->x, event->y);
        if (dratm_p != NULL) 
          {
          /* Delete all the adjacent bonds first */
	  jj = dratm_p->adj_info.num_neighbs;
	  for (adj_i = 0; adj_i < jj; adj_i++) 
            {
	    adj_j = (int) dratm_p->adj_info.adj_atms[0];
            atom_p = DrawMol_p->atoms + adj_j;
	    drbnd_p = get_Bond_There (DrawMol_p, dratm_p->x, dratm_p->y,
              atom_p->x, atom_p->y);
	    draw_Bond (DrawDisplay, 
              XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), gc, 
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), DrawMol_p, 
              drbnd_p, TRUE, TRUE);	  
	    draw_Bond (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), gc,
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), 
              DrawMol_p, drbnd_p, TRUE, TRUE);	  
	    delete_Bond (DrawMol_p, drbnd_p);
	    if ((atom_p->adj_info.num_neighbs == 0) && (atom_p->isC) 
	        && (strcmp (atom_p->chg, "\0") == 0))
	      delete_Atom (DrawMol_p, atom_p);

	    /* dratm_p could point at the last atom in the molecule.
	       Then, delete_Atom(DrawMol_p, atom_p) copied it to the
	       place of atom_p. Since we want to point at the last version
	       of dratm_p, redirect dratm_p
	     */
	    if ((DrawMol_p->atoms + DrawMol_p->natms + 1) == dratm_p)
	      dratm_p = atom_p;
	    } /* End of for all adjacent bonds */

	  draw_Atom (DrawDisplay, 
            XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), gc, 
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), dratm_p, 
            DRW_ATOM_SIZE_LRG);
	  draw_Atom (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), gc,
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), dratm_p, 
            DRW_ATOM_SIZE_LRG);
	  delete_Atom (DrawMol_p, dratm_p);
          }
      
        /* If there is a click on a bond, delete it */
        drbnd_p = in_Bond_Scope (DrawMol_p, event->x, event->y);
        if (drbnd_p != NULL) 
          {
	  draw_Bond (DrawDisplay, 
            XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), gc, 
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), DrawMol_p, 
            drbnd_p, TRUE, TRUE);
	  erase_Bond (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), gc,
            DrawMol_p, drbnd_p);
          }
        } /* End of "down1" */
      break; /* End of DRW_CLEAR_FLAG */

    case DRW_SELECT_FLAG:
      if (strcmp (args[0], "down1") == 0) 
        {
        /* If this is the first click in selection mode 
           or outside the selected area, start selection.
        */
        if (!DrawSelInfo->move_it && (!DrawSelInfo->isSelected 
	    || event->x < DrawSelInfo->selected_x 
	    || event->y < DrawSelInfo->selected_y 
	    || event->x > DrawSelInfo->selected_x + DrawSelInfo->sel_width
	    || event->y > DrawSelInfo->selected_y + DrawSelInfo->sel_height))
          {
	  /* If smth was selected prior, unselect it */
	  unmark_selected (DrawMol_p, event->display, 
            DrawTool_Pixmap_Get (&GDrawToolCB), event->window, gc);
	  DrawBondsInfo->clicked_x = event->x;
	  DrawBondsInfo->clicked_y = event->y;
	  DrawBondsInfo->prev_x = event->x;
	  DrawBondsInfo->prev_y = event->y;

	  /* Initialize information about position of atom 
	     reletively to position of pointer.
	  */
	  dratm_p = DrawMol_p->atoms;
	  for (jj = 0; jj < DrawMol_p->natms; jj++) 
            {
	    dratm_p->kvadr = atom_pos_to_pointer_pos (dratm_p->x,dratm_p->y,
              event->x, event->y);
	    ++dratm_p;
	    }
          } /* End of the first click in selection mode */

        /* If this is the second (completing) click in selection mode */
        else if  (DrawSelInfo->move_it && !DrawSelInfo->isSelected) 
          {
	  DrawSelInfo->move_it = FALSE;

	  /* Draw a line that complete selecting loop */
	  XSetForeground (DrawDisplay, gc, 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLUE));
	  XDrawLine (event->display, event->window,
            gc, event->x, event->y,
            DrawBondsInfo->clicked_x, DrawBondsInfo->clicked_y);

	  /* Check selection status of atoms, if someone dragged mouse
	     directly to the point of the first click.
	  */
	  check_kvadr (DrawMol_p, DrawBondsInfo->clicked_x, 
            DrawBondsInfo->clicked_y, event->x, event->y);

	  /* Copy the Pixmap in order to erase the selecting curve */
	  XCopyArea (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
            XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), gc, 0, 0, 
            DRW_DA_PXMP_WD, DRW_DA_PXMP_HT, 0, 0);

	  /* Highlight atoms that have been selected */
	  mark_selected (DrawMol_p, event->display, event->window, 
            gc);
          } /* End of the second click in selection mode */
  
        /* If there is click inside of the selected area, 
	   make preparation to move it.
         */
        else if (DrawSelInfo->isSelected && !DrawSelInfo->move_it 
	    && event->x > DrawSelInfo->selected_x 
            && event->y > DrawSelInfo->selected_y 
            && event->x < DrawSelInfo->selected_x + DrawSelInfo->sel_width 
            && event->y < DrawSelInfo->selected_y + DrawSelInfo->sel_height) 
          {
	  DrawSelInfo->move_it = TRUE;

	  /* Store click coords to calculate deltas when the area will be 
             moved.
          */
	  DrawBondsInfo->clicked_x = event->x;
	  DrawBondsInfo->clicked_y = event->y;
	  DrawBondsInfo->prev_x = event->x;
	  DrawBondsInfo->prev_y = event->y;

	  /* Erase the selected area on the pixmap */
	  draw_selected (DrawMol_p, event->display, 
            DrawTool_Pixmap_Get (&GDrawToolCB), gc, 
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), 0, 0, FALSE);

          } /* End of initializing for movement of selected area */

        /* If there is a click during the movement of selected area,
	   stop moving it.
         */
        else if ((DrawSelInfo->isSelected) && (DrawSelInfo->move_it)) 
          {
	  DrawSelInfo->move_it = FALSE;
	  draw_selected (DrawMol_p, event->display, 
            DrawTool_Pixmap_Get (&GDrawToolCB), 
            gc, SynAppR_IthColor_Get (&GSynAppR, 
            SAR_CLRI_BLUE), event->x - DrawBondsInfo->prev_x, 
            event->y - DrawBondsInfo->prev_y, TRUE);
	  XCopyArea (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
            XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), gc, 0, 0, 
            DRW_DA_PXMP_WD, DRW_DA_PXMP_HT, 0, 0);
          }
        } /* End of "down1" */

      if (strcmp (args[0], "motion") == 0) 
        {
        /* If smth is already selected, this motion event
	   should move selected area.
         */
        if ((DrawSelInfo->isSelected) && (DrawSelInfo->move_it)) 
          {
	  draw_selected (DrawMol_p, event->display, event->window, 
            gc, SynAppR_IthColor_Get (&GSynAppR, 
            SAR_CLRI_BLUE), event->x - DrawBondsInfo->prev_x, 
            event->y - DrawBondsInfo->prev_y,TRUE);
          DrawBondsInfo->prev_x = event->x;
	  DrawBondsInfo->prev_y = event->y;
          }

        else if (DrawSelInfo->move_it && !DrawSelInfo->isSelected) 
          {
	  XSetForeground (DrawDisplay, gc, 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLUE));
	  XDrawLine (event->display, event->window, gc,
            DrawBondsInfo->prev_x, DrawBondsInfo->prev_y,
            event->x, event->y);

	  /* Check selection status of atoms after this motion event */
	  check_kvadr (DrawMol_p, event->x, event->y,
            DrawBondsInfo->prev_x, DrawBondsInfo->prev_y);
	  DrawBondsInfo->prev_x = event->x;
	  DrawBondsInfo->prev_y = event->y;
          }
        } /* End of "motion" */
      break;  /* End of DRW_SELECT_FLAG */

    case DRW_ATOM_FLAG:
      if (strcmp (args[0], "down1") == 0) 
        {
        dratm_p = get_Atom_There (DrawMol_p, event->x, event->y);
        if (dratm_p != NULL) 
          {
	  DrawBondsInfo->clicked_x = event->x;
	  DrawBondsInfo->clicked_y = event->y;

	  /* Assign current atom symbol (specified by pushbuttons in
	     periodic table) to the clicked atom by invoking Atom_Mark_CB.
	   */
	  Atom_Mark_CB ((Widget) NULL, (XtPointer) DrawFlags.cur_atmsym,
            (XtPointer) NULL);
          }
        }
      break;  /* End of DRW_ATOM_FLAG */

    case DRW_RXNCNTR_FLAG:
      dratm_p = get_Atom_There (DrawMol_p, event->x, event->y);
      if ((strcmp (args[0], "down1") == 0) && dratm_p != NULL) 
        {
        /* If rxn center has been specified, clear the previous one */
        if (DrawMol_p->rxncnr_p) 
          {
	  XSetForeground (DrawDisplay, gc, 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
	  XDrawRectangle (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), gc, 
            DrawMol_p->rxncnr_p->x - (DRW_HILIT_AREA << 1),
            DrawMol_p->rxncnr_p->y - (DRW_HILIT_AREA << 1),
            DRW_HILIT_AREA << 2, DRW_HILIT_AREA << 2);
          }

        /* Mark clicked atom as a reaction center */
        XSetForeground (DrawDisplay, gc, 
          SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_GOLD));
        XDrawRectangle (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), gc,
          dratm_p->x - (DRW_HILIT_AREA << 1), 
          dratm_p->y - (DRW_HILIT_AREA << 1), DRW_HILIT_AREA << 2, 
          DRW_HILIT_AREA << 2);
        XCopyArea (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
          XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), gc, 0, 0, 
          DRW_DA_PXMP_WD, DRW_DA_PXMP_HT, 0, 0);

       /* da_wid = retrieve_widget_id ("working_da", 1);
        XUndefineCursor (XtDisplay (da_wid), XtWindow (da_wid));*/
        
        DrawMol_p->rxncnr_p = dratm_p;
        DrawFlags.drawing_mode = DRW_BOND_FLAG;
        Draw_Mode_Reset (DRW_BOND_FLAG);
        }
      break;  /* End of DRW_RXNCNTR_FLAG */
    }/*End of switch drawing mode */

  /* General actions need to be done regardless drawing mode 
     (except selecting drawing mode), if motion event occurs.
   */
  if ((strcmp (args[0], "motion") == 0) 
      && (DrawFlags.drawing_mode != DRW_SELECT_FLAG)) 
    {
    /* If there is an atom at this point, highlight it.
     */    
    if ((dratm_p = get_Atom_There (DrawMol_p, event->x, event->y)) 
	&& !(DrawBondsInfo->in_atom_area)) 
      {
      switch (DrawFlags.drawing_mode) 
        {
        case DRW_BOND_FLAG:
	  XSetForeground (DrawDisplay, gc, 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_GREEN));
	  break;
        case DRW_ATOM_FLAG:
	  XSetForeground (DrawDisplay, gc, 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_VIOLET));
	  break;
        case DRW_CLEAR_FLAG:
	  XSetForeground (DrawDisplay, gc, 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_RED));
	  break;
        case DRW_RXNCNTR_FLAG:
	  XSetForeground (DrawDisplay, gc, 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_GOLD));
	  break;
        }

      XFillRectangle (DrawDisplay, 
        XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), gc,
        dratm_p->x - DRW_HILIT_AREA, dratm_p->y - DRW_HILIT_AREA,     
        (DRW_HILIT_AREA << 1), (DRW_HILIT_AREA << 1));

      DrawBondsInfo->in_atom_area = TRUE;
      DrawBondsInfo->in_bond_area = FALSE;

      /* If there is "clear" drawing mode, highlight all bonds adjacent
	 to this atom.
       */
      if (DrawFlags.drawing_mode == DRW_CLEAR_FLAG) 
        {
	/* Highlight bonds */
	for (adj_i = 0; adj_i < dratm_p->adj_info.num_neighbs; adj_i++) 
          {
	  adj_j = (int) dratm_p->adj_info.adj_atms[adj_i];
	  drbnd_p = get_Bond_There (DrawMol_p, dratm_p->x, dratm_p->y,
            (DrawMol_p->atoms + adj_j)->x, (DrawMol_p->atoms + adj_j)->y);
	  draw_Bond (DrawDisplay, 
            XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), gc, 
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_RED), DrawMol_p, 
            drbnd_p, TRUE, TRUE);	  
	  }
        } /* End hilite bonds in "clear" mode */

      XSetForeground (DrawDisplay, gc, 
        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
      }

    /* If we get out of the scope of atom, erase highlighted area 
     */
    if (!(dratm_p = get_Atom_There (DrawMol_p, event->x, event->y)) 
	&& (DrawBondsInfo->in_atom_area)) 
      {
      /* Copy the Pixmap in order to erase the previously hihglighted areas */
      XCopyArea (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
        XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), gc,
        0, 0, DRW_DA_PXMP_WD, DRW_DA_PXMP_HT, 0, 0);
      DrawBondsInfo->in_atom_area = FALSE;
      }

    if (DrawFlags.drawing_mode == DRW_CLEAR_FLAG) 
      {
      /* If there is a bond at this point, highlight it.
       */    
      if ((drbnd_p = in_Bond_Scope (DrawMol_p, event->x, event->y))
	  && !DrawBondsInfo->in_bond_area && !DrawBondsInfo->in_atom_area) 
        {
	draw_Bond (DrawDisplay, 
          XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), gc,
          SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_RED), DrawMol_p, 
          drbnd_p, TRUE, TRUE);
	DrawBondsInfo->in_bond_area = TRUE;
        }

      /* If we get out of the scope of bond, erase highlighted area 
       */
      if (!(drbnd_p = in_Bond_Scope (DrawMol_p, event->x, event->y)) 
	  && (DrawBondsInfo->in_bond_area)) 
        {
	/* Copy Pixmap in order to erase the previously highlighted areas */
	XCopyArea (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
          XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), gc,
          0, 0, DRW_DA_PXMP_WD, DRW_DA_PXMP_HT, 0, 0);
	DrawBondsInfo->in_bond_area = FALSE;
        }
      } /* End of not in the DRW_BOND mode */
    } /* End of "general motion" */

  return ; 
}
/*  End of Draw_Draw  */

/****************************************************************************
*  
*  Function Name:                 mark_selected
*  
*****************************************************************************/  

void mark_selected 
  (
  Dsp_Molecule_t  *mol_p,
  Display         *disp, 
  Drawable         drbl, 
  GC               gc
  )
{
  Dsp_Atom_t      *slatm1_p, *slatm2_p, *atom_p;
  Dsp_Bond_t      *bond_p;
  int              max_x, max_y;
  int              atm_i, adj_i, ii;
  int              neighb_count;

  DrawSelInfo->isSelected = FALSE;
  DrawSelInfo->selected_x = DrawSelInfo->selected_y = 0;
  max_x = max_y = 0;

  slatm1_p = mol_p->atoms;
  for (atm_i = 0; atm_i < mol_p->natms; atm_i++) 
    {
    if (slatm1_p->xp && slatm1_p->xn && slatm1_p->yp && slatm1_p->yn) 
      {
      neighb_count = 0;

      /* If atom is selected and it doesn't have neighbors,
	 include it in selected atoms.
       */
      if (slatm1_p->adj_info.num_neighbs == 0) 
        {
	DrawSelInfo->isSelected = TRUE;
	slatm1_p->isSelected = DRW_SELECT_TOTAL;
	draw_Atom (disp, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
          SAR_CLRI_BLUE), slatm1_p, DRW_ATOM_SIZE_LRG);

	/* Update info about left upper corner of selected area */
	if ((slatm1_p->x < DrawSelInfo->selected_x) 
	    || (DrawSelInfo->selected_x == 0))
	  DrawSelInfo->selected_x = slatm1_p->x;

	if ((slatm1_p->y < DrawSelInfo->selected_y) 
	    || (DrawSelInfo->selected_y == 0))
	  DrawSelInfo->selected_y = slatm1_p->y;

	/* Update info about right buttom corner of selected area */
	if (slatm1_p->x > max_x)
	  max_x = slatm1_p->x;

	if (slatm1_p->y > max_y)
	  max_y = slatm1_p->y;

        } /* End if atom has no neighbors */

      else 
        {
	/* If atom is selected and has neighbors, 
	   check whether its neighbors are selected 
	 */
	for (adj_i = 0; adj_i < slatm1_p->adj_info.num_neighbs; adj_i++) 
          {
	  slatm2_p = mol_p->atoms + slatm1_p->adj_info.adj_atms[adj_i];
	  if (slatm2_p->xp && slatm2_p->xn && slatm2_p->yp && slatm2_p->yn) 
            {
	    /* If neighbor atom is selected too, increment neighbor counter
	       and draw bond that connects selected neighbors
	     */
	    DrawSelInfo->isSelected = TRUE;
	    slatm1_p->isSelected = DRW_SELECT_MOVE;
	    ++neighb_count;
	    bond_p = get_Bond_There (mol_p, slatm1_p->x, slatm1_p->y,
              slatm2_p->x, slatm2_p->y);
	    draw_Bond (disp, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
              SAR_CLRI_BLUE), mol_p, bond_p, FALSE, FALSE);

	    /* Update info about left upper corner of selected area */
	    if ((slatm1_p->x < DrawSelInfo->selected_x) 
		|| (DrawSelInfo->selected_x == 0))
	      DrawSelInfo->selected_x = slatm1_p->x;

	    if ((slatm1_p->y < DrawSelInfo->selected_y) 
		|| (DrawSelInfo->selected_y == 0))
	      DrawSelInfo->selected_y = slatm1_p->y;

	    if ((slatm2_p->x < DrawSelInfo->selected_x) 
		|| (DrawSelInfo->selected_x == 0))
	      DrawSelInfo->selected_x = slatm2_p->x;

	    if ((slatm2_p->y < DrawSelInfo->selected_y) 
		|| (DrawSelInfo->selected_y == 0))
	      DrawSelInfo->selected_y = slatm2_p->y;

	    /* Update info about right buttom corner of selected area */
	    if (slatm1_p->x > max_x)
	      max_x = slatm1_p->x;

	    if (slatm1_p->y > max_y)
	      max_y = slatm1_p->y;

	    if (slatm2_p->x > max_x)
	      max_x = slatm2_p->x;

	    if (slatm2_p->y > max_y)
	      max_y = slatm2_p->y;
            } /* End if neighbor is selected */
          } /* End for all neighbors */

	/* Increment isSelected field of all bonds that adjacent to 
	   slatm1_p. Since we scan all atoms, bond that coonects
	   selected atoms will be incremented twice. "Move" bonds
	   will be incremented only once.
	 */
	if (slatm1_p->isSelected == DRW_SELECT_MOVE)
	  for (ii = 0; ii < slatm1_p->adj_info.num_neighbs; ii++) 
            {
	    atom_p = mol_p->atoms + slatm1_p->adj_info.adj_atms[ii];
	    bond_p = get_Bond_There (mol_p, slatm1_p->x, slatm1_p->y,
				     atom_p->x, atom_p->y);
	    ++bond_p->isSelected;
	    }

	/* If its all neighbors are selected, mark it and draw it. */
	if (neighb_count == slatm1_p->adj_info.num_neighbs) 
          {
	  slatm1_p->isSelected = DRW_SELECT_TOTAL;
	  draw_Atom (disp, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
            SAR_CLRI_BLUE), slatm1_p, DRW_ATOM_SIZE_LRG);
	  }
        } /* End if atom has neighbors (else) */
      } /* End if atom is selected */

    ++slatm1_p;
    }/* End for all atoms */

  /* If bond(s) was selected, mark that area */
  if (DrawSelInfo->isSelected) 
    {
    DrawSelInfo->sel_width = max_x - DrawSelInfo->selected_x 
      + (DRW_HILIT_AREA << 1) - 1;
    DrawSelInfo->sel_height = max_y - DrawSelInfo->selected_y
      + (DRW_HILIT_AREA << 1) - 1;

    /* Draw a rectangle around selected area */
    XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLUE));
    XDrawRectangle (disp, drbl, gc, DrawSelInfo->selected_x - DRW_HILIT_AREA,
      DrawSelInfo->selected_y - DRW_HILIT_AREA, DrawSelInfo->sel_width,    
      DrawSelInfo->sel_height);
    XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
    }
  return ;
}
/*  End of mark_selected  */

/****************************************************************************
*  
*  Function Name:                 unmark_selected
*  
*****************************************************************************/  

void unmark_selected 
  (
  Dsp_Molecule_t  *mol_p,
  Display         *disp, 
  Drawable         drbl1,
  Drawable         drbl2,
  GC               gc)
{
  Dsp_Atom_t      *atom_p;
  Dsp_Bond_t      *bond_p;
  int              ii;

  if (DrawSelInfo->isSelected)
    draw_selected (mol_p,disp, drbl1, gc, SynAppR_IthColor_Get (&GSynAppR, 
      SAR_CLRI_BLACK), 0, 0, FALSE);

  XCopyArea (DrawDisplay, drbl1, drbl2, gc, 0, 0, DRW_DA_PXMP_WD, DRW_DA_PXMP_HT,
    0, 0);
  DrawSelInfo->isSelected = FALSE;
  DrawSelInfo->move_it = TRUE;

  /* Make all atoms and bonds unselected */
  atom_p = mol_p->atoms;
  for (ii = 0; ii < mol_p->natms; ii++) 
    {
    atom_p->xp = FALSE;
    atom_p->xn = FALSE;
    atom_p->yp = FALSE;
    atom_p->yn = FALSE;
    atom_p->isSelected = DRW_SELECT_NONE;
    ++atom_p;
    }

  bond_p = mol_p->bonds;
  for (ii = 0; ii < mol_p->nbnds; ii++) 
    {
    bond_p->isSelected = DRW_SELECT_NONE;
    ++bond_p;
    }

  return ;
}
/*  End of unmark_selected  */

/****************************************************************************
*  
*  Function Name:                 draw_selected
*  
*****************************************************************************/  

void draw_selected 
  (
  Dsp_Molecule_t  *mol_p,
  Display     *disp, 
  Drawable     drbl, 
  GC           gc,
  XColor       color,
  int          delta_x,
  int          delta_y,
  Boolean_t    draw_rect)
{
  Dsp_Atom_t      *dratm_p;
  Dsp_Bond_t      *drbnd_p;
  int              ii;

  /* Erase previous selected area, first */
  drbnd_p = mol_p->bonds;
  for (ii = 0; ii < mol_p->nbnds; ii++) 
    {
    if (drbnd_p->isSelected != DRW_SELECT_NONE)
      draw_Bond (disp, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
        SAR_CLRI_WHITE), mol_p, drbnd_p, FALSE, FALSE);

    ++drbnd_p;
    }

  dratm_p = mol_p->atoms;
  for (ii = 0; ii < mol_p->natms; ii++) 
    {
    if (dratm_p->isSelected != DRW_SELECT_NONE)
      draw_Atom (disp, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
        SAR_CLRI_WHITE), dratm_p, DRW_ATOM_SIZE_LRG);

    ++dratm_p;
    }

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
  XDrawRectangle (disp, drbl, gc, DrawSelInfo->selected_x - DRW_HILIT_AREA,
    DrawSelInfo->selected_y - DRW_HILIT_AREA, DrawSelInfo->sel_width,    
    DrawSelInfo->sel_height);

  /* Now redraw selected area in the new place specified by deltas */
  dratm_p = mol_p->atoms;
  for (ii = 0; ii < mol_p->natms; ii++) 
    {
    if (dratm_p->isSelected != DRW_SELECT_NONE) 
      {
      dratm_p->x += delta_x; 
      dratm_p->y += delta_y;
      } 

    ++dratm_p;
    }

  drbnd_p = mol_p->bonds;
  for (ii = 0; ii < mol_p->nbnds; ii++) 
    {
    if (drbnd_p->isSelected == DRW_SELECT_TOTAL)
      draw_Bond (disp, drbl, gc, color, mol_p, drbnd_p, FALSE, FALSE);      

    if (drbnd_p->isSelected == DRW_SELECT_MOVE) 
      {
      if (drbnd_p->latm_p->isSelected == DRW_SELECT_MOVE) 
        {
	if (draw_rect)
	  draw_Bond (disp, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
            SAR_CLRI_BLACK), mol_p, drbnd_p, FALSE, TRUE);
	else
	  draw_Bond (disp, drbl, gc, color, mol_p, drbnd_p, FALSE, TRUE);
        }

      if (drbnd_p->ratm_p->isSelected == DRW_SELECT_MOVE) 
        {
	if (draw_rect)
	  draw_Bond (disp, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
            SAR_CLRI_BLACK), mol_p, drbnd_p, TRUE, FALSE);
	else
	  draw_Bond (disp, drbl, gc, color, mol_p, drbnd_p, TRUE, FALSE);
        }
      }

    ++drbnd_p;
    } /* End for all bonds */

  dratm_p = mol_p->atoms;
  for (ii = 0; ii < mol_p->natms; ii++) 
    {
    if (dratm_p->isSelected != DRW_SELECT_NONE)
      draw_Atom (disp, drbl, gc, color, dratm_p, DRW_ATOM_SIZE_LRG);

    ++dratm_p;
    }

  if (draw_rect) 
    {
    DrawSelInfo->selected_x += delta_x; 
    DrawSelInfo->selected_y += delta_y; 
    XDrawRectangle (disp, drbl, gc, DrawSelInfo->selected_x - DRW_HILIT_AREA,
      DrawSelInfo->selected_y - DRW_HILIT_AREA, DrawSelInfo->sel_width,    
      DrawSelInfo->sel_height);
    }

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
  return ;
}
/*  End of draw_selected  */

/****************************************************************************
*  
*  Function Name:                 in_Bond_Scope
*  
*****************************************************************************/  

Dsp_Bond_t *in_Bond_Scope 
  (
  Dsp_Molecule_t* mol_p,
  int             x, 
  int             y
  )
{
  float         k, l;
  int           x0, y0, yy;
  int           x1, x2, y1, y2;
  int           bnd_i;
  Dsp_Bond_t   *bnd_p;

  bnd_p = mol_p->bonds;
  for (bnd_i = 0; bnd_i < mol_p->nbnds; bnd_i++)
    {
    if ((((x < bnd_p->latm_p->x + DRW_HILIT_AREA) && 
	  (x > bnd_p->ratm_p->x - DRW_HILIT_AREA)) ||
	 ((x < bnd_p->ratm_p->x + DRW_HILIT_AREA) && 
	  (x > bnd_p->latm_p->x - DRW_HILIT_AREA))) &&
	(((y < bnd_p->latm_p->y + DRW_HILIT_AREA) && 
	  (y > bnd_p->ratm_p->y - DRW_HILIT_AREA)) ||
	 ((y < bnd_p->ratm_p->y + DRW_HILIT_AREA) && 
	  (y > bnd_p->latm_p->y - DRW_HILIT_AREA)))) 
      {
      if (abs(bnd_p->ratm_p->y - bnd_p->latm_p->y) < 
	  abs(bnd_p->ratm_p->x - bnd_p->latm_p->x)) 
        {
	x0 = x;
	y0 = y;
	x1 = bnd_p->latm_p->x;
	x2 = bnd_p->ratm_p->x;
	y1 = bnd_p->latm_p->y;
	y2 = bnd_p->ratm_p->y;
        }
      else 
        {
	x0 = y;
	y0 = x;
	x1 = bnd_p->latm_p->y;
	x2 = bnd_p->ratm_p->y;
	y1 = bnd_p->latm_p->x;
	y2 = bnd_p->ratm_p->x;
        }

      k = (float) (y2 - y1) / (float) (x2 - x1);
      l = y1 - (k * x1);
      yy = (int) ( (float) k * x0 + l);
      if ((y0 < yy + DRW_HILIT_AREA) && (y0 > yy - DRW_HILIT_AREA))
	return (bnd_p);
      }

    ++bnd_p;
    }

  return ((Dsp_Bond_t *) NULL);
}
/*  End of in_Bond_Scope  */

/****************************************************************************
*  
*  Function Name:                 Atom_Mark_CB
*  
*****************************************************************************/  

void Atom_Mark_CB 
  (
  Widget    w,
  XtPointer client_data,
  XtPointer call_data
  )
{
  Dsp_Atom_t      *dratm_p, *atom_p;
  Dsp_Bond_t      *bond_p;
  String          tag_string; 
  int             adj_i;

  tag_string = (String) client_data;

  if (strcmp ((char *) tag_string, CS_MULTIPLE) == 0) 
    {
    DrawFlags.drawing_mode = DRW_ATOM_FLAG;
    DrawBondsInfo->esc_mode = TRUE;
    strcpy (DrawFlags.cur_atmsym, "C");
    return;
    }

  if (strcmp ((char *) tag_string, CS_DISMISS) == 0) 
    {
    DrawFlags.drawing_mode = DRW_BOND_FLAG;
    DrawBondsInfo->esc_mode = TRUE;
    strcpy (DrawFlags.cur_atmsym, "C");
    Draw_Mode_Reset (DRW_BOND_FLAG);
    XtUnmanageChild (DrawTool_PeriodicTbl_Get (&GDrawToolCB));
    return;
    } 

  /* If callback was activated by a pushbutton in "Draw Atom" mode,
     store cur_atmsym.
   */
  if ((DrawFlags.drawing_mode == DRW_ATOM_FLAG) && (w != (Widget) NULL)) 
    {
    strcpy (DrawFlags.cur_atmsym, tag_string);
    return;
    }

  if (!(dratm_p = get_Atom_There (DrawMol_p, DrawBondsInfo->clicked_x, 
       DrawBondsInfo->clicked_y)))
    return;

  /* Erase previouse atom symbols */
  draw_Atom (DrawDisplay, XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), 
    DrawTool_GC_Get (&GDrawToolCB), 
    SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), dratm_p, 
    DRW_ATOM_SIZE_LRG);

  draw_Atom (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
    DrawTool_GC_Get (&GDrawToolCB), 
    SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), dratm_p, 
    DRW_ATOM_SIZE_LRG);

  if (strcmp ((char *) tag_string, "+") == 0)
    strcpy (dratm_p->chg, (char *) tag_string);

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
	atom_p = DrawMol_p->atoms + (int) dratm_p->adj_info.adj_atms[adj_i];
        bond_p = get_Bond_There (DrawMol_p, dratm_p->x, dratm_p->y,
          atom_p->x, atom_p->y);
	if (bond_p != NULL) 
          {
	  draw_Bond (DrawDisplay, 
            XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), 
            DrawTool_GC_Get (&GDrawToolCB),
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), DrawMol_p, 
            bond_p, TRUE, TRUE);
	  draw_Bond (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
            DrawTool_GC_Get (&GDrawToolCB), 
            SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), DrawMol_p, 
            bond_p, TRUE, TRUE);
	  }
        }
      }

    else
      dratm_p->isC = FALSE;
    }

  draw_Atom (DrawDisplay, 
    XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), 
    DrawTool_GC_Get (&GDrawToolCB), 
    SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), dratm_p, 
    DRW_ATOM_SIZE_LRG);

  draw_Atom (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
    DrawTool_GC_Get (&GDrawToolCB), 
    SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), dratm_p,
    DRW_ATOM_SIZE_LRG);
}
/*  End of Atom_Mark_CB  */

/****************************************************************************
*  
*  Function Name:                 atom_pos_to_pointer_pos
*  
*****************************************************************************/  

int atom_pos_to_pointer_pos 
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
/*  End of atom_pos_to_pointer_pos  */

/****************************************************************************
*  
*  Function Name:                 check_kvadr
*  
*****************************************************************************/  

void check_kvadr 
  (
  Dsp_Molecule_t  *mol_p,
  int              point_x, 
  int              point_y,
  int               old_x, 
  int              old_y
  )
{
  Dsp_Atom_t      *atom_p;
  int              new_kvadr;
  int              ii;

  atom_p = mol_p->atoms;
  for (ii = 0; ii < mol_p->natms; ii++) 
    {
    /* If kvadrant of pointer has been changed */
    if (atom_p->kvadr != (new_kvadr = atom_pos_to_pointer_pos (atom_p->x,
         atom_p->y, point_x, point_y))) 
      {
      switch (atom_p->kvadr) 
        {
        case 1:
	  switch (new_kvadr) 
            {
	    case 2:
	      atom_p->yn = not(atom_p->yn);
	      break;
	    case 3:
	      if (DRW_Slope (atom_p->x, point_x, point_y,old_x, old_y) 
                  < atom_p->y) 
                {
	        atom_p->yn = not(atom_p->yn);
	        atom_p->xn = not(atom_p->xn);
	        } 
	      else 
                {
	        atom_p->yp = not(atom_p->yp);
	        atom_p->xp = not(atom_p->xp);
	        }
	      break;
	    case 4:
	      atom_p->xp = not(atom_p->xp);
	      break;
	    }
	  break;

        case 2:
	  switch (new_kvadr) 
            {
	    case 1:
	      atom_p->yn = not(atom_p->yn);
	      break;
	    case 3:
	      atom_p->xn = not(atom_p->xn);
	      break;
	    case 4:
	      if (DRW_Slope(atom_p->x, point_x, point_y, old_x, old_y) 
                  < atom_p->y) 
                {
	        atom_p->yn = not(atom_p->yn);
	        atom_p->xp = not(atom_p->xp);
	        }
	      else 
                {
	        atom_p->yp = not(atom_p->yp);
	        atom_p->xn = not(atom_p->xn);
	        }
	      break;
	     }
	  break;

        case 3:
	  switch (new_kvadr) 
            {
	    case 1:
	      if (DRW_Slope(atom_p->x, point_x, point_y, old_x, old_y) 
                  < atom_p->y) 
                {
	        atom_p->yn = not(atom_p->yn);
	        atom_p->xn = not(atom_p->xn);
	        } 
	      else 
                {
	        atom_p->yp = not(atom_p->yp);
	        atom_p->xp = not(atom_p->xp);
	        }
	      break;
	    case 2:
	      atom_p->xn = not(atom_p->xn);
	      break;
	    case 4:
	      atom_p->yp = not(atom_p->yp);
	      break;
	    }
	  break;

        case 4:
	  switch (new_kvadr) 
            {
	    case 1:
	      atom_p->xp = not(atom_p->xp);
	      break;
	    case 2:
	      if (DRW_Slope(atom_p->x, point_x, point_y, old_x, old_y) 
                  < atom_p->y) 
                {
	        atom_p->yn = not(atom_p->yn);
	        atom_p->xp = not(atom_p->xp);
	        }
	      else 
                {
	        atom_p->yp = not(atom_p->yp);
	        atom_p->xn = not(atom_p->xn);
	        }
	      break;
            case 3:
	      atom_p->yp = not(atom_p->yp);
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
/*  End of check_kvadr  */

/****************************************************************************
*  
*  Function Name:                 Option_Choose_CB
*  
*****************************************************************************/  

void Option_Choose_CB  
  (
  Widget         w,
  XtPointer      client_data,
  XtPointer      call_data
  )
{
  int            tag;
  Dimension      sw_h, sw_w;
  Widget         da_wid;

  da_wid = DrawTool_DrawArea_Get(&GDrawToolCB);
  tag = (int) client_data;
  DrawFlags.drawing_mode = tag;
  DrawBondsInfo->esc_mode = TRUE;

  /* If smth was selected prior, unselect it */
  unmark_selected (DrawMol_p, DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
    XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), 
    DrawTool_GC_Get (&GDrawToolCB));

  switch (tag) 
    {
    case DRW_BOND_FLAG:
      XUndefineCursor (XtDisplay (da_wid), XtWindow (da_wid));

      XSetForeground (DrawDisplay, DrawTool_GC_Get(&GDrawToolCB), 
        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
      DrawBondsInfo->cur_bnd_p = (Dsp_Bond_t *) NULL;
      DrawBondsInfo->in_atom_area = FALSE;
      DrawBondsInfo->in_bond_area = FALSE;
      break;

    case DRW_ATOM_FLAG:
      XUndefineCursor (XtDisplay (da_wid), XtWindow (da_wid));
      
      InfoWarn_Show ("periodic_table_dialog");
      DrawFlags.drawing_mode = DRW_BOND_FLAG;
      Draw_Mode_Reset (DRW_BOND_FLAG);
      break;

    case DRW_CLEAR_FLAG:
      XDefineCursor (XtDisplay (da_wid), XtWindow (da_wid), 
        SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_ERASE));
      break;

    case DRW_SELECT_FLAG:
      XDefineCursor (XtDisplay (da_wid), XtWindow (da_wid), 
        SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_WRITE));
     
      DrawSelInfo->isSelected = FALSE;
      DrawSelInfo->move_it = FALSE;
      break;

    case DRW_REDRAW_FLAG:
      if (dsp_Shelley (DrawMol_p)) 
        {
        XUndefineCursor (XtDisplay (da_wid), XtWindow (da_wid));
        
        XSetForeground (DrawDisplay, DrawTool_GC_Get(&GDrawToolCB), 
          SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));

        XFillRectangle (DrawDisplay, 
          XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), 
          DrawTool_GC_Get(&GDrawToolCB), 
          0, 0, DRW_DA_PXMP_WD, DRW_DA_PXMP_HT);
        XFillRectangle (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
           DrawTool_GC_Get(&GDrawToolCB), 0, 0, DRW_DA_PXMP_WD, DRW_DA_PXMP_HT);

        XSetForeground (DrawDisplay, DrawTool_GC_Get(&GDrawToolCB), 
          SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));

        XtVaGetValues (DrawTool_ScrlldWin_Get (&GDrawToolCB),
          XmNwidth, &sw_w,
          XmNheight, &sw_h,
          NULL);

        draw_Molecule (DrawDisplay, 
          XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), 
          DrawTool_GC_Get(&GDrawToolCB), 
          DrawMol_p, AppDim_MolScale_Get (&GAppDim), sw_h, sw_w, TRUE);
        draw_Molecule (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
          DrawTool_GC_Get(&GDrawToolCB), DrawMol_p, 1.0, 0, 0, FALSE);
        }

      /* If Shelley code could not handle this transformation,
          popup error dialog.
      */
      else
        InfoWarn_Show ("kka:shelley_code_errdg");

      DrawFlags.drawing_mode = DRW_BOND_FLAG;
      Draw_Mode_Reset (DRW_BOND_FLAG);
      DrawBondsInfo->cur_bnd_p = (Dsp_Bond_t *) NULL;
      XmProcessTraversal (w, XmTRAVERSE_HOME);
      break;

    case DRW_RXNCNTR_FLAG:
      XDefineCursor (XtDisplay (da_wid), XtWindow (da_wid), 
        SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_TARGET));
      
      XmProcessTraversal (w, XmTRAVERSE_HOME);
      break;
    }
  
  return ;
}
/*  End of Option_Choose_CB  */

/****************************************************************************
*  
*  Function Name:                 Menu_Choice_CB
*  
*****************************************************************************/  

void Menu_Choice_CB 
  (
  Widget w,
  XtPointer client_data,
  XtPointer call_data
  )
				  
{
  Dsp_Atom_t      *atom_p;
  Dsp_Bond_t      *bond_p;
  int              ii;
  Boolean_t        buble;
  Widget           da_wid;
  int              tag;

  tag = (int) client_data;
  da_wid = DrawTool_DrawArea_Get(&GDrawToolCB);

  switch (tag) 
    {
    
    case DRW_MENU_DELETE_ALL:
      tag = SMU_YES_RESPONSE;
      Draw_Mode_Reset (DRW_BOND_FLAG);

    case DRW_MENU_SELECT_ALL:
      DrawBondsInfo->esc_mode = TRUE;
      DrawSelInfo->move_it = FALSE;
      XDefineCursor (XtDisplay (da_wid), XtWindow (da_wid), 
        SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_WRITE));
      
      DrawFlags.drawing_mode = DRW_SELECT_FLAG;
      Draw_Mode_Reset (DRW_SELECT_FLAG);

      /* Mark all atoms as selected */
      atom_p = DrawMol_p->atoms;
      for (ii = 0; ii < DrawMol_p->natms; ii++) 
        {
        atom_p->xp = atom_p->xn = atom_p->yp = atom_p->yn = TRUE;
        ++atom_p;
        }

      mark_selected (DrawMol_p, DrawDisplay, 
        DrawTool_Pixmap_Get (&GDrawToolCB), DrawTool_GC_Get (&GDrawToolCB));
      XCopyArea (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
        XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), 
        DrawTool_GC_Get (&GDrawToolCB), 0, 0, DRW_DA_PXMP_WD, DRW_DA_PXMP_HT, 0, 0);

      if ((tag == SMU_NO_RESPONSE) || (tag == DRW_MENU_SELECT_ALL))
        break;

    case DRW_MENU_DELETE_SEL:
      DrawBondsInfo->esc_mode = TRUE;

      /* Delete all selected bonds */
      do 
        {
        buble = FALSE;
        bond_p = DrawMol_p->bonds;
        for (ii = 0; ii < DrawMol_p->nbnds; ii++) 
          {
	  if (bond_p->isSelected == DRW_SELECT_TOTAL) 
            {
	    draw_Bond (DrawDisplay, 
              XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), 
              DrawTool_GC_Get (&GDrawToolCB), 
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), DrawMol_p, 
              bond_p, TRUE, TRUE);
	    draw_Bond (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
              DrawTool_GC_Get (&GDrawToolCB), 
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), 
              DrawMol_p, bond_p, TRUE, TRUE);
	    delete_Bond (DrawMol_p, bond_p);
	    buble = TRUE;
	    }

	  ++bond_p;
          }
        } while (buble);

      /* Delete all selected atoms */
      do 
        {
        buble = FALSE;
        atom_p = DrawMol_p->atoms;
        for (ii = 0; ii < DrawMol_p->natms; ii++) 
          {
	  if (atom_p->isSelected == DRW_SELECT_TOTAL) 
            {
	    draw_Atom (DrawDisplay, 
              XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), 
              DrawTool_GC_Get (&GDrawToolCB), 
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), atom_p, 
              DRW_ATOM_SIZE_LRG);
	    draw_Atom (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
              DrawTool_GC_Get (&GDrawToolCB), 
              SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), 
              atom_p, DRW_ATOM_SIZE_LRG);
	    delete_Atom (DrawMol_p, atom_p);
	    buble = TRUE;
	    }

	  ++atom_p;
          }
        } while (buble);

      unmark_selected (DrawMol_p, DrawDisplay, 
        DrawTool_Pixmap_Get (&GDrawToolCB), 
        XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), 
        DrawTool_GC_Get (&GDrawToolCB));
      DrawSelInfo->move_it = FALSE;

      if (tag == SMU_YES_RESPONSE) 
        {
        /* White out the drawing area */
        XSetForeground (DrawDisplay, DrawTool_GC_Get (&GDrawToolCB), 
          SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
        XFillRectangle (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
          DrawTool_GC_Get (&GDrawToolCB), 0, 0, DRW_DA_PXMP_WD, DRW_DA_PXMP_HT);
        XCopyArea (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
          XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), 
          DrawTool_GC_Get (&GDrawToolCB), 0, 0, DRW_DA_PXMP_WD, DRW_DA_PXMP_HT,
          0, 0);

        XUndefineCursor (XtDisplay (da_wid), XtWindow (da_wid));
        
        DrawFlags.drawing_mode = DRW_BOND_FLAG;
        Draw_Mode_Reset (DRW_BOND_FLAG);
        }
      break;
    case DRW_MENU_RETRACE:
      DrawFlags.retrace_mode = TRUE;
      /*wid = retrieve_widget_id ("options_menu_bondpl_mode_tb", 0);    
      XmToggleButtonSetState (wid, FALSE, FALSE);*/
      break;

    case DRW_MENU_BOND_PLUS:
      DrawFlags.retrace_mode = FALSE;
     /* wid = retrieve_widget_id ("options_menu_retrace_mode_tb", 0);
      XmToggleButtonSetState (wid, FALSE, FALSE);*/
      break;
    case DSP_BOND_DOUBLE:
      DrawFlags.dflt_bond = DSP_BOND_DOUBLE;
      break;
    
    case DSP_BOND_TRIPLE:
      DrawFlags.dflt_bond = DSP_BOND_TRIPLE;
      break;

   case DRW_MENU_EXIT:
      exit (0);

    case DRW_MENU_SUBMIT:
      exit (0);
    }

  return ;
}
/*  End of Menu_Choice_CB  */

/****************************************************************************
*  
*  Function Name:                 Selected_Bond_Draw_CB
*  
*****************************************************************************/  

void Selected_Bond_Draw_CB 
  (
  Widget           w,
  XtPointer        client_data,
  XtPointer        call_data             
  )
{
  int             tag;
  Dsp_Atom_t      *atom1_p, *atom2_p;

  tag = (int) client_data;

  if (tag > 10)
    /* Function was invoked by middle default buttons */
    DrawFlags.dflt_bond = tag - 10;

  else 
    {
    /* Function was invoked by "last drawn bond" buttons */
    if ((DrawBondsInfo->cur_bnd_p) &&
	(DrawFlags.drawing_mode == DRW_BOND_FLAG)) 
      {
      if (tag != DSP_BOND_ERASE) 
        {
	draw_Bond (DrawDisplay, 
          XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), 
          SynAppR_MolGC_Get (&GSynAppR),
          SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), DrawMol_p, 
          DrawBondsInfo->cur_bnd_p, FALSE, FALSE);
	draw_Bond (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
          SynAppR_MolGC_Get (&GSynAppR),
          SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), DrawMol_p, 
          DrawBondsInfo->cur_bnd_p, FALSE, FALSE);
	if ((tag == DSP_BOND_STEREO_DOWN)  
            || (tag == DSP_BOND_STEREO_OPP_DOWN) 
	    || (tag == DSP_BOND_STEREO_UP) 
	    || (tag == DSP_BOND_STEREO_OPP_UP)) 
          {
	  atom1_p = get_Atom_There (DrawMol_p, DrawBondsInfo->clicked_x,
            DrawBondsInfo->clicked_y);
	  DrawBondsInfo->cur_bnd_p->latm_p = 
	    (DrawBondsInfo->cur_bnd_p->ratm_p == atom1_p) ?
	      DrawBondsInfo->cur_bnd_p->latm_p :
	      DrawBondsInfo->cur_bnd_p->ratm_p;
	  DrawBondsInfo->cur_bnd_p->ratm_p = atom1_p;
	  DrawBondsInfo->cur_bnd_p->stereo = tag;
	  }
	else 
          {
	  DrawBondsInfo->cur_bnd_p->nlines = tag;
	  DrawBondsInfo->cur_bnd_p->stereo = DSP_BOND_STEREO_NONE;
          }

	draw_Bond (DrawDisplay, 
          XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), 
          SynAppR_MolGC_Get (&GSynAppR), 
          SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), DrawMol_p, 
          DrawBondsInfo->cur_bnd_p, TRUE, TRUE);
	draw_Bond (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
          SynAppR_MolGC_Get (&GSynAppR),
          SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_BLACK), DrawMol_p, 
          DrawBondsInfo->cur_bnd_p, TRUE, TRUE);
        }
      else 
        {
	atom1_p = get_Atom_There (DrawMol_p, DrawBondsInfo->clicked_x,
          DrawBondsInfo->clicked_y);
	atom2_p = (atom1_p == DrawBondsInfo->cur_bnd_p->latm_p) ?
	  DrawBondsInfo->cur_bnd_p->ratm_p : DrawBondsInfo->cur_bnd_p->latm_p;

	if ((atom1_p->adj_info.num_neighbs == 1) &&
	    (atom2_p->adj_info.num_neighbs == 1)) 
          {
	  DrawBondsInfo->esc_mode = TRUE;
	  } 
	else if (!((atom1_p->adj_info.num_neighbs >  1) &&
		   (atom2_p->adj_info.num_neighbs == 1))) 
          {
	  DrawBondsInfo->clicked_x = atom2_p->x;
	  DrawBondsInfo->clicked_y = atom2_p->y;
	  }

	draw_Bond (DrawDisplay, 
          XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), 
          SynAppR_MolGC_Get (&GSynAppR), 
          SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), DrawMol_p, 
          DrawBondsInfo->cur_bnd_p, TRUE, TRUE);
	erase_Bond (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
          SynAppR_MolGC_Get (&GSynAppR), DrawMol_p, 
          DrawBondsInfo->cur_bnd_p);
	DrawBondsInfo->cur_bnd_p = (Dsp_Bond_t *) NULL;

        } /* End of DSP_BOND_ERASE */
      }
    } /* End of else */

  XCopyArea (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
    XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), 
    SynAppR_MolGC_Get (&GSynAppR), 0, 0, DRW_DA_PXMP_WD, DRW_DA_PXMP_HT, 0, 0);

  return ;
}
/*  End of Selected_Bond_Draw_CB  */

/****************************************************************************
*  
*  Function Name:                 Done_CB
*  
*****************************************************************************/  

void Done_CB 
  (
  Widget           w,
  XtPointer        client_data,
  XtPointer        call_data             
  )
{
  Dsp_Molecule_t  *mol_p;
  JobInfoCB_t     *jobinfo_p;

  if (done_pb_function != NULL)
  {
    (*done_pb_function) (w, NULL, call_data);
    return;
  }

  jobinfo_p = (JobInfoCB_t *) client_data;
  mol_p = DrawMol_p;

  if (mol_p == NULL)
    return;

  JobInfo_SlingTxt_Update (jobinfo_p, mol_p);
  Draw_Mode_Reset (DRW_BOND_FLAG);

  return;
}
/* End of Done_CB */

/****************************************************************************
*  
*  Function Name:                 MOLDRAW_StereoChem_CB
*  
*****************************************************************************/  

void MOLDRAW_StereoChem_CB
 (
  Widget           w,
  XtPointer        client_data,
  XtPointer        call_data             
  )
{
 
  return;
}
/*  End of MOLDRAW_StereoChem_CB  */

/****************************************************************************
*  
*  Function Name:                 New_Dsp2Xtr
*  
*****************************************************************************/  

Xtr_t *New_Dsp2Xtr (Dsp_Molecule_t *mol_p)
{
  Xtr_t            *xtr_p;
  Dsp_Atom_t       *atom_p, *adjatm_p;
  Dsp_Bond_t       *bond_p;
  int               atm_i, ii;
  char              sym[4];

  /*xtr_p = (Xtr_t *) malloc (XTRSIZE);*/
  xtr_p = Xtr_Create (mol_p->natms);

  /* Transform atom relative info first */
  Xtr_NumAtoms_Put (xtr_p, mol_p->natms);
  xtr_p->attributes = (XtrRow_t *) malloc (XTRROWSIZE * mol_p->natms);

  atom_p = mol_p->atoms;
  for (atm_i = 0; atm_i < mol_p->natms; atm_i++) 
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
      adjatm_p = mol_p->atoms + (int) atom_p->adj_info.adj_atms[ii];
      bond_p = get_Bond_There (mol_p, atom_p->x, atom_p->y,
        adjatm_p->x, adjatm_p->y);

      /* Convert multiplicity of bond */
      switch (bond_p->nlines) 
        {
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
/*  End of New_Dsp2Xtr  */

/****************************************************************************
*  
*  Function Name:                 MolDraw_Sling_Draw
*  
*****************************************************************************/  

void MolDraw_Sling_Draw 
  (
  Sling_t sling
  )
{
  U16_t           num_atoms;                 /* Number of atoms found */
  Dimension       sw_h, sw_w;
  Widget          da_wid;
  Xtr_t           *xtr_p;
  Dsp_Molecule_t  *old_mol_p;

  if (!Sling_Validate (sling, &num_atoms))
    {
    InfoWarn_Show ("Invalid sling.");
    return;
    }

  xtr_p = Sling2Xtr (Sling2CanonSling(sling));
  old_mol_p = DrawMol_p;
  DrawMol_p = Xtr2Dsp (xtr_p);

  if (old_mol_p != NULL)
    free_Molecule (old_mol_p);

  da_wid = DrawTool_DrawArea_Get(&GDrawToolCB);
   
  if (dsp_Shelley (DrawMol_p)) 
    {
    XUndefineCursor (XtDisplay (da_wid), XtWindow (da_wid));
        
    XSetForeground (DrawDisplay, DrawTool_GC_Get (&GDrawToolCB), 
      SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));

    XFillRectangle (DrawDisplay, 
      XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), 
      DrawTool_GC_Get (&GDrawToolCB), 
      0, 0, DRW_DA_PXMP_WD, DRW_DA_PXMP_HT);
    XFillRectangle (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
      DrawTool_GC_Get (&GDrawToolCB), 0, 0, DRW_DA_PXMP_WD, DRW_DA_PXMP_HT);

    XSetForeground (DrawDisplay, DrawTool_GC_Get (&GDrawToolCB), 
      SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));

    XtVaGetValues (DrawTool_ScrlldWin_Get (&GDrawToolCB),
      XmNwidth, &sw_w,
      XmNheight, &sw_h,
      NULL);

    draw_Molecule (DrawDisplay, 
      XtWindow (DrawTool_DrawArea_Get (&GDrawToolCB)), 
      DrawTool_GC_Get (&GDrawToolCB), 
      DrawMol_p, AppDim_MolScale_Get (&GAppDim), sw_h, sw_w, TRUE);
    draw_Molecule (DrawDisplay, DrawTool_Pixmap_Get (&GDrawToolCB), 
      DrawTool_GC_Get (&GDrawToolCB), DrawMol_p, 1.0, 0, 0, FALSE);

    Draw_Mode_Reset (DRW_BOND_FLAG);
    }

    /* If Shelley code could not handle this transformation,
       popup error dialog.
    */
    else
      InfoWarn_Show ("kka:Unable to draw molecule (Shelley module)."); 

   return;
}
/*  End of MolDraw_Sling_Draw  */


/****************************************************************************
*  
*  Function Name:                 Dsp2Tsd
*  
*****************************************************************************/  

Tsd_t *Dsp2Tsd 
  (
  Dsp_Molecule_t *mol_p
  )
{
  Tsd_t            *tsd_p;
  Dsp_Atom_t       *atom_p, *adjatm_p;
  Dsp_Bond_t       *bond_p;
  int               atm_i, ii;
  char              sym[8];

  tsd_p = Tsd_Create (mol_p->natms);

  /* Transform atom relative info first */
  atom_p = mol_p->atoms;
  for (atm_i = 0; atm_i < mol_p->natms; atm_i++) 
    {
    /* Convert atom symbol in dsp format to atom id */
    strcpy (sym, atom_p->sym);
    if (strcmp (atom_p->sym, " I ") == 0)
      strcpy (sym, "I");      
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
      adjatm_p = mol_p->atoms + (int) atom_p->adj_info.adj_atms[ii];
      bond_p = get_Bond_There (mol_p, atom_p->x, atom_p->y,
        adjatm_p->x, adjatm_p->y);

      /* Convert multiplicity of bond */
      switch (bond_p->nlines) 
        {
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
        default:
          Tsd_Atom_NeighborBond_Put (tsd_p, atm_i, ii, BOND_SINGLE);
        }

      } /* End for all neighbors */

    atom_p++;
    } /* End for all atoms */

  return (tsd_p);
}
/*  End of Dsp2Tsd  */

/****************************************************************************
*  
*  Function Name:                 Sling2CanonSling
*  
*****************************************************************************/  

Sling_t Sling2CanonSling 
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
/*  End of Sling2CanonSling  */

/****************************************************************************
*  
*  Function Name:                 Atom_Copy
*  
*****************************************************************************/  

void Atom_Copy 
  (
  Dsp_Atom_t  *src_atom,
  Dsp_Atom_t  *dest_atom
  )
{
  int i;

  if (src_atom != NULL)
    return;

  strcpy (dest_atom->sym, src_atom->sym);
  strcpy (dest_atom->map, src_atom->map);
  strcpy (dest_atom->chg, src_atom->chg);
  dest_atom->x = src_atom->x;
  dest_atom->y = src_atom->y;
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
/*  End of Atom_Copy  */

/****************************************************************************
*  
*  Function Name:                 Molecule_Double
*  
*****************************************************************************/  

Dsp_Molecule_t *Molecule_Double 
   (
   Dsp_Molecule_t *mol_p
   )
{
  Dsp_Molecule_t  *doubled_mol_p;


  /* Allocate memory for molecule data structure */
  doubled_mol_p = (Dsp_Molecule_t *) malloc (DSP_MOLECULE_SIZE);
 
   /* Allocate double the memory for atoms and bonds */
  doubled_mol_p->atoms = (Dsp_Atom_t *) malloc (DSP_ATOM_SIZE * 
    mol_p->nallocatms * 2);
  doubled_mol_p->bonds = (Dsp_Bond_t *) malloc (DSP_BOND_SIZE * 
    mol_p->nallocbnds * 2);

  copy_Molecule (doubled_mol_p, mol_p);
  
  doubled_mol_p->nallocatms = mol_p->nallocatms * 2;
  doubled_mol_p->nallocbnds = mol_p->nallocbnds * 2;

  free_Molecule (mol_p);
  
  return doubled_mol_p;
}
/*  End of Molecule_Double  */

/****************************************************************************
*  
*  Function Name:                 DrawTool_Destroy
*  
*****************************************************************************/  

void DrawTool_Destroy 
  (
  void
  )
{
  XFreeGC (XtDisplay (DrawTool_BondForm_Get (&GDrawToolCB)), DrawTool_GC_Get 
    (&GDrawToolCB));
  
  free_Molecule (DrawMol_p);

  return;
}
/*  End of DrawTool_Destroy  */
  
/****************************************************************************
*
*  Function Name:                 store_AtomNew (used to be store_Atom)
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
*    mol_p - pointer to DSP data structure
*    sym   - atom symbol
*    chg   - atom charge
*    map   - not used
*    x     - atom's x-coordinate
*    y     - atom's y-coordinate
*
*  Return Values:
*
*    store_AtomNew returns a pointer to a new atom that has been stored.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Dsp_Atom_t  *store_AtomNew (Dsp_Molecule_t  *mol_p,
			 char *sym, char *chg, char *map,
			 int x, int y)
{
  Dsp_Atom_t      *dratm_p;

  
  /* If num of atoms exceeds num of atom data structures allocated
     in memory, double the memory allocated to molecule
   */
  if (mol_p->natms == mol_p->nallocatms) 
    {
    mol_p = Molecule_Double (mol_p);
    DrawMol_p = mol_p;
    }

  /* If atom already exists, just return pointer to it 
   */
  dratm_p = get_Atom_There (mol_p, x, y);
  if (dratm_p != NULL)
    return (dratm_p);
      
  strcpy ((mol_p->atoms + mol_p->natms)->sym, sym);
  strcpy ((mol_p->atoms + mol_p->natms)->chg, chg);
  strcpy ((mol_p->atoms + mol_p->natms)->map, map);

  (mol_p->atoms + mol_p->natms)->x = x;
  (mol_p->atoms + mol_p->natms)->y = y;

  if (strcmp (sym, "C") == 0)
    (mol_p->atoms + mol_p->natms)->isC = TRUE;
  else
    (mol_p->atoms + mol_p->natms)->isC = FALSE;

  (mol_p->atoms + mol_p->natms)->isSelected = DRW_SELECT_NONE;
  (mol_p->atoms + mol_p->natms)->adj_info.num_neighbs = 0;

  mol_p->natms++;
  return (mol_p->atoms + mol_p->natms - 1);
}
/*  End of store_AtomNew  */

  
/****************************************************************************
*
*  Function Name:                 store_BondNew (used to be store_Bond)
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
*    store_BondNew returns a pointer to a new bond that has been stored.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
 Dsp_Bond_t *store_BondNew (Dsp_Molecule_t  *mol_p,
			       int x1, int y1,
			       int x2, int y2,
			       int nlines)
{
  int             adj_i, adj_j;
  Dsp_Atom_t      *statm1_p, *statm2_p;


  /* If num of bonds exceeds num of bond data structures allocated
     in memory, then double it.
   */
  if (mol_p->nbnds == mol_p->nallocbnds) 
    {
    mol_p = Molecule_Double (mol_p);
    DrawMol_p = mol_p;
    }

  statm1_p = get_Atom_There (mol_p, x1, y1);

  /* If the bond to be stored exceed max number of bonds
     that could be attached to one atom, return NULL.
   */
  if (statm1_p->adj_info.num_neighbs == 6)
    return ((Dsp_Bond_t *) NULL);

  statm2_p = get_Atom_There (mol_p, x2, y2);

if (statm1_p == statm2_p)
{
  InfoWarn_Show ("Oops!  Attempt to bond atom to itself!");\
  return ((Dsp_Bond_t *) NULL);
}

  /* If the bond to be stored exceed max number of bonds
     that could be attached to one atom, return NULL.
   */
  if (statm2_p->adj_info.num_neighbs == 6)
    return ((Dsp_Bond_t *) NULL);

  /* Find number of "left" atom that is adjacent to the bond to be stored 
     in "mol_p" strusture */
  for (adj_i = 0; adj_i < mol_p->natms; adj_i++)
    if (((mol_p->atoms + adj_i)->x == statm1_p->x) &&
	((mol_p->atoms + adj_i)->y == statm1_p->y))
      break;

  /* Find number of "right" atom that is adjacent to the bond to be stored
     in "mol_p" strusture */
  for (adj_j = 0; adj_j < mol_p->natms; adj_j++)
    if (((mol_p->atoms + adj_j)->x == statm2_p->x) &&
	((mol_p->atoms + adj_j)->y == statm2_p->y))
      break;

  statm1_p->adj_info.adj_atms[statm1_p->adj_info.num_neighbs] = (long) adj_j;
  statm1_p->adj_info.num_neighbs++;
  statm2_p->adj_info.adj_atms[statm2_p->adj_info.num_neighbs] = (long) adj_i;
  statm2_p->adj_info.num_neighbs++;


  /* Store pointers to the adjacent atoms of the bond. */
  (mol_p->bonds + mol_p->nbnds)->latm_p = statm1_p;
  (mol_p->bonds + mol_p->nbnds)->ratm_p = statm2_p;
  (mol_p->bonds + mol_p->nbnds)->nlines = nlines;
  (mol_p->bonds + mol_p->nbnds)->isSelected = DRW_SELECT_NONE;
  (mol_p->bonds + mol_p->nbnds)->stereo = DSP_BOND_STEREO_NONE;
  mol_p->nbnds++;

  return (mol_p->bonds + mol_p->nbnds - 1);
}
/*  End of store_BondNew  */

/****************************************************************************
*
*  Function Name:                 draw_Double_Bond
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
void draw_Double_Bond 
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

  norm_x *= DRW_DBL_BOND_OFFSET;
  norm_y *= DRW_DBL_BOND_OFFSET;

  x0 = (int) norm_x;
  y0 = (int) norm_y;

  XDrawLine (disp, drbl, gc, x1 + x0, y1 - y0, x2 + x0, y2 - y0);
  XDrawLine (disp, drbl, gc, x1 - x0, y1 + y0, x2 - x0, y2 + y0);

  return ;
}
/*  End of draw_Double_Bond  */

/****************************************************************************
*
*  Function Name:                 draw_Stereo_Bond
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
void draw_Stereo_Bond 
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

  norm_x *= DRW_STR_BOND_OFFSET;
  norm_y *= DRW_STR_BOND_OFFSET;

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
/*  End of draw_Stereo_Bond  */


/****************************************************************************
*
*  Function Name:                 draw_Bond
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
void draw_Bond 
  (
  Display         *disp, 
  Drawable         drbl, 
  GC               gc,
  XColor           color,
  Dsp_Molecule_t  *mol_p,
  Dsp_Bond_t      *bond_p,
  Boolean_t        redraw_atom1,
  Boolean_t        redraw_atom2
  )
{
 
  XSetForeground (disp, gc, color.pixel);
  XSetFillStyle (disp, gc, FillSolid);
  if (bond_p->stereo != DSP_BOND_STEREO_NONE)
    draw_Stereo_Bond (disp, drbl, gc, bond_p->stereo, bond_p->latm_p->x, 
      bond_p->latm_p->y, bond_p->ratm_p->x, bond_p->ratm_p->y);
  else 
    {
    switch (bond_p->nlines) 
      {
      case DSP_BOND_SINGLE:
        XDrawLine (disp, drbl, gc, bond_p->latm_p->x, bond_p->latm_p->y,
          bond_p->ratm_p->x, bond_p->ratm_p->y);
        break;
      case DSP_BOND_TRIPLE:
        XDrawLine (disp, drbl, gc, bond_p->latm_p->x, bond_p->latm_p->y,
          bond_p->ratm_p->x, bond_p->ratm_p->y);
      case DSP_BOND_DOUBLE:	  
        draw_Double_Bond (disp, drbl, gc, bond_p->latm_p->x, bond_p->latm_p->y,
          bond_p->ratm_p->x, bond_p->ratm_p->y);
        break;
      case DSP_BOND_RESONANT:
        XSetLineAttributes (disp, gc, 0, LineOnOffDash, CapButt, JoinRound);
        draw_Double_Bond (disp, drbl, gc, bond_p->latm_p->x, bond_p->latm_p->y,
          bond_p->ratm_p->x, bond_p->ratm_p->y);
        break;
      case DSP_BOND_VARIABLE:
        XSetLineAttributes (disp, gc, 0, LineOnOffDash, CapButt, JoinRound);
        XDrawLine (disp, drbl, gc, bond_p->latm_p->x, bond_p->latm_p->y,
          bond_p->ratm_p->x, bond_p->ratm_p->y);
        break;
      } /* End of switch (bond num lines) */
    } /* End of else (if stereo is NONE) */

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
  XSetLineAttributes (disp, gc, 0, LineSolid, CapButt, JoinRound);

  /* If nessesary, redraw atoms on both sides of the bond */
  if (redraw_atom1)
    draw_Atom (disp, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
      SAR_CLRI_BLACK), bond_p->latm_p, DRW_ATOM_SIZE_LRG);

  if (redraw_atom2)
    draw_Atom (disp, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
      SAR_CLRI_BLACK), bond_p->ratm_p, DRW_ATOM_SIZE_LRG);
  
  return ;
}
/*  End of draw_Bond  */


/****************************************************************************
*
*  Function Name:                 draw_Atom
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
*    drbl  - X11 Drawable where the drawings take place; could be
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
void draw_Atom 
  (
  Display     *disp, 
  Drawable     drbl, 
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
  XmString       chg_cs;                /* Xm string for charge */
  Dimension      aw, ah;                /* entire atom's width & height */
  Dimension      cw, ch;                /* charge width & height */
  Dimension      mw, mh;                /* mapping width & height */
  Dimension      sw, sh;                /* symbol width & height */
  int            ax, ay;                /* entire atom's coords */
  int            cx, cy;                /* charge coords */
  int            mx, my;                /* mapping coords */
  int            sx, sy;                /* symbol coords */

  XSetForeground (disp, gc, color.pixel);

  /* Calculate atom-sym and atom-chg sizes depending on required size
     of entire atom image
   */
  switch (size) 
    {
    case DRW_ATOM_SIZE_LRG:
      if (dratm_p->isC) 
        {
        sw = 0;
        sh = 0;
        }
      else 
        {
        symfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_LRG);
        sym_cs = XmStringCreateLtoR (dratm_p->sym, SAR_FONTTAG_ATM_LRG);
        XmStringExtent (SynAppR_FontList_Get (&GSynAppR), sym_cs, &sw, &sh);
        }

      if (dratm_p->chg[0] == '\0') 
        {
        cw = 0;
        ch = 0;
        }
      else 
        {
        if (dratm_p->isC) 
          {
	  chgfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_LRG);
	  chg_cs = XmStringCreateLtoR (dratm_p->chg, SAR_FONTTAG_ATM_LRG);
          }
        else 
          {
	  chgfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_NRML);
	  chg_cs = XmStringCreateLtoR (dratm_p->chg, SAR_FONTTAG_ATM_NML);
          }

        XmStringExtent (SynAppR_FontList_Get (&GSynAppR), chg_cs, &cw, &ch);
        }

      if (dratm_p->map[0] == '\0' /*|| !mol_p->map_em*/ ) 
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

    case DRW_ATOM_SIZE_NML:
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
	  sym_cs = XmStringCreateLtoR (dratm_p->sym, SAR_FONTTAG_ATM_LRG);
          }
        else 
          {
	  symfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_NRML);
	  sym_cs = XmStringCreateLtoR (dratm_p->sym, SAR_FONTTAG_ATM_NML);
          }

        XmStringExtent (SynAppR_FontList_Get (&GSynAppR), sym_cs, &sw, &sh);
        }

      if (dratm_p->chg[0] == '\0') 
        {
        cw = 0;
        ch = 0;
        }
      else 
        {
        if (dratm_p->isC) 
          {
	  chgfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_NRML);
	  chg_cs = XmStringCreateLtoR (dratm_p->chg, SAR_FONTTAG_ATM_NML);
          }
        else 
          {
	  chgfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_SML);
	  chg_cs = XmStringCreateLtoR (dratm_p->chg, SAR_FONTTAG_ATM_SML);
          }

        XmStringExtent (SynAppR_FontList_Get (&GSynAppR), chg_cs, &cw, &ch);
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

    case DRW_ATOM_SIZE_SML:
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
	  sym_cs = XmStringCreateLtoR (dratm_p->sym, SAR_FONTTAG_ATM_NML);
          }
        else 
          {
	  symfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_SML);
	  sym_cs = XmStringCreateLtoR (dratm_p->sym, SAR_FONTTAG_ATM_SML);
          }

        XmStringExtent (SynAppR_FontList_Get (&GSynAppR), sym_cs, &sw, &sh);
        }

      if (dratm_p->chg[0] == '\0') 
        {
        cw = 0;
        ch = 0;
        }
      else 
        {
        chgfnt_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_SML);
        chg_cs = XmStringCreateLtoR (dratm_p->chg, SAR_FONTTAG_ATM_SML);
        XmStringExtent (SynAppR_FontList_Get (&GSynAppR), chg_cs, &cw, &ch);
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

  ah = sh + (ch >> 1) + (mh >> 1);
  aw = sw + ((cw > mw) ? cw : mw);
  ax = dratm_p->x - (aw >> 1);
  ay = dratm_p->y + (ah >> 1);
  sx = ax;
  sy = ay - (mh >> 1);
  cx = ax + sw;
  cy = ay - (ah >> 1);
  mx = ax + sw;
  my = ay;

  if (!dratm_p->isC) 
    {
    XSetFont (disp, gc, symfnt_p->fid);
    XDrawImageString (disp, drbl, gc, sx, sy, dratm_p->sym, 
      strlen (dratm_p->sym));    
    XmStringFree (sym_cs);
    }

  if (dratm_p->chg[0] != '\0') 
    {
    XSetFont (disp, gc, chgfnt_p->fid);
    XDrawImageString (disp, drbl, gc, cx, cy, dratm_p->chg, 
      strlen (dratm_p->chg));
    XmStringFree (chg_cs);
    }

  if ( /*mol_p->map_em && */ dratm_p->map[0] != '\0') 
    {
    XDrawImageString (disp, drbl, gc, mx, my, dratm_p->sym, 
      strlen (dratm_p->map));
    XmStringFree (map_cs);
    }

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
  return ;
}
/*  End of draw_Atom  */


/****************************************************************************
*
*  Function Name:                 erase_Bond
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
void erase_Bond 
  (
  Display         *disp, 
  Drawable         drbl, 
  GC               gc,
  Dsp_Molecule_t  *mol_p,
  Dsp_Bond_t      *erbnd_p
  )
{
  Dsp_Atom_t      *left_p, *right_p;
  int              atms_to_del = 0;

  left_p = erbnd_p->latm_p;
  right_p = erbnd_p->ratm_p;
  
  draw_Bond (disp, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_WHITE), 
    mol_p, erbnd_p, TRUE, TRUE);

  /* Delete this bond from mol data structure */
  delete_Bond (mol_p, erbnd_p);

  /* If an adjacent atoms don't have neighbors and are Carbons,
     delete them.
   */
  if (left_p->adj_info.num_neighbs == 0 && left_p->isC
      && strcmp (left_p->chg, "\0") == 0)
    atms_to_del += 1;

  if (right_p->adj_info.num_neighbs == 0 && right_p->isC 
      && strcmp (right_p->chg, "\0") == 0)
    atms_to_del += 2;

  switch (atms_to_del) 
    {
    case 1:
      delete_Atom (mol_p, left_p);
      break;

    case 2:
      delete_Atom (mol_p, right_p);
      break;

    case 3:
      if (left_p == (mol_p->atoms + mol_p->natms)) 
        {
        delete_Atom (mol_p, right_p);
        delete_Atom (mol_p, right_p);
        }
      else if (right_p == (mol_p->atoms + mol_p->natms)) 
        {
        delete_Atom (mol_p, left_p);
        delete_Atom (mol_p, left_p);
        }
      else 
        {
        delete_Atom (mol_p, right_p);
        delete_Atom (mol_p, left_p);
        }
      break;

    default:
      break;
    } /* End of switch */    

  return;
}
/*  End of erase_Bond  */

/****************************************************************************
*
*  Function Name:                 draw_Molecule
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
void draw_Molecule 
  (
  Display         *disp, 
  Drawable         drbl, 
  GC               gc,
  Dsp_Molecule_t  *mol_p,
  float            sc_factor,
  Dimension        height,
  Dimension        width,
  Boolean_t        to_center
  )
{
  Dsp_Atom_t    *atom_p;
  Dsp_Bond_t    *bond_p;
  int            maxx, maxy, minx, miny;
  int            atom_size;
  int            h_RPDeltaDAW, h_RPDeltaDAH;
  int            atm_i, bnd_i;

  if (sc_factor < 0) 
    {
    fprintf (stderr, "draw_Molecule : Negative sc factor\n");
    exit (-1);
    }

  /* Calculate actual width and height */
  width  -= AppDim_AtmMaxW_Get (&GAppDim);
  height -= AppDim_AtmMaxH_Get (&GAppDim);
  if (sc_factor != 1.00) 
    {
    /* Calculate atom xy-coordinates according to scaling factor */
    atom_p = mol_p->atoms;
    for (atm_i = 0; atm_i < mol_p->natms; atm_i++) 
      {
      atom_p->x = (int) atom_p->x * sc_factor;
      atom_p->y = (int) atom_p->y * sc_factor;
      ++atom_p;
      }
    }

  /* If molecule needs to be centered, calculate max xy coordinates */
  if (to_center) 
    {
    maxx = maxy = 0;
    h_RPDeltaDAW = AppDim_AtmMaxW_Get (&GAppDim) >> 1;
    h_RPDeltaDAH = AppDim_AtmMaxH_Get (&GAppDim) >> 1;
    atom_p = mol_p->atoms;
    minx = maxx = atom_p->x;
    miny = maxy = atom_p->y;
    for (atm_i = 0; atm_i < mol_p->natms; atm_i++) 
      {
      if (minx > atom_p->x) minx = atom_p->x;
      if (maxx < atom_p->x) maxx = atom_p->x;
      if (miny > atom_p->y) miny = atom_p->y;
      if (maxy < atom_p->y) maxy = atom_p->y;
      ++atom_p;
      }

    maxx += h_RPDeltaDAW;
    maxy += h_RPDeltaDAH;
    maxx = ((maxx < (int) width) && ((width  - maxx) >> 1 > h_RPDeltaDAW)) 
      ? ((int) width - maxx) >> 1 : h_RPDeltaDAW;
    maxy = ((maxy < (int)height) && ((height - maxy) >> 1 > h_RPDeltaDAH)) 
      ? ((int)height - maxy) >> 1 : h_RPDeltaDAH;

    /* Recalculate atom xy-coordinates to center the molecule */
    atom_p = mol_p->atoms;
    for (atm_i = 0; atm_i < mol_p->natms; atm_i++) 
      {
      atom_p->x += maxx;
      atom_p->y += maxy;
      ++atom_p;
      }
    }

  /*  Draw the bonds of the molecule first. */
  bond_p = mol_p->bonds;
  for (bnd_i = 0; bnd_i < mol_p->nbnds; bnd_i++) 
    {
    draw_Bond (disp, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
      SAR_CLRI_BLACK), mol_p, bond_p, FALSE, FALSE);
    ++bond_p;
    }

  /* Set atom size depending on a scaling factor (since he uses large
     everywhere else, use it here as well).  - DK
  if (mol_p->scale < DSP_DELTA_NORM_MIN && mol_p->scale != 1.0)
    atom_size = DRW_ATOM_SIZE_LRG;
  else if (mol_p->scale < DSP_DELTA_NORM_MAX)
    atom_size = DRW_ATOM_SIZE_NML;
  else
    atom_size = DRW_ATOM_SIZE_SML;
   */

  atom_size = DRW_ATOM_SIZE_LRG;

  /*  Draw the atoms of the molecule. */
  atom_p = mol_p->atoms;
  for (atm_i = 0; atm_i < mol_p->natms; atm_i++) 
    {
    /* If atom has no neighbors and is Carbon, draw its symbol */
    if ((atom_p->adj_info.num_neighbs == 0) 
        && (strcmp (atom_p->sym, "C") == 0))
      atom_p->isC = FALSE;

    draw_Atom (disp, drbl, gc, SynAppR_IthColor_Get (&GSynAppR, 
      SAR_CLRI_BLACK), atom_p, atom_size);
    ++atom_p;
    }
  
  return ;
}
/*  End of draw_Molecule  */

void Modify_Done_Lbl (char *label)
{
  XmString lbl;

  lbl = XmStringCreateLocalized (label);
  XtVaSetValues (DrawTool_DonePB_Get (&GDrawToolCB),
    XmNlabelString, lbl,
    NULL);
  XmStringFree (lbl);
}

/*  End of SUBMIT_DRAW.C  */
