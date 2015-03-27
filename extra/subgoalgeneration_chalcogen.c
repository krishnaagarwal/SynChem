/******************************************************************************
*
*  Copyright (C) 1993-1996, Synchem Group at SUNY-Stony Brook 
*
*  Module Name:                     SUBGOALGENERATION.C
*
*    This module contains all the code for generating subgoals from a given
*    target compound (molecule) and a given reaction schema.  The reaction 
*    schema will have been tested for suitability by React_Schema_IsOk prior
*    to calling this module.
*
*    Each schema may have multiple pieces in it, each has its own syntheme
*    and syntheme root atom.  Each root can be embedded in potentially several
*    places in the target molecule.  For each set of embeddings of all the
*    goal pattern root atoms that is unique a subgoal is generated and for 
*    each syntheme root atom in the goal pattern there is potentially one
*    compound.  The "potentially" comes about from retrosynthetic ring-forming
*    reactions where the syntheme roots lie in the same molecule even though
*    each represents a distinct piece in the goal pattern.
*
*    This routine will be called once for each reaction schema that passes
*    React_Schmema_IsOk for a given target molecule.
*
*    These routines used to be contained in SUBGENR.PLI, MATCH.PLI, SEPMOL.PLI,
*     FORMCG.PLI, FORMPR.PLI, SEPERAT.PLI
*
*  Routines:
*
*    Duplicates_Check
*    Match_Tree_Dump
*    SubGenr_Subgoals_Generate
*    SubGenr_Active_Match
*    SubGenr_Fragment_Match
*    SubGenr_MatchCB_Create
*    SubGenr_MatchCB_Destroy
*    SAlkyl_Check
*    SApplication_IsOk
*    SAtoms_Inactive_Set
*    SAtoms_Remaining_Set
*    SCheck_Tsd_Map
*    SCompound_Destroy
*    SConstant_Set
*    SEmpties_Set
*    SGenerate
*    SGoal_ActiveBonds_Set
*    SGoal_Atom_Set
*    SGoal_Bond_IsActive
*    SGoal_Complete
*    SMatch_All_Find
*    SMatch_AllCB_Destroy
*    SMatch_Bond
*    SMatch_Extension_Find
*    SMatch_Id
*    SMatch_Info_Clear
*    SMatch_Marker
*    SMatch_Next_Find
*    SMatch_Node_Create
*    SMatch_Pattern_Find
*    SMatch_Piece_Set
*    SMatch_Tetrahedral_Check
*    SMatch_Top
*    SMatch_Transform
*    SMatch_Tree_Dump
*    SMatch_Tree_Setup
*    SMatch_Trigonal_Check
*    SMerit_Update
*    SPi_Orbital_Check
*    SSchema_Active_Find
*    SSchema_Apply
*    SSeparate
*    SSeparate_Count
*    SStrategicBond_Exists
*    SSubgoal_Atom_Set
*    SSubgoal_Bond_IsActive
*    SSubgoal_Sort
*    SVariable_Set
*
*  Creation Date:
*
*    01-Jan-1993
*
*  Authors:
*
*    Daren Krebsbach
*    Tito Autrey (rewritten based on others PLI code)
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 23-Sep-97  Krebsbach  Combined sequential and distributed versions
*                       of SYNCHEM into single file.
* 14-Jan-97  Krebsbach  Major modifications to avoid duplicate name creation
*                       and functional group creation; and to ensure ring 
*                       definition and functional group creation are done 
*                       with canonical xtr's.
* 12-Sep-00  Miller     Added "sneak preview" to find interfering matches that
*                       would previously have prevented a MAO application.  The
*                       preview sets a new variable (max_possible_noninterfering_matches)
*                       to replace max_possible_matches in the appropriate places.
*                       Also, when the two are not equivalent (i.e., there is an
*                       interfering set of matches), the values of these variables
*                       are not modified during a backtrack, lest another MAX
*                       NONINTERFERING APP be bypassed.  (There can only be one
*                       TRUE MAX APP, but there may be several combinations that
*                       lead to a MAX NONINTERFERING APP.)  Finally required that
*                       "done" logic only be used with 1-piece goal patterns,
*                       again accommodating interference-related combinatorics.
* 24-Aug-01  Miller     Corrected Array_Copy() to Array_CopyContents() in
*                       SMatch_Top() to avoid orphaned memory.  Also freed linked
*                       lists within MatchCB_t before destroying.
* 11-Sep-01  Miller     Modified Duplicates_Check to ensure that strings are
*                       created properly, never with the dangerous String_Make!
* 
******************************************************************************/
#include <stdlib.h>
#include <string.h>

#include "synchem.h"
#include "synio.h"
#include "debug.h"

#ifndef _H_ATOMSYM_
#include "atomsym_chalcogen.h"
#endif

#ifndef _H_ARRAY_
#include "array.h"
#endif

#ifndef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_TSD_
#include "tsd.h"
#endif

#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_SUBGOALGENERATION_
#define _GLOB_SUBGENR_DEF_
#include "subgoalgeneration.h"
#undef _GLOB_SUBGENR_DEF_
#endif

#ifndef _H_TSD2XTR_
#include "tsd2xtr.h"
#endif

#ifndef _H_SLING2XTR_
#include "sling2xtr.h"
#endif

#ifndef _H_PST_
#include "pst.h"
#endif

#ifndef _H_SGLSHOT_
#include "sglshot.h"
#endif

/* Static Routine Prototypes */

static void      SNode_Destroy            (Match_Node_t *);
static void      SLeaf_Destroy            (Match_Leaf_t *);
static Boolean_t SAlkyl_Check             (Xtr_t *, U16_t, U16_t, MatchCB_t *);
static Boolean_t SApplication_IsOk        (U16_t, U16_t, U16_t);
static void      SAtoms_Inactive_Set      (Tsd_t *, Tsd_t *, Tsd_t *, Xtr_t *, 
		   Xtr_t *, Array_t *, U16_t, U16_t, Boolean_t *, Boolean_t *, 
		   U16_t *, U16_t *);
static void      SAtoms_Remaining_Set     (Tsd_t *, Tsd_t *, Tsd_t *, U16_t, 
		   U16_t, Boolean_t *, Boolean_t *, U16_t *, U16_t *);
static void      SCheck_Tsd_Map           (Xtr_t *, Array_t *);
static void      SCompound_Destroy        (SubGenr_Compound_t *);
static void      SConstant_Set            (Tsd_t *, Tsd_t *, Tsd_t *, Tsd_t *, 
		   Xtr_t *, Xtr_t *, Array_t *, U16_t, U16_t);
static void      SEmpties_Set             (Tsd_t *, Tsd_t *, U16_t, U16_t, 
		   Boolean_t *, Boolean_t *, U16_t *, U16_t *);
static Boolean_t SGenerate                (SubGenr_Compound_t *, Xtr_t *, 
		   Array_t *, Array_t *, String_t *, Sling_t *, U32_t, U16_t, 
		   U16_t, Boolean_t, Boolean_t, U16_t, Boolean_t, Boolean_t *, 
		   SShotInfo_t *);
static void      SGoal_ActiveBonds_Set    (Tsd_t *, Tsd_t *, Xtr_t *, Xtr_t *,
		   Array_t *, U16_t, U16_t, Boolean_t *, U16_t *);
static void      SGoal_Atom_Set           (Tsd_t *, Tsd_t *, U16_t, U16_t,  
		   U16_t);
static Tsd_t    *SGoal_Complete           (Tsd_t *, Tsd_t *, U16_t);
static Boolean_t SGoalBond_IsActive       (Xtr_t *, Xtr_t *, U16_t, U16_t);
static U16_t     SMatch_All_Find          (Xtr_t *, Xtr_t *, Xtr_t *, 
		   Array_t *, Array_t *, Array_t *, U16_t);
static void      SMatch_AllCB_Destroy     (Array_t *);
static Boolean_t SMatch_Bond              (U16_t, U16_t, U16_t, U16_t, 
		   MatchCB_t *);
static Boolean_t SMatch_Extension_Find    (U16_t, U16_t, U16_t, U16_t,
		   MatchCB_t *);
static Boolean_t SMatch_Id                (U16_t, U16_t, MatchCB_t *);
static void      SMatch_Info_Clear        (Array_t *, Array_t *, Array_t *,
		   Array_t *, U16_t, U16_t, U16_t, U16_t);
static Match_Leaf_t *SMatch_Leaf_Create   (void);
static void      SMatch_Marker            (Array_t *, SubGenr_Compound_t *, 
		   U16_t, U16_t, U16_t);
static U16_t     SMatch_Max               (React_Head_t *);
static void      SMatch_Next_Find         (Xtr_t *, Xtr_t *, Array_t *, 
		   Array_t *, Array_t *, Array_t *, Array_t *, Array_t *, 
		   Array_t *, Match_Piece_t *, Array_t *, Array_t *, Array_t *,
		   U16_t, U16_t, U16_t, Boolean_t, U16_t *, Boolean_t *);
static Match_Node_t *SMatch_Node_Create   (void);
static void      SMatch_Pattern_Find      (MatchCB_t *, U16_t);
static void      SMatch_Piece_Set         (Match_Piece_t *, U16_t);
static Boolean_t SMatch_Tetrahedral_Check (U16_t, U16_t, U16_t, U16_t,
		   MatchCB_t *);
static Boolean_t SMatch_Top               (MatchCB_t *);
/*
static void      SMatch_Transform         (SubGenr_Compound_t *, Tsd_t *, 
		   Xtr_t *, Tsd_t *, Xtr_t *, Array_t *, Array_t *, U16_t, 
		   U16_t, U16_t, U16_t, Boolean_t);
*/
static Boolean_t SMatch_Transform         (SubGenr_Compound_t *, Tsd_t *, 
		   Xtr_t *, Tsd_t *, Xtr_t *, Array_t *, Array_t *, U16_t, 
		   U16_t, U16_t, U16_t, Boolean_t);
static void      SMatch_Tree_Dump         (Match_Node_t *, U16_t, FILE *);
static void      SMatch_Tree_Setup        (Array_t *, Array_t *, Array_t *);
static Boolean_t SMatch_Trigonal_Check    (U16_t, U16_t, U16_t, U16_t,
		   MatchCB_t *);
static Boolean_t SMerit_Update            (SubGenr_Compound_t *, U32_t, U16_t, 
		   S16_t, S16_t, S16_t, U16_t, U16_t, SShotInfo_t *);
static Boolean_t SPi_Orbital_Check        (Xtr_t *, U16_t, U16_t);
static void      SSchema_Active_Find      (Xtr_t *, Xtr_t *, U16_t, Array_t *,
		   Array_t *);
static Tsd_t    *SSchema_Apply            (Tsd_t *, Tsd_t *, Xtr_t *, Tsd_t *,
		   Xtr_t *, Tsd_t *, Array_t *, Array_t *, U16_t, U16_t);
static void      SSeparate                (Tsd_t *, List_t *, Array_t *);
static U16_t     SSeparate_Count          (Tsd_t *, List_t *, Array_t *);
static Boolean_t SStrategicBond_Exists    (SubGenr_Compound_t *, U16_t);
static void      SSubgoal_ActiveBonds_Set (Tsd_t *, Tsd_t *, Xtr_t *, Xtr_t *,
		   Array_t *, U16_t, U16_t, Boolean_t *, U16_t *);
static void      SSubgoal_Atom_Set        (Tsd_t *, Tsd_t *, U16_t, U16_t, 
		   U16_t, U16_t);
static Boolean_t SSubgoalBond_IsActive    (Xtr_t *, Xtr_t *, U16_t, U16_t);
static void      SSubgoal_Sort            (Array_t *, Array_t *, Sling_t *, 
		   Array_t *, Array_t *, U16_t *, U16_t, Boolean_t);
static void      SVariable_Set            (Tsd_t *, Tsd_t *, Tsd_t *, Tsd_t *, Tsd_t *,
		   Array_t *, U16_t, U16_t, Boolean_t *);
static void      SApplicationMaps_Collect (Array_t *, Array_t *, List_t *, U32_t ***, U16_t, U16_t);
static Boolean_t SRepeat_Tsd              (Tsd_t *[], U16_t, Array_t *, char *);

/* Static Data Declarations */

static U8_t SPermutations[MAX_PERMS][4] = {
   { BOND_DIR_UP, BOND_DIR_DOWN, BOND_DIR_LEFT, BOND_DIR_RIGHT },
   { BOND_DIR_UP, BOND_DIR_LEFT, BOND_DIR_RIGHT, BOND_DIR_DOWN },
   { BOND_DIR_UP, BOND_DIR_RIGHT, BOND_DIR_DOWN, BOND_DIR_LEFT },
   { BOND_DIR_DOWN, BOND_DIR_UP, BOND_DIR_RIGHT, BOND_DIR_LEFT },
   { BOND_DIR_DOWN, BOND_DIR_LEFT, BOND_DIR_UP, BOND_DIR_RIGHT },
   { BOND_DIR_DOWN, BOND_DIR_RIGHT, BOND_DIR_LEFT, BOND_DIR_UP },
   { BOND_DIR_LEFT, BOND_DIR_UP, BOND_DIR_DOWN, BOND_DIR_RIGHT },
   { BOND_DIR_LEFT, BOND_DIR_DOWN, BOND_DIR_RIGHT, BOND_DIR_UP },
   { BOND_DIR_LEFT, BOND_DIR_RIGHT, BOND_DIR_UP, BOND_DIR_DOWN },
   { BOND_DIR_RIGHT, BOND_DIR_UP, BOND_DIR_LEFT, BOND_DIR_DOWN },
   { BOND_DIR_RIGHT, BOND_DIR_DOWN, BOND_DIR_UP, BOND_DIR_LEFT },
   { BOND_DIR_RIGHT, BOND_DIR_LEFT, BOND_DIR_DOWN, BOND_DIR_UP } };

static U32_t current_schema; /* because "Major foobar!" tells me absolutely nothing! */


/****************************************************************************
*  MSepMol - should NOT be a static internal procedure!
*    restored here to take exactly what the original did
****************************************************************************/
void MSepMol (Tsd_t *tsd_p, void **mptr, int *npc, int *map)
{
  List_t        *list_p;
  ListElement_t *elem_p;
  Array_t        subgmap;
  int            num_atoms, i;
  struct molecs
    {
    struct molecs *next;
    Tsd_t *tptr;
    }           *molecptr;

  num_atoms = Tsd_NumAtoms_Get (tsd_p);
#ifdef _MIND_MEM_
  mind_Array_2d_Create ("&subgmap", "subgoalgeneration{1}", &subgmap, num_atoms, 2, WORDSIZE);
#else
  Array_2d_Create (&subgmap, num_atoms, 2, WORDSIZE);
#endif
  list_p = List_Create (LIST_NORMAL);
  SSeparate (tsd_p, list_p, &subgmap);
  *npc = List_Size_Get (list_p);
  if (*npc == 0) molecptr = NULL;
#ifdef _MIND_MEM_
  mind_malloc ("molecptr", "subgoalgeneration{1}", &molecptr, sizeof (struct molecs));
#else
  else molecptr = (struct molecs *) malloc (sizeof (struct molecs));
#endif
  *mptr = (void *) molecptr;

  for (elem_p = List_Front_Get (list_p); elem_p != NULL; elem_p = LstElem_Next_Get (elem_p))
    {
    molecptr->tptr = (Tsd_t *) LstElem_ValueAdd_Get (elem_p);
    if (LstElem_Next_Get (elem_p) == NULL) molecptr->next = NULL;
    else
      {
#ifdef _MIND_MEM_
      mind_malloc ("molecptr->next", "subgoalgeneration{1}", &molecptr->next, sizeof (struct molecs));
#else
      molecptr->next = (struct molecs *) malloc (sizeof (struct molecs));
#endif
      molecptr = molecptr->next;
      }
    }

  for (i = 0; i < num_atoms; i++)
    {
    map[2 * i] = Array_2d16_Get (&subgmap, i, SUBG_COL);
    map[2 * i + 1] = Array_2d16_Get (&subgmap, i, ATOM_COL);
    }
}


