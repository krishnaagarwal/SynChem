#ifndef _H_MOL_VIEW_
#define _H_MOL_VIEW_  1
/******************************************************************************
*
*  Copyright (C) 1995-1996 Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     MOL_VIEW.H
*
*    This header file defines the data structures for managing
*    the information needed to view information for a given
*    molecule. 
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
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Lolita	xxx
*
******************************************************************************/

#ifndef _H_DSP_
#include "dsp.h"
#endif

#define MVW_SCALE_SML        0.48

#define MVW_CMPNAME          "Available"
#define MVW_CMPMRT           "Number and Merit"

/*** Data Structures ***/

typedef struct mol_view_s
  {
  Dsp_Molecule_t       *mol;                /* display mol */
  Widget                da;                 /* mol drawing area */ 
  Widget                form;               /* form for the mol view */ 
  Widget                merit_lbl;          /* merit label (or catalog num) */ 
  Widget                name_lbl;           /* name label (avail cmp) */ 
  Widget                sign_lbl;           /* sign (arrow or plus) label */
  Dimension             da_w;               /* drawing area width */   
  Dimension             lbls_h;             /* Height of two labels */   
  Dimension             lbl_w;              /* longest label width */   
  } 
  MolView_t;

/* Macro Prototypes

  Widget          MolView_DA_Get       (MolView_t);
  void            MolView_DA_Put       (MolView_t, Widget);
  Dimension       MolView_DAW_Get      (MolView_t);
  void            MolView_DAW_Put      (MolView_t, Dimension);
  Widget          MolView_Form_Get     (MolView_t);
  void            MolView_Form_Put     (MolView_t, Widget);
  Dimension       MolView_LblsH_Get    (MolView_t);
  void            MolView_LblsH_Put    (MolView_t, Dimension);
  Dimension       MolView_LblW_Get     (MolView_t);
  void            MolView_LblW_Put     (MolView_t, Dimension);
  Dsp_Molecule_t *MolView_Mol_Get      (MolView_t);
  void            MolView_Mol_Put      (MolView_t, Dsp_Molecule_t *);
  Widget          MolView_MrtLbl_Get   (MolView_t);
  void            MolView_MrtLbl_Put   (MolView_t, Widget);
  Widget          MolView_NameLbl_Get  (MolView_t);
  void            MolView_NameLbl_Put  (MolView_t, Widget);
  Widget          MolView_Sign_Get     (MolView_t);
  void            MolView_Sign_Put     (MolView_t, Widget);
*/

#define MolView_DA_Get(mvs)\
  (mvs).da
#define MolView_DA_Put(mvs, value)\
  (mvs).da = (value)
#define MolView_DAW_Get(mvs)\
  (mvs).da_w
#define MolView_DAW_Put(mvs, value)\
  (mvs).da_w = (value)
#define MolView_Form_Get(mvs)\
  (mvs).form
#define MolView_Form_Put(mvs, value)\
  (mvs).form = (value)
#define MolView_LblsH_Get(mvs)\
  (mvs).lbls_h
#define MolView_LblsH_Put(mvs, value)\
  (mvs).lbls_h = (value)
#define MolView_LblW_Get(mvs)\
  (mvs).lbl_w
#define MolView_LblW_Put(mvs, value)\
  (mvs).lbl_w = (value)
#define MolView_Mol_Get(mvs)\
  (mvs).mol
#define MolView_Mol_Put(mvs, value)\
  (mvs).mol = (value)
#define MolView_MrtLbl_Get(mvs)\
  (mvs).merit_lbl
#define MolView_MrtLbl_Put(mvs, value)\
  (mvs).merit_lbl = (value)
#define MolView_NameLbl_Get(mvs)\
  (mvs).name_lbl
#define MolView_NameLbl_Put(mvs, value)\
  (mvs).name_lbl = (value)
#define MolView_Sign_Get(mvs)\
  (mvs).sign_lbl
#define MolView_Sign_Put(mvs, value)\
  (mvs).sign_lbl = (value)

/*** Macros ***/

/* Macro Prototypes

*/

/* Routine Prototypes */
void    Mol_Draw          (Dsp_Molecule_t *, Window);
void    Mol_Scale         (Dsp_Molecule_t *, Dimension, Dimension);
void    MolForm_Create    (MolView_t *, Widget);
void    MolForm_Destroy   (MolView_t *);

void    MolView_Redraw_CB  (Widget, XtPointer, XtPointer);


#endif
