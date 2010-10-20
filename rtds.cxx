/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astroid_dose.h"
#include "bstring_util.h"
#include "cxt_apply_dicom.h"
#include "cxt_extract.h"
#include "file_util.h"
#include "gdcm_dose.h"
#include "gdcm_rtss.h"
#include "mc_dose.h"
#include "plm_image_patient_position.h"
#include "rtds.h"
#include "rtds_dicom.h"
#include "ss_list_io.h"
#include "xio_ct.h"
#include "xio_dir.h"
#include "xio_dose.h"
#include "xio_io.h"
#include "xio_structures.h"

void
Rtds::load_dicom_dir (const char *dicom_dir)
{
    const char *dicom_dir_tmp;  /* In case dicom_dir is a file, not dir */

    /* Use existing itk reader for the image.
       This is required because the native dicom reader doesn't yet 
       handle things like MR. */

    if (is_directory (dicom_dir)) {
	dicom_dir_tmp = dicom_dir;
    } else {
	dicom_dir_tmp = file_util_dirname (dicom_dir);
    }
    this->m_img = plm_image_load_native (dicom_dir_tmp);

    rtds_dicom_load (this, dicom_dir_tmp);

    if (dicom_dir_tmp != dicom_dir) {
	free ((void*) dicom_dir_tmp);
    }
}

void
Rtds::load_xio (
    const char *xio_dir,
    const char *dicom_dir,
    Plm_image_patient_position patient_pos
)
{
    Xio_dir *xd;
    Xio_patient_dir *xpd;
    Xio_studyset_dir *xsd;
    Xio_plan_dir *xtpd;

    xd = xio_dir_create (xio_dir);

    if (xd->num_patient_dir <= 0) {
	print_and_exit ("Error, xio num_patient_dir = %d\n", 
	    xd->num_patient_dir);
    }
    xpd = &xd->patient_dir[0];
    if (xd->num_patient_dir > 1) {
	printf ("Warning: multiple patients found in xio directory.\n"
	    "Defaulting to first directory: %s\n", xpd->path);
    }

    if (xpd->num_plan_dir > 0) {

	/* When plans exist, load the first plan */
	xtpd = &xpd->plan_dir[0];
	if (xpd->num_studyset_dir > 1) {
	    printf ("Warning: multiple plans found in xio patient directory.\n"
		"Defaulting to first directory: %s\n", xtpd->path);
	}

	/* Load the summed XiO dose file */
	this->m_dose = new Plm_image ();
	printf ("calling xio_dose_load\n");
	std::string xio_dose_file = std::string(xtpd->path) + "/dose.1";
	strncpy(this->m_xio_dose_input, xio_dose_file.c_str(), _MAX_PATH);
	xio_dose_load (this->m_dose, xio_dose_file.c_str());

	/* Find studyset associated with plan */
	xsd = xio_plan_dir_get_studyset_dir (xtpd);

    } else {

	/* No plans exist, load only studyset */

	if (xpd->num_studyset_dir <= 0) {
	    print_and_exit ("Error, xio patient has no studyset.");
	}

	printf ("Warning: no plans found, only loading studyset.");

	xsd = &xpd->studyset_dir[0];
	if (xpd->num_studyset_dir > 1) {
	    printf ("Warning: multiple studyset found in xio patient directory.\n"
	    "Defaulting to first directory: %s\n", xsd->path);
	}

    }

    /* Load the XiO studyset CT images */
    this->m_img = new Plm_image;
    printf ("calling xio_ct_load\n");
    printf("path is :: %s\n", xsd->path);
    xio_ct_load (this->m_img, xsd->path);

    /* Load the XiO studyset structure set */
    this->m_ss_image = new Ss_image;
    this->m_ss_image->load_xio (xsd->path);

    /* Apply XiO CT geometry to structures */
    if (this->m_ss_image->m_cxt) {
	printf ("calling cxt_set_geometry_from_plm_image\n");
	this->m_ss_image->m_cxt->set_geometry_from_plm_image (this->m_img);
    }

    /* Set patient position for XiO CT */
    if (this->m_img) {
	if (patient_pos == PATIENT_POSITION_UNKNOWN && dicom_dir[0]) {
	    rtds_patient_pos_from_dicom_dir (this, dicom_dir);
	} else {
	    this->m_img->m_patient_pos = patient_pos;
	}
    }

    /* If a directory with original DICOM CT is provided,
       the structures will be associated with the original CT UIDs.
       The coordinates will be transformed from XiO to DICOM LPS
       with the same origin as the original CT.

       Otherwise, the XiO CT will be saved as DICOM and the structures
       will be associated to those slices. The coordinates will be
       transformed to DICOM LPS based on the --patient-pos command
       line parameter and the origin will remain the same. */

    if (this->m_img && dicom_dir[0]) {
	/* Determine transformation based original DICOM */
	xio_ct_get_transform_from_dicom_dir (
	    this->m_img, this->m_xio_transform, dicom_dir);
    } else {
	/* Determine transformation based on patient position */
	xio_ct_get_transform (this->m_img, this->m_xio_transform);
    }

    if (this->m_img) {
	xio_ct_apply_transform (this->m_img, this->m_xio_transform);
    }
    if (this->m_ss_image->m_cxt) {
	xio_structures_apply_transform (this->m_ss_image->m_cxt, 
	    this->m_xio_transform);
    }
    if (this->m_dose) {
	xio_dose_apply_transform (this->m_dose, this->m_xio_transform);
    }
}

void
Rtds::load_ss_img (const char *ss_img, const char *ss_list)
{
    this->m_ss_image = new Ss_image;
    this->m_ss_image->load (ss_img, ss_list);
}

void
Rtds::load_dose_img (const char *dose_img)
{
    if (this->m_dose) {
	delete this->m_dose;
    }
    if (dose_img) {
	this->m_dose = plm_image_load_native (dose_img);
    }
}

void
Rtds::load_dose_xio (const char *dose_xio)
{
    if (this->m_dose) {
	delete this->m_dose;
    }
    if (dose_xio) {
	strncpy(this->m_xio_dose_input, dose_xio, _MAX_PATH);
	this->m_dose = new Plm_image ();
	xio_dose_load (this->m_dose, dose_xio);
	xio_dose_apply_transform (this->m_dose, this->m_xio_transform);
    }
}

void
Rtds::load_dose_astroid (const char *dose_astroid)
{
    if (this->m_dose) {
	delete this->m_dose;
    }
    if (dose_astroid) {
	this->m_dose = new Plm_image ();
	astroid_dose_load (this->m_dose, dose_astroid);
	astroid_dose_apply_transform (this->m_dose, this->m_xio_transform);
    }
}

void
Rtds::load_dose_mc (const char *dose_mc)
{
    if (this->m_dose) {
	delete this->m_dose;
    }
    if (dose_mc) {
	this->m_dose = new Plm_image ();
	mc_dose_load (this->m_dose, dose_mc);
	mc_dose_apply_transform (this->m_dose, this->m_xio_transform);
    }
}

void
Rtds::save_dicom (const char *dicom_dir)
{
    if (this->m_img) {
	this->m_img->save_short_dicom (dicom_dir);
    }
    if (this->m_ss_image) {
	this->m_ss_image->save_gdcm_rtss (dicom_dir);
    }
    if (this->m_dose) {
	char fn[_MAX_PATH];
	snprintf (fn, _MAX_PATH, "%s/%s", dicom_dir, "dose.dcm");
	gdcm_dose_save (this->m_dose, fn);
    }
}

