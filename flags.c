/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:                     FLAGS.C
*
*    GUI for changing the values of the protection, lookahead, and disable
*    schema flags.
*
*  Creation Date:
*
*    23-Feb-2000
*
*  Authors:
*
*    Gerald A. Miller
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
*
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
#include <Xm/RowColumn.h>

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#include "synchem.h"
#include "app_resrc.h"

extern Boolean_t glob_rxlform;
static void (*ret_func)(Boolean_t, Boolean_t, Boolean_t);

void FlagsCB_CB (Widget, XtPointer, XtPointer);
void FlagsPB_CB (Widget, XtPointer, XtPointer);

static Widget formdg, flagcb, flagpb[2];
static Boolean_t local_values[3];

void Flags_Form_Create (Widget top_level)
{
  XmString title, button_lbl[2], choice_lbl[3];
  int i, j, nlines, nchoices;
  char wname[16], eycval[3][5];

  formdg = XmCreateFormDialog (top_level, "FlagsForm", NULL, 0);

  title = XmStringCreateLocalized ("Reaction Flags");
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
    XmNheight,          AppDim_AppHeight_Get (&GAppDim)/5,
    XmNwidth,           AppDim_AppWidth_Get (&GAppDim)/2,
    XmNfractionBase,    100,
    NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized ("Schema Flags");
  choice_lbl[0] = XmStringCreateLocalized ("<PROT>: Schema is a protection reaction");
  choice_lbl[1] = XmStringCreateLocalized ("<LOOK>: Schema is a lookahead reaction");
  choice_lbl[2] = XmStringCreateLocalized ("<DISA>: Schema is disabled");
  flagcb = XmVaCreateSimpleCheckBox (formdg, "Schema_Flags", FlagsCB_CB,
    XmVaCHECKBUTTON, choice_lbl[0], NULL, NULL, NULL,
    XmVaCHECKBUTTON, choice_lbl[1], NULL, NULL, NULL,
    XmVaCHECKBUTTON, choice_lbl[2], NULL, NULL, NULL,
    XmNtitle, title,
    NULL);
  XmStringFree (title);
  for (i = 0; i < 3; i++)
    XmStringFree (choice_lbl[i]);

  XtVaSetValues (flagcb,
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftPosition, 5,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);

  button_lbl[0] = XmStringCreateLocalized ("Exit and Save");
  button_lbl[1] = XmStringCreateLocalized ("Quit and Cancel");
  for (i = 0; i < 2; i++)
    {
    sprintf (wname, "FlagPB%d", i);
    flagpb[i] = XtVaCreateManagedWidget (wname,
      xmPushButtonGadgetClass, formdg,
      XmNlabelString, button_lbl[i],
      XmNtopPosition, 80,
      XmNtopAttachment, XmATTACH_POSITION,
      XmNleftPosition, 50 * i + 10,
      XmNleftAttachment, XmATTACH_POSITION,
      NULL);
    XmStringFree (button_lbl[i]);
    XtAddCallback (flagpb[i], XmNactivateCallback, FlagsPB_CB, (XtPointer) i);
    }

  XtManageChild (flagcb);
}

void Flags_Open (Boolean_t prot, Boolean_t look, Boolean_t disa, void (*return_function)(Boolean_t, Boolean_t, Boolean_t))
{
  Widget togbut[3];
  int i;
  XmString label[3];

  togbut[0] = XtNameToWidget (flagcb, "button_0");
  togbut[1] = XtNameToWidget (flagcb, "button_1");
  togbut[2] = XtNameToWidget (flagcb, "button_2");
  local_values[0] = prot;
  local_values[1] = look;
  local_values[2] = disa;
  label[0] = XmStringCreateLocalized (prot ? "<PROT>: Schema is a protection reaction" :
    "<PROT>: Schema is NOT a protection reaction");
  label[1] = XmStringCreateLocalized (look ? "<LOOK>: Schema is a lookahead reaction" :
    "<LOOK>: Schema is NOT a lookahead reaction");
  label[2] = XmStringCreateLocalized (disa ? "<DISA>: Schema is disabled" : "<DISA>: Schema is NOT disabled");

  for (i = 0; i < 3; i++)
    {
    XmToggleButtonSetState (togbut[i], local_values[i] ? True : False, False);
    XtVaSetValues (togbut[i],
      XmNlabelString, label[i],
      NULL);
    XmStringFree (label[i]);
    }
  ret_func = return_function;

  XtManageChild (formdg);
  if (glob_rxlform) XtManageChild (flagpb[0]);
  else XtUnmanageChild (flagpb[0]);
}

void FlagsCB_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *wname;
  int which, sel;
  XmString label;

  wname = XtName (w);
  which = wname[strlen(wname) - 1] - '0';
  local_values[which] = XmToggleButtonGetState (w);

  switch (which)
    {
  case 0:
    label = XmStringCreateLocalized (local_values[which] ? "<PROT>: Schema is a protection reaction" :
      "<PROT>: Schema is NOT a protection reaction");
    break;
  case 1:
    label = XmStringCreateLocalized (local_values[which] ? "<LOOK>: Schema is a lookahead reaction" :
      "<LOOK>: Schema is NOT a lookahead reaction");
    break;
  case 2:
    label = XmStringCreateLocalized (local_values[which] ? "<DISA>: Schema is disabled" : "<DISA>: Schema is NOT disabled");
    break;
    }
  XtVaSetValues (w,
    XmNlabelString, label,
    NULL);
  XmStringFree (label);
}

void FlagsPB_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *ts, choices[4], eycval[3][5];
  int i;

  switch ((int) client_data)
    {
  case 0: /* Exit and Save */
    XtUnmanageChild (formdg);
    (*ret_func) (local_values[0], local_values[1], local_values[2]);
    break;
  case 1: /* Quit and Cancel */
    XtUnmanageChild (formdg);
    break;
    }
}
