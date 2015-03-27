#ifndef _H_SYNCHEM_
#define _H_SYNCHEM_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     SYNCHEM.H
*
*      This module contains all the generally useful types and literals for
*      Synchem.  These are the base types or the sizes of base types.
*
*  Creation Date:
*
*      01-Jan-1991
*
*  Authors:
*
*      Daren Krebsbach
*      Tito Autrey (rewritten based on others PLI code)
*
*  Modification History, reverse chronological
*
* Date       Author     Modifcation Description
*------------------------------------------------------------------------------
* 17-Oct-01  Miller     Corrected macro definitions for MIN and MAX to be
*                       parenthesized for proper handling of complex expressions!!!
*
******************************************************************************/

#include <stddef.h>

/* MIPS compiler has trouble with void *. */

#ifdef MIPS
#define Address char *
#else
#define Address void *
#endif

/*** Literal Values ***/

#define FALSE   (Boolean_t)0
#define TRUE    (Boolean_t)1

#define LOCAL            "local"
#define GLOBAL           "global"

#define MX_NEIGHBORS     6        /* # of neighboring atoms to an atom */

/* Need to keep track of invalid values, want them unique so can figure out
   where they are coming from.  Start 8-bit values at -100, 16-bit values
   at -200.  next 8-bit = -105, next 16-bit = -207
*/

#define INFINITY         -1       /* Needed in a few places */

/* Status values */

#define SYN_NORMAL      (SynCondValue)0
#define SYN_GENFAILURE  (SynCondValue)-1
#define SYN_EOF         (SynCondValue)-2
#define SYN_NOTDENSE    (SynCondValue)-3
#define SYN_NOINPUT     (SynCondValue)1

/*** Data Structures ***/

typedef unsigned long  SynCondValue;          /* For status handling */
typedef unsigned long  Date_t;


#ifdef _WORD64BIT_
typedef unsigned int  U32_t;
typedef signed   int  S32_t;
/*
typedef unsigned long U64_t;
typedef signed   long S64_t;
*/
#else
typedef unsigned long  U32_t;
typedef signed   long  S32_t;
/*
typedef unsigned long long U64_t;
typedef signed   long long S64_t;
*/
#endif

typedef unsigned short U16_t;
typedef signed   short S16_t;
typedef unsigned char  U8_t;
typedef signed   char  S8_t;
typedef unsigned char  Boolean_t;             

/*** Macros  ***/

/* ??? No DEBUG versions of macros (general synchem) */

#define FILL(object, datum)\
  (void) memset (&(object), datum, sizeof ((object)))

#define MIN(a, b)\
  ((a) < (b) ? (a) : (b))

#define MAX(a, b)\
  ((a) > (b) ? (a) : (b))

/* To prepare for day with our own memory allocator.  Kind is an attempt
   to distinguish between global and local memory so shmalloc can be used.
*/

#ifdef ALLOC_DEBUG
#define Mem_Alloc(ptr_type, ptr, size, kind)\
  { U8_t *foo; U32_t t,v; t = (U32_t)((size) + 4);\
  foo = (U8_t *) malloc (t + 4);\
  v = 0xDEADBEEF; memcpy (foo, &v, 4); memcpy (&foo[t], &v, 4);\
  foo += 4; ptr = (ptr_type) foo; }

#define Mem_Dealloc(ptr, size, kind)\
  { U8_t *foo; U32_t t,v; t = (U32_t)(size); foo = (U8_t *)(ptr); foo -= 4;\
  v = 0xDEADBEEF; if (t == (U32_t)INFINITY) /*printf ("Tail not checked\n")*/;\
  else if (!(memcmp (foo, &v, 4) == 0 && memcmp (&foo[t + 4], &v, 4) == 0))\
  IO_Exit_Error (R_MAIN, X_MEMTRASHED, "<censored!!!>"); free (foo); \
  foo = NULL;}

#else
#define Mem_Alloc(ptr_type, ptr, size, kind)\
  ptr = (ptr_type) malloc (size)

#define Mem_Dealloc(ptr, size, kind)\
  free (ptr)

#endif


/*** Routine Prototypes ***/

#ifdef _SUN4_OS_
extern fprintf ();      /* Bug with GCC on SPARC */
extern printf ();
extern fscanf ();
extern fgetc ();
extern fputs ();
extern fclose ();
extern fflush ();
extern fread ();
extern fseek ();
extern fwrite ();
extern sscanf ();
extern system ();
extern perror ();
extern time ();
extern tolower ();
extern toupper ();
extern int   strcasecmp   (const char *, const char *);
extern memset ();

#elif defined (_SOLARIS_OS_)
extern int   strcasecmp   (const char *, const char *);
extern char *strdup       (const char *);
#endif

/*** Global Variables ***/

/* End of Synchem.h */
#endif
