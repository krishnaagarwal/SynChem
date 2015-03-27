#ifndef _H_SUBMIT_DRAW_
#define _H_SUBMIT_DRAW_  1
/****************************************************************************
*  
* Copyright (C) 1997 Synchem Group at SUNY-Stony Brook, Daren Krebsbach  
*  
*  
*  Module Name:               SUBMIT_DRAW.H  
*  
*    This header file defines the information needed for the    
*    compound drawing panel of the job submission module.  
*      
*  Creation Date:  
*  
*     25-Aug-1997  
*  
*  Authors: 
*      Daren Krebsbach 
*        based on version by Nader Abdelsadek
*  
*****************************************************************************/  

#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_DSP_
#include "dsp.h"
#endif

#ifndef _H_SUBMIT_JOBINFO_
#include "submit_jobinfo.h" 
#endif

/** Used to be in submt_hlp.h **/ 
#define SMU_YES_RESPONSE   1
#define SMU_NO_RESPONSE    2

/*** Literal Values ***/  

#define DRW_DA_PXMP_WD         1800
#define DRW_DA_PXMP_HT          900

#define DRW_MOL_SCALE           1.55

#define DRW_ATOM_SIZE_LRG       0
#define DRW_ATOM_SIZE_NML       2
#define DRW_ATOM_SIZE_SML       3

#define DRW_DBL_BOND_OFFSET     2.87
#define DRW_STR_BOND_OFFSET     5.71

#define DRW_CLEAR_FLAG          2

#define SBM_MSG_CLEAR_ALL1      "Are you sure you want to "
#define SBM_MSG_CLEAR_ALL2      "CLEAR ALL drawings"
#define SBM_DLG_CLRALL_TITLE    "Clear All Dialog"

#define  CS_DRAW_BOND         "draw"
#define  CS_REDRAW	      "redraw"
#define  CS_SELECT	      "select"
#define  CS_SELECTALL	      "select all"
#define  CS_CLEAR             "delete"
#define  CS_CLEARALL          "clear"
#define  CS_CLEARSEL          "delete selected"
#define  CS_DONE              "done"
#define  CS_DISMISS           "Dismiss"
#define  CS_MULTIPLE          "Multiple"

#define  SBD_MODE_LBL         "Edit Mode:"
#define  SBD_EDIT_LBL         "Edit Options:"
#define  SBD_ATOM_LBL         "Atoms:"
#define  SBD_BOND_LBL         "Bonds:"

/*  The the following definitions for the Draw Menu are obsolete, and the
    Atom symbols are redundant.
*/

#define DRW_SNGLMOL_SW_WIDTH      800
#define DRW_RXN_SW_WIDTH          500
#define DRW_SNGLMOL_DA_WIDTH      1000
#define DRW_RXN_DA_WIDTH          800

#define DRW_BOND_FLAG             0
#define DRW_SELECT_FLAG           1
#define DRW_CLEAR_FLAG            2
#define DRW_REDRAW_FLAG           5
#define DRW_ATOM_FLAG             6
#define DRW_RXNCNTR_FLAG          7

#define DRW_ATOM_SIZE_LRG         0
#define DRW_ATOM_SIZE_NML         2
#define DRW_ATOM_SIZE_SML         3


/* Main menu definitions */
#define DRW_MENU_OPEN             11
#define DRW_MENU_SAVE             12
#define DRW_MENU_SAVE_AS          13
#define DRW_MENU_EXIT             14
#define DRW_MENU_SUBMIT           15

#define DRW_MENU_SELECT_ALL        21
#define DRW_MENU_DELETE_SEL        22
#define DRW_MENU_DELETE_ALL        23
#define DRW_MENU_DELETE_ALL_OK     24
#define DRW_MENU_DELETE_ALL_CANCEL 25

#define DRW_MENU_RETRACE          31
#define DRW_MENU_BOND_PLUS        32

#define DRW_MENU_CLICK            41
#define DRW_MENU_ENTER            42

#define DRW_MENU_RUN_PARAMS       51
#define DRW_MENU_STRATEGIES       52
#define DRW_MENU_LIBRARIES        53
#define DRW_MENU_PARAMS_OTHER     54

