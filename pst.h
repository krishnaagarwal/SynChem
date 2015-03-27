#ifndef _H_PST_
#define _H_PST_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     PST.H
*
*    This module is the abstraction of the Problem-Solving Tree data-structure.
*    It is not a true tree, but it is useful to think of it as such.  It
*    contains all the AND and OR nodes for the heuristic search.  This module
*    knows nothing about how to interpret the heuristic information, that is
*    in the strategy module(s).  
*
*    Along with the Subgoal_t and Compound_t structures there is the Symtab_t
*    structure to represent the symbol table.
*    The symbol table is the data-structure that keeps track of all the
*    chemistry information about the compounds in the Problem Solving
*    Tree (PST).  All compound nodes have their compound represented
*    here and so this can be used to eliminate duplicate solutions to
*    compound synthesis.
*
*    The symbol table is a hashed data-structure, the canonical Sling 
*    is used as the unique value to hash on.
*    Used to be CREATE.PLI, SYMTAB.PLI
*
*    Routines are found in PST.C
*
*  Creation Date:
*
*    01-Jan-1991
*
*  Authors:
*
*    Tito Autrey (rewritten based on others PLI code)
*    Shu Cheung
*
*  Modification History, reverse chronological
*
* Date       Author     Modifcation Description
*------------------------------------------------------------------------------
* 23-Sep-97  Krebsbach  Combined sequential and distributed versions
*                       of SYNCHEM into single file, making many changes to
*                       data structures and function prototypes.
* 11-Mar-95  Cheung     Added fields, merit_info, total_compounds, 
*   last_compound, and current_comp in structure PstCB_t.
* 14-Aug-95  Cheung     Changed all the pointer fields in the structures to 
*   union of pointer and index. Modified and added 
*   corresponding macros.
* 14-Sep-95  Cheung     only the merit of target compound is stored as shelved
*   merit.
*
******************************************************************************/

#ifndef _H_SYNIO_
#include "synio.h"
#endif

#ifndef _H_ARRAY_
#include "array.h"
#endif

#ifndef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_TIMER_
#include "timer.h"
#endif

#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_SUBGOALGENERATION_
#include "subgoalgeneration.h"
#endif

#ifndef _H_STRATEGY_
#include "strategy.h"
#endif


/*** Literal values ***/

#define MX_COMP_MERIT        100              /* Maximum compound merit */
#define MX_SYMTAB_BUCKETS    4111

#define COMP_MERIT_INIT      (U16_t)-204
#define SUBG_MERIT_INIT      -205
#define SYMTAB_MERIT_INIT    (U16_t)-206
#define COMP_MERIT_SELECT    0
#define SUBG_MERIT_SELECT    -3
#define SYM_WORKER_NONE      (U16_t)-1

/* PST Node types */

#define PST_INVALID          (U8_t)-102
#define PST_COMPOUND         1
#define PST_SUBGOAL          2

/*** Data-structures ***/

/* This is the symbol table data-structure.  It is a table of all the compounds
   needed so far in the problem solving tree.  The idea is to avoid replicating
   information when the same compound appears multiple times in the tree.
   For a given compound this seems like a likely occurence so this is a useful
   thing to do.

   Note: some forward declarations are needed.
*/

struct s_subg;
struct s_comp;

typedef struct s_symtab
  {
  Sling_t        slingb;                     /* Sling for this compound */
  Time_t         cycle_time;                 /* Time spent on this compound */
  union u_first
    {
    struct s_comp *addr;
    U32_t          index;
    } first;                      /* Comp index of first instance */
  union u_developed
    {
    struct s_comp *addr;
    U32_t          index;
    } developed;                  /* Developed instance */
  union u_bucket
    {
    struct s_symtab *addr;
    U32_t            index;
    } bucket;                   /* Bucket chain pointer */
  union u_current
    {
    struct s_comp *addr;
    U32_t          index;
    } current;                               /* Comp node on current path */
  U32_t          avlcmp_key;                 /* Index for Avail Comp Lib */
  U32_t          index;                      /* Identifying number */
  U16_t          main_merit;                 /* Est. compound merit */
  U16_t          solved_merit;               /* Est. solved merit */
  U16_t          initial_merit;              /* Est. initial merit */
  U16_t          cycle_number;               /* Cycle in which comp expanded */
  U16_t          first_solution;             /* Cycle of first solution */
  U16_t          last_solution;              /* Cycle of most recent solutn */
  U16_t          flags;                      /* Bitvector for flags */
  U16_t          instances_cnt;              /* Number of instances */
  U16_t          num_sols;                   /* Number of solutions */
  U16_t          worker_id;                  /* Worker who expanded comp */
  } SymTab_t;
#define SYMTABSIZE sizeof (SymTab_t)

/* Flag literal values */

#define SymM_Available       0x1
#define SymM_Solved          0x2
#define SymM_Stuck           0x4
#define SymM_Open            0x8
#define SymM_UnSolveable     0x10
#define SymM_NewlySolved     0x20
#define SymM_Selected        0x100
#define SymM_WasSelected     0x200
#define SymM_GlobalSelect    0x400
#define SymM_LocalSelect     0x800
#define SymM_DupSelect       0x1000

/** Field Access Macros for a SymTab_t **/

