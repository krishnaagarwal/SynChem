/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     ISAM.C
*
*    This module is a simple ISAM package.  It supports only 32-bit integer
*    keys, and they should be densely packed for optimal space use.  An
*    array of offsets and an array of sizes are kept for all records.  Since
*    the keys are expected to be densely packed, the lookup is direct, no
*    searching need be done.  The current position of the Un*x file pointer
*    is kept in order to minimize system calls (fseek).  This is on the
*    assumption that even though the file is keyed it will likely be read
*    sequentially.  Note that record size is limited to 16-bit numbers.
*
*  Routines:
*
*    Isam_Close
*    Isam_Open
*    Isam_Read
*    Isam_Read_Nobuffer
*    Isam_Write
*
*  Creation Date:
*
*    01-Jan-1992
*
*  Authors:
*
*    Tito Autrey
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred       xxx
* 26-Feb-95  Krebsbach  Modified Isam_Open to initialize crc and next_key
*                       fields.  Modified Isam_Write to copy contents of
*                       record size and offset arrays directly rather than
*                       calling Array_CopyContents, which doesn't work.
*
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_ARRAY_
#include "array.h"
#endif

#ifndef _H_ISAM_
#include "isam.h"
#endif

#ifdef _ENCRYPT_
static char keychars[] = {'\n', ' ', '-', '.', '1', '2', ':', '?',
  'A', 'Q', 'W', 'a', 'b', 'd', 'e', 'f', 'h', 'i', 'l', 'm', 'n', 'o', 'r', 's', 't', 'x', 'y', 'z'};
static char key_array[] = {69, 9, 6, 1, 10, 16, 11, 24, 1, 17, 23, 1, 19, 14, 11, 20, 24, 1, 12, 26, 1, 23, 17, 25, 1, 21, 15, 1,
  21, 20, 14, 1, 11, 20, 13, 1, 11, 1, 16, 11, 18, 15, 2, 13, 21, 27, 14, 20, 1, 21, 15, 1, 24, 16, 14, 1, 21, 24, 16, 14, 22, 7, 0,
  8, 6, 1, 4, 5, 3, 0};

FILE *Isam_fopen (char *name, char *mode)
{
  char true_name[200], *isam_pos;
  FILE *f;

  strcpy (true_name, name);
  isam_pos = strstr (true_name, ".isam");
  strcpy (isam_pos, isam_pos + 5);
  f = fopen (true_name, mode);
  return (f);
}

int Isam_fread (void *buffer, int size, int nitems, FILE *f)
{
  int offset, enclen, buflen, i;
  char decval, *bufcast;

  enclen = key_array[0];
  bufcast = (char *) buffer;
  offset = ftell (f) % enclen;
  buflen = fread (buffer, size, nitems, f);
  for (i = 0; i < buflen * size; i++)
  {
    decval = keychars[key_array[(i + offset) % enclen + 1]];
    if (bufcast[i] != 0 && bufcast[i] != decval) bufcast[i] ^= decval;
  }
  return (buflen);
}

int Isam_fwrite (void *buffer, int size, int nitems, FILE *f)
{
  int offset, enclen, buflen, i;
  char encval, *bufcast, *bufcopy;

  enclen = key_array[0];
  bufcast = (char *) buffer;
#ifdef _MIND_MEM_
  mind_malloc ("bufcopy", "isam{1}", &bufcopy, size * nitems);
#else
  bufcopy = (char *) malloc (size * nitems);
#endif
  offset = ftell (f) % enclen;
  for (i = 0; i < size * nitems; i++)
  {
    encval = keychars[key_array[(i + offset) % enclen + 1]];
    bufcopy[i] = bufcast[i];
    if (bufcopy[i] != 0 && bufcopy[i] != encval) bufcopy[i] ^= encval;
  }
  buflen = fwrite (bufcopy, size, nitems, f);
  free (bufcopy);
  return (buflen);
}
#else
#define Isam_fopen(a,b) fopen ((a), (b))
#define Isam_fread(a,b,c,d) fread ((a), (b), (c), (d))
#define Isam_fwrite(a,b,c,d) fwrite ((a), (b), (c), (d))
#endif