#define  CS_ELM_N             "N"
#define  CS_ELM_Na            "Na"		
#define  CS_ELM_H             "H"
#define  CS_ELM_O             "O"
#define  CS_ELM_F             "F"
#define  CS_ELM_P             "P"
#define  CS_ELM_S             "S"
#define  CS_ELM_Cl            "Cl"
#define  CS_ELM_Br            "Br"
#define  CS_ELM_I             "I"
#define  CS_ELM_X             "X"
#define  CS_ELM_R             "R"
#define  CS_ELM_R_P           "R"
#define  CS_ELM_DOT           "."
#define  CS_ELM_CLN           ":"
#define  CS_ELM_PLS           "+"
#define  CS_ELM_C             "C"
#define  CS_ELM_OTHER         "other"

#define  CS_ELM_He            "He"
#define  CS_ELM_Li            "Li"
#define  CS_ELM_Be            "Be"
#define  CS_ELM_B             "B"
#define  CS_ELM_Ne            "Ne"
#define  CS_ELM_Mg            "Mg"
#define  CS_ELM_Al            "Al"
#define  CS_ELM_Si            "Si"
#define  CS_ELM_Ar            "Ar"
#define  CS_ELM_K             "K"
#define  CS_ELM_Ca            "Ca"
#define  CS_ELM_Sc            "Sc"
#define  CS_ELM_Ti            "Ti"
#define  CS_ELM_V             "V"
#define  CS_ELM_Cr            "Cr"
#define  CS_ELM_Mn            "Mn"
#define  CS_ELM_Fe            "Fe"
#define  CS_ELM_Co            "Co"
#define  CS_ELM_Ni            "Ni"
#define  CS_ELM_Cu            "Cu"
#define  CS_ELM_Zn            "Zn"
#define  CS_ELM_Ga            "Ga"
#define  CS_ELM_Ge            "Ge"
#define  CS_ELM_As            "As"
#define  CS_ELM_Se            "Se"
#define  CS_ELM_Kr            "Kr"
#define  CS_ELM_Rb            "Rb"
#define  CS_ELM_Sr            "Sr"
#define  CS_ELM_Y             "Y"
#define  CS_ELM_Zr            "Zr"
#define  CS_ELM_Nb            "Nb"
#define  CS_ELM_Mo            "Mo"
#define  CS_ELM_Tc            "Tc"
#define  CS_ELM_Ru            "Ru"
#define  CS_ELM_Rh            "Rh"
#define  CS_ELM_Pd            "Pd"
#define  CS_ELM_Ag            "Ag"
#define  CS_ELM_Cd            "Cd"
#define  CS_ELM_In            "In"
#define  CS_ELM_Sn            "Sn"
#define  CS_ELM_Sb            "Sb"
#define  CS_ELM_Te            "Te"
#define  CS_ELM_Xe            "Xe"
#define  CS_ELM_Cs            "Cs"
#define  CS_ELM_Ba            "Ba"
#define  CS_ELM_La            "La"
#define  CS_ELM_Hf            "Hf"
#define  CS_ELM_Ta            "Ta"
#define  CS_ELM_W             "W"
#define  CS_ELM_Re            "Re"
#define  CS_ELM_Os            "Os"
#define  CS_ELM_Ir            "Ir"
#define  CS_ELM_Pt            "Pt"
#define  CS_ELM_Au            "Au"
#define  CS_ELM_Hg            "Hg"
#define  CS_ELM_Tl            "Tl"
#define  CS_ELM_Pb            "Pb"
#define  CS_ELM_Bi            "Bi"
#define  CS_ELM_Po            "Po"
#define  CS_ELM_At            "At"
#define  CS_ELM_Rn            "Rn"
#define  CS_ELM_Fr            "Fr"
#define  CS_ELM_Ra            "Ra"
#define  CS_ELM_Ac            "Ac"
#define  CS_ELM_Ce            "Ce"
#define  CS_ELM_Pr            "Pr"
#define  CS_ELM_Nd            "Nd"
#define  CS_ELM_Pm            "Pm"
#define  CS_ELM_Sm            "Sm"
#define  CS_ELM_Eu            "Eu"
#define  CS_ELM_Gd            "Gd"
#define  CS_ELM_Tb            "Tb"
#define  CS_ELM_Dy            "Dy"
#define  CS_ELM_Ho            "Ho"
#define  CS_ELM_Er            "Er"
#define  CS_ELM_Tm            "Tm"
#define  CS_ELM_Yb            "Yb"
#define  CS_ELM_Lu            "Lu"
#define  CS_ELM_Th            "Th"
#define  CS_ELM_Pa            "Pa"
#define  CS_ELM_U             "U"
#define  CS_ELM_Np            "Np"
#define  CS_ELM_Pu            "Pu"
#define  CS_ELM_Am            "Am"
#define  CS_ELM_Cm            "Cm"
#define  CS_ELM_Bk            "Bk"
#define  CS_ELM_Cf            "Cf"
#define  CS_ELM_Es            "Es"
#define  CS_ELM_Fm            "Fm"
#define  CS_ELM_Md            "Md"
#define  CS_ELM_No            "No"
#define  CS_ELM_Lw            "Lw"

