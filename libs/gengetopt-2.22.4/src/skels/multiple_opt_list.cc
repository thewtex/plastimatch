/*
 * File automatically generated by
 * gengen 1.2 by Lorenzo Bettini 
 * http://www.gnu.org/software/gengen
 */

#include "multiple_opt_list.h"

void
multiple_opt_list_gen_class::generate_multiple_opt_list(ostream &stream, unsigned int indent)
{
  string indent_str (indent, ' ');
  indent = 0;

  stream << "struct generic_list * ";
  generate_string (arg_name, stream, indent + indent_str.length ());
  stream << "_list = NULL;";
}
