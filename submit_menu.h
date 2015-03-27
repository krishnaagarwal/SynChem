#ifndef _H_SUBMIT_MENU_  
#define _H_SUBMIT_MENU_  1  
/****************************************************************************
*  
* Copyright (C) 1997 Synchem Group at SUNY-Stony Brook, Daren Krebsbach  
*  
*  
*  Module Name:               SUBMIT_MENU.H  
*  
*    This header file defines the information needed for the main menu of  
*    the SUBMIT module.  
*      
*    The following represents the layout of the menu:  
*  
*      File                  Options               Help  
*        Open-submit...        Directories...        Help...  
*        Open-status...        Files...  
*        Save                  Strategies...  
*        
*  
*  Creation Date:  
*  
*     25-Aug-1997  
*  
*  Authors: 
*      Daren Krebsbach 
*        based (very loosely) on version by Nader Abdelsadek
*  
*****************************************************************************/  
  
#ifndef _H_RCB_
#include "rcb.h"
#endif
 
#ifndef _H_SUBMIT_
#include "submit.h"
#endif

/*** Literal Values ***/  
  
/*  Menu Labels (in order of appearance in menu bar)  */  
  
#define SMU_FILE_CPB_LBL          "File"  
#define SMU_F_OPEN_SBMT_PB_LBL    "Open - submit ..."  
#define SMU_F_OPEN_STAT_PB_LBL    "Open - status ..."  
#define SMU_F_SAVE_PB_LBL         "Save"  
#define SMU_OPTIONS_CPB_LBL       "Options"  
#define SMU_O_DIRECTORIES_PB_LBL  "Directories ..."  
#define SMU_O_FILES_PB_LBL        "Files ..."  
#define SMU_O_STRATEGIES_LBL      "Strategies ..."  
#define SMU_HELP_CPB_LBL          "Help"  
#define SMU_HELP_PB_LBL           "Help ..."  
   
#define SMU_OPTIONS_DIR_TAG       "D"  
#define SMU_OPTIONS_FILE_TAG      "F"  
#define SMU_OPTIONS_STRAT_TAG     "S"  
  
/* Directories Dialog Labels. */ 
 
#define SMU_DIRBUF_MAXLEN   127
#define SMU_DIRTXT_MAXLEN   64

#define SMU_DIRS_DIALOG_TITLE     "Directories"

#define SMU_AVAIL_DIR_LBL         "Available Compounds:" 
#define SMU_FUNGP_DIR_LBL         "Functional Groups:" 
#define SMU_REACT_DIR_LBL         "Reactions Libraries:" 
#define SMU_SUBMIT_DIR_LBL        "Job Submission Files:" 
#define SMU_TEMP_DIR_LBL          "Target Templates:" 
#define SMU_STATUS_DIR_LBL        "Run Status Files:" 
#define SMU_PATH_DIR_LBL          "Synthesis Paths:"  
#define SMU_TRACE_DIR_LBL         "Run Trace Files:" 
 
 
#define SMU_LVLBUF_MAXLEN   3
#define SMU_LVLTXT_MAXLEN   3

#define SMU_FILES_DIALOG_TITLE      "Files"
#define SMU_SAVE_STAUS_LBL          "Save Status?" 
#define SMU_SAVE_TRACE_LBL          "Save Trace?" 
#define SMU_TRACE_LEVEL_LBL         "Trace Level:" 
 
#define SMU_STRATS_DIALOG_TITLE     "Strategies"
#define SMU_NO_STEREO_CHEM_LBL      "No Stereochemistry?"  
#define SMU_PRESERVE_STRUCT_LBL     "Preserve substructures?"   
#define SMU_STRATEGIC_BONDS_LBL     "Strategic bonds?" 
        
#define SMU_CANCELPB_LBL            "cancel" 
#define SMU_OKAYPB_LBL              "okay" 
#define SMU_RESETPB_LBL             "reset" 

/*** Data Structures ***/  
  

