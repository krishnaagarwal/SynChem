/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              POSTCOMP.C
*
*    This module provides for the display of posttransform conditions and tests
*    for a schema and its intended replacement.
*
*  Creation Date:
*
*       19-Apr-2002
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

static int            ncond[2] = {0,0}, ntest[2] = {0,0}, curr_schn[2] = {0, 0}, NSch, init_ncond = 0, init_ntest = 0;
static Condition_t   *cond[2];
static Posttest_t    *test[2];
static String_t      *reas[2], *chem[2];
static char        ***condv[2][2], ***testv[2][2], text_buf[2][2][50000];
static Boolean_t      new_view = FALSE, node_used[2][100], text_ready = FALSE, callbacks_enabled = FALSE;
static Widget         tl, formdg, darea[2][2], text[2][2];
static Display *disp;
static GC gc;
static Pixmap drawp[2][2];
static char *condtypes[] =
                {"MENU", "ELEC", "MOLEC", "SBULK", "DIST", "CONN", "RNGSZ", "RNGCP", "ALSMR", "CARSB",
                 "LVGRP", "MGAPT", "ATOM", "XCESS", "CSEQ", "FGEQ", "MORE", "ARNOD",
                 "ARSTD", "ARRAT", "ARCET", "ARTIE", "BRGHD"};

void SchcompManage ();
void pt_update_tests (int, int, Condition_t *, int, Posttest_t *, String_t *, String_t *);

void ExitPC_CB (Widget, XtPointer, XtPointer);
void NumCPat_CB (Widget, XtPointer, XtPointer);
void redraw_gsgpatt (Widget, XtPointer, XtPointer);

void ptread_no_init (int, int *, Condition_t **, int *, Posttest_t **, String_t **, String_t **);
void ptwrite (int, Condition_t *, char ****, int, Posttest_t *, char ****, String_t *, String_t *, int, Boolean_t *);
void mdraw2 (Dsp_Molecule_t *, Tsd_t *, Display *, Window, GC, int, int, int, int, Boolean_t, Boolean_t *, Boolean_t *);
Widget fixed_widget(Widget);

void Post_Comp_Create (Widget top_level, int schn[2])
{
  Boolean_t first, new_schn, new_schema, update_init;
  static Boolean_t numbered[2] = {FALSE, FALSE};
  static Widget lbl[2][2], twin[2][2], pb[2], pwin, dwpform, dwin[2][2], tform, dwform[2], dwlbl[2][2];
  static ScreenAttr_t     *sca_p;
  XmString title, msg_str;
  int i, j, k, testnum, which;
  char *tbuf_p[2][2], *cond_p, condname[16];
#ifdef _CYGHAT_
  Widget box[2][2];
  Arg al[50];
  int ac;
#endif

  tl = top_level;
  NSch = React_NumSchemas_Get ();
  first = curr_schn[0] == 0;

  for (i=0; i<2; i++) curr_schn[i] = schn[i] + 1;
  text_ready = FALSE;

  if (first)
    {
    sca_p = SynAppR_ScrnAtrb_Get (&GSynAppR);

printf("pc1\n");
    formdg = XmCreateFormDialog (top_level, "PTComp", NULL, 0);
    title = XmStringCreateLocalized ("PostTransform Comparison");
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

printf("pc2\n");
    tform = XtVaCreateManagedWidget ("TForm",
      xmFormWidgetClass, pwin,
      XmNpositionIndex, 0,
#ifdef _CYGWIN_
      XmNpaneMinimum, 450,
#else
      XmNpaneMinimum, 150,
#endif
      XmNpaneMaximum, 450,
      XmNfractionBase,    800,
      NULL);

    dwpform = XtVaCreateManagedWidget ("DWPaneForm",
      xmFormWidgetClass, pwin,
      XmNpositionIndex, 1,
      XmNpaneMinimum, 500,
      XmNpaneMaximum, 1000,
      XmNfractionBase, 800,
      NULL);

printf("pc3\n");
disp = Screen_TDisplay_Get (sca_p);
gc = XCreateGC (disp, Screen_RootWin_Get (sca_p), 0, NULL);
XSetBackground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
  SAR_CLRI_WHITE));
XSetFillStyle (disp, gc, FillSolid);
XSetLineAttributes (disp, gc, 1, LineSolid, CapButt, JoinMiter);
XSetFont (disp, gc, XLoadFont (disp, "-*-courier-bold-r-normal-*-18-*-75-75-*-*-iso8859-1"));

