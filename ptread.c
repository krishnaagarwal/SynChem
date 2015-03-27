/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              PTREAD.C
*
*    This module provides for the extraction and storage of posttransform
*    conditions and tests in a human-readable form.
*
*  Creation Date:
*
*       16-Feb-2000
*
*  Authors:
*
*    Jerry Miller
*
*  Modification History, reverse chronological
*
******************************************************************************/

#include <stdio.h>

#include <string.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#include "synchem.h"
#include "debug.h"
#include "synio.h"

/* NEED TO ADD APP_RESRC TO CAPTURE XtManageChild and XtUnmanageChild */
#ifndef _H_APP_RESRC_
#include "app_resrc.h"
#endif

#ifndef _H_RCB_
#include "rcb.h"
#endif

#ifndef _H_LOGIN_
#include "login.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_REACT_FILE_
#include "react_file.h"
#endif


#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_ATOMSYM_
#include "atomsym.h"
#endif

#ifndef _H_EXTERN_
#include "extern.h"
#endif

#define RXNLIB  FCB_SEQDIR_RXNS ("")
#define MAXNODE 128
#define NATTRIB 66

Boolean_t PostIn (char *);
Boolean_t inpost (char *);

void Post_Cond_Mod_Cont ();
void Post_Test_Mod_Cont ();
void Cond_Prep_Mark (Boolean_t);

char *UserId ();

extern Boolean_t glob_rxlform;
Boolean_t cond_marked[100], test_marked[100], curr_is_test, cond_add_canceled, test_add_canceled, current_marked;
int curr_num;

static Widget glob_tl, glob_mgw;
static Condition_t *glob_cond, *glob_cond_root;
static Posttest_t *glob_test, *glob_test_root;
static int glob_condnum, glob_testnum, glob_cond_type, glob_nconds, glob_ntests;
static String_t *glob_reas, *glob_chem;
static char glob_new_cond_type[10];
static Boolean_t initted = FALSE,
                 glob_new_cond = FALSE,
                 glob_new_test = FALSE,
                 array[MAXNODE];
static U32_t     NSch, curr_sch;
static int       cond_map[100][2];
static char     *condtypes[] =
                {"ELEC", "MOLEC", "SBULK", "DIST", "CONN", "RNGSZ", "RNGCP", "ALSMR", "CARSB",
                 "LVGRP", "MGAPT", "ATOM", "XCESS", "CSEQ", "FGEQ", "MORE", "ARNOD",
                 "ARSTD", "ARRAT", "ARCET", "ARTIE", "BRGHD", "MENU"};
static char     *fg[] =
                {"Alcohol",
                 "Carbonyl",
                 "Carbon-Carbon Double Bond",
                 "Ether",
                 "Carboxylic Acid",
                 "Carbon-Carbon Triple Bond",
                 "Carboxylic Ester",
                 "Halogen",
                 "Peroxide",
                 "Carbon-Nitrogen Single Bond",
                 "Carbon-Nitrogen Double Bond",
                 "Carbon-Nitrogen Triple Bond",
                 "Nitrogen-Nitrogen Double or Triple Bond",
                 "Aromatic Nitrogen; Pyridine",
                 "Carboxylic Amide",
                 "Organosulfur",
                 "Nitrogen-Oxygen Bond (Single or Double)",
                 "Pyrylium; Thiapyrylium",
                 "Aryne",
                 "Organomagnesium",
                 "Organolithium",
                 "Organophosphorus",
                 "Pyrazole; Imidazole; Triazole; Tetrazole; N-N Single Bond",
                 "Oxazole; Isoxazole; Thiazole; Isothiazole",
                 "Furan; Pyrrole; Thiophene",
                 "",
                 "Special Hydrocarbons",
                 "Size 3 Carbocycle",
                 "Size 4 Carbocycle",
                 "Size 5 Carbocycle",
                 "Size 6 Carbocycle",
                 "Size 7 Carbocycle",
                 "Size 8 Carbocycle",
                 "Specific Methylenes (Methyl, 1-Ethyl or Activated)",
                 "O-Esters of Nitrogen, Sulfur or Phosphorus Acids",
                 "", "", "",
                 "Carbene & Nitrene",
                 "General Aromatic Ring",
                 "Phenolic Hydroxyl",
                 "Primary Alcohol",
                 "Secondary Alcohol",
                 "Tertiary Alcohol",
                 "Vicinal Diol",
                 "Allylic Alcohol",
                 "Benzylic Alcohol",
                 "Enol",
                 "Beta,Gamma-Unsaturated Carbonyl with Alpha Proton",
                 "Aldehyde",
                 "Ketone",
                 "Carbonyl with Alpha Proton",
                 "Alpha,Beta-Unsaturated Carbonyl",
                 "Alpha Dicarbonyl",
                 "Beta Dicarbonyl with Alpha Proton",
                 "Alpha Ketol",
                 "Beta Ketol with Alpha Proton",
                 "Ketene",
                 "1,5-Dicarbonyl",
                 "Non-Cumulated Alkene",
                 "Cumulated Alkene",
                 "1,3-Diene",
                 "1,4-Diene",
                 "1,3,5-Triene",
                 "Epoxide",
                 "Divinyl Ether/Linear Embedding of Furan",
                 "Allylic Ether",
                 "Vinyl Ether",
                 "Acetal",
                 "Ketal",
                 "Hemiacetal/Ketal",
                 "Terminal Acetylene",
                 "Acetylenic Ether",
                 "Orthoester",
                 "Beta-Keto Ester",
                 "Beta-Keto Acid",
                 "Beta Dicarboxylic Acid",
                 "Simple Carboxylic Ester",
                 "Anhydride",
                 "Carbonate",
                 "Alpha Lactone",
                 "Beta Lactone",
                 "Iminoester",
                 "Beta-Halo Carbonyl with Alpha Proton",
                 "Acyl Halide",
                 "Alpha-Halo Olefin",
                 "Aromatic Halide",
                 "Alpha-Halo Carbonyl",
                 "Allylic Halide",
                 "Propargylic Halide",
                 "Benzylic Halide",
                 "Primary Halide",
                 "Secondary Halide",
                 "Tertiary Halide with Beta Proton",
                 "Haloformate",
                 "Hypohalites & N-Haloamines",
                 "Halohydrin",
                 "Geminal Dihalide",
                 "Trihalomethyl",
                 "Vinyl",
                 "Vicinal Disubstituted Olefin",
                 "Geminal Disubstituted Olefin",
                 "Tri-Substituted Olefin",
                 "Alpha-Halo Ethers & Amines",
                 "Tetra-Substituted Olefin",
                 "Allylic or Benzylic Hydrogens",
                 "Active Dienophile",
                 "N-H Bond",
                 "Nitro(so)alkene",
                 "Benzene Ring",
                 "", "", "",
                 "Aryl Diazonium Salt",
                 "Enamine",
                 "Isocyanate",
                 "Hydrazone",
                 "N-Nitroso",
                 "Diazo",
                 "Nitrile",
                 "Nitro",
                 "Diazoketone",
                 "Primary Amine",
                 "Secondary Amine",
                 "Tertiary Amine",
                 "Oxime",
                 "Primary Amide",
                 "Secondary Amide",
                 "Tertiary Amide",
                 "Quaternary Ammonium Salt",
                 "Allylic Alcohol with Alpha Sulfate",
                 "Dialkyl Sulfate",
                 "",
                 "Sulfoxide",
                 "Thioether",
                 "Thiol",
                 "Sulfonic Acid",
                 "Sulfonic Esters & Sulfonyl Halides",
                 "Sulfone",
                 "Catechol",
                 "Tetrahydropyranyl Ether",
                 "Phenoxy",
                 "Benzylic Ether/Ester",
                 "Benzylic Amine/Amide",
                 "Tosylate",
                 "Tosylamide",
                 "Benzoate",
                 "Benzamide",
                 "Carbobenzyloxy Ester",
                 "Carbobenzyloxy Amide",
                 "Morpholino",
                 "Piperidino",
                 "Pyrrolidino",
                 "2,4-Dinitrophenyl Amine/Amide",
                 "Succinimido",
                 "Phthalimido",
                 "Ethylene Ketal",
                 "Ethylene Glycol Ether/Ester",
                 "1,3 Dithiane",
                 "Ethylene Thioketal",
                 "Trichloroethyl Ether/Ester",
                 "Methoxymethyl Ether/Ester",
                 "Trifluoroacetate",
                 "Trifluoroacetamide",
                 "Trichloroacetate",
                 "Trichloroacetamide",
                 "Tert-Butoxycarbonyl Ester",
                 "Tert-Butoxycarbonyl Amide",
                 "Trifluoromethanesulfonate",
                 "Trifluoromethanesulfonamide",
                 "Succinic Ester/Acid",
                 "Succinic Amide",
                 "Glutaric Ester/Acid",
                 "Oxime Methyl Ether",
                 "", "",
                 "Benzyl Thioether/Thioester",
                 "Para-Nitrobenzyl Ether/Ester",
                 "Para-Nitrobenzyl Amine/Amide",
                 "Para-Nitrobenzyl Thioether/Thioester",
                 "O-Phenyl",
                 "N-Phenyl",
                 "S-Phenyl",
                 "Para-Nitrophenyl Ether/Ester",
                 "Para-Nitrophenyl Amine/Amide",
                 "Para-Nitrophenyl Thioether/Thioester",
                 "Glutaric Amide",
                 "Adipic Ester/Acid",
                 "N-Pyridyl",
                 "Trimethylsilyl Ether",
                 "Trimethylsilyl Amine",
                 "2,4,6-Trinitrophenyl Ether/Ester",
                 "Glycyl Ester/Acid",
                 "Glycyl Amide",
                 "Alanyl Ester/Acid",
                 "Alanyl Amide",
                 "Valyl Ester/Acid",
                 "Valyl Amide",
                 "Leucyl Ester/Acid",
                 "Leucyl Amide",
                 "Isoleucyl Ester/Acid",
                 "Isoleucyl Amide",
                 "Phenylalanyl Ester/Acid",
                 "Phenylalanyl Amide",
                 "Tyrosyl Ester/Acid",
                 "Tyrosyl Amide",
                 "Seryl Ester/Acid",
                 "Seryl Amide",
                 "Threonyl Ester/Acid",
                 "Threonyl Amide",
                 "Cysteyl Ester/Acid",
                 "Cysteyl Amide",
                 "Methionyl Ester/Acid",
                 "Methionyl Amide",
                 "Ornithyl Ester/Acid",
                 "Ornithyl Amide",
                 "Arginyl Ester/Acid",
                 "Arginyl Amide",
                 "Lysyl Ester/Acid",
                 "Lysyl Amide",
                 "Aspartyl Ester/Acid",
                 "Aspartyl Amide",
                 "Asparagyl Ester/Acid",
                 "Asparagyl Amide",
                 "Glutamyl Ester/Acid",
                 "Glutamyl Amide",
                 "Glutamyl Ester/Acid",
                 "Glutamyl Amide",
                 "Prolyl Ester/Acid",
                 "Prolyl Amide",
                 "Hydroxyprolyl Ester/Acid",
                 "Hydroxyprolyl Amide",
                 "Tryptophanyl Ester/Acid",
                 "Tryptophanyl Amide",
                 "Histidyl Ester/Acid",
                 "Histidyl Amide",
                 "Glucosyl Ether/Ester",
                 "Fructosyl Ether/Ester",
                 "Ribosyl Ether/Ester",
                 "Adipic Amide",
                 "Pimelic Ester/Acid",
                 "Pimelic Amide",
                 "Suberic Ester/Acid",
                 "Suberic Amide",
                 "N-Imidazoyl",
                 "Alpha-Fluoro Alcohol",
                 "Alpha-Chloro Alcohol",
                 "Alpha-Bromo Alcohol",
                 "Alpha-Iodo Alcohol",
                 "Geminal Diol",
                 "Aminol",
                 "Alpha-Hydroxy Epoxide",
                 "Alpha-Acyloxy Alcohol",
                 "Alpha-Sulfonoxy Alcohol",
                 "Alpha-Nitroxy Alcohol",
                 "Alpha-Hydroxy Thioether",
                 "Alpha-Fluoro Amine",
                 "Alpha-Chloro Amine",
                 "Alpha-Bromo Amine",
                 "Alpha-Iodo Amine",
                 "",
                 "Alpha-Hydroxy Aziridine",
                 "Alpha-Amino Epoxide",
                 "Alpha-Acyloxy Amine",
                 "Alpha-Sulfonoxy Amine",
                 "Alpha-Nitroxy Amine",
                 "Alpha-Amino Aziridine",
                 "Alpha-Fluoro Hydrosulfide",
                 "Alpha-Chloro Hydrosulfide",
                 "Alpha-Bromo Hydrosulfide",
                 "Alpha-Iodo Hydrosulfide",
                 "Alpha-Hydroxy Hydrosulfide",
                 "Alpha-Mercapto Epoxide",
                 "Alpha-Acyloxy Hydrosulfide",
                 "Alpha-Sulfonoxy Hydrosulfide",
                 "Para-Nitrobenzenesulfonate",
                 "Para-Nitrobenzenesulfonamide",
                 "Acetate",
                 "Acetamide",
                 "O-Methyl",
                 "N-Methyl",
                 "O-Ethyl",
                 "N-Ethyl",
                 "O-Isopropyl",
                 "N-Isopropyl",
                 "O-Propyl",
                 "N-Propyl",
                 "O-N-Butyl",
                 "N-N-Butyl",
                 "O-Sec-Butyl",
                 "N-Sec-Butyl",
                 "O-Iso-Butyl",
                 "N-Iso-Butyl",
                 "O-Tert-Butyl",
                 "N-Tert-Butyl",
                 "Piperonyl",
                 "Veratryl",
                 "6-Chloropiperonyl",
                 "Methyl",
                 "Ethyl",
                 "Disulfide",
                 "Tri- or Polysulfide",
                 "Thioacid",
                 "Thiolester",
                 "Thionester",
                 "Thioanhydride",
                 "Dithioacid",
                 "Dithioester",
                 "Thioamide",
                 "Thiocyanate",
                 "Isothiocyanate",
                 "Enethiol",
                 "Gamma Thiolactone",
                 "Delta Thiolactone",
                 "Episulfide",
                 "Mercaptal/Mercaptole",
                 "Linear Embedding of Thiophene",
                 "Sulfenyl Halide",
                 "Sulfinyl Halide",
                 "Sulfenamide",
                 "Sulfinamide",
                 "Sulfenic Acid/Ester",
                 "Sulfinic Acid/Ester",
                 "Linear Embedding of Thiazole",
                 "Linear Embedding of Isothiazole",
                 "Thianaphthene",
                 "Sulfonium Salt",
                 "Alkylidenesulfurane",
                 "Thione",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "Aziridine",
                 "Divinylamine/Linear Embedding of Pyrrole",
                 "Imide",
                 "Amidine",
                 "Linear Embedding of Imidazole",
                 "Diaziridine",
                 "Aminal",
                 "Beta Lactam",
                 "Gamma Lactam",
                 "Delta Lactam",
                 "Indole",
                 "Quinoline",
                 "Isoquinoline",
                 "Isonitrile",
                 "Carbodiimide",
                 "Amidol",
                 "Enamide",
                 "Nitrate Ester",
                 "Nitrite Ester",
                 "Alpha-Acylamido Acid with Alpha & Beta Proton",
                 "Alpha-Nitro Alcohol",
                 "Arylhydroxylamine",
                 "Arylhydroxylamine Sulfate",
                 "Nitryloxy",
                 "Alpha-Amino Acid",
                 "Alpha-Amino Sulfonic Acid",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "Linear Embedding of Oxazole",
                 "Linear Embedding of Isoxazole",
                 "Benzofuran",
                 "Chroman",
                 "Coumarin",
                 "Cyanate",
                 "Methylenedioxybenzene",
                 "Aryloxy",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "Phosphine",
                 "Phosphonium Salt",
                 "Alkylidenephosphorane",
                 "Phosphate Ester",
                 "Phosphite Ester",
                 "Phosphonate Ester",
                 "Phosphinate Ester",
                 "Phosphorus-Sulfur Double Bond",
                 "Phosphorus-Oxygen Double Bond",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "Selenol",
                 "Gamma Selenolactone",
                 "Delta Selenolactone",
                 "Selenophene",
                 "Selenide",
                 "Diselenide",
                 "Organolead",
                 "Organomercury",
                 "Organothallium",
                 "Organocadmium",
                 "Organozinc",
                 "", "", "", "",
                 "Silane",
                 "Silyl Halide",
                 "Silyl Hydride",
                 "Siloxane",
                 "Silazane",
                 "", "", "", "", "",
                 "Linear Embedding of Pyrazole",
                 "Methylenelactone",
                 "Glucuronide",
                 "Hydrogen Sulfate Ester",
                 "Glutathion Conjugate",
                 "Hippuric Acid",
                 "Gamma Carboxyketone with Alpha Proton",
                 "Alpha,Beta-Unsaturated Aldehyde/Ketone with Gamma Proton",
                 "Alpha Ketol with Beta Proton",
                 "Alpha Ketoacid",
                 "Enol Sulfate of Alpha,Beta-Unsaturated Carbonyl",
                 "Enol Sulfate of Aldehyde/Ketone",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "Electronically Unstable Cyclopentadienone",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "Ethylxanthate",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "Aliphatic Aldehyde",
                 "Aromatic Aldehyde",
                 "Intramolecular Hydrogen Bond",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "Anthracenoid Hydrocarbon",
                 "Phenanthrenoid Hydrocarbon",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "Beta-Hydroxyalkyl Sulfate",
                 "Beta-Hydroxyalkyl Nitrate",
                 "Acidic Hydroxyl Delta to Oxirane",
                 "Allylic Sulfate with Delta Proton",
                 "", "", "", "", "", "", "",
                 "Hydroxylamine with 0-Substituent",
                 "Hydroxylamine with N-Mono-Substituent",
                 "Hydroxylamine with N-Di-Substituent",
                 "Hydroxylamine with N,N,O-Substituents",
                 "Organofluorides",
                 "Organochlorides",
                 "Organobromides",
                 "Organoiodides",
                 "Polyaromatic Hydrocarbons (Pah)",
                 "Hydrogen Sulfate or Sulfonate",
                 "Hydrogen Phosphate or Phosphonate",
                 "Pyridine Ring",
                 "Sulfamate; Sulfonamide",
                 "Phosphamate; Phosphonamide",
                 "Vinylic Chloride & Bromide",
                 "Terminal-Methylene Michael Receptors",
                 "", "", "",
                 "Tautomerizable Cyclohexadienone Derivative",
                 "Spontaneously Aromatizing Cyclohexadiene",
                 "Tautomerizable Alkylidenecyclohexadiene",
                 "", "", "", "", "", "", "",
                 "Imine with alpha proton",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "Tetraarylcyclopentadienone",
                 "Pyridinium; Quinolinium",
                 "Quinolizinium",
                 "", "", "", "", "", "", "", "",
                 "Acetyl",
                 "Methyl Ketone",
                 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                 "Alkylarylamine",
                 "Dialkylarylamine",
                 "Alkyldiarylamine",
                 "Vinylogous amide",
                 "Vinylogous ester",
                 "", "",
                 "2-Halopyridine/pyrazine/pyrimidine",
                 "4-Halopyridine",
                 "Arylamino",
                 "Urea",
                 "Monoarylamine",
                 "Diarylamine",
                 "Triarylamine",
                 "Monoalkylamine",
                 "Dialkylamine",
                 "Trialkylamine",
                 "",
                 "Dithiocarbamate",
                 "Urethane"};

