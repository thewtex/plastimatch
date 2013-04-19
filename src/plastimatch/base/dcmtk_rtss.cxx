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
#include "dcmtk_metadata.h"
#include "dcmtk_loader.h"
#include "dcmtk_loader_p.h"
#include "dcmtk_rt_study.h"
#include "dcmtk_rt_study_p.h"
#include "dcmtk_save.h"
#include "dcmtk_save_p.h"
#include "dcmtk_series.h"
#include "dcmtk_slice_data.h"
#include "dicom_rt_study.h"
#include "file_util.h"
#include "logfile.h"
#include "metadata.h"
#include "plm_uid_prefix.h"
#include "plm_version.h"
#include "print_and_exit.h"
#include "rtss_structure.h"
#include "rtss_structure_set.h"

PLMBASE_C_API bool 
dcmtk_rtss_probe (const char *rtss_fn)
{
    DcmFileFormat dfile;
    OFCondition ofrc = dfile.loadFile (rtss_fn, EXS_Unknown, EGL_noChange);
    if (ofrc.bad()) {
        return false;
    }

    const char *c;
    DcmDataset *dset = dfile.getDataset();
    ofrc = dset->findAndGetString (DCM_Modality, c);
    if (ofrc.bad() || !c) {
        return false;
    }

    if (strncmp (c, "RTSTRUCT", strlen("RTSTRUCT"))) {
	return false;
    } else {
	return true;
    }
}

void
Dcmtk_loader::rtss_load (void)
{
    d_ptr->cxt = Rtss_structure_set::New();
    
    /* Modality -- better be RTSTRUCT */
    std::string modality = ds_rtss->get_modality();
    if (modality == "RTSTRUCT") {
        lprintf ("Trying to load rt structure set.\n");
    } else {
        print_and_exit ("Oops.\n");
    }

    /* FIX: load metadata such as patient name, etc. */

    /* ReferencedFrameOfReferenceSequence */
    DcmSequenceOfItems *seq = 0;
    bool rc = ds_rtss->get_sequence (
        DCM_ReferencedFrameOfReferenceSequence, seq);
    if (!rc) {
        lprintf ("Huh? Why no RFOR sequence???\n");
    }

    /* StructureSetROISequence */
    seq = 0;
    rc = ds_rtss->get_sequence (DCM_StructureSetROISequence, seq);
    if (rc) {
        for (unsigned long i = 0; i < seq->card(); i++) {
            int structure_id;
            OFCondition orc;
            const char *val = 0;
            orc = seq->getItem(i)->findAndGetString (DCM_ROINumber, val);
            if (!orc.good()) {
                continue;
            }
            if (1 != sscanf (val, "%d", &structure_id)) {
                continue;
            }
            val = 0;
            orc = seq->getItem(i)->findAndGetString (DCM_ROIName, val);
            lprintf ("Adding structure (%d), %s\n", structure_id, val);
            d_ptr->cxt->add_structure (
                Pstring (val), Pstring (), structure_id);
        }
    }

    /* ROIContourSequence */
    seq = 0;
    rc = ds_rtss->get_sequence (DCM_ROIContourSequence, seq);
    if (rc) {
        for (unsigned long i = 0; i < seq->card(); i++) {
            Rtss_structure *curr_structure;
            int structure_id;
            OFCondition orc;
            const char *val = 0;
            DcmItem *item = seq->getItem(i);

            /* Get ID and color */
            orc = item->findAndGetString (DCM_ReferencedROINumber, val);
            if (!orc.good()) {
                lprintf ("Error finding DCM_ReferencedROINumber.\n");
                continue;
            }
            if (1 != sscanf (val, "%d", &structure_id)) {
                continue;
            }
            val = 0;
            orc = item->findAndGetString (DCM_ROIDisplayColor, val);
            lprintf ("Structure %d has color %s\n", structure_id, val);

            /* Look up the structure for this id and set color */
            curr_structure = d_ptr->cxt->find_structure_by_id (structure_id);
            if (!curr_structure) {
                lprintf ("Couldn't reference structure with id %d\n", 
                    structure_id);
                continue;
            }
            curr_structure->set_color (val);

            /* ContourSequence */
            DcmSequenceOfItems *c_seq = 0;
            orc = item->findAndGetSequence (DCM_ContourSequence, c_seq);
            if (!orc.good()) {
                lprintf ("Error finding DCM_ContourSequence.\n");
                continue;
            }
            for (unsigned long j = 0; j < c_seq->card(); j++) {
		int i, p, n, contour_data_len;
		int num_points;
		const char *contour_geometric_type;
		const char *contour_data;
		const char *number_of_contour_points;
		Rtss_polyline *curr_polyline;
                DcmItem *c_item = c_seq->getItem(j);

		/* ContourGeometricType */
                orc = c_item->findAndGetString (DCM_ContourGeometricType, 
                    contour_geometric_type);
                if (!orc.good()) {
		    lprintf ("Error finding DCM_ContourGeometricType.\n");
                    continue;
                }
		if (strncmp (contour_geometric_type, "CLOSED_PLANAR", 
                        strlen("CLOSED_PLANAR"))) {
		    /* Might be "POINT".  Do I want to preserve this? */
		    lprintf ("Skipping geometric type: [%s]\n", 
                        contour_geometric_type);
		    continue;
		}

                /* NumberOfContourPoints */
                orc = c_item->findAndGetString (DCM_NumberOfContourPoints,
                    number_of_contour_points);
                if (!orc.good()) {
		    lprintf ("Error finding DCM_NumberOfContourPoints.\n");
                    continue;
                }
		if (1 != sscanf (number_of_contour_points, "%d", &num_points)) {
		    lprintf ("Error parsing number_of_contour_points...\n");
		    continue;
		}
		if (num_points <= 0) {
		    /* Polyline with zero points?  Skip it. */
		    continue;
		}

                /* ContourData */
                orc = c_item->findAndGetString (DCM_ContourData, contour_data);
                if (!orc.good()) {
		    lprintf ("Error finding DCM_ContourData.\n");
		    continue;
		}

		/* Create a new polyline for this structure */
		curr_polyline = curr_structure->add_polyline ();
		curr_polyline->slice_no = -1;
		//curr_polyline->ct_slice_uid = "";
		curr_polyline->num_vertices = num_points;
		curr_polyline->x = (float*) malloc (num_points * sizeof(float));
		curr_polyline->y = (float*) malloc (num_points * sizeof(float));
		curr_polyline->z = (float*) malloc (num_points * sizeof(float));

		/* Parse dicom data string */
		i = 0;
		n = 0;
		contour_data_len = strlen (contour_data);
		for (p = 0; p < 3 * num_points; p++) {
		    float f;
		    int this_n;
		
		    /* Skip \\ */
		    if (n < contour_data_len) {
			if (contour_data[n] == '\\') {
			    n++;
			}
		    }

		    /* Parse float value */
		    if (1 != sscanf (&contour_data[n], "%f%n", &f, &this_n)) {
			lprintf ("Error parsing data...\n");
			break;
		    }
		    n += this_n;

		    /* Put value into polyline */
		    switch (i) {
		    case 0:
			curr_polyline->x[p/3] = f;
			break;
		    case 1:
			curr_polyline->y[p/3] = f;
			break;
		    case 2:
			curr_polyline->z[p/3] = f;
			break;
		    }
		    i = (i + 1) % 3;
		}
            }
        }
    }
}

