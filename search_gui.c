/******************************************************************************
*
*  Copyright (C) 1996 Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:	         SYN_SEARCH.C
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
*	28-Sep-1996
*
*  Authors:
*
*    Daren Krebsbach
*
*  Modification History, reverse chronological
*
* Date       Author	Modifcation Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Lolita	xxx
*
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <Xm/MessageB.h>

#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif

#include "synchem.h"
#include "debug.h"
#include "synio.h"

/* NEED TO ADD APP_RESRC TO CAPTURE XtManageChild and XtUnmanageChild */
#ifndef _H_APP_RESRC_
#include "app_resrc.h"
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

#ifndef _H_SLING2XTR_
#include "sling2xtr.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_PST_
#include "pst.h"
#endif

#ifndef _H_STRATEGY_
#include "strategy.h"
#endif

#ifndef _H_RCB_
#include "rcb.h"
#endif

#ifndef _H_STATUS_
#include "status.h"
#endif

#ifndef _H_SUBMIT_SAVEPLAN_
#include "submit_saveplan.h"
#endif

#ifndef _H_SEARCH_GUI_
#include "search_gui.h"
#endif

#ifndef _H_PERSIST_
#include "persist.h"
#endif

#ifndef _H_PST_VIEW_
#include "pst_view.h"
#endif

#ifndef _H_EXTERN_
#include "extern.h"
#endif

#ifdef _MIND_MEM_
Boolean_t in_cycles = FALSE;
#endif

//#define _DEBUG_ //kka

char *Sensible_Time_Format (Time_t, Boolean_t);
void Status_Pst_Serialize   (Array_t *, Array_t *, PstNode_t);
PstView_t *PstView_Handle_Grab ();
void PstView_SelInRow (PstView_t *, U8_t, U32_t);
Boolean_t Halt_File_Present ();

void SS_WalkPath (U32_t cmpinx, String_t filename)
{
  char int_filename[128], *dotpath;
  FILE *path;
  U32_t nnodes, nodes[100];
  int i;
  PstView_t *pv_p;

  pv_p = PstView_Handle_Grab ();

  strcpy (int_filename, String_Value_Get (filename));
  if (strstr (int_filename, "_interactive") == NULL)
  {
    dotpath = strstr (int_filename, ".path");
    strcpy (dotpath, "_interactive.path");
  }
  path = fopen (int_filename, "r");
  for (nnodes = 0; fscanf (path, "%d", nodes + nnodes) == 1; nnodes++);
  fclose (path);
  for (i = nnodes - 1; i >= 0; i--)
    PstView_SelInRow (pv_p, i == nnodes - 1 ? PVW_LEVEL_TOP : i == nnodes - 2 ? PVW_LEVEL_MID : PVW_LEVEL_BTM, nodes[i]);
}


/****************************************************************************
*
*  Function Name:                 Synchem_Search
*
*    This is the GUI version of the SYNCHEM search engine.  The only
*    difference between the GUI version and the command-line version
*    is that this version prints the cycle number to an information
*    window rather than stdout.
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
void Synchem_Search
  (
  Compound_t      *subroot_p,
  Rcb_t           *rcb_p,
  RunStats_t      *rstat_p, 
  Widget           cycle_mess,
  Widget           top_app
  )
{
  Array_t        stereo_options;             /* Array descriptor */
  Array_t        strategic_bonds;            /* Array descriptor */
  char           buffer[GSS_CYCLE_BUFLEN];   /* To store cycle count */
  XmString       mess_str;                   /* Message string */
  Timer_t        cur_timers;                 /* Current cycle timer */
  Compound_t    *cur_compound_p;             /* Temp. Current compound */
  SymTab_t      *goal_symtab_p;
  Xtr_t         *xtr_p;                      /* Address of current goal XTR */
  U32_t          save_maxcycles;             /* Save the max cycles */  
  U32_t          schema, schema_inx;         /* Index of schema to work on */
  U16_t		 num_rings;
  U16_t          ring_i;                     /* Counter - ring definition */
  Boolean_t      accept;                     /* Passed pre-transform tests */
  Boolean_t      end_search;
  Boolean_t is_temp;
