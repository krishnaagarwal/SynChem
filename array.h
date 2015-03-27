#ifndef _H_ARRAY_
#define _H_ARRAY_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1997, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     ARRAY.H
*
*    This module is the header for the variable size array abstraction.
*    The idea is to allow runtime determined array sizing rather than compile-
*    time since that is very useful, but C doesn't support it so there needs
*    to be a set of routines and a data-structure.
*
*    Routines are found in ARRAY.C.
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
* 02-Mar-97  Krebsbach  Changed array parameters from U16_t to U32_t.
*
******************************************************************************/
#include <stdlib.h>

/*** Literal Values ***/

#define BITSIZE      3
#define BYTESIZE     1
#define WORDSIZE     2
#define LONGSIZE     4
#define DOUBLESIZE   5

#ifndef BIGADDR
#define BITUNITSIZE  4
#define ADDRSIZE     LONGSIZE
#define Array_1dAddr_Get Array_1d32_Get
#define Array_1dAddr_Put Array_1d32_Put
#define Array_2dAddr_Get Array_2d32_Get
#define Array_2dAddr_Put Array_2d32_Put
#define Array_3dAddr_Get Array_3d32_Get
#define Array_3dAddr_Put Array_3d32_Put
#else
#define BITUNITSIZE  8
#define ADDRSIZE     DOUBLESIZE
#define Array_1dAddr_Get Array_1d64_Get
#define Array_1dAddr_Put Array_1d64_Put
#define Array_2dAddr_Get Array_2d64_Get
#define Array_2dAddr_Put Array_2d64_Put
#define Array_3dAddr_Get Array_3d64_Get
#define Array_3dAddr_Put Array_3d64_Put
#endif

/*** Data Structures ***/

/* There needs to be a data-structure that can handle up to 3D arrays without
   difficulty.  It seems best to make the data-structure as independent of
   the sort of array as possible and then to have many specialized routines
   or macros to manipulate it sensibly.
*/

typedef struct s_array
  {
  Address        storage;                   /* Storage for array */
  U32_t          columns;                   /* Number of columns */
  U32_t          rows;                      /* Number of rows for 2d */
  U32_t          planes;                    /* Number of planes for 3d */
  U32_t          size;                      /* Bytes per unit of storage */
  } Array_t;
#define ARRAYSIZE sizeof (Array_t)

/** Field Access Macros for Array_t **/

/* Macro Prototypes
   U32_t Array_Columns_Get   (Array_t *);
   void  Array_Columns_Put   (Array_t *, U32_t);
   U32_t Array_Planes_Get    (Array_t *);
   void  Array_Planes_Put    (Array_t *, U32_t);
   U32_t Array_Rows_Get      (Array_t *);
   void  Array_Rows_Put      (Array_t *, U32_t);
   U32_t Array_Size_Get      (Array_t *);
   void  Array_Size_Put      (Array_t *, U32_t);
   Address Array_Storage_Get (Array_t *);
   void  Array_Storage_Put   (Array_t *, Address);
*/

#ifndef ARRAY_DEBUG
#define Array_Columns_Get(array)\
  (array)->columns

#define Array_Columns_Put(array, value)\
  (array)->columns = (U32_t)(value)

#define Array_Planes_Get(array)\
  (array)->planes

#define Array_Planes_Put(array, value)\
  (array)->planes = (U32_t)(value)

#define Array_Rows_Get(array)\
  (array)->rows

#define Array_Rows_Put(array, value)\
  (array)->rows = (U32_t)(value)

#define Array_Size_Get(array)\
  (array)->size

#define Array_Size_Put(array, value)\
  (array)->size = (U32_t)(value)

#define Array_Storage_Get(array)\
  (array)->storage

#define Array_Storage_Put(array, value)\
  (array)->storage = (Address)(value)
#else
#define Array_Columns_Get(array)\
  ((array) == NULL ? HALT : (array)->columns)

#define Array_Columns_Put(array, value)\
  { if ((array) == NULL) HALT; else\
  (array)->columns = (U32_t)(value); }

#define Array_Planes_Get(array)\
  ((array) == NULL ? HALT : (array)->planes)

#define Array_Planes_Put(array, value)\
  { if ((array) == NULL) HALT; else\
  (array)->planes = (U32_t)(value); }

#define Array_Rows_Get(array)\
  ((array) == NULL ? HALT : (array)->rows)

#define Array_Rows_Put(array, value)\
  { if ((array) == NULL) ? HALT; else\
  (array)->rows = (U32_t)(value); }

#define Array_Size_Get(array)\
  ((array) == NULL ? HALT : (array)->size)

#define Array_Size_Put(array, value)\
  { if ((array) == NULL) HALT; else\
  (array)->size = (U32_t)(value); }

#define Array_Storage_Get(array)\
  ((array) == NULL ? HALTP : (array)->storage)

