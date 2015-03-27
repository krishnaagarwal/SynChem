/******************************************************************************
*
*  Copyright (C) 1991-1996 Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     RCB.C
*
*    This module contains the code for the Run Control Block abstraction,
*    and the run statistics.
*
*  Creation Date:
*
*    01-Jan-1991
*
*  Routines:
*
*    FileNames_Complete
*    Rcb_Copy
*    Rcb_Destroy
*    Rcb_Dump
*    Rcb_Init
*    Rcb_Load
*
*    RunStats_Destroy
*    RunStats_Dump
*    RunStats_Init
*
*
*
*  Authors:
*
*    Daren Krebsbach
*    Shu Cheung
*    Tito Autrey (rewritten based on others PL/I code)
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 23-Sep-97  Krebsbach  Combined sequential and distributed versions
*                       of SYNCHEM into single file, making many changes to
*                       data structures and function prototypes.
* 02-Oct-96  Krebsbach  Added routine Status_File_Peek.
* 26-Sep-96  Krebsbach  Created RunStats_t data structure.  Added functions
*                       RunStats_Dump and RunStats_Init.  
* 23-Sep-96  Krebsbach  Added functions Rcb_Init and Rcb_Load.
* 14-Sep-95  Cheng      Added functions Rcb_AllowVisit_Get 
*                       and Rcb_AllowVisit_Put.
*
******************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

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

#define RCB_GLOBALS 1
#ifndef _H_RCB_
#include "rcb.h"
#endif
#undef RCB_GLOBALS

#ifndef _H_EXTERN_
#include "extern.h"
#endif

char *Sensible_Time_Format (Time_t, Boolean_t);

/****************************************************************************
*
*  Function Name:                 FileNames_Complete
*
*    Completes the filenames for submit, status, path and trace files using
*    the directories specified in the file control blocks, the compound name,
*    and the proper extension.
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
*    None.
*
******************************************************************************/
void FileNames_Complete 
  (
  Filecb_t     *fcbs,
  String_t      name
  )
{

  String_Concat_c (&FileCB_FileStr_Get (fcbs[FCB_IND_PATH]), FCB_PATH_SEP);
  String_Concat (&FileCB_FileStr_Get (fcbs[FCB_IND_PATH]), name);
  String_Concat_c (&FileCB_FileStr_Get (fcbs[FCB_IND_PATH]), FCB_EXT_PATH);

  String_Concat_c (&FileCB_FileStr_Get (fcbs[FCB_IND_STAT]), FCB_PATH_SEP);
  String_Concat (&FileCB_FileStr_Get (fcbs[FCB_IND_STAT]), name);
  String_Concat_c (&FileCB_FileStr_Get (fcbs[FCB_IND_STAT]), FCB_EXT_STAT);

  String_Concat_c (&FileCB_FileStr_Get (fcbs[FCB_IND_SBMT]), FCB_PATH_SEP);
  String_Concat (&FileCB_FileStr_Get (fcbs[FCB_IND_SBMT]), name);
  String_Concat_c (&FileCB_FileStr_Get (fcbs[FCB_IND_SBMT]), FCB_EXT_SBMT);

  String_Concat_c (&FileCB_FileStr_Get (fcbs[FCB_IND_TRCE]), FCB_PATH_SEP);
  String_Concat (&FileCB_FileStr_Get (fcbs[FCB_IND_TRCE]), name);
  String_Concat_c (&FileCB_FileStr_Get (fcbs[FCB_IND_TRCE]), FCB_EXT_TRCE);

  if (!FileCB_Changed_Get (fcbs[FCB_IND_FSEL]))
    {
    String_Concat_c (&FileCB_FileStr_Get (fcbs[FCB_IND_FSEL]), FCB_PATH_SEP);
    String_Concat (&FileCB_FileStr_Get (fcbs[FCB_IND_FSEL]), name);
    String_Concat_c (&FileCB_FileStr_Get (fcbs[FCB_IND_FSEL]), FCB_EXT_FSEL);
    FileCB_Changed_Put (fcbs[FCB_IND_FSEL], TRUE);
    }

  if (!FileCB_Changed_Get (fcbs[FCB_IND_COVR]))
    {
    String_Concat_c (&FileCB_FileStr_Get (fcbs[FCB_IND_COVR]), FCB_PATH_SEP);
    String_Concat (&FileCB_FileStr_Get (fcbs[FCB_IND_COVR]), name);
    String_Concat_c (&FileCB_FileStr_Get (fcbs[FCB_IND_COVR]), FCB_EXT_COVR);
    FileCB_Changed_Put (fcbs[FCB_IND_COVR], TRUE);
    }

  return ;
}
/* End of FileNames_Complete */

/****************************************************************************
*
*  Function Name:                 Rcb_Copy
*
*    This function copies the source rcb into the destination rcb.  New memory
*    is allocated for the strings in the run control block including those in 
*    the file control blocks.
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
*    None.
*
******************************************************************************/
void Rcb_Copy 
  (
  Rcb_t         *dest_rcb_p,
  const Rcb_t   *src_rcb_p
  )
{
  U8_t          fcb_i;

  *dest_rcb_p = *src_rcb_p;

  Rcb_Comment_Put (dest_rcb_p, String_Copy (Rcb_Comment_Get (src_rcb_p)));
  Rcb_Goal_Put (dest_rcb_p, Sling_Copy (Rcb_Goal_Get (src_rcb_p)));
  Rcb_Name_Put (dest_rcb_p, String_Copy (Rcb_Name_Get (src_rcb_p)));
  Rcb_User_Put (dest_rcb_p, String_Copy (Rcb_User_Get (src_rcb_p)));

  for (fcb_i = 0; fcb_i < FCB_IND_NUMOF; fcb_i++)
    {
    FileCB_DirStr_Put (Rcb_IthFileCB_Get (dest_rcb_p, fcb_i), 
      String_Copy (FileCB_DirStr_Get (Rcb_IthFileCB_Get (src_rcb_p, fcb_i))));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (dest_rcb_p, fcb_i), 
      String_Copy (FileCB_FileStr_Get (Rcb_IthFileCB_Get (src_rcb_p, fcb_i))));
    }

  return ;
}
/* End of Rcb_Copy */

/****************************************************************************
*
*  Function Name:                 Rcb_Destroy
*
*    This function destroys the strings in the run control block
*    including those in the file control blocks.
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
*    None.
*
******************************************************************************/
void Rcb_Destroy 
  (
  Rcb_t         *rcb_p
  )
{
  U8_t          fcb_i;

  String_Destroy (Rcb_Comment_Get (rcb_p));
  String_Value_Put (Rcb_Comment_Get (rcb_p), NULL);
  String_Length_Put (Rcb_Comment_Get (rcb_p), 0);

  Sling_Destroy (Rcb_Goal_Get (rcb_p));
  Sling_Name_Put (Rcb_Goal_Get (rcb_p), NULL);
  Sling_Length_Put (Rcb_Goal_Get (rcb_p), 0);

  String_Destroy (Rcb_Name_Get (rcb_p));
  String_Value_Put (Rcb_Name_Get (rcb_p), NULL);
  String_Length_Put (Rcb_Name_Get (rcb_p), 0);

  String_Destroy (Rcb_User_Get (rcb_p));
  String_Value_Put (Rcb_User_Get (rcb_p), NULL);
  String_Length_Put (Rcb_User_Get (rcb_p), 0);

  for (fcb_i = 0; fcb_i < FCB_IND_NUMOF; fcb_i++)
    {
    String_Destroy (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, fcb_i)));
    String_Value_Put (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, fcb_i)), 
      NULL);
    String_Length_Put (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, fcb_i)),
      0);

    String_Destroy (FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, fcb_i)));
    String_Value_Put (FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, fcb_i)), 
      NULL);
    String_Length_Put (FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, fcb_i)), 
      0);
    }

  return ;
}
/* End of Rcb_Destroy */

