#ifndef _H_CMP_INST_
#define _H_CMP_INST_  1
/******************************************************************************
*
*  Copyright (C) 1998 SmithKline Beecham, Daren Krebsbach
*  Based upon PST.H and PST_VIEW.H, Copyright (C) The Synchem Project
*
*  Module Name:                     CMP_INST.H
*
*    This header file defines the data structure for managing
*    the information needed to find all instances of a compound
*    in a given status file and display them in the PST viewer. 
*
*
*  Creation Date:
*
*    24-Jul-1998
*
*  Authors:
*
*    Daren Krebsbach
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
*
******************************************************************************/
#include "synchem.h"

#ifndef _H_PST_
#include "pst.h"
#endif

#ifndef _H_DSP_
#include "dsp.h"
#endif


/*** Literal Values ***/

#define CDI_CMPINST_NONE      (U16_t) -1
#define CDI_NUM_BUFLEN        8
#define CDI_TEXT_BUFLEN       32
#define CDI_DA_WIDTH_MIN      225
#define CDI_DA_WIDTH_SCALE    0.20

#define CDI_TITLE             "Compound Instances"
#define CDI_LABEL_CMP         "Compound Number:  "
#define CDI_LABEL_INSTOUTOF   " out of "
#define CDI_PUSHB_DISMISS     "dismiss"
#define CDI_PUSHB_SELECT      "select"

#define CDI_ERROR_NOSTRING\
          "Comp Inst:  error.  Text window returned NULL string."
#define CDI_WARN_CONVERT\
          "Comp Inst:  invalid entry.  Unable to convert string to integer."
#define CDI_WARN_INVALID\
          "Comp Inst:  invalid instance entered."
#define CDI_WARN_NOPST\
          "Comp Inst:  not currently examining a PST."
#define CDI_WARN_NOCURCMP\
          "Comp Inst:  no current compound."
#define CDI_WARN_BADCMPNUM\
          "Comp Inst:  invalid compound number."
#define CDI_WARN_NOTSOLVED\
          "Comp Inst:  not all subgoals along the path from the root\n"\
          "to the selected compound are completely solved."
#define CDI_WARN_BADSEL\
          "Comp Inst:  no compound was selected."
#define CDI_WARN_SHELLEY_ABORT\
          "Comp Inst:  unable to calculate coordinates of compound."

#define CDI_ARROW_DIM           25


typedef struct cmp_instances_s
  {
  Compound_t    **instances;            /* Array of compound instances */
  Compound_t     *curr_cmp;             /* Current compound */ 
  Dsp_Molecule_t *mol_p;                /* Display molecule */
  Widget          formdlg;              /* Compound Instance form dialog */ 
  Widget          mol_da;               /* Target mol drawing area */ 
  Widget          cmplbl;               /* Compound label */ 
  Widget          cmpsel_pb;            /* Compound select push button */ 
  Widget          cmptxt;               /* Compound number text--editable */ 
  Widget          inst_text;            /* Current instance text--editable */ 
  Widget          instnext_ab;          /* Next subgoal arrow button */ 
  Widget          instprev_ab;          /* Previous subgoal arrow button */ 
  Widget          total_lbl;            /* Instance out of total label */ 
  Widget          total_txt;            /* ITotal text */ 
  Widget          dismiss_pb;           /* Dismiss pushbutton */
  Dimension       da_ht;
  Dimension       da_wd;
  U16_t           curr_inst;            /* Current instance  */ 
  U16_t           num_insts;            /* Number of instances */
  char            cmpbuf[CDI_NUM_BUFLEN];
  char            instbuf[CDI_NUM_BUFLEN];
  char            totalbuf[CDI_TEXT_BUFLEN];
  Boolean_t       is_created;           /* Compound Instances created? */
  }  CmpInst_t;
#define CDI_CMPINST_SIZE  (sizeof (CmpInst_t))

/** Field Access Macros for CmpInst_t **/

