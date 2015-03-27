/******************************************************************************
*
*  Copyright (C) 1991-1996 Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     PST.C
*
*    This module contains the implementation of the abstraction of the
*    Problem Solving Tree.  The search tree is an AND-OR graph, the root
*    node is broken down into a series of alternative first level syntheses,
*    OR-nodes, because you can do one or the other synthesis, they are the
*    reactions.  Each OR-node is comprised of a series of AND-nodes which are
*    the molecules necessary for the reaction to work, because you need all
*    of them.  OR-nodes are called sub-goals, AND-nodes are called compounds.
*
*    This module is contains description and manipulation of the symbol table
*    for the SYMCHEM Problem Solving Tree.  The symbol table contains all the
*    compound relevant information for each compound node in the PST, while
*    the PstCompound_t node contains all the search relevant information.
*    The most important information is that this node is unique and has a
*    canonical Sling.  Also information on whether the compound is an
*    "available" one is key.
*
*  Routines:
*
*    Pst_All_Solved
*    Pst_Brother_Get
*    Pst_Brother_Put
*    Pst_Compound_Create
*    Pst_Compound_Dump
*    Pst_Compound_Merit_Set
*    Pst_ControlHandle_Get
*    Pst_Destroy
*    Pst_Dump
*    Pst_Father_Get
*    Pst_Father_Put
*    Pst_Hash
*    Pst_Merit_Compute
*    Pst_NumSons_Get
*    Pst_ReactionMerit_Get
*    Pst_Root_Set
*    Pst_Son_Get
*    Pst_Son_Put
*    Pst_SubGoal_Create
*    Pst_SubGoal_Dump
*    Pst_SubGoal_Insert
*    Pst_Subgoal_IsDuplicate
*    Pst_SymTab_Create
*    Pst_SymTab_Dump
*    Pst_SymTab_Index_Find
*    Pst_Update
*    PstCB_Dump
*    PstCB_NextExpandedComp_Put
*    PstComp_NumGrandSons_Get
*    PstComp_NumSons_Get
*    PstSubg_AdjMerit_Compute 
*    PstSubg_Circlar 
*    PstSubg_Closed
*    PstSubg_Instances_Get
*    PstSubg_JustSolved
*    PstSubg_NumSons_Get
*    PstSubg_TempClosed
*    PstSubg_Unsolvable
*    SymbolTable_Destroy
*    SymbolTable_Dump
*    SymbolTable_Init
*    SymbolTable_PostSelect_Reset
*    SymTab_HashBucketHead_Get
*    SymTab_HashBucketHead_Put
*    SymTab_JustSolved_Put
*
*  Static Routines:
*
*    Pst_Calculate
*    Pst_CarbonPenalty_Compute
*    Pst_CE_Adjust
*    Pst_Circular
*    Pst_FGroupPenalty_Find
*    Pst_FuncGrpPenalty_Compute
*    Pst_NextCompound_Get
*    Pst_RingPenalty_Compute
*    Pst_Stack_Contain
*    Pst_SubgoalMerit_Compute
*    Pst_SubSet
*
*  Distributed Routines:
*
*    Dis_Pst_WorkerRoot_Set
*    Dis_PstCB_Worker_Clear
*    Dis_PstCB_Worker_Init
*    Pst_Any_Selected
*    Pst_LBM_CmpMerit_Compute
*    Pst_LBM_SgMerit_Recompute
*    Pst_LowerBoundMerit_Find
*    Pst_Symbol_Selected
*    SymbolTable_Clear
*
*  Creation Date:
*
*    01-Jan-1991
*
*  Authors:
*
*    Daren Krebsbach
*    Tito Autrey (rewritten based on others PL/I code)
*    Shu Cheung
*
*  Modification History, reverse chronological
*
* Date       Author     Modifcation Description
*------------------------------------------------------------------------------
* 23-Sep-97  Krebsbach  Combined sequential and distributed versions
*                       of SYNCHEM into single file, making many changes to
*                       data structures and function prototypes.
* 09-Jul-97  Krebsbach  Removed inertia and max visits.
* 12-Mar-96  Krebsbach  Corrected bug in hash function (and use old version).
* 04-Oct-96  Krebsbach  Added routines several routines.
* 31-May-95  Cheung     Added many routines. 
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_ATOMSYM_
#include "atomsym.h"
#endif

#ifndef _H_ARRAY_
#include "array.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_SLING2XTR_
#include "sling2xtr.h"
#endif

#ifndef _H_AVLCOMP_
#include "avlcomp.h"
#endif

#ifndef _H_RCB_
#include "rcb.h"
#endif

#define PST_GLOBALS 1
#ifndef _H_PST_
#include "pst.h"
#endif
#undef PST_GLOBALS

/* Static Routines Prototypes */

static U16_t       Pst_CE_Adjust           (Xtr_t *, U16_t);
static void        Pst_Calculate           (Xtr_t *, Array_t *, Array_t *,  
		     Array_t *, Array_t *, Array_t *, Array_t *, Array_t *); 
static U16_t       Pst_CarbonPenalty_Compute (Xtr_t *, Array_t, Array_t, 
		     Array_t, Array_t, Array_t);
static Boolean_t   Pst_Circular            (Sling_t *, U16_t);
static U16_t       Pst_FGroupPenalty_Find  (Xtr_t *);
static U16_t       Pst_FuncGrpPenalty_Compute (Xtr_t *, Array_t, Array_t, 
		      Array_t, Array_t, Array_t, Array_t, Array_t);
static Compound_t *Pst_NextCompound_Get    (SymTab_t *, Boolean_t);
static U16_t       Pst_RingPenalty_Compute (Xtr_t *, Array_t, Array_t, Array_t);
static Boolean_t   Pst_Stack_Contain       (Stack_t *, SymTab_t *);
static Boolean_t   Pst_SubSet              (Sling_t *, U16_t, Sling_t *, U16_t);
static S16_t       Pst_SubgoalMerit_Compute  (Subgoal_t *);

static U16_t Ring_Table [4][8] = 
  { {10, 15,  4,  2, 15, 20, 30, 40},
    {20, 60, 20,  0, 20, 40, 60, 80},
    { 0, 10,  6,  4,  8, 12, 16, 24},
    {20, 20,  2,  2, 10, 20, 30, 40}
  };
 
static U16_t Carbon_Table [3][4][4] = 
  {
    /* for acyclic carbons: */
    { {3, 12, 21, 90},
      {3, 12, 45, 300},
      {3,  9, 300, 300},
      {3, 300, 300, 300}
     },
    /* for cyclic nonaromatic carbons: */
    { {3, 12, 21, 60},
      {3, 12, 33, 300},
      {3, 9, 300, 300},
      {3, 300, 300, 300}
    },
    /* for cyclic aromatic carbons: */
    { {0, 9, 18, 60},
      {0, 0, 33, 300},
      {0, 9, 300, 300},
      {3, 300, 300, 300}
    }
  };
 
static U16_t Single_Attr[49][2] = 
  {
    {  9,  1},     /* PEROXIDE */
    { 20,  2},     /* ORGANO-Mg */
    { 21,  3},     /* ORGANO-Li */
    { 43,  2},     /* sec-ALCOHOL */
    { 58,  1},     /* KETENE */
    { 61,  1},     /* CUMULENE */
    { 71,  1},     /* HEMIACETAL/KETAL */
    { 73,  1},     /* ALKYNYL ETHER */
    { 81,  1},     /* alpha-LACTONE */
    { 82,  1},     /* beta-LACTONE */
    { 93,  2},     /* sec-HALIDE */
    { 95,  2},     /* HALOFORMATE */
    { 96,  1},     /* HYPOHALITE/HALOAMINE */
    { 97,  2},     /* HALOHYDRIN */
    { 98,  2},     /* gem-DIHALIDE */
    { 99,  2},     /* TRIHALOMETHYL */
    {116,  1},     /* ISOCYANATE */
    {119,  2},     /* DIAZO */
    {301,  3},     /* DISULFIDE */
    {302,  1},     /* POLYSULFIDE */
    {303,  1},     /* THIOACID */
    {305,  1},     /* THIONESTER */
    {307,  1},     /* DITHIOACID */
    {308,  1},     /* DITHIOESTER */
    {309,  1},     /* THIOAMIDE */
    {311,  1},     /* ISOTHIOCYANATE */
    {312,  2},     /* ENETHIOL */
    {318,  1},     /* SULFENYL HALIDE */
    {322,  1},     /* SULFENIC ACID/ESTER */
    {323,  1},     /* SULFINIC ACID/ESTER */
    {352,  1},     /* IMIDE */
    {355,  1},     /* DIAZIRIDINE */
    {357,  1},     /* beta-LACTAM */
    {363,  1},     /* ISONITRILE */
    {364,  1},     /* CARBODIIMIDE */
    {455,  1},     /* PHOSPHONATE */
    {456,  1},     /* PHOSPHINATE */
    {478,  1},     /* SELENOPHENE */
    {480,  2},     /* DISELENIDE */
    {481,  1},     /* ORGANO-Pb */
    {482,  1},     /* ORGANO-Hg */
    {483,  1},     /* ORGANO-Tl */
    {484,  1},     /* ORGANO-Cd */
    {485,  1},     /* ORGANO-Zn */
    {490,  1},     /* SILANE */
    {491,  1},     /* SILYL HALIDE */
    {492,  1},     /* SILYL HYDRIDE */
    {493,  1},     /* SILOXANE */
    {494,  1}      /* SILAZANE */
  };
 
static U16_t Pairs[47][2] = 
  {
    /* PHENOL AND... */
    {41,  68},   /* ENOL ETHER */
    {41,  69},   /* ACETAL */
    {41,  70},   /* KETAL */
    {41,  74},   /* ORTHOESTER */
    {41, 119},   /* DIAZO */
    {41, 350},   /* AZIRIDINE */
    {41, 452},   /* P-YLID */
    /* ALDEHYDE AND... */           
    {50,  68},   /* VINYL ETHER */
    {50,  69},   /* ACETAL */
    {50,  70},   /* KETAL */
    {50,  74},   /* ORTHOESTER */
    {50, 117},   /* HYDRAZONE */
    {50, 119},   /* DIAZO */
    {50, 123},   /* PRIMARY AMINE */
    {50, 124},   /* SECONDARY AMINE */
    {50, 136},   /* MERCAPTAN */
    {50, 353},   /* AMIDINE */
    {50, 356},   /* AMINAL */
    {50, 452},   /* P-YLID */
    {50, 475},   /* SELENOL */
    /* KETONE AND... */             
    {51, 117},   /* HYDRAZONE */
    {51, 123},   /* PRIMARY AMINE */
    {51, 475},   /* SELENOL */
    /* EPOXIDE AND... */            
    {65,   5},   /* RCOOH */
    {65,  79},   /* ANHYDRIDE */
    {65, 114},   /* DIAZONIUM SALT */
    {65, 135},   /* SULFIDE */
    {65, 136},   /* MERCAPTAN */
    {65, 315},   /* EPISULFIDE */
    {65, 350},   /* AZIRIDINE */
    {65, 450},   /* PHOSPHINE */
    {65, 475},   /* SELENOL */
    {65, 479},   /* SELENIDE */
    /* ANHYDRIDE AND... */          
    {79,   1},   /* HYDROXYL */
    {79,  14},   /* PYRIDINE, ETC. */
    {79, 126},   /* OXIME */
    {79, 134},   /* SULFOXIDE */
    {79, 136},   /* MERCAPTAN */
    {79, 137},   /* SULFONIC ACID */
    {79, 475},   /* SELENOL */
    /* ACID HALIDES AND... */       
    {85,   1},   /* HYDROXYL */
    {85, 126},   /* OXIME */
    {85, 134},   /* SULFOXIDE */
    {85, 136},   /* MERCAPTAN */
    {85, 475},   /* SELENOL */
    /* EPISULFIDE AND... */         
    {315, 350},   /* AZIRIDINE */
    {315, 450}    /* PHOSPHINE */
  };


/****************************************************************************
*
*  Function Name:              Pst_All_Solved
*
*    This routine checks if all sons of a given subgoal are solved.
*
*  Used to be:
*
*    solved
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
*    TRUE       if all are solved
*    FALSE       otherwise
*
*  Side Effects:
*
*    N/A
*
****************************************************************************/
Boolean_t  Pst_All_Solved
  (
  Subgoal_t     *subgoal_p
  )
{
  Compound_t    *compound_p;
  SymTab_t      *symtab_p;
 
  compound_p = PstSubg_Son_Get (subgoal_p);
  while (compound_p != NULL)
    {
    symtab_p = PstComp_SymbolTable_Get (compound_p);
    if (SymTab_Flags_NewlySolved_Get (symtab_p) == FALSE && 
	SymTab_Flags_Solved_Get (symtab_p) == FALSE)
      return FALSE;

    compound_p = PstComp_Brother_Get (compound_p);
    }

  return TRUE;
}
/* End of Pst_All_Solved */

/****************************************************************************
*
*  Function Name:                 Pst_Brother_Get
*
*    This function returns a typed pointer to the brother of a node.  Each
*    node in the PST has a father and a son, and then there is a linked list
*    of brothers that are at the same level in the tree.
*
*  Used to be:
*
*    brthr:
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
*    PstNode_t describing the brother node
*
*  Side Effects:
*
*    May call IO_Exit_Error
*
******************************************************************************/
PstNode_t Pst_Brother_Get
  (
  PstNode_t     *node_p                     /* Address of node in question */
  )
{
  PstNode_t      tempnode;                  /* Node pointer to construct */

  DEBUG (R_PST, DB_PSTBROTHERGET, TL_PARAMS, (outbuf,
    "Entering Pst_Brother_Get, type = %hu, address = %p",
    PstNode_Type_Get (node_p), PstNode_Compound_Get (node_p)));

  switch (PstNode_Type_Get (node_p))
    {
    case PST_COMPOUND :

      PstNode_Compound_Put (&tempnode,
	PstComp_Brother_Get (PstNode_Compound_Get (node_p)));
      break;

    case PST_SUBGOAL :

      PstNode_Subgoal_Put (&tempnode,
	PstSubg_Brother_Get (PstNode_Subgoal_Get (node_p)));
      break;

    default :

      IO_Exit_Error (R_PST, X_SYNERR, "Incorrect type for PST node pointer");
      break;
    }

  DEBUG (R_PST, DB_PSTBROTHERGET, TL_PARAMS, (outbuf,
    "Leaving Pst_Brother_Get, return node.type = %hu, node.address = %p",
    tempnode.type, tempnode.p.subgoal));

  return tempnode;
}
/* End of Pst_Brother_Get */

/****************************************************************************
*
*  Function Name:                 Pst_Brother_Put
*
*    This function stores a typed pointer to the brother of a node.  Each
*    node in the PST has a father and a son, and then there is a linked list
*    of brothers that are at the same level in the tree.
*
*  Used to be:
*
*    $brthr:
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
*    May call IO_Exit_Error
*
******************************************************************************/
void Pst_Brother_Put
  (
  PstNode_t     *node_p,                    /* Address of node in question */
  PstNode_t     *brother_p                  /* Node to become the father */
  )
{
  DEBUG (R_PST, DB_PSTBROTHERPUT, TL_PARAMS, (outbuf,
    "Entering Pst_Brother_Put, node.type = %hu, node.address = %p,"
    " brother.type = %hu, brother.address = %p",
    PstNode_Type_Get (node_p), PstNode_Subgoal_Get (node_p),
    PstNode_Type_Get (brother_p), PstNode_Subgoal_Get (brother_p)));

  if (PstNode_Type_Get (node_p) != PstNode_Type_Get (brother_p))
    IO_Exit_Error (R_PST, X_SYNERR, "Incompatible types in Pst_Brother_Put");

  switch (PstNode_Type_Get (node_p))
    {
    case PST_COMPOUND :

      PstComp_Brother_Put (PstNode_Compound_Get (node_p), 
       PstNode_Compound_Get (brother_p));
      break;

    case PST_SUBGOAL :

      PstSubg_Brother_Put (PstNode_Subgoal_Get (node_p), 
       PstNode_Subgoal_Get (brother_p));
      break;

    default :

      IO_Exit_Error (R_PST, X_SYNERR, "Incorrect type for PST node pointer");
      break;
    }

  DEBUG (R_PST, DB_PSTBROTHERPUT, TL_PARAMS, (outbuf,
    "Leaving Pst_Brother_Put, status = void"));

  return;
}
/* End of Pst_Brother_Put */

/****************************************************************************
*
*  Function Name:                 Pst_Compound_Create
*
*    This routine creates a compound node and fills in the fields, but it
*    does not hook it into the PST.
*
*  Used to be:
*
*    init_cmp:
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
Compound_t *Pst_Compound_Create
  (
  void
  )
{
  Compound_t    *compound_p;                 /* Address of node in question */

  DEBUG (R_PST, DB_PSTCOMPCREATE, TL_PARAMS, (outbuf,
    "Entering Pst_Compound_Create"));

#ifdef _MIND_MEN_
  mind_malloc ("compound_p", "pst{1}", &compound_p, PSTCOMPOUNDSIZE);
#else
  Mem_Alloc (Compound_t *, compound_p, PSTCOMPOUNDSIZE, GLOBAL);
#endif

  DEBUG (R_PST, DB_PSTCOMPCREATE, TL_MEMORY, (outbuf,
    "Allocated memory for a PST Compound in Pst_Compound_Create at %p",
    compound_p));

  if (compound_p == NULL)
    IO_Exit_Error (R_PST, X_LIBCALL,
      "No memory for PST compound node in Pst_Compound_Create");

  (void) memset (compound_p, 0, PSTCOMPOUNDSIZE);

  PstComp_Index_Put (compound_p, PstCB_CompoundIndex_Get (&SPstcb));
  PstCB_CompoundIndex_Put (&SPstcb, PstCB_CompoundIndex_Get (&SPstcb) + 1);

  DEBUG (R_PST, DB_PSTCOMPCREATE, TL_PARAMS, (outbuf,
    "Leaving Pst_Compound_Create, compound addr = %p", compound_p));

  return compound_p;
}
/* End of Pst_Compound_Create */

