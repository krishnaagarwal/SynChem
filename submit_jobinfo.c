/****************************************************************************
*  
* Copyright (C) 1997 Synchem Group at SUNY-Stony Brook, Daren Krebsbach  
*  
*  
*  Module Name:               SUBMIT_JOBINFO.C
*  
*    Creates and updates the compound and user information panel of 
*    the job submission module.  
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
*    JobInfo_Create
*    JobInfo_SlingTxt_Update
*    JobInfo_Values_Get
*    JobInfo_Values_Set
*
*    JobInfo_Compound_CB
*    JobInfo_Draw_CB
*    JobInfo_Reset_CB
*

*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
*
*******************************************************************************/  
#include <string.h>
#include <Xm/PushB.h> 
#include <Xm/RowColumn.h> 
#include <Xm/Text.h> 
#include <Xm/Label.h> 
#include <Xm/LabelG.h> 
#include <Xm/Frame.h> 
#include <Xm/Form.h>
#include <Xm/ScrolledW.h>
#include <Xm/Separator.h>
#include <Xm/List.h>
#include <Xm/TextF.h>

#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif

#include "synchem.h"
#include "debug.h"
#include "synio.h"
 
#ifndef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_RCB_
#include "rcb.h"
#endif

#ifndef _H_SLING2XTR_
#include "sling2xtr.h"
#endif

#ifndef _H_APP_RESRC_
#include "app_resrc.h"
#endif

#ifndef _H_SUBMIT_DRAW_
#include "submit_draw.h"
#endif

#ifndef _H_INFO_VIEW_
#include "info_view.h"
#endif
 
#ifndef _H_SUBMIT_TEMPLATES_
#include "submit_templates.h"
#endif

#ifndef _H_SUBMIT_JOBINFO_
#include "submit_jobinfo.h" 
#endif

#ifndef _H_SUBMIT_
#include "submit.h" 
#endif

/****************************************************************************
*  
*  Function Name:                 JobInfo_Create
*  
*****************************************************************************/  

