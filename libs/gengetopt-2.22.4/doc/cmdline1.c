/*
  File autogenerated by gengetopt version 2.22.4
  generated with the following command:
  ../src/gengetopt -i ../../doc/sample1.ggo -Fcmdline1 --long-help -u --show-required 

  The developers of gengetopt consider the fixed text that goes in all
  gengetopt output files to be in the public domain:
  we make no copyright claims on it.
*/

/* If we use autoconf.  */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FIX_UNUSED
#define FIX_UNUSED(X) (void) (X) /* avoid warnings for unused params */
#endif

#include <getopt.h>

#include "cmdline1.h"

const char *gengetopt_args_info_purpose = "";

const char *gengetopt_args_info_usage = "Usage: sample1 -iINT|--int-opt=INT [-h|--help] [--detailed-help] [--full-help] \n         [-V|--version] [-sfilename|--str-opt=filename] [-mINT|--my-opt=INT] \n         [--flag-opt] [-F|--funct-opt] [--long-opt=LONG] [--def-opt=STRING] \n         [--enum-opt=STRING] [-DINT|--dependant=INT] [FILES]...";

const char *gengetopt_args_info_description = "";

const char *gengetopt_args_info_detailed_help[] = {
  "  -h, --help              Print help and exit",
  "      --detailed-help     Print help, including all details and hidden options, \n                            and exit",
  "      --full-help         Print help, including hidden options, and exit",
  "  -V, --version           Print version and exit",
  "  -s, --str-opt=filename  A string option, for a filename",
  "\nA brief text description before the other options.\n",
  "  -m, --my-opt=INT        Another integer option, this time the description of \n                            the option should be \"quite\" long to require \n                            wrapping... possibly more than one wrapping :-) \n                            especially if I\n                            require a line break",
  "  -i, --int-opt=INT       A int option (mandatory)",
  "\nmore involved options:",
  "  the following options\n  are more complex",
  "",
  "      --flag-opt          A flag option  (default=off)",
  "  -F, --funct-opt         A function option",
  "  \n  A function option is basically an option with no argument.  It can be used, \n  e.g., to specify a specific behavior for a program.\n\n  Well, this further explanation is quite useless, but it's only to show an \n  example of an option with details, which will be printed only when \n  --detailed-help is given at the command line.",
  "\nlast option section:",
  "      --long-opt=LONG     A long option",
  "      --def-opt=STRING    A string option with default  (default=`Hello')",
  "      --enum-opt=STRING   A string option with list of values  (possible \n                            values=\"foo\", \"bar\", \"hello\", \"bye\" \n                            default=`hello')",
  "  -S, --secret=INT        hidden option will not appear in --help",
  "  -D, --dependant=INT     option that depends on str-opt",
  "\nAn ending text.",
    0
};
static void
init_full_help_array(void)
{
  gengetopt_args_info_full_help[0] = gengetopt_args_info_detailed_help[0];
  gengetopt_args_info_full_help[1] = gengetopt_args_info_detailed_help[1];
  gengetopt_args_info_full_help[2] = gengetopt_args_info_detailed_help[2];
  gengetopt_args_info_full_help[3] = gengetopt_args_info_detailed_help[3];
  gengetopt_args_info_full_help[4] = gengetopt_args_info_detailed_help[4];
  gengetopt_args_info_full_help[5] = gengetopt_args_info_detailed_help[5];
  gengetopt_args_info_full_help[6] = gengetopt_args_info_detailed_help[6];
  gengetopt_args_info_full_help[7] = gengetopt_args_info_detailed_help[7];
  gengetopt_args_info_full_help[8] = gengetopt_args_info_detailed_help[8];
  gengetopt_args_info_full_help[9] = gengetopt_args_info_detailed_help[9];
  gengetopt_args_info_full_help[10] = gengetopt_args_info_detailed_help[10];
  gengetopt_args_info_full_help[11] = gengetopt_args_info_detailed_help[11];
  gengetopt_args_info_full_help[12] = gengetopt_args_info_detailed_help[12];
  gengetopt_args_info_full_help[13] = gengetopt_args_info_detailed_help[14];
  gengetopt_args_info_full_help[14] = gengetopt_args_info_detailed_help[15];
  gengetopt_args_info_full_help[15] = gengetopt_args_info_detailed_help[16];
  gengetopt_args_info_full_help[16] = gengetopt_args_info_detailed_help[17];
  gengetopt_args_info_full_help[17] = gengetopt_args_info_detailed_help[18];
  gengetopt_args_info_full_help[18] = gengetopt_args_info_detailed_help[19];
  gengetopt_args_info_full_help[19] = gengetopt_args_info_detailed_help[20];
  gengetopt_args_info_full_help[20] = 0; 
  
}

