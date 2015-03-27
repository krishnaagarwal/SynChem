#ifndef _H_PST_VIEW_
#define _H_PST_VIEW_  1
/******************************************************************************
*
*  Copyright (C) 1995 Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     PST_VIEW.H
*
*    This header file defines the data structures for managing
*    the information needed to view SYNCHEM's problem solving
*    tree (graph). 
*
*
*  Creation Date:
*
*    10-Jun-1995
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

#ifndef _H_PST_
#include "pst.h"
#endif

#ifndef _H_APP_RESRC_
#include "app_resrc.h"
#endif

#ifndef _H_RXN_PREVIEW_
#include "rxn_preview.h"
#endif

#ifndef _H_RXN_VIEW_
#include "rxn_view.h"
#endif

#ifndef _H_CMP_INST_
#include "cmp_inst.h"
#endif

#ifndef _H_SEL_TRACE_
#include "sel_trace.h"
#endif

/*** Literal Values ***/

/*  Default and status values  */
#define PVW_NODE_NONE         65535

#define PVW_STATUS_NONE       0x00
#define PVW_STATUS_VISITED    0x01
#define PVW_STATUS_VISELSE    0x02
#define PVW_STATUS_TRACED     0x04
#define PVW_STATUS_TRACELSE   0x08
#define PVW_STATUS_MARKED     0x10
#define PVW_STATUS_MARKELSE   0x20

#define PVW_TREETYPE_DEV      2
#define PVW_TREETYPE_FULL     4
#define PVW_TREETYPE_NONE     0
#define PVW_TREETYPE_SOLV     1

#define PVW_LEVEL_NUMOF       3  
#define PVW_LEVEL_TOP         0  
#define PVW_LEVEL_MID         1 
#define PVW_LEVEL_BTM         2  
#define PVW_LEVEL_PATH        253
#define PVW_LEVEL_NONE        255  

/*  Init number of symbols per tree level and path.  */
#define PVW_PST_NUMSYMS_INIT  100
#define PVW_PATH_NUMSYMS_INIT 100

/**  Line widths of boxes, connecting lines, bars, etc.  **/
#define PVW_DEFAULT_LNWD      1
#define PVW_FOCUSBOX_LNWD     1
#define PVW_PTH_HBAR_LNWD     3
#define PVW_SELECTBOX_LNW     2
#define PVW_SELECTLN_LNWD     2
#define PVW_SYMBOL_LNWD       2

/*  Initial label strings  */
#define PVW_LBL_PATH          "Path:"
#define PVW_LBL_TNSG          "0"
#define PVW_LBL_MNSG          "0"
#define PVW_LBL_TLVL          "tgt"
#define PVW_LBL_MLVL          "1"
#define PVW_LBL_BLVL          "2"

#define PVW_CYCLE_WARN \
  "Cycle detected:  selected compound already lies along current path\n"\
  "in level "
#define PVW_NOROOT_WARN \
  "Problem Solving Tree does not have a root." 
#define PVW_NOTINIT_WARN \
  "Unable to initialize Problem Solving Tree." 

/*** Data Structures ***/

/**  The PST View Compound Information data structure.  **/

typedef struct psv_cmpinfo_s
  {
  char          *rxn_comment;           /* parent => cmp rxn comment */ 
  U32_t          cycle;                 /* Cycle compound was expanded */
  short int      midbar;                /* Bar mid offset from head mid */ 
  U8_t           cmt_len;               /* Length of reaction comment */ 
  U8_t           status;                /* Visit status */ 
  }  PsvCmpInfo_t;
#define PSVCMPINFO_SIZE (sizeof (PsvCmpInfo_t))

/** Field Access Macros for PsvCmpInfo_t **/

/* Macro Prototypes

  U8_t        PsvCmpI_CmtLen_Get   (PsvCmpInfo_t);
  void        PsvCmpI_CmtLen_Put   (PsvCmpInfo_t, U8_t);
  U32_t       PsvCmpI_Cycle_Get    (PsvCmpInfo_t);
  void        PsvCmpI_Cycle_Put    (PsvCmpInfo_t, U32_t);
  Boolean_t   PsvCmpI_Marked_Get   (PsvCmpInfo_t);
  void        PsvCmpI_Marked_Set   (PsvCmpInfo_t);
  void        PsvCmpI_Marked_Unset (PsvCmpInfo_t);
  Boolean_t   PsvCmpI_MarkElse_Get (PsvCmpInfo_t);
  void        PsvCmpI_MarkElse_Set (PsvCmpInfo_t);
  void        PsvCmpI_MarkElse_Unset (PsvCmpInfo_t);
  short int   PsvCmpI_MidBar_Get   (PsvCmpInfo_t);
  void        PsvCmpI_MidBar_Put   (PsvCmpInfo_t, U8_t);
  char       *PsvCmpI_RxnCmmt_Get  (PsvCmpInfo_t);
  void        PsvCmpI_RxnCmmt_Put  (PsvCmpInfo_t, char *);
  U8_t        PsvCmpI_Status_Get   (PsvCmpInfo_t);
  void        PsvCmpI_Status_Put   (PsvCmpInfo_t, U8_t);
  Boolean_t   PsvCmpI_Traced_Get   (PsvCmpInfo_t);
  void        PsvCmpI_Traced_Set   (PsvCmpInfo_t);
  void        PsvCmpI_Traced_Unset (PsvCmpInfo_t);
  Boolean_t   PsvCmpI_TracElse_Get (PsvCmpInfo_t);
  void        PsvCmpI_TracElse_Set (PsvCmpInfo_t);
  void        PsvCmpI_TracElse_Unset (PsvCmpInfo_t);
  Boolean_t   PsvCmpI_Visited_Get  (PsvCmpInfo_t);
  void        PsvCmpI_Visited_Set  (PsvCmpInfo_t);
  void        PsvCmpI_Visited_Unset (PsvCmpInfo_t);
  Boolean_t   PsvCmpI_VstElse_Get  (PsvCmpInfo_t);
  void        PsvCmpI_VstElse_Set  (PsvCmpInfo_t);
  void        PsvCmpI_VstElse_Unset (PsvCmpInfo_t);
*/