SymTab_t      *symtab_p;
PstCB_t       *pcb_p;
U32_t cmpinx, cycle_num;
Boolean_t first_solved_this_cycle, already_solved;
printf("kka:entering Synchem_Search\n");
  first_solved_this_cycle = already_solved = FALSE;

  if (!XtIsManaged (cycle_mess))
    {
    Display     *dsp = XtDisplay (top_app);
    Widget       halt_pb;

    mess_str = XmStringCreateLtoR ("Search started ...", 
      XmFONTLIST_DEFAULT_TAG);
    XtVaSetValues (cycle_mess, XmNmessageString, mess_str, NULL);
    XmStringFree (mess_str);

    XtVaGetValues (cycle_mess,
      XmNcancelButton, &halt_pb,
      NULL);

    if (!XtIsManaged (halt_pb))
      XtManageChild (halt_pb);

    XtManageChild (cycle_mess);
    XFlush (dsp);
    XmUpdateDisplay (top_app);
    }

  if (Halt_File_Present ()); /* Don't let an existing halt file stop run before it starts! */
  if (RunStats_Flags_SearchHalted_Get (rstat_p))
    {
    String_Destroy (RunStats_ExitCond_Get (rstat_p));
    RunStats_ExitCond_Put (rstat_p, 
      String_Create ("Search halted by user.", 0));
    RunStats_Flags_SearchHalted_Put (rstat_p, FALSE);
    if (XtIsManaged (cycle_mess))
      {
      Widget        halt_pb;

      mess_str = XmStringCreateLtoR ((char *) String_Value_Get (
        RunStats_ExitCond_Get (rstat_p)), XmFONTLIST_DEFAULT_TAG);
      XtVaSetValues (cycle_mess, XmNmessageString, mess_str, NULL);
      XmStringFree (mess_str);
      XtVaGetValues (cycle_mess,
        XmNcancelButton, &halt_pb,
        NULL);
      XtUnmanageChild (halt_pb);
      }
    return;
    }

  if (Rcb_Flags_Continue_Get (rcb_p))
    {
    if (Rcb_LeapSize_Get (rcb_p) == 0)
      {
      String_Destroy (RunStats_ExitCond_Get (rstat_p));
      RunStats_ExitCond_Put (rstat_p, 
        String_Create ("Leapsize more cycles executed.", 0));
      if (XtIsManaged (cycle_mess))
        {
        Widget        halt_pb;

        mess_str = XmStringCreateLtoR ((char *) String_Value_Get (
          RunStats_ExitCond_Get (rstat_p)), XmFONTLIST_DEFAULT_TAG);
        XtVaSetValues (cycle_mess, XmNmessageString, mess_str, NULL);
        XmStringFree (mess_str);
        XtVaGetValues (cycle_mess,
          XmNcancelButton, &halt_pb,
          NULL);
        XtUnmanageChild (halt_pb);
        }
      return;
      }
    else
      {
      save_maxcycles = Rcb_MaxCycles_Get (rcb_p);
      Rcb_MaxCycles_Put (rcb_p, RunStats_CumCycles_Get (rstat_p)
        + Rcb_LeapSize_Get (rcb_p));
      }
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
      if (XtIsManaged (cycle_mess))
        {
        Widget        halt_pb;

        mess_str = XmStringCreateLtoR ((char *) String_Value_Get (
          RunStats_ExitCond_Get (rstat_p)), XmFONTLIST_DEFAULT_TAG);
        XtVaSetValues (cycle_mess, XmNmessageString, mess_str, NULL);
        XmStringFree (mess_str);
        XtVaGetValues (cycle_mess,
          XmNcancelButton, &halt_pb,
          NULL);
        XtUnmanageChild (halt_pb);
        }
      return;
      }

    else if (Time_Format (Timer_Cycle_Get (RunStats_CumTimes_Get (rstat_p))) 
        >= Rcb_MaxRuntime_Get (rcb_p) * 60)
      {
      String_Destroy (RunStats_ExitCond_Get (rstat_p));
      RunStats_ExitCond_Put (rstat_p, 
        String_Create ("Maximum runtime exceeded.", 0));
      if (XtIsManaged (cycle_mess))
        {
        Widget        halt_pb;

        mess_str = XmStringCreateLtoR ((char *) String_Value_Get (
          RunStats_ExitCond_Get (rstat_p)), XmFONTLIST_DEFAULT_TAG);
        XtVaSetValues (cycle_mess, XmNmessageString, mess_str, NULL);
        XmStringFree (mess_str);
        XtVaGetValues (cycle_mess,
          XmNcancelButton, &halt_pb,
          NULL);
        XtUnmanageChild (halt_pb);
        }
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
        if (XtIsManaged (cycle_mess))
          {
          Widget        halt_pb;

          mess_str = XmStringCreateLtoR ((char *) String_Value_Get (
            RunStats_ExitCond_Get (rstat_p)), XmFONTLIST_DEFAULT_TAG);
          XtVaSetValues (cycle_mess, XmNmessageString, mess_str, NULL);
          XmStringFree (mess_str);
          XtVaGetValues (cycle_mess,
            XmNcancelButton, &halt_pb,
            NULL);
          XtUnmanageChild (halt_pb);
          }
        return;
        }

      else if (Time_Format (Timer_Cycle_Get (RunStats_CumTimes_Get (rstat_p))) 
          >= (Time_Format (RunStats_TimeFirstSol_Get (rstat_p))
          + Rcb_AddTime_Get (rcb_p) * 60))
        {
        String_Destroy (RunStats_ExitCond_Get (rstat_p));
        RunStats_ExitCond_Put (rstat_p, String_Create (
          "Additional runtime exceeded after first solution was found.", 0));
        if (XtIsManaged (cycle_mess))
          {
          Widget        halt_pb;

          mess_str = XmStringCreateLtoR ((char *) String_Value_Get (
            RunStats_ExitCond_Get (rstat_p)), XmFONTLIST_DEFAULT_TAG);
          XtVaSetValues (cycle_mess, XmNmessageString, mess_str, NULL);      char outbuf[MX_OUTPUT_SIZE];
          XmStringFree (mess_str);
          XtVaGetValues (cycle_mess,
            XmNcancelButton, &halt_pb,
            NULL);
          XtUnmanageChild (halt_pb);
          }
        return;
        }
      }
    }  /* End of else not continuation */

  Timer_Cycle_Put (&cur_timers, Syn_Time_Get (FALSE), SET);
  cur_compound_p = NULL;
  end_search = FALSE;

  Timer_Select_Put (&cur_timers, Syn_Time_Get (FALSE), SET);

