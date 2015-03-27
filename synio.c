/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     SYNIO.C
*
*    This module contains all of the user input routines and all non-dump
*    oriented output routines.  The reaction library and available compounds
*    library have their own supporting modules, but for the most part the
*    rest of the IO is contained here, in one place.
*
*  Routines:
*
*    Debug_Init
*    IO_Exit_Error
*    IO_Get_Boolean_PD
*    IO_Get_Integer
*    IO_Get_Integer_PD
*    IO_Get_String
*    IO_Get_String_PDA
*    IO_Init
*    IO_Init_Files
*    IO_Put_Debug
*    IO_Put_Trace
*    IO_Put_Trace_Time
*    Trace_Init
*    SInput_Integer
*    SInput_String
*
*  Creation Date:
*
*    01-Jan-1991
*
*  Authors:
*
*    Daren Krebsbach
*    Tito Autrey
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 28-Sep-96  Krebsbach  Changed IO_Init_Files so that it appends an existing
*                       trace file on a restart run rather than truncate.
* 28-Sep-96  Krebsbach  Added function IO_Close_Files.
*
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
/* #include <sys/file.h> */

#define IO_GLOBALS

#include "synchem.h"
#include "debug.h"

#ifndef _H_SYNIO_
#include "synio.h"
#endif

#undef IO_GLOBALS

/* Static routine prototypes */

static SynCondValue SInput_Integer (U32_t *, U8_t, FileDsc_t *);
static SynCondValue SInput_String  (U8_t *, U16_t, FileDsc_t *);

