/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              SCHFORM.C
*
*    This module provides for the display and editing of a given schema in the
*    reaction library.
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
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <time.h>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Xutil.h>
#include <Xm/Xm.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/MessageB.h>
#include <Xm/Text.h>
#include <Xm/PushB.h>
#include <Xm/ScrolledW.h>
#include <Xm/DrawingA.h>
#include <Xm/MainW.h>
#include <Xm/PanedW.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#include "app_resrc.h"

#include "synhelp.h"

#include "submit_draw.h"
#include "sshot_view.h"

#include "rxnpatt_draw.h"

#ifndef _H_ISAM_
#include "isam.h"
#endif

#ifndef _H_FUNCGROUP_FILE_
#include "funcgroup_file.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_SUBMIT_
#include "submit.h"
#endif

#ifndef _H_TIMER_
#include "timer.h"
#endif

#ifndef _H_TSD_
#include "tsd.h"
#endif

#ifndef _H_TSD2XTR_
#include "tsd2xtr.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_SLING2XTR_
#include "sling2xtr.h"
#endif

#ifndef _H_LOGIN_
#include "login.h"
#endif


#ifndef _H_RXLFORM_
#include "rxlform.h"
#endif

#ifndef _H_SGLSHOT_
#include "sglshot.h"
#endif

#ifndef _H_SCHFORM_
#define _GLOBAL_DEF_
#include "schform.h"
#undef _GLOBAL_DEF_
#endif

#ifndef _H_PERSIST_
#include "persist.h"
#endif

#ifndef _H_EXTERN_
#include "extern.h"
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

#define ISTHERE_CLOSE 128

/*
Boolean_t glob_rxlform = TRUE;
*/
extern Boolean_t postform_modified;

int Post_Test_Count (int, Boolean_t);
void Post_Form_Create (Widget, int);
void Pre_Form_Create (Widget, int, U32_t *, U32_t *);
void isthere_cont (int);

void Sglshot_CB (Widget, XtPointer, XtPointer);
void EYCCalc_CB (Widget, XtPointer, XtPointer);
void ModFlags_CB (Widget, XtPointer, XtPointer);
void TypeMol_CB (Widget, XtPointer, XtPointer);
void RefComm_CB (Widget, XtPointer, XtPointer);
void SFQuit_Confirm_CB (Widget, XtPointer, XtPointer);
void BpTg_CB (Widget, XtPointer, XtPointer);

static time_t now;
static Boolean_t save_exit_flag = TRUE, pretests_modified = FALSE, posttests_modified = FALSE, transform_modified = FALSE,
  eyc_modified = FALSE, typmol_modified = FALSE, refs_modified = FALSE, flags_modified = FALSE, name_modified = FALSE,
  bypass_exit = FALSE, incflag_set = FALSE;
static Widget schhelppb, istherepb[2], left_brace_lbl, bypass_legend_lbl[2], bypass_toggle_pb;

void SchEdit_Create_Form (Widget top_level, XtAppContext schlContext, int schn, int *prev_schn, Boolean_t new_schema_param,
  Boolean_t last_closed_param, Boolean_t from_rxlform)
{
	ScreenAttr_t     *sca_p;
	XmFontList flhv12, flhv18,flco18,flcob18,fl6x10,flhv48;
	XmFontListEntry helv12, helv18,cour18,courb18,fs6x10,helv48;
	static Widget frame, schtext_form, schbutt_form, liblistwin, chaplistwin, schlistwin, textwin, drawwin,
		schform,toppaned,ldummy,cdummy,detailpb,srch_txt,srch_pb,srch_lbl, testwin, pretrans_label, namewin, name_edit;
#ifdef _CYGHAT_
        static Widget testbox, textbox;
Arg al[50];
int ac;
#endif
	XtWidgetGeometry geom;
	int i, j, k, nptests /*, which*/, ttlen;
	char itemstr[500], temp[10], fname[100], ttname[128];
	XmString item, label, labels[41], msg;
	Arg scrolling[2];
        React_Record_t *schema;
        React_Head_t *sch_head;
        React_TextRec_t *text;
        React_TextHead_t *txt_head;
unsigned short tfw,tfh;
	FILE *f;

printf("schform: schn=%d\n",schn);
  bypass_exit = incflag_set = FALSE;

now = time (NULL);
fprintf(stderr,"start of schedit: %s\n",ctime (&now));
  glob_rxlform = from_rxlform;
  tl = top_level;
  schlC = schlContext;
  new_schema_local = new_schema_param; /* schn == (num_schemata = *nsch); */
  schema_open = new_schema_open = FALSE;
  last_closed_local = last_closed_param;
  tsd_altered = FALSE;

  if (prev_schn == NULL)
    {
    prev_schema = schn;
    if (!new_schema_local)
      {
      fprintf (stderr, "SchEdit_Create_Form error: NULL value passed for prev_schn when modifying existing schema\n");
      exit(1);
      }
    }
  else prev_schema = *prev_schn;

  nptests = /* new_schema_local ? 0 : */ Post_Test_Count (schn, FALSE);

/*
  if (new_schema_local) curr_schema = 0;
  else
*/
    {
    schema=React_Schema_Handle_Get(schn);
    sch_head=React_Head_Get(schema);
/*
    curr_schema = schn + 1;
*/
    curr_schema = schn;
    text=React_Text_Get(schema);
    txt_head=React_TxtRec_Head_Get(text);
    }

now = time (NULL);
fprintf(stderr,"schedit split: %s\n",ctime (&now));
  if (topform != (Widget) NULL)
    {
    XtVaGetValues(topform,
      XmNwidth,&tfw,
      XmNheight,&tfh,
      NULL);
now = time (NULL);
fprintf(stderr,"after topform get: %s\n",ctime (&now));

/*
  label = XmStringCreateLocalized (new_schema_local ? "New Schema Entry" : " Schema  Editor ");
*/
    label = XmStringCreateLocalized ("Schema Editor");

    XtVaSetValues (topform /*[which]*/,
      XmNdialogTitle, label,
      NULL);
    XmStringFree (label);
now = time (NULL);
fprintf(stderr,"after topform set: %s\n",ctime (&now));

    if (!glob_rxlform)
      strcpy (itemstr, "View Pretransform Tests");
    else if (new_schema_local && !pretests_modified)
      strcpy (itemstr, "Enter Pretransform Tests");
    else /*if (i==NUM_BUTTONS_PER_ROW) */
      strcpy (itemstr, "Edit Pretransform Tests");

    label = XmStringCreateLocalized (itemstr);
    XtVaSetValues (schpb[NUM_BUTTONS_PER_ROW],
      XmNlabelString, label,
      NULL);
    XmStringFree (label);

/* sprintf (itemstr, "[%02d] PostTransform Tests", nptests); */

    if (new_schema_local && !posttests_modified)
      strcpy (itemstr, "Enter PostTransform Tests");
    else if (nptests == 0) strcpy (itemstr, "No PostTransform Tests");
    else sprintf (itemstr, "[%d] PostTransform Tests", nptests);

    label = XmStringCreateLocalized (itemstr);
    XtVaSetValues (schpb[2 * NUM_BUTTONS_PER_ROW],
      XmNlabelString, label,
      NULL);
    XmStringFree (label);

    if (!glob_rxlform)
      strcpy (itemstr, "View Transform");
    else if (new_schema_local && !transform_modified)
      strcpy (itemstr, "Enter Transform");
    else strcpy (itemstr, "Edit Transform");

    label = XmStringCreateLocalized (itemstr);
    XtVaSetValues (schpb[2],
      XmNlabelString, label,
      NULL);
    XmStringFree (label);

/*
strcpy (temp, "not a null string");
*/
now = time (NULL);
fprintf(stderr,"before remove: %s\n",ctime (&now));
    XtRemoveAllCallbacks (name_edit, XmNmodifyVerifyCallback);
    XtRemoveAllCallbacks (name_edit, XmNvalueChangedCallback);

    XtVaSetValues (name_edit,
      XmNvalue, new_schema_local ? (String) "Enter schema name here" : (String) String_Value_Get (React_TxtRec_Name_Get(text)),
      NULL);

    XtAddCallback (name_edit, XmNmodifyVerifyCallback, SchName_CB,
      (XtPointer) FALSE);
    XtAddCallback (name_edit, XmNvalueChangedCallback, SchName_CB,
      (XtPointer) TRUE);
now = time (NULL);
fprintf(stderr,"after add: %s\n",ctime (&now));

    get_text(schn,new_schema_local);
    get_tests(schn,new_schema_local);

    XtVaGetValues(topform,
      XmNwidth,&tfw,
      XmNheight,&tfh,
      NULL);
now = time (NULL);
fprintf(stderr,"after topform: %s\n",ctime (&now));

printf ("managing topform\n");
    if (!XtIsManaged (topform)) XtManageChild (topform);
printf ("managed topform\n");
now = time (NULL);
fprintf(stderr,"after manage topform: %s\n",ctime (&now));

    XtUnmanageChild (left_brace_lbl);
    XtUnmanageChild (bypass_legend_lbl[0]);
    XtUnmanageChild (bypass_legend_lbl[1]);
    XtUnmanageChild (bypass_toggle_pb);

    if (glob_rxlform)
    {
now = time (NULL);
fprintf(stderr,"before manage (true): %s\n",ctime (&now));
      XtManageChild (schpb[1]);
      XtManageChild (schpb[NUM_BUTTONS_PER_ROW + 2]);
      XtManageChild (schpb[2 * NUM_BUTTONS_PER_ROW + 2]);
      XtManageChild (schpb[3 * NUM_BUTTONS_PER_ROW + 2]);
      XtManageChild (schpb[NUM_BUTTONS - 1]);
      XtUnmanageChild (istherepb[0]);
      XtUnmanageChild (istherepb[1]);
      if (sshot_comp[0] != 0)
      {
        label = XmStringCreateLocalized ("exit directly");
        XtVaSetValues (bypass_legend_lbl[0],
          XmNlabelString, label,
          NULL);
        XmStringFree (label);

        label = XmStringCreateLocalized ("to syn_view");
        XtVaSetValues (bypass_legend_lbl[1],
          XmNlabelString, label,
          NULL);

        XmStringFree (label);
        XtManageChild (left_brace_lbl);
        XtManageChild (bypass_legend_lbl[0]);
        XtManageChild (bypass_legend_lbl[1]);
        XtManageChild (bypass_toggle_pb);
      }
now = time (NULL);
fprintf(stderr,"after manage (true): %s\n",ctime (&now));
    }
    else
    {
now = time (NULL);
fprintf(stderr,"before manage (false): %s\n",ctime (&now));
      XtUnmanageChild (schpb[1]);
      XtUnmanageChild (schpb[NUM_BUTTONS_PER_ROW + 2]);
      XtUnmanageChild (schpb[2 * NUM_BUTTONS_PER_ROW + 2]);
      XtUnmanageChild (schpb[3 * NUM_BUTTONS_PER_ROW + 2]);
      XtUnmanageChild (schpb[NUM_BUTTONS - 1]);
      XtManageChild (istherepb[0]);
      XtManageChild (istherepb[1]);
now = time (NULL);
fprintf(stderr,"after manage (false): %s\n",ctime (&now));
    }

now = time (NULL);
fprintf(stderr,"before get_patts (2): %s\n",ctime (&now));
    get_patts(schn,new_schema_local);

now = time (NULL);
fprintf(stderr,"end of schedit (2): %s\n",ctime (&now));
    return;
    }
now = time (NULL);
fprintf(stderr,"end schedit split: %s\n",ctime (&now));

/* initialization */

  init_templates ();

/*
  which = (int) new_schema_local;
*/
  topform /*[which]*/ = XmCreateFormDialog (top_level, "Schema Edit Form", NULL, 0);

/*
  label = XmStringCreateLocalized (new_schema_local ? "New Schema Entry" : " Schema  Editor ");
*/
  label = XmStringCreateLocalized ("Schema Editor");

  XtVaSetValues (topform /*[which]*/,
    XmNresizePolicy, XmRESIZE_NONE,
    XmNdialogTitle, label,
    XmNwidth,1100,
    XmNheight,900,
    XmNfractionBase,    800,
XmNy, 25, /* for window managers that are too stupid to put the top border on the screen! */
    NULL);
  XmStringFree (label);

  conf_box = XmCreateMessageDialog (top_level, "Confirm", NULL, 0);
  label=XmStringCreateLocalized("Confirm");
  XtVaSetValues (conf_box,
      XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
      XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
      XmNdialogTitle, label,
      NULL);
  XmStringFree(label);

  XtUnmanageChild (XmMessageBoxGetChild (conf_box, XmDIALOG_HELP_BUTTON));

  XtAddCallback (XmMessageBoxGetChild (conf_box, XmDIALOG_OK_BUTTON), XmNactivateCallback, SFQuit_Confirm_CB,
    (XtPointer) (int) TRUE);
  XtAddCallback (XmMessageBoxGetChild (conf_box, XmDIALOG_CANCEL_BUTTON), XmNactivateCallback, SFQuit_Confirm_CB,
    (XtPointer) (int) FALSE);
  XtUnmanageChild (conf_box);

  toppaned = XtVaCreateWidget ("paned", xmPanedWindowWidgetClass, topform /*[which]*/, NULL);

  schbutt_form = XtVaCreateWidget ("SchButtFm",
    xmFormWidgetClass,  topform,
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNallowResize,     True,
    XmNpositionIndex,   2,
    XmNfractionBase,    800,
    NULL);

  schtext_form = XtVaCreateWidget ("SchTextFm",
    xmFormWidgetClass,  toppaned,
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNallowResize,     True,
    XmNpositionIndex,   0,
    XmNpaneMinimum,     300,
    XmNpaneMaximum,     500,
    XmNfractionBase,    800,
    NULL);

  schpatt_form = XtVaCreateWidget ("SchPattFm",
    xmFormWidgetClass,  toppaned,
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNallowResize,     True,
    XmNpositionIndex,   1,
    XmNpaneMinimum,     300,
    XmNpaneMaximum,     500,
    XmNfractionBase,    800,
    NULL);

  helv12 = XmFontListEntryLoad (XtDisplay (topform), "-*-helvetica-medium-r-normal-*-12-*-75-75-*-*-iso8859-1",
    XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);
  helv18 = XmFontListEntryLoad (XtDisplay (topform), "-*-helvetica-bold-r-normal-*-18-*-75-75-*-*-iso8859-1",
    XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);
  helv48 = XmFontListEntryLoad (XtDisplay (topform), "-*-helvetica-bold-r-normal-*-48-*-75-75-*-*-iso8859-1",
    XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);
  courb18 = XmFontListEntryLoad (XtDisplay (topform), "-*-courier-bold-r-normal-*-18-*-75-75-*-*-iso8859-1",
    XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);
  cour18 = XmFontListEntryLoad (XtDisplay (topform), "-*-courier-medium-r-normal-*-18-*-75-75-*-*-iso8859-1",
    XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);
  fs6x10 = XmFontListEntryLoad (XtDisplay (topform), "-*-courier-medium-r-normal-*-14-*-75-75-*-*-iso8859-1",
    XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);
/*
  fs6x10 = XmFontListEntryLoad (XtDisplay (topform[which], "6x10", XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);
*/
  flhv12 = XmFontListAppendEntry (NULL, helv12);
  flhv18 = XmFontListAppendEntry (NULL, helv18);
  flhv48 = XmFontListAppendEntry (NULL, helv48);
  flcob18 = XmFontListAppendEntry (NULL, courb18);
  flco18 = XmFontListAppendEntry (NULL, cour18);
  fl6x10 = XmFontListAppendEntry (NULL, fs6x10);

  sca_p = SynAppR_ScrnAtrb_Get (&GSynAppR);
  disp = Screen_TDisplay_Get (sca_p);
  gc = XCreateGC (disp, Screen_RootWin_Get (sca_p), 0, NULL);
  XSetBackground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_WHITE));
  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_WHITE)); /* WHITE initially for pixmaps */
  XSetFillStyle (disp, gc, FillSolid);
  XSetLineAttributes (disp, gc, 1, LineSolid, CapButt, JoinMiter);
  XSetFont (disp, gc, XLoadFont (disp, "-*-courier-bold-r-normal-*-18-*-75-75-*-*-iso8859-1"));

  drawwin = XtVaCreateManagedWidget ("SchemaTransform",
    xmScrolledWindowWidgetClass, schpatt_form,
    XmNscrollingPolicy,          XmAUTOMATIC,
    XmNscrollBarDisplayPolicy,   XmAS_NEEDED,
    NULL);

  draww = XtVaCreateManagedWidget ("TransformPatterns",
    xmDrawingAreaWidgetClass, drawwin,
    XmNbackground,        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNheight,            PDRW_DA_PXMP_HT,
    XmNwidth,             2 * PDRW_DA_PXMP_WD,
    XmNfontList, fl6x10,
    XmNresize, False,
    NULL);

  drawp = XCreatePixmap (disp, RootWindow(disp, DefaultScreen(disp)),
    2 * PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, Screen_Depth_Get (sca_p));

  XFillRectangle (disp, drawp, gc, 0, 0, 2 * PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT);

  XtAddCallback (draww,
    XmNexposeCallback,
    redraw_schpatt, (XtPointer) &drawp);

  XtVaSetValues (drawwin,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_BLACK));

  label = XmStringCreateLocalized ("Pretransform Tests");

  pretrans_label = XtVaCreateManagedWidget ("PretransLabel",
    xmLabelWidgetClass, topform,
    XmNfontList, flhv18,
    XmNlabelString, label,
    NULL);

  XmStringFree (label);

