#ifndef _H_PERSIST_
#define _H_PERSIST_
/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     PERSIST.H
*
*    This module defines the mnemonics, datatypes, and functions used in the
*    management of a persistent knowledge base.
*
*  Creation Date:
*
*    18-Feb-2000
*
*  Authors:
*
*    Gerald A. Miller
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
*
******************************************************************************/


#ifndef _H_SYNCHEM_
#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#include "synchem.h"
#endif

#ifndef _H_RCB_
#include "rcb.h"
#endif

#define PER_NONE 0xFFFFFFFF
#define PER_DELETED 0xFEFEFEFE
#define MX_RXN_INFO 1000000
#define PER_TIME_LEGACY 0
#define PER_TIME_ANY 0x7FFFFFFF
#define PER_STD 0
#define PER_TEMP 1
#define PER_PREV 0
#define PER_NEW 1

typedef struct
  {
  Date_t date_last_mod; /* PER_TIME_LEGACY if already exists */
  long time_last_mod; /* PER_TIME_LEGACY if already exists */
  long replacement_rec; /* PER_NONE if none */
  long temp_replacement_rec; /* PER_NONE if none */
  long replaces_which_rec; /* PER_NONE if none */
  long replaces_which_temp_rec; /* PER_NONE if none */
  }
Persist_RxnInfo;
#define PER_RXNINFO_SIZE sizeof (Persist_RxnInfo)

Boolean_t Persist_Inx_OK (char *, char *);
Date_t Persist_Today ();
long Persist_Now ();
Boolean_t Persist_Legacy_Rxn (long, Date_t, long, Boolean_t *);
long Persist_ModTime (int, long);
long Persist_Current_Rec (int, long, Date_t, long, Boolean_t *);
long Persist_Orig_Rec (int, long, Boolean_t *);
void Persist_Open (char *, char *, char *, Boolean_t);
void Persist_Close();
void Persist_LinkRecs (FILE *, int, int, int, Date_t, char *, Boolean_t);
void Persist_Init_RxnInx ();
void Persist_Dump_RxnInx (FILE *);
void Persist_Update_Rxn (long, long, Boolean_t, Boolean_t, char *);
void Persist_Add_Rxn (long, Boolean_t, char *);
void Persist_Delete_Rxn (long, Boolean_t);
void Persist_Recalc_Time ();

#ifdef _H_PERSIST_GLOBAL_
static Date_t today;
static long time_now;
static Boolean_t temp_exists;
static FILE *rxn_inx = NULL, *temp_rxn_inx = NULL;
static Persist_RxnInfo rxn_info[2][MX_RXN_INFO];
static int recs_in_inx, recs_in_temp_inx;
#endif

#endif
