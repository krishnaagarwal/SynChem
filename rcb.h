#ifndef _H_RCB_
#define _H_RCB_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1997 Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     RCB.H
*
*    This module is the abstraction for the Run Control Block.  It contains
*    all the stuff that is write-once at the beginning of a run and possibly
*    pointers to current data that should be kept in one place.
*
*    Routines can be found in RCB.C.
*
*  Creation Date:
*
*    01-Jan-1991
*
*  Authors:
*
*    Daren Krebsbach
*    Shu Cheung
*    Tito Autrey (rewritten based on others PLI code)
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 23-Sep-97  Krebsbach  Combined sequential and distributed versions
*                       of SYNCHEM into single file, making many changes to
*                       data structures and constants.
* 09-Jul-97  Krebsbach  Added flags for controling localized dis search.
* 09-Jul-97  Krebsbach  Added flag for shelved merits.
* 07-Oct-96  Krebsbach  Added flag for effort distribution.
* 26-Sep-96  Krebsbach  Moved cumalative timers into RunStats_t.
* 26-Sep-96  Krebsbach  Placed run statistics data in a separate structure.
* 23-Sep-96  Krebsbach  Added file tags for Rcb_t information.
* 18-Sep-96  Krebsbach  Added parallel information to Rcb_t.
* 09-Aug-95  Cheung     Created structure Filecb_t, added field data_files in
*                       Rcb_t.
*
******************************************************************************/

#ifndef _H_SYNIO_
#include "synio.h"
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

#ifndef _H_STRATEGY_
#include "strategy.h"
#endif

/*** Literal Values ***/

#define RCB_TYPE_NONE              0
#define RCB_TYPE_SEQ               1
#define RCB_TYPE_SEQ_FORCESEL      2          /* Forced selection */
#define RCB_TYPE_DIS_LIN           10
#define RCB_TYPE_DIS_PVM           11

#define RCB_TYPE_SEQ_START         RCB_TYPE_SEQ
#define RCB_TYPE_DIS_START         RCB_TYPE_DIS_LIN

#define RCB_FILE_DELIMS            " \t\n"
#define RCB_FILE_COMMENT           '!'

#define RCB_FILE_TAG_ADD_CYCLES    "AdditionalCycles"
#define RCB_FILE_TAG_ADD_TIME      "AdditionalTime"
#define RCB_FILE_TAG_BACKGROUND    "BackgroundProcess"
#define RCB_FILE_TAG_CHEM_TRACE    "ChemistryTraceLevel"
#define RCB_FILE_TAG_COMMENT       "Comment"
#define RCB_FILE_TAG_EFFORTDIS     "EffortDistribution"
#define RCB_FILE_TAG_FIRSTSOL      "FirstSolution"
#define RCB_FILE_TAG_GOAL          "TargetCompound"
#define RCB_FILE_TAG_LEAP          "LeapSize"
#define RCB_FILE_TAG_MAX_CYCLES    "MaxCycles"
#define RCB_FILE_TAG_MAX_NODES     "MaxNodes"
#define RCB_FILE_TAG_MAX_TIME      "MaxRuntime"
#define RCB_FILE_TAG_MAX_WORKERS   "MaxWorkers"
#define RCB_FILE_TAG_MIN_WORKERS   "MinWorkers"
#define RCB_FILE_TAG_NAME          "CompoundName"
#define RCB_FILE_TAG_NTCL          "NTCL"
#define RCB_FILE_TAG_PRESERVE      "PreserveStructures"
#define RCB_FILE_TAG_PROC_NODE     "ProcessesPerNode"
#define RCB_FILE_TAG_RUNCONTINUE   "RunContinuation"
#define RCB_FILE_TAG_RUNRESTART    "RunRestart"
#define RCB_FILE_TAG_RUNTYPE       "RunType"
#define RCB_FILE_TAG_SAVE_MERITS   "SaveShelvedMerits"
#define RCB_FILE_TAG_SAVE_STATUS   "SaveStatusFile"
#define RCB_FILE_TAG_SAVE_TRACE    "SaveTraceFile"
#define RCB_FILE_TAG_STEREO        "NoStereochemistry"
#define RCB_FILE_TAG_STRATBONDS    "StrategicBonds"

#define RCB_FILE_TAG_DIR_AVL       "AvailCompLibDir"
#define RCB_FILE_TAG_DIR_COVER     "CoverDir"
#define RCB_FILE_TAG_DIR_FG        "FunctionalGroupDir"
#define RCB_FILE_TAG_DIR_FORCESEL  "ForceSelectDir"
#define RCB_FILE_TAG_DIR_PATH      "PathDir"
#define RCB_FILE_TAG_DIR_RXN       "ReactionLibDir"
#define RCB_FILE_TAG_DIR_STAT      "StatusDir"
#define RCB_FILE_TAG_DIR_SUBMIT    "SubmitDir"
#define RCB_FILE_TAG_DIR_TEMPL     "TemplateDir"
#define RCB_FILE_TAG_DIR_TRACE     "TraceDir"

#define RCB_FILE_TAG_FILE_COVER    "CoverFile"
#define RCB_FILE_TAG_FILE_FORCESEL "ForceSelectFile"

#define RCB_FILE_TAG_COVER_PRCT    "DevCoverPercent"
#define RCB_FILE_TAG_COVER_DEV     "SearchSpaceCoverDev"
#define RCB_FILE_TAG_COVER_SLV     "SearchSpaceCoverSlv"

#define RCB_FILE_TAG_DIS_SHARED_SELS   "DisSharedSels"
#define RCB_FILE_TAG_AND_PARALLELISM   "AndParallelism"
#define RCB_FILE_TAG_MASTER_SEL_NB     "MasterSelNextBest"
#define RCB_FILE_TAG_WORKER_LOCAL_SEL  "WorkerLocalSel"
#define RCB_FILE_TAG_WORKER_MAX_CYCLES "WorkerMaxCycles"
#define RCB_FILE_TAG_WORKER_CURMERIT   "WorkerCurrentMerit"
#define RCB_FILE_TAG_WORKER_NBMERIT    "WorkerNextBestMerit"
#define RCB_FILE_TAG_WORKER_MERIT_PRCT "WorkerMeritPercent"
#define RCB_FILE_TAG_WORKER_PREF_GLOBAL "WorkerPreferGlobalSel"

/*  Default directories and filenames  */

#define FCB_IND_NUMOF           10
#define FCB_IND_AVLC            0
#define FCB_IND_FNGP            1
#define FCB_IND_RXNS            2
#define FCB_IND_TMPL            3
#define FCB_IND_PATH            4
#define FCB_IND_STAT            5
#define FCB_IND_SBMT            6
#define FCB_IND_TRCE            7
#define FCB_IND_FSEL            8
#define FCB_IND_COVR            9

