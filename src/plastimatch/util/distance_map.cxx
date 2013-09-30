/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "itkImage.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"

#include "distance_map.h"
#include "itk_image_type.h"
#include "plm_image.h"

class Distance_map_private {
public:
    Distance_map_private () {
        inside_is_positive = true;
        use_squared_distance = true;
        algorithm = Distance_map::ITK_SIGNED_MAURER;
    }
public:
    Distance_map::Algorithm algorithm;
    bool inside_is_positive;
    bool use_squared_distance;
    UCharImageType::Pointer input;
    FloatImageType::Pointer output;
public:
    void run_itk_signed_maurer ();
    void run ();
};

void
Distance_map_private::run_itk_signed_maurer ()
{
    typedef itk::SignedMaurerDistanceMapImageFilter< 
        UCharImageType, FloatImageType >  FilterType;
    FilterType::Pointer filter = FilterType::New ();

    if (this->use_squared_distance) {
        filter->SetSquaredDistance (true);
    } else {
        filter->SetSquaredDistance (false);
    }

    /* Always compute map in millimeters, never voxels */
    filter->SetUseImageSpacing (true);

    if (this->inside_is_positive) {
        filter->SetInsideIsPositive (true);
    } else {
        filter->SetInsideIsPositive (false);
    }

    /* ??? */
    /* filter->SetNumberOfThreads (2); */

    /* Run the filter */
    filter->SetInput (this->input);
    filter->Update();
    this->output = filter->GetOutput ();
}

void
Distance_map_private::run ()
{
    switch (this->algorithm) {
    case Distance_map::ITK_SIGNED_MAURER:
    default:
        this->run_itk_signed_maurer ();
        break;
    }
}

Distance_map::Distance_map () {
    d_ptr = new Distance_map_private;
}

Distance_map::~Distance_map () {
    delete d_ptr;
}

void
Distance_map::set_input_image (const std::string& image_fn)
{
    Plm_image pli (image_fn);
    d_ptr->input = pli.itk_uchar();
}

void
Distance_map::set_input_image (const char* image_fn)
{
    Plm_image pli (image_fn);
    d_ptr->input = pli.itk_uchar();
}

void
Distance_map::set_input_image (UCharImageType::Pointer image)
{
    d_ptr->input = image;
}

void 
Distance_map::set_use_squared_distance (bool use_squared_distance)
{
    d_ptr->use_squared_distance = use_squared_distance;
}

void 
Distance_map::set_inside_is_positive (bool inside_is_positive)
{
    d_ptr->inside_is_positive = inside_is_positive;
}

void 
Distance_map::set_algorithm (Distance_map::Algorithm algorithm)
{
    d_ptr->algorithm = algorithm;
}

void
Distance_map::run ()
{
    d_ptr->run ();
}

FloatImageType::Pointer
Distance_map::get_output_image ()
{
    return d_ptr->output;
}
