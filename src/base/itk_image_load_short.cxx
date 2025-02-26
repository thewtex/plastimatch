/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"

#include "itk_image_load.txx"

ShortImageType::Pointer
itk_image_load_short (const char* fname, Plm_image_type* original_type)
{
    ShortImageType::Pointer img;

    /* If it is directory, then must be dicom */
    if (is_directory(fname)) {
#if defined (commentout)
	img = load_dicom_short (fname);
#endif
        print_and_exit ("Error: attempt to load DICOM using ITK reader.\n");
    } else {
	img = itk_image_load_any (fname, original_type, static_cast<short>(0));
    }
    return itk_image_load_postprocess (img);
}
