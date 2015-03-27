/****************************************************************************
*  
* Copyright (C) 1997 Synchem Group at SUNY-Stony Brook, Daren Krebsbach  
*  
*  
*  Module Name:               SUBMIT_MENU.C
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
*    DirsForm_Create
*    DirsForm_Values_Get
*    DirsForm_Values_Set
*    FilesForm_Create
*    FilesForm_Values_Get
*    FilesForm_Values_Set
*    StratsForm_Create
*    StratsForm_Values_Get
*    StratsForm_Values_Set
*    SubmitMenu_Create
*    SubmitMenu_Destroy
*    SubmitMenu_Dialogs_PopDown
*    SubmitMenu_Forms_Get
*    SubmitMenu_Forms_Set
*
*    DirsForm_Cancel_CB
*    DirsForm_Reset_CB
*    FilesForm_Cancel_CB
*    FilesForm_Reset_CB
*    StratsForm_Cancel_CB
*    StratsForm_Reset_CB
*    SubmitMenu_FSave_CB
*    SubmitMenu_FSbmt_CB
*    SubmitMenu_FStat_CB
*    SubmitMenu_Help_CB
*    SubmitMenu_ODirs_CB
*    SubmitMenu_OFiles_CB
*    SubmitMenu_OStrats_CB
*
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
*
*******************************************************************************/  
#include <stdlib.h> 
#include <string.h> 
#include <Xm/CascadeB.h>  
#include <Xm/CascadeBG.h>  
#include <Xm/PushB.h>  
#include <Xm/PushBG.h>  
#include <Xm/Form.h>  
#include <Xm/RowColumn.h>  
#include <Xm/Text.h> 
#include <Xm/Label.h>  
#include <Xm/LabelG.h>  
#include <Xm/ToggleB.h>  
#include <Xm/MessageB.h>  
#include <Xm/ToggleB.h>  
#include <Xm/Separator.h>
/*   
#include <Xm/DialogS.h>
*/

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
 
#ifndef _H_SV_FILES_
#include "sv_files.h"
#endif

#ifndef _H_INFO_VIEW_
#include "info_view.h"
#endif

#ifndef _H_SYN_VIEW_
#include "syn_view.h"
#endif

#ifndef _H_SUBMIT_
#include "submit.h" 
#endif

#ifndef _H_SUBMIT_MENU_
#include "submit_menu.h" 
#endif


/* Callback Routine Prototypes */  
  
static void   DirsForm_Cancel_CB    (Widget, XtPointer, XtPointer);
static void   DirsForm_Reset_CB     (Widget, XtPointer, XtPointer);
static void   FilesForm_Cancel_CB   (Widget, XtPointer, XtPointer);
static void   FilesForm_Reset_CB    (Widget, XtPointer, XtPointer);
static void   StratsForm_Cancel_CB  (Widget, XtPointer, XtPointer);
static void   StratsForm_Reset_CB   (Widget, XtPointer, XtPointer);
/* static */ void   SubmitMenu_FSave_CB   (Widget, XtPointer, XtPointer);  /* abuse of "static" on routines useful elsewhere! */
static void   SubmitMenu_FSbmt_CB   (Widget, XtPointer, XtPointer);  
static void   SubmitMenu_FStat_CB   (Widget, XtPointer, XtPointer);  
static void   SubmitMenu_ODirs_CB   (Widget, XtPointer, XtPointer);  
static void   SubmitMenu_OFiles_CB  (Widget, XtPointer, XtPointer);  
static void   SubmitMenu_OStrats_CB (Widget, XtPointer, XtPointer);  


static SubmitMenuCB_t  SSubMenuCB;

/****************************************************************************
*  
*  Function Name:                 DirsForm_Create
*  
*****************************************************************************/  

