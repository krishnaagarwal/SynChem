/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              REFCOMM.C
*
*    This module provides for the display and editing of references and
*    comments associated with a given reaction schema.
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
#include <Xm/RowColumn.h>
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

#include "synhelp.h"

#ifndef _H_REACTION_
#include "reaction.h"
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

void RCTxt_CB (Widget, XtPointer, XtPointer);
void RCPB_CB (Widget, XtPointer, XtPointer);
void disp_curr ();

extern Boolean_t glob_rxlform;

static void (*ret_func)(int, int, String_t *, String_t *);

static int nr = 0, nc = 0, current = -1, refnums[500], num_not_libupd = 0;
static String_t *r_array_ptr = NULL, *c_array_ptr = NULL;
static Boolean_t current_is_ref = TRUE;

static char *rc_directions = "Current reference or comment may be edited below:";

static char *rc_buttons[8] =
  {"Previous", "Next", "Delete Current", "New Reference", "New Comment", "Exit and Save", "Quit and Cancel", "Help"};

static int rc_bpos[8][4]  = {{625, 675, 305, 390}, {625, 675, 405, 500}, {690, 740, 0, 125}, {690, 740, 245, 370},
  {690, 740, 430, 555}, {750, 800, 220, 345}, {750, 800, 455, 580}, {690, 740, 674, 799}};

static Widget formdg, rctxt, rclbl2, rcpb[7];
#ifdef _CYGHAT_
static Widget box;
#endif

void RC_Form_Create (Widget top_level)
{
  XmString title, label;
  Widget rclbl;
  int i, j;
  char wname[16];
#ifdef _CYGHAT_
Arg al[50];
int ac;
#endif

  formdg = XmCreateFormDialog (top_level, "TMForm", NULL, 0);

  title = XmStringCreateLocalized ("References and Comments");
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
    XmNheight,          AppDim_AppHeight_Get (&GAppDim) / 2,
    XmNwidth,           AppDim_AppWidth_Get (&GAppDim),
    XmNfractionBase,    800,
XmNy, 25, /* for window managers that are too stupid to put the top border on the screen! */
    NULL);
  XmStringFree (title);

  label = XmStringCreateLocalized (rc_directions);
  rclbl = XtVaCreateManagedWidget ("RCLbl",
    xmLabelWidgetClass, formdg,
    XmNlabelString, label,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    NULL);
  XmStringFree (label);

  label = XmStringCreateLocalized ("<REF>");
  rclbl2 = XtVaCreateManagedWidget ("RCLbl2",
    xmLabelWidgetClass, formdg,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNlabelString, label,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, rclbl,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    NULL);
  XmStringFree (label);

#ifdef _CYGHAT_
  box = XtVaCreateManagedWidget ("box",
    xmRowColumnWidgetClass, formdg,
    XmNleftAttachment,      XmATTACH_WIDGET,
    XmNleftWidget,          rclbl2,
    XmNrightAttachment,     XmATTACH_FORM,
    XmNbottomAttachment,    XmATTACH_POSITION,
    XmNbottomPosition,      600,
    XmNorientation,         XmHORIZONTAL,
    XmNheight,              1,
    NULL);

ac=0;
XtSetArg(al[ac],
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR, SAR_CLRI_WHITE)); ac++;
XtSetArg(al[ac],
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR, SAR_CLRI_BLACK)); ac++;
XtSetArg(al[ac],
    XmNeditMode, XmMULTI_LINE_EDIT); ac++;
XtSetArg(al[ac],
    XmNscrollVertical, True); ac++;

  rctxt = XmCreateScrolledText (formdg, "rctxt", al, ac);
#else

  rctxt = XmCreateScrolledText (formdg, "rctxt", NULL, 0);

  XtVaSetValues (rctxt,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNeditMode, XmMULTI_LINE_EDIT,
    XmNscrollVertical, True,
    NULL);
#endif

  XtVaSetValues (XtParent (rctxt),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, rclbl,
#ifdef _CYGHAT_
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget,     box,
#else
    XmNbottomPosition, 600,
    XmNbottomAttachment, XmATTACH_POSITION,
#endif
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, rclbl2,
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    NULL);
  XtManageChild (rctxt);
  XtAddCallback (rctxt, XmNvalueChangedCallback, RCTxt_CB, (XtPointer) NULL);

  for (i = 0; i < 8; i++)
    {
    label = XmStringCreateLocalized (rc_buttons[i]);
    sprintf (wname, "RCPB%d", i);
    rcpb[i] = XtVaCreateManagedWidget (wname,
      xmPushButtonGadgetClass, formdg,
      XmNlabelString, label,
      XmNtopPosition, rc_bpos[i][0],
      XmNtopAttachment, XmATTACH_POSITION,
      XmNbottomPosition, rc_bpos[i][1],
      XmNbottomAttachment, XmATTACH_POSITION,
      XmNleftPosition, rc_bpos[i][2],
      XmNleftAttachment, XmATTACH_POSITION,
      XmNrightPosition, rc_bpos[i][3],
      XmNrightAttachment, XmATTACH_POSITION,
      NULL);
    XmStringFree (label);
    if (i == 7)
      XtAddCallback (rcpb[i], XmNactivateCallback, Help_CB, (XtPointer) "refcomm:Reference and Comment Editor");
    else
      XtAddCallback (rcpb[i], XmNactivateCallback, RCPB_CB, (XtPointer) i);
    }
}

void delete_ref ()
{
  int i;

  String_Destroy (r_array_ptr[refnums[current]]);
  for (i = refnums[current]; i < nr - 1; i++) r_array_ptr[i] = r_array_ptr[i + 1];
  for (i = current; i < num_not_libupd - 1; i++) refnums[i] = refnums[i + 1];
  nr--;
  num_not_libupd--;
  r_array_ptr = (String_t *) realloc (r_array_ptr, nr * sizeof (String_t));
}

void delete_comm ()
{
  int i;

  String_Destroy (c_array_ptr[current]);
  for (i = current; i < nc - 1; i++) c_array_ptr[i] = c_array_ptr[i + 1];
  nc--;
  c_array_ptr = (String_t *) realloc (c_array_ptr, nc * sizeof (String_t));
}

void add_rc ()
{
  int new;
  String_t *p;

  if (current_is_ref)
    {
    current = num_not_libupd++;
    new = refnums[current] = nr++;
    p = r_array_ptr = (String_t *) realloc (r_array_ptr, nr * sizeof (String_t));
    }
  else
    {
    new = current = nc++;
    p = c_array_ptr = (String_t *) realloc (c_array_ptr, nc * sizeof (String_t));
    }

  p[new] = String_Create ("", 0);
  disp_curr ();
}

void copy_strings (String_t **dest, String_t *src, int nstr)
{
  int i;

  dest[0] = (String_t *) malloc (nstr * sizeof (String_t));
  for (i = 0; i < nstr; i++) dest[0][i] = String_Copy (src[i]);
}

void del_strings (String_t **array, int nstr)
{
  int i;

  for (i = 0; i < nstr; i++) String_Destroy (array[0][i]);
  free (array[0]);
  array[0] = NULL;
}

void RCWrap_Text (char *textstr)
{
  int wrap_pos, i;

  for (i=0, wrap_pos=-1; i<strlen(textstr); i++)
    {
    if (textstr[i] == '\n') textstr[i] = ' ';
    if (textstr[i] == ' ' && i - wrap_pos > 125) textstr[wrap_pos = i] = '\n';
    }
}

void update_string (String_t *string)
{
  char *ts;

  XtVaGetValues (rctxt,
    XmNvalue, &ts,
    NULL);

  String_Destroy (string[0]);
  string[0] = String_Create ((const char *) /* WHY???!!! */ ts, 0);
}

void disp_curr ()
{
  char textstr[2048], lblstr[8];
  XmString label;

  if (current < 0)
    {
    strcpy (textstr, "");
    strcpy (lblstr, "<NONE>");
    }
  else if (current_is_ref)
    {
    strcpy (textstr, String_Value_Get (r_array_ptr[refnums[current]]));
    strcpy (lblstr, "<REF>");
    }
  else
    {
    strcpy (textstr, String_Value_Get (c_array_ptr[current]));
    strcpy (lblstr, "<COMM>");
    }

  label = XmStringCreateLocalized (lblstr);
  XtVaSetValues (rclbl2,
    XmNlabelString, label,
    NULL);
  XmStringFree (label);

  XtVaSetValues (rctxt,
    XmNvalue, textstr,
    XmNcursorPosition, strlen(textstr),
    NULL);
}

void next_comm (Boolean_t beep)
{
  if (current == nc - 1)
    {
    if (beep) XBell (XtDisplay (formdg), 10);
    else disp_curr ();
    return;
    }
  current++;
  disp_curr ();
}

void next_ref (Boolean_t beep)
{
  current++;
  if (current == num_not_libupd)
    {
    if (nc > 0 || !beep)
      {
      current = -1;
      current_is_ref = FALSE;
      next_comm (beep);
      }
    else
      {
      if (beep) XBell (XtDisplay (formdg), 10);
      current--;
      }
    return;
    }
  disp_curr ();
}

void prev_ref (Boolean_t beep)
{
  if (current == 0)
    {
    if (beep) XBell (XtDisplay (formdg), 10);
    return;
    }
  current--;
  disp_curr ();
}

void prev_comm (Boolean_t beep)
{
  current--;
  if (current < 0)
    {
    if (num_not_libupd > 0)
      {
      current = num_not_libupd + current + 1;
      current_is_ref = TRUE;
      prev_ref (beep);
      }
    else
      {
      if (beep) XBell (XtDisplay (formdg), 10);
      current++;
      }
    return;
    }
  disp_curr ();
}

void RC_Open (int inp_nr, int inp_nc, String_t *inp_r, String_t *inp_c, void (*return_function)(int, int, String_t *, String_t *))
{
  XmString title, label, choice_lbl[5];
  char molval[4], wname[16];
  int i;

  nr = inp_nr;
  nc = inp_nc;
  copy_strings (&r_array_ptr, inp_r, nr);
  for (i = num_not_libupd = 0; i < nr; i++) if (String_Value_Get (r_array_ptr[i])[0] != '\007')
    refnums[num_not_libupd++] = i;
  copy_strings (&c_array_ptr, inp_c, nc);
  ret_func = return_function;
  current = -1;
  current_is_ref = TRUE;
  next_ref (FALSE);

  XtManageChild (formdg);
  for (i = 2; i < 6; i++) if (glob_rxlform) XtManageChild (rcpb[i]);
  else XtUnmanageChild (rcpb[i]);
}

void RCTxt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *ts, textstr[2048];
  int curpos;
  static Boolean_t internal_change = FALSE;

  if (internal_change) return; /* ignore callback due to username clearing */

  internal_change = TRUE;
  XtVaGetValues (rctxt,
    XmNcursorPosition, &curpos,
    XmNvalue, &ts,
    NULL);

  strcpy (textstr, ts);

  RCWrap_Text (textstr);

  XtVaSetValues (rctxt,
    XmNcursorPosition, curpos,
    XmNvalue, textstr,
    NULL);
  internal_change = FALSE;
}

void RCPB_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *ts, choices[4], eycval[3][5];
  int i;
  XmString title;

  if (current >= 0)
    {
    if (current_is_ref) update_string (r_array_ptr + refnums[current]);
    else update_string (c_array_ptr + current);
    }

  switch ((int) client_data)
    {
  case 0: /* Previous */
    if (current_is_ref) prev_ref (TRUE);
    else prev_comm (TRUE);
    break;
  case 1: /* Next */
    if (current_is_ref) next_ref (TRUE);
    else next_comm (TRUE);
    break;
  case 2: /* Delete Current */
    if (current_is_ref) delete_ref ();
    else delete_comm ();
    current_is_ref = TRUE;
    current = -1;
    next_ref (FALSE);
    break;
  case 3: /* New Reference */
    current_is_ref = TRUE;
    add_rc ();
    break;
  case 4: /* New Comment */
    current_is_ref = FALSE;
    add_rc ();
    break;
  case 5: /* Exit and Save */
    XtUnmanageChild (formdg);
    (*ret_func) (nr, nc, r_array_ptr, c_array_ptr);
    break;
  case 6: /* Quit and Cancel */
    del_strings (&r_array_ptr, nr);
    del_strings (&c_array_ptr, nc);
    XtUnmanageChild (formdg);
    break;
    }
}