/****************************************************************************
*
*  Function Name:                 Debug_Init
*
*    This routine initializes the debugging output routine messages and any
*    other things that debugging may need early on.
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
void Debug_Init
  (
  )
{
  /* Bad address trapping (avoid the "All-the-world's-a-VAX" syndrome) */

  GBAddr = (Address)1024;


  GModuleNames[R_RESERVED] = M_RESERVED;
  GModuleNames[R_AVL] = M_AVL;
  GModuleNames[R_REACTION] = M_REACTION;
  GModuleNames[R_IO] = M_IO;
  GModuleNames[R_MAIN] = M_MAIN;
  GModuleNames[R_SUBGENR] = M_SUBGENR;
  GModuleNames[R_ARRAY] = M_ARRAY;
  GModuleNames[R_UTL] = M_UTL;
  GModuleNames[R_SLING] = M_SLING;
  GModuleNames[R_XTR] = M_XTR;
  GModuleNames[R_PST] = M_PST;
  GModuleNames[R_STRATEGY] = M_STRATEGY;
  GModuleNames[R_TSD] = M_TSD;
  GModuleNames[R_ISAM] = M_ISAM;
  GModuleNames[R_POSTTEST] = M_POSTTEST;
  GModuleNames[R_NONE] = M_NONE;

  /* MAIN */

  GTrace[DB_RESERVED].params = TL_NONE;
  GTrace[DB_MAIN].params = TL_NONE;
  GTrace[DB_CHEMISTRY].params = TL_NONE;
  GTrace[DB_INIT].params = TL_NONE;

  /* SYNIO */

  GTrace[DB_IO_GET].params = TL_ALWAYS;
  GTrace[DB_IO_PUT].params = TL_ALWAYS;
  GTrace[DB_IO_DUMP].params = TL_NONE;

  /* ARRAY */

  GTrace[DB_ARRAY1DCREATE].params = TL_NONE;
  GTrace[DB_ARRAY2DCREATE].params = TL_NONE;
  GTrace[DB_ARRAY3DCREATE].params = TL_NONE;
  GTrace[DB_ARRAYDESTROY].params = TL_NONE;
  GTrace[DB_ARRAYCOPY].params = TL_NONE;
  GTrace[DB_ARRAYCOPYCONTENTS].params = TL_NONE;

  /* UTL */

  GTrace[DB_ATMARRCOPY].params = TL_NONE;
  GTrace[DB_ATMARRCREATE].params = TL_NONE;
  GTrace[DB_ATMARRDESTROY].params = TL_NONE;
  GTrace[DB_STACKCREATE].params = TL_NONE;
  GTrace[DB_STACKDESTROY].params = TL_NONE;
  GTrace[DB_STACKPOP].params = TL_NONE;
  GTrace[DB_STACKPUSH].params = TL_NONE;
  GTrace[DB_LNKDTCOPY].params = TL_NONE;
  GTrace[DB_LNKDTCREATE].params = TL_NONE;
  GTrace[DB_LNKDTDESTROY].params = TL_NONE;
  GTrace[DB_STRINGCONCAT].params = TL_NONE;
  GTrace[DB_STRINGCOPY].params = TL_NONE;
  GTrace[DB_STRINGCREATE].params = TL_NONE;
  GTrace[DB_STRINGDESTROY].params = TL_NONE;

  /* AROMATICS */

  GTrace[DB_AROMCOPY].params = TL_NONE;
  GTrace[DB_AROMCREATE].params = TL_NONE;
  GTrace[DB_AROMDESTROY].params = TL_NONE;
  GTrace[DB_AROMSET].params = TL_NONE;
  GTrace[DB_AROMFINDBONDS].params = TL_NONE;

  /* ATOMS */

  GTrace[DB_ATOMCOPY].params = TL_NONE;
  GTrace[DB_ATOMCREATE].params = TL_NONE;
  GTrace[DB_ATOMDESTROY].params = TL_NONE;
  GTrace[DB_ATOMSET].params = TL_NONE;

  /* AVLCOMP */

  GTrace[DB_AVCPINFDESTROY].params = TL_NONE;
  GTrace[DB_AVCPINFEXTRACT].params = TL_NONE;
  GTrace[DB_AVCPINFEXTRUDE].params = TL_NONE;

  /* FUNCGROUPS */

  GTrace[DB_FNGRPCOPY].params = TL_NONE;
  GTrace[DB_FNGRPCREATE].params = TL_NONE;
  GTrace[DB_FNGRPDESTROY].params = TL_NONE;
  GTrace[DB_FNGRPSUBINSTOK].params = TL_NONE;
  GTrace[DB_FNGRPINIT].params = TL_NONE;
  GTrace[DB_FNGRPSTATIC].params = TL_NONE;

  /* NAME */

  GTrace[DB_NAMECOPY].params = TL_NONE;
  GTrace[DB_NAMECREATE].params = TL_NONE;
  GTrace[DB_NAMEDESTROY].params = TL_NONE;
  GTrace[DB_NAMESET].params = TL_NONE;
  GTrace[DB_NAMESLINGGET].params = TL_NONE;
  GTrace[DB_NAMESLINGMAP].params = TL_NONE;

  /* PST */

  GTrace[DB_PSTBROTHERGET].params = TL_NONE;
  GTrace[DB_PSTBROTHERPUT].params = TL_NONE;
  GTrace[DB_PSTCOMPCREATE].params = TL_NONE;
  GTrace[DB_PSTFATHERGET].params = TL_NONE;
  GTrace[DB_PSTFATHERPUT].params = TL_NONE;
  GTrace[DB_PSTSONGET].params = TL_NONE;
  GTrace[DB_PSTSONPUT].params = TL_NONE;
  GTrace[DB_PSTNUMSONS].params = TL_NONE;
  GTrace[DB_PSTSUBGCREATE].params = TL_NONE;
  GTrace[DB_PSTCOMPMERITSET].params = TL_NONE;
  GTrace[DB_PSTROOTSET].params = TL_NONE;
  GTrace[DB_PSTSYMTABCREATE].params = TL_NONE;
  GTrace[DB_PSTSUBGINSERT].params = TL_NONE;

  /* REACTION */

  GTrace[DB_REACTDESTROY].params = TL_NONE;
  GTrace[DB_REACTREAD].params = TL_NONE;
  GTrace[DB_REACTXTREAD].params = TL_NONE;
  GTrace[DB_REACTWRITE].params = TL_NONE;
  GTrace[DB_REACTXTWRITE].params = TL_NONE;

  /* RESONANTBONDS */

  GTrace[DB_RESBONDSFIND].params = TL_NONE;
  GTrace[DB_RESBONDSFINDSPIRO].params = TL_ALWAYS;
  GTrace[DB_RESBONDSTATIC].params = TL_NONE;
  GTrace[DB_RESBONDSFIX].params = TL_NONE;

  /* RINGDEFINITION */

  GTrace[DB_RINGDEFCOPY].params = TL_NONE;
  GTrace[DB_RINGDEFCREATE].params = TL_NONE;
  GTrace[DB_RINGDEFDESTROY].params = TL_NONE;
  GTrace[DB_RINGDEFCOMFIND].params = TL_NONE;
  GTrace[DB_RINGDEFMINRINGFIND].params = TL_NONE;
  GTrace[DB_RINGDEFMIN4RINGFIND].params = TL_ALWAYS;
  GTrace[DB_RINGDEFPRMRNGNDSGET].params = TL_NONE;
  GTrace[DB_RINGDEFSTATIC].params = TL_NONE;
  GTrace[DB_RINGDEFPRMCYCFIND].params = TL_NONE;
  GTrace[DB_RINGDEFCITERNG].params = TL_ALWAYS;

  /* RINGS */

  GTrace[DB_RINGCOPY].params = TL_NONE;
  GTrace[DB_RINGCREATE].params = TL_NONE;
  GTrace[DB_RINGDESTROY].params = TL_NONE;
  GTrace[DB_RINGSYSFIND].params = TL_NONE;
  GTrace[DB_RINGBONDSET].params = TL_NONE;
  GTrace[DB_RINGSTATIC].params = TL_NONE;

  /* RINGSYSTEMS */

  GTrace[DB_RINGSYSCOPY].params = TL_NONE;
  GTrace[DB_RINGSYSCREATE].params = TL_NONE;
  GTrace[DB_RINGSYSDESTROY].params = TL_NONE;
  GTrace[DB_RINGSYSSTATIC].params = TL_NONE;

  /* SLING */

  GTrace[DB_SLINGCOPY].params = TL_NONE;
  GTrace[DB_SLINGCREATE].params = TL_NONE;
  GTrace[DB_SLINGDESTROY].params = TL_NONE;
  GTrace[DB_SLINGCANNAME2XTR].params = TL_NONE;
  GTrace[DB_SLING2TSD].params = TL_NONE;
  GTrace[DB_SLING2TSDPLHY].params = TL_NONE;
  GTrace[DB_SLING2XTR].params = TL_NONE;
  GTrace[DB_SLING2XTRPLHY].params = TL_NONE;
  GTrace[DB_SLINGSTATIC].params = TL_NONE;
  GTrace[DB_SLINGCANONGEN].params = TL_NONE;
  GTrace[DB_SLINGVALIDATE].params = TL_NONE;

  /* TSD */

  GTrace[DB_TSDATOMCONN].params = TL_NONE;
  GTrace[DB_TSDATOMCONCHNG].params = TL_NONE;
  GTrace[DB_TSDATOMDISC].params = TL_NONE;
  GTrace[DB_TSDATOMNEIGHGET].params = TL_NONE;
  GTrace[DB_TSDBONDCHANGE].params = TL_NONE;
  GTrace[DB_TSDATOMVALGET].params = TL_NONE;
  GTrace[DB_TSDCOPY].params = TL_NONE;
  GTrace[DB_TSDCREATE].params = TL_NONE;
  GTrace[DB_TSDDESTROY].params = TL_NONE;
  GTrace[DB_TSDEXPAND].params = TL_NONE;
  GTrace[DB_TSDMATCOMP].params = TL_ALWAYS;
  GTrace[DB_TSDVERIFY].params = TL_ALWAYS;
  GTrace[DB_TSD2XTR].params = TL_NONE;
  GTrace[DB_TSD2SLING].params = TL_NONE;
  GTrace[DB_TSD2SLINGX].params = TL_NONE;
  GTrace[DB_TSDSTATIC].params = TL_NONE;

  /* XTR */

  GTrace[DB_XTRCOMPRESS].params = TL_NONE;
  GTrace[DB_XTRCOPY].params = TL_NONE;
  GTrace[DB_XTRCREATE].params = TL_NONE;
  GTrace[DB_XTRDESTROY].params = TL_NONE;
  GTrace[DB_XTRCOPYSUBSET].params = TL_NONE;
  GTrace[DB_XTRCOPYEXPAND].params = TL_NONE;
  GTrace[DB_XTRRESBONDSET].params = TL_NONE;
  GTrace[DB_XTRRINGSET].params = TL_NONE;
  GTrace[DB_XTRRINGDEFSET].params = TL_NONE;
  GTrace[DB_XTR2SLING].params = TL_NONE;
  GTrace[DB_XTR2TSD].params = TL_NONE;
  GTrace[DB_XTRSTATIC].params = TL_NONE;

  /* ISAM */

  GTrace[DB_ISAMCLOSE].params = TL_NONE;
  GTrace[DB_ISAMOPEN].params = TL_NONE;
  GTrace[DB_ISAMREAD].params = TL_NONE;
  GTrace[DB_ISAMREADNEXT].params = TL_NONE;
  GTrace[DB_ISAMREADNOBUF].params = TL_NONE;
  GTrace[DB_ISAMRECINBUF].params = TL_NONE;
  GTrace[DB_ISAMINBUFSIZE].params = TL_NONE;
  GTrace[DB_ISAMWRITE].params = TL_NONE;

  /* FUNCGROUP FILES */

  GTrace[DB_FNGRPRECREAD].params = TL_NONE;
  GTrace[DB_FNGRPRECWRITE].params = TL_NONE;

  return;
}