/****************************************************************************
*
*  Function Name:                 SubGenr_Subgoals_Generate
*
*    This procedure guides the construction of a set of subgoals for the input
*    compound (molecule) using the specified reaction schema.  The process
*    requires using the Match algorithm (graph isomorphism, NP-Complete) to
*    generate a set (could be more than 1) of possible matches for each piece
*    of the goal pattern.  Multiple matches are generally caused by symmetry
*    in the input compound.  The goal pattern may have more than one piece if it
*    is a ring opening reaction.  The subgoal pattern may have more than one
*    piece if it is a ring closure reaction.  Likewise for disposing of a FG or
*    for adding a FG, respectively.  Each embedding for a given piece is called
*    an application.  A complete match is formed by finding a set of
*    applications that covers all the pieces and has no interfering atom or bond
*    mappings (could be broken or added bonds/atoms) and doesn't destroy
*    preservable substructures.
*
*    Each complete match generates one subgoal.  Each separate piece in the
*    subgoal pattern generates one compound for that subgoal, except for
*    retrosynthetic ring-opening reactions which map at least 2 root atoms
*    to the same piece.  These compounds must be built up to form the subgoal
*    and once they are all accumulated then the subgoal may be generated and
*    placed in the Problem Solving Tree (PST).
*
*    There are some N**2 algorithms that can probably be optimized, but until
*    performance results are available showing they contribute to the runtime
*    of this module it isn't worth obscuring the match with the original PL/I
*    any further.
*
*    Note that the caller must ensure that the Funcational Groups are attached
*    to the target molecule.
*
*  Used to be:
*
*    subgenr:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    Calls Pst_Subgoal_Insert
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    Allocates memory (temporarily)
*    May call IO_Exit_Error
*
******************************************************************************/
void SubGenr_Subgoals_Generate
  (
  Xtr_t        *molecule_p,                 /* Handle for mole. to process */
  Array_t      *strategicbonds_p,           /* 2d-bit, strategic bonds */
  Array_t      *stereooptions_p,            /* 1d-bit, stereo options flags */
  U32_t         schema,                     /* Schema to apply */
  Boolean_t     stereo_chemistry,           /* Flag for stereochemistry */
  Boolean_t     preserve_substructures,     /* Flag for preservable FGs */
  SShotGenSGs_t *ssgensg_p
  )
{
  Match_Piece_t *matchpiece_base_p;         /* Base of match piece vector */
  Match_Piece_t *match_piece_p;             /* For manip. entry in vector */
  MatchCB_t    *mcb_p;                      /* Temp for match_trees array */
  SubGenr_Compound_t *compound_base_p;      /* Base for compounds vector */
  React_Record_t *react_p;                  /* Reaction schema record */
  React_Head_t *head_p;                     /* Reaction schema header */
  Tsd_t        *mole_tsd_p;                 /* Molecule TSD handle */
  Tsd_t        *goal_tsd_p;                 /* Goal TSD handle */
  Tsd_t        *subgoal_tsd_p;              /* Subgoal TSD handle */
  Xtr_t        *goal_xtr_p;                 /* Goal XTR handle */
  Xtr_t        *subgoal_xtr_p;              /* Subgoal XTR handle */
  Stack_t      *match_stack_p;              /* Stack for assembling complete
    matches */
  Stack_t      *piece_stack_p;              /* Ditto, but for goal pieces array
    */
  AtomArray_t  *tmparr_p;                   /* Temporary for stack manip. */
  Array_t      *complete_match_p;           /* Handle for current complete match
    */
  Array_t      *goal_pieces_p;              /* Same, but for goal pieces */
  Sling_t      *comp_slings_p;              /* Sling accumulation array */
  String_t     *hashstring_p;               /* Sling (as String_t) hash table */
  U16_t         num_atoms;                  /* # atoms in goal molecule */
  U16_t         goal_pattern_size;          /* # atoms in fixed part of goal
    pattern */
  U16_t         goal_num_atoms;             /* Was complet_goal_pattern_size */
  U16_t         num_goal_pieces;            /* # pieces in goal pattern */
  U16_t         syntheme;                   /* Which syntheme consider now */
  U16_t         max_instances;              /* Max # instances of a syntheme */
  U16_t         max_possible_matches;       /* Max # matches allowed by various
    constraints. */
  U16_t         max_possible_noninterfering_matches;   /* Max # matches allowed by various
    constraints, including "sneak preview" for interfering matches. */
  U16_t         cur_possible_matches;       /* Current match state/rxn type */
  U16_t         max_compounds;              /* Max # compounds possible */
  U16_t         num_piece_matches;          /* # matches for *a* goal piece */
  U16_t         num_matches;                /* Index into spec. match_piece */
  U16_t         num_trees;                  /* # match trees found */
  U16_t         num_duplicates;             /* # duplicates encountered sofar */
  U16_t         i, j, k;                    /* Counters */
  U16_t         tree;                       /* Index into matchcbs */
  U16_t         gproot;                     /* Index into root_nodes */
  U16_t         application;                /* Index */
  U16_t         piece;                      /* Index */
  U16_t         new_match;                  /* Counter */
  U16_t         pop_counter;                /* Counter */
  Boolean_t     sshot_data_ready;           /* Flag - prevent sglshot bomb on MAO rxn */
  Boolean_t     single_max_only;            /* Flag - limited reaction type */
  Boolean_t     done;                       /* Flag - match assembly state */
  Boolean_t     backtracked;                /* Flag - match assembly action */
  Boolean_t     duplicate;                  /* Flag - compound is a duplicate */
  Boolean_t     use_cur;                    /* Flag - which max. poss. match */
  Boolean_t     max_used;                   /* Flag - match assembly state */
  Boolean_t     max_appl_schema;            /* Flag - old NUM_APP hack fix */
  Boolean_t     strategic;                  /* Flag - strategic bond encount. */
  Boolean_t     reject;                     /* Flag - match assembly action */
  Boolean_t     all_rejected;               /* Flag - match assembly action; avoid premature rejection of maxapp */
  Array_t       goal_active_atoms;          /* 1d-bit, active atoms in goal */
  Array_t       goal_active_bonds;          /* 2d-bit, active bonds in goal */
  Array_t       match_trees;                /* 2d-addr, match CBs */
  Array_t       matchcbs;                   /* 1d-addr, compress match_trees */
  Array_t       match_sizes;                /* 1d-word, # trees per matchcb */
  Array_t       root_nodes;                 /* 1d-word, root atm to synthems */
  Array_t       active_atoms;               /* 2d-word, appl + piece #s by
    atom */
  Array_t       active_bonds;               /* 3d-word, appl + piece #s by
    bond */
  Array_t       selector;                   /* 2d-word, tree index by appl and
    piece */
  Array_t       preserve_bonds;             /* 2d-bit, preservable bonds */
  Array_t       hashcount;                  /* 1d-word, # entries in hash
    bucket */
  SShotInfo_t   *sshotinfo_p;
  U32_t        **sshot_maps;

#ifdef _DEBUG_
printf("subgenr entered\n");
#endif
  DEBUG (R_SUBGENR, DB_SUBGENR, TL_PARAMS, (outbuf,
    "Entering SubGenr_Subgoals_Generate, compound = %p, strategic bonds = %p,"
    " stereo options = %p, schema = %lu, stereo flag = %hu, preserve"
    " substructures flag = %hu", molecule_p, strategicbonds_p, 
    stereooptions_p, schema, stereo_chemistry, preserve_substructures));

  current_schema = schema;

  num_atoms = Xtr_NumAtoms_Get (molecule_p);
  react_p = React_Schema_Handle_Get (schema);
  head_p = React_Head_Get (react_p);
  mole_tsd_p = Xtr2Tsd (molecule_p);
  goal_tsd_p = React_Goal_Get (react_p);
  goal_xtr_p = Tsd2Xtr (goal_tsd_p);
  Xtr_Rings_Set (goal_xtr_p);
  goal_pattern_size = React_Head_GoalSize_Get (head_p);
  goal_num_atoms = Xtr_NumAtoms_Get (goal_xtr_p);
  subgoal_tsd_p = React_Subgoal_Get (react_p);
  subgoal_xtr_p = Tsd2Xtr (subgoal_tsd_p);

  /* Find out information about schema transformations.  Specifically which
     atoms in the patterns represent changed nodes and bonds.  The mappings
     for these must be unique.  SSchema_Active_Find computes these active
     entities and records them in the obvious arrays.
  */

#ifdef _MIND_MEM_
  in_subgenr (1);
  mind_Array_1d_Create ("&goal_active_atoms", "subgoalgeneration{2}", &goal_active_atoms, goal_num_atoms, BITSIZE);
  Array_Set (&goal_active_atoms, FALSE);
  mind_Array_2d_Create ("&goal_active_bonds", "subgoalgeneration{2}", &goal_active_bonds, goal_num_atoms, MX_NEIGHBORS, BITSIZE);
  Array_Set (&goal_active_bonds, FALSE);

  SSchema_Active_Find (goal_xtr_p, subgoal_xtr_p, goal_pattern_size, 
    &goal_active_atoms, &goal_active_bonds); 
    

  /* Need to know # goal pattern pieces.  Must be a syntheme root node for
     each one.
  */

  num_goal_pieces = 0;
  while (React_Head_RootNode_Get (head_p, num_goal_pieces) !=
    REACT_NODE_INVALID)
    num_goal_pieces++;
  
  mind_Array_1d_Create ("&root_nodes", "subgoalgeneration{2}", &root_nodes, goal_num_atoms, WORDSIZE);
  Array_Set (&root_nodes, FUNCGRP_INVALID);

  if (Xtr_FuncGroups_Get (molecule_p) == NULL)
    Xtr_FuncGroups_Put (molecule_p, FuncGroups_Create (molecule_p));

  for (i = 0, max_instances = 0; i < num_goal_pieces; i++)
    {
    syntheme = React_Head_RootSyntheme_Get (head_p, i);
    max_instances = MAX (Xtr_FuncGrp_NumInstances_Get (molecule_p, syntheme), 
      max_instances);
    Array_1d16_Put (&root_nodes, React_Head_RootNode_Get (head_p, i),
      syntheme);
    }

  max_possible_matches = SMatch_Max (head_p);

  mind_Array_2d_Create ("&match_trees", "subgoalgeneration{2}", &match_trees, num_atoms, goal_num_atoms, ADDRSIZE);
  Array_Set (&match_trees, (U32_t) NULL);

  num_trees = SMatch_All_Find (molecule_p, goal_xtr_p, subgoal_xtr_p,
    &root_nodes, &match_trees, stereooptions_p, max_possible_matches);

  if (num_trees == 0)
    {
    /* No matches found!  Clean up allocated storage and blow this stack
       frame.
    */

    Tsd_Destroy (mole_tsd_p);
    Xtr_Destroy (goal_xtr_p);
    Xtr_Destroy (subgoal_xtr_p);

    SMatch_AllCB_Destroy (&match_trees);
    mind_Array_Destroy ("&match_trees", "subgoalgeneration", &match_trees);
    mind_Array_Destroy ("&root_nodes", "subgoalgeneration", &root_nodes);
    mind_Array_Destroy ("&goal_active_atoms", "subgoalgeneration", &goal_active_atoms);
    mind_Array_Destroy ("&goal_active_bonds", "subgoalgeneration", &goal_active_bonds);

    DEBUG (R_SUBGENR, DB_SUBGENR, TL_PARAMS, (outbuf,
      "Leaving SubGenr_Subgoals_Generate, status = <void>"));

    return;
    }

  /* The objective of the block of code below is to collect the matches
     found into sets for each piece of the goal pattern.  Each piece may be
     embedded at many different sites in the target molecule.  This means
     that the target molecule has many instances of the syntheme that is
     the root of each particular piece of the goal pattern.

     First compress the match_trees down to vectors that can be directly
     indexed by match #.

     For each piece, count # of matches by summing along the possible
     syntheme roots.  Record each match (matchcb index and tree within
     each matchcb) for each piece.
  */

  mind_Array_1d_Create ("&matchcbs", "subgoalgeneration{2}", &matchcbs, num_trees, ADDRSIZE);
  mind_Array_1d_Create ("&match_sizes", "subgoalgeneration{2}", &match_sizes, num_trees, WORDSIZE);

  SMatch_Tree_Setup (&match_trees, &matchcbs, &match_sizes);

  mind_malloc ("matchpiece_base_p", "subgoalgeneration{2}", &matchpiece_base_p, MATCHPIECESIZE * num_goal_pieces);
  in_subgenr (2);
#else
  Array_1d_Create (&goal_active_atoms, goal_num_atoms, BITSIZE);
  Array_Set (&goal_active_atoms, FALSE);
  Array_2d_Create (&goal_active_bonds, goal_num_atoms, MX_NEIGHBORS, BITSIZE);
  Array_Set (&goal_active_bonds, FALSE);

  SSchema_Active_Find (goal_xtr_p, subgoal_xtr_p, goal_pattern_size, 
    &goal_active_atoms, &goal_active_bonds); 
    

  /* Need to know # goal pattern pieces.  Must be a syntheme root node for
     each one.
  */

  num_goal_pieces = 0;
  while (React_Head_RootNode_Get (head_p, num_goal_pieces) !=
    REACT_NODE_INVALID)
    num_goal_pieces++;
  
  Array_1d_Create (&root_nodes, goal_num_atoms, WORDSIZE);
  Array_Set (&root_nodes, FUNCGRP_INVALID);

  if (Xtr_FuncGroups_Get (molecule_p) == NULL)
    Xtr_FuncGroups_Put (molecule_p, FuncGroups_Create (molecule_p));

  for (i = 0, max_instances = 0; i < num_goal_pieces; i++)
    {
    syntheme = React_Head_RootSyntheme_Get (head_p, i);
    max_instances = MAX (Xtr_FuncGrp_NumInstances_Get (molecule_p, syntheme), 
      max_instances);
    Array_1d16_Put (&root_nodes, React_Head_RootNode_Get (head_p, i),
      syntheme);
    }

  max_possible_matches = SMatch_Max (head_p);

  Array_2d_Create (&match_trees, num_atoms, goal_num_atoms, ADDRSIZE);
  Array_Set (&match_trees, (U32_t) NULL);

  num_trees = SMatch_All_Find (molecule_p, goal_xtr_p, subgoal_xtr_p,
    &root_nodes, &match_trees, stereooptions_p, max_possible_matches);

  if (num_trees == 0)
    {
    /* No matches found!  Clean up allocated storage and blow this stack
       frame.
    */

    Tsd_Destroy (mole_tsd_p);
    Xtr_Destroy (goal_xtr_p);
    Xtr_Destroy (subgoal_xtr_p);

    SMatch_AllCB_Destroy (&match_trees);
    Array_Destroy (&match_trees);
    Array_Destroy (&root_nodes);                                  /* DK */
    Array_Destroy (&goal_active_atoms);
    Array_Destroy (&goal_active_bonds);

    DEBUG (R_SUBGENR, DB_SUBGENR, TL_PARAMS, (outbuf,
      "Leaving SubGenr_Subgoals_Generate, status = <void>"));

    return;
    }

  /* The objective of the block of code below is to collect the matches
     found into sets for each piece of the goal pattern.  Each piece may be
     embedded at many different sites in the target molecule.  This means
     that the target molecule has many instances of the syntheme that is
     the root of each particular piece of the goal pattern.

     First compress the match_trees down to vectors that can be directly
     indexed by match #.

     For each piece, count # of matches by summing along the possible
     syntheme roots.  Record each match (matchcb index and tree within
     each matchcb) for each piece.
  */

  Array_1d_Create (&matchcbs, num_trees, ADDRSIZE);
  Array_1d_Create (&match_sizes, num_trees, WORDSIZE);

  SMatch_Tree_Setup (&match_trees, &matchcbs, &match_sizes);

  Mem_Alloc (Match_Piece_t *, matchpiece_base_p, MATCHPIECESIZE *
    num_goal_pieces, GLOBAL);
#endif

  DEBUG (R_SUBGENR, DB_SUBGENR, TL_MEMORY, (outbuf,
    "Allocated memory for Match Piece array in SubGenr_Subgoals_Generate at %p",
    matchpiece_base_p));

  if (matchpiece_base_p == NULL)
    IO_Exit_Error (R_SUBGENR, X_LIBCALL,
      "No memory for Match Piece array in SubGenr_Subgoals_Generate");

  for (i = 0, gproot = 0; i < num_goal_pieces; i++)
    {
    /* Find node # of next goal pattern root and compress so that the
       piece number (index) also matches the root_node index
    */

    for ( ; gproot < goal_num_atoms && Array_1d16_Get (&root_nodes, gproot)
	 == FUNCGRP_INVALID; gproot++)
      /* Empty loop body */ ;

    if (i < gproot)
      {
      Array_1d16_Put (&root_nodes, i, Array_1d16_Get (&root_nodes, gproot));
      Array_1d16_Put (&root_nodes, gproot, FUNCGRP_INVALID);
      }

    /* Count the matches and then organize into a vector so they can be more
       easily referenced.  We are counting the matches for the piece that
       is rooted at atom # gproot in the goal pattern.  This atom could be
       matched anywhere in the target molecule which is why we iterate over
       all atoms looking for a matchCB.
    */

    for (j = 0, num_piece_matches = 0; j < num_atoms; j++)
      {
      mcb_p = (MatchCB_t *)Array_2dAddr_Get (&match_trees, j, gproot);
      if (mcb_p != NULL)
	num_piece_matches += MatchCB_NumMatches_Get (mcb_p);
      }

    match_piece_p = &matchpiece_base_p[i];
    SMatch_Piece_Set (match_piece_p, num_piece_matches);
    max_possible_matches = MIN (num_piece_matches, max_possible_matches);
    num_matches = 0;

    for (j = 0; j < num_atoms; j++)
      {
      mcb_p = (MatchCB_t *)Array_2dAddr_Get (&match_trees, j, gproot);
      if (mcb_p != NULL)
	{
	for (k = 0, tree = num_trees + 2; k < num_trees && tree ==
	     num_trees + 2; k++)
	  if ((MatchCB_t *)Array_1dAddr_Get (&matchcbs, k) == mcb_p)
	    tree = k;

	if (tree == num_trees + 2) {
	   printf ("Match tree pointer not found on match tree pointer list\n");
	   exit(1);
	} 

	/* Now record the matches based at each root node.  An embedding
	   may take several paths from a particular root so there may
	   be several trees rooted at the same place.
	*/

	for (k = 0; k < Array_1d16_Get (&match_sizes, tree); k++,
	     num_matches++)
	  {
	  Match_Piece_Tree_Put (match_piece_p, num_matches, tree);
	  Match_Piece_Match_Put (match_piece_p, num_matches, k);
	  }
	}
      }

    gproot++;
    }                        /* End for-i loop */

  /* Check for only 1 syntheme type, FG, and note that the number of
     possible matches is then limited by the number of goal pieces because
     they will all be matching at the same places.
  */

  if (Array_1d16_Get (&root_nodes, 0) == Array_1d16_Get (&root_nodes,
      num_goal_pieces - 1))
    max_possible_matches = MAX (max_possible_matches / num_goal_pieces, 1);

#ifdef _MIND_MEM_
  in_subgenr (3);
  mind_Array_Destroy ("&root_nodes", "subgoalgeneration", &root_nodes);
  mind_Array_Destroy ("&match_sizes", "subgoalgeneration", &match_sizes);
  mind_Array_Destroy ("&match_trees", "subgoalgeneration", &match_trees);

  /* End of setting up information about match CBs and trees within each CB
     on a per goal piece basis.

     Now to assemble complete matches and for each one found, generate the
     actual subgoals and compounds so they are placed in the PST.  The
     zeroth entry is the base complete goal compound.
  */

  max_compounds = max_possible_matches;
  mind_malloc ("compound_base_p", "subgoalgeneration{2}", &compound_base_p, SUBGENRCOMPOUNDSIZE * (max_compounds + 1));
  in_subgenr (4);
#else
  Array_Destroy (&root_nodes);
  Array_Destroy (&match_sizes);
  Array_Destroy (&match_trees);                              /* DK */

  /* End of setting up information about match CBs and trees within each CB
     on a per goal piece basis.

     Now to assemble complete matches and for each one found, generate the
     actual subgoals and compounds so they are placed in the PST.  The
     zeroth entry is the base complete goal compound.
  */

  max_compounds = max_possible_matches;
  Mem_Alloc (SubGenr_Compound_t *, compound_base_p, SUBGENRCOMPOUNDSIZE *
    (max_compounds + 1), GLOBAL);
#endif

  DEBUG (R_SUBGENR, DB_SUBGENR, TL_MEMORY, (outbuf,
    "Allocated the compounds array in SubGenr_Subgoals_Generate at %p",
    compound_base_p));

  if (compound_base_p == NULL)
    IO_Exit_Error (R_SUBGENR, X_LIBCALL, "No memory for Compounds vector in\
 SubGenr_Subgoals_Generate");

  memset (compound_base_p, 0, SUBGENRCOMPOUNDSIZE * (max_compounds + 1));

  SubGenr_Compound_Tsd_Put (compound_base_p, mole_tsd_p);
  SubGenr_Compound_Xtr_Put (compound_base_p, molecule_p);
  SubGenr_Compound_Ease_Put (compound_base_p, React_Head_Ease_Get (head_p));
  SubGenr_Compound_Confidence_Put (compound_base_p, MX_CONFIDENCE);
  SubGenr_Compound_Yield_Put (compound_base_p, MX_YIELD);
  SubGenr_Compound_NumCompounds_Put (compound_base_p, 1);
#ifdef _MIND_MEM_
  in_subgenr (5);
  mind_Array_1d_Create ("SubGenr_Compound_AtomHandle_Get(compound_base_p)", "subgoalgeneration{2}",
    SubGenr_Compound_AtomHandle_Get (compound_base_p), num_atoms, BITSIZE);
  Array_Set (SubGenr_Compound_AtomHandle_Get (compound_base_p), TRUE);

  /*
     Selector (i, j) - the match# for the ith application's jth piece
     Compounds.tsd_p (i + 1) - the TSD produced by the ith application
     Compounds.xtr_p (i) - the XTR corresponding to (i).tsd_p
     Compounds.atomsb - flags which atoms in target molecule in this compound
     Active_atoms (i, APPL_IDX/PIECE_IDX) - indexes for Selector to make
       ith atom active.  Used to index Selector array.
     Active_bonds (i, k, APPL_IDX/PIECE_IDX) - ditto for kth bond of ith atom
     Complete_match (i) - atom # in target which matches ith atom in goal
       pattern
     Goal_pieces (i) - the piece index which is used to match the ith pattern
       atom
  */

  mind_Array_2d_Create ("&active_atoms", "subgoalgeneration{2}", &active_atoms, num_atoms, 2, WORDSIZE);
  Array_Set (&active_atoms, MATCH_INVALID);
  mind_Array_3d_Create ("&active_bonds", "subgoalgeneration{2}", &active_bonds, num_atoms, MX_NEIGHBORS, 2, WORDSIZE);
  Array_Set (&active_bonds, MATCH_INVALID);
  mind_Array_2d_Create ("&selector", "subgoalgeneration{2}", &selector, max_possible_matches, num_goal_pieces, WORDSIZE);
  Array_Set (&selector, MATCH_INVALID);
  mind_Array_2d_Create ("&preserve_bonds", "subgoalgeneration{2}", &preserve_bonds, num_atoms, MX_NEIGHBORS, BITSIZE);
  in_subgenr (6);
#else
  Array_1d_Create (SubGenr_Compound_AtomHandle_Get (compound_base_p),
    num_atoms, BITSIZE);
  Array_Set (SubGenr_Compound_AtomHandle_Get (compound_base_p), TRUE);

  /*
     Selector (i, j) - the match# for the ith application's jth piece
     Compounds.tsd_p (i + 1) - the TSD produced by the ith application
     Compounds.xtr_p (i) - the XTR corresponding to (i).tsd_p
     Compounds.atomsb - flags which atoms in target molecule in this compound
     Active_atoms (i, APPL_IDX/PIECE_IDX) - indexes for Selector to make
       ith atom active.  Used to index Selector array.
     Active_bonds (i, k, APPL_IDX/PIECE_IDX) - ditto for kth bond of ith atom
     Complete_match (i) - atom # in target which matches ith atom in goal
       pattern
     Goal_pieces (i) - the piece index which is used to match the ith pattern
       atom
  */

  Array_2d_Create (&active_atoms, num_atoms, 2, WORDSIZE);
  Array_Set (&active_atoms, MATCH_INVALID);
  Array_3d_Create (&active_bonds, num_atoms, MX_NEIGHBORS, 2, WORDSIZE);
  Array_Set (&active_bonds, MATCH_INVALID);
  Array_2d_Create (&selector, max_possible_matches, num_goal_pieces, WORDSIZE);
  Array_Set (&selector, MATCH_INVALID);
  Array_2d_Create (&preserve_bonds, num_atoms, MX_NEIGHBORS, BITSIZE);
#endif

  /* The objective of the following major loop is to examine all the available
     matches for each piece and attempt to find all combinations that lead
     to complete matches.  A complete match must find a match for each piece
     of the goal pattern but without the matches for each piece interfering 
     with each other or with preservable bonds (certain FGs).

     Each schema also has a limit on the number of applications it will allow
     and this limit must be observed.  There is also a limit on the
     number of duplicates that are tolerated.  Regular ring structures can
     create very large numbers of equivilant matches and at some point it is
     no longer worth looking at the matches because you are likely to see only
     more duplicates.

     Keep in mind that with C, arrays are 0-based, so the "first" piece 
     actually has the index 0.  Note also that NUM_GOAL_PIECES is a count
     on the number of pieces, and since the index, PIECE, starts at 0, a
     number of +- 1 operations must be performed.  The same holds true with
     respect to MAX_POSSIBLE_MATCHES and APPLICATION.

     Once a complete match is constructed, other matches can be derived from
     it by backtracking.

     In this case "vertical" means to start another match after skipping
     the applications tried so far.

     And "horizontal" means to add another piece to the match that has been
     processed so far.
  */

  match_stack_p = Stack_Create (STACK_ATMARR);
  piece_stack_p = Stack_Create (STACK_ATMARR);
  tmparr_p = AtmArr_Create (goal_num_atoms, ATMARR_NOBONDS_WORD, XTR_INVALID);
  complete_match_p = AtmArr_Array_Get (tmparr_p);
  Stack_PushAdd (match_stack_p, tmparr_p);
  tmparr_p = AtmArr_Create (goal_num_atoms, ATMARR_NOBONDS_WORD,
    MATCH_INVALID);
  goal_pieces_p = AtmArr_Array_Get (tmparr_p);
  Stack_PushAdd (piece_stack_p, tmparr_p);
  Xtr_FuncGrp_PreservableBonds_Set (molecule_p, &preserve_bonds);

  /* The single_and_max flag exists solely due to the complexity of the
     algorithm, ie potentially exponential in the number of matches and
     number of pieces.  So if the total exceeds some threshold, then we
     drop back to honoring only a certain type of match.
  */

  if (max_possible_matches + num_goal_pieces > MAX_PIECES_MATCHES &&
      max_possible_matches > MAX_MATCHES_ONLY)
    single_max_only = TRUE;
  else
    single_max_only = FALSE;

  done = FALSE;
  backtracked = FALSE;
  application = 0;
  piece = 0;
  num_duplicates = 0;

  /* ZNUM_APP(CHAPTER,SCHEMA); */
  use_cur = FALSE;
  max_used = FALSE;
  cur_possible_matches = React_Head_ReactionType_Get (head_p) ==
    REACT_MAX_APPL_ONLY ? 10000 : 1; 

/*
  max_appl_schema = (use_cur == TRUE ? (max_used == FALSE ? TRUE : FALSE) :
    React_Head_ReactionType_Get (head_p) == REACT_MAX_APPL_ONLY);
*/
#ifdef _MIND_MEM_
  in_subgenr (7);
  mind_Array_1d_Create ("&hashcount", "subgoalgeneration{2}", &hashcount, MX_SUBGENR_HASH, WORDSIZE);
  Array_Set (&hashcount, 0);

  mind_malloc ("hashstring_p", "subgoalgeneration{2}", &hashstring_p, MX_SUBGENR_HASH * STRINGSIZE);
#else
  Array_1d_Create (&hashcount, MX_SUBGENR_HASH, WORDSIZE);
  Array_Set (&hashcount, 0);

  Mem_Alloc (String_t *, hashstring_p, MX_SUBGENR_HASH * STRINGSIZE, GLOBAL);
#endif

  DEBUG (R_SUBGENR, DB_SUBGENR, TL_MEMORY, (outbuf,
    "Allocated memory for Sling hash table in SubGenr_Subgoals_Generate at %p",
    hashstring_p));

  if (hashstring_p == NULL)
    IO_Exit_Error (R_SUBGENR, X_LIBCALL,
      "No memory for Sling hash table in SubGenr_Subgoals_Generate");

  memset (hashstring_p, 0, MX_SUBGENR_HASH * STRINGSIZE);

#ifdef _MIND_MEM_
  in_subgenr (8);
  mind_malloc ("comp_slings_p", "subgoalgeneration{2}", &comp_slings_p, num_atoms * SLINGSIZE);
#else
  Mem_Alloc (Sling_t *, comp_slings_p, num_atoms * SLINGSIZE, GLOBAL);
#endif

  DEBUG (R_SUBGENR, DB_SUBGENR, TL_MEMORY, (outbuf,
    "Allocated memory for compound Sling array in"
    " SubGenr_Subgoals_Generate at %p", comp_slings_p));

  if (comp_slings_p == NULL)
    IO_Exit_Error (R_SUBGENR, X_LIBCALL,
      "No memory for Sling array in SubGenr_Subgoals_Generate");

  memset (comp_slings_p, 0, num_atoms * SLINGSIZE);

max_possible_noninterfering_matches=max_possible_matches;
if (React_Head_ReactionType_Get (head_p) == REACT_MAX_APPL_ONLY)
{
pop_counter=max_possible_noninterfering_matches=0;
/* "Sneak preview" to test for interfering matches */
  match_piece_p = &matchpiece_base_p[piece];
  SMatch_Next_Find (molecule_p, goal_xtr_p, &selector, &active_atoms,
    &active_bonds, &goal_active_atoms, &goal_active_bonds, complete_match_p,
    goal_pieces_p, match_piece_p, &matchcbs, strategicbonds_p, &preserve_bonds,
    application, piece, goal_num_atoms, preserve_substructures, &new_match,
    &strategic);
  while (!(application==0 && piece==0 && new_match == MATCH_INVALID))
  {
    if (new_match==MATCH_INVALID)
    {
      if (piece>0)
      {
        piece--;
        SMatch_Info_Clear (complete_match_p, goal_pieces_p, &active_atoms, 
          &active_bonds, goal_num_atoms, num_atoms, application, piece);
      }
      else if (application>0)
      {
        application--;
        piece=num_goal_pieces-1;
        if (pop_counter>0)
        {
	  Stack_Pop (match_stack_p);
	  Stack_Pop (piece_stack_p);
          pop_counter--;
        }
        tmparr_p = (AtomArray_t *) Stack_TopAdd (match_stack_p);
        complete_match_p = AtmArr_Array_Get (tmparr_p);
        tmparr_p = (AtomArray_t *) Stack_TopAdd (piece_stack_p);
        goal_pieces_p = AtmArr_Array_Get (tmparr_p);
 
        SMatch_Info_Clear (complete_match_p, goal_pieces_p, &active_atoms,
          &active_bonds, goal_num_atoms, num_atoms, application, piece);
      }
    }
    else
    {
      Array_2d16_Put (&selector, application, piece, new_match);
      if (piece==num_goal_pieces-1)
      {
        piece=0;
        application++;
        max_possible_noninterfering_matches=MAX(max_possible_noninterfering_matches,application);
        tmparr_p = AtmArr_Create (goal_num_atoms, ATMARR_NOBONDS_WORD,
          XTR_INVALID);
        complete_match_p = AtmArr_Array_Get (tmparr_p);
        Stack_PushAdd (match_stack_p, tmparr_p);
        tmparr_p = AtmArr_Create (goal_num_atoms, ATMARR_NOBONDS_WORD,
          MATCH_INVALID);
        goal_pieces_p = AtmArr_Array_Get (tmparr_p);
        Stack_PushAdd (piece_stack_p, tmparr_p);
        pop_counter++;
      }
      else piece++;
    }
    match_piece_p = &matchpiece_base_p[piece];
    if (application==max_possible_matches)
    {
      application=piece=0;
      new_match=MATCH_INVALID;
    }
    else SMatch_Next_Find (molecule_p, goal_xtr_p, &selector, &active_atoms,
      &active_bonds, &goal_active_atoms, &goal_active_bonds, complete_match_p,
      goal_pieces_p, match_piece_p, &matchcbs, strategicbonds_p, &preserve_bonds,
      application, piece, goal_num_atoms, preserve_substructures, &new_match,
      &strategic);
  }
  for (i=0; i<pop_counter; i++)
  {
	  Stack_Pop (match_stack_p);
	  Stack_Pop (piece_stack_p);
  }
  Array_Set (&active_atoms, MATCH_INVALID);
  Array_Set (&active_bonds, MATCH_INVALID);
  Array_Set (&selector, MATCH_INVALID);
  tmparr_p = (AtomArray_t *) Stack_TopAdd (match_stack_p);
  complete_match_p = AtmArr_Array_Get (tmparr_p);
  tmparr_p = (AtomArray_t *) Stack_TopAdd (piece_stack_p);
  goal_pieces_p = AtmArr_Array_Get (tmparr_p);
  Array_Set (complete_match_p, XTR_INVALID);
  Array_Set (goal_pieces_p, MATCH_INVALID);
  application=piece=0;
  if (max_possible_noninterfering_matches + num_goal_pieces <= MAX_PIECES_MATCHES ||
      max_possible_noninterfering_matches <= MAX_MATCHES_ONLY) single_max_only = FALSE;
}

  match_piece_p = &matchpiece_base_p[piece];
  SMatch_Next_Find (molecule_p, goal_xtr_p, &selector, &active_atoms,
    &active_bonds, &goal_active_atoms, &goal_active_bonds, complete_match_p,
    goal_pieces_p, match_piece_p, &matchcbs, strategicbonds_p, &preserve_bonds,
    application, piece, goal_num_atoms, preserve_substructures, &new_match,
    &strategic);

  /* This loop should be processed for two cases:
     1) This is the first application to the first piece and a match was found
     2) All possible applications for all pieces have not yet been rejected.
	In this case APPLICATION and PIECE will be reset to 0
  */

  all_rejected = TRUE; /* until first non-rejected SG */

#ifdef _DEBUG_
printf("subgenr entering while loop\n");
#endif
  while (!(application == 0 && piece == 0 && new_match == MATCH_INVALID))
    {
    /* use for debugging */
#ifdef _DEBUG_
printf("subgenr: application=%d piece=%d new_match=%d [MATCH_INVALID=%d]\n",application,piece,new_match,MATCH_INVALID);
#endif
/*
    for (i = 0; i < num_atoms; ++i)
      {
      if (Array_2d16_Get (&active_atoms, i, 0) != MATCH_INVALID)
	 {
	 printf("atom %d is active by way of bonds ", i);
	 for (j=0; j < 6; ++j)
	 if (Array_3d16_Get (&active_bonds, i, j, 0) != MATCH_INVALID)
	   printf("%d (%d, %d)\n", j, Array_3d16_Get (&active_bonds, i, j, 0),
	   Array_3d16_Get (&active_bonds, i, j, 1));
	 }
      }
*/
    Array_2d16_Put (&selector, application, piece, new_match);
    if (new_match != MATCH_INVALID)
      {
      /* Check if we have matched all the pieces for this complete match.  If
	 so, then we have some more checking to do to see what to do next.
	 Otherwise we need to match all the pieces before we can get greedy
	 and look for more matches.
      */

      if (piece == num_goal_pieces - 1)
	{
#ifdef _MIND_MEM_
  in_subgenr (81);
#endif
	/* Check to see if we can add another application to the set of
	   complete matches.
	*/

#ifdef _OBSOLETE_CODE_
	if (application < (use_cur == TRUE ? cur_possible_matches :
	    SMatch_Max (head_p))) /* NUM_APP */
	  {
	  SMatch_Transform (compound_base_p, goal_tsd_p,
	    goal_xtr_p, subgoal_tsd_p, subgoal_xtr_p, complete_match_p,
	    goal_pieces_p, goal_pattern_size, goal_num_atoms, application,
	    piece, strategic);
#else
	if (application < (use_cur == TRUE ? cur_possible_matches :
	    SMatch_Max (head_p)) /* NUM_APP */ &&
	    SMatch_Transform (compound_base_p, goal_tsd_p,
	    goal_xtr_p, subgoal_tsd_p, subgoal_xtr_p, complete_match_p,
	    goal_pieces_p, goal_pattern_size, goal_num_atoms, application,
	    piece, strategic))
	  {
#endif
          if (subgenr_interactive) SApplicationMaps_Collect (complete_match_p, NULL, NULL, NULL, application, goal_num_atoms);

	  if (!((React_Head_ReactionType_Get (head_p) == REACT_MAX_APPL_ONLY) &&
/*
	      application < max_possible_matches - 1))
*/
	      application < max_possible_noninterfering_matches - 1))
	    {
	    if (ssgensg_p != NULL)
	      {
	      sshotinfo_p = SShotInfo_Create (React_Head_NumTests_Get (head_p));
	      SShotGenSG_IthInfo_Put (ssgensg_p, 
		SShotGenSG_NumInfos_Get (ssgensg_p), sshotinfo_p);
	      SShotGenSG_NumInfos_Put (ssgensg_p, 
		SShotGenSG_NumInfos_Get (ssgensg_p) + 1);
	      }
	    else
	      sshotinfo_p = NULL;

#ifdef _MIND_MEM_
  in_subgenr (811);
#endif
	    reject = SGenerate (compound_base_p, goal_xtr_p, complete_match_p,
	      &hashcount, hashstring_p, comp_slings_p, schema, application,
/*
	      max_possible_matches, backtracked,
*/
	      max_possible_noninterfering_matches, backtracked,
	      SStrategicBond_Exists (compound_base_p, application),
	      ((use_cur == TRUE) ? cur_possible_matches : SMatch_Max (head_p)),
	      stereo_chemistry, &duplicate, sshotinfo_p);
sshot_data_ready=TRUE;
#ifdef _MIND_MEM_
  in_subgenr (812);
#endif
	    }
	  else
	    {
	    duplicate = FALSE; 
if (subgenr_interactive)
    reject = SGenerate (compound_base_p, goal_xtr_p, complete_match_p,
      &hashcount, hashstring_p, comp_slings_p, schema, application,
/*
      max_possible_matches, backtracked,
*/
      max_possible_noninterfering_matches, backtracked,
      SStrategicBond_Exists (compound_base_p, application),
      ((use_cur == TRUE) ? cur_possible_matches : SMatch_Max (head_p)),
      stereo_chemistry, &duplicate, NULL);
else
	    reject = FALSE;
sshot_data_ready=FALSE;
	    }

#ifdef _MIND_MEM_
  in_subgenr (82);
#endif
	  done = (React_Head_ReactionType_Get (head_p) == REACT_MAX_APPL_ONLY &&
	    application == max_possible_matches - 1 &&
/* Add condition to prevent yet another premature exit! */
            num_goal_pieces == 1);

	  num_duplicates += (duplicate == TRUE ? 1 : 0);

if (!reject) all_rejected = FALSE;
	  if (single_max_only == TRUE && (SApplication_IsOk (
/*
	      max_possible_matches, application, ((use_cur == TRUE) ? 
*/
	      max_possible_noninterfering_matches, application, ((use_cur == TRUE) ? 
		cur_possible_matches : SMatch_Max (head_p))) == TRUE ||
/*
	      reject == TRUE))
*/
/* substitute all_rejected, since rejection of the last compound may cause premature exit w/o maxapp */
	      all_rejected == TRUE))
	    { /* $NUM_APP */
	    use_cur = TRUE;
	    if (cur_possible_matches == 10000)
	      {
	      if (React_Head_ReactionType_Get (head_p) == REACT_MAX_APPL_ONLY)
		cur_possible_matches = 0;
	      else
		cur_possible_matches = 1;
	      max_used = TRUE;
	      }
	    else
	      if (max_used == FALSE)
		cur_possible_matches = 10000; 

	    max_appl_schema = (use_cur == TRUE ? (max_used == FALSE ? TRUE :
	      FALSE) : React_Head_ReactionType_Get (head_p) ==
	      REACT_MAX_APPL_ONLY);
	    }

          if (subgenr_interactive
&& sshot_data_ready)
            {
            SApplicationMaps_Collect (NULL, NULL, NULL, &sshot_maps, application, goal_num_atoms);
            SShotInfo_TargMaps_Put (sshotinfo_p, sshot_maps);
            SShotInfo_TMapRowCnt_Put (sshotinfo_p, application + 1);
            SShotInfo_TMapColCnt_Put (sshotinfo_p, goal_num_atoms);
          }
#ifdef _MIND_MEM_
  in_subgenr (83);
#endif

	  /* This compound was rejected by SGenerate, so back out the
	     information.
	  */

	  if (reject == TRUE)
	    {
	    SMatch_Info_Clear (complete_match_p, goal_pieces_p, &active_atoms,
	      &active_bonds, goal_num_atoms, num_atoms, application, piece);
	    SCompound_Destroy (&compound_base_p[application + 1]);
	    }
	  else
	    {
	    /* Not rejected, so try for another match for the first piece,
	       ie, try to advance "vertically".  Note that we are inside
	       an if-block which indicates that we have matched all the
	       goal pieces.
	    */

	    piece = 0;
	    application++;

/*
	    if (application >= max_possible_matches || application >=
*/
	    if (application >= max_possible_noninterfering_matches || application >=
		(use_cur == TRUE ? cur_possible_matches : SMatch_Max (head_p)))
	      { /* NUM_APP */
	      /* Not possible to advance "vertically", we have reached the
		 maximum allowable number of matches for the piece.  So
		 backtrack "horizontally"
	      */

	      application--;

	      if (backtracked == FALSE)
		{
                if (max_possible_matches == max_possible_noninterfering_matches)
		  max_possible_matches = max_possible_noninterfering_matches = application + 1;
                /* ELSE don't rule out another MAX NONINTERFERING MATCH!!! */
		backtracked = TRUE;

/*
		if (max_possible_matches + num_goal_pieces <=
		    MAX_PIECES_MATCHES || max_possible_matches <=
		    MAX_MATCHES_ONLY)
*/
		if (max_possible_noninterfering_matches + num_goal_pieces <=
		    MAX_PIECES_MATCHES || max_possible_noninterfering_matches <=
		    MAX_MATCHES_ONLY)
		  single_max_only = FALSE;
		}

	      piece = num_goal_pieces - 1;
	      SMatch_Info_Clear (complete_match_p, goal_pieces_p, &active_atoms,
		&active_bonds, goal_num_atoms, num_atoms, application, piece);
	      SCompound_Destroy (&compound_base_p[application + 1]);
	      }
	    else
	      {
	      /* We are allowed to try a new application, so create new
		 complete_match and goal_pieces arrays.
	      */

	      tmparr_p = AtmArr_Create (goal_num_atoms, ATMARR_NOBONDS_WORD,
		XTR_INVALID);
	      complete_match_p = AtmArr_Array_Get (tmparr_p);
	      Stack_PushAdd (match_stack_p, tmparr_p);
	      tmparr_p = AtmArr_Create (goal_num_atoms, ATMARR_NOBONDS_WORD,
		MATCH_INVALID);
	      goal_pieces_p = AtmArr_Array_Get (tmparr_p);
	      Stack_PushAdd (piece_stack_p, tmparr_p);
	      }
	    }                  /* End else-reject == TRUE */
	  }                    /* Enf if-application <= NUM_APP () */
	else
	  { /* NUM_APP switched to 0 or 1 */
	  new_match = MATCH_INVALID;
	  SMatch_Info_Clear (complete_match_p, goal_pieces_p, &active_atoms,
	    &active_bonds, goal_num_atoms, num_atoms, application, piece);
	  }                    /* End else-application <= NUM_APP () */
#ifdef _MIND_MEM_
  in_subgenr (84);
#endif
	}                      /* End if-piece == num_goal_pieces - 1 */
      else
	/* We haven't matched all the pieces yet, so we can advance
	   "horizontally" provided we haven't just exceeded the number of
	   applications in which case we must backtrack first.
	*/
	if (application < (use_cur == TRUE ? cur_possible_matches :
	    SMatch_Max (head_p))) /* NUM_APP */
	  piece++;
#ifdef _MIND_MEM_
  in_subgenr (85);
#endif
      }                        /* End if-new_match != MATCH_INVALID */
    else
      {
      /* The previously attempted match was invalid for some reason, so
	 decide which way to backtrack.
      */

      if (piece > 0)
	{
	/* All possible selections for this piece have been tried.  Backtrack
	   "horizontally" by trying out another match for the previous goal
	   piece.
	*/

	piece--;

	SMatch_Info_Clear (complete_match_p, goal_pieces_p, &active_atoms, 
	  &active_bonds, goal_num_atoms, num_atoms, application, piece);
	}
      else
	{
	if (application > 0)
	  {
	  /* All possible combinations of matches for goal pattern PIECE have
	     been attempted with respect to the matches already existing in
	     the  SELECTOR array.  So backtrack "vertically" by trying out
	     another match for the previous application.
	  */

	  application--;

	  if (backtracked == FALSE)
	    {
            if (max_possible_matches == max_possible_noninterfering_matches)
	      max_possible_matches = max_possible_noninterfering_matches = application + 1;
            /* ELSE don't rule out another MAX NONINTERFERING MATCH!!! */
	    backtracked = TRUE;

/*
	    if (max_possible_matches + num_goal_pieces <= MAX_PIECES_MATCHES
		|| max_possible_matches <= MAX_MATCHES_ONLY)
*/
	    if (max_possible_noninterfering_matches + num_goal_pieces <= MAX_PIECES_MATCHES
		|| max_possible_noninterfering_matches <= MAX_MATCHES_ONLY)
	      single_max_only = FALSE;
	    }

	  Stack_Pop (match_stack_p);
	  Stack_Pop (piece_stack_p);
	  tmparr_p = (AtomArray_t *) Stack_TopAdd (match_stack_p);
	  complete_match_p = AtmArr_Array_Get (tmparr_p);
	  tmparr_p = (AtomArray_t *) Stack_TopAdd (piece_stack_p);
	  goal_pieces_p = AtmArr_Array_Get (tmparr_p);

	  piece = num_goal_pieces - 1;
	  SMatch_Info_Clear (complete_match_p, goal_pieces_p, &active_atoms,
	    &active_bonds, goal_num_atoms, num_atoms, application, piece);
	  SCompound_Destroy (&compound_base_p[application + 1]);
	  }                    /* End if-application > 0 */
	}                      /* End else-piece > 0 */
      }                        /* End else-new_match != MATCH_INVALID */

    /* Setting new_match to invalid causes any remaining applications to
       be missed
    */

    if (num_duplicates > MAX_DUPLICATES)
      { /* $NUM_APP0 */
      use_cur = TRUE;
      cur_possible_matches = 0;

      new_match = MATCH_INVALID;
      max_appl_schema = (use_cur == TRUE ? (max_used == FALSE ? TRUE : FALSE) :
	React_Head_ReactionType_Get (head_p) == REACT_MAX_APPL_ONLY);
      }
    else
      {
      if (done == FALSE)
	{
	match_piece_p = &matchpiece_base_p[piece];
	SMatch_Next_Find (molecule_p, goal_xtr_p, &selector, &active_atoms,
	  &active_bonds, &goal_active_atoms, &goal_active_bonds,
	  complete_match_p, goal_pieces_p, match_piece_p, &matchcbs,
	  strategicbonds_p, &preserve_bonds, application, piece, goal_num_atoms,
	  preserve_substructures, &new_match, &strategic);
	}
      else
	new_match = MATCH_INVALID;
      }
#ifdef _MIND_MEM_
  in_subgenr (86);
#endif
    }                          /* End while-!(application ...) */
#ifdef _DEBUG_
printf("subgenr exited while loop\n");
#endif

  /* End of block to construct all possible complete match trees.
     Now to delete all the remaining allocated memory and then exit
     from the procedure.
  */

  for (i = 0; i < num_trees; i++)
    SubGenr_MatchCB_Destroy ((MatchCB_t *)Array_1dAddr_Get (&matchcbs, i));

  for (i = 1; i < max_compounds; i++)
    SCompound_Destroy (&compound_base_p[i]);

#ifdef _MIND_MEM_
  in_subgenr (9);
  mind_Array_Destroy ("SubGenr_Compound_AtomHandle_Get(compound_base_p)", "subgoalgeneration",
    SubGenr_Compound_AtomHandle_Get (compound_base_p));
  mind_free ("compound_base_p", "subgoalgeneration", compound_base_p);

  DEBUG (R_SUBGENR, DB_SUBGENR, TL_MEMORY, (outbuf,
    "Deallocated memory for Compounds array in"
    " SubGenr_Subgoals_Generate at %p", compound_base_p));

#ifdef _DEBUG_
printf("subgenr: destroying array (new code)\n");
#endif
/* Must destroy array before deallocating the array of pointers to the structure that contains it! */
  for (i=0; i < num_goal_pieces; i++)
  {
#ifdef _DEBUG_
printf("i=%d num_goal_pieces=%d\n",i,num_goal_pieces);
#endif
    match_piece_p = &matchpiece_base_p[i];
#ifdef _DEBUG_
printf("match_piece_p=%p\n",match_piece_p);
Array_Dump(&match_piece_p->tree2matchnumb,&GStdErr);
#endif
    mind_Array_Destroy ("&match_piece_p->tree2matchnumb", "subgoalgeneration", &match_piece_p->tree2matchnumb);
#ifdef _DEBUG_
printf("end i=%d\n",i);
#endif
  }
#ifdef _DEBUG_
printf("subgenr: destroyed array (new code)\n");
#endif
  mind_free ("matchpiece_base_p", "subgoalgeneration", matchpiece_base_p);

  DEBUG (R_SUBGENR, DB_SUBGENR, TL_MEMORY, (outbuf,
    "Deallocated memory for Match pieces array in"
    " SubGenr_Subgoals_Generate at %p", matchpiece_base_p));

  for (i = 0; i < num_atoms; i++)
    if (Sling_Name_Get (comp_slings_p[i]) != NULL)
      Sling_Destroy (comp_slings_p[i]);

  mind_free ("comp_slings_p", "subgoalgeneration", comp_slings_p);

  DEBUG (R_SUBGENR, DB_SUBGENR, TL_MEMORY, (outbuf,
    "Deallocated memory for Sling array in SubGenr_Subgoals_Generate at %p",
    comp_slings_p));

  for (i = 0; i < MX_SUBGENR_HASH; i++)
    if (String_Value_Get (hashstring_p[i]) != NULL)
      String_Destroy (hashstring_p[i]);

  mind_free ("hashstring_p", "subgoalgeneration", hashstring_p);

  DEBUG (R_SUBGENR, DB_SUBGENR, TL_MEMORY, (outbuf,
    "Deallocated memory for Sling hash table in"
    " SubGenr_Subgoals_Generate at %p", hashstring_p));

  Stack_Destroy (match_stack_p);
  Stack_Destroy (piece_stack_p);
  Xtr_Destroy (goal_xtr_p);
  Xtr_Destroy (subgoal_xtr_p);
  Tsd_Destroy (mole_tsd_p);

  mind_Array_Destroy ("&goal_active_atoms", "subgoalgeneration", &goal_active_atoms);
  mind_Array_Destroy ("&goal_active_bonds", "subgoalgeneration", &goal_active_bonds);
  mind_Array_Destroy ("&matchcbs", "subgoalgeneration", &matchcbs);
  mind_Array_Destroy ("&preserve_bonds", "subgoalgeneration", &preserve_bonds);
  mind_Array_Destroy ("&active_atoms", "subgoalgeneration", &active_atoms);
  mind_Array_Destroy ("&active_bonds", "subgoalgeneration", &active_bonds);
  mind_Array_Destroy ("&selector", "subgoalgeneration", &selector);
  mind_Array_Destroy ("&hashcount", "subgoalgeneration", &hashcount);
  in_subgenr (10);
#else
  Array_Destroy (SubGenr_Compound_AtomHandle_Get (compound_base_p));
  Mem_Dealloc (compound_base_p, SUBGENRCOMPOUNDSIZE * (max_compounds + 1),
    GLOBAL);

  DEBUG (R_SUBGENR, DB_SUBGENR, TL_MEMORY, (outbuf,
    "Deallocated memory for Compounds array in"
    " SubGenr_Subgoals_Generate at %p", compound_base_p));

/* Must destroy array before deallocating the array of pointers to the structure that contains it! */
  for (i=0; i < num_goal_pieces; i++)
  {
    match_piece_p = &matchpiece_base_p[i];
    Array_Destroy (&match_piece_p->tree2matchnumb);
  }
  Mem_Dealloc (matchpiece_base_p, MATCHPIECESIZE * num_goal_pieces, GLOBAL);

  DEBUG (R_SUBGENR, DB_SUBGENR, TL_MEMORY, (outbuf,
    "Deallocated memory for Match pieces array in"
    " SubGenr_Subgoals_Generate at %p", matchpiece_base_p));

  for (i = 0; i < num_atoms; i++)
    if (Sling_Name_Get (comp_slings_p[i]) != NULL)
      Sling_Destroy (comp_slings_p[i]);

  Mem_Dealloc (comp_slings_p, num_atoms * SLINGSIZE, GLOBAL);

  DEBUG (R_SUBGENR, DB_SUBGENR, TL_MEMORY, (outbuf,
    "Deallocated memory for Sling array in SubGenr_Subgoals_Generate at %p",
    comp_slings_p));

  for (i = 0; i < MX_SUBGENR_HASH; i++)
    if (String_Value_Get (hashstring_p[i]) != NULL)
      String_Destroy (hashstring_p[i]);

  Mem_Dealloc (hashstring_p, MX_SUBGENR_HASH * STRINGSIZE, GLOBAL);

  DEBUG (R_SUBGENR, DB_SUBGENR, TL_MEMORY, (outbuf,
    "Deallocated memory for Sling hash table in"
    " SubGenr_Subgoals_Generate at %p", hashstring_p));

  Stack_Destroy (match_stack_p);
  Stack_Destroy (piece_stack_p);
  Xtr_Destroy (goal_xtr_p);
  Xtr_Destroy (subgoal_xtr_p);
  Tsd_Destroy (mole_tsd_p);

  Array_Destroy (&goal_active_atoms);
  Array_Destroy (&goal_active_bonds);
  Array_Destroy (&matchcbs);
  Array_Destroy (&preserve_bonds);
  Array_Destroy (&active_atoms);
  Array_Destroy (&active_bonds);
  Array_Destroy (&selector);
  Array_Destroy (&hashcount);
#endif

#ifdef _DEBUG_
printf("leaving subgenr\n");
#endif
  DEBUG (R_SUBGENR, DB_SUBGENR, TL_PARAMS, (outbuf,
    "Leaving SubGenr_Subgoals_Generate, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SubGenr_Active_Match
*
*    This routine matches a whole pattern and considers that every bond
*    is "active".  See Krishna Agarwal's thesis for details.  The idea is
*    to perform a dual DFS search of the pattern and goal XTRs and to maintain
*    a chemically valid matching all the way down.  There are of course
*    multiple matches possible.  The algorithm will be described in the
*    relevant routines.
*
*  Used to be:
*
*    symatch:
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
*    N/A
*
******************************************************************************/
MatchCB_t *SubGenr_Active_Match
  (
  Xtr_t        *goal_p,                     /* Goal molecule handle */
  Xtr_t        *pattern_p,                  /* Pattern XTR handle */
  Xtr_t        *sgpattern_p,                /* Subgoal pattern XTR handle */
  Array_t      *stereoopt_p,                /* 1d-bit, stereo option flags */
  U16_t         goal_root,                  /* Root node in goal */
  U16_t         pattern_root,               /* Root node in pattern */
  Boolean_t     match_one                   /* Flag to match once only */
  )
{
  MatchCB_t    *mcb_p;                      /* Control block for matching */

  DEBUG (R_SUBGENR, DB_SUBGENRACTIVE, TL_PARAMS, (outbuf,
    "Entering SubGenr_Active_Match, goal addr %p, pattern addr %p, subgoal"
    " addr %p, stereo flags addr %p, goal root %u, pattern root %u,"
    " match one %hu", goal_p, pattern_p, sgpattern_p, stereoopt_p, goal_root, 
    pattern_root, match_one));

  mcb_p = SubGenr_MatchCB_Create ();
  
  MatchCB_Goal_Put (mcb_p, goal_p);
  mcb_p->pattern_p = pattern_p;
  mcb_p->sgpattern_p = sgpattern_p;
  mcb_p->stereoopt_p = stereoopt_p;
  mcb_p->goal_root = goal_root;
  mcb_p->pattern_root = pattern_root;
  mcb_p->valence_exact = TRUE;
  mcb_p->only_one = match_one;
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("MatchCB_GoalMark_Handle(mcb_p)", "subgoalgeneration{3}",
    MatchCB_GoalMark_Handle (mcb_p), Xtr_NumAtoms_Get (goal_p), BITSIZE);
#else
  Array_1d_Create (MatchCB_GoalMark_Handle (mcb_p), Xtr_NumAtoms_Get (goal_p),
    BITSIZE);
#endif
  Array_Set (MatchCB_GoalMark_Handle (mcb_p), FALSE);

  if (SMatch_Top (mcb_p) == FALSE) 
    {
     SubGenr_MatchCB_Destroy (mcb_p);
     mcb_p = NULL;
    }

  DEBUG (R_SUBGENR, DB_SUBGENRACTIVE, TL_PARAMS, (outbuf,
    "Leaving SubGenr_Active_Match, Match CB %p", mcb_p));

  return mcb_p;
}

/****************************************************************************
*
*  Function Name:                 SubGenr_Fragment_Match
*
*    This routine is used to perform a graph embedding of a fragment.  By
*    setting the GoalMark array to the fragment array the atoms flagged in
*    the fragment array are NOT matched.
*
*  Used to be:
*
*    fmatch:
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
*    N/A
*
******************************************************************************/
MatchCB_t *SubGenr_Fragment_Match
  (
  Xtr_t        *goal_p,                     /* Goal molecule handle */
  Xtr_t        *pattern_p,                  /* Pattern XTR handle */
  Array_t      *stereoopt_p,                /* 1d-bit, stereo option flags */
  Array_t      *fragment_p,                 /* 1d-bit, fragment valid flags */
  U16_t         goal_root,                  /* Root node in goal */
  U16_t         pattern_root,               /* Root node in pattern */
  Boolean_t     match_one                   /* Flag to match once only */
  )
{
  MatchCB_t    *mcb_p;                      /* Control block for matching */

  DEBUG (R_SUBGENR, DB_SUBGENRFRAGMTCH, TL_PARAMS, (outbuf,
    "Entering SubGenr_Fragment_Match, goal addr %p, pattern addr %p, stereo"
    " flags addr %p, fragment flags addr %p, goal root %u, pattern root %u,"
    " match one %hu", goal_p, pattern_p, stereoopt_p, fragment_p, goal_root,
    pattern_root, match_one));

  mcb_p = SubGenr_MatchCB_Create ();

  mcb_p->goal_p = goal_p;
  mcb_p->pattern_p = pattern_p;
  mcb_p->sgpattern_p = NULL;
  mcb_p->stereoopt_p = stereoopt_p;
  mcb_p->goal_root = goal_root;
  mcb_p->pattern_root = pattern_root;
  mcb_p->valence_exact = FALSE;
  mcb_p->only_one = match_one;
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("MatchCB_GoalMark_Handle(mcb_p)", "subgoalgeneration{4}",
    MatchCB_GoalMark_Handle(mcb_p), Xtr_NumAtoms_Get (goal_p), BITSIZE);
#else
  Array_1d_Create (MatchCB_GoalMark_Handle(mcb_p), Xtr_NumAtoms_Get (goal_p),
    BITSIZE);
#endif
  Array_CopyContents (fragment_p, MatchCB_GoalMark_Handle (mcb_p));

  if (SMatch_Top (mcb_p) != TRUE)
    {
    SubGenr_MatchCB_Destroy (mcb_p);
    mcb_p = NULL;
    }

  DEBUG (R_SUBGENR, DB_SUBGENRFRAGMTCH, TL_PARAMS, (outbuf,
    "Leaving SubGenr_Fragment_Match, Match CB = %p", mcb_p));

  return mcb_p;
}

/****************************************************************************
*
*  Function Name:                 SubGenr_MatchCB_Create
*
*    This routine allocates a MatchCB in the heap.
*
*  Used to be:
*
*    N/A:
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
*    Address of new MatchCB_t
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
MatchCB_t *SubGenr_MatchCB_Create
  (
  )
{
  MatchCB_t    *mcb_p;                      /* Result */

  DEBUG (R_SUBGENR, DB_SUBGENRCREATE, TL_PARAMS, (outbuf,
    "Entering SubGenr_MatchCB_Create"));

#ifdef _MIND_MEM_
  mind_malloc ("mcb_p", "subgoalgeneration{5}", &mcb_p, MATCHCBSIZE);
#else
  Mem_Alloc (MatchCB_t *, mcb_p, MATCHCBSIZE, GLOBAL);
#endif

  DEBUG (R_SUBGENR, DB_SUBGENRCREATE, TL_MEMORY, (outbuf,
    "Allocated a Match Control in SubGenr_MatchCB_Create at %p", mcb_p));

  if (mcb_p == NULL)
    IO_Exit_Error (R_SUBGENR, X_LIBCALL,
      "No memory for a MatchCB in SubGenr_MatchCB_Create");

  memset (mcb_p, 0, MATCHCBSIZE);

  DEBUG (R_SUBGENR, DB_SUBGENRCREATE, TL_PARAMS, (outbuf,
    "Leaving SubGenr_MatchCB_Create, address = %p", mcb_p));

  return mcb_p;
}

/****************************************************************************
*
*  Function Name:                 SubGenr_MatchCB_Destroy
*
*    This routine destroys the MatchCB except for the scratch arrays.
*
*  Used to be:
*
*    N/A:
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
*    Deallocates memory
*
******************************************************************************/
void SubGenr_MatchCB_Destroy
  (
  MatchCB_t    *mcb_p                       /* Handle of MatchCB to destroy */
  )
{
  DEBUG (R_SUBGENR, DB_SUBGENRDESTROY, TL_PARAMS, (outbuf,
    "Entering SubGenr_MatchCB_Destroy"));

/* Up until now, this structure has been freed without freeing any of the allocations within it! */
#ifdef _MIND_MEM_
  mind_Array_Destroy ("MatchCB_GoalMark_Handle(mcb_p)", "subgoalgeneration", MatchCB_GoalMark_Handle (mcb_p));
  mind_Array_Destroy ("MatchCB_PatternMark_Handle(mcb_p)", "subgoalgeneration", MatchCB_PatternMark_Handle (mcb_p));
  mind_Array_Destroy ("MatchCB_Pivot_Handle(mcb_p)", "subgoalgeneration", MatchCB_Pivot_Handle (mcb_p));
  mind_Array_Destroy ("MatchCB_PivotTry_Handle(mcb_p)", "subgoalgeneration", MatchCB_PivotTry_Handle (mcb_p));
  mind_Array_Destroy ("MatchCB_Image_Handle(mcb_p)", "subgoalgeneration", MatchCB_Image_Handle (mcb_p));
#else
  Array_Destroy (MatchCB_GoalMark_Handle (mcb_p));
  Array_Destroy (MatchCB_PatternMark_Handle (mcb_p));
  Array_Destroy (MatchCB_Pivot_Handle (mcb_p));
  Array_Destroy (MatchCB_PivotTry_Handle (mcb_p));
  Array_Destroy (MatchCB_Image_Handle (mcb_p));
#endif
  SNode_Destroy (MatchCB_Node_Get (mcb_p));
  SLeaf_Destroy (MatchCB_Leaf_Get (mcb_p));

  DEBUG_DO (DB_SUBGENRDESTROY, TL_MEMORY,
    {
    memset (mcb_p, 0, MATCHCBSIZE);
    });

#ifdef _MIND_MEM_
  mind_free ("mcb_p", "subgoalgeneration", mcb_p);
#else
  Mem_Dealloc (mcb_p, MATCHCBSIZE, GLOBAL);
#endif

  DEBUG (R_SUBGENR, DB_SUBGENRDESTROY, TL_MEMORY, (outbuf,
    "Deallocated a Match Control in SubGenr_MatchCB_Destroy at %p", mcb_p));

  DEBUG (R_SUBGENR, DB_SUBGENRDESTROY, TL_PARAMS, (outbuf,
    "Leaving SubGenr_MatchCB_Destroy, address = %p", mcb_p));

  return;
}

/****************************************************************************
*
*  Function Name:                 Duplicates_Check
*
*    This function creates a single "sling" from all the unique compounds
*    in the subgoal and concatenates them together and loads them into
*    the hash table.  It returns a Boolean_t value indicating whether the 
*    compound is a duplicate or not.
*
*  Used to be:
*
*    unload_slings:
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
*    True - this subgoal is a duplicate
*    False - this subgoal is unique (so far)
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t Duplicates_Check
  (
  Sling_t      *comp_slings_p,              /* Array of slings, filled in */
  Array_t      *hashcount_p,                /* 1d-word, hash buckets count */
  String_t     *hashstring_p,               /* Array of mega-slings for dups */
  U16_t         num_compounds,              /* # non-duplicate compounds */
  Boolean_t     stereo_chemistry            /* Use stereochemistry? */
  )
{
  String_t      string;                     /* Temp. for using Concat_c */
  Sling_t       sling;                      /* Temp. */
  U16_t         i;                          /* Counter */
  U32_t         hash;                       /* Hash value of mega-sling */
  U32_t         bucket;                     /* Where to insert in hash table */

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Entering Duplicates_Check, sorted slings %p, "
    " hash count %p, hash buckets %p, # unique %u", comp_slings_p, 
      hashcount_p, hashstring_p, num_compounds));

  /* Form a canonical subgoal sling by concatenating the sorted conjuct slings
     with a chosen separator character.  Place it in the hash table and return
     duplicate status.  This is an inlining of HASHTAB_SEARCH.
  */
  if (num_compounds == 0)
    {
/* Invalid use of String_Make - cannot use with String_Destroy!
    String_Make (string, "");
*/
#ifdef _MIND_MEM_
in_subgenr(-12345);
#endif
    string = String_Create ("", 0);
#ifdef _MIND_MEM_
in_subgenr(-12346);
#endif
    }
  else
    {
#ifdef _MIND_MEM_
in_subgenr(-12347);
#endif
    string = Sling2String (comp_slings_p[0]);
#ifdef _MIND_MEM_
in_subgenr(-12348);
#endif
    }

  /* Currently using NO separator character ??? */

  for (i = 1; i < num_compounds; i++)
    String_Concat_c (&string, (char *) Sling_Name_Get (comp_slings_p[i]));

  sling = String2Sling (string);
  hash = Pst_Hash (sling, MX_SUBGENR_HASH);
  bucket = hash;

  while (Array_1d16_Get (hashcount_p, bucket) != 0 && String_Compare (string,
	 hashstring_p[bucket]) == 0)
    {
    bucket = (bucket + 1) % MX_SUBGENR_HASH;
    if (bucket == hash)
      IO_Exit_Error (R_SUBGENR, X_SYNERR, "Hash table too small");
    }

  if (Array_1d16_Get (hashcount_p, bucket) == 0)
    {
    Array_1d16_Put (hashcount_p, bucket, 1);
    hashstring_p[bucket] = string;
    Sling_Destroy (sling);

    DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
      "Leaving Duplicates_Check, status = FALSE"));

    return FALSE;
    }
  else
    {
    Array_1d16_Put (hashcount_p, bucket,
      Array_1d16_Get (hashcount_p, bucket) + 1);
    Sling_Destroy (sling);
    String_Destroy (string);

    DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
      "Leaving Duplicates_Check, status = TRUE"));

    return TRUE;
    }
}