/* Macro Prototypes
   U32_t          SymTab_AvailCompKey_Get      (SymTab_t *);
   void           SymTab_AvailCompKey_Put      (SymTab_t *, U32_t);
   Compound_t    *SymTab_Current_Get           (SymTab_t *);
   void           SymTab_Current_Put           (SymTab_t *, Compound_t *);
   U32_t          SymTab_CurrentIndex_Get      (SymTab_t *);
   void           SymTab_CurrentIndex_Put      (SymTab_t *, U32_t);
   U16_t          SymTab_Cycle_First_Get       (SymTab_t *);
   void           SymTab_Cycle_First_Put       (SymTab_t *, U16_t);
   U16_t          SymTab_Cycle_Last_Get        (SymTab_t *);
   void           SymTab_Cycle_Last_Put        (SymTab_t *, U16_t);
   U16_t          SymTab_Cycle_Number_Get      (SymTab_t *);
   void           SymTab_Cycle_Number_Put      (SymTab_t *, U16_t);
   Time_t         SymTab_Cycle_Time_Get        (SymTab_t *);
   void           SymTab_Cycle_Time_Put        (SymTab_t *, Time_t);
   Compound_t    *SymTab_DevelopedComp_Get     (SymTab_t *);
   void           SymTab_DevelopedComp_Put     (SymTab_t *, Compound_t *);
   U32_t          SymTab_DevelopedCompIndex_Get (SymTab_t *);
   void           SymTab_DevelopedCompIndex_Put (SymTab_t *, U32_t);
   Compound_t    *SymTab_FirstComp_Get         (SymTab_t *);
   void           SymTab_FirstComp_Put         (SymTab_t *, Compound_t *);
   U32_t          SymTab_FirstCompIndex_Get    (SymTab_t *);
   void           SymTab_FirstCompIndex_Put    (SymTab_t *, U32_t);
   U16_t          SymTab_Flags_Get             (SymTab_t *);
   void           SymTab_Flags_Put             (SymTab_t *, U16_t);
   Boolean_t      SymTab_Flags_Available_Get   (SymTab_t *);
   void           SymTab_Flags_Available_Put   (SymTab_t *, Boolean_t);
   Boolean_t      SymTab_Flags_DupSelect_Get   (SymTab_t *);
   void           SymTab_Flags_DupSelect_Put   (SymTab_t *, Boolean_t);
   Boolean_t      SymTab_Flags_GlobalSelect_Get (SymTab_t *);
   void           SymTab_Flags_GlobalSelect_Put (SymTab_t *, Boolean_t);
   Boolean_t      SymTab_Flags_LocalSelect_Get (SymTab_t *);
   void           SymTab_Flags_LocalSelect_Put (SymTab_t *, Boolean_t);
   Boolean_t      SymTab_Flags_NewlySolved_Get (SymTab_t *);
   void           SymTab_Flags_NewlySolved_Put (SymTab_t *, Boolean_t);
   Boolean_t      SymTab_Flags_Open_Get        (SymTab_t *);
   void           SymTab_Flags_Open_Put        (SymTab_t *, Boolean_t);
   Boolean_t      SymTab_Flags_Selected_Get    (SymTab_t *);
   void           SymTab_Flags_Selected_Put    (SymTab_t *, Boolean_t);
   Boolean_t      SymTab_Flags_Solved_Get      (SymTab_t *);
   void           SymTab_Flags_Solved_Put      (SymTab_t *, Boolean_t);
   Boolean_t      SymTab_Flags_Stuck_Get       (SymTab_t *);
   void           SymTab_Flags_Stuck_Put       (SymTab_t *, Boolean_t);
   Boolean_t      SymTab_Flags_Unsolveable_Get (SymTab_t *);
   void           SymTab_Flags_Unsolveable_Put (SymTab_t *, Boolean_t);
   Boolean_t      SymTab_Flags_WasSelected_Get (SymTab_t *);
   void           SymTab_Flags_WasSelected_Put (SymTab_t *, Boolean_t);
   SymTab_t      *SymTab_HashChain_Get         (SymTab_t *);
   void           SymTab_HashChain_Put         (SymTab_t *, SymTab_t *);
   U32_t          SymTab_HashChainIndex_Get    (SymTab_t *);
   void           SymTab_HashChainIndex_Put    (SymTab_t *, U32_t);
   U16_t          SymTab_HashSize_Get          (void);
   U32_t          SymTab_Index_Get             (SymTab_t *);
   void           SymTab_Index_Put             (SymTab_t *, U32_t);
   U16_t          SymTab_InstancesCnt_Get      (SymTab_t *);
   void           SymTab_InstancesCnt_Put      (SymTab_t *, U16_t);
   U16_t          SymTab_Merit_Initial_Get     (SymTab_t *);
   void           SymTab_Merit_Initial_Put     (SymTab_t *, U16_t);
   U16_t          SymTab_Merit_Main_Get        (SymTab_t *);
   void           SymTab_Merit_Main_Put        (SymTab_t *, U16_t);
   U16_t          SymTab_Merit_Solved_Get      (SymTab_t *);
   void           SymTab_Merit_Solved_Put      (SymTab_t *, U16_t);
   U16_t          SymTab_NumSols_Get           (SymTab_t *);
   void           SymTab_NumSols_Put           (SymTab_t *, U16_t);
   Sling_t        SymTab_Sling_Get             (SymTab_t *);
   void           SymTab_Sling_Put             (SymTab_t *, Sling_t);
   U16_t          SymTab_WorkerId_Get          (SymTab_t *);
   void           SymTab_WorkerId_Put          (SymTab_t *, U16_t);
*/

#define SymTab_AvailCompKey_Get(symtab_p)\
  (symtab_p)->avlcmp_key
#define SymTab_AvailCompKey_Put(symtab_p, value)\
  (symtab_p)->avlcmp_key = (value)

#define SymTab_Current_Get(symtab_p)\
  (symtab_p)->current.addr
#define SymTab_Current_Put(symtab_p, value)\
  (symtab_p)->current.addr = (value)

#define SymTab_CurrentIndex_Get(symtab_p)\
  (symtab_p)->current.index
#define SymTab_CurrentIndex_Put(symtab_p, value)\
  (symtab_p)->current.index = (value)

#define SymTab_Cycle_First_Get(symtab_p)\
  (symtab_p)->first_solution
#define SymTab_Cycle_First_Put(symtab_p, value)\
  (symtab_p)->first_solution = (value)

#define SymTab_Cycle_Last_Get(symtab_p)\
  (symtab_p)->last_solution
#define SymTab_Cycle_Last_Put(symtab_p, value)\
  (symtab_p)->last_solution = (value)

#define SymTab_Cycle_Number_Get(symtab_p)\
  (symtab_p)->cycle_number
#define SymTab_Cycle_Number_Put(symtab_p, value)\
  (symtab_p)->cycle_number = (value)

#define SymTab_Cycle_Time_Get(symtab_p)\
  (symtab_p)->cycle_time
