/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              PTATOM.C
*
*    This module provides for the display and editing of the posttransform
*    condition type ATOM, which determines the identity of the atrom at a
*    given position.
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

extern int cond_type;
extern Widget pttl, ptformdg, condform, condpb[2], emptytextmsg, nodexceedmsg, invalidatommsg, condlbl;
extern Boolean_t cond_add_canceled;

static Widget atomform, atomlbl[2], atomtxt[3], atomdummy, atommenu;
static Condition_t *glob_cond;
static char glob_condstr[1024];
static Boolean_t *glob_nu, new_cond;

void Post_Cond_Put (Condition_t *);

void Atom_Display (char *, int, int, int);
void AtomOpt_CB (Widget, XtPointer, XtPointer);
void CondExit_CB (Widget, XtPointer, XtPointer);
void AtomExit_CB (Widget, XtPointer, XtPointer);
void AtomVal_CB (Widget, XtPointer, XtPointer);
void MtTxtMsg_Dismiss_CB (Widget, XtPointer, XtPointer);
void PTSetCondname (int, char *);

void PTAtom (Condition_t *cond, Boolean_t *node_used, int condnum, Boolean_t new)
{
  char *condstr, tmpstr[128];
  String_t string;
  XmString label[19], title;
  int i, sel, node, atom, dist;
  XmFontList flhv18;
  XmFontListEntry helv18;

  cond_type = PT_TYPE_ATOM;

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
    strcpy (condstr, "11101001");
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

  sscanf (glob_condstr + 3, "%2d%3d%2d", &node, &atom, &dist);
  if (strlen (glob_condstr) == 8) dist = 0;

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

  atomform = XmCreateForm (condform, "atomform", NULL, 0);
  XtVaSetValues (atomform,
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

  title = XmStringCreateLocalized ("The atom at distance");
  atomlbl[0] = XtVaCreateWidget ("atomlbl0",
               xmLabelWidgetClass, atomform,
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

  atomtxt[0] = XmCreateTextField (atomform, "atomtxt0", NULL, 0);

  XtVaSetValues (atomtxt[0],
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, atomlbl[0],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftPosition, 25,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 50,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("from node");
  atomlbl[1] = XtVaCreateWidget ("atomlbl1",
               xmLabelWidgetClass, atomform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, atomtxt[0],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  atomtxt[1] = XmCreateTextField (atomform, "atomtxt1", NULL, 0);

  XtVaSetValues (atomtxt[1],
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, atomlbl[1],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftPosition, 25,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 50,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("IS");
  label[1] = XmStringCreateLocalized ("IS NOT");

  atomdummy = XmVaCreateSimplePulldownMenu (atomform, "_atompulldown0", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  if (glob_condstr[0] == '0') sel = 1;
  else sel = 0;

  atommenu = XmVaCreateSimpleOptionMenu (atomform, "atommenu", title, '\0', sel, AtomOpt_CB,
      XmNsubMenuId, atomdummy,
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

  XtVaSetValues (atommenu,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, atomtxt[1],
    XmNleftPosition, 25,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);

  atomtxt[2] = XmCreateTextField (atomform, "atomtxt2", NULL, 0);

  XtVaSetValues (atomtxt[2],
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, atomtxt[1],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, atommenu,
    XmNrightPosition, 75,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  for (i = 0; i < 3; i++)
    XtAddCallback (atomtxt[i], XmNvalueChangedCallback, AtomVal_CB, (XtPointer) i);

  Atom_Display (glob_condstr, node, dist, atom);
}

void Atom_Display (char *v, int node, int dist, int atom)
{
  int i, sel;
  char tmpstr[5];

  XtManageChild (atomlbl[0]);

  sprintf (tmpstr, "%02d", dist);
  XtVaSetValues (atomtxt[0],
    XmNvalue, tmpstr,
    NULL);
  XtManageChild (atomtxt[0]);

  XtManageChild (atomlbl[1]);

  sprintf (tmpstr, "%02d", node);
  XtVaSetValues (atomtxt[1],
    XmNvalue, tmpstr,
    NULL);
  XtManageChild (atomtxt[1]);

  XtManageChild (atommenu);

  strcpy (tmpstr, Atomid2Symbol (atom));
  XtVaSetValues (atomtxt[2],
    XmNvalue, tmpstr,
    NULL);
  XtManageChild (atomtxt[2]);

  XtManageChild (atomform);
  XtManageChild (ptformdg);
}

void AtomOpt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  int sel;

  sel = (int) client_data;

  glob_condstr[0] = sel == 0 ? '1' : '0';
}

void AtomExit_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  String_t string;
  char tmpstr[128], tmpstr2[128], savestr[128], *ts, *ts2, *ts3, *comma, *space, *newstr;
  int i, atom, node, dist;

  if ((Boolean_t) (int) client_data)
    {
/* get final results of text field */
    XtVaGetValues (atomtxt[0],
      XmNvalue, &ts,
      NULL);

    if (ts[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }
    sscanf (ts, "%d", &dist);

    XtVaGetValues (atomtxt[1],
      XmNvalue, &ts2,
      NULL);

    if (ts2[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }
    sscanf (ts2, "%d", &node);

    XtVaGetValues (atomtxt[2],
      XmNvalue, &ts3,
      NULL);

    if (ts3[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }
    atom = Atomsymbol2Id (ts3);
    if (atom == 0)
      {
      XtManageChild (invalidatommsg);
      return;
      }

/* incorporate into string */
    sprintf (glob_condstr + 3, "%02d%03d", node, atom);
    if (dist != 0)
      sprintf (glob_condstr + 8, "%02d", dist);

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
  XtDestroyWidget (atomform);
}

void AtomVal_CB (Widget w, XtPointer client_data, XtPointer call_data)
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
