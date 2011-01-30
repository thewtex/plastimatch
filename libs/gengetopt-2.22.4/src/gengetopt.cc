/**
 * Copyright (C) 1999-2007  Free Software Foundation, Inc.
 *
 * This file is part of GNU gengetopt
 *
 * GNU gengetopt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU gengetopt is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with gengetopt; see the file COPYING. If not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

void gengetopt_free (void);

/* The following one is generated by gengetopt itself */
#include "cmdline.h"

extern int yyparse () ;

#include "gengetopt.h"
#include "errorcodes.h"

#include "argsdef.h"
#include "global_opts.h"

#include "my_sstream.h"
#include "my_string.h"

#include "gm.h"
#include "groups.h"
#include "gm_utils.h"

#include "skels/copyright.h"

#include "yyerror.h"

gengetopt_option_list gengetopt_options;
char * gengetopt_package = NULL;
char * gengetopt_version = NULL;
char * gengetopt_purpose = NULL;
char * gengetopt_description = NULL;
char * gengetopt_usage = NULL;

groups_collection_t gengetopt_groups;
modes_collection_t gengetopt_modes;

int gengetopt_count_line = 1;
char * gengetopt_input_filename = 0;
char *current_section = 0;
char *current_section_desc = 0;
char *current_text = 0;
char *current_args = 0;

/// whether, if not specified, an option is considered optional
bool default_to_optional = false;

int canonize_vars (void);
static void set_default_required_properties(void);

static void print_copyright();
static void print_reportbugs();

static void output_formatted_string(const string &);

static bool check_dependencies();

static int gengetopt_create_option (gengetopt_option *&opt, const char * long_opt, char short_opt,
                          const char * desc,
                          int type, int flagstat, int required,
                          const char *default_value,
                          const char * group_value,
                          const char * mode_value,
                          const char * type_str,
                          const AcceptedValues *acceptedvalues,
                          int multiple = 0,
                          int argoptional = 0);


#include <algorithm>
#include <iterator>

using namespace std;

/// the structure for command line arguments
struct gengetopt_args_info args_info ;

