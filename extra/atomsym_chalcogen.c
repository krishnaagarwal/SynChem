/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     ATOMSYM.C
*
*    This module supports conversion of atomic numbers to atomic symbols,
*    for example, "He" or ("he"), to 2, and vice-versa.
*
*    This used to be ATOMSYM.PLI, MAXVAL.PLI, SUBGENR.PLI
*
*  Routines:
*
*    Atomid_MaxValence
*    Atomid2Symbol
*    Atomid2Weight
*    AtomSymbol2Id
*    Atomid_IsHalogen
*
*  Creation Date:
*
*    01-Jan-1991
*
*  Authors:
*
*    Tito Autrey (rewritten based on others PL/I code)
*
*  Modification History, reverse chronological
*
* Date       Author	Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred	xxx
*
******************************************************************************/

#include <string.h>
#include <ctype.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_UTL_
#include "utl.h"
#endif

#define ATOM_GLOBALS

#ifndef _H_ATOMSYM_
#include "atomsym_chalcogen.h"
#endif

#undef ATOM_GLOBALS


/****************************************************************************
*
*  Function Name:                 Atomid_MaxValence
*
*    This function calculates the maximum valence for a given atom, including
*    variable nodes.
*
*  Used to be:
*
*    maxval:
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
*    See code
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U8_t Atomid_MaxValence
  (
  U16_t          atomid                     /* Which atom */
  )
{
  U8_t           valence;                   /* Result */

  if (atomid == 0)
    valence = VALENCE_INVALID;
  else if (atomid < SCANDIUM)
    valence = GValence[atomid];
  else if (atomid == BROMINE || atomid == IODINE || atomid == ASTATINE ||
      atomid == GENERIC_HALOGEN)
    valence = 1;
  else if (atomid >= SPECIAL_START && atomid <= SPECIAL_END)
    valence = 1;
  else if (atomid <= LAWRENCIUM)
    valence = MX_VALENCE;
  else if (atomid > LAWRENCIUM)
    valence = VALENCE_INVALID;
  else
    IO_Exit_Error (R_XTR, X_SYNERR,
      "Unexpected atomic element in Atomid_MaxValence");

  return valence;
}

/****************************************************************************
*
*  Function Name:                 Atomid2Symbol
*
*    This function allows conversion between atomic numbers and atomic symbols.
*
*  Used to be:
*
*    atomsymn:
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
*    Address of string for symbol in global variable
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
const char *Atomid2Symbol
  (
  U16_t         atomid                     /* Atomic number */
  )
{
/*
  if (atomid > 255)
*/
  if (atomid >= 255) /* 255-member array ends at 254! */
    return "?";

  return GAtomicSymbols[atomid];
}

/****************************************************************************
*
*  Function Name:                 Atomid2Weight
*
*    This function returns the atomic weight of a given atom.
*
*  Used to be:
*
*    atomwgt:
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
float Atomid2Weight
  (
  U16_t         atomid                     /* Atomic number */
  )
{
  if (atomid > ATOM_END)
    return WEIGHT_INVALID;

  return GAtomicWeights[atomid];
}

/****************************************************************************
*
*  Function Name:                 Atomsymbol2Id
*
*    This function allows conversion from atomic symbols to atomic numbers.
*    And it handles the special types used by SYNCHEM.  The table is layed
*    out such that each symbol takes up 3 characters so by altering the
*    input symbol a simple string search will produce the index.
*
*  Used to be:
*
*    atomsym:
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
U16_t Atomsymbol2Id
  (
  U8_t         *atomsym_p                  /* Atomic symbol (mixed case) */
  )
{
  U16_t         index;                     /* Result */
  U8_t          i;                         /* Counter */
  U8_t          length;                    /* # chars in symbol */
  U8_t          buf[8];                    /* For conversion */

  length = strlen ((char *)atomsym_p);
  if (length == 0)
    return 0;

  switch (atomsym_p[0])
    {
    case '$':

      for (i = 1, index = 0; i < length; i++)
        index = index * 10 + atomsym_p[i] - '0';

      index += VARIABLE_START;
      if (index > VARIABLE_END)
        index = 0;
      break;

    case '&':

      for (i = 1, index = 0; i < length; i++)
        index = index * 10 + atomsym_p[i] - '0';

      index += ALKYL_START;
      if (index > ALKYL_END)
        index = 0;
      break;

    case '#':

      for (index = SUPER_START;
           memcmp (atomsym_p, GAtomicSymbols[index], length) &&
           index <= SUPER_END; index++)
        /* Empty loop body */ ;

      if (index > SUPER_END)
        index = 0;
      break;

    case '+':

      return CATION;
      break;

    case '-':

      return ANION;
      break;

/* Correction: CARBENE and NITRENE are equivalent - both contain an electron pair; RADICAL contains a lone electron;
   there is no symbol containing a comma (as far as I know)! */

    case ':':

      return CARBENE;
/*
      return NITRENE;
*/
      break;

    case '.':

      if (isdigit (atomsym_p[1])) return (atomsym_p[1] - '0' + 100);

/*
      return NITRENE;
*/
      return RADICAL;
      break;

/*
    case ',':

      return RADICAL;
      break;
*/

    default:

      buf[0] = ' ';
      for (i = 0; i < length; i++)
        buf[i + 1] = isupper (atomsym_p[i]) ? tolower (atomsym_p[i]) :
          atomsym_p[i];

      buf[++length] = ' ';
      length++;
      index = PL1_Index (GSym2IdMap, sizeof (GSym2IdMap), (char *)buf, length);

      if (index != (U16_t)INFINITY)
        index = (index / 3) + 1;
      else
        index = 0;
      break;
    }

  return index;
}

/****************************************************************************
*
*  Function Name:                 Atomid_IsHalogen
*
*    This function checks to see if the given atomid is a halogen atom.
*
*  Used to be:
*
*    halogen:
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
Boolean_t Atomid_IsHalogen
  (
  U16_t         atomid                     /* Atomic number */
  )
{
  if (atomid == FLUORINE || atomid == CHLORINE || atomid == BROMINE ||
      atomid == IODINE || atomid == ASTATINE || atomid == GENERIC_HALOGEN)
    return TRUE;

  return FALSE;
}
/* End of Atomid_IsHalogen */

/****************************************************************************
*
*  Function Name:                 Atomid_IsChalcogen
*
*    This function checks to see if the given atomid is a halogen atom.
*
*  Used to be:
*
*    halogen:
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
Boolean_t Atomid_IsChalcogen
  (
  U16_t         atomid                     /* Atomic number */
  )
{
  if (atomid == OXYGEN || atomid == SULFUR || atomid == SELENIUM ||
      atomid == TELLURIUM || atomid == POLONIUM || atomid == GENERIC_CHALCOGEN)
    return TRUE;

  return FALSE;
}
/* End of Atomid_IsChalcogen */
/* End of ATOMSYM.C */
