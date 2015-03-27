#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "synchem.h"
#include "synio.h"
#include "array.h"
#include "rcb.h"
#include "utl.h"
#include "extern.h"

#ifdef _MIND_MEM_
extern Boolean_t in_cycles;
#endif

static U32_t nallocs = 0, allocd = 0, array_dim = 0, save_size;
static char *save_name, *save_module;
static char **allocs = NULL;

void before_subgenr (int schema, int inx)
{
  char varname[100];

  sprintf (varname, "00000000.before_subgenr.%d.%d",schema,inx);
  if (nallocs == allocd)
  {
    allocd += 10000;
    allocs = (char **) realloc (allocs, allocd * sizeof (char *));
  }
  allocs[nallocs] = (char *) malloc (strlen (varname) + 1);
  strcpy (allocs[nallocs], varname);
  nallocs++;
}

void in_subgenr (int position)
{
  char varname[100];

  sprintf (varname, "00000000.in_subgenr.%d",position);
  if (nallocs == allocd)
  {
    allocd += 10000;
    allocs = (char **) realloc (allocs, allocd * sizeof (char *));
  }
  allocs[nallocs] = (char *) malloc (strlen (varname) + 1);
  strcpy (allocs[nallocs], varname);
  nallocs++;
}

void after_subgenr (int schema, int inx)
{
  char varname[100];

  sprintf (varname, "00000000.after_subgenr.%d.%d",schema,inx);
  if (nallocs == allocd)
  {
    allocd += 10000;
    allocs = (char **) realloc (allocs, allocd * sizeof (char *));
  }
  allocs[nallocs] = (char *) malloc (strlen (varname) + 1);
  strcpy (allocs[nallocs], varname);
  nallocs++;
}

void dump_vars ()
{
  FileDsc_t tmp;
  FILE *f;
  int i;
  char type[10];
  void *ptr;

#ifndef _MIND_MEM_
return;
#endif
  f = fopen (FCB_SEQDIR_SYS ("/vardump"), "w");
  IO_FileHandle_Put (&tmp, f);
  for (i = 0; i < nallocs; i++)
  {
    fprintf (f, "%s\n", allocs[i]);
    sscanf(allocs[i],"%x.",(int *)&ptr);
    if (strstr(allocs[i],"sling{2}") || strstr(allocs[i],"sling{3}") ||
      strstr(allocs[i],"utl{11}") || strstr(allocs[i],"utl{12}")) fprintf(f,"\t\"%s\"\n",(char *)ptr);
    else if (strstr(allocs[i],"utl{7}"))
    {
      if (Stack_IsAtmArr((Stack_t *)ptr)) strcpy(type,"AtmArr");
      else if (Stack_IsNormal((Stack_t *)ptr)) strcpy(type,"Normal");
      else if (Stack_IsScalar((Stack_t *)ptr)) strcpy(type,"Scalar");
      else strcpy(type,"???");
      fprintf(f,"\ttop=0x%08x; bottom=0x08x; size=%d; type=%s\n",
        Stack_Top_Get((Stack_t *)ptr),Stack_Bottom_Get((Stack_t *)ptr),type);
    }
    else if (strstr(allocs[i],"utl{8}")) fprintf(f,"\t0x%08x\n",*(unsigned int *)ptr);
    else if (strstr(allocs[i],"xtr{5}.xtr_tmp")) Xtr_Dump((Xtr_t *)ptr,&tmp);
  }
  fclose (f);
}

void add_var (char *name, char *module, void *ptr, int size, U32_t adim, U32_t asize)
{
  char varname[100];

  sprintf (varname, "%08x.%s.%s%s.%d.%d.%d",
    (U32_t) ptr, module, name, adim?"<array>":"", size, adim, asize);
  if (nallocs == allocd)
  {
    allocd += 10000;
    allocs = (char **) realloc (allocs, allocd * sizeof (char *));
  }
  allocs[nallocs] = (char *) malloc (strlen (varname) + 1);
  strcpy (allocs[nallocs], varname);
  nallocs++;
}

void sub_var (char *name, char *module, void *ptr, U32_t adim, U32_t asize)
{
  char varname[100];
  int i, j;

  sprintf (varname, "%08x.", (U32_t) ptr);
  for (i = 0; i < nallocs; i++) if (!strncmp (allocs[i], varname, strlen (varname)))
  {
    free (allocs[i]);
    nallocs--;
    for (j = i; j < nallocs; j++) allocs[j] = allocs[j + 1];
    return;
  }
/*
return;
*/
  fprintf (stderr, "Error trying to free %p (%s.%s)\n", (U32_t) ptr, module, name);
  dump_vars ();
  exit (1);
}

void *mind_malloc (char *name, char *module, void **ptr, int size)
{
#ifdef _DEBUG_
printf("mind_malloc(\"%s\", \"%s\", ...) entered\n", name, module);
#endif
  *ptr = malloc (size);
#ifdef _MIND_MEM_
  if (/*FALSE*/ /* !*/in_cycles /**/)
#endif
    return;
  if (array_dim == 0) add_var (name, module, *ptr, size, 0, 0);
  else add_var (save_name, save_module, *ptr, size, array_dim, save_size);
#ifdef _DEBUG_
printf("mind_malloc() exited\n");
#endif
}

