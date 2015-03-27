#ifndef _H_XTRFUNGROUP_
#define _H_XTRFUNGROUP_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     FUNCGROUPS.H
*
*    This module is the abstraction for the Functional Group data-structure.
*    As such it is a sub-structure of the XTR.  The functional groups are
*    used to represent at a higher-level a lot of the electronic effects
*    and reaction possibilities of molecules.
*
*    Routines are found in FUNCGROUPS.C unless otherwise noted.
*
*  Creation Date:
*
*    01-Jan-1991
*
*  Authors:
*
*    Tito Autrey (rewritten based on others PLI code)
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred       xxx
*
******************************************************************************/

#ifndef _H_SYNIO_
#include "synio.h"
#endif

#ifndef _H_ARRAY_
#include "array.h"
#endif

/*** Literal Values ***/

#define MX_FUNCGROUPS 1024
#define FUNCGRP_INVALID  (U16_t)-200
#define FUNCGRP_FILENAME "/fgdata.isam"

#define READ_ONLY	20
#define	WRITE_ONLY	22
#define ENCODE_FILENAME	"/fgdata.encode"


#define ALCOHOL           1
#define CARBONYL          2
#define CC_DOUBLE         3
#define ETHER             4
#define CARBOXYLIC_ACID   5
#define CC_TRIPLE         6
#define CARBOXYLIC_ESTER  7
#define HALOGEN           8
#define PEROXIDE          9
#define CN_SINGLE         10
#define CN_DOUBLE         11
#define CN_TRIPLE         12
#define NN_DOUBLE_TRIPLE  13
#define PYRIDINE          14
#define CARBOXYLIC_AMIDE  15
#define ORGANOSULFUR      16
#define NO_SINGLE_DOUBLE  17
#define PYRYLIUM          18
#define ARYNE             19
#define ORGANOMAGNESIUM   20
#define ORGANOLITHIUM     21
#define ORGANOPHOSPHORUS  22
#define PYRAZOLE          23
#define OXAZOLE           24
#define FURAN             25
#define BLANK_26          26
#define HYDROCARBONS_1    27
#define CARBOCYCLE_SIZE3  28
#define CARBOCYCLE_SIZE4  29
#define CARBOCYCLE_SIZE5  30
#define CARBOCYCLE_SIZE6  31
#define CARBOCYCLE_SIZE7  32
#define CARBOCYCLE_SIZE8  33
#define METHYLENES        34
#define O_ESTERS          35
#define BLANK_36          36
#define BLANK_37          37
#define BLANK_38          38
#define CARBENE_NITRENE   39
#define AROMATIC_RING     40
#define ALDEHYDE          50
#define KETONE            51
#define KETENE            58
#define ALLENE            61
#define EPOXIDE           65
#define HEMIACETAL        71
#define ANHYDRIDE         79
#define ACYL_HALIDE       85
#define HALOFORMATE       95
#define HYPOHALITE        96
#define ARYL_DIAZONIUM    114
#define ISOCYANATE        116
#define DIAZO             119
#define TRIMETHYLSILYL_ETHER 190
#define THIOACID          303
#define ISOTHIOCYANATE    311
#define ENETHIOL          312
#define EPISULFIDE        315
#define SULFENYL_HALIDE   318
#define SULFINYL_HALIDE   319
#define THIONE            329
#define AZIRIDINE         350
#define DIAZIRIDINE       355
#define ISONITRILE        363
#define CARBODIIMIDE      364

/*** Data Structures ***/

/* Functional groups have numbers.  The mapping from numbers to Slings
   (molecular fragments) is kept in the Func. Group data file.  The
   structures for manipulating that file are described in FUNCGROUP_FILE.H.
   The sum of all fragments in encoded and is described in FG2ENCODE.C and
   in FUNCGROUPS.C.

   The results of searching a molecule for Functional Groups is stored in
   the data-structure FuncGroups_t.  There is one array which records
   the # of occurences and the root atom for each occurence for all
   Func. Groups.  Another array records which bonds belong to preservable
   Func. Groups.  And lastly the number of non-hydrogen atoms is recorded.
*/

