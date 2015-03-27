#ifndef _H_DSP_
#define _H_DSP_  1
/******************************************************************************
*
*  Copyright (C) 1993-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     DSP.H
*
*    This module contains the definions of atoms, bonds and molecules
*    for displaying purposes.  
*    
*  Creation Date:
*
*    01-Aug-1993
*
*  Authors:
*
*    Daren Krebsbach
*    Rulsan Bilorusets
*
*  Modification History, reverse chronological
*
* Date       Author	Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Lolita	xxx
* 25-Feb-95  Krebsbach  Brought the code up to specs (sort of) and 
*                       made relevant modifications.
*
******************************************************************************/

#ifndef _H_XTR_
#include "xtr.h"
#endif

#define DSP_BOND_SINGLE           1
#define DSP_BOND_DOUBLE           2
#define DSP_BOND_TRIPLE           3
#define DSP_BOND_RESONANT         4
#define DSP_BOND_VARIABLE         5
#define DSP_BOND_STEREO_DOWN      6
#define DSP_BOND_STEREO_UP        7
#define DSP_BOND_STEREO_OPP_DOWN  8
#define DSP_BOND_STEREO_OPP_UP    9
#define DSP_BOND_ERASE            0

#define DSP_DELTA_NORM_MIN        1.10
#define DSP_DELTA_NORM_MAX        2.0

#define DRW_SELECT_NONE           0
#define DRW_SELECT_MOVE           1
#define DRW_SELECT_TOTAL          2

#define DSP_BOND_STEREO_NONE      0

#define DRW_HILIT_AREA            8

#define DRW_MEM_UNIT             128
#define DSP_MAXNUM_ATMS          512

#define SHELLY2DSP_SCALE         50
#define DSP_SHELLEY_SHIFT        15



/*  Data structure used to form adjacency list for drawing molecule
 */
typedef struct dsp_adj_info_s {
  U8_t       num_neighbs;       /* number of adjacent atoms */
  long       adj_atms [6];      /* indecies of adjacent atoms in the
				 * atom list (atoms) */
} Dsp_adj_info_t;

/*  The Atom Description  
 */
typedef struct dsp_atom_s
  {
  char           sym[4];         /* atom symbol *///was sym[3], changed to sym[4] by kka on 12/2/08
  char           map[3];         /* atom mapping */
  char           chg[3];         /* atom charge */
  int            x;              /* x-coord */
  int            y;              /* y-coord */
  Boolean_t      isC;            /* Is carbon? */
  Dsp_adj_info_t adj_info;       /* Adjacent info structure */

  /* The following fields are used for selection procedures only */
  int            kvadr;          /* position of atom to pointer */
  Boolean_t      xp;             /* Info about the selection curve */
  Boolean_t      xn;             /* relatively to this atom: does it */
  Boolean_t      yp;             /* cross the axises of the atom. */
  Boolean_t      yn;             /* xp - x/positive, xn - x/negative, etc. */
  int            isSelected;     /* Is atom selected? */
  } Dsp_Atom_t;
#define DSP_ATOM_SIZE (sizeof (Dsp_Atom_t))

/**  The Bond Description  **/

typedef struct dsp_bond_s
  {
  Dsp_Atom_t    *latm_p;             /* pointer to "left" atom */
  Dsp_Atom_t    *ratm_p;             /* pointer to "right" atom */
  U8_t           nlines;             /* number of lines in bond */
  U8_t           stereo;             /* stereochemistry of bond */
  int            isSelected;         /* selection information */
  } Dsp_Bond_t;
#define DSP_BOND_SIZE (sizeof (Dsp_Bond_t))

/**  The Molecule Description  **/

typedef struct dsp_molecule_s
  {
  Dsp_Atom_t    *atoms;            /* atoms of molecule */
  Dsp_Bond_t    *bonds;            /* bonds of molecule */
  U16_t          natms;            /* number of atoms */
  int            nbnds;            /* number of bonds */
  int            nallocatms;       /* number of allocated atoms */
  int            nallocbnds;       /* number of allocated bonds */
  Dimension      molh;             /* height of molecule */
  Dimension      molw;             /* width of molecule */
  Dsp_Atom_t    *rxncnr_p;         /* pointer to rxn center atom */
  float          scale;            /* relative scaling factor */
  Boolean_t      map_em;           /* display the atom mappings? */
  } Dsp_Molecule_t;
#define DSP_MOLECULE_SIZE (sizeof (Dsp_Molecule_t))


/* Function prototypes (defined in dsp.c) */

Dsp_Atom_t  *get_Atom_There (Dsp_Molecule_t *, int, int);
Dsp_Bond_t  *get_Bond_There (Dsp_Molecule_t *, int, int, int, int);
void         delete_Atom    (Dsp_Molecule_t *, Dsp_Atom_t *);
void         delete_Bond    (Dsp_Molecule_t *, Dsp_Bond_t *);
Dsp_Atom_t  *store_Atom     (Dsp_Molecule_t *, char *, char *, char *, 
                               int, int);
Dsp_Bond_t  *store_Bond     (Dsp_Molecule_t *, int, int, int, int, int);
void         copy_Molecule  (Dsp_Molecule_t *, Dsp_Molecule_t *);
void         free_Molecule  (Dsp_Molecule_t *);

Dsp_Molecule_t *Xtr2Dsp     (Xtr_t *);
Xtr_t       *Dsp2Xtr        (Dsp_Molecule_t *);
Boolean_t    dsp_Shelley    (Dsp_Molecule_t *);
int          draw2_         (long int *, long int *, float *, float *);
#endif
