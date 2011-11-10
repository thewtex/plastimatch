/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include "plastimatch_slicer_threshboxCLP.h"

#include "itk_image.h"
#include "itk_image_save.h"
#include "threshbox.h"
#include "string.h"

int 
main (int argc, char * argv [])
{
    PARSE_ARGS;

    Threshbox_parms parms;
    
    parms.center[0] = plmslc_threshbox_center[0];
    parms.center[1] = plmslc_threshbox_center[1];
    parms.center[2] = plmslc_threshbox_center[2];
    parms.boxsize[0] = plmslc_threshbox_boxsize[0];
    parms.boxsize[1] = plmslc_threshbox_boxsize[1];
    parms.boxsize[2] = plmslc_threshbox_boxsize[2];
    parms.threshold = plmslc_threshbox_threshold;

    parms.img_in = plm_image_load_native ( plmslc_threshbox_img_in.c_str() );

    do_threshbox( &parms );
    
//    FloatImageType::Pointer img = parms.img_out->itk_float();
//    itk_image_save_float (img, plmslc_threshbox_img_out.c_str());

    UCharImageType::Pointer img = parms.img_out->itk_uchar();
    itk_image_save (img, plmslc_threshbox_img_out.c_str());

    UCharImageType::Pointer img_box = parms.img_box->itk_uchar();
    itk_image_save (img_box, plmslc_threshbox_img_box.c_str());

    if (plmslc_need_overlap_fraction) {  
	
	strcpy( parms.overlap_fn, plmslc_overlap_fraction.c_str() );
	parms.overlap_labelmap1 = 
	    plm_image_load_native( plmslc_threshbox_labelmap_1.c_str() );
	parms.overlap_labelmap2 = 
	    plm_image_load_native( plmslc_threshbox_labelmap_2.c_str() );

	do_overlap_fraction( &parms );    
	}

    return EXIT_SUCCESS;
}