/********************* INITLIB *******************************/

void initlib ()
{
  if (initted) return;
  React_Init ((U8_t *) RXNLIB);
  NSch = React_NumSchemas_Get ();
  initted = TRUE;
}

/********************* CLR_ARRAY *****************************/

void clr_array ()
{
  int i;

  for (i = 0; i < MAXNODE; i++)
    array[i] = FALSE;
}

/********************* PTRPUTMSG *****************************/

static void PTRputmsg (char *msg)
{
  printf ("%s\n", msg);
}

/********************* VERIFY ********************************/

int verify (char *string, char *validstr)
{
  int i, j, ok;

  for (i = 0; i < strlen (string); i++)
    {
    for (j = ok = 0; j < strlen (validstr) && !ok; j++)
      {
      if (string[i] == validstr[j]) ok = 1;
      }
    if (!ok) return (i + 1);
    }
  return (0);
}

/********************* RATEXP ********************************/

char *ratexp (int n)
{
  static char  retval[50],
              *activ[] =
               {"very strongly deactivated",
                "strongly deactivated",
                "moderately deactivated",
                "weakly deactivated",
                "same as benzene",
                "weakly activated",
                "moderately activated",
                "strongly activated",
                "very strongly activated"};
  static int  actrat[] = {120, 90, 60, 30, 0, -30, -60, -90, -120};
  int i;

  for (i = 0; i < 9; i++)
    if (n > actrat[i] || i == 8 || 2 * n > actrat[i] + actrat[i + 1])
    {
    sprintf (retval, "%d (%s)", i + 1, activ[i]);
    return (retval);
    }
}

/********************* ELECPR ********************************/

void elecpr (char *condstr)
{
  int strptr;
  int numnodes, node, i;

  sscanf (condstr, "%2d", &numnodes);
  strptr = 2;
  for (i = 0; i < numnodes; i++)
    {
    sscanf (condstr + strptr, "%2d", &node);
    array[node - 1] = TRUE;
    strptr += 2;
    }
  sscanf (condstr + strptr + 2, "%2d", &numnodes);
  strptr += 4;
  if (numnodes != 0) for (i=0; i < numnodes; i++)
    {
    sscanf (condstr + strptr, "%2d", &node);
    array[node - 1] = TRUE;
    strptr += 2;
    }
}

/********************* BULKPR ********************************/

void bulkpr (char *condstr)
{
  int node1, node2;
  char msg[200];

  if (strlen (condstr) != 10)
    {
        sprintf (msg, "INVALID CONDITION FIELD %s", condstr);
        PTRputmsg (msg);
        return;
    }
  sscanf (condstr, "%2d%2d", &node1, &node2);
  if (node1 == 0)
    {
        sprintf (msg, "INVALID CONDITION FIELD %s", condstr);
        PTRputmsg (msg);
        return;
    }
  array[node1 - 1] = TRUE;
  if (node2 != 0) array[node2 - 1] = TRUE;
  sscanf (condstr + 6, "%2d%2d", &node1, &node2);
  if (node1 == 0) return;
  array[node1 - 1] = TRUE;
  if (node2 != 0) array[node2 - 1] = TRUE;
}

/********************* DISTPR ********************************/

void distpr (char *condstr)
{
  int node;
  char msg[200];

  if (strlen (condstr) < 7)
    {
    sprintf (msg, "INVALID CONDITION FIELD %s", condstr);
    PTRputmsg (msg);
    return;
    }
  sscanf (condstr, "%2d", &node);
  array[node - 1] = TRUE;
}

/********************* RINGPR ********************************/

void ringpr (char *condstr)
{
  int node1, node2, node3, node4;
  char msg[200];

  if (strlen (condstr) != 10)
    {
    sprintf (msg, "INVALID CONDITION FIELD %s", condstr);
    PTRputmsg (msg);
    return;
    }
  sscanf (condstr, "%2d%2d", &node1, &node2);
  sscanf (condstr + 6, "%2d%2d", &node3, &node4);
  array[node1 - 1] = TRUE;
  array[node2 - 1] = TRUE;
  if (node3 != 0)
    {
    array[node3 - 1] = TRUE;
    array[node4 - 1] = TRUE;
    }
}

/********************* ALKPR *********************************/

void alkpr (char *condstr)
{
  int node1, node2;
  char msg[200];

  if (strlen (condstr) != 4)
    {
    sprintf (msg, "INVALID CONDITION FIELD %s", condstr);
    PTRputmsg (msg);
    return;
    }
  sscanf (condstr, "%2d%2d", &node1, &node2);
  array[node1 - 1] = TRUE;
  array[node2 - 1] = TRUE;
}

/********************* ALLPR *********************************/

void allpr (char *condstr)
{
  int node1, node2;
  char msg[200];

  if (strlen (condstr) != 4)
    {
    sprintf (msg, "INVALID CONDITION FIELD %s", condstr);
    PTRputmsg (msg);
    return;
    }
  sscanf (condstr, "%2d%2d", &node1, &node2);
  array[node1 - 1] = TRUE;
  array[node2 - 1] = TRUE;
}

/********************* RINGPR ********************************/

void stabpr (char *condstr)
{
  int node1, node2, node3, node4;
  char msg[200];

  if (strlen (condstr) != 10)
    {
    sprintf (msg, "INVALID CONDITION FIELD %s", condstr);
    PTRputmsg (msg);
    return;
    }
  sscanf (condstr, "%2d%2d", &node1, &node2);
  sscanf (condstr + 6, "%2d%2d", &node3, &node4);
  array[node1 - 1] = TRUE;
  array[node2 - 1] = TRUE;
  array[node3 - 1] = TRUE;
  array[node4 - 1] = TRUE;
}

/********************* LVGRPR ********************************/

void lvgrpr (char *condstr)
{
  int node1, node2;
  char msg[200];

  if (strlen (condstr) != 12)
    {
    sprintf (msg, "INVALID CONDITION FIELD %s", condstr);
    PTRputmsg (msg);
    return;
    }
  sscanf (condstr, "%2d", &node1);
  sscanf (condstr + 7, "%2d", &node2);
  array[node1 - 1] = TRUE;
  if (node2 != 0) array[node2 - 1] = TRUE;
}

/********************* MGAPTPR *******************************/

void mgaptpr (char *condstr)
{
  int node1, node2;
  char msg[200];

  if (strlen (condstr) != 7)
    {
    sprintf (msg, "INVALID CONDITION FIELD %s", condstr);
    PTRputmsg (msg);
    return;
    }
  sscanf (condstr, "%2d", &node1);
  sscanf (condstr + 4, "%2d", &node2);
  array[node1 - 1] = TRUE;
  if (node2 == 0) array[node2 - 1] = TRUE;
}

/********************* ATOMPR ********************************/

void atompr (char *condstr)
{
  int node;
  char msg[200];

  if (strlen (condstr) != 5 && strlen (condstr) != 7)
    {
    sprintf (msg, "INVALID CONDITION FIELD %s", condstr);
    PTRputmsg (msg);
    return;
    }
  sscanf (condstr, "%2d", &node);
  array[node - 1] = TRUE;
}

/********************* XCESSPR *******************************/

void excesspr (char *condstr)
{
  int node;

  sscanf (condstr + 3, "%2d", &node);
  array[node - 1] = TRUE;
}

/********************* IDENTPR *******************************/

void identpr (char *condstr, int constonly)
{
  int i, numnod, node;
  Boolean_t ok;
  char msg[200];

  i = strlen (condstr);
  ok = i > 5 && verify (condstr, "1234567890") == 0;
  if (ok)
    {
    sscanf (condstr, "%2d", &numnod);
    if (numnod > 50) numnod -= 50;
    ok = numnod > 1 && i == 2 * numnod + 2;
    }
  if (!ok)
    {
    sprintf (msg, "INVALID CONDITION FIELD %s", condstr);
    PTRputmsg (msg);
    return;
    }
  for (i = 0; i < numnod; i++)
    {
    sscanf (condstr + 2 * i, "%2d", &node);
    array[node - 1] = TRUE;
    }
}

/********************* INSTCEPR ******************************/

void instcepr (char *condstr, int constonly)
{
  int i, numnod;
  Boolean_t ok;
  char msg[200];

  i = strlen (condstr);
  ok = i == 4 && verify (condstr, "1234567890") == 0 && strcmp (condstr, "2000") < 0;
  if (!ok)
    {
    sprintf (msg, "INVALID CONDITION FIELD %s", condstr);
    PTRputmsg (msg);
    return;
    }
}

/********************* DISCNPR *******************************/

void discnpr (char *condstr, int constonly)
{
  int i, numnod, node;
  Boolean_t ok;
  char msg[200];

  i = strlen (condstr);
  ok = i > 5 && verify (condstr, "1234567890") == 0;
  if (ok)
    {
    sscanf (condstr, "%2d", &numnod);
    ok = numnod > 1 && i == 2 * numnod + 2;
    }
  if (!ok)
    {
    sprintf (msg, "INVALID CONDITION FIELD %s", condstr);
    PTRputmsg (msg);
    return;
    }
  for (i=0; i<numnod; i++)
    {
    sscanf (condstr + 2 * i, "%2d", &node);
    array[node] = TRUE;
    }
}

/********************* MORE_INSTS_PR *************************/

void more_insts_pr (char *condstr)
{
  Boolean_t ok;
  char msg[200], temp[4];

  strncpy (temp, condstr, 3);
  temp[3] = 0;
  ok = strlen (condstr) == 4 && verify (temp, "1234567890") == 0;
  if (!ok)
    {
    sprintf (msg, "INVALID CONDITION FIELD %s", condstr);
    PTRputmsg (msg);
    }
}

/********************* ARSUB_PR ******************************/

void arsub_pr (char *condstr)
{
  int type, sgnode;
  char msg[200];

  type = condstr[0] - '0';
  sscanf (condstr + 2, "%2d", &sgnode);
  sprintf (msg, "INVALID CONDITION FIELD %s", condstr);
  switch (type)
    {
  case 1:
    if (strlen (condstr) != 4)
      {
      PTRputmsg (msg);
      return;
      }
    array[sgnode - 1] = TRUE;
    break;
  case 2:
    if (strlen (condstr) != 7 || verify (condstr, "1234567890~= <>") != 0)
      {
      PTRputmsg (msg);
      return;
      }
    array[sgnode - 1] = TRUE;
    break;
  case 3:
    if (strlen (condstr) != 10 || verify (condstr, "1234567890~= <>+-") != 0)
      {
      PTRputmsg (msg);
      return;
      }
    array[sgnode - 1] = TRUE;
    break;
  case 4:
    if (strlen (condstr) != 4 || verify (condstr, "1234567890~= <>+-") != 0)
      {
      PTRputmsg (msg);
      return;
      }
    array[sgnode - 1] = TRUE;
    break;
  case 5:
    if (strlen (condstr) != 7 || verify (condstr, "1234567890~= <>+-") != 0)
      {
      PTRputmsg (msg);
      return;
      }
    array[sgnode - 1] = TRUE;
    break;
  default:
    PTRputmsg (msg);
    return;
    }
}

/********************* BHEADPR *******************************/

void bheadpr (char *condstr)
{
  int node;

  sscanf (condstr, "%2d", &node);
  array[node - 1] = TRUE;
}

/********************* MOLECPR *******************************/

void molecpr (char *condstr)
{
  char msg[200];

  if (strlen (condstr) != 1)
    {
    sprintf (msg, "INVALID CONDITION FIELD %s", condstr);
    PTRputmsg (msg);
    }
}

/********************* ELEC **********************************/

char *elec (char *c)
{
  static char v[1024];
  int i, n, nn;

  strcpy (v, "\t\tThe sum of the electron-withdrawing effects of the substituents rooted at node(s)\n\t\t");
  sscanf (c + 3, "%2d", &n);
  for (i=0; i<n; i++)
    {
    strcat (v, "\t");
    strncat (v, c + 5 + 2 * i, 2);
    }
  strcat (v, "\n\t\tin the goal pattern IS ");
  if (c[0] == '0') strcat (v, "NOT ");
  strncat (v, c + 5 + 2 * n, 2);
  sscanf (c + 7 + 2 * n, "%2d", &nn);
  if (nn == 0)
    {
    strcat (v, "\n\t\t\tTHE CONSTANT ");
    strncat (v, c + 9 + 2 * n, 2);
    }
  else
    {
    strcat (v, "\n\t\tTHE SUM OF THE ELECTRON-WITHDRAWING EFFECTS OF THE SUBSTITUENTS ROOTED AT NODE(S)\n\t\t");
    for (i = 0; i < nn; i++)
      {
      strcat (v, "\t");
      strncat (v, c + 9 + 2 * n + 2 * i, 2);
      }
    }
  strcat (v, "\n\t\t(where positive numbers indicate electron-withdrawing groups).");

  return (v);
}

