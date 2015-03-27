#ifndef _H_RXNPATT_DRW_
#define _H_RXNPATT_DRW_  1
/****************************************************************************
*  
* Copyright (C) 1999 Synchem Group at SUNY-Stony Brook, Jerry Miller  
*  
*  
*  Module Name:               RXNPATT_DRW.H  
*  
*    This header file defines the information needed for the    
*    compound drawing panel of the job submission module.  
*      
*  Creation Date:  
*  
*     11-Feb-1999  
*  
*  Authors: 
*      Jerry Miller
*        based on submit_draw.h by Daren Krebsbach 
*        based on version by Nader Abdelsadek
*  
*****************************************************************************/  

#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_RXNDSP_
#include "rxndsp.h"
#endif

#define FGFILE FCB_SEQDIR_FNGP ("/fgdata.isam")

/** Used to be in submt_hlp.h **/ 
#define SMU_PYES_RESPONSE   1
#define SMU_PNO_RESPONSE    2

/*** Bond Macro ***/

#define GS_NEWBOND(which, bord)\
  gsgsame || gsg == (which) ? (bord) : DSP_BOND_ERASE

/*** Literal Values ***/  

#define RXN_SLINGBUF_MAXLEN      255

#define PDRW_HILIT_AREA          DRW_HILIT_AREA
#define PDRW_SELECT_TOTAL        DRW_SELECT_TOTAL
#define PDRW_SELECT_MOVE         DRW_SELECT_MOVE
#define PDRW_SELECT_NONE         DRW_SELECT_NONE
#define PDRW_SELECT_FORMERLY     123

#define PDRW_DA_PXMP_WD          1000
#define PDRW_DA_PXMP_HT          1000
/*
#define PDRW_DA_LIST_WD          900
#define PDRW_DA_LIST_HT          900
*/
#define PDRW_MOL_SCALE           1.55

#define PDRW_ATOM_SIZE_LRG       0
#define PDRW_ATOM_SIZE_NML       2
#define PDRW_ATOM_SIZE_SML       3

#define PDRW_DBL_BOND_OFFSET     2.87
#define PDRW_STR_BOND_OFFSET     5.71
/*
#define PDRW_CLEAR_FLAG          3
*/
#define SBM_PMSG_CLEAR_ALL1      "Are you sure you want to "
#define SBM_PMSG_CLEAR_ALL2      "CLEAR ALL drawings"
#define SBM_PDLG_CLRALL_TITLE    "Clear All Dialog"

#define  CS_PDRAW_BOND         "draw"
#define  CS_PREDRAW           "redraw"
#define  CS_PSELECT           "select"
#define  CS_PSELECTROOT       "select root"
#define  CS_PDELETEROOT       "delete root"
#define  CS_PADDFGS           "add functional groups"
#define  CS_PSELECTALL        "select all"
#define  CS_PCLEAR             "delete"
#define  CS_PCLEARALL          "clear"
#define  CS_PCLEARSEL          "delete selected"
#define  CS_PHELP              "Help"
#define  CS_PDONE              "Exit; Update Buffer"
#define  CS_PQUIT              "Quit and Cancel"
#define  CS_PISOLATE           "isolate"
#define  CS_PNUMTOGL           "toggle #"
#define  CS_PDISMISS           "Dismiss"
#define  CS_PMULTIPLE          "Multiple"

#define  SBD_PMODE_LBL         "Edit Mode:"
#define  SBD_PEDIT_LBL         "Edit Options:"
#define  SBD_PATOM_LBL         "Atoms:"
#define  SBD_PBOND_LBL         "Bonds:"
#define  SBD_PGOAL_LBL         "  Goal Pattern:"
#define  SBD_PSGOAL_LBL        "  Subgoal Pattern:"
#define  SBD_PSYNTHEME_LBL     "List of Synthemes:"
#define  SBD_PROOTSYN_LBL      "Roots & Synthemes:"

/*  The the following definitions for the Draw Menu are obsolete, and the
    Atom symbols are redundant.
*/

#define PDRW_SNGLMOL_SW_WIDTH      800
#define PDRW_RXN_SW_WIDTH          500
#define PDRW_SNGLMOL_DA_WIDTH      1000
#define PDRW_RXN_DA_WIDTH          800

#define PDRW_BOND_FLAG             0
#define PDRW_SELECT_FLAG           1
#define PDRW_SELECTROOT_FLAG       2
#define PDRW_CLEAR_FLAG            3
#define PDRW_DELETEROOT_FLAG       4
#define PDRW_REDRAW_FLAG           5
#define PDRW_ATOM_FLAG             6
#define PDRW_RXNCNTR_FLAG          7
#define PDRW_SYNWAIT_FLAG          8
#define PDRW_SYNSELECT_FLAG        9

#define PDRW_ATOM_SIZE_LRG         0
#define PDRW_ATOM_SIZE_NML         2
#define PDRW_ATOM_SIZE_SML         3


/* Main menu definitions */
#define PDRW_MENU_OPEN             11
#define PDRW_MENU_SAVE             12
#define PDRW_MENU_SAVE_AS          13
#define PDRW_MENU_EXIT             14
#define PDRW_MENU_SUBMIT           15

#define PDRW_MENU_SELECT_ALL        21
#define PDRW_MENU_DELETE_SEL        22
#define PDRW_MENU_DELETE_ALL        23
#define PDRW_MENU_DELETE_ALL_OK     24
#define PDRW_MENU_DELETE_ALL_CANCEL 25

#define PDRW_MENU_RETRACE          31
#define PDRW_MENU_BOND_PLUS        32

#define PDRW_MENU_CLICK            41
#define PDRW_MENU_ENTER            42

#define PDRW_MENU_RUN_PARAMS       51
#define PDRW_MENU_STRATEGIES       52
#define PDRW_MENU_LIBRARIES        53
#define PDRW_MENU_PARAMS_OTHER     54

#define  CS_PELM_N             "N"
#define  CS_PELM_Na            "Na"             
#define  CS_PELM_H             "H"
#define  CS_PELM_O             "O"
#define  CS_PELM_F             "F"
#define  CS_PELM_P             "P"
#define  CS_PELM_S             "S"
#define  CS_PELM_Cl            "Cl"
#define  CS_PELM_Br            "Br"
#define  CS_PELM_I             "I"
#define  CS_PELM_X             "X"
#define  CS_PELM_R             "R"
#define  CS_PELM_R_P           "R'"
#define  CS_PELM_DOT           "."
#define  CS_PELM_CLN           ":"
#define  CS_PELM_DOTLBL        "\267(lone e\257)"
#define  CS_PELM_CLNLBL        ": (e\257 pair)"
#define  CS_PELM_PLS           "+"
#define  CS_PELM_C             "C"
#define  CS_PELM_CH            "Ch"
#define  CS_PELM_CHALCOGEN     "Ch (chalcogen)"
#define  CS_PELM_OTHER         "other"