void DirsForm_Create  
  (  
  DirCB_t       *dircb_p,
  Rcb_t         *status_rcb_p,
  Widget         topform 
  )  
{  
  XmString       lbl_ok, lbl_reset, lbl_cancel, title, lbl_str;  
  Widget         form/*, dialog*/;

  lbl_ok = XmStringCreateLocalized (SMU_OKAYPB_LBL);  
  lbl_reset = XmStringCreateLocalized (SMU_RESETPB_LBL);  
  lbl_cancel = XmStringCreateLocalized (SMU_CANCELPB_LBL);  
  title = XmStringCreateLocalized (SMU_DIRS_DIALOG_TITLE);  
/*
dialog=XmCreateDialogShell(topform,"DirsDgSh",NULL,0);
*/
  DirCB_MsgDg_Put (dircb_p, 
    XmCreateMessageDialog (/*dialog*/topform, "DirsMDg", NULL, 0));

  XtVaSetValues (DirCB_MsgDg_Get (dircb_p), 
    XmNdialogTitle,        title,
    XmNokLabelString,      lbl_ok,  
    XmNcancelLabelString,  lbl_cancel,   
    XmNhelpLabelString,    lbl_reset,   
    XmNdialogType,         XmDIALOG_TEMPLATE,  
    XmNlabelFontList,      SynAppR_FontList_Get (&GSynAppR),
    XmNbuttonFontList,     SynAppR_FontList_Get (&GSynAppR),
    XmNtextFontList,       SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,       AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,        AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,          True,
    NULL);  

  XmStringFree (lbl_ok);  
  XmStringFree (lbl_reset);  
  XmStringFree (lbl_cancel);  
  XmStringFree (title);  
   
  form = XtVaCreateWidget ("DirsFm", 
    xmFormWidgetClass,  DirCB_MsgDg_Get (dircb_p), 
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       True,
    NULL);
  DirCB_Form_Put (dircb_p, form);

  lbl_str = XmStringCreateLocalized (SMU_AVAIL_DIR_LBL);  
  DirCB_AvailLbl_Put (dircb_p,
    XtVaCreateManagedWidget ("DirsLbl",   
      xmLabelWidgetClass, form, 
      XmNlabelType, XmSTRING,  
      XmNlabelString, lbl_str,   
      NULL));  
  XmStringFree (lbl_str);

  DirCB_AvailBuf_Get (dircb_p)[0] = '\0';
  DirCB_AvailTxt_Put (dircb_p,
    XtVaCreateManagedWidget("DirsTxt",   
      xmTextWidgetClass, form, 
      XmNeditMode,       XmSINGLE_LINE_EDIT,
      XmNmaxLength,      SMU_DIRBUF_MAXLEN,
      XmNcolumns,        SMU_DIRTXT_MAXLEN, 
      XmNmarginHeight,   AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,    AppDim_MargLblPB_Get (&GAppDim),
      XmNvalue,          DirCB_AvailBuf_Get (dircb_p),
      NULL));  

  lbl_str = XmStringCreateLocalized (SMU_FUNGP_DIR_LBL);  
  DirCB_FunGpLbl_Put (dircb_p,
    XtVaCreateManagedWidget ("DirsLbl",   
      xmLabelWidgetClass, form, 
      XmNlabelType, XmSTRING,  
      XmNlabelString, lbl_str,   
      NULL));  
  XmStringFree (lbl_str);

  DirCB_FunGpBuf_Get (dircb_p)[0] = '\0';
  DirCB_FunGpTxt_Put (dircb_p,
    XtVaCreateManagedWidget ("DirsTxt",   
      xmTextWidgetClass, form, 
      XmNeditMode,       XmSINGLE_LINE_EDIT,
      XmNmaxLength,      SMU_DIRBUF_MAXLEN,
      XmNcolumns,        SMU_DIRTXT_MAXLEN, 
      XmNmarginHeight,   AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,    AppDim_MargLblPB_Get (&GAppDim),
      XmNvalue,          DirCB_FunGpBuf_Get (dircb_p),
      NULL)); 
 
  lbl_str = XmStringCreateLocalized (SMU_REACT_DIR_LBL);  
  DirCB_RxnLbl_Put (dircb_p,
    XtVaCreateManagedWidget ("DirsLbl",   
      xmLabelWidgetClass, form, 
      XmNlabelType, XmSTRING,  
      XmNlabelString, lbl_str,   
      NULL));  
  XmStringFree (lbl_str);

  DirCB_RxnBuf_Get (dircb_p)[0] = '\0';
  DirCB_RxnTxt_Put (dircb_p,
    XtVaCreateManagedWidget("DirsTxt",   
      xmTextWidgetClass, form,
      XmNeditMode,       XmSINGLE_LINE_EDIT,
      XmNmaxLength,      SMU_DIRBUF_MAXLEN,
      XmNcolumns,        SMU_DIRTXT_MAXLEN, 
      XmNmarginHeight,   AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,    AppDim_MargLblPB_Get (&GAppDim),
      XmNvalue,          DirCB_RxnBuf_Get (dircb_p),
      NULL)); 
     
  lbl_str = XmStringCreateLocalized (SMU_SUBMIT_DIR_LBL);  
  DirCB_SubmitLbl_Put (dircb_p,
    XtVaCreateManagedWidget ("DirsLbl",   
      xmLabelWidgetClass, form, 
      XmNlabelType, XmSTRING,  
      XmNlabelString, lbl_str,   
      NULL));  
  XmStringFree (lbl_str);

  DirCB_SubmitBuf_Get (dircb_p)[0] = '\0';
  DirCB_SubmitTxt_Put (dircb_p,
    XtVaCreateManagedWidget ("DirsTxt",   
      xmTextWidgetClass, form, 
      XmNeditMode,       XmSINGLE_LINE_EDIT,
      XmNmaxLength,      SMU_DIRBUF_MAXLEN,
      XmNcolumns,        SMU_DIRTXT_MAXLEN, 
      XmNmarginHeight,   AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,    AppDim_MargLblPB_Get (&GAppDim),
      XmNvalue,          DirCB_SubmitBuf_Get (dircb_p),
      NULL)); 
     
  lbl_str = XmStringCreateLocalized (SMU_TEMP_DIR_LBL);  
  DirCB_TempLbl_Put (dircb_p,
    XtVaCreateManagedWidget ("DirsLbl",   
      xmLabelWidgetClass, form, 
      XmNlabelType, XmSTRING,  
      XmNlabelString, lbl_str,   
      NULL));  
  XmStringFree (lbl_str);

  DirCB_TempBuf_Get (dircb_p)[0] = '\0';
  DirCB_TempTxt_Put (dircb_p,
    XtVaCreateManagedWidget ("DirsTxt",   
      xmTextWidgetClass, form, 
      XmNeditMode,       XmSINGLE_LINE_EDIT,
      XmNmaxLength,      SMU_DIRBUF_MAXLEN,
      XmNcolumns,        SMU_DIRTXT_MAXLEN, 
      XmNmarginHeight,   AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,    AppDim_MargLblPB_Get (&GAppDim),
      XmNvalue,          DirCB_TempBuf_Get (dircb_p),
      NULL)); 
     
  lbl_str = XmStringCreateLocalized (SMU_STATUS_DIR_LBL);  
  DirCB_StatusLbl_Put (dircb_p,
    XtVaCreateManagedWidget ("DirsLbl",   
      xmLabelWidgetClass, form, 
      XmNlabelType, XmSTRING,  
      XmNlabelString, lbl_str,   
      NULL));  
  XmStringFree (lbl_str);

  DirCB_StatusBuf_Get (dircb_p)[0] = '\0';
  DirCB_StatusTxt_Put (dircb_p,
    XtVaCreateManagedWidget ("DirsTxt",   
      xmTextWidgetClass, form, 
      XmNeditMode,       XmSINGLE_LINE_EDIT,
      XmNmaxLength,      SMU_DIRBUF_MAXLEN,
      XmNcolumns,        SMU_DIRTXT_MAXLEN, 
      XmNmarginHeight,   AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,    AppDim_MargLblPB_Get (&GAppDim),
      XmNvalue,          DirCB_StatusBuf_Get (dircb_p),
      NULL)); 
 
  lbl_str = XmStringCreateLocalized (SMU_PATH_DIR_LBL);  
  DirCB_PathLbl_Put (dircb_p,
    XtVaCreateManagedWidget ("DirsLbl",   
      xmLabelWidgetClass, form,
      XmNlabelType, XmSTRING,  
      XmNlabelString, lbl_str,   
      NULL));  
  XmStringFree (lbl_str);

  DirCB_PathBuf_Get (dircb_p)[0] = '\0';
  DirCB_PathTxt_Put (dircb_p,
    XtVaCreateManagedWidget("DirsTxt",   
      xmTextWidgetClass, form, 
      XmNeditMode,       XmSINGLE_LINE_EDIT,
      XmNmaxLength,      SMU_DIRBUF_MAXLEN,
      XmNcolumns,        SMU_DIRTXT_MAXLEN, 
      XmNmarginHeight,   AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,    AppDim_MargLblPB_Get (&GAppDim),
      XmNvalue,          DirCB_PathBuf_Get (dircb_p),
      NULL)); 

  lbl_str = XmStringCreateLocalized (SMU_TRACE_DIR_LBL);  
  DirCB_TraceLbl_Put (dircb_p,
    XtVaCreateManagedWidget ("DirsLbl",   
      xmLabelWidgetClass, form, 
      XmNlabelType, XmSTRING,  
      XmNlabelString, lbl_str,   
      NULL));  
  XmStringFree (lbl_str);

  DirCB_TraceBuf_Get (dircb_p)[0] = '\0';
  DirCB_TraceTxt_Put (dircb_p,
    XtVaCreateManagedWidget ("DirsTxt",   
      xmTextWidgetClass, form, 
      XmNeditMode,       XmSINGLE_LINE_EDIT,
      XmNmaxLength,      SMU_DIRBUF_MAXLEN,
      XmNcolumns,        SMU_DIRTXT_MAXLEN, 
      XmNmarginHeight,   AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,    AppDim_MargLblPB_Get (&GAppDim),
      XmNvalue,          DirCB_TraceBuf_Get (dircb_p),
      NULL)); 

  XtVaSetValues (DirCB_AvailTxt_Get (dircb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_NONE,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (DirCB_AvailLbl_Get (dircb_p),
    XmNtopOffset,        AppDim_MargLblPB_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        DirCB_AvailTxt_Get (dircb_p),
    XmNbottomOffset,     AppDim_MargLblPB_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     DirCB_AvailTxt_Get (dircb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      DirCB_AvailTxt_Get (dircb_p),
    NULL);

  XtVaSetValues (DirCB_FunGpTxt_Get (dircb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        DirCB_AvailTxt_Get (dircb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       DirCB_AvailTxt_Get (dircb_p),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (DirCB_FunGpLbl_Get (dircb_p),
    XmNtopOffset,        AppDim_MargLblPB_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        DirCB_FunGpTxt_Get (dircb_p),
    XmNbottomOffset,     AppDim_MargLblPB_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     DirCB_FunGpTxt_Get (dircb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      DirCB_FunGpTxt_Get (dircb_p),
    NULL);

  XtVaSetValues (DirCB_RxnTxt_Get (dircb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        DirCB_FunGpTxt_Get (dircb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       DirCB_AvailTxt_Get (dircb_p),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (DirCB_RxnLbl_Get (dircb_p),
    XmNtopOffset,        AppDim_MargLblPB_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        DirCB_RxnTxt_Get (dircb_p),
    XmNbottomOffset,     AppDim_MargLblPB_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     DirCB_RxnTxt_Get (dircb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      DirCB_RxnTxt_Get (dircb_p),
    NULL);

  XtVaSetValues (DirCB_SubmitTxt_Get (dircb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        DirCB_RxnTxt_Get (dircb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       DirCB_AvailTxt_Get (dircb_p),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (DirCB_SubmitLbl_Get (dircb_p),
    XmNtopOffset,        AppDim_MargLblPB_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        DirCB_SubmitTxt_Get (dircb_p),
    XmNbottomOffset,     AppDim_MargLblPB_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     DirCB_SubmitTxt_Get (dircb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      DirCB_SubmitTxt_Get (dircb_p),
    NULL);

  XtVaSetValues (DirCB_TempTxt_Get (dircb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        DirCB_SubmitTxt_Get (dircb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       DirCB_AvailTxt_Get (dircb_p),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (DirCB_TempLbl_Get (dircb_p),
    XmNtopOffset,        AppDim_MargLblPB_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        DirCB_TempTxt_Get (dircb_p),
    XmNbottomOffset,     AppDim_MargLblPB_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     DirCB_TempTxt_Get (dircb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      DirCB_TempTxt_Get (dircb_p),
    NULL);

  XtVaSetValues (DirCB_StatusTxt_Get (dircb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        DirCB_TempTxt_Get (dircb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       DirCB_AvailTxt_Get (dircb_p),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (DirCB_StatusLbl_Get (dircb_p),
    XmNtopOffset,        AppDim_MargLblPB_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        DirCB_StatusTxt_Get (dircb_p),
    XmNbottomOffset,     AppDim_MargLblPB_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     DirCB_StatusTxt_Get (dircb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      DirCB_StatusTxt_Get (dircb_p),
    NULL);

  XtVaSetValues (DirCB_PathTxt_Get (dircb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        DirCB_StatusTxt_Get (dircb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       DirCB_AvailTxt_Get (dircb_p),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (DirCB_PathLbl_Get (dircb_p),
    XmNtopOffset,        AppDim_MargLblPB_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        DirCB_PathTxt_Get (dircb_p),
    XmNbottomOffset,     AppDim_MargLblPB_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     DirCB_PathTxt_Get (dircb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      DirCB_PathTxt_Get (dircb_p),
    NULL);

  XtVaSetValues (DirCB_TraceTxt_Get (dircb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        DirCB_PathTxt_Get (dircb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget,       DirCB_AvailTxt_Get (dircb_p),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (DirCB_TraceLbl_Get (dircb_p),
    XmNtopOffset,        AppDim_MargLblPB_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        DirCB_TraceTxt_Get (dircb_p),
    XmNbottomOffset,     AppDim_MargLblPB_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     DirCB_TraceTxt_Get (dircb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      DirCB_TraceTxt_Get (dircb_p),
    NULL);

  XtAddCallback (DirCB_MsgDg_Get (dircb_p), 
    XmNhelpCallback, DirsForm_Reset_CB, NULL);

  XtAddCallback (DirCB_MsgDg_Get (dircb_p), 
    XmNcancelCallback,  DirsForm_Cancel_CB, (XtPointer) status_rcb_p);    
 
  XtManageChild (form);

  return;  
}  
/*  End of DirsForm_Create  */  

/****************************************************************************
*  
*  Function Name:                 DirsForm_Values_Get
*  
*****************************************************************************/  

void DirsForm_Values_Get
  (
  DirCB_t    *dircb_p,
  Rcb_t      *rcb_p
  )
{
  char  *field;
    
  field = XmTextGetString (DirCB_AvailTxt_Get (dircb_p));
  if (strcmp (field, (char *) String_Value_Get (FileCB_DirStr_Get (
      Rcb_IthFileCB_Get (rcb_p, FCB_IND_AVLC)))) != 0)
    {
    String_Destroy (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
      FCB_IND_AVLC)));
    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_AVLC), 
      String_Create (field, 0));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_AVLC), TRUE);
    Rcb_Flags_LibsChanged_Put (rcb_p, TRUE);
    }
  XtFree (field);

  field = XmTextGetString (DirCB_FunGpTxt_Get (dircb_p));
  if (strcmp (field, (char *) String_Value_Get (FileCB_DirStr_Get (
      Rcb_IthFileCB_Get (rcb_p, FCB_IND_FNGP)))) != 0)
    {
    String_Destroy (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
      FCB_IND_FNGP)));
    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FNGP), 
      String_Create (field, 0));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FNGP), TRUE);
    Rcb_Flags_LibsChanged_Put (rcb_p, TRUE);
    }
  XtFree (field);

  field = XmTextGetString (DirCB_RxnTxt_Get (dircb_p));
  if (strcmp (field, (char *) String_Value_Get (FileCB_DirStr_Get (
      Rcb_IthFileCB_Get (rcb_p, FCB_IND_RXNS)))) != 0)
    {
    String_Destroy (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
      FCB_IND_RXNS)));
    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_RXNS), 
      String_Create (field, 0));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_RXNS), TRUE);
    Rcb_Flags_LibsChanged_Put (rcb_p, TRUE);
    }
  XtFree (field);

  field = XmTextGetString (DirCB_TempTxt_Get (dircb_p));
  if (strcmp (field, (char *) String_Value_Get (FileCB_DirStr_Get (
     Rcb_IthFileCB_Get (rcb_p, FCB_IND_TMPL)))) != 0)
    {
    String_Destroy (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
      FCB_IND_TMPL)));
    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TMPL), 
      String_Create (field, 0));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TMPL), TRUE);
    }
  XtFree (field);

  field = XmTextGetString (DirCB_SubmitTxt_Get (dircb_p));
  if (strcmp (field, (char *) String_Value_Get (FileCB_DirStr_Get (
     Rcb_IthFileCB_Get (rcb_p, FCB_IND_SBMT)))) != 0)
   {
    String_Destroy (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
      FCB_IND_SBMT)));
    String_Destroy (FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, 
      FCB_IND_SBMT)));
    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_SBMT), 
      String_Create (field, 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_SBMT), 
      String_Create (field, MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_SBMT), TRUE);
    }
  XtFree (field);

  field = XmTextGetString (DirCB_StatusTxt_Get (dircb_p));
  if (strcmp (field, (char *) String_Value_Get (FileCB_DirStr_Get (
      Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT)))) != 0)
    {
    String_Destroy (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
      FCB_IND_STAT)));
    String_Destroy (FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, 
      FCB_IND_STAT)));
    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT), 
      String_Create (field, 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT), 
    String_Create (field, MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT), TRUE);
    }
   XtFree (field);

  field = XmTextGetString (DirCB_PathTxt_Get (dircb_p));
  if (strcmp (field, (char *) String_Value_Get (FileCB_DirStr_Get (
      Rcb_IthFileCB_Get (rcb_p, FCB_IND_PATH)))) != 0)
    {
    String_Destroy (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
      FCB_IND_PATH)));
    String_Destroy (FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, 
      FCB_IND_PATH)));
    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_PATH), 
      String_Create (field, 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_PATH), 
      String_Create (field, MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_PATH), TRUE);
    }
   XtFree (field);
  
  field = XmTextGetString (DirCB_TraceTxt_Get (dircb_p));
  if (strcmp (field, (char *) String_Value_Get (FileCB_DirStr_Get (
      Rcb_IthFileCB_Get (rcb_p, FCB_IND_TRCE)))) != 0)
    {
    String_Destroy (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
      FCB_IND_TRCE)));
    String_Destroy (FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, 
      FCB_IND_TRCE)));
    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TRCE), 
      String_Create (field, 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TRCE), 
      String_Create (field, MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TRCE), TRUE);
    }
  XtFree (field);
  
  return;
}
/*  End of DirsForm_Values_Get  */

