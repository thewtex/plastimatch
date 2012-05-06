/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _xio_dir_h
#define _xio_dir_h

#include "plmbase_config.h"
#include <vector>
#include "pstring.h"

class Xio_patient;
struct Xio_studyset_dir;
struct Xio_plan_dir;

/* This class represents the input directory, which could be a 
   patient directory, plan directory, or even a directory which 
   contains multiple patients */
class API Xio_dir {
public:
    Pstring path;
    std::vector<Xio_patient*> patient_dir;
public:
    Xio_dir (const char *input_dir);
    ~Xio_dir ();
    void analyze ();
    void analyze_recursive (std::string dir);
    Xio_patient* add_patient_dir (std::string dir);
    int num_patients () const;
public:
    static int is_xio_patient_dir (std::string dir);
    static int is_xio_studyset_dir (std::string dir);
    static int is_xio_plan_dir (std::string dir);
};

API int xio_dir_num_patients (Xio_dir* xd);

API Xio_studyset_dir* xio_plan_dir_get_studyset_dir (Xio_plan_dir* xtpd);

#endif