/****************************************************************************
*
*  Function Name:                 Rcb_Dump
*
*    This routine prints a formatted dump of the Run Control Block.
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
void Rcb_Dump
  (
  Rcb_t         *rcb_p,                      /* Instance to dump */
  FileDsc_t     *filed_p                     /* File to dump to */
  )
{
  FILE          *f;                          /* Temporary */

  f = IO_FileHandle_Get (filed_p);
  if (rcb_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL RCB\n");
    return;
    }

  fprintf (f, "Dump of RCB\n");
  fprintf (f, "  Directories:\n");
  fprintf (f, "%-25s  %s\n", "    Available Compounds:", 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_AVLC))));
  fprintf (f, "%-25s  %s\n", "    Functional Groups:", 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_FNGP))));
  fprintf (f, "%-25s  %s\n", "    Reaction KB:", 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p,
    FCB_IND_RXNS))));
  fprintf (f, "%-25s  %s\n", "    Paths:", 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p,
    FCB_IND_PATH))));
  fprintf (f, "%-25s  %s\n", "    Status:", 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_STAT))));
  fprintf (f, "%-25s  %s\n", "    Submission:", 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_SBMT))));
  fprintf (f, "%-25s  %s\n", "    Template:", 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_TMPL))));
  fprintf (f, "%-25s  %s\n", "    Trace:", 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_TRCE))));
  fprintf (f, "%-25s  %s\n", "    Force Selection:", 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_FSEL))));
  fprintf (f, "%-25s  %s\n", "    Search Space Cover:", 
    (char *) String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
    FCB_IND_COVR))));


  fprintf (f, "%-16s  %s\n", "  User name:", 
    String_Value_Get (Rcb_User_Get (rcb_p)));
  fprintf (f, "%-16s  %s\n", "  Target name:", 
    String_Value_Get (Rcb_Name_Get (rcb_p)));
  fprintf (f, "%-16s  %s\n", "  Target sling:", 
    Sling_Name_Get (Rcb_Goal_Get (rcb_p)));
  fprintf (f, "%-16s  %s\n", "  Comment:", 
    String_Value_Get (Rcb_Comment_Get (rcb_p)));
  fprintf (f, "%-16s  %s", "  Date of run:", 
    ctime (&((Rcb_Date_Get (rcb_p)).tv_sec)));
  fprintf (f, "%-16s  %5lu     %-16s  %5lu minutes\n", "  Max cycles:",
    Rcb_MaxCycles_Get (rcb_p), "  Max runtime:", Rcb_MaxRuntime_Get (rcb_p));
  fprintf (f, "%-16s  %8s   %-16s  %8s\n", "  Restart:", 
    (Rcb_Flags_Restart_Get (rcb_p)) ? "enabled" : "disabled", "Background:", 
    (Rcb_Flags_Background_Get (rcb_p)) ? "enabled" : "disabled");
  fprintf (f, "%-16s  %1hu\n", "  Tracing level:", 
    Rcb_ChemistryTrace_Get (rcb_p));
  if (Rcb_Flags_EffortDis_Get (rcb_p))
    fprintf (f, "%-16s  %s     %-16s  %hu\n", "  Effort Dist:", "enabled", 
      "  NTCL:", Rcb_NTCL_Get (rcb_p));
  else
    fprintf (f, "%-16s  %s     %-16s  %hu\n", "  Effort Dist:", "disabled", 
      "  NTCL:", Rcb_NTCL_Get (rcb_p));

  if (Rcb_Flags_Continue_Get (rcb_p))
    {
    fprintf (f, "%-16s  %lu\n", "  Leapsize:", Rcb_LeapSize_Get (rcb_p));
    }

  if (Rcb_Flags_FirstSol_Get (rcb_p))
    {
    fprintf (f, "  End search after first solution found:\n");
    fprintf (f, "%-24s  %5lu     %-24s  %5lu minutes\n", 
      "    Additional cycles:", Rcb_AddCycles_Get (rcb_p), 
      "    Additional time:", Rcb_AddTime_Get (rcb_p));
    }

  if (Rcb_Flags_SearchCoverDev_Get (rcb_p) 
      || Rcb_Flags_SearchCoverSlv_Get (rcb_p))
    {
    fprintf (f, "  Seq Cover:  %s\n    With:  ", 
      (char *) String_Value_Get (FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, 
      FCB_IND_COVR))));

    if (Rcb_Flags_SearchCoverSlv_Get (rcb_p))
      fprintf (f, "  solved coverage");

    if (Rcb_Flags_SearchCoverDev_Get (rcb_p))
      fprintf (f, "  developed coverage (%u%%)", Rcb_CoverPrct_Get (rcb_p));

    fprintf (f, "\n");
    }

  if (Rcb_RunType_Get (rcb_p) == RCB_TYPE_SEQ)
    fprintf (f, "  Run type:  Sequential\n");
  else if (Rcb_RunType_Get (rcb_p) == RCB_TYPE_SEQ_FORCESEL)
    fprintf (f, "  Run type:  Forced Selection\n");
  else if (Rcb_RunType_Get (rcb_p) == RCB_TYPE_DIS_LIN)
    fprintf (f, "  Run type:  Distributed, Linda version\n");
  else
    fprintf (f, "  Run type:  Unknown Type\n");

  if (Rcb_RunType_Get (rcb_p) >= RCB_TYPE_DIS_START)
    {
    fprintf (f, "  Distributed run parameters:\n");

    fprintf (f, "    Workers: min %2u max %2u, Procs:  %1hu per %1hu nodes\n", 
      Rcb_MaxWorkers_Get (rcb_p), Rcb_MinWorkers_Get (rcb_p), 
      Rcb_ProcPerNode_Get (rcb_p), Rcb_MaxNodes_Get (rcb_p));

    fprintf (f, "    Master Selection:  %s  %s\n", 
      Rcb_Flags_MasterSelNB_Get (rcb_p) ? "Next Best" : "Selected Sibling",
      Rcb_Flags_AndParallel_Get (rcb_p) ? "And Parallelism" : "");

    fprintf (f, "    Workers, max cycles:  %u\n      %s %s %s %s %s (%hu%%)\n",
      Rcb_WorkerMaxCycles_Get (rcb_p),
      Rcb_Flags_WorkerLocalSel_Get (rcb_p) ? "LocalSelect" : "",
      Rcb_Flags_DisSharedSels_Get (rcb_p) ? "ShareSels" : "",
      Rcb_Flags_WorkerPreferGlobal_Get (rcb_p) ? "PreferGlobal" : "",
      Rcb_Flags_WorkerCurMerit_Get (rcb_p) ? "CurrMerit" : "",
      Rcb_Flags_WorkerNBMerit_Get (rcb_p) ? "NextBestMerit" : "",
      Rcb_WorkerMeritPrct_Get (rcb_p));

    }

  return;
}
/* End of Rcb_Dump */