for (j=0; j<2; j++)
{
    for (i=0; i<2; i++)
      {
      title = XmStringCreateLocalized (i==0?(j==0?"PostTransform Conditions (current)":"(to be imported)"):
        (j==0?"PostTransform Tests (current)":"(to be imported)"));
      lbl[i][j] = XtVaCreateManagedWidget (i==0?"Lbl1":"Lbl2",
        xmLabelWidgetClass, i == 0 ? formdg : tform,
        XmNlabelString, title,
        XmNtopPosition, 0,
        XmNtopAttachment, XmATTACH_FORM,
        XmNleftOffset, 0,
        XmNleftAttachment, j==0?XmATTACH_FORM:XmATTACH_POSITION,
        XmNleftPosition, 200*(i+1),
        NULL);
      XmStringFree (title);

printf("pc4\n");

#ifdef _CYGHAT_
      box[i][j] = XtVaCreateManagedWidget ("box", xmRowColumnWidgetClass, i == 0 ? formdg : tform,
        XmNleftAttachment,   j==0 ? XmATTACH_FORM : XmATTACH_WIDGET,
        XmNleftWidget,       box[i][0],
        XmNrightAttachment,  i==0 || j==0 ? XmATTACH_POSITION : XmATTACH_FORM,
        XmNrightPosition,    i==0 && j==0 ? 200 : 400,
        XmNbottomAttachment, i == 0 ? XmATTACH_POSITION : XmATTACH_FORM,
        XmNbottomPosition,   775,
        XmNorientation,      XmHORIZONTAL,
        XmNheight,           1,
        NULL);
#endif

      twin[i][j] = XtVaCreateManagedWidget (i==0?"Win1":"Win2",
        xmScrolledWindowWidgetClass, i == 0 ? formdg : tform,
#ifdef _CYGWIN_
        XmNscrollingPolicy, XmAUTOMATIC,
        XmNscrollBarDisplayPolicy, XmAS_NEEDED,
#endif
        XmNtopOffset, 0,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, lbl[i][j],
#ifdef _CYGHAT_
        XmNbottomAttachment, XmATTACH_WIDGET,
        XmNbottomWidget,     box[i][j],
#else
        i == 0 ? XmNbottomPosition : XmNbottomOffset, i == 0 ? 775 : 0,
        XmNbottomAttachment, i == 0 ? XmATTACH_POSITION : XmATTACH_FORM,
#endif
        XmNleftOffset, 0,
        XmNleftAttachment, j==0 ? XmATTACH_FORM : XmATTACH_WIDGET,
        XmNleftWidget, twin[i][0],
        XmNrightOffset, 0,
        XmNrightAttachment, i == 0 || j == 0 ? XmATTACH_POSITION : /* i==0 ? XmATTACH_WIDGET : */ XmATTACH_FORM,
        XmNrightWidget, pwin,
        XmNrightPosition, i==0 && j==0 ? 200 : 400,
        NULL);

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
        XmNvalue, text_buf[i][j]); ac++;
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
        XmNleftAttachment, j==0 ? XmATTACH_FORM : XmATTACH_WIDGET); ac++;
XtSetArg(al[ac],
        XmNleftWidget, twin[i][0]); ac++;
XtSetArg(al[ac],
        XmNrightOffset, 0); ac++;
XtSetArg(al[ac],
        XmNrightAttachment, i == 0 || j == 0 ? XmATTACH_POSITION : /* i == 0 ? XmATTACH_WIDGET : */ XmATTACH_FORM); ac++;
XtSetArg(al[ac],
        XmNrightPosition, i == 0 && j == 0 ? 200 : 400); ac++;
XtSetArg(al[ac],
        XmNrightWidget, pwin); ac++;
      text[i][j] = XmCreateText (twin[i][j], i==0?"PTCond":"PTTest", al, ac);