/****************************************************************************
*
*  Function Name:                 SAlkyl_Check
*
*    This procedure determines if neigh is an alkyl carbon of an alkyl group
*    which does not include atom.
*
*    Alkyl chains are tetra-valent carbons with at most 2 non-hydrogen
*    neighbors, usually there are other tetra-valent carbons, but the chains
*    tend to end in carbons with 3 hydrogens.  They may not end in ring
*    structures.
*
*  Used to be:
*
*    alkyl_chain:
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
*    N/A
*
******************************************************************************/
static Boolean_t SAlkyl_Check
  (
  Xtr_t        *xtr_p,                      /* XTR to check against */
  U16_t         atom,                       /* Atom to check */
  U16_t         prev,                       /* Previous atom in chain */
  MatchCB_t    *mcb_p                       /* Control block handle */
  )
{
  U16_t         atomid;                     /* Atomic element of atom */
  U16_t         next;                       /* Next interesting neighbor */
  U8_t          nonh;                       /* # non-hydrogen neighbors */
  U8_t          i;                          /* Counter */

  atomid = Xtr_Attr_Atomid_Get (xtr_p, atom);
  if (atomid != CARBON)
    return FALSE;

  if (Xtr_Attr_NumNeighbors_Get (xtr_p, atom) != 4)
    return FALSE;

  nonh = Xtr_Attr_NumNonHydrogen_Get (xtr_p, atom);
  if (nonh > 2)
    return FALSE;

  /* If only one non-hydrogen neighbor then prev must be that neighbor */

  if ((nonh == 1 && prev != XTR_INVALID) || nonh == 0)
      return TRUE;

  if (Ring_AtomIn (xtr_p, atom) == TRUE)
    return FALSE;

  for (i = 0; i < nonh; i++)
    {
    next = Xtr_Attr_NextNonHydrogen_Get (xtr_p, atom, i);
    if (next != prev && MatchCB_GoalMark_Get (mcb_p, next) == FALSE)
      if (SAlkyl_Check (xtr_p, next, atom, mcb_p) == TRUE)
	return TRUE;
    }

  return FALSE;
}

/****************************************************************************
*
*  Function Name:                 SApplication_IsOk
*
*    This function checks to see if the current application is allowed by the
*    reaction type and the current set of matches seen.
*
*  Used to be:
*
*    application_ok:
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
*    True - yes, this application is good
*    False - no, this application is not good
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static Boolean_t SApplication_IsOk
  (
  U16_t         max_possible_matches,       /* Max # matches for this schema */
  U16_t         application,                /* Application # */
  U16_t         appl_schema                 
  )
{
  if (appl_schema >= 10000) /* NUM_APP */
    if (max_possible_matches == application + 1)
      return TRUE;
    else
      return FALSE;
  else
    return TRUE;
}

/****************************************************************************
*
*  Function Name:                 SAtoms_Inactive_Set
*
*    This procedure copies all the inactive bonds in the subgoal pattern
*    also existing in the goal compound into the corresponding places in
*    the subgoal.  There is a routine which checks for "active" bonds.
*
*  Used to be:
*
*    copy_all_inactive:
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
*    N/A
*
******************************************************************************/
static void SAtoms_Inactive_Set
  (
  Tsd_t        *tmptsd_p,                   /* Before image of subgoal */
  Tsd_t        *subtsd_p,                   /* New subgoal being created */
  Tsd_t        *subgoal_tsd_p,              /* Subgoal pattern */
  Xtr_t        *goal_xtr_p,                 /* Goal pattern, XTR format */
  Xtr_t        *subgoal_xtr_p,              /* Subgoal pattern, XTR format */
  Array_t      *complete_match_p,           /* 1d-word, match map */
  U16_t         sub_atom,                   /* Atom in new subgoal */
  U16_t         goal_atom,                  /* Atom in goal pattern */
  Boolean_t     goal_done[],                /* Neighbor done flags, subgoal */
  Boolean_t     subgoal_done[],             /* Neighbor done flags, pattern */
  U16_t        *goal_count_p,               /* Subgoal neighbors done */
  U16_t        *subgoal_count_p             /* Subgoal pattern neighs done */
  )
{
  U16_t         i, j;                       /* Counters */
  U16_t         goal_neighid;               /* Neighbor in goal compound */
  U16_t         sub_neighid;                /* Neighbor in subgoal pattern */
  U16_t         image;                      /* Match image of goal atom */

  for (i = 0; i < MX_NEIGHBORS; i++)
    {
    sub_neighid = Tsd_Atom_NeighborId_Get (subgoal_tsd_p, goal_atom, i);
    if (sub_neighid != TSD_INVALID && SSubgoalBond_IsActive (goal_xtr_p,
	subgoal_xtr_p, goal_atom, sub_neighid) == FALSE && subgoal_done[i] ==
	FALSE)
      {
      image = Array_1d16_Get (complete_match_p, sub_neighid);
      for (j = 0; j < MX_NEIGHBORS; j++)
	{
	goal_neighid = Tsd_Atom_NeighborId_Get (tmptsd_p, sub_atom, j);
	if (goal_neighid == image && goal_neighid != TSD_INVALID &&
	    goal_done[j] == FALSE)
	  {
	  goal_done[j] = TRUE;
	  (*goal_count_p)++;
	  SGoal_Atom_Set (subtsd_p, tmptsd_p, sub_atom, i, j);
	  subgoal_done[i] = TRUE;
	  (*subgoal_count_p)++;
	  }
	}
      }
    }

  return;
}

/****************************************************************************
*
*  Function Name:                 SAtoms_Remaining_Set
*
*    This procedure checks for the case where an otherwise complete subgoal
*    has inactive bonds of the goal that have been changed by an earlier
*    application of the schema, so we have to copy them as-is from the
*    goal molecule.
*
*  Used to be:
*
*    copy_remaining_neighbors:
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
*    N/A
*
******************************************************************************/
static void SAtoms_Remaining_Set
  (
  Tsd_t        *tmptsd_p,                   /* Before image of subgoal */
  Tsd_t        *subtsd_p,                   /* New subgoal being created */
  Tsd_t        *subgoal_tsd_p,              /* Subgoal pattern */
  U16_t         sub_atom,                   /* Atom in new subgoal */
  U16_t         goal_atom,                  /* Atom in goal pattern */
  Boolean_t     goal_done[],                /* Neighbor done flags, subgoal */
  Boolean_t     subgoal_done[],             /* Neighbor done flags, pattern */
  U16_t        *goal_count_p,               /* Subgoal neighbors done */
  U16_t        *subgoal_count_p             /* Subgoal pattern neighs done */
  )
{
  U16_t         i, j;                       /* Counters */
  Boolean_t     done;                       /* Flag */

  for (i = 0; i < MX_NEIGHBORS; i++)
    if (goal_done[i] == FALSE)
      {
      goal_done[i] = TRUE;
      (*goal_count_p)++;
      for (j = 0, done = FALSE; j < MX_NEIGHBORS && done == FALSE; j++)
	if (subgoal_done[j] == FALSE && Tsd_Atom_NeighborId_Get (subgoal_tsd_p,
	    goal_atom, j) != TSD_INVALID)
	  {
	  subgoal_done[j] = TRUE;
	  (*subgoal_count_p)++;
	  SGoal_Atom_Set (subtsd_p, tmptsd_p, sub_atom, i, j);
	  done = TRUE;
	  }

      /* Copy atom into first free spot */

      for (j = 0; j < MX_NEIGHBORS && done == FALSE; j++)
	if (Tsd_Atom_NeighborId_Get (subtsd_p, sub_atom, j) == TSD_INVALID)
	  {
	  SGoal_Atom_Set (subtsd_p, tmptsd_p, sub_atom, i, j);
	  done = TRUE;
	  }
      }

  return;
}
/******************************************************************************
*
*  Function Name :          SCheck_Tsd_Map
*
*    This routine creates a Tsd Map for each non_trivial piece of the
*    subgoals and creates the canonical name of the xtr.
*
*  Used to be :
*    
*    check_tsd_map 
*
*  Implicit Inputs :
*
*    Used to convert xtr to tsd to get the number of atoms.  Use the number
*    of atoms in the xtr instead.
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static void SCheck_Tsd_Map 
  (
  Xtr_t                 *xtr_p, 
  Array_t               *tsdmap
  )
{
  Array_t               map;
  U16_t                 j;              
  U16_t                 num_atoms;

  num_atoms = Xtr_NumAtoms_Get (xtr_p);

  if (num_atoms == 1)
    {
    Array_Size_Put (tsdmap, 0); 
    Xtr_Name_Put (xtr_p, NULL);
    }
  else
    {
#ifdef _MIND_MEM_
in_subgenr(81110100);
#endif
    Xtr_Name_Set (xtr_p, &map);
#ifdef _MIND_MEM_
in_subgenr(81110200);
    mind_Array_2d_Create ("tsdmap", "subgoalgeneration{6}", tsdmap, num_atoms, 2, WORDSIZE);
#else
    Array_2d_Create (tsdmap, num_atoms, 2, WORDSIZE);
#endif
    for (j = 0; j < num_atoms; ++j)
      {
      Array_2d16_Put (tsdmap, j, 0, Array_1d16_Get (&map, j));
      Array_2d16_Put (tsdmap, j, 1, 0);
      }

    for (j = 0; j < num_atoms; ++j)
      if (Array_1d16_Get (&map, j) != TSD_INVALID)
	Array_2d16_Put (tsdmap, Array_1d16_Get (&map, j), 1, j);

#ifdef _MIND_MEM_
    mind_Array_Destroy ("&map", "subgoalgeneration", &map);
#else
    Array_Destroy (&map);
#endif
    }
}


/****************************************************************************
*
*  Function Name:                 SCompound_Destroy
*
*    This procedure destroys the contents of a SubGenr_Compound_t structure
*
*  Used to be:
*
*    free_produce:
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
*    N/A
*
******************************************************************************/
static void SCompound_Destroy
  (
  SubGenr_Compound_t *compound_p             /* Handle to relevant compound */
  )
{
  Tsd_Destroy (SubGenr_Compound_Tsd_Get (compound_p));
  SubGenr_Compound_Tsd_Put (compound_p, NULL);
  Xtr_Destroy (SubGenr_Compound_Xtr_Get (compound_p));
  SubGenr_Compound_Xtr_Put (compound_p, NULL);
#ifdef _MIND_MEM_
  mind_Array_Destroy ("SubGenr_Compound_AtomHandle_Get(compound_p)", "subgoalgeneration",
    SubGenr_Compound_AtomHandle_Get (compound_p));
#else
  Array_Destroy (SubGenr_Compound_AtomHandle_Get (compound_p));
#endif
  SubGenr_Compound_Strategic_Put (compound_p, FALSE);

  return;
}

/****************************************************************************
*
*  Function Name:                 SConstant_Set
*
*    This procedure sets a node in the new subgoal when it is instantiated by a
*    constant node in the goal pattern.
*
*  Used to be:
*
*    atom_is_constant:
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
*    N/A
*
******************************************************************************/
static void SConstant_Set
  (
  Tsd_t        *goal_tsd_p,                 /* Goal pattern */
  Tsd_t        *subgoal_tsd_p,              /* Subgoal pattern */
  Tsd_t        *tmptsd_p,                   /* Before image of subgoal */
  Tsd_t        *subtsd_p,                   /* New subgoal being created */
  Xtr_t        *goal_xtr_p,                 /* Goal pattern, XTR format */
  Xtr_t        *subgoal_xtr_p,              /* Subgoal pattern, XTR format */
  Array_t      *complete_match_p,           /* 1d-word, match map */
  U16_t         sub_atom,                   /* Atom in new subgoal */
  U16_t         goal_atom                   /* Atom in goal pattern */
  )
{
  U16_t         goal_count;                 /* # neighbors done */
  U16_t         subgoal_count;              /* # neighbors done */
  Boolean_t     goal_done[MX_NEIGHBORS];    /* Flag - done with neighbor */
  Boolean_t     subgoal_done[MX_NEIGHBORS]; /* Flag - done with neighbor */

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Entering SConstant_Set, goal TSD %p, subgoal TSD %p, prev subgoal %p,"
    " new subgoal %p, goal XTR %p, subgoal XTR %p, complete match %p,"
    " subgoal atom %u, goal pattern atom %u", goal_tsd_p, subgoal_tsd_p, 
    tmptsd_p, subtsd_p, goal_xtr_p, subgoal_xtr_p, complete_match_p, 
    sub_atom, goal_atom));

  FILL (goal_done, FALSE);
  FILL (subgoal_done, FALSE);
  goal_count = 0;
  subgoal_count = 0;

  Tsd_Atomid_Put (subtsd_p, sub_atom, Tsd_Atomid_Get (tmptsd_p, sub_atom));

  SEmpties_Set (tmptsd_p, subgoal_tsd_p, sub_atom, goal_atom, goal_done,
    subgoal_done, &goal_count, &subgoal_count);

  SGoal_ActiveBonds_Set (tmptsd_p, goal_tsd_p, goal_xtr_p, subgoal_xtr_p,
    complete_match_p, sub_atom, goal_atom, goal_done, &goal_count);

  SSubgoal_ActiveBonds_Set (subtsd_p, subgoal_tsd_p, goal_xtr_p, subgoal_xtr_p,
    complete_match_p, sub_atom, goal_atom, subgoal_done, &subgoal_count);

  SAtoms_Inactive_Set (tmptsd_p, subtsd_p, subgoal_tsd_p, goal_xtr_p,
    subgoal_xtr_p, complete_match_p, sub_atom, goal_atom, goal_done,
    subgoal_done, &goal_count, &subgoal_count);

  /* Check for the case that several neighbors of this subgoal atom have
     been changed.  In this case we can no longer describe the
     stereochemistry.  So the atom is marked "Don't Care".
  */

  if (goal_count < MX_NEIGHBORS)
    {
    if (goal_count < MX_NEIGHBORS - 1)
      Tsd_AtomFlags_DontCare_Put (subtsd_p, sub_atom, TRUE);

    SAtoms_Remaining_Set (tmptsd_p, subtsd_p, subgoal_tsd_p, 
      sub_atom, goal_atom, goal_done, subgoal_done, &goal_count,
      &subgoal_count);
    }

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Leaving SConstant_Set, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SEmpties_Set
*
*    This procedure marks as examined all the neighbors of the new subgoal
*    and the subgoal pattern as done if the bond direction slot has no 
*    neighbor in it.
*
*  Used to be:
*
*    mark_empties:
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
*    N/A
*
******************************************************************************/
static void SEmpties_Set
  (
  Tsd_t        *tmptsd_p,                   /* Before image of subgoal */
  Tsd_t        *subgoal_tsd_p,              /* Subgoal pattern */
  U16_t         sub_atom,                   /* Atom in new subgoal */
  U16_t         goal_atom,                  /* Atom in goal pattern */
  Boolean_t     goal_done[],                /* Neighbor done flags, subgoal */
  Boolean_t     subgoal_done[],             /* Neighbor done flags, pattern */
  U16_t        *goal_count_p,               /* Subgoal neighbors done */
  U16_t        *subgoal_count_p             /* Subgoal pattern neighs done */
  )
{
  U16_t         i;                          /* Counter */

  for (i = 0; i < MX_NEIGHBORS; i++)
    {
    if (Tsd_Atom_NeighborId_Get (tmptsd_p, sub_atom, i) == TSD_INVALID)
      {
      goal_done[i] = TRUE;
      (*goal_count_p)++;
      }

    if (Tsd_Atom_NeighborId_Get (subgoal_tsd_p, goal_atom, i) == TSD_INVALID)
      {
      subgoal_done[i] = TRUE;
      (*subgoal_count_p)++;
      }
    }

  return;
}