/****************************************************************************
*
*  Function Name:                 Pst_Compound_Dump
*
*    This routine prints a formatted dump of a PST Compound node.
*
*  Used to be:
*
*    ???:
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
void Pst_Compound_Dump
  (
  Compound_t    *compound_p,                 /* Instance to dump */
  FileDsc_t     *filed_p                     /* File to dump to */
  )
{
  FILE          *f;                          /* File handle */

  f = IO_FileHandle_Get (filed_p);
  if (compound_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL PST Compound\n");
    return;
    }

  DEBUG_ADDR (R_PST, DB_PSTCOMPCREATE, compound_p);

  fprintf (f, "Comp index: %6lu,  prev: %8p,  next: %8p\n", 
    PstComp_Index_Get (compound_p), PstComp_Prev_Get (compound_p), 
    PstComp_Next_Get (compound_p));
  fprintf (f, "  father:  %8p,  son:  %8p,  bro:  %8p,  sym:  %8p\n",
    PstComp_Father_Get (compound_p), PstComp_Son_Get (compound_p),
    PstComp_Brother_Get (compound_p), PstComp_SymbolTable_Get (compound_p));

  return;
}
/* End of Pst_Compound_Dump */

/****************************************************************************
*
*  Function Name:                 Pst_Compound_Merit_Set
*
*    This function calculates a compound's merit. 
*
*  Used to be:
*
*    setcomp:
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
*    Compound's initial merit 
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U16_t Pst_Compound_Merit_Set
  (
  Xtr_t         *xtr_p                      /* Molecule to evaluate */
  )
{
  U16_t         merit;                     /* Result of calculation */
  U16_t         rings;
  U16_t         carbons;
  U16_t         fgroups;
  U16_t         num_atoms;
  U16_t         rings_penalty;
  U16_t         carbons_penalty;
  U16_t         fgroups_penalty;
  Array_t       sigma;
  Array_t       z;
  Array_t       pi;
  Array_t       hydrogen;
  Array_t       resonant;
  Array_t       aromatic;
  Array_t       preserve;
  

  DEBUG (R_PST, DB_PSTCOMPMERITSET, TL_PARAMS, (outbuf,
    "Entering Pst_Compound_Merit_Set, XTR addr %p", xtr_p));
  
  num_atoms = Xtr_NumAtoms_Get (xtr_p);

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&sigma", "pst{2}", &sigma, num_atoms, WORDSIZE);
  mind_Array_1d_Create ("&z", "pst{2}", &z, num_atoms, WORDSIZE);
  mind_Array_1d_Create ("&pi", "pst{2}", &pi, num_atoms, WORDSIZE);
  mind_Array_1d_Create ("&hydrogen", "pst{2}", &hydrogen, num_atoms, WORDSIZE);
  mind_Array_1d_Create ("&resonant", "pst{2}", &resonant, num_atoms, BITSIZE);
  mind_Array_1d_Create ("&aromatic", "pst{2}", &aromatic, num_atoms, BITSIZE);
  mind_Array_1d_Create ("&preserve", "pst{2}", &preserve, num_atoms, BITSIZE);
#else
  Array_1d_Create (&sigma, num_atoms, WORDSIZE);
  Array_1d_Create (&z, num_atoms, WORDSIZE);
  Array_1d_Create (&pi, num_atoms, WORDSIZE);
  Array_1d_Create (&hydrogen, num_atoms, WORDSIZE);
  Array_1d_Create (&resonant, num_atoms, BITSIZE);
  Array_1d_Create (&aromatic, num_atoms, BITSIZE);
  Array_1d_Create (&preserve, num_atoms, BITSIZE);
#endif
  
  Pst_Calculate (xtr_p, &sigma, &z, &pi, &hydrogen, &resonant, &aromatic, 
       &preserve);

  rings_penalty = Pst_RingPenalty_Compute (xtr_p, resonant, aromatic, preserve);
  carbons_penalty = Pst_CarbonPenalty_Compute (xtr_p, sigma, hydrogen, preserve,
       aromatic, resonant);
  fgroups_penalty = Pst_FuncGrpPenalty_Compute (xtr_p, sigma, z, pi, hydrogen,
       resonant, aromatic, preserve);

  if (rings_penalty > 10)
    {
    rings = 50;
    carbons = 30;
    fgroups = 20;
    }

  if (rings_penalty <= 10 && rings_penalty > 0)
    {
    rings = 20;
    carbons = 50;
    fgroups = 30;
    }

  if (rings_penalty == 0)
    {
    rings = 5;
    carbons = 60;
    fgroups = 35;
    }

  if (rings_penalty > 0)
    PstCB_MeritRings_Put (&SPstcb, (float) rings / (float) rings_penalty);
  else
    PstCB_MeritRings_Put (&SPstcb, 1.00);

  if (carbons_penalty > 0)
    PstCB_MeritCarbons_Put (&SPstcb, (float) carbons / (float) carbons_penalty);
  else
    PstCB_MeritCarbons_Put (&SPstcb, 1.00);

  PstCB_MeritFGroups_Put (&SPstcb, (float) fgroups / 100.0);

  merit = MAX (2, MIN (100, (100 - (PstCB_MeritRings_Get (&SPstcb) * 
	     rings_penalty + PstCB_MeritCarbons_Get (&SPstcb) * carbons_penalty
	    + PstCB_MeritFGroups_Get (&SPstcb) * fgroups_penalty)))) + 0.5;

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&sigma", "pst", &sigma);
  mind_Array_Destroy ("&z", "pst", &z);
  mind_Array_Destroy ("&pi", "pst", &pi);
  mind_Array_Destroy ("&hydrogen", "pst", &hydrogen);
  mind_Array_Destroy ("&resonant", "pst", &resonant);
  mind_Array_Destroy ("&aromatic", "pst", &aromatic);
  mind_Array_Destroy ("&preserve", "pst", &preserve);
#else
  Array_Destroy (&sigma);
  Array_Destroy (&z);
  Array_Destroy (&pi);
  Array_Destroy (&hydrogen);
  Array_Destroy (&resonant);
  Array_Destroy (&aromatic);
  Array_Destroy (&preserve);
#endif
    
  merit += Pst_CE_Adjust (xtr_p, merit);

  DEBUG (R_PST, DB_PSTCOMPMERITSET, TL_PARAMS, (outbuf,
    "Leaving Pst_Compound_Merit_Set, merit = %u", merit));

  return merit;
}
/* End of Pst_Compound_Merit_Set */

/****************************************************************************
*
*  Function Name:                 Pst_ControlHandle_Get
*
*    This function returns a handle to the PST Control Block.
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
*    N/A
*
******************************************************************************/
PstCB_t *Pst_ControlHandle_Get
  (
  void
  )
{
  return (&SPstcb);
}
/* End of Pst_ControlHandle_Get */

/****************************************************************************
*
*  Function Name:                 Pst_Destroy
*
*    This routine destroys the pst and returns all the memory allocated for the
*    subgoal nodes and compound nodes in the tree.
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
*    Deallocate memory
*
******************************************************************************/
void Pst_Destroy
  (
  PstNode_t     node
  )
{
  PstNode_t     nextnode;
  
/*  When the list of memory blocks for each expansion is used.
  if (PstCB_Expansions_Get (&SPstcb) != NULL 
      && List_Size_Get (PstCB_Expansions_Get (&SPstcb)) > 0)
    {
    List_Destroy (PstCB_Expansions_Get (&SPstcb));
    PstCB_Expansions_Put (&SPstcb, NULL);
    }
*/

  while (PstNode_Compound_Get (&node) != NULL)
    {
    nextnode = Pst_Son_Get (&node);
    Pst_Destroy (nextnode);
    nextnode = Pst_Brother_Get (&node);
#ifdef _MIND_MEM_
    if (PstNode_Type_Get (&node) == PST_SUBGOAL)
      {
      mind_free ("PstNode_Subgoal_Get(&node)", "pst", PstNode_Subgoal_Get (&node));
      }
    else
      {
      mind_free ("PstNode_Compound_Get(&node)", "pst", PstNode_Compound_Get (&node));
      }
#else
    if (PstNode_Type_Get (&node) == PST_SUBGOAL)
      {
      Mem_Dealloc (PstNode_Subgoal_Get (&node), PSTSUBGOALSIZE, GLOBAL);
      }
    else
      {
      Mem_Dealloc (PstNode_Compound_Get (&node), PSTCOMPOUNDSIZE, GLOBAL);
      }
#endif
 
    node = nextnode;
    }

  return;
}
/* End of Pst_Destroy */

/****************************************************************************
*
*  Function Name:                 Pst_Dump
*
*    This routine prints a formatted dump of the Problem Solving Tree.
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
*    N/A
*
******************************************************************************/
void Pst_Dump
  (
  Subgoal_t     *sg_p,
  FileDsc_t     *filed_p                     /* File to dump to */
  )
{
  Compound_t    *cmp_p;

  while (sg_p != NULL)
    {
    Pst_SubGoal_Dump (sg_p, filed_p);
    cmp_p = PstSubg_Son_Get (sg_p);
    while (cmp_p != NULL)
      {
      Pst_Compound_Dump (cmp_p, filed_p);
      Pst_SymTab_Dump (PstComp_SymbolTable_Get (cmp_p), filed_p);
      Pst_Dump (PstComp_Son_Get (cmp_p), filed_p);
      cmp_p = PstComp_Brother_Get (cmp_p);
      }
    sg_p = PstSubg_Brother_Get (sg_p);
    }

  return;
}
/* End of Pst_Dump */

/****************************************************************************
*
*  Function Name:                 Pst_Father_Get
*
*    This routine gets the father PST node of a given node.  This is 
*    dependent on the node type, ie subgoals are fathers of compound nodes
*    and vice-versa.
*
*  Used to be:
*
*    father:
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
*    PstNode_t describing the father node
*
*  Side Effects:
*
*    May call IO_Exit_Error
*
******************************************************************************/
PstNode_t Pst_Father_Get
  (
  PstNode_t     *node_p                     /* Address of node in question */
  )
{
  PstNode_t      tempnode;                  /* Node pointer to construct */

  DEBUG (R_PST, DB_PSTFATHERGET, TL_PARAMS, (outbuf,
    "Entering Pst_Father_Get, node.type = %hu, node.address = %p",
    PstNode_Type_Get (node_p), PstNode_Subgoal_Get (node_p)));

  switch (PstNode_Type_Get (node_p))
    {
    case PST_COMPOUND :

      PstNode_Subgoal_Put (&tempnode,
	PstComp_Father_Get (PstNode_Compound_Get (node_p)));
      break;

    case PST_SUBGOAL :

      PstNode_Compound_Put (&tempnode,
	PstSubg_Father_Get (PstNode_Subgoal_Get (node_p)));
      break;

    default :

      IO_Exit_Error (R_PST, X_SYNERR,
	"Incorrect type for PST node pointer in Pst_Father_Get");
      break;
    }

  DEBUG (R_PST, DB_PSTFATHERGET, TL_PARAMS, (outbuf,
    "Leaving Pst_Father_Get, node.type = %hu, node.address = %p",
    tempnode.type, tempnode.p.subgoal));

  return tempnode;
}
/* End of Pst_Father_Get */

/****************************************************************************
*
*  Function Name:                 Pst_Father_Put
*
*    This routine stores a new father pointer into a PST node.  The types 
*    must not be the same, subgoal nodes are fathers to compound nodes and
*    vice-versa.
*
*  Used to be:
*
*    $father:
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
*    May call IO_Exit_Error
*
******************************************************************************/
void Pst_Father_Put
  (
  PstNode_t     *node_p,                    /* Address of node in question */
  PstNode_t     *father_p                   /* Node to become the father */
  )
{
  DEBUG (R_PST, DB_PSTFATHERPUT, TL_PARAMS, (outbuf, 
    "Entering Pst_Father_Put, node.type = %hu, node.address = %p,"
    " father.type = %hu, father.address = %p",
    PstNode_Type_Get (node_p), PstNode_Subgoal_Get (node_p),
    PstNode_Type_Get (father_p), PstNode_Subgoal_Get (father_p)));

  if (PstNode_Type_Get (node_p) == PstNode_Type_Get (father_p))
    IO_Exit_Error (R_PST, X_SYNERR, "Incompatible types in Pst_Father_Put");

  switch (PstNode_Type_Get (node_p))
    {
    case PST_COMPOUND :

      PstComp_Father_Put (PstNode_Compound_Get (node_p),
	PstNode_Subgoal_Get (father_p));
      break;

    case PST_SUBGOAL :

      PstSubg_Father_Put (PstNode_Subgoal_Get (node_p),
	PstNode_Compound_Get (father_p));
      break;

    default :

      IO_Exit_Error (R_PST, X_SYNERR, "Incorrect type for PST node pointer");
      break;
    }

  DEBUG (R_PST, DB_PSTFATHERPUT, TL_PARAMS, (outbuf,
    "Leaving Pst_Father_Get, status = void"));

  return;
}
/* End of Pst_Father_Put */

/****************************************************************************
*
*  Function Name:                 Pst_Hash
*
*    This function hashes the input Sling in a (supposedly) good way.
*
*    Use a hash function similar to the one in the original PLI code -- DK.
*
*  Used to be:
*
*    hash:
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
*    Hash value of Sling modulo # buckets
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U32_t Pst_Hash
  (
  Sling_t        sling,                     /* Sling to hash */
  U32_t          max_buckets                /* # buckets in caller */
  )
{
  U8_t          *str_p;
  U8_t          *n_p;
  U32_t          hash;                      /* Accumulated hash value */
  U16_t          n_i;                       /* Counter */
  U16_t          s_j;                       /* Counter */

  /*  Ignore the seb's and ceb's, since too many slings end in ``|no ceb's''
      and this can skew the hash function.  Otherwise, do repeated bytewise
      Xor of the hash value and sling character.
  */

  hash = 0;
  n_p = (U8_t *) &hash;
  n_i = 0;

  str_p = Sling_Name_Get (sling);

  for (s_j = 0; s_j < Sling_Length_Get (sling); s_j++)
    {
    if (*str_p == '|') break;

    *n_p = *n_p ^ *str_p;
     n_i++;
     if (n_i % sizeof (U32_t) == 0)
       n_p = (U8_t *) &hash;
     else
       n_p++;

     str_p++;
    }

  return (hash % max_buckets);
}
/* End of Pst_Hash */

/****************************************************************************
*
*  Function Name:              Pst_Merit_Compute
*
*     To estimate the synthetic complexity of a particular
*     molecule developed during a synthesis search relative to the
*     complexity of the goal molecule.  the synthetic complexity or
*     compound merit is defined as a value on a scale between zero
*     and one hundred where very simple molecules are given very
*     high values (near one hundred) and molecules of greater complexity
*     very low values.  the goal molecule is initially given a merit value
*     based on its apparent absolute complexity, and all complexity estimates
*     are scaled to coincide with this value.
*
*  used to be:
*   
*   compmer
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
*    merit
*
*  Side Effects:
*
*    N/A
*
****************************************************************************/
U16_t       Pst_Merit_Compute
 (
 Xtr_t       *xtr_p
 )
{
  U16_t         compound_merit;
  U16_t         num_atoms;
  U16_t         rings_penalty;
  U16_t         carbons_penalty;
  U16_t         fgroups_penalty;
  Array_t       sigma;
  Array_t       z;
  Array_t       pi;
  Array_t       hydrogen;
  Array_t       resonant;
  Array_t       aromatic;
  Array_t       preserve;

  num_atoms = Xtr_NumAtoms_Get (xtr_p);
 
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&sigma", "pst{3}", &sigma, num_atoms, WORDSIZE);
  mind_Array_1d_Create ("&z", "pst{3}", &z, num_atoms, WORDSIZE);
  mind_Array_1d_Create ("&pi", "pst{3}", &pi, num_atoms, WORDSIZE);
  mind_Array_1d_Create ("&hydrogen", "pst{3}", &hydrogen, num_atoms, WORDSIZE);
  mind_Array_1d_Create ("&resonant", "pst{3}", &resonant, num_atoms, BITSIZE);
  mind_Array_1d_Create ("&aromatic", "pst{3}", &aromatic, num_atoms, BITSIZE);
  mind_Array_1d_Create ("&preserve", "pst{3}", &preserve, num_atoms, BITSIZE);
#else
  Array_1d_Create (&sigma, num_atoms, WORDSIZE);
  Array_1d_Create (&z, num_atoms, WORDSIZE);
  Array_1d_Create (&pi, num_atoms, WORDSIZE);
  Array_1d_Create (&hydrogen, num_atoms, WORDSIZE);
  Array_1d_Create (&resonant, num_atoms, BITSIZE);
  Array_1d_Create (&aromatic, num_atoms, BITSIZE);
  Array_1d_Create (&preserve, num_atoms, BITSIZE);
#endif

  Pst_Calculate (xtr_p, &sigma, &z, &pi, &hydrogen, &resonant, &aromatic,
	&preserve);
 
  rings_penalty = Pst_RingPenalty_Compute (xtr_p, resonant, aromatic, preserve);
  carbons_penalty = Pst_CarbonPenalty_Compute (xtr_p, sigma, hydrogen, preserve,
       aromatic, resonant);
  fgroups_penalty = Pst_FuncGrpPenalty_Compute (xtr_p, sigma, z, pi, hydrogen,
       resonant, aromatic, preserve);
 
  compound_merit = MAX (2, MIN (100, (100 - (PstCB_MeritRings_Get (&SPstcb) *
	     rings_penalty + PstCB_MeritCarbons_Get (&SPstcb) * carbons_penalty
	     + PstCB_MeritFGroups_Get (&SPstcb) * fgroups_penalty)))) + 0.5;
 
  compound_merit += Pst_CE_Adjust (xtr_p, compound_merit);

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&sigma", "pst", &sigma);
  mind_Array_Destroy ("&z", "pst", &z);
  mind_Array_Destroy ("&pi", "pst", &pi);
  mind_Array_Destroy ("&hydrogen", "pst", &hydrogen);
  mind_Array_Destroy ("&resonant", "pst", &resonant);
  mind_Array_Destroy ("&aromatic", "pst", &aromatic);
  mind_Array_Destroy ("&preserve", "pst", &preserve);
#else
  Array_Destroy (&sigma);
  Array_Destroy (&z);
  Array_Destroy (&pi);
  Array_Destroy (&hydrogen);
  Array_Destroy (&resonant);
  Array_Destroy (&aromatic);
  Array_Destroy (&preserve);
#endif

  return compound_merit;
}
/* End of Pst_Merit_Compute */

