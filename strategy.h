#ifndef _H_STRATEGY_
#define _H_STRATEGY_ 1
/******************************************************************************
*
*  Copyright (C) 1993-1996 Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     STRATEGY.H
*
*    This module is the abstraction of the various possible search strategies.
*    As such it contains separate types for each strategy plus a union of all
*    those types to have generically.
*
*  Routines are found in STRATEGY.C.
*
*  Creation Date:
*
*    01-Jan-1993
*
*  Authors:
*
*    Daren Krebsbach
*    Shu Cheung
*    Tito Autrey (translated from others PLI code)
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred       xxx
*
******************************************************************************/

#ifndef _H_SYNIO_
#include "synio.h"
#endif

/*** Literal Values ***/

#define EFFDIST_MINCLOSED   0

/*** Data Structures ***/

/* Note that auto-protection and strategic bonds are NYI */

/* Strategy parameter block for Effort Distribution.
   Eventually all of them should be Union'ed together.
*/

typedef struct s_strat_par
  {
  U8_t           min_closed;                 /* Min # cycles a node must be
                                                closed */
  } Strategy_Params_t;
#define STRATEGYPARAMSSIZE sizeof (Strategy_Params_t)

/** Field Access Macros for Strategy_Params_t **/

/* Macro Prototypes
   U8_t    Strategy_Params_MinClosed_Get   (Strategy_Params_t *);
   void    Strategy_Params_MinClosed_Put   (Strategy_Params_t *, U8_t);
*/

#define Strategy_Params_MinClosed_Get(stpa_p)\
  (stpa_p)->min_closed

#define Strategy_Params_MinClosed_Put(stpa_p, value)\
  (stpa_p)->min_closed = (value)

/** End of Field Access Macros for Strategy_Params_t **/

/* This data-structure captures the necessary bits for a subgoal to be
   evaluated by the Effort Distribution strategy.
*/

typedef struct s_strat_sg
  {
  Time_t         time;                       /* Time to solve this node */
  U16_t          closed_cnt;                 /* Cycles to leave node closed */
  U16_t          visits;                     /* # visits to this node */
  } Strategy_Subgoal_t;
#define STRATEGYSUBGOALSIZE sizeof (Strategy_Subgoal_t)

/** Field Access Macros for Strategy_Subgoal_t **/
/* Macro Prototypes
   U16_t   Strategy_Subg_ClosedCnt_Get (Strategy_Subgoal_t *);
   void    Strategy_Subg_ClosedCnt_Put (Strategy_Subgoal_t *, U16_t);
   Time_t  Strategy_Subg_TimeSpent_Get (Strategy_Subgoal_t *);
   void    Strategy_Subg_TimeSpent_Put (Strategy_Subgoal_t *, Time_t);
   U16_t   Strategy_Subg_Visits_Get    (Strategy_Subgoal_t *);
   void    Strategy_Subg_Visits_Put    (Strategy_Subgoal_t *, U16_t);
*/

#define Strategy_Subg_ClosedCnt_Get(stratsg_p)\
  (stratsg_p)->closed_cnt

#define Strategy_Subg_ClosedCnt_Put(stratsg_p, value)\
  (stratsg_p)->closed_cnt = (value)

#define Strategy_Subg_TimeSpent_Get(stratsg_p)\
  (stratsg_p)->time

#define Strategy_Subg_TimeSpent_Put(stratsg_p, value)\
  (stratsg_p)->time = (value)

#define Strategy_Subg_Visits_Get(stratsg_p)\
  (stratsg_p)->visits

#define Strategy_Subg_Visits_Put(stratsg_p, value)\
  (stratsg_p)->visits = (value)

/** End of Field Access Macros for Strategy_Subgoal_t **/

/*** Macros ***/

/*** Routine Prototypes ***/

void Strategy_Init (Strategy_Params_t *, U8_t);
void Strategy_Dump (Strategy_Params_t *, FileDsc_t *);

/*** Global Variables ***/

/* End of Strategy.H */
#endif