/****************************************************************************
*
*  Function Name:                 SGenerate
*
*    This procecure stores the compounds in compounds[application + 1].xtr,
*    ie, multiple compounds in 1(!) XTR.  They are then separated and canonical
*    Slings are generated.  They are then run through the Post-Transform Tests
*    and if they are "not chemically reasonable" then they are destroyed and
*    rejected is set TRUE.
*
*  Used to be:
*
*    storepr:
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
*    True - this application was rejected
*    False - this application was accepted
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static Boolean_t SGenerate
  (
  SubGenr_Compound_t *comp_base_p,          /* Compound vector base */
  Xtr_t        *goal_xtr_p,                 /* XTR for goal pattern */
  Array_t      *complete_match_p,           /* 1d-word, map target to goal */
  Array_t      *hashcount_p,                /* 1d-word, # in hash bucket */
  String_t     *hashstring_p,               /* Array of String_t's for table */
  Sling_t      *comp_slings_p,              /* Sling hash table */
  U32_t         schema,                     /* Schema index */
  U16_t         application,                /* Application # */
  U16_t         max_possible_matches,       /* Max # matches for this schema
    application instance */
  Boolean_t     backtracked,                /* Flag - have we backtracked yet */
  Boolean_t     strategic,                  /* Strategic bond in application */
  U16_t         appl_schema,               
  Boolean_t     stereo_chemistry,           /* Flag - use stereo chemistry */
  Boolean_t    *duplicate_p,                /* Did we find a duplicate */
  SShotInfo_t  *sshotinfo_p
  )
{
  SubGenr_Compound_t *compound_p;           /* Compound for this application */
  React_Head_t *head_p;                     /* Reaction header handle */
  Xtr_t        *comp_xtr_p;                 /* Compound XTR handle */
  Xtr_t        *canonxtr_p;                 /* canonical xtr */
  Xtr_t        *xtr_p;                      /* Subset, only marked atoms */
  Tsd_t        *tsd_p;                      /* TSD format of subset */
  List_t       *list_p;                     /* List of TSDs of molecules 
    comprising the current compound */
  ListElement_t *elem_p;                    /* List traversal ptr */
  U16_t         num_atoms;                  /* # atoms in compound */
  U16_t         num_molecules;              /* # pieces with 2+ atoms in them */
  U16_t         num_tsds;                   /* # pieces in this compound */
  U16_t         num_conjuncts;              /* # new pieces for this application */
  U16_t         num_compounds;              /* # non-duplicate 2+ molecules */
  S16_t         ease_adj;                   /* Ease metric adjustment */
  S16_t         yield_adj;                  /* Yield metric adjustment */
  S16_t         conf_adj;                   /* Confidence metric adjustment */
  U16_t         i;                          /* Counter */
  Boolean_t     reject;                     /* Flag - acceptable/working */
  Boolean_t     pass;                       /* Flag - pass tests, new merit */
  Array_t       xtrmap;                     /* 1d-word, included atoms only
    map */
  Array_t       subgmap;                    /* 2d-word, atom -> subgoal & atom
    map */
  Array_t       matchdup;                   /* 1d-word, copy complete match */
  Array_t       canonxtr;                   /* 1d-addr, canonical XTR */
  Array_t       sortcanxtr;                 /* 1d-addr, sorted canonical XTR */
  Array_t       subgxtr;                    /* 1d-addr, XTR format molecules */
  Array_t       sortxtr;                    /* 1d-addr, sorted XTRs */
  Array_t      *tsdmaps_p;                  /* Array of tsdmap arrays */
  Array_t       tmp;                        /* 1d-bit, copy of compound atoms */
  U16_t         ring_idx;                   /* Counter - ring definition */
  U16_t         num_rings;

  /* Algorithm:
     - Make copy of atom flag array from Compound
     - Copy interesting subset of XTR (check xtrmap use/def?)
     - Use SSeparate_Count to figure out how many pieces (molecules) and single
       atoms are in the currently mapped set of atoms
     - Convert list of TSDs for pieces into array of XTRs
     - Calculate # new pieces for this application
     - Do the Post-transform tests/check (return if fail)
     - Update the subgoal merit (return if fail)
     - Check that we are within the application bounds for the schema's reaction
       type
     - Check for duplicate compound generation, enter new ones in hash table
     - Check that we haven't exceeded the max. # non-identical subgoals for this
       schema
     - Call Pst_Subgoal_Insert to place the subgoal in the PST
  */

#ifdef _MIND_MEM_
  char varname[100];

in_subgenr(8110);
  reject = FALSE;
  *duplicate_p = FALSE;
  compound_p = &comp_base_p[application + 1];
  comp_xtr_p = SubGenr_Compound_Xtr_Get (compound_p);
  mind_Array_Copy ("&tmp", "subgoalgeneration{7}", SubGenr_Compound_AtomHandle_Get (compound_p), &tmp);
  mind_Array_1d_Create ("&xtrmap", "subgoalgeneration{7}", &xtrmap, Xtr_NumAtoms_Get (comp_xtr_p), WORDSIZE);
  xtr_p = Xtr_CopySubset_Atom (comp_xtr_p, &tmp, &xtrmap);
  tsd_p = Xtr2Tsd (xtr_p);

in_subgenr(81101);
  num_atoms = Tsd_NumAtoms_Get (tsd_p);
  mind_Array_2d_Create ("&subgmap", "subgoalgeneration{7}", &subgmap, num_atoms, 2, WORDSIZE);
  list_p = List_Create (LIST_NORMAL);

in_subgenr(81102);
  num_molecules = SSeparate_Count (tsd_p, list_p, &subgmap);
  num_tsds = (U16_t) List_Size_Get (list_p);
  if (subgenr_interactive) SApplicationMaps_Collect (NULL, &subgmap, list_p, NULL, application, Xtr_NumAtoms_Get (goal_xtr_p));

in_subgenr(81103);
  Xtr_Destroy (xtr_p);
  Tsd_Destroy (tsd_p);
  mind_Array_Destroy ("&xtrmap", "subgoalgeneration", &xtrmap);
  mind_Array_Destroy ("&tmp", "subgoalgeneration", &tmp);
if (subgenr_interactive && sshotinfo_p==NULL)
{
  mind_Array_Destroy("&subgmap", "subgoalgeneration", &subgmap);
  List_Destroy(list_p);
  return(FALSE);
}

  /* Hack the list so that the TSDs get destroyed properly */

  mind_Array_1d_Create ("&subxtr", "subgoalgeneration{7}", &subgxtr, num_tsds, ADDRSIZE);
  mind_Array_1d_Create ("&canonxtr", "subgoalgeneration{7}", &canonxtr, num_tsds, ADDRSIZE);
  mind_malloc ("tsdmaps_p", "subgoalgeneration{7}", &tsdmaps_p, ARRAYSIZE * num_tsds);
in_subgenr(8111);
#else
  reject = FALSE;
  *duplicate_p = FALSE;
  compound_p = &comp_base_p[application + 1];
  comp_xtr_p = SubGenr_Compound_Xtr_Get (compound_p);
  Array_Copy (SubGenr_Compound_AtomHandle_Get (compound_p), &tmp);
  Array_1d_Create (&xtrmap, Xtr_NumAtoms_Get (comp_xtr_p), WORDSIZE);
  xtr_p = Xtr_CopySubset_Atom (comp_xtr_p, &tmp, &xtrmap);
  tsd_p = Xtr2Tsd (xtr_p);

  num_atoms = Tsd_NumAtoms_Get (tsd_p);
  Array_2d_Create (&subgmap, num_atoms, 2, WORDSIZE);
  list_p = List_Create (LIST_NORMAL);

  num_molecules = SSeparate_Count (tsd_p, list_p, &subgmap);
  num_tsds = (U16_t) List_Size_Get (list_p);
  if (subgenr_interactive) SApplicationMaps_Collect (NULL, &subgmap, list_p, NULL, application, Xtr_NumAtoms_Get (goal_xtr_p));

  Xtr_Destroy (xtr_p);
  Tsd_Destroy (tsd_p);
  Array_Destroy (&xtrmap);
  Array_Destroy (&tmp);
if (subgenr_interactive && sshotinfo_p==NULL)
{
  Array_Destroy(&subgmap);
  List_Destroy(list_p);
  return(FALSE);
}

  /* Hack the list so that the TSDs get destroyed properly */

  Array_1d_Create (&subgxtr, num_tsds, ADDRSIZE);
  Array_1d_Create (&canonxtr, num_tsds, ADDRSIZE);
  Mem_Alloc (Array_t *, tsdmaps_p, ARRAYSIZE * num_tsds, GLOBAL);
#endif

  for (elem_p = List_Front_Get (list_p), i = 0; i < num_tsds; i++, elem_p =
       LstElem_Next_Get (elem_p))
    {
    /*  Create cannonical name and tsdmap for each subgoal-embedded xtr
	if the xtr has more than one atom.  It is then assumed that 
	routines will use the name in the xtr.
    */
    tsd_p = (Tsd_t *) LstElem_ValueAdd_Get (elem_p);
    xtr_p = Tsd2Xtr (tsd_p);
#ifdef _MIND_MEM_
in_subgenr(81110);
#endif

/*
Xtr_Aromat_Set (xtr_p);
*/
    Xtr_Attr_ResonantBonds_Set (xtr_p);
    num_rings = Xtr_Rings_NumRingSys_Get (xtr_p);
    for (ring_idx = 0; ring_idx < num_rings; ring_idx++)
      Xtr_Ringdef_Set (xtr_p, ring_idx);

    SCheck_Tsd_Map (xtr_p, &tsdmaps_p[i]);

    Array_1dAddr_Put (&subgxtr, i, xtr_p);

/* No longer needed - subsrch.c identifies FG's correctly despite xtr ordering **/
/* OOPS!  Not that simple!  (SCheck_Tsd_Map does the canonization AND creates
   tsdmaps_p used in Posttest_Check) */
    if (Xtr_NumAtoms_Get (xtr_p) > 1)
      {
#ifdef _MIND_MEM_
in_subgenr(81111);
#endif
      canonxtr_p = Sling_CanonicalName2Xtr (Name_Canonical_Get (
	Xtr_Name_Get (xtr_p)));
#ifdef _MIND_MEM_
in_subgenr(81112);
#endif
      Xtr_Attr_ResonantBonds_Set (canonxtr_p);
#ifdef _MIND_MEM_
in_subgenr(8111201);
#endif
      Xtr_Aromat_Set (canonxtr_p);
#ifdef _MIND_MEM_
in_subgenr(8111202);
#endif
      num_rings = Xtr_Rings_NumRingSys_Get (canonxtr_p);
#ifdef _MIND_MEM_
in_subgenr(8111203);
#endif
      for (ring_idx = 0; ring_idx < num_rings; ring_idx++)
      Xtr_Ringdef_Set (canonxtr_p, ring_idx);

#ifdef _MIND_MEM_
in_subgenr(811121);
#endif
      Array_1dAddr_Put (&canonxtr, i, canonxtr_p);
#ifdef _MIND_MEM_
in_subgenr(81113);
#endif
      }
    else
/********************************************************************************/
      Array_1dAddr_Put (&canonxtr, i, NULL);


    Tsd_Destroy (tsd_p);
    LstElem_ValueAdd_Put (elem_p, NULL);
    }

  List_Destroy (list_p);

#ifdef _MIND_MEM_
in_subgenr(8112);
  mind_Array_Copy ("&matchdup", "subgoalgeneration{7}", complete_match_p, &matchdup);
  comp_xtr_p = SubGenr_Compound_Xtr_Get (&comp_base_p[application]);
  num_conjuncts = num_molecules + 1 - SubGenr_Compound_NumCompounds_Get (
    &comp_base_p[application]);
  SubGenr_Compound_NumCompounds_Put (compound_p, num_conjuncts);

in_subgenr(81121);
  pass = Posttest_Check (SubGenr_Compound_Xtr_Get (comp_base_p), comp_xtr_p,
    goal_xtr_p, &subgxtr, &canonxtr, tsdmaps_p, &matchdup, &subgmap, num_tsds, 
    num_conjuncts, schema, &ease_adj, &yield_adj, &conf_adj, sshotinfo_p);
in_subgenr(81122);

  /*  Destroy Tsd maps.  */
  for (i = 0; i < num_tsds; i++)
    {
    sprintf (varname, "tsdmaps_p+%d", i);
    mind_Array_Destroy (varname, "subgoalgeneration", tsdmaps_p+i);
    }
  mind_free ("tsdmaps_p", "subgoalgeneration", tsdmaps_p);

  mind_Array_Destroy ("&subgmap", "subgoalgeneration", &subgmap);
  mind_Array_Destroy ("&matchdup", "subgoalgeneration", &matchdup);

  if (pass == FALSE)
    {
    if (sshotinfo_p != NULL)
      {
/* generate slings regardless of compok status
      if (SShotInfo_CompOk_Get (sshotinfo_p))
*/
	{
	mind_Array_1d_Create ("&sortxtr", "subgoalgeneration{7}", &sortxtr, num_molecules, ADDRSIZE);
	SSubgoal_Sort (&sortxtr, NULL, comp_slings_p, &subgxtr, NULL,
	  &num_compounds, num_tsds, stereo_chemistry);
	SShotInfo_Slings_Save (sshotinfo_p, comp_slings_p, num_compounds);
	mind_Array_Destroy ("&sortxtr", "subgoalgeneration", &sortxtr);
	}
      }

    for (i = 0; i < num_tsds; i++)
      {
      Xtr_Destroy ((Xtr_t *) Array_1dAddr_Get (&subgxtr, i));
      Xtr_Destroy ((Xtr_t *) Array_1dAddr_Get (&canonxtr, i));
      }

    mind_Array_Destroy ("&subgxtr", "subgoalgeneration", &subgxtr);
    mind_Array_Destroy ("&canonxtr", "subgoalgeneration", &canonxtr);

    DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
      "Leaving SGenerate, status = TRUE"));

    return TRUE;
    }

  pass = SMerit_Update (comp_base_p, schema, application, ease_adj, yield_adj,
    conf_adj, max_possible_matches, appl_schema, sshotinfo_p);

  if (pass == FALSE)
    {
    if (sshotinfo_p != NULL)
      {
      SShotInfo_MrtUpdate_Put (sshotinfo_p, FALSE);
      mind_Array_1d_Create ("&sortxtr", "subgoalgeneration{7a}", &sortxtr, num_molecules, ADDRSIZE);
      SSubgoal_Sort (&sortxtr,  NULL,comp_slings_p, &subgxtr, NULL,
	&num_compounds, num_tsds, stereo_chemistry);
      SShotInfo_Slings_Save (sshotinfo_p, comp_slings_p, num_compounds);
      mind_Array_Destroy ("&sortxtr", "subgoalgeneration", &sortxtr);
      }

    for (i = 0; i < num_tsds; i++)
      {
      Xtr_Destroy ((Xtr_t *) Array_1dAddr_Get (&subgxtr, i));
      Xtr_Destroy ((Xtr_t *) Array_1dAddr_Get (&canonxtr, i));
      }

    mind_Array_Destroy ("&subgxtr", "subgoalgeneration", &subgxtr);
    mind_Array_Destroy ("&canonxtr", "subgoalgeneration", &canonxtr);
in_subgenr(8113);
#else
  Array_Copy (complete_match_p, &matchdup);
  comp_xtr_p = SubGenr_Compound_Xtr_Get (&comp_base_p[application]);
  num_conjuncts = num_molecules + 1 - SubGenr_Compound_NumCompounds_Get (
    &comp_base_p[application]);
  SubGenr_Compound_NumCompounds_Put (compound_p, num_conjuncts);

  pass = Posttest_Check (SubGenr_Compound_Xtr_Get (comp_base_p), comp_xtr_p,
    goal_xtr_p, &subgxtr, &canonxtr, tsdmaps_p, &matchdup, &subgmap, num_tsds, 
    num_conjuncts, schema, &ease_adj, &yield_adj, &conf_adj, sshotinfo_p);

  /*  Destroy Tsd maps.  */
  for (i = 0; i < num_tsds; i++)
    Array_Destroy (&(tsdmaps_p[i]));
  Mem_Dealloc (tsdmaps_p, ARRAYSIZE * num_tsds, GLOBAL);

  Array_Destroy (&subgmap);
  Array_Destroy (&matchdup);

  if (pass == FALSE)
    {
    if (sshotinfo_p != NULL)
      {
/* generate slings regardless of compok status
      if (SShotInfo_CompOk_Get (sshotinfo_p))
*/
	{
	Array_1d_Create (&sortxtr, num_molecules, ADDRSIZE);
	SSubgoal_Sort (&sortxtr, NULL, comp_slings_p, &subgxtr, NULL,
	  &num_compounds, num_tsds, stereo_chemistry);
	SShotInfo_Slings_Save (sshotinfo_p, comp_slings_p, num_compounds);
	Array_Destroy (&sortxtr);
	}
      }

    for (i = 0; i < num_tsds; i++)
      {
      Xtr_Destroy ((Xtr_t *) Array_1dAddr_Get (&subgxtr, i));
      Xtr_Destroy ((Xtr_t *) Array_1dAddr_Get (&canonxtr, i));
      }

    Array_Destroy (&subgxtr);
    Array_Destroy (&canonxtr);

    DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
      "Leaving SGenerate, status = TRUE"));

    return TRUE;
    }

  pass = SMerit_Update (comp_base_p, schema, application, ease_adj, yield_adj,
    conf_adj, max_possible_matches, appl_schema, sshotinfo_p);

  if (pass == FALSE)
    {
    if (sshotinfo_p != NULL)
      {
      SShotInfo_MrtUpdate_Put (sshotinfo_p, FALSE);
      Array_1d_Create (&sortxtr, num_molecules, ADDRSIZE);
      SSubgoal_Sort (&sortxtr,  NULL,comp_slings_p, &subgxtr, NULL,
	&num_compounds, num_tsds, stereo_chemistry);
      SShotInfo_Slings_Save (sshotinfo_p, comp_slings_p, num_compounds);
      Array_Destroy (&sortxtr);
      }

    for (i = 0; i < num_tsds; i++)
      {
      Xtr_Destroy ((Xtr_t *) Array_1dAddr_Get (&subgxtr, i));
      Xtr_Destroy ((Xtr_t *) Array_1dAddr_Get (&canonxtr, i));
      }

    Array_Destroy (&subgxtr);
    Array_Destroy (&canonxtr);
#endif

    DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
      "Leaving SGenerate, status = TRUE"));

    return TRUE;
    }

#ifdef _MIND_MEM_
in_subgenr(81131);
#endif
  head_p = React_Head_Get (React_Schema_Handle_Get (schema));
  if (appl_schema >= 10000 && application != max_possible_matches - 1
      && (React_Head_ReactionType_Get (head_p) == REACT_MAX_APPL_ONLY ||
      (backtracked == TRUE && application != 0)))
    { /* NUM_APP */
#ifdef _MIND_MEM_
in_subgenr(8114);
    if (sshotinfo_p != NULL)
      {
      SShotInfo_NumApply_Put (sshotinfo_p, FALSE);
      mind_Array_1d_Create ("&sortxtr", "subgoalgeneration{7b}", &sortxtr, num_molecules, ADDRSIZE);
      SSubgoal_Sort (&sortxtr, NULL, comp_slings_p, &subgxtr, NULL,
	&num_compounds, num_tsds, stereo_chemistry);
      SShotInfo_Slings_Save (sshotinfo_p, comp_slings_p, num_compounds);
      mind_Array_Destroy ("&sortxtr", "subgoalgeneration", &sortxtr);
      }

    for (i = 0; i < num_tsds; i++)
      {
      Xtr_Destroy ((Xtr_t *) Array_1dAddr_Get (&subgxtr, i));
      Xtr_Destroy ((Xtr_t *) Array_1dAddr_Get (&canonxtr, i));
      }

    mind_Array_Destroy ("&subgxtr", "subgoalgeneration", &subgxtr);
    mind_Array_Destroy ("&canonxtr", "subgoalgeneration", &canonxtr);
in_subgenr(8115);
#else
    if (sshotinfo_p != NULL)
      {
      SShotInfo_NumApply_Put (sshotinfo_p, FALSE);
      Array_1d_Create (&sortxtr, num_molecules, ADDRSIZE);
      SSubgoal_Sort (&sortxtr, NULL, comp_slings_p, &subgxtr, NULL,
	&num_compounds, num_tsds, stereo_chemistry);
      SShotInfo_Slings_Save (sshotinfo_p, comp_slings_p, num_compounds);
      Array_Destroy (&sortxtr);
      }

    for (i = 0; i < num_tsds; i++)
      {
      Xtr_Destroy ((Xtr_t *) Array_1dAddr_Get (&subgxtr, i));
      Xtr_Destroy ((Xtr_t *) Array_1dAddr_Get (&canonxtr, i));
      }

    Array_Destroy (&subgxtr);
    Array_Destroy (&canonxtr);
#endif

    DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
      "Leaving SGenerate, status = FALSE"));

    return FALSE;
    }

/* Restored original placement of this code above (before Posttest_Check)
for (i=0; i<num_tsds; i++)
{
  xtr_p = (Xtr_t *) Array_1dAddr_Get (&subgxtr, i);
  if (Xtr_NumAtoms_Get (xtr_p) > 1)
    {
    canonxtr_p = Sling_CanonicalName2Xtr (Name_Canonical_Get (
	Xtr_Name_Get (xtr_p)));
    Xtr_Attr_ResonantBonds_Set (canonxtr_p);
    Xtr_Aromat_Set (canonxtr_p);
    num_rings = Xtr_Rings_NumRingSys_Get (canonxtr_p);
    for (ring_idx = 0; ring_idx < num_rings; ring_idx++)
    Xtr_Ringdef_Set (canonxtr_p, ring_idx);

    Array_1dAddr_Put (&canonxtr, i, canonxtr_p);
    }
  else
    Array_1dAddr_Put (&canonxtr, i, NULL);
}
*/

#ifdef _MIND_MEM_
in_subgenr(81160000);
  mind_Array_1d_Create ("&sortxtr", "subgoalgeneration{7c}", &sortxtr, num_molecules, ADDRSIZE);
in_subgenr(81160010);
  mind_Array_1d_Create ("&sortcanxtr", "subgoalgeneration{7}", &sortcanxtr, num_molecules, ADDRSIZE);
in_subgenr(81160100);
#else
  Array_1d_Create (&sortxtr, num_molecules, ADDRSIZE);
  Array_1d_Create (&sortcanxtr, num_molecules, ADDRSIZE);
#endif
  SSubgoal_Sort (&sortxtr, &sortcanxtr, comp_slings_p, &subgxtr, &canonxtr,
    &num_compounds, num_tsds, stereo_chemistry);
#ifdef _MIND_MEM_
in_subgenr(81160110);
#endif
  *duplicate_p = Duplicates_Check (comp_slings_p, hashcount_p, 
    hashstring_p, stereo_chemistry, num_compounds);
#ifdef _MIND_MEM_
in_subgenr(81160120);
#endif

  if (num_compounds > React_Head_MaxNonident_Get (head_p))
    {
    reject = TRUE;
    if (sshotinfo_p != NULL)
      {
      SShotInfo_MaxNonid_Put (sshotinfo_p, FALSE);
      SShotInfo_Slings_Save (sshotinfo_p, comp_slings_p, num_compounds);
      }
    }

  if (reject == FALSE)
    {
#ifdef _MIND_MEM_
in_subgenr(81161000);
#endif
    if (sshotinfo_p == NULL)
{
#ifdef _MIND_MEM_
in_subgenr(81161001);
#endif
      Pst_SubGoal_Insert (&sortxtr, &sortcanxtr, comp_slings_p, compound_p, 
	num_compounds, schema, duplicate_p, strategic);
#ifdef _MIND_MEM_
in_subgenr(81161002);
#endif
}
    else
{
#ifdef _MIND_MEM_
in_subgenr(81161003);
#endif
      SShotInfo_Slings_Save (sshotinfo_p, comp_slings_p, num_compounds);
}
#ifdef _MIND_MEM_
in_subgenr(81162);
#endif
    }

  for (i = 0; i < num_tsds; i++)
    {
    Xtr_Destroy ((Xtr_t *) Array_1dAddr_Get (&subgxtr, i));
    Xtr_Destroy ((Xtr_t *) Array_1dAddr_Get (&canonxtr, i));
    }

#ifdef _MIND_MEM_
in_subgenr(8117);
  mind_Array_Destroy ("&sortxtr", "subgoalgeneration", &sortxtr);
  mind_Array_Destroy ("&sortcanxtr", "subgoalgeneration", &sortcanxtr);
  mind_Array_Destroy ("&subgxtr", "subgoalgeneration", &subgxtr);
  mind_Array_Destroy ("&canonxtr", "subgoalgeneration", &canonxtr);
in_subgenr(8118);
#else
  Array_Destroy (&sortxtr);
  Array_Destroy (&sortcanxtr);
  Array_Destroy (&subgxtr);
  Array_Destroy (&canonxtr);
#endif

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Leaving SGenerate, status = %hu", reject));

  return reject;
}

/****************************************************************************
*
*  Function Name:                 SGoal_ActiveBonds_Set
*
*    This procedure finds all the active bonds in the goal pattern, these are
*    the ones that are matched and the bond is active.  These neighbors are
*    then marked as processed.
*
*  Used to be:
*
*    mark_active_goal_bonds:
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
*    N/A
*
******************************************************************************/
static void SGoal_ActiveBonds_Set
  (
  Tsd_t        *tmptsd_p,                   /* Before image of subgoal */
  Tsd_t        *goal_tsd_p,                 /* Goal pattern */
  Xtr_t        *goal_xtr_p,                 /* Goal pattern, XTR format */
  Xtr_t        *subgoal_xtr_p,              /* Subgoal pattern, XTR format */
  Array_t      *complete_match_p,           /* 1d-word, match map */
  U16_t         sub_atom,                   /* Atom in new subgoal */
  U16_t         goal_atom,                  /* Atom in goal pattern */
  Boolean_t    *goal_done,                  /* Neighbor done flags, subgoal */
  U16_t        *goal_count_p                /* Subgoal neighbors done */
  )
{
  U16_t         i, j;                       /* Counters */
  U16_t         goal_neighid;               /* Neighbor in goal pattern */
  U16_t         image;                      /* Match image for neighid */
  U16_t         sub_neighid;                /* Current subgoal neighbor */

  /* Don't count images that evaluate to TSD_INVALID as those were already
     counted by SEmpties_Set.
  */

  for (i = 0; i < MX_NEIGHBORS; i++)
    {
    goal_neighid = Tsd_Atom_NeighborId_Get (goal_tsd_p, goal_atom, i);
    if (goal_neighid != TSD_INVALID && SGoalBond_IsActive (goal_xtr_p,
	subgoal_xtr_p, goal_atom, goal_neighid) == TRUE)
      {
      image = Array_1d16_Get (complete_match_p, goal_neighid);
      for (j = 0; j < MX_NEIGHBORS; j++)
	{
	sub_neighid = Tsd_Atom_NeighborId_Get (tmptsd_p, sub_atom, j);
	if (sub_neighid != TSD_INVALID && sub_neighid == image &&
	    goal_done[j] == FALSE)
	  {
	  goal_done[j] = TRUE;
	  (*goal_count_p)++;
	  }
	}
      }
    }

  return;
}

/****************************************************************************
*
*  Function Name:                 SGoal_Atom_Set
*
*    This procedure sets an atom in the new subgoal, including its bond.
*    This is for the case when it is inactive and can therefore be copied
*    directly, ie the patterns don't specify that any of the information 
*    changes, or they were changed by an earlier application.
*
*  Used to be:
*
*    copy_goal_to_subgoal:
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
*    N/A
*
******************************************************************************/
static void SGoal_Atom_Set
  (
  Tsd_t        *subtsd_p,                   /* New subgoal being created */
  Tsd_t        *tmptsd_p,                   /* Before image of subgoal */
  U16_t         sub_atom,                   /* Atom in new subgoal (originally
    found 3 scope levels up! */
  U16_t         sub_neigh,                  /* Neighbor index in subgoal
    pattern */
  U16_t         goal_neigh                  /* Neighbor id goal */
  )
{

  /* More nasty, ugly, disgusting multiple scope up-level references ...
     jerks ... turkeys ...
  */

  Tsd_Atom_NeighborId_Put (subtsd_p, sub_atom, sub_neigh,
    Tsd_Atom_NeighborId_Get (tmptsd_p, sub_atom, goal_neigh));

  Tsd_Atom_NeighborBond_Put (subtsd_p, sub_atom, sub_neigh,
    Tsd_Atom_NeighborBond_Get (tmptsd_p, sub_atom, goal_neigh));

  return;
}

/****************************************************************************
*
*  Function Name:                 SGoal_Complete
*
*    This procedure takes a compound in TSD format and a goal pattern also in
*    TSD format and extends the compound with the atoms that will appear
*    courtesy of the transform schema.  This gives the full set of atoms that
*    will be rearranged by the subgoal pattern.
*
*  Used to be:
*
*    formcg:
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
*    Address of TSD that is the "complete" goal
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static Tsd_t *SGoal_Complete
  (
  Tsd_t        *curtsd_p,                   /* Current product, TSD format */
  Tsd_t        *goal_tsd_p,                 /* Goal pattern, TSD format */
  U16_t         goal_pattern_size           /* # connected atoms in pattern */
  )
{
  Tsd_t        *nexttsd_p;                  /* Temp. for updated TSD */
  U16_t         num_atoms;                  /* # atoms in updated TSD */
  U16_t         goal_atom;                  /* Atom in goal pattern */
  U16_t         i, j;                       /* Counters */

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Entering SGoal_Complete, current TSD %p, goal TSD %p,"
    " goal pattern size %u", curtsd_p, goal_tsd_p, goal_pattern_size));

  /* The purpose is to create a TSD that contains all the atoms of the
     target molecule plus all the ones that appear in the goal pattern
     because they are part of the subgoal pattern.
  */

  num_atoms = Tsd_NumAtoms_Get (curtsd_p) + Tsd_NumAtoms_Get (goal_tsd_p) -
    goal_pattern_size;

  nexttsd_p = Tsd_Create (num_atoms);

  for (i = 0; i < num_atoms; i++)
    if (i >= Tsd_NumAtoms_Get (curtsd_p))
      {
      goal_atom = goal_pattern_size + i - Tsd_NumAtoms_Get (curtsd_p);
      Tsd_Atomid_Put (nexttsd_p, i, Tsd_Atomid_Get (goal_tsd_p, goal_atom));
      Tsd_Atom_Flags_Put (nexttsd_p, i, Tsd_Atom_Flags_Get (goal_tsd_p,
	goal_atom));

      for (j = 0; j < MX_NEIGHBORS; j++)
	{
	if (Tsd_Atom_NeighborId_Get (goal_tsd_p, goal_atom, j) != TSD_INVALID)
	  {
	  Tsd_Atom_NeighborBond_Put (nexttsd_p, i, j,
	    Tsd_Atom_NeighborBond_Get (goal_tsd_p, goal_atom, j));
	  Tsd_Atom_NeighborId_Put (nexttsd_p, i, j,
	    Tsd_Atom_NeighborId_Get (goal_tsd_p, goal_atom, j) +
	    Tsd_NumAtoms_Get (curtsd_p) - goal_pattern_size);
	  }
	}
      }
    else
      {
      for (j = 0; j < MX_NEIGHBORS; j++)
	{
	Tsd_Atom_NeighborBond_Put (nexttsd_p, i, j,
	  Tsd_Atom_NeighborBond_Get (curtsd_p, i, j));
	Tsd_Atom_NeighborId_Put (nexttsd_p, i, j,
	  Tsd_Atom_NeighborId_Get (curtsd_p, i, j));
	}
      Tsd_Atomid_Put (nexttsd_p, i, Tsd_Atomid_Get (curtsd_p, i));
      Tsd_Atom_Flags_Put (nexttsd_p, i, Tsd_Atom_Flags_Get (curtsd_p, i));
      }

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Leaving SGoal_Complete, complete TSD %p", nexttsd_p));

  return nexttsd_p;
}

/****************************************************************************
*
*  Function Name:                 SGoalBond_IsActive
*
*    This procedure checks for active bonds in the goal pattern.  To be
*    active the bond must be different or it must have a different neighbor.
*
*  Used to be:
*
*    gp_activeb:
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
*    True - bond between atom and neighbor is active
*    False - bond between atom and neighbor is inactive
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static Boolean_t SGoalBond_IsActive
  (
  Xtr_t        *goal_xtr_p,                 /* Goal pattern, XTR format */
  Xtr_t        *subgoal_xtr_p,              /* Subgoal pattern, XTR format */
  U16_t         atom,                       /* Atom in goal */
  U16_t         neigh                       /* Neighbor in goal */
  )
{
  U8_t          i;                          /* Counter */
  U8_t          sub_bond;                   /* Bond in subgoal pattern */
  U8_t          goal_bond;                  /* Bond in goal pattern */

  if (neigh == TSD_INVALID)
    return FALSE;

  goal_bond = Xtr_Attr_NeighborBond_Find (goal_xtr_p, atom, neigh);
  for (i = 0; i < MX_NEIGHBORS; i++)
    {
    sub_bond = Xtr_Attr_NeighborBond_Find (subgoal_xtr_p, atom, 
	Xtr_Attr_NeighborId_Get (subgoal_xtr_p, atom, i));
    if (goal_bond == sub_bond && neigh == Xtr_Attr_NeighborId_Get (
	subgoal_xtr_p, atom, i))
      return FALSE;
    }

  return TRUE;
}