const char *gengetopt_args_info_full_help[21];

static void
init_help_array(void)
{
  gengetopt_args_info_help[0] = gengetopt_args_info_detailed_help[0];
  gengetopt_args_info_help[1] = gengetopt_args_info_detailed_help[1];
  gengetopt_args_info_help[2] = gengetopt_args_info_detailed_help[2];
  gengetopt_args_info_help[3] = gengetopt_args_info_detailed_help[3];
  gengetopt_args_info_help[4] = gengetopt_args_info_detailed_help[4];
  gengetopt_args_info_help[5] = gengetopt_args_info_detailed_help[5];
  gengetopt_args_info_help[6] = gengetopt_args_info_detailed_help[6];
  gengetopt_args_info_help[7] = gengetopt_args_info_detailed_help[7];
  gengetopt_args_info_help[8] = gengetopt_args_info_detailed_help[8];
  gengetopt_args_info_help[9] = gengetopt_args_info_detailed_help[9];
  gengetopt_args_info_help[10] = gengetopt_args_info_detailed_help[10];
  gengetopt_args_info_help[11] = gengetopt_args_info_detailed_help[11];
  gengetopt_args_info_help[12] = gengetopt_args_info_detailed_help[12];
  gengetopt_args_info_help[13] = gengetopt_args_info_detailed_help[14];
  gengetopt_args_info_help[14] = gengetopt_args_info_detailed_help[15];
  gengetopt_args_info_help[15] = gengetopt_args_info_detailed_help[16];
  gengetopt_args_info_help[16] = gengetopt_args_info_detailed_help[17];
  gengetopt_args_info_help[17] = gengetopt_args_info_detailed_help[19];
  gengetopt_args_info_help[18] = gengetopt_args_info_detailed_help[20];
  gengetopt_args_info_help[19] = 0; 
  
}

const char *gengetopt_args_info_help[20];

typedef enum {ARG_NO
  , ARG_FLAG
  , ARG_STRING
  , ARG_INT
  , ARG_LONG
} cmdline_parser_arg_type;

static
void clear_given (struct gengetopt_args_info *args_info);
static
void clear_args (struct gengetopt_args_info *args_info);

static int
cmdline_parser_internal (int argc, char **argv, struct gengetopt_args_info *args_info,
                        struct cmdline_parser_params *params, const char *additional_error);

static int
cmdline_parser_required2 (struct gengetopt_args_info *args_info, const char *prog_name, const char *additional_error);

const char *cmdline_parser_enum_opt_values[] = {"foo", "bar", "hello", "bye", 0}; /*< Possible values for enum-opt. */

static char *
gengetopt_strdup (const char *s);

static
void clear_given (struct gengetopt_args_info *args_info)
{
  args_info->help_given = 0 ;
  args_info->detailed_help_given = 0 ;
  args_info->full_help_given = 0 ;
  args_info->version_given = 0 ;
  args_info->str_opt_given = 0 ;
  args_info->my_opt_given = 0 ;
  args_info->int_opt_given = 0 ;
  args_info->flag_opt_given = 0 ;
  args_info->funct_opt_given = 0 ;
  args_info->long_opt_given = 0 ;
  args_info->def_opt_given = 0 ;
  args_info->enum_opt_given = 0 ;
  args_info->secret_given = 0 ;
  args_info->dependant_given = 0 ;
}

