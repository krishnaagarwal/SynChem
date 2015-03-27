#ifndef _H_ISAM_
#define _H_ISAM_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     ISAM.H
*
*    This module is the abstraction for an Indexed Sequential Access Method
*    file package for Synchem.  This package supports lookup by a single 32-bit
*    integer key.  It guarantees that the entire record will be available in
*    the buffer so it only returns a pointer to the record unless you
*    specifically call Isam_Read_NoBuffer.
*
*    Routines are found in ISAM.C
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
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred       xxx
*
******************************************************************************/

#ifndef _H_SYNIO_
#include "synio.h"
#endif

#ifndef _H_ARRAY_
#include "array.h"
#endif

/*** Literal Values ***/

#define ISAM_NUM_BUFS     2
#define ISAM_MIN_BUFSIZE  8096
#define ISAM_MIN_RECSIZE  1024
#define ISAM_MIN_KEYINCR  1024
#define ISAM_INVALID      (U32_t)-201
#define ISAM_VERSION      0x00000001        /* (Maj, Min) 16-bit fields */

#define ISAM_OPEN_READ    6                 /* Open access flags */
#define ISAM_OPEN_WRITE   7
#define ISAM_OPEN_INIT    8

#define ISAM_TYPE_AVLCOMP 1                 /* File type flags */
#define ISAM_TYPE_RKBDATA 2
#define ISAM_TYPE_RID     3
#define ISAM_TYPE_FGINFO  4
#define ISAM_TYPE_RKBTEXT 5
#define ISAM_TYPE_ANY     10                /* Escape for integrity facility */
#define ISAM_TYPE_MAX     11

#define ISAM_IARG_HEAD    "-h"              /* Integrity cmnd line options */
#define ISAM_IARG_INTACT  "-i"
#define ISAM_IARG_INTG    "-c"
#define ISAM_IARG_CAT     "-p"

/*** Data Structures ***/

/* The ISAM file has a binary format header.  Note that the files are NOT
   portable across different endian machines!  The header includes indexing,
   typing, and integrity verification information.  Note that the record
   offset array and the size array are kept at the end of the file so that
   a fixed amount of space need not be reserved for them.
*/

typedef struct s_isamhdr
  {
  U32_t          version;                    /* What version of the header */
  U32_t          type;                       /* Type and status bits */
  U32_t          crc;                        /* CRC integrity value */
  U32_t          next_key;                   /* Next key to write */
  U32_t          data_end;                   /* End of last record */
  } Isam_Head_t;
#define ISAMHEADSIZE sizeof (Isam_Head_t)

#define IsamM_Type       0xff
#define IsamM_Dense      0x100               /* No missing key values */
#define IsamM_Altered    0x200               /* File written to */

/* The in memory ISAM file control structure keeps track of what the current
   header looks like and the state of the file.
*/

typedef struct s_isamctl
  {
  Array_t        recordsb;                   /* 1d longword, rec. seek addr. */
  Array_t        sizesb;                     /* 1d word, rec. sizes */
  FileDsc_t      fileb;                      /* File descriptor */
  Isam_Head_t    headb;                      /* ISAM header */
  U32_t          seek;                       /* Current seek position */
  U32_t          bufsize;                    /* Size of allocated buffers */
  struct         s_bufctl
    {
    U8_t        *buffer;                     /* Actual buffer allocation */
    U32_t        offset;                     /* Offset this was read from */
    } b[ISAM_NUM_BUFS];                      /* Multi-buffering */
  U16_t          max_recsize;                /* Size of biggest record */
  U8_t           which;                      /* Which buffer is current */
  } Isam_Control_t;
#define ISAMCONTROLSIZE sizeof (Isam_Control_t)

/** Field Access Macros for Isam_Control_t */