/****************************************************************************
*
*  Function Name:                 IO_Close_Files
*
*    This routine is responsible for initializing files that are used 
*    during the run of SYNCHEM.
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
void IO_Close_Files
  (
  )
{

  fclose (IO_FileHandle_Get (&GTraceFile));

  DEBUG (R_IO, DB_IO_DUMP, TL_IO, (outbuf,
    "Run files initialized successfully"));

  return;
}

/****************************************************************************
*
*  Function Name:                 IO_Exit_Error
*
*    This routine is called when an unrecoverable error occurs.  It will
*    print out a message and then call the exit(1) system service.
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
*    This routine does NOT return!
*
******************************************************************************/
U16_t IO_Exit_Error
  (
  int            routine,                    /* Final routine */
  int            reason,                     /* Failure reason */
  const char    *message                     /* Incident specific message */
  )
{
  printf ("Exiting from %s due to %s, %s\n", GModuleNames[routine],
    GExitReasons[reason], message);

  fprintf (IO_FileHandle_Get (&GStdErr), "Exiting from %s due to %s, %s\n",
    GModuleNames[routine], GExitReasons[reason], message);

  exit (-1);
  return INFINITY;
}

/****************************************************************************
*
*  Function Name:                 IO_Get_Boolean_PD
*
*    This routine knows how to get a boolean answer from the user.  It does
*    prompting and defaulting, hence the PD.
*
*    A boolean may be a yes or no, first character thereof, upper or lower
*    case, the same goes for true or false.
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
Boolean_t IO_Get_Boolean_PD
  (
  const char    *prompt,                     /* User prompt */
  Boolean_t      dfault                      /* Default value */
  )
{
  SynCondValue   status;                     /* Status of string input */
  U16_t          i;                          /* Counter */
  U16_t          length;                     /* Number of bytes input */
  U8_t           inbuf[MX_INPUT_SIZE - 1];   /* Input buffer */
  Boolean_t      owtput;                     /* User input interpreted */

  printf ("%s: ", prompt);
  status = SInput_String (inbuf, sizeof (inbuf), &GStdIn);
  if (status == SYN_EOF)
    IO_Exit_Error (R_IO, X_USERREQ, "User entered ^D");

  length = strlen ((char *)inbuf);

  for (i = 0; i < length; i++)
    inbuf[i] = isalpha (inbuf[i]) && isupper (inbuf[i])
      ? tolower (inbuf[i]) : inbuf[i];

  if (inbuf[0] == 't' || inbuf[0] == 'y' || inbuf[0] == '1')
    owtput = TRUE;
  else
    owtput = FALSE;

  if (!length)
    owtput = dfault;

  return owtput;
}

