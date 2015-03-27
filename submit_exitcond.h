#ifndef _H_SUBMIT_EXITCOND_  
#define _H_SUBMIT_EXITCOND_  1  
/****************************************************************************
*  
* Copyright (C) 1997 Synchem Group at SUNY-Stony Brook, Daren Krebsbach  
*  
*  
*  Module Name:               SUBMIT_EXITCOND.H  
*  
*    This header file defines the information needed for the    
*    exit conditions panel of the job submission module.  
*      
*  
*  
*  Creation Date:  
*  
*     25-Aug-1997  
*  
*  Authors: 
*      Daren Krebsbach 
*        based very loosely on a version by Nader Abdelsadek
*  
*****************************************************************************/  

#ifndef _H_RCB_
#include "rcb.h"
#endif

/*** Literal Values ***/  

#define SEC_EXITCOND_TITLE   "Exit Conditions:"
#define SEC_MAXCYCLES_LBL    "Maximum Cycles:" 
#define SEC_MAXTIME_LBL      "Maximum Time:" 
#define SEC_FIRSTSOL_RB_LBL  "First Solution" 
#define SEC_ADDCYCLES_LBL    "Additional Cycles:" 
#define SEC_ADDTIME_LBL      "Additional Time:" 
 
#define SEC_MAX_FIELD_SIZE   5


/*** ExitCondCB_t Structure Definition ***/
 
typedef struct exit_conditions_cb_s 
  { 
  Widget     form; 
  Widget     frame; 
  Widget     frame_title;
  Widget     lbl_max_cycles; 
  Widget     txt_max_cycles; 
  Widget     lbl_max_time; 
  Widget     txt_max_time; 
  Widget     togb_first_solution; 
  Widget     lbl_add_cycles; 
  Widget     txt_add_cycles; 
  Widget     lbl_add_time; 
  Widget     txt_add_time; 
  Widget     lbl_blank; 
  Widget     pb_reset;
  char       max_cycles[SEC_MAX_FIELD_SIZE + 1];
  char       max_time[SEC_MAX_FIELD_SIZE + 1];
  char       add_cycles[SEC_MAX_FIELD_SIZE + 1];
  char       add_time[SEC_MAX_FIELD_SIZE + 1];
  }  ExitCondCB_t; 

/* Macro Prototypes for ExitCondCB_t

char     *ExitCondCB_AddCycles_Get    (ExitCondCB_t *);
Widget    ExitCondCB_AddCyclesLbl_Get (ExitCondCB_t *);
void      ExitCondCB_AddCyclesLbl_Put (ExitCondCB_t *, Widget);
Widget    ExitCondCB_AddCyclesTxt_Get (ExitCondCB_t *);
void      ExitCondCB_AddCyclesTxt_Put (ExitCondCB_t *, Widget);
char     *ExitCondCB_AddTime_Get      (ExitCondCB_t *);
Widget    ExitCondCB_AddTimeLbl_Get   (ExitCondCB_t *);
void      ExitCondCB_AddTimeLbl_Put   (ExitCondCB_t *, Widget);
Widget    ExitCondCB_AddTimeTxt_Get   (ExitCondCB_t *);
void      ExitCondCB_AddTimeTxt_Put   (ExitCondCB_t *, Widget);
Widget    ExitCondCB_FirstSolTB_Get   (ExitCondCB_t *);
void      ExitCondCB_FirstSolTB_Put   (ExitCondCB_t *, Widget);
Widget    ExitCondCB_Form_Get         (ExitCondCB_t *);
void      ExitCondCB_Form_Put         (ExitCondCB_t *, Widget);
Widget    ExitCondCB_Frame_Get        (ExitCondCB_t *);
void      ExitCondCB_Frame_Put        (ExitCondCB_t *, Widget);
Widget    ExitCondCB_FrameTitle_Get   (ExitCondCB_t *);
void      ExitCondCB_FrameTitle_Put   (ExitCondCB_t *, Widget);
char     *ExitCondCB_MaxCycles_Get    (ExitCondCB_t *);
Widget    ExitCondCB_MaxCyclesLbl_Get (ExitCondCB_t *);
void      ExitCondCB_MaxCyclesLbl_Put (ExitCondCB_t *, Widget); 
Widget    ExitCondCB_MaxCyclesTxt_Get (ExitCondCB_t *);
void      ExitCondCB_MaxCyclesTxt_Put (ExitCondCB_t *, Widget);
char     *ExitCondCB_MaxTime_Get      (ExitCondCB_t *);
Widget    ExitCondCB_MaxTimeLbl_Get   (ExitCondCB_t *);
void      ExitCondCB_MaxTimeLbl_Put   (ExitCondCB_t, Widget);
Widget    ExitCondCB_MaxTimeTxt_Get   (ExitCondCB_t *);
void      ExitCondCB_MaxTimeTxt_Put   (ExitCondCB_t *, Widget);
Widget    ExitCondCB_ResetPB_Get      (ExitCondCB_t *);
void      ExitCondCB_ResetPB_Put      (ExitCondCB_t *, Widget);
*/

