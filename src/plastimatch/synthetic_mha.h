/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _synthetic_mha_h_
#define _synthetic_mha_h_

#include "plm_config.h"
#include "direction_cosines.h"
#include "itk_image.h"

class Rtds;

enum Pattern_type {
    PATTERN_GAUSS,
    PATTERN_RECT,
    PATTERN_SPHERE,
    PATTERN_ENCLOSED_RECT,
    PATTERN_OBJSTRUCTDOSE
};

enum Pattern_structset_type {
    PATTERN_SS_ONE,
    PATTERN_SS_TWO_APART,
    PATTERN_SS_TWO_OVERLAP_PLUS_ONE,
    PATTERN_SS_TWO_OVERLAP_PLUS_ONE_PLUS_EMBED
};

class Synthetic_mha_parms {
public:
    int output_type;
    Pattern_type pattern;
    int dim[3];
    float origin[3];
    float spacing[3];
    Direction_cosines dc;

    float background;
    float foreground;
    float gauss_center[3];
    float gauss_std[3];
    float rect_size[6];
    float sphere_center[3];
    float sphere_radius[3];
    
    float enclosed_intens_f1, enclosed_intens_f2;
    float enclosed_xlat1[3], enclosed_xlat2[3];

    bool m_want_ss_img;
    bool m_want_dose_img;

    Pattern_structset_type pattern_ss;

public:
    Synthetic_mha_parms () {
	output_type = PLM_IMG_TYPE_ITK_FLOAT;
	pattern = PATTERN_GAUSS;
	for (int i = 0; i < 3; i++) {
	    spacing[i] = 5.0f;
	    dim[i] = 100;
	    origin[i] = 0.0f;
	    gauss_center[i] = 0.0f;
	    gauss_std[i] = 100.0f;
	    sphere_center[i] = 0.0f;
	    sphere_radius[i] = 50.0f;
	}
	background = -1000.0f;
	foreground = 0.0f;
	rect_size[0] = -50.0f;
	rect_size[1] = +50.0f;
	rect_size[2] = -50.0f;
	rect_size[3] = +50.0f;
	rect_size[4] = -50.0f;
	rect_size[5] = +50.0f;
	m_want_ss_img = false;
	m_want_dose_img = false;
    
	pattern_ss = PATTERN_SS_ONE;
    }
};

plastimatch1_EXPORT void synthetic_mha (Rtds *rtds, 
    Synthetic_mha_parms *parms);

#endif
