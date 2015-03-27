/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:                     FGDRAW.C
*
*    Displays drawings of all attributes in the functional group table.
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
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include <Xm/PushBG.h>
#include <Xm/ScrollBar.h>
#include <Xm/ScrolledW.h>

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#include "synchem.h"
#include "synio.h"
#include "utl.h"
#include "debug.h"
#include "app_resrc.h"

#ifndef _H_RCB_
#include "rcb.h"
#endif

#ifndef _H_ISAM_
#include "isam.h"
#endif

#ifndef _H_FUNCGROUP_FILE_
#include "funcgroup_file.h"
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

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_DSP_
#include "dsp.h"
#endif

#ifndef _H_EXTERN_
#include "extern.h"
#endif

#define FGFILE FCB_SEQDIR_FNGP ("/fgdata.isam")
#define PMFILES(n) ((n) == 0 ? FCB_SEQDIR_FNGP ("/fgdata.pixmap") : FCB_SEQDIR_FNGP ("/atable.pixmap"))
#define BBMFILE(n) ((n) == 0 ? FCB_SEQDIR_FNGP ("/fgdata.binarymap") : FCB_SEQDIR_FNGP ("/atable.binarymap"))
#define POSFILE(n) ((n) == 0 ? FCB_SEQDIR_FNGP ("/fgdata.vpos") : FCB_SEQDIR_FNGP ("/atable.vpos"))
#define FGDRW_DA_PXMP_WD 3000
#define FGDRW_DA_PXMP_HT 1000
#define NBMAPS(n) ((n) == 0 ? 67 : (n) == 1 ? 7 : 4)

static struct
{
  Boolean_t fg_exists;
  String_t  fg_name;
  String_t  fg_slings;
} fgdraw_info[1000];

static Widget topform[3], drawarea[3], vsb[3], hsb[3];
static Pixmap pixmap[NBMAPS(0)];
static Display *display;
static GC gc;
static FILE *bmdata[3];
static int xoffset[3] = {0, 0, 0}, yoffset[3] = {0, 0, 0};
static Dimension wwidth[3], wheight[3];
static Boolean_t need_initial_resize[3] = {TRUE, TRUE, TRUE};

char *atable[][2] = {
  {"C=O", "Carbonyl"},
  {"CH-1C=O", "Enolizable Carbonyl"},
  {"CH-1C*N", "Acidic Nitrile"},
  {"C<C", "Aromatic Ring"},
  {"C=C", "Alkene"},
  {"C*C", "Alkyne"},
  {"C*N", "Nitrile"},
  {"N=O-1=O", "Nitro"},
  {"S=O-1=O-1($1)", "Sulfone"},
  {"H", "Hydrogen"},
  {"C=O-1OC", "Ester (Acyl)"},
  {"C=O-1NC-1C", "Tertiary Amide"},
  {"C=CC=O", "alpha,beta-Unsaturated Carbonyl"},
  {"COC=O", "Ester (Alkyl)"},
  {"OC=O-1C", "O Ester"},
  {"F", "Fluorine"},
  {"(CL)", "Chlorine"},
  {"(BR)", "Bromine"},
  {"I", "Iodine"},
  {"S", "Sulfur"},
  {"OH", "Hydroxyl"},
  {"OC($2)-1($4)-1($6)", "Ether"},
  {"NH-1H", "Primary Amine"},
  {"NH-1C", "Secondary Amine"},
  {"NC-1C", "Tertiary Amine"},
  {"C=O-1OH", "Carboxylic Acid"},
  {"CH-1H-1H", "Methyl"},
  {"CH-1H-1C", "Methylene"},
  {"CC-1H-1C", "Methinyl"},
  {"NC-1C-1C", "Quaternary Ammonium"},
  {"SC-1C", "Sulfonium"},
  {"OS=O-1=O-1($1)", "Sulfonate Ester"},
  {"OC<C", "Phenoxy"},
  {"OC($2)-1($4)-1($6)", "Alkoxy"},
  {"ON=O-1=O", "Nitrate"},
  {"OP=O-1($1)-1($2)", "Phosphate"},
  {"OB($1)-1($2)", "Borate"},
  {"C=CCH-1($2)-1($4)", "Vinyl with gamma-Hydrogen"},
  {"S=O", "Sulfoxide"},
  {"P=O", "Phosphorus Oxides"},
  {"SC<C", "Thiophenol"},
  {"SH", "Mercaptan"},
  {"SC", "Sulfide"},
  {"P($2)-1($4)-1($6)", "Phosphine"},
  {"P=O-1OC-2OC", "Phosphonate Ester at P"},
  {"(SI)C-1C-1C", "Trialkylsilyl"},
  {"C<N", "alpha-Pyridyl"},
  {"C<C<N", "beta-Pyridyl"},
  {"C<C<C<N", "gamma-Pyridyl"},
  {"C=CC>CO/0", "alpha-Furyl"},
  {"C=CC>CS/0", "alpha-Thienyl"},
  {"C=CC>CN/0", "alpha-Pyrryl"},
  {"CC>C>($3)-1OC($1)-1=/0", "beta-Furyl, alpha,alpha'-Disubstituted"},
  {"CC>C>($3)-1SC($1)-1=/0", "beta-Thienyl, alpha,alpha'-Disubstituted"},
  {"CC>C>($3)-1NC($1)-1=/0", "beta-Pyrryl, alpha,alpha'-Disubstituted"},
  {"C<C<C<COH-2<C<C</0", "pata-Hydroxyphenyl"},
  {"C<C<C<CN-1<C<C</0", "para-Aminophenyl"},
  {"C<COH-2<C<C<C<C</0", "ortho-Hydroxyphenyl"},
  {"C<CN-1<C<C<C<C</0", "ortho-Aminophenyl"},
  {"C(#j)-1(#j)-1(#j)", "Trihalomethyl"},
  {"CC-1C-1C-1C", "Quaternary Carbon"},
  {"COC/0", "Oxirane"},
  {"NC>CC>C/0", "1-Pyrryl"},
  {"NC>NC>C/0", "1-Imidazolyl"},
  {"NN>CC>C/0", "1-Pyrazolyl"},
  {"CH-1H", "Methyl or Methylene"},
  {NULL, NULL}};