/* Macro Prototypes
   U8_t        *Isam_Buffer_Get      (Isam_Control_t *, U8_t);
   U32_t        Isam_BufSize_Get     (Isam_Control_t *);
   void         Isam_BufSize_Put     (Isam_Control_t *, U32_t);
   U32_t        Isam_Crc_Get         (Isam_Control_t *);
   void         Isam_Crc_Put         (Isam_Control_t *, U32_t);
   U8_t         Isam_Curbuf_Get      (Isam_Control_t *);
   void         Isam_Curbuf_Put      (Isam_Control_t *, U8_t);
   U32_t        Isam_Curpos_Get      (Isam_Control_t *);
   void         Isam_Curpos_Put      (Isam_Control_t *, U32_t);
   U32_t        Isam_Dataend_Get     (Isam_Control_t *);
   void         Isam_Dataend_Put     (Isam_Control_t *, U32_t);
   FileDsc_t   *Isam_File_Get        (Isam_Control_t *);
   Boolean_t    Isam_Flags_Altered_Get (Isam_Control_t *);
   void         Isam_Flags_Altered_Put (Isam_Control_t *, Boolean_t);
   Boolean_t    Isam_Flags_Dense_Get (Isam_Control_t *);
   U32_t        Isam_Flags_Get       (Isam_Control_t *);
   void         Isam_Flags_Put       (Isam_Control_t *, U32_t);
   Isam_Head_t *Isam_Head_Get        (Isam_Control_t *);
   U16_t        Isam_MaxRecSize_Get  (Isam_Control_t *);
   void         Isam_MaxRecSize_Put  (Isam_Control_t *, U16_t);
   U32_t        Isam_NextKey_Get     (Isam_Control_t *);
   void         Isam_NextKey_Put     (Isam_Control_t *, U32_t);
   U32_t        Isam_Offset_Get      (Isam_Control_t *, U8_t);
   void         Isam_Offset_Put      (Isam_Control_t *, U8_t, U32_t);
   U32_t        Isam_Record_Get      (Isam_Control_t *, U32_t);
   void         Isam_Record_Put      (Isam_Control_t *, U32_t, U32_t);
   U16_t        Isam_RecSize_Get     (Isam_Control_t *, U32_t);
   void         Isam_RecSize_Put     (Isam_Control_t *, U32_t, U16_t);
   U8_t         Isam_Type_Get        (Isam_Control_t *);
   U32_t        Isam_Version_Get     (Isam_Control_t *);
   void         Isam_Version_Put     (Isam_Control_t *, U32_t);
*/

#ifndef FILE_DEBUG
#define Isam_Buffer_Get(isam_p, key)\
  (isam_p)->b[key].buffer

#define Isam_BufSize_Get(isam_p)\
  (isam_p)->bufsize

#define Isam_BufSize_Put(isam_p, value)\
  (isam_p)->bufsize = (value)

#define Isam_Crc_Get(isam_p)\
  (isam_p)->headb.crc

#define Isam_Crc_Put(isam_p, value)\
  (isam_p)->headb.crc = (value)

#define Isam_Curbuf_Get(isam_p)\
  (isam_p)->which

#define Isam_Curbuf_Put(isam_p, value)\
  (isam_p)->which = (value)

#define Isam_Curpos_Get(isam_p)\
  (isam_p)->seek

#define Isam_Curpos_Put(isam_p, value)\
  (isam_p)->seek = (value)

#define Isam_Dataend_Get(isam_p)\
  (isam_p)->headb.data_end

#define Isam_Dataend_Put(isam_p, value)\
  (isam_p)->headb.data_end = (value)

#define Isam_File_Get(isam_p)\
  ((FileDsc_t *)&(isam_p)->fileb)

#define Isam_Flags_Altered_Get(isam_p)\
  (((isam_p)->headb.type & IsamM_Altered) ? TRUE : FALSE)

#define Isam_Flags_Altered_Put(isam_p, value)\
  { if ((value) == TRUE)\
    (isam_p)->headb.type |= IsamM_Altered;\
  else\
    (isam_p)->headb.type &= ~IsamM_Altered; }

#define Isam_Flags_Dense_Get(isam_p)\
  (((isam_p)->headb.type & IsamM_Dense) ? TRUE : FALSE)

