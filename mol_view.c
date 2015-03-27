/******************************************************************************
*
*  Copyright (C) 1995, Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     MOL_VIEW.C
*
*    This module defines the routines needed to draw a molecule.
*    
*
*  Creation Date:
*
*    10-Dec-1995
*
*  Authors:
*
*    Daren Krebsbach
*
*  Routines:
*
*    Mol_Draw
*    Mol_Scale
*    MolForm_Create
*    MolForm_Destroy
*
*  Callback Routines:
*
*    MolView_Redraw_CB
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 11/21/00   Miller     Added Mol_Draw_Either to allow drawing to EITHER a
*                       Window OR a Pixmap (When is a Drawable not a Drawable?)
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>

#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include <Xm/Label.h>

#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_APP_RESRC_
#include "app_resrc.h"
#endif

#ifndef _H_DSP_
#include "dsp.h"
#endif

#ifndef _H_MOL_VIEW_
#include "mol_view.h"
#endif

void Mol_Draw_Either (Dsp_Molecule_t *, Drawable, Boolean_t);
void Mol_Draw_Either_Color (Dsp_Molecule_t *, Drawable, Boolean_t, Pixel);


/****************************************************************************
*
*  Function Name:                 Mol_Draw
*
*    Draw the given molecule in the reaction display window.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    None.
*
******************************************************************************/
void Mol_Draw
  (
  Dsp_Molecule_t  *mol_p,
  Window           dawin
  )
{
  Mol_Draw_Either (mol_p, dawin, TRUE); /* special case for Window - does not work for Pixmap - go figure! */
}

