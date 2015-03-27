/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              PTCSEQ.C
*
*    This module provides for the display and editing of the posttransform
*    conditions for determining whether 2 or more nodes are equivalent from
*    either a constitutional or a stereochemical perspective.
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

#ifndef _H_POSTTEST_
#include "posttest.h"
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

extern int cond_type;
extern Widget pttl, ptformdg, condform, condpb[2], emptytextmsg, nodexceedmsg, invalidatommsg, invalidfgmsg, condlbl;
extern Boolean_t cond_add_canceled;

static Widget cseqform, cseqlbl, cseqtxt, cseqmenu[3], cseqdummy[3];
static Condition_t *glob_cond;
static char glob_condstr[1024];
static Boolean_t *glob_nu, new_cond;

void Post_Cond_Put (Condition_t *);
void PTSetCondname (int, char *);

void FixNodeList (char *, char *);
void CSEq_Display (char *, int);
void CSEqOpt_CB (Widget, XtPointer, XtPointer);
void CondExit_CB (Widget, XtPointer, XtPointer);
void CSEqExit_CB (Widget, XtPointer, XtPointer);
void NodeList_CB (Widget, XtPointer, XtPointer);
void MtTxtMsg_Dismiss_CB (Widget, XtPointer, XtPointer);
Boolean_t BadNodeList (char *);

