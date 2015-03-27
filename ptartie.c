/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              PTARTIE.C
*
*    This module provides for the display and editing of the aromatic subsitution
*    posttransform condition subtype for determining how many nodes in an aromatic
*    ring compete with (i.e., have similar activation as) the node in question.
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

static Widget artieform, artielbl[2], artietxt, artiedummy[4], artiemenu[4];
static Condition_t *glob_cond;
static char glob_condstr[1024];
static Boolean_t *glob_nu, new_cond;

void Post_Cond_Put (Condition_t *);
void PTSetCondname (int, char *);

void ArTie_Display (char *, int);
void ArTieOpt_CB (Widget, XtPointer, XtPointer);
void CondExit_CB (Widget, XtPointer, XtPointer);
void ArNode_CB (Widget, XtPointer, XtPointer);
void ArTieExit_CB (Widget, XtPointer, XtPointer);
void MtTxtMsg_Dismiss_CB (Widget, XtPointer, XtPointer);

void PTArTie (Condition_t *cond, Boolean_t *node_used, int condnum, Boolean_t new)
{
  char *condstr, tmpstr[128];
  String_t string;
  XmString label[19], title;
  int i, sel, node, value;
  XmFontList flhv18;
  XmFontListEntry helv18;
  Boolean_t arh;

  cond_type = PT_TYPE_AROMSUB;
  cond_subtype = 5;

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
    strcpy (condstr, "1205101> 1");
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

  arh = glob_condstr[4] == '1';
  sscanf (glob_condstr + 5, "%02d", &node);
  strcpy (tmpstr, glob_condstr + 7);

  value = tmpstr[2] - '0';
  tmpstr[2] = '\0';

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

  artieform = XmCreateForm (condform, "artieform", NULL, 0);
  XtVaSetValues (artieform,
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

  title = XmStringCreateLocalized ("The number of");
  artielbl[0] = XtVaCreateWidget ("artielbl0",
               xmLabelWidgetClass, artieform,
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

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("HYDROGEN-BEARING NODES");
  label[1] = XmStringCreateLocalized ("NODES");

  artiedummy[0] = XmVaCreateSimplePulldownMenu (artieform, "_artiepulldown0", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  if (arh) sel = 0;
  else sel = 1;

  artiemenu[0] = XmVaCreateSimpleOptionMenu (artieform, "artiemenu0", title, '\0', sel, ArTieOpt_CB,
      XmNsubMenuId, artiedummy[0],
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

  XtVaSetValues (artiemenu[0],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, artielbl[0],
    XmNleftPosition, 25,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);

  title =
    XmStringCreateLocalized ("in the subgoal pattern having the same stabilization toward electrophilic substitution as node");
  artielbl[1] = XtVaCreateWidget ("artielbl1",
               xmLabelWidgetClass, artieform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, artiemenu[0],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  artietxt = XmCreateTextField (artieform, "artietxt0", NULL, 0);

  XtVaSetValues (artietxt,
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, artielbl[1],
    XmNleftPosition, 25,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 50,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("IS");
  label[1] = XmStringCreateLocalized ("IS NOT");

  artiedummy[1] = XmVaCreateSimplePulldownMenu (artieform, "_artiepulldown1", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  if (glob_condstr[0] == '0') sel = 1;
  else sel = 0;

  artiemenu[1] = XmVaCreateSimpleOptionMenu (artieform, "artiemenu1", title, '\0', sel, ArTieOpt_CB,
      XmNsubMenuId, artiedummy[1],
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

  XtVaSetValues (artiemenu[1],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, artietxt,
    XmNleftPosition, 25,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("GREATER THAN");
  label[1] = XmStringCreateLocalized ("GREATER THAN OR EQUAL TO");
  label[2] = XmStringCreateLocalized ("EQUAL TO");
  label[3] = XmStringCreateLocalized ("UNEQUAL TO");
  label[4] = XmStringCreateLocalized ("LESS THAN");
  label[5] = XmStringCreateLocalized ("LESS THAN OR EQUAL TO");

  artiedummy[2] = XmVaCreateSimplePulldownMenu (artieform, "_artiepulldown2", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  if (strcmp (tmpstr, "> ") == 0) sel = 0;
  else if (strcmp (tmpstr, ">=") == 0) sel = 1;
  else if (strcmp (tmpstr, "= ") == 0) sel = 2;
  else if (strcmp (tmpstr, "~=") == 0) sel = 3;
  else if (strcmp (tmpstr, "< ") == 0) sel = 4;
  else if (strcmp (tmpstr, "<=") == 0) sel = 5;

  artiemenu[2] = XmVaCreateSimpleOptionMenu (artieform, "artiemenu2", title, '\0', sel, ArTieOpt_CB,
      XmNsubMenuId, artiedummy[2],
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
      XmVaPUSHBUTTON, label[4], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[5], '\0', NULL, NULL,
      NULL);

  XmStringFree (title);
  XmStringFree (label[0]);
  XmStringFree (label[1]);
  XmStringFree (label[2]);
  XmStringFree (label[3]);
  XmStringFree (label[4]);
  XmStringFree (label[5]);

  XtVaSetValues (artiemenu[2],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, artietxt,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, artiemenu[1],
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("0");
  label[1] = XmStringCreateLocalized ("1");
  label[2] = XmStringCreateLocalized ("2");
  label[3] = XmStringCreateLocalized ("3");
  label[4] = XmStringCreateLocalized ("4");
  label[5] = XmStringCreateLocalized ("5");
/*
  label[6] = XmStringCreateLocalized ("6");
  label[7] = XmStringCreateLocalized ("7");
  label[8] = XmStringCreateLocalized ("8");
  label[9] = XmStringCreateLocalized ("9");
*/

  artiedummy[3] = XmVaCreateSimplePulldownMenu (artieform, "_artiepulldown3", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  sel = value;

  artiemenu[3] = XmVaCreateSimpleOptionMenu (artieform, "artiemenu3", title, '\0', sel, ArTieOpt_CB,
      XmNsubMenuId, artiedummy[3],
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
      XmVaPUSHBUTTON, label[4], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[5], '\0', NULL, NULL,
/*
      XmVaPUSHBUTTON, label[6], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[7], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[8], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[9], '\0', NULL, NULL,
*/
      NULL);

  XmStringFree (title);
  XmStringFree (label[0]);
  XmStringFree (label[1]);
  XmStringFree (label[2]);
  XmStringFree (label[3]);
  XmStringFree (label[4]);
  XmStringFree (label[5]);
/*
  XmStringFree (label[6]);
  XmStringFree (label[7]);
  XmStringFree (label[8]);
  XmStringFree (label[9]);
*/

  XtVaSetValues (artiemenu[3],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, artietxt,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, artiemenu[2],
    NULL);

  XtAddCallback (artietxt, XmNvalueChangedCallback, ArNode_CB, (XtPointer) i);

  ArTie_Display (glob_condstr, node);
}

void ArTie_Display (char *v, int node)
{
  int i, sel;
  char tmpstr[5];

  XtManageChild (artielbl[0]);

  XtManageChild (artiemenu[0]);

  XtManageChild (artielbl[1]);

  sprintf (tmpstr, "%02d", node);
  XtVaSetValues (artietxt,
    XmNvalue, tmpstr,
    NULL);
  XtManageChild (artietxt);

  XtManageChild (artiemenu[1]);
  XtManageChild (artiemenu[2]);
  XtManageChild (artiemenu[3]);

  XtManageChild (artieform);
  XtManageChild (ptformdg);
}

void ArTieOpt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *wname;
  int sel;

  sel = (int) client_data;
  wname = XtName (XtParent (w));

  if (strstr (wname, "0") != NULL) glob_condstr[4] = sel == 0 ? '1' : '0';
  else if (strstr (wname, "1") != NULL) glob_condstr[0] = sel == 0 ? '1' : '0';
  else if (strstr (wname, "2") != NULL) switch (sel)
    {
  case 0:
    strncpy (glob_condstr + 7, "> ", 2);
    break;
  case 1:
    strncpy (glob_condstr + 7, ">=", 2);
    break;
  case 2:
    strncpy (glob_condstr + 7, "= ", 2);
    break;
  case 3:
    strncpy (glob_condstr + 7, "~=", 2);
    break;
  case 4:
    strncpy (glob_condstr + 7, "< ", 2);
    break;
  case 5:
    strncpy (glob_condstr + 7, "<=", 2);
    break;
    }
  else if (strstr (wname, "3") != NULL) glob_condstr[9] = sel + '0';
}

void ArTieExit_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  String_t string;
  char tmpstr[128], tmpstr2[128], savestr[128], *ts, *ts2, *ts3, *comma, *space, *newstr;
  int i, atom, node, dist;

  if ((Boolean_t) (int) client_data)
    {
/* get final results of text field */
    XtVaGetValues (artietxt,
      XmNvalue, &ts,
      NULL);

    if (ts[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }
    sscanf (ts, "%d", &node);

/* incorporate into string */
    sprintf (tmpstr, "%02d", node);
    strncpy (glob_condstr + 5, tmpstr, 2);

    newstr = (char *) malloc (strlen (glob_condstr) + 1);
    strcpy (newstr, glob_condstr);
    String_Value_Put (string, newstr);
    String_Length_Put (string, strlen (newstr));
    String_Alloc_Put (string, strlen (newstr) + 1);

    condition_export (glob_cond, &string, glob_nu);
    }
  else if (new_cond) cond_add_canceled = TRUE;

  XtUnmanageChild (ptformdg);
  XtDestroyWidget (artieform);
}
