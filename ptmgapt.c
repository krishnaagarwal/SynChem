/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              PTMGAPT.C
*
*    This module provides for the display and editing of the posttransform
*    conditions for determining the migratory aptitude of a given node.
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

#ifndef NATTRIB
#define NATTRIB 62
#endif

extern int cond_type;
extern Widget pttl, ptformdg, condform, condpb[2], emptytextmsg, nodexceedmsg, condlbl;
extern Boolean_t cond_add_canceled;

static Widget mgaptform, mgaptlbl[3], mgapttxt[2], mgaptmenu[3], mgaptdummy[3];
static Condition_t *glob_cond;
static char glob_condstr[1024];
static Boolean_t *glob_nu, new_cond;

void Post_Cond_Put (Condition_t *);
void PTSetCondname (int, char *);

void MgApt_Display (char *, int, int);
void MgAptOpt_CB (Widget, XtPointer, XtPointer);
void CondExit_CB (Widget, XtPointer, XtPointer);
void MgAptExit_CB (Widget, XtPointer, XtPointer);
void MgAptNode_CB (Widget, XtPointer, XtPointer);

static char *gname[] = {"A NON-MIGRATORY GROUP", "AN ALKYL GROUP", "AN ARYL GROUP", "A PROTON", "THE GROUP DEFINED BY ..."};