#define FCB_LIB_START           FCB_IND_AVLC
#define FCB_USR_START           FCB_IND_PATH
#define FCB_EXP_START           FCB_IND_FSEL

#define FCB_SEQDIR_SYS(rest)    concat (concat (glob_main_dir, "/data"), (rest))
#define FCB_SEQDIR_USR(rest)    concat (concat (glob_main_dir, "/data"), (rest))

#define FCB_DISDIR_SYS(rest)    concat (concat (glob_main_dir, "/data"), (rest))
#define FCB_DISDIR_USR(rest)    concat (concat (glob_main_dir, "/data"), (rest))

#define FCB_PATH_SEP            "/"

/*  System data subdirectories  */
#define FCB_SUBDIR_AVLC         "/avlcomp"
#define FCB_SUBDIR_FNGP         "/fg"
#define FCB_SUBDIR_RXNS         "/rxnlib"
#define FCB_SUBDIR_TMPL         "/template"

/*  User data subdirectories  */
#define FCB_SUBDIR_PATH         "/pfile"
#define FCB_SUBDIR_STAT         "/status"
#define FCB_SUBDIR_SBMT         "/submit"
#define FCB_SUBDIR_TRCE         "/trace"

#define FCB_SUBDIR_COVR         "/cover"
#define FCB_SUBDIR_FSEL         "/select"

#define FCB_SUBDIR_DSTM         "/draw_sling_template"
#define FCB_SUBDIR_FGTM         "/fg_template"
#define FCB_SUBDIR_USER         "/users"

/*  User data filename extentions  */

#define FCB_EXT_PATH            ".path"
#define FCB_EXT_STAT            ".status"
#define FCB_EXT_SBMT            ".submit"
#define FCB_EXT_TRCE            ".trace"
#define FCB_EXT_COVR            ".cover"
#define FCB_EXT_FSEL            ".order"


/*  Default startup values  */

/*  Sequential and Distributed:  */

#define RCB_DFLT_MAX_CYCLES        1000
#define RCB_DFLT_CHEMTRACE         TL_RUNSTATS
#define RCB_DFLT_CMP_NAME          ""
#define RCB_DFLT_ADD_CYCLES        0           /* Add cycles beyond 1st sol */
#define RCB_DFLT_ADD_TIME          0           /* Add time beyond 1st sol */
#define RCB_DFLT_COMMENT           ""
#define RCB_DFLT_FCONTINUE         FALSE       /* Continuation of run? */
#define RCB_DFLT_FFIRST_SOL        FALSE       /* Stop at first solution? */
#define RCB_DFLT_FPRES_STRCT       TRUE
#define RCB_DFLT_FRESTART          FALSE       /* Restart of saved of run? */
#define RCB_DFLT_FSAVE_MERITS      FALSE       /* Save shelved merits? */
#define RCB_DFLT_FSAVE_STATUS      TRUE        /* Save status file? */
#define RCB_DFLT_FSAVE_TRACE       TRUE        /* Save trace file? */
#define RCB_DFLT_FSTEREO_CHEM      FALSE
#define RCB_DFLT_FSTRAT_BONDS      FALSE
#define RCB_DFLT_GOAL              ""
#define RCB_DFLT_LEAPSIZE          0

#define RCB_DFLT_COVER_DEV_PRCT    0           /* Percent develeped cover */
#define RCB_DFLT_FSEARCH_COVER_DEV FALSE       /* Test for developed cover? */
#define RCB_DFLT_FSEARCH_COVER_SLV FALSE       /* Test for solved cover? */

/*  Sequential Only:  */

#define FCB_SEQDIR_AVLC(rest)   concat (FCB_SEQDIR_SYS (FCB_SUBDIR_AVLC), (rest))
#define FCB_SEQDIR_FNGP(rest)   concat (FCB_SEQDIR_SYS (FCB_SUBDIR_FNGP), (rest))
#define FCB_SEQDIR_RXNS(rest)   concat (FCB_SEQDIR_SYS (FCB_SUBDIR_RXNS), (rest))
#define FCB_SEQDIR_TMPL(rest)   concat (FCB_SEQDIR_SYS (FCB_SUBDIR_TMPL), (rest))
#define FCB_SEQDIR_USER(rest)   concat (FCB_SEQDIR_SYS (FCB_SUBDIR_USER), (rest))
#define FCB_SEQDIR_PATH(rest)   concat (FCB_SEQDIR_USR (FCB_SUBDIR_PATH), (rest))
#define FCB_SEQDIR_STAT(rest)   concat (FCB_SEQDIR_USR (FCB_SUBDIR_STAT), (rest))
#define FCB_SEQDIR_SBMT(rest)   concat (FCB_SEQDIR_USR (FCB_SUBDIR_SBMT), (rest))
#define FCB_SEQDIR_TRCE(rest)   concat (FCB_SEQDIR_USR (FCB_SUBDIR_TRCE), (rest))
#define FCB_SEQDIR_COVR(rest)   concat (FCB_SEQDIR_USR (FCB_SUBDIR_COVR), (rest))
#define FCB_SEQDIR_FSEL(rest)   concat (FCB_SEQDIR_USR (FCB_SUBDIR_FSEL), (rest))
#define FCB_SEQDIR_DSTM(rest)   concat (FCB_SEQDIR_USR (FCB_SUBDIR_DSTM), (rest))
#define FCB_SEQDIR_FGTM(rest)   concat (FCB_SEQDIR_USR (FCB_SUBDIR_FGTM), (rest))


#define RCB_SEQ_RUNTYPE            RCB_TYPE_SEQ
#define RCB_SEQ_BACKGROUND         FALSE       /* Run in the background? */
#define RCB_SEQ_EFFORTDIS          TRUE
#define RCB_SEQ_NTCL               25
#define RCB_SEQ_MAX_RUNTIME        120

#define RCB_SEQ_MAX_NODES          0
#define RCB_SEQ_MAX_WORKERS        0
#define RCB_SEQ_MIN_WORKERS        0
#define RCB_SEQ_PROC_PER_NODE      0

#define RCB_SEQ_DIS_SHARED_SELS    FALSE
#define RCB_SEQ_MASTER_SEL_NB      FALSE
#define RCB_SEQ_ANDPARALLEL        FALSE
#define RCB_SEQ_WORKER_LOCAL_SEL   FALSE
#define RCB_SEQ_WORKER_MAX_CYCLES  0
#define RCB_SEQ_WORKER_CURMERIT    FALSE
#define RCB_SEQ_WORKER_NBMERIT     FALSE
#define RCB_SEQ_WORKER_MERIT_PRCT  0
#define RCB_SEQ_WORKER_PREF_GLOBAL FALSE


/*  Default values for parallel/distributed runs.  */

