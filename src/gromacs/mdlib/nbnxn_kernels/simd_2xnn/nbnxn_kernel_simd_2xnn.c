/*
 * This file is part of the GROMACS molecular simulation package.
 *
 * Copyright (c) 2012,2013,2014, by the GROMACS development team, led by
 * Mark Abraham, David van der Spoel, Berk Hess, and Erik Lindahl,
 * and including many others, as listed in the AUTHORS file in the
 * top-level source directory and at http://www.gromacs.org.
 *
 * GROMACS is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * GROMACS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GROMACS; if not, see
 * http://www.gnu.org/licenses, or write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *
 * If you want to redistribute modifications to GROMACS, please
 * consider that scientific software is very special. Version
 * control is crucial - bugs must be traceable. We will be happy to
 * consider code for inclusion in the official distribution, but
 * derived work must not be called official GROMACS. Details are found
 * in the README & COPYING files - if they are missing, get the
 * official version at http://www.gromacs.org.
 *
 * To help us fund GROMACS development, we humbly ask that you cite
 * the research papers on the package. Check out http://www.gromacs.org.
 */
/*
 * Note: this file was generated by the Verlet kernel generator for
 * kernel type 2xnn.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "typedefs.h"

#ifdef GMX_NBNXN_SIMD_2XNN

/* Include the full-width SIMD macros */

#include "gromacs/simd/macros.h"
#include "gromacs/simd/vector_operations.h"

#if !(GMX_SIMD_REAL_WIDTH == 8 || GMX_SIMD_REAL_WIDTH == 16)
#error "unsupported SIMD width"
#endif

#define GMX_SIMD_J_UNROLL_SIZE 2
#include "nbnxn_kernel_simd_2xnn.h"
#include "../nbnxn_kernel_common.h"
#include "gmx_omp_nthreads.h"
#include "types/force_flags.h"
#include "gmx_fatal.h"

/*! \brief Kinds of electrostatic treatments in SIMD Verlet kernels
 */
enum {
    coultRF, coultTAB, coultTAB_TWIN, coultEWALD, coultEWALD_TWIN, coultNR
};

/* Declare and define the kernel function pointer lookup tables.
 * The minor index of the array goes over both the LJ combination rules,
 * which is only supported by plain cut-off, and the LJ switch functions.
 */
static p_nbk_func_noener p_nbk_noener[coultNR][ljcrNR+2] =
{
    {
        nbnxn_kernel_ElecRF_VdwLJCombGeom_F_2xnn,
        nbnxn_kernel_ElecRF_VdwLJCombLB_F_2xnn,
        nbnxn_kernel_ElecRF_VdwLJ_F_2xnn,
        nbnxn_kernel_ElecRF_VdwLJFSw_F_2xnn,
        nbnxn_kernel_ElecRF_VdwLJPSw_F_2xnn,
    },
    {
        nbnxn_kernel_ElecQSTab_VdwLJCombGeom_F_2xnn,
        nbnxn_kernel_ElecQSTab_VdwLJCombLB_F_2xnn,
        nbnxn_kernel_ElecQSTab_VdwLJ_F_2xnn,
        nbnxn_kernel_ElecQSTab_VdwLJFSw_F_2xnn,
        nbnxn_kernel_ElecQSTab_VdwLJPSw_F_2xnn,
    },
    {
        nbnxn_kernel_ElecQSTabTwinCut_VdwLJCombGeom_F_2xnn,
        nbnxn_kernel_ElecQSTabTwinCut_VdwLJCombLB_F_2xnn,
        nbnxn_kernel_ElecQSTabTwinCut_VdwLJ_F_2xnn,
        nbnxn_kernel_ElecQSTabTwinCut_VdwLJFSw_F_2xnn,
        nbnxn_kernel_ElecQSTabTwinCut_VdwLJPSw_F_2xnn,
    },
    {
        nbnxn_kernel_ElecEw_VdwLJCombGeom_F_2xnn,
        nbnxn_kernel_ElecEw_VdwLJCombLB_F_2xnn,
        nbnxn_kernel_ElecEw_VdwLJ_F_2xnn,
        nbnxn_kernel_ElecEw_VdwLJFSw_F_2xnn,
        nbnxn_kernel_ElecEw_VdwLJPSw_F_2xnn,
    },
    {
        nbnxn_kernel_ElecEwTwinCut_VdwLJCombGeom_F_2xnn,
        nbnxn_kernel_ElecEwTwinCut_VdwLJCombLB_F_2xnn,
        nbnxn_kernel_ElecEwTwinCut_VdwLJ_F_2xnn,
        nbnxn_kernel_ElecEwTwinCut_VdwLJFSw_F_2xnn,
        nbnxn_kernel_ElecEwTwinCut_VdwLJPSw_F_2xnn,
    },
};