#define PsvCmpI_CmtLen_Get(pcip)\
  (pcip).cmt_len
#define PsvCmpI_CmtLen_Put(pcip, value)\
  (pcip).cmt_len = (value)
#define PsvCmpI_Cycle_Get(pcip)\
  (pcip).cycle
#define PsvCmpI_Cycle_Put(pcip, value)\
  (pcip).cycle = (value)
#define PsvCmpI_MidBar_Get(pcip)\
  (pcip).midbar
#define PsvCmpI_MidBar_Put(pcip, value)\
  (pcip).midbar = (value)
#define PsvCmpI_RxnCmmt_Get(pcip)\
  (pcip).rxn_comment
#define PsvCmpI_RxnCmmt_Put(pcip, value)\
  (pcip).rxn_comment = (value)
#define PsvCmpI_Status_Get(pcip)\
  (pcip).status
#define PsvCmpI_Status_Put(pcip, value)\
  (pcip).status = (value)

#define PsvCmpI_Marked_Get(pcip)\
  ((pcip).status & PVW_STATUS_MARKED)
#define PsvCmpI_Marked_Set(pcip)\
  (pcip).status |= PVW_STATUS_MARKED
#define PsvCmpI_Marked_Unset(pcip)\
  (pcip).status &= ~PVW_STATUS_MARKED

#define PsvCmpI_MarkElse_Get(pcip)\
  ((pcip).status & PVW_STATUS_MARKELSE)
#define PsvCmpI_MarkElse_Set(pcip)\
  (pcip).status |= PVW_STATUS_MARKELSE
#define PsvCmpI_MarkElse_Unset(pcip)\
  (pcip).status &= ~PVW_STATUS_MARKELSE

#define PsvCmpI_Traced_Get(pcip)\
  ((pcip).status & PVW_STATUS_TRACED)
#define PsvCmpI_Traced_Set(pcip)\
  (pcip).status |= PVW_STATUS_TRACED
#define PsvCmpI_Traced_Unset(pcip)\
  (pcip).status &= ~PVW_STATUS_TRACED

#define PsvCmpI_TracElse_Get(pcip)\
  ((pcip).status & PVW_STATUS_TRACELSE)
#define PsvCmpI_TracElse_Set(pcip)\
  (pcip).status |= PVW_STATUS_TRACELSE
#define PsvCmpI_TracElse_Unset(pcip)\
  (pcip).status &= ~PVW_STATUS_TRACELSE

#define PsvCmpI_Visited_Get(pcip)\
  ((pcip).status & PVW_STATUS_VISITED)
#define PsvCmpI_Visited_Set(pcip)\
  (pcip).status |= PVW_STATUS_VISITED
#define PsvCmpI_Visited_Unset(pcip)\
  (pcip).status &= ~PVW_STATUS_VISITED

#define PsvCmpI_VstElse_Get(pcip)\
  ((pcip).status & PVW_STATUS_VISELSE)
#define PsvCmpI_VstElse_Set(pcip)\
  (pcip).status |= PVW_STATUS_VISELSE
#define PsvCmpI_VstElse_Unset(pcip)\
  (pcip).status &= ~PVW_STATUS_VISELSE

/**  Drawing Context data structure.  **/
typedef struct psv_drawcxt_s
  {
  GC             gc;                    /* GC for pst view */ 
  Pixmap         pm;                    /* Pixmap for path view */ 
  Widget         da;                    /* Drawing area */ 
  unsigned int   linew;                 /* Last line width used in GC */ 
  unsigned int   da_ht;                 /* Height of drawing area */ 
  unsigned int   da_wd;                 /* Width of drawing area */ 
  unsigned int   pm_wd;                 /* Width of pixmap */ 
  int            lnsty;                 /* Last line style used in GC */ 
  U16_t          num_vsb_nodes;         /* Number of visible nodes in da */
  Pixel          bg;                    /* Background pixel for da */ 
  U8_t           color_fg;              /* Last fg color used in GC */ 
  U8_t           fill;                  /* Last fill style used in GC */ 
  }  DrawCxt_t;
#define DRAWCXT_SIZE (sizeof (DrawCxt_t))

