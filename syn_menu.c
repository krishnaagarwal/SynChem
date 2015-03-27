/******************************************************************************
*
*  Copyright (C) 1995, Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     SYN_MENU.C
*
*    This module defines the routines needed to handle the main menu bar of
*    the SYNCHEM expert system.  Several of the callback routines are stubs
*    that simply return when evoked.  See the header file syn_menu.h for a 
*    layout of the menu bar.
*
*  Creation Date:
*
*    10-Jun-1995
*
*  Authors:
*
*    Daren Krebsbach
*
*  Routines:
*
*    SynMenu_Create
*    SynMenu_Destroy
*
*  Callback Routines:
*
*    SynMenu_ClearMark_CB
*    SynMenu_ClearVisit_CB
*    SynMenu_CmpInst_CB
*    SynMenu_Exit_CB
*    SynMenu_File_CB
*    SynMenu_Help_CB         (stub)
*    SynMenu_Info_CB
*    SynMenu_KBaseEdit_CB
*    SynMenu_KBaseSearch_CB
*    SynMenu_MultiShot_CB    (stub)
*    SynMenu_AvCmpEdit_CB
*    SynMenu_Path_CB
*    SynMenu_PreView_CB
*    SynMenu_RxnCmmt_CB
*    SynMenu_SearchCont_CB   (stub)
*    SynMenu_SelTrace_CB
*    SynMenu_SglShot_CB
*    SynMenu_Submit_CB
*    SynMenu_SymLegend_CB
*    SynMenu_TreeType_CB
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 2/7/2001   Miller     Corrected find_marked (which writes a PathTrc-friendly
*                       version of the path file) to be a Boolean_t function,
*                       aborting and returning FALSE in the event a circularity
*                       is discovered during the attempt to write the path.
*
******************************************************************************/

#include <stdlib.h>
#include <time.h>

#include <Xm/CascadeB.h>
#include <Xm/CascadeBG.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include <Xm/MessageB.h>

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

#ifndef _H_PST_
#include "pst.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_RCB_
#include "rcb.h"
#endif

#ifndef _H_STATUS_
#include "status.h"
#endif

#ifndef _H_APP_RESRC_
#include "app_resrc.h"
#endif

#ifndef _H_INFO_VIEW_
#include "info_view.h"
#endif

#ifndef _H_RXN_PREVIEW_
#include "rxn_preview.h"
#endif

#ifndef _H_PST_VIEW_
#include "pst_view.h"
#endif

#ifndef _H_SV_FILES_
#include "sv_files.h"
#endif

#ifndef _H_SSHOT_VIEW_
#include "sshot_view.h"
#endif

#ifndef _H_CMP_INST_
#include "cmp_inst.h"
#endif

#ifndef _H_SEL_TRACE_
#include "sel_trace.h"
#endif

#ifndef _H_SUBMIT_ 
#include "submit.h" 
#endif

#ifndef _H_SYN_VIEW_
#include "syn_view.h"
#endif

#ifndef _H_SYN_MENU_
#include "syn_menu.h"
#endif

#ifndef _H_SYNHELP_
#include "synhelp.h"
#endif

#ifndef _H_LOGIN_
#include "login.h"
#endif

#ifndef _H_EXTERN_
#include "extern.h"
#endif

void PstView_Set_Avledit_Called ();
void PathTrc_Create ();
void PathTrc_Display (char *);
void Status_Pst_Serialize   (Array_t *, Array_t *, PstNode_t);
Boolean_t PstView_SelCmp (PstView_t *, Compound_t *);
void CmpInst_Path_Put (char *);

/* Static Variables */

  static SynMenuCB_t    SSMenuCB;      /* Main menu bar control block */
  static SynViewCB_t   *SynViewCB_P;   /* SynView control block */
  static char *glob_path;
  static Sling_t glob_sling;
  static U32_t glob_rxn_key;
static Boolean_t glob_norxn;
static Widget submarked_pb;
static Widget delrun_pb;

/* Static Routine Prototypes */
void SynMenu_PathTrc_CB (Widget, XtPointer, XtPointer);
void SynMenu_SubMarked_CB (Widget, XtPointer, XtPointer);
void SynMenu_Expand_CB (Widget, XtPointer, XtPointer);

debug_print (char *command)
{
  int i;

  printf("\n\n%s\n",command);
  for (i=0; i<strlen(command); i++) printf(" %02x",command[i]);
  printf("\n\n");
}


/****************************************************************************
*
*  Function Name:                 SynMenu_Create
*
*    This routine creates the main menu bar and all of its children:
*    the pulldown menus (cascade buttons), pushbuttons and toggle buttons.
*
*    Those options that have not been implemented are desensitized.
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
*    Main menu widget.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Widget SynMenu_Create
  (
  Widget         topform,
  SynViewCB_t   *svcb_p,
  char          *path
  )
{
  PstView_t     *pstv_p;
  XmString       lbl_str;
  XmString       acc_str;
  
glob_path=path;

  PathTrc_Create (); /* called from syn_menu, so it may as well be created here to minimize changes. */

  SynViewCB_P = svcb_p;
  pstv_p = SynVCB_PstView_Get (svcb_p);

  /* Create the main menubar */
  SSMenuCB.menu_bar = XmCreateMenuBar (topform, 
    "SynMenu", NULL, 0);

  /* Create each pulldown menu:

       1)  Create the pulldown menu;
       2)  Create the cascade button with the pulldown menu as a child;
       3)  Create and manage the buttons on the pulldown menu.  If the 
           button is itself a cascade button, then create and manage
           its child pulldown menu and children buttons before adding it 
           to the cascade button.
  */

  /*----------------------------------------------------------------------*/
  /*  The File Menu (cascade button and pulldown menu).                   */
  /*----------------------------------------------------------------------*/
  SSMenuCB.file_pdm = XmCreatePulldownMenu (SSMenuCB.menu_bar, 
    "SynMenuPDM", NULL, 0);

  lbl_str = XmStringCreateLocalized (SMU_FILE_CPB_LBL);
  SSMenuCB.file_cpb = XtVaCreateManagedWidget ("SynMenuCB", 
    xmCascadeButtonWidgetClass, SSMenuCB.menu_bar, 
    XmNlabelString, lbl_str,
    XmNsubMenuId, SSMenuCB.file_pdm,
    XmNmnemonic, SMU_FILE_MNEM, 
    NULL);
  XmStringFree (lbl_str);

  /*  The Open push button.  */
  lbl_str = XmStringCreateLocalized (SMU_F_OPEN_PB_LBL);
  SSMenuCB.f_open_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.file_pdm, 
    XmNlabelType, XmSTRING,
    XmNlabelString, lbl_str, 
    NULL);
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.f_open_pb, XmNactivateCallback, 
    SynMenu_File_CB, (XtPointer) SMU_FILE_OPEN_TAG);

  /*  The Save push button.  */
  lbl_str = XmStringCreateLocalized (SMU_F_SAVE_PB_LBL);
  SSMenuCB.f_save_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.file_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.f_save_pb, XmNactivateCallback, 
    SynMenu_File_CB, (XtPointer) SMU_FILE_SAVE_TAG);

  /*  The Save developed compounds push button.  */
  lbl_str = XmStringCreateLocalized (SMU_F_SAVECVR_PB_LBL);
  SSMenuCB.f_savecvr_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.file_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.f_savecvr_pb, XmNactivateCallback, 
    SynMenu_File_CB, (XtPointer) SMU_FILE_SAVECVR_TAG);

/**/  XtSetSensitive (SSMenuCB.f_savecvr_pb, False);

  /*  The Print push button.  */
  lbl_str = XmStringCreateLocalized (SMU_F_PRINT_WIN_PB_LBL);
  SSMenuCB.f_printwin_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.file_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.f_printwin_pb, XmNactivateCallback, 
    SynMenu_File_CB, (XtPointer) SMU_FILE_PRINT_WIN_TAG);

  lbl_str = XmStringCreateLocalized (SMU_F_PRINT_RXN_PB_LBL);
  SSMenuCB.f_printrxn_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.file_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.f_printrxn_pb, XmNactivateCallback, 
    SynMenu_File_CB, (XtPointer) SMU_FILE_PRINT_RXN_TAG);

  /*  The "Delete a Run" push button.  */
  lbl_str = XmStringCreateLocalized ("Delete a Run");
  delrun_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.file_pdm, 
    XmNlabelString, lbl_str,
    NULL);
  XmStringFree (lbl_str);
  XtAddCallback (delrun_pb, XmNactivateCallback, 
    SynMenu_File_CB, (XtPointer) "D");

  /*  The Exit push button.  */
  lbl_str = XmStringCreateLocalized (SMU_F_EXIT_PB_LBL);
  acc_str = XmStringCreateLocalized (SMU_EXIT_ACLBL);
  SSMenuCB.f_exit_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.file_pdm, 
    XmNlabelString, lbl_str,
    XmNaccelerator, SMU_EXIT_ACCEL,
    XmNacceleratorText, acc_str,
    NULL);
  XmStringFree (lbl_str);
  XmStringFree (acc_str);
  XtAddCallback (SSMenuCB.f_exit_pb, XmNactivateCallback, 
    SynMenu_Exit_CB, (XtPointer) NULL);

  /*----------------------------------------------------------------------*/
  /*  The Path Menu (cascade button and pulldown menu).                   */
  /*----------------------------------------------------------------------*/
  SSMenuCB.path_pdm = XmCreatePulldownMenu (SSMenuCB.menu_bar, 
    "SynMenuPDM", NULL, 0);

  lbl_str = XmStringCreateLocalized (SMU_PATH_CPB_LBL);
  SSMenuCB.path_cpb = XtVaCreateManagedWidget ("SynMenuCB", 
    xmCascadeButtonWidgetClass, SSMenuCB.menu_bar, 
    XmNlabelString, lbl_str,
    XmNsubMenuId, SSMenuCB.path_pdm,
    XmNmnemonic, SMU_PATH_MNEM,
    NULL);  
  XmStringFree (lbl_str);

  /*  The Read push button.  */
  lbl_str = XmStringCreateLocalized (SMU_P_READ_PB_LBL);
  SSMenuCB.p_read_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.path_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.p_read_pb, XmNactivateCallback, 
    SynMenu_Path_CB, (XtPointer) SMU_PATH_READ_TAG);

  /*  The Write push button.  */
  lbl_str = XmStringCreateLocalized (SMU_P_WRITE_PB_LBL);
  SSMenuCB.p_write_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.path_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.p_write_pb, XmNactivateCallback, 
    SynMenu_Path_CB, (XtPointer) SMU_PATH_WRITE_TAG);

  /*  The View (PathTrc) push button.  */
  lbl_str = XmStringCreateLocalized (SMU_P_VIEW_PB_LBL);
  SSMenuCB.p_view_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.path_pdm,
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.p_view_pb, XmNactivateCallback, 
/*
    SynMenu_Path_CB, (XtPointer) SMU_PATH_VIEW_TAG);
*/
    SynMenu_PathTrc_CB, (XtPointer) NULL);

/**/  XtSetSensitive (SSMenuCB.p_view_pb, True);

  /*----------------------------------------------------------------------*/
  /*  The Options Menu (cascade button and pulldown menu).                */
  /*----------------------------------------------------------------------*/
  SSMenuCB.option_pdm = XmCreatePulldownMenu (SSMenuCB.menu_bar, 
    "SynMenuPDM", NULL, 0);

  lbl_str = XmStringCreateLocalized (SMU_OPTION_CPB_LBL);
  SSMenuCB.option_cpb = XtVaCreateManagedWidget ("SynMenuCB", 
    xmCascadeButtonWidgetClass, SSMenuCB.menu_bar, 
    XmNlabelString, lbl_str,
    XmNsubMenuId, SSMenuCB.option_pdm,
    XmNmnemonic, SMU_OPT_MNEM,
    NULL);  
  XmStringFree (lbl_str);

  /*---------------------------------------------------*/
  /*  The Tree Type cascade button and pulldown menu.  */
  SSMenuCB.o_tree_pdm = XmCreatePulldownMenu (SSMenuCB.option_pdm, 
    "SynMenuPDM", NULL, 0);
  XtVaSetValues (SSMenuCB.o_tree_pdm, 
    XmNtearOffModel, XmTEAR_OFF_ENABLED, 
    NULL);

  lbl_str = XmStringCreateLocalized (SMU_O_TREE_CPB_LBL);
  SSMenuCB.o_tree_cpb = XtVaCreateManagedWidget ("SynMenuCB", 
    xmCascadeButtonWidgetClass, SSMenuCB.option_pdm, 
    XmNlabelString, lbl_str,
    XmNsubMenuId, SSMenuCB.o_tree_pdm,
    XmNmnemonic, SMU_TREET_MNEM,
    NULL);  
  XmStringFree (lbl_str);

  /*  Tree Type:  The Full toggle button.  */
  lbl_str = XmStringCreateLocalized (SMU_OT_FULL_TB_LBL);
  SSMenuCB.ot_full_tb = XtVaCreateManagedWidget ("SynMenuTB", 
    xmToggleButtonWidgetClass, SSMenuCB.o_tree_pdm, 
    XmNlabelString, lbl_str,
    XmNset, True,
    XmNindicatorType, XmONE_OF_MANY,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.ot_full_tb, XmNvalueChangedCallback, 
    SynMenu_TreeType_CB, (XtPointer) SMU_OTREE_FULL_TAG);

  /*  Tree Type:  The Developed toggle button.  */
  lbl_str = XmStringCreateLocalized (SMU_OT_DEV_TB_LBL);
  SSMenuCB.ot_dev_tb = XtVaCreateManagedWidget ("SynMenuTB", 
    xmToggleButtonWidgetClass, SSMenuCB.o_tree_pdm, 
    XmNlabelString, lbl_str,
    XmNset, False,
    XmNindicatorType, XmONE_OF_MANY,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.ot_dev_tb, XmNvalueChangedCallback, 
    SynMenu_TreeType_CB, (XtPointer) SMU_OTREE_DEV_TAG);

  /*  Tree Type:  The Solved toggle button.  */
  lbl_str = XmStringCreateLocalized (SMU_OT_SOLV_TB_LBL);
  SSMenuCB.ot_solv_tb = XtVaCreateManagedWidget ("SynMenuTB", 
    xmToggleButtonWidgetClass, SSMenuCB.o_tree_pdm, 
    XmNlabelString, lbl_str,
    XmNset, False,
    XmNindicatorType, XmONE_OF_MANY,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.ot_solv_tb, XmNvalueChangedCallback, 
    SynMenu_TreeType_CB, (XtPointer) SMU_OTREE_SOLV_TAG);

  /*----------------------------------*/
  /*  The Symbol Legend push button.  */
  lbl_str = XmStringCreateLocalized (SMU_O_LEGEND_PB_LBL);
  acc_str = XmStringCreateLocalized (SMU_LEGEND_ACLBL);
  SSMenuCB.o_legend_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.option_pdm, 
    XmNlabelString, lbl_str,
    XmNaccelerator, SMU_LEGEND_ACCEL,
    XmNacceleratorText, acc_str,
    NULL);  
  XmStringFree (lbl_str);
  XmStringFree (acc_str);
  XtAddCallback (SSMenuCB.o_legend_pb, XmNactivateCallback, 
    SynMenu_SymLegend_CB, (XtPointer) NULL);