typedef struct directories_cb_s 
  { 
  Widget    messdg; 
  Widget    form; 
  Widget    avail_lbl; 
  Widget    avail_txt; 
  Widget    fungp_lbl; 
  Widget    fungp_txt; 
  Widget    path_lbl; 
  Widget    path_txt; 
  Widget    react_lbl; 
  Widget    react_txt; 
  Widget    status_lbl; 
  Widget    status_txt; 
  Widget    submit_lbl; 
  Widget    submit_txt; 
  Widget    temp_lbl; 
  Widget    temp_txt; 
  Widget    trace_lbl; 
  Widget    trace_txt;
  char      avail_buf[SMU_DIRBUF_MAXLEN + 1];
  char      fg_buf[SMU_DIRBUF_MAXLEN + 1];
  char      path_buf[SMU_DIRBUF_MAXLEN + 1];
  char      rxn_buf[SMU_DIRBUF_MAXLEN + 1];
  char      stat_buf[SMU_DIRBUF_MAXLEN + 1];
  char      submit_buf[SMU_DIRBUF_MAXLEN + 1];
  char      tmpl_buf[SMU_DIRBUF_MAXLEN + 1];
  char      trace_buf[SMU_DIRBUF_MAXLEN + 1];
  }  DirCB_t; 

/*  Macro Prototypes for DirCB_t

char  *DirCB_AvailBuf_Get  (DirCB_t *); 
Widget DirCB_AvailLbl_Get  (DirCB_t *); 
void   DirCB_AvailLbl_Put  (DirCB_t *, Widget);
Widget DirCB_AvailTxt_Get  (DirCB_t *);
void   DirCB_AvailTxt_Put  (DirCB_t *, Widget);
Widget DirCB_Form_Get      (DirCB_t *);
void   DirCB_Form_Put      (DirCB_t *, Widget);
char  *DirCB_FunGpBuf_Get  (DirCB_t *); 
Widget DirCB_FunGpLbl_Get  (DirCB_t *); 
void   DirCB_FunGpLbl_Put  (DirCB_t *, Widget); 
Widget DirCB_FunGpTxt_Get  (DirCB_t *); 
void   DirCB_FunGpTxt_Put  (DirCB_t *, Widget);
Widget DirCB_MsgDg_Get     (DirCB_t *);
void   DirCB_MsgDg_Put     (DirCB_t *, Widget);
char  *DirCB_PathBuf_Get   (DirCB_t *);
Widget DirCB_PathLbl_Get   (DirCB_t *);
void   DirCB_PathLbl_Put   (DirCB_t *, Widget);
Widget DirCB_PathTxt_Get   (DirCB_t *);
void   DirCB_PathTxt_Put   (DirCB_t *, Widget);
char  *DirCB_RxnBuf_Get    (DirCB_t *); 
Widget DirCB_RxnLbl_Get    (DirCB_t *); 
void   DirCB_RxnLbl_Put    (DirCB_t *, Widget); 
Widget DirCB_RxnTxt_Get    (DirCB_t *); 
void   DirCB_RxnTxt_Put    (DirCB_t *, Widget); 
char  *DirCB_StatusBuf_Get (DirCB_t *);
Widget DirCB_StatusLbl_Get (DirCB_t *);
void   DirCB_StatusLbl_Put (DirCB_t *, Widget);
Widget DirCB_StatusTxt_Get (DirCB_t *);
void   DirCB_StatusTxt_Put (DirCB_t *, Widget);
char  *DirCB_SubmitBuf_Get (DirCB_t *);
Widget DirCB_SubmitLbl_Get (DirCB_t *);
void   DirCB_SubmitLbl_Put (DirCB_t *, Widget);
Widget DirCB_SubmitTxt_Get (DirCB_t *);
void   DirCB_SubmitTxt_Put (DirCB_t *, Widget);
char  *DirCB_TempBuf_Get   (DirCB_t *); 
Widget DirCB_TempLbl_Get   (DirCB_t *); 
void   DirCB_TempLbl_Put   (DirCB_t *, Widget); 
Widget DirCB_TempTxt_Get   (DirCB_t *); 
void   DirCB_TempTxt_Put   (DirCB_t *, Widget);
char  *DirCB_TraceBuf_Get  (DirCB_t *);  
Widget DirCB_TraceLbl_Get  (DirCB_t *);  
void   DirCB_TraceLbl_Put  (DirCB_t *, Widget); 
Widget DirCB_TraceTxt_Get  (DirCB_t *); 
void   DirCB_TraceTxt_Put  (DirCB_t *, Widget); 
*/

#define DirCB_AvailBuf_Get(dcb)\
   (dcb)->avail_buf
#define DirCB_AvailLbl_Get(dcb)\
   (dcb)->avail_lbl
#define DirCB_AvailLbl_Put(dcb, value)\
   (dcb)->avail_lbl = (value)
#define DirCB_AvailTxt_Get(dcb)\
   (dcb)->avail_txt 
