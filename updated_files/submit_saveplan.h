#ifndef _H_SUBMIT_SAVEPLAN_  
#define _H_SUBMIT_SAVEPLAN_  1  
/****************************************************************************
*  
* Copyright (C) 2001 Synchem Group at SUNY-Stony Brook, Gerald A. Miller 
*  
*  
*  Module Name:               SUBMIT_SAVEPLAN.H  
*  
*    This header file defines the information needed for the    
*    save plan panel of the job submission module.  
*      
*  
*  
*  Creation Date:  
*  
*     04-Sep-2001  
*  
*  Authors: 
*      Gerald A. Miller
*        based on submit_exitcond by Daren Krebsbach
*  
*****************************************************************************/  

/*** Literal Values ***/  

#define SSP_SAVEPLAN_TITLE     "Auto-Save Parameters:"
#define SSP_PRECYCLES_LBL      "Save every" 
#define SSP_POSTCYCLES_LBL     "cycles,"
#define SSP_BEFOREAFTER_LBL    "starting at" 
#define SSP_AFTERAFTER_LBL     "cycles" 
#define SSP_MAXTIME_MAXCYC_LBL "Save at end of run" 
#define SSP_FIRSTSOL_SAVE_LBL  "Save at 1st solution" 
 
#define SSP_MAX_FIELD_SIZE   5

typedef struct runtime_rcb_s
  {
  union
    {
    U32_t pack;
    Boolean_t save[3];
    } flags;
  U32_t save_cycles;
  U32_t after_cycles;
  } RT_Rcb_t;

/*
Boolean_t RTRcb_CycleSave_Flag_Get (RT_Rcb_t *);
void      RTRcb_CycleSave_Flag_Put (RT_Rcb_t *, Boolean_t);
Boolean_t RTRcb_FinalSave_Flag_Get (RT_Rcb_t *);
void      RTRcb_FinalSave_Flag_Put (RT_Rcb_t *, Boolean_t);
Boolean_t RTRcb_1stSolSave_Flag_Get (RT_Rcb_t *);
void      RTRcb_1stSolSave_Flag_Put (RT_Rcb_t *, Boolean_t);
U32_t     RTRcb_CycleIncr_Get (RTRcb_t *);
void      RTRcb_CycleIncr_Put (RTRcb_t *, U32_t);
U32_t     RTRcb_CycleStart_Get (RTRcb_t *);
void      RTRcb_CycleStart_Put (RTRcb_t *, U32_t);
*/

#define RTRcb_CycleSave_Flag_Get(rp) \
  (rp)->flags.save[0]
#define RTRcb_CycleSave_Flag_Put(rp, value) \
  (rp)->flags.save[0] = (value)
#define RTRcb_FinalSave_Flag_Get(rp) \
  (rp)->flags.save[1]
#define RTRcb_FinalSave_Flag_Put(rp, value) \
  (rp)->flags.save[1] = (value)
#define RTRcb_1stSolSave_Flag_Get(rp) \
  (rp)->flags.save[2]
#define RTRcb_1stSolSave_Flag_Put(rp, value) \
  (rp)->flags.save[2] = (value)
#define RTRcb_CycleIncr_Get(rp) \
  (rp)->save_cycles
#define RTRcb_CycleIncr_Put(rp, value) \
  (rp)->save_cycles = (value)
#define RTRcb_CycleStart_Get(rp) \
  (rp)->after_cycles
#define RTRcb_CycleStart_Put(rp, value) \
  (rp)->after_cycles = (value)

/*** SavePlanCB_t Structure Definition ***/
 
typedef struct saveplan_cb_s 
  { 
  Widget     form; 
  Widget     frame; 
  Widget     frame_title;
  Widget     cbx_cycles;
  Widget     lbl_precycles; 
  Widget     txt_num_cycles; 
  Widget     lbl_postcycles; 
  Widget     lbl_before_after_cycles;
  Widget     txt_after_cycles;
  Widget     lbl_after_after_cycles;
  Widget     cbx_maxmax;
  Widget     lbl_maxtime_maxcyc; 
  Widget     cbx_1st_sol; 
  Widget     lbl_first_sol; 
  Widget     pb_reset;
  char       num_cycles[SSP_MAX_FIELD_SIZE + 1];
  char       after_cycles[SSP_MAX_FIELD_SIZE + 1];
  }  SavePlanCB_t; 

