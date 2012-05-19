/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"
#include <list>
#include "itkAddImageFilter.h"
#include "itkNaryAddImageFilter.h"
#include "itkDivideByConstantImageFilter.h"
#include "itkMultiplyByConstantImageFilter.h"

#include "plmbase.h"
#include "plmsys.h"
#include "plmutil.h"

#include "plm_clp.h"
#include "pstring.h"

class Add_parms {
public:
    Pstring output_fn;
    bool average;
    std::vector<float> weight_vector;
    std::list<Pstring> input_fns;
public:
    Add_parms () {
        average = false;
    }
};

static void
scale_image (Plm_image *img, float weight)
{
    /* GCS 2012-01-27 -- If you re-use the multiply filter 
       with new inputs, it gives the wrong answer. 
       Or maybe it has some default behavior you need to 
       override?? */
    img->set_itk (itk_scale (img->itk_float(), weight));
}

void
add_main (Add_parms *parms)
{
    typedef itk::AddImageFilter< 
        FloatImageType, FloatImageType, FloatImageType > AddFilterType;

    AddFilterType::Pointer addition = AddFilterType::New();

    /* Make sure we got the same number of input files and weights */
    if (parms->weight_vector.size() > 0 
        && parms->weight_vector.size() != parms->input_fns.size())
    {
        print_and_exit (
            "Error, you specified %d input files and %d weights\n",
            parms->input_fns.size(),
            parms->weight_vector.size());
    }

    /* Load the first input image */
    std::list<Pstring>::iterator it = parms->input_fns.begin();
    Plm_image *sum = plm_image_load ((*it), PLM_IMG_TYPE_ITK_FLOAT);
    ++it;

    /* Weigh the first input image */
    int widx = 0;
    if (parms->weight_vector.size() > 0) {
        scale_image (sum, parms->weight_vector[widx]);
        ++widx;
    }

    /* Loop through remaining images */
    while (it != parms->input_fns.end()) {
        /* Load the images */
        Plm_image *tmp = plm_image_load ((*it), PLM_IMG_TYPE_ITK_FLOAT);

        /* Weigh it */
        if (parms->weight_vector.size() > 0) {
            scale_image (tmp, parms->weight_vector[widx]);
            ++widx;
        }

        /* Add it to running sum */
	addition->SetInput1 (sum->itk_float());
	addition->SetInput2 (tmp->itk_float());
	addition->Update();
	sum->m_itk_float = addition->GetOutput ();
        ++it;
    }

    /* Take average */
    if (parms->average) {
        float avg = 1.0f / parms->input_fns.size();
        scale_image (sum, avg);
    }

    /* Save the sum image */
    sum->convert_to_original_type ();
    sum->save_image (parms->output_fn);
}

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    printf ("Usage: plastimatch %s [options] input_file [input_file ...]\n", 
        argv[1]);
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Add_parms* parms, 
    dlib::Plm_clp* parser, 
    int argc, 
    char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Output files */
    parser->add_long_option ("", "output", "output image", 1, "");

    /* Weight vector */
    parser->add_long_option ("", "weight", "", 1, "");

    /* Average option */
    parser->add_long_option ("", "average", 
        "produce an output file which is the average of the input files "
        "(if no weights are specified), or multiply the weights by 1/n",
        0);

    /* Parse options */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    /* Check that an output file was given */
    if (!parser->option ("output")) {
	throw (dlib::error ("Error.  Please specify an output file "
		"using the --output option"));
    }

    /* Check that no extraneous options were given */
    if (parser->number_of_arguments() == 0) {
	throw (dlib::error ("Error.  You must specify at least one "
                "file to add."));
    }

    /* Copy input filenames to parms struct */
    for (unsigned long i = 0; i < parser->number_of_arguments(); i++) {
        parms->input_fns.push_back (Pstring((*parser)[i].c_str()));
    }

    /* Output files */
    parms->output_fn = parser->get_string("output").c_str();
    if (parser->option ("weight")) {
        parser->assign_float_vec (&parms->weight_vector, "weight");
    }
}

void
do_command_add (int argc, char *argv[])
{
    Add_parms parms;

    /* Check if we're doing add or average */
    if (!strcmp (argv[1], "add")) {
        parms.average = false;
    } else {
        parms.average = true;
    }

    /* Parse command line parameters */
    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    add_main (&parms);
}