/****************************************************************************
*
*  Function Name:                 SMatch_All_Find
*
*    This function finds all the possible sets of matches for a given target
*    molecule and a given goal and subgoal pattern.  These are collected into
*    an array that is indexed by number of syntheme roots, places to start an
*    embedding and by the syntheme index itself.
*
*  Used to be:
*
*    find_all_matches:
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
*    # match control blocks (trees) found
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static U16_t SMatch_All_Find
  (
  Xtr_t        *target_xtr_p,               /* Target molecule */
  Xtr_t        *goal_xtr_p,                 /* Goal pattern */
  Xtr_t        *subgoal_xtr_p,              /* Subgoal pattern */
  Array_t      *root_nodes_p,               /* 1d-word, syntheme root nodes */
  Array_t      *trees_p,                    /* 2d-addr, match trees */
  Array_t      *stereo_p,                   /* 1d-bit, stereochemistry flags */
  U16_t        max_possible_matches         /* Max # matches */
  )
{
  MatchCB_t    *mcb_p;                      /* Match control block temp */
  U16_t         gpatom, instance;           /* Counters */
  U16_t         syntheme;                   /* FG number to play with */
  U16_t         inst_root;                  /* Target root of syntheme index */
  U16_t         num_trees;                  /* Output, # trees found */
  U16_t         num_instances;              /* # instances of root syntheme */
  Boolean_t     match_all;                  /* Flag for all roots */
  Boolean_t     match_cur;                  /* Flag for current root */

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Entering SMatch_All_Find, target molecule %p, goal pattern %p, subgoal\
 pattern %p, root nodes %p, match trees %p, stereo options %p", target_xtr_p,
    goal_xtr_p, subgoal_xtr_p, root_nodes_p, trees_p, stereo_p));

  /* Algorithm for finding all the matches goes as follows:
     - Initialize the counters and the flags
     - Loop over all the atoms in the goal pattern XTR
     - If there is a syntheme rooted there, then try to find a match for it
       with all such synthemes in the target molecule XTR
     - Store the match control blocks returned and count the total trees
  */

  if (Xtr_FuncGroups_Get (target_xtr_p) == NULL)
    Xtr_FuncGroups_Put (target_xtr_p, FuncGroups_Create (target_xtr_p));

  match_all = TRUE;
  for (gpatom = 0, match_all = TRUE, num_trees = 0; gpatom <
       Xtr_NumAtoms_Get (goal_xtr_p) && match_all == TRUE; gpatom++)
    {
    syntheme = Array_1d16_Get (root_nodes_p, gpatom);
    if (syntheme != FUNCGRP_INVALID)
      {
      num_instances = Xtr_FuncGrp_NumInstances_Get (target_xtr_p, syntheme);
      for (instance = 1, match_cur = FALSE; instance <= num_instances;
	   instance++)
	{
	/* Check that this instance is valid */

	if (Xtr_FuncGrp_Instance_IsValid (target_xtr_p, syntheme, instance) ==
	    TRUE && (max_possible_matches > 1 || (max_possible_matches == 1 && 
	    Xtr_FuncGrp_Instance_IsDiff (target_xtr_p, syntheme, instance)
	    == TRUE)))
	  {
	  inst_root = Xtr_FuncGrp_SubstructureInstance_Get (target_xtr_p,
	    syntheme, instance);
	  mcb_p = SubGenr_Active_Match (target_xtr_p, goal_xtr_p,
	    subgoal_xtr_p, stereo_p, inst_root, gpatom, FALSE);

	  if (mcb_p != NULL)
	    {
	    Array_2dAddr_Put (trees_p, inst_root, gpatom, mcb_p);
	    num_trees++;
	    match_cur = TRUE;
	    }
	  }
	}                    /* End for-instance loop */

      if (match_cur == FALSE)
	match_all = FALSE;

      }                      /* End if-syntheme block */
    }                        /* End for-gpatom loop */

  /* Should provide some debugging to indicate when a match for a particular
     root is not found.  This provides a gross check on when match fails.
  */

  if (match_all == FALSE)
/*
    SMatch_AllCB_Destroy (trees_p);
*/
    return 0;

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Leaving SMatch_All_Find, # trees found = %u", num_trees));

  return num_trees;
}

/****************************************************************************
*
*  Function Name:                 SMatch_AllCB_Destroy
*
*    This procedure destroys an array of Match control blocks.
*
*  Used to be:
*
*    freetrs:
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
*    N/A
*
******************************************************************************/
static void SMatch_AllCB_Destroy
  (
  Array_t      *mcbs_p                      /* 2d-addr, MatchCBs to destroy */
  )
{
  MatchCB_t    *mcb_p;                      /* Handle of element to destroy */
  U16_t         i, j;                       /* Counters */

  for (i = 0; i < Array_Rows_Get (mcbs_p); i++)
    for (j = 0; j < Array_Columns_Get (mcbs_p); j++)
      {
      mcb_p = (MatchCB_t *)Array_2dAddr_Get (mcbs_p, i, j);
      if (mcb_p != NULL)
	SubGenr_MatchCB_Destroy (mcb_p);
      }

  return;
}

/****************************************************************************
*
*  Function Name:                 SMatch_Bond
*
*    This procedure tests the bond (patt_root, patt_neigh) in the pattern
*    to see if it matches the bond (goal_root, goal_neigh) in the goal.
*
*  Used to be:
*
*    bond_match:
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
*    N/A
*
******************************************************************************/
static Boolean_t SMatch_Bond
  (
  U16_t         patt_root,                  /* Pattern root atom index */
  U16_t         patt_neigh,                 /* Pattern neighbor atom index */
  U16_t         goal_root,                  /* Goal root atom index */
  U16_t         goal_neigh,                 /* Goal neighbor atom index */
  MatchCB_t    *mcb_p                       /* Match control block handle */
  )
{
  U8_t          patt_bond;                  /* Bond in pattern */
  U8_t          goal_bond;                  /* Bond in goal */

  patt_bond = Xtr_Attr_NeighborBond_Find (MatchCB_Pattern_Get (mcb_p),
    patt_root, patt_neigh);
  goal_bond = Xtr_Attr_NeighborBond_Find (MatchCB_Goal_Get (mcb_p),
    goal_root, goal_neigh);

  if (patt_bond == goal_bond)
    return TRUE;
  
  if (patt_bond == BOND_VARIABLE)
    if (goal_bond == BOND_SINGLE || goal_bond == BOND_DOUBLE ||
	goal_bond == BOND_RESONANT)
      return TRUE;
    else
      return FALSE;
  else
    return FALSE;
}

/****************************************************************************
*
*  Function Name:                 SMatch_Extension_Find
*
*    This procedure determines if the isomorphic subgraphs which are being
*    built can be extended by adding patt_neigh to the pattern subgraph and
*    goal_neigh to the goal subgraph.  Note that adding patt_neigh and
*    goal_neigh necessarily implies that the bonds (patt_root, patt_neigh)
*    and (goal_root, goal_neigh) must be added to the pattern and goal
*    subgraphs respectively.  Patt_root is the father of patt_neigh and
*    goal_root is the father of goal_neigh in the DFS trees of the pattern
*    and goal respectively.  There may also be additional bonds added to
*    the subgraphs if any fronds emanate from patt_neigh or goal_neigh.
* 
*    The following conditions must be met:
*      1) The id codes for patt_neigh and goal_neigh must be equivalent.
*         This is determined by the procedure SMatch_Id.
*      2) If patt_neigh is a nonvariable and the flag valence_must_be_exact
*         is set (entry through vmatch) then patt_neigh must have exactly
*         as many bonds as tau.  Otherwise, the valence of patt_neigh must
*         not be greater than that of goal_neigh.  The procedure SMatch_Id
*         checks for this.
*      3) The types of bonds which are added to the trees must be equivalent.
*         (i.e., if (patt_neigh, patt_root) is a double bond which is added
*         to the pattern subgraph, then the bond (goal_neigh, goal_patt)
*         must be either a double bond or a variable bond).  This is
*         determined by the procedure SMatch_Bond.
*      4) If goal_neigh has any neighbors which require a particular stereo
*         orientation, then each such neighbor goal_next, must be checked to
*         insure the addition of the bond (goal_neigh, goal_next) does not
*         destroy that orientation.  The procedures which do this are given
*         below.
*      5) If goal_neigh has more than one frond emanating from it, then the
*         stereo orientation of goal_neigh is relevant (providing
*         goal_stereo_flags (goal_neigh) is set) and must be checked.  The
*         appropriate procedures are given below.
* 
*    Stereo orientation is checked by the procedures SMatch_Tetrahedral_Check
*    (patt_next, patt_neigh, goal_next, goal_neigh) and SMatch_Trigonal_Check
*    (patt_next, patt_neigh, goal_next, goal_neigh) both return TRUE if the
*    orientation of nodes patt_next and goal_next remain equivalent when
*    the bonds (patt_next, patt_neigh) and (goal_next, goal_neigh) are added
*    to their respective subgraphs.  Otherwise FALSE is returned.
*    SMatch_Trigonal_Check is used if degree of patt_neigh and goal_neigh 
*    is 3 and SMatch_Tetrahedral_Check is used if degree (patt_neigh and
*    goal_neigh is 4.  Stereo orientation is not considered for nodes of
*    degree < 3 or > 4.  If stereo chemistry is requested for such a node,
*    one of the above procedures (either degree 3 or degree 4 is always
*    considered the default case when a stereo match is required) will detect
*    the invalid request, give a message and return FALSE.
*
*  Used to be:
*
*    match_extension:
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
*    N/A
*
******************************************************************************/
static Boolean_t SMatch_Extension_Find
  (
  U16_t         patt_root,                  /* Pattern root atom index */
  U16_t         patt_neigh,                 /* Pattern neighbor atom index */
  U16_t         goal_root,                  /* Goal root atom index */
  U16_t         goal_neigh,                 /* Goal neighbor atom index */
  MatchCB_t    *mcb_p                       /* Match control block handle */
  )
{
  Xtr_t        *pattern_p;                  /* Pattern handle */
  Xtr_t        *goal_p;                     /* Goal handle */
  U16_t         patt_next;                  /* Next atom in pattern to check */
  U16_t         goal_next;                  /* Next atom in goal to check */
  U16_t         num_fronds;                 /* # stereo sensitive neighbors */
  U16_t         i;                          /* Counter */
  U8_t          patt_num_neigh;             /* Counter */
  Boolean_t     result;                     /* Output of this function */

  /* The algorithm for determining a valid extension goes as follows:
     - Check that the selected neighbors are compatible
     - Check that either both or neither are ring members
     - For each neighbor in the pattern
       - Check that bonds are compatible
  */

  if (SMatch_Id (patt_neigh, goal_neigh, mcb_p) == FALSE)
    return FALSE;

  pattern_p = MatchCB_Pattern_Get (mcb_p);
  goal_p = MatchCB_Goal_Get (mcb_p);

  if (Ring_AtomIn (pattern_p, patt_neigh) == TRUE)
    if (Ring_AtomIn (goal_p, goal_neigh) == FALSE)
      return FALSE;

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Entering SMatch_Extension_Find, match cb %p", mcb_p));

  num_fronds = 0;
  patt_num_neigh = Xtr_Attr_NumNeighbors_Get (pattern_p, patt_neigh);

  for (i = 0, result = TRUE; i < patt_num_neigh && result == TRUE; i++)
    {
    patt_next = Xtr_Attr_NeighborId_Get (pattern_p, patt_neigh, i);
    if (MatchCB_PatternMark_Get (mcb_p, patt_next) == TRUE)
      {
      goal_next = MatchCB_Image_Get (mcb_p, patt_next);
      if (SMatch_Bond (patt_next, patt_neigh, goal_next, goal_neigh, mcb_p) ==
	  FALSE)
	result = FALSE;

      if (MatchCB_StereoOption_Get (mcb_p, goal_next) == TRUE)
	if (Xtr_Attr_NumNeighbors_Get (goal_p, goal_next) == 3)
	  if (SMatch_Trigonal_Check (patt_next, patt_neigh, goal_next,
	      goal_neigh, mcb_p) == TRUE)
	    num_fronds++;
	  else
	    result = FALSE;
	else
	  if (SMatch_Tetrahedral_Check (patt_next, patt_neigh, goal_next,
	      goal_neigh, mcb_p) == TRUE)
	    num_fronds++;
	  else
	    result = FALSE;
      }
    }

  if (MatchCB_StereoOption_Get (mcb_p, goal_neigh) == TRUE &&
      num_fronds > 1 && result == TRUE)
    if (Xtr_Attr_NumNeighbors_Get (goal_p, goal_neigh) == 3)
      result = SMatch_Trigonal_Check (patt_neigh, patt_root, goal_neigh,
	goal_root, mcb_p);
    else
      result = SMatch_Tetrahedral_Check (patt_neigh, patt_root, goal_neigh,
	goal_root, mcb_p);

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Leaving SMatch_Extension_Find, status = %hu", result));

  return result;
}

/****************************************************************************
*
*  Function Name:                 SMatch_Id
*
*    This routine checks to make sure that two atom ids match.  The 
*    interesting ids are the Alkyl super-atoms and the variable nodes.
*    The even variable nodes must match hydrogens, odds can match anything.
*
*  Used to be:
*
*    id_match:
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
*    N/A
*
******************************************************************************/
static Boolean_t SMatch_Id
  (
  U16_t         patt_atom,                  /* Pattern root index */
  U16_t         goal_atom,                  /* Goal root index */
  MatchCB_t    *mcb_p                       /* Control block handle */
  )
{
  Xtr_t        *pattern_p;                  /* Pattern handle */
  Xtr_t        *goal_p;                     /* Goal handle */
  U16_t         goal_id;                    /* Next atom in goal */
  U16_t         patt_id;                    /* Next atom in pattern */

  pattern_p = MatchCB_Pattern_Get (mcb_p);
  goal_p = MatchCB_Goal_Get (mcb_p);
  patt_id = Xtr_Attr_Atomid_Get (pattern_p, patt_atom);
  goal_id = Xtr_Attr_Atomid_Get (goal_p, goal_atom);

  if (patt_id >= ATOM_START && patt_id <= ATOM_END)
    {
    if (MatchCB_ValenceExact_Get (mcb_p) == TRUE)
      {
      if (Xtr_Attr_NumNeighbors_Get (pattern_p, patt_atom) !=
	  Xtr_Attr_NumNeighbors_Get (goal_p, goal_atom))
	return FALSE;
      }
    else
      if (Xtr_Attr_NumNeighbors_Get (pattern_p, patt_atom) >
	  Xtr_Attr_NumNeighbors_Get (goal_p, goal_atom))
	return FALSE;
    }

  if (patt_id == goal_id)
    return TRUE;

  if (patt_id >= VARIABLE_START && patt_id <= VARIABLE_END)

  /*  if (patt_id % 2 == 1) */
    if ((patt_id - VARIABLE_START) % 2 == 1)
      if (goal_id != HYDROGEN)
	return TRUE;
      else
	return FALSE;
    else
      return TRUE;

  if (patt_id == GENERIC_HALOGEN)
    if (Atomid_IsHalogen (goal_id) == TRUE)
      return TRUE;
    else
      return FALSE;

  if (patt_id == GENERIC_CHALCOGEN)
    if (Atomid_IsChalcogen (goal_id) == TRUE)
      return TRUE;
    else
      return FALSE;

  if (patt_id == ALKYL)
    if (SAlkyl_Check (goal_p, goal_atom, XTR_INVALID, mcb_p) == TRUE)
      return TRUE;
    else
      return FALSE;

  return FALSE;
}

/****************************************************************************
*
*  Function Name:                 SMatch_Info_Clear
*
*    This procedure clears out all the information about the given
*    application and piece.  The information is purged from complete match, goal
*    pieces, active atoms and active bonds arrays.  This takes care of matches
*    that are found to fail due to a post-transform test.
*
*  Used to be:
*
*    clear_interfere_info:
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
*    N/A
*
******************************************************************************/
static void SMatch_Info_Clear
  (
  Array_t      *complete_match_p,           /* 1d-word, match map */
  Array_t      *goal_pieces_p,              /* 1d-word, pieces to match map */
  Array_t      *active_atoms_p,             /* 2d-word, atoms to appl+piece */
  Array_t      *active_bonds_p,             /* 3d-word, bonds to appl+piece */
  U16_t         goal_num_atoms,             /* # atoms complete goal pattern */
  U16_t         num_atoms,                  /* # atoms in target molecule */
  U16_t         application,                /* Which application */
  U16_t         piece                       /* Which piece to try to match */
  )
{
  U16_t         i, j;                       /* Counters */

  for (i = 0; i < goal_num_atoms; i++)
    if (Array_1d16_Get (goal_pieces_p, i) >= piece)
      {
      Array_1d16_Put (goal_pieces_p, i, MATCH_INVALID);
      Array_1d16_Put (complete_match_p, i, XTR_INVALID);
      }

  for (i = 0; i < num_atoms; i++)
    {
    if (Array_2d16_Get (active_atoms_p, i, APPL_IDX) == application &&
	Array_2d16_Get (active_atoms_p, i, PIECE_IDX) == piece)
      {
      Array_2d16_Put (active_atoms_p, i, APPL_IDX, MATCH_INVALID);
      Array_2d16_Put (active_atoms_p, i, PIECE_IDX, MATCH_INVALID);
      }

    for (j = 0; j < MX_NEIGHBORS; j++)
      if (Array_3d16_Get (active_bonds_p, i, j, APPL_IDX) == application &&
	  Array_3d16_Get (active_bonds_p, i, j, PIECE_IDX) == piece)
	{
	Array_3d16_Put (active_bonds_p, i, j, APPL_IDX, MATCH_INVALID);
	Array_3d16_Put (active_bonds_p, i, j, PIECE_IDX, MATCH_INVALID);
	}
    }

  return;
}

/****************************************************************************
*
*  Function Name:                 SMatch_Leaf_Create
*
*    This routine creates a new Match_Leaf in the heap.
*
*  Used to be:
*
*    N/A:
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
*    Address of new Match_Leaf_t
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
static Match_Leaf_t *SMatch_Leaf_Create
  (
  )
{
  Match_Leaf_t *leaf_p;                     /* Result */

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Entering SMatch_Leaf_Create"));

#ifdef _MIND_MEM_
  mind_malloc ("leaf_p", "subgoalgeneration{8}", &leaf_p, MATCHLEAFSIZE);
#else
  Mem_Alloc (Match_Leaf_t *, leaf_p, MATCHLEAFSIZE, GLOBAL);
#endif

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_MEMORY, (outbuf,
    "Allocated a Match Leaf in SMatch_Leaf_Create at %p", leaf_p));

  if (leaf_p == NULL)
    IO_Exit_Error (R_SUBGENR, X_LIBCALL,
      "No memory for a Match_Leaf in SMatch_Leaf_Create");

  memset (leaf_p, 0, MATCHLEAFSIZE);

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Leaving SMatch_Leaf_Create, leaf addr = %p", leaf_p));

  return leaf_p;
}

/****************************************************************************
*
*  Function Name:                 SMatch_Marker
*
*    This procedure creates the new "atoms vector" for this compound by 
*    looking at the complete match map and the previous atoms vector.
*    The atoms vector keeps track of which complete goal atoms are actually 
*    mapped by a given compound (application).
*
*  Used to be:
*
*    marker:
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
*    N/A
*
******************************************************************************/
static void SMatch_Marker
  (
  Array_t      *complete_match_p,           /* 1d-word, match map */
  SubGenr_Compound_t *comp_base_p,           /* Used to create this one */
  U16_t         application,                /* Application # */
  U16_t         goal_num_atoms,             /* # atoms in goal pattern */
  U16_t         num_atoms                   /* # atoms in this compound */
  )
{
  SubGenr_Compound_t *newcomp_p;            /* Compound we are marking */
  Xtr_t        *xtr_p;                      /* Temp. for new compound XTR */
  U16_t         prev_num_atoms;             /* # atoms in previous compound */
  U16_t         slot;                       /* Index */
  U16_t         node;                       /* Index into new XTR */
  U16_t         neigh;                      /* Neighbor of node */
  U16_t         last;                       /* Counter */
  U16_t         i, j;                       /* Counters */
  Boolean_t     found;                      /* Flag - atom seen yet */
  Array_t       marker;                     /* 1d-word, temp matching */

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Entering SMatch_Marker, complete match %p, previous compound %p,"
    " application %u, # atoms %u", complete_match_p, comp_base_p, application,
    num_atoms));

  newcomp_p = &comp_base_p[application + 1];

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("SubGenr_Compound_AtomHandle_Get(newcomp_p)", "subgoalgeneration{9}",
    SubGenr_Compound_AtomHandle_Get (newcomp_p), num_atoms, BITSIZE);
#else
  Array_1d_Create (SubGenr_Compound_AtomHandle_Get (newcomp_p), num_atoms,
    BITSIZE);
#endif
  Array_Set (SubGenr_Compound_AtomHandle_Get (newcomp_p), FALSE);
  prev_num_atoms = Xtr_NumAtoms_Get (SubGenr_Compound_Xtr_Get (
	&comp_base_p[application]));


  for (i=0; i < prev_num_atoms; ++i) 
     Array_1d1_Put (SubGenr_Compound_AtomHandle_Get (newcomp_p), i, 
	Array_1d1_Get (SubGenr_Compound_AtomHandle_Get (
		&comp_base_p[application]), i));

  /* These arrays *should* be of different sizes so this may not be a good
     idea.  The old PL/I code assumed that the new compound was always
     larger than the old one.
  */
/*
  Array_CopyContents (SubGenr_Compound_AtomHandle_Get (prevcomp_p),
    SubGenr_Compound_AtomHandle_Get (newcomp_p));
*/

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&marker", "subgoalgeneration{9}", &marker, num_atoms, WORDSIZE);
#else
  Array_1d_Create (&marker, num_atoms, WORDSIZE);
#endif
  Array_Set (&marker, XTR_INVALID);
  xtr_p = SubGenr_Compound_Xtr_Get (newcomp_p);

  /* Both of these search algorithms are O(N**2) which is not so hot.
     The first algorithm eliminates duplicates and XTR_INVALID nodes
     The second algorithm catches all the neighbors of matched nodes,
     and it does this recursively which ensures that variable-node matched
     pieces of the target compound are marked.  And these marked atoms are
     entered into the atoms vector of the currently constructed compound.
  */

  for (i = 0, slot = 0; i < goal_num_atoms; i++)
    {
    node = Array_1d16_Get (complete_match_p, i);
    if (node != XTR_INVALID)
      {
      for (j = 0, found = FALSE; j < slot && found == FALSE; j++)
	if (Array_1d16_Get (&marker, j) == node)
	  found = TRUE;

      if (found == FALSE)
	{
	Array_1d16_Put (&marker, slot, node);
	slot++;
	}
      }
    }

  for (last = 0; last < slot; last++)
    {
    node = Array_1d16_Get (&marker, last);
    SubGenr_Compound_Atom_Put (newcomp_p, node, TRUE);

    for (i = 0; i < MX_NEIGHBORS; i++)
      {
      neigh = Xtr_Attr_NeighborId_Get (xtr_p, node, i);
      if (neigh != XTR_INVALID)
	{
	for (j = 0, found = FALSE; j < slot && found == FALSE; j++)
	  if (Array_1d16_Get (&marker, j) == neigh)
	    found = TRUE;

	if (found == FALSE)
	  {
	  Array_1d16_Put (&marker, slot, neigh);
	  slot++;
	  }
	}
      }
    }

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&marker", "subgoalgeneration", &marker);
#else
  Array_Destroy (&marker);
#endif

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Leaving SMatch_Marker, status = <void>"));

  return;
}

/*****************************************************************************
*
*
*
*
*
*****************************************************************************/
static U16_t SMatch_Max
 (
 React_Head_t   *head_p
 )

{
  U16_t max_possible_matches;

  switch (React_Head_ReactionType_Get (head_p)) 
    {
  case REACT_SING_APPL_ONLY :

    max_possible_matches = 1;
    break;

  case REACT_MULT_PREF_SING :

    max_possible_matches = 10;
    break;

  case REACT_MULT_NO_PREF :

    max_possible_matches = 100; 
    break;

  case REACT_MULT_PREF_MULT :

    max_possible_matches = 1000; 
    break;

  case REACT_MAX_APPL_ONLY :

    max_possible_matches = 10000; 
    break;

  default :

    max_possible_matches = 0;
    break;
    }

  return max_possible_matches;
}

/****************************************************************************
*
*  Function Name:                 SMatch_Next_Find
*
*    This function looks for the next match index for a given piece and
*    application.  If one does not exist, then MATCH_INVALID is returned.
*
*    Not sure how it starts, but it checks for intersection of current
*    complete match with the match map for this piece, also checks for 
*    preserveable substructure bond interference, and of course active
*    bonds and active atoms, ie being matched multiple times to same node.   
*
*  Used to be:
*
*    next_match:
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
*    N/A
*
******************************************************************************/
static void SMatch_Next_Find
  (
  Xtr_t        *molecule_p,                 /* Target molecule XTR handle */
  Xtr_t        *goal_xtr_p,                 /* Goal pattern XTR format */
  Array_t      *selector_p,                 /* 2d-word, reverse of active_*_p */
  Array_t      *active_atoms_p,             /* 2d-word, match info for target */
  Array_t      *active_bonds_p,             /* 3d-word, match info for target */
  Array_t      *goal_active_atoms_p,        /* 1d-bit, active flags for goal */
  Array_t      *goal_active_bonds_p,        /* 2d-bit, active flags for goal */
  Array_t      *complete_match_p,           /* 1d-word, final match map */
  Array_t      *goal_pieces_p,              /* 1d-word, final piece map */
  Match_Piece_t *match_piece_p,             /* Match descriptor for piece */
  Array_t      *trees_p,                    /* 1d-addr, match CB vector */
  Array_t      *strategicbonds_p,           /* 1d-bit, strategic bonds */
  Array_t      *preservebonds_p,            /* 2d-bit, preserveable bonds */
  U16_t         application,                /* Which application */
  U16_t         piece,                      /* Which piece to try to match */
  U16_t         goal_num_atoms,             /* # atoms in goal pattern */
  Boolean_t     preserve_substructures,     /* Flag - maintain preservable FG */
  U16_t        *flag_p,                     /* New match flag */
  Boolean_t    *strategic_p                 /* Match includes strategic bond */
  )
{
  MatchCB_t    *mcb_p;                      /* Match CB handle */
  Match_Node_t *node_p;                     /* For traversing match tree */
  Match_Leaf_t *leaf_p;                     /* For traversing match tree */
  U16_t         num_matches;                /* # matches in this match piece */
  U16_t         match_idx;                  /* Which match of all of them */
  U16_t         tree_idx;                   /* Which match tree */
  U16_t         i, j;                       /* Counters */
  U16_t         goal_atom;                  /* Counter */
  U16_t         target_atom;                /* Match in target for goal_atom */
  U16_t         goal_neigh;                 /* Active bond neighbor in pttern */
  U16_t         target_neigh;               /* Match in target for goal_neigh */
  U16_t         start;                      /* Starting match to consider */
  U8_t          goal_bond;                  /* Counter */
  U8_t          trgt_bond;                  /* Counter */
  Boolean_t     interferes;                 /* Flag - interfering matches */
  Array_t       matchmap;                   /* Mapping from target to pattern */

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Entering SMatch_Next_Find, molecule %p, selector %p, active atoms %p,"
    " active bonds %p, goal active atoms %p, goal active bonds %p,"
    " complete match %p, goal pieces %p, match piece %p, trees %p,"
    " strategic bonds %p, preservable bonds %p, application %u, piece %u,"
    " goal # atoms %u, preserve flag %hu, output match # %p,"
    " output strategic flag %p", molecule_p, selector_p,active_atoms_p, 
    active_bonds_p, goal_active_atoms_p, goal_active_bonds_p, 
    complete_match_p, goal_pieces_p, match_piece_p, trees_p, strategicbonds_p,
    preservebonds_p, application, piece, goal_num_atoms, preserve_substructures,
    flag_p, strategic_p));

  /* Algorithm:
     - Start counting the matches at 0, they will be assigned in some order
       to the possible application/piece combinations.  The maximum is limited
       by the reaction type and a few other variables
     - Get tree and match information, just so we can construct the match
       map from the original leaves of the match tree
     - Check for active atom interfering
     - Check for active bond interfering
     - Loop over all possible matches for this piece to find a NON-interfereing
       one
     - If new match is found, update complete_match, goal_pieces, active atoms
       active bonds
  */

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&matchmap", "subgoalgeneration{10}", &matchmap, goal_num_atoms, WORDSIZE);
#else
  Array_1d_Create (&matchmap, goal_num_atoms, WORDSIZE);
#endif

  num_matches = Match_Piece_NumMatches_Get (match_piece_p);

  start = Array_2d16_Get (selector_p, application, piece);
  if (start == MATCH_INVALID)
    start = 0;
  else
    start++;

  for (i = start, interferes = TRUE; i < num_matches && interferes == TRUE;
       i++)
    {
    *flag_p = i;

    match_idx = Match_Piece_Match_Get (match_piece_p, i);
    tree_idx = Match_Piece_Tree_Get (match_piece_p, i);

    /* Create the match map for this piece so we can check it against
       previously matched bonds and atoms for interference.
    */

    mcb_p = (MatchCB_t *)Array_1dAddr_Get (trees_p, tree_idx);
    Array_Set (&matchmap, XTR_INVALID);

    /* Have inlined SELMTCH since it is short and no other call sites found */

    if (match_idx <= MatchCB_NumMatches_Get (mcb_p) - 1) {
    for (j = MatchCB_NumMatches_Get (mcb_p) - 1, leaf_p =
	 MatchCB_Leaf_Get (mcb_p); j != match_idx; j--, leaf_p =
	 Match_Leaf_Next_Get (leaf_p))
      /* Empty loop body */ ;

    node_p = Match_Leaf_Node_Get (leaf_p);
    while (node_p != NULL)
      {
      Array_1d16_Put (&matchmap, Match_Node_PatternLink_Get (node_p),
	Match_Node_GoalLink_Get (node_p));
      node_p = Match_Node_Back_Get (node_p);
      }
    }
    /* End SELMTCH */

    /* Now check if this match interferes with any previous matches made
       so far.  Assume it does not.
    */

    interferes = FALSE;
    *strategic_p = FALSE;

    for (goal_atom = 0; goal_atom < goal_num_atoms && interferes == FALSE;
	 goal_atom++)
      {
      target_atom = Array_1d16_Get (&matchmap, goal_atom);
      if (target_atom != XTR_INVALID)
	{
	/* Check if active atom interferes */

	if ((Array_1d1_Get (goal_active_atoms_p, goal_atom) == TRUE && 
	    Array_1d16_Get (complete_match_p, goal_atom) != XTR_INVALID) ||
	    Array_2d16_Get (active_atoms_p, target_atom, APPL_IDX) !=
	    MATCH_INVALID)
	  interferes = TRUE;

	/* Check if active bond interferes */

	for (goal_bond = 0; goal_bond < MX_NEIGHBORS && interferes == FALSE;
	     goal_bond++)
	  {
	  goal_neigh = Xtr_Attr_NeighborId_Get (goal_xtr_p, goal_atom,
	    goal_bond);

	  /* First check that active bond in goal doesn't interfere */

	  if (goal_neigh != XTR_INVALID ? Array_2d1_Get (goal_active_bonds_p,
	      goal_atom, goal_bond) == TRUE : FALSE)
	    {
	    target_neigh = Array_1d16_Get (&matchmap, goal_neigh);

	    /* Preservable FG in the way */

	    if (preserve_substructures == TRUE && Array_2d1_Get (
		preservebonds_p, target_atom, Xtr_Attr_NeighborIndex_Find (
		molecule_p, target_atom, target_neigh)) == TRUE)
	      {
	      interferes = TRUE;

	      DEBUG_DO (DB_CHEMISTRY, TL_MINOR,
		{
		IO_Put_Debug (R_SUBGENR,
		  "Partial match rejected due to interfering preservable bond");
		});
	      }

	    /* This bond is strategic */

	    if (Array_2d1_Get (strategicbonds_p, target_atom,
		Xtr_Attr_NeighborIndex_Find (molecule_p, target_atom,
		target_neigh)) == TRUE)
	      *strategic_p = TRUE;
	    else
	      *strategic_p = FALSE;

	    /* Previously matched active bond in target in the way */

	    for (trgt_bond = 0; trgt_bond < MX_NEIGHBORS && interferes ==
		 FALSE; trgt_bond++)
	      {
	      if (Array_3d16_Get (active_bonds_p, target_atom, trgt_bond,
		  APPL_IDX) != MATCH_INVALID && 
		 Xtr_Attr_NeighborId_Get (molecule_p, target_atom, trgt_bond) 
		   == target_neigh) 
		interferes = TRUE;
	      else
		if (Array_3d16_Get (active_bonds_p, target_neigh, trgt_bond,
		    APPL_IDX) != MATCH_INVALID && 
		   Xtr_Attr_NeighborId_Get (molecule_p,
		    target_neigh, trgt_bond) == target_atom)
		  interferes = TRUE;
	      }
	    }                     /* End if-goal_neigh != XTR_INVALID */
	  }                       /* End for-goal_bond */
	}                         /* End if-goal_atom != XTR_INVALID */
      }                           /* End for-goal_atom */
    }                             /* End for-i */

  if (interferes == TRUE)
    {
    *flag_p = MATCH_INVALID;
#ifdef _MIND_MEM_
    mind_Array_Destroy ("&matchmap", "subgoalgeneration", &matchmap);
#else
    Array_Destroy (&matchmap);
#endif

    DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
      "Leaving SMatch_Next_Find, status = <void>"));

    return;
    }

  /* There is no interference from previously activated atoms or previously
     activated bonds.  Update the complete_match, goal_pieces, active_atoms
     and active_bonds arrays.
  */

  for (goal_atom = 0; goal_atom < goal_num_atoms; goal_atom++)
    {
    target_atom = Array_1d16_Get (&matchmap, goal_atom);
    if (target_atom != XTR_INVALID)
      {
      Array_1d16_Put (complete_match_p, goal_atom, target_atom);
      Array_1d16_Put (goal_pieces_p, goal_atom, piece);

      if (Array_1d1_Get (goal_active_atoms_p, goal_atom) == TRUE)
	{
	Array_2d16_Put (active_atoms_p, target_atom, APPL_IDX, application);
	Array_2d16_Put (active_atoms_p, target_atom, PIECE_IDX, piece);
	}

      for (goal_bond = 0; goal_bond < MX_NEIGHBORS; goal_bond++)
	{
	goal_neigh = Xtr_Attr_NeighborId_Get (goal_xtr_p, goal_atom, goal_bond);
	if (goal_neigh != XTR_INVALID)
	  {
	  if (Array_2d1_Get (goal_active_bonds_p, goal_atom, goal_bond) == TRUE)
	    {
	    target_neigh = Array_1d16_Get (&matchmap, goal_neigh);
	    for (i = 0; i < MX_NEIGHBORS; i++)
	      {
	      if (Xtr_Attr_NeighborId_Get (molecule_p, target_atom, i) ==
		  target_neigh)
		{
		Array_3d16_Put (active_bonds_p, target_atom, i, APPL_IDX,
		  application);
		Array_3d16_Put (active_bonds_p, target_atom, i, PIECE_IDX,
		  piece);
		}

	      if (Xtr_Attr_NeighborId_Get (molecule_p, target_neigh, i) ==
		  target_atom)
		{
		Array_3d16_Put (active_bonds_p, target_neigh, i, APPL_IDX,
		  application);
		Array_3d16_Put (active_bonds_p, target_neigh, i, PIECE_IDX,
		  piece);
		}                
	      }
	    }
	  }              /* End if-goal_neigh != XTR_INVALID */
	}                /* End for-goal_bond */
      }                  /* End if-target_atom != XTR_INVALID */
    }                    /* End for-goal_atom */

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&matchmap", "subgoalgeneration", &matchmap);
#else
  Array_Destroy (&matchmap);
