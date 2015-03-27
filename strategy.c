/******************************************************************************
*
*  Copyright (C) 1993-1997 Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     STRATEGY.C
*
*    This module is the abstraction for the search strategy.  It contains
*    routines for the bookkeeping involved in organizing the heuristics 
*    used for the strategy.  The idea is that a set of routines can be
*    implemented for *each* strategy option that SYNCHEM wants to offer
*    and then at runtime they are vectored through a jump table or else
*    the strategy is picked at compile time.
*
*    Currently SYNCHEM offers a strategy called Effort Distribution which
*    seems to work very well.  There are little side features that are
*    *not* currently offered but that did exist in the PL/I version in
*    order to be able to closely emulate LHASA; such as: auto-protection,
*    look-ahead and strategic bonds.  The latter has support embedded in
*    the subgoal generation module.  A potentially useful strategy to add
*    if it can be properly formulated is IDA* which has nice properties
*    for termination, is well studied (it has a theoretical framework which
*    effort distribution does not), and there is a fair bit of knowledge on
*    how to parallelize it.
*
*  Routines:
*
*    Strategy_ClosedCnt_Set
*    Strategy_Dump
*    Strategy_Init
*    Strategy_NextCompound_Select
*    Strategy_NextSubgoal_Select
*    Strategy_SearchEffort_Calculate
*    Strategy_SearchTimes_Backup
*    Strategy_Select_Next
*    Strategy_Subg_ExceedEffort
*
*  Creation Date:
*
*    01-Jan-1993
*
*  Authors:
*
*    Daren Krebsbach
*    Shu Cheung
*    Tito Autrey (translated from others PLI code)
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 09-Jul-97  Krebsbach  Added subtree root for selection.
* 09-Jul-97  Krebsbach  Removed inertia and max visits.
* 22-May-95  Cheung	Added routines Strategy_ClosedCnt_Set,
*    			Strategy_Dump,
*			Strategy_NextCompound_Select,
*			Strategy_NextSubgoal_Select,
*			Strategy_SearchEffort_Calculate,
*			Strategy_SearchTimes_Backup,
*			Strategy_Select_Next,
*			Strategy_Subg_ExceedEffort
* 02-Sep-95 Cheung      num_tcl is passed as an argument in routine 
*			Strategy_Init
*
******************************************************************************/
#include <stdlib.h>

#include "synchem.h"
#include "synio.h"
#include "debug.h"

#ifndef _H_ARRAY_
#include "array.h"
#endif

#ifndef _H_TIMER_
#include "timer.h"
#endif

#ifndef _H_PST_
#include "pst.h"
#endif

#ifndef _H_STRATEGY_
#include "strategy.h"
#endif

/* static routine prototypes */

static Boolean_t Strategy_NextCompound_Select (Subgoal_t *, U16_t, 
                   Strategy_Params_t *, Boolean_t, Boolean_t, Stack_t *);
static Boolean_t Strategy_NextSubgoal_Select  (Compound_t *, U16_t,
                   Strategy_Params_t *, Boolean_t, Boolean_t, Stack_t *);
static void      Strategy_SearchEffort_Calculate (Array_t , float *);
static Boolean_t Strategy_Subg_ExceedEffort   (Subgoal_t *, float, float);



