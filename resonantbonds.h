#ifndef _H_RESONANTBONDS_
#define _H_RESONANTBONDS_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     RESONANTBONDS.H
*
*    This module contains the few little data-structures needed for the
*    finding of resonant bonds in a molecule.  This is a piece of Xtr
*    functionality, but since the code for finding the bonds is so great it
*    warrants its own module.
*
*    Routines are found in RESONANTBONDS.C unless otherwise noted
*
*  Creation Date:
*
*    01-Jan-1991
*
*  Authors:
*
*    Tito Autrey (rewritten based on others PLI code)
*
*  Modification History, reverse chronological
*
* Date       Author	Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred	xxx
*
******************************************************************************/

#ifndef _H_ARRAY_
#include "array.h"
#endif

#ifndef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

/*** Literal Values ***/

#define ONEBRANCH     1
#define TWOBRANCH     2
#define EVENBRANCH    3

#define INVALID_BRANCH 30000

/*** Data Structures ***/

/* In order to avoid having static variables that would ruin the re-entrancy
   of the module a number of one-time global variables have to be bundled
   up into a data-structure to pass from routine to routine.
*/

typedef struct s_resbcb
  {
  Xtr_t        *xtr;                        /* Address of XTR */
  Stack_t      *analhsubg;                  /* Stack for analyzing ??? */
  Stack_t      *gencomp;                    /* Generic compound search */
  Stack_t      *map;                        /* Map of current XTR atom indices
    to original XTR atom indices */
  Stack_t      *parasys;                    /* Parallel system search */
  Stack_t      *val4comp;                   /* Valence 4 compound search */
  Stack_t      *val4ring;                   /* Valence 4 ring search */
  Array_t      *owtput;                     /* 2-d bit array output mask, TRUE
    for bonds that are resonant */
  Array_t       abcb;                       /* Bonds in ABCs */
  Boolean_t     spiro;                      /* Flag for spiro search */
  } ResbondsCB_t;
#define RESBONDSCBSIZE sizeof (ResbondsCB_t)

/** Field Access Macros for ResbondsCB_t **/

/* Macro Prototypes
   Array_t  *RsbndCB_ABCbonds_Get (ResbondsCB_t *);
   Stack_t  *RsbndCB_Analyze_Get  (ResbondsCB_t *);
   void      RsbndCB_Analyze_Put  (ResbondsCB_t *, Stack_t *);
   Stack_t  *RsbndCB_GenComp_Get  (ResbondsCB_t *);
   void      RsbndCB_GenComp_Put  (ResbondsCB_t *, Stack_t *);
   Stack_t  *RsbndCB_Map_Get      (ResbondsCB_t *);
   void      RsbndCB_Map_Put      (ResbondsCB_t *, Stack_t *);
   Array_t  *RsbndCB_Output_Get   (ResbondsCB_t *);
   Stack_t  *RsbndCB_ParaSys_Get  (ResbondsCB_t *);
   void      RsbndCB_ParaSys_Put  (ResbondsCB_t *, Stack_t *);
   Boolean_t RsbndCB_Spiro_Get    (ResbondsCB_t *);
   Stack_t  *RsbndCB_Val4Comp_Get (ResbondsCB_t *);
   void      RsbndCB_Val4Comp_Put (ResbondsCB_t *, Stack_t *);
   Stack_t  *RsbndCB_Val4Ring_Get (ResbondsCB_t *);
   void      RsbndCB_Val4Ring_Put (ResbondsCB_t *, Stack_t *);
   Xtr_t    *RsbndCB_Xtr_Get      (ResbondsCB_t *);
*/

#ifndef XTR_DEBUG
#define RsbndCB_ABCbonds_Get(rbcb_p)\
  &(rbcb_p)->abcb

#define RsbndCB_Analyze_Get(rbcb_p)\
  (rbcb_p)->analhsubg

#define RsbndCB_Analyze_Put(rbcb_p, value)\
  (rbcb_p)->analhsubg = (value)

#define RsbndCB_GenComp_Get(rbcb_p)\
  (rbcb_p)->gencomp