void PTMgApt (Condition_t *cond, Boolean_t *node_used, int condnum, Boolean_t new)
{
  char *condstr, tmpstr[128];
  String_t string;
  XmString label[NATTRIB + 1], title;
  int i, sel, node, node2;
  XmFontList flhv18;
  XmFontListEntry helv18;

  cond_type = PT_TYPE_MIGRATAP;

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
    strcpy (condstr, "11001> 001");
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

  mgaptform = XmCreateForm (condform, "mgaptform", NULL, 0);
  XtVaSetValues (mgaptform,
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

  title = XmStringCreateLocalized ("The group in the target molecule defined by node");
  mgaptlbl[0] = XtVaCreateWidget ("mgaptlbl0",
               xmLabelWidgetClass, mgaptform,
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

  mgapttxt[0] = XmCreateTextField (mgaptform, "mgapttxt0", NULL, 0);

  XtVaSetValues (mgapttxt[0],
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, mgaptlbl[0],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftPosition, 25,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 50,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("has a migratory aptitude that");
  label[0] = XmStringCreateLocalized ("IS");
  label[1] = XmStringCreateLocalized ("IS NOT");

  mgaptdummy[0] = XmVaCreateSimplePulldownMenu (mgaptform, "_mgaptpulldown0", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  if (glob_condstr[0] == '1') sel = 0;
  else sel = 1;

  mgaptmenu[0] = XmVaCreateSimpleOptionMenu (mgaptform, "mgaptmenu0", title, '\0', sel, MgAptOpt_CB,
      XmNsubMenuId, mgaptdummy[0],
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

  XtVaSetValues (mgaptmenu[0],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, mgapttxt[0],
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("GREATER THAN");
  label[1] = XmStringCreateLocalized ("GREATER THAN OR EQUAL TO");
  label[2] = XmStringCreateLocalized ("EQUAL TO");
  label[3] = XmStringCreateLocalized ("UNEQUAL TO");
  label[4] = XmStringCreateLocalized ("LESS THAN");
  label[5] = XmStringCreateLocalized ("LESS THAN OR EQUAL TO");

  mgaptdummy[1] = XmVaCreateSimplePulldownMenu (mgaptform, "_mgaptpulldown1", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  strncpy (tmpstr, glob_condstr + 3, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", &node);
  strncpy (tmpstr, glob_condstr + 7, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", &node2);
  strncpy (tmpstr, glob_condstr + 5, 2);
  tmpstr[2] = '\0';
  if (strcmp (tmpstr, "> ") == 0) sel = 0;
  else if (strcmp (tmpstr, ">=") == 0) sel = 1;
  else if (strcmp (tmpstr, "= ") == 0) sel = 2;
  else if (strcmp (tmpstr, "~=") == 0) sel = 3;
  else if (strcmp (tmpstr, "< ") == 0) sel = 4;
  else if (strcmp (tmpstr, "<=") == 0) sel = 5;

  mgaptmenu[1] = XmVaCreateSimpleOptionMenu (mgaptform, "mgaptmenu1", title, '\0', sel, MgAptOpt_CB,
      XmNsubMenuId, mgaptdummy[1],
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

  XtVaSetValues (mgaptmenu[1],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, mgapttxt[0],
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, mgaptmenu[0],
    NULL);

  title = XmStringCreateLocalized ("THAT OF ");
  for (i = 0; i < 5; i++)
    label[i] = XmStringCreateLocalized (gname[i]);

  mgaptdummy[2] = XmVaCreateSimplePulldownMenu (mgaptform, "_mgaptpulldown2", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  if (glob_condstr[9] == '+') sel = 4;
  else sel = glob_condstr[9] - '0';

  mgaptmenu[2] = XmVaCreateSimpleOptionMenu (mgaptform, "mgaptmenu2", title, '\0', sel, MgAptOpt_CB,
      XmNsubMenuId, mgaptdummy[2],
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
      NULL);

  XmStringFree (title);
  XmStringFree (label[0]);
  XmStringFree (label[1]);
  XmStringFree (label[2]);
  XmStringFree (label[3]);
  XmStringFree (label[4]);

  XtVaSetValues (mgaptmenu[2],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, mgaptmenu[0],
    XmNleftPosition, 25,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("NODE ");
  mgaptlbl[1] = XtVaCreateWidget ("mgaptlbl1",
               xmLabelWidgetClass, mgaptform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, mgaptmenu[2],
               XmNleftPosition, 25,
               XmNleftAttachment, XmATTACH_POSITION,
               NULL);
  XmStringFree (title);

  mgapttxt[1] = XmCreateTextField (mgaptform, "mgapttxt1", NULL, 0);

  XtVaSetValues (mgapttxt[1],
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, mgaptmenu[2],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, mgaptlbl[1],
    XmNrightPosition, 100,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("NODE ");
  mgaptlbl[2] = XtVaCreateWidget ("mgaptlbl2",
               xmLabelWidgetClass, mgaptform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNtopWidget, mgapttxt[1],
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, mgapttxt[1],
               XmNleftPosition, 25,
               XmNleftAttachment, XmATTACH_POSITION,
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_WIDGET,
               XmNrightWidget, mgapttxt[1],
               NULL);
  XmStringFree (title);

  for (i = 0; i < 2; i++)
    XtAddCallback (mgapttxt[i], XmNvalueChangedCallback, MgAptNode_CB, (XtPointer) i);

  MgApt_Display (glob_condstr, node, node2);
}

void MgApt_Display (char *v, int node, int node2)
{
  int i, sel;
  char nodelist[256], tmpstr[5];

  XtManageChild (mgaptlbl[0]);

  sprintf (nodelist, "%02d", node);

  XtVaSetValues (mgapttxt[0],
    XmNvalue, nodelist,
    NULL);

  XtManageChild (mgapttxt[0]);

  XtManageChild (mgaptmenu[0]);
  XtManageChild (mgaptmenu[1]);
  XtManageChild (mgaptmenu[2]);

  XtManageChild (mgaptlbl[1]);

  sprintf (nodelist, "%02d", node2);

  XtVaSetValues (mgapttxt[1],
    XmNvalue, nodelist,
    NULL);

  XtManageChild (mgapttxt[1]);

  XtManageChild (mgaptlbl[2]);

  XtManageChild (mgaptform);
  XtManageChild (ptformdg);

  XtUnmanageChild (mgaptlbl[1]); /* dummy for positioning - AARGH! */

  if (node2 == 0)
    {
    XtUnmanageChild (mgaptlbl[2]);
    XtUnmanageChild (mgapttxt[1]);
    }
}

void MgAptOpt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *wname, tmpstr[128], *ts, *comma, *space;
  int sel, sel2, nnodes, nnodes2;

  wname = XtName (XtParent (w));
  sel = (int) client_data;

  if (strstr (wname, "0") != NULL) glob_condstr[0] = sel == 0 ? '1' : '0';
  else if (strstr (wname, "1") != NULL) switch (sel)
    {
  case 0:
    strncpy (glob_condstr + 5, "> ", 2);
    break;
  case 1:
    strncpy (glob_condstr + 5, ">=", 2);
    break;
  case 2:
    strncpy (glob_condstr + 5, "= ", 2);
    break;
  case 3:
    strncpy (glob_condstr + 5, "~=", 2);
    break;
  case 4:
    strncpy (glob_condstr + 5, "< ", 2);
    break;
  case 5:
    strncpy (glob_condstr + 5, "<=", 2);
    break;
    }
  else if (strstr (wname, "2") != NULL) switch (sel)
    {
  case 0:
  case 1:
  case 2:
  case 3:
    XmTextReplace (mgapttxt[1],0,2,"");
    XtUnmanageChild (mgaptlbl[2]);
    XtUnmanageChild (mgapttxt[1]);
    sprintf (glob_condstr + 7, "00%d", sel);
    break;
  case 4:
    XtManageChild (mgaptlbl[2]);
    XtManageChild (mgapttxt[1]);
    glob_condstr[9] = '+';
    break;
    }
}

void MgAptNode_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *lcl_node_str, node_str[128];
  int i, len;
  Boolean_t ok;
  static Boolean_t internal_change = FALSE;

  if (internal_change) return; /* ignore callback due to field truncation */

  lcl_node_str = XmTextGetString(w);

  len = strlen (lcl_node_str);
  for (i = 0, ok = TRUE; i < len && ok; i++)
    {
    ok = i < 2 && lcl_node_str[i] >= '0' && lcl_node_str[i] <= '9';
    if (!ok) i--; /* adjust prior to increment */
    }

  if (!ok)
    {
    strcpy (node_str, lcl_node_str);
    strcpy (node_str + i, node_str + i + 1);
    XtFree(lcl_node_str);
    internal_change = TRUE;
    XmTextReplace (w,0,len,node_str);
    internal_change = FALSE;
    XBell (XtDisplay (w), 10);
    }
  else XtFree(lcl_node_str);
}

void MgAptExit_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  String_t string;
  char sling[128], tmpstr[128], tmpstr2[128], savestr[128], *ts, *ts2, *comma, *space, *newstr;
  int i, node, node2;

  if ((Boolean_t) (int) client_data)
    {
/* first get final results of text fields */
    XtVaGetValues (mgapttxt[0],
      XmNvalue, &ts,
      NULL);

    if (ts[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }
    sscanf (ts, "%d", &node);
    sprintf (tmpstr, "%02d", node);

    if (XtIsManaged (mgapttxt[1]))
      {
      XtVaGetValues (mgapttxt[1],
        XmNvalue, &ts2,
        NULL);

      if (ts2[0] == '\0')
        {
        XtManageChild (emptytextmsg);
        return;
        }
      sscanf (ts2, "%d", &node2);
      }
    else node2 = 0;
    sprintf (tmpstr2, "%02d", node2);

/* incorporate into string */
    strncpy (glob_condstr + 3, tmpstr, 2);
    strncpy (glob_condstr + 7, tmpstr2, 2);

    newstr = (char *) malloc (strlen (glob_condstr) + 1);
    strcpy (newstr, glob_condstr);
    String_Value_Put (string, newstr);
    String_Length_Put (string, strlen (newstr));
    String_Alloc_Put (string, strlen (newstr) + 1);

    condition_export (glob_cond, &string, glob_nu);
    }
  else if (new_cond) cond_add_canceled = TRUE;

  XtUnmanageChild (ptformdg);
  XtDestroyWidget (mgaptform);
}