/****************************************************************************
*
*  Function Name:                 Strategy_ClosedCnt_Set
*
*    This function resets the closed counters of the subgoal nodes in the Pst.
*
*  Used to be:
*
*    ???   
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
void Strategy_ClosedCnt_Set
  (
  PstNode_t	node
  )
{
  PstNode_t	newnode;
  U16_t		closed;

  while (PstNode_Compound_Get (&node) != NULL)
    {
    newnode = Pst_Son_Get (&node);
    Strategy_ClosedCnt_Set (newnode);
    newnode = Pst_Brother_Get (&node);
    if (PstNode_Type_Get (&node) == PST_SUBGOAL)
      {
      closed = Strategy_Subg_ClosedCnt_Get (PstSubg_Strategy_Get (
          PstNode_Subgoal_Get (&node)));
      if (closed > 0)
        if (PstSubg_JustSolved (PstNode_Subgoal_Get (&node)) == FALSE)
          Strategy_Subg_ClosedCnt_Put (PstSubg_Strategy_Get (
               PstNode_Subgoal_Get (&node)), closed - 1);
      }
    node = newnode;
    }
  
  return ;
}

/****************************************************************************
*
*  Function Name:                 Strategy_Dump
*
*    This routine prints a formatted dump of the strategy block.
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
void Strategy_Dump
  (
  Strategy_Params_t *stpa_p,
  FileDsc_t     *filed_p                     /* File to dump to */
  )
{
   FILE          *f;                          /* Temporary */
  
  f = IO_FileHandle_Get (filed_p);
  if (stpa_p == NULL)
    {
    fprintf (f, "\nAttempted to dump a NULL strategy parameter block\n");
    return;
    }

  fprintf (f, "%-16s  %hu\n", "  NTCL:", 
    Strategy_Params_MinClosed_Get (stpa_p));

  return; 
}
/****************************************************************************
*
*  Function Name:                 Strategy_Init
*
*    This routine initializes the strategy parameters.     
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
void Strategy_Init
  (
  Strategy_Params_t  *strategy_p,             /* Handle of struct to init. */
  U8_t	             num_tcl		
  )
{
  Strategy_Params_MinClosed_Put (strategy_p, num_tcl);

  return;
}

