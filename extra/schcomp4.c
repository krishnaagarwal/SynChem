/*******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              SCHCOMP.C
*
*    This module provides for the display and of two schemata.  Its initial
*    purpose is for the comparison of an existing schema and its proposed
*    replacement version.
*
*  Creation Date:
*
*       03-Apr-2002
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
#include <Xm/FileSB.h>
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
#include <Xm/Separator.h>

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
#define _SCHCOMP_
#include "schform.h"
#undef _SCHCOMP_
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

int Post_Test_Count (int, Boolean_t);
void Post_Form_Create (Widget, int);

void SCQuit_Confirm_CB (Widget, XtPointer, XtPointer);
void get_comptext (int *, React_Record_t **);
void get_comptests (int *, React_Record_t **);
void get_comppatts (int *, React_Record_t **);
void ExitComp_CB (Widget, XtPointer, XtPointer);
void PostComp_CB (Widget, XtPointer, XtPointer);
void redraw_compwin (Widget, XtPointer, XtPointer);
void merge_strs (char [][512]);
void Display_String (Widget, Pixmap *, char *);
void Post_Comp_Create (Widget, int [2]);
Window portable_XtWindow (Widget);
void wrap_line_w_cr (char **, char *, int);
Boolean_t pattern_match(Tsd_t **, Tsd_t **, U32_t [][MX_ROOTS], U32_t [][MX_ROOTS]);

static XColor color[2];
static Pixmap textp, testp;
static XmFontList flhv12, flhv18,flco18,flcob18,fl6x10,flhv48;
static void (*return_function)();
static Boolean_t *glob_conf;
static int curr_schemata[2];

void SchComp_Create_Form (Widget top_level, XtAppContext schlContext, int *schn, React_Record_t **schema, Boolean_t *confirm,
  void (*retfunc)())
{
	ScreenAttr_t     *sca_p;
XColor exact;
Status status;
	XmFontListEntry helv12, helv18,cour18,courb18,fs6x10,helv48;
	static Widget frame, schtext_form, schbutt_form, liblistwin, chaplistwin, schlistwin, textwin, drawwin,
		schform,toppaned,ldummy,cdummy,detailpb,srch_txt,srch_pb,srch_lbl, testwin, pretrans_label, namewin;
	XtWidgetGeometry geom;
	int i, j, k, nptests[2], ttlen;
	char itemstr[500], temp[10], fname[100], ttname[128];
	XmString item, label, labels[41], msg;
	Arg scrolling[2];
        React_Head_t *sch_head[2];
        React_TextRec_t *text[2];
        React_TextHead_t *txt_head[2];
unsigned short tfw,tfh;
	FILE *f;

printf("schcomp: schn[0]=%d schn[1]=%d\n",schn[0],schn[1]);

return_function=retfunc;
glob_conf=confirm;

  low_h = FALSE;

  tl = top_level;
  schlC = schlContext;

printf("schcomp 0\n");
  for (i=0; i<2; i++)
    {
curr_schemata[i]=schn[i];
printf("schcomp 0.1 (%d)\n", i);
    nptests[i] = Post_Test_Count (schn[i], FALSE);
printf("schcomp 0.2\n");

    sch_head[i]=React_Head_Get(schema[i]);
    text[i]=React_Text_Get(schema[i]);
    txt_head[i]=React_TxtRec_Head_Get(text[i]);
printf("schcomp 0.3\n");
    }

printf("schcomp 1: topform=%p\n",topform);
  if (topform != (Widget) NULL)
    {
    XtVaGetValues(topform,
      XmNwidth,&tfw,
      XmNheight,&tfh,
      NULL);

    label = XmStringCreateLocalized ("Schema Comparison");

    XtVaSetValues (topform,
      XmNdialogTitle, label,
      NULL);
    XmStringFree (label);

    if (nptests[0]+nptests[1] == 0) strcpy (itemstr, "No PostTransform Tests");
    else if (nptests[0]==nptests[1]) sprintf (itemstr, "[%d] PostTransform Tests", nptests[0]);
    else sprintf (itemstr, "[%d vs %d] PostTransform Tests", nptests[0], nptests[1]);

    label = XmStringCreateLocalized (itemstr);
    XtVaSetValues (schpb[0],
      XmNlabelString, label,
      XmNsensitive, nptests[0]+nptests[1]?True:False,
      NULL);
    XmStringFree (label);

/*
    get_comptext(schn,schema);
    get_comptests(schn,schema);
*/

    XtVaGetValues(topform,
      XmNwidth,&tfw,
      XmNheight,&tfh,
      NULL);

printf ("managing topform\n");
    if (!XtIsManaged (topform)) XtManageChild (topform);
printf ("managed topform\n");

    XtManageChild (schpb[0]);
    if (confirm == NULL)
      {
      XtUnmanageChild (schpb[NUM_BUTTONS_PER_ROW]);
      label = XmStringCreateLocalized ("Quit");
      }
    else
      {
      XtManageChild (schpb[NUM_BUTTONS_PER_ROW]);
      label = XmStringCreateLocalized ("Quit; Cancel Import");
      }

    XtVaGetValues(schpb[NUM_BUTTONS_PER_ROW + 2],
      XmNlabelString, label,
      NULL);

    XmStringFree (label);

    XtManageChild (schpb[NUM_BUTTONS_PER_ROW + 2]);

    get_comptext(schn,schema);
    get_comptests(schn,schema);
    get_comppatts(schn,schema);

    return;
    }

/* initialization */

printf("schcomp 2\n");
  if (confirm==NULL) init_templates ();
printf("schcomp 3\n");

  topform = XmCreateFormDialog (top_level, "Schema Comparison Form", NULL, 0);

  label = XmStringCreateLocalized ("Schema Comparison");

  XtVaSetValues (topform,
    XmNresizePolicy, XmRESIZE_NONE,
    XmNdialogTitle, label,
    XmNwidth,1100,
    XmNheight,900,
    XmNfractionBase,    800,
XmNy, 25, /* for window managers that are too stupid to put the top border on the screen! */
    NULL);
  XmStringFree (label);

  toppaned = XtVaCreateWidget ("paned", xmPanedWindowWidgetClass, topform, NULL);

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
    XmNpaneMinimum,     325,
    XmNpaneMaximum,     425,
    XmNfractionBase,    800,
    NULL);

  schpatt_form = XtVaCreateWidget ("SchPattFm",
    xmFormWidgetClass,  toppaned,
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNallowResize,     True,
    XmNpositionIndex,   1,
    XmNpaneMinimum,     325,
    XmNpaneMaximum,     425,
    XmNfractionBase,    800,
    NULL);

printf("schcomp 4\n");
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
printf("schcomp 4.4 gc=%p\n",gc);
  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_WHITE)); /* WHITE initially for pixmaps */
printf("schcomp 4.5 gc=%p\n",gc);
  XSetFillStyle (disp, gc, FillSolid);
  XSetLineAttributes (disp, gc, 1, LineSolid, CapButt, JoinMiter);
  XSetFont (disp, gc, XLoadFont (disp, "-*-courier-bold-r-normal-*-18-*-75-75-*-*-iso8859-1"));
  if ((status=XAllocNamedColor (disp, SynAppR_Colormap_Get (&GSynAppR), "gray55", color, &exact))!=Success) printf("color[0] failed: status=%d Success=%d\n",status,Success);
  if ((status=XAllocNamedColor (disp, SynAppR_Colormap_Get (&GSynAppR), "gray80", color+1, &exact))!=Success) printf("color[1] failed\n",status,Success);
