/******************************************************************************
*
*  Copyright (C) 1981, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     SCANNER.C
*
*    This procedure takes character by character input and
*    constructs tokens.  The tokens are assembled according to
*    the rules embodied by the scan table.  The maximum token length
*    is determined by the length of the Word parameter.  If the
*    procedure is invoked on an empty input buffer a message is
*    issued and EndOfInput is set to true (1).
*
*  Parameters:
*     INPUT - Character string used to store the users
*          responses.  It is consumed by this procedure.
*
*     WORD - Character string used to return an identifier.
*          (the default is empty '')
*
*     NUMBER - Integer used to return the value of a number when
*          encountered in the input buffer.
*          (the default value is zero)
*
*     TOKEN_TYPE - Character string used to identifiy the type of
*          the token being returned as word (WRD), integer (INT),
*          special (SPE), literal (LIT), or illegal (ILL).
*          (the default is ILL)
*
*     END_OF_INPUT - Bit one set to true (1) if the end of the
*          input buffer is encountered.
*          (default value is false)
*
*
*  Creation Date:
*
*    Dec. 1981
*
*  Authors:
*
*    Donald J. Berndt
*    Gerald A. Miller (conversion to C)
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred       xxx
*
******************************************************************************/

#include <string.h>

#include "synchem.h"

void Next_Char (char *, char *);

Boolean_t Scanner (char *INPUT, char *WORD, int *NUMBER, char *TOKEN_TYPE)
{
/* Copyright 1981 - SYNCHEM Research Group - SUNY Stony Brook */

/***********************************************************

Author: D. J. Berndt     Date: December 1981

**********************************************************/
  static int  NEXT_STATE[7][6] = {{-1, 5, 2, 3, 1, 4},
                                 {-1, -1, 2, -1, 6, -1},
                                 {-1, -1, -1, 3, 6, -1},
                                 {-1, -1, -1, -1, 6, 4},
                                 {-1, 6, 7, 7, 7, 7},
                                 {-1, -1, -1, -1, 6, -1},
                                 {-1, 6, 7, 7, 7, 7}};
  const char *LETTERS          = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  const char *DIGITS           = "0123456789";
  const char *DELIMITERS       = " ,";

  /* variable declarations */
  Boolean_t   End_of_Input;
  char        Chr[4]           = {0};
  int         State            = 1,
              Chr_Type         = 1,
              i;

/*
DCL DEBUG FIXED BIN(15) INIT(0);
DCL (INDEX, SUBSTR, LENGTH) BUILTIN;
*/

  /* initialize parameters */
  WORD[0] = *NUMBER = 0;
  strcpy (TOKEN_TYPE, "ILL");
  End_of_Input = FALSE;

  /* loop for token construction */
  while (State > 0)
  {
    Next_Char (INPUT, Chr);
    /* case on character type */
    if (!strcmp (Chr, "EOI")) /* EOI */
    {
      Chr_Type = 1;
      End_of_Input = TRUE;
    } /* if */
    else if (Chr[0] == '\"' || Chr[0] == '\'') Chr_Type = 2; /* lit */
    else if (strstr (LETTERS, Chr) != NULL) Chr_Type = 3;
    else if (strstr (DIGITS, Chr) != NULL) Chr_Type = 4;
    else if (strstr (DELIMITERS, Chr) != NULL) Chr_Type = 5;
    else Chr_Type = 6; /* special */

    /* find next state */
    State = NEXT_STATE[State - 1][Chr_Type - 1];
    /* case on states for actions */
    switch (State)
    {
    case 2:
      strcpy (TOKEN_TYPE, "WRD");
      strcat (WORD, Chr);
      break;
    case 3:
      /* convert number */
      strcpy (TOKEN_TYPE, "INT");
      *NUMBER = *NUMBER * 10 + Chr[0] - '0';
      break;
    case 4:
      strcpy (TOKEN_TYPE, "SPE");
      strcat (WORD, Chr);
      break;
    case 7:
      strcpy (TOKEN_TYPE, "LIT");
      strcat (WORD, Chr);
      break;
    case -1:
      for (i = strlen (INPUT); i >= 0; i--) INPUT[i + strlen (Chr)] = INPUT[i];
      strncpy (INPUT, Chr, strlen (Chr));
      break;
    } /* switch */

/* debug
    IF DEBUG > 2 THEN DO;
      PUT SKIP EDIT('chr ', CHR, '  chr type ', CHR_TYPE,
         '  state ', STATE) (A, A, A, F(2), A, F(2));
    END; /* if */

  } /* while */

/* debug
IF DEBUG > 1 THEN DO;
   PUT SKIP EDIT(INPUT, WORD, NUMBER, TOKEN_TYPE)
      (A, SKIP, A, SKIP, F(10), SKIP, A);
   IF END_OF_INPUT THEN PUT SKIP EDIT('EOI encountered') (A);
END; /* if */

  return (End_of_Input);
}

/* local procedures */
void Next_Char (char *INPUT, char *CHR)
{
  /* initialize CHR and begin search of INPUT */
  strcpy (CHR, "EOI");
  if (strlen (INPUT) > 0)
  {
    CHR[0] = INPUT[0];
    CHR[1] = 0;
    strcpy (INPUT, INPUT + 1);
  } /* if */
}