#define FCB_DISDIR_AVLC(rest)   concat (FCB_DISDIR_SYS (FCB_SUBDIR_AVLC), (rest))
#define FCB_DISDIR_FNGP(rest)   concat (FCB_DISDIR_SYS (FCB_SUBDIR_FNGP), (rest))
#define FCB_DISDIR_RXNS(rest)   concat (FCB_DISDIR_SYS (FCB_SUBDIR_RXNS), (rest))
#define FCB_DISDIR_TMPL(rest)   concat (FCB_DISDIR_SYS (FCB_SUBDIR_TMPL), (rest))
#define FCB_DISDIR_PATH(rest)   concat (FCB_DISDIR_USR (FCB_SUBDIR_PATH), (rest))
#define FCB_DISDIR_STAT(rest)   concat (FCB_DISDIR_USR (FCB_SUBDIR_STAT), (rest))
#define FCB_DISDIR_SBMT(rest)   concat (FCB_DISDIR_USR (FCB_SUBDIR_SBMT), (rest))
#define FCB_DISDIR_TRCE(rest)   concat (FCB_DISDIR_USR (FCB_SUBDIR_TRCE), (rest))
#define FCB_DISDIR_COVR(rest)   concat (FCB_DISDIR_USR (FCB_SUBDIR_COVR), (rest))
#define FCB_DISDIR_FSEL(rest)   concat (FCB_DISDIR_USR (FCB_SUBDIR_FSEL), (rest))

#define RCB_DIS_RUNTYPE            RCB_TYPE_DIS_LIN
#define RCB_DIS_BACKGROUND         TRUE       /* Run in the background? */
#define RCB_DIS_EFFORTDIS          FALSE
#define RCB_DIS_NTCL               0
#define RCB_DIS_MAX_RUNTIME        60

#define RCB_DIS_MAX_NODES          9
#define RCB_DIS_MAX_WORKERS        20
#define RCB_DIS_MIN_WORKERS        20
#define RCB_DIS_PROC_PER_NODE      4

#define RCB_DIS_DIS_SHARED_SELS    FALSE
#define RCB_DIS_MASTER_SEL_NB      TRUE
#define RCB_DIS_ANDPARALLEL        FALSE
#define RCB_DIS_WORKER_LOCAL_SEL   TRUE
#define RCB_DIS_WORKER_MAX_CYCLES  10
#define RCB_DIS_WORKER_CURMERIT    TRUE
#define RCB_DIS_WORKER_NBMERIT     FALSE
#define RCB_DIS_WORKER_MERIT_PRCT  100
#define RCB_DIS_WORKER_PREF_GLOBAL TRUE

/*** Data Structures ***/

/*** File Control Block Data Structure ***/

typedef struct s_filecb
  {
  String_t     dir;                      /* directory for the file */
  String_t     file;                     /* complete path for file */
  U32_t        magic;                    /* magic number for the file */
  U16_t        version;                  /* version # for this run */
  U8_t         type;                     /* type of data file */
  Boolean_t    changed;                  /* has name/dir been changed? */
  } Filecb_t;
#define FILECBSIZE (sizeof (Filecb_t))

/** Field Access Macros for Filecb_t **/

/* Macro Prototypes

  Boolean_t   FileCB_Changed_Get  (Filecb_t);
  void        FileCB_Changed_Put  (Filecb_t, Boolean_t);
  String_t    FileCB_DirStr_Get   (Filecb_t);
  void        FileCB_DirStr_Put   (Filecb_t, String_t);
  String_t    FileCB_FileStr_Get  (Filecb_t);
  void        FileCB_FileStr_Put  (Filecb_t, String_t);
  U32_t       FileCB_MagicNum_Get (Filecb_t);
  void        FileCB_MagicNum_Put (Filecb_t, U32_t);
  U8_t        FileCB_Type_Get     (Filecb_t);
  void        FileCB_Type_Put     (Filecb_t, U8_t);
  U16_t       FileCB_Version_Get  (Filecb_t);
  void        FileCB_Version_Put  (Filecb_t, U16_t);
*/
#define FileCB_Changed_Get(fcb)\
  (fcb).changed
#define FileCB_Changed_Put(fcb, value)\
  (fcb).changed = (value)
#define FileCB_DirStr_Get(fcb)\
  (fcb).dir
#define FileCB_DirStr_Put(fcb, value)\
  (fcb).dir = (value)
#define FileCB_FileStr_Get(fcb)\
  (fcb).file
#define FileCB_FileStr_Put(fcb, value)\
  (fcb).file = (value)
#define FileCB_MagicNum_Get(fcb)\
  (fcb).magic
#define FileCB_MagicNum_Put(fcb, value)\
  (fcb).magic = (value)
#define FileCB_Type_Get(fcb)\
  (fcb).type
#define FileCB_Type_Put(fcb, value)\
  (fcb).type = (value)
#define FileCB_Version_Get(fcb)\
  (fcb).version
#define FileCB_Version_Put(fcb, value)\
  (fcb).version = (value)

/*** Run Statistics Data Structure ***/

typedef struct run_stats_s
  {
  String_t       exit_cond;                  /* Exit condition */
  Timer_t        master_cum_times;           /* Master/seq cummulative times */
  Timer_t        worker_cum_times;           /* Worker cummulative times */
  Time_t         time_first_sol;             /* time of 1st sol */
  U32_t          cum_cycles;                 /* Cummulative cycles this run */
  U32_t          flags;                      /* Bitvector for flag values */
  U32_t          cycle_first_sol;            /* Cycle for 1st sol */
  U32_t          num_cmps;                   /* Number of compound nodes */
  U32_t          num_subgs;                  /* Number of subgoal nodes */
  U32_t          num_symbols;                /* Number of unique symbols */
  } RunStats_t;
#define RUNSTATS_SIZE sizeof (RunStats_t)

#define RstatsM_StatusSaved           0x1
#define RstatsM_TraceSaved            0x2
#define RstatsM_FirstSolFound         0x10
#define RstatsM_SearchHalted          0x20
#define RstatsM_DevSPCovered          0x40
#define RstatsM_SolSPCovered          0x80
#define RstatsM_PstChanged            0x100

/** Field Access Macros for a RunStats_t **/