/****************************************************************************
*
*  Function Name:                 IO_Get_Integer
*
*    This routine handles getting a value from the specified file.  The
*    default value is 0.
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
*    Integer - the value input or the default if none was provided
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U32_t IO_Get_Integer
  (
  FileDsc_t     *filed_p                     /* File to get info from */
  )
{
  U32_t          inbuf;                      /* Input buffer */
  SynCondValue   status;                     /* Status of call */

  status = SInput_Integer (&inbuf, sizeof (inbuf), filed_p);
  if (status == SYN_NOINPUT)
    inbuf = 0;

  return inbuf;
}

/****************************************************************************
*
*  Function Name:                 IO_Get_Integer_PD
*
*    This routine handles getting a value from the user and providing a
*    default if they don't specify anything.  Since it does prompting and
*    defaulting, it has the suffix PD.
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
*    Integer - the value input or the default if none was provided
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U32_t IO_Get_Integer_PD
  (
  const char    *prompt,                     /* User prompt */
  int            dfault                      /* Default value */
  )
{
  U32_t          inbuf;                      /* Input buffer */
  SynCondValue   status;                     /* Status of call */

  printf ("%s: ", prompt);
  status = SInput_Integer (&inbuf, sizeof (inbuf), &GStdIn);
  if (status == SYN_NOINPUT)
    inbuf = dfault;

  return inbuf;
}

/****************************************************************************
*
*  Function Name:                 IO_Get_String
*
*    This function knows how to get a string input from the specified file.
*    The buffer must be preallocated and if it is overflowed, then the
*    program will be halted.
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
*    Integer - length of data in the string
*
*  Side Effects:
*
*    May call IO_Exit_Error
*
******************************************************************************/
U16_t IO_Get_String
  (
  U8_t          *owtput                      /* Address of buffer */
  )
{
  SynCondValue   status;                     /* Status of call */
  int            ilen;                       /* Input length */
  U8_t           ibuf[MX_INPUT_SIZE - 1];    /* gets buffer */

  status = SInput_String (ibuf, sizeof (ibuf), &GStdIn);
  if (status == SYN_EOF)
    IO_Exit_Error (R_IO, X_USERREQ, "User entered ^D");

  ilen = strlen ((char *)ibuf);
  owtput[ilen] = '\0';
  memcpy (owtput, ibuf, ilen);

  return ilen;
}

/****************************************************************************
*
*  Function Name:                 IO_Get_String_PDA
*
*    This function knows how to get a string input from the user, and to
*    provide a default value if no input is received.  It can also handle
*    allocating the buffer for the string if requested to do so.  It does
*    prompting, defaulting and allocating, hence the PDA.
*
*    Note: The trailing null is only provided when the storage is allocated
*    by the caller, otherwise only the length returned can be used to find
*    the end of the string.
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
*    Integer - length of data in the string
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
U16_t IO_Get_String_PDA
  (
  const char    *prompt,                     /* Prompt for user */
  char         **owtput,                     /* Address of pointer to buffer */
  Boolean_t      allocated,                  /* User allocated? */
  const char    *dfault                      /* Default string value */
  )
{
  SynCondValue   status;                     /* Status of call */
  int            ilen;                       /* Input length */
  U8_t          *inp_p;                      /* Pointer to valid buffer */
  U8_t           ibuf[MX_INPUT_SIZE - 1];    /* gets buffer */

  printf ("%s: ", prompt);
  status = SInput_String (ibuf, sizeof (ibuf), &GStdIn);
  if (status == SYN_EOF)
    IO_Exit_Error (R_IO, X_USERREQ, "User entered ^D");

  ilen = strlen ((char *)ibuf);

  if (ilen == 0)
    {
    inp_p = (U8_t *)dfault;
    ilen = strlen (dfault);
    }
  else
    inp_p = ibuf;

  if (allocated == FALSE)
    {
#ifdef _MIND_MEM_
    mind_malloc ("*owtput", "synio{1}", owtput, ilen + 1);
#else
    Mem_Alloc (char *, *owtput, ilen + 1, GLOBAL);
#endif

    DEBUG (R_IO, DB_IO_GET, TL_MEMORY, (outbuf,
      "Allocated memory for an input string in IO_Get_String_PDA at %p",
      owtput));

    if (*owtput == NULL)
      IO_Exit_Error (R_IO, X_LIBCALL,
        "No memory for input buffer in IO_Get_String_PDA");
    }

  (void) strncpy (*owtput, (char *)inp_p, ilen);
  (*owtput)[ilen] = '\0';

  return ilen;
}