#ifdef _CYGHAT_
  testbox = XtVaCreateManagedWidget ("testbox",
        xmRowColumnWidgetClass, topform,
        XmNleftAttachment,      XmATTACH_WIDGET,
        XmNleftWidget,          pretrans_label,
        XmNrightAttachment,     XmATTACH_FORM,
        XmNbottomAttachment,    XmATTACH_FORM,
        XmNorientation,         XmHORIZONTAL,
        XmNheight,              1,
        NULL);
#endif

#ifdef _CYGWIN_
  testwin = XtVaCreateWidget ("Pretransform Tests", xmScrolledWindowWidgetClass, topform,
        XmNscrollingPolicy, XmAUTOMATIC,
        XmNscrollBarDisplayPolicy, XmAS_NEEDED,
        NULL);
#else
  testwin = XmCreateScrolledWindow (topform, "Pretransform Tests", NULL, 0);
#endif

  XtVaSetValues (toppaned,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomPosition,   625,
    XmNbottomAttachment, XmATTACH_POSITION,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightPosition,    600,
    XmNrightAttachment,  XmATTACH_POSITION,
    NULL);

  XtVaSetValues (pretrans_label,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomPosition,   30,
    XmNbottomAttachment, XmATTACH_POSITION,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       toppaned,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (testwin,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        pretrans_label,
#ifdef _CYGHAT_
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget,     testbox,
#else
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
#endif
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       toppaned,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  namewin = XtVaCreateManagedWidget ("SchemaName",
    xmScrolledWindowWidgetClass, topform,
    XmNscrollingPolicy,          XmAPPLICATION_DEFINED,
    XmNscrollBarDisplayPolicy,   XmSTATIC,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, toppaned,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_WIDGET,
    XmNrightWidget, testwin,
    XmNbottomPosition, 675,
    XmNbottomAttachment, XmATTACH_POSITION,
    NULL);

  name_edit = XtVaCreateManagedWidget ("NameEdit",
    xmTextWidgetClass, namewin,
    XmNmaxLength, SCHNAME_LEN,
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
          XmNfontList, flcob18,
    XmNresizeWidth, True,
    XmNmaxLength, 500,
    XmNvalue, new_schema_local ? (String) "Enter schema name here" : (String) String_Value_Get (React_TxtRec_Name_Get(text)),
    NULL);

  XtAddCallback (name_edit, XmNmodifyVerifyCallback, SchName_CB,
    (XtPointer) FALSE);
  XtAddCallback (name_edit, XmNvalueChangedCallback, SchName_CB,
    (XtPointer) TRUE);

  XtVaSetValues (topform,
    XmNinitialFocus, name_edit,
    NULL);

  XtVaSetValues (schbutt_form,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
/*
    XmNtopWidget,        name_edit,
*/
    XmNtopWidget,        namewin,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      testwin,
    NULL);

#ifdef _CYGHAT_
  textbox = XtVaCreateManagedWidget ("textbox",
        xmRowColumnWidgetClass, schtext_form,
        XmNleftAttachment,      XmATTACH_FORM,
        XmNrightAttachment,     XmATTACH_FORM,
        XmNbottomAttachment,    XmATTACH_FORM,
        XmNorientation,         XmHORIZONTAL,
        XmNheight,              1,
        NULL);
#endif

#ifdef _CYGWIN_
  textwin = XtVaCreateWidget ("Schema Data", xmScrolledWindowWidgetClass, schtext_form,
        XmNscrollingPolicy, XmAUTOMATIC,
        XmNscrollBarDisplayPolicy, XmAS_NEEDED,
        NULL);
#else
  textwin = XmCreateScrolledWindow (schtext_form, "Schema Data", NULL, 0);
#endif

#ifdef _CYGHAT_
ac=0;
XtSetArg(al[ac],
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR, SAR_CLRI_WHITE)); ac++;
XtSetArg(al[ac],
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR, SAR_CLRI_BLACK)); ac++;
XtSetArg(al[ac],
    XmNfontList, flhv18); ac++;
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
    XmNvalue, schbuf[0]); ac++;
XtSetArg(al[ac],
    XmNmarginHeight, 0); ac++;
XtSetArg(al[ac],
    XmNmarginWidth, 0); ac++;
XtSetArg(al[ac],
    XmNtitle,"Schema Data"); ac++;
XtSetArg(al[ac],
    XmNx,0); ac++;
XtSetArg(al[ac],
    XmNy,0); ac++;
  textw = XmCreateText (textwin, "Schema Data", al, ac);
XtSetArg(al[8],
    XmNvalue, schbuf[1]);  /*ac++;removed by kka 1-23-08, gives segmentation fault on next XmCreateText */
XtSetArg(al[11],
    XmNtitle,"Pretransform Tests"); /*ac++;removed by kka 1-23-08, gives segmentation fault on next XmCreateText */
  testw = XmCreateText (testwin, "Pretransform Tests", al, ac);
#else
  textw = XmCreateText (textwin, "Schema Data", NULL, 0);

  XtVaSetValues(textw,XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNfontList, /*flco18*/ flhv18,
    XmNscrollingPolicy, XmAUTOMATIC,
    XmNscrollBarDisplayPolicy, XmAS_NEEDED,
    XmNscrollVertical, True,
    XmNeditMode, XmMULTI_LINE_EDIT,
    XmNeditable, False,
    XmNautoShowCursorPosition, False,
    XmNcursorPositionVisible, False,
    XmNvalue, schbuf[0],
    XmNmarginHeight, 0,
    XmNmarginWidth, 0,
    XmNtitle,"Schema Data",XmNx,0,XmNy,0,NULL);

/*
  XtAddCallback (textw, XmNmotionVerifyCallback,
    Text_CB, (XtPointer) NULL);
*/
  testw = XmCreateText (testwin, "Pretransform Tests", NULL, 0);
  XtVaSetValues(testw,XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNfontList, /*flco18*/ flhv18,
    XmNscrollingPolicy, XmAUTOMATIC,
    XmNscrollBarDisplayPolicy, XmAS_NEEDED,
    XmNscrollVertical, True,
    XmNeditMode, XmMULTI_LINE_EDIT,
    XmNeditable, False,
    XmNautoShowCursorPosition, False,
    XmNcursorPositionVisible, False,
    XmNvalue, schbuf[1],
    XmNmarginHeight, 0,
    XmNmarginWidth, 0,
    XmNtitle,"Pretransform Tests",XmNx,0,XmNy,0,NULL);
