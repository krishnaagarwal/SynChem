/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              POSTFORM.C
*
*    This module provides for the display and editing of posttransform
*    conditions and tests.
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
#include <stdlib.h>
#include <malloc.h>
#include <math.h>

#include <Xm/DialogS.h>
#include <Xm/DrawingA.h>
#include <Xm/Text.h>
#include <Xm/Label.h>
#include <Xm/MessageB.h>
#include <Xm/PanedW.h>
#include <Xm/PushB.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>

#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_TSD_
#include "tsd.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_TSD2XTR_
#include "tsd2xtr.h"
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

#include "rxnpatt_draw.h"

#ifndef _CYGHAT_ /* This is really only needed for OpenMotif, which is lamer than lesstif, but for testing purposes ... */
#ifdef _CYGWIN_
#define _CYGHAT_
#else
#ifdef _REDHAT_
#define _CYGHAT_
#endif
#endif
#endif

extern Boolean_t cond_marked[], test_marked[], cond_add_canceled, test_add_canceled, glob_rxlform;
Boolean_t postform_modified;

static int            ncond = 0, ntest = 0, curr_schn = 0, NSch, init_ncond = 0, init_ntest = 0;
static Condition_t   *cond;
static Posttest_t    *test;
static String_t      *reas, *chem;
static char        ***condv[2], ***testv[2], text_buf[2][50000];
static Boolean_t      new_view = FALSE, node_used[100], text_ready = FALSE, callbacks_enabled = FALSE;
static Widget         tl, formdg, darea[2], conf_box, text[2];
static Display *disp;
static GC gc;
static Pixmap drawp[2];
static char *condtypes[] =
                {"MENU", "ELEC", "MOLEC", "SBULK", "DIST", "CONN", "RNGSZ", "RNGCP", "ALSMR", "CARSB",
                 "LVGRP", "MGAPT", "ATOM", "XCESS", "CSEQ", "FGEQ", "MORE", "ARNOD",
                 "ARSTD", "ARRAT", "ARCET", "ARTIE", "BRGHD"};

void SchformManage (int, Boolean_t);
void pt_update_tests (int, int, Condition_t *, int, Posttest_t *, String_t *, String_t *);

void ScrlText_CB (Widget, XtPointer, XtPointer);
void ExitPT_CB (Widget, XtPointer, XtPointer);
void NumPat_CB (Widget, XtPointer, XtPointer);
void PFQuit_Confirm_CB (Widget, XtPointer, XtPointer);
void redraw_gsgpatt (Widget, XtPointer, XtPointer);

void Post_Cond_Mod (Condition_t *, Condition_t *, Posttest_t *, int, int, int, Widget, Widget, char *);
void Post_Test_Mod (Posttest_t *, Condition_t *, Posttest_t *, String_t *, String_t *, int, int, int, Widget, Widget, Boolean_t);
void ptread_no_init (int, int *, Condition_t **, int *, Posttest_t **, String_t **, String_t **);
void ptwrite (int, Condition_t *, char ****, int, Posttest_t *, char ****, String_t *, String_t *, int, Boolean_t *);
void draw_pattern (Widget, Pixmap, int, int, Boolean_t *);
void mdraw2 (Dsp_Molecule_t *, Tsd_t *, Display *, Window, GC, int, int, int, int, Boolean_t, Boolean_t *, Boolean_t *);

int Post_Test_Count (int schn, Boolean_t new_schema)
{
/*
  Boolean_t new_schema;

  NSch = React_NumSchemas_Get ();
  new_schema = NSch == schn;
*/
  if (!new_schema) ptread_no_init (schn + 1, &ncond, &cond, &ntest, &test, &reas, &chem);

  return (ntest);
}