/****************************************************************************
*
*  Function Name:                 IO_Init
*
*    This routine initializes all of the external variables and stuff for
*    the SYNIO module.
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
void IO_Init
  (
  )
{
  GExitReasons[X_RESERVED] = "Reserved";
  GExitReasons[X_SYSCALL] = "System call failed";
  GExitReasons[X_LIBCALL] = "Library call failed";
  GExitReasons[X_USERREQ] = "User request";
  GExitReasons[X_SYNERR] = "Synchem Internal Error";
  GExitReasons[X_MEMTRASHED] = "Heap memory was stomped on";

  IO_FileHandle_Put (&GStdIn, stdin);
  strcpy (IO_FileName_Get (&GStdIn), "Standard Input");

  IO_FileHandle_Put (&GStdOut, stdout);
  strcpy (IO_FileName_Get (&GStdOut), "Standard Output");

  IO_FileHandle_Put (&GStdErr, stderr);
  strcpy (IO_FileName_Get (&GStdErr), "Standard Error");

  return;
}

/****************************************************************************
*
*  Function Name:                 IO_Init_Files
*
*    This routine is responsible for initializing files that are used 
*    during the run of SYNCHEM.
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
void IO_Init_Files
  (
  char         *tracefn,
  Boolean_t     restart
  )
{
  if (restart)
#ifdef _WIN32
    IO_FileHandle_Put (&GTraceFile, fopen (gccfix (tracefn), "a+")); 
  else
    IO_FileHandle_Put (&GTraceFile, fopen (gccfix (tracefn), "w+")); 
#else
    IO_FileHandle_Put (&GTraceFile, fopen (tracefn, "a+")); 
  else
    IO_FileHandle_Put (&GTraceFile, fopen (tracefn, "w+")); 
#endif

  if (IO_FileHandle_Get (&GTraceFile) == NULL)
    {
    perror ("Failed to open the trace file");
    IO_Exit_Error (R_IO, X_SYSCALL, "Failed to open the trace file");
    }

  DEBUG (R_IO, DB_IO_DUMP, TL_INIT, (outbuf,
    "Run files initialized successfully"));

  return;
}

/****************************************************************************
*
*  Function Name:                 IO_Put_Debug
*
*      This routine prints out debugging messages.
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
void IO_Put_Debug
  (
  int            mod_index,                  /* Module to name */
  const char    *outbuf                      /* buffer to print */
  )
{
  fprintf (IO_FileHandle_Get (&GTraceFile),
    "DEBUG: %s => %s\n", GModuleNames[mod_index], outbuf);

  return;
}

/****************************************************************************
*
*  Function Name:                 IO_Put_Trace
*
*    This routine prints out tracing information to the trace file.
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
void IO_Put_Trace
  (
  int            rtn_index,                  /* Index into routine names */
  const char    *outbuf                      /* buffer to print */
  )
{
  if (rtn_index == R_NONE)
    fprintf (IO_FileHandle_Get (&GTraceFile),
      "%s\n", outbuf);
  else
    fprintf (IO_FileHandle_Get (&GTraceFile),
      "%s => %s\n", GModuleNames[rtn_index], outbuf);

  return;
}

/****************************************************************************
*
*  Function Name:                 IO_Put_Trace_Time
*
*    This routine prints out a time stamped message, to the status file.
*
*  Used to be:
*
*    timechk:
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
void IO_Put_Trace_Time
  (
  const char    *message                     /* System message */
  )
{
  /*
  U32_t          secs;                       

  
  secs = time (&secs);
  fprintf (IO_FileHandle_Get (&GTraceFile), "Secs (%d) => %s\n", secs, message);
  */

  fprintf (IO_FileHandle_Get (&GTraceFile), "\n  ===> %s\n", message);
  return;
}