/**/  XtSetSensitive (SSMenuCB.o_legend_pb, False);

  /*  The Cycle Numbers push button.  */
  lbl_str = XmStringCreateLocalized (SMU_O_CYCLENUMS_PB_LBL);
  SSMenuCB.o_cycles_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.option_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.o_cycles_pb, XmNactivateCallback, 
    SelTrace_NumAll_CB, (XtPointer) pstv_p);

  /*  The Clear All Visits push button.  */
  lbl_str = XmStringCreateLocalized (SMU_O_CLEARVST_PB_LBL);
  SSMenuCB.o_clearvst_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.option_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.o_clearvst_pb, XmNactivateCallback, 
    SynMenu_ClearVisit_CB, (XtPointer) svcb_p);

  /*  The Clear All Marks push button.  */
  lbl_str = XmStringCreateLocalized (SMU_O_CLEARMRK_PB_LBL);
  SSMenuCB.o_clearmrk_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.option_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.o_clearmrk_pb, XmNactivateCallback, 
    SynMenu_ClearMark_CB, (XtPointer) svcb_p);

  /*----------------------------------------------------------------------*/
  /*  The Search Menu (cascade button and pulldown menu).                 */
  /*----------------------------------------------------------------------*/
  SSMenuCB.search_pdm = XmCreatePulldownMenu (SSMenuCB.menu_bar, 
    "SynMenuPDM", NULL, 0);

  lbl_str = XmStringCreateLocalized (SMU_SEARCH_CPB_LBL);
  SSMenuCB.search_cpb = XtVaCreateManagedWidget ("SynMenuCB", 
    xmCascadeButtonWidgetClass, SSMenuCB.menu_bar, 
    XmNlabelString, lbl_str,
    XmNsubMenuId, SSMenuCB.search_pdm,
    XmNmnemonic, SMU_SRCH_MNEM,
    NULL);  
  XmStringFree (lbl_str);

  /*  The Submit push button.  */
  lbl_str = XmStringCreateLocalized (SMU_S_SUBMIT_PB_LBL);
  SSMenuCB.s_submit_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.search_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.s_submit_pb, XmNactivateCallback, 
    SynMenu_Submit_CB, (XtPointer) svcb_p);

  /*  The Continue Search push button.  */
/* superfluous - appropriated for "Expand marked compound" option
  lbl_str = XmStringCreateLocalized (SMU_S_CONTINUE_CPB_LBL);
*/
  lbl_str = XmStringCreateLocalized ("Expand marked compound");
  SSMenuCB.s_continue_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.search_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
/*
  XtAddCallback (SSMenuCB.s_continue_pb, XmNactivateCallback, 
    SynMenu_SearchCont_CB, (XtPointer) svcb_p);
*/
  XtAddCallback (SSMenuCB.s_continue_pb, XmNactivateCallback, 
    SynMenu_Expand_CB, (XtPointer) svcb_p);

/*  XtSetSensitive (SSMenuCB.s_continue_pb, False); */
/**/  XtSetSensitive (SSMenuCB.s_continue_pb, True);

  lbl_str = XmStringCreateLocalized ("Submit marked compound");
  submarked_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.search_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (submarked_pb, XmNactivateCallback, 
    SynMenu_SubMarked_CB, (XtPointer) svcb_p);

  XtSetSensitive (submarked_pb, True);

  /*----------------------------------------------------------------------*/
  /*  The KBase Menu (cascade button and pulldown menu).                  */
  /*----------------------------------------------------------------------*/
  SSMenuCB.kbase_pdm = XmCreatePulldownMenu (SSMenuCB.menu_bar, 
    "SynMenuPDM", NULL, 0);

  lbl_str = XmStringCreateLocalized (SMU_KBASE_CPB_LBL);
  SSMenuCB.kbase_cpb = XtVaCreateManagedWidget ("SynMenuCB", 
    xmCascadeButtonWidgetClass, SSMenuCB.menu_bar, 
    XmNlabelString, lbl_str,
    XmNsubMenuId, SSMenuCB.kbase_pdm,
    XmNmnemonic, SMU_KBASE_MNEM,
    NULL);  
  XmStringFree (lbl_str);

  /*  The KBase Edit push button.  */
  lbl_str = XmStringCreateLocalized (SMU_K_REDIT_PB_LBL);
  SSMenuCB.k_edit_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.kbase_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.k_edit_pb, XmNactivateCallback, 
    SynMenu_KBaseEdit_CB, (XtPointer) svcb_p);

/**/  XtSetSensitive (SSMenuCB.k_edit_pb, True);

  /*  The KBase Search push button.  */
  lbl_str = XmStringCreateLocalized (SMU_K_SEARCH_PB_LBL);
  SSMenuCB.k_search_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.kbase_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.k_search_pb, XmNactivateCallback, 
    SynMenu_KBaseSearch_CB, (XtPointer) svcb_p);

/**/  XtSetSensitive (SSMenuCB.k_search_pb, True);

  /*  The Singleshot push button.  */
  lbl_str = XmStringCreateLocalized (SMU_K_SGLSHOT_PB_LBL);
  SSMenuCB.k_sglshot_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.kbase_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.k_sglshot_pb, XmNactivateCallback, 
    SynMenu_SglShot_CB, (XtPointer) svcb_p);

  /*  The Multishot push button.  */
  lbl_str = XmStringCreateLocalized (SMU_K_MULTSHOT_PB_LBL);
  SSMenuCB.k_mltshot_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.kbase_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.k_mltshot_pb, XmNactivateCallback, 
    SynMenu_MultiShot_CB, (XtPointer) svcb_p);

/**/  XtSetSensitive (SSMenuCB.k_mltshot_pb, False);

  /*  The AvlComp Edit push button.  */
  lbl_str = XmStringCreateLocalized (SMU_K_AEDIT_PB_LBL);
  SSMenuCB.ka_edit_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.kbase_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.ka_edit_pb, XmNactivateCallback, 
    SynMenu_AvCmpEdit_CB, (XtPointer) svcb_p);

/**/  XtSetSensitive (SSMenuCB.ka_edit_pb, True);

  /*----------------------------------------------------------------------*/
  /*  The Navigate Menu (cascade button and pulldown menu).               */
  /*----------------------------------------------------------------------*/
  SSMenuCB.nav_pdm = XmCreatePulldownMenu (SSMenuCB.menu_bar, 
    "SynMenuPDM", NULL, 0);

  lbl_str = XmStringCreateLocalized (SMU_NAVGT_CPB_LBL);
  SSMenuCB.nav_cpb = XtVaCreateManagedWidget ("SynMenuCB", 
    xmCascadeButtonWidgetClass, SSMenuCB.menu_bar, 
    XmNlabelString, lbl_str,
    XmNsubMenuId, SSMenuCB.nav_pdm,
    XmNmnemonic, SMU_NAV_MNEM,
    NULL);  
  XmStringFree (lbl_str);

  /*  The Path Preview push button.  */
  lbl_str = XmStringCreateLocalized (SMU_N_PTHTPV_PB_LBL);
  acc_str = XmStringCreateLocalized (SMU_PVPTH_ACLBL);
  SSMenuCB.n_pthtpv_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.nav_pdm, 
    XmNaccelerator, SMU_PVPTH_ACCEL,
    XmNacceleratorText, acc_str,
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XmStringFree (acc_str);
  XtAddCallback (SSMenuCB.n_pthtpv_pb, XmNactivateCallback, 
    SynMenu_PreView_CB, (XtPointer) ViewLvl_RxnPV_Get (PstView_PathVLvl_Get 
    (pstv_p)));

  /*  The Top Compound Preview push button.  */
  lbl_str = XmStringCreateLocalized (SMU_N_TOPTPV_PB_LBL);
  acc_str = XmStringCreateLocalized (SMU_PVTOP_ACLBL);
  SSMenuCB.n_toptpv_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.nav_pdm, 
    XmNaccelerator, SMU_PVTOP_ACCEL,
    XmNacceleratorText, acc_str,
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XmStringFree (acc_str);
  XtAddCallback (SSMenuCB.n_toptpv_pb, XmNactivateCallback, 
    SynMenu_PreView_CB, (XtPointer) ViewLvl_RxnPV_Get (PstView_IthVLvl_Get 
    (pstv_p, PVW_LEVEL_TOP)));

  /*  The Middle Target Preview push button.  */
  lbl_str = XmStringCreateLocalized (SMU_N_MIDTPV_PB_LBL);
  acc_str = XmStringCreateLocalized (SMU_PVMID_ACLBL);
  SSMenuCB.n_midtpv_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.nav_pdm, 
    XmNaccelerator, SMU_PVMID_ACCEL,
    XmNacceleratorText, acc_str,
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XmStringFree (acc_str);
  XtAddCallback (SSMenuCB.n_midtpv_pb, XmNactivateCallback, 
    SynMenu_PreView_CB, (XtPointer) ViewLvl_RxnPV_Get (PstView_IthVLvl_Get 
    (pstv_p, PVW_LEVEL_MID)));


  /*  The Bottom Target Preview push button.  */
  lbl_str = XmStringCreateLocalized (SMU_N_BTMTPV_PB_LBL);
  acc_str = XmStringCreateLocalized (SMU_PVBTM_ACLBL);
  SSMenuCB.n_btmtpv_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.nav_pdm, 
    XmNaccelerator, SMU_PVBTM_ACCEL,
    XmNacceleratorText, acc_str,
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XmStringFree (acc_str);
  XtAddCallback (SSMenuCB.n_btmtpv_pb, XmNactivateCallback, 
    SynMenu_PreView_CB, (XtPointer) ViewLvl_RxnPV_Get (PstView_IthVLvl_Get 
    (pstv_p, PVW_LEVEL_BTM)));

  /*  The Compound Instances push button.  */
  lbl_str = XmStringCreateLocalized (SMU_N_CMPINST_PB_LBL);
  SSMenuCB.n_cmpinst_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.nav_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.n_cmpinst_pb, XmNactivateCallback, 
    SynMenu_CmpInst_CB, (XtPointer) svcb_p);

/*  XtSetSensitive (SSMenuCB.n_cmpinst_pb, False);  */

  /*  The Select Trace push button.  */
  lbl_str = XmStringCreateLocalized (SMU_N_SELTRACE_PB_LBL);
  SSMenuCB.n_seltrace_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.nav_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.n_seltrace_pb, XmNactivateCallback, 
    SynMenu_SelTrace_CB, (XtPointer) svcb_p);


  /*----------------------------------------------------------------------*/
  /*  The Info Menu (cascade button and pulldown menu).                  */
  /*----------------------------------------------------------------------*/
  SSMenuCB.info_pdm = XmCreatePulldownMenu (SSMenuCB.menu_bar, 
    "SynMenuPDM", NULL, 0);

  lbl_str = XmStringCreateLocalized (SMU_INFO_CPB_LBL);
  SSMenuCB.info_cpb = XtVaCreateManagedWidget ("SynMenuCB", 
    xmCascadeButtonWidgetClass, SSMenuCB.menu_bar, 
    XmNlabelString, lbl_str,
    XmNsubMenuId, SSMenuCB.info_pdm,
    XmNmnemonic, SMU_INFO_MNEM,
    NULL);  
  XmStringFree (lbl_str);

  /*  The Run Info push button.  */
  lbl_str = XmStringCreateLocalized (SMU_I_RUNINFO_PB_LBL);
  SSMenuCB.i_runinfo_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.info_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.i_runinfo_pb, XmNactivateCallback, 
    SynMenu_Info_CB, (XtPointer) SMU_INFO_RUN_TAG);

  /*  The Compond Info push button.  */
  lbl_str = XmStringCreateLocalized (SMU_I_CMPINFO_PB_LBL);
  SSMenuCB.i_cmpinfo_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.info_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.i_cmpinfo_pb, XmNactivateCallback, 
    SynMenu_Info_CB, (XtPointer) SMU_INFO_CMP_TAG);

  /*  The SubGoal Info push button.  */
  lbl_str = XmStringCreateLocalized (SMU_I_SGINFO_PB_LBL);
  SSMenuCB.i_sginfo_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.info_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.i_sginfo_pb, XmNactivateCallback, 
    SynMenu_Info_CB, (XtPointer) SMU_INFO_SG_TAG);

 /*  The Reaction Info push button.  */
  lbl_str = XmStringCreateLocalized (SMU_I_RXNINFO_PB_LBL);
  SSMenuCB.i_rxninfo_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.info_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.i_rxninfo_pb, XmNactivateCallback, 
    SynMenu_Info_CB, (XtPointer) SMU_INFO_RXN_TAG);