if (glob_sel_cmp == NULL)
  cur_compound_p = Strategy_Select_Next (subroot_p, Rcb_Strategy_Get (rcb_p),
    Rcb_Flags_EffortDis_Get (rcb_p), NULL);
else
{
  pcb_p = Pst_ControlHandle_Get ();

  symtab_p = PstComp_SymbolTable_Get (glob_sel_cmp);
  SymTab_Current_Put (symtab_p, glob_sel_cmp);
  SymTab_DevelopedComp_Put (symtab_p, glob_sel_cmp);
  PstCB_CurrentComp_Put (pcb_p, glob_sel_cmp);
  SymTab_Flags_Selected_Put (symtab_p, TRUE);
  SymTab_Flags_WasSelected_Put (symtab_p, TRUE);
  cur_compound_p = glob_sel_cmp;
}

  SymbolTable_PostSelect_Reset (Rcb_NTCL_Get (rcb_p) > 0, TRUE);
  Timer_Select_Put (&cur_timers, Syn_Time_Get (TRUE), ADD);
  Timer_Select_Put (RunStats_CumTimes_Get (rstat_p), 
    Timer_Select_Get (&cur_timers), ADD);

  if (cur_compound_p == NULL)
      {
      String_Destroy (RunStats_ExitCond_Get (rstat_p));
      RunStats_ExitCond_Put (rstat_p, 
        String_Create ("No compounds to expand (search is stuck).", 0));
      if (XtIsManaged (cycle_mess))
        {
        Widget        halt_pb;

        mess_str = XmStringCreateLtoR ((char *) String_Value_Get (
          RunStats_ExitCond_Get (rstat_p)), XmFONTLIST_DEFAULT_TAG);
        XtVaSetValues (cycle_mess, XmNmessageString, mess_str, NULL);
        XmStringFree (mess_str);
        XtVaGetValues (cycle_mess,
          XmNcancelButton, &halt_pb,
          NULL);
        XtUnmanageChild (halt_pb);
        }
      return;
      }

  /* Now comes the main search loop which executes till time runs out.  There
     does not appear to be a convenient manner for exiting when there are no
     more nodes to search look at.
  */

