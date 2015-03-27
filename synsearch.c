/******************************************************************************
*
*  Copyright (C) 1996 Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:          SYN_SEARCH.C
*
*    This module contains the SYNCHEM synthesis search algorithm.
*
*    For further comments, see synchem3.c.
*
*  Routines:
*
*    Synchem_Search
*
*
*  Creation Date:
*
*       28-Sep-1996
*
*  Authors:
*
*    Daren Krebsbach
*
*  Modification History, reverse chronological
*
* Date       Author     Modifcation Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Lolita     xxx
*
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

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

#ifndef _H_SLING2XTR_
#include "sling2xtr.h"
#endif

#ifndef _H_PST_
#include "pst.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_STRATEGY_
#include "strategy.h"
#endif

#ifndef _H_SEARCH_COVER_
#include "search_cover.h"
#endif

#ifndef _H_RCB_
#include "rcb.h"
#endif

#ifndef _H_STATUS_
#include "status.h"
#endif

#ifndef _H_SYN_SEARCH_
#include "synsearch.h"
#endif

#ifdef FORCED_SELECTION

static Compound_t *SSelect_Next (U32_t);

static  Sling_t      *SSelect_Slings;
static  U32_t        *SSelect_Indices;

#endif


/****************************************************************************
*
*  Function Name:                 Synchem_Search
*
*    This is the SYNCHEM executive.  It starts, runs, and cleans up the
*    whole search.
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
*    Produces a synthesis in the trace file
*    May call IO_Exit_Error
*
******************************************************************************/
void NonGUI_Synchem_Search
  (
  Compound_t      *subtreeroot_compound_p,
  Rcb_t           *rcb_p,
  RunStats_t      *rstat_p
  )
{
  Array_t        stereo_options;             /* Array descriptor */
  Array_t        strategic_bonds;            /* Array descriptor */
  Timer_t        cur_timers;                 /* Current cycle timer */
  Xtr_t         *xtr_p;                      /* Address of current goal XTR */
  Compound_t    *cur_compound_p;             /* Temp. Current compound */
  SymTab_t      *goal_symtab_p;
  U32_t          save_maxcycles;             /* Save the max cycles */  
  U32_t          schema;                     /* Index of schema to work on */
  U16_t          num_rings;
  U16_t          ring_i;                     /* Counter - ring definition */
  Boolean_t      accept;                     /* Passed pre-transform tests */
  Boolean_t      end_search;

#ifdef FORCED_SELECTION
  {
  char       *buf_p;
  char       *selfilename;
  FILE       *self_p;
  U32_t       sel_i;
  char        buff[MX_INPUT_SIZE];

  SSelect_Slings = (Sling_t *) malloc (SLINGSIZE * Rcb_MaxCycles_Get (rcb_p));
  if (SSelect_Slings == NULL)
    {
    fprintf (stderr, "\nForceSel unable to allocate memory for slings.\n");
    exit (-1);
    }

  SSelect_Indices = (U32_t *) malloc (sizeof(U32_t) 
    * Rcb_MaxCycles_Get (rcb_p));
  if (SSelect_Indices == NULL)
    {
    fprintf (stderr, "\nForceSel unable to allocate memory for indices.\n");
    exit (-1);
    }

  selfilename = (char *) String_Value_Get (FileCB_FileStr_Get (
    Rcb_IthFileCB_Get (rcb_p, FCB_IND_FSEL)));
#ifdef _WIN32
  self_p = fopen (gccfix (selfilename), "r");
#else
  self_p = fopen (selfilename, "r");
#endif
  if (self_p == NULL)
    {
    fprintf (stderr, "\nForceSel unable to open selection file:  %s.\n", 
      selfilename);
    exit (-1);
    }

  for (sel_i = 0; sel_i < Rcb_MaxCycles_Get (rcb_p); sel_i++)
    {
    if (fgets (buff, MX_INPUT_SIZE, self_p) == NULL)
      {
      fprintf (stderr, "\nForceSel:  unable to read file %s\n",
	selfilename);
      exit (-1);
      }

    buf_p = strtok (buff, " ");
    SSelect_Indices[sel_i] = atol (buf_p);
    buf_p = strtok (NULL, "\n");
    SSelect_Slings[sel_i] = Sling_Create (strlen (buf_p));
    memcpy ((char *) Sling_Name_Get (SSelect_Slings[sel_i]), 
      buf_p, Sling_Length_Get (SSelect_Slings[sel_i]) + 1);
    }

  fclose (self_p);
  }
#endif

  if (Rcb_Flags_Continue_Get (rcb_p))
    {
    save_maxcycles = Rcb_MaxCycles_Get (rcb_p);
    Rcb_MaxCycles_Put (rcb_p, RunStats_CumCycles_Get (rstat_p)
      + Rcb_LeapSize_Get (rcb_p));
    }

  else
    {
    goal_symtab_p = PstComp_SymbolTable_Get (PstSubg_Son_Get (PstCB_Root_Get (
      Pst_ControlHandle_Get ())));
    if (!RunStats_Flags_FirstSolFound_Get (rstat_p)
	&& SymTab_Flags_Solved_Get (goal_symtab_p))
      {
      RunStats_Flags_FirstSolFound_Put (rstat_p, TRUE);
      RunStats_CyclesFirstSol_Put (rstat_p, 
	RunStats_CumCycles_Get (rstat_p));
      RunStats_TimeFirstSol_Put (rstat_p, 
	Timer_Cycle_Get (RunStats_CumTimes_Get (rstat_p)));
      }

    if (RunStats_CumCycles_Get (rstat_p) >= Rcb_MaxCycles_Get (rcb_p))
      {
      String_Destroy (RunStats_ExitCond_Get (rstat_p));
      RunStats_ExitCond_Put (rstat_p, 
	String_Create ("Maximum cycles exceeded.", 0));
      return;
      }

    else if (Time_Format (Timer_Cycle_Get (RunStats_CumTimes_Get (rstat_p))) 
	>= Rcb_MaxRuntime_Get (rcb_p) * 60)
      {
      String_Destroy (RunStats_ExitCond_Get (rstat_p));
      RunStats_ExitCond_Put (rstat_p, 
	String_Create ("Maximum runtime exceeded.", 0));
      return;
      }

    else if (Rcb_Flags_FirstSol_Get (rcb_p)
	&& RunStats_Flags_FirstSolFound_Get (rstat_p))
      {
      if (RunStats_CumCycles_Get (rstat_p) 
	  >= RunStats_CyclesFirstSol_Get (rstat_p)
	    + Rcb_AddCycles_Get (rcb_p))
	{
	String_Destroy (RunStats_ExitCond_Get (rstat_p));
	RunStats_ExitCond_Put (rstat_p, String_Create (
	  "Additional cycles exceeded after first solution was found.", 0));
	return;
	}

      if (Time_Format (Timer_Cycle_Get (RunStats_CumTimes_Get (rstat_p))) 
	  >= (Time_Format (RunStats_TimeFirstSol_Get (rstat_p))
	  + Rcb_AddTime_Get (rcb_p) * 60))
	{
	String_Destroy (RunStats_ExitCond_Get (rstat_p));
	RunStats_ExitCond_Put (rstat_p, String_Create (
	  "Additional runtime exceeded after first solution was found.", 0));
	return;
	}
      }
    }  /* End of else not continuation */

  Timer_Cycle_Put (&cur_timers, Syn_Time_Get (FALSE), SET);
  cur_compound_p = NULL;
  end_search = FALSE;
  
  Timer_Select_Put (&cur_timers, Syn_Time_Get (FALSE), SET);
  cur_compound_p = Strategy_Select_Next (subtreeroot_compound_p, 
    Rcb_Strategy_Get (rcb_p), Rcb_Flags_EffortDis_Get (rcb_p), NULL);
  SymbolTable_PostSelect_Reset (Rcb_NTCL_Get (rcb_p) > 0, TRUE);
  Timer_Select_Put (&cur_timers, Syn_Time_Get (TRUE), ADD);
  Timer_Select_Put (RunStats_CumTimes_Get (rstat_p), 
    Timer_Select_Get (&cur_timers), ADD);

  if (cur_compound_p == NULL)
      {
      String_Destroy (RunStats_ExitCond_Get (rstat_p));
      RunStats_ExitCond_Put (rstat_p, 
	String_Create ("No compounds to expand (search is stuck).", 0));
      return;
      }
  fprintf (stdout, "\n\nEnd of cycle         ");
  fflush (stdout);

  /* Now comes the main search loop which executes till time runs out.  There
     does not appear to be a convenient manner for exiting when there are no
     more nodes to search look at.
  */

  while (!end_search)
    {
    Timer_Expand_Put (&cur_timers, Syn_Time_Get (FALSE), SET);
    PstCB_NextExpandedComp_Put (cur_compound_p);
    xtr_p = Sling_CanonicalName2Xtr (SymTab_Sling_Get (
      PstComp_SymbolTable_Get (cur_compound_p)));
    Xtr_Attr_ResonantBonds_Set (xtr_p);
    Xtr_Aromat_Set (xtr_p);
    num_rings = Xtr_Rings_NumRingSys_Get (xtr_p);
    for (ring_i = 0; ring_i < num_rings; ring_i++)
      Xtr_Ringdef_Set (xtr_p, ring_i);

    Xtr_Atoms_Set (xtr_p);
    Xtr_FuncGroups_Put (xtr_p, FuncGroups_Create (xtr_p));

    Array_2d_Create (&strategic_bonds, Xtr_NumAtoms_Get (xtr_p),
      MX_NEIGHBORS, BITSIZE);
    Array_1d_Create (&stereo_options, Xtr_NumAtoms_Get (xtr_p),
      BITSIZE);

    /* There is only one reaction library and it has NO chapters so the
       original set of nested loops has been greatly simplified.
       - Eventually, may have decision procedure pick out the reactions
	 from the library and then loop through them.  May even have them
	 ordered in some sensible fashion.
       - Create the temporary arrays for subgoal generation, sort of
	 strategy related (strategic bonds)
       - Initialize them according to the input parameters
       - Generate subgoals
       - Destroy temporary arrays after generating subgoals
    */

    for (schema = 0; schema < React_NumSchemas_Get (); schema++)
      {
      accept = React_Schema_IsOk (xtr_p, schema);
      if (accept == TRUE)
	{
	TRACE (R_MAIN, DB_CHEMISTRY, TL_LOOP, (outbuf,
	  "Schema %lu is now being examined for application", schema));

	TRACE (R_MAIN, DB_CHEMISTRY, TL_DETAIL, (outbuf,
	  "The name of the schema is %s", String_Value_Get (
	  React_TxtRec_Name_Get (React_Text_Get (React_Schema_Handle_Get (
	  schema))))));
	
	if (Rcb_Flags_StrategicBonds_Get (rcb_p) == FALSE)
	  Array_Set (&strategic_bonds, TRUE);
	else
	  IO_Exit_Error (R_MAIN, X_SYNERR, "Strategic bonds not supported");

	Array_Set (&stereo_options, Rcb_Flags_StereoChemistry_Get (rcb_p));
	
	SubGenr_Subgoals_Generate (xtr_p, &strategic_bonds,
	  &stereo_options, schema, Rcb_Flags_StereoChemistry_Get (rcb_p),
	  Rcb_Flags_PreserveStructures_Get (rcb_p), NULL);
	}  /* End of if schema okay */

      else
	{
	TRACE (R_MAIN, DB_CHEMISTRY, TL_LOOP, (outbuf,
	  "Schema %lu rejected", schema));
	}
      }  /* End of for-schema loop */

    Array_Destroy (&strategic_bonds);
    Array_Destroy (&stereo_options);
    Xtr_Destroy (xtr_p);
    xtr_p = NULL;

    Timer_Expand_Put (&cur_timers, Syn_Time_Get (TRUE), ADD);
    Timer_Expand_Put (RunStats_CumTimes_Get (rstat_p), 
      Timer_Expand_Get (&cur_timers), ADD);

    /* Back up time for effort heuristic. */
    Timer_UpdatePST_Put (&cur_timers, Syn_Time_Get (FALSE), SET);
    SymTab_Flags_Selected_Put (PstComp_SymbolTable_Get (cur_compound_p), FALSE);
    Strategy_SearchTimes_Backup (cur_compound_p, 
      Timer_Expand_Get (&cur_timers), NULL);
    Pst_Update ();
    Timer_UpdatePST_Put (&cur_timers, Syn_Time_Get (TRUE), ADD);
    Timer_UpdatePST_Put (RunStats_CumTimes_Get (rstat_p), 
      Timer_UpdatePST_Get (&cur_timers), ADD);

    if (Rcb_Flags_SaveTrace_Get (rcb_p) 
	&& Rcb_Flags_SaveMerits_Get (rcb_p))
      {
      fprintf (IO_FileHandle_Get (&GTraceFile), "Shelved Merit:  %1u\n",
	SymTab_Merit_Main_Get (PstComp_SymbolTable_Get (PstSubg_Son_Get (
	PstCB_Root_Get (Pst_ControlHandle_Get ())))));
      }

    RunStats_CumCycles_Get (rstat_p)++;

    /* Calculate the number of cycles executed so far this run. 
       Need to check if we have done as much work on this problem as
       the user specified in which case it is time to quit.
    */
    if (Rcb_Flags_Continue_Get (rcb_p) 
	&& RunStats_CumCycles_Get (rstat_p) >= Rcb_MaxCycles_Get (rcb_p))
      {
      if (save_maxcycles > Rcb_MaxCycles_Get (rcb_p))
	Rcb_MaxCycles_Put (rcb_p, save_maxcycles);
      if (Time_Format (Timer_Cycle_Get (RunStats_CumTimes_Get (rstat_p))) 
	  >= Rcb_MaxRuntime_Get (rcb_p) * 60)
	Rcb_MaxRuntime_Put (rcb_p, (U32_t) (Time_Format (
	  Timer_Cycle_Get (RunStats_CumTimes_Get (rstat_p))) / 60));
      String_Destroy (RunStats_ExitCond_Get (rstat_p));
      RunStats_ExitCond_Put (rstat_p, 
	String_Create ("Leapsize more cycles completed.", 0));
      end_search = TRUE;
      }

    else
      {
      if (!RunStats_Flags_FirstSolFound_Get (rstat_p)
	  && SymTab_Flags_Solved_Get (goal_symtab_p))
	{
	RunStats_Flags_FirstSolFound_Put (rstat_p, TRUE);
	RunStats_CyclesFirstSol_Put (rstat_p, 
	  RunStats_CumCycles_Get (rstat_p));
	RunStats_TimeFirstSol_Put (rstat_p, 
	  Timer_Cycle_Get (RunStats_CumTimes_Get (rstat_p)));
	}

      if (RunStats_CumCycles_Get (rstat_p) >= Rcb_MaxCycles_Get (rcb_p))
	{
	String_Destroy (RunStats_ExitCond_Get (rstat_p));
	RunStats_ExitCond_Put (rstat_p, 
	  String_Create ("Maximum cycles exceeded.", 0));
	end_search = TRUE;
	}

      else if (Time_Format (Timer_Cycle_Get (RunStats_CumTimes_Get (rstat_p))) 
	  >= Rcb_MaxRuntime_Get (rcb_p) * 60)
	{
	String_Destroy (RunStats_ExitCond_Get (rstat_p));
	RunStats_ExitCond_Put (rstat_p, 
	  String_Create ("Maximum runtime exceeded.", 0));
	end_search = TRUE;
	}

      else if (Rcb_Flags_FirstSol_Get (rcb_p)
	  && RunStats_Flags_FirstSolFound_Get (rstat_p))
	{
	if (RunStats_CumCycles_Get (rstat_p) 
	    >= RunStats_CyclesFirstSol_Get (rstat_p)
	      + Rcb_AddCycles_Get (rcb_p))
	  {
	  String_Destroy (RunStats_ExitCond_Get (rstat_p));
	  RunStats_ExitCond_Put (rstat_p, String_Create (
	    "Additional cycles exceeded after first solution was found.", 
	    0));
	  end_search = TRUE;
	  }

	else if (Time_Format (Timer_Cycle_Get (RunStats_CumTimes_Get (rstat_p)))
	    >= (Time_Format (RunStats_TimeFirstSol_Get (rstat_p))
	      + Rcb_AddTime_Get (rcb_p) * 60))
	  {
	  String_Destroy (RunStats_ExitCond_Get (rstat_p));
	  RunStats_ExitCond_Put (rstat_p, String_Create (
	    "Additional runtime exceeded after first solution was found.", 
	    0));
	  end_search = TRUE;
	  }
	}

      else if (Rcb_Flags_SearchCoverDev_Get (rcb_p) 
	  || Rcb_Flags_SearchCoverSlv_Get (rcb_p))
	{
	U8_t            coverage;

	coverage = SearchSpace_Cover (SymTab_Sling_Get (
	  PstComp_SymbolTable_Get (cur_compound_p)));

	if (!RunStats_Flags_SolSPCovered_Get (rstat_p)
	    && (coverage & COVER_SOL_COVERED))
	  {
	  RunStats_Flags_SolSPCovered_Put (rstat_p, TRUE);
	  TRACE (R_NONE, DB_CHEMISTRY, TL_RUNSTATS, (outbuf,
	    "\nSolved Pathways Covered in cycle #%lu."
	    "\nSolved Pathways Covered in %f seconds.\n",  
	    RunStats_CumCycles_Get (rstat_p),
	    Time_Format (Timer_Cycle_Get (RunStats_CumTimes_Get (rstat_p)))));
	  }

	if (!RunStats_Flags_DevSPCovered_Get (rstat_p)
	    && (coverage & COVER_DEV_COVERED))
	  {
	  RunStats_Flags_DevSPCovered_Put (rstat_p, TRUE);
	  TRACE (R_NONE, DB_CHEMISTRY, TL_RUNSTATS, (outbuf,
	    "\nDeveloped Search Space Covered in cycle #%lu."
	    "\nDeveloped Search Space Covered in %f seconds.\n",  
	    RunStats_CumCycles_Get (rstat_p),
	    Time_Format (Timer_Cycle_Get (RunStats_CumTimes_Get (rstat_p)))));
	  }

	if ((!Rcb_Flags_SearchCoverDev_Get (rcb_p)
	      || RunStats_Flags_DevSPCovered_Get (rstat_p))
	    && (!Rcb_Flags_SearchCoverSlv_Get (rcb_p)
	      || RunStats_Flags_SolSPCovered_Get (rstat_p)))
	  {
	  String_Destroy (RunStats_ExitCond_Get (rstat_p));
	  RunStats_ExitCond_Put (rstat_p, String_Create (
	    "Developed search space and/or solved pathways covered.", 0));
	  end_search = TRUE;
	  }
	}
      }  /* End of else not continuation */


    Timer_Cycle_Put (&cur_timers, Syn_Time_Get (TRUE), ADD);
    Timer_Cycle_Put (RunStats_CumTimes_Get (rstat_p), 
      Timer_Cycle_Get (&cur_timers), ADD);

    SymTab_Cycle_Number_Put (PstComp_SymbolTable_Get (cur_compound_p), 
       (U16_t) RunStats_CumCycles_Get (rstat_p));
    SymTab_Cycle_Time_Put (PstComp_SymbolTable_Get (cur_compound_p),
       Timer_Cycle_Get (&cur_timers));

    fprintf (stdout, "\b\b\b\b\b\b\b%6lu ", RunStats_CumCycles_Get (rstat_p));
    fflush (stdout);

    TRACE (R_NONE, DB_CHEMISTRY, TL_SELECT, (outbuf, 
      "Cyc: %5lu  Sels: %6lu  Selc: %6lu  Tim: %10.4f  CumT: %10.4f",
      PstCB_TotalExpandedCompounds_Get (Pst_ControlHandle_Get ()),
      SymTab_Index_Get (PstComp_SymbolTable_Get (cur_compound_p)),
      PstComp_Index_Get (cur_compound_p),
      Time_Format (Timer_Cycle_Get (&cur_timers)), 
      Time_Format (Timer_Cycle_Get (RunStats_CumTimes_Get (rstat_p)))));
 
    if (!end_search)
      {
      /*  Starting clock for time spent on next cycle.  */
      Timer_Cycle_Put (&cur_timers, Syn_Time_Get (FALSE), SET);
      Timer_Select_Put (&cur_timers, Syn_Time_Get (FALSE), SET);

#ifdef FORCED_SELECTION
      cur_compound_p = SSelect_Next (PstCB_TotalExpandedCompounds_Get (
	Pst_ControlHandle_Get ()));
      SymbolTable_PostSelect_Reset (Rcb_NTCL_Get (rcb_p) > 0, TRUE);
#else
      cur_compound_p = Strategy_Select_Next (subtreeroot_compound_p, 
	Rcb_Strategy_Get (rcb_p), Rcb_Flags_EffortDis_Get (rcb_p), NULL);
      SymbolTable_PostSelect_Reset (Rcb_NTCL_Get (rcb_p) > 0, TRUE);
#endif

      Timer_Select_Put (&cur_timers, Syn_Time_Get (TRUE), ADD);
      Timer_Select_Put (RunStats_CumTimes_Get (rstat_p), 
	Timer_Select_Get (&cur_timers), ADD);
      if (cur_compound_p == NULL)
	{
	String_Destroy (RunStats_ExitCond_Get (rstat_p));
	RunStats_ExitCond_Put (rstat_p, 
	  String_Create ("No compounds to expand (search is stuck).", 0));
	end_search = TRUE;
	}
      }

  }  /* End of while-loop, not end of search */

  printf ("\n%s\n\n", 
    (char *) String_Value_Get (RunStats_ExitCond_Get (rstat_p)));

  RunStats_NumCompounds_Put (rstat_p, 
    PstCB_CompoundIndex_Get (Pst_ControlHandle_Get ()));
  RunStats_NumSubgoals_Put (rstat_p, 
    PstCB_SubgoalIndex_Get (Pst_ControlHandle_Get ()));
  RunStats_NumSymbols_Put (rstat_p, 
    PstCB_SymtabIndex_Get (Pst_ControlHandle_Get ()));

  return;
}
/* End of Synchem_Search */