#define CS_STEREO_OPP_DOWN    "stereo_opp_down"
#define CS_STEREO_DWN         "stereo_down"
#define CS_STEREO_OPP_UP      "stereo_opp_up"
#define CS_STEREO_UP          "stereo_up"
#define CS_SNGL_BOND          "sngl_bond"
#define CS_RSDNT_BOND         "rsdnt_bond"
#define CS_VARBL_BOND         "varbl_bond"
#define CS_ERASE_BOND         "erase_bond"
#define CS_DUBL_BOND          "dubl_bond"
#define CS_TRPL_BOND          "trpl_bond"

typedef struct s_draw_tool
  {
  GC              gc;               /* Drawing tool graphics context */
  Pixmap          pixmap;           /* Pixmap used by the drawing area */
  Widget          form;             /* Main drawing tool form */
  Widget          frame;
  Widget          atom_form;
  Widget          atom_lbl;
  Widget          atm_b_pb;
  Widget          atm_br_pb;
  Widget          atm_c_pb;
  Widget          atm_cl_pb;
  Widget          atm_f_pb;
  Widget          atm_h_pb;
  Widget          atm_i_pb;
  Widget          atm_n_pb;
  Widget          atm_o_pb;
  Widget          atm_other_pb;
  Widget          atm_p_pb;
  Widget          atm_s_pb;
  Widget          bond_form;
  Widget          bnd_lft_lbl;
  Widget          bnd_lsgl_pb;
  Widget          bnd_ldbl_pb;
  Widget          bnd_ltpl_pb;
  Widget          bnd_mid_lbl;
  Widget          bnd_mid_rb;
  Widget          edit_form;
  Widget          edt_mode_lbl;
  Widget          edt_mode_rb;
  Widget          edt_opt_lbl;
  Widget          edt_selall_pb;
  Widget          edt_delsel_pb;
  Widget          scrolled_window;
  Widget          drawing_area;     /* Drawing Area widget */
  Widget          pb_form;
  Widget          clear_pb;
  Widget          done_pb;
  Widget          redraw_pb;
  Widget          periodic_tbl;
  Dsp_Molecule_t *mol_p;            /* Pointer to molecule structure */
  }  DrawTool_t; 