/** Field Access Macros for DrawCxt_t **/

/* Macro Prototypes

  Pixel        DrawCxt_BgPxl_Get    (DrawCxt_t *);
  void         DrawCxt_BgPxl_Put    (DrawCxt_t *, Pixel);
  Widget       DrawCxt_DA_Get       (DrawCxt_t *);
  void         DrawCxt_DA_Put       (DrawCxt_t *, Widget);
  unsigned int DrawCxt_DAH_Get      (DrawCxt_t *);
  void         DrawCxt_DAH_Put      (DrawCxt_t *, unsigned int);
  unsigned int DrawCxt_DAW_Get      (DrawCxt_t *);
  void         DrawCxt_DAW_Put      (DrawCxt_t *, unsigned int);
  GC           DrawCxt_GC_Get       (DrawCxt_t *);
  void         DrawCxt_GC_Put       (DrawCxt_t *, GC);
  U8_t         DrawCxt_GCFg_Get     (DrawCxt_t *);
  void         DrawCxt_GCFg_Put     (DrawCxt_t *, U8_t);
  U8_t         DrawCxt_GCFill_Get   (DrawCxt_t *);
  void         DrawCxt_GCFill_Put   (DrawCxt_t *, U8_t);
  unsigned int DrawCxt_GCLineW_Get  (DrawCxt_t *);
  void         DrawCxt_GCLineW_Put  (DrawCxt_t *, unsigned int);
  int          DrawCxt_GCLnSty_Get  (DrawCxt_t *);
  void         DrawCxt_GCLnSty_Put  (DrawCxt_t *, int);
  U16_t        DrawCxt_NumVsbNd_Get (DrawCxt_t *);
  void         DrawCxt_NumVsbNd_Put (DrawCxt_t *, U16_t);
  Pixmap       DrawCxt_Pmap_Get     (DrawCxt_t *);
  void         DrawCxt_Pmap_Put     (DrawCxt_t *, Pixmap);
  unsigned int DrawCxt_PmapW_Get    (DrawCxt_t *);
  void         DrawCxt_PmapW_Put    (DrawCxt_t *, unsigned int);
*/

#define DrawCxt_BgPxl_Get(dcp)\
  (dcp)->bg
#define DrawCxt_BgPxl_Put(dcp, value)\
  (dcp)->bg = (value)
#define DrawCxt_DA_Get(dcp)\
  (dcp)->da
#define DrawCxt_DA_Put(dcp, value)\
  (dcp)->da = (value)
#define DrawCxt_DAH_Get(dcp)\
  (dcp)->da_ht
#define DrawCxt_DAH_Put(dcp, value)\
  (dcp)->da_ht = (value)
#define DrawCxt_DAW_Get(dcp)\
  (dcp)->da_wd
#define DrawCxt_DAW_Put(dcp, value)\
  (dcp)->da_wd = (value)
#define DrawCxt_GC_Get(dcp)\
  (dcp)->gc
#define DrawCxt_GC_Put(dcp, value)\
  (dcp)->gc = (value)
#define DrawCxt_GCFg_Get(dcp)\
  (dcp)->color_fg
#define DrawCxt_GCFg_Put(dcp, value)\
  (dcp)->color_fg = (value)
#define DrawCxt_GCFill_Get(dcp)\
  (dcp)->fill
#define DrawCxt_GCFill_Put(dcp, value)\
  (dcp)->fill = (value)
#define DrawCxt_GCLineW_Get(dcp)\
  (dcp)->linew
#define DrawCxt_GCLineW_Put(dcp, value)\
  (dcp)->linew = (value)
#define DrawCxt_GCLnSty_Get(dcp)\
  (dcp)->lnsty
#define DrawCxt_GCLnSty_Put(dcp, value)\
  (dcp)->lnsty = (value)
#define DrawCxt_NumVsbNd_Get(dcp)\
  (dcp)->num_vsb_nodes
#define DrawCxt_NumVsbNd_Put(dcp, value)\
  (dcp)->num_vsb_nodes = (value)
#define DrawCxt_Pmap_Get(dcp)\
  (dcp)->pm
#define DrawCxt_Pmap_Put(dcp, value)\
  (dcp)->pm = (value)
#define DrawCxt_PmapW_Get(dcp)\
  (dcp)->pm_wd
#define DrawCxt_PmapW_Put(dcp, value)\
  (dcp)->pm_wd = (value)

/**  The PST Tree Level data structure.  **/

typedef struct psv_treelvl_s
  {
  Compound_t   **nodes;                 /* Array of compound nodes */ 
  U32_t          fcs_sg;                /* Current focus subgoal index */ 
  U32_t          sel_sg;                /* Current selected subgoal index */ 
  U16_t          focus;                 /* Focus on this node */
  U16_t          head;                  /* Head of nodes */ 
  U16_t          ncmps;                 /* Number of compounds  */ 
  U16_t          nsgs;                  /* Number of subgoals  */ 
  U16_t          sel;                   /* Which compound is selected  */ 
  U16_t          level_num;             /* Actual Pst tree level */ 
  }  PsvTreeLvl_t;