void PTCSEq (Condition_t *cond, Boolean_t *node_used, int condnum, Boolean_t new)
{
  char *condstr, tmpstr[128];
  String_t string;
  XmString label[19], title;
  int i, sel, nnodes;
  XmFontList flhv18;
  XmFontListEntry helv18;
  Boolean_t stereo, goal, disconn;

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
    strcpy (condstr, "113020102");
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

  cseqform = XmCreateForm (condform, "cseqform", NULL, 0);
  XtVaSetValues (cseqform,
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

  title = XmStringCreateLocalized ("The nodes");
  cseqlbl = XtVaCreateWidget ("cseqlbl0",
               xmLabelWidgetClass, cseqform,
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

  cseqtxt = XmCreateTextField (cseqform, "cseqtxt0", NULL, 0);

  XtVaSetValues (cseqtxt,
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, cseqlbl,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftPosition, 25,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("ARE");
  label[1] = XmStringCreateLocalized ("ARE NOT");

  cseqdummy[0] = XmVaCreateSimplePulldownMenu (cseqform, "_cseqpulldown0", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  if (glob_condstr[0] == '0') sel = 1;
  else sel = 0;

  cseqmenu[0] = XmVaCreateSimpleOptionMenu (cseqform, "cseqmenu0", title, '\0', sel, CSEqOpt_CB,
      XmNsubMenuId, cseqdummy[0],
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

  XtVaSetValues (cseqmenu[0],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, cseqtxt,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("CONSTITUTIONALLY EQUIVALENT");
  label[1] = XmStringCreateLocalized ("STEREOCHEMICALLY EQUIVALENT");

  cseqdummy[1] = XmVaCreateSimplePulldownMenu (cseqform, "_cseqpulldown1", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  strncpy (tmpstr, glob_condstr + 1, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", &cond_type);
  strncpy (tmpstr, glob_condstr + 3, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", &nnodes);
  disconn = cond_type > 16;
  stereo = cond_type % 2 == 0;
  goal = nnodes > 50 || disconn;
  if (goal && !disconn) nnodes -= 50;

  sel = stereo ? 1 : 0;

  cseqmenu[1] = XmVaCreateSimpleOptionMenu (cseqform, "cseqmenu1", title, '\0', sel, CSEqOpt_CB,
      XmNsubMenuId, cseqdummy[1],
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

  XtVaSetValues (cseqmenu[1],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, cseqtxt,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, cseqmenu[0],
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("IN THE SUBGOAL PATTERN");
  label[1] = XmStringCreateLocalized ("IN THE GOAL PATTERN");
  label[2] = XmStringCreateLocalized ("WHEN DISCONNECTED FROM THE GOAL PATTERN");

  cseqdummy[2] = XmVaCreateSimplePulldownMenu (cseqform, "_cseqpulldown2", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  sel = disconn ? 2 : goal ? 1 : 0;

  cseqmenu[2] = XmVaCreateSimpleOptionMenu (cseqform, "cseqmenu2", title, '\0', sel, CSEqOpt_CB,
      XmNsubMenuId, cseqdummy[2],
      XmNbuttonFontList, flhv18,
      XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
      SAR_CLRI_WHITE),
      XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
      SAR_CLRI_BLACK),
      XmVaPUSHBUTTON, label[0], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[1], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[2], '\0', NULL, NULL,
      NULL);

  XmStringFree (title);
  XmStringFree (label[0]);
  XmStringFree (label[1]);
  XmStringFree (label[2]);

  XtVaSetValues (cseqmenu[2],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, cseqmenu[0],
    XmNleftPosition, 25,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);

  XtAddCallback (cseqtxt, XmNvalueChangedCallback, NodeList_CB, (XtPointer) i);

  CSEq_Display (glob_condstr, nnodes);
}

void CSEq_Display (char *v, int nnodes)
{
  int i, sel;
  char nodelist[256], tmpstr[5];

  XtManageChild (cseqlbl);

  strncpy (nodelist, v + 5, 2);
  nodelist[2] = '\0';
  for (i = 1; i < nnodes; i++)
    {
    strcat (nodelist, ", ");
    strncat (nodelist, v + 5 + 2 * i, 2);
    nodelist[4 * i + 2] = '\0';
    }
  XtVaSetValues (cseqtxt,
    XmNvalue, nodelist,
    NULL);
  XtManageChild (cseqtxt);

  XtManageChild (cseqmenu[0]);

  XtManageChild (cseqmenu[1]);

  XtManageChild (cseqmenu[2]);

  XtManageChild (cseqform);
  XtManageChild (ptformdg);
}

void CSEqOpt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *wname, tmpstr[128], *ts, *comma, *space;
  int sel, type, nnodes;

  wname = XtName (XtParent (w));
  sel = (int) client_data;
  strncpy (tmpstr, glob_condstr + 1, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", &type);
  strncpy (tmpstr, glob_condstr + 3, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", &nnodes);

  if (strstr (wname, "0") != NULL) glob_condstr[0] = sel == 0 ? '1' : '0';
  else if (strstr (wname, "1") != NULL)
    {
    if (sel == 0 && !(type & 1)) type--;
    else if (sel == 1 && (type & 1)) type++;
    sprintf (tmpstr, "%02d", type);
    strncpy (glob_condstr + 1, tmpstr, 2);
    }
  else if (strstr (wname, "2") != NULL)
    {
    switch (sel)
      {
    case 0:
      if (nnodes > 50) nnodes -= 50;
      if (type > 16) type -= 4;
      break;
    case 1:
      if (nnodes <= 50) nnodes += 50;
      if (type > 16) type -= 4;
      break;
    case 2:
      if (type < 17)
        {
        type += 4;
        if (nnodes > 50) nnodes -= 50;
        }
      break;
      }
    }
    sprintf (tmpstr, "%02d%02d", type, nnodes);
    strncpy (glob_condstr + 1, tmpstr, 4);
}

void CSEqExit_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  String_t string;
  char tmpstr[128], tmpstr2[128], savestr[128], *ts, *ts2, *comma, *space, *newstr;
  int i, nnodes;
  Boolean_t goal;

  if ((Boolean_t) (int) client_data)
    {
/* first get final results of text fields */
    XtVaGetValues (cseqtxt,
      XmNvalue, &ts,
      NULL);
    strcpy (tmpstr, ts);
    ts = tmpstr;
    do
      {
      comma = strstr (ts, ",");
      space = strstr (ts, " ");
      if (comma != NULL && (space == NULL || comma < space)) FixNodeList (ts, comma);
      else if (space != NULL) FixNodeList (ts, space);
      }
    while (comma != NULL || space != NULL);

    FixNodeList (ts, ts + strlen (ts));

    if (ts[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }

/* incorporate into string */
    sscanf (glob_condstr + 3, "%2d", &nnodes);
    goal = nnodes > 50;
    nnodes = strlen (ts) / 2;
    if (nnodes >= MX_NODES)
      {
      XtManageChild (nodexceedmsg);
      return;
      }
    nnodes += goal ? 50 : 0;
    sprintf (glob_condstr + 3, "%02d%s", nnodes, ts);

    newstr = (char *) malloc (strlen (glob_condstr) + 1);
    strcpy (newstr, glob_condstr);
    String_Value_Put (string, newstr);
    String_Length_Put (string, strlen (newstr));
    String_Alloc_Put (string, strlen (newstr) + 1);

    condition_export (glob_cond, &string, glob_nu);
    }
  else if (new_cond) cond_add_canceled = TRUE;

  XtUnmanageChild (ptformdg);
  XtDestroyWidget (cseqform);
}
