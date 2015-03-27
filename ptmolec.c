/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              PTMOLEC.C
*
*    This module provides for the display and editing of the posttransform
*    conditions for determining the molecularity of the reaction.
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
/*
char *elec (char *);
*/

extern int cond_type;
extern Widget pttl, ptformdg, condform, condpb[2], emptytextmsg, nodexceedmsg, condlbl;
extern Boolean_t cond_add_canceled;

static Widget molecform, molectxt, molecdummy, molecmenu;
static Condition_t *glob_cond;
static char glob_condstr[1024];
static Boolean_t *glob_nu, new_cond;

void Post_Cond_Put (Condition_t *);

void Molec_Display (char *, int, int);
void MolecOpt_CB (Widget, XtPointer, XtPointer);
void CondExit_CB (Widget, XtPointer, XtPointer);
void MolecExit_CB (Widget, XtPointer, XtPointer);
void NumMol_CB (Widget, XtPointer, XtPointer);
void MtTxtMsg_Dismiss_CB (Widget, XtPointer, XtPointer);
Boolean_t BadNum (char *);
void PTSetCondname (int, char *);

void PTMolec (Condition_t *cond, Boolean_t *node_used, int condnum, Boolean_t new)
{
  char *condstr, tmpstr[128];
  String_t string;
  XmString label[19], title;
  int i, sel, nnodes, nnodes2;
/*
  XmFontList flhv18;
  XmFontListEntry helv18;
*/
  XmFontList flhv14;
  XmFontListEntry helv14;

  cond_type = PT_TYPE_NUMMOLEC;

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
    strcpy (condstr, "1021");
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

/*
  helv18 = XmFontListEntryLoad (XtDisplay (pttl), "-*-helvetica-bold-r-normal-*-18-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);

  flhv18 = XmFontListAppendEntry (NULL, helv18);
*/
  helv14 = XmFontListEntryLoad (XtDisplay (pttl), "-*-helvetica-bold-r-normal-*-14-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);

  flhv14 = XmFontListAppendEntry (NULL, helv14);

  PTSetCondname (condnum, tmpstr);

  title = XmStringCreateLocalized (tmpstr);
  XtVaSetValues (condlbl,
    XmNlabelString, title,
    NULL);
  XmStringFree (title);

  molecform = XmCreateForm (condform, "molecform", NULL, 0);
  XtVaSetValues (molecform,
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

  title = XmStringCreateLocalized ("The number of reacting molecules");
  label[0] = XmStringCreateLocalized ("IS");
  label[1] = XmStringCreateLocalized ("IS NOT");

/*
  molecdummy = XmVaCreateSimplePulldownMenu (molecform, "_molecpulldown0", 0, NULL, XmNbuttonFontList, flhv18, NULL);
*/
  molecdummy = XmVaCreateSimplePulldownMenu (molecform, "_molecpulldown0", 0, NULL, XmNbuttonFontList, flhv14, NULL);

  if (glob_condstr[0] == '0') sel = 1;
  else sel = 0;

  molecmenu = XmVaCreateSimpleOptionMenu (molecform, "molecmenu", title, '\0', sel, MolecOpt_CB,
      XmNsubMenuId, molecdummy,
/*
      XmNlabelFontList, flhv18,
      XmNbuttonFontList, flhv18,
*/
      XmNlabelFontList, flhv14,
      XmNbuttonFontList, flhv14,
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

  XtVaSetValues (molecmenu,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    NULL);

  molectxt = XmCreateTextField (molecform, "molectxt", NULL, 0);

  XtVaSetValues (molectxt,
/*
    XmNfontList, flhv18,
*/
    XmNfontList, flhv14,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, molecmenu,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftPosition, 12,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 50,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  XtAddCallback (molectxt, XmNvalueChangedCallback, NumMol_CB, (XtPointer) i);

  Molec_Display (glob_condstr, nnodes, nnodes2);
}

void Molec_Display (char *v, int nnodes, int nnodes2)
{
  int i, sel;
  char nummol[2], tmpstr[5];

  XtManageChild (molecmenu);

  nummol[0] = v[3];
  nummol[1] = '\0';
  XtVaSetValues (molectxt,
    XmNvalue, nummol,
    NULL);
  XtManageChild (molectxt);

  XtManageChild (molecform);
  XtManageChild (ptformdg);
}

void MolecOpt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  int sel;

  sel = (int) client_data;

  glob_condstr[0] = sel == 0 ? '1' : '0';
}

void MolecExit_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  String_t string;
  char tmpstr[128], tmpstr2[128], savestr[128], *ts, *ts2, *comma, *space, *newstr;
  int i, nnodes, nnodes2;

  if ((Boolean_t) (int) client_data)
    {
/* get final results of text field */
    XtVaGetValues (molectxt,
      XmNvalue, &ts,
      NULL);

    if (ts[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }

/* incorporate into string */
    glob_condstr[3] = ts[0];

/*
    String_Value_Put (string, glob_condstr);
    String_Length_Put (string, strlen (glob_condstr));
    String_Alloc_Put (string, 0);
just in case someone tries to free a static char array!
*/

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
  XtDestroyWidget (molecform);
}

void NumMol_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *lcl_node_str, node_str[128];
  int len;
  static Boolean_t internal_change = FALSE;

  if (internal_change) return; /* ignore callback due to username clearing */

  lcl_node_str = XmTextGetString(w);

  if (BadNum (lcl_node_str))
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

Boolean_t BadNum (char *str)
{
  char chr;
  int len, i;

/* empty string OK here */
  if (str[0] == '\0') return (FALSE);
/* must be only one character long */
  if (str[1] != '\0') return (TRUE);
/* must start with digit */
  if (str[0] < '0' || str[0] > '9') return (TRUE);
  return (FALSE);
}