/* Macro Prototypes 
   U32_t     RunStats_CumCycles_Get         (RunStats_t *);
   void      RunStats_CumCycles_Put         (RunStats_t *, U32_t);
   Timer_t  *RunStats_CumTimes_Get          (RunStats_t *);
   U32_t     RunStats_CyclesFirstSol_Get    (RunStats_t *);
   void      RunStats_CyclesFirstSol_Put    (RunStats_t *, U32_t);
   String_t  RunStats_ExitCond_Get          (RunStats_t *);
   void      RunStats_ExitCond_Put          (RunStats_t *, String_t);
   U32_t     RunStats_Flags_Get             (RunStats_t *);
   void      RunStats_Flags_Put             (RunStats_t *, U32_t);
   Boolean_t RunStats_Flags_DevSPCovered_Get (RunStats_t *);
   void      RunStats_Flags_DevSPCovered_Put (RunStats_t *, Boolean_t);
   Boolean_t RunStats_Flags_FirstSolFound_Get (RunStats_t *);
   void      RunStats_Flags_FirstSolFound_Put (RunStats_t *, Boolean_t);
   Boolean_t RunStats_Flags_PstChanged_Get  (RunStats_t *);
   void      RunStats_Flags_PstChanged_Put  (RunStats_t *, Boolean_t);
   Boolean_t RunStats_Flags_SearchHalted_Get (RunStats_t *);
   void      RunStats_Flags_SearchHalted_Put (RunStats_t *, Boolean_t);
   Boolean_t RunStats_Flags_SolSPCovered_Get (RunStats_t *);
   void      RunStats_Flags_SolSPCovered_Put (RunStats_t *, Boolean_t);
   Boolean_t RunStats_Flags_StatusSaved_Get (RunStats_t *);
   void      RunStats_Flags_StatusSaved_Put (RunStats_t *, Boolean_t);
   Boolean_t RunStats_Flags_TraceSaved_Get  (RunStats_t *);
   void      RunStats_Flags_TraceSaved_Put  (RunStats_t *, Boolean_t);
   Timer_t  *RunStats_MasterCumTimes_Get    (RunStats_t *);
   U32_t     RunStats_NumCompounds_Get      (RunStats_t *);
   void      RunStats_NumCompounds_Put      (RunStats_t *, U32_t);
   U32_t     RunStats_NumSubgoals_Get       (RunStats_t *);
   void      RunStats_NumSubgoals_Put       (RunStats_t *, U32_t);
   U32_t     RunStats_NumSymbols_Get        (RunStats_t *);
   void      RunStats_NumSymbols_Put        (RunStats_t *, U32_t);
   Time_t    RunStats_TimeFirstSol_Get      (RunStats_t *);
   void      RunStats_TimeFirstSol_Put      (RunStats_t *, Time_t);
   Timer_t  *RunStats_WorkerCumTimes_Get    (RunStats_t *);
*/

#define RunStats_CumCycles_Get(rs_p)\
  (rs_p)->cum_cycles
#define RunStats_CumCycles_Put(rs_p, value)\
 (rs_p)->cum_cycles = (value)

#define RunStats_CumTimes_Get(rs_p)\
  &(rs_p)->master_cum_times

#define RunStats_CyclesFirstSol_Get(rs_p)\
  (rs_p)->cycle_first_sol
#define RunStats_CyclesFirstSol_Put(rs_p, value)\
 (rs_p)->cycle_first_sol = (value)

#define RunStats_ExitCond_Get(rs_p)\
  (rs_p)->exit_cond
#define RunStats_ExitCond_Put(rs_p, value)\
 (rs_p)->exit_cond = (value)

#define RunStats_Flags_Get(rs_p)\
  (rs_p)->flags
#define RunStats_Flags_Put(rs_p, value)\
 (rs_p)->flags = (value)

#define RunStats_Flags_DevSPCovered_Get(rs_p)\
  ((rs_p)->flags & RstatsM_DevSPCovered ? TRUE : FALSE)
#define RunStats_Flags_DevSPCovered_Put(rs_p, value)\
  { if ((value) == TRUE)\
    (rs_p)->flags |= RstatsM_DevSPCovered;\
  else\
    (rs_p)->flags &= ~RstatsM_DevSPCovered; }

#define RunStats_Flags_FirstSolFound_Get(rs_p)\
  ((rs_p)->flags & RstatsM_FirstSolFound ? TRUE : FALSE)
#define RunStats_Flags_FirstSolFound_Put(rs_p, value)\
  { if ((value) == TRUE)\
    (rs_p)->flags |= RstatsM_FirstSolFound;\
  else\
    (rs_p)->flags &= ~RstatsM_FirstSolFound; }

#define RunStats_Flags_PstChanged_Get(rs_p)\
  ((rs_p)->flags & RstatsM_PstChanged ? TRUE : FALSE)
#define RunStats_Flags_PstChanged_Put(rs_p, value)\
  { if ((value) == TRUE)\
    (rs_p)->flags |= RstatsM_PstChanged;\
  else\
    (rs_p)->flags &= ~RstatsM_PstChanged; }

#define RunStats_Flags_SearchHalted_Get(rs_p)\
  ((rs_p)->flags & RstatsM_SearchHalted ? TRUE : FALSE)
#define RunStats_Flags_SearchHalted_Put(rs_p, value)\
  { if ((value) == TRUE)\
    (rs_p)->flags |= RstatsM_SearchHalted;\
  else\
    (rs_p)->flags &= ~RstatsM_SearchHalted; }

#define RunStats_Flags_SolSPCovered_Get(rs_p)\
  ((rs_p)->flags & RstatsM_SolSPCovered ? TRUE : FALSE)
#define RunStats_Flags_SolSPCovered_Put(rs_p, value)\
  { if ((value) == TRUE)\
    (rs_p)->flags |= RstatsM_SolSPCovered;\
  else\
    (rs_p)->flags &= ~RstatsM_SolSPCovered; }

#define RunStats_Flags_StatusSaved_Get(rs_p)\
  ((rs_p)->flags & RstatsM_StatusSaved ? TRUE : FALSE)
#define RunStats_Flags_StatusSaved_Put(rs_p, value)\
  { if ((value) == TRUE)\
    (rs_p)->flags |= RstatsM_StatusSaved;\
  else\
    (rs_p)->flags &= ~RstatsM_StatusSaved; }

#define RunStats_Flags_TraceSaved_Get(rs_p)\
  ((rs_p)->flags & RstatsM_TraceSaved ? TRUE : FALSE)
#define RunStats_Flags_TraceSaved_Put(rs_p, value)\
  { if ((value) == TRUE)\
    (rs_p)->flags |= RstatsM_TraceSaved;\
  else\
    (rs_p)->flags &= ~RstatsM_TraceSaved; }

#define RunStats_MasterCumTimes_Get(rs_p)\
  &(rs_p)->master_cum_times

#define RunStats_NumCompounds_Get(rs_p)\
  (rs_p)->num_cmps
#define RunStats_NumCompounds_Put(rs_p, value)\
 (rs_p)->num_cmps = (value)

#define RunStats_NumSubgoals_Get(rs_p)\
  (rs_p)->num_subgs
#define RunStats_NumSubgoals_Put(rs_p, value)\
 (rs_p)->num_subgs = (value)

#define RunStats_NumSymbols_Get(rs_p)\
  (rs_p)->num_symbols
#define RunStats_NumSymbols_Put(rs_p, value)\
 (rs_p)->num_symbols = (value)