#define SymTab_Cycle_Time_Put(symtab_p, value)\
  (symtab_p)->cycle_time = (value)

#define SymTab_DevelopedComp_Get(symtab_p)\
  (symtab_p)->developed.addr
#define SymTab_DevelopedComp_Put(symtab_p, value)\
  (symtab_p)->developed.addr = (value)

#define SymTab_DevelopedCompIndex_Get(symtab_p)\
  (symtab_p)->developed.index
#define SymTab_DevelopedCompIndex_Put(symtab_p, value)\
  (symtab_p)->developed.index = (value)

#define SymTab_FirstComp_Get(symtab_p)\
  (symtab_p)->first.addr
#define SymTab_FirstComp_Put(symtab_p, value)\
  (symtab_p)->first.addr = (value)

#define SymTab_FirstCompIndex_Get(symtab_p)\
  (symtab_p)->first.index
#define SymTab_FirstCompIndex_Put(symtab_p, value)\
  (symtab_p)->first.index = (value)

#define SymTab_Flags_Get(symtab_p)\
  (symtab_p)->flags
#define SymTab_Flags_Put(symtab_p, value)\
  (symtab_p)->flags = (value)

#define SymTab_Flags_Available_Get(symtab_p)\
  ((symtab_p)->flags & SymM_Available ? TRUE : FALSE)
#define SymTab_Flags_Available_Put(symtab_p, value)\
  { if ((value) == TRUE)\
    (symtab_p)->flags |= SymM_Available;\
  else\
    (symtab_p)->flags &= ~SymM_Available; }

#define SymTab_Flags_DupSelect_Get(symtab_p)\
  ((symtab_p)->flags & SymM_DupSelect ? TRUE : FALSE)
#define SymTab_Flags_DupSelect_Put(symtab_p, value)\
  { if ((value) == TRUE)\
    (symtab_p)->flags |= SymM_DupSelect;\
  else\
    (symtab_p)->flags &= ~SymM_DupSelect;}

#define SymTab_Flags_GlobalSelect_Get(symtab_p)\
  ((symtab_p)->flags & SymM_GlobalSelect ? TRUE : FALSE)
#define SymTab_Flags_GlobalSelect_Put(symtab_p, value)\
  { if ((value) == TRUE)\
    (symtab_p)->flags |= SymM_GlobalSelect;\
  else\
    (symtab_p)->flags &= ~SymM_GlobalSelect;}

#define SymTab_Flags_LocalSelect_Get(symtab_p)\
  ((symtab_p)->flags & SymM_LocalSelect ? TRUE : FALSE)
#define SymTab_Flags_LocalSelect_Put(symtab_p, value)\
  { if ((value) == TRUE)\
    (symtab_p)->flags |= SymM_LocalSelect;\
  else\
    (symtab_p)->flags &= ~SymM_LocalSelect;}

#define SymTab_Flags_NewlySolved_Get(symtab_p)\
  ((symtab_p)->flags & SymM_NewlySolved ? TRUE : FALSE)
#define SymTab_Flags_NewlySolved_Put(symtab_p, value)\
  { if ((value) == TRUE)\
    (symtab_p)->flags |= SymM_NewlySolved;\
  else\
    (symtab_p)->flags &= ~SymM_NewlySolved;}

#define SymTab_Flags_Open_Get(symtab_p)\
  ((symtab_p)->flags & SymM_Open ? TRUE : FALSE)
#define SymTab_Flags_Open_Put(symtab_p, value)\
  { if ((value) == TRUE)\
    (symtab_p)->flags |= SymM_Open;\
  else\
    (symtab_p)->flags &= ~SymM_Open; }

#define SymTab_Flags_Selected_Get(symtab_p)\
  ((symtab_p)->flags & SymM_Selected ? TRUE : FALSE)
#define  SymTab_Flags_Selected_Put(symtab_p, value)\
  { if ((value) == TRUE)\
    (symtab_p)->flags |= SymM_Selected;\
  else\
    (symtab_p)->flags &= ~SymM_Selected; }
 
#define SymTab_Flags_Solved_Get(symtab_p)\
  ((symtab_p)->flags & SymM_Solved ? TRUE : FALSE)
#define SymTab_Flags_Solved_Put(symtab_p, value)\
  { if ((value) == TRUE)\
    (symtab_p)->flags |= SymM_Solved;\
  else\
    (symtab_p)->flags &= ~SymM_Solved; }

#define SymTab_Flags_Stuck_Get(symtab_p)\
  ((symtab_p)->flags & SymM_Stuck ? TRUE : FALSE)
#define SymTab_Flags_Stuck_Put(symtab_p, value)\
  { if ((value) == TRUE)\
    (symtab_p)->flags |= SymM_Stuck;\
  else\
    (symtab_p)->flags &= ~SymM_Stuck; }

#define SymTab_Flags_Unsolveable_Get(symtab_p)\
  ((symtab_p)->flags & SymM_UnSolveable ? TRUE : FALSE)
#define SymTab_Flags_Unsolveable_Put(symtab_p, value)\
  { if ((value) == TRUE)\
    (symtab_p)->flags |= SymM_UnSolveable;\
  else\
    (symtab_p)->flags &= ~SymM_UnSolveable; }

#define SymTab_Flags_WasSelected_Get(symtab_p)\
  ((symtab_p)->flags & SymM_WasSelected ? TRUE : FALSE)
#define  SymTab_Flags_WasSelected_Put(symtab_p, value)\
  { if ((value) == TRUE)\
    (symtab_p)->flags |= SymM_WasSelected;\
  else\
    (symtab_p)->flags &= ~SymM_WasSelected; }
 
#define SymTab_HashChain_Get(symtab_p)\
  (symtab_p)->bucket.addr
#define SymTab_HashChain_Put(symtab_p, value)\
  (symtab_p)->bucket.addr = (value)

#define SymTab_HashChainIndex_Get(symtab_p)\
  (symtab_p)->bucket.index
#define SymTab_HashChainIndex_Put(symtab_p, value)\
  (symtab_p)->bucket.index = (value)

#define SymTab_HashSize_Get()\
  MX_SYMTAB_BUCKETS

