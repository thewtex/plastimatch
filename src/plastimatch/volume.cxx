/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "math_util.h"
#include "plm_int.h"
#include "print_and_exit.h"
#include "volume.h"

#define CONVERT_VOLUME(old_type,new_type,new_type_enum)			\
    {									\
	int v;								\
	old_type* old_img;						\
	new_type* new_img;						\
	old_img = (old_type*) ref->img;					\
	new_img = (new_type*) malloc (sizeof(new_type) * ref->npix);	\
	if (!new_img) {							\
	    fprintf (stderr, "Memory allocation failed.\n");		\
	    exit(1);							\
	}								\
	for (v = 0; v < ref->npix; v++) {				\
	    new_img[v] = (new_type) old_img[v];				\
	}								\
	ref->pix_size = sizeof(new_type);				\
	ref->pix_type = new_type_enum;					\
	ref->img = (void*) new_img;					\
	free (old_img);							\
    }

void
directions_cosine_debug (float *m)
{
    printf ("%8f %8f %8f\n%8f %8f %8f\n%8f %8f %8f\n",
	m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8]);
}

void
Volume::allocate (void)
{
    if (this->pix_type == PT_VF_FLOAT_PLANAR) {
	int i;
	float** der = (float**) malloc (3*sizeof(float*));
	if (!der) {
	    fprintf (stderr, "Memory allocation failed.\n");
	    exit(1);
	}
	int alloc_size = this->npix;
	for (i=0; i < 3; i++) {
	    der[i] = (float*) malloc (alloc_size*sizeof(float));
	    if (!der[i]) {
		fprintf (stderr, "Memory allocation failed.\n");
		exit(1);
	    }
	    memset(der[i], 0, alloc_size*sizeof(float));
	}
	this->img = (void*) der;
    } else {
	this->img = (void*) malloc (this->pix_size * this->npix);
	if (!this->img) {
	    fprintf (stderr, "Memory allocation failed (alloc size = %d).\n",
		this->pix_size * this->npix);
	    exit(1);
	}
	memset (this->img, 0, this->pix_size * this->npix);
    }
}

Volume::~Volume ()
{
    if (this->pix_type == PT_VF_FLOAT_PLANAR) {
	float** planes = (float**) this->img;
	free (planes[0]);
	free (planes[1]);
	free (planes[2]);
    }
    free (this->img);
}

void 
Volume::create (
    const int dim[3], 
    const float offset[3], 
    const float spacing[3], 
    const float direction_cosines[9], 
    enum Volume_pixel_type vox_type, 
    int vox_planes
)
{
    int i;

    for (i = 0; i < 3; i++) {
	this->dim[i] = dim[i];
	this->offset[i] = offset[i];
	this->spacing[i] = spacing[i];
    }
    this->npix = this->dim[0] * this->dim[1] * this->dim[2];
    this->pix_type = vox_type;
    this->vox_planes = vox_planes;

    set_direction_cosines (direction_cosines);

    switch (vox_type) {
    case PT_UCHAR:
	this->pix_size = sizeof(unsigned char);
	break;
    case PT_SHORT:
	this->pix_size = sizeof(short);
	break;
    case PT_UINT16:
	this->pix_size = sizeof(uint16_t);
	break;
    case PT_UINT32:
	this->pix_size = sizeof(uint32_t);
	break;
    case PT_FLOAT:
	this->pix_size = sizeof(float);
	break;
    case PT_VF_FLOAT_INTERLEAVED:
	this->pix_size = 3 * sizeof(float);
	break;
    case PT_VF_FLOAT_PLANAR:
	this->pix_size = sizeof(float);
	break;
    case PT_UCHAR_VEC_INTERLEAVED:
	this->pix_size = this->vox_planes * sizeof(unsigned char);
	break;
    default:
	fprintf (stderr, "Unhandled type in volume_create().\n");
	exit (-1);
    }

    this->allocate ();
}

void 
Volume::create (
    const Volume_header& vh, 
    enum Volume_pixel_type vox_type, 
    int vox_planes
)
{
    this->create (vh.m_dim, vh.m_origin, vh.m_spacing, 
	vh.m_direction_cosines, vox_type, vox_planes);
}