/* Macro Prototypes for DrawTool_t

Widget          DrawTool_AtomForm_Get    (DrawTool_t *);
void            DrawTool_AtomForm_Put    (DrawTool_t *, Widget);
Widget          DrawTool_AtomLbl_Get     (DrawTool_t *);
void            DrawTool_AtomLbl_Put     (DrawTool_t *, Widget);
Widget          DrawTool_Atm_B_PB_Get    (DrawTool_t *);
void            DrawTool_Atm_B_PB_Put    (DrawTool_t *, Widget);
Widget          DrawTool_Atm_Br_PB_Get   (DrawTool_t *);
void            DrawTool_Atm_Br_PB_Put   (DrawTool_t *, Widget);
Widget          DrawTool_Atm_C_PB_Get    (DrawTool_t *);
void            DrawTool_Atm_C_PB_Put    (DrawTool_t *, Widget);
Widget          DrawTool_Atm_Cl_PB_Get   (DrawTool_t *);
void            DrawTool_Atm_Cl_PB_Put   (DrawTool_t *, Widget);
Widget          DrawTool_Atm_F_PB_Get    (DrawTool_t *);
void            DrawTool_Atm_F_PB_Put    (DrawTool_t *, Widget);
Widget          DrawTool_Atm_H_PB_Get    (DrawTool_t *);
void            DrawTool_Atm_H_PB_Put    (DrawTool_t *, Widget);
Widget          DrawTool_Atm_I_PB_Get    (DrawTool_t *);
void            DrawTool_Atm_I_PB_Put    (DrawTool_t *, Widget);
Widget          DrawTool_Atm_N_PB_Get    (DrawTool_t *);
void            DrawTool_Atm_N_PB_Put    (DrawTool_t *, Widget);
Widget          DrawTool_Atm_O_PB_Get    (DrawTool_t *);
void            DrawTool_Atm_O_PB_Put    (DrawTool_t *, Widget);
Widget          DrawTool_Atm_Other_PB_Get (DrawTool_t *);
void            DrawTool_Atm_Other_PB_Put (DrawTool_t *, Widget);
Widget          DrawTool_Atm_P_PB_Get    (DrawTool_t *);
void            DrawTool_Atm_P_PB_Put    (DrawTool_t *, Widget);
Widget          DrawTool_Atm_S_PB_Get    (DrawTool_t *);
void            DrawTool_Atm_S_PB_Put    (DrawTool_t *, Widget);
Widget          DrawTool_BondForm_Get    (DrawTool_t *);
void            DrawTool_BondForm_Put    (DrawTool_t *, Widget);
Widget          DrawTool_BndLftLbl_Get   (DrawTool_t *);
void            DrawTool_BndLftLbl_Put   (DrawTool_t *, Widget);
Widget          DrawTool_BndLDblPB_Get   (DrawTool_t *);
void            DrawTool_BndLDblPB_Put   (DrawTool_t *, Widget);
Widget          DrawTool_BndLSglPB_Get   (DrawTool_t *);
void            DrawTool_BndLSglPB_Put   (DrawTool_t *, Widget);
Widget          DrawTool_BndLTplPB_Get   (DrawTool_t *);
void            DrawTool_BndLTplPB_Put   (DrawTool_t *, Widget);
Widget          DrawTool_BndMidLbl_Get   (DrawTool_t *);
void            DrawTool_BndMidLbl_Put   (DrawTool_t *, Widget);
Widget          DrawTool_BndMidRB_Get    (DrawTool_t *);
void            DrawTool_BndMidRB_Put    (DrawTool_t *, Widget);
Widget          DrawTool_ClearPB_Get     (DrawTool_t *);
void            DrawTool_ClearPB_Put     (DrawTool_t *, Widget);
Widget          DrawTool_DonePB_Get      (DrawTool_t *);
void            DrawTool_DonePB_Put      (DrawTool_t *, Widget);
Widget          DrawTool_DrawArea_Get    (DrawTool_t *);
void            DrawTool_DrawArea_Put    (DrawTool_t *, Widget);
Widget          DrawTool_EditDelSelPB_Get (DrawTool_t *);
void            DrawTool_EditDelSelPB_Put (DrawTool_t *, Widget);
Widget          DrawTool_EditForm_Get    (DrawTool_t *);
void            DrawTool_EditForm_Put    (DrawTool_t *, Widget);
Widget          DrawTool_EditModeLbl_Get (DrawTool_t *);
void            DrawTool_EditModeLbl_Put (DrawTool_t *, Widget);
Widget          DrawTool_EditModeRB_Get  (DrawTool_t *);
void            DrawTool_EditModeRB_Put  (DrawTool_t *, Widget);
Widget          DrawTool_EditOptLbl_Get  (DrawTool_t *);
void            DrawTool_EditOptLbl_Put  (DrawTool_t *, Widget);
Widget          DrawTool_EditSelAllPB_Get (DrawTool_t *);
void            DrawTool_EditSelAllPB_Put (DrawTool_t *, Widget);
Widget          DrawTool_Form_Get        (DrawTool_t *);
void            DrawTool_Form_Put        (DrawTool_t *, Widget);
Widget          DrawTool_Frame_Get       (DrawTool_t *);
void            DrawTool_Frame_Put       (DrawTool_t *, Widget);
Widget          DrawTool_GC_Get          (DrawTool_t *);
void            DrawTool_GC_Put          (DrawTool_t *, GC);
Dsp_Molecule_t *DrawTool_Molecule_Get    (DrawTool_t *);
void            DrawTool_Molecule_Put    (DrawTool_t *, Dsp_Molecule_t *);
Widget          DrawTool_PBForm_Get      (DrawTool_t *);
void            DrawTool_PBForm_Put      (DrawTool_t *, Widget);
Widget          DrawTool_PeriodicTbl_Get (DrawTool_t *);
void            DrawTool_PeriodicTbl_Put (DrawTool_t *, Widget);
Widget          DrawTool_Pixmap_Get      (DrawTool_t *);
void            DrawTool_Pixmap_Put      (DrawTool_t *, Pixmap);
Widget          DrawTool_RedrawPB_Get    (DrawTool_t *);
void            DrawTool_RedrawPB_Put    (DrawTool_t *, Widget);
Widget          DrawTool_ScrlldWin_Get   (DrawTool_t *);
void            DrawTool_ScrlldWin_Put   (DrawTool_t *, Widget);
*/