/****************************************************************************
*
*  Function Name:                 Rcb_Init
*
*    This function initializes the run control block with default
*    values for sequential runs.
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
*    None.
*
******************************************************************************/
void Rcb_Init 
  (
  Rcb_t         *rcb_p,
  Boolean_t      distributed_run
  )
{
  String_t       empty_sling;
  char          *login_name;

  empty_sling = String_Create (RCB_DFLT_GOAL, 0);
  Rcb_Goal_Put (rcb_p, String2Sling (empty_sling));
  String_Destroy (empty_sling);

  Rcb_Name_Put (rcb_p, String_Create (RCB_DFLT_CMP_NAME, 0));
  Rcb_Comment_Put (rcb_p, String_Create (RCB_DFLT_COMMENT, 0));
  Rcb_Date_Put (rcb_p, Syn_Time_Get (TRUE));

  login_name = getlogin ();
/*  
  if (login_name == NULL)
    login_name = getpwuid(getuid())->pw_name;
*/
  Rcb_User_Put (rcb_p, String_Create (login_name, 0));
  Rcb_MaxCycles_Put (rcb_p, RCB_DFLT_MAX_CYCLES);

  Rcb_Flags_Put (rcb_p, 0);

  Rcb_Flags_PreserveStructures_Put (rcb_p, RCB_DFLT_FPRES_STRCT);
  Rcb_Flags_StereoChemistry_Put (rcb_p, RCB_DFLT_FSTEREO_CHEM);
  Rcb_Flags_StrategicBonds_Put (rcb_p, RCB_DFLT_FSTRAT_BONDS);

  Rcb_CoverPrct_Put (rcb_p, RCB_DFLT_COVER_DEV_PRCT);
  Rcb_Flags_SearchCoverDev_Put (rcb_p, RCB_DFLT_FSEARCH_COVER_DEV);
  Rcb_Flags_SearchCoverSlv_Put (rcb_p, RCB_DFLT_FSEARCH_COVER_SLV);

  Rcb_Flags_FirstSol_Put (rcb_p, RCB_DFLT_FFIRST_SOL);
  Rcb_AddCycles_Put (rcb_p, RCB_DFLT_ADD_CYCLES); 
  Rcb_AddTime_Put (rcb_p, RCB_DFLT_ADD_TIME); 
  Rcb_Flags_Continue_Put (rcb_p, RCB_DFLT_FCONTINUE);
  Rcb_LeapSize_Put (rcb_p, RCB_DFLT_LEAPSIZE);
  Rcb_Flags_Restart_Put (rcb_p, RCB_DFLT_FRESTART);

  Rcb_Flags_SaveStatus_Put (rcb_p, RCB_DFLT_FSAVE_STATUS);
  Rcb_Flags_SaveTrace_Put (rcb_p, RCB_DFLT_FSAVE_TRACE);
  Rcb_Flags_SaveMerits_Put (rcb_p, RCB_DFLT_FSAVE_MERITS);

  Rcb_ChemistryTrace_Put (rcb_p, RCB_DFLT_CHEMTRACE); 

  Rcb_Flags_LibsChanged_Put (rcb_p, FALSE);
  Rcb_Flags_LibsLoaded_Put (rcb_p, FALSE);


 if (distributed_run)
    {
    Rcb_RunType_Put (rcb_p, RCB_DIS_RUNTYPE);
    Rcb_Flags_Background_Put (rcb_p, RCB_DIS_BACKGROUND);
    Rcb_Flags_EffortDis_Put (rcb_p, RCB_DIS_EFFORTDIS);
    Rcb_NTCL_Put (rcb_p, RCB_DIS_NTCL);
    Rcb_MaxRuntime_Put (rcb_p, RCB_DIS_MAX_RUNTIME);
    Rcb_MaxNodes_Put (rcb_p, RCB_DIS_MAX_NODES);
    Rcb_MaxWorkers_Put (rcb_p, RCB_DIS_MAX_WORKERS);
    Rcb_MinWorkers_Put (rcb_p, RCB_DIS_MIN_WORKERS);
    Rcb_ProcPerNode_Put (rcb_p, RCB_DIS_PROC_PER_NODE);

    Rcb_Flags_MasterSelNB_Put (rcb_p, RCB_DIS_MASTER_SEL_NB);
    Rcb_Flags_AndParallel_Put (rcb_p, RCB_DIS_ANDPARALLEL);

    Rcb_Flags_WorkerLocalSel_Put (rcb_p, RCB_DIS_WORKER_LOCAL_SEL);
    Rcb_Flags_DisSharedSels_Put (rcb_p, RCB_DIS_DIS_SHARED_SELS);
    Rcb_WorkerMaxCycles_Put (rcb_p, RCB_DIS_WORKER_MAX_CYCLES);
    Rcb_Flags_WorkerCurMerit_Put (rcb_p, RCB_DIS_WORKER_CURMERIT);
    Rcb_Flags_WorkerNBMerit_Put (rcb_p, RCB_DIS_WORKER_NBMERIT);
    Rcb_WorkerMeritPrct_Put (rcb_p, RCB_DIS_WORKER_MERIT_PRCT);
    Rcb_Flags_WorkerPreferGlobal_Put (rcb_p, RCB_DIS_WORKER_PREF_GLOBAL);

    /*  Initialize the file control blocks.  Both the directory
	and file paths are initialized with the directory string, so that
	the file name can be appended to the directory, once it is known.
	The file magic number and type are ignored, and the version is 
	initialized to zero.
    */ 
    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_AVLC), 
      String_Create (FCB_DISDIR_AVLC(""), 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_AVLC), 
      String_Create (FCB_DISDIR_AVLC(""), MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_AVLC), FALSE);
    FileCB_MagicNum_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_AVLC), 0);
    FileCB_Type_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_AVLC), 0);
    FileCB_Version_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_AVLC), 0);

    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FNGP), 
      String_Create (FCB_DISDIR_FNGP(""), 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FNGP), 
      String_Create (FCB_DISDIR_FNGP(""), MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FNGP), FALSE);
    FileCB_MagicNum_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FNGP), 0);
    FileCB_Type_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FNGP), 0);
    FileCB_Version_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FNGP), 0);

    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_RXNS), 
      String_Create (FCB_DISDIR_RXNS(""), 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_RXNS), 
      String_Create (FCB_DISDIR_RXNS(""), MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_RXNS), FALSE);
    FileCB_MagicNum_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_RXNS), 0);
    FileCB_Type_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_RXNS), 0);
    FileCB_Version_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_RXNS), 0);

    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TMPL), 
      String_Create (FCB_DISDIR_TMPL(""), 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TMPL), 
      String_Create (FCB_DISDIR_TMPL(""), MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TMPL), FALSE);
    FileCB_MagicNum_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TMPL), 0);
    FileCB_Type_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TMPL), 0);
    FileCB_Version_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TMPL), 0);

    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_PATH), 
      String_Create (FCB_DISDIR_PATH(""), 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_PATH), 
      String_Create (FCB_DISDIR_PATH(""), MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_PATH), FALSE);
    FileCB_MagicNum_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_PATH), 0);
    FileCB_Type_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_PATH), 0);
    FileCB_Version_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_PATH), 0);

    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT), 
      String_Create (FCB_DISDIR_STAT(""), 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT), 
      String_Create (FCB_DISDIR_STAT(""), MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT), FALSE);
    FileCB_MagicNum_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT), 0);
    FileCB_Type_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT), 0);
    FileCB_Version_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT), 0);

    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_SBMT), 
      String_Create (FCB_DISDIR_SBMT(""), 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_SBMT), 
      String_Create (FCB_DISDIR_SBMT(""), MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_SBMT), FALSE);
    FileCB_MagicNum_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_SBMT), 0);
    FileCB_Type_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_SBMT), 0);
    FileCB_Version_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_SBMT), 0);

    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TRCE), 
      String_Create (FCB_DISDIR_TRCE(""), 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TRCE), 
      String_Create (FCB_DISDIR_TRCE(""), MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TRCE), FALSE);
    FileCB_MagicNum_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TRCE), 0);
    FileCB_Type_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TRCE), 0);
    FileCB_Version_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TRCE), 0);

    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_COVR), 
      String_Create (FCB_DISDIR_COVR(""), 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_COVR), 
      String_Create (FCB_DISDIR_COVR(""), MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_COVR), FALSE);
    FileCB_MagicNum_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_COVR), 0);
    FileCB_Type_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_COVR), 0);
    FileCB_Version_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_COVR), 0);

    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FSEL), 
      String_Create (FCB_DISDIR_FSEL(""), 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FSEL), 
      String_Create (FCB_DISDIR_FSEL(""), MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FSEL), FALSE);
    FileCB_MagicNum_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FSEL), 0);
    FileCB_Type_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FSEL), 0);
    FileCB_Version_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FSEL), 0);
    }

  else
    {
    Rcb_RunType_Put (rcb_p, RCB_SEQ_RUNTYPE);
    Rcb_Flags_Background_Put (rcb_p, RCB_SEQ_BACKGROUND);
    Rcb_Flags_EffortDis_Put (rcb_p, RCB_SEQ_EFFORTDIS);
    Rcb_NTCL_Put (rcb_p, RCB_SEQ_NTCL);
    Rcb_MaxRuntime_Put (rcb_p, RCB_SEQ_MAX_RUNTIME);

    Rcb_MaxNodes_Put (rcb_p, RCB_SEQ_MAX_NODES);
    Rcb_MaxWorkers_Put (rcb_p, RCB_SEQ_MAX_WORKERS);
    Rcb_MinWorkers_Put (rcb_p, RCB_SEQ_MIN_WORKERS);
    Rcb_ProcPerNode_Put (rcb_p, RCB_SEQ_PROC_PER_NODE);

    Rcb_Flags_MasterSelNB_Put (rcb_p, RCB_SEQ_MASTER_SEL_NB);
    Rcb_Flags_AndParallel_Put (rcb_p, RCB_SEQ_ANDPARALLEL);

    Rcb_Flags_WorkerLocalSel_Put (rcb_p, RCB_SEQ_WORKER_LOCAL_SEL);
    Rcb_Flags_DisSharedSels_Put (rcb_p, RCB_SEQ_DIS_SHARED_SELS);
    Rcb_WorkerMaxCycles_Put (rcb_p, RCB_SEQ_WORKER_MAX_CYCLES);
    Rcb_Flags_WorkerCurMerit_Put (rcb_p, RCB_SEQ_WORKER_CURMERIT);
    Rcb_Flags_WorkerNBMerit_Put (rcb_p, RCB_SEQ_WORKER_NBMERIT);
    Rcb_WorkerMeritPrct_Put (rcb_p, RCB_SEQ_WORKER_MERIT_PRCT);
    Rcb_Flags_WorkerPreferGlobal_Put (rcb_p, RCB_SEQ_WORKER_PREF_GLOBAL);

    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_AVLC), 
      String_Create (FCB_SEQDIR_AVLC(""), 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_AVLC), 
      String_Create (FCB_SEQDIR_AVLC(""), MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_AVLC), FALSE);
    FileCB_MagicNum_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_AVLC), 0);
    FileCB_Type_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_AVLC), 0);
    FileCB_Version_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_AVLC), 0);

    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FNGP), 
      String_Create (FCB_SEQDIR_FNGP(""), 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FNGP), 
      String_Create (FCB_SEQDIR_FNGP(""), MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FNGP), FALSE);
    FileCB_MagicNum_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FNGP), 0);
    FileCB_Type_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FNGP), 0);
    FileCB_Version_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FNGP), 0);

    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_RXNS), 
      String_Create (FCB_SEQDIR_RXNS(""), 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_RXNS), 
      String_Create (FCB_SEQDIR_RXNS(""), MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_RXNS), FALSE);
    FileCB_MagicNum_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_RXNS), 0);
    FileCB_Type_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_RXNS), 0);
    FileCB_Version_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_RXNS), 0);

    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TMPL), 
      String_Create (FCB_SEQDIR_TMPL(""), 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TMPL), 
      String_Create (FCB_SEQDIR_TMPL(""), MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TMPL), FALSE);
    FileCB_MagicNum_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TMPL), 0);
    FileCB_Type_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TMPL), 0);
    FileCB_Version_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TMPL), 0);

    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_PATH), 
      String_Create (FCB_SEQDIR_PATH(""), 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_PATH), 
      String_Create (FCB_SEQDIR_PATH(""), MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_PATH), FALSE);
    FileCB_MagicNum_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_PATH), 0);
    FileCB_Type_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_PATH), 0);
    FileCB_Version_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_PATH), 0);

    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT), 
      String_Create (FCB_SEQDIR_STAT(""), 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT), 
      String_Create (FCB_SEQDIR_STAT(""), MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT), FALSE);
    FileCB_MagicNum_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT), 0);
    FileCB_Type_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT), 0);
    FileCB_Version_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT), 0);

    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_SBMT), 
      String_Create (FCB_SEQDIR_SBMT(""), 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_SBMT), 
      String_Create (FCB_SEQDIR_SBMT(""), MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_SBMT), FALSE);
    FileCB_MagicNum_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_SBMT), 0);
    FileCB_Type_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_SBMT), 0);
    FileCB_Version_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_SBMT), 0);

    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TRCE), 
      String_Create (FCB_SEQDIR_TRCE(""), 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TRCE), 
      String_Create (FCB_SEQDIR_TRCE(""), MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TRCE), FALSE);
    FileCB_MagicNum_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TRCE), 0);
    FileCB_Type_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TRCE), 0);
    FileCB_Version_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TRCE), 0);

    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_COVR), 
      String_Create (FCB_SEQDIR_COVR(""), 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_COVR), 
      String_Create (FCB_SEQDIR_COVR(""), MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_COVR), FALSE);
    FileCB_MagicNum_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_COVR), 0);
    FileCB_Type_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_COVR), 0);
    FileCB_Version_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_COVR), 0);

    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FSEL), 
      String_Create (FCB_SEQDIR_FSEL(""), 0));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FSEL), 
      String_Create (FCB_SEQDIR_FSEL(""), MX_FILENAME));
    FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FSEL), FALSE);
    FileCB_MagicNum_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FSEL), 0);
    FileCB_Type_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FSEL), 0);
    FileCB_Version_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FSEL), 0);
    }

  return ;
}
/* End of Rcb_Init */

