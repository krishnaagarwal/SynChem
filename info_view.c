/******************************************************************************
*
*  Copyright (C) 1996-1997, Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     INFO_VIEW.C
*
*    This module defines the routines needed to handle the information
*    windows.  
*
*  Creation Date:
*
*    16-Nov-1996
*
*  Authors:
*
*    Daren Krebsbach
*
*  Routines:
*
*    InfoMess_Create
*    InfoMess_Dismiss_CB
*    InfoMess_Show
*
*    InfoWarn_Create
*    InfoWarn_Dismiss_CB
*    InfoWarn_Show
*
*    InfoWin_Create
*    InfoWin_Dismiss_CB
*
*    InfoWin_Compound_Update
*    InfoWin_Reaction_Update
*    InfoWin_Subgoal_Update
*
*    InfoWin_Status_Load_CB
*    InfoWin_Status_Update
*
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
*
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Xm/Form.h>
#include <Xm/MessageB.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/PushB.h>

#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_PST_
#include "pst.h"
#endif

#ifndef _H_RCB_
#include "rcb.h"
#endif

#ifndef _H_APP_RESRC_
#include "app_resrc.h"
#endif

#ifndef _H_PST_VIEW_
#include "pst_view.h"
#endif

#ifndef _H_SYN_VIEW_
#include "syn_view.h"
#endif

#ifndef _H_INFO_VIEW_
#include "info_view.h"
#endif

#ifndef _CYGHAT_
#ifdef _CYGWIN_
#define _CYGHAT_
#else
#ifdef _REDHAT_
#define _CYGHAT_
#endif
#endif
#endif

/* Static Variables */

static  Widget         SMessDialog;            /* Message dialog widget */
static  Widget         SWarnDialog;            /* Warning dialog widget */
static  InfoWin_t      SCompInfo;              /* Compound info window */
static  InfoWin_t      SSubGInfo;              /* Subgoal info window */
static  InfoWin_t      SStatInfo;              /* Run statistics info window */
static  InfoWin_t      SRxnInfo;               /* Reaction text information */

/* static */ char *Sensible_Time_Format (Time_t, Boolean_t);

/****************************************************************************
*
*  Function Name:                 InfoMess_Create
*
*    This routine creates the warning message dialog.
*
*
*  Implicit Inputs:
*
*    SMessDialog, the information message dialog.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void InfoMess_Create
  (
  Widget      parent
  )
{
  Widget         child;
  XmString       title_str;                 /* Dialog title string */

  SMessDialog = XmCreateInformationDialog (parent, "SynMessMD", NULL, 0);
  child = XmMessageBoxGetChild (SMessDialog, XmDIALOG_CANCEL_BUTTON);
  XtUnmanageChild (child);
  child = XmMessageBoxGetChild (SMessDialog, XmDIALOG_HELP_BUTTON);
  XtUnmanageChild (child);

  title_str = XmStringCreateLtoR ("SynView Message:", 
    XmFONTLIST_DEFAULT_TAG);
  XtVaSetValues (SMessDialog,
    XmNdialogTitle, title_str, 
    XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
    XmNdefaultPosition, True,
XmNy, 25, /* for window managers that are too stupid to put the top border on the screen! */
    NULL);
  XmStringFree (title_str);

  XtAddCallback (SMessDialog, XmNokCallback, 
    InfoMess_Dismiss_CB, (XtPointer) NULL);

  return;
}
/*  End of InfoMess_Create  */

/****************************************************************************
*
*  Function Name:                 InfoMess_Dismiss_CB
*
*    This routine unmanages the warning message dialog.
*
*
*  Implicit Inputs:
*
*    SMessDialog, the information message dialog.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void InfoMess_Dismiss_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  XtUnmanageChild (SMessDialog);

  return ;
}
/*  End of InfoMess_Dismiss_CB  */


/****************************************************************************
*
*  Function Name:                 InfoMess_Show
*
*    This routine displays a message in the warning message dialog.
*
*
*  Implicit Inputs:
*
*    SMessDialog, the information message dialog.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void InfoMess_Show
  (
  char      *mess
  )
{
  char *tilde, title[100], message[500];
  XmString       mess_str;                 /* Message string */
  XmString       title_str;

  if (XtIsManaged (SMessDialog))
    return;

  strcpy (message, mess);
  tilde = strstr (message, "~");
  if (tilde != NULL)
  {
    *tilde++ = 0;
    strcpy (title, message);
    strcpy (message, tilde);
    title_str = XmStringCreateLtoR (title, XmFONTLIST_DEFAULT_TAG);
  }
  else title_str = XmStringCreateLtoR ("SynView Message:", 
    XmFONTLIST_DEFAULT_TAG);
  mess_str = XmStringCreateLtoR (message, XmFONTLIST_DEFAULT_TAG);
  XtVaSetValues (SMessDialog,
    XmNmessageAlignment, XmALIGNMENT_BEGINNING,
    XmNmessageString, mess_str,
    NULL);
  XmStringFree (mess_str);

  XtManageChild (SMessDialog);

  XtVaSetValues (SMessDialog,
    XmNdialogTitle, title_str,
    NULL);
  XmStringFree (title_str);

  return;
}
/*  End of InfoMess_Show  */

/****************************************************************************
*
*  Function Name:                 InfoWarn_Create
*
*    This routine creates the warning message dialog.
*
*
*  Implicit Inputs:
*
*    SWarnDialog, the warning message dialog.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void InfoWarn_Create
  (
  Widget      parent
  )
{
  Widget         child;
  XmString       title_str;                 /* Dialog title string */

  SWarnDialog = XmCreateWarningDialog (parent, "SynWarnMD", NULL, 0);
  child = XmMessageBoxGetChild (SWarnDialog, XmDIALOG_CANCEL_BUTTON);
  XtUnmanageChild (child);
  child = XmMessageBoxGetChild (SWarnDialog, XmDIALOG_HELP_BUTTON);
  XtUnmanageChild (child);

  title_str = XmStringCreateLtoR ("SynView Warning!", 
    XmFONTLIST_DEFAULT_TAG);
  XtVaSetValues (SWarnDialog,
    XmNdialogTitle, title_str, 
    XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
    XmNdefaultPosition, True,
    NULL);
  XmStringFree (title_str);

  XtAddCallback (SWarnDialog, XmNokCallback, 
    InfoWarn_Dismiss_CB, (XtPointer) NULL);

  return;
}
/*  End of InfoWarn_Create  */

/****************************************************************************
*
*  Function Name:                 InfoWarn_Dismiss_CB
*
*    This routine unmanages the warning message dialog.
*
*
*  Implicit Inputs:
*
*    SWarnDialog, the warning message dialog.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void InfoWarn_Dismiss_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  XtUnmanageChild (SWarnDialog);

  return ;
}
/*  End of InfoWarn_Dismiss_CB  */