#define DrawTool_AtomForm_Get(draw_p)\
  (draw_p)->atom_form
#define DrawTool_AtomForm_Put(draw_p, value)\
  (draw_p)->atom_form = (value)
#define DrawTool_AtomLbl_Get(draw_p)\
  (draw_p)->atom_lbl
#define DrawTool_AtomLbl_Put(draw_p, value)\
  (draw_p)->atom_lbl = (value)
#define DrawTool_Atm_B_PB_Get(draw_p)\
  (draw_p)->atm_b_pb
#define DrawTool_Atm_B_PB_Put(draw_p, value)\
  (draw_p)->atm_b_pb = (value)
#define DrawTool_Atm_Br_PB_Get(draw_p)\
  (draw_p)->atm_br_pb
#define DrawTool_Atm_Br_PB_Put(draw_p, value)\
  (draw_p)->atm_br_pb = (value)
#define DrawTool_Atm_C_PB_Get(draw_p)\
  (draw_p)->atm_c_pb
#define DrawTool_Atm_C_PB_Put(draw_p, value)\
  (draw_p)->atm_c_pb = (value)
#define DrawTool_Atm_Cl_PB_Get(draw_p)\
  (draw_p)->atm_cl_pb
#define DrawTool_Atm_Cl_PB_Put(draw_p, value)\
  (draw_p)->atm_cl_pb = (value)
#define DrawTool_Atm_F_PB_Get(draw_p)\
  (draw_p)->atm_f_pb
#define DrawTool_Atm_F_PB_Put(draw_p, value)\
  (draw_p)->atm_f_pb = (value)
#define DrawTool_Atm_H_PB_Get(draw_p)\
  (draw_p)->atm_h_pb
#define DrawTool_Atm_H_PB_Put(draw_p, value)\
  (draw_p)->atm_h_pb = (value)
#define DrawTool_Atm_I_PB_Get(draw_p)\
  (draw_p)->atm_i_pb
#define DrawTool_Atm_I_PB_Put(draw_p, value)\
  (draw_p)->atm_i_pb = (value)
#define DrawTool_Atm_N_PB_Get(draw_p)\
  (draw_p)->atm_n_pb
#define DrawTool_Atm_N_PB_Put(draw_p, value)\
  (draw_p)->atm_n_pb = (value)
#define DrawTool_Atm_O_PB_Get(draw_p)\
  (draw_p)->atm_o_pb
#define DrawTool_Atm_O_PB_Put(draw_p, value)\
  (draw_p)->atm_o_pb = (value)
#define DrawTool_Atm_Other_PB_Get(draw_p)\
  (draw_p)->atm_other_pb
#define DrawTool_Atm_Other_PB_Put(draw_p, value)\
  (draw_p)->atm_other_pb = (value)
#define DrawTool_Atm_P_PB_Get(draw_p)\
  (draw_p)->atm_p_pb
#define DrawTool_Atm_P_PB_Put(draw_p, value)\
  (draw_p)->atm_p_pb = (value)
#define DrawTool_Atm_S_PB_Get(draw_p)\
  (draw_p)->atm_s_pb
