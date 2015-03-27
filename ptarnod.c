/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              PTARNOD.C
*
*    This module provides for the display and editing of the aromatic subsitution
*    posttransform condition subtype for determining whether a given node is in
*    an isolated aromatic ring.
*
*  Creation Date:
*
*       23-Feb-2000
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

extern int cond_type, cond_subtype;
extern Widget pttl, ptformdg, condform, condpb[2], emptytextmsg, nodexceedmsg, invalidatommsg, condlbl;
extern Boolean_t cond_add_canceled;

static Widget arnodform, arnodlbl[3], arnodtxt, arnoddummy, arnodmenu;
static Condition_t *glob_cond;
static char glob_condstr[1024];
static Boolean_t *glob_nu, new_cond;

void Post_Cond_Put (Condition_t *);
void PTSetCondname (int, char *);

void ArNod_Display (char *, int);
void ArNodOpt_CB (Widget, XtPointer, XtPointer);
void CondExit_CB (Widget, XtPointer, XtPointer);
void ArNode_CB (Widget, XtPointer, XtPointer);
void ArNodExit_CB (Widget, XtPointer, XtPointer);
void MtTxtMsg_Dismiss_CB (Widget, XtPointer, XtPointer);

void PTArNod (Condition_t *cond, Boolean_t *node_used, int condnum, Boolean_t new)
{
  char *condstr, tmpstr[128];
  String_t string;
  XmString label[19], title;
  int i, sel, node;
  XmFontList flhv18;
  XmFontListEntry helv18;

  cond_type = PT_TYPE_AROMSUB;
  cond_subtype = 1;

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
    strcpy (condstr, "1201101");
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

  sscanf (glob_condstr + 5, "%d", &node);

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

  arnodform = XmCreateForm (condform, "arnodform", NULL, 0);
  XtVaSetValues (arnodform,
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

  title = XmStringCreateLocalized ("In the subgoal pattern, there ");
  label[0] = XmStringCreateLocalized ("EXISTS");
  label[1] = XmStringCreateLocalized ("DOES NOT EXIST");

  arnoddummy = XmVaCreateSimplePulldownMenu (arnodform, "_arnodpulldown0", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  if (glob_condstr[0] == '0') sel = 1;
  else sel = 0;

  arnodmenu = XmVaCreateSimpleOptionMenu (arnodform, "arnodmenu", title, '\0', sel, ArNodOpt_CB,
      XmNsubMenuId, arnoddummy,
      XmNlabelFontList, flhv18,
      XmNbuttonFontList, flhv18,
      XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
      SAR_CLRI_WHITE),
      XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
      SAR_CLRI_BLACK),
      XmVaPUSHBUTTON, label[0], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[1], '\0', NULL, NULL,
      NULL);

  XmStringFree (title);
  XmStringFree (label[0]);
  XmStringFree (label[1]);

  XtVaSetValues (arnodmenu,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    NULL);

  title = XmStringCreateLocalized ("an isolated carbocyclic aromatic ring containing");
  arnodlbl[0] = XtVaCreateWidget ("arnodlbl0",
               xmLabelWidgetClass, arnodform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, arnodmenu,
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized ("NODE ");
  arnodlbl[1] = XtVaCreateWidget ("arnodlbl1",
               xmLabelWidgetClass, arnodform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, arnodlbl[0],
               XmNleftPosition, 25,
               XmNleftAttachment, XmATTACH_POSITION,
               NULL);
  XmStringFree (title);

  arnodtxt = XmCreateTextField (arnodform, "arnodtxt0", NULL, 0);

  XtVaSetValues (arnodtxt,
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, arnodlbl[0],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, arnodlbl[1],
    XmNrightPosition, 75,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("NODE ");
  arnodlbl[2] = XtVaCreateWidget ("arnodlbl2",
               xmLabelWidgetClass, arnodform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNtopWidget, arnodtxt,
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, arnodtxt,
               XmNleftPosition, 25,
               XmNleftAttachment, XmATTACH_POSITION,
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_WIDGET,
               XmNrightWidget, arnodtxt,
               NULL);
  XmStringFree (title);

  XtAddCallback (arnodtxt, XmNvalueChangedCallback, ArNode_CB, (XtPointer) i);

  ArNod_Display (glob_condstr, node);
}

void ArNod_Display (char *v, int node)
{
  int i, sel;
  char tmpstr[5];

  XtManageChild (arnodmenu);

  XtManageChild (arnodlbl[0]);
  XtManageChild (arnodlbl[1]);

  sprintf (tmpstr, "%02d", node);
  XtVaSetValues (arnodtxt,
    XmNvalue, tmpstr,
    NULL);
  XtManageChild (arnodtxt);

  XtManageChild (arnodlbl[2]);

  XtManageChild (arnodform);
  XtManageChild (ptformdg);

  XtUnmanageChild (arnodlbl[1]);
}

void ArNodOpt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  int sel;

  sel = (int) client_data;

  glob_condstr[0] = sel == 0 ? '1' : '0';
}

void ArNodExit_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  String_t string;
  char tmpstr[128], tmpstr2[128], savestr[128], *ts, *ts2, *ts3, *comma, *space, *newstr;
  int i, atom, node, dist;

  if ((Boolean_t) (int) client_data)
    {
/* get final results of text field */
    XtVaGetValues (arnodtxt,
      XmNvalue, &ts,
      NULL);

    if (ts[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }
    sscanf (ts, "%d", &node);

/* incorporate into string */
    sprintf (glob_condstr + 5, "%02d", node);

    newstr = (char *) malloc (strlen (glob_condstr) + 1);
    strcpy (newstr, glob_condstr);
    String_Value_Put (string, newstr);
    String_Length_Put (string, strlen (newstr));
    String_Alloc_Put (string, strlen (newstr) + 1);

    condition_export (glob_cond, &string, glob_nu);

/* add to postform2.c later
    if (new_cond)
      Post_Cond_Add (glob_cond);
*/
    }
  else if (new_cond) cond_add_canceled = TRUE;
/*
    {
    free (glob_cond);
    glob_cond = NULL;
    }
*/

  XtUnmanageChild (ptformdg);
  XtDestroyWidget (arnodform);
}

void ArNode_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *lcl_node_str, node_str[128];
  int len, i;
  Boolean_t ok;
  static Boolean_t internal_change = FALSE;

  if (internal_change) return; /* ignore callback due to username clearing */

  lcl_node_str = XmTextGetString(w);

  len = strlen (lcl_node_str);

  for (i = 0, ok = TRUE; i < len && ok; i++)
    {
    if (i == 2) ok = FALSE;
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