/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              PTXCESS.C
*
*    This module provides for the display and editing of the posttransform
*    condition type XCESS, which determines the existence of a functional
*    group (relative to a number of instances) within a given node.
*
*  Creation Date:
*
*       16-Feb-2000
*
*  Authors:
*
*    Jerry Miller
*
*  Modification History, reverse chronological
*
******************************************************************************/

#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
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

#ifdef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_SLING2XTR_
#include "sling2xtr.h"
#endif

#ifndef _H_PST_
#include "pst.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_SGLSHOT_
#include "sglshot.h"
#endif

#ifndef _H_APP_RESRC_
#include "app_resrc.h"
#endif

#ifndef _H_DSP_
#include "dsp.h"
#endif

#ifndef _H_MOL_VIEW_
#include "mol_view.h"
#endif

#ifndef _H_RXN_VIEW_
#include "rxn_view.h"
#endif

#ifndef _H_SSHOT_VIEW_
#include "sshot_view.h"
#endif

#ifndef _H_ATOMSYM_
#include "atomsym.h"
#endif

#ifndef _H_EXTERN_
#include "extern.h"
#endif

extern int cond_type;
extern Widget pttl, ptformdg, condform, condpb[2], emptytextmsg, nodexceedmsg, invalidfgmsg, condlbl;
extern Boolean_t cond_add_canceled;

static Widget xcessform, xcesslbl[4], xcesstxt[2], xcessdummy, xcessmenu, xcessfgpb, xcessfglbl[2];
static Condition_t *glob_cond;
static char glob_condstr[1024];
static Boolean_t *glob_nu, new_cond;
static int menusel[25], fg;

void Post_Cond_Put (Condition_t *);
void PTSetCondname (int, char *);

void getfg (int);
void Xcess_Display (char *, int, int, int);
void XcessOpt_CB (Widget, XtPointer, XtPointer);
void CondExit_CB (Widget, XtPointer, XtPointer);
void XcessExit_CB (Widget, XtPointer, XtPointer);
void XcessVal_CB (Widget, XtPointer, XtPointer);
void XcessPB_CB (Widget, XtPointer, XtPointer);
void MtTxtMsg_Dismiss_CB (Widget, XtPointer, XtPointer);

