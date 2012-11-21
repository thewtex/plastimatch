/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"

#include "dcm_probe.h"

#if PLM_DCM_USE_DCMTK
#include "dcmtk_rtdose.h"
#include "dcmtk_rtss.h"
#elif GDCM_VERSION_1
#include "gdcm1_dose.h"
#include "gdcm1_rtss.h"
#else /* GDCM_VERSION_2 */
/* Nothing */
#endif
#include "make_string.h"

/* Return true if the file is a dicom rtstruct */
bool
dcm_probe_rtss (const char *rtss_fn)
{
#if PLM_DCM_USE_DCMTK
    return dcmtk_rtss_probe (rtss_fn);
#elif GDCM_VERSION_1
    return gdcm_rtss_probe (rtss_fn);
#else /* GDCM_VERSION_2 */
    return false;
#endif
}

/* Return true if the file is a dicom rt dose */
bool
dcm_probe_dose (const char *fn)
{
#if PLM_DCM_USE_DCMTK
    return dcmtk_dose_probe (fn);
#elif GDCM_VERSION_1
    return gdcm1_dose_probe (fn);
#else /* GDCM_VERSION_2 */
    return false;
#endif
}
