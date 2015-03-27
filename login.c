/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:                     LOGIN.C
*
*    Module containing functions for maintaining security clearance for access
*    to Synchem editing and maintenance functions.
*
*  Creation Date:
*
*    23-Feb-2000
*
*  Authors:
*
*    Gerald A. Miller
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
#include <math.h>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xutil.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/Scale.h>
#include <Xm/ScrolledW.h>
#include <Xm/Text.h>

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#include "synchem.h"
#include "app_resrc.h"

#ifndef _H_RCB_
#include "rcb.h"
#endif

#define _GLOBAL_DEF_
#include "login.h"
#undef _GLOBAL_DEF_

#ifndef _H_EXTERN_
#include "extern.h"
#endif

/*******************************************************************************************
login -

  defined by macros into the following "entry" prototypes:
    void Login (Widget, void (*)());
    void AddUser (Widget);
    void ChangePassword (Widget);
    void ChangeLevel (Widget);
    void DelUser (Widget);
    void ListUsers (Widget);
    void LoginOrCancel (Widget, void (*)());
*******************************************************************************************/

char *UserId ()
{
  return (userstr);
}

void login (Widget top_level, void (*restricted_function)(), int entry_id, Widget manage_widg)
{
  Widget user_lbl, pass_lbl, dup_pass_lbl, user_txt, pass_txt, dup_pass_txt, login_pb, cancel_pb, clr_slider, widg,
    lev_pb, pw_pb, dismiss_pb, optional_lbl;
  XmFontList flhv18;
  XmFontListEntry helv18;
  XmString label;
  int i;

  tl = top_level;
  mgw = manage_widg;

  for (i=1; i<NUM_PW_TYPES; i++) pwstr[i][0] = '\0';

  rest_func[entry_id] = restricted_function;

  if (!first_login[entry_id])
    {
    if ((entry_id == ELogin || entry_id == ELoginOrCancel) && logged_in)
      {
      AlreadyLoggedIn (top_level);
      return;
      }
    XtVaSetValues (XtNameToWidget (login_popup[entry_id], "UserTxt"),
      XmNvalue, "",
      NULL);

    switch (entry_id)
      {
    case EAddUser:
    case EChangePassword:
      XtVaSetValues (XtNameToWidget (login_popup[entry_id], "PassTxt2"),
        XmNvalue, "",
        NULL);
      /* no break - drop through */
    case ELogin:
    case ELoginOrCancel:
      XtVaSetValues (XtNameToWidget (login_popup[entry_id], "PassTxt"),
        XmNvalue, "",
        NULL);
      break;
    case EChangeLevel:
    case EDelUser:
    case EListUsers:
    default:
      break;
      }

    XtManageChild (login_popup[entry_id]);
    return;
    }

  first_login[entry_id] = FALSE;

  login_popup[entry_id] = XmCreateFormDialog (top_level, "LoginFmDg", NULL, 0);

  helv18 = XmFontListEntryLoad (XtDisplay (login_popup[entry_id]),
    "-*-helvetica-bold-r-normal-*-18-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);
  flhv18 = XmFontListAppendEntry (NULL, helv18);

  switch (entry_id)
    {
  case ELogin:
    label = XmStringCreateLocalized ("Login");
    break;
  case EAddUser:
    label = XmStringCreateLocalized ("New User");
    break;
  case EChangePassword:
    label = XmStringCreateLocalized ("Change Password");
    break;
  case EChangeLevel:
    label = XmStringCreateLocalized ("Change Clearance Level");
    break;
  case EDelUser:
    label = XmStringCreateLocalized ("Delete User");
    break;
  case EListUsers:
    label = XmStringCreateLocalized ("List Users");
    break;
  case ELoginOrCancel:
    label = XmStringCreateLocalized ("Optional Global Login");
    break;
  default:
    label = XmStringCreateLocalized ("UNKNOWN ENTRY!");
    break;
    }

  XtVaSetValues (login_popup[entry_id],
    XmNdialogTitle,  label,
    XmNdialogStyle,  XmDIALOG_MODELESS,
    XmNinitialFocus, user_txt,
    XmNautoUnmanage, False,
    NULL);

  XmStringFree (label);

  if (entry_id == EChangePassword)
    label = XmStringCreateLocalized ("Old Password:");
  else if (entry_id == EListUsers)
    label = XmStringCreateLocalized ("Please Enter Your Password Again:");
  else
    label = XmStringCreateLocalized ("Username:");

  user_lbl =  XtVaCreateManagedWidget ("UserLbl",
    xmLabelWidgetClass, login_popup[entry_id],
          XmNfontList, flhv18,
    XmNlabelString,  label,
    XmNtopAttachment, XmATTACH_FORM,
    XmNtopOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    NULL);

  XmStringFree (label);

  user_txt =  XtVaCreateManagedWidget ("UserTxt",
    xmTextWidgetClass, login_popup[entry_id],
    XmNmaxLength, entry_id == EChangePassword ? PW_LEN : USER_LEN,
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
          XmNfontList, flhv18,
    XmNresizeWidth, True,
    XmNmaxLength, 475,
    XmNvalue, "",
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, user_lbl,
    XmNtopOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    NULL);

  XtVaSetValues (login_popup[entry_id],
    XmNinitialFocus, user_txt,
    NULL);

  switch (entry_id)
    {
  case ELogin:
  case EAddUser:
  case ELoginOrCancel:
    XtAddCallback (user_txt, XmNvalueChangedCallback, User_CB,
      (XtPointer) entry_id);

    label = XmStringCreateLocalized ("Password:");
    pass_lbl =  XtVaCreateManagedWidget ("PassLbl",
      xmLabelWidgetClass, login_popup[entry_id],
          XmNfontList, flhv18,
      XmNlabelString,  label,
      XmNtopAttachment, XmATTACH_WIDGET,
      XmNtopWidget, user_txt,
      XmNtopOffset, 0,
      XmNleftAttachment, XmATTACH_FORM,
      XmNleftOffset, 0,
      NULL);
    XmStringFree (label);

    pass_txt =  XtVaCreateManagedWidget ("PassTxt",
      xmTextWidgetClass, login_popup[entry_id],
      XmNmaxLength, PW_LEN,
      XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
      XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
          XmNfontList, flhv18,
      XmNresizeWidth, True,
      XmNmaxLength, 475,
      XmNvalue, "",
      XmNtopAttachment, XmATTACH_WIDGET,
      XmNtopWidget, pass_lbl,
      XmNtopOffset, 0,
      XmNleftAttachment, XmATTACH_FORM,
      XmNleftOffset, 0,
      NULL);

    XtAddCallback (pass_txt, XmNmodifyVerifyCallback, PW_CB,
      (XtPointer) (entry_id == ELogin || entry_id == ELoginOrCancel ? NOTIFY : NEW_NOTIFY));
    XtAddCallback (pass_txt, XmNvalueChangedCallback, PW_CB,
      (XtPointer) (entry_id == ELogin || entry_id == ELoginOrCancel ? MODIFY : NEW_MODIFY));
    break;

  case EChangePassword:
    /* user_txt contains old password - handle concealment */
    XtAddCallback (user_txt, XmNmodifyVerifyCallback, PW_CB,
      (XtPointer) OLD_NOTIFY);
    XtAddCallback (user_txt, XmNvalueChangedCallback, PW_CB,
      (XtPointer) OLD_MODIFY);

    label = XmStringCreateLocalized ("New Password:");
    pass_lbl =  XtVaCreateManagedWidget ("PassLbl",
      xmLabelWidgetClass, login_popup[entry_id],
          XmNfontList, flhv18,
      XmNlabelString,  label,
      XmNtopAttachment, XmATTACH_WIDGET,
      XmNtopWidget, user_txt,
      XmNtopOffset, 0,
      XmNleftAttachment, XmATTACH_FORM,
      XmNleftOffset, 0,
      NULL);
    XmStringFree (label);

    pass_txt =  XtVaCreateManagedWidget ("PassTxt",
      xmTextWidgetClass, login_popup[entry_id],
      XmNmaxLength, PW_LEN,
      XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
      XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
          XmNfontList, flhv18,
      XmNresizeWidth, True,
      XmNmaxLength, 475,
      XmNvalue, "",
      XmNtopAttachment, XmATTACH_WIDGET,
      XmNtopWidget, pass_lbl,
      XmNtopOffset, 0,
      XmNleftAttachment, XmATTACH_FORM,
      XmNleftOffset, 0,
      NULL);

    XtAddCallback (pass_txt, XmNmodifyVerifyCallback, PW_CB,
      (XtPointer) NEW_NOTIFY);
    XtAddCallback (pass_txt, XmNvalueChangedCallback, PW_CB,
      (XtPointer) NEW_MODIFY);
    break;

  case EListUsers:
    /* user_txt contains password - handle concealment */
    XtAddCallback (user_txt, XmNmodifyVerifyCallback, PW_CB,
      (XtPointer) OLD_NOTIFY);
    XtAddCallback (user_txt, XmNvalueChangedCallback, PW_CB,
      (XtPointer) OLD_MODIFY);

    usr_dlg = XmCreateFormDialog (tl, "UsrForm", NULL, 0);
    usr_win =  XmCreateScrolledWindow (usr_dlg, "UsrWin", NULL, 0);
    usr_list =  XmCreateList (usr_win, "UsrList", NULL, 0);

    label = XmStringCreateLocalized ("List of Synchem Users");

    XtVaSetValues (usr_dlg,
      XmNdialogTitle,  label,
      XmNdialogStyle,  XmDIALOG_MODELESS,
      XmNautoUnmanage, False,
      NULL);

    XmStringFree (label);

    XtVaSetValues (usr_win,
      XmNtopOffset, 0,
      XmNtopAttachment, XmATTACH_FORM,
      XmNbottomOffset, 0,
      XmNbottomAttachment, XmATTACH_NONE,
      XmNleftOffset, 0,
      XmNleftAttachment, XmATTACH_FORM,
      XmNrightOffset, 0,
      XmNrightAttachment, XmATTACH_FORM,
      NULL);

    label = XmStringCreateLocalized ("Dismiss");

    dismiss_pb = XtVaCreateManagedWidget ("DismissPB",
      xmPushButtonGadgetClass, usr_dlg,
      XmNfontList, flhv18,
      XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
      XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
      XmNlabelString, label,
      XmNtopAttachment, XmATTACH_WIDGET,
      XmNtopWidget, usr_win,
      XmNtopOffset, 0,
      XmNrightAttachment, XmATTACH_FORM,
      XmNrightOffset, 0,
      NULL);

    XmStringFree (label);

    XtAddCallback (dismiss_pb, XmNactivateCallback, UnmapList_CB,
      (XtPointer) NULL);

    XtVaSetValues (usr_list,
      XmNbackground,               SynAppR_IthClrPx_Get(&GSynAppR,
                                                        SAR_CLRI_WHITE),
      XmNforeground,               SynAppR_IthClrPx_Get(&GSynAppR,
                                                        SAR_CLRI_BLACK),
      XmNfontList,                 flhv18,
      XmNselectionPolicy,          XmSINGLE_SELECT,
      XmNvisibleItemCount,         25,
      NULL);

    break;

  case EChangeLevel:
  case EDelUser:
  default:
    break;
    }

  if (entry_id == EAddUser || entry_id == EChangePassword)
    {
    label = XmStringCreateLocalized ("Re-enter New Password:");

    dup_pass_lbl =  XtVaCreateManagedWidget ("PassLbl2",
      xmLabelWidgetClass, login_popup[entry_id],
      XmNfontList, flhv18,
      XmNlabelString,  label,
      XmNtopAttachment, XmATTACH_WIDGET,
      XmNtopWidget, pass_txt,
      XmNtopOffset, 0,
      XmNleftAttachment, XmATTACH_FORM,
      XmNleftOffset, 0,
      NULL);

    XmStringFree (label);

    dup_pass_txt =  XtVaCreateManagedWidget ("PassTxt2",
      xmTextWidgetClass, login_popup[entry_id],
      XmNmaxLength, PW_LEN,
      XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
      XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
      XmNfontList, flhv18,
      XmNresizeWidth, True,
      XmNmaxLength, 475,
      XmNvalue, "",
      XmNtopAttachment, XmATTACH_WIDGET,
      XmNtopWidget, dup_pass_lbl,
      XmNtopOffset, 0,
      XmNleftAttachment, XmATTACH_FORM,
      XmNleftOffset, 0,
      NULL);

    XtAddCallback (dup_pass_txt, XmNmodifyVerifyCallback, PW_CB,
      (XtPointer) DUP_NOTIFY);
    XtAddCallback (dup_pass_txt, XmNvalueChangedCallback, PW_CB,
      (XtPointer) DUP_MODIFY);
    }

  if (entry_id == EAddUser || entry_id == EChangeLevel)
    {
    label = XmStringCreateLocalized ("Security Clearance Level");

    clr_slider = XtVaCreateManagedWidget ("Clearance",
      xmScaleWidgetClass, login_popup[entry_id],
      XmNtitleString, label,
      XmNfontList, flhv18,
      XmNminimum, 1,
      XmNmaximum, 9,
      XmNvalue, 1,
      XmNshowValue, True,
      XmNsensitive, True,
      XmNorientation, XmHORIZONTAL,
      XmNtopAttachment, XmATTACH_WIDGET,
      XmNtopOffset, 0,
      XmNtopWidget, entry_id == EAddUser ? dup_pass_txt : user_txt,
      XmNleftAttachment, XmATTACH_FORM,
      XmNleftOffset, 0,
      NULL);

    XmStringFree (label);

    widg = clr_slider;
    }
  else if (entry_id == EDelUser || entry_id == EListUsers)
    widg = user_txt;
  else if (entry_id == ELogin || entry_id == ELoginOrCancel)
    widg = pass_txt;
  else
    widg = dup_pass_txt;

  if (entry_id != ELogin)
    {
    cancel_pb =  XtVaCreateManagedWidget ("Cancel",
      xmPushButtonGadgetClass, login_popup[entry_id],
      XmNfontList, flhv18,
      XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
      XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
      XmNtopAttachment, XmATTACH_WIDGET,
      XmNtopWidget, widg,
      XmNtopOffset, 0,
      XmNrightAttachment, XmATTACH_FORM,
      XmNrightOffset, 0,
      NULL);

    XtAddCallback (cancel_pb, XmNactivateCallback, Cancel_CB,
      (XtPointer) entry_id);

    if (entry_id == ELoginOrCancel)
      {
      label = XmStringCreateLocalized ("Click 'Cancel' to login from KB function instead.");

      optional_lbl = XtVaCreateManagedWidget ("OptLbl",
        xmLabelWidgetClass, login_popup[entry_id],
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, cancel_pb,
        XmNrightAttachment, XmATTACH_FORM,
        XmNrightOffset, 0,
        NULL);

      XmStringFree (label);
      }
    }

  switch (entry_id)
    {
  case ELogin:
  case ELoginOrCancel:
    label = XmStringCreateLocalized ("Login");
    break;
  case EAddUser:
    label = XmStringCreateLocalized ("Store User and Password");
    break;
  case EChangePassword:
    label = XmStringCreateLocalized ("Update Password");
    break;
  case EChangeLevel:
    label = XmStringCreateLocalized ("Update Clearance Level");
    break;
  case EDelUser:
    label = XmStringCreateLocalized ("Delete this User");
    break;
  case EListUsers:
    label = XmStringCreateLocalized ("List Users");
    break;
  default:
    label = XmStringCreateLocalized ("UNKNOWN FUNCTION!");
    break;
    }

  login_pb =  XtVaCreateManagedWidget ("LoginPB",
    xmPushButtonGadgetClass, login_popup[entry_id],
    XmNfontList, flhv18,
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNlabelString, label,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, widg,
    XmNtopOffset, 0,
    XmNrightAttachment, entry_id == ELogin ? XmATTACH_FORM : XmATTACH_WIDGET,
    XmNrightWidget, cancel_pb,
    XmNrightOffset, 0,
    NULL);
  XmStringFree (label);

  switch (entry_id)
    {
  case ELogin:
    XtAddCallback (login_pb, XmNactivateCallback, Login_CB, (XtPointer) entry_id);

    XtManageChild (pass_txt);
    XtManageChild (login_pb);

    logged_in = FALSE;
    break;

  case EAddUser:
    XtAddCallback (login_pb, XmNactivateCallback, StoreUsPW_CB, (XtPointer) NULL);

    XtManageChild (pass_txt);
    XtManageChild (login_pb);
    XtManageChild (clr_slider);
    XtManageChild (dup_pass_lbl);
    XtManageChild (dup_pass_txt);
    XtManageChild (cancel_pb);
    break;

  case EChangePassword:
    XtAddCallback (login_pb, XmNactivateCallback, ChangePW_CB, (XtPointer) NULL);

    XtManageChild (pass_txt);
    XtManageChild (login_pb);
    XtManageChild (dup_pass_lbl);
    XtManageChild (dup_pass_txt);
    XtManageChild (cancel_pb);
    break;

  case EChangeLevel:
    XtAddCallback (login_pb, XmNactivateCallback, ChangeLvl_CB, (XtPointer) NULL);

    XtManageChild (login_pb);
    XtManageChild (clr_slider);
    XtManageChild (cancel_pb);
    break;

  case EDelUser:
    XtAddCallback (login_pb, XmNactivateCallback, DelUser_CB, (XtPointer) NULL);

    XtManageChild (login_pb);
    XtManageChild (cancel_pb);
    break;

  case EListUsers:
    XtAddCallback (login_pb, XmNactivateCallback, ListUsers_CB, (XtPointer) 0);

    XtManageChild (dismiss_pb);
    XtManageChild (usr_list);
    XtManageChild (usr_win);
    XtManageChild (login_pb);
    XtManageChild (cancel_pb);

    if (clearance_level >= LIST_LEV_LEV)
      {
      label = XmStringCreateLocalized ("List User Clearances");
      lev_pb =  XtVaCreateManagedWidget ("LevelPB",
        xmPushButtonGadgetClass, login_popup[entry_id],
        XmNfontList, flhv18,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, widg,
        XmNtopOffset, 0,
        XmNrightAttachment, XmATTACH_WIDGET,
        XmNrightWidget, login_pb,
        XmNrightOffset, 0,
        NULL);
      XmStringFree (label);
      XtAddCallback (lev_pb, XmNactivateCallback, ListUsers_CB, (XtPointer) 1);
      XtManageChild (lev_pb);

      if (clearance_level >= LIST_PW_LEV)
        {
        label = XmStringCreateLocalized ("List User Clearances & Passwords");
        pw_pb =  XtVaCreateManagedWidget ("PWPB",
          xmPushButtonGadgetClass, login_popup[entry_id],
          XmNfontList, flhv18,
          XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
          XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
          XmNlabelString, label,
          XmNtopAttachment, XmATTACH_WIDGET,
          XmNtopWidget, widg,
          XmNtopOffset, 0,
          XmNrightAttachment, XmATTACH_WIDGET,
          XmNrightWidget, lev_pb,
          XmNrightOffset, 0,
          NULL);
        XmStringFree (label);
        XtAddCallback (pw_pb, XmNactivateCallback, ListUsers_CB, (XtPointer) 2);
        XtManageChild (pw_pb);
        }
      }
    break;
  case ELoginOrCancel:
    XtAddCallback (login_pb, XmNactivateCallback, Login_CB, (XtPointer) entry_id);

    XtManageChild (pass_txt);
    XtManageChild (login_pb);
    XtManageChild (cancel_pb);
    XtManageChild (optional_lbl);

    logged_in = FALSE;
    break;

  default:
    break;
    }

  XtManageChild (user_txt);
  XtManageChild (login_popup[entry_id]);
}

void length_exceeded (char *string, int maxlen)
{
  XmString label, msg;
  char message[100];

  if (first_denial)
    {
    first_denial = FALSE;
    denied_box = XmCreateMessageDialog (tl, "Denied", NULL, 0);
    XtUnmanageChild (XmMessageBoxGetChild (denied_box, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild (XmMessageBoxGetChild (denied_box, XmDIALOG_HELP_BUTTON));
    XtAddCallback (XmMessageBoxGetChild (denied_box, XmDIALOG_OK_BUTTON), XmNactivateCallback, Denied_CB,
      (XtPointer) NULL);
    }

  label=XmStringCreateLocalized ("Maximum Length Exceeded");
  sprintf (message, "Maximum length of %s is %d characters - please try again.", string, maxlen);
  msg = XmStringCreateLocalized (message);

  XtVaSetValues (denied_box,
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNdialogTitle, label,
    XmNmessageString, msg,
    NULL);
  XmStringFree(label);
  XmStringFree (msg);

  XtManageChild (denied_box);
}

/*******************************************************************************************
access_denied -

  defined by macros into the following "entry" prototypes:
    void LoginFail (Widget);
    void AddUserFail (Widget, int);
    void ChangePasswordFail (Widget, int);
    void ChangeLevelFail (Widget, int);
    void DelUserFail (Widget, int);
    void ListUsersFail (Widget, int);
    void AlreadyLoggedIn (Widget);
    void LoginOrCancelFail (Widget, int);
*******************************************************************************************/

void access_denied (Widget top_level, int entry_id, int reason)
{
  XmString label, msg;

  if (first_denial)
    {
    first_denial = FALSE;
    denied_box = XmCreateMessageDialog (top_level, "Denied", NULL, 0);
    XtUnmanageChild (XmMessageBoxGetChild (denied_box, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild (XmMessageBoxGetChild (denied_box, XmDIALOG_HELP_BUTTON));
    XtAddCallback (XmMessageBoxGetChild (denied_box, XmDIALOG_OK_BUTTON), XmNactivateCallback, Denied_CB,
      (XtPointer) entry_id);
    }

    switch (entry_id)
      {
    case ELogin:
      if (!logged_in && ++login_attempts < LOGIN_TRIES)
        {
        label=XmStringCreateLocalized ("Incorrect Login");
        msg = XmStringCreateLocalized ("Login incorrect - try again.");
        retry_login = TRUE;
        }
      else
        {
        label=XmStringCreateLocalized ("Access Denied");
        msg = XmStringCreateLocalized ("You are not authorized to use this function.");
        if (mgw != (Widget) NULL) XtManageChild (mgw);
        }
      break;
    case ELoggedIn:
      label=XmStringCreateLocalized ("Redundant Login");
      msg = XmStringCreateLocalized ("You are already logged in!");
      break;
    case EAddUser:
      label=XmStringCreateLocalized ("User Creation Failed");
      switch (reason)
        {
      case USER:
        msg = XmStringCreateLocalized ("Unable to create new user due to missing username.");
        break;
      case PW:
        msg = XmStringCreateLocalized ("Unable to create new user due to missing password(s).");
        break;
      case PW_MISMATCH:
        msg = XmStringCreateLocalized ("Unable to create new user due to password verification mismatch - please try again.");
        break;
      case DUP:
        msg = XmStringCreateLocalized ("Unable to create new user due to username/password conflict.");
        break;
      default:
        msg = XmStringCreateLocalized ("Unable to create new user (REASON UNDEFINED).");
        break;
        }
      break;
    case EChangePassword:
      label=XmStringCreateLocalized ("Password Update Failed");
      switch (reason)
        {
      case PW:
        msg = XmStringCreateLocalized ("Unable to create new user due to missing password(s).");
        break;
      case OLD_PW_MISMATCH:
        msg = XmStringCreateLocalized ("Unable to change password - POSSIBLE SECURITY VIOLATION!");
        break;
      case PW_MISMATCH:
        msg = XmStringCreateLocalized ("Unable to update password due to verification mismatch - please try again.");
        break;
      default:
        msg = XmStringCreateLocalized ("Unable to update password (REASON UNDEFINED).");
        break;
        }
      break;
    case EChangeLevel:
      label=XmStringCreateLocalized ("Clearance Level Update Failed");
      switch (reason)
        {
      case USER:
        msg = XmStringCreateLocalized ("Unable to update clearance level due to missing username.");
        break;
      case SELF:
        msg = XmStringCreateLocalized ("You may NOT update your OWN clearance level!");
        break;
      case NO_SUCH_USER:
        msg = XmStringCreateLocalized ("Unable to update clearance level for nonexistent username.");
        break;
      default:
        msg = XmStringCreateLocalized ("Unable to update clearance level (REASON UNDEFINED).");
        break;
        }
      break;
    case EDelUser:
      label=XmStringCreateLocalized ("User Deletion Failed");
      switch (reason)
        {
      case USER:
        msg = XmStringCreateLocalized ("Unable to delete user due to missing username.");
        break;
      case SELF:
        msg = XmStringCreateLocalized ("You may NOT delete your OWN username!");
        break;
      case NO_SUCH_USER:
        msg = XmStringCreateLocalized ("Unable to delete nonexistent username.");
        break;
      default:
        msg = XmStringCreateLocalized ("Unable to delete user (REASON UNDEFINED).");
        break;
        }
      break;
    case EListUsers:
      label=XmStringCreateLocalized ("List Users Failed");
      switch (reason)
        {
      case PW:
        msg = XmStringCreateLocalized ("Unable to list users due to missing password.");
        break;
      case OLD_PW_MISMATCH:
        msg = XmStringCreateLocalized ("Unable to list users - invalid password - POSSIBLE SECURITY VIOLATION!");
        break;
      default:
        msg = XmStringCreateLocalized ("Unable to list users (REASON UNDEFINED).");
        break;
        }
      break;
    case ELoginOrCancel:
      switch (reason)
        {
      case FAILURE:
        if (!logged_in && ++login_attempts < LOGIN_TRIES)
          {
          label=XmStringCreateLocalized ("Incorrect Login");
          msg = XmStringCreateLocalized ("Login incorrect - try again.");
          retry_login = TRUE;
          }
        else
          {
          label=XmStringCreateLocalized ("Access Denied");
          msg = XmStringCreateLocalized ("You are not authorized to use this function.");
          if (mgw != (Widget) NULL) XtManageChild (mgw);
          }
        break;
      case CANCELLATION:
        label=XmStringCreateLocalized ("Login Deferred");
        msg = XmStringCreateLocalized ("You will have to login from within this function.");
        if (mgw != (Widget) NULL) XtManageChild (mgw);
        break;
      default:
        msg = XmStringCreateLocalized ("Unable to process login (REASON UNDEFINED).");
        break;
        }
      break;
    default:
      label=XmStringCreateLocalized ("UNKNOWN FUNCTION Failed!");
      msg = XmStringCreateLocalized ("FUNCTION UNDEFINED!");
      break;
      }

  XtVaSetValues (denied_box,
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNdialogTitle, label,
    XmNmessageString, msg,
    NULL);
  XmStringFree(label);
  XmStringFree (msg);

  XtManageChild (denied_box);
}

/*******************************************************************************************
User_CB -

  callback from user_txt widget to check that maximum username length is not exceeded
*******************************************************************************************/

void User_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  char *lcl_user_str;
  int len;
  static Boolean_t internal_change = FALSE;

  if (internal_change) return; /* ignore callback due to username clearing */

  lcl_user_str = XmTextGetString(widg);
  if (strlen (lcl_user_str) > USER_LEN)
    {
    XtFree(lcl_user_str);
/*
    XtUnmanageChild (login_popup[(int) client_data]);
*/
    internal_change = TRUE;
    XtVaSetValues (widg,
      XmNvalue, "",
      NULL);
    internal_change = FALSE;
    length_exceeded ("username", USER_LEN);
    }
  else XtFree(lcl_user_str);
}

/*******************************************************************************************
PW_CB -

  callback from XmText widgets that take password input, both to check that maximum password
  length is not exceeded and to extract a current value before hiding all input characters
  as asterisks
*******************************************************************************************/

void PW_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  char *lcl_pw_str, pwdisp[32];
  int newlen, pwlen, i, mode, pwinx;
  static Boolean_t internal_change = FALSE;

  if (internal_change) return; /* ignore callbacks due to password clearing or concealment toggling */

  switch ((int) client_data)
    {
  case NOTIFY:
    mode = NOTIFY;
    pwinx = CURRENT_PW;
    break;
  case MODIFY:
    mode = MODIFY;
    pwinx = CURRENT_PW;
    break;
  case OLD_NOTIFY:
    mode = NOTIFY;
    pwinx = OLD_PW;
    break;
  case OLD_MODIFY:
    mode = MODIFY;
    pwinx = OLD_PW;
    break;
  case DUP_NOTIFY:
    mode = NOTIFY;
    pwinx = DUP_PW;
    break;
  case DUP_MODIFY:
    mode = MODIFY;
    pwinx = DUP_PW;
    break;
  case NEW_NOTIFY:
    mode = NOTIFY;
    pwinx = NEW_PW;
    break;
  case NEW_MODIFY:
    mode = MODIFY;
    pwinx = NEW_PW;
    break;
    }

  lcl_pw_str = XmTextGetString(widg);
  if (strlen (lcl_pw_str) > PW_LEN)
    {
    XtFree(lcl_pw_str);
/*
    XtUnmanageChild (login_popup[pwinx]);
*/
    internal_change = TRUE;
    XtVaSetValues (widg,
      XmNvalue, "",
      NULL);
    pwstr[pwinx][0] = '\0';
    internal_change = FALSE;
    length_exceeded ("password", PW_LEN);
    return;
    }
  strcpy (pwdisp, lcl_pw_str);
  XtFree(lcl_pw_str);

  newlen = strlen (pwdisp);
  pwlen = strlen (pwstr[pwinx]);

  if (mode == NOTIFY) /* undo concealment (invisibly, due to immediate MODIFY callback) to permit imminent pwstr update */
  {
    internal_change = TRUE;
    XmTextReplace (widg,0,pwlen,pwstr[pwinx]);
    internal_change = FALSE;
  }
  else /* MODIFY: update pwstr from display, then reconceal displayed password */
  {
    strcpy(pwstr[pwinx],pwdisp);
    for (i=0; i<newlen; i++) pwdisp[i]='*';
    internal_change = TRUE;
    XmTextReplace (widg,0,newlen,pwdisp);
    internal_change = FALSE;
  }
}

/*******************************************************************************************
Login_From_Main -

  function to allow the main function to login using argv parameter, allowing a user to
  carry over a login from syn_view instead of having to login each time another program
  is run from the menu
*******************************************************************************************/

void Login_From_Main (char *user_str)
{
  int level;
  static char *pw = NULL;

  if (user_str[0] == 0) return;
  logged_in = TRUE;
  if (!user_pw_found (user_str, &pw, &level)) clearance_level = 0;
  else strcpy (pwstr[CURRENT_PW], pw);
  strcpy (userstr, user_str);
  clearance_level = level;
}

/*******************************************************************************************
Login_CB -

  callback from OK button to check that username and password are filled in and found in
  the user db
*******************************************************************************************/

void Login_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  char *lcl_user_str, *lcl_pw_str, user[32];
  XmString msg;
  int level, which;

  which = (int) client_data;

  XtUnmanageChild (login_popup[which]);

  lcl_user_str = XmTextGetString(XtNameToWidget(login_popup[which],"UserTxt"));
  strcpy (user, lcl_user_str);
  XtFree(lcl_user_str);

  if (!user[0])
    {
    login_failed = TRUE;
    if (which == ELogin) LoginFail (tl);
    else LoginOrCancelFail (tl, FAILURE);
    return;
    }

  if (!pwstr[CURRENT_PW][0])
    {
    login_failed = TRUE;
    if (which == ELogin) LoginFail (tl);
    else LoginOrCancelFail (tl, FAILURE);
    return;
    }

  lcl_pw_str = pwstr[CURRENT_PW];
  if (!user_pw_found (user, &lcl_pw_str, &level))
    {
    login_failed = TRUE;
    if (which == ELogin) LoginFail (tl);
    else LoginOrCancelFail (tl, FAILURE);
    return;
    }

  strcpy (userstr, user);
  logged_in = TRUE;
  clearance_level = level;
  if (rest_func[which]) (*rest_func[which]) ();
}

/*******************************************************************************************
StoreUsPW_CB -

  callback from OK button to check for non-blank fields, password verification match, and
  values that do not duplicate existing records, adding to db when successful
*******************************************************************************************/

void StoreUsPW_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  char *lcl_user_str, user[32];
  int input_level;
  XmString msg;

  XtUnmanageChild (login_popup[EAddUser]);

  lcl_user_str = XmTextGetString(XtNameToWidget(login_popup[EAddUser],"UserTxt"));
  strcpy (user, lcl_user_str);
  XtFree(lcl_user_str);

  if (!user[0])
    {
    AddUserFail (tl, USER);
    return;
    }
  if (!pwstr[NEW_PW][0] || !pwstr[DUP_PW][0])
    {
    AddUserFail (tl, PW);
    return;
    }
  if (strcmp (pwstr[NEW_PW], pwstr[DUP_PW]))
    {
    AddUserFail (tl, PW_MISMATCH);
    return;
    }
  XtVaGetValues (XtNameToWidget (login_popup[EAddUser], "Clearance"),
    XmNvalue, &input_level,
    NULL);
  if (!store_user_pw (user, pwstr[NEW_PW], FALSE, input_level))
    {
    AddUserFail (tl, DUP);
    return;
    }

  if (rest_func[EAddUser]) (*rest_func[EAddUser]) ();
}

/*******************************************************************************************
ChangeLvl_CB -

  callback from OK button to check for non-blank username field, non-identity of username
  with current user, and existence of user, updating clearance field in db when successful
*******************************************************************************************/

void ChangeLvl_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  char *lcl_user_str, user[32];
  int input_level;
  XmString msg;

  XtUnmanageChild (login_popup[EChangeLevel]);

  lcl_user_str = XmTextGetString(XtNameToWidget(login_popup[EChangeLevel],"UserTxt"));
  strcpy (user, lcl_user_str);
  XtFree(lcl_user_str);

  if (!user[0])
    {
    ChangeLevelFail (tl, USER);
    return;
    }
  if (!strcmp (user, userstr))
    {
    ChangeLevelFail (tl, SELF);
    return;
    }
  XtVaGetValues (XtNameToWidget (login_popup[EChangeLevel], "Clearance"),
    XmNvalue, &input_level,
    NULL);
  if (!store_user_pw (user, NULL, TRUE, input_level))
    {
    ChangeLevelFail (tl, NO_SUCH_USER);
    return;
    }
  if (rest_func[EChangeLevel]) (*rest_func[EChangeLevel]) ();
}

