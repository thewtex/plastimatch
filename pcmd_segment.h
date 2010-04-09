/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _pcmd_segment_h_
#define _pcmd_segment_h_

#include "plm_config.h"
#include <string.h>
#include <stdlib.h>
#include "plm_path.h"

class Segment_parms {
public:
    char img_in_fn[_MAX_PATH];
    char img_out_fn[_MAX_PATH];
    int thumbnail_dim;
    float thumbnail_spacing;
public:
    Segment_parms () {
	img_in_fn[0] = 0;
	strcpy (img_out_fn, "thumb.mhd");
	thumbnail_dim = 16;
	thumbnail_spacing = 30.0;
    }
};

void
do_command_segment (int argc, char *argv[]);

#endif