#define  CS_PELM_He            "He"
#define  CS_PELM_Li            "Li"
#define  CS_PELM_Be            "Be"
#define  CS_PELM_B             "B"
#define  CS_PELM_Ne            "Ne"
#define  CS_PELM_Mg            "Mg"
#define  CS_PELM_Al            "Al"
#define  CS_PELM_Si            "Si"
#define  CS_PELM_Ar            "Ar"
#define  CS_PELM_K             "K"
#define  CS_PELM_Ca            "Ca"
#define  CS_PELM_Sc            "Sc"
#define  CS_PELM_Ti            "Ti"
#define  CS_PELM_V             "V"
#define  CS_PELM_Cr            "Cr"
#define  CS_PELM_Mn            "Mn"
#define  CS_PELM_Fe            "Fe"
#define  CS_PELM_Co            "Co"
#define  CS_PELM_Ni            "Ni"
#define  CS_PELM_Cu            "Cu"
#define  CS_PELM_Zn            "Zn"
#define  CS_PELM_Ga            "Ga"
#define  CS_PELM_Ge            "Ge"
#define  CS_PELM_As            "As"
#define  CS_PELM_Se            "Se"
#define  CS_PELM_Kr            "Kr"
#define  CS_PELM_Rb            "Rb"
#define  CS_PELM_Sr            "Sr"
#define  CS_PELM_Y             "Y"
#define  CS_PELM_Zr            "Zr"
#define  CS_PELM_Nb            "Nb"
#define  CS_PELM_Mo            "Mo"
#define  CS_PELM_Tc            "Tc"
#define  CS_PELM_Ru            "Ru"
#define  CS_PELM_Rh            "Rh"
#define  CS_PELM_Pd            "Pd"
#define  CS_PELM_Ag            "Ag"
#define  CS_PELM_Cd            "Cd"
#define  CS_PELM_In            "In"
#define  CS_PELM_Sn            "Sn"
#define  CS_PELM_Sb            "Sb"
#define  CS_PELM_Te            "Te"
#define  CS_PELM_Xe            "Xe"
#define  CS_PELM_Cs            "Cs"
#define  CS_PELM_Ba            "Ba"
#define  CS_PELM_La            "La"
#define  CS_PELM_Hf            "Hf"
#define  CS_PELM_Ta            "Ta"
#define  CS_PELM_W             "W"
#define  CS_PELM_Re            "Re"
#define  CS_PELM_Os            "Os"
#define  CS_PELM_Ir            "Ir"
#define  CS_PELM_Pt            "Pt"
#define  CS_PELM_Au            "Au"
#define  CS_PELM_Hg            "Hg"
#define  CS_PELM_Tl            "Tl"
#define  CS_PELM_Pb            "Pb"
#define  CS_PELM_Bi            "Bi"
#define  CS_PELM_Po            "Po"
#define  CS_PELM_At            "At"
#define  CS_PELM_Rn            "Rn"
#define  CS_PELM_Fr            "Fr"
#define  CS_PELM_Ra            "Ra"
#define  CS_PELM_Ac            "Ac"
#define  CS_PELM_Ce            "Ce"
#define  CS_PELM_Pr            "Pr"
#define  CS_PELM_Nd            "Nd"
#define  CS_PELM_Pm            "Pm"
#define  CS_PELM_Sm            "Sm"
#define  CS_PELM_Eu            "Eu"
#define  CS_PELM_Gd            "Gd"
#define  CS_PELM_Tb            "Tb"
#define  CS_PELM_Dy            "Dy"
#define  CS_PELM_Ho            "Ho"
#define  CS_PELM_Er            "Er"
#define  CS_PELM_Tm            "Tm"
#define  CS_PELM_Yb            "Yb"
#define  CS_PELM_Lu            "Lu"
#define  CS_PELM_Th            "Th"
#define  CS_PELM_Pa            "Pa"
#define  CS_PELM_U             "U"
#define  CS_PELM_Np            "Np"
#define  CS_PELM_Pu            "Pu"
#define  CS_PELM_Am            "Am"
#define  CS_PELM_Cm            "Cm"
#define  CS_PELM_Bk            "Bk"
#define  CS_PELM_Cf            "Cf"
#define  CS_PELM_Es            "Es"
#define  CS_PELM_Fm            "Fm"
#define  CS_PELM_Md            "Md"
#define  CS_PELM_No            "No"
#define  CS_PELM_Lw            "Lw"

#define CS_PSTEREO_OPP_DOWN    "stereo_opp_down"
#define CS_PSTEREO_DWN         "stereo_down"
#define CS_PSTEREO_OPP_UP      "stereo_opp_up"
#define CS_PSTEREO_UP          "stereo_up"
#define CS_PSNGL_BOND          "sngl_bond"
#define CS_PRSDNT_BOND         "rsdnt_bond"
#define CS_PVARBL_BOND         "varbl_bond"
#define CS_PERASE_BOND         "erase_bond"
#define CS_PDUBL_BOND          "dubl_bond"
#define CS_PTRPL_BOND          "trpl_bond"