static p_nbk_func_ener p_nbk_ener[coultNR][ljcrNR+2] =
{
    {
        nbnxn_kernel_ElecRF_VdwLJCombGeom_VF_2xnn,
        nbnxn_kernel_ElecRF_VdwLJCombLB_VF_2xnn,
        nbnxn_kernel_ElecRF_VdwLJ_VF_2xnn,
        nbnxn_kernel_ElecRF_VdwLJFSw_VF_2xnn,
        nbnxn_kernel_ElecRF_VdwLJPSw_VF_2xnn,
    },
    {
        nbnxn_kernel_ElecQSTab_VdwLJCombGeom_VF_2xnn,
        nbnxn_kernel_ElecQSTab_VdwLJCombLB_VF_2xnn,
        nbnxn_kernel_ElecQSTab_VdwLJ_VF_2xnn,
        nbnxn_kernel_ElecQSTab_VdwLJFSw_VF_2xnn,
        nbnxn_kernel_ElecQSTab_VdwLJPSw_VF_2xnn,
    },
    {
        nbnxn_kernel_ElecQSTabTwinCut_VdwLJCombGeom_VF_2xnn,
        nbnxn_kernel_ElecQSTabTwinCut_VdwLJCombLB_VF_2xnn,
        nbnxn_kernel_ElecQSTabTwinCut_VdwLJ_VF_2xnn,
        nbnxn_kernel_ElecQSTabTwinCut_VdwLJFSw_VF_2xnn,
        nbnxn_kernel_ElecQSTabTwinCut_VdwLJPSw_VF_2xnn,
    },
    {
        nbnxn_kernel_ElecEw_VdwLJCombGeom_VF_2xnn,
        nbnxn_kernel_ElecEw_VdwLJCombLB_VF_2xnn,
        nbnxn_kernel_ElecEw_VdwLJ_VF_2xnn,
        nbnxn_kernel_ElecEw_VdwLJFSw_VF_2xnn,
        nbnxn_kernel_ElecEw_VdwLJPSw_VF_2xnn,
    },
    {
        nbnxn_kernel_ElecEwTwinCut_VdwLJCombGeom_VF_2xnn,
        nbnxn_kernel_ElecEwTwinCut_VdwLJCombLB_VF_2xnn,
        nbnxn_kernel_ElecEwTwinCut_VdwLJ_VF_2xnn,
        nbnxn_kernel_ElecEwTwinCut_VdwLJFSw_VF_2xnn,
        nbnxn_kernel_ElecEwTwinCut_VdwLJPSw_VF_2xnn,
    },
};

