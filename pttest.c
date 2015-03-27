/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              PTTEST.C
*
*    This module provides for the display and editing of posttransform tests.
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

#ifndef _H_LOGIN_
#include "login.h"
#endif

#ifndef _CYGHAT_
#ifdef _CYGWIN_
#define _CYGHAT_
#else
#ifdef _REDHAT_
#define _CYGHAT_
#endif
#endif
#endif

extern int cond_type;
extern Widget pttl, ptformdg, condform, condpb[2], emptytextmsg, nodexceedmsg, badteststrmsg, condlbl;
extern Boolean_t test_add_canceled;

static Widget ptestform, ptestlbl[9], ptesttxt[5], ptestdummy, ptestmenu;
#ifdef _CYGHAT_
static Widget box;
#endif
static Posttest_t *glob_test;
static char glob_teststr[1024], new_chem[32];
static String_t *glob_reas, *glob_chem;
static Boolean_t new_test, glob_res[2];
static int glob_eyc[3], glob_tnum;

void Post_Cond_Put (Condition_t *);
Boolean_t ptest_store (Posttest_t *, char *, Boolean_t *, int *, String_t *, String_t *, int);
char *UserId ();

void PTest_Display (int);
void PTestOpt_CB (Widget, XtPointer, XtPointer);
void CondExit_CB (Widget, XtPointer, XtPointer);
void PTestExit_CB (Widget, XtPointer, XtPointer);
void PTestTxt_CB (Widget, XtPointer, XtPointer);
void MtTxtMsg_Dismiss_CB (Widget, XtPointer, XtPointer);
Boolean_t BadSigNum (char *);
void PTSetCondname (int, char *);
void PTWrap_Test (char *);

