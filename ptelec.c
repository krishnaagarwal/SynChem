/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              PTELEC.C
*
*    This module provides for the display and editing of the posttransform
*    conditions for determining the electronic effects contributed by the
*    substituents at a group of nodes, either as a constant or in comparison
*    against another group of nodes.
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
#include <Xm/Separator.h>
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

#ifndef _H_SYNHELP_
#include "synhelp.h"
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

int cond_type = PT_TYPE_ELECWD, cond_subtype;
Widget pttl, ptformdg, condform, condpb[2], conddelpb[2], emptytextmsg, nodexceedmsg, invalidatommsg, invalidfgmsg, condlbl,
  condsep, conddellbl, badteststrmsg;
Boolean_t marked_for_del = FALSE;

extern Boolean_t cond_marked[], test_marked[], curr_is_test, cond_add_canceled, test_add_canceled, current_marked,
  postform_modified, glob_rxlform;
extern int curr_num;

static Widget elecform, eleclbl[2], electxt[2], elecmenu[4], elecdummy[4];
static Condition_t *glob_cond;
static char glob_condstr[1024];
static Boolean_t *glob_nu, new_cond;

void Post_Cond_Put (Condition_t *);
void PTSetCondname (int, char *);

void FixNodeList (char *, char *);
void Elec_Display (char *, int, int);
void ElecOpt_CB (Widget, XtPointer, XtPointer);
void CondDel_CB (Widget, XtPointer, XtPointer);
void PTHelp_CB (Widget, XtPointer, XtPointer);
void CondExit_CB (Widget, XtPointer, XtPointer);
void ElecExit_CB (Widget, XtPointer, XtPointer);
void MolecExit_CB (Widget, XtPointer, XtPointer);
void BulkExit_CB (Widget, XtPointer, XtPointer);
void DistExit_CB (Widget, XtPointer, XtPointer);
void NodeList_CB (Widget, XtPointer, XtPointer);
void MtTxtMsg_Dismiss_CB (Widget, XtPointer, XtPointer);
Boolean_t BadNodeList (char *);

