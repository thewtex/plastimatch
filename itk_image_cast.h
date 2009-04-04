/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_image_cast_h_
#define _itk_image_cast_h_

#include "plm_config.h"
#include "itk_image.h"

/* -----------------------------------------------------------------------
   Function prototypes
   ----------------------------------------------------------------------- */
template<class T> plastimatch1_EXPORT UCharImageType::Pointer cast_uchar (T image);
template<class T> plastimatch1_EXPORT ShortImageType::Pointer cast_short (T image);
template<class T> plastimatch1_EXPORT FloatImageType::Pointer cast_float (T image);
#endif
