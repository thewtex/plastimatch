/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#if defined (_WIN32)
#include <windows.h>
#endif
#include "plm_config.h"
#include "volume.h"
#include "readmha.h"
#include "timer.h"
#include "bspline_optimize_lbfgsb.h"
#include "bspline_macros.h"
#include "bspline_opts.h"
#include "bspline.h"
#include "bspline_cuda.h"

#define CPU_HISTS
#define CPU_SCORE
//#define CPU_GRAD

/***********************************************************************
 * A few of the CPU functions are reproduced here for testing purposes.
 * Once the CPU code is removed from the functions below, these
 * functions can be deleted.
 ***********************************************************************/
#define round_int(x) ((x)>=0?(long)((x)+0.5):(long)(-(-(x)+0.5)))

#if defined (commentout)
void
bspline_interp_pix_b (
    float out[3], 
    BSPLINE_Xform* bxf, 
    int pidx, 
    int qidx
)
{
    int i, j, k, m;
    int cidx;
    float* q_lut = &bxf->q_lut[qidx*64];
    int* c_lut = &bxf->c_lut[pidx*64];

    out[0] = out[1] = out[2] = 0;
    m = 0;
    for (k = 0; k < 4; k++) {
	for (j = 0; j < 4; j++) {
	    for (i = 0; i < 4; i++) {
		cidx = 3 * c_lut[m];
		out[0] += q_lut[m] * bxf->coeff[cidx+0];
		out[1] += q_lut[m] * bxf->coeff[cidx+1];
		out[2] += q_lut[m] * bxf->coeff[cidx+2];
		m ++;
	    }
	}
    }
}

int
bspline_find_correspondence 
(
 float *mxyz,             /* Output: xyz coordinates in moving image (mm) */
 float *mijk,             /* Output: ijk indices in moving image (vox) */
 const float *fxyz,       /* Input:  xyz coordinates in fixed image (mm) */
 const float *dxyz,       /* Input:  displacement from fixed to moving (mm) */
 const Volume *moving     /* Input:  moving image */
 )
{
    mxyz[0] = fxyz[0] + dxyz[0];
    mijk[0] = (mxyz[0] - moving->offset[0]) / moving->pix_spacing[0];
    if (mijk[0] < -0.5 || mijk[0] > moving->dim[0] - 0.5) return 0;

    mxyz[1] = fxyz[1] + dxyz[1];
    mijk[1] = (mxyz[1] - moving->offset[1]) / moving->pix_spacing[1];
    if (mijk[1] < -0.5 || mijk[1] > moving->dim[1] - 0.5) return 0;

    mxyz[2] = fxyz[2] + dxyz[2];
    mijk[2] = (mxyz[2] - moving->offset[2]) / moving->pix_spacing[2];
    if (mijk[2] < -0.5 || mijk[2] > moving->dim[2] - 0.5) return 0;

    return 1;
}
#endif