/****************************************************************************
*  
*  Function Name:                 DirsForm_Values_Set
*  
*****************************************************************************/  

void DirsForm_Values_Set
  (
  DirCB_t    *dircb_p,
  Rcb_t      *rcb_p
  )
{
  strncpy (DirCB_AvailBuf_Get (dircb_p),
    (char *) String_Value_Get (FileCB_DirStr_Get (
    Rcb_IthFileCB_Get (rcb_p, FCB_IND_AVLC))), SMU_DIRBUF_MAXLEN);
  XmTextSetString (DirCB_AvailTxt_Get (dircb_p), 
    DirCB_AvailBuf_Get (dircb_p));

  strncpy (DirCB_FunGpBuf_Get (dircb_p),
    (char *) String_Value_Get (FileCB_DirStr_Get (
    Rcb_IthFileCB_Get (rcb_p, FCB_IND_FNGP))), SMU_DIRBUF_MAXLEN);
  XmTextSetString (DirCB_FunGpTxt_Get (dircb_p), 
    DirCB_FunGpBuf_Get (dircb_p));

  strncpy (DirCB_RxnBuf_Get (dircb_p),
    (char *) String_Value_Get (FileCB_DirStr_Get (
    Rcb_IthFileCB_Get (rcb_p, FCB_IND_RXNS))), SMU_DIRBUF_MAXLEN);
  XmTextSetString (DirCB_RxnTxt_Get (dircb_p), 
    DirCB_RxnBuf_Get (dircb_p));

  strncpy (DirCB_SubmitBuf_Get (dircb_p),
    (char *) String_Value_Get (FileCB_DirStr_Get (
    Rcb_IthFileCB_Get (rcb_p, FCB_IND_SBMT))), SMU_DIRBUF_MAXLEN);
  XmTextSetString (DirCB_SubmitTxt_Get (dircb_p), 
    DirCB_SubmitBuf_Get (dircb_p));

  strncpy (DirCB_TempBuf_Get (dircb_p),
    (char *) String_Value_Get (FileCB_DirStr_Get (
    Rcb_IthFileCB_Get (rcb_p, FCB_IND_TMPL))), SMU_DIRBUF_MAXLEN);
  XmTextSetString (DirCB_TempTxt_Get (dircb_p), 
    DirCB_TempBuf_Get (dircb_p));

  strncpy (DirCB_StatusBuf_Get (dircb_p),
    (char *) String_Value_Get (FileCB_DirStr_Get (
    Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT))), SMU_DIRBUF_MAXLEN);
  XmTextSetString (DirCB_StatusTxt_Get (dircb_p), 
    DirCB_StatusBuf_Get (dircb_p));

  strncpy (DirCB_PathBuf_Get (dircb_p),
    (char *) String_Value_Get (FileCB_DirStr_Get (
    Rcb_IthFileCB_Get (rcb_p, FCB_IND_PATH))), SMU_DIRBUF_MAXLEN);
  XmTextSetString (DirCB_PathTxt_Get (dircb_p), 
    DirCB_PathBuf_Get (dircb_p));

  strncpy (DirCB_TraceBuf_Get (dircb_p),
    (char *) String_Value_Get (FileCB_DirStr_Get (
    Rcb_IthFileCB_Get (rcb_p, FCB_IND_TRCE))), SMU_DIRBUF_MAXLEN);
  XmTextSetString (DirCB_TraceTxt_Get (dircb_p), 
    DirCB_TraceBuf_Get (dircb_p));
  return;  
}
/*  End of DirsForm_Values_Set  */

