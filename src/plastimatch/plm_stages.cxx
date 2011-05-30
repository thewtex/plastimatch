/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "itkTimeProbe.h"

#include "gpuit_bspline.h"
#include "gpuit_demons.h"
#include "itk_demons.h"
#include "itk_image_load.h"
#include "itk_image_save.h"
#include "itk_optim.h"
#include "itk_registration.h"
#include "itk_warp.h"
#include "logfile.h"
#include "mha_io.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "plm_parms.h"
#include "plm_stages.h"
#include "plm_warp.h"
#include "resample_mha.h"
#include "vf.h"
#include "xform.h"

#define FIXME_BACKGROUND_MAX (-1200)

/* This helps speed up the registration, by setting the bounding box to the 
   smallest size needed.  To find the bounding box, either use the extent 
   of the fixed_mask (if one is used), or by eliminating excess air 
   by thresholding */
static void
set_fixed_image_region_global (Registration_Data* regd)
{
    int use_magic_value = 1;

    regd->fixed_region_origin = regd->fixed_image->itk_float()->GetOrigin();
    regd->fixed_region_spacing = regd->fixed_image->itk_float()->GetSpacing();

    if (regd->fixed_mask) {
	FloatImageType::RegionType::IndexType valid_index;
	FloatImageType::RegionType::SizeType valid_size;

	/* Search for bounding box of fixed mask */
	typedef itk::ImageRegionConstIteratorWithIndex< 
	    UCharImageType > IteratorType;
	UCharImageType::RegionType region 
	    = regd->fixed_mask->GetLargestPossibleRegion();
	IteratorType it (regd->fixed_mask, region);

	int first = 1;
	valid_index[0] = 0;
	valid_index[1] = 0;
	valid_index[2] = 0;
	valid_size[0] = 0;
	valid_size[1] = 0;
	valid_size[2] = 0;

	for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
	    unsigned char c = it.Get();
	    if (c) {
		UCharImageType::RegionType::IndexType idx = it.GetIndex();
		if (first) {
		    first = 0;
		    valid_index = idx;
		    valid_size[0] = 1;
		    valid_size[1] = 1;
		    valid_size[2] = 1;
		} else {
		    int updated = 0;
		    for (int i = 0; i < 3; i++) {
			if (valid_index[i] > idx[i]) {
			    valid_size[i] += valid_index[i] - idx[i];
			    valid_index[i] = idx[i];
			    updated = 1;
			}
			if (idx[i] - valid_index[i] >= (long) valid_size[i]) {
			    valid_size[i] = idx[i] - valid_index[i] + 1;
			    updated = 1;
			}
		    }
		}
	    }
	}
	regd->fixed_region.SetIndex(valid_index);
	regd->fixed_region.SetSize(valid_size);
    } else if (use_magic_value) {
	FloatImageType::RegionType::IndexType valid_index;
	FloatImageType::RegionType::SizeType valid_size;
	valid_index[0] = 0;
	valid_index[1] = 0;
	valid_index[2] = 0;
	valid_size[0] = 1;
	valid_size[1] = 1;
	valid_size[2] = 1;

	/* Make sure the image is ITK float */
	FloatImageType::Pointer fixed_image = regd->fixed_image->itk_float();

	/* Search for bounding box of patient */
	typedef itk::ImageRegionConstIteratorWithIndex <
	    FloatImageType > IteratorType;
	FloatImageType::RegionType region 
	    = fixed_image->GetLargestPossibleRegion();
	IteratorType it (fixed_image, region);

	int first = 1;
	for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
	    float c = it.Get();
	    if (c > FIXME_BACKGROUND_MAX) {
		FloatImageType::RegionType::IndexType idx = it.GetIndex();
		if (first) {
		    first = 0;
		    valid_index = idx;
		    valid_size[0] = 1;
		    valid_size[1] = 1;
		    valid_size[2] = 1;
		} else {
		    int updated = 0;
		    for (int i = 0; i < 3; i++) {
			if (valid_index[i] > idx[i]) {
			    valid_size[i] += valid_index[i] - idx[i];
			    valid_index[i] = idx[i];
			    updated = 1;
			}
			if (idx[i] - valid_index[i] >= (long) valid_size[i]) {
			    valid_size[i] = idx[i] - valid_index[i] + 1;
			    updated = 1;
			}
		    }
		}
	    }
	}
	/* Try to include a margin of at least one air pixel everywhere */
	for (int i = 0; i < 3; i++) {
	    if (valid_index[i] > 0) {
		valid_index[i]--;
		valid_size[i]++;
	    }
	    if (valid_size[i] + valid_index[i] 
		< fixed_image->GetLargestPossibleRegion().GetSize()[i])
	    {
		valid_size[i]++;
	    }
	}
	regd->fixed_region.SetIndex(valid_index);
	regd->fixed_region.SetSize(valid_size);
    } else {
	regd->fixed_region 
	    = regd->fixed_image->itk_float()->GetLargestPossibleRegion();
    }
}