/****************************************************************************
*
*  Function Name:                 InfoWarn_Show
*
*    This routine displays a message in the warning message dialog.
*
*
*  Implicit Inputs:
*
*    SWarnDialog, the warning message dialog.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void InfoWarn_Show
  (
  char      *mess
  )
{
  char *tilde, title[100], message[500];
  XmString       mess_str;                 /* Message string */
  XmString       title_str;

  if (XtIsManaged (SWarnDialog))
    {
    if (mess == NULL)
      {
      XtUnmanageChild (SWarnDialog);
      XtManageChild (SWarnDialog);
      }
    return;
    }

  if (mess == NULL) return;

  strcpy (message, mess);
  tilde = strstr (message, "~");
  if (tilde != NULL)
  {
    *tilde++ = 0;
    strcpy (title, message);
    strcpy (message, tilde);
    title_str = XmStringCreateLtoR (title, XmFONTLIST_DEFAULT_TAG);
  }
  else title_str = XmStringCreateLtoR ("SynView Warning!", 
    XmFONTLIST_DEFAULT_TAG);
  mess_str = XmStringCreateLtoR (message, XmFONTLIST_DEFAULT_TAG);
  XtVaSetValues (SWarnDialog,
    XmNmessageAlignment, XmALIGNMENT_BEGINNING,
    XmNmessageString, mess_str,
    NULL);
  XmStringFree (mess_str);

  XtManageChild (SWarnDialog);

  XtVaSetValues (SWarnDialog,
    XmNdialogTitle, title_str,
    NULL);
  XmStringFree (title_str);

  return;
}
/*  End of InfoWarn_Show  */

/****************************************************************************
*
*  Function Name:                 InfoWin_Create
*
*    This routine creates the Information Dialog.  The okay and help
*    buttons are unmanaged, and no callbacks are added at this point.
*
*    Because the information window is used for displaying different types 
*    of information, the following protocol should be used:
*      1)  A separate update routine should be written for each type
*          of information to be displayed.  The update routine should
*          change the title of the dialogue and add its own dismiss callback 
*          for the cancel button.  If the okay button is to be used, it
*          should be relabeled, managed, and the its callback routine added.
*      2)  The dismiss callback should remove all callbacks (and unmanage
*          the okay button if it was used) after unmanaging the information
*          window itself.  
*
*    Disclaimer:  Separate information windows are now created, but the same
*    protocol is followed, more or less.
*
*  Implicit Inputs:
*
*    SCompInfo, SSubGInfo, SStatInfo, SRxnInfo:  the SynView info windows.
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
*    None.
*
******************************************************************************/
void InfoWin_Create
  (
  Widget         parent,
  U8_t           type
  )
{
  
  InfoWin_t     *iw_p;
  XmString       dismiss;
  XmString       title;
  Widget         child;
  Widget         infowin;
  Widget         textwin;
  short int      nrows;
#ifdef _CYGHAT_
  Widget         form;
  Widget         box;
  Arg al[50];
  int ac;
#endif

  /* Create the information dialog, and scrollable text window.  */
  infowin = XmCreateMessageDialog (parent, "InfoWinDlg", NULL, 0);
  XtVaSetValues (infowin,
    XmNdialogStyle,         XmDIALOG_MODELESS, 
    XmNbuttonFontList,      SynAppR_FontList_Get (&GSynAppR),
    XmNlabelFontList,       SynAppR_FontList_Get (&GSynAppR), 
    XmNtextFontList,        SynAppR_FontList_Get (&GSynAppR),
    XmNautoUnmanage,        False, 
XmNwidth, 600,
XmNheight, 400,
    NULL);

  child = XmMessageBoxGetChild (infowin, XmDIALOG_OK_BUTTON);
  XtUnmanageChild (child);
  child = XmMessageBoxGetChild (infowin, XmDIALOG_HELP_BUTTON);
  XtUnmanageChild (child);


  if (type == SVI_INFO_STAT)
    {
    iw_p = &SStatInfo;
    title = XmStringCreateLtoR (SVI_INFO_TITLE_STAT, XmFONTLIST_DEFAULT_TAG);
    nrows = SVI_INFO_NUMROWS;
    InfoWin_Type_Put (*iw_p, SVI_INFO_STAT);
    InfoWin_SelectPB_Put (*iw_p, NULL);
    }

  else if (type == SVI_INFO_RXN)
    {
    iw_p = &SRxnInfo;
    title = XmStringCreateLtoR (SVI_INFO_TITLE_RXN, XmFONTLIST_DEFAULT_TAG);
    nrows = SVI_INFO_ROWS_RXN;
    InfoWin_Type_Put (*iw_p, SVI_INFO_RXN);
    InfoWin_SelectPB_Put (*iw_p, NULL);
    }

  else if (type == SVI_INFO_COMPOUND || type == SVI_INFO_SUBGOAL)
    {
    XmString       selpbstr;

    if (type == SVI_INFO_COMPOUND)
      {
      iw_p = &SCompInfo;
      title = XmStringCreateLtoR (SVI_INFO_TITLE_COMP, 
        XmFONTLIST_DEFAULT_TAG);
      selpbstr = XmStringCreateLtoR (SVI_INFO_PB_SELECT_CMP, 
        XmFONTLIST_DEFAULT_TAG);
      nrows = SVI_INFO_ROWS_CMP;
      InfoWin_Type_Put (*iw_p, SVI_INFO_SUBGOAL);
      }

    else
      {
      iw_p = &SSubGInfo;
      title = XmStringCreateLtoR (SVI_INFO_TITLE_SUBG, 
        XmFONTLIST_DEFAULT_TAG);
      selpbstr = XmStringCreateLtoR (SVI_INFO_PB_SELECT_SG, 
        XmFONTLIST_DEFAULT_TAG);
      nrows = SVI_INFO_ROWS_SG;
      InfoWin_Type_Put (*iw_p, SVI_INFO_SUBGOAL);
      }

    InfoWin_SelectPB_Put (*iw_p, XtVaCreateWidget ("InfoWinSelPB", 
      xmPushButtonWidgetClass,  infowin, 
      XmNlabelType, XmSTRING,
      XmNlabelString, selpbstr, 
      XmNshowAsDefault, True,
      NULL));
    XmStringFree (selpbstr);

    if (type == SVI_INFO_COMPOUND)
      XtAddCallback (InfoWin_SelectPB_Get (*iw_p), XmNactivateCallback, 
        InfoWin_Compound_Update, NULL);
    else
      XtAddCallback (InfoWin_SelectPB_Get (*iw_p), XmNactivateCallback, 
        InfoWin_Subgoal_Update, NULL);

    XtManageChild (InfoWin_SelectPB_Get (*iw_p));
    }

  else
    return;

  dismiss = XmStringCreateLtoR (SVI_INFO_PB_DISMISS, XmFONTLIST_DEFAULT_TAG);
  XtVaSetValues (infowin,
    XmNcancelLabelString,   dismiss,
    XmNdialogTitle,         title, 
    NULL);

#ifdef _CYGHAT_
  form = XtCreateManagedWidget ("info_form",
    xmFormWidgetClass, infowin,
    NULL, 0);

  box = XtVaCreateManagedWidget ("info_box",
    xmRowColumnWidgetClass, form,
    XmNleftAttachment,      XmATTACH_FORM,
    XmNrightAttachment,     XmATTACH_FORM,
    XmNbottomAttachment,    XmATTACH_FORM,
    XmNorientation,         XmHORIZONTAL,
    XmNheight,              1,
    NULL);

ac=0;
XtSetArg(al[ac],
    XmNeditable,               False); ac++;
XtSetArg(al[ac],
    XmNeditMode,               XmMULTI_LINE_EDIT); ac++;
XtSetArg(al[ac],
    XmNscrollVertical,         True); ac++;
XtSetArg(al[ac],
    XmNautoShowCursorPosition, False); ac++;
XtSetArg(al[ac],
    XmNcursorPositionVisible,  False); ac++;
XtSetArg(al[ac],
    XmNmarginHeight,           AppDim_MargLblPB_Get (&GAppDim)); ac++;
XtSetArg(al[ac],
    XmNmarginWidth,            AppDim_MargLblPB_Get (&GAppDim)); ac++;
XtSetArg(al[ac],
    XmNcolumns,                SVI_INFO_NUMCOLS); ac++;
XtSetArg(al[ac],
    XmNrows,                   nrows); ac++;
XtSetArg(al[ac],
    XmNvalue,                  InfoWin_TextBuf_Get (*iw_p)); ac++;
  textwin = XmCreateScrolledText (form, "InfoWinTxt", al, ac);
#else
  textwin = XmCreateScrolledText (infowin, "InfoWinTxt", NULL, 0);
  XtVaSetValues (textwin,
    XmNeditable,               False,
    XmNeditMode,               XmMULTI_LINE_EDIT,
    XmNautoShowCursorPosition, False,
    XmNcursorPositionVisible,  False,
    XmNmarginHeight,           AppDim_MargLblPB_Get (&GAppDim),
    XmNmarginWidth,            AppDim_MargLblPB_Get (&GAppDim),
    XmNcolumns,                SVI_INFO_NUMCOLS,
    XmNrows,                   nrows,
    XmNvalue,                  InfoWin_TextBuf_Get (*iw_p), 
    NULL);
#endif
  XmStringFree (dismiss);
  XmStringFree (title);

#ifdef _CYGHAT_
  XtVaSetValues (XtParent (textwin),
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget,     box,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);
#endif

  *InfoWin_TextBuf_Get (*iw_p) = '\0';
  *InfoWin_Filename_Get (*iw_p) = '\0';
  InfoWin_TextWin_Put (*iw_p, textwin);
  InfoWin_InfoDlg_Put (*iw_p, infowin);
  XtManageChild (textwin);

  XtAddCallback (InfoWin_InfoDlg_Get (*iw_p), XmNcancelCallback, 
      InfoWin_Dismiss_CB, iw_p);

  return ;
}
/*  End of InfoWin_Create  */


