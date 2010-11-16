/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _referenced_dicom_dir_h_
#define _referenced_dicom_dir_h_

#include "plm_config.h"
#include <vector>
#include "cxt_io.h"
#include "demographics.h"
#include "plm_image_header.h"

class Referenced_dicom_dir {
public:
    bool m_loaded;
    Plm_image_header m_pih;
    Demographics m_demographics;
    CBString m_study_id;
    CBString m_ct_study_uid;
    CBString m_ct_series_uid;
    CBString m_ct_fref_uid;
    //std::list<CBString> m_ct_slice_uids;
    std::vector<CBString> m_ct_slice_uids;
public:
    Referenced_dicom_dir ();
    ~Referenced_dicom_dir ();
    void load (const char *dicom_dir);
    void get_slice_info (int *slice_no, CBString *ct_slice_uid, float z);

};

#if defined __cplusplus
extern "C" {
#endif

plastimatch1_EXPORT
void
cxt_apply_dicom_dir (Rtss *cxt, const char *dicom_dir);

#if defined __cplusplus
}
#endif

#endif