typedef struct s_pdraw_tool
  {
  GC              gc;               /* Drawing tool graphics context */
  Pixmap          pixmap[2];        /* Pixmaps used by the drawing areas */
  Widget          form;             /* Main drawing tool form */
  Widget          frame;
  Widget          error_msg_box;
  Widget          atom_form;
  Widget          atom_lbl;
  Widget          atm_b_pb;
  Widget          atm_br_pb;
  Widget          atm_c_pb;
  Widget          atm_ch_pb;
  Widget          atm_cl_pb;
  Widget          atm_cln_pb;
  Widget          atm_dot_pb;
  Widget          atm_f_pb;
  Widget          atm_h_pb;
  Widget          atm_i_pb;
  Widget          atm_n_pb;
  Widget          atm_o_pb;
  Widget          atm_other_pb;
  Widget          atm_p_pb;
  Widget          atm_r_pb;
  Widget          atm_rp_pb;
  Widget          atm_s_pb;
  Widget          atm_x_pb;
  Widget          bond_form;
  Widget          bnd_lft_lbl;
  Widget          bnd_lsgl_pb;
  Widget          bnd_ldbl_pb;
  Widget          bnd_ltpl_pb;
  Widget          bnd_lvbl_pb;
  Widget          bnd_lres_pb;
  Widget          bnd_mid_lbl;
  Widget          bnd_mid_rb;
  Widget          draw_form;
  Widget          edit_form;
  Widget          edt_mode_lbl;
  Widget          edt_mode_rb;
  Widget          edt_opt_lbl;
  Widget          edt_selall_pb;
  Widget          edt_delsel_pb;
  Widget          patt_lbl[2];
  Widget          scrolled_window[2];
  Widget          drawing_area[2];     /* Drawing Area widgets */
  Widget          rootsyn_form;
  Widget          rootsyn_label;
/*
  Widget          rootsyn_window;
*/
  Widget          rootsyn_area;
  Widget          syn_list_form;
  Widget          syn_list_lbl;
  Widget          syn_list_win;
  Widget          syn_list;
  int             syn_list_num[40 * MX_NAMES_PER_FG];
  char           *slings[40 * MX_NAMES_PER_FG];
  Widget          sling_list_form;
  Widget          sling_list;
  Widget          pb_form;
  Widget          clear_pb;
  Widget          help_pb;
  Widget          done_pb;
  Widget          quit_pb;
  Widget          redraw_pb;
  Widget          isolate_pb;
  Widget          numtogl_pb;
  Widget          periodic_tbl;
  RxnDsp_Molecule_t *rxn_p;            /* Pointers to molecule structures */
  }  PDrawTool_t; 