/****************************************************************************
*
*  Function Name:                 Pst_NumSons_Get
*
*    This function returns the number of sons of a node in the PST.
*
*  Used to be:
*
*     nsons:    
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
*    Value of number of sons of a given node
*
*  Side Effects:
*
*    May call IO_Exit_Error
*
******************************************************************************/
U16_t Pst_NumSons_Get
  (
  PstNode_t    *node_p                     /* Address of node in question */
  )
{
  U16_t         count;                     /* Number of sons found */
  Compound_t   *compound_p;                /* List traversal */
  Subgoal_t    *subgoal_p;                 /* List traversal */

  DEBUG (R_PST, DB_PSTNUMSONS, TL_PARAMS, (outbuf,
    "Entering Pst_NumSons_Get, node.type = %hu, node.address = %p",
    PstNode_Type_Get (node_p), PstNode_Subgoal_Get (node_p)));

  count = 0;

  switch (PstNode_Type_Get (node_p))
    {
    case PST_COMPOUND :

      for (subgoal_p = PstComp_Son_Get (PstNode_Compound_Get (node_p));
	   subgoal_p != NULL;
	   subgoal_p = PstSubg_Brother_Get (subgoal_p), count++)
	/* Empty loop body */ ;
      break;

    case PST_SUBGOAL :

      for (compound_p = PstSubg_Son_Get (PstNode_Subgoal_Get (node_p));
	   compound_p != NULL;
	   compound_p = PstComp_Brother_Get (compound_p), count++)
	/* Empty loop body */ ;
      break;

    default :

      IO_Exit_Error (R_PST, X_SYNERR,
	"Incorrect type for PST node pointer in PsNumSons_Get");
      break;
    }

  DEBUG (R_PST, DB_PSTNUMSONS, TL_PARAMS, (outbuf,
    "Leaving Pst_NumSons_Get, count = %u", count));

  return count;
}
/* End of Pst_NumSons_Get */

/****************************************************************************
*
*  Function Name:              Pst_ReactionMerit_Get
*    
*    This routine calculates the reaction merit for a particular subgoal based
*    on its stored values for ease yield and confidence.  This provides a 
*    global definition for reaction merit - the function cofactors will only
*    need to be changed in this one place to provide a redefinition.
*
*  Used to be:
*
*    rxmerit
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
*    reaction merit for the subgoal
*
*  Side Effects:
*
*    N/A
*
****************************************************************************/
U16_t  Pst_ReactionMerit_Get 
  (
  Subgoal_t   *subgoal_p
  )
{
   U16_t       reaction_merit;
   U16_t       A;       /* cofactors */
   U16_t       B;
   U16_t       C;
   U16_t       D;       /* penalty for not mapping any strategic bond to an 
			active bond in this application of the reaction
			transform
		     */

   A = 2;
   B = 2;
   C = 1;
   D = 7;

   reaction_merit = (A * PstSubg_Reaction_Ease_Get (subgoal_p) + 
		   B * PstSubg_Reaction_Yield_Get (subgoal_p) +
		   C * PstSubg_Reaction_Confidence_Get (subgoal_p)) /
		     (A + B + C);
   if (PstSubg_Flags_Active_Get (subgoal_p) == FALSE)
      reaction_merit = MAX (0, reaction_merit - D);
   return reaction_merit;
}
/* End of Pst_ReactionMerit_Get */

/****************************************************************************
*
*  Function Name:                 Pst_Root_Set
*
*    This function takes the goal compound (of the whole run!) and enters
*    it into the PST.  Before doing so, it creates and initializes all the
*    data-structures used by the PST.
*
*  Used to be:
*
*    setroot:
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
Compound_t *Pst_Root_Set
  (
  Xtr_t         *xtr_p,                     /* Target compound XTR format */
  Sling_t       sling                          /* sling of the compound */              
  )
{
  Subgoal_t     *subg_p;                    /* Primary goal of this run */
  Compound_t    *compound_p;                /* Goal compound */
  SymTab_t      *symtab_p;                  /* Symbol table entry */
  U32_t          bucket;                    /* Symbol table index */

  DEBUG (R_PST, DB_PSTROOTSET, TL_PARAMS, (outbuf,
    "Entering Pst_Root_Set, XTR addr %p", xtr_p));

  /* Initialize the PST global data */

  PstCB_CompoundIndex_Put (&SPstcb, 1);
  PstCB_TotalExpandedCompounds_Put (&SPstcb, 0);
  PstCB_SubgoalIndex_Put (&SPstcb, 1);
  PstCB_SymtabIndex_Put (&SPstcb, 1);
  PstCB_Expansions_Put (&SPstcb, List_Create (LIST_NORMAL));

  subg_p = Pst_SubGoal_Create ();
  compound_p = Pst_Compound_Create ();
  symtab_p = Pst_SymTab_Create ();
  PstCB_Root_Put (&SPstcb, subg_p);
  PstCB_CurrentComp_Put (&SPstcb, compound_p);
  bucket = Pst_Hash (sling, MX_SYMTAB_BUCKETS);

  /* All but father, symtab of initial compound node is NULL or zero */

  PstComp_Father_Put (compound_p, subg_p);
  PstComp_SymbolTable_Put (compound_p, symtab_p);
  PstComp_Brother_Put (compound_p, NULL);
  PstComp_Son_Put (compound_p, NULL);
  PstComp_Prev_Put (compound_p, NULL);

  /* All but son of initial subgoal is NULL or zero */

  PstSubg_Son_Put (subg_p, compound_p);
  PstSubg_Brother_Put (subg_p, NULL);
  PstSubg_Father_Put (subg_p, NULL);

  /* Initial symbol table entry is mostly normal, except we know that
     the current hashtable is empty.
  */

  SymbolTable_Init ();
  SymTab_FirstComp_Put (symtab_p, compound_p);
  SymTab_Sling_Put (symtab_p, sling);
  SymTab_Merit_Main_Put (symtab_p, Pst_Compound_Merit_Set (xtr_p));
  SymTab_HashBucketHead_Put (bucket, symtab_p);

  PstCB_MainTarget_Put (&SPstcb, sling);

  DEBUG (R_PST, DB_PSTROOTSET, TL_PARAMS, (outbuf,
    "Leaving Pst_Root_Set, compound addr = %p", compound_p));

  return compound_p;
}
/* End of Pst_Root_Set */

/****************************************************************************
*
*  Function Name:                 Pst_Son_Get
*
*    This function retrieves a descriptor for the son of a given node.
*    This must contain at least the address and the type of the node since
*    the function may be applied to either a compound or a subgoal node and
*    they use each other (not themselves!) for sons.
*
*  Used to be:
*
*    son:
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
*    May call IO_Exit_Error
*
******************************************************************************/
PstNode_t Pst_Son_Get
  (
  PstNode_t     *node_p                     /* Address of node in question */
  )
{
  PstNode_t      tempnode;                  /* Node pointer to construct */

  DEBUG (R_PST, DB_PSTSONGET, TL_PARAMS, (outbuf,
    "Entering Pst_Son_Get, node.type = %hu, node.address = %p",
    PstNode_Type_Get (node_p), PstNode_Subgoal_Get (node_p)));

  switch (PstNode_Type_Get (node_p))
    {
    case PST_COMPOUND :

      PstNode_Subgoal_Put (&tempnode,
	PstComp_Son_Get (PstNode_Compound_Get (node_p)));
      break;

    case PST_SUBGOAL :

      PstNode_Compound_Put (&tempnode,
	PstSubg_Son_Get (PstNode_Subgoal_Get (node_p)));
      break;

    default :

      IO_Exit_Error (R_PST, X_SYNERR, "Incorrect type for PST node pointer");
      break;
    }

  DEBUG (R_PST, DB_PSTSONGET, TL_PARAMS, (outbuf,
    "Leaving Pst_Son_Get, node.type = %hu, node.address = %p", tempnode.type,
    tempnode.p.subgoal));

  return tempnode;
}
/* End of Pst_Son_Get */

/****************************************************************************
*
*  Function Name:                 Pst_Son_Put
*
*    This function stores a descriptor for the son of a given node.
*    This must contain at least the address and the type of the node since
*    the function may be applied to either a compound or a subgoal node and
*    they use each other (not themselves!) for sons.
*
*  Used to be:
*
*    $son:
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
*    May call IO_Exit_Error
*
******************************************************************************/
void Pst_Son_Put
  (
  PstNode_t     *node_p,                    /* Address of node in question */
  PstNode_t     *son_p                      /* Node to become the father */
  )
{
  DEBUG (R_PST, DB_PSTSONPUT, TL_PARAMS, (outbuf, 
    "Entering Pst_Son_Put, node.type = %hu, node.address = %p,"
    " son.type = %hu, son.address = %p",
    PstNode_Type_Get (node_p), PstNode_Subgoal_Get (node_p),
    PstNode_Type_Get (son_p), PstNode_Subgoal_Get (son_p)));

  if (PstNode_Type_Get (node_p) == PstNode_Type_Get (son_p))
    IO_Exit_Error (R_PST, X_SYNERR, "Incompatible types in Pst_Son_Put");

  switch (PstNode_Type_Get (node_p))
    {
    case PST_COMPOUND :

      PstComp_Son_Put (PstNode_Compound_Get (node_p),
	PstNode_Subgoal_Get (son_p));
      break;

    case PST_SUBGOAL :

      PstSubg_Son_Put (PstNode_Subgoal_Get (node_p),
	PstNode_Compound_Get (son_p));
      break;

    default :

      IO_Exit_Error (R_PST, X_SYNERR, "Incorrect type for PST node pointer");
      break;
    }

  DEBUG (R_PST, DB_PSTSONPUT, TL_PARAMS, (outbuf,
    "Leaving Pst_Son_Put, status = void"));

  return;
}
/* End of Pst_Son_Get */

/****************************************************************************
*
*  Function Name:                 Pst_SubGoal_Create
*
*    This routine creates a subgoal node, initializes it.  But it does not
*    connect it into the PST.
*
*  Used to be:
*
*    init_subg:
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
Subgoal_t *Pst_SubGoal_Create
  (
  void
  )
{
  Subgoal_t    *subgoal_p;                 /* Address of node in question */

  DEBUG (R_PST, DB_PSTSUBGCREATE, TL_PARAMS, (outbuf,
    "Entering Pst_SubGoal_Create"));

#ifdef _MIND_MEM_
  mind_malloc ("subgoal_p", "pst{4}", &subgoal_p, PSTSUBGOALSIZE);
#else
  Mem_Alloc (Subgoal_t *, subgoal_p, PSTSUBGOALSIZE, GLOBAL);
#endif

  DEBUG (R_PST, DB_PSTSUBGCREATE, TL_MEMORY, (outbuf,
    "Allocated memory for a PST Sub-Goal in Pst_SubGoal_Create at %p",
    subgoal_p));

  if (subgoal_p == NULL)
    IO_Exit_Error (R_PST, X_LIBCALL,
      "No memory for PST Sub-Goal in Pst_SubGoal_Create");

  (void) memset (subgoal_p, 0, PSTSUBGOALSIZE);

  PstSubg_Merit_Main_Put (subgoal_p, SUBG_MERIT_INIT);
  PstSubg_Merit_Solved_Put (subgoal_p, SUBG_MERIT_INIT);
  PstSubg_Merit_Initial_Put (subgoal_p, SUBG_MERIT_INIT);
  PstSubg_Index_Put (subgoal_p, PstCB_SubgoalIndex_Get (&SPstcb));
  PstCB_SubgoalIndex_Put (&SPstcb, PstCB_SubgoalIndex_Get (&SPstcb) + 1);

  DEBUG (R_PST, DB_PSTSUBGCREATE, TL_PARAMS, (outbuf,
    "Leaving Pst_SubGoal_Create, subgoal addr = %p", subgoal_p));

  return subgoal_p;
}
/* End of Pst_SubGoal_Create */

/****************************************************************************
*
*  Function Name:                 Pst_SubGoal_Dump
*
*    This routine prints a formatted dump of a PST Subgoal.
*
*  Used to be:
*
*    ???:
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
void Pst_SubGoal_Dump
  (
  Subgoal_t   *subgoal_p,                  /* Node to dump */
  FileDsc_t   *filed_p                     /* File to dump to */
  )
{
  FILE        *f;                          /* Temporary */
  Time_t       sgtime;                     /* system dependent variable */

  f = IO_FileHandle_Get (filed_p);
  if (subgoal_p == NULL)
    {
    fprintf (f, "In Pst_SubGoal_Dump sub-goal address is NULL\n");
    return;
    }

  DEBUG_ADDR (R_PST, DB_PSTSUBGCREATE, subgoal_p);

  fprintf (f,"Subgoal index:  %5lu,  level:  %2hu\n", 
    PstSubg_Index_Get (subgoal_p), 
    PstSubg_Level_Get (subgoal_p));

  fprintf (f, "   rxn:  %4lu,  ease:  %3u,  yield:  %3u,  conf:  %3u\n",
    PstSubg_Reaction_Schema_Get (subgoal_p),    
    PstSubg_Reaction_Ease_Get (subgoal_p),
    PstSubg_Reaction_Yield_Get (subgoal_p),
    PstSubg_Reaction_Confidence_Get (subgoal_p));

  fprintf (f, "  merit main:  %3d,  sol:  %3d,  init:  %3d\n",
    PstSubg_Merit_Main_Get (subgoal_p), 
    PstSubg_Merit_Solved_Get (subgoal_p),
    PstSubg_Merit_Initial_Get (subgoal_p));

  sgtime = Strategy_Subg_TimeSpent_Get (PstSubg_Strategy_Get (subgoal_p));
  fprintf (f, "  strategy time:  %8.4f secs,  closed:  %3u,  visits:  %3u\n",
    Time_Format (sgtime), 
    Strategy_Subg_ClosedCnt_Get (PstSubg_Strategy_Get (subgoal_p)), 
    PstSubg_Visits_Get (subgoal_p));

  fprintf (f, "  links father:  %8p,  son:  %8p,  bro:  %8p\n",
    PstSubg_Father_Get (subgoal_p), PstSubg_Son_Get (subgoal_p),
    PstSubg_Brother_Get (subgoal_p));

  return;
}
/* End of Pst_SubGoal_Dump */