/*
  XtAddCallback (testw, XmNmotionVerifyCallback,
    Text_CB, (XtPointer) NULL);
*/
#endif

  XtVaSetValues (textwin,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
#ifdef _CYGHAT_
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget,     textbox,
#else
    XmNbottomAttachment, XmATTACH_FORM,
#endif
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  for (i=0; i<NUM_BUTTONS; i++)
    {
    if (i==0)
      strcpy(itemstr,"Validate Schema");
    else if (i==NUM_BUTTONS_PER_ROW)
      {
      if (!glob_rxlform) strcpy (itemstr, "View Pretransform Tests");
      else if (new_schema_local && !pretests_modified)
        strcpy (itemstr, "Enter Pretransform Tests");
      else strcpy (itemstr, "Edit Pretransform Tests");
      }
    else if (i==2 * NUM_BUTTONS_PER_ROW)
      {
      if (new_schema_local && !posttests_modified)
        strcpy (itemstr, "Enter PostTransform Tests");
      else if (nptests == 0) strcpy (itemstr, "No PostTransform Tests");
      else sprintf (itemstr, "[%d] PostTransform Tests", nptests);
      }
    else if (i==3 * NUM_BUTTONS_PER_ROW)
      {
      strcpy (itemstr, "H on C: ");
      if (low_h)
        {
        if (dont_care) strcat (itemstr, "none");
        else strcat (itemstr, "asymmetric only");
        }
      else strcat (itemstr, "show all");
      }
/*
    else if (i==1) strcpy (itemstr, "Ease, Yield, Confidence");
    else if (i==NUM_BUTTONS_PER_ROW + 1) strcpy (itemstr, "Flags");
    else if (i==2 * NUM_BUTTONS_PER_ROW + 1) strcpy (itemstr, "Type, Molecularity");
    else if (i==3 * NUM_BUTTONS_PER_ROW + 1) strcpy (itemstr, "Comments/References");
    else if (i==2) strcpy (itemstr, "Edit Transform");
    else if (i==NUM_BUTTONS_PER_ROW + 2) strcpy (itemstr, "Exit and Save");
    else if (i==2 * NUM_BUTTONS_PER_ROW + 2) strcpy (itemstr, "Exit, Save, and Close");
    else if (i==3 * NUM_BUTTONS_PER_ROW + 2) strcpy (itemstr, "Quit and Cancel");
    else sprintf (itemstr, "Button %d", i + 1);
*/
    else if (i==1) strcpy (itemstr, "Ease, Yield, Confidence");
    else if (i==NUM_BUTTONS_PER_ROW + 1) strcpy (itemstr, "Flags");
    else if (i==2 * NUM_BUTTONS_PER_ROW + 1) strcpy (itemstr, "Type, Molecularity");
    else if (i==3 * NUM_BUTTONS_PER_ROW + 1) strcpy (itemstr, "Comments/References");
    else if (i==2)
      {
      if (!glob_rxlform) strcpy (itemstr, "View Transform");
      else if (new_schema_local && !transform_modified)
        strcpy (itemstr, "Enter Transform");
      else strcpy (itemstr, "Edit Transform");
      }
    else if (i==NUM_BUTTONS_PER_ROW + 2) strcpy (itemstr, "Save Buffer to File and Continue");
    else if (i==2 * NUM_BUTTONS_PER_ROW + 2) strcpy (itemstr, "Exit and Save");
    else if (i==3 * NUM_BUTTONS_PER_ROW + 2) strcpy (itemstr, "Quit; Cancel Unsaved Modifications");
    else if (i==NUM_BUTTONS - 1) strcpy (itemstr, "Exit, Save, and Close");
    else sprintf (itemstr, "Button %d", i + 1);

    label = XmStringCreateLocalized (itemstr);

  if (i == 2 * NUM_BUTTONS_PER_ROW + 2)
    schpb[i] = XtVaCreateManagedWidget (itemstr,
      xmPushButtonGadgetClass, schbutt_form,
      XmNfontList, sshot_comp[0] == 0 ? flhv18 : flhv12,
      XmNlabelString, label,
      XmNrecomputeSize, True,
      XmNtopPosition, 410,
      XmNtopAttachment, XmATTACH_POSITION,
      XmNbottomOffset, 0,
      XmNbottomAttachment, XmATTACH_NONE,
      XmNleftPosition, sshot_comp[0] == 0 ? 470 : 575,
      XmNleftAttachment, XmATTACH_POSITION,
      XmNrightOffset, 0,
      XmNrightAttachment, XmATTACH_NONE,
      NULL);

  else if (i == 3 * NUM_BUTTONS_PER_ROW + 2)
    schpb[i] = XtVaCreateManagedWidget (itemstr,
      xmPushButtonGadgetClass, schbutt_form,
      XmNfontList, sshot_comp[0] == 0 ? flhv18 : flhv12,
      XmNlabelString, label,
      XmNrecomputeSize, True,
      XmNtopPosition, i/NUM_BUTTONS_PER_ROW*200+10,
      XmNtopAttachment, XmATTACH_POSITION,
      XmNbottomOffset, 0,
      XmNbottomAttachment, XmATTACH_NONE,
      XmNleftPosition, sshot_comp[0] == 0 ? 470 : 575,
      XmNleftAttachment, XmATTACH_POSITION,
      XmNrightOffset, 0,
      XmNrightAttachment, XmATTACH_NONE,
      NULL);

  else if (i == NUM_BUTTONS - 1)
    schpb[i] = XtVaCreateManagedWidget (itemstr,
      xmPushButtonGadgetClass, schbutt_form,
      XmNfontList, sshot_comp[0] == 0 ? flhv18 : flhv12,
      XmNlabelString, label,
      XmNrecomputeSize, True,
      XmNtopPosition, 410,
      XmNtopAttachment, XmATTACH_POSITION,
      XmNbottomOffset, 0,
      XmNbottomAttachment, XmATTACH_NONE,
      XmNleftPosition, sshot_comp[0] == 0 ? 600 : 660,
      XmNleftAttachment, XmATTACH_POSITION,
      XmNrightOffset, 0,
      XmNrightAttachment, XmATTACH_NONE,
      NULL);

  else
    schpb[i] = XtVaCreateManagedWidget (itemstr,
      xmPushButtonGadgetClass, schbutt_form,
      XmNfontList, flhv18,
      XmNlabelString, label,
      XmNrecomputeSize, True,
      XmNtopPosition, i/NUM_BUTTONS_PER_ROW*200+10,
      XmNtopAttachment, XmATTACH_POSITION,
      XmNbottomOffset, 0,
      XmNbottomAttachment, XmATTACH_NONE,
      XmNleftPosition, i%NUM_BUTTONS_PER_ROW*250+5,
      XmNleftAttachment, XmATTACH_POSITION,
      XmNrightOffset, 0,
      XmNrightAttachment, XmATTACH_NONE,
      NULL);

    XmStringFree (label);

/*
    if (i == NUM_BUTTONS_PER_ROW + 2) XtUnmanageChild (schpb[i]);
*/
    }

  label = XmStringCreateLocalized ("Help");
  schhelppb = XtVaCreateManagedWidget ("HelpPB",
    xmPushButtonGadgetClass, schbutt_form,
    XmNfontList, flhv18,
    XmNlabelString, label,
    XmNrecomputeSize, True,
    XmNtopPosition, 10,
    XmNtopAttachment, XmATTACH_POSITION,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_NONE,
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    NULL);
  XmStringFree (label);

  XtAddCallback (schhelppb, XmNactivateCallback, Help_CB, (XtPointer) "schform:Schema Editor");

  label = XmStringCreateLocalized ("Next \"IsThere\" Match");
  istherepb[0] = XtVaCreateManagedWidget ("IstPB0",
    xmPushButtonGadgetClass, schbutt_form,
    XmNfontList, flhv18,
    XmNlabelString, label,
    XmNrecomputeSize, True,
    XmNtopPosition, 10,
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget, schpb[2 * NUM_BUTTONS_PER_ROW + 2],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget, schpb[2 * NUM_BUTTONS_PER_ROW + 2],
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_NONE,
    NULL);
  XmStringFree (label);

  label = XmStringCreateLocalized ("Exit \"IsThere\" with this schema selected");
  istherepb[1] = XtVaCreateManagedWidget ("IstPB1",
    xmPushButtonGadgetClass, schbutt_form,
    XmNfontList, flhv18,
    XmNlabelString, label,
    XmNrecomputeSize, True,
    XmNtopPosition, 10,
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget, schpb[3 * NUM_BUTTONS_PER_ROW + 2],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNleftWidget, schpb[3 * NUM_BUTTONS_PER_ROW + 2],
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_NONE,
    NULL);
  XmStringFree (label);

  for (i = 0; i < 2; i++)
    {
    strcpy (itemstr, i == 0 ? "save required" : "before validation");
    label = XmStringCreateLocalized (itemstr);

    sch_savelbl[i] = XtVaCreateWidget (itemstr,
      xmLabelWidgetClass, schbutt_form,
      XmNfontList, flhv12,
      XmNlabelString, label,
      XmNtopOffset, 0,
      XmNtopAttachment, i == 0 ? XmATTACH_OPPOSITE_WIDGET : XmATTACH_WIDGET,
      XmNtopWidget, i == 0 ? schpb[0] : sch_savelbl[0],
      XmNbottomOffset, 0,
      XmNbottomAttachment, XmATTACH_NONE,
      XmNleftOffset, 0,
      XmNleftAttachment, XmATTACH_WIDGET,
      XmNleftWidget, schpb[0],
      XmNrightOffset, 0,
      XmNrightAttachment, XmATTACH_NONE,
      NULL);

    XmStringFree (label);
    }

  label = XmStringCreateLocalized ("{");

  left_brace_lbl = XtVaCreateWidget ("left brace",
    xmLabelWidgetClass, schbutt_form,
    XmNfontList, flhv48,
    XmNlabelString, label,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget, schpb[2 * NUM_BUTTONS_PER_ROW + 2],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget, schpb[3 * NUM_BUTTONS_PER_ROW + 2],
    XmNleftPosition, 550,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_NONE,
    NULL);

  XmStringFree (label);

  label = XmStringCreateLocalized ("exit directly");

  bypass_legend_lbl[0] = XtVaCreateWidget ("bypass0",
    xmLabelWidgetClass, schbutt_form,
    XmNfontList, flhv12,
    XmNlabelString, label,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget, left_brace_lbl,
    XmNleftPosition, 475,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);

  XmStringFree (label);

  label = XmStringCreateLocalized ("to syn_view");

  bypass_legend_lbl[1] = XtVaCreateWidget ("bypass1",
    xmLabelWidgetClass, schbutt_form,
    XmNfontList, flhv12,
    XmNlabelString, label,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, bypass_legend_lbl[0],
    XmNleftPosition, 475,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);

  XmStringFree (label);

  label = XmStringCreateLocalized ("(toggle)");
  bypass_toggle_pb = XtVaCreateWidget ("togglePB",
    xmPushButtonGadgetClass, schbutt_form,
    XmNfontList, flhv12,
    XmNlabelString, label,
    XmNrecomputeSize, True,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, bypass_legend_lbl[1],
    XmNleftPosition, 475,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);
  XmStringFree (label);

  XtAddCallback (bypass_toggle_pb, XmNactivateCallback, BpTg_CB, (XtPointer) NULL);

/*
  XtAddCallback (schpb[0], XmNactivateCallback, Sglshot_CB, (XtPointer) NULL);
  XtAddCallback (schpb[1], XmNactivateCallback, EYCCalc_CB, (XtPointer) NULL);
  XtAddCallback (schpb[2], XmNactivateCallback, DrwPatt_CB, (XtPointer) NULL);
  XtAddCallback (schpb[NUM_BUTTONS_PER_ROW], XmNactivateCallback, Pretran_CB, (XtPointer) NULL);
  XtAddCallback (schpb[NUM_BUTTONS_PER_ROW + 1], XmNactivateCallback, ModFlags_CB, (XtPointer) NULL);
  XtAddCallback (schpb[NUM_BUTTONS_PER_ROW + 2], XmNactivateCallback, ExitSch_CB, (XtPointer) 1);
  XtAddCallback (schpb[2 * NUM_BUTTONS_PER_ROW], XmNactivateCallback, Postran_CB, (XtPointer) NULL);
  XtAddCallback (schpb[2 * NUM_BUTTONS_PER_ROW + 1], XmNactivateCallback, TypeMol_CB, (XtPointer) NULL);
  XtAddCallback (schpb[2 * NUM_BUTTONS_PER_ROW + 2], XmNactivateCallback, ExitSch_CB, (XtPointer) 3);
  XtAddCallback (schpb[3 * NUM_BUTTONS_PER_ROW], XmNactivateCallback, HOnC_CB, (XtPointer) NULL);
  XtAddCallback (schpb[3 * NUM_BUTTONS_PER_ROW + 1], XmNactivateCallback, RefComm_CB, (XtPointer) NULL);
  XtAddCallback (schpb[3 * NUM_BUTTONS_PER_ROW + 2], XmNactivateCallback, ExitSch_CB, (XtPointer) 0);
*/
  XtAddCallback (schpb[0], XmNactivateCallback, Sglshot_CB, (XtPointer) NULL);
  XtAddCallback (schpb[1], XmNactivateCallback, EYCCalc_CB, (XtPointer) NULL);
  XtAddCallback (schpb[2], XmNactivateCallback, DrwPatt_CB, (XtPointer) NULL);
  XtAddCallback (schpb[NUM_BUTTONS_PER_ROW], XmNactivateCallback, Pretran_CB, (XtPointer) NULL);
  XtAddCallback (schpb[NUM_BUTTONS_PER_ROW + 1], XmNactivateCallback, ModFlags_CB, (XtPointer) NULL);
  XtAddCallback (schpb[NUM_BUTTONS_PER_ROW + 2], XmNactivateCallback, SaveSch_CB, (XtPointer) NULL);
  XtAddCallback (schpb[2 * NUM_BUTTONS_PER_ROW], XmNactivateCallback, Postran_CB, (XtPointer) NULL);
  XtAddCallback (schpb[2 * NUM_BUTTONS_PER_ROW + 1], XmNactivateCallback, TypeMol_CB, (XtPointer) NULL);
  XtAddCallback (schpb[2 * NUM_BUTTONS_PER_ROW + 2], XmNactivateCallback, ExitSch_CB, (XtPointer) SCHEDIT_SAVE);
  XtAddCallback (schpb[3 * NUM_BUTTONS_PER_ROW], XmNactivateCallback, HOnC_CB, (XtPointer) NULL);
  XtAddCallback (schpb[3 * NUM_BUTTONS_PER_ROW + 1], XmNactivateCallback, RefComm_CB, (XtPointer) NULL);
  XtAddCallback (schpb[3 * NUM_BUTTONS_PER_ROW + 2], XmNactivateCallback, ExitSch_CB, (XtPointer) 0);
  XtAddCallback (schpb[NUM_BUTTONS - 1], XmNactivateCallback, ExitSch_CB, (XtPointer) (SCHEDIT_SAVE | SCHEDIT_CLOSE));

  XtAddCallback (istherepb[0], XmNactivateCallback, ExitSch_CB, (XtPointer) 0);
  XtAddCallback (istherepb[1], XmNactivateCallback, ExitSch_CB, (XtPointer) ISTHERE_CLOSE);

  close_box = XmCreateMessageDialog (top_level, "Confirm", NULL, 0);
  label=XmStringCreateLocalized("Confirm");
  msg=XmStringCreateLocalized("Saving this schema will automatically close the previous one.  Continue?");
  XtVaSetValues (close_box,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNdialogTitle, label,
        XmNmessageString, msg,
        NULL);
  XmStringFree(label);
  XmStringFree(msg);

  XtUnmanageChild (XmMessageBoxGetChild (close_box, XmDIALOG_HELP_BUTTON));

  XtAddCallback (XmMessageBoxGetChild (close_box, XmDIALOG_OK_BUTTON), XmNactivateCallback, ExitSch_CB,
    (XtPointer) (SCHEDIT_SAVE | SCHEDIT_INDIRECT));
  XtAddCallback (XmMessageBoxGetChild (close_box, XmDIALOG_CANCEL_BUTTON), XmNactivateCallback, ExitSch_CB,
    (XtPointer) (SCHEDIT_INDIRECT | SCHEDIT_CANCEL));
  XtUnmanageChild (close_box);

  get_text(schn,new_schema_local);
  get_tests(schn,new_schema_local);

  /* Set schbutt_form so that it can't be resized */
/*
  geom.request_mode = CWHeight;
  XtQueryGeometry (schpb[0], NULL, &geom);

  XtVaSetValues (schbutt_form,
    XmNpaneMinimum,     6*geom.height,
    XmNpaneMaximum,     6*geom.height,
    NULL);
*/

  XtManageChild (name_edit);

  XtManageChild (textw);
  XtManageChild (testw);
  XtManageChild (draww);

  XtManageChild (drawwin);
  XtManageChild (textwin);

  XtManageChild (testwin);

  XtManageChild (pretrans_label);

  XtManageChild (schpatt_form);

  XtManageChild (toppaned);

  XtManageChild (schtext_form);

  XtManageChild (namewin);

  XtManageChild (schbutt_form);

printf ("managing topform (initial)\n");
  if (!XtIsManaged (topform)) XtManageChild (topform/*[which]*/);
printf ("managed topform\n");

  if (glob_rxlform)
  {
    XtManageChild (schpb[1]);
    XtManageChild (schpb[NUM_BUTTONS_PER_ROW + 2]);
    XtManageChild (schpb[2 * NUM_BUTTONS_PER_ROW + 2]);
    XtManageChild (schpb[3 * NUM_BUTTONS_PER_ROW + 2]);
    XtManageChild (schpb[NUM_BUTTONS - 1]);
    XtUnmanageChild (istherepb[0]);
    XtUnmanageChild (istherepb[1]);
    if (sshot_comp[0] != 0)
    {
      XtManageChild (left_brace_lbl);
      XtManageChild (bypass_legend_lbl[0]);
      XtManageChild (bypass_legend_lbl[1]);
      XtManageChild (bypass_toggle_pb);
    }
    else
    {
      XtUnmanageChild (left_brace_lbl);
      XtUnmanageChild (bypass_legend_lbl[0]);
      XtUnmanageChild (bypass_legend_lbl[1]);
      XtUnmanageChild (bypass_toggle_pb);
    }
  }
  else
  {
    XtUnmanageChild (left_brace_lbl);
    XtUnmanageChild (bypass_legend_lbl[0]);
    XtUnmanageChild (bypass_legend_lbl[1]);
    XtUnmanageChild (bypass_toggle_pb);
    XtUnmanageChild (schpb[1]);
    XtUnmanageChild (schpb[NUM_BUTTONS_PER_ROW + 2]);
    XtUnmanageChild (schpb[2 * NUM_BUTTONS_PER_ROW + 2]);
    XtUnmanageChild (schpb[3 * NUM_BUTTONS_PER_ROW + 2]);
    XtUnmanageChild (schpb[NUM_BUTTONS - 1]);
    XtManageChild (istherepb[0]);
    XtManageChild (istherepb[1]);
  }

now = time (NULL);
fprintf(stderr,"before get_patts (1): %s\n",ctime (&now));
  get_patts(schn,new_schema_local);
now = time (NULL);
fprintf(stderr,"end of schedit (1): %s\n",ctime (&now));
}

