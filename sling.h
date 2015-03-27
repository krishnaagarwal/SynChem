#ifndef _H_SLING_
#define _H_SLING_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     SLING.H
*
*    This module is the abstraction for the Sling data-structure.
*    Sling stands for (Synchem LINear Graph) which is a typeable set of
*    characters that can fully describe a molecule (in 2-D space).  It also
*    has the property of having a unique or canonical representation for each
*    and every molecule, including taking into account stereochemistry.
*
*    Routines are found in SLING.C unless otherwise noted
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
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 06-Jun-96  Krebsbach  Changed atom symols "Va" to "V" for Vanadium.
*
******************************************************************************/

#ifndef _H_SYNIO_
#include "synio.h"
#endif

#ifndef _H_ARRAY_
#include "array.h"
#endif

#ifndef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_TSD_
#include "tsd.h"
#endif

/*** Literal Values ***/

#define NOCEBS             "no ceb's"
#define NOSEBS             "no seb's"

#define CE_SEP             "|"
#define SE_SEP             "|"
#define PARITY_SEP         "$"
#define EQUIVCLASS_SEP     ",,"
#define CLASSMEMBER_SEP    ","

#define BOND_NONE_SYM      '_'
#define BOND_SINGLE_SYM    '\0'
#define BOND_DOUBLE_SYM    '='
#define BOND_TRIPLE_SYM    '*'
#define BOND_VARIABLE_SYM  '>'
#define BOND_RESONANT_SYM  '<'
#define BOND_SINGLE_OR_RESONANT_SYM  '%'

#define DONTCARE_SYM       '?'
#define SLASH_STR          "/"
#define SLASH_SYM          '/'
#define PARITY_SPACE_STR   "~"
#define PARITY_SPACE_SYM   '~'
#define RETRACE_STR        "-"
#define RETRACE_SYM        '-'
#define FORTRACE_SYM       '+'
#define GROUP_START_SYM    '('
#define GROUP_END_SYM      ')'

#define MX_CANON_ATOM             103
#define MX_CANON_ID               512
#define CANON_SHORT_DIFF          2048
#define CANON_MINASI_INIT         10240
#define CANON_NEIGHWEIGHT_INIT    32768
#define CANON_ATOMID_PERM         10
#define CANON_TRIVALENT           12
#define CANON_TETRAVALENT         4

/* Canonical atom identifiers */

#define CANON_CARBON              1
#define CANON_HYDROGEN            27

/*** Data Structures ***/

/* Sling descriptor. */

typedef struct s_sling
  {
  U8_t          *name;                       /* Sling contents */
  U16_t          length;                     /* Length of sling */
  } Sling_t;
#define SLINGSIZE sizeof (Sling_t)

/** Field Access Macros for a Sling_t **/

/* Macro Prototypes
   U16_t Sling_Alloc_Get  (Sling_t);
   U16_t Sling_Length_Get (Sling_t);
   void  Sling_Length_Put (Sling_t, U16_t);
   U8_t *Sling_Name_Get   (Sling_t);
   void  Sling_Name_Put   (Sling_t, U8_t *);
*/

#ifndef XTR_DEBUG
#define Sling_Alloc_Get(slng)\
  ((slng).length + 1)

#define Sling_Length_Get(slng)\
  (slng).length

#define Sling_Length_Put(slng, value)\
  (slng).length = (value)

#define Sling_Name_Get(slng)\
  (slng).name

#define Sling_Name_Put(slng, value)\
  (slng).name = (value)
#else
/* ??? Want to check &(slng), but compiler fails us on some occaisions */
#define Sling_Alloc_Get(slng)\
  (&(slng) < GBAddr ? HALT : (slng).length + 1)

#define Sling_Length_Get(slng)\
  ((slng).name < GBAddr ? HALT : (slng).length)

#define Sling_Length_Put(slng, value)\
  { if (&(slng) < GBAddr) HALT; else (slng).length = (value); }

#define Sling_Name_Get(slng)\
   ((slng).name < GBAddr ? HALT : (slng).name)

#define Sling_Name_Put(slng, value)\
  { if (&(slng) < GBAddr) HALT; else (slng).name = (value); }
#endif

/** End of Field Access Macros for Sling_t **/

/* The conversion of a Sling to a TSD requires a control block to minimize
   the number of parameters passed from one routine to another and to make
   the code easier to read (unlike the Aromatic bonds code).
*/

typedef struct s_slng2tsd
  {
  U8_t         *curoffset;                 /* Current position in Sling */
  U8_t         *endoffset;                 /* Pointer to last char in Sling */
  U8_t          next_char;                 /* Last char read from curoffset */
  } Slng2TsdCB_t;