/****************************************************************************
*
*  Function Name:                 Trace_Init
*
*    This routine initializes the tracing parameters.
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
void Trace_Init
  (
  )
{
  /* MAIN */

  GTrace[DB_RESERVED].options = TL_NONE;
  GTrace[DB_MAIN].options = TL_NONE;
  GTrace[DB_CHEMISTRY].options = TL_ALWAYS;
  GTrace[DB_INIT].options = TL_NONE;

  /* SYNIO */

  GTrace[DB_IO_GET].options = TL_NONE;
  GTrace[DB_IO_PUT].options = TL_NONE;
  GTrace[DB_IO_DUMP].options = TL_NONE;

  /* ARRAY */

  GTrace[DB_ARRAY1DCREATE].options = TL_NONE;
  GTrace[DB_ARRAY2DCREATE].options = TL_NONE;
  GTrace[DB_ARRAY3DCREATE].options = TL_NONE;
  GTrace[DB_ARRAYDESTROY].options = TL_NONE;
  GTrace[DB_ARRAYCOPY].options = TL_NONE;
  GTrace[DB_ARRAYCOPYCONTENTS].options = TL_NONE;

  /* UTL */

  GTrace[DB_ATMARRCOPY].options = TL_NONE;
  GTrace[DB_ATMARRCREATE].options = TL_NONE;
  GTrace[DB_ATMARRDESTROY].options = TL_NONE;
  GTrace[DB_STACKCREATE].options = TL_NONE;
  GTrace[DB_STACKDESTROY].options = TL_NONE;
  GTrace[DB_STACKPOP].options = TL_NONE;
  GTrace[DB_STACKPUSH].options = TL_NONE;
  GTrace[DB_LNKDTCOPY].options = TL_NONE;
  GTrace[DB_LNKDTCREATE].options = TL_NONE;
  GTrace[DB_LNKDTDESTROY].options = TL_NONE;
  GTrace[DB_STRINGCONCAT].options = TL_NONE;
  GTrace[DB_STRINGCOPY].options = TL_NONE;
  GTrace[DB_STRINGCREATE].options = TL_NONE;
  GTrace[DB_STRINGDESTROY].options = TL_NONE;

  /* AROMATICS */

  GTrace[DB_AROMCOPY].options = TL_NONE;
  GTrace[DB_AROMCREATE].options = TL_NONE;
  GTrace[DB_AROMDESTROY].options = TL_NONE;
  GTrace[DB_AROMSET].options = TL_NONE;
  GTrace[DB_AROMFINDBONDS].options = TL_NONE;

  /* ATOMS */

  GTrace[DB_ATOMCOPY].options = TL_NONE;
  GTrace[DB_ATOMCREATE].options = TL_NONE;
  GTrace[DB_ATOMDESTROY].options = TL_NONE;
  GTrace[DB_ATOMSET].options = TL_NONE;

  /* AVLCOMP */

  GTrace[DB_AVCPINFDESTROY].options = TL_NONE;
  GTrace[DB_AVCPINFEXTRACT].options = TL_NONE;
  GTrace[DB_AVCPINFEXTRUDE].options = TL_NONE;

  /* FUNCGROUPS */

  GTrace[DB_FNGRPCOPY].options = TL_NONE;
  GTrace[DB_FNGRPCREATE].options = TL_NONE;
  GTrace[DB_FNGRPDESTROY].options = TL_NONE;
  GTrace[DB_FNGRPSUBINSTOK].options = TL_NONE;
  GTrace[DB_FNGRPINIT].options = TL_NONE;
  GTrace[DB_FNGRPSTATIC].options = TL_NONE;

  /* NAME */

  GTrace[DB_NAMECOPY].options = TL_NONE;
  GTrace[DB_NAMECREATE].options = TL_NONE;
  GTrace[DB_NAMEDESTROY].options = TL_NONE;
  GTrace[DB_NAMESET].options = TL_NONE;
  GTrace[DB_NAMESLINGGET].options = TL_NONE;
  GTrace[DB_NAMESLINGMAP].options = TL_NONE;

  /* PST */

  GTrace[DB_PSTBROTHERGET].options = TL_NONE;
  GTrace[DB_PSTBROTHERPUT].options = TL_NONE;
  GTrace[DB_PSTCOMPCREATE].options = TL_NONE;
  GTrace[DB_PSTFATHERGET].options = TL_NONE;
  GTrace[DB_PSTFATHERPUT].options = TL_NONE;
  GTrace[DB_PSTSONGET].options = TL_NONE;
  GTrace[DB_PSTSONPUT].options = TL_NONE;
  GTrace[DB_PSTNUMSONS].options = TL_NONE;
  GTrace[DB_PSTSUBGCREATE].options = TL_NONE;
  GTrace[DB_PSTCOMPMERITSET].options = TL_NONE;
  GTrace[DB_PSTROOTSET].options = TL_NONE;
  GTrace[DB_PSTSYMTABCREATE].options = TL_NONE;
  GTrace[DB_PSTSUBGINSERT].options = TL_NONE;

  /* REACTION */

  GTrace[DB_REACTDESTROY].options = TL_ALWAYS;
  GTrace[DB_REACTREAD].options = TL_ALWAYS;
  GTrace[DB_REACTXTREAD].options = TL_ALWAYS;
  GTrace[DB_REACTWRITE].options = TL_ALWAYS;
  GTrace[DB_REACTXTWRITE].options = TL_ALWAYS;

  /* RESONANTBONDS */

  GTrace[DB_RESBONDSFIND].options = TL_NONE;
  GTrace[DB_RESBONDSFINDSPIRO].options = TL_NONE;
  GTrace[DB_RESBONDSTATIC].options = TL_NONE;
  GTrace[DB_RESBONDSFIX].options = TL_NONE;

  /* RINGDEFINITION */

  GTrace[DB_RINGDEFCOPY].options = TL_NONE;
  GTrace[DB_RINGDEFCREATE].options = TL_NONE;
  GTrace[DB_RINGDEFDESTROY].options = TL_NONE;
  GTrace[DB_RINGDEFCOMFIND].options = TL_NONE;
  GTrace[DB_RINGDEFMINRINGFIND].options = TL_NONE;
  GTrace[DB_RINGDEFMIN4RINGFIND].options = TL_NONE;
  GTrace[DB_RINGDEFPRMRNGNDSGET].options = TL_NONE;
  GTrace[DB_RINGDEFSTATIC].options = TL_NONE;
  GTrace[DB_RINGDEFPRMCYCFIND].options = TL_NONE;
  GTrace[DB_RINGDEFCITERNG].options = TL_NONE;

  /* RINGS */

  GTrace[DB_RINGCOPY].options = TL_NONE;
  GTrace[DB_RINGCREATE].options = TL_NONE;
  GTrace[DB_RINGDESTROY].options = TL_NONE;
  GTrace[DB_RINGSYSFIND].options = TL_NONE;
  GTrace[DB_RINGBONDSET].options = TL_NONE;
  GTrace[DB_RINGSTATIC].options = TL_NONE;

  /* RINGSYSTEMS */

  GTrace[DB_RINGSYSCOPY].options = TL_NONE;
  GTrace[DB_RINGSYSCREATE].options = TL_NONE;
  GTrace[DB_RINGSYSDESTROY].options = TL_NONE;
  GTrace[DB_RINGSYSSTATIC].options = TL_NONE;

  /* SLING */

  GTrace[DB_SLINGCOPY].options = TL_NONE;
  GTrace[DB_SLINGCREATE].options = TL_NONE;
  GTrace[DB_SLINGDESTROY].options = TL_NONE;
  GTrace[DB_SLINGCANNAME2XTR].options = TL_NONE;
  GTrace[DB_SLING2TSD].options = TL_NONE;
  GTrace[DB_SLING2TSDPLHY].options = TL_NONE;
  GTrace[DB_SLING2XTR].options = TL_NONE;
  GTrace[DB_SLING2XTRPLHY].options = TL_NONE;
  GTrace[DB_SLINGSTATIC].options = TL_NONE;
  GTrace[DB_SLINGCANONGEN].options = TL_NONE;
  GTrace[DB_SLINGVALIDATE].options = TL_NONE;

  /* TSD */

  GTrace[DB_TSDATOMCONN].options = TL_NONE;
  GTrace[DB_TSDATOMCONCHNG].options = TL_NONE;
  GTrace[DB_TSDATOMDISC].options = TL_NONE;
  GTrace[DB_TSDATOMNEIGHGET].options = TL_NONE;
  GTrace[DB_TSDBONDCHANGE].options = TL_NONE;
  GTrace[DB_TSDATOMVALGET].options = TL_NONE;
  GTrace[DB_TSDCOPY].options = TL_NONE;
  GTrace[DB_TSDCREATE].options = TL_NONE;
  GTrace[DB_TSDDESTROY].options = TL_NONE;
  GTrace[DB_TSDEXPAND].options = TL_NONE;
  GTrace[DB_TSDMATCOMP].options = TL_NONE;
  GTrace[DB_TSDVERIFY].options = TL_NONE;
  GTrace[DB_TSD2XTR].options = TL_NONE;
  GTrace[DB_TSD2SLING].options = TL_NONE;
  GTrace[DB_TSD2SLINGX].options = TL_NONE;
  GTrace[DB_TSDSTATIC].options = TL_NONE;

  /* XTR */

  GTrace[DB_XTRCOMPRESS].options = TL_NONE;
  GTrace[DB_XTRCOPY].options = TL_NONE;
  GTrace[DB_XTRCREATE].options = TL_NONE;
  GTrace[DB_XTRDESTROY].options = TL_NONE;
  GTrace[DB_XTRCOPYSUBSET].options = TL_NONE;
  GTrace[DB_XTRCOPYEXPAND].options = TL_NONE;
  GTrace[DB_XTRRESBONDSET].options = TL_NONE;
  GTrace[DB_XTRRINGSET].options = TL_NONE;
  GTrace[DB_XTRRINGDEFSET].options = TL_NONE;
  GTrace[DB_XTR2SLING].options = TL_NONE;
  GTrace[DB_XTR2TSD].options = TL_NONE;
  GTrace[DB_XTRSTATIC].options = TL_NONE;

  /* ISAM */

  GTrace[DB_ISAMCLOSE].options = TL_NONE;
  GTrace[DB_ISAMOPEN].options = TL_NONE;
  GTrace[DB_ISAMREAD].options = TL_NONE;
  GTrace[DB_ISAMREADNEXT].options = TL_NONE;
  GTrace[DB_ISAMREADNOBUF].options = TL_NONE;
  GTrace[DB_ISAMRECINBUF].options = TL_NONE;
  GTrace[DB_ISAMINBUFSIZE].options = TL_NONE;
  GTrace[DB_ISAMWRITE].options = TL_NONE;

  /* FUNCGROUP FILES */

  GTrace[DB_FNGRPRECREAD].options = TL_NONE;
  GTrace[DB_FNGRPRECWRITE].options = TL_NONE;

  return;
}

