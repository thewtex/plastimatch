/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "plm_config.h"
#include "volume.h"
#include "readmha.h"
#include "bspline_opts.h"
#include "bspline.h"
#if defined (HAVE_F2C_LIBRARY)
#include "bspline_optimize_lbfgsb.h"
#endif

int
main (int argc, char* argv[])
{
    BSPLINE_Options options;
    BSPLINE_Parms* parms = &options.parms;
    BSPLINE_Xform bxf;
    Volume *moving, *fixed, *moving_grad;
    Volume *vector_field;
    Volume *moving_warped;
    int roi_offset[3];

    if (argc != 2) {
	printf ("Error, no argument\n");
	return -1;
    }


#if defined (commentout)
    parse_args (&options, argc, argv);

    fixed = read_mha (options. fixed_fn);
    if (!fixed) exit (-1);
    moving = read_mha (options. moving_fn);
    if (!moving) exit (-1);

    volume_convert_to_float (moving);
    volume_convert_to_float (fixed);

    printf ("Making gradient\n");
    moving_grad = volume_make_gradient (moving);

    /* Debug */
    //write_mha ("moving_grad.mha", moving_grad);

    /* Allocate memory and build lookup tables */
    printf ("Allocating lookup tables\n");
    memset (roi_offset, 0, 3*sizeof(int));
    bspline_xform_initialize (&bxf,
			      fixed->offset,
			      fixed->pix_spacing,
			      fixed->dim,
			      roi_offset,
			      fixed->dim,
			      options.vox_per_rgn
			      );

    /* Run the optimization */
    printf ("Running optimization.\n");
    bspline_optimize (&bxf, parms, fixed, moving, moving_grad);

    /* Create vector field from bspline coefficients and save */
    vector_field = volume_create (fixed->dim, fixed->offset, fixed->pix_spacing,
	PT_VF_FLOAT_INTERLEAVED, fixed->direction_cosines, 0);
    bspline_interpolate_vf (vector_field, &bxf);
    write_mha (options.output_fn, vector_field);

    /* Create warped output image and save */
    moving_warped = volume_warp (0, moving, vector_field);
    write_mha ("warped.mha", moving_warped);

    /* Free memory */
    bspline_parms_free (parms);
    bspline_xform_free (&bxf);
    volume_free (fixed);
    volume_free (moving);
    volume_free (moving_grad);
    volume_free (moving_warped);
    volume_free (vector_field);
#endif

    return 0;
}