/********************* MOLEC *********************************/

char *molec (char *c)
{
  static char v[1024];

  strcpy (v, "\t\tThe number of reacting molecules IS ");
  if (c[0] == '0') strcat (v, "NOT ");
  strcat (v, c + 3);
  strcat (v, ".");

  return (v);
}

/********************* SBULK *********************************/

char *sbulk (char *c)
{
  static char v[1024], *gname[] = {"METHYL", "ETHYL", "ISOPROPYL", "t-BUTYL"};
  int n, nn;

  strcpy (v, "\t\tThe bulk of the substituent defined by\n\t\t\t");
  if (!strncmp (c + 5, "00", 2))
    {
    strcat (v, "NODE ");
    strncat (v, c + 3, 2);
    }
  else
    {
    strcat (v, "THE EDGE BETWEEN NODES ");
    strncat (v, c + 3, 2);
    strcat (v, " AND ");
    strncat (v, c + 5, 2);
    }
  strcat (v, "\n\t\tin the goal pattern IS ");
  if (c[0] == '0') strcat (v, "NOT ");
  strncat (v, c + 7, 2);
  strcat (v, "\n\t\tthe bulk of the substituent\n\t\t\t");
  sscanf (c + 9, "%2d%2d", &n, &nn);
  if (n == 0)
    {
    strcat (v, "GROUP ");
    strcat (v, gname[nn - 1]);
    }
  else if (nn == 0)
    {
    strcat (v, "NODE ");
    strncat (v, c + 9, 2);
    }
  else
    {
    strcat (v, "defined by THE EDGE BETWEEN NODES ");
    strncat (v, c + 9, 2);
    strcat (v, " AND ");
    strncat (v, c + 11, 2);
    }
  strcat (v, ".");

  return (v);
}

/********************* DIST **********************************/

char *dist (char *c)
{
  static char  v[1024],
              *attr[NATTRIB] =
                {"CARBONYL",
                 "ENOLIZABLE CARBONYL",
                 "ACIDIC NITRITE",
                 "AROMATIC RING",
                 "ALKENE",
                 "ALKYNE",
                 "NITRILE",
                 "NITRO",
                 "SULFONE",
                 "HYDROGEN",
                 "ESTER (ACYL)",
                 "TERTIARY AMIDE",
                 "ALPHA,BETA UNSATURATED CARBONYL",
                 "ESTER (ALKYL)",
                 "O ESTER",
                 "FLUORINE",
                 "CHLORINE",
                 "BROMINE",
                 "IODINE",
                 "SULFUR",
                 "HYDROXYL",
                 "ETHER",
                 "PRIMARY AMINE",
                 "SECONDARY AMINE",
                 "TERTIARY AMINE",
                 "CARBOXYLIC ACID",
                 "METHYL",
                 "METHYLENE",
                 "METHINYL",
                 "QUATERNARY AMINE",
                 "SULFONIUM",
                 "SULFONATE ESTER",
                 "PHENOXY",
                 "ALKOXY",
                 "NITRATE",
                 "PHOSPHATE",
                 "BORATE",
                 "VINYL WITH GAMMA HYDROGEN",
                 "SULFOXIDE",
                 "PHOSPHOROUS OXIDES",
                 "THIOPHENOL",
                 "MERCAPTAN",
                 "SULFIDE",
                 "PHOSPHINE",
                 "PHOSPHONATE ESTER AT P",
                 "TRIALKYLSILYL",
                 "ALPHA-PYRIDYL",
                 "BETA-PYRIDYL",
                 "GAMMA-PYRIDYL",
                 "ALPHA-FURYL",
                 "ALPHA-THIENYL",
                 "ALPHA-PYRRYL",
                 "BETA-FURYL, ALPHA,ALPHA-DISUBSTITUTED",
                 "BETA-THIENYL, ALPHA,ALPHA-DISUBSTITUTED",
                 "BETA-PYRRYL, ALPHA,ALPHA-DISUBSTITUTED",
                 "PARA-HYDROXYPHENYL",
                 "PARA-AMINOPHENYL",
                 "ORTHO-HYDROXYPHENYL",
                 "ORTHO-AMINOPHENYL",
                 "TRIHALOMETHYL",
                 "QUATERNARY CARBON",
                 "OXIRANE",
                 "1-PYRRYL",
                 "1-IMIDAZOLYL",
                 "1-PYRAZOLYL",
                 "METHYL OR METHYLENE"};
  int n;

  strcpy (v, "\t\tThe directed attribute\n\t\t\t");
  sscanf (c + 5, "%3d", &n);
  if (n == 0)
    {
    strcat (v, "SUBSTITUENT WITH SLING \"");
    strcat (v, c + 12);
    strcat (v, "\" AND ROOT ");
    strncat (v, c + 10, 2);
    }
  else
    strcat (v, attr[n - 1]);
  strcat (v, "\n\t\tIS ");
  if (c[0] == '0') strcat (v, "NOT ");
  strcat (v, "present at distance\n\t\t\t");
  strncat (v, c + 8, 2);
  strcat (v, " from NODE ");
  strncat (v, c + 3, 2);
  strcat (v, ".");

  return (v);
}

/********************* CONN **********************************/

char *conn (char *c)
{
  static char v[1024];

  strcpy (v, "\t\tThe path (outside of the matched part of the target molecule) between nodes\n\t\t\t");
  strncat (v, c + 3, 2);
  strcat (v, " AND ");
  strncat (v, c + 5, 2);
  if (!strncmp (c + 7, "++", 2))
    {
    if (c[0] == '0')
      strcat (v, "\n\t\t\tDOES NOT EXIST");
    else
      strcat (v, "\n\t\t\tEXISTS");
    }
  else
    {
    strcat (v, "\n\t\t\tIS ");
    if (c[0] == '0') strcat (v, "NOT ");
    strncat (v, c + 7, 2);
    if (!strncmp (c + 9, "00", 2))
      {
      strcat (v, "\n\t\ta total of\n\t\t\t");
      strncat (v, c + 11, 2);
      strcat (v, " BOND LENGTHS");
      }
    else
      {
      strcat (v, "\n\t\tthe path connecting nodes\n\t\t\t");
      strncat (v, c + 9, 2);
      strcat (v, " AND ");
      strncat (v, c + 11, 2);
      }
    }
  strcat (v, ".");

  return (v);
}

/********************* ALSMR *********************************/

char *alsmr (char *c)
{
  static char v[1024];

  if (!strncmp (c + 1, "06", 2))
    strcpy (v, "\t\tThe ALKYNE defined by nodes\n\t\t\t");
  else
    strcpy (v, "\t\tThe ALLENE defined by nodes\n\t\t\t");
  strncat (v, c + 3, 2);
  strcat (v, " AND ");
  strncat (v, c + 5, 2);
  strcat (v, "\n\t\tIS ");
  if (c[0] == '0') strcat (v, "NOT ");
  strcat (v, "in a small ring.");

  return (v);
}

/********************* CARSB *********************************/

char *carsb (char *c)
{
  static char v[1024];

  strcpy (v, "\t\tThe stability of the carbonium ion defined by nodes\n\t\t\t");
  strncat (v, c + 3, 2);
  strcat (v, " AND ");
  strncat (v, c + 5, 2);
  strcat (v, "\n\t\t\tIS ");
  if (c[0] == '0') strcat (v, "NOT ");
  strncat (v, c + 7, 2);
  strcat (v, "\n\t\tthe stability of the carbonium ion defined by nodes\n\t\t\t");
  strncat (v, c + 9, 2);
  strcat (v, " AND ");
  strncat (v, c + 11, 2);
  strcat (v, ".");

  return (v);
}

/********************* LVGRP *********************************/

char *lvgrp (char *c)
{
  static char  v[1024],
              *cond[] = {"ACIDIC", "NEUTRAL", "BASIC"};
  int n;

  strcpy (v, "\t\tUnder\n\t\t\t");
  sscanf (c + 7, "%1d", &n);
  strcat (v, cond[n - 1]);
  strcat (v, "\n\t\tconditions there ");
  if (c[0] == '1')
    strcat (v, "EXISTS ");
  else
    strcat (v, "DOES NOT EXIST ");
  strcat (v, "a leaving group at distance\n\t\t\t");
  strncat (v, c + 5, 2);
  strcat (v, "\n\t\tin the part of the target molecule defined by node\n\t\t\t");
  strncat (v, c + 3, 2);
  strcat (v, "\n\t\tin the goal pattern whose leaving ability is ");
  strncat (v, c + 8, 2);
  strcat (v, "\n\t\t\t");
  if (!strncmp (c + 10, "00", 2))
    {
    strcat (v, "THE VALUE (0=VERY POOR; 10=VERY GOOD): ");
    strncat (v, c + 12, 2);
    }
  else
    {
    strcat (v, "THAT OF THE LEAVING GROUP AT\n\t\t\tDISTANCE ");
    strncat (v, c + 12, 2);
    strcat (v, "\n\t\t\tIN THE PART OF THE TARGET MOLECULE DEFINED BY NODE ");
    strncat (v, c + 10, 2);
    strcat (v, "\n\t\t\tUNDER ");
    sscanf (c + 14, "%1d", &n);
    strcat (v, cond[n - 1]);
    strcat (v, " CONDITIONS");
    }
  strcat (v, ".");

  return (v);
}

/********************* MGAPT *********************************/

char *mgapt (char *c)
{
  static char  v[1024],
              *gname[] = {"A NON-MIGRATORY GROUP", "AN ALKYL GROUP", "AN ARYL GROUP", "A PROTON"};
  int n;

  strcpy (v, "\t\tThe group in the target molecule defined by node\n\t\t\t");
  strncat (v, c + 3, 2);
  strcat (v, "\n\t\thas a migratory aptitude that IS ");
  if (c[0] == '0') strcat (v, "NOT ");
  strncat (v, c + 5, 2);
  strcat (v, "\n\t\t\tTHAT OF ");
  sscanf (c + 7, "%2d", &n);
  if (n == 0)
    {
    sscanf (c + 9, "%d", &n);
    strcat (v, gname[n]);
    }
  else
    {
    strcat (v, "THE GROUP DEFINED BY\n\t\t\tNODE ");
    strncat (v, c + 7, 2);
    }
  strcat (v, ".");

  return (v);
}

/********************* ATOM **********************************/

char *atom (char *c)
{
  static char  v[1024];
  int n;

  strcpy (v, "\t\tThe atom at distance\n\t\t\t");
  if (strlen (c) == 8)
    strcat (v, "00");
  else
    strncat (v, c + 8, 2);
  strcat (v, "\n\t\tfrom node\n\t\t\t");
  strncat (v, c + 3, 2);
  strcat (v, "\n\t\t\tIS ");
  if (c[0] == '0')
    strcat (v, "NOT ");
  sscanf (c + 5, "%3d", &n);
  strcat (v, Atomid2Symbol (n));
  strcat (v, ".");

  return (v);
}

/********************* XCESS *********************************/

char *xcess (char *c)
{
  static char  v[1024];
  int n;

  strcpy (v, "\t\tThe functional group\n\t\t\t");
  sscanf (c + 3, "%3d", &n);
  strcat (v, fg[n - 1]);
  strcat (v, "\n\t\t\t");
  if (!strncmp (c + 8, "00", 2))
    {
    strcat (v, "IS ");
    if (c[0] == '0') strcat (v, "NOT ");
    strcat (v, "PRESENT IN THE PART OF THE TARGET MOLECULE DEFINED BY\n\t\t\tNODE ");
    strncat (v, c + 6, 2);
    }
  else
    {
    if (c[0] == '0') strcat (v, "DOES NOT OCCUR AT LEAST ");
    else strcat (v, "OCCURS AT LEAST ");
    strncat (v, c + 8, 2);
    strcat (v, "  TIMES WITHIN THE PART OF THE TARGET MOLECULE DEFINED BY\n\t\t\tNODE ");
    strncat (v, c + 6, 2);
    }
  strcat (v, ".");

  return (v);
}

/********************* CSEQ **********************************/

char *cseq (char *c)
{
  static char v[1024];
  int i, n, nn;
  Boolean_t discbit, stereo, goalbit;

  strcpy (v, "\t\tThe nodes\n\t\t");
  sscanf (c + 1, "%2d%2d", &n, &nn);
  discbit = n > 16;
  stereo = n % 2 == 0;
  goalbit = nn > 50 || discbit;
  if (goalbit && !discbit) nn -= 50;

  for (i = 0; i < nn; i++)
    {
    strcat (v, "\t");
    strncat (v, c + 5 + 2 * i, 2);
    }
  strcat (v, "\n\t\t\tARE ");
  if (c[0] == '0') strcat (v, "NOT ");
  if (stereo) strcat (v, "STEREOCHEMICALLY");
  else strcat (v, "CONSTITUTIONALLY");
  strcat (v, " EQUIVALENT\n\t\t\t");
  if (discbit) strcat (v, "WHEN DISCONNECTED FROM THE GOAL PATTERN.");
  else if (goalbit) strcat (v, "IN THE GOAL PATTERN.");
  else strcat (v, "IN THE SUBGOAL PATTERN.");

  return (v);
}

/********************* FGEQ **********************************/

char *fgeq (char *c)
{
  static char v[1024];
  int n, nn, gb;
  Boolean_t stereo, goalbit;

  sscanf (c + 1, "%2d%1d%3d", &n, &gb, &nn);
  goalbit = gb != 0;
  stereo = n % 2 == 0;
  strcpy (v, "\t\tEvery instance of the functional group\n\t\t");
  strcat (v, fg[nn - 1]);
  strcat (v, "\n\t\t\t");

  if (goalbit) strcat (v, "IN THE GOAL PATTERN");
  else strcat (v, "IN THE SUBGOAL PATTERN");

  strcat (v, "\n\t\t\tIS ");
  if (c[0] == '0') strcat (v, "NOT ");
  if (stereo) strcat (v, "STEREOCHEMICALLY");
  else strcat (v, "CONSTITUTIONALLY");
  strcat (v, " EQUIVALENT\n\t\tto every other instance.");

  return (v);
}

/********************* MORE **********************************/

char *more (char *c)
{
  static char v[1024];
  int n;

  sscanf (c + 3, "%3d", &n);
  strcpy (v, "\t\tThere\n\t\t\tARE ");
  if (c[0] == '0') strcat (v, "NOT ");
  if (c[6] == '>') strcat (v, "MORE");
  else strcat (v, "FEWER");
  strcat (v, "\n\t\tinstances of the functional group\n\t\t\t");
  strcat (v, fg[n - 1]);
  strcat (v, "\n\t\tin the goal than there are in the subgoal.");

  return (v);
}

/********************* AR ************************************/