void Post_Form_Create (Widget top_level, int schn)
{
  Boolean_t first, new_schn, new_schema, update_init;
  static Boolean_t numbered[2] = {FALSE, FALSE};
  static Widget lbl[2], twin[2], pb[3], pwin, dwpform, dwin[2], tform, dwform[2], dwlbl[2], helppb;
  static ScreenAttr_t     *sca_p;
  XmString title, msg_str;
  int i, j, k, testnum;
  char *tbuf_p[2], *cond_p, condname[16];
#ifdef _CYGHAT_
  Widget box[2];
  Arg al[50];
  int ac;
#endif

  tl = top_level;
  NSch = React_NumSchemas_Get ();
  first = curr_schn == 0;
  if (schn < -1)
  {
    schn = -schn - 2;
    update_init = FALSE;
  }
  else update_init = TRUE;

/*
  new_schn = curr_schn != schn + 1;
*/
  new_schn = schn >= 0 && curr_schn != schn + 1;
/*
  new_schema = NSch == schn;
*/
  new_schema = schn == -1;
/*
  curr_schn = schn + 1;
*/
  if (!new_schema) curr_schn = schn + 1;
  text_ready = FALSE;

  if (first)
    {
    conf_box = XmCreateMessageDialog (top_level, "Confirm", NULL, 0);
    title=XmStringCreateLocalized("Confirm");
    msg_str=XmStringCreateLocalized("Quitting will cause all your modifications to be discarded.  Continue?");
    XtVaSetValues (conf_box,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNdialogTitle, title,
        XmNmessageString, msg_str,
        NULL);
    XmStringFree(title);
    XmStringFree(msg_str);

    XtUnmanageChild (XmMessageBoxGetChild (conf_box, XmDIALOG_HELP_BUTTON));

    XtAddCallback (XmMessageBoxGetChild (conf_box, XmDIALOG_OK_BUTTON), XmNactivateCallback, PFQuit_Confirm_CB,
      (XtPointer) (int) TRUE);
    XtAddCallback (XmMessageBoxGetChild (conf_box, XmDIALOG_CANCEL_BUTTON), XmNactivateCallback, PFQuit_Confirm_CB,
      (XtPointer) (int) FALSE);
    XtUnmanageChild (conf_box);

    sca_p = SynAppR_ScrnAtrb_Get (&GSynAppR);

    formdg = XmCreateFormDialog (top_level, "PTEdit", NULL, 0);
    title = XmStringCreateLocalized ("PostTransform Test Editor");
    XtVaSetValues (formdg,
      XmNdialogStyle,     XmDIALOG_MODELESS,
      XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
      XmNbuttonFontList,  SynAppR_FontList_Get (&GSynAppR),
      XmNtextFontList,    SynAppR_FontList_Get (&GSynAppR),
      XmNresizePolicy,    XmRESIZE_NONE,
      XmNresizable,       True,
      XmNautoUnmanage,    False,
      XmNdialogTitle,     title,
      XmNdefaultPosition, False,
/*
      XmNheight,          AppDim_AppHeight_Get (&GAppDim),
      XmNwidth,           AppDim_AppWidth_Get (&GAppDim),
*/
      XmNheight,          9 * Screen_Height_Get (sca_p) / 10,
      XmNwidth,           9 * Screen_Width_Get (sca_p) / 10,
      XmNfractionBase,    800,
XmNy, 25, /* for window managers that are too stupid to put the top border on the screen! */
      NULL);
    XmStringFree (title);

    pwin = XtVaCreateManagedWidget ("PanedWin",
      xmPanedWindowWidgetClass, formdg,
      XmNtopPosition, 0,
      XmNtopAttachment, XmATTACH_FORM,
      XmNbottomPosition, 775,
      XmNbottomAttachment, XmATTACH_POSITION,
      XmNleftPosition, 400,
      XmNleftAttachment, XmATTACH_POSITION,
      XmNrightOffset, 0,
      XmNrightAttachment, XmATTACH_FORM,
      NULL);

    tform = XtVaCreateManagedWidget ("TForm",
      xmFormWidgetClass, pwin,
      XmNpositionIndex, 0,
#ifdef _CYGWIN_
      XmNpaneMinimum, 450,
#else
      XmNpaneMinimum, 150,
#endif
      XmNpaneMaximum, 450,
      NULL);

    dwpform = XtVaCreateManagedWidget ("DWPaneForm",
      xmFormWidgetClass, pwin,
      XmNpositionIndex, 1,
      XmNpaneMinimum, 500,
      XmNpaneMaximum, 1000,
      XmNfractionBase, 800,
      NULL);

/*
sca_p = SynAppR_ScrnAtrb_Get (&GSynAppR);
*/
disp = Screen_TDisplay_Get (sca_p);
gc = XCreateGC (disp, Screen_RootWin_Get (sca_p), 0, NULL);
XSetBackground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
  SAR_CLRI_WHITE));
XSetFillStyle (disp, gc, FillSolid);
XSetLineAttributes (disp, gc, 1, LineSolid, CapButt, JoinMiter);
XSetFont (disp, gc, XLoadFont (disp, "-*-courier-bold-r-normal-*-18-*-75-75-*-*-iso8859-1"));

    for (i=0; i<2; i++)
      {
      title = XmStringCreateLocalized (i==0?"PostTransform Conditions":"PostTransform Tests");
      lbl[i] = XtVaCreateManagedWidget (i==0?"Lbl1":"Lbl2",
        xmLabelWidgetClass, i == 0 ? formdg : tform,
        XmNlabelString, title,
        XmNtopPosition, 0,
        XmNtopAttachment, XmATTACH_FORM,
        XmNleftOffset, 0,
        XmNleftAttachment, XmATTACH_FORM,
        NULL);
      XmStringFree (title);

#ifdef _CYGHAT_
      box[i] = XtVaCreateManagedWidget ("box", xmRowColumnWidgetClass, i == 0 ? formdg : tform,
        XmNleftAttachment,   XmATTACH_FORM,
        XmNrightAttachment,  XmATTACH_FORM,
        XmNbottomAttachment, i == 0 ? XmATTACH_POSITION : XmATTACH_FORM,
        XmNbottomPosition,   775,
        XmNorientation,      XmHORIZONTAL,
        XmNheight,           1,
        NULL);
#endif

      twin[i] = XtVaCreateManagedWidget (i==0?"Win1":"Win2",
        xmScrolledWindowWidgetClass, i == 0 ? formdg : tform,
#ifdef _CYGWIN_
        XmNscrollingPolicy, XmAUTOMATIC,
        XmNscrollBarDisplayPolicy, XmAS_NEEDED,
#endif
        XmNtopOffset, 0,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, lbl[i],
#ifdef _CYGHAT_
        XmNbottomAttachment, XmATTACH_WIDGET,
        XmNbottomWidget,     box[i],
#else
        i == 0 ? XmNbottomPosition : XmNbottomOffset, i == 0 ? 775 : 0,
        XmNbottomAttachment, i == 0 ? XmATTACH_POSITION : XmATTACH_FORM,
#endif
        XmNleftOffset, 0,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightOffset, 0,
        XmNrightAttachment, i == 0 ? XmATTACH_WIDGET : XmATTACH_FORM,
        XmNrightWidget, pwin,
        NULL);

/*
      text[i] = XmCreateScrolledText (twin[i], i==0?"PTCond":"PTTest", NULL, 0);
*/
#ifdef _CYGHAT_
ac=0;
XtSetArg(al[ac],
        XmNscrollVertical, True); ac++;
XtSetArg(al[ac],
        XmNeditMode, XmMULTI_LINE_EDIT); ac++;
XtSetArg(al[ac],
        XmNeditable, False); ac++;
XtSetArg(al[ac],
        XmNautoShowCursorPosition, False); ac++;
XtSetArg(al[ac],
        XmNcursorPositionVisible, False); ac++;
XtSetArg(al[ac],
        XmNvalue, text_buf[i]); ac++;
XtSetArg(al[ac],
        XmNmarginHeight, 0); ac++;
XtSetArg(al[ac],
        XmNmarginWidth, 0); ac++;
/* set ridiculously large dimensions to get around Motif's flaky insistence on displaying a single line despite attachments! */
XtSetArg(al[ac],
        XmNheight, 2000); ac++;
XtSetArg(al[ac],
        XmNwidth, 2000); ac++;
/****************/
XtSetArg(al[ac],
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE)); ac++;
XtSetArg(al[ac],
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK)); ac++;
XtSetArg(al[ac],
        XmNtopOffset, 0); ac++;
