/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     POSTIN.C
*
*    Converts the machine-friendly postfix notation used in posttransform test
*    logic into the more user-friendly infix notation for display and editing.
*
*  Creation Date:
*
*    16-Feb-2000
*
*  Authors:
*
*    Gerald A. Miller
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
#include "utl.h"

#define MAX_STACK 256
#define max(a,b) \
  ((a) > (b) ? (a) : (b))

/* external entry declarations */
Boolean_t Scanner (char *, char *, int *, char *);
void push (char *, char, int);
void pop (char *, char *, int *);
void parenthesize (char *, int *);
void Sep_NOPs (char *);
int VERIFY (char *, const char *);

static char *Stack_Elem[MAX_STACK];
static char Stack_Oper[MAX_STACK];
static int Stack_Lev[MAX_STACK], Stack_Size = 0;

Boolean_t PostIn (char *expression)
{
/* Author: DJB   Date: 27 November 1982 */
/* Converts the given postfix boolean expression into a
   parenthesized infix expression.  If the error flag is set to
   true a problem was encountered during the conversion and
   the partial result on the stack top is returned. */
/* 14 DEC 1982 - DJB - Modified the parenthesization policy */
/* 4 FEB 1983 - DJB - Added procedure SEP_NOPS to treate
   the postfix string before conversion is performed.
   The procedure places a blank after any NOP operands
   so the general scanner can isolate them as single
   tokens. */
/* 2 JUNE 1983 - GAM - Freed ARG1, ARG2, and RESULT before return. */
/* 6 JULY 1983 - DJB & GAM - Set LOGIC_EXPR to '' when operating
   on a terminal NOP in SEP_NOPS, to prevent infinite looping. */
/* 16 March 1987 - GAM & DJB: Tried to simplify POSTIN by removing unnecessary
   association of expressions.  Added XOR/NEQ (#) operator, after verifying
   its associativity. */
/* 17 March 1987 - GAM & DJB: Added EQ (=) operator, after verifying its
   associativity. */
/* 16 February 1988 - GAM: Added LEV parameters to PUSH, POP, and
   PARENTHESIZE, in an attempt to improve parenthesization performance. */

  const char *ALPHA_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

/* local variable declarations */
  int         Exp_Len,
              Num,
              Lev1        = 0,
              Lev2        = 0,
              Ident_Cnt   = 0,
              Bin_Op_Cnt  = 0,
              i;
  Boolean_t   Error,
              EOI,
              More_Spe_Chars;
  char       *Arg1,
             *Arg2,
             *Result,
              Token[81],
              Tok_Type[4],
              Two_Digits[3],
              dummy       = 0,
              Spe_Chr     = 0,
              Oper1       = 0,
              Oper2       = 0;

/*
DCL (ARG1 BASED(A1PTR), ARG2 BASED(A2PTR), RESULT BASED(RESPTR),
   STACK_ELEM BASED(SELPTR)) CHAR(EXP_LEN) VAR;
DCL STACK_OPER CHAR(1) BASED(SOPPTR);
DCL STACK_LEV BIN FIXED(15) BASED(SLVPTR);
DCL (A1PTR,A2PTR,RESPTR,SELPTR,SOPPTR,SLVPTR) PTR;
DCL (FALSE, EOI, MORE_SPE_CHARS) BIT(1) INIT('0'B);
DCL TRUE BIT(1) INIT('1'B);
DCL TOKEN CHAR(80) VAR;
DCL ALPHA_CHARS CHAR(26) INIT('ABCDEFGHIJKLMNOPQRSTUVWXYZ');
DCL TWO_DIGITS PIC '99';
DCL (IDENT_CNT, BIN_OP_CNT, I, NUM, EXP_LEN, DEBUG, LEV1, LEV2)
   FIXED BIN(15) INIT(0);
DCL TOK_TYPE CHAR(3) VAR;
DCL (SPE_CHR, DUMMY, OPER1, OPER2) CHAR(1) INIT('');
DCL (INDEX, LENGTH, SUBSTR, VERIFY) BUILTIN;
DCL XLIFO ENTRY;
DCL ALLOCT ENTRY(CHAR(*) VAR,CHAR(*) VAR,PTR,BIN FIXED(15),CHAR(*) VAR,
   BIN FIXED(15));
DCL ALLOCN ENTRY(CHAR(*) VAR,CHAR(*) VAR) RETURNS(BIN FIXED(15));
DCL FREE ENTRY(CHAR(*) VAR,CHAR(*) VAR,PTR);
*/

/* initialize scratch strings and variables */
  Sep_NOPs (expression); /* separate NOPs with blanks */
/*
DEBUG = TRACE.OPTIONS(90);
IF DEBUG>0 THEN PUT SKIP EDIT('POSTIN postfix: ', EXPRESSION) (A, A);
*/
  Error = EOI = More_Spe_Chars = FALSE;
  Exp_Len = 2 * strlen (expression); /* double length scratch strs */
  Arg1 = (char *) malloc (Exp_Len + 1);
  Arg2 = (char *) malloc (Exp_Len + 1);
  Result = (char *) malloc (Exp_Len + 1);

  while ((!EOI || More_Spe_Chars) && !Error)
  {
    if (!More_Spe_Chars) /* no special chars */ EOI = Scanner (expression, Token, &Num, Tok_Type);
    if (!strcmp (Tok_Type, "WRD")) /* stack operand */
    {
      push (Token, ' ', 0);
      Ident_Cnt++;
    } /* if */
    else if (!strcmp (Tok_Type, "INT")) /* handle numeric */
    {
      pop (Arg1, &dummy, &Lev1);
      i = VERIFY (Arg1, ALPHA_CHARS); /* check for alpha ident */
      if (i == 0 && dummy == ' ') /* associate numeric with identifier */
      {
         sprintf (Two_Digits, "%02d", Num);
         strcat (Arg1, Two_Digits);
      } /* if */
      else /* numeric not associated properly */
      {
         printf ("POSTIN: numeric not associated\n");
         Error = TRUE;
      } /* else */
      push (Arg1, ' ', Lev1);
    } /* else if */
    else if (!strcmp (Tok_Type, "SPE"))
    {
      Spe_Chr = Token[0];
      strcpy (Token, Token + 1);
      /* handle special char groups in single chars */
      More_Spe_Chars = Token[0] != 0;

      if (Spe_Chr == '~') /* logical negation */
      {
         pop (Arg1, &Oper1, &Lev1);
         if (Oper1 != ' ') parenthesize(Arg1, &Lev1);
         Result[0] = Spe_Chr;
         strcpy (Result + 1, Arg1);
/* Try out parenthesizing NOT expression to remove any chance of ambiguity among non-logicians */
if (!EOI || More_Spe_Chars)
  parenthesize (Result, &Lev1);
         push (Result, ' ', Lev1);
      } /* if */
      else if (Spe_Chr == '&') /* logical and */
      {
        pop (Arg2, &Oper2, &Lev2); /* get args in proper order */
        pop (Arg1, &Oper1, &Lev1);
        if (Oper1 == '|' || Oper1 == '=' || Oper1 == '#')
          parenthesize (Arg1, &Lev1);
        if (Oper2 == '|' || Oper2 == '=' || Oper2 == '#')
          parenthesize (Arg2, &Lev2);
        strcpy (Result, Arg1);
        strcat (Result, "   ");
        Result[strlen (Result) - 2] = Spe_Chr;
        strcat (Result, Arg2);
        push (Result, '&', max (Lev1, Lev2));
        Bin_Op_Cnt++;
      } /* else if */
      else if (Spe_Chr == '|') /* logical inclusive or */
      {
        pop (Arg2, &Oper2, &Lev2); /* get args in proper order */
        pop (Arg1, &Oper1, &Lev1);
        if (Oper1 == '&' || Oper1 == '=' || Oper1 == '#')
          parenthesize (Arg1, &Lev1);
        if (Oper2 == '&' || Oper2 == '=' || Oper2 == '#')
          parenthesize (Arg2, &Lev2);
        strcpy (Result, Arg1);
        strcat (Result, "   ");
        Result[strlen (Result) - 2] = Spe_Chr;
        strcat (Result, Arg2);
        push (Result, '|', max (Lev1, Lev2));
        Bin_Op_Cnt++;
      } /* else if */
      else if (Spe_Chr == '=') /* logical equivalence operator */
      {
        pop (Arg2, &Oper2, &Lev2); /* get args in proper order */
        pop (Arg1, &Oper1, &Lev1);
        if (Oper1 == '&' || Oper1 == '|' || Oper1 == '#')
          parenthesize (Arg1, &Lev1);
        if (Oper2 == '&' || Oper2 == '|' || Oper2 == '#')
          parenthesize (Arg2, &Lev2);
        strcpy (Result, Arg1);
        strcat (Result, "   ");
        Result[strlen (Result) - 2] = Spe_Chr;
        strcat (Result, Arg2);
        push (Result, '=', max (Lev1, Lev2));
        Bin_Op_Cnt++;
      } /* else if */
      else if (Spe_Chr == '#') /* logical exclusive or (non-equivalence) operator */
      {
        pop (Arg2, &Oper2, &Lev2); /* get args in proper order */
        pop (Arg1, &Oper1, &Lev1);
        if (Oper1 == '&' || Oper1 == '|' || Oper1 == '=')
          parenthesize (Arg1, &Lev1);
        if (Oper2 == '&' || Oper2 == '|' || Oper2 == '=')
          parenthesize (Arg2, &Lev2);
        strcpy (Result, Arg1);
        strcat (Result, "   ");
        Result[strlen (Result) - 2] = Spe_Chr;
        strcat (Result, Arg2);
        push (Result, '#', max (Lev1, Lev2));
        Bin_Op_Cnt++;
      } /* else if */
      else
      {
        printf ("POSTIN: unknown operator (%c) encountered\n", Spe_Chr);
        Error = TRUE;
      } /* else */
    } /* else if */
    else
    {
      printf ("POSTIN: bad token (%s) encountered\n", Token);
      Error = TRUE;
    } /* else */
  } /* while */

  pop (expression, &dummy, &Lev2); /* get infix expression */
  pop (Arg1, &dummy, &Lev1); /* check to see if stack empty */
  if (Arg1[0] != 0)
  {
    printf ("POSTIN: stack dump\n%s\n(level =%2d)\n", expression, Lev2);
    while (Arg1[0] != 0) /* dump stack for debug */
    {
      printf ("%s\n(level =%2d)\n", Arg1, Lev1);
      pop (Arg1, &dummy, &Lev1);
    } /* while */
    printf ("POSTIN: conversion not completed\n");
    Error = TRUE;
  } /* if */
  if (Ident_Cnt != Bin_Op_Cnt + 1)
  {
    printf ("POSTIN: bad identifier to binary oper ratio\n");
    Error = TRUE;
  } /* if */
/*
  if (!Error && DEBUG>0) THEN
   PUT SKIP EDIT('POSTIN infix: ', EXPRESSION) (A, A);
*/
  free (Arg1);
  free (Arg2);
  free (Result);

  return (!Error);
}

