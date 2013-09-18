/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */

#include "itkArray.h"
#include "itkCommand.h"

#include "itk_demons.h"
#include "itk_diff_demons.h"
#include "itk_log_demons.h"
#include "itk_sym_log_demons.h"
#include "itk_fsf_demons.h"
#include "itk_demons_util.h"
#include "itk_demons_registration_filter.h"
#include "itk_resample.h"
#include "logfile.h"
#include "plm_image.h"
#include "plm_timer.h"
#include "print_and_exit.h"
#include "registration_data.h"
#include "stage_parms.h"
#include "xform.h"
#include "itkPDEDeformableRegistrationWithMaskFilter.h"

typedef itk::PDEDeformableRegistrationWithMaskFilter<FloatImageType,FloatImageType,DeformationFieldType>  PDEDeformableRegistrationFilterType;

typedef itk::ImageMaskSpatialObject< 3 >                                                                  MaskType;

PDEDeformableRegistrationFilterType::Pointer m_filter;

class Demons_Observer : public itk::Command
{
public:
    typedef Demons_Observer Self;
    typedef itk::Command Superclass;
    typedef itk::SmartPointer<Demons_Observer> Pointer;
    itkNewMacro (Demons_Observer);

public:
    Plm_timer* timer;
    int m_feval;

protected:
    Demons_Observer() {
        timer = new Plm_timer;
        timer->start ();
        m_feval = 0;
    };
    ~Demons_Observer () {
        delete timer;
    }
    unsigned int filtertype;

public:
    void SetFilterType(unsigned int f){filtertype=f;}
    void Execute(itk::Object *caller, const itk::EventObject & event)
    {
	Execute( (const itk::Object *)caller, event);
    }

    void Execute(const itk::Object * object, const itk::EventObject & event)
    {
        //using update version of PDEDeformableRegistrationFilter class
        const PDEDeformableRegistrationFilterType * filter=dynamic_cast< const PDEDeformableRegistrationFilterType* >(object);

        double val = filter->GetMetric();
        double duration = timer->report ();
        if (typeid(event) == typeid(itk::IterationEvent)) {
            logfile_printf ("MSE [%4d] %9.3f [%6.3f secs]\n",
            m_feval, val, duration);
            timer->start ();
            m_feval++;
        }
        else {
            std::cout << "Unknown event type." << std::endl;
            event.Print(std::cout);
        }
    }
};

//*Setting fixed and moving image masks if available
static void set_and_subsample_masks(Registration_data* regd,Stage_parms* stage )
{
    /* Subsample fixed & moving images */
    if(regd->fixed_roi)
    {
      MaskType::Pointer fixedSpatialObjectMask = MaskType::New();
      UCharImageType::Pointer fixed_mask
        = subsample_image (regd->fixed_roi->itk_uchar(),
        stage->fixed_subsample_rate[0],
        stage->fixed_subsample_rate[1],
        stage->fixed_subsample_rate[2],
        0);
      fixedSpatialObjectMask->SetImage(fixed_mask);
      fixedSpatialObjectMask->Update();
      m_filter->SetFixedImageMask (fixedSpatialObjectMask);
    }
    if(regd->moving_roi)
    {
      MaskType::Pointer movingSpatialObjectMask = MaskType::New();
      UCharImageType::Pointer moving_mask
        = subsample_image (regd->moving_roi->itk_uchar(),
        stage->fixed_subsample_rate[0],
        stage->fixed_subsample_rate[1],
        stage->fixed_subsample_rate[2],
        0);
      movingSpatialObjectMask->SetImage(moving_mask);
      movingSpatialObjectMask->Update();
      m_filter->SetMovingImageMask(movingSpatialObjectMask);
    }
}

//*Setting fixed and moving image masks if available
static void set_general_parameters(Stage_parms* stage )
{
    m_filter->SetNumberOfIterations (stage->max_its);
    m_filter->SetStandardDeviations (stage->demons_std);
    m_filter->SetUpdateFieldStandardDeviations(stage->demons_std_update_field);
    m_filter->SetSmoothUpdateField(stage->demons_smooth_update_field);
}

static void
do_demons_stage_internal (Registration_data* regd,
    Xform *xf_out, 
    Xform *xf_in,
    Stage_parms* stage)
{
    /* Subsample fixed & moving images */
    FloatImageType::Pointer fixed_ss
    = subsample_image (regd->fixed_image->itk_float(),
        stage->fixed_subsample_rate[0],
        stage->fixed_subsample_rate[1],
        stage->fixed_subsample_rate[2],
        stage->default_value);
    FloatImageType::Pointer moving_ss
    = subsample_image (regd->moving_image->itk_float(),
        stage->moving_subsample_rate[0],
        stage->moving_subsample_rate[1],
        stage->moving_subsample_rate[2],
        stage->default_value);

    m_filter->SetFixedImage (fixed_ss);
    m_filter->SetMovingImage (moving_ss);
    /* Get vector field of matching resolution */
    if (xf_in->m_type != STAGE_TRANSFORM_NONE) {
	xform_to_itk_vf (xf_out, xf_in, fixed_ss);

    //Set initial deformation field
    m_filter->SetInput (xf_out->get_itk_vf());
    }

    if (stage->max_its <= 0) {
	print_and_exit ("Error demons iterations must be greater than 0\n");
    }
    if (stage->demons_std <= 0.0001) {
	print_and_exit ("Error demons std must be greater than 0\n");
    }

    printf ("Ready to start registration.\n");
    m_filter->Update();
    printf ("Done with registration.  Writing output...\n");

    xf_out->set_itk_vf (m_filter->GetOutput());
}

void
do_demons_stage (Registration_data* regd,
		 Xform *xf_out, 
		 Xform *xf_in,
		 Stage_parms* stage)
{
    itk_demons_registration_filter* demons_filter = NULL;
    if(stage->optim_subtype == OPTIMIZATION_SUB_FSF)
    {
        demons_filter = new itk_fsf_demons_filter();
    }
    else if(stage->optim_subtype == OPTIMIZATION_SUB_DIFF_ITK)
    {
        demons_filter = new itk_diffeomorphic_demons_filter();
    }
    else if(stage->optim_subtype ==OPTIMIZATION_SUB_LOGDOM_ITK)
    {
        demons_filter = new itk_log_domain_demons_filter();
    }
    else if(stage->optim_subtype ==OPTIMIZATION_SUB_SYM_LOGDOM_ITK)
    {
        demons_filter = new itk_sym_log_domain_demons_filter();
    }

    m_filter=demons_filter->get_demons_filter_impl();

    //Set mask if available for implementation
    set_and_subsample_masks(regd,stage);

    //Set paramters that are used by all demons implementations
    set_general_parameters(stage);

    //Adding observer
    Demons_Observer::Pointer observer = Demons_Observer::New();
    m_filter->AddObserver (itk::IterationEvent(), observer);

    //Let filter set filter specific parameters
    demons_filter->update_specific_parameters(stage);

    do_demons_stage_internal (regd, xf_out, xf_in, stage);
    printf ("Deformation stats (out)\n");
    itk_demons_util::deformation_stats (xf_out->get_itk_vf());
    delete demons_filter;
}