#define PSVTREELVL_SIZE (sizeof (PsvTreeLvl_t))

/** Field Access Macros for PsvTreeLvl_t **/

/* Macro Prototypes

  Compound_t  **TreeLvl_CmpNodes_Get (PsvTreeLvl_t *);
  void          TreeLvl_CmpNodes_Put (PsvTreeLvl_t *, Compound_t **);
  U16_t         TreeLvl_FocusNd_Get  (PsvTreeLvl_t *);
  void          TreeLvl_FocusNd_Put  (PsvTreeLvl_t *, U16_t);
  U32_t         TreeLvl_FocusSG_Get  (PsvTreeLvl_t *);
  void          TreeLvl_FocusSG_Put  (PsvTreeLvl_t *, U32_t);
  U16_t         TreeLvl_HeadNd_Get   (PsvTreeLvl_t *);
  void          TreeLvl_HeadNd_Put   (PsvTreeLvl_t *, U16_t);
  Compound_t   *TreeLvl_IthCmpNd_Get (PsvTreeLvl_t *, U16_t);
  void          TreeLvl_IthCmpNd_Put (PsvTreeLvl_t *, U16_t, Compound_t *);
  U16_t         TreeLvl_LvlNum_Get   (PsvTreeLvl_t *);
  void          TreeLvl_LvlNum_Put   (PsvTreeLvl_t *, U16_t);
  U16_t         TreeLvl_NCmps_Get    (PsvTreeLvl_t *);
  void          TreeLvl_NCmps_Put    (PsvTreeLvl_t *, U16_t);
  U16_t         TreeLvl_NSGs_Get     (PsvTreeLvl_t *);
  void          TreeLvl_NSGs_Put     (PsvTreeLvl_t *, U16_t);
  U16_t         TreeLvl_SelNd_Get    (PsvTreeLvl_t *);
  void          TreeLvl_SelNd_Put    (PsvTreeLvl_t *, U16_t);
  U32_t         TreeLvl_SelSG_Get    (PsvTreeLvl_t *);
  void          TreeLvl_SelSG_Put    (PsvTreeLvl_t *, U32_t);
*/

#define TreeLvl_CmpNodes_Get(plp)\
  (plp)->nodes
#define TreeLvl_CmpNodes_Put(plp, value)\
  (plp)->nodes = (value)
#define TreeLvl_FocusNd_Get(plp)\
  (plp)->focus
#define TreeLvl_FocusNd_Put(plp, value)\
  (plp)->focus = (value)
#define TreeLvl_FocusSG_Get(plp)\
  (plp)->fcs_sg
#define TreeLvl_FocusSG_Put(plp, value)\
  (plp)->fcs_sg = (value)
#define TreeLvl_HeadNd_Get(plp)\
  (plp)->head
#define TreeLvl_HeadNd_Put(plp, value)\
  (plp)->head = (value)
#define TreeLvl_IthCmpNd_Get(plp, ith)\
  (plp)->nodes[ith]
#define TreeLvl_IthCmpNd_Put(plp, ith, value)\
  (plp)->nodes[ith] = (value)
#define TreeLvl_LvlNum_Get(plp)\
  (plp)->level_num
#define TreeLvl_LvlNum_Put(plp, value)\
  (plp)->level_num = (value)
#define TreeLvl_NCmps_Get(plp)\
  (plp)->ncmps
#define TreeLvl_NCmps_Put(plp, value)\
  (plp)->ncmps = (value)
#define TreeLvl_NSGs_Get(plp)\
  (plp)->nsgs
#define TreeLvl_NSGs_Put(plp, value)\
  (plp)->nsgs = (value)
#define TreeLvl_SelNd_Get(plp)\
  (plp)->sel
#define TreeLvl_SelNd_Put(plp, value)\
  (plp)->sel = (value)
#define TreeLvl_SelSG_Get(plp)\
  (plp)->sel_sg
#define TreeLvl_SelSG_Put(plp, value)\
  (plp)->sel_sg = (value)

/**  The PST View Level data structure.  **/

typedef struct psv_viewlvl_s
  {
  Widget         nsg_lbl;               /* Num of sgoals label */ 
  Widget         lvl_lbl;               /* Level number label */ 
  DrawCxt_t     *dc_p;                  /* Drawing context */ 
  PsvTreeLvl_t  *treelvl;               /* Tree level for this view level */ 
  RxnPreView_t   preview;               /* Reaction preview */ 
  Pixel          fcs_clr;               /* Color for this focus box */
  Pixel          sel_clr;               /* Color for this select box */
  U16_t          level_num;             /* View level (top, mid, or btm) */ 
  int            bar_y_beg;             /* Beginning position of bar */
  int            bar_y_end;             /* Ending position of bar */
  int            ln_y_beg;              /* Beg position of line or level num */
  int            ln_y_end;              /* End position of line or level num */
  int            sym_y_beg;             /* Beginning position of symbol */
  int            sym_y_end;             /* Ending position of symbol */
  }  PsvViewLvl_t;
#define PSVVIEWLVL_SIZE (sizeof (PsvViewLvl_t))

/** Field Access Macros for PsvLevel_t **/