void get_text(int schnum, Boolean_t new)
{
        React_Record_t *schema;
        React_Head_t *sch_head;
        React_TextRec_t *text;
        React_TextHead_t *txt_head;
	int lib,chap,sch,year,hr,min,sec;
	U32_t
        	j,
        	rtype,
        	credate, moddate, cretime, modtime,
        	num_comm, num_ref;
	char schname[512], crefstr[512], string[25], *sbuf, *sname,
                credate_str[16], moddate_str[16], cretime_str[16], modtime_str[16];
	Boolean_t f1,f2,f3,dummy;

  schema=React_Schema_Handle_Get(schnum);
  sch_head=React_Head_Get(schema);
  text=React_Text_Get(schema);
  txt_head=React_TxtRec_Head_Get(text);

/*
  if (new)
    {
    sname = String_Value_Get (React_TxtRec_Name_Get (text));
    sprintf (schbuf[0], "\n Library  3: %s\t\tChapter %2d: %s\n%s\n Schema %3d: %s\n%s\n",
      libname[2], chap_num, chapname[chap_num - 1], TEXT_RULE, sch_count[2][chap_num] + 1,
      sname[0] == '\0' ? "<NEW SCHEMA>" : sname, TEXT_RULE);
    strcat (schbuf[0], "EASE: %d; YLD: %d; CONF: %d          FLAG: <NONE>;     TYPE: SAO;     MAXUSM: 1\n\n",
      React_Head_Ease_Get(sch_head), React_Head_Yield_Get(sch_head), React_Head_Confidence_Get(sch_head));
    sbuf=schbuf[0]+strlen(schbuf[0]);
    credate=React_TxtHd_Created_Get(txt_head);
    moddate=React_TxtHd_LastMod_Get(txt_head);
    sprintf(sbuf," Created: %s\n",date_calc(credate));
    sbuf=schbuf[0]+strlen(schbuf[0]);
    sprintf(sbuf," Modified: %s\n\n",date_calc(moddate));
    XmTextSetString (textw, schbuf[0]);
    return;
    }
*/

  lib = React_Head_Library_Get (sch_head);
  sch_syntheme = chap = React_Head_SynthemeFG_Get (sch_head);
  sch = React_TxtHd_OrigSchema_Get(txt_head);

  if (new)
    {
/*
    sch = sch_count[2][chap_num] + 1;
    React_TxtHd_OrigSchema_Put(txt_head, sch);
*/
    sprintf (schbuf[0], "\n Library  3: %s\t\tChapter %2d: %s\n%s\n",
      libname[2], chap_num, chapname[chap_num - 1], TEXT_RULE);
    sname = String_Value_Get (React_TxtRec_Name_Get (text));
    sprintf (schname, "Schema %3d: %s", sch, sname[0] == '\0' ? "<NEW SCHEMA - Schema cannot be saved without a name>" : sname);
    name_incomplete = sname[0] == '\0';
    if (name_incomplete)
      {
      exit_enable (FALSE);
/*
      val_enable (FALSE);
*/
      }
    else
      {
      exit_enable (!patterns_incomplete);
/*
      val_enable (!(patterns_incomplete || pretests_incomplete));
*/
      }
    val_enable (FALSE, TRUE);
    }
  else
    {
/* only allow validation on saved schemata - avoid the need to start over after a crash */
    val_enable (!pretests_incomplete, FALSE);
    name_incomplete = patterns_incomplete = FALSE;
    exit_enable (TRUE);
    val_enable (!pretests_incomplete, FALSE);
/*
    sch = React_TxtHd_OrigSchema_Get(txt_head);
*/
    sprintf (schbuf[0], "\n Library %2d: %s\t\tChapter %2d: %s\n%s\n",
      lib, libname[lib - 1], chap, chapname[chap - 1], TEXT_RULE);

    sprintf(schname, "Schema %3d: %s", sch, String_Value_Get (React_TxtRec_Name_Get(text)));
/*
  sbuf = schbuf[0];
*/
    }

  sbuf=schbuf[0]+strlen(schbuf[0]);
  wrap_line (&sbuf, schname, MAX_SCH_TEXT);

/*
  sbuf=schbuf[0]+strlen(schbuf[0]);
*/

  if (React_HeadFlags_Incomplete_Get (sch_head)) incflag_set = (!logged_in || clearance_level > 0);

  sprintf(sbuf,"%s%s%s",
    React_HeadFlags_Incomplete_Get(sch_head) ?
    "\n <SCHEMA IS MARKED AS INCOMPLETE: Validation required for completion>\n" : "",
    new ? " (Schema cannot be validated until after first save - this is a safety precaution)\n" : "", TEXT_RULE);
  sbuf=schbuf[0]+strlen(schbuf[0]);

  credate=React_TxtHd_Created_Get(txt_head);
  moddate=React_TxtHd_LastMod_Get(txt_head);
  num_comm=React_TxtHd_NumComments_Get(txt_head);
  num_ref=React_TxtHd_NumReferences_Get(txt_head);

  sprintf(sbuf," EASE: %d; YLD: %d; CONF: %d          FLAG:%s%s%s",
    React_Head_Ease_Get(sch_head),
    React_Head_Yield_Get(sch_head),
    React_Head_Confidence_Get(sch_head),
    (f1=React_HeadFlags_Protection_Get(sch_head))?" <PROT>":"",
    (f2=React_HeadFlags_Lookahead_Get(sch_head))?" <LOOK>":"",
    (f3=React_HeadFlags_Disabled_Get(sch_head))?" <DISA>":"");
  sbuf=schbuf[0]+strlen(schbuf[0]);

  eyc_incomplete = React_Head_Ease_Get (sch_head) == 0 && React_Head_Yield_Get (sch_head) == 0 &&
    React_Head_Confidence_Get (sch_head) == 0;

  sprintf(sbuf,"%s;     TYPE: ",
    f1 || f2 || f3?"":" <NONE>");
rtype=React_Head_ReactionType_Get(sch_head);
  switch(rtype)
    {
  case REACT_MULT_NO_PREF:
    strcat(sbuf,"MNP");
    break;
  case REACT_MULT_PREF_SING:
    strcat(sbuf,"MPS");
    break;
  case REACT_MULT_PREF_MULT:
    strcat(sbuf,"MPM");
    break;
  case REACT_SING_APPL_ONLY:
    strcat(sbuf,"SAO");
    break;
  case REACT_MAX_APPL_ONLY:
    strcat(sbuf,"MAO");
    break;
    }

  sbuf=schbuf[0]+strlen(schbuf[0]);
  sprintf(sbuf,";     MAXUSM: %d\n%s\n",
    React_Head_MaxNonident_Get(sch_head), eyc_incomplete ? " (Schema cannot be validated with zero initial merit)\n" : "");

/*
  if (credate)
    {
    sbuf=schbuf[0]+strlen(schbuf[0]);
    sprintf(sbuf," Created: %s\n",date_calc(credate));
    if (!moddate) strcat(sbuf,"\n");
    }
  if (moddate)
    {
    modtime = Persist_ModTime (PER_STD, schnum);
    sbuf=schbuf[0]+strlen(schbuf[0]);
    sprintf(sbuf," Modified: %s %s\n\n",date_calc(moddate),time_calc(modtime));
    }
*/

  if (credate)
    {
    sbuf=schbuf[0]+strlen(schbuf[0]);
    date_calc(credate,credate_str,&year,NULL,NULL);
    if (year >= 2000 && !new)
      {
      cretime = Persist_ModTime (PER_STD, Persist_Orig_Rec (PER_STD, prev_schema, &dummy));
      time_calc(cretime,cretime_str,&hr,&min,&sec);
      sprintf(sbuf," Created: %s %s\n",credate_str,cretime_str);
      }
    else sprintf(sbuf," Created: %s\n",credate_str);
    if (!moddate) strcat(sbuf,"\n");
    }

  if (moddate)
    {
    sbuf=schbuf[0]+strlen(schbuf[0]);
    date_calc(moddate,moddate_str,&year,NULL,NULL);
    if (year >= 2000 && !new)
      {
      modtime = Persist_ModTime (PER_STD, prev_schema);
      time_calc(modtime,modtime_str,&hr,&min,&sec);
      sprintf(sbuf," Modified: %s %s\n\n",moddate_str,modtime_str);
      }
    else sprintf(sbuf," Modified: %s\n\n",moddate_str);
    }

  for (j=0; j<num_ref; j++) if (String_Value_Get (React_TxtRec_Reference_Get(text,j))[0] != '\007')
    {
    sprintf(crefstr,"<REF> %s", String_Value_Get (React_TxtRec_Reference_Get(text,j)));
    wrap_line (&sbuf, crefstr, MAX_SCH_TEXT);
    }

  for (j=0; j<num_comm; j++)
    {
    sprintf(crefstr,"<COMM> %s", String_Value_Get (React_TxtRec_Comment_Get(text,j)));
    wrap_line (&sbuf, crefstr, MAX_SCH_TEXT);
    }

  for (j=0; j<num_ref; j++) if (String_Value_Get (React_TxtRec_Reference_Get(text,j))[0] == '\007')
    {
    sprintf(crefstr,"<LIBRARY UPDATE> %s", String_Value_Get (React_TxtRec_Reference_Get(text,j)) + 1);
    wrap_line (&sbuf, crefstr, MAX_SCH_TEXT);
    }

  XmTextSetString (textw, schbuf[0]);
}

