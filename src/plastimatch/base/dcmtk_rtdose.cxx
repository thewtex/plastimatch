/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdlib.h>
#include <stdio.h>
#include "dcmtk_config.h"
#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/dcmdata/dctk.h"

#include "dcmtk_file.h"
#include "dcmtk_loader.h"
#include "dcmtk_metadata.h"
#include "dcmtk_rtdose.h"
#include "dcmtk_save.h"
#include "dcmtk_save_p.h"
#include "dcmtk_series.h"
#include "dicom_rt_study.h"
#include "file_util.h"
#include "logfile.h"
#include "plm_image.h"
#include "plm_math.h"
#include "plm_uid_prefix.h"
#include "plm_version.h"
#include "print_and_exit.h"
#include "string_util.h"
#include "volume.h"
#include "volume_stats.h"

bool 
dcmtk_dose_probe (const char *fn)
{
    DcmFileFormat dfile;
    OFCondition ofrc = dfile.loadFile (fn, EXS_Unknown, EGL_noChange);
    if (ofrc.bad()) {
        return false;
    }

    const char *c;
    DcmDataset *dset = dfile.getDataset();
    ofrc = dset->findAndGetString (DCM_Modality, c);
    if (ofrc.bad() || !c) {
        return false;
    }

    if (strncmp (c, "RTDOSE", strlen("RTDOSE"))) {
	return false;
    } else {
	return true;
    }
}


/* This is the tolerance on irregularity of the grid spacing (in mm) */
#define GFOV_SPACING_TOL (1e-1)

template <class T> 
void
dcmtk_dose_copy (float *img_out, T *img_in, int nvox, float scale)
{
    for (int i = 0; i < nvox; i++) {
	img_out[i] = img_in[i] * scale;
    }
}