#define SymTab_Index_Get(symtab_p)\
  (symtab_p)->index
#define SymTab_Index_Put(symtab_p, value)\
  (symtab_p)->index = (value)

#define SymTab_InstancesCnt_Get(symtab_p)\
  (symtab_p)->instances_cnt
#define SymTab_InstancesCnt_Put(symtab_p, value)\
  (symtab_p)->instances_cnt = (value)

#define SymTab_Merit_Initial_Get(symtab_p)\
  (symtab_p)->initial_merit
#define SymTab_Merit_Initial_Put(symtab_p, value)\
  (symtab_p)->initial_merit = (value)

#define SymTab_Merit_Main_Get(symtab_p)\
  (symtab_p)->main_merit
#define SymTab_Merit_Main_Put(symtab_p, value)\
  (symtab_p)->main_merit = (value)

#define SymTab_Merit_Solved_Get(symtab_p)\
  (symtab_p)->solved_merit
#define SymTab_Merit_Solved_Put(symtab_p, value)\
  (symtab_p)->solved_merit = (value)

#define SymTab_NumSols_Get(symtab_p)\
  (symtab_p)->num_sols
#define SymTab_NumSols_Put(symtab_p, value)\
  (symtab_p)->num_sols = (value)

#define SymTab_Sling_Get(symtab_p)\
  (symtab_p)->slingb
#define SymTab_Sling_Put(symtab_p, value)\
  (symtab_p)->slingb = (value)

#define SymTab_WorkerId_Get(symtab_p)\
  (symtab_p)->worker_id
#define SymTab_WorkerId_Put(symtab_p, value)\
  (symtab_p)->worker_id = (value)


/* Compound node data-structure.  This is an AND-node in the PST.  Father
   is parent, son is descendant in the tree.  Brother is for linked list
   of nodes at the same level in the same place such as when the reaction
   transform splits into two fragments.  The prev field provides for a list
   of multiple occurences of the same compound ordered by ascending age.
   The next field is for ??? expansion order ???  The index field is to
   give each compound node in the tree a unique number.  The symboltable
   field is to link the compound to the symbol table which contains all
   the direct information.

*/

typedef struct s_comp
  {
  union u_father
    {
    struct s_subg *addr;
    U32_t          index;
    } father;                 /* Father sub-goal node in PST */
  union u_son
    {
    struct s_subg *addr;
    U32_t          index;
    } son;                    /* Son sub-goal node in PST */
  union u_brother
    {
    struct s_comp *addr;
    U32_t          index;
    } brother;                /* Brother compound node in PST */
  union u_symbol
    {
    SymTab_t      *addr;
    U32_t          index;
    } symboltable;            /* Symbol table information */
  union u_prev
    {
    struct s_comp *addr;
    U32_t          index;
    } prev;                   /* Previous instance of comp, NULL for first.  */
  union u_next
    {
    struct s_comp *addr;
    U32_t          index;
    } next;                   /* Next comp that has been expanded.  */
  U32_t            index;     /* Identifying number */
  } Compound_t;
#define PSTCOMPOUNDSIZE sizeof (Compound_t)

/** Field Access Macros for Compound_t **/
/* Macro Prototypes
   Compound_t    *PstComp_Brother_Get     (Compound_t *);
   void           PstComp_Brother_Put     (Compound_t *, Compound_t *);
   U32_t   PstComp_BrotherIndex_Get(Compound_t *);
   void           PstComp_BrotherIndex_Put(Compound_t *, U32_t);
   Subgoal_t     *PstComp_Father_Get      (Compound_t *);
   void           PstComp_Father_Put      (Compound_t *, Subgoal_t *);
   U32_t    PstComp_FatherIndex_Get (Compound_t *);
   void           PstComp_FatherIndex_Put (Compound_t *, U32_t);
   Compound_t    *PstComp_Next_Get        (Compound_t *);
   void    PstComp_Next_Put   (Compound_t *, Compound_t *);
   U32_t   PstComp_NextIndex_Get   (Compound_t *);
   void    PstComp_NextIndex_Put   (Compound_t *, U32_t);
   U32_t          PstComp_Index_Get       (Compound_t *);
   void           PstComp_Index_Put       (Compound_t *, U32_t);
   Compound_t    *PstComp_Prev_Get        (Compound_t *);
   void           PstComp_Prev_Put        (Compound_t *);
   U32_t   PstComp_PrevIndex_Get   (Compound_t *);
   void           PstComp_PrevIndex_Put   (Compound_t *, U32_t);
   Subgoal_t     *PstComp_Son_Get         (Compound_t *);
   void           PstComp_Son_Put         (Compound_t *, Subgoal_t *);
   U32_t   PstComp_SonIndex_Get    (Compound_t *);
   void           PstComp_SonIndex_Put    (Compound_t *, U32_t);
   SymTab_t      *PstComp_SymbolTable_Get (Compound_t *);
   void           PstComp_SymbolTable_Put (Compound_t *, SymTab_t *);
   U32_t          PstComp_SymbolTableIndex_Get (Compound_t *);
   void           PstComp_SymbolTableIndex_Put (Compound_t *, U32_t);
*/

#define PstComp_Brother_Get(compound_p)\
  (compound_p)->brother.addr
#define PstComp_Brother_Put(compound_p, value)\
  (compound_p)->brother.addr = (value)

#define PstComp_BrotherIndex_Get(compound_p)\
  (compound_p)->brother.index
#define PstComp_BrotherIndex_Put(compound_p, value)\
  (compound_p)->brother.index = (value)

#define PstComp_Father_Get(compound_p)\
  (compound_p)->father.addr
#define PstComp_Father_Put(compound_p, value)\
  (compound_p)->father.addr = (value)

#define PstComp_FatherIndex_Get(compound_p)\
  (compound_p)->father.index
#define PstComp_FatherIndex_Put(compound_p, value)\
  (compound_p)->father.index = (value)

#define PstComp_Next_Get(compound_p)\
  (compound_p)->next.addr
#define PstComp_Next_Put(compound_p, value)\
  (compound_p)->next.addr = (value)
 