void get_tests(int schnum, Boolean_t new)
{
        React_Record_t *schema;
        React_Head_t *sch_head;
        React_TextRec_t *text;
        React_TextHead_t *txt_head;
	char *sbuf;
	int j, k;
	Boolean_t anycl,anycn,anyml,anymn,notall[MX_FUNCGROUPS],notany[MX_FUNCGROUPS],mustall[MX_FUNCGROUPS],mustany[MX_FUNCGROUPS];

/*
  if (new)
    {
    strcpy (schbuf[1], "\n No Pretransform Tests");
    XmTextSetString (testw, schbuf[1]);
    return;
    }
*/

  schema=React_Schema_Handle_Get(schnum);
  sch_head=React_Head_Get(schema);
  text=React_Text_Get(schema);
  txt_head=React_TxtRec_Head_Get(text);

  anycl=React_HeadFlags_CantAll_Get(sch_head);
  anycn=React_HeadFlags_CantAny_Get(sch_head);
  anyml=React_HeadFlags_MustAll_Get(sch_head);
  anymn=React_HeadFlags_MustAny_Get(sch_head);

  if (pretests_incomplete = !anycl && !anycn && !anyml && !anymn)
    {
    strcpy (schbuf[1], "\n No Pretransform Tests\n\n (Schema cannot be validated\n   without tests)");
    XmTextSetString (testw, schbuf[1]);
    val_enable (FALSE, FALSE);
    }
  else val_enable (!(new || name_incomplete || patterns_incomplete || eyc_incomplete), new);

  for (j = 0; j < 4; j++) for (k = 0; k < NUM_TEMPLATES; k++) template_complete[j][k] = template_valid[j][k];

  for (j=0; j<MX_FUNCGROUPS; j++)
    {
    notall[j]=anycl?React_CantAll_Get(schema, j):0;
    for (k = 0; k < NUM_TEMPLATES; k++) if (template[k][j] && !notall[j]) template_complete[3][k] = FALSE;
    notany[j]=anycn?React_CantAny_Get(schema, j):0;
    for (k = 0; k < NUM_TEMPLATES; k++) if (template[k][j] && !notany[j]) template_complete[0][k] = FALSE;
    mustall[j]=anyml?React_MustAll_Get(schema, j):0;
    for (k = 0; k < NUM_TEMPLATES; k++) if (template[k][j] && !mustall[j]) template_complete[2][k] = FALSE;
    mustany[j]=anymn?React_MustAny_Get(schema, j):0;
    for (k = 0; k < NUM_TEMPLATES; k++)
    if (template[k][j] && !mustany[j]) template_complete[1][k] = FALSE;
    }

  sbuf=schbuf[1];

  if (anycl)
    {
    sprintf(sbuf,"\n Can't have all of the following:\n\n");
    sbuf+=strlen(sbuf);
    for (j=1; j<fgend; j++) if (notall[j])
      {
      sprintf(sbuf,"%s%3d: %s\n",j<10?"    ":j<100?"   ":"  ",j,fgname[j]);
      sbuf+=strlen(sbuf);
      }
/*
		strcat(sbuf,"\n");
sbuf+=strlen(sbuf);
*/
    for (j=0; j<NUM_TEMPLATES; j++) if (template_complete[3][j])
      {
      sprintf(sbuf,"\n[%s]\n", tempname[j]);
      sbuf+=strlen(sbuf);
      for (k=1; k<fgend; k++) if (template[j][k])
        {
        sprintf(sbuf,"%s%3d: %s\n",k<10?"    ":k<100?"   ":"  ",k,fgname[k]);
        sbuf+=strlen(sbuf);
        }
      }
    }

  if (anycn)
    {
    if (anycl)
      {
      strcat (sbuf, TEST_RULE);
      sbuf+=strlen(sbuf);
      }
    sprintf(sbuf,"\n Can't have any of the following:\n\n");
    sbuf+=strlen(sbuf);
    for (j=1; j<fgend; j++) if (notany[j])
      {
      sprintf(sbuf,"%s%3d: %s\n",j<10?"    ":j<100?"   ":"  ",j,fgname[j]);
      sbuf+=strlen(sbuf);
      }
/*
		strcat(sbuf,"\n");
sbuf+=strlen(sbuf);
*/
    for (j=0; j<NUM_TEMPLATES; j++) if (template_complete[0][j])
      {
      sprintf(sbuf,"\n[%s]\n", tempname[j]);
      sbuf+=strlen(sbuf);
      for (k=1; k<fgend; k++) if (template[j][k])
        {
        sprintf(sbuf,"%s%3d: %s\n",k<10?"    ":k<100?"   ":"  ",k,fgname[k]);
        sbuf+=strlen(sbuf);
        }
      }
    }

  if (anyml)
    {
    if (anycl || anycn)
      {
      strcat (sbuf, TEST_RULE);
      sbuf+=strlen(sbuf);
      }
    sprintf(sbuf,"\n Must have all of the following:\n\n");
    sbuf+=strlen(sbuf);
    for (j=1; j<fgend; j++) if (mustall[j])
      {
      sprintf(sbuf,"%s%3d: %s\n",j<10?"    ":j<100?"   ":"  ",j,fgname[j]);
      sbuf+=strlen(sbuf);
      }
/*
		strcat(sbuf,"\n");
sbuf+=strlen(sbuf);
*/
    for (j=0; j<NUM_TEMPLATES; j++)
    if (template_complete[2][j])
      {
      sprintf(sbuf,"\n[%s]\n", tempname[j]);
      sbuf+=strlen(sbuf);
      for (k=1; k<fgend; k++) if (template[j][k])
        {
        sprintf(sbuf,"%s%3d: %s\n",k<10?"    ":k<100?"   ":"  ",k,fgname[k]);
        sbuf+=strlen(sbuf);
        }
      }
    }

  if (anymn)
    {
    if (anycl || anycn || anyml)
      {
      strcat (sbuf, TEST_RULE);
      sbuf+=strlen(sbuf);
      }
    sprintf(sbuf,"\n Must have any of the following:\n\n");
    sbuf+=strlen(sbuf);
    for (j=1; j<fgend; j++) if (mustany[j])
      {
      sprintf(sbuf,"%s%3d: %s\n",j<10?"    ":j<100?"   ":"  ",j,fgname[j]);
      sbuf+=strlen(sbuf);
      }
/*
		strcat(sbuf,"\n");
sbuf+=strlen(sbuf);
*/
    for (j=0; j<NUM_TEMPLATES; j++) if (template_complete[1][j])
      {
      sprintf(sbuf,"\n[%s]\n", tempname[j]);
      sbuf+=strlen(sbuf);
      for (k=1; k<fgend; k++) if (template[j][k])
        {
        sprintf(sbuf,"%s%3d: %s\n",k<10?"    ":k<100?"   ":"  ",k,fgname[k]);
        sbuf+=strlen(sbuf);
        }
      }
    }

  XmTextSetString (testw, schbuf[1]);
}

void get_patts(int schnum, Boolean_t new)
{
  React_Record_t *schema;
  React_Head_t *sch_head;
  Tsd_t *goal, *subgoal;
  Xtr_t *txtr;
  Dsp_Molecule_t *dsp;
  char string[256];
  int h, w, i, j, k, x, y, twh, txy, npieces, minx[6], miny[6], maxx[6], maxy[6], goal_x_offset;
  Boolean_t rotate, dots[100];

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_WHITE));

  XClearWindow (disp, XtWindow (draww));
  XFillRectangle (disp, drawp, gc, 0, 0, 2 * PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT);

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_BLACK));

  sprintf(string,"Subgoal TSD");
  XDrawString(disp,XtWindow(draww),gc,25,25,string,strlen(string));
  XDrawString(disp,drawp,gc,25,25,string,strlen(string));

/*
  if (new && (React_Schema_Handle_Get (schnum) == NULL || React_Goal_Get (React_Schema_Handle_Get (schnum)) == NULL))
*/
  if (patterns_incomplete = React_Goal_Get (React_Schema_Handle_Get (schnum)) == NULL)
    {
    sprintf(string,"Undefined");
    XDrawString(disp,XtWindow(draww),gc,50,50,string,strlen(string));
    XDrawString(disp,drawp,gc,50,50,string,strlen(string));

    goal_x_offset = 200;
    x = 165;
    y = 40;

    XDrawLine (disp, XtWindow (draww), gc, x + 4, y - 4, x + 25, y - 4);
    XDrawLine (disp, XtWindow (draww), gc, x + 4, y + 4, x + 25, y + 4);
    XDrawLine (disp, XtWindow (draww), gc, x, y, x + 10, y - 10);
    XDrawLine (disp, XtWindow (draww), gc, x, y, x + 10, y + 10);
    XDrawLine (disp, drawp, gc, x + 4, y - 4, x + 25, y - 4);
    XDrawLine (disp, drawp, gc, x + 4, y + 4, x + 25, y + 4);
    XDrawLine (disp, drawp, gc, x, y, x + 10, y - 10);
    XDrawLine (disp, drawp, gc, x, y, x + 10, y + 10);

    XDrawString(disp,XtWindow(draww),gc,goal_x_offset + 25,50,string,strlen(string));
    XDrawString(disp,drawp,gc,goal_x_offset + 25,50,string,strlen(string));

    sprintf(string,"Goal TSD");
    XDrawString(disp,XtWindow(draww),gc,goal_x_offset,25,string,strlen(string));
    XDrawString(disp,drawp,gc,goal_x_offset,25,string,strlen(string));

    sprintf(string, "(Schema cannot be saved without a fully-defined transform)");
    XDrawString(disp,XtWindow(draww),gc,50,100,string,strlen(string));
    XDrawString(disp,drawp,gc,50,100,string,strlen(string));

    exit_enable (FALSE);
    val_enable (FALSE, new);

    return;
    }

  exit_enable (!name_incomplete);
  val_enable (!(new || name_incomplete || pretests_incomplete || eyc_incomplete), new);

  schema=React_Schema_Handle_Get(schnum);
  sch_head=React_Head_Get(schema);

  for (j=0; j<MX_ROOTS; j++)
    {
    roots[j]=React_Head_RootNode_Get(sch_head, j);
    synthemes[j]=React_Head_RootSyntheme_Get(sch_head, j);
    }

  subgoal=Tsd_Copy(React_Subgoal_Get(schema));

  if (low_h) Tsd_Strip_H (subgoal, dont_care);

  txtr=Tsd2Xtr(subgoal);

  dsp=Xtr2Dsp(txtr);

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
    mdraw(dsp,subgoal,disp,XtWindow(draww),gc,25,50,275,200,
                        NULL,NULL,NULL,TRUE);
    mdraw(dsp,subgoal,disp,drawp,gc,25,50,275,200, NULL,NULL,NULL,FALSE);

    get_dsp_stats (dsp, &npieces, minx, miny, maxx, maxy);

    y = 50 + (miny[0] + maxy[0]) / 2;

    for (i=1; i<npieces; i++)
      {
      x = 25 + (maxx[i] + minx[i+1]) / 2;
      XDrawLine (disp, XtWindow (draww), gc, x - 10, y, x + 10, y);
      XDrawLine (disp, XtWindow (draww), gc, x, y - 10, x, y + 10);
      XDrawLine (disp, drawp, gc, x - 10, y, x + 10, y);
      XDrawLine (disp, drawp, gc, x, y - 10, x, y + 10);
      }

    free_Molecule (dsp);
    }
  Xtr_Destroy (txtr);
  Tsd_Destroy (subgoal);

  goal_x_offset = maxx[0] + 100;
  x = maxx[0] + 65;
/* draw retrosynthetic arrow */
  XDrawLine (disp, XtWindow (draww), gc, x + 4, y - 4, x + 25, y - 4);
  XDrawLine (disp, XtWindow (draww), gc, x + 4, y + 4, x + 25, y + 4);
  XDrawLine (disp, XtWindow (draww), gc, x, y, x + 10, y - 10);
  XDrawLine (disp, XtWindow (draww), gc, x, y, x + 10, y + 10);
  XDrawLine (disp, drawp, gc, x + 4, y - 4, x + 25, y - 4);
  XDrawLine (disp, drawp, gc, x + 4, y + 4, x + 25, y + 4);
  XDrawLine (disp, drawp, gc, x, y, x + 10, y - 10);
  XDrawLine (disp, drawp, gc, x, y, x + 10, y + 10);

  sprintf(string,"Goal TSD (dotted nodes in red: set \"H on C: show all\" to see all dotted H's)");
  XDrawString(disp,XtWindow(draww),gc,goal_x_offset,25,string,strlen(string));
  XDrawString(disp,drawp,gc,goal_x_offset,25,string,strlen(string));

  goal=Tsd_Copy(React_Goal_Get(schema));
  if (low_h) Tsd_Strip_H (goal, dont_care);
for (i=0; i<Tsd_NumAtoms_Get(goal); i++) dots[i]=Tsd_AtomFlags_Dot_Get(goal,i);
  txtr=Tsd2Xtr(goal);
  dsp=Xtr2Dsp(txtr);

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
    mdraw(dsp,goal,disp,XtWindow(draww),gc,goal_x_offset,50,goal_x_offset + 225,200,
                        roots,synthemes,dots,TRUE);
    mdraw(dsp,goal,disp,drawp,gc,goal_x_offset,50,goal_x_offset + 225,200, roots,synthemes,dots,FALSE);

    get_dsp_stats (dsp, &npieces, minx, miny, maxx, maxy);

    y = 50 + (miny[0] + maxy[0]) / 2;

    for (i=1; i<npieces; i++)
      {
      x = goal_x_offset + (maxx[i] + minx[i+1]) / 2;
/* draw plus signs between subgoal pieces */
      XDrawLine (disp, XtWindow (draww), gc, x - 10, y, x + 10, y);
      XDrawLine (disp, XtWindow (draww), gc, x, y - 10, x, y + 10);
      XDrawLine (disp, drawp, gc, x - 10, y, x + 10, y);
      XDrawLine (disp, drawp, gc, x, y - 10, x, y + 10);
      }

    free_Molecule (dsp);
    }
  Xtr_Destroy (txtr);
  Tsd_Destroy (goal);

  XFlush (disp);
}

void SchName_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  String_t schema_name;
  React_Record_t *schema;
  React_TextRec_t *text;
  char *schname;
/*
  Boolean_t new_schema;
*/
  Boolean_t modify;
  static Boolean_t internal_change = FALSE;

  if (internal_change) return;

  modify = (Boolean_t) (int) client_data;
  if (!modify && !new_schema_local) return;

/*
  new_schema = * (Boolean_t *) client_data;

  if (new_schema) return;

  schema=React_Schema_Handle_Get(curr_schema - 1);
  text=React_Text_Get(schema);
*/
  schname = XmTextGetString(widg);
  if (modify)
    {
    if (new_schema_local && schname[0] == '\0')
      {
      internal_change = TRUE;
      XmTextSetString (widg, "Enter schema name here");
      internal_change = FALSE;
      }
    }
  else
    {
    if (strcmp (schname, "Enter schema name here") == 0)
      {
      internal_change = TRUE;
      XmTextSetString (widg, "");
      internal_change = FALSE;
      }
    else if (schname[0] == '\0')
      {
      internal_change = TRUE;
      XmTextSetString (widg, "Enter schema name here");
      internal_change = FALSE;
      }
    XtFree (schname);
    return;
    }
  schema_name = String_Create (schname, 0);
  XtFree (schname);
  schema=React_Schema_Handle_Get(curr_schema);
  text=React_Text_Get(schema);
  String_Destroy (React_TxtRec_Name_Get (text));
  React_TxtRec_Name_Put (text, schema_name);
  name_modified = TRUE;
/*
  get_text (curr_schema - 1, FALSE);
*/
  get_text (curr_schema, new_schema_local);
}