/**//* XtSetSensitive (SSMenuCB.r_info_pb, False); */

  /*  The Comment Reaction push button.  */
  lbl_str = XmStringCreateLocalized (SMU_I_COMMENT_PB_LBL);
  SSMenuCB.i_rxncmmt_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.info_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.i_rxncmmt_pb, XmNactivateCallback, 
    SynMenu_RxnCmmt_CB, (XtPointer) NULL);

 /**/  XtSetSensitive (SSMenuCB.i_rxncmmt_pb, False);

  /*----------------------------------------------------------------------*/
  /*  The Help Menu (cascade button and pulldown menu).                   */
  /*----------------------------------------------------------------------*/
  SSMenuCB.help_pdm = XmCreatePulldownMenu (SSMenuCB.menu_bar, 
    "SynMenuPDM", NULL, 0);

  lbl_str = XmStringCreateLocalized (SMU_HELP_CPB_LBL);
  SSMenuCB.help_cpb = XtVaCreateManagedWidget ("SynMenuCB", 
    xmCascadeButtonWidgetClass, SSMenuCB.menu_bar, 
    XmNlabelString, lbl_str,
    XmNsubMenuId, SSMenuCB.help_pdm,
    XmNmnemonic, SMU_HELP_MNEM,
    NULL);  
  XmStringFree (lbl_str);

/**/  XtSetSensitive (SSMenuCB.help_cpb, True);

  /*  The Intro push button.  */
  lbl_str = XmStringCreateLocalized (SMU_H_INTRO_PB_LBL);
  SSMenuCB.h_intro_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.help_pdm, 
    XmNlabelString, lbl_str, 
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.h_intro_pb, XmNactivateCallback, 
/*
    SynMenu_Help_CB, (XtPointer) SMU_HELP_INTRO_TAG);
*/
    Help_CB, (XtPointer) "syn_view:Synchem Executive");

/**/  XtSetSensitive (SSMenuCB.h_intro_pb, True);

  /*  The Manual push button.  */
  lbl_str = XmStringCreateLocalized (SMU_H_MANUAL_PB_LBL);
  SSMenuCB.h_man_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.help_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.h_man_pb, XmNactivateCallback, 
    SynMenu_Help_CB, (XtPointer) SMU_HELP_MANUAL_TAG);

/**/  XtSetSensitive (SSMenuCB.h_man_pb, False);

  /*  The Context push button.  */
  lbl_str = XmStringCreateLocalized (SMU_H_CTXT_PB_LBL);
  SSMenuCB.h_context_pb = XtVaCreateManagedWidget ("SynMenuPB", 
    xmPushButtonWidgetClass, SSMenuCB.help_pdm, 
    XmNlabelString, lbl_str,
    NULL);  
  XmStringFree (lbl_str);
  XtAddCallback (SSMenuCB.h_context_pb, XmNactivateCallback, 
    SynMenu_Help_CB, (XtPointer) SMU_HELP_CONTEXT_TAG);

/**/  XtSetSensitive (SSMenuCB.h_context_pb, False);

  /*----------------------------------------------------------------------*/
  /*  Set the Help Button widget in the Menu Bar.
  */
  XtVaSetValues (SSMenuCB.menu_bar,
    XmNmenuHelpWidget, SSMenuCB.help_cpb,
    NULL);

  return (SSMenuCB.menu_bar);
}
/*  End of SynMenu_Create  */


/****************************************************************************
*
*  Function Name:                 SynMenu_Destroy
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
void SynMenu_Destroy
  (
  void
  )
{

  return ;
}
/*  End of SynMenu_Destroy  */

/****************************************************************************
*
*  Function Name:                 SynMenu_ClearMark_CB
*
*    This routine clears all of the marks on compound nodes in the PST.
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
void SynMenu_ClearMark_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p
  )
{
  SynViewCB_t   *svcb_p;
  PstView_t     *pv_p;
  PsvCmpInfo_t  *cmp_infos;               /* Compound info records */
  U32_t          cmp_i;

  svcb_p = (SynViewCB_t *) client_p;
  if (svcb_p == NULL) 
    return ;

  pv_p = SynVCB_PstView_Get (svcb_p);
  if (pv_p == NULL) 
    return;

  cmp_infos = PstView_CmpInfo_Get (pv_p);
  if (cmp_infos == NULL) 
    return ;

  PstView_Mouse_Remove (pv_p);
  for (cmp_i = 0; cmp_i < PstView_NumCmpI_Get (pv_p); cmp_i++)
    {
    PsvCmpI_Marked_Unset (cmp_infos[cmp_i]);
    PsvCmpI_MarkElse_Unset (cmp_infos[cmp_i]);
    }

  PstView_Display (pv_p);

  return ;
}
/*  End of SynMenu_ClearMark_CB  */

/****************************************************************************
*
*  Function Name:                 SynMenu_ClearVisit_CB
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
void SynMenu_ClearVisit_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p
  )
{
  SynViewCB_t   *svcb_p;
  PstView_t     *pv_p;
  PsvTreeLvl_t  *tlvl_p;                  /* Pst tree level */
  PsvCmpInfo_t  *cmp_infos;               /* Compound info records */
  U32_t          cmp_i;                   /* Ith compound info record */
  U16_t          lvl_i;

  svcb_p = (SynViewCB_t *) client_p;
  if (svcb_p == NULL) 
    return ;

  pv_p = SynVCB_PstView_Get (svcb_p);
  if (pv_p == NULL) 
    return ;

  cmp_infos = PstView_CmpInfo_Get (pv_p);
  if (cmp_infos == NULL) 
    return ;

  PstView_Mouse_Remove (pv_p);
  for (cmp_i = 0; cmp_i < PstView_NumCmpI_Get (pv_p); cmp_i++)
    {
    PsvCmpI_Visited_Unset (cmp_infos[cmp_i]);
    PsvCmpI_VstElse_Unset (cmp_infos[cmp_i]);
    }

  for (lvl_i = 0; lvl_i < PstView_PathLen_Get (pv_p); lvl_i++)
    {
    tlvl_p = PstView_IthPTLvl_Get (pv_p, lvl_i);
    if (TreeLvl_SelNd_Get (tlvl_p) != PVW_NODE_NONE)
      {
      PstView_Visit_Store (PstView_CmpInfo_Get (pv_p), 
        TreeLvl_IthCmpNd_Get (tlvl_p, TreeLvl_SelNd_Get (tlvl_p)));
      }
    }

  PstView_Display (pv_p);

  return ;
}
/*  End of SynMenu_ClearVisit_CB  */

/****************************************************************************
*
*  Function Name:                 SynMenu_CmpInst_CB
*
*    This routine simply manages the compound instances dialog.  
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
void SynMenu_CmpInst_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  SynViewCB_t    *svcb_p;
  PstView_t      *pv_p;
  CmpInst_t      *cinst_p;
  Position        parent_xpos;
  Position        cinst_pos;
  Dimension       cinst_width;

  svcb_p = (SynViewCB_t *) client_p;
  pv_p = SynVCB_PstView_Get (svcb_p);

  if (svcb_p == NULL) 
    return ;

  pv_p = SynVCB_PstView_Get (svcb_p);
  cinst_p = PstView_CmpInsts_Get (pv_p);

  if (!CmpInst_IsCreated_Get (cinst_p))
    {
    CmpInst_Create (PstView_Form_Get (pv_p), pv_p);
    CmpInst_Path_Put (glob_path);
    }

  else if (XtIsManaged (CmpInst_FormDlg_Get (cinst_p)))
    {
    return;
    }


  XtVaGetValues (SynVCB_TopApp_Get (svcb_p),
    XmNx, &parent_xpos, 
    NULL);

  XtVaGetValues (CmpInst_FormDlg_Get (cinst_p),
    XmNwidth, &cinst_width,
    NULL);
    
  if (parent_xpos + AppDim_AppWidth_Get (&GAppDim) + cinst_width >
      Screen_Width_Get (SynAppR_ScrnAtrb_Get (&GSynAppR)))
    cinst_pos = Screen_Width_Get (SynAppR_ScrnAtrb_Get (&GSynAppR)) 
      - cinst_width;
  else
    cinst_pos = parent_xpos + AppDim_AppWidth_Get (&GAppDim);

  XtVaSetValues (CmpInst_FormDlg_Get (cinst_p),
  XmNdefaultPosition, False,
  XmNx, cinst_pos,
  XmNy, 0,
  NULL);

  XtManageChild (CmpInst_FormDlg_Get (cinst_p));

  return ;
}
/*  End of SynMenu_CmpInst_CB  */


/****************************************************************************
*
*  Function Name:                 SynMenu_Exit_CB
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
void SynMenu_Exit_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  /*  Check to make sure that the status and pfiles have been saved.
      If not, warn user and ignore exit.  
  */
  if (Rcb_Flags_SaveStatus_Get (&GGoalRcb) 
    && RunStats_Flags_PstChanged_Get (&GRunStats))
    {
    SVF_QueryDg_Update (SVF_QUERY_TYPE_SAVESTAT, SVF_COMMAND_TYPE_EXIT, NULL);
    return;
    }

  SynView_Exit ();

  return;
 }
/*  End of SynMenu_Exit_CB  */

/****************************************************************************
*
*  Function Name:                 SynMenu_File_CB
*
*    This routine.
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
void SynMenu_File_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  PstView_t     *pv_p;
  char          *tag;
  Boolean_t      save_ok;
  tag = (char *) client_p;

  pv_p = SynVCB_PstView_Get (SynViewCB_P);
  if (pv_p == NULL) 
    return;

  if (strcmp (tag, SMU_FILE_OPEN_TAG) == 0)
    {
    SVF_FileDg_Update (SVF_FILED_TYPE_OPENSTAT, NULL,
    SynVCB_FlagParallel_Get (SynViewCB_P));

    return;
    }  /* End of if open status file */

  else if (strcmp (tag, "D") == 0)
    {
    SVF_FileDg_Update ("DeleteRun", NULL,
    SynVCB_FlagParallel_Get (SynViewCB_P));

    return;
    }  /* End of if delete a run */

  else if (strcmp (tag, SMU_FILE_SAVE_TAG) == 0)
    {
    Display       *dsp;

    dsp = XtDisplay (SynVCB_TopApp_Get (SynViewCB_P));
    XDefineCursor (dsp, XtWindow (SynVCB_TopApp_Get (SynViewCB_P)), 
    SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_WAIT_W));
    XFlush (dsp);

    write_fail_fatal = FALSE;
    write_fail_str[0] = '\0';
    Status_File_Write ((char *)String_Value_Get (FileCB_FileStr_Get (
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_STAT))), &GGoalRcb,
      &GRunStats);

    if (save_ok = write_fail_str[0] == '\0')
      {
      RunStats_Flags_StatusSaved_Put (&GRunStats, TRUE);
      RunStats_Flags_PstChanged_Put (&GRunStats, FALSE);
      }

    XUndefineCursor (dsp, XtWindow (SynVCB_TopApp_Get (SynViewCB_P)));
    if (!save_ok)
      {
      InfoWarn_Show (write_fail_str);
      write_fail_str[0] = '\0';
      }
    return;
    }  /* End of if save status file */

  else if (strcmp (tag, SMU_FILE_SAVECVR_TAG) == 0)
    {
    char          *seqcoverfile;
    FILE          *f_p;
    Compound_t    *cmp_p;
    Compound_t    *else_p;
    PsvCmpInfo_t  *cmpinfos;
    PstCB_t       *pstcb_p;
    U32_t          cycle_i;
    U32_t          num_cycles;
    char           mess[128];
    Boolean_t      non_solved;
    Boolean_t      marked;

    pstcb_p = PstView_PstCB_Get (pv_p);
    cmpinfos = PstView_CmpInfo_Get (pv_p);
    if (pstcb_p == NULL || cmpinfos == NULL)
      {
      InfoWarn_Show ("No developed tree to save.");
      return ;
      }

    num_cycles = PstCB_TotalExpandedCompounds_Get (pstcb_p);
    if (num_cycles == 0)
      {
      InfoWarn_Show ("No developed tree to save.");
      return ;
      }

    seqcoverfile = (char *) String_Value_Get (FileCB_FileStr_Get ( 
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_COVR)));
#ifdef _WIN32
    f_p = fopen (gccfix (seqcoverfile), "w");
#else
    f_p = fopen (seqcoverfile, "w");