/****************************************************************************
*
*  Function Name:                 SInput_Integer
*
*    This routine knows how to get an integer from a file.  It will 
*    allow restarts when a non-integer character is input, and if a ^D is
*    entered it will exit from the program.  This is best used for stdin, but
*    it can deal with integers that are the only thing on a line in any file.
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
*    SynNoInput - to allow a entered value of zero to be distinguished from
*                 no input
*
*    SynNormal  - which indicates that something was entered
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static SynCondValue SInput_Integer
  (
  U32_t         *result,                     /* Address of output buffer */
  U8_t           size,                       /* Actual size of buffer */
  FileDsc_t     *filed_p                     /* File to get info from */
  )
{
  U32_t          owtput;                     /* Buffer to collect output in */
  int            inbuf;                      /* Input buffer */
  SynCondValue   status;                     /* Status of call */
  U8_t           neg;                        /* Negative number flag */

  neg = FALSE, owtput = 0, status = SYN_NOINPUT;
  while ((inbuf = fgetc (IO_FileHandle_Get (filed_p))) != EOF && inbuf != NL)
    {
    if (!owtput && inbuf == '-')
      neg = TRUE, inbuf = '0';

    if (!isdigit (inbuf))
      {
      fprintf (IO_FileHandle_Get (&GStdOut),
        "Input discarded, decimal digits and minus sign only.\n");
      for ( ; (inbuf = fgetc (IO_FileHandle_Get (filed_p))) != NL; )
        /* Empty Loop Body */ ;

      fprintf (IO_FileHandle_Get (&GStdOut), "Please try again. -> ");
      owtput = 0, status = SYN_NOINPUT, neg = FALSE;
      }
    else
      {
      owtput = owtput * 10 + inbuf - '0';
      status = SYN_NORMAL;
      }
    }

  if (neg)
    owtput = -owtput;

  if (size == sizeof (U32_t))
    *result = owtput;
  else
    if (size == sizeof (U16_t))
      *(U16_t *)result = (U16_t) owtput;
  else
    if (size == sizeof (U8_t))
      *(U8_t *)result = (U8_t) owtput;

  return status;
}