void PTXcess (Condition_t *cond, Boolean_t *node_used, int condnum, Boolean_t new)
{
  char *condstr, tmpstr[128], dummyname[25], menuname[25];
  String_t string;
  XmString label[42], title;
  int i, sel, node, amt;
  XmFontList flhv18;
  XmFontListEntry helv18;

  cond_type = PT_TYPE_FG_XCESS;

/*
  if (cond == NULL)
*/
  if (new)
    {
    new_cond = TRUE;
/*
    cond = (Condition_t *) malloc (sizeof (Condition_t));
*/
    condstr = (char *) malloc (100);
    strcpy (condstr, "1120010100");
    String_Alloc_Put (string, 100);
    String_Value_Put (string, condstr);
    }
  else
    {
    new_cond = FALSE;
    condition_import (cond, &string, NULL);
    condstr = String_Value_Get (string);
    }

  glob_cond = cond;
  strcpy (glob_condstr, condstr);
  glob_nu = node_used;

  sscanf (glob_condstr + 3, "%3d%2d%2d", &fg, &node, &amt);

  String_Destroy (string);

  helv18 = XmFontListEntryLoad (XtDisplay (pttl), "-*-helvetica-bold-r-normal-*-18-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);

  flhv18 = XmFontListAppendEntry (NULL, helv18);

  PTSetCondname (condnum, tmpstr);

  title = XmStringCreateLocalized (tmpstr);
  XtVaSetValues (condlbl,
    XmNlabelString, title,
    NULL);
  XmStringFree (title);

  xcessform = XmCreateForm (condform, "xcessform", NULL, 0);
  XtVaSetValues (xcessform,
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    XmNfractionBase, 200,
    NULL);

  title = XmStringCreateLocalized ("The functional group ");
  xcessfglbl[0] = XtVaCreateWidget ("xcessfglbl0",
               xmLabelWidgetClass, xcessform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_FORM,
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized (fgname[fg]);
  xcessfgpb = XtVaCreateWidget ("xcessfgpb",
               xmPushButtonGadgetClass, xcessform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_FORM,
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_NONE,
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_WIDGET,
               XmNleftWidget, xcessfglbl[0],
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  XtAddCallback (xcessfgpb, XmNactivateCallback, XcessPB_CB, (XtPointer) &fg);

  title = XmStringCreateLocalized ("The functional group ");
  xcessfglbl[1] = XtVaCreateWidget ("xcessfglbl1",
               xmLabelWidgetClass, xcessform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNtopWidget, xcessfgpb,
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, xcessfgpb,
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_WIDGET,
               XmNrightWidget, xcessfgpb,
               NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("IS ");
  label[1] = XmStringCreateLocalized ("IS NOT ");
  label[2] = XmStringCreateLocalized ("OCCURS AT LEAST ... ");
  label[3] = XmStringCreateLocalized ("DOES NOT OCCUR AT LEAST ... ");

  xcessdummy = XmVaCreateSimplePulldownMenu (xcessform, "_xcesspulldown0", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  if (glob_condstr[0] == '0') sel = 1;
  else sel = 0;

  xcessmenu = XmVaCreateSimpleOptionMenu (xcessform, "xcessmenu0", title, '\0', sel, XcessOpt_CB,
      XmNsubMenuId, xcessdummy,
      XmNlabelFontList, flhv18,
      XmNbuttonFontList, flhv18,
      XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
      SAR_CLRI_WHITE),
      XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
      SAR_CLRI_BLACK),
      XmVaPUSHBUTTON, label[0], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[1], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[2], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[3], '\0', NULL, NULL,
      NULL);

  XmStringFree (title);
  XmStringFree (label[0]);
  XmStringFree (label[1]);
  XmStringFree (label[2]);
  XmStringFree (label[3]);

  XtVaSetValues (xcessmenu,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, xcessfglbl[1],
    XmNleftPosition, 25,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("PRESENT IN THE PART OF THE TARGET MOLECULE DEFINED BY");
  xcesslbl[0] = XtVaCreateWidget ("xcesslbl0",
               xmLabelWidgetClass, xcessform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, xcessfglbl[1],
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, xcessmenu,
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_WIDGET,
               XmNleftWidget, xcessmenu,
               NULL);
  XmStringFree (title);

  xcesstxt[0] = XmCreateTextField (xcessform, "xcesstxt0", NULL, 0);

  XtVaSetValues (xcesstxt[0],
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, xcessfglbl[1],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, xcessmenu,
    XmNrightPosition, 100,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized (" TIMES WITHIN THE PART OF THE TARGET MOLECULE DEFINED BY");
  xcesslbl[1] = XtVaCreateWidget ("xcesslbl1",
               xmLabelWidgetClass, xcessform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, xcessfglbl[1],
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, xcesstxt[0],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_WIDGET,
               XmNleftWidget, xcesstxt[0],
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_NONE,
               NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized ("NODE ");
  xcesslbl[2] = XtVaCreateWidget ("xcesslbl2",
               xmLabelWidgetClass, xcessform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, xcessmenu,
               XmNleftPosition, 25,
               XmNleftAttachment, XmATTACH_POSITION,
               NULL);
  XmStringFree (title);

  xcesstxt[1] = XmCreateTextField (xcessform, "xcesstxt1", NULL, 0);

  XtVaSetValues (xcesstxt[1],
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, xcessmenu,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, xcesslbl[2],
    XmNrightPosition, 100,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("NODE ");
  xcesslbl[3] = XtVaCreateWidget ("xcesslbl3",
               xmLabelWidgetClass, xcessform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNtopWidget, xcesstxt[1],
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, xcesstxt[1],
               XmNleftPosition, 25,
               XmNleftAttachment, XmATTACH_POSITION,
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_WIDGET,
               XmNrightWidget, xcesstxt[1],
               NULL);
  XmStringFree (title);

  for (i = 0; i < 2; i++)
    XtAddCallback (xcesstxt[i], XmNvalueChangedCallback, XcessVal_CB, (XtPointer) i);

  Xcess_Display (glob_condstr, node, fg, amt);
}

void Xcess_Display (char *condstr, int node, int fg, int amt)
{
  int i, sel;
  char tmpstr[5];

  XtManageChild (xcessfglbl[0]);
  XtManageChild (xcessfgpb);
  XtManageChild (xcessfglbl[1]);
  XtManageChild (xcessmenu);
  XtManageChild (xcesslbl[0]);

  if (amt != 0)
    {
    sprintf (tmpstr, "%02d", amt);
    XtVaSetValues (xcesstxt[0],
      XmNvalue, tmpstr,
      NULL);
    }

  XtManageChild (xcesstxt[0]);
  XtManageChild (xcesslbl[1]);
  XtManageChild (xcesslbl[2]);

  sprintf (tmpstr, "%02d", node);
  XtVaSetValues (xcesstxt[1],
    XmNvalue, tmpstr,
    NULL);

  XtManageChild (xcesstxt[1]);
  XtManageChild (xcesslbl[3]);

  XtManageChild (xcessform);
  XtManageChild (ptformdg);

  XtUnmanageChild (xcessfglbl[0]);
  XtUnmanageChild (xcesslbl[2]);
  if (amt == 0)
    {
    XtUnmanageChild (xcesstxt[0]);
    XtUnmanageChild (xcesslbl[1]);
    }
  else XtUnmanageChild (xcesslbl[0]);
}

void XcessOpt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *wname, tmpstr[128];
  int sel, which, fg, menupos;

  wname = XtName (XtParent (w));
  sel = (int) client_data;

    glob_condstr[0] = sel % 2 == 0 ? '1' : '0';
    if (sel < 2)
      {
      XtManageChild (xcesslbl[0]);
      XtUnmanageChild (xcesstxt[0]);
      XtUnmanageChild (xcesslbl[1]);
      strcpy (glob_condstr + 8, "00");
      }
    else
      {
      XtManageChild (xcesstxt[0]);
      XtManageChild (xcesslbl[1]);
      XtUnmanageChild (xcesslbl[0]);
      }
}

void XcessExit_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  String_t string;
  char tmpstr[128], tmpstr2[128], savestr[128], *ts, *ts2, *ts3, *comma, *space, *newstr;
  int i, fg, node, amt;

  if ((Boolean_t) (int) client_data)
    {
/*
    strncpy (tmpstr, glob_condstr + 3, 3);
    tmpstr[3] = '\0';
    sscanf (tmpstr, "%d", &fg);
    if (fg == 0 || fgname[fg - 1][0] == '\0')
      {
      XtManageChild (invalidfgmsg);
      return;
      }
*/
/* get final results of text field */
    if (XtIsManaged (xcesstxt[0]))
      {
      XtVaGetValues (xcesstxt[0],
        XmNvalue, &ts,
        NULL);

      if (ts[0] == '\0')
        {
        XtManageChild (emptytextmsg);
        return;
        }
      sscanf (ts, "%d", &amt);
      }
    else amt = 0;

    XtVaGetValues (xcesstxt[1],
      XmNvalue, &ts2,
      NULL);

    if (ts2[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }
    sscanf (ts2, "%d", &node);

/* incorporate into string */
    sprintf (glob_condstr + 6, "%02d%02d", node, amt);

    newstr = (char *) malloc (strlen (glob_condstr) + 1);
    strcpy (newstr, glob_condstr);
    String_Value_Put (string, newstr);
    String_Length_Put (string, strlen (newstr));
    String_Alloc_Put (string, strlen (newstr) + 1);

    condition_export (glob_cond, &string, glob_nu);
    }
  else if (new_cond) cond_add_canceled = TRUE;

  XtUnmanageChild (ptformdg);
  XtDestroyWidget (xcessform);
  FG_Form_Close();
}

void XcessVal_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *lcl_node_str, node_str[128];
  int len, which, i;
  Boolean_t ok;
  static Boolean_t internal_change = FALSE;

  if (internal_change) return; /* ignore callback due to username clearing */

  lcl_node_str = XmTextGetString(w);

  len = strlen (lcl_node_str);

  which = (int) client_data;

  for (i = 0, ok = TRUE; i < len && ok; i++)
    {
    if (i == 2) ok = FALSE;
    else if (which == 2)
      ok = (lcl_node_str[i] >= 'A' && lcl_node_str[i] <= 'Z') || (lcl_node_str[i] >= 'a' && lcl_node_str[i] <= 'z');
    else
      ok = lcl_node_str[i] >= '0' && lcl_node_str[i] <= '9';
    if (!ok) i--; /* adjust prior to increment */
    }

  if (!ok)
    {
    strcpy (node_str, lcl_node_str);
    len = strlen (node_str);
    node_str[len - 1] = '\0';
    XtFree(lcl_node_str);
    internal_change = TRUE;
    XmTextReplace (w,0,len,node_str);
    internal_change = FALSE;
    XBell (XtDisplay (w), 10);
    }
  else XtFree(lcl_node_str);
}

void XcessPB_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  FG_Form_Create (XtParent (XtParent (XtParent (XtParent (w)))), *(int *) client_data, getfg);
}

void getfg (int fgnum)
{
  XmString title;
  char tmpstr[5];

  fg = fgnum;

  title = XmStringCreateLocalized (fgname[fg]);
  XtVaSetValues (xcessfgpb,
    XmNlabelString, title,
    NULL);
  XmStringFree (title);
  sprintf (tmpstr, "%03d", fg);
  strncpy (glob_condstr + 3, tmpstr, 3);
}
