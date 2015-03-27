/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:                     EDITTEMP.C
*
*    The pretransform template editor for the Synchem GUI.
*
*  Creation Date:
*
*    06-Sep-2000
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
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
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
#include "debug.h"
#include "synio.h"

#include "app_resrc.h"

#include "synhelp.h"

#include "submit_draw.h"

#ifndef _H_FUNCGROUP_FILE_
#include "funcgroup_file.h"
#endif

#ifndef _H_ISAM_
#include "isam.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
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

#ifndef _H_PRE_TEMPLATE_
#include "pre_template.h"
#endif

#ifndef _H_EXTERN_
#include "extern.h"
#endif

#define MAX_TEMPLATES 50
#define NUMBER_OF_ILLEGAL_STRUCTURES 29
#define ILLEGAL_FLAG 0x7ffe
#define READ_FILE 0
#define WRITE_FILE 1

extern char *tempname[], *tempabbr[];
extern int NUM_TEMPLATES;
extern Boolean_t template[MAX_TEMPLATES][MX_FUNCGROUPS], template_complete[4][MAX_TEMPLATES];

static Boolean_t fgsel[2][MX_FUNCGROUPS],fgpossel[2][MX_FUNCGROUPS],illegal[MX_FUNCGROUPS],modified,merge_flag;
static Widget topform = (Widget) NULL, prel,WriteFailMsg,OverwriteMsg,MissingInfoMsg, fileform, filel, filet[3], fileb[2];
static int illegal_fg[NUMBER_OF_ILLEGAL_STRUCTURES] /* FROM COMPOK */ =
	{48, 246, 247, 248, 249, 250,      252, 253, 254, 255,
	 257, 258, 259, 260, 262, 263, 264, 265, 266, 267,
	 268, 269, 271, 272, 273, 274, 275, 270, 549},
  file_mode;
static React_Record_t *schema;
static React_Head_t *sch_head;
static char tempfname[MAX_TEMPLATES][128],*selected_fname=NULL,abbrev_selfname[128];

void Refresh_Pretests (Boolean_t);

/*
void alphabetize_templates ();
*/
void strip_path_ext(char *);
void FileWrite_Button_CB (Widget, XtPointer, XtPointer);
void ETToggle_FG (Widget, XtPointer, XtPointer);
void ETToggle_File (Widget, XtPointer, XtPointer);
void ETHelp_CB (Widget, XtPointer, XtPointer);
void ETButt_CB (Widget, XtPointer, XtPointer);
void ETRW_File_CB (Widget, XtPointer, XtPointer);
void ETWrite_CB (Widget, XtPointer, XtPointer);
void ETFill_FGList(Widget);
void ETFill_FileList(Widget, Boolean_t);
void ETFG_Select(Widget,int,Boolean_t);
void ETFG_Deselect(Widget,int);
void ETFG_Pos_Select(Widget,int);
void ETFG_Pos_Deselect(Widget,int);
void ETFG_Deselect_All(Widget,Boolean_t,Boolean_t);
void ETFG_Restore(Widget w);

