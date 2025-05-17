#include <Catalog.h>
#include <SupportDefs.h>
#include <TypeConstants.h>
#include <cstring>
#include <sys/socket.h>
#include "mainwindow.h"

[[maybe_unused]] type_code
TypeCodeForString(const char* str)
{
	if(strcmp(str, "B_AFFINE_TRANSFORM_TYPE") == 0)
		return B_AFFINE_TRANSFORM_TYPE;
	else if(strcmp(str, "B_ALIGNMENT_TYPE") == 0)
		return B_ALIGNMENT_TYPE;
	else if(strcmp(str, "B_ATOM_TYPE") == 0)
		return B_ATOM_TYPE;
	else if(strcmp(str, "B_ATOMREF_TYPE") == 0)
		return B_ATOMREF_TYPE;
	else if(strcmp(str, "B_BOOL_TYPE") == 0)
		return B_BOOL_TYPE;
	else if(strcmp(str, "B_CHAR_TYPE") == 0)
		return B_CHAR_TYPE;
	else if(strcmp(str, "B_COLOR_8_BIT_TYPE") == 0)
		return B_COLOR_8_BIT_TYPE;
	else if(strcmp(str, "B_DOUBLE_TYPE") == 0)
		return B_DOUBLE_TYPE;
	else if(strcmp(str, "B_FLOAT_TYPE") == 0)
		return B_FLOAT_TYPE;
	else if(strcmp(str, "B_GRAYSCALE_8_BIT_TYPE") == 0)
		return B_GRAYSCALE_8_BIT_TYPE;
	else if(strcmp(str, "B_INT16_TYPE") == 0)
		return B_INT16_TYPE;
	else if(strcmp(str, "B_INT32_TYPE") == 0)
		return B_INT32_TYPE;
	else if(strcmp(str, "B_INT64_TYPE") == 0)
		return B_INT64_TYPE;
	else if(strcmp(str, "B_INT8_TYPE") == 0)
		return B_INT8_TYPE;
	else if(strcmp(str, "B_LARGE_ICON_TYPE") == 0)
		return B_LARGE_ICON_TYPE;
	else if(strcmp(str, "B_MEDIA_PARAMETER_GROUP_TYPE") == 0)
		return B_MEDIA_PARAMETER_GROUP_TYPE;
	else if(strcmp(str, "B_MEDIA_PARAMETER_TYPE") == 0)
		return B_MEDIA_PARAMETER_TYPE;
	else if(strcmp(str, "B_MEDIA_PARAMETER_WEB_TYPE") == 0)
		return B_MEDIA_PARAMETER_WEB_TYPE;
	else if(strcmp(str, "B_MESSAGE_TYPE") == 0)
		return B_MESSAGE_TYPE;
	else if(strcmp(str, "B_MESSENGER_TYPE") == 0)
		return B_MESSENGER_TYPE;
	else if(strcmp(str, "B_MIME_TYPE") == 0)
		return B_MIME_TYPE;
	else if(strcmp(str, "B_MINI_ICON_TYPE") == 0)
		return B_MINI_ICON_TYPE;
	else if(strcmp(str, "B_MONOCHROME_1_BIT_TYPE") == 0)
		return B_MONOCHROME_1_BIT_TYPE;
	else if(strcmp(str, "B_OBJECT_TYPE") == 0)
		return B_OBJECT_TYPE;
	else if(strcmp(str, "B_OFF_T_TYPE") == 0)
		return B_OFF_T_TYPE;
	else if(strcmp(str, "B_PATTERN_TYPE") == 0)
		return B_PATTERN_TYPE;
	else if(strcmp(str, "B_POINTER_TYPE") == 0)
		return B_POINTER_TYPE;
	else if(strcmp(str, "B_POINT_TYPE") == 0)
		return B_POINT_TYPE;
	else if(strcmp(str, "B_PROPERTY_INFO_TYPE") == 0)
		return B_PROPERTY_INFO_TYPE;
	else if(strcmp(str, "B_RAW_TYPE") == 0)
		return B_RAW_TYPE;
	else if(strcmp(str, "B_RECT_TYPE") == 0)
		return B_RECT_TYPE;
	else if(strcmp(str, "B_REF_TYPE") == 0)
		return B_REF_TYPE;
	else if(strcmp(str, "B_NODE_REF_TYPE") == 0)
		return B_NODE_REF_TYPE;
	else if(strcmp(str, "B_RGB_32_BIT_TYPE") == 0)
		return B_RGB_32_BIT_TYPE;
	else if(strcmp(str, "B_RGB_COLOR_TYPE") == 0)
		return B_RGB_COLOR_TYPE;
	else if(strcmp(str, "B_SIZE_TYPE") == 0)
		return B_SIZE_TYPE;
	else if(strcmp(str, "B_SIZE_T_TYPE") == 0)
		return B_SIZE_T_TYPE;
	else if(strcmp(str, "B_SSIZE_T_TYPE") == 0)
		return B_SSIZE_T_TYPE;
	else if(strcmp(str, "B_STRING_TYPE") == 0)
		return B_STRING_TYPE;
	else if(strcmp(str, "B_STRING_LIST_TYPE") == 0)
		return B_STRING_LIST_TYPE;
	else if(strcmp(str, "B_TIME_TYPE") == 0)
		return B_TIME_TYPE;
	else if(strcmp(str, "B_UINT16_TYPE") == 0)
		return B_UINT16_TYPE;
	else if(strcmp(str, "B_UINT32_TYPE") == 0)
		return B_UINT32_TYPE;
	else if(strcmp(str, "B_UINT64_TYPE") == 0)
		return B_UINT64_TYPE;
	else if(strcmp(str, "B_UINT8_TYPE") == 0)
		return B_UINT8_TYPE;
	else if(strcmp(str, "B_VECTOR_ICON_TYPE") == 0)
		return B_VECTOR_ICON_TYPE;
	else if(strcmp(str, "B_XATTR_TYPE") == 0)
		return B_XATTR_TYPE;
	else if(strcmp(str, "B_NETWORK_ADDRESS_TYPE") == 0)
		return B_NETWORK_ADDRESS_TYPE;
	else if(strcmp(str, "B_MIME_STRING_TYPE") == 0)
		return B_MIME_STRING_TYPE;

	return B_ANY_TYPE;
}