#endif
    if (f_p == NULL)
      {
      sprintf (mess, "Unable to open file:  %s", seqcoverfile);
      InfoWarn_Show (mess);
      return ;
      }

    /*  Store the number of cycles in the file.  Then get each compound
        that was selected for expansion (has been developed).  Check 
        whether the compound has been marked, and if so whether it has
        been solved.  Store the sling in the file with the proper tag.
    */
    fprintf (f_p, "%lu\n", num_cycles);
    non_solved = FALSE;
    cmp_p = PstSubg_Son_Get (PstCB_Root_Get (pstcb_p));
    for (cycle_i = 0; cycle_i < num_cycles && cmp_p != NULL; cycle_i++)
      {
      /*  Check whether an instance in the PST of this compound has
          been marked.  If so, make sure the compound has been solved.
      */
      else_p = SymTab_FirstComp_Get (PstComp_SymbolTable_Get (cmp_p));
      marked = FALSE;
      while (else_p != NULL && !marked)
        {
        if (PsvCmpI_Marked_Get (cmpinfos[PstComp_Index_Get (else_p)]))
          marked = TRUE;
        else
          else_p = PstComp_Prev_Get (else_p);
        }

      if (marked)
        {
        if (SymTab_Flags_Solved_Get (PstComp_SymbolTable_Get (cmp_p)))
          fprintf (f_p, "S%s\n", (char *) Sling_Name_Get (SymTab_Sling_Get (
             PstComp_SymbolTable_Get (cmp_p))));
        else
          non_solved = TRUE;
        }
      else
        {
        fprintf (f_p, "D%s\n", (char *) Sling_Name_Get (SymTab_Sling_Get (
           PstComp_SymbolTable_Get (cmp_p))));
        }

      cmp_p = PstComp_Next_Get (cmp_p);
      }

    SynMenu_Path_CB (w, (XtPointer)SMU_PATH_WRITE_TAG, NULL);

    if (non_solved)
      InfoWarn_Show (
        "Warning:  a marked node was not solved --ignored.");
    else
      InfoMess_Show ("Developed search space file saved.");

    fclose (f_p);
    return ;
    }  /* End of else if save developed compounds */

  else if (strcmp (tag, SMU_FILE_PRINT_WIN_TAG) == 0 
      || strcmp (tag, SMU_FILE_PRINT_RXN_TAG) == 0)
    {
    char          *remote_display;
    char          *printer;
    Window         win;       
    char           commandline[255];

    if (SynAppR_Printer_Get (&GSynAppR) != NULL)
      printer = SynAppR_Printer_Get (&GSynAppR);
    else
      printer = SAR_PRINTER_DFLT;

    if (SynAppR_RemoteDisp_Get (&GSynAppR) != NULL)
      remote_display = SynAppR_RemoteDisp_Get (&GSynAppR);
    else
      remote_display = SAR_REMOTEDISLAY_DFLT;

    if (strcmp (tag, SMU_FILE_PRINT_WIN_TAG) == 0)
      win = XtWindow (SynVCB_TopApp_Get (SynViewCB_P)); 
    else
      win = XtWindow (RxnView_Frame_Get (PstView_RxnView_Get (pv_p))); 

#ifdef  _IRIX64_OS_
    if (SynAppR_RemoteDisp_Get (&GSynAppR) == NULL)
      sprintf (commandline, "%s%s%s%lu%s%s\"%s (%s)\"", SAR_PRINT_COMMAND,
        printer, SAR_PRINT_WINDOW_ID, win, SAR_PRINT_OPTIONS, SAR_PRINT_TAIL,
        (char *) String_Value_Get (Rcb_Name_Get (&GGoalRcb)),
        (char *) String_Value_Get (Rcb_Comment_Get (&GGoalRcb)));
    else
      sprintf (commandline, "%s%s%s%lu%s%s%s%s\"%s (%s)\"", SAR_PRINT_COMMAND,
        printer, SAR_PRINT_WINDOW_ID, win, SAR_PRINT_DISPLAY, 
        SynAppR_RemoteDisp_Get (&GSynAppR), SAR_PRINT_OPTIONS, SAR_PRINT_TAIL,
        (char *) String_Value_Get (Rcb_Name_Get (&GGoalRcb)),
        (char *) String_Value_Get (Rcb_Comment_Get (&GGoalRcb)));

#else
#ifdef  _IRIX62_OS_
    if (SynAppR_RemoteDisp_Get (&GSynAppR) == NULL)
      sprintf (commandline, "%s%s%s%lu%s%s\"%s (%s)\"", SAR_PRINT_COMMAND,
        printer, SAR_PRINT_WINDOW_ID, win, SAR_PRINT_OPTIONS, SAR_PRINT_TAIL,
        (char *) String_Value_Get (Rcb_Name_Get (&GGoalRcb)),
        (char *) String_Value_Get (Rcb_Comment_Get (&GGoalRcb)));
    else
      sprintf (commandline, "%s%s%s%lu%s%s%s%s\"%s (%s)\"", SAR_PRINT_COMMAND,
        printer, SAR_PRINT_WINDOW_ID, win, SAR_PRINT_DISPLAY, 
        SynAppR_RemoteDisp_Get (&GSynAppR), SAR_PRINT_OPTIONS, SAR_PRINT_TAIL,
        (char *) String_Value_Get (Rcb_Name_Get (&GGoalRcb)),
        (char *) String_Value_Get (Rcb_Comment_Get (&GGoalRcb)));

#else

    sprintf (commandline, "%s%s%lu%s%s%s", SAR_PRINT_COMMAND,
      SAR_PRINT_WINDOW_ID, win, SAR_PRINT_OPTIONS, SAR_PRINT_TAIL, printer);

#endif
#endif

#ifdef _DEBUG_
debug_print(commandline);
#endif
    system (commandline); 
    return ;
    }

  return ;
}
/*  End of SynMenu_File_CB  */

/****************************************************************************
*
*  Function Name:                 SynMenu_Help_CB
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
void SynMenu_Help_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  
  return ;
}
/*  End of SynMenu_Help_CB  */

/****************************************************************************
*
*  Function Name:                 SynMenu_Info_CB
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
void SynMenu_Info_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  char        *tag;
  InfoWin_t   *iw_p;
  tag = (char *) client_p;

  if (strcmp (tag, SMU_INFO_RUN_TAG) == 0)
    {
    InfoWin_Status_Update (&GGoalRcb, &GRunStats, NULL);
    }

  else if (strcmp (tag, SMU_INFO_CMP_TAG) == 0)
    {
    iw_p = InfoWin_Handle_Get (SVI_INFO_COMPOUND);

    if (iw_p == NULL || XtIsManaged (InfoWin_InfoDlg_Get (*iw_p)))
      return;

    *InfoWin_TextBuf_Get (*iw_p) = '\0';

    XtManageChild (InfoWin_InfoDlg_Get (*iw_p));
    }

  else if (strcmp (tag, SMU_INFO_SG_TAG) == 0)
    {
    iw_p = InfoWin_Handle_Get (SVI_INFO_SUBGOAL);

    if (iw_p == NULL || XtIsManaged (InfoWin_InfoDlg_Get (*iw_p)))
      return;

    *InfoWin_TextBuf_Get (*iw_p) = '\0';

    XtManageChild (InfoWin_InfoDlg_Get (*iw_p));
    }

  else if (strcmp (tag, SMU_INFO_RXN_TAG) == 0)
    {
    iw_p = InfoWin_Handle_Get (SVI_INFO_RXN);

    InfoWin_Reaction_Update ();
    }


  return ;
}
/*  End of SynMenu_Info_CB  */

/****************************************************************************
*
*  Function Name:                 SynMenu_KBaseEdit_CB
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
void RxlEdit_Cont ()
{
  char command[768];

if (glob_norxn) sprintf(command,"%srxledit",glob_path);
else
  if (login_failed)
    sprintf(command,"%srxledit %d \"%s\" \"\"",glob_path,glob_rxn_key,Sling_Name_Get(glob_sling));
  else
    sprintf(command,"%srxledit %d \"%s\" \"%s\"",glob_path,glob_rxn_key,Sling_Name_Get(glob_sling),UserId());
/*
printf("command=\"%s\"\n",command);
*/
if (!glob_norxn)
  Sling_Destroy (glob_sling);
#ifdef _DEBUG_
debug_print(command);
#endif
  system(command);
  
  return ;
}

void SynMenu_KBaseEdit_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  SynViewCB_t    *svcb_p;
  SShotView_t    *ssv_p;
  PstView_t      *pv_p;
  PsvTreeLvl_t   *tlvl_p;                  /* Pst tree level */
  PsvViewLvl_t   *vlvl_p;                  /* Pst view level */
  Subgoal_t      *sg_p;
  SymTab_t       *symtab_p;

  svcb_p = (SynViewCB_t *) client_p;
  if (svcb_p == NULL) 
    return ;

  ssv_p = SynVCB_SShotView_Get (svcb_p);
  pv_p = SynVCB_PstView_Get (svcb_p);
  if (ssv_p == NULL || pv_p == NULL) 
    return ;

glob_norxn = FALSE;
  /*  Get selected reaction.  */
  vlvl_p = PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM);
  tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
  if (TreeLvl_SelNd_Get (tlvl_p) != PVW_NODE_NONE)
    {
    sg_p = PstComp_Father_Get (TreeLvl_IthCmpNd_Get (tlvl_p, 
      TreeLvl_SelNd_Get (tlvl_p)));
    symtab_p = PstComp_SymbolTable_Get (PstSubg_Father_Get (sg_p));
    glob_rxn_key = PstSubg_Reaction_Schema_Get (sg_p);
    }
  else 
    {
    vlvl_p = PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID);
    tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
    if (TreeLvl_SelNd_Get (tlvl_p) != PVW_NODE_NONE)
      {
      sg_p = PstComp_Father_Get (TreeLvl_IthCmpNd_Get (tlvl_p, 
        TreeLvl_SelNd_Get (tlvl_p)));
      symtab_p = PstComp_SymbolTable_Get (PstSubg_Father_Get (sg_p));
      glob_rxn_key = PstSubg_Reaction_Schema_Get (sg_p);
      }
    else
      {
/*
      InfoWarn_Show (SSV_WARN_NORXN);
*/
glob_norxn = TRUE;
RxlEdit_Cont ();
      return;
      }
    }

  glob_sling = Sling_CopyTrunc (SymTab_Sling_Get (symtab_p));

  if (logged_in) RxlEdit_Cont ();
  else
    {
    if (login_failed) LoginFail ((Widget) toplevel);
    else LoginOrCancel ((Widget) toplevel, RxlEdit_Cont, NULL);
    }
}

/*  End of SynMenu_KBaseEdit_CB  */

/****************************************************************************
*
*  Function Name:                 SynMenu_AvCmpEdit_CB
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
void AvlEdit_Cont ()
{
  char command[768];
  static Boolean_t first_avl = TRUE;

if (glob_norxn) sprintf(command,"%savledit",glob_path);
else
  if (login_failed)
    sprintf(command,"%savledit \"%s\" \"\" %s",glob_path,Sling_Name_Get(glob_sling),first_avl?"yes":"no");
  else
    sprintf(command,"%savledit \"%s\" \"%s\" %s",glob_path,Sling_Name_Get(glob_sling),UserId(),first_avl?"yes":"no");
  first_avl = FALSE;
/*
printf("command=\"%s\"\n",command);
*/
if (!glob_norxn)
  Sling_Destroy (glob_sling);
#ifdef _DEBUG_
debug_print(command);
#endif
  system(command);

if (!glob_norxn)
  PstView_Set_Avledit_Called (); /* convoluted way of getting pst_view to get rxn_view to update!!! */
  
  return ;
}

void SynMenu_AvCmpEdit_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  SynViewCB_t    *svcb_p;
  SShotView_t    *ssv_p;
  PstView_t      *pv_p;
  PsvTreeLvl_t   *tlvl_p;                  /* Pst tree level */
  PsvViewLvl_t   *vlvl_p;                  /* Pst view level */
  SymTab_t       *symtab_p;

  svcb_p = (SynViewCB_t *) client_p;
  if (svcb_p == NULL) 
    return ;

  ssv_p = SynVCB_SShotView_Get (svcb_p);
  pv_p = SynVCB_PstView_Get (svcb_p);
  if (ssv_p == NULL || pv_p == NULL) 
    return ;

glob_norxn = FALSE;
  /*  Get selected reaction.  */
  vlvl_p = PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM);
  tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
  if (TreeLvl_SelNd_Get (tlvl_p) != PVW_NODE_NONE)
    symtab_p = PstComp_SymbolTable_Get (TreeLvl_IthCmpNd_Get (tlvl_p, 
    TreeLvl_SelNd_Get (tlvl_p)));
  else 
    {
    vlvl_p = PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID);
    tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
    if (TreeLvl_SelNd_Get (tlvl_p) != PVW_NODE_NONE)
      symtab_p = PstComp_SymbolTable_Get (TreeLvl_IthCmpNd_Get (tlvl_p, 
      TreeLvl_SelNd_Get (tlvl_p)));
    else
      {
/*
      InfoWarn_Show (SSV_WARN_NORXN);
*/
glob_norxn = TRUE;
AvlEdit_Cont ();
      return;
      }
    }

  glob_sling = Sling_CopyTrunc (SymTab_Sling_Get (symtab_p));

  if (logged_in) AvlEdit_Cont ();
  else
    {
    if (login_failed) LoginFail ((Widget) toplevel);
    else LoginOrCancel ((Widget) toplevel, AvlEdit_Cont, NULL);
    }
  
  return ;
}
/*  End of SynMenu_AvCmpEdit_CB  */

/****************************************************************************
*
*  Function Name:                 SynMenu_PathTrc_CB
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
void SynMenu_PathTrc_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  PstView_t     *pv_p;
  PsvCmpInfo_t  *cmpinfos;
  PstCB_t       *pstcb_p;
  char commmsg[768], sfname[256], pfname[256], *slash, *dot;
  static Boolean_t first_time = TRUE;

  pv_p = SynVCB_PstView_Get (SynViewCB_P);
  if (pv_p == NULL) 
    return;

  if (client_p == NULL && !first_time)
  {
    first_time = TRUE; /* allow for cancel */
    return;
  }

  pstcb_p = PstView_PstCB_Get (pv_p);
  cmpinfos = PstView_CmpInfo_Get (pv_p);
  if (pstcb_p == NULL || cmpinfos == NULL)
    {
    InfoWarn_Show ("No PST to view.");
    return ;
    }

  if (first_time)
  {
    first_time = FALSE;
    SVF_FileDg_Update (SVF_FILED_TYPE_PATHTRC, NULL,
      SynVCB_FlagParallel_Get (SynViewCB_P));
    return;
  }

  first_time = TRUE;
  strcpy (pfname, (char *) client_p);

