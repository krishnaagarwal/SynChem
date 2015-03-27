/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     INPOST.C
*
*    Converts infix notation used in posttransform test display for human-
*    readability into the more efficient postfix notation understood by the
*    POSTTEST module.
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

#include <stdio.h>

#include "synchem.h"
#include "utl.h"

Boolean_t Scanner (char *, char *, int *, char *);

void ippush (char);
char ippop ();
int in_stk_pri (char);
int in_com_pri (char);

static int stack_elem_ptr = 0;
static char stack_elem[1000];

Boolean_t inpost (char *expression)
{
  int exp_len, num, ident_cnt, i, bin_op_cnt, paren_cnt;
  char *new_expr, token[81], tok_type[4], last_char, two_digits[3], spe_chr, operator[2];
  Boolean_t eoi, more_spe_chars;

  exp_len = 2 * strlen (expression); /* expr double length scr strs */
  new_expr = (char *) malloc (exp_len + 1);
  new_expr[0] = operator[1] = '\0';
  eoi = more_spe_chars = FALSE;
  ident_cnt = bin_op_cnt = paren_cnt = 0;

  while(!eoi || more_spe_chars)
    {
    if (!more_spe_chars) /* no special chars */
      eoi = Scanner (expression, token, &num, tok_type);
    if (strcmp (tok_type, "WRD") == 0) /* concatenate operand */
      {
      strcat (new_expr, token);
      ident_cnt++;
      }
    else if (strcmp (tok_type, "INT") == 0) /* handle numeric */
      {
      i = strlen (new_expr);
      last_char = new_expr[i - 1]; /* check for ident */
      if (last_char >= 'A' && last_char <= 'Z')
        {
        sprintf (two_digits, "%02d", num);
        strcat (new_expr, two_digits);
        }
      else /* numeric not associated properly */
        {
        printf ("INPOST: numeric not associated\n");
        return (FALSE);
        }
      }
    else if (strcmp (tok_type, "SPE") == 0)
      {
      if (strlen (token) > 1)
        {
        /* handle special char groups in single chars */
        more_spe_chars = TRUE;
        spe_chr = token[0];
        strcpy (token, token + 1);
        }
      else /* last special char */
        {
        more_spe_chars = FALSE;
        spe_chr = token[0];
        token[0] = '\0';
        }
      if (spe_chr == '~' || spe_chr == '&' || spe_chr == '|' || spe_chr == '=' || spe_chr == '#')
        {
        operator[0] = ippop (); /* remove all high prty stack opers */
        while (in_stk_pri (operator[0]) >= in_com_pri (spe_chr) && operator[0] != '\0')
          {
          strcat (new_expr, operator);
          operator[0] = ippop ();
          }
        /* push back last operator if stack not empty */
        if (operator[0] != '\0') ippush (operator[0]);
        ippush (spe_chr); /* stack operator */
        if (spe_chr == '&' || spe_chr == '|' || spe_chr == '=' || spe_chr == '#')
          bin_op_cnt++; /* count opers */
        }
      else if (spe_chr == '(' || spe_chr == '[' || spe_chr == '{')
        {
        paren_cnt++;
        ippush (spe_chr);
        }
      else if (spe_chr == ')' || spe_chr == ']' || spe_chr == '}')
        {
        operator[0] = ippop (); /* pop opers until open paren */
        while (operator[0] != '(' && operator[0] != '[' && operator[0] != '{' && operator[0] !='\0')
          {
          strcat (new_expr, operator);
          operator[0] = ippop ();
          }
        /* remove open paren and decrement paren count */
        paren_cnt--;
        }
      else
        {
        printf ("INPOST: unknown operator\n");
        return (FALSE);
        }
      }
    else
      {
      printf ("INPOST: bad token encountered\n");
      return (FALSE);
      }
    }
  /* empty remainder of stack */
  operator[0] = ippop ();
  while (operator[0] != '\0')
    {
    strcat (new_expr, operator);
    operator[0] = ippop ();
    }
  strcpy (expression, new_expr); /* get postfix expression */
  free (new_expr);
  if (paren_cnt != 0)
    {
    printf ("INPOST: unmatched parentheses\n");
    return (FALSE);
    }
  if (ident_cnt != bin_op_cnt + 1)
    {
    printf ("INPOST: bad identifier to binary oper ratio\n");
    return (FALSE);
    }

  return (TRUE);
}

/* local procedures */
void ippush (char elem)
{
  stack_elem[stack_elem_ptr++] = elem;
}

char ippop ()
{
  if (stack_elem_ptr > 0) return (stack_elem[--stack_elem_ptr]);
  return ('\0'); /* stack empty */
}

int in_stk_pri (char opratr)
{
/* Returns the in stack priority of an operand. */
  int prty;

  switch (opratr)
    {
  case '&':
    return (2);
  case '|':
    return (1);
  case '~':
    return (5);
  case '=':
  case '#':
    return (3);
  default:
    return (-1); /* error value */
    }
}

int in_com_pri (char opratr)
{
/* Returns the incoming priority of an operand. */
  int prty;

  switch (opratr)
    {
  case '&':
    return (2);
  case '|':
    return (1);
  case '~':
    return (6);
  case '=':
  case '#':
    return (3);
  default:
    return (-1); /* error value */
    }
}
