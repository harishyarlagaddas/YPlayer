#include "string_utils.h"

int8* ConvertUTF16ToUTF8(int16 *aInputStr)
{
	if(NULL == aInputStr){
		return NULL;
	}

	int32 len = 0;
	int16 *inPtr = aInputStr;
	int8 *outStr = NULL, *outStrPtr = NULL;
	UTF16ToUTF8Struct utf16Struct;

	/* Fist calculate the length of the string for allocating the memory accordingly. */
	utf16Struct.UTF16Val = *inPtr++;
	while('\0' != utf16Struct.UTF16Val) {
		if ((utf16Struct.UTF8Val[1] == 0) && (utf16Struct.UTF8Val[0] < 0x80)) {
			len ++;
		} else if (utf16Struct.UTF8Val[1] < 0x08) {
			len += 2;
		} else {
			len += 3;
		}
		utf16Struct.UTF16Val = *inPtr++;
	}
	if(0 == len){
		return NULL;
	}

	len++;
	outStr = (int8*)os_malloc(len);
	if (NULL!= outStr){
		os_memset(outStr,0,len);
		outStrPtr = outStr;
		/* Again initialize the pointer back. Now actual conversion starts. */
		inPtr = aInputStr;
		utf16Struct.UTF16Val = *inPtr++;
		while('\0' != utf16Struct.UTF16Val){
		   if ((utf16Struct.UTF8Val[1] == 0) && (utf16Struct.UTF8Val[0] < 0x80)) {
			*outStrPtr++ = utf16Struct.UTF8Val[0];
		    } else if(utf16Struct.UTF8Val[1] < 0x08) {
			*outStrPtr++ = (int8)(0xC0 | (utf16Struct.UTF8Val[0] >> 6) | (utf16Struct.UTF8Val[1] << 2));
			*outStrPtr++ = (int8)(0x80 | (utf16Struct.UTF8Val[0] & 0x3F));
		    } else {
			*outStrPtr++ = (int8)(0xE0 | (utf16Struct.UTF8Val[1] >> 4));
			*outStrPtr++ = (int8)(0x80 | (((utf16Struct.UTF8Val[0] >> 6) | (utf16Struct.UTF8Val[1] << 2)) & 0x3F));
			*outStrPtr++ = (int8)(0x80 | (utf16Struct.UTF8Val[0] & 0x3F));
		    }
		    utf16Struct.UTF16Val = *inPtr++;
		}
		return outStr;
	}else{
		return NULL;
	}
}
