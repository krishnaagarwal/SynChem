#ifndef _H_SUBMIT_ 
#define _H_SUBMIT_ 1 
/****************************************************************************
*  
* Copyright (C) 1997 Synchem Group at SUNY-Stony Brook, Daren Krebsbach  
*  
*  
*  Module Name:               SUBMIT.H  
*  
*    This header file defines the information needed for the    
*    job submission module.  
*      
*  
*  
*  Creation Date:  
*  
*     25-Aug-1997  
*  
*  Authors: 
*      Daren Krebsbach 
*        based on version by Nader Abdelsadek
*  
*****************************************************************************/  
 
#ifndef _H_RCB_
#include "rcb.h"
#endif

#ifndef _H_SUBMIT_DRAW_
#include "submit_draw.h"
#endif

#ifndef _H_SUBMIT_EXITCOND_
#include "submit_exitcond.h"
#endif

#ifndef _H_SUBMIT_JOBINFO_
#include "submit_jobinfo.h"
#endif

#ifndef _H_SUBMIT_RUNTYPE_
#include "submit_runtype.h"
#endif

/*** Literal Values ***/  

#define SBT_SUBMIT_TITLE      "Problem Submission" 
 
#define SBT_CANCEL_PB_LBL     "cancel" 
#define SBT_RESET_PB_LBL      "reset" 
#define SBT_SUBMIT_PB_LBL     "submit" 

typedef struct submit_cb_s 
  { 
  Rcb_t               temp_rcb;               /* Temp Run CB */
  ExitCondCB_t        exit_cond_cb;           /* Exit Condition cb */ 
  JobInfoCB_t         jobinfo_cb;             /* Compound and user cb */ 
  RunTypeCB_t         run_type_cb;            /* Run Type control block */ 
  Widget              submit_formdlg;         /* Submit form dialog */ 
  Widget              main_form;              /* Main form */
  Widget              cancel_pb;              /* Cancel PushButton */
  Widget              reset_pb;               /* Reset PushButton */
  Widget              submit_pb;              /* Submit PushButton */
  Boolean_t           cont_loaded;            /* Continue loaded PST? */
  Boolean_t           submit_created;         /* Has submit been created? */                                                                                                                                                                }  SubmitCB_t;

/*  Macro Prototypes for SubmitCB_t 

Widget          Submit_CancelPB_Get   (SubmitCB_t *);
void            Submit_CancelPB_Put   (SubmitCB_t *, Widget);
Boolean_t       Submit_ContLoaded_Get (SubmitCB_t *);
void            Submit_ContLoaded_Put (SubmitCB_t *);
Boolean_t       Submit_Created_Get    (SubmitCB_t *);
void            Submit_Created_Put    (SubmitCB_t *);
ExitCondCB_t   *Submit_ExitCondCB_Get (SubmitCB_t *);
Widget          Submit_FormDlg_Get    (SubmitCB_t *);
void            Submit_FormDlg_Put    (SubmitCB_t *, Widget);
JobInfoCB_t    *Submit_JobInfoCB_Get  (SubmitCB_t *);
Widget          Submit_MainForm_Get   (SubmitCB_t *);
void            Submit_MainForm_Put   (SubmitCB_t *, Widget);
Widget          Submit_ResetPB_Get    (SubmitCB_t *);
void            Submit_ResetPB_Put    (SubmitCB_t *, Widget);
RunTypeCB_t    *Submit_RunTypeCB_Get  (SubmitCB_t *);
Widget          Submit_SubmitPB_Get   (SubmitCB_t *);
void            Submit_SubmitPB_Put   (SubmitCB_t *, Widget);
Rcb_t          *Submit_TempRcb_Get    (SubmitCB_t *);
*/

#define Submit_CancelPB_Get(scb)\
   (scb)->cancel_pb
#define Submit_CancelPB_Put(scb, value)\
   (scb)->cancel_pb = (value)

#define Submit_ContLoaded_Get(scb)\
   (scb)->cont_loaded
#define Submit_ContLoaded_Put(scb, value)\
   (scb)->cont_loaded = (value)

#define Submit_Created_Get(scb)\
   (scb)->submit_created
#define Submit_Created_Put(scb, value)\
   (scb)->submit_created = (value)

#define  Submit_ExitCondCB_Get(scb)\
   &(scb)->exit_cond_cb

#define  Submit_FormDlg_Get(scb)\
   (scb)->submit_formdlg
#define  Submit_FormDlg_Put(scb, value)\
   (scb)->submit_formdlg = (value)

#define  Submit_JobInfoCB_Get(scb)\
   &(scb)->jobinfo_cb

#define Submit_MainForm_Get(scb)\
   (scb)->main_form
#define Submit_MainForm_Put(scb, value)\
   (scb)->main_form = (value)

#define Submit_ResetPB_Get(scb)\
   (scb)->reset_pb
#define Submit_ResetPB_Put(scb, value)\
   (scb)->reset_pb = (value)

#define Submit_RunTypeCB_Get(scb)\
   &(scb)->run_type_cb

#define Submit_SubmitPB_Get(scb)\
   (scb)->submit_pb
#define Submit_SubmitPB_Put(scb, value)\
   (scb)->submit_pb = (value)

#define Submit_TempRcb_Get(scb)\
   &(scb)->temp_rcb


/*** Function Prototypes ***/

void      Submission_Create    (SubmitCB_t *, Widget, XtAppContext *);
void      Submission_Destroy   (SubmitCB_t *);
Boolean_t Submission_Dump      (Rcb_t *);
void      Submission_Execute   (SubmitCB_t *);
Boolean_t Submit_IsDistrib     (SubmitCB_t *);
void      Submit_Values_Get    (SubmitCB_t *, Rcb_t *, Boolean_t);
void      Submit_Values_Set    (SubmitCB_t *, Rcb_t *);

/** Callback Prototypes **/ 

void Submit_CancelPB_CB        (Widget, XtPointer, XtPointer);
void Submit_ResetPB_CB         (Widget, XtPointer, XtPointer);
void Submit_SubmitPB_CB        (Widget, XtPointer, XtPointer);
void Submit_NumVerify_CB       (Widget, XtPointer, XtPointer);

#endif

/*** End of SUBMIT.H ***/ 