/* Macro Prototypes for SavePlanCB_t

Widget    SavePlanCB_AfterAfterLbl_Get  (SavePlanCB_t *);
void      SavePlanCB_AfterAfterLbl_Put  (SavePlanCB_t *, Widget);
char     *SavePlanCB_AfterCycles_Get    (SavePlanCB_t *);
Widget    SavePlanCB_AfterCyclesTxt_Get (SavePlanCB_t *);
void      SavePlanCB_AfterCyclesTxt_Put (SavePlanCB_t *, Widget);
Widget    SavePlanCB_BeforeAfterLbl_Get (SavePlanCB_t *);
void      SavePlanCB_BeforeAfterLbl_Put (SavePlanCB_t *, Widget);
char     *SavePlanCB_Cycles_Get         (SavePlanCB_t *);
Widget    SavePlanCB_CyclesTB_Get       (SavePlanCB_t *);
void      SavePlanCB_CyclesTB_Put       (SavePlanCB_t *, Widget);
Widget    SavePlanCB_CyclesTxt_Get      (SavePlanCB_t *);
void      SavePlanCB_CyclesTxt_Put      (SavePlanCB_t *, Widget);
Widget    SavePlanCB_FirstSolLbl_Get    (SavePlanCB_t *);
void      SavePlanCB_FirstSolLbl_Put    (SavePlanCB_t *, Widget);
Widget    SavePlanCB_FirstSolTB_Get     (SavePlanCB_t *);
void      SavePlanCB_FirstSolTB_Put     (SavePlanCB_t *, Widget);
Widget    SavePlanCB_Form_Get           (SavePlanCB_t *);
void      SavePlanCB_Form_Put           (SavePlanCB_t *, Widget);
Widget    SavePlanCB_Frame_Get          (SavePlanCB_t *);
void      SavePlanCB_Frame_Put          (SavePlanCB_t *, Widget);
Widget    SavePlanCB_FrameTitle_Get     (SavePlanCB_t *);
void      SavePlanCB_FrameTitle_Put     (SavePlanCB_t *, Widget);
Widget    SavePlanCB_MaxMaxLbl_Get      (SavePlanCB_t *);
void      SavePlanCB_MaxMaxLbl_Put      (SavePlanCB_t *, Widget);
Widget    SavePlanCB_MaxMaxTB_Get       (SavePlanCB_t *);
void      SavePlanCB_MaxMaxTB_Put       (SavePlanCB_t *, Widget);
Widget    SavePlanCB_PostCyclesLbl_Get  (SavePlanCB_t *);
void      SavePlanCB_PostCyclesLbl_Put  (SavePlanCB_t *, Widget); 
Widget    SavePlanCB_PreCyclesLbl_Get   (SavePlanCB_t *);
void      SavePlanCB_PreCyclesLbl_Put   (SavePlanCB_t *, Widget); 
Widget    SavePlanCB_ResetPB_Get        (SavePlanCB_t *);
void      SavePlanCB_ResetPB_Put        (SavePlanCB_t *, Widget);
*/

#define   SavePlanCB_AfterAfterLbl_Get(spf_p)\
   (spf_p)->lbl_after_after_cycles
#define   SavePlanCB_AfterAfterLbl_Put(spf_p, value)\
   (spf_p)->lbl_after_after_cycles = (value)
#define   SavePlanCB_AfterCycles_Get(spf_p)\
   (spf_p)->after_cycles
#define   SavePlanCB_AfterCyclesTxt_Get(spf_p)\
   (spf_p)->txt_after_cycles