void ETemp_Form_Create (Widget top_level)
{
  XmFontListEntry
	courb18, helv18;
  XmFontList
	flcob18, flhv18;
  Display
	*disp;
  int
	i, j;
  XmString
	label, title, title2;
  Widget
	prew, button[6], filewrite_msg[3], filew, filelbl[3], helppb;
  static char
	*itemstr[7] = {"Reset", "Restore Previous", "Reset & Open", "Open & Merge", "View Structures", "Save As ...", "Exit"};

  disp = XtDisplay (top_level);

  modified = FALSE;

  if (topform == (Widget) NULL)
    {
    topform = XmCreateFormDialog (top_level, "Pretransform Template Editor", NULL, 0);
    label = XmStringCreateLocalized ("Pretransform Test Editor");

    XtVaSetValues (topform /*[which]*/,
      XmNresizePolicy, XmRESIZE_NONE,
      XmNdialogTitle, label,
      XmNwidth,1100,
      XmNheight,900,
      XmNfractionBase,    800,
      NULL);

    prew = XmCreateScrolledWindow(topform, "Tenplate Functional Groups", NULL, 0);
    prel = XmCreateList(prew, "PreTransform Test List", NULL, 0);

    courb18 = XmFontListEntryLoad (disp, "-*-courier-bold-r-normal-*-18-*-75-75-*-*-iso8859-1",
    	XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);
    helv18 = XmFontListEntryLoad (disp, "-*-helvetica-bold-r-normal-*-18-*-75-75-*-*-iso8859-1",
    	XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);

    flcob18=XmFontListAppendEntry (NULL, courb18);
    flhv18 = XmFontListAppendEntry (NULL, helv18);

    XtVaSetValues(prel,XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
	SAR_CLRI_WHITE),
	XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
	SAR_CLRI_BLACK),
	XmNfontList,flcob18,
	/* XmNtitle,"PreTransform Tests", */ XmNx,0,XmNy,0,
	XmNselectionPolicy,XmMULTIPLE_SELECT,NULL);

    XtVaSetValues(prew,
	XmNtopOffset,0,XmNtopAttachment,XmATTACH_FORM,
	XmNbottomPosition,775,XmNbottomAttachment,XmATTACH_POSITION,
	XmNleftOffset,0,XmNleftAttachment,XmATTACH_FORM,
	XmNrightOffset,0,XmNrightAttachment,XmATTACH_FORM,
	NULL);
    XtAddCallback(prel,XmNmultipleSelectionCallback,ETToggle_FG,(XtPointer) disp);

    label = XmStringCreateLocalized ("Help");

    helppb = XtVaCreateManagedWidget ("HelpPB",
      xmPushButtonGadgetClass, topform,
      XmNfontList, flhv18,
      XmNlabelString, label,
      XmNrecomputeSize, True,
      XmNtopOffset, 0,
      XmNtopAttachment, XmATTACH_WIDGET,
      XmNtopWidget, prew,
      XmNbottomOffset, 0,
      XmNbottomAttachment, XmATTACH_FORM,
      XmNleftPosition, 1,
      XmNleftAttachment, XmATTACH_POSITION,
      XmNrightPosition, 49,
      XmNrightAttachment, XmATTACH_POSITION,
      NULL);

    XmStringFree (label);

    XtAddCallback (helppb, XmNactivateCallback, ETHelp_CB, (XtPointer) "edittemp:Pretransform Template Editor");

    for (i=0; i<7; i++)
      {
      label = XmStringCreateLocalized (itemstr[i]);

      button[i] = XtVaCreateManagedWidget (itemstr[i],
        xmPushButtonGadgetClass, topform,
        XmNfontList, flhv18,
        XmNlabelString, label,
        XmNrecomputeSize, True,
        XmNtopOffset, 0,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, prew,
        XmNbottomOffset, 0,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftPosition, 107 * i + 51,
        XmNleftAttachment, XmATTACH_POSITION,
        XmNrightPosition, 107 * (i + 1) + 49,
        XmNrightAttachment, XmATTACH_POSITION,
        NULL);

      XmStringFree (label);

      XtAddCallback (button[i], XmNactivateCallback, ETButt_CB, (XtPointer) i);
      XtManageChild (button[i]);
      }

    for (i=0; i<3; i++)
      {
      filewrite_msg[i] = XmCreateMessageDialog (top_level, "Message", NULL, 0);
      title=XmStringCreateLocalized("Dismiss");
      title2=XmStringCreateLocalized("Overwrite");
      label=XmStringCreateLocalized("");
      XtVaSetValues (filewrite_msg[i],
        XmNmessageString, label,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNokLabelString, i==0?title2:title,
        NULL);
      XmStringFree(title);
      XmStringFree(title2);
      XmStringFree(label);

      if (i==0)
        XtAddCallback (XmMessageBoxGetChild (filewrite_msg[i], XmDIALOG_CANCEL_BUTTON), XmNactivateCallback, FileWrite_Button_CB,
        (XtPointer) 2);
      else XtUnmanageChild (XmMessageBoxGetChild (filewrite_msg[i], XmDIALOG_CANCEL_BUTTON));
      XtUnmanageChild (XmMessageBoxGetChild (filewrite_msg[i], XmDIALOG_HELP_BUTTON));
      XtAddCallback (XmMessageBoxGetChild (filewrite_msg[i], XmDIALOG_OK_BUTTON), XmNactivateCallback, FileWrite_Button_CB,
        (XtPointer) i);
      XtUnmanageChild (filewrite_msg[i]);
      }

    OverwriteMsg=filewrite_msg[0];
    WriteFailMsg=filewrite_msg[1];
    MissingInfoMsg=filewrite_msg[2];

    fileform = XmCreateFormDialog (top_level, "Template Files", NULL, 0);
    label = XmStringCreateLocalized ("Pretransform Template Files");

    XtVaSetValues (fileform,
      XmNresizePolicy, XmRESIZE_NONE,
      XmNdialogTitle, label,
      XmNwidth,700,
      XmNheight,300,
      XmNfractionBase,    800,
      NULL);

    filew = XmCreateScrolledWindow(fileform, "PreTransform Templates", NULL, 0);
    filel = XmCreateList(filew, "PreTransform Template List", NULL, 0);
    filelbl[0] = XmCreateLabel(fileform,"tfnamelbl", NULL, 0);
    filelbl[1] = XmCreateLabel(fileform,"tnamelbl", NULL, 0);
    filelbl[2] = XmCreateLabel(fileform,"tabbrlbl", NULL, 0);
    filet[0] = XmCreateTextField(fileform, "Template Filename", NULL, 0);
    filet[1] = XmCreateTextField(fileform, "Template Name", NULL, 0);
    filet[2] = XmCreateTextField(fileform, "Template Abbr", NULL, 0);

    XtVaSetValues(filel,XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
      	SAR_CLRI_WHITE),
       	XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
       	SAR_CLRI_BLACK),
       	XmNfontList,flcob18,
       	XmNtitle,"PreTransform Templates",XmNx,0,XmNy,0,
       	XmNselectionPolicy,XmSINGLE_SELECT,NULL);

    XtVaSetValues(filew,
	XmNtopOffset,0,XmNtopAttachment,XmATTACH_FORM,
	XmNbottomPosition,500,XmNbottomAttachment,XmATTACH_POSITION,
	XmNleftOffset,0,XmNleftAttachment,XmATTACH_FORM,
	XmNrightOffset,0,XmNrightAttachment,XmATTACH_FORM,
	NULL);

    label=XmStringCreateLocalized ("Filename");

    XtVaSetValues(filelbl[0],
       	XmNfontList,flhv18,
	XmNlabelString, label,
	XmNtopOffset,0,XmNtopAttachment,XmATTACH_WIDGET,
	XmNtopWidget, filel,
	XmNbottomPosition,600,XmNbottomAttachment,XmATTACH_POSITION,
	XmNleftOffset,0,XmNleftAttachment,XmATTACH_FORM,
	XmNrightPosition, 400,XmNrightAttachment,XmATTACH_POSITION,
	NULL);

    XmStringFree(label);

    label=XmStringCreateLocalized ("Template");

    XtVaSetValues(filelbl[1],
        XmNfontList,flhv18,
	XmNlabelString, label,
	XmNtopOffset,0,XmNtopAttachment,XmATTACH_WIDGET,
	XmNtopWidget, filel,
	XmNbottomPosition,600,XmNbottomAttachment,XmATTACH_POSITION,
	XmNleftOffset,0,XmNleftAttachment,XmATTACH_WIDGET,
	XmNleftWidget, filelbl[0],
	XmNrightPosition, 700,XmNrightAttachment,XmATTACH_POSITION,
	NULL);

    XmStringFree(label);

    label=XmStringCreateLocalized ("Abbr.");

    XtVaSetValues(filelbl[2],
        XmNfontList,flhv18,
	XmNlabelString, label,
	XmNtopOffset,0,XmNtopAttachment,XmATTACH_WIDGET,
	XmNtopWidget, filel,
	XmNbottomPosition,600,XmNbottomAttachment,XmATTACH_POSITION,
	XmNleftOffset,0,XmNleftAttachment,XmATTACH_WIDGET,
	XmNleftWidget, filelbl[1],
	XmNrightOffset, 0, XmNrightAttachment,XmATTACH_FORM,
	NULL);

    XmStringFree(label);

    XtVaSetValues(filet[0],
        XmNfontList,flcob18,
	XmNtopOffset,0,XmNtopAttachment,XmATTACH_WIDGET,
	XmNtopWidget, filelbl[0],
	XmNbottomPosition,700,XmNbottomAttachment,XmATTACH_POSITION,
	XmNleftOffset,0,XmNleftAttachment,XmATTACH_FORM,
	XmNrightOffset,0,XmNrightAttachment,XmATTACH_WIDGET,
	XmNrightWidget, filelbl[1],
	NULL);

    XtVaSetValues(filet[1],
        XmNfontList,flcob18,
	XmNtopOffset,0,XmNtopAttachment,XmATTACH_WIDGET,
	XmNtopWidget, filelbl[1],
	XmNbottomPosition,700,XmNbottomAttachment,XmATTACH_POSITION,
	XmNleftOffset,0,XmNleftAttachment,XmATTACH_WIDGET,
	XmNleftWidget, filet[0],
	XmNrightOffset,0,XmNrightAttachment,XmATTACH_WIDGET,
	XmNrightWidget, filelbl[2],
	NULL);

    XtVaSetValues(filet[2],
        XmNfontList,flcob18,
	XmNtopOffset,0,XmNtopAttachment,XmATTACH_WIDGET,
	XmNtopWidget, filelbl[2],
	XmNbottomPosition,700,XmNbottomAttachment,XmATTACH_POSITION,
	XmNleftOffset,0,XmNleftAttachment,XmATTACH_WIDGET,
	XmNleftWidget, filet[1],
	XmNrightOffset,0,XmNrightAttachment,XmATTACH_FORM,
	NULL);

    label = XmStringCreateLocalized ("Cancel");

    for (i=0; i<2; i++)
      {
      fileb[i] = XtVaCreateManagedWidget (itemstr[i],
        xmPushButtonGadgetClass, fileform,
        XmNfontList, flhv18,
        XmNlabelString, label,
        XmNrecomputeSize, True,
        XmNtopOffset, 0,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, filet[i],
        XmNbottomOffset, 0,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftPosition, 400 * i,
        XmNleftAttachment, XmATTACH_POSITION,
        XmNrightPosition, 400 * (i + 1),
        XmNrightAttachment, XmATTACH_POSITION,
        NULL);

      XtAddCallback(fileb[i],XmNactivateCallback,ETRW_File_CB,(XtPointer) i);
      }
    XmStringFree (label);

    XtAddCallback(filel,XmNsingleSelectionCallback,ETToggle_File,(XtPointer) FALSE);
    XtAddCallback(filel,XmNdefaultActionCallback,ETToggle_File,(XtPointer) TRUE);

    XtManageChild (filel);
    XtManageChild (filew);
    XtManageChild (filelbl[0]);
    XtManageChild (filelbl[1]);
    XtManageChild (filelbl[2]);
    XtManageChild (filet[0]);
    XtManageChild (filet[1]);
    XtManageChild (filet[2]);
    XtManageChild (fileb[0]);
    XtManageChild (fileb[1]);

    ETFill_FGList (prel);
    XtManageChild (prel);
    XtManageChild (prew);
    }
  ETFill_FileList(filel, FALSE);
  XtManageChild (topform);
  XtUnmanageChild (fileform);

  ETFG_Deselect_All (prel,TRUE,FALSE);
}