/*******************************************************************************************
DelUser_CB -

  callback from OK button to check for non-blank username field, non-identity of username
  with current user, and existence of user, deleting from db when successful
*******************************************************************************************/

void DelUser_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  char *lcl_user_str, user[32];
  int input_level;
  XmString msg;

  XtUnmanageChild (login_popup[EDelUser]);

  lcl_user_str = XmTextGetString(XtNameToWidget(login_popup[EDelUser],"UserTxt"));
  strcpy (user, lcl_user_str);
  XtFree(lcl_user_str);

  if (!user[0])
    {
    DelUserFail (tl, USER);
    return;
    }
  if (!strcmp (user, userstr))
    {
    DelUserFail (tl, SELF);
    return;
    }
  if (!delete_udb_rec (user))
    {
    DelUserFail (tl, NO_SUCH_USER);
    return;
    }
  if (rest_func[EDelUser]) (*rest_func[EDelUser]) ();
}

/*******************************************************************************************
ListUsers_CB -

  callback from Cancel button to close login_popup
*******************************************************************************************/

void ListUsers_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  char *user = NULL, **pw;
  int *level, output_level;

  XtUnmanageChild (login_popup[EListUsers]);

  if (!pwstr[OLD_PW][0])
    {
    ListUsersFail (tl, PW);
    return;
    }
  if (strcmp (pwstr[OLD_PW], pwstr[CURRENT_PW]))
    {
    ListUsersFail (tl, OLD_PW_MISMATCH);
    return;
    }

  XmListDeleteAllItems (usr_list);
  XmListSetAddMode (usr_list, TRUE);

  level = &output_level;
  pw = &user;
  output_level = (int) client_data;
  if (output_level > 0) level = NULL;
  if (output_level == 2) pw = NULL;
  user_pw_found (user, pw, level);

  XmListSetAddMode (usr_list, FALSE);

  XmListSetPos (usr_list, 1);

  XtManageChild (usr_dlg);

  if (rest_func[EListUsers]) (*rest_func[EListUsers]) ();
}