void PTest (Posttest_t *test, char *teststr, Boolean_t *result, int *eyc, String_t *reas, String_t *chem, int testnum,
  Boolean_t new_test_in)
{
  char tmpstr[128];
  XmString label[19], title;
  int i, sel, nnodes, nnodes2;
  XmFontList flhv18;
  XmFontListEntry helv18;
#ifdef _CYGHAT_
Arg al[50];
int ac;
#endif

  cond_type = PT_TEST_ADD;

  new_test = new_test_in;

  glob_test = test;
  strcpy (glob_teststr, teststr);
  PTWrap_Test (glob_teststr);
  glob_res[0] = result[0];
  glob_res[1] = result[1];
  glob_eyc[0] = eyc[0];
  glob_eyc[1] = eyc[1];
  glob_eyc[2] = eyc[2];
  glob_reas = reas;
  glob_chem = chem;
  glob_tnum = testnum;

  strcpy (new_chem, UserId ());

  helv18 = XmFontListEntryLoad (XtDisplay (pttl), "-*-helvetica-bold-r-normal-*-18-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);

  flhv18 = XmFontListAppendEntry (NULL, helv18);

  sprintf (tmpstr, "TEST%d", testnum + 1);

  title = XmStringCreateLocalized (tmpstr);

  XtVaSetValues (condlbl,
    XmNlabelString, title,
    NULL);

  XmStringFree (title);

  ptestform = XmCreateForm (condform, "ptestform", NULL, 0);
  XtVaSetValues (ptestform,
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

  title = XmStringCreateLocalized ("If ");
  ptestlbl[0] = XtVaCreateWidget ("ptestlbl0",
                xmLabelWidgetClass, ptestform,
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

#ifdef _CYGHAT_
  box = XtVaCreateManagedWidget ("box",
    xmRowColumnWidgetClass, ptestform,
    XmNleftAttachment,      XmATTACH_WIDGET,
    XmNleftWidget,          ptestlbl[0],
    XmNrightAttachment,     XmATTACH_FORM,
    XmNbottomAttachment,    XmATTACH_POSITION,
    XmNbottomPosition,      75,
    XmNorientation,         XmHORIZONTAL,
    XmNheight,              1,
    NULL);

ac=0;
XtSetArg(al[ac],
    XmNfontList, flhv18); ac++;
XtSetArg(al[ac],
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR, SAR_CLRI_WHITE)); ac++;
XtSetArg(al[ac],
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR, SAR_CLRI_BLACK)); ac++;
XtSetArg(al[ac],
    XmNeditMode, XmMULTI_LINE_EDIT); ac++;
XtSetArg(al[ac],
    XmNscrollVertical, True); ac++;

  ptesttxt[0] = XmCreateScrolledText (ptestform, "ptesttxt0", al, ac);
#else

  ptesttxt[0] = XmCreateScrolledText (ptestform, "ptesttxt0", NULL, 0);

  XtVaSetValues (ptesttxt[0],
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNeditMode, XmMULTI_LINE_EDIT,
    XmNscrollVertical, True,
    NULL);
#endif

  XtVaSetValues (XtParent (ptesttxt[0]),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
#ifdef _CYGHAT_
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget,     box,
#else
    XmNbottomPosition, 75,
    XmNbottomAttachment, XmATTACH_POSITION,
#endif
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, ptestlbl[0],
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  title = XmStringCreateLocalized ("If ");

  ptestlbl[1] = XtVaCreateWidget ("ptestlbl1",
                xmLabelWidgetClass, ptestform,
                XmNfontList, flhv18,
                XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
                SAR_CLRI_WHITE),
                XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
                SAR_CLRI_BLACK),
                XmNlabelString, title,
                XmNtopOffset, 0,
                XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
                XmNtopWidget, XtParent(ptesttxt[0]),
                XmNbottomOffset, 0,
                XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
                XmNbottomWidget, XtParent(ptesttxt[0]),
                XmNleftOffset, 0,
                XmNleftAttachment, XmATTACH_FORM,
                XmNrightOffset, 0,
                XmNrightAttachment, XmATTACH_WIDGET,
                XmNrightWidget, XtParent(ptesttxt[0]),
                NULL);

  XmStringFree (title);

  title = XmStringCreateLocalized ("then ");
  label[0] = XmStringCreateLocalized ("FAIL");
  label[1] = XmStringCreateLocalized ("PASS");
  label[2] = XmStringCreateLocalized ("PASS AND STOP");

  ptestdummy = XmVaCreateSimplePulldownMenu (ptestform, "_ptestpulldown0", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  if (!glob_res[0]) sel = 0;
  else if (!glob_res[1]) sel = 1;
  else sel = 2;

  ptestmenu = XmVaCreateSimpleOptionMenu (ptestform, "ptestmenu", title, '\0', sel, PTestOpt_CB,
      XmNsubMenuId, ptestdummy,
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
  XmStringFree (label[0]);
  XmStringFree (label[1]);
  XmStringFree (label[2]);

  XtVaSetValues (ptestmenu,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, ptestlbl[1],
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
#ifdef _CYGWIN_
    XmNrightPosition, 400,
    XmNrightAttachment, XmATTACH_POSITION,
#endif
    NULL);

  title = XmStringCreateLocalized ("adjusting ease ");
  ptestlbl[2] = XtVaCreateWidget ("ptestlbl2",
                xmLabelWidgetClass, ptestform,
                XmNfontList, flhv18,
                XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
                SAR_CLRI_WHITE),
                XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
                SAR_CLRI_BLACK),
                XmNlabelString, title,
                XmNtopOffset, 0,
                XmNtopAttachment, XmATTACH_WIDGET,
                XmNtopWidget, ptestmenu,
                XmNleftPosition, 25,
                XmNleftAttachment, XmATTACH_POSITION,
                NULL);
  XmStringFree (title);

  ptesttxt[1] = XmCreateTextField (ptestform, "ptesttxt1", NULL, 0);

  XtVaSetValues (ptesttxt[1],
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, ptestmenu,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, ptestlbl[2],
    XmNrightPosition, 75,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("adjusting ease ");
  ptestlbl[3] = XtVaCreateWidget ("ptestlbl3",
                xmLabelWidgetClass, ptestform,
                XmNfontList, flhv18,
                XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
                SAR_CLRI_WHITE),
                XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
                SAR_CLRI_BLACK),
                XmNlabelString, title,
                XmNtopOffset, 0,
                XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
                XmNtopWidget, ptesttxt[1],
                XmNbottomOffset, 0,
                XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
                XmNbottomWidget, ptesttxt[1],
                XmNleftPosition, 25,
                XmNleftAttachment, XmATTACH_POSITION,
                XmNrightOffset, 0,
                XmNrightAttachment, XmATTACH_WIDGET,
                XmNrightWidget, ptesttxt[1],
                NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized (", yield ");
  ptestlbl[4] = XtVaCreateWidget ("ptestlbl4",
                xmLabelWidgetClass, ptestform,
                XmNfontList, flhv18,
                XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
                SAR_CLRI_WHITE),
                XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
                SAR_CLRI_BLACK),
                XmNlabelString, title,
                XmNtopOffset, 0,
                XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
                XmNtopWidget, ptesttxt[1],
                XmNbottomOffset, 0,
                XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
                XmNbottomWidget, ptesttxt[1],
                XmNleftOffset, 0,
                XmNleftAttachment, XmATTACH_WIDGET,
                XmNleftWidget, ptesttxt[1],
                NULL);
  XmStringFree (title);

  ptesttxt[2] = XmCreateTextField (ptestform, "ptesttxt2", NULL, 0);

  XtVaSetValues (ptesttxt[2],
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget, ptestlbl[4],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget, ptestlbl[4],
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, ptestlbl[4],
    XmNrightPosition, 125,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized (", and confidence ");
  ptestlbl[5] = XtVaCreateWidget ("ptestlbl5",
                xmLabelWidgetClass, ptestform,
                XmNfontList, flhv18,
                XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
                SAR_CLRI_WHITE),
                XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
                SAR_CLRI_BLACK),
                XmNlabelString, title,
                XmNtopOffset, 0,
                XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
                XmNtopWidget, ptesttxt[2],
                XmNbottomOffset, 0,
                XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
                XmNbottomWidget, ptesttxt[2],
                XmNleftOffset, 0,
                XmNleftAttachment, XmATTACH_WIDGET,
                XmNleftWidget, ptesttxt[2],
                NULL);
  XmStringFree (title);

  ptesttxt[3] = XmCreateTextField (ptestform, "ptesttxt3", NULL, 0);

  XtVaSetValues (ptesttxt[3],
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget, ptestlbl[5],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget, ptestlbl[5],
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, ptestlbl[5],
    XmNrightPosition, 175,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("because ");
  ptestlbl[6] = XtVaCreateWidget ("ptestlbl6",
                xmLabelWidgetClass, ptestform,
                XmNfontList, flhv18,
                XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
                SAR_CLRI_WHITE),
                XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
                SAR_CLRI_BLACK),
                XmNlabelString, title,
                XmNtopOffset, 0,
                XmNtopAttachment, XmATTACH_WIDGET,
                XmNtopWidget, ptestlbl[3],
                XmNleftPosition, 25,
                XmNleftAttachment, XmATTACH_POSITION,
                NULL);
  XmStringFree (title);

  ptesttxt[4] = XmCreateTextField (ptestform, "ptesttxt4", NULL, 0);

  XtVaSetValues (ptesttxt[4],
    XmNfontList, flhv18,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, ptestlbl[3],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, ptestlbl[6],
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  title = XmStringCreateLocalized ("because ");
  ptestlbl[7] = XtVaCreateWidget ("ptestlbl7",
                xmLabelWidgetClass, ptestform,
                XmNfontList, flhv18,
                XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
                SAR_CLRI_WHITE),
                XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
                SAR_CLRI_BLACK),
                XmNlabelString, title,
                XmNtopOffset, 0,
                XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
                XmNtopWidget, ptesttxt[4],
                XmNbottomOffset, 0,
                XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
                XmNbottomWidget, ptesttxt[4],
                XmNleftPosition, 25,
                XmNleftAttachment, XmATTACH_POSITION,
                XmNrightOffset, 0,
                XmNrightAttachment, XmATTACH_WIDGET,
                XmNrightWidget, ptesttxt[4],
                NULL);
  XmStringFree (title);
  if (new_test)
    sprintf (tmpstr, "Chemist: %s", String_Value_Get (*glob_chem));
  else
    sprintf (tmpstr, "Chemist: %s (replaces %s)", new_chem, String_Value_Get (*glob_chem));

  title = XmStringCreateLocalized (tmpstr);
  ptestlbl[8] = XtVaCreateWidget ("ptestlbl8",
                xmLabelWidgetClass, ptestform,
                XmNfontList, flhv18,
                XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
                SAR_CLRI_WHITE),
                XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
                SAR_CLRI_BLACK),
                XmNlabelString, title,
                XmNtopOffset, 0,
                XmNtopAttachment, XmATTACH_WIDGET,
                XmNtopWidget, ptestlbl[7],
                XmNleftPosition, 25,
                XmNleftAttachment, XmATTACH_POSITION,
                NULL);
  XmStringFree (title);

  for (i = 0; i < 5; i++)
    XtAddCallback (ptesttxt[i], XmNvalueChangedCallback, PTestTxt_CB, (XtPointer) i);

  PTest_Display (sel);
}