#ifdef FORCED_SELECTION

static Compound_t *SSelect_Next
  (
  U32_t         num
  )
{
  Sling_t        sling;
  Compound_t    *cmp_p;
  Compound_t    *closest_cmp_p;
  SymTab_t      *symtab_p; 
  U32_t          hash;
  U32_t          index;
  U32_t          diff;
  U32_t          closest_diff;

   sling = SSelect_Slings[num];
   index = SSelect_Indices[num];
   hash = Pst_Hash (sling, MX_SYMTAB_BUCKETS);
   symtab_p = SymTab_HashBucketHead_Get (hash); 

  if (symtab_p == NULL) 
    {
    fprintf (stderr, "\nSSelect_Next:  empty hash bucket for %s.\n",
      (char *) Sling_Name_Get (sling));
    return (NULL);
    }

  while (symtab_p != NULL && (strcasecmp ((char *) Sling_Name_Get (sling), 
	 (char *) Sling_Name_Get (SymTab_Sling_Get (symtab_p)))) != 0)
    symtab_p = SymTab_HashChain_Get (symtab_p);

  if (symtab_p == NULL) 
    {
    fprintf (stderr, "\nSSelect_Next:  unable to find sling %s.\n",
      (char *) Sling_Name_Get (sling));
    return (NULL);
    }

  cmp_p = SymTab_FirstComp_Get (symtab_p);
  if (cmp_p == NULL) 
    {
    fprintf (stderr, "\nSSelect_Next:  NULL compound node for %s\n",
      (char *) Sling_Name_Get (sling));
    return (NULL);
    }

  /*  Among the compound instances of this symbol, find the one with
      the closest compound node index of the distributed run.
  */
  closest_cmp_p = cmp_p;
  closest_diff = 100000;
  while (cmp_p != NULL)
    {
    diff = (index > PstComp_Index_Get (cmp_p)) 
      ? index - PstComp_Index_Get (cmp_p)
      : PstComp_Index_Get (cmp_p) - index;

    if (diff < closest_diff)
      {
      closest_diff = diff;
      closest_cmp_p = cmp_p;
      }

    cmp_p = PstComp_Prev_Get (cmp_p);
    }

  SymTab_DevelopedComp_Put (symtab_p, closest_cmp_p);
  PstCB_CurrentComp_Put (Pst_ControlHandle_Get (), closest_cmp_p);

  return closest_cmp_p; 
}

#endif

/* End of SYN_SEARCH.C */
