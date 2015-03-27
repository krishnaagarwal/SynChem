/******************************************************************************
*
*  Copyright (C) 1991-1996 Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     TIMER.C
*
*    This module is contains the implementation of the timer data structure.
*
*
*  Routines:
*
*    Timers_Dump
*
*  Creation Date:
*
*    23-Sep-1996
*
*  Authors:
*
*    Tito Autrey (rewritten based on others PL/I code)
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 23-Sep-96  Krebsbach  Moved Timer_t to timer.h.
*
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#define TIMER_GLOBALS 1
#ifndef _H_TIMER_
#include "timer.h"
#endif
#undef TIMER_GLOBALS


/****************************************************************************
*
*  Function Name:                 Syn_Time_Get
*
*    This is a "system independent" function to return the time.  It has to
*    be modified for each different operating system that SYNCHEM is ported
*    to.
*
*  Used to be:
*
*    elptime:
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
Time_t Syn_Time_Get
  (
  Boolean_t      positive                    /* Flag - postive time */
  )
{
  Time_t         buf;                        /* Buffer to read time into */

  #ifdef _SUN4_OS_

  if (gettimeofday (&buf, NULL) != 0)
    IO_Exit_Error (R_UTL, X_SYSCALL, "Call to gettimeofday failed");

  #else

  #ifdef _SOLARIS_OS_

  if (gettimeofday (&buf, NULL) != 0)
    IO_Exit_Error (R_UTL, X_SYSCALL, "Call to gettimeofday failed");

  #else

  #ifdef _IRIX62_OS_

  if (gettimeofday (&buf, NULL) != 0)
    IO_Exit_Error (R_UTL, X_SYSCALL, "Call to gettimeofday failed");

  #else

  #ifdef _WIN32

  if (gettimeofday (&buf, NULL) != 0)
    IO_Exit_Error (R_UTL, X_SYSCALL, "Call to gettimeofday failed");

  #else

  #ifdef _CYGWIN_

  if (gettimeofday (&buf, NULL) != 0)
    IO_Exit_Error (R_UTL, X_SYSCALL, "Call to gettimeofday failed");

  #else

  #ifdef _REDHAT_

  if (gettimeofday (&buf, NULL) != 0)
    IO_Exit_Error (R_UTL, X_SYSCALL, "Call to gettimeofday failed");

  #else

  if (gettimeofday (&buf, NULL) != 0)
    IO_Exit_Error (R_UTL, X_SYSCALL, "Call to gettimeofday failed");

  #endif
  #endif
  #endif
  #endif
  #endif
  #endif

  if (positive == FALSE)
    {
    buf.tv_sec *= -1;
    buf.tv_usec *= -1;
    }

  return buf;
}


/****************************************************************************
*
*  Function Name:                 Timers_Dump
*
*    This routine prints a formatted dump of a timers block.
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
void Timers_Dump
  (
  Timer_t       *t_p,                        /* Block to dump */
  FileDsc_t     *filed_p                     /* File to dump to */
  )
{
  FILE          *f;                          /* Temporary */

  f = IO_FileHandle_Get (filed_p);
  if (t_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL timers block\n");
    return;
    }

  DEBUG_ADDR (R_UTL, DB_MAIN, t_p);
  fprintf (f, "%-24s  %9.4f\n", "    Initialization:", 
    Time_Format (Timer_Init_Get (t_p)));
  fprintf (f, "%-24s  %9.4f\n", "    Select next:", 
    Time_Format (Timer_Select_Get (t_p)));
  fprintf (f, "%-24s  %9.4f\n", "    Expansion:", 
    Time_Format (Timer_Expand_Get (t_p)));
  fprintf (f, "%-24s  %9.4f\n", "    Update:", 
    Time_Format (Timer_UpdatePST_Get (t_p)));
  fprintf (f, "%-24s  %9.4f\n\n", "    Cycle time:", 
    Time_Format (Timer_Cycle_Get (t_p)));
  fprintf (f, "%-24s  %9.4f\n", "    Communication:", 
    Time_Format (Timer_Commun_Get (t_p)));
  fprintf (f, "%-24s  %9.4f\n", "    Wait:", 
    Time_Format (Timer_Wait_Get (t_p)));
  fprintf (f, "%-24s  %9.4f\n\n", "    Tuple wait:", 
    Time_Format (Timer_TupleWait_Get (t_p)));
  
  return;
}
/* End of Timers_Dump */
