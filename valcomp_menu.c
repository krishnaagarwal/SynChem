
/****************************************************************************
*  
* Copyright (C) 1999 Synchem Group at SUNY-Stony Brook, Jerry Miller
*  
*  
*  Module Name:               VALCOMP_MENU.C
*  
*    Creates and manages the menu bar of the job submission tool.  
*      
*  Creation Date:  
*  
*     30-Oct=1999  
*  
*  Authors: 
*      Jerry Miller based on version by Daren Krebsbach 
*  
*  Routines:
*
*    ValCompMenu_Create
*    ValCompMenu_Destroy
*
*    ValCompMenu_FSbmt_CB
*    ValCompMenu_FStat_CB
*    ValCompMenu_Help_CB
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

#ifndef _H_EXTERN_
#include "extern.h"
#endif

/* Callback Routine Prototypes */  
  
static void   DirsForm_Cancel_CB    (Widget, XtPointer, XtPointer);
static void   DirsForm_Reset_CB     (Widget, XtPointer, XtPointer);
static void   FilesForm_Cancel_CB   (Widget, XtPointer, XtPointer);
static void   FilesForm_Reset_CB    (Widget, XtPointer, XtPointer);
static void   StratsForm_Cancel_CB  (Widget, XtPointer, XtPointer);
static void   StratsForm_Reset_CB   (Widget, XtPointer, XtPointer);
static void   ValCompMenu_FSave_CB   (Widget, XtPointer, XtPointer);  
static void   ValCompMenu_FSbmt_CB   (Widget, XtPointer, XtPointer);  
static void   ValCompMenu_FStat_CB   (Widget, XtPointer, XtPointer);  
static void   ValCompMenu_FTemp_CB   (Widget, XtPointer, XtPointer);

static SubmitMenuCB_t  SSubMenuCB;

/****************************************************************************
*  
*  Function Name:                 ValCompMenu_Create
*  
*****************************************************************************/  