/*******************************************************************************************
ChangePW_CB -

  callback from OK button to check for non-blank password fields, non-identity of username
  with current user, and existence of user, deleting from db when successful
*******************************************************************************************/

void ChangePW_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  XmString msg;

  XtUnmanageChild (login_popup[EChangePassword]);

  if (!pwstr[OLD_PW][0] || !pwstr[NEW_PW][0] || !pwstr[DUP_PW][0])
    {
    ChangePasswordFail (tl, PW);
    return;
    }
  if (strcmp (pwstr[OLD_PW], pwstr[CURRENT_PW]))
    {
    ChangePasswordFail (tl, OLD_PW_MISMATCH);
    return;
    }
  if (strcmp (pwstr[NEW_PW], pwstr[DUP_PW]))
    {
    ChangePasswordFail (tl, PW_MISMATCH);
    return;
    }
  if (!store_user_pw (userstr, pwstr[NEW_PW], TRUE, clearance_level))
    {
    printf ("\n\n\t************************************");
    printf ("\n\n\t! ! POTENTIAL SECURITY VIOLATION ! !");
    printf ("\n\n\tAttempt to alter nonexistent record!");
    printf ("\n\n\t************************************\n\n");
    exit (1);
    }
  if (rest_func[EChangePassword]) (*rest_func[EChangePassword]) ();
}