void 
Volume::set_direction_cosines (
    const float direction_cosines[9]
)
{
    const float identity[9] = {1., 0., 0., 0., 1., 0., 0., 0., 1.};
    const float* dc;
    if (direction_cosines) {
	dc = direction_cosines;
    } else {
	dc = identity;
    }

    memcpy (this->direction_cosines, dc, sizeof(this->direction_cosines));

    // NSH version of step and proj
    // works ok for matrix, still needs testing for spacing
    volume_matrix3x3inverse (this->inverse_direction_cosines, 
	this->direction_cosines);

    for (int i = 0; i < 3; i++) {
	for (int j = 0; j < 3; j++) {
	    this->step[i][j] = this->direction_cosines[3*i+j] 
		* this->spacing[j];
	    this->proj[i][j] = this->inverse_direction_cosines[3*i+j] 
		/ this->spacing[i];
	}
    }

}

Volume*
volume_clone_empty (Volume* ref)
{
    Volume* vout;
    vout = new Volume (ref->dim, ref->offset, ref->spacing, 
	ref->direction_cosines, ref->pix_type, ref->vox_planes);
    return vout;
}

Volume*
volume_clone (Volume* ref)
{
    Volume* vout;
    vout = new Volume (ref->dim, ref->offset, ref->spacing, 
	ref->direction_cosines, ref->pix_type, ref->vox_planes);
    switch (ref->pix_type) {
    case PT_UCHAR:
    case PT_SHORT:
    case PT_UINT16:
    case PT_UINT32:
    case PT_FLOAT:
    case PT_VF_FLOAT_INTERLEAVED:
    case PT_UCHAR_VEC_INTERLEAVED:
	memcpy (vout->img, ref->img, ref->npix * ref->pix_size);
	break;
    case PT_VF_FLOAT_PLANAR:
    default:
	fprintf (stderr, "Unsupported clone\n");
	exit (-1);
	break;
    }
    return vout;
}

void
volume_convert_to_float (Volume* ref)
{
    switch (ref->pix_type) {
    case PT_UCHAR:
	CONVERT_VOLUME (unsigned char, float, PT_FLOAT);
	break;
    case PT_SHORT:
	CONVERT_VOLUME (short, float, PT_FLOAT);
	break;
    case PT_UINT16:
	CONVERT_VOLUME (uint16_t, float, PT_FLOAT);
	break;
    case PT_UINT32:
	CONVERT_VOLUME (uint32_t, float, PT_FLOAT);
	break;
    case PT_FLOAT:
	/* Nothing to do */
	break;
    case PT_VF_FLOAT_INTERLEAVED:
    case PT_VF_FLOAT_PLANAR:
    case PT_UCHAR_VEC_INTERLEAVED:
    default:
	/* Can't convert this */
	fprintf (stderr, "Sorry, unsupported conversion to FLOAT\n");
	exit (-1);
	break;
    }
}

void
volume_convert_to_short (Volume* ref)
{
    switch (ref->pix_type) {
    case PT_UCHAR:
	fprintf (stderr, "Sorry, UCHAR to SHORT is not implemented\n");
	exit (-1);
	break;
    case PT_SHORT:
	/* Nothing to do */
	break;
    case PT_UINT16:
    case PT_UINT32:
	fprintf (stderr, "Sorry, UINT16/UINT32 to SHORT is not implemented\n");
	exit (-1);
	break;
    case PT_FLOAT:
	CONVERT_VOLUME (float, short, PT_SHORT);
	break;
    case PT_VF_FLOAT_INTERLEAVED:
    case PT_VF_FLOAT_PLANAR:
    case PT_UCHAR_VEC_INTERLEAVED:
    default:
	/* Can't convert this */
	fprintf (stderr, "Sorry, unsupported conversion to SHORT\n");
	exit (-1);
	break;
    }
}

void
volume_convert_to_uint16 (Volume* ref)
{
    switch (ref->pix_type) {
    case PT_UCHAR:
    case PT_SHORT:
	fprintf (stderr, "Sorry, UCHAR/SHORT to UINT16 is not implemented\n");
	exit (-1);
	break;
    case PT_UINT16:
	/* Nothing to do */
	break;
    case PT_UINT32:
	fprintf (stderr, "Sorry, UINT32 to UINT16 is not implemented\n");
	break;
    case PT_FLOAT:
	CONVERT_VOLUME (float, uint16_t, PT_UINT32);
	break;
    case PT_VF_FLOAT_INTERLEAVED:
    case PT_VF_FLOAT_PLANAR:
    case PT_UCHAR_VEC_INTERLEAVED:
    default:
	/* Can't convert this */
	fprintf (stderr, "Sorry, unsupported conversion to UINT32\n");
	exit (-1);
	break;
    }
}