void store_eyc (int e, int y, int c)
{
  React_Record_t *schema;
  React_Head_t *sch_head;

/*
  schema=React_Schema_Handle_Get(curr_schema - 1);
*/
  schema=React_Schema_Handle_Get(curr_schema);
  sch_head=React_Head_Get(schema);
  React_Head_Ease_Put (sch_head, e);
  React_Head_Yield_Put (sch_head, y);
  React_Head_Confidence_Put (sch_head, c);
  eyc_modified = TRUE;

/*
  get_text (curr_schema - 1, curr_schema == 0);
*/
  get_text (curr_schema, new_schema_local);
}

void EYCCalc_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  React_Record_t *schema;
  React_Head_t *sch_head;
  int ease, yield, conf;

/*
  schema=React_Schema_Handle_Get(curr_schema - 1);
*/
  schema=React_Schema_Handle_Get(curr_schema);
  sch_head=React_Head_Get(schema);
  ease = React_Head_Ease_Get (sch_head);
  yield = React_Head_Yield_Get (sch_head);
  conf = React_Head_Confidence_Get (sch_head);
  EYC_Open (ease, yield, conf, store_eyc);
}

void store_flags (Boolean_t p, Boolean_t l, Boolean_t d)
{
  React_Record_t *schema;
  React_Head_t *sch_head;

  schema=React_Schema_Handle_Get(curr_schema);
  sch_head=React_Head_Get(schema);
  React_HeadFlags_Protection_Put(sch_head, p);
  React_HeadFlags_Lookahead_Put(sch_head, l);
  React_HeadFlags_Disabled_Put(sch_head, d);
  flags_modified = TRUE;

  get_text (curr_schema, new_schema_local);
}

void ModFlags_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  React_Record_t *schema;
  React_Head_t *sch_head;
  Boolean_t prot, look, disa;

  schema=React_Schema_Handle_Get(curr_schema);
  sch_head=React_Head_Get(schema);
  prot = React_HeadFlags_Protection_Get (sch_head);
  look = React_HeadFlags_Lookahead_Get (sch_head);
  disa = React_HeadFlags_Disabled_Get (sch_head);
  Flags_Open (prot, look, disa, store_flags);
}

void store_typemol (int t, int m)
{
  React_Record_t *schema;
  React_Head_t *sch_head;

  schema=React_Schema_Handle_Get(curr_schema);
  sch_head=React_Head_Get(schema);
  React_Head_ReactionType_Put(sch_head, t);
  React_Head_MaxNonident_Put(sch_head, m);
  typmol_modified = TRUE;

  get_text (curr_schema, new_schema_local);
}

void TypeMol_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  React_Record_t *schema;
  React_Head_t *sch_head;
  int type, mol;

  schema=React_Schema_Handle_Get(curr_schema);
  sch_head=React_Head_Get(schema);
  type = React_Head_ReactionType_Get(sch_head);
  mol = React_Head_MaxNonident_Get(sch_head);
  TM_Open (type, mol, store_typemol);
}

void store_refcomm (int nr, int nc, String_t *refs, String_t *comms)
{
  React_Record_t *schema;
  React_Head_t *sch_head;
  React_TextRec_t *text;
  React_TextHead_t *txt_head;
  int num_ref, num_comm, i;

  schema=React_Schema_Handle_Get(curr_schema);
  sch_head=React_Head_Get(schema);
  text=React_Text_Get(schema);
  txt_head=React_TxtRec_Head_Get(text);

  num_ref=React_TxtHd_NumReferences_Get(txt_head);
  num_comm=React_TxtHd_NumComments_Get(txt_head);
  for (i = 0; i < num_ref; i++) String_Destroy (text->lit_refs[i]);
  for (i = 0; i < num_comm; i++) String_Destroy (text->comments[i]);
  free (text->lit_refs);
  free (text->comments);
  React_TxtHd_NumReferences_Put(txt_head, nr);
  React_TxtHd_NumComments_Put(txt_head, nc);
  text->lit_refs = refs;
  text->comments = comms;
  refs_modified = TRUE;

  get_text (curr_schema, new_schema_local);
}

void RefComm_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  React_Record_t *schema;
  React_Head_t *sch_head;
  React_TextRec_t *text;
  React_TextHead_t *txt_head;
  int num_ref, num_comm;
  String_t *ref_p, *comm_p;

  schema=React_Schema_Handle_Get(curr_schema);
  sch_head=React_Head_Get(schema);
  text=React_Text_Get(schema);
  txt_head=React_TxtRec_Head_Get(text);

  num_ref=React_TxtHd_NumReferences_Get(txt_head);
  num_comm=React_TxtHd_NumComments_Get(txt_head);
  ref_p = text->lit_refs;
  comm_p = text->comments;

  RC_Open (num_ref, num_comm, ref_p, comm_p, store_refcomm);
}

void Sglshot_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  static SubmitCB_t scbt;
  static Boolean_t first_time = TRUE;
  char buffer[25600];

  if (first_time)
    {
    first_time = FALSE;
    Submit_Created_Put (&scbt, FALSE);
    }

/*
  ValCompEntry_Create (&scbt, tl, &schlC, curr_schema - 1);
*/

  sshot_embedded = FALSE;

  buffer[0] = '\0';
  if (check_dots (curr_schema, buffer, new_schema_local, tsd_altered))
    {
    Persist_Update_Rxn (curr_schema, prev_schema, FALSE, FALSE, "Sglshot_CB");
    schema_open = TRUE;
    }

printf("Calling ValCompEntry_Create for %d\n",curr_schema);
  ValCompEntry_Create (&scbt, tl, &schlC, curr_schema);

  XtManageChild (Submit_FormDlg_Get (&scbt));
}

void Schema_Test_Incomplete_Flag ()
{
  React_Record_t *schema;
  React_Head_t *sch_head;

  schema=React_Schema_Handle_Get(curr_schema);
  sch_head=React_Head_Get(schema);

  if (React_HeadFlags_Incomplete_Get (sch_head) && sshot_embedded)
    {
    React_HeadFlags_Incomplete_Put (sch_head, FALSE);

/* Don't tamper with this condition, or you may end up making the rxnlib out of sync with the persistent index file!
   (The voice of experience!!!) */

    if (curr_schema == React_NumSchemas_Get () && (!logged_in || clearance_level > 0))
      {
      incflag_set = FALSE;

      backup_rxnlib ();
      React_Schema_Write (curr_schema);

      if (new_schema_local)
        {
        Persist_Add_Rxn (curr_schema, FALSE, "Schema_Test_Incomplete_Flag");
        new_schema_open = TRUE;
        }
      else
        Persist_Update_Rxn (curr_schema, prev_schema, FALSE, FALSE, "Schema_Test_Incomplete_Flag");
      new_schema_local = FALSE;
      schema_open = TRUE;
      }
    get_text (curr_schema, new_schema_local);
    }
}

void HOnC_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  XmString label;
  char itemstr[32];

  if (low_h)
    {
    dont_care = !dont_care;
    if (dont_care) low_h = FALSE;
    }
  else low_h = TRUE;

/*
  strcpy (itemstr, "H on C: 0 (0=none/1=low/2=all)");
  if (low_h)
    {
    if (dont_care) itemstr[8] = '0';
    else itemstr[8] = '1';
    }
  else itemstr[8] = '2';
*/
  strcpy (itemstr, "H on C: ");
  if (low_h)
    {
    if (dont_care) strcat (itemstr, "none");
    else strcat (itemstr, "asymmetric only");
    }
  else strcat (itemstr, "show all");

  label = XmStringCreateLocalized (itemstr);

  XtVaSetValues (schpb[3 * NUM_BUTTONS_PER_ROW],
    XmNlabelString, label,
    NULL);

  XmStringFree (label);

/*
  get_patts (curr_schema - 1, new_schema_local);
*/
  get_patts (curr_schema, new_schema_local);
}

void Pretran_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  if (!glob_rxlform) Pretran_Cont ();
  else if (logged_in)
	{
  	if (clearance_level < EDIT_PRETRAN_LEV)
		{
		LoginFail (tl);
		return;
		}
	XtUnmanageChild (topform);
	Pretran_Cont ();
	}
  else if (login_failed)
	{
	LoginFail (tl);
	return;
	}
  else
	{
	XtUnmanageChild (topform);
	Login (tl, Pretran_Cont, topform);
	}
}

void Pretran_Cont ()
{
/*
  Pre_Form_Create (tl, curr_schema - 1, roots, synthemes);
*/
  Pre_Form_Create (tl, curr_schema, roots, synthemes);
}

void Pretran_Update (Boolean_t change)
{
  React_Record_t *schema;
  React_Head_t *sch_head;

  schema=React_Schema_Handle_Get(curr_schema);
  sch_head=React_Head_Get(schema);

  XtManageChild (topform);
/*
  if (change) get_tests(curr_schema - 1, FALSE);
*/
  if (change)
    {
    pretests_modified = TRUE;
    get_tests(curr_schema, new_schema_local);
    if (!new_schema_local)
      {
      React_HeadFlags_Incomplete_Put (sch_head, TRUE);
      incflag_set = TRUE;
      get_text (curr_schema, FALSE);
      val_enable (FALSE, TRUE);
      }
    }
}

void Postran_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  React_Record_t *schema;
  React_Head_t *sch_head;

  schema=React_Schema_Handle_Get(curr_schema);
  sch_head=React_Head_Get(schema);

  XtUnmanageChild (topform);
/*
  Post_Form_Create (tl, curr_schema - 1);
*/
  postform_modified = FALSE;

  Post_Form_Create (tl, curr_schema);
}

void SaveSch_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  if (!last_closed_local && prev_schema != curr_schema)
    {
    save_exit_flag = FALSE;
    XtManageChild (close_box);
    return;
    }

  if (logged_in)
        {
        if (clearance_level < EDIT_TRANSFORM_LEV)
                {
                LoginFail (tl);
                return;
                }
        SaveSch_Cont ();
        }
  else if (login_failed)
        {
        LoginFail (tl);
        return;
        }
  else
        {
        XtUnmanageChild (topform);
        Login (tl, SaveSch_Cont, topform);
        }
}

void SaveSch_Cont ()
{
  int nrefs, hour, minute, now, i;
  React_Record_t *schema;
  React_Head_t *sch_head;
  React_TextRec_t *text;
  React_TextHead_t *txt_head;
  Date_t today;
  String_t modify_string;
  char modification[128], today_str[16];

  if (!XtIsManaged (topform)) XtManageChild (topform);

  today = Persist_Today ();
  now = Persist_Now () / 60;

  schema = React_Schema_Handle_Get (curr_schema);
  sch_head = React_Head_Get(schema);
  text=React_Text_Get(schema);
  txt_head=React_TxtRec_Head_Get(text);

  date_calc (today, today_str, NULL, NULL, NULL);
  hour = now / 60;
  minute = now - 60 * hour;

  if (new_schema_local || new_schema_open)
    {
    if (new_schema_local) schema_open = new_schema_open = TRUE;
    else
    /* update creation date to retain synchronization between date_last_mod and time_last_mod in persist index */
      {
      React_TxtHd_Created_Put(txt_head, today);
      sprintf (modification, "\007Schema created by %s on %s at %02d%02d", UserId (), today_str, hour, minute);
      nrefs = React_TxtHd_NumReferences_Get (txt_head);
      for (i = nrefs - 1; i >= 0 && String_Value_Get (React_TxtRec_Reference_Get (text, i))[0] != '\007'; i--);
      if (i < 0) printf ("Warning: no <LIBRARY UPDATE> record exists for schema %d\n", curr_schema);
      else
        {
        String_Destroy (React_TxtRec_Reference_Get (text, i));
        modify_string = String_Create ((const char *) modification, 0);
        React_TxtRec_Reference_Put (text, i, modify_string);
        }
      }
    }
  else
    {
    React_TxtHd_LastMod_Put(txt_head, today);
    sprintf (modification, "\007Schema modified by %s on %s at %02d%02d", UserId (), today_str, hour, minute);
    if (schema_open)
      {
      nrefs = React_TxtHd_NumReferences_Get (txt_head);
      for (i = nrefs - 1; i >= 0 && String_Value_Get (React_TxtRec_Reference_Get (text, i))[0] != '\007'; i--);
      if (i < 0) printf ("Warning: no <LIBRARY UPDATE> record exists for schema %d\n", curr_schema);
      else
        {
        String_Destroy (React_TxtRec_Reference_Get (text, i));
        modify_string = String_Create ((const char *) modification, 0);
        React_TxtRec_Reference_Put (text, i, modify_string);
        }
      }
    else
      {
      nrefs = React_TxtHd_NumReferences_Get (txt_head) + 1;
      React_TxtHd_NumReferences_Put (txt_head, nrefs);
      text->lit_refs = (String_t *) realloc (text->lit_refs, nrefs * sizeof (String_t));
      modify_string = String_Create ((const char *) modification, 0);
      React_TxtRec_Reference_Put (text, nrefs - 1, modify_string);
      }
    schema_open = TRUE;
    }

  get_text(curr_schema, FALSE);

  backup_rxnlib ();
  React_Schema_Write (curr_schema);

  if (new_schema_local)
    Persist_Add_Rxn (curr_schema, FALSE, "SaveSch_CB");
  else
    Persist_Update_Rxn (curr_schema, prev_schema, FALSE, FALSE, "SaveSch_CB");
  new_schema_local = FALSE;
  val_enable (!(patterns_incomplete || pretests_incomplete || eyc_incomplete), FALSE);
  pretests_modified = posttests_modified = transform_modified = eyc_modified = typmol_modified = refs_modified =
    flags_modified = name_modified = FALSE;
}

void SFQuit_Confirm_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild (conf_box);
  if (!(Boolean_t) (int) client_data) return;
  pretests_modified = posttests_modified = transform_modified = eyc_modified = typmol_modified = refs_modified =
    flags_modified = name_modified = FALSE;
  refresh_schema (FALSE, FALSE, schema_open);
  XtUnmanageChild (topform);