printf("colors: %d %d\n",color[0].pixel, color[1].pixel);

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
    XmNheight,            1500,
    XmNwidth,             1500,
    XmNfontList, fl6x10,
    XmNresize, False,
    NULL);

  drawp = XCreatePixmap (disp, Screen_RootWin_Get (sca_p),
    1500, 1500, Screen_Depth_Get (sca_p));

  XFillRectangle (disp, drawp, gc, 0, 0, 1500, 1500);

  textp = XCreatePixmap (disp, Screen_RootWin_Get (sca_p),1500,1500,
    Screen_Depth_Get (sca_p));

  XFillRectangle (disp, textp, gc, 0, 0, 1500, 1500);

  testp = XCreatePixmap (disp, Screen_RootWin_Get (sca_p),500,4500,
    Screen_Depth_Get (sca_p));

  XFillRectangle (disp, testp, gc, 0, 0, 500, 4500);

  XtAddCallback (draww,
    XmNexposeCallback,
    redraw_compwin, (XtPointer) &drawp);

printf("schcomp 5\n");
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

  label = XmStringCreateLocalized ("Pretransform Tests");

  pretrans_label = XtVaCreateManagedWidget ("PretransLabel",
    xmLabelWidgetClass, topform,
    XmNfontList, flhv18,
    XmNlabelString, label,
    NULL);

  XmStringFree (label);

  testwin = XtVaCreateWidget ("Pretransform Tests",
    xmScrolledWindowWidgetClass, topform,
    XmNscrollingPolicy, XmAUTOMATIC,
    XmNscrollBarDisplayPolicy, XmAS_NEEDED,
    NULL);

  XtVaSetValues (toppaned,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomPosition,   700,
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
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       toppaned,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (schbutt_form,
    XmNtopOffset,        3,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        toppaned,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_WIDGET,
    XmNrightWidget,      testwin,
    NULL);

printf("schcomp 6\n");
  textwin = XtVaCreateWidget ("Schema Data",
    xmScrolledWindowWidgetClass, schtext_form,
    XmNscrollingPolicy, XmAUTOMATIC,
    XmNscrollBarDisplayPolicy, XmAS_NEEDED,
    NULL);

  textw = XtVaCreateManagedWidget ("Schema_Data",
    xmDrawingAreaWidgetClass, textwin,
    XmNbackground,        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNheight,            1500,
    XmNwidth,             1500,
    XmNfontList, flhv18,
    XmNresize, False,
    NULL);
printf("creation: textw=%p\n",textw);


  XtAddCallback (textw,
    XmNexposeCallback,
    redraw_compwin, (XtPointer) &textp);
printf("callback: textw=%p\n",textw);

  testw = XtVaCreateManagedWidget ("Pretransform_Tests",
    xmDrawingAreaWidgetClass, testwin,
    XmNbackground,        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNheight,            4500,
    XmNwidth,             500,
    XmNfontList, flhv18,
    XmNresize, False,
    NULL);


  XtAddCallback (testw,
    XmNexposeCallback,
    redraw_compwin, (XtPointer) &testp);

/* done clearing pixmaps - restore foreground to black */
  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_BLACK));

XSync(disp,False);
XFlush(disp);

  XtVaSetValues (textwin,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

printf("schcomp 7\n");
  for (i=0; i<NUM_BUTTONS; i++)
    {
    if (i==0)
      {
      if (nptests[0]+nptests[1] == 0) strcpy (itemstr, "No PostTransform Tests");
      else if (nptests[0]==nptests[1]) sprintf (itemstr, "[%d] PostTransform Tests", nptests[0]);
      else sprintf (itemstr, "[%d vs %d] PostTransform Tests", nptests[0], nptests[1]);
      }
    else if (i==NUM_BUTTONS_PER_ROW) strcpy (itemstr, "Exit; Confirm Import");
    else if (i==NUM_BUTTONS_PER_ROW + 2)
      {
      if (confirm == NULL) strcpy (itemstr, "Quit");
      else strcpy (itemstr, "Quit; Cancel Import");
      }
    else continue;

    label = XmStringCreateLocalized (itemstr);

    schpb[i] = XtVaCreateWidget (itemstr,
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
    }

  XtAddCallback (schpb[0], XmNactivateCallback, PostComp_CB, (XtPointer) NULL);
  XtAddCallback (schpb[NUM_BUTTONS_PER_ROW], XmNactivateCallback, ExitComp_CB, (XtPointer) TRUE);
  XtAddCallback (schpb[NUM_BUTTONS_PER_ROW + 2], XmNactivateCallback, ExitComp_CB, (XtPointer) FALSE);

/*
printf("schcomp 8\n");
  get_comptext(schn,schema);
printf("schcomp 9\n");
  get_comptests(schn,schema);
printf("schcomp 10\n");
*/

  XtManageChild (schpb[0]);
  if (confirm != NULL) XtManageChild (schpb[NUM_BUTTONS_PER_ROW]);
  XtManageChild (schpb[NUM_BUTTONS_PER_ROW+2]);

  XtManageChild (textw);
printf("manage: textw=%p\n",textw);
  XtManageChild (testw);
  XtManageChild (draww);

  XtManageChild (drawwin);
  XtManageChild (textwin);

  XtManageChild (testwin);

  XtManageChild (pretrans_label);

  XtManageChild (schpatt_form);

  XtManageChild (toppaned);

  XtManageChild (schtext_form);

  XtManageChild (schbutt_form);

printf ("managing topform (initial)\n");
  if (!XtIsManaged (topform)) XtManageChild (topform);
printf ("managed topform\n");

  get_comptext(schn,schema);
  get_comptests(schn,schema);
  get_comppatts(schn,schema);
printf("drawp=%p textp=%p testp=%p\n",drawp,textp,testp);
}

void get_comptext(int *schnum, React_Record_t **schema)
{
        React_Head_t *sch_head[2];
        React_TextRec_t *text[2];
        React_TextHead_t *txt_head[2];
	int lib[2],chap[2],sch[2],year,hr,min,sec;
	U32_t
        	i, j, jj[2],
        	rtype[2], maxusm[2],
        	credate[2], moddate[2], cretime, modtime,
        	num_comm[2], num_ref[2], syntheme_num[2], eyc_vals[2][3];
	char schname[2][512], crefstr[512], string[25], *sbuf, *sname,
                credate_str[16], moddate_str[16], cretime_str[16], modtime_str[16], tempstr[3][512];
	Boolean_t f1[2],f2[2],f3[2],dummy,eyc_incomplete[2],must_merge;

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_WHITE));

  XClearWindow (disp, portable_XtWindow (textw));
  XFillRectangle (disp, textp, gc, 0, 0, 1500,1500);

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_BLACK));