#define Array_Storage_Put(array, value)\
  { if ((array) == NULL) HALT; else\
  (array)->storage = (Address)(value); }
#endif

/** End of Field Access Macros for Array_t **/

/*** Macros ***/

/* Arranged by array dimension, element size, get/put
*/

#ifndef ARRAY_DEBUG
#define Array_1d8_Get(array, column)\
  ((U8_t *)(array)->storage)[column]

#define Array_1d8_Put(array, column, value)\
  ((U8_t *)(array)->storage)[column] = (U8_t)(value)

#define Array_1d8_Addr(array, column)\
  &((U8_t *)(array)->storage)[column]

#define Array_1d16_Get(array, column)\
  ((U16_t *)(array)->storage)[column]

#define Array_1d16_Put(array, column, value)\
  ((U16_t *)(array)->storage)[column] = (U16_t)(value)

#define Array_1d16_Addr(array, column)\
  &((U16_t *)(array)->storage)[column]

#define Array_1d32_Get(array, column)\
  ((U32_t *)(array)->storage)[column]

#define Array_1d32_Put(array, column, value)\
  ((U32_t *)(array)->storage)[column] = (U32_t)(value)

#define Array_1d32_Addr(array, column)\
  &((U32_t *)(array)->storage)[column]

#define Array_1d64_Get(array, column)\
  ((U64_t *)(array)->storage)[column]

#define Array_1d64_Put(array, column, value)\
  ((U64_t *)(array)->storage)[column] = (U64_t)(value)

#define Array_2d8_Get(array, row, column)\
  ((U8_t *)(array)->storage)[(row) * (array)->columns + (column)]

#define Array_2d8_Put(array, row, column, value)\
  ((U8_t *)(array)->storage)[(row) * (array)->columns + (column)] = (U8_t)(value)

#define Array_2d8_Addr(array, row, column)\
  &((U8_t *)(array)->storage)[(row) * (array)->columns + (column)]

#define Array_2d16_Get(array, row, column)\
  ((U16_t *)(array)->storage)[(row) * (array)->columns + (column)]

#define Array_2d16_Put(array, row, column, value)\
  ((U16_t *)(array)->storage)[(row) * (array)->columns + (column)] = (U16_t)(value)

#define Array_2d16_Addr(array, row, column)\
  &((U16_t *)(array)->storage)[(row) * (array)->columns + (column)]

#define Array_2d32_Get(array, row, column)\
  ((U32_t *)(array)->storage)[(row) * (array)->columns + (column)]

#define Array_2d32_Put(array, row, column, value)\
  ((U32_t *)(array)->storage)[(row) * (array)->columns + (column)] = (U32_t)(value)

#define Array_2d32_Addr(array, row, column)\
  &((U32_t *)(array)->storage)[(row) * (array)->columns + (column)]

#define Array_2d64_Get(array, row, column)\
  ((U64_t *)(array)->storage)[(row) * (array)->columns + (column)]

#define Array_2d64_Put(array, row, column, value)\
  ((U64_t *)(array)->storage)[(row) * (array)->columns + (column)] = (U64_t)(value)

#define Array_3d8_Get(array, plane, row, column)\
  ((U8_t *)(array)->storage)[(plane) * (array)->rows * (array)->columns\
    + (row) * (array)->columns + (column)]

#define Array_3d8_Put(array, plane, row, column, value)\
  ((U8_t *)(array)->storage)[(plane) * (array)->rows * (array)->columns\
    + (row) * (array)->columns + (column)] = (u8_t)(value)

#define Array_3d16_Get(array, plane, row, column)\
  ((U16_t *)(array)->storage)[(plane) * (array)->rows * (array)->columns\
    + (row) * (array)->columns + (column)]

#define Array_3d16_Put(array, plane, row, column, value)\
  ((U16_t *)(array)->storage)[(plane) * (array)->rows * (array)->columns\
    + (row) * (array)->columns + (column)] = (U16_t)(value)

#define Array_3d32_Get(array, plane, row, column)\
  ((U32_t *)(array)->storage)[(plane) * (array)->rows * (array)->columns\
    + (row) * (array)->columns + (column)]

#define Array_3d32_Put(array, plane, row, column, value)\
  ((U32_t *)(array)->storage)[(plane) * (array)->rows * (array)->columns\
    + (row) * (array)->columns + (column)] = (U32_t)(value)

#define Array_3d64_Get(array, plane, row, column)\
  ((U64_t *)(array)->storage)[(plane) * (array)->rows * (array)->columns\
    + (row) * (array)->columns + (column)]

#define Array_3d64_Put(array, plane, row, column, value)\
  ((U64_t *)(array)->storage)[(plane) * (array)->rows * (array)->columns\
    + (row) * (array)->columns + (column)] = (U64_t)(value)