/** Field Access Macros for Slng2TsdCB_t **/
/* Macro Prototypes
*/

#ifndef XTR_DEBUG
#define S2TCB_AtSlingEnd(p)\
  (S2TCB_Offset_Get (p) > S2TCB_End_Get (p))

#define S2TCB_End_Get(p)\
  (p)->endoffset

#define S2TCB_End_Put(p, value)\
  (p)->endoffset = (value)

#define S2TCB_NextChar_Get(p)\
  (p)->next_char

#define S2TCB_NextChar_Put(p, value)\
  (p)->next_char = (value)

#define S2TCB_NextChar_Set(p)\
  (p)->next_char = *(++((p)->curoffset))

#define S2TCB_Offset_Get(p)\
  (p)->curoffset

#define S2TCB_Offset_Incr(p)\
  (p)->curoffset++

#define S2TCB_Offset_Put(p, value)\
  (p)->curoffset = (value)
#else
#define S2TCB_AtSlingEnd(p)\
  ((p) < GBAddr ? HALT : S2TCB_Offset_Get (p) > S2TCB_End_Get (p))

#define S2TCB_End_Get(p)\
  ((p) < GBAddr ? HALT : (p)->endoffset)

#define S2TCB_End_Put(p, value)\
  { if ((p) < GBAddr) HALT; else (p)->endoffset = (value); }

#define S2TCB_NextChar_Get(p)\
  ((p) < GBAddr ? HALT : (p)->next_char)

#define S2TCB_NextChar_Put(p, value)\
  { if ((p) < GBAddr) HALT; else (p)->next_char = (value); }

#define S2TCB_NextChar_Set(p)\
  { if ((p) < GBAddr) HALT; else (p)->next_char = *(++((p)->curoffset)); }

#define S2TCB_Offset_Get(p)\
  ((p) < GBAddr ? HALT : (p)->curoffset)

#define S2TCB_Offset_Incr(p)\
  { if ((p) < GBAddr) HALT; else (p)->curoffset++; }

#define S2TCB_Offset_Put(p, value)\
  { if ((p) < GBAddr) HALT; else (p)->curoffset = (value); }
#endif

/** End of Field Access Macros for Slng2TsdCB_t **/

/* Coded TSD data-structure.  This is for a list of "better" TSD encodings
   during the search for the canonical or "minimum" encoding.
*/

typedef struct s_tsdcode
  {
  struct s_tsdcode *next;                   /* Next in linked list */
  Tsd_t        *tsd;                        /* Actual TSD */
  AtomArray_t  *parity;                     /* 1-d byte, parity indicators */
  U16_t         last_nonh;                  /* Last non-hydrogen index */
  Array_t       mapb;                       /* 1-d word, mapping array */
  Array_t       bondweightb;                /* 1-d word, bondweights - TSD */
  } CodedTsd_t;
#define CODEDTSDSIZE sizeof (CodedTsd_t)

/** Field Access Macros for a CodedTsd_t **/

/* Macro Prototypes
   Array_t     *CodeTsd_BondWeight_Get (CodedTsd_t *);
   U16_t        CodeTsd_LastNonH_Get   (CodedTsd_t *);
   void         CodeTsd_LastNonH_Put   (CodedTsd_t *, U16_t);
   Array_t     *CodeTsd_Map_Get        (CodedTsd_t *);
   CodedTsd_t  *CodeTsd_Next_Get       (CodedTsd_t *);
   void         CodeTsd_Next_Put       (CodedTsd_t *, CodedTsd_t *);
   AtomArray_t *CodeTsd_Parity_Get     (CodedTsd_t *);
   void         CodeTsd_Parity_Put     (CodedTsd_t *, AtomArray_t *);
   Tsd_t       *CodeTsd_Tsd_Get        (CodedTsd_t *);
   void         CodeTsd_Tsd_Put        (CodedTsd_t *, Tsd_t *);
*/

#ifndef XTR_DEBUG
#define CodeTsd_BondWeight_Get(code_p)\
  &(code_p)->bondweightb

#define CodeTsd_LastNonH_Get(code_p)\
  (code_p)->last_nonh

#define CodeTsd_LastNonH_Put(code_p, value)\
  (code_p)->last_nonh = (value)

#define CodeTsd_Map_Get(code_p)\
  &(code_p)->mapb

#define CodeTsd_Next_Get(code_p)\
  (code_p)->next

#define CodeTsd_Next_Put(code_p, value)\
  (code_p)->next = (value)

#define CodeTsd_Parity_Get(code_p)\
  (code_p)->parity

#define CodeTsd_Parity_Put(code_p, value)\
  (code_p)->parity = (value)