int
main (int argc, char **argv)
{
  char *cmdline_parser_name ; /* name of the generated function */
  char *cmdline_filename ; /* name of generated file */
  char *c_ext ; /* extenstion of c file */
  char *header_ext  ; /* extenstion of header file */
  string output_dir, header_output_dir, src_output_dir; /* output directory (default empty -> current dir)*/

  int i, has_help, has_version;
  FILE *input_file ;

  if (cmdline_parser (argc, argv, &args_info) != 0) {
      fprintf (stderr, "Run gengetopt --help to see the list of options.\n");
      exit(1) ;
  }

  if (args_info.help_given)
  {
    cmdline_parser_print_help ();
    print_reportbugs ();
    exit (0);
  }

  if (args_info.detailed_help_given)
    {
      cmdline_parser_print_detailed_help ();
      print_reportbugs ();
      exit (0);
    }

  if (args_info.version_given)
  {
    cmdline_parser_print_version ();
    print_copyright ();
    exit (0);
  }

  if ( args_info.input_arg )
    {
      gengetopt_input_filename = strdup (args_info.input_arg);
      input_file = freopen (args_info.input_arg, "r", stdin) ;
      if (!input_file)
        {
          fprintf (stderr, "Error opening input file: %s\n",
                   args_info.input_arg);
          exit (1);
        }
    } /* else standard input is used */

  if (yyparse () != 0) {
        gengetopt_free ();
        return 1;
  }

  // check whether some options were given in the input file
  bool no_options = (gengetopt_options.size() == 0);

  if (current_args) {
    // parse the arguments passed in the "args" part of the input file
    // by possibily overriding those given at command line
    if (cmdline_parser_string2 (current_args, &args_info, "gengetopt", 1, 0, 0) != 0) {
      fprintf (stderr, "Error in the args specification of the input_file.\n");
      exit(1) ;
    }
  }

  cmdline_parser_name = args_info.func_name_arg ;
  cmdline_filename = args_info.file_name_arg ;
  c_ext = args_info.c_extension_arg;
  header_ext = args_info.header_extension_arg;

  default_to_optional = args_info.default_optional_given;

  // now set the default "required" property for options
  set_default_required_properties();

  if (! check_dependencies()) {
    gengetopt_free();
    return 1;
  }

  // insert options for help and version if not already present
  gengetopt_option *opt;

  if (current_section)
    free(current_section);
  current_section = 0;

  has_version = gengetopt_has_option (VERSION_LONG_OPT, VERSION_SHORT_OPT);

  if (has_version != REQ_LONG_OPTION && !args_info.no_version_given) {
    gengetopt_create_option (opt, VERSION_LONG_OPT, has_version ? '-' : VERSION_SHORT_OPT,
                                 VERSION_OPT_DESCR, ARG_NO, 0, 0, 0, 0, 0, 0, 0);
    gengetopt_options.push_front(opt);
  }

  if (!args_info.no_help_given && has_hidden_options() && gengetopt_has_option(FULL_HELP_LONG_OPT, 0) == 0) {
      gengetopt_create_option (opt, FULL_HELP_LONG_OPT, '-',
                               FULL_HELP_OPT_DESCR, ARG_NO, 0, 0, 0, 0, 0, 0, 0);
      gengetopt_options.push_front(opt);
  }

  if (!args_info.no_help_given && has_options_with_details() && gengetopt_has_option(DETAILED_HELP_LONG_OPT, 0) == 0) {
      gengetopt_create_option (opt, DETAILED_HELP_LONG_OPT, '-',
              DETAILED_HELP_OPT_DESCR, ARG_NO, 0, 0, 0, 0, 0, 0, 0);
      gengetopt_options.push_front(opt);
  }

  has_help = gengetopt_has_option(HELP_LONG_OPT, HELP_SHORT_OPT);

  if (has_help != REQ_LONG_OPTION && !args_info.no_help_given) {
    gengetopt_create_option (opt, HELP_LONG_OPT, has_help ? '-' : HELP_SHORT_OPT,
                                 HELP_OPT_DESCR, ARG_NO, 0, 0, 0, 0, 0, 0, 0);
    gengetopt_options.push_front(opt);
  }

  // check whether there's pending text after all options and
  // in case, set it to the last option
  if (current_text && gengetopt_options.size()) {
    gengetopt_options.back()->text_after = current_text;
    gengetopt_set_text(0);
  }

  if (canonize_vars ()) {
        gengetopt_free ();
        return 1;
  }

  if (args_info.set_package_given) {
    if (gengetopt_package)
      free (gengetopt_package);
    gengetopt_package = args_info.set_package_arg;
  }

  if (args_info.set_version_given) {
    if (gengetopt_version)
      free (gengetopt_version);
    gengetopt_version = args_info.set_version_arg;
  }

  ostringstream command_line;
  for ( i = 0; i < argc ; ++i )
      command_line << argv[i] << " ";

  // add also possible args specification in the input file
  if (current_args)
    command_line << current_args;

  if (args_info.output_dir_given)
      output_dir = args_info.output_dir_arg;
  if (args_info.header_output_dir_given)
      header_output_dir = args_info.header_output_dir_arg;
  if (args_info.src_output_dir_given)
      src_output_dir = args_info.src_output_dir_arg;

  CmdlineParserCreator cmdline_parser_creator
    (cmdline_parser_name,
     args_info.arg_struct_name_arg,
     (args_info.unamed_opts_given ? args_info.unamed_opts_arg : 0),
     cmdline_filename,
     header_ext,
     c_ext,
     args_info.long_help_given,
     args_info.no_handle_help_given,
     args_info.no_help_given,
     args_info.no_handle_version_given,
     args_info.no_version_given,
     args_info.no_handle_error_given,
     args_info.conf_parser_given,
     args_info.string_parser_given,
     args_info.gen_version_flag,
     args_info.include_getopt_given,
     no_options,
     command_line.str (),
     output_dir,
     header_output_dir,
     src_output_dir,
     (args_info.show_required_given ? args_info.show_required_arg : ""));

  if (! gengetopt_package && (args_info.show_version_given || args_info.show_help_given))
    {
      cerr << "package not defined; please specify it with --set-package" << endl;
      return 1;
    }
  else if (! gengetopt_version && (args_info.show_version_given || args_info.show_help_given))
    {
      cerr << "version not defined; please specify it with --set-version" << endl;
      return 1;
    }
  else if (args_info.show_version_given)
    {
      cout << gengetopt_package << " " << gengetopt_version << endl;
    }
  else if (args_info.show_help_given ||
          args_info.show_full_help_given || args_info.show_detailed_help_given)
    {
      cout << gengetopt_package << " " << gengetopt_version << "\n" << endl;

      if (gengetopt_purpose) {
        output_formatted_string(cmdline_parser_creator.generate_purpose());
        cout << endl;
      }

      output_formatted_string("Usage: " +
        cmdline_parser_creator.generate_usage_string(false) + "\n");

      if (gengetopt_description) {
        output_formatted_string(cmdline_parser_creator.generate_description());
        cout << endl;
      }

      // if --show-full-help is specified we have to generate also hidden options
      OptionHelpList *option_list =
          cmdline_parser_creator.generate_help_option_list(args_info.show_full_help_given, args_info.show_detailed_help_given);

      std::for_each(option_list->begin(), option_list->end(),
              output_formatted_string);

      delete option_list;
    }
  else if (cmdline_parser_creator.generate ())
    {
        gengetopt_free ();
        return 1;
    }

  gengetopt_free ();

  return 0;
}