/****************************************************************************
*
*  Function Name:                 Pst_SubGoal_Insert
*
*    This routine inserts a newly generated subgoal into the PST.
*
*  Used to be:
*
*    addsub, add_new_subgoal_node, add_new_compound_node,
*    store_initial_subgoal_merit:
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
void Pst_SubGoal_Insert
  (
  Array_t      *sortxtr_p,                /* 1d-addr, sorted XTRs */
  Array_t      *sortcanxtr_p,             /* 1d-addr, sorted canonical XTRs */
  Sling_t      *slings_p,                 /* Array of slings */
  SubGenr_Compound_t *compound_p,          /* SubGenr compound format */
  U16_t         num_compounds,             /* # compounds in subgoal */
  U32_t         schema,                    /* Reaction schema used */
  Boolean_t    *duplicate_p,               /* Pointer to duplicate flag */
  Boolean_t     strategic                  /* Flag - strategic bond matched */
  )
{
  char         *sling_p;
  char         *temp_p;
  Compound_t   *newcomp_p;
  Compound_t   *comp_p;
  Subgoal_t    *subgoal_p;
  Subgoal_t    *son_p;
  SymTab_t     *symtab_p;
  U16_t         compound_merit;
  U16_t         length;
  U16_t         i;
  U16_t         reaction_merit;
  S16_t         adjusted_merit;
  U32_t         index;
  S16_t         min_merit;
  S16_t         total_merit;
  Boolean_t     all_solved;
  Boolean_t     found;


  DEBUG (R_PST, DB_PSTSUBGINSERT, TL_PARAMS, (outbuf,
    "Entering Pst_SubGoal_Insert, sorted XTR array %p, sling vector %p,"
    " subgenr compound %p, # compounds %u, schema %lu, duplicate flag %p,"
    " strategic flag %hu", sortxtr_p, slings_p, compound_p, num_compounds, 
    schema, duplicate_p, strategic));

#ifdef _MIND_MEM_
in_subgenr(-1000);
#endif
  *duplicate_p = Pst_Subgoal_IsDuplicate (slings_p, num_compounds, schema);
  if (*duplicate_p == FALSE 
      && Pst_Circular (slings_p, num_compounds) == FALSE)
    {
    /* add the new node to Pst */

#ifdef _MIND_MEM_
in_subgenr(-1001);
if (num_compounds==0) in_subgenr(schema);
#endif
    subgoal_p = Pst_SubGoal_Create ();
    son_p = PstComp_Son_Get (PstCB_CurrentComp_Get (&SPstcb));
    if (son_p == NULL)
       PstComp_Son_Put (PstCB_CurrentComp_Get (&SPstcb), subgoal_p);
    else 
      {
      while (PstSubg_Brother_Get (son_p) != NULL)
	son_p = PstSubg_Brother_Get (son_p);
      PstSubg_Brother_Put(son_p, subgoal_p);
      }

#ifdef _MIND_MEM_
in_subgenr(-1002);
#endif
    PstSubg_Father_Put (subgoal_p, PstCB_CurrentComp_Get (&SPstcb));
    PstSubg_Brother_Put (subgoal_p, NULL);
    PstSubg_Son_Put (subgoal_p, NULL);
    PstSubg_Level_Put (subgoal_p, PstSubg_Level_Get (PstComp_Father_Get (
      PstCB_CurrentComp_Get (&SPstcb))) + 1);
    PstSubg_Reaction_Ease_Put (subgoal_p,
      SubGenr_Compound_Ease_Get (compound_p));
    PstSubg_Reaction_Yield_Put (subgoal_p,
      SubGenr_Compound_Yield_Get (compound_p));
    PstSubg_Reaction_Confidence_Put (subgoal_p,
      SubGenr_Compound_Confidence_Get (compound_p));
    PstSubg_Reaction_Schema_Put (subgoal_p, schema);
    PstSubg_Flags_Active_Put (subgoal_p, strategic);

#ifdef _MIND_MEM_
in_subgenr(-1003);
#endif
    /* do for each compound of the subgoal  */

    for (i = 0; i < num_compounds; ++i) 
      {
      Pst_SymTab_Index_Find (slings_p[i], &symtab_p, &found);
      if (found == FALSE) 
	{
#ifdef _MIND_MEM_
in_subgenr(-1004);
#endif
	temp_p = (char *)Sling_Name_Get (slings_p[i]);
	length = 0;
	while (*temp_p != '|' && length < Sling_Length_Get (slings_p[i]))
	  {
	  ++length;
	  ++temp_p;
	  }

#ifdef _MIND_MEM_
	mind_malloc("sling_p", "pst{5}", &sling_p, length + 1);
in_subgenr(-1005);
#else
	Mem_Alloc(char *, sling_p, length + 1, GLOBAL);
#endif
	memcpy (sling_p, Sling_Name_Get (slings_p[i]), length);
#ifdef _MIND_MEM_
in_subgenr(-1006);
#endif
	sling_p[length] = '\0';

	index = AvcLib_Exists (sling_p);
	if (index != AVC_INDEX_NOT_FOUND) 
	  {
	  SymTab_AvailCompKey_Put (symtab_p, index);
	  SymTab_Merit_Main_Put (symtab_p, 100);
	  SymTab_Merit_Solved_Put (symtab_p, 100);
	  SymTab_Flags_Available_Put (symtab_p, TRUE);
	  SymTab_Flags_Solved_Put (symtab_p, TRUE);
	  SymTab_Flags_Open_Put (symtab_p, FALSE);                   
	  }
	else 
	  {
	  /* Synthesis search :  compute merit based on canonical xtr.  */
	  Xtr_t       *canonxtr_p;

	  canonxtr_p = (Xtr_t *) Array_1d32_Get (sortcanxtr_p, i);
	  compound_merit = Pst_Merit_Compute (canonxtr_p);
	  SymTab_Merit_Main_Put (symtab_p, compound_merit); 
	  SymTab_AvailCompKey_Put (symtab_p, AVC_KEY_NOT_FOUND);
	  }

#ifdef _MIND_MEM_
in_subgenr(-1007);
#endif
	SymTab_Merit_Initial_Put (symtab_p, 
	  SymTab_Merit_Main_Get (symtab_p));

#ifdef _MIND_MEM_
in_subgenr(-1008);
	mind_free ("sling_p", "pst", sling_p);
#else
	Mem_Dealloc (sling_p, length + 1, GLOBAL);
#endif
	}

      /* add_new_compound_node */

      newcomp_p = Pst_Compound_Create ();
      if (PstSubg_Son_Get (subgoal_p) == NULL)
	PstComp_Brother_Put (newcomp_p, NULL);
      else
	PstComp_Brother_Put (newcomp_p, PstSubg_Son_Get (subgoal_p)); 

#ifdef _MIND_MEM_
in_subgenr(-1009);
#endif
      PstSubg_Son_Put (subgoal_p, newcomp_p);
      PstComp_Father_Put (newcomp_p, subgoal_p);
      PstComp_Son_Put (newcomp_p, NULL);
      PstComp_SymbolTable_Put (newcomp_p, symtab_p);
      if (found == TRUE)
	PstComp_Prev_Put (newcomp_p, SymTab_FirstComp_Get (symtab_p));

      SymTab_FirstComp_Put (symtab_p, newcomp_p);
      }
  
#ifdef _MIND_MEM_
in_subgenr(-1010);
#endif
    /* store_initial_subgoal_merit */

    reaction_merit = Pst_ReactionMerit_Get (subgoal_p);
    all_solved = Pst_All_Solved (subgoal_p);
    comp_p = PstSubg_Son_Get (subgoal_p);
    i = 0;
    min_merit = MX_COMP_MERIT;
    total_merit = 0;
    while (comp_p != NULL) 
      {
      symtab_p = PstComp_SymbolTable_Get (comp_p);
      if (all_solved || SymTab_Flags_Solved_Get (symtab_p) == FALSE) 
	{
	++i;
	min_merit = MIN (min_merit, 
	  (S16_t) SymTab_Merit_Main_Get (symtab_p)); 
	total_merit +=  SymTab_Merit_Main_Get (symtab_p); 
	}

      comp_p = PstComp_Brother_Get (comp_p);
      } 
#ifdef _MIND_MEM_
in_subgenr(-1011);
#endif

    adjusted_merit = (min_merit
      - (100 * (i - 1) + min_merit - total_merit) / 20);
    PstSubg_Merit_Initial_Put (subgoal_p, 
      (MAX (adjusted_merit, 0)) * reaction_merit / 100);

    if (PstSubg_Reaction_Confidence_Get(subgoal_p) < 75)
      PstSubg_Merit_Initial_Put (subgoal_p, 
	PstSubg_Merit_Initial_Get (subgoal_p) - 2);
    }

  DEBUG (R_PST, DB_PSTSUBGINSERT, TL_PARAMS, (outbuf,
    "Leaving Pst_SubGoal_Insert, status = <void>"));

#ifdef _MIND_MEM_
in_subgenr(-1012);
#endif
  return;
}
/* End of Pst_SubGoal_Insert */

/****************************************************************************
*
*  Function Name:                 Pst_Subgoal_IsDuplicate
*
*    This routine checks to see if the candidate subgoal presented by an
*    array of names is a duplicate of some previous subgoal at this level
*    from the same syntheme and schema.
*
*  Used to be:
*
*    duplicate_subgoal 
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
*    TRUE       if duplicate
*    FALSE       otherwise
*
*  Side Effects:
*
*    N/A
*
****************************************************************************/
Boolean_t  Pst_Subgoal_IsDuplicate
 (
 Sling_t       *slings_p,
 U16_t          num_compound ,
 U32_t          schema
 )
{
   Subgoal_t       *subgoal_p;
   Sling_t       *array;                     
   U16_t       size;
   Compound_t       *compound_p;
   U16_t       maxconjuncts = 4; /* assume max. of 4 conjuncts */  

#ifdef _MIND_MEM_
in_subgenr(-10001);
   mind_malloc ("array", "pst{6}", &array, maxconjuncts * SLINGSIZE);
#else
   Mem_Alloc (Sling_t *, array, maxconjuncts * SLINGSIZE, GLOBAL);
#endif

   subgoal_p = PstComp_Son_Get (PstCB_CurrentComp_Get (&SPstcb)); 
   while (subgoal_p != NULL) 
     {
#ifdef _MIND_MEM_
in_subgenr(-100011);
#endif
      if (PstSubg_Reaction_Schema_Get (subgoal_p) == schema) 
	{
	 /* consider only those subgoals resulting from the same reaction
	    that produced subgoal_p */
#ifdef _MIND_MEM_
in_subgenr(-100012);
#endif

	 /* build_subgoal_array */
	 compound_p = PstSubg_Son_Get (subgoal_p);
	 size = 0;
	 while (compound_p != NULL) 
	   {
#ifdef _MIND_MEM_
in_subgenr(-100013);
#endif
	    if (size >= maxconjuncts)
	      {
#ifdef _MIND_MEM_
in_subgenr(-100014);
#endif
	      printf ("Error: more than 4 conjuncts in a subgaol\n");
	      exit (-1);
	      }

	    Sling_Name_Put (array[size], Sling_Name_Get (SymTab_Sling_Get (   
	      PstComp_SymbolTable_Get (compound_p))));
	    Sling_Length_Put (array[size], Sling_Length_Get (SymTab_Sling_Get (
	       PstComp_SymbolTable_Get (compound_p))));
	    compound_p = PstComp_Brother_Get (compound_p);     
	    ++size; 
#ifdef _MIND_MEM_
in_subgenr(-100015);
#endif
	   }
	 
#ifdef _MIND_MEM_
in_subgenr(-100016);
#endif
	 if (Pst_SubSet (slings_p, num_compound, array, size) &&
	    Pst_SubSet (array, size, slings_p, num_compound))
            {
#ifdef _MIND_MEM_
in_subgenr(-10002);
   mind_free ("array", "pst", array);
#else
   Mem_Dealloc (array, 4 * SLINGSIZE, GLOBAL);
#endif
	    return TRUE;
            }
	} 
#ifdef _MIND_MEM_
in_subgenr(-100021);
#endif
      subgoal_p = PstSubg_Brother_Get (subgoal_p);
     } 

#ifdef _MIND_MEM_
in_subgenr(-10003);
   mind_free ("array", "pst", array);
#else
   Mem_Dealloc (array, 4 * SLINGSIZE, GLOBAL);
#endif

   return FALSE;
}
/* End of Pst_Subgoal_IsDuplicate */

/****************************************************************************
*
*  Function Name:                 Pst_SymTab_Create
*
*    This routine creates a symbol table node, but does not link it into
*    the PST.
*
*  Used to be:
*
*    init_sym:
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
*    Address of newly created symbol table entry
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
SymTab_t *Pst_SymTab_Create
  (
  void
  )
{
  SymTab_t     *symtab_p;                  /* Address of node in question */

  DEBUG (R_PST, DB_PSTSYMTABCREATE, TL_PARAMS, (outbuf,
    "Entering SymTab_Create"));

#ifdef _MIND_MEM_
  mind_malloc ("symtab_p", "pst{7}", &symtab_p, SYMTABSIZE);
#else
  Mem_Alloc (SymTab_t *, symtab_p, SYMTABSIZE, GLOBAL);
#endif

  DEBUG (R_PST, DB_PSTSYMTABCREATE, TL_MEMORY, (outbuf,
    "Allocated memory for a Symbol table entry in SymTab_Create at %p",
    symtab_p));

  if (symtab_p == NULL)
    IO_Exit_Error (R_PST, X_LIBCALL,
      "No memory for symbol table entry in SymTab_Create");

  (void) memset (symtab_p, 0, SYMTABSIZE);

  SymTab_Flags_Open_Put (symtab_p, TRUE);
  SymTab_Flags_Selected_Put (symtab_p, FALSE);
  SymTab_Flags_WasSelected_Put (symtab_p, FALSE);
  SymTab_Merit_Initial_Put (symtab_p, SYMTAB_MERIT_INIT);
  SymTab_Cycle_Time_Put (symtab_p, TIME_ZERO);
  SymTab_InstancesCnt_Put (symtab_p, 1);
  SymTab_Index_Put (symtab_p, PstCB_SymtabIndex_Get (&SPstcb));
  PstCB_SymtabIndex_Put (&SPstcb, PstCB_SymtabIndex_Get (&SPstcb) + 1);

  SymTab_NumSols_Put (symtab_p, 0);
  SymTab_WorkerId_Put (symtab_p, SYM_WORKER_NONE);

  DEBUG (R_PST, DB_PSTSYMTABCREATE, TL_PARAMS, (outbuf,
    "Leaving SymTab_Create, symtab entry = %p", symtab_p));

  return symtab_p;
}
/* End of Pst_SymTab_Create */

/****************************************************************************
*
*  Function Name:                 Pst_SymTab_Dump
*
*    This routine prints a formatted dump of a Symbol Table node.
*
*  Used to be:
*
*    ???:
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
void Pst_SymTab_Dump
  (
  SymTab_t      *symbol_p,                   /* Symbol table entry to dump */
  FileDsc_t     *filed_p                     /* File to dump to */
  )
{
  FILE          *f;                          /* Temporary */
  Time_t         symtime;                    /* system dependent variable */

  f = IO_FileHandle_Get (filed_p);
  if (symbol_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL symbol table entry\n");
    return;
    }

  DEBUG_ADDR (R_PST, DB_PSTSYMTABCREATE, symbol_p);

  fprintf (f, "Symbol index:  %6lu,  inst:  %3u,  sols:  %3u  ",
    SymTab_Index_Get (symbol_p), SymTab_InstancesCnt_Get (symbol_p),
    SymTab_NumSols_Get (symbol_p));

  if (SymTab_WorkerId_Get (symbol_p) != SYM_WORKER_NONE)
    fprintf (f, "\n  worker: %3u  ", SymTab_WorkerId_Get (symbol_p));

  if (SymTab_Flags_WasSelected_Get (symbol_p))
    fprintf(f, "WAS ");

  if (SymTab_Flags_Selected_Get (symbol_p))
    fprintf(f, "SEL ");

  if (SymTab_Flags_GlobalSelect_Get (symbol_p))
    fprintf(f, "GBL ");

  if (SymTab_Flags_LocalSelect_Get (symbol_p))
    fprintf(f, "LCL ");

  if (SymTab_Flags_Unsolveable_Get (symbol_p))
    fprintf(f, "UNS ");

  if (SymTab_Flags_Stuck_Get (symbol_p))
    fprintf(f, "STK ");

  if (SymTab_Flags_Open_Get (symbol_p))
    fprintf(f, "OPN ");

  if (SymTab_Flags_Solved_Get (symbol_p))
    fprintf(f, "SOL ");

  if (SymTab_Flags_NewlySolved_Get (symbol_p))
    fprintf(f, "NSL ");

  if (SymTab_Flags_Available_Get (symbol_p))
    fprintf(f, "AVL:  %5lu", SymTab_AvailCompKey_Get (symbol_p));
  fprintf (f, "\n");

  symtime = SymTab_Cycle_Time_Get (symbol_p);
  fprintf (f, "  cycle:  %4u,  time:  %8.3f sec,  first:  %3u,  last:  %3u\n",
    SymTab_Cycle_Number_Get (symbol_p), Time_Format (symtime), 
    SymTab_Cycle_First_Get (symbol_p), SymTab_Cycle_Last_Get (symbol_p));

  fprintf (f, "  merit main:  %3u,  sol:  %3u,  init:  %3u\n",
    SymTab_Merit_Main_Get (symbol_p), SymTab_Merit_Solved_Get (symbol_p),
    SymTab_Merit_Initial_Get (symbol_p));

  fprintf (f, "  link first:  %8p,  dev:  %8p,  curr:  %8p\n",
    SymTab_FirstComp_Get (symbol_p), SymTab_DevelopedComp_Get (symbol_p),
    SymTab_Current_Get (symbol_p));

  fprintf (f, "  %s\n", (char *) Sling_Name_Get (SymTab_Sling_Get (symbol_p)));

  return;
}
/* End of Pst_SymTab_Dump */

/****************************************************************************
*
*  Function Name:                 Pst_SymTab_Index_Find
*
*    This function looks up a symbol table entry by its index value.
*
*  Used to be:
*
*    symsrch 
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
*    Address of the searched for Symbol Table Entry
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void Pst_SymTab_Index_Find
  (
  Sling_t         sling,                    /* sling to find */
  SymTab_t      **index,                    /* address of the entry */
  Boolean_t      *found                     /* already in the table */
  )
 {
  SymTab_t       *symtab_p; 
  SymTab_t       *last_p; 
  SymTab_t       *newsymtab_p;
  U32_t           hash;

  hash = Pst_Hash (sling, MX_SYMTAB_BUCKETS);
  symtab_p = SymTab_HashBucketHead_Get (hash); 
 
  if (symtab_p == NULL) 
     {
     newsymtab_p = Pst_SymTab_Create ();
     SymTab_HashBucketHead_Put (hash, newsymtab_p);
     *found = FALSE;
     SymTab_InstancesCnt_Put (newsymtab_p, 1);
     SymTab_Sling_Put (newsymtab_p, Sling_Copy(sling)); 
     *index = newsymtab_p;
     }
  else  
     {
     while (symtab_p != NULL) 
	{
	last_p = symtab_p;
	if (strcasecmp ((char *) Sling_Name_Get (sling), 
	    (char *) Sling_Name_Get (SymTab_Sling_Get (symtab_p))) == 0) 
	   {
	   *found = TRUE;
	   SymTab_InstancesCnt_Put (symtab_p, SymTab_InstancesCnt_Get (symtab_p)
	   + 1);
	   *index = symtab_p;
	   return;
	   } 
	else
	   symtab_p = SymTab_HashChain_Get (symtab_p);
	}

     *found = FALSE;
     newsymtab_p = Pst_SymTab_Create ();
     SymTab_InstancesCnt_Put (newsymtab_p, 1);
     SymTab_Sling_Put (newsymtab_p, Sling_Copy(sling)); 
     *index = newsymtab_p;
     SymTab_HashChain_Put (last_p, newsymtab_p);
     }

  return;
}
/* End of Pst_SymTab_Index_Find */