#if defined (commentout)
static Plm_image_type
choose_image_type (int xform_type, int optim_type, int impl_type)
{
    switch (impl_type) {
    case IMPLEMENTATION_PLASTIMATCH:
	return PLM_IMG_TYPE_GPUIT_FLOAT;
    default:
	return PLM_IMG_TYPE_ITK_FLOAT;
    }
}
#endif

static void
save_output (
    Registration_Data* regd, 
    Xform *xf_out, 
    char *xf_out_fn,
    bool xf_out_itk, 
    int img_out_fmt,
    char *img_out_fn,
    char *vf_out_fn
)
{
    if (xf_out_fn[0]) {
	logfile_printf ("Writing transformation ...\n");
	if (xf_out_itk && xf_out->m_type == XFORM_GPUIT_BSPLINE) {
	    Xform xf_tmp;
	    Plm_image_header pih;
	    pih.set_from_plm_image (regd->fixed_image);
	    xform_to_itk_bsp (&xf_tmp, xf_out, &pih, 0);
	    xform_save (&xf_tmp, xf_out_fn);
	} else {
	    xform_save (xf_out, xf_out_fn);
	}
    }

    if (img_out_fn[0] || vf_out_fn[0]) {
	DeformationFieldType::Pointer vf;
	DeformationFieldType::Pointer *vfp;
	Plm_image im_warped;
	Plm_image *imp;
	Plm_image_header pih;
	float default_val = 0.0f;   /* GCS FIX: hard coded. */

	if (vf_out_fn[0]) {
	    vfp = &vf;
	} else {
	    vfp = 0;
	}
	if (img_out_fn[0]) {
	    imp = &im_warped;
	} else {
	    imp = 0;
	}
	
	pih.set_from_plm_image (regd->fixed_image);

	logfile_printf ("Warping...\n");
	plm_warp (imp, vfp, xf_out, &pih, regd->moving_image, 
	    default_val, 0, 1);

	if (img_out_fn[0]) {
	    logfile_printf ("Saving image...\n");
	    if (img_out_fmt == IMG_OUT_FMT_AUTO) {
		im_warped.save_image (img_out_fn);
	    } else {
		im_warped.save_short_dicom (img_out_fn, 0, 0);
	    }
	}
	if (vf_out_fn[0]) {
	    logfile_printf ("Saving vf...\n");
	    itk_image_save (vf, vf_out_fn);
	}
    }
}

static void
do_registration_stage (
    Registration_Parms* regp, 
    Registration_Data* regd, 
    Xform *xf_out, Xform *xf_in, 
    Stage_parms* stage)
{
    logfile_printf ("[1] xf_in->m_type = %d, xf_out->m_type = %d\n", 
	xf_in->m_type, xf_out->m_type);

    /* Run registration */
    if (stage->optim_type == OPTIMIZATION_DEMONS) {
	if (stage->impl_type == IMPLEMENTATION_ITK) {
	    do_demons_stage (regd, xf_out, xf_in, stage);
	} else {
	    do_gpuit_demons_stage (regd, xf_out, xf_in, stage);
	}
    }
    else if (stage->xform_type == STAGE_TRANSFORM_BSPLINE) {
	if (stage->impl_type == IMPLEMENTATION_ITK) {
	    do_itk_registration_stage (regd, xf_out, xf_in, stage);
	} else {
	    do_gpuit_bspline_stage (regp, regd, xf_out, xf_in, stage);
	}
    }
    else if (stage->xform_type == STAGE_TRANSFORM_ALIGN_CENTER) {
	do_itk_center_stage (regd, xf_out, xf_in, stage);
	std::cout << "Centering done" << std::endl;
    }
    else {
	do_itk_registration_stage (regd, xf_out, xf_in, stage);
    }

    logfile_printf ("[2] xf_out->m_type = %d, xf_in->m_type = %d\n", 
	xf_out->m_type, xf_in->m_type);

    /* Save intermediate output */
    save_output (regd, xf_out, stage->xf_out_fn, stage->xf_out_itk, 
	stage->img_out_fmt, stage->img_out_fn, stage->vf_out_fn);
}