void Mol_Draw_Either (Dsp_Molecule_t *mol_p, Drawable dawin, Boolean_t iswin) /* gets around the stupidity of X in making a
                                                                                 Pixmap invalid for the XClearArea function */
{
  Mol_Draw_Either_Color (mol_p, dawin, iswin, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
}

void Mol_Draw_Either_Color (Dsp_Molecule_t *mol_p, Drawable dawin, Boolean_t iswin, Pixel color) /* gets around the stupidity of X
                                                                                                    in making a Pixmap invalid for
                                                                                                    the XClearArea function; also,
                                                                                                    does not assume foreground is
                                                                                                    black! */
{
  GC              gc;
  Display        *dsp;
  Dsp_Atom_t     *atom_p;
  Dsp_Bond_t     *bond_p;
  char           *ftag;
  XmString        atomsym, chgstr;
  float           length;
  float           norm_x, norm_y;
  int             x0, y0, x1, y1, x2, y2;
  int             atm_i, bnd_i;
  Dimension       aw, ah, ax, ay, cw, ch, cx, cy; 
  char            tmpchg[4];

  if (mol_p == NULL)
    return ;

  gc = SynAppR_MolGC_Get (&GSynAppR);
  dsp = Screen_TDisplay_Get (SynAppR_ScrnAtrb_Get (&GSynAppR));

  /*  First draw bonds.  */
  bond_p = mol_p->bonds;
  for (bnd_i = 0; bnd_i < mol_p->nbnds; bnd_i++) 
    {    
    x1 = bond_p->latm_p->x;
    y1 = bond_p->latm_p->y;
    x2 = bond_p->ratm_p->x;
    y2 = bond_p->ratm_p->y;
    switch (bond_p->nlines) 
      {
      case DSP_BOND_SINGLE:
        XDrawLine (dsp, dawin, gc, x1, y1, x2, y2);
        break;
      case DSP_BOND_DOUBLE:	  
        length = sqrt ((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
        norm_x = (y2-y1)/length; 
        norm_y = (x2-x1)/length;
        norm_x *= AppDim_BndDblOff_Get (&GAppDim);
        norm_y *= AppDim_BndDblOff_Get (&GAppDim);
        
        norm_x = (norm_x > 0) ? norm_x + 0.5 : norm_x - 0.5;
        norm_y = (norm_y > 0) ? norm_y + 0.5 : norm_y - 0.5;
        x0 = (int) norm_x;
        y0 = (int) norm_y;
        XDrawLine (dsp, dawin, gc, x1 + x0, y1 - y0, x2 + x0, y2 - y0);
        XDrawLine (dsp, dawin, gc, x1 - x0, y1 + y0, x2 - x0, y2 + y0);
        break;
      case DSP_BOND_TRIPLE:
        XDrawLine (dsp, dawin, gc, x1, y1, x2, y2);
        length = sqrt ((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
        norm_x = (y2-y1)/length; 
        norm_y = (x2-x1)/length;
        norm_x *= AppDim_BndTplOff_Get (&GAppDim);
        norm_y *= AppDim_BndTplOff_Get (&GAppDim);
          
        norm_x = (norm_x > 0) ? norm_x + 0.5 : norm_x - 0.5;
        norm_y = (norm_y > 0) ? norm_y + 0.5 : norm_y - 0.5;
        x0 = (int) norm_x;
        y0 = (int) norm_y;
        XDrawLine (dsp, dawin, gc, x1 + x0, y1 - y0, x2 + x0, y2 - y0);
        XDrawLine (dsp, dawin, gc, x1 - x0, y1 + y0, x2 - x0, y2 + y0);
        break;
      }

    ++bond_p;
    }

  /*  Then draw atoms.  */
  if (mol_p->scale < MVW_SCALE_SML)
    ftag = SAR_FONTTAG_ATM_SML;
  else
    ftag = SAR_FONTTAG_ATM_NML;

  atom_p = mol_p->atoms;
  for (atm_i = 0; atm_i < mol_p->natms; atm_i++) 
    {
    if (!atom_p->isC || atom_p->chg[0] != '\0')
      {
      atomsym = XmStringCreateLtoR (atom_p->sym, ftag);
      XmStringExtent (SynAppR_FontList_Get (&GSynAppR), atomsym, &aw, &ah);
      aw += 2;
      ax = (atom_p->x - (aw >> 1) > 0) ? atom_p->x - (aw >> 1) : 0;
      ay = (atom_p->y - (ah >> 1) > 0) ? atom_p->y - (ah >> 1) : 0;

      if (iswin)
        {
        XClearArea (dsp, dawin, ax, ay, aw, ah, FALSE);
        XmStringDrawImage (dsp, dawin, SynAppR_FontList_Get (&GSynAppR), 
          atomsym, gc, ax, ay, aw, XmALIGNMENT_BEGINNING, 
          XmSTRING_DIRECTION_L_TO_R, NULL); 
        }
      else
        {
        XSetForeground (dsp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
          SAR_CLRI_WHITE));
        XFillRectangle (dsp, dawin, gc, ax, ay, aw, ah);
        XSetForeground (dsp, gc, color /* SynAppR_IthClrPx_Get (&GSynAppR,
          SAR_CLRI_BLACK) */ );
        XDrawString (dsp, dawin, gc, ax, ay + ah, atom_p->sym, strlen (atom_p->sym));
        }
      XmStringFree (atomsym);
      if (atom_p->chg[0] != '\0')
        {
        sprintf (tmpchg, "%d", (unsigned) atom_p->chg[0]);
        chgstr = XmStringCreateLtoR (tmpchg, SAR_FONTTAG_ATM_SML);
        XmStringExtent (SynAppR_FontList_Get (&GSynAppR), chgstr, &cw, &ch);
        cw++;
        cx = ax + aw;
        cy = ay - (ah >> 1);
        if (iswin)
          {
          XClearArea (dsp, dawin, cx, cy, cw, ch, FALSE);
          XmStringDrawImage (dsp, dawin, SynAppR_FontList_Get (&GSynAppR),
            chgstr, gc, cx, cy, cw, XmALIGNMENT_BEGINNING,
            XmSTRING_DIRECTION_L_TO_R, NULL);
          }
        else
          {
          XSetForeground (dsp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
            SAR_CLRI_WHITE));
          XFillRectangle (dsp, dawin, gc, cx, cy, cw, ch);
          XSetForeground (dsp, gc, color /* SynAppR_IthClrPx_Get (&GSynAppR,
            SAR_CLRI_BLACK) */ );
          XDrawString (dsp, dawin, gc, cx, cy + ch, tmpchg, strlen (tmpchg));
          }
        XmStringFree (chgstr);
        }
      }

    ++atom_p;
    }

  return ;
}
/*  End of Mol_Draw  */


/****************************************************************************
*
*  Function Name:                 Mol_Scale
*
*    Scale the single molecule so that it fits nicely in the drawing area.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    None.
*
******************************************************************************/
void Mol_Scale
  (
  Dsp_Molecule_t *mol_p,
  Dimension       dah,
  Dimension       daw
  )
{
  Dsp_Atom_t     *atom_p;
  Dimension       x_off, y_off;
  Dimension       use_dah, use_daw;
  U16_t           atm_i;

  if (mol_p == NULL)
    return;

  use_dah = dah - (2 * AppDim_AtmMaxH_Get (&GAppDim) 
    + AppDim_MargMol_Get (&GAppDim));
  use_daw = daw - (2 * AppDim_AtmMaxW_Get (&GAppDim) 
    + AppDim_MargMol_Get (&GAppDim));

  if (mol_p->molh == 0)
    mol_p->scale = AppDim_MolScale_Get (&GAppDim);
  else
    {
    mol_p->scale = use_dah / (float) mol_p->molh;
    if (mol_p->scale > AppDim_MolScale_Get (&GAppDim))
      mol_p->scale = AppDim_MolScale_Get (&GAppDim);
    }
    
  if ((Dimension) (mol_p->scale  * mol_p->molw) > use_daw)
    mol_p->scale = use_daw / (float) mol_p->molw;

  /*  Calculate the coordinates of the molecules.  */
  x_off = ((Dimension) (daw - mol_p->molw * mol_p->scale)) >> 1;
  y_off = ((Dimension) (dah - mol_p->molh * mol_p->scale)) >> 1;
  atom_p = mol_p->atoms;
  for (atm_i = 0; atm_i < mol_p->natms; atm_i++)
    {
    atom_p->x = (int) (atom_p->x * mol_p->scale) + x_off;
    atom_p->y = (int) (atom_p->y * mol_p->scale) + y_off;
    atom_p++;
    }

  return ;
}
/*  End of Mol_Scale  */


/****************************************************************************
*
*  Function Name:                 MolForm_Create
*
*    This routine.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void MolForm_Create
  (
  MolView_t      *molv_p,
  Widget          parent
  )
{
  Widget         form;
  XmString       lbl_str;                 /* Label string */
  Dimension      nlh, mlh;


  form = XmCreateForm (parent, "MolVF", NULL, 0);
  XtVaSetValues (form,
    XmNresizable, True,
    XmNlabelFontList, SynAppR_FontList_Get (&GSynAppR),
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNleftAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    XmNrightAttachment, XmATTACH_NONE,
    XmNrightOffset, 0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNbottomOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNtopOffset, 0,
    NULL);

  MolView_Form_Put (*molv_p, form);

  lbl_str = XmStringCreateLtoR (MVW_CMPNAME, AppDim_RxnMrtTag_Get (&GAppDim));
  nlh = XmStringHeight (SynAppR_FontList_Get (&GSynAppR), lbl_str);
  MolView_NameLbl_Put (*molv_p, 
      XtVaCreateManagedWidget ("MolVCNL", 
      xmLabelWidgetClass,  form, 
      XmNlabelType, XmSTRING,
      XmNalignment, XmALIGNMENT_CENTER,
      XmNlabelString, lbl_str,
      XmNmarginHeight, 0,
      XmNmarginWidth, 0,
      XmNshadowThickness, 0,
      XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
      XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_AVL_VD),
      NULL));
  XmStringFree (lbl_str);

  lbl_str = XmStringCreateLtoR (MVW_CMPMRT, AppDim_RxnMrtTag_Get (&GAppDim));
  mlh = XmStringHeight (SynAppR_FontList_Get (&GSynAppR), lbl_str);
  MolView_MrtLbl_Put (*molv_p, 
      XtVaCreateManagedWidget ("MolVCML", 
      xmLabelWidgetClass,  form, 
      XmNlabelType, XmSTRING,
      XmNalignment, XmALIGNMENT_CENTER,
      XmNlabelString, lbl_str,
      XmNmarginHeight, 0,
      XmNmarginWidth, 0,
      XmNshadowThickness, 0,
      XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
      XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
      NULL));
  XmStringFree (lbl_str);

  MolView_DA_Put (*molv_p, XtVaCreateWidget ("MolVDA", 
    xmDrawingAreaWidgetClass,  form, 
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    NULL));
  XtAddCallback (MolView_DA_Get (*molv_p), XmNexposeCallback, 
    MolView_Redraw_CB, (XtPointer) molv_p);

  XtVaSetValues (MolView_MrtLbl_Get (*molv_p),
    XmNtopAttachment, XmATTACH_NONE,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset, AppDim_MargLblPB_Get (&GAppDim),
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, AppDim_MargLblPB_Get (&GAppDim),
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  XtVaSetValues (MolView_NameLbl_Get (*molv_p),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, AppDim_MargLblPB_Get (&GAppDim),
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, AppDim_MargLblPB_Get (&GAppDim),
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  XtVaSetValues (MolView_DA_Get (*molv_p),
    XmNtopOffset, 0,
    XmNtopWidget, MolView_NameLbl_Get (*molv_p),
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNbottomOffset, 0,
    XmNbottomWidget, MolView_MrtLbl_Get (*molv_p),
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightOffset, 0,
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  XtManageChild (MolView_NameLbl_Get (*molv_p));
  XtManageChild (MolView_MrtLbl_Get (*molv_p));
  XtManageChild (MolView_DA_Get (*molv_p));

  MolView_Mol_Put (*molv_p, NULL);
  MolView_DAW_Put (*molv_p, 0);
  MolView_LblsH_Put (*molv_p, nlh + mlh);

  return ;
}
/*  End of MolForm_Create  */


/****************************************************************************
*
*  Function Name:                 MolForm_Destroy
*
*    This routine frees memory allocated to the molecule
*    being displayed.  It does not destroy the widgets.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    Frees memory for molecules.
*
******************************************************************************/
void MolForm_Destroy
  (
  MolView_t    *molv_p
  )
{
  if (MolView_Mol_Get (*molv_p) != NULL)
    free_Molecule (MolView_Mol_Get (*molv_p));
  
  return ;
}
/*  End of MolForm_Destroy  */


/****************************************************************************
*
*  Function Name:                 MolView_Redraw_CB
*
*    Redraw call back for the given molecule view.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    None.
*
******************************************************************************/
void MolView_Redraw_CB
  (
  Widget        w, 
  XtPointer     client_p, 
  XtPointer     call_p  
  )
{
  XmDrawingAreaCallbackStruct *cb_p;
  MolView_t  *molv_p;

  molv_p = (MolView_t *) client_p;
  cb_p = (XmDrawingAreaCallbackStruct *) call_p;

  if (cb_p->reason == XmCR_EXPOSE && molv_p != NULL)
    Mol_Draw (MolView_Mol_Get (*molv_p), cb_p->window);

  return ;
}
/*  End of MolView_Redraw_CB  */

/*  End of MOL_VIEW.C  */