/* Macro Prototypes

  int           ViewLvl_BarYBeg_Get  (PsvViewLvl_t *);
  void          ViewLvl_BarYBeg_Put  (PsvViewLvl_t *, int);
  int           ViewLvl_BarYEnd_Get  (PsvViewLvl_t *);
  void          ViewLvl_BarYEnd_Put  (PsvViewLvl_t *, int);
  DrawCxt_t    *ViewLvl_DrawCxt_Get  (PsvViewLvl_t *);
  void          ViewLvl_DrawCxt_Put  (PsvViewLvl_t *, DrawCxt_t *);
  Pixel         ViewLvl_FocusClr_Get (PsvViewLvl_t *);
  void          ViewLvl_FocusClr_Put (PsvViewLvl_t *, Pixel);
  int           ViewLvl_LnYBeg_Get   (PsvViewLvl_t *);
  void          ViewLvl_LnYBeg_Put   (PsvViewLvl_t *, int);
  int           ViewLvl_LnYEnd_Get   (PsvViewLvl_t *);
  void          ViewLvl_LnYEnd_Put   (PsvViewLvl_t *, int);
  U16_t         ViewLvl_LvlNum_Get   (PsvViewLvl_t *);
  void          ViewLvl_LvlNum_Put   (PsvViewLvl_t *, U16_t);
  Widget        ViewLvl_NSGLbl_Get   (PsvViewLvl_t *);
  void          ViewLvl_NSGLbl_Put   (PsvViewLvl_t *, Widget);
  RxnPreView_t *ViewLvl_RxnPV_Get    (PsvViewLvl_t *);
  Pixel         ViewLvl_SelClr_Get   (PsvViewLvl_t *);
  void          ViewLvl_SelClr_Put   (PsvViewLvl_t *, Pixel);
  int           ViewLvl_SymYBeg_Get  (PsvViewLvl_t *);
  void          ViewLvl_SymYBeg_Put  (PsvViewLvl_t *, int);
  int           ViewLvl_SymYEnd_Get  (PsvViewLvl_t *);
  void          ViewLvl_SymYEnd_Put  (PsvViewLvl_t *, int);
  Widget        ViewLvl_TLNLbl_Get   (PsvViewLvl_t *);
  void          ViewLvl_TLNLbl_Put   (PsvViewLvl_t *, Widget);
  PsvTreeLvl_t *ViewLvl_TreeLvl_Get  (PsvViewLvl_t *);
  void          ViewLvl_TreeLvl_Put  (PsvViewLvl_t *, PsvTreeLvl_t *);
*/

#define ViewLvl_BarYBeg_Get(plp)\
  (plp)->bar_y_beg
#define ViewLvl_BarYBeg_Put(plp, value)\
  (plp)->bar_y_beg = (value)
#define ViewLvl_BarYEnd_Get(plp)\
  (plp)->bar_y_end
#define ViewLvl_BarYEnd_Put(plp, value)\
  (plp)->bar_y_end = (value)
#define ViewLvl_DrawCxt_Get(plp)\
  (plp)->dc_p
#define ViewLvl_DrawCxt_Put(plp, value)\
  (plp)->dc_p = (value)
#define ViewLvl_FocusClr_Get(plp)\
  (plp)->fcs_clr
#define ViewLvl_FocusClr_Put(plp, value)\
  (plp)->fcs_clr = (value)
#define ViewLvl_LnYBeg_Get(plp)\
  (plp)->ln_y_beg
#define ViewLvl_LnYBeg_Put(plp, value)\
  (plp)->ln_y_beg = (value)
#define ViewLvl_LnYEnd_Get(plp)\
  (plp)->ln_y_end
#define ViewLvl_LnYEnd_Put(plp, value)\
  (plp)->ln_y_end = (value)
#define ViewLvl_LvlNum_Get(plp)\
  (plp)->level_num
#define ViewLvl_LvlNum_Put(plp, value)\
  (plp)->level_num = (value)
#define ViewLvl_NSGLbl_Get(plp)\
  (plp)->nsg_lbl
#define ViewLvl_NSGLbl_Put(plp, value)\
  (plp)->nsg_lbl = (value)
#define ViewLvl_RxnPV_Get(plp)\
  &((plp)->preview)
#define ViewLvl_SelClr_Get(plp)\
  (plp)->sel_clr
#define ViewLvl_SelClr_Put(plp, value)\
  (plp)->sel_clr = (value)
#define ViewLvl_SymYBeg_Get(plp)\
  (plp)->sym_y_beg
#define ViewLvl_SymYBeg_Put(plp, value)\
  (plp)->sym_y_beg = (value)
#define ViewLvl_SymYEnd_Get(plp)\
  (plp)->sym_y_end
#define ViewLvl_SymYEnd_Put(plp, value)\
  (plp)->sym_y_end = (value)
#define ViewLvl_TLNLbl_Get(plp)\
  (plp)->lvl_lbl
#define ViewLvl_TLNLbl_Put(plp, value)\
  (plp)->lvl_lbl = (value)
#define ViewLvl_TreeLvl_Get(plp)\
  (plp)->treelvl
#define ViewLvl_TreeLvl_Put(plp, value)\
  (plp)->treelvl = (value)