#endif

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Leaving SMatch_Next_Find, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SMatch_Node_Create
*
*    This routine allocates a Match_Node in the heap.
*
*  Used to be:
*
*    N/A:
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
*    Address of new Match_Node_t
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
static Match_Node_t *SMatch_Node_Create
  (
  )
{
  Match_Node_t *node_p;                     /* Result */
  
  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Entering SMatch_Node_Create"));

#ifdef _MIND_MEM_
  mind_malloc ("node_p", "subgoalgeneration{11}", &node_p, MATCHNODESIZE);
#else
  Mem_Alloc (Match_Node_t *, node_p, MATCHNODESIZE, GLOBAL);
#endif

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_MEMORY, (outbuf,
    "Allocated a Match Node in SMatch_Node_Create at %p", node_p));

  if (node_p == NULL)
    IO_Exit_Error (R_SUBGENR, X_LIBCALL,
      "No memory for a Match_Node in SMatch_Node_Create");

  memset (node_p, 0, MATCHNODESIZE);

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Leaving SMatch_Node_Create, node addr = %p", node_p));

  return node_p;
}

/****************************************************************************
*
*  Function Name:                 SMatch_Pattern_Find
*
*    This routine is the primary recursive routine that performs the match.
*    It looks for an unmarked (not yet in the match) neighbor of the current
*    pattern root, and the same in the goal root.  It then checks to see
*    if the goal atom to be added is a valid extension of the match.  If so,
*    then it is added to the match tree and recursion is performed if the
*    atom is a pivot, if not, then look for the next neighbor of the goal
*    root, if none, then try next pattern root neighbor, if none, then we
*    have tried all possible matches at this level so exit this recursive
*    call.
*
*  Used to be:
*
*    match_pattern:
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
*    N/A
*
******************************************************************************/
static void SMatch_Pattern_Find
  (
  MatchCB_t    *mcb_p,                      /* Match control block handle */
  U16_t         patt_root                   /* Part to match next */
  )
{
  Xtr_t        *pattern_p;                  /* Pattern XTR handle */
  Xtr_t        *sgpattern_p;                /* Subgoal pattern XTR handle */
  Xtr_t        *goal_p;                     /* Goal XTR handle */
  Match_Node_t *newnode_p;                  /* New node in match tree */
  Match_Leaf_t *newleaf_p;                  /* New leaf in match tree */
  Match_Node_t *prev_currentnode;           /* Previous current handle */
  Match_Node_t *cur_p;                      /* Temporary for current node */
  U16_t         pattern_atoms;              /* # atoms in pattern */
  U16_t         goal_root;                  /* Atom index in goal matched to
					       patt_root */
  U16_t         patt_neigh;                 /* Neighbor of patt_root */
  U16_t         goal_neigh;                 /* Neighbor of goal_root */
  U16_t         prev_depth;                 /* Depth on entry */
  U8_t          patt_num_neigh;             /* # neighbors of patt_root */
  U8_t          i, j;                       /* Counters for neighbor loops */
  U8_t          goal_num_neigh;             /* # neighbors of goal_root */
  Boolean_t     look_neigh;                 /* Flag to tell if still looking */
  Boolean_t     no_matches_found;           /* Flag - have we matched yet */
  Boolean_t     need_goal_neigh;            /* Flag-look for goal_root neigh */
  Boolean_t     const_active;               /* Flag - type of bond activity */
  Boolean_t     done;                       /* Flag - avoids goto if 1-match */
  Array_t       pivot_status;               /* 1d-bit, pivot before recursive
    call so can backtrack for multiple/alternate matches */
  Array_t       prev_pivot;                 /* 1d-bit, pivot array on entry */

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Entering SMatch_Pattern_Find, match cb %p", mcb_p));

  /* - Extract local temporaries.
     - Make scratch space for this level of the search.
     - Make backup copies of information so can perform backtracking
     - Set flags to initial values
     - Start search to match neighbors of current root in pattern XTR
     - For unmatched one, start search for neighbors of root in goal XTR
     - Check unmatched ones, good ones add to match tree and match arrays
       Try to recursively match the pattern neighbor
     - If a complete match, check if only 1 is sufficient
     - Mark success status for this node
  */

  pattern_p = MatchCB_Pattern_Get (mcb_p);
  goal_p = MatchCB_Goal_Get (mcb_p);
  pattern_atoms = Xtr_NumAtoms_Get (pattern_p);
  
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&pivot_status", "subgoalgeneration{12}", &pivot_status, pattern_atoms, BITSIZE);
  mind_Array_Copy ("&prev_pivot", "subgoalgeneration{12}", MatchCB_Pivot_Handle (mcb_p), &prev_pivot);
#else
  Array_1d_Create (&pivot_status, pattern_atoms, BITSIZE);
  Array_Copy (MatchCB_Pivot_Handle (mcb_p), &prev_pivot);
#endif

  prev_depth = MatchCB_Depth_Get (mcb_p);
  cur_p = prev_currentnode = MatchCB_CurrentNode_Get (mcb_p);
  goal_root = MatchCB_Image_Get (mcb_p, patt_root);
  patt_num_neigh = Xtr_Attr_NumNeighbors_Get (pattern_p, patt_root);
  no_matches_found = FALSE;

  for (i = 0, look_neigh = TRUE, done = FALSE; i < patt_num_neigh &&
       look_neigh == TRUE && done == FALSE; i++)
    {
    /* Take the next neighbor.  Check that it isn't in the match yet, assume
       it will work.  If we have looked at all the neighbors and haven't
       found one yet (otherwise we would have exited the outer loop) then
       this node is definitely not a pivot node.
    */

    patt_neigh = Xtr_Attr_NeighborId_Get (pattern_p, patt_root, i);
    if (MatchCB_PatternMark_Get (mcb_p, patt_neigh) == FALSE)
      {
      look_neigh = FALSE;
      MatchCB_PatternMark_Put (mcb_p, patt_neigh, TRUE);

      if (i == patt_num_neigh - 1)
	MatchCB_Pivot_Put (mcb_p, patt_root, FALSE);

      Array_CopyContents (MatchCB_Pivot_Handle (mcb_p), &pivot_status);

      /* Move current tree node forward */

      MatchCB_Depth_Put (mcb_p, MatchCB_Depth_Get (mcb_p) + 1);
      no_matches_found = TRUE;
      
      if (Match_Node_Forward_Get (cur_p) != NULL)
	cur_p = Match_Node_Forward_Get (cur_p);
      else
	{
	newnode_p = SMatch_Node_Create ();
	Match_Node_Forward_Put (cur_p, newnode_p);
	Match_Node_Back_Put (newnode_p, cur_p);
	cur_p = newnode_p;
	}

      /* Found a new pattern node, now need to find a matching neighbor in
	 the goal molecule.
      */

      goal_num_neigh = Xtr_Attr_NumNeighbors_Get (goal_p, goal_root);
      for (j = 0, need_goal_neigh = TRUE; j < goal_num_neigh &&
	   need_goal_neigh == TRUE && done == FALSE; j++)
	{
	goal_neigh = Xtr_Attr_NeighborId_Get (goal_p, goal_root, j);
	if (MatchCB_GoalMark_Get (mcb_p, goal_neigh) == FALSE)
	  if (SMatch_Extension_Find (patt_root, patt_neigh, goal_root,
	      goal_neigh, mcb_p) == TRUE)
	    {
	    MatchCB_GoalMark_Put (mcb_p, goal_neigh, TRUE);

	    /* Move current tree node down */

	    if (Match_Node_Success_Get (cur_p) == TRUE)
	      {
	      if (Match_Node_Brother_Get (cur_p) != NULL)
		cur_p = Match_Node_Brother_Get (cur_p);
	      else
		{
		newnode_p = SMatch_Node_Create ();
		Match_Node_Brother_Put (cur_p, newnode_p);
		Match_Node_Back_Put (newnode_p, Match_Node_Back_Get (cur_p));
		cur_p = newnode_p;
		}

	      /* Since we will again be moving forward in the match
		 tree, we must restore pivot status.
	      */

	      Array_CopyContents (&pivot_status, MatchCB_Pivot_Handle (mcb_p));
	      }            /* if-success */

	    /* Link patt_neigh and goal_neigh */

	    MatchCB_Image_Put (mcb_p, patt_neigh, goal_neigh);
	    Match_Node_PatternLink_Put (cur_p, patt_neigh);
	    Match_Node_GoalLink_Put (cur_p, goal_neigh);
	    Match_Node_Success_Put (cur_p, TRUE);
	    MatchCB_Pivot_Put (mcb_p, patt_neigh, MatchCB_PivotTry_Get (
	      mcb_p, patt_neigh));

	    /* Do not backtrack until all pivots are fully matched */

	    newnode_p = cur_p;
	    MatchCB_CurrentNode_Put (mcb_p, cur_p);
	    while (newnode_p != NULL && 
	      Match_Node_Success_Get (MatchCB_CurrentNode_Get (mcb_p)) == TRUE)
	      {
	      if (MatchCB_Pivot_Get (mcb_p, Match_Node_PatternLink_Get (
		  newnode_p)) == TRUE)
		{
		SMatch_Pattern_Find (mcb_p,
		  Match_Node_PatternLink_Get (newnode_p));
		}

	      newnode_p = Match_Node_Back_Get (newnode_p);
	      }

	    if (Match_Node_Success_Get (cur_p) == TRUE)
	      {
	      no_matches_found = FALSE;
	      if (MatchCB_Depth_Get (mcb_p) == MatchCB_ConnectSize_Get (mcb_p))
		{
		Array_Set (&prev_pivot, FALSE);

		/* We will not seek alternate matches until we have
		   backtracked over an active bond.
		*/

		MatchCB_AlternateMatch_Put (mcb_p, FALSE);

		DEBUG_DO (DB_SUBGENRSTATIC, TL_MAJOR,
		  {
		  IO_Put_Debug (R_SUBGENR, (const char *)"Complete match found!");
		  });

		/* A new complete match has been found, match_tree_root
		   must be updated to indicate new number of matches and
		   then leaf list must be extended to include the new leaf
		   which defines this new match.
		*/

		MatchCB_NumMatches_Put (mcb_p, MatchCB_NumMatches_Get (
		  mcb_p) + 1);
		newleaf_p = SMatch_Leaf_Create ();
		Match_Leaf_Next_Put (newleaf_p, MatchCB_Leaf_Get (mcb_p));
		Match_Leaf_Node_Put (newleaf_p, cur_p);
		MatchCB_Leaf_Put (mcb_p, newleaf_p);
		}
	      else
		Array_CopyContents (MatchCB_Pivot_Handle (mcb_p), &prev_pivot);

	      if (MatchCB_Only1Match_Get (mcb_p) == TRUE)
		{
		done = TRUE;
		continue;
		}
	      }                   /* if-success */

	    MatchCB_GoalMark_Put (mcb_p, goal_neigh, FALSE);

	    /* We have just backtracked over the bond (patt_root, patt_neigh)
	       so we must check to see if it is active (note: if we entered
	       match via an entry point other than symatch then all bonds
	       are active.)
	    */

	    sgpattern_p = MatchCB_SGPattern_Get (mcb_p);
	    if (sgpattern_p == NULL)
	      {
	      MatchCB_AlternateMatch_Put (mcb_p, TRUE);
	      }
	    else
	      if (MatchCB_AlternateMatch_Get (mcb_p) == FALSE)
		{
		const_active = (Xtr_Attr_NeighborBond_Find (sgpattern_p,
		  patt_root, patt_neigh) != Xtr_Attr_NeighborBond_Find (
		  pattern_p, patt_root, patt_neigh));

		if (MatchCB_AnyStereo_Get (mcb_p) == TRUE && const_active == 
		    FALSE)
		  const_active = (Xtr_Attr_NeighborStereo_Find (sgpattern_p,
		    patt_root, patt_neigh) != Xtr_Attr_NeighborStereo_Find (
		    pattern_p, patt_root, patt_neigh));

		MatchCB_AlternateMatch_Put (mcb_p, const_active);
		}
	    }                     /* if-SMatch_Extension_Find */

	/* Once a complete match is found, we must test to see if it is
	   dotted, we must also check to see if alternate matches are
	   being sought.

	   A dotted pattern node indicates that only one image is sought,
	   dots are put into patterns because of recognizable symmetries
	*/

	if (no_matches_found == TRUE)
	  need_goal_neigh = TRUE;
	else
	  need_goal_neigh = Xtr_AttrFlags_Dot_Get (pattern_p, patt_neigh)
	    == FALSE && MatchCB_AlternateMatch_Get (mcb_p) == TRUE;
	}                         /* for-j */

      /* Done examining this neighbor, so mark it as not matched */

      MatchCB_PatternMark_Put (mcb_p, patt_neigh, FALSE);
      }                           /* if-pattern_mark == FALSE */
    }                             /* for-i */

  /* Didn't find a neighbor of the pattern root, so it can not be pivot */

  if (look_neigh == TRUE)
    MatchCB_Pivot_Put (mcb_p, patt_root, FALSE);

  /* Reset the saved values.  If no match was found, then the current is not
     a success state.
  */

  MatchCB_CurrentNode_Put (mcb_p, prev_currentnode);
  MatchCB_Depth_Put (mcb_p, prev_depth);
  Array_CopyContents (&prev_pivot, MatchCB_Pivot_Handle (mcb_p));

  if (no_matches_found == TRUE)
    Match_Node_Success_Put (MatchCB_CurrentNode_Get (mcb_p), FALSE);

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&pivot_status", "subgoalgeneration", &pivot_status);
  mind_Array_Destroy ("&prev_pivot", "subgoalgeneration", &prev_pivot);
#else
  Array_Destroy (&pivot_status);
  Array_Destroy (&prev_pivot);
#endif

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Leaving SMatch_Pattern_Find, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SMatch_Piece_Set
*
*    This routine sets all the fields in a Match_Piece_t, including allocating
*    the vector of tree map Array_t.
*
*  Used to be:
*
*    N/A:
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
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
static void SMatch_Piece_Set
  (
  Match_Piece_t *piece_p,                   /* Piece to fill in */
  U16_t         num_matches                 /* # matches for this piece */
  )
{
  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Entering SMatch_Piece_Set"));

  Match_Piece_NumMatches_Put (piece_p, num_matches);
#ifdef _MIND_MEM_
  mind_Array_2d_Create ("&piece_p->tree2matchnumb", "subgoalgeneration{13}", &piece_p->tree2matchnumb, num_matches, 2, WORDSIZE);
#else
  Array_2d_Create (&piece_p->tree2matchnumb, num_matches, 2, WORDSIZE);
#endif

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Leaving SMatch_Piece_Set, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SMatch_Tetrahedral_Check
*
*    This procedure checks that the tetrahedral nodes patt_root and goal_root
*    still match (stereochemically) when the isomorphic DFS spanning trees 
*    are extended by adding the nodes patt_neigh and goal_neigh.  Note that
*    although patt_neigh and goal_neigh are the extension nodes, it is the
*    neighbors of patt_neigh and goal_neigh which must be checked.  Patt_root
*    is a neighbor of patt_neigh and goal_root is a neighbor of goal_neigh
*    such that goal_root = image (patt_root).
*
*    Note: parameters do not match calling order of arguments
*
*  Used to be:
*
*    tetrahedral_match:
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
*    N/A
*
******************************************************************************/
static Boolean_t SMatch_Tetrahedral_Check
  (
  U16_t         patt_root,                  /* Pattern root atom index */
  U16_t         patt_neigh,                 /* Pattern neighbor atom index */
  U16_t         goal_root,                  /* Goal root atom index */
  U16_t         goal_neigh,                 /* Goal neighbor atom index */
  MatchCB_t    *mcb_p                       /* Match control block handle */
  )
{
  Xtr_t        *pattern_p;                  /* Pattern handle */
  Xtr_t        *goal_p;                     /* Goal handle */
  U16_t         goal_next;                  /* Next atom in goal */
  U16_t         patt_next;                  /* Next atom in pattern */
  U16_t         first_neigh;                /* First neighbor found */
  U16_t         i, j, k;                    /* Counters */
  U8_t          map[4];                     /* Pattern => goal mapping */
  U8_t          goal_dir;                   /* Goal bond direction */
  U8_t          patt_dir;                   /* Pattern bond direction */
  U8_t          num_neigh;                  /* Compiler bug */
  U8_t          neigh_cnt;                  /* # neighbors checked */
  Boolean_t     done;                       /* Flag for outer loop */
  Boolean_t     result;                     /* Output value */

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Entering SMatch_Tetrahedral_Check, patt root %u, patt neigh %u, goal"
    " root %u, goal neigh %u, match cb %p", patt_root, patt_neigh, goal_root,
    goal_neigh, mcb_p));

  /* - Load temporaries
     - Loop over all neighbors of the root that are matched, except the
       neighbor being tested
     - Save the direction mapping, and count two neighbors (third is
       superfluous)
     - Find the first matching permutation.  If it is the one that
       is correct then okay, otherwise this match is invalid
  */

  pattern_p = MatchCB_Pattern_Get (mcb_p);
  goal_p = MatchCB_Goal_Get (mcb_p);
  first_neigh = XTR_INVALID;
  num_neigh = Xtr_Attr_NumNeighbors_Get (pattern_p, patt_root);

  for (i = 0, done = FALSE, neigh_cnt = 0; i < num_neigh && done == FALSE; i++)
    {
    patt_next = Xtr_Attr_NeighborId_Get (pattern_p, patt_root, i);
    if (MatchCB_PatternMark_Get (mcb_p, patt_next) == TRUE &&
	patt_next != patt_neigh)
      {
      goal_next = MatchCB_Image_Get (mcb_p, patt_next);
      patt_dir = Xtr_Attr_NeighborStereo_Get (pattern_p, patt_root, i);
      goal_dir = Xtr_Attr_NeighborStereo_Find (goal_p, goal_root, goal_next);
      neigh_cnt++;
      map[patt_dir] = goal_dir;

      /* Two bonds are enough to determine orientation */

      if (neigh_cnt > 1)
	{
	done = TRUE;
	for (j = 0; j < MAX_PERMS; j++)
	  if (SPermutations[j][patt_dir] == goal_dir &&
	      SPermutations[j][first_neigh] == map[first_neigh])
	    for (k = 0; k < 4; k++)
	      map[k] = SPermutations[j][k];
	}
      else
	first_neigh = patt_next;
      }
    }

  if (done == TRUE)
    {
    patt_dir = Xtr_Attr_NeighborStereo_Find (pattern_p, patt_root, patt_neigh);
    goal_dir = Xtr_Attr_NeighborStereo_Find (goal_p, goal_root, goal_neigh);
    if (map[patt_dir] == goal_dir)
      result = TRUE;
    else
      result = FALSE;
    }
  else
    result = TRUE;

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Leaving SMatch_Tetrahedral_Check, result = %hu", result));

  return result;
}

/****************************************************************************
*
*  Function Name:                 SMatch_Top
*
*    This routine sets up the rest of the match context and starts the
*    matching at the root nodes of the pattern and goal molecule.
*
*  Used to be:
*
*    start_match (label):
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
*    N/A
*
******************************************************************************/
static Boolean_t SMatch_Top
  (
  MatchCB_t    *mcb_p                       /* Control block for matching */
  )
{
  Xtr_t        *pattern_p;                  /* Temporary */
  Match_Node_t *tmp_p;                      /* Initial node */
  U16_t         goal_atoms;                 /* # atoms in goal */
  U16_t         pattern_atoms;              /* # atoms in pattern */
  U16_t         goal_root;                  /* Goal root index, temporary */
  U16_t         pattern_root;               /* Pattern root index, temporary */
  U16_t         i;                          /* Counter */
  U16_t         temp;                       /* Temporary */

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Entering SMatch_Top, match cb %p", mcb_p));

  /* This check that the roots of the goal and pattern XTRs are compatible
     should be done prior to the actual call.

     - Initialize the rest of the match control block.  Assume that we are
       not looking for alternate matches.  This will eventually depend on 
       whether there is a subgoal pattern and stereochemistry.
     - Discover the size of the connected of the component, when it is all
       matched we have a complete match.
  */

  goal_root = MatchCB_GoalRoot_Get (mcb_p);
  pattern_root = MatchCB_PatternRoot_Get (mcb_p);
  pattern_p = MatchCB_Pattern_Get (mcb_p);
  goal_atoms = Xtr_NumAtoms_Get (MatchCB_Goal_Get (mcb_p));
  pattern_atoms = Xtr_NumAtoms_Get (pattern_p);

  if (SMatch_Id (pattern_root, goal_root, mcb_p) != TRUE)
    {
    return FALSE;
    }

  MatchCB_Depth_Put (mcb_p, 1);
  MatchCB_AlternateMatch_Put (mcb_p, FALSE);
  MatchCB_NumMatches_Put (mcb_p, 0);
  MatchCB_ConnectSize_Put (mcb_p, Xtr_ConnectedSize_Find (pattern_p,
    pattern_root));
  MatchCB_GoalMark_Put (mcb_p, goal_root, TRUE);

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&mcb_p->pattern_markb", "subgoalgeneration{14}", &mcb_p->pattern_markb, pattern_atoms, BITSIZE);
  Array_Set (&mcb_p->pattern_markb, FALSE);
  MatchCB_PatternMark_Put (mcb_p, pattern_root, TRUE);

  mind_Array_1d_Create ("&mcb_p->pivotb", "subgoalgeneration{14}", &mcb_p->pivotb, pattern_atoms, BITSIZE);
  mind_Array_1d_Create ("&mcb_p->pot_pivotb", "subgoalgeneration{14}", &mcb_p->pot_pivotb, pattern_atoms, BITSIZE);
  mind_Array_1d_Create ("&mcb_p->imageb", "subgoalgeneration{14}", &mcb_p->imageb, pattern_atoms, WORDSIZE);
#else
  Array_1d_Create (&mcb_p->pattern_markb, pattern_atoms, BITSIZE);
  Array_Set (&mcb_p->pattern_markb, FALSE);
  MatchCB_PatternMark_Put (mcb_p, pattern_root, TRUE);

  Array_1d_Create (&mcb_p->pivotb, pattern_atoms, BITSIZE);
  Array_1d_Create (&mcb_p->pot_pivotb, pattern_atoms, BITSIZE);
  Array_1d_Create (&mcb_p->imageb, pattern_atoms, WORDSIZE);
#endif
  Array_Set (&mcb_p->imageb, XTR_INVALID);
  MatchCB_Image_Put (mcb_p, pattern_root, goal_root);

  /* Pivots can only be at atoms where there is more than one bond.
     they are places where the match can choose which way to go, or
     rather it has to consider all possible bonds.
  */

  for (i = 0; i < pattern_atoms; i++)
    {
    if (Xtr_Attr_NumNeighbors_Get (pattern_p, i) == 1)
      {
      MatchCB_PivotTry_Put (mcb_p, i, FALSE);
      }
    else
      MatchCB_PivotTry_Put (mcb_p, i, TRUE);
    }

  /* Find out whether it will ever be worth checking for stereochemistry
     in the goal molecule.
  */

  for (i = 0, temp = 0; i < goal_atoms; i++)
    temp += MatchCB_StereoOption_Get (mcb_p, i);

  MatchCB_AnyStereo_Put (mcb_p, temp >= 1 ? TRUE : FALSE);

/*
#ifdef _MIND_MEM_
  mind_Array_Copy ("MatchCB_Pivot_Handle(mcb_p)", "subgoalgeneration{14}",
    MatchCB_PivotTry_Handle (mcb_p), MatchCB_Pivot_Handle (mcb_p));
#else
  Array_Copy (MatchCB_PivotTry_Handle (mcb_p), MatchCB_Pivot_Handle (mcb_p));
#endif
*/

  /* The above creates an array where one exists, leaving orphaned memory!  Substitute the following: */

  Array_CopyContents (MatchCB_PivotTry_Handle (mcb_p), MatchCB_Pivot_Handle (mcb_p));

  /* Place the mapping between the goal and pattern roots at the head of the
     match tree.  Initialize the match tree.
  */

  tmp_p = SMatch_Node_Create ();
  Match_Node_GoalLink_Put (tmp_p, goal_root);
  Match_Node_PatternLink_Put (tmp_p, pattern_root);
  Match_Node_Success_Put (tmp_p, TRUE);

  MatchCB_CurrentNode_Put (mcb_p, tmp_p);
  MatchCB_Node_Put (mcb_p, tmp_p);

  /* Start the search for a(ll) match trees */

  SMatch_Pattern_Find (mcb_p, pattern_root);

/*
if (Match_Node_Success_Get (MatchCB_CurrentNode_Get (mcb_p)) == TRUE)
{
  Array_Dump(&mcb_p->imageb, &GStdOut);
  SMatchTree_Maps_Collect (&mcb_p->imageb, NULL, NULL, pattern_atoms);
}
*/

  /* Clean up the temporary scratch space used to generate the match trees */

#ifdef _MIND_MEM_
  mind_Array_Destroy ("MatchCB_GoalMark_Handle(mcb_p)", "subgoalgeneration", MatchCB_GoalMark_Handle (mcb_p));
  mind_Array_Destroy ("MatchCB_PatternMark_Handle(mcb_p)", "subgoalgeneration", MatchCB_PatternMark_Handle (mcb_p));
  mind_Array_Destroy ("MatchCB_Pivot_Handle(mcb_p)", "subgoalgeneration", MatchCB_Pivot_Handle (mcb_p));
  mind_Array_Destroy ("MatchCB_PivotTry_Handle(mcb_p)", "subgoalgeneration", MatchCB_PivotTry_Handle (mcb_p));
  mind_Array_Destroy ("MatchCB_Image_Handle(mcb_p)", "subgoalgeneration", MatchCB_Image_Handle (mcb_p));
#else
  Array_Destroy (MatchCB_GoalMark_Handle (mcb_p));
  Array_Destroy (MatchCB_PatternMark_Handle (mcb_p));
  Array_Destroy (MatchCB_Pivot_Handle (mcb_p));
  Array_Destroy (MatchCB_PivotTry_Handle (mcb_p));
  Array_Destroy (MatchCB_Image_Handle (mcb_p));
#endif

  if (Match_Node_Success_Get (MatchCB_CurrentNode_Get (mcb_p)) == TRUE)
    {
    DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
      "Leaving SMatch_Top, match cb = %p", mcb_p));

    return TRUE;
    }

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Leaving SMatch_Top, match cb = %p", mcb_p));

  return FALSE;
}

/****************************************************************************
*
*  Function Name:                 SMatch_Transform
*
*    This routine makes a filled in ("completed") from the current application
*    compound TSD and the goal pattern.  Then it applys the schema to generate
*    the new subgoal TSD.  The atoms used are marked by SMatch_Marker and the
*    results are all left in the compound pointed to by newcomp_p.
*
*  Used to be:
*
*    transform:
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
*    N/A
*
******************************************************************************/
/*
static void SMatch_Transform
*/
static Boolean_t SMatch_Transform
  (
  SubGenr_Compound_t *comp_base_p,          /* Compound vector base */ 
  Tsd_t        *goal_tsd_p,                 /* Goal pattern TSD format */
  Xtr_t        *goal_xtr_p,                 /* Goal pattern XTR format */
  Tsd_t        *subgoal_tsd_p,              /* Subgoal pattern TSD format */
  Xtr_t        *subgoal_xtr_p,              /* Subgoal pattern XTR format */
  Array_t      *complete_match_p,           /* 1d-word, match map so far */
  Array_t      *goal_pieces_p,              /* 1d-word, piece to match map */
  U16_t         goal_pattern_size,          /* # atoms in main goal pattern */
  U16_t         goal_num_atoms,             /* # atoms in goal pattern XTR */
  U16_t         application,                /* Application # */
  U16_t         piece,                      /* Goal pattern piece # */
  Boolean_t     strategic                   /* We matched a strategic bond */
  )
{
  SubGenr_Compound_t *newcomp_p;            /* Handle to relevant compound */
  Tsd_t        *comp_tsd_p;                 /* Compound TSD base of update */
  Tsd_t        *tmptsd_p;                   /* Temp for updated TSD */
  Tsd_t        *subtsd_p;                   /* New subgoal TSD handle */
  U16_t         num_atoms;                  /* # atoms in new compound */

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Entering SMatch_Transform, prev compound %p, goal TSD %p, subgoal TSD %p,"
    " complete match %p, goal pieces %p, goal_pattern_size %u, application %u,"
    " piece %u, strategic flag %hu", comp_base_p, goal_tsd_p, subgoal_tsd_p,
    complete_match_p, goal_pieces_p, goal_pattern_size, application, piece,
    strategic));

  /* A complete match has been formed, so perform the transform indicated by
     complete_match using SubGenr_Schema_Apply.
  */

  newcomp_p = &comp_base_p[application +1]; 

  comp_tsd_p = SubGenr_Compound_Tsd_Get (&comp_base_p[application]);
  tmptsd_p = SGoal_Complete (comp_tsd_p, goal_tsd_p, goal_pattern_size);

  subtsd_p = SSchema_Apply (comp_tsd_p, goal_tsd_p, goal_xtr_p, subgoal_tsd_p,
    subgoal_xtr_p, tmptsd_p, complete_match_p, goal_pieces_p, goal_pattern_size,
    piece);

  if (subtsd_p == NULL) return (FALSE);

  /* Note: SSchema_Apply updates complete_match to include the mappings for the
     constant part of the goal pattern.
  */

  SubGenr_Compound_Tsd_Put (newcomp_p, subtsd_p);
  SubGenr_Compound_Xtr_Put (newcomp_p, Tsd2Xtr (subtsd_p));
/*
Xtr_Aromat_Set (SubGenr_Compound_Xtr_Get (newcomp_p));
*/
  SubGenr_Compound_Strategic_Put (newcomp_p, strategic);

  num_atoms = Xtr_NumAtoms_Get (SubGenr_Compound_Xtr_Get (newcomp_p));
  SMatch_Marker (complete_match_p, comp_base_p, application, goal_num_atoms,
    num_atoms);
  Tsd_Destroy (tmptsd_p);

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Leaving SMatch_Transform, status = <void>"));

/*
  return;
*/
  return (TRUE);
}