if (sshot_comp[0]!='\0' && !bypass_exit)
{
Persist_Close();
exit(0);
}
}

void ExitSch_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  static int saved_client_data;
  char msg_str[600], *ms, *tms;
  XmString msg;

  if (((int) client_data & SCHEDIT_INDIRECT) == 0) /* direct call via PB */
    {
    saved_client_data = (int) client_data;
    save_exit_flag = TRUE;
    if (!last_closed_local && prev_schema != curr_schema && (saved_client_data & 1) != 0)
      {
/*
      save_exit_flag = TRUE;
*/
      XtManageChild (close_box);
      return;
      }
    }
  else /* indirect call via message box */
    {
    if (!save_exit_flag) saved_client_data = (int) client_data;
    XtUnmanageChild (close_box);
    if (((int) client_data & SCHEDIT_CANCEL) != 0) /* cancel button */ return;
    }
  if ((saved_client_data & SCHEDIT_SAVE) == 0)
    {
/*
    if (!save_exit_flag)
      {
      save_exit_flag = last_closed_local = TRUE;
      SaveSch_CB (widg, (XtPointer) NULL, call_data);
      return;
      }
*/
    if (pretests_modified || posttests_modified || transform_modified || eyc_modified || typmol_modified || refs_modified ||
      flags_modified || name_modified || incflag_set)
      {
      if (pretests_modified || posttests_modified || transform_modified || eyc_modified || typmol_modified || refs_modified ||
        flags_modified || name_modified)
        {
        strcpy (msg_str, "If you quit, you will lose recent ");
        if (pretests_modified || posttests_modified)
          {
          if (pretests_modified)
            {
            strcat (msg_str, "pre");
            if (posttests_modified) strcat (msg_str, "- and post");
            }
          else strcat (msg_str, "post");
          strcat (msg_str, "transform test &&& ");
          }
        if (transform_modified) strcat (msg_str, "transform &&& ");
        if (eyc_modified) strcat (msg_str, "EYC value &&& ");
        if (typmol_modified) strcat (msg_str, "type/molecularity &&& ");
        if (refs_modified) strcat (msg_str, "reference &&& ");
        if (flags_modified) strcat (msg_str, "flag &&& ");
        if (name_modified) strcat (msg_str, "name &&& ");
        if (incflag_set) strcat (msg_str, "validation/completion &&& ");
        strcpy (msg_str + strlen (msg_str) - 4, "modifications.  Continue?");
        for (ms = strstr (msg_str, " &&& "); ms != NULL && (tms = strstr (ms + 5, " &&& ")) != NULL; ms = tms);
        if (ms != NULL) strncpy (ms, " and ", 5);
        while ((ms = strstr (msg_str, " &&& ")) != NULL)
          {
          *ms = ',';
          strcpy (ms + 1, ms + 4);
          }
        }
      else strcpy (msg_str, "If you quit, you will be leaving the schema marked as incomplete.  Continue?");
      msg = XmStringCreateLocalized (msg_str);
      XtVaSetValues (conf_box,
        XmNmessageString, msg,
        NULL);
      XmStringFree (msg);
      XtManageChild (conf_box);
      return;
      }
    if (glob_rxlform)
      {
      refresh_schema (FALSE, FALSE, schema_open);
      XtUnmanageChild (topform);
if (sshot_comp[0]!='\0' && !bypass_exit)
{
Persist_Close();
exit(0);
}
      }
    else
      {
      if ((saved_client_data & ISTHERE_CLOSE) != 0) /* don't unmanage otherwise - then we don't have the overhead of managing! */
        XtUnmanageChild (topform); /* order is important here! */
      else
        {
        XmTextSetString (textw, "\n Please wait while IsThere searches for the next matching schema ...");
        XFlush (XtDisplay (topform));
        }
      isthere_cont (saved_client_data & ISTHERE_CLOSE);
      }
    return;
    }

  must_close_schema = (saved_client_data & SCHEDIT_CLOSE) != 0;

  if (logged_in)
        {
        if (clearance_level < EDIT_TRANSFORM_LEV)
                {
                LoginFail (tl);
                return;
                }
/*
        XtUnmanageChild (topform);
*/
        ExitSch_Cont ();
        }
  else if (login_failed)
        {
        LoginFail (tl);
        return;
        }
  else
        {
        XtUnmanageChild (topform);
        Login (tl, ExitSch_Cont, topform);
        }
}

void ExitSch_Cont ()
{
  if (!save_exit_flag)
    {
    last_closed_local = TRUE;
    SaveSch_Cont ();
    return;
    }
  XtUnmanageChild (topform);
  refresh_schema (TRUE, must_close_schema, schema_open);
  pretests_modified = posttests_modified = transform_modified = eyc_modified = typmol_modified = refs_modified =
    flags_modified = name_modified = FALSE;
if (sshot_comp[0]!='\0' && !bypass_exit)
{
Persist_Close();
exit(0);
}
}

void DrwPatt_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  if (!glob_rxlform) DrwPatt_Cont ();
  else if (logged_in)
	{
  	if (clearance_level < EDIT_TRANSFORM_LEV)
		{
		LoginFail (tl);
		return;
		}
	XtUnmanageChild (topform);
	DrwPatt_Cont ();
	}
  else if (login_failed)
	{
	LoginFail (tl);
	return;
	}
  else
	{
	XtUnmanageChild (topform);
	Login (tl, DrwPatt_Cont, topform);
	}
}

void DrwPatt_Cont ()
{
  static RxnInfoCB_t rxninfo_p[2];

  React_Record_t *schema;
  React_Head_t *sch_head;
  Widget drawtool;
  Tsd_t *gtsd, *stsd;
  Xtr_t *gxtr, *sxtr;
  int j;

/*
  if (curr_schema)
*/
  if (!new_schema_local || React_Goal_Get (React_Schema_Handle_Get (curr_schema)) != NULL)
        {
/*
        schema=React_Schema_Handle_Get(curr_schema - 1);
*/
        schema=React_Schema_Handle_Get(curr_schema);
        sch_head=React_Head_Get(schema);
        for (j=0; j<MX_ROOTS; j++)
                {
                roots[j]=React_Head_RootNode_Get(sch_head, j);
                synthemes[j]=React_Head_RootSyntheme_Get(sch_head, j);
                }
        gtsd=React_Goal_Get(schema);
        stsd=React_Subgoal_Get(schema);
        gxtr=Tsd2Xtr(gtsd);
        sxtr=Tsd2Xtr(stsd);
        drawtool=PDraw_Tool_Create(rxninfo_p,tl,&schlC,
                 gxtr,sxtr,roots,synthemes,sch_syntheme);
        Xtr_Destroy (gxtr);
        Xtr_Destroy (sxtr);
        }
  else drawtool=PDraw_Tool_Create(rxninfo_p,tl,&schlC,
                NULL,NULL,NULL,NULL,sch_syntheme);
}

void redraw_schpatt
  (
  Widget      drawing_a,
  XtPointer   client_data,
  XtPointer   call_data
  )

{
  XmDrawingAreaCallbackStruct *cbs =
    (XmDrawingAreaCallbackStruct *) call_data;

  XCopyArea (cbs->event->xexpose.display,
    *(Pixmap *) client_data, XtWindow (drawing_a), gc,
    0, 0, 2 * PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);
}

void get_dsp_stats (Dsp_Molecule_t *dsp, int *npieces, int *minx, int *miny, int *maxx, int *maxy)
{
  Boolean_t visited[100], change;
  int i, j, temp;

  for (i=0; i<dsp->natms; i++) visited[i]=dsp->atoms[i].adj_info.num_neighbs==0;

  minx[0]=miny[0]=100000;
  maxx[0]=maxy[0]=0;

  for (*npieces=i=0; i<dsp->natms; i++) if (!visited[i])
    {
    ++*npieces;
    minx[*npieces]=miny[*npieces]=100000;
    maxx[*npieces]=maxy[*npieces]=0;
    nbr_dfs (dsp->atoms, i, visited, minx, miny, maxx, maxy, *npieces); 
    }

  for (change=TRUE; change;)
    for (i=1, change=FALSE; i<*npieces; i++)
      for (j=i+1; j<=*npieces; j++) if (minx[i]>minx[j])
    {
    change = TRUE;

    temp=minx[i];
    minx[i]=minx[j];
    minx[j]=temp;

    temp=miny[i];
    miny[i]=miny[j];
    miny[j]=temp;

    temp=maxx[i];
    maxx[i]=maxx[j];
    maxx[j]=temp;

    temp=maxy[i];
    maxy[i]=maxy[j];
    maxy[j]=temp;
    }
}

void nbr_dfs (Dsp_Atom_t *atoms, int ainx, Boolean_t *visit, int *minx, int *miny, int *maxx, int *maxy, int piece) 
{
  int i, ninx, x, y;

  visit[ainx] = TRUE;
  x = atoms[ainx].x;
  y = atoms[ainx].y;

  for (i=0; i<=piece; i+=piece)
    {
    if (minx[i]>x) minx[i]=x;
    if (miny[i]>y) miny[i]=y;
    if (maxx[i]<x) maxx[i]=x;
    if (maxy[i]<y) maxy[i]=y;
    }

  for (i=0; i<atoms[ainx].adj_info.num_neighbs; i++)
    {
    ninx=atoms[ainx].adj_info.adj_atms[i];
    if (!visit[ninx]) nbr_dfs (atoms, ninx, visit, minx, miny, maxx, maxy, piece);
    }
}