/****************************************************************************
*
*  Function Name:                 InfoWin_Handle_Get
*
*    Return the handle to the InfoWin data structure.
*
*
*  Implicit Inputs:
*
*    SInfoWin, the SynView information window.
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
*    None.
*
******************************************************************************/
InfoWin_t *InfoWin_Handle_Get
  (
  U8_t     type
  )
{
  if (type == SVI_INFO_STAT || type == SVI_INFO_STAT_LOAD)
    return (&SStatInfo);

  else if (type == SVI_INFO_COMPOUND)
    return (&SCompInfo);

  else if (type == SVI_INFO_SUBGOAL)
    return (&SSubGInfo);

  else if (type == SVI_INFO_RXN)
    return (&SRxnInfo);

  else
    return (NULL);
}
/*  End of InfoWin_Handle_Get  */


/****************************************************************************
*
*  Function Name:                 InfoWin_Dismiss_CB
*
*    This routine unmanages the information window dialog.  It removes the 
*    callbacks, and unmanages the okay button for the status load information
*    window.
*
*
*  Implicit Inputs:
*
*    None.
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
*    None.
*
******************************************************************************/
void InfoWin_Dismiss_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p
  )
{
  InfoWin_t      *infowin_p;

  infowin_p = (InfoWin_t *) client_p;

  XtUnmanageChild (InfoWin_InfoDlg_Get (*infowin_p));

  if (InfoWin_Type_Get (*infowin_p) == SVI_INFO_STAT_LOAD)
    {
    Widget         child;

    child = XmMessageBoxGetChild (InfoWin_InfoDlg_Get (*infowin_p), 
      XmDIALOG_OK_BUTTON);
    XtUnmanageChild (child);
    XtRemoveCallback (InfoWin_InfoDlg_Get (*infowin_p), XmNokCallback, 
      InfoWin_Status_Load_CB, NULL);
    }

  return ;
}
/*  End of InfoWin_Dismiss_CB  */


/****************************************************************************
*
*  Function Name:                 InfoWin_Compound_Update
*
*    Update the Information window with the information for the compound
*    the user selects.
*
*  Implicit Inputs:
*
*    SCompInfo, a SynView information window.
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
void InfoWin_Compound_Update
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p
  )
{
  PstView_t     *pv_p;
  char          *b_p;
  Compound_t    *compound_p;
  SymTab_t      *symbol_p;
  Time_t	symtime;

  pv_p = PstView_Handle_Grab ();

  /*  Get the selected compound.  */
  PstView_Compound_Sel (pv_p);
  compound_p = PstView_LastSelCmp_Get (pv_p);
  if (compound_p == NULL)
    {
    InfoWarn_Show ("No compound was selected.");
    return ;
    }

  /*  Write the information to the buffer and update buffer.  */
  b_p = InfoWin_TextBuf_Get (SCompInfo);
  symbol_p = PstComp_SymbolTable_Get (compound_p);

  sprintf (b_p, "Comp index:  %6lu,  prev:  %6lu,  next:  %6lu\n", 
    PstComp_Index_Get (compound_p), 
    (PstComp_Prev_Get (compound_p) != NULL) 
      ? PstComp_Index_Get (PstComp_Prev_Get (compound_p)) : 0,
    (PstComp_Next_Get (compound_p) != NULL) 
      ? PstComp_Index_Get (PstComp_Next_Get (compound_p)) : 0);
  b_p += strlen (b_p); 

  sprintf (b_p, "  father:  %6lu,  son:  %6lu,  bro:  %6lu\n",
    (PstComp_Father_Get (compound_p) != NULL) 
      ? PstSubg_Index_Get (PstComp_Father_Get (compound_p)) : 0,
    (PstComp_Son_Get (compound_p) != NULL) 
      ? PstSubg_Index_Get (PstComp_Son_Get (compound_p)) : 0,
    (PstComp_Brother_Get (compound_p) != NULL) 
      ? PstComp_Index_Get (PstComp_Brother_Get (compound_p)) : 0);
  b_p += strlen (b_p); 

  sprintf (b_p, "Symbol index:  %6lu, inst:  %3u,  sols:  %3u  ",
    SymTab_Index_Get (symbol_p), SymTab_InstancesCnt_Get (symbol_p),
    SymTab_NumSols_Get (symbol_p));
  b_p += strlen (b_p); 

  if (SymTab_WorkerId_Get (symbol_p) != SYM_WORKER_NONE)
    {
    sprintf (b_p, "\n  worker:  %3u  ", SymTab_WorkerId_Get (symbol_p));
    b_p += strlen (b_p); 
    }

  if (SymTab_Flags_WasSelected_Get (symbol_p))
    {
    sprintf (b_p, "WAS ");
    b_p += strlen (b_p); 
    }

  if (SymTab_Flags_DupSelect_Get (symbol_p))
    {
    sprintf (b_p, "DUP ");
    b_p += strlen (b_p); 
    }

  if (SymTab_Flags_Selected_Get (symbol_p))
    {
    sprintf (b_p, "SEL ");
    b_p += strlen (b_p); 
    }

  if (SymTab_Flags_GlobalSelect_Get (symbol_p))
    {
    sprintf (b_p, "GBL ");
    b_p += strlen (b_p); 
    }

  if (SymTab_Flags_LocalSelect_Get (symbol_p))
    {
    sprintf (b_p, "LCL ");
    b_p += strlen (b_p); 
    }

  if (SymTab_Flags_Unsolveable_Get (symbol_p))
    {
    sprintf (b_p, "UNS ");
    b_p += strlen (b_p); 
    }

  if (SymTab_Flags_Stuck_Get (symbol_p))
    {
    sprintf (b_p, "STK ");
    b_p += strlen (b_p); 
    }

  if (SymTab_Flags_Open_Get (symbol_p))
    {
    sprintf (b_p, "OPN ");
    b_p += strlen (b_p); 
    }

  if (SymTab_Flags_Solved_Get (symbol_p))
    {
    sprintf (b_p, "SOL ");
    b_p += strlen (b_p); 
    }

  if (SymTab_Flags_NewlySolved_Get (symbol_p))
    {
    sprintf (b_p, "NSL ");
    b_p += strlen (b_p); 
    }

  if (SymTab_Flags_Available_Get (symbol_p))
    {
    sprintf (b_p, "AVL:  %5lu", SymTab_AvailCompKey_Get (symbol_p));
    b_p += strlen (b_p); 
    }

  sprintf (b_p, "\n");
  b_p += strlen (b_p); 

  sprintf (b_p, "  merit main:  %3u,  sol:  %3u,  init:  %3u\n",
    SymTab_Merit_Main_Get (symbol_p), SymTab_Merit_Solved_Get (symbol_p),
    SymTab_Merit_Initial_Get (symbol_p));
  b_p += strlen (b_p); 

  symtime = SymTab_Cycle_Time_Get (symbol_p);