#define PstComp_NextIndex_Get(compound_p)\
  (compound_p)->next.index
#define PstComp_NextIndex_Put(compound_p, value)\
  (compound_p)->next.index = (value)
 
#define PstComp_Index_Get(compound_p)\
  (compound_p)->index
#define PstComp_Index_Put(compound_p, value)\
  (compound_p)->index = (value)

#define PstComp_Prev_Get(compound_p)\
  (compound_p)->prev.addr
#define PstComp_Prev_Put(compound_p, prev_p)\
  (compound_p)->prev.addr = (prev_p)

#define PstComp_PrevIndex_Get(compound_p)\
  (compound_p)->prev.index
#define PstComp_PrevIndex_Put(compound_p, prev_p)\
  (compound_p)->prev.index = (prev_p)

#define PstComp_Son_Get(compound_p)\
  (compound_p)->son.addr
#define PstComp_Son_Put(compound_p, value)\
  (compound_p)->son.addr = (value)

#define PstComp_SonIndex_Get(compound_p)\
  (compound_p)->son.index
#define PstComp_SonIndex_Put(compound_p, value)\
  (compound_p)->son.index = (value)

#define PstComp_SymbolTable_Get(compound_p)\
 (compound_p)->symboltable.addr
#define PstComp_SymbolTable_Put(compound_p, symtab_p)\
  (compound_p)->symboltable.addr = (symtab_p)

#define PstComp_SymbolTableIndex_Get(compound_p)\
 (compound_p)->symboltable.index
#define PstComp_SymbolTableIndex_Put(compound_p, symtab_p)\
  (compound_p)->symboltable.index = (symtab_p)


/** End of Field Access Macros for Compound_t **/

/* Subgoal node structure.  This is an OR node in the PST.  The father, son
   and brother fields function just like the ones in Compound_t.  The
   time field records how long it took to solve this node (and below?).
   The index field is the unique number for this OR-node.  The reaction
   sub-structure contains the resulting merits for this reaction instance 
   and a key to find detailed reaction information.  The merit sub-structure
   contains information related to the strategy in use and how it values
   this sub-goal (rename to strategy?).  Flags, closed count, visits and
   level are all also related to the strategy in use.
*/

typedef struct s_subg
  {
  Strategy_Subgoal_t strategyb;              /* Strategy values */
  union u_sfather
    {
    Compound_t    *addr;
    U32_t          index;
    } father;                                /* Father compound node in PST */
  union u_sson
    {
    Compound_t    *addr;
    U32_t          index;
    } son;                                   /* Son compound node in PST */
  union u_sbrother
    {
    struct s_subg *addr;
    U32_t          index;
    } brother;                               /* Brother sub-goal node in PST */
  U32_t          index;                      /* Identifying number */
  U32_t          schema_key;                 /* Key to reaction into. */
  U16_t          rxn_ease;                   /* Est. ease of reaction */
  U16_t          rxn_yield;                  /* Est. yield of reaction */
  U16_t          rxn_confidence;             /* Est. confidence of reaction */
  S16_t          main_merit;                 /* Actual merit of subgoal */
  S16_t          solved_merit;               /* Solved merit */
  S16_t          initial_merit;              /* Initial merit */
  U8_t           flags;                      /* Subgoal attribute flags */
  U8_t           level;                      /* Level in PST of this subgoal */
  } Subgoal_t;
#define PSTSUBGOALSIZE sizeof (Subgoal_t)

/* Flag literal values */

#define SubgM_ActiveStrategicBond      0x1

/** Field Access Macros for Subgoal_t **/

/* Macro prototypes
   Subgoal_t  *PstSubg_Brother_Get             (Subgoal_t *);
   void        PstSubg_Brother_Put             (Subgoal_t *, Subgoal_t *);
   U32_t       PstSubg_BrotherIndex_Get        (Subgoal_t *);
   void        PstSubg_BrotherIndex_Put        (Subgoal_t *, U32_t);
   Compound_t *PstSubg_Father_Get              (Subgoal_t *);
   void        PstSubg_Father_Put              (Subgoal_t *, Compound_t *);
   U32_t       PstSubg_FatherIndex_Get         (Subgoal_t *);
   void        PstSubg_FatherIndex_Put         (Subgoal_t *, U32_t*);
   U8_t        PstSubg_Flags_Get               (Subgoal_t *); 
   void        PstSubg_Flags_Put               (Subgoal_t *, U8_t);
   Boolean_t   PstSubg_Flags_Active_Get        (Subgoal_t *);
   void        PstSubg_Flags_Active_Put        (Subgoal_t *, Boolean_t);
   U32_t       PstSubg_Index_Get               (Subgoal_t *);
   void        PstSubg_Index_Put               (Subgoal_t *, U32_t);
   U8_t        PstSubg_Level_Get               (Subgoal_t *);
   void        PstSubg_Level_Put               (Subgoal_t *, U8_t);
   S16_t       PstSubg_Merit_Initial_Get       (Subgoal_t *);
   void        PstSubg_Merit_Initial_Put       (Subgoal_t *, S16_t);
   S16_t       PstSubg_Merit_Main_Get          (Subgoal_t *);
   void        PstSubg_Merit_Main_Put          (Subgoal_t *, S16_t);
   S16_t       PstSubg_Merit_Solved_Get        (Subgoal_t *);
   void        PstSubg_Merit_Solved_Put        (Subgoal_t *, S16_t);
   U16_t       PstSubg_Reaction_Confidence_Get (Subgoal_t *);
   void        PstSubg_Reaction_Confidence_Put (Subgoal_t *, U16_t);
   U16_t       PstSubg_Reaction_Ease_Get       (Subgoal_t *);
   void        PstSubg_Reaction_Ease_Put       (Subgoal_t *, U16_t);
   U32_t       PstSubg_Reaction_Schema_Get     (Subgoal_t *);
   void        PstSubg_Reaction_Schema_Put     (Subgoal_t *, U32_t);
   U16_t       PstSubg_Reaction_Yield_Get      (Subgoal_t *);
   void        PstSubg_Reaction_Yield_Put      (Subgoal_t *, U16_t);
   Compound_t *PstSubg_Son_Get                 (Subgoal_t *);
   void        PstSubg_Son_Put                 (Subgoal_t *, Compound_t *);
   U32_t       PstSubg_SonIndex_Get            (Subgoal_t *);
   void        PstSubg_SonIndex_Put            (Subgoal_t *, U32_t);
   Strategy_Subgoal_t *PstSubg_Strategy_Get    (Subgoal_t *);
   U16_t       PstSubg_Visits_Get              (Subgoal_t *);
   void        PstSubg_Visits_Put              (Subgoal_t *, U16_t);
*/

