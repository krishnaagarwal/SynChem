#ifndef _H_TIMER_
#define _H_TIMER_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     TIMER.H
*
*    This module contains the abstraction for the Timer_t data structure.
*
*    Routines can be found in TIMER.C
*
*    Used to be in UTL.H
*
*  Creation Date:
*
*    23-Sep-1996
*
*  Authors:
*
*    Tito Autrey (rewritten based on others PLI code)
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 23-Sep-96  Krebsbach  Moved Timer_t to timer.h.
*
******************************************************************************/

/* Need to get definition for time_t */

#ifdef _SUN4_OS_
#include <sys/stdtypes.h>
#include <sys/time.h>
extern int   gettimeofday (struct timeval *, struct timezone *tzp);
typedef struct timeval Time_t;               /* Synchem time handling */
#define TIME_ZERO GZeroTime

#else
#ifdef _SOLARIS_OS_
#include <sys/time.h>
typedef struct timeval Time_t;               /* Synchem time handling */
#define TIME_ZERO GZeroTime

#else
#ifdef _IRIX64_OS_
#include <sys/time.h>
typedef struct timeval Time_t;               /* Synchem time handling */
#define TIME_ZERO GZeroTime

#else
#ifdef _IRIX62_OS_
#include <sys/time.h>
typedef struct timeval Time_t;               /* Synchem time handling */
#define TIME_ZERO GZeroTime

#else
#ifdef _WIN32
#include <sys/time.h>
typedef struct timeval Time_t;               /* Synchem time handling */
#define TIME_ZERO GZeroTime

#else
#ifdef _REDHAT_
#include <sys/time.h>
typedef struct timeval Time_t;               /* Synchem time handling */
#define TIME_ZERO GZeroTime

#else
#ifdef _CYGWIN_
#include <sys/time.h>
typedef struct timeval Time_t;               /* Synchem time handling */
#define TIME_ZERO GZeroTime

#else

struct timeb {
        time_t time;
        unsigned short millitm;
        short timezone;
        short dstflag;
        };


typedef float Time_t;
#define TIME_ZERO (float)0.0

#endif
#endif
#endif
#endif
#endif
#endif
#endif

#ifdef _WIN32
#define USECFAC 1000000
#else
#ifdef _WINNT_OS_
#define USECFAC 1000
#else
#define USECFAC 1000000
#endif
#endif

#ifndef _H_SYNIO_
#include "synio.h"
#endif

/*** Literal Values ***/

#define SET             9
#define ADD             10


/* Timer structure for accounting / monitoring purposes. */

typedef struct s_times
  {
  Time_t         commun;                     /* Communication time */
  Time_t         cycle_time;                 /* Overall cycle time */
  Time_t         init_time;                  /* Initialization time */
  Time_t         expand;                     /* Expansion */
  Time_t         select_next;                /* Select next node to expand */
  Time_t         tuple_wait;                 /* Tuple wait time */
  Time_t         update_pst;                 /* Update the PST */
  Time_t         wait;                       /* Wait time */
  } Timer_t;
#define TIMERSIZE sizeof (Timer_t)

/** Field Access Macros for Timer_t **/

/* Macro Prototypes

   void   Timer_Init          (Timer_t *);
   Time_t Timer_Commun_Get    (Timer_t *);
   void   Timer_Commun_Put    (Timer_t *, Time_t, U8_t);
   Time_t Timer_Cycle_Get     (Timer_t *);
   void   Timer_Cycle_Put     (Timer_t *, Time_t, U8_t);
   Time_t Timer_Expand_Get    (Timer_t *);
   void   Timer_Expand_Put    (Timer_t *, Time_t, U8_t);
   Time_t Timer_Init_Get      (Timer_t *);
   void   Timer_Init_Put      (Timer_t *, Time_t, U8_t);
   Time_t Timer_Select_Get    (Timer_t *);
   void   Timer_Select_Put    (Timer_t *, Time_t, U8_t);
   Time_t Timer_TupleWait_Get (Timer_t *);
   void   Timer_TupleWait_Put (Timer_t *, Time_t, U8_t);
   Time_t Timer_UpdatePST_Get (Timer_t *);
   void   Timer_UpdatePST_Put (Timer_t *, Time_t, U8_t);
   Time_t Timer_Wait_Get      (Timer_t *);
   void   Timer_Wait_Put      (Timer_t *, Time_t, U8_t);
*/