void moldraw(Dsp_Molecule_t *,Tsd_t *,Display *,Window,GC,int,int,int,int,U32_t *,U32_t *,Boolean_t);
U32_t molroot_and_syntheme(U32_t,U32_t *,U32_t *);
void FGDraw_Init (ScreenAttr_t *, Boolean_t);
Boolean_t FGDraw_Data (U16_t, char **, char **);
void FGDraw_Window_Create (Widget, ScreenAttr_t *);
void FGDraw_Dismiss_CB (Widget, XtPointer, XtPointer);
void FGDraw_Draw_CB (Widget, XtPointer, XtPointer);
void FGDraw_Scroll_CB (Widget, XtPointer, XtPointer);
void FGDraw_Resize_CB (Widget, XtPointer, XtPointer);
void FGDraw_Draw_Molecules (int);
void FGDraw_Draw_Molecule (char *, int, int, int, int *, int *);
void FGDraw_Create_Binary_Bitmap (int);
void FGDraw_Redraw (int, int, int, int, int);
void FGDraw_Show_Window (int, int);

void FGDraw_Show_Window (int which, int ypos)
{
  XtManageChild (topform[which]);
  if (which == 2)
    XtVaSetValues (vsb[which],
      XmNvalue, ypos < 75 * FGDRW_DA_PXMP_HT / 20 - wheight[which] ? ypos :
        75 * FGDRW_DA_PXMP_HT / 20 - wheight[which],
      NULL);
  else
    XtVaSetValues (vsb[which],
      XmNvalue, ypos < NBMAPS (which) * FGDRW_DA_PXMP_HT - wheight[which] ? ypos :
        NBMAPS (which) * FGDRW_DA_PXMP_HT - wheight[which],
      NULL);
/*
  yoffset[which] = ypos;
*/
  XtVaGetValues (vsb[which],
    XmNvalue, yoffset + which,
    NULL);
  FGDraw_Redraw (0, 0, wwidth[which], wheight[which], which);
}

void FGDraw_Create_Binary_Bitmap (int which)
{
  FILE *in,*out;
  char line[81],*l,hex[10],fname[128];
  int width,height,x,y,val,i;

  out=fopen(BBMFILE(which),"w");
  for (i=0; i<NBMAPS(which); i++)
    {
    sprintf(fname,"%s%d.xbm",PMFILES(which),i);
    in=fopen(fname,"r");
    fgets(line,80,in);
    l=strstr(line,"width");
    sscanf(l,"width %d",&width);
    fgets(line,80,in);
    l=strstr(line,"height");
    sscanf(l,"height %d",&height);
    fgets(line,80,in);
    x=y=0;
    while(fscanf(in,"%s",hex)==1)
      {
      sscanf(hex,"0x%x",&val);
      putc(val,out);
      x+=8;
      if (x>=width)
        {
        x=0;
        y++;
        }
      }
    fclose(in);
    }
  fclose(out);
}