/*
  sprintf (b_p, 
    "\n  cycle:  %4u,  time:  %9.3f sec,  first:  %4u,  last:  %4u\n",
    SymTab_Cycle_Number_Get (symbol_p), (double) Time_Format (symtime), 
    SymTab_Cycle_First_Get (symbol_p), SymTab_Cycle_Last_Get (symbol_p));
*/
  sprintf (b_p, 
    "\n  cycle:  %4u,  time:  %s,  first:  %4u,  last:  %4u\n",
    SymTab_Cycle_Number_Get (symbol_p), Sensible_Time_Format (symtime, FALSE), 
    SymTab_Cycle_First_Get (symbol_p), SymTab_Cycle_Last_Get (symbol_p));
  b_p += strlen (b_p); 

  sprintf (b_p, "  link first:  %6lu,  dev:  %6lu,  curr:  %6lu\n",
    (SymTab_FirstComp_Get (symbol_p) != NULL) 
      ? PstComp_Index_Get (SymTab_FirstComp_Get (symbol_p)) : 0,
    (SymTab_DevelopedComp_Get (symbol_p) != NULL) 
      ? PstComp_Index_Get (SymTab_DevelopedComp_Get (symbol_p)) : 0,
    (SymTab_Current_Get (symbol_p) != NULL) 
      ? PstComp_Index_Get (SymTab_Current_Get (symbol_p)) : 0);
  b_p += strlen (b_p); 

  sprintf (b_p, "\n  %s\n", 
    (char *) Sling_Name_Get (SymTab_Sling_Get (symbol_p)));
  b_p += strlen (b_p); 

  XmTextSetString (InfoWin_TextWin_Get (SCompInfo), 
    InfoWin_TextBuf_Get (SCompInfo));

  return ;
}
/*  End of InfoWin_Compound_Update  */


/****************************************************************************
*
*  Function Name:                 InfoWin_Reaction_Update
*
*    Update the Information window with the textual information for the  
*    current reaction.
*
*  Implicit Inputs:
*
*    None.
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
void InfoWin_Reaction_Update
  (
  void
  )
{
  PstView_t        *pv_p;
  React_Head_t     *rxn_head_p;
  React_Record_t   *rxn_rec_p;
  React_TextHead_t *txt_head_p;
  React_TextRec_t  *txt_rec_p;
  Subgoal_t        *sg_p;
  char             *b_p;
  U8_t              i;

  pv_p = PstView_Handle_Grab ();
  sg_p = RxnView_CurSG_Get (PstView_RxnView_Get (pv_p));

  if (sg_p == NULL)
    {
    InfoWarn_Show ("No reaction for which to show information.");
    return ;
    }

  rxn_rec_p = React_Schema_Handle_Get (PstSubg_Reaction_Schema_Get (sg_p));
  rxn_head_p = React_Head_Get (rxn_rec_p);
  txt_rec_p = React_Text_Get (rxn_rec_p);
  txt_head_p = React_TxtRec_Head_Get (txt_rec_p);

  /*  Write the information to the buffer and update buffer.  */
  b_p = InfoWin_TextBuf_Get (SRxnInfo);

  sprintf (b_p, "%s\n\n", 
    (char *) String_Value_Get (React_TxtRec_Name_Get (txt_rec_p)));
  b_p += strlen (b_p); 


  if (React_TxtHd_NumReferences_Get (txt_head_p) == 0
      || txt_rec_p->lit_refs == NULL)
    {
    sprintf (b_p, "No literature references.\n\n");
    b_p += strlen (b_p); 
    }
  else
    {
    sprintf (b_p, "Literature References:\n");
    b_p += strlen (b_p); 

    for (i = 0; i < React_TxtHd_NumReferences_Get (txt_head_p); i++)
      {
      if (String_Value_Get (React_TxtRec_Reference_Get (txt_rec_p, i)) != NULL)
        {
        sprintf (b_p, "  %s\n", 
          (char *) String_Value_Get (React_TxtRec_Reference_Get (txt_rec_p, 
          i)));
        b_p += strlen (b_p); 
        }
      }
    }

  if (React_TxtHd_NumComments_Get (txt_head_p) == 0 
      || txt_rec_p->comments == NULL)
    {
    sprintf (b_p, "\nNo chemist comments.\n");
    b_p += strlen (b_p); 
    }
  else
    {
    sprintf (b_p, "\nChemist comments:\n");
    b_p += strlen (b_p); 

    for (i = 0; i < React_TxtHd_NumComments_Get (txt_head_p); i++)
      {
      if (String_Value_Get (React_TxtRec_Comment_Get (txt_rec_p, i)) != NULL)
        {
        sprintf (b_p, "  %s\n", 
          (char *) String_Value_Get (React_TxtRec_Comment_Get (txt_rec_p, i)));
        b_p += strlen (b_p); 
        }

      if (txt_rec_p->chemists != NULL 
          && String_Value_Get (React_TxtRec_Chemist_Get (txt_rec_p, i)) != NULL)
        {
        sprintf (b_p, "    Chemist:  %s\n\n", 
          (char *) String_Value_Get (React_TxtRec_Chemist_Get (txt_rec_p, i)));
        b_p += strlen (b_p); 
        }
      }
    }

  XmTextSetString (InfoWin_TextWin_Get (SRxnInfo), 
    InfoWin_TextBuf_Get (SRxnInfo));

  if (!XtIsManaged (InfoWin_InfoDlg_Get (SRxnInfo)))
    XtManageChild (InfoWin_InfoDlg_Get (SRxnInfo));
  return ;
}
/*  End of InfoWin_Reaction_Update  */