/* ??? No DEBUG version of macros (timer_t) */
#define Timer_Init(tim_p)\
 {(tim_p)->init_time.tv_sec = 0.0;\
  (tim_p)->init_time.tv_usec = 0.0;\
  (tim_p)->commun.tv_sec = 0.0;\
  (tim_p)->commun.tv_usec = 0.0;\
  (tim_p)->cycle_time.tv_sec = 0.0;\
  (tim_p)->cycle_time.tv_usec = 0.0;\
  (tim_p)->expand.tv_sec = 0.0;\
  (tim_p)->expand.tv_usec = 0.0;\
  (tim_p)->tuple_wait.tv_sec = 0.0;\
  (tim_p)->tuple_wait.tv_usec = 0.0;\
  (tim_p)->update_pst.tv_sec = 0.0;\
  (tim_p)->update_pst.tv_usec = 0.0;\
  (tim_p)->wait.tv_sec = 0.0;\
  (tim_p)->wait.tv_usec = 0.0;\
  (tim_p)->select_next.tv_usec = 0.0;\
  (tim_p)->select_next.tv_sec = 0.0;}
 
#define Timer_Commun_Get(tim_p)\
  (tim_p)->commun

#define Timer_Commun_Put(tim_p, value, op)\
  if (op == SET)\
    (tim_p)->commun = (value);\
  else\
    {Time_t tmp1; tmp1 = (value); (tim_p)->commun.tv_usec += tmp1.tv_usec;\
    (tim_p)->commun.tv_sec += tmp1.tv_sec;\
  if ((tim_p)->commun.tv_usec > USECFAC)\
    (tim_p)->commun.tv_sec++, (tim_p)->commun.tv_usec -= USECFAC;\
  else if ((tim_p)->commun.tv_usec < 0)\
    (tim_p)->commun.tv_sec--, (tim_p)->commun.tv_usec += USECFAC; }

#define Timer_Cycle_Get(tim_p)\
  (tim_p)->cycle_time

#define Timer_Cycle_Put(tim_p, value, op)\
  if (op == SET)\
    (tim_p)->cycle_time = (value);\
  else\
    {Time_t tmp1; tmp1 = (value); (tim_p)->cycle_time.tv_usec += tmp1.tv_usec;\
    (tim_p)->cycle_time.tv_sec += tmp1.tv_sec;\
  if ((tim_p)->cycle_time.tv_usec > USECFAC)\
    (tim_p)->cycle_time.tv_sec++, (tim_p)->cycle_time.tv_usec -= USECFAC;\
  else if ((tim_p)->cycle_time.tv_usec < 0)\
    (tim_p)->cycle_time.tv_sec--, (tim_p)->cycle_time.tv_usec += USECFAC; }

#define Timer_Expand_Get(tim_p)\
  (tim_p)->expand

#define Timer_Expand_Put(tim_p, value, op)\
  if (op == SET)\
    (tim_p)->expand = (value);\
  else\
    {Time_t tmp1; tmp1 = (value); (tim_p)->expand.tv_usec += tmp1.tv_usec;\
    (tim_p)->expand.tv_sec += tmp1.tv_sec;\
  if ((tim_p)->expand.tv_usec > USECFAC)\
    (tim_p)->expand.tv_sec++, (tim_p)->expand.tv_usec -= USECFAC;\
  else if ((tim_p)->expand.tv_usec < 0)\
    (tim_p)->expand.tv_sec--, (tim_p)->expand.tv_usec += USECFAC; }

#define Timer_Init_Get(tim_p)\
  (tim_p)->init_time

#define Timer_Init_Put(tim_p, value, op)\
  if (op == SET)\
    (tim_p)->init_time = (value);\
  else\
    {Time_t tmp1; tmp1 = (value); (tim_p)->init_time.tv_usec += tmp1.tv_usec;\
    (tim_p)->init_time.tv_sec += tmp1.tv_sec;\
  if ((tim_p)->init_time.tv_usec > USECFAC)\
    (tim_p)->init_time.tv_sec++, (tim_p)->init_time.tv_usec -= USECFAC;\
  else if ((tim_p)->init_time.tv_usec < 0)\
    (tim_p)->init_time.tv_sec--, (tim_p)->init_time.tv_usec += USECFAC; }

#define Timer_Select_Get(tim_p)\
  (tim_p)->select_next

#define Timer_Select_Put(tim_p, value, op)\
  if (op == SET)\
    (tim_p)->select_next = (value);\
  else\
    {Time_t tmp1; tmp1 = (value); (tim_p)->select_next.tv_usec += tmp1.tv_usec;\
    (tim_p)->select_next.tv_sec += tmp1.tv_sec;\
  if ((tim_p)->select_next.tv_usec > USECFAC)\
    (tim_p)->select_next.tv_sec++, (tim_p)->select_next.tv_usec -= USECFAC;\
  else if ((tim_p)->select_next.tv_usec < 0)\
    (tim_p)->select_next.tv_sec--, (tim_p)->select_next.tv_usec += USECFAC; }

