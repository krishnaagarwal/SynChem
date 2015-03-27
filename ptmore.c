/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              PTMORE.C
*
*    This module provides for the display and editing of the posttransform
*    conditions for determining whether a driving force exists, by virtue of
*    the gain or loss of a given functionality during the reaction.
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
#include <Xm/PushBG.h>
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

#ifndef _H_EXTERN_
#include "extern.h"
#endif

extern int cond_type;
extern Widget pttl, ptformdg, condform, condpb[2], emptytextmsg, nodexceedmsg, invalidatommsg, invalidfgmsg, condlbl;
extern Boolean_t cond_add_canceled;

static Widget moreform, morelbl, moremenu[2], moredummy[2], morefgpb, morefglbl[2];
static Condition_t *glob_cond;
static char glob_condstr[1024];
static Boolean_t *glob_nu, new_cond;
static int menusel[25], fg;

void Post_Cond_Put (Condition_t *);
void PTSetCondname (int, char *);

void getmorefg (int);
void FixNodeList (char *, char *);
void More_Display (char *, int);
void MoreOpt_CB (Widget, XtPointer, XtPointer);
void CondExit_CB (Widget, XtPointer, XtPointer);
void MoreExit_CB (Widget, XtPointer, XtPointer);
void NodeList_CB (Widget, XtPointer, XtPointer);
void MorePB_CB (Widget, XtPointer, XtPointer);
void MtTxtMsg_Dismiss_CB (Widget, XtPointer, XtPointer);
Boolean_t BadNodeList (char *);

void PTMore (Condition_t *cond, Boolean_t *node_used, int condnum, Boolean_t new)
{
  char *condstr, tmpstr[128], menuname[25], dummyname[25];
  String_t string;
  XmString label[42], title;
  int i, sel, menuinx, fgcnt, offset, lblinx;
  XmFontList flhv18;
  XmFontListEntry helv18;
  Boolean_t more;

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
    strcpy (condstr, "119001>");
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

  cond_type = PT_TYPE_FG_CNT;

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

  moreform = XmCreateForm (condform, "moreform", NULL, 0);
  XtVaSetValues (moreform,
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

  more = glob_condstr[6] == '>';
  sscanf (glob_condstr + 3, "%d", &fg);

  title = XmStringCreateLocalized ("There ");
  label[0] = XmStringCreateLocalized ("ARE");
  label[1] = XmStringCreateLocalized ("ARE NOT");

  moredummy[0] = XmVaCreateSimplePulldownMenu (moreform, "_morepulldown0", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  if (glob_condstr[0] == '0') sel = 1;
  else sel = 0;

  moremenu[0] = XmVaCreateSimpleOptionMenu (moreform, "moremenu0", title, '\0', sel, MoreOpt_CB,
      XmNsubMenuId, moredummy[0],
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

  XtVaSetValues (moremenu[0],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("MORE");
  label[1] = XmStringCreateLocalized ("FEWER");

  moredummy[1] = XmVaCreateSimplePulldownMenu (moreform, "_morepulldown1", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  sel = more ? 0 : 1;

  moremenu[1] = XmVaCreateSimpleOptionMenu (moreform, "moremenu1", title, '\0', sel, MoreOpt_CB,
      XmNsubMenuId, moredummy[1],
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

  XtVaSetValues (moremenu[1],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, moremenu[0],
    NULL);

  title = XmStringCreateLocalized ("instances of the functional group ");
  morefglbl[0] = XtVaCreateWidget ("morefglbl0",
               xmLabelWidgetClass, moreform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, moremenu[0],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized (fgname[fg]);
  morefgpb = XtVaCreateWidget ("morefgpb",
               xmPushButtonGadgetClass, moreform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNtopWidget, morefglbl[0],
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_NONE,
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_WIDGET,
               XmNleftWidget, morefglbl[0],
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  XtAddCallback (morefgpb, XmNactivateCallback, MorePB_CB, (XtPointer) &fg);

  title = XmStringCreateLocalized ("instances of the functional group ");
  morefglbl[1] = XtVaCreateWidget ("morefglbl1",
               xmLabelWidgetClass, moreform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNtopWidget, morefgpb,
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, morefgpb,
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_WIDGET,
               XmNrightWidget, morefgpb,
               NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized ("in the goal than there are in the subgoal.");
  morelbl = XtVaCreateWidget ("morelbl0",
               xmLabelWidgetClass, moreform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, morefglbl[1],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  More_Display (glob_condstr, fg);
}

void More_Display (char *v, int fg)
{
  int i, sel;
  char nodelist[256], tmpstr[5];

  XtManageChild (moremenu[0]);
  XtManageChild (moremenu[1]);

  XtManageChild (morefglbl[0]);
  XtManageChild (morefgpb);
  XtManageChild (morefglbl[1]);

  XtManageChild (morelbl);

  XtManageChild (moreform);
  XtManageChild (ptformdg);

  XtUnmanageChild (morefglbl[0]);
}

void MoreOpt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *wname, tmpstr[128], *ts, *comma, *space;
  int sel, type, which, fg, menupos;

  wname = XtName (XtParent (w));
  sel = (int) client_data;
  strncpy (tmpstr, glob_condstr + 1, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", &type);

  if (strstr (wname, "0") != NULL) glob_condstr[0] = sel == 0 ? '1' : '0';
  else if (strstr (wname, "1") != NULL) glob_condstr[6] = sel == 0 ? '>' : '<';
}

void MoreExit_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  String_t string;
  char tmpstr[128], tmpstr2[128], savestr[128], *ts, *ts2, *comma, *space, *newstr;
  int i, fg;
  Boolean_t goal;

  if ((Boolean_t) (int) client_data)
    {
    sscanf (glob_condstr + 3, "%d", &fg);
    if (fg == 0 || *fgname[fg] == '\0')
      {
      XtManageChild (invalidfgmsg);
      return;
      }
/* first get final results of text fields */
/* incorporate into string */

    newstr = (char *) malloc (strlen (glob_condstr) + 1);
    strcpy (newstr, glob_condstr);
    String_Value_Put (string, newstr);
    String_Length_Put (string, strlen (newstr));
    String_Alloc_Put (string, strlen (newstr) + 1);

    condition_export (glob_cond, &string, glob_nu);
    }
  else if (new_cond) cond_add_canceled = TRUE;

  XtUnmanageChild (ptformdg);
  XtDestroyWidget (moreform);
FG_Form_Close();
}

void MorePB_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  FG_Form_Create (XtParent (XtParent (XtParent (XtParent (w)))), *(int *) client_data, getmorefg);
}

void getmorefg (int fgnum)
{
  XmString title;
  char tmpstr[5];

  fg = fgnum;

  title = XmStringCreateLocalized (fgname[fg]);
  XtVaSetValues (morefgpb,
    XmNlabelString, title,
    NULL);
  XmStringFree (title);
  sprintf (tmpstr, "%03d", fg);
  strncpy (glob_condstr + 3, tmpstr, 3);
}