char *ar (char *c)
{
  static char v[1024];
  int         subtype, node, cmpval, ho;
  char        operator[3];
  Boolean_t   h_only, negate;

  negate = c[0] == '0';
  sscanf (c + 3, "%1d%1d%2d%2s%d", &subtype, &ho, &node, operator, &cmpval);
  h_only = ho != 0;
  switch (subtype)
    {
  case 1:
    strcpy (v, "\t\tIn the subgoal pattern, there ");
    if (negate) strcat (v, "DOES NOT EXIST");
    else strcat (v, "EXISTS");
    strcat (v, "\n\t\tan isolated carbocyclic aromatic ring containing\n\t\t\t");
    sprintf (v + strlen (v), "NODE %02d.", node);
    break;
  case 2:
    strcpy (v, "\t\tIn the subgoal pattern, the activation at\n\t\t\t");
    sprintf (v + strlen (v), "NODE %02d\n\t\trelative to the remainder of the ring\n\t\t\tIS ", node);
    if (negate) strcat (v, "NOT ");
    strcat (v, operator);
    strcat (v, "\n\t\tthe constant\n\t\t\t");
    sprintf (v + strlen (v), "%d\n\t\t[1 represents the most activated position(s)], considering\n\t\t\t", cmpval);
    if (h_only) strcat (v, "ONLY HYDROGENS.");
    else strcat (v, "ALL SUBSTITUENTS.");
    break;
  case 3:
    if (operator[0] == '>') operator[0] = '<';
    else if (operator[0] == '<') operator[0] = '>';
    sprintf (v, "\t\tSubgoal pattern node\n\t\t\t%02d\n\t\t", node);
    strcat (v, "is stabilized during electrophilic substitution to the extent represented by a number which\n\t\t\tIS ");
    if (negate) strcat (v, "NOT ");
    strcat (v, operator);
    strcat (v, "\n\t\tthe constant\n\t\t\t");
    strcat (v, ratexp (cmpval));
    strcat (v, "\n\t\t(numbers above 5 represent activation), considering\n\t\t\t");
    if (h_only) strcat (v, "ONLY HYDROGENS.");
    else strcat (v, "ALL SUBSTITUENTS.");
    break;
  case 4:
    strcpy (v, "\t\tIn the subgoal pattern,\n\t\t\t");
    if (negate) strcat (v, "NOT ");
    strcat (v, "EVERY\n\t\t\t");
    if (h_only) strcat (v, "HYDROGEN-BEARING ");
    sprintf (v + strlen (v), "NODE\n\t\tin the ring that has the same stabilization as node\n\t\t\t%02d\n\t\t", node);
    strcat (v, "is constitutionally equivalent to that node.");
    break;
  case 5:
    strcpy (v, "\t\tThe number of\n\t\t\t");
    if (h_only) strcat (v, "HYDROGEN-BEARING ");
    strcat (v, "NODES\n\t\tin the subgoal pattern having the same stabilization toward electrophilic substitution as node\n\t\t\t");
    sprintf (v + strlen (v), "%02d\n\t\t\tIS ", node);
    if (negate) strcat (v, "NOT ");
    sprintf (v + strlen (v), "%s\n\t\t\t%d.", operator, cmpval);
    break;
  default:
    sprintf (v, "Unrecognized AROMATIC SUBSTITUTION subtype: %d.", subtype);
    break;
  }

  return (v);
}

/********************* BRGHD *********************************/

char *brghd (char *c)
{
  static char v[1024];
  int node;

  sscanf (c + 3, "%2d", &node);
  sprintf (v, "\t\tThe atom in the target molecule that is matched to node\n\t\t\t%02d\n\t\tin the goal pattern\n\t\t\tIS", node);
  if (c[0] == '0') strcat (v, "NOT");
  strcat (v, "\n\t\ta bridgehead carbon in a [9.9.9] or smaller bicyclic system.");

  return (v);
}

/********************* PTREAD ********************************/

void ptread (int schnum, int *num_conditions, Condition_t **conditions, int *num_tests, Posttest_t **tests,
             String_t **reasons, String_t **chemists)
{
  React_Record_t *schrxn;
  char msg[200];
  int i;

  if (!initted) initlib ();
  if (schnum < 1 || schnum > NSch)
    {
    sprintf (msg, "Error in schema number: %d\n", schnum);
    PTRputmsg (msg);
    *num_conditions = *num_tests = 0;
    *conditions = NULL;
    *tests = NULL;
    return;
    }

  /* Adjust schema number value to zero-aligned storage */
  schnum--;
/*
  printf ("PostTransform Tests for Schema %d\n", schnum);
*/
  if (React_Schema_Allocated (NSch))
    React_Schema_Free (NSch);
  React_Schema_Copy (schnum, NSch);
  schrxn = React_Schema_Handle_Get (NSch);
  *num_conditions = React_Head_NumConditions_Get (React_Head_Get (schrxn));
  *num_tests = React_Head_NumTests_Get (React_Head_Get (schrxn));
  *conditions = schrxn->conditions;
  *tests = schrxn->tests;
  *reasons = React_Text_Get (schrxn)->reasons;
  *chemists = React_Text_Get (schrxn)->chemists;

  for (i=0; i<100; i++) cond_marked[i] = test_marked[i] = 0;
}

/********************* PTREAD_NO_INIT ************************/

void ptread_no_init (int schnum, int *num_conditions, Condition_t **conditions, int *num_tests, Posttest_t **tests,
             String_t **reasons, String_t **chemists)
{
  React_Record_t *schrxn;
  char msg[200];
  int i;

  schrxn = React_Schema_Handle_Get (schnum - 1);
  *num_conditions = React_Head_NumConditions_Get (React_Head_Get (schrxn));
  *num_tests = React_Head_NumTests_Get (React_Head_Get (schrxn));
  *conditions = schrxn->conditions;
  *tests = schrxn->tests;
  *reasons = React_Text_Get (schrxn)->reasons;
  *chemists = React_Text_Get (schrxn)->chemists;

  for (i=0; i<100; i++) cond_marked[i] = test_marked[i] = 0;
}

/********************* PT_UPDATE_TESTS************************/

void pt_update_tests (int schnum, int ncond, Condition_t *cond, int ntest, Posttest_t *test, String_t *reas, String_t *chem)
{
  React_Record_t *schrxn;

  schrxn = React_Schema_Handle_Get (schnum - 1);
  React_Head_NumConditions_Put (React_Head_Get (schrxn), ncond);
  React_Head_NumTests_Put (React_Head_Get (schrxn), ntest);
  schrxn->conditions = cond;
  schrxn->tests = test;
  React_Text_Get (schrxn)->reasons = reas;
  React_Text_Get (schrxn)->chemists = chem;
}

/********************* CONDITION_IMPORT **********************/

void condition_import (Condition_t *cond, String_t *string, Boolean_t *node_used)
{
  char *condstr, *cs;
  Boolean_t cond_neg, arh;
  int count,len, i, j, k, dist, node;

  condstr = (char *) malloc (100);
  String_Alloc_Put (*string, 100);
  String_Value_Put (*string, condstr);
  cond_neg = Cond_Negate_Get (cond);
  switch (Cond_Type_Get (cond))
    {
  case PT_TYPE_ELECWD:
    count = Cond_Count_Get (cond);
    sprintf (condstr, "%d01%02d", cond_neg ? 0 : 1, count);
    len = 5;
    for (j=0; j<count; j++)
      {
      node = Cond_ElecWith_Prime_Get (cond, j);
      if (node_used != NULL) node_used[node] = TRUE;
      sprintf(condstr + len, "%02d", node + 1);
      len += 2;
      }
    switch(Cond_Op_Get(cond))
      {
    case OP_GE:
      strcat (condstr, ">=");
      break;
    case OP_GT:
      strcat (condstr, "> ");
      break;
    case OP_LE:
      strcat (condstr, "<=");
      break;
    case OP_LT:
      strcat (condstr, "< ");
      break;
    case OP_EQ:
      strcat (condstr, "= ");
      break;
    case OP_NE:
      strcat (condstr, "~=");
      break;
      }
    len += 2;
    if (Cond_Base_Exists (cond)) sprintf (condstr + len, "00%+d", (S8_t) Cond_Base_Get (cond));
    else
      {
      count = Cond_Count2_Get (cond);
      sprintf (condstr + len, "%02d", count);
      len += 2;
      for (j=0; j<count; j++)
        {
        node = Cond_ElecWith_Second_Get (cond, j);
        if (node_used != NULL) node_used[node] = TRUE;
        sprintf (condstr + len, "%02d", Cond_ElecWith_Second_Get (cond, j) + 1);
        len += 2;
        }
      }
    break;
  case PT_TYPE_NUMMOLEC:
    sprintf (condstr, "%d02%d", cond_neg ? 0 : 1, Cond_NumMolecules_Get (cond));
    break;
  case PT_TYPE_BULKY:
    count = Cond_Count_Get (cond);
    if (node_used != NULL)
      for (j=0; j<count; j++)
      {
      node = Cond_Bulk_Prime_Get (cond, j);
      node_used[node] = TRUE;
      }
    sprintf (condstr, "%d03%02d%02d", cond_neg ? 0 : 1, Cond_Bulk_Prime_Get (cond, 0) + 1,
      count == 2 ? Cond_Bulk_Prime_Get (cond, 1) + 1 : 0);
    switch (Cond_Op_Get (cond))
      {
    case OP_GE:
      strcat (condstr, ">=");
      break;
    case OP_GT:
      strcat (condstr, "> ");
      break;
    case OP_LE:
      strcat (condstr, "<=");
      break;
    case OP_LT:
      strcat (condstr, "< ");
      break;
    case OP_EQ:
      strcat (condstr, "= ");
      break;
    case OP_NE:
      strcat (condstr, "~=");
      break;
      }
    len = 9;
    if (Cond_Base_Exists (cond))
      {
      switch (Cond_Base_Get (cond))
        {
      case PT_METHYL:
        strcat (condstr, "0001");
        break;
      case PT_ETHYL:
        strcat (condstr, "0002");
        break;
      case PT_ISOPROPYL:
        strcat (condstr, "0003");
        break;
      case PT_T_BUTYL:
        strcat (condstr, "0004");
        break;
        }
      }
    else
      {
      count = Cond_Count2_Get (cond);
      if (node_used != NULL)
        for (j=0; j<count; j++)
        {
        node = Cond_Bulk_Second_Get (cond, j);
        node_used[node] = TRUE;
        }
      sprintf (condstr + len, "%02d%02d", Cond_Bulk_Second_Get (cond, 0) + 1,
        count == 2 ? Cond_Bulk_Second_Get (cond, 1) + 1 : 0);
      }
    break;
  case PT_TYPE_DIST:
    node = Cond_Dist_Base_Get (cond);
    if (node_used != NULL) node_used[node] = TRUE;
    sprintf (condstr, "%d04%02d", cond_neg ? 0 : 1, node + 1);
    if (Cond_Dist_Sling_Get (cond))
      {
      strcat (condstr, "00000");
      cs = condstr + 10;
      count=Cond_Count_Get(cond);
      for (j=0; j<count; j++) *cs++ = Cond_Dist_Value_Get (cond, j);
      *cs = 0;
      }
    else sprintf(condstr + 5, "%03d%02d", Cond_Dist_FuncGrp_Get (cond), Cond_Dist_Value_Get (cond, 0));
    break;
  case PT_TYPE_PATHLEN:
    if (node_used != NULL)
      for (j=0; j<2; j++)
      {
      node = Cond_Path_Prime_Get (cond, j);
      node_used[node] = TRUE;
      }
    sprintf(condstr, "%d05%02d%02d", cond_neg ? 0 : 1, Cond_Path_Prime_Get (cond, 0) + 1, Cond_Path_Prime_Get (cond, 1) + 1);
    if (Cond_Path_Connected_Get (cond)) strcat (condstr, "++0000");
    else
      {
      switch(Cond_Op_Get(cond))
        {
      case OP_GE:
        strcat (condstr, ">=");
        break;
      case OP_GT:
        strcat (condstr, "> ");
        break;
      case OP_LE:
        strcat (condstr, "<=");
        break;
      case OP_LT:
        strcat (condstr, "< ");
        break;
      case OP_EQ:
        strcat (condstr, "= ");
        break;
      case OP_NE:
        strcat (condstr, "~=");
        break;
        }
      if (Cond_Base_Exists (cond)) sprintf (condstr + 9, "00%02d", Cond_Base_Get(cond));
      else
        {
        if (node_used != NULL)
          for (j=0; j<2; j++)
          {
          node = Cond_Path_Second_Get (cond, j);
          node_used[node] = TRUE;
          }
        sprintf (condstr + 9, "%02d%02d", Cond_Path_Second_Get (cond, 0) + 1, Cond_Path_Second_Get (cond, 1) + 1);
        }
      }
    break;
  case PT_TYPE_ALKYNE:
    if (node_used != NULL)
      {
      node = Cond_Alkyne_Prime_Get (cond);
      node_used[node] = TRUE;
      node = Cond_Alkyne_Second_Get (cond);
      node_used[node] = TRUE;
      }
    sprintf (condstr, "%d06%02d%02d", cond_neg ? 0 : 1, Cond_Alkyne_Prime_Get (cond) + 1, Cond_Alkyne_Second_Get (cond) + 1);
    break;
  case PT_TYPE_ALLENE:
    if (node_used != NULL)
      {
      node = Cond_Allene_Prime_Get (cond);
      node_used[node] = TRUE;
      node = Cond_Allene_Second_Get (cond);
      node_used[node] = TRUE;
      }
    sprintf (condstr, "%d07%02d%02d", cond_neg ? 0 : 1, Cond_Allene_Prime_Get (cond) + 1, Cond_Allene_Second_Get (cond) + 1);
    break;
  case PT_TYPE_CARBONIUM:
    if (node_used != NULL)
      for (j=0; j<2; j++)
      {
      node = Cond_Stability_Prime_Get (cond, j);
      node_used[node] = TRUE;
      }
    sprintf(condstr, "%d08%02d%02d", cond_neg ? 0 : 1, Cond_Stability_Prime_Get (cond, 0) + 1,
      Cond_Stability_Prime_Get (cond, 1) + 1);
    switch (Cond_Op_Get (cond))
      {
    case OP_GE:
      strcat (condstr, ">=");
      break;
    case OP_GT:
      strcat (condstr, "> ");
      break;
    case OP_LE:
      strcat (condstr, "<=");
      break;
    case OP_LT:
      strcat (condstr, "< ");
      break;
    case OP_EQ:
      strcat (condstr, "= ");
      break;
    case OP_NE:
      strcat (condstr, "~=");
      break;
      }
    if (node_used != NULL)
      for (j=0; j<2; j++)
      {
      node = Cond_Stability_Second_Get (cond, j);
      node_used[node] = TRUE;
      }
    sprintf (condstr + 9, "%02d%02d", Cond_Stability_Second_Get (cond, 0) + 1, Cond_Stability_Second_Get (cond, 1) + 1);
    break;
  case PT_TYPE_LVNGROUP:
    node = Cond_LeavingGrp_Prime_Get (cond);
    if (node_used != NULL)
      node_used[node] = TRUE;
    sprintf (condstr, "%d09%02d%02d", cond_neg ? 0 : 1, node + 1, Cond_LeavingGrp_PrimeDist_Get (cond));
    switch (Cond_LeavingGrp_PrimePh_Get (cond))
      {
    case PT_ACIDIC:
      strcat (condstr, "1");
      break;
    case PT_NEUTRAL:
      strcat (condstr, "2");
      break;
    case PT_BASIC:
      strcat (condstr, "3");
      break;
      }
    switch (Cond_Op_Get (cond))
      {
    case OP_GE:
      strcat (condstr, ">=");
      break;
    case OP_GT:
      strcat (condstr, "> ");
      break;
    case OP_LE:
      strcat (condstr, "<=");
      break;
    case OP_LT:
      strcat (condstr, "< ");
      break;
    case OP_EQ:
      strcat (condstr, "= ");
      break;
    case OP_NE:
      strcat (condstr, "~=");
      break;
      }
    if (Cond_Base_Exists (cond)) sprintf (condstr + 10, "00%02d+", Cond_Base_Get (cond));
    else
      {
      node = Cond_LeavingGrp_Second_Get (cond);
      if (node_used != NULL)
        node_used[node] = TRUE;
      sprintf (condstr + 10, "%02d%02d", node + 1, Cond_LeavingGrp_SecondDist_Get (cond));
      switch (Cond_LeavingGrp_SecondPh_Get (cond))
        {
      case PT_ACIDIC:
        strcat (condstr, "1");
        break;
      case PT_NEUTRAL:
        strcat (condstr, "2");
        break;
      case PT_BASIC:
        strcat (condstr, "3");
        break;
        }
      }
    break;
  case PT_TYPE_MIGRATAP:
    node = Cond_MigratoryApt_Prime_Get (cond);
    if (node_used != NULL)
      node_used[node] = TRUE;
    sprintf (condstr, "%d10%02d", cond_neg ? 0 : 1, node + 1);
    switch (Cond_Op_Get (cond))
      {
    case OP_GE:
      strcat (condstr, ">=");
      break;
    case OP_GT:
      strcat (condstr, "> ");
      break;
    case OP_LE:
      strcat (condstr, "<=");
      break;
    case OP_LT:
      strcat (condstr, "< ");
      break;
    case OP_EQ:
      strcat (condstr, "= ");
      break;
    case OP_NE:
      strcat (condstr, "~=");
      break;
      }
    if (Cond_Base_Exists (cond)) switch (Cond_Base_Get (cond))
      {
    case PT_NOMIG:
      strcat (condstr, "000");
      break;
    case PT_ALKYL:
      strcat (condstr, "001");
      break;
    case PT_ARYL:
      strcat (condstr, "002");
      break;
    case PT_PROTON:
      strcat (condstr, "003");
      break;
      }
    else
      {
      node = Cond_MigratoryApt_Second_Get (cond);
      if (node_used != NULL)
        node_used[node] = TRUE;
      sprintf (condstr + 7, "%02d+", node + 1);
      }
    break;
  case PT_TYPE_ATOM:
    node = Cond_Atom_Node_Get (cond);
    if (node_used != NULL)
      node_used[node] = TRUE;
    sprintf (condstr, "%d11%02d%03d", cond_neg ? 0 : 1, node + 1, Cond_Atom_Id_Get (cond));
    dist = Cond_Atom_Distance_Get (cond);
    if (dist) sprintf (condstr + 8, "%02d", dist);
    break;
  case PT_TYPE_FG_XCESS:
    node = Cond_Excess_Node_Get (cond);
    if (node_used != NULL)
      node_used[node] = TRUE;
    sprintf(condstr, "%d12%03d%02d%02d", cond_neg ? 0 : 1, Cond_Excess_FGNum_Get (cond), node + 1, Cond_Excess_Count_Get (cond));
    break;
  case PT_TYPE_AT_CONEQ:
    sprintf (condstr, "%d13%02d", cond_neg ? 0 : 1, (count = Cond_Count_Get (cond)) + (Cond_Goal_Get (cond) ? 50 : 0));
    for (j=0; j<count; j++)
      {
      node = Cond_AtomsCE_Get (cond, j);
      if (node_used != NULL) node_used[node] = TRUE;
      sprintf (condstr + 2 * j + 5, "%02d", node + 1);
      }
    break;
  case PT_TYPE_AT_STREQ:
    sprintf (condstr, "%d14%02d", cond_neg ? 0 : 1, (count = Cond_Count_Get (cond)) + (Cond_Goal_Get (cond) ? 50 : 0));
    for (j=0; j<count; j++)
      {
      node = Cond_AtomsSE_Get (cond, j);
      if (node_used != NULL) node_used[node] = TRUE;
      sprintf (condstr + 2 * j + 5, "%02d", node + 1);
      }
    break;
  case PT_TYPE_FG_CONEQ:
    sprintf(condstr, "%d15%d%03d", cond_neg ? 0 : 1, Cond_Goal_Get (cond) ? 1 : 0, Cond_FuncGrpCE_Get(cond));
    break;
  case PT_TYPE_FG_STREQ:
    sprintf(condstr, "%d16%d%03d", cond_neg ? 0 : 1, Cond_Goal_Get (cond) ? 1 : 0, Cond_FuncGrpSE_Get(cond));
    break;
  case PT_TYPE_DISC_CONEQ:
    sprintf (condstr, "%d17%02d", cond_neg ? 0 : 1, (count = Cond_Count_Get (cond)));
    for (j=0; j<count; j++)
      {
      node = Cond_DiscCE_Get (cond, j);
      if (node_used != NULL) node_used[node] = TRUE;
      sprintf (condstr + 2 * j + 5, "%02d", node + 1);
      }
    break;
  case PT_TYPE_DISC_STREQ:
    sprintf (condstr, "%d18%02d", cond_neg ? 0 : 1, (count = Cond_Count_Get (cond)));
    for (j=0; j<count; j++)
      {
      node = Cond_DiscSE_Get (cond, j);
      if (node_used != NULL) node_used[node] = TRUE;
      sprintf (condstr + 2 * j + 5, "%02d", node + 1);
      }
    break;
  case PT_TYPE_FG_CNT:
    sprintf(condstr, "%d19%03d", cond_neg ? 0 : 1, Cond_FuncGrpCnt_Get(cond));
    switch(Cond_Op_Get(cond))
      {
    case OP_GT:
      strcat (condstr, ">");
      break;
    case OP_LT:
      strcat (condstr, "<");
      break;
      }
    break;
  case PT_TYPE_AROMSUB:
    sprintf (condstr, "%d20", cond_neg ? 0 : 1);
    node=Cond_AromatSub_Node_Get(cond)+1;
    if (node_used != NULL) node_used[node] = TRUE;
    arh=Cond_AromatSub_Hydrogen_Get(cond);
    switch(Cond_AromatSub_Type_Get(cond))
      {
    case PT_AROM1:
      sprintf (condstr + 3, "1%d%02d", arh ? 1 : 0, node);
      break;
    case PT_AROM2:
      sprintf (condstr + 3, "2%d%02d", arh ? 1 : 0, node);
      switch(Cond_Op_Get(cond))
        {
      case OP_GE:
        strcat (condstr, ">=");
        break;
      case OP_GT:
        strcat (condstr, "> ");
        break;
      case OP_LE:
        strcat (condstr, "<=");
        break;
      case OP_LT:
        strcat (condstr, "< ");
        break;
      case OP_EQ:
        strcat (condstr, "= ");
        break;
      case OP_NE:
        strcat (condstr, "~=");
        break;
        }
      sprintf(condstr + 9, "%1d", Cond_AromatSub_Metric_Get(cond));
      break;
    case PT_AROM3:
      sprintf (condstr + 3, "3%d%02d", arh ? 1 : 0, node);
      switch(Cond_Op_Get(cond))
        {
      case OP_GE:
        strcat (condstr, ">=");
        break;
      case OP_GT:
        strcat (condstr, "> ");
        break;
      case OP_LE:
        strcat (condstr, "<=");
        break;
      case OP_LT:
        strcat (condstr, "< ");
        break;
      case OP_EQ:
        strcat (condstr, "= ");
        break;
      case OP_NE:
        strcat (condstr, "~=");
        break;
        }
      sprintf(condstr + 9, "%+04d", Cond_AromatSub_Metric_Get(cond));
      break;
    case PT_AROM4:
      sprintf (condstr + 3, "4%d%02d", arh ? 1 : 0, node);
      break;
    case PT_AROM5:
      sprintf (condstr + 3, "5%d%02d", arh ? 1 : 0, node);
      switch(Cond_Op_Get(cond))
        {
      case OP_GE:
        strcat (condstr, ">=");
        break;
      case OP_GT:
        strcat (condstr, "> ");
        break;
      case OP_LE:
        strcat (condstr, "<=");
        break;
      case OP_LT:
        strcat (condstr, "< ");
        break;
      case OP_EQ:
        strcat (condstr, "= ");
        break;
      case OP_NE:
        strcat (condstr, "~=");
        break;
        }
      sprintf(condstr + 9, "%1d", Cond_AromatSub_Metric_Get(cond));
      break;
      }
    break;
    }
  String_Length_Put (*string, strlen (condstr));
}

