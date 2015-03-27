/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              PTCONN.C
*
*    This module provides for the display and editing of the posttransform
*    conditions for determining or comparing connectivity between variable
*    nodes.
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

static Widget connform, connlbl[3], conntxt[4], connmenu[3], conndummy[3];
static Condition_t *glob_cond;
static char glob_condstr[1024], namestr[128];
static Boolean_t *glob_nu, new_cond;
static int connmenu1sel;
static char *connops[6] = {"> ", ">=", "= ", "~=", "< ", "<="};
void Post_Cond_Put (Condition_t *);

void Conn_Display (char *, int *, int *, Boolean_t);
void ConnOpt_CB (Widget, XtPointer, XtPointer);
void CondExit_CB (Widget, XtPointer, XtPointer);
void ConnExit_CB (Widget, XtPointer, XtPointer);
void ConnNode_CB (Widget, XtPointer, XtPointer);
void PTSetCondname (int, char *);
void PTUpdateCondname (char *, char *);

void PTConn (Condition_t *cond, Boolean_t *node_used, int condnum, Boolean_t new, char *new_type)
{
  char *condstr, tmpstr[128], *sling;
  String_t string;
  XmString label[6], title;
  int i, sel, node[2], node2[2];
/*
  XmFontList flhv18;
  XmFontListEntry helv18;
*/
  XmFontList flhv14;
  XmFontListEntry helv14;
  Boolean_t measure;

  cond_type = PT_TYPE_PATHLEN;

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
    if (strcmp (new_type, "CONN") == 0) strcpy (condstr, "1050102++0000");
    else if (strcmp (new_type, "RNGSZ") == 0) strcpy (condstr, "1050102>=0001");
    else strcpy (condstr, "1050102>=0304");
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

  PTSetCondname (condnum, namestr);

  title = XmStringCreateLocalized (namestr);
  XtVaSetValues (condlbl,
    XmNlabelString, title,
    NULL);
  XmStringFree (title);

  connform = XmCreateForm (condform, "connform", NULL, 0);
  XtVaSetValues (connform,
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

  title = XmStringCreateLocalized ("The path (outside of the matched part of the target molecule) between nodes");
  connlbl[0] = XtVaCreateWidget ("connlbl0",
               xmLabelWidgetClass, connform,
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
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  conntxt[0] = XmCreateTextField (connform, "conntxt0", NULL, 0);

  XtVaSetValues (conntxt[0],
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
    XmNtopWidget, connlbl[0],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftPosition, 12,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 50,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized (" AND ");
  connlbl[1] = XtVaCreateWidget ("connlbl1",
               xmLabelWidgetClass, connform,
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
               XmNtopWidget, connlbl[0],
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, conntxt[0],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_WIDGET,
               XmNleftWidget, conntxt[0],
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_NONE,
               NULL);
  XmStringFree (title);

  conntxt[1] = XmCreateTextField (connform, "conntxt1", NULL, 0);

  XtVaSetValues (conntxt[1],
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
    XmNtopWidget, connlbl[0],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, connlbl[1],
    XmNrightPosition, 100,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("EXISTS");
  label[1] = XmStringCreateLocalized ("DOES NOT EXIST");
  label[2] = XmStringCreateLocalized ("IS");
  label[3] = XmStringCreateLocalized ("IS NOT");

/*
  conndummy[0] = XmVaCreateSimplePulldownMenu (connform, "_connpulldown0", 0, NULL, XmNbuttonFontList, flhv18, NULL);
*/
  conndummy[0] = XmVaCreateSimplePulldownMenu (connform, "_connpulldown0", 0, NULL, XmNbuttonFontList, flhv14, NULL);

  if (glob_condstr[0] == '0') sel = 1;
  else sel = 0;
  if (measure = (glob_condstr[7] != '+')) sel |= 2;

  connmenu[0] = XmVaCreateSimpleOptionMenu (connform, "connmenu0", title, '\0', sel, ConnOpt_CB,
      XmNsubMenuId, conndummy[0],
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
      XmVaPUSHBUTTON, label[2], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[3], '\0', NULL, NULL,
      NULL);

  XmStringFree (title);
  XmStringFree (label[0]);
  XmStringFree (label[1]);
  XmStringFree (label[2]);
  XmStringFree (label[3]);

  XtVaSetValues (connmenu[0],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, conntxt[0],
    XmNleftPosition, 12,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("GREATER THAN");
  label[1] = XmStringCreateLocalized ("GREATER THAN OR EQUAL TO");
  label[2] = XmStringCreateLocalized ("EQUAL TO");
  label[3] = XmStringCreateLocalized ("UNEQUAL TO");
  label[4] = XmStringCreateLocalized ("LESS THAN");
  label[5] = XmStringCreateLocalized ("LESS THAN OR EQUAL TO");

/*
  conndummy[1] = XmVaCreateSimplePulldownMenu (connform, "_connpulldown1", 0, NULL, XmNbuttonFontList, flhv18, NULL);
*/
  conndummy[1] = XmVaCreateSimplePulldownMenu (connform, "_connpulldown1", 0, NULL, XmNbuttonFontList, flhv14, NULL);

  strncpy (tmpstr, glob_condstr + 7, 2);
  tmpstr[2] = '\0';
  connmenu1sel = -1;
  for (i = 0; i < 6 && connmenu1sel < 0; i++)
    if (strcmp (tmpstr, connops[i]) == 0) connmenu1sel = i;
  if (connmenu1sel < 0) connmenu1sel = 2;

  connmenu[1] = XmVaCreateSimpleOptionMenu (connform, "connmenu1", title, '\0', connmenu1sel, ConnOpt_CB,
      XmNsubMenuId, conndummy[1],
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

  XtVaSetValues (connmenu[1],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, conntxt[0],
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, connmenu[0],
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("a total of ");
  label[1] = XmStringCreateLocalized ("the path connecting nodes ");

/*
  conndummy[2] = XmVaCreateSimplePulldownMenu (connform, "_connpulldown2", 0, NULL, XmNbuttonFontList, flhv18, NULL);
*/
  conndummy[2] = XmVaCreateSimplePulldownMenu (connform, "_connpulldown2", 0, NULL, XmNbuttonFontList, flhv14, NULL);

  strncpy (tmpstr, glob_condstr + 3, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", node);
  strncpy (tmpstr, glob_condstr + 5, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", node2);
  if (glob_condstr[9] == '\0') node[1] = node2[1] = sel = 0;
  else
    {
    strncpy (tmpstr, glob_condstr + 9, 2);
    tmpstr[2] = '\0';
    sscanf (tmpstr, "%d", node + 1);
    strncpy (tmpstr, glob_condstr + 11, 2);
    tmpstr[2] = '\0';
    sscanf (tmpstr, "%d", node2 + 1);
    if (node[1] == 0) sel = 0;
    else sel = 1;
    }

  connmenu[2] = XmVaCreateSimpleOptionMenu (connform, "connmenu2", title, '\0', sel, ConnOpt_CB,
      XmNsubMenuId, conndummy[2],
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

  XtVaSetValues (connmenu[2],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, connmenu[0],
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    NULL);

  conntxt[2] = XmCreateTextField (connform, "conntxt2", NULL, 0);

  XtVaSetValues (conntxt[2],
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
    XmNtopWidget, connmenu[2],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftPosition, 12,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 50,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  if (node[1] == 0)
    title = XmStringCreateLocalized (" BOND LENGTHS.");
  else
    title = XmStringCreateLocalized (" AND ");
  connlbl[2] = XtVaCreateWidget ("connlbl2",
               xmLabelWidgetClass, connform,
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
               XmNtopWidget, connmenu[2],
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, conntxt[2],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_WIDGET,
               XmNleftWidget, conntxt[2],
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_NONE,
               NULL);
  XmStringFree (title);

  conntxt[3] = XmCreateTextField (connform, "conntxt3", NULL, 0);

  XtVaSetValues (conntxt[3],
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
    XmNtopWidget, connmenu[2],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, connlbl[2],
    XmNrightPosition, 100,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  for (i = 0; i < 4; i++)
    XtAddCallback (conntxt[i], XmNvalueChangedCallback, ConnNode_CB, (XtPointer) i);

  Conn_Display (glob_condstr, node, node2, measure);
}

void Conn_Display (char *v, int *node, int *node2, Boolean_t measure)
{
  int i, sel;
  char nodelist[256], tmpstr[5];

  XtManageChild (connlbl[0]);

  sprintf (nodelist, "%02d", node[0]);

  XtVaSetValues (conntxt[0],
    XmNvalue, nodelist,
    NULL);

  XtManageChild (conntxt[0]);

  XtManageChild (connlbl[1]);

  sprintf (nodelist, "%02d", node2[0]);

  XtVaSetValues (conntxt[1],
    XmNvalue, nodelist,
    NULL);

  XtManageChild (conntxt[1]);

  XtManageChild (connmenu[0]);
  XtManageChild (connmenu[1]);
  XtManageChild (connmenu[2]);

  if (measure)
    {
    if (node[1] == 0)
      sprintf (nodelist, "%02d", node2[1]);
    else
      sprintf (nodelist, "%02d", node[1]);

    XtVaSetValues (conntxt[2],
      XmNvalue, nodelist,
      NULL);
    }

  XtManageChild (conntxt[2]);

  XtManageChild (connlbl[2]);

  if (measure && node[1] != 0)
    {
    sprintf (nodelist, "%02d", node2[1]);

    XtVaSetValues (conntxt[3],
      XmNvalue, nodelist,
      NULL);
    }

  XtManageChild (conntxt[3]);

  XtManageChild (connform);
  XtManageChild (ptformdg);

  if (!measure)
    {
    XtUnmanageChild (connmenu[1]);
    XtUnmanageChild (connmenu[2]);
    XtUnmanageChild (conntxt[2]);
    XtUnmanageChild (connlbl[2]);
    XtUnmanageChild (conntxt[3]);
    }
  else if (node[1] == 0)
    XtUnmanageChild (conntxt[3]);
}

void ConnOpt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *wname, tmpstr[128], *ts, *comma, *space;
  int sel, sel2, node, node2;
  XmString title;

  wname = XtName (XtParent (w));
  sel = (int) client_data;

  if (strstr (wname, "0") != NULL)
    {
    if ((sel & 2) == 0)
      {
      PTUpdateCondname (namestr, "CONN");
      title = XmStringCreateLocalized (namestr);
      XtVaSetValues (condlbl,
        XmNlabelString, title,
        NULL);
      XmStringFree (title);
      XtUnmanageChild (connmenu[1]);
      XtUnmanageChild (connmenu[2]);
      XtUnmanageChild (conntxt[2]);
      XtUnmanageChild (connlbl[2]);
      XtUnmanageChild (conntxt[3]);
      strcpy (glob_condstr + 7, "++0000");
      }
    else
      {
      XtManageChild (connmenu[1]);
      XtManageChild (connmenu[2]);
      XtManageChild (conntxt[2]);
      XtManageChild (connlbl[2]);
      strncpy (glob_condstr + 7, connops[connmenu1sel], 2);
      strncpy (tmpstr, glob_condstr + 9, 2);
      tmpstr[2] = '\0';
      sscanf (tmpstr, "%d", &node);
      if (node == 0)
        {
        PTUpdateCondname (namestr, "RNGSZ");
        title = XmStringCreateLocalized (namestr);
        XtVaSetValues (condlbl,
          XmNlabelString, title,
          NULL);
        XmStringFree (title);
        XtUnmanageChild (conntxt[3]);
        }
      else
        {
        PTUpdateCondname (namestr, "RNGCP");
        title = XmStringCreateLocalized (namestr);
        XtVaSetValues (condlbl,
          XmNlabelString, title,
          NULL);
        XmStringFree (title);
        XtManageChild (conntxt[3]);
        }
      }
    glob_condstr[0] = (sel & 1) == 0 ? '1' : '0';
    }
  else if (strstr (wname, "1") != NULL)
    {
    connmenu1sel = sel;
    strncpy (glob_condstr + 7, connops[connmenu1sel], 2);
    }
  else if (strstr (wname, "2") != NULL)
    {
    if (sel == 0)
      {
      PTUpdateCondname (namestr, "RNGSZ");
      title = XmStringCreateLocalized (namestr);
      XtVaSetValues (condlbl,
        XmNlabelString, title,
        NULL);
      XmStringFree (title);
      title = XmStringCreateLocalized (" BOND LENGTHS.");
      XtVaSetValues (connlbl[2],
        XmNlabelString, title,
        NULL);
      XmStringFree (title);

      XtVaGetValues (conntxt[2],
        XmNvalue, &ts,
        NULL);
      if (ts[0] != '\0')
        {
        if (ts[1] == '\0')
          {
          ts[2] = ts[1];
          ts[1] = ts[0];
          ts[0] = '0';
          }
        sprintf (tmpstr, "00%s", ts);
        strncpy (glob_condstr + 9, tmpstr, 4);
        }
      XtUnmanageChild (conntxt[3]);
      }
    else
      {
      PTUpdateCondname (namestr, "RNGCP");
      title = XmStringCreateLocalized (namestr);
      XtVaSetValues (condlbl,
        XmNlabelString, title,
        NULL);
      XmStringFree (title);
      title = XmStringCreateLocalized (" AND ");
      XtVaSetValues (connlbl[2],
        XmNlabelString, title,
        NULL);
      XmStringFree (title);

      XtVaGetValues (conntxt[2],
        XmNvalue, &ts,
        NULL);
      if (ts[0] != '\0')
        {
        if (ts[1] == '\0')
          {
          ts[2] = ts[1];
          ts[1] = ts[0];
          ts[0] = '0';
          }
        sscanf (ts, "%d", &node);
        }
      else node = 0;

      XtVaGetValues (conntxt[3],
        XmNvalue, &ts,
        NULL);
      if (ts[0] != '\0')
        {
        if (ts[1] == '\0')
          {
          ts[2] = ts[1];
          ts[1] = ts[0];
          ts[0] = '0';
          }
        sscanf (ts, "%d", &node2);
        }
      else node2 = 0;

      sprintf (tmpstr, "%02d%02d", node, node2);
      strncpy (glob_condstr + 9, tmpstr, 4);

      XtManageChild (conntxt[3]);
      }
    }
}

void ConnNode_CB (Widget w, XtPointer client_data, XtPointer call_data)
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

void ConnExit_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  String_t string;
  char sling[128], tmpstr[128], tmpstr2[128], savestr[128], *ts, *ts2, *comma, *space, *newstr;
  int i, node[2], node2[2];

  if ((Boolean_t) (int) client_data)
    {
/* first get final results of text fields */
    XtVaGetValues (conntxt[0],
      XmNvalue, &ts,
      NULL);

    if (ts[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }
    sscanf (ts, "%d", node);

    XtVaGetValues (conntxt[1],
      XmNvalue, &ts2,
      NULL);

    if (ts2[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }
    sscanf (ts2, "%d", node2);
    sprintf (tmpstr, "%02d%02d", node[0], node2[0]);

    if (XtIsManaged (conntxt[2]))
      {
      XtVaGetValues (conntxt[2],
        XmNvalue, &ts,
        NULL);

      if (ts[0] == '\0')
        {
        XtManageChild (emptytextmsg);
        return;
        }
      sscanf (ts, "%d", node + 1);

      if (XtIsManaged (conntxt[3]))
        {
        XtVaGetValues (conntxt[3],
          XmNvalue, &ts2,
          NULL);

        if (ts2[0] == '\0')
          {
          XtManageChild (emptytextmsg);
          return;
          }
        sscanf (ts2, "%d", node2 + 1);
        }
      else
        {
        node2[1] = node[1];
        node[1] = 0;
        }
      }

    else node[1] = node2[1] = 0;
    sprintf (tmpstr2, "%02d%02d", node[1], node2[1]);

/* incorporate into string */
    strncpy (glob_condstr + 3, tmpstr, 4);
    strcpy (glob_condstr + 9, tmpstr2);

    newstr = (char *) malloc (strlen (glob_condstr) + 1);
    strcpy (newstr, glob_condstr);
    String_Value_Put (string, newstr);
    String_Length_Put (string, strlen (newstr));
    String_Alloc_Put (string, strlen (newstr) + 1);

    condition_export (glob_cond, &string, glob_nu);
    }
  else if (new_cond) cond_add_canceled = TRUE;

  XtUnmanageChild (ptformdg);
  XtDestroyWidget (connform);
}
