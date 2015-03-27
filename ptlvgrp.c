/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              PTLVGRP.C
*
*    This module provides for the display and editing of the posttransform
*    conditions for determining the leaving group tendency at a given node.
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

static Widget lvgrpform, lvgrplbl[7], lvgrptxt[4], lvgrpmenu[6], lvgrpdummy[6];
static Condition_t *glob_cond;
static char glob_condstr[1024];
static int condsel = 1;
static Boolean_t *glob_nu, new_cond;

void Post_Cond_Put (Condition_t *);
void PTSetCondname (int, char *);

void fix_sling_case (char *);
void LvGrp_Display (char *, int *, int *, int *);
void LvGrpOpt_CB (Widget, XtPointer, XtPointer);
void CondExit_CB (Widget, XtPointer, XtPointer);
void LvGrpExit_CB (Widget, XtPointer, XtPointer);
void LvGrpNode_CB (Widget, XtPointer, XtPointer);

static char *cond_desc[3] =
  {"ACIDIC",
   "NEUTRAL",
   "BASIC"};

void PTLvGrp (Condition_t *cond, Boolean_t *node_used, int condnum, Boolean_t new)
{
  char *condstr, tmpstr[128];
  String_t string;
  XmString label[NATTRIB + 1], title;
  int i, sel, node[2], dist[2], rxncond[2];
  XmFontList flhv18;
  XmFontListEntry helv18;

  cond_type = PT_TYPE_LVNGROUP;

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
    strcpy (condstr, "10901002>=0005+");
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

  lvgrpform = XmCreateForm (condform, "lvgrpform", NULL, 0);
  XtVaSetValues (lvgrpform,
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

  title = XmStringCreateLocalized ("Under ");
  for (i = 0; i < 3; i++)
    label[i] = XmStringCreateLocalized (cond_desc[i]);

  lvgrpdummy[0] = XmVaCreateSimplePulldownMenu (lvgrpform, "_lvgrppulldown0", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  strncpy (tmpstr, glob_condstr + 3, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", node);
  strncpy (tmpstr, glob_condstr + 5, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", dist);
  strncpy (tmpstr, glob_condstr + 10, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", node + 1);
  strncpy (tmpstr, glob_condstr + 12, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", dist + 1);
  rxncond[0] = glob_condstr[7] - '1';
  if (glob_condstr[14] == '+') rxncond[1] = condsel = 1;
  else rxncond[1] = condsel = glob_condstr[14] - '1';

  lvgrpmenu[0] = XmVaCreateSimpleOptionMenu (lvgrpform, "lvgrpmenu0", title, '\0', rxncond[0], LvGrpOpt_CB,
      XmNsubMenuId, lvgrpdummy[0],
      XmNlabelFontList, flhv18,
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
  for (i = 0; i < 3; i++)
    XmStringFree (label[i]);

  XtVaSetValues (lvgrpmenu[0],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    NULL);

  title = XmStringCreateLocalized (" conditions there ");
  label[0] = XmStringCreateLocalized ("EXISTS");
  label[1] = XmStringCreateLocalized ("DOES NOT EXIST");

  lvgrpdummy[1] = XmVaCreateSimplePulldownMenu (lvgrpform, "_lvgrppulldown1", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  if (glob_condstr[0] == '0') sel = 1;
  else sel = 0;

  lvgrpmenu[1] = XmVaCreateSimpleOptionMenu (lvgrpform, "lvgrpmenu1", title, '\0', sel, LvGrpOpt_CB,
      XmNsubMenuId, lvgrpdummy[1],
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

  XtVaSetValues (lvgrpmenu[1],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, lvgrpmenu[0],
    NULL);

  title = XmStringCreateLocalized (" a leaving group at distance");
  lvgrplbl[0] = XtVaCreateWidget ("lvgrplbl0",
               xmLabelWidgetClass, lvgrpform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_FORM,
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, lvgrpmenu[1],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_WIDGET,
               XmNleftWidget, lvgrpmenu[1],
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_NONE,
               NULL);
  XmStringFree (title);

  lvgrptxt[0] = XmCreateTextField (lvgrpform, "lvgrptxt0", NULL, 0);

  XtVaSetValues (lvgrptxt[0],
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, lvgrpmenu[0],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftPosition, 25,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 50,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("in the part of the target molecule defined by node");
  lvgrplbl[1] = XtVaCreateWidget ("lvgrplbl1",
               xmLabelWidgetClass, lvgrpform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, lvgrptxt[0],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  lvgrptxt[1] = XmCreateTextField (lvgrpform, "lvgrptxt1", NULL, 0);

  XtVaSetValues (lvgrptxt[1],
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, lvgrplbl[1],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftPosition, 25,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 50,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("in the goal pattern whose leaving ability is");
  label[0] = XmStringCreateLocalized ("GREATER THAN");
  label[1] = XmStringCreateLocalized ("GREATER THAN OR EQUAL TO");
  label[2] = XmStringCreateLocalized ("EQUAL TO");
  label[3] = XmStringCreateLocalized ("UNEQUAL TO");
  label[4] = XmStringCreateLocalized ("LESS THAN");
  label[5] = XmStringCreateLocalized ("LESS THAN OR EQUAL TO");

  lvgrpdummy[2] = XmVaCreateSimplePulldownMenu (lvgrpform, "_lvgrppulldown2", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  strncpy (tmpstr, glob_condstr + 8, 2);
  tmpstr[2] = '\0';
  if (strcmp (tmpstr, "> ") == 0) sel = 0;
  else if (strcmp (tmpstr, ">=") == 0) sel = 1;
  else if (strcmp (tmpstr, "= ") == 0) sel = 2;
  else if (strcmp (tmpstr, "~=") == 0) sel = 3;
  else if (strcmp (tmpstr, "< ") == 0) sel = 4;
  else if (strcmp (tmpstr, "<=") == 0) sel = 5;

  lvgrpmenu[2] = XmVaCreateSimpleOptionMenu (lvgrpform, "lvgrpmenu2", title, '\0', sel, LvGrpOpt_CB,
      XmNsubMenuId, lvgrpdummy[2],
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

  XtVaSetValues (lvgrpmenu[2],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, lvgrptxt[1],
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("THE VALUE (0=VERY POOR; 10=VERY GOOD) ...");
  label[1] = XmStringCreateLocalized ("THAT OF THE LEAVING GROUP AT ...");

  lvgrpdummy[3] = XmVaCreateSimplePulldownMenu (lvgrpform, "_lvgrppulldown3", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  if (node[1] == 0) sel = 0;
  else sel = 1;

  lvgrpmenu[3] = XmVaCreateSimpleOptionMenu (lvgrpform, "lvgrpmenu3", title, '\0', sel, LvGrpOpt_CB,
      XmNsubMenuId, lvgrpdummy[3],
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

  XtVaSetValues (lvgrpmenu[3],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, lvgrpmenu[2],
    XmNleftPosition, 25,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized (" 0=VERY POOR");
  label[1] = XmStringCreateLocalized (" 1");
  label[2] = XmStringCreateLocalized (" 2");
  label[3] = XmStringCreateLocalized (" 3");
  label[4] = XmStringCreateLocalized (" 4");
  label[5] = XmStringCreateLocalized (" 5");
  label[6] = XmStringCreateLocalized (" 6");
  label[7] = XmStringCreateLocalized (" 7");
  label[8] = XmStringCreateLocalized (" 8");
  label[9] = XmStringCreateLocalized (" 9");
  label[10] = XmStringCreateLocalized ("10=VERY GOOD");

  lvgrpdummy[4] = XmVaCreateSimplePulldownMenu (lvgrpform, "_lvgrppulldown4", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  sel = dist[1];

  lvgrpmenu[4] = XmVaCreateSimpleOptionMenu (lvgrpform, "lvgrpmenu4", title, '\0', sel, LvGrpOpt_CB,
      XmNsubMenuId, lvgrpdummy[4],
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
      XmVaPUSHBUTTON, label[6], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[7], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[8], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[9], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[10], '\0', NULL, NULL,
      NULL);

  XmStringFree (title);
  XmStringFree (label[0]);
  XmStringFree (label[1]);
  XmStringFree (label[2]);
  XmStringFree (label[3]);
  XmStringFree (label[4]);
  XmStringFree (label[5]);
  XmStringFree (label[6]);
  XmStringFree (label[7]);
  XmStringFree (label[8]);
  XmStringFree (label[9]);
  XmStringFree (label[10]);

  XtVaSetValues (lvgrpmenu[4],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, lvgrpmenu[2],
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, lvgrpmenu[3],
    NULL);

  title = XmStringCreateLocalized ("DISTANCE ");
  lvgrplbl[2] = XtVaCreateWidget ("lvgrplbl2",
               xmLabelWidgetClass, lvgrpform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, lvgrpmenu[3],
               XmNleftPosition, 25,
               XmNleftAttachment, XmATTACH_POSITION,
               NULL);
  XmStringFree (title);

  lvgrptxt[2] = XmCreateTextField (lvgrpform, "lvgrptxt2", NULL, 0);

  XtVaSetValues (lvgrptxt[2],
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, lvgrpmenu[3],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, lvgrplbl[2],
    XmNrightPosition, 75,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("DISTANCE ");
  lvgrplbl[3] = XtVaCreateWidget ("lvgrplbl3",
               xmLabelWidgetClass, lvgrpform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNtopWidget, lvgrptxt[2],
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, lvgrptxt[2],
               XmNleftPosition, 25,
               XmNleftAttachment, XmATTACH_POSITION,
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_WIDGET,
               XmNrightWidget, lvgrptxt[2],
               NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized ("IN THE PART OF THE TARGET MOLECULE DEFINED BY NODE ");
  lvgrplbl[4] = XtVaCreateWidget ("lvgrplbl4",
               xmLabelWidgetClass, lvgrpform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, lvgrplbl[3],
               XmNleftPosition, 25,
               XmNleftAttachment, XmATTACH_POSITION,
               NULL);
  XmStringFree (title);

  lvgrptxt[3] = XmCreateTextField (lvgrpform, "lvgrptxt3", NULL, 0);

  XtVaSetValues (lvgrptxt[3],
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, lvgrptxt[2],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, lvgrplbl[4],
    XmNrightPosition, 175,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("IN THE PART OF THE TARGET MOLECULE DEFINED BY NODE ");
  lvgrplbl[5] = XtVaCreateWidget ("lvgrplbl5",
               xmLabelWidgetClass, lvgrpform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNtopWidget, lvgrptxt[3],
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, lvgrptxt[3],
               XmNleftPosition, 25,
               XmNleftAttachment, XmATTACH_POSITION,
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_WIDGET,
               XmNrightWidget, lvgrptxt[3],
               NULL);
  XmStringFree (title);

/*
  title = XmStringCreateLocalized ("UNDER ");
  for (i = 0; i < 3; i++)
    label[i] = XmStringCreateLocalized (cond_desc[i]);

  lvgrpdummy[5] = XmVaCreateSimplePulldownMenu (lvgrpform, "_lvgrppulldown5", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  lvgrpmenu[5] = XmVaCreateSimpleOptionMenu (lvgrpform, "lvgrpmenu5", title, '\0', rxncond[1], LvGrpOpt_CB,
      XmNsubMenuId, lvgrpdummy[5],
      XmNlabelFontList, flhv18,
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
  for (i = 0; i < 3; i++)
    XmStringFree (label[i]);

  XtVaSetValues (lvgrpmenu[5],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, lvgrplbl[5],
    XmNleftPosition, 25,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized (" CONDITIONS.");
  lvgrplbl[6] = XtVaCreateWidget ("lvgrplbl6",
               xmLabelWidgetClass, lvgrpform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, lvgrplbl[5],
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, lvgrpmenu[5],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_WIDGET,
               XmNleftWidget, lvgrpmenu[5],
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_NONE,
               NULL);
  XmStringFree (title);
*/

  for (i = 0; i < 4; i++)
    XtAddCallback (lvgrptxt[i], XmNvalueChangedCallback, LvGrpNode_CB, (XtPointer) i);

  LvGrp_Display (glob_condstr, dist, node, rxncond);
}

void LvGrp_Display (char *v, int *dist, int *node, int *cond)
{
  int i, sel;
  char nodelist[256], tmpstr[5];

  XtManageChild (lvgrpmenu[0]);
  XtManageChild (lvgrpmenu[1]);

  XtManageChild (lvgrplbl[0]);

  sprintf (nodelist, "%02d", dist[0]);

  XtVaSetValues (lvgrptxt[0],
    XmNvalue, nodelist,
    NULL);

  XtManageChild (lvgrptxt[0]);

  XtManageChild (lvgrplbl[1]);

  sprintf (nodelist, "%02d", node[0]);

  XtVaSetValues (lvgrptxt[1],
    XmNvalue, nodelist,
    NULL);

  XtManageChild (lvgrptxt[1]);

  XtManageChild (lvgrpmenu[2]);
  XtManageChild (lvgrpmenu[3]);
  XtManageChild (lvgrpmenu[4]);

  XtManageChild (lvgrplbl[2]);

  if (node[1] != 0)
    {
    sprintf (nodelist, "%02d", dist[1]);

    XtVaSetValues (lvgrptxt[2],
      XmNvalue, nodelist,
      NULL);
    }

  XtManageChild (lvgrptxt[2]);

  XtManageChild (lvgrplbl[3]);

  XtManageChild (lvgrplbl[4]);

  if (node[1] != 0)
    {
    sprintf (nodelist, "%02d", node[1]);

    XtVaSetValues (lvgrptxt[3],
      XmNvalue, nodelist,
      NULL);
    }

  XtManageChild (lvgrptxt[3]);

  XtManageChild (lvgrplbl[5]);

/*
  XtManageChild (lvgrpmenu[5]);

  XtManageChild (lvgrplbl[6]);
*/

  XtManageChild (lvgrpform);
  XtManageChild (ptformdg);

  XtUnmanageChild (lvgrplbl[2]);
  XtUnmanageChild (lvgrplbl[4]);

  if (node[1] == 0)
    {
    XtUnmanageChild (lvgrplbl[3]);
    XtUnmanageChild (lvgrptxt[2]);
    XtUnmanageChild (lvgrplbl[5]);
    XtUnmanageChild (lvgrptxt[3]);
/*
    XtUnmanageChild (lvgrpmenu[5]);
    XtUnmanageChild (lvgrplbl[6]);
*/
    }
  else
    XtUnmanageChild (lvgrpmenu[4]);
}

void LvGrpOpt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *wname, tmpstr[128], *ts, *comma, *space;
  int sel, sel2, nnodes, nnodes2;

  wname = XtName (XtParent (w));
  sel = (int) client_data;

  if (strstr (wname, "0") != NULL)
    {
    glob_condstr[7] = sel + '1';
    condsel = sel;
    }
  else if (strstr (wname, "1") != NULL) glob_condstr[0] = sel == 0 ? '1' : '0';
  else if (strstr (wname, "2") != NULL) switch (sel)
    {
  case 0:
    strncpy (glob_condstr + 8, "> ", 2);
    break;
  case 1:
    strncpy (glob_condstr + 8, ">=", 2);
    break;
  case 2:
    strncpy (glob_condstr + 8, "= ", 2);
    break;
  case 3:
    strncpy (glob_condstr + 8, "~=", 2);
    break;
  case 4:
    strncpy (glob_condstr + 8, "< ", 2);
    break;
  case 5:
    strncpy (glob_condstr + 8, "<=", 2);
    break;
    }
  else if (strstr (wname, "3") != NULL) switch (sel)
    {
  case 0:
    XmTextReplace (lvgrptxt[3],0,2,"");
    XtUnmanageChild (lvgrplbl[3]);
    XtUnmanageChild (lvgrptxt[2]);
    XtUnmanageChild (lvgrplbl[5]);
    XtUnmanageChild (lvgrptxt[3]);
/*
    XtUnmanageChild (lvgrpmenu[5]);
    XtUnmanageChild (lvgrplbl[6]);
*/
    XtManageChild (lvgrpmenu[4]);
    strncpy (glob_condstr + 10, "00", 2);
    glob_condstr[14] = '+';
    break;
  case 1:
    XtUnmanageChild (lvgrpmenu[4]);
    XtManageChild (lvgrplbl[3]);
    XtManageChild (lvgrptxt[2]);
    XtManageChild (lvgrplbl[5]);
    XtManageChild (lvgrptxt[3]);
/*
    XtManageChild (lvgrpmenu[5]);
    XtManageChild (lvgrplbl[6]);
*/
    glob_condstr[14] = condsel + '1';
    break;
    }
  else if (strstr (wname, "4") != NULL)
    {
    sprintf (tmpstr, "%02d", sel);
    strncpy (glob_condstr + 12, tmpstr, 2);
    }
/*
  else if (strstr (wname, "5") != NULL)
    glob_condstr[14] = sel + '1';
*/
}

void LvGrpNode_CB (Widget w, XtPointer client_data, XtPointer call_data)
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

void LvGrpExit_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  String_t string;
  char sling[128], tmpstr[128], tmpstr2[128], savestr[128], *ts, *ts2, *comma, *space, *newstr;
  int i, node, cond, dist;

  if ((Boolean_t) (int) client_data)
    {
/* first get final results of text fields */
    XtVaGetValues (lvgrptxt[0],
      XmNvalue, &ts,
      NULL);

    if (ts[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }
    sscanf (ts, "%d", &dist);

    XtVaGetValues (lvgrptxt[1],
      XmNvalue, &ts2,
      NULL);

    if (ts2[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }
    sscanf (ts2, "%d", &node);
    sprintf (tmpstr, "%02d%02d", node, dist);

    if (XtIsManaged (lvgrptxt[2]) && XtIsManaged (lvgrptxt[3]))
      {
      XtVaGetValues (lvgrptxt[2],
        XmNvalue, &ts,
        NULL);

      if (ts[0] == '\0')
        {
        XtManageChild (emptytextmsg);
        return;
        }
      sscanf (ts, "%d", &dist);

      XtVaGetValues (lvgrptxt[3],
        XmNvalue, &ts2,
        NULL);

      if (ts2[0] == '\0')
        {
        XtManageChild (emptytextmsg);
        return;
        }
      sscanf (ts2, "%d", &node);
      sprintf (tmpstr2, "%02d%02d", node, dist);
      }
    else tmpstr2[0] = '\0';

/* incorporate into string */
    strncpy (glob_condstr + 3, tmpstr, 4);
    if (tmpstr2[0] == '\0')
      glob_condstr[14] = '+';
    else
      strncpy (glob_condstr + 10, tmpstr2, 4);

    newstr = (char *) malloc (strlen (glob_condstr) + 1);
    strcpy (newstr, glob_condstr);
    String_Value_Put (string, newstr);
    String_Length_Put (string, strlen (newstr));
    String_Alloc_Put (string, strlen (newstr) + 1);

    condition_export (glob_cond, &string, glob_nu);
    }
  else if (new_cond) cond_add_canceled = TRUE;

  XtUnmanageChild (ptformdg);
  XtDestroyWidget (lvgrpform);
}