void PT_Form_Create (Widget top_level)
{
  XmString title, msg;
/*
  XmFontList flhv18;
  XmFontListEntry helv18;
*/
  XmFontList flhv14;
  XmFontListEntry helv14;
  int i;
  Widget helppb;

/*
  helv18 = XmFontListEntryLoad (XtDisplay (top_level), "-*-helvetica-bold-r-normal-*-18-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);

  flhv18 = XmFontListAppendEntry (NULL, helv18);
*/
  helv14 = XmFontListEntryLoad (XtDisplay (top_level), "-*-helvetica-bold-r-normal-*-14-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);

  flhv14 = XmFontListAppendEntry (NULL, helv14);

  pttl = top_level;

  emptytextmsg = XmCreateMessageDialog (top_level, "Message", NULL, 0);
  title=XmStringCreateLocalized("Dismiss");
  msg=XmStringCreateLocalized("A text field is blank.");
  XtVaSetValues (emptytextmsg,
        XmNmessageString, msg,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNokLabelString, title,
        NULL);
  XmStringFree(title);
  XmStringFree(msg);

  XtUnmanageChild (XmMessageBoxGetChild (emptytextmsg, XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild (XmMessageBoxGetChild (emptytextmsg, XmDIALOG_HELP_BUTTON));
  XtAddCallback (XmMessageBoxGetChild (emptytextmsg, XmDIALOG_OK_BUTTON), XmNactivateCallback, MtTxtMsg_Dismiss_CB,
    (XtPointer) NULL);
  XtUnmanageChild (emptytextmsg);

  nodexceedmsg = XmCreateMessageDialog (top_level, "Message", NULL, 0);
  title=XmStringCreateLocalized("Dismiss");
  msg=XmStringCreateLocalized("Maximum of 7 nodes (total per condition) exceeded.");
  XtVaSetValues (nodexceedmsg,
        XmNmessageString, msg,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNokLabelString, title,
        NULL);
  XmStringFree(title);
  XmStringFree(msg);

  XtUnmanageChild (XmMessageBoxGetChild (nodexceedmsg, XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild (XmMessageBoxGetChild (nodexceedmsg, XmDIALOG_HELP_BUTTON));
  XtAddCallback (XmMessageBoxGetChild (nodexceedmsg, XmDIALOG_OK_BUTTON), XmNactivateCallback, MtTxtMsg_Dismiss_CB,
    (XtPointer) NULL);
  XtUnmanageChild (nodexceedmsg);

  invalidatommsg = XmCreateMessageDialog (top_level, "Message", NULL, 0);
  title=XmStringCreateLocalized("Dismiss");
  msg=XmStringCreateLocalized("Atomic symbol entered is not recognized.");
  XtVaSetValues (invalidatommsg,
        XmNmessageString, msg,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNokLabelString, title,
        NULL);
  XmStringFree(title);
  XmStringFree(msg);

  XtUnmanageChild (XmMessageBoxGetChild (invalidatommsg, XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild (XmMessageBoxGetChild (invalidatommsg, XmDIALOG_HELP_BUTTON));
  XtAddCallback (XmMessageBoxGetChild (invalidatommsg, XmDIALOG_OK_BUTTON), XmNactivateCallback, MtTxtMsg_Dismiss_CB,
    (XtPointer) NULL);
  XtUnmanageChild (invalidatommsg);

  invalidfgmsg = XmCreateMessageDialog (top_level, "Message", NULL, 0);
  title=XmStringCreateLocalized("Dismiss");
  msg=XmStringCreateLocalized("Selected functional group is not valid.");
  XtVaSetValues (invalidfgmsg,
        XmNmessageString, msg,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNokLabelString, title,
        NULL);
  XmStringFree(title);
  XmStringFree(msg);

  XtUnmanageChild (XmMessageBoxGetChild (invalidfgmsg, XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild (XmMessageBoxGetChild (invalidfgmsg, XmDIALOG_HELP_BUTTON));
  XtAddCallback (XmMessageBoxGetChild (invalidfgmsg, XmDIALOG_OK_BUTTON), XmNactivateCallback, MtTxtMsg_Dismiss_CB,
    (XtPointer) NULL);
  XtUnmanageChild (invalidfgmsg);

  badteststrmsg = XmCreateMessageDialog (top_level, "Message", NULL, 0);
  title=XmStringCreateLocalized("Dismiss");
  msg=XmStringCreateLocalized("Test string is badly formed - check syntax and parenthesis balance.");
  XtVaSetValues (badteststrmsg,
        XmNmessageString, msg,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNokLabelString, title,
        NULL);
  XmStringFree(title);
  XmStringFree(msg);

  XtUnmanageChild (XmMessageBoxGetChild (badteststrmsg, XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild (XmMessageBoxGetChild (badteststrmsg, XmDIALOG_HELP_BUTTON));
  XtAddCallback (XmMessageBoxGetChild (badteststrmsg, XmDIALOG_OK_BUTTON), XmNactivateCallback, MtTxtMsg_Dismiss_CB,
    (XtPointer) NULL);
  XtUnmanageChild (badteststrmsg);

  ptformdg = XmCreateFormDialog (top_level, "PTForm", NULL, 0);
  title = XmStringCreateLocalized ("PT Condition Editor");
  XtVaSetValues (ptformdg,
    XmNdialogStyle,     XmDIALOG_MODELESS,
/*
    XmNlabelFontList,   flhv18,
    XmNbuttonFontList,  flhv18,
    XmNtextFontList,    flhv18,
*/
    XmNlabelFontList,   flhv14,
    XmNbuttonFontList,  flhv14,
    XmNtextFontList,    flhv14,
    XmNresizePolicy,    XmRESIZE_NONE,
    XmNresizable,       True,
    XmNautoUnmanage,    False,
    XmNdialogTitle,     title,
    XmNdefaultPosition, False,
    XmNheight,          AppDim_AppHeight_Get (&GAppDim) / 2,
    XmNwidth,           AppDim_AppWidth_Get (&GAppDim),
    XmNfractionBase,    800,
    NULL);
  XmStringFree (title);

  condform = XtVaCreateManagedWidget ("condform",
    xmFormWidgetClass, ptformdg,
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomPosition, 700,
    XmNbottomAttachment, XmATTACH_POSITION,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  condsep = XtVaCreateManagedWidget ("condsep",
               xmSeparatorWidgetClass, ptformdg,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, condform,
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_NONE,
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_FORM,
               NULL);

  title = XmStringCreateLocalized ("Undefined condition");
  condlbl = XtVaCreateManagedWidget ("condlbl",
               xmLabelWidgetClass, ptformdg,
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
               XmNtopWidget, condsep,
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_NONE,
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized ("Help");
  helppb = XtVaCreateManagedWidget ("HelpPB",
    xmPushButtonWidgetClass, ptformdg,
    XmNlabelString, title,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, condlbl,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightPosition, 50,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);
  XmStringFree (title);

  XtAddCallback (helppb, XmNactivateCallback, PTHelp_CB, (XtPointer) NULL);

  title = XmStringCreateLocalized ("Marked for deletion");
  conddellbl = XtVaCreateWidget ("conddellbl",
               xmLabelWidgetClass, ptformdg,
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
               XmNtopWidget, condlbl,
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_FORM,
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_WIDGET,
               XmNleftWidget, helppb,
               XmNrightPosition, 200,
               XmNrightAttachment, XmATTACH_POSITION,
               NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized ("Mark for Deletion");
  conddelpb[0] = XtVaCreateManagedWidget ("CondDelPB",
    xmPushButtonWidgetClass, ptformdg,
    XmNlabelString, title,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, condlbl,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, helppb,
    XmNrightPosition, 300,
    XmNrightAttachment, XmATTACH_POSITION,
    XmNsensitive, False,
    NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized ("Undelete");
  conddelpb[1] = XtVaCreateWidget ("CondUndelPB",
    xmPushButtonWidgetClass, ptformdg,
    XmNlabelString, title,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, condlbl,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, conddellbl,
    XmNrightPosition, 300,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);
  XmStringFree (title);

  for (i=0; i<2; i++)
    XtAddCallback (conddelpb[i], XmNactivateCallback, CondDel_CB, (XtPointer) i);

  title = XmStringCreateLocalized ("Exit and Update Postran Buffer");
  condpb[0] = XtVaCreateManagedWidget ("CondSavePB",
    xmPushButtonWidgetClass, ptformdg,
    XmNlabelString, title,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, condlbl,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, conddelpb[0],
    XmNrightPosition, 550,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized ("Cancel");
  condpb[1] = XtVaCreateManagedWidget ("CondCancelPB",
    xmPushButtonWidgetClass, ptformdg,
    XmNlabelString, title,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, condlbl,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, condpb[0],
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    NULL);
  XmStringFree (title);

  for (i = 0; i < 2; i++)
    XtAddCallback (condpb[i], XmNactivateCallback, CondExit_CB, (XtPointer) (i == 0));
}

void PT_Enable_Deletion (Boolean_t enable)
{
  XtVaSetValues (conddelpb[0],
    XmNsensitive, enable ? True : False,
    NULL);
}

void PT_Enable_Undeletion (Boolean_t enable)
{
  XtVaSetValues (conddelpb[1],
    XmNsensitive, enable ? True : False,
    NULL);
}

void PTElec (Condition_t *cond, Boolean_t *node_used, int condnum, Boolean_t new)
{
  char *condstr, tmpstr[128];
  String_t string;
  XmString label[19], title;
  int i, sel, nnodes, nnodes2;
/*
  XmFontList flhv18;
  XmFontListEntry helv18;
*/
  XmFontList flhv14;
  XmFontListEntry helv14;

  cond_type = PT_TYPE_ELECWD;

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
    strcpy (condstr, "1010101>=00+0");
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

  elecform = XmCreateForm (condform, "elecform", NULL, 0);

  XtVaSetValues (elecform,
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

  title = XmStringCreateLocalized ("The sum of the electron-withdrawing effects of the substituents rooted at node(s)");
  eleclbl[0] = XtVaCreateWidget ("eleclbl0",
               xmLabelWidgetClass, elecform,
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

  electxt[0] = XmCreateTextField (elecform, "electxt0", NULL, 0);

  XtVaSetValues (electxt[0],
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
    XmNtopWidget, eleclbl[0],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftPosition, 12,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  title = XmStringCreateLocalized ("in the goal pattern");
  label[0] = XmStringCreateLocalized ("IS");
  label[1] = XmStringCreateLocalized ("IS NOT");

/*
  elecdummy[0] = XmVaCreateSimplePulldownMenu (elecform, "_elecpulldown0", 0, NULL, XmNbuttonFontList, flhv18, NULL);
*/
  elecdummy[0] = XmVaCreateSimplePulldownMenu (elecform, "_elecpulldown0", 0, NULL, XmNbuttonFontList, flhv14, NULL);

  if (glob_condstr[0] == '0') sel = 1;
  else sel = 0;

  elecmenu[0] = XmVaCreateSimpleOptionMenu (elecform, "elecmenu0", title, '\0', sel, ElecOpt_CB,
      XmNsubMenuId, elecdummy[0],
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

  XtVaSetValues (elecmenu[0],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, electxt[0],
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

/*
  elecdummy[1] = XmVaCreateSimplePulldownMenu (elecform, "_elecpulldown1", 0, NULL, XmNbuttonFontList, flhv18, NULL);
*/
  elecdummy[1] = XmVaCreateSimplePulldownMenu (elecform, "_elecpulldown1", 0, NULL, XmNbuttonFontList, flhv14, NULL);

  strncpy (tmpstr, glob_condstr + 3, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", &nnodes);
  strncpy (tmpstr, glob_condstr + 5 + 2 * nnodes, 2);
  tmpstr[2] = '\0';
  if (strcmp (tmpstr, "> ") == 0) sel = 0;
  else if (strcmp (tmpstr, ">=") == 0) sel = 1;
  else if (strcmp (tmpstr, "= ") == 0) sel = 2;
  else if (strcmp (tmpstr, "~=") == 0) sel = 3;
  else if (strcmp (tmpstr, "< ") == 0) sel = 4;
  else if (strcmp (tmpstr, "<=") == 0) sel = 5;
  else sel=-1;
  elecmenu[1] = XmVaCreateSimpleOptionMenu (elecform, "elecmenu1", title, '\0', sel, ElecOpt_CB,
      XmNsubMenuId, elecdummy[1],
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

  XtVaSetValues (elecmenu[1],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, electxt[0],
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, elecmenu[0],
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("THE CONSTANT");
  label[1] = XmStringCreateLocalized ("THE SUM OF THE ELECTRON-WITHDRAWING EFFECTS OF THE SUBSTITUENTS ROOTED AT NODE(S)");

/*
  elecdummy[2] = XmVaCreateSimplePulldownMenu (elecform, "_elecpulldown2", 0, NULL, XmNbuttonFontList, flhv18, NULL);
*/
  elecdummy[2] = XmVaCreateSimplePulldownMenu (elecform, "_elecpulldown2", 0, NULL, XmNbuttonFontList, flhv14, NULL);

  strncpy (tmpstr, glob_condstr + 7 + 2 * nnodes, 2);
  tmpstr[2] = '\0';
  if (strcmp (tmpstr, "00") == 0) nnodes2 = sel = 0;
  else
    {
    sel = 1;
    sscanf (tmpstr, "%d", &nnodes2);
    }

  elecmenu[2] = XmVaCreateSimpleOptionMenu (elecform, "elecmenu2", title, '\0', sel, ElecOpt_CB,
      XmNsubMenuId, elecdummy[2],
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

  XtVaSetValues (elecmenu[2],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, elecmenu[0],
    XmNleftPosition, 12,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("");
/*
  label[0] = XmStringCreateLocalized ("-9");
  label[1] = XmStringCreateLocalized ("-8");
  label[2] = XmStringCreateLocalized ("-7");
  label[3] = XmStringCreateLocalized ("-6");
  label[4] = XmStringCreateLocalized ("-5");
  label[5] = XmStringCreateLocalized ("-4");
  label[6] = XmStringCreateLocalized ("-3");
*/
  label[7] = XmStringCreateLocalized ("-2");
  label[8] = XmStringCreateLocalized ("-1");
  label[9] = XmStringCreateLocalized (" 0");
  label[10] = XmStringCreateLocalized ("+1");
  label[11] = XmStringCreateLocalized ("+2");
  label[12] = XmStringCreateLocalized ("+3");
  label[13] = XmStringCreateLocalized ("+4");
/*
  label[14] = XmStringCreateLocalized ("+5");
  label[15] = XmStringCreateLocalized ("+6");
  label[16] = XmStringCreateLocalized ("+7");
  label[17] = XmStringCreateLocalized ("+8");
  label[18] = XmStringCreateLocalized ("+9");
*/

/*
  elecdummy[3] = XmVaCreateSimplePulldownMenu (elecform, "_elecpulldown3", 0, NULL, XmNbuttonFontList, flhv18, NULL);
*/
  elecdummy[3] = XmVaCreateSimplePulldownMenu (elecform, "_elecpulldown3", 0, NULL, XmNbuttonFontList, flhv14, NULL);

  if (nnodes2 == 0)
    {
    strncpy (tmpstr, glob_condstr + 9 + 2 * nnodes, 2);
    tmpstr[2] = '\0';
    sscanf (tmpstr, "%d", &sel);
    sel += /* 9 */ 2;
    }
  else sel = /* 9 */ 2;

  elecmenu[3] = XmVaCreateSimpleOptionMenu (elecform, "elecmenu3", title, '\0', sel, ElecOpt_CB,
      XmNsubMenuId, elecdummy[3],
/*
      XmNbuttonFontList, flhv18,
*/
      XmNbuttonFontList, flhv14,
      XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
      SAR_CLRI_WHITE),
      XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
      SAR_CLRI_BLACK),
/*
      XmVaPUSHBUTTON, label[0], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[1], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[2], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[3], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[4], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[5], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[6], '\0', NULL, NULL,
*/
      XmVaPUSHBUTTON, label[7], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[8], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[9], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[10], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[11], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[12], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[13], '\0', NULL, NULL,
/*
      XmVaPUSHBUTTON, label[14], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[15], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[16], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[17], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[18], '\0', NULL, NULL,
*/
      NULL);

  XmStringFree (title);
/*
  XmStringFree (label[0]);
  XmStringFree (label[1]);
  XmStringFree (label[2]);
  XmStringFree (label[3]);
  XmStringFree (label[4]);
  XmStringFree (label[5]);
  XmStringFree (label[6]);
*/
  XmStringFree (label[7]);
  XmStringFree (label[8]);
  XmStringFree (label[9]);
  XmStringFree (label[10]);
  XmStringFree (label[11]);
  XmStringFree (label[12]);
  XmStringFree (label[13]);
/*
  XmStringFree (label[14]);
  XmStringFree (label[15]);
  XmStringFree (label[16]);
  XmStringFree (label[17]);
  XmStringFree (label[18]);
*/

  XtVaSetValues (elecmenu[3],
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, elecmenu[0],
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, elecmenu[2],
    NULL);

  electxt[1] = XmCreateTextField (elecform, "electxt1", NULL, 0);

  XtVaSetValues (electxt[1],
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
    XmNtopWidget, elecmenu[2],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftPosition, 12,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  title = XmStringCreateLocalized ("(where positive numbers indicate electron-withdrawing groups).");
  eleclbl[1] = XtVaCreateWidget ("eleclbl1",
               xmLabelWidgetClass, elecform,
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
               XmNtopWidget, electxt[1],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  for (i = 0; i < 2; i++)
    XtAddCallback (electxt[i], XmNvalueChangedCallback, NodeList_CB, (XtPointer) i);
  Elec_Display (glob_condstr, nnodes, nnodes2);
}

void Elec_Display (char *v, int nnodes, int nnodes2)
{
  int i, sel;
  char nodelist[256], tmpstr[5];

  XtManageChild (eleclbl[0]);

  strncpy (nodelist, v + 5, 2);
  nodelist[2] = '\0';
  for (i = 1; i < nnodes; i++)
    {
    strcat (nodelist, ", ");
    strncat (nodelist, v + 5 + 2 * i, 2);
    nodelist[4 * i + 2] = '\0';
    }
  XtVaSetValues (electxt[0],
    XmNvalue, nodelist,
    NULL);
  XtManageChild (electxt[0]);

  XtManageChild (elecmenu[0]);

  XtManageChild (elecmenu[1]);

  XtManageChild (elecmenu[2]);

  XtManageChild (elecmenu[3]);
  XtManageChild (electxt[1]);
  XtManageChild (eleclbl[1]);

  XtManageChild (elecform);
  XtManageChild (ptformdg);

  if (nnodes2 == 0)
    XtUnmanageChild (electxt[1]);
  else
    {
    strncpy (nodelist, v + 9 + 2 * nnodes, 2);
    nodelist[2] = '\0';
    for (i = 1; i < nnodes2; i++)
      {
      strcat (nodelist, ", ");
      strncat (nodelist, v + 9 + 2 * (nnodes + i), 2);
      nodelist[4 * i + 2] = '\0';
      }
    XtVaSetValues (electxt[1],
      XmNvalue, nodelist,
      NULL);
    XtUnmanageChild (elecmenu[3]);
    }
}

void ElecOpt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *wname, tmpstr[128], *ts, *comma, *space;
  int sel, sel2, nnodes, nnodes2;

  wname = XtName (XtParent (w));
  sel = (int) client_data;
  strncpy (tmpstr, glob_condstr + 3, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", &nnodes);

  if (strstr (wname, "0") != NULL) glob_condstr[0] = sel == 0 ? '1' : '0';
  else if (strstr (wname, "1") != NULL) switch (sel)
    {
  case 0:
    strncpy (glob_condstr + 5 + 2 * nnodes, "> ", 2);
    break;
  case 1:
    strncpy (glob_condstr + 5 + 2 * nnodes, ">=", 2);
    break;
  case 2:
    strncpy (glob_condstr + 5 + 2 * nnodes, "= ", 2);
    break;
  case 3:
    strncpy (glob_condstr + 5 + 2 * nnodes, "~=", 2);
    break;
  case 4:
    strncpy (glob_condstr + 5 + 2 * nnodes, "< ", 2);
    break;
  case 5:
    strncpy (glob_condstr + 5 + 2 * nnodes, "<=", 2);
    break;
    }
  else if (strstr (wname, "2") != NULL) switch (sel)
    {
  case 0: 
    XtUnmanageChild (electxt[1]);
    XtManageChild (elecmenu[3]);
    XtVaGetValues (elecdummy[3],
      XmNpostFromButton, &sel2,
      NULL);
    sprintf (tmpstr, "00%+d", sel2 - /* 9 */ 2);
    strcpy (glob_condstr + 7 + 2 * nnodes, tmpstr);
    break;
  case 1:
    XtUnmanageChild (elecmenu[3]);
    XtManageChild (electxt[1]);
    XtVaGetValues (electxt[1],
      XmNvalue, &ts,
      NULL);
    do
      {
      comma = strstr (ts, ",");
      space = strstr (ts, " ");
      if (comma != NULL && (space == NULL || comma < space)) FixNodeList (ts, comma);
      else if (space != NULL) FixNodeList (ts, space);
      }
      while (comma != NULL || space != NULL);
    FixNodeList (ts, ts + strlen (ts));
    nnodes2 = strlen (ts) / 2;
    sprintf (glob_condstr + 7 + 2 * nnodes, "%02d%s", nnodes2, ts);
    break;
    }
  else if (strstr (wname, "3") != NULL)
    sprintf (glob_condstr + 9 + 2 * nnodes, "%+d", sel - /* 9 */ 2);
}

void FixNodeList (char *str, char *del)
{
  int offset, value;

  offset = del - str;
  if (offset % 2 != 0)
    {
    value = str[offset - 1] - '0';
    if (*del == '\0') sprintf (str + offset - 1, "%02d", value);
    else
      {
      *del = str[offset - 1];
      str[offset - 1] = '0';
      }
    }
  else if (*del != '\0') strcpy (del, del + 1);
}

void CondDel_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  int which;

  which = (int) client_data;

  marked_for_del = !marked_for_del;

  if (curr_is_test) test_marked[curr_num] = marked_for_del;
  else cond_marked[curr_num] = marked_for_del;

  XtUnmanageChild (conddelpb[which]);

  if (marked_for_del) XtManageChild (conddellbl);
  else XtUnmanageChild (conddellbl);

  XtManageChild (conddelpb[1 - which]);
}

void PTHelp_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  switch (cond_type)
    {
  case PT_TYPE_ELECWD:
    Help_CB (w, (XtPointer) "ptelec:ELEC Condition Editor", call_data);
    break;
  case PT_TYPE_NUMMOLEC:
    Help_CB (w, (XtPointer) "ptmolec:MOLEC Condition Editor", call_data);
    break;
  case PT_TYPE_BULKY:
    Help_CB (w, (XtPointer) "ptsbulk:SBULK Condition Editor", call_data);
    break;
  case PT_TYPE_DIST:
    Help_CB (w, (XtPointer) "ptdist:DIST Condition Editor", call_data);
    break;
  case PT_TYPE_PATHLEN:
    Help_CB (w, (XtPointer) "ptconn:CONN/RNGSZ/RNGCP Condition Editor", call_data);
    break;
  case PT_TYPE_ALKYNE:
  case PT_TYPE_ALLENE:
    Help_CB (w, (XtPointer) "ptalsmr:ALSMR Condition Editor", call_data);
    break;
  case PT_TYPE_CARBONIUM:
    Help_CB (w, (XtPointer) "ptcarsb:CARSB Condition Editor", call_data);
    break;
  case PT_TYPE_LVNGROUP:
    Help_CB (w, (XtPointer) "ptlvgrp:LVGRP Condition Editor", call_data);
    break;
  case PT_TYPE_MIGRATAP:
    Help_CB (w, (XtPointer) "ptmgapt:MGAPT Condition Editor", call_data);
    break;
  case PT_TYPE_ATOM:
    Help_CB (w, (XtPointer) "ptatom:ATOM Condition Editor", call_data);
    break;
  case PT_TYPE_FG_XCESS:
    Help_CB (w, (XtPointer) "ptxcess:XCESS Condition Editor", call_data);
    break;
  case PT_TYPE_AT_CONEQ:
  case PT_TYPE_AT_STREQ:
  case PT_TYPE_DISC_CONEQ:
  case PT_TYPE_DISC_STREQ:
    Help_CB (w, (XtPointer) "ptcseq:CSEQ Condition Editor", call_data);
    break;
  case PT_TYPE_FG_CONEQ:
  case PT_TYPE_FG_STREQ:
    Help_CB (w, (XtPointer) "ptfgeq:FGEQ Condition Editor", call_data);
    break;
  case PT_TYPE_FG_CNT:
    Help_CB (w, (XtPointer) "ptmore:MORE Condition Editor", call_data);
    break;
  case PT_TYPE_AROMSUB:
    switch (cond_subtype)
      {
    case 1:
      Help_CB (w, (XtPointer) "ptarnod:ARNOD Condition Editor", call_data);
      break;
    case 2:
      Help_CB (w, (XtPointer) "ptarstd:ARSTD Condition Editor", call_data);
      break;
    case 3:
      Help_CB (w, (XtPointer) "ptarrat:ARRAT Condition Editor", call_data);
      break;
    case 4:
      Help_CB (w, (XtPointer) "ptarcet:ARCET Condition Editor", call_data);
      break;
    case 5:
      Help_CB (w, (XtPointer) "ptartie:ARTIE Condition Editor", call_data);
      break;
      }
    break;
  case PT_TEST_ADD:
    Help_CB (w, (XtPointer) "pttest:Individual Posttransform Test Editor", call_data);
    break;
  default:
    Help_CB (w, (XtPointer) "unknown_module:Unknown Module", call_data);
    break;
    }
}

void CondExit_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  if (!(Boolean_t) (int) client_data)
    {
    if (curr_is_test) test_marked[curr_num] = current_marked;
    else cond_marked[curr_num] = current_marked;
    }
  else postform_modified = TRUE;

  switch (cond_type)
    {
  case PT_TYPE_ELECWD:
    ElecExit_CB (w, client_data, call_data);
    break;
  case PT_TYPE_NUMMOLEC:
    MolecExit_CB (w, client_data, call_data);
    break;
  case PT_TYPE_BULKY:
    BulkExit_CB (w, client_data, call_data);
    break;
  case PT_TYPE_DIST:
    DistExit_CB (w, client_data, call_data);
    break;
  case PT_TYPE_PATHLEN:
    ConnExit_CB (w, client_data, call_data);
    break;
  case PT_TYPE_ALKYNE:
  case PT_TYPE_ALLENE:
    AlSmRExit_CB (w, client_data, call_data);
    break;
  case PT_TYPE_CARBONIUM:
    CarSbExit_CB (w, client_data, call_data);
    break;
  case PT_TYPE_LVNGROUP:
    LvGrpExit_CB (w, client_data, call_data);
    break;
  case PT_TYPE_MIGRATAP:
    MgAptExit_CB (w, client_data, call_data);
    break;
  case PT_TYPE_ATOM:
    AtomExit_CB (w, client_data, call_data);
    break;
  case PT_TYPE_FG_XCESS:
    XcessExit_CB (w, client_data, call_data);
    break;
  case PT_TYPE_AT_CONEQ:
  case PT_TYPE_AT_STREQ:
  case PT_TYPE_DISC_CONEQ:
  case PT_TYPE_DISC_STREQ:
    CSEqExit_CB (w, client_data, call_data);
    break;
  case PT_TYPE_FG_CONEQ:
  case PT_TYPE_FG_STREQ:
    FGEqExit_CB (w, client_data, call_data);
    break;
  case PT_TYPE_FG_CNT:
    MoreExit_CB (w, client_data, call_data);
    break;
  case PT_TYPE_AROMSUB:
    switch (cond_subtype)
      {
    case 1:
      ArNodExit_CB (w, client_data, call_data);
      break;
    case 2:
      ArStdExit_CB (w, client_data, call_data);
      break;
    case 3:
      ArRatExit_CB (w, client_data, call_data);
      break;
    case 4:
      ArCETExit_CB (w, client_data, call_data);
      break;
    case 5:
      ArTieExit_CB (w, client_data, call_data);
      break;
      }
    break;
  case PT_TEST_ADD:
    PTestExit_CB (w, client_data, call_data);
    break;
  default:
    break;
    }

/*
  if (cond_add_canceled || test_add_canceled) PostForm_Refresh ();
*/
  if (!client_data) PostForm_Refresh ();
}

void Cond_Prep_Mark (Boolean_t marked)
{
  marked_for_del = marked;

  if (!glob_rxlform)
    {
    XtUnmanageChild (condpb[0]);
    XtUnmanageChild (conddelpb[0]);
    XtUnmanageChild (conddellbl);
    XtUnmanageChild (conddelpb[1]);
    return;
    }
  XtManageChild (condpb[0]);
  if (marked)
    {
    XtUnmanageChild (conddelpb[0]);
    XtManageChild (conddellbl);
    XtManageChild (conddelpb[1]);
    }
  else
    {
    XtUnmanageChild (conddellbl);
    XtUnmanageChild (conddelpb[1]);
    XtManageChild (conddelpb[0]);
    }
}

void ElecExit_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  String_t string;
  char tmpstr[128], tmpstr2[128], savestr[128], *ts, *ts2, *comma, *space, *newstr;
  int i, nnodes, nnodes2;
  static char *blank = "";

  if ((Boolean_t) (int) client_data)
    {
/* first get final results of text fields */
    XtVaGetValues (electxt[0],
      XmNvalue, &ts,
      NULL);

    strcpy (tmpstr, ts);
    ts = tmpstr;
    do
      {
      comma = strstr (ts, ",");
      space = strstr (ts, " ");
      if (comma != NULL && (space == NULL || comma < space)) FixNodeList (ts, comma);
      else if (space != NULL) FixNodeList (ts, space);
      }
      while (comma != NULL || space != NULL);
    FixNodeList (ts, ts + strlen (ts));

    if (ts[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }

    if (XtIsManaged (electxt[1]))
      {
      XtVaGetValues (electxt[1],
        XmNvalue, &ts2,
        NULL);

      strcpy (tmpstr2, ts2);
      ts2 = tmpstr2;
      do
        {
        comma = strstr (ts2, ",");
        space = strstr (ts2, " ");
        if (comma != NULL && (space == NULL || comma < space)) FixNodeList (ts2, comma);
        else if (space != NULL) FixNodeList (ts2, space);
        }
        while (comma != NULL || space != NULL);
      FixNodeList (ts2, ts2 + strlen (ts2));
      if (ts2[0] == '\0')
        {
        XtManageChild (emptytextmsg);
        return;
        }
      }
    else ts2 = blank;

/* incorporate into string */
    sscanf (glob_condstr + 3, "%2d", &nnodes);
    strcpy (savestr, glob_condstr + 5 + 2 * nnodes);
    nnodes = strlen (ts) / 2;
    sprintf (glob_condstr + 3, "%02d%s%s", nnodes, ts, savestr);
    nnodes2 = strlen (ts2) / 2;
    if (nnodes2 != 0)
      sprintf (glob_condstr + 7 + 2 * nnodes, "%02d%s", nnodes2, ts2);

    if (nnodes + nnodes2 >= MX_NODES)
      {
      XtManageChild (nodexceedmsg);
      return;
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
  XtDestroyWidget (elecform);
}

void NodeList_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *lcl_node_str, node_str[128];
  int len;
  static Boolean_t internal_change = FALSE;

  if (internal_change) return; /* ignore callback due to field truncation */

  lcl_node_str = XmTextGetString(w);

  if (BadNodeList (lcl_node_str))
    {
    strcpy (node_str, lcl_node_str);
    len = strlen (node_str);
    node_str[len - 1] = '\0';
    XtFree(lcl_node_str);
    internal_change = TRUE;
    XmTextReplace (w,0,len,node_str);
/*
    XtVaSetValues (w,
      XmNvalue, node_str,
      NULL);
*/
    internal_change = FALSE;
    XBell (XtDisplay (w), 10);
    }
  else XtFree(lcl_node_str);
}

void MtTxtMsg_Dismiss_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild (XtParent (widg));
}

Boolean_t BadNodeList (char *str)
{
  char chr;
  int len, i;

/* empty string OK here */
  if (str[0] == '\0') return (FALSE);
/* must start with digit */
  if (str[0] < '0' || str[0] > '9') return (TRUE);

  len = strlen (str);
  chr = str[len - 1];

/* multiple spaces are fine */
  if (chr == ' ') return (FALSE);

  if (chr >= '0' && chr <= '9')
    {
/* 1 or 2 digits per node */
    if (len <= 2) return (FALSE);

    for (i = 2; i < 4; i++)
      {
      chr = str[len - i];
      if (chr < '0' || chr > '9') return (FALSE);
      }

    return (TRUE);
    }

  if (chr == ',')
    {
/* comma must follow digit to be valid */
    chr = str[len - 2];
    return (chr < '0' || chr > '9');
    }

/* any other character is invalid */
  return (TRUE);
}