void ETHelp_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  XtManageChild (topform);
  Help_CB (w, client_data, call_data);
}

void ETButt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
	int which, i;
	Boolean_t anycn, anyml, anymn;
        XmString label;

	which = (int) client_data;

	switch (which)
	{
	case 0:
XtManageChild (topform);
		ETFG_Deselect_All (prel,FALSE,TRUE);
return;
		break;
	case 1:
XtManageChild (topform);
		ETFG_Restore (prel);
return;
		break;
	case 2:
	case 3:
		merge_flag=which==3;
		/* menu of filenames */
		file_mode = READ_FILE;
		XtManageChild (topform);
		label=XmStringCreateLocalized("Open Selected File");
		XtVaSetValues (fileb[0],
		  XmNlabelString, label,
		  NULL);
		XmStringFree(label);
		for (i=0; i<3; i++)
		  XmTextFieldSetString (filet[i], "");
		XtManageChild (fileform);
		for (i=0; i<NUM_TEMPLATES; i++) XmListDeselectPos (filel, i);
		break;
	case 4:
		XtManageChild (topform);
		FGDraw_Show_Window (0, 0);
		break;
	case 5:
		/* add (re)write file logic */
		file_mode = WRITE_FILE;
		XtManageChild (topform);
		label=XmStringCreateLocalized("(Re)write this File");
		XtVaSetValues (fileb[0],
		  XmNlabelString, label,
		  NULL);
		XmStringFree(label);
		for (i=0; i<3; i++)
		  XmTextFieldSetString (filet[i], "");
		XtManageChild (fileform);
		break;
	case 6:
		Refresh_Pretests (modified);
		ETFG_Deselect_All (prel,FALSE,FALSE);
		XtUnmanageChild (topform);
		break;
	default:
		break;
	}
}