Widget ValCompMenu_Create  
  (
  SubmitCB_t      *submitcb_p,
  Widget           topform 
  )  
{  
  XmString         lbl_str;  
      
  /* Create the main menubar */  
  SubmitMenu_MenuBar_Put (&SSubMenuCB,
    XmCreateMenuBar (topform, "ValCompMenu", NULL, 0));  

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
    "ValCompMenuPDM", NULL, 0));  
  
  lbl_str = XmStringCreateLocalized (SMU_FILE_CPB_LBL);  
  SubmitMenu_FileCPB_Put (&SSubMenuCB,
    XtVaCreateManagedWidget ("ValCompMenuCB",
    xmCascadeButtonWidgetClass, SubmitMenu_MenuBar_Get (&SSubMenuCB),   
    XmNlabelString, lbl_str,  
    XmNsubMenuId, SubmitMenu_FilePDM_Get (&SSubMenuCB),  
    NULL));  
  XmStringFree (lbl_str);  
  
  /*  The Open - submit push button.  */  
  lbl_str = XmStringCreateLocalized (SMU_F_OPEN_SBMT_PB_LBL);  
  SubmitMenu_FSbmtPB_Put (&SSubMenuCB, 
    XtVaCreateManagedWidget ("ValCompMenuPB",   
    xmPushButtonWidgetClass, SubmitMenu_FilePDM_Get (&SSubMenuCB),   
    XmNlabelType, XmSTRING,  
    XmNlabelString, lbl_str,   
    NULL));  
  XmStringFree (lbl_str);  
  XtAddCallback (SubmitMenu_FSbmtPB_Get (&SSubMenuCB), XmNactivateCallback,   
    ValCompMenu_FSbmt_CB, (XtPointer) submitcb_p);    
    
  /* The Open - status push button */  
  lbl_str = XmStringCreateLocalized (SMU_F_OPEN_STAT_PB_LBL);  
  SubmitMenu_FStatPB_Put (&SSubMenuCB, 
    XtVaCreateManagedWidget ("ValCompMenuPB",   
    xmPushButtonWidgetClass, SubmitMenu_FilePDM_Get (&SSubMenuCB),   
    XmNlabelType, XmSTRING,  
    XmNlabelString, lbl_str,   
    NULL));  
  XmStringFree (lbl_str);  
  XtAddCallback (SubmitMenu_FStatPB_Get (&SSubMenuCB), XmNactivateCallback,   
    ValCompMenu_FStat_CB, (XtPointer) submitcb_p);  

  lbl_str = XmStringCreateLocalized ("Save - draw_sling_template ...");
  SubmitMenu_FSavePB_Put (&SSubMenuCB,
    XtVaCreateManagedWidget ("ValCompMenuCB",
    xmPushButtonWidgetClass, SubmitMenu_FilePDM_Get (&SSubMenuCB),   
    XmNlabelType, XmSTRING,  
    XmNlabelString, lbl_str,  
    NULL));  
  XmStringFree (lbl_str);  
  XtAddCallback (SubmitMenu_FSavePB_Get (&SSubMenuCB), XmNactivateCallback,   
    ValCompMenu_FSave_CB, (XtPointer) submitcb_p);  
  
  /*----------------------------------------------------------------------*/  
  /*  The Help Menu (cascade button and pulldown menu).                   */  
  /*----------------------------------------------------------------------*/  
  SubmitMenu_HelpPDM_Put (&SSubMenuCB, 
    XmCreatePulldownMenu (SubmitMenu_MenuBar_Get (&SSubMenuCB),   
    "HelpMenuPDM", NULL, 0));  
  
  lbl_str = XmStringCreateLocalized (/* SMU_HELP_CPB_LBL*/ "");  /* Does nothing and confuses the issue of the real help pb */
  SubmitMenu_HelpCPB_Put (&SSubMenuCB, 
    XtVaCreateManagedWidget ("ValCompMenuCB",   
    xmCascadeButtonWidgetClass, SubmitMenu_MenuBar_Get (&SSubMenuCB),   
    XmNlabelString, lbl_str,  
    XmNsubMenuId, SubmitMenu_HelpPDM_Get (&SSubMenuCB),  
    NULL));   
    
  /* Help Push Button */  
  lbl_str = XmStringCreateLocalized (SMU_HELP_PB_LBL);  
  SubmitMenu_HelpPB_Put (&SSubMenuCB, 
    XtVaCreateManagedWidget ("ValCompMenuPB",   
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

  return (SubmitMenu_MenuBar_Get (&SSubMenuCB));  
}  
/*  End of ValCompMenu_Create  */
  
/****************************************************************************
*  
*  Function Name:                 ValCompMenu_Destroy
*  
*****************************************************************************/  

void ValCompMenu_Destroy 
  (
  void
  )
{
  return;
}  
/*  End of ValCompMenu_Destroy  */

/****************************************************************************
*  
*  Function Name:                 ValCompMenu_FSbmt_CB
*  
*****************************************************************************/  

void ValCompMenu_FSbmt_CB  
  (  
  Widget    w,   
  XtPointer client_data,   
  XtPointer call_data    
  )  
{  
  SVF_FileDg_Update (SVF_FILED_TYPE_OPENSBMT, client_data, FALSE);

  return;  
}  
/*  End of ValCompMenu_FSbmt_CB  */

/****************************************************************************
*  
*  Function Name:                 ValCompMenu_FStat_CB
*  
*****************************************************************************/  

void ValCompMenu_FStat_CB  
  (  
  Widget    w,   
  XtPointer client_data,   
  XtPointer call_data    
  )  
{  
   
  SVF_FileDg_Update (SVF_FILED_TYPE_PEEKSTAT, client_data, FALSE);

  return;  
}  

/****************************************************************************
*  
*  Function Name:                 ValCompMenu_FSave_CB
*  
*****************************************************************************/  

void ValCompMenu_FSave_CB  
  (  
  Widget    w,   
  XtPointer client_data,   
  XtPointer call_data    
  )  
{  
  SubmitCB_t *sbmtcb_p;   
  char       *user,
             *date,
             *comment,
             *runid,
             *sling,
             *str_p,
             *nl,
              filename[256];
  FILE       *f_p;

  sbmtcb_p = (SubmitCB_t *) client_data;

  str_p = XmTextGetString (JobInfo_UserTxt_Get (Submit_JobInfoCB_Get (sbmtcb_p)));
  if ((nl=strstr(str_p,"\n"))!=NULL) *nl='\0';
  user = JobInfo_User_Get (Submit_JobInfoCB_Get (sbmtcb_p));
  strcpy (user, str_p);
  XtFree (str_p);

  str_p = XmTextGetString (JobInfo_DateTxt_Get (Submit_JobInfoCB_Get (sbmtcb_p)));
  if ((nl=strstr(str_p,"\n"))!=NULL) *nl='\0';
  date = JobInfo_Date_Get (Submit_JobInfoCB_Get (sbmtcb_p));
  strcpy (date, str_p);
  XtFree (str_p);

  str_p = XmTextGetString (JobInfo_CommentTxt_Get (Submit_JobInfoCB_Get (sbmtcb_p)));
  if ((nl=strstr(str_p,"\n"))!=NULL) *nl='\0';
  comment = JobInfo_Comment_Get (Submit_JobInfoCB_Get (sbmtcb_p));
  strcpy (comment, str_p);
  XtFree (str_p);

  str_p = XmTextGetString (JobInfo_RunIdTxt_Get (Submit_JobInfoCB_Get (sbmtcb_p)));
  if ((nl=strstr(str_p,"\n"))!=NULL) *nl='\0';
  runid = JobInfo_RunId_Get (Submit_JobInfoCB_Get (sbmtcb_p));
  strcpy (runid, str_p);
  XtFree (str_p);

  str_p = XmTextGetString (JobInfo_SlingTxt_Get (Submit_JobInfoCB_Get (sbmtcb_p)));
  if ((nl=strstr(str_p,"\n"))!=NULL) *nl='\0';
  sling = JobInfo_Sling_Get (Submit_JobInfoCB_Get (sbmtcb_p));
  strcpy (sling, str_p);
  XtFree (str_p);

  sprintf (filename, "%s%s/%s.txt", FCB_DISDIR_USR (""), "/draw_sling_template", runid);

#ifdef _WIN32
    f_p = fopen (gccfix (filename), "w");
#else
    f_p = fopen (filename, "w");
#endif

  fprintf (f_p, "%s\n%s\n%s\n%s\n%s\n", user,date,comment,runid,sling);

  fclose (f_p);

  InfoMess_Show ("draw_sling_template file saved");

  return;  
}  
/*  End of ValCompMenu_FSave_CB  */