/********************* CONDITION_EXPORT **********************/

void condition_export_refresh (Condition_t *cond, String_t *string, Boolean_t *node_used, Boolean_t refresh)
{
  char *condstr, *cs, tempstr[10];
  Boolean_t cond_neg, arh, cond_base_exists;
  int count,len, i, j, k, dist, node, node2, cond_type, cond_op, cond_base,
    fg, atom, subtype, metric;

  condstr = String_Value_Get (*string);
  cond_neg = condstr[0] == '0';
  Cond_Negate_Put (cond, cond_neg);
  strncpy (tempstr, condstr + 1, 2);
  tempstr[2] = '\0';
  sscanf (tempstr, "%d", &cond_type);
  Cond_Type_Put (cond, cond_type);
  len = 3;

/* Initialize counts to accommodate conditions that don't use them -
   otherwise, posttest.c will complain about too many items stuffed
   into a Condition_t! */

  Cond_Count_Put (cond, 0);
  Cond_Count2_Put (cond, 0);

  switch (cond_type)
    {
  case PT_TYPE_ELECWD:
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &count);
    Cond_Count_Put (cond, count);
    len += 2;
    for (j=0; j<count; j++)
      {
      strncpy (tempstr, condstr + len, 2);
      tempstr[2] = '\0';
      sscanf (tempstr, "%d", &node);
      node--;
      Cond_ElecWith_Prime_Put (cond, j, node);
      if (node_used != NULL) node_used[node] = TRUE;
      len += 2;
      }
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    if (strcmp (tempstr, ">=") == 0) cond_op = OP_GE;
    else if (strcmp (tempstr, "> ") == 0) cond_op = OP_GT;
    else if (strcmp (tempstr, "<=") == 0) cond_op = OP_LE;
    else if (strcmp (tempstr, "< ") == 0) cond_op = OP_LT;
    else if (strcmp (tempstr, "= ") == 0) cond_op = OP_EQ;
    else if (strcmp (tempstr, "~=") == 0) cond_op = OP_NE;
    Cond_Op_Put (cond, cond_op);
    len += 2;
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &count);
    cond_base_exists = count == 0;
    len += 2;

    if (cond_base_exists)
      {
      sscanf (condstr + len, "%d", &cond_base);
      Cond_Base_Put (cond, (U8_t) cond_base); /* unsigned storage!?!? */
      }
    else
      {
      Cond_Base_Put (cond, COND_INVALID);
      Cond_Count2_Put (cond, count);
      for (j=0; j<count; j++)
        {
        strncpy (tempstr, condstr + len, 2);
        tempstr[2] = '\0';
        sscanf (tempstr, "%d", &node);
        node--;
        Cond_ElecWith_Second_Put (cond, j, node);
        if (node_used != NULL) node_used[node] = TRUE;
        len += 2;
        }
      }
    break;
  case PT_TYPE_NUMMOLEC:
    sscanf (condstr + len, "%d", &count);
    Cond_NumMolecules_Put (cond, count);
    break;
  case PT_TYPE_BULKY:
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = 0;
    sscanf (tempstr, "%d", &node);
    len += 2;
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = 0;
    sscanf (tempstr, "%d", &node2);
    len += 2;
    count = node2 == 0 ? 1 : 2;
    node--;
    node2--;
    Cond_Count_Put (cond, count);
    Cond_Bulk_Prime_Put (cond, 0, node);
    if (node_used != NULL) node_used[node] = TRUE;
    if (count == 2)
      {
      Cond_Bulk_Prime_Put (cond, 1, node2);
      if (node_used != NULL) node_used[node2] = TRUE;
      }
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    if (strcmp (tempstr, ">=") == 0) cond_op = OP_GE;
    else if (strcmp (tempstr, "> ") == 0) cond_op = OP_GT;
    else if (strcmp (tempstr, "<=") == 0) cond_op = OP_LE;
    else if (strcmp (tempstr, "< ") == 0) cond_op = OP_LT;
    else if (strcmp (tempstr, "= ") == 0) cond_op = OP_EQ;
    else if (strcmp (tempstr, "~=") == 0) cond_op = OP_NE;
    Cond_Op_Put (cond, cond_op);
    len += 2;
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &node);
    len += 2;
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &node2);
    cond_base_exists = node == 0;
    count = node2 == 0 ? 1 : 2;
    node--;
    cond_base = node2--;
    if (cond_base_exists) switch (cond_base)
      {
    case 1:
      Cond_Base_Put (cond, PT_METHYL);
      break;
    case 2:
      Cond_Base_Put (cond, PT_ETHYL);
      break;
    case 3:
      Cond_Base_Put (cond, PT_ISOPROPYL);
      break;
    case 4:
      Cond_Base_Put (cond, PT_T_BUTYL);
      break;
      }
    else
      {
      Cond_Base_Put (cond, COND_INVALID);
      Cond_Count2_Put (cond, count);
      Cond_Bulk_Second_Put (cond, 0, node);
      if (node_used != NULL) node_used[node] = TRUE;
      if (count == 2)
        {
        Cond_Bulk_Second_Put (cond, 1, node2);
        if (node_used != NULL) node_used[node2] = TRUE;
        }
      }
    break;
  case PT_TYPE_DIST:
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &node);
    node--;
    len += 2;
    Cond_Dist_Base_Put (cond, node);
    if (node_used != NULL) node_used[node] = TRUE;
    strncpy (tempstr, condstr + len, 3);
    tempstr[3] = '\0';
    sscanf (tempstr, "%d", &fg);
    len += 3;
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = 0;
    sscanf (tempstr, "%d", &dist);
    len += 2;
    if (fg == 0)
      {
      Cond_Dist_Sling_Put (cond, TRUE);
      cs = condstr + len;
      count = strlen (cs);
      Cond_Count_Put (cond, count);
      for (j=0; j<count; j++) Cond_Dist_Value_Put (cond, j, *cs++);
      }
    else
      {
      Cond_Dist_Sling_Put (cond, FALSE);
      Cond_Dist_FuncGrp_Put (cond, fg);
      Cond_Dist_Value_Put (cond, 0, dist);
      }
    break;
  case PT_TYPE_PATHLEN:
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = 0;
    sscanf (tempstr, "%d", &node);
    node--;
    len += 2;
    Cond_Path_Prime_Put (cond, 0, node);
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = 0;
    sscanf (tempstr, "%d", &node2);
    node2--;
    len += 2;
    Cond_Path_Prime_Put (cond, 1, node2);
    if (node_used != NULL) node_used[node] = node_used[node2] = TRUE;
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    len += 2;
    if (strcmp (tempstr, "++") == 0) {Cond_Path_Connected_Put (cond, TRUE);}
