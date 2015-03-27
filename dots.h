/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     DOTS.H
*
*    This module declares the dots functions.
*
*  Creation Date:
*
*    11-Aug-2000
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

#ifndef _H_DOTS_
#define _H_DOTS_
void dots (Xtr_t *, Xtr_t *, U32_t *, char *);
void find_all_monovalent_neighbors (Xtr_t *, int, int, int *, int *);
void dot_constants (Xtr_t *, int *, int, char *);
void dot_pairs (Xtr_t *, Xtr_t *, int, int *, int, char *);
Boolean_t same_kind_of_atom (Xtr_t *, int *, int, int, int);
void dot_remainder (Xtr_t *, Xtr_t *, int, int, int *, int, char *);
Boolean_t dotted_constant (Xtr_t *, int *, int, int);
#endif