printf("get_comptext([%d,%d],[%p,%p])]\n",schnum[0],schnum[1],schema[0],schema[1]);
  for (j=0; j<2; j++)
  {
    sch_head[j]=React_Head_Get(schema[j]);
    text[j]=React_Text_Get(schema[j]);
    txt_head[j]=React_TxtRec_Head_Get(text[j]);

    lib[j] = React_Head_Library_Get (sch_head[j]);
    syntheme_num[j] = chap[j] = React_Head_SynthemeFG_Get (sch_head[j]);
    sch[j] = React_TxtHd_OrigSchema_Get(txt_head[j]);
printf("sch[%d]=%d\n",j,sch[j]);
    sprintf(schname[j], "Schema %3d: %s", sch[j], String_Value_Get (React_TxtRec_Name_Get(text[j])));
printf("schname[%d]=%s\n",j,schname[j]);
    credate[j]=React_TxtHd_Created_Get(txt_head[j]);
    moddate[j]=React_TxtHd_LastMod_Get(txt_head[j]);
    num_comm[j]=React_TxtHd_NumComments_Get(txt_head[j]);
    num_ref[j]=React_TxtHd_NumReferences_Get(txt_head[j]);
    eyc_vals[j][0]=React_Head_Ease_Get(sch_head[j]);
    eyc_vals[j][1]=React_Head_Yield_Get(sch_head[j]);
    eyc_vals[j][2]=React_Head_Confidence_Get(sch_head[j]);
    eyc_incomplete[j]=eyc_vals[j][0]==0 && eyc_vals[j][1]==0 && eyc_vals[j][2]==0;
    f1[j]=React_HeadFlags_Protection_Get(sch_head[j]);
    f2[j]=React_HeadFlags_Lookahead_Get(sch_head[j]);
    f3[j]=React_HeadFlags_Disabled_Get(sch_head[j]);
    rtype[j]=React_Head_ReactionType_Get(sch_head[j]);
    maxusm[j]=React_Head_MaxNonident_Get(sch_head[j]);

    sprintf (tempstr[j], "%2d: %s\t\tChapter %2d: %s\n",
      lib[j], libname[lib[j] - 1], chap[j], chapname[chap[j] - 1]);
  }
printf("get_comptext 1\n");
  merge_strs(tempstr);
/***********************************************************************
Original version (modified version that follows is for testing only

  sprintf (schbuf[0], "\n Library %s\n%s\n", tempstr[2], TEXT_RULE);
  sprintf (schbuf[0], "This is the \\old\\new\\ version of the test string\n");
*/
  sprintf (schbuf[0], "Library %s\n%s\n", tempstr[2], TEXT_RULE);

printf("get_comptext 2\n");
  for (j=0; j<2; j++)
  {
    sbuf=tempstr[j];
    *sbuf='\0';
    wrap_line_w_cr (&sbuf, schname[j], MAX_SCH_TEXT);
  }
  merge_strs(tempstr);
  sbuf=schbuf[0]+strlen(schbuf[0]);
  strcpy(sbuf,tempstr[2]);
  sbuf=schbuf[0]+strlen(schbuf[0]);

printf("get_comptext 3\n");
  sprintf(sbuf, "\n\\\\ <SCHEMA IS MARKED AS INCOMPLETE: Validation required for completion>\n%s", TEXT_RULE);
  sbuf=schbuf[0]+strlen(schbuf[0]);

  for (j=0; j<2; j++)
    {
    sprintf(tempstr[j]," EASE: %d; YLD: %d; CONF: %d          FLAG:%s%s%s%s;     TYPE: ",
      eyc_vals[j][0], eyc_vals[j][1], eyc_vals[j][2], f1[j] ? " <PROT>":"", f2[j] ? " <LOOK>":"", f3[j] ? " <DISA>":"",
      f1[j] || f2[j] || f3[j] ? "":" <NONE>");
    switch(rtype[j])
      {
    case REACT_MULT_NO_PREF:
      strcat(tempstr[j],"MNP");
      break;
    case REACT_MULT_PREF_SING:
      strcat(tempstr[j],"MPS");
      break;
    case REACT_MULT_PREF_MULT:
      strcat(tempstr[j],"MPM");
      break;
    case REACT_SING_APPL_ONLY:
      strcat(tempstr[j],"SAO");
      break;
    case REACT_MAX_APPL_ONLY:
      strcat(tempstr[j],"MAO");
      break;
      }
    sprintf(tempstr[j]+strlen(tempstr[j]),";     MAXUSM: %d \n%s \n", maxusm[j],
      eyc_incomplete[j] ? " (Schema cannot be validated with zero initial merit) \n" : "");
    }
  merge_strs(tempstr);
  strcpy(sbuf, tempstr[2]);

/******************************************************************************************
1) Review Persist_ModTime(): prev_schema appears to be uninitialized in this implementation;
2) Do NOT update Creation Date (unless a completely different name and number is used) -
   make it a MODIFICATION date instead.
******************************************************************************************/

printf("get_comptext 4\n");
  for (j=0; j<2; j++)
    {
    if (credate[j])
      {
      date_calc(credate[j],credate_str,&year,NULL,NULL);
/*
      if (year >= 2000)
        {
        cretime = Persist_ModTime (PER_STD, Persist_Orig_Rec (PER_STD, prev_schema, &dummy));
        time_calc(cretime,cretime_str,&hr,&min,&sec);
        sprintf(tempstr[j]," Created: %s %s \n",credate_str,cretime_str);
        }
      else
*/
        sprintf(tempstr[j]," Created: %s \n",credate_str);
      if (!moddate[0] && !moddate[1]) strcat(tempstr[j],"  \n");
      }
    else tempstr[j][0]=0;
    }
  merge_strs(tempstr);
  sbuf=schbuf[0]+strlen(schbuf[0]);
  strcpy(sbuf, tempstr[2]);

/******************************************************************************************
Review Persist_ModTime(): prev_schema appears to be uninitialized in this implementation
******************************************************************************************/

  for (j=0; j<2; j++)
    {
    if (moddate[j])
      {
      date_calc(moddate[j],moddate_str,&year,NULL,NULL);
/*
      if (year >= 2000)
        {
        modtime = Persist_ModTime (PER_STD, prev_schema);
        time_calc(modtime,modtime_str,&hr,&min,&sec);
        sprintf(tempstr[j]," Last Modified: %s %s \n \n",moddate_str,modtime_str);
        }
      else
*/
        sprintf(tempstr[j]," Last Modified: %s \n  \n",moddate_str);
      }
    else tempstr[j][0]=0;
    }
  merge_strs(tempstr);
  sbuf=schbuf[0]+strlen(schbuf[0]);
  strcpy(sbuf, tempstr[2]);

printf("get_comptext 5\n");
  for (jj[0]=jj[1]=0; jj[0]<num_ref[0] || jj[1]<num_ref[1]; jj[0]++, jj[1]++)
    {
must_merge=FALSE;
    for (i=0; i<2; i++)
      {
      tempstr[i][0]=0;
      while (jj[i] < num_ref[i] && String_Value_Get (React_TxtRec_Reference_Get(text[i],jj[i]))[0] == '\007') jj[i]++;
      if (jj[i] < num_ref[i] && String_Value_Get (React_TxtRec_Reference_Get(text[i],jj[i]))[0] != '\007')
        {
must_merge=TRUE;
        sprintf(crefstr,"<REF> %s", String_Value_Get (React_TxtRec_Reference_Get(text[i],jj[i])));
        sbuf=tempstr[i];
        wrap_line_w_cr (&sbuf, crefstr, MAX_SCH_TEXT);
        }
/*
      else strcpy(tempstr[i]," \n");
*/
      }
if (must_merge)
{
    merge_strs(tempstr);
    strcat(schbuf[0],tempstr[2]);
}
    }

