/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              PTALSMR.C
*
*    This module provides for the display and editing of the allene/alkyne
*    in a small ring posttransform conditions.
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
extern Widget pttl, ptformdg, condform, condpb[2], emptytextmsg, nodexceedmsg, condlbl;
extern Boolean_t cond_add_canceled;

static Widget alsmrform, alsmrlbl[3], alsmrtxt[2], alsmrmenu[2], alsmrdummy[2];
static Condition_t *glob_cond;
static char glob_condstr[1024];
static Boolean_t *glob_nu, new_cond;
void Post_Cond_Put (Condition_t *);

void AlSmR_Display (char *, int, int);
void AlSmROpt_CB (Widget, XtPointer, XtPointer);
void CondExit_CB (Widget, XtPointer, XtPointer);
void AlSmRExit_CB (Widget, XtPointer, XtPointer);
void AlSmRNode_CB (Widget, XtPointer, XtPointer);
void PTSetCondname (int, char *);

void PTAlSmR (Condition_t *cond, Boolean_t *node_used, int condnum, Boolean_t new)
{
  char *condstr, tmpstr[128], *sling;
  String_t string;
  XmString label[6], title;
  int i, sel, node, node2;
/*
  XmFontList flhv18;
  XmFontListEntry helv18;
*/
  XmFontList flhv14;
  XmFontListEntry helv14;

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
    strcpy (condstr, "1060102");
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

  strncpy (tmpstr, glob_condstr + 3, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", &node);
  strncpy (tmpstr, glob_condstr + 5, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", &node2);

  alsmrform = XmCreateForm (condform, "alsmrform", NULL, 0);
  XtVaSetValues (alsmrform,
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

  title = XmStringCreateLocalized ("The ");
  label[0] = XmStringCreateLocalized ("ALKYNE");
  label[1] = XmStringCreateLocalized ("ALLENE");

/*
  alsmrdummy[0] = XmVaCreateSimplePulldownMenu (alsmrform, "_alsmrpulldown0", 0, NULL, XmNbuttonFontList, flhv18, NULL);
*/
  alsmrdummy[0] = XmVaCreateSimplePulldownMenu (alsmrform, "_alsmrpulldown0", 0, NULL, XmNbuttonFontList, flhv14, NULL);

  sel = glob_condstr[2] - '6';
  if (sel == 0) cond_type = PT_TYPE_ALKYNE;
  else cond_type = PT_TYPE_ALLENE;

  alsmrmenu[0] = XmVaCreateSimpleOptionMenu (alsmrform, "alsmrmenu0", title, '\0', sel, AlSmROpt_CB,
      XmNsubMenuId, alsmrdummy[0],
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

  XtVaSetValues (alsmrmenu[0],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftPosition, 0,
    XmNleftAttachment, XmATTACH_FORM,
    NULL);

  title = XmStringCreateLocalized (" defined by nodes");
  alsmrlbl[0] = XtVaCreateWidget ("alsmrlbl0",
               xmLabelWidgetClass, alsmrform,
/*
               XmNfontList, flhv18,
*/
               XmNfontList, flhv14,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_FORM,
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, alsmrmenu[0],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_WIDGET,
               XmNleftWidget, alsmrmenu[0],
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_NONE,
               NULL);
  XmStringFree (title);

  alsmrtxt[0] = XmCreateTextField (alsmrform, "alsmrtxt0", NULL, 0);

  XtVaSetValues (alsmrtxt[0],
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
    XmNtopWidget, alsmrmenu[0],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftPosition, 12,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 50,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized (" AND ");
  alsmrlbl[1] = XtVaCreateWidget ("alsmrlbl1",
               xmLabelWidgetClass, alsmrform,
/*
               XmNfontList, flhv18,
*/
               XmNfontList, flhv14,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, alsmrmenu[0],
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, alsmrtxt[0],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_WIDGET,
               XmNleftWidget, alsmrtxt[0],
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_NONE,
               NULL);
  XmStringFree (title);

  alsmrtxt[1] = XmCreateTextField (alsmrform, "alsmrtxt1", NULL, 0);

  XtVaSetValues (alsmrtxt[1],
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
    XmNtopWidget, alsmrlbl[0],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, alsmrlbl[1],
    XmNrightPosition, 100,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("IS");
  label[1] = XmStringCreateLocalized ("IS NOT");

/*
  alsmrdummy[1] = XmVaCreateSimplePulldownMenu (alsmrform, "_alsmrpulldown1", 0, NULL, XmNbuttonFontList, flhv18, NULL);
*/
  alsmrdummy[1] = XmVaCreateSimplePulldownMenu (alsmrform, "_alsmrpulldown1", 0, NULL, XmNbuttonFontList, flhv14, NULL);

  if (glob_condstr[0] == '1') sel = 0;
  else sel = 1;

  alsmrmenu[1] = XmVaCreateSimpleOptionMenu (alsmrform, "alsmrmenu1", title, '\0', sel, AlSmROpt_CB,
      XmNsubMenuId, alsmrdummy[1],
/*
      XmNbuttonFontList, flhv18,
*/
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

  XtVaSetValues (alsmrmenu[1],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, alsmrtxt[0],
    XmNleftPosition, 12,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized (" in a small ring.");
  alsmrlbl[2] = XtVaCreateWidget ("alsmrlbl2",
               xmLabelWidgetClass, alsmrform,
/*
               XmNfontList, flhv18,
*/
               XmNfontList, flhv14,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, alsmrlbl[1],
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, alsmrmenu[1],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_WIDGET,
               XmNleftWidget, alsmrmenu[1],
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_NONE,
               NULL);
  XmStringFree (title);

  for (i = 0; i < 2; i++)
    XtAddCallback (alsmrtxt[i], XmNvalueChangedCallback, AlSmRNode_CB, (XtPointer) i);

  AlSmR_Display (glob_condstr, node, node2);
}

void AlSmR_Display (char *v, int node, int node2)
{
  int i, sel;
  char nodelist[256], tmpstr[5];

  XtManageChild (alsmrmenu[0]);

  XtManageChild (alsmrlbl[0]);

  sprintf (nodelist, "%02d", node);

  XtVaSetValues (alsmrtxt[0],
    XmNvalue, nodelist,
    NULL);

  XtManageChild (alsmrtxt[0]);

  XtManageChild (alsmrlbl[1]);

  sprintf (nodelist, "%02d", node2);

  XtVaSetValues (alsmrtxt[1],
    XmNvalue, nodelist,
    NULL);

  XtManageChild (alsmrtxt[1]);

  XtManageChild (alsmrmenu[1]);

  XtManageChild (alsmrlbl[2]);

  XtManageChild (alsmrform);
  XtManageChild (ptformdg);
}

void AlSmROpt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *wname, tmpstr[128], *ts, *comma, *space;
  int sel, sel2, node, node2;
  XmString title;

  wname = XtName (XtParent (w));
  sel = (int) client_data;

  if (strstr (wname, "0") != NULL)
    {
    glob_condstr[2] = sel + '6';
    if (sel == 0) cond_type = PT_TYPE_ALKYNE;
    else cond_type = PT_TYPE_ALLENE;
    }
  else if (strstr (wname, "1") != NULL)
    {
    if (sel == 0) glob_condstr[0] = '1';
    else glob_condstr[0] = '0';
    }
}

void AlSmRNode_CB (Widget w, XtPointer client_data, XtPointer call_data)
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

void AlSmRExit_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  String_t string;
  char sling[128], tmpstr[128], tmpstr2[128], savestr[128], *ts, *ts2, *comma, *space, *newstr;
  int i, node, node2;

  if ((Boolean_t) (int) client_data)
    {
/* first get final results of text fields */
    XtVaGetValues (alsmrtxt[0],
      XmNvalue, &ts,
      NULL);

    if (ts[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }
    sscanf (ts, "%d", &node);

    XtVaGetValues (alsmrtxt[1],
      XmNvalue, &ts2,
      NULL);

    if (ts2[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }
    sscanf (ts2, "%d", &node2);
    sprintf (tmpstr, "%02d%02d", node, node2);

/* incorporate into string */
    strncpy (glob_condstr + 3, tmpstr, 4);

    newstr = (char *) malloc (strlen (glob_condstr) + 1);
    strcpy (newstr, glob_condstr);
    String_Value_Put (string, newstr);
    String_Length_Put (string, strlen (newstr));
    String_Alloc_Put (string, strlen (newstr) + 1);

    condition_export (glob_cond, &string, glob_nu);
    }
  else if (new_cond) cond_add_canceled = TRUE;

  XtUnmanageChild (ptformdg);
  XtDestroyWidget (alsmrform);
}
