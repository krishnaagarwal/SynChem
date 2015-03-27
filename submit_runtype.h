#ifndef _H_SUBMIT_RUNTYPE_ 
#define _H_SUBMIT_RUNTYPE_  1 
/****************************************************************************
*  
* Copyright (C) 1997 Synchem Group at SUNY-Stony Brook, Daren Krebsbach  
*  
*  
*  Module Name:               SUBMIT_RUNTYPE.H  
*  
*    This header file defines the information needed for the    
*    run type panel of the job submission module.  
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
 
/*** Literal Values ***/ 

#define SMU_RUNTYPE_TITLE               "Run Types and Search Options:"
#define SMU_SEQUENTIAL_RB_LBL		"Sequential"
#define SMU_SEQUENTIAL_RB_IND           0
#define SMU_DISTRIBUTED_RB_LBL		"Distributed" 
#define SMU_DISTRIBUTED_RB_IND		1 
#define SMU_BACKGROUND_TB_LBL           "Background" 
#define SMU_CONTINUE_TB_LBL             "Continue Search" 
#define SMU_EFFORT_DIST_TB_LBL		"Effort Distribution" 
#define SMU_NTCL_LBL  			"Temporary Closure:" 
#define SMU_RESET_PB_LBL		"reset"
#define SMU_NTCL_BUFF_MAXLEN            4

/***  RunTypeCB_t Structure Definition ***/
 
typedef struct runtype_cb_s 
  { 
  Widget     form;
  Widget     frame;
  Widget     type_form;
  Widget     type_frame;
  Widget     rb_seq_dis;
  Widget     tb_background;
  Widget     tb_continue;
  Widget     search_form;
  Widget     search_frame;
  Widget     tb_effort_dist; 
  Widget     lbl_ntcl;
  Widget     txt_ntcl;
  Widget     pb_reset;
  Boolean_t  show_dist;
  char       ntcl_buff[SMU_NTCL_BUFF_MAXLEN + 1];
  }  RunTypeCB_t; 

/*  Macro Prototypes for RunTypeCB_t

Widget   RunTypeCB_BackGroundTB_Get (RunTypeCB_t *);
void     RunTypeCB_BackGroundTB_Put (RunTypeCB_t *, Widget);
Widget   RunTypeCB_ContinueTB_Get   (RunTypeCB_t *);
void     RunTypeCB_ContinueTB_Put   (RunTypeCB_t *, Widget);
Widget   RunTypeCB_EffDistTB_Get    (RunTypeCB_t *);
void     RunTypeCB_EffDistTB_Put    (RunTypeCB_t *, Widget);
Widget   RunTypeCB_Form_Get         (RunTypeCB_t *);
void     RunTypeCB_Form_Put         (RunTypeCB_t *, Widget);
Widget   RunTypeCB_Frame_Get        (RunTypeCB_t *);
void     RunTypeCB_Frame_Put        (RunTypeCB_t *, Widget);
char    *RunTypeCB_NtclBuf_Get      (RunTypeCB_t *);
Widget   RunTypeCB_NtclLbl_Get      (RunTypeCB_t *);
void     RunTypeCB_NtclLbl_Put      (RunTypeCB_t *, Widget);
Widget   RunTypeCB_NtclTxt_Get      (RunTypeCB_t *);
void     RunTypeCB_NtclTxt_Put      (RunTypeCB_t *, Widget);
Widget   RunTypeCB_ResetPB_Get      (RunTypeCB_t *);
void     RunTypeCB_ResetPB_Put      (RunTypeCB_t *, Widget);
Widget   RunTypeCB_SearchForm_Get   (RunTypeCB_t *);
void     RunTypeCB_SearchForm_Put   (RunTypeCB_t *, Widget);
Widget   RunTypeCB_SearchFrame_Get  (RunTypeCB_t *);
void     RunTypeCB_SearchFrame_Put  (RunTypeCB_t *, Widget);
Widget   RunTypeCB_SeqDisRB_Get     (RunTypeCB_t *);
void     RunTypeCB_SeqDisRB_Put     (RunTypeCB_t *, Widget);
U8_t     RunTypeCB_ShowDistPar_Get  (RunTypeCB_t *);
void     RunTypeCB_ShowDistPar_Put  (RunTypeCB_t *, U8_t);
Widget   RunTypeCB_TypeForm_Get     (RunTypeCB_t *);
void     RunTypeCB_TypeForm_Put     (RunTypeCB_t *, Widget);
Widget   RunTypeCB_TypeFrame_Get    (RunTypeCB_t *);
void     RunTypeCB_TypeFrame_Put    (RunTypeCB_t *, Widget);
*/