/*******************************************************************************************
Denied_CB -

  callback from OK button to close message dialog
*******************************************************************************************/

void Denied_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  int which;

  which = (int) client_data;

  XtUnmanageChild (denied_box);
  if (retry_login)
    {
    retry_login = FALSE;
    pwstr[which][0] = '\0';
    if (which == ELogin) Login (tl, rest_func[which], mgw);
    else LoginOrCancel (tl, rest_func[which], mgw);
    }
  else if (which == ELoginOrCancel && logged_in && rest_func[which]) (*rest_func[which]) ();
}

/*******************************************************************************************
UnmapList_CB -

  callback from scrolled list to close usr_list
*******************************************************************************************/

void UnmapList_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild (usr_dlg);
}

/*******************************************************************************************
Cancel_CB -

  callback from Cancel button to close login_popup
*******************************************************************************************/

void Cancel_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  int which;

  which = (int) client_data;
  XtUnmanageChild (login_popup[which]);
  if (which == ELoginOrCancel)
  {
    logged_in = login_failed = TRUE;
    LoginOrCancelFail (tl, CANCELLATION);
  }
}

/*******************************************************************************************
store_user_pw -

  checks existence of record; handles addition of new users (when not present) and rewriting
  of clearance level (pw == NULL) or password (when record already exists)
*******************************************************************************************/