/****************************************************************************
*
*  Function Name:                 Pst_Update
*
*    This routine updates the Pst and analyses syntheses.
*
*  Used to be:
*
*     update
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
void Pst_Update
  (
  void
  )
{
  SymTab_t      *symtab_p;
  Compound_t    *cur_compound_p;
  Subgoal_t     *subgoal_p;
  Boolean_t      any_open;
  Boolean_t      any_solved;
  Boolean_t      any_just_solved;
  Boolean_t      all_unsolvable;
  U16_t          cur_merit;
  S16_t          cur_solved_merit;
  S16_t          subgoal_merit;
Boolean_t changed;
Boolean_t cc_solved;
Boolean_t *sg_solved;
int nsg;
int sginx;
 
 
  symtab_p = PstComp_SymbolTable_Get (PstCB_CurrentComp_Get (&SPstcb));
  cur_compound_p = Pst_NextCompound_Get (symtab_p, FALSE);

  if (SymTab_Flags_Selected_Get (PstComp_SymbolTable_Get (cur_compound_p))
      == TRUE)
    cur_compound_p = Pst_NextCompound_Get (symtab_p, FALSE);

cc_solved = FALSE;
  if (PstComp_Son_Get (PstCB_CurrentComp_Get (&SPstcb)) == NULL
       && SymTab_Flags_Selected_Get (symtab_p) == FALSE)
    {
    SymTab_Flags_Stuck_Put (symtab_p, TRUE);
    SymTab_Flags_Unsolveable_Put (symtab_p, TRUE);
    SymTab_Flags_Open_Put (symtab_p, FALSE);
    }
else
{
 subgoal_p = PstComp_Son_Get (cur_compound_p);
 while (!cc_solved && subgoal_p != NULL)
 {
  if (Pst_All_Solved (subgoal_p)) cc_solved = TRUE;
  subgoal_p = PstSubg_Brother_Get (subgoal_p);
 }
 if (cc_solved)
 {
  nsg = PstCB_SubgoalIndex_Get (&SPstcb);
#ifdef _MIND_MEM_
  mind_malloc ("sg_solved", "pst{8}", &sg_solved, nsg * sizeof (Boolean_t));
#else
  sg_solved = (Boolean_t *) malloc (nsg * sizeof (Boolean_t));
#endif
  for (sginx = 0; sginx < nsg; sginx++) sg_solved[sginx] = FALSE;
 }
}
 
do
{
 changed = FALSE;
  while (cur_compound_p != NULL)
    {
/*
printf("current compound = %d (STI = %d)\n",
PstComp_Index_Get(cur_compound_p),
SymTab_Index_Get(PstComp_SymbolTable_Get(cur_compound_p)));
*/
    /* only examine compounds affected by the current path */
    subgoal_p = PstComp_Son_Get (cur_compound_p);
    any_open = FALSE;
    any_solved = FALSE;
    any_just_solved = FALSE;
    all_unsolvable = TRUE;
    cur_merit = 0;
    cur_solved_merit = -1;
    while (subgoal_p != NULL)
      {
      if (Pst_All_Solved (subgoal_p) == TRUE)
{
	any_solved = TRUE;
 if (cc_solved)
 {
  sginx = PstSubg_Index_Get (subgoal_p) - 1;
  if (!sg_solved[sginx]) changed = sg_solved[sginx] = TRUE;
 }
}
 
      if (cur_compound_p == PstCB_CurrentComp_Get (&SPstcb))
	any_just_solved = any_solved;
      else
	if (PstSubg_JustSolved (subgoal_p) == TRUE)
	  any_just_solved = TRUE;
 
      if (PstSubg_Unsolvable (subgoal_p) == FALSE)
	all_unsolvable = FALSE;
 
      /*  Unconditional call to ensure solved-merit calculation for 
	  solved subgoals.
      */
      subgoal_merit = Pst_SubgoalMerit_Compute (subgoal_p);
 
      if (PstSubg_Closed (subgoal_p) == FALSE)
	{
	any_open = TRUE;
	cur_merit = MAX ((S16_t) cur_merit, subgoal_merit);
	}

      if (Pst_All_Solved (subgoal_p) == TRUE)
	cur_solved_merit = MAX (cur_solved_merit,
	   PstSubg_Merit_Solved_Get (subgoal_p));

/*
printf("\tsubgoal = %d solved=%d\n",PstSubg_Index_Get(subgoal_p),Pst_All_Solved(subgoal_p));
*/
      subgoal_p = PstSubg_Brother_Get (subgoal_p);
      }
 
    symtab_p = PstComp_SymbolTable_Get (cur_compound_p);
    if (any_solved == TRUE)
      {
      SymTab_Flags_Solved_Put (symtab_p, TRUE);
      if (any_just_solved == TRUE)
	{
	SymTab_JustSolved_Put (symtab_p, TRUE);
	SymTab_NumSols_Put (symtab_p, SymTab_NumSols_Get (symtab_p) + 1);
	}
      }
 
    if (all_unsolvable == TRUE && SymTab_Flags_Selected_Get (symtab_p) == FALSE)
      SymTab_Flags_Unsolveable_Put (symtab_p, TRUE);
 
    if (any_open == TRUE)
      {
      SymTab_Merit_Main_Put (symtab_p, cur_merit);
      }
    else
      {
      if (any_solved == TRUE)
	{
	SymTab_Merit_Main_Put (symtab_p, cur_solved_merit);
	}
      else
	{
	if (all_unsolvable == TRUE)
	  {
	  SymTab_Merit_Main_Put (symtab_p, 0);
	  }
	else
	  printf("Error in update\n");
	}

      SymTab_Flags_Open_Put (symtab_p, FALSE);
      }
 
    if (any_solved == TRUE)
      SymTab_Merit_Solved_Put (symtab_p, cur_solved_merit);
 
    cur_compound_p = Pst_NextCompound_Get (symtab_p, FALSE);
    }

 if (changed) cur_compound_p = Pst_NextCompound_Get (symtab_p, TRUE);
}
while (changed);

if (cc_solved) free (sg_solved);

  return;
}

/* End of Pst_Update */

/****************************************************************************
*
*  Function Name:                PstCB_Dump
*
*    This routine prints some of the information stored in PstCB.
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
*    N/A
*
******************************************************************************/
void PstCB_Dump
  (
  FileDsc_t * filed_p
  )
{
  FILE              *f;

  f = IO_FileHandle_Get (filed_p);

  fprintf (f, "Total expansions generated:   %6ld\n",
       PstCB_TotalExpandedCompounds_Get (&SPstcb));
  fprintf (f, "Total subgoals generated:     %6ld\n",
       PstCB_SubgoalIndex_Get (&SPstcb));
  fprintf (f, "Total compounds generated:    %6ld\n",
	PstCB_CompoundIndex_Get (&SPstcb));
  fprintf (f, "Unique compounds generated:   %6ld\n", 
	PstCB_SymtabIndex_Get (&SPstcb));
}
/* End of PstCB_Dump */

/******************************************************************************
*
*  Function Name:                 PstCB_NextExpandedComp_Put
*
*    This routine stores stores the compound node number for the currently 
*    expanded compound node in the last expanded compound field, links this 
*    node onto the expanded compound node list and increments the count of
*    expanded compounds. 
*
*  used to be:
*
*    $nxtexp
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
void PstCB_NextExpandedComp_Put 
  (
  Compound_t       *compound_p
  )
{
  if (PstCB_LastExpandedCompound_Get (&SPstcb) != NULL)
    PstComp_Next_Put (PstCB_LastExpandedCompound_Get (&SPstcb), compound_p);
  PstCB_LastExpandedCompound_Put (&SPstcb, compound_p);
  PstCB_TotalExpandedCompounds_Put (&SPstcb,
       PstCB_TotalExpandedCompounds_Get (&SPstcb) + 1); 
}
/* End of PstCB_NextExpandedComp_Put */

/****************************************************************************
*
*  Function Name:                 PstComp_NumGrandSons_Get
*
*    This function returns the number of grandsons of a compound in the PST.
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
*    Value of number of grandsons of a compound
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U16_t PstComp_NumGrandSons_Get
  (
  Compound_t    *compound_p
  )
{
  U16_t          total;
  Subgoal_t     *subgoal_p;
  Compound_t    *cur_compound_p;

  if (compound_p == NULL) 
    return 0;
 
  total = 0;
 
  subgoal_p = PstComp_Son_Get (compound_p);
  while (subgoal_p != NULL)
    {
    cur_compound_p = PstSubg_Son_Get (subgoal_p);
    while (cur_compound_p != NULL)
      {
      total++;
      cur_compound_p = PstComp_Brother_Get (cur_compound_p);
      }

    subgoal_p = PstSubg_Brother_Get (subgoal_p);
    }
 
  return total;
}
/* End of PstComp_NumGrandSons_Get */

/****************************************************************************
*
*  Function Name:                 PstComp_NumSons_Get
*
*    This function returns the number of sons of a compound in the PST.
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
*    Value of number of sons of a compound 
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U16_t PstComp_NumSons_Get
  (
  Compound_t   *compound_p
  )
{
  U16_t         count;                     /* Number of sons found */
  Subgoal_t    *subgoal_p;                 /* List traversal */
 
  DEBUG (R_PST, DB_PSTNUMSONS, TL_PARAMS, (outbuf,
    "Entering PstComp_NumSons_Get, address = %p", compound_p));
 
  count = 0;
 
  for (subgoal_p = PstComp_Son_Get (compound_p);
       subgoal_p != NULL;
       subgoal_p = PstSubg_Brother_Get (subgoal_p), count++)
    /* Empty loop body */ ;
 
  DEBUG (R_PST, DB_PSTNUMSONS, TL_PARAMS, (outbuf,
    "Leaving PstComp_NumSons_Get, count = %u", count));
 
  return count;
}
/* End of PstComp_NumSons_Get */

/****************************************************************************
*
*  Function Name:                 PstSubg_Circlar
*
*     This routine determines if any sons of the given subgoal node are already
*     on the current pathway
*
*  Used to be:
*
*   circlar
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
*    TRUE       if one of the sons is on the current pathway 
*    FALSE      otherwise
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t PstSubg_Circlar
  (
  Subgoal_t     *subgoal_p
  )
{
  Compound_t    *compound_p;

  compound_p = PstSubg_Son_Get (subgoal_p);
  while (compound_p != NULL)
    {
    if (SymTab_Current_Get (PstComp_SymbolTable_Get (compound_p)) != NULL)
      return TRUE;

    compound_p = PstComp_Brother_Get (compound_p);
    }

  return FALSE; 
}
/* End of PstSubg_Circlar */

/****************************************************************************
*
*  Function Name:                 PstSubg_Closed
*
*    This routine determine whether or not the given subgoal node subgoal_p
*    is closed.  A subgoal node is considered closed if either
*           1. all of its compound sons are closed, or
*           2. any of its sons are both closed and unsolved
*
*  Used to be:
*
*   closed 
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
*    TRUE       if the given node is closed
*    FALSE       if the given node is not closed
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t PstSubg_Closed
  (
  Subgoal_t      *subgoal_p
  )
{
  Compound_t     *compound_p;
  Boolean_t       all_closed;

  all_closed = TRUE;
  compound_p = PstSubg_Son_Get (subgoal_p);
  while (compound_p != NULL)
    {
    if (SymTab_Flags_Open_Get (PstComp_SymbolTable_Get (compound_p)) == TRUE)
      all_closed = FALSE;
    else
      if (SymTab_Flags_Solved_Get (PstComp_SymbolTable_Get (compound_p)) == 
	FALSE)
	return TRUE;

    compound_p = PstComp_Brother_Get (compound_p);
    }

  return all_closed;
}
/* End of PstSubg_Closed */

/****************************************************************************
*
*  Function Name:                 PstSubg_Instances_Get
*
*     This function counts all instances for all open, unsolved compound sons
*     that have not been developed.
*
*  Used to be:
*
*    instances 
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
*    number of instances
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U16_t  PstSubg_Instances_Get 
  (
  Subgoal_t       *subgoal_p
  )
{
  Compound_t      *compound_p;
  U16_t            total;
  SymTab_t        *symtab_p;

  total = 0;
  compound_p = PstSubg_Son_Get (subgoal_p);
  while (compound_p != NULL)
    {
    symtab_p = PstComp_SymbolTable_Get (compound_p);
    if (SymTab_Flags_Solved_Get (symtab_p) == FALSE && 
       SymTab_Flags_Open_Get (symtab_p) == TRUE && 
	SymTab_DevelopedComp_Get (symtab_p) == NULL)
      total += SymTab_InstancesCnt_Get (symtab_p);
    
    compound_p = PstComp_Brother_Get (compound_p);
    }
  return total;
} 
/* End of PstSubg_Instances_Get */

/****************************************************************************
*
*  Function Name:                 PstSubg_JustSolved
*
*  Used to be:
*
*    jsolved
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
*    TRUE      or  
*    FALSE     
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t       PstSubg_JustSolved
  (
  Subgoal_t      *subgoal_p
  )
{
  Compound_t     *compound_p;
  Boolean_t       found;
  SymTab_t       *symtab_p;

  found = FALSE;
  compound_p = PstSubg_Son_Get (subgoal_p);
  while (compound_p != NULL)
    {
    symtab_p = PstComp_SymbolTable_Get (compound_p);
    if (SymTab_Flags_NewlySolved_Get (symtab_p) == TRUE)
      found = TRUE;
    else
      if (SymTab_Flags_Solved_Get (symtab_p) == FALSE)
	return FALSE;
    compound_p = PstComp_Brother_Get (compound_p);
    }

  return found;
}
/* End of PstSubg_JustSolved */

/****************************************************************************
*
*  Function Name:                 PstSubg_NumSons_Get
*
*     This function returns the number of sons of a subgoal in the PST.
*
*  Used to be:
*
*     N/A    
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
*    Value of number of sons of a subgoal
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U16_t PstSubg_NumSons_Get
  (
  Subgoal_t    *subgoal_p 
  )
{
  U16_t         count;                     /* Number of sons found */
  Compound_t   *compound_p;                /* List traversal */

  DEBUG (R_PST, DB_PSTNUMSONS, TL_PARAMS, (outbuf,
    "Entering PstSubg_NumSons_Get, address = %p", subgoal_p));
 
  count = 0;

  for (compound_p = PstSubg_Son_Get (subgoal_p);
       compound_p != NULL;
       compound_p = PstComp_Brother_Get (compound_p), count++)
    /* Empty loop body */ ;

  DEBUG (R_PST, DB_PSTNUMSONS, TL_PARAMS, (outbuf,
    "Leaving PstSubg_NumSons_Get, count = %u", count));
 
  return count;
}
/* End of PstSubg_NumSons_Get */

/****************************************************************************
*
*  Function Name:                 PstSubg_TempClosed
*
*    This routine determines of the given subgoal node is temporarily closed.
*
*  Used to be:
*
*    tclose
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
*    TRUE       if given node is temporarily closed
*    FALSE       otherwise
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t PstSubg_TempClosed
  (
  Subgoal_t        *subgoal_p
  )
{
  if (Strategy_Subg_ClosedCnt_Get (PstSubg_Strategy_Get (subgoal_p)) > 0)
    return TRUE;
  else
    return FALSE;
}
/* End of PstSubg_TempClosed */

/****************************************************************************
*
*  Function Name:                 PstSubg_Unsolvable
*
*     This routine determines if any sons of the given subgoal node are 
*     unsolvable.       
*
*  Used to be:
*
*    unslvb
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
*    TRUE       if one of the sons is unsolvable
*    FALSE      otherwise
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t PstSubg_Unsolvable
  (
  Subgoal_t       *subgoal_p
  )
{
  Compound_t    *compound_p;

  compound_p = PstSubg_Son_Get (subgoal_p);
  while (compound_p != NULL)
    {
    if (SymTab_Flags_Unsolveable_Get (PstComp_SymbolTable_Get (compound_p)) == 
       TRUE)
      return TRUE;

    compound_p = PstComp_Brother_Get (compound_p);
    }
 
  return FALSE;
}
/* End of PstSubg_Unsolvable */

/****************************************************************************
*
*  Function Name:                 SymbolTable_Destroy
*
*    This routine destroys the table of symbols by deallocating
*    the memory for each symbol table entry.
*
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
*    Dellocates memory
*
******************************************************************************/
void SymbolTable_Destroy
  (
  void
  )
{
  SymTab_t     *back_p; 
  SymTab_t     *symtab_p;
  U16_t         tab_i;

  for (tab_i = 0; tab_i < SymTab_HashSize_Get (); ++tab_i)
    {
    symtab_p = SymTab_HashBucketHead_Get (tab_i);
    back_p = symtab_p;
    while (symtab_p != NULL)
      {
      symtab_p = SymTab_HashChain_Get (symtab_p);
      Sling_Destroy (SymTab_Sling_Get (back_p));
#ifdef _MIND_MEM_
      mind_free ("back_p", "pst", back_p);
#else
      Mem_Dealloc (back_p, SYMTABSIZE, GLOBAL);
#endif
      back_p = symtab_p;
      }
    }

  SymbolTable_Init ();

  return;
}
/* End of SymbolTable_Destroy */

/****************************************************************************
*
*  Function Name:                 SymbolTable_Dump
*
*    This routine prints a formatted dump of the entire symbol table.
*
*  Used to be:
*
*    ???:
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
void SymbolTable_Dump
  (
  FileDsc_t     *filed_p                     /* File to dump to */
  )
{
  FILE          *f;                          /* Temporary */
  SymTab_t      *symtab_p;                   /* Temporary */
  U16_t          i;                          /* Counter */

  f = IO_FileHandle_Get (filed_p);
  fprintf (f, "Symbol Table dump\n");
  for (i = 0; i < SymTab_HashSize_Get (); i++)
    {
    for (symtab_p = SymTab_HashBucketHead_Get (i); symtab_p != NULL;
	 symtab_p = SymTab_HashChain_Get (symtab_p))
      {
      fprintf (f, "\nBucket #%u\n", i);
      Pst_SymTab_Dump (symtab_p, filed_p);
      fprintf (f, "\n");
      }
    }

  return;
}
/* End of SymbolTable_Dump */

/****************************************************************************
*
*  Function Name:                 SymbolTable_Init
*
*    This routine initializes the SymbolTable.
*
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
*    Dellocates memory
*
******************************************************************************/
void SymbolTable_Init
  (
  void
  )
{
  memset (SSymbolTable, 0, sizeof (SymTab_t *) * SymTab_HashSize_Get ());

  return;
}
/* End of SymbolTable_Init */

/****************************************************************************
*
*  Function Name:                SymbolTable_PostSelect_Reset
*
*    This routine resets the newly solved flag and current compound pointers
*    in the symbol table.  It also decrements temporary closed subgoals if
*    NTCL > 0.
*
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
void SymbolTable_PostSelect_Reset
  (
  Boolean_t      ntcl_enabled,
  Boolean_t      sequential
  )
{
  SymTab_t      *symtab_p;
  U16_t          i;

  if (ntcl_enabled)
    {
    PstNode_t       node;

    PstNode_Subgoal_Put (&node, PstCB_Root_Get (&SPstcb));
    Strategy_ClosedCnt_Set (node);
    }

  for (i = 0; i < SymTab_HashSize_Get (); i++)
    for (symtab_p = SymTab_HashBucketHead_Get (i); symtab_p != NULL;
	 symtab_p = SymTab_HashChain_Get (symtab_p))
      {
      SymTab_Flags_NewlySolved_Put (symtab_p, FALSE);

      if (!sequential)
	SymTab_Current_Put (symtab_p, NULL);
      }

  return;
}
/*  End of SymbolTable_PostSelect_Reset  */

/******************************************************************************
*
*  Function Name:                 SymTab_HashBucketHead_Get
*
*    This routine returns the symbol table index of a compound with a 
*    canonical sling that has the specified hash value. 
*
*  Used to be:
* 
*    hmatch
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
SymTab_t *SymTab_HashBucketHead_Get
  (
  U32_t       index
  )
{
  return (SSymbolTable[index]);
}
/* End of SymTab_HashBucketHead_Get */

/******************************************************************************
*
*  Function Name:                 SymTab_HashBucketHead_Put
*
*    This routine stores the symbol table index of a compound into the proper 
*    position in the hash table.
*
*  used to be:
* 
*    $hmatch
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
void SymTab_HashBucketHead_Put
  (
  U32_t        index,
  SymTab_t    *value
  )
{
  SSymbolTable[index] = value;
  return;
}
/* End of SymTab_HashBucketHead_Put */

