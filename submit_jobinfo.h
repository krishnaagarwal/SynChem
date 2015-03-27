#ifndef _H_SUBMIT_JOBINFO_ 
#define _H_SUBMIT_JOBINFO_  1 
/****************************************************************************
*  
* Copyright (C) 1997 Synchem Group at SUNY-Stony Brook, Daren Krebsbach  
*  
*  
*  Module Name:               SUBMIT_JOBINFO.H  
*  
*    This header file defines the information needed for the    
*    compound and  user information panel of the job submission module.  
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

#ifndef _H_DSP_
#include "dsp.h"
#endif

#ifndef _H_SUBMIT_TEMPLATES_
#include "submit_templates.h"
#endif

/*** Literal Values ***/ 

#define SJI_JOBINFO_TITLE    "Problem Submission Information:"
#define SJI_USER_LBL         "User:" 
#define SJI_DATE_LBL         "Date:" 
#define SJI_COMMENT_LBL      "Comment:" 
#define SJI_RUNID_LBL        "Run ID:"
#define SJI_SLING_LBL        "Sling:"

#define SJI_COMNTBUF_MAXLEN   80
#define SJI_COMNTTXT_MAXLEN   80
#define SJI_DATEBUF_MAXLEN    48
#define SJI_DATETXT_MAXLEN    30
#define SJI_RUNIDBUF_MAXLEN   80
#define SJI_RUNIDTXT_MAXLEN   80
#define SJI_SLINGBUF_MAXLEN   255
#define SJI_SLINGTXT_MAXLEN   80
#define SJI_USERBUF_MAXLEN    48
#define SJI_USERTXT_MAXLEN    30

#define SJI_COMP_PB_LBL      "templates" 
#define SJI_DRAW_PB_LBL      "draw" 
#define SJI_RESET_PB_LBL     "reset" 


typedef struct jobinfo_cb_s 
  { 
  TemplateCB_t         templates;
  Widget               form; 
  Widget               frame; 
  Widget               lbl_user; 
  Widget               txt_user; 
  Widget               lbl_date; 
  Widget               txt_date; 
  Widget               lbl_comment; 
  Widget               txt_comment; 
  Widget               lbl_runid; 
  Widget               txt_runid;
  Widget               lbl_sling; 
  Widget               txt_sling; 
  Widget               pb_reset; 
  Widget               pb_draw;
  Widget               pb_compound;
  char                 comment[SJI_COMNTBUF_MAXLEN + 1];
  char                 date[SJI_DATEBUF_MAXLEN + 1];
  char                 runid[SJI_RUNIDBUF_MAXLEN + 1];
  char                 sling[SJI_SLINGBUF_MAXLEN + 1];
  char                 user[SJI_USERBUF_MAXLEN + 1];
  }  JobInfoCB_t; 

/* Macro Prototypes for JobInfoCB_t

char         *JobInfo_Comment_Get      (JobInfoCB_t *);
Widget        JobInfo_CommentLbl_Get   (JobInfoCB_t *);
void          JobInfo_CommentLbl_Put   (JobInfoCB_t *, Widget);
Widget        JobInfo_CommentTxt_Get   (JobInfoCB_t *);
void          JobInfo_CommentTxt_Put   (JobInfoCB_t *, Widget);
Widget        JobInfo_CompsPB_Get      (JobInfoCB_t *);
void          JobInfo_CompsPB_Put      (JobInfoCB_t *, Widget);
char         *JobInfo_Date_Get         (JobInfoCB_t *);
Widget        JobInfo_DateLbl_Get      (JobInfoCB_t *);
void          JobInfo_DateLbl_Put      (JobInfoCB_t *, Widget);
Widget        JobInfo_DateTxt_Get      (JobInfoCB_t *);
void          JobInfo_DateTxt_Put      (JobInfoCB_t *, Widget);
Widget        JobInfo_DrawPB_Get       (JobInfoCB_t *);
void          JobInfo_DrawPB_Put       (JobInfoCB_t *, Widget);
Widget        JobInfo_Form_Get         (JobInfoCB_t *);
void          JobInfo_Form_Put         (JobInfoCB_t *, Widget);
Widget        JobInfo_Frame_Get        (JobInfoCB_t *);
void          JobInfo_Frame_Put        (JobInfoCB_t *, Widget);
Widget        JobInfo_ResetPB_Get      (JobInfoCB_t *);
void          JobInfo_ResetPB_Put      (JobInfoCB_t *, Widget);
char         *JobInfo_RunId_Get        (JobInfoCB_t *);
Widget        JobInfo_RunIdLbl_Get     (JobInfoCB_t *);
void          JobInfo_RunIdLbl_Put     (JobInfoCB_t *, Widget);
Widget        JobInfo_RunIdTxt_Get     (JobInfoCB_t *);
void          JobInfo_RunIdTxt_Put     (JobInfoCB_t *, Widget);  
char         *JobInfo_Sling_Get        (JobInfoCB_t *);
Widget        JobInfo_SlingLbl_Get     (JobInfoCB_t *);
void          JobInfo_SlingLbl_Put     (JobInfoCB_t *, Widget);
Widget        JobInfo_SlingTxt_Get     (JobInfoCB_t *);
void          JobInfo_SlingTxt_Put     (JobInfoCB_t *, Widget); 
TemplateCB_t *JobInfo_Templates_Get    (JobInfoCB_t *);
char         *JobInfo_User_Get         (JobInfoCB_t *);
Widget        JobInfo_UserLbl_Get      (JobInfoCB_t *);
void          JobInfo_UserLbl_Put      (JobInfoCB_t *, Widget);
Widget        JobInfo_UserTxt_Get      (JobInfoCB_t *);
void          JobInfo_UserTxt_Put      (JobInfoCB_t *, Widget);
*/