void
volume_convert_to_uint32 (Volume* ref)
{
    switch (ref->pix_type) {
    case PT_UCHAR:
    case PT_SHORT:
	fprintf (stderr, "Sorry, UCHAR/SHORT to UINT32 is not implemented\n");
	exit (-1);
	break;
    case PT_UINT16:
	fprintf (stderr, "Sorry, UINT16 to UINT32 is not implemented\n");
	exit (-1);
	break;
    case PT_UINT32:
	/* Nothing to do */
	break;
    case PT_FLOAT:
	CONVERT_VOLUME (float, uint32_t, PT_UINT32);
	break;
    case PT_VF_FLOAT_INTERLEAVED:
    case PT_VF_FLOAT_PLANAR:
    case PT_UCHAR_VEC_INTERLEAVED:
    default:
	/* Can't convert this */
	fprintf (stderr, "Sorry, unsupported conversion to UINT32\n");
	exit (-1);
	break;
    }
}

void
vf_convert_to_interleaved (Volume* vf)
{
    switch (vf->pix_type) {
    case PT_VF_FLOAT_INTERLEAVED:
	/* Nothing to do */
	break;
    case PT_VF_FLOAT_PLANAR:
	{
	    int v;
	    float** planar = (float**) vf->img;
	    float* inter = (float*) malloc (3*sizeof(float*)*vf->npix);
	    if (!inter) {
		fprintf (stderr, "Memory allocation failed.\n");
		exit(1);
	    }
	    for (v = 0; v < vf->npix; v++) {
		inter[3*v + 0] = planar[0][v];
		inter[3*v + 1] = planar[1][v];
		inter[3*v + 2] = planar[2][v];
	    }
	    free (planar[0]);
	    free (planar[1]);
	    free (planar[2]);
	    free (planar);
	    vf->img = (void*) inter;
	    vf->pix_type = PT_VF_FLOAT_INTERLEAVED;
	    vf->pix_size = 3*sizeof(float);
	}
	break;
    case PT_UCHAR:
    case PT_SHORT:
    case PT_UINT16:
    case PT_UINT32:
    case PT_FLOAT:
    case PT_UCHAR_VEC_INTERLEAVED:
    default:
	/* Can't convert this */
	fprintf (stderr, "Sorry, unsupported conversion to VF\n");
	exit (-1);
	break;
    }
}

void
vf_convert_to_planar (Volume* ref)
{
    switch (ref->pix_type) {
    case PT_VF_FLOAT_INTERLEAVED:
	{
	    int i;
	    float* img = (float*) ref->img;
	    float** der = (float**) malloc (3*sizeof(float*));
	    if (!der) {
		printf ("Memory allocation failed.\n");
		exit(1);
	    }
	    int alloc_size = ref->npix;
	    for (i=0; i < 3; i++) {
		der[i] = (float*) malloc (alloc_size*sizeof(float));
		if (!der[i]) {
		    print_and_exit ("Memory allocation failed.\n");
		}
	    }
	    for (i = 0; i < ref->npix; i++) {
		der[0][i] = img[3*i + 0];
		der[1][i] = img[3*i + 1];
		der[2][i] = img[3*i + 2];
	    }
	    free (ref->img);
	    ref->img = (void*) der;
	    ref->pix_type = PT_VF_FLOAT_PLANAR;
	    ref->pix_size = sizeof(float);
	}
        break;
    case PT_VF_FLOAT_PLANAR:
	/* Nothing to do */
	break;
    case PT_UCHAR:
    case PT_SHORT:
    case PT_UINT32:
    case PT_FLOAT:
    case PT_UCHAR_VEC_INTERLEAVED:
    default:
	/* Can't convert this */
	fprintf (stderr, "Sorry, unsupportd conversion to VF\n");
	exit (-1);
	break;
    }
}