void *mind_realloc (char *name, char *module, void **ptr, void *prev_ptr, int size)
{
#ifdef _DEBUG_
printf("mind_realloc(\"%s\", \"%s\", ...) entered\n", name, module);
#endif
#ifdef _MIND_MEM_
  if (/*TRUE*/ ! /**/ in_cycles /**/)
  {
    if (array_dim == 0) sub_var (name, module, prev_ptr, 0, 0);
    else sub_var (save_name, save_module, prev_ptr, array_dim, save_size);
  }
#endif
  *ptr = realloc (prev_ptr, size);
#ifdef _MIND_MEM_
  if (/*FALSE*/ /* !*/ in_cycles /**/)
#endif
    return;
  if (array_dim == 0) add_var (name, module, *ptr, size, 0, 0);
  else add_var (save_name, save_module, *ptr, size, array_dim, save_size);
#ifdef _DEBUG_
printf("mind_realloc() exited\n");
#endif
}

void mind_free (char *name, char *module, void *ptr)
{
#ifdef _DEBUG_
printf("mind_free(\"%s\", \"%s\", ...) entered\n", name, module);
#endif
#ifdef _MIND_MEM_
  if (/*TRUE*/ ! /**/ in_cycles /**/)
  {
    if (array_dim == 0) sub_var (name, module, ptr, 0, 0);
    else sub_var (save_name, save_module, ptr, array_dim, save_size);
  }
#endif
  free (ptr);
#ifdef _DEBUG_
printf("mind_free() exited\n");
#endif
}

void mind_Array_Copy (char *outname, char *module, Array_t *in, Array_t *out)
{
#ifdef _DEBUG_
printf("mind_Array_Copy(\"%s\", \"%s\", ...) entered\n", outname, module);
#endif
#ifdef _MIND_MEM_
  save_size = Array_Size_Get (in);
  if (Array_Rows_Get (in) == 0) array_dim = 1;
  else if (Array_Planes_Get (in) == 0) array_dim = 2;
  else array_dim = 3;
  save_name = outname;
  save_module = module;
#endif
  Array_Copy (in, out);
#ifndef _MIND_MEM_
return;
#endif
  array_dim = 0;
#ifdef _DEBUG_
printf("mind_Array_Copy() exited\n");
#endif
}

void mind_Array_1d_Create (char *name, char *module, Array_t *array, U32_t columns, U32_t size)
{
#ifdef _DEBUG_
printf("mind_Array_1d_Create(\"%s\", \"%s\", ...) entered\n", name, module);
#endif
#ifdef _MIND_MEM_
  array_dim = 1;
  save_name = name;
  save_module = module;
#endif
  Array_1d_Create (array, columns, size);
#ifndef _MIND_MEM_
return;
#endif
  array_dim = 0;
#ifdef _DEBUG_
printf("mind_Array_1d_Create() exited\n");
#endif
}

void mind_Array_2d_Create (char *name, char *module, Array_t *array, U32_t rows, U32_t columns, U32_t size)
{
#ifdef _DEBUG_
printf("mind_Array_2d_Create(\"%s\", \"%s\", ...) entered\n", name, module);
#endif
#ifdef _MIND_MEM_
  array_dim = 2;
  save_name = name;
  save_module = module;
#endif
  Array_2d_Create (array, rows, columns, size);
#ifndef _MIND_MEM_
return;
#endif
  array_dim = 0;
#ifdef _DEBUG_
printf("mind_Array_2d_Create() exited\n");
#endif
}

void mind_Array_3d_Create (char *name, char *module, Array_t *array, U32_t planes, U32_t rows, U32_t columns, U32_t size)
{
#ifdef _DEBUG_
printf("mind_Array_3d_Create(\"%s\", \"%s\", ...) entered\n", name, module);
#endif
#ifdef _MIND_MEM_
  array_dim = 3;
  save_name = name;
  save_module = module;
#endif
  Array_3d_Create (array, planes, rows, columns, size);
#ifndef _MIND_MEM_
return;
#endif
  array_dim = 0;
#ifdef _DEBUG_
printf("mind_Array_3d_Create() exited\n");
#endif
}

void mind_Array_Destroy (char *name, char *module, Array_t *array)
{
#ifdef _DEBUG_
printf("mind_Array_Destroy(\"%s\", \"%s\", ...) entered\n", name, module);
#endif
#ifdef _MIND_MEM_
  save_size = Array_Size_Get (array);
  if (Array_Rows_Get (array) == 0) array_dim = 1;
  else if (Array_Planes_Get (array) == 0) array_dim = 2;
  else array_dim = 3;
  save_name = name;
  save_module = module;
#endif
  Array_Destroy (array);
#ifndef _MIND_MEM_
return;
#endif
  array_dim = 0;
#ifdef _DEBUG_
printf("mind_Array_Destroy() exited\n");
#endif
}