Widget JobInfo_Create
  (
  JobInfoCB_t   *jobinfo_p,
  Widget         parent
  ) 
{ 
  Widget         form, sep, title;
  XmString       lbl_str;

  JobInfo_Frame_Put (jobinfo_p, XtVaCreateWidget ("JobInfoFr",
    xmFrameWidgetClass, parent,
    XmNshadowType,      XmSHADOW_IN,
    XmNshadowThickness, 3,
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    NULL));

  form = XtVaCreateWidget ("JobInfoFm", 
    xmFormWidgetClass, JobInfo_Frame_Get (jobinfo_p), 
    XmNmarginHeight,   AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,    AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,      False,
    NULL);
  JobInfo_Form_Put (jobinfo_p, form); 

  lbl_str = XmStringCreateLocalized (SJI_JOBINFO_TITLE);  
  title = XtVaCreateManagedWidget ("JobInfoLbl",
    xmLabelGadgetClass,        JobInfo_Frame_Get (jobinfo_p),
    XmNchildType,              XmFRAME_TITLE_CHILD,
    XmNchildVerticalAlignment, XmALIGNMENT_WIDGET_TOP,
    XmNchildHorizontalSpacing, XmALIGNMENT_CENTER, 
    XmNlabelType,              XmSTRING,  
    XmNlabelString,            lbl_str,
    NULL);
  XmStringFree (lbl_str);  
    
  lbl_str = XmStringCreateLocalized (SJI_USER_LBL);
  JobInfo_UserLbl_Put (jobinfo_p, 
    XtVaCreateManagedWidget ("JobInfoLbl",  
      xmLabelWidgetClass, form, 
      XmNlabelType,       XmSTRING,  
      XmNlabelString,     lbl_str,
      XmNalignment,       XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,   False,
      XmNmarginWidth,     AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  JobInfo_User_Get (jobinfo_p)[0] = '\0';
  JobInfo_UserTxt_Put (jobinfo_p, 
    XtVaCreateManagedWidget ("JobInfoTxt", 
      xmTextWidgetClass, form, 
      XmNeditable,       FALSE, 
      XmNeditMode,       XmSINGLE_LINE_EDIT,
      XmNmaxLength,      SJI_USERBUF_MAXLEN,
      XmNcolumns,        SJI_USERTXT_MAXLEN, 
      XmNvalue,          JobInfo_User_Get (jobinfo_p),
      XmNmarginHeight,   AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,    AppDim_MargLblPB_Get (&GAppDim),
      XmNheight,         AppDim_HtTxt_Get (&GAppDim),
XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
/* to make color correct WITHOUT doing whatever syn_view does to hide the process??? */
      NULL)); 

  lbl_str = XmStringCreateLocalized (SJI_DATE_LBL);
  JobInfo_DateLbl_Put (jobinfo_p, 
    XtVaCreateManagedWidget ("JobInfoLbl",  
      xmLabelWidgetClass, form, 
      XmNlabelType,       XmSTRING,  
      XmNlabelString,     lbl_str,
      XmNalignment,       XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,   False,
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  JobInfo_Date_Get (jobinfo_p)[0] = '\0';
  JobInfo_DateTxt_Put (jobinfo_p, 
    XtVaCreateManagedWidget("JobInfoTxt", 
      xmTextWidgetClass, form,
      XmNeditable,       FALSE,  
      XmNeditMode,       XmSINGLE_LINE_EDIT,
      XmNmaxLength,      SJI_DATEBUF_MAXLEN,
      XmNcolumns,        SJI_DATETXT_MAXLEN, 
      XmNvalue,          JobInfo_Date_Get (jobinfo_p),
      XmNmarginHeight,   AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,    AppDim_MargLblPB_Get (&GAppDim),
      XmNheight,         AppDim_HtTxt_Get (&GAppDim),
XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
/* to make color correct WITHOUT doing whatever syn_view does to hide the process??? */
      NULL)); 

  lbl_str = XmStringCreateLocalized (SJI_COMMENT_LBL);
  JobInfo_CommentLbl_Put (jobinfo_p, 
    XtVaCreateManagedWidget("JobInfoLbl",  
      xmLabelWidgetClass, form, 
      XmNlabelType,       XmSTRING,  
      XmNlabelString,     lbl_str,
      XmNalignment,       XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,   False,
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  JobInfo_Comment_Get (jobinfo_p)[0] = '\0';
  JobInfo_CommentTxt_Put (jobinfo_p, 
    XtVaCreateManagedWidget("JobInfoTxt", 
      xmTextWidgetClass, form,  
      XmNeditMode,       XmSINGLE_LINE_EDIT,
      XmNmaxLength,      SJI_COMNTBUF_MAXLEN,
      XmNcolumns,        SJI_COMNTTXT_MAXLEN, 
      XmNvalue,          JobInfo_Comment_Get (jobinfo_p),
      XmNmarginHeight,   AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,    AppDim_MargLblPB_Get (&GAppDim),
      XmNheight,         AppDim_HtTxt_Get (&GAppDim),
XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
/* to make color correct WITHOUT doing whatever syn_view does to hide the process??? */
      NULL));

  lbl_str = XmStringCreateLocalized (SJI_RUNID_LBL);
  JobInfo_RunIdLbl_Put (jobinfo_p, 
    XtVaCreateManagedWidget("JobInfoLbl",  
      xmLabelWidgetClass, form, 
      XmNlabelType,       XmSTRING,  
      XmNlabelString,     lbl_str,
      XmNalignment,       XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,   False,
      XmNmarginWidth,      AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  JobInfo_RunId_Get (jobinfo_p)[0] = '\0';
  JobInfo_RunIdTxt_Put (jobinfo_p, 
    XtVaCreateManagedWidget ("JobInfoTxt", 
      xmTextWidgetClass, form,  
      XmNeditMode,       XmSINGLE_LINE_EDIT,
      XmNmaxLength,      SJI_RUNIDBUF_MAXLEN,
      XmNcolumns,        SJI_RUNIDTXT_MAXLEN, 
      XmNvalue,          JobInfo_RunId_Get (jobinfo_p),
      XmNmarginHeight,   AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,    AppDim_MargLblPB_Get (&GAppDim),
      XmNheight,         AppDim_HtTxt_Get (&GAppDim),
XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
/* to make color correct WITHOUT doing whatever syn_view does to hide the process??? */
      NULL));

  lbl_str = XmStringCreateLocalized (SJI_SLING_LBL);
  JobInfo_SlingLbl_Put (jobinfo_p, 
    XtVaCreateManagedWidget("JobInfoLbl",  
      xmLabelWidgetClass, form, 
      XmNlabelType,       XmSTRING,  
      XmNlabelString,     lbl_str,
      XmNalignment,       XmALIGNMENT_BEGINNING,
      XmNrecomputeSize,   False,
      XmNmarginWidth,     AppDim_MargLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);

  JobInfo_Sling_Get (jobinfo_p)[0] = '\0';
  JobInfo_SlingTxt_Put (jobinfo_p, 
    XtVaCreateManagedWidget ("JobInfoTxt",
      xmTextWidgetClass, form, 
      XmNeditMode,       XmSINGLE_LINE_EDIT,
      XmNmaxLength,      SJI_SLINGBUF_MAXLEN,
      XmNcolumns,        SJI_SLINGTXT_MAXLEN, 
      XmNvalue,          JobInfo_Sling_Get (jobinfo_p),
      XmNmarginHeight,   AppDim_MargLblPB_Get (&GAppDim),
      XmNmarginWidth,    AppDim_MargLblPB_Get (&GAppDim),
      XmNheight,         AppDim_HtTxt_Get (&GAppDim),
XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
/* to make color correct WITHOUT doing whatever syn_view does to hide the process??? */
      NULL)); 

  sep = XtVaCreateManagedWidget("JobInfoSep", 
    xmSeparatorWidgetClass, form, 
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmDOUBLE_LINE, 
    XmNheight,              AppDim_SepSmall_Get (&GAppDim),
    NULL);

  lbl_str = XmStringCreateLocalized (SJI_DRAW_PB_LBL);
  JobInfo_DrawPB_Put (jobinfo_p, 
    XtVaCreateManagedWidget("JobInfoPB",  
      xmPushButtonWidgetClass, form, 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,
      XmNrecomputeSize,        False,
      XmNheight,               AppDim_HtLblPB_Get (&GAppDim),
      NULL));
  XmStringFree (lbl_str);
  XtAddCallback (JobInfo_DrawPB_Get (jobinfo_p),
     XmNactivateCallback, JobInfo_Draw_CB, (XtPointer) jobinfo_p);

  lbl_str = XmStringCreateLocalized (SJI_RESET_PB_LBL);
  JobInfo_ResetPB_Put (jobinfo_p, 
    XtVaCreateManagedWidget("JobInfoPB",  
      xmPushButtonWidgetClass, form, 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,
      XmNrecomputeSize,        False,
      NULL));
  XmStringFree (lbl_str);
  XtAddCallback (JobInfo_ResetPB_Get (jobinfo_p),
     XmNactivateCallback, JobInfo_Reset_CB, (XtPointer) jobinfo_p);

  lbl_str = XmStringCreateLocalized (SJI_COMP_PB_LBL);
  JobInfo_CompsPB_Put (jobinfo_p, 
    XtVaCreateManagedWidget("JobInfoPB",  
      xmPushButtonWidgetClass, form, 
      XmNlabelType,            XmSTRING,  
      XmNlabelString,          lbl_str,
      XmNrecomputeSize,        False,
      NULL));
  XmStringFree (lbl_str);
  XtAddCallback (JobInfo_CompsPB_Get (jobinfo_p),
     XmNactivateCallback, JobInfo_Compound_CB, (XtPointer) jobinfo_p);

  XtVaSetValues (JobInfo_DateTxt_Get (jobinfo_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_NONE,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (JobInfo_DateLbl_Get (jobinfo_p),
    XmNtopOffset,        AppDim_MargLblPB_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        JobInfo_DateTxt_Get (jobinfo_p),
    XmNbottomOffset,     AppDim_MargLblPB_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     JobInfo_DateTxt_Get (jobinfo_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_NONE,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      JobInfo_DateTxt_Get (jobinfo_p),
    NULL);

  XtVaSetValues (JobInfo_UserLbl_Get (jobinfo_p),
    XmNtopOffset,        AppDim_MargLblPB_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        JobInfo_DateTxt_Get (jobinfo_p),
    XmNbottomOffset,     AppDim_MargLblPB_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     JobInfo_DateTxt_Get (jobinfo_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_NONE,
    NULL);

  XtVaSetValues (JobInfo_UserTxt_Get (jobinfo_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        JobInfo_DateTxt_Get (jobinfo_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     JobInfo_DateTxt_Get (jobinfo_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       JobInfo_UserLbl_Get (jobinfo_p),
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_NONE,
    NULL);

  XtVaSetValues (JobInfo_RunIdTxt_Get (jobinfo_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        JobInfo_DateTxt_Get (jobinfo_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_NONE,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (JobInfo_RunIdLbl_Get (jobinfo_p),
    XmNtopOffset,        AppDim_MargLblPB_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        JobInfo_RunIdTxt_Get (jobinfo_p),
    XmNbottomOffset,     AppDim_MargLblPB_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     JobInfo_RunIdTxt_Get (jobinfo_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      JobInfo_RunIdTxt_Get (jobinfo_p),
    NULL);

  XtVaSetValues (JobInfo_CommentTxt_Get (jobinfo_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        JobInfo_RunIdTxt_Get (jobinfo_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_NONE,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

/*  XtAddCallback (SelTrace_CmpText_Get (selt_p), XmNactivateCallback,
    JobInfo_Directory_Validate, (XtPointer) jobinfo_p);
*/

  XtVaSetValues (JobInfo_CommentLbl_Get (jobinfo_p),
    XmNtopOffset,        AppDim_MargLblPB_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        JobInfo_CommentTxt_Get (jobinfo_p),
    XmNbottomOffset,     AppDim_MargLblPB_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     JobInfo_CommentTxt_Get (jobinfo_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      JobInfo_CommentTxt_Get (jobinfo_p),
    NULL);

  XtVaSetValues (JobInfo_SlingTxt_Get (jobinfo_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        JobInfo_CommentTxt_Get (jobinfo_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_NONE,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (JobInfo_SlingLbl_Get (jobinfo_p),
    XmNtopOffset,        AppDim_MargLblPB_Get (&GAppDim),
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        JobInfo_SlingTxt_Get (jobinfo_p),
    XmNbottomOffset,     AppDim_MargLblPB_Get (&GAppDim),
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     JobInfo_SlingTxt_Get (jobinfo_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      JobInfo_SlingTxt_Get (jobinfo_p),
    NULL);

  XtVaSetValues (sep,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        JobInfo_SlingTxt_Get (jobinfo_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (JobInfo_DrawPB_Get (jobinfo_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        sep,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     10,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    30,
    NULL);

  XtVaSetValues (JobInfo_CompsPB_Get (jobinfo_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        JobInfo_DrawPB_Get (jobinfo_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     JobInfo_DrawPB_Get (jobinfo_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     40,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    60,
    NULL);

  XtVaSetValues (JobInfo_ResetPB_Get (jobinfo_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        JobInfo_DrawPB_Get (jobinfo_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     JobInfo_DrawPB_Get (jobinfo_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     70,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    90,
    NULL);


  XtManageChild (JobInfo_Form_Get (jobinfo_p));
  TemplateCB_Created_Put (JobInfo_Templates_Get (jobinfo_p), FALSE);
  TemplateCB_ListLoaded_Put (JobInfo_Templates_Get (jobinfo_p), FALSE);
  TemplateCB_ListModified_Put (JobInfo_Templates_Get (jobinfo_p), FALSE);

  XtSetSensitive (JobInfo_CompsPB_Get (jobinfo_p), False);

  return (JobInfo_Frame_Get (jobinfo_p)); 
}  
/* End of JobInfo_Create */

/****************************************************************************
*  
*  Function Name:                 JobInfo_SlingTxt_Update
*  
*****************************************************************************/  

void JobInfo_SlingTxt_Update 
  ( 
  JobInfoCB_t    *jobinfo_p, 
  Dsp_Molecule_t *mol_p
  )
{
  Tsd_t    *tsd_p; 
  Sling_t   sling;  
  Sling_t   canon_sling;  

  if (mol_p == NULL)
    return; 
    
  tsd_p = Dsp2Tsd (mol_p);
  sling = Tsd2Sling (tsd_p);
  canon_sling = Sling2CanonSling (sling);

  strncpy (JobInfo_Sling_Get (jobinfo_p), 
    (char *) Sling_Name_Get (canon_sling), SJI_SLINGBUF_MAXLEN);
  JobInfo_Sling_Get (jobinfo_p)[SJI_SLINGBUF_MAXLEN] = '\0';

  XmTextSetString (JobInfo_SlingTxt_Get (jobinfo_p), 
    JobInfo_Sling_Get (jobinfo_p));

  Sling_Destroy (sling);
  Sling_Destroy (canon_sling);
  Tsd_Destroy (tsd_p);

  return;
}
/* End of JobInfo_SlingTxt_Update */

/****************************************************************************
*  
*  Function Name:                 JobInfo_Values_Get
*  
*****************************************************************************/  

void JobInfo_Values_Get
  (
   JobInfoCB_t  *jobinfo_p,
   Rcb_t        *rcb_p
  )
{
  char         *str_p; 
  String_t      sling;
   
  /** Get value for the user name **/
  /*{ USER NAME IS NOT MODIFIALBLE BY USER }*/

  /** Get value for date **/
  /*{ Date Is NOT MODIFIALBLE BY USER }*/

  /** Get value for comment **/
  str_p = XmTextGetString (JobInfo_CommentTxt_Get (jobinfo_p));
  String_Destroy (Rcb_Comment_Get (rcb_p));
  Rcb_Comment_Put (rcb_p, String_Create (str_p, 0));
  XtFree (str_p);

  /** Get value for runid **/
  str_p = XmTextGetString (JobInfo_RunIdTxt_Get (jobinfo_p));
  String_Destroy (Rcb_Name_Get (rcb_p));
  Rcb_Name_Put (rcb_p, String_Create (str_p, 0));
  XtFree (str_p);

  /** Get value for target compound (sling) **/
  str_p = XmTextGetString (JobInfo_SlingTxt_Get (jobinfo_p));
  Sling_Destroy (Rcb_Goal_Get (rcb_p));
  sling = String_Create (str_p, 0);
  Rcb_Goal_Put (rcb_p, String2Sling (sling));
  XtFree (str_p);
  String_Destroy (sling);

  return;
} 
/* End of JobInfo_Values_Get */
 
/****************************************************************************
*  
*  Function Name:                 JobInfo_Values_Set
*  
*****************************************************************************/  

void JobInfo_Values_Set
  (
   JobInfoCB_t  *jobinfo_p,
   Rcb_t        *rcb_p
  )
{
  Sling_t    canon_sling;
 
  /** Set value for the user name.  **/
  strncpy (JobInfo_User_Get (jobinfo_p), 
    (char *) String_Value_Get (Rcb_User_Get (rcb_p)), SJI_USERBUF_MAXLEN);
  JobInfo_User_Get (jobinfo_p)[SJI_USERBUF_MAXLEN] = '\0';
  XmTextSetString (JobInfo_UserTxt_Get (jobinfo_p), 
    JobInfo_User_Get (jobinfo_p));

  /** Set value for date.  **/
  sprintf (JobInfo_Date_Get (jobinfo_p), "%s", 
    ctime (&((Rcb_Date_Get (rcb_p)).tv_sec)));
  XmTextSetString (JobInfo_DateTxt_Get (jobinfo_p), 
    JobInfo_Date_Get (jobinfo_p));

  /** Set value for comment.  **/
  strncpy (JobInfo_Comment_Get (jobinfo_p), 
    (char *) String_Value_Get (Rcb_Comment_Get (rcb_p)), SJI_COMNTBUF_MAXLEN);
  JobInfo_Comment_Get (jobinfo_p)[SJI_COMNTBUF_MAXLEN] = '\0';
  XmTextSetString (JobInfo_CommentTxt_Get (jobinfo_p), 
    JobInfo_Comment_Get (jobinfo_p));

  /** Set value for runid.  **/
  strncpy (JobInfo_RunId_Get (jobinfo_p), 
    (char *) String_Value_Get (Rcb_Name_Get (rcb_p)), SJI_RUNIDBUF_MAXLEN);
  JobInfo_RunId_Get (jobinfo_p)[SJI_RUNIDBUF_MAXLEN] = '\0';
  XmTextSetString (JobInfo_RunIdTxt_Get (jobinfo_p), 
    JobInfo_RunId_Get (jobinfo_p));

  /** Set value for target compound (sling).  **/
  canon_sling = Sling2CanonSling (Rcb_Goal_Get (rcb_p));
  strncpy (JobInfo_Sling_Get (jobinfo_p), 
    (char *) Sling_Name_Get (canon_sling), SJI_SLINGBUF_MAXLEN);
  JobInfo_Sling_Get (jobinfo_p)[SJI_SLINGBUF_MAXLEN] = '\0';
  XmTextSetString (JobInfo_SlingTxt_Get (jobinfo_p), 
    JobInfo_Sling_Get (jobinfo_p));

  Sling_Destroy (canon_sling);

  return;
}  
/* End of JobInfo_Values_Set */

/****************************************************************************
*  
*  Function Name:                 JobInfo_Compound_CB
*  
*****************************************************************************/  

void JobInfo_Compound_CB 
  (  
  Widget      button, 
  XtPointer   client_data, 
  XtPointer   call_data 
  ) 
{ 
  JobInfoCB_t      *jobinfo_p;
  TemplateCB_t     *templates_p;


  jobinfo_p = (JobInfoCB_t *) client_data;
  templates_p = JobInfo_Templates_Get (jobinfo_p);

  if (!TemplateCB_Created_Get (templates_p))
    Templates_Create (templates_p, JobInfo_Form_Get (jobinfo_p));

  if (XtIsManaged (TemplateCB_FormDg_Get (templates_p)))
    return;

  if (!TemplateCB_ListLoaded_Get (templates_p))
    {
    if (!TemplatesList_Load (templates_p, (char *) String_Value_Get (
        FileCB_DirStr_Get (Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_TMPL)))))
      return;
    }

  XtManageChild (TemplateCB_FormDg_Get (templates_p));
  return;
}  
/* End of JobInfo_Compound_CB */

/****************************************************************************
*  
*  Function Name:                 JobInfo_Draw_CB
*  
*****************************************************************************/  

void JobInfo_Draw_CB 
  (  
  Widget          button, 
  XtPointer       client_data, 
  XtPointer       call_data 
  ) 
{ 
  String_t      temp_str;
  Sling_t       canon_sling;  
  Sling_t       temp_sling;  
  char         *str_p; 
  JobInfoCB_t  *jobinfo_p;
  U16_t         num_atoms;

  jobinfo_p = (JobInfoCB_t *) client_data;
  
  /* Get the character string from the sling text widget and convert
   * it to a sling.
   */
  str_p = XmTextGetString (JobInfo_SlingTxt_Get (jobinfo_p));
  if (str_p == NULL)
    {
    InfoWarn_Show ("No sling to draw.");
    return;
    }

  if (strlen (str_p) <= 0)
    {
    InfoWarn_Show ("No sling to draw.");
    XtFree (str_p); 
    return;
    }

  temp_str = String_Create (str_p, 0);
  temp_sling = String2Sling (temp_str);

  if (!Sling_Validate (temp_sling, &num_atoms))
    {
    InfoWarn_Show ("Invalid sling.");
    String_Destroy (temp_str);
    Sling_Destroy (temp_sling);
    XtFree (str_p); 
    return;
    }

  canon_sling = Sling2CanonSling (temp_sling);
  XmTextSetString (JobInfo_SlingTxt_Get (jobinfo_p), 
    (char *) Sling_Name_Get (canon_sling));
  
  MolDraw_Sling_Draw (canon_sling);

  XtFree (str_p); 
  String_Destroy (temp_str);
  Sling_Destroy (canon_sling);
  Sling_Destroy (temp_sling);

  return;
} 
/* End of JobInfo_Draw_CB */
  
/****************************************************************************
*  
*  Function Name:                 JobInfo_Reset_CB
*  
*****************************************************************************/  

void JobInfo_Reset_CB 
  (  
  Widget      button, 
  XtPointer   client_data, 
  XtPointer   call_data 
  ) 
{ 
  Rcb_t         temp_rcb;
  JobInfoCB_t  *jobinfo_p;

  jobinfo_p = (JobInfoCB_t *) client_data;
  Rcb_Init (&temp_rcb, FALSE);
  JobInfo_Values_Set (jobinfo_p, &temp_rcb); 
  Rcb_Destroy (&temp_rcb);

  return;
}
/* End of JobInfo_Reset_CB */

/*   End of SUBMIT_JOBINFO.C   */