/* Nearest neighbor interpolation */
static Volume*
volume_resample_float (Volume* vol_in, int* dim, float* offset, float* spacing)
{
    int i, j, k, v;
    float x, y, z;
    float x_in, y_in, z_in;
    int xidx, yidx, zidx;
    Volume* vol_out;
    float *in_img, *out_img;
    float val;
    float default_val = 0.0f;

    vol_out = new Volume (dim, offset, spacing, vol_in->direction_cosines, 
	PT_FLOAT, 1);
    in_img = (float*) vol_in->img;
    out_img = (float*) vol_out->img;

    for (k = 0, v = 0, z = offset[2]; k < dim[2]; k++, z += spacing[2]) {
	z_in = (z - vol_in->offset[2]) / vol_in->spacing[2];
	zidx = ROUND_INT (z_in);
	for (j = 0, y = offset[1]; j < dim[1]; j++, y += spacing[1]) {
	    y_in = (y - vol_in->offset[1]) / vol_in->spacing[1];
	    yidx = ROUND_INT (y_in);
	    for (i = 0, x = offset[0]; i < dim[0]; i++, x += spacing[0], v++) {
		x_in = (x - vol_in->offset[0]) / vol_in->spacing[0];
		xidx = ROUND_INT (x_in);
		if (zidx < 0 || zidx >= vol_in->dim[2] || yidx < 0 || yidx >= vol_in->dim[1] || xidx < 0 || xidx >= vol_in->dim[0]) {
		    val = default_val;
		} else {
		    int idx = zidx*vol_in->dim[1]*vol_in->dim[0] + yidx*vol_in->dim[0] + xidx;
		    val = in_img[idx];
		}
		out_img[v] = val;
	    }
	}
    }

    return vol_out;
}

/* Nearest neighbor interpolation */
static Volume*
volume_resample_vf_float_interleaved (Volume* vol_in, int* dim, 
				      float* offset, float* spacing)
{
    int d, i, j, k, v;
    float x, y, z;
    float x_in, y_in, z_in;
    int xidx, yidx, zidx;
    Volume* vol_out;
    float *in_img, *out_img;
    float* val;
    float default_val[3] = { 0.0f, 0.0f, 0.0f };

    vol_out = new Volume (dim, offset, spacing, vol_in->direction_cosines, 
	PT_VF_FLOAT_INTERLEAVED, 3);
    in_img = (float*) vol_in->img;
    out_img = (float*) vol_out->img;

    for (k = 0, v = 0, z = offset[2]; k < dim[2]; k++, z += spacing[2]) {
	z_in = (z - vol_in->offset[2]) / vol_in->spacing[2];
	zidx = ROUND_INT (z_in);
	for (j = 0, y = offset[1]; j < dim[1]; j++, y += spacing[1]) {
	    y_in = (y - vol_in->offset[1]) / vol_in->spacing[1];
	    yidx = ROUND_INT (y_in);
	    for (i = 0, x = offset[0]; i < dim[0]; i++, x += spacing[0]) {
		x_in = (x - vol_in->offset[0]) / vol_in->spacing[0];
		xidx = ROUND_INT (x_in);
		if (zidx < 0 || zidx >= vol_in->dim[2] || yidx < 0 || yidx >= vol_in->dim[1] || xidx < 0 || xidx >= vol_in->dim[0]) {
		    val = default_val;
		} else {
		    int idx = zidx*vol_in->dim[1]*vol_in->dim[0] + yidx*vol_in->dim[0] + xidx;
		    val = &in_img[idx*3];
		}
		for (d = 0; d < 3; d++, v++) {
		    out_img[v] = val[d];
		}
	    }
	}
    }

    return vol_out;
}