static inline void
bspline_mi_hist_add_pvi_8 (
    BSPLINE_MI_Hist* mi_hist, 
    Volume *fixed, 
    Volume *moving, 
    int fv, 
    int mvf, 
    float li_1[3],           /* Fraction of interpolant in lower index */
    float li_2[3])           /* Fraction of interpolant in upper index */
{
    float w1, w2, w3, w4, w5, w6, w7, w8;
    int   n1, n2, n3, n4, n5, n6, n7, n8;
    int idx_fbin, idx_mbin, idx_jbin;
    int offset_fbin;
    float* f_img = (float*) fixed->img;
    float* m_img = (float*) moving->img;
#ifdef DOUBLE_HISTS
    double *f_hist = mi_hist->f_hist_d;
    double *m_hist = mi_hist->m_hist_d;
    double *j_hist = mi_hist->j_hist_d;
#else
    printf ("DOUBLE_HISTS must be enabled to use PV8.\n");
    exit(0);
#endif

    w1 = li_1[0] * li_1[1] * li_1[2];	// Partial Volume w1
    w2 = li_2[0] * li_1[1] * li_1[2];	// Partial Volume w2
    w3 = li_1[0] * li_2[1] * li_1[2];	// Partial Volume w3
    w4 = li_2[0] * li_2[1] * li_1[2];	// Partial Volume w4
    w5 = li_1[0] * li_1[1] * li_2[2];	// Partial Volume w5
    w6 = li_2[0] * li_1[1] * li_2[2];	// Partial Volume w6
    w7 = li_1[0] * li_2[1] * li_2[2];	// Partial Volume w7
    w8 = li_2[0] * li_2[1] * li_2[2];	// Partial Volume w8

    // Note that Sum(wN) for N within [1,8] should = 1 (checked OK)

    // Calculate Point Indices for 8 neighborhood
    n1 = mvf;
    n2 = n1 + 1;
    n3 = n1 + moving->dim[0];
    n4 = n1 + moving->dim[0] + 1;
    n5 = n1 + moving->dim[0]*moving->dim[1];
    n6 = n1 + moving->dim[0]*moving->dim[1] + 1;
    n7 = n1 + moving->dim[0]*moving->dim[1] + moving->dim[0];
    n8 = n1 + moving->dim[0]*moving->dim[1] + moving->dim[0] + 1;

    // Calculate fixed histogram bin and increment it
    idx_fbin = floor ((f_img[fv] - mi_hist->fixed.offset) / mi_hist->fixed.delta);
    f_hist[idx_fbin]++;

    offset_fbin = idx_fbin * mi_hist->moving.bins;

    // Add PV w1 to moving & joint histograms   
    idx_mbin = floor ((m_img[n1] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    m_hist[idx_mbin] += w1;
    j_hist[idx_jbin] += w1;

    // Add PV w2 to moving & joint histograms   
    idx_mbin = floor ((m_img[n2] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    m_hist[idx_mbin] += w2;
    j_hist[idx_jbin] += w2;

    // Add PV w3 to moving & joint histograms   
    idx_mbin = floor ((m_img[n3] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    m_hist[idx_mbin] += w3;
    j_hist[idx_jbin] += w3;

    // Add PV w4 to moving & joint histograms   
    idx_mbin = floor ((m_img[n4] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    m_hist[idx_mbin] += w4;
    j_hist[idx_jbin] += w4;

    // Add PV w5 to moving & joint histograms   
    idx_mbin = floor ((m_img[n5] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    m_hist[idx_mbin] += w5;
    j_hist[idx_jbin] += w5;
    
    // Add PV w6 to moving & joint histograms   
    idx_mbin = floor ((m_img[n6] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    m_hist[idx_mbin] += w6;
    j_hist[idx_jbin] += w6;

    // Add PV w7 to moving & joint histograms   
    idx_mbin = floor ((m_img[n7] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    m_hist[idx_mbin] += w7;
    j_hist[idx_jbin] += w7;
    
    // Add PV w8 to moving & joint histograms   
    idx_mbin = floor ((m_img[n8] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    m_hist[idx_mbin] += w8;
    j_hist[idx_jbin] += w8;

}

static inline void
bspline_mi_pvi_8_dc_dv (
    float dc_dv[3],                /* Output */
    BSPLINE_MI_Hist* mi_hist,      /* Input */
    Bspline_state *bst,            /* Input */
    Volume *fixed,                 /* Input */
    Volume *moving,                /* Input */
    int fv,                        /* Input */
    int mvf,                       /* Input */
    float mijk[3],                 /* Input */
    float num_vox_f,               /* Input */
    float li_1[3],                 /* Input */
    float li_2[3]                  /* Input */
)
{
    float dS_dP;
    float* f_img = (float*) fixed->img;
    float* m_img = (float*) moving->img;
    float* f_hist = mi_hist->f_hist;
    float* m_hist = mi_hist->m_hist;
    float* j_hist = mi_hist->j_hist;
    BSPLINE_Score* ssd = &bst->ssd;
    int n1, n2, n3, n4, n5, n6, n7, n8;
    int idx_fbin, idx_mbin, idx_jbin;
    int offset_fbin;
    float dw1[3], dw2[3], dw3[3], dw4[3], dw5[3], dw6[3], dw7[3], dw8[3];

    dc_dv[0] = dc_dv[1] = dc_dv[2] = 0.0f;

    // Calculate Point Indices for 8 neighborhood
    n1 = mvf;
    n2 = n1 + 1;
    n3 = n1 + moving->dim[0];
    n4 = n1 + moving->dim[0] + 1;
    n5 = n1 + moving->dim[0]*moving->dim[1];
    n6 = n1 + moving->dim[0]*moving->dim[1] + 1;
    n7 = n1 + moving->dim[0]*moving->dim[1] + moving->dim[0];
    n8 = n1 + moving->dim[0]*moving->dim[1] + moving->dim[0] + 1;

    // Pre-compute differential PV slices
    dw1[0] = (  -1 ) * li_1[1] * li_1[2];
    dw1[1] = li_1[0] * (  -1 ) * li_1[2];
    dw1[2] = li_1[0] * li_1[1] * (  -1 );

    dw2[0] = (  +1 ) * li_1[1] * li_1[2];
    dw2[1] = li_2[0] * (  -1 ) * li_1[2];
    dw2[2] = li_2[0] * li_1[1] * (  -1 );

    dw3[0] = (  -1 ) * li_2[1] * li_1[2];
    dw3[1] = li_1[0] * (  +1 ) * li_1[2];
    dw3[2] = li_1[0] * li_2[1] * (  -1 );

    dw4[0] = (  +1 ) * li_2[1] * li_1[2];
    dw4[1] = li_2[0] * (  +1 ) * li_1[2];
    dw4[2] = li_2[0] * li_2[1] * (  -1 );

    dw5[0] = (  -1 ) * li_1[1] * li_2[2];
    dw5[1] = li_1[0] * (  -1 ) * li_2[2];
    dw5[2] = li_1[0] * li_1[1] * (  +1 );

    dw6[0] = (  +1 ) * li_1[1] * li_2[2];
    dw6[1] = li_2[0] * (  -1 ) * li_2[2];
    dw6[2] = li_2[0] * li_1[1] * (  +1 );

    dw7[0] = (  -1 ) * li_2[1] * li_2[2];
    dw7[1] = li_1[0] * (  +1 ) * li_2[2];
    dw7[2] = li_1[0] * li_2[1] * (  +1 );

    dw8[0] = (  +1 ) * li_2[1] * li_2[2];
    dw8[1] = li_2[0] * (  +1 ) * li_2[2];
    dw8[2] = li_2[0] * li_2[1] * (  +1 );

    // Fixed image voxel's histogram index
    idx_fbin = floor ((f_img[fv] - mi_hist->fixed.offset) / mi_hist->fixed.delta);
    offset_fbin = idx_fbin * mi_hist->moving.bins;

    // Partial Volume w1's Contribution
    idx_mbin = floor ((m_img[n1] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > 0.0001) {
    	dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - ssd->score;
    	dc_dv[0] -= dw1[0] * dS_dP;
    	dc_dv[1] -= dw1[1] * dS_dP;
    	dc_dv[2] -= dw1[2] * dS_dP;
    }

    // Partial Volume w2
    idx_mbin = floor ((m_img[n2] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > 0.0001) {
    	dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - ssd->score;
    	dc_dv[0] -= dw2[0] * dS_dP;
    	dc_dv[1] -= dw2[1] * dS_dP;
    	dc_dv[2] -= dw2[2] * dS_dP;
    }

    // Partial Volume w3
    idx_mbin = floor ((m_img[n3] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > 0.0001) {
    	dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - ssd->score;
    	dc_dv[0] -= dw3[0] * dS_dP;
    	dc_dv[1] -= dw3[1] * dS_dP;
    	dc_dv[2] -= dw3[2] * dS_dP;
    }

    // Partial Volume w4
    idx_mbin = floor ((m_img[n4] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > 0.0001) {
    	dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - ssd->score;
    	dc_dv[0] -= dw4[0] * dS_dP;
    	dc_dv[1] -= dw4[1] * dS_dP;
    	dc_dv[2] -= dw4[2] * dS_dP;
    }

    // Partial Volume w5
    idx_mbin = floor ((m_img[n5] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > 0.0001) {
    	dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - ssd->score;
    	dc_dv[0] -= dw5[0] * dS_dP;
    	dc_dv[1] -= dw5[1] * dS_dP;
    	dc_dv[2] -= dw5[2] * dS_dP;
    }

    // Partial Volume w6
    idx_mbin = floor ((m_img[n6] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > 0.0001) {
    	dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - ssd->score;
    	dc_dv[0] -= dw6[0] * dS_dP;
    	dc_dv[1] -= dw6[1] * dS_dP;
    	dc_dv[2] -= dw6[2] * dS_dP;
    }

    // Partial Volume w7
    idx_mbin = floor ((m_img[n7] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > 0.0001) {
    	dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - ssd->score;
    	dc_dv[0] -= dw7[0] * dS_dP;
    	dc_dv[1] -= dw7[1] * dS_dP;
    	dc_dv[2] -= dw7[2] * dS_dP;
    }

    // Partial Volume w8
    idx_mbin = floor ((m_img[n8] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > 0.0001) {
    	dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - ssd->score;
    	dc_dv[0] -= dw8[0] * dS_dP;
    	dc_dv[1] -= dw8[1] * dS_dP;
    	dc_dv[2] -= dw8[2] * dS_dP;
    }

    dc_dv[0] = dc_dv[0] / num_vox_f / moving->pix_spacing[0];
    dc_dv[1] = dc_dv[1] / num_vox_f / moving->pix_spacing[1];
    dc_dv[2] = dc_dv[2] / num_vox_f / moving->pix_spacing[2];
}

inline void
bspline_update_grad_b_inline (Bspline_state* bst, BSPLINE_Xform* bxf, 
		     int pidx, int qidx, float dc_dv[3])
{
    BSPLINE_Score* ssd = &bst->ssd;
    int i, j, k, m;
    int cidx;
    float* q_lut = &bxf->q_lut[qidx*64];
    int* c_lut = &bxf->c_lut[pidx*64];

    m = 0;
    for (k = 0; k < 4; k++) {
	for (j = 0; j < 4; j++) {
	    for (i = 0; i < 4; i++) {
		cidx = 3 * c_lut[m];
		ssd->grad[cidx+0] += dc_dv[0] * q_lut[m];
		ssd->grad[cidx+1] += dc_dv[1] * q_lut[m];
		ssd->grad[cidx+2] += dc_dv[2] * q_lut[m];
		m ++;
	    }
	}
    }
}


////////////////////////////////////////////////////////////////////////////////
int CPU_MI_Hist (BSPLINE_MI_Hist *mi_hist,	// OUTPUT: Histograms
		BSPLINE_Xform *bxf,		//  INPUT: Bspline X-Form
		Volume* fixed,			//  INPUT: Fixed Image
		Volume* moving)			//  INPUT: Moving Image
{
	int rijk[3];
	int fijk[3];
	int fv;
	int p[3];
	int q[3];
	float fxyz[3];
	int pidx, qidx;
	float dxyz[3];
	float mxyz[3];
	float mijk[3];
	int mijk_f[3];	// floor: mijk
	int mijk_r[3];	// round: mijk
	int mvf;	// floor: mv
	float li_1[3];
	float li_2[3];
	int num_vox = 0;


	for (rijk[2] = 0, fijk[2] = bxf->roi_offset[2]; rijk[2] < bxf->roi_dim[2]; rijk[2]++, fijk[2]++) {
		p[2] = rijk[2] / bxf->vox_per_rgn[2];
		q[2] = rijk[2] % bxf->vox_per_rgn[2];
		fxyz[2] = bxf->img_origin[2] + bxf->img_spacing[2] * fijk[2];
		for (rijk[1] = 0, fijk[1] = bxf->roi_offset[1]; rijk[1] < bxf->roi_dim[1]; rijk[1]++, fijk[1]++) {
			p[1] = rijk[1] / bxf->vox_per_rgn[1];
			q[1] = rijk[1] % bxf->vox_per_rgn[1];
			fxyz[1] = bxf->img_origin[1] + bxf->img_spacing[1] * fijk[1];
			for (rijk[0] = 0, fijk[0] = bxf->roi_offset[0]; rijk[0] < bxf->roi_dim[0]; rijk[0]++, fijk[0]++) {
				int rc;
				p[0] = rijk[0] / bxf->vox_per_rgn[0];
				q[0] = rijk[0] % bxf->vox_per_rgn[0];
				fxyz[0] = bxf->img_origin[0] + bxf->img_spacing[0] * fijk[0];

				// Get B-spline deformation vector
				pidx = INDEX_OF (p, bxf->rdims);
				qidx = INDEX_OF (q, bxf->vox_per_rgn);
				bspline_interp_pix_b (dxyz, bxf, pidx, qidx);

				// Find correspondence in moving image
				rc = bspline_find_correspondence (mxyz, mijk, fxyz, dxyz, moving);

				// If voxel is not inside moving image
				if (!rc) continue;

				// Compute tri-linear interpolation weights
				CLAMP_LINEAR_INTERPOLATE_3D (mijk, mijk_f, mijk_r, li_1, li_2, moving);

				// Find linear index of fixed image voxel
				fv = INDEX_OF (fijk, fixed->dim);

				// Find linear index of "corner voxel" in moving image
				mvf = INDEX_OF (mijk_f, moving->dim);

				// PARTIAL VALUE INTERPOLATION - 8 neighborhood
				bspline_mi_hist_add_pvi_8 (mi_hist, fixed, moving, fv, mvf, li_1, li_2);

				// Increment the voxel count
				num_vox ++;
			}
		}
	}

	return num_vox;
}
////////////////////////////////////////////////////////////////////////////////


static float
CPU_MI_Score (BSPLINE_MI_Hist* mi_hist, int num_vox)
{
#ifdef DOUBLE_HISTS
    double* f_hist = mi_hist->f_hist_d;
    double* m_hist = mi_hist->m_hist_d;
    double* j_hist = mi_hist->j_hist_d;
#else
    float* f_hist = mi_hist->f_hist;
    float* m_hist = mi_hist->m_hist;
    float* j_hist = mi_hist->j_hist;
#endif

    int i, j, v;
    double fnv = (double) num_vox;
    double score = 0;
    double hist_thresh = 0.001 / (mi_hist->moving.bins * mi_hist->fixed.bins);

    /* Compute cost */
    for (i = 0, v = 0; i < mi_hist->fixed.bins; i++) {
	for (j = 0; j < mi_hist->moving.bins; j++, v++) {
	    if (j_hist[v] > hist_thresh) {
		score -= j_hist[v] * logf (fnv * j_hist[v] / (m_hist[j] * f_hist[i]));
	    }
	}
    }

    score = score / fnv;
    return (float) score;
}

void CPU_MI_Grad (BSPLINE_MI_Hist *mi_hist,	// OUTPUT: Histograms
		Bspline_state *bst,		//  INPUT: Bspline State
		BSPLINE_Xform *bxf,		//  INPUT: Bspline X-Form
		Volume* fixed,			//  INPUT: Fixed Image
		Volume* moving,			//  INPUT: Moving Image
		float num_vox_f)		//  INPUT: Number of voxels
{
	int rijk[3];
	int fijk[3];
	int fv;
	int p[3];
	int q[3];
	float fxyz[3];
	int pidx, qidx;
	float dxyz[3];
	float mxyz[3];
	float mijk[3];
	int mijk_f[3];	// floor: mijk
	int mijk_r[3];	// round: mijk
	int mvf;	// floor: mv
	float li_1[3];
	float li_2[3];
	float dc_dv[3];

	for (rijk[2] = 0, fijk[2] = bxf->roi_offset[2]; rijk[2] < bxf->roi_dim[2]; rijk[2]++, fijk[2]++) {
		p[2] = rijk[2] / bxf->vox_per_rgn[2];
		q[2] = rijk[2] % bxf->vox_per_rgn[2];
		fxyz[2] = bxf->img_origin[2] + bxf->img_spacing[2] * fijk[2];
		for (rijk[1] = 0, fijk[1] = bxf->roi_offset[1]; rijk[1] < bxf->roi_dim[1]; rijk[1]++, fijk[1]++) {
			p[1] = rijk[1] / bxf->vox_per_rgn[1];
			q[1] = rijk[1] % bxf->vox_per_rgn[1];
			fxyz[1] = bxf->img_origin[1] + bxf->img_spacing[1] * fijk[1];
			for (rijk[0] = 0, fijk[0] = bxf->roi_offset[0]; rijk[0] < bxf->roi_dim[0]; rijk[0]++, fijk[0]++) {
				int rc;

				p[0] = rijk[0] / bxf->vox_per_rgn[0];
				q[0] = rijk[0] % bxf->vox_per_rgn[0];
				fxyz[0] = bxf->img_origin[0] + bxf->img_spacing[0] * fijk[0];

				/* Get B-spline deformation vector */
				pidx = INDEX_OF (p, bxf->rdims);
				qidx = INDEX_OF (q, bxf->vox_per_rgn);
				bspline_interp_pix_b (dxyz, bxf, pidx, qidx);

				/* Find linear index of fixed image voxel */
				fv = INDEX_OF (fijk, fixed->dim);

				/* Find correspondence in moving image */
				rc = bspline_find_correspondence (mxyz, mijk, fxyz, dxyz, moving);

				/* If voxel is not inside moving image */
				if (!rc) continue;

				/* PARTIAL VALUE INTERPOLATION - 8 neighborhood */
				CLAMP_LINEAR_INTERPOLATE_3D (mijk, mijk_f, mijk_r, li_1, li_2, moving);

				/* Find linear index of fixed image voxel */
				fv = INDEX_OF (fijk, fixed->dim);

				/* Find linear index of "corner voxel" in moving image */
				mvf = INDEX_OF (mijk_f, moving->dim);

				// Partial Volume Interpolation 8-neighbor Gradient 
				bspline_mi_pvi_8_dc_dv (dc_dv, mi_hist, bst, fixed, moving, 
							fv, mvf, mijk, num_vox_f, li_1, li_2);

				// B-Spline parameterization
				bspline_update_grad_b_inline (bst, bxf, pidx, qidx, dc_dv);
			}
		}
	}
}
	

////////////////////////////////////////////////////////////////////////////////
// This implementation uses the CPU to generate the histgrams only.
// Everything else is performed on the GPU.
//
// This is done in the interest of time.  I don't feel like developing
// a parallel prefix scan right now that can deal with the large data
// sets we use.
////////////////////////////////////////////////////////////////////////////////
void bspline_cuda_MI_a (
	BSPLINE_Parms *parms,
	Bspline_state *bst,
	BSPLINE_Xform *bxf,
	Volume *fixed,
	Volume *moving,
	Volume *moving_grad,
	Dev_Pointers_Bspline *dev_ptrs)
{

	// --- DECLARE LOCAL VARIABLES ------------------------------
	BSPLINE_Score* ssd;	// Holds the SSD "Score" information
	int num_vox;		// Holds # of voxels in the fixed volume
	Timer timer;
	Timer timer0;
	double interval;
	BSPLINE_MI_Hist* mi_hist = &parms->mi_hist;
	float* f_hist = mi_hist->f_hist;
	float* m_hist = mi_hist->m_hist;
	float* j_hist = mi_hist->j_hist;
#ifdef DOUBLE_HISTS
	double* f_hist_d = mi_hist->f_hist_d;
	double* m_hist_d = mi_hist->m_hist_d;
	double* j_hist_d = mi_hist->j_hist_d;
#endif
	static int it=0;	// Holds Iteration Number
	char debug_fn[1024];	// Debug message buffer
	FILE* fp = NULL;	// File Pointer to Debug File
	int i;			// Good ol' i
	// ----------------------------------------------------------


	// --- INITIALIZE LOCAL VARIABLES ---------------------------
	ssd = &bst->ssd;
	
	if (parms->debug) {
		sprintf (debug_fn, "dump_mse_%02d.txt", it++);
		fp = fopen (debug_fn, "w");
	}
	// ----------------------------------------------------------

	//
	// --- INITIALIZE CPU MEMORY --------------------------------
	memset(f_hist, 0, mi_hist->fixed.bins * sizeof (float));
	memset(m_hist, 0, mi_hist->moving.bins * sizeof (float));
	memset(j_hist, 0, mi_hist->moving.bins * mi_hist->fixed.bins * sizeof (float));
#ifdef DOUBLE_HISTS
	memset(f_hist_d, 0, mi_hist->fixed.bins * sizeof (double));
	memset(m_hist_d, 0, mi_hist->moving.bins * sizeof (double));
	memset(j_hist_d, 0, mi_hist->moving.bins * mi_hist->fixed.bins * sizeof (double));
#endif
	// ----------------------------------------------------------

	// --- INITIALIZE GPU MEMORY --------------------------------
	bspline_cuda_h_push_coeff_lut(dev_ptrs, bxf);
	bspline_cuda_h_clear_score(dev_ptrs);
	bspline_cuda_h_clear_grad(dev_ptrs);
	// ----------------------------------------------------------

	plm_timer_start (&timer);	// <=== START TIMING HERE
	
	// generate histograms
	plm_timer_start (&timer0);
#ifdef CPU_HISTS
	num_vox = CPU_MI_Hist (mi_hist, bxf, fixed, moving);
	printf (" * hists: %9.3f s\t [CPU]\n", plm_timer_report(&timer0));
#else
	// Invoke Parallel Prefix Scan driven GPU HIST kernel set
	num_vox = CUDA_MI_Hist_a (mi_hist, bxf, fixed, moving, dev_ptrs);
	printf (" * hists: %9.3f s\t [GPU]\n", plm_timer_report(&timer0));
#endif

	// if using double histograms for accumulation
	// copy their contents over to floating histograms

#if defined (DOUBLE_HISTS) && defined (CPU_HISTS)
    for (i=0; i < mi_hist->fixed.bins; i++)
    	f_hist[i] = (float)f_hist_d[i];
    for (i=0; i < mi_hist->moving.bins; i++)
    	m_hist[i] = (float)m_hist_d[i];
    for (i=0; i < mi_hist->moving.bins * mi_hist->fixed.bins; i++)
    	j_hist[i] = (float)j_hist_d[i];
#endif

	// dump histogram images?
	if (parms->xpm_hist_dump) {
		dump_xpm_hist (mi_hist, parms->xpm_hist_dump, bst->it);
	}

	if (parms->debug) {
		dump_hist (mi_hist, bst->it);
	}

	// compute score
	plm_timer_start (&timer0);
#ifdef CPU_SCORE
	 ssd->score = CPU_MI_Score(mi_hist, num_vox);
	printf (" * score: %9.3f s\t [CPU]\n", plm_timer_report(&timer0));
#else
	// Doing this on the GPU may be silly.
	// GPU_MI_Score_a();
	printf (" * score: %9.3f s\t [GPU]\n", plm_timer_report(&timer0));
#endif

	// compute gradient
	plm_timer_start (&timer0);
#ifdef CPU_GRAD
	CPU_MI_Grad(mi_hist, bst, bxf, fixed, moving, (float)num_vox);
	printf (" *  grad: %9.3f s\t [CPU]\n", plm_timer_report(&timer0));
#else
	CUDA_MI_Grad_a(mi_hist, bst, bxf, fixed, moving, (float)num_vox, dev_ptrs);
	printf (" *  grad: %9.3f s\t [GPU]\n", plm_timer_report(&timer0));
#endif


	interval = plm_timer_report (&timer);
	report_score ("MI", bxf, bst, num_vox, interval);
}
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
// FUNCTION: bspline_cuda_score_j_mse()
//
////////////////////////////////////////////////////////////////////////////////
void bspline_cuda_score_j_mse (
    BSPLINE_Parms* parms,
    Bspline_state *bst,
    BSPLINE_Xform* bxf,
    Volume* fixed,
    Volume* moving,
    Volume* moving_grad,
    Dev_Pointers_Bspline* dev_ptrs)
{
    // --- DECLARE LOCAL VARIABLES ------------------------------
    BSPLINE_Score* ssd;		// Holds the SSD "Score" information
    int num_vox;		// Holds # of voxels in the fixed volume
    float ssd_grad_norm;	// Holds the SSD Gradient's Norm
    float ssd_grad_mean;	// Holds the SSD Gradient's Mean
    Timer timer;

    static int it=0;	// Holds Iteration Number
    char debug_fn[1024];	// Debug message buffer
    FILE* fp = NULL;		// File Pointer to Debug File
    // ----------------------------------------------------------


    // --- INITIALIZE LOCAL VARIABLES ---------------------------
    ssd = &bst->ssd;
	
    if (parms->debug) {
	sprintf (debug_fn, "dump_mse_%02d.txt", it++);
	fp = fopen (debug_fn, "w");
    }
    // ----------------------------------------------------------


    plm_timer_start (&timer);	// <=== START TIMING HERE

	
    // --- INITIALIZE GPU MEMORY --------------------------------
    bspline_cuda_h_push_coeff_lut(dev_ptrs, bxf);
    bspline_cuda_h_clear_score(dev_ptrs);
    bspline_cuda_h_clear_grad(dev_ptrs);
    // ----------------------------------------------------------


	
    // --- LAUNCH STUB FUNCTIONS --------------------------------

    // Populate the score, dc_dv, and gradient
    bspline_cuda_j_stage_1(
	fixed,
	moving,
	moving_grad,
	bxf,
	parms,
	dev_ptrs);


    // Calculate the score and gradient
    // via sum reduction
    bspline_cuda_j_stage_2(
	parms,
	bxf,
	fixed,
	bxf->vox_per_rgn,
	fixed->dim,
	&(ssd->score),
	bst->ssd.grad, //ssd->grad,
	&ssd_grad_mean,
	&ssd_grad_norm,
	dev_ptrs,
	&num_vox);

    if (parms->debug) {
	fclose (fp);
    }

    // --- USER FEEDBACK ----------------------------------------
    report_score ("MSE", bxf, bst, num_vox, plm_timer_report (&timer));
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// FUNCTION: bspline_cuda_score_i_mse()
//
////////////////////////////////////////////////////////////////////////////////
void bspline_cuda_score_i_mse (
    BSPLINE_Parms* parms,
    Bspline_state *bst,
    BSPLINE_Xform* bxf,
    Volume* fixed,
    Volume* moving,
    Volume* moving_grad,
    Dev_Pointers_Bspline* dev_ptrs)
{


    // --- DECLARE LOCAL VARIABLES ------------------------------
    BSPLINE_Score* ssd;	// Holds the SSD "Score" information
    int num_vox;		// Holds # of voxels in the fixed volume
    float ssd_grad_norm;	// Holds the SSD Gradient's Norm
    float ssd_grad_mean;	// Holds the SSD Gradient's Mean
    Timer timer;            // The timer

    static int it=0;	// Holds Iteration Number
    char debug_fn[1024];	// Debug message buffer
    FILE* fp = NULL;		// File Pointer to Debug File
    // ----------------------------------------------------------


    // --- INITIALIZE LOCAL VARIABLES ---------------------------
    ssd = &bst->ssd;
	
    if (parms->debug) {
	    sprintf (debug_fn, "dump_mse_%02d.txt", it++);
	    fp = fopen (debug_fn, "w");
	}
    // ----------------------------------------------------------

    plm_timer_start (&timer);  // <=== START TIMING HERE
	
    // --- INITIALIZE GPU MEMORY --------------------------------
    bspline_cuda_h_push_coeff_lut(dev_ptrs, bxf);
    bspline_cuda_h_clear_score(dev_ptrs);
    bspline_cuda_h_clear_grad(dev_ptrs);
    // ----------------------------------------------------------


	
    // --- LAUNCH STUB FUNCTIONS --------------------------------

    // Populate the score, dc_dv, and gradient
    bspline_cuda_i_stage_1(
	fixed,
	moving,
	moving_grad,
	bxf,
	parms,
	dev_ptrs);


    // Calculate the score and gradient
    // via sum reduction
    bspline_cuda_j_stage_2(
	parms,
	bxf,
	fixed,
	bxf->vox_per_rgn,
	fixed->dim,
	&(ssd->score),
	bst->ssd.grad, //ssd->grad,
	&ssd_grad_mean,
	&ssd_grad_norm,
	dev_ptrs,
	&num_vox);

    if (parms->debug) {
	fclose (fp);
    }

    // --- USER FEEDBACK ----------------------------------------
    report_score ("MSE", bxf, bst, num_vox, plm_timer_report (&timer));
}
////////////////////////////////////////////////////////////////////////////////