#define PstSubg_Brother_Get(subgoal_p)\
  (subgoal_p)->brother.addr
#define PstSubg_Brother_Put(subgoal_p, value)\
  (subgoal_p)->brother.addr = (value)

#define PstSubg_BrotherIndex_Get(subgoal_p)\
  (subgoal_p)->brother.index
#define PstSubg_BrotherIndex_Put(subgoal_p, value)\
  (subgoal_p)->brother.index = (value)

#define PstSubg_Father_Get(subgoal_p)\
  (subgoal_p)->father.addr
#define PstSubg_Father_Put(subgoal_p, value)\
  (subgoal_p)->father.addr = (value)

#define PstSubg_FatherIndex_Get(subgoal_p)\
  (subgoal_p)->father.index
#define PstSubg_FatherIndex_Put(subgoal_p, value)\
  (subgoal_p)->father.index = (value)

#define PstSubg_Flags_Get(subgoal_p)\
  (subgoal_p)->flags
#define PstSubg_Flags_Put(subgoal_p, value)\
  (subgoal_p)->flags = (value)

#define PstSubg_Flags_Active_Get(subgoal_p)\
  ((subgoal_p)->flags & SubgM_ActiveStrategicBond ? TRUE : FALSE)

#define PstSubg_Flags_Active_Put(subgoal_p, value)\
  { if ((value) == TRUE)\
    (subgoal_p)->flags |= SubgM_ActiveStrategicBond ;\
  else\
    (subgoal_p)->flags &= ~SubgM_ActiveStrategicBond; }

#define PstSubg_Index_Get(subgoal_p)\
  (subgoal_p)->index
#define PstSubg_Index_Put(subgoal_p, value)\
  (subgoal_p)->index = (value)

#define PstSubg_Level_Get(subgoal_p)\
  (subgoal_p)->level
#define PstSubg_Level_Put(subgoal_p, value)\
  (subgoal_p)->level = value

#define PstSubg_Merit_Initial_Get(subgoal_p)\
  (subgoal_p)->initial_merit
#define PstSubg_Merit_Initial_Put(subgoal_p, value)\
  (subgoal_p)->initial_merit = (value)

#define PstSubg_Merit_Main_Get(subgoal_p)\
  (subgoal_p)->main_merit
#define PstSubg_Merit_Main_Put(subgoal_p, value)\
  (subgoal_p)->main_merit = (value)

#define PstSubg_Merit_Solved_Get(subgoal_p)\
  (subgoal_p)->solved_merit
#define PstSubg_Merit_Solved_Put(subgoal_p, value)\
  (subgoal_p)->solved_merit = (value)

#define PstSubg_Reaction_Confidence_Get(subgoal_p)\
  (subgoal_p)->rxn_confidence
#define PstSubg_Reaction_Confidence_Put(subgoal_p, value)\
  (subgoal_p)->rxn_confidence = (value)

#define PstSubg_Reaction_Ease_Get(subgoal_p)\
  (subgoal_p)->rxn_ease
#define PstSubg_Reaction_Ease_Put(subgoal_p, value)\
  (subgoal_p)->rxn_ease = (value)

#define PstSubg_Reaction_Yield_Get(subgoal_p)\
  (subgoal_p)->rxn_yield
#define PstSubg_Reaction_Yield_Put(subgoal_p, value)\
  (subgoal_p)->rxn_yield = (value)

#define PstSubg_Reaction_Schema_Get(subgoal_p)\
  (subgoal_p)->schema_key
#define PstSubg_Reaction_Schema_Put(subgoal_p, value)\
  (subgoal_p)->schema_key = (value)

#define PstSubg_Son_Get(subgoal_p)\
  (subgoal_p)->son.addr
#define PstSubg_Son_Put(subgoal_p, value)\
  (subgoal_p)->son.addr = (value)

#define PstSubg_SonIndex_Get(subgoal_p)\
  (subgoal_p)->son.index
#define PstSubg_SonIndex_Put(subgoal_p, value)\
  (subgoal_p)->son.index = (value)

#define PstSubg_Strategy_Get(subgoal_p)\
  &(subgoal_p)->strategyb

#define PstSubg_Visits_Get(subgoal_p)\
  (subgoal_p)->strategyb.visits
#define PstSubg_Visits_Put(subgoal_p, value)\
  (subgoal_p)->strategyb.visits = (value)


/* Need a generic node indentifier that everybody can pass around that can
   reference either an AND or an OR node in the PST.
*/

typedef struct s_pstnode
  {
  union u_pst1
    {
    Compound_t  *compound;                   /* Compound address */
    Subgoal_t   *subgoal;                    /* Subgoal address */
    } p;
  U8_t           type;                       /* Type for union */
  } PstNode_t;
#define PSTNODESIZE sizeof (PstNode_t)

/** Field Access Macros for a PstNode_t **/

/* Macro Prototypes
   Compound_t     *PstNode_Compound_Get (PstNode_t *);
   void           PstNode_Compound_Put (PstNode_t *, Compound_t *);
   Subgoal_t     *PstNode_Subgoal_Get  (PstNode_t *);
   void           PstNode_Subgoal_Put  (PstNode_t *, Subgoal_t *);
   U8_t           PstNode_Type_Get     (PstNode_t *);
*/

#define PstNode_Compound_Get(pstn_p)\
  (pstn_p)->p.compound
#define PstNode_Compound_Put(pstn_p, value)\
  {(pstn_p)->p.compound = (value); (pstn_p)->type = PST_COMPOUND;} 

#define PstNode_Subgoal_Get(pstn_p)\
  (pstn_p)->p.subgoal
