/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              PTARSTD.C
*
*    This module provides for the display and editing of the aromatic subsitution
*    posttransform condition subtype for determining the ranking of a given node
*    relative to others in the same aromatic ring.
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

static Widget arstdform, arstdlbl[5], arstdtxt, arstddummy[4], arstdmenu[4];
static Condition_t *glob_cond;
static char glob_condstr[1024];
static Boolean_t *glob_nu, new_cond;

void Post_Cond_Put (Condition_t *);
void PTSetCondname (int, char *);

void ArStd_Display (char *, int);
void ArStdOpt_CB (Widget, XtPointer, XtPointer);
void CondExit_CB (Widget, XtPointer, XtPointer);
void ArNode_CB (Widget, XtPointer, XtPointer);
void ArStdExit_CB (Widget, XtPointer, XtPointer);
void MtTxtMsg_Dismiss_CB (Widget, XtPointer, XtPointer);

void PTArStd (Condition_t *cond, Boolean_t *node_used, int condnum, Boolean_t new)
{
  char *condstr, tmpstr[128];
  String_t string;
  XmString label[19], title;
  int i, sel, node, value;
  XmFontList flhv18;
  XmFontListEntry helv18;
  Boolean_t arh;

  cond_type = PT_TYPE_AROMSUB;
  cond_subtype = 2;

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
    strcpy (condstr, "1202101= 1");
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

  arstdform = XmCreateForm (condform, "arstdform", NULL, 0);
  XtVaSetValues (arstdform,
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

  title = XmStringCreateLocalized ("In the subgoal pattern, the activation at");
  arstdlbl[0] = XtVaCreateWidget ("arstdlbl0",
               xmLabelWidgetClass, arstdform,
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

  title = XmStringCreateLocalized ("NODE ");
  arstdlbl[1] = XtVaCreateWidget ("arstdlbl1",
               xmLabelWidgetClass, arstdform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, arstdlbl[0],
               XmNleftPosition, 25,
               XmNleftAttachment, XmATTACH_POSITION,
               NULL);
  XmStringFree (title);

  arstdtxt = XmCreateTextField (arstdform, "arstdtxt0", NULL, 0);

  XtVaSetValues (arstdtxt,
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, arstdlbl[0],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, arstdlbl[1],
    XmNrightPosition, 75,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("NODE ");
  arstdlbl[2] = XtVaCreateWidget ("arstdlbl2",
               xmLabelWidgetClass, arstdform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNtopWidget, arstdtxt,
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, arstdtxt,
               XmNleftPosition, 25,
               XmNleftAttachment, XmATTACH_POSITION,
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_WIDGET,
               XmNrightWidget, arstdtxt,
               NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized ("relative to the remainder of the ring");
  arstdlbl[3] = XtVaCreateWidget ("arstdlbl3",
               xmLabelWidgetClass, arstdform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, arstdtxt,
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("IS");
  label[1] = XmStringCreateLocalized ("IS NOT");

  arstddummy[0] = XmVaCreateSimplePulldownMenu (arstdform, "_arstdpulldown0", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  if (glob_condstr[0] == '0') sel = 1;
  else sel = 0;

  arstdmenu[0] = XmVaCreateSimpleOptionMenu (arstdform, "arstdmenu0", title, '\0', sel, ArStdOpt_CB,
      XmNsubMenuId, arstddummy[0],
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

  XtVaSetValues (arstdmenu[0],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, arstdlbl[3],
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

  arstddummy[1] = XmVaCreateSimplePulldownMenu (arstdform, "_arstdpulldown1", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  if (strcmp (tmpstr, "> ") == 0) sel = 0;
  else if (strcmp (tmpstr, ">=") == 0) sel = 1;
  else if (strcmp (tmpstr, "= ") == 0) sel = 2;
  else if (strcmp (tmpstr, "~=") == 0) sel = 3;
  else if (strcmp (tmpstr, "< ") == 0) sel = 4;
  else if (strcmp (tmpstr, "<=") == 0) sel = 5;

  arstdmenu[1] = XmVaCreateSimpleOptionMenu (arstdform, "arstdmenu1", title, '\0', sel, ArStdOpt_CB,
      XmNsubMenuId, arstddummy[1],
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

  XtVaSetValues (arstdmenu[1],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, arstdlbl[3],
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, arstdmenu[0],
    NULL);

  title = XmStringCreateLocalized ("the constant ");
  label[0] = XmStringCreateLocalized ("1");
  label[1] = XmStringCreateLocalized ("2");
  label[2] = XmStringCreateLocalized ("3");
  label[3] = XmStringCreateLocalized ("4");
  label[4] = XmStringCreateLocalized ("5");
  label[5] = XmStringCreateLocalized ("6");
/*
  label[6] = XmStringCreateLocalized ("7");
  label[7] = XmStringCreateLocalized ("8");
  label[8] = XmStringCreateLocalized ("9");
*/

  arstddummy[2] = XmVaCreateSimplePulldownMenu (arstdform, "_arstdpulldown2", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  sel = value - 1;

  arstdmenu[2] = XmVaCreateSimpleOptionMenu (arstdform, "arstdmenu2", title, '\0', sel, ArStdOpt_CB,
      XmNsubMenuId, arstddummy[2],
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
*/

  XtVaSetValues (arstdmenu[2],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, arstdmenu[0],
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    NULL);

  title = XmStringCreateLocalized ("[1 represents the most activated position(s)], considering");
  arstdlbl[4] = XtVaCreateWidget ("arstdlbl4",
               xmLabelWidgetClass, arstdform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, arstdmenu[2],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("ONLY HYDROGENS");
  label[1] = XmStringCreateLocalized ("ALL SUBSTITUENTS");

  arstddummy[3] = XmVaCreateSimplePulldownMenu (arstdform, "_arstdpulldown3", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  if (arh) sel = 0;
  else sel = 1;

  arstdmenu[3] = XmVaCreateSimpleOptionMenu (arstdform, "arstdmenu3", title, '\0', sel, ArStdOpt_CB,
      XmNsubMenuId, arstddummy[3],
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

  XtVaSetValues (arstdmenu[3],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, arstdlbl[4],
    XmNleftPosition, 25,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);

  XtAddCallback (arstdtxt, XmNvalueChangedCallback, ArNode_CB, (XtPointer) i);

  ArStd_Display (glob_condstr, node);
}

void ArStd_Display (char *v, int node)
{
  int i, sel;
  char tmpstr[5];

  XtManageChild (arstdlbl[0]);
  XtManageChild (arstdlbl[1]);

  sprintf (tmpstr, "%02d", node);
  XtVaSetValues (arstdtxt,
    XmNvalue, tmpstr,
    NULL);
  XtManageChild (arstdtxt);

  XtManageChild (arstdlbl[2]);
  XtManageChild (arstdlbl[3]);

  XtManageChild (arstdmenu[0]);
  XtManageChild (arstdmenu[1]);
  XtManageChild (arstdmenu[2]);

  XtManageChild (arstdlbl[4]);

  XtManageChild (arstdmenu[3]);

  XtManageChild (arstdform);
  XtManageChild (ptformdg);

  XtUnmanageChild (arstdlbl[1]);
}

void ArStdOpt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *wname;
  int sel;

  sel = (int) client_data;
  wname = XtName (XtParent (w));

  if (strstr (wname, "0") != NULL) glob_condstr[0] = sel == 0 ? '1' : '0';
  else if (strstr (wname, "1") != NULL) switch (sel)
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
  else if (strstr (wname, "2") != NULL) glob_condstr[9] = sel + '1';
  else if (strstr (wname, "3") != NULL) glob_condstr[4] = sel == 0 ? '1' : '0';
}

void ArStdExit_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  String_t string;
  char tmpstr[128], tmpstr2[128], savestr[128], *ts, *ts2, *ts3, *comma, *space, *newstr;
  int i, atom, node, dist;

  if ((Boolean_t) (int) client_data)
    {
/* get final results of text field */
    XtVaGetValues (arstdtxt,
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
  XtDestroyWidget (arstdform);
}
