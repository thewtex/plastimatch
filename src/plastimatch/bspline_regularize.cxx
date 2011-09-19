/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"

#include "bspline.h"
#include "bspline_regularize_analytic.h"
#include "bspline_regularize_numeric.h"
#include "bspline_xform.h"
#include "print_and_exit.h"

void
bspline_regularize_initialize (
    Reg_parms* reg_parms,
    Bspline_regularize_state* rst,
    Bspline_xform* bxf
)
{
    switch (reg_parms->implementation) {
    case 'a':
        print_and_exit (
            "Sorry, regularization implementation (%c) is currently unavailable.\n",
            reg_parms->implementation
        );
        break;
    case 'b':
    case 'c':
        vf_regularize_analytic_init (rst, bxf);
        break;
    case 'd':
	bspline_regularize_numeric_init (rst, bxf);
        break;
    default:
        print_and_exit (
            "Error: unknown reg_parms->implementation (%c)\n",
            reg_parms->implementation
        );
        break;
    }
}

void
bspline_regularize_destroy (
    Reg_parms* reg_parms,
    Bspline_regularize_state* rst,
    Bspline_xform* bxf
)
{
    switch (reg_parms->implementation) {
    case 'a':
        print_and_exit (
            "Sorry, regularization implementation (%c) is currently unavailable.\n",
            reg_parms->implementation
        );
        break;
    case 'b':
    case 'c':
        vf_regularize_analytic_destroy (rst);
        break;
    case 'd':
	bspline_regularize_numeric_destroy (rst, bxf);
        break;
    default:
        print_and_exit (
            "Error: unknown reg_parms->implementation (%c)\n",
            reg_parms->implementation
        );
        break;
    }
}

void
bspline_regularize (
    Bspline_score *bspline_score,    /* Gets updated */
    Bspline_regularize_state* rst,
    const Reg_parms* reg_parms,
    const Bspline_xform* bxf
)
{
    switch (reg_parms->implementation) {
    case 'a':
//        S = vf_regularize_numerical (compute_vf_from_coeff (bxf));
        print_and_exit (
            "Sorry, regularization implementation (%c) is currently unavailable.\n",
            reg_parms->implementation
        );
        break;
    case 'b':
        vf_regularize_analytic (bspline_score, reg_parms, rst, bxf);
        break;
    case 'c':
#if (OPENMP_FOUND)
        vf_regularize_analytic_omp (bspline_score, reg_parms, rst, bxf);
#else
        vf_regularize_analytic (bspline_score, reg_parms, rst, bxf);
#endif
        break;
    case 'd':
        bspline_regularize_score (bspline_score, reg_parms, rst, bxf);
        break;
    default:
        print_and_exit (
            "Error: unknown reg_parms->implementation (%c)\n",
            reg_parms->implementation
        );
        break;
    }
}