void ETToggle_FG(Widget w, XtPointer client_data, XtPointer call_data)
{
        int *pos,count,i,j;
        Boolean_t found;

        XmListGetSelectedPos(w,&pos,&count);
        if (!count)
        {
          XtFree ((char *) pos);
          return;
        }
        for (i=0; i<count; i++) if (!fgpossel[1][pos[i]])
                ETFG_Pos_Select(w,pos[i]);
        for (i=1; i<MX_FUNCGROUPS; i++) if (fgpossel[1][i])
        {
                found=FALSE;
                for (j=0; j<count && !found; j++) found=pos[j]==i;
                if (!found) ETFG_Pos_Deselect(w,i);
        }
        XtFree ((char *) pos);
}

void ETToggle_File(Widget w, XtPointer client_data, XtPointer call_data)
{
  int *pos, count, i, j;
  Boolean_t double_clicked;

  double_clicked=(Boolean_t) (int) client_data;

  XmListGetSelectedPos(w,&pos,&count);
  for (i=0; i<count; i++) XmListSelectPos (w, pos[i], FALSE);
  if (count==0)
    {
    XtFree ((char *) pos);
    selected_fname=NULL;
    XmTextFieldSetString (filet[0], "");
    XmTextFieldSetString (filet[1], "");
    XmTextFieldSetString (filet[2], "");
    return;
    }
  selected_fname=tempfname[pos[0]-1];
	strcpy(abbrev_selfname, selected_fname);
	strip_path_ext(abbrev_selfname);
	selected_fname=abbrev_selfname;
  XmTextFieldSetString (filet[0], selected_fname);
  XmTextFieldSetString (filet[1], tempname[pos[0]-1]);
  XmTextFieldSetString (filet[2], tempabbr[pos[0]-1]);
  XtFree ((char *) pos);

  if (double_clicked) ETRW_File_CB (w, (XtPointer) 0, call_data);
}