/**  The PST View data structure.  **/

typedef struct pst_view_s
  {
  Widget         clip_win;              /* Clip window of scrolling win */ 
  Widget         form;                  /* Form containing pst view */ 
  Widget         frame;                 /* Frame containing pst view form */ 
  Widget         lbl_bb;                /* Bulletin board for level labels */ 
  Widget         scrollbar;             /* Scrolled bar of scrolled win */ 
  Widget         scrollwin;             /* Scrolled window containing pst */ 
  Widget         spformdg;              /* Form containing surr pnt dialog */ 
  Compound_t    *last_sel_cmp;          /* Last selected compound */
  PstCB_t       *pstcb_p;               /* SYNCHEM PST control Block */
  PsvCmpInfo_t  *cmp_infos;             /* Compound info records */
  PsvTreeLvl_t **pathtlvls;             /* List of tree lvls along path */
  PsvViewLvl_t   vlvls[PVW_LEVEL_NUMOF]; /* View levels of PST */
  PsvViewLvl_t   pathlvl;               /* Path view level */
  RxnView_t      rxnview;               /* Reaction view data structure */
  SelTrace_t     sel_trace;             /* Select Trace data structure */
  CmpInst_t      cmp_inst;              /* Compound instances data structure */
  DrawCxt_t      pathdc;                /* Path drawing context */ 
  DrawCxt_t      pstdc;                 /* Pst tree drawing context */
  U32_t          num_cmps;              /* Number of compound info recs */
  U16_t          pthlvlnums[PVW_PATH_NUMSYMS_INIT]; /* Path level numbers */
  U16_t          leftmost_vsb;          /* Leftmost visible node in pst */
  Dimension      clip_h;                /* Height of clip window */
  Dimension      clip_w;                /* Width of clip window */
  Dimension      left_edge;             /* X-coord of left edge of vis da */
  U8_t           actvlvl;               /* Current active view level */
  U8_t           nactpv;                /* Number of active previewers */
  U8_t           pathlen;               /* Length of current path */
  U8_t           treetype;              /* Type of displayed tree */
  Boolean_t      psteh_act;             /* Is the pst event handler active? */
  Boolean_t      ptheh_act;             /* Is the path event handler active? */
  }
  PstView_t;
#define PSTVIEW_SIZE (sizeof (PstView_t))

/** Field Access Macros for PstView_t **/

/* Macro Prototypes

  U8_t           PstView_ActVLvl_Get  (PstView_t *);
  void           PstView_ActVLvl_Put  (PstView_t *, U8_t);
  Dimension      PstView_ClipH_Get    (PstView_t *);
  void           PstView_ClipH_Put    (PstView_t *, Dimension);
  Dimension      PstView_ClipW_Get    (PstView_t *);
  void           PstView_ClipW_Put    (PstView_t *, Dimension);
  Widget         PstView_ClipWin_Get  (PstView_t *);
  void           PstView_ClipWin_Put  (PstView_t *, Widget);
  PsvCmpInfo_t  *PstView_CmpInfo_Get  (PstView_t *);
  void           PstView_CmpInfo_Put  (PstView_t *, PsvCmpInfo_t *);
  CmpInst_t     *PstView_CmpInsts_Get (PstView_t *);
  Widget         PstView_Form_Get     (PstView_t *);
  void           PstView_Form_Put     (PstView_t *, Widget);
  Widget         PstView_Frame_Get    (PstView_t *);
  void           PstView_Frame_Put    (PstView_t *, Widget);
  PsvCmpInfo_t   PstView_IthCmpI_Get  (PstView_t *, U32_t);
  void           PstView_IthCmpI_Put  (PstView_t *, U32_t, PsvCmpInfo_t);
  U16_t          PstView_IthPthLN_Get (PstView_t *, U8_t);
  void           PstView_IthPthLN_Put (PstView_t *, U8_t, U16_t);
  PsvTreeLvl_t  *PstView_IthPTLvl_Get (PstView_t *, U8_t);
  void           PstView_IthPTLvl_Put (PstView_t *, U8_t, PsvTreeLvl_t *);
  PsvViewLvl_t  *PstView_IthVLvl_Get  (PstView_t *, U8_t);
  Widget         PstView_LabelBB_Get  (PstView_t *);
  void           PstView_LabelBB_Put  (PstView_t *, Widget);
  Compound      *PstView_LastSelCmp_Get (PstView_t *);
  void           PstView_LastSelCmp_Put (PstView_t *, Compound *);
  Dimension      PstView_LeftEdge_Get (PstView_t *);
  void           PstView_LeftEdge_Put (PstView_t *, Dimension);
  U16_t          PstView_LeftNd_Get   (PstView_t *);
  void           PstView_LeftNd_Put   (PstView_t *, U16_t);
  U8_t           PstView_NumActPV_Get (PstView_t *);
  void           PstView_NumActPV_Put (PstView_t *, U8_t);
  U32_t          PstView_NumCmpI_Get  (PstView_t *);
  void           PstView_NumCmpI_Put  (PstView_t *, U32_t);
  DrawCxt_t     *PstView_PathDC_Get   (PstView_t *);
  U8_t           PstView_PathLen_Get  (PstView_t *);
  void           PstView_PathLen_Put  (PstView_t *, U8_t);
  PsvViewLvl_t  *PstView_PathVLvl_Get (PstView_t *);
  PstCB_t       *PstView_PstCB_Get    (PstView_t *);
  void           PstView_PstCB_Put    (PstView_t *, PstCB_t *);
  DrawCxt_t     *PstView_PstDC_Get    (PstView_t *);
  Boolean_t      PstView_PstEHAct_Get (PstView_t *);
  void           PstView_PstEHAct_Put (PstView_t *, Boolean_t);
  Boolean_t      PstView_PthEHAct_Get (PstView_t *);
  void           PstView_PthEHAct_Put (PstView_t *, Boolean_t);
  PsvTreeLvl_t **PstView_PthTLvls_Get (PstView_t *);
  void           PstView_PthTLvls_Put (PstView_t *, PsvTreeLvl_t **);
  RxnView_t     *PstView_RxnView_Get  (PstView_t *);
  Widget         PstView_ScrollBar_Get (PstView_t *);
  void           PstView_ScrollBar_Put (PstView_t *, Widget);
  Widget         PstView_ScrollWin_Get (PstView_t *);
  void           PstView_ScrollWin_Put (PstView_t *, Widget);
  SelTrace_t    *PstView_SelTrace_Get (PstView_t *);
  U8_t           PstView_TreeType_Get (PstView_t *);
  void           PstView_TreeType_Put (PstView_t *, U8_t);
  PsvViewLvl_t  *PstView_ViewLvls_Get (PstView_t *);
*/