/****************************************************************************
*
*  Function Name:                 Rcb_Load
*
*    This function reads in the submission file and changes the tagged 
*    values of the rcb.  If it is a new job submission, it will be 
*    assumed that the target compound name will be known after reading 
*    the submission file, so the user filenames can be completed before
*    returning.  Otherwise, filenames and directory names are ignored
*    (assume they have already been read from a status file).
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
*    None.
*
******************************************************************************/
void Rcb_Load 
  (
  Rcb_t         *rcb_p,
  char          *path,
  Boolean_t      new_submit
  )
{
  FILE          *f_p;
  String_t       goal_sling;
  char           buff[MX_INPUT_SIZE+1];
  char          *tag;
  char          *field;
  char          *cr_in_nl;

  if (path != NULL)
    {
#ifdef _WIN32
    f_p = fopen (gccfix (path), "r");
    if (f_p == NULL)
      {
      fprintf (stderr, "Unable to open submission file:  %s.\n", path);
      exit (-1);
      }
    }
  else
    {
    sprintf (buff, "%s%s%s%s", FCB_SEQDIR_SBMT(""), FCB_PATH_SEP, 
      RCB_DFLT_CMP_NAME, FCB_EXT_SBMT);
    fprintf (stderr, "Using default submission file:  %s.\n", buff);
    f_p = fopen (gccfix (buff), "r");
#else
    f_p = fopen (path, "r");
    if (f_p == NULL)
      {
      fprintf (stderr, "Unable to open submission file:  %s.\n", path);
      exit (-1);
      }
    }
  else
    {
    sprintf (buff, "%s%s%s%s", FCB_SEQDIR_SBMT(""), FCB_PATH_SEP, 
      RCB_DFLT_CMP_NAME, FCB_EXT_SBMT);
    fprintf (stderr, "Using default submission file:  %s.\n", buff);
    f_p = fopen (buff, "r");
#endif
    if (f_p == NULL)
      {
      fprintf (stderr, "Unable to open submission file:  %s.\n", buff);
      exit (-1);
      }
    }

  while (fgets (buff, MX_INPUT_SIZE, f_p) != NULL)
    {
    cr_in_nl = strstr (buff, "\r\n");
    if (cr_in_nl != NULL)
      strcpy (cr_in_nl, cr_in_nl + 1);
    tag = strtok (buff, RCB_FILE_DELIMS);
    if (tag == NULL || *tag == RCB_FILE_COMMENT)
      continue;

    if (strcasecmp (tag, RCB_FILE_TAG_RUNTYPE) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      Rcb_RunType_Put (rcb_p, (U8_t) atoi (field));
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_NAME) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      String_Destroy (Rcb_Name_Get (rcb_p));
      Rcb_Name_Put (rcb_p, String_Create (field, 0));
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_GOAL) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      Sling_Destroy (Rcb_Goal_Get (rcb_p));
      goal_sling = String_Create (field, 0);
      Rcb_Goal_Put (rcb_p, String2Sling (goal_sling));
      String_Destroy (goal_sling);
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_COMMENT) == 0)
      {
      field = strtok (NULL, "");
      while (*field == ' ' || *field == '\t') field++;
      String_Destroy (Rcb_Comment_Get (rcb_p));
      Rcb_Comment_Put (rcb_p, String_Create (field, 0));
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_MAX_CYCLES) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      Rcb_MaxCycles_Put (rcb_p, (U32_t) atol (field));
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_MAX_TIME) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      Rcb_MaxRuntime_Put (rcb_p, (U32_t) atol (field));
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_FIRSTSOL) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcasecmp (field, "TRUE") == 0)
	{
	Rcb_Flags_FirstSol_Put (rcb_p, TRUE);
	}
      else if (strcasecmp (field, "FALSE") == 0)
	{
	Rcb_Flags_FirstSol_Put (rcb_p, FALSE);
	}
      else
	fprintf (stderr, 
	"Unrecognized field %s in submission file for tag %s."
	"  Ignoring ...\n", field, tag);
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_ADD_CYCLES) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      Rcb_AddCycles_Put (rcb_p, (U32_t) atol (field)); 
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_ADD_TIME) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      Rcb_AddTime_Put (rcb_p, (U32_t) atol (field)); 
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_MAX_NODES) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      Rcb_MaxNodes_Put (rcb_p, (U8_t) atoi (field));
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_MAX_WORKERS) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      Rcb_MaxWorkers_Put (rcb_p, (U16_t) atoi (field));
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_MIN_WORKERS) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      Rcb_MinWorkers_Put (rcb_p, (U16_t) atoi (field));
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_PROC_NODE) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      Rcb_ProcPerNode_Put (rcb_p, (U8_t) atoi (field));
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_RUNRESTART) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcasecmp (field, "TRUE") == 0)
	{
	Rcb_Flags_Restart_Put (rcb_p, TRUE);
	}
      else if (strcasecmp (field, "FALSE") == 0)
	{
	Rcb_Flags_Restart_Put (rcb_p, FALSE);
	}
      else
	fprintf (stderr, 
	"Unrecognized field %s in submission file for tag %s."
	"  Ignoring ...\n", field, tag);
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_BACKGROUND) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcasecmp (field, "TRUE") == 0)
	{
	Rcb_Flags_Background_Put (rcb_p, TRUE);
	}
      else if (strcasecmp (field, "FALSE") == 0)
	{
	Rcb_Flags_Background_Put (rcb_p, FALSE);
	}
      else
	fprintf (stderr, 
	"Unrecognized field %s in submission file for tag %s."
	"  Ignoring ...\n", field, tag);
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_SAVE_STATUS) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcasecmp (field, "TRUE") == 0)
	{
	Rcb_Flags_SaveStatus_Put (rcb_p, TRUE);
	}
      else if (strcasecmp (field, "FALSE") == 0)
	{
	Rcb_Flags_SaveStatus_Put (rcb_p, FALSE);
	}
      else
	fprintf (stderr, 
	"Unrecognized field %s in submission file for tag %s."
	"  Ignoring ...\n", field, tag);
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_SAVE_TRACE) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcasecmp (field, "TRUE") == 0)
	{
	Rcb_Flags_SaveTrace_Put (rcb_p, TRUE);
	}
      else if (strcasecmp (field, "FALSE") == 0)
	{
	Rcb_Flags_SaveTrace_Put (rcb_p, FALSE);
	}
      else
	fprintf (stderr, 
	"Unrecognized field %s in submission file for tag %s."
	"  Ignoring ...\n", field, tag);
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_SAVE_MERITS) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcasecmp (field, "TRUE") == 0)
	{
	Rcb_Flags_SaveMerits_Put (rcb_p, TRUE);
	}
      else if (strcasecmp (field, "FALSE") == 0)
	{
	Rcb_Flags_SaveMerits_Put (rcb_p, FALSE);
	}
      else
	fprintf (stderr, 
	"Unrecognized field %s in submission file for tag %s."
	"  Ignoring ...\n", field, tag);
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_CHEM_TRACE) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      Rcb_ChemistryTrace_Put (rcb_p, (U8_t) atoi (field)); 
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_COVER_DEV) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcasecmp (field, "TRUE") == 0)
	{
	Rcb_Flags_SearchCoverDev_Put (rcb_p, TRUE);
	}
      else if (strcasecmp (field, "FALSE") == 0)
	{
	Rcb_Flags_SearchCoverDev_Put (rcb_p, FALSE);
	}
      else
	fprintf (stderr, 
	"Unrecognized field %s in submission file for tag %s."
	"  Ignoring ...\n", field, tag);
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_COVER_SLV) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcasecmp (field, "TRUE") == 0)
	{
	Rcb_Flags_SearchCoverSlv_Put (rcb_p, TRUE);
	}
      else if (strcasecmp (field, "FALSE") == 0)
	{
	Rcb_Flags_SearchCoverSlv_Put (rcb_p, FALSE);
	}
      else
	fprintf (stderr, 
	"Unrecognized field %s in submission file for tag %s."
	"  Ignoring ...\n", field, tag);
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_COVER_PRCT) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      Rcb_CoverPrct_Put (rcb_p, (U16_t) atoi (field)); 
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_MASTER_SEL_NB) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcasecmp (field, "TRUE") == 0)
	{
	Rcb_Flags_MasterSelNB_Put (rcb_p, TRUE);
	}
      else if (strcasecmp (field, "FALSE") == 0)
	{
	Rcb_Flags_MasterSelNB_Put (rcb_p, FALSE);
	}
      else
	fprintf (stderr, 
	"Unrecognized field %s in submission file for tag %s."
	"  Ignoring ...\n", field, tag);
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_AND_PARALLELISM) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcasecmp (field, "TRUE") == 0)
	{
	Rcb_Flags_AndParallel_Put (rcb_p, TRUE);
	}
      else if (strcasecmp (field, "FALSE") == 0)
	{
	Rcb_Flags_AndParallel_Put (rcb_p, FALSE);
	}
      else
	fprintf (stderr, 
	"Unrecognized field %s in submission file for tag %s."
	"  Ignoring ...\n", field, tag);
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_WORKER_LOCAL_SEL) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcasecmp (field, "TRUE") == 0)
	{
	Rcb_Flags_WorkerLocalSel_Put (rcb_p, TRUE);
	}
      else if (strcasecmp (field, "FALSE") == 0)
	{
	Rcb_Flags_WorkerLocalSel_Put (rcb_p, FALSE);
	}
      else
	fprintf (stderr, 
	"Unrecognized field %s in submission file for tag %s."
	"  Ignoring ...\n", field, tag);
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_DIS_SHARED_SELS) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcasecmp (field, "TRUE") == 0)
	{
	Rcb_Flags_DisSharedSels_Put (rcb_p, TRUE);
	}
      else if (strcasecmp (field, "FALSE") == 0)
	{
	Rcb_Flags_DisSharedSels_Put (rcb_p, FALSE);
	}
      else
	fprintf (stderr, 
	"Unrecognized field %s in submission file for tag %s."
	"  Ignoring ...\n", field, tag);
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_WORKER_CURMERIT) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcasecmp (field, "TRUE") == 0)
	{
	Rcb_Flags_WorkerCurMerit_Put (rcb_p, TRUE);
	}
      else if (strcasecmp (field, "FALSE") == 0)
	{
	Rcb_Flags_WorkerCurMerit_Put (rcb_p, FALSE);
	}
      else
	fprintf (stderr, 
	"Unrecognized field %s in submission file for tag %s."
	"  Ignoring ...\n", field, tag);
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_WORKER_NBMERIT) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcasecmp (field, "TRUE") == 0)
	{
	Rcb_Flags_WorkerNBMerit_Put (rcb_p, TRUE);
	}
      else if (strcasecmp (field, "FALSE") == 0)
	{
	Rcb_Flags_WorkerNBMerit_Put (rcb_p, FALSE);
	}
      else
	fprintf (stderr, 
	"Unrecognized field %s in submission file for tag %s."
	"  Ignoring ...\n", field, tag);
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_WORKER_PREF_GLOBAL) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcasecmp (field, "TRUE") == 0)
	{
	Rcb_Flags_WorkerPreferGlobal_Put (rcb_p, TRUE);
	}
      else if (strcasecmp (field, "FALSE") == 0)
	{
	Rcb_Flags_WorkerPreferGlobal_Put (rcb_p, FALSE);
	}
      else
	fprintf (stderr, 
	"Unrecognized field %s in submission file for tag %s."
	"  Ignoring ...\n", field, tag);
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_WORKER_MAX_CYCLES) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      Rcb_WorkerMaxCycles_Put (rcb_p, (U16_t) atoi (field));
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_WORKER_MERIT_PRCT) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      Rcb_WorkerMeritPrct_Put (rcb_p, (U8_t) atoi (field));
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_EFFORTDIS) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcasecmp (field, "TRUE") == 0)
	{
	Rcb_Flags_EffortDis_Put (rcb_p, TRUE);
	}
      else if (strcasecmp (field, "FALSE") == 0)
	{
	Rcb_Flags_EffortDis_Put (rcb_p, FALSE);
	}
      else
	fprintf (stderr, 
	"Unrecognized field %s in submission file for tag %s."
	"  Ignoring ...\n", field, tag);
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_NTCL) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      Rcb_NTCL_Put (rcb_p, (U8_t) atoi (field));
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_PRESERVE) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcasecmp (field, "TRUE") == 0)
	{
	Rcb_Flags_PreserveStructures_Put (rcb_p, TRUE);
	}
      else if (strcasecmp (field, "FALSE") == 0)
	{
	Rcb_Flags_PreserveStructures_Put (rcb_p, FALSE);
	}
      else
	fprintf (stderr, 
	"Unrecognized field %s in submission file for tag %s."
	"  Ignoring ...\n", field, tag);
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_STEREO) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcasecmp (field, "TRUE") == 0)
	{
	Rcb_Flags_StereoChemistry_Put (rcb_p, TRUE);
	}
      else if (strcasecmp (field, "FALSE") == 0)
	{
	Rcb_Flags_StereoChemistry_Put (rcb_p, FALSE);
	}
      else
	fprintf (stderr, 
	"Unrecognized field %s in submission file for tag %s."
	"  Ignoring ...\n", field, tag);
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_STRATBONDS) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcasecmp (field, "TRUE") == 0)
	{
	Rcb_Flags_StrategicBonds_Put (rcb_p, TRUE);
	}
      else if (strcasecmp (field, "FALSE") == 0)
	{
	Rcb_Flags_StrategicBonds_Put (rcb_p, FALSE);
	}
      else
	fprintf (stderr, 
	"Unrecognized field %s in submission file for tag %s."
	"  Ignoring ...\n", field, tag);
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_RUNCONTINUE) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcasecmp (field, "TRUE") == 0)
	{
	Rcb_Flags_Continue_Put (rcb_p, TRUE);
	}
      else if (strcasecmp (field, "FALSE") == 0)
	{
	Rcb_Flags_Continue_Put (rcb_p, FALSE);
	}
      else
	fprintf (stderr, 
	"Unrecognized field %s in submission file for tag %s."
	"  Ignoring ...\n", field, tag);
      }

    else if (strcasecmp (tag, RCB_FILE_TAG_LEAP) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      Rcb_LeapSize_Put (rcb_p, (U32_t) atol (field));
      }

    else if (new_submit && strcasecmp (tag, RCB_FILE_TAG_DIR_AVL) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcmp (field, (char *) String_Value_Get (FileCB_DirStr_Get (
	  Rcb_IthFileCB_Get (rcb_p, FCB_IND_AVLC)))) != 0)
	{
	String_Destroy (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	  FCB_IND_AVLC)));
	FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_AVLC), 
	  String_Create (field, 0));
	FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_AVLC), TRUE);
	Rcb_Flags_LibsChanged_Put (rcb_p, TRUE);
	}
      }

    else if (new_submit && strcasecmp (tag, RCB_FILE_TAG_DIR_FG) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcmp (field, (char *) String_Value_Get (FileCB_DirStr_Get (
	  Rcb_IthFileCB_Get (rcb_p, FCB_IND_FNGP)))) != 0)
	{
	String_Destroy (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	  FCB_IND_FNGP)));
	FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FNGP), 
	  String_Create (field, 0));
	FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FNGP), TRUE);
	Rcb_Flags_LibsChanged_Put (rcb_p, TRUE);
	}
      }

    else if (new_submit && strcasecmp (tag, RCB_FILE_TAG_DIR_RXN) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcmp (field, (char *) String_Value_Get (FileCB_DirStr_Get (
	  Rcb_IthFileCB_Get (rcb_p, FCB_IND_RXNS)))) != 0)
	{
	String_Destroy (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	  FCB_IND_RXNS)));
	FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_RXNS), 
	  String_Create (field, 0));
	FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_RXNS), TRUE);
	Rcb_Flags_LibsChanged_Put (rcb_p, TRUE);
	}
      }

    else if (new_submit && strcasecmp (tag, RCB_FILE_TAG_DIR_TEMPL) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcmp (field, (char *) String_Value_Get (FileCB_DirStr_Get (
	  Rcb_IthFileCB_Get (rcb_p, FCB_IND_TMPL)))) != 0)
	{
	String_Destroy (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	  FCB_IND_TMPL)));
	FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TMPL), 
	  String_Create (field, 0));
	FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TMPL), 
	  TRUE);
	}
      }

    else if (new_submit && strcasecmp (tag, RCB_FILE_TAG_DIR_PATH) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcmp (field, (char *) String_Value_Get (FileCB_DirStr_Get (
	  Rcb_IthFileCB_Get (rcb_p, FCB_IND_PATH)))) != 0)
	{
	String_Destroy (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	  FCB_IND_PATH)));
	String_Destroy (FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	  FCB_IND_PATH)));
	FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_PATH), 
	  String_Create (field, 0));
	FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_PATH), 
	  String_Create (field, MX_FILENAME));
	FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_PATH), TRUE);
	}
      }

    else if (new_submit && strcasecmp (tag, RCB_FILE_TAG_DIR_STAT) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcmp (field, (char *) String_Value_Get (FileCB_DirStr_Get (
	  Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT)))) != 0)
	{
	String_Destroy (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	  FCB_IND_STAT)));
	String_Destroy (FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	  FCB_IND_STAT)));
	FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT), 
	  String_Create (field, 0));
	FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT), 
	  String_Create (field, MX_FILENAME));
	FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_STAT), TRUE);
	}
      }

    else if (new_submit && strcasecmp (tag, RCB_FILE_TAG_DIR_SUBMIT) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcmp (field, (char *) String_Value_Get (FileCB_DirStr_Get (
	  Rcb_IthFileCB_Get (rcb_p, FCB_IND_SBMT)))) != 0)
	{
	String_Destroy (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	  FCB_IND_SBMT)));
	String_Destroy (FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	  FCB_IND_SBMT)));
	FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_SBMT), 
	  String_Create (field, 0));
	FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_SBMT), 
	  String_Create (field, MX_FILENAME));
	FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_SBMT), 
	  TRUE);
	}
      }

    else if (new_submit && strcasecmp (tag, RCB_FILE_TAG_DIR_TRACE) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcmp (field, (char *) String_Value_Get (FileCB_DirStr_Get (
	  Rcb_IthFileCB_Get (rcb_p, FCB_IND_TRCE)))) != 0)
	{
	String_Destroy (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	  FCB_IND_TRCE)));
	String_Destroy (FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	  FCB_IND_TRCE)));
	FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TRCE), 
	  String_Create (field, 0));
	FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TRCE), 
	  String_Create (field, MX_FILENAME));
	FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_TRCE), TRUE);
	}
      }

    else if (new_submit && strcasecmp (tag, RCB_FILE_TAG_DIR_FORCESEL) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcmp (field, (char *) String_Value_Get (FileCB_DirStr_Get (
	  Rcb_IthFileCB_Get (rcb_p, FCB_IND_FSEL)))) != 0)
	{
	String_Destroy (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	  FCB_IND_FSEL)));
	String_Destroy (FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	  FCB_IND_FSEL)));
	FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FSEL), 
	  String_Create (field, 0));
	FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FSEL), 
	  String_Create (field, MX_FILENAME));
	}
      }

    else if (new_submit && strcasecmp (tag, RCB_FILE_TAG_FILE_FORCESEL) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      String_Concat_c (&FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	FCB_IND_FSEL)), FCB_PATH_SEP);
      String_Concat_c (&FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	FCB_IND_FSEL)), field);
      String_Concat_c (&FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	FCB_IND_FSEL)), FCB_EXT_FSEL);
      FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_FSEL), 
	TRUE);
      }

    else if (new_submit && strcasecmp (tag, RCB_FILE_TAG_DIR_COVER) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      if (strcmp (field, (char *) String_Value_Get (FileCB_DirStr_Get (
	  Rcb_IthFileCB_Get (rcb_p, FCB_IND_COVR)))) != 0)
	{
	String_Destroy (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	  FCB_IND_COVR)));
	String_Destroy (FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	  FCB_IND_COVR)));
	FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_COVR), 
	  String_Create (field, 0));
	FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_COVR), 
	  String_Create (field, MX_FILENAME));
       }
      }

    else if (new_submit && strcasecmp (tag, RCB_FILE_TAG_FILE_COVER) == 0)
      {
      field = strtok (NULL, RCB_FILE_DELIMS);
      String_Concat_c (&FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	FCB_IND_COVR)), FCB_PATH_SEP);
      String_Concat_c (&FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	FCB_IND_COVR)), field);
      String_Concat_c (&FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p, 
	FCB_IND_COVR)), FCB_EXT_COVR);
      FileCB_Changed_Put (Rcb_IthFileCB_Get (rcb_p, FCB_IND_COVR), 
	TRUE);
      }

    else
      {
      if (!new_submit && 
	    (strcasecmp (tag, RCB_FILE_TAG_DIR_AVL) == 0
	  || strcasecmp (tag, RCB_FILE_TAG_DIR_FG) == 0
	  || strcasecmp (tag, RCB_FILE_TAG_DIR_RXN) == 0
	  || strcasecmp (tag, RCB_FILE_TAG_DIR_PATH) == 0
	  || strcasecmp (tag, RCB_FILE_TAG_DIR_STAT) == 0
	  || strcasecmp (tag, RCB_FILE_TAG_DIR_SUBMIT) == 0
	  || strcasecmp (tag, RCB_FILE_TAG_DIR_TEMPL) == 0
	  || strcasecmp (tag, RCB_FILE_TAG_DIR_TRACE) == 0
	  || strcasecmp (tag, RCB_FILE_TAG_DIR_FORCESEL) == 0
	  || strcasecmp (tag, RCB_FILE_TAG_FILE_FORCESEL) == 0
	  || strcasecmp (tag, RCB_FILE_TAG_DIR_COVER) == 0
	  || strcasecmp (tag, RCB_FILE_TAG_FILE_COVER) == 0))
	fprintf (stderr, 
	  "File name changes not allowed:  %s.  Ignoring ...\n", tag);
      else
	fprintf (stderr, 
	  "Unrecognized tag in submission file:  %s.  Ignoring ...\n", tag);
      continue;
      }
    }

  /*  Complete file names.  */
  if (new_submit)
    {
    FileNames_Complete (Rcb_FileCBs_Get (rcb_p), Rcb_Name_Get (rcb_p));

    }

  fclose (f_p);
  return ;
}
/* End of Rcb_Load */