/* Macro Prototypes for DrawTool_t

Widget          PDrawTool_AtomForm_Get    (PDrawTool_t *);
void            PDrawTool_AtomForm_Put    (PDrawTool_t *, Widget);
Widget          PDrawTool_AtomLbl_Get     (PDrawTool_t *);
void            PDrawTool_AtomLbl_Put     (PDrawTool_t *, Widget);
Widget          PDrawTool_Atm_B_PB_Get    (PDrawTool_t *);
void            PDrawTool_Atm_B_PB_Put    (PDrawTool_t *, Widget);
Widget          PDrawTool_Atm_Br_PB_Get   (PDrawTool_t *);
void            PDrawTool_Atm_Br_PB_Put   (PDrawTool_t *, Widget);
Widget          PDrawTool_Atm_C_PB_Get    (PDrawTool_t *);
void            PDrawTool_Atm_C_PB_Put    (PDrawTool_t *, Widget);
Widget          PDrawTool_Atm_Ch_PB_Get   (PDrawTool_t *);
void            PDrawTool_Atm_Ch_PB_Put   (PDrawTool_t *, Widget);
Widget          PDrawTool_Atm_Cl_PB_Get   (PDrawTool_t *);
void            PDrawTool_Atm_Cl_PB_Put   (PDrawTool_t *, Widget);
Widget          PDrawTool_Atm_Cln_PB_Get  (PDrawTool_t *);
void            PDrawTool_Atm_Cln_PB_Put  (PDrawTool_t *, Widget);
Widget          PDrawTool_Atm_Dot_PB_Get  (PDrawTool_t *);
void            PDrawTool_Atm_Dot_PB_Put  (PDrawTool_t *, Widget);
Widget          PDrawTool_Atm_F_PB_Get    (PDrawTool_t *);
void            PDrawTool_Atm_F_PB_Put    (PDrawTool_t *, Widget);
Widget          PDrawTool_Atm_H_PB_Get    (PDrawTool_t *);
void            PDrawTool_Atm_H_PB_Put    (PDrawTool_t *, Widget);
Widget          PDrawTool_Atm_I_PB_Get    (PDrawTool_t *);
void            PDrawTool_Atm_I_PB_Put    (PDrawTool_t *, Widget);
Widget          PDrawTool_Atm_N_PB_Get    (PDrawTool_t *);
void            PDrawTool_Atm_N_PB_Put    (PDrawTool_t *, Widget);
Widget          PDrawTool_Atm_O_PB_Get    (PDrawTool_t *);
void            PDrawTool_Atm_O_PB_Put    (PDrawTool_t *, Widget);
Widget          PDrawTool_Atm_Other_PB_Get (PDrawTool_t *);
void            PDrawTool_Atm_Other_PB_Put (PDrawTool_t *, Widget);
Widget          PDrawTool_Atm_P_PB_Get    (PDrawTool_t *);
void            PDrawTool_Atm_P_PB_Put    (PDrawTool_t *, Widget);
Widget          PDrawTool_Atm_R_PB_Get    (PDrawTool_t *);
void            PDrawTool_Atm_R_PB_Put    (PDrawTool_t *, Widget);
Widget          PDrawTool_Atm_Rp_PB_Get   (PDrawTool_t *);
void            PDrawTool_Atm_Rp_PB_Put   (PDrawTool_t *, Widget);
Widget          PDrawTool_Atm_S_PB_Get    (PDrawTool_t *);
void            PDrawTool_Atm_S_PB_Put    (PDrawTool_t *, Widget);
Widget          PDrawTool_Atm_X_PB_Get    (PDrawTool_t *);
void            PDrawTool_Atm_X_PB_Put    (PDrawTool_t *, Widget);
Widget          PDrawTool_BondForm_Get    (PDrawTool_t *);
void            PDrawTool_BondForm_Put    (PDrawTool_t *, Widget);
Widget          PDrawTool_BndLftLbl_Get   (PDrawTool_t *);
void            PDrawTool_BndLftLbl_Put   (PDrawTool_t *, Widget);
Widget          PDrawTool_BndLDblPB_Get   (PDrawTool_t *);
void            PDrawTool_BndLDblPB_Put   (PDrawTool_t *, Widget);
Widget          PDrawTool_BndLSglPB_Get   (PDrawTool_t *);
void            PDrawTool_BndLSglPB_Put   (PDrawTool_t *, Widget);
Widget          PDrawTool_BndLTplPB_Get   (PDrawTool_t *);
void            PDrawTool_BndLTplPB_Put   (PDrawTool_t *, Widget);
Widget          PDrawTool_BndLVblPB_Get   (PDrawTool_t *);
void            PDrawTool_BndLVblPB_Put   (PDrawTool_t *, Widget);
Widget          PDrawTool_BndLResPB_Get   (PDrawTool_t *);
void            PDrawTool_BndLResPB_Put   (PDrawTool_t *, Widget);
Widget          PDrawTool_BndMidLbl_Get   (PDrawTool_t *);
void            PDrawTool_BndMidLbl_Put   (PDrawTool_t *, Widget);
Widget          PDrawTool_BndMidRB_Get    (PDrawTool_t *);
void            PDrawTool_BndMidRB_Put    (PDrawTool_t *, Widget);
Widget          PDrawTool_ClearPB_Get     (PDrawTool_t *);
void            PDrawTool_ClearPB_Put     (PDrawTool_t *, Widget);
Widget          PDrawTool_DonePB_Get      (PDrawTool_t *);
void            PDrawTool_DonePB_Put      (PDrawTool_t *, Widget);
Widget          PDrawTool_DrawArea_Get    (PDrawTool_t *, int);
void            PDrawTool_DrawArea_Put    (PDrawTool_t *, int, Widget);
Widget          PDrawTool_DrawForm_Get    (PDrawTool_t *);
void            PDrawTool_DrawForm_Put    (PDrawTool_t *, Widget);
Widget          PDrawTool_EditDelSelPB_Get (PDrawTool_t *);
void            PDrawTool_EditDelSelPB_Put (PDrawTool_t *, Widget);
Widget          PDrawTool_EditForm_Get    (PDrawTool_t *);
void            PDrawTool_EditForm_Put    (PDrawTool_t *, Widget);
Widget          PDrawTool_EditModeLbl_Get (PDrawTool_t *);
void            PDrawTool_EditModeLbl_Put (PDrawTool_t *, Widget);
Widget          PDrawTool_EditModeRB_Get  (PDrawTool_t *);
void            PDrawTool_EditModeRB_Put  (PDrawTool_t *, Widget);
Widget          PDrawTool_EditOptLbl_Get  (PDrawTool_t *);
void            PDrawTool_EditOptLbl_Put  (PDrawTool_t *, Widget);
Widget          PDrawTool_EditSelAllPB_Get (PDrawTool_t *);
void            PDrawTool_EditSelAllPB_Put (PDrawTool_t *, Widget);
Widget          PDrawTool_ExitErrorMsg_Get (PDrawTool_t *);
void            PDrawTool_ExitErrorMsg_Put (PDrawTool_t *, Widget);
Widget          PDrawTool_Form_Get        (PDrawTool_t *);
void            PDrawTool_Form_Put        (PDrawTool_t *, Widget);
Widget          PDrawTool_Frame_Get       (PDrawTool_t *);
void            PDrawTool_Frame_Put       (PDrawTool_t *, Widget);
Widget          PDrawTool_GC_Get          (PDrawTool_t *);
void            PDrawTool_GC_Put          (PDrawTool_t *, GC);
Widget          PDrawTool_HelpPB_Get      (PDrawTool_t *);
void            PDrawTool_HelpPB_Put      (PDrawTool_t *, Widget);
RxnDsp_Molecule_t *PDrawTool_Molecule_Get    (PDrawTool_t *);
void            PDrawTool_Molecule_Put    (PDrawTool_t *,
					   RxnDsp_Molecule_t *);
Widget          PDrawTool_PBForm_Get      (PDrawTool_t *);
void            PDrawTool_PBForm_Put      (PDrawTool_t *, Widget);
Widget          PDrawTool_PeriodicTbl_Get (PDrawTool_t *);
void            PDrawTool_PeriodicTbl_Put (PDrawTool_t *, Widget);
Widget          PDrawTool_Pixmap_Get      (PDrawTool_t *, int);
void            PDrawTool_Pixmap_Put      (PDrawTool_t *, int, Pixmap);
Widget          PDrawTool_QuitPB_Get      (PDrawTool_t *);
void            PDrawTool_QuitPB_Put      (PDrawTool_t *, Widget);
Widget          PDrawTool_RedrawPB_Get    (PDrawTool_t *);
void            PDrawTool_RedrawPB_Put    (PDrawTool_t *, Widget);
Widget          PDrawTool_IsolatePB_Get   (PDrawTool_t *);
void            PDrawTool_IsolatePB_Put   (PDrawTool_t *, Widget);
Widget          PDrawTool_NumToglPB_Get   (PDrawTool_t *);
void            PDrawTool_NumToglPB_Put   (PDrawTool_t *, Widget);
Widget          PDrawTool_PattLbl_Get     (PDrawTool_t *, int);
void            PDrawTool_PattLbl_Put     (PDrawTool_t *, int, Widget);
Widget          PDrawTool_RootSynArea_Get (PDrawTool_t *);
void            PDrawTool_RootSynArea_Put (PDrawTool_t *, Widget);
Widget          PDrawTool_RootSynForm_Get (PDrawTool_t *);
void            PDrawTool_RootSynForm_Put (PDrawTool_t *, Widget);
Widget          PDrawTool_RootSynLbl_Get  (PDrawTool_t *);
void            PDrawTool_RootSynLbl_Put  (PDrawTool_t *, Widget);
Widget          PDrawTool_RootSynWin_Get  (PDrawTool_t *);
void            PDrawTool_RootSynWin_Put  (PDrawTool_t *, Widget);
Widget          PDrawTool_ScrlldWin_Get   (PDrawTool_t *, int);
void            PDrawTool_ScrlldWin_Put   (PDrawTool_t *, int, Widget);
Widget          PDrawTool_SlingList_Get     (PDrawTool_t *);
void            PDrawTool_SlingList_Put     (PDrawTool_t *, Widget);
Widget          PDrawTool_SlingListForm_Get  (PDrawTool_t *);
void            PDrawTool_SlingListForm_Put  (PDrawTool_t *, Widget);
Widget          PDrawTool_SynList_Get     (PDrawTool_t *);
void            PDrawTool_SynList_Put     (PDrawTool_t *, Widget);
Widget          PDrawTool_SynListForm_Get  (PDrawTool_t *);
void            PDrawTool_SynListForm_Put  (PDrawTool_t *, Widget);
Widget          PDrawTool_SynListLbl_Get  (PDrawTool_t *);
void            PDrawTool_SynListLbl_Put  (PDrawTool_t *, Widget);
int             PDrawTool_SynListNum_Get  (PDrawTool_t *, int);
void            PDrawTool_SynListNum_Put  (PDrawTool_t *, int, int);
Widget          PDrawTool_SynListWin_Get  (PDrawTool_t *);
void            PDrawTool_SynListWin_Put  (PDrawTool_t *, Widget);
*/

#define PDrawTool_AtomForm_Get(draw_p)\
  (draw_p)->atom_form
#define PDrawTool_AtomForm_Put(draw_p, value)\
  (draw_p)->atom_form = (value)
#define PDrawTool_AtomLbl_Get(draw_p)\
  (draw_p)->atom_lbl