#else
      text[i][j] = XmCreateText (twin[i][j], i==0?"PTCond":"PTTest", NULL, 0);
      XtVaSetValues (text[i][j],
        XmNscrollingPolicy, XmAPPLICATION_DEFINED,
        XmNscrollBarDisplayPolicy, XmAS_NEEDED,
        XmNscrollVertical, True,
        XmNeditMode, XmMULTI_LINE_EDIT,
        XmNeditable, False,
        XmNautoShowCursorPosition, False,
        XmNcursorPositionVisible, False,
        XmNvalue, text_buf[i][j],
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
printf("pc5\n");

    if (j==0)
    {
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
        XmNfractionBase, 800,
        NULL);
    }

      title = XmStringCreateLocalized (i==0?(j==0?"Goal Pattern (current)":"(to be imported)"):
        (j==0?"Subgoal Pattern (current)":"(to be imported)"));
      dwlbl[i][j] = XtVaCreateManagedWidget (i==0?"DWLbl1":"DWLbl2",
        xmLabelWidgetClass, dwform[i],
        XmNlabelString, title,
        XmNtopPosition, 0,
        XmNtopAttachment, XmATTACH_FORM,
        XmNleftOffset, 0,
        XmNleftAttachment, j==0?XmATTACH_FORM:XmATTACH_POSITION,
        XmNleftPosition, 400,
        NULL);
      XmStringFree (title);

printf("pc6\n");
      dwin[i][j] = XtVaCreateManagedWidget ("DrawWin",
        xmScrolledWindowWidgetClass, dwform[i],
        XmNscrollingPolicy,          XmAUTOMATIC,
        XmNscrollBarDisplayPolicy,   XmAS_NEEDED,
        XmNtopOffset, 0,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, dwlbl[i][j],
        XmNbottomOffset, 0,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftOffset, 0,
        XmNleftAttachment, j==0 ? XmATTACH_FORM : XmATTACH_WIDGET,
        XmNleftWidget, dwin[i][0],
        XmNrightOffset, 0,
        XmNrightAttachment, j==0 ? XmATTACH_POSITION : XmATTACH_FORM,
        XmNrightPosition, 400,
        NULL);
  
      darea[i][j] = XtVaCreateManagedWidget ("DrawArea",
        xmDrawingAreaWidgetClass, dwin[i][j],
        XmNbackground,        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
        XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_BLACK),
        XmNheight,            PDRW_DA_PXMP_HT,
        XmNwidth,             2*PDRW_DA_PXMP_WD,
        XmNresize, False,
        NULL);

printf("pc7\n");
      drawp[i][j] = XCreatePixmap (disp, RootWindow(disp, DefaultScreen(disp)),
        2 * PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, Screen_Depth_Get (sca_p));

      XtAddCallback (darea[i][j],
        XmNexposeCallback,
        redraw_gsgpatt, (XtPointer) (drawp[i] + j));

      XtManageChild (lbl[i][j]);
      XtManageChild (text[i][j]);
      XtManageChild (twin[i][j]);
      XtManageChild (darea[i][j]);
      XtManageChild (dwin[i][j]);
      XtManageChild (dwlbl[i][j]);
      if (j==0) XtManageChild (dwform[i]);
}
      }

printf("pc8\n");
    XtManageChild (dwpform);
    XtManageChild (tform);
    XtManageChild (pwin);

    title = XmStringCreateLocalized ("Quit");
    pb[0] = XtVaCreateManagedWidget ("PB1",
      xmPushButtonWidgetClass, formdg,
      XmNlabelString, title,
      XmNtopPosition, 775,
      XmNtopAttachment, XmATTACH_POSITION,
      XmNbottomOffset, 0,
      XmNbottomAttachment, XmATTACH_FORM,
      XmNleftOffset, 150,
      XmNleftAttachment, XmATTACH_FORM,
      XmNrightPosition, 250,
      XmNrightAttachment, XmATTACH_POSITION,
      NULL);
    XmStringFree(title);

    title = XmStringCreateLocalized ("Pattern Node Symbols");
    pb[1] = XtVaCreateManagedWidget ("PB2",
      xmPushButtonWidgetClass, formdg,
      XmNlabelString, title,
      XmNtopPosition, 775,
      XmNtopAttachment, XmATTACH_POSITION,
      XmNbottomOffset, 0,
      XmNbottomAttachment, XmATTACH_FORM,
      XmNleftOffset, 300,
      XmNleftAttachment, XmATTACH_WIDGET,
      XmNleftWidget, pb[0],
      XmNrightOffset, 150,
      XmNrightAttachment, XmATTACH_FORM,
      NULL);
    XmStringFree(title);

