/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              TYPMOL.C
*
*    This module provides for the display and editing of the application type
*    and maximum molecularity of the reaction schema.
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

#include <Xm/DialogS.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/Form.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#include "synchem.h"
#include "synio.h"
#include "app_resrc.h"

#ifndef _H_REACTION_
#include "reaction.h"
#endif

void TMTxt_CB (Widget, XtPointer, XtPointer);
void TMOpt_CB (Widget, XtPointer, XtPointer);
void TMPB_CB (Widget, XtPointer, XtPointer);

extern Boolean_t glob_rxlform;

static void (*ret_func)(int, int);

static int type = REACT_SING_APPL_ONLY, mol = 1;

static char *type_directions[2] =
  {"To dictate how SYNCHEM should rank subgoals on the basis of the number of times this schema is applied to a given target",
   "molecule, please select one of the following:"};

static char *type_choices[5] =
  {"SAO (Single Application Only - reject all others)",
   "MPS (Multiple Prefer Single)",
   "MNP (Multiple No Preference)",
   "MPM (Multiple Prefer Multiple)",
   "MAO (Maximum Application Only - reject all others)"};

static char *mol_directions[4] =
  {"Certain reactions having more than one subgoal piece (e.g., dimerizations) are practical only when like molecules react.",
   "This can be accomplished by choosing an appropriate MAXUSM (maximum number of unique subgoal molecules).  If necessary,",
   "finer control may be had through the use of the MOLEC type of posttransform condition.  Please enter a value for MAXUSM",
   "that is appropriate for this schema:"};

static char *tm_buttons[2] =
  {"Exit and Save", "Quit and Cancel"};

static Widget formdg, moltxt, tdummy, tmenu, mollbl[4], varform, tmpform, tmpb[2];

void TM_Form_Create (Widget top_level)
{
  XmString title, label, choice_lbl[5];
  Widget typelbl[2];
  int i, j;
  char wname[16];

  formdg = XmCreateFormDialog (top_level, "TMForm", NULL, 0);

  title = XmStringCreateLocalized ("Type and Molecularity");
  XtVaSetValues (formdg,
    XmNdialogStyle,     XmDIALOG_MODELESS,
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNbuttonFontList,  SynAppR_FontList_Get (&GSynAppR),
    XmNtextFontList,    SynAppR_FontList_Get (&GSynAppR),
    XmNbackground,      SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground,      SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNresizePolicy,    XmRESIZE_NONE,
    XmNresizable,       True,
    XmNautoUnmanage,    False,
    XmNdialogTitle,     title,
    XmNdefaultPosition, False,
    XmNheight,          2 * AppDim_AppHeight_Get (&GAppDim) / 5,
    XmNwidth,           AppDim_AppWidth_Get (&GAppDim),
    XmNfractionBase,    800,
    NULL);
  XmStringFree (title);

  for (i = 0; i < 2; i++)
    {
    label = XmStringCreateLocalized (type_directions[i]);
    sprintf (wname, "TLbl%d", i);
    typelbl[i] = XtVaCreateManagedWidget (wname,
      xmLabelWidgetClass, formdg,
      XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
      XmNforeground,      SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
      XmNlabelString, label,
      XmNtopOffset, 0,
      XmNtopAttachment, i == 0 ? XmATTACH_FORM : XmATTACH_WIDGET,
      XmNtopWidget, i == 0 ? NULL : typelbl[i - 1],
      XmNleftPosition, 10,
      XmNleftAttachment, XmATTACH_POSITION,
      NULL);
    XmStringFree (label);
    }

  for (i = 0; i < 2; i++)
    {
    label = XmStringCreateLocalized (tm_buttons[i]);
    sprintf (wname, "TMPB%d", i);
    tmpb[i] = XtVaCreateManagedWidget (wname,
      xmPushButtonGadgetClass, formdg,
      XmNlabelString, label,
      XmNtopPosition, 750,
      XmNtopAttachment, XmATTACH_POSITION,
      XmNleftPosition, 300 * i + 200,
      XmNleftAttachment, XmATTACH_POSITION,
      NULL);
    XmStringFree (label);
    XtAddCallback (tmpb[i], XmNactivateCallback, TMPB_CB, (XtPointer) i);
    }

  varform = XtVaCreateManagedWidget ("TMVarForm",
    xmFormWidgetClass, formdg,
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground,      SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, typelbl[1],
    XmNbottomPosition, 700,
    XmNbottomAttachment, XmATTACH_POSITION,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, 5,
    XmNrightAttachment, XmATTACH_FORM,
    XmNfractionBase,    800,
    NULL);
}

