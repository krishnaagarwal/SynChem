#ifndef _H_SUBGOALGENERATION_
#define _H_SUBGOALGENERATION_ 1
/******************************************************************************
*
*  Copyright (C) 1993-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     SUBGOALGENERATION.H
*
*    This module is the abstraction for subgoal generation.
*
*  Routines can be found in SUBGOALGENERATION.C
*
*  Creation Date:
*
*    01-Jan-1993
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

#ifndef _H_TSD_
#include "tsd.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_SGLSHOT_
#include "sglshot.h"
#endif

/*** Literals ***/

#define MAX_PERMS 12
#define MATCH_INVALID (U16_t)-203
#define TREE_COL  0
#define MATCH_COL 1
#define APPL_IDX  0
#define PIECE_IDX 1
#define SUBG_COL  0
#define ATOM_COL  1

#define MAX_PIECES_MATCHES 4
#define MAX_MATCHES_ONLY   2
#define MAX_DUPLICATES     0
#define EASE_PREF_SINGLE_FACTOR 4
#define EASE_PREF_MULTIPLE_FACTOR 2
#define MX_SUBGENR_HASH    257

/* This data-structure keeps track of potential (actual) matches found so
   far by traversing the goal and pattern XTRs.  Forward means going towards
   a longer match, back for shorter and brother for ones of the same length
   with the same prefix.
*/

typedef struct mch_nd
  {
  struct mch_nd *forward;                   /* Handle to next node forward */
  struct mch_nd *back;                      /* Handle to next node backward */
  struct mch_nd *brother;                   /* Handle to next brother node */
  U16_t         goal_link;                  /* Next atom index in goal */
  U16_t         pattern_link;               /* Next atom index in pattern */
  Boolean_t     success;                    /* Flag - is this a match? */
  } Match_Node_t;
#define MATCHNODESIZE sizeof (Match_Node_t)

/** Field Access Macros for Match_Node_t **/

/* Macro Prototypes
   Match_Node_t *Match_Node_Back_Get     (Match_Node_t *);
   void          Match_Node_Back_Put     (Match_Node_t *, Match_Node_t *);
   Match_Node_t *Match_Node_Brother_Get  (Match_Node_t *);
   void          Match_Node_Brother_Put  (Match_Node_t *, Match_Node_t *);
   Match_Node_t *Match_Node_Forward_Get  (Match_Node_t *);
   void          Match_Node_Forward_Put  (Match_Node_t *, Match_Node_t *);
   U16_t         Match_Node_GoalLink_Get (Match_Node_t *);
   void          Match_Node_GoalLink_Put (Match_Node_t *, U16_t);
   U16_t         Match_Node_PatternLink_Get (Match_Node_t *);
   void          Match_Node_PatternLink_Put (Match_Node_t *, U16_t);
   Boolean_t     Match_Node_Success_Get  (Match_Node_t *);
   void          Match_Node_Success_Put  (Match_Node_t *, Boolean_t);
*/

#ifndef SUBGENR_DEBUG
#define Match_Node_Back_Get(mnode_p)\
  (mnode_p)->back

#define Match_Node_Back_Put(mnode_p, value)\
  (mnode_p)->back = (value)

#define Match_Node_Brother_Get(mnode_p)\
  (mnode_p)->brother

#define Match_Node_Brother_Put(mnode_p, value)\
  (mnode_p)->brother = (value)

#define Match_Node_Forward_Get(mnode_p)\
  (mnode_p)->forward

#define Match_Node_Forward_Put(mnode_p, value)\
  (mnode_p)->forward = (value)

#define Match_Node_GoalLink_Get(mnode_p)\
  (mnode_p)->goal_link

#define Match_Node_GoalLink_Put(mnode_p, value)\
  (mnode_p)->goal_link = (value)

#define Match_Node_PatternLink_Get(mnode_p)\
  (mnode_p)->pattern_link

#define Match_Node_PatternLink_Put(mnode_p, value)\
  (mnode_p)->pattern_link = (value)

#define Match_Node_Success_Get(mnode_p)\
  (mnode_p)->success

#define Match_Node_Success_Put(mnode_p, value)\
  (mnode_p)->success = (value)
#else
#define Match_Node_Back_Get(mnode_p)\
  ((mnode_p) < GBAddr ? HALT : (mnode_p)->back)

#define Match_Node_Back_Put(mnode_p, value)\
  { if ((mnode_p) < GBAddr) HALT; else (mnode_p)->back = (value); }

#define Match_Node_Brother_Get(mnode_p)\
  ((mnode_p) < GBAddr ? HALT : (mnode_p)->brother)

#define Match_Node_Brother_Put(mnode_p, value)\
  { if ((mnode_p) < GBAddr) HALT; else (mnode_p)->brother = (value); }