#define PstNode_Subgoal_Put(pstn_p, value)\
  {(pstn_p)->p.subgoal = (value); (pstn_p)->type = PST_SUBGOAL;}

#define PstNode_Type_Get(pstn_p)\
  (pstn_p)->type


/* This is the data-structure that describes all the global data needed by
   the PST module to manage the PST.
*/

typedef struct s_pstcb
  {
  Sling_t       main_target;            /* Main target compound */
  List_t       *expansions;             /* List of expansion blocks */
  union u_root
    {
    Subgoal_t     *addr;                       
    U32_t   index; 
    } root;    /* Root subgoal in PST */ 
  union u_current_comp
    {
    Compound_t *addr;              
    U32_t index;
    } current_comp;    /* current developed compound */ 
  union u_last_compound
    {
    Compound_t *addr;
    U32_t index;
    } last_compound;       /* last developed compound */
  U32_t         comp_idx;               /* Next compound index */
  U32_t         subg_idx;               /* Next subgoal index */
  U32_t         symtab_idx;             /* Next symbol table index */
  U32_t  total_compounds;     /* total developed compounds */
  struct 
    {
    float  carbons;         
    float  fgroups;
    float    rings;
    } merit_info;       /* scaling factors */
  } PstCB_t;
#define PSTCBSIZE sizeof (PstCB_t)

/** Field Access Macros for PstCB_t **/
/* Macro Prototypes
   U32_t       PstCB_CompoundIndex_Get             (PstCB_t *);
   void        PstCB_CompoundIndex_Put             (PstCB_t *, U32_t);
   Compound_t *PstCB_CurrentComp_Get               (PstCB_t *);
   void        PstCB_CurrentComp_Put               (PstCB_t *, Compound_t *);
   U32_t       PstCB_CurrentCompIndex_Get          (PstCB_t *);
   void        PstCB_CurrentCompIndex_Put          (PstCB_t *, U32_t);
  List_t      *PstCB_Expansions_Get                (PstCB_t *);
  void         PstCB_Expansions_Put                (PstCB_t *, List_t *);
   Compound_t *PstCB_LastExpandedCompound_Get      (PstCB_t *);
   void        PstCB_LastExpandedCompound_Put      (PstCB_t *, Compound_t *);
   U32_t       PstCB_LastExpandedCompoundIndex_Get (PstCB_t *);
   void        PstCB_LastExpandedCompoundIndex_Put (PstCB_t *, U32_t);
   Sling_t     PstCB_MainTarget_Get                (PstCB_t *);
   void        PstCB_MainTarget_Put                (PstCB_t *, Sling_t);
   float       PstCB_MeritCarbons_Get              (PstCB_t *);
   void        PstCB_MeritCarbons_Put              (PstCB_t *, float);
   float       PstCB_MeritFGroups_Get              (PstCB_t *);
   void        PstCB_MeritFGroups_Put              (PstCB_t *, float);
   float       PstCB_MeritRings_Get                (PstCB_t *);
   void        PstCB_MeritRings_Put                (PstCB_t *, float);
   Subgoal_t  *PstCB_Root_Get                      (PstCB_t *);
   void        PstCB_Root_Put                      (PstCB_t *, Subgoal_t *);
   U32_t       PstCB_RootIndex_Get                 (PstCB_t *);
   void        PstCB_RootIndex_Put                 (PstCB_t *, U32_t);
   U32_t       PstCB_SubgoalIndex_Get              (PstCB_t *);
   void        PstCB_SubgoalIndex_Put              (PstCB_t *, U32_t);
   U32_t       PstCB_SymtabIndex_Get               (PstCB_t *);
   void        PstCB_SymtabIndex_Put               (PstCB_t *, U32_t);
   U32_t       PstCB_TotalExpandedCompounds_Get    (PstCB_t *);
   void        PstCB_TotalExpandedCompounds_Put    (PstCB_t *, U32_t);
*/

#define PstCB_CompoundIndex_Get(pcb_p)\
  (pcb_p)->comp_idx
#define PstCB_CompoundIndex_Put(pcb_p, value)\
  (pcb_p)->comp_idx = (value)

#define PstCB_CurrentComp_Get(pcb_p)\
  (pcb_p)->current_comp.addr
#define PstCB_CurrentComp_Put(pcb_p, value)\
  (pcb_p)->current_comp.addr = (value)

#define PstCB_CurrentCompIndex_Get(pcb_p)\
  (pcb_p)->current_comp.index
#define PstCB_CurrentCompIndex_Put(pcb_p, value)\
  (pcb_p)->current_comp.index = (value)

#define PstCB_Expansions_Get(pcb_p)\
  (pcb_p)->expansions
#define PstCB_Expansions_Put(pcb_p, value)\
  (pcb_p)->expansions = (value)

#define PstCB_LastExpandedCompound_Get(pcb_p)\
  (pcb_p)->last_compound.addr 
#define PstCB_LastExpandedCompound_Put(pcb_p, value)\
  (pcb_p)->last_compound.addr = (value)

#define PstCB_LastExpandedCompoundIndex_Get(pcb_p)\
  (pcb_p)->last_compound.index
#define PstCB_LastExpandedCompoundIndex_Put(pcb_p, value)\
  (pcb_p)->last_compound.index = (value)

#define PstCB_MainTarget_Get(pcb_p)\
  (pcb_p)->main_target
#define PstCB_MainTarget_Put(pcb_p, value)\
  (pcb_p)->main_target = (value)

#define PstCB_MeritCarbons_Get(pcb_p)\
  (pcb_p)->merit_info.carbons
#define PstCB_MeritCarbons_Put(pcb_p, value)\
  (pcb_p)->merit_info.carbons = (value)

#define PstCB_MeritFGroups_Get(pcb_p)\
  (pcb_p)->merit_info.fgroups
#define PstCB_MeritFGroups_Put(pcb_p, value)\
  (pcb_p)->merit_info.fgroups = (value)

#define PstCB_MeritRings_Get(pcb_p)\
  (pcb_p)->merit_info.rings