void TM_Open (int inp_type, int inp_mol, void (*return_function)(int, int))
{
  XmString title, label, choice_lbl[5];
  char molval[4], wname[16];
  int i, sel;

#ifdef _DEBUG_
printf("TM_Open 1\n");
#endif
  type = inp_type;
  mol = inp_mol;
  ret_func = return_function;

  sprintf (molval, "%d", mol);

#ifdef _DEBUG_
printf("TM_Open 2\n");
#endif
  tmpform = XtVaCreateManagedWidget ("TMVarForm",
    xmFormWidgetClass, varform,
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground,      SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    XmNfractionBase,    800,
    NULL);

#ifdef _DEBUG_
printf("TM_Open 3\n");
#endif
  for (i = 0; i < 5; i++) choice_lbl[i] = XmStringCreateLocalized (type_choices[i]);
  tdummy = XmVaCreateSimplePulldownMenu (tmpform, "_tpulldown", 0, NULL, XmNbuttonFontList, SynAppR_FontList_Get (&GSynAppR), NULL);

  if (type == REACT_SING_APPL_ONLY) sel = 0;
  else if (type == REACT_MULT_PREF_SING) sel = 1;
  else if (type == REACT_MULT_NO_PREF) sel = 2;
  else if (type == REACT_MULT_PREF_MULT) sel = 3;
  else if (type == REACT_MAX_APPL_ONLY) sel = 4;
  else sel = 0;

#ifdef _DEBUG_
printf("TM_Open 4\n");
#endif
  title = XmStringCreateLocalized ("Type and Molecularity");
  tmenu = XmVaCreateSimpleOptionMenu (tmpform, "_tmenu", title, '\0', sel, TMOpt_CB,
    XmNsubMenuId, tdummy,
    XmNbuttonFontList, SynAppR_FontList_Get (&GSynAppR),
    XmNbackground, SynAppR_IthClrPx_Get(&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get(&GSynAppR, SAR_CLRI_BLACK),
    XmVaPUSHBUTTON, choice_lbl[0], '\0', NULL, NULL,
    XmVaPUSHBUTTON, choice_lbl[1], '\0', NULL, NULL,
    XmVaPUSHBUTTON, choice_lbl[2], '\0', NULL, NULL,
    XmVaPUSHBUTTON, choice_lbl[3], '\0', NULL, NULL,
    XmVaPUSHBUTTON, choice_lbl[4], '\0', NULL, NULL,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftPosition, 20,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);
  XmStringFree (title);
  for (i=0; i<5; i++) XmStringFree (choice_lbl[i]);
#ifdef _DEBUG_
printf("TM_Open 4.5\n");
#endif
  XtManageChild (tmenu);

#ifdef _DEBUG_
printf("TM_Open 5\n");
#endif
  for (i = 0; i < 4; i++)
    {
    label = XmStringCreateLocalized (mol_directions[i]);
    sprintf (wname, "MLbl%d", i);
    mollbl[i] = XtVaCreateManagedWidget (wname,
      xmLabelWidgetClass, tmpform,
      XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
      XmNforeground,      SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
      XmNlabelString, label,
      XmNtopOffset, i == 0 ? 25 : 0,
      XmNtopAttachment, XmATTACH_WIDGET,
      XmNtopWidget, i == 0 ? tmenu : mollbl[i - 1],
      XmNleftPosition, 10,
      XmNleftAttachment, XmATTACH_POSITION,
      NULL);
    XmStringFree (label);
    }

#ifdef _DEBUG_
printf("TM_Open 6\n");
#endif
  moltxt = XtVaCreateManagedWidget ("MTxt",
    xmTextFieldWidgetClass, tmpform,
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground,      SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNvalue, molval,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget, mollbl[3],
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, mollbl[3],
    NULL);
  XtAddCallback (moltxt, XmNvalueChangedCallback, TMTxt_CB, (XtPointer) NULL);

#ifdef _DEBUG_
printf("TM_Open 7\n");
#endif
  XtManageChild (formdg);
  if (glob_rxlform) XtManageChild (tmpb[0]);
  else XtUnmanageChild (tmpb[0]);
#ifdef _DEBUG_
printf("TM_Open 8\n");
#endif
}

void TMTxt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *lcl_str, str[128];
  int len, i;
  static Boolean_t internal_change = FALSE;

  if (internal_change) return; /* ignore callback due to field truncation */

  lcl_str = XmTextGetString(w);
  strcpy (str, lcl_str);
  XtFree (lcl_str);

  len = strlen (str);

  if (len > 1)
    {
    str[len - 1] = '\0';
    internal_change = TRUE;
    XmTextReplace (w,0,len,str);
    len--;
    internal_change = FALSE;
    XBell (XtDisplay (w), 10);
    }

  if (len == 1 && (str[0] < '0' || str[0] > '9'))
    {
    strcpy (str, str + 1);
    internal_change = TRUE;
    XmTextReplace (w,0,len,str);
    len--;
    internal_change = FALSE;
    XBell (XtDisplay (w), 10);
    }
}

void TMOpt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *wname;
  int sel;

  sel = (int) client_data;

  if (sel == 0) type = REACT_SING_APPL_ONLY;
  else if (sel == 1) type = REACT_MULT_PREF_SING;
  else if (sel == 2) type = REACT_MULT_NO_PREF;
  else if (sel == 3) type = REACT_MULT_PREF_MULT;
  else type = REACT_MAX_APPL_ONLY;
}

void TMPB_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *ts, choices[4], eycval[3][5];
  int i;
  XmString title;

  switch ((int) client_data)
    {
  case 0: /* Exit and Save */
    XtVaGetValues (moltxt,
      XmNvalue, &ts,
      NULL);
    mol = atoi (ts);
    XtDestroyWidget (tmpform);
    XtUnmanageChild (formdg);
    (*ret_func) (type, mol);
    break;
  case 1: /* Quit and Cancel */
    XtDestroyWidget (tmpform);
    XtUnmanageChild (formdg);
    break;
    }
}