typedef struct s_fungro
  {
  Array_t       substructures;             /* Array for substructure table
                                              (2-d word array), 0th column
                                              is for count */
  Array_t       preservable;               /* Array of flags of bonds
                                              (2-d bit array) */
  U16_t         num_nonhydrogen;           /* # non-hydrogen atoms */
  } FuncGroups_t;
#define FUNCGROUPSSIZE sizeof (FuncGroups_t)

/** Field Access Macros for a FuncGroups_t **/

/* Macro Prototypes
   U16_t     FuncGrp_NumNonHydrogen_Get       (FuncGroups_t *);
   void      FuncGrp_NumNonHydrogen_Put       (FuncGroups_t *, U16_t);
   U16_t     FuncGrp_NumSubstructures_Get     (FuncGroups_t *);
   Array_t  *FuncGrp_Preservable_Get          (FuncGroups_t *);
   Boolean_t FuncGrp_PreservableBond_Get      (FuncGroups_t *, U16_t, U8_t);
   Boolean_t FuncGrp_PreservableBond_Put      (FuncGroups_t *, U16_t, U8_t,
     Boolean_t);
   U16_t     FuncGrp_SubstructureCount_Get    (FuncGroups_t *, U16_t);
   void      FuncGrp_SubstructureCount_Put    (FuncGroups_t *, U16_t, U16_t);
   U16_t     FuncGrp_SubstructureInstance_Get (FuncGroups_t *, U16_t, U8_t);
   void      FuncGrp_SubstructureInstance_Put (FuncGroups_t *, U16_t, U8_t,
     U16_t);
   Array_t  *FuncGrp_Substructures_Get        (FuncGroups_t *);
*/

#ifndef XTR_DEBUG
#define FuncGrp_NumNonHydrogen_Get(fg_p)\
  (fg_p)->num_nonhydrogen

#define FuncGrp_NumNonHydrogen_Put(fg_p, value)\
  (fg_p)->num_nonhydrogen = (value)

#define FuncGrp_NumSubstructures_Get(fg_p)\
  Array_Columns_Get (FuncGrp_Substructures_Get (fg_p))

#define FuncGrp_Preservable_Get(fg_p)\
  &(fg_p)->preservable

#define FuncGrp_PreservableBond_Get(fg_p, atom, bond)\
  Array_2d1_Get (FuncGrp_Preservable_Get (fg_p), atom, bond)

#define FuncGrp_PreservableBond_Put(fg_p, atom, bond, value)\
  Array_2d1_Put (FuncGrp_Preservable_Get (fg_p), atom, bond, value)

#define FuncGrp_SubstructureCount_Get(fg_p, substructure)\
  Array_2d16_Get (FuncGrp_Substructures_Get (fg_p), 0, substructure)

#define FuncGrp_SubstructureCount_Put(fg_p, substructure, value)\
  Array_2d16_Put (FuncGrp_Substructures_Get (fg_p), 0, substructure, value)

#define FuncGrp_SubstructureInstance_Get(fg_p, substructure, instance)\
  Array_2d16_Get (FuncGrp_Substructures_Get (fg_p), instance, substructure)

#define FuncGrp_SubstructureInstance_Put(fg_p, substructure, instance, value)\
  Array_2d16_Put (FuncGrp_Substructures_Get (fg_p), instance,\
    substructure, value)

#define FuncGrp_Substructures_Get(fg_p)\
  &(fg_p)->substructures
#else
#define FuncGrp_NumNonHydrogen_Get(fg_p)\
  ((fg_p) < GBAddr ? HALT : (fg_p)->num_nonhydrogen)

#define FuncGrp_NumNonHydrogen_Put(fg_p, value)\
  { if ((fg_p) < GBAddr) HALT; else (fg_p)->num_nonhydrogen = (value); }