static
void clear_args (struct gengetopt_args_info *args_info)
{
  FIX_UNUSED (args_info);
  args_info->str_opt_arg = NULL;
  args_info->str_opt_orig = NULL;
  args_info->my_opt_orig = NULL;
  args_info->int_opt_orig = NULL;
  args_info->flag_opt_flag = 0;
  args_info->long_opt_orig = NULL;
  args_info->def_opt_arg = gengetopt_strdup ("Hello");
  args_info->def_opt_orig = NULL;
  args_info->enum_opt_arg = gengetopt_strdup ("hello");
  args_info->enum_opt_orig = NULL;
  args_info->secret_orig = NULL;
  args_info->dependant_orig = NULL;
  
}

static
void init_args_info(struct gengetopt_args_info *args_info)
{
  init_full_help_array(); 
  init_help_array(); 
  args_info->help_help = gengetopt_args_info_detailed_help[0] ;
  args_info->detailed_help_help = gengetopt_args_info_detailed_help[1] ;
  args_info->full_help_help = gengetopt_args_info_detailed_help[2] ;
  args_info->version_help = gengetopt_args_info_detailed_help[3] ;
  args_info->str_opt_help = gengetopt_args_info_detailed_help[4] ;
  args_info->my_opt_help = gengetopt_args_info_detailed_help[6] ;
  args_info->int_opt_help = gengetopt_args_info_detailed_help[7] ;
  args_info->flag_opt_help = gengetopt_args_info_detailed_help[11] ;
  args_info->funct_opt_help = gengetopt_args_info_detailed_help[12] ;
  args_info->long_opt_help = gengetopt_args_info_detailed_help[15] ;
  args_info->def_opt_help = gengetopt_args_info_detailed_help[16] ;
  args_info->enum_opt_help = gengetopt_args_info_detailed_help[17] ;
  args_info->secret_help = gengetopt_args_info_detailed_help[18] ;
  args_info->dependant_help = gengetopt_args_info_detailed_help[19] ;
  
}

void
cmdline_parser_print_version (void)
{
  printf ("%s %s\n",
     (strlen(CMDLINE_PARSER_PACKAGE_NAME) ? CMDLINE_PARSER_PACKAGE_NAME : CMDLINE_PARSER_PACKAGE),
     CMDLINE_PARSER_VERSION);
}

static void print_help_common(void) {
  cmdline_parser_print_version ();

  if (strlen(gengetopt_args_info_purpose) > 0)
    printf("\n%s\n", gengetopt_args_info_purpose);

  if (strlen(gengetopt_args_info_usage) > 0)
    printf("\n%s\n", gengetopt_args_info_usage);

  printf("\n");

  if (strlen(gengetopt_args_info_description) > 0)
    printf("%s\n\n", gengetopt_args_info_description);
}

void
cmdline_parser_print_help (void)
{
  int i = 0;
  print_help_common();
  while (gengetopt_args_info_help[i])
    printf("%s\n", gengetopt_args_info_help[i++]);
}

void
cmdline_parser_print_full_help (void)
{
  int i = 0;
  print_help_common();
  while (gengetopt_args_info_full_help[i])
    printf("%s\n", gengetopt_args_info_full_help[i++]);
}

void
cmdline_parser_print_detailed_help (void)
{
  int i = 0;
  print_help_common();
  while (gengetopt_args_info_detailed_help[i])
    printf("%s\n", gengetopt_args_info_detailed_help[i++]);
}

void
cmdline_parser_init (struct gengetopt_args_info *args_info)
{
  clear_given (args_info);
  clear_args (args_info);
  init_args_info (args_info);

  args_info->inputs = 0;
  args_info->inputs_num = 0;
}

void
cmdline_parser_params_init(struct cmdline_parser_params *params)
{
  if (params)
    { 
      params->override = 0;
      params->initialize = 1;
      params->check_required = 1;
      params->check_ambiguity = 0;
      params->print_errors = 1;
    }
}