/****************************************************************************
*
*  Function Name:                SymTab_JustSolved_Put
*
*    This routine modified the flag in a symbol table entry to indicate that
*    the corresponding compound is newly solved.
*
*  Used to be:
*
*    $jslvd
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
void SymTab_JustSolved_Put
  (
  SymTab_t       *symtab_p,
  Boolean_t       value
  )
{
  SymTab_Flags_NewlySolved_Put (symtab_p, value);

  if (value == TRUE)
    {
    if (SymTab_Cycle_First_Get (symtab_p) == 0)
      SymTab_Cycle_First_Put (symtab_p, 
       (U16_t) PstCB_TotalExpandedCompounds_Get (&SPstcb));
    
    SymTab_Cycle_Last_Put (symtab_p, 
       (U16_t) PstCB_TotalExpandedCompounds_Get (&SPstcb));
    }

  return;
}
/* End of SymTab_JustSolved_Put */

/****************************************************************************
*
*  Function Name:                 Pst_Calculate
*
*
*  Used to be:
*
*    calculate
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
static void Pst_Calculate
  (
  Xtr_t        *xtr_p,
  Array_t      *sigma,
  Array_t      *z,
  Array_t      *pi,
  Array_t      *hydrogen,
  Array_t      *resonant,
  Array_t      *aromatic,
  Array_t      *preserve
  )
{
  U8_t          multiplicity;
  U16_t         neighbor;
  U16_t         i;
  U16_t         j;
  Array_t       bonds;
  U8_t          num_nonhydrogen;
  U16_t         num_atoms;
 
  num_atoms = Xtr_NumAtoms_Get (xtr_p);
#ifdef _MIND_MEM_
  mind_Array_2d_Create ("&bonds", "pst{9}", &bonds, num_atoms, MX_NEIGHBORS, BITSIZE);
#else
  Array_2d_Create (&bonds, num_atoms, MX_NEIGHBORS, BITSIZE);
#endif
 
  if (Xtr_FuncGroups_Get (xtr_p) == NULL)
    Xtr_FuncGroups_Put (xtr_p, FuncGroups_Create (xtr_p));
 
  Xtr_FuncGrp_PreservableBonds_Set (xtr_p, &bonds);
  Array_Set (sigma, 0);
  Array_Set (z, 0);
  Array_Set (pi, 0);
  Array_Set (hydrogen, 0);
  Array_Set (resonant, FALSE);
  Array_Set (aromatic, FALSE);
  Array_Set (preserve, FALSE);
 
  for (i = 0; i < num_atoms; ++i)
    if (Xtr_Attr_Atomid_Get (xtr_p, i) > 1 && Xtr_Attr_Atomid_Get (xtr_p, i) <
	VARIABLE_START)
      { 
      Array_1d1_Put (aromatic, i, Xtr_Aromat_Node_Get (xtr_p, i)); 
      num_nonhydrogen = Xtr_Attr_NumNonHydrogen_Get (xtr_p, i);
      Array_1d16_Put (hydrogen, i, Xtr_Attr_NumNeighbors_Get (xtr_p, i) -
	  num_nonhydrogen);
      for (j = 0; j < num_nonhydrogen; ++j)
	{
	neighbor = Xtr_Attr_NonHydrogenNeighbor_Get (xtr_p, i, j + 1);
	if (Xtr_Attr_Atomid_Get (xtr_p, neighbor) == CARBON)
	  Array_1d16_Put (sigma, i, Array_1d16_Get (sigma, i) + 1);
	multiplicity = Xtr_Attr_NeighborBond_Find (xtr_p, i, neighbor);
	if (multiplicity > 1 && multiplicity < 4)
	  Array_1d16_Put (pi, i, Array_1d16_Get (pi, i) + multiplicity - 1);
	if (multiplicity == BOND_RESONANT)
	  Array_1d1_Put (resonant, i, TRUE);
	if (Array_2d1_Get (&bonds, i,
	     Xtr_Attr_NeighborIndex_Find (xtr_p, i, neighbor)) == TRUE)
	  Array_1d1_Put (preserve, i, TRUE);
	}
      if (Xtr_Attr_Atomid_Get (xtr_p, i) == CARBON)
	Array_1d16_Put (z, i, 4 - (Array_1d16_Get (sigma, i) +
	    Array_1d16_Get (hydrogen, i)));
    }
 
#ifdef _MIND_MEM_
  mind_Array_Destroy ("&bonds", "pst", &bonds);
#else
  Array_Destroy (&bonds);
#endif

  return;
}
/* End of Pst_Calculate */

/****************************************************************************
*
*  Function Name:                 Pst_CarbonPenalty_Compute
*
*
*  Used to be:
*
*    carbons
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
static U16_t Pst_CarbonPenalty_Compute
  (
  Xtr_t        *xtr_p,
  Array_t       sigma,
  Array_t       hydrogen,
  Array_t       preserve,
  Array_t       aromatic,
  Array_t       resonant
  )
{
  U16_t         atom;
  U16_t         i;
  U16_t         j;
  U16_t         factor;
  U16_t         sum;
  U16_t         num_carbons;
 
  sum = 0;
  num_carbons =  Atoms_Count_Find (xtr_p, CARBON);
  
  for (i = 1; i <= num_carbons; ++i)
    {
    atom = Atoms_InstanceIndex_Get (xtr_p, CARBON, i);
    if (Ring_AtomIn (xtr_p, atom) == TRUE)
      if (Array_1d1_Get (&resonant, atom) == TRUE ||
	  Array_1d1_Get (&aromatic, atom) == TRUE)
	j = 3;
      else
	j = 2;
    else
      j = 1;
 
    if (Array_1d1_Get (&preserve, atom) == TRUE)
      factor = 3;
    else
      factor = 1;
  
    /* routine carbon table */
 
    if (Array_1d16_Get (&sigma, atom) != 0)
      {
      if (j > 3 || j < 1 || Array_1d16_Get (&hydrogen, atom) > 3 ||
	  Array_1d16_Get (&sigma, atom) > 4 ||
	  Array_1d16_Get (&sigma, atom) < 1 ||
	  Array_1d16_Get (&hydrogen, atom) + Array_1d16_Get (&sigma, atom) > 4)
	sum += (300 / factor);
      else
	sum += (Carbon_Table [j - 1][Array_1d16_Get (&hydrogen, atom)]
		[Array_1d16_Get (&sigma, atom) - 1] / factor);
      }
    }

  return (sum / 3);
}
/* End of Pst_CarbonPenalty_Compute */

/****************************************************************************
*
*  Function Name:                 Pst_CE_Adjust
*
*
*  Used to be:
*
*    ce_adjust
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
static U16_t Pst_CE_Adjust
  (
  Xtr_t        *xtr_p,
  U16_t         merit
  )
{
  Sling_t       sling;
  char         *string_p;
  char         *rest_p;
  U16_t         factor;
  U16_t         i;
  U16_t         bonus;
 
 
  sling = Name_Sling_Get (xtr_p, TRUE);
  string_p = strstr ((char *)Sling_Name_Get (sling), "|");
  if (string_p == NULL)
    {
    printf ("Error in Pst_CE_Adjust\n");
    exit (1);
    }
 
  ++string_p;
 
  if (strstr (string_p, "no ceb's") != NULL)
    {
    Sling_Destroy (sling);
    return 0;
    }

  rest_p = strstr (string_p, ",,");
  while (rest_p != NULL)
    {
    *rest_p = '\0';
    strcat (string_p, ++rest_p);
    rest_p = strstr (string_p, ",,");
    }
 
  rest_p = strstr (string_p, ",");
  bonus = 1;
  while (rest_p != NULL)
    {
    ++bonus;
    *rest_p = ' ';
    rest_p = strstr (string_p, ",");
    }
 
  for (i = 0, factor = 0; i < Xtr_NumAtoms_Get (xtr_p); ++i)
    if (Xtr_Attr_NumNeighbors_Get (xtr_p, i) > 1)
      ++factor;
 
  if (factor == 0)
    factor = 1;
 
  bonus = 20 * bonus / factor;
  bonus = MAX (0, MIN (90 - merit, bonus));

  Sling_Destroy (sling);
  return (bonus);
}
/* End of Pst_CE_Adjust */

/****************************************************************************
*  
*  Function Name:              Pst_Circular
*
*     This routine determines if any of the conjuncts in this array slings_p
*     are the same as their compound father or grandfather then this subgoal is
*     unacceptably.
*
*     Removed grandfather check as it could eliminate potential solutions.
*       Krebsbach
*
*  used to be:
*  
*    too_circular 
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
*    TRUE       if same
*    FALSE       otherwise
*
*  Side Effects:
*
*    N/A
*
****************************************************************************/
static Boolean_t  Pst_Circular
  (
  Sling_t       *slings_p,
  U16_t          num_compound
  )
{
  Sling_t      tsling;
  Sling_t      fsling;
  U16_t        i;

  tsling = PstCB_MainTarget_Get (&SPstcb);
  fsling = SymTab_Sling_Get (PstComp_SymbolTable_Get (
    PstCB_CurrentComp_Get (&SPstcb)));

  for (i = 0; i < num_compound; ++i) 
    {
    if (strcasecmp ((char *) Sling_Name_Get (tsling), 
       (char *) Sling_Name_Get (slings_p[i])) == 0)
       return TRUE;

    if (strcasecmp ((char *) Sling_Name_Get (slings_p[i]), 
       (char *) Sling_Name_Get (fsling)) == 0)
       return TRUE;
    }

  return (FALSE);
}
/* End of Pst_Circular */

/****************************************************************************
*
*  Function Name:                 Pst_FGroupPenalty_Find
*
*
*  Used to be:
*
*    fgpen
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
static U16_t Pst_FGroupPenalty_Find
  (
  Xtr_t        *xtr_p
  )
{
  U16_t         i;
  Boolean_t     flag;
 
  if (Xtr_FuncGroups_Get (xtr_p) == NULL)
    Xtr_FuncGroups_Put (xtr_p, FuncGroups_Create (xtr_p));

  flag = FALSE;

  for (i = 0; i < 49 && flag == FALSE; ++i)
    flag = (Xtr_FuncGrp_NumInstances_Get (xtr_p, Single_Attr[i][0]) >=
	    Single_Attr[i][1]);
 
  for (i = 0; i < 47 && flag == FALSE; ++i)
    flag = (Xtr_FuncGrp_NumInstances_Get (xtr_p, Pairs[i][0]) > 0 &&
	    Xtr_FuncGrp_NumInstances_Get (xtr_p, Pairs[i][1]) > 0);
 
  if (flag == TRUE)
    return 35;
  else
    return 0;
}
/* End of Pst_FGroupPenalty_Find */

/****************************************************************************
*
*  Function Name:                 Pst_FuncGrpPenalty_Compute
*
*
*  Used to be:
*
*    fgroups
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
static U16_t Pst_FuncGrpPenalty_Compute
  (
  Xtr_t        *xtr_p,
  Array_t       sigma,
  Array_t       z,
  Array_t       pi,
  Array_t       hydrogen,
  Array_t       resonant,
  Array_t       aromatic,
  Array_t       preserve
  )
{
  U16_t              sum;
  U16_t              num_carbons;
  U16_t              num_fgroups;
  U16_t              atom;
  U16_t              i;
  U16_t              ratio;
  Array_t            type;
  U16_t              total;
  U16_t              metastable;
  U16_t              count;
  U16_t              atom_id;

  sum = 0;
  metastable = 0;
  num_carbons =  Atoms_Count_Find (xtr_p, CARBON);
  for (i = 1; i <= num_carbons; ++i)
    {
    num_fgroups = 0;
    atom = Atoms_InstanceIndex_Get (xtr_p, CARBON, i);
    if (Array_1d16_Get (&z, atom) > 0)
      {
      num_fgroups += 10;
      if (Array_1d1_Get (&resonant, atom) == TRUE ||
	  Array_1d1_Get (&aromatic, atom) == TRUE)
	{
	if (Array_1d16_Get (&z, atom) == 1)
	  num_fgroups -= 10; 
	}
      else
	if (Array_1d16_Get (&pi, atom) > 0)
	  if (Array_1d16_Get (&pi, atom) == Array_1d16_Get (&z, atom))
	    num_fgroups -= 5;
 
      if (Array_1d1_Get (&preserve, atom) == TRUE)
	num_fgroups /= 5;
 
      sum += num_fgroups;
 
      if (Array_1d16_Get (&hydrogen, atom) > 0 &&
	  Array_1d16_Get (&z, atom) > Array_1d16_Get (&pi, atom) &&
	  Array_1d1_Get (&resonant, atom) == FALSE &&
	  Array_1d16_Get (&sigma, atom) > 0)
	++metastable;
      }
    }
 
  num_fgroups = sum;
  if (num_carbons == 0)
    num_carbons = 1;
 
  ratio = num_fgroups / num_carbons;
  sum = 0;
  if (ratio < 2)                /* not enough functional groups */
    sum = 10 * (2 - ratio);
  
  if (ratio > 5)               /* too many functional groups */
    sum = 10 * (ratio - 5);
 
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&type", "pst{10}", &type, 115, WORDSIZE);
#else
  Array_1d_Create (&type, 115, WORDSIZE);
#endif
  Array_Set (&type, 0);
  for (i = 0; i < Xtr_NumAtoms_Get (xtr_p); ++i)
    {
    atom_id = Xtr_Attr_Atomid_Get (xtr_p, i);
    if (atom_id < 115 && atom_id > 0)
      Array_1d16_Put (&type, atom_id, Array_1d16_Get (&type, atom_id) + 1);
    }
 
  Array_1d16_Put(&type, HYDROGEN, 0);
  Array_1d16_Put(&type, CARBON, 0);
  count = 0;
  total = 0;
  for (i = 1; i < 115; ++i)
    {
    total += Array_1d16_Get (&type, i);
    if (Array_1d16_Get (&type, i) > 0)
      ++count;
    }
 
  if (count == 0)
    sum += 20;                  /* hydrocarbon */
  else
    {
    if (Array_1d16_Get (&type, 7) > 0)
      --count;
    if (Array_1d16_Get (&type, 8) > 0)
      --count;
    }
 
  total -= (Array_1d16_Get (&type, 7) + Array_1d16_Get (&type, 8));
  sum += 10 * count;
  sum += total;
  sum += 5 * metastable;
  sum += Pst_FGroupPenalty_Find (xtr_p);
#ifdef _MIND_MEM_
  mind_Array_Destroy ("&type", "pst", &type);
#else
  Array_Destroy (&type);
#endif
 
  return (MIN (100, sum));
}
/* End of Pst_FuncGrpPenalty_Compute */

/****************************************************************************
*
*  Function Name:                       Pst_NextCompound_Get
*
*
*  Used to be:
*
*    next_compound
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
static Compound_t *Pst_NextCompound_Get
  (
  SymTab_t             *symtab_p,
  Boolean_t             reset_counter
  )
{
  static Compound_t    *cur_compound_p = NULL;
  static S16_t          counter;
  static U32_t          stack_size;
  static Array_t        array;
  Compound_t           *compound_p;
  Stack_t              *stack_p;
  Subgoal_t            *father_p;
  SymTab_t             *father_sym_p;
  S32_t                 i;
  StackElement_t       *elem_p;
 
  if (reset_counter) counter = -1;
  else if (cur_compound_p != PstCB_CurrentComp_Get (&SPstcb))
    {
    if (Array_Storage_Get (&array) != NULL)
#ifdef _MIND_MEM_
      mind_Array_Destroy ("&array", "pst", &array);
#else
      Array_Destroy (&array);
#endif
 
    stack_p = Stack_Create (STACK_NORMAL);
    Stack_PushAdd (stack_p, symtab_p);
    counter = 1;
    cur_compound_p = PstCB_CurrentComp_Get (&SPstcb);
    compound_p = SymTab_FirstComp_Get (symtab_p);
 
    /* for each instance of each compound affected by the current path */
 
    while (compound_p != NULL)
      {
      father_p = PstComp_Father_Get (compound_p);
      if (father_p != PstCB_Root_Get (&SPstcb))
	{
	father_sym_p = PstComp_SymbolTable_Get (PstSubg_Father_Get (father_p));
	if (Pst_Stack_Contain (stack_p, father_sym_p) == FALSE)
	  Stack_PushAdd (stack_p, father_sym_p);
	}
      compound_p = PstComp_Prev_Get(compound_p);
      if (compound_p == NULL)
	if (counter < Stack_Size_Get (stack_p))
	  {
	  ++counter;
	  elem_p = Stack_Top_Get (stack_p);
	  for (i = 0; i < Stack_Size_Get (stack_p) - counter; ++i) 
	    elem_p = StkElem_Prev_Get  (elem_p);

	  compound_p = SymTab_FirstComp_Get (
	      (SymTab_t *) StkElem_ValueAdd_Get (elem_p));
	  }
      }
    stack_size = Stack_Size_Get (stack_p);
#ifdef _MIND_MEM_
    mind_Array_1d_Create ("&array", "pst{11}", &array, stack_size, ADDRSIZE);
#else
    Array_1d_Create (&array, stack_size, ADDRSIZE);
#endif
 
    for (i = (S32_t) Stack_Size_Get (stack_p) - 1; i >= 0; --i)
      {
      Array_1d32_Put (&array, i, (SymTab_t *)(Stack_TopAdd (stack_p)));
      Stack_Pop_Save (stack_p);
      }
    Stack_Destroy (stack_p);
    counter = -1;
    }
 
  if (counter == stack_size - 1)        /* end of list */
    {
    cur_compound_p = NULL;
    return NULL;
    }
 
  ++counter;

  return (SymTab_DevelopedComp_Get (
      (SymTab_t *)(Array_1d32_Get (&array, counter))));
}
/* End of Pst_NextCompound_Get */