printf("get_comptext 6\n");
  for (j=tempstr[0][0]=tempstr[1][0]=0; j<num_comm[0] || j<num_comm[1]; j++)
    {
    for (i=0; i<2; i++)
      {
      tempstr[i][0]=0;
      if (j < num_comm[i])
        {
        sprintf(crefstr,"<COMM> %s", String_Value_Get (React_TxtRec_Comment_Get(text[i],j)));
        sbuf=tempstr[i];
        wrap_line_w_cr (&sbuf, crefstr, MAX_SCH_TEXT);
        }
/*
      else strcpy(tempstr[i]," \n");
*/
      }
    merge_strs(tempstr);
    strcat(schbuf[0],tempstr[2]);
    }

printf("get_comptext 7\n");
  for (jj[0]=jj[1]=tempstr[0][0]=tempstr[1][0]=0; jj[0]<num_ref[0] || jj[1]<num_ref[1]; jj[0]++, jj[1]++)
    {
must_merge=FALSE;
    for (i=0; i<2; i++)
      {
      tempstr[i][0]=0;
      while (jj[i] < num_ref[i] && String_Value_Get (React_TxtRec_Reference_Get(text[i],jj[i]))[0] != '\007') jj[i]++;
      if (jj[i] < num_ref[i] && String_Value_Get (React_TxtRec_Reference_Get(text[i],jj[i]))[0] == '\007')
        {
must_merge=TRUE;
        sprintf(crefstr,"<LIBRARY UPDATE> %s", String_Value_Get (React_TxtRec_Reference_Get(text[i],jj[i])) + 1);
        sbuf=tempstr[i];
        wrap_line_w_cr (&sbuf, crefstr, MAX_SCH_TEXT);
        }
/*
      else strcpy(tempstr[i]," \n");
*/
      }
if (must_merge)
{
    merge_strs(tempstr);
    strcat(schbuf[0],tempstr[2]);
}
    }

printf("before displaying: textw=%p\n",textw);
  Display_String (textw, &textp, schbuf[0]);
printf("get_comptext 8\n");
}

void get_comptests(int *schnum, React_Record_t **schema)
{
  React_Head_t *sch_head[2];
  React_TextRec_t *text[2];
  React_TextHead_t *txt_head[2];
  char *sbuf, tempstr[3][512];
  int i, j, k;
  Boolean_t anycl[2],anycn[2],anyml[2],anymn[2], incomp[2], tcomp[2][4][NUM_TEMPLATES],
    notall[2][MX_FUNCGROUPS],notany[2][MX_FUNCGROUPS],mustall[2][MX_FUNCGROUPS],mustany[2][MX_FUNCGROUPS];

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_WHITE));

  XClearWindow (disp, portable_XtWindow (testw));
  XFillRectangle (disp, testp, gc, 0, 0, 500, 4500);

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_BLACK));

  for (i=0; i<2; i++)
  {
    sch_head[i]=React_Head_Get(schema[i]);
    text[i]=React_Text_Get(schema[i]);
    txt_head[i]=React_TxtRec_Head_Get(text[i]);

    anycl[i]=React_HeadFlags_CantAll_Get(sch_head[i]);
    anycn[i]=React_HeadFlags_CantAny_Get(sch_head[i]);
    anyml[i]=React_HeadFlags_MustAll_Get(sch_head[i]);
    anymn[i]=React_HeadFlags_MustAny_Get(sch_head[i]);

    if (incomp[i] = !anycl[i] && !anycn[i] && !anyml[i] && !anymn[i])
      strcpy (tempstr[i], "No Pretransform Tests\r\r(Schema cannot be validated\r  without tests)");
    else tempstr[i][0]=0;

    for (j = 0; j < 4; j++) for (k = 0; k < NUM_TEMPLATES; k++) tcomp[i][j][k] = template_valid[j][k];

    for (j=0; j<MX_FUNCGROUPS; j++)
    {
      notall[i][j]=anycl[i]?React_CantAll_Get(schema[i], j):0;
      for (k = 0; k < NUM_TEMPLATES; k++) if (template[k][j] && !notall[i][j]) tcomp[i][3][k] = FALSE;
      notany[i][j]=anycn[i]?React_CantAny_Get(schema[i], j):0;
      for (k = 0; k < NUM_TEMPLATES; k++) if (template[k][j] && !notany[i][j]) tcomp[i][0][k] = FALSE;
      mustall[i][j]=anyml[i]?React_MustAll_Get(schema[i], j):0;
      for (k = 0; k < NUM_TEMPLATES; k++) if (template[k][j] && !mustall[i][j]) tcomp[i][2][k] = FALSE;
      mustany[i][j]=anymn[i]?React_MustAny_Get(schema[i], j):0;
      for (k = 0; k < NUM_TEMPLATES; k++)
      if (template[k][j] && !mustany[i][j]) tcomp[i][1][k] = FALSE;
    }
  }

  sbuf=schbuf[1];