struct cmdline_parser_params *
cmdline_parser_params_create(void)
{
  struct cmdline_parser_params *params = 
    (struct cmdline_parser_params *)malloc(sizeof(struct cmdline_parser_params));
  cmdline_parser_params_init(params);  
  return params;
}

static void
free_string_field (char **s)
{
  if (*s)
    {
      free (*s);
      *s = 0;
    }
}


static void
cmdline_parser_release (struct gengetopt_args_info *args_info)
{
  unsigned int i;
  free_string_field (&(args_info->str_opt_arg));
  free_string_field (&(args_info->str_opt_orig));
  free_string_field (&(args_info->my_opt_orig));
  free_string_field (&(args_info->int_opt_orig));
  free_string_field (&(args_info->long_opt_orig));
  free_string_field (&(args_info->def_opt_arg));
  free_string_field (&(args_info->def_opt_orig));
  free_string_field (&(args_info->enum_opt_arg));
  free_string_field (&(args_info->enum_opt_orig));
  free_string_field (&(args_info->secret_orig));
  free_string_field (&(args_info->dependant_orig));
  
  
  for (i = 0; i < args_info->inputs_num; ++i)
    free (args_info->inputs [i]);

  if (args_info->inputs_num)
    free (args_info->inputs);

  clear_given (args_info);
}

/**
 * @param val the value to check
 * @param values the possible values
 * @return the index of the matched value:
 * -1 if no value matched,
 * -2 if more than one value has matched
 */
static int
check_possible_values(const char *val, const char *values[])
{
  int i, found, last;
  size_t len;

  if (!val)   /* otherwise strlen() crashes below */
    return -1; /* -1 means no argument for the option */

  found = last = 0;

  for (i = 0, len = strlen(val); values[i]; ++i)
    {
      if (strncmp(val, values[i], len) == 0)
        {
          ++found;
          last = i;
          if (strlen(values[i]) == len)
            return i; /* exact macth no need to check more */
        }
    }

  if (found == 1) /* one match: OK */
    return last;

  return (found ? -2 : -1); /* return many values or none matched */
}


static void
write_into_file(FILE *outfile, const char *opt, const char *arg, const char *values[])
{
  int found = -1;
  if (arg) {
    if (values) {
      found = check_possible_values(arg, values);      
    }
    if (found >= 0)
      fprintf(outfile, "%s=\"%s\" # %s\n", opt, arg, values[found]);
    else
      fprintf(outfile, "%s=\"%s\"\n", opt, arg);
  } else {
    fprintf(outfile, "%s\n", opt);
  }
}


int
cmdline_parser_dump(FILE *outfile, struct gengetopt_args_info *args_info)
{
  int i = 0;

  if (!outfile)
    {
      fprintf (stderr, "%s: cannot dump options to stream\n", CMDLINE_PARSER_PACKAGE);
      return EXIT_FAILURE;
    }

  if (args_info->help_given)
    write_into_file(outfile, "help", 0, 0 );
  if (args_info->detailed_help_given)
    write_into_file(outfile, "detailed-help", 0, 0 );
  if (args_info->full_help_given)
    write_into_file(outfile, "full-help", 0, 0 );
  if (args_info->version_given)
    write_into_file(outfile, "version", 0, 0 );
  if (args_info->str_opt_given)
    write_into_file(outfile, "str-opt", args_info->str_opt_orig, 0);
  if (args_info->my_opt_given)
    write_into_file(outfile, "my-opt", args_info->my_opt_orig, 0);
  if (args_info->int_opt_given)
    write_into_file(outfile, "int-opt", args_info->int_opt_orig, 0);
  if (args_info->flag_opt_given)
    write_into_file(outfile, "flag-opt", 0, 0 );
  if (args_info->funct_opt_given)
    write_into_file(outfile, "funct-opt", 0, 0 );
  if (args_info->long_opt_given)
    write_into_file(outfile, "long-opt", args_info->long_opt_orig, 0);
  if (args_info->def_opt_given)
    write_into_file(outfile, "def-opt", args_info->def_opt_orig, 0);
  if (args_info->enum_opt_given)
    write_into_file(outfile, "enum-opt", args_info->enum_opt_orig, cmdline_parser_enum_opt_values);
  if (args_info->secret_given)
    write_into_file(outfile, "secret", args_info->secret_orig, 0);
  if (args_info->dependant_given)
    write_into_file(outfile, "dependant", args_info->dependant_orig, 0);
  

  i = EXIT_SUCCESS;
  return i;
}

