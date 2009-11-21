/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "logfile.h"
#include "print_and_exit.h"
#include "volume.h"
#include "tps.h"

#define BUFLEN 1024
#define MOTION_TOL 0.01
#define DIST_MULTIPLIER 5.0

#define INDEX_OF(ijk, dim) \
    (((ijk[2] * dim[1] + ijk[1]) * dim[0]) + ijk[0])

Tps_xform*
tps_xform_alloc (void)
{
    Tps_xform *tps;

    tps = (Tps_xform*) malloc (sizeof (Tps_xform));
    if (!tps) {
	print_and_exit ("Out of memory\n");
    }
    memset (tps, 0, sizeof (Tps_xform));
    return tps;
}

Tps_xform*
tps_xform_load (char* fn)
{
    FILE *fp;
    Tps_xform *tps;
    char buf[1024];
    int rc;
    float img_origin[3];         /* Image origin (in mm) */
    float img_spacing[3];        /* Image spacing (in mm) */
    int img_dim[3];              /* Image size (in vox) */
    int d;

    tps = tps_xform_alloc ();

    /* If file doesn't exist, then no tps for this phase */
    fp = fopen (fn, "r");
    if (!fp) return tps;

    /* Skip first line */
    fgets (buf, 1024, fp);

    /* Read header */
    rc = fscanf (fp, "img_origin = %f %f %f\n", 
	&img_origin[0], &img_origin[1], &img_origin[2]);
    if (rc != 3) {
	logfile_printf ("Error parsing input xform (img_origin): %s\n", fn);
	return tps;
    }
    rc = fscanf (fp, "img_spacing = %f %f %f\n", 
	&img_spacing[0], &img_spacing[1], &img_spacing[2]);
    if (rc != 3) {
	logfile_printf ("Error parsing input xform (img_spacing): %s\n", fn);
	return tps;
    }
    rc = fscanf (fp, "img_dim = %d %d %d\n", 
	&img_dim[0], &img_dim[1], &img_dim[2]);
    if (rc != 3) {
	logfile_printf ("Error parsing input xform (img_dim): %s\n", fn);
	return tps;
    }

    for (d = 0; d < 3; d++) {
	tps->img_spacing[d] = img_spacing[d];
	tps->img_origin[d] = img_origin[d];
	tps->img_dim[d] = img_dim[d];
    }

    /* Read control points */
    while (fgets (buf, 1024, fp)) {
	int rc;
	Tps_node *curr_node;
	tps->tps_nodes = (struct tps_node*) 
	    realloc ((void*) (tps->tps_nodes), 
		(tps->num_tps_nodes + 1) * sizeof (Tps_node));
	if (!(tps->tps_nodes)) {
	    print_and_exit ("Error allocating memory");
	}
	curr_node = &tps->tps_nodes[tps->num_tps_nodes];
	rc = sscanf (buf, "%g %g %g %g %g %g %g", 
	    &curr_node->src[0],
	    &curr_node->src[1],
	    &curr_node->src[2],
	    &curr_node->tgt[0],
	    &curr_node->tgt[1],
	    &curr_node->tgt[2],
	    &curr_node->alpha
	);
	if (rc != 7) {
	    print_and_exit ("Ill-formed input file: %s\n", fn);
	}

	/* Compute weights, based on distance and alpha */
	curr_node->wxyz[2] = curr_node->tgt[2] - curr_node->src[2];
	curr_node->wxyz[1] = curr_node->tgt[1] - curr_node->src[1];
	curr_node->wxyz[0] = curr_node->tgt[0] - curr_node->src[0];
	printf ("wxyz = (%g %g %g)\n", 
	    curr_node->wxyz[0],
	    curr_node->wxyz[1],
	    curr_node->wxyz[2]);

	tps->num_tps_nodes++;
    }
    fclose (fp);
    return tps;
}

void
tps_xform_save (Tps_xform *tps, char *fn)
{
    int i;
    FILE *fp = fopen (fn, "w");
    if (!fp) {
	print_and_exit ("Couldn't open file \"%s\" for write\n", fn);
    }

    for (i = 0; i < tps->num_tps_nodes; i++) {
	float dist;
	float *src = tps->tps_nodes[i].src;
	float *tgt = tps->tps_nodes[i].tgt;
	dist = sqrt (((src[0] - tgt[0]) * (src[0] - tgt[0]))
	    + ((src[1] - tgt[1]) * (src[1] - tgt[1]))
	    + ((src[2] - tgt[2]) * (src[2] - tgt[2])));

	fprintf (fp, "%g %g %g %g %g %g %g\n", 
	    src[0],
	    src[1],
	    src[2],
	    tgt[0]-src[0],
	    tgt[1]-src[1],
	    tgt[2]-src[2],
	    DIST_MULTIPLIER * dist
	);
    }
    fclose (fp);
}

void
tps_xform_free (Tps_xform *tps)
{
    free (tps->tps_nodes);
    free (tps);
}

float
tps_default_alpha (float src[3], float tgt[3])
{
    float dist;

    dist = sqrt (((src[0] - tgt[0]) * (src[0] - tgt[0]))
	+ ((src[1] - tgt[1]) * (src[1] - tgt[1]))
	+ ((src[2] - tgt[2]) * (src[2] - tgt[2])));
    return DIST_MULTIPLIER * dist;
}