#else
#define Array_1d8_Get(array, column)\
  ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
   (array)->size == BYTESIZE ? ((U8_t *)(array)->storage)[column] : HALT)

#define Array_1d8_Put(array, column, value)\
  { if ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
  (array)->size == BYTESIZE)\
    ((U8_t *)(array)->storage)[column] = (U8_t)(value);\
  else\
    (array)->planes = HALT; }

#define Array_1d8_Addr(array, column)\
  ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
   (array)->size == BYTESIZE ? &((U8_t *)(array)->storage)[column] : HALTP)

#define Array_1d16_Get(array, column)\
  ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
   (array)->size == WORDSIZE ? ((U16_t *)(array)->storage)[column] : HALT)

#define Array_1d16_Put(array, column, value)\
  { if ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
  (array)->size == WORDSIZE)\
    ((U16_t *)(array)->storage)[column] = (U16_t)(value);\
  else\
    (array)->planes = HALT; }

#define Array_1d16_Addr(array, column)\
  ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) && (array)->size ==\
  WORDSIZE ? &((U16_t *)(array)->storage)[column] : (U16_t *)HALTP)

#define Array_1d32_Get(array, column)\
  ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) && (array)->size ==\
  LONGSIZE ? ((U32_t *)(array)->storage)[column] : HALT)

#define Array_1d32_Put(array, column, value)\
  { if ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
  (array)->size == LONGSIZE)\
    ((U32_t *)(array)->storage)[column] = (U32_t)(value);\
  else\
    (array)->planes = HALT; }

#define Array_1d64_Get(array, column)\
  ((Address)(array) > GBAddr && (array)->columns > (column) &&\
  (array)->size == DOUBLESIZE ? ((U64_t *)(array)->storage)[column] : HALT)

#define Array_1d64_Put(array, column, value)\
  { if ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
  (array)->size == DOUBLESIZE)\
    ((U64_t *)(array)->storage)[column] = (U64_t)(value);\
  else\
    (array)->planes = HALT; }

#define Array_2d8_Get(array, row, column)\
  ((Address)(array) > GBAddr && (array)->columns > (column) &&\
    (array)->rows > (row) &&\
    (array)->size == BYTESIZE ? ((U8_t *)(array)->storage)[(row)\
    * (array)->columns + (column)] : HALT)

#define Array_2d8_Put(array, row, column, value)\
  { if ((Address)(array) > GBAddr && (array)->columns > (column) &&\
  (array)->rows > (row) && (array)->size == BYTESIZE)\
    ((U8_t *)(array)->storage)[(row) * (array)->columns + (column)] =\
    (U8_t)(value);\
  else\
    (array)->planes = HALT; }

#define Array_2d8_Addr(array, row, column)\
  ((Address)(array) > GBAddr && (array)->columns > (column) &&\
    (array)->rows > (row) &&\
    (array)->size == BYTESIZE ? &((U8_t *)(array)->storage)[(row)\
     * (array)->columns + (column)] : HALT)

#define Array_2d16_Get(array, row, column)\
  ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
   (array)->rows >\
   (U32_t)(row) && (array)->size == WORDSIZE ? ((U16_t *)(array)->storage)\
  [(row) * (array)->columns + (column)] : HALT)

#define Array_2d16_Put(array, row, column, value)\
  { if ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
  (array)->rows > (U32_t)(row) && (array)->size == WORDSIZE)\
    ((U16_t *)(array)->storage)[(row) * (array)->columns + (column)] =\
    (U16_t)(value);\
  else\
    (array)->planes = HALT; }

#define Array_2d16_Addr(array, row, column)\
  ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
    (array)->rows > (U32_t)(row) &&\
    (array)->size == WORDSIZE ? &((U16_t *)(array)->storage)[(row)\
     * (array)->columns + (column)] : (U16_t *)HALTP)

#define Array_2d32_Get(array, row, column)\
  ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
    (array)->rows > (U32_t)(row) &&\
    (array)->size == LONGSIZE ? ((U32_t *)(array)->storage)[(row)\
    * (array)->columns + (column)] : HALT)

#define Array_2d32_Put(array, row, column, value)\
  { if ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
  (array)->rows > (U32_t)(row) && (array)->size == LONGSIZE)\
    ((U32_t *)(array)->storage)[(row) * (array)->columns + (column)] =\
    (U32_t)(value);\
  else\
    (array)->planes = HALT; }

#define Array_2d32_Addr(array, row, column)\
  ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
    (array)->rows > (U32_t)(row) &&\
    (array)->size == LONGSIZE ?  &((U32_t *)(array)->storage)[(row)\
     * (array)->columns + (column)] : HALTP)

#define Array_2d64_Get(array, row, column)\
  ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
    (array)->rows > (U32_t)(row) &&\
    (array)->size == DOUBLESIZE ? ((U64_t *)(array)->storage)[(row)\
    * (array)->columns + (column)] : HALT)