/****************************************************************************
*
*  Function Name:                 InfoWin_Subgoal_Update
*
*    Update the Information window with the information for the parent 
*    subgoal of compound the user selects.
*
*  Implicit Inputs:
*
*    None.
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
void InfoWin_Subgoal_Update
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p
  )
{
  PstView_t     *pv_p;
  char          *b_p;
  Compound_t    *comp_p;
  Subgoal_t     *subgoal_p;
  Time_t	 sgtime;

  pv_p = PstView_Handle_Grab ();

   /*  Get the selected compound.  */
  PstView_Compound_Sel (pv_p);
  comp_p = PstView_LastSelCmp_Get (pv_p);
  if (comp_p == NULL)
    {
    InfoWarn_Show ("No subgoal was selected.");
    return ;
    }

  subgoal_p = PstComp_Father_Get (comp_p);

  /*  Write the information to the buffer and update buffer.  */
  b_p = InfoWin_TextBuf_Get (SSubGInfo);

  sprintf (b_p,"Subgoal index:  %6lu,  level:  %2hu\n", 
    PstSubg_Index_Get (subgoal_p), PstSubg_Level_Get (subgoal_p));
  b_p += strlen (b_p); 

  sprintf (b_p, "\n  merit main:  %3d,  sol:  %3d,  init:  %3d\n",
    PstSubg_Merit_Main_Get (subgoal_p), 
    PstSubg_Merit_Solved_Get (subgoal_p),
    PstSubg_Merit_Initial_Get (subgoal_p));
  b_p += strlen (b_p); 

  sgtime = Strategy_Subg_TimeSpent_Get (PstSubg_Strategy_Get (subgoal_p));
/*
  sprintf (b_p, 
    "  strategy time:  %9.4f sec,  closed:  %3u,  visits:  %3u\n",
    (double) Time_Format (sgtime), 
    Strategy_Subg_ClosedCnt_Get (PstSubg_Strategy_Get (subgoal_p)), 
    PstSubg_Visits_Get (subgoal_p));
*/
  sprintf (b_p, 
    "  strategy time:  %s,  closed:  %3u,  visits:  %3u\n",
    Sensible_Time_Format (sgtime, TRUE), 
    Strategy_Subg_ClosedCnt_Get (PstSubg_Strategy_Get (subgoal_p)), 
    PstSubg_Visits_Get (subgoal_p));
  b_p += strlen (b_p); 

  sprintf (b_p, "\n   rxn:  %4lu,  ease:  %3u,  yield:  %3u,  conf:  %3u\n",
    PstSubg_Reaction_Schema_Get (subgoal_p),    
    PstSubg_Reaction_Ease_Get (subgoal_p),
    PstSubg_Reaction_Yield_Get (subgoal_p),
    PstSubg_Reaction_Confidence_Get (subgoal_p));
  b_p += strlen (b_p); 

  sprintf (b_p, "  links father:  %6lu,  son:  %6lu,  bro:  %6lu\n",
    (PstSubg_Father_Get (subgoal_p) != NULL) 
      ? PstComp_Index_Get (PstSubg_Father_Get (subgoal_p)) : 0,
    (PstSubg_Son_Get (subgoal_p) != NULL) 
      ? PstComp_Index_Get (PstSubg_Son_Get (subgoal_p)) : 0,
    (PstSubg_Brother_Get (subgoal_p) != NULL) 
      ? PstSubg_Index_Get (PstSubg_Brother_Get (subgoal_p)) : 0);
  b_p += strlen (b_p); 

  XmTextSetString (InfoWin_TextWin_Get (SSubGInfo), 
    InfoWin_TextBuf_Get (SSubGInfo));

  return ;
}
/*  End of InfoWin_Subgoal_Update  */


/****************************************************************************
*
*  Function Name:                 InfoWin_Status_Load_CB
*
*    This routine unmanages the information window dialog, removes the 
*    callbacks, and unmanages the okay button.  It then calls the 
*    function to load in the status file.
*
*
*  Implicit Inputs:
*
*    SStatInfo, the SynView information window.
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
*    None.
*
******************************************************************************/
void InfoWin_Status_Load_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p
  )
{
  Widget           child;

  XtUnmanageChild (InfoWin_InfoDlg_Get (SStatInfo));

  child = XmMessageBoxGetChild (InfoWin_InfoDlg_Get (SStatInfo), 
    XmDIALOG_OK_BUTTON);
  XtUnmanageChild (child);
  XtRemoveCallback (InfoWin_InfoDlg_Get (SStatInfo), XmNokCallback, 
    InfoWin_Status_Load_CB, NULL);

  /*  Load status file stored in info win field.  */

  return ;
}
/*  End of InfoWin_Status_Load_CB  */