/* Macro Prototypes

  char           *CmpInst_CmpBuf_Get     (CmpInst_t *);
  Compound_t    **CmpInst_CmpInsts_Get   (CmpInst_t *);
  void            CmpInst_CmpInsts_Put   (CmpInst_t *, Compound_t **);
  Widget          CmpInst_CmpLbl_Get     (CmpInst_t *);
  void            CmpInst_CmpLbl_Put     (CmpInst_t *, Widget);
  Widget          CmpInst_CmpSelPB_Get   (CmpInst_t *);
  void            CmpInst_CmpSelPB_Put   (CmpInst_t *, Widget);
  Widget          CmpInst_CmpText_Get    (CmpInst_t *);
  void            CmpInst_CmpText_Put    (CmpInst_t *, Widget);
  Compound_t     *CmpInst_CurrCmp_Get    (CmpInst_t *);
  void            CmpInst_CurrCmp_Put    (CmpInst_t *, Compound_t *);
  U16_t           CmpInst_CurrInst_Get   (CmpInst_t *);
  void            CmpInst_CurrInst_Put   (CmpInst_t *, U16_t);
  Widget          CmpInst_DismissPB_Get  (CmpInst_t *);
  void            CmpInst_DismissPB_Put  (CmpInst_t *, Widget);
  Widget          CmpInst_FormDlg_Get    (CmpInst_t *);
  void            CmpInst_FormDlg_Put    (CmpInst_t *, Widget);
  char           *CmpInst_InstBuf_Get    (CmpInst_t *);
  Widget          CmpInst_InstNextAB_Get (CmpInst_t *);
  void            CmpInst_InstNextAB_Put (CmpInst_t *, Widget);
  Widget          CmpInst_InstPrevAB_Get (CmpInst_t *);
  void            CmpInst_InstPrevAB_Put (CmpInst_t *, Widget);
  Widget          CmpInst_InstText_Get   (CmpInst_t *);
  void            CmpInst_InstText_Put   (CmpInst_t *, Widget);
  Boolean_t       CmpInst_IsCreated_Get  (CmpInst_t *);
  void            CmpInst_IsCreated_Put  (CmpInst_t *, Boolean_t);
  Compound_t     *CmpInst_IthCmpInst_Get (CmpInst_t *, U16_t);
  void            CmpInst_IthCmpInst_Put (CmpInst_t *, U16_t, Compound_t *);
  Dsp_Molecule_t *CmpInst_Molecule_Get   (CmpInst_t *);
  void            CmpInst_Molecule_Put   (CmpInst_t *, Dsp_Molecule_t *);
  Widget          CmpInst_MolDA_Get      (CmpInst_t *);
  void            CmpInst_MolDA_Put      (CmpInst_t *, Widget);
  Dimension       CmpInst_MolDAHt_Get    (CmpInst_t *);
  void            CmpInst_MolDAHt_Put    (CmpInst_t *, Dimension);
  Dimension       CmpInst_MolDAWd_Get    (CmpInst_t *);
  void            CmpInst_MolDAWd_Put    (CmpInst_t *, Dimension);
  U16_t           CmpInst_NumInsts_Get   (CmpInst_t *);
  void            CmpInst_NumInsts_Put   (CmpInst_t *, U16_t);
  char           *CmpInst_TotalBuf_Get   (CmpInst_t *);
  Widget          CmpInst_TotalLbl_Get   (CmpInst_t *);
  void            CmpInst_TotalLbl_Put   (CmpInst_t *, Widget);
  Widget          CmpInst_TotalText_Get  (CmpInst_t *);
  void            CmpInst_TotalText_Put  (CmpInst_t *, Widget);
*/

#define CmpInst_CmpBuf_Get(cip)\
  (cip)->cmpbuf
#define CmpInst_CmpInsts_Get(cip)\
  (cip)->instances
#define CmpInst_CmpInsts_Put(cip, value)\
  (cip)->instances = (value)
#define CmpInst_CmpLbl_Get(cip)\
  (cip)->cmplbl
#define CmpInst_CmpLbl_Put(cip, value)\
  (cip)->cmplbl = (value)
#define CmpInst_CmpSelPB_Get(cip)\
  (cip)->cmpsel_pb
#define CmpInst_CmpSelPB_Put(cip, value)\
  (cip)->cmpsel_pb = (value)
#define CmpInst_CmpText_Get(cip)\
  (cip)->cmptxt
#define CmpInst_CmpText_Put(cip, value)\
  (cip)->cmptxt = (value)