#define DrawTool_Atm_S_PB_Put(draw_p, value)\
  (draw_p)->atm_s_pb = (value)
#define DrawTool_BondForm_Get(draw_p)\
  (draw_p)->bond_form
#define DrawTool_BondForm_Put(draw_p, value)\
  (draw_p)->bond_form = (value)
#define DrawTool_BndLftLbl_Get(draw_p)\
  (draw_p)->bnd_lft_lbl
#define DrawTool_BndLftLbl_Put(draw_p, value)\
  (draw_p)->bnd_lft_lbl = (value)
#define DrawTool_BndLDblPB_Get(draw_p)\
  (draw_p)->bnd_ldbl_pb
#define DrawTool_BndLDblPB_Put(draw_p, value)\
  (draw_p)->bnd_ldbl_pb = (value)
#define DrawTool_BndLSglPB_Get(draw_p)\
  (draw_p)->bnd_lsgl_pb
#define DrawTool_BndLSglPB_Put(draw_p, value)\
  (draw_p)->bnd_lsgl_pb = (value)
#define DrawTool_BndLTplPB_Get(draw_p)\
  (draw_p)->bnd_ltpl_pb
#define DrawTool_BndLTplPB_Put(draw_p, value)\
  (draw_p)->bnd_ltpl_pb = (value)
#define DrawTool_BndMidLbl_Get(draw_p)\
  (draw_p)->bnd_mid_lbl
#define DrawTool_BndMidLbl_Put(draw_p, value)\
  (draw_p)->bnd_mid_lbl = (value)
#define DrawTool_BndMidRB_Get(draw_p)\
  (draw_p)->bnd_mid_rb
#define DrawTool_BndMidRB_Put(draw_p, value)\
  (draw_p)->bnd_mid_rb = (value)
#define DrawTool_ClearPB_Get(draw_p)\
  (draw_p)->clear_pb
#define DrawTool_ClearPB_Put(draw_p, value)\
  (draw_p)->clear_pb = (value)
#define DrawTool_DonePB_Get(draw_p)\
  (draw_p)->done_pb
#define DrawTool_DonePB_Put(draw_p, value)\
  (draw_p)->done_pb = (value)
#define DrawTool_DrawArea_Get(draw_p)\
  (draw_p)->drawing_area
#define DrawTool_DrawArea_Put(draw_p, value)\
  (draw_p)->drawing_area = (value)
#define DrawTool_EditDelSelPB_Get(draw_p)\
  (draw_p)->edt_delsel_pb
#define DrawTool_EditDelSelPB_Put(draw_p, value)\
  (draw_p)->edt_delsel_pb = (value)
#define DrawTool_EditForm_Get(draw_p)\
  (draw_p)->edit_form
#define DrawTool_EditForm_Put(draw_p, value)\
  (draw_p)->edit_form = (value)
#define DrawTool_EditModeLbl_Get(draw_p)\
  (draw_p)->edt_mode_lbl
#define DrawTool_EditModeLbl_Put(draw_p, value)\
  (draw_p)->edt_mode_lbl = (value)
#define DrawTool_EditModeRB_Get(draw_p)\
  (draw_p)->edt_mode_rb
#define DrawTool_EditModeRB_Put(draw_p, value)\
  (draw_p)->edt_mode_rb = (value)
#define DrawTool_EditOptLbl_Get(draw_p)\
  (draw_p)->edt_opt_lbl
#define DrawTool_EditOptLbl_Put(draw_p, value)\
  (draw_p)->edt_opt_lbl = (value)
#define DrawTool_EditSelAllPB_Get(draw_p)\
  (draw_p)->edt_selall_pb
#define DrawTool_EditSelAllPB_Put(draw_p, value)\
  (draw_p)->edt_selall_pb = (value)
#define DrawTool_Form_Get(draw_p)\
  (draw_p)->form
#define DrawTool_Form_Put(draw_p, value)\
  (draw_p)->form = (value)
#define DrawTool_Frame_Get(draw_p)\
  (draw_p)->frame
#define DrawTool_Frame_Put(draw_p, value)\
  (draw_p)->frame = (value)
#define DrawTool_GC_Get(draw_p)\
  (draw_p)->gc
#define DrawTool_GC_Put(draw_p, value)\
  (draw_p)->gc = (value)