static void
load_input_files (Registration_Data* regd, Registration_Parms* regp)
{
    Plm_image_type image_type = PLM_IMG_TYPE_ITK_FLOAT;

    /* Always load as ITK so we can find the ROI */
    logfile_printf ("fixed image=%s\n", regp->fixed_fn);
    logfile_printf ("Loading fixed image...\n");
    regd->fixed_image = plm_image_load (regp->fixed_fn, image_type);
    logfile_printf ("done!\n");

    logfile_printf ("moving image=%s\n", regp->moving_fn);
    logfile_printf ("Loading moving image...\n");
    regd->moving_image = plm_image_load (regp->moving_fn, image_type);
    logfile_printf ("done!\n");

    if (regp->fixed_mask_fn[0]) {
	logfile_printf ("Loading fixed mask...\n");
	regd->fixed_mask = itk_image_load_uchar (regp->fixed_mask_fn, 0);
	logfile_printf ("done!\n");
    } else {
	regd->fixed_mask = 0;
    }
    if (regp->moving_mask_fn[0]) {
	logfile_printf ("Loading moving mask...\n");
	regd->moving_mask = itk_image_load_uchar (regp->moving_mask_fn, 0);
	logfile_printf ("done!\n");
    } else {
	regd->moving_mask = 0;
    }
}

static void
set_auto_subsampling (int subsample_rate[], Plm_image *pli)
{
    Plm_image_header pih (pli);
    
    for (int d = 0; d < 3; d++) {
	subsample_rate[d] = (pih.Size(d)+99) / 100;
    }
}

static void
set_automatic_parameters (Registration_Data* regd, Registration_Parms* regp)
{
    for (int i = 0; i < regp->num_stages; i++) {
	Stage_parms *stagep = regp->stages[i];
	if (stagep->subsampling_type == SUBSAMPLING_AUTO) {
	    set_auto_subsampling (
		stagep->fixed_subsample_rate, regd->fixed_image);
	    set_auto_subsampling (
		stagep->moving_subsample_rate, regd->moving_image);
	}
    }
}

void
do_registration (Registration_Parms* regp)
{
    int i;
    Registration_Data regd;
    Xform xf1, xf2;
    Xform *xf_in, *xf_out, *xf_tmp;
    itk::TimeProbe timer1, timer2, timer3;

    xf_in = &xf1;
    xf_out = &xf2;

    /* Start logging */
    logfile_open (regp->log_fn);

    /* Load images */
    timer1.Start();
    load_input_files (&regd, regp);

    /* Load initial guess of xform */
    if (regp->xf_in_fn[0]) {
	xform_load (xf_out, regp->xf_in_fn);
    }

    /* Set fixed image region */
    set_fixed_image_region_global (&regd);

    /* Set automatic parameters based on image size */
    set_automatic_parameters (&regd, regp);

    timer1.Stop();

    timer2.Start();
    for (i = 0; i < regp->num_stages; i++) {
	/* Swap xf_in and xf_out */
	xf_tmp = xf_out; xf_out = xf_in; xf_in = xf_tmp;
	/* Run registation, results are stored in xf_out */
	do_registration_stage (regp, &regd, xf_out, xf_in, regp->stages[i]);
    }
    timer2.Stop();

    /* RMK: If no stages, we still generate output (same as input) */

    timer3.Start();
    save_output (&regd, xf_out, regp->xf_out_fn, regp->xf_out_itk, 
	regp->img_out_fmt, regp->img_out_fn, regp->vf_out_fn);
    timer3.Stop();

    logfile_printf (
	"Load:   %g\n"
	"Run:    %g\n"
	"Save:   %g\n"
	"Total:  %g\n",
	(double) timer1.GetMeanTime(),
	(double) timer2.GetMeanTime(),
	(double) timer3.GetMeanTime(),
	(double) timer1.GetMeanTime() + 
	(double) timer2.GetMeanTime() +
	(double) timer3.GetMeanTime());

    /* Done logging */
    logfile_printf ("Finished!\n");
    logfile_close ();
}