printf("pc9\n");
    XtAddCallback (pb[0], XmNactivateCallback, ExitPC_CB, (XtPointer) NULL);
    XtAddCallback (pb[1], XmNactivateCallback, NumCPat_CB, (XtPointer) numbered);

    XtManageChild (pb[0]);
    XtManageChild (pb[1]);
    }
  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_WHITE)); /* WHITE initially for pixmaps */
  for (i=0; i<2; i++) for (j=0; j<2; j++)
    XFillRectangle (disp, drawp[i][j], gc, 0, 0, 2 * PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT);
  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_BLACK));

printf("pc10\n");
for (which=0; which<2; which++)
{
  for (i = 0; i < 100; i++) node_used[which][i] = FALSE;

    condv[which][0]=testv[which][0]=NULL;
    ptread_no_init (schn[which]+1, ncond+which, cond+which, ntest+which, test+which, reas+which, chem+which);
    ptwrite (ncond[which], cond[which], condv[which], ntest[which], test[which], testv[which], reas[which], chem[which], 25,
      node_used[which]);

    for (i = 0; i < 2; i++)
      {
      tbuf_p[which][i] = text_buf[which][i];
      tbuf_p[which][i][0] = '\0';
      }

printf("pc11\n");
    for (i = 0; i < ncond[which]; i++)
      {
      for (j = 0; j < 3; j++)
        {
        strcpy (tbuf_p[which][0], condv[which][0][i][j] + (j == 1 ? 0 : 1));
        free (condv[which][0][i][j]);
        if (j != 1)
          for (k = 2; k < strlen (tbuf_p[which][0]); k++)
          if (tbuf_p[which][0][k] == '\t' && tbuf_p[which][0][k - 1] == '\n')
          {
          strcpy (tbuf_p[which][0] + k, tbuf_p[which][0] + k + 1);
          k++;
          }
        tbuf_p[which][0] += strlen (tbuf_p[which][0]);
        }

      free (condv[which][0][i]);
      }
    if (condv[which][0] != NULL) free (condv[which][0]);

printf("pc12\n");
    for (testnum = 0; testnum < ntest[which]; testnum++)
      {
      for (i = 0; i < 5; i++)
        {
        strcpy (tbuf_p[which][1], testv[which][0][testnum][i] + (i == 1 ? 0 : 1));
        free (testv[which][0][testnum][i]);
        if (i != 1)
          for (j = 2; j < strlen (tbuf_p[which][1]); j++)
          if (tbuf_p[which][1][j] == '\t' && tbuf_p[which][1][j - 1] == '\n')
          {
          strcpy (tbuf_p[which][1] + j, tbuf_p[which][1] + j + 1);
          k++;
          }
        tbuf_p[which][1] += strlen (tbuf_p[which][1]);
        }

      free (testv[which][0][testnum]);
      }
    if (testv[which][0] != NULL) free (testv[which][0]);

printf("pc13\n");
    for (i = 0; i < 2; i++)
      XmTextSetString (text[i][which], text_buf[which][i]);
}
  XtManageChild (formdg);

printf("pc14\n");
  for (i = 0; i < 2; i++) for (j=0; j<2; j++)
    draw_pattern (fixed_widget(darea[i][j]), drawp[i][j], schn[j], i, numbered);

printf("pc15\n");
  text_ready = TRUE;
}

void ExitPC_CB (Widget w, XtPointer cld, XtPointer cd)
{
  int i, j, k, new_ncond, new_ntest, cond_num_map[100], test_num_map[100], op;
  Boolean_t found;

    XtUnmanageChild (formdg);
    SchcompManage ();
}

void NumCPat_CB (Widget w, XtPointer cld, XtPointer cd)
{
  Boolean_t *numbered;
  XmString title;
  int i, j;

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
  for (i=0; i<2; i++) for (j=0; j<2; j++)
    XFillRectangle (disp, drawp[i][j], gc, 0, 0, 2 * PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT);
  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_BLACK));

  for (i = 0; i < 2; i++) for (j=0; j<2; j++)
    draw_pattern (fixed_widget(darea[i][j]), drawp[i][j], curr_schn[j] - 1, i, numbered);
}

Widget fixed_widget(Widget flaky_widget)
{
/* Necessary because either lesstif or xfree86 is even lamer than X11 and OSF Motif in terms of consistency! */
  Widget parent;

  for (parent=flaky_widget; parent!=NULL && XtWindow(parent)==(Window)NULL; parent=XtParent(parent));
printf("parent=%p window=%d\n",parent,XtWindow(parent));

  return (parent);
}