void
output_formatted_string(const string &s)
{
  for (string::const_iterator it = s.begin(); it != s.end(); ++it)
    {
      if (*it == '\\' && ((it+1) != s.end()) && *(it+1) == 'n') {
        cout << "\n";
        ++it;
      } else if (*it == '\\' && ((it+1) != s.end()) && *(it+1) == '"') {
        cout << "\"";
        ++it;
      } else
        cout << *it;
    }

  cout << endl;
}

/* ************* */

int
gengetopt_define_package (char * s)
{
        gengetopt_package = strdup (s);
        if (gengetopt_package == NULL)
                return 1;
        return 0;
}

int
gengetopt_define_version (char * s)
{
        gengetopt_version = strdup (s);
        if (gengetopt_version == NULL)
                return 1;
        return 0;
}

int
gengetopt_define_purpose (char * s)
{
  gengetopt_purpose = strdup (s);
  if (gengetopt_purpose == NULL)
    return 1;
  return 0;
}

int
gengetopt_define_description (char * s)
{
  gengetopt_description = strdup (s);
  if (gengetopt_description == NULL)
    return 1;
  return 0;
}

int gengetopt_define_usage (char * s)
{
  gengetopt_usage = strdup (s);
  if (gengetopt_usage == NULL)
    return 1;
  return 0;
}

int
gengetopt_add_group (const char *s, const char *desc, int required)
{
  string group_desc;
  if (desc)
    group_desc = desc;
  if ( !gengetopt_groups.insert
       (make_pair(string(s),Group (group_desc, required != 0))).second )
    return 1;
  else
    return 0;
}

int
gengetopt_add_mode (const char *s, const char *desc)
{
  string mode_desc;
  if (desc)
    mode_desc = desc;
  if ( !gengetopt_modes.insert
       (make_pair(string(s),Mode (mode_desc))).second )
    return 1;
  else
    return 0;
}

void
gengetopt_set_section (const char * s, const char *desc)
{
  if (current_section)
    free (current_section);
  if (current_section_desc)
    free (current_section_desc);
  current_section = strdup (s);
  if (desc)
    current_section_desc = strdup (desc);
  else
    current_section_desc = 0;
}

void
gengetopt_set_text (const char * desc)
{
  /*
  no need to free it, since it will be then owned by the
  option only

  if (current_text)
    free (current_text);
  */

  // the current text is reset
  if (!desc) {
    current_text = 0;
    return;
  }

  if (current_text) {
    // a previous text was collected, so we append the new text
    // to the current one.
    string buffer = current_text;
    buffer += desc;
    current_text = strdup(buffer.c_str());
    return;
  }

  // otherwise simply copy the passed text
  current_text = strdup (desc);
}

