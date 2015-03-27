/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     ARRAY.C
*
*    This module is the data-abstraction for runtime variable size arrays.
*    C doesn't support them as part of the language like PL/1 does, so we 
*    have to fake them.  1, 2 and 3-dimensional arrays are supported.  Along
*    with 1, 8, 16, 32 (64?)-bit datatypes.  Much of the access functions
*    are implemented as macros for efficiency reasons.
*
*  Routines:
*
*    Array_Copy
*    Array_CopyContents
*    Array_1d1_Get
*    Array_1d1_Put
*    Array_1d_Create
*    Array_2d1_Get
*    Array_2d1_Put
*    Array_2d_Create
*    Array_3d1_Get
*    Array_3d1_Put
*    Array_3d_Create
*    Array_Destroy
*    Array_Dump
*    Array_Set
*
*  Creation Date:
*
*    01-Jan-1991
*
*  Authors:
*
*    Tito Autrey
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 02-Mar-97  Krebsbach  Changed array parameters from U16_t to U32_t
*                       and added memset (0) to all created arrays.
*                       (20% of execution time initializing arrays).
* 10-Apr-95  Cheung	In Array_Destroy, deallocate memory only when the
*			size of array != 0, and set memory to 0 after
*			deallocation.
*
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifdef ARRAY_DEBUG
#undef ARRAY_DEBUG
#ifndef _H_ARRAY_
#include "array.h"
#endif
#define ARRAY_DEBUG 1
#else
#ifndef _H_ARRAY_
#include "array.h"
#endif
#endif

/* Static variables */

static const char *SArrayType[5] =
  { "Invalid", "Byte", "Word", "Bit", "Longword" };