int
cmdline_parser_file_save(const char *filename, struct gengetopt_args_info *args_info)
{
  FILE *outfile;
  int i = 0;

  outfile = fopen(filename, "w");

  if (!outfile)
    {
      fprintf (stderr, "%s: cannot open file for writing: %s\n", CMDLINE_PARSER_PACKAGE, filename);
      return EXIT_FAILURE;
    }

  i = cmdline_parser_dump(outfile, args_info);
  fclose (outfile);

  return i;
}

void
cmdline_parser_free (struct gengetopt_args_info *args_info)
{
  cmdline_parser_release (args_info);
}

/** @brief replacement of strdup, which is not standard */
char *
gengetopt_strdup (const char *s)
{
  char *result = 0;
  if (!s)
    return result;

  result = (char*)malloc(strlen(s) + 1);
  if (result == (char*)0)
    return (char*)0;
  strcpy(result, s);
  return result;
}

int
cmdline_parser (int argc, char **argv, struct gengetopt_args_info *args_info)
{
  return cmdline_parser2 (argc, argv, args_info, 0, 1, 1);
}

int
cmdline_parser_ext (int argc, char **argv, struct gengetopt_args_info *args_info,
                   struct cmdline_parser_params *params)
{
  int result;
  result = cmdline_parser_internal (argc, argv, args_info, params, 0);

  if (result == EXIT_FAILURE)
    {
      cmdline_parser_free (args_info);
      exit (EXIT_FAILURE);
    }
  
  return result;
}

int
cmdline_parser2 (int argc, char **argv, struct gengetopt_args_info *args_info, int override, int initialize, int check_required)
{
  int result;
  struct cmdline_parser_params params;
  
  params.override = override;
  params.initialize = initialize;
  params.check_required = check_required;
  params.check_ambiguity = 0;
  params.print_errors = 1;

  result = cmdline_parser_internal (argc, argv, args_info, &params, 0);

  if (result == EXIT_FAILURE)
    {
      cmdline_parser_free (args_info);
      exit (EXIT_FAILURE);
    }
  
  return result;
}

int
cmdline_parser_required (struct gengetopt_args_info *args_info, const char *prog_name)
{
  int result = EXIT_SUCCESS;

  if (cmdline_parser_required2(args_info, prog_name, 0) > 0)
    result = EXIT_FAILURE;

  if (result == EXIT_FAILURE)
    {
      cmdline_parser_free (args_info);
      exit (EXIT_FAILURE);
    }
  
  return result;
}

int
cmdline_parser_required2 (struct gengetopt_args_info *args_info, const char *prog_name, const char *additional_error)
{
  int error = 0;
  FIX_UNUSED (additional_error);

  /* checks for required options */
  if (! args_info->int_opt_given)
    {
      fprintf (stderr, "%s: '--int-opt' ('-i') option required%s\n", prog_name, (additional_error ? additional_error : ""));
      error = 1;
    }
  
  
  /* checks for dependences among options */
  if (args_info->dependant_given && ! args_info->str_opt_given)
    {
      fprintf (stderr, "%s: '--dependant' ('-D') option depends on option 'str-opt'%s\n", prog_name, (additional_error ? additional_error : ""));
      error = 1;
    }

  return error;
}


static char *package_name = 0;