void Isam_Flush
  (
  Isam_Control_t *file_p                     /* ISAM file control block */
  )
{
  FILE           *f;                         /* Temporary file handle */
  U32_t           flag;                      /* Temporary */
  U8_t            i;                         /* Counter */

  f = IO_FileHandle_Get (Isam_File_Get (file_p));
  if (f == NULL)
    return;

  DEBUG (R_ISAM, DB_ISAMCLOSE, TL_PARAMS,
    (outbuf, "Entering Isam_Flush, control block = %p", file_p));

  /* If the file has been altered then we must write out the updated header
  */

  if (Isam_Flags_Altered_Get (file_p) == TRUE)
    {
    if (Isam_Curpos_Get (file_p) != Isam_Dataend_Get (file_p))
      if (fseek (f, (S32_t) Isam_Dataend_Get (file_p), 0) < 0)
        {
        perror ("Error seeking end of ISAM file");
        IO_Exit_Error (R_ISAM, X_LIBCALL,
          "Failed to reach end of ISAM file");
        }

    /* Write out the offset array and then the size array */

    if (Isam_fwrite (Array_Storage_Get (&file_p->recordsb), 1, 
        (Isam_NextKey_Get (file_p) + 1) * sizeof (U32_t), f) != 
        (Isam_NextKey_Get (file_p) + 1) * sizeof (U32_t))
      IO_Exit_Error (R_ISAM, X_LIBCALL, "Failed to write ISAM record array");

    if (Isam_fwrite (Array_Storage_Get (&file_p->sizesb), 1, 
        (Isam_NextKey_Get (file_p) + 1) * sizeof (U16_t), f) != 
        (Isam_NextKey_Get (file_p) + 1) * sizeof (U16_t))
      IO_Exit_Error (R_ISAM, X_LIBCALL, "Failed to write ISAM sizes array");

    if (fseek (f, 0, 0) < 0)
      {
      perror ("Error seeking front of ISAM file");
      IO_Exit_Error (R_ISAM, X_LIBCALL, "Failed to reach front of ISAM file");
      }

    Isam_Flags_Altered_Put (file_p, FALSE);

    if (Isam_fwrite (Isam_Head_Get (file_p), 1, ISAMHEADSIZE, f) != ISAMHEADSIZE)
      IO_Exit_Error (R_ISAM, X_LIBCALL, "Failed to write ISAM header");

    fflush (f);
    fseek (f, (S32_t) Isam_Curpos_Get (file_p), 0);
    }

  return;
}
/****************************************************************************
*
*  Function Name:                 Isam_Close
*
*    This function closes an ISAM file.  If it has been altered it will first
*    flush the header out to disk and the trailing offset and size arrays
*    as well.
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
*    May call IO_Exit_Error
*    File may be written
*    File is closed
*
******************************************************************************/
void Isam_Close
  (
  Isam_Control_t *file_p                     /* ISAM file control block */
  )
{
  FILE           *f;                         /* Temporary file handle */
  U32_t           flag;                      /* Temporary */
  U8_t            i;                         /* Counter */
#ifdef _MIND_MEM_
  char varname[100];
#endif

  f = IO_FileHandle_Get (Isam_File_Get (file_p));
  if (f == NULL)
    return;

  DEBUG (R_ISAM, DB_ISAMCLOSE, TL_PARAMS,
    (outbuf, "Entering Isam_Close, control block = %p", file_p));

  /* If the file has been altered then we must write out the updated header
  */

  if (Isam_Flags_Altered_Get (file_p) == TRUE)
    {
    if (Isam_Curpos_Get (file_p) != Isam_Dataend_Get (file_p))
      if (fseek (f, (S32_t) Isam_Dataend_Get (file_p), 0) < 0)
        {
        perror ("Error seeking end of ISAM file");
        IO_Exit_Error (R_ISAM, X_LIBCALL,
          "Failed to reach end of ISAM file");
        }

    /* Write out the offset array and then the size array */

    if (Isam_fwrite (Array_Storage_Get (&file_p->recordsb), 1, 
        (Isam_NextKey_Get (file_p) + 1) * sizeof (U32_t), f) != 
        (Isam_NextKey_Get (file_p) + 1) * sizeof (U32_t))
      IO_Exit_Error (R_ISAM, X_LIBCALL, "Failed to write ISAM record array");

    if (Isam_fwrite (Array_Storage_Get (&file_p->sizesb), 1, 
        (Isam_NextKey_Get (file_p) + 1) * sizeof (U16_t), f) != 
        (Isam_NextKey_Get (file_p) + 1) * sizeof (U16_t))
      IO_Exit_Error (R_ISAM, X_LIBCALL, "Failed to write ISAM sizes array");

    if (fseek (f, 0, 0) < 0)
      {
      perror ("Error seeking front of ISAM file");
      IO_Exit_Error (R_ISAM, X_LIBCALL, "Failed to reach front of ISAM file");
      }

    Isam_Flags_Altered_Put (file_p, FALSE);

    if (Isam_fwrite (Isam_Head_Get (file_p), 1, ISAMHEADSIZE, f) != ISAMHEADSIZE)
      IO_Exit_Error (R_ISAM, X_LIBCALL, "Failed to write ISAM header");
    }

  flag = fclose (f);
  if (flag == EOF)
    {
    perror ("Failed to correctly close ISAM file");
    IO_Exit_Error (R_ISAM, X_LIBCALL, "Error closing ISAM file");
    }

  /* Set file handle to NULL so as to prevent any mistakes.
     Free the buffer memory to minimize lost memory.
  */

  IO_FileHandle_Get (Isam_File_Get (file_p)) = NULL;

  for (i = 0; i < ISAM_NUM_BUFS; i++)
    {
#ifdef _MIND_MEM_
    sprintf (varname, "Isam_Buffer_Get(file_p,%d)", i);
    mind_free (varname, "isam", Isam_Buffer_Get (file_p, i));
#else
    Mem_Dealloc (Isam_Buffer_Get (file_p, i), Isam_BufSize_Get (file_p), 
      GLOBAL);
#endif

    DEBUG (R_ISAM, DB_ISAMCLOSE, TL_MEMORY, (outbuf,
      "Deallocated memory for an ISAM buffer in Isam_Close at %p",
      Isam_Buffer_Get (file_p, i)));
    }

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&file_p->recordsb", "isam", &file_p->recordsb);
  mind_Array_Destroy ("&file_p->sizesb", "isam", &file_p->sizesb);
#else
  Array_Destroy (&file_p->recordsb);
  Array_Destroy (&file_p->sizesb);
#endif

  DEBUG (R_ISAM, DB_ISAMCLOSE, TL_PARAMS,
    (outbuf, "Leaving Isam_Close, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Isam_Open
*
*    This function opens the ISAM file and reads in the header and sets up
*    the rest of the control block.  This includes reading in the trailing
*    offset and size arrays.  Recall that in the header the logical length
*    of the file is kept.
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
*    May call IO_Exit_Error
*    Opens the file
*
******************************************************************************/
void Isam_Open
  (
  Isam_Control_t *file_p,                    /* ISAM file control block */
  U8_t           type,                       /* What type of file is this? */
  U8_t           flag                        /* File to be written, inited */
  )
{
  FILE          *f;                          /* The file we are opening */
  U32_t          temp;                       /* Temporary */
  U32_t          i;                          /* Counter */
#ifdef _MIND_MEM_
  char varname[100];
#endif

  DEBUG (R_ISAM, DB_ISAMOPEN, TL_PARAMS,
    (outbuf, "Entering Isam_Open, control block = %p, type = %d, flag = %d",
    file_p, type, flag));

  /* Open the flag with the correct access (SUNos bug!  Does not support
     binary files!)
  */

  if (flag == ISAM_OPEN_READ)
#ifdef _WIN32
    f = Isam_fopen (gccfix (IO_FileName_Get (Isam_File_Get (file_p))), "rb");
  else
    if (flag == ISAM_OPEN_WRITE)
      f = Isam_fopen (gccfix (IO_FileName_Get (Isam_File_Get (file_p))), "r+b");
  else
    if (flag == ISAM_OPEN_INIT)
      f = Isam_fopen (gccfix (IO_FileName_Get (Isam_File_Get (file_p))), "w+b");
#else
    f = Isam_fopen (IO_FileName_Get (Isam_File_Get (file_p)), "rb");
  else
    if (flag == ISAM_OPEN_WRITE)
      f = Isam_fopen (IO_FileName_Get (Isam_File_Get (file_p)), "r+b");
  else
    if (flag == ISAM_OPEN_INIT)
      f = Isam_fopen (IO_FileName_Get (Isam_File_Get (file_p)), "w+b");
#endif
  else
    f = NULL;

  if (f == NULL)
    {
    perror ("Failed to open ISAM file");
perror (IO_FileName_Get (Isam_File_Get (file_p)));
    IO_Exit_Error (R_ISAM, X_SYSCALL, "Failed to open ISAM file");
    }

  if (flag == ISAM_OPEN_INIT)
    {
    /* To initialize the control block we must set the record keys to INVALID
       and the sizes to 0.  Fill in header block with default values.
    */

#ifdef _MIND_MEM_
    mind_Array_1d_Create ("&file_p->recordsb", "isam{2}", &file_p->recordsb, ISAM_MIN_KEYINCR, LONGSIZE);
    Array_Set (&file_p->recordsb, ISAM_INVALID);

    mind_Array_1d_Create ("&file_p->sizesb", "isam{2}", &file_p->sizesb, ISAM_MIN_KEYINCR, WORDSIZE);
    Array_Set (&file_p->sizesb, 0);
#else
    Array_1d_Create (&file_p->recordsb, ISAM_MIN_KEYINCR, LONGSIZE);
    Array_Set (&file_p->recordsb, ISAM_INVALID);

    Array_1d_Create (&file_p->sizesb, ISAM_MIN_KEYINCR, WORDSIZE);
    Array_Set (&file_p->sizesb, 0);
#endif

    temp = type | IsamM_Dense;
    Isam_Flags_Put (file_p, temp);
    Isam_Version_Put (file_p, ISAM_VERSION);
    Isam_Dataend_Put (file_p, ISAMHEADSIZE);
    Isam_MaxRecSize_Put (file_p, ISAM_MIN_RECSIZE);
    Isam_Crc_Put (file_p, 0);
    Isam_NextKey_Put (file_p, 0);

    if (Isam_fwrite (Isam_Head_Get (file_p), 1, ISAMHEADSIZE, f) != ISAMHEADSIZE)
      IO_Exit_Error (R_ISAM, X_LIBCALL, "Failed to write ISAM header");
    }
  else
    {
    if (Isam_fread (Isam_Head_Get (file_p), 1, ISAMHEADSIZE, f) != ISAMHEADSIZE)
      IO_Exit_Error (R_ISAM, X_LIBCALL, "Failed to read ISAM header");

    i = Isam_NextKey_Get (file_p) + 2 * ISAM_MIN_KEYINCR;
    i &= ~(ISAM_MIN_KEYINCR - 1);
#ifdef _MIND_MEM_
    mind_Array_1d_Create ("&file_p->recordsb", "isam{2a}", &file_p->recordsb, i, LONGSIZE);
    Array_Set (&file_p->recordsb, ISAM_INVALID);

    mind_Array_1d_Create ("&file_p->sizesb", "isam{2a}", &file_p->sizesb, i, WORDSIZE);
    Array_Set (&file_p->sizesb, 0);
#else
    Array_1d_Create (&file_p->recordsb, i, LONGSIZE);
    Array_Set (&file_p->recordsb, ISAM_INVALID);

    Array_1d_Create (&file_p->sizesb, i, WORDSIZE);
    Array_Set (&file_p->sizesb, 0);
#endif

    if (fseek (f, (S32_t) Isam_Dataend_Get (file_p), 0) < 0)
      {
      perror ("Dataend Seek failed on ISAM file");
      IO_Exit_Error (R_ISAM, X_LIBCALL, "Seek failed on ISAM file");
      }

    if (Isam_NextKey_Get (file_p) != 0)
      {
      if (Isam_fread (Array_Storage_Get (&file_p->recordsb), 1,
          (Isam_NextKey_Get (file_p) + 1) * sizeof (U32_t), f) !=
          (Isam_NextKey_Get (file_p) + 1) * sizeof (U32_t))
        IO_Exit_Error (R_ISAM, X_LIBCALL, "Failed to read ISAM record array");

      if (Isam_fread (Array_Storage_Get (&file_p->sizesb), 1,
          (Isam_NextKey_Get (file_p) + 1) * sizeof (U16_t), f) !=
          (Isam_NextKey_Get (file_p) + 1) * sizeof (U16_t))
        IO_Exit_Error (R_ISAM, X_LIBCALL, "Failed to read ISAM sizes array");
      }
    }

  /* File is open, header has been read.
     - Check that this file is the correct type and that we know how to read it
     - Want to figure out largest record size so that we can set buffer size
     - Allocate buffer memory (now that header is so big this may move to a
       create routine)
     - Rewind the file to the first record data
     - Set the file handle, current buffer and current seek position
  */

  if (type != ISAM_TYPE_ANY ? Isam_Type_Get (file_p) != type : FALSE ||
      Isam_Version_Get (file_p) != ISAM_VERSION)
    IO_Exit_Error (R_ISAM, X_SYNERR, "Type or version mismatch for ISAM file");

  for (i = 0, temp = 0; i < Isam_NextKey_Get (file_p); i++)
    if (temp < Isam_RecSize_Get (file_p, i))
      temp = Isam_RecSize_Get (file_p, i);

  temp = MAX (temp, ISAM_MIN_RECSIZE);
  Isam_MaxRecSize_Put (file_p, (U16_t) temp);

  /* We want to be able to read *at least* two records, and then some */

  temp = ((temp << 1) + ISAM_MIN_BUFSIZE) & ~((ISAM_MIN_BUFSIZE >> 2) - 1);
  Isam_BufSize_Put (file_p, temp);

  for (i = 0; i < ISAM_NUM_BUFS; i++)
    {
    Isam_Offset_Put (file_p, i, INFINITY);
#ifdef _MIND_MEM_
    sprintf (varname, "Isam_Buffer_Get(file_p,%d):%s", i, IO_FileName_Get (Isam_File_Get (file_p)));
    mind_malloc (varname, "isam{2}", &Isam_Buffer_Get (file_p, i), temp);
#else
    Mem_Alloc (U8_t *, Isam_Buffer_Get (file_p, i), temp, GLOBAL);
#endif

    DEBUG (R_ISAM, DB_ISAMOPEN, TL_MEMORY,
      (outbuf, "Allocated memory for an ISAM buffer in Isam_Open at %p",
      Isam_Buffer_Get (file_p, i)));

    if (Isam_Buffer_Get (file_p, i) == NULL)
      IO_Exit_Error (R_ISAM, X_LIBCALL, "No memory for ISAM buffer");
    }

  /* Update last few things */

  if (fseek (f, ISAMHEADSIZE, 0) < 0)
    {
    perror ("ISAMHEADSIZE Seek failed on ISAM file");
    IO_Exit_Error (R_ISAM, X_LIBCALL, "Seek failed on ISAM file");
    }

  IO_FileHandle_Put (Isam_File_Get (file_p), f);
  Isam_Curbuf_Put (file_p, 0);
  Isam_Curpos_Put (file_p, ISAMHEADSIZE);

  DEBUG (R_ISAM, DB_ISAMOPEN, TL_PARAMS,
    (outbuf, "Leaving Isam_Open, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Isam_Read
*
*    This function returns a pointer to the buffer which contains the first
*    part of the specified record.  It may do an actual disk read, but the
*    ISAM system will attempt to do some caching on behalf of the callers.
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
*    Address of the first portion of the record (NULL if key not found)
*
*  Side Effects:
*
*    May alter buffer state which may render previously returned pointers
*      invalid.
*
******************************************************************************/
void *Isam_Read
  (
  Isam_Control_t *file_p,                    /* ISAM file control block */
  U32_t          key                         /* Which record to read */
  )
{
  FILE          *f;                          /* File handle */
  void          *offset;                     /* Pointer to start of record */
  U32_t          curoff;                     /* Offset of current buffer */
  U32_t          keyrec;                     /* Offset of keyed record */
  U32_t          bufsize;                    /* Size of read buffers */
  U16_t          count;                      /* Counter / flag */
  U16_t          recsize;                    /* Size of specified record */
  U8_t           curbuf;                     /* Current buffer */
  U8_t           oldbuf;                     /* Old buffer (for copying) */

  keyrec = Isam_Record_Get (file_p, key);
  if (keyrec == ISAM_INVALID)
    return NULL;

  DEBUG (R_ISAM, DB_ISAMREAD, TL_PARAMS,
    (outbuf, "Entering Isam_Read, control block = %p, key = %lu",
    file_p, key));

  ASSERT (key < Isam_NextKey_Get (file_p),
    {
    IO_Exit_Error (R_ISAM, X_SYNERR, "Key value too large for file");
    });

  /* Algorithm for reads:
     - Check record fully in current buffer (return pointer if found)
     - Check current file position (seek if necessary)
     - Copy first part of record into next buffer
     - Read remaining buffer space into next buffer
     - Return pointer
  */

  curbuf = Isam_Curbuf_Get (file_p);
  curoff = Isam_Offset_Get (file_p, curbuf);
  bufsize = Isam_BufSize_Get (file_p);
  recsize = Isam_RecSize_Get (file_p, key);

  if (keyrec < curoff || keyrec + recsize > curoff + bufsize)
    {
    /* Record not wholly in the current buffer.  Get a new buffer.  Figure out
       how much is in this buffer, copy it, read in the rest of the buffer,
       continue.
    */

    oldbuf = curbuf;
    curbuf = (curbuf + 1) % ISAM_NUM_BUFS;
    f = IO_FileHandle_Get (Isam_File_Get (file_p));

    if (keyrec >= curoff && keyrec < curoff + bufsize)
      {
      recsize = (U16_t)(curoff + bufsize - keyrec);
      memcpy (Isam_Buffer_Get (file_p, curbuf),
        Isam_Buffer_Get (file_p, oldbuf) + keyrec - curoff, recsize);

      curoff += bufsize;
      bufsize -= recsize;
      }
    else
      {
      curoff = keyrec & ~(ISAM_MIN_BUFSIZE - 1);
      recsize = 0;
      }

    if (fseek (f, (S32_t) curoff, 0) < 0)
      {
      perror ("curoff(1) Seek failed on ISAM file");
      IO_Exit_Error (R_ISAM, X_LIBCALL, "Seek failed on ISAM file");
      }

    count = Isam_fread (Isam_Buffer_Get (file_p, curbuf) + recsize, 1, bufsize, f);
    if (count != bufsize && (curoff + bufsize < Isam_Dataend_Get (file_p)))
      {
      perror ("Failed to read a full ISAM buffer");
      IO_Exit_Error (R_ISAM, X_LIBCALL, "Failed to read a full ISAM buffer");
      }

    /* Check if we read a full buffer.  If not, then reset current offset and
       buffer sizes so that the information from the read can be recorded.
    */

    if (recsize != 0)
      curoff = keyrec;

    Isam_Curbuf_Put (file_p, curbuf);
    Isam_Offset_Put (file_p, curbuf, curoff);
    Isam_Curpos_Put (file_p, curoff + bufsize);
    }

  offset = keyrec - curoff + Isam_Buffer_Get (file_p, curbuf);

  DEBUG (R_ISAM, DB_ISAMREAD, TL_PARAMS,
    (outbuf, "Leaving Isam_Read, record offset = %p", offset));

  return offset;
}

/****************************************************************************
*
*  Function Name:                 Isam_Read_Nobuffer
*
*    This function reads without buffering.  The caller provides the buffer 
*    and the data is read directly into it.  Overflow is checked for.
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
*    TRUE  - Data is valid
*    FALSE - Buffer was too small or key was invalid
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t Isam_Read_Nobuffer
  (
  Isam_Control_t *file_p,                    /* ISAM file control block */
  U32_t          key,                        /* Which record to read */
  void          *buf_p,                      /* Address to store data at */
  U16_t          size                        /* Size of buffer supplied */
  )
{
  FILE          *f;                          /* File handle (temporary) */
  U32_t          curoff;                     /* Current (seek) offset */
  U16_t          cursize;                    /* Size of record to be read */
  U16_t          count;                      /* Amount read from file */
  Boolean_t      status;                     /* Result status */

  curoff = Isam_Record_Get (file_p, key);
  if (curoff == ISAM_INVALID)
    return FALSE;

  DEBUG (R_ISAM, DB_ISAMREADNOBUF, TL_PARAMS, (outbuf,
    "Entering Isam_Read_Nobuffer, control block = %p, key = %lu,"
    " buffer = %p, size = %u",file_p, key, buf_p, size));

  ASSERT (key < Isam_NextKey_Get (file_p),
    {
    IO_Exit_Error (R_ISAM, X_SYNERR, "Key value too large for file");
    });

  /* Algorithm for reading without a buffer:
     - Check that buffer is big enough
     - Check current position (seek if necessary)
     - Read into buffer
  */

  cursize = Isam_RecSize_Get (file_p, key);
  if (cursize > size)
    status = FALSE;
  else
    {
    f = IO_FileHandle_Get (Isam_File_Get (file_p));
    if (Isam_Curpos_Get (file_p) != curoff)
      if (fseek (f, (S32_t) curoff, 0) < 0)
        {
        perror ("curoff(2) Seek failed on ISAM file");
        IO_Exit_Error (R_ISAM, X_LIBCALL, "Seek failed on ISAM file");
        }

    count = Isam_fread (buf_p, 1, cursize, f);
    if (count != cursize)
      {
      perror ("Error reading ISAM file");
      IO_Exit_Error (R_ISAM, X_LIBCALL, "Failed to read the full ISAM record");
      }

    Isam_Curpos_Put (file_p, curoff + cursize);
    status = TRUE;
    }

  DEBUG (R_ISAM, DB_ISAMREADNOBUF, TL_PARAMS,
    (outbuf, "Leaving Isam_Read_Nobuffer, status = %d", status));

  return status;
}

/****************************************************************************
*
*  Function Name:                 Isam_Write
*
*    This function writes the given buffer to the file.  It will let the caller
*    know if they are creating holes in the file or if they are writing the
*    records "out of order".
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
*    SYN_NORMAL   - everything went as hoped
*    SYN_NOTDENSE - this write created/filled in a "missing" key value
*
*  Side Effects:
*
*    File is written
*
******************************************************************************/
SynCondValue Isam_Write
  (
  Isam_Control_t *file_p,                    /* ISAM file control block */
  U32_t          key,                        /* Which key to write */
  void          *buf_p,                      /* Buffer of data */
  U16_t          size                        /* Size of data */
  )
{
  FILE          *f;                          /* File handle (temporary) */
  Array_t        temp;                       /* For expanding key slots */
  U32_t          curoff;                     /* Current (seek) offset */
  U32_t          newsize;                    /* new size of storage */
  SynCondValue   status;                     /* Result status */
  U16_t          i;                          /* Counter */
#ifdef _MIND_MEM_
  char varname[100];
#endif

  DEBUG (R_ISAM, DB_ISAMWRITE, TL_PARAMS, (outbuf,
    "Entering Isam_Write, control block = %p, key = %lu, buffer = %p,"
    " size = %u", file_p, key, buf_p, size));

  /* Check if the record offset and size arrays need to be expanded.
     This is a bad design.  The size should go up by some factor related
     to the current size of the index in order to produce an efficient
     algorithm.
  */

  if ((Isam_NextKey_Get (file_p) >= Isam_MaxRecKey_Get (file_p)) ||
      (key != INFINITY && key >= Isam_MaxRecKey_Get (file_p)))
    {
#ifdef _MIND_MEM_
    mind_Array_1d_Create ("&temp", "isam{3}", &temp, Isam_MaxRecKey_Get (file_p) + ISAM_MIN_KEYINCR,
      LONGSIZE);
#else
    Array_1d_Create (&temp, Isam_MaxRecKey_Get (file_p) + ISAM_MIN_KEYINCR,
      LONGSIZE);
#endif
    Array_Set (&temp, ISAM_INVALID);

    /*  Array_CopyContents no longer works since arrays are of different 
        sizes.  Use code from the function directly (DK).
    Array_CopyContents (&file_p->recordsb, &temp);
    */
    newsize = MAX (Array_Rows_Get (&file_p->recordsb), 1) 
      * MAX (Array_Columns_Get (&file_p->recordsb), 1) 
      * MAX (Array_Planes_Get (&file_p->recordsb), 1);

    if (Array_Size_Get (&file_p->recordsb) == BITSIZE)
      newsize = ((newsize + (BITUNITSIZE * 8)) / (BITUNITSIZE * 8)) 
        * BITUNITSIZE;
    else
      newsize = newsize * Array_Size_Get (&file_p->recordsb);

    (void) memcpy (Array_Storage_Get (&temp), 
      Array_Storage_Get (&file_p->recordsb), newsize);

#ifdef _MIND_MEM_
    mind_Array_Destroy ("&file_p->recordsb", "isam", &file_p->recordsb);
#else
    Array_Destroy (&file_p->recordsb);
#endif

    file_p->recordsb = temp;

#ifdef _MIND_MEM_
    mind_Array_1d_Create ("&temp", "isam{3a}", &temp, Isam_MaxRecKey_Get (file_p) + ISAM_MIN_KEYINCR,
      WORDSIZE);
#else
    Array_1d_Create (&temp, Isam_MaxRecKey_Get (file_p) + ISAM_MIN_KEYINCR,
      WORDSIZE);
#endif
    Array_Set (&temp, 0);

    /*  Array_CopyContents no longer works since arrays are of different 
        sizes.  Use code from the function directly (DK).
    Array_CopyContents (&file_p->sizesb, &temp);
    */
    newsize = MAX (Array_Rows_Get (&file_p->sizesb), 1) 
      * MAX (Array_Columns_Get (&file_p->sizesb), 1) 
      * MAX (Array_Planes_Get (&file_p->sizesb), 1);

    if (Array_Size_Get (&file_p->sizesb) == BITSIZE)
      newsize = ((newsize + (BITUNITSIZE * 8)) / (BITUNITSIZE * 8)) 
        * BITUNITSIZE;
    else
      newsize = newsize * Array_Size_Get (&file_p->sizesb);

    (void) memcpy (Array_Storage_Get (&temp), 
      Array_Storage_Get (&file_p->sizesb), newsize);

#ifdef _MIND_MEM_
    mind_Array_Destroy ("&file_p->sizesb", "isam", &file_p->sizesb);
#else
    Array_Destroy (&file_p->sizesb);
#endif

    file_p->sizesb = temp;
    }

  ASSERT ((Isam_Record_Get (file_p, key) == ISAM_INVALID) ||
          /* allow rewrite to last record regardless of size to permit multiple edits of last record */
          (key == Isam_NextKey_Get (file_p) - 1) ||
          (Isam_RecSize_Get (file_p, key) == size),
    {
    IO_Exit_Error (R_ISAM, X_SYNERR,
      "Key already has data associated with it or size is incorrect on rewrite");
    });

  f = IO_FileHandle_Get (Isam_File_Get (file_p));

  Isam_Flags_Altered_Put (file_p, TRUE);

  /* Check if this record is being rewritten */

  if (Isam_Record_Get (file_p, key) != ISAM_INVALID)
    {
    curoff = Isam_Record_Get (file_p, key);

    if (Isam_Curpos_Get (file_p) != curoff)
      if (fseek (f, (S32_t) curoff, 0) < 0)
        {
        perror ("curoff(3) Seek failed on ISAM file");
        IO_Exit_Error (R_ISAM, X_LIBCALL, "Seek failed on ISAM file");
        }

    if (Isam_fwrite (buf_p, 1, size, f) != size)
      IO_Exit_Error (R_ISAM, X_LIBCALL, "Failed to write ISAM record");

    if (key == Isam_NextKey_Get (file_p) - 1)
      /* must revise size and data end to retain integrity */
      {
      Isam_RecSize_Put (file_p, key, size);
      Isam_Dataend_Put (file_p, curoff + size);
      }
    }
  else
    {
    /* This is the first write to this key, make sure we are writing to the
       end of the file.
    */

    curoff = Isam_Dataend_Get (file_p);

    if (Isam_Curpos_Get (file_p) != curoff)
      if (fseek (f, (S32_t) curoff, 0) < 0)
        {
        perror ("curoff(4) Seek failed on ISAM file");
        IO_Exit_Error (R_ISAM, X_LIBCALL, "Seek failed on ISAM file");
        }

    if (Isam_fwrite (buf_p, 1, size, f) != size)
      IO_Exit_Error (R_ISAM, X_LIBCALL, "Failed to write ISAM record");

    if (key == INFINITY || key == Isam_NextKey_Get (file_p))
      {
      key = Isam_NextKey_Get (file_p);

      Isam_NextKey_Put (file_p, key + 1);

      status = SYN_NORMAL;
      }
    else
      {
      status = SYN_NOTDENSE;

      if (key > Isam_NextKey_Get (file_p))
        Isam_NextKey_Put (file_p, key + 1);
      }

    Isam_Record_Put (file_p, key, curoff);
    Isam_RecSize_Put (file_p, key, size);
    Isam_Dataend_Put (file_p, curoff + size);
    }

  Isam_Curpos_Put (file_p, curoff + size);

  /* Check if we have a new maximum record size */

  if (Isam_MaxRecSize_Get (file_p) < size)
    Isam_MaxRecSize_Put (file_p, size);

  /* Check if this write is too big for the current buffer size */

  if (Isam_BufSize_Get (file_p) <= size)
    {
    size += Isam_BufSize_Get (file_p) + ISAM_MIN_BUFSIZE - 1;
    size &= ~(ISAM_MIN_BUFSIZE - 1);

    for (i = 0; i < ISAM_NUM_BUFS; i++)
      {
      Isam_Offset_Put (file_p, i, INFINITY);
#ifdef _MIND_MEM_
      sprintf (varname, "Isam_Buffer_Get(file_p,%d)", i);
      mind_free (varname, "isam", Isam_Buffer_Get (file_p, i));
#else
      Mem_Dealloc (Isam_Buffer_Get (file_p, i), Isam_BufSize_Get (file_p), 
        GLOBAL);
#endif

      DEBUG (R_ISAM, DB_ISAMWRITE, TL_MEMORY,
        (outbuf, "Deallocated memory for an ISAM buffer in Isam_Write at %p",
        Isam_Buffer_Get (file_p, i)));

#ifdef _MIND_MEM_
      sprintf (varname, "Isam_Buffer_Get(file_p,%d)", i);
      mind_malloc (varname, "isam{3}", &Isam_Buffer_Get (file_p, i), size);
#else
      Mem_Alloc (U8_t *, Isam_Buffer_Get (file_p, i), size, GLOBAL);
#endif

      DEBUG (R_ISAM, DB_ISAMWRITE, TL_MEMORY,
        (outbuf, "Allocated memory for an ISAM buffer in Isam_Write at %p",
        Isam_Buffer_Get (file_p, i)));

      if (Isam_Buffer_Get (file_p, i) == NULL)
        IO_Exit_Error (R_ISAM, X_LIBCALL, "No memory for ISAM buffer");
      }

    Isam_BufSize_Put (file_p, size);
    }

  /* Need to invalidate the buffers because of the write.  For the moment
     use a simple trick of setting the offset to something very large, it
     just can't overflow the check in Isam_Read.
  */

  Isam_Offset_Put (file_p, Isam_Curbuf_Get (file_p), INFINITY -
    Isam_BufSize_Get (file_p));

  DEBUG (R_ISAM, DB_ISAMWRITE, TL_PARAMS,
    (outbuf, "Leaving Isam_Write, status = %lu", status));

  return status;
}
/* End of Isam_Write */
/* End of ISAM.C */