/*
strcpy(sbuf,"got here\n");
sbuf+=strlen(sbuf);
*/

  if (tempstr[0][0] || tempstr[1][0])
  {
    merge_strs(tempstr);
    strcpy(sbuf,tempstr[2]);
    sbuf=sbuf+strlen(sbuf);
  }

  if (anycl[0] || anycl[1])
  {
    sprintf(sbuf,
      "\n%sCan't have all of the following:\r%s\n",anycl[0]?(anycl[1]?"":"\\"):"\\\\",anycl[1]?(anycl[0]?"":"\\"):"\\\\");
    sbuf+=strlen(sbuf);
    for (j=1; j<fgend; j++) if (notall[0][j] || notall[1][j])
    {
      sprintf(sbuf,"%s%s%3d: %s%s\n",notall[0][j]?(notall[1][j]?"":"\\"):"\\\\",j<10?"    ":j<100?"   ":"  ",j,fgname[j],
        notall[1][j]?(notall[0][j]?"":"\\"):"\\\\");
      sbuf+=strlen(sbuf);
    }
    for (j=0; j<NUM_TEMPLATES; j++) if (tcomp[0][3][j] || tcomp[1][3][j])
    {
      sprintf(sbuf,"\n%s[%s]%s\n", tcomp[0][3][j]?(tcomp[1][3][j]?"":"\\"):"\\\\", tempname[j],
        tcomp[1][3][j]?(tcomp[0][3][j]?"":"\\"):"\\\\");
      sbuf+=strlen(sbuf);
      for (k=1; k<fgend; k++) if (template[j][k])
      {
        sprintf(sbuf,"%s%s%3d: %s%s\n",tcomp[0][3][j]?(tcomp[1][3][j]?"":"\\"):"\\\\", k<10?"    ":k<100?"   ":"  ",k,fgname[k],
          tcomp[1][3][j]?(tcomp[0][3][j]?"":"\\"):"\\\\");
        sbuf+=strlen(sbuf);
      }
    }
  }

  if (anycn[0] || anycn[1])
    {
    if (anycl[0] || anycl[1])
      {
      strcat (sbuf, TEST_RULE);
      sbuf+=strlen(sbuf);
      }
    sprintf(sbuf,
      "\n%sCan't have any of the following:\r%s\n",anycn[0]?(anycn[1]?"":"\\"):"\\\\",anycn[1]?(anycn[0]?"":"\\"):"\\\\");
    sbuf+=strlen(sbuf);
    for (j=1; j<fgend; j++) if (notany[0][j] || notany[1][j])
      {
      sprintf(sbuf,"%s%s%3d: %s%s\n",notany[0][j]?(notany[1][j]?"":"\\"):"\\\\",j<10?"    ":j<100?"   ":"  ",j,fgname[j],
        notany[1][j]?(notany[0][j]?"":"\\"):"\\\\");
      sbuf+=strlen(sbuf);
      }
    for (j=0; j<NUM_TEMPLATES; j++) if (tcomp[0][0][j] || tcomp[1][0][j])
      {
      sprintf(sbuf,"\n%s[%s]%s\n", tcomp[0][0][j]?(tcomp[1][0][j]?"":"\\"):"\\\\", tempname[j],
        tcomp[1][0][j]?(tcomp[0][0][j]?"":"\\"):"\\\\");
      sbuf+=strlen(sbuf);
      for (k=1; k<fgend; k++) if (template[j][k])
        {
        sprintf(sbuf,"%s%s%3d: %s%s\n",tcomp[0][0][j]?(tcomp[1][0][j]?"":"\\"):"\\\\", k<10?"    ":k<100?"   ":"  ",k,fgname[k],
          tcomp[1][0][j]?(tcomp[0][0][j]?"":"\\"):"\\\\");
        sbuf+=strlen(sbuf);
        }
      }
    }

  if (anyml[0] || anyml[1])
    {
    if (anycl[0] || anycl[1] || anycn[0] || anycn[1])
      {
      strcat (sbuf, TEST_RULE);
      sbuf+=strlen(sbuf);
      }
    sprintf(sbuf,
      "\n%sMust have all of the following:\r%s\n",anyml[0]?(anyml[1]?"":"\\"):"\\\\",anyml[1]?(anyml[0]?"":"\\"):"\\\\");
    sbuf+=strlen(sbuf);
    for (j=1; j<fgend; j++) if (mustall[0][j] || mustall[1][j])
    {
      sprintf(sbuf,"%s%s%3d: %s%s\n",mustall[0][j]?(mustall[1][j]?"":"\\"):"\\\\",j<10?"    ":j<100?"   ":"  ",j,fgname[j],
        mustall[1][j]?(mustall[0][j]?"":"\\"):"\\\\");
      sbuf+=strlen(sbuf);
    }
    for (j=0; j<NUM_TEMPLATES; j++) if (tcomp[0][2][j] || tcomp[1][2][j])
    {
      sprintf(sbuf,"\n%s[%s]%s\n", tcomp[0][2][j]?(tcomp[1][2][j]?"":"\\"):"\\\\", tempname[j],
        tcomp[1][2][j]?(tcomp[0][2][j]?"":"\\"):"\\\\");
      sbuf+=strlen(sbuf);
      for (k=1; k<fgend; k++) if (template[j][k])
      {
        sprintf(sbuf,"%s%s%3d: %s%s\n",tcomp[0][2][j]?(tcomp[1][2][j]?"":"\\"):"\\\\", k<10?"    ":k<100?"   ":"  ",k,fgname[k],
          tcomp[1][2][j]?(tcomp[0][2][j]?"":"\\"):"\\\\");
        sbuf+=strlen(sbuf);
      }
    }
  }

  if (anymn[0] || anymn[1])
  {
    if (anycl[0] || anycl[1] || anycn[0] || anycn[1] || anyml[0] || anyml[1])
    {
      strcat (sbuf, TEST_RULE);
      sbuf+=strlen(sbuf);
    }
    sprintf(sbuf,
      "\n%sMust have any of the following:\r%s\n",anymn[0]?(anymn[1]?"":"\\"):"\\\\",anymn[1]?(anymn[0]?"":"\\"):"\\\\");
    sbuf+=strlen(sbuf);
    for (j=1; j<fgend; j++) if (mustany[0][j] || mustany[1][j])
    {
      sprintf(sbuf,"%s%s%3d: %s%s\n",mustany[0][j]?(mustany[1][j]?"":"\\"):"\\\\",j<10?"    ":j<100?"   ":"  ",j,fgname[j],
        mustany[1][j]?(mustany[0][j]?"":"\\"):"\\\\");
      sbuf+=strlen(sbuf);
    }
    for (j=0; j<NUM_TEMPLATES; j++) if (tcomp[0][1][j] || tcomp[1][1][j])
    {
      sprintf(sbuf,"\n%s[%s]%s\n", tcomp[0][1][j]?(tcomp[1][1][j]?"":"\\"):"\\\\", tempname[j],
        tcomp[1][1][j]?(tcomp[0][1][j]?"":"\\"):"\\\\");
      sbuf+=strlen(sbuf);
      for (k=1; k<fgend; k++) if (template[j][k])
      {
        sprintf(sbuf,"%s%s%3d: %s%s\n",tcomp[0][1][j]?(tcomp[1][1][j]?"":"\\"):"\\\\", k<10?"    ":k<100?"   ":"  ",k,fgname[k],
          tcomp[1][1][j]?(tcomp[0][1][j]?"":"\\"):"\\\\");
        sbuf+=strlen(sbuf);
      }
    }
  }

  Display_String (testw, &testp, schbuf[1]);
}

