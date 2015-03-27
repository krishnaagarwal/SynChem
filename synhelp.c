/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              SYNHELP.C
*
*    This module provides for the display of context-specific help screens
*    for the reaction library editor.
*
*  Creation Date:
*
*       31-Mar-2000
*
*  Authors:
*
*    Jerry Miller
*
*  Modification History, reverse chronological
*
******************************************************************************/

#include <stdio.h>
#include <string.h>

#include <Xm/DialogS.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#include "synchem.h"
#include "app_resrc.h"

#include "synhelp.h"

#include "rcb.h"

#include "extern.h"

#ifndef _CYGHAT_
#ifdef _CYGWIN_
#define _CYGHAT_
#else
#ifdef _REDHAT_
#define _CYGHAT_
#endif
#endif
#endif

extern Boolean_t IsThere_Draw_Flag;

void Help_Form_Create (Widget top_level)
{
  XmString title;
  Widget dismisspb;
  XmFontList flco18;
  XmFontListEntry cour18;
  static ScreenAttr_t *sca_p;
#ifdef _CYGHAT_
  Widget box;
Arg al[50];
int ac;
#endif

  helpform = XmCreateFormDialog (top_level, "HelpForm", NULL, 0);
  XtUnmanageChild (helpform);

  sca_p = SynAppR_ScrnAtrb_Get (&GSynAppR);

  title = XmStringCreateLocalized ("Synchem Help");
  XtVaSetValues (helpform,
    XmNdialogStyle,     XmDIALOG_MODELESS,
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNbuttonFontList,  SynAppR_FontList_Get (&GSynAppR),
    XmNtextFontList,    SynAppR_FontList_Get (&GSynAppR),
    XmNbackground,      SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
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
    XmNwidth,           3 * Screen_Width_Get (sca_p) / 4,
    XmNfractionBase,    800,
XmNy, 25, /* for window managers that are too stupid to put the top border on the screen! */
    NULL);
  XmStringFree (title);

  title = XmStringCreateLocalized ("Dismiss");
  dismisspb = XtVaCreateManagedWidget ("HelpDismissPB",
    xmPushButtonGadgetClass, helpform,
    XmNlabelString, title,
    XmNtopPosition, 775,
    XmNtopAttachment, XmATTACH_POSITION,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftPosition, 350,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 450,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);
  XmStringFree (title);

  XtAddCallback (dismisspb, XmNactivateCallback, HelpDismiss_CB, (XtPointer) NULL);

#ifdef _CYGHAT_
  box = XtVaCreateManagedWidget ("box",
    xmRowColumnWidgetClass, helpform,
    XmNleftAttachment,      XmATTACH_FORM,
    XmNrightAttachment,     XmATTACH_FORM,
    XmNbottomAttachment,    XmATTACH_WIDGET,
    XmNbottomWidget,        dismisspb,
    XmNorientation,         XmHORIZONTAL,
    XmNheight,              1,
    NULL);
#endif

  helpwin = XmCreateScrolledWindow (helpform, "Help Text", NULL, 0);

  cour18 = XmFontListEntryLoad (XtDisplay (helpform), "-*-courier-medium-r-normal-*-18-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);
  flco18 = XmFontListAppendEntry (NULL, cour18);

#ifdef _CYGHAT_
ac=0;
XtSetArg(al[ac],
        XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR, SAR_CLRI_WHITE)); ac++;
XtSetArg(al[ac],
        XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR, SAR_CLRI_BLACK)); ac++;
XtSetArg(al[ac],
        XmNfontList, flco18); ac++;
XtSetArg(al[ac],
        XmNscrollingPolicy, XmAUTOMATIC); ac++;
XtSetArg(al[ac],
        XmNscrollBarDisplayPolicy, XmAS_NEEDED); ac++;
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
        XmNvalue, helpbuf); ac++;
XtSetArg(al[ac],
        XmNmarginHeight, 0); ac++;
XtSetArg(al[ac],
        XmNmarginWidth, 0); ac++;
XtSetArg(al[ac],
        XmNx,0); ac++;
XtSetArg(al[ac],
        XmNy,0); ac++;
  helptxt = XmCreateText (helpwin, "Help Text", al, ac);
#else
  helptxt = XmCreateText (helpwin, "Help Text", NULL, 0);

  XtVaSetValues(helptxt,XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_WHITE),
        XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_BLACK),
        XmNfontList, flco18,
        XmNscrollingPolicy, XmAUTOMATIC,
        XmNscrollBarDisplayPolicy, XmAS_NEEDED,
        XmNscrollVertical, True,
        XmNeditMode, XmMULTI_LINE_EDIT,
        XmNeditable, False,
        XmNautoShowCursorPosition, False,
        XmNcursorPositionVisible, False,
        XmNvalue, helpbuf,
        XmNmarginHeight, 0,
        XmNmarginWidth, 0,
        XmNx,0,
        XmNy,0,
        NULL);
#endif

  XtVaSetValues (helpwin,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_WIDGET,
#ifdef _CYGHAT_
    XmNbottomWidget,     box,
#else
    XmNbottomWidget,     dismisspb,
#endif
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtManageChild (helptxt);
  XtManageChild (helpwin);
}

void Help_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  char helpfilename[64], which_help[64], *title_str, *hb, *alt_help;
  XmString title;
  FILE *f;
  int c;

  strcpy (which_help, (char *) client_data);
  alt_help = strstr (which_help, "\\");
  if (alt_help == NULL) alt_help = which_help;
  else
  {
    *alt_help++ = '\0';
    if (!(IsThere_Draw_Flag && glob_rxlform)) alt_help = which_help;
  }
  title_str = strstr (alt_help, ":");
  *title_str++ = '\0';
  strcat (title_str, " Help");

  XtManageChild (helpform);

  title = XmStringCreateLocalized (title_str);
  XtVaSetValues (helpform,
    XmNdialogTitle, title,
    NULL);
  XmStringFree (title);

  strcpy (helpfilename, FCB_SEQDIR_SYS (""));
  strcat (helpfilename, "/help/");
  strcat (helpfilename, alt_help);
  strcat (helpfilename, ".txt");

  f = fopen (helpfilename, "r");

  if (f == NULL) sprintf (helpbuf, "\n Help file %s was not found\n", helpfilename);
  else
    {
    hb = helpbuf;
    *hb++ = '\n';
    *hb++ = ' ';
    while ((c = getc (f)) != EOF) if (c != '\r')
      {
      *hb++ = c;
      if (c == '\n') *hb++ = ' ';
      }
    *hb = '\0';
    fclose (f);
    }

  XmTextSetString (helptxt, helpbuf);
}

void HelpDismiss_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild (helpform);
}