/*
  strcpy (sfname, (char *) String_Value_Get (FileCB_FileStr_Get ( 
    Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_STAT))));
  dot = strstr (sfname, ".status");
  if (dot == NULL)
  {
    sprintf (commmsg, "Unexpected status filename: %s.\n\nHave you opened a valid file???", sfname);
    InfoWarn_Show (commmsg);
    return;
  }
  *dot = '\0';
  while ((slash = strstr (sfname, "/")) != NULL)
    strcpy (sfname, slash + 1);
*/

/*
  strcpy (pfname, (char *) String_Value_Get (FileCB_FileStr_Get ( 
    Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_PATH))));
*/
/*
  dot = strstr (pfname, ".path");
  if (dot == NULL)
  {
    sprintf (commmsg, "Unexpected path filename: %s.", pfname);
    InfoWarn_Show (commmsg);
    return;
  }
  *dot = '\0';
  while ((slash = strstr (pfname, "/")) != NULL)
    strcpy (pfname, slash + 1);
*/

/*
  sprintf(commmsg,"%spathtrc12 -f %s -p %s",glob_path,sfname,pfname);
  system(commmsg);
*/

printf("before PathTrc_Display\n");
  PathTrc_Display (pfname);
printf("after PathTrc_Display\n");

  return;
}

/****************************************************************************
*
*  Function Name:                 SynMenu_KBaseSearch_CB
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
void SynMenu_KBaseSearch_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  char command[768];
  SynViewCB_t    *svcb_p;
  SShotView_t    *ssv_p;
  PstView_t      *pv_p;
  PsvTreeLvl_t   *tlvl_p;                  /* Pst tree level */
  PsvViewLvl_t   *vlvl_p;                  /* Pst view level */
  Subgoal_t      *sg_p;
  SymTab_t       *symtab_p;
  Sling_t         temp_sling;
  U32_t           rxn_key;

  svcb_p = (SynViewCB_t *) client_p;
  if (svcb_p == NULL) 
    return ;

  ssv_p = SynVCB_SShotView_Get (svcb_p);
  pv_p = SynVCB_PstView_Get (svcb_p);
  if (ssv_p == NULL || pv_p == NULL) 
    return ;

  /*  Get selected reaction.  */
  vlvl_p = PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM);
  tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
  if (TreeLvl_SelNd_Get (tlvl_p) != PVW_NODE_NONE)
    {
    sg_p = PstComp_Father_Get (TreeLvl_IthCmpNd_Get (tlvl_p, 
      TreeLvl_SelNd_Get (tlvl_p)));
    symtab_p = PstComp_SymbolTable_Get (PstSubg_Father_Get (sg_p));
    rxn_key = PstSubg_Reaction_Schema_Get (sg_p);
    }
  else 
    {
    vlvl_p = PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID);
    tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
    if (TreeLvl_SelNd_Get (tlvl_p) != PVW_NODE_NONE)
      {
      sg_p = PstComp_Father_Get (TreeLvl_IthCmpNd_Get (tlvl_p, 
        TreeLvl_SelNd_Get (tlvl_p)));
      symtab_p = PstComp_SymbolTable_Get (PstSubg_Father_Get (sg_p));
      rxn_key = PstSubg_Reaction_Schema_Get (sg_p);
      }
    else
      {
      InfoWarn_Show (SSV_WARN_NORXN);
      return;
      }
    }

  temp_sling = Sling_CopyTrunc (SymTab_Sling_Get (symtab_p));

  sprintf(command,"%srxledit %d \"%s\" \"\" %x %x",glob_path,rxn_key,Sling_Name_Get(temp_sling),glob_run_date,glob_run_time);
/*
printf("command=\"%s\"\n",command);
*/
  Sling_Destroy (temp_sling);
#ifdef _DEBUG_
debug_print(command);
#endif
  system(command);
  
  return ;
}
/*  End of SynMenu_KBaseSearch_CB  */

/****************************************************************************
*
*  Function Name:                 SynMenu_MultiShot_CB
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
void SynMenu_MultiShot_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  SynViewCB_t    *svcb_p;
  SShotView_t    *ssv_p;
  PstView_t      *pv_p;
/*
  PsvTreeLvl_t   *tlvl_p;
  PsvViewLvl_t   *vlvl_p;
  Subgoal_t      *sg_p;
  SymTab_t       *symtab_p;
  Sling_t         temp_sling;
  U32_t           rxn_key;
*/

  svcb_p = (SynViewCB_t *) client_p;
  if (svcb_p == NULL) 
    return ;

  ssv_p = SynVCB_SShotView_Get (svcb_p);
  pv_p = SynVCB_PstView_Get (svcb_p);
  if (ssv_p == NULL || pv_p == NULL) 
    return ;

  
  return ;
}
/*  End of SynMenu_MultiShot_CB  */

/****************************** find_marked *********************************
Recursive function for a more contextual save of the marked path.
****************************************************************************/
Boolean_t find_marked (FILE *f_p, Compound_t *c_p, PsvCmpInfo_t *ciparm, char *message)
{
  Compound_t *conjuncts[10], *cmp_p;
  SymTab_t *sym_p;
  Subgoal_t *subg_p;
  U32_t nconj, conj_i, conj_j, merits[10], tm, indices[10], ti, cli;
  Boolean_t changed, conj_marked[10], tcm, valid;
  static Boolean_t mark_found = FALSE;
  static PsvCmpInfo_t *cmpinfos = NULL;               /* Compound info records */
  static level = 0;
  static int ncomps = 0;
  static Compound_t **comp_list = NULL;

  level++;

  if (ciparm != NULL) cmpinfos = ciparm;

  for (cmp_p = c_p, nconj = 0; cmp_p != NULL; cmp_p = PstComp_Brother_Get (cmp_p))
  {
    for (cli = 0; cli < ncomps; cli++)
      if (comp_list[cli] == cmp_p)
    {
      strcpy (message, "WCircularity discovered - aborting.\n\nPath file invalid or incomplete.");
      level--;
      return (FALSE);
    }
    conjuncts[nconj] = cmp_p;
    indices[nconj] = PstComp_Index_Get (cmp_p);
    if (conj_marked[nconj] = PsvCmpI_Marked_Get (cmpinfos[indices[nconj]])) mark_found = TRUE;
    sym_p = PstComp_SymbolTable_Get (cmp_p);
    merits[nconj++] =
      SymTab_Flags_Solved_Get (sym_p) ? SymTab_Merit_Solved_Get (sym_p) : SymTab_Merit_Main_Get (sym_p);
  }

  for (conj_i = nconj - 1, changed = TRUE; conj_i > 0 && changed; conj_i--)
  {
    changed = FALSE;
    for (conj_j = 0; conj_j < conj_i; conj_j++)
      if (merits[conj_j] > merits[conj_j + 1]) /* move highest merit to the end */
    {
      changed = TRUE;
      tm = merits[conj_i];
      ti = indices[conj_i];
      tcm = conj_marked[conj_i];
      cmp_p = conjuncts[conj_i];
      merits[conj_i] = merits[conj_j];
      indices[conj_i] = indices[conj_j];
      conj_marked[conj_i] = conj_marked[conj_j];
      conjuncts[conj_i] = conjuncts[conj_j];
      merits[conj_j] = tm;
      indices[conj_j] = ti;
      conj_marked[conj_j] = tcm;
      conjuncts[conj_j] = cmp_p;
    }
  }

  comp_list = (Compound_t **) realloc (comp_list, (ncomps + nconj) * sizeof (Compound_t *));
  for (conj_i = 0; conj_i < nconj; conj_i++)
    comp_list[ncomps++] = conjuncts[conj_i];

  for (conj_i = 0, valid = TRUE; conj_i < nconj && valid; conj_i++)
    if (!mark_found || conj_marked[conj_i])
  {
    sym_p = PstComp_SymbolTable_Get (conjuncts[conj_i]);
    cmp_p = SymTab_DevelopedComp_Get (sym_p);
    if (cmp_p != NULL)
    {
      subg_p = PstComp_Son_Get (cmp_p);
      while (valid && subg_p != NULL)
      {
        if (!find_marked (f_p, PstSubg_Son_Get (subg_p), NULL, message))
          valid = FALSE;
        else
          subg_p = PstSubg_Brother_Get (subg_p);
      }
    }
    if (valid && conj_marked[conj_i])
    {
      fprintf (f_p, "%09lu\n", indices[conj_i]); /* leading zero flags as PathTrc-friendly */
      conj_marked[conj_i] = FALSE;
    }
  }

  level--;

  if (level == 0)
  {
    free (comp_list);
    comp_list = NULL;
    cmpinfos = NULL;
    ncomps = 0;
  }
  else
  {
    ncomps -= nconj;
    comp_list = (Compound_t **) realloc (comp_list, ncomps * sizeof (Compound_t *));
  }

  return (valid);
}

/****************************************************************************
*
*  Function Name:                 SynMenu_Path_CB
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
void SynMenu_Path_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  PstView_t     *pv_p;
  char          *tag, tfilename[50];
  static char message[512];
  static Boolean_t first_time = TRUE;

  tag = (char *) client_p;
  if (tag == NULL) 
  {
    first_time = TRUE; /* allow for cancel */
    return;
  }

  pv_p = SynVCB_PstView_Get (SynViewCB_P);
  if (pv_p == NULL) 
    return;

#ifdef _WIN32
  sprintf (tfilename, "temp.pth.%s", session_code);
#else
  sprintf (tfilename, "/tmp/temp.pth.%s", session_code);
#endif

  if (strcmp (tag, SMU_PATH_READ_TAG) == 0)
    {
    PsvCmpInfo_t  *cmpinfos;
    PstCB_t       *pstcb_p;
    FILE          *f_p;
    U32_t          cmp_index;
    char           buff[MX_INPUT_SIZE+1];
    char           mess[128];
    char           command[128];

    pstcb_p = PstView_PstCB_Get (pv_p);
    cmpinfos = PstView_CmpInfo_Get (pv_p);
    if (pstcb_p == NULL || cmpinfos == NULL)
      {
      InfoWarn_Show ("No PST to mark.");
      return ;
      }

    if (first_time)
    {
      first_time = FALSE;
      SVF_FileDg_Update (SVF_FILED_TYPE_OPENPTH, (XtPointer) tag,
        SynVCB_FlagParallel_Get (SynViewCB_P));
      return;
    }

    /*  Don't waste time or contribute to further obscurity by
        trying to synchronize with a callback.  Read a temp file
        that callback copied from the proper pathname!!!
        If necessary, this can be cleaned up later through a
        complete rewrite, because one path was not enough!
    */

    first_time = TRUE;

    f_p = fopen (tfilename, "r");

/*
#ifdef _WIN32
    f_p = fopen (gccfix ((char *) String_Value_Get (FileCB_FileStr_Get ( 
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_PATH)))), "r");
#else
    f_p = fopen ((char *) String_Value_Get (FileCB_FileStr_Get ( 
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_PATH))), "r");
#endif
*/
    if (f_p == NULL)
      {
      sprintf (mess, "Unable to open file:\n  %s", 
        tfilename);
/*
        (char *) String_Value_Get (FileCB_FileStr_Get ( 
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_PATH))));
*/
      InfoWarn_Show (mess);
      return ;
      }

    PstView_Mouse_Remove (pv_p);

    for (cmp_index = 0; cmp_index < PstView_NumCmpI_Get (pv_p); cmp_index++)
      {
      PsvCmpI_Marked_Unset (cmpinfos[cmp_index]);
      PsvCmpI_MarkElse_Unset (cmpinfos[cmp_index]);
      }

    while (fgets (buff, MX_INPUT_SIZE, f_p) != NULL)
      {
      cmp_index = (U32_t) atol (buff);
      PsvCmpI_Marked_Set (cmpinfos[cmp_index]);
      }

    PstView_Display (pv_p);
    fclose (f_p);
    sprintf (command, "rm %s", tfilename);
/*
    system (command);
*/
    remove (command + 3);
    return ;
    }  /* End of if read path file */


  else if (strcmp (tag, SMU_PATH_WRITE_TAG) == 0)
    {
    FILE          *f_p;
    Compound_t    *cmp_p;
    Compound_t    *else_p;
    PsvCmpInfo_t  *cmpinfos;
    PstCB_t       *pstcb_p;
    U32_t          cycle_i;
    U32_t          num_cycles;
    char           mess[128];
    Boolean_t      marked;

    pstcb_p = PstView_PstCB_Get (pv_p);
    cmpinfos = PstView_CmpInfo_Get (pv_p);
    if (pstcb_p == NULL || cmpinfos == NULL)
      {
      InfoWarn_Show ("No solved paths to save.");
      return ;
      }

    num_cycles = PstCB_TotalExpandedCompounds_Get (pstcb_p);
    if (num_cycles == 0)
      {
      InfoWarn_Show ("No solved paths to save.");
      return ;
      }

/*
#ifdef _WIN32
    f_p = fopen (gccfix ((char *) String_Value_Get (FileCB_FileStr_Get ( 
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_PATH)))), "w");
#else
    f_p = fopen ((char *) String_Value_Get (FileCB_FileStr_Get ( 
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_PATH))), "w");
#endif
*/
    /*  Don't waste time or contribute to further obscurity by
        trying to synchronize with a callback.  Write a temp file
        and let the callback copy it into the proper pathname!!!
        If necessary, this can be cleaned up later through a
        complete rewrite, because one path was not enough!
    */

    f_p = fopen (tfilename, "w");

    if (f_p == NULL)
      {
      sprintf (mess, "Unable to open file:  %s",
        tfilename);
/*
        (char *) String_Value_Get (FileCB_FileStr_Get ( 
      Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_PATH))));
*/
      InfoWarn_Show (mess);
      return ;
      }

    message[0] = '\0';

    /*  For each compound that has been expanded, check whether the 
        compound has been marked, and if so store the compound index.
    */
    cmp_p = PstSubg_Son_Get (PstCB_Root_Get (pstcb_p));

/*********** This is a more pathtrc-friendly method,************/
    if (PsvCmpI_Marked_Get (cmpinfos[PstComp_Index_Get (cmp_p)]))
      {
      if (find_marked (f_p, cmp_p, cmpinfos, message))
        strcpy (message, "MMarked paths saved in PathTrc-friendly format.");
      }
/*********** but it only works if the target is marked! ********/

    else
      for (cycle_i = 0; cycle_i < num_cycles && cmp_p != NULL; cycle_i++)
      {
      else_p = SymTab_FirstComp_Get (PstComp_SymbolTable_Get (cmp_p));
      marked = FALSE;
      while (else_p != NULL && !marked)
        {
        if (PsvCmpI_Marked_Get (cmpinfos[PstComp_Index_Get (else_p)]))
          marked = TRUE;
        else
          else_p = PstComp_Prev_Get (else_p);
        }

      if (marked)
        {
        fprintf (f_p, "%lu\n", PstComp_Index_Get (else_p));
        }

      cmp_p = PstComp_Next_Get (cmp_p);
      strcpy (message, "MMarked paths saved, but PathTrc may not find the file usable.\n"
                     "(To correct, mark target, have all branches converge onto it,\n"
                     "and mark all paths to completion--INCLUDING available conjuncts.\n"
                     "Note that the PathTrc-friendly save will bypass any disjoint path.)");
      }

    fclose (f_p);

    /* This is where the pathname is specified -- AFTER the file has ALREADY been written
       through logic that belongs is THIS module!
    */

    SVF_FileDg_Update (SVF_FILED_TYPE_SAVEPTH, (XtPointer) message,
      SynVCB_FlagParallel_Get (SynViewCB_P));

    return ;
    }  /* End of else if save path */

  return ;
}
/*  End of SynMenu_Path_CB  */