/****************************************************************************
*
*  Function Name:                 Pst_RingPenalty_Compute
*
*
*  Used to be:
*
*    rings
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
static U16_t Pst_RingPenalty_Compute
  (
  Xtr_t        *xtr_p,
  Array_t       resonant,
  Array_t       aromatic,
  Array_t       preserve
  )
{
  U16_t         num_ringsys;
  U16_t         num_primary_cycles;
  U16_t         sum;
  U16_t         sum1;
  U16_t         complexity;
  U16_t         total;
  Array_t       rs;
  Array_t       nodes;
  U16_t         cyclesize;
  U16_t         i;
  U16_t         j;
  U16_t         k;
  U16_t         index;
  U16_t         rtval;
  Boolean_t     flag;
  U16_t         row;
  U16_t         counter;
 
  sum = 0;
  complexity = 0;
  num_ringsys = Xtr_Rings_NumRingSys_Get (xtr_p);
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&rs", "pst{12}", &rs, Xtr_NumAtoms_Get (xtr_p), WORDSIZE);
#else
  Array_1d_Create (&rs, Xtr_NumAtoms_Get (xtr_p), WORDSIZE);
#endif
  Array_Set (&rs, XTR_INVALID);
 
  for (i = 0; i < num_ringsys; ++i)
    {
    sum1 = 0;
    complexity = 0;
    num_primary_cycles = Ringdef_NumPrimaryCycles_Find (xtr_p, i);
    for (j = 0; j < num_primary_cycles; ++j)
      {
      cyclesize = Ringdef_CycleSize_Find (xtr_p, i, j);
#ifdef _MIND_MEM_
      mind_Array_1d_Create ("&nodes", "pst{12}", &nodes, cyclesize, WORDSIZE);
#else
      Array_1d_Create (&nodes, cyclesize, WORDSIZE);
#endif
      Ringdef_PrimaryRingNodes_Get (xtr_p, i, j, &nodes);
      flag = FALSE;
      counter = 0;
      for (k = 0; k < cyclesize; ++k)
	{
	index = Array_1d16_Get (&nodes, k);
	Array_1d16_Put (&rs, index, i);
	if (Array_1d1_Get (&resonant, index) == TRUE ||
	    Array_1d1_Get (&aromatic, index) == TRUE)
	  flag = TRUE;
	if (Array_1d1_Get (&preserve, index) == TRUE)
	  ++counter;
	}
      if (counter < 2)
	{
	   /* routine ring_table */
 
	row = 1;
	if (cyclesize > 10)
	  rtval = cyclesize * 10;
	else
	  if (cyclesize < 3)
	    rtval = 100;
	  else
	    {
	    if (Ringdef_Carbocyclic_Find (xtr_p, i, j) == FALSE)
	      row = 3;
	    if (flag == TRUE)
	      ++row;
	    rtval = Ring_Table [row - 1][cyclesize - 3];
	    }
 
	if (num_primary_cycles == 1 && rtval <= 10)
	  rtval /= 2;
	sum1 += rtval;
	complexity += cyclesize;
	}  
#ifdef _MIND_MEM_
      mind_Array_Destroy ("&nodes", "pst", &nodes);
#else
      Array_Destroy (&nodes);
#endif
      }
 
    for (j = 0, total = 0; j < Xtr_NumAtoms_Get (xtr_p); ++j)
      if (Array_1d16_Get (&rs, j) == i)
	++total;
  
    complexity = complexity * 100 / total;
 
    sum += complexity * sum1 / 100;
    }
 
#ifdef _MIND_MEM_
  mind_Array_Destroy ("&rs", "pst", &rs);
#else
  Array_Destroy (&rs);
#endif

  return (sum);
}
/* End of Pst_RingPenalty_Compute */

/****************************************************************************
*
*  Function Name:                 Pst_Stack_Contain
*
*    This routine determines if a given symtab_p is in a stack.
*
*  Used to be:
*
*    in_stack
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
static Boolean_t Pst_Stack_Contain
  (
  Stack_t       *stack_p,
  SymTab_t      *symtab_p
  )
{
  StackElement_t        *elem_tmp;
 
  if (Stack_Size_Get (stack_p) == 0)
    return FALSE;
 
  elem_tmp = Stack_Top_Get (stack_p);
  while (elem_tmp != NULL)
    {
    if (StkElem_ValueAdd_Get (elem_tmp) == symtab_p)
      return TRUE;
    else
      elem_tmp = StkElem_Prev_Get  (elem_tmp);
    }
 
  return FALSE;
}
/* End of Pst_Stack_Contain */

/****************************************************************************
*
*  Function Name:               Pst_SubgoalMerit_Compute
*
*    This routine calculates the subgoal merit.  If there are any unsolved
*    conjunct compounds then the adjusted compound merit is calculated using
*    only the unsolved conjuncts.  If all conjuncts are solved then the
*    adjusted compound merit is calculated using all the conjuncts.  The
*    adjusted compound merit is the minimum merit of all the conjunct merits
*    being considered less a penalty dependent on the number and quality of
*    the other conjuncts.
*
*  Used to be:
*
*    subgoal_merit
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
static S16_t Pst_SubgoalMerit_Compute
  (
  Subgoal_t    *subgoal_p
  )
{
  Boolean_t     all_solved;
  U16_t         reaction_merit;
  S16_t         final_merit;
  U16_t         min_merit;
  U16_t         min_smerit;
  U16_t         total_merit;
  U16_t         total_smerit;
  U16_t         counter;
  U16_t         compound_merit;
  S16_t         solved_merit;
  U16_t         compound_smerit;
  S16_t         adjusted_merit;
  S16_t         adjusted_csmerit;
  Compound_t   *compound_p;
  SymTab_t     *symtab_p;
 
  min_merit = MX_COMP_MERIT;
  min_smerit = MX_COMP_MERIT;
  total_smerit = 0;
  total_merit = 0;
  reaction_merit = Pst_ReactionMerit_Get (subgoal_p);
  all_solved = Pst_All_Solved (subgoal_p);
  counter = 0;
  compound_p = PstSubg_Son_Get (subgoal_p);
  while (compound_p != NULL)
    {
    symtab_p = PstComp_SymbolTable_Get(compound_p);
    if (all_solved == TRUE || SymTab_Flags_Solved_Get(symtab_p) == FALSE)
      {
      compound_merit = SymTab_Merit_Main_Get(symtab_p);
      ++counter;
      min_merit = MIN (min_merit, compound_merit);
      total_merit += compound_merit;
      if (all_solved == TRUE)
	{
	compound_smerit = SymTab_Merit_Solved_Get(symtab_p);
	min_smerit = MIN (min_smerit, compound_smerit);
	total_smerit += compound_smerit;
	}
      }
      compound_p = PstComp_Brother_Get (compound_p);
    }
 
  adjusted_merit = min_merit - (MX_COMP_MERIT * (counter - 1) + 
       min_merit - total_merit) / 20;

  final_merit = MAX (adjusted_merit, 0);
  final_merit = final_merit * reaction_merit / MX_COMP_MERIT; 
  if (PstSubg_Reaction_Confidence_Get(subgoal_p) < 75)
    final_merit -= 2;
 
  if (all_solved == TRUE)
    {
    adjusted_csmerit = min_smerit - (MX_COMP_MERIT * (counter - 1) + 
       min_smerit - total_smerit) / 20; 
	
    solved_merit = (reaction_merit * MAX (adjusted_csmerit, 0)) / MX_COMP_MERIT;
    }
 
  PstSubg_Merit_Main_Put(subgoal_p, final_merit);
  if (all_solved == TRUE)
    PstSubg_Merit_Solved_Put(subgoal_p, solved_merit);
 
  return (final_merit);
}
/* End of Pst_SubgoalMerit_Compute */
 
/****************************************************************************
*
*  Function Name:               Pst_SubSet 
*
*    This routine determines whether the names in slings1_p are a subset of 
*    those in slings2_p.  It assumes that there are no duplicate names in
*    either array.
*
*  Used to be:
*
*    subset
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
*    TRUE       or
*    FALSE
*
*  Side Effects:
*
*    N/A
*
****************************************************************************/
static Boolean_t Pst_SubSet 
 (
 Sling_t       *slings1_p,
 U16_t          size1,
 Sling_t       *slings2_p,
 U16_t          size2
 )
{
   Boolean_t       match;
   U16_t       i;
   U16_t       j;

   if (size1 > size2)
      return FALSE;
  
   for (i = 0; i < size1; ++i) 
     {
     match = FALSE;
     for (j= 0; j < size2 && match != TRUE; ++j) 
	if (strcasecmp ((char *) Sling_Name_Get (slings1_p[i]), 
	    (char *) Sling_Name_Get (slings2_p[j])) == 0)
	  match = TRUE;
     if (match == FALSE)
	 return FALSE;
     }

   return TRUE;
}
/* End of Pst_SubSet */

/****************************************************************************
*
*  Function Name:                 Dis_Pst_WorkerRoot_Set
*
*    This function takes the globally selected compound and begins a new
*    local search using the workers local PST.  Only certain fields are
*    reinitialized.
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
Compound_t *Dis_Pst_WorkerRoot_Set
  (
  Sling_t       selcomp,                   /* Target compound */
  U16_t         comp_merit
  )
{
  Subgoal_t    *subg_p;                    /* Primary goal of this run */
  Compound_t   *compound_p;                /* Goal compound */
  SymTab_t     *symtab_p;                  /* Symbol table entry */
  Boolean_t     found;

  /* Initialize the PST global data */

  subg_p = Pst_SubGoal_Create ();
  compound_p = Pst_Compound_Create ();
  PstCB_Root_Put (&SPstcb, subg_p);
  PstCB_CurrentComp_Put (&SPstcb, compound_p);

  /*  Since workers may now retain symboltable, find symtab instead of
      assuming it is a new bucket head.  Since Pst_SymTab_Index_Find
      makes a copy of the sling, destroy the copy and store the given
      sling.
  */

  Pst_SymTab_Index_Find (selcomp, &symtab_p, &found);
  Sling_Destroy (SymTab_Sling_Get (symtab_p));
  SymTab_Sling_Put (symtab_p, selcomp);
  SymTab_FirstComp_Put (symtab_p, compound_p);
  SymTab_Merit_Main_Put (symtab_p, comp_merit);

  /* All but father, symtab of initial compound node is NULL or zero */

  PstComp_Father_Put (compound_p, subg_p);
  PstComp_SymbolTable_Put (compound_p, symtab_p);
  PstComp_Brother_Put (compound_p, NULL);
  PstComp_Son_Put (compound_p, NULL);
  PstComp_Prev_Put (compound_p, NULL);

  /* All but son of initial subgoal is NULL or zero */

  PstSubg_Son_Put (subg_p, compound_p);
  PstSubg_Brother_Put (subg_p, NULL);
  PstSubg_Father_Put (subg_p, NULL);

  /* Initial symbol table entry is mostly normal, except we know that
     the current hashtable is empty.
  */


  return compound_p;
}
/* End of Dis_Pst_WorkerRoot_Set */

/****************************************************************************
*
*  Function Name:                 Dis_PstCB_Worker_Clear
*
*    This function clears the PstCB for the next local search.  The Shelved
*    Merits are retained so that they can be dumped for the entire run.
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
void Dis_PstCB_Worker_Clear
  (
  Boolean_t      share_sels
  )
{
  PstNode_t      node;

  /* Clear the PST global data */

  if (PstCB_Root_Get (&SPstcb) == NULL)
    return;

  PstNode_Subgoal_Put (&node, PstCB_Root_Get (&SPstcb));
  Pst_Destroy (node);
  if (share_sels)
    SymbolTable_Clear ();
  else
    {
    SymbolTable_Destroy ();
    PstCB_SymtabIndex_Put (&SPstcb, 1);
    }

  PstCB_CompoundIndex_Put (&SPstcb, 1);
  PstCB_TotalExpandedCompounds_Put (&SPstcb, 0);
  PstCB_SubgoalIndex_Put (&SPstcb, 1);

  PstCB_Root_Put (&SPstcb, NULL);
  PstCB_CurrentComp_Put (&SPstcb, NULL);
  PstCB_LastExpandedCompound_Put (&SPstcb, NULL);

  return ;
}
/* End of Dis_PstCB_Worker_Clear */

/****************************************************************************
*
*  Function Name:                 Dis_PstCB_Worker_Init
*
*    This function takes the goal compound (of the whole run!) and enters
*    it into the PST.  Before doing so, it creates and initializes all the
*    data-structures used by the PST.
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
void Dis_PstCB_Worker_Init
  (
  Sling_t        main_target,               /* Sling of the main target */
  float          carbon,                    /* Merits of main target */
  float          fgroup,
  float          ring
  )
{
  /* Initialize the PST global data */

  PstCB_Expansions_Put (&SPstcb, List_Create (LIST_NORMAL));

  PstCB_CompoundIndex_Put (&SPstcb, 1);
  PstCB_TotalExpandedCompounds_Put (&SPstcb, 0);
  PstCB_SubgoalIndex_Put (&SPstcb, 1);
  PstCB_SymtabIndex_Put (&SPstcb, 1);

  PstCB_MainTarget_Put (&SPstcb, main_target);
  PstCB_MeritCarbons_Put (&SPstcb, carbon);
  PstCB_MeritFGroups_Put (&SPstcb, fgroup);
  PstCB_MeritRings_Put (&SPstcb, ring);

  PstCB_Root_Put (&SPstcb, NULL);
  PstCB_CurrentComp_Put (&SPstcb, NULL);
  PstCB_LastExpandedCompound_Put (&SPstcb, NULL);
  SymbolTable_Init ();

  return ;
}
/* End of Dis_PstCB_Worker_Init */

/****************************************************************************
*
*  Function Name:              Pst_Any_Selected
*
*    A subgoal is considered selected if any son of a given subgoal
*    is currently a selected compound,
*
*  Used to be:
*
*    solved
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
*    TRUE       if any is selected
*    FALSE       otherwise
*
*  Side Effects:
*
*    N/A
*
****************************************************************************/
Boolean_t  Pst_Any_Selected
  (
  Subgoal_t     *subgoal_p
  )
{
  Compound_t    *compound_p;
  SymTab_t      *symtab_p;

  compound_p = PstSubg_Son_Get (subgoal_p);
  symtab_p = PstComp_SymbolTable_Get (compound_p);
  while (compound_p != NULL)
    {
    if (SymTab_Flags_Selected_Get (symtab_p) == TRUE)
      return TRUE;

    compound_p = PstComp_Brother_Get (compound_p);
    }

  return FALSE;
}
/* End of Pst_Any_Selected */

/****************************************************************************
*
*  Function Name:               Pst_LBM_CmpMerit_Compute
*
*    For Lower Bound Merit:  this routine calculates the inverse of the
*    subgoal merit function.  That is, given a lower bound on the subgoal
*    merit, calculate the lower bound on the selected compound merit.
*
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
U16_t Pst_LBM_CmpMerit_Compute
  (
  Compound_t    *selcomp_p,
  U16_t          new_sg_merit
  )
{
  Compound_t    *compound_p;
  Subgoal_t     *subgoal_p;
  SymTab_t      *symtab_p;
  U16_t         compound_merit;
  U16_t         counter;
  U16_t         reaction_merit;
  U16_t         sg_adj;
  Boolean_t     all_solved;

  if (selcomp_p == NULL)
    return 0;

  sg_adj = 0;
  counter = 0;
  subgoal_p = PstComp_Father_Get (selcomp_p);
  reaction_merit = Pst_ReactionMerit_Get (subgoal_p);
  all_solved = Pst_All_Solved (subgoal_p);
  compound_p = PstSubg_Son_Get (subgoal_p);
  while (compound_p != NULL)
    {
    symtab_p = PstComp_SymbolTable_Get (compound_p);
    if (selcomp_p != compound_p 
	&& (all_solved == TRUE 
	    || SymTab_Flags_Solved_Get (symtab_p) == FALSE))
      {
      sg_adj += SymTab_Merit_Main_Get (symtab_p);
      ++counter;
      }
      compound_p = PstComp_Brother_Get (compound_p);
    }
 
  sg_adj = (MX_COMP_MERIT * counter - sg_adj) / 20;

  if (PstSubg_Reaction_Confidence_Get (subgoal_p) < 75)
    new_sg_merit += 2;

  new_sg_merit = new_sg_merit * MX_COMP_MERIT / reaction_merit; 

  compound_merit = (new_sg_merit + sg_adj);
 
  return compound_merit;
}
/*  End of Pst_LBM_CmpMerit_Compute  */

/****************************************************************************
*
*  Function Name:               Pst_LBM_SgMerit_Recompute
*
*    For Lower Bound Merit:  this routine recalculates the subgoal merit 
*    assuming that the compound along the selected pathway now has the 
*    given new merit.
*
*    If there are any unsolved conjunct compounds then the adjusted compound 
*    merit is calculated using only the unsolved conjuncts.  
*    If all conjuncts are solved then the adjusted compound merit is 
*    calculated using all the conjuncts.  The adjusted compound merit is
*    the minimum merit of all the conjunct merits being considered less
*    a penalty dependent on the number and quality of the other conjuncts.
*
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
U16_t Pst_LBM_SgMerit_Recompute
  (
  Compound_t    *pathcomp_p,
  U16_t          new_cmp_merit
  )
{
  Compound_t    *compound_p;
  Subgoal_t     *subgoal_p;
  SymTab_t      *symtab_p;
  U16_t          reaction_merit;
  U16_t          final_merit;
  U16_t          min_merit;
  U16_t          sg_adj;
  U16_t          total_merit;
  U16_t          counter;
  U16_t          compound_merit;
  Boolean_t      all_solved;

  if (pathcomp_p == NULL)
    return (0);

  min_merit = MX_COMP_MERIT;
  total_merit = 0;
  counter = 0;
  subgoal_p = PstComp_Father_Get (pathcomp_p);
  reaction_merit = Pst_ReactionMerit_Get (subgoal_p);
  all_solved = Pst_All_Solved (subgoal_p);
  compound_p = PstSubg_Son_Get (subgoal_p);
  while (compound_p != NULL)
    {
    symtab_p = PstComp_SymbolTable_Get (compound_p);
    if (all_solved == TRUE || SymTab_Flags_Solved_Get (symtab_p) == FALSE)
      {
      if (pathcomp_p == compound_p)
	compound_merit = new_cmp_merit;
      else
	compound_merit = SymTab_Merit_Main_Get (symtab_p);

      ++counter;
      min_merit = MIN (min_merit, compound_merit);
      total_merit += compound_merit;
      }
      compound_p = PstComp_Brother_Get (compound_p);
    }

  sg_adj = (MX_COMP_MERIT * (counter - 1) + min_merit - total_merit) / 20;
  final_merit = (min_merit > sg_adj) ? min_merit - sg_adj : 0;
  final_merit = final_merit * reaction_merit / MX_COMP_MERIT; 

  if (PstSubg_Reaction_Confidence_Get (subgoal_p) < 75)
    final_merit = (final_merit > 2) ? final_merit - 2 : 0;
  
  return final_merit;
}
/*  End of Pst_LBM_SgMerit_Recompute  */