#define   ExitCondCB_AddCycles_Get(ecf_p)\
   (ecf_p)->add_cycles
#define   ExitCondCB_AddCyclesLbl_Get(ecf_p)\
   (ecf_p)->lbl_add_cycles
#define   ExitCondCB_AddCyclesLbl_Put(ecf_p, value)\
   (ecf_p)->lbl_add_cycles = (value)
#define   ExitCondCB_AddCyclesTxt_Get(ecf_p)\
   (ecf_p)->txt_add_cycles
#define   ExitCondCB_AddCyclesTxt_Put(ecf_p, value)\
   (ecf_p)->txt_add_cycles = (value)
#define   ExitCondCB_AddTime_Get(ecf_p)\
   (ecf_p)->add_time 
#define   ExitCondCB_AddTimeLbl_Get(ecf_p)\
   (ecf_p)->lbl_add_time 
#define   ExitCondCB_AddTimeLbl_Put(ecf_p, value)\
   (ecf_p)->lbl_add_time = (value)
#define   ExitCondCB_AddTimeTxt_Get(ecf_p)\
   (ecf_p)->txt_add_time
#define   ExitCondCB_AddTimeTxt_Put(ecf_p, value)\
   (ecf_p)->txt_add_time = (value)
#define   ExitCondCB_FirstSolTB_Get(ecf_p)\
   (ecf_p)->togb_first_solution
#define   ExitCondCB_FirstSolTB_Put(ecf_p, value)\
   (ecf_p)->togb_first_solution = (value)
#define   ExitCondCB_Form_Get(ecf_p)\
   (ecf_p)->form
#define   ExitCondCB_Form_Put(ecf_p, value)\
   (ecf_p)->form = (value)
#define   ExitCondCB_Frame_Get(ecf_p)\
   (ecf_p)->frame
#define   ExitCondCB_Frame_Put(ecf_p, value)\
   (ecf_p)->frame = (value)
#define   ExitCondCB_FrameTitle_Get(ecf_p)\
   (ecf_p)->frame_title
#define   ExitCondCB_FrameTitle_Put(ecf_p, value)\
   (ecf_p)->frame_title = (value)
#define   ExitCondCB_MaxCycles_Get(ecf_p)\
   (ecf_p)->max_cycles
#define   ExitCondCB_MaxCyclesLbl_Get(ecf_p)\
   (ecf_p)->lbl_max_cycles 
#define   ExitCondCB_MaxCyclesLbl_Put(ecf_p, value)\
   (ecf_p)->lbl_max_cycles = (value)
#define   ExitCondCB_MaxCyclesTxt_Get(ecf_p)\
   (ecf_p)->txt_max_cycles
#define   ExitCondCB_MaxCyclesTxt_Put(ecf_p, value)\
   (ecf_p)->txt_max_cycles = (value)
#define   ExitCondCB_MaxTime_Get(ecf_p)\
   (ecf_p)->max_time
#define   ExitCondCB_MaxTimeLbl_Get(ecf_p)\
   (ecf_p)->lbl_max_time
#define   ExitCondCB_MaxTimeLbl_Put(ecf_p, value)\
   (ecf_p)->lbl_max_time = (value)
#define   ExitCondCB_MaxTimeTxt_Get(ecf_p)\
   (ecf_p)->txt_max_time 
#define   ExitCondCB_MaxTimeTxt_Put(ecf_p, value)\
   (ecf_p)->txt_max_time = (value)
#define   ExitCondCB_ResetPB_Get(ecf_p)\
   (ecf_p)->pb_reset
#define   ExitCondCB_ResetPB_Put(ecf_p, value)\
   (ecf_p)->pb_reset = (value)


/*** Function Prototype Declarations ***/

Widget ExitCond_Create         (ExitCondCB_t *, Widget);
void   ExitCond_Values_Get     (ExitCondCB_t *, Rcb_t *);
void   ExitCond_Values_Set     (ExitCondCB_t *, Rcb_t *);


/*** CallBack Prototypes Declarations ***/

void   ExitCond_Reset_CB       (Widget, XtPointer, XtPointer);


#endif

/*** End Of SUBMIT_EXITCOND.H ***/
