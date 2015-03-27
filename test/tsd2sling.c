/****************************************************************************
*
*  Function Name:                 Tsd2Sling
*
*    This function converts a TSD molecule format into a Sling format.
*
*  Used to be:
*
*    tsdslna:, somewhat tsdslng:
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
*    Sling format of molecule
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Sling_t Tsd2Sling
  (
  Tsd_t        *tsd_p                      /* Molecule to convert */
  )
{
  Tsd_t        *tsd_tmp;                   /* Copy of TSD to munge */
  Stack_t      *stack_p;                   /* For traversing the m'cule */
  U16_t         num_atoms;                 /* # atoms in molecule */
  U16_t         retrace;                   /* Number of atoms to retrace */
  U16_t         atom;                      /* Counter */
  U16_t         pos;                       /* Index into Sling */
  U16_t         perm;                      /* Counter */
  S16_t         atomid;                    /* For conversion */
  S16_t         neighid;                   /* Neighbor's atom index */
  U8_t          neigh;                     /* Counter */
  U8_t          k;                         /* Counter */
  char          tbuf[6];                   /* Convert word to string */
  String_t      output;                    /* Result of the operation */
  String_t      temp_str;                  /* For conversion */
  Sling_t       sling;                     /* Output format */
  Array_t       bond_mult;                 /* 2-d byte, bond multiplicities */
  Array_t       parity_ok;                 /* 1-d bit, done with parity flag */
  Array_t       in_stack;                  /* 1-d bit, mark atom as in stack */
  Array_t       sling_pos;                 /* 1-d word, offsets into Sling */

  FILL (output, 0);

  if (tsd_p == NULL || !Tsd_NumAtoms_Get (tsd_p))
    return String2Sling (output);

  DEBUG (R_TSD, DB_TSD2SLING, TL_PARAMS, (outbuf,
    "Entering Tsd2Sling, tsd = %p", tsd_p));

  tsd_tmp = Tsd_Copy (tsd_p);
  num_atoms = Tsd_NumAtoms_Get (tsd_tmp);

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&in_stock", "sling{6}", &in_stack, num_atoms, BITSIZE);
  mind_Array_1d_Create ("&parity_ok", "sling{6}", &parity_ok, num_atoms, BITSIZE);
  mind_Array_1d_Create ("&sling_pos", "sling{6}", &sling_pos, num_atoms, WORDSIZE);
  mind_Array_2d_Create ("&bond_mult", "sling{6}", &bond_mult, num_atoms, MX_NEIGHBORS, BYTESIZE);
#else
  Array_1d_Create (&in_stack, num_atoms, BITSIZE);
  Array_1d_Create (&parity_ok, num_atoms, BITSIZE);
  Array_1d_Create (&sling_pos, num_atoms, WORDSIZE);
  Array_2d_Create (&bond_mult, num_atoms, MX_NEIGHBORS, BYTESIZE);
#endif
  Array_Set (&in_stack, FALSE);
  Array_Set (&parity_ok, TRUE);
  Array_Set (&bond_mult, 0);
  output = String_Create (NULL, num_atoms << 2);

  /* Number of atoms to retrace is initially zero.  The position in the sling
     of the first atom is zero, and the index for the next one is one.
  */

  retrace = 0;
  Array_1d16_Put (&sling_pos, 0, 0);
  pos = 1;

  for (atom = 0; atom < num_atoms; atom++)
    {
    for (neigh = 0; neigh < MX_NEIGHBORS; neigh++)
      {
      if (Tsd_Atom_NeighborBond_Get (tsd_tmp, atom, neigh) == BOND_NONE)
        {
        Array_2d8_Put (&bond_mult, atom, neigh, BOND_NONE_SYM);
        }
      else
        if (Tsd_Atom_NeighborBond_Get (tsd_tmp, atom, neigh) == BOND_SINGLE)
          {
          Array_2d8_Put (&bond_mult, atom, neigh, BOND_SINGLE_SYM);
          }
      else
        if (Tsd_Atom_NeighborBond_Get (tsd_tmp, atom, neigh) == BOND_DOUBLE)
          {
          Array_2d8_Put (&bond_mult, atom, neigh, BOND_DOUBLE_SYM);
          }
      else
        if (Tsd_Atom_NeighborBond_Get (tsd_tmp, atom, neigh) == BOND_TRIPLE)
          {
          Array_2d8_Put (&bond_mult, atom, neigh, BOND_TRIPLE_SYM);
          }
      else
        if (Tsd_Atom_NeighborBond_Get (tsd_tmp, atom, neigh) == BOND_VARIABLE)
          {
          Array_2d8_Put (&bond_mult, atom, neigh, BOND_VARIABLE_SYM);
          }
      else
        if (Tsd_Atom_NeighborBond_Get (tsd_tmp, atom, neigh) == BOND_RESONANT)
          {
          Array_2d8_Put (&bond_mult, atom, neigh, BOND_RESONANT_SYM);
          }
      else
        if (Tsd_Atom_NeighborBond_Get (tsd_tmp, atom, neigh) == (BOND_SINGLE
            | BOND_RESONANT))
          {
          Array_2d8_Put (&bond_mult, atom, neigh, BOND_SINGLE_OR_RESONANT_SYM);
          }
      else
        IO_Exit_Error (R_TSD, X_SYNERR,
          "Illegal bond found in TSD in Tsd2Sling");
      }
    }                      /* End for-atom loop */

  stack_p = Stack_Create (STACK_SCALAR);
  atomid = Tsd_Atomid_Get (tsd_tmp, 0);
  temp_str = SLetter (atomid);
  String_Concat (&output, temp_str);
  String_Destroy (temp_str);

  Stack_PushU16 (stack_p, 0);
  Array_1d1_Put (&in_stack, 0, TRUE);

  /* Continue while there remain bonds to be processed */

  while (Stack_Size_Get (stack_p) > 0)
    {
    atom = Stack_TopU16 (stack_p);

    DEBUG (R_TSD, DB_TSD2SLING, TL_MAJOR, (outbuf,
      "Sling = %s", String_Value_Get (output)));

    for (neigh = 0; neigh < MX_NEIGHBORS && Tsd_Atom_NeighborId_Get (tsd_tmp,
         atom, neigh) == TSD_INVALID; neigh++)
      /* Empty loop body */ ;

    /* Atom on top of stack has more neighbors */

    if (neigh < MX_NEIGHBORS)
      {
      if (retrace > 0)
        {
        String_Concat_c (&output, RETRACE_STR);
        Number2Char (retrace, tbuf);
        String_Concat_c (&output, tbuf);
        }

      retrace = 0;

      if (!Array_1d1_Get (&parity_ok, atom))
        {
        k = neigh;
        neigh++;
        for (; neigh < MX_NEIGHBORS && Tsd_Atom_NeighborId_Get (tsd_tmp,
             atom, neigh) == TSD_INVALID; neigh++)
          /* Empty loop body */ ;

        /* There is only one bond on which to exit from this atom, therefore
           since the parity is wrong . . .
        */

        if (neigh >= MX_NEIGHBORS)
          {
          String_Concat_c (&output, PARITY_SPACE_STR);
          neigh = k;
          }
        else
          Array_1d1_Put (&parity_ok, atom, TRUE);
        }

      neighid = Tsd_Atom_NeighborId_Get (tsd_tmp, atom, neigh);
      for (k = 0, perm = 0; k < MX_NEIGHBORS && Tsd_Atom_NeighborId_Get (
           tsd_tmp, neighid, k) != atom; k++)
        if (Tsd_Atom_NeighborId_Get (tsd_tmp, neighid, k) != TSD_INVALID)
          perm++;

      if ((perm % 2) == 1 && Tsd_Atomid_Get (tsd_tmp, neighid) == CARBON)
        Array_1d1_Put (&parity_ok, neighid, FALSE);

      Tsd_Atom_NeighborId_Put (tsd_tmp, atom, neigh, TSD_INVALID);
      Tsd_Atom_NeighborBond_Put (tsd_tmp, atom, neigh, BOND_NONE);
      Tsd_Atom_NeighborId_Put (tsd_tmp, neighid, k, TSD_INVALID);
      Tsd_Atom_NeighborBond_Put (tsd_tmp, neighid, k, BOND_NONE);
      tbuf[0] = Array_2d8_Get (&bond_mult, atom, neigh);
      tbuf[1] = '\0';
      String_Concat_c (&output, tbuf);

      if (!Array_1d1_Get (&in_stack, neighid))
        {
        temp_str = SLetter (Tsd_Atomid_Get (tsd_tmp, neighid));
        String_Concat (&output, temp_str);
        String_Destroy (temp_str);
        Array_1d1_Put (&in_stack, neighid, TRUE);
        Array_1d16_Put (&sling_pos, neighid, pos);
        pos++;
        }
      else
        {
        String_Concat_c (&output, SLASH_STR);
        Number2Char (Array_1d16_Get (&sling_pos, neighid), tbuf);
        String_Concat_c (&output, tbuf);
        }

      Stack_PushU16 (stack_p, neighid);
      }                      /* End if-neigh < MX_NEIGHBORS */
    else
      {
      Stack_Pop (stack_p);
      retrace++;
      }
    }                        /* End while-stack_size loop */

  sling = String2Sling (output);
  String_Destroy (output);
#ifdef _MIND_MEM_
  mind_Array_Destroy ("&in_stack", "sling", &in_stack);
  mind_Array_Destroy ("&parity_ok", "sling", &parity_ok);
  mind_Array_Destroy ("&sling_pos", "sling", &sling_pos);
  mind_Array_Destroy ("&bond_mult", "sling", &bond_mult);
#else
  Array_Destroy (&in_stack);
  Array_Destroy (&parity_ok);
  Array_Destroy (&sling_pos);
  Array_Destroy (&bond_mult);
#endif
  Stack_Destroy (stack_p);
  Tsd_Destroy (tsd_tmp);

  DEBUG (R_TSD, DB_TSD2SLING, TL_PARAMS, (outbuf,
    "Leaving Tsd2Sling, sling not shown"));

  return sling;
}
