/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              PTFGEQ.C
*
*    This module provides for the display and editing of the posttransform
*    conditions for determining whether all occurrences of a given functional
*    group are equivalent from either a constitutional or a stereochemical
*    perspective.
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

static Widget fgeqform, fgeqlbl, fgeqmenu[3], fgeqdummy[3], fgeqfgpb, fgeqfglbl[2];
static Condition_t *glob_cond;
static char glob_condstr[1024];
static Boolean_t *glob_nu, new_cond;
static int menusel[25], fg;

void Post_Cond_Put (Condition_t *);
void PTSetCondname (int, char *);

void getfgeqfg (int);
void FixNodeList (char *, char *);
void FGEq_Display (char *, int);
void FGEqOpt_CB (Widget, XtPointer, XtPointer);
void CondExit_CB (Widget, XtPointer, XtPointer);
void FGEqExit_CB (Widget, XtPointer, XtPointer);
void NodeList_CB (Widget, XtPointer, XtPointer);
void FGEqPB_CB (Widget, XtPointer, XtPointer);
void MtTxtMsg_Dismiss_CB (Widget, XtPointer, XtPointer);
Boolean_t BadNodeList (char *);

void PTFGEq (Condition_t *cond, Boolean_t *node_used, int condnum, Boolean_t new)
{
  char *condstr, tmpstr[128], menuname[25], dummyname[25];
  String_t string;
  XmString label[42], title;
  int i, sel, menuinx, fgcnt, offset, lblinx;
  XmFontList flhv18;
  XmFontListEntry helv18;
  Boolean_t stereo, goal;

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
    strcpy (condstr, "1150001");
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

  fgeqform = XmCreateForm (condform, "fgeqform", NULL, 0);
  XtVaSetValues (fgeqform,
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

  strncpy (tmpstr, glob_condstr + 1, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", &cond_type);
  stereo = cond_type % 2 == 0;
  goal = glob_condstr[3] == '1';
  sscanf (glob_condstr + 4, "%d", &fg);

  title = XmStringCreateLocalized ("Every instance of the functional group ");
  fgeqfglbl[0] = XtVaCreateWidget ("fgeqfglbl0",
               xmLabelWidgetClass, fgeqform,
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

  title = XmStringCreateLocalized (fgname[fg]);
  fgeqfgpb = XtVaCreateWidget ("fgeqfgpb",
               xmPushButtonGadgetClass, fgeqform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_FORM,
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_NONE,
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_WIDGET,
               XmNleftWidget, fgeqfglbl[0],
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  XtAddCallback (fgeqfgpb, XmNactivateCallback, FGEqPB_CB, (XtPointer) &fg);

  title = XmStringCreateLocalized ("Every instance of the functional group ");
  fgeqfglbl[1] = XtVaCreateWidget ("fgeqfglbl1",
               xmLabelWidgetClass, fgeqform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNtopWidget, fgeqfgpb,
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, fgeqfgpb,
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_WIDGET,
               XmNrightWidget, fgeqfgpb,
               NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("IN THE SUBGOAL PATTERN");
  label[1] = XmStringCreateLocalized ("IN THE GOAL PATTERN");

  fgeqdummy[0] = XmVaCreateSimplePulldownMenu (fgeqform, "_fgeqpulldown0", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  sel = goal ? 1 : 0;

  fgeqmenu[0] = XmVaCreateSimpleOptionMenu (fgeqform, "fgeqmenu0", title, '\0', sel, FGEqOpt_CB,
      XmNsubMenuId, fgeqdummy[0],
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

  XtVaSetValues (fgeqmenu[0],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, fgeqfglbl[1],
    XmNleftPosition, 25,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("IS");
  label[1] = XmStringCreateLocalized ("IS NOT");

  fgeqdummy[1] = XmVaCreateSimplePulldownMenu (fgeqform, "_fgeqpulldown1", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  if (glob_condstr[0] == '0') sel = 1;
  else sel = 0;

  fgeqmenu[1] = XmVaCreateSimpleOptionMenu (fgeqform, "fgeqmenu1", title, '\0', sel, FGEqOpt_CB,
      XmNsubMenuId, fgeqdummy[1],
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

  XtVaSetValues (fgeqmenu[1],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, fgeqmenu[0],
    XmNleftPosition, 25,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("CONSTITUTIONALLY EQUIVALENT");
  label[1] = XmStringCreateLocalized ("STEREOCHEMICALLY EQUIVALENT");

  fgeqdummy[2] = XmVaCreateSimplePulldownMenu (fgeqform, "_fgeqpulldown2", 0, NULL, XmNbuttonFontList, flhv18, NULL);

  sel = stereo ? 1 : 0;

  fgeqmenu[2] = XmVaCreateSimpleOptionMenu (fgeqform, "fgeqmenu2", title, '\0', sel, FGEqOpt_CB,
      XmNsubMenuId, fgeqdummy[2],
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

  XtVaSetValues (fgeqmenu[2],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, fgeqmenu[0],
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, fgeqmenu[1],
    NULL);

  title = XmStringCreateLocalized ("to every other instance.");
  fgeqlbl = XtVaCreateWidget ("fgeqlbl0",
               xmLabelWidgetClass, fgeqform,
               XmNfontList, flhv18,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, fgeqmenu[2],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  FGEq_Display (glob_condstr, fg);
}

void FGEq_Display (char *v, int fg)
{
  int i, sel;
  char nodelist[256], tmpstr[5];

  XtManageChild (fgeqfglbl[0]);
  XtManageChild (fgeqfgpb);
  XtManageChild (fgeqfglbl[1]);
  for (i=0; i<3; i++)
    XtManageChild (fgeqmenu[i]);

  XtManageChild (fgeqlbl);

  XtManageChild (fgeqform);
  XtManageChild (ptformdg);

  XtUnmanageChild (fgeqfglbl[0]);
}

void FGEqOpt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *wname, tmpstr[128], *ts, *comma, *space;
  int sel, type, which, fg, menupos;

  wname = XtName (XtParent (w));
  sel = (int) client_data;
  strncpy (tmpstr, glob_condstr + 1, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", &type);

  if (strstr (wname, "0") != NULL) glob_condstr[3] = sel + '0';
  else if (strstr (wname, "1") != NULL) glob_condstr[0] = sel == 0 ? '1' : '0';
  else if (strstr (wname, "2") != NULL)
    {
    if (sel == 0 && !(type & 1)) type--;
    else if (sel == 1 && (type & 1)) type++;
    sprintf (tmpstr, "%02d", type);
    strncpy (glob_condstr + 1, tmpstr, 2);
    }
}

void FGEqExit_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  String_t string;
  char tmpstr[128], tmpstr2[128], savestr[128], *ts, *ts2, *comma, *space, *newstr;
  int i, fg;
  Boolean_t goal;

  if ((Boolean_t) (int) client_data)
    {
    sscanf (glob_condstr + 4, "%d", &fg);
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
  XtDestroyWidget (fgeqform);
  FG_Form_Close();
}

void FGEqPB_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  FG_Form_Create (XtParent (XtParent (XtParent (XtParent (w)))), *(int *) client_data, getfgeqfg);
}

void getfgeqfg (int fgnum)
{
  XmString title;
  char tmpstr[5];

  fg = fgnum;

  title = XmStringCreateLocalized (fgname[fg]);
  XtVaSetValues (fgeqfgpb,
    XmNlabelString, title,
    NULL);
  XmStringFree (title);
  sprintf (tmpstr, "%03d", fg);
  strncpy (glob_condstr + 4, tmpstr, 3);
}