#define PDrawTool_AtomLbl_Put(draw_p, value)\
  (draw_p)->atom_lbl = (value)
#define PDrawTool_Atm_B_PB_Get(draw_p)\
  (draw_p)->atm_b_pb
#define PDrawTool_Atm_B_PB_Put(draw_p, value)\
  (draw_p)->atm_b_pb = (value)
#define PDrawTool_Atm_Br_PB_Get(draw_p)\
  (draw_p)->atm_br_pb
#define PDrawTool_Atm_Br_PB_Put(draw_p, value)\
  (draw_p)->atm_br_pb = (value)
#define PDrawTool_Atm_C_PB_Get(draw_p)\
  (draw_p)->atm_c_pb
#define PDrawTool_Atm_C_PB_Put(draw_p, value)\
  (draw_p)->atm_c_pb = (value)
#define PDrawTool_Atm_Ch_PB_Get(draw_p)\
  (draw_p)->atm_ch_pb
#define PDrawTool_Atm_Ch_PB_Put(draw_p, value)\
  (draw_p)->atm_ch_pb = (value)
#define PDrawTool_Atm_Cl_PB_Get(draw_p)\
  (draw_p)->atm_cl_pb
#define PDrawTool_Atm_Cl_PB_Put(draw_p, value)\
  (draw_p)->atm_cl_pb = (value)
#define PDrawTool_Atm_Cln_PB_Get(draw_p)\
  (draw_p)->atm_cln_pb
#define PDrawTool_Atm_Cln_PB_Put(draw_p, value)\
  (draw_p)->atm_cln_pb = (value)
#define PDrawTool_Atm_Dot_PB_Get(draw_p)\
  (draw_p)->atm_dot_pb
#define PDrawTool_Atm_Dot_PB_Put(draw_p, value)\
  (draw_p)->atm_dot_pb = (value)
#define PDrawTool_Atm_F_PB_Get(draw_p)\
  (draw_p)->atm_f_pb
#define PDrawTool_Atm_F_PB_Put(draw_p, value)\
  (draw_p)->atm_f_pb = (value)
#define PDrawTool_Atm_H_PB_Get(draw_p)\
  (draw_p)->atm_h_pb
#define PDrawTool_Atm_H_PB_Put(draw_p, value)\
  (draw_p)->atm_h_pb = (value)
#define PDrawTool_Atm_I_PB_Get(draw_p)\
  (draw_p)->atm_i_pb
#define PDrawTool_Atm_I_PB_Put(draw_p, value)\
  (draw_p)->atm_i_pb = (value)
#define PDrawTool_Atm_N_PB_Get(draw_p)\
  (draw_p)->atm_n_pb
#define PDrawTool_Atm_N_PB_Put(draw_p, value)\
  (draw_p)->atm_n_pb = (value)
#define PDrawTool_Atm_O_PB_Get(draw_p)\
  (draw_p)->atm_o_pb
#define PDrawTool_Atm_O_PB_Put(draw_p, value)\
  (draw_p)->atm_o_pb = (value)
#define PDrawTool_Atm_Other_PB_Get(draw_p)\
  (draw_p)->atm_other_pb
#define PDrawTool_Atm_Other_PB_Put(draw_p, value)\
  (draw_p)->atm_other_pb = (value)
#define PDrawTool_Atm_P_PB_Get(draw_p)\
  (draw_p)->atm_p_pb
#define PDrawTool_Atm_P_PB_Put(draw_p, value)\
  (draw_p)->atm_p_pb = (value)
#define PDrawTool_Atm_R_PB_Get(draw_p)\
  (draw_p)->atm_r_pb
#define PDrawTool_Atm_R_PB_Put(draw_p, value)\
  (draw_p)->atm_r_pb = (value)
#define PDrawTool_Atm_Rp_PB_Get(draw_p)\
  (draw_p)->atm_rp_pb
#define PDrawTool_Atm_Rp_PB_Put(draw_p, value)\
  (draw_p)->atm_rp_pb = (value)
#define PDrawTool_Atm_S_PB_Get(draw_p)\
  (draw_p)->atm_s_pb
#define PDrawTool_Atm_S_PB_Put(draw_p, value)\
  (draw_p)->atm_s_pb = (value)
#define PDrawTool_Atm_X_PB_Put(draw_p, value)\
  (draw_p)->atm_x_pb = (value)
#define PDrawTool_Atm_X_PB_Get(draw_p)\
  (draw_p)->atm_x_pb
#define PDrawTool_BondForm_Get(draw_p)\
  (draw_p)->bond_form
#define PDrawTool_BondForm_Put(draw_p, value)\
  (draw_p)->bond_form = (value)
#define PDrawTool_BndLftLbl_Get(draw_p)\
  (draw_p)->bnd_lft_lbl
#define PDrawTool_BndLftLbl_Put(draw_p, value)\
  (draw_p)->bnd_lft_lbl = (value)
#define PDrawTool_BndLDblPB_Get(draw_p)\
  (draw_p)->bnd_ldbl_pb
#define PDrawTool_BndLDblPB_Put(draw_p, value)\
  (draw_p)->bnd_ldbl_pb = (value)
#define PDrawTool_BndLSglPB_Get(draw_p)\
  (draw_p)->bnd_lsgl_pb
#define PDrawTool_BndLSglPB_Put(draw_p, value)\
  (draw_p)->bnd_lsgl_pb = (value)
#define PDrawTool_BndLTplPB_Get(draw_p)\
  (draw_p)->bnd_ltpl_pb
#define PDrawTool_BndLTplPB_Put(draw_p, value)\
  (draw_p)->bnd_ltpl_pb = (value)
#define PDrawTool_BndLVblPB_Get(draw_p)\
  (draw_p)->bnd_lvbl_pb
#define PDrawTool_BndLVblPB_Put(draw_p, value)\
  (draw_p)->bnd_lvbl_pb = (value)
#define PDrawTool_BndLResPB_Get(draw_p)\
  (draw_p)->bnd_lres_pb
#define PDrawTool_BndLResPB_Put(draw_p, value)\
  (draw_p)->bnd_lres_pb = (value)
#define PDrawTool_BndMidLbl_Get(draw_p)\
  (draw_p)->bnd_mid_lbl
#define PDrawTool_BndMidLbl_Put(draw_p, value)\
  (draw_p)->bnd_mid_lbl = (value)
#define PDrawTool_BndMidRB_Get(draw_p)\
  (draw_p)->bnd_mid_rb
#define PDrawTool_BndMidRB_Put(draw_p, value)\
  (draw_p)->bnd_mid_rb = (value)