#define PstView_ActVLvl_Get(pvp)\
  (pvp)->actvlvl
#define PstView_ActVLvl_Put(pvp, value)\
  (pvp)->actvlvl = (value)
#define PstView_ClipH_Get(pvp)\
  (pvp)->clip_h
#define PstView_ClipH_Put(pvp, value)\
  (pvp)->clip_h = (value)
#define PstView_ClipW_Get(pvp)\
  (pvp)->clip_w
#define PstView_ClipW_Put(pvp, value)\
  (pvp)->clip_w = (value)
#define PstView_ClipWin_Get(pvp)\
  (pvp)->clip_win
#define PstView_ClipWin_Put(pvp, value)\
  (pvp)->clip_win = (value)
#define PstView_CmpInfo_Get(pvp)\
  (pvp)->cmp_infos
#define PstView_CmpInfo_Put(pvp, value)\
  (pvp)->cmp_infos = (value)
#define PstView_CmpInsts_Get(pvp)\
  &((pvp)->cmp_inst)
#define PstView_Form_Get(pvp)\
  (pvp)->form
#define PstView_Form_Put(pvp, value)\
  (pvp)->form = (value)
#define PstView_Frame_Get(pvp)\
  (pvp)->frame
#define PstView_Frame_Put(pvp, value)\
  (pvp)->frame = (value)
#define PstView_IthCmpI_Get(pvp, ith)\
  (pvp)->cmp_infos[ith]
#define PstView_IthCmpI_Put(pvp, ith, value)\
  (pvp)->cmp_infos[ith] = (value)
#define PstView_IthPthLN_Get(pvp, ith)\
  (pvp)->pthlvlnums[ith]
#define PstView_IthPthLN_Put(pvp, ith, value)\
  (pvp)->pthlvlnums[ith] = (value)
#define PstView_IthPTLvl_Get(pvp, ith)\
  (pvp)->pathtlvls[ith]
#define PstView_IthPTLvl_Put(pvp, ith, value)\
  (pvp)->pathtlvls[ith] = (value)
#define PstView_IthVLvl_Get(pvp, ith)\
  &((pvp)->vlvls[ith])
#define PstView_LabelBB_Get(pvp)\
  (pvp)->lbl_bb
#define PstView_LabelBB_Put(pvp, value)\
  (pvp)->lbl_bb = (value)
#define PstView_LastSelCmp_Get(pvp)\
  (pvp)->last_sel_cmp
#define PstView_LastSelCmp_Put(pvp, value)\
  (pvp)->last_sel_cmp = (value)
#define PstView_LeftEdge_Get(pvp)\
  (pvp)->left_edge
#define PstView_LeftEdge_Put(pvp, value)\
  (pvp)->left_edge = (value)
#define PstView_LeftNd_Get(pvp)\
  (pvp)->leftmost_vsb
#define PstView_LeftNd_Put(pvp, value)\
  (pvp)->leftmost_vsb = (value)
#define PstView_NumActPV_Get(pvp)\
  (pvp)->nactpv
#define PstView_NumActPV_Put(pvp, value)\
  (pvp)->nactpv = (value)
#define PstView_NumCmpI_Get(pvp)\
  (pvp)->num_cmps
#define PstView_NumCmpI_Put(pvp, value)\
  (pvp)->num_cmps = (value)
#define PstView_PathDC_Get(pvp)\
  &((pvp)->pathdc)
#define PstView_PathLen_Get(pvp)\
  (pvp)->pathlen
#define PstView_PathLen_Put(pvp, value)\
  (pvp)->pathlen = (value)
