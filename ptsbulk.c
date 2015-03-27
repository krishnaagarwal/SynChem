/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              PTSBULK.C
*
*    This module provides for the display and editing of the posttransform
*    condition type SBULK, which compares nodes or edges against each other
*    (or against a typical substituent) for relative steric bulk.
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
/*
char *elec (char *);
*/

extern int cond_type;
extern Widget pttl, ptformdg, condform, condpb[2], emptytextmsg, nodexceedmsg, condlbl;
extern Boolean_t cond_add_canceled;

static Widget bulkform, bulklbl[10], bulktxt[4], bulkmenu[5], bulkdummy[5];
static Condition_t *glob_cond;
static char glob_condstr[1024];
static int grpsel = 2;
static Boolean_t *glob_nu, new_cond;

void Post_Cond_Put (Condition_t *);

void Bulk_Display (char *, int *, int *);
void BulkOpt_CB (Widget, XtPointer, XtPointer);
void CondExit_CB (Widget, XtPointer, XtPointer);
void BulkExit_CB (Widget, XtPointer, XtPointer);
void BulkNode_CB (Widget, XtPointer, XtPointer);
void PTSetCondname (int, char *);

void PTBulk (Condition_t *cond, Boolean_t *node_used, int condnum, Boolean_t new)
{
  char *condstr, tmpstr[128];
  String_t string;
  XmString label[19], title;
  int i, sel, node[2], node2[2];
/*
  XmFontList flhv18;
  XmFontListEntry helv18;
*/
  XmFontList flhv14;
  XmFontListEntry helv14;

  cond_type = PT_TYPE_BULKY;

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
    strcpy (condstr, "1030100> 0003");
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

  bulkform = XmCreateForm (condform, "bulkform", NULL, 0);
  XtVaSetValues (bulkform,
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

  title = XmStringCreateLocalized ("The bulk of the substituent defined by");
  bulklbl[0] = XtVaCreateWidget ("bulklbl0",
               xmLabelWidgetClass, bulkform,
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

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("NODE");
  label[1] = XmStringCreateLocalized ("THE EDGE BETWEEN NODES");

/*
  bulkdummy[0] = XmVaCreateSimplePulldownMenu (bulkform, "_bulkpulldown0", 0, NULL, XmNbuttonFontList, flhv18, NULL);
*/
  bulkdummy[0] = XmVaCreateSimplePulldownMenu (bulkform, "_bulkpulldown0", 0, NULL, XmNbuttonFontList, flhv14, NULL);

  strncpy (tmpstr, glob_condstr + 3, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", node);
  strncpy (tmpstr, glob_condstr + 5, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", node2);
  if (node2[0] == 0) sel = 0;
  else sel = 1;

  bulkmenu[0] = XmVaCreateSimpleOptionMenu (bulkform, "bulkmenu0", title, '\0', sel, BulkOpt_CB,
      XmNsubMenuId, bulkdummy[0],
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

  XtVaSetValues (bulkmenu[0],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, bulklbl[0],
    XmNleftPosition, 12,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);

  bulktxt[0] = XmCreateTextField (bulkform, "bulktxt0", NULL, 0);

  XtVaSetValues (bulktxt[0],
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
    XmNtopWidget, bulklbl[0],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, bulkmenu[0],
    XmNrightPosition, 100,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("AND");
  bulklbl[1] = XtVaCreateWidget ("bulklbl1",
               xmLabelWidgetClass, bulkform,
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
               XmNtopWidget, bulklbl[0],
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, bulktxt[0],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_WIDGET,
               XmNleftWidget, bulktxt[0],
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_NONE,
               NULL);
  XmStringFree (title);

  bulktxt[1] = XmCreateTextField (bulkform, "bulktxt1", NULL, 0);

  XtVaSetValues (bulktxt[1],
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
    XmNtopWidget, bulklbl[0],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, bulklbl[1],
    XmNrightPosition, 150,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("in the goal pattern");
  label[0] = XmStringCreateLocalized ("IS");
  label[1] = XmStringCreateLocalized ("IS NOT");

/*
  bulkdummy[1] = XmVaCreateSimplePulldownMenu (bulkform, "_bulkpulldown1", 0, NULL, XmNbuttonFontList, flhv18, NULL);
*/
  bulkdummy[1] = XmVaCreateSimplePulldownMenu (bulkform, "_bulkpulldown1", 0, NULL, XmNbuttonFontList, flhv14, NULL);

  if (glob_condstr[0] == '0') sel = 1;
  else sel = 0;

  bulkmenu[1] = XmVaCreateSimpleOptionMenu (bulkform, "bulkmenu1", title, '\0', sel, BulkOpt_CB,
      XmNsubMenuId, bulkdummy[1],
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

  XtVaSetValues (bulkmenu[1],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, bulkmenu[0],
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("GREATER THAN");
  label[1] = XmStringCreateLocalized ("LESS THAN");

/*
  bulkdummy[2] = XmVaCreateSimplePulldownMenu (bulkform, "_bulkpulldown2", 0, NULL, XmNbuttonFontList, flhv18, NULL);
*/
  bulkdummy[2] = XmVaCreateSimplePulldownMenu (bulkform, "_bulkpulldown2", 0, NULL, XmNbuttonFontList, flhv14, NULL);

  strncpy (tmpstr, glob_condstr + 7, 2);
  tmpstr[2] = '\0';
  if (strcmp (tmpstr, "> ") == 0) sel = 0;
  else if (strcmp (tmpstr, "< ") == 0) sel = 1;

  bulkmenu[2] = XmVaCreateSimpleOptionMenu (bulkform, "bulkmenu2", title, '\0', sel, BulkOpt_CB,
      XmNsubMenuId, bulkdummy[2],
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

  XtVaSetValues (bulkmenu[2],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, bulkmenu[0],
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, bulkmenu[1],
    NULL);

  title = XmStringCreateLocalized ("the bulk of the substituent");
  bulklbl[2] = XtVaCreateWidget ("bulklbl2",
               xmLabelWidgetClass, bulkform,
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
               XmNtopWidget, bulkmenu[1],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("GROUP");
  label[1] = XmStringCreateLocalized ("NODE");
  label[2] = XmStringCreateLocalized ("defined by THE EDGE BETWEEN NODES");

/*
  bulkdummy[3] = XmVaCreateSimplePulldownMenu (bulkform, "_bulkpulldown3", 0, NULL, XmNbuttonFontList, flhv18, NULL);
*/
  bulkdummy[3] = XmVaCreateSimplePulldownMenu (bulkform, "_bulkpulldown3", 0, NULL, XmNbuttonFontList, flhv14, NULL);

  strncpy (tmpstr, glob_condstr + 9, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", node + 1);
  strncpy (tmpstr, glob_condstr + 11, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", node2 + 1);
  if (node[1] == 0) sel = 0;
  else if (node2[1] == 0) sel = 1;
  else sel = 2;

  bulkmenu[3] = XmVaCreateSimpleOptionMenu (bulkform, "bulkmenu3", title, '\0', sel, BulkOpt_CB,
      XmNsubMenuId, bulkdummy[3],
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
      NULL);

  XmStringFree (title);
  XmStringFree (label[0]);
  XmStringFree (label[1]);
  XmStringFree (label[2]);

  XtVaSetValues (bulkmenu[3],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, bulklbl[2],
    XmNleftPosition, 12,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("METHYL");
  label[1] = XmStringCreateLocalized ("ETHYL");
  label[2] = XmStringCreateLocalized ("ISOPROPYL");
  label[3] = XmStringCreateLocalized ("t-BUTYL");

/*
  bulkdummy[4] = XmVaCreateSimplePulldownMenu (bulkform, "_bulkpulldown4", 0, NULL, XmNbuttonFontList, flhv18, NULL);
*/
  bulkdummy[4] = XmVaCreateSimplePulldownMenu (bulkform, "_bulkpulldown4", 0, NULL, XmNbuttonFontList, flhv14, NULL);

  if (node[1] == 0) grpsel = node2[1] - 1;
  else grpsel = 2;

  bulkmenu[4] = XmVaCreateSimpleOptionMenu (bulkform, "bulkmenu4", title, '\0', grpsel, BulkOpt_CB,
      XmNsubMenuId, bulkdummy[4],
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
      NULL);

  XmStringFree (title);
  XmStringFree (label[0]);
  XmStringFree (label[1]);
  XmStringFree (label[2]);
  XmStringFree (label[3]);

  XtVaSetValues (bulkmenu[4],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, bulklbl[2],
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, bulkmenu[3],
    NULL);

  bulktxt[2] = XmCreateTextField (bulkform, "bulktxt2", NULL, 0);

  XtVaSetValues (bulktxt[2],
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
    XmNtopWidget, bulklbl[2],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, bulkmenu[3],
    XmNrightPosition, 100,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("AND");
  bulklbl[3] = XtVaCreateWidget ("bulklbl3",
               xmLabelWidgetClass, bulkform,
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
               XmNtopWidget, bulklbl[2],
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, bulktxt[2],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_WIDGET,
               XmNleftWidget, bulktxt[2],
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_NONE,
               NULL);
  XmStringFree (title);

  bulktxt[3] = XmCreateTextField (bulkform, "bulktxt3", NULL, 0);

  XtVaSetValues (bulktxt[3],
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
    XmNtopWidget, bulklbl[2],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, bulklbl[3],
    XmNrightPosition, 150,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  for (i = 0; i < 4; i++)
    XtAddCallback (bulktxt[i], XmNvalueChangedCallback, BulkNode_CB, (XtPointer) i);

  title = XmStringCreateLocalized ("NODE/EDGE Menu:");
  bulklbl[4] = XtVaCreateWidget ("bulklbl4",
               xmLabelWidgetClass, bulkform,
               XmNfontList, flhv14,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopPosition, 135,
               XmNtopAttachment, XmATTACH_POSITION,
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized ("  1) If NODE is selected, the user will specify a variable node whose bulk");
  bulklbl[5] = XtVaCreateWidget ("bulklbl5",
               xmLabelWidgetClass, bulkform,
               XmNfontList, flhv14,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, bulklbl[4],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized ("     is to be assessed for the comparison that defines this condition.");
  bulklbl[6] = XtVaCreateWidget ("bulklbl6",
               xmLabelWidgetClass, bulkform,
               XmNfontList, flhv14,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, bulklbl[5],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized ("  2) If EDGE is selected, two neighboring nodes are specified.  The second");
  bulklbl[7] = XtVaCreateWidget ("bulklbl7",
               xmLabelWidgetClass, bulkform,
               XmNfontList, flhv14,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, bulklbl[6],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized ("     node defines a fragment whose bulk is to be determined, and its bond");
  bulklbl[8] = XtVaCreateWidget ("bulklbl8",
               xmLabelWidgetClass, bulkform,
               XmNfontList, flhv14,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, bulklbl[7],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized ("     to the first is the one whose cleavage defines the end of the fragment.");
  bulklbl[9] = XtVaCreateWidget ("bulklbl9",
               xmLabelWidgetClass, bulkform,
               XmNfontList, flhv14,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, bulklbl[8],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);


  Bulk_Display (glob_condstr, node, node2);
}

void Bulk_Display (char *v, int *node, int *node2)
{
  int i, sel;
  char nodelist[256], tmpstr[5];

  XtManageChild (bulklbl[0]);

  XtManageChild (bulkmenu[0]);

  sprintf (nodelist, "%02d", node[0]);

  XtVaSetValues (bulktxt[0],
    XmNvalue, nodelist,
    NULL);

  XtManageChild (bulktxt[0]);

  XtManageChild (bulklbl[1]);

  if (node2[0] != 0)
    {
    sprintf (nodelist, "%02d", node2[0]);

    XtVaSetValues (bulktxt[1],
      XmNvalue, nodelist,
      NULL);
    }

  XtManageChild (bulktxt[1]);

  XtManageChild (bulkmenu[1]);

  XtManageChild (bulkmenu[2]);

  XtManageChild (bulklbl[2]);

  XtManageChild (bulkmenu[3]);

  XtManageChild (bulkmenu[4]);

  if (node[1] != 0)
    {
    sprintf (nodelist, "%02d", node[1]);

    XtVaSetValues (bulktxt[2],
      XmNvalue, nodelist,
      NULL);
    }

  XtManageChild (bulktxt[2]);

  XtManageChild (bulklbl[3]);

  if (node[1] != 0 && node2[1] != 0)
    {
    sprintf (nodelist, "%02d", node2[1]);

    XtVaSetValues (bulktxt[3],
      XmNvalue, nodelist,
      NULL);
    }

  XtManageChild (bulktxt[3]);

  for (i = 4; i < 10; i++)
    XtManageChild (bulklbl[i]);

  XtManageChild (bulkform);
  XtManageChild (ptformdg);

  if (node2[0] == 0)
    {
    XtUnmanageChild (bulklbl[1]);
    XtUnmanageChild (bulktxt[1]);
    }

  if (node[1] == 0)
    {
    XtUnmanageChild (bulktxt[2]);
    XtUnmanageChild (bulklbl[3]);
    XtUnmanageChild (bulktxt[3]);
    }
  else
    {
    XtUnmanageChild (bulkmenu[4]);
    if (node2[1] == 0)
      {
      XtUnmanageChild (bulklbl[3]);
      XtUnmanageChild (bulktxt[3]);
      }
    }
}

void BulkOpt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *wname, tmpstr[128], *ts, *comma, *space;
  int sel, sel2, nnodes, nnodes2;

  wname = XtName (XtParent (w));
  sel = (int) client_data;

  if (strstr (wname, "0") != NULL)
    {
    XtVaGetValues (bulktxt[0],
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
      strncpy (glob_condstr + 3, ts, 2);
      }
    switch (sel)
      {
    case 0: 
      XtUnmanageChild (bulklbl[1]);
      XtUnmanageChild (bulktxt[1]);
      strncpy (glob_condstr + 5, "00", 2);
      break;
    case 1:
      XtManageChild (bulklbl[1]);
      XtManageChild (bulktxt[1]);

      XtVaGetValues (bulktxt[1],
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
        strncpy (glob_condstr + 5, ts, 2);
        }
      break;
      }
    }
  else if (strstr (wname, "1") != NULL) glob_condstr[0] = sel == 0 ? '1' : '0';
  else if (strstr (wname, "2") != NULL) switch (sel)
    {
  case 0:
    strncpy (glob_condstr + 7, "> ", 2);
    break;
  case 1:
    strncpy (glob_condstr + 7, "< ", 2);
    break;
    }
  else if (strstr (wname, "3") != NULL) switch (sel)
    {
  case 0: 
    sprintf (glob_condstr + 9, "00%02d", grpsel + 1);
    XtUnmanageChild (bulktxt[2]);
    XtUnmanageChild (bulklbl[3]);
    XtUnmanageChild (bulktxt[3]);

    XtManageChild (bulkmenu[4]);
    break;
  case 1:
    XtUnmanageChild (bulkmenu[4]);
    XtUnmanageChild (bulklbl[3]);
    XtUnmanageChild (bulktxt[3]);

    XtManageChild (bulktxt[2]);
    XtVaGetValues (bulktxt[2],
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
      strncpy (glob_condstr + 9, ts, 2);
      }
    break;
  case 2:
    XtUnmanageChild (bulkmenu[4]);

    XtManageChild (bulktxt[2]);

    XtVaGetValues (bulktxt[2],
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
      strncpy (glob_condstr + 9, ts, 2);
      }

    XtManageChild (bulklbl[3]);

    XtManageChild (bulktxt[3]);

    XtVaGetValues (bulktxt[3],
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
      strncpy (glob_condstr + 11, ts, 2);
      }
    break;
    }
  else if (strstr (wname, "4") != NULL)
    {
    grpsel = sel;
    sprintf (glob_condstr + 11, "%02d", grpsel + 1);
    }
}

void BulkNode_CB (Widget w, XtPointer client_data, XtPointer call_data)
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

void BulkExit_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  String_t string;
  char tmpstr[128], tmpstr2[128], savestr[128], *ts, *ts2, *comma, *space, *newstr;
  int i, node, node2;

  if ((Boolean_t) (int) client_data)
    {
/* first get final results of text fields */
    XtVaGetValues (bulktxt[0],
      XmNvalue, &ts,
      NULL);

    if (ts[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }
    sscanf (ts, "%d", &node);

    if (XtIsManaged (bulktxt[1]))
      {
      XtVaGetValues (bulktxt[1],
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
    sprintf (tmpstr, "%02d%02d", node, node2);

/* incorporate into string */
    strncpy (glob_condstr + 3, tmpstr, 4);

    if (XtIsManaged (bulktxt[2]))
      {
      XtVaGetValues (bulktxt[2],
        XmNvalue, &ts,
        NULL);

      if (ts[0] == '\0')
        {
        XtManageChild (emptytextmsg);
        return;
        }
      sscanf (ts, "%d", &node);

      if (XtIsManaged (bulktxt[3]))
        {
        XtVaGetValues (bulktxt[3],
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
      sprintf (tmpstr, "%02d%02d", node, node2);
      strncpy (glob_condstr + 9, tmpstr, 4);
      }

    newstr = (char *) malloc (strlen (glob_condstr) + 1);
    strcpy (newstr, glob_condstr);
    String_Value_Put (string, newstr);
    String_Length_Put (string, strlen (newstr));
    String_Alloc_Put (string, strlen (newstr) + 1);

    condition_export (glob_cond, &string, glob_nu);
    }
  else if (new_cond) cond_add_canceled = TRUE;

  XtUnmanageChild (ptformdg);
  XtDestroyWidget (bulkform);
}
