#ifndef _H_SYNIO_
#define _H_SYNIO_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     SYNIO.H
*
*    This module is the description for the IO package that SYNCHEM uses.
*    All routines for input and output are here, for all files.  Except for
*    the data-structure _Dump routines.  And the data-file routines.
*
*    Routines can be found in SYNIO.C unless otherwise noted
*
*  Creation Date:    01-Jan-1991
*
*  Authors:          Tito Autrey (rewritten based on others PLI code)
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred       xxx
*
******************************************************************************/

#include <stdio.h>

/*** Literal Values ***/

#define NL              0xA

/* Maximum values */

#define MX_INPUT_SIZE   1024
#define MX_OUTPUT_SIZE  384

/* Size values */

#define MX_FILENAME     128       /* Length of file name -- 64 is too cheap and risky!!! */
#define MX_USERNAME      64

/*** Data Structures ***/

/* File descriptor, keep all the information about a file in one place */

typedef struct s_fileh
  {
  FILE          *handle;                     /* From system call */
  char           name[MX_FILENAME];          /* Full file specification */
  } FileDsc_t;
#define FILEDSCSIZE sizeof (FileDsc_t)

/** Field Access Macros for a FileDsc_t **/

/* Macro Prototypes
   FILE *IO_FileHandle_Get (FileDsc_t *);
   void  IO_FileHandle_Put (FileDsc_t *, FILE *);
   U8_t *IO_FileName_Get (FileDsc_t *);
*/

#ifndef UTL_DEBUG
#define IO_FileHandle_Get(filed_p)\
  (filed_p)->handle

#define IO_FileHandle_Put(filed_p, value)\
  (filed_p)->handle = (value)

#define IO_FileName_Get(filed_p)\
  (filed_p)->name
#else
#define IO_FileHandle_Get(filed_p)\
  ((filed_p) < GBAddr ? ((FileDsc_t *)HALTP)->handle : (filed_p)->handle)

#define IO_FileHandle_Put(filed_p, value)\
  { if ((filed_p) < GBAddr) HALT; else (filed_p)->handle = (value); }

#define IO_FileName_Get(filed_p)\
  ((filed_p) < GBAddr ? HALT : (filed_p)->name)
#endif

/** End of Field Access Macros for FileDsc_t **/

/*** Macros ***/

/*** Routine Prototypes ***/

void      Debug_Init         (void);
void      IO_Close_Files     (void);
U16_t     IO_Exit_Error      (int, int, const char *);
Boolean_t IO_Get_Boolean_PD  (const char *, Boolean_t);
U32_t     IO_Get_Integer     (FileDsc_t *);
U32_t     IO_Get_Integer_PD  (const char *, int);
U16_t     IO_Get_String      (U8_t *);
U16_t     IO_Get_String_PDA  (const char *, char **, Boolean_t,
  const char *);
void      IO_Init            (void);
void      IO_Init_Files      (char *, Boolean_t);
void      IO_Put_Debug       (int, const char *);
void      IO_Put_Trace       (int, const char *);
void      IO_Put_Trace_Time  (const char *);
void      Trace_Init         (void);
#ifdef _WIN32
char     *gccfix             (char *);
#endif

/*** Global Variables ***/

#ifdef IO_GLOBALS
       FileDsc_t   GTraceFile;               /* Trace file for run */
       FileDsc_t   GStdOut;                  /* Standard out descriptor */
       FileDsc_t   GStdErr;                  /* Standard error descriptor */
       FileDsc_t   GStdIn;                   /* Standard input descriptor */
#else
extern FileDsc_t   GTraceFile;               /* Trace file for run */
extern FileDsc_t   GStdOut;                  /* Standard out descriptor */
extern FileDsc_t   GStdErr;                  /* Standard error descriptor */
extern FileDsc_t   GStdIn;                   /* Standard input descriptor */
#endif

/* End of Synio.H */
#endif