/* Nearest neighbor interpolation */
static Volume*
volume_resample_vf_float_planar (Volume* vol_in, int* dim, 
			      float* offset, float* spacing)
{
    int d, i, j, k, v;
    float x, y, z;
    float x_in, y_in, z_in;
    int xidx, yidx, zidx;
    Volume* vol_out;
    float **in_img, **out_img;

    vol_out = new Volume (dim, offset, spacing, vol_in->direction_cosines, 
	PT_VF_FLOAT_PLANAR, 3);
    in_img = (float**) vol_in->img;
    out_img = (float**) vol_out->img;

    for (k = 0, v = 0, z = offset[2]; k < dim[2]; k++, z += spacing[2]) {
	z_in = (z - vol_in->offset[2]) / vol_in->spacing[2];
	zidx = ROUND_INT (z_in);
	for (j = 0, y = offset[1]; j < dim[1]; j++, y += spacing[1]) {
	    y_in = (y - vol_in->offset[1]) / vol_in->spacing[1];
	    yidx = ROUND_INT (y_in);
	    for (i = 0, x = offset[0]; i < dim[0]; i++, x += spacing[0], v++) {
		x_in = (x - vol_in->offset[0]) / vol_in->spacing[0];
		xidx = ROUND_INT (x_in);
		if (zidx < 0 || zidx >= vol_in->dim[2] || yidx < 0 || yidx >= vol_in->dim[1] || xidx < 0 || xidx >= vol_in->dim[0]) {
		    for (d = 0; d < 3; d++) {
			out_img[d][v] = 0.0;		/* Default value */
		    }
		} else {
		    for (d = 0; d < 3; d++) {
			int idx = zidx*vol_in->dim[1]*vol_in->dim[0] + yidx*vol_in->dim[0] + xidx;
			out_img[d][v] = in_img[d][idx];
		    }
		}
	    }
	}
    }

    return vol_out;
}

Volume*
volume_resample (Volume* vol_in, int* dim, float* offset, float* spacing)
{
    switch (vol_in->pix_type) {
    case PT_UCHAR:
    case PT_SHORT:
    case PT_UINT32:
	fprintf (stderr, "Error, resampling PT_SHORT or PT_UCHAR is unsupported\n");
	return 0;
    case PT_FLOAT:
	return volume_resample_float (vol_in, dim, offset, spacing);
    case PT_VF_FLOAT_INTERLEAVED:
	return volume_resample_vf_float_interleaved (vol_in, dim, offset, spacing);
    case PT_VF_FLOAT_PLANAR:
	return volume_resample_vf_float_planar (vol_in, dim, offset, spacing);
    case PT_UCHAR_VEC_INTERLEAVED:
	fprintf (stderr, "Error, resampling PT_UCHAR_VEC_INTERLEAVED is unsupported\n");
	return 0;
    default:
	fprintf (stderr, "Error, unknown pix_type: %d\n", vol_in->pix_type);
	return 0;
    }
}

Volume*
volume_subsample (Volume* vol_in, int* sampling_rate)
{
    int d;
    int dim[3];
    float offset[3];
    float spacing[3];

    for (d = 0; d < 3; d++) {
	float in_size = vol_in->dim[d] * vol_in->spacing[d];

	dim[d] = vol_in->dim[d] / sampling_rate[d];
	if (dim[d] < 1) dim[d] = 1;
	spacing[d] = in_size / dim[d];
	offset[d] = (float) (vol_in->offset[d] - 0.5 * vol_in->spacing[d] 
	    + 0.5 * spacing[d]);
    }
    return volume_resample (vol_in, dim, offset, spacing);
}

void
volume_scale (Volume* vol, float scale)
{
    int i;
    float *img;

    if (vol->pix_type != PT_FLOAT) {
	print_and_exit ("volume_scale required PT_FLOAT type.\n");
    }

    img = (float*) vol->img;
    for (i = 0; i < vol->npix; i++) {
	img[i] = img[i] * scale;
    }
}