#define PDrawTool_ClearPB_Get(draw_p)\
  (draw_p)->clear_pb
#define PDrawTool_ClearPB_Put(draw_p, value)\
  (draw_p)->clear_pb = (value)
#define PDrawTool_HelpPB_Get(draw_p)\
  (draw_p)->help_pb
#define PDrawTool_HelpPB_Put(draw_p, value)\
  (draw_p)->help_pb = (value)
#define PDrawTool_DonePB_Get(draw_p)\
  (draw_p)->done_pb
#define PDrawTool_DonePB_Put(draw_p, value)\
  (draw_p)->done_pb = (value)
#define PDrawTool_QuitPB_Get(draw_p)\
  (draw_p)->quit_pb
#define PDrawTool_QuitPB_Put(draw_p, value)\
  (draw_p)->quit_pb = (value)
#define PDrawTool_DrawArea_Get(draw_p, pattern)\
  (draw_p)->drawing_area[(pattern)]
#define PDrawTool_DrawArea_Put(draw_p, pattern, value)\
  (draw_p)->drawing_area[(pattern)] = (value)
#define PDrawTool_DrawForm_Get(draw_p)\
  (draw_p)->draw_form
#define PDrawTool_DrawForm_Put(draw_p, value)\
  (draw_p)->draw_form = (value)
#define PDrawTool_EditDelSelPB_Get(draw_p)\
  (draw_p)->edt_delsel_pb
#define PDrawTool_EditDelSelPB_Put(draw_p, value)\
  (draw_p)->edt_delsel_pb = (value)
#define PDrawTool_EditForm_Get(draw_p)\
  (draw_p)->edit_form
#define PDrawTool_EditForm_Put(draw_p, value)\
  (draw_p)->edit_form = (value)
#define PDrawTool_EditModeLbl_Get(draw_p)\
  (draw_p)->edt_mode_lbl
#define PDrawTool_EditModeLbl_Put(draw_p, value)\
  (draw_p)->edt_mode_lbl = (value)
#define PDrawTool_EditModeRB_Get(draw_p)\
  (draw_p)->edt_mode_rb
#define PDrawTool_EditModeRB_Put(draw_p, value)\
  (draw_p)->edt_mode_rb = (value)
#define PDrawTool_EditOptLbl_Get(draw_p)\
  (draw_p)->edt_opt_lbl
#define PDrawTool_EditOptLbl_Put(draw_p, value)\
  (draw_p)->edt_opt_lbl = (value)
#define PDrawTool_EditSelAllPB_Get(draw_p)\
  (draw_p)->edt_selall_pb
#define PDrawTool_EditSelAllPB_Put(draw_p, value)\
  (draw_p)->edt_selall_pb = (value)
#define PDrawTool_ExitErrorMsg_Get(draw_p)\
  (draw_p)->error_msg_box
#define PDrawTool_ExitErrorMsg_Put(draw_p, value)\
  (draw_p)->error_msg_box = (value)
#define PDrawTool_Form_Get(draw_p)\
  (draw_p)->form
#define PDrawTool_Form_Put(draw_p, value)\
  (draw_p)->form = (value)
#define PDrawTool_Frame_Get(draw_p)\
  (draw_p)->frame
#define PDrawTool_Frame_Put(draw_p, value)\
  (draw_p)->frame = (value)
#define PDrawTool_GC_Get(draw_p)\
  (draw_p)->gc
#define PDrawTool_GC_Put(draw_p, value)\
  (draw_p)->gc = (value)
#define PDrawTool_Molecule_Get(draw_p)\
  (draw_p)->rxn_p
#define PDrawTool_Molecule_Put(draw_p, value)\
  (draw_p)->rxn_p = (value)
#define PDrawTool_PBForm_Get(draw_p)\
  (draw_p)->pb_form
#define PDrawTool_PBForm_Put(draw_p, value)\
  (draw_p)->pb_form = (value)
#define PDrawTool_PeriodicTbl_Get(draw_p)\
  (draw_p)->periodic_tbl
#define PDrawTool_PeriodicTbl_Put(draw_p, value)\
  (draw_p)->periodic_tbl = (value)
#define PDrawTool_Pixmap_Get(draw_p, pattern)\
  (draw_p)->pixmap[(pattern)]
#define PDrawTool_Pixmap_Put(draw_p, pattern, value)\
  (draw_p)->pixmap[(pattern)] = (value)
#define PDrawTool_RedrawPB_Get(draw_p)\
  (draw_p)->redraw_pb
#define PDrawTool_RedrawPB_Put(draw_p, value)\
  (draw_p)->redraw_pb = (value)
#define PDrawTool_IsolatePB_Get(draw_p)\
  (draw_p)->isolate_pb
#define PDrawTool_IsolatePB_Put(draw_p, value)\
  (draw_p)->isolate_pb = (value)
#define PDrawTool_NumToglPB_Get(draw_p)\
  (draw_p)->numtogl_pb
#define PDrawTool_NumToglPB_Put(draw_p, value)\
  (draw_p)->numtogl_pb = (value)
#define PDrawTool_PattLbl_Get(draw_p, pattern)\
  (draw_p)->patt_lbl[(pattern)]
#define PDrawTool_PattLbl_Put(draw_p, pattern, value)\
  (draw_p)->patt_lbl[(pattern)] = (value)
#define PDrawTool_RootSynArea_Get(draw_p) \
  (draw_p)->rootsyn_area
#define PDrawTool_RootSynArea_Put(draw_p, value) \
  (draw_p)->rootsyn_area = (value)
#define PDrawTool_RootSynForm_Get(draw_p) \
  (draw_p)->rootsyn_form
#define PDrawTool_RootSynForm_Put(draw_p, value) \
  (draw_p)->rootsyn_form = (value)
#define PDrawTool_RootSynLbl_Get(draw_p) \
  (draw_p)->rootsyn_label
#define PDrawTool_RootSynLbl_Put(draw_p, value) \
  (draw_p)->rootsyn_label = (value)
/*
#define PDrawTool_RootSynWin_Get(draw_p) \
  (draw_p)->rootsyn_window
#define PDrawTool_RootSynWin_Put(draw_p, value) \
  (draw_p)->rootsyn_window = (value)
*/
#define PDrawTool_ScrlldWin_Get(draw_p, pattern)\
  (draw_p)->scrolled_window[(pattern)]
#define PDrawTool_ScrlldWin_Put(draw_p, pattern, value)\
  (draw_p)->scrolled_window[(pattern)] = (value)
#define PDrawTool_SlingList_Get(draw_p)\
  (draw_p)->sling_list