Boolean_t store_user_pw (char *user, char *pw, Boolean_t rewrite_flag, int level)
{
  int tmp_level;
  char *lcl_pw;

  if (pw == NULL && !rewrite_flag)
    {
    printf ("FATAL ERROR: NULL password passed to store_user_pw!\n");
    exit (1);
    }
 
  lcl_pw = pw;
  if (!rewrite_flag) pw = NULL; /* force user_pw_found to return TRUE on duplicate username,
                                   regardless of password, when adding new user */

  if (rewrite_flag != user_pw_found (user, &pw, &tmp_level)) return (FALSE);
  if (rewrite_flag)
    {
    if (lcl_pw == NULL)
      return (replace_lvl (user, pw, level));
    else
      return (replace_pw (user, pw));
    }
  else add_user (user, lcl_pw, level);

  return (TRUE);
}

/*******************************************************************************************
delete_udc_rec -

  handles deletion of users
*******************************************************************************************/

Boolean_t delete_udb_rec (char *user)
{
  FILE *udb, *tmp;
  char line[100], *tmp_pw = NULL;
  int tmp_level, c, udb_pos;

  if (!user_pw_found (user, &tmp_pw, &tmp_level)) return (FALSE);

  tmp = fopen (TEMPDB, "wb");
  udb = fopen (USERSDB, "rb");

  while (get_udb_line (line, udb, (udb_pos = ftell (udb))))
    if (udb_pos != file_pos) put_udb_line (line, tmp, ftell (tmp));

  fclose (tmp);
  fclose (udb);

  tmp = fopen (TEMPDB, "rb");
  udb = fopen (USERSDB, "wb");

  while ((c = getc (tmp)) != EOF) putc (c, udb);

  fclose (tmp);
  fclose (udb);

  tmp = fopen (TEMPDB, "wb");
  fclose (tmp);

  return (TRUE);
}