/****************************************************************************
*
*  Function Name:                 SynMenu_PreView_CB
*
*    This routine manages the previously created reaction previewer
*    if it hasn't already been done so.  It also adds the event
*    handler to the PST tree viewing drawing area, if needed.
*
*
*  Implicit Inputs:
*
*    SynViewCB_P, SynView control block (to get pstview).
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
void SynMenu_PreView_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  PstView_t           *pview_p;
  RxnPreView_t        *rpv_p;
  PsvTreeLvl_t        *tlvl_p;                  /* Pst tree level */
  PsvViewLvl_t        *vlvl_p;                  /* Pst view level */
  XSetWindowAttributes winattr;
  XmString             title;
  Dimension            x_coord, y_coord;
  Dimension            parent_xpos;

  rpv_p = (RxnPreView_t *) client_p;
  if (rpv_p == NULL)
    return ;

  /*  Only manage previewer if it has not already been managed.
      Calculate its postition based on the position of parent.
  */
  if (!XtIsManaged (RxnPreV_FormDlg_Get (rpv_p)))
    {
    XtVaGetValues (SynVCB_TopApp_Get (SynViewCB_P),
      XmNx, &parent_xpos, 
      NULL);

    if (parent_xpos + AppDim_AppWidth_Get (&GAppDim) >
        Screen_Width_Get (SynAppR_ScrnAtrb_Get (&GSynAppR)))
      {
      parent_xpos = 0;
      }

    if (RxnPreV_Which_Get (rpv_p) == RPV_PREVIEW_PATH)
      {
      title = XmStringCreateLtoR (RPV_PREVIEW_TITLE_PATH, 
        XmFONTLIST_DEFAULT_TAG);
      x_coord = parent_xpos + AppDim_AppWidth_Get (&GAppDim) 
        - (RxnPreV_DAWd_Get (rpv_p) + RxnPreV_RemWd_Get (rpv_p));
      y_coord = 0;
      }

    else if (RxnPreV_Which_Get (rpv_p) == RPV_PREVIEW_TOP)
      {
      title = XmStringCreateLtoR (RPV_PREVIEW_TITLE_TOPL, 
        XmFONTLIST_DEFAULT_TAG);
      if (AppDim_AppSize_Get (&GAppDim) & SAR_APPSIZE_HALF_WD)
        y_coord = RxnPreV_DAHt_Get (rpv_p) + RxnPreV_RemHt_Get (rpv_p);
      else
        y_coord = 0;

      if (AppDim_AppSize_Get (&GAppDim) & SAR_APPSIZE_DFLT_WD)
        x_coord = parent_xpos + AppDim_AppWidth_Get (&GAppDim);
      else
      x_coord = parent_xpos + AppDim_AppWidth_Get (&GAppDim) 
        - (RxnPreV_DAWd_Get (rpv_p) + RxnPreV_RemWd_Get (rpv_p));
      }

    else if (RxnPreV_Which_Get (rpv_p) == RPV_PREVIEW_MID)
      {
      title = XmStringCreateLtoR (RPV_PREVIEW_TITLE_PRIM, 
        XmFONTLIST_DEFAULT_TAG);
      if (AppDim_AppSize_Get (&GAppDim) & SAR_APPSIZE_HALF_WD)
        y_coord = 2 * (RxnPreV_DAHt_Get (rpv_p) + RxnPreV_RemHt_Get (rpv_p));
      else
        y_coord = (RxnPreV_DAHt_Get (rpv_p) + RxnPreV_RemHt_Get (rpv_p));

      if (AppDim_AppSize_Get (&GAppDim) & SAR_APPSIZE_DFLT_WD)
        x_coord = parent_xpos + AppDim_AppWidth_Get (&GAppDim);
      else
      x_coord = parent_xpos + AppDim_AppWidth_Get (&GAppDim) 
        - (RxnPreV_DAWd_Get (rpv_p) + RxnPreV_RemWd_Get (rpv_p));
      }

    else 
      {
      title = XmStringCreateLtoR (RPV_PREVIEW_TITLE_SEC, 
        XmFONTLIST_DEFAULT_TAG);
      if (AppDim_AppSize_Get (&GAppDim) & SAR_APPSIZE_HALF_WD)
        y_coord = 3 * (RxnPreV_DAHt_Get (rpv_p) + RxnPreV_RemHt_Get (rpv_p));
      else
        y_coord = 2 * (RxnPreV_DAHt_Get (rpv_p) + RxnPreV_RemHt_Get (rpv_p));

      if (AppDim_AppSize_Get (&GAppDim) & SAR_APPSIZE_DFLT_WD)
        x_coord = parent_xpos + AppDim_AppWidth_Get (&GAppDim);
      else
        x_coord = parent_xpos + AppDim_AppWidth_Get (&GAppDim) 
          - (RxnPreV_DAWd_Get (rpv_p) + RxnPreV_RemWd_Get (rpv_p));
      }
//printf("kka1:x=%d, y=%d\n",x_coord,y_coord);
    XtVaSetValues (RxnPreV_FormDlg_Get (rpv_p),
      XmNdialogTitle, title, 
      XmNdefaultPosition, False,
      XmNx, x_coord,
      XmNy, y_coord,
      NULL);
    XmStringFree (title);

    XtManageChild (RxnPreV_FormDlg_Get (rpv_p));
    if (RxnPreV_IsNew_Get (rpv_p))
      {
      /*  Set window gravity on reaction preview drawing area.  */
      winattr.bit_gravity = ForgetGravity;
      XChangeWindowAttributes (XtDisplay (RxnPreV_FormDlg_Get (rpv_p)), 
        XtWindow (RxnPreV_MolDA_Get (rpv_p)), 
        CWBitGravity, &winattr);
      XDefineCursor (XtDisplay (RxnPreV_FormDlg_Get (rpv_p)), 
        XtWindow (RxnPreV_FormDlg_Get (rpv_p)), 
        SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_RXNPVW));
      RxnPreV_ResetPB_CB ((Widget) NULL, (XtPointer) rpv_p, (XtPointer) NULL);
      RxnPreV_IsNew_Put (rpv_p, FALSE);
      }

    /*  Check to see if we need to add event handler to drawing window.  */
    pview_p = SynVCB_PstView_Get (SynViewCB_P);
    if (RxnPreV_Which_Get (rpv_p) == RPV_PREVIEW_PATH)
      {
      if (PstView_PstCB_Get (pview_p) != NULL)
        {
        /*  Update the path previewer.  */
        vlvl_p = PstView_PathVLvl_Get (pview_p);
        tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
        if (TreeLvl_FocusNd_Get (tlvl_p) != PVW_NODE_NONE)
          {
          React_Record_t *rxn_rec_p;

          PstView_Focus_Draw  (vlvl_p);
          if (TreeLvl_FocusNd_Get (tlvl_p) == 0)
            {
            if (String_Alloc_Get (RxnPreV_RxnName_Get (rpv_p)))
              String_Destroy (RxnPreV_RxnName_Get (rpv_p));
            RxnPreV_RxnName_Put (rpv_p, String_Create (RPV_LABEL_TARGET, 
              0));
            RxnPreV_Update (rpv_p, TreeLvl_IthCmpNd_Get (tlvl_p, 0), 
              TRUE);
            }
          else
            {
            rxn_rec_p = React_Schema_Handle_Get (PstSubg_Reaction_Schema_Get (
              PstComp_Father_Get (TreeLvl_IthCmpNd_Get (tlvl_p, 
              TreeLvl_FocusNd_Get (tlvl_p)))));
            if (String_Alloc_Get (RxnPreV_RxnName_Get (rpv_p)))
              String_Destroy (RxnPreV_RxnName_Get (rpv_p));
            RxnPreV_RxnName_Put (rpv_p, String_Copy (React_TxtRec_Name_Get (
              React_Text_Get (rxn_rec_p))));

            /* Quirk of String_Copy:  doesn't null terminate string!! */
            *(String_Value_Get (RxnPreV_RxnName_Get (rpv_p))
              + String_Length_Get (RxnPreV_RxnName_Get (rpv_p))) 
              = (U8_t) '\0';
             RxnPreV_Update (rpv_p, TreeLvl_IthCmpNd_Get (tlvl_p, 
               TreeLvl_FocusNd_Get (tlvl_p)), TRUE);
            }
          }

        if (!PstView_PthEHAct_Get (pview_p))
          {
          XtAddEventHandler (DrawCxt_DA_Get (PstView_PathDC_Get (pview_p)),  
            PointerMotionMask, False, PathView_Preview_EH, (XtPointer) pview_p);
          PstView_PthEHAct_Put (pview_p, TRUE);
          }
        }
      }

    else
      {
//printf("kka2:x=%d, y=%d\n",x_coord,y_coord);
      if (PstView_PstCB_Get (pview_p) != NULL) 
        {
        /*  Update the reaction previewer.  */
        vlvl_p = PstView_IthVLvl_Get (pview_p, RxnPreV_Which_Get (rpv_p));
        tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
        if (TreeLvl_FocusNd_Get (tlvl_p) != PVW_NODE_NONE)
          {
          React_Record_t *rxn_rec_p;

          PstView_Focus_Draw  (vlvl_p);
          if (TreeLvl_LvlNum_Get (tlvl_p) == 0)
            {
            if (String_Alloc_Get (RxnPreV_RxnName_Get (rpv_p)))
              String_Destroy (RxnPreV_RxnName_Get (rpv_p));
            RxnPreV_RxnName_Put (rpv_p, String_Create (RPV_LABEL_TARGET,
              0));
            RxnPreV_Update (rpv_p, TreeLvl_IthCmpNd_Get (tlvl_p, 0), 
              TRUE);
            }
          else
            {
            rxn_rec_p = React_Schema_Handle_Get (PstSubg_Reaction_Schema_Get (
              PstComp_Father_Get (TreeLvl_IthCmpNd_Get (tlvl_p, 
              TreeLvl_FocusNd_Get (tlvl_p)))));
            if (String_Alloc_Get (RxnPreV_RxnName_Get (rpv_p)))
              String_Destroy (RxnPreV_RxnName_Get (rpv_p));
            RxnPreV_RxnName_Put (rpv_p, String_Copy (React_TxtRec_Name_Get (
              React_Text_Get (rxn_rec_p))));

            /* Quirk of String_Copy:  doesn't null terminate string!! */
            *(String_Value_Get (RxnPreV_RxnName_Get (rpv_p))
              + String_Length_Get (RxnPreV_RxnName_Get (rpv_p))) 
              = (U8_t) '\0';
            RxnPreV_Update (rpv_p, TreeLvl_IthCmpNd_Get (tlvl_p, 
              TreeLvl_FocusNd_Get (tlvl_p)), TRUE);
            }
          }
        }

      if (!PstView_PstEHAct_Get (pview_p))
        {
        XtAddEventHandler (DrawCxt_DA_Get (PstView_PstDC_Get (pview_p)),  
          PointerMotionMask, False, PstView_Preview_EH, (XtPointer) pview_p);
        PstView_PstEHAct_Put (pview_p, TRUE);
        }

      PstView_NumActPV_Get (pview_p)++;
      }
    }

  return ;
}
/*  End of SynMenu_PreView_CB  */