#define DirCB_AvailTxt_Put(dcb, value)\
   (dcb)->avail_txt = (value)
#define DirCB_Form_Get(dcb)\
   (dcb)->form
#define DirCB_Form_Put(dcb, value)\
   (dcb)->form = (value)
#define DirCB_FunGpBuf_Get(dcb)\
   (dcb)->fg_buf
#define DirCB_FunGpLbl_Get(dcb)\
   (dcb)->fungp_lbl
#define DirCB_FunGpLbl_Put(dcb, value)\
   (dcb)->fungp_lbl = (value)
#define DirCB_FunGpTxt_Get(dcb)\
   (dcb)->fungp_txt 
#define DirCB_FunGpTxt_Put(dcb, value)\
   (dcb)->fungp_txt = (value)
#define DirCB_MsgDg_Get(dcb)\
   (dcb)->messdg
#define DirCB_MsgDg_Put(dcb, value)\
   (dcb)->messdg = (value)
#define DirCB_PathBuf_Get(dcb)\
   (dcb)->path_buf
#define DirCB_PathLbl_Get(dcb)\
   (dcb)->path_lbl
#define DirCB_PathLbl_Put(dcb, value)\
   (dcb)->path_lbl = (value)
#define DirCB_PathTxt_Get(dcb)\
   (dcb)->path_txt    
#define DirCB_PathTxt_Put(dcb, value)\
   (dcb)->path_txt = (value)
#define DirCB_RxnBuf_Get(dcb)\
   (dcb)->rxn_buf
#define DirCB_RxnLbl_Get(dcb)\
   (dcb)->react_lbl
#define DirCB_RxnLbl_Put(dcb, value)\
   (dcb)->react_lbl = (value)
#define DirCB_RxnTxt_Get(dcb)\
   (dcb)->react_txt 
#define DirCB_RxnTxt_Put(dcb, value)\
   (dcb)->react_txt = (value)
#define DirCB_StatusBuf_Get(dcb)\
   (dcb)->stat_buf
#define DirCB_StatusLbl_Get(dcb)\
   (dcb)->status_lbl 
#define DirCB_StatusLbl_Put(dcb, value)\
   (dcb)->status_lbl = (value)
#define DirCB_StatusTxt_Get(dcb)\
   (dcb)->status_txt
#define DirCB_StatusTxt_Put(dcb, value)\
   (dcb)->status_txt = (value)
#define DirCB_SubmitBuf_Get(dcb)\
   (dcb)->submit_buf
#define DirCB_SubmitLbl_Get(dcb)\
   (dcb)->submit_lbl 
#define DirCB_SubmitLbl_Put(dcb, value)\
   (dcb)->submit_lbl = (value)
#define DirCB_SubmitTxt_Get(dcb)\
   (dcb)->submit_txt
#define DirCB_SubmitTxt_Put(dcb, value)\
   (dcb)->submit_txt = (value)
#define DirCB_TempBuf_Get(dcb)\
   (dcb)->tmpl_buf
#define DirCB_TempLbl_Get(dcb)\
   (dcb)->temp_lbl
#define DirCB_TempLbl_Put(dcb, value)\
   (dcb)->temp_lbl = (value)
#define DirCB_TempTxt_Get(dcb)\
   (dcb)->temp_txt 
#define DirCB_TempTxt_Put(dcb, value)\
   (dcb)->temp_txt = (value)
#define DirCB_TraceBuf_Get(dcb)\
   (dcb)->trace_buf
#define DirCB_TraceLbl_Get(dcb)\
   (dcb)->trace_lbl
#define DirCB_TraceLbl_Put(dcb, value)\
   (dcb)->trace_lbl = (value)
#define DirCB_TraceTxt_Get(dcb)\
   (dcb)->trace_txt
#define DirCB_TraceTxt_Put(dcb, value)\
   (dcb)->trace_txt = (value)


/*** FilesCB_t Strcuture Definition ****/

typedef struct files_cb_s 
  { 
  Widget messdg; 
  Widget form; 
  Widget tgb_save_status; 
  Widget tgb_save_trace; 
  Widget lbl_trace_level; 
  Widget txt_trace_level;
  char   trace_level[SMU_LVLBUF_MAXLEN + 1];
  }  FilesCB_t; 