#define Isam_Flags_Get(isam_p)\
  (isam_p)->headb.type

#define Isam_Flags_Put(isam_p, value)\
  (isam_p)->headb.type = (value)

#define Isam_Head_Get(isam_p)\
  ((Isam_Control_t *)&(isam_p)->headb)

#define Isam_MaxRecKey_Get(isam_p)\
  Array_Columns_Get (&(isam_p)->recordsb)

#define Isam_MaxRecSize_Get(isam_p)\
  (isam_p)->max_recsize

#define Isam_MaxRecSize_Put(isam_p, value)\
  (isam_p)->max_recsize = (value)

#define Isam_NextKey_Get(isam_p)\
  (isam_p)->headb.next_key

#define Isam_NextKey_Put(isam_p, value)\
  (isam_p)->headb.next_key = (value)

#define Isam_Offset_Get(isam_p, key)\
  (isam_p)->b[key].offset

#define Isam_Offset_Put(isam_p, key, value)\
  (isam_p)->b[key].offset = (value)

#define Isam_Record_Get(isam_p, key)\
  Array_1d32_Get (&(isam_p)->recordsb, key)

#define Isam_Record_Put(isam_p, key, value)\
  Array_1d32_Put (&(isam_p)->recordsb, key, value)

#define Isam_RecSize_Get(isam_p, key)\
  Array_1d16_Get (&(isam_p)->sizesb, key)

#define Isam_RecSize_Put(isam_p, key, value)\
  Array_1d16_Put (&(isam_p)->sizesb, key, value)

#define Isam_Type_Get(isam_p)\
  ((isam_p)->headb.type & IsamM_Type)

#define Isam_Version_Get(isam_p)\
  (isam_p)->headb.version

#define Isam_Version_Put(isam_p, value)\
  (isam_p)->headb.version = (value)
#else
#define Isam_Buffer_Get(isam_p, key)\
  ((isam_p) < GBAddr || (key) >= ISAM_NUM_BUFS ?\
  ((Isam_Control_t *)HALTP)->b[key].buffer : (isam_p)->b[key].buffer)

#define Isam_BufSize_Get(isam_p)\
  ((isam_p) < GBAddr ? HALT : (isam_p)->bufsize)

#define Isam_BufSize_Put(isam_p, value)\
  { if ((isam_p) < GBAddr) HALT; else (isam_p)->bufsize = (value); }

#define Isam_Crc_Get(isam_p)\
  ((isam_p) < GBAddr ? HALT : (isam_p)->headb.crc)

#define Isam_Crc_Put(isam_p, value)\
  { if ((isam_p) < GBAddr) HALT; else (isam_p)->headb.crc = (value); }

#define Isam_Curbuf_Get(isam_p)\
  ((isam_p) < GBAddr ? HALT : (isam_p)->which)

#define Isam_Curbuf_Put(isam_p, value)\
  { if ((isam_p) < GBAddr) HALT; else (isam_p)->which = (value); }

#define Isam_Curpos_Get(isam_p)\
  ((isam_p) < GBAddr ? HALT : (isam_p)->seek)

#define Isam_Curpos_Put(isam_p, value)\
  { if ((isam_p) < GBAddr) HALT; else (isam_p)->seek = (value); }

#define Isam_Dataend_Get(isam_p)\
  ((isam_p) < GBAddr ? HALT : (isam_p)->headb.data_end)

#define Isam_Dataend_Put(isam_p, value)\
  { if ((isam_p) < GBAddr) HALT; else (isam_p)->headb.data_end = (value); }

#define Isam_File_Get(isam_p)\
  ((isam_p) < GBAddr ? HALT : ((FileDsc_t *)&(isam_p)->fileb))

#define Isam_Flags_Altered_Get(isam_p)\
  (((isam_p)->headb.type & IsamM_Altered) ? TRUE : FALSE)