#define Match_Node_Forward_Get(mnode_p)\
  ((mnode_p) < GBAddr ? HALT : (mnode_p)->forward)

#define Match_Node_Forward_Put(mnode_p, value)\
  { if ((mnode_p) < GBAddr) HALT; else (mnode_p)->forward = (value); }

#define Match_Node_GoalLink_Get(mnode_p)\
  ((mnode_p) < GBAddr ? HALT : (mnode_p)->goal_link)

#define Match_Node_GoalLink_Put(mnode_p, value)\
  { if ((mnode_p) < GBAddr) HALT; else (mnode_p)->goal_link = (value); }

#define Match_Node_PatternLink_Get(mnode_p)\
  ((mnode_p) < GBAddr ? HALT : (mnode_p)->pattern_link)

#define Match_Node_PatternLink_Put(mnode_p, value)\
  { if ((mnode_p) < GBAddr) HALT; else (mnode_p)->pattern_link = (value); }
num_duplicates
#define Match_Node_Success_Get(mnode_p)\
  ((mnode_p) < GBAddr ? HALT : (mnode_p)->success)

#define Match_Node_Success_Put(mnode_p, value)\
  { if ((mnode_p) < GBAddr) HALT; else (mnode_p)->success = (value); }
#endif

/** End of Field Access Macros for Match_Node_t **/

/* This data-structure keeps track of the leaf nodes of the actual match
   tree ???
*/

typedef struct mtch_lf
  {
  Match_Node_t   *node_p;                   /* Next match node */
  struct mtch_lf *leaf_p;                   /* Next leaf node */
  } Match_Leaf_t;
#define MATCHLEAFSIZE sizeof (Match_Leaf_t)

/** Field Access Macros for Match_Leaf_t **/

/* Macro Prototypes
   Match_Leaf_t *Match_Leaf_Next_Get (Match_Leaf_t *);
   void          Match_Leaf_Next_Put (Match_Leaf_t *, Match_Leaf_t *);
   Match_Node_t *Match_Leaf_Node_Get (Match_Leaf_t *);
   void          Match_Leaf_Node_Put (Match_Leaf_t *, Match_Node_t *);
*/

#ifndef SUBGENR_DEBUG
#define Match_Leaf_Next_Get(lnode_p)\
  (lnode_p)->leaf_p

#define Match_Leaf_Next_Put(lnode_p, value)\
  (lnode_p)->leaf_p = (value)

#define Match_Leaf_Node_Get(lnode_p)\
  (lnode_p)->node_p

#define Match_Leaf_Node_Put(lnode_p, value)\
  (lnode_p)->node_p = (value)
#else
#define Match_Leaf_Next_Get(lnode_p)\
  ((lnode_p) < GBAddr ? HALT : (lnode_p)->leaf_p)

#define Match_Leaf_Next_Put(lnode_p, value)\
  { if ((lnode_p) < GBAddr) HALT; else (lnode_p)->leaf_p = (value); }

#define Match_Leaf_Node_Get(lnode_p)\
  ((lnode_p) < GBAddr ? HALT : (lnode_p)->node_p)

#define Match_Leaf_Node_Put(lnode_p, value)\
  { if ((lnode_p) < GBAddr) HALT; else (lnode_p)->node_p = (value); }
#endif

/** End of Field Access Macros for Match_Leaf_t **/

/* This data-structure keeps all the parameters to the match activity
   together.  Most of these were global variables, but that would inhibit
   parallelizing the code.
*/

typedef struct s_subgenr_match
  {
  Xtr_t        *goal_p;                     /* Goal molecule handle */
  Xtr_t        *pattern_p;                  /* Pattern XTR handle */
  Xtr_t        *sgpattern_p;                /* Subgoal pattern XTR handle */
  Array_t      *stereoopt_p;                /* Stereo options array */
  Match_Node_t *root_p;                     /* Root of match tree */
  Match_Node_t *node_p;                     /* Head of match tree node list*/
  Match_Node_t *current_p;                  /* Current node to expand */
  Match_Leaf_t *leaf_p;                     /* Head of match leaf list */
  Array_t       goal_markb;                 /* 1d-bit, goal atom flags */
  Array_t       pattern_markb;              /* 1d-bit, pattern atom flags */
  Array_t       pivotb;                     /* 1d-bit, pivot nodes in tree */
  Array_t       pot_pivotb;                 /* 1d-bit, potential pivot nodes */
  Array_t       imageb;                     /* 1d-word, pattern->goal atomap */
  U16_t         goal_root;                  /* Root node in goal molecule */
  U16_t         pattern_root;               /* Root node in pattern XTR */
  U16_t         num_matches;                /* # complete matches */
  U16_t         depth;                      /* Matching depth so far */
  U16_t         conn_size;                  /* # nodes in patt. conn. comp. */
  Boolean_t     valence_exact;              /* Exact valence match rqmt */
  Boolean_t     only_one;                   /* Only one match flag */
  Boolean_t     any_stereo;                 /* Is stereoopts not all FALSE? */
  Boolean_t     alt_matches;                /* Are we looking for ? matches */
  } MatchCB_t;