#ifdef _MIND_MEM_
in_cycles = TRUE;
#endif
  while (!end_search)
    {
    Timer_Expand_Put (&cur_timers, Syn_Time_Get (FALSE), SET);
    PstCB_NextExpandedComp_Put (cur_compound_p);
    xtr_p = Sling_CanonicalName2Xtr (SymTab_Sling_Get (
      PstComp_SymbolTable_Get (cur_compound_p)));
#ifdef _DEBUG_
Xtr_Dump(xtr_p,&GStdErr);
#endif
    Xtr_Attr_ResonantBonds_Set (xtr_p);
    Xtr_Aromat_Set (xtr_p);
    num_rings = Xtr_Rings_NumRingSys_Get (xtr_p);
    for (ring_i = 0; ring_i < num_rings; ring_i++)
      Xtr_Ringdef_Set (xtr_p, ring_i);

    Xtr_Atoms_Set (xtr_p);
    Xtr_FuncGroups_Put (xtr_p, FuncGroups_Create (xtr_p));
#ifdef _MIND_MEM_
    mind_Array_2d_Create ("&strategic_bonds", "search_gui", &strategic_bonds, Xtr_NumAtoms_Get (xtr_p),
      MX_NEIGHBORS, BITSIZE);
    mind_Array_1d_Create ("&stereo_options", "search_gui", &stereo_options, Xtr_NumAtoms_Get (xtr_p),
      BITSIZE);
#else
    Array_2d_Create (&strategic_bonds, Xtr_NumAtoms_Get (xtr_p),
      MX_NEIGHBORS, BITSIZE);
    Array_1d_Create (&stereo_options, Xtr_NumAtoms_Get (xtr_p),
      BITSIZE);
#endif


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

    for (schema_inx = 0; schema_inx < React_NumSchemas_Get (); schema_inx++)
      if (Persist_Legacy_Rxn (schema_inx, PER_TIME_ANY, PER_TIME_ANY, &is_temp))
      {
      schema = Persist_Current_Rec (PER_STD, schema_inx, PER_TIME_ANY, PER_TIME_ANY, &is_temp);
      accept = React_Schema_IsOk (xtr_p, schema);
#ifdef _DEBUG_
printf("begin loop: schema %d (schema_inx %d)\n%s\n",schema,schema_inx,
    Sling_Name_Get (SymTab_Sling_Get (PstComp_SymbolTable_Get (cur_compound_p))));
#endif
      if (accept == TRUE)
        {      char outbuf[MX_OUTPUT_SIZE];
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

#ifdef _MIND_MEM_
  before_subgenr (schema, schema_inx);
#endif
        SubGenr_Subgoals_Generate (xtr_p, &strategic_bonds,
          &stereo_options, schema, Rcb_Flags_StereoChemistry_Get (rcb_p),
          Rcb_Flags_PreserveStructures_Get (rcb_p), NULL);
#ifdef _MIND_MEM_
  after_subgenr (schema, schema_inx);
#endif
        }  /* End of if schema okay */

      else
        {
        TRACE (R_MAIN, DB_CHEMISTRY, TL_LOOP, (outbuf,
          "Schema %lu rejected", schema == PER_DELETED ? schema_inx : schema));
        }
#ifdef _DEBUG_
printf("end loop\n");
#endif
      }  /* End of for-schema loop */

#ifdef _DEBUG_
printf("destroying strategic_bonds and stereo_options arrays\n");
#endif
#ifdef _MIND_MEM_
    mind_Array_Destroy ("&strategic_bonds", "search_gui", &strategic_bonds);
    mind_Array_Destroy ("&stereo_options", "search_gui", &stereo_options);
#else
    Array_Destroy (&strategic_bonds);
    Array_Destroy (&stereo_options);
#endif
    Xtr_Destroy (xtr_p);
    xtr_p = NULL;
#ifdef _DEBUG_
printf("destroyed arrays and xtr_p\n");
#endif

#ifdef _MIND_MEM_
  after_subgenr (0, 0);
#endif
    Timer_Expand_Put (&cur_timers, Syn_Time_Get (TRUE), ADD);
    Timer_Expand_Put (RunStats_CumTimes_Get (rstat_p), 
      Timer_Expand_Get (&cur_timers), ADD);

#ifdef _MIND_MEM_
  after_subgenr (0, 1);
#endif
    /* Back up time for effort heuristic. */
    SymTab_Flags_Selected_Put (PstComp_SymbolTable_Get (cur_compound_p), FALSE);
    Timer_UpdatePST_Put (&cur_timers, Syn_Time_Get (FALSE), SET);
if (glob_sel_cmp == NULL)
    Strategy_SearchTimes_Backup (cur_compound_p, 
      Timer_Expand_Get (&cur_timers), NULL);
    Pst_Update ();
    Timer_UpdatePST_Put (&cur_timers, Syn_Time_Get (TRUE), ADD);
    Timer_UpdatePST_Put (RunStats_CumTimes_Get (rstat_p), 
      Timer_UpdatePST_Get (&cur_timers), ADD);

#ifdef _MIND_MEM_
  after_subgenr (1, 1);
#endif
    RunStats_CumCycles_Get (rstat_p)++;
#ifdef _DEBUG_
printf("updated cycles; about to update exit condition\n");
#endif

    /* Calculate the number of cycles executed so far this run. 
       Need to check if we have done as much work on this problem as
       the user specified in which case it is time to quit.
    */
#ifdef _MIND_MEM_
  after_subgenr (2, 1);
#endif
    if (RunStats_Flags_SearchHalted_Get (rstat_p) || Halt_File_Present ())
      {
      String_Destroy (RunStats_ExitCond_Get (rstat_p));
      RunStats_ExitCond_Put (rstat_p, 
        String_Create ("Search halted by user.", 0));
      RunStats_Flags_SearchHalted_Put (rstat_p, FALSE);
      end_search = TRUE;
      }

    else if (Rcb_Flags_Continue_Get (rcb_p) 
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
      }  /* End of else not continuation */

#ifdef _DEBUG_
printf("updating timer info\n");
#endif

#ifdef _MIND_MEM_
  after_subgenr (2, 2);
#endif
    Timer_Cycle_Put (&cur_timers, Syn_Time_Get (TRUE), ADD);
    Timer_Cycle_Put (RunStats_CumTimes_Get (rstat_p), 
      Timer_Cycle_Get (&cur_timers), ADD);

#ifdef _MIND_MEM_
  after_subgenr (2, 3);
#endif
#ifdef _DEBUG_
printf("updating symbol table\n");
#endif
    SymTab_Cycle_Number_Put (PstComp_SymbolTable_Get (cur_compound_p), 
       RunStats_CumCycles_Get (rstat_p));
    SymTab_Cycle_Time_Put (PstComp_SymbolTable_Get (cur_compound_p),
       Timer_Cycle_Get (&cur_timers));
#ifdef _MIND_MEM_
  after_subgenr (3, 1);
#endif

#ifdef _DEBUG_
printf("displaying cycle message\n");
#endif

    if (first_solved_this_cycle) first_solved_this_cycle = FALSE;
    else if (!already_solved && RunStats_Flags_FirstSolFound_Get (rstat_p))
      first_solved_this_cycle=already_solved=TRUE;
#ifdef _MIND_MEM_
  after_subgenr (3, 2);
#endif

    if (XtIsManaged (cycle_mess))
      {
      Display     *dsp = XtDisplay (top_app);
      Window       win = XtWindow (cycle_mess);
      XEvent       event;

      if (RunStats_Flags_FirstSolFound_Get (rstat_p))
        sprintf (buffer, "End of cycle  %1lu\nTarget compound solved", 
          RunStats_CumCycles_Get (rstat_p));
      else
        sprintf (buffer, "End of cycle  %1lu", 
          RunStats_CumCycles_Get (rstat_p));
      mess_str = XmStringCreateLtoR (buffer, XmFONTLIST_DEFAULT_TAG);
      XtVaSetValues (cycle_mess, XmNmessageString, mess_str, NULL);
      XmStringFree (mess_str);
      XFlush (dsp);
      XmUpdateDisplay (top_app);

      while (XCheckMaskEvent (dsp,  ButtonPressMask | ButtonReleaseMask
          | ButtonMotionMask | KeyPressMask | KeyReleaseMask 
          | PointerMotionMask | ExposureMask, &event))
        {
        if (event.xany.window == win || event.xany.type == Expose)
          XtDispatchEvent (&event);
        else
          XBell (dsp, 10);
        }
      }

#ifdef _MIND_MEM_
  after_subgenr (3, 3);
#endif
    cycle_num = PstCB_TotalExpandedCompounds_Get (Pst_ControlHandle_Get ());

    if (GTrace[DB_CHEMISTRY].options < TL_SELECT)
      {
      char outbuf[MX_OUTPUT_SIZE];

      sprintf (outbuf, "End cycle #%lu", cycle_num /* PstCB_TotalExpandedCompounds_Get (Pst_ControlHandle_Get ()) */);
      IO_Put_Trace (R_NONE, outbuf);
      fflush (IO_FileHandle_Get (&GTraceFile));
      }
    else TRACE (R_NONE, DB_CHEMISTRY, TL_SELECT, (outbuf, 
      "Cyc: %5lu  Sels: %6lu  Selc: %6lu  Tim: %10.4f  CumT: %10.4f",
      cycle_num,
/*
      PstCB_TotalExpandedCompounds_Get (Pst_ControlHandle_Get ()),
*/
      SymTab_Index_Get (PstComp_SymbolTable_Get (cur_compound_p)),
      PstComp_Index_Get (cur_compound_p),
      Time_Format (Timer_Cycle_Get (&cur_timers)), 
      Time_Format (Timer_Cycle_Get (RunStats_CumTimes_Get (rstat_p)))));
 

#ifdef _MIND_MEM_
  after_subgenr (3, 4);
#endif
    if (!end_search)
      {
      /*  Starting clock for time spent on next cycle.  */
      Timer_Cycle_Put (&cur_timers, Syn_Time_Get (FALSE), SET);
      Timer_Select_Put (&cur_timers, Syn_Time_Get (FALSE), SET);

      cur_compound_p = Strategy_Select_Next (subroot_p, 
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
        end_search = TRUE;
        }
      }

#ifdef _MIND_MEM_
  after_subgenr (3, 5);
#endif
    if ((end_search && RTRcb_FinalSave_Flag_Get (&GRTRcb)) ||
      ((first_solved_this_cycle && RTRcb_1stSolSave_Flag_Get (&GRTRcb)) ||
      (RTRcb_CycleSave_Flag_Get (&GRTRcb) && cycle_num >= RTRcb_CycleStart_Get (&GRTRcb) &&
      (cycle_num - RTRcb_CycleStart_Get (&GRTRcb)) % RTRcb_CycleIncr_Get (&GRTRcb) == 0)))
      {
      char outbuf[MX_OUTPUT_SIZE];

      write_fail_fatal = FALSE;
      write_fail_str[0] = '\0';

      Status_File_Write ((char *)String_Value_Get (FileCB_FileStr_Get (
        Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_STAT))), &GGoalRcb, &GRunStats);

      if (end_search && write_fail_str[0] == '\0') RunStats_Flags_StatusSaved_Put (rstat_p, TRUE);
      if (write_fail_str[0] != '\0')
        {
        sprintf (outbuf, "Status file write failed: %s [%sFATAL]", write_fail_str, write_fail_fatal ? "" : "NON-");
        IO_Put_Trace (R_NONE, outbuf);
        }
      }
    if (!end_search && ((first_solved_this_cycle && RTRcb_1stSolSave_Flag_Get (&GRTRcb)) ||
      (RTRcb_CycleSave_Flag_Get (&GRTRcb) && cycle_num >= RTRcb_CycleStart_Get (&GRTRcb) &&
      (cycle_num - RTRcb_CycleStart_Get (&GRTRcb)) % RTRcb_CycleIncr_Get (&GRTRcb) == 0)))
      {
      char outbuf[MX_OUTPUT_SIZE];
      PstCB_t *pstcb_p;

      sprintf (outbuf, "\n\nDump of Run Status\n%-30s  %lu", "  Cycles completed:", RunStats_CumCycles_Get (rstat_p));
      IO_Put_Trace (R_NONE, outbuf);

      if (RunStats_Flags_FirstSolFound_Get (rstat_p))
        {
        sprintf (outbuf, "%-30s  %lu\n%-30s  %s", "  Cycles to first solution:", RunStats_CyclesFirstSol_Get (rstat_p),
          "  Time to first solution:", Sensible_Time_Format (RunStats_TimeFirstSol_Get (rstat_p), FALSE));
        IO_Put_Trace (R_NONE, outbuf);
        }

      pstcb_p = Pst_ControlHandle_Get ();
      sprintf (outbuf, "%-30s  %lu\n%-30s  %lu\n%-30s  %lu\n", "  Unique compounds generated:", PstCB_SymtabIndex_Get (pstcb_p),
        "  Total compounds generated:", PstCB_CompoundIndex_Get (pstcb_p), "  Total subgoals generated:",
        PstCB_SubgoalIndex_Get (pstcb_p));
      IO_Put_Trace (R_NONE, outbuf);
      fflush (IO_FileHandle_Get (&GTraceFile));
      }