/*  Macro Prototypes for FilesCB_t

Widget FilesCB_Form_Get         (FilesCB_t *);
void   FilesCB_Form_Put         (FilesCB_t *, Widget);
Widget FilesCB_MsgDg_Get        (FilesCB_t *);
void   FilesCB_MsgDg_Put        (FilesCB_t *, Widget);
Widget FilesCB_SaveStatus_Get   (FilesCB_t *);
void   FilesCB_SaveStatus_Put   (FilesCB_t *, Widget);
Widget FilesCB_SaveTrace_Get    (FilesCB_t *);
void   FilesCB_SaveTrace_Put    (FilesCB_t *, Widget);
char  *FilesCB_TraceLvlBuf_Get  (FilesCB_t *);
Widget FilesCB_TraceLvlLbl_Get  (FilesCB_t *);
void   FilesCB_TraceLvlLbl_Put  (FilesCB_t *, Widget);
Widget FilesCB_TraceLvlTxt_Get  (FilesCB_t *);
void   FilesCB_TraceLvlTxt_Put  (FilesCB_t *, Widget);
*/

#define FilesCB_Form_Get(fcb)\
  (fcb)->form
#define FilesCB_Form_Put(fcb, value)\
  (fcb)->form = (value)
#define FilesCB_MsgDg_Get(fcb)\
  (fcb)->messdg
#define FilesCB_MsgDg_Put(fcb, value)\
  (fcb)->messdg = (value)
#define  FilesCB_SaveStatus_Get(fcb)\
  (fcb)->tgb_save_status
#define  FilesCB_SaveStatus_Put(fcb, value)\
  (fcb)->tgb_save_status = (value)
#define  FilesCB_SaveTrace_Get(fcb)\
  (fcb)->tgb_save_trace
#define  FilesCB_SaveTrace_Put(fcb, value)\
  (fcb)->tgb_save_trace = (value)
#define  FilesCB_TraceLvlBuf_Get(fcb)\
  (fcb)->trace_level
#define  FilesCB_TraceLvlLbl_Get(fcb)\
  (fcb)->lbl_trace_level
#define  FilesCB_TraceLvlLbl_Put(fcb, value)\
  (fcb)->lbl_trace_level = (value)
#define  FilesCB_TraceLvlTxt_Get(fcb)\
  (fcb)->txt_trace_level
#define  FilesCB_TraceLvlTxt_Put(fcb, value)\
  (fcb)->txt_trace_level = (value)


/*** StratCB_t Strcuture Definition ****/

typedef struct strategies_cb_s 
  { 
  Widget strat_form; 
  Widget strat_rowcol; 
  Widget tgb_no_stereo_chem; 
  Widget tgb_preserve_struct; 
  Widget tgb_strategic_bonds;
  }  StratCB_t; 

/*  Macro Prototypes for StratCB_t

Widget   StratCB_FormDg_Get         (StratCB_t *);
void     StratCB_FormDg_Put         (StratCB_t *, Widget);
Widget   StratCB_NoStereoChem_Get   (StratCB_t *);
void     StratCB_NoStereoChem_Put   (StratCB_t *, Widget);
Widget   StratCB_PreserveStruct_Get (StratCB_t *);
void     StratCB_PreserveStruct_Put (StratCB_t *, Widget);
Widget   StratCB_RowCol_Get         (StratCB_t *);
void     StratCB_RowCol_Put         (StratCB_t, Widget);
Widget   StratCB_StratBonds_Get     (StratCB_t *);
void     StratCB_StratBonds_Put     (StratCB_t *, Widget);
*/
 
#define   StratCB_FormDg_Get(scb)\
   (scb)->strat_form
#define   StratCB_FormDg_Put(scb, value)\
   (scb)->strat_form = (value)
#define   StratCB_RowCol_Get(scb)\
   (scb)->strat_rowcol
#define   StratCB_RowCol_Put(scb, value)\
   (scb)->strat_rowcol = (value)
#define   StratCB_NoStereoChem_Get(scb)\
   (scb)->tgb_no_stereo_chem
#define   StratCB_NoStereoChem_Put(scb, value)\
   (scb)->tgb_no_stereo_chem = (value)
#define   StratCB_PreserveStruct_Get(scb)\
   (scb)->tgb_preserve_struct
#define   StratCB_PreserveStruct_Put(scb, value)\
   (scb)->tgb_preserve_struct = (value)
#define   StratCB_StratBonds_Get(scb)\
   (scb)->tgb_strategic_bonds
#define   StratCB_StratBonds_Put(scb, value)\
   (scb)->tgb_strategic_bonds = (value)


/*** SubmitMenuCB_t Strcuture Definition ****/