static p_nbk_func_ener p_nbk_energrp[coultNR][ljcrNR+2] =
{
    {
        nbnxn_kernel_ElecRF_VdwLJCombGeom_VgrpF_2xnn,
        nbnxn_kernel_ElecRF_VdwLJCombLB_VgrpF_2xnn,
        nbnxn_kernel_ElecRF_VdwLJ_VgrpF_2xnn,
        nbnxn_kernel_ElecRF_VdwLJFSw_VgrpF_2xnn,
        nbnxn_kernel_ElecRF_VdwLJPSw_VgrpF_2xnn,
    },
    {
        nbnxn_kernel_ElecQSTab_VdwLJCombGeom_VgrpF_2xnn,
        nbnxn_kernel_ElecQSTab_VdwLJCombLB_VgrpF_2xnn,
        nbnxn_kernel_ElecQSTab_VdwLJ_VgrpF_2xnn,
        nbnxn_kernel_ElecQSTab_VdwLJFSw_VgrpF_2xnn,
        nbnxn_kernel_ElecQSTab_VdwLJPSw_VgrpF_2xnn,
    },
    {
        nbnxn_kernel_ElecQSTabTwinCut_VdwLJCombGeom_VgrpF_2xnn,
        nbnxn_kernel_ElecQSTabTwinCut_VdwLJCombLB_VgrpF_2xnn,
        nbnxn_kernel_ElecQSTabTwinCut_VdwLJ_VgrpF_2xnn,
        nbnxn_kernel_ElecQSTabTwinCut_VdwLJFSw_VgrpF_2xnn,
        nbnxn_kernel_ElecQSTabTwinCut_VdwLJPSw_VgrpF_2xnn,
    },
    {
        nbnxn_kernel_ElecEw_VdwLJCombGeom_VgrpF_2xnn,
        nbnxn_kernel_ElecEw_VdwLJCombLB_VgrpF_2xnn,
        nbnxn_kernel_ElecEw_VdwLJ_VgrpF_2xnn,
        nbnxn_kernel_ElecEw_VdwLJFSw_VgrpF_2xnn,
        nbnxn_kernel_ElecEw_VdwLJPSw_VgrpF_2xnn,
    },
    {
        nbnxn_kernel_ElecEwTwinCut_VdwLJCombGeom_VgrpF_2xnn,
        nbnxn_kernel_ElecEwTwinCut_VdwLJCombLB_VgrpF_2xnn,
        nbnxn_kernel_ElecEwTwinCut_VdwLJ_VgrpF_2xnn,
        nbnxn_kernel_ElecEwTwinCut_VdwLJFSw_VgrpF_2xnn,
        nbnxn_kernel_ElecEwTwinCut_VdwLJPSw_VgrpF_2xnn,
    },
};


static void
reduce_group_energies(int ng, int ng_2log,
                      const real *VSvdw, const real *VSc,
                      real *Vvdw, real *Vc)
{
    const int unrollj      = GMX_SIMD_REAL_WIDTH/GMX_SIMD_J_UNROLL_SIZE;
    const int unrollj_half = unrollj/2;
    int       ng_p2, i, j, j0, j1, c, s;

    ng_p2 = (1<<ng_2log);

    /* The size of the x86 SIMD energy group buffer array is:
     * ng*ng*ng_p2*unrollj_half*simd_width
     */
    for (i = 0; i < ng; i++)
    {
        for (j = 0; j < ng; j++)
        {
            Vvdw[i*ng+j] = 0;
            Vc[i*ng+j]   = 0;
        }

        for (j1 = 0; j1 < ng; j1++)
        {
            for (j0 = 0; j0 < ng; j0++)
            {
                c = ((i*ng + j1)*ng_p2 + j0)*unrollj_half*unrollj;
                for (s = 0; s < unrollj_half; s++)
                {
                    Vvdw[i*ng+j0] += VSvdw[c+0];
                    Vvdw[i*ng+j1] += VSvdw[c+1];
                    Vc  [i*ng+j0] += VSc  [c+0];
                    Vc  [i*ng+j1] += VSc  [c+1];
                    c             += unrollj + 2;
                }
            }
        }
    }
}

#else /* GMX_NBNXN_SIMD_2XNN */

#include "gmx_fatal.h"

#endif /* GMX_NBNXN_SIMD_2XNN */

void
nbnxn_kernel_simd_2xnn(nbnxn_pairlist_set_t      gmx_unused *nbl_list,
                       const nbnxn_atomdata_t    gmx_unused *nbat,
                       const interaction_const_t gmx_unused *ic,
                       int                       gmx_unused  ewald_excl,
                       rvec                      gmx_unused *shift_vec,
                       int                       gmx_unused  force_flags,
                       int                       gmx_unused  clearF,
                       real                      gmx_unused *fshift,
                       real                      gmx_unused *Vc,
                       real                      gmx_unused *Vvdw)