#define Array_2d64_Put(array, row, column, value)\
  { if ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
  (array)->rows > (U32_t)(row) && (array)->size == DOUBLESIZE)\
    ((U64_t *)(array)->storage)[(row) * (array)->columns + (column)] =\
    (U64_t)(value);\
  else\
    (array)->planes = HALT; }

#define Array_3d8_Get(array, plane, row, column)\
  ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
    (array)->rows > (U32_t)(row) &&\
    (array)->planes > (U32_t)(plane) && (array)->size == BYTESIZE\
    ? ((U8_t *)(array)->storage)[(plane) * (array)->rows * (array)->columns\
    + (row) * (array)->columns + (column)] : HALT)

#define Array_3d8_Put(array, plane, row, column, value)\
  { if ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
    (array)->rows > (U32_t)(row) && (array)->planes > (U32_t)(plane) && \
    (array)->size == BYTESIZE)\
    ((U8_t *)(array)->storage)[(plane) * (array)->rows * (array)->columns\
      + (row) * (array)->columns + (column)] = (U8_t)(value);\
  else\
    (array)->planes = HALT; }

#define Array_3d16_Get(array, plane, row, column)\
  ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
    (array)->rows > (U32_t)(row) &&\
    (array)->planes > (U32_t)(plane) && (array)->size == WORDSIZE\
    ? ((U16_t *)(array)->storage)[(plane) * (array)->rows * (array)->columns\
    + (row) * (array)->columns + (column)] : HALT)

#define Array_3d16_Put(array, plane, row, column, value)\
  { if ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
    (array)->rows > (U32_t)(row) && (array)->planes > (U32_t)(plane) && \
    (array)->size == WORDSIZE)\
    ((U16_t *)(array)->storage)[(plane) * (array)->rows * (array)->columns\
      + (row) * (array)->columns + (column)] = (U16_t)(value);\
  else\
    (array)->planes = HALT; }

#define Array_3d32_Get(array, plane, row, column)\
  ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
    (array)->rows > (U32_t)(row) &&\
    (array)->planes > (U32_t)(plane) && (array)->size == LONGSIZE\
    ? ((U32_t *)(array)->storage)[(plane) * (array)->rows * (array)->columns\
    + (row) * (array)->columns + (column)] : HALT)

#define Array_3d32_Put(array, plane, row, column, value)\
  { if ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
    (array)->rows > (U32_t)(row) && (array)->planes > (U32_t)(plane) && \
    (array)->size == LONGSIZE)\
    ((U32_t *)(array)->storage)[(plane) * (array)->rows * (array)->columns\
      + (row) * (array)->columns + (column)] = (U32_t)(value);\
  else\
    (array)->planes = HALT; }

#define Array_3d64_Get(array, plane, row, column)\
  ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
    (array)->rows > (U32_t)(row) &&\
    (array)->planes > (U32_t)(plane) && (array)->size == DOUBLESIZE\
    ? ((U64_t *)(array)->storage)[(plane) * (array)->rows * (array)->columns\
    + (row) * (array)->columns + (column)] : HALT)

#define Array_3d64_Put(array, plane, row, column, value)\
  { if ((Address)(array) > GBAddr && (array)->columns > (U32_t)(column) &&\
    (array)->rows > (U32_t)(row) && (array)->planes > (U32_t)(plane) && \
    (array)->size == DOUBLESIZE)\
    ((U64_t *)(array)->storage)[(plane) * (array)->rows * (array)->columns\
      + (row) * (array)->columns + (column)] = (U64_t)(value);\
  else\
    (array)->planes = HALT; }

#endif

/*** Routine Prototypes ***/

Boolean_t Array_1d1_Get      (Array_t *, U32_t);
void      Array_1d1_Put      (Array_t *, U32_t, Boolean_t);
void      Array_1d_Create    (Array_t *, U32_t, U32_t);
Boolean_t Array_2d1_Get      (Array_t *, U32_t, U32_t);
void      Array_2d1_Put      (Array_t *, U32_t, U32_t, Boolean_t);
void      Array_2d_Create    (Array_t *, U32_t, U32_t, U32_t);
Boolean_t Array_3d1_Get      (Array_t *, U32_t, U32_t, U32_t);
void      Array_3d1_Put      (Array_t *, U32_t, U32_t, U32_t, Boolean_t);
void      Array_3d_Create    (Array_t *, U32_t, U32_t, U32_t, U32_t);
void      Array_Copy         (Array_t *, Array_t *);
void      Array_CopyContents (Array_t *, Array_t *);
void      Array_Destroy      (Array_t *);
void      Array_Dump         (Array_t *, FileDsc_t *);
void      Array_Set          (Array_t *, U32_t);

/*** Global Variables ***/

/* End of Array.H */
#endif