#ifdef _MIND_MEM_
  after_subgenr (3, 6);
#endif

#ifdef _DEBUG_
printf("end while loop\n");
#endif
  }  /* End of while-loop, not end of search */
#ifdef _DEBUG_
printf("end search in progress\n");
#endif

#ifdef _MIND_MEM_
in_cycles = FALSE;
#endif

  if (XtIsManaged (cycle_mess))
    {
    Widget        halt_pb;

    mess_str = XmStringCreateLtoR ((char *) String_Value_Get (
      RunStats_ExitCond_Get (rstat_p)), XmFONTLIST_DEFAULT_TAG);
    XtVaSetValues (cycle_mess, XmNmessageString, mess_str, NULL);
    XmStringFree (mess_str);
    XtVaGetValues (cycle_mess,
      XmNcancelButton, &halt_pb,
      NULL);
    XtUnmanageChild (halt_pb);
    }

#ifdef _DEBUG_
printf("updating run stats\n");
#endif
  RunStats_NumCompounds_Put (rstat_p, 
    PstCB_CompoundIndex_Get (Pst_ControlHandle_Get ()));
  RunStats_NumSubgoals_Put (rstat_p, 
    PstCB_SubgoalIndex_Get (Pst_ControlHandle_Get ()));
  RunStats_NumSymbols_Put (rstat_p, 
    PstCB_SymtabIndex_Get (Pst_ControlHandle_Get ()));
  RunStats_Flags_PstChanged_Put (rstat_p, TRUE);
/*
Have to wait until tree is redisplayed!
if (glob_sel_cmp != NULL)
{
  cmpinx=PstComp_Index_Get (glob_sel_cmp);
  SS_WalkPath (cmpinx, FileCB_FileStr_Get(Rcb_IthFileCB_Get(rcb_p, FCB_IND_PATH)));
}
*/
#ifdef _DEBUG_
printf("returning from Synchem_Search\n");
#endif
printf("kka:exiting Synchem_Search\n");
  return;
}
/* End of Synchem_Search */

Boolean_t Halt_File_Present ()
{
  char filename[128], *status;
  FILE *f;

  strcpy (filename, String_Value_Get (FileCB_FileStr_Get (Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_STAT))));
  status = strstr (filename, "status");
  strncpy (status, "halt", 4);
  strcpy (status + 4, status + 6);
  status = strstr (filename, "status");
  strncpy (status, "halt", 4);
  strcpy (status + 4, status + 6);
  f = fopen (filename, "r");
  if (f != NULL)
  {
    fclose (f);
    remove (filename);
    return (TRUE);
  }
  return (FALSE);
}

/* End of SYN_SEARCH.C */
