#ifndef _H_SYN_MENU_
#define _H_SYN_MENU_  1
/******************************************************************************
*
*  Copyright (C) 1995 Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     SYN_MENU.H
*
*    This header file defines the information needed for the main
*    menu bar of the SYNCHEM expert system.  Button labels are defined
*    for each button in the menu.  The cascade buttons in the menu bar 
*    have mnemonics, but only a few of the submenu buttons do.  Several 
*    of the more commonly accessed buttons also have accelerators:
*    the Crtl key is used for actions and the Alt key is used for 
*    choosing between alternative options.
*
*    Layout of Menu Bar and pulldown menus:
*      
*    File   Path  Option  Search  KBase    Nav     Info      HELP
*     Open   Read  Tree    Sbmt    EDIT     PthPV   CmpInfo   INTRO
*     Save   Write  Full   CONT    SEARCH   TopPV   SgInfo    MANUAL
*     SavDev VIEW   Dev            Sglshot  MidPV   RunInfo   CONTEXT
*     Print         Solv           Mltshot  BtmPV   RxnCmmt
*     Print        Legend                   CmpInst 
*     Exit         CycleNum                 SelTrce 
*                  ClearVts                       
*                  ClearMks   
*                               
*                               
*
*    NOTE:  The callback funtions for those buttons whose labels above are
*           shown in all capital letters have NOT yet been fully implemented.
*           These callbacks are stubs that simply return when evoked. 
*                 
* 
*  Creation Date:
*
*    10-Jun-1995
*
*  Authors:
*
*    Daren Krebsbach
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
*
******************************************************************************/

#ifndef _H_SYN_VIEW_
#include "syn_view.h"
#endif

/*** Literal Values ***/

/*  Menu Labels (in order of appearance in menu bar)  */
#define SMU_FILE_CPB_LBL        "File"
#define SMU_F_OPEN_PB_LBL        "Open ..."
#define SMU_F_SAVE_PB_LBL        "Save ..."
#define SMU_F_SAVECVR_PB_LBL     "Save cover ..."
#define SMU_F_PRINT_WIN_PB_LBL   "Print window"
#define SMU_F_PRINT_RXN_PB_LBL   "Print reaction"
#define SMU_F_EXIT_PB_LBL        "Exit"
#define SMU_PATH_CPB_LBL        "Path"
#define SMU_P_READ_PB_LBL        "Read ..."
#define SMU_P_WRITE_PB_LBL       "Write ..."
#define SMU_P_VIEW_PB_LBL        "View ..."
#define SMU_OPTION_CPB_LBL      "Options"
#define SMU_O_TREE_CPB_LBL       "Tree type"
#define SMU_OT_FULL_TB_LBL        "Full"
#define SMU_OT_DEV_TB_LBL         "Developed"
#define SMU_OT_SOLV_TB_LBL        "Solved"
#define SMU_O_LEGEND_PB_LBL      "Symbol legend"
#define SMU_O_CYCLENUMS_PB_LBL   "Cycle numbers "
#define SMU_O_CLEARVST_PB_LBL    "Clear visits"
#define SMU_O_CLEARMRK_PB_LBL    "Clear marks"
#define SMU_SEARCH_CPB_LBL      "Search"
#define SMU_S_SUBMIT_PB_LBL      "Submit ..."
#define SMU_S_CONTINUE_CPB_LBL   "Continue search ..."
#define SMU_KBASE_CPB_LBL       "Kbase"
/*
#define SMU_K_EDIT_PB_LBL        "Edit ..."
#define SMU_K_SEARCH_PB_LBL      "Search ..."
#define SMU_K_SGLSHOT_PB_LBL     "Singleshot ..."
#define SMU_K_MULTSHOT_PB_LBL    "Multishot ..."
*/
#define SMU_K_REDIT_PB_LBL       "Edit RxnLib (current) ..."
#define SMU_K_SEARCH_PB_LBL      "View RxnLib (persistent) ..."
#define SMU_K_SGLSHOT_PB_LBL     "Singleshot ..."
#define SMU_K_MULTSHOT_PB_LBL    "Multishot ..."
#define SMU_K_AEDIT_PB_LBL       "Edit AvlCmp ..."

#define SMU_NAVGT_CPB_LBL       "Navigate"
#define SMU_N_BTMTPV_PB_LBL      "Bottom preview"
#define SMU_N_MIDTPV_PB_LBL      "Middle preview"
#define SMU_N_PTHTPV_PB_LBL      "Path preview"
#define SMU_N_TOPTPV_PB_LBL      "Top preview"
#define SMU_N_CMPINST_PB_LBL     "Compound instances ..."
#define SMU_N_SELTRACE_PB_LBL    "Selection trace ..."
#define SMU_INFO_CPB_LBL       "Info"
#define SMU_I_RUNINFO_PB_LBL     "Search info"
#define SMU_I_CMPINFO_PB_LBL     "Compound info"
#define SMU_I_SGINFO_PB_LBL      "Subgoal info"
#define SMU_I_RXNINFO_PB_LBL     "Reaction info"
#define SMU_I_COMMENT_PB_LBL     "Add reaction comment ..."
#define SMU_HELP_CPB_LBL        "Help"
#define SMU_H_INTRO_PB_LBL       "Intro ..."
#define SMU_H_MANUAL_PB_LBL      "Manual ..."
#define SMU_H_CTXT_PB_LBL        "Context ..."