/* In mm coordinates */
void
volume_calc_grad (Volume* vout, Volume* vref)
{
    int v;
    int i_p, i, i_n, j_p, j, j_n, k_p, k, k_n; /* p is prev, n is next */
    int gi, gj, gk;
    int idx_p, idx_n;
    float *out_img, *ref_img;

    out_img = (float*) vout->img;
    ref_img = (float*) vref->img;

    v = 0;
    for (k = 0; k < vref->dim[2]; k++) {
	k_p = k - 1;
	k_n = k + 1;
	if (k == 0) k_p = 0;
	if (k == vref->dim[2]-1) k_n = vref->dim[2]-1;
	for (j = 0; j < vref->dim[1]; j++) {
	    j_p = j - 1;
	    j_n = j + 1;
	    if (j == 0) j_p = 0;
	    if (j == vref->dim[1]-1) j_n = vref->dim[1]-1;
	    for (i = 0; i < vref->dim[0]; i++, v++) {
		i_p = i - 1;
		i_n = i + 1;
		if (i == 0) i_p = 0;
		if (i == vref->dim[0]-1) i_n = vref->dim[0]-1;
		
		gi = 3 * v + 0;
		gj = 3 * v + 1;
		gk = 3 * v + 2;
		
		idx_p = volume_index (vref->dim, i_p, j, k);
		idx_n = volume_index (vref->dim, i_n, j, k);
		out_img[gi] = (float) (ref_img[idx_n] - ref_img[idx_p]) / 2.0 / vref->spacing[0];
		idx_p = volume_index (vref->dim, i, j_p, k);
		idx_n = volume_index (vref->dim, i, j_n, k);
		out_img[gj] = (float) (ref_img[idx_n] - ref_img[idx_p]) / 2.0 / vref->spacing[1];
		idx_p = volume_index (vref->dim, i, j, k_p);
		idx_n = volume_index (vref->dim, i, j, k_n);
		out_img[gk] = (float) (ref_img[idx_n] - ref_img[idx_p]) / 2.0 / vref->spacing[2];
	    }
	}
    }
}

Volume* 
volume_make_gradient (Volume* ref)
{
    Volume *grad;
    grad = new Volume (ref->dim, ref->offset, ref->spacing, 
	ref->direction_cosines, PT_VF_FLOAT_INTERLEAVED, 3);
    volume_calc_grad (grad, ref);

    return grad;
}

// Computes the intensity differences between two images
Volume*
volume_difference (Volume* vol, Volume* warped)
{
    int i, j, k;
    int p = 0; // Voxel index
    short* temp2;
    short* temp1;
    short* temp3;
    Volume* temp;

    temp = (Volume*) malloc (sizeof(Volume));
    if (!temp) {
	fprintf (stderr, "Memory allocation failed.\n");
	exit(1);
    }

    if(!temp){
	printf("Memory allocation failed for volume...Exiting\n");
	exit(1);
    }
	
    for(i=0;i<3; i++){
	temp->dim[i] = vol->dim[i];
	temp->offset[i] = vol->offset[i];
	temp->spacing[i] = vol->spacing[i];
    }

    temp->npix = vol->npix;
    temp->pix_type = vol->pix_type;

    temp->img = (void*) malloc (sizeof(short)*temp->npix);
    if (!temp->img) {
	fprintf (stderr, "Memory allocation failed.\n");
	exit(1);
    }
    memset (temp->img, -1200, sizeof(short)*temp->npix);

    p = 0; // Voxel index
    temp2 = (short*)vol->img;
    temp1 = (short*)warped->img;
    temp3 = (short*)temp->img;

    for (i=0; i < vol->dim[2]; i++) {
	for (j=0; j < vol->dim[1]; j++) {
	    for (k=0; k < vol->dim[0]; k++) {
		temp3[p] = (temp2[p] - temp1[p]) - 1200;
		p++;
	    }
	}
    }
    return temp;
}

void volume_matrix3x3inverse (float *out, float *m)
{
    float det;

    det =  
	m[0]*(m[4]*m[8]-m[5]*m[7])
	-m[1]*(m[3]*m[8]-m[5]*m[6])
	+m[2]*(m[3]*m[7]-m[4]*m[6]);

    if (fabs(det)<1e-8) {
	directions_cosine_debug (m);
	print_and_exit ("Error: singular matrix of direction cosines\n");
    }

    out[0]=  (m[4]*m[8]-m[5]*m[7]) / det;
    out[1]= -(m[1]*m[8]-m[2]*m[7]) / det;
    out[2]=  (m[1]*m[5]-m[2]*m[4]) / det;

    out[3]= -(m[3]*m[8]-m[5]*m[6]) / det;
    out[4]=  (m[0]*m[8]-m[2]*m[6]) / det;
    out[5]= -(m[0]*m[5]-m[2]*m[3]) / det;

    out[6]=  (m[3]*m[7]-m[4]*m[6]) / det;
    out[7]= -(m[0]*m[7]-m[1]*m[6]) / det;
    out[8]=  (m[0]*m[4]-m[1]*m[3]) / det;
}