void PTest_Display (int sel)
{
  int i;
  char eyc[5];

  XtManageChild (ptestlbl[0]);

  XtVaSetValues (ptesttxt[0],
    XmNvalue, glob_teststr,
    NULL);

  XtManageChild (ptesttxt[0]);

  XtManageChild (ptestlbl[1]);

  XtManageChild (ptestmenu);

  sprintf (eyc, "%+d", glob_eyc[0]);
  XtManageChild (ptestlbl[2]);
  XtVaSetValues (ptesttxt[1],
    XmNvalue, eyc,
    NULL);
  XtManageChild (ptesttxt[1]);
  XtManageChild (ptestlbl[3]);

  sprintf (eyc, "%+d", glob_eyc[1]);
  XtManageChild (ptestlbl[4]);
  XtVaSetValues (ptesttxt[2],
    XmNvalue, eyc,
    NULL);
  XtManageChild (ptesttxt[2]);

  sprintf (eyc, "%+d", glob_eyc[2]);
  XtManageChild (ptestlbl[5]);
  XtVaSetValues (ptesttxt[3],
    XmNvalue, eyc,
    NULL);
  XtManageChild (ptesttxt[3]);

  XtManageChild (ptestlbl[6]);
  XtVaSetValues (ptesttxt[4],
    XmNvalue, String_Value_Get (*glob_reas),
    NULL);
  XtManageChild (ptesttxt[4]);
  XtManageChild (ptestlbl[7]);

  XtManageChild (ptestlbl[8]);

  XtManageChild (ptestform);
  XtManageChild (ptformdg);

/*
  XtUnmanageChild (ptestlbl[0]);
*/
  XtUnmanageChild (ptestlbl[1]);
  XtUnmanageChild (ptestlbl[2]);
  XtUnmanageChild (ptestlbl[6]);

  if (sel == 0)
    {
    XtUnmanageChild (ptesttxt[1]);
    XtUnmanageChild (ptestlbl[3]);
    XtUnmanageChild (ptestlbl[4]);
    XtUnmanageChild (ptesttxt[2]);
    XtUnmanageChild (ptestlbl[5]);
    XtUnmanageChild (ptesttxt[3]);
    }
}