/*******************************************************************************************
replace_pw -

  handles rewriting of current user's record with updated password
*******************************************************************************************/

Boolean_t replace_pw (char *user, char *pw)
{
  FILE *udb;

  udb = fopen (USERSDB, "r+b");
  if (udb == NULL) return (FALSE);

  if (fseek (udb, file_pos, SEEK_SET) != 0)
    {
    fclose (udb);
    return (FALSE);
    }

  write_user_record (udb, user, pw, clearance_level);

  strcpy (pwstr[0], pw);

  return (TRUE);
}

/*******************************************************************************************
replace_lvl -

  handles rewriting of another user's record with updated clearance level
*******************************************************************************************/

Boolean_t replace_lvl (char *user, char *pw, int level)
{
  FILE *udb;

  udb = fopen (USERSDB, "r+b");
  if (udb == NULL) return (FALSE);

  if (fseek (udb, file_pos, SEEK_SET) != 0)
    {
    fclose (udb);
    return (FALSE);
    }

  write_user_record (udb, user, pw, level);

  return (TRUE);
}

/*******************************************************************************************
add_user -

  handles creation of a new user record in the db
*******************************************************************************************/

void add_user (char *user, char *pw, int level)
{
  FILE *udb;

  udb = fopen (USERSDB, "r+b");
  fseek (udb, 0L, SEEK_END);

  write_user_record (udb, user, pw, level);
}