#define PstView_PathVLvl_Get(pvp)\
  &((pvp)->pathlvl)
#define PstView_PstCB_Get(pvp)\
  (pvp)->pstcb_p
#define PstView_PstCB_Put(pvp, value)\
  (pvp)->pstcb_p = (value)
#define PstView_PstDC_Get(pvp)\
  &((pvp)->pstdc)
#define PstView_PstEHAct_Get(pvp)\
  (pvp)->psteh_act
#define PstView_PstEHAct_Put(pvp, value)\
  (pvp)->psteh_act = (value)
#define PstView_PthEHAct_Get(pvp)\
  (pvp)->ptheh_act
#define PstView_PthEHAct_Put(pvp, value)\
  (pvp)->ptheh_act = (value)
#define PstView_PthTLvls_Get(pvp)\
  (pvp)->pathtlvls
#define PstView_PthTLvls_Put(pvp, value)\
  (pvp)->pathtlvls = (value)
#define PstView_RxnView_Get(pvp)\
  &((pvp)->rxnview)
#define PstView_ScrollBar_Get(pvp)\
  (pvp)->scrollbar
#define PstView_ScrollBar_Put(pvp, value)\
  (pvp)->scrollbar = (value)
#define PstView_ScrollWin_Get(pvp)\
  (pvp)->scrollwin
#define PstView_ScrollWin_Put(pvp, value)\
  (pvp)->scrollwin = (value)
#define PstView_SelTrace_Get(pvp)\
  &((pvp)->sel_trace)
#define PstView_TreeType_Get(pvp)\
  (pvp)->treetype
#define PstView_TreeType_Put(pvp, value)\
  (pvp)->treetype = (value)
#define PstView_ViewLvls_Get(pvp)\
  (pvp)->vlvls

/*** Macros ***/

/* Macro Prototypes

  Compound_t   *PstView_SelCmp_Get    (PstView_t *, U8_t);
  U16_t         PstView_SelNode_Get   (PstView_t *, U8_t);
*/

#define PstView_SelCmp_Get(pvp, lvl)\
  TreeLvl_IthCmpNd_Get (ViewLvl_TreeLvl_Get (PstView_IthVLvl_Get (pvp, lvl)),\
  TreeLvl_SelNd_Get (ViewLvl_TreeLvl_Get (PstView_IthVLvl_Get (pvp, lvl))))

#define PstView_SelNode_Get(pvp, lvl)\
  TreeLvl_SelNd_Get (ViewLvl_TreeLvl_Get (PstView_IthVLvl_Get (pvp, lvl)))

/*** Routine Prototypes ***/

void          PstView_Compound_Sel (PstView_t *);
Widget        PstView_Create       (Widget, Widget, PstView_t *);
void          PstView_Destroy      (PstView_t *);
void          PstView_Display      (PstView_t *);
void          PstView_Focus_Draw   (PsvViewLvl_t *);
void          PstView_Focus_Undraw (PsvViewLvl_t *);
void          PstView_Mark_Store   (PsvCmpInfo_t *, Compound_t *);
void          PstView_Mouse_Remove (PstView_t *);
void          PstView_Mouse_Reset  (PstView_t *, Display *);
void          PstView_PostInit     (Widget, PstView_t *);
void          PstView_Reset        (PstView_t *);
void          PstView_Trace_Store  (PsvCmpInfo_t *, Compound_t *, 
                U32_t, Boolean_t);
Boolean_t     PstView_Tree_Init    (PstView_t *, PstCB_t *);
void          PstView_Unmark_Store (PsvCmpInfo_t *, Compound_t *);
void          PstView_Visit_Store  (PsvCmpInfo_t *, Compound_t *);
void          PstVLvls_SetAll      (PstView_t *, Compound_t *, U16_t);
void          PstVLvls_SetTwo      (PstView_t *);
Boolean_t     PstVLvls_Update      (PstView_t *, U16_t, U8_t);
PsvTreeLvl_t *TreeLvl_Create       (Compound_t *, U16_t, U16_t, U8_t);
void          TreeLvl_Destroy      (PsvTreeLvl_t *);
U16_t         TreeLvls_Backup      (PstView_t *, Compound_t *);

/*  Routines defined for sel_trace.c (to prevent circular includes)  */
void          SelTrace_Create      (Widget, PstView_t *);
Boolean_t     SelTrace_Cycles_Set  (PstView_t *);
Boolean_t     SelTrace_Setup       (PstView_t *);
void          SelTrace_Update      (PstView_t *, U32_t);

/*  Routines defined for comp_inst.c (to prevent circular includes)  */
void         CmpInst_Create     (Widget, PstView_t *);
Boolean_t    CmpInst_Insts_Set  (PstView_t *);
Boolean_t    CmpInst_Setup      (PstView_t *);
void         CmpInst_Update     (PstView_t *, U16_t);

/* Event Handler Routine Prototypes */

void          PathView_Preview_EH (Widget, XtPointer, XEvent *, Boolean *);
void          PstView_Preview_EH  (Widget, XtPointer, XEvent *, Boolean *);

#endif
/*  End of PST_VIEW.H  */