#if defined (commentout)
void
tps_warp_point (
    float new_pos[3], 
    Tps_xform* tps, 
    float pos[3])
{
    int i, j;

    if (!tps) return;

    memcpy (new_pos, pos, 3 * sizeof(float));
    for (i = 0; i < tps->num_tps_nodes; i++) {
	float dist = sqrt (
	    ((pos[0] - tps[i].src[0]) * (pos[0] - tps[i].src[0])) +
	    ((pos[1] - tps[i].src[1]) * (pos[1] - tps[i].src[1])) +
	    ((pos[2] - tps[i].src[2]) * (pos[2] - tps[i].src[2])));
	dist = dist / tps[i].alpha;
	if (dist < 1.0) {
	    float weight = (1 - dist) * (1 - dist);
	    for (j = 0; j < 3; j++) {
		new_pos[j] += weight * tps[i].tgt[j];
	    }
	}
    }
}
#endif

static void
tps_update_point (
    float vf[3],       /* Output: displacement to update */
    Tps_node* tpsn,    /* Input: the tps control point */
    float fxyz[3])     /* Input: location of voxel to update */
{
    int d;

    float dist2 = 
	((fxyz[0] - tpsn->src[0]) * (fxyz[0] - tpsn->src[0])) +
	((fxyz[1] - tpsn->src[1]) * (fxyz[1] - tpsn->src[1])) +
	((fxyz[2] - tpsn->src[2]) * (fxyz[2] - tpsn->src[2]));
    dist2 = dist2 / (tpsn->alpha * tpsn->alpha);
    if (dist2 < 1.0) {
	float dist = sqrt (dist2);
	float weight = (1 - dist) * (1 - dist);
	if (weight > 1.0) {
	    printf ("(d2,d,w) = (%g,%g,%g)\n", dist2, dist, weight);
	}
	for (d = 0; d < 3; d++) {
	    vf[d] += weight * tpsn->wxyz[d];
	}
    }
}

void
tps_warp (
    Volume *vout,       /* Output image (sized and allocated) */
    Volume *vf_out,     /* Output vf (sized and allocated, can be null) */
    Tps_xform *tps,     /* TPS control points */
    Volume *moving,     /* Input image */
    int linear_interp,  /* 1 = trilinear, 0 = nearest neighbors */
    float default_val   /* Fill in this value outside of image */
)
{
    int d;
    int vidx;

    int cpi;
    int fijk[3], fidx;       /* Indices within fixed image (vox) */
    float fxyz[3];           /* Position within fixed image (mm) */
    Volume *vf;
    float *vf_img;

    /* A few sanity checks */
    if (vout && vout->pix_type != PT_FLOAT) {
	fprintf (stderr, "Error: tps_warp pix type mismatch\n");
	return;
    }
    if (vf_out && vf_out->pix_type != PT_VF_FLOAT_INTERLEAVED) {
	fprintf (stderr, "Error: tps_warp requires interleaved vf\n");
	return;
    }

    /* Set defaults */
    if (vout) {
	float* vout_img = (float*) vout->img;
	for (vidx = 0; vidx < vout->npix; vidx++) {
	    vout_img[vidx] = default_val;
	}
    }
    if (vf_out) {
	vf = vf_out;
	memset (vf->img, 0, vf->pix_size * vf_out->npix);
    } else {
	vf = volume_create (
	    tps->img_dim,
	    tps->img_origin,
	    tps->img_spacing,
	    PT_VF_FLOAT_INTERLEAVED,
	    0, 0);
    }
    vf_img = (float*) vf->img;
	
    /* Loop through control points, and construct the vector field */
    for (cpi = 0; cpi < tps->num_tps_nodes; cpi++) {
	Tps_node *curr_node = &tps->tps_nodes[cpi];
	long roi_offset[3];
	long roi_size[3];

	/* Compute "region of influence" for current control point. 
	   The region of interest is the set of voxels that the control 
	   point influences, clipped against the entire volume. */
	for (d = 0; d < 3; d++) {
	    float rmin, rmax;
	    long rmini, rmaxi;

	    rmin = curr_node->src[d] - curr_node->alpha;
	    rmax = curr_node->src[d] + curr_node->alpha;
	    rmini = floorl (rmin - tps->img_origin[d] / tps->img_spacing[d]);
	    rmaxi = ceill (rmax - tps->img_origin[d] / tps->img_spacing[d]);

	    if (rmini < 0) rmini = 0;
	    if (rmaxi >= moving->dim[d]) rmaxi = moving->dim[d] - 1;

	    roi_offset[d] = rmini;
	    roi_size[d] = rmaxi - rmini + 1;
	}

	printf (
	    "cpi = %d, offset = (%ld %ld %ld), size = (%ld %ld %ld)"
	    " alpha = %g\n",
	    cpi, roi_offset[0], roi_offset[1], roi_offset[2], 
	    roi_size[0], roi_size[1], roi_size[2],
	    curr_node->alpha
	);

	/* Loop through ROI */
	for (fijk[2] = roi_offset[2]; fijk[2] < roi_offset[2] + roi_size[2] - 1; fijk[2]++) {
	    fxyz[2] = tps->img_origin[2] + tps->img_spacing[2] * fijk[2];
	    for (fijk[1] = roi_offset[1]; fijk[1] < roi_offset[1] + roi_size[1] - 1; fijk[1]++) {
		fxyz[1] = tps->img_origin[1] + tps->img_spacing[1] * fijk[1];
		for (fijk[0] = roi_offset[0]; fijk[0] < roi_offset[0] + roi_size[0] - 1; fijk[0]++) {

		    /* Update vf at this voxel, for this node */
		    fxyz[0] = tps->img_origin[0] 
			+ tps->img_spacing[0] * fijk[0];
		    fidx = INDEX_OF (fijk, tps->img_dim);
		    tps_update_point (&vf_img[3*fidx], curr_node, fxyz);
		}
	    }
	}
    }

    /* Warp the image */
    /* GCS FIX: Does not implement linear interpolation */
    if (vout) {
	volume_warp (vout, moving, vf);
    }

    if (!vf_out) {
	volume_free (vf);
    }
}