/* Braces are needed due to prototype-incompatible macro definition (contains if/else w/o braces) */
    else
      {
      Cond_Path_Connected_Put (cond, FALSE);
      if (strcmp (tempstr, ">=") == 0) cond_op = OP_GE;
      else if (strcmp (tempstr, "> ") == 0) cond_op = OP_GT;
      else if (strcmp (tempstr, "<=") == 0) cond_op = OP_LE;
      else if (strcmp (tempstr, "< ") == 0) cond_op = OP_LT;
      else if (strcmp (tempstr, "= ") == 0) cond_op = OP_EQ;
      else if (strcmp (tempstr, "~=") == 0) cond_op = OP_NE;
      Cond_Op_Put (cond, cond_op);
      strncpy (tempstr, condstr + len, 2);
      tempstr[2] = '\0';
      sscanf (tempstr, "%d", &node);
      len+=2;
      strncpy (tempstr, condstr + len, 2);
      tempstr[2] = '\0';
      sscanf (tempstr, "%d", &node2);
      len += 2;
      if (node == 0) Cond_Base_Put (cond, node2);
      else
        {
        node--;
        node2--;
        Cond_Base_Put (cond, COND_INVALID);
        Cond_Path_Second_Put (cond, 0, node);
        Cond_Path_Second_Put (cond, 1, node2);
        if (node_used != NULL) node_used[node] = node_used[node2] = TRUE;
        }
      }
    break;
  case PT_TYPE_ALKYNE:
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &node);
    node--;
    len += 2;
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &node2);
    node2--;
    len += 2;
    Cond_Alkyne_Prime_Put (cond, node);
    Cond_Alkyne_Second_Put (cond, node2);
    if (node_used != NULL) node_used[node] = node_used[node2] = TRUE;
    break;
  case PT_TYPE_ALLENE:
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &node);
    node--;
    len += 2;
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &node2);
    node2--;
    len += 2;
    Cond_Allene_Prime_Put (cond, node);
    Cond_Allene_Second_Put (cond, node2);
    if (node_used != NULL) node_used[node] = node_used[node2] = TRUE;
    break;
  case PT_TYPE_CARBONIUM:
    for (j=0; j<2; j++)
      {
      strncpy (tempstr, condstr + len, 2);
      tempstr[2] = '\0';
      sscanf (tempstr, "%d", &node);
      node--;
      len += 2;
      Cond_Stability_Prime_Put (cond, j, node);
      if (node_used != NULL) node_used[node] = TRUE;
      }
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    if (strcmp (tempstr, ">=") == 0) cond_op = OP_GE;
    else if (strcmp (tempstr, "> ") == 0) cond_op = OP_GT;
    else if (strcmp (tempstr, "<=") == 0) cond_op = OP_LE;
    else if (strcmp (tempstr, "< ") == 0) cond_op = OP_LT;
    else if (strcmp (tempstr, "= ") == 0) cond_op = OP_EQ;
    else if (strcmp (tempstr, "~=") == 0) cond_op = OP_NE;
    Cond_Op_Put (cond, cond_op);
    len += 2;
    for (j=0; j<2; j++)
      {
      strncpy (tempstr, condstr + len, 2);
      tempstr[2] = '\0';
      sscanf (tempstr, "%d", &node);
      node--;
      len += 2;
      Cond_Stability_Second_Put (cond, j, node);
      if (node_used != NULL) node_used[node] = TRUE;
      }
    break;
  case PT_TYPE_LVNGROUP:
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &node);
    node--;
    len += 2;
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &dist);
    len += 2;
    Cond_LeavingGrp_Prime_Put (cond, node);
    Cond_LeavingGrp_PrimeDist_Put (cond, dist);
    if (node_used != NULL) node_used[node] = TRUE;
    switch (condstr[len++])
      {
    case '1':
      Cond_LeavingGrp_PrimePh_Put (cond, PT_ACIDIC);
      break;
    case '2':
      Cond_LeavingGrp_PrimePh_Put (cond, PT_NEUTRAL);
      break;
    case '3':
      Cond_LeavingGrp_PrimePh_Put (cond, PT_BASIC);
      break;
      }
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    if (strcmp (tempstr, ">=") == 0) cond_op = OP_GE;
    else if (strcmp (tempstr, "> ") == 0) cond_op = OP_GT;
    else if (strcmp (tempstr, "<=") == 0) cond_op = OP_LE;
    else if (strcmp (tempstr, "< ") == 0) cond_op = OP_LT;
    else if (strcmp (tempstr, "= ") == 0) cond_op = OP_EQ;
    else if (strcmp (tempstr, "~=") == 0) cond_op = OP_NE;
    Cond_Op_Put (cond, cond_op);
    len += 2;
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &node);
    len += 2;
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &dist);
    len += 2;
    if (node == 0) Cond_Base_Put (cond, dist);
    else
      {
      Cond_Base_Put (cond, COND_INVALID);
      node--;
      Cond_LeavingGrp_Second_Put (cond, node);
      if (node_used != NULL) node_used[node] = TRUE;
      Cond_LeavingGrp_SecondDist_Put (cond, dist);
      switch (condstr[len++])
        {
      case '1':
        Cond_LeavingGrp_SecondPh_Put (cond, PT_ACIDIC);
        break;
      case '2':
        Cond_LeavingGrp_SecondPh_Put (cond, PT_NEUTRAL);
        break;
      case '3':
        Cond_LeavingGrp_SecondPh_Put (cond, PT_BASIC);
        break;
        }
      }
    break;
  case PT_TYPE_MIGRATAP:
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &node);
    node--;
    len += 2;
    Cond_MigratoryApt_Prime_Put (cond, node);
    if (node_used != NULL) node_used[node] = TRUE;
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    if (strcmp (tempstr, ">=") == 0) cond_op = OP_GE;
    else if (strcmp (tempstr, "> ") == 0) cond_op = OP_GT;
    else if (strcmp (tempstr, "<=") == 0) cond_op = OP_LE;
    else if (strcmp (tempstr, "< ") == 0) cond_op = OP_LT;
    else if (strcmp (tempstr, "= ") == 0) cond_op = OP_EQ;
    else if (strcmp (tempstr, "~=") == 0) cond_op = OP_NE;
    Cond_Op_Put (cond, cond_op);
    len += 2;
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &node);
    len += 2;
    if (node == 0) switch (condstr[len++])
      {
    case '0':
      Cond_Base_Put (cond, PT_NOMIG);
      break;
    case '1':
      Cond_Base_Put (cond, PT_ALKYL);
      break;
    case '2':
      Cond_Base_Put (cond, PT_ARYL);
      break;
    case '3':
      Cond_Base_Put (cond, PT_PROTON);
      break;
      }
    else
      {
      Cond_Base_Put (cond, COND_INVALID);
      node--;
      Cond_MigratoryApt_Second_Put (cond, node);
      if (node_used != NULL) node_used[node] = TRUE;
      }
    break;
  case PT_TYPE_ATOM:
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &node);
    node--;
    len += 2;
    Cond_Atom_Node_Put (cond, node);
    if (node_used != NULL) node_used[node] = TRUE;
    strncpy (tempstr, condstr + len, 3);
    tempstr[3] = '\0';
    sscanf (tempstr, "%d", &atom);
    len += 3;
    Cond_Atom_Id_Put (cond, atom);
    if (condstr[len] == '\0') Cond_Atom_Distance_Put (cond, 0);
    else
      {
      sscanf (condstr + len, "%d", &dist);
      Cond_Atom_Distance_Put (cond, dist);
      }
    break;
  case PT_TYPE_FG_XCESS:
    strncpy (tempstr, condstr + len, 3);
    tempstr[3] = '\0';
    sscanf (tempstr, "%d", &fg);
    len += 3;
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &node);
    node--;
    len += 2;
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &count);
    len += 2;
    Cond_Excess_FGNum_Put (cond, fg);
    Cond_Excess_Node_Put (cond, node);
    if (node_used != NULL) node_used[node] = TRUE;
    Cond_Excess_Count_Put (cond, count);
    break;
  case PT_TYPE_AT_CONEQ:
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &count);
    len += 2;
    if (count > 50)
      {
      Cond_Goal_Put (cond, TRUE);
      count -= 50;
      }
    else Cond_Goal_Put (cond, FALSE);
    Cond_Count_Put (cond, count);
    for (j=0; j<count; j++)
      {
      strncpy (tempstr, condstr + len, 2);
      tempstr[2] = '\0';
      sscanf (tempstr, "%d", &node);
      node--;
      len += 2;
      Cond_AtomsCE_Put (cond, j, node);
      if (node_used != NULL) node_used[node] = TRUE;
      }
    break;
  case PT_TYPE_AT_STREQ:
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &count);
    len += 2;
    if (count > 50)
      {
      Cond_Goal_Put (cond, TRUE);
      count -= 50;
      }
    else Cond_Goal_Put (cond, FALSE);
    Cond_Count_Put (cond, count);
    for (j=0; j<count; j++)
      {
      strncpy (tempstr, condstr + len, 2);
      tempstr[2] = '\0';
      sscanf (tempstr, "%d", &node);
      node--;
      len += 2;
      Cond_AtomsSE_Put (cond, j, node);
      if (node_used != NULL) node_used[node] = TRUE;
      }
    break;
  case PT_TYPE_FG_CONEQ:
    Cond_Goal_Put (cond, condstr[len++] == '1');
    sscanf (condstr + len, "%d", &fg);
    Cond_FuncGrpCE_Put (cond, fg);
    break;
  case PT_TYPE_FG_STREQ:
    Cond_Goal_Put (cond, condstr[len++] == '1');
    sscanf (condstr + len, "%d", &fg);
    Cond_FuncGrpSE_Put (cond, fg);
    break;
  case PT_TYPE_DISC_CONEQ:
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &count);
    len += 2;
    Cond_Count_Put (cond, count);
    for (j=0; j<count; j++)
      {
      strncpy (tempstr, condstr + len, 2);
      tempstr[2] = '\0';
      sscanf (tempstr, "%d", &node);
      node--;
      len += 2;
      Cond_DiscCE_Put (cond, j, node);
      if (node_used != NULL) node_used[node] = TRUE;
      }
    break;
  case PT_TYPE_DISC_STREQ:
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &count);
    len += 2;
    Cond_Count_Put (cond, count);
    for (j=0; j<count; j++)
      {
      strncpy (tempstr, condstr + len, 2);
      tempstr[2] = '\0';
      sscanf (tempstr, "%d", &node);
      node--;
      len += 2;
      Cond_DiscSE_Put (cond, j, node);
      if (node_used != NULL) node_used[node] = TRUE;
      }
    break;
  case PT_TYPE_FG_CNT:
    strncpy (tempstr, condstr + len, 3);
    tempstr[3] = '\0';
    sscanf (tempstr, "%d", &fg);
    len += 3;
    Cond_FuncGrpCnt_Put (cond, fg);
    switch (condstr[len])
      {
    case '>':
      Cond_Op_Put (cond, OP_GT);
      break;
    case '<':
      Cond_Op_Put (cond, OP_LT);
      break;
      }
    break;
  case PT_TYPE_AROMSUB:
    subtype = condstr[len++] - '0';
    arh = condstr[len++] == '1';
    strncpy (tempstr, condstr + len, 2);
    tempstr[2] = '\0';
    sscanf (tempstr, "%d", &node);
    node--;
    len += 2;
    Cond_AromatSub_Node_Put (cond, node);
    if (node_used != NULL) node_used[node] = TRUE;
    Cond_AromatSub_Hydrogen_Put(cond, arh);
    switch (subtype)
      {
    case 1:
      Cond_AromatSub_Type_Put (cond, PT_AROM1);
      break;
    case 2:
      Cond_AromatSub_Type_Put (cond, PT_AROM2);
      strncpy (tempstr, condstr + len, 2);
      tempstr[2] = '\0';
      if (strcmp (tempstr, ">=") == 0) cond_op = OP_GE;
      else if (strcmp (tempstr, "> ") == 0) cond_op = OP_GT;
      else if (strcmp (tempstr, "<=") == 0) cond_op = OP_LE;
      else if (strcmp (tempstr, "< ") == 0) cond_op = OP_LT;
      else if (strcmp (tempstr, "= ") == 0) cond_op = OP_EQ;
      else if (strcmp (tempstr, "~=") == 0) cond_op = OP_NE;
      Cond_Op_Put (cond, cond_op);
      len += 2;
      Cond_AromatSub_Metric_Put (cond, condstr[len] - '0');
      break;
    case 3:
      Cond_AromatSub_Type_Put (cond, PT_AROM3);
      strncpy (tempstr, condstr + len, 2);
      tempstr[2] = '\0';
      if (strcmp (tempstr, ">=") == 0) cond_op = OP_GE;
      else if (strcmp (tempstr, "> ") == 0) cond_op = OP_GT;
      else if (strcmp (tempstr, "<=") == 0) cond_op = OP_LE;
      else if (strcmp (tempstr, "< ") == 0) cond_op = OP_LT;
      else if (strcmp (tempstr, "= ") == 0) cond_op = OP_EQ;
      else if (strcmp (tempstr, "~=") == 0) cond_op = OP_NE;
      Cond_Op_Put (cond, cond_op);
      len += 2;
      sscanf (condstr + len, "%d", &metric);
      Cond_AromatSub_Metric_Put(cond, metric);
      break;
    case 4:
      Cond_AromatSub_Type_Put (cond, PT_AROM4);
      sprintf (condstr + 3, "4%d%02d", arh ? 1 : 0, node);
      break;
    case 5:
      Cond_AromatSub_Type_Put (cond, PT_AROM5);
      strncpy (tempstr, condstr + len, 2);
      tempstr[2] = '\0';
      if (strcmp (tempstr, ">=") == 0) cond_op = OP_GE;
      else if (strcmp (tempstr, "> ") == 0) cond_op = OP_GT;
      else if (strcmp (tempstr, "<=") == 0) cond_op = OP_LE;
      else if (strcmp (tempstr, "< ") == 0) cond_op = OP_LT;
      else if (strcmp (tempstr, "= ") == 0) cond_op = OP_EQ;
      else if (strcmp (tempstr, "~=") == 0) cond_op = OP_NE;
      Cond_Op_Put (cond, cond_op);
      len += 2;
      Cond_AromatSub_Metric_Put (cond, condstr[len] - '0');
      break;
      }
    break;
    }
  if (refresh) PostForm_Refresh ();
}

void condition_export (Condition_t *cond, String_t *string, Boolean_t *node_used)
{
  condition_export_refresh (cond, string, node_used, TRUE);
}

/********************* PTWRITE *******************************/