#define  RunTypeCB_BackGroundTB_Get(rtc_p)\
   (rtc_p)->tb_background
#define  RunTypeCB_BackGroundTB_Put(rtc_p, value)\
   (rtc_p)->tb_background = (value)
#define  RunTypeCB_ContinueTB_Get(rtc_p)\
   (rtc_p)->tb_continue
#define  RunTypeCB_ContinueTB_Put(rtc_p, value)\
   (rtc_p)->tb_continue = (value)
#define  RunTypeCB_EffDistTB_Get(rtc_p)\
   (rtc_p)->tb_effort_dist
#define  RunTypeCB_EffDistTB_Put(rtc_p, value)\
   (rtc_p)->tb_effort_dist = (value)
#define  RunTypeCB_Form_Get(rtc_p)\
   (rtc_p)->form
#define  RunTypeCB_Form_Put(rtc_p, value)\
   (rtc_p)->form = (value)
#define  RunTypeCB_Frame_Get(rtc_p)\
   (rtc_p)->frame
#define  RunTypeCB_Frame_Put(rtc_p, value)\
   (rtc_p)->frame = (value)
#define  RunTypeCB_NtclBuf_Get(rtc_p)\
   (rtc_p)->ntcl_buff
#define  RunTypeCB_NtclLbl_Get(rtc_p)\
   (rtc_p)->lbl_ntcl
#define  RunTypeCB_NtclLbl_Put(rtc_p, value)\
   (rtc_p)->lbl_ntcl = (value)
#define  RunTypeCB_NtclTxt_Get(rtc_p)\
   (rtc_p)->txt_ntcl
#define  RunTypeCB_NtclTxt_Put(rtc_p, value)\
   (rtc_p)->txt_ntcl = (value)
#define  RunTypeCB_ResetPB_Get(rtc_p)\
   (rtc_p)->pb_reset
#define  RunTypeCB_ResetPB_Put(rtc_p, value)\
   (rtc_p)->pb_reset = (value)
#define  RunTypeCB_SearchForm_Get(rtc_p)\
   (rtc_p)->search_form
#define  RunTypeCB_SearchForm_Put(rtc_p, value)\
   (rtc_p)->search_form = (value)
#define  RunTypeCB_SearchFrame_Get(rtc_p)\
   (rtc_p)->search_frame
#define  RunTypeCB_SearchFrame_Put(rtc_p, value)\
   (rtc_p)->search_frame = (value)
#define  RunTypeCB_SeqDisRB_Get(rtc_p)\
   (rtc_p)->rb_seq_dis
#define  RunTypeCB_SeqDisRB_Put(rtc_p, value)\
   (rtc_p)->rb_seq_dis = (value)
#define  RunTypeCB_ShowDistPar_Get(rtc_p)\
   (rtc_p)->show_dist
#define  RunTypeCB_ShowDistPar_Put(rtc_p, value)\
   (rtc_p)->show_dist = (value)
#define  RunTypeCB_TypeForm_Get(rtc_p)\
   (rtc_p)->type_form
#define  RunTypeCB_TypeForm_Put(rtc_p, value)\
   (rtc_p)->type_form = (value)
#define  RunTypeCB_TypeFrame_Get(rtc_p)\
   (rtc_p)->type_frame
#define  RunTypeCB_TypeFrame_Put(rtc_p, value)\
   (rtc_p)->type_frame = (value)

/*** Function Prototypes ***/

Widget RunType_Create        (RunTypeCB_t *, Widget); 
void   RunType_Values_Get    (RunTypeCB_t *, Rcb_t *);
void   RunType_Values_Set    (RunTypeCB_t *, Rcb_t *);

/** CallBack Prototypes  **/
void   RunType_Reset_CB      (Widget, XtPointer, XtPointer); 

#endif 

/*** End Of SUBMIT_RUNTYPE.H ***/