/*  Main Menu Mnemonics (in order of appearance in menu bar) */
#define SMU_FILE_MNEM          'F'
#define SMU_PATH_MNEM          'P'
#define SMU_OPT_MNEM           'O'
#define SMU_SRCH_MNEM          'S'
#define SMU_KBASE_MNEM         'K'
#define SMU_NAV_MNEM           'N'
#define SMU_INFO_MNEM          'I'
#define SMU_HELP_MNEM          'H'

/*  Sub Menu Mnemonics  */
#define SMU_CONT_MNEM          'C'
#define SMU_TREET_MNEM         'T'

/*  Button Accelerator labels */
/*  Ctrl+ for actions  ---  Alt+ for setting options */
#define SMU_EXIT_ACLBL          "Ctrl+X"   /* File-Exit */
#define SMU_LEGEND_ACLBL        "Ctrl+L"   /* Option-Legend */
#define SMU_TREE_DEV_ACLBL      "Alt+D"    /* Option-TreeType-Developed */
#define SMU_TREE_FULL_ACLBL     "Alt+F"    /* Option-TreeType-Full */
#define SMU_TREE_SOLV_ACLBL     "Alt+S"    /* Option-TreeType-Solved */
#define SMU_PVBTM_ACLBL         "Ctrl+B"   /* Reaction-Bottom Preview */
#define SMU_PVMID_ACLBL         "Ctrl+M"   /* Reaction-Middle Preview */
#define SMU_PVPTH_ACLBL         "Ctrl+P"   /* Reaction-Path Preview */
#define SMU_PVTOP_ACLBL         "Ctrl+T"   /* Reaction-Top Preview */

/*  Button Accelerators  */

#define SMU_EXIT_ACCEL          "Ctrl<Key>X"
#define SMU_LEGEND_ACCEL        "Ctrl<Key>L"
#define SMU_PVBTM_ACCEL         "Ctrl<Key>B"
#define SMU_PVMID_ACCEL         "Ctrl<Key>M"
#define SMU_PVPTH_ACCEL         "Ctrl<Key>P"
#define SMU_PVTOP_ACCEL         "Ctrl<Key>T"
#define SMU_TREE_FULL_ACCEL     "Alt<Key>F"
#define SMU_TREE_DEV_ACCEL      "Alt<Key>D"
#define SMU_TREE_SOLV_ACCEL     "Alt<Key>S"

/*  Menu CallBack Tags  */
#define SMU_FILE_OPEN_TAG       "O"
#define SMU_FILE_PRINT_WIN_TAG  "W"
#define SMU_FILE_PRINT_RXN_TAG  "R"
#define SMU_FILE_SAVE_TAG       "S"
#define SMU_FILE_SAVECVR_TAG    "C"

#define SMU_HELP_CONTEXT_TAG    "C"
#define SMU_HELP_INTRO_TAG      "I"
#define SMU_HELP_MANUAL_TAG     "M"

#define SMU_PATH_READ_TAG       "R"
#define SMU_PATH_VIEW_TAG       "V"
#define SMU_PATH_WRITE_TAG      "W"

#define SMU_OTREE_DEV_TAG       "D"
#define SMU_OTREE_FULL_TAG      "F"
#define SMU_OTREE_SOLV_TAG      "S"

#define SMU_INFO_CMP_TAG        "C"
#define SMU_INFO_SG_TAG         "S"
#define SMU_INFO_RUN_TAG        "J"
#define SMU_INFO_RXN_TAG        "R"

/*** Data Structures ***/