#define MATCHCBSIZE sizeof (MatchCB_t)

/** Field Access Macros for MatchCB_t **/

/* Macro Prototypes
   Boolean_t     MatchCB_AlternateMatch_Get (MatchCB_t *);
   void          MatchCB_AlternateMatch_Put (MatchCB_t *, Boolean_t);
   Boolean_t     MatchCB_AnyStereo_Get    (MatchCB_t *);
   void          MatchCB_AnyStereo_Put    (MatchCB_t *, Boolean_t);
   U16_t         MatchCB_ConnectSize_Get  (MatchCB_t *);
   void          MatchCB_ConnectSize_Put  (MatchCB_t *, U16_t);
   Match_Node_t *MatchCB_CurrentNode_Get  (MatchCB_t *);
   void          MatchCB_CurrentNode_Put  (MatchCB_t *, Match_Node_t *);
   U16_t         MatchCB_Depth_Get        (MatchCB_t *);
   void          MatchCB_Depth_Put        (MatchCB_t *, U16_t);
   Xtr_t        *MatchCB_Goal_Get         (MatchCB_t *);
   void		 MatchCB_Goal_Put         (MatchCB_t *, Xtr_t *);
   Boolean_t     MatchCB_GoalMark_Get     (MatchCB_t *, U16_t);
   Array_t      *MatchCB_GoalMark_Handle  (MatchCB_t *);
   void          MatchCB_GoalMark_Put     (MatchCB_t *, U16_t, Boolean_t);
   U16_t         MatchCB_GoalRoot_Get     (MatchCB_t *);
   U16_t         MatchCB_Image_Get        (MatchCB_t *, U16_t);
   Array_t      *MatchCB_Image_Handle     (MatchCB_t *);
   void          MatchCB_Image_Put        (MatchCB_t *, U16_t, U16_t);
   Match_Leaf_t *MatchCB_Leaf_Get         (MatchCB_t *);
   void          MatchCB_Leaf_Put         (MatchCB_t *, Match_Leaf_t *);
   Match_Node_t *MatchCB_Node_Get         (MatchCB_t *);
   void          MatchCB_Node_Put         (MatchCB_t *, Match_Node_t *);
   U16_t         MatchCB_NumMatches_Get   (MatchCB_t *);
   void          MatchCB_NumMatches_Put   (MatchCB_t *, U16_t);
   Boolean_t     MatchCB_Only1Match_Get   (MatchCB_t *);
   Xtr_t        *MatchCB_Pattern_Get      (MatchCB_t *);
   Boolean_t     MatchCB_PatternMark_Get  (MatchCB_t *, U16_t);
   Array_t      *MatchCB_PatternMark_Handle (MatchCB_t *);
   void          MatchCB_PatternMark_Put  (MatchCB_t *, U16_t, Boolean_t);
   U16_t         MatchCB_PatternRoot_Get  (MatchCB_t *);
   Boolean_t     MatchCB_Pivot_Get        (MatchCB_t *, U16_t);
   Array_t      *MatchCB_Pivot_Handle     (MatchCB_t *);
   void          MatchCB_Pivot_Put        (MatchCB_t *, U16_t, Boolean_t);
   Boolean_t     MatchCB_PivotTry_Get     (MatchCB_t *, U16_t);
   Array_t      *MatchCB_PivotTry_Handle  (MatchCB_t *);
   void          MatchCB_PivotTry_Put     (MatchCB_t *, U16_t, Boolean_t);
   Match_Node_t *MatchCB_Root_Get         (MatchCB_t *);
   void          MatchCB_Root_Put         (MatchCB_t *, Match_Node_t *);
   Xtr_t        *MatchCB_SGPattern_Get    (MatchCB_t *);
   Boolean_t     MatchCB_StereoOption_Get (MatchCB_t *, U16_t);
   void          MatchCB_StereoOption_Put (MatchCB_t *, U16_t, Boolean_t);
   Boolean_t     MatchCB_ValenceExact_Get (MatchCB_t *);
*/

#ifndef SUBGENR_DEBUG
#define MatchCB_AlternateMatch_Get(mcb_p)\
  (mcb_p)->alt_matches

#define MatchCB_AlternateMatch_Put(mcb_p, value)\
  (mcb_p)->alt_matches = (value)

#define MatchCB_AnyStereo_Get(mcb_p)\
  (mcb_p)->any_stereo

#define MatchCB_AnyStereo_Put(mcb_p, value)\
  (mcb_p)->any_stereo = (value)

#define MatchCB_ConnectSize_Get(mcb_p)\
  (mcb_p)->conn_size