#define CodeTsd_Tsd_Get(code_p)\
  (code_p)->tsd

#define CodeTsd_Tsd_Put(code_p, value)\
  (code_p)->tsd = (value)
#else
#define CodeTsd_BondWeight_Get(code_p)\
  ((code_p) < GBAddr ? HALT : &(code_p)->bondweightb)

#define CodeTsd_LastNonH_Get(code_p)\
  ((code_p) < GBAddr ? HALT : (code_p)->last_nonh)

#define CodeTsd_LastNonH_Put(code_p, value)\
  { if ((code_p) < GBAddr) HALT; else (code_p)->last_nonh = (value); }

#define CodeTsd_Map_Get(code_p)\
  ((code_p) < GBAddr ? HALT : &(code_p)->mapb)

#define CodeTsd_Next_Get(code_p)\
  ((code_p) < GBAddr ? HALT : (code_p)->next)

#define CodeTsd_Next_Put(code_p, value)\
  { if ((code_p) < GBAddr) HALT; else (code_p)->next = (value); }

#define CodeTsd_Parity_Get(code_p)\
  ((code_p) < GBAddr ? HALT : (code_p)->parity)

#define CodeTsd_Parity_Put(code_p, value)\
  { if ((code_p) < GBAddr) HALT; else (code_p)->parity = (value); }

#define CodeTsd_Tsd_Get(code_p)\
  ((code_p) < GBAddr ? HALT : (code_p)->tsd)

#define CodeTsd_Tsd_Put(code_p, value)\
  { if ((code_p) < GBAddr) HALT; else (code_p)->tsd = (value); }
#endif

/** End of Field Access Macros for CodedTsd_t **/

/*** Macros ***/

#define SLING_LOGICAL_END_CHECK(p)\
  if (S2TCB_AtSlingEnd (p))\
    {\
    Sling_Dump (sling, &GStdErr);\
    IO_Put_Debug (R_SLING, \
      "Error in Sling2Tsd, end of sling found before logical end");\
    return FALSE;\
    }

/*** Routine Prototypes ***/

Sling_t    IO_Exit_Error_Sling (int, int, const char *);
void       Sling_Canonical_Generate (Tsd_t *, Tsd_t **, AtomArray_t **,
  Array_t *, Array_t *, String_t *, Array_t *, String_t *, String_t *,
  Array_t *, Array_t *, Boolean_t, Boolean_t *, U16_t *, U16_t *);
Sling_t    Sling_Copy     (Sling_t);
Sling_t    Sling_CopyTrunc (Sling_t);
Sling_t    Sling_Create   (U16_t);
void       Sling_Destroy  (Sling_t);
void       Sling_Dump     (Sling_t, FileDsc_t *);
Boolean_t  Sling_Validate (Sling_t, U16_t *);
String_t   Sling2String   (Sling_t);
Tsd_t     *Sling2Tsd      (Sling_t);
Tsd_t     *Sling2Tsd_PlusHydrogen (Sling_t);
Sling_t    String2Sling   (String_t);
Sling_t    Tsd2Sling      (Tsd_t *);
Sling_t    Tsd2SlingX     (Tsd_t *, Array_t *, Array_t *, Array_t *, Array_t *,
  Array_t *, String_t *, String_t *, U16_t, U16_t);

/*** Global Variables ***/

#ifdef SLING_GLOBALS
static const char *SAtomsymbols [MX_CANON_ATOM + 1] = { "Err",
    "C",  "Si", "Ge", "Sn", "Pb", "N",  "P",  "As", "Sb", "Bi", "O",  "S",
    "Se", "Te", "Po", "F",  "Cl", "Br", "I",  "At", "He", "Ne", "Ar", "Kr",
    "Xe", "Rn", "H",  "Li", "Na", "K",  "Rb", "Cs", "Fr", "Be", "Mg", "Ca",
    "Sr", "Ba", "Ra", "Sc", "Y",  "La", "Ce", "Pr", "Nd", "Pm", "Sm", "Eu",
    "Gd", "Tb", "Dy", "Ho", "Er", "Tm", "Yb", "Lu", "Ac", "Th", "Pa", "U",
    "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm", "Md", "No", "Lw", "Ti",
    "Zr", "Hf", "V",  "Nb", "Ta", "Cr", "Mo", "W",  "Mn", "Tc", "Re", "Fe",
    "Ru", "Os", "Co", "Rh", "Ir", "Ni", "Pd", "Pt", "Cu", "Ag", "Au", "Zn",
    "Cd", "Hg", "B",  "Al", "Ga", "In", "Tl"};
#endif

/* End of Sling.H */
#endif