Boolean_t Sch_Tsds_Store (Tsd_t *gptsd, Tsd_t *sgptsd, U32_t *roots, U32_t *synthemes, int nroots, char *errmsg)
{
  React_Record_t *schema;
  React_Head_t *sch_head;
  Tsd_t *gtsd, *stsd;
  Boolean_t must_compress, ok;
  char first_char;
  int i, j, k, l, h_count, gnbr_cnt, sgnbr_cnt, old_num_atoms, new_num_atoms, gnbr, sgnbr, next_new_atom, next_gatom, next_sgatom,
    gpsize;
  static int bond_order_x2[] = {0, 2, 4, 6, 8, 3, 3};

  XtManageChild (topform);
  if (gptsd == NULL) return;
  
  errmsg[0] = '\0';

  if (gptsd == NULL || sgptsd == NULL || Tsd_NumAtoms_Get (gptsd) == 0 || Tsd_NumAtoms_Get (sgptsd) != Tsd_NumAtoms_Get (gptsd))
    {
/* Must show error message */
/*
    get_patts (curr_schema - 1, TRUE);
*/
    if (gptsd == NULL || sgptsd == NULL) strcpy (errmsg, "INTERNAL ERROR!  Failed to allocate one or both patterns!");
    else if (Tsd_NumAtoms_Get (gptsd) == 0)
      {
      strcpy (errmsg, "No transform to be stored - quitting");
      return(TRUE);
      }
    else strcpy (errmsg, "INTERNAL ERROR!  Patterns do not map one to one!");
    return (FALSE);
    }

  schema = React_Schema_Handle_Get (curr_schema);
  sch_head = React_Head_Get (schema);

  if (new_schema_local && React_Goal_Get (schema) == NULL)
  {
/*
    React_Schema_Handle_Set (num_schemata);
    schema = React_Schema_Handle_Get (num_schemata);
    React_Head_SynthemeFG_Put (React_Head_Get (schema), chap_num);
*/
    React_Head_SynthemeFG_Put (sch_head, chap_num);
/*
    curr_schema = num_schemata + 1;
*/
  }
/*
  else schema = React_Schema_Handle_Get (curr_schema - 1);

  sch_head = React_Head_Get (schema);
*/

  for (i = 0, ok = FALSE; i < nroots; i++)
    {
    if (disconnected_atom (gptsd, roots[i]))
      {
      sprintf (errmsg, "Root %d (node %d) is a disconnected atom in the goal pattern.", i + 1, roots[i] + 1);
      return (FALSE);
      }
    if (synthemes[i] == React_Head_SynthemeFG_Get (sch_head)) ok = TRUE;
    for (j = 0; j < i; j++) if (nodes_in_same_piece (gptsd, roots[i], roots[j]))
      {
      sprintf (errmsg, "Root %d (node %d) and root %d (node %d) are both in the same piece of the goal pattern.",
        j + 1, roots[j] + 1, i + 1, roots[i] + 1);
      return (FALSE);
      }
    }

  if (!ok)
    {
/* Must show error message */
/*
    get_patts (curr_schema - 1, TRUE);
*/
    sprintf (errmsg, "No root corresponding to chapter syntheme (%d).", React_Head_SynthemeFG_Get (sch_head));
    return (FALSE);
    }

  if (!all_pieces_rooted (gptsd, roots, nroots))
    {
    strcpy (errmsg, "Not every piece in the goal pattern is assigned a root/syntheme pair.");
    return (FALSE);
    }

  for (i = 0; i < Tsd_NumAtoms_Get (gptsd); i++) if (((first_char = * (char *) Atomid2Symbol (Tsd_Atomid_Get (sgptsd, i))) == '$' ||
    first_char == '#') && (disconnected_atom (gptsd, i) || disconnected_atom (sgptsd, i)))
    {
    strcpy (errmsg, "Variables must be connected in both transform patterns.");
    return (FALSE);
    }

  Tsd_MatchCompress_Fix (gptsd,sgptsd,roots);

  old_num_atoms = Tsd_NumAtoms_Get (gptsd);

  for (i = h_count = 0; i < old_num_atoms; i++)
    if (Tsd_Atomid_Get (gptsd, i) == 6)
    {
    for (j = 0, gnbr_cnt = sgnbr_cnt = 1; j < MX_NEIGHBORS; j++)
      {
      if (Tsd_Atom_NeighborId_Get (gptsd, i, j) != TSD_INVALID)
        gnbr_cnt+= bond_order_x2[Tsd_Atom_NeighborBond_Get (gptsd, i, j)];
      if (Tsd_Atom_NeighborId_Get (sgptsd, i, j) != TSD_INVALID)
        sgnbr_cnt+= bond_order_x2[Tsd_Atom_NeighborBond_Get (sgptsd, i, j)];
      }

    gnbr_cnt /= 2;
    sgnbr_cnt /= 2;
    if (gnbr_cnt == 0 || gnbr_cnt > 4) gnbr_cnt = 4;
    if (sgnbr_cnt == 0 || sgnbr_cnt > 4) sgnbr_cnt = 4;
    if (gnbr_cnt < sgnbr_cnt) h_count += 4 - gnbr_cnt;
    else h_count += 4 - sgnbr_cnt;
    }

  new_num_atoms = old_num_atoms + h_count;

  gtsd = Tsd_Create (new_num_atoms);
  stsd = Tsd_Create (new_num_atoms);
/*
if (h_count==0) return (TRUE);
*/

  for (i = next_new_atom = old_num_atoms; i < new_num_atoms; i++)
    {
    Tsd_Atomid_Put (gtsd, i, 1);
    Tsd_Atomid_Put (stsd, i, 1);
    for (j = 0; j < MX_NEIGHBORS; j++)
      {
      Tsd_Atom_NeighborId_Put (gtsd, i, j, TSD_INVALID);
      Tsd_Atom_NeighborId_Put (stsd, i, j, TSD_INVALID);
      }
    }
  for (i = 0; i < old_num_atoms; i++)
  {
    Tsd_Atomid_Put (gtsd, i, Tsd_Atomid_Get (gptsd, i));
    Tsd_Atomid_Put (stsd, i, Tsd_Atomid_Get (sgptsd, i));
    for (j = 0; j < MX_NEIGHBORS; j++)
    {
      Tsd_Atom_NeighborId_Put (gtsd, i, j, TSD_INVALID);
      Tsd_Atom_NeighborId_Put (stsd, i, j, TSD_INVALID);
    }
  }
  for (i = 0; i < old_num_atoms; i++)
  {
    for (j = 0; j < MX_NEIGHBORS; j++)
    {
      if (Tsd_Atom_NeighborId_Get (gptsd, i, j) != TSD_INVALID && Tsd_Atom_NeighborBond_Get (gptsd, i, j) != BOND_NONE &&
        !Tsd_Atom_AreConnected (gtsd, i, Tsd_Atom_NeighborId_Get(gptsd, i, j)))
        Tsd_Atom_Connect (gtsd, i, Tsd_Atom_NeighborId_Get (gptsd, i, j), Tsd_Atom_NeighborBond_Get (gptsd, i, j));
      if (Tsd_Atom_NeighborId_Get (sgptsd, i, j) != TSD_INVALID && Tsd_Atom_NeighborBond_Get (sgptsd, i, j) != BOND_NONE &&
        !Tsd_Atom_AreConnected (stsd, i, Tsd_Atom_NeighborId_Get(sgptsd, i, j)))
        Tsd_Atom_Connect (stsd, i, Tsd_Atom_NeighborId_Get (sgptsd, i, j), Tsd_Atom_NeighborBond_Get (sgptsd, i, j));
    }
    next_gatom = next_sgatom = next_new_atom;
    if (Tsd_Atomid_Get (gptsd, i) == 6 && next_new_atom != new_num_atoms)
      for (j = 0, k = l = 1; j < 4 && (k < 8 || l < 8); j++)
      {
      gnbr = Tsd_Atom_NeighborId_Get (gptsd, i, j);
      k += bond_order_x2[Tsd_Atom_NeighborBond_Get (gptsd, i, j)];
      sgnbr = Tsd_Atom_NeighborId_Get (sgptsd, i, j);
      l += bond_order_x2[Tsd_Atom_NeighborBond_Get (sgptsd, i, j)];
/*
      if ((k != 0 && k < 8 && gnbr == TSD_INVALID) || (l != 0 && l < 8 && sgnbr == TSD_INVALID))
        {
printf("connecting %d and %d\n",i,next_new_atom);
        if (k != 0 && k < 8 && gnbr == TSD_INVALID)
          {
          Tsd_Atom_Connect (gtsd, i, next_new_atom, 1);
          k+=2;
          }
        if (l != 0 && l < 8 && sgnbr == TSD_INVALID)
          {
          Tsd_Atom_Connect (stsd, i, next_new_atom, 1);
          l+=2;
          }
        next_new_atom++;
        }
*/
      if (k > 1 && k < 8 && gnbr == TSD_INVALID)
        {
        Tsd_Atom_Connect (gtsd, i, next_gatom, 1);
        next_gatom++;
        k+=2;
        }
      if (l > 1 && l < 8 && sgnbr == TSD_INVALID)
        {
        Tsd_Atom_Connect (stsd, i, next_sgatom, 1);
        next_sgatom++;
        l+=2;
        }
      }
      if (next_gatom > next_new_atom) next_new_atom = next_gatom;
      if (next_sgatom > next_new_atom) next_new_atom = next_sgatom;
    }

  Tsd_MatchCompress_Fix (gtsd,stsd,roots);

  if (!new_schema_local || React_Goal_Get (schema) != NULL)
    {
    Tsd_Destroy (React_Goal_Get (schema));
    Tsd_Destroy (React_Subgoal_Get (schema));
    }

  React_Goal_Put (schema, gtsd);
  React_Subgoal_Put (schema, stsd);
  React_Head_NumGoalAtoms_Put (sch_head, new_num_atoms);
  React_Head_NumSubgoalAtoms_Put (sch_head, new_num_atoms);

  for (i = gpsize = 0; i < new_num_atoms; i++)
    if (!disconnected_atom (gtsd, (U32_t) i)) gpsize++;
  React_Head_GoalSize_Put (sch_head, gpsize);

  for (j=0; j<nroots; j++)
    {
    React_Head_RootNode_Put (sch_head, j, roots[j]);
    React_Head_RootSyntheme_Put (sch_head, j, synthemes[j]);
    }

  for (; j<MX_ROOTS; j++)
    {
    React_Head_RootNode_Put (sch_head, j, REACT_NODE_INVALID);
    React_Head_RootSyntheme_Put (sch_head, j, 0);
    }

/*
  get_patts (curr_schema - 1, FALSE);
*/
  get_patts (curr_schema, FALSE);

  transform_modified = tsd_altered = TRUE;
  if (!new_schema_local)
    {
    React_HeadFlags_Incomplete_Put (sch_head, TRUE);
    incflag_set = TRUE;
    get_text (curr_schema, FALSE);
    val_enable (FALSE, TRUE);
    }

  return (TRUE);
}

Boolean_t must_compress_tsd (Tsd_t *t)
{
  int i, j, k;
  Boolean_t must_compress;

  for (i = 0, must_compress = FALSE; i < Tsd_NumAtoms_Get (t); i++)
    for (j = 0; j < MX_NEIGHBORS; j++)
      if (Tsd_Atom_NeighborBond_Get (t, i, j) == BOND_NONE)
    {
    must_compress = TRUE;
    Tsd_Atom_NeighborId_Put (t, i, j, TSD_INVALID);
    }

  return (must_compress);
}

Boolean_t nodes_in_same_piece (Tsd_t *tsd_p, U32_t node1, U32_t node2)
{
  Boolean_t *visited, found;
  int i;

  visited = (Boolean_t *) malloc (Tsd_NumAtoms_Get (tsd_p) * sizeof (Boolean_t));

  for (i = 0; i < Tsd_NumAtoms_Get (tsd_p); i++) visited[i] = FALSE;

  visit_nbrs (tsd_p, node1, visited);

  found = visited[node2];

  free (visited);

  return (found);
}

void visit_nbrs (Tsd_t *tsd_p, U32_t node, Boolean_t *visited)
{
  int i, j, bond;

  visited[node] = TRUE;

  for (i = 0; i < MX_NEIGHBORS; i++)
    {
    j = Tsd_Atom_NeighborId_Get (tsd_p, node, i);
    bond = Tsd_Atom_NeighborBond_Get (tsd_p, node, i);
    if (j != TSD_INVALID && bond != BOND_NONE && !visited[j]) visit_nbrs (tsd_p, j, visited);
    }
}

Boolean_t disconnected_atom (Tsd_t *tsd_p, U32_t atom)
{
  int i, j, bond;

  for (i = 0; i < MX_NEIGHBORS; i++)
    {
    j = Tsd_Atom_NeighborId_Get (tsd_p, atom, i);
    bond = Tsd_Atom_NeighborBond_Get (tsd_p, atom, i);
    if (j != TSD_INVALID && bond != BOND_NONE) return (FALSE);
    }

  return (TRUE);
}

Boolean_t all_pieces_rooted (Tsd_t *tsd_p, U32_t *roots, int nroots)
{
  Boolean_t *visited, found;
  int i;

  visited = (Boolean_t *) malloc (Tsd_NumAtoms_Get (tsd_p) * sizeof (Boolean_t));

  for (i = 0; i < Tsd_NumAtoms_Get (tsd_p); i++) visited[i] = FALSE;

  for (i = 0; i < nroots; i++) visit_nbrs (tsd_p, roots[i], visited);

  for (i = 0, found = TRUE; i < Tsd_NumAtoms_Get (tsd_p) && found; i++) if (!visited[i] && !disconnected_atom (tsd_p, (U32_t) i))
    found = FALSE;

  free (visited);

  return (found);
}

void SchformManage (int nptests, Boolean_t tests_changed)
{
  XmString label;
  char itemstr[100];
  React_Record_t *schema;
  React_Head_t *sch_head;

  schema=React_Schema_Handle_Get(curr_schema);
  sch_head=React_Head_Get(schema);

  if (nptests == 0) strcpy (itemstr, "No PostTransform Tests");
  else sprintf (itemstr, "[%d] PostTransform Tests", nptests);

  label = XmStringCreateLocalized (itemstr);
  XtVaSetValues (schpb[2 * NUM_BUTTONS_PER_ROW],
    XmNlabelString, label,
    NULL);
  XmStringFree (label);

  XtManageChild (topform);

  posttests_modified = tests_changed;
  if (!new_schema_local && tests_changed)
    {
    React_HeadFlags_Incomplete_Put (sch_head, TRUE);
    incflag_set = TRUE;
    get_text (curr_schema, FALSE);
    val_enable (FALSE, TRUE);
    }
}

void init_templates ()
{
	int i,j,k, ttlen;
	FILE *f;
	char ttname[128];

  NUM_TEMPLATES = PreTemplatesRead (tempfname, MAX_TEMPLATES);

  for (i = 0; i < NUM_TEMPLATES; i++)
    {
    for (j=0; j<4; j++) template_valid[j][i] = FALSE;
    for (j = 0; j < MX_FUNCGROUPS; j++) template[i][j] = FALSE;
    f = fopen (tempfname[i], "r");
    fgets (ttname, 126, f);
    ttlen = strlen (ttname);
    tempname[i] = (char *) malloc (ttlen);
    while (ttname[ttlen - 1] < ' ') ttname[--ttlen] = '\0';
    strcpy (tempname[i], ttname);
    fscanf (f, "%s", ttname);
    ttlen = strlen (ttname);
    tempabbr[i] = (char *) malloc (ttlen + 1);
    strcpy (tempabbr[i], ttname);
    while (fscanf (f, "%d", &j) == 1)
	{
	template[i][j] = TRUE;
	for (k=0; k<4; k++) template_valid[k][i] = TRUE;
	}
    fclose (f);
    }
}

void val_enable (Boolean_t enable, Boolean_t save_label)
{
  XtVaSetValues (schpb[0],
    XmNsensitive, enable ? True : False,
    NULL);

  if (save_label && !enable)
    {
    XtManageChild (sch_savelbl[0]);
    XtManageChild (sch_savelbl[1]);
    }
  else
    {
    XtUnmanageChild (sch_savelbl[0]);
    XtUnmanageChild (sch_savelbl[1]);
    }
}

void exit_enable (Boolean_t enable)
{
  XtVaSetValues (schpb[NUM_BUTTONS_PER_ROW + 2],
    XmNsensitive, enable ? True : False,
    NULL);

  XtVaSetValues (schpb[2 * NUM_BUTTONS_PER_ROW + 2],
    XmNsensitive, enable ? True : False,
    NULL);

  XtVaSetValues (schpb[NUM_BUTTONS - 1],
    XmNsensitive, enable ? True : False,
    NULL);
}

void schedit_window_close ()
{
  if (XtIsManaged (topform)) XtUnmanageChild (topform);
}

void BpTg_CB (Widget w, XtPointer cld, XtPointer cad)
{
  XmString label;

  XtUnmanageChild (bypass_legend_lbl[0]);
  XtUnmanageChild (bypass_legend_lbl[1]);
  bypass_exit = !bypass_exit;

  if (bypass_exit)
  {
    label = XmStringCreateLocalized ("exit to main");
    XtVaSetValues (bypass_legend_lbl[0],
      XmNlabelString, label,
      NULL);
    XmStringFree (label);

    label = XmStringCreateLocalized ("schema list");
    XtVaSetValues (bypass_legend_lbl[1],
      XmNlabelString, label,
      NULL);
    XmStringFree (label);
  }
  else
  {
    label = XmStringCreateLocalized ("exit directly");
    XtVaSetValues (bypass_legend_lbl[0],
      XmNlabelString, label,
      NULL);
    XmStringFree (label);

    label = XmStringCreateLocalized ("to syn_view");
    XtVaSetValues (bypass_legend_lbl[1],
      XmNlabelString, label,
      NULL);
    XmStringFree (label);
  }

  XtManageChild (bypass_legend_lbl[0]);
  XtManageChild (bypass_legend_lbl[1]);
}