void ptwrite (int nc, Condition_t *cond, char ****condv, int nt, Posttest_t *test, char ****testv, String_t *reas, String_t *chem, int linebreak, Boolean_t *node_used)
{
  char *(*condentry[]) () = {elec, molec, sbulk, dist, conn, alsmr, alsmr, carsb, lvgrp, mgapt,
                             atom, xcess, cseq, cseq, fgeq, fgeq, cseq, cseq, more, ar, brghd};
  int i, j, last_break, op, type, num, n_each_type[22];
  String_t string;
  char cond_name[16], cond_expr[128], test_expr[512], cond_verbose[1024], test_name[16], test_hist[128], test_verbose[4096],
    test_result[128], test_reason[256], *parsestr, msg[200];

  clr_array ();

  if (nc == 0)
    condv[0] = (char ***) malloc (sizeof (char **));
  else
    {
    condv[0] = (char ***) malloc (nc * sizeof (char **));
    for (i = 0; i < nc; i++)
      condv[0][i] = (char **) malloc (3 * sizeof (char *));
    }
  if (nt == 0)
    testv[0] = (char ***) malloc (sizeof (char **));
  else
    {
    testv[0] = (char ***) malloc (nt * sizeof (char **));
    for (i = 0; i < nt; i++)
      testv[0][i] = (char **) malloc (5 * sizeof (char *));
    }

  for (i = 0; i < 22; i++)
    n_each_type[i] = 0;
  for (i = 0; i < nc; i++)
    {
    condition_import (cond + i, &string, node_used);
    strcpy (cond_expr, String_Value_Get (string));
    sscanf (cond_expr + 1, "%2d", &type);

    parsestr = cond_expr + 3;

    /* Before adjusting value of type, parse rest of string for correctness and referenced nodes */
    switch (type)
      {
    case 1:
      elecpr(parsestr);
      break;
    case 2:
      molecpr(parsestr);
      break;
    case 3:
      bulkpr(parsestr);
      break;
    case 4:
      distpr(parsestr);
      break;
    case 5:
      ringpr(parsestr);
      break;
    case 6:
      alkpr(parsestr);
      break;
    case 7:
      allpr(parsestr);
      break;
    case 8:
      stabpr(parsestr);
      break;
    case 9:
      lvgrpr(parsestr);
      break;
    case 10:
      mgaptpr(parsestr);
      break;
    case 11:
      atompr(parsestr);
      break;
    case 12:
      excesspr(parsestr);
      break;
    case 13:
      identpr(parsestr,1);
      break;
    case 14:
      identpr(parsestr,0);
      break;
    case 15:
      instcepr(parsestr,1);
      break;
    case 16:
      instcepr(parsestr,0);
      break;
    case 17:
      discnpr(parsestr,1);
      break;
    case 18:
      discnpr(parsestr,0);
      break;
    case 19:
      more_insts_pr(parsestr);
      break;
    case 20:
      arsub_pr(parsestr);
      break;
    case 21:
      bheadpr(parsestr);
      break;
    default:
      sprintf (msg, "Unrecognized type (%d) for condition #%d\n", type, i + 1);
      PTRputmsg (msg);
      break;
      }

    strcpy (cond_verbose, condentry[type - 1] (cond_expr));

    /* Temporarily move CEDISCN and SEDISCN out of the way */
    if (type == 17 || type == 18) type -= 18;

    /* Make room for 5 ARSUB types */
    if (type > 20) type += 4;

    /* Get ARSUB type */
    if (type == 20)
      {
      sscanf (cond_expr + 3, "%1d", &num);
      type += num - 1;
      }

    /* Fill in the hole left by moving DISCN */
    if (type > 16) type -= 2;

    /* Combine FG CONEQ and STREQ */
    if (type > 15) type--;

    /* Combine AT CONEQ and STREQ */
    if (type > 13) type--;

    /* Combine ALKYNE and ALLENE */
    if (type > 6) type--;

    /* Make room for CONN subtypes RNGSZ and RNGCP */
    if (type > 5) type+=2;

    /* Get CONN type */
    if (type == 5)
      {
      if (cond_expr[7] != '+' && cond_expr[7] != ' ') type++;
      sscanf (cond_expr + 9, "%2d", &num);
      if (num != 0) type++;
      }

    /* point DISCN to CSEQ */
    if (type <= 0) type = 13;

    /* Finally, decrement type to zero-adjust index */
    cond_map[i][0] = --type;
    cond_map[i][1] = ++n_each_type[type];

    sprintf (cond_name, "\t%s%d:\t", condtypes[cond_map[i][0]], cond_map[i][1]);
    condv[0][i][0] = (char *) malloc (strlen (cond_name) + 1);
    strcpy (condv[0][i][0], cond_name);
    condv[0][i][1] = (char *) malloc (strlen (cond_expr) + 2);
    sprintf (condv[0][i][1], "%s\n", cond_expr);
    condv[0][i][2] = (char *) malloc (strlen (cond_verbose) + 3);
    sprintf (condv[0][i][2], "%s\n\n", cond_verbose);
    }

  for (i = 0; i < nt; i++)
    {
    sprintf (test_name, "\tTEST%d", i + 1);
    sprintf (test_hist, " [Created or last modified by %s]:\n", String_Value_Get (chem[i]));
    for (j = test_expr[0] = 0; j < test[i].h.length; j++)
      {
      op = test[i].ops[j];
      if (op < PT_TEST_ADD) sprintf (test_expr + strlen (test_expr), "C%02d", op + 1);
      else switch (op)
        {
      case OP_AND:
        strcat (test_expr, "&");
        break;
      case OP_OR:
        strcat (test_expr, "|");
        break;
      case OP_NOT:
        strcat (test_expr, "~");
        break;
      case OP_NOPASS:
        strcat (test_expr, "NOP");
        break;
      case BOOLOP_EQ:
        strcat (test_expr, "=");
        break;
      case BOOLOP_XOR:
        strcat (test_expr, "#");
        break;
      default:
        sprintf (test_expr + strlen (test_expr), "T%02d", op - PT_TEST_ADD + 1);
        break;
        }
      }
    if (PostIn (test_expr))
      {
      sprintf (test_verbose, "\t\tif ");
      for (j = last_break = 0; j < strlen (test_expr); j++)
        switch (test_expr[j])
        {
      case '&':
        strcat (test_verbose, "and");
        break;
      case '|':
        strcat (test_verbose, "or");
        break;
      case '~':
        strcat (test_verbose, "not ");
        break;
      case '=':
        strcat (test_verbose, "eq");
        break;
      case '#':
        strcat (test_verbose, "ne");
        break;
      case 'N':
        strcat (test_verbose, "<no tests applied so far>");
        j += 2;
        break;
      case 'T':
        sscanf (test_expr + j + 1, "%2d", &num);
        sprintf (test_verbose + strlen (test_verbose), "TEST%d", num);
        j += 2;
        break;
      case 'C':
        sscanf (test_expr + j + 1, "%2d", &num);
        num--;
        sprintf (test_verbose + strlen (test_verbose), "%s%d", condtypes[cond_map[num][0]], cond_map[num][1]);
        j += 2;
        break;
      case ' ':
        if (j - last_break > linebreak && test_expr[j + 1] != 0 && test_expr[j + 1] != '&' && test_expr[j + 1] != '|' &&
          test_expr[j + 1] != '=' && test_expr[j + 1] != '#')
          {
          last_break = j;
          strcat (test_verbose, "\n\t\t\t");
          }
        else strcat (test_verbose, " ");
        break;
      default:
        sprintf (test_verbose + strlen (test_verbose), "%c", test_expr[j]);
        break;
        }
      strcat (test_verbose, "\n");
      }
    else
      strcpy (test_verbose, "\t\t?!? Bad test expression !?!\n");
    if (test[i].h.flags & PostM_Result)
      sprintf (test_result, "\t\tthen %s, adjusting EASE %d, YIELD %d, and CONFIDENCE %d,\n",
      (test[i].h.flags & PostM_Stop) ? "PASS AND STOP" : "PASS", test[i].h.ease_adj, test[i].h.yield_adj, test[i].h.confidence_adj);
    else
      strcpy (test_result, "\t\tthen FAIL,\n");
    sprintf (test_reason, "\t\tbecause %s\n\n", String_Value_Get (reas[i]));
    testv[0][i][0] = (char *) malloc (strlen (test_name) + 1);
    strcpy (testv[0][i][0], test_name);
    testv[0][i][1] = (char *) malloc (strlen (test_hist) + 1);
    strcpy (testv[0][i][1], test_hist);
    testv[0][i][2] = (char *) malloc (strlen (test_verbose) + 1);
    strcpy (testv[0][i][2], test_verbose);
    testv[0][i][3] = (char *) malloc (strlen (test_result) + 1);
    strcpy (testv[0][i][3], test_result);
    testv[0][i][4] = (char *) malloc (strlen (test_reason) + 1);
    strcpy (testv[0][i][4], test_reason);
    }
}

Boolean_t ptest_store (Posttest_t *test, char *teststr, Boolean_t *result, int *eyc, String_t *reas, String_t *chem, int testnum)
{
  int i, j, k, num, uslen, endlen;
  char locstr[1024], numstr[5], *endpos, *delpos, *uspos, condname[10], opstr[512], *ops;
  Boolean_t found;
  React_Record_t *schrxn;
  React_TextRec_t *schtxt;

  strcpy (locstr, teststr);
  for (i = 0; i < strlen (locstr); i++)
    {
    if (locstr[i] == '\n') locstr[i] = ' ';
    else locstr[i] = toupper (locstr[i]);
    }

  for (i = 0; i < strlen (locstr); i++) if (locstr[i] >='A' && locstr[i] <= 'Z')
    {
    if (strncmp (locstr + i, "AND ", 4) == 0)
      {
      locstr[i] = '&';
      strcpy (locstr + i + 1, locstr + i + 3);
      }
    else if (strncmp (locstr + i, "OR ", 3) == 0)
      {
      locstr[i] = '|';
      strcpy (locstr + i + 1, locstr + i + 2);
      }
    else if (strncmp (locstr + i, "NOT ", 4) == 0)
      {
      locstr[i] = '~';
      strcpy (locstr + i + 1, locstr + i + 4);
      }
    else if (strncmp (locstr + i, "EQ ", 3) == 0)
      {
      locstr[i] = '=';
      strcpy (locstr + i + 1, locstr + i + 2);
      }
    else if (strncmp (locstr + i, "NEQ ", 4) == 0 || strncmp (locstr + i, "XOR ", 4) == 0)
      {
      locstr[i] = '#';
      strcpy (locstr + i + 1, locstr + i + 3);
      }
    else if (strncmp (locstr + i, "NOP", 3) == 0)
      {
      i+=2;
      }
    else if (locstr[i] != ' ' && locstr[i] != '(' && locstr[i] != ')' && locstr[i] != '[' && locstr[i] != ']' &&
      locstr[i] != '{' && locstr[i] != '}')
      {
      endpos = strstr (locstr + i, " ");
      if (endpos == NULL) endpos = locstr + strlen (locstr);
      delpos = strstr (locstr + i, "}");
      if (delpos != NULL && delpos < endpos) endpos = delpos;
      delpos = strstr (locstr + i, "]");
      if (delpos != NULL && delpos < endpos) endpos = delpos;
      delpos = strstr (locstr + i, ")");
      if (delpos != NULL && delpos < endpos) endpos = delpos;
      endlen = endpos - locstr - i;
      if (strncmp (locstr + i, "TEST", 4) == 0)
        {
        sscanf (locstr + i + 4, "%d", &num);
        sprintf (numstr, "%02d", num);
        strncpy (locstr + i + 1, numstr, 2);
        strcpy (locstr + i + 3, endpos); 
        }
      else
        {
        for (uspos = locstr + i; *uspos > '9'; uspos++);
        if (*uspos == '\0') return (FALSE);
        uslen = uspos - locstr - i;
        for (j = 0, found = FALSE; j < 22 && !found; j++)
          if (strncmp (locstr + i, condtypes[j], uslen) == 0)
          {
          strncpy (condname, locstr + i, endlen);
          condname[endlen] = '\0';
          num = PTGetCondnum (condname);
          if (num != 0)
            {
            found = TRUE;
            sprintf (numstr, "C%02d", num);
            strncpy (locstr + i, numstr, 3);
            strcpy (locstr + i + 3, endpos);
            }
          }
        if (!found) return (FALSE);
        }
      }
    }

  if (!inpost (locstr)) return (FALSE);

  for (i = j = 0; i < strlen (locstr); i++) switch (locstr[i])
    {
    case '&':
      opstr[j++] = OP_AND;
      break;
    case '|':
      opstr[j++] = OP_OR;
      break;
    case '~':
      opstr[j++] = OP_NOT;
      break;
    case '=':
      opstr[j++] = BOOLOP_EQ;
      break;
    case '#':
      opstr[j++] = BOOLOP_XOR;
      break;
    case 'N':
      if (strncmp (locstr + i, "NOP", 3) != 0) return (FALSE);
      i += 2;
      opstr[j++] = OP_NOPASS;
      break;
    case 'C':
      sscanf (locstr + i + 1, "%d", &num);
      i += 2;
      opstr[j++] = num - 1;
      break;
    case 'T':
      sscanf (locstr + i + 1, "%d", &num);
      i += 2;
      opstr[j++] = num + PT_TEST_ADD - 1;
      break;
    default:
      return (FALSE);
    }

  if (!glob_new_test)
    {
    ops = Post_OpHandle_Get (test);
    free (ops);
    }
  Post_OpHandle_Get (test) = (char *) malloc (j);
  Post_Length_Put (test, j);
  for (i = 0; i < j; i++) Post_Op_Put (test, i, opstr[i]);

  Post_EaseAdj_Put (test, eyc[0]);
  Post_YieldAdj_Put (test, eyc[1]);
  Post_ConfidenceAdj_Put (test, eyc[2]);
  Post_Head_Get (test)->flags = 0;
  Post_Result_Put (test, result[0]);
  Post_Stop_Put (test, result[1]);

/* redundant and incorrect:
  schrxn = React_Schema_Handle_Get (NSch);
  schtxt = React_Text_Get (schrxn);
  React_TxtRec_Reason_Put (schtxt, testnum, *reas);
  React_TxtRec_Chemist_Put (schtxt, testnum, *chem);
*/

  PostForm_Refresh ();

  return (TRUE);
}

void dump_marked ()
{
  int i;

  printf ("Conditions:\n");
  for (i=0; i<glob_nconds; i++) printf ("%d: %d\n",i,cond_marked[i]);
  printf ("Tests:\n");
  for (i=0; i<glob_ntests; i++) printf ("%d: %d\n",i,test_marked[i]);
}

Boolean_t OK_To_Delete_Cond (int condnum)
{
  int i, t, op;

/*
dump_marked ();
*/
  for (t = 0; t < glob_ntests; t++) if (!test_marked[t])
    for (i = 0; i < Post_Length_Get (glob_test_root + t); i++)
    {
    op = Post_Op_Get (glob_test_root + t, i);
    switch(op)
      {
    case OP_AND:
    case OP_OR:
    case OP_NOT:
    case OP_NOPASS:
    case BOOLOP_EQ:
    case BOOLOP_XOR:
      break;
    default:
      if (op >= PT_TEST_ADD) break;
      if (op == condnum)
	{
	printf("Can't delete C%02d due to T%02d\n",condnum+1,t+1);
	return (FALSE);
	}
      }
    }

  return(TRUE);
}

Boolean_t OK_To_Delete_Test (int testnum)
{
  int i, t, op;

/*
dump_marked ();
*/
  for (t = testnum + 1; t < glob_ntests; t++) if (!test_marked[t])
    for (i = 0; i < Post_Length_Get (glob_test_root + t); i++)
    {
    op = Post_Op_Get (glob_test_root + t, i);
    switch(op)
      {
    case OP_AND:
    case OP_OR:
    case OP_NOT:
    case OP_NOPASS:
    case BOOLOP_EQ:
    case BOOLOP_XOR:
      break;
    default:
      if (op < PT_TEST_ADD) break;
      if (op - PT_TEST_ADD == testnum)
	{
	printf("Can't delete T%02d due to T%02d\n",testnum+1,t+1);
	return (FALSE);
	}
      }
    }

  return(TRUE);
}

Boolean_t OK_To_Undelete_Test (int testnum)
{
  int i, op;

  for (i = 0; i < Post_Length_Get (glob_test_root + testnum); i++)
    {
    op = Post_Op_Get (glob_test_root + testnum, i);
    switch(op)
      {
    case OP_AND:
    case OP_OR:
    case OP_NOT:
    case OP_NOPASS:
    case BOOLOP_EQ:
    case BOOLOP_XOR:
      break;
    default:
      if (op < PT_TEST_ADD)
        {
        if (cond_marked[op])
          {
          printf ("Can't undelete T%02d if C%02d remains deleted\n", testnum + 1, op + 1);
          return (FALSE);
          }
        }
      else
        {
        if (test_marked[op - PT_TEST_ADD])
	  {
	  printf("Can't undelete T%02d if T%02d remains deleted\n",testnum + 1, op - PT_TEST_ADD + 1);
	  return (FALSE);
          }
	}
      }
    }

  return(TRUE);
}