#define RunStats_TimeFirstSol_Get(rs_p)\
  (rs_p)->time_first_sol
#define RunStats_TimeFirstSol_Put(rs_p, value)\
 (rs_p)->time_first_sol = (value)

#define RunStats_WorkerCumTimes_Get(rs_p)\
  &(rs_p)->worker_cum_times

	
/* Control block for the whole run, so only need to pass a pointer to this
   rather than a fortune in parameters or individual global values.  As much
   as possible "global" data is encapsulated inside other modules in order to
   limit the amount of code to consider that could interfere with the data
   consistency.

   Note:  This also has the contents of the header block from CREATE.PLI.

   Also flag values.
*/

typedef struct s_runcontrolblock
  {
  Filecb_t       kbdfiles[FCB_IND_NUMOF];    /* kb and data files */
  Sling_t        goalb;                      /* Goal compound, sling format */
  String_t       userb;                      /* User's name */
  String_t       nameb;                      /* English name of goal cmpd */
  String_t       commentb;                   /* Comment for status file */
  Strategy_Params_t strategyb;               /* Strategy control block */
  Time_t         date;                       /* Date submitted */
  U32_t          flags;                      /* Bitvector for flag values */
  U32_t          add_cycles;                 /* Add # cycles after 1st sol */
  U32_t          add_time;                   /* Add time after 1st sol */
  U32_t          leap_size;                  /* Num of cycles per leap */
  U32_t          max_cycles;                 /* Max # cycles this run */
  U32_t          max_runtime;                /* Max runtime (minutes) */
  U16_t          cover_prct;                 /* Search space cover percent */
  U16_t          max_workers;                /* Max workers, parallel run */
  U16_t          min_workers;                /* Min workers, parallel run */
  U16_t          worker_maxcycles;           /* Max cycles for workers */
  U8_t           chemistry_trace;            /* Chemistry tracing level */
  U8_t           max_nodes;                  /* Max number of nodes */
  U8_t           ntcl;                       /* Num of temp closed cycles */
  U8_t           proc_per_node;              /* Processes per node */
  U8_t           run_type;                   /* Run type:  seq or dis */
  U8_t           worker_merit_prct;          /* Percent merit for worker lb */
  } Rcb_t;
#define RCBSIZE sizeof (Rcb_t)

/* Flag literal values for run characteristics */

#define RcbM_StrategicBonds        0x00000001
#define RcbM_StereoChemistry       0x00000002
#define RcbM_PreserveStructures    0x00000004
#define RcbM_EffortDistribution    0x00000008
#define RcbM_Continuation          0x00000010
#define RcbM_Restart               0x00000020
#define RcbM_FirstSol              0x00000040
#define RcbM_Background            0x00000080
#define RcbM_SaveTrace             0x00000100
#define RcbM_SaveStatus            0x00000200
#define RcbM_SaveMerits            0x00000400
#define RcbM_LibsChanged           0x00001000
#define RcbM_LibsLoaded            0x00002000
#define RcbM_SearchCoverDev        0x00010000
#define RcbM_SearchCoverSlv        0x00020000
#define RcbM_WorkerLocalSel        0x00100000
#define RcbM_WorkerCurMerit        0x00200000
#define RcbM_WorkerNBMerit         0x00400000
#define RcbM_WorkerPreferGlobal    0x00800000
#define RcbM_MasterSelNB           0x01000000
#define RcbM_DisSharedSels         0x02000000
#define RcbM_AndParallelism        0x04000000

/** Field Access Macros for a Rcb_t **/