/****************************************************************************
*
*  Function Name:                 Strategy_Select_Next
*
*    This routine scans the Pst and selects the compound node to be expanded in
*    the next level of subgoal generation.
*
*  Used to be:
*
*    select 
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
*    pointer to a compound in Pst 
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Compound_t *Strategy_Select_Next 
  (
  Compound_t	    *compound_p,
  Strategy_Params_t *strategy_p,
  Boolean_t          use_effort,
  Stack_t	    *pathway_p
  )
{
  PstCB_t      *pcb_p;
  SymTab_t     *symtab_p;
  U16_t	        num_subgoals;
  Boolean_t     success;

  pcb_p = Pst_ControlHandle_Get ();

  symtab_p = PstComp_SymbolTable_Get (compound_p);
  SymTab_Current_Put (symtab_p, compound_p);
  if (pathway_p != NULL)
    Stack_PushAdd (pathway_p, compound_p);

  num_subgoals = PstComp_NumSons_Get (compound_p);
  if (num_subgoals == 0)
    {
    if (SymTab_DevelopedComp_Get (symtab_p) == NULL)
      {
      SymTab_DevelopedComp_Put (symtab_p, compound_p);
      PstCB_CurrentComp_Put (pcb_p, compound_p);
      SymTab_Flags_Selected_Put (symtab_p, TRUE); 
      SymTab_Flags_WasSelected_Put (symtab_p, TRUE); 
      }
    else
      PstCB_CurrentComp_Put (pcb_p, NULL);

    return PstCB_CurrentComp_Get (pcb_p);
    }
  
  success = Strategy_NextSubgoal_Select (compound_p, num_subgoals, 
    strategy_p, use_effort, FALSE, pathway_p);
  if (success == FALSE)
    {
    success = Strategy_NextSubgoal_Select (compound_p, num_subgoals, 
      strategy_p, use_effort, TRUE, pathway_p);
    if (success == FALSE)
      {
      PstCB_CurrentComp_Put (pcb_p, NULL);
      return NULL;
      }
    }

  return PstCB_CurrentComp_Get (pcb_p);
} 
  
/****************************************************************************
*
*  Function Name:                Strategy_NextSubgoal_Select 
*
*    This routine selects the best subgoal son of the given compound father.  
*    The selected subgoal must first satisfy a range of conditions (such as
*    non-circularity, etc.), and then Strategy_NextCompound_Select is called
*    to select an appropriate compound son if one exists.  Two co-routines
*    alternative calls recursively until the best pathway is found.  If the
*    subgoal is rejected because it does not lead to a satisfactory path, then
*    the next best subgoal is considered, and the process continues until a
*    pathway is found or all the subgoal sons are examined.  If they all are
*    rejected, then their compound father is also rejected.
*
*  Used to be:
*
*    select_subgoal
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
*    TRUE		if a pathway is found,
*    FLASE		otherwise
*
*  Side Effects:
*
*
******************************************************************************/
static Boolean_t Strategy_NextSubgoal_Select 
  (
  Compound_t	*compound_p,
  U16_t		num_subgoals,
  Strategy_Params_t *strategy_p,
  Boolean_t	use_effort,
  Boolean_t	second_pass,
  Stack_t	*pathway_p
  )
{
  Subgoal_t	*subgoal_p;
  Subgoal_t	*subgoal_temp_p;
  Array_t	order;
  Array_t	merit_table;
  Array_t	instance_table;
  Array_t	conf_table;
  U16_t		i;
  U16_t		j;
  U16_t		num_compounds;
  float 	total_time;
  Boolean_t	swap;
  S16_t		temp;
  float		*effort_p;
  
  total_time = 0.0;
#ifdef _MIND_MEM_
  mind_malloc ("effort_p", "strategy{1}", &effort_p, num_subgoals * sizeof (float));
  mind_Array_1d_Create ("&order", "strategy{1}", &order, num_subgoals, ADDRSIZE);
  mind_Array_1d_Create ("&merit_table", "strategy{1}", &merit_table, num_subgoals, WORDSIZE);
  mind_Array_1d_Create ("&instance_table", "strategy{1}", &instance_table, num_subgoals, WORDSIZE);
  mind_Array_1d_Create ("&conf_table", "strategy{1}", &conf_table, num_subgoals, WORDSIZE);
#else
  effort_p = (float *) calloc (num_subgoals, sizeof (float));
  Array_1d_Create (&order, num_subgoals, ADDRSIZE);
  Array_1d_Create (&merit_table, num_subgoals, WORDSIZE);
  Array_1d_Create (&instance_table, num_subgoals, WORDSIZE);
  Array_1d_Create (&conf_table, num_subgoals, WORDSIZE);
#endif

  subgoal_p = PstComp_Son_Get (compound_p);
  for (i = 0; i < num_subgoals; ++i)
    {
    Array_1dAddr_Put (&order, i, subgoal_p);
    total_time +=  Time_Format (Strategy_Subg_TimeSpent_Get (
	PstSubg_Strategy_Get (subgoal_p)));
    Array_1d16_Put (&merit_table, i, PstSubg_Merit_Main_Get (subgoal_p));
    Array_1d16_Put (&instance_table, i, PstSubg_Instances_Get (subgoal_p));
    Array_1d16_Put (&conf_table, i,
      PstSubg_Reaction_Confidence_Get (subgoal_p));
    subgoal_p = PstSubg_Brother_Get (subgoal_p);
    }

   /*
    Sort the subgoals into decreasing subgoal merits.  The merit 
    and instance tables are reordered along with the order array.
   */
  
  for (i = 1, swap = TRUE; i < num_subgoals && swap == TRUE; ++i)
    for (j = num_subgoals - 1, swap = FALSE; j >= i; --j)
      if ((S16_t) Array_1d16_Get (&merit_table, j) > 
	  (S16_t) Array_1d16_Get (&merit_table, j - 1)
          || 
          ((S16_t) Array_1d16_Get (&merit_table, j) == 
	    (S16_t) Array_1d16_Get (&merit_table, j - 1)
           &&
           (Array_1d16_Get (&instance_table, j) > 
            Array_1d16_Get (&instance_table, j - 1)
            ||
            Array_1d16_Get (&conf_table, j) > 
            Array_1d16_Get (&conf_table, j))))
        {
        swap = TRUE;
        subgoal_temp_p = (Subgoal_t *)Array_1dAddr_Get (&order, j);
        Array_1dAddr_Put (&order, j,  Array_1dAddr_Get (&order, j - 1));
        Array_1dAddr_Put (&order, j - 1, subgoal_temp_p);
        temp = Array_1d16_Get (&merit_table, j);
        Array_1d16_Put (&merit_table, j, 
	   Array_1d16_Get (&merit_table, j - 1)); 
         Array_1d16_Put (&merit_table, j - 1, temp);
        temp = Array_1d16_Get (&instance_table, j);
        Array_1d16_Put (&instance_table, j, 
            Array_1d16_Get (&instance_table, j - 1));  
        Array_1d16_Put (&instance_table, j - 1, temp);
        temp = Array_1d16_Get (&conf_table, j);
        Array_1d16_Put (&conf_table, j, 
          Array_1d16_Get (&conf_table, j - 1));  
        Array_1d16_Put (&conf_table, j - 1, temp);
        }

  /* If using effort distribution, calculate the search effort 
     distribution for the subgoals.
  */
  if (use_effort)
    Strategy_SearchEffort_Calculate (order, effort_p);

  /* 
     Now consider each subgoal in turn (the best one first, then the next best,
     etc., until either a satisfactory path is chosen, or it is determined that 
     none exists
  */

  for (i = 0; i < num_subgoals; ++i)
    {
    subgoal_p = (Subgoal_t *)Array_1dAddr_Get (&order, i);
    if (PstSubg_Closed (subgoal_p) == FALSE && 
        PstSubg_Unsolvable (subgoal_p) == FALSE &&
        PstSubg_Circlar (subgoal_p) == FALSE)
      {
        /* check temp closed, number of visits, and search effort */
      
      if (PstSubg_TempClosed (subgoal_p) == FALSE || second_pass == TRUE)
        {
        if (PstSubg_JustSolved (subgoal_p) == TRUE &&
           Strategy_Subg_ClosedCnt_Get (PstSubg_Strategy_Get (subgoal_p))== 0
           && second_pass == FALSE)
	  Strategy_Subg_ClosedCnt_Put (PstSubg_Strategy_Get (subgoal_p),
	     Strategy_Params_MinClosed_Get (strategy_p));
        else
          {
          Boolean_t       eff_exceeded;

          if (use_effort)
            eff_exceeded = Strategy_Subg_ExceedEffort (subgoal_p, 
              effort_p[i], total_time);
          else
            eff_exceeded = FALSE;

          if (eff_exceeded == FALSE || second_pass == TRUE) 
            {
            num_compounds = PstSubg_NumSons_Get (subgoal_p);
            if (Strategy_NextCompound_Select (subgoal_p, num_compounds,
	        strategy_p, use_effort, second_pass, pathway_p) == TRUE)
              {
              PstSubg_Visits_Put (subgoal_p, 
                PstSubg_Visits_Get (subgoal_p) + 1);
#ifdef _MIND_MEM_
              mind_Array_Destroy ("&order", "strategy", &order);
              mind_Array_Destroy ("&instance_table", "strategy", &instance_table);
              mind_Array_Destroy ("&merit_table", "strategy", &merit_table);
              mind_Array_Destroy ("&conf_table", "strategy", &conf_table);
              mind_free ("effort_p", "strategy", effort_p);
#else
              Array_Destroy (&order);
              Array_Destroy (&instance_table);
              Array_Destroy (&merit_table);
              Array_Destroy (&conf_table);
              free (effort_p);
#endif
              return TRUE;
              }
            }
          }
        }
      }
    }

  /* 
     since none of the subgoals satisfy the above requirements, reject the 
     compound 
  */

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&order", "strategy", &order);
  mind_Array_Destroy ("&instance_table", "strategy", &instance_table);
  mind_Array_Destroy ("&merit_table", "strategy", &merit_table);
  mind_Array_Destroy ("&conf_table", "strategy", &conf_table);
  mind_free ("effort_p", "strategy", effort_p);
#else
  Array_Destroy (&order);
  Array_Destroy (&instance_table);
  Array_Destroy (&merit_table);
  Array_Destroy (&conf_table);
  free (effort_p);
#endif

  return FALSE;
} 