void gengetopt_set_args(const char *a)
{
  if (current_args)
    free(current_args);

  if (a)
    current_args = strdup(a);
  else
    current_args = 0;
}

int
gengetopt_has_option (const char * long_opt, char short_opt)
{
  gengetopt_option * n;

  for (gengetopt_option_list::const_iterator it = gengetopt_options.begin();
       it != gengetopt_options.end(); ++it)
    {
      n = *it;
      if (!strcmp (n->long_opt, long_opt)) return REQ_LONG_OPTION;
      if (short_opt && n->short_opt == short_opt)
        return REQ_SHORT_OPTION;
    }

  return 0;
}

bool check_numeric_validity(const char *val, int opt_type)
{
    char *end_of_string, *expected_eos;

    expected_eos = (char *) (val + strlen(val));

    switch ( opt_type )
      {
      case ARG_INT :
      case ARG_SHORT :
      case ARG_LONG :
      case ARG_LONGLONG :
        (void) strtol(val, &end_of_string, 0);
        break;

      case ARG_FLOAT:
      case ARG_DOUBLE:
      case ARG_LONGDOUBLE:
        (void) strtod(val, &end_of_string);
        break;

      default :
        // This will allow us to factorise as a single line the
        // test for correctness of the default value
        end_of_string = expected_eos;
        break;
      }
    return ( end_of_string == expected_eos );
}

bool
check_values(const AcceptedValues *acceptedvalues, int opt_type)
{
    for (AcceptedValues::const_iterator it = acceptedvalues->begin();
        it != acceptedvalues->end(); ++it) {
        if (!check_numeric_validity((*it).c_str(), opt_type))
            return false;
    }

    return true;
}

int
gengetopt_create_option (gengetopt_option *&n, const char * long_opt, char short_opt,
                      const char * desc,
                      int type, int flagstat, int required,
                      const char * default_value,
                      const char * group_value,
                      const char * mode_value,
                      const char * type_str,
                      const AcceptedValues *acceptedvalues,
                      int multiple,
                      int argoptional)
{
  if ((long_opt == NULL) ||
      (long_opt[0] == 0) ||
      (desc == NULL))
    return FOUND_BUG;

  n = new gengetopt_option;
  if (n == NULL)
    return NOT_ENOUGH_MEMORY;

  // here we will set required anyway
  n->required_set = true;

  n->long_opt = strdup (long_opt);
  if (n->long_opt == NULL)
    {
      free (n);
      return NOT_ENOUGH_MEMORY;
    }

  n->desc = strdup (desc);
  if (n->desc == NULL)
    {
      free (n->long_opt);
      free (n);
      return NOT_ENOUGH_MEMORY;
    }

  n->short_opt = ((short_opt == '-') ? 0 : short_opt);
  n->type = type;
  n->flagstat = flagstat;
  n->required = required;
  n->multiple = (multiple != 0);
  n->arg_is_optional = (argoptional != 0);

  if (type_str != 0)
    n->type_str = strdup(type_str);
  else
    n->type_str = NULL;

  n->section = 0;
  n->section_desc = 0;
  if (current_section)
    n->section = strdup (current_section);
  if (current_section_desc)
    n->section_desc = strdup (current_section_desc);

  if (group_value != 0)
    {
      n->group_value = strdup(group_value);
      n->required = 0;
      groups_collection_t::const_iterator it =
        gengetopt_groups.find(string(n->group_value));
      if (it == gengetopt_groups.end())
        return GROUP_UNDEFINED;
      n->group_desc = strdup (it->second.desc.c_str ());
    }
  else
    {
      n->group_value = 0;
    }

  if (mode_value != 0) {
      n->mode_value = strdup(mode_value);
      modes_collection_t::const_iterator it =
          gengetopt_modes.find(string(n->mode_value));
      if (it == gengetopt_modes.end())
          return MODE_UNDEFINED;
      n->mode_desc = strdup (it->second.desc.c_str ());
  } else {
      n->mode_value = 0;
  }

  if (n->group_value && n->mode_value)
      return FOUND_BUG;


  n->acceptedvalues = acceptedvalues;

  // if (acceptedvalues && type != ARG_NO)
  // return NOT_REQUESTED_TYPE;

  if (acceptedvalues && ! (n->type))
    n->type = ARG_STRING;

  n->default_string = 0;
  n->default_given = (default_value != 0);
  if (n->default_given)
    {
      n->default_string = strdup (default_value);
      if ( ! check_numeric_validity(default_value, n->type) )
        {
          free (n);
          return INVALID_DEFAULT_VALUE;
        }

      if (acceptedvalues)
        {
          if (! acceptedvalues->contains(n->default_string))
            return INVALID_DEFAULT_VALUE;
        }
    }

  if (acceptedvalues) {
      if (!check_values(acceptedvalues, n->type))
          return INVALID_NUMERIC_VALUE;
  }

  n->var_arg = NULL;

  return 0;
}