#define MatchCB_ConnectSize_Put(mcb_p, value)\
  (mcb_p)->conn_size = (value)

#define MatchCB_CurrentNode_Get(mcb_p)\
  (mcb_p)->current_p

#define MatchCB_CurrentNode_Put(mcb_p, value)\
  (mcb_p)->current_p = (value)

#define MatchCB_Depth_Get(mcb_p)\
  (mcb_p)->depth

#define MatchCB_Depth_Put(mcb_p, value)\
  (mcb_p)->depth = (value)

#define MatchCB_Goal_Get(mcb_p)\
  (mcb_p)->goal_p

#define MatchCB_Goal_Put(mcb_p, value)\
  (mcb_p)->goal_p = (value)

#define MatchCB_GoalMark_Get(mcb_p, atom)\
  Array_1d1_Get (&(mcb_p)->goal_markb, atom)

#define MatchCB_GoalMark_Handle(mcb_p)\
  &(mcb_p)->goal_markb

#define MatchCB_GoalMark_Put(mcb_p, atom, value)\
  Array_1d1_Put (&(mcb_p)->goal_markb, atom, value)

#define MatchCB_GoalRoot_Get(mcb_p)\
  (mcb_p)->goal_root

#define MatchCB_Image_Get(mcb_p, atom)\
  Array_1d16_Get (&(mcb_p)->imageb, atom)

#define MatchCB_Image_Handle(mcb_p)\
  &(mcb_p)->imageb

#define MatchCB_Image_Put(mcb_p, atom, value)\
  Array_1d16_Put (&(mcb_p)->imageb, atom, value)

#define MatchCB_Leaf_Get(mcb_p)\
  (mcb_p)->leaf_p

#define MatchCB_Leaf_Put(mcb_p, value)\
  (mcb_p)->leaf_p = (value)

#define MatchCB_Node_Get(mcb_p)\
  (mcb_p)->node_p

#define MatchCB_Node_Put(mcb_p, value)\
  (mcb_p)->node_p = (value)

#define MatchCB_NumMatches_Get(mcb_p)\
  (mcb_p)->num_matches

#define MatchCB_NumMatches_Put(mcb_p, value)\
  (mcb_p)->num_matches = (value)

#define MatchCB_Only1Match_Get(mcb_p)\
  (mcb_p)->only_one

#define MatchCB_Pattern_Get(mcb_p)\
  (mcb_p)->pattern_p

#define MatchCB_PatternMark_Get(mcb_p, atom)\
  Array_1d1_Get (&(mcb_p)->pattern_markb, atom)

#define MatchCB_PatternMark_Handle(mcb_p)\
  &(mcb_p)->pattern_markb

#define MatchCB_PatternMark_Put(mcb_p, atom, value)\
  Array_1d1_Put (&(mcb_p)->pattern_markb, atom, value)

#define MatchCB_PatternRoot_Get(mcb_p)\
  (mcb_p)->pattern_root

#define MatchCB_Pivot_Get(mcb_p, atom)\
  Array_1d1_Get (&(mcb_p)->pivotb, atom)

#define MatchCB_Pivot_Handle(mcb_p)\
  &(mcb_p)->pivotb

#define MatchCB_Pivot_Put(mcb_p, atom, value)\
  Array_1d1_Put (&(mcb_p)->pivotb, atom, value)

#define MatchCB_PivotTry_Get(mcb_p, atom)\
  Array_1d1_Get (&(mcb_p)->pot_pivotb, atom)

#define MatchCB_PivotTry_Handle(mcb_p)\
  &(mcb_p)->pot_pivotb

#define MatchCB_PivotTry_Put(mcb_p, atom, value)\
  Array_1d1_Put (&(mcb_p)->pot_pivotb, atom, value)

#define MatchCB_Root_Get(mcb_p)\
  (mcb_p)->root_p

#define MatchCB_Root_Put(mcb_p, value)\
  (mcb_p)->root_p = (value)

#define MatchCB_StereoOption_Get(mcb_p, atom)\
  Array_1d1_Get ((mcb_p)->stereoopt_p, atom)

#define MatchCB_StereoOption_Put(mcb_p, atom, value)\
  Array_1d1_Put ((mcb_p)->stereoopt_p, atom, value)

#define MatchCB_SGPattern_Get(mcb_p)\
  (mcb_p)->sgpattern_p

#define MatchCB_ValenceExact_Get(mcb_p)\
  (mcb_p)->valence_exact
#else
#define MatchCB_AlternateMatch_Get(mcb_p)\
  ((mcb_p) < GBAddr ? HALT : (mcb_p)->alt_matches)

#define MatchCB_AlternateMatch_Put(mcb_p, value)\
  { if ((mcb_p) < GBAddr) HALT; else (mcb_p)->alt_matches = (value); }

#define MatchCB_AnyStereo_Get(mcb_p)\
  ((mcb_p) < GBAddr ? HALT : (mcb_p)->any_stereo)

#define MatchCB_AnyStereo_Put(mcb_p, value)\
  { if ((mcb_p) < GBAddr) HALT; else (mcb_p)->any_stereo = (value); }

#define MatchCB_ConnectSize_Get(mcb_p)\
  ((mcb_p) < GBAddr ? HALT : (mcb_p)->conn_size)

#define MatchCB_ConnectSize_Put(mcb_p, value)\
  { if ((mcb_p) < GBAddr) HALT; else (mcb_p)->conn_size = (value); }

#define MatchCB_CurrentNode_Get(mcb_p)\
  ((mcb_p) < GBAddr ? HALT : (mcb_p)->current_p)

#define MatchCB_CurrentNode_Put(mcb_p, value)\
  { if ((mcb_p) < GBAddr) HALT; else (mcb_p)->current_p = (value); }

#define MatchCB_Depth_Get(mcb_p)\
  ((mcb_p) < GBAddr ? HALT : (mcb_p)->depth)

#define MatchCB_Depth_Put(mcb_p, value)\
  { if ((mcb_p) < GBAddr) HALT; else (mcb_p)->depth = (value); }

#define MatchCB_Goal_Get(mcb_p)\
  ((mcb_p) < GBAddr ? HALT : (mcb_p)->goal_p)

#define MatchCB_Goal_Put(mcb_p, value)\
  { if ((mcb_p) < GBAddr) HALT; else (mcb_p)->goal_p = (value);}

#define MatchCB_GoalMark_Get(mcb_p, atom)\
  ((mcb_p) < GBAddr ? HALT : Array_1d1_Get (&(mcb_p)->goal_markb, atom))

#define MatchCB_GoalMark_Handle(mcb_p)\
  ((mcb_p) < GBAddr ? HALT : &(mcb_p)->goal_markb)

#define MatchCB_GoalMark_Put(mcb_p, atom, value)\
  { if ((mcb_p) < GBAddr) HALT; else Array_1d1_Put (&(mcb_p)->goal_markb,\
  atom, value); }

#define MatchCB_GoalRoot_Get(mcb_p)\
  ((mcb_p) < GBAddr ? HALT : (mcb_p)->goal_root)

#define MatchCB_Image_Get(mcb_p, atom)\
  ((mcb_p) < GBAddr ? HALT : Array_1d16_Get (&(mcb_p)->imageb, atom))

#define MatchCB_Image_Handle(mcb_p)\
  ((mcb_p) < GBAddr ? HALT : &(mcb_p)->imageb)

#define MatchCB_Image_Put(mcb_p, atom, value)\
  { if ((mcb_p) < GBAddr) HALT; else Array_1d16_Put (&(mcb_p)->imageb, atom,\
  value); }

#define MatchCB_Leaf_Get(mcb_p)\
  ((mcb_p) < GBAddr ? HALT : (mcb_p)->leaf_p)

#define MatchCB_Leaf_Put(mcb_p, value)\
  { if ((mcb_p) < GBAddr) HALT; else (mcb_p)->leaf_p = (value); }

#define MatchCB_Node_Get(mcb_p)\
  ((mcb_p) < GBAddr ? HALT : (mcb_p)->node_p)

#define MatchCB_Node_Put(mcb_p, value)\
  { if ((mcb_p) < GBAddr) HALT; else (mcb_p)->node_p = (value); }

#define MatchCB_NumMatches_Get(mcb_p)\
  ((mcb_p) < GBAddr ? HALT : (mcb_p)->num_matches)

#define MatchCB_NumMatches_Put(mcb_p, value)\
  { if ((mcb_p) < GBAddr) HALT; else (mcb_p)->num_matches = (value); }

#define MatchCB_Only1Match_Get(mcb_p)\
  ((mcb_p) < GBAddr ? HALT : (mcb_p)->only_one)

#define MatchCB_Pattern_Get(mcb_p)\
  ((mcb_p) < GBAddr ? HALT : (mcb_p)->pattern_p)

#define MatchCB_PatternMark_Get(mcb_p, atom)\
  ((mcb_p) < GBAddr ? HALT : Array_1d1_Get (&(mcb_p)->pattern_markb, atom))

#define MatchCB_PatternMark_Handle(mcb_p)\
  ((mcb_p) < GBAddr ? HALT : &(mcb_p)->pattern_markb)

#define MatchCB_PatternMark_Put(mcb_p, atom, value)\
  { if ((mcb_p) < GBAddr) HALT; else Array_1d1_Put (&(mcb_p)->pattern_markb,\
  atom, value); }

#define MatchCB_PatternRoot_Get(mcb_p)\
  ((mcb_p) < GBAddr ? HALT : (mcb_p)->pattern_root)

#define MatchCB_Pivot_Get(mcb_p, atom)\
  ((mcb_p) < GBAddr ? HALT : Array_1d1_Get (&(mcb_p)->pivotb, atom))

#define MatchCB_Pivot_Handle(mcb_p)\
  ((mcb_p) < GBAddr ? HALT : &(mcb_p)->pivotb)

#define MatchCB_Pivot_Put(mcb_p, atom, value)\
  { if ((mcb_p) < GBAddr) HALT; else Array_1d1_Put (&(mcb_p)->pivotb, atom,\
  value); }

#define MatchCB_PivotTry_Get(mcb_p, atom)\
  ((mcb_p) < GBAddr ? HALT : Array_1d1_Get (&(mcb_p)->pot_pivotb, atom))

#define MatchCB_PivotTry_Handle(mcb_p)\
  ((mcb_p) < GBAddr ? HALT : &(mcb_p)->pot_pivotb)

#define MatchCB_PivotTry_Put(mcb_p, atom, value)\
  { if ((mcb_p) < GBAddr) HALT; else Array_1d1_Put (&(mcb_p)->pot_pivotb,\
  atom, value); }

#define MatchCB_Root_Get(mcb_p)\
  ((mcb_p) < GBAddr ? HALT : (mcb_p)->root_p)

#define MatchCB_Root_Put(mcb_p, value)\
  { if ((mcb_p) < GBAddr) HALT; else (mcb_p)->root_p = (value); }

#define MatchCB_StereoOption_Get(mcb_p, atom)\
  ((mcb_p) < GBAddr ? HALT : Array_1d1_Get ((mcb_p)->stereoopt_p, atom))

#define MatchCB_StereoOption_Put(mcb_p, atom, value)\
  { if ((mcb_p) < GBAddr) HALT; else Array_1d1_Put ((mcb_p)->stereoopt_p,\
  atom, value); }

#define MatchCB_SGPattern_Get(mcb_p)\
  ((mcb_p) < GBAddr ? HALT : (mcb_p)->sgpattern_p)

#define MatchCB_ValenceExact_Get(mcb_p)\
  ((mcb_p) < GBAddr ? HALT : (mcb_p)->valence_exact)
#endif

/** End of Field Access Macros for MatchCB_t **/

/* This data-structure describes the matches made to each piece in the
   goal pattern schema.

   Note: there used to be fields for last node selected and for the pattern
   root node, but no uses could be found.
*/

typedef struct mtch_piece
  {
  Array_t       tree2matchnumb;             /* Mapping of trees to matches */
  U16_t         num_matches;                /* # matches in treemap */
  } Match_Piece_t;
#define MATCHPIECESIZE sizeof (Match_Piece_t)

/** Field Access Macros for Match_Piece_t **/

/* Macro Prototypes
   U16_t    Match_Piece_Match_Get        (Match_Piece_t *, U16_t);
   void     Match_Piece_Match_Put        (Match_Piece_t *, U16_t, U16_t);
   U16_t    Match_Piece_NumMatches_Get   (Match_Piece_t *);
   void     Match_Piece_NumMatches_Get   (Match_Piece_t *, U16_t);
   U16_t    Match_Piece_Tree_Get         (Match_Piece_t *, U16_t);
   void     Match_Piece_Tree_Put         (Match_Piece_t *, U16_t, U16_t);
*/

#ifndef SUBGENR_DEBUG
#define Match_Piece_Match_Get(mp_p, index)\
  Array_2d16_Get (&(mp_p)->tree2matchnumb, index, MATCH_COL)

#define Match_Piece_Match_Put(mp_p, index, value)\
  Array_2d16_Put (&(mp_p)->tree2matchnumb, index, MATCH_COL, value)

#define Match_Piece_NumMatches_Get(mp_p)\
  (mp_p)->num_matches

#define Match_Piece_NumMatches_Put(mp_p, value)\
  (mp_p)->num_matches = (value)

#define Match_Piece_Tree_Get(mp_p, index)\
  Array_2d16_Get (&(mp_p)->tree2matchnumb, index, TREE_COL)

#define Match_Piece_Tree_Put(mp_p, index, value)\
  Array_2d16_Put (&(mp_p)->tree2matchnumb, index, TREE_COL, value)
#else
#define Match_Piece_Match_Get(mp_p, index)\
  ((mp_p) < GBAddr ? HALT : Array_2d16_Get (&(mp_p)->tree2matchnumb, index,\
  MATCH_COL))

#define Match_Piece_Match_Put(mp_p, index, value)\
  { if ((mp_p) < GBAddr) HALT; else Array_2d16_Put (&(mp_p)->tree2matchnumb,\
  index, MATCH_COL, value); }

#define Match_Piece_NumMatches_Get(mp_p)\
  ((mp_p) < GBAddr ? HALT : (mp_p)->num_matches)