typedef struct submit_menu_cb_s  
  {  
  DirCB_t    dir_cb; 
  FilesCB_t  files_cb; 
  StratCB_t  strat_cb; 
  Widget     menu_bar;              /* main menu bar */  
  Widget     file_cpb;              /* File cascade button */  
  Widget     file_pdm;              /* File pulldown menu */  
  Widget     f_save_pb;             /* File save push button*/  
  Widget     f_sbmt_pb;             /* File open-submit push button*/  
  Widget     f_stat_pb;             /* File open-stat push button*/  
  Widget     options_cpb;           /* Options cascade button */  
  Widget     options_pdm;           /* Options pulldown menu */  
  Widget     o_dir_pb;              /* Options Directories push button*/  
  Widget     o_files_pb;            /* Options files push button */  
  Widget     o_strategies_pb;       /* Options strategies push button */      
  Widget     help_cpb;              /* Help cascade button */   
  Widget     help_pdm;              /* Help pulldown menu */   
  Widget     h_help_pb;             /* Help Push Button */  
  }  SubmitMenuCB_t;  


/* SubmitMenuCB_t Prototype Declarations

DirCB_t      *SubmitMenu_DirCB_Get    (SubmitMenuCB_t *); 
Widget        SubmitMenu_FileCPB_Get  (SubmitMenuCB_t *);
void          SubmitMenu_FileCPB_Put  (SubmitMenuCB_t *, Widget);
Widget        SubmitMenu_FilePDM_Get  (SubmitMenuCB_t *);
void          SubmitMenu_FilePDM_Put  (SubmitMenuCB_t *, Widget);
FilesCB_t    *SubmitMenu_FilesCB_Get  (SubmitMenuCB_t *);
Widget        SubmitMenu_FSavePB_Get  (SubmitMenuCB_t *);
void          SubmitMenu_FSavePB_Put  (SubmitMenuCB_t *, Widget);
Widget        SubmitMenu_FSbmtPB_Get  (SubmitMenuCB_t *);
void          SubmitMenu_FSbmtPB_Put  (SubmitMenuCB_t *, Widget);
Widget        SubmitMenu_FStatPB_Get  (SubmitMenuCB_t *);
void          SubmitMenu_FStatPB_Put  (SubmitMenuCB_t *, Widget);
Widget        SubmitMenu_HelpCPB_Get  (SubmitMenuCB_t *);
void          SubmitMenu_HelpCPB_Put  (SubmitMenuCB_t *, Widget);
Widget        SubmitMenu_HelpPB_Get   (SubmitMenuCB_t *);
void          SubmitMenu_HelpPB_Put   (SubmitMenuCB_t *, Widget);
Widget        SubmitMenu_HelpPDM_Get  (SubmitMenuCB_t *);
void          SubmitMenu_HelpPDM_Put  (SubmitMenuCB_t *, Widget);
Widget        SubmitMenu_MenuBar_Get  (SubmitMenuCB_t *);
void          SubmitMenu_MenuBar_Put  (SubmitMenuCB_t *, Widget);
Widget        SubmitMenu_OptCPB_Get   (SubmitMenuCB_t *);
void          SubmitMenu_OptCPB_Put   (SubmitMenuCB_t *, Widget);
Widget        SubmitMenu_OptPDM_Get   (SubmitMenuCB_t *);
void          SubmitMenu_OptPDM_Put   (SubmitMenuCB_t *, Widget);
Widget        SubmitMenu_ODirPB_Get   (SubmitMenuCB_t *);
void          SubmitMenu_ODirPB_Put   (SubmitMenuCB_t *, Widget);
Widget        SubmitMenu_OFilesPB_Get (SubmitMenuCB_t *);
void          SubmitMenu_OFilesPB_Put (SubmitMenuCB_t *, Widget);
Widget        SubmitMenu_OStratPB_Get (SubmitMenuCB_t *);
void          SubmitMenu_OStratPB_Put (SubmitMenuCB_t *, Widget);
StratCB_t    *SubmitMenu_StratCB_Get  (SubmitMenuCB_t *);
*/

#define  SubmitMenu_DirCB_Get(smcb_p)\
   &(smcb_p)->dir_cb
#define  SubmitMenu_FileCPB_Get(smcb_p)\
   (smcb_p)->file_cpb
#define  SubmitMenu_FileCPB_Put(smcb_p, value)\
   (smcb_p)->file_cpb = (value)