#define DrawTool_Molecule_Get(draw_p)\
  (draw_p)->mol_p
#define DrawTool_Molecule_Put(draw_p, value)\
  (draw_p)->mol_p = (value)
#define DrawTool_PBForm_Get(draw_p)\
  (draw_p)->pb_form
#define DrawTool_PBForm_Put(draw_p, value)\
  (draw_p)->pb_form = (value)
#define DrawTool_PeriodicTbl_Get(draw_p)\
  (draw_p)->periodic_tbl
#define DrawTool_PeriodicTbl_Put(draw_p, value)\
  (draw_p)->periodic_tbl = (value)
#define DrawTool_Pixmap_Get(draw_p)\
  (draw_p)->pixmap
#define DrawTool_Pixmap_Put(draw_p, value)\
  (draw_p)->pixmap = (value)
#define DrawTool_RedrawPB_Get(draw_p)\
  (draw_p)->redraw_pb
#define DrawTool_RedrawPB_Put(draw_p, value)\
  (draw_p)->redraw_pb = (value)
#define DrawTool_ScrlldWin_Get(draw_p)\
  (draw_p)->scrolled_window
#define DrawTool_ScrlldWin_Put(draw_p, value)\
  (draw_p)->scrolled_window = (value)

#ifdef DRAW_GLOBALS

const String atoms_list[12] = 
  {
  CS_ELM_C,  CS_ELM_H,  CS_ELM_N,  
  CS_ELM_O,  CS_ELM_S,  CS_ELM_P, 
  CS_ELM_Cl, CS_ELM_F,  CS_ELM_Br, 
  CS_ELM_I,  CS_ELM_B,  CS_ELM_OTHER
  };

const String Li_Ra_Elms[12] =
  {
  CS_ELM_Li, CS_ELM_Na, CS_ELM_K, CS_ELM_Rb,
  CS_ELM_Cs, CS_ELM_Fr, CS_ELM_Be, CS_ELM_Mg,
  CS_ELM_Ca, CS_ELM_Sr, CS_ELM_Ba, CS_ELM_Ra
  };

const String B_Ar_Elms[12] =
  {
  CS_ELM_B, CS_ELM_Al, CS_ELM_C , CS_ELM_Si,
  CS_ELM_N, CS_ELM_P , CS_ELM_O , CS_ELM_S ,  
  CS_ELM_F, CS_ELM_Cl, CS_ELM_Ne, CS_ELM_Ar
  };

const String Sc_Rn_Elms[48] =
  {
  CS_ELM_Sc, CS_ELM_Y , CS_ELM_La, CS_ELM_Ti, CS_ELM_Zr, CS_ELM_Hf, 
  CS_ELM_V , CS_ELM_Nb, CS_ELM_Ta, CS_ELM_Cr, CS_ELM_Mo, CS_ELM_W, 
  CS_ELM_Mn, CS_ELM_Tc, CS_ELM_Re, CS_ELM_Fe, CS_ELM_Ru, CS_ELM_Os, 
  CS_ELM_Co, CS_ELM_Rh, CS_ELM_Ir, CS_ELM_Ni, CS_ELM_Pd, CS_ELM_Pt, 
  CS_ELM_Cu, CS_ELM_Ag, CS_ELM_Au, CS_ELM_Zn, CS_ELM_Cd, CS_ELM_Hg, 
  CS_ELM_Ga, CS_ELM_In, CS_ELM_Tl, CS_ELM_Ge, CS_ELM_Sn, CS_ELM_Pb, 
  CS_ELM_As, CS_ELM_Sb, CS_ELM_Bi, CS_ELM_Se, CS_ELM_Te, CS_ELM_Po, 
  CS_ELM_Br, CS_ELM_I , CS_ELM_At, CS_ELM_Kr, CS_ELM_Xe, CS_ELM_Rn 
  };