void
Dcmtk_loader::rtdose_load ()
{
    int rc;
    const char *val;
    uint16_t val_u16;
    plm_long dim[3];
    float ipp[3];
    float spacing[3];
    float *gfov;    /* gfov = GridFrameOffsetVector */
    plm_long gfov_len;
    const char *gfov_str;

    /* Modality -- better be RTDOSE */
    std::string modality = this->ds_rtdose->get_modality();
    if (modality == "RTDOSE") {
        printf ("Trying to load rt dose.\n");
    } else {
        print_and_exit ("Oops.\n");
    }

    /* FIX: load metadata such as patient name, etc. */

    /* ImagePositionPatient */
    val = this->ds_rtdose->get_cstr (DCM_ImagePositionPatient);
    if (!val) {
        print_and_exit ("Couldn't find DCM_ImagePositionPatient in rtdose\n");
    }
    rc = sscanf (val, "%f\\%f\\%f", &ipp[0], &ipp[1], &ipp[2]);
    if (rc != 3) {
	print_and_exit ("Error parsing RTDOSE ipp.\n");
    }

    /* Rows */
    if (!this->ds_rtdose->get_uint16 (DCM_Rows, &val_u16)) {
        print_and_exit ("Couldn't find DCM_Rows in rtdose\n");
    }
    dim[1] = val_u16;

    /* Columns */
    if (!this->ds_rtdose->get_uint16 (DCM_Columns, &val_u16)) {
        print_and_exit ("Couldn't find DCM_Columns in rtdose\n");
    }
    dim[0] = val_u16;

    /* PixelSpacing */
    val = this->ds_rtdose->get_cstr (DCM_PixelSpacing);
    if (!val) {
        print_and_exit ("Couldn't find DCM_PixelSpacing in rtdose\n");
    }
    rc = sscanf (val, "%g\\%g", &spacing[1], &spacing[0]);
    if (rc != 2) {
	print_and_exit ("Error parsing RTDOSE pixel spacing.\n");
    }

    /* GridFrameOffsetVector */
    val = this->ds_rtdose->get_cstr (DCM_GridFrameOffsetVector);
    if (!val) {
        print_and_exit ("Couldn't find DCM_GridFrameOffsetVector in rtdose\n");
    }
    gfov = 0;
    gfov_len = 0;
    gfov_str = val;
    while (1) {
	int len;
	gfov = (float*) realloc (gfov, (gfov_len + 1) * sizeof(float));
	rc = sscanf (gfov_str, "%g%n", &gfov[gfov_len], &len);
	if (rc != 1) {
	    break;
	}
	gfov_len ++;
	gfov_str += len;
	if (gfov_str[0] == '\\') {
	    gfov_str ++;
	}
    }
    dim[2] = gfov_len;
    if (gfov_len == 0) {
	print_and_exit ("Error parsing RTDOSE gfov.\n");
    }

    /* --- Analyze GridFrameOffsetVector --- */

    /* (1) Make sure first element is 0. */
    if (gfov[0] != 0.) {
	if (gfov[0] == ipp[2]) {
	    /* In this case, gfov values are absolute rather than relative 
	       positions, but we process the same way. */
	} else {
	    /* This is wrong.  But Nucletron does it. */
	    logfile_printf (
		"Warning: RTDOSE gfov[0] is neither 0 nor ipp[2].\n"
		"This violates the DICOM standard.  Proceeding anyway...\n");
	    /* Nucletron seems to work by ignoring absolute offset (???) */
	}
    }

    /* (2) Handle case where gfov_len == 1 (only one slice). */
    if (gfov_len == 1) {
	spacing[2] = spacing[0];
    }

    /* (3) Check to make sure spacing is regular. */
    for (plm_long i = 1; i < gfov_len; i++) {
	if (i == 1) {
	    spacing[2] = gfov[1] - gfov[0];
	} else {
	    float sp = gfov[i] - gfov[i-1];
	    if (fabs(sp - spacing[2]) > GFOV_SPACING_TOL) {
		print_and_exit ("Error RTDOSE grid has irregular spacing:"
		    "%f vs %f.\n", sp, spacing[2]);
	    }
	}
    }

    /* DoseGridScaling -- if element doesn't exist, scaling is 1.0 */
    float dose_scaling = 1.0;
    val = this->ds_rtdose->get_cstr (DCM_DoseGridScaling);
    if (val) {
        /* No need to check for success, let scaling be 1.0 if failure */
        sscanf (val, "%f", &dose_scaling);
    }

    printf ("RTDOSE: dim = %d %d %d\n        %f %f %f\n        %f %f %f\n",
        (int) dim[0], (int) dim[1], (int) dim[2],
        ipp[0], ipp[1], ipp[2], 
        spacing[0], spacing[1], spacing[2]);

    uint16_t bits_alloc, bits_stored, high_bit, pixel_rep;
    rc = this->ds_rtdose->get_uint16 (DCM_BitsAllocated, &bits_alloc);
    if (!rc) {
        print_and_exit ("Couldn't find DCM_BitsAllocated in rtdose\n");
    }
    rc = this->ds_rtdose->get_uint16 (DCM_BitsStored, &bits_stored);
    if (!rc) {
        print_and_exit ("Couldn't find DCM_BitsStored in rtdose\n");
    }
    rc = this->ds_rtdose->get_uint16 (DCM_HighBit, &high_bit);
    if (!rc) {
        print_and_exit ("Couldn't find DCM_HighBit in rtdose\n");
    }
    rc = this->ds_rtdose->get_uint16 (DCM_PixelRepresentation, &pixel_rep);
    if (!rc) {
        print_and_exit ("Couldn't find DCM_PixelRepresentation in rtdose\n");
    }

    printf ("Bits_alloc: %d\n", (int) bits_alloc);
    printf ("Bits_stored: %d\n", (int) bits_stored);
    printf ("High_bit: %d\n", (int) high_bit);
    printf ("Pixel_rep: %d\n", (int) pixel_rep);

    /* Create output dose image */
    delete this->dose;
    this->dose = new Plm_image;

    /* Create Volume */
    Volume *vol = new Volume (dim, ipp, spacing, 0, PT_FLOAT, 1);
    float *img = (float*) vol->img;

    /* Bind volume to plm_image */
    this->dose->set_gpuit (vol);

    /* PixelData */
    unsigned long length = 0;
    if (pixel_rep == 0) {
        const uint16_t* pixel_data;
        rc = this->ds_rtdose->get_uint16_array (
            DCM_PixelData, &pixel_data, &length);
        printf ("rc = %d, length = %lu, npix = %ld\n", 
            rc, length, (long) vol->npix);
        if (bits_stored == 16) {
            dcmtk_dose_copy (img, (const uint16_t*) pixel_data, 
                vol->npix, dose_scaling);
        } else if (bits_stored == 32) {
            dcmtk_dose_copy (img, (const uint32_t*) pixel_data, 
                vol->npix, dose_scaling);
        } else {
            delete this->dose;
            this->dose = 0;
            print_and_exit ("Unknown pixel representation (%d %d)\n",
                bits_stored, pixel_rep);
        }
    } else {
        const int16_t* pixel_data;
        rc = this->ds_rtdose->get_int16_array (
            DCM_PixelData, &pixel_data, &length);
        if (bits_stored == 16) {
            dcmtk_dose_copy (img, (const int16_t*) pixel_data, 
                vol->npix, dose_scaling);
        } else if (bits_stored == 32) {
            dcmtk_dose_copy (img, (const int32_t*) pixel_data, 
                vol->npix, dose_scaling);
        } else {
            delete this->dose;
            this->dose = 0;
            print_and_exit ("Unknown pixel representation (%d %d)\n",
                bits_stored, pixel_rep);
        }
    }
}