#ifdef GMX_NBNXN_SIMD_2XNN
{
    int                nnbl;
    nbnxn_pairlist_t **nbl;
    int                coult, ljtreatment = 0;
    int                nb;

    nnbl = nbl_list->nnbl;
    nbl  = nbl_list->nbl;

    if (EEL_RF(ic->eeltype) || ic->eeltype == eelCUT)
    {
        coult = coultRF;
    }
    else
    {
        if (ewald_excl == ewaldexclTable)
        {
            if (ic->rcoulomb == ic->rvdw)
            {
                coult = coultTAB;
            }
            else
            {
                coult = coultTAB_TWIN;
            }
        }
        else
        {
            if (ic->rcoulomb == ic->rvdw)
            {
                coult = coultEWALD;
            }
            else
            {
                coult = coultEWALD_TWIN;
            }
        }
    }

    switch (ic->vdw_modifier)
    {
        case eintmodNONE:
        case eintmodPOTSHIFT:
            ljtreatment = nbat->comb_rule;
            break;
        /* Switch functions follow after cut-off combination rule kernels */
        case eintmodFORCESWITCH:
            ljtreatment = ljcrNR;
            break;
        case eintmodPOTSWITCH:
            ljtreatment = ljcrNR + 1;
            break;
        default:
            gmx_incons("Unsupported VdW interaction modifier");
    }

#pragma omp parallel for schedule(static) num_threads(gmx_omp_nthreads_get(emntNonbonded))
    for (nb = 0; nb < nnbl; nb++)
    {
        nbnxn_atomdata_output_t *out;
        real                    *fshift_p;

        out = &nbat->out[nb];

        if (clearF == enbvClearFYes)
        {
            clear_f(nbat, nb, out->f);
        }

        if ((force_flags & GMX_FORCE_VIRIAL) && nnbl == 1)
        {
            fshift_p = fshift;
        }
        else
        {
            fshift_p = out->fshift;

            if (clearF == enbvClearFYes)
            {
                clear_fshift(fshift_p);
            }
        }

        if (!(force_flags & GMX_FORCE_ENERGY))
        {
            /* Don't calculate energies */
            p_nbk_noener[coult][ljtreatment](nbl[nb], nbat,
                                             ic,
                                             shift_vec,
                                             out->f,
                                             fshift_p);
        }
        else if (out->nV == 1)
        {
            /* No energy groups */
            out->Vvdw[0] = 0;
            out->Vc[0]   = 0;

            p_nbk_ener[coult][ljtreatment](nbl[nb], nbat,
                                           ic,
                                           shift_vec,
                                           out->f,
                                           fshift_p,
                                           out->Vvdw,
                                           out->Vc);
        }
        else
        {
            /* Calculate energy group contributions */
            int i;

            for (i = 0; i < out->nVS; i++)
            {
                out->VSvdw[i] = 0;
            }
            for (i = 0; i < out->nVS; i++)
            {
                out->VSc[i] = 0;
            }

            p_nbk_energrp[coult][ljtreatment](nbl[nb], nbat,
                                              ic,
                                              shift_vec,
                                              out->f,
                                              fshift_p,
                                              out->VSvdw,
                                              out->VSc);

            reduce_group_energies(nbat->nenergrp, nbat->neg_2log,
                                  out->VSvdw, out->VSc,
                                  out->Vvdw, out->Vc);
        }
    }

    if (force_flags & GMX_FORCE_ENERGY)
    {
        reduce_energies_over_lists(nbat, nnbl, Vvdw, Vc);
    }
}
#else
{
    gmx_incons("nbnxn_kernel_simd_2xnn called when such kernels "
               " are not enabled.");
}
#endif
#undef GMX_SIMD_J_UNROLL_SIZE