typedef struct syn_menu_cb_s
  {
  Widget         menu_bar;              /* main menu bar */ 
  Widget         file_cpb;              /* File cascade button */ 
  Widget         file_pdm;              /* File pulldown menu */ 
  Widget         f_open_pb;             /* File open push button */ 
  Widget         f_save_pb;             /* File save push button */ 
  Widget         f_savecvr_pb;          /* File save developed push button */ 
  Widget         f_printwin_pb;         /* File print push button */ 
  Widget         f_printrxn_pb;         /* File print push button */ 
  Widget         f_exit_pb;             /* File exit menu button */ 
  Widget         path_cpb;              /* Path cascade menu button */ 
  Widget         path_pdm;              /* Path pulldown menu */ 
  Widget         p_read_pb;             /* Path read push button */ 
  Widget         p_write_pb;            /* Path write push button */ 
  Widget         p_view_pb;             /* Path view push button */ 
  Widget         option_cpb;            /* Options cascade menu button */ 
  Widget         option_pdm;            /* Options pulldown menu */ 
  Widget         o_tree_cpb;            /* Options tree type cascade button */ 
  Widget         o_tree_pdm;            /* Options tree type pulldown menu */ 
  Widget         ot_full_tb;            /* Options tree full toggle button */ 
  Widget         ot_dev_tb;             /* Options tree dev toggle button */ 
  Widget         ot_solv_tb;            /* Options tree solv toggle button */ 
  Widget         o_legend_pb;           /* Options symbol legend push button */ 
  Widget         o_cycles_pb;           /* Options unmark last push button */ 
  Widget         o_clearvst_pb;         /* Options clear visit push button */ 
  Widget         o_clearmrk_pb;         /* Options mark compound push button */ 
  Widget         search_cpb;            /* Search cascade menu button */ 
  Widget         search_pdm;            /* Search pulldown menu */ 
  Widget         s_submit_pb;           /* Search submit new push button */ 
  Widget         s_continue_pb;         /* Search continue push button */ 
  Widget         kbase_cpb;             /* Kbase cascade menu button */ 
  Widget         kbase_pdm;             /* Kbase pulldown menu */ 
  Widget         k_edit_pb;             /* Kbase edit push button */ 
  Widget         k_search_pb;           /* Kbase search push button */ 
  Widget         k_sglshot_pb;          /* Kbase single shot push button */ 
  Widget         k_mltshot_pb;          /* Kbase muliple shot push button */ 
  Widget         ka_edit_pb;            /* Kbase (avlcmp) edit push button */ 
  Widget         nav_cpb;               /* Navigate cascade menu button */ 
  Widget         nav_pdm;               /* Navigate pulldown menu */ 
  Widget         n_pthtpv_pb;           /* Navigate path preview push button */ 
  Widget         n_toptpv_pb;           /* Navigate top preview push button */ 
  Widget         n_midtpv_pb;           /* Navigate mid preview push button */ 
  Widget         n_btmtpv_pb;           /* Navigate btm preview push button */ 
  Widget         n_cmpinst_pb;          /* Nav comp instances push button */ 
  Widget         n_seltrace_pb;         /* Select Trace push button */ 
  Widget         info_cpb;              /* Info cascade menu button */ 
  Widget         info_pdm;              /* Info pulldown menu */ 
  Widget         i_runinfo_pb;          /* Info run info push button */ 
  Widget         i_cmpinfo_pb;          /* Info compound info push button */ 
  Widget         i_sginfo_pb;           /* Info subgoal info push button */ 
  Widget         i_rxninfo_pb;          /* Info rxn info push button */ 
  Widget         i_rxncmmt_pb;          /* Info add comment push button */ 
  Widget         help_cpb;              /* Help menu button */ 
  Widget         help_pdm;              /* Help pulldown menu */ 
  Widget         h_intro_pb;            /* Help intro push button */ 
  Widget         h_man_pb;              /* Help manual push button */ 
  Widget         h_context_pb;          /* Help context push button */ 
  }
  SynMenuCB_t;

/** Field Access Macros for SynMenu_t **/

/* Macro Prototypes

*/

/*** Routine Prototypes ***/

Widget SynMenu_Create         (Widget, SynViewCB_t *, char *);
void   SynMenu_Destroy        (void);

/* Callback Routine Prototypes */

void SynMenu_ClearMark_CB   (Widget, XtPointer, XtPointer);
void SynMenu_ClearVisit_CB  (Widget, XtPointer, XtPointer);
void SynMenu_CmpInst_CB     (Widget, XtPointer, XtPointer);
void SynMenu_Exit_CB        (Widget, XtPointer, XtPointer);
void SynMenu_File_CB        (Widget, XtPointer, XtPointer);
void SynMenu_Help_CB        (Widget, XtPointer, XtPointer);
void SynMenu_Info_CB        (Widget, XtPointer, XtPointer);
void SynMenu_KBaseEdit_CB   (Widget, XtPointer, XtPointer);
void SynMenu_KBaseSearch_CB (Widget, XtPointer, XtPointer);
void SynMenu_MultiShot_CB   (Widget, XtPointer, XtPointer);
void SynMenu_AvCmpEdit_CB   (Widget, XtPointer, XtPointer);
void SynMenu_Path_CB        (Widget, XtPointer, XtPointer);
void SynMenu_PreView_CB     (Widget, XtPointer, XtPointer);
void SynMenu_RxnCmmt_CB     (Widget, XtPointer, XtPointer);
void SynMenu_SearchCont_CB  (Widget, XtPointer, XtPointer);
void SynMenu_SelTrace_CB    (Widget, XtPointer, XtPointer);
void SynMenu_SglShot_CB     (Widget, XtPointer, XtPointer);
void SynMenu_Submit_CB      (Widget, XtPointer, XtPointer);
void SynMenu_SymLegend_CB   (Widget, XtPointer, XtPointer);
void SynMenu_TreeType_CB    (Widget, XtPointer, XtPointer);

#endif
/*  End of SYN_MENU.H  */