int
gengetopt_check_option (gengetopt_option *n, bool groupoption, bool modeoption)
{
  if ((n->long_opt == NULL) ||
      (n->long_opt[0] == 0) ||
      (n->desc == NULL))
    return FOUND_BUG;

  if (strcmp(n->long_opt, HELP_LONG_OPT) == 0 && !args_info.no_help_given)
      return HELP_REDEFINED;

  if (strcmp(n->long_opt, VERSION_LONG_OPT) == 0 && !args_info.no_version_given)
      return VERSION_REDEFINED;

  n->section = 0;
  n->section_desc = 0;
  if (current_section)
    n->section = strdup (current_section);
  if (current_section_desc)
    n->section_desc = strdup (current_section_desc);

  n->text_before = current_text;
  // reset the description
  gengetopt_set_text(0);

  if (n->group_value != 0)
  {
      if (! groupoption)
          return NOT_GROUP_OPTION;

      n->required = 0;
      n->required_set = true;

      groups_collection_t::const_iterator it =
          gengetopt_groups.find(string(n->group_value));
      if (it == gengetopt_groups.end())
          return GROUP_UNDEFINED;
      n->group_desc = strdup (it->second.desc.c_str ());
  }
  else
  {
      if (groupoption)
          return SPECIFY_GROUP;
  }

  if (n->mode_value != 0)
  {
      if (! modeoption)
          return NOT_MODE_OPTION;

      modes_collection_t::const_iterator it =
          gengetopt_modes.find(string(n->mode_value));
      if (it == gengetopt_modes.end())
          return MODE_UNDEFINED;
      n->mode_desc = strdup (it->second.desc.c_str ());
  }
  else
  {
      if (modeoption)
          return SPECIFY_MODE;
  }

  if (n->group_value && n->mode_value)
      return FOUND_BUG;

  // now we have to check for flag options
  if (n->type == ARG_FLAG)
  {
    if (n->flagstat < 0)
      return SPECIFY_FLAG_STAT;

    if (n->default_string || n->multiple || n->arg_is_optional
        || n->type_str || n->acceptedvalues || n->required_set)
      return NOT_VALID_SPECIFICATION;

    n->required = 0;
    n->required_set = true;
  }
  else
  {
    if (n->flagstat >= 0)
      return NOT_VALID_SPECIFICATION;
  }

  // enum type can only be specified with options with values
  if (n->type == ARG_ENUM && !(n->acceptedvalues)) {
      return INVALID_ENUM_TYPE_USE;
  }

  // if (acceptedvalues && type != ARG_NO)
  // return NOT_REQUESTED_TYPE;

  if (n->acceptedvalues && ! (n->type))
    n->type = ARG_STRING;

  n->default_given = (n->default_string != 0);
  if (n->default_given)
    {
      if ( !check_numeric_validity(n->default_string, n->type) )
        {
          return INVALID_DEFAULT_VALUE;
        }

      if (n->acceptedvalues)
        {
          if (! n->acceptedvalues->contains(n->default_string))
            return INVALID_DEFAULT_VALUE;
        }
    }

  if (n->acceptedvalues) {
      if (!check_values(n->acceptedvalues, n->type))
          return INVALID_NUMERIC_VALUE;
  }

  n->var_arg = NULL;

  return 0;
}