void
Dcmtk_rt_study::save_rtss (const char *dicom_dir)
{
    OFCondition ofc;
    Rtss_structure_set::Pointer& cxt = d_ptr->cxt;
    const Metadata *rtss_metadata = 0;
    if (d_ptr->dicom_metadata) {
        rtss_metadata = d_ptr->dicom_metadata->get_rtss_metadata ();
    }

    /* Prepare structure set with slice uids */
    const Slice_list *slice_list = d_ptr->dicom_metadata->get_slice_list ();
    cxt->apply_slice_list (slice_list);

    /* Prepare output file */
    Pstring rtss_fn;
    rtss_fn.format ("%s/rtss.dcm", dicom_dir);
    make_directory_recursive (rtss_fn);

    /* Prepare dcmtk */
    DcmFileFormat fileformat;
    DcmDataset *dataset = fileformat.getDataset();

    /* ----------------------------------------------------------------- */
    /*     Part 1  -- General header                                     */
    /* ----------------------------------------------------------------- */
    dataset->putAndInsertOFStringArray(DCM_InstanceCreationDate, 
        d_ptr->dicom_metadata->get_study_date());
    dataset->putAndInsertOFStringArray(DCM_InstanceCreationTime, 
        d_ptr->dicom_metadata->get_study_time());
    dataset->putAndInsertOFStringArray(DCM_InstanceCreatorUID, 
        PLM_UID_PREFIX);
    dataset->putAndInsertString (DCM_SOPClassUID, UID_RTStructureSetStorage);
    dataset->putAndInsertString (DCM_SOPInstanceUID, 
        d_ptr->dicom_metadata->get_rtss_instance_uid());
    dataset->putAndInsertOFStringArray (DCM_StudyDate, 
        d_ptr->dicom_metadata->get_study_date());
    dataset->putAndInsertOFStringArray (DCM_StudyTime, 
        d_ptr->dicom_metadata->get_study_time());
    dataset->putAndInsertOFStringArray (DCM_AccessionNumber, "");
    dataset->putAndInsertOFStringArray (DCM_Modality, "RTSTRUCT");
    dataset->putAndInsertString (DCM_Manufacturer, "Plastimatch");
    dataset->putAndInsertString (DCM_InstitutionName, "");
    dataset->putAndInsertString (DCM_ReferringPhysicianName, "");
    dataset->putAndInsertString (DCM_StationName, "");
    dcmtk_copy_from_metadata (dataset, rtss_metadata, 
        DCM_SeriesDescription, "");
    dataset->putAndInsertString (DCM_ManufacturerModelName, "Plastimatch");
    dcmtk_copy_from_metadata (dataset, rtss_metadata, DCM_PatientName, "");
    dcmtk_copy_from_metadata (dataset, rtss_metadata, DCM_PatientID, "");
    dataset->putAndInsertString (DCM_PatientBirthDate, "");
    dcmtk_copy_from_metadata (dataset, rtss_metadata, DCM_PatientSex, "O");
    dataset->putAndInsertString (DCM_SoftwareVersions,
        PLASTIMATCH_VERSION_STRING);

#if defined (commentout)
    /* GCS FIX */
    /* PatientPosition */
    // gf->InsertValEntry (xxx, 0x0018, 0x5100);
#endif

    dataset->putAndInsertString (DCM_StudyInstanceUID, 
        d_ptr->dicom_metadata->get_study_uid());
    dataset->putAndInsertString (DCM_SeriesInstanceUID, 
        d_ptr->dicom_metadata->get_rtss_series_uid());
    dcmtk_copy_from_metadata (dataset, rtss_metadata, DCM_StudyID, "");
    dataset->putAndInsertString (DCM_SeriesNumber, "103");
    dataset->putAndInsertString (DCM_InstanceNumber, "1");
    dataset->putAndInsertString (DCM_StructureSetLabel, "AutoSS");
    dataset->putAndInsertString (DCM_StructureSetName, "AutoSS");
    dataset->putAndInsertOFStringArray (DCM_StructureSetDate, 
        d_ptr->dicom_metadata->get_study_date());
    dataset->putAndInsertOFStringArray (DCM_StructureSetTime, 
        d_ptr->dicom_metadata->get_study_time());

    /* ----------------------------------------------------------------- */
    /*     Part 2  -- UID's for CT series                                */
    /* ----------------------------------------------------------------- */
    DcmSequenceOfItems *rfor_seq = 0;
    DcmItem *rfor_item = 0;
    dataset->findOrCreateSequenceItem (
        DCM_ReferencedFrameOfReferenceSequence, rfor_item, -2);
    rfor_item->putAndInsertString (DCM_FrameOfReferenceUID, 
        d_ptr->dicom_metadata->get_frame_of_reference_uid());
    dataset->findAndGetSequence (
        DCM_ReferencedFrameOfReferenceSequence, rfor_seq);
    DcmItem *rtrstudy_item = 0;
    rfor_item->findOrCreateSequenceItem (
        DCM_RTReferencedStudySequence, rtrstudy_item, -2);
    rtrstudy_item->putAndInsertString (
        DCM_ReferencedSOPClassUID, 
        UID_RETIRED_StudyComponentManagementSOPClass);
    rtrstudy_item->putAndInsertString (
        DCM_ReferencedSOPInstanceUID, d_ptr->dicom_metadata->get_study_uid());
    DcmItem *rtrseries_item = 0;
    rtrstudy_item->findOrCreateSequenceItem (
        DCM_RTReferencedSeriesSequence, rtrseries_item, -2);
    rtrseries_item->putAndInsertString (
        DCM_SeriesInstanceUID, d_ptr->dicom_metadata->get_ct_series_uid());

    for (int k = 0; k < d_ptr->dicom_metadata->num_slices(); k++) {
        DcmItem *ci_item = 0;
        rtrseries_item->findOrCreateSequenceItem (
            DCM_ContourImageSequence, ci_item, -2);
        ci_item->putAndInsertString (
            DCM_ReferencedSOPClassUID, UID_CTImageStorage);
        ci_item->putAndInsertString (
            DCM_ReferencedSOPInstanceUID, 
            d_ptr->dicom_metadata->get_slice_uid (k));
    }

    /* ----------------------------------------------------------------- */
    /*     Part 3  -- Structure info                                     */
    /* ----------------------------------------------------------------- */
    for (size_t i = 0; i < cxt->num_structures; i++) {
        DcmItem *ssroi_item = 0;
        Pstring tmp;
        dataset->findOrCreateSequenceItem (
            DCM_StructureSetROISequence, ssroi_item, -2);
        tmp.format ("%d", cxt->slist[i]->id);
        ssroi_item->putAndInsertString (DCM_ROINumber, tmp.c_str());
        ssroi_item->putAndInsertString (DCM_ReferencedFrameOfReferenceUID,
            d_ptr->dicom_metadata->get_frame_of_reference_uid());
        ssroi_item->putAndInsertString (DCM_ROIName, cxt->slist[i]->name);
        ssroi_item->putAndInsertString (DCM_ROIGenerationAlgorithm, "");
    }

    /* ----------------------------------------------------------------- */
    /*     Part 4  -- Contour info                                       */
    /* ----------------------------------------------------------------- */
    for (size_t i = 0; i < cxt->num_structures; i++) {
	Rtss_structure *curr_structure = cxt->slist[i];
        DcmItem *roic_item = 0;
	Pstring tmp;
        dataset->findOrCreateSequenceItem (
            DCM_ROIContourSequence, roic_item, -2);
        curr_structure->get_dcm_color_string (&tmp);
        roic_item->putAndInsertString (DCM_ROIDisplayColor, tmp.c_str());
	for (size_t j = 0; j < curr_structure->num_contours; j++) {
	    Rtss_polyline *curr_contour = curr_structure->pslist[j];
	    if (curr_contour->num_vertices <= 0) continue;

	    /* GE -> XiO transfer does not work if contour does not have 
	       corresponding slice uid */
	    if (curr_contour->ct_slice_uid.empty()) {
		printf ("Warning: Omitting contour (%ld,%ld)\n", 
                    (long) i, (long) j);
		continue;
	    }

	    /* ContourImageSequence */
            DcmItem *c_item = 0;
            roic_item->findOrCreateSequenceItem (
                DCM_ContourSequence, c_item, -2);
            DcmItem *ci_item = 0;
            c_item->findOrCreateSequenceItem (
                DCM_ContourImageSequence, ci_item, -2);
            ci_item->putAndInsertString (DCM_ReferencedSOPClassUID,
                UID_CTImageStorage);
            ci_item->putAndInsertString (DCM_ReferencedSOPInstanceUID,
                curr_contour->ct_slice_uid.c_str());

            /* ContourGeometricType */
            c_item->putAndInsertString (DCM_ContourGeometricType, 
                "CLOSED_PLANAR");

            /* NumberOfContourPoints */
            tmp.format ("%d", curr_contour->num_vertices);
            c_item->putAndInsertString (DCM_NumberOfContourPoints, tmp);

	    /* ContourData */
            tmp.format ("%g\\%g\\%g", 
                curr_contour->x[0],
                curr_contour->y[0],
                curr_contour->z[0]);
	    for (int k = 1; k < curr_contour->num_vertices; k++) {
                Pstring tmp2;
                tmp2.format ("\\%g\\%g\\%g",
		    curr_contour->x[k],
		    curr_contour->y[k],
		    curr_contour->z[k]);
                tmp += tmp2;
	    }
            c_item->putAndInsertString (DCM_ContourData, tmp);
        }

        tmp.format ("%d", (int) curr_structure->id);
        roic_item->putAndInsertString (DCM_ReferencedROINumber, tmp);
    }

    /* ----------------------------------------------------------------- */
    /*     Part 5  -- More structure info                                */
    /* ----------------------------------------------------------------- */
    for (size_t i = 0; i < cxt->num_structures; i++) {
	Rtss_structure *curr_structure = cxt->slist[i];
	Pstring tmp;

        /* RTROIObservationsSequence */
        DcmItem *rtroio_item = 0;
        dataset->findOrCreateSequenceItem (
            DCM_RTROIObservationsSequence, rtroio_item, -2);

	/* ObservationNumber */
        tmp.format ("%d", (int) curr_structure->id);
	rtroio_item->putAndInsertString (DCM_ObservationNumber, tmp);
	/* ReferencedROINumber */
	rtroio_item->putAndInsertString (DCM_ReferencedROINumber, tmp);
	/* ROIObservationLabel */
	rtroio_item->putAndInsertString (DCM_ROIObservationLabel, 
	    (const char*) curr_structure->name);
	/* RTROIInterpretedType */
	rtroio_item->putAndInsertString (DCM_RTROIInterpretedType, "");
	/* ROIInterpreter */
	rtroio_item->putAndInsertString (DCM_ROIInterpreter, "");
    }

    /* ----------------------------------------------------------------- */
    /*     Write the output file                                         */
    /* ----------------------------------------------------------------- */
    ofc = fileformat.saveFile (rtss_fn.c_str(), EXS_LittleEndianExplicit);
    if (ofc.bad()) {
        print_and_exit ("Error: cannot write DICOM RTSTRUCT (%s)\n", 
            ofc.text());
    }
}