void get_comppatts(int *schnum, React_Record_t **schema)
{
  React_Head_t *sch_head[2];
  Tsd_t *goal[2], *subgoal[2];
  Xtr_t *txtr;
  Dsp_Molecule_t *dsp;
  char string[256];
  int h, w, i, j, k, x, y, twh, txy, npieces, minx[6], miny[6], maxx[6], maxy[6], goal_x_offset, width, height, spwidth,
    start, yoff, dy, max_y_inc, max_x, midpoint_y;
  U32_t proots[2][MX_ROOTS], psy[2][MX_ROOTS];
  Boolean_t rotate, *dots;
XmString text;

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_WHITE));

  XClearWindow (disp, portable_XtWindow (draww));
  XFillRectangle (disp, drawp, gc, 0, 0, 1500, 1500);

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_BLACK));

  text=XmStringCreateLocalized(" ");
  spwidth=XmStringWidth(fl6x10,text);
  XmStringFree(text);

  sprintf(string,"Subgoal TSD");
  XDrawString(disp,portable_XtWindow(draww),gc,25,25,string,strlen(string));
  XDrawString(disp,drawp,gc,25,25,string,strlen(string));

  if (React_Goal_Get (schema[0]) == NULL)
    {
    strcpy(string,"Undefined");

    if (React_Goal_Get (schema[1]) == NULL) XSetForeground (disp, gc, color[1].pixel);
    else XSetForeground (disp, gc, color[0].pixel);

    XDrawString(disp,portable_XtWindow(draww),gc,50,50,string,strlen(string));
    XDrawString(disp,drawp,gc,50,50,string,strlen(string));

    if (React_Goal_Get (schema[1]) != NULL)
      {
      text=XmStringCreateLocalized(string);
      width=XmStringWidth(fl6x10,text);
      height=XmStringHeight(fl6x10,text);
      XmStringFree(text);
      XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
        SAR_CLRI_RED));
      XDrawLine(disp,portable_XtWindow(draww),gc,50-2*spwidth/3,50-height/3,50+width+2*spwidth/3,50-height/3);
      XDrawLine(disp,drawp,gc,50-2*spwidth/3,50-height/3,50+width+2*spwidth/3,50-height/3);
      }

    goal_x_offset = 200;
    x = 165;
    y = 40;

    if (React_Goal_Get (schema[1]) == NULL) XSetForeground (disp, gc, color[1].pixel);
    else XSetForeground (disp, gc, color[0].pixel);

    XDrawLine (disp, portable_XtWindow (draww), gc, x + 4, y - 4, x + 25, y - 4);
    XDrawLine (disp, portable_XtWindow (draww), gc, x + 4, y + 4, x + 25, y + 4);
    XDrawLine (disp, portable_XtWindow (draww), gc, x, y, x + 10, y - 10);
    XDrawLine (disp, portable_XtWindow (draww), gc, x, y, x + 10, y + 10);
    XDrawLine (disp, drawp, gc, x + 4, y - 4, x + 25, y - 4);
    XDrawLine (disp, drawp, gc, x + 4, y + 4, x + 25, y + 4);
    XDrawLine (disp, drawp, gc, x, y, x + 10, y - 10);
    XDrawLine (disp, drawp, gc, x, y, x + 10, y + 10);

    XDrawString(disp,portable_XtWindow(draww),gc,goal_x_offset + 25,50,string,strlen(string));
    XDrawString(disp,drawp,gc,goal_x_offset + 25,50,string,strlen(string));

    if (React_Goal_Get (schema[1]) != NULL)
      {
      text=XmStringCreateLocalized(string);
      width=XmStringWidth(fl6x10,text);
      height=XmStringHeight(fl6x10,text);
      XmStringFree(text);
      XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
        SAR_CLRI_RED));
      XDrawLine(disp,portable_XtWindow(draww),gc,goal_x_offset+25-2*spwidth/3,50-height/3,
        goal_x_offset+25+width+2*spwidth/3,50-height/3);
      XDrawLine(disp,drawp,gc,goal_x_offset+25-2*spwidth/3,50-height/3,goal_x_offset+25+width+2*spwidth/3,50-height/3);
      }

    XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
      SAR_CLRI_BLACK));

    sprintf(string,"Goal TSD");
    XDrawString(disp,portable_XtWindow(draww),gc,goal_x_offset,25,string,strlen(string));
    XDrawString(disp,drawp,gc,goal_x_offset,25,string,strlen(string));

    sprintf(string, "(Schema cannot be saved without a fully-defined transform)");
    XDrawString(disp,portable_XtWindow(draww),gc,50,100,string,strlen(string));
    XDrawString(disp,drawp,gc,50,100,string,strlen(string));

    if (React_Goal_Get (schema[1]) == NULL) return;

    text=XmStringCreateLocalized(string);
    width=XmStringWidth(fl6x10,text);
    height=XmStringHeight(fl6x10,text);
    XmStringFree(text);
    XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
      SAR_CLRI_RED));
    XDrawLine(disp,portable_XtWindow(draww),gc,50-2*spwidth/3,100-height/3,50+width+2*spwidth/3,100-height/3);
    XDrawLine(disp,drawp,gc,50-2*spwidth/3,100-height/3,50+width+2*spwidth/3,100-height/3);
    }

  for (i=0; i<2; i++)
  {
    sch_head[i]=React_Head_Get(schema[i]);

    for (j=0; j<MX_ROOTS; j++)
    {
      proots[i][j]=React_Head_RootNode_Get(sch_head[i], j);
      psy[i][j]=React_Head_RootSyntheme_Get(sch_head[i], j);
    }

    subgoal[i]=Tsd_Copy(React_Subgoal_Get(schema[i]));
    goal[i]=Tsd_Copy(React_Goal_Get(schema[i]));
  }

  if (pattern_match(goal,subgoal,proots,psy)) start=1;
  else start=0;
  yoff=0;

  XSetForeground (disp, gc, color[1].pixel);

  for (i=start, dy=0, max_y_inc=10; i<2; i++)
  {
    max_x=0;
    if (i == 0) XSetForeground (disp, gc, color[0].pixel);
    else if (start==0) XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
    txtr=Tsd2Xtr(subgoal[i]);

    dsp=Xtr2Dsp(txtr);

    if (dsp_Shelley(dsp))
    {
      Dsp_Compress (dsp);
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
        if (y>max_y_inc) max_y_inc=y;
      }
      mdraw(dsp,subgoal[i],disp,portable_XtWindow(draww),gc,25,50+dy,275,200+dy,
                        NULL,NULL,NULL,TRUE);
      mdraw(dsp,subgoal[i],disp,drawp,gc,25,50+dy,275,200+dy, NULL,NULL,NULL,FALSE);

      get_dsp_stats (dsp, &npieces, minx, miny, maxx, maxy);

      midpoint_y = y = 50 + (miny[0] + maxy[0]) / 2;

      for (j=1; j<npieces; j++)
      {
        x = 25 + (maxx[j] + minx[j+1]) / 2;
        XDrawLine (disp, portable_XtWindow (draww), gc, x - 10, y + dy, x + 10, y + dy);
        XDrawLine (disp, portable_XtWindow (draww), gc, x, y - 10 + dy, x, y + 10 + dy);
        XDrawLine (disp, drawp, gc, x - 10, y + dy, x + 10, y + dy);
        XDrawLine (disp, drawp, gc, x, y - 10 + dy, x, y + 10 + dy);
      }

      free_Molecule (dsp);
    }
    Xtr_Destroy (txtr);
/*
    Tsd_Destroy (subgoal[i]);
*/
    goal_x_offset = maxx[0] + 100;
    x = maxx[0] + 65;