void ETFill_FGList(Widget w)
{
	XmString item;
	char itemstr[200];
	int i, pos;

	for (i=0; i<MX_FUNCGROUPS; i++) illegal[i]=FALSE;
	for (i=0; i<NUMBER_OF_ILLEGAL_STRUCTURES; i++) illegal[illegal_fg[i]]=TRUE;
	XmListSetAddMode(w,TRUE);
	for (i=pos=1; i<fgend; i++) if (fgname[i]!=NULL)
	{
		if (illegal[i]) sprintf(itemstr,"*ILLEGAL* %d: %s", i, fgname[i]);
		else
		{
			sprintf(itemstr,"%3d: %s",i,fgname[i]);
			if (fgprsv[i]) strcat (itemstr, "[Preservable Substructure]");
		}
		item=XmStringCreate(itemstr,XmSTRING_DEFAULT_CHARSET);
		XmListAddItem(w,item,0);
		XmStringFree(item);
	}
	XmListSetAddMode(w,FALSE);
	XmListSetBottomPos(w,0);
	XmListSetPos(w,1);
}

void ETFill_FileList(Widget w, Boolean_t update_name_abbr)
{
	XmString item;
	char itemstr[200],*local_tempfname[MAX_TEMPLATES], line[128];
	int i, ntemps, len;
	FILE *f;

	XmListDeleteAllItems(w);
        ntemps = PreTemplatesRead (local_tempfname, MAX_TEMPLATES);
        if (ntemps!=NUM_TEMPLATES)
		{
		printf("Error in # templates\n");
		exit(1);
		}

	if (update_name_abbr) for (i=0; i<ntemps; i++)
	{
		f=fopen(local_tempfname[i],"r");
		fgets(line,126,f);
		len=strlen(line);
		free(tempname[i]);
		tempname[i]=(char *)malloc(len);
		while(line[len-1]<' ') line[--len]='\0';
		strcpy(tempname[i],line);
		fgets(line,126,f);
		len=strlen(line);
		free(tempabbr[i]);
		tempabbr[i]=(char *)malloc(len);
		while(line[len-1]<' ') line[--len]='\0';
		strcpy(tempabbr[i],line);
		fclose(f);
	}
	XmListSetAddMode(w,TRUE);
	for (i=0; i<NUM_TEMPLATES; i++)
	{
		strcpy(tempfname[i],local_tempfname[i]);
		sprintf(itemstr,"%s: %s [%s]",tempfname[i],tempname[i],tempabbr[i]);
		item=XmStringCreate(itemstr,XmSTRING_DEFAULT_CHARSET);
		XmListAddItem(w,item,0);
		XmStringFree(item);
	}
	XmListSetAddMode(w,FALSE);
	XmListSetBottomPos(w,0);
	XmListSetPos(w,1);
}