void FGDraw_Init (ScreenAttr_t *scra_p, Boolean_t write_pixmaps)
{
  char              fg_name[256], tfg_name[256], *sling, *sl, filename[128];
  int               fg_num, fgrec_num, fglist_pos, name_inx, sl_inx, i;
  FuncGrp_Record_t *fgrec;
  Isam_Control_t   *fgfile;

  for (fg_num = 0; fg_num<1000; fg_num++) fgdraw_info[fg_num].fg_exists = FALSE;

  fgfile = (Isam_Control_t *) malloc (ISAMCONTROLSIZE);
  if (fgfile == NULL) IO_Exit_Error (R_AVL, X_SYSCALL,
    "Unable to allocate memory for Isam Control Block.");
  strcpy (IO_FileName_Get (Isam_File_Get (fgfile)), FGFILE);
  Isam_Open (fgfile, ISAM_TYPE_FGINFO, ISAM_OPEN_READ);

  fgrec_num = fglist_pos = 1;
  while((fgrec = FuncGrp_Rec_Read (fgrec_num, fgfile)) != NULL
    && (fg_num = FuncGrp_Head_FGNum_Get (FuncGrp_Rec_Head_Get (fgrec))) <= 1000 /*40*/)
    {
    for (name_inx = 0;
         name_inx < FuncGrp_Head_NumNames_Get (FuncGrp_Rec_Head_Get (fgrec));
         name_inx++, fglist_pos++)
      {
      if (name_inx == 0)
        {
        fgdraw_info[fg_num].fg_exists = TRUE;
        sprintf(fg_name, "%2d: %s", fg_num,
          String_Value_Get (FuncGrp_Rec_Name_Get (fgrec, name_inx)));
        }
      else
        {
        sprintf(tfg_name, ", %s",
          String_Value_Get (FuncGrp_Rec_Name_Get (fgrec, name_inx)));
        strcat(fg_name, tfg_name);
        }
      if (name_inx == 0)
        {
        sling =
          (char *) malloc
          (100 * FuncGrp_Head_NumSlings_Get (FuncGrp_Rec_Head_Get(fgrec)));
        sling[0]=0;
        for (sl_inx = 0;
             sl_inx < FuncGrp_Head_NumSlings_Get (FuncGrp_Rec_Head_Get(fgrec));
             sl_inx++)
          {
          strcat(sling, Sling_Name_Get (FuncGrp_Rec_Sling_Get (fgrec, sl_inx)));
          strcat(sling,"\\");
          }
        }
      }
    fgdraw_info[fg_num].fg_name = String_Create ((const char *) fg_name, 0);
    fgdraw_info[fg_num].fg_slings = String_Create ((const char *) sling, 0);

    fgrec_num++;
    }

  Isam_Close (fgfile);
  free (fgfile);

  /*  Create a GC and Pixmap(s) for the drawing area.  */
  display = Screen_TDisplay_Get (scra_p);
  gc = XCreateGC (display, Screen_RootWin_Get (scra_p), 0, NULL);
  XSetBackground (display, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
  XSetForeground (display, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
  XSetFillStyle (display, gc, FillSolid);
  XSetLineAttributes (display, gc, 1, LineSolid, CapButt, JoinMiter);
  XSetFont (display, gc, SynAppR_IthFont_Get (&GSynAppR,
    SAR_FONTSTRCTS_NRML)->fid);

  if (write_pixmaps)
    {
    XSetForeground (display, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
    for (i = 0; i < 2; i++)
      {
      pixmap[i] = XCreatePixmap (display,
        Screen_RootWin_Get (scra_p), FGDRW_DA_PXMP_WD, FGDRW_DA_PXMP_HT,
        Screen_Depth_Get (scra_p));

      XFillRectangle (display, pixmap[i], gc, 0, 0, FGDRW_DA_PXMP_WD, FGDRW_DA_PXMP_HT);
      }

    XSetForeground (display, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
    }

  else
    {
    bmdata[0] = fopen (BBMFILE(0), "r");
    bmdata[1] = fopen (BBMFILE(1), "r");
    bmdata[2] = bmdata[0];
    }
}

Boolean_t FGDraw_Data (U16_t fgnum, char **name, char **slings)
{
  if (fgnum > 999 || !fgdraw_info[fgnum].fg_exists) return (FALSE);
  *name = (char *) malloc (String_Alloc_Get (fgdraw_info[fgnum].fg_name));
  strcpy (*name, String_Value_Get (fgdraw_info[fgnum].fg_name));
  *slings = (char *) malloc (String_Alloc_Get (fgdraw_info[fgnum].fg_slings));
  strcpy (*slings, String_Value_Get (fgdraw_info[fgnum].fg_slings));
  return (TRUE);
}

void FGDraw_Window_Create (Widget top_level, ScreenAttr_t *scra_p)
{
  Widget   dwdismiss[3], drawwin[3];
  XmString lbl_str;
  int      i;

  wwidth[0] = wwidth[1] = wwidth[2] = 400;
  wheight[0] = wheight[1] = wheight[2] = 775;

  topform[0] = XmCreateFormDialog (top_level, "FGD_Form", NULL, 0);

  lbl_str = XmStringCreateLocalized ("Functional Group Display");

  XtVaSetValues (topform[0],
    XmNdialogTitle, lbl_str,
    XmNwidth,wwidth[0],
    XmNheight,800,
    XmNshadowType,       XmSHADOW_IN,
    XmNshadowThickness,  3,
    XmNmarginWidth,      AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginHeight,     AppDim_MargFmFr_Get (&GAppDim),
    XmNfractionBase,     800,
    NULL);

  XmStringFree (lbl_str);

  topform[1] = XmCreateFormDialog (top_level, "ATD_Form", NULL, 0);

  lbl_str = XmStringCreateLocalized ("Directed Attribute Display");

  XtVaSetValues (topform[1],
    XmNdialogTitle, lbl_str,
    XmNwidth,wwidth[1],
    XmNheight,800,
    XmNshadowType,       XmSHADOW_IN,
    XmNshadowThickness,  3,
    XmNmarginWidth,      AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginHeight,     AppDim_MargFmFr_Get (&GAppDim),
    XmNfractionBase,     800,
    NULL);

  XmStringFree (lbl_str);

  topform[2] = XmCreateFormDialog (top_level, "SyD_Form", NULL, 0);

  lbl_str = XmStringCreateLocalized ("Chapter Syntheme Display");

  XtVaSetValues (topform[2],
    XmNdialogTitle, lbl_str,
    XmNwidth,wwidth[2],
    XmNheight,800,
    XmNshadowType,       XmSHADOW_IN,
    XmNshadowThickness,  3,
    XmNmarginWidth,      AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginHeight,     AppDim_MargFmFr_Get (&GAppDim),
    XmNfractionBase,     800,
    NULL);

  XmStringFree (lbl_str);

  drawwin[0] = XtVaCreateManagedWidget ("FGD_SWin",
    xmScrolledWindowWidgetClass, topform[0],
    XmNscrollingPolicy,          XmAPPLICATION_DEFINED,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomPosition, wheight[0],
    XmNbottomAttachment, XmATTACH_POSITION,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  drawwin[1] = XtVaCreateManagedWidget ("ATD_SWin",
    xmScrolledWindowWidgetClass, topform[1],
    XmNscrollingPolicy,          XmAPPLICATION_DEFINED,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomPosition, wheight[1],
    XmNbottomAttachment, XmATTACH_POSITION,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  drawwin[2] = XtVaCreateManagedWidget ("FGD_SWin",
    xmScrolledWindowWidgetClass, topform[2],
    XmNscrollingPolicy,          XmAPPLICATION_DEFINED,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomPosition, wheight[2],
    XmNbottomAttachment, XmATTACH_POSITION,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  vsb[0]=XtVaCreateManagedWidget ("vsb0", xmScrollBarWidgetClass, drawwin[0],
    XmNorientation, XmVERTICAL,
    XmNmaximum, NBMAPS(0)*FGDRW_DA_PXMP_HT,
    XmNsliderSize, /* NBMAPS(0)*FGDRW_DA_PXMP_HT/ */ wheight[0],
    XmNpageIncrement, /* NBMAPS(0)*FGDRW_DA_PXMP_HT/wheight - */ wheight[0],
    NULL);

  vsb[1]=XtVaCreateManagedWidget ("vsb1", xmScrollBarWidgetClass, drawwin[1],
    XmNorientation, XmVERTICAL,
    XmNmaximum, NBMAPS(1)*FGDRW_DA_PXMP_HT,
    XmNsliderSize, /* NBMAPS(0)*FGDRW_DA_PXMP_HT/ */ wheight[1],
    XmNpageIncrement, /* NBMAPS(0)*FGDRW_DA_PXMP_HT/wheight - */ wheight[1],
    NULL);

  vsb[2]=XtVaCreateManagedWidget ("vsb2", xmScrollBarWidgetClass, drawwin[2],
    XmNorientation, XmVERTICAL,
    XmNmaximum, 75*FGDRW_DA_PXMP_HT/20,
    XmNsliderSize, /* NBMAPS(2)*FGDRW_DA_PXMP_HT/ */ wheight[2],
    XmNpageIncrement, /* NBMAPS(2)*FGDRW_DA_PXMP_HT/wheight - */ wheight[2],
    NULL);

  hsb[0]=XtVaCreateManagedWidget ("hsb0", xmScrollBarWidgetClass, drawwin[0],
    XmNorientation, XmHORIZONTAL,
    XmNmaximum, FGDRW_DA_PXMP_WD,
    XmNsliderSize, /* FGDRW_DA_PXMP_WD/ */ wwidth[0],
    XmNpageIncrement, /* FGDRW_DA_PXMP_WD/wwidth - */ wwidth[0],
    NULL);

  hsb[1]=XtVaCreateManagedWidget ("hsb1", xmScrollBarWidgetClass, drawwin[1],
    XmNorientation, XmHORIZONTAL,
    XmNmaximum, FGDRW_DA_PXMP_WD,
    XmNsliderSize, /* FGDRW_DA_PXMP_WD/ */ wwidth[1],
    XmNpageIncrement, /* FGDRW_DA_PXMP_WD/wwidth - */ wwidth[1],
    NULL);

  hsb[2]=XtVaCreateManagedWidget ("hsb2", xmScrollBarWidgetClass, drawwin[2],
    XmNorientation, XmHORIZONTAL,
    XmNmaximum, FGDRW_DA_PXMP_WD,
    XmNsliderSize, /* FGDRW_DA_PXMP_WD/ */ wwidth[2],
    XmNpageIncrement, /* FGDRW_DA_PXMP_WD/wwidth - */ wwidth[2],
    NULL);

  XtAddCallback (vsb[0], XmNvalueChangedCallback, FGDraw_Scroll_CB, (XtPointer) 'V');
  XtAddCallback (vsb[1], XmNvalueChangedCallback, FGDraw_Scroll_CB, (XtPointer) 'v');
  XtAddCallback (vsb[2], XmNvalueChangedCallback, FGDraw_Scroll_CB, (XtPointer) 'Y');
  XtAddCallback (hsb[0], XmNvalueChangedCallback, FGDraw_Scroll_CB, (XtPointer) 'H');
  XtAddCallback (hsb[1], XmNvalueChangedCallback, FGDraw_Scroll_CB, (XtPointer) 'h');
  XtAddCallback (hsb[2], XmNvalueChangedCallback, FGDraw_Scroll_CB, (XtPointer) 'X');
  XtAddCallback (vsb[0], XmNdragCallback, FGDraw_Scroll_CB, (XtPointer) 'V');
  XtAddCallback (vsb[1], XmNdragCallback, FGDraw_Scroll_CB, (XtPointer) 'v');
  XtAddCallback (vsb[2], XmNdragCallback, FGDraw_Scroll_CB, (XtPointer) 'Y');
  XtAddCallback (hsb[0], XmNdragCallback, FGDraw_Scroll_CB, (XtPointer) 'H');
  XtAddCallback (hsb[1], XmNdragCallback, FGDraw_Scroll_CB, (XtPointer) 'h');
  XtAddCallback (hsb[2], XmNdragCallback, FGDraw_Scroll_CB, (XtPointer) 'X');

  drawarea[0] = XtVaCreateManagedWidget ("FGD_DA",
    xmDrawingAreaWidgetClass, drawwin[0],
    XmNbackground,        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground,        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNheight,            2*FGDRW_DA_PXMP_HT,
    XmNwidth,             FGDRW_DA_PXMP_WD,
    NULL);
  XtAddCallback (drawarea[0], XmNexposeCallback, FGDraw_Draw_CB, (XtPointer) 0);
  XtAddCallback (drawarea[0], XmNresizeCallback, FGDraw_Resize_CB, (XtPointer) 0);

  drawarea[1] = XtVaCreateManagedWidget ("ATD_DA",
    xmDrawingAreaWidgetClass, drawwin[1],
    XmNbackground,        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground,        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNheight,            2*FGDRW_DA_PXMP_HT,
    XmNwidth,             FGDRW_DA_PXMP_WD,
    NULL);
  XtAddCallback (drawarea[1], XmNexposeCallback, FGDraw_Draw_CB, (XtPointer) 1);
  XtAddCallback (drawarea[1], XmNresizeCallback, FGDraw_Resize_CB, (XtPointer) 1);

  drawarea[2] = XtVaCreateManagedWidget ("SyD_DA",
    xmDrawingAreaWidgetClass, drawwin[2],
    XmNbackground,        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground,        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNheight,            2*FGDRW_DA_PXMP_HT,
    XmNwidth,             FGDRW_DA_PXMP_WD,
    NULL);
  XtAddCallback (drawarea[2], XmNexposeCallback, FGDraw_Draw_CB, (XtPointer) 2);
  XtAddCallback (drawarea[2], XmNresizeCallback, FGDraw_Resize_CB, (XtPointer) 2);

  lbl_str = XmStringCreateLocalized ("Dismiss");

  dwdismiss[0] =  XtVaCreateManagedWidget ("FGD_PB",
    xmPushButtonGadgetClass, topform[0],
    XmNlabelString, lbl_str,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopOffset, 0,
    XmNtopWidget, drawwin[0],
    XmNbottomAttachment, XmATTACH_FORM,
    XmNbottomOffset, 0,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNleftPosition, 325,
    XmNrightAttachment, XmATTACH_POSITION,
    XmNrightPosition, 475,
    NULL);
  XtAddCallback (dwdismiss[0], XmNactivateCallback, FGDraw_Dismiss_CB, (XtPointer) 0);

  dwdismiss[1] =  XtVaCreateManagedWidget ("ATD_PB",
    xmPushButtonGadgetClass, topform[1],
    XmNlabelString, lbl_str,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopOffset, 0,
    XmNtopWidget, drawwin[1],
    XmNbottomAttachment, XmATTACH_FORM,
    XmNbottomOffset, 0,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNleftPosition, 325,
    XmNrightAttachment, XmATTACH_POSITION,
    XmNrightPosition, 475,
    NULL);
  XtAddCallback (dwdismiss[1], XmNactivateCallback, FGDraw_Dismiss_CB, (XtPointer) 1);

  dwdismiss[2] =  XtVaCreateManagedWidget ("SyD_PB",
    xmPushButtonGadgetClass, topform[2],
    XmNlabelString, lbl_str,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopOffset, 0,
    XmNtopWidget, drawwin[2],
    XmNbottomAttachment, XmATTACH_FORM,
    XmNbottomOffset, 0,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNleftPosition, 325,
    XmNrightAttachment, XmATTACH_POSITION,
    XmNrightPosition, 475,
    NULL);
  XtAddCallback (dwdismiss[2], XmNactivateCallback, FGDraw_Dismiss_CB, (XtPointer) 2);

  XmStringFree (lbl_str);
}

void FGDraw_Scroll_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  int which;
  XmScrollBarCallbackStruct *cbs = (XmScrollBarCallbackStruct *) call_data;

  which = 0;

  switch ((int) client_data)
    {
  case 'Y':
    which++;
  case 'v':
    which++;
  case 'V':
    yoffset[which] = cbs->value;
    break;
  case 'X':
    which++;
  case 'h':
    which++;
  case 'H':
    xoffset[which] = cbs->value;
    break;
    }
  FGDraw_Redraw (0, 0, wwidth[which], wheight[which], which);
}

void FGDraw_Resize_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  Widget workw, hsb, vsb;
  int which;

  which = (int) client_data;

  XtVaGetValues (XtParent (w),
    XmNworkWindow, &workw,
    XmNverticalScrollBar, &vsb,
    XmNhorizontalScrollBar, &hsb,
    NULL);
printf("kka: which=%d, vsb=%d, hsb=%d\n", which, vsb, hsb);
  XtVaGetValues (workw,
    XmNwidth, wwidth+which,
    XmNheight, wheight+which,
    NULL);
printf("kka: wheight[which]=%d, wwidth[which]=%d\n", wheight[which],wwidth[which]);
  XtVaSetValues (vsb,
    XmNsliderSize, /* NBMAPS(which)*FGDRW_DA_PXMP_HT/ */ wheight[which],
    XmNpageIncrement, /* NBMAPS(which)*FGDRW_DA_PXMP_HT/wheight - */ wheight[which],
    NULL);

  XtVaSetValues (hsb,
    XmNsliderSize, /* FGDRW_DA_PXMP_WD/ */ wwidth[which],
    XmNpageIncrement, /* FGDRW_DA_PXMP_WD/wwidth - */ wwidth[which],
    NULL);
}

void FGDraw_Dismiss_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild (topform[(int) client_data]);
}

void FGDraw_Draw_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  int pnum1,pnum2,x1,x2,y1,y2,i,width,height,xh,yh,wystart,pystart,x,y,byte,dx, which;
  char filename[128];

  XmDrawingAreaCallbackStruct *cbs = (XmDrawingAreaCallbackStruct *) call_data;

  which = (int) client_data;

  if (need_initial_resize[which])
    {
    FGDraw_Resize_CB (w, (XtPointer) which, (XtPointer) NULL);
    need_initial_resize[which] = FALSE;
    }

  FGDraw_Redraw (cbs->event->xexpose.x, cbs->event->xexpose.y, cbs->event->xexpose.width, cbs->event->xexpose.height, which);
}

void FGDraw_Redraw (int ix, int iy, int width, int height, int which)
{
  int x, y, bmx, bmy, x1, x2, y1, y2, dx, byte;

  XClearArea (display, XtWindow (drawarea[which]), ix, iy, width, height, False);

  x1 = ix;
  x2 = ix + width - 1;
  y1 = iy;
  y2 = iy + height - 1;

  bmx = x1 + xoffset[which];

  for (y=y1; y<=y2; y++)
    {
    bmy = y + yoffset[which];
    fseek (bmdata[which], (FGDRW_DA_PXMP_WD + 7) / 8 * bmy + bmx / 8, SEEK_SET);
    for (x=x1/8; x<=(x2 + 7) / 8; x++)
      {
      byte=getc(bmdata[which]);
      for (dx=0; dx<8; dx++)
        {
        if ((byte&1)==0) XDrawPoint(display,XtWindow(drawarea[which]),gc,8*x+dx,y);
        byte>>=1;
        }
      }
    }
}

void FGDraw_Draw_Molecules (int which_table)
{
  int xpos, ypos, actual_ypos, width, height, maxheight, pnum, which;
  U16_t fg_num;
  char *name, *sling, *sl, filename[128], atabname[64];
  FILE *posfile;

  posfile = fopen (POSFILE (which_table), "w");

  XSetForeground (display, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
  XFillRectangle (display, pixmap[0], gc, 0, 0, FGDRW_DA_PXMP_WD, FGDRW_DA_PXMP_HT);
  XFillRectangle (display, pixmap[1], gc, 0, 0, FGDRW_DA_PXMP_WD, FGDRW_DA_PXMP_HT);
  XSetForeground (display, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));

  if (which_table == 0) for (fg_num = pnum = which = actual_ypos = 0, ypos = 15; fg_num < 1000; fg_num++)
    {
    if (FGDraw_Data (fg_num, &name, &sling))
      {
      fprintf (posfile, "%d\n", actual_ypos);
      XDrawLine (display, pixmap[which], gc, 0, ypos - 16, 250, ypos - 16);
      XDrawLine (display, pixmap[1 - which], gc, 0, ypos - FGDRW_DA_PXMP_HT - 16, 250, ypos - FGDRW_DA_PXMP_HT - 16);
      XDrawString (display, pixmap[which], gc, 10, ypos, name, strlen (name));
      XDrawString (display, pixmap[1 - which], gc, 10, ypos - FGDRW_DA_PXMP_HT, name, strlen (name));
      free (name);
      xpos = 20;
      ypos += 10;
      actual_ypos += 10;
      maxheight = 0;
  
      do
        {
        sl = strstr (sling, "\\");
        *sl++ = '\0';
        FGDraw_Draw_Molecule (sling, xpos, ypos, which, &width, &height);
        if (xpos + width > FGDRW_DA_PXMP_WD)
          {
          XSetForeground (display, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
          XFillRectangle (display, pixmap[which], gc, xpos, ypos, width, height);
          XFillRectangle (display, pixmap[1 - which], gc, xpos, ypos - FGDRW_DA_PXMP_HT, width, height);
          XSetForeground (display, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
          xpos = 20;
          ypos += maxheight + 20;
          actual_ypos += maxheight + 20;
          maxheight = 0;
          FGDraw_Draw_Molecule (sling, xpos, ypos, which, &width, &height);
         }
        if (height > maxheight) maxheight = height;
        xpos += width + 25;
        strcpy (sling, sl);
        }
      while (sling[0] != '\0');
      ypos += maxheight + 20;
      actual_ypos += maxheight + 20;
  
      if (ypos > FGDRW_DA_PXMP_HT + 15)
        {
        sprintf (filename, "%s%d.xbm", PMFILES(which_table), pnum);
        XFlush (display);
        XWriteBitmapFile (display, filename, pixmap[which], FGDRW_DA_PXMP_WD, FGDRW_DA_PXMP_HT, -1, -1);
  
        XSetForeground (display, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
        XFillRectangle (display, pixmap[which], gc, 0, 0, FGDRW_DA_PXMP_WD, FGDRW_DA_PXMP_HT);
  
        XSetForeground (display, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
  
        which = 1 - which;
        ypos -= FGDRW_DA_PXMP_HT;
        pnum++;
        }
      free (sling);
      }
    }
  else for (fg_num = pnum = which = actual_ypos = 0, ypos = 15; atable[fg_num][0] != NULL; fg_num++)
    {
    fprintf (posfile, "%d\n", actual_ypos);
    XDrawLine (display, pixmap[which], gc, 0, ypos - 16, 250, ypos - 16);
    XDrawLine (display, pixmap[1 - which], gc, 0, ypos - FGDRW_DA_PXMP_HT - 16, 250, ypos - FGDRW_DA_PXMP_HT - 16);
    sprintf (atabname, "%2d: %s", fg_num + 1, atable[fg_num][1]);
    XDrawString (display, pixmap[which], gc, 10, ypos, atabname, strlen (atabname));
    XDrawString (display, pixmap[1 - which], gc, 10, ypos - FGDRW_DA_PXMP_HT, atabname, strlen (atabname));
    xpos = 20;
    ypos += 10;
    actual_ypos += 10;
    FGDraw_Draw_Molecule (atable[fg_num][0], xpos, ypos, which, &width, &height);
    ypos += height + 20;
    actual_ypos += height + 20;
    if (ypos > FGDRW_DA_PXMP_HT + 15)
      {
      sprintf (filename, "%s%d.xbm", PMFILES(which_table), pnum);
      XFlush (display);
      XWriteBitmapFile (display, filename, pixmap[which], FGDRW_DA_PXMP_WD, FGDRW_DA_PXMP_HT, -1, -1);

      XSetForeground (display, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
      XFillRectangle (display, pixmap[which], gc, 0, 0, FGDRW_DA_PXMP_WD, FGDRW_DA_PXMP_HT);

      XSetForeground (display, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));

      which = 1 - which;
      ypos -= FGDRW_DA_PXMP_HT;
      pnum++;
      }
    }

  sprintf (filename, "%s%d.xbm", PMFILES(which_table), pnum);
  XFlush (display);
  XWriteBitmapFile (display, filename, pixmap[which], FGDRW_DA_PXMP_WD, FGDRW_DA_PXMP_HT, -1, -1);

  fclose (posfile);
}

void FGDraw_Draw_Molecule (char *sling, int xpos, int ypos, int which, int *width, int *height)
{
  String_t string;
  Sling_t sl;
  Tsd_t *tsd;
  Xtr_t *xtr;
  Dsp_Molecule_t *dsp;
  U32_t roots[2],synthemes[2];
  int i;

  string = String_Create ((const char *) sling, 0);
  sl = String2Sling (string);
  String_Destroy (string);
  tsd = Sling2Tsd (sl);
  Sling_Destroy (sl);
  xtr = Tsd2Xtr (tsd);
  dsp = Xtr2Dsp (xtr);
  Xtr_Destroy (xtr);
  dsp_Shelley (dsp);
  *width = dsp->molw+20;
  *height = dsp->molh+20;
  for (i=0; i<dsp->natms; i++)
  {
    dsp->atoms[i].x+=10;
    dsp->atoms[i].y+=10;
  }
  roots[0]=0;
  roots[1]=REACT_NODE_INVALID;
  synthemes[0]=1;
  moldraw(dsp,tsd,display,(Window)pixmap[which],gc,xpos,ypos,xpos+*width,ypos+*height,roots,synthemes,FALSE);
  ypos-=FGDRW_DA_PXMP_HT;
  moldraw(dsp,tsd,display,(Window)pixmap[1-which],gc,xpos,ypos,xpos+*width,ypos+*height,roots,synthemes,FALSE);
  Tsd_Destroy (tsd);
  free_Molecule (dsp);
}

void moldraw(Dsp_Molecule_t *mol_p,Tsd_t *patt,Display *dsp, Window dawin,GC gc,
        int X1,int Y1,int X2,int Y2,U32_t *roots,U32_t *synthemes, Boolean_t iswin)
{
        Dsp_Atom_t *atom_p;
        Dsp_Bond_t *bond_p;
        char *ftag,atomsym[10];
        float length;
        float norm_x, norm_y;
        int x0, y0, x1, y1, x2, y2;
        int atm_i, bnd_i, var_num;
        int i,j,k,l,bond_order[600];
        Boolean_t atom_conn[100];
        U32_t s;
        Dimension aw, ah, ax, ay;

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

        atom_p = mol_p->atoms;

        for (atm_i = 0; atm_i < mol_p->natms; atm_i++)
        {
                if (roots) s=molroot_and_syntheme(atm_i,roots,synthemes);
                else s=0;
                if ((!atom_p->isC && atom_conn[atm_i]) || s)
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

                        aw=7*strlen(atomsym)+5;
                        ah=16;

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
                        if (s) XDrawRectangle(dsp,dawin,gc,ax+X1,ay+Y1,aw,ah);

                        XDrawString(dsp,dawin,gc,ax+aw/4+X1,ay+3*ah/4+Y1,
                                atomsym,strlen(atomsym));
                }
                ++atom_p;
        }
}


U32_t molroot_and_syntheme(U32_t n,U32_t *r,U32_t *s)
{
        while(*r!=REACT_NODE_INVALID)
        {
                if (n==*r) return(*s);
                r++;
                s++;
        }
        return(0);
}