void
Dcmtk_save::save_dose (
    const char *dicom_dir)
{
    OFCondition ofc;
    std::string s;
    const Metadata *dose_metadata = 0;
    if (d_ptr->m_drs) {
        dose_metadata = d_ptr->m_drs->get_dose_metadata ();
    }

    /* Prepare output file */
    std::string filename = string_format ("%s/dose.dcm", dicom_dir);
    make_directory_recursive (filename);

    /* Prepare dcmtk */
    DcmFileFormat fileformat;
    DcmDataset *dataset = fileformat.getDataset();

    /* ----------------------------------------------------------------- */
    /*     Part 1  -- General header                                     */
    /* ----------------------------------------------------------------- */
    dataset->putAndInsertString (DCM_ImageType, 
        "DERIVED\\SECONDARY\\REFORMATTED");
    dataset->putAndInsertOFStringArray(DCM_InstanceCreationDate, 
        d_ptr->m_drs->get_study_date());
    dataset->putAndInsertOFStringArray(DCM_InstanceCreationTime, 
        d_ptr->m_drs->get_study_time());
    dataset->putAndInsertOFStringArray(DCM_InstanceCreatorUID, 
        PLM_UID_PREFIX);
    dataset->putAndInsertString (DCM_SOPClassUID, UID_RTDoseStorage);
    dataset->putAndInsertString (DCM_SOPInstanceUID, 
        d_ptr->m_drs->get_dose_instance_uid());
    dataset->putAndInsertOFStringArray (DCM_StudyDate, 
        d_ptr->m_drs->get_study_date());
    dataset->putAndInsertOFStringArray (DCM_StudyTime, 
        d_ptr->m_drs->get_study_time());
    dataset->putAndInsertOFStringArray (DCM_AccessionNumber, "");
    dataset->putAndInsertOFStringArray (DCM_Modality, "RTDOSE");
    dataset->putAndInsertString (DCM_Manufacturer, "Plastimatch");
    dataset->putAndInsertString (DCM_InstitutionName, "");
    dataset->putAndInsertString (DCM_ReferringPhysicianName, "");
    dataset->putAndInsertString (DCM_StationName, "");
    dcmtk_copy_from_metadata (dataset, dose_metadata, 
        DCM_SeriesDescription, "");
    dataset->putAndInsertString (DCM_ManufacturerModelName, "Plastimatch");
    dcmtk_copy_from_metadata (dataset, dose_metadata, DCM_PatientName, "");
    dcmtk_copy_from_metadata (dataset, dose_metadata, DCM_PatientID, "");
    dataset->putAndInsertString (DCM_PatientBirthDate, "");
    dcmtk_copy_from_metadata (dataset, dose_metadata, DCM_PatientSex, "O");
    dataset->putAndInsertString (DCM_SliceThickness, "");
    dataset->putAndInsertString (DCM_SoftwareVersions,
        PLASTIMATCH_VERSION_STRING);
    dataset->putAndInsertString (DCM_StudyInstanceUID, 
        d_ptr->m_drs->get_study_uid());
    dataset->putAndInsertString (DCM_SeriesInstanceUID, 
        d_ptr->m_drs->get_dose_series_uid());
    dcmtk_copy_from_metadata (dataset, dose_metadata, DCM_StudyID, "10001");
    dataset->putAndInsertString (DCM_SeriesNumber, "");
    dataset->putAndInsertString (DCM_InstanceNumber, "1");
    s = string_format ("%g\\%g\\%g", 
	this->dose->offset[0], this->dose->offset[1], this->dose->offset[2]);
    /* GCS FIX: PatientOrientation */
    dataset->putAndInsertString (DCM_PatientOrientation, "L/P");
    dataset->putAndInsertString (DCM_ImagePositionPatient, s.c_str());
    s = string_format ("%g\\%g\\%g\\%g\\%g\\%g",
	this->dose->direction_cosines[0],
	this->dose->direction_cosines[1],
	this->dose->direction_cosines[2],
	this->dose->direction_cosines[3],
	this->dose->direction_cosines[4],
	this->dose->direction_cosines[5]);
    dataset->putAndInsertString (DCM_ImageOrientationPatient, s.c_str());
    dataset->putAndInsertString (DCM_FrameOfReferenceUID, 
        d_ptr->m_drs->get_frame_of_reference_uid());

    dataset->putAndInsertString (DCM_SamplesPerPixel, "1");
    dataset->putAndInsertString (DCM_PhotometricInterpretation, "MONOCHROME2");
    s = string_format ("%d", (int) this->dose->dim[2]);
    dataset->putAndInsertString (DCM_NumberOfFrames, s.c_str());

    /* GCS FIX: Add FrameIncrementPointer */
    dataset->putAndInsertString (DCM_FrameIncrementPointer, 
        "(3004,000c)");

    dataset->putAndInsertUint16 (DCM_Rows, this->dose->dim[1]);
    dataset->putAndInsertUint16 (DCM_Columns, this->dose->dim[0]);
    s = string_format ("%g\\%g", 
        this->dose->spacing[1], this->dose->spacing[0]);
    dataset->putAndInsertString (DCM_PixelSpacing, s.c_str());

    dataset->putAndInsertString (DCM_BitsAllocated, "32");
    dataset->putAndInsertString (DCM_BitsStored, "32");
    dataset->putAndInsertString (DCM_HighBit, "31");
    if (dose_metadata 
        && dose_metadata->get_metadata(0x3004, 0x0004) == "ERROR")
    {
        dataset->putAndInsertString (DCM_PixelRepresentation, "1");
    } else {
        dataset->putAndInsertString (DCM_PixelRepresentation, "0");
    }

    dataset->putAndInsertString (DCM_DoseUnits, "GY");
    dcmtk_copy_from_metadata (dataset, dose_metadata, 
        DCM_DoseType, "PHYSICAL");
    dataset->putAndInsertString (DCM_DoseSummationType, "PLAN");

    s = std::string ("0");
    for (int i = 1; i < this->dose->dim[2]; i++) {
	s += string_format ("\\%g", i * this->dose->spacing[2]);
    }
    dataset->putAndInsertString (DCM_GridFrameOffsetVector, s.c_str());
    
    /* GCS FIX:
       Leave ReferencedRTPlanSequence empty (until I can cross reference) */

    /* We need to convert image to uint16_t, but first we need to 
       scale it so that the maximum dose fits in a 16-bit unsigned 
       integer.  Compute an appropriate scaling factor based on the 
       maximum dose. */

    /* Copy the image so we don't corrupt the original */
    //Volume *dose_copy = this->dose->clone();
    Volume::Pointer dose_copy = this->dose->clone();

    /* Find the maximum value in the image */
    double min_val, max_val, avg;
    int non_zero, num_vox;
    dose_copy->convert (PT_FLOAT);
    volume_stats (dose_copy, &min_val, &max_val, &avg, &non_zero, &num_vox);

    /* Find scale factor */
    float dose_scale;
    if (dose_metadata 
        && dose_metadata->get_metadata(0x3004, 0x0004) == "ERROR")
    {
	/* Dose error is signed integer */
	float dose_scale_min = min_val / INT32_T_MIN * 1.001;
	float dose_scale_max = max_val / INT32_T_MAX * 1.001;
	dose_scale = std::max(dose_scale_min, dose_scale_max);
    } else {
        /* Dose is unsigned integer */
        dose_scale = max_val / UINT32_T_MAX * 1.001;
    }

    /* Scale the image and add scale factor to dataset */
    volume_scale (dose_copy.get(), 1 / dose_scale);
    s = string_format ("%g", dose_scale);
    dataset->putAndInsertString (DCM_DoseGridScaling, s.c_str());

    /* Convert image bytes to integer, then add to dataset */
    if (dose_metadata 
        && dose_metadata->get_metadata(0x3004, 0x0004) == "ERROR")
    {
	dose_copy->convert (PT_INT32);
        dataset->putAndInsertSint16Array (DCM_PixelData, 
            (Sint16*) dose_copy->img, 2*dose_copy->npix);
    } else {
	dose_copy->convert (PT_UINT32);
        dataset->putAndInsertUint16Array (DCM_PixelData, 
            (Uint16*) dose_copy->img, 2*dose_copy->npix);
    }

    /* ----------------------------------------------------------------- */
    /*     Write the output file                                         */
    /* ----------------------------------------------------------------- */
    ofc = fileformat.saveFile (filename.c_str(), EXS_LittleEndianExplicit);
    if (ofc.bad()) {
        print_and_exit ("Error: cannot write DICOM RTDOSE (%s)\n", 
            ofc.text());
    }

    /* Delete the dose copy */
    // delete dose_copy;
}