/****************************************************************************
*
*  Function Name:                 RunStats_Destroy
*
*    This routine frees allocated memory used in the Run Status data
*    structure.
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
*    Deallocates memory.
*
******************************************************************************/
void RunStats_Destroy
  (
  RunStats_t   *rstat_p                     /* Instance to destroy */
  )
{
  String_Destroy (RunStats_ExitCond_Get (rstat_p));
  return;
}
/* End of RunStats_Destroy */

/****************************************************************************
*
*  Function Name:                 RunStats_Dump
*
*    This routine prints a formatted dump of the Run Status.
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
void RunStats_Dump
  (
  RunStats_t    *rstat_p,                    /* Instance to dump */
  FileDsc_t     *filed_p,                    /* File to dump to */
  Boolean_t      distributed                 /* Distributed run? */
  )
{
  FILE          *f;                          /* Temporary */

  f = IO_FileHandle_Get (filed_p);
  if (rstat_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL Run Status\n");
    return;
    }

  fprintf (f, "\nDump of Run Status\n");
  fprintf (f, "%-30s  %lu\n", "  Cycles completed:", 
    RunStats_CumCycles_Get (rstat_p));
  if (RunStats_Flags_FirstSolFound_Get (rstat_p))
    {
    fprintf (f, "%-30s  %lu\n", "  Cycles to first solution:", 
      RunStats_CyclesFirstSol_Get (rstat_p));
    fprintf (f, "%-30s  %f seconds\n", "  Time to first solution:", 
      Sensible_Time_Format (RunStats_TimeFirstSol_Get (rstat_p), FALSE));
    }

  fprintf (f, "%-30s  %lu\n", "  Unique compounds generated:", 
    RunStats_NumSymbols_Get (rstat_p));
  fprintf (f, "%-30s  %lu\n", "  Total compounds generated:", 
    RunStats_NumCompounds_Get (rstat_p));
  fprintf (f, "%-30s  %lu\n\n", "  Total subgoals generated:", 
    RunStats_NumSubgoals_Get (rstat_p));

  if (RunStats_Flags_StatusSaved_Get (rstat_p))
    fprintf (f, "  Status file was saved\n");
  else
    fprintf (f, "  Status file was not saved\n");

  if (RunStats_Flags_TraceSaved_Get (rstat_p))
    fprintf (f, "  Trace file was saved\n\n");
  else
    fprintf (f, "  Trace file was not saved\n\n");

  if (distributed)
    {
    fprintf (f, "  Master cummulative times:\n");
    Timers_Dump (RunStats_MasterCumTimes_Get (rstat_p), filed_p);
    fprintf (f, "  Worker cummulative times:\n");
    Timers_Dump (RunStats_WorkerCumTimes_Get (rstat_p), filed_p);
    }

  else
    {
    fprintf (f, "  Cummulative times:\n");
    Timers_Dump (RunStats_CumTimes_Get (rstat_p), filed_p);
    }

  fprintf (f, "  Exit Condition:  %s\n\n", (char *) String_Value_Get (
    RunStats_ExitCond_Get (rstat_p)));

  return;
}
/* End of RunStats_Dump */