/* draw retrosynthetic arrow */
    XDrawLine (disp, portable_XtWindow (draww), gc, x + 4, y - 4 + dy, x + 25, y - 4 + dy);
    XDrawLine (disp, portable_XtWindow (draww), gc, x + 4, y + 4 + dy, x + 25, y + 4 + dy);
    XDrawLine (disp, portable_XtWindow (draww), gc, x, y + dy, x + 10, y - 10 + dy);
    XDrawLine (disp, portable_XtWindow (draww), gc, x, y + dy, x + 10, y + 10 + dy);
    XDrawLine (disp, drawp, gc, x + 4, y - 4 + dy, x + 25, y - 4 + dy);
    XDrawLine (disp, drawp, gc, x + 4, y + 4 + dy, x + 25, y + 4 + dy);
    XDrawLine (disp, drawp, gc, x, y + dy, x + 10, y - 10 + dy);
    XDrawLine (disp, drawp, gc, x, y + dy, x + 10, y + 10 + dy);

    if (dy==0)
    {
      sprintf(string,"Goal TSD (dotted nodes in red: set \"H on C: show all\" to see all dotted H's)");
      XDrawString(disp,portable_XtWindow(draww),gc,goal_x_offset,25,string,strlen(string));
      XDrawString(disp,drawp,gc,goal_x_offset,25,string,strlen(string));
    }

    dots = (Boolean_t *) malloc (Tsd_NumAtoms_Get(goal[i])*sizeof(Boolean_t));
    for (j=0; j<Tsd_NumAtoms_Get(goal[i]); j++) dots[j]=Tsd_AtomFlags_Dot_Get(goal[i],j);
    txtr=Tsd2Xtr(goal[i]);
    dsp=Xtr2Dsp(txtr);

    if (dsp_Shelley(dsp))
    {
      Dsp_Compress (dsp);
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
        if (x>max_x) max_x=x;
        dsp->atoms[k].y=y;
        if (y>max_y_inc) max_y_inc=y;
      }
      mdraw(dsp,goal[i],disp,portable_XtWindow(draww),gc,goal_x_offset,50+dy,goal_x_offset + 225,200+dy,
                        proots[i],psy[i],dots,TRUE);
      mdraw(dsp,goal[i],disp,drawp,gc,goal_x_offset,50+dy,goal_x_offset + 225,200+dy, proots[i],psy[i],dots,FALSE);

      free(dots);

      get_dsp_stats (dsp, &npieces, minx, miny, maxx, maxy);

      y = 50 + (miny[0] + maxy[0]) / 2;

      for (j=1; j<npieces; j++)
      {
        x = goal_x_offset + (maxx[j] + minx[j+1]) / 2;
/* draw plus signs between subgoal pieces */
        XDrawLine (disp, portable_XtWindow (draww), gc, x - 10, y + dy, x + 10, y + dy);
        XDrawLine (disp, portable_XtWindow (draww), gc, x, y - 10 + dy, x, y + 10 + dy);
        XDrawLine (disp, drawp, gc, x - 10, y + dy, x + 10, y + dy);
        XDrawLine (disp, drawp, gc, x, y - 10 + dy, x, y + 10 + dy);
      }
      if (i==0)
      {
        XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_RED));
        XDrawLine (disp, portable_XtWindow (draww), gc, 20, midpoint_y, maxx[npieces-1]/*max_x*/+goal_x_offset+15, midpoint_y);
        XDrawLine (disp, drawp, gc, 20, midpoint_y, maxx[npieces-1]/*max_x*/+goal_x_offset+15, midpoint_y);
      }

      free_Molecule (dsp);
    }
    Xtr_Destroy (txtr);
/*
    Tsd_Destroy (goal[i]);
*/
    dy = max_y_inc + 25;
  }
  for (i=0; i<2; i++)
  {
    if (goal[i]!=NULL) Tsd_Destroy (goal[i]);
    if (subgoal[i]!=NULL) Tsd_Destroy (subgoal[i]);
  }

XSync(disp,False);
  XFlush (disp);
}

void PostComp_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild (topform);
  Post_Comp_Create (tl, curr_schemata);
}

void ExitComp_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
/*
exit(0);
*/
  XtUnmanageChild (topform);
  if (glob_conf!=NULL) *glob_conf=(Boolean_t)(int)client_data;
  return_function();
}

void redraw_compwin
  (
  Widget      drawing_a,
  XtPointer   client_data,
  XtPointer   call_data
  )

{
  XmDrawingAreaCallbackStruct *cbs =
    (XmDrawingAreaCallbackStruct *) call_data;
  Pixmap *p =
    (Pixmap *) client_data;
  Window win;
  int i;

win=portable_XtWindow(drawing_a);
XCopyArea (cbs->event->xexpose.display,*p,win,gc,
  cbs->event->xexpose.x, cbs->event->xexpose.y, cbs->event->xexpose.width, cbs->event->xexpose.height,
  cbs->event->xexpose.x, cbs->event->xexpose.y);
XSync(disp,False);
XFlush(disp);
}

void schcomp_window_close ()
{
  if (XtIsManaged (topform)) XtUnmanageChild (topform);
}

void merge_strs (char tempstr[][512])
{
  int i, j, k, len;
  char *s, temp_tempstr[3][512], *cr[2], *tcr[2];
  Boolean_t cr_found;

for (i=tempstr[2][0]=0, cr_found=FALSE; i<2; i++) cr[i]=tempstr[i];
do
{
  for (i=0; i<2; i++)
  {
    tcr[i]=strstr(cr[i],"\r");
    if (tcr[i]==NULL) tcr[i]=cr[i]+strlen(cr[i]);
    else *tcr[i]++=0;
  }
  if (tcr[0][0] || tcr[1][0])
  {
    cr_found=TRUE;
    for (i=0; i<2; i++) if (tcr[i][0] || !cr[i][0]) sprintf (temp_tempstr[i], "%s     \n", cr[i]);
    else strcpy (temp_tempstr[i], cr[i]);
    merge_strs (temp_tempstr);
    strcat(tempstr[2],temp_tempstr[2]);
    for (i=0; i<2; i++) cr[i]=tcr[i];
  }
}
while (tcr[0][0] || tcr[1][0]);
if (cr_found)
{
  for (i=0; i<2; i++) if (!cr[i][0]) strcpy (temp_tempstr[i], "     \n");
  else strcpy (temp_tempstr[i], cr[i]);
  merge_strs (temp_tempstr);
  strcat(tempstr[2],temp_tempstr[2]);
  return;
}

printf("merging\n");
for (s=tempstr[0]; *s; s++) if (*s<' ' || *s>'~') printf ("[%02x]",(unsigned int)*s);
else putchar((int)*s);
putchar('\n');
printf("and\n");
for (s=tempstr[1]; *s; s++) if (*s<' ' || *s>'~') printf ("[%02x]",(unsigned int)*s);
else putchar((int)*s);
putchar('\n');
  if (!strcmp (tempstr[0], tempstr[1]))
  {
    strcpy (tempstr[2], tempstr[0]);
    return;
  }
  if (tempstr[0][0]==0)
  {
    len = strlen(tempstr[1]);
    if (tempstr[1][len-1]!='\n') sprintf (tempstr[2], "\\\\%s\\", tempstr[1]);
    else
    {
      tempstr[1][len-1]=0;
      sprintf (tempstr[2], "\\\\%s\\\n", tempstr[1]);
      tempstr[1][len-1]='\n';
    }
    return;
  }
  if (tempstr[1][0]==0)
  {
    len = strlen(tempstr[0]);
    if (tempstr[0][len-1]!='\n') sprintf (tempstr[2], "\\%s\\\\", tempstr[0]);
    else
    {
      tempstr[0][len-1]=0;
      sprintf (tempstr[2], "\\%s\\\\\n", tempstr[0]);
      tempstr[0][len-1]='\n';
    }
    return;
  }
  for (i=0; tempstr[0][i]==tempstr[1][i]; i++);
  while (i>0 && tempstr[0][i-1]>' ') i--;
  for (j=strlen(tempstr[0]), k=strlen(tempstr[1]); j>i && k>i && tempstr[0][j]==tempstr[1][k]; j--, k--);
  while (j<strlen(tempstr[0]) && tempstr[0][j+1]>' ') j++, k++;
  j++, k++;
  strncpy(tempstr[2],tempstr[0],i);
  strcpy(tempstr[2]+i,"\\");
  strncat(tempstr[2],tempstr[0]+i,j-i);
  strcpy(tempstr[2]+j+1,"\\");
  strncat(tempstr[2],tempstr[1]+i,k-i);
  strcpy(tempstr[2]+j+k-i+2,"\\");
  strcat(tempstr[2],tempstr[0]+j);
printf("tempstr[2]=%s\n",tempstr[2]);
}

