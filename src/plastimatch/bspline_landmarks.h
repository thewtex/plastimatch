/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_landmarks_h_
#define _bspline_landmarks_h_

#include "plm_config.h"
#include "pointset.h"
#include "volume.h"

typedef struct bspline_landmarks Bspline_landmarks;
struct bspline_landmarks {
    int num_landmarks;
    
    Pointset_old *fixed_landmarks;
    Pointset_old *moving_landmarks;
    float *warped_landmarks; //moving landmarks displaced by current vector field
    int *landvox_mov;
    int *landvox_fix;
    int *landvox_warp;
    float *rbf_coeff;
    float *landmark_dxyz; //temporary array used in RBF
};

#if defined __cplusplus
extern "C" {
#endif

gpuit_EXPORT
Bspline_landmarks*
bspline_landmarks_load (char *fixed_fn, char *moving_fn);

gpuit_EXPORT
void
bspline_landmarks_adjust (Bspline_landmarks *blm, Volume *fixed, Volume *moving);

void
bspline_landmarks_score (
    Bspline_parms *parms, 
    Bspline_state *bst, 
    Bspline_xform *bxf, 
    Volume *fixed, 
    Volume *moving
);

gpuit_EXPORT
void bspline_landmarks_warp (
	Volume *vector_field, 
	Bspline_parms *parms,
	Bspline_xform* bxf, 
    Volume *fixed, 
    Volume *moving);

gpuit_EXPORT
void bspline_landmarks_write_file( char *fn, char *title, float *coords, int n);

#if defined __cplusplus
}
#endif

#endif