#define RsbndCB_GenComp_Put(rbcb_p, value)\
  (rbcb_p)->gencomp = (value)

#define RsbndCB_Map_Get(rbcb_p)\
  (rbcb_p)->map

#define RsbndCB_Map_Put(rbcb_p, value)\
  (rbcb_p)->map = (value)

#define RsbndCB_Output_Get(rbcb_p)\
  (rbcb_p)->owtput

#define RsbndCB_ParaSys_Get(rbcb_p)\
  (rbcb_p)->parasys

#define RsbndCB_ParaSys_Put(rbcb_p, value)\
  (rbcb_p)->parasys = (value)

#define RsbndCB_Spiro_Get(rbcb_p)\
  (rbcb_p)->spiro

#define RsbndCB_Val4Comp_Get(rbcb_p)\
  (rbcb_p)->val4comp

#define RsbndCB_Val4Comp_Put(rbcb_p, value)\
  (rbcb_p)->val4comp = (value)

#define RsbndCB_Val4Ring_Get(rbcb_p)\
  (rbcb_p)->val4ring

#define RsbndCB_Val4Ring_Put(rbcb_p, value)\
  (rbcb_p)->val4ring = (value)

#define RsbndCB_Xtr_Get(rbcb_p)\
  (rbcb_p)->xtr
#else
#define RsbndCB_ABCbonds_Get(rbcb_p)\
  ((rbcb_p) < GBAddr ? HALT : &(rbcb_p)->abcb)

#define RsbndCB_Analyze_Get(rbcb_p)\
  ((rbcb_p) < GBAddr ? HALT : (rbcb_p)->analhsubg)

#define RsbndCB_Analyze_Put(rbcb_p, value)\
  { if ((rbcb_p) < GBAddr) HALT; else (rbcb_p)->analhsubg = (value); }

#define RsbndCB_GenComp_Get(rbcb_p)\
  ((rbcb_p) < GBAddr ? HALT : (rbcb_p)->gencomp)

#define RsbndCB_GenComp_Put(rbcb_p, value)\
  { if ((rbcb_p) < GBAddr) HALT; else (rbcb_p)->gencomp = (value); }

#define RsbndCB_Map_Get(rbcb_p)\
  ((rbcb_p) < GBAddr ? HALT : (rbcb_p)->map)

#define RsbndCB_Map_Put(rbcb_p, value)\
  { if ((rbcb_p) < GBAddr) HALT; else (rbcb_p)->map = (value); }

#define RsbndCB_Output_Get(rbcb_p)\
  ((rbcb_p) < GBAddr ? HALT : (rbcb_p)->owtput)

#define RsbndCB_ParaSys_Get(rbcb_p)\
  ((rbcb_p) < GBAddr ? HALT : (rbcb_p)->parasys)

#define RsbndCB_ParaSys_Put(rbcb_p, value)\
  { if ((rbcb_p) < GBAddr) HALT; else (rbcb_p)->parasys = (value); }

#define RsbndCB_Spiro_Get(rbcb_p)\
  ((rbcb_p) < GBAddr ? HALT : (rbcb_p)->spiro)

#define RsbndCB_Val4Comp_Get(rbcb_p)\
  ((rbcb_p) < GBAddr ? HALT : (rbcb_p)->val4comp)

#define RsbndCB_Val4Comp_Put(rbcb_p, value)\
  { if ((rbcb_p) < GBAddr) HALT; else (rbcb_p)->val4comp = (value); }

#define RsbndCB_Val4Ring_Get(rbcb_p)\
  ((rbcb_p) < GBAddr ? HALT : (rbcb_p)->val4ring)

#define RsbndCB_Val4Ring_Put(rbcb_p, value)\
  { if ((rbcb_p) < GBAddr) HALT; else (rbcb_p)->val4ring = (value); }

#define RsbndCB_Xtr_Get(rbcb_p)\
  ((rbcb_p) < GBAddr ? HALT : (rbcb_p)->xtr)
#endif

/** End of Field Access Macros for ResbondsCB_t **/

/*** Macros ***/

/*** Routine Prototypes ***/

/*** Global Variables ***/

/* End of ResonantBonds.H */
#endif