const String Ce_Lw_Elms[28] =
  {
  CS_ELM_Ce, CS_ELM_Th, CS_ELM_Pr, CS_ELM_Pa, CS_ELM_Nd, CS_ELM_U ,
  CS_ELM_Pm, CS_ELM_Np, CS_ELM_Sm, CS_ELM_Pu, CS_ELM_Eu, CS_ELM_Am, 
  CS_ELM_Gd, CS_ELM_Cm, CS_ELM_Tb, CS_ELM_Bk, CS_ELM_Dy, CS_ELM_Cf, 
  CS_ELM_Ho, CS_ELM_Es, CS_ELM_Er, CS_ELM_Fm, CS_ELM_Tm, CS_ELM_Md, 
  CS_ELM_Yb, CS_ELM_No, CS_ELM_Lu, CS_ELM_Lw
  };  

const String bond_bitmaps[3] =
  {
  CS_SNGL_BOND,   CS_DUBL_BOND,  CS_TRPL_BOND,
  };

#else
extern const String atoms_list[12];
extern const String Li_Ra_Elms[12];
extern const String B_Ar_Elms[12];
extern const String Sc_Rn_Elms[48];
extern const String Ce_Lw_Elms[28];
extern const String bond_bitmaps[3];
#endif

/*** Globally Defined DrawTool Control Block ***/

#ifdef DRAW_GLOBALS
void (*done_pb_function)(Widget, XtPointer, XtPointer);

static  DrawTool_t GDrawToolCB;
#else
extern void (*done_pb_function)(Widget, XtPointer, XtPointer);
#endif

/* Macro Prototypes

int  DRW_Slope  (int, int, int, int, int);
*/

#define DRW_Slope(x0,x1,y1,x2,y2) ((y2-y1)*(x0-x1)/(x2-x1)+y1)


/*** Function Prototypes ***/


void   Atom_Copy            (Dsp_Atom_t  *, Dsp_Atom_t  *);
void   CreateOptionMenu     (Widget, char *, char *, int, int, int, int);
Widget Draw_Tool_Create     (JobInfoCB_t *, Widget, XtAppContext *);
void   Draw_Tool_Initialize (void);
void   DrawTool_Destroy     (void);
void   Pixmap_Install       (Widget, char *);
void   Periodic_TblDlg_Create (Widget);
void   MolDraw_Sling_Draw   (Sling_t);

void draw_Double_Bond (Display *, Drawable, GC, int, int, int, int);
void draw_Stereo_Bond (Display *, Drawable, GC, int, int, int, int, int);
void draw_Bond        (Display *, Drawable, GC, XColor, Dsp_Molecule_t *,
       Dsp_Bond_t *, Boolean_t, Boolean_t);
void draw_Atom        (Display *, Drawable, GC, XColor, Dsp_Atom_t  *, int);
void erase_Bond       (Display *, Drawable, GC, Dsp_Molecule_t *, Dsp_Bond_t *);
void draw_Molecule    (Display *, Drawable, GC, Dsp_Molecule_t  *, float,
       Dimension, Dimension, Boolean_t);

Xtr_t          *New_Dsp2Xtr      (Dsp_Molecule_t *mol_p);
Tsd_t          *Dsp2Tsd          (Dsp_Molecule_t *mol_p);
Sling_t         Sling2CanonSling (Sling_t);
Dsp_Molecule_t *Molecule_Double  (Dsp_Molecule_t *);
Dsp_Bond_t     *store_BondNew    (Dsp_Molecule_t  *, int, int, int, int, int);
Dsp_Atom_t     *store_AtomNew    (Dsp_Molecule_t *, char *, char *, char *, 
                  int, int);

/** Callback Prototypes **/ 

void Draw_Mode_Reset         (int);
void redraw                  (Widget, XtPointer, XtPointer);
void clear_it                (Widget, XtPointer, XtPointer);	       
void Done_CB                 (Widget, XtPointer, XtPointer);
void PixMap_Display_CB       (Widget, XtPointer, XtPointer);
void PixMap_Resize_CB        (Widget, XtPointer, XtPointer);
void Atom_Mark_CB            (Widget, XtPointer, XtPointer);
void Option_Choose_CB        (Widget, XtPointer, XtPointer);
void Menu_Choice_CB          (Widget, XtPointer, XtPointer);
void Periodic_TblDlg_Show_CB (Widget, XtPointer, XtPointer);
void Selected_Bond_Draw_CB   (Widget, XtPointer, XtPointer);
void MOLDRAW_StereoChem_CB   (Widget, XtPointer, XtPointer);

#endif