/****************************************************************************
*
*  Function Name:                 RunStats_Init
*
*    This function initializes the run statistics.
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
*    None.
*
******************************************************************************/
void RunStats_Init 
  (
  RunStats_t   *rstat_p
  )
{
  RunStats_CumCycles_Put (rstat_p, 0); 
  RunStats_CyclesFirstSol_Put (rstat_p, 0); 
  RunStats_ExitCond_Put (rstat_p, String_Create (NULL, 0));
  RunStats_Flags_Put (rstat_p, 0);
  RunStats_Flags_DevSPCovered_Put (rstat_p, FALSE)
  RunStats_Flags_FirstSolFound_Put (rstat_p, FALSE);
  RunStats_Flags_PstChanged_Put (rstat_p, FALSE);
  RunStats_Flags_SearchHalted_Put (rstat_p, FALSE);
  RunStats_Flags_SolSPCovered_Put (rstat_p, FALSE)
  RunStats_Flags_StatusSaved_Put (rstat_p, FALSE);
  RunStats_Flags_TraceSaved_Put (rstat_p, FALSE);
  RunStats_NumCompounds_Put (rstat_p, 0); 
  RunStats_NumSubgoals_Put (rstat_p, 0); 
  RunStats_NumSymbols_Put (rstat_p, 0); 
  RunStats_TimeFirstSol_Put (rstat_p, TIME_ZERO);

  Timer_Init (RunStats_MasterCumTimes_Get (rstat_p)); 
  Timer_Init (RunStats_WorkerCumTimes_Get (rstat_p)); 

  return ;
}
/* End of RunStats_Init */

/* End of RCB.C */