/****************************************************************************
*
*  Function Name:                 SMatch_Tree_Setup
*
*    This routine looks like it simply converts the big 2d-trees array into
*    a 1d-array which can be more easily gone through, ie no NULL tests
*    are needed.
*
*  Used to be:
*
*    setup_matchtrees:
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
*    N/A
*
******************************************************************************/
static void SMatch_Tree_Setup
  (
  Array_t      *trees_p,                    /* 2d-addr, MatchCBs to compress */
  Array_t      *treevec_p,                  /* 1d-addr, output MatchCBs */
  Array_t      *sizevec_p                   /* 1d-word, output tree sizes */
  )
{
  MatchCB_t    *mcb_p;                      /* Temporary */
  U16_t         i, j, k;                    /* Counters */

  for (i = 0, k = 0; i < Array_Rows_Get (trees_p); i++)
    for (j = 0; j < Array_Columns_Get (trees_p); j++)
      {
      mcb_p = (MatchCB_t *)Array_2dAddr_Get (trees_p, i, j);
      if (mcb_p != NULL)
	{
	Array_1dAddr_Put (treevec_p, k, mcb_p);
	Array_1d16_Put (sizevec_p, k, MatchCB_NumMatches_Get (mcb_p));
	k++;
	}
      }

  return;
}

/****************************************************************************
*
*  Function Name:                 SMatch_Trigonal_Check
*
*    This procedure checks for cis-trans correspondence.  Patt_neigh and
*    goal_neigh are extension nodes.  Patt_root and goal_root are neighbors
*    of patt_neigh and goal_neigh respectively.  Patt_root and goal_root
*    have double bonds.
*
*  Used to be:
*
*    trigonal_match:
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
*    N/A
*
******************************************************************************/
static Boolean_t SMatch_Trigonal_Check
  (
  U16_t         patt_root,                  /* Pattern root atom index */
  U16_t         patt_neigh,                 /* Pattern neighbor atom index */
  U16_t         goal_root,                  /* Goal root atom index */
  U16_t         goal_neigh,                 /* Goal neighbor atom index */
  MatchCB_t    *mcb_p                       /* Match control block handle */
  )
{
  Xtr_t        *pattern_p;                  /* Pattern handle */
  Xtr_t        *goal_p;                     /* Goal handle */
  U16_t         goal_next;                  /* Next atom in goal */
  U16_t         patt_next;                  /* Next atom in pattern */
  U16_t         tmp;                        /* Temporary atom index */
  U16_t         pi;                         /* Goal image of pattern tmp */
  U8_t          i;                          /* Counter */
  U8_t          num_neigh;                  /* Compiler bug */
  Boolean_t     result;                     /* Output from check */

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Entering SMatch_Trigonal_Check, patt root %u, patt neigh %u, goal"
    " root %u, goal neigh %u, match cb %p", patt_root, patt_neigh, goal_root,
    goal_neigh, mcb_p));

  /* - First extract local temporaries from control block.
     - Find other atom in double bond.  If this is the neighbor or it
       is not yet valid, then nothing to really check, so return
     - Check all single bond neighbors that they have the proper stereochemistry
  */
  
  pattern_p = MatchCB_Pattern_Get (mcb_p);
  goal_p = MatchCB_Goal_Get (mcb_p);
  num_neigh = Xtr_Attr_NumNeighbors_Get (pattern_p, patt_root);
  result = TRUE;

  for (i = 0, patt_next = XTR_INVALID; i < num_neigh &&
       patt_next == XTR_INVALID; i++)
    {
    tmp = Xtr_Attr_NeighborId_Get (pattern_p, patt_root, i);
    if (SPi_Orbital_Check (pattern_p, patt_root, tmp) == TRUE)
      patt_next = tmp;
    }

  if (patt_next == XTR_INVALID)
    result = FALSE;
  else
    if (patt_next == patt_neigh)
      result = TRUE;
    else
      {
      goal_next = MatchCB_Image_Get (mcb_p, patt_next);

      /* Check all degree 1 neighbors of patt_next to be sure that cis-trans
	 relationships are preserved.
      */

      num_neigh = Xtr_Attr_NumNeighbors_Get (pattern_p, patt_next);
      for (i = 0; i < num_neigh; i++)
	{
	tmp = Xtr_Attr_NeighborId_Get (pattern_p, patt_next, i);
	if (MatchCB_PatternMark_Get (mcb_p, tmp) == TRUE &&
	    Xtr_Attr_NeighborBond_Get (pattern_p, patt_next, i) ==
	    BOND_SINGLE)
	  {
	  pi = MatchCB_Image_Get (mcb_p, tmp);
	  if (Xtr_Attr_NeighborStereo_Find (pattern_p, patt_next, tmp) ==
	      Xtr_Attr_NeighborStereo_Find (pattern_p, patt_root, patt_neigh))
	    if (Xtr_Attr_NeighborStereo_Find (goal_p, goal_next, pi) ==
		Xtr_Attr_NeighborStereo_Find (goal_p, goal_root, goal_neigh))
	      result = TRUE;
	    else
	      result = FALSE;
	  else
	    if (Xtr_Attr_NeighborStereo_Find (goal_p, goal_next, pi) !=
		Xtr_Attr_NeighborStereo_Find (goal_p, goal_root, goal_neigh))
	      result = TRUE;
	    else
	      result = FALSE;
	  }
	}
      }

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Leaving SMatch_Trigonal_Check, result = %hu", result));

  return result;
}

/****************************************************************************
*
*  Function Name:                 SMerit_Update
*
*    This function figures out the compound merit figures based on the schema
*    and the current cumulative merits for ease, yield and confidence.
*
*  Used to be:
*
*    update_merit:
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
*    True - compound is acceptable
*    False - compound is rejected
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static Boolean_t SMerit_Update
  (
  SubGenr_Compound_t *comp_base_p,           /* Compound vector base */ 
  U32_t         schema,                     /* Schema number */
  U16_t         application,                /* Application being updated */
  S16_t         ease_adj,                   /* Ease adjustment - Posttests */
  S16_t         yield_adj,                  /* Yield adjustment - Posttests */
  S16_t         conf_adj,                   /* Confidence adjustment - ditto */
  U16_t         max_possible_matches,       /* # matches for this schema */
  U16_t         appl_schema,
  SShotInfo_t  *sshotinfo_p
  )
{
  SubGenr_Compound_t *compound_p;           /* Compound being evaluated */
  React_Record_t *react_p;                  /* Reaction schema handle */
  React_Head_t *head_p;                     /* Reaction schema header handle */
  S16_t         tmp_ease;                   /* For ease calculations */
  S16_t         tmp_yield;                  /* For yield calculations */
  S16_t         tmp_conf;                   /* For confidence calculations */
  S16_t         schema_yld;                 /* For yield calculations */
  S16_t         ease;                       /* Resulting ease */
  S16_t         yield;                      /* Resulting yield */
  S16_t         conf;                       /* Resulting confidence */
  Boolean_t     pass;                       /* Flag - application is okay */

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Entering SMerit_Update, compound = %p, schema = %lu, application = %u,"
    " ease adj %d, yield adj %d, conf adj %d, max possible matches %u, appl"
    " schema %u", comp_base_p, schema, application, ease_adj, yield_adj, 
    conf_adj, max_possible_matches, appl_schema));

  compound_p = &comp_base_p[application];
  tmp_ease = SubGenr_Compound_Ease_Get (compound_p);
  tmp_yield = SubGenr_Compound_Yield_Get (compound_p);
  tmp_conf = SubGenr_Compound_Confidence_Get (compound_p);
  react_p = React_Schema_Handle_Get (schema);
  head_p = React_Head_Get (react_p);
  schema_yld = React_Head_Yield_Get (head_p);

  if (React_Head_ReactionType_Get (head_p) == REACT_MAX_APPL_ONLY)
    {
    tmp_ease = React_Head_Ease_Get (head_p) + ease_adj;
    tmp_conf = React_Head_Confidence_Get (head_p) + conf_adj;
    }

  conf = MAX (0, MIN (tmp_conf, React_Head_Confidence_Get (head_p) +
    conf_adj));

  if (appl_schema < 11)
    yield = MAX (0, MIN (MX_YIELD, (tmp_yield * (schema_yld + yield_adj)) /
      MX_YIELD));
  else
    yield = MAX (0, MIN (MX_YIELD, schema_yld + yield_adj));

  if (appl_schema < 11)
    ease = tmp_ease + ease_adj - EASE_PREF_SINGLE_FACTOR * application;
  else
    if (appl_schema < 101)
      ease = tmp_ease + ease_adj;
  else
    /* Looking for at least half the possible applications */
    if (appl_schema < 1001)
      ease = tmp_ease + ease_adj + EASE_PREF_MULTIPLE_FACTOR *
	(2 * (application + 1) - max_possible_matches);
  else
    /* Ease can't be raised above its schema listed maximum */

    ease = MIN (tmp_ease, React_Head_Ease_Get (head_p) + ease_adj);

  ease = MAX (0, MIN (MX_EASE, ease));

  if (ease == 0 || yield == 0 || conf == 0)
    pass = FALSE;
  else
    pass = TRUE;

if (subgenr_interactive)
{
  SShotInfo_MeritEA_Put (sshotinfo_p, ease - React_Head_Ease_Get (head_p) - ease_adj);
  SShotInfo_MeritYA_Put (sshotinfo_p, yield - schema_yld - yield_adj);
  SShotInfo_MeritCA_Put (sshotinfo_p, conf - React_Head_Confidence_Get (head_p) - conf_adj);
}

  compound_p = &comp_base_p[application + 1];
  SubGenr_Compound_Ease_Put (compound_p, ease);
  SubGenr_Compound_Yield_Put (compound_p, yield);
  SubGenr_Compound_Confidence_Put (compound_p, conf);

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Leaving SMerit_Update, status = %hu", pass));

  return pass;
}

/****************************************************************************
*
*  Function Name:                 SPi_Orbital_Check
*
*    This routine simply checks to make sure that the bond between the two
*    atoms is a double bond.
*
*  Used to be:
*
*    pi_orb:
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
*    N/A
*
******************************************************************************/
static Boolean_t SPi_Orbital_Check
  (
  Xtr_t        *xtr_p,                      /* XTR to check */
  U16_t         atom1,                      /* One atom of potential pi-bond */
  U16_t         atom2                       /* Other atom of pot. pi-bond */
  )
{
  U8_t          bond;                       /* Bond between atoms */

  bond = Xtr_Attr_NeighborBond_Find (xtr_p, atom1, atom2);
  if (bond == BOND_DOUBLE || bond == BOND_RESONANT)
    return TRUE;

  return FALSE;
}

/****************************************************************************
*
*  Function Name:                 SSchema_Active_Find
*
*    This procedure locates all the active atoms in the schema.  These are the
*    ones that change their bonds.  It also locates active bonds which are
*    the ones that change.
*
*  Used to be:
*
*    find_all_active:
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
*    N/A
*
******************************************************************************/
static void SSchema_Active_Find
  (
  Xtr_t        *goal_xtr_p,                 /* Goal pattern */
  Xtr_t        *subgoal_xtr_p,              /* Subgoal pattern */
  U16_t        num_atoms,       
  Array_t      *active_atoms_p,             /* 1d-bit, active atoms found */
  Array_t      *active_bonds_p              /* 2d-bit, active bonds found */
  )
{
  U16_t         i, j;                       /* Counters */
  U16_t         neigh;                      /* Neighbor atom index */
  U8_t          bond;                       /* Bond size between neighbors */
  U8_t          num_neighbors;              /* Compiler bug */


  /* Keep in mind that the number of subgoal atoms is not necessarily
     equal to the number of goal atoms.
  */

  for (i = 0; i < num_atoms; i++)
    {
    num_neighbors = Xtr_Attr_NumNeighbors_Get (goal_xtr_p, i);
    for (j = 0; j < num_neighbors; j++)
      {
      neigh = Xtr_Attr_NeighborId_Get (goal_xtr_p, i, j);
      bond = Xtr_Attr_NeighborBond_Get (goal_xtr_p, i, j);
      if (!(bond == BOND_VARIABLE || bond == Xtr_Attr_NeighborBond_Find (
	  subgoal_xtr_p, i, neigh)))
	{
	Array_1d1_Put (active_atoms_p, i, TRUE);
	Array_2d1_Put (active_bonds_p, i, j, TRUE);
	}
      }
    }

  return;
}

/****************************************************************************
*
*  Function Name:                 SSchema_Apply
*
*    This procedure takes a complete goal compound, the goal and subgoal
*    patterns and transforms the goal compound into the subgoal using the match
*    map.
*
*  Used to be:
*
*    formpr:
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
*    Address of new subgoal TSD
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static Tsd_t *SSchema_Apply
  (
  Tsd_t        *curtsd_p,                   /* Current product, TSD format */
  Tsd_t        *goal_tsd_p,                 /* Goal pattern, TSD format */
  Xtr_t        *goal_xtr_p,                 /* Goal pattern, XTR format */
  Tsd_t        *subgoal_tsd_p,              /* Subgoal pattern, TSD format */
  Xtr_t        *subgoal_xtr_p,              /* Subgoal pattern, XTR format */
  Tsd_t        *complete_tsd_p,             /* "Complete" subgoal, TSD format,
    this is an extension of curtsd_p */
  Array_t      *complete_match_p,           /* 1d-word, match map */
  Array_t      *goal_pieces_p,              /* 1d-word, root -> piece map */
  U16_t         goal_pattern_size,          /* # connected atoms in pattern */
  U16_t         piece                       /* Piece number */
  )
{
  Tsd_t        *tmptsd_p;                   /* Before image of subgoal */
  Tsd_t        *subtsd_p;                   /* New subgoal, generated using
    patterns from tmptsd (complete_tsd) */
  U16_t         goal_num_atoms;             /* # atoms in goal pattern */
  U16_t         comp_num_atoms;             /* # atoms in complete goal */
  U16_t         i, j;                       /* Counters */
  U16_t         neigh;
  Array_t       newmatch;                   /* 1d-bit, indicates which atoms
    are actually matched, as opposed to subsumed by variable nodes */
U16_t ndb, nrb, bondmult, neigh_neigh, nbr_id;
Boolean_t varbond_flag;
Xtr_t *subxtr_p;
Sling_t sling;

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Entering SSchema_Apply, current TSD %p, goal TSD %p, goal XTR %p,"
    " subgoal TSD %p, subgoal XTR %p, complete TSD %p, complete match %p,"
    " goal pieces %p, goal pattern size %u, piece # %u", curtsd_p, 
    goal_tsd_p, goal_xtr_p, subgoal_tsd_p, subgoal_xtr_p, complete_tsd_p, 
    complete_match_p, goal_pieces_p, goal_pattern_size, piece));

  /* For most of this procedure tmptsd is the input to match against the
     goal pattern, and subtsd is the subgoal being constructed using the
     subgoal pattern.

     - Fill in the part of the match map corresponding to the "constant"
       nodes at the end of the goal pattern in the goal_tsd
     - Set the quickie match map, make sure that we have in fact matched
       everything (should know this!)
     - Loop over all the atoms to be added to the subgoal under construction
  */


  goal_num_atoms = Tsd_NumAtoms_Get (goal_tsd_p);
  comp_num_atoms = Tsd_NumAtoms_Get (complete_tsd_p);
  tmptsd_p = Tsd_Copy (complete_tsd_p);
  subtsd_p = Tsd_Create (comp_num_atoms);
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&newmatch", "subgoalgeneration{15}", &newmatch, comp_num_atoms, BITSIZE);
#else
  Array_1d_Create (&newmatch, comp_num_atoms, BITSIZE);
#endif
  Array_Set (&newmatch, FALSE);

  for (i = goal_pattern_size; i < goal_num_atoms; i++)
    {
    Array_1d16_Put (complete_match_p, i, i - goal_pattern_size +
      Tsd_NumAtoms_Get (curtsd_p));
    Array_1d16_Put (goal_pieces_p, i, piece + 1); /* ??? */
    }

  for (i = 0; i < goal_num_atoms; i++)
    {
    j = Array_1d16_Get (complete_match_p, i);
    if (j != XTR_INVALID)
      Array_1d1_Put (&newmatch, j, TRUE);
    else
      printf ("Major foobar! schema = %d\n", current_schema);
    }

  /* Generate the output TSD, matched and unmatched atoms get different
     treatment (obviously).
     - Atom id is copied for all atoms
     - If the atom is NOT matched, ie it is part of a variable structure
       then just copy it straight
       else if find atom matching this index in subgoal, handle it,
	    variable goal pattern nodes vs constant goal pattern nodes
  */

  for (i = 0, varbond_flag = FALSE; i < comp_num_atoms; i++)
    {
    Tsd_Atomid_Put (subtsd_p, i, Tsd_Atomid_Get (tmptsd_p, i));

    if (Array_1d1_Get (&newmatch, i) == FALSE)
      {
      Tsd_RowCopy (subtsd_p, i, tmptsd_p, i);
      }
    else
      for (j = 0; j < goal_num_atoms; j++)
	{
	if (Array_1d16_Get (complete_match_p, j) == i)
	  {
	  if (Atom_IsVariable (Tsd_Atomid_Get (goal_tsd_p, j)) == TRUE)
	    SVariable_Set (goal_tsd_p, subgoal_tsd_p, tmptsd_p, subtsd_p, curtsd_p,
              complete_match_p, i, j, &varbond_flag);
	  else
	    SConstant_Set (goal_tsd_p, subgoal_tsd_p, tmptsd_p, subtsd_p,
	      goal_xtr_p, subgoal_xtr_p, complete_match_p, i, j);

	  Tsd_RowCopy (tmptsd_p, i, subtsd_p, i);
    
	  Tsd_Atomid_Put (subtsd_p, i, TSD_INVALID);
	  Tsd_Atom_Flags_Put (subtsd_p, i, 0);
	  for (neigh = 0; neigh < MX_NEIGHBORS; neigh++)
	    {
	    Tsd_Atom_NeighborId_Put (subtsd_p, i, neigh, TSD_INVALID);
	    Tsd_Atom_NeighborBond_Put (subtsd_p, i, neigh, BOND_NONE);
	    }

	  }
	}
    }                    /* End for-i loop */

  for (i = 0; i < comp_num_atoms; i++)
    Tsd_RowCopy (subtsd_p, i, tmptsd_p, i);

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&newmatch", "subgoalgeneration", &newmatch);
#else
  Array_Destroy (&newmatch);
#endif
  Tsd_Destroy (tmptsd_p);

/* Now check for interrupted aromaticity; fix if possible, otherwise reject */
#ifdef _DEBUG_
printf("checking for interrupted aromaticity\n");
#endif
if (varbond_flag)
  {
  for (i = 0; i < comp_num_atoms; i++)
    {
    ndb=nrb=0;
    for (neigh = 0; neigh < MX_NEIGHBORS; neigh++)
      {
      bondmult = Tsd_Atom_NeighborBond_Get (subtsd_p, i, neigh);
      if (bondmult == BOND_DOUBLE) ndb++;
      else if (bondmult == BOND_RESONANT) nrb++;
      }
    if (nrb == 1)
      {
#ifdef _DEBUG_
printf("found one at %d\n",i);
#endif
      if (ndb != 1)
        {
#ifdef _DEBUG_
printf("bad bonding\n");
#endif
        Tsd_Destroy (subtsd_p);
        return (NULL);
        }
#ifdef _DEBUG_
printf("ok\n");
#endif
      for (neigh = 0; neigh < MX_NEIGHBORS; neigh++)
        if (Tsd_Atom_NeighborBond_Get (subtsd_p, i, neigh) == BOND_DOUBLE)
        {
        Tsd_Atom_NeighborBond_Put (subtsd_p, i, neigh, BOND_RESONANT);
        nbr_id = Tsd_Atom_NeighborId_Get (subtsd_p, i, neigh);
#ifdef _DEBUG_
printf("neighbor at %d\n",nbr_id);
#endif
        ndb=nrb=0;
        for (neigh_neigh = 0; neigh_neigh < MX_NEIGHBORS; neigh_neigh++)
          {
          bondmult = Tsd_Atom_NeighborBond_Get (subtsd_p, nbr_id, neigh_neigh);
          if (bondmult == BOND_DOUBLE) ndb++;
          else if (bondmult == BOND_RESONANT) nrb++;
          }
        if (nrb != 1 || ndb != 1)
          {
#ifdef _DEBUG_
printf("bad retro-bonding\n");
#endif
          Tsd_Destroy (subtsd_p);
          return (NULL);
          }
#ifdef _DEBUG_
printf("found neighbor at %d\n",nbr_id);
#endif
        for (neigh_neigh = 0; neigh_neigh < MX_NEIGHBORS; neigh_neigh++)
          if (Tsd_Atom_NeighborBond_Get (subtsd_p, nbr_id, neigh_neigh) == BOND_DOUBLE)
          Tsd_Atom_NeighborBond_Put (subtsd_p, nbr_id, neigh_neigh, BOND_RESONANT);
        }
      }
    }
  subxtr_p = Tsd2Xtr (subtsd_p);
for (i=0; i<Xtr_NumAtoms_Get(subxtr_p); i++)
for (neigh=0; neigh<Xtr_Attr_NumNeighbors_Get(subxtr_p,i); neigh++)
if (Xtr_Attr_NeighborBond_Get (subxtr_p,i,neigh)==BOND_RESONANT)
ResonantBonds_Fix(subxtr_p,i,neigh,1);
  Xtr_Attr_ResonantBonds_Set (subxtr_p);
  subtsd_p = Xtr2Tsd (subxtr_p);
  Xtr_Destroy (subxtr_p);
#ifdef _DEBUG_
Tsd_Dump (subtsd_p, &GStdOut);
sling=Tsd2Sling(subtsd_p);
printf("%s\n",Sling_Name_Get(sling));
Sling_Destroy(sling);
#endif
  }

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Leaving SSchema_Apply, subgoal TSD %p", subtsd_p));

  return subtsd_p;
}

/****************************************************************************
*
*  Function Name:                 SSeparate
*
*    This procedure separates a TSD containing several interleaved molecules
*    such as produced by SSchema_Apply into a list of TSDs, one for each
*    individual molecule.
*
*    The procedure works by the reachable nodes concept from graph theory.  It
*    starts at a node it knows has not been processed yet and then it keeps
*    a stack of neighbors that haven't been processed yet.  When the stack
*    runs dry then all reachable nodes have been processed so a new TSD is
*    created and placed at the end of the list.
*
*  Used to be:
*
*    separat, mpseprt:
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
*    N/A
*
******************************************************************************/
static void SSeparate
  (
  Tsd_t        *tsd_p,                      /* Handle of TSD to unentwine */
  List_t       *list_p,                     /* List of resulting TSDs */
  Array_t      *map_p                       /* 2d-word, atom -> subgoal # &
    new atom idx */
  )
{
  Stack_t      *stack_p;                    /* For handling single molecule */
  Tsd_t        *modtsd_p;                   /* Accumulator for new TSD */
  Tsd_t        *newtsd_p;                   /* Proper sized TSD, with mappng */
  U16_t         num_atoms;                  /* # atoms in input TSD */
  U16_t         next_row;                   /* Index into modtsd */
  U16_t         neigh;                      /* Temporary */
  U16_t         atom;                       /* Counter */
  U16_t         subgoal;                    /* Index */
  U16_t         i, j;                       /* Counters */
  U32_t         stacksize;                  /* size of stack */
  Array_t       rowdone;                    /* 1d-bit, row done flags */
  Array_t       instack;                    /* 1d-bit, processed flags */
  Array_t       tmpmap;                     /* 1d-word, input -> new atomidx */

  num_atoms = Tsd_NumAtoms_Get (tsd_p);
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&rowdone", "subgoalgeneration{16}", &rowdone, num_atoms, BITSIZE);
  Array_Set (&rowdone, FALSE);
  mind_Array_1d_Create ("&instack", "subgoalgeneration{16}", &instack, num_atoms, BITSIZE);
  mind_Array_1d_Create ("&tmpmap", "subgoalgeneration{16}", &tmpmap, num_atoms, WORDSIZE);
#else
  Array_1d_Create (&rowdone, num_atoms, BITSIZE);
  Array_Set (&rowdone, FALSE);
  Array_1d_Create (&instack, num_atoms, BITSIZE);
  Array_1d_Create (&tmpmap, num_atoms, WORDSIZE);
#endif
  stack_p = Stack_Create (STACK_SCALAR);
  modtsd_p = Tsd_Create (num_atoms);

  atom = 0;
  subgoal = 0;
  while (atom < num_atoms)
    {
    next_row = 0;
    Array_Set (&tmpmap, TSD_INVALID);
    Array_Set (&instack, FALSE);
    for (i = 0; i < num_atoms; i++)
      {
      Tsd_Atomid_Put (modtsd_p, i, TSD_INVALID);

/*  The following initialization is already done in Tsd_Create () */
/*
      Tsd_Atom_Flags_Put (modtsd_p, i, 0);
      for (j = 0; j < MX_NEIGHBORS; j++)
	{
	Tsd_Atom_NeighborId_Put (modtsd_p, i, j, TSD_INVALID);
	Tsd_Atom_NeighborBond_Put (modtsd_p, i, j, BOND_NONE);
	}
*/
      }

    for (atom = 0; atom < num_atoms && Array_1d1_Get (&rowdone, atom) ==
	 TRUE; atom++)
      /* Empty loop */ ;
  

    if (atom < num_atoms)
      {
      Array_1d1_Put (&instack, atom, TRUE);
      do
	{
	Array_1d1_Put (&rowdone, atom, TRUE);
	Array_2d16_Put (map_p, atom, SUBG_COL, subgoal);
	Array_2d16_Put (map_p, atom, ATOM_COL, next_row);
	Tsd_RowCopy (modtsd_p, next_row, tsd_p, atom);
	for (j = 0; j < MX_NEIGHBORS; j++)
	  {
	  neigh = Tsd_Atom_NeighborId_Get (tsd_p, atom, j);
	  if (neigh != TSD_INVALID && Array_1d1_Get (&instack, neigh) == FALSE)
	    {
	    Stack_PushU16 (stack_p, neigh);
	    Array_1d1_Put (&instack, neigh, TRUE);
	    }
	  }

	Array_1d16_Put (&tmpmap, atom, next_row);
	next_row++;
	stacksize = Stack_Size_Get (stack_p);
	if (stacksize > 0)
	   {
	   atom = Stack_TopU16 (stack_p);
	   Stack_Pop (stack_p);
	   }
	}
      while (stacksize > 0);

      newtsd_p = Tsd_Create (next_row);
      for (i = 0; i < next_row; i++)
	{
	Tsd_RowCopy (newtsd_p, i, modtsd_p, i);
	for (j = 0; j < MX_NEIGHBORS; j++)
	  if (Tsd_Atom_NeighborId_Get (modtsd_p, i, j) != TSD_INVALID)
	    Tsd_Atom_NeighborId_Put (newtsd_p, i, j, Array_1d16_Get (&tmpmap,
	      Tsd_Atom_NeighborId_Get (modtsd_p, i, j)));
	}

	/* should insert in the front instead of the end */
      List_InsertAdd (list_p, NULL, newtsd_p);
/*
      List_InsertAdd (list_p, List_Tail_Get (list_p), newtsd_p);
*/
      }
    subgoal++;
    }

  for (i = 0; i < num_atoms; ++i)
    Array_2d16_Put (map_p, i, SUBG_COL, (subgoal - 
	Array_2d16_Get (map_p, i, SUBG_COL) - 2));

  Tsd_Destroy (modtsd_p);
  Stack_Destroy (stack_p);
#ifdef _MIND_MEM_
  mind_Array_Destroy ("&rowdone", "subgoalgeneration", &rowdone);
  mind_Array_Destroy ("&instack", "subgoalgeneration", &instack);
  mind_Array_Destroy ("&tmpmap", "subgoalgeneration", &tmpmap);
#else
  Array_Destroy (&rowdone);
  Array_Destroy (&instack);
  Array_Destroy (&tmpmap);
#endif

  return;
}

/****************************************************************************
*
*  Function Name:                 SSeparate_Count
*
*    This function counts the number of molecules with more than a single
*    atom that are disentangled by SSeparate.
*
*  Used to be:
*
*    sepmol, msepmol:
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
*    # molecules found in TSD
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static U16_t SSeparate_Count
  (
  Tsd_t        *tsd_p,                      /* Handle of TSD to unentwine */
  List_t       *list_p,                     /* List of resulting TSDs */
  Array_t      *map_p                       /* 2d-word, atom -> subgoal # &
    new atom idx */
  )
{
  ListElement_t *elem_p;                    /* For list traversal */
  U16_t         num_molecules;              /* Counter */

  /* This procedure calls SSeparate, and then counts the number of non-atoms
     found.
  */

  SSeparate (tsd_p, list_p, map_p);

  for (elem_p = List_Front_Get (list_p), num_molecules = 0; elem_p != NULL;
       elem_p = LstElem_Next_Get (elem_p))
    {
    tsd_p = (Tsd_t *) LstElem_ValueAdd_Get (elem_p);
    if (Tsd_NumAtoms_Get (tsd_p) > 1)
      num_molecules++;
    }

  return num_molecules;
}

/****************************************************************************
*
*  Function Name:                 SStrategicBond_Exists
*
*    This function searches through all the compounds generated so far and
*    sees whether any of them have used a strategic bond.
*
*    Note: Since the strategic bond detection algorithm is currently (9/93)
*    unimplemented this is always TRUE.
*
*  Used to be:
*
*    at_least_one_bond_strategic:
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
*    True - there is a strategic bond in the match
*    False - there is not a strategic bond in the match
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static Boolean_t SStrategicBond_Exists
  (
  SubGenr_Compound_t *comp_base_p,          /* Handle to relevant compound */
  U16_t         application                 /* Application # */
  )
{
  U16_t         i;                          /* Counter */

  for (i = 1; i < application + 2; i++)     /* + 2 goes with <, 0th is base */
    if (SubGenr_Compound_Strategic_Get (&comp_base_p[i]) == TRUE)
      return TRUE;

  return FALSE;
}

/****************************************************************************
*
*  Function Name:                 SSubgoal_ActiveBonds_Set
*
*    This procedure checks for active bonds in the subgoal pattern, these are
*    the ones that are matched and the bond is active.  These neighbors are
*    then marked as processed.
*
*  Used to be:
*
*    add_active_sgp_bonds:
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
*    N/A
*
******************************************************************************/
static void SSubgoal_ActiveBonds_Set
  (
  Tsd_t        *subtsd_p,                   /* New subgoal being created */
  Tsd_t        *subgoal_tsd_p,              /* Subgoal pattern */
  Xtr_t        *goal_xtr_p,                 /* Goal pattern, XTR format */
  Xtr_t        *subgoal_xtr_p,              /* Subgoal pattern, XTR format */
  Array_t      *complete_match_p,           /* 1d-word, match map */
  U16_t         sub_atom,                   /* Atom in new subgoal */
  U16_t         goal_atom,                  /* Atom in goal pattern */
  Boolean_t    *subgoal_done,               /* Neighbor done flags, pattern */
  U16_t        *subgoal_count_p             /* Subgoal pattern neighs done */
  )
{
  U16_t         i;                          /* Counter */
  U16_t         sub_neighid;                /* Neighbor in subgoal pattern */
  U16_t         image;                      /* Match image of goal atom */

  /* Don't count images that evaluate to TSD_INVALID as those were already
     counted by SEmpties_Set.
  */

  for (i = 0; i < MX_NEIGHBORS; i++)
    {
    sub_neighid = Tsd_Atom_NeighborId_Get (subgoal_tsd_p, goal_atom, i);
    if (sub_neighid != TSD_INVALID && SSubgoalBond_IsActive (goal_xtr_p,
	subgoal_xtr_p, goal_atom, sub_neighid) == TRUE)
      {
      image = Array_1d16_Get (complete_match_p, sub_neighid);
      SSubgoal_Atom_Set (subtsd_p, subgoal_tsd_p, sub_atom, goal_atom, i,
	image);
      subgoal_done[i] = TRUE;
      (*subgoal_count_p)++;
      }
    }

  return;
}

/****************************************************************************
*
*  Function Name:                 SSubgoal_Atom_Set
*
*    This procedure sets an atom in the new subgoal, including its bond.
*    This is for the case when the bond is active.
*
*  Used to be:
*
*    set_subgoal_column:
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
*    N/A
*
******************************************************************************/
static void SSubgoal_Atom_Set
  (
  Tsd_t        *subtsd_p,                   /* New subgoal being created */
  Tsd_t        *subgoal_tsd_p,              /* Subgoal pattern we are using */
  U16_t         sub_atom,                   /* Atom in new subgoal (originally
    found 3 scope levels up! */
  U16_t         goal_atom,                  /* Atom in subgoal pattern (this
    was only 2(!) scope levels up!  Jerks. */
  U16_t         neigh_idx,                  /* Neighbor index in goal */
  U16_t         neigh                       /* Neighbor in goal */
  )
{
  U16_t         sub_idx;                    /* Index in new subgoal */
  U16_t         i;                          /* Counter */
  Boolean_t     done;                       /* Flag for first open slot */

  /* Find the neighbor index (bond direction) that this bond should be mapped
     to.  Default is the one it was originally mapped to by the pattern.
  */

  sub_idx = neigh_idx;

  if (Tsd_Atom_NeighborId_Get (subtsd_p, sub_atom, neigh_idx) != TSD_INVALID)
    {
    Tsd_AtomFlags_DontCare_Put (subtsd_p, sub_atom, TRUE);
    for (i = 0, done = FALSE; i < MX_NEIGHBORS && done == FALSE; i++)
       if (Tsd_Atom_NeighborId_Get (subtsd_p, sub_atom, i) == TSD_INVALID)
	 {
	 sub_idx = i;
	 done = TRUE;
	 }

    ASSERT (done != FALSE,
      {
      printf ("Major foobar in SSubgoal_Set\n");
      });
    }

  Tsd_Atom_NeighborId_Put (subtsd_p, sub_atom, sub_idx, neigh);
  Tsd_Atom_NeighborBond_Put (subtsd_p, sub_atom, sub_idx,
    Tsd_Atom_NeighborBond_Get (subgoal_tsd_p, goal_atom, neigh_idx));

  return;
}