void ETFG_Select(Widget w,int num,Boolean_t initial)
{
	int i,j,pos;
	Boolean_t already_selected;

	for (i=pos=1; i<num; i++) if (fgname[i]!=NULL) pos++;
	already_selected=fgsel[1][i];
	for (j = initial ? 0: 1; j < 2; j++) fgsel[j][i]=fgpossel[j][pos]=TRUE;
	if (!already_selected) /* get around the flaky idiocy of Motif's "support" for lists! (Is this MS-Motif?!?!) */
		XmListSelectPos(w,pos,FALSE);
}

void ETFG_Deselect(Widget w,int num)
{
	int i,j,pos;
	Boolean_t already_deselected;

	for (i=pos=1; i<num; i++) if (fgname[i]!=NULL) pos++;
	already_deselected = !fgsel[1][i];
	fgsel[1][i]=fgpossel[1][pos]=FALSE;
	if (!already_deselected) /* just in case Motif is also flaky and stupid about this! */
		XmListDeselectPos(w,pos);
}

void ETFG_Deselect_All(Widget w,Boolean_t initial,Boolean_t save_first)
{
	int i,j;

	if (initial) for (j=0; j<2; j++)
		for (i=0; i<MX_FUNCGROUPS; i++) fgpossel[j][i]=fgsel[j][i]=FALSE;
	else
		for (i=1; i<MX_FUNCGROUPS; i++)
		{
		if (save_first)
			{
			fgsel[0][i]=fgsel[1][i];
			fgpossel[0][i]=fgpossel[1][i];
			}
		if (fgsel[1][i]) ETFG_Deselect(w,i);
		}
}

void ETFG_Restore(Widget w)
{
	int i;

	ETFG_Deselect_All(w,FALSE,FALSE);
	for (i=1; i<MX_FUNCGROUPS; i++) if (fgsel[0][i]) ETFG_Select (w,i,FALSE);
	else if (fgsel[1][i]) ETFG_Deselect (w,i);
}

