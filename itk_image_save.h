/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_image_save_h_
#define _itk_image_save_h_

#include "plm_config.h"
#include "itk_image.h"

/* -----------------------------------------------------------------------
   Function prototypes
   ----------------------------------------------------------------------- */
template<class T> void itk_image_save (T img_ptr, const char* fname);
template<class T> void itk_image_save_short_dicom (T image, const char* dir_name, Plm_image_patient_position);
template<class T> plastimatch1_EXPORT void itk_image_save_uchar (T img_ptr, char* fname);
template<class T> plastimatch1_EXPORT void itk_image_save_short (T img_ptr, char* fname);
template<class T> plastimatch1_EXPORT void itk_image_save_ushort (T img_ptr, char* fname);
template<class T> plastimatch1_EXPORT void itk_image_save_uint32 (T img_ptr, char* fname);
template<class T> plastimatch1_EXPORT void itk_image_save_float (T img_ptr, const char* fname);
#endif