/*******************************************************************************************
user_pw_found -

  checks for existence of db, using current login data to create from scratch, if necessary,
  with current user at level 9; otherwise returns TRUE if user found (*pw == NULL) or user/
  password combination found (*pw != NULL) [dumps list to stdout (for now) if user == NULL]
*******************************************************************************************/

Boolean_t user_pw_found (char *user, char **pw, int *level)
{
  FILE *udb;
  char line[100], lcl_user[32], list_str[100];
  int i, lcl_lvl, lcl_file_pos = 0;
  XmString lbl_str;
  static char lcl_pw[32];

  udb = fopen (USERSDB, "rb");
  if (udb == NULL) /* new db - assume administrator status */
    {
    udb = fopen (USERSDB, "wb");
    fclose (udb);
    add_user (user, *pw, 9);
    *level = 9;
    return(TRUE);
    }

  while (get_udb_line (line, udb, lcl_file_pos))
    {
    for (i=0; i<=USER_LEN; i++) lcl_user[i] = line[i];
    for (; i<=USER_LEN+PW_LEN+1; i++) lcl_pw[i-USER_LEN-1] = line[i];
    lcl_lvl = line[i];

    if (user == NULL)
      {
      sprintf (list_str, "User Name = \"%s\"", lcl_user);
      if (level == NULL)
        {
        sprintf (list_str + strlen (list_str), " - Clearance = %d", lcl_lvl);
        if (pw == NULL) sprintf (list_str + strlen (list_str), " - Password = \"%s\"", lcl_pw);
        }

      lbl_str = XmStringCreateLocalized (list_str);
      XmListAddItem (usr_list, lbl_str, 0);
      XmStringFree (lbl_str);
      }

    else if (!strcmp (user, lcl_user) && (*pw == NULL || !strcmp (*pw, lcl_pw)))
      {
      *level = lcl_lvl;
      if (*pw == NULL) *pw = lcl_pw;
      file_pos = lcl_file_pos;
      fclose (udb);
      return (TRUE);
      }

    lcl_file_pos = ftell (udb);
    }

  if (level != NULL) *level = 0;

  fclose (udb);

  return(FALSE);
}

