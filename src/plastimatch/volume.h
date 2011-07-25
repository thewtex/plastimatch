/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _volume_h_
#define _volume_h_

#include "plm_config.h"
#include "volume_macros.h"

enum Volume_pixel_type {
    PT_UNDEFINED,
    PT_UCHAR,
    PT_SHORT,
    PT_UINT16,
    PT_UINT32,
    PT_FLOAT,
    PT_VF_FLOAT_INTERLEAVED,
    PT_VF_FLOAT_PLANAR,
    PT_UCHAR_VEC_INTERLEAVED
};

class gpuit_EXPORT Volume
{
public:
    int dim[3];		        // x, y, z Dims
    int npix;			// # of voxels in volume
				// = dim[0] * dim[1] * dim[2] 
    float offset[3];
    float spacing[3];
    float direction_cosines[9];
    float inverse_direction_cosines[9];
    float step[3][3];           // direction_cosines * spacing
    float proj[3][3];           // inv direction_cosines / spacing

    enum Volume_pixel_type pix_type;	// Voxel Data type
    int vox_planes;                     // # planes per voxel
    int pix_size;		        // # bytes per voxel
    void* img;			        // Voxel Data
public:
    Volume () {
	init ();
    }
    Volume (
	const int dim[3], 
	const float offset[3], 
	const float spacing[3], 
	const float direction_cosines[9], 
	enum Volume_pixel_type vox_type, 
	int vox_planes, 
	int min_size
    ) {
	init ();
	create (dim, offset, spacing, direction_cosines, vox_type, 
	    vox_planes, min_size);
    }
    ~Volume ();
public:
    void init () {
	for (int d = 0; d < 3; d++) {
	    dim[d] = 0;
	    offset[d] = 0;
	    spacing[d] = 0;
	}
	for (int d = 0; d < 9; d++) {
	    direction_cosines[d] = 0;
	    inverse_direction_cosines[d] = 0;
	}
	for (int i = 0; i < 3; i++) {
	    for (int j = 0; j < 3; j++) {
		proj[i][j] = 0;
		step[i][j] = 0;
	    }
	}
	npix = 0;
	pix_type = PT_UNDEFINED;
	vox_planes = 0;
	pix_size = 0;
	img = 0;
    }
    void create (
	const int dim[3], 
	const float offset[3], 
	const float spacing[3], 
	const float direction_cosines[9], 
	enum Volume_pixel_type vox_type, 
	int vox_planes, 
	int min_size
    );
    void set_direction_cosines (const float direction_cosines[9]);
};

#if defined __cplusplus
extern "C" {
#endif

gpuit_EXPORT
void volume_convert_to_float (Volume* ref);
gpuit_EXPORT
void volume_convert_to_short (Volume* ref);
gpuit_EXPORT
void
volume_convert_to_uint16 (Volume* ref);
gpuit_EXPORT
void
volume_convert_to_uint32 (Volume* ref);
gpuit_EXPORT
void vf_convert_to_interleaved (Volume* ref);
void vf_convert_to_planar (Volume* ref, int min_size);
void vf_pad_planar (Volume* vol, int size);
gpuit_EXPORT
Volume* volume_clone_empty (Volume* ref);
gpuit_EXPORT
Volume* volume_clone (Volume* ref);
gpuit_EXPORT
Volume* volume_make_gradient (Volume* ref);
Volume* volume_difference (Volume* vol, Volume* warped);
gpuit_EXPORT
Volume* volume_warp (Volume* vout, Volume* vin, Volume* vf);
gpuit_EXPORT
Volume* volume_resample (Volume* vol_in, int* dim, float* offset, float* spacing);
gpuit_EXPORT
Volume* volume_subsample (Volume* vol_in, int* sampling_rate);
gpuit_EXPORT
void volume_scale (Volume *vol, float scale);

gpuit_EXPORT
void volume_matrix3x3inverse(float *out, float *m);
gpuit_EXPORT
void
directions_cosine_debug (float *m);

#if defined __cplusplus
}
#endif

#endif