#define Isam_Flags_Altered_Put(isam_p, value)\
  { if ((isam_p) < GBAddr) HALT; else if ((value) == TRUE)\
  (isam_p)->headb.type |= IsamM_Altered; else\
  (isam_p)->headb.type &= ~IsamM_Altered; }

#define Isam_Flags_Dense_Get(isam_p)\
  ((isam_p) < GBAddr ? HALT : (isam_p)->headb.type & IsamM_Dense ? TRUE : FALSE)

#define Isam_Flags_Get(isam_p)\
  ((isam_p) < GBAddr ? HALT : (isam_p)->headb.type)

#define Isam_Flags_Put(isam_p, value)\
  { if ((isam_p) < GBAddr) HALT; else (isam_p)->headb.type = (value); }

#define Isam_Head_Get(isam_p)\
  ((isam_p) < GBAddr ? HALT : (Isam_Control_t *)&(isam_p)->headb)

#define Isam_MaxRecKey_Get(isam_p)\
  ((isam_p) < GBAddr ? HALT : Array_Columns_Get (&(isam_p)->recordsb))

#define Isam_MaxRecSize_Get(isam_p)\
  ((isam_p) < GBAddr ? HALT : (isam_p)->max_recsize)

#define Isam_MaxRecSize_Put(isam_p, value)\
  { if ((isam_p) < GBAddr) HALT; else (isam_p)->max_recsize = (value); }

#define Isam_NextKey_Get(isam_p)\
  ((isam_p) < GBAddr ? HALT : (isam_p)->headb.next_key)

#define Isam_NextKey_Put(isam_p, value)\
  { if ((isam_p) < GBAddr) HALT; else (isam_p)->headb.next_key = (value); }

#define Isam_Offset_Get(isam_p, key)\
  ((isam_p) < GBAddr || (key) >= ISAM_NUM_BUFS ? HALT : (isam_p)->b[key].offset)

#define Isam_Offset_Put(isam_p, key, value)\
  { if ((isam_p) < GBAddr || (key) >= ISAM_NUM_BUFS) HALT; else\
  (isam_p)->b[key].offset = (value); }

/* Record/RecSize macros are (supposed to be?) used only in contexts where
   the key value has already been checked.
*/

#define Isam_Record_Get(isam_p, key)\
  ((isam_p) < GBAddr ? HALT : Array_1d32_Get (&(isam_p)->recordsb, key))

#define Isam_Record_Put(isam_p, key, value)\
  { if ((isam_p) < GBAddr) HALT; else Array_1d32_Put (&(isam_p)->recordsb, key, value); }

#define Isam_RecSize_Get(isam_p, key)\
  ((isam_p) < GBAddr ? HALT : Array_1d16_Get (&(isam_p)->sizesb, key))

#define Isam_RecSize_Put(isam_p, key, value)\
  { if ((isam_p) < GBAddr) HALT; else Array_1d16_Put (&(isam_p)->sizesb, key, value); }

#define Isam_Type_Get(isam_p)\
  ((isam_p) < GBAddr ? HALT : (isam_p)->headb.type & IsamM_Type)

#define Isam_Version_Get(isam_p)\
  ((isam_p) < GBAddr ? HALT : (isam_p)->headb.version)

#define Isam_Version_Put(isam_p, value)\
  { if ((isam_p) < GBAddr) HALT; else (isam_p)->headb.version = (value); }
#endif

/** End of Field Access Macros for Isam_Control_t **/

/*** Macros ***/

/*** Routine Prototypes ***/

void      Isam_Flush         (Isam_Control_t *);
void      Isam_Close         (Isam_Control_t *);
void      Isam_Open          (Isam_Control_t *, U8_t, U8_t);
void     *Isam_Read          (Isam_Control_t *, U32_t);
Boolean_t Isam_Read_Nobuffer (Isam_Control_t *, U32_t, void *, U16_t);
SynCondValue Isam_Write      (Isam_Control_t *, U32_t, void *, U16_t);

/*** Global Variables ***/

/* End of Isam.h */
#endif