/****************************************************************************
*
*  Function Name:                 SInput_String
*
*    This routine knows how to input a string from a file.  It uses the
*    potentially dangerous gets () routine, and if the input buffer is
*    overflowed it will terminate the program.
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
static SynCondValue SInput_String
  (
  U8_t          *result,                     /* Address of output buffer */
  U16_t          size,                       /* Actual size of buffer */
  FileDsc_t     *filed_p                     /* File to get info from */
  )
{
  char          *eofdetect;                  /* Pointer to det end of file */
  int            length;                     /* Length of input string */
  U8_t           inbuf[MX_INPUT_SIZE];       /* Input buffer */

  if (size >= MX_INPUT_SIZE)
    IO_Exit_Error (R_IO, X_SYNERR,
      "Compile-time input buffer size is too small");

  inbuf[MX_INPUT_SIZE - 1] = NL;
  length = size + 1;
  while (length > size)
    {
    eofdetect = fgets ((char *)inbuf, size, IO_FileHandle_Get (filed_p));
    if (eofdetect == NULL)
      return SYN_EOF;

    length = (U8_t *)strchr ((char *)inbuf, NL) - inbuf;
    if (length >= size - 1)
      {
      fprintf (IO_FileHandle_Get (&GStdOut),
        "Input too long, discarded, please try again. ->");
      inbuf[0] = '\0';
      }
    }

  inbuf[length++] = '\0';
  (void) memcpy (result, inbuf, length);
  return SYN_NORMAL;
}
/* End of SInput_String */

#ifdef _WIN32
char *gccfix (char *filename)
{
  static char transname[250];
  char *s;
  int i;

  if (filename[1]==':')
    sprintf (transname, "//%c/%s", toupper (filename[0]),
      filename[2] == '\\' || filename[2] == '/' ? filename + 3 : filename + 2);
  else strcpy (transname, filename);
  for (i = 0; i < strlen (transname); i++)
    if (transname[i] == '\\') transname[i] = '/';
  s = strstr (transname, "synchem");
  if (s != NULL) strncpy (s, "SYNCHEM", 7);

  return (transname);
}
#endif

/* End of SYNIO.C */