/****************************************************************************
*
*  Function Name:                 InfoWin_Status_Update
*
*    Update the Information window with run control block and run
*    status data.
*
*  Implicit Inputs:
*
*    SStatInfo, the SynView information window.
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
void InfoWin_Status_Update
  (
  Rcb_t         *rcb_p,
  RunStats_t    *rstat_p,
  char          *filename
  )
{
  XmString       title;
  char          *b_p;
  Timer_t       *t_p;

  if (rcb_p == NULL || rstat_p == NULL)
    return;

  if (XtIsManaged (InfoWin_InfoDlg_Get (SStatInfo)))
    return;

  /*  If there is a filename, add callback for okay button.  */
  if (filename != NULL)
    {
    Widget         child;
    XmString       load;

    InfoWin_Type_Put (SStatInfo, SVI_INFO_STAT_LOAD);
    strncpy (InfoWin_Filename_Get (SStatInfo), filename, MX_FILENAME - 1);

    title = XmStringCreateLtoR (SVI_INFO_TITLE_STLD, XmFONTLIST_DEFAULT_TAG);
    load = XmStringCreateLtoR (SVI_INFO_PB_LOAD, XmFONTLIST_DEFAULT_TAG);
    XtVaSetValues (InfoWin_InfoDlg_Get (SStatInfo),
      XmNdialogTitle,   title, 
      XmNokLabelString, load,
      NULL);
    XmStringFree (load);
    XmStringFree (title);

    child = XmMessageBoxGetChild (InfoWin_InfoDlg_Get (SStatInfo), 
      XmDIALOG_OK_BUTTON);
    XtManageChild (child);
    XtAddCallback (InfoWin_InfoDlg_Get (SStatInfo), XmNokCallback, 
      InfoWin_Status_Load_CB, NULL);
    }

  else
    {
    InfoWin_Type_Put (SStatInfo, SVI_INFO_STAT);

    title = XmStringCreateLtoR (SVI_INFO_TITLE_STAT, XmFONTLIST_DEFAULT_TAG);
    XtVaSetValues (InfoWin_InfoDlg_Get (SStatInfo),
      XmNdialogTitle, title, 
      NULL);
    XmStringFree (title);
    }

  /*  Write the information to the buffer and update buffer.  */
  b_p = InfoWin_TextBuf_Get (SStatInfo);

  sprintf (b_p, "Dump of RCB\n");
  b_p += strlen (b_p); 

  sprintf (b_p, "  Directories:\n");
  b_p += strlen (b_p); 

  sprintf (b_p, "%-25s  %s\n", "    Available Compounds:", 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_AVLC))));
  b_p += strlen (b_p); 

  sprintf (b_p, "%-25s  %s\n", "    Functional Groups:", 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_FNGP))));
  b_p += strlen (b_p); 

  sprintf (b_p, "%-25s  %s\n", "    Reaction KB:", 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p,
    FCB_IND_RXNS))));
  b_p += strlen (b_p); 

  sprintf (b_p, "%-25s  %s\n\n", "    Template:", 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_TMPL))));
  b_p += strlen (b_p); 

  sprintf (b_p, "%-25s  %s\n", "    Paths:", 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p,
    FCB_IND_PATH))));
  b_p += strlen (b_p); 

  sprintf (b_p, "%-25s  %s\n", "    Status:", 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_STAT))));
  b_p += strlen (b_p); 

  sprintf (b_p, "%-25s  %s\n", "    Submission:", 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_SBMT))));
  b_p += strlen (b_p); 

  sprintf (b_p, "%-25s  %s\n", "    Trace:", 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_TRCE))));
  b_p += strlen (b_p); 

  sprintf (b_p, "%-25s  %s\n", "    Force Selection:", 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_FSEL))));
  b_p += strlen (b_p); 


  sprintf (b_p, "%-25s  %s\n\n", "    Search Space Cover:", 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_COVR))));
  b_p += strlen (b_p); 


  sprintf (b_p, "\n%-16s  %s\n", "  User name:", 
    String_Value_Get (Rcb_User_Get (rcb_p)));
  b_p += strlen (b_p); 

  sprintf (b_p, "%-16s  %s\n", "  Target name:", 
    String_Value_Get (Rcb_Name_Get (rcb_p)));
  b_p += strlen (b_p); 

  sprintf (b_p, "%-16s  %s\n", "  Target sling:", 
    Sling_Name_Get (Rcb_Goal_Get (rcb_p)));
  b_p += strlen (b_p); 

  sprintf (b_p, "%-16s  %s", "  Comment:", 
    String_Value_Get (Rcb_Comment_Get (rcb_p)));
  b_p += strlen (b_p); 

  sprintf (b_p, "%-16s  %s\n", "  Date of run:", 
    ctime (&((Rcb_Date_Get (rcb_p)).tv_sec)));
  b_p += strlen (b_p); 

  sprintf (b_p, "%-16s  %lu\n", "  Max cycles:", Rcb_MaxCycles_Get (rcb_p));
  b_p += strlen (b_p); 

  sprintf (b_p, "%-16s  %lu minutes\n", "  Max runtime:", 
    Rcb_MaxRuntime_Get (rcb_p));
  b_p += strlen (b_p); 

  sprintf (b_p, "%-16s  %8s   %-16s  %8s\n", "  Restart:", 
    (Rcb_Flags_Restart_Get (rcb_p)) ? "enabled" : "disabled", "Background:", 
    (Rcb_Flags_Background_Get (rcb_p)) ? "enabled" : "disabled");
  b_p += strlen (b_p); 

  sprintf (b_p, "%-16s  %hu\n", "  Tracing level:", 
    Rcb_ChemistryTrace_Get (rcb_p));
  b_p += strlen (b_p); 

  if (Rcb_Flags_EffortDis_Get (rcb_p))
    sprintf (b_p, "%-16s  %s\n", "  Effort Dist:", "enabled");
  else
    sprintf (b_p, "%-16s  %s\n", "  Effort Dist:", "disabled");
  b_p += strlen (b_p); 

  sprintf (b_p, "%-16s  %hu\n", "  NTCL:", Rcb_NTCL_Get (rcb_p));
  b_p += strlen (b_p); 

  if (Rcb_Flags_Continue_Get (rcb_p))
    {
    sprintf (b_p, "%-16s  %lu\n", "  Leapsize:", Rcb_LeapSize_Get (rcb_p));
    b_p += strlen (b_p); 
    }

  if (Rcb_Flags_FirstSol_Get (rcb_p))
    {
    sprintf (b_p, "  End search after first solution found:\n");
    b_p += strlen (b_p); 

    sprintf (b_p, "%-24s  %lu\n", "    Additional cycles:", 
      Rcb_AddCycles_Get (rcb_p));
    b_p += strlen (b_p); 

    sprintf (b_p, "%-24s  %lu minutes\n", "    Additional time:", 
      Rcb_AddTime_Get (rcb_p));
    b_p += strlen (b_p); 
    }

  if (Rcb_Flags_SearchCoverDev_Get (rcb_p) 
      || Rcb_Flags_SearchCoverSlv_Get (rcb_p))
    {
    sprintf (b_p, "  Seq Cover:  %s\n    With:  ", 
      (char *) String_Value_Get (FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, 
      FCB_IND_COVR))));
    b_p += strlen (b_p); 

    if (Rcb_Flags_SearchCoverSlv_Get (rcb_p))
      {
      sprintf (b_p, "solved coverage");
      b_p += strlen (b_p); 
      }

    if (Rcb_Flags_SearchCoverDev_Get (rcb_p))
      {
      sprintf (b_p, "   developed coverage (%u%%)", Rcb_CoverPrct_Get (rcb_p));
      b_p += strlen (b_p); 
      }

    sprintf (b_p, "\n");
    b_p += strlen (b_p); 
    }

  if (Rcb_RunType_Get (rcb_p) == RCB_TYPE_SEQ)
    sprintf (b_p, "  Run type:  Sequential\n");
  else if (Rcb_RunType_Get (rcb_p) == RCB_TYPE_SEQ_FORCESEL)
    sprintf (b_p, "  Run type:  Forced Selection\n");
  else if (Rcb_RunType_Get (rcb_p) == RCB_TYPE_DIS_LIN)
    sprintf (b_p, "  Run type:  Distributed, Linda version\n");
  else
    sprintf (b_p, "  Run type:  Unknown Type\n");
  b_p += strlen (b_p); 

  if (Rcb_RunType_Get (rcb_p) >= RCB_TYPE_DIS_START)
    {
    sprintf (b_p, "  Distributed run with parameters:\n");
    b_p += strlen (b_p); 

    sprintf (b_p, "%-16s  %hu\n", "    Max nodes:", 
      Rcb_MaxNodes_Get (rcb_p));
    b_p += strlen (b_p); 

    sprintf (b_p, "%-16s  %u\n", "    Max workers:", 
      Rcb_MaxWorkers_Get (rcb_p));
    b_p += strlen (b_p); 

    sprintf (b_p, "%-16s  %u\n", "    Min workers:", 
      Rcb_MinWorkers_Get (rcb_p));
    b_p += strlen (b_p); 

    sprintf (b_p, "%-16s  %hu per node\n", "    Max procs:", 
      Rcb_ProcPerNode_Get (rcb_p));
    b_p += strlen (b_p); 

    sprintf (b_p, "    Master Selection:  %s\n", 
      Rcb_Flags_MasterSelNB_Get (rcb_p) ? "Next Best" : "Selected Sibling");
    b_p += strlen (b_p); 

    if (Rcb_Flags_AndParallel_Get (rcb_p))
      sprintf (b_p, "      And Parallelism:  enabled\n");
    else
      sprintf (b_p, "      And Parallelism:  disabled\n");
    b_p += strlen (b_p); 

    sprintf (b_p, "    Workers, max cycles:  %u\n",
      Rcb_WorkerMaxCycles_Get (rcb_p));
    b_p += strlen (b_p); 

    if (Rcb_Flags_WorkerLocalSel_Get (rcb_p))
      sprintf (b_p, "      Local Selection:  enabled\n");
    else
      sprintf (b_p, "      Local Selection:  disabled\n");
    b_p += strlen (b_p); 

    if (Rcb_Flags_DisSharedSels_Get (rcb_p))
      sprintf (b_p, "      Distribute Shared Sels:  enabled\n");
    else
      sprintf (b_p, "      Distribute Shared Sels:  disabled\n");
    b_p += strlen (b_p); 

    if (Rcb_Flags_WorkerPreferGlobal_Get (rcb_p))
      sprintf (b_p, "      Prefer Global Selection:  enabled\n");
    else
      sprintf (b_p, "      Prefer Global Selection:  disabled\n");
    b_p += strlen (b_p); 

    if (Rcb_Flags_WorkerCurMerit_Get (rcb_p))
      sprintf (b_p, "      Current Merit:  enabled (%hu%%)\n", 
        Rcb_WorkerMeritPrct_Get (rcb_p));
    else
      sprintf (b_p, "      Current Merit:  disabled\n");
    b_p += strlen (b_p); 

    if (Rcb_Flags_WorkerNBMerit_Get (rcb_p))
      sprintf (b_p, "      Next Best Merit:  enabled (%hu%%)\n", 
        Rcb_WorkerMeritPrct_Get (rcb_p));
    else
      sprintf (b_p, "      Next Best Merit:  disabled\n");
    b_p += strlen (b_p); 
    }

  sprintf (b_p, "\n\nDump of Run Status\n");
  b_p += strlen (b_p); 

  sprintf (b_p, "%-30s  %lu\n", "  Cycles completed:", 
    RunStats_CumCycles_Get (rstat_p));
  b_p += strlen (b_p); 

  if (RunStats_Flags_FirstSolFound_Get (rstat_p))
    {
    sprintf (b_p, "%-30s  %lu\n", "  Cycles to first solution:", 
      RunStats_CyclesFirstSol_Get (rstat_p));
    b_p += strlen (b_p); 

/*
    sprintf (b_p, "%-30s  %f seconds\n", "  Time to first solution:", 
      Time_Format (RunStats_TimeFirstSol_Get (rstat_p)));
*/
    sprintf (b_p, "%-30s  %s\n", "  Time to first solution:", 
      Sensible_Time_Format (RunStats_TimeFirstSol_Get (rstat_p), FALSE));
    b_p += strlen (b_p); 
    }

  sprintf (b_p, "%-30s  %lu\n", "  Unique compounds generated:", 
    RunStats_NumSymbols_Get (rstat_p));
  b_p += strlen (b_p); 

  sprintf (b_p, "%-30s  %lu\n", "  Total compounds generated:", 
    RunStats_NumCompounds_Get (rstat_p));
  b_p += strlen (b_p); 

  sprintf (b_p, "%-30s  %lu\n\n", "  Total subgoals generated:", 
    RunStats_NumSubgoals_Get (rstat_p));
  b_p += strlen (b_p); 

  if (RunStats_Flags_StatusSaved_Get (rstat_p))
    sprintf (b_p, "  Status file was saved\n");
  else
    sprintf (b_p, "  Status file was not saved\n");
  b_p += strlen (b_p); 

  if (RunStats_Flags_TraceSaved_Get (rstat_p))
    sprintf (b_p, "  Trace file was saved\n\n");
  else
    sprintf (b_p, "  Trace file was not saved\n\n");
  b_p += strlen (b_p); 

  if (Rcb_RunType_Get (rcb_p) >= RCB_TYPE_DIS_START)
    {
    t_p = RunStats_MasterCumTimes_Get (rstat_p);

    sprintf (b_p, "  Master cummulative times:\n");
    b_p += strlen (b_p); 

/*
    sprintf (b_p, "%-24s  %9.4f\n", "    Initialization:", 
      Time_Format (Timer_Init_Get (t_p)));
*/
    sprintf (b_p, "%-24s  %s\n", "    Initialization:", 
      Sensible_Time_Format (Timer_Init_Get (t_p), TRUE));
    b_p += strlen (b_p); 

/*
    sprintf (b_p, "%-24s  %9.4f\n", "    Select next:", 
      Time_Format (Timer_Select_Get (t_p)));
*/
    sprintf (b_p, "%-24s  %s\n", "    Select next:", 
      Sensible_Time_Format (Timer_Select_Get (t_p), TRUE));
    b_p += strlen (b_p); 

/*
    sprintf (b_p, "%-24s  %9.4f\n", "    Update:", 
      Time_Format (Timer_UpdatePST_Get (t_p)));
*/
    sprintf (b_p, "%-24s  %s\n", "    Update:", 
      Sensible_Time_Format (Timer_UpdatePST_Get (t_p), TRUE));
    b_p += strlen (b_p); 

/*
    sprintf (b_p, "%-24s  %9.4f\n\n", "    Cycle time:", 
      Time_Format (Timer_Cycle_Get (t_p)));
*/
    sprintf (b_p, "%-24s  %s\n\n", "    Cycle time:", 
      Sensible_Time_Format (Timer_Cycle_Get (t_p), TRUE));
    b_p += strlen (b_p); 

/*
    sprintf (b_p, "%-24s  %9.4f\n", "    Wait:", 
      Time_Format (Timer_Wait_Get (t_p)));
*/
    sprintf (b_p, "%-24s  %s\n", "    Wait:", 
      Sensible_Time_Format (Timer_Wait_Get (t_p), TRUE));
    b_p += strlen (b_p); 

/*
    sprintf (b_p, "%-24s  %9.4f\n\n", "    Communication:", 
      Time_Format (Timer_Commun_Get (t_p)));
*/
    sprintf (b_p, "%-24s  %s\n\n", "    Communication:", 
      Sensible_Time_Format (Timer_Commun_Get (t_p), TRUE));
    b_p += strlen (b_p); 

/*
    sprintf (b_p, "%-24s  %9.4f\n\n", "    Tuple wait:", 
      Time_Format (Timer_TupleWait_Get (t_p)));
*/
    sprintf (b_p, "%-24s  %s\n\n", "    Tuple wait:", 
      Sensible_Time_Format (Timer_TupleWait_Get (t_p), TRUE));
    b_p += strlen (b_p); 

    t_p = RunStats_WorkerCumTimes_Get (rstat_p);

    sprintf (b_p, "  Worker cummulative times:\n");
    b_p += strlen (b_p); 

/*
    sprintf (b_p, "%-24s  %9.4f\n", "    Initialization:", 
      Time_Format (Timer_Init_Get (t_p)));
*/
    sprintf (b_p, "%-24s  %s\n", "    Initialization:", 
      Sensible_Time_Format (Timer_Init_Get (t_p), TRUE));
    b_p += strlen (b_p); 

/*
    sprintf (b_p, "%-24s  %9.4f\n", "    Select next:", 
      Time_Format (Timer_Select_Get (t_p)));
*/
    sprintf (b_p, "%-24s  %s\n", "    Select next:", 
      Sensible_Time_Format (Timer_Select_Get (t_p), TRUE));
    b_p += strlen (b_p); 

/*
    sprintf (b_p, "%-24s  %9.4f\n", "    Expansion:", 
      Time_Format (Timer_Expand_Get (t_p)));
*/
    sprintf (b_p, "%-24s  %s\n", "    Expansion:", 
      Sensible_Time_Format (Timer_Expand_Get (t_p), TRUE));
    b_p += strlen (b_p); 

/*
    sprintf (b_p, "%-24s  %9.4f\n", "    Update:", 
      Time_Format (Timer_UpdatePST_Get (t_p)));
*/
    sprintf (b_p, "%-24s  %s\n", "    Update:", 
      Sensible_Time_Format (Timer_UpdatePST_Get (t_p), TRUE));
    b_p += strlen (b_p); 

/*
    sprintf (b_p, "%-24s  %9.4f\n\n", "    Cycle time:", 
      Time_Format (Timer_Cycle_Get (t_p)));
*/
    sprintf (b_p, "%-24s  %s\n\n", "    Cycle time:", 
      Sensible_Time_Format (Timer_Cycle_Get (t_p), TRUE));
    b_p += strlen (b_p); 

/*
    sprintf (b_p, "%-24s  %9.4f\n", "    Communication:", 
      Time_Format (Timer_Commun_Get (t_p)));
*/
    sprintf (b_p, "%-24s  %s\n", "    Communication:", 
      Sensible_Time_Format (Timer_Commun_Get (t_p), TRUE));
    b_p += strlen (b_p); 

/*
    sprintf (b_p, "%-24s  %9.4f\n\n", "    Wait:", 
      Time_Format (Timer_Wait_Get (t_p)));
*/
    sprintf (b_p, "%-24s  %s\n\n", "    Wait:", 
      Sensible_Time_Format (Timer_Wait_Get (t_p), TRUE));
    b_p += strlen (b_p); 

/*
    sprintf (b_p, "%-24s  %9.4f\n\n", "    Tuple wait:", 
      Time_Format (Timer_TupleWait_Get (t_p)));
*/
    sprintf (b_p, "%-24s  %s\n\n", "    Tuple wait:", 
      Sensible_Time_Format (Timer_TupleWait_Get (t_p), TRUE));
    b_p += strlen (b_p); 

    }

  else  /*  Sequential run */
    {
    t_p = RunStats_CumTimes_Get (rstat_p);

    sprintf (b_p, "  Cummulative times:\n");
    b_p += strlen (b_p); 

/*
    sprintf (b_p, "%-24s  %9.4f\n", "    Initialization:", 
      Time_Format (Timer_Init_Get (t_p)));
*/
    sprintf (b_p, "%-24s  %s\n", "    Initialization:", 
      Sensible_Time_Format (Timer_Init_Get (t_p), TRUE));
    b_p += strlen (b_p); 

/*
    sprintf (b_p, "%-24s  %9.4f\n", "    Select next:", 
      Time_Format (Timer_Select_Get (t_p)));
*/
    sprintf (b_p, "%-24s  %s\n", "    Select next:", 
      Sensible_Time_Format (Timer_Select_Get (t_p), TRUE));
    b_p += strlen (b_p); 

/*
    sprintf (b_p, "%-24s  %9.4f\n", "    Expansion:", 
      Time_Format (Timer_Expand_Get (t_p)));
*/
    sprintf (b_p, "%-24s  %s\n", "    Expansion:", 
      Sensible_Time_Format (Timer_Expand_Get (t_p), TRUE));
    b_p += strlen (b_p); 

/*
    sprintf (b_p, "%-24s  %9.4f\n", "    Update:", 
      Time_Format (Timer_UpdatePST_Get (t_p)));
*/
    sprintf (b_p, "%-24s  %s\n", "    Update:", 
      Sensible_Time_Format (Timer_UpdatePST_Get (t_p), TRUE));
    b_p += strlen (b_p); 

/*
    sprintf (b_p, "%-24s  %9.4f\n\n", "    Cycle time:", 
      Time_Format (Timer_Cycle_Get (t_p)));
*/
    sprintf (b_p, "%-24s  %s\n\n", "    Cycle time:", 
      Sensible_Time_Format (Timer_Cycle_Get (t_p), TRUE));
    b_p += strlen (b_p); 
    }


  sprintf (b_p, "  Exit Condition:  %s\n\n", (char *) String_Value_Get (
    RunStats_ExitCond_Get (rstat_p)));
  b_p += strlen (b_p); 

  XtVaSetValues (InfoWin_TextWin_Get (SStatInfo),
    XmNrows, SVI_INFO_NUMROWS,
    NULL);
  XmTextSetString (InfoWin_TextWin_Get (SStatInfo), 
    InfoWin_TextBuf_Get (SStatInfo));
  XtManageChild (InfoWin_InfoDlg_Get (SStatInfo));

  return ;
}
/*  End of InfoWin_Status_Update  */