/****************************************************************************
*
*  Function Name:                 SSubgoalBond_IsActive
*
*    This procedure checks for active bonds in the subgoal pattern.  To be
*    active the bond must be different or it must have a different neighbor.
*
*  Used to be:
*
*    sgp_activeb:
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
*    True - bond between atom and neighbor is active
*    False - bond between atom and neighbor is inactive
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static Boolean_t SSubgoalBond_IsActive
  (
  Xtr_t        *goal_xtr_p,                 /* Goal pattern, XTR format */
  Xtr_t        *subgoal_xtr_p,              /* Subgoal pattern, XTR format */
  U16_t         atom,                       /* Atom in goal */
  U16_t         neigh                       /* Neighbor in goal */
  )
{
  U8_t          i;                          /* Counter */
  U8_t          sub_bond;                   /* Bond in subgoal pattern */
  U8_t          goal_bond;                  /* Bond in goal pattern */

  if (neigh == TSD_INVALID)
    return FALSE;

  sub_bond = Xtr_Attr_NeighborBond_Find (subgoal_xtr_p, atom, neigh);
  for (i = 0; i < MX_NEIGHBORS; i++)
    {
/*
    goal_bond = Xtr_Attr_NeighborBond_Get (goal_xtr_p, atom, i);
*/
    goal_bond = Xtr_Attr_NeighborBond_Find (goal_xtr_p, atom, 
	Xtr_Attr_NeighborId_Get (goal_xtr_p, atom, i));  
	
    if (goal_bond == sub_bond && neigh == Xtr_Attr_NeighborId_Get (goal_xtr_p,
	atom, i))
      return FALSE;
    }

  return TRUE;
}

/****************************************************************************
*
*  Function Name:                 SSubgoal_Sort
*
*    This function sorts the array of xtr's according the the cannonical
*    name (sling).  It returns the sorted slings in subgxtr_p and the
*    number of unique nontrivial slings in num_compounds_p.
*
*  Used to be:
*
*    N/A
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
*    None.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SSubgoal_Sort
  (
  Array_t      *sortxtr_p,                  /* 1d-addr, output sorted array */
  Array_t      *sortcanxtr_p,               /* 1d-addr, output sorted canon */
  Sling_t      *comp_slings_p,              /* Array of sorted slings */
  Array_t      *subgxtr_p,                  /* 1d-addr, XTRs of compounds */
  Array_t      *canonxtr_p,                 /* 1d-addr, canonical XTRs */
  U16_t        *num_compounds_p,            /* # non-duplicate compounds */
  U16_t         num_xtrs,                   /* # entries in subgxtr */
  Boolean_t     stereo_chemistry
  )
{
  Xtr_t        *xtr_p;                      /* Temp handle */
  Xtr_t        *cxtr_p;                     /* Temp handle */
  S16_t         j;                          /* Counter */
  U16_t         i;                          /* Counter */
  U16_t         insert_idx;                 /* Where to insert */
  S16_t         diff;                       /* Sling comparision result */
  Boolean_t     less;                       /* Comparision flags */
  Sling_t       sling;                      /* Temp. */

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Entering SSubgoal_Sort, sorted XTRs %p, sorted slings %p,"
    " unsorted XTRs %p, # XTRs %u", sortxtr_p, comp_slings_p, 
    subgxtr_p, num_xtrs));

  *num_compounds_p = 0;

  /* Use bubble-sort, O(n**2) to sort compounds by canonical sling, this
     gives a definitive ordering for the compounds within the mega-sling.
  */

#ifdef _MIND_MEM_
in_subgenr(-1);
#endif
  for (i = 0; i < num_xtrs; i++)
    {
    xtr_p = (Xtr_t *) Array_1dAddr_Get (subgxtr_p, i);
    if (canonxtr_p != NULL)
      cxtr_p = (Xtr_t *) Array_1dAddr_Get (canonxtr_p, i);

    if (Xtr_NumAtoms_Get (xtr_p) > 1)
      {
#ifdef _MIND_MEM_
in_subgenr(-2);
#endif
      sling = Name_Sling_Get (xtr_p, stereo_chemistry == TRUE ? FALSE : TRUE);
#ifdef _MIND_MEM_
in_subgenr(-3);
#endif
      for (j = 0, less = TRUE; j < (S16_t) *num_compounds_p && less == TRUE; 
	  j++)
	{
#ifdef _MIND_MEM_
in_subgenr(-4);
#endif
	diff = strcmp ((char *) Sling_Name_Get (sling), 
	  (char *) Sling_Name_Get (comp_slings_p[j]));
#ifdef _MIND_MEM_
in_subgenr(-5);
#endif
	less = diff < 0;
	insert_idx = j;
	}

      if (less)
	{
if (Sling_Name_Get (comp_slings_p[*num_compounds_p]) != NULL) Sling_Destroy (comp_slings_p[*num_compounds_p]);
	Array_1dAddr_Put (sortxtr_p, *num_compounds_p, xtr_p);
	if (canonxtr_p != NULL)
	  Array_1dAddr_Put (sortcanxtr_p, *num_compounds_p, cxtr_p);
	comp_slings_p[*num_compounds_p] = sling;
	(*num_compounds_p)++;
	}
      else
	if (diff != 0)  /* Not the same */
	  {
if (Sling_Name_Get (comp_slings_p[*num_compounds_p]) != NULL) Sling_Destroy (comp_slings_p[*num_compounds_p]);
	  for (j = (S16_t) *num_compounds_p - 1; j >= (S16_t) insert_idx; j--)
	    {
	    comp_slings_p[j + 1] = comp_slings_p[j];
	    Array_1dAddr_Put (sortxtr_p, j + 1, 
	      Array_1dAddr_Get (sortxtr_p, j));
	    if (canonxtr_p != NULL)
	      Array_1dAddr_Put (sortcanxtr_p, j + 1,
	       Array_1dAddr_Get (sortcanxtr_p, j));
	    }

	  comp_slings_p[insert_idx] = sling;
	  Array_1dAddr_Put (sortxtr_p, insert_idx, xtr_p);
	  if (canonxtr_p != NULL)
	    Array_1dAddr_Put (sortcanxtr_p, insert_idx, cxtr_p);
	  (*num_compounds_p)++;
	  }
      }  /* End if-Xtr_NumAtoms_Get */
    }  /* End for-i */
#ifdef _MIND_MEM_
in_subgenr(-6);
#endif

  return ;
}

/****************************************************************************
*
*  Function Name:                 SVariable_Set
*
*    This procedure sets an atom in the new subgoal when it is
*    instantiated by a variable node in the goal pattern.  Match image is the
*    term used to indicate the index in the subgoal under construction that
*    corresponds to a given goal pattern index.
*
*  Used to be:
*
*    change_variable_connections:
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
*    N/A
*
******************************************************************************/
static void SVariable_Set
  (
  Tsd_t        *goal_tsd_p,                 /* Goal pattern */
  Tsd_t        *subgoal_tsd_p,              /* Subgoal pattern */
  Tsd_t        *tmptsd_p,                   /* Before image of subgoal */
  Tsd_t        *subtsd_p,                   /* New subgoal being created */
Tsd_t *curtsd_p,
  Array_t      *complete_match_p,           /* 1d-word, match map */
  U16_t         sub_atom,                   /* Atom in new subgoal */
  U16_t         goal_atom,                  /* Atom in goal pattern */
Boolean_t    *varbond_flag
  )
{
  U16_t         goal_neigh;                 /* Neigh. index in goal patt. */
  U16_t         subgoal_neigh;              /* Neigh. index in subgoal patt */
  U16_t         i;                          /* Counter */
U16_t target_atom, target_neigh, j;
U8_t goal_bond, subgoal_bond;

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Entering SVariable_Set, goal TSD %p, subgoal TSD %p, prev subgoal %p,"
    " new subgoal %p, complete match %p, subgoal atom %u, goal pattern atom %u",
    goal_tsd_p, subgoal_tsd_p, tmptsd_p, subtsd_p, complete_match_p, sub_atom,
    goal_atom));

  /* Algorithm for processing goes as follows:
     - Copy the "new" atom and its neighbors
     - Find the last (highest bond direction) neighbors in the goal and subgoal
       patterns using pattern atom index
     - If the new subgoal's neighbor has the match image of the goal pattern
       atom index, then replace it with the match image of the subgoal pattern
       atom index.
  */

  Tsd_RowCopy (subtsd_p, sub_atom, tmptsd_p, sub_atom);

  for (i = 0, goal_neigh = TSD_INVALID, subgoal_neigh = TSD_INVALID; i <
       MX_NEIGHBORS; i++)
    {
     if (Tsd_Atom_NeighborId_Get (goal_tsd_p, goal_atom, i) != TSD_INVALID)
       {
       goal_neigh = Tsd_Atom_NeighborId_Get (goal_tsd_p, goal_atom, i);
       goal_bond = Tsd_Atom_NeighborBond_Get (goal_tsd_p, goal_atom, i);
       }

     if (Tsd_Atom_NeighborId_Get (subgoal_tsd_p, goal_atom, i) != TSD_INVALID)
       {
       subgoal_neigh = Tsd_Atom_NeighborId_Get (subgoal_tsd_p, goal_atom, i);
       subgoal_bond = Tsd_Atom_NeighborBond_Get (subgoal_tsd_p, goal_atom, i);
       if (subgoal_bond == BOND_VARIABLE)
         {
*varbond_flag = TRUE;
         if (subgoal_neigh != goal_neigh || subgoal_bond != goal_bond)
           {
           fprintf (stderr, "Error: Variable bond is active in schema %d\n", current_schema);
           exit (1);
           }
         target_atom = Array_1d16_Get (complete_match_p, goal_atom);
         target_neigh = Array_1d16_Get (complete_match_p, goal_neigh);
         for (j=subgoal_bond=0; j<MX_NEIGHBORS && subgoal_bond==0; j++)
           if (Tsd_Atom_NeighborId_Get (curtsd_p, target_atom, j) == target_neigh)
{
/*
printf("Found neighbor %d for atom %d: %d (bond=%d)\n",j,target_atom,target_neigh,Tsd_Atom_NeighborBond_Get(curtsd_p,target_atom,j));
*/
           subgoal_bond = Tsd_Atom_NeighborBond_Get (curtsd_p, target_atom, j);
}
         if (subgoal_bond == 0)
           {
printf("\ntarget_atom=%d (goal_atom=%d) target_neigh=%d (goal_neigh=%d)\n",
target_atom,goal_atom,target_neigh,goal_neigh);
printf("curtsd_p:\n");
Tsd_Dump(curtsd_p,&GStdOut);
printf("goal_tsd_p:\n");
Tsd_Dump(goal_tsd_p,&GStdOut);
           fprintf (stderr, "Error: Unable to find target bond to match variable pattern bond (schema %d)\n", current_schema);
           exit (1);
           }
         }
       }
    }

  /* Every atom that we are considering should be in the pattern itself, ie
     not a constant atom and so it should have a neighbor.  This is true ???
     of the subgoal pattern as well.
  */

  ASSERT ((goal_neigh != TSD_INVALID && subgoal_neigh != TSD_INVALID),
    {
    printf ("Major foobar (SVariable_Set)\n");
    });

  for (i = 0; i < MX_NEIGHBORS; i++)
    if (Tsd_Atom_NeighborId_Get (subtsd_p, sub_atom, i) == Array_1d16_Get (
	complete_match_p, goal_neigh))
      {
      Tsd_Atom_NeighborId_Put (subtsd_p, sub_atom, i, Array_1d16_Get (
	complete_match_p, subgoal_neigh));
      Tsd_Atom_NeighborBond_Put (subtsd_p, sub_atom, i, subgoal_bond);
      }

  DEBUG (R_SUBGENR, DB_SUBGENRSTATIC, TL_PARAMS, (outbuf,
    "Leaving SVariable_Set, status = <void>"));

  return;
}
/* End of SVariable_Set */


/****************************************************************************
*
*  Function Name:                 Match_Tree_Dump
*
*
*  Used to be:
*
*    N/A
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
*    N/A
*
******************************************************************************/
void Match_Tree_Dump
  (
  Match_Node_t *node_p,
  FileDsc_t    *filed_p
  )
{
  FILE         *f;

  f = IO_FileHandle_Get (filed_p);
  if (node_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL Match Node\n");
    return;
    }

  SMatch_Tree_Dump (node_p, 1, f);
  fprintf (f, "\n\n");

  return;
}


/****************************************************************************
*
*  Function Name:                 SMatch_Tree_Dump
*
*
*  Used to be:
*
*    N/A
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
*    N/A
*
******************************************************************************/
static void SMatch_Tree_Dump
  (
  Match_Node_t *node_p,
  U16_t         depth,
  FILE         *f
  )
{
  U16_t         i;

  fprintf (f, " (%2d,%2d)", Match_Node_PatternLink_Get (node_p),
    Match_Node_GoalLink_Get (node_p));

  if (Match_Node_Forward_Get (node_p) != NULL)
    SMatch_Tree_Dump (Match_Node_Forward_Get (node_p), depth + 1, f);

  while (Match_Node_Brother_Get (node_p) != NULL)
    {
    fprintf (f, "\n");
    for (i = 0; i < depth; i++)
      fprintf (f, "        ");

    node_p = Match_Node_Brother_Get (node_p);
    SMatch_Tree_Dump (node_p, depth + 1, f);
    }

  return;
}


/****************************************************************************
*
*  Function Name:                 SApplicationMaps_Collect
*
*
*  Used to be:
*
*    N/A
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
*    N/A
*
******************************************************************************/
void SApplicationMaps_Collect
(
  Array_t  *match_p,
  Array_t  *sgmatch_p,
  List_t   *sgtsd_list,
  U32_t  ***retmaps,
  U16_t     application,
  U16_t     goal_num_atoms
)
{
  static U16_t *maps[256][2], *sgmaps[256][2], num_tsds=0;
  static Tsd_t *loc_sgtsds[10];
  static highest_app=-1, highest_sgapp=-1, last_app=-1, prev_app=-1;
  int app,i, j, k, image, sgnum, molnode, pattnode,tsdmap[10],cansln_map[10],posmap[10];
  char can_slings[10][1024];
  Boolean_t sorted, canmap_created[10];
  ListElement_t *elem_p;
  Array_t canmap[10];

#ifdef _MIND_MEM_
  char varname[100];

#ifdef _DEBUG_
printf("SApplicationMaps_Collect entered\n");
#endif
  if (retmaps != NULL)
    {
#ifdef _DEBUG_
printf("SApplicationMaps_Collect (retmaps)\n");
#endif
    mind_malloc ("retmaps[0]", "subgoalgeneration{17}", retmaps, (application + 1) * sizeof (U32_t *));
    for (i=0; i<=application; i++)
      {
      sprintf (varname, "retmaps[0][%d]", i);
      mind_malloc (varname, "subgoalgeneration{17}", retmaps[0] + i, goal_num_atoms * sizeof (U32_t));
      if (i < 256) for (j=0; j<goal_num_atoms; j++) retmaps[0][i][j] = j < 256 ?
        maps[i][0][j] | (sgmaps[i][0][j] << 8) | (maps[i][1][j] << 16) | (sgmaps[i][1][j] << 24) :
        XTR_INVALID | (j << 16);
      else for (j=0; j<goal_num_atoms; j++) retmaps[0][i][j] = XTR_INVALID | (j << 16);
      }
#ifdef _DEBUG_
printf("SApplicationMaps_Collect returning (retmaps)\n");
#endif
    return;
    }

  if (application>255) return;

  if (sgmatch_p != NULL)
    {
#ifdef _DEBUG_
printf("SApplicationMaps_Collect (sgmatch_p)\n");
#endif
    if (application != last_app)
      {
      printf("Error: subgoal mapping out of synch with goal mapping\n");
      exit(1);
      }

    for (i=application; i<=highest_sgapp; i++)
      {
      sprintf (varname, "sgmaps[%d][0]", i);
      mind_free (varname, "subgoalgeneration", sgmaps[i][0]);
      sprintf (varname, "sgmaps[%d][1]", i);
      mind_free (varname, "subgoalgeneration", sgmaps[i][1]);
      }
    for (i = 0; i < num_tsds; i++) Tsd_Destroy (loc_sgtsds[i]);
    highest_sgapp = application;
    sprintf (varname, "sgmaps[%d][0]", application);
    mind_malloc (varname, "subgoalgeneration{17}", sgmaps[application], 256 * sizeof (U16_t));
    sprintf (varname, "sgmaps[%d][1]", application);
    mind_malloc (varname, "subgoalgeneration{17}", sgmaps[application] + 1, 256 * sizeof (U16_t));
#else
  if (retmaps != NULL)
    {
    retmaps[0] = (U32_t **) malloc ((application + 1) * sizeof (U32_t *));
    for (i=0; i<=application; i++)
      {
      retmaps[0][i] = (U32_t *) malloc (goal_num_atoms * sizeof (U32_t));
      if (i < 256) for (j=0; j<goal_num_atoms; j++) retmaps[0][i][j] = j < 256 ?
        maps[i][0][j] | (sgmaps[i][0][j] << 8) | (maps[i][1][j] << 16) | (sgmaps[i][1][j] << 24) :
        XTR_INVALID | (j << 16);
      else for (j=0; j<goal_num_atoms; j++) retmaps[0][i][j] = XTR_INVALID | (j << 16);
      }
    return;
    }

  if (application>255) return;

  if (sgmatch_p != NULL)
    {
    if (application != last_app)
      {
      printf("Error: subgoal mapping out of synch with goal mapping\n");
      exit(1);
      }

    for (i=application; i<=highest_sgapp; i++)
      {
      free (sgmaps[i][0]);
      free (sgmaps[i][1]);
      }
    for (i = 0; i < num_tsds; i++) Tsd_Destroy (loc_sgtsds[i]);
    highest_sgapp = application;
    sgmaps[application][0] = (U16_t *) malloc (256 * sizeof (U16_t));
    sgmaps[application][1] = (U16_t *) malloc (256 * sizeof (U16_t));
#endif

    num_tsds = (U16_t) List_Size_Get (sgtsd_list);
    for (elem_p = List_Front_Get (sgtsd_list), i = 0; i < num_tsds; i++, elem_p = LstElem_Next_Get (elem_p))
      loc_sgtsds[i] = Tsd_Copy ((Tsd_t *) LstElem_ValueAdd_Get (elem_p));

    for (i=sgnum=0; i<num_tsds; i++)
      {
#ifdef _DEBUG_
printf("SApplicationMaps_Collect: i=%d sgnum=%d\n",i,sgnum);
#endif
      if (Tsd_NumAtoms_Get (loc_sgtsds[i]) > 1 && !SRepeat_Tsd (loc_sgtsds, (U16_t) i, canmap + i, can_slings[i]))
        {
        tsdmap[i] = sgnum++;
        canmap_created[i] = TRUE;
        }
      else
        {
        tsdmap[i] = -1;
        canmap_created[i] = FALSE;
        }
#ifdef _DEBUG_
printf("SApplicationMaps_Collect: i=%d sgnum=%d\n",i,sgnum);
#endif
      }

    for (i=0; i<num_tsds; i++) cansln_map[i]=i;

    for (i=0, sorted=FALSE; i<num_tsds-1 && !sorted; i++)
      {
      sorted = TRUE;
      for (j=num_tsds-1; j>i; j--) if (canmap_created[cansln_map[j]])
        {
        for (k=j-1; k>=i && !canmap_created[cansln_map[k]]; k--);
        if (k >= i && strcmp (can_slings[cansln_map[j]], can_slings[cansln_map[k]]) > 0)
          {
          sorted = FALSE;
          image = cansln_map[j];
          cansln_map[j] = cansln_map[k];
          cansln_map[k] = image;
          }
        }
      }

    for (i=0; i<num_tsds; i++) posmap[cansln_map[i]]=i;

    for (app=0; app<=application; app++)
      {
      for (i = 0; i < goal_num_atoms; i++)
        {
        molnode = maps[app][0][i];
        pattnode = maps[app][1][i];
        j = Array_2d16_Get (sgmatch_p, molnode, SUBG_COL);
        sgnum = tsdmap[posmap[j]] & 0xff;
        if (sgnum == 255) image = 255;
        else
          {
          image = Array_1d16_Get (canmap + j, Array_2d16_Get (sgmatch_p, molnode, ATOM_COL));
          if (image == TSD_INVALID) image = 255;
          }
        sgmaps[app][0][i] = sgnum;
        sgmaps[app][1][i] = image;
        }
      }

#ifdef _MIND_MEM_
    for (i = 0; i < num_tsds; i++) if (canmap_created[i])
      {
      sprintf(varname, "canmap+%d", i);
      mind_Array_Destroy (varname, "subgoalgeneration", canmap + i);
      }
#else
    for (i = 0; i < num_tsds; i++) if (canmap_created[i]) Array_Destroy (canmap + i);
#endif

#ifdef _DEBUG_
printf("SApplicationMaps_Collect returning (sgmatch_p)\n");
#endif
    return;
    }

/*
  if (match_p == NULL)
    {
    if (application != last_app)
      {
      printf ("Error: goal/subgoal purge out of synch with goal mapping\n");
      exit(1);
      }
    for (i=application; i<=highest_app; i++)
      {
      free (maps[i][0]);
      free (maps[i][1]);
      }
    for (i=0; i<=application; i++)
      {
      free (sgmaps[i][0]);
      free (sgmaps[i][1]);
      }

    last_app = prev_app;

    return;
    }
*/
#ifdef _DEBUG_
printf("SApplicationMaps_Collect (default)\n");
#endif

  prev_app = last_app;
  last_app = application;

#ifdef _MIND_MEM_
  for (i=application; i<=highest_app; i++)
    {
    sprintf (varname, "maps[%d][0]", i);
    mind_free (varname, "subgoalgeneration", maps[i][0]);
    sprintf (varname, "maps[%d][1]", i);
    mind_free (varname, "subgoalgeneration", maps[i][1]);
    }
  highest_app = application;
  sprintf (varname, "maps[%d][0]", application);
  mind_malloc (varname, "subgoalgeneration{17}", maps[application], 256 * sizeof (U16_t));
  sprintf (varname, "maps[%d][1]", application);
  mind_malloc (varname, "subgoalgeneration{17}", maps[application] + 1, 256 * sizeof (U16_t));
#else
  for (i=application; i<=highest_app; i++)
    {
    free (maps[i][0]);
    free (maps[i][1]);
    }
  highest_app = application;
  maps[application][0] = (U16_t *) malloc (256 * sizeof (U16_t));
  maps[application][1] = (U16_t *) malloc (256 * sizeof (U16_t));
#endif

  for (i=0; i<goal_num_atoms && i<256; i++)
    {
    image = Array_1d16_Get (match_p, i);
    maps[application][0][i] = image;
    maps[application][1][i] = i;
    }
  for (i=0, sorted=FALSE; i<goal_num_atoms - 1 && !sorted; i++)
    {
    sorted = TRUE;
    for (j=goal_num_atoms - 1; j>i; j--) if (maps[application][0][j] < maps[application][0][j-1])
      {
      sorted = FALSE;
      image = maps[application][0][j];
      maps[application][0][j] = maps[application][0][j-1];
      maps[application][0][j-1] = image;
      image = maps[application][1][j];
      maps[application][1][j] = maps[application][1][j-1];
      maps[application][1][j-1] = image;
      }
    }
#ifdef _DEBUG_
printf("SApplicationMaps_Collect returning (default)\n");
#endif
}


/****************************************************************************
*
*  Function Name:                 SRepeat_Tsd
*
*
*  Used to be:
*
*    N/A
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
*    N/A
*
******************************************************************************/
Boolean_t SRepeat_Tsd (Tsd_t *tsd_array[], U16_t current_tsdnum, Array_t *can_map, char *can_sling)
{
  Xtr_t *curr_xtr, *compare_xtr, *canxtr;
  Tsd_t *cantsd;
  Sling_t cansln;
  Array_t map,backmap,longmap;
  U8_t curr_name[1024], *compare_name;
  Boolean_t sling_found,nodemap_found,nbr_used[256][MX_NEIGHBORS];
  int i,j,k,mult,orig_natms,natms,multatms,where,can_where,nbr;
  char *barpos,*canname;

#ifdef _DEBUG_
printf("SRepeat_Tsd entered: %d\n",current_tsdnum);
Tsd_Dump(tsd_array[current_tsdnum],&GStdOut);
#endif
  curr_xtr = Tsd2Xtr (tsd_array[current_tsdnum]);
#ifdef _DEBUG_
printf("curr_xtr=%p\n",curr_xtr);
Xtr_Dump(curr_xtr,&GStdOut);
#endif
  Xtr_Name_Set (curr_xtr, can_map);
#ifdef _DEBUG_
printf("After Xtr_Name_Set\n");
Xtr_Dump(curr_xtr,&GStdOut);
#endif
  strcpy (curr_name, Sling_Name_Get (Name_Canonical_Get (Xtr_Name_Get (curr_xtr))));
#ifdef _DEBUG_
printf("\t\"%s\"\n",curr_name);

  for (i = 0, sling_found = FALSE; i < current_tsdnum && !sling_found; i++)
  {
    printf("i=%d current_tsdnum=%d\n",i,current_tsdnum);
#else

  for (i = 0, sling_found = FALSE; i < current_tsdnum && !sling_found; i++)
#endif
    if (Tsd_NumAtoms_Get (tsd_array[i]) > 1)
    {
    compare_xtr = Tsd2Xtr (tsd_array[i]);
    Xtr_Name_Set (compare_xtr, &map);
#ifdef _MIND_MEM_
    mind_Array_Destroy ("&map", "subgoalgeneration", &map);
#else
    Array_Destroy (&map);
#endif
    compare_name = Sling_Name_Get (Name_Canonical_Get (Xtr_Name_Get (compare_xtr)));

    if ((char *) strcmp (curr_name, (char *) compare_name) == 0) sling_found = TRUE;
    Xtr_Destroy (compare_xtr);
    }
#ifdef _DEBUG_
    printf("end i=%d\n",i);
  }
#endif

  Xtr_Destroy (curr_xtr);

  if (sling_found)
    {
#ifdef _MIND_MEM_
    mind_Array_Destroy ("can_map", "subgoalgeneration", can_map);
#else
    Array_Destroy (can_map);
#endif
#ifdef _DEBUG_
printf("\tTRUE\n");
#endif
    return (TRUE);
    }

  barpos=strstr(curr_name,"|");
  if (barpos!=NULL) *barpos='\0';
  strcpy (can_sling, curr_name);
  Sling_Name_Put (cansln,curr_name);
  Sling_Length_Put (cansln,strlen(curr_name));

  cantsd=Sling2Tsd_PlusHydrogen(cansln);
  canxtr=Tsd2Xtr(cantsd);
Xtr_Dump(canxtr,&GStdOut);
  Xtr_Name_Set(canxtr,&map);
  canname = Sling_Name_Get (Name_Canonical_Get (Xtr_Name_Get (canxtr)));

  natms=Tsd_NumAtoms_Get (cantsd);
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&longmap", "subgoalgeneration{17}", &longmap, natms, WORDSIZE);
  Array_Set (&longmap, TSD_INVALID);

  for (i=multatms=0; i<natms; i++)
    {
    for (j=mult=0; j<MX_NEIGHBORS; j++) if (Tsd_Atom_NeighborId_Get (cantsd, i, j) != TSD_INVALID) mult++;
    if (mult > 1)
      {
      Array_1d16_Put (&longmap, multatms, i);
      multatms++;
      }
    }

  mind_Array_1d_Create ("&backmap", "subgoalgeneration{17}", &backmap, multatms, WORDSIZE);
  Array_Set (&backmap, TSD_INVALID);

  for (i=j=0; i<multatms; i++, j++)
    {
    do
      {
      k=Array_1d16_Get (&map, j);
      if (k==TSD_INVALID) j++;
      }
    while (k==TSD_INVALID);
    Array_1d16_Put (&backmap, k, Array_1d16_Get (&longmap, i));
    }

  mind_Array_Destroy("&longmap", "subgoalgeneration", &longmap);

  orig_natms=Tsd_NumAtoms_Get (tsd_array[current_tsdnum]);

  for (i=0; i<orig_natms; i++)
    {
    j=Array_1d16_Get (can_map, i);
    if (j!=TSD_INVALID) Array_1d16_Put (can_map, i, Array_1d16_Get (&backmap, j));
    }

  mind_Array_Destroy("&backmap", "subgoalgeneration", &backmap);
#else
  Array_1d_Create (&longmap, natms, WORDSIZE);
  Array_Set (&longmap, TSD_INVALID);

  for (i=multatms=0; i<natms; i++)
    {
    for (j=mult=0; j<MX_NEIGHBORS; j++) if (Tsd_Atom_NeighborId_Get (cantsd, i, j) != TSD_INVALID) mult++;
    if (mult > 1)
      {
      Array_1d16_Put (&longmap, multatms, i);
      multatms++;
      }
    }

  Array_1d_Create (&backmap, multatms, WORDSIZE);
  Array_Set (&backmap, TSD_INVALID);

  for (i=j=0; i<multatms; i++, j++)
    {
    do
      {
      k=Array_1d16_Get (&map, j);
      if (k==TSD_INVALID) j++;
      }
    while (k==TSD_INVALID);
    Array_1d16_Put (&backmap, k, Array_1d16_Get (&longmap, i));
    }

  Array_Destroy(&longmap);

  orig_natms=Tsd_NumAtoms_Get (tsd_array[current_tsdnum]);

  for (i=0; i<orig_natms; i++)
    {
    j=Array_1d16_Get (can_map, i);
    if (j!=TSD_INVALID) Array_1d16_Put (can_map, i, Array_1d16_Get (&backmap, j));
    }

  Array_Destroy(&backmap);
#endif

  for (i=0; i<natms; i++) for (j=0; j<MX_NEIGHBORS; j++) nbr_used[i][j] = Tsd_Atom_NeighborId_Get (cantsd, i, j) == TSD_INVALID;

  for (i=0; i<natms; i++) if (Array_1d16_Get (can_map, i) == TSD_INVALID)
    {
    for (j=0; j<MX_NEIGHBORS; j++) if ((k=Tsd_Atom_NeighborId_Get (tsd_array[current_tsdnum], i, j)) != TSD_INVALID)
      {
      where=k;
      can_where = Array_1d16_Get (can_map, where);
      }

    for (j=0, nodemap_found=FALSE; j<MX_NEIGHBORS && !nodemap_found; j++) if (!nbr_used[can_where][j])
      {
      nbr = Tsd_Atom_NeighborId_Get (cantsd, can_where, j);

      if (Tsd_Atomid_Get (cantsd, nbr) == Tsd_Atomid_Get (tsd_array[current_tsdnum], i))
        {
        for (k=mult=0; k<MX_NEIGHBORS; k++) if (Tsd_Atom_NeighborId_Get (cantsd, nbr, k) != TSD_INVALID) mult++;
        if (mult == 1)
          {
          Array_1d16_Put (can_map, i, nbr);
          nodemap_found=nbr_used[can_where][j] = TRUE;
          }
        }
      }
    }

#ifdef _DEBUG_
printf("\tFALSE\n");
#endif
  return(FALSE);
}

static void      SNode_Destroy            (Match_Node_t *node_p)
{
  Match_Node_t *tnode_p;
/*
static int level=0;
*/

  if (node_p == NULL) return;
/*
printf("entering SNode_Destroy(%d)\n",level++);
*/
  if ((tnode_p = Match_Node_Brother_Get (node_p)) != NULL) SNode_Destroy (tnode_p);
  if ((tnode_p = Match_Node_Forward_Get (node_p)) != NULL) SNode_Destroy (tnode_p);
#ifdef _MIND_MEM_
  mind_free ("node_p", "subgoalgeneration", node_p);
#else
  Mem_Dealloc (node_p, MATCHNODESIZE, GLOBAL);
#endif
/*
printf("leaving SNode_Destroy(%d)\n",--level);
*/
}

static void      SLeaf_Destroy            (Match_Leaf_t *leaf_p)
{
  Match_Leaf_t *tleaf_p;
  Match_Node_t *tnode_p;
/*
static int level=0;
*/

  if (leaf_p == NULL) return;
/*
printf("entering SLeaf_Destroy(%d)\n",level++);
*/
  if ((tleaf_p = Match_Leaf_Next_Get (leaf_p)) != NULL) SLeaf_Destroy (tleaf_p);
/* already freed by SNode_Destroy()
  if ((tnode_p = Match_Leaf_Node_Get (leaf_p)) != NULL) SNode_Destroy (tnode_p);
*/
#ifdef _MIND_MEM_
  mind_free ("leaf_p", "subgoalgeneration", leaf_p);
#else
  Mem_Dealloc (leaf_p, MATCHLEAFSIZE, GLOBAL);
#endif
/*
printf("leaving SLeaf_Destroy(%d)\n",--level);
*/
}