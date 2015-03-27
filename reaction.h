#ifndef _H_REACTION_
#define _H_REACTION_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     REACTION.H
*
*    This module is the .H file for the reaction library record data 
*    abstraction.  A reaction is split between two files, the Reaction
*    data file and the Reaction text file.  The former contains the 
*    information that Synchem needs for runtime access, ie transform
*    patterns, pre- and post-tests.  The latter contains update histories
*    literature references, post-test rationale, etc.
*
*    Routines can be found in REACTION.C except where noted
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
* 14-Sep-95  Cheung	Added field library in structure React_Head_t and
*			corresponding macros.
******************************************************************************/

#ifndef _H_ARRAY_
#include "array.h"
#endif

#ifndef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_ISAM_
#include "isam.h"
#endif

#ifndef _H_TSD_
#include "tsd.h"
#endif

#ifndef _H_POSTTEST_
#include "posttest.h"
#endif

/*** Literal values ***/

#define RXN_BUFFER_SIZE       1024
#define MX_ROOTS              4
#define REACT_NODE_INVALID    (U8_t)-104

#define REACT_SING_APPL_ONLY  11   /* 1 */
#define REACT_MULT_PREF_SING  12   /* 2 */
#define REACT_MULT_NO_PREF    13   /* 10 */
#define REACT_MULT_PREF_MULT  14   /* 100 */
#define REACT_MAX_APPL_ONLY   15   /* 10000 */

#define REACT_DATANAME "/rkbdata.isam"
#define REACT_TEXTNAME "/rkbtext.isam"

#define MX_CONFIDENCE  100
#define MX_YIELD       100
#define MX_EASE        100

#define INVALID_LIB_NUM 0
#define BAS_LIB_NAME	"bas.lib"
#define BAS_LIB_NUM	1
#define DEV_LIB_NAME	"dev.lib"
#define DEV_LIB_NUM	2

/*** Data-structures ***/

/* Reaction data file record header.  This contains all the fixed size
   information.
*/

typedef struct s_reacthead
  {
  U32_t         schema_num;                /* Unique number for this schema */
  struct s_roots
    {
    U8_t        node;                      /* Root node atom */
    U8_t        syntheme;                  /* Sytheme group */
    } roots[MX_ROOTS];                     /* Array of syntheme roots */
  U16_t         flags;                     /* Reaction flags */
  U16_t         ease;                      /* Initial ease of reaction */
  U16_t         confidence;                /* Initial confidence of reaction */
  U8_t          yield;                     /* Initial yield of reaction */
  U8_t          num_conditions;            /* Number of conditions in array */
  U8_t          num_tests;                 /* Number of tests in array */
  U8_t          react_type;                /* Reaction type */
  U8_t          max_nonident;              /* Max # non-identical sub-goals */
  U8_t          goal_atoms;                /* # atoms in goal TSD */
  U8_t          subgoal_atoms;             /* # atoms in subgoal TSD */
  U8_t          goal_pattern_size;         /* # atoms in goal pattern */
  U8_t          syntheme_fg;               /* Original chapter - primary FG */
  U8_t          library;		   /* Original library # */
  } React_Head_t;
#define REACTHEADSIZE sizeof (React_Head_t)

/* Reaction type flags */

#define ReactM_Protection    0x1
#define ReactM_Lookahead     0x2
#define ReactM_Disabled      0x4
#define ReactM_Incomplete    0x8
#define ReactM_MustAll       0x10
#define ReactM_MustAny       0x20
#define ReactM_CantAll       0x40
#define ReactM_CantAny       0x80

/** Field Access Macros for React_Head_t **/

/* Macro Prototypes
   Boolean_t React_HeadFlags_CantAll_Get    (React_Head_t *);
   void      React_HeadFlags_CantAll_Put    (React_Head_t *, Boolean_t);
   Boolean_t React_HeadFlags_CantAny_Get    (React_Head_t *);
   void      React_HeadFlags_CantAny_Put    (React_Head_t *, Boolean_t);
   Boolean_t React_HeadFlags_Disabled_Get   (React_Head_t *);
   void      React_HeadFlags_Disabled_Put   (React_Head_t *, Boolean_t);
   Boolean_t React_HeadFlags_Incomplete_Get (React_Head_t *);
   void      React_HeadFlags_Incomplete_Put (React_Head_t *, Boolean_t);
   Boolean_t React_HeadFlags_Lookahead_Get  (React_Head_t *);
   void      React_HeadFlags_Lookahead_Put  (React_Head_t *, Boolean_t);
   Boolean_t React_HeadFlags_MustAll_Get    (React_Head_t *);
   void      React_HeadFlags_MustAll_Put    (React_Head_t *, Boolean_t);
   Boolean_t React_HeadFlags_MustAny_Get    (React_Head_t *);
   void      React_HeadFlags_MustAny_Put    (React_Head_t *, Boolean_t);
   Boolean_t React_HeadFlags_Protection_Get (React_Head_t *);
   void      React_HeadFlags_Protection_Put (React_Head_t *, Boolean_t);
   Boolean_t React_HeadFlags_Unknown1_Get   (React_Head_t *);
   void      React_HeadFlags_Unknown1_Put   (React_Head_t *, Boolean_t);
   U8_t      React_Head_Confidence_Get      (React_Head_t *);
   void      React_Head_Confidence_Put      (React_Head_t *, U8_t);
   U8_t      React_Head_Ease_Get            (React_Head_t *);
   void      React_Head_Ease_Put            (React_Head_t *, U8_t);
   U16_t     React_Head_Flags_Get           (React_Head_t *);
   void      React_Head_Flags_Put           (React_Head_t *, U16_t);
   U8_t      React_Head_GoalSize_Get        (React_Head_t *);
   void      React_Head_GoalSize_Put        (React_Head_t *, U8_t);
   U8_t	     React_Head_Library_Get	    (React_Head_t *);
   void      React_Head_Library_Put	    (React_Head_t *, U8_t);
   U8_t      React_Head_MaxNonident_Get     (React_Head_t *);
   void      React_Head_MaxNonident_Put     (React_Head_t *, U8_t);
   U8_t      React_Head_NumConditions_Get   (React_Head_t *);
   void      React_Head_NumConditions_Put   (React_Head_t *, U8_t);
   U8_t      React_Head_NumGoalAtoms_Get    (React_Head_t *);
   void      React_Head_NumGoalAtoms_Put    (React_Head_t *, U8_t);
   U8_t      React_Head_NumSubgoalAtoms_Get (React_Head_t *);
   void      React_Head_NumSubGoalAtoms_Put (React_Head_t *, U8_t);
   U8_t      React_Head_NumTests_Get        (React_Head_t *);
   void      React_Head_NumTests_Put        (React_Head_t *, U8_t);
   U8_t      React_Head_ReactionType_Get    (React_Head_t *);
   void      React_Head_ReactionType_Put    (React_Head_t *, U8_t);
   U8_t      React_Head_RootNode_Get        (React_Head_t *, U8_t);
   void      React_Head_RootNode_Put        (React_Head_t *, U8_t, U8_t);
   U8_t      React_Head_RootSyntheme_Get    (React_Head_t *, U8_t);
   void      React_Head_RootSyntheme_Put    (React_Head_t *, U8_t, U8_t);
   U32_t     React_Head_SchemaId_Get        (React_Head_t *);
   void      React_Head_SchemaId_Put        (React_Head_t *, U32_t);
   U8_t      React_Head_SynthemeFG_Get      (React_Head_t *);
   void      React_Head_SynthemeFG_Put      (React_Head_t *, U8_t);
   U8_t      React_Head_Yield_Get           (React_Head_t *);
   void      React_Head_Yield_Put           (React_Head_t *, U8_t);
*/

#ifndef REACT_DEBUG
#define React_HeadFlags_CantAll_Get(rxhd_p)\
  ((rxhd_p)->flags & ReactM_CantAll ? TRUE : FALSE)

#define React_HeadFlags_CantAll_Put(rxhd_p, value)\
  { if ((value) == TRUE)\
    (rxhd_p)->flags |= ReactM_CantAll;\
  else\
    (rxhd_p)->flags &= ~ReactM_CantAll; }

#define React_HeadFlags_CantAny_Get(rxhd_p)\
  ((rxhd_p)->flags & ReactM_CantAny ? TRUE : FALSE)

#define React_HeadFlags_CantAny_Put(rxhd_p, value)\
  { if ((value) == TRUE)\
    (rxhd_p)->flags |= ReactM_CantAny;\
  else\
    (rxhd_p)->flags &= ~ReactM_CantAny; }

#define React_HeadFlags_Disabled_Get(rxhd_p)\
  ((rxhd_p)->flags & ReactM_Disabled ? TRUE : FALSE)

#define React_HeadFlags_Disabled_Put(rxhd_p, value)\
  { if ((value) == TRUE)\
    (rxhd_p)->flags |= ReactM_Disabled;\
  else\
    (rxhd_p)->flags &= ~ReactM_Disabled; }

#define React_HeadFlags_Incomplete_Get(rxhd_p)\
  ((rxhd_p)->flags & ReactM_Incomplete ? TRUE : FALSE)

#define React_HeadFlags_Incomplete_Put(rxhd_p, value)\
  { if ((value) == TRUE)\
    (rxhd_p)->flags |= ReactM_Incomplete;\
  else\
    (rxhd_p)->flags &= ~ReactM_Incomplete; }

#define React_HeadFlags_Lookahead_Get(rxhd_p)\
  ((rxhd_p)->flags & ReactM_Lookahead ? TRUE : FALSE)

#define React_HeadFlags_Lookahead_Put(rxhd_p, value)\
  { if ((value) == TRUE)\
    (rxhd_p)->flags |= ReactM_Lookahead;\
  else\
    (rxhd_p)->flags &= ~ReactM_Lookahead; }

#define React_HeadFlags_MustAll_Get(rxhd_p)\
  ((rxhd_p)->flags & ReactM_MustAll ? TRUE : FALSE)

#define React_HeadFlags_MustAll_Put(rxhd_p, value)\
  { if ((value) == TRUE)\
    (rxhd_p)->flags |= ReactM_MustAll;\
  else\
    (rxhd_p)->flags &= ~ReactM_MustAll; }

#define React_HeadFlags_MustAny_Get(rxhd_p)\
  ((rxhd_p)->flags & ReactM_MustAny ? TRUE : FALSE)

#define React_HeadFlags_MustAny_Put(rxhd_p, value)\
  { if ((value) == TRUE)\
    (rxhd_p)->flags |= ReactM_MustAny;\
  else\
    (rxhd_p)->flags &= ~ReactM_MustAny; }

#define React_HeadFlags_Protection_Get(rxhd_p)\
  ((rxhd_p)->flags & ReactM_Protection ? TRUE : FALSE)

#define React_HeadFlags_Protection_Put(rxhd_p, value)\
  { if ((value) == TRUE)\
    (rxhd_p)->flags |= ReactM_Protection;\
  else\
    (rxhd_p)->flags &= ~ReactM_Protection; }

#define React_HeadFlags_Unknown1_Get(rxhd_p)\
  ((rxhd_p)->flags & ReactM_Unknown1 ? TRUE : FALSE)

#define React_HeadFlags_Unknown1_Put(rxhd_p, value)\
  { if ((value) == TRUE)\
    (rxhd_p)->flags |= ReactM_Unknown1;\
  else\
    (rxhd_p)->flags &= ~ReactM_Unknown1; }

#define React_Head_Confidence_Get(rxhd_p)\
  (rxhd_p)->confidence

#define React_Head_Confidence_Put(rxhd_p, value)\
  (rxhd_p)->confidence = (value)

#define React_Head_Ease_Get(rxhd_p)\
  (rxhd_p)->ease

#define React_Head_Ease_Put(rxhd_p, value)\
  (rxhd_p)->ease = (value)

#define React_Head_Flags_Get(rxhd_p)\
  (rxhd_p)->flags

#define React_Head_Flags_Put(rxhd_p, value)\
  (rxhd_p)->flags = (value)

#define React_Head_GoalSize_Get(rxhd_p)\
  (rxhd_p)->goal_pattern_size

#define React_Head_GoalSize_Put(rxhd_p, value)\
  (rxhd_p)->goal_pattern_size = (value)

#define React_Head_Library_Get(rxhd_p)\
  (rxhd_p)->library
 
#define React_Head_Library_Put(rxhd_p, value)\
  (rxhd_p)->library = (value)
 
#define React_Head_NumSubgoalAtoms_Get(rxhd_p)\
  (rxhd_p)->subgoal_atoms

#define React_Head_MaxNonident_Get(rxhd_p)\
  (rxhd_p)->max_nonident

#define React_Head_MaxNonident_Put(rxhd_p, value)\
  (rxhd_p)->max_nonident = (value)

#define React_Head_NumConditions_Get(rxhd_p)\
  (rxhd_p)->num_conditions

#define React_Head_NumConditions_Put(rxhd_p, value)\
  (rxhd_p)->num_conditions = (value)

#define React_Head_NumGoalAtoms_Get(rxhd_p)\
  (rxhd_p)->goal_atoms

#define React_Head_NumGoalAtoms_Put(rxhd_p, value)\
  (rxhd_p)->goal_atoms = (value)

#define React_Head_NumSubgoalAtoms_Get(rxhd_p)\
  (rxhd_p)->subgoal_atoms

#define React_Head_NumSubgoalAtoms_Put(rxhd_p, value)\
  (rxhd_p)->subgoal_atoms = (value)

#define React_Head_NumTests_Get(rxhd_p)\
  (rxhd_p)->num_tests

#define React_Head_NumTests_Put(rxhd_p, value)\
  (rxhd_p)->num_tests = (value)

#define React_Head_ReactionType_Get(rxhd_p)\
  (rxhd_p)->react_type

#define React_Head_ReactionType_Put(rxhd_p, value)\
  (rxhd_p)->react_type = (value)

#define React_Head_RootNode_Get(rxhd_p, idx)\
  (rxhd_p)->roots[idx].node

#define React_Head_RootNode_Put(rxhd_p, idx, value)\
  (rxhd_p)->roots[idx].node = (value)

#define React_Head_RootSyntheme_Get(rxhd_p, idx)\
  (rxhd_p)->roots[idx].syntheme

#define React_Head_RootSyntheme_Put(rxhd_p, idx, value)\
  (rxhd_p)->roots[idx].syntheme = (value)

#define React_Head_SchemaId_Get(rxhd_p)\
  (rxhd_p)->schema_num

#define React_Head_SchemaId_Put(rxhd_p, value)\
  (rxhd_p)->schema_num = (value)

#define React_Head_SynthemeFG_Get(rxhd_p)\
  (rxhd_p)->syntheme_fg

#define React_Head_SynthemeFG_Put(rxhd_p, value)\
  (rxhd_p)->syntheme_fg = (value)

#define React_Head_Yield_Get(rxhd_p)\
  (rxhd_p)->yield

#define React_Head_Yield_Put(rxhd_p, value)\
  (rxhd_p)->yield = (value)
#else
#define React_HeadFlags_CantAll_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->flags & ReactM_CantAll ? TRUE : FALSE)

#define React_HeadFlags_CantAll_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else if ((value) == TRUE)\
    (rxhd_p)->flags |= ReactM_CantAll;\
  else (rxhd_p)->flags &= ~ReactM_CantAll; }

#define React_HeadFlags_CantAny_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->flags & ReactM_CantAny ? TRUE : FALSE)

#define React_HeadFlags_CantAny_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else if ((value) == TRUE)\
    (rxhd_p)->flags |= ReactM_CantAny;\
  else (rxhd_p)->flags &= ~ReactM_CantAny; }

#define React_HeadFlags_Disabled_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->flags & ReactM_Disabled ? TRUE : FALSE)

#define React_HeadFlags_Disabled_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else if ((value) == TRUE)\
    (rxhd_p)->flags |= ReactM_Disabled;\
  else (rxhd_p)->flags &= ~ReactM_Disabled; }

#define React_HeadFlags_Incomplete_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->flags & ReactM_Incomplete ? TRUE :\
  FALSE)

#define React_HeadFlags_Incomplete_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else if ((value) == TRUE)\
    (rxhd_p)->flags |= ReactM_Incomplete;\
  else (rxhd_p)->flags &= ~ReactM_Incomplete; }

#define React_HeadFlags_Lookahead_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->flags & ReactM_Lookahead ? TRUE :\
  FALSE)

#define React_HeadFlags_Lookahead_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else if ((value) == TRUE)\
    (rxhd_p)->flags |= ReactM_Lookahead;\
  else (rxhd_p)->flags &= ~ReactM_Lookahead; }

#define React_HeadFlags_MustAll_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->flags & ReactM_MustAll ? TRUE : FALSE)

#define React_HeadFlags_MustAll_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else if ((value) == TRUE)\
    (rxhd_p)->flags |= ReactM_MustAll;\
  else (rxhd_p)->flags &= ~ReactM_MustAll; }

#define React_HeadFlags_MustAny_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->flags & ReactM_MustAny ? TRUE : FALSE)

#define React_HeadFlags_MustAny_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else if ((value) == TRUE)\
    (rxhd_p)->flags |= ReactM_MustAny;\
  else (rxhd_p)->flags &= ~ReactM_MustAny; }

#define React_HeadFlags_Protection_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->flags & ReactM_Protection ? TRUE :\
  FALSE)

#define React_HeadFlags_Protection_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else if ((value) == TRUE)\
    (rxhd_p)->flags |= ReactM_Protection;\
  else (rxhd_p)->flags &= ~ReactM_Protection; }

#define React_HeadFlags_Unknown1_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->flags & ReactM_Unknown1 ? TRUE : FALSE)

#define React_HeadFlags_Unknown1_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else if ((value) == TRUE)\
    (rxhd_p)->flags |= ReactM_Unknown1;\
  else (rxhd_p)->flags &= ~ReactM_Unknown1; }

#define React_Head_Confidence_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->confidence)

#define React_Head_Confidence_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else (rxhd_p)->confidence = (value); }

#define React_Head_Ease_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->ease)

#define React_Head_Ease_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else (rxhd_p)->ease = (value); }

#define React_Head_Flags_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->flags)

#define React_Head_Flags_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else (rxhd_p)->flags = (value); }

#define React_Head_GoalSize_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->goal_pattern_size)

#define React_Head_GoalSize_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else (rxhd_p)->goal_pattern_size = (value); }

#define React_Head_Library_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->library)
 
#define React_Head_Library_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else (rxhd_p)->library = (value); }

#define React_Head_NumSubgoalAtoms_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->subgoal_atoms)

#define React_Head_MaxNonident_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->max_nonident)

#define React_Head_MaxNonident_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else (rxhd_p)->max_nonident = (value); }

#define React_Head_NumConditions_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->num_conditions)

#define React_Head_NumConditions_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else (rxhd_p)->num_conditions = (value); }

#define React_Head_NumGoalAtoms_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->goal_atoms)

#define React_Head_NumGoalAtoms_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else (rxhd_p)->goal_atoms = (value); }

#define React_Head_NumSubgoalAtoms_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->subgoal_atoms)

#define React_Head_NumSubgoalAtoms_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else (rxhd_p)->subgoal_atoms = (value); }

#define React_Head_NumTests_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->num_tests)

#define React_Head_NumTests_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else (rxhd_p)->num_tests = (value); }

#define React_Head_ReactionType_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->react_type)

#define React_Head_ReactionType_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else (rxhd_p)->react_type = (value); }

#define React_Head_RootNode_Get(rxhd_p, idx)\
  ((rxhd_p) < GBAddr || (idx) < 0 || (idx) >= MX_ROOTS ? HALT :\
  (rxhd_p)->roots[idx].node)

#define React_Head_RootNode_Put(rxhd_p, idx, value)\
  { if ((rxhd_p) < GBAddr || (idx) < 0 || (idx) >= MX_ROOTS) HALT; else\
  (rxhd_p)->roots[idx].node = (value); }

#define React_Head_RootSyntheme_Get(rxhd_p, idx)\
  ((rxhd_p) < GBAddr || (idx) < 0 || (idx) >= MX_ROOTS ? HALT :\
  (rxhd_p)->roots[idx].syntheme)

#define React_Head_RootSyntheme_Put(rxhd_p, idx, value)\
  { if ((rxhd_p) < GBAddr || (idx) < 0 || (idx) >= MX_ROOTS) HALT; else\
  (rxhd_p)->roots[idx].syntheme = (value); }

#define React_Head_SchemaId_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->schema_num)

#define React_Head_SchemaId_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else (rxhd_p)->schema_num = (value); }

#define React_Head_SynthemeFG_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->syntheme_fg)

#define React_Head_SynthemeFG_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else (rxhd_p)->syntheme_fg = (value); }

#define React_Head_Yield_Get(rxhd_p)\
  ((rxhd_p) < GBAddr ? HALT : (rxhd_p)->yield)

#define React_Head_Yield_Put(rxhd_p, value)\
  { if ((rxhd_p) < GBAddr) HALT; else (rxhd_p)->yield = (value); }
#endif

/** End of Field Access Macros for React_Head_t **/

/* Reaction text file record header.  This contains all the fixed size
   information found in the Reaction text file.
*/

typedef struct s_reacttxthd
  {
  Date_t        created;                   /* Created date */
  Date_t        last_mod;                  /* Last modified date */
  U8_t          orig_schema;               /* Original schema */
  U8_t          num_references;            /* # references */
  U8_t          num_comments;              /* # comments */
  U8_t          filler1;                   /* To round out the size */
  } React_TextHead_t;
#define REACTEXTHEADSIZE sizeof (React_TextHead_t)

/** Field Access Macros for React_TextHead_t **/

/* Macro Prototypes
   Date_t React_TxtHd_Created_Get       (React_TextHead_t *);
   void   React_TxtHd_Created_Put       (React_TextHead_t *, Date_t);
   Date_t React_TxtHd_LastMod_Get       (React_TextHead_t *);
   void   React_TxtHd_LastMod_Put       (React_TextHead_t *, Date_t);
   U8_t   React_TxtHd_NumComments_Get   (React_TextHead_t *);
   void   React_TxtHd_NumComments_Put   (React_TextHead_t *, U8_t);
   U8_t   React_TxtHd_NumReferences_Get (React_TextHead_t *);
   void   React_TxtHd_NumReferences_Put (React_TextHead_t *, U8_t);
   U8_t   React_TxtHd_OrigSchema_Get    (React_TextHead_t *);
   void   React_TxtHd_OrigSchema_Put    (React_TextHead_t *, U8_t);
*/

#ifndef REACT_DEBUG
#define React_TxtHd_Created_Get(rxthd_p)\
  (rxthd_p)->created

#define React_TxtHd_Created_Put(rxthd_p, value)\
  (rxthd_p)->created = (value)

#define React_TxtHd_LastMod_Get(rxthd_p)\
  (rxthd_p)->last_mod

#define React_TxtHd_LastMod_Put(rxthd_p, value)\
  (rxthd_p)->last_mod = (value)

#define React_TxtHd_NumComments_Get(rxthd_p)\
  (rxthd_p)->num_comments

#define React_TxtHd_NumComments_Put(rxthd_p, value)\
  (rxthd_p)->num_comments = (value)

#define React_TxtHd_NumReferences_Get(rxthd_p)\
  (rxthd_p)->num_references

#define React_TxtHd_NumReferences_Put(rxthd_p, value)\
  (rxthd_p)->num_references = (value)

#define React_TxtHd_OrigSchema_Get(rxthd_p)\
  (rxthd_p)->orig_schema

#define React_TxtHd_OrigSchema_Put(rxthd_p, value)\
  (rxthd_p)->orig_schema = (value)
#else
#define React_TxtHd_Created_Get(rxthd_p)\
  ((rxthd_p) < GBAddr ? HALT : (rxthd_p)->created)

#define React_TxtHd_Created_Put(rxthd_p, value)\
  { if ((rxthd_p) < GBAddr) HALT; else (rxthd_p)->created = (value); }

#define React_TxtHd_LastMod_Get(rxthd_p)\
  ((rxthd_p) < GBAddr ? HALT : (rxthd_p)->last_mod)

#define React_TxtHd_LastMod_Put(rxthd_p, value)\
  { if ((rxthd_p) < GBAddr) HALT; else (rxthd_p)->last_mod = (value); }

#define React_TxtHd_NumComments_Get(rxthd_p)\
  ((rxthd_p) < GBAddr ? HALT : (rxthd_p)->num_comments)

#define React_TxtHd_NumComments_Put(rxthd_p, value)\
  { if ((rxthd_p) < GBAddr) HALT; else (rxthd_p)->num_comments = (value); }

#define React_TxtHd_NumReferences_Get(rxthd_p)\
  ((rxthd_p) < GBAddr ? HALT : (rxthd_p)->num_references)

#define React_TxtHd_NumReferences_Put(rxthd_p, value)\
  { if ((rxthd_p) < GBAddr) HALT; else (rxthd_p)->num_references = (value); }

#define React_TxtHd_OrigSchema_Get(rxthd_p)\
  ((rxthd_p) < GBAddr ? HALT : (rxthd_p)->orig_schema)

#define React_TxtHd_OrigSchema_Put(rxthd_p, value)\
  { if ((rxthd_p) < GBAddr) HALT; else (rxthd_p)->orig_schema = (value); }
#endif

/** End of Field Acces Macros for React_TextHead_t **/

/* Reaction text file record data-structure.  This contains references
   to all the data that might be needed at interpretation or inferencing
   time.
*/

typedef struct s_reacttxtrec
  {
  React_TextHead_t headb;                  /* Header */
  String_t     *reasons;                   /* Post-test reason array */
  String_t     *chemists;                  /* Post-test chemist array */
  String_t     *comments;                  /* Reaction comments */
  String_t     *lit_refs;                  /* Reaction Literature references */
  String_t      name;                      /* Reaction name */
  } React_TextRec_t;
#define REACTEXTRECSIZE sizeof (React_TextRec_t)

/** Field Access Macros for React_TextRec_t **/

/* Macro Prototypes
   String_t React_TxtRec_Chemist_Get   (React_TextRec_t *, U8_t);
   void     React_TxtRec_Chemist_Put   (React_TextRec_t *, U8_t, String_t);
   String_t React_TxtRec_Comment_Get   (React_TextRec_t *, U8_t);
   void     React_TxtRec_Comment_Put   (React_TextRec_t *, U8_t, String_t);
   React_TextHead_t *React_TxtRec_Head_Get (React_TextRec_t *, U8_t);
   String_t React_TxtRec_Name_Get      (React_TextRec_t *);
   void     React_TxtRec_Name_Put      (React_TextRec_t *, String_t);
   String_t React_TxtRec_Reason_Get    (React_TextRec_t *, U8_t);
   void     React_TxtRec_Reason_Put    (React_TextRec_t *, U8_t, String_t);
   String_t React_TxtRec_Reference_Get (React_TextRec_t *, U8_t);
   void     React_TxtRec_Reference_Put (React_TextRec_t *, U8_t, String_t);
*/

#ifndef REACT_DEBUG
#define React_TxtRec_Chemist_Get(rtxr_p, idx)\
  (rtxr_p)->chemists[idx]

#define React_TxtRec_Chemist_Put(rtxr_p, idx, value)\
  (rtxr_p)->chemists[idx] = (value)

#define React_TxtRec_Comment_Get(rtxr_p, idx)\
  (rtxr_p)->comments[idx]

#define React_TxtRec_Comment_Put(rtxr_p, idx, value)\
  (rtxr_p)->comments[idx] = (value)

#define React_TxtRec_Name_Get(rtxr_p)\
  (rtxr_p)->name

#define React_TxtRec_Name_Put(rtxr_p, value)\
  (rtxr_p)->name = (value)

#define React_TxtRec_Head_Get(rtxr_p)\
  &(rtxr_p)->headb

#define React_TxtRec_Reason_Get(rtxr_p, idx)\
  (rtxr_p)->reasons[idx]

#define React_TxtRec_Reason_Put(rtxr_p, idx, value)\
  (rtxr_p)->reasons[idx] = (value)

#define React_TxtRec_Reference_Get(rtxr_p, idx)\
  (rtxr_p)->lit_refs[idx]

#define React_TxtRec_Reference_Put(rtxr_p, idx, value)\
  (rtxr_p)->lit_refs[idx] = (value)
#else
#define React_TxtRec_Chemist_Get(rtxr_p, idx)\
  ((rtxr_p) < GBAddr || (idx) < 0 || (idx) >= (rtxr_p)->headb.num_comments ?\
  HALTST : (rtxr_p)->chemists[idx])

#define React_TxtRec_Chemist_Put(rtxr_p, idx, value)\
  { if ((rtxr_p) < GBAddr || (idx) < 0 || (idx) >=\
  (rtxr_p)->headb.num_comments) HALT; else (rtxr_p)->chemists[idx] = (value); }

#define React_TxtRec_Comment_Get(rtxr_p, idx)\
  ((rtxr_p) < GBAddr || (idx) < 0 || (idx) >= (rtxr_p)->headb.num_comments ?\
  HALTST : (rtxr_p)->comments[idx])

#define React_TxtRec_Comment_Put(rtxr_p, idx, value)\
  { if ((rtxr_p) < GBAddr || (idx) < 0 || (idx) >=\
  (rtxr_p)->headb.num_comments) HALT; else (rtxr_p)->comments[idx] = (value); }

#define React_TxtRec_Name_Get(rtxr_p)\
  ((rtxr_p) < GBAddr ? HALTST : (rtxr_p)->name)

#define React_TxtRec_Name_Put(rtxr_p, value)\
  (rtxr_p)->name = (value)

#define React_TxtRec_Head_Get(rtxr_p)\
  ((rtxr_p) < GBAddr ? HALT : &(rtxr_p)->headb)

#define React_TxtRec_Reason_Get(rtxr_p, idx)\
  ((rtxr_p) < GBAddr || (idx) < 0 || (idx) >= (rtxr_p)->headb.num_comments ?\
  HALTST : (rtxr_p)->reasons[idx])

#define React_TxtRec_Reason_Put(rtxr_p, idx, value)\
  { if ((rtxr_p) < GBAddr || (idx) < 0 || (idx) >=\
  (rtxr_p)->headb.num_comments) HALT; else (rtxr_p)->reasons[idx] = (value); }

#define React_TxtRec_Reference_Get(rtxr_p, idx)\
  ((rtxr_p) < GBAddr || (idx) < 0 || (idx) >= (rtxr_p)->headb.num_references ?\
  HALTST : (rtxr_p)->lit_refs[idx])

#define React_TxtRec_Reference_Put(rtxr_p, idx, value)\
  { if ((rtxr_p) < GBAddr || (idx) < 0 || (idx) >=\
  (rtxr_p)->headb.num_references) HALT; else\
  (rtxr_p)->lit_refs[idx] = (value); }
#endif

/** Field Access Macros for React_TextRec_t **/

/* Reaction data file record data-structure.  This contains references to
   every bit of information Synchem needs at runtime.  It also has a place
   to reference the text file information.
*/
typedef struct s_postrec
  {
  React_Head_t  headb;                     /* Reaction data header */
  Tsd_t        *goal;                      /* Goal molecule in TSD format */
  Tsd_t        *subgoal;                   /* Sub-goal m'cule in TSD format */
  Condition_t  *conditions;                /* Post-transform conditions */
  Posttest_t   *tests;                     /* Post-transform tests */
  React_TextRec_t *text;                   /* Text record handle */
  Array_t       mustall;                   /* Must have allof - attributes
                                              1-d bit array */
  Array_t       mustany;                   /* Must have anyof - attributes
                                              1-d bit array */
  Array_t       notany;                    /* Can't have anyof - attributes
                                              1-d bit array */
  Array_t       notall;                    /* Can't have allof - attributes
                                              1-d bit array */
  } React_Record_t;
#define REACTRECSIZE sizeof (React_Record_t)

/** Field Access Macros for React_Record_t **/

/* Macro Prototypes
   Boolean_t     React_CantAll_Get   (React_Record_t *, U16_t);
   void          React_CantAll_Put   (React_Record_t *, U16_t, U8_t);
   Boolean_t     React_CantAny_Get   (React_Record_t *, U16_t);
   void          React_CantAny_Put   (React_Record_t *, U16_t, U8_t);
   Condition_t  *React_Condition_Get (React_Record_t *, U8_t);
   void          React_Condition_Put (React_Record_t *, U8_t, Condition_t);
   Tsd_t        *React_Goal_Get      (React_Record_t *);
   void          React_Goal_Put      (React_Record_t *, Tsd_t *);
   React_Head_t *React_Head_Get      (React_Record_t *);
   Boolean_t     React_MustAll_Get   (React_Record_t *, U16_t);
   void          React_MustAll_Put   (React_Record_t *, U16_t, U8_t);
   Boolean_t     React_MustAny_Get   (React_Record_t *, U16_t);
   void          React_MustAny_Put   (React_Record_t *, U16_t, U8_t);
   Tsd_t        *React_Subgoal_Get   (React_Record_t *);
   void          React_Subgoal_Put   (React_Record_t *, Tsd_t *);
   Posttest_t   *React_Test_Get      (React_Record_t *, U8_t);
   void          React_Test_Put      (React_Record_t *, U8_t, Posttest_t);
   React_TextRec_t *React_Text_Get   (React_Record_t *);
   void          React_Text_Put      (React_Record_t *, React_TextRec_t *);
*/

#ifndef REACT_FILE
#define React_CantAll_Get(rxn_p, idx)\
  Array_1d1_Get (&(rxn_p)->notall, idx)

#define React_CantAll_Put(rxn_p, idx, value)\
  Array_1d1_Put (&(rxn_p)->notall, idx, value)

#define React_CantAny_Get(rxn_p, idx)\
  Array_1d1_Get (&(rxn_p)->notany, idx)

#define React_CantAny_Put(rxn_p, idx, value)\
  Array_1d1_Put (&(rxn_p)->notany, idx, value)

#define React_Condition_Get(rxn_p, idx)\
  &(rxn_p)->conditions[idx]

#define React_Condition_Put(rxn_p, idx, value)\
  (rxn_p)->conditions[idx] = (value)

#define React_Goal_Get(rxn_p)\
  (rxn_p)->goal

#define React_Goal_Put(rxn_p, value)\
  (rxn_p)->goal = (value)

#define React_Head_Get(rxn_p)\
  &(rxn_p)->headb

#define React_MustAll_Get(rxn_p, idx)\
  Array_1d1_Get (&(rxn_p)->mustall, idx)

#define React_MustAll_Put(rxn_p, idx, value)\
  Array_1d1_Put (&(rxn_p)->mustall, idx, value)

#define React_MustAny_Get(rxn_p, idx)\
  Array_1d1_Get (&(rxn_p)->mustany, idx)

#define React_MustAny_Put(rxn_p, idx, value)\
  Array_1d1_Put (&(rxn_p)->mustany, idx, value)

#define React_Subgoal_Get(rxn_p)\
  (rxn_p)->subgoal

#define React_Subgoal_Put(rxn_p, value)\
  (rxn_p)->subgoal = (value)

#define React_Test_Get(rxn_p, idx)\
  &(rxn_p)->tests[idx]

#define React_Test_Put(rxn_p, idx, value)\
  (rxn_p)->tests[idx] = (value)

#define React_Text_Get(rxn_p)\
  (rxn_p)->text

#define React_Text_Put(rxn_p, value)\
  (rxn_p)->text = (value)
#else
  ((rxn_p) < GBAddr ? HALT : Array_1d1_Get (&(rxn_p)->notall, idx))

#define React_CantAll_Put(rxn_p, idx, value)\
  { if ((rxn_p) < GBAddr) HALT; else Array_1d1_Put (&(rxn_p)->notall, idx,\
  value); }

#define React_CantAny_Get(rxn_p, idx)\
  ((rxn_p) < GBAddr ? HALT : Array_1d1_Get (&(rxn_p)->notany, idx))

#define React_CantAny_Put(rxn_p, idx, value)\
  { if ((rxn_p) < GBAddr) HALT; else Array_1d1_Put (&(rxn_p)->notany, idx,\
  value); }

#define React_Condition_Get(rxn_p, idx)\
  ((rxn_p) < GBAddr || (idx) < 0 || (idx) >= (rxn_p)->headb.num_tests ? HALT :\
  &(rxn_p)->conditions[idx])

#define React_Condition_Put(rxn_p, idx, value)\
  { if ((rxn_p) < GBAddr || (idx) < 0 || (idx) >=\
  (rxn_p)->headb.num_conditions) HALT; else\
  (rxn_p)->conditions[idx] = (value); }

#define React_Goal_Get(rxn_p)\
  ((rxn_p) < GBAddr ? HALT : (rxn_p)->goal)

#define React_Goal_Put(rxn_p, value)\
  { if ((rxn_p) < GBAddr) HALT; else (rxn_p)->goal = (value); }

#define React_Head_Get(rxn_p)\
  ((rxn_p) < GBAddr ? HALT : &(rxn_p)->headb)

#define React_MustAll_Get(rxn_p, idx)\
  ((rxn_p) < GBAddr ? HALT : Array_1d1_Get (&(rxn_p)->mustall, idx))

#define React_MustAll_Put(rxn_p, idx, value)\
  { if ((rxn_p) < GBAddr) HALT; else Array_1d1_Put (&(rxn_p)->mustall, idx,\
  value); }

#define React_MustAny_Get(rxn_p, idx)\)
  ((rxn_p) < GBAddr ? HALT : Array_1d1_Get (&(rxn_p)->mustany, idx))

#define React_MustAny_Put(rxn_p, idx, value)\
  { if ((rxn_p) < GBAddr) HALT; else Array_1d1_Put (&(rxn_p)->mustany, idx,\
  value); }

#define React_Subgoal_Get(rxn_p)\
  ((rxn_p) < GBAddr ? HALT : (rxn_p)->subgoal)

#define React_Subgoal_Put(rxn_p, value)\
  { if ((rxn_p) < GBAddr) HALT; else (rxn_p)->subgoal = (value); }

#define React_Test_Get(rxn_p, idx)\
  ((rxn_p) < GBAddr || (idx) < 0 || (idx) >= (rxn_p)->headb.num_tests ? HALT :\
  &(rxn_p)->tests[idx])

#define React_Test_Put(rxn_p, idx, value)\
  { if ((rxn_p) < GBAddr || (idx) < 0 || (idx) >= (rxn_p)->headb.num_tests)\
  HALT; else (rxn_p)->tests[idx] = (value); }

#define React_Text_Get(rxn_p)\
  ((rxn_p) < GBAddr ? HALT : (rxn_p)->text)

#define React_Text_Put(rxn_p, value)\
  { if ((rxn_p) < GBAddr) HALT; else (rxn_p)->text = (value); }
#endif

/** End of Field Access Macros for React_Record_t **/

/*** Macros ***/

/*** Routine Prototypes ***/

void            React_Init              (U8_t *);
void            React_Open              (U8_t *);
void            React_Force_Open        (U8_t *);
void            React_Open_Files        (U8_t *, Boolean_t);
void            React_Initialize        (U8_t *, Boolean_t);
void            React_Force_Initialize  (U8_t *, Boolean_t);
void            React_Initialization    (U8_t *, Boolean_t, Boolean_t);
void            React_Check_Alloc       (U32_t);
void            React_Temp_Init         (U8_t *);
U32_t           React_NumSchemas_Get    (void);
React_Record_t *React_Schema_Handle_Get (U32_t);
Boolean_t       React_Schema_Copy       (int, int);
Boolean_t       React_Schema_Allocated  (int);
void            React_Schema_Free       (int);
void            React_Schema_Write      (U32_t);
void            React_Close             ();

/* In XTR.H
   Boolean_t React_Schema_Ok      (Xtr_t *, U16_t, U16_t);
*/

/*** Global Variables ***/

#ifdef REACTION_GLOBALS
Boolean_t bypass_incomplete_flag = FALSE;
static Isam_Control_t   SReactDataFile;
static Isam_Control_t   SReactTextFile;
static React_Record_t **SReactions;
static char             rxn_buffer[RXN_BUFFER_SIZE];
static int              curr_schemata_allocated = 0;
#else
extern Boolean_t bypass_incomplete_flag;
#endif

/* End of Reaction.H */
#endif
