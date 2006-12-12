/******* ks_multicg_qop_P.c - multi-mass CG for SU3/fermions ****/
/* MIMD version 7 */

/* This is the MILC wrapper for the SciDAC Level 3 QOP inverter */

/* NOTE: This code is actually an include file for ks_multicg_qop_F.c
   and ks_multicg_qop_D.c, so any edits should be consistent with this
   purpose. */

/* Entry points (must be redefined to precision-specific names)

   KS_MULTICG_OFFSET
   KS_MULTICG_MASS 

   Externals (must be redefined to precision-specific names)

   KS_CONGRAD_QOP_SITE2SITE
   KS_CONGRAD_QOP_SITE2FIELD   

*/


/* 6/2006 C. DeTar created */

/*
 * $Log: ks_multicg_offset_qop_P.c,v $
 * Revision 1.1  2006/12/09 13:52:40  detar
 * Add mixed precision capability for KS inverter in QOP and QDP
 *
 * Revision 1.3  2006/11/04 23:41:21  detar
 * Add QOP and QDP support for FN fermion links
 * Create QDP version of fermion_links_fn_multi
 * Add nrestart parameter for ks_congrad
 *
 * Revision 1.2  2006/08/22 21:16:40  detar
 * Move functionality from ks_multicg_qop.c to ks_multicg_offset_qop.c
 * Remove extraneous globals from ks_multicg_offset.c
 * Remove make rule for ks_multicg_qop.c from Make_template
 *
 * Revision 1.1  2006/08/13 15:01:50  detar
 * Realign procedures to accommodate ks_imp_rhmc code
 * Add Level 3 wrappers and MILC dummy Level 3 implementation
 *
 */

#if ( QOP_Precision == 1 )

#define KS_MULTICG_MASS           ks_multicg_mass_F
#define KS_MULTICG_OFFSET         ks_multicg_offset_F
#define KS_CONGRAD_QOP_SITE2SITE  ks_congrad_qop_F_site2site
#define KS_CONGRAD_QOP_SITE2FIELD ks_congrad_qop_F_site2field
#define MYREAL                    float

#else

#define KS_MULTICG_MASS           ks_multicg_mass_D
#define KS_MULTICG_OFFSET         ks_multicg_offset_D
#define KS_CONGRAD_QOP_SITE2SITE  ks_congrad_qop_F_site2site
#define KS_CONGRAD_QOP_SITE2FIELD ks_congrad_qop_D_site2field
#define MYREAL                    double

#endif

#include "generic_ks_includes.h"
#include "../include/generic_ks_qop.h"
#include "../include/loopend.h"

static char* cvsHeader = "$Header: /lqcdproj/detar/cvsroot/milc_qcd/generic_ks/ks_multicg_offset_qop_P.c,v 1.1 2006/12/09 13:52:40 detar Exp $";

/* Standard MILC interface for the Asqtad multimass inverter 
   single source, multiple masses.  Uses the prevailing precision */

/* Offsets are 4 * mass * mass and must be positive */
int KS_MULTICG_OFFSET(	/* Return value is number of iterations taken */
    field_offset src,	/* source vector (type su3_vector) */
    su3_vector **psim,	/* solution vectors */
    Real *offsets,	/* the offsets */
    int num_offsets,	/* number of offsets */
    int niter,		/* maximal number of CG interations */
    Real rsqmin,	/* desired residue squared */
    int parity,		/* parity to be worked on */
    Real *final_rsq_ptr	/* final residue squared */
    )
{
  int num_masses = num_offsets;
  int i,j;
  MYREAL *masses;
  int iterations_used;
  int nrestart = 1;
  MYREAL *masses2[1];
  int nmass[1], nsrc;
  field_offset milc_srcs[1];
  su3_vector **milc_sols[1];
  char myname[] = "ks_multicg_offset";

  /* Generate mass table */
  masses = (MYREAL *)malloc(sizeof(MYREAL)*num_masses);
  if(masses == NULL){
    printf("%s(%d): No room for masses\n",myname,this_node);
    terminate(1);
  }
  for(i = 0; i < num_masses; i++){
    if(offsets[i] < 0){
      printf("%s(%d): called with negative offset %e\n",
	     myname, this_node,offsets[i]);
      terminate(1);
    }
    masses[i] = sqrt(offsets[i]/4.0);
  }

  /* Set up general source and solution pointers for one mass, one source */
  nsrc = 1;
  milc_srcs[0] = src;

  nmass[0] = num_masses;
  masses2[0] = masses;

  /* Require zero initial guess for multicg */
  for(i = 0; i < num_masses; i++)
    for(j = 0; j < sites_on_node; j++)
      clearvec(psim[i]+j);
	
  /* Just set pointer for source 1 array of solutions */
  milc_sols[0] =  psim;

  iterations_used = KS_CONGRAD_QOP_SITE2FIELD( niter, nrestart, rsqmin, 
					       masses2, nmass, milc_srcs,
					       milc_sols, nsrc, final_rsq_ptr,
					       parity );

  free(masses);
  return iterations_used;
}


/* Standard MILC interface for the Asqtad multimass inverter 
   single source, multiple masses.  Uses the prevailing precision */

int KS_MULTICG_MASS(	/* Return value is number of iterations taken */
    field_offset src,	/* source vector (type su3_vector) */
    su3_vector **psim,	/* solution vectors (preallocated) */
    Real *masses,	/* the masses */
    int num_masses,	/* number of masses */
    int niter,		/* maximal number of CG interations */
    Real rsqmin,	/* desired residue squared */
    int parity,		/* parity to be worked on */
    Real *final_rsq_ptr	/* final residue squared */
    )
{

  int iterations_used;
  int nrestart = 1;
  MYREAL *masses2[1];
  int nmass[1], nsrc;
  int i;
  field_offset milc_srcs[1];
  su3_vector **milc_sols[1];

  /* Set up general source and solution pointers for one mass, one source */
  nsrc = 1;
  milc_srcs[0] = src;

  nmass[0] = num_masses;
  masses2[0] = (MYREAL *)malloc(num_masses*sizeof(MYREAL));
  for(i=0;i<num_masses;i++)
    masses2[0][i] = (MYREAL)masses[i];

  milc_sols[0] =  psim;

  iterations_used = KS_CONGRAD_QOP_SITE2FIELD( niter, nrestart, rsqmin, 
					       masses2, nmass, milc_srcs,
					       milc_sols, nsrc, final_rsq_ptr,
					       parity );

  free(masses2[0]);
  total_iters += iterations_used;
  return( iterations_used );
}