/**
 * @brief updates an option
 * @param field the generic pointer to the field to update
 * @param orig_field the pointer to the orig field
 * @param field_given the pointer to the number of occurrence of this option
 * @param prev_given the pointer to the number of occurrence already seen
 * @param value the argument for this option (if null no arg was specified)
 * @param possible_values the possible values for this option (if specified)
 * @param default_value the default value (in case the option only accepts fixed values)
 * @param arg_type the type of this option
 * @param check_ambiguity @see cmdline_parser_params.check_ambiguity
 * @param override @see cmdline_parser_params.override
 * @param no_free whether to free a possible previous value
 * @param multiple_option whether this is a multiple option
 * @param long_opt the corresponding long option
 * @param short_opt the corresponding short option (or '-' if none)
 * @param additional_error possible further error specification
 */
static
int update_arg(void *field, char **orig_field,
               unsigned int *field_given, unsigned int *prev_given, 
               char *value, const char *possible_values[],
               const char *default_value,
               cmdline_parser_arg_type arg_type,
               int check_ambiguity, int override,
               int no_free, int multiple_option,
               const char *long_opt, char short_opt,
               const char *additional_error)
{
  char *stop_char = 0;
  const char *val = value;
  int found;
  char **string_field;
  FIX_UNUSED (field);

  stop_char = 0;
  found = 0;

  if (!multiple_option && prev_given && (*prev_given || (check_ambiguity && *field_given)))
    {
      if (short_opt != '-')
        fprintf (stderr, "%s: `--%s' (`-%c') option given more than once%s\n", 
               package_name, long_opt, short_opt,
               (additional_error ? additional_error : ""));
      else
        fprintf (stderr, "%s: `--%s' option given more than once%s\n", 
               package_name, long_opt,
               (additional_error ? additional_error : ""));
      return 1; /* failure */
    }

  if (possible_values && (found = check_possible_values((value ? value : default_value), possible_values)) < 0)
    {
      if (short_opt != '-')
        fprintf (stderr, "%s: %s argument, \"%s\", for option `--%s' (`-%c')%s\n", 
          package_name, (found == -2) ? "ambiguous" : "invalid", value, long_opt, short_opt,
          (additional_error ? additional_error : ""));
      else
        fprintf (stderr, "%s: %s argument, \"%s\", for option `--%s'%s\n", 
          package_name, (found == -2) ? "ambiguous" : "invalid", value, long_opt,
          (additional_error ? additional_error : ""));
      return 1; /* failure */
    }
    
  if (field_given && *field_given && ! override)
    return 0;
  if (prev_given)
    (*prev_given)++;
  if (field_given)
    (*field_given)++;
  if (possible_values)
    val = possible_values[found];

  switch(arg_type) {
  case ARG_FLAG:
    *((int *)field) = !*((int *)field);
    break;
  case ARG_INT:
    if (val) *((int *)field) = strtol (val, &stop_char, 0);
    break;
  case ARG_LONG:
    if (val) *((long *)field) = (long)strtol (val, &stop_char, 0);
    break;
  case ARG_STRING:
    if (val) {
      string_field = (char **)field;
      if (!no_free && *string_field)
        free (*string_field); /* free previous string */
      *string_field = gengetopt_strdup (val);
    }
    break;
  default:
    break;
  };

  /* check numeric conversion */
  switch(arg_type) {
  case ARG_INT:
  case ARG_LONG:
    if (val && !(stop_char && *stop_char == '\0')) {
      fprintf(stderr, "%s: invalid numeric value: %s\n", package_name, val);
      return 1; /* failure */
    }
    break;
  default:
    ;
  };

  /* store the original value */
  switch(arg_type) {
  case ARG_NO:
  case ARG_FLAG:
    break;
  default:
    if (value && orig_field) {
      if (no_free) {
        *orig_field = value;
      } else {
        if (*orig_field)
          free (*orig_field); /* free previous string */
        *orig_field = gengetopt_strdup (value);
      }
    }
  };

  return 0; /* OK */
}


int
cmdline_parser_internal (
  int argc, char **argv, struct gengetopt_args_info *args_info,
                        struct cmdline_parser_params *params, const char *additional_error)
{
  int c;	/* Character of the parsed option.  */

  int error = 0;
  struct gengetopt_args_info local_args_info;
  
  int override;
  int initialize;
  int check_required;
  int check_ambiguity;
  
  package_name = argv[0];
  
