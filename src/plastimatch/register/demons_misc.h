/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _demons_misc_h_
#define _demons_misc_h_

#include "plmregister_config.h"

PLMREGISTER_C_API float* create_ker (float coeff, int half_width);
PLMREGISTER_C_API void validate_filter_widths (int *fw_out, int *fw_in);
PLMREGISTER_C_API void kernel_stats (float* kerx, float* kery, float* kerz, int fw[]);

#endif