#define   SavePlanCB_AfterCyclesTxt_Put(spf_p, value)\
   (spf_p)->txt_after_cycles = (value)
#define   SavePlanCB_BeforeAfterLbl_Get(spf_p)\
   (spf_p)->lbl_before_after_cycles
#define   SavePlanCB_BeforeAfterLbl_Put(spf_p, value)\
   (spf_p)->lbl_before_after_cycles = (value)
#define   SavePlanCB_Cycles_Get(spf_p)\
   (spf_p)->num_cycles
#define   SavePlanCB_CyclesTB_Get(spf_p)\
   (spf_p)->cbx_cycles
#define   SavePlanCB_CyclesTB_Put(spf_p, value)\
   (spf_p)->cbx_cycles = (value)
#define   SavePlanCB_CyclesTxt_Get(spf_p)\
   (spf_p)->txt_num_cycles
#define   SavePlanCB_CyclesTxt_Put(spf_p, value)\
   (spf_p)->txt_num_cycles = (value)
#define   SavePlanCB_FirstSolLbl_Get(spf_p)\
   (spf_p)->lbl_first_sol
#define   SavePlanCB_FirstSolLbl_Put(spf_p, value)\
   (spf_p)->lbl_first_sol = (value)
#define   SavePlanCB_FirstSolTB_Get(spf_p)\
   (spf_p)->cbx_1st_sol
#define   SavePlanCB_FirstSolTB_Put(spf_p, value)\
   (spf_p)->cbx_1st_sol = (value)
#define   SavePlanCB_Form_Get(spf_p)\
   (spf_p)->form
#define   SavePlanCB_Form_Put(spf_p, value)\
   (spf_p)->form = (value)
#define   SavePlanCB_Frame_Get(spf_p)\
   (spf_p)->frame
#define   SavePlanCB_Frame_Put(spf_p, value)\
   (spf_p)->frame = (value)
#define   SavePlanCB_FrameTitle_Get(spf_p)\
   (spf_p)->frame_title
#define   SavePlanCB_FrameTitle_Put(spf_p, value)\
   (spf_p)->frame_title = (value)
#define   SavePlanCB_MaxMaxLbl_Get(spf_p)\
   (spf_p)->lbl_maxtime_maxcyc
#define   SavePlanCB_MaxMaxLbl_Put(spf_p, value)\
   (spf_p)->lbl_maxtime_maxcyc = (value)
#define   SavePlanCB_MaxMaxTB_Get(spf_p)\
   (spf_p)->cbx_maxmax
#define   SavePlanCB_MaxMaxTB_Put(spf_p, value)\
   (spf_p)->cbx_maxmax = (value)
#define   SavePlanCB_PostCyclesLbl_Get(spf_p)\
   (spf_p)->lbl_postcycles
#define   SavePlanCB_PostCyclesLbl_Put(spf_p, value)\
   (spf_p)->lbl_postcycles = (value)
#define   SavePlanCB_PreCyclesLbl_Get(spf_p)\
   (spf_p)->lbl_precycles
#define   SavePlanCB_PreCyclesLbl_Put(spf_p, value)\
   (spf_p)->lbl_precycles = (value)
#define   SavePlanCB_ResetPB_Get(spf_p)\
   (spf_p)->pb_reset
#define   SavePlanCB_ResetPB_Put(spf_p, value)\
   (spf_p)->pb_reset = (value)

#ifdef _GLOBAL_DEF_
RT_Rcb_t GRTRcb;
#else
extern RT_Rcb_t GRTRcb;
#endif

/*** Function Prototype Declarations ***/

Widget SavePlan_Create         (SavePlanCB_t *, Widget);
void   SavePlan_Values_Get     (SavePlanCB_t *);
void   SavePlan_Values_Set     (SavePlanCB_t *);


/*** CallBack Prototypes Declarations ***/

void   SavePlan_Reset_CB       (Widget, XtPointer, XtPointer);


#endif

/*** End Of SUBMIT_SAVEPLAN.H ***/