#define Match_Piece_NumMatches_Put(mp_p, value)\
  { if ((mp_p) < GBAddr) HALT; else (mp_p)->num_matches = (value); }

#define Match_Piece_Tree_Get(mp_p, index)\
  ((mp_p) < GBAddr ? HALT : Array_2d16_Get (&(mp_p)->tree2matchnumb, index,\
  TREE_COL))

#define Match_Piece_Tree_Put(mp_p, index, value)\
  { if ((mp_p) < GBAddr) HALT; else Array_2d16_Put (&(mp_p)->tree2matchnumb,\
  index, TREE_COL, value); }
#endif

/** End of Field Access Macros for Match_Piece_t **/

/* This data-structure keeps track of the compounds from the matching
   process.
*/

typedef struct mtch_prods
  {
  Xtr_t        *xtr_p;                      /* Compound in XTR format */
  Tsd_t        *tsd_p;                      /* Compound in TSD format */
  Array_t       atomsb;                     /* 1d bit, which atoms are mapped
    */
  U16_t         ease;                       /* Cumulative ease seen */
  U16_t         yield;                      /* Cumulative yield seen */
  U16_t         confidence;                 /* Cumulative confidence seen */
  U16_t         num_compounds;              /* # compounds so far */
  Boolean_t     strategic;                  /* Strategic bond matched */
  } SubGenr_Compound_t;
#define SUBGENRCOMPOUNDSIZE sizeof (SubGenr_Compound_t)

/** Field Access Macros for SubGenr_Compound_t **/

/* Macro Prototypes
   U16_t     SubGenr_Compound_Atom_Get    (SubGenr_Compound_t *, U16_t);
   void      SubGenr_Compound_Atom_Put    (SubGenr_Compound_t *, U16_t, U16_t);
   Array_t  *SubGenr_Compound_AtomHandle_Get   (SubGenr_Compound_t *);
   U16_t     SubGenr_Compound_Confidence_Get   (SubGenr_Compound_t *);
   void      SubGenr_Compound_Confidence_Put   (SubGenr_Compound_t *, U16_t);
   U16_t     SubGenr_Compound_Ease_Get         (SubGenr_Compound_t *);
   void      SubGenr_Compound_Ease_Put         (SubGenr_Compound_t *, U16_t);
   U16_t     SubGenr_Compound_NumCompounds_Get (SubGenr_Compound_t *);
   void      SubGenr_Compound_NumCompounds_Put (SubGenr_Compound_t *, U16_t);
   Boolean_t SubGenr_Compound_Strategic_Get  (SubGenr_Compound_t *);
   void      SubGenr_Compound_Strategic_Put  (SubGenr_Compound_t *, Boolean_t);
   Tsd_t    *SubGenr_Compound_Tsd_Get          (SubGenr_Compound_t *);
   void      SubGenr_Compound_Tsd_Put          (SubGenr_Compound_t *, Tsd_t *);
   Xtr_t    *SubGenr_Compound_Xtr_Get          (SubGenr_Compound_t *);
   void      SubGenr_Compound_Xtr_Put          (SubGenr_Compound_t *, Xtr_t *);
   U16_t     SubGenr_Compound_Yield_Get        (SubGenr_Compound_t *);
   void      SubGenr_Compound_Yield_Put        (SubGenr_Compound_t *, U16_t);
*/

#ifndef SUBGENR_DEBUG
#define SubGenr_Compound_Atom_Get(sgpr_p, index)\
  Array_1d1_Get (&(sgpr_p)->atomsb, index)

#define SubGenr_Compound_Atom_Put(sgpr_p, index, value)\
  Array_1d1_Put (&(sgpr_p)->atomsb, index, value)

#define SubGenr_Compound_AtomHandle_Get(sgpr_p)\
  &(sgpr_p)->atomsb

#define SubGenr_Compound_Confidence_Get(sgpr_p)\
  (sgpr_p)->confidence

#define SubGenr_Compound_Confidence_Put(sgpr_p, value)\
  (sgpr_p)->confidence = (value)

#define SubGenr_Compound_Ease_Get(sgpr_p)\
  (sgpr_p)->ease

#define SubGenr_Compound_Ease_Put(sgpr_p, value)\
  (sgpr_p)->ease = (value)

#define SubGenr_Compound_NumCompounds_Get(sgpr_p)\
  (sgpr_p)->num_compounds

#define SubGenr_Compound_NumCompounds_Put(sgpr_p, value)\
  (sgpr_p)->num_compounds = (value)

#define SubGenr_Compound_Strategic_Get(sgpr_p)\
  (sgpr_p)->strategic

#define SubGenr_Compound_Strategic_Put(sgpr_p, value)\
  (sgpr_p)->strategic = (value)

#define SubGenr_Compound_Tsd_Get(sgpr_p)\
  (sgpr_p)->tsd_p

