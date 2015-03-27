#ifndef _H_EDIT_
#define _H_EDIT_  1

#ifndef _H_DSP_
#include "dsp.h"
#endif

/*  The flag data structure
 */
typedef struct drw_flags_s {
  int              drawing_mode;
  int              dflt_bond;    /* bond drawn by middle button default */
  char             cur_atmsym[3];
  Boolean_t        retrace_mode;
  Boolean_t        switch_by_click;
} Drw_flags_t;
#define DRW_FLAGS_SIZE (sizeof (Drw_flags_t))


/*  Data structure used to draw bonds
 */
typedef struct drw_bonds_info_s {
  Boolean_t     esc_mode;
  Boolean_t     new_atom;
  Dsp_Bond_t   *cur_bnd_p;
  Boolean_t     in_atom_area;
  Boolean_t     in_bond_area;
  Position      clicked_x;
  Position      clicked_y;
  Position      prev_x;
  Position      prev_y;
} Drw_bonds_info_t;
#define DRW_BONDS_INFO_SIZE (sizeof (Drw_bonds_info_t))

/*  Data structure used by selection functions
 */
typedef struct drw_select_info_s {
  Boolean_t     move_it;
  Boolean_t     isSelected;
  Position      selected_x;      /* Coordinates of left upper */
  Position      selected_y;      /* corner of selected area   */
  int           sel_width;       /* Width of selected area */
  int           sel_height;      /* Height of selected area */
} Drw_select_info_t;
#define DRW_SELECT_INFO_SIZE (sizeof (Drw_select_info_t))

/**  Macros **/
/*  Macro Prototypes 
Boolean_t  not  (Boolean_t)
*/

#define not(x) ((x) ? FALSE : TRUE)


/* Function Prototypes
 */
void Draw_Initialize (Widget);
void Draw_Draw (Widget, XButtonEvent *, String *, int *);

/*void        switch_Mols     (); */
Dsp_Bond_t *in_Bond_Scope   (Dsp_Molecule_t *, int, int);

void cb_mark_atom (Widget, XtPointer, XtPointer);

int  atom_pos_to_pointer_pos (int, int, int, int);
void check_kvadr     (Dsp_Molecule_t *, int, int, int, int);
void mark_selected   (Dsp_Molecule_t *, Display *, Drawable, GC);
void unmark_selected (Dsp_Molecule_t *, Display *, Drawable, Drawable, GC);
void draw_selected   (Dsp_Molecule_t *, Display *, Drawable, GC, XColor,
                       int, int, Boolean_t); 


/* Static variables definitions */

#ifdef DRAW_GLOBALS

static Display       *DrawDisplay;
static Screen        *DrawScreen;
static Drw_flags_t    DrawFlags;

static Dsp_Molecule_t    *DrawMol_p;
static Drw_bonds_info_t  *DrawBondsInfo;
static Drw_select_info_t *DrawSelInfo;

#endif

#endif