/****************************************************************************
*
*  Function Name:                 SynMenu_RxnCmmt_CB
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
void SynMenu_RxnCmmt_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p 
  )
{
  
  return ;
}
/*  End of SynMenu_RxnCmmt_CB  */

/****************************************************************************
*
*  Function Name:                 SynMenu_SearchCont_CB
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
void SynMenu_SearchCont_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  
  return ;
}
/*  End of SynMenu_SearchCont_CB  */

/****************************************************************************
*
*  Function Name:                 SynMenu_SelTrace_CB
*
*    This routine.
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
void SynMenu_SelTrace_CB
  (
  Widget          w, 
  XtPointer       client_p, 
  XtPointer       call_p  
  )
{
  SynViewCB_t    *svcb_p;
  PstView_t      *pv_p;
  SelTrace_t     *selt_p;
  
  svcb_p = (SynViewCB_t *) client_p;
  if (svcb_p == NULL) 
    return ;

  pv_p = SynVCB_PstView_Get (svcb_p);
  selt_p = PstView_SelTrace_Get (pv_p);
  if (!SelTrace_IsCreated_Get (selt_p))
    {
    SelTrace_Create (PstView_Form_Get (pv_p), pv_p);
    }

  else if (XtIsManaged (SelTrace_FormDlg_Get (selt_p)))
    {
    return;
    }

  if (SelTrace_Setup (pv_p))
    {
    Dimension            parent_xpos;
    Dimension            selt_width;

    XtVaGetValues (SynVCB_TopApp_Get (svcb_p),
      XmNx, &parent_xpos, 
      NULL);

    if (parent_xpos + AppDim_AppWidth_Get (&GAppDim) >
        Screen_Width_Get (SynAppR_ScrnAtrb_Get (&GSynAppR)))
      {
      parent_xpos = 0;
      }

    XtVaGetValues (SelTrace_FormDlg_Get (selt_p),
      XmNwidth, &selt_width,
      NULL);
    
    XtVaSetValues (SelTrace_FormDlg_Get (selt_p),
    XmNdefaultPosition, False,
    XmNx, parent_xpos + AppDim_AppWidth_Get (&GAppDim) - selt_width,
    XmNy, 0,
    NULL);

    XtManageChild (SelTrace_FormDlg_Get (selt_p));
    }
  return ;
}
/*  End of SynMenu_SelTrace_CB  */

/****************************************************************************
*
*  Function Name:                 SynMenu_SglShot_CB
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
void SynMenu_SglShot_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  SynViewCB_t    *svcb_p;
  SShotView_t    *ssv_p;
  PstView_t      *pv_p;
  PsvTreeLvl_t   *tlvl_p;                  /* Pst tree level */
  PsvViewLvl_t   *vlvl_p;                  /* Pst view level */
  Subgoal_t      *sg_p;
  SymTab_t       *symtab_p;
  Sling_t         temp_sling;
  U32_t           rxn_key;

  svcb_p = (SynViewCB_t *) client_p;
  if (svcb_p == NULL) 
    return ;

  ssv_p = SynVCB_SShotView_Get (svcb_p);
  pv_p = SynVCB_PstView_Get (svcb_p);
  if (ssv_p == NULL || pv_p == NULL) 
    return ;

  /*  Get selected target compound and reaction.  */
  vlvl_p = PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM);
  tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
  if (TreeLvl_SelNd_Get (tlvl_p) != PVW_NODE_NONE)
    {
    sg_p = PstComp_Father_Get (TreeLvl_IthCmpNd_Get (tlvl_p, 
      TreeLvl_SelNd_Get (tlvl_p)));
    symtab_p = PstComp_SymbolTable_Get (PstSubg_Father_Get (sg_p));
    rxn_key = PstSubg_Reaction_Schema_Get (sg_p);
    }
  else 
    {
    vlvl_p = PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID);
    tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
    if (TreeLvl_SelNd_Get (tlvl_p) != PVW_NODE_NONE)
      {
      sg_p = PstComp_Father_Get (TreeLvl_IthCmpNd_Get (tlvl_p, 
        TreeLvl_SelNd_Get (tlvl_p)));
      symtab_p = PstComp_SymbolTable_Get (PstSubg_Father_Get (sg_p));
      rxn_key = PstSubg_Reaction_Schema_Get (sg_p);
      }
    else
      {
      InfoWarn_Show (SSV_WARN_NORXN);
      return;
      }
    }

  if (!SShotV_IsCreated_Get (ssv_p))
    {
    SShotView_Create (SynVCB_TopApp_Get (svcb_p), ssv_p);
    }

  else if (XtIsManaged (SShotV_FormDlg_Get (ssv_p, 0)))
    {
    return;
    }

  /*  Get the results of applying singleshot to the given reaction
      and target compound, update and manage singleshot viewer.
  */

  temp_sling = Sling_CopyTrunc (SymTab_Sling_Get (symtab_p));
  SShotV_TgtXtr_Put (ssv_p, Sling2Xtr (temp_sling));
  Sling_Destroy (temp_sling);
  SShotV_RxnRec_Put (ssv_p, React_Schema_Handle_Get (rxn_key));

  SingleShot_Apply (SShotV_GenSGs_Get (ssv_p), SymTab_Sling_Get (symtab_p), 
    rxn_key, Rcb_Flags_StereoChemistry_Get (&GGoalRcb),
    Rcb_Flags_PreserveStructures_Get (&GGoalRcb));

  SShotView_Setup (ssv_p);
  SShotView_Update (ssv_p, 0);

  XtManageChild (SShotV_FormDlg_Get (ssv_p, 0));
  return ;
}
/*  End of SynMenu_SglShot_CB  */

/****************************************************************************
*
*  Function Name:                 SynMenu_Submit_CB
*
*    This routine creates (if needed) and manages the job submission dialog.
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
void SynMenu_Submit_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  SubmitCB_t    *submitcb_p;
  SynViewCB_t   *svcb_p;

  svcb_p = (SynViewCB_t *) client_p;
  submitcb_p = SynVCB_JobSubmit_Get (svcb_p);

  if (!Submit_Created_Get (submitcb_p))
    Submission_Create (submitcb_p, SynVCB_TopForm_Get (svcb_p), 
      SynVCB_AppCon_Get (svcb_p));
 
  Rcb_Destroy (Submit_TempRcb_Get (submitcb_p));
  Rcb_Copy (Submit_TempRcb_Get (submitcb_p), &GGoalRcb);
  Submit_Values_Set (submitcb_p, Submit_TempRcb_Get (submitcb_p));
  if (RunStats_CumCycles_Get (&GRunStats))
    JobInfo_Draw_CB (w, (XtPointer) Submit_JobInfoCB_Get (submitcb_p), 
      (XtPointer) NULL);

  XtManageChild (Submit_FormDlg_Get (submitcb_p));

  return ;
}
/*  End of SynMenu_Submit_CB  */

/****************************************************************************
*
*  Function Name:                 SynMenu_Expand_CB
*
*    This routine continues the current run with a new name for one cycle,
*    selecting the marked compound.
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
void SynMenu_Expand_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  SubmitCB_t    *submitcb_p;
  SynViewCB_t   *svcb_p;
  SymTab_t      *symtab_p;
  Sling_t        mark_sling;
  String_t       new_name, new_comment;
  char           comment_message[256], date_str[32], name_str[48], filename[128], *old_fname, *change_pos, new_fname[128];
  PstView_t     *pv_p;
  PstCB_t       *pstcb_p;
  U32_t          cmp_i;
  Boolean_t      marked;
  PsvCmpInfo_t  *cmpinfos;
  Compound_t    *cmp_p, *else_p;
  PstNode_t      node;
  Array_t        compound_list, subgoal_list;
  FILE          *f_p;
  U8_t           fcb_i;
  Rcb_t         *rcb_p;
Boolean_t already_interactive;

  svcb_p = (SynViewCB_t *) client_p;

  pv_p = SynVCB_PstView_Get (SynViewCB_P);

  pstcb_p = PstView_PstCB_Get (pv_p);
  cmpinfos = PstView_CmpInfo_Get (pv_p);
  if (pstcb_p == NULL || cmpinfos == NULL)
    {
    InfoWarn_Show ("No PST to view.");
    return ;
    }

  pstcb_p = Pst_ControlHandle_Get ();
  PstNode_Subgoal_Put (&node, PstCB_Root_Get (pstcb_p));

  Array_1d_Create (&subgoal_list, PstCB_SubgoalIndex_Get (pstcb_p) - 1,
     ADDRSIZE);
  Array_1d_Create (&compound_list, PstCB_CompoundIndex_Get (pstcb_p) - 1,
    ADDRSIZE);
  Array_Set (&subgoal_list, 0);
  Array_Set (&compound_list, 0);

  Status_Pst_Serialize (&subgoal_list, &compound_list, node);

  for (cmp_i = 0, marked = FALSE, symtab_p = NULL; cmp_i < PstView_NumCmpI_Get (pv_p); cmp_i++)
    if (PsvCmpI_Marked_Get (cmpinfos[cmp_i]))
    {
if (cmp_i==0)
{
InfoWarn_Show ("cmpinfos has a zeroth member, but it's meaningless and misleading!\n"
  "Not only that, but it duplicates the first member, which probably means that the last member is missing!\n"
  "On top of everything else, it's marked, which means I'd have to use -1 as an array index if I didn't show this warning instead!!!");
  Array_Destroy (&subgoal_list);
  Array_Destroy (&compound_list);
return;
}
    cmp_p = (Compound_t *) Array_1d32_Get (&compound_list, cmp_i - 1);
    if (marked && symtab_p != PstComp_SymbolTable_Get (cmp_p))
      {
      InfoWarn_Show ("More than one compound is marked.");
  Array_Destroy (&subgoal_list);
  Array_Destroy (&compound_list);
      return;
      }
    else if (!marked)
      {
      glob_sel_cmp = else_p = cmp_p;
      marked = TRUE;
      symtab_p = PstComp_SymbolTable_Get (cmp_p);
      }
    }

  Array_Destroy (&subgoal_list);
  Array_Destroy (&compound_list);

  if (!marked)
  {
    InfoWarn_Show ("No marked compound was found.");
    return;
  }

already_interactive = strstr (String_Value_Get (Rcb_Name_Get (&GGoalRcb)), "_interactive") != NULL;
  strcpy (filename, String_Value_Get (FileCB_FileStr_Get (Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_PATH))));
/*
  filename[strlen (filename) - 4] = '\0';
  sprintf (filename + strlen (filename), "cmp%d.path", SymTab_Index_Get (PstComp_SymbolTable_Get (else_p)));
*/
if (!already_interactive)
{
  filename[strlen (filename) - 5] = '\0';
  strcat (filename, "_interactive.path");
}
  f_p = fopen (filename, "r");
  if (f_p != NULL)
  {
    fclose (f_p);
if (!already_interactive)
{
    sprintf (comment_message, "Interactive path file %s already exists - sorry!", filename);
    InfoWarn_Show (comment_message);
    return;
}
  }

  if (PstView_SelCmp (pv_p, else_p))
  {
    f_p = fopen (filename, "w");
    cmp_p = PstSubg_Son_Get (PstCB_Root_Get (pstcb_p));
    comment_message[0] = '\0';
    find_marked (f_p, cmp_p, cmpinfos, comment_message);
    fclose (f_p);
  }

  submitcb_p = SynVCB_JobSubmit_Get (svcb_p);

/* non-graphical - shouldn't need this:
  if (!Submit_Created_Get (submitcb_p))
    Submission_Create (submitcb_p, SynVCB_TopForm_Get (svcb_p), 
      SynVCB_AppCon_Get (svcb_p));
*/
 
  Rcb_Destroy (Submit_TempRcb_Get (submitcb_p));

  Rcb_Copy (Submit_TempRcb_Get (submitcb_p), &GGoalRcb);

/*
  sprintf (name_str, "%s_selcmp%d", String_Value_Get (Rcb_Name_Get (&GGoalRcb)), SymTab_Index_Get (symtab_p));
*/

  rcb_p = Submit_TempRcb_Get (submitcb_p);

if (!already_interactive)
{
  sprintf (name_str, "%s_interactive", String_Value_Get (Rcb_Name_Get (&GGoalRcb)));
  new_name = String_Create ((const char *) name_str, 0);
  Rcb_Name_Put (rcb_p, new_name);

  while (name_str[strlen (name_str) - 1] != '_') name_str[strlen (name_str) - 1] = '\0';
  name_str[strlen (name_str) - 1] = '\0'; /* truncate to original runid for substitution which follows */

  for (fcb_i = 0; fcb_i < FCB_IND_NUMOF; fcb_i++)
    {
    old_fname = (char *) String_Value_Get (FileCB_FileStr_Get (
      Rcb_IthFileCB_Get (rcb_p, fcb_i)));
    change_pos = strstr (old_fname, name_str);
    if (change_pos != NULL)
      {
      *change_pos = '\0';
      sprintf (new_fname, "%s%s%s", old_fname, String_Value_Get (new_name), change_pos + strlen (name_str));
      String_Destroy (FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, fcb_i)));
      FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, fcb_i), String_Create (new_fname, 0));
      }
    }
}

  strcpy (date_str, ctime (&((Rcb_Date_Get (&GGoalRcb)).tv_sec)));
  sprintf (comment_message, "Expansion of compound %d; %s, %s",
    SymTab_Index_Get (symtab_p), String_Value_Get (Rcb_Name_Get (&GGoalRcb)), date_str);
  new_comment = String_Create ((const char *) comment_message, 0);
  Rcb_Comment_Put (rcb_p, new_comment);

  Status_File_Write (String_Value_Get (FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT))), rcb_p, &GRunStats);

  Rcb_Flags_Restart_Put (rcb_p, TRUE);