#define FuncGrp_NumSubstructures_Get(fg_p)\
  ((fg_p) < GBAddr ? HALT : Array_Columns_Get (FuncGrp_Substructures_Get (fg_p)))

#define FuncGrp_Preservable_Get(fg_p)\
  ((fg_p) < GBAddr ? HALT : &(fg_p)->preservable)

#define FuncGrp_PreservableBond_Get(fg_p, atom, bond)\
  ((fg_p) < GBAddr ? HALT : Array_2d1_Get (FuncGrp_Preservable_Get (fg_p),\
  atom, bond))

#define FuncGrp_PreservableBond_Put(fg_p, atom, bond, value)\
  { if ((fg_p) < GBAddr) HALT; else Array_2d1_Put (FuncGrp_Preservable_Get (\
 fg_p), atom, bond, value); }

#define FuncGrp_SubstructureCount_Get(fg_p, substructure)\
  ((fg_p) < GBAddr ? HALT : Array_2d16_Get (FuncGrp_Substructures_Get (fg_p),\
  0, substructure))

#define FuncGrp_SubstructureCount_Put(fg_p, substructure, value)\
  { if ((fg_p) < GBAddr) HALT; else Array_2d16_Put (\
  FuncGrp_Substructures_Get (fg_p), 0, substructure, value); }

#define FuncGrp_SubstructureInstance_Get(fg_p, substructure, instance)\
  ((fg_p) < GBAddr ? HALT : Array_2d16_Get (FuncGrp_Substructures_Get (fg_p),\
  instance, substructure))

#define FuncGrp_SubstructureInstance_Put(fg_p, substructure, instance, value)\
  { if ((fg_p) < GBAddr) HALT; else Array_2d16_Put (\
  FuncGrp_Substructures_Get (fg_p), instance, substructure, value); }

#define FuncGrp_Substructures_Get(fg_p)\
  ((fg_p) < GBAddr ? HALT : &(fg_p)->substructures)
#endif

/** End of Field Access Macros for FuncGroups_t **/

/*** Macros ***/

/* Macro Prototypes
   U16_t     FuncGroups_NumGroups_Get       (void);
   Boolean_t FuncGroups_Substructure_Exists (FuncGroups_t *, U16_t);
*/

#ifndef XTR_DEBUG
#define FuncGroups_NumGroups_Get()\
  Isam_NextKey_Get (&SFuncGrpFile)

#define FuncGroups_Substructure_Exists(fg_p, substructure)\
  (Array_2d16_Get (FuncGrp_Substructures_Get (fg_p), 0, substructure) != 0\
    ? TRUE : FALSE)
#else
#define FuncGroups_NumGroups_Get()\
  Isam_NextKey_Get (&SFuncGrpFile)

#define FuncGroups_Substructure_Exists(fg_p, substructure)\
  ((fg_p) < GBAddr ? HALT : Array_2d16_Get (FuncGrp_Substructures_Get (fg_p),\
  0, substructure) != 0 ? TRUE : FALSE)
#endif

/* In XTR.H
   Xtr_FuncGrp_AnyInstance_Check
   Xtr_FuncGrp_NumSubstructures_Get
   Xtr_FuncGrp_PreservableBonds_Set
   Xtr_FuncGrp_SubstructureInstance_Get
   Xtr_FuncGrps_Equiv
*/

/*** Routine Prototypes ***/

FuncGroups_t *FuncGroups_Copy    (FuncGroups_t *);
void          FuncGroups_Destroy (FuncGroups_t *);
void          FuncGroups_Dump    (FuncGroups_t *, FileDsc_t *);
void          FuncGroups_Init    (U8_t *);
void          FuncGroups_Reset   ();


/* In XTR.H
   FuncGroups_Create
   Xtr_FuncGrp_SubstructureInstance_IsOk
*/

/*** Global Variables ***/

/* End of FuncGroups.H */
#endif
