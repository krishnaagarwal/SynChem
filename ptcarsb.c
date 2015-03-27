/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              PTCARSB.C
*
*    This module provides for the display and editing of the posttransform
*    condition for comparing nodes for reletive ability to stabilize an
*    incipient carbonium ion.
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
extern Widget pttl, ptformdg, condform, condpb[2], emptytextmsg, nodexceedmsg, condlbl;
extern Boolean_t cond_add_canceled;

static Widget carsbform, carsblbl[4], carsbtxt[4], carsbmenu[2], carsbdummy[2];
static Condition_t *glob_cond;
static char glob_condstr[1024];
static Boolean_t *glob_nu, new_cond;
void Post_Cond_Put (Condition_t *);

void CarSb_Display (char *, int *, int *);
void CarSbOpt_CB (Widget, XtPointer, XtPointer);
void CondExit_CB (Widget, XtPointer, XtPointer);
void CarSbExit_CB (Widget, XtPointer, XtPointer);
void CarSbNode_CB (Widget, XtPointer, XtPointer);
void PTSetCondname (int, char *);

void PTCarSb (Condition_t *cond, Boolean_t *node_used, int condnum, Boolean_t new)
{
  char *condstr, tmpstr[128], *sling;
  String_t string;
  XmString label[6], title;
  int i, sel, node[2], node2[2];
  XmFontList flhv18;
  XmFontListEntry helv18;

  cond_type = PT_TYPE_CARBONIUM;

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
    strcpy (condstr, "1080102<=0304");
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

  strncpy (tmpstr, glob_condstr + 3, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", node);
  strncpy (tmpstr, glob_condstr + 5, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", node2);
  strncpy (tmpstr, glob_condstr + 9, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", node + 1);
  strncpy (tmpstr, glob_condstr + 11, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", node2 + 1);

  carsbform = XmCreateForm (condform, "carsbform", NULL, 0);
  XtVaSetValues (carsbform,
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

  title = XmStringCreateLocalized ("The stability of the carbonium ion defined by nodes");
  carsblbl[0] = XtVaCreateWidget ("carsblbl0",
               xmLabelWidgetClass, carsbform,
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

  carsbtxt[0] = XmCreateTextField (carsbform, "carsbtxt0", NULL, 0);

  XtVaSetValues (carsbtxt[0],
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, carsblbl[0],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftPosition, 25,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 50,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized (" AND ");
  carsblbl[1] = XtVaCreateWidget ("carsblbl1",
               xmLabelWidgetClass, carsbform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, carsblbl[0],
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, carsbtxt[0],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_WIDGET,
               XmNleftWidget, carsbtxt[0],
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_NONE,
               NULL);
  XmStringFree (title);

  carsbtxt[1] = XmCreateTextField (carsbform, "carsbtxt1", NULL, 0);

  XtVaSetValues (carsbtxt[1],
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, carsblbl[0],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, carsblbl[1],
    XmNrightPosition, 100,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("IS");
  label[1] = XmStringCreateLocalized ("IS NOT");

  carsbdummy[0] = XmVaCreateSimplePulldownMenu (carsbform, "_carsbpulldown0", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  if (glob_condstr[0] == '1') sel = 0;
  else sel = 1;

  carsbmenu[0] = XmVaCreateSimpleOptionMenu (carsbform, "carsbmenu0", title, '\0', sel, CarSbOpt_CB,
      XmNsubMenuId, carsbdummy[0],
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

  XtVaSetValues (carsbmenu[0],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, carsbtxt[0],
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

  carsbdummy[1] = XmVaCreateSimplePulldownMenu (carsbform, "_carsbpulldown1", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  strncpy (tmpstr, glob_condstr + 7, 2);
  tmpstr[2] = '\0';
  if (strcmp (tmpstr, "> ") == 0) sel = 0;
  else if (strcmp (tmpstr, ">=") == 0) sel = 1;
  else if (strcmp (tmpstr, "= ") == 0) sel = 2;
  else if (strcmp (tmpstr, "~=") == 0) sel = 3;
  else if (strcmp (tmpstr, "< ") == 0) sel = 4;
  else if (strcmp (tmpstr, "<=") == 0) sel = 5;

  carsbmenu[1] = XmVaCreateSimpleOptionMenu (carsbform, "carsbmenu1", title, '\0', sel, CarSbOpt_CB,
      XmNsubMenuId, carsbdummy[1],
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

  XtVaSetValues (carsbmenu[1],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, carsbtxt[0],
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, carsbmenu[0],
    NULL);

  title = XmStringCreateLocalized ("the stability of the carbonium ion defined by nodes");
  carsblbl[2] = XtVaCreateWidget ("carsblbl2",
               xmLabelWidgetClass, carsbform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, carsbmenu[0],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  carsbtxt[2] = XmCreateTextField (carsbform, "carsbtxt2", NULL, 0);

  XtVaSetValues (carsbtxt[2],
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, carsblbl[2],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftPosition, 25,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 50,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized (" AND ");
  carsblbl[3] = XtVaCreateWidget ("carsblbl3",
               xmLabelWidgetClass, carsbform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, carsblbl[2],
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, carsbtxt[2],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_WIDGET,
               XmNleftWidget, carsbtxt[2],
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_NONE,
               NULL);
  XmStringFree (title);

  carsbtxt[3] = XmCreateTextField (carsbform, "carsbtxt3", NULL, 0);

  XtVaSetValues (carsbtxt[3],
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, carsblbl[2],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, carsblbl[3],
    XmNrightPosition, 100,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  for (i = 0; i < 4; i++)
    XtAddCallback (carsbtxt[i], XmNvalueChangedCallback, CarSbNode_CB, (XtPointer) i);

  CarSb_Display (glob_condstr, node, node2);
}

void CarSb_Display (char *v, int *node, int *node2)
{
  int i, sel;
  char nodelist[256], tmpstr[5];

  XtManageChild (carsblbl[0]);

  sprintf (nodelist, "%02d", node[0]);

  XtVaSetValues (carsbtxt[0],
    XmNvalue, nodelist,
    NULL);

  XtManageChild (carsbtxt[0]);

  XtManageChild (carsblbl[1]);

  sprintf (nodelist, "%02d", node2[0]);

  XtVaSetValues (carsbtxt[1],
    XmNvalue, nodelist,
    NULL);

  XtManageChild (carsbtxt[1]);

  XtManageChild (carsbmenu[0]);
  XtManageChild (carsbmenu[1]);

  XtManageChild (carsblbl[2]);

  sprintf (nodelist, "%02d", node[1]);

  XtVaSetValues (carsbtxt[2],
    XmNvalue, nodelist,
    NULL);

  XtManageChild (carsbtxt[2]);

  XtManageChild (carsblbl[3]);

  sprintf (nodelist, "%02d", node2[1]);

  XtVaSetValues (carsbtxt[3],
    XmNvalue, nodelist,
    NULL);

  XtManageChild (carsbtxt[3]);

  XtManageChild (carsbform);
  XtManageChild (ptformdg);
}

void CarSbOpt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *wname, tmpstr[128], *ts, *comma, *space;
  int sel, sel2, node, node2;
  XmString title;

  wname = XtName (XtParent (w));
  sel = (int) client_data;

  if (strstr (wname, "0") != NULL)
    {
    if (sel == 0) glob_condstr[0] = '1';
    else glob_condstr[0] = '0';
    }
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
}

void CarSbNode_CB (Widget w, XtPointer client_data, XtPointer call_data)
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

void CarSbExit_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  String_t string;
  char sling[128], tmpstr[128], tmpstr2[128], savestr[128], *ts, *ts2, *comma, *space, *newstr;
  int i, node, node2;

  if ((Boolean_t) (int) client_data)
    {
/* first get final results of text fields */
    XtVaGetValues (carsbtxt[0],
      XmNvalue, &ts,
      NULL);

    if (ts[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }
    sscanf (ts, "%d", &node);

    XtVaGetValues (carsbtxt[1],
      XmNvalue, &ts2,
      NULL);

    if (ts2[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }
    sscanf (ts2, "%d", &node2);
    sprintf (tmpstr, "%02d%02d", node, node2);

    XtVaGetValues (carsbtxt[2],
      XmNvalue, &ts,
      NULL);

    if (ts[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }
    sscanf (ts, "%d", &node);

    XtVaGetValues (carsbtxt[3],
      XmNvalue, &ts2,
      NULL);

    if (ts2[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }
    sscanf (ts2, "%d", &node2);
    sprintf (tmpstr2, "%02d%02d", node, node2);

/* incorporate into string */
    strncpy (glob_condstr + 3, tmpstr, 4);
    strncpy (glob_condstr + 9, tmpstr2, 4);

    newstr = (char *) malloc (strlen (glob_condstr) + 1);
    strcpy (newstr, glob_condstr);
    String_Value_Put (string, newstr);
    String_Length_Put (string, strlen (newstr));
    String_Alloc_Put (string, strlen (newstr) + 1);

    condition_export (glob_cond, &string, glob_nu);
    }
  else if (new_cond) cond_add_canceled = TRUE;

  XtUnmanageChild (ptformdg);
  XtDestroyWidget (carsbform);
}