/*
  Rcb_Flags_Continue_Put (rcb_p, TRUE); ambiguous, misleading flag - doesn't mean "continue"!
*/
  Rcb_LeapSize_Put (rcb_p, 1);
  Rcb_MaxCycles_Put (rcb_p, Rcb_MaxCycles_Get (rcb_p) + 1);

  /*  Check to see if there is an unsaved PST in the viewer.  */
  if (PstCB_TotalExpandedCompounds_Get (pstcb_p) > 0
      && Rcb_Flags_SaveStatus_Get (&GGoalRcb)
      && RunStats_Flags_PstChanged_Get (&GRunStats)
      && !already_interactive)
    {
    SVF_QueryDg_Update (SVF_QUERY_TYPE_SAVESTAT, SVF_COMMAND_TYPE_EXEC,
      (XtPointer) submitcb_p);
    }

  else
    {
/* Don't set to FALSE - for whatever reason, it never seems to get set back to TRUE on a restart!
    RunStats_Flags_PstChanged_Put (&GRunStats, FALSE);
*/
    RunStats_Flags_PstChanged_Put (&GRunStats, TRUE);
    Submission_Execute (submitcb_p);
    }

  return ;
}
/*  End of SynMenu_Expand_CB  */


/****************************************************************************
*
*  Function Name:                 SynMenu_SubMarked_CB
*
*    This routine creates (if needed) and manages the job submission dialog
*    for the marked compound.
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
void SynMenu_SubMarked_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  SubmitCB_t    *submitcb_p;
  SynViewCB_t   *svcb_p;
  SymTab_t      *symtab_p;
  Sling_t        mark_sling;
  String_t       new_name, new_comment;
  char           comment_message[256], date_str[32], name_str[48], filename[128];
  PstView_t     *pv_p;
  PstCB_t       *pstcb_p;
/*
  U32_t          cycle_i;
  U32_t          num_cycles;
*/
  U32_t          cmp_i;
  Boolean_t      marked;
  PsvCmpInfo_t  *cmpinfos;
  Compound_t    *cmp_p, *else_p;
  PstNode_t      node;
  Array_t        compound_list, subgoal_list;
  FILE          *f_p;

  svcb_p = (SynViewCB_t *) client_p;

  pv_p = SynVCB_PstView_Get (SynViewCB_P);

  pstcb_p = PstView_PstCB_Get (pv_p);
  cmpinfos = PstView_CmpInfo_Get (pv_p);
  if (pstcb_p == NULL || cmpinfos == NULL)
    {
    InfoWarn_Show ("No PST to view.");
    return ;
    }

/* this is MUCH simpler - AND more complete - than the code that's commented out below! */

  pstcb_p = Pst_ControlHandle_Get ();
  PstNode_Subgoal_Put (&node, PstCB_Root_Get (pstcb_p));

  Array_1d_Create (&subgoal_list, PstCB_SubgoalIndex_Get (pstcb_p) - 1,
     ADDRSIZE);
  Array_1d_Create (&compound_list, PstCB_CompoundIndex_Get (pstcb_p) - 1,
    ADDRSIZE);
  Array_Set (&subgoal_list, 0);
  Array_Set (&compound_list, 0);

  Status_Pst_Serialize (&subgoal_list, &compound_list, node);

  for (cmp_i = 0, marked = FALSE, symtab_p = NULL; cmp_i < PstView_NumCmpI_Get (pv_p); cmp_i++)
    if (PsvCmpI_Marked_Get (cmpinfos[cmp_i]))
    {
if (cmp_i==0)
{
InfoWarn_Show ("cmpinfos has a zeroth member, but it's meaningless and misleading!\n"
  "Not only that, but it duplicates the first member, which probably means that the last member is missing!\n"
  "On top of everything else, it's marked, which means I'd have to use -1 as an array index if I didn't show this warning instead!!!");
  Array_Destroy (&subgoal_list);
  Array_Destroy (&compound_list);
return;
}
    cmp_p = (Compound_t *) Array_1d32_Get (&compound_list, cmp_i - 1);
    if (marked && symtab_p != PstComp_SymbolTable_Get (cmp_p))
      {
      InfoWarn_Show ("More than one compound is marked.");
  Array_Destroy (&subgoal_list);
  Array_Destroy (&compound_list);
      return;
      }
    else if (!marked)
      {
      else_p=cmp_p;
      marked = TRUE;
      symtab_p = PstComp_SymbolTable_Get (cmp_p);
      }
    }

  Array_Destroy (&subgoal_list);
  Array_Destroy (&compound_list);

/* code adapted from (all but obsolete) path writing code ignores marks on undeveloped compounds!

  num_cycles = PstCB_TotalExpandedCompounds_Get (pstcb_p);

  cmp_p = PstSubg_Son_Get (PstCB_Root_Get (pstcb_p));

  for (cycle_i = 0, marked = FALSE, symtab_p = NULL; cycle_i < num_cycles && cmp_p != NULL; cycle_i++)
    {
    else_p = SymTab_FirstComp_Get (PstComp_SymbolTable_Get (cmp_p));
    while (else_p != NULL)
      {
      if (PsvCmpI_Marked_Get (cmpinfos[PstComp_Index_Get (else_p)]))
        {
        if (marked && symtab_p != PstComp_SymbolTable_Get (else_p))
          {
          InfoWarn_Show ("More than one compound is marked.");
          return;
          }
        else if (!marked)
          {
          marked = TRUE;
          symtab_p = PstComp_SymbolTable_Get (else_p);
          }
        }
      else_p = PstComp_Prev_Get (else_p);
      }

    cmp_p = PstComp_Next_Get (cmp_p);
    }
*/

  if (!marked)
  {
/*
    InfoWarn_Show ("No marked (developed) compound was found.");
*/
    InfoWarn_Show ("No marked compound was found.");
    return;
  }

  strcpy (filename, String_Value_Get (FileCB_FileStr_Get (Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_PATH))));
  filename[strlen (filename) - 4] = '\0';
  sprintf (filename + strlen (filename), "cmp%d.path", SymTab_Index_Get (PstComp_SymbolTable_Get (else_p)));
  f_p = fopen (filename, "r");
  if (f_p != NULL)
  {
    fclose (f_p);
    sprintf (comment_message, "File %s already exists - sorry!", filename);
    InfoWarn_Show (comment_message);
    return;
  }

  if (PstView_SelCmp (pv_p, else_p))
  {
    f_p = fopen (filename, "w");
    cmp_p = PstSubg_Son_Get (PstCB_Root_Get (pstcb_p));
    comment_message[0] = '\0';
    find_marked (f_p, cmp_p, cmpinfos, comment_message);
    fclose (f_p);
  }

  submitcb_p = SynVCB_JobSubmit_Get (svcb_p);

  if (!Submit_Created_Get (submitcb_p))
    Submission_Create (submitcb_p, SynVCB_TopForm_Get (svcb_p), 
      SynVCB_AppCon_Get (svcb_p));
 
  Rcb_Destroy (Submit_TempRcb_Get (submitcb_p));
/*
  Rcb_Copy (Submit_TempRcb_Get (submitcb_p), &GGoalRcb);
*/
  Rcb_Init (Submit_TempRcb_Get (submitcb_p), FALSE);

  mark_sling = Sling_CopyTrunc (SymTab_Sling_Get (symtab_p));
  Rcb_Goal_Put (Submit_TempRcb_Get (submitcb_p), mark_sling);
/*
  String_Destroy (Rcb_Name_Get (Submit_TempRcb_Get (submitcb_p)));
  String_Destroy (Rcb_Comment_Get (Submit_TempRcb_Get (submitcb_p)));
  empty = String_Create ("", 0);
*/
  sprintf (name_str, "%s_cmp%d", String_Value_Get (Rcb_Name_Get (&GGoalRcb)), SymTab_Index_Get (symtab_p));
  new_name = String_Create ((const char *) name_str, 0);
  Rcb_Name_Put (Submit_TempRcb_Get (submitcb_p), new_name);
/*
  Rcb_Name_Put (Submit_TempRcb_Get (submitcb_p), empty);
  Rcb_Name_Put (Submit_TempRcb_Get (submitcb_p), String_Copy (empty));
  Rcb_Comment_Put (Submit_TempRcb_Get (submitcb_p), empty);
*/
  strcpy (date_str, ctime (&((Rcb_Date_Get (&GGoalRcb)).tv_sec)));
  sprintf (comment_message, "Compound %d; %s, %s",
    SymTab_Index_Get (symtab_p), String_Value_Get (Rcb_Name_Get (&GGoalRcb)), date_str);
  new_comment = String_Create ((const char *) comment_message, 0);
  Rcb_Comment_Put (Submit_TempRcb_Get (submitcb_p), new_comment);

  Submit_Values_Set (submitcb_p, Submit_TempRcb_Get (submitcb_p));
  JobInfo_Values_Set (Submit_JobInfoCB_Get (submitcb_p), Submit_TempRcb_Get (submitcb_p));
/*
  if (RunStats_CumCycles_Get (&GRunStats))
*/
    JobInfo_Draw_CB (w, (XtPointer) Submit_JobInfoCB_Get (submitcb_p), 
      (XtPointer) NULL);

  XtManageChild (Submit_FormDlg_Get (submitcb_p));

  return ;
}
/*  End of SynMenu_SubMarked_CB  */


/****************************************************************************
*
*  Function Name:                 SynMenu_SymLegend_CB
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
void SynMenu_SymLegend_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  
  return ;
}
/*  End of SynMenu_SymLegend_CB  */

/****************************************************************************
*
*  Function Name:                 SynMenu_TreeType_CB
*
*    This routine.
*
*
*  Implicit Inputs:
*
*    SSMenuCB,    the main menu bar control block
*    SynViewCB_P, the SynView control block
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
void SynMenu_TreeType_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  PsvTreeLvl_t  *pthtlvl_p;             /* Path tree level */
  PstView_t     *pv_p;
  CmpInst_t     *cinst_p;
  SelTrace_t    *selt_p;
  Compound_t    *curcmp_p;
  char          *tag;
  XmString       num_lbl;

  tag = (char *) client_p;

  pv_p = SynVCB_PstView_Get (SynViewCB_P);
  if (pv_p == NULL) 
    return;

  if (strcmp (tag, SMU_OTREE_SOLV_TAG) == 0)
    {
    if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_SOLV)
      {
      XtVaSetValues (SSMenuCB.ot_solv_tb,
        XmNset, True,
        NULL);
      return;
      }
    else
      {
      if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_DEV)
        {
        XtVaSetValues (SSMenuCB.ot_dev_tb,
          XmNset, False,
          NULL);
        PstView_TreeType_Put (pv_p, PVW_TREETYPE_SOLV);
        }

      else if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_FULL)
        {
        XtVaSetValues (SSMenuCB.ot_full_tb,
          XmNset, False,
          NULL);
        PstView_TreeType_Put (pv_p, PVW_TREETYPE_SOLV);
        }
      else
        return;
      }
    }

  else if (strcmp (tag, SMU_OTREE_DEV_TAG) == 0)
    {
    if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_DEV)
      {
      XtVaSetValues (SSMenuCB.ot_dev_tb,
        XmNset, True,
        NULL);
      return;
      }
    else
      {
      if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_SOLV)
        {
        XtVaSetValues (SSMenuCB.ot_solv_tb,
          XmNset, False,
          NULL);
        PstView_TreeType_Put (pv_p, PVW_TREETYPE_DEV);
        }

      else if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_FULL)
        {
        XtVaSetValues (SSMenuCB.ot_full_tb,
          XmNset, False,
          NULL);
        PstView_TreeType_Put (pv_p, PVW_TREETYPE_DEV);
        }
      else
        return;
      }
    }

  else if (strcmp (tag, SMU_OTREE_FULL_TAG) == 0)
    {
    if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_FULL)
      {
      XtVaSetValues (SSMenuCB.ot_full_tb,
        XmNset, True,
        NULL);
      return;
      }
    else
      {
      if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_DEV)
        {
        XtVaSetValues (SSMenuCB.ot_dev_tb,
          XmNset, False,
          NULL);
        PstView_TreeType_Put (pv_p, PVW_TREETYPE_FULL);
        }

      else if (PstView_TreeType_Get (pv_p) == PVW_TREETYPE_SOLV)
        {
        XtVaSetValues (SSMenuCB.ot_solv_tb,
          XmNset, False,
          NULL);
        PstView_TreeType_Put (pv_p, PVW_TREETYPE_FULL);
        }
      else
        return;
      }
    }
  else
    return;

  if (PstView_PstCB_Get (pv_p) == NULL)
    return ;

  /*  Reset top tree level and move view levels to top.  */
  PstView_Mouse_Remove (pv_p);

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

  selt_p = PstView_SelTrace_Get (pv_p);
  if (SelTrace_IsCreated_Get (selt_p) 
      && XtIsManaged (SelTrace_FormDlg_Get (selt_p))) 
    {
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
    }

  cinst_p = PstView_CmpInsts_Get (pv_p);
  if (CmpInst_IsCreated_Get (cinst_p) 
      && XtIsManaged (CmpInst_FormDlg_Get (cinst_p)))
    {
    PstView_LastSelCmp_Put (pv_p, CmpInst_CurrCmp_Get (cinst_p));
    CmpInst_Setup (pv_p);
    }

  PstView_Display (pv_p);

  return;
}
/*  End of SynMenu_TreeType_CB  */


/*  End of SYN_MENU.C  */
