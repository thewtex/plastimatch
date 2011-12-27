/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _advantech_h_
#define _advantech_h_

#include "ise_config.h"
#include <windows.h>

class Advantech {
public:
    Advantech ();
    ~Advantech ();
public:
    bool have_device;
    ULONG device_number;
    LONG driver_handle;
};

#endif
