#ifndef _H_PATTEDT_
#define _H_PATTEDT_  1
/******************************************************************************
*
*  Copyright (C) 1999, Synchem Group at SUNY-Stony Brook, Gerald A. Miller
*
*  Module Name:                     RXNPATT_EDIT.H
*
*    This header contains the declarations needed for drawing within the module
*    rxnpatt_draw.c.
*
*  Routines:
*
*    xxx
*
*  Creation Date:
*
*    24-Feb-1999
*
*  Authors:
*
*    Jerry Miller
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Lolita     xxx
*
******************************************************************************/


#ifndef _H_RXNDSP_
#include "rxndsp.h"
#endif

/*  The flag data structure
 */
typedef struct pdrw_flags_s {
  int              drawing_mode;
  int              dflt_bond;    /* bond drawn by middle button default */
  char             cur_atmsym[3];
  Boolean_t        retrace_mode;
  Boolean_t        switch_by_click;
} PDrw_flags_t;
#define PDRW_FLAGS_SIZE (sizeof (PDrw_flags_t))


/*  Data structure used to draw bonds
 */
typedef struct pdrw_bonds_info_s {
  Boolean_t     esc_mode;
  Boolean_t     new_atom;
  RxnDsp_Bond_t *cur_bnd_p;
  Boolean_t     in_atom_area;
  Boolean_t     in_bond_area;
  Position      clicked_x;
  Position      clicked_y;
  int           clicked_gsg;
  Position      prev_x;
  Position      prev_y;
  int           prev_gsg;
} PDrw_bonds_info_t;
#define PDRW_BONDS_INFO_SIZE (sizeof (PDrw_bonds_info_t))

/*  Data structure used by selection functions
 */
typedef struct pdrw_select_info_s {
  Boolean_t     move_it;
  Boolean_t     isSelected;
  Position      selected_x;      /* Coordinates of left upper */
  Position      selected_y;      /* corner of selected area   */
  int           selected_gsg;
  int           sel_width;       /* Width of selected area */
  int           sel_height;      /* Height of selected area */
} PDrw_select_info_t;
#define PDRW_SELECT_INFO_SIZE (sizeof (PDrw_select_info_t))

/**  Macros **/
/*  Macro Prototypes 
Boolean_t  pnot  (Boolean_t)
*/

#define pnot(x) ((x) ? FALSE : TRUE)


/* Function Prototypes
 */
void PDraw_Initialize (Widget);
void PDraw_Draw (Widget, XButtonEvent *, String *, int *);

/*void        pswitch_Mols     (); */
RxnDsp_Bond_t *pin_Bond_Scope   (RxnDsp_Molecule_t *, int, int);

void pcb_mark_atom (Widget, XtPointer, XtPointer);

int  patom_pos_to_pointer_pos (int, int, int, int);
void pcheck_kvadr     (RxnDsp_Molecule_t *, int, int, int, int);
void pmark_selected   (RxnDsp_Molecule_t *, Display *, Drawable, GC);
void punmark_selected (RxnDsp_Molecule_t *, Display *, Drawable, Drawable, GC);
void pdraw_selected   (RxnDsp_Molecule_t *, Display *, Drawable, GC, XColor,
                       int, int, Boolean_t); 


/* Static variables definitions */

#ifdef PDRAW_GLOBALS

static Display       *PDrawDisplay;
static Screen        *PDrawScreen;
static PDrw_flags_t    PDrawFlags;

static RxnDsp_Molecule_t  *PDrawMol_p;
static PDrw_bonds_info_t  *PDrawBondsInfo;
static PDrw_select_info_t *PDrawSelInfo;

#endif

#endif