/* Macro Prototypes 
   U32_t     Rcb_AddCycles_Get            (Rcb_t *);
   void      Rcb_AddCycles_Put            (Rcb_t *, U32_t);
   U32_t     Rcb_AddTime_Get              (Rcb_t *);
   void      Rcb_AddTime_Put              (Rcb_t *, U32_t);
   U8_t      Rcb_ChemistryTrace_Get       (Rcb_t *);
   void      Rcb_ChemistryTrace_Put       (Rcb_t *, U8_t);
   String_t  Rcb_Comment_Get              (Rcb_t *);
   void      Rcb_Comment_Put              (Rcb_t *, String_t);
   U16_t     Rcb_CoverPrct_Get            (Rcb_t *);
   void      Rcb_CoverPrct_Put            (Rcb_t *, U16_t);
   Time_t    Rcb_Date_Get                 (Rcb_t *);
   void      Rcb_Date_Put                 (Rcb_t *, Time_t);
   Filecb_t *Rcb_FileCBs_Get              (Rcb_t *);
   U32_t     Rcb_Flags_Get                (Rcb_t *);
   void      Rcb_Flags_Put                (Rcb_t *, U32_t);
   Boolean_t Rcb_Flags_AndParallel_Get    (Rcb_t *);
   void      Rcb_Flags_AndParallel_Put    (Rcb_t *, Boolean_t);
   Boolean_t Rcb_Flags_Background_Get     (Rcb_t *);
   void      Rcb_Flags_Background_Put     (Rcb_t *, Boolean_t);
   Boolean_t Rcb_Flags_Continue_Get       (Rcb_t *);
   void      Rcb_Flags_Continue_Put       (Rcb_t *, Boolean_t);
   Boolean_t Rcb_Flags_DisSharedSels_Get  (Rcb_t *);
   void      Rcb_Flags_DisSharedSels_Put  (Rcb_t *, Boolean_t);
   Boolean_t Rcb_Flags_EffortDis_Get      (Rcb_t *);
   void      Rcb_Flags_EffortDis_Put      (Rcb_t *, Boolean_t);
   Boolean_t Rcb_Flags_FirstSol_Get       (Rcb_t *);
   void      Rcb_Flags_FirstSol_Put       (Rcb_t *, Boolean_t);
   Boolean_t Rcb_Flags_LibsChanged_Get    (Rcb_t *);
   void      Rcb_Flags_LibsChanged_Put    (Rcb_t *, Boolean_t);
   Boolean_t Rcb_Flags_LibsLoaded_Get     (Rcb_t *);
   void      Rcb_Flags_LibsLoaded_Put     (Rcb_t *, Boolean_t);
   Boolean_t Rcb_Flags_MasterSelNB_Get    (Rcb_t *);
   void      Rcb_Flags_MasterSelNB_Put    (Rcb_t *, Boolean_t);
   Boolean_t Rcb_Flags_PreserveStructures_Get (Rcb_t *);
   void      Rcb_Flags_PreserveStructures_Put (Rcb_t *, Boolean_t);
   Boolean_t Rcb_Flags_Restart_Get        (Rcb_t *);
   void      Rcb_Flags_Restart_Put        (Rcb_t *, Boolean_t);
   Boolean_t Rcb_Flags_SaveMerits_Get     (Rcb_t *);
   void      Rcb_Flags_SaveMerits_Put     (Rcb_t *, Boolean_t);
   Boolean_t Rcb_Flags_SaveStatus_Get     (Rcb_t *);
   void      Rcb_Flags_SaveStatus_Put     (Rcb_t *, Boolean_t);
   Boolean_t Rcb_Flags_SaveTrace_Get      (Rcb_t *);
   void      Rcb_Flags_SaveTrace_Put      (Rcb_t *, Boolean_t);
   Boolean_t Rcb_Flags_SearchCoverDev_Get (Rcb_t *);
   void      Rcb_Flags_SearchCoverDev_Put (Rcb_t *, Boolean_t);
   Boolean_t Rcb_Flags_SearchCoverSlv_Get (Rcb_t *);
   void      Rcb_Flags_SearchCoverSlv_Put (Rcb_t *, Boolean_t);
   Boolean_t Rcb_Flags_StereoChemistry_Get (Rcb_t *);
   void      Rcb_Flags_StereoChemistry_Put (Rcb_t *, Boolean_t);
   Boolean_t Rcb_Flags_StrategicBonds_Get (Rcb_t *);
   void      Rcb_Flags_StrategicBonds_Put (Rcb_t *, Boolean_t);
   Boolean_t Rcb_Flags_WorkerCurMerit_Get (Rcb_t *);
   void      Rcb_Flags_WorkerCurMerit_Put (Rcb_t *, Boolean_t);
   Boolean_t Rcb_Flags_WorkerLocalSel_Get (Rcb_t *);
   void      Rcb_Flags_WorkerLocalSel_Put (Rcb_t *, Boolean_t);
   Boolean_t Rcb_Flags_WorkerNBMerit_Get  (Rcb_t *);
   void      Rcb_Flags_WorkerNBMerit_Put  (Rcb_t *, Boolean_t);
   Boolean_t Rcb_Flags_WorkerPreferGlobal_Get (Rcb_t *);
   void      Rcb_Flags_WorkerPreferGlobal_Put (Rcb_t *, Boolean_t);
   Sling_t   Rcb_Goal_Get                 (Rcb_t *);
   void      Rcb_Goal_Put                 (Rcb_t *, Sling_t);
   Filecb_t  Rcb_IthFileCB_Get            (Rcb_t *, U8_t);
   U32_t     Rcb_LeapSize_Get             (Rcb_t *);
   void      Rcb_LeapSize_Put             (Rcb_t *, U32_t);
   U32_t     Rcb_MaxCycles_Get            (Rcb_t *);
   void      Rcb_MaxCycles_Put            (Rcb_t *, U32_t);
   U8_t      Rcb_MaxNodes_Get             (Rcb_t *);
   void      Rcb_MaxNodes_Put             (Rcb_t *, U8_t);
   U32_t     Rcb_MaxRuntime_Get           (Rcb_t *);
   void      Rcb_MaxRuntime_Put           (Rcb_t *, U32_t);
   U16_t     Rcb_MaxWorkers_Get           (Rcb_t *);
   void      Rcb_MaxWorkers_Put           (Rcb_t *, U16_t);
   U16_t     Rcb_MinWorkers_Get           (Rcb_t *);
   void      Rcb_MinWorkers_Put           (Rcb_t *, U16_t);
   String_t  Rcb_Name_Get                 (Rcb_t *);
   void      Rcb_Name_Put                 (Rcb_t *, String_t);
   U8_t      Rcb_NTCL_Get                 (Rcb_t *);
   void      Rcb_NTCL_Put                 (Rcb_t *, U8_t);
   U8_t      Rcb_ProcPerNode_Get          (Rcb_t *);
   void      Rcb_ProcPerNode_Put          (Rcb_t *, U8_t);
   U8_t      Rcb_RunType_Get              (Rcb_t *);
   void      Rcb_RunType_Put              (Rcb_t *, U8_t);
   Strategy_Params_t *Rcb_Strategy_Get    (Rcb_t *);
   String_t  Rcb_User_Get                 (Rcb_t *);
   void      Rcb_User_Put                 (Rcb_t *, String_t);
   U16_t     Rcb_WorkerMaxCycles_Get      (Rcb_t *);
   void      Rcb_WorkerMaxCycles_Put      (Rcb_t *, U16_t);
   U8_t      Rcb_WorkerMeritPrct_Get      (Rcb_t *);
   void      Rcb_WorkerMeritPrct_Put      (Rcb_t *, U8_t);
*/


#define Rcb_AddCycles_Get(rcb_p)\
  (rcb_p)->add_cycles
#define Rcb_AddCycles_Put(rcb_p, value)\
 (rcb_p)->add_cycles = (value)

#define Rcb_AddTime_Get(rcb_p)\
  (rcb_p)->add_time
#define Rcb_AddTime_Put(rcb_p, value)\
 (rcb_p)->add_time = (value)

#define Rcb_ChemistryTrace_Get(rcb_p)\
  (rcb_p)->chemistry_trace
#define Rcb_ChemistryTrace_Put(rcb_p, value)\
 (rcb_p)->chemistry_trace = (value)

#define Rcb_Comment_Get(rcb_p)\
  (rcb_p)->commentb
#define Rcb_Comment_Put(rcb_p, value)\
  (rcb_p)->commentb = (value)

#define Rcb_CoverPrct_Get(rcb_p)\
  (rcb_p)->cover_prct
#define Rcb_CoverPrct_Put(rcb_p, value)\
  (rcb_p)->cover_prct = (value)

#define Rcb_CumCycles_Get(rcb_p)\
  (rcb_p)->cum_cycles
#define Rcb_CumCycles_Put(rcb_p, value)\
  (rcb_p)->cum_cycles = (value)

#define Rcb_CumTimes_Get(rcb_p)\
  &((rcb_p)->cum_times)

#define Rcb_CyclesFirstSol_Get(rcb_p)\
  (rcb_p)->cycle_first_sol
#define Rcb_CyclesFirstSol_Put(rcb_p, value)\
  (rcb_p)->cycle_first_sol = (value)

#define Rcb_Date_Get(rcb_p)\
  (rcb_p)->date
#define Rcb_Date_Put(rcb_p, value)\
  (rcb_p)->date = (value)

#define Rcb_FileCBs_Get(rcb_p)\
  (rcb_p)->kbdfiles

#define Rcb_Flags_Get(rcb_p)\
  (rcb_p)->flags
#define Rcb_Flags_Put(rcb_p, value)\
  (rcb_p)->flags = (value)

#define Rcb_Flags_AndParallel_Get(rcb_p)\
  ((rcb_p)->flags & RcbM_AndParallelism ? TRUE : FALSE)
#define Rcb_Flags_AndParallel_Put(rcb_p, value)\
  { if ((value) == TRUE)\
    (rcb_p)->flags |= RcbM_AndParallelism;\
  else\
    (rcb_p)->flags &= ~RcbM_AndParallelism; }