  override = params->override;
  initialize = params->initialize;
  check_required = params->check_required;
  check_ambiguity = params->check_ambiguity;

  if (initialize)
    cmdline_parser_init (args_info);

  cmdline_parser_init (&local_args_info);

  optarg = 0;
  optind = 0;
  opterr = params->print_errors;
  optopt = '?';

  while (1)
    {
      int option_index = 0;

      static struct option long_options[] = {
        { "help",	0, NULL, 'h' },
        { "detailed-help",	0, NULL, 0 },
        { "full-help",	0, NULL, 0 },
        { "version",	0, NULL, 'V' },
        { "str-opt",	1, NULL, 's' },
        { "my-opt",	1, NULL, 'm' },
        { "int-opt",	1, NULL, 'i' },
        { "flag-opt",	0, NULL, 0 },
        { "funct-opt",	0, NULL, 'F' },
        { "long-opt",	1, NULL, 0 },
        { "def-opt",	1, NULL, 0 },
        { "enum-opt",	1, NULL, 0 },
        { "secret",	1, NULL, 'S' },
        { "dependant",	1, NULL, 'D' },
        { 0,  0, 0, 0 }
      };

      c = getopt_long (argc, argv, "hVs:m:i:FS:D:", long_options, &option_index);

      if (c == -1) break;	/* Exit from `while (1)' loop.  */

      switch (c)
        {
        case 'h':	/* Print help and exit.  */
          cmdline_parser_print_help ();
          cmdline_parser_free (&local_args_info);
          exit (EXIT_SUCCESS);

        case 'V':	/* Print version and exit.  */
          cmdline_parser_print_version ();
          cmdline_parser_free (&local_args_info);
          exit (EXIT_SUCCESS);

        case 's':	/* A string option, for a filename.  */
        
        
          if (update_arg( (void *)&(args_info->str_opt_arg), 
               &(args_info->str_opt_orig), &(args_info->str_opt_given),
              &(local_args_info.str_opt_given), optarg, 0, 0, ARG_STRING,
              check_ambiguity, override, 0, 0,
              "str-opt", 's',
              additional_error))
            goto failure;
        
          break;
        case 'm':	/* Another integer option, this time the description of the option should be \"quite\" long to require wrapping... possibly more than one wrapping :-) especially if I\nrequire a line break.  */
        
        
          if (update_arg( (void *)&(args_info->my_opt_arg), 
               &(args_info->my_opt_orig), &(args_info->my_opt_given),
              &(local_args_info.my_opt_given), optarg, 0, 0, ARG_INT,
              check_ambiguity, override, 0, 0,
              "my-opt", 'm',
              additional_error))
            goto failure;
        
          break;
        case 'i':	/* A int option.  */
        
        
          if (update_arg( (void *)&(args_info->int_opt_arg), 
               &(args_info->int_opt_orig), &(args_info->int_opt_given),
              &(local_args_info.int_opt_given), optarg, 0, 0, ARG_INT,
              check_ambiguity, override, 0, 0,
              "int-opt", 'i',
              additional_error))
            goto failure;
        
          break;
        case 'F':	/* A function option.  */
        
        
          if (update_arg( 0 , 
               0 , &(args_info->funct_opt_given),
              &(local_args_info.funct_opt_given), optarg, 0, 0, ARG_NO,
              check_ambiguity, override, 0, 0,
              "funct-opt", 'F',
              additional_error))
            goto failure;
        
          break;
        case 'S':	/* hidden option will not appear in --help.  */
        
        
          if (update_arg( (void *)&(args_info->secret_arg), 
               &(args_info->secret_orig), &(args_info->secret_given),
              &(local_args_info.secret_given), optarg, 0, 0, ARG_INT,
              check_ambiguity, override, 0, 0,
              "secret", 'S',
              additional_error))
            goto failure;
        
          break;
        case 'D':	/* option that depends on str-opt.  */
        
        
          if (update_arg( (void *)&(args_info->dependant_arg), 
               &(args_info->dependant_orig), &(args_info->dependant_given),
              &(local_args_info.dependant_given), optarg, 0, 0, ARG_INT,
              check_ambiguity, override, 0, 0,
              "dependant", 'D',
              additional_error))
            goto failure;
        
          break;

        case 0:	/* Long option with no short option */
          if (strcmp (long_options[option_index].name, "detailed-help") == 0) {
            cmdline_parser_print_detailed_help ();
            cmdline_parser_free (&local_args_info);
            exit (EXIT_SUCCESS);
          }

          if (strcmp (long_options[option_index].name, "full-help") == 0) {
            cmdline_parser_print_full_help ();
            cmdline_parser_free (&local_args_info);
            exit (EXIT_SUCCESS);
          }

          /* A flag option.  */
          if (strcmp (long_options[option_index].name, "flag-opt") == 0)
          {
          
          
            if (update_arg((void *)&(args_info->flag_opt_flag), 0, &(args_info->flag_opt_given),
                &(local_args_info.flag_opt_given), optarg, 0, 0, ARG_FLAG,
                check_ambiguity, override, 1, 0, "flag-opt", '-',
                additional_error))
              goto failure;
          
          }
          /* A long option.  */
          else if (strcmp (long_options[option_index].name, "long-opt") == 0)
          {
          
          
            if (update_arg( (void *)&(args_info->long_opt_arg), 
                 &(args_info->long_opt_orig), &(args_info->long_opt_given),
                &(local_args_info.long_opt_given), optarg, 0, 0, ARG_LONG,
                check_ambiguity, override, 0, 0,
                "long-opt", '-',
                additional_error))
              goto failure;
          
          }
          /* A string option with default.  */
          else if (strcmp (long_options[option_index].name, "def-opt") == 0)
          {
          
          
            if (update_arg( (void *)&(args_info->def_opt_arg), 
                 &(args_info->def_opt_orig), &(args_info->def_opt_given),
                &(local_args_info.def_opt_given), optarg, 0, "Hello", ARG_STRING,
                check_ambiguity, override, 0, 0,
                "def-opt", '-',
                additional_error))
              goto failure;
          
          }
          /* A string option with list of values.  */
          else if (strcmp (long_options[option_index].name, "enum-opt") == 0)
          {
          
          
            if (update_arg( (void *)&(args_info->enum_opt_arg), 
                 &(args_info->enum_opt_orig), &(args_info->enum_opt_given),
                &(local_args_info.enum_opt_given), optarg, cmdline_parser_enum_opt_values, "hello", ARG_STRING,
                check_ambiguity, override, 0, 0,
                "enum-opt", '-',
                additional_error))
              goto failure;
          
          }
          
          break;
        case '?':	/* Invalid option.  */
          /* `getopt_long' already printed an error message.  */
          goto failure;

        default:	/* bug: option not considered.  */
          fprintf (stderr, "%s: option unknown: %c%s\n", CMDLINE_PARSER_PACKAGE, c, (additional_error ? additional_error : ""));
          abort ();
        } /* switch */
    } /* while */



  if (check_required)
    {
      error += cmdline_parser_required2 (args_info, argv[0], additional_error);
    }

  cmdline_parser_release (&local_args_info);

  if ( error )
    return (EXIT_FAILURE);

  if (optind < argc)
    {
      int i = 0 ;
      int found_prog_name = 0;
      /* whether program name, i.e., argv[0], is in the remaining args
         (this may happen with some implementations of getopt,
          but surely not with the one included by gengetopt) */

      i = optind;
      while (i < argc)
        if (argv[i++] == argv[0]) {
          found_prog_name = 1;
          break;
        }
      i = 0;

      args_info->inputs_num = argc - optind - found_prog_name;
      args_info->inputs =
        (char **)(malloc ((args_info->inputs_num)*sizeof(char *))) ;
      while (optind < argc)
        if (argv[optind++] != argv[0])
          args_info->inputs[ i++ ] = gengetopt_strdup (argv[optind-1]) ;
    }

  return 0;

failure:
  
  cmdline_parser_release (&local_args_info);
  return (EXIT_FAILURE);
}