#define  SubmitMenu_FilePDM_Get(smcb_p)\
   (smcb_p)->file_pdm
#define  SubmitMenu_FilePDM_Put(smcb_p, value)\
   (smcb_p)->file_pdm = (value)
#define  SubmitMenu_FSbmtPB_Get(smcb_p)\
   (smcb_p)->f_sbmt_pb
#define  SubmitMenu_FSbmtPB_Put(smcb_p, value)\
   (smcb_p)->f_sbmt_pb = (value)
#define  SubmitMenu_FStatPB_Get(smcb_p)\
   (smcb_p)->f_stat_pb
#define  SubmitMenu_FStatPB_Put(smcb_p, value)\
   (smcb_p)->f_stat_pb = (value)
#define  SubmitMenu_FSavePB_Get(smcb_p)\
   (smcb_p)->f_save_pb
#define  SubmitMenu_FSavePB_Put(smcb_p, value)\
   (smcb_p)->f_save_pb = (value)
#define   SubmitMenu_FilesCB_Get(smcb_p)\
   &(smcb_p)->files_cb
#define  SubmitMenu_HelpCPB_Get(smcb_p)\
   (smcb_p)->help_cpb
#define  SubmitMenu_HelpCPB_Put(smcb_p, value)\
   (smcb_p)->help_cpb = (value)
#define  SubmitMenu_HelpPB_Get(smcb_p)\
   (smcb_p)->h_help_pb
#define  SubmitMenu_HelpPB_Put(smcb_p, value)\
   (smcb_p)->h_help_pb = (value)
#define  SubmitMenu_HelpPDM_Get(smcb_p)\
   (smcb_p)->help_pdm
#define  SubmitMenu_HelpPDM_Put(smcb_p, value)\
   (smcb_p)->help_pdm = (value)
#define  SubmitMenu_MenuBar_Get(smcb_p)\
   (smcb_p)->menu_bar
#define  SubmitMenu_MenuBar_Put(smcb_p, value)\
   (smcb_p)->menu_bar = (value)
#define SubmitMenu_OptCPB_Get(smcb_p)\
   (smcb_p)->options_cpb
#define SubmitMenu_OptCPB_Put(smcb_p, value)\
   (smcb_p)->options_cpb = (value)
#define  SubmitMenu_OptPDM_Get(smcb_p)\
   (smcb_p)->options_pdm
#define  SubmitMenu_OptPDM_Put(smcb_p, value)\
   (smcb_p)->options_pdm = (value)
#define  SubmitMenu_ODirPB_Get(smcb_p)\
   (smcb_p)->o_dir_pb
#define  SubmitMenu_ODirPB_Put(smcb_p, value)\
   (smcb_p)->o_dir_pb = (value)
#define  SubmitMenu_OFilesPB_Get(smcb_p)\
   (smcb_p)->o_files_pb
#define  SubmitMenu_OFilesPB_Put(smcb_p, value)\
   (smcb_p)->o_files_pb = (value)
#define  SubmitMenu_OStratPB_Get(smcb_p)\
   (smcb_p)->o_strategies_pb
#define  SubmitMenu_OStratPB_Put(smcb_p, value)\
   (smcb_p)->o_strategies_pb  = (value)
#define  SubmitMenu_StratCB_Get(smcb_p)\
   &(smcb_p)->strat_cb


/*** Routine Prototypes ***/  
  
void   DirsForm_Create        (DirCB_t *, Rcb_t *, Widget);
void   DirsForm_Values_Get    (DirCB_t *, Rcb_t *);
void   DirsForm_Values_Set    (DirCB_t *, Rcb_t *);
void   FilesForm_Create       (FilesCB_t *, Rcb_t *, Widget);
void   FilesForm_Values_Get   (FilesCB_t *, Rcb_t *);
void   FilesForm_Values_Set   (FilesCB_t *, Rcb_t *);
void   StratsForm_Create      (StratCB_t *, Rcb_t *, Widget); 
void   StratsForm_Values_Get  (StratCB_t *, Rcb_t *);
void   StratsForm_Values_Set  (StratCB_t *, Rcb_t *);
Widget SubmitMenu_Create      (SubmitCB_t *, Widget);
void   SubmitMenu_Destroy     (void);  
void   SubmitMenu_Dialogs_PopDown (void);
void   SubmitMenu_Forms_Get   (Rcb_t *);
void   SubmitMenu_Forms_Set   (Rcb_t *);

#endif  

/***  End of SUBMIT_MENU.H  ***/  