[[maybe_unused]] type_code
TypeCodeForCommand(uint32 command)
{
	switch(command)
	{
		case MW_ADD_AFFINE_TX:
			return B_AFFINE_TRANSFORM_TYPE;
		case MW_ADD_ALIGNMENT:
			return B_ALIGNMENT_TYPE;
		case MW_ADD_BOOL:
			return B_BOOL_TYPE;
		case MW_ADD_COLOR:
			return B_RGB_COLOR_TYPE;
		case MW_ADD_INT8:
			return B_INT8_TYPE;
		case MW_ADD_INT16:
			return B_INT16_TYPE;
		case MW_ADD_INT32:
			return B_INT32_TYPE;
		case MW_ADD_INT64:
			return B_INT64_TYPE;
		case MW_ADD_UINT8:
			return B_UINT8_TYPE;
		case MW_ADD_UINT16:
			return B_UINT16_TYPE;
		case MW_ADD_UINT32:
			return B_UINT32_TYPE;
		case MW_ADD_UINT64:
			return B_UINT64_TYPE;
		case MW_ADD_OFF_T:
			return B_OFF_T_TYPE;
		case MW_ADD_SIZE_T:
			return B_SIZE_T_TYPE;
		case MW_ADD_SSIZE_T:
			return B_SSIZE_T_TYPE;
		case MW_ADD_FLOAT:
			return B_FLOAT_TYPE;
		case MW_ADD_DOUBLE:
			return B_DOUBLE_TYPE;
		case MW_ADD_ENTRY_REF:
			return B_REF_TYPE;
		case MW_ADD_NODE_REF:
			return B_NODE_REF_TYPE;
		case MW_ADD_STRING:
			return B_STRING_TYPE;
		case MW_ADD_POINT:
			return B_POINT_TYPE;
		case MW_ADD_SIZE:
			return B_SIZE_TYPE;
		case MW_ADD_RECT:
			return B_RECT_TYPE;
		case MW_ADD_TIME:
			return B_TIME_TYPE;
		default:
			return B_ANY_TYPE;
	}
}

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "net addr family stringer"

[[maybe_unused]] BString
NetAddressFamilyString(sa_family_t family)
{
	switch(family)
	{
		case AF_UNSPEC:
			return B_TRANSLATE("Unspecified");
		case AF_INET:
			return B_TRANSLATE("Internet protocol (IPv4)");
		case AF_APPLETALK:
			return B_TRANSLATE("AppleTalk");
		case AF_ROUTE:
			return B_TRANSLATE("Internal Routing Protocol");
		case AF_LINK:
			return B_TRANSLATE("Link layer interface");
		case AF_INET6:
			return B_TRANSLATE("Internet protocol (IPv6)");
		case AF_DLI:
			return B_TRANSLATE("DEC Direct data link interface");
		case AF_IPX:
			return B_TRANSLATE("Novell Internet Protocol");
		case AF_NOTIFY: // Not documented anywhere
			return B_TRANSLATE("AF_NOTIFY");
		case AF_LOCAL: // Alias: AF_UNIX
			return B_TRANSLATE("Local to host (UNIX)");
		case AF_BLUETOOTH:
			return B_TRANSLATE("Bluetooth");
		default:
			return B_TRANSLATE("Unknown");
	}
}