#define Timer_TupleWait_Get(tim_p)\
  (tim_p)->tuple_wait

#define Timer_TupleWait_Put(tim_p, value, op)\
  if (op == SET)\
    (tim_p)->tuple_wait = (value);\
  else\
    {Time_t tmp1; tmp1 = (value); (tim_p)->tuple_wait.tv_usec += tmp1.tv_usec;\
    (tim_p)->tuple_wait.tv_sec += tmp1.tv_sec;\
  if ((tim_p)->tuple_wait.tv_usec > USECFAC)\
    (tim_p)->tuple_wait.tv_sec++, (tim_p)->tuple_wait.tv_usec -= USECFAC;\
  else if ((tim_p)->tuple_wait.tv_usec < 0)\
    (tim_p)->tuple_wait.tv_sec--, (tim_p)->tuple_wait.tv_usec += USECFAC; }

#define Timer_UpdatePST_Get(tim_p)\
  (tim_p)->update_pst

#define Timer_UpdatePST_Put(tim_p, value, op)\
  if (op == SET)\
    (tim_p)->update_pst = (value);\
  else\
    {Time_t tmp1; tmp1 = (value); (tim_p)->update_pst.tv_usec += tmp1.tv_usec;\
    (tim_p)->update_pst.tv_sec += tmp1.tv_sec;\
  if ((tim_p)->update_pst.tv_usec > USECFAC)\
    (tim_p)->update_pst.tv_sec++, (tim_p)->update_pst.tv_usec -= USECFAC;\
  else if ((tim_p)->update_pst.tv_usec < 0)\
    (tim_p)->update_pst.tv_sec--, (tim_p)->update_pst.tv_usec += USECFAC; }

#define Timer_Wait_Get(tim_p)\
  (tim_p)->wait

#define Timer_Wait_Put(tim_p, value, op)\
  if (op == SET)\
    (tim_p)->wait = (value);\
  else\
    {Time_t tmp1; tmp1 = (value); (tim_p)->wait.tv_usec += tmp1.tv_usec;\
    (tim_p)->wait.tv_sec += tmp1.tv_sec;\
  if ((tim_p)->wait.tv_usec > USECFAC)\
    (tim_p)->wait.tv_sec++, (tim_p)->wait.tv_usec -= USECFAC;\
  else if ((tim_p)->wait.tv_usec < 0)\
    (tim_p)->wait.tv_sec--, (tim_p)->wait.tv_usec += USECFAC; }

/** End of Field Access Macros for Timer_t **/


/*** Macros ***/

/* Macro prototypes
  void    Time_Add        (Time_t *, Time_t);
  float   Time_Format     (Time_t);
  void    Time_Sub        (Time_t *, Time_t);
*/

#define Time_Add(sum_p, add)\
  { Time_t tmp1; tmp1 = (add); (sum_p)->tv_usec += tmp1.tv_usec;\
    (sum_p)->tv_sec += tmp1.tv_sec;\
  if ((sum_p)->tv_usec > USECFAC)\
    (sum_p)->tv_sec++, (sum_p)->tv_usec -= USECFAC;\
  else if ((sum_p)->tv_usec < 0)\
    (sum_p)->tv_sec--, (sum_p)->tv_usec += USECFAC; }

#define Time_Format(tim)\
  (float)tim.tv_sec + ((float)tim.tv_usec) / (float)USECFAC

#define Time_Sub(diff_p, sub)\
  { Time_t tmp1; tmp1 = (sub); (diff_p)->tv_usec -= tmp1.tv_usec;\
    (diff_p)->tv_sec -= tmp1.tv_sec;\
  if ((diff_p)->tv_usec > USECFAC)\
    (diff_p)->tv_sec++, (diff_p)->tv_usec -= USECFAC;\
  else if ((diff_p)->tv_usec < 0)\
    (diff_p)->tv_sec--, (diff_p)->tv_usec += USECFAC; }



/*** Routine prototypes ***/

Time_t        Syn_Time_Get     (Boolean_t);
void          Timers_Dump       (Timer_t *, FileDsc_t *);

/*** Global Variables ***/

#ifdef TIMER_GLOBALS
  struct timeval         GZeroTime = {0, 0};
#else
  extern  struct timeval GZeroTime;
#endif


/* End of TIMER.H */
#endif