/****************************************************************************
*
*  Function Name:                 Pst_LowerBoundMerit_Find
*
*    For a given selected compound, this function attempts to find the
*    the subgoal with the next best merit, and then calculates a lower 
*    bound for the selected compound merit.
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
U16_t Pst_LowerBoundMerit_Find
  (
  Stack_t        *path_p,           /* Path from Strategy_Select_Next () */
  Compound_t     *selcmp_p          /* Selected compound */
  )
{
  Stack_t        *nbsg_stack;
  StackElement_t *elem_p;
  Compound_t     *curcmp_p;
  Subgoal_t      *cursg_p;
  Subgoal_t      *subgoal_p;
  U16_t           cur_merit;
  U16_t           max_merit;
  Boolean_t       done;

  if (selcmp_p == NULL)
    return (0);

  /*  Perculate up the selected pathway the next best subgoal merit until
      no possible larger next best merit is found.
  */
  curcmp_p = selcmp_p;
  cursg_p = PstComp_Father_Get (curcmp_p);

  /*  If the current compound is the target compound, force the worker to
      wait for the next global selection after expanding target.
  */
  if (cursg_p == PstCB_Root_Get (&SPstcb))
    return (MX_COMP_MERIT);

  nbsg_stack = Stack_Create (STACK_NORMAL);
  max_merit = 0;

  Stack_PushAdd (nbsg_stack, curcmp_p);

  subgoal_p = PstComp_Son_Get (PstSubg_Father_Get (cursg_p));
  while (subgoal_p != NULL)
    {
    if (subgoal_p != cursg_p 
	&& !PstSubg_Closed (subgoal_p) 
	&& !PstSubg_TempClosed (subgoal_p) 
	&& !PstSubg_Circlar (subgoal_p)
	&& PstSubg_Merit_Main_Get (subgoal_p) > (S16_t) max_merit)
      {
      max_merit = (U16_t) PstSubg_Merit_Main_Get (subgoal_p);
      }

    subgoal_p = PstSubg_Brother_Get (subgoal_p);
    }

  if (path_p != NULL)
    {
    done = FALSE;
    elem_p = Stack_Top_Get (path_p);
    while (elem_p != NULL && !done)
      {
      curcmp_p = (Compound_t *) StkElem_ValueAdd_Get (elem_p);
      cursg_p = PstComp_Father_Get (curcmp_p);
      if (cursg_p == PstCB_Root_Get (&SPstcb))
	{
	done = TRUE;
	}
      else
	{
	Stack_PushAdd (nbsg_stack, curcmp_p);
	if (max_merit > 0)
	  max_merit = Pst_LBM_SgMerit_Recompute (curcmp_p, max_merit);

	cur_merit = 0;
	subgoal_p = PstComp_Son_Get (PstSubg_Father_Get (cursg_p));
	while (subgoal_p != NULL)
	  {
	  if (subgoal_p != cursg_p 
	      && !PstSubg_Closed (subgoal_p) 
	      && !PstSubg_TempClosed (subgoal_p) 
	      && !PstSubg_Circlar (subgoal_p)
	      && PstSubg_Merit_Main_Get (subgoal_p) > (S16_t) cur_merit)
	    {
	    cur_merit = (U16_t) PstSubg_Merit_Main_Get (subgoal_p);
	    }

	  subgoal_p = PstSubg_Brother_Get (subgoal_p);
	  }

	if (cur_merit > max_merit)
	  {
	  max_merit = cur_merit;
	  }
	else
	  {
	  if (cur_merit != 0)
	    {
	    done = TRUE;
	    }
	  }
	}

      elem_p = StkElem_Prev_Get (elem_p);
      }
    }

  /*  Trickle back down the selected pathway computing the lower bound on
      the compound merit.
  */
  curcmp_p = (Compound_t *) Stack_TopAdd (nbsg_stack);
  while (curcmp_p != NULL)
    {
    max_merit = Pst_LBM_CmpMerit_Compute (curcmp_p, max_merit);
    Stack_Pop_Save (nbsg_stack);
    curcmp_p = (Compound_t *) Stack_TopAdd (nbsg_stack);
    }

  Stack_Destroy_Safe (nbsg_stack);

  return (max_merit);
}
/*  End of Pst_LowerBoundMerit_Find  */

/****************************************************************************
*
*  Function Name:                 Pst_PostSel_Update
*
*    This routine updates the Pst and analyses syntheses.  Since no
*    new expansions have been added to the PST, we need not update
*    information concerning new solutions, etc.
*
*  Used to be:
*
*     N/A
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
void Pst_PostSel_Update
  (
  Boolean_t     and_parallel
  )
{
  Compound_t    *cmp_p;
  Compound_t    *cur_compound_p;
  Subgoal_t     *subgoal_p;
  SymTab_t      *cursym_p;
  SymTab_t      *symtab_p;
  U16_t          cur_merit;

  /*  Rather than re-evaluating each child subgoal of a compound instance
      that may have been affected by the possible change in the merit of
      the selected symbol, only re-evaluate the parent subgoal of a compound
      with a new merit.
  */

  symtab_p = PstComp_SymbolTable_Get (PstCB_CurrentComp_Get (&SPstcb));
  cur_compound_p = Pst_NextCompound_Get (symtab_p, FALSE);

  /*  First re-evaluate the parent subgoals of all instances of the 
      selected compound.  If allowing and-parallelism, only change the
      subgoal if there is no other open conjunct of the subgoal.
  */
  cmp_p = SymTab_FirstComp_Get (symtab_p);
  while (cmp_p != NULL)
    {
    subgoal_p = PstComp_Father_Get (cmp_p);
    if (and_parallel)
      {
      Compound_t    *sibcmp_p;
      SymTab_t      *sibsym_p;
      Boolean_t      any_openselconj;

      sibcmp_p = PstSubg_Son_Get (subgoal_p);
      any_openselconj = FALSE;
      while (sibcmp_p != NULL && !any_openselconj)
	{
	sibsym_p = PstComp_SymbolTable_Get (sibcmp_p);
	if (!SymTab_Flags_Selected_Get (sibsym_p) 
	    && SymTab_Flags_Open_Get (sibsym_p))
	  any_openselconj = TRUE;

	sibcmp_p = PstComp_Brother_Get (sibcmp_p);
	}

      if (!any_openselconj)
	PstSubg_Merit_Main_Put (subgoal_p, SUBG_MERIT_SELECT);
      }

    else
      PstSubg_Merit_Main_Put (subgoal_p, SUBG_MERIT_SELECT);

    cmp_p = PstComp_Prev_Get (cmp_p);    
    }

  /*  Since we are working bottom-up, all changes to the subgoals of
      a compound still on the next compound list should have already 
      been made, so see if the there is subgoal with a higher merit
      than the current subgoal merit.  If so, re-evaluate the current
      compound merit and its father subgoal.  Otherwise, skip this
      compound.
  */

  cur_compound_p = Pst_NextCompound_Get (symtab_p, FALSE);

  while (cur_compound_p != NULL)
    {
    cursym_p = PstComp_SymbolTable_Get (cur_compound_p);
    subgoal_p = PstComp_Son_Get (SymTab_DevelopedComp_Get (cursym_p));
    cur_merit = 0;
    while (subgoal_p != NULL)
      {
      if (PstSubg_Merit_Main_Get (subgoal_p) > (S16_t) cur_merit)
	 cur_merit = (U16_t) PstSubg_Merit_Main_Get (subgoal_p);
      
      subgoal_p = PstComp_Brother_Get (subgoal_p);
      }

    if (cur_merit > SymTab_Merit_Main_Get (cursym_p))
      {
      /*  Re-evaluate the parent subgoals of all instances of this compound.
      */
      SymTab_Merit_Main_Put (cursym_p, cur_merit);
      
      while (cur_compound_p != NULL 
	  && PstComp_SymbolTable_Get (cur_compound_p) == cursym_p)
	{
	Compound_t   *compound_p;
	SymTab_t     *tmpsym_p;
	U16_t         min_merit;
	U16_t         total_merit;
	U16_t         counter;
	U16_t         compound_merit;
	S16_t         final_merit;
	S16_t         sg_adj;
	Boolean_t     all_solved;

	subgoal_p = PstComp_Father_Get (cur_compound_p);

	min_merit = MX_COMP_MERIT;
	total_merit = 0;
	all_solved = Pst_All_Solved (subgoal_p);
	counter = 0;
	compound_p = PstSubg_Son_Get (subgoal_p);
	while (compound_p != NULL)
	  {
	  tmpsym_p = PstComp_SymbolTable_Get (compound_p);
	  if (all_solved || !SymTab_Flags_Solved_Get (tmpsym_p))
	    {
	    compound_merit = SymTab_Merit_Main_Get (tmpsym_p);
	    ++counter;
	    min_merit = MIN (min_merit, compound_merit);
	    total_merit += compound_merit;
	    }

	  compound_p = PstComp_Brother_Get (compound_p);
	  }

	sg_adj = (S16_t) (MX_COMP_MERIT * (counter - 1) + min_merit 
	  - total_merit) / 20;
	final_merit = ((S16_t) min_merit > sg_adj) 
	  ? (S16_t) min_merit - sg_adj : 0;
	final_merit = final_merit * Pst_ReactionMerit_Get (subgoal_p) 
	  / MX_COMP_MERIT; 
	if (PstSubg_Reaction_Confidence_Get (subgoal_p) < 75)
	  final_merit -= 2;

	PstSubg_Merit_Main_Put (subgoal_p, final_merit);

	cur_compound_p = Pst_NextCompound_Get (symtab_p, FALSE);
	}
      }

    else
      {
      /*  Since the merit of this symbol did not change, we can remove
	  all instances of this compound from the next list.
      */
      while (cur_compound_p != NULL 
	  && PstComp_SymbolTable_Get (cur_compound_p) == cursym_p)
	cur_compound_p = Pst_NextCompound_Get (symtab_p, FALSE);
      }
    }

  return;
}

/* End of Pst_PostSel_Update */

/****************************************************************************
*
*  Function Name:                 Pst_Symbol_Selected
*
*    This function determines whether a symbol in the symbol table has
*    already been selected by another worker.  If the symbol doesn't exist
*    in the table, an entry is created for it.  Hopefully, this won't
*    interfere with the normal use of the table.
*
*  Used to be:
*
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
*    Whether symbol in symbol table has already been selected.
*
*  Side Effects:
*
*    Create entry for non-existing symbols.
*
******************************************************************************/
Boolean_t Pst_Symbol_Selected
  (
  Sling_t         sling,
  Time_t         *update_time,
  U16_t           worker,
  Boolean_t       local,
  Boolean_t       nextbest,
  Boolean_t       and_par
  )
 {
  SymTab_t       *symtab_p; 
  SymTab_t       *last_p; 
  SymTab_t       *newsymtab_p;
  U32_t           hash;

  *update_time = TIME_ZERO;
  hash = Pst_Hash (sling, MX_SYMTAB_BUCKETS);
  symtab_p = SymTab_HashBucketHead_Get (hash); 
 
  if (symtab_p == NULL) 
    {
    newsymtab_p = Pst_SymTab_Create ();
    SymTab_HashBucketHead_Put (hash, newsymtab_p);
    SymTab_InstancesCnt_Put (newsymtab_p, 0);
    SymTab_Sling_Put (newsymtab_p, Sling_Copy (sling));
    SymTab_Flags_Selected_Put (newsymtab_p, TRUE);
    SymTab_Flags_WasSelected_Put (newsymtab_p, TRUE);
    SymTab_Merit_Main_Put (newsymtab_p, 0);
    SymTab_Merit_Solved_Put (newsymtab_p, 0);
    SymTab_WorkerId_Put (newsymtab_p, worker);
    if (local)
      {
      SymTab_Flags_LocalSelect_Put (newsymtab_p, TRUE);
      }
    else
      {
      SymTab_Flags_GlobalSelect_Put (newsymtab_p, TRUE);
      }
    
    return (FALSE);
    }

  while (symtab_p != NULL) 
    {
    last_p = symtab_p;
    if (strcasecmp ((char *) Sling_Name_Get (sling), 
	(char *) Sling_Name_Get (SymTab_Sling_Get (symtab_p))) == 0) 
      {
      if (SymTab_Flags_WasSelected_Get (symtab_p))
	{
	SymTab_Flags_DupSelect_Put (symtab_p, TRUE);
	return (TRUE);
	}
      else
	{
	/*  Symbol is already in PST and has not been previously selected,
	    so do an update if using global nextbest search.
	*/
	SymTab_Flags_Selected_Put (symtab_p, TRUE);
	SymTab_Flags_WasSelected_Put (symtab_p, TRUE);
	SymTab_WorkerId_Put (symtab_p, worker);
	SymTab_Merit_Main_Put (symtab_p, 0);
	SymTab_Merit_Solved_Put (symtab_p, 0);
	if (local)
	  {
	  SymTab_Flags_LocalSelect_Put (symtab_p, TRUE);
	  }
	else
	  {
	  SymTab_Flags_GlobalSelect_Put (symtab_p, TRUE);
	  }
 
	/*  Do a PostSelect Update of the PST if necessary.  Temporarily
	    set current and developed to first compound.
	*/
	if (nextbest)
	  {
	  *update_time = Syn_Time_Get (FALSE);
	  PstCB_CurrentComp_Put (&SPstcb, SymTab_FirstComp_Get (symtab_p));
	  SymTab_DevelopedComp_Put (symtab_p, SymTab_FirstComp_Get (symtab_p));
	  Pst_PostSel_Update (and_par);
	  PstCB_CurrentComp_Put (&SPstcb, NULL);
	  SymTab_DevelopedComp_Put (symtab_p, NULL);
	  Time_Add (update_time, Syn_Time_Get (TRUE));
	  }

	return (FALSE);
	}
      }

    else
      symtab_p = SymTab_HashChain_Get (symtab_p);
    }

  newsymtab_p = Pst_SymTab_Create ();
  SymTab_HashChain_Put (last_p, newsymtab_p);
  SymTab_InstancesCnt_Put (newsymtab_p, 0);
  SymTab_Sling_Put (newsymtab_p, Sling_Copy (sling)); 
  SymTab_Flags_Selected_Put (newsymtab_p, TRUE);
  SymTab_Flags_WasSelected_Put (newsymtab_p, TRUE);
  SymTab_Merit_Main_Put (newsymtab_p, 0);
  SymTab_Merit_Solved_Put (newsymtab_p, 0);
  SymTab_WorkerId_Put (newsymtab_p, worker);
  if (local)
    {
    SymTab_Flags_LocalSelect_Put (newsymtab_p, TRUE);
    }
  else
    {
    SymTab_Flags_GlobalSelect_Put (newsymtab_p, TRUE);
    }

  return (FALSE);
}
/* End of Pst_Symbol_Selected */

/****************************************************************************
*
*  Function Name:             SymbolTable_Clear
*
*    This routine cleans up the table of symbols by deallocating the 
*    memory for each symbol table entry unless the symbol had been selected
*    for expansion, either by this worker or some other worker (or master);
*    in which case, reinitialize the fields in the symbol table entry. 
*
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
*    Dellocates memory
*
******************************************************************************/
void SymbolTable_Clear
  (
  void
  )
{
  SymTab_t     *back_p; 
  SymTab_t     *symtab_p;
  U16_t         tab_i;

  PstCB_SymtabIndex_Put (&SPstcb, 1);
  for (tab_i = 0; tab_i < SymTab_HashSize_Get (); ++tab_i)
    {
    back_p = NULL;
    symtab_p = SymTab_HashBucketHead_Get (tab_i);
    while (symtab_p != NULL)
      {
      if (SymTab_Flags_WasSelected_Get (symtab_p))
	{
	SymTab_AvailCompKey_Put (symtab_p, AVC_KEY_NOT_FOUND);
	SymTab_Flags_Put (symtab_p, 0);
	SymTab_Flags_Open_Put (symtab_p, TRUE);
	SymTab_Flags_Selected_Put (symtab_p, TRUE);
	SymTab_Flags_WasSelected_Put (symtab_p, TRUE);
	SymTab_Cycle_First_Put (symtab_p, 0);
	SymTab_Cycle_Last_Put (symtab_p, 0);
	SymTab_Cycle_Time_Put (symtab_p, TIME_ZERO);
	SymTab_Current_Put (symtab_p, NULL);
	SymTab_DevelopedComp_Put (symtab_p, NULL);
	SymTab_FirstComp_Put (symtab_p, NULL);
	SymTab_InstancesCnt_Put (symtab_p, 0);
	SymTab_Merit_Main_Put (symtab_p, 0);
	SymTab_Merit_Solved_Put (symtab_p, 0);
	SymTab_NumSols_Put (symtab_p, 0);

	SymTab_Index_Put (symtab_p, PstCB_SymtabIndex_Get (&SPstcb));
	PstCB_SymtabIndex_Put (&SPstcb, PstCB_SymtabIndex_Get (&SPstcb) + 1);

	back_p = symtab_p;
	symtab_p = SymTab_HashChain_Get (symtab_p);
	}

      else
	{
	if (back_p == NULL)
	  {
	  back_p = symtab_p;
	  symtab_p = SymTab_HashChain_Get (symtab_p);
	  SymTab_HashBucketHead_Put (tab_i, symtab_p);
	  Sling_Destroy (SymTab_Sling_Get (back_p));
#ifdef _MIND_MEM_
	  mind_free ("back_p", "pst", back_p);
#else
	  Mem_Dealloc (back_p, SYMTABSIZE, GLOBAL);
#endif
	  back_p = NULL;
	  }

	else
	  {
	  SymTab_HashChain_Put (back_p, SymTab_HashChain_Get (symtab_p));
	  Sling_Destroy (SymTab_Sling_Get (symtab_p));
#ifdef _MIND_MEM_
	  mind_free ("symtab_p", "pst", symtab_p);
#else
	  Mem_Dealloc (symtab_p, SYMTABSIZE, GLOBAL);
#endif
	  symtab_p = SymTab_HashChain_Get (back_p);
	  }
	}
      }
    }

  return;
}
/* End of SymbolTable_Clear */


/* End of PST.C */
