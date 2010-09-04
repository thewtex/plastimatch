/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _pcmd_compare_h_
#define _pcmd_compare_h_

#include "plm_config.h"
#include <string.h>
#include <stdlib.h>
#include "bstrwrap.h"

class Compare_parms {
public:
    CBString img_in_1_fn;
    CBString img_in_2_fn;
};

void do_command_compare (int argc, char *argv[]);

#endif