#define PDrawTool_SlingList_Put(draw_p, value)\
  (draw_p)->sling_list = (value)
#define PDrawTool_SlingListForm_Get(draw_p)\
  (draw_p)->sling_list_form
#define PDrawTool_SlingListForm_Put(draw_p, value)\
  (draw_p)->sling_list_form = (value)
#define PDrawTool_SynList_Get(draw_p)\
  (draw_p)->syn_list
#define PDrawTool_SynList_Put(draw_p, value)\
  (draw_p)->syn_list = (value)
#define PDrawTool_SynListForm_Get(draw_p)\
  (draw_p)->syn_list_form
#define PDrawTool_SynListForm_Put(draw_p, value)\
  (draw_p)->syn_list_form = (value)
#define PDrawTool_SynListLbl_Get(draw_p)\
  (draw_p)->syn_list_lbl
#define PDrawTool_SynListLbl_Put(draw_p, value)\
  (draw_p)->syn_list_lbl = (value)
#define PDrawTool_SynListNum_Get(draw_p, inx)\
  (draw_p)->syn_list_num[(inx) - 1]
#define PDrawTool_SynListNum_Put(draw_p, inx, value)\
  (draw_p)->syn_list_num[(inx) - 1] = (value)
#define PDrawTool_SynListWin_Get(draw_p)\
  (draw_p)->syn_list_win
#define PDrawTool_SynListWin_Put(draw_p, value)\
  (draw_p)->syn_list_win = (value)

typedef struct rxninfo_cb_s 
  { 
  Widget               form; 
  Widget               frame; 
  Widget               lbl_sling; 
  Widget               txt_sling; 
  Widget               pb_reset; 
  Widget               pb_draw;
  Widget               pb_compound;
  char                 sling[RXN_SLINGBUF_MAXLEN + 1];
  }  RxnInfoCB_t; 

/* Macro Prototypes for RxnInfoCB_t

Widget        RxnInfo_DrawPB_Get       (RxnInfoCB_t *);
void          RxnInfo_DrawPB_Put       (RxnInfoCB_t *, Widget);
Widget        RxnInfo_Form_Get         (RxnInfoCB_t *);
void          RxnInfo_Form_Put         (RxnInfoCB_t *, Widget);
Widget        RxnInfo_Frame_Get        (RxnInfoCB_t *);
void          RxnInfo_Frame_Put        (RxnInfoCB_t *, Widget);
Widget        RxnInfo_ResetPB_Get      (RxnInfoCB_t *);
void          RxnInfo_ResetPB_Put      (RxnInfoCB_t *, Widget);
char         *RxnInfo_Sling_Get        (RxnInfoCB_t *);
Widget        RxnInfo_SlingLbl_Get     (RxnInfoCB_t *);
void          RxnInfo_SlingLbl_Put     (RxnInfoCB_t *, Widget);
Widget        RxnInfo_SlingTxt_Get     (RxnInfoCB_t *);
void          RxnInfo_SlingTxt_Put     (RxnInfoCB_t *, Widget); 
*/

/*** RxnInfoCB_t Macros ***/

#define  RxnInfo_DrawPB_Get(cucb)\
   (cucb)->pb_draw
#define  RxnInfo_DrawPB_Put(cucb, value)\
   (cucb)->pb_draw = (value)
#define  RxnInfo_Form_Get(cucb)\
   (cucb)->form
#define  RxnInfo_Form_Put(cucb, value)\
   (cucb)->form = (value)
#define  RxnInfo_Frame_Get(cucb)\
   (cucb)->frame
#define  RxnInfo_Frame_Put(cucb, value)\
   (cucb)->frame = (value)
#define  RxnInfo_ResetPB_Get(cucb)\
   (cucb)->pb_reset
#define  RxnInfo_ResetPB_Put(cucb, value)\
   (cucb)->pb_reset = (value)
#define  RxnInfo_Sling_Get(cucb)\
   (cucb)->sling
#define  RxnInfo_SlingLbl_Get(cucb)\
   (cucb)->lbl_sling
#define  RxnInfo_SlingLbl_Put(cucb, value)\
   (cucb)->lbl_sling = (value)
#define  RxnInfo_SlingTxt_Get(cucb)\
   (cucb)->txt_sling 
#define  RxnInfo_SlingTxt_Put(cucb, value)\
   (cucb)->txt_sling = (value)

#ifdef PDRAW_GLOBALS

const String patoms_list[12] = 
  {
  CS_PELM_C,  CS_PELM_H,  CS_PELM_N,  
  CS_PELM_O,  CS_PELM_S,  CS_PELM_P, 
  CS_PELM_Cl, CS_PELM_F,  CS_PELM_Br, 
  CS_PELM_I,  CS_PELM_B,  CS_PELM_OTHER
  };

const String Li_Ra_PElms[12] =
  {
  CS_PELM_Li, CS_PELM_Na, CS_PELM_K, CS_PELM_Rb,
  CS_PELM_Cs, CS_PELM_Fr, CS_PELM_Be, CS_PELM_Mg,
  CS_PELM_Ca, CS_PELM_Sr, CS_PELM_Ba, CS_PELM_Ra
  };

const String B_Ar_PElms[12] =
  {
  CS_PELM_B, CS_PELM_Al, CS_PELM_C , CS_PELM_Si,
  CS_PELM_N, CS_PELM_P , CS_PELM_O , CS_PELM_S ,  
  CS_PELM_F, CS_PELM_Cl, CS_PELM_Ne, CS_PELM_Ar
  };

const String Sc_Rn_PElms[48] =
  {
  CS_PELM_Sc, CS_PELM_Y , CS_PELM_La, CS_PELM_Ti, CS_PELM_Zr, CS_PELM_Hf, 
  CS_PELM_V , CS_PELM_Nb, CS_PELM_Ta, CS_PELM_Cr, CS_PELM_Mo, CS_PELM_W, 
  CS_PELM_Mn, CS_PELM_Tc, CS_PELM_Re, CS_PELM_Fe, CS_PELM_Ru, CS_PELM_Os, 
  CS_PELM_Co, CS_PELM_Rh, CS_PELM_Ir, CS_PELM_Ni, CS_PELM_Pd, CS_PELM_Pt, 
  CS_PELM_Cu, CS_PELM_Ag, CS_PELM_Au, CS_PELM_Zn, CS_PELM_Cd, CS_PELM_Hg, 
  CS_PELM_Ga, CS_PELM_In, CS_PELM_Tl, CS_PELM_Ge, CS_PELM_Sn, CS_PELM_Pb, 
  CS_PELM_As, CS_PELM_Sb, CS_PELM_Bi, CS_PELM_Se, CS_PELM_Te, CS_PELM_Po, 
  CS_PELM_Br, CS_PELM_I , CS_PELM_At, CS_PELM_Kr, CS_PELM_Xe, CS_PELM_Rn 
  };