#define Rcb_Flags_Background_Get(rcb_p)\
  ((rcb_p)->flags & RcbM_Background ? TRUE : FALSE)
#define Rcb_Flags_Background_Put(rcb_p, value)\
  { if ((value) == TRUE)\
    (rcb_p)->flags |= RcbM_Background;\
  else\
    (rcb_p)->flags &= ~RcbM_Background; }

#define Rcb_Flags_Continue_Get(rcb_p)\
  ((rcb_p)->flags & RcbM_Continuation ? TRUE : FALSE)
#define Rcb_Flags_Continue_Put(rcb_p, value)\
  { if ((value) == TRUE)\
    (rcb_p)->flags |= RcbM_Continuation;\
  else\
    (rcb_p)->flags &= ~RcbM_Continuation; }

#define Rcb_Flags_DisSharedSels_Get(rcb_p)\
  ((rcb_p)->flags & RcbM_DisSharedSels ? TRUE : FALSE)
#define Rcb_Flags_DisSharedSels_Put(rcb_p, value)\
  { if ((value) == TRUE)\
    (rcb_p)->flags |= RcbM_DisSharedSels;\
  else\
    (rcb_p)->flags &= ~RcbM_DisSharedSels; }

#define Rcb_Flags_EffortDis_Get(rcb_p)\
  ((rcb_p)->flags & RcbM_EffortDistribution ? TRUE : FALSE)
#define Rcb_Flags_EffortDis_Put(rcb_p, value)\
  { if ((value) == TRUE)\
    (rcb_p)->flags |= RcbM_EffortDistribution;\
  else\
    (rcb_p)->flags &= ~RcbM_EffortDistribution; }

#define Rcb_Flags_FirstSol_Get(rcb_p)\
  ((rcb_p)->flags & RcbM_FirstSol ? TRUE : FALSE)
#define Rcb_Flags_FirstSol_Put(rcb_p, value)\
  { if ((value) == TRUE)\
    (rcb_p)->flags |= RcbM_FirstSol;\
  else\
    (rcb_p)->flags &= ~RcbM_FirstSol; }

#define Rcb_Flags_LibsChanged_Get(rs_p)\
  ((rs_p)->flags & RcbM_LibsChanged ? TRUE : FALSE)
#define Rcb_Flags_LibsChanged_Put(rs_p, value)\
  { if ((value) == TRUE)\
    (rs_p)->flags |= RcbM_LibsChanged;\
  else\
    (rs_p)->flags &= ~RcbM_LibsChanged; }

#define Rcb_Flags_LibsLoaded_Get(rs_p)\
  ((rs_p)->flags & RcbM_LibsLoaded ? TRUE : FALSE)
#define Rcb_Flags_LibsLoaded_Put(rs_p, value)\
  { if ((value) == TRUE)\
    (rs_p)->flags |= RcbM_LibsLoaded;\
  else\
    (rs_p)->flags &= ~RcbM_LibsLoaded; }

#define Rcb_Flags_MasterSelNB_Get(rcb_p)\
  ((rcb_p)->flags & RcbM_MasterSelNB ? TRUE : FALSE)
#define Rcb_Flags_MasterSelNB_Put(rcb_p, value)\
  { if ((value) == TRUE)\
    (rcb_p)->flags |= RcbM_MasterSelNB;\
  else\
    (rcb_p)->flags &= ~RcbM_MasterSelNB; }

#define Rcb_Flags_PreserveStructures_Get(rcb_p)\
  ((rcb_p)->flags & RcbM_PreserveStructures ? TRUE : FALSE)
#define Rcb_Flags_PreserveStructures_Put(rcb_p, value)\
  { if ((value) == TRUE)\
    (rcb_p)->flags |= RcbM_PreserveStructures;\
  else\
    (rcb_p)->flags &= ~RcbM_PreserveStructures; }

#define Rcb_Flags_Restart_Get(rcb_p)\
  ((rcb_p)->flags & RcbM_Restart ? TRUE : FALSE)
#define Rcb_Flags_Restart_Put(rcb_p, value)\
  { if ((value) == TRUE)\
    (rcb_p)->flags |= RcbM_Restart;\
  else\
    (rcb_p)->flags &= ~RcbM_Restart; }

#define Rcb_Flags_SaveMerits_Get(rcb_p)\
  ((rcb_p)->flags & RcbM_SaveMerits ? TRUE : FALSE)
#define Rcb_Flags_SaveMerits_Put(rcb_p, value)\
  { if ((value) == TRUE)\
    (rcb_p)->flags |= RcbM_SaveMerits;\
  else\
    (rcb_p)->flags &= ~RcbM_SaveMerits; }

#define Rcb_Flags_SaveStatus_Get(rcb_p)\
  ((rcb_p)->flags & RcbM_SaveStatus ? TRUE : FALSE)
#define Rcb_Flags_SaveStatus_Put(rcb_p, value)\
  { if ((value) == TRUE)\
    (rcb_p)->flags |= RcbM_SaveStatus;\
  else\
    (rcb_p)->flags &= ~RcbM_SaveStatus; }

#define Rcb_Flags_SaveTrace_Get(rcb_p)\
  ((rcb_p)->flags & RcbM_SaveTrace ? TRUE : FALSE)
#define Rcb_Flags_SaveTrace_Put(rcb_p, value)\
  { if ((value) == TRUE)\
    (rcb_p)->flags |= RcbM_SaveTrace;\
  else\
    (rcb_p)->flags &= ~RcbM_SaveTrace; }

#define Rcb_Flags_SearchCoverDev_Get(rcb_p)\
  ((rcb_p)->flags & RcbM_SearchCoverDev ? TRUE : FALSE)
#define Rcb_Flags_SearchCoverDev_Put(rcb_p, value)\
  { if ((value) == TRUE)\
    (rcb_p)->flags |= RcbM_SearchCoverDev;\
  else\
    (rcb_p)->flags &= ~RcbM_SearchCoverDev; }

#define Rcb_Flags_SearchCoverSlv_Get(rcb_p)\
  ((rcb_p)->flags & RcbM_SearchCoverSlv ? TRUE : FALSE)
#define Rcb_Flags_SearchCoverSlv_Put(rcb_p, value)\
  { if ((value) == TRUE)\
    (rcb_p)->flags |= RcbM_SearchCoverSlv;\
  else\
    (rcb_p)->flags &= ~RcbM_SearchCoverSlv; }

#define Rcb_Flags_StereoChemistry_Get(rcb_p)\
  ((rcb_p)->flags & RcbM_StereoChemistry ? TRUE : FALSE)
#define Rcb_Flags_StereoChemistry_Put(rcb_p, value)\
  { if ((value) == TRUE)\
    (rcb_p)->flags |= RcbM_StereoChemistry;\
  else\
    (rcb_p)->flags &= ~RcbM_StereoChemistry; }