void Display_String (Widget w, Pixmap *p, char *string)
{
  int i, x, y, substr_counter, substr1_len, curr_len, height, curr_width, substr1_width, space_width;
  char *curr_string, *next_string, *curr_substr, *next_substr, *s;
  XmString curr_text;
Window win;
Boolean_t text_found;

win=portable_XtWindow(w);
  XSetFont (disp, gc, XLoadFont (disp, "-*-helvetica-bold-r-normal-*-18-*-75-75-*-*-iso8859-1"));

printf("displaying\n");
for (s=string; *s; s++) if (*s<' ' || *s>'~') printf ("[%02x]",(unsigned int)*s);
else putchar((int)*s);
putchar('\n');

printf("displaying: textw=%p testw=%p w=%p p=%p textp=%p testp=%p\n",textw,testw,w,p,textp,testp);

  curr_text=XmStringCreateLocalized(" ");
  space_width=XmStringWidth(flhv18,curr_text);
  XmStringFree(curr_text);
  curr_text=XmStringCreateLocalized(string);
  height = XmStringHeight (flhv18, curr_text);
  XmStringFree(curr_text);
  if (w==textw) y=2*height;
  else y=height;
  for (curr_string=string; curr_string[0]; curr_string=next_string)
  {
    x=2*space_width;
    next_string=strstr(curr_string, "\n");
    if (next_string==NULL) next_string=curr_string+strlen(curr_string);
    else *next_string++=0;
    for (curr_substr=curr_string, substr_counter=0; curr_substr[0]; curr_substr=next_substr, substr_counter++)
    {
      switch (substr_counter)
      {
      case 0:
      case 3:
        XSetForeground (disp, gc, color[0].pixel);
        break;
      case 1:
        XSetForeground (disp, gc, color[1].pixel);
        break;
      default:
        XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
        break;
      }
      next_substr=strstr(curr_substr,"\\");
      if (next_substr==NULL) next_substr=curr_substr+strlen(curr_substr);
      else *next_substr++=0;
      curr_len=strlen(curr_substr);
      for (i=0, text_found=FALSE; i<curr_len; i++) if (curr_substr[i]>' ') text_found=TRUE;
      curr_text=XmStringCreateLocalized(curr_substr);
      curr_width=XmStringWidth(flhv18,curr_text);
      XmStringFree(curr_text);
      if (curr_len!=0 && w==testw) y+=height;
      if (curr_len!=0 || w==textw)
      {
        XDrawString(disp,win,gc,x,y,curr_substr,curr_len);
        XDrawString(disp,*p,gc,x,y,curr_substr,curr_len);
        if (substr_counter==1 && curr_width!=0)
        {
          if (text_found)
          {
            XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_RED));
            XDrawLine(disp,win,gc,x-2*space_width/3,y-height/3,x+curr_width+2*space_width/3,y-height/3);
            XDrawLine(disp,*p,gc,x-2*space_width/3,y-height/3,x+curr_width+2*space_width/3,y-height/3);
printf("\ndrawing line: (%d,%d)-(%d,%d)\n\n",x-2*space_width/3,y-height/3,x+curr_width+2*space_width/3,y-height/3);
          }
        }
      }
      if (w==textw) switch (substr_counter)
      {
      case 0:
        x+=curr_width+(text_found?space_width:0);
        y+=next_substr[0]?-5*height/12:2*height;
        break;
      case 1:
        substr1_width=curr_width;
        y+=5*height/6;
        break;
      case 2:
        x+=space_width+(substr1_width>curr_width?substr1_width:curr_width);
        y+=next_substr[0]?-5*height/12:19*height/12;
        break;
      default:
        y+=2*height;
        break;
      }
    }
  }
/* restore to default for patterns */
  XSetFont (disp, gc, XLoadFont (disp, "-*-courier-bold-r-normal-*-18-*-75-75-*-*-iso8859-1"));
XSync(disp,False);
XFlush(disp);
}

void SchcompManage ()
{
  XtManageChild (topform);
}

Window portable_XtWindow (Widget w)
{
  Window win;
  Widget winw;

  for (winw=w; winw; winw=XtParent(winw)) if (win=XtWindow(winw)) return (win);
  return((Window)0);
}

void wrap_line_w_cr (char **buf, char *line, int max)
{
  char *save_buf, *nl;

  save_buf = *buf;
  wrap_line (buf, line, max);
  while ((nl = strstr (save_buf, "\n")) != NULL && nl<*buf-1) *nl = '\r';
  if (nl==*buf-1 && (nl==0 || *(nl-1)!=' ')) strcpy(nl," \n");
}

Boolean_t pattern_match(Tsd_t **g, Tsd_t **subg, U32_t r[][MX_ROOTS], U32_t s[][MX_ROOTS])
{
  int i,j;

  for (i=0; i<MX_ROOTS && (r[0][i]!=REACT_NODE_INVALID || r[1][i]!=REACT_NODE_INVALID); i++)
    if (r[0][i]!=r[1][i] || s[0][i]!=s[1][i]) return(FALSE);
  if (Tsd_NumAtoms_Get (g[0]) != Tsd_NumAtoms_Get (g[1]) || Tsd_NumAtoms_Get (subg[0]) != Tsd_NumAtoms_Get (subg[1]))
    return(FALSE);
  for (i=0; i<Tsd_NumAtoms_Get (g[0]); i++)
  {
    if (Tsd_Atomid_Get (g[0], i) != Tsd_Atomid_Get (g[1], i)) return(FALSE);
    for (j=0; j<MX_NEIGHBORS; j++) if (Tsd_Atom_NeighborId_Get (g[0], i, j) != Tsd_Atom_NeighborId_Get (g[1], i, j) ||
      Tsd_Atom_NeighborBond_Get (g[0], i, j) != Tsd_Atom_NeighborBond_Get (g[1], i, j)) return(FALSE);
  }
  for (i=0; i<Tsd_NumAtoms_Get (subg[0]); i++)
  {
    if (Tsd_Atomid_Get (subg[0], i) != Tsd_Atomid_Get (subg[1], i)) return(FALSE);
    for (j=0; j<MX_NEIGHBORS; j++) if (Tsd_Atom_NeighborId_Get (subg[0], i, j) != Tsd_Atom_NeighborId_Get (subg[1], i, j) ||
      Tsd_Atom_NeighborBond_Get (subg[0], i, j) != Tsd_Atom_NeighborBond_Get (subg[1], i, j)) return(FALSE);
  }
}