void Post_Cond_Mod (Condition_t *cond, Condition_t *cond_root, Posttest_t *test_root, int condnum, int nconds, int ntests,
  Widget tl, Widget mgw, char *newtype)
{
  int i, num_of_type, loc_cond_type;

  cond_add_canceled = test_add_canceled = FALSE;

  if (strcmp (newtype, "MENU") == 0)
  {
    cond_add_canceled = TRUE;
    return;
  }

  curr_is_test = FALSE;
  curr_num = condnum;

  glob_cond = cond;
  glob_cond_root = cond_root;
  glob_test_root = test_root;
  glob_condnum = condnum;
  glob_nconds = nconds;
  glob_ntests = ntests;
  glob_tl = tl;
  glob_mgw = mgw;
  glob_new_cond = newtype[0] != '\0';
  if (glob_new_cond)
    {
    strcpy (glob_new_cond_type, newtype);
    if (strcmp (newtype, "ELEC") == 0)
      {
      glob_cond_type = PT_TYPE_ELECWD;
      loc_cond_type = 0;
      }
    else if (strcmp (newtype, "MOLEC") == 0)
      {
      glob_cond_type = PT_TYPE_NUMMOLEC;
      loc_cond_type = 1;
      }
    else if (strcmp (newtype, "SBULK") == 0)
      {
      glob_cond_type = PT_TYPE_BULKY;
      loc_cond_type = 2;
      }
    else if (strcmp (newtype, "DIST") == 0)
      {
      glob_cond_type = PT_TYPE_DIST;
      loc_cond_type = 3;
      }
    else if (strcmp (newtype, "CONN") == 0)
      {
      glob_cond_type = PT_TYPE_PATHLEN;
      loc_cond_type = 4;
      }
    else if (strcmp (newtype, "RNGSZ") == 0)
      {
      glob_cond_type = PT_TYPE_PATHLEN;
      loc_cond_type = 5;
      }
    else if (strcmp (newtype, "RNGCP") == 0)
      {
      glob_cond_type = PT_TYPE_PATHLEN;
      loc_cond_type = 6;
      }
    else if (strcmp (newtype, "ALSMR") == 0)
      {
      glob_cond_type = PT_TYPE_ALKYNE;
      loc_cond_type = 7;
      }
    else if (strcmp (newtype, "CARSB") == 0)
      {
      glob_cond_type = PT_TYPE_CARBONIUM;
      loc_cond_type = 8;
      }
    else if (strcmp (newtype, "LVGRP") == 0)
      {
      glob_cond_type = PT_TYPE_LVNGROUP;
      loc_cond_type = 9;
      }
    else if (strcmp (newtype, "MGAPT") == 0)
      {
      glob_cond_type = PT_TYPE_MIGRATAP;
      loc_cond_type = 10;
      }
    else if (strcmp (newtype, "ATOM") == 0)
      {
      glob_cond_type = PT_TYPE_ATOM;
      loc_cond_type = 11;
      }
    else if (strcmp (newtype, "XCESS") == 0)
      {
      glob_cond_type = PT_TYPE_FG_XCESS;
      loc_cond_type = 12;
      }
    else if (strcmp (newtype, "CSEQ") == 0)
      {
      glob_cond_type = PT_TYPE_AT_CONEQ;
      loc_cond_type = 13;
      }
    else if (strcmp (newtype, "FGEQ") == 0)
      {
      glob_cond_type = PT_TYPE_FG_CONEQ;
      loc_cond_type = 14;
      }
    else if (strcmp (newtype, "MORE") == 0)
      {
      glob_cond_type = PT_TYPE_FG_CNT;
      loc_cond_type = 15;
      }
    else if (strcmp (newtype, "ARNOD") == 0)
      {
      glob_cond_type = PT_TYPE_AROMSUB;
      loc_cond_type = 16;
      }
    else if (strcmp (newtype, "ARSTD") == 0)
      {
      glob_cond_type = PT_TYPE_AROMSUB;
      loc_cond_type = 17;
      }
    else if (strcmp (newtype, "ARRAT") == 0)
      {
      glob_cond_type = PT_TYPE_AROMSUB;
      loc_cond_type = 18;
      }
    else if (strcmp (newtype, "ARCET") == 0)
      {
      glob_cond_type = PT_TYPE_AROMSUB;
      loc_cond_type = 19;
      }
    else if (strcmp (newtype, "ARTIE") == 0)
      {
      glob_cond_type = PT_TYPE_AROMSUB;
      loc_cond_type = 20;
      }
    else
      {
      printf ("Unsupported PT condition type: %s\n", newtype);
      return;
      }
    cond_map[condnum][0] = loc_cond_type;
    for (i = num_of_type = 0; i < condnum; i++) if (cond_map[i][0] == loc_cond_type) num_of_type++;
    cond_map[condnum][1] = num_of_type + 1;
    }

  if (!glob_rxlform || logged_in) Post_Cond_Mod_Cont ();
  else if (login_failed)
        {
        LoginFail (tl);
        return;
        }
  else
	{
	XtUnmanageChild (mgw);
	Login (tl, Post_Cond_Mod_Cont, mgw);
	}
}

void Post_Cond_Mod_Cont ()
{
  String_t str;
  char *tstr;
  int type, subtype;

  if (!XtIsManaged (glob_mgw)) XtManageChild (glob_mgw);

  if (glob_rxlform && clearance_level < EDIT_POSTRAN_LEV)
        {
        LoginFail (glob_tl);
        return;
        }

  if (glob_new_cond)
    {
    Cond_Prep_Mark (current_marked = cond_marked[curr_num] = FALSE);
    type = glob_cond_type;
    if (type == PT_TYPE_AROMSUB)
      {
      if (strcmp (glob_new_cond_type, "ARNOD") == 0) subtype = '1';
      else if (strcmp (glob_new_cond_type, "ARSTD") == 0) subtype = '2';
      else if (strcmp (glob_new_cond_type, "ARRAT") == 0) subtype = '3';
      else if (strcmp (glob_new_cond_type, "ARCET") == 0) subtype = '4';
      else if (strcmp (glob_new_cond_type, "ARTIE") == 0) subtype = '5';
      }
    }
  else
    {
    Cond_Prep_Mark (current_marked = cond_marked[curr_num]);
    condition_import (glob_cond, &str, NULL);
    tstr = String_Value_Get (str);
/*
    printf ("modifying condition %d: %s\n", glob_condnum, tstr);
*/
    sscanf (tstr + 1, "%02d", &type);
    subtype = tstr[3];
    if (cond_marked[curr_num]) PT_Enable_Undeletion (TRUE);
    else
      {
      if (OK_To_Delete_Cond (glob_condnum)) PT_Enable_Deletion (TRUE);
      else PT_Enable_Deletion (FALSE);
      }
    }
  switch (type)
    {
  case PT_TYPE_ELECWD:
    PTElec (glob_cond, NULL, glob_condnum, glob_new_cond);
    break;
  case PT_TYPE_NUMMOLEC:
    PTMolec (glob_cond, NULL, glob_condnum, glob_new_cond);
    break;
  case PT_TYPE_BULKY:
    PTBulk (glob_cond, NULL, glob_condnum, glob_new_cond);
    break;
  case PT_TYPE_DIST:
    PTDist (glob_cond, NULL, glob_condnum, glob_new_cond);
    break;
  case PT_TYPE_PATHLEN:
    PTConn (glob_cond, NULL, glob_condnum, glob_new_cond, glob_new_cond_type);
    break;
  case PT_TYPE_ALKYNE:
  case PT_TYPE_ALLENE:
    PTAlSmR (glob_cond, NULL, glob_condnum, glob_new_cond);
    break;
  case PT_TYPE_CARBONIUM:
    PTCarSb (glob_cond, NULL, glob_condnum, glob_new_cond);
    break;
  case PT_TYPE_LVNGROUP:
    PTLvGrp (glob_cond, NULL, glob_condnum, glob_new_cond);
    break;
  case PT_TYPE_MIGRATAP:
    PTMgApt (glob_cond, NULL, glob_condnum, glob_new_cond);
    break;
  case PT_TYPE_ATOM:
    PTAtom (glob_cond, NULL, glob_condnum, glob_new_cond);
    break;
  case PT_TYPE_FG_XCESS:
    PTXcess (glob_cond, NULL, glob_condnum, glob_new_cond);
    break;
  case PT_TYPE_AT_CONEQ:
  case PT_TYPE_AT_STREQ:
  case PT_TYPE_DISC_CONEQ:
  case PT_TYPE_DISC_STREQ:
    PTCSEq (glob_cond, NULL, glob_condnum, glob_new_cond);
    break;
  case PT_TYPE_FG_CONEQ:
  case PT_TYPE_FG_STREQ:
    PTFGEq (glob_cond, NULL, glob_condnum, glob_new_cond);
    break;
  case PT_TYPE_FG_CNT:
    PTMore (glob_cond, NULL, glob_condnum, glob_new_cond);
    break;
  case PT_TYPE_AROMSUB:
    switch (subtype)
      {
    case '1':
      PTArNod (glob_cond, NULL, glob_condnum, glob_new_cond);
      break;
    case '2':
      PTArStd (glob_cond, NULL, glob_condnum, glob_new_cond);
      break;
    case '3':
      PTArRat (glob_cond, NULL, glob_condnum, glob_new_cond);
      break;
    case '4':
      PTArCET (glob_cond, NULL, glob_condnum, glob_new_cond);
      break;
    case '5':
      PTArTie (glob_cond, NULL, glob_condnum, glob_new_cond);
      break;
      }
    break;
  default:
    break;
    }
/* need to free, but this isn't final form anyway */
}

void Post_Test_Mod (Posttest_t *test, Condition_t *cond_root, Posttest_t *test_root, String_t *reas, String_t *chem, int testnum,
  int nconds, int ntests, Widget tl, Widget mgw, Boolean_t new)
{
  cond_add_canceled = test_add_canceled = FALSE;

  curr_is_test = TRUE;
  curr_num = testnum;

  glob_test = test;
  glob_cond_root = cond_root;
  glob_test_root = test_root;
  glob_reas = reas;
  glob_chem = chem;
  glob_testnum = testnum;
  glob_nconds = nconds;
  glob_ntests = ntests;
  glob_tl = tl;
  glob_mgw = mgw;
  glob_new_test = new;

  if (!glob_rxlform || logged_in) Post_Test_Mod_Cont ();
  else if (login_failed)
        {
        LoginFail (tl);
        return;
        }
  else
	{
	XtUnmanageChild (mgw);
	Login (tl, Post_Test_Mod_Cont, mgw);
	}
}

void Post_Test_Mod_Cont ()
{
  char teststr[1000], flagstr[20], tempstr[10], locstr[1000];
  int i, j, eyc[3], op, num;
  Boolean_t result[2];

  if (!XtIsManaged (glob_mgw)) XtManageChild (glob_mgw);

  if (glob_rxlform && clearance_level < EDIT_POSTRAN_LEV)
        {
        LoginFail (glob_tl);
        return;
        }

  if (glob_new_test)
    {
    Cond_Prep_Mark (current_marked = test_marked[curr_num] = FALSE);
    strcpy (teststr, "NOP");
    result[0] = result[1] = FALSE;
    eyc[0] = eyc[1] = eyc[2] = 0;
    *glob_reas = String_Create ("I have a brain the size of a planet ...", 0);
    strcpy (tempstr, UserId ());
    *glob_chem = String_Create ((const char *) tempstr, 0);

    PTest (glob_test, teststr, result, eyc, glob_reas, glob_chem, glob_testnum, TRUE);

    return;
    }
  else
    {
    Cond_Prep_Mark (current_marked = test_marked[curr_num]);
    if (test_marked[curr_num])
      {
      if (OK_To_Undelete_Test (glob_testnum)) PT_Enable_Undeletion (TRUE);
      else PT_Enable_Undeletion (FALSE);
      }
    else
      {
      if (OK_To_Delete_Test (glob_testnum)) PT_Enable_Deletion (TRUE);
      else PT_Enable_Deletion (FALSE);
      }
    }

  for (i = locstr[0] = 0; i < Post_Length_Get (glob_test); i++)
    {
    op = Post_Op_Get (glob_test,i);
    switch (op)
      {
    case OP_AND:
      strcpy (tempstr, "&");
      break;
    case OP_OR:
      strcpy (tempstr, "|");
      break;
    case OP_NOT:
      strcpy (tempstr, "~");
      break;
    case OP_NOPASS:
      strcpy (tempstr, "NOP");
      break;
    case BOOLOP_EQ:
      strcpy (tempstr, "=");
      break;
    case BOOLOP_XOR:
      strcpy (tempstr, "#");
      break;
    default:
      if (op < PT_TEST_ADD)
        sprintf (tempstr, "C%02d", op + 1);
      else
        sprintf (tempstr, "T%02d", op - PT_TEST_ADD + 1);
      break;
      }
    strcat (locstr, tempstr);
    }

  if (!PostIn (locstr))
    {
    printf ("Error in string: %s\n", locstr);;
    teststr[0] = 0;
    }
  else
    {
    for (i = j = 0; i < strlen (locstr); i++) switch (locstr[i])
      {
    case '&':
      strcpy (teststr + j, "and");
      j += 3;
      break;
    case '|':
      strcpy (teststr + j, "or");
      j += 2;
      break;
    case '~':
      strcpy (teststr + j, "not ");
      j += 4;
      break;
    case '=':
      strcpy (teststr + j, "eq");
      j += 2;
      break;
    case '#':
      strcpy (teststr + j, "neq");
      j += 2;
      break;
    case 'T':
      strcpy (teststr + j, "TEST");
      j += 4;
      if (locstr[i + 1] == '0') i++;
      break;
    case 'C':
      sscanf (locstr + i + 1, "%d", &num);
      sprintf (teststr + j, "%s%d", condtypes[cond_map[num - 1][0]], cond_map[num - 1][1]);
      j = strlen (teststr);
      i += 2;
      break;
    default:
      teststr[j++] = locstr[i];
      break;
      }
    teststr[j]=0;
    }

  result[0] = Post_Result_Get (glob_test);
  result[1] = Post_Stop_Get (glob_test);
  strcpy (flagstr, result[0] ? (result[1] ? "PASS AND STOP" : "PASS") : "FAIL");
  eyc[0] = Post_EaseAdj_Get (glob_test);
  eyc[1] = Post_YieldAdj_Get (glob_test);
  eyc[2] = Post_ConfidenceAdj_Get (glob_test);
/*
  printf ("modifying test %d: %s (%s: e%d y%d c%d)\n\t%s\n\t%s\n", glob_testnum, teststr, flagstr, eyc[0], eyc[1], eyc[2],
    String_Value_Get (*glob_reas), String_Value_Get (*glob_chem));
*/
  PTest (glob_test, teststr, result, eyc, glob_reas, glob_chem, glob_testnum, FALSE);
}

void PTSetCondname (int cond_num, char *cond_name)
{
  sprintf (cond_name, "%s%d", condtypes[cond_map[cond_num][0]], cond_map[cond_num][1]);
}

int PTGetCondnum (char *cond_name)
{
  char *us, savecond[16];
  int cond_num, cond_inx, typelen, old_num;
  Boolean_t found;

  for (us = cond_name; *us > '9'; us++);
  typelen=us-cond_name;
  sscanf (us, "%d", &old_num);
  for (cond_inx=0, found=FALSE; cond_inx<22 && !found; cond_inx++)
    if (strncmp (condtypes[cond_inx], cond_name, typelen) == 0) found=TRUE;
  cond_inx--;
  if (!found)
    {
    strcpy (savecond, cond_name);
    sprintf (cond_name, "ERROR: Unrecognized condition type in %s", savecond);
    return (0);
    }

  for (cond_num=0, found=FALSE; cond_num<100 && !found; cond_num++)
    if (cond_map[cond_num][0] == cond_inx && cond_map[cond_num][1] == old_num) found=TRUE;
  if (!found)
    {
    strcpy (savecond, cond_name);
    sprintf (cond_name, "ERROR: Condition %s not found", savecond);
    return (0);
    }
  return (cond_num);
}

void PTUpdateCondname (char *cond_name, char *new_type)
{
  char *us, savecond[16];
  int i, num, cond_num, cond_inx, new_inx, typelen, old_num;
  Boolean_t found;

  for (us = cond_name; *us > '9'; us++);
  typelen=us-cond_name;
  sscanf (us, "%d", &old_num);
  for (cond_inx=0, found=FALSE; cond_inx<22 && !found; cond_inx++)
    if (strncmp (condtypes[cond_inx], cond_name, typelen) == 0) found=TRUE;
  cond_inx--;
  if (!found)
    {
    strcpy (savecond, cond_name);
    sprintf (cond_name, "ERROR: Unrecognized condition type in %s", savecond);
    return;
    }

  for (new_inx=0, found=FALSE; new_inx<22 && !found; new_inx++)
    if (strncmp (condtypes[new_inx], new_type, strlen (new_type)) == 0) found=TRUE;
  new_inx--;
  if (!found)
    {
    sprintf (cond_name, "ERROR: Unrecognized condition type: %s", new_type);
    return;
    }

  for (cond_num=0, found=FALSE; cond_num<100 && !found; cond_num++)
    if (cond_map[cond_num][0] == cond_inx && cond_map[cond_num][1] == old_num) found=TRUE;
  cond_num--;
  if (!found)
    {
    strcpy (savecond, cond_name);
    sprintf (cond_name, "ERROR: Condition %s not found", savecond);
    return;
    }

  for (i=0, num=1; i<cond_num; i++)
    if (cond_map[i][0] == new_inx) num++;
  for (; i<100; i++)
    if (cond_map[i][0] == new_inx) cond_map[i][1]++;
  cond_map[cond_num][0]=new_inx;
  cond_map[cond_num][1]=num;
  PTSetCondname (cond_num, cond_name);
}