XtSetArg(al[ac],
        XmNtopAttachment, XmATTACH_FORM); ac++;
XtSetArg(al[ac],
        XmNbottomOffset, 0); ac++;
XtSetArg(al[ac],
        XmNbottomAttachment, XmATTACH_FORM); ac++;
XtSetArg(al[ac],
        XmNleftOffset, 0); ac++;
XtSetArg(al[ac],
        XmNleftAttachment, XmATTACH_FORM); ac++;
XtSetArg(al[ac],
        XmNrightOffset, 0); ac++;
XtSetArg(al[ac],
        XmNrightAttachment, XmATTACH_FORM); ac++;
      text[i] = XmCreateText (twin[i], i==0?"PTCond":"PTTest", al, ac);
#else
      text[i] = XmCreateText (twin[i], i==0?"PTCond":"PTTest", NULL, 0);
      XtVaSetValues (text[i],
        XmNscrollingPolicy, XmAPPLICATION_DEFINED,
        XmNscrollBarDisplayPolicy, XmAS_NEEDED,
        XmNscrollVertical, True,
        XmNeditMode, XmMULTI_LINE_EDIT,
        XmNeditable, False,
        XmNautoShowCursorPosition, False,
        XmNcursorPositionVisible, False,
        XmNvalue, text_buf[i],
        XmNmarginHeight, 0,
        XmNmarginWidth, 0,
/* set ridiculously large dimensions to get around Motif's flaky insistence on displaying a single line despite attachments! */
        XmNheight, 2000,
        XmNwidth, 2000,
/****************/
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        NULL);
#endif

if (!callbacks_enabled)
{
      XtAddCallback (text[i], XmNmotionVerifyCallback,
        ScrlText_CB, (XtPointer) i);
/*
printf("Callbacks enabled 1\n");
*/
callbacks_enabled=i==1;
}
/*
else printf("attempt to enable nonexistent callbacks (1)\n");
*/

      dwform[i] = XtVaCreateManagedWidget ("TForm",
        xmFormWidgetClass, dwpform,
        XmNtopPosition, 350 * i,
        XmNtopAttachment, XmATTACH_POSITION,
        XmNbottomPosition, 350 * (i + 1),
        XmNbottomAttachment, XmATTACH_POSITION,
        XmNleftOffset, 0,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightOffset, 0,
        XmNrightAttachment, XmATTACH_FORM,
        NULL);

      title = XmStringCreateLocalized (i==0?"Goal Pattern":"Subgoal Pattern");
      dwlbl[i] = XtVaCreateManagedWidget (i==0?"DWLbl1":"DWLbl2",
        xmLabelWidgetClass, dwform[i],
        XmNlabelString, title,
        XmNtopPosition, 0,
        XmNtopAttachment, XmATTACH_FORM,
        XmNleftOffset, 0,
        XmNleftAttachment, XmATTACH_FORM,
        NULL);
      XmStringFree (title);

      dwin[i] = XtVaCreateManagedWidget ("DrawWin",
        xmScrolledWindowWidgetClass, dwform[i],
        XmNscrollingPolicy,          XmAUTOMATIC,
        XmNscrollBarDisplayPolicy,   XmAS_NEEDED,
        XmNtopOffset, 0,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, dwlbl[i],
        XmNbottomOffset, 0,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftOffset, 0,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightOffset, 0,
        XmNrightAttachment, XmATTACH_FORM,
        NULL);
  
      darea[i] = XtVaCreateManagedWidget ("DrawArea",
        xmDrawingAreaWidgetClass, dwin[i],
        XmNbackground,        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
        XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_BLACK),
        XmNheight,            PDRW_DA_PXMP_HT,
        XmNwidth,             PDRW_DA_PXMP_WD,