void PTestOpt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  int sel;

  sel = (int) client_data;

  glob_res[0] = sel != 0;
  glob_res[1] = sel == 2;

  if (sel == 0)
    {
    XtUnmanageChild (ptesttxt[1]);
    XtUnmanageChild (ptestlbl[3]);
    XtUnmanageChild (ptestlbl[4]);
    XtUnmanageChild (ptesttxt[2]);
    XtUnmanageChild (ptestlbl[5]);
    XtUnmanageChild (ptesttxt[3]);
    }
  else
    {
    XtManageChild (ptesttxt[1]);
    XtManageChild (ptestlbl[3]);
    XtManageChild (ptestlbl[4]);
    XtManageChild (ptesttxt[2]);
    XtManageChild (ptestlbl[5]);
    XtManageChild (ptesttxt[3]);
    }
}

void PTestExit_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *ts, *reas_str, *chem_str;
  int i, reas_len, chem_len;

  if ((Boolean_t) (int) client_data)
    {
/* get final results of text fields */
    XtVaGetValues (ptesttxt[0],
      XmNvalue, &ts,
      NULL);

    if (ts[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }

    strcpy (glob_teststr, ts);

    if (glob_res[0])
      {
      XtVaGetValues (ptesttxt[1],
        XmNvalue, &ts,
        NULL);

      if (ts[0] == '\0')
        {
        XtManageChild (emptytextmsg);
        return;
        }

      sscanf (ts, "%d", glob_eyc);

      XtVaGetValues (ptesttxt[2],
        XmNvalue, &ts,
        NULL);

      if (ts[0] == '\0')
        {
        XtManageChild (emptytextmsg);
        return;
        }

      sscanf (ts, "%d", glob_eyc + 1);

      XtVaGetValues (ptesttxt[3],
        XmNvalue, &ts,
        NULL);

      if (ts[0] == '\0')
        {
        XtManageChild (emptytextmsg);
        return;
        }

      sscanf (ts, "%d", glob_eyc + 2);
      }
    else glob_eyc[0] = glob_eyc[1] = glob_eyc[2] = 0;

    XtVaGetValues (ptesttxt[4],
      XmNvalue, &ts,
      NULL);

    if (ts[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }

    String_Destroy (*glob_reas);
    reas_len = strlen (ts);
    reas_str = (char *) malloc (reas_len + 1);
    strcpy (reas_str, ts);
    String_Value_Put (*glob_reas, reas_str);
    String_Length_Put (*glob_reas, reas_len);
    String_Alloc_Put (*glob_reas, reas_len + 1);

    String_Destroy (*glob_chem);
    chem_len = strlen (new_chem);
    chem_str = (char *) malloc (chem_len + 1);
    strcpy (chem_str, new_chem);
    String_Value_Put (*glob_chem, chem_str);
    String_Length_Put (*glob_chem, chem_len);
    String_Alloc_Put (*glob_chem, chem_len + 1);

    if (!ptest_store (glob_test, glob_teststr, glob_res, glob_eyc, glob_reas, glob_chem, glob_tnum))
      {
      XtManageChild (badteststrmsg);
      return;
      }
    }
  else if (new_test) test_add_canceled = TRUE;

  XtUnmanageChild (ptformdg);
  XtDestroyWidget (ptestform);
}

void PTestTxt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *lcl_node_str, node_str[128], *ts, teststr[1024];
  int len, which, curpos;
  static Boolean_t internal_change = FALSE;

  if (internal_change) return; /* ignore callback due to username clearing */

  which = (int) client_data;

  if (which == 4) return;

  if (which == 0)
    {
    internal_change = TRUE;
    XtVaGetValues (ptesttxt[0],
      XmNvalue, &ts,
      XmNcursorPosition, &curpos,
      NULL);
    strcpy (teststr, ts);

    PTWrap_Test (teststr);

    XtVaSetValues (ptesttxt[0],
      XmNvalue, teststr,
      XmNcursorPosition, curpos,
      NULL);
    internal_change = FALSE;
    return;
    }

  lcl_node_str = XmTextGetString(w);

  if (BadSigNum (lcl_node_str))
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

Boolean_t BadSigNum (char *str)
{
  char chr;
  int len, i;

/* empty string OK here */
  if (str[0] == '\0') return (FALSE);
/* must be 4 chars or less */
  if (strlen (str) > 4) return (TRUE);
/* must start with digit or sign */
  if ((str[0] < '0' || str[0] > '9') && str[0] != '+' && str[0] != '-') return (TRUE);
  return (FALSE);
}

void PTWrap_Test (char *teststr)
{
  int wrap_pos, i;

  for (i=0, wrap_pos=-1; i<strlen(teststr); i++)
    {
    if (teststr[i] == '\n') teststr[i] = ' ';
    if (teststr[i] == ' ' && i - wrap_pos > 95) teststr[wrap_pos = i] = '\n';
    }
}
