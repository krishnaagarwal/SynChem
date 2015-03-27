/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:                     FGDRAW_IMAGES.C
*
*    Creates drawing bitmaps of all attributes in the functional group table
*    for display within the Synchem GUI.
*
*  Creation Date:
*
*    16-Feb-2000
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

#include <Xm/Xm.h>

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#include "synchem.h"
#include "synio.h"
#include "app_resrc.h"
#define _GLOBAL_DEF_
#include "extern.h"
#undef _GLOBAL_DEF_

void FGDraw_Init (ScreenAttr_t *, Boolean_t);
void FGDraw_Draw_Molecules (int);
void FGDraw_Create_Binary_Bitmap (int);

main(int argc, char *argv[])
{
  U16_t fg_num;
  char *name, *sling, *sl;
  XtAppContext schlContext;
  ScreenAttr_t *scra_p;

  scra_p = SynAppR_ScrnAtrb_Get (&GSynAppR);

  toplevel=XtVaAppInitialize(&schlContext,"SchList",NULL,0,&argc,argv, NULL,NULL);

  SynAppR_PreInit((Widget) toplevel,SAR_APPSIZE_DFLT_WD);

  SynAppR_PostInit((Widget) toplevel);

  XtVaSetValues((Widget) toplevel,XmNheight,10, XmNwidth,13, XmNtitle,"Reaction Library Editor",XmNx,0,XmNy,0,NULL);
  XtRealizeWidget((Widget) toplevel);

  AppDim_AppHeight_Put (&GAppDim, 1000);

  IO_Init ();
  FGDraw_Init (scra_p,TRUE);
  FGDraw_Draw_Molecules (0);
  FGDraw_Draw_Molecules (1);
  FGDraw_Create_Binary_Bitmap (0);
  FGDraw_Create_Binary_Bitmap (1);
}