#define SubGenr_Compound_Tsd_Put(sgpr_p, value)\
  (sgpr_p)->tsd_p = (value)

#define SubGenr_Compound_Xtr_Get(sgpr_p)\
  (sgpr_p)->xtr_p

#define SubGenr_Compound_Xtr_Put(sgpr_p, value)\
  (sgpr_p)->xtr_p = (value)

#define SubGenr_Compound_Yield_Get(sgpr_p)\
  (sgpr_p)->yield

#define SubGenr_Compound_Yield_Put(sgpr_p, value)\
  (sgpr_p)->yield = (value)
#else
#define SubGenr_Compound_Atom_Get(sgpr_p, index)\
  ((sgpr_p) < GBAddr ? HALT : Array_1d1_Get (&(sgpr_p)->atomsb, index))

#define SubGenr_Compound_Atom_Put(sgpr_p, index, value)\
  { if ((sgpr_p) < GBAddr) HALT; else Array_1d1_Put (&(sgpr_p)->atomsb,\
  index, value); }

#define SubGenr_Compound_AtomHandle_Get(sgpr_p)\
  ((sgpr_p) < GBAddr ? HALT : &(sgpr_p)->atomsb)

#define SubGenr_Compound_Confidence_Get(sgpr_p)\
  ((sgpr_p) < GBAddr ? HALT : (sgpr_p)->confidence)

#define SubGenr_Compound_Confidence_Put(sgpr_p, value)\
  { if ((sgpr_p) < GBAddr) HALT; else (sgpr_p)->confidence = (value); }

#define SubGenr_Compound_Ease_Get(sgpr_p)\
  ((sgpr_p) < GBAddr ? HALT : (sgpr_p)->ease)

#define SubGenr_Compound_Ease_Put(sgpr_p, value)\
  { if ((sgpr_p) < GBAddr) HALT; else (sgpr_p)->ease = (value); }

#define SubGenr_Compound_NumCompounds_Get(sgpr_p)\
  ((sgpr_p) < GBAddr ? HALT : (sgpr_p)->num_compounds)

#define SubGenr_Compound_NumCompounds_Put(sgpr_p, value)\
  { if ((sgpr_p) < GBAddr) HALT; else (sgpr_p)->num_compounds = (value); }

#define SubGenr_Compound_Strategic_Get(sgpr_p)\
  ((sgpr_p) < GBAddr ? HALT : (sgpr_p)->strategic)

#define SubGenr_Compound_Strategic_Put(sgpr_p, value)\
  { if ((sgpr_p) < GBAddr) HALT; else (sgpr_p)->strategic = (value); }

#define SubGenr_Compound_Tsd_Get(sgpr_p)\
  ((sgpr_p) < GBAddr ? HALT : (sgpr_p)->tsd_p)

#define SubGenr_Compound_Tsd_Put(sgpr_p, value)\
  { if ((sgpr_p) < GBAddr) HALT; else (sgpr_p)->tsd_p = (value); }

#define SubGenr_Compound_Xtr_Get(sgpr_p)\
  ((sgpr_p) < GBAddr ? HALT : (sgpr_p)->xtr_p)

#define SubGenr_Compound_Xtr_Put(sgpr_p, value)\
  { if ((sgpr_p) < GBAddr) HALT; else (sgpr_p)->xtr_p = (value); }

#define SubGenr_Compound_Yield_Get(sgpr_p)\
  ((sgpr_p) < GBAddr ? HALT : (sgpr_p)->yield)

#define SubGenr_Compound_Yield_Put(sgpr_p, value)\
  { if ((sgpr_p) < GBAddr) HALT; else (sgpr_p)->yield = (value); }
#endif

/** End of Field Access Macros for SubGenr_Compound_t **/

/*** Routine Prototypes ***/

Boolean_t  Duplicates_Check          (Sling_t *, Array_t *, String_t *, 
             U16_t, Boolean_t);
void       Match_Tree_Dump           (Match_Node_t *, FileDsc_t *);
MatchCB_t *SubGenr_Active_Match      (Xtr_t *, Xtr_t *, Xtr_t *, Array_t *, 
             U16_t, U16_t, Boolean_t);
MatchCB_t *SubGenr_Fragment_Match    (Xtr_t *, Xtr_t *, Array_t *, Array_t *,
             U16_t, U16_t, Boolean_t);
MatchCB_t *SubGenr_MatchCB_Create    (void);
void       SubGenr_MatchCB_Destroy   (MatchCB_t *);
void       SubGenr_Subgoals_Generate (Xtr_t *, Array_t *, Array_t *, U32_t,
             Boolean_t, Boolean_t, SShotGenSGs_t *);

#ifdef _GLOB_SUBGENR_DEF_
Boolean_t subgenr_interactive = FALSE;
#else
extern Boolean_t subgenr_interactive;
#endif

/* End of Subgoalgenerate.h */
#endif