/*
        XmNfontList, fl6x10,
*/
        XmNresize, False,
        NULL);

      drawp[i] = XCreatePixmap (disp, RootWindow(disp, DefaultScreen(disp)),
        2 * PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, Screen_Depth_Get (sca_p));

      XtAddCallback (darea[i],
        XmNexposeCallback,
        redraw_gsgpatt, (XtPointer) (drawp + i));

      XtManageChild (lbl[i]);
      XtManageChild (text[i]);
      XtManageChild (twin[i]);
      XtManageChild (darea[i]);
      XtManageChild (dwin[i]);
      XtManageChild (dwlbl[i]);
      XtManageChild (dwform[i]);
      }

    XtManageChild (dwpform);
    XtManageChild (tform);
    XtManageChild (pwin);

    title = XmStringCreateLocalized ("Help");
    helppb = XtVaCreateManagedWidget ("HelpPB",
      xmPushButtonWidgetClass, formdg,
      XmNlabelString, title,
      XmNtopPosition, 775,
      XmNtopAttachment, XmATTACH_POSITION,
      XmNbottomOffset, 0,
      XmNbottomAttachment, XmATTACH_FORM,
      XmNleftOffset, 0,
      XmNleftAttachment, XmATTACH_FORM,
      XmNrightPosition, 50,
      XmNrightAttachment, XmATTACH_POSITION,
      NULL);
    XmStringFree(title);

    XtAddCallback (helppb, XmNactivateCallback, Help_CB, (XtPointer) "postform:Posttransform Test Editor");

    title = XmStringCreateLocalized ("Exit and Update Schema Buffer");
    pb[0] = XtVaCreateManagedWidget ("PB1",
      xmPushButtonWidgetClass, formdg,
      XmNlabelString, title,
      XmNtopPosition, 775,
      XmNtopAttachment, XmATTACH_POSITION,
      XmNbottomOffset, 0,
      XmNbottomAttachment, XmATTACH_FORM,
      XmNleftOffset, 0,
      XmNleftAttachment, XmATTACH_WIDGET,
      XmNleftWidget, helppb,
      XmNrightPosition, 300,
      XmNrightAttachment, XmATTACH_POSITION,
      NULL);
    XmStringFree(title);

    title = XmStringCreateLocalized ("Quit and Cancel");
    pb[1] = XtVaCreateManagedWidget ("PB1",
      xmPushButtonWidgetClass, formdg,
      XmNlabelString, title,
      XmNtopPosition, 775,
      XmNtopAttachment, XmATTACH_POSITION,
      XmNbottomOffset, 0,
      XmNbottomAttachment, XmATTACH_FORM,
      XmNleftOffset, 0,
      XmNleftAttachment, XmATTACH_WIDGET,
      XmNleftWidget, pb[0],
      XmNrightPosition, 550,
      XmNrightAttachment, XmATTACH_POSITION,
      NULL);
    XmStringFree(title);

    title = XmStringCreateLocalized ("Pattern Node Symbols");
    pb[2] = XtVaCreateManagedWidget ("PB2",
      xmPushButtonWidgetClass, formdg,
      XmNlabelString, title,
      XmNtopPosition, 775,
      XmNtopAttachment, XmATTACH_POSITION,
      XmNbottomOffset, 0,
      XmNbottomAttachment, XmATTACH_FORM,
      XmNleftOffset, 0,
      XmNleftAttachment, XmATTACH_WIDGET,
      XmNleftWidget, pb[1],
      XmNrightOffset, 0,
      XmNrightAttachment, XmATTACH_FORM,
      NULL);
    XmStringFree(title);

    XtAddCallback (pb[0], XmNactivateCallback, ExitPT_CB, (XtPointer) TRUE);
    XtAddCallback (pb[1], XmNactivateCallback, ExitPT_CB, (XtPointer) FALSE);
    XtAddCallback (pb[2], XmNactivateCallback, NumPat_CB, (XtPointer) numbered);

    XtManageChild (pb[0]);
    XtManageChild (pb[1]);
    }
  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_WHITE)); /* WHITE initially for pixmaps */
  for (i=0; i<2; i++)
    XFillRectangle (disp, drawp[i], gc, 0, 0, 2 * PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT);
  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_BLACK));

  for (i = 0; i < 100; i++) node_used[i] = FALSE;

/*
  if (new_schn) if (new_schema) ncond = ntest = text_buf[0][0] = text_buf[1][0] = 0;
*/
  if (new_schn && new_schema) ncond = ntest = text_buf[0][0] = text_buf[1][0] = init_ncond = init_ntest = 0;
  else
    {
if (update_init)
{
init_ncond=ncond;
init_ntest=ntest;
}
    condv[0]=testv[0]=NULL;
    ptwrite (ncond, cond, condv, ntest, test, testv, reas, chem, 25, node_used);

    for (i = 0; i < 2; i++)
      {
      tbuf_p[i] = text_buf[i];
      tbuf_p[i][0] = '\0';
      }

    for (i = 0; i < ncond; i++)
      {
      for (j = 0; j < 3; j++)
        {
        strcpy (tbuf_p[0], condv[0][i][j] + (j == 1 ? 0 : 1));
        free (condv[0][i][j]);
        if (j != 1)
          for (k = 2; k < strlen (tbuf_p[0]); k++)
          if (tbuf_p[0][k] == '\t' && tbuf_p[0][k - 1] == '\n')
          {
          strcpy (tbuf_p[0] + k, tbuf_p[0] + k + 1);
          k++;
          }
        tbuf_p[0] += strlen (tbuf_p[0]);
        }

      if (cond_marked[i])
        {
        strcpy (tbuf_p[0] - 1, "\t * * *   M A R K E D   F O R   D E L E T I O N   * * *\n\n");
        tbuf_p[0] += strlen (tbuf_p[0]);
        }
      free (condv[0][i]);
      }
    if (condv[0] != NULL) free (condv[0]);

    strcpy (tbuf_p[0], "<ADD CONDITION (select type below)>\n");
    strcat (tbuf_p[0], "\tELEC: compare electron withdrawing/donating capability\n");
    strcat (tbuf_p[0], "\tMOLEC: reaction has a certain molecularity\n");
    strcat (tbuf_p[0], "\tSBULK: compare steric bulks\n");
    strcat (tbuf_p[0], "\tDIST: node is a certain directed attribute at some distance\n");
    strcat (tbuf_p[0], "\tCONN: determine the existence of a connection\n");
    strcat (tbuf_p[0], "\tRNGSZ: compare a connection against some number of bonds\n");
    strcat (tbuf_p[0], "\tRNGCP: compare two specified rings or connectivities\n");
    strcat (tbuf_p[0], "\tALSMR: an allene or alkyne in a small ring\n");
    strcat (tbuf_p[0], "\tCARSB: compare stability of two carbonium ions\n");
    strcat (tbuf_p[0], "\tLVGRP: compare leaving-group tendencies\n");
    strcat (tbuf_p[0], "\tMGAPT: compare migratory aptitudes\n");
    strcat (tbuf_p[0], "\tATOM: node is a given atom at some distance\n");
    strcat (tbuf_p[0], "\tXCESS: node has more than some number of instances of attribute\n");
    strcat (tbuf_p[0], "\tCSEQ: a group of nodes are all equivalent\n");
    strcat (tbuf_p[0], "\tFGEQ: each instance of attribute is equivalent to all others\n");
    strcat (tbuf_p[0], "\tMORE: goal has more or fewer instances of attribute than subgoal\n");
    strcat (tbuf_p[0], "\tARNOD: node is in an isolated benzene ring\n");
    strcat (tbuf_p[0], "\tARSTD: activation of node to electrophilic aromatic substitution\n");
    strcat (tbuf_p[0], "\tARRAT: compare activation of node against an absolute scale\n");
    strcat (tbuf_p[0], "\tARCET: node is equivalent to every node ''tied'' for activation\n");
    strcat (tbuf_p[0], "\tARTIE: compare number of ''tied'' nodes against a given number\n");
    strcat (tbuf_p[0], "\tBRGHD: <UNIMPLEMENTED> determine whether node is a bridgehead carbon\n");

    for (testnum = 0; testnum < ntest; testnum++)
      {
      for (i = 0; i < 5; i++)
        {
        strcpy (tbuf_p[1], testv[0][testnum][i] + (i == 1 ? 0 : 1));
        free (testv[0][testnum][i]);
        if (i != 1)
          for (j = 2; j < strlen (tbuf_p[1]); j++)
          if (tbuf_p[1][j] == '\t' && tbuf_p[1][j - 1] == '\n')
          {
          strcpy (tbuf_p[1] + j, tbuf_p[1] + j + 1);
          k++;
          }
        tbuf_p[1] += strlen (tbuf_p[1]);
        }

      if (test_marked[testnum])
        {
        strcpy (tbuf_p[1] - 1, "\t * * *   M A R K E D   F O R   D E L E T I O N   * * *\n\n");
        tbuf_p[1] += strlen (tbuf_p[1]);
        }
      free (testv[0][testnum]);
      }
    if (testv[0] != NULL) free (testv[0]);

    strcpy (tbuf_p[1], "<ADD TEST>\n");

    for (i = 0; i < 2; i++)
      XmTextSetString (text[i], text_buf[i]);
    }
  XtManageChild (formdg);
  if (glob_rxlform) XtManageChild (pb[0]);
  else XtUnmanageChild (pb[0]);

  for (i = 0; i < 2; i++)
    draw_pattern (darea[i], drawp[i], schn, i, numbered);

  text_ready = TRUE;
}

void PFQuit_Confirm_CB (Widget w, XtPointer cld, XtPointer cd)
{
  XtUnmanageChild (conf_box);
  if ((Boolean_t) (int) cld)
    {
    XtUnmanageChild (formdg);
    if (ncond != init_ncond)
      {
      ncond = init_ncond;
      cond = (Condition_t *) realloc (cond, ncond * sizeof (Condition_t));
      }
    if (ntest != init_ntest)
      {
      ntest = init_ntest;
      test = (Posttest_t *) realloc (test, ntest * sizeof (Posttest_t));
      reas = (String_t *) realloc (reas, ntest * sizeof (String_t));
      chem = (String_t *) realloc (chem, ntest * sizeof (String_t));
      }
    SchformManage (ntest, FALSE);
    }
}

void ExitPT_CB (Widget w, XtPointer cld, XtPointer cd)
{
  int i, j, k, new_ncond, new_ntest, cond_num_map[100], test_num_map[100], op;
  Boolean_t found;

  if (!(Boolean_t) (int) cld)
    {
    if (postform_modified)
      {
      XtManageChild (conf_box);
      return;
      }
    XtUnmanageChild (formdg);
    SchformManage (ntest, FALSE);
    return;
    }

  XtUnmanageChild (formdg);

  for (i = 0; i < ncond; i++) cond_num_map[i] = i;
  for (i = 0; i < ntest; i++) test_num_map[i] = i;

  for (i = 0, new_ncond = ncond; i < new_ncond;) if (cond_marked[i])
  {
    for (j = i + 1; j < new_ncond; j++)
    {
      cond[j - 1] = cond[j];
      cond_num_map[j - 1] = cond_num_map[j];
      cond_marked[j - 1] = cond_marked[j];
    }
    new_ncond--;
  }
  else i++;
  cond = (Condition_t *) realloc (cond, (ncond = new_ncond) * sizeof (Condition_t));

  for (i = 0, new_ntest = ntest; i < new_ntest;) if (test_marked[i])
  {
    for (j = i + 1; j < new_ntest; j++)
    {
      test[j - 1] = test[j];
      reas[j - 1] = reas[j];
      chem[j - 1] = chem[j];
      test_num_map[j - 1] = test_num_map[j];
      test_marked[j - 1] = test_marked[j];
    }
    new_ntest--;
  }
  else i++;
  test = (Posttest_t *) realloc (test, (ntest = new_ntest) * sizeof (Posttest_t));
  reas = (String_t *) realloc (reas, ntest * sizeof (String_t));
  chem = (String_t *) realloc (chem, ntest * sizeof (String_t));

  for (i=0; i < ntest; i++)
  {
    for (j = 0; j <Post_Length_Get (test + i); j++)
    {
      op = Post_Op_Get (test + i, j);
      if (op < PT_TEST_ADD)
        for (k = 0, found = FALSE; k < ncond && !found; k++)
      {
        if (cond_num_map[k] != k && cond_num_map[k] == op)
        {
          Post_Op_Put (test + i, j, k);
          found = TRUE;
        }
      }
      else if (op < PT_TEST_ADD + 100)
        for (k = 0, found = FALSE; k < ntest && !found; k++)
      {
        if (test_num_map[k] != k && test_num_map[k] == op - PT_TEST_ADD)
        {
          Post_Op_Put (test + i, j, k + PT_TEST_ADD);
          found = TRUE;
        }
      }
    }
  }

  pt_update_tests (curr_schn, ncond, cond, ntest, test, reas, chem);

  SchformManage (ntest, TRUE);
}

void NumPat_CB (Widget w, XtPointer cld, XtPointer cd)
{
  Boolean_t *numbered;
  XmString title;
  int i;

  numbered = (Boolean_t *) cld;
  if (numbered[0])
    {
    if (numbered[1]) numbered[0] = numbered[1] = FALSE;
    else numbered[1] = TRUE;
    }
  else numbered[0] = TRUE;

  title = XmStringCreateLocalized (numbered[0] ? (numbered[1] ? "Pattern Nodes Numbered" : "Numbering By Conditions") :
    "Pattern Node Symbols");
  XtVaSetValues (w,
    XmNlabelString, title);
  XmStringFree (title);

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_WHITE)); /* WHITE initially for pixmaps */
  for (i=0; i<2; i++)
    XFillRectangle (disp, drawp[i], gc, 0, 0, 2 * PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT);
  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_BLACK));

  for (i = 0; i < 2; i++)
    draw_pattern (darea[i], drawp[i], curr_schn - 1, i, numbered);
}

void redraw_gsgpatt (Widget w, XtPointer cld, XtPointer cd)
{
  XmDrawingAreaCallbackStruct *cbs =
    (XmDrawingAreaCallbackStruct *) cd;
Display *local_disp;
GC local_gc;

local_disp=cbs->event->xexpose.display;
local_gc=DefaultGC(local_disp,DefaultScreen(local_disp));
printf("r_g1: w=%d window=%d pixmap=%d\n",w,XtWindow(w),*(Pixmap *)cld);
  XCopyArea (local_disp,
    *(Pixmap *) cld, XtWindow (w), local_gc,
    0, 0, 2 * PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);
printf("r_g2\n");
}

void draw_pattern (Widget wg, Pixmap pm, int schn, int which, Boolean_t *numbered)
{
  React_Record_t *schema;
  Tsd_t *pattsd, *gstsd;
  Xtr_t *txtr;
  Dsp_Molecule_t *dsp;
  int h,w,twh,k,x,y,txy,i;
  Boolean_t rotate,low_h,dont_care;
Display *local_disp;
GC local_gc;

local_disp=XtDisplay(wg);
local_gc=DefaultGC(local_disp,DefaultScreen(local_disp));

printf("dp1\n");
  low_h = dont_care = TRUE;
printf("dp1.2: wg=%p window=%d\n",wg,XtWindow(wg));

  XClearWindow (local_disp, XtWindow (wg));
printf("dp1.25\n");

  if (schn < 0)
    {
printf("dp1.3\n");
    schema=NULL;
    gstsd=NULL;
printf("dp1.4\n");
    }
  else
    {
printf("dp1.5\n");
    schema = React_Schema_Handle_Get (schn);
printf("dp1.6\n");

    gstsd = which == 0 ? React_Goal_Get(schema) : React_Subgoal_Get(schema);
printf("dp1.7\n");
    }

printf("dp2\n");
  if (gstsd == NULL)
    {
    XDrawString (local_disp,XtWindow(wg),local_gc,10,25,"<Undefined>",11);
    XDrawString (local_disp,pm,local_gc,10,25,"<Undefined>",11);
    return;
    }
  pattsd=Tsd_Copy(gstsd);
  for (i = 0; i < Tsd_NumAtoms_Get (pattsd) && low_h; i++) if (numbered[1] || (numbered[0] && node_used[i])
    && Tsd_Atomid_Get (pattsd, i) == 1) low_h = FALSE;

printf("dp3\n");
  if (low_h) Tsd_Strip_H (pattsd, dont_care);
  txtr=Tsd2Xtr(pattsd);

  dsp=Xtr2Dsp(txtr);

printf("dp4\n");
  if (dsp_Shelley(dsp))
    {
    Dsp_Compress (dsp);
/*
    fixdsp (dsp);
*/
    h=dsp->molh;
    w=dsp->molw;
    if (rotate=w<h)
      {
      twh=w;
      w=h;
      h=twh;
      }
    dsp->molh=h;
    dsp->molw=w;
    for (k=0; k<dsp->natms; k++)
      {
      x=dsp->atoms[k].x;
      y=dsp->atoms[k].y;
      if (rotate)
        {
        txy=x;
        x=y;
        y=txy;
        }
      dsp->atoms[k].x=x;
      dsp->atoms[k].y=y;
      }

/*
    if (numbered[0]) for (i = 0; i < dsp->natms; i++) if (node_used[i] || numbered[1])
      {
      sprintf (dsp->atoms[i].sym, "%d", i + 1);
      dsp->atoms[i].isC=FALSE;
      }
*/

printf("dp5\n");
    mdraw2(dsp,pattsd,local_disp,XtWindow(wg),local_gc,25,50,275,200,TRUE,node_used,numbered);

printf("dp6\n");
    mdraw2(dsp,pattsd,local_disp,pm,local_gc,25,50,275,200,FALSE,node_used,numbered);

    free_Molecule (dsp);
    }
  Xtr_Destroy (txtr);
  Tsd_Destroy (pattsd);
printf("dp7\n");
}

void ScrlText_CB (Widget w, XtPointer client_p, XtPointer call_p)
{
  int which;
  XmTextVerifyPtr  cbs;
  char            *textb, *cond_p, condname[16], *tbuf_p;
  int              pos, oldpos, testnum, i, j, k, condnum, pos2, condtype, savepos;

printf("t_r=%d\n",text_ready);
  if (!text_ready) return;

  which = (int) client_p;

  cbs = (XmTextVerifyPtr) call_p;
  oldpos = cbs->currInsert;
  pos = cbs->newInsert;
  textb = text_buf[which];

printf("which=%d textb=\"%s\"\n",which,textb);
  if (strlen(textb)==0) return;

printf("c_e=%d\n",callbacks_enabled);
if (callbacks_enabled)
{
  /* disable callbacks to prevent bizarre intermittent behavior */
  for (i=0; i<2; i++)
    XtRemoveCallback (text[i], XmNmotionVerifyCallback, ScrlText_CB, (XtPointer) i);
/*
printf("Callbacks disabled 1\n");
*/
callbacks_enabled=FALSE;
}
/*
else printf("attempt to disable nonexistent callbacks (1)\n");
*/

  if (which == 0)
    {
    savepos = pos;
    while(pos > 0 && (textb[pos - 1] != '\n' || textb[pos] == '\n' || textb[pos] == '\t')) pos--;
    for (pos2 = condnum = 0; pos2 < pos && condnum < ncond; pos2++)
      if (textb[pos2] == '\n' && textb[pos2 + 1] != '\n' && textb[pos2 + 1] != '\t') condnum++;
    if (condnum < ncond) Post_Cond_Mod (cond + condnum, cond, test, condnum, ncond, ntest, tl, formdg, "");
    else
      {
      condnum = ncond; /* prevent positional glitches */
      for (condtype = 0; pos2 < savepos && condtype < 23; pos2++)
        if (textb[pos2] == '\t') condtype++;
      if (!glob_rxlform || condtype == 0 || condtype > 21)
      {
if (!callbacks_enabled)
{
        /* restore callback here */
        for (i=0; i<2; i++)
          XtAddCallback (text[i], XmNmotionVerifyCallback, ScrlText_CB, (XtPointer) i);
/*
printf("Callbacks enabled 2\n");
*/
callbacks_enabled=TRUE;
}
/*
else printf("attempt to enable enabled callbacks (2)\n");
*/
        XBell (XtDisplay(tl), 10);
        return;
      }
      ncond++;
      cond = (Condition_t *) realloc (cond, ncond * sizeof (Condition_t));
      Post_Cond_Mod (cond + condnum, cond, test, condnum, ncond, ntest, tl, formdg, condtypes[condtype]);
      }
    }
  else
    {
/*
    while(pos > 0 && strncmp (textb + pos, "TEST_", 5) != 0) pos--;
    while(pos < strlen (textb) - 6 && strncmp (textb + pos, "TEST_", 5) != 0) pos++;
    if (strncmp (textb + pos, "TEST_", 5) != 0) return;
    sscanf (textb + pos + 5, "%d", &testnum);
    testnum--;
*/
    while(pos > 0 && (textb[pos - 1] != '\n' || textb[pos] == '\n' || textb[pos] == '\t')) pos--;
    for (pos2 = testnum = 0; pos2 < pos; pos2++)
      if (textb[pos2] == '\n' && textb[pos2 + 1] != '\n' && textb[pos2 + 1] != '\t') testnum++;
    if (testnum < ntest) Post_Test_Mod (test + testnum, cond, test, reas + testnum, chem + testnum, testnum, ncond, ntest,
      tl, formdg, FALSE);
    else
      {
      testnum = ntest; /* prevent positional glitches */
      if (!glob_rxlform)
      {
if (!callbacks_enabled)
{
        /* restore callback here */
        for (i=0; i<2; i++)
          XtAddCallback (text[i], XmNmotionVerifyCallback, ScrlText_CB, (XtPointer) i);
/*
printf("Callbacks enabled 3\n");
*/
callbacks_enabled=TRUE;
}
/*
else printf("attempt to enable enabled callbacks (3)\n");
*/
        XBell (XtDisplay(tl), 10);
        return;
      }
      ntest++;
      test = (Posttest_t *) realloc (test, ntest * sizeof (Posttest_t));
      reas = (String_t *) realloc (reas, ntest * sizeof (String_t));
      chem = (String_t *) realloc (chem, ntest * sizeof (String_t));
      Post_Test_Mod (test + testnum, cond, test, reas + testnum, chem + testnum, testnum, ncond, ntest, tl, formdg, TRUE);
      }
    }
}

void PostForm_Refresh ()
{
  int i;

if (!callbacks_enabled)
{
  /* restore callback here */
  for (i=0; i<2; i++)
    XtAddCallback (text[i], XmNmotionVerifyCallback, ScrlText_CB, (XtPointer) i);
/*
printf("Callbacks enabled 4\n");
*/
callbacks_enabled=TRUE;
}
/*
else printf("attempt to enable enabled callbacks (4)\n");
*/

  XtUnmanageChild (formdg);
  if (cond_add_canceled)
    {
    ncond--;
    cond = (Condition_t *) realloc (cond, ncond * sizeof (Condition_t));
    }
  else if (test_add_canceled)
    {
    ntest--;
    test = (Posttest_t *) realloc (test, ntest * sizeof (Posttest_t));
    String_Destroy (reas[ntest]);
    reas = (String_t *) realloc (reas, ntest * sizeof (String_t));
    String_Destroy (chem[ntest]);
    chem = (String_t *) realloc (chem, ntest * sizeof (String_t));
    }
  Post_Form_Create (tl, -curr_schn - 1);
}

void mdraw2(Dsp_Molecule_t *mol_p,Tsd_t *patt,Display *dsp,/* Drawable */ Window dawin,GC gc,
        int X1,int Y1,int X2,int Y2, Boolean_t iswin, Boolean_t *node_used, Boolean_t *numbered)
{
        Dsp_Atom_t *atom_p;
        Dsp_Bond_t *bond_p;
        char *ftag,atomsym[10],number[4];
        float length;
        float norm_x, norm_y;
        int x0, y0, x1, y1, x2, y2;
        int atm_i, bnd_i, var_num;
        int i,j,k,l,bond_order[600];
        Boolean_t atom_conn[100];
        U32_t s;
        Dimension aw, ah, ax, ay, cw, ch;
	XmString numlbl;
	XFontStruct *font_p;

        for (i=0; i<600; i++) bond_order[i]=0;
        for (i=0; i<100; i++) atom_conn[i]=FALSE;
        for (i=0; i<mol_p->nbnds; i++)
        {
                j=mol_p->bonds[i].latm_p-mol_p->atoms;
                k=mol_p->bonds[i].ratm_p-mol_p->atoms;
                for (l=0; l<MX_NEIGHBORS; l++)
                        if (patt->atoms[j].neighbors[l].id==k)
                {
                        bond_order[i]=patt->atoms[j].neighbors[l].bond;
                        atom_conn[j]=atom_conn[k]=TRUE;
                }
        }

/*  First draw bonds.  */
        bond_p = mol_p->bonds;

        for (bnd_i = 0; bnd_i < mol_p->nbnds; bnd_i++)
        {
                x1 = bond_p->latm_p->x;
                y1 = bond_p->latm_p->y;
                x2 = bond_p->ratm_p->x;
                y2 = bond_p->ratm_p->y;

                if (bond_order[bnd_i]>3) XSetLineAttributes(dsp,gc,1,
                        LineOnOffDash,CapNotLast,JoinMiter);
                else XSetLineAttributes(dsp,gc,1,
                        LineSolid,CapNotLast,JoinMiter);

                switch(bond_order[bnd_i])
                {
                case 0:
                        break;
                case 1:
                case 5:
                        XDrawLine (dsp, dawin, gc, x1+X1, y1+Y1, x2+X1, y2+Y1);
                        break;
                case 2:
                case 6:
                        length = sqrt ((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
                        norm_x = (y2-y1)/length;
                        norm_y = (x2-x1)/length;
                        norm_x *= AppDim_BndDblOff_Get (&GAppDim);
                        norm_y *= AppDim_BndDblOff_Get (&GAppDim);

                        norm_x = (norm_x > 0) ? norm_x + 0.5 : norm_x - 0.5;
                        norm_y = (norm_y > 0) ? norm_y + 0.5 : norm_y - 0.5;
                        x0 = (int) norm_x;
                        y0 = (int) norm_y;
                        XDrawLine (dsp, dawin, gc, x1 + x0 + X1, y1 - y0 + Y1,
                                x2 + x0 + X1, y2 - y0 + Y1);
                        XDrawLine (dsp, dawin, gc, x1 - x0 + X1, y1 + y0 +Y1,
                                x2 - x0 + X1, y2 + y0 + Y1);
                        break;
                case 3:
                        XDrawLine (dsp, dawin, gc, x1 + X1, y1 + Y1,
                                x2 + X1, y2 + Y1);
                        length = sqrt ((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
                        norm_x = (y2-y1)/length;
                        norm_y = (x2-x1)/length;
                        norm_x *= AppDim_BndTplOff_Get (&GAppDim);
                        norm_y *= AppDim_BndTplOff_Get (&GAppDim);

                        norm_x = (norm_x > 0) ? norm_x + 0.5 : norm_x - 0.5;
                        norm_y = (norm_y > 0) ? norm_y + 0.5 : norm_y - 0.5;
                        x0 = (int) norm_x;
                        y0 = (int) norm_y;
                        XDrawLine (dsp, dawin, gc, x1 + x0 + X1, y1 - y0 + Y1,
                                x2 + x0 + X1, y2 - y0 + Y1);
                        XDrawLine (dsp, dawin, gc, x1 - x0 + X1, y1 + y0 + Y1,
                                x2 - x0 + X1, y2 + y0 + Y1);
                        break;
                default:
                        printf("unrecognized bond order: %d\n",
                                bond_order[bnd_i]);
                        break;
                }
                ++bond_p;
        }
        XSetLineAttributes(dsp,gc,1,LineSolid,CapNotLast,JoinMiter);

/*  Then draw atoms.  */
        atom_p = mol_p->atoms;

	font_p = SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_SML);
	XSetFont (dsp, gc, font_p->fid);

        for (atm_i = 0; atm_i < mol_p->natms; atm_i++)
        {
	    if (atom_conn[atm_i])
	    {
		if (atom_p->isC)
		{
			ax = atom_p->x;
			ay = atom_p->y;
			aw = ah = 10;
			XFillArc (dsp, dawin, gc, ax + X1 - 5, ay + Y1 - 5, 10, 10, 0, 360 * 64);
		}
                else
                {
                        strncpy(atomsym,atom_p->sym,4);
                        atomsym[4]=0;
                        if (!strcasecmp (atomsym, "#J")) strcpy (atomsym, "X");
                        else if (atomsym[0] == '$')
                          {
                          sscanf (atomsym + 1, "%d", &var_num);
                          if (var_num % 2) strcpy (atomsym, "R'");
                          else strcpy (atomsym, "R");
                          }

                        aw=14*strlen(atomsym)+11;
                        ah=18;

                        ax = (atom_p->x - (aw >> 1) > 0) ?
                                atom_p->x - (aw >> 1) : 0;
                        ay = (atom_p->y - (ah >> 1) > 0) ?
                                atom_p->y - (ah >> 1) : 0;
                        if (iswin)
                            XClearArea (dsp, dawin, ax + X1, ay + Y1, aw, ah, FALSE);
                        else
                          {
                          XSetForeground (dsp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
                            SAR_CLRI_WHITE));
                          XFillRectangle (dsp, dawin, gc, ax + X1, ay + Y1, aw, ah);
                          XSetForeground (dsp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
                            SAR_CLRI_BLACK));
                          }

                        XDrawString(dsp,dawin,gc,ax+aw/4+X1,ay+3*ah/4+Y1,
                                atomsym,strlen(atomsym));
		}
    		if (numbered[0] && (node_used[atm_i] || numbered[1]))
		{
			sprintf (number, "%d", atm_i + 1);
			numlbl = XmStringCreateLtoR (number, SAR_FONTTAG_ATM_SML);
			XmStringExtent (SynAppR_FontList_Get (&GSynAppR), numlbl, &cw, &ch);
			XDrawImageString (dsp, dawin, gc, ax + X1 + (aw >> 1) + 3, ay + Y1 + (ah >> 1) - 3, number, strlen(number));
			XmStringFree (numlbl);
		}
	    }
            ++atom_p;
        }
}