/****************************************************************************
*  
*  Function Name:                 FilesForm_Create
*  
*****************************************************************************/  

void FilesForm_Create  
  (  
  FilesCB_t     *filecb_p,
  Rcb_t         *status_rcb_p,
  Widget         topform
  )  
{  
  XmString        lbl_ok, lbl_reset, lbl_cancel, lbl_str;  
  Widget          form;
  Widget          sep;
  
  lbl_ok = XmStringCreateLocalized(SMU_OKAYPB_LBL);  
  lbl_reset = XmStringCreateLocalized(SMU_RESETPB_LBL);  
  lbl_cancel =XmStringCreateLocalized(SMU_CANCELPB_LBL);  
  lbl_str = XmStringCreateLocalized (SMU_FILES_DIALOG_TITLE);  
   
  FilesCB_MsgDg_Put (filecb_p, 
    XmCreateMessageDialog (topform, "FilesMDg", NULL, 0));

  XtVaSetValues (FilesCB_MsgDg_Get (filecb_p),  
    XmNdialogTitle,        lbl_str,
    XmNokLabelString,      lbl_ok,  
    XmNcancelLabelString,  lbl_cancel,   
    XmNhelpLabelString,    lbl_reset,   
    XmNdialogType,         XmDIALOG_TEMPLATE,  
    XmNlabelFontList,      SynAppR_FontList_Get (&GSynAppR),
    XmNbuttonFontList,     SynAppR_FontList_Get (&GSynAppR),
    XmNtextFontList,       SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,       AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,        AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,          True,  
    NULL);  
 
  XmStringFree (lbl_ok);  
  XmStringFree (lbl_reset);  
  XmStringFree (lbl_cancel);  
  XmStringFree (lbl_str);  


  form = XtVaCreateWidget ("FilesFm", 
    xmFormWidgetClass,  FilesCB_MsgDg_Get (filecb_p), 
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       True,
    NULL);

  FilesCB_Form_Put (filecb_p, form);

  lbl_str = XmStringCreateLocalized (SMU_SAVE_STAUS_LBL);  
  FilesCB_SaveStatus_Put (filecb_p,
    XtVaCreateManagedWidget("FilesTB",   
      xmToggleButtonWidgetClass, form,     
      XmNlabelType, XmSTRING,  
      XmNlabelString, lbl_str,   
      NULL));  
  XmStringFree (lbl_str);  
 
  lbl_str = XmStringCreateLocalized (SMU_SAVE_TRACE_LBL);  
  FilesCB_SaveTrace_Put (filecb_p,
    XtVaCreateManagedWidget("FilesTB",   
      xmToggleButtonWidgetClass, form,  
      XmNlabelType, XmSTRING,  
      XmNlabelString, lbl_str,   
      NULL));  
  XmStringFree (lbl_str);  

  sep = XtVaCreateManagedWidget ("FilesSep",  
    xmSeparatorWidgetClass, form, 
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmDOUBLE_LINE,
    XmNshadowThickness,     AppDim_SepLarge_Get (&GAppDim),
    NULL);
 
  lbl_str = XmStringCreateLocalized (SMU_TRACE_LEVEL_LBL);  
  FilesCB_TraceLvlLbl_Put (filecb_p,
    XtVaCreateManagedWidget("FilesLbl",   
      xmLabelWidgetClass, form,     
      XmNlabelType, XmSTRING,  
      XmNlabelString, lbl_str,   
      NULL));  
  XmStringFree (lbl_str);  

  FilesCB_TraceLvlBuf_Get (filecb_p)[0] = '\0';
  FilesCB_TraceLvlTxt_Put (filecb_p,
    XtVaCreateManagedWidget("FilesTxt",   
      xmTextWidgetClass, form,  
      XmNeditMode,       XmSINGLE_LINE_EDIT,
      XmNmaxLength,      SMU_LVLBUF_MAXLEN,
      XmNcolumns,        SMU_LVLTXT_MAXLEN, 
      XmNmarginHeight,   AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,    AppDim_MargLblPB_Get (&GAppDim),
      XmNvalue,          FilesCB_TraceLvlBuf_Get (filecb_p),
    NULL)); 

  XtVaSetValues (FilesCB_SaveStatus_Get (filecb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (FilesCB_SaveTrace_Get (filecb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        FilesCB_SaveStatus_Get (filecb_p),
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
    XmNtopWidget,        FilesCB_SaveTrace_Get (filecb_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (FilesCB_TraceLvlTxt_Get (filecb_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        sep,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_NONE,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (FilesCB_TraceLvlLbl_Get (filecb_p),
    XmNtopOffset,        AppDim_MargLblPB_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        FilesCB_TraceLvlTxt_Get (filecb_p),
    XmNbottomOffset,     AppDim_MargLblPB_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     FilesCB_TraceLvlTxt_Get (filecb_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      FilesCB_TraceLvlTxt_Get (filecb_p),
    NULL);


  XtAddCallback(FilesCB_TraceLvlTxt_Get (filecb_p),
    XmNmodifyVerifyCallback, Submit_NumVerify_CB, NULL);
 
  XtAddCallback(FilesCB_MsgDg_Get (filecb_p),
    XmNhelpCallback, FilesForm_Reset_CB, NULL); 

  XtAddCallback(FilesCB_MsgDg_Get (filecb_p),
    XmNcancelCallback, FilesForm_Cancel_CB, (XtPointer) status_rcb_p); 
 
  XtManageChild (form);

  return; 
}  
/*  End of FilesForm_Create  */

/****************************************************************************
*  
*  Function Name:                 FilesForm_Values_Get
*  
*****************************************************************************/  

void FilesForm_Values_Get
  (
  FilesCB_t *filecb_p,
  Rcb_t     *rcb_p
  )
{
  char      *temp_str;
  U32_t      number;


  /** Get save status flag **/
  if (XmToggleButtonGetState (FilesCB_SaveStatus_Get (filecb_p)))
    {
    Rcb_Flags_SaveStatus_Put (rcb_p, TRUE);
    }
  else
    {
    Rcb_Flags_SaveStatus_Put (rcb_p, FALSE);
    }

  /** Get save trace flag **/
  if (XmToggleButtonGetState (FilesCB_SaveTrace_Get (filecb_p)))
    {
    Rcb_Flags_SaveTrace_Put (rcb_p, TRUE);
    }
  else
    {
    Rcb_Flags_SaveTrace_Put (rcb_p, FALSE);
    }

  /** Get chemistry trace level **/ 
  temp_str = XmTextGetString (FilesCB_TraceLvlTxt_Get (filecb_p));
  if (temp_str == NULL || sscanf (temp_str, "%lu", &number) != 1)
    {
    InfoWarn_Show ("Trace level must be an integer between 0 and 255.");
    XtFree (temp_str);
    return;
    }

  if (number > 0xff)
    Rcb_ChemistryTrace_Put (rcb_p, 0xff);
  else
    Rcb_ChemistryTrace_Put (rcb_p, (U8_t) number);

  XtFree (temp_str);

  return;
}
/*  End of FilesForm_Values_Get  */

/****************************************************************************
*  
*  Function Name:                 FilesForm_Values_Set
*  
*****************************************************************************/  

void FilesForm_Values_Set
  (
  FilesCB_t *filecb_p,
  Rcb_t     *rcb_p
  )
{

  /** Set save status flag **/
  XtVaSetValues (FilesCB_SaveStatus_Get (filecb_p),
    XmNset, Rcb_Flags_SaveStatus_Get (rcb_p),
    NULL);

  /** Set save trace flag **/
  XtVaSetValues (FilesCB_SaveTrace_Get (filecb_p),
    XmNset, Rcb_Flags_SaveTrace_Get (rcb_p),
    NULL);

  /** Set chemistry trace level **/ 
  sprintf(FilesCB_TraceLvlBuf_Get (filecb_p), "%hu", 
    Rcb_ChemistryTrace_Get (rcb_p));
  XmTextSetString (FilesCB_TraceLvlTxt_Get (filecb_p),
    FilesCB_TraceLvlBuf_Get (filecb_p));

  return;
}
/*  End of FilesForm_Values_Set  */

/****************************************************************************
*  
*  Function Name:                 StratsForm_Create
*  
*****************************************************************************/  

void StratsForm_Create  
  (  
  StratCB_t     *stratcb_p,  
  Rcb_t         *status_rcb_p,
  Widget         topform  
  )  
{  
  XmString      lbl_ok, lbl_reset, lbl_cancel, title, lbl_str;  
    
  
  lbl_ok     = XmStringCreateLocalized(SMU_OKAYPB_LBL);  
  lbl_reset  = XmStringCreateLocalized(SMU_RESETPB_LBL);  
  lbl_cancel = XmStringCreateLocalized(SMU_CANCELPB_LBL);  
  title = XmStringCreateLocalized (SMU_STRATS_DIALOG_TITLE);  

  StratCB_FormDg_Put (stratcb_p, XmCreateMessageDialog (topform,  
    "StratsMDg", NULL, 0));  
  XtVaSetValues (StratCB_FormDg_Get (stratcb_p),  
    XmNdialogTitle,        title,
    XmNokLabelString,      lbl_ok,  
    XmNcancelLabelString,  lbl_cancel,   
    XmNhelpLabelString,    lbl_reset,   
    XmNdialogType,         XmDIALOG_TEMPLATE,  
    XmNresizable,          FALSE,  
    XmNlabelFontList,      SynAppR_FontList_Get (&GSynAppR),
    XmNbuttonFontList,     SynAppR_FontList_Get (&GSynAppR),
    XmNtextFontList,       SynAppR_FontList_Get (&GSynAppR),
    NULL);  
  
  XmStringFree (lbl_ok);  
  XmStringFree (lbl_reset);  
  XmStringFree (lbl_cancel);  
  XmStringFree (title);  

  StratCB_RowCol_Put (stratcb_p, XtVaCreateManagedWidget ("StratsRC",   
      xmRowColumnWidgetClass, StratCB_FormDg_Get (stratcb_p),  
      XmNpacking,         XmPACK_COLUMN,  
      XmNnumColumns,      3,  
      XmNorientation,     XmHORIZONTAL, 
      XmNadjustLast,      FALSE,  
      XmNisAligned,       TRUE,  
      XmNentryAlignment,  XmALIGNMENT_BEGINNING,  
      XmNrightAttachment, XmATTACH_FORM,  
      XmNleftAttachment,  XmATTACH_FORM,  
      XmNtopAttachment,   XmATTACH_FORM,  
      XmNbottomAttachment,XmATTACH_FORM,  
      NULL));  
 
  lbl_str = XmStringCreateLocalized (SMU_PRESERVE_STRUCT_LBL);  
  StratCB_PreserveStruct_Put (stratcb_p,
    XtVaCreateManagedWidget("StratsTB",   
      xmToggleButtonWidgetClass, StratCB_RowCol_Get (stratcb_p),  
      XmNlabelType, XmSTRING,  
      XmNlabelString, lbl_str,   
      NULL));  
  XmStringFree (lbl_str);  
 
  lbl_str = XmStringCreateLocalized (SMU_NO_STEREO_CHEM_LBL);  
  StratCB_NoStereoChem_Put (stratcb_p,
    XtVaCreateManagedWidget("StratsTB",   
    xmToggleButtonWidgetClass, StratCB_RowCol_Get (stratcb_p),  
      XmNlabelType, XmSTRING,  
      XmNlabelString, lbl_str,   
      NULL));  
  XmStringFree (lbl_str);  
 
  lbl_str = XmStringCreateLocalized (SMU_STRATEGIC_BONDS_LBL);  
  StratCB_StratBonds_Put (stratcb_p,
    XtVaCreateManagedWidget("StratsTB",   
    xmToggleButtonWidgetClass, StratCB_RowCol_Get (stratcb_p),  
      XmNlabelType, XmSTRING,  
      XmNlabelString, lbl_str,   
      NULL));  
  XmStringFree (lbl_str);  
     
  XtAddCallback(StratCB_FormDg_Get (stratcb_p),
    XmNhelpCallback, StratsForm_Reset_CB, NULL); 

  XtAddCallback(StratCB_FormDg_Get (stratcb_p),
    XmNcancelCallback, StratsForm_Cancel_CB, (XtPointer) status_rcb_p);

  return;
}  
/*  End of StratsForm_Create  */

/****************************************************************************
*  
*  Function Name:                 StratsForm_Values_Get
*  
*****************************************************************************/  

void StratsForm_Values_Get
  (
  StratCB_t     *stratcb_p,  
  Rcb_t         *rcb_p
  )
{

  /** Get preserve structure flag **/
  if (XmToggleButtonGetState (StratCB_PreserveStruct_Get (stratcb_p)))
    {
    Rcb_Flags_PreserveStructures_Put (rcb_p, TRUE);
    }
  else
    {
    Rcb_Flags_PreserveStructures_Put (rcb_p, FALSE);
    }


  /** Get no stereo chemistry flag **/
  if (XmToggleButtonGetState (StratCB_NoStereoChem_Get (stratcb_p)))
    {
    Rcb_Flags_StereoChemistry_Put (rcb_p, TRUE);
    }
  else
    {
    Rcb_Flags_StereoChemistry_Put (rcb_p, FALSE);
    }

  
  /** Get strategic bonds flag **/
  if (XmToggleButtonGetState (StratCB_StratBonds_Get (stratcb_p)))
    {
    Rcb_Flags_StrategicBonds_Put (rcb_p, TRUE);
    }
  else
    {
    Rcb_Flags_StrategicBonds_Put (rcb_p, FALSE);
    }

  return;
}
/*  End of StratsForm_Values_Get  */

/****************************************************************************
*  
*  Function Name:                 StratsForm_Values_Set
*  
*****************************************************************************/  

void StratsForm_Values_Set
  (
  StratCB_t     *stratcb_p,  
  Rcb_t          *rcb_p
  )
{
  /** Set preserve structure flag **/
  XtVaSetValues (StratCB_PreserveStruct_Get (stratcb_p),
    XmNset, Rcb_Flags_PreserveStructures_Get (rcb_p),
    NULL);

  /** Set no stereo chemistry flag **/
  XtVaSetValues (StratCB_NoStereoChem_Get (stratcb_p),
    XmNset, Rcb_Flags_StereoChemistry_Get (rcb_p),
    NULL);

  /** Set strtegic bonds flag **/
  XtVaSetValues (StratCB_StratBonds_Get (stratcb_p),
    XmNset, Rcb_Flags_StrategicBonds_Get (rcb_p),
    NULL);

  return;
}
/*  End of StratsForm_Values_Set  */

/****************************************************************************
*  
*  Function Name:                 SubmitMenu_Create
*  
*****************************************************************************/  

Widget SubmitMenu_Create  
  (
  SubmitCB_t      *submitcb_p,
  Widget           topform 
  )  
{  
  XmString         lbl_str;  
      
  /* Create the main menubar */  
  SubmitMenu_MenuBar_Put (&SSubMenuCB,
    XmCreateMenuBar (topform, "SubmitMenu", NULL, 0));  

  /* Create each pulldown menu:  
       1)  Create the pulldown menu;  
       2)  Create the cascade button with the pulldown menu as a child;  
       3)  Create and manage the buttons on the pulldown menu.  If the   
           button is itself a cascade button, then create and manage  
           its child pulldown menu and children buttons before adding it   
           to the cascade button.  
  */  
  
  /*----------------------------------------------------------------------*/  
  /*  The File Menu (cascade button and pulldown menu).                */  
  /*----------------------------------------------------------------------*/  
  SubmitMenu_FilePDM_Put (&SSubMenuCB, 
    XmCreatePulldownMenu (SubmitMenu_MenuBar_Get (&SSubMenuCB),   
    "SubmitMenuPDM", NULL, 0));  
  
  lbl_str = XmStringCreateLocalized (SMU_FILE_CPB_LBL);  
  SubmitMenu_FileCPB_Put (&SSubMenuCB,
    XtVaCreateManagedWidget ("SubmitMenuCB",
    xmCascadeButtonWidgetClass, SubmitMenu_MenuBar_Get (&SSubMenuCB),   
    XmNlabelString, lbl_str,  
    XmNsubMenuId, SubmitMenu_FilePDM_Get (&SSubMenuCB),  
    NULL));  
  XmStringFree (lbl_str);  
  
  /*  The Open - submit push button.  */  
  lbl_str = XmStringCreateLocalized (SMU_F_OPEN_SBMT_PB_LBL);  
  SubmitMenu_FSbmtPB_Put (&SSubMenuCB, 
    XtVaCreateManagedWidget ("SubmitMenuPB",   
    xmPushButtonWidgetClass, SubmitMenu_FilePDM_Get (&SSubMenuCB),   
    XmNlabelType, XmSTRING,  
    XmNlabelString, lbl_str,   
    NULL));  
  XmStringFree (lbl_str);  
  XtAddCallback (SubmitMenu_FSbmtPB_Get (&SSubMenuCB), XmNactivateCallback,   
    SubmitMenu_FSbmt_CB, (XtPointer) submitcb_p);    
    
  /* The Open - status push button */  
  lbl_str = XmStringCreateLocalized (SMU_F_OPEN_STAT_PB_LBL);  
  SubmitMenu_FStatPB_Put (&SSubMenuCB, 
    XtVaCreateManagedWidget ("SubmitMenuPB",   
    xmPushButtonWidgetClass, SubmitMenu_FilePDM_Get (&SSubMenuCB),   
    XmNlabelType, XmSTRING,  
    XmNlabelString, lbl_str,   
    NULL));  
  XmStringFree (lbl_str);  
  XtAddCallback (SubmitMenu_FStatPB_Get (&SSubMenuCB), XmNactivateCallback,   
    SubmitMenu_FStat_CB, (XtPointer) submitcb_p);  
  
  /* save push button */  
  lbl_str = XmStringCreateLocalized (SMU_F_SAVE_PB_LBL);  
  SubmitMenu_FSavePB_Put (&SSubMenuCB, 
    XtVaCreateManagedWidget ("SubmitMenuPB",   
    xmPushButtonWidgetClass, SubmitMenu_FilePDM_Get (&SSubMenuCB),   
    XmNlabelType, XmSTRING,  
    XmNlabelString, lbl_str,   
    NULL));  
  XmStringFree (lbl_str);  
  XtAddCallback (SubmitMenu_FSavePB_Get (&SSubMenuCB), XmNactivateCallback,   
    SubmitMenu_FSave_CB, (XtPointer) submitcb_p);  
  
  /*----------------------------------------------------------------------*/  
  /*  The Options Menu (cascade button and pulldown menu).                */  
  /*----------------------------------------------------------------------*/  
  SubmitMenu_OptPDM_Put (&SSubMenuCB, 
    XmCreatePulldownMenu (SubmitMenu_MenuBar_Get (&SSubMenuCB),   
    "SubmitMenuPDM", NULL, 0));  
  
  lbl_str = XmStringCreateLocalized (SMU_OPTIONS_CPB_LBL);  
  SubmitMenu_OptCPB_Put (&SSubMenuCB,
    XtVaCreateManagedWidget ("SubmitMenuCB",
    xmCascadeButtonWidgetClass, SubmitMenu_MenuBar_Get (&SSubMenuCB),   
    XmNlabelString, lbl_str,  
    XmNsubMenuId, SubmitMenu_OptPDM_Get (&SSubMenuCB),  
    NULL));  
  XmStringFree (lbl_str);  
  
  /*  The Directories push button.  */  
  lbl_str = XmStringCreateLocalized (SMU_O_DIRECTORIES_PB_LBL);  
  SubmitMenu_ODirPB_Put (&SSubMenuCB, 
    XtVaCreateManagedWidget ("SubmitMenuPB",   
    xmPushButtonWidgetClass, SubmitMenu_OptPDM_Get (&SSubMenuCB),   
    XmNlabelType, XmSTRING,  
    XmNlabelString, lbl_str,   
    NULL));  
  XmStringFree (lbl_str);  
  XtAddCallback (SubmitMenu_ODirPB_Get (&SSubMenuCB), XmNactivateCallback,   
    SubmitMenu_ODirs_CB, (XtPointer) SubmitMenu_DirCB_Get (&SSubMenuCB));    
    
  /* The Files push button */  
  lbl_str = XmStringCreateLocalized (SMU_O_FILES_PB_LBL);  
  SubmitMenu_OFilesPB_Put (&SSubMenuCB, 
    XtVaCreateManagedWidget ("SubmitMenuPB",   
    xmPushButtonWidgetClass, SubmitMenu_OptPDM_Get (&SSubMenuCB),   
    XmNlabelType, XmSTRING,  
    XmNlabelString, lbl_str,   
    NULL));  
  XmStringFree (lbl_str);  
  XtAddCallback (SubmitMenu_OFilesPB_Get (&SSubMenuCB), XmNactivateCallback,   
    SubmitMenu_OFiles_CB, (XtPointer) SubmitMenu_FilesCB_Get (&SSubMenuCB));  
  
  /* Strategies push button */  
  lbl_str = XmStringCreateLocalized (SMU_O_STRATEGIES_LBL);  
  SubmitMenu_OStratPB_Put (&SSubMenuCB, 
    XtVaCreateManagedWidget ("SubmitMenuPB",   
    xmPushButtonWidgetClass, SubmitMenu_OptPDM_Get (&SSubMenuCB),   
    XmNlabelType, XmSTRING,  
    XmNlabelString, lbl_str,   
    NULL));  
  XmStringFree (lbl_str);  
  XtAddCallback (SubmitMenu_OStratPB_Get (&SSubMenuCB), XmNactivateCallback,   
    SubmitMenu_OStrats_CB, (XtPointer) SubmitMenu_StratCB_Get (&SSubMenuCB));  

  /*----------------------------------------------------------------------*/  
  /*  The Help Menu (cascade button and pulldown menu).                   */  
  /*----------------------------------------------------------------------*/  
  SubmitMenu_HelpPDM_Put (&SSubMenuCB, 
    XmCreatePulldownMenu (SubmitMenu_MenuBar_Get (&SSubMenuCB),   
    "HelpMenuPDM", NULL, 0));  
  
  lbl_str = XmStringCreateLocalized (SMU_HELP_CPB_LBL);  
  SubmitMenu_HelpCPB_Put (&SSubMenuCB, 
    XtVaCreateManagedWidget ("SubmitMenuCB",   
    xmCascadeButtonWidgetClass, SubmitMenu_MenuBar_Get (&SSubMenuCB),   
    XmNlabelString, lbl_str,  
    XmNsubMenuId, SubmitMenu_HelpPDM_Get (&SSubMenuCB),  
    NULL));   
    
  /* Help Push Button */  
  lbl_str = XmStringCreateLocalized (SMU_HELP_PB_LBL);  
  SubmitMenu_HelpPB_Put (&SSubMenuCB, 
    XtVaCreateManagedWidget ("SubmitMenuPB",   
    xmPushButtonWidgetClass, SubmitMenu_HelpPDM_Get (&SSubMenuCB),   
    XmNlabelType, XmSTRING,  
    XmNlabelString, lbl_str,   
    NULL));  
  XmStringFree (lbl_str);

/*  
  XtAddCallback (SubmitMenu_HelpPB_Get (&SSubMenuCB), XmNactivateCallback,   
    ContextHelp_CB, (XtPointer) topform);
*/

  
  XtVaSetValues (SubmitMenu_MenuBar_Get (&SSubMenuCB),  
    XmNmenuHelpWidget, SubmitMenu_HelpCPB_Get (&SSubMenuCB),
    NULL);   
  
  XtSetSensitive (SubmitMenu_HelpCPB_Get (&SSubMenuCB), FALSE);

  DirsForm_Create (SubmitMenu_DirCB_Get (&SSubMenuCB), 
    Submit_TempRcb_Get (submitcb_p), XtParent(topform));
  FilesForm_Create (SubmitMenu_FilesCB_Get (&SSubMenuCB), 
    Submit_TempRcb_Get (submitcb_p), XtParent(topform));
  StratsForm_Create (SubmitMenu_StratCB_Get (&SSubMenuCB), 
    Submit_TempRcb_Get (submitcb_p), XtParent(topform));

  return (SubmitMenu_MenuBar_Get (&SSubMenuCB));  
}  
/*  End of SubmitMenu_Create  */
  
/****************************************************************************
*  
*  Function Name:                 SubmitMenu_Destroy
*  
*****************************************************************************/  

void SubmitMenu_Destroy 
  (
  void
  )
{
  return;
}  
/*  End of SubmitMenu_Destroy  */

/****************************************************************************
*  
*  Function Name:                 SubmitMenu_Dialogs_PopDown
*  
*****************************************************************************/  

void SubmitMenu_Dialogs_PopDown
  (
  void
  )
{

  if (XtIsManaged (DirCB_MsgDg_Get (SubmitMenu_DirCB_Get (&SSubMenuCB))))
    XtUnmanageChild (DirCB_MsgDg_Get (SubmitMenu_DirCB_Get (&SSubMenuCB))); 
  
  if (XtIsManaged (FilesCB_MsgDg_Get (SubmitMenu_FilesCB_Get (&SSubMenuCB))))
    XtUnmanageChild (FilesCB_MsgDg_Get (SubmitMenu_FilesCB_Get (&SSubMenuCB)));
  
  if (XtIsManaged (StratCB_FormDg_Get (SubmitMenu_StratCB_Get (&SSubMenuCB))))
    XtUnmanageChild (StratCB_FormDg_Get (SubmitMenu_StratCB_Get (&SSubMenuCB)));

  return;
}  
/*  End of SubmitMenu_Dialogs_PopDown  */

/****************************************************************************
*  
*  Function Name:                 SubmitMenu_Forms_Get
*  
*****************************************************************************/  

void SubmitMenu_Forms_Get 
  (
  Rcb_t      *rcb_p
  )
{
  DirsForm_Values_Get (SubmitMenu_DirCB_Get (&SSubMenuCB), rcb_p);
  FilesForm_Values_Get (SubmitMenu_FilesCB_Get (&SSubMenuCB), rcb_p);
  StratsForm_Values_Get (SubmitMenu_StratCB_Get (&SSubMenuCB), rcb_p);  

  return;
}  
/*  End of SubmitMenu_Forms_Get  */

/****************************************************************************
*  
*  Function Name:                 SubmitMenu_Forms_Set
*  
*****************************************************************************/  

void SubmitMenu_Forms_Set 
  (
  Rcb_t      *rcb_p
  )
{
  DirsForm_Values_Set (SubmitMenu_DirCB_Get (&SSubMenuCB), rcb_p);
  FilesForm_Values_Set (SubmitMenu_FilesCB_Get (&SSubMenuCB), rcb_p);
  StratsForm_Values_Set (SubmitMenu_StratCB_Get (&SSubMenuCB), rcb_p);  

  return;
}  
/*  End of SubmitMenu_Forms_Set  */

/****************************************************************************
*  
*  Function Name:                 DirsForm_Cancel_CB
*  
*****************************************************************************/  

void DirsForm_Cancel_CB
(
  Widget    w,   
  XtPointer client_data,   
  XtPointer call_data
)
{
  Rcb_t  *rcb_p;

  rcb_p = (Rcb_t *) client_data;
  DirsForm_Values_Set (SubmitMenu_DirCB_Get (&SSubMenuCB), rcb_p);

  return;
}
/*  End of DirsForm_Cancel_CB  */

/****************************************************************************
*  
*  Function Name:                 DirsForm_Reset_CB
*  
*****************************************************************************/  

void DirsForm_Reset_CB
(
  Widget    w,   
  XtPointer client_data,   
  XtPointer call_data
)
{
  Rcb_t rcb;

  Rcb_Init (&rcb, FALSE);
  DirsForm_Values_Set (SubmitMenu_DirCB_Get (&SSubMenuCB), &rcb);
  Rcb_Destroy (&rcb);

  return;
}
/*  End of DirsForm_Reset_CB  */

/****************************************************************************
*  
*  Function Name:                 FilesForm_Cancel_CB
*  
*****************************************************************************/  

void FilesForm_Cancel_CB
(
  Widget    w,   
  XtPointer client_data,   
  XtPointer call_data
)
{
  Rcb_t  *rcb_p;

  rcb_p = (Rcb_t *) client_data;
  FilesForm_Values_Set (SubmitMenu_FilesCB_Get (&SSubMenuCB), rcb_p);

  return;
}
/*  End of FilesForm_Cancel_CB  */

/****************************************************************************
*  
*  Function Name:                 FilesForm_Reset_CB
*  
*****************************************************************************/  

void FilesForm_Reset_CB
(
  Widget    w,   
  XtPointer client_data,   
  XtPointer call_data
)
{
  Rcb_t rcb;

  Rcb_Init (&rcb, FALSE);
  FilesForm_Values_Set (SubmitMenu_FilesCB_Get (&SSubMenuCB), &rcb);
  Rcb_Destroy (&rcb);

  return;
}
/*  End of FilesForm_Reset_CB  */

/****************************************************************************
*  
*  Function Name:                 StratsForm_Cancel_CB
*  
*****************************************************************************/  

void StratsForm_Cancel_CB
(
  Widget    w,   
  XtPointer client_data,   
  XtPointer call_data
)
{
  Rcb_t  *rcb_p;

  rcb_p = (Rcb_t *) client_data;
  StratsForm_Values_Set (SubmitMenu_StratCB_Get (&SSubMenuCB), rcb_p);

  return;
}
/*  End of StratsForm_Cancel_CB  */

/****************************************************************************
*  
*  Function Name:                 StratsForm_Reset_CB
*  
*****************************************************************************/  

void StratsForm_Reset_CB
(
  Widget    w,   
  XtPointer client_data,   
  XtPointer call_data
)
{
  Rcb_t rcb;

  Rcb_Init (&rcb, FALSE);
  StratsForm_Values_Set (SubmitMenu_StratCB_Get (&SSubMenuCB), &rcb);
  Rcb_Destroy (&rcb);

  return;
}
/*  End of StratsForm_Reset_CB  */

/****************************************************************************
*  
*  Function Name:                 SubmitMenu_FSave_CB
*  
*****************************************************************************/  

void SubmitMenu_FSave_CB  
  (  
  Widget    w,   
  XtPointer client_data,   
  XtPointer call_data    
  )  
{  
  SubmitCB_t  *scb_p;  
  Rcb_t        rcb;

  scb_p = (SubmitCB_t *) client_data;

  Rcb_Init (&rcb, FALSE);
  Submit_Values_Get (scb_p, &rcb, FALSE); 
  FileNames_Complete (Rcb_FileCBs_Get (&rcb), 
    Rcb_Name_Get (&rcb));

  if (Submission_Dump (&rcb))
    InfoMess_Show ("Job submission file saved.");

  Rcb_Destroy (&rcb);

  return;  
}  
/*  End of SubmitMenu_FSave_CB  */

/****************************************************************************
*  
*  Function Name:                 SubmitMenu_FSbmt_CB
*  
*****************************************************************************/  

void SubmitMenu_FSbmt_CB  
  (  
  Widget    w,   
  XtPointer client_data,   
  XtPointer call_data    
  )  
{  
  SVF_FileDg_Update (SVF_FILED_TYPE_OPENSBMT, client_data, FALSE);

  return;  
}  
/*  End of SubmitMenu_FSbmt_CB  */

/****************************************************************************
*  
*  Function Name:                 SubmitMenu_FStat_CB
*  
*****************************************************************************/  

void SubmitMenu_FStat_CB  
  (  
  Widget    w,   
  XtPointer client_data,   
  XtPointer call_data    
  )  
{  
   
  SVF_FileDg_Update (SVF_FILED_TYPE_PEEKSTAT, client_data, FALSE);

  return;  
}  
/*  End of SubmitMenu_FStat_CB  */


/****************************************************************************
*  
*  Function Name:                 SubmitMenu_ODirs_CB
*  
*****************************************************************************/  

void SubmitMenu_ODirs_CB  
  (  
  Widget    w,   
  XtPointer client_data,   
  XtPointer call_data    
  )  
{  
  SubmitMenuCB_t  *menucb_p;  
   
  menucb_p = (SubmitMenuCB_t *) client_data;
  if (!XtIsManaged (DirCB_MsgDg_Get (SubmitMenu_DirCB_Get (menucb_p))))
    XtManageChild (DirCB_MsgDg_Get (SubmitMenu_DirCB_Get (menucb_p))); 

  return;  
}  
/*  End of SubmitMenu_ODirs_CB  */

/****************************************************************************
*  
*  Function Name:                 SubmitMenu_OFiles_CB
*  
*****************************************************************************/  

void SubmitMenu_OFiles_CB  
  (  
  Widget    w,   
  XtPointer client_data,   
  XtPointer call_data     
  )  
{  
  FilesCB_t  *fcb_p;  
   
  fcb_p = (FilesCB_t *) client_data;

  if (!XtIsManaged (FilesCB_MsgDg_Get (fcb_p)))
    XtManageChild (FilesCB_MsgDg_Get (fcb_p)); 

  return;  
}  
/*  End of SubmitMenu_OFiles_CB  */
  
/****************************************************************************
*  
*  Function Name:                 SubmitMenu_OStrats_CB
*  
*****************************************************************************/  

void SubmitMenu_OStrats_CB  
  (  
  Widget    w,   
  XtPointer client_data,   
  XtPointer call_data  
  )  
{  
  StratCB_t  *scb_p;  
   
  scb_p = (StratCB_t *) client_data;
  if (!XtIsManaged (StratCB_FormDg_Get (scb_p)))
    XtManageChild (StratCB_FormDg_Get (scb_p)); 

 return;  
}  
/*  End of SubmitMenu_OStrats_CB  */

/*  End of SUBMIT_MENU.C  */  