/*******************************************************************************************
next_char -

  encrypts (before) or decrypts (!before) a character on the basis of a dynamic seed value
  and the file position of the record and returns the encrypted or decrypted value
*******************************************************************************************/

int next_char (U8_t *line, int pos, int fpos, Boolean_t before)
{
  static int seed;
  int retval, temp_seed;

  if (pos == 0)
    seed = SEED ^ fpos;

  temp_seed = seed;
  if (before) temp_seed ^= (int) line[pos];

  retval = (int) line[pos] ^ seed;

  seed = temp_seed;
  if (!before) seed ^= retval;

  return (retval);
}

/*******************************************************************************************
write_user_record -

  constructs a line from filled-out user and password fields plus a one-byte clearance level
  and encrypts the line before writing it to the db (followed by a newline character)
*******************************************************************************************/

void write_user_record (FILE *udb, char *user, char *pw, int level)
{
  char line[100], lcl_user[32], lcl_pw[32];
  int i, lcl_file_pos;

  lcl_file_pos = ftell (udb);

  strcpy (lcl_user, user);
  fill_udb_string (lcl_user, USER_LEN);
  strcpy (lcl_pw, pw);
  fill_udb_string (lcl_pw, PW_LEN);

  for (i=0; i<=USER_LEN; i++) line[i] = lcl_user[i];
  for (; i<=USER_LEN+PW_LEN+1; i++) line[i] = lcl_pw[i-USER_LEN-1];
  line[i] = level;

/*
  for (i=0; i<USER_LEN+PW_LEN+3; i++) putc (next_char ((U8_t *) line, i, lcl_file_pos, TRUE), udb);
  putc ('\n', udb);
*/

  put_udb_line (line, udb, lcl_file_pos);

  fclose (udb);
}

/*******************************************************************************************
fill_udb_string -

  pads out past the zero-termination to the full length+1 of a string to make the entire
  record reproducible
*******************************************************************************************/

void fill_udb_string (char *string, int length)
{
  int i;

  for (i = strlen (string) + 1; i <= length; i++) string[i] = (i + SEED) % 256;
}

/*******************************************************************************************
get_udb_line -

  reads line from current file position with decryption
*******************************************************************************************/

char *get_udb_line (char *line, FILE *f, int parm_file_pos)
{
  int c, i;

  if ((c = getc (f)) == EOF) return (NULL);

  for (i=0; i<USER_LEN+PW_LEN+3; i++)
    {
    line[i] = c;
    c = getc (f);
    }

  line[i]=0; /* skip newline */

  for (i=0; i<USER_LEN+PW_LEN+3; i++) line[i] = next_char ((U8_t *) line, i, parm_file_pos, FALSE);

  return (line);
}

/*******************************************************************************************
put_udb_line -

  writes line to current file position with encryption
*******************************************************************************************/

void put_udb_line (char *line, FILE *f, int parm_file_pos)
{
  int i;

  for (i=0; i<USER_LEN+PW_LEN+3; i++) putc (next_char ((U8_t *) line, i, parm_file_pos, TRUE), f);
  putc ('\n', f);
}
