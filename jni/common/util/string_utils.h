#ifndef STRING_UTILS_H_INCLUDED
#define STRING_UTILS_H_INCLUDED

#include "../include/basic_datatypes.h"
#include "../../os/os.h"

typedef union{
	int8 UTF8Val[2];
	int16 UTF16Val;
}UTF16ToUTF8Struct;

int8* ConvertUTF16ToUTF8(int16 *aInputStr);

#endif /* STRING_UTILS_H_INCLUDED */