#define Rcb_Flags_StrategicBonds_Get(rcb_p)\
  ((rcb_p)->flags & RcbM_StrategicBonds ? TRUE : FALSE)
#define Rcb_Flags_StrategicBonds_Put(rcb_p, value)\
  { if ((value) == TRUE)\
    (rcb_p)->flags |= RcbM_StrategicBonds;\
  else\
    (rcb_p)->flags &= ~RcbM_StrategicBonds; }

#define Rcb_Flags_WorkerCurMerit_Get(rcb_p)\
  ((rcb_p)->flags & RcbM_WorkerCurMerit ? TRUE : FALSE)
#define Rcb_Flags_WorkerCurMerit_Put(rcb_p, value)\
  { if ((value) == TRUE)\
    (rcb_p)->flags |= RcbM_WorkerCurMerit;\
  else\
    (rcb_p)->flags &= ~RcbM_WorkerCurMerit; }

#define Rcb_Flags_WorkerLocalSel_Get(rcb_p)\
  ((rcb_p)->flags & RcbM_WorkerLocalSel ? TRUE : FALSE)
#define Rcb_Flags_WorkerLocalSel_Put(rcb_p, value)\
  { if ((value) == TRUE)\
    (rcb_p)->flags |= RcbM_WorkerLocalSel;\
  else\
    (rcb_p)->flags &= ~RcbM_WorkerLocalSel; }

#define Rcb_Flags_WorkerNBMerit_Get(rcb_p)\
  ((rcb_p)->flags & RcbM_WorkerNBMerit ? TRUE : FALSE)
#define Rcb_Flags_WorkerNBMerit_Put(rcb_p, value)\
  { if ((value) == TRUE)\
    (rcb_p)->flags |= RcbM_WorkerNBMerit;\
  else\
    (rcb_p)->flags &= ~RcbM_WorkerNBMerit; }

#define Rcb_Flags_WorkerPreferGlobal_Get(rcb_p)\
  ((rcb_p)->flags & RcbM_WorkerPreferGlobal ? TRUE : FALSE)
#define Rcb_Flags_WorkerPreferGlobal_Put(rcb_p, value)\
  { if ((value) == TRUE)\
    (rcb_p)->flags |= RcbM_WorkerPreferGlobal;\
  else\
    (rcb_p)->flags &= ~RcbM_WorkerPreferGlobal; }

#define Rcb_Goal_Get(rcb_p)\
  (rcb_p)->goalb
#define Rcb_Goal_Put(rcb_p, value)\
  (rcb_p)->goalb = (value)

#define Rcb_IthFileCB_Get(rcb_p, index)\
  (rcb_p)->kbdfiles[index]

#define Rcb_LeapSize_Get(rcb_p)\
  (rcb_p)->leap_size
#define Rcb_LeapSize_Put(rcb_p, value)\
  (rcb_p)->leap_size = (value)

#define Rcb_MaxCycles_Get(rcb_p)\
  (rcb_p)->max_cycles
#define Rcb_MaxCycles_Put(rcb_p, value)\
  (rcb_p)->max_cycles = (value)

#define Rcb_MaxNodes_Get(rcb_p)\
  (rcb_p)->max_nodes
#define Rcb_MaxNodes_Put(rcb_p, value)\
  (rcb_p)->max_nodes = (value)

/* Used to be runtim and $runtim: */
#define Rcb_MaxRuntime_Get(rcb_p)\
  (rcb_p)->max_runtime
#define Rcb_MaxRuntime_Put(rcb_p, value)\
  (rcb_p)->max_runtime = (value)

#define Rcb_MaxWorkers_Get(rcb_p)\
  (rcb_p)->max_workers
#define Rcb_MaxWorkers_Put(rcb_p, value)\
  (rcb_p)->max_workers = (value)

#define Rcb_MinWorkers_Get(rcb_p)\
  (rcb_p)->min_workers
#define Rcb_MinWorkers_Put(rcb_p, value)\
  (rcb_p)->min_workers = (value)

#define Rcb_Name_Get(rcb_p)\
  (rcb_p)->nameb
#define Rcb_Name_Put(rcb_p, value)\
  (rcb_p)->nameb = (value)

#define Rcb_NTCL_Get(rcb_p)\
  (rcb_p)->ntcl
#define Rcb_NTCL_Put(rcb_p, value)\
  (rcb_p)->ntcl = (value)

#define Rcb_ProcPerNode_Get(rcb_p)\
  (rcb_p)->proc_per_node
#define Rcb_ProcPerNode_Put(rcb_p, value)\
  (rcb_p)->proc_per_node = (value)

#define Rcb_RunType_Get(rcb_p)\
  (rcb_p)->run_type
#define Rcb_RunType_Put(rcb_p, value)\
  (rcb_p)->run_type = (value)

#define Rcb_Strategy_Get(rcb_p)\
  &(rcb_p)->strategyb

#define Rcb_TimeFirstSol_Get(rcb_p)\
  (rcb_p)->time_first_sol
#define Rcb_TimeFirstSol_Put(rcb_p, value)\
  (rcb_p)->time_first_sol = (value)

#define Rcb_User_Get(rcb_p)\
  (rcb_p)->userb
#define Rcb_User_Put(rcb_p, value)\
  (rcb_p)->userb = (value)

#define Rcb_WorkerMaxCycles_Get(rcb_p)\
  (rcb_p)->worker_maxcycles
#define Rcb_WorkerMaxCycles_Put(rcb_p, value)\
  (rcb_p)->worker_maxcycles = (value)

#define Rcb_WorkerMeritPrct_Get(rcb_p)\
  (rcb_p)->worker_merit_prct
#define Rcb_WorkerMeritPrct_Put(rcb_p, value)\
  (rcb_p)->worker_merit_prct = (value)

/** End of Field Access Macros for a Rcb_t **/

/*** Macros ***/

/*** Routine Prototypes ***/

void FileNames_Complete (Filecb_t *, String_t);
void Rcb_Copy           (Rcb_t *, const Rcb_t *);
void Rcb_Destroy        (Rcb_t *);
void Rcb_Dump           (Rcb_t *, FileDsc_t *);
void Rcb_Init           (Rcb_t *, Boolean_t);
void Rcb_Load           (Rcb_t *, char *, Boolean_t);

void RunStats_Destroy   (RunStats_t *);
void RunStats_Dump      (RunStats_t *, FileDsc_t *, Boolean_t);
void RunStats_Init      (RunStats_t *);


/*** Global Variables ***/

#ifdef RCB_GLOBALS
  Rcb_t                  GGoalRcb;
  RunStats_t             GRunStats;
#else
  extern  Rcb_t          GGoalRcb;
  extern  RunStats_t     GRunStats;
#endif

/* End of Rcb.H */
#endif