#define CmpInst_CurrCmp_Get(cip)\
  (cip)->curr_cmp
#define CmpInst_CurrCmp_Put(cip, value)\
  (cip)->curr_cmp = (value)
#define CmpInst_CurrInst_Get(cip)\
  (cip)->curr_inst
#define CmpInst_CurrInst_Put(cip, value)\
  (cip)->curr_inst = (value)
#define CmpInst_DismissPB_Get(cip)\
  (cip)->dismiss_pb
#define CmpInst_DismissPB_Put(cip, value)\
  (cip)->dismiss_pb = (value)
#define CmpInst_FormDlg_Get(cip)\
  (cip)->formdlg
#define CmpInst_FormDlg_Put(cip, value)\
  (cip)->formdlg = (value)
#define CmpInst_InstBuf_Get(cip)\
  (cip)->instbuf
#define CmpInst_InstNextAB_Get(cip)\
  (cip)->instnext_ab
#define CmpInst_InstNextAB_Put(cip, value)\
  (cip)->instnext_ab = (value)
#define CmpInst_InstPrevAB_Get(cip)\
  (cip)->instprev_ab
#define CmpInst_InstPrevAB_Put(cip, value)\
  (cip)->instprev_ab = (value)
#define CmpInst_InstText_Get(cip)\
  (cip)->inst_text
#define CmpInst_InstText_Put(cip, value)\
  (cip)->inst_text = (value)
#define CmpInst_IsCreated_Get(cip)\
  (cip)->is_created
#define CmpInst_IsCreated_Put(cip, value)\
  (cip)->is_created = (value)
#define CmpInst_IthCmpInst_Get(cip, ith)\
  (cip)->instances[ith]
#define CmpInst_IthCmpInst_Put(cip, ith, value)\
  (cip)->instances[ith] = (value)
#define CmpInst_Molecule_Get(cip)\
  (cip)->mol_p
#define CmpInst_Molecule_Put(cip, value)\
  (cip)->mol_p = (value)
#define CmpInst_MolDA_Get(cip)\
  (cip)->mol_da
#define CmpInst_MolDA_Put(cip, value)\
  (cip)->mol_da = (value)
#define CmpInst_MolDAHt_Get(cip)\
  (cip)->da_ht
#define CmpInst_MolDAHt_Put(cip, value)\
  (cip)->da_ht = (value)
#define CmpInst_MolDAWd_Get(cip)\
  (cip)->da_wd
#define CmpInst_MolDAWd_Put(cip, value)\
  (cip)->da_wd = (value)
#define CmpInst_NumInsts_Get(cip)\
  (cip)->num_insts
#define CmpInst_NumInsts_Put(cip, value)\
  (cip)->num_insts = (value)
#define CmpInst_TotalBuf_Get(cip)\
  (cip)->totalbuf
#define CmpInst_TotalLbl_Get(cip)\
  (cip)->total_lbl
#define CmpInst_TotalLbl_Put(cip, value)\
  (cip)->total_lbl = (value)
#define CmpInst_TotalText_Get(cip)\
  (cip)->total_txt
#define CmpInst_TotalText_Put(cip, value)\
  (cip)->total_txt = (value)



/*** Macros ***/

/* Macro Prototypes

*/

/*** Routine Prototypes ***/

void         CmpInst_Init       (CmpInst_t *);
void         CmpInst_Reset      (CmpInst_t *);

/*  Routines defined in pst_view.h  (to prevent circular includes) 
void         CmpInst_Create     (Widget, PstView_t *);
Boolean_t    CmpInst_Insts_Set  (PstView_t *);
Boolean_t    CmpInst_Setup      (PstView_t *);
void         CmpInst_Update     (PstView_t *, U16_t);
*/

void CmpInst_CmpSel_CB       (Widget, XtPointer, XtPointer);
void CmpInst_CmpText_CB      (Widget, XtPointer, XtPointer);
void CmpInst_InstNext_CB     (Widget, XtPointer, XtPointer);
void CmpInst_InstPrev_CB     (Widget, XtPointer, XtPointer);
void CmpInst_InstText_CB     (Widget, XtPointer, XtPointer);
void CmpInst_Dismiss_CB      (Widget, XtPointer, XtPointer);

#endif
/*  End of CMP_INST.H  */