int
gengetopt_add_option (const char * long_opt, char short_opt,
                      const char * desc,
                      int type, int flagstat, int required,
                      const char * default_value,
                      const char * group_value,
                      const char * mode_value,
                      const char * type_str,
                      const AcceptedValues *acceptedvalues,
                      int multiple,
                      int argoptional)
{
  gengetopt_option * n;

  /* search for collisions */
  int res = gengetopt_has_option(long_opt, short_opt);
  if (res != 0)
    return res;

  res = gengetopt_create_option(n, long_opt, short_opt,
    desc, type, flagstat, required, default_value, group_value, mode_value,
    type_str, acceptedvalues, multiple, argoptional);
  if (res != 0)
    return res;

  gengetopt_options.push_back(n);

  return 0;
}

int
gengetopt_add_option (gengetopt_option * n)
{
  /* search for collisions */
  int res = gengetopt_has_option(n);
  if (res != 0)
    return res;

  gengetopt_options.push_back(n);

  return 0;
}

int
gengetopt_has_option (gengetopt_option * opt)
{
  gengetopt_option * n;

  for (gengetopt_option_list::const_iterator it = gengetopt_options.begin();
       it != gengetopt_options.end(); ++it)
    {
      n = *it;
      if (!strcmp (n->long_opt, opt->long_opt))
        return REQ_LONG_OPTION;
      if (opt->short_opt && n->short_opt == opt->short_opt)
        return REQ_SHORT_OPTION;
    }

  return 0;
}

bool
check_dependencies()
{
  gengetopt_option * n;
  bool result = true;

  for (gengetopt_option_list::const_iterator it = gengetopt_options.begin();
       it != gengetopt_options.end(); ++it)
  {
    n = *it;
    if (n->dependon) {
        if (strcmp(n->dependon, n->long_opt) == 0) {
            yyerror(n, "option depends on itself");
            result = false;
            continue;
        }

        bool found = false;
        for (gengetopt_option_list::const_iterator it2 = gengetopt_options.begin();
            it2 != gengetopt_options.end(); ++it2)
        {
            if (strcmp(n->dependon, (*it2)->long_opt) == 0) {
                found = true;
                break;
            }
        }

        if (! found) {
            yyerror (n, "option depends on undefined option");
            result = false;
        }
    }
  }

  return result;
}

void
gengetopt_free (void)
{
  gengetopt_option *p;

  if (gengetopt_package != NULL)
    free (gengetopt_package);

  for (gengetopt_option_list::iterator it = gengetopt_options.begin(); it != gengetopt_options.end(); ++it)
  {
        p = *it;
        if (p->long_opt != NULL) free (p->long_opt);
        if (p->desc != NULL) free (p->desc);
        if (p->var_arg != NULL) free (p->var_arg);
        if (p->acceptedvalues) delete p->acceptedvalues;
        delete p;
  }
}

static void
canonize_var (gengetopt_option *p)
{
  char *pvar;

  p->var_arg = strdup (p->long_opt);
  if (p->var_arg == NULL) {
      printf ("gengetopt: not enough memory to canonize vars\n");
      abort();
  }

  for (pvar = p->var_arg; *pvar; pvar++)
    if (*pvar == '.' || *pvar == '-') *pvar = '_';
}

int
canonize_vars (void)
{
  for_each(gengetopt_options.begin(), gengetopt_options.end(), canonize_var);

  return 0;
}

static void
set_default_required_prop (gengetopt_option *p)
{
    if (!p->required_set)
        p->required = (default_to_optional ? 0 : 1);
}

static void set_default_required_properties(void)
{
  for_each(gengetopt_options.begin(), gengetopt_options.end(), set_default_required_prop);
}

void
print_copyright()
{
  copyright_gen_class copyright_g;

  copyright_g.set_year ("1999-2009");
  copyright_g.generate_copyright (cout);
}

void
print_reportbugs()
{
  cout << endl;
  cout << "Maintained by Lorenzo Bettini <http://www.lorenzobettini.it>" << endl;
  cout << "Report bugs to <bug-gengetopt at gnu.org>" << endl;
}
