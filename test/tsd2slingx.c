/****************************************************************************
*
*  Function Name:                 Tsd2SlingX
*
*    This function converts a TSD molecule into a Sling format molecule, plus
*    it fills in a bunch of ancilliary information about the molecule.
*
*  Used to be:
*
*    tsdslnb:, somewhat tsdslnc:
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
Sling_t Tsd2SlingX
  (
  Tsd_t        *tsd_p,                     /* Molecule to convert */
  Array_t      *parityvec_p,               /* 1-d byte, parity vector */
  Array_t      *cebros_p,                  /* 1-d word, CE brothers */
  Array_t      *sebros_p,                  /* 1-d word, SE brothers */
  Array_t      *asymmetric_p,              /* 1-d bit, asymmetric flags */
  Array_t      *paritybits_p,              /* 1-d bit, parity flags (Out) */
  String_t     *ce_str_p,                  /* CE string format */
  String_t     *se_str_p,                  /* SE string format */
  U16_t         num_bros,                  /* Number of entries in C/SE bros */
  U16_t         num_carbons                /* Number of carbons found */
  )
{
  Tsd_t        *tsd_tmp;                   /* Copy of TSD to munge */
  Stack_t      *stack_p;                   /* For traversing the m'cule */
  U16_t         num_atoms;                 /* # atoms in molecule */
  U16_t         last_carbon;               /* Carbon index */
  U16_t         retrace;                   /* Number of atoms to retrace */
  U16_t         atom;                      /* Counter */
  U16_t         pos;                       /* Index into Sling */
  U16_t         perm;                      /* Counter */
  U16_t         atomid;                    /* For conversion */
  U16_t         neighid;                   /* Neighbor's atom index */
  U16_t          neigh;                     /* Counter */
  U16_t          k;                         /* Counter */
  char          tbuf[6];                   /* Convert word to string */
  String_t      output;                    /* Result of the operation */
  String_t      temp_str;                  /* For conversion */
  Sling_t       sling;                     /* For output */
  Array_t       bond_mult;                 /* 2-d byte, bond multiplicities */
  Array_t       parity_ok;                 /* 1-d bit, done with parity flag */
  Array_t       in_stack;                  /* 1-d bit, mark atom as in stack */
  Array_t       sling_pos;                 /* 1-d word, offsets into Sling */
  Array_t       parityvec_pos;             /* 1-d word, parity vector offset */
  Array_t       sorted_parity;             /* 2-d byte, sorted by Sling ord */
  Array_t       dontcare;                  /* 1-d bit, keep hydrogens? */

  FILL (output, 0);

  if (tsd_p == NULL || !Tsd_NumAtoms_Get (tsd_p))
    return String2Sling (output);

  DEBUG (R_TSD, DB_TSD2SLINGX, TL_PARAMS, (outbuf,
    "Entering Tsd2SlingX, tsd = %p, # brothers = %u, # carbons = %u",
    tsd_p, num_bros, num_carbons));

  TRACE_DO (DB_TSD2SLINGX, TL_MAJOR,
    {
    IO_Put_Trace (R_TSD, "Parity vector in Tsd2SlingX");
    Array_Dump (parityvec_p, &GTraceFile);
    IO_Put_Trace (R_TSD, "CE Brothers");
    Array_Dump (cebros_p, &GTraceFile);
    IO_Put_Trace (R_TSD, "SE Brothers");
    Array_Dump (sebros_p, &GTraceFile);
    IO_Put_Trace (R_TSD, "Asymmetric flags");
    Array_Dump (asymmetric_p, &GTraceFile);
    });

  tsd_tmp = Tsd_Copy (tsd_p);
  num_atoms = Tsd_NumAtoms_Get (tsd_tmp);

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&in_stack", "sling{7}", &in_stack, num_atoms, BITSIZE);
  mind_Array_1d_Create ("&parity_ok", "sling{7}", &parity_ok, num_atoms, BITSIZE);
  mind_Array_1d_Create ("&dontcare", "sling{7}", &dontcare, num_atoms, BITSIZE);
  mind_Array_1d_Create ("&sling_pos", "sling{7}", &sling_pos, num_atoms, WORDSIZE);
  mind_Array_2d_Create ("&bond_mult", "sling{7}", &bond_mult, num_atoms, MX_NEIGHBORS, BYTESIZE);
#else
  Array_1d_Create (&in_stack, num_atoms, BITSIZE);
  Array_1d_Create (&parity_ok, num_atoms, BITSIZE);
  Array_1d_Create (&dontcare, num_atoms, BITSIZE);
  Array_1d_Create (&sling_pos, num_atoms, WORDSIZE);
  Array_2d_Create (&bond_mult, num_atoms, MX_NEIGHBORS, BYTESIZE);
#endif
  Array_Set (&in_stack, FALSE);
  Array_Set (&dontcare, FALSE);
  Array_Set (&parity_ok, TRUE);
  Array_Set (&bond_mult, 0);
  output = String_Create (NULL, num_atoms << 2);

  retrace = 0;
  Array_1d16_Put (&sling_pos, 0, 0);
  pos = 1;

  for (atom = 0, last_carbon = 0; atom < num_atoms; atom++)
    {
    if (Tsd_Atomid_Get (tsd_tmp, atom) != CARBON)
      Array_1d1_Put (&dontcare, atom, TRUE);
    else
      {
      Array_1d1_Put (&dontcare, atom, Array_1d8_Get (parityvec_p,
        last_carbon) == PARITY_INIT);
      last_carbon++;
      }

    for (neigh = 0; neigh < MX_NEIGHBORS; neigh++)
      {
      if (Tsd_Atomid_Get (tsd_tmp, atom) == CARBON && Array_1d1_Get (
            &dontcare, atom) == TRUE)
        if (Tsd_Atom_NeighborId_Get (tsd_tmp, atom, neigh) != TSD_INVALID &&
            Tsd_Atomid_Get (tsd_tmp, Tsd_Atom_NeighborId_Get (tsd_tmp, atom,
            neigh)) == HYDROGEN)
          Tsd_Atom_NeighborId_Put (tsd_tmp, atom, neigh, TSD_INVALID);

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
          "Illegal bond found in TSD in Tsd2SlingX");
      }
    }

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

    DEBUG (R_TSD, DB_TSD2SLINGX, TL_MAJOR, (outbuf,
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

      if (Array_1d1_Get (&parity_ok, atom) == FALSE)
        {
        k = neigh;
        neigh++;
        for (; neigh < MX_NEIGHBORS && Tsd_Atom_NeighborId_Get (tsd_tmp,\
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

      if ((perm % 2) == 1 && Array_1d1_Get (&dontcare, neighid) == FALSE)
        Array_1d1_Put (&parity_ok, neighid, FALSE);

      Tsd_Atom_NeighborId_Put (tsd_tmp, atom, neigh, TSD_INVALID);
      Tsd_Atom_NeighborId_Put (tsd_tmp, neighid, k, TSD_INVALID);
      tbuf[0] = Array_2d8_Get (&bond_mult, atom, neigh);
      tbuf[1] = '\0';
      String_Concat_c (&output, tbuf);

      if (Array_1d1_Get (&in_stack, neighid) == FALSE)
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

  Tsd_Destroy (tsd_tmp);
  tsd_tmp = Tsd_Copy (tsd_p);

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&parityvec_pos", "sling{7}", &parityvec_pos, num_atoms, WORDSIZE);
  mind_Array_1d_Create ("paritybits_p", "sling{7}", paritybits_p, num_carbons, BITSIZE);
  mind_Array_2d_Create ("&sorted_parity", "sling{7}", &sorted_parity, num_atoms, 2, BYTESIZE);
#else
  Array_1d_Create (&parityvec_pos, num_atoms, WORDSIZE);
  Array_1d_Create (paritybits_p, num_carbons, BITSIZE);
  Array_2d_Create (&sorted_parity, num_atoms, 2, BYTESIZE);
#endif
  Array_Set (&parityvec_pos, 0);
  Array_Set (paritybits_p, FALSE);
  Array_Set (&sorted_parity, PARITY_INVALID1);

  /* Note the position in the parity vector for each carbon in the molecule */

  for (atom = 0, last_carbon = 0; atom < num_atoms; atom++)
    if (Tsd_Atomid_Get (tsd_tmp, atom) == CARBON)
      {
      Array_1d16_Put (&parityvec_pos, atom, last_carbon);
      last_carbon++;
      }

  /* Sort the parities according to the order in which the carbons appear in
     the Sling.  Also change the parity of one end of a +2 olefin bond to 0.
     This is necessary due to the different ways in which cis and trans
     parities about olefin bonds are represented in the nomenclature algorithm
     and in the Sling.
  */

  for (atom = 0, last_carbon = 0; atom < num_atoms; atom++)
    {
    if (Tsd_Atomid_Get (tsd_tmp, atom) == CARBON)
      {
      Array_2d8_Put (&sorted_parity, Array_1d16_Get (&sling_pos, atom), 0,
        Array_1d8_Get (parityvec_p, last_carbon));
      Array_2d8_Put (&sorted_parity, Array_1d16_Get (&sling_pos, atom), 1,
        Array_1d1_Get (asymmetric_p, last_carbon));
      last_carbon++;

      if (Array_1d8_Get (parityvec_p, atom) == PARITY_TRIODD)
        {
        for (neigh = 0; neigh < MX_NEIGHBORS && Tsd_Atom_NeighborBond_Get (
             tsd_tmp, atom, neigh) != BOND_DOUBLE; neigh++)
          /* Empty loop body */ ;

        Array_1d8_Put (parityvec_p, Array_1d16_Get (&parityvec_pos,
          Tsd_Atom_NeighborId_Get (tsd_tmp, atom, neigh)), PARITY_INVALID2);
        }
      }
    }                         /* End of for-atom loop */

  for (atom = 0, last_carbon = 0; atom < num_atoms; atom++)
    {
    /* The atom has a parity which should be added to the parity bit string */

    if (atom >= num_carbons)
      Array_1d1_Put (asymmetric_p, atom, FALSE);

    if (Array_2d8_Get (&sorted_parity, atom, 0) != PARITY_INVALID1)
      {
      if ((S8_t)Array_2d8_Get (&sorted_parity, atom, 0) <= 0)
        Array_1d1_Put (paritybits_p, last_carbon, TRUE);

      if (Array_2d8_Get (&sorted_parity, atom, 0) == PARITY_DONTCARE)
        Tsd_AtomFlags_DontCare_Put (tsd_p, atom, TRUE);

      last_carbon++;
      }
    }

  *ce_str_p = String_Create (NULL, 3 * num_atoms + 8);
  *se_str_p = String_Create (NULL, 3 * num_atoms + 8);

  SBrothersXlate (cebros_p, &sling_pos, ce_str_p, num_bros);
  if (!String_Compare_c (*ce_str_p, ""))
    String_Concat_c (ce_str_p, NOCEBS);

  SBrothersXlate (sebros_p, &sling_pos, se_str_p, num_bros);
  if (!String_Compare_c (*se_str_p, ""))
    String_Concat_c (se_str_p, NOSEBS);

  for (atom = 0; atom < num_carbons; atom++)
    if (Array_1d8_Get (parityvec_p, atom) == PARITY_UNSURE)
      Array_1d8_Put (parityvec_p, atom, PARITY_TRIODD);

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&in_stack", "sling", &in_stack);
  mind_Array_Destroy ("&parity_ok", "sling", &parity_ok);
  mind_Array_Destroy ("&sling_pos", "sling", &sling_pos);
  mind_Array_Destroy ("&bond_mult", "sling", &bond_mult);
  mind_Array_Destroy ("&parityvec_pos", "sling", &parityvec_pos);
  mind_Array_Destroy ("&sorted_parity", "sling", &sorted_parity);
  mind_Array_Destroy ("&dontcare", "sling", &dontcare);
#else
  Array_Destroy (&in_stack);
  Array_Destroy (&parity_ok);
  Array_Destroy (&sling_pos);
  Array_Destroy (&bond_mult);
  Array_Destroy (&parityvec_pos);
  Array_Destroy (&sorted_parity);
  Array_Destroy (&dontcare);
#endif
  Stack_Destroy (stack_p);
  Tsd_Destroy (tsd_tmp);

  TRACE_DO (DB_TSD2SLINGX, TL_PARAMS,
    {
    IO_Put_Trace (R_TSD, "Output parameters from Tsd2SlingX");
    fprintf (IO_FileHandle_Get (&GTraceFile),
      "Sling (as String) => %s\n", String_Value_Get (output));
    IO_Put_Trace (R_TSD, "Parity bits");
    Array_Dump (paritybits_p, &GTraceFile);
    IO_Put_Trace (R_TSD, "Asymmetric flags");
    Array_Dump (asymmetric_p, &GTraceFile);
    fprintf (IO_FileHandle_Get (&GTraceFile),
      "CE Brother String => %s\n", String_Value_Get (*ce_str_p));
    fprintf (IO_FileHandle_Get (&GTraceFile),
      "SE Brother String => %s\n", String_Value_Get (*se_str_p));
    });

  sling = String2Sling (output);
  String_Destroy (output);

  DEBUG (R_TSD, DB_TSD2SLINGX, TL_PARAMS, (outbuf,
    "Leaving Tsd2SlingX, sling not shown"));

  return sling;
}