void ETFG_Pos_Select(Widget w,int num)
{
	int i,pos;

	for (i=pos=1; pos<=num; i++) if (fgname[i]!=NULL) pos++;
	pos--;
	i--;
	fgsel[1][i]=fgpossel[1][pos]=TRUE;
	XmListSelectPos(w,pos,FALSE);
	XmListSelectPos(w,pos,FALSE);
/* Apparently, the first one doesn't count!  Motif is a kludge! */
}

void ETFG_Pos_Deselect(Widget w,int num)
{
	int i,pos;

	for (i=pos=1; pos<=num; i++) if (fgname[i]!=NULL) pos++;
	pos--;
	i--;
	fgsel[1][i]=fgpossel[1][pos]=FALSE;
	XmListDeselectPos(w,pos);
}

void ETWrite_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  XmString label;
  char message[256];
  Boolean_t overwrite;
  int status,i;

  overwrite=(Boolean_t) (int) client_data;
  if (overwrite)
    {
    if (PreTemplateWrite (selected_fname, tempname[NUM_TEMPLATES], tempabbr[NUM_TEMPLATES], template[NUM_TEMPLATES],
	TRUE)==PTFILE_OK)
      {
      modified=TRUE;
      ETFill_FileList(filel, TRUE);
      }
    else
      {
      sprintf(message,"Write failed for file %s.",tempfname[NUM_TEMPLATES]);
      label=XmStringCreateLocalized (message);
      XtVaSetValues (WriteFailMsg,
        XmNmessageString, label,
        NULL);
      XmStringFree(label);
      XtManageChild (WriteFailMsg);
      free (tempname[NUM_TEMPLATES]);
      free (tempabbr[NUM_TEMPLATES]);
      }
    selected_fname=NULL;
    }
  else
    {
    if ((status=PreTemplateWrite (selected_fname, tempname[NUM_TEMPLATES], tempabbr[NUM_TEMPLATES], template[NUM_TEMPLATES],
	FALSE))==PTFILE_OK)
      {
      for (i=0; i<4; i++) template_complete[i][NUM_TEMPLATES]=FALSE;
      NUM_TEMPLATES++;
/*
      alphabetize_templates ();
*/
      modified=TRUE;
      ETFill_FileList(filel, TRUE);
      selected_fname=NULL;
      }
    else if (status==PTFILE_EXISTS)
      {
      sprintf(message,"A file named %s already exists.  Overwrite this file?",selected_fname);
      label=XmStringCreateLocalized (message);
      XtVaSetValues (OverwriteMsg,
        XmNmessageString, label,
        NULL);
      XmStringFree(label);
      XtManageChild (OverwriteMsg);
      }
    else
      {
      sprintf(message,"Write failed for file %s.",tempfname[NUM_TEMPLATES]);
      label=XmStringCreateLocalized (message);
      XtVaSetValues (WriteFailMsg,
        XmNmessageString, label,
        NULL);
      XmStringFree(label);
      XtManageChild (WriteFailMsg);
      selected_fname=NULL;
      free (tempname[NUM_TEMPLATES]);
      free (tempabbr[NUM_TEMPLATES]);
      }
    }
}

void FileWrite_Button_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  int which;

  XtUnmanageChild (XtParent(w));

  if ((which=(int)client_data)==0) ETWrite_CB (w, (XtPointer) (int) TRUE, call_data);
  else if (which==2)
	{
	XtUnmanageChild (topform);
	XtManageChild (topform);
	XtManageChild (fileform);
	}
}

