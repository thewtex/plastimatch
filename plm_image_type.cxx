/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <string.h>
#include "plm_image_type.h"

Plm_image_type
plm_image_type_parse (const char* string)
{
    if (!strcmp (string,"char")) {
	return PLM_IMG_TYPE_ITK_CHAR;
    }
    else if (!strcmp (string,"mask") || !strcmp (string,"uchar")) {
	return PLM_IMG_TYPE_ITK_UCHAR;
    }
    else if (!strcmp (string,"short")) {
	return PLM_IMG_TYPE_ITK_SHORT;
    }
    else if (!strcmp (string,"ushort")) {
	return PLM_IMG_TYPE_ITK_USHORT;
    }
    else if (!strcmp (string,"int") || !strcmp (string,"long")
	     || !strcmp (string,"int32")) {
	return PLM_IMG_TYPE_ITK_LONG;
    }
    else if (!strcmp (string,"uint") || !strcmp (string,"ulong")
	     || !strcmp (string,"uint32")) {
	return PLM_IMG_TYPE_ITK_ULONG;
    }
    else if (!strcmp (string,"float")) {
	return PLM_IMG_TYPE_ITK_FLOAT;
    }
    else if (!strcmp (string,"vf")) {
	return PLM_IMG_TYPE_ITK_FLOAT_FIELD;
    }
    else if (!strcmp (string,"ssimg")) {
	return PLM_IMG_TYPE_ITK_UCHAR_4D;
    }
    else {
	return PLM_IMG_TYPE_UNDEFINED;
    }
}

char* 
plm_image_type_string (Plm_image_type type)
{
    switch (type) {
    case PLM_IMG_TYPE_UNDEFINED:
	return "PLM_IMG_TYPE_UNDEFINED";
    case PLM_IMG_TYPE_ITK_CHAR:
	return "PLM_IMG_TYPE_ITK_CHAR";
    case PLM_IMG_TYPE_ITK_UCHAR:
	return "PLM_IMG_TYPE_ITK_UCHAR";
    case PLM_IMG_TYPE_ITK_SHORT:
	return "PLM_IMG_TYPE_ITK_SHORT";
    case PLM_IMG_TYPE_ITK_USHORT:
	return "PLM_IMG_TYPE_ITK_USHORT";
    case PLM_IMG_TYPE_ITK_LONG:
	return "PLM_IMG_TYPE_ITK_LONG";
    case PLM_IMG_TYPE_ITK_ULONG:
	return "PLM_IMG_TYPE_ITK_ULONG";
    case PLM_IMG_TYPE_ITK_FLOAT:
	return "PLM_IMG_TYPE_ITK_FLOAT";
    case PLM_IMG_TYPE_ITK_DOUBLE:
	return "PLM_IMG_TYPE_ITK_DOUBLE";
    case PLM_IMG_TYPE_ITK_FLOAT_FIELD:
	return "PLM_IMG_TYPE_ITK_FLOAT_FIELD";
    case PLM_IMG_TYPE_GPUIT_UCHAR:
	return "PLM_IMG_TYPE_GPUIT_UCHAR";
    case PLM_IMG_TYPE_GPUIT_SHORT:
	return "PLM_IMG_TYPE_GPUIT_SHORT";
    case PLM_IMG_TYPE_GPUIT_UINT16:
	return "PLM_IMG_TYPE_GPUIT_UINT16";
    case PLM_IMG_TYPE_GPUIT_UINT32:
	return "PLM_IMG_TYPE_GPUIT_UINT32";
    case PLM_IMG_TYPE_GPUIT_FLOAT:
	return "PLM_IMG_TYPE_GPUIT_FLOAT";
    case PLM_IMG_TYPE_GPUIT_FLOAT_FIELD:
	return "PLM_IMG_TYPE_GPUIT_FLOAT_FIELD";
    case PLM_IMG_TYPE_ITK_UCHAR_4D:
	return "PLM_IMG_TYPE_ITK_UCHAR_4D";
    default:
	return "(unknown image type)";
    }
}