/*** JobInfoCB_t Macros ***/

#define  JobInfo_Comment_Get(cucb)\
   (cucb)->comment
#define  JobInfo_CommentLbl_Get(cucb)\
   (cucb)->lbl_comment
#define  JobInfo_CommentLbl_Put(cucb, value)\
   (cucb)->lbl_comment = (value)
#define  JobInfo_CommentTxt_Get(cucb)\
   (cucb)->txt_comment
#define  JobInfo_CommentTxt_Put(cucb, value)\
   (cucb)->txt_comment = (value)
#define  JobInfo_CompsPB_Get(cucb)\
   (cucb)->pb_compound
#define  JobInfo_CompsPB_Put(cucb, value)\
   (cucb)->pb_compound = (value)  
#define  JobInfo_Date_Get(cucb)\
   (cucb)->date
#define  JobInfo_DateLbl_Get(cucb)\
   (cucb)->lbl_date
#define  JobInfo_DateLbl_Put(cucb, value)\
   (cucb)->lbl_date = (value)
#define  JobInfo_DateTxt_Get(cucb)\
   (cucb)->txt_date
#define  JobInfo_DateTxt_Put(cucb, value)\
   (cucb)->txt_date = (value)
#define  JobInfo_DrawPB_Get(cucb)\
   (cucb)->pb_draw
#define  JobInfo_DrawPB_Put(cucb, value)\
   (cucb)->pb_draw = (value)
#define  JobInfo_Form_Get(cucb)\
   (cucb)->form
#define  JobInfo_Form_Put(cucb, value)\
   (cucb)->form = (value)
#define  JobInfo_Frame_Get(cucb)\
   (cucb)->frame
#define  JobInfo_Frame_Put(cucb, value)\
   (cucb)->frame = (value)
#define  JobInfo_ResetPB_Get(cucb)\
   (cucb)->pb_reset
#define  JobInfo_ResetPB_Put(cucb, value)\
   (cucb)->pb_reset = (value)
#define  JobInfo_RunId_Get(cucb)\
   (cucb)->runid
#define  JobInfo_RunIdLbl_Get(cucb)\
   (cucb)->lbl_runid
#define  JobInfo_RunIdLbl_Put(cucb, value)\
   (cucb)->lbl_runid =  (value)
#define  JobInfo_RunIdTxt_Get(cucb)\
   (cucb)->txt_runid
#define  JobInfo_RunIdTxt_Put(cucb, value)\
   (cucb)->txt_runid = (value)
#define  JobInfo_Sling_Get(cucb)\
   (cucb)->sling
#define  JobInfo_SlingLbl_Get(cucb)\
   (cucb)->lbl_sling
#define  JobInfo_SlingLbl_Put(cucb, value)\
   (cucb)->lbl_sling = (value)
#define  JobInfo_SlingTxt_Get(cucb)\
   (cucb)->txt_sling 
#define  JobInfo_SlingTxt_Put(cucb, value)\
   (cucb)->txt_sling = (value)
#define  JobInfo_Templates_Get(cucb)\
   &(cucb)->templates
#define  JobInfo_User_Get(cucb)\
   (cucb)->user
#define  JobInfo_UserLbl_Get(cucb)\
   (cucb)->lbl_user
#define  JobInfo_UserLbl_Put(cucb, value)\
   (cucb)->lbl_user = (value)
#define  JobInfo_UserTxt_Get(cucb)\
   (cucb)->txt_user
#define  JobInfo_UserTxt_Put(cucb, value)\
   (cucb)->txt_user = (value)


/* Function Prototypes */

Widget JobInfo_Create           (JobInfoCB_t *, Widget); 
void   JobInfo_SlingTxt_Update  (JobInfoCB_t *, Dsp_Molecule_t *);
void   JobInfo_Values_Get       (JobInfoCB_t *, Rcb_t *);
void   JobInfo_Values_Set       (JobInfoCB_t *, Rcb_t *);

/* Callback Prototypes */  
void   JobInfo_Compound_CB      (Widget, XtPointer, XtPointer); 
void   JobInfo_Draw_CB          (Widget, XtPointer, XtPointer);
void   JobInfo_Reset_CB         (Widget, XtPointer, XtPointer);

#endif
 
/*** End Of SUBMIT_JOBINFO.H ***/