void ETRW_File_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  FILE *f;
  int fgnum,i;
  char *name,*abbr;
  XmString label;
  Boolean_t something_selected,found;

  if ((int) client_data==1)
    {
    selected_fname=NULL;
    return;
    }
  if (file_mode==WRITE_FILE)
    {
    selected_fname=XmTextFieldGetString(filet[0]);
    strcpy (tempfname[NUM_TEMPLATES],selected_fname);
    if (tempfname[NUM_TEMPLATES][0]=='\0')
	{
        label=XmStringCreateLocalized ("You have forgotten to supply a filename for this template.");
        XtVaSetValues (MissingInfoMsg,
          XmNmessageString, label,
          NULL);
        XmStringFree(label);
	XtManageChild(MissingInfoMsg);
	return;
	}
    XtFree(selected_fname);
    selected_fname=tempfname[NUM_TEMPLATES];
    name=XmTextFieldGetString(filet[1]);
    if (name[0]=='\0')
	{
	XtFree(name);
        label=XmStringCreateLocalized ("You have forgotten to supply a name for this template.");
        XtVaSetValues (MissingInfoMsg,
          XmNmessageString, label,
          NULL);
        XmStringFree(label);
	XtManageChild(MissingInfoMsg);
	return;
	}
    tempname[NUM_TEMPLATES]=(char *)malloc(strlen(name)+1);
    strcpy (tempname[NUM_TEMPLATES],name);
    XtFree(name);
    abbr=XmTextFieldGetString(filet[2]);
    if (abbr[0]=='\0')
	{
    	XtFree(abbr);
        label=XmStringCreateLocalized ("You have forgotten to supply an abbreviation for this template.");
        XtVaSetValues (MissingInfoMsg,
          XmNmessageString, label,
          NULL);
        XmStringFree(label);
	XtManageChild(MissingInfoMsg);
	return;
	}
    tempabbr[NUM_TEMPLATES]=(char *)malloc(strlen(abbr)+1);
    strcpy (tempabbr[NUM_TEMPLATES],abbr);
    XtFree(abbr);
    for (i=0, something_selected=FALSE; i<MX_FUNCGROUPS; i++) if (template[NUM_TEMPLATES][i]=fgsel[1][i]) something_selected=TRUE;
    if (!something_selected)
	{
    	XtFree(abbr);
        label=XmStringCreateLocalized ("You have forgotten to select at least one functional group.");
        XtVaSetValues (MissingInfoMsg,
          XmNmessageString, label,
          NULL);
        XmStringFree(label);
	XtManageChild(MissingInfoMsg);
	return;
	}
    ETWrite_CB (w, (XtPointer) (int) FALSE, call_data);
    }
  else
    {
    if (selected_fname==NULL) return;
    if (!merge_flag) ETFG_Deselect_All (prel, FALSE, TRUE);
    for (i=0, found=FALSE; i<NUM_TEMPLATES; i++) if (strstr(tempfname[i],selected_fname) != NULL)
      {
      found=TRUE;
      selected_fname=tempfname[i];
      }
    f=fopen (selected_fname,"r");
    while(getc(f)!='\n');
    while(getc(f)!='\n');
    while(fscanf(f,"%d",&fgnum)==1)
	ETFG_Select (prel, fgnum, FALSE);
    fclose(f);
    selected_fname=NULL;
    }
}

/*
void alphabetize_templates ()
{
  int i, j, k, tmp;
  char *tmpstrptr, tmpstr[128], strp[2][128];
  Boolean_t sorted;

  for (i=0, sorted=FALSE; i<NUM_TEMPLATES-1 && !sorted; i++)
	{
	sorted=TRUE;
	for (j=NUM_TEMPLATES-1; j>i; j--)
		{
		k=j-1;
		strcpy(strp[0],tempfname[k]);
		strcpy(strp[1],tempfname[j]);
		strip_path_ext(strp[0]);
		strip_path_ext(strp[1]);
		if (strcmp(strp[0],strp[1]) > 0)
			{
			sorted=FALSE;
			strcpy (tmpstr, tempfname[k]);
			strcpy (tempfname[k], tempfname[j]);
			strcpy (tempfname[j], tmpstr);
			tmpstrptr=tempname[k];
			tempname[k]=tempname[j];
			tempname[j]=tmpstrptr;
			tmpstrptr=tempabbr[k];
			tempabbr[k]=tempabbr[j];
			tempabbr[j]=tmpstrptr;
			}
		}
	}
}
*/

void strip_path_ext(char *s)
{
  char *p;

  p=strstr(s,".");
  if (p!=NULL) *p=0;
  while ((p=strstr(s,"/"))!=NULL) strcpy(s,p+1);
}