/****************************************************************************
*
*  Function Name:                 Strategy_NextCompound_Select
*
*    This routine selects a compound son of the given subgoal father as follow:
*    If open and unsolved conjuncts exist, then only they are considered in the
*    selection process.  Otherwise, all open conjuncts are allowed.  Among
*    these, the one with the worst compound merit is examined first.  If it is
*    not a leaf node in the Pst, then Strategy_NextSubgoal_Select is called to
*    to select the best of its subgoal sons, and the two co-routines call each
*    other alternately until a pathway is found or the compound is rejected.
*    If it doesn't pass, then the conjunct with the second worst merit is
*    evaluated and the process continues recursively until either a pathway
*    is selected or all the conjuncts fail.  In the second case, the subgoal
*    father is rejected.  During this process, if a compound node under 
*    consideration is a leaf node in the pst, then a pathway has been found.
*    First, however, the subgoal father is checked to see if it is solved, and
*    has never been temporarily closed before in the current run.  If so, then
*    the subgoal node is temporarily closed for a given number of levels of 
*    subgoal generation, and an alternative pathway is searched for.  If the
*    entire tree is searched and no alternative is discovered, then solved 
*    subgoals are not brought into consideration, and the pathway is selected.
*
*  Used to be:
*
*    select_compound 
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
*    TRUE	if valid pathway has been discovered,
*    FALSE      otherwise 
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static Boolean_t Strategy_NextCompound_Select
  (
  Subgoal_t	*subgoal_p,
  U16_t		num_compounds,
  Strategy_Params_t *strategy_p,
  Boolean_t	use_effort,
  Boolean_t	second_pass,
  Stack_t	*pathway_p
  )
{
  U16_t		size;
  Boolean_t	all_solved;
  Boolean_t	first_unsolved;
  Boolean_t	swap;
  Array_t	compound_list;
  Array_t	symtable;
  U16_t		i;
  U16_t		j;
  Compound_t	*compound_p;
  Compound_t	*father_p;
  Compound_t	*compound_temp_p;
  SymTab_t	*symtab_p;
  PstCB_t	*pcb_p;
  U16_t		num_subgoals;


  size = 0;
  all_solved = TRUE;
  first_unsolved = TRUE;
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&compound_list", "strategy{2}", &compound_list, num_compounds, ADDRSIZE);
  mind_Array_1d_Create ("&symtable", "strategy{2}", &symtable, num_compounds, ADDRSIZE);
#else
  Array_1d_Create (&compound_list, num_compounds, ADDRSIZE);
  Array_1d_Create (&symtable, num_compounds, ADDRSIZE);
#endif
  
  compound_p = PstSubg_Son_Get (subgoal_p);
  for (i = 0; i < num_compounds; ++i)
    {
    if (SymTab_Flags_Open_Get (PstComp_SymbolTable_Get (compound_p)) == TRUE)
      {
      if (first_unsolved == TRUE && 
          SymTab_Flags_Solved_Get (PstComp_SymbolTable_Get (compound_p)) == 
	  FALSE)
        {
        all_solved = FALSE;
        first_unsolved = FALSE;
        size = 0;
        }

      if (all_solved == TRUE || 
         SymTab_Flags_Solved_Get (PstComp_SymbolTable_Get (compound_p)) ==FALSE)
        {
        Array_1dAddr_Put (&compound_list, size, compound_p);
        Array_1dAddr_Put (&symtable, size,PstComp_SymbolTable_Get (compound_p));
        ++size;
        }
      }  

    compound_p = PstComp_Brother_Get (compound_p);
    } 

  /*  Now sort the compound_list into increasing compound merits (use
      the solved merits if all of the compounds have been solved).
  */
  if (all_solved)
    {
    for (i = 1, swap = TRUE; i < size && swap == TRUE; ++i)
      for (j = size - 1, swap = FALSE; j >= i; --j)
        if (SymTab_Merit_Solved_Get (
             (SymTab_t *) Array_1dAddr_Get (&symtable, j)) 
            < SymTab_Merit_Solved_Get (
             (SymTab_t *) Array_1dAddr_Get (&symtable, j- 1)))
          {
          swap = TRUE;
          compound_temp_p = (Compound_t *)Array_1dAddr_Get (&compound_list, j);
          Array_1dAddr_Put (&compound_list, j, 
	    Array_1dAddr_Get (&compound_list, j - 1));
          Array_1dAddr_Put (&compound_list, j - 1, compound_temp_p);
          symtab_p = (SymTab_t *)Array_1dAddr_Get (&symtable, j);
          Array_1dAddr_Put (&symtable, j, Array_1dAddr_Get (&symtable, j - 1));  
          Array_1dAddr_Put (&symtable, j - 1, symtab_p);
          }
    }
  else
    {
    for (i = 1, swap = TRUE; i < size && swap == TRUE; ++i)
      for (j = size - 1, swap = FALSE; j >= i; --j)
        if (SymTab_Merit_Main_Get (
             (SymTab_t *)Array_1dAddr_Get (&symtable, j)) 
            < SymTab_Merit_Main_Get (
             (SymTab_t *)Array_1dAddr_Get (&symtable, j- 1)))
          {
          swap = TRUE;
          compound_temp_p = (Compound_t *)Array_1dAddr_Get (&compound_list, j);
          Array_1dAddr_Put (&compound_list, j, 
	     Array_1dAddr_Get (&compound_list, j - 1));
          Array_1dAddr_Put (&compound_list, j - 1, compound_temp_p);
          symtab_p = (SymTab_t *)Array_1dAddr_Get (&symtable, j);
          Array_1dAddr_Put (&symtable, j, Array_1dAddr_Get (&symtable, j - 1));  
          Array_1dAddr_Put (&symtable, j - 1, symtab_p);
          }
    }

  /* for synthetic selection, check compound conjuncts in order of
     increasing merit, i.e., worst first
     for metabolic selection, not implemented yet
  */

  for (i = 0; i < size; ++i)
    {
    compound_p = (Compound_t *)Array_1dAddr_Get (&compound_list, i);
    symtab_p = (SymTab_t *)Array_1dAddr_Get (&symtable, i);
    
    /* place compound on current path */
 
    SymTab_Current_Put (symtab_p, compound_p);
    
    if (SymTab_DevelopedComp_Get (symtab_p) != NULL)
       father_p = SymTab_DevelopedComp_Get (symtab_p);
    else
       father_p = compound_p;

    if (PstComp_Son_Get (father_p) == NULL)   /* leaf node of Pst */
      {
      if (SymTab_Flags_Selected_Get (symtab_p) == FALSE 
          && SymTab_Flags_WasSelected_Get (symtab_p) == FALSE)
        {
        /*  a valid pathway has been discovered */

        pcb_p = Pst_ControlHandle_Get ();
        PstCB_CurrentComp_Put (pcb_p, compound_p);
        SymTab_DevelopedComp_Put (symtab_p, compound_p);
        SymTab_Flags_Selected_Put (symtab_p, TRUE);
        SymTab_Flags_WasSelected_Put (symtab_p, TRUE);
#ifdef _MIND_MEM_
        mind_Array_Destroy ("&compound_list", "strategy", &compound_list);
        mind_Array_Destroy ("&symtable", "strategy", &symtable);
#else
        Array_Destroy (&compound_list);
        Array_Destroy (&symtable);
#endif
        return TRUE;
        }
      }

    else
      {
      num_subgoals = PstComp_NumSons_Get (father_p);
      if (pathway_p != NULL)
        Stack_PushAdd (pathway_p, compound_p);

      if (Strategy_NextSubgoal_Select (father_p, num_subgoals, 
          strategy_p, use_effort, second_pass, pathway_p) == TRUE)
        {
#ifdef _MIND_MEM_
        mind_Array_Destroy ("&compound_list", "strategy", &compound_list);
        mind_Array_Destroy ("&symtable", "strategy", &symtable);
#else
        Array_Destroy (&compound_list);
        Array_Destroy (&symtable);
#endif
        return TRUE;
        }

      else if (pathway_p != NULL)
        {
        Stack_Pop (pathway_p);
        }
      }

    /* go back and choose the next worst compound */

    SymTab_Current_Put (symtab_p, NULL);
    }

  /* At this point, since all of the compound conjuncts were unsatisfactory, 
     fail the father
  */

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&compound_list", "strategy", &compound_list);
  mind_Array_Destroy ("&symtable", "strategy", &symtable);