/* local procedures */
void push (char *ELEM, char OPER, int LEV)
{
  if (Stack_Size == MAX_STACK)
  {
    printf ("Stack Overflow in PostIn (push)\n");
    return;
  }
  Stack_Elem[Stack_Size] = (char *) malloc (strlen (ELEM) + 1);
  strcpy (Stack_Elem[Stack_Size], ELEM);
  Stack_Oper[Stack_Size] = OPER;
  Stack_Lev[Stack_Size] = LEV;
  Stack_Size++;
} /* Push */

void pop (char *ELEM, char *OPER, int *LEV)
{
  *OPER = '?';
  if (Stack_Size > 0)
  {
    Stack_Size--;
    strcpy (ELEM, Stack_Elem[Stack_Size]);
    free (Stack_Elem[Stack_Size]);
    *OPER = Stack_Oper[Stack_Size];
    *LEV = Stack_Lev[Stack_Size];
  } /* if */
  else ELEM[0] = 0; /* stack empty */
} /* Pop */

void parenthesize (char *EXPR, int *LEV)
{
  /* Encloses string in parentheses, braces, or brackets.  It
     alternates between the three character types by level. */
  const char *OP = "([{", *CL = ")]}";
  int SELECTOR, i;

/* OP = SUBSTR(EXPR, 1, 1); /* check first char for paren type */
/* CL = SUBSTR(EXPR, LENGTH(EXPR), 1); /* check last char for paren type */
/* IF OP = '(' | CL = ')' THEN DO; /* second level brackets */
/*    OP = '['; CL = ']'; */
/* END; /* if */
/* ELSE IF OP = '[' | CL = ']' THEN DO; /* third level braces */
/*    OP = '{'; CL = '}'; */
/* END; /* else if */
/* ELSE DO; /* round parens complete the alternation */
/*    OP = '('; CL = ')'; */
/* END; /* else */
/* above tentatively removed to test superiority of the following: */

  SELECTOR = *LEV % 3;
  strncat (EXPR, CL + SELECTOR, 1);
  for (i = strlen (EXPR); i >= 0; i--)
    EXPR[i+1] = EXPR[i];
  EXPR[0] = OP[SELECTOR];
  ++*LEV;
} /* Parenthesize */

void Sep_NOPs (char *Logic_Expr)
{
  /* Inserts blanks after any NOP operand found in the
  logic string. */

  /* variable declarations */
  char string[81] = {0}, *jp;
  int j;

  jp = strstr (Logic_Expr, "NOP");
  j = jp == NULL ? 0 : ((int) (jp - Logic_Expr)) + 1;
  if (j != 0) /* separate NOPs with blanks */
  { 
    while(j != 0)
    {
      strncat (string, Logic_Expr, j + 2);
      strcat (string, " ");
      strcpy (Logic_Expr, Logic_Expr + j + 2);
      jp = strstr (Logic_Expr, "NOP");
      j = jp == NULL ? 0 : ((int) (jp - Logic_Expr)) + 1;
    }
    strcat (string, Logic_Expr); /* get tail */
    strcpy (Logic_Expr, string);
  }
}

int VERIFY (char *str, const char *vstr)
{
  int pos, vpos;
  Boolean_t found;

  for (pos = 0; pos < strlen (str); pos++)
  {
    for (vpos = 0, found = FALSE; vpos < strlen (vstr) && !found; vpos++)
      found = str[pos] == vstr[vpos];
    if (!found) return (pos + 1);
  }
  return (0);
}