/****************************************************************************
*
*  Function Name:                 Array_Copy
*
*      This function copies an Array_t.  The Array_t to copy to must
*      have valid storage.
*
*  Used to be:
*
*    N/A
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
*    Allocates memory
*
******************************************************************************/
void Array_Copy
  (
  Array_t       *iarray_p,                   /* Array to copy */
  Array_t       *oarray_p                    /* Array to copy to */
  )
{
  U32_t          size;                       /* Number of bytes of storage */

  DEBUG (R_ARRAY, DB_ARRAYCOPY, TL_PARAMS, (outbuf,
    "Entering Array_Copy, input  = %p, output = %p", iarray_p, oarray_p));

  if (Array_Rows_Get (iarray_p) == 0)
    {
    Array_1d_Create (oarray_p, Array_Columns_Get (iarray_p),
      Array_Size_Get (iarray_p));
    size = (Array_Size_Get (iarray_p) != BITSIZE) ?
      Array_Columns_Get (iarray_p) * Array_Size_Get (iarray_p) :
      ((Array_Columns_Get (iarray_p) + (BITUNITSIZE * 8)) / (BITUNITSIZE * 8))
      * BITUNITSIZE;
    }
  else
    if (Array_Planes_Get (iarray_p) == 0)
      {
      Array_2d_Create (oarray_p, Array_Rows_Get (iarray_p),
        Array_Columns_Get (iarray_p), Array_Size_Get (iarray_p));
      size = (Array_Size_Get (iarray_p) != BITSIZE) ?
        Array_Columns_Get (iarray_p) * Array_Rows_Get (iarray_p) *
        Array_Size_Get (iarray_p) : ((Array_Columns_Get (iarray_p) *
        Array_Rows_Get (iarray_p) + (BITUNITSIZE * 8)) / (BITUNITSIZE * 8))
        * BITUNITSIZE;
      }
    else
      {
      Array_3d_Create (oarray_p, Array_Planes_Get (iarray_p), Array_Rows_Get (
        iarray_p), Array_Columns_Get (iarray_p), Array_Size_Get (iarray_p));
      size = (Array_Size_Get (iarray_p) != BITSIZE) ? Array_Columns_Get (
        iarray_p) * Array_Rows_Get (iarray_p) * Array_Planes_Get (iarray_p) *
        Array_Size_Get (iarray_p) : (((Array_Columns_Get (iarray_p) *
        Array_Rows_Get (iarray_p) * Array_Planes_Get (iarray_p)) +
        (BITUNITSIZE * 8)) / (BITUNITSIZE * 8)) * BITUNITSIZE;
      }

  (void) memcpy (Array_Storage_Get (oarray_p), Array_Storage_Get (iarray_p),
    size);

  DEBUG (R_ARRAY, DB_ARRAYCOPY, TL_PARAMS, (outbuf,
    "Leaving Array_Copy, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Array_CopyContents
*
*      This function copies an Array_t's contents, but assumes the structures
*      have been already created.
*
*  Used to be:
*
*    N/A
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
void Array_CopyContents
  (
  Array_t       *iarray_p,                   /* Array to copy */
  Array_t       *oarray_p                    /* Array to copy to */
  )
{
  U32_t          size;                       /* Number of bytes of storage */

  DEBUG (R_ARRAY, DB_ARRAYCOPY, TL_PARAMS, (outbuf,
    "Entering Array_CopyContents, input = %p, output = %p", iarray_p,
    oarray_p));

  if (Array_Columns_Get (iarray_p) != Array_Columns_Get (oarray_p) ||
      Array_Rows_Get (iarray_p) != Array_Rows_Get (oarray_p) ||
      Array_Planes_Get (iarray_p) != Array_Planes_Get (oarray_p) ||
      Array_Size_Get (iarray_p) != Array_Size_Get (oarray_p))
    {
    IO_Put_Debug (R_ARRAY,
      "Arrays of different sizes/types in Array_CopyContents");
    return;
    }
  else
    size = MAX (Array_Rows_Get (iarray_p), 1) * MAX (Array_Columns_Get (
      iarray_p), 1) * MAX (Array_Planes_Get (iarray_p), 1);

  if (Array_Size_Get (iarray_p) == BITSIZE)
    size = ((size + (BITUNITSIZE * 8)) / (BITUNITSIZE * 8)) * BITUNITSIZE;
  else
    if (Array_Size_Get (iarray_p) == DOUBLESIZE)
      size = size * 8;
    else
      size = size * Array_Size_Get (iarray_p);

  (void) memcpy (Array_Storage_Get (oarray_p), Array_Storage_Get (iarray_p),
    size);

  DEBUG (R_ARRAY, DB_ARRAYCOPY, TL_PARAMS, (outbuf,
    "Leaving Array_CopyContents, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:        Array_1d_Create, Array_2d_Create, Array_3d_Create
*
*    These routines allocate the memory for the array storage and setup
*    the array descriptor.  Note that the descriptor does not contain an
*    explicit field for the number of dimesions, that should be obvious from
*    the context in which the array is used.
*
*    Bit arrays have their storage allocation calculated to round off to
*    the nearest 32-bit quantity (should be 64-bit on 64-bit architectures).
*
*  Used to be:
*
*    N/A
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
*    Allocates memory
*    May call  IO_Exit_Error
*
******************************************************************************/
void Array_1d_Create
  (
  Array_t       *array_p,                   /* Array to manipulate */
  U32_t          columns,                   /* Number of columns */
  U32_t          size                       /* Element type flag */
  )
{
  U32_t          storage_size;              /* Temporary */

  DEBUG (R_ARRAY, DB_ARRAY1DCREATE, TL_PARAMS, (outbuf,
    "Entering Array_1d_Create, array = %p, columns = %lu, size = %lu", array_p,
    columns, size));

  Array_Columns_Put (array_p, columns);
  Array_Rows_Put (array_p, 0);
  Array_Planes_Put (array_p, 0);
  Array_Size_Put (array_p, size);

  /* Calculate the # bytes needed.  Bit arrays get rounded # longwords which
     comprise 4 bytes (8 in supported architectures)
  */

  if (size == BITSIZE)
    storage_size = ((columns + (BITUNITSIZE * 8)) / (BITUNITSIZE * 8)) *
      BITUNITSIZE;
  else
    if (size == DOUBLESIZE)
      storage_size = columns * 8;
    else
      storage_size = size * columns;

#ifdef _MIND_MEM_
  mind_malloc ("Array_Storage_Get(array_p)", "array", &Array_Storage_Get (array_p), storage_size);
#else
  Mem_Alloc (Address, Array_Storage_Get (array_p), storage_size, GLOBAL);
#endif

  DEBUG (R_ARRAY, DB_ARRAY1DCREATE, TL_MEMORY, (outbuf,
    "Allocated storage for 1D array in Array_1d_Create at %p",
    Array_Storage_Get (array_p)));

  if (Array_Storage_Get (array_p) == NULL)
    IO_Exit_Error (R_ARRAY, X_LIBCALL,
      "No memory for array creation in Array_1d_Create");

  DEBUG_DO (DB_ARRAY1DCREATE, TL_MEMORY,
    {
    memset (Array_Storage_Get (array_p), 0, storage_size);
    });

  memset (Array_Storage_Get (array_p), 0, storage_size);

  DEBUG (R_ARRAY, DB_ARRAY1DCREATE, TL_PARAMS, (outbuf,
    "Leaving Array_1d_Create, status = <void>"));

  return;
}

void Array_2d_Create
  (
  Array_t       *array_p,                   /* Array to manipulate */
  U32_t          rows,                      /* Number of rows */
  U32_t          columns,                   /* Number of columns */
  U32_t          size                       /* Element size */
  )
{
  U32_t          storage_size;              /* Temporary */

  DEBUG (R_ARRAY, DB_ARRAY3DCREATE, TL_PARAMS, (outbuf,
    "Entering Array_2d_Create, array = %p, rows = %lu, columns = %lu,"
    " size = %lu", array_p, rows, columns, size));

  Array_Columns_Put (array_p, columns);
  Array_Rows_Put (array_p, rows);
  Array_Planes_Put (array_p, 0);
  Array_Size_Put (array_p, size);

  /* Calculate the # bytes needed.  Bit arrays get rounded # longwords which
     comprise 4 bytes (8 in supported architectures)
  */

  if (size == BITSIZE)
    storage_size = (((columns * rows) + (BITUNITSIZE * 8)) / (BITUNITSIZE * 8))
      * BITUNITSIZE;
  else
    if (size == DOUBLESIZE)
      storage_size = columns * rows * 8;
    else
      storage_size = size * columns * rows;

#ifdef _MIND_MEM_
  mind_malloc ("Array_Storage_Get(array_p)", "array", &Array_Storage_Get (array_p), storage_size);
#else
  Mem_Alloc (Address, Array_Storage_Get (array_p), storage_size, GLOBAL);
#endif

  DEBUG (R_ARRAY, DB_ARRAY2DCREATE, TL_MEMORY, (outbuf,
    "Allocated storage for 2D array in Array_2d_Create at %p",
    Array_Storage_Get (array_p)));

  if (Array_Storage_Get (array_p) == NULL)
    IO_Exit_Error (R_ARRAY, X_LIBCALL,
      "No memory for array creation in Array_2d_Create");

  DEBUG_DO (DB_ARRAY2DCREATE, TL_MEMORY,
    {
    memset (Array_Storage_Get (array_p), 0, storage_size);
    });

  memset (Array_Storage_Get (array_p), 0, storage_size);

  DEBUG (R_ARRAY, DB_ARRAY2DCREATE, TL_PARAMS, (outbuf,
    "Leaving Array_2d_Create, status = <void>"));

  return;
}

void Array_3d_Create
  (
  Array_t       *array_p,                   /* Array to manipulate */
  U32_t          planes,                    /* Number of planes */
  U32_t          rows,                      /* Number of rows */
  U32_t          columns,                   /* Number of columns */
  U32_t          size                       /* Element size */
  )
{
  U32_t          storage_size;              /* Temporary */

  DEBUG (R_ARRAY, DB_ARRAY3DCREATE, TL_PARAMS, (outbuf,
    "Entering Array_3d_Create, array = %p, rows = %lu, columns = %lu,"
    " planes = %lu, size = %lu", array_p, rows, columns, planes, size));

  Array_Columns_Put (array_p, columns);
  Array_Rows_Put (array_p, rows);
  Array_Planes_Put (array_p, planes);
  Array_Size_Put (array_p, size);

  /* Calculate the # bytes needed.  Bit arrays get rounded # longwords which
     comprise 4 bytes (8 in supported architectures)
  */

  if (size == BITSIZE)
    storage_size = (((columns * rows * planes) + (BITUNITSIZE * 8)) /
    (BITUNITSIZE * 8)) * BITUNITSIZE;
  else
    if (size == DOUBLESIZE)
      storage_size = columns * rows * planes * 8;
    else
      storage_size = size * columns * rows * planes;

#ifdef _MIND_MEM_
  mind_malloc ("Array_Storage_Get(array_p)", "array", &Array_Storage_Get (array_p), storage_size);
#else
  Mem_Alloc (Address, Array_Storage_Get (array_p), storage_size, GLOBAL);
#endif

  DEBUG (R_ARRAY, DB_ARRAY3DCREATE, TL_MEMORY, (outbuf,
    "Allocating storage for 3D array in Array_3d_Create at %p",
    Array_Storage_Get (array_p)));

  if (Array_Storage_Get (array_p) == NULL)
    IO_Exit_Error (R_ARRAY, X_LIBCALL,
      "No memory for array creation in Array_3d_Create");

  DEBUG_DO (DB_ARRAY3DCREATE, TL_MEMORY,
    {
    memset (Array_Storage_Get (array_p), 0, storage_size);
    });

  memset (Array_Storage_Get (array_p), 0, storage_size);

  DEBUG (R_ARRAY, DB_ARRAY3DCREATE, TL_PARAMS, (outbuf,
    "Leaving Array_3d_Create, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:      Array_1d1_Get, Array_1d1_Put, Array_2d1_Get,
*                      Array_2d1_Put, Array_3d1_Get, Array_3d1_Put
*
*    These functions and routines address the access problems for the
*    bit-arrays.  The only real trick is figuring out which byte to access
*    and generating the mask for fetching or storing the value.
*
*  Used to be:
*
*    N/A
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
*    *Get returns TRUE if bit is 1, FALSE otherwise
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t Array_1d1_Get
  (
  Array_t       *array_p,                   /* Array to manipulate */
  U32_t          column                     /* Which column */
  )
{
  U32_t          offset;                    /* Byte offset into vector */
  U32_t          mask;                      /* Mask for bit to access */

#ifdef ARRAY_DEBUG
  if (!(Array_Columns_Get (array_p) > column || Array_Size_Get (array_p)
      != BITSIZE))
    mask = HALT;
#endif

  mask = 1 << (column % (BITUNITSIZE * 8));
  offset = column / (BITUNITSIZE * 8);

  if ((Array_1d32_Get (array_p, offset) & mask) != 0)
    return TRUE;
  else
    return FALSE;
}

void Array_1d1_Put
  (
  Array_t       *array_p,                   /* Array to manipulate */
  U32_t          column,                    /* Which column */
  Boolean_t      value                      /* Value to store */
  )
{
  U32_t          offset;                    /* Byte offset into vector */
  U32_t          mask;                      /* Mask for bit to access */
  U32_t          temp;                      /* For value resetting */

#ifdef ARRAY_DEBUG
  if (!(Array_Columns_Get (array_p) > column || Array_Size_Get (array_p)
      != BITSIZE))
    mask = HALT;
#endif

  mask = 1 << (column % (BITUNITSIZE * 8));
  offset = column / (BITUNITSIZE * 8);

  temp = Array_1d32_Get (array_p, offset);
  temp = (value) ? temp | mask : temp & ~mask;
  Array_1d32_Put (array_p, offset, temp);

  return;
}

Boolean_t Array_2d1_Get
  (
  Array_t       *array_p,                   /* Array to manipulate */
  U32_t          row,                       /* Which row */
  U32_t          column                     /* Which column */
  )
{
  U32_t          offset;                    /* Byte offset into vector */
  U32_t          mask;                      /* Mask for bit to access */

#ifdef ARRAY_DEBUG
  if (!(Array_Columns_Get (array_p) > column || Array_Size_Get (array_p)
      != BITSIZE || Array_Rows_Get (array_p) > row))
    mask = HALT;
#endif

  mask = 1 << (((row * Array_Columns_Get (array_p)) + column) %
    (BITUNITSIZE * 8));
  offset = ((row * Array_Columns_Get (array_p)) + column) / (BITUNITSIZE * 8);

  if ((Array_1d32_Get (array_p, offset) & mask) != 0)
    return TRUE;
  else
    return FALSE;
}

void Array_2d1_Put
  (
  Array_t       *array_p,                   /* Array to manipulate */
  U32_t          row,                       /* Which row */
  U32_t          column,                    /* Which column */
  Boolean_t      value                      /* Value to store */
  )
{
  U32_t          offset;                    /* Byte offset into vector */
  U32_t          mask;                      /* Mask for bit to access */
  U32_t          temp;                      /* For value resetting */

#ifdef ARRAY_DEBUG
  if (!(Array_Columns_Get (array_p) > column || Array_Size_Get (array_p)
      != BITSIZE || Array_Rows_Get (array_p) > row))
    mask = HALT;
#endif

  mask = 1 << (((row * Array_Columns_Get (array_p)) + column) %
    (BITUNITSIZE * 8));
  offset = ((row * Array_Columns_Get (array_p)) + column) / (BITUNITSIZE * 8);

  temp = Array_1d32_Get (array_p, offset);
  temp = (value) ? temp | mask : temp & ~mask;
  Array_1d32_Put (array_p, offset, temp);

  return;
}

Boolean_t Array_3d1_Get
  (
  Array_t       *array_p,                   /* Array to manipulate */
  U32_t          plane,                     /* Which plane */
  U32_t          row,                       /* Which row */
  U32_t          column                     /* Which column */
  )
{
  U32_t          offset;                    /* Byte offset into vector */
  U32_t          mask;                      /* Mask for bit to access */

#ifdef ARRAY_DEBUG
  if (!(Array_Columns_Get (array_p) > column || Array_Size_Get (array_p)
      != BITSIZE || Array_Rows_Get (array_p) > row || Array_Planes_Get (
      array_p) > plane))
    mask = HALT;
#endif

  mask = 1 << (((plane * Array_Rows_Get (array_p) * Array_Columns_Get (
    array_p)) + (row * Array_Columns_Get (array_p)) + column) %
    (BITUNITSIZE * 8));
  offset =  ((plane * Array_Rows_Get (array_p) * Array_Columns_Get (array_p)) +
    (row * Array_Columns_Get (array_p)) + column) / (BITUNITSIZE * 8);

  if ((Array_1d32_Get (array_p, offset) & mask) != 0)
    return TRUE;
  else
    return FALSE;
}

void Array_3d1_Put
  (
  Array_t       *array_p,                   /* Array to manipulate */
  U32_t          plane,                     /* Which plane */
  U32_t          row,                       /* Which row */
  U32_t          column,                    /* Which column */
  Boolean_t      value                      /* Value to store */
  )
{
  U32_t          offset;                    /* Byte offset into vector */
  U32_t          mask;                      /* Mask for bit to access */
  U32_t          temp;                      /* For value resetting */

#ifdef ARRAY_DEBUG
  if (!(Array_Columns_Get (array_p) > column || Array_Size_Get (array_p)
      != BITSIZE || Array_Rows_Get (array_p) > row || Array_Planes_Get (
      array_p) > plane))
    mask = HALT;
#endif

  mask = 1 << (((plane * Array_Rows_Get (array_p) * Array_Columns_Get (
    array_p)) + (row * Array_Columns_Get (array_p)) + column) %
    (BITUNITSIZE * 8));
  offset =  (plane * Array_Rows_Get (array_p) * Array_Columns_Get (array_p) +
    (row * Array_Columns_Get (array_p)) + column) / (BITUNITSIZE * 8);

  temp = Array_1d32_Get (array_p, offset);
  temp = (value) ? temp | mask : temp & ~mask;
  Array_1d32_Put (array_p, offset, temp);

  return;
}

/****************************************************************************
*
*  Function Name:                 Array_Destroy
*
*    This function deallocates the storage for the array, and zeroes out 
*    the descriptor.
*
*  Used to be:
*
*    N/A
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
*    Deallocates memory
*
******************************************************************************/
void Array_Destroy
  (
  Array_t       *array_p                    /* Array to destroy */
  )
{
  U32_t          size;                      /* # bytes of storage */
  U32_t          bits;                      /* # bits in bit array */

#ifdef _DEBUG_
printf("array_p=%p; Array_Storage_Get(array_p=%p\n",array_p,Array_Storage_Get(array_p));
#endif
  DEBUG (R_ARRAY, DB_ARRAYDESTROY, TL_PARAMS, (outbuf,
    "Entering Array_Destroy, array = %p", array_p));

  if (Array_Size_Get (array_p) == BITSIZE)
    {
    bits = Array_Columns_Get (array_p) * MAX (Array_Rows_Get (array_p), 1) *
      MAX (Array_Planes_Get (array_p), 1);
    size = ((bits + (BITUNITSIZE * 8)) / (BITUNITSIZE * 8)) *
      BITUNITSIZE;
    }
  else
    {
    if (Array_Size_Get (array_p) == DOUBLESIZE)
      size = Array_Columns_Get (array_p) * 8;
    else
      size = Array_Size_Get (array_p) * Array_Columns_Get (array_p);

    size *= MAX (Array_Rows_Get (array_p), 1) *
      MAX (Array_Planes_Get (array_p), 1);
    }

  if (size != 0)    
#ifdef _MIND_MEM_
     mind_free ("Array_Storage_Get(array_p)", "array", Array_Storage_Get (array_p));
#else
     Mem_Dealloc (Array_Storage_Get (array_p), size, GLOBAL);
#endif

  if (Array_Planes_Get (array_p) != 0)
    {
    DEBUG (R_ARRAY, DB_ARRAYDESTROY, TL_MEMORY, (outbuf,
      "Deallocated 3D array storage in Array_Destroy at %p",
      Array_Storage_Get (array_p)));
    }
  else
    if (Array_Rows_Get (array_p) != 0)
      {
      DEBUG (R_ARRAY, DB_ARRAYDESTROY, TL_MEMORY, (outbuf,
        "Deallocated 2D array storage in Array_Destroy at %p",
        Array_Storage_Get (array_p)));
      }
  else
    {
    DEBUG (R_ARRAY, DB_ARRAYDESTROY, TL_MEMORY, (outbuf,
      "Deallocated 1D array storage in Array_Destroy at %p",
      Array_Storage_Get (array_p)));
    }

  DEBUG_DO (DB_ARRAYDESTROY, TL_MEMORY,
    {
    (void) memset (array_p, 0, ARRAYSIZE);
    });

  (void) memset (array_p, 0, ARRAYSIZE);

  DEBUG (R_ARRAY, DB_ARRAYDESTROY, TL_PARAMS, (outbuf,
    "Leaving Array_Destroy, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Array_Dump
*
*    This routine prints a formatted dump of a given array.
*
*  Used to be:
*
*    N/A:
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
void Array_Dump
  (
  Array_t       *array_p,                   /* Array to dump */
  FileDsc_t     *filed_p                    /* File to dump to */
  )
{
  FILE          *f;                         /* Temporary */
  U32_t          c, r, p;                   /* Indices */
  U32_t          rows, planes;              /* Limits */

  f = IO_FileHandle_Get (filed_p);
  if (array_p == NULL)
    {
    fprintf (f, "Attempted to dump NULL Array\n\n");
    return;
    }

  DEBUG_ADDR (R_ARRAY, DB_ARRAY1DCREATE, array_p);
  fprintf (f, "\tDump of Array, size = %s planes = %lu, rows = %lu,"
    " columns = %lu\n\n",
    SArrayType[Array_Size_Get (array_p)], Array_Planes_Get (array_p),
    Array_Rows_Get (array_p), Array_Columns_Get (array_p));

  rows = MAX (Array_Rows_Get (array_p), 1);
  planes = MAX (Array_Planes_Get (array_p), 1);
  for (p = 0; p < planes; p++)
    {
    if (Array_Planes_Get (array_p) != 0)
      fprintf (f, "plane : %lu\n", p);
    for (r = 0; r < rows; r++)
      {
      for (c = 0; c < Array_Columns_Get (array_p); c++)
        switch (Array_Size_Get (array_p))
          {
          case BITSIZE:

            fprintf (f, " %1d", (S8_t)Array_3d1_Get (array_p, p, r, c));
            break;

          case BYTESIZE:

            fprintf (f, " %3d", (S8_t)Array_3d8_Get (array_p, p, r, c));
            break;

          case WORDSIZE:

            fprintf (f, " %5d", (S16_t)Array_3d16_Get (array_p, p, r, c));
            break;

          case LONGSIZE:

            fprintf (f, " %5ld", (S32_t)Array_3d32_Get (array_p, p, r, c));
            break;

          default:

            fprintf (f, "Unknown array element size\n");
            break;
          }
      fprintf (f, "\n");
      }
    fprintf (f, "\n");
    }

  return;
}

/****************************************************************************
*
*  Function Name:                 Array_Set
*
*    This function initializes the storage for the array.
*
*  Used to be:
*
*    N/A
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
void Array_Set
  (
  Array_t       *array_p,                   /* Array to initialize */
  U32_t          value                      /* Initial value */
  )
{
  U32_t          i, j, k;                   /* Counters */
  U32_t          rowsize, planesize;        /* Partial size temps. */
  int            setval;

  /* When the value to be set is zero or negative one, you can
     simply use memset.
  */

  rowsize = MAX (Array_Rows_Get (array_p), 1);
  planesize = MAX (Array_Planes_Get (array_p), 1);

  if (Array_Size_Get (array_p) == BITSIZE)
    {
    i = (((Array_Columns_Get (array_p) * rowsize * planesize) +
      (BITUNITSIZE * 8)) / (BITUNITSIZE * 8)) * BITUNITSIZE;
    setval = (value) ? -1 : 0;
    (void) memset (Array_Storage_Get (array_p), setval, i);
    }
  else
    if (value == (U32_t)(-1) || value == 0)
      {
      i = Array_Size_Get (array_p) * Array_Columns_Get (array_p) * rowsize *
        planesize;
      (void) memset (Array_Storage_Get (array_p), (int) value, i);
      }
    else
      {
      for (i = 0; i < Array_Columns_Get (array_p); i++)
        for (j = 0; j < rowsize; j++)
          for (k = 0; k < planesize; k++)
            switch (Array_Size_Get (array_p))
              {
              case BYTESIZE:

                ((U8_t *)Array_Storage_Get (array_p))[(k * rowsize *
                  Array_Columns_Get (array_p)) + (j * (Array_Columns_Get (
                  array_p))) + i] = (U8_t)value;
                break;

              case WORDSIZE:

                ((U16_t *)Array_Storage_Get (array_p))[(k * rowsize *
                  Array_Columns_Get (array_p)) + (j * (Array_Columns_Get (
                  array_p))) + i] = (U16_t)value;
                break;

              case LONGSIZE:

                ((U32_t *)Array_Storage_Get (array_p))[(k * rowsize *
                  Array_Columns_Get (array_p)) + (j * (Array_Columns_Get (
                  array_p))) + i] = value;
                break;

              default:
                IO_Exit_Error (R_ARRAY, X_SYNERR,
                  "Array_Set not intended for non-scalar use");
                break;
              }
      }

  return;
}
/* End of Array_Set */
/* End of ARRAY.C */