char *Sensible_Time_Format (Time_t tim, Boolean_t fourth_decimal_place)
{
  double seconds;
  int frac_sec, sec, min, hr, days;
  char decimal_str[8];
  static char stf[64];

  seconds = (double) Time_Format (tim);
  if (fourth_decimal_place)
  {
    seconds += .00005;
    frac_sec = (int) (10000. * seconds) % 10000;
    sprintf (decimal_str, "%04d", frac_sec);
  }
  else
  {
    seconds += .0005;
    frac_sec = (int) (1000. * seconds) % 1000;
    sprintf (decimal_str, "%03d", frac_sec);
  }
  sec = (int) seconds;
  min = (int) sec / 60;
  sec %= 60;
  hr = min / 60;
  min %= 60;
  days = hr / 24;
  hr %= 24;
  if (days > 0) sprintf (stf, "%d day%s + %d:%02d:%02d.%s", days, days > 1 ? "s" : "", hr, min, sec, decimal_str);
  else if (hr > 0) sprintf (stf, "%d:%02d:%02d.%s", hr, min, sec, decimal_str);
  else if (min > 0) sprintf (stf, "%d min., %d.%s sec.", min, sec, decimal_str);
  else sprintf (stf, "%d.%s sec.", sec, decimal_str);
  return (stf);
}

/*  End of INFO_VIEW.C  */