#else
  Array_Destroy (&compound_list);
  Array_Destroy (&symtable);
#endif
  return FALSE;
}
/******************************************************************************
*
*  Functionn Name:                Strategy_SearchEffort_Calculate 
*
*     This routine calculates the search effort percentage for each subgoal 
*     given the input parameter which is an array of subgoals in decreasing 
*     merit order. 
*
*  Used to be:
*
*    effdist
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
******************************************************************************/
static void Strategy_SearchEffort_Calculate
  (
  Array_t	subgoals,
  float		*effort
  )
{
  U16_t		num_subgoals;
  U16_t		i;
  U16_t		j;
  float		tree_depth;
  float		depth_factor;
  float		remaining_effort;
  float		percentage;
  float		next_lower_merit;
  float		merit_ratio;
  Subgoal_t	*subgoal_p;
  Array_t	merit;
  Array_t	ok;

  num_subgoals = (U16_t) Array_Columns_Get (&subgoals);
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&ok", "strategy{3}", &ok, num_subgoals, BITSIZE);
  mind_Array_1d_Create ("&merit", "strategy{3}", &merit, num_subgoals, WORDSIZE);
#else
  Array_1d_Create (&ok, num_subgoals, BITSIZE);
  Array_1d_Create (&merit, num_subgoals, WORDSIZE);
#endif
  for (i = 0; i < num_subgoals; ++i)
    {
    subgoal_p = (Subgoal_t *)Array_1dAddr_Get (&subgoals, i);
    Array_1d16_Put (&merit, i, PstSubg_Merit_Main_Get (subgoal_p));  
    Array_1d1_Put (&ok, i, (PstSubg_TempClosed (subgoal_p) == FALSE &&
        PstSubg_Closed (subgoal_p) == FALSE &&
        PstSubg_Unsolvable (subgoal_p) == FALSE &&
        PstSubg_Circlar (subgoal_p) == FALSE &&
        !(PstSubg_JustSolved (subgoal_p) == TRUE &&
        Strategy_Subg_ClosedCnt_Get (PstSubg_Strategy_Get (subgoal_p)) == 0)));
    }

  tree_depth = (float) PstSubg_Level_Get ((Subgoal_t *)
	Array_1dAddr_Get (&subgoals, 0));
  depth_factor = MAX (((tree_depth / 2.0) + 0.5), 2.0);
  remaining_effort = 1.0;
  for (i = 0; i < num_subgoals - 1; ++i)
    {
    if (Array_1d1_Get (&ok, i) == FALSE)
      effort[i] = 0;
    else 
      if (Array_1d16_Get (&merit, i) == 0)
        effort[i] = remaining_effort;
      else
        {
        j = i + 1;
        while (j < num_subgoals && Array_1d1_Get (&ok, j) == FALSE)
          ++j;
      
        if (j < num_subgoals)
          next_lower_merit = (S16_t)Array_1d16_Get (&merit, j);
        else
          next_lower_merit = 0;

        merit_ratio = next_lower_merit / (S16_t)Array_1d16_Get (&merit, i);
        percentage = 1.0 - (merit_ratio / depth_factor);
        effort[i] = percentage * remaining_effort;
        }
    remaining_effort = MAX (remaining_effort - effort[i], 0);
    }

  if (Array_1d1_Get (&ok, num_subgoals - 1) == TRUE)
    effort[num_subgoals - 1] = remaining_effort;
  else
    effort[num_subgoals - 1] = 0;

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&ok", "strategy", &ok);
  mind_Array_Destroy ("&merit", "strategy", &merit);
#else
  Array_Destroy (&ok);
  Array_Destroy (&merit);
#endif
  return;
}
/****************************************************************************
*
*  Function Name:                 Strategy_Subg_ExceedEffort
*
*    This routine decide if the time spent on the subgoal exceeds the effort
*    percentage given the subgoal by Strategy_SearchEffort_Calculate.
*
*  Used to be:
*
*    effexcd
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
*    TRUE	 if exceeds
*    FALSE	 otherwise
*
*  Side Effects:
*
*    N/A
*
*****************************************************************************/
static Boolean_t Strategy_Subg_ExceedEffort 
  (
  Subgoal_t	*subgoal_p,
  float		effort,
  float		total_time
  )
{
  float		time_spent;

  if (total_time > 0 && PstSubg_Visits_Get (subgoal_p) >= 2)
    {
    time_spent = Time_Format (Strategy_Subg_TimeSpent_Get (
       PstSubg_Strategy_Get (subgoal_p)));
    if ((time_spent / total_time) > effort)
      return TRUE;
    }

  return FALSE;
}
/****************************************************************************
*
*  Function Name:                 Strategy_SearchTimes_Backup
*
*     This routine is used to back the search times up the PST from the
*     subgoal just expanded to all its ancestor subgoals.  The cycle
*     time is added to the subgoal just expanded and its ancestor subgoals
*     but NOT the subgoals just generated.  If the current pathway is not 
*     explicitly given (as in the distributed synchem), use the current 
*     compound pointers in the symbol table entries to determine the current
*     pathway.
*
*  Used to be:
*
*    effback
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
*****************************************************************************/
void Strategy_SearchTimes_Backup
  (
  Compound_t   *cur_compound_p,
  Time_t        cycle_time,
  Stack_t      *pathway_p
  )
{
  Compound_t   *compound_p;
  Subgoal_t    *subgoal_p;
  SymTab_t     *symtab_p;
  Time_t        new_time;

  if (pathway_p == NULL)
    {
    SymTab_Current_Put (PstComp_SymbolTable_Get (cur_compound_p), NULL);
    }

  subgoal_p = PstComp_Father_Get (cur_compound_p);
  while (subgoal_p != PstCB_Root_Get (Pst_ControlHandle_Get ()))
    {
    new_time = Strategy_Subg_TimeSpent_Get (PstSubg_Strategy_Get (subgoal_p));
    Time_Add (&new_time, cycle_time); 
    Strategy_Subg_TimeSpent_Put (PstSubg_Strategy_Get (subgoal_p), new_time);

  if (pathway_p == NULL)
    {
    compound_p = PstSubg_Father_Get (subgoal_p);
    symtab_p = PstComp_SymbolTable_Get (compound_p);

#ifndef FORCED_SELECTION
    compound_p = SymTab_Current_Get (symtab_p);
#endif

    SymTab_Current_Put (symtab_p, NULL);
    }

  else
    {
    compound_p = (Compound_t *) Stack_TopAdd (pathway_p);
    Stack_Pop_Save (pathway_p);
    }

    subgoal_p = PstComp_Father_Get (compound_p);
    }

  return;
}
       