const String Ce_Lw_PElms[28] =
  {
  CS_PELM_Ce, CS_PELM_Th, CS_PELM_Pr, CS_PELM_Pa, CS_PELM_Nd, CS_PELM_U ,
  CS_PELM_Pm, CS_PELM_Np, CS_PELM_Sm, CS_PELM_Pu, CS_PELM_Eu, CS_PELM_Am, 
  CS_PELM_Gd, CS_PELM_Cm, CS_PELM_Tb, CS_PELM_Bk, CS_PELM_Dy, CS_PELM_Cf, 
  CS_PELM_Ho, CS_PELM_Es, CS_PELM_Er, CS_PELM_Fm, CS_PELM_Tm, CS_PELM_Md, 
  CS_PELM_Yb, CS_PELM_No, CS_PELM_Lu, CS_PELM_Lw
  };  

const String pbond_bitmaps[3] =
  {
  CS_PSNGL_BOND,   CS_PDUBL_BOND,  CS_PTRPL_BOND,
  };

#else
extern const String patoms_list[12];
extern const String Li_Ra_PElms[12];
extern const String B_Ar_PElms[12];
extern const String Sc_Rn_PElms[48];
extern const String Ce_Lw_PElms[28];
extern const String pbond_bitmaps[3];
#endif

/*** Globally Defined DrawTool Control Block ***/

#ifdef PDRAW_GLOBALS
Boolean_t IsThere_Draw_Flag = FALSE;

static  PDrawTool_t GPDrawToolCB;
static  Selected_Syntheme = -1;
static  int goalfg[10] = {0}, subgfg[10] = {0}, gminin[10] = {0}, sminin[10] = {0};
/*
static  int         nroots = 0;
static  int         root_atoms[5];
*/
#else
extern Boolean_t IsThere_Draw_Flag;
#endif

/* Macro Prototypes

int  PDRW_Slope  (int, int, int, int, int);
*/

#define PDRW_Slope(x0,x1,y1,x2,y2) ((y2-y1)*(x0-x1)/(x2-x1)+y1)


/*** Function Prototypes ***/


void   PAtom_Copy            (Dsp_Atom_t  *, Dsp_Atom_t  *);
void   PCreateOptionMenu     (Widget, char *, char *, int, int, int, int);
Widget PDraw_Tool_Create     (RxnInfoCB_t *, Widget, XtAppContext *,
			      Xtr_t *, Xtr_t *, U32_t *, U32_t *, U32_t);
void   PDraw_Tool_Initialize (void);
void   PDrawTool_Destroy     (void);
void   PPixmap_Install       (Widget, char *);
void   PPeriodic_TblDlg_Create (Widget);
void   SynList_Create        (Widget);
void   Slings_Create         (Widget);
void   PMolDraw_Sling_Draw   (Sling_t, int);

void pdraw_Double_Bond (Display *, Drawable, GC, int, int, int, int);
void pdraw_Stereo_Bond (Display *, Drawable, GC, int, int, int, int, int);
void pdraw_Bond        (Display *, Drawable, GC, XColor, RxnDsp_Molecule_t *,
       RxnDsp_Bond_t *, int, Boolean_t, Boolean_t);
void pdraw_Atom        (Display *, Drawable, Drawable, GC, XColor,
       RxnDsp_Atom_t *, int);
void perase_Bond       (Display *, Drawable, Drawable, Drawable, Drawable, GC,
       RxnDsp_Molecule_t *, RxnDsp_Bond_t *, int);
void pdraw_Molecule    (Display *, Drawable, GC, RxnDsp_Molecule_t  *, float,
       Dimension, Dimension, int, Boolean_t);

Xtr_t             *PNew_RxnDsp2Xtr   (RxnDsp_Molecule_t *mol_p, int);
Tsd_t             *RxnDsp2Tsd       (RxnDsp_Molecule_t *mol_p, int);
Sling_t            PSling2CanonSling (Sling_t);
RxnDsp_Molecule_t *PMolecule_Double  (RxnDsp_Molecule_t *);
RxnDsp_Bond_t     *pstore_BondNew    (RxnDsp_Molecule_t  *, int, int, int, int,
				      int, int);
RxnDsp_Atom_t     *pstore_AtomNew    (RxnDsp_Molecule_t *, char *, char *,
				      char *, int, int);

void              RxnInfo_SlingTxt_Update (RxnInfoCB_t **, RxnDsp_Molecule_t *);
void              draw_roots         (Display *, Drawable, Drawable, GC,
				      RxnDsp_Molecule_t *);
void              check_roots        (RxnDsp_Molecule_t *, RxnDsp_Atom_t *);

/** Callback Prototypes **/ 

void PDraw_Mode_Reset         (int);
void predraw                  (Widget, XtPointer, XtPointer);
void pclear_it                (Widget, XtPointer, XtPointer);          
void PDone_CB                 (Widget, XtPointer, XtPointer);
void PQuit_CB                 (Widget, XtPointer, XtPointer);
void PIsolate_CB              (Widget, XtPointer, XtPointer);
void PNumTogl_CB              (Widget, XtPointer, XtPointer);
void PPixMap_Display_CB       (Widget, XtPointer, XtPointer);
void PPixMap_Resize_CB        (Widget, XtPointer, XtPointer);
void PAtom_Mark_CB            (Widget, XtPointer, XtPointer);
void POption_Choose_CB        (Widget, XtPointer, XtPointer);
void PMenu_Choice_CB          (Widget, XtPointer, XtPointer);
void PPeriodic_TblDlg_Show_CB (Widget, XtPointer, XtPointer);
void PSelected_Bond_Draw_CB   (Widget, XtPointer, XtPointer);
void PMOLDRAW_StereoChem_CB   (Widget, XtPointer, XtPointer);
void Syntheme_Store_CB        (Widget, XtPointer, XtPointer);
void Sling_Select_CB          (Widget, XtPointer, XtPointer);
void PDismissErr_CB           (Widget, XtPointer, XtPointer);
void redisplay_roots          (Widget, XtPointer, XtPointer);

/** EventHandler Prototypes **/

void pleave_drawarea          (Widget, XtPointer, XEvent *, Boolean *);
void penter_drawarea          (Widget, XtPointer, XEvent *, Boolean *);

#endif