#define PstCB_MeritRings_Put(pcb_p, value)\
  (pcb_p)->merit_info.rings = (value)

#define PstCB_Root_Get(pcb_p)\
  (pcb_p)->root.addr
#define PstCB_Root_Put(pcb_p, value)\
  (pcb_p)->root.addr = (value)

#define PstCB_RootIndex_Get(pcb_p)\
  (pcb_p)->root.index
#define PstCB_RootIndex_Put(pcb_p, value)\
  (pcb_p)->root.index = (value)

#define PstCB_SubgoalIndex_Get(pcb_p)\
  (pcb_p)->subg_idx
#define PstCB_SubgoalIndex_Put(pcb_p, value)\
  (pcb_p)->subg_idx = (value)

#define PstCB_SymtabIndex_Get(pcb_p)\
  (pcb_p)->symtab_idx
#define PstCB_SymtabIndex_Put(pcb_p, value)\
  (pcb_p)->symtab_idx = (value)

#define PstCB_TotalExpandedCompounds_Get(pcb_p)\
  (pcb_p)->total_compounds
#define PstCB_TotalExpandedCompounds_Put(pcb_p, value)\
  (pcb_p)->total_compounds = (value)

/** End of Field Access Macros for PstCB_t **/

/*** Routine Prototypes ***/

/* in pst.c */

Boolean_t   Pst_All_Solved         (Subgoal_t *);
PstNode_t   Pst_Brother_Get        (PstNode_t *);
void        Pst_Brother_Put        (PstNode_t *, PstNode_t *);
Compound_t *Pst_Compound_Create    (void);
void        Pst_Compound_Dump      (Compound_t *, FileDsc_t *);
U16_t       Pst_Compound_Merit_Set (Xtr_t *);
PstCB_t    *Pst_ControlHandle_Get  (void);
void        Pst_Destroy            (PstNode_t);
void        Pst_Dump               (Subgoal_t *, FileDsc_t *);
PstNode_t   Pst_Father_Get         (PstNode_t *);
void        Pst_Father_Put         (PstNode_t *, PstNode_t *);
U32_t       Pst_Hash               (Sling_t, U32_t);
U16_t       Pst_Merit_Compute      (Xtr_t *);
U16_t       Pst_NumSons_Get        (PstNode_t *);
U16_t       Pst_ReactionMerit_Get  (Subgoal_t *);
Compound_t *Pst_Root_Set           (Xtr_t *, Sling_t);
PstNode_t   Pst_Son_Get            (PstNode_t *);
void        Pst_Son_Put            (PstNode_t *, PstNode_t *);
Subgoal_t  *Pst_SubGoal_Create     (void);
void        Pst_SubGoal_Dump       (Subgoal_t *, FileDsc_t *);
void        Pst_SubGoal_Insert     (Array_t *, Array_t *, Sling_t *, 
              SubGenr_Compound_t *, U16_t, U32_t, Boolean_t *, Boolean_t);
Boolean_t   Pst_Subgoal_IsDuplicate (Sling_t *, U16_t, U32_t);
SymTab_t   *Pst_SymTab_Create      (void);
void        Pst_SymTab_Dump        (SymTab_t *, FileDsc_t *);
void        Pst_SymTab_Init        (void);
void        Pst_SymTab_Index_Find  (Sling_t, SymTab_t **, Boolean_t *);
void        Pst_Update             (void);

void        PstCB_Dump             (FileDsc_t *);
void        PstCB_NextExpandedComp_Put (Compound_t *);

U16_t       PstComp_NumGrandSons_Get (Compound_t *);
U16_t       PstComp_NumSons_Get    (Compound_t *);

S16_t       PstSubg_AdjMerit_Compute (Subgoal_t *, U16_t, Array_t *);
Boolean_t   PstSubg_Circlar        (Subgoal_t *);
Boolean_t   PstSubg_Closed         (Subgoal_t *);
U16_t       PstSubg_Instances_Get  (Subgoal_t *);
Boolean_t   PstSubg_JustSolved     (Subgoal_t *);
U16_t       PstSubg_NumSons_Get    (Subgoal_t *);
Boolean_t   PstSubg_TempClosed     (Subgoal_t *);
Boolean_t   PstSubg_Unsolvable     (Subgoal_t *);

void        SymbolTable_Destroy    (void);
void        SymbolTable_Dump       (FileDsc_t *);
void        SymbolTable_Init       (void);
void        SymbolTable_PostSelect_Reset (Boolean_t, Boolean_t);

SymTab_t   *SymTab_HashBucketHead_Get (U32_t);
void        SymTab_HashBucketHead_Put (U32_t, SymTab_t *);
void        SymTab_JustSolved_Put  (SymTab_t *, Boolean_t);

/*  In strategy.c  */

void        Strategy_ClosedCnt_Set  (PstNode_t);
void        Strategy_SearchTimes_Backup (Compound_t *, Time_t, Stack_t *);
Compound_t *Strategy_Select_Next    (Compound_t *, Strategy_Params_t *, 
              Boolean_t, Stack_t *);

/*  Distributed functions and versions  */

Compound_t *Dis_Pst_WorkerRoot_Set    (Sling_t, U16_t);
void        Dis_PstCB_Worker_Clear    (Boolean_t);
void        Dis_PstCB_Worker_Init     (Sling_t, float, float, float);

Boolean_t   Pst_Any_Selected          (Subgoal_t *);
U16_t       Pst_LBM_CmpMerit_Compute  (Compound_t *, U16_t);
U16_t       Pst_LBM_SgMerit_Recompute (Compound_t *, U16_t);
U16_t       Pst_LowerBoundMerit_Find  (Stack_t *, Compound_t *);
void        Pst_PostSel_Update        (Boolean_t);
Boolean_t   Pst_Symbol_Selected       (Sling_t, Time_t*, U16_t, Boolean_t,
              Boolean_t, Boolean_t);
void        SymbolTable_Clear         (void);

/*** Global Variables ***/

#ifdef PST_GLOBALS
static PstCB_t   SPstcb;
static SymTab_t *SSymbolTable[MX_SYMTAB_BUCKETS];
#endif

/* End of Pst.H */
#endif
