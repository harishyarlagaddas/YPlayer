#ifndef XML_UTILS_H_INCLUDED
#define XML_UTILS_H_INCLUDED

#include "../include/basic_datatypes.h"
#include "../../os/os.h"
#include "../include/return_codes.h"
#include "../log/log.h"
#include "../../external/tinyxml/tinyxml.h"

class XmlUtils
{
public:
	static ReturnStatus GetTagValue(IN TiXmlDocument &aXmlDoc, IN int8* tagName, OUT int8* tagValue);
	static ReturnStatus GetTagValue(IN TiXmlNode* aNode, IN int8* aTagName, OUT int8* aTagValue);
	static int8* GetTagValue(IN TiXmlNode* aNode, IN int8* aTagName);
	static TiXmlNode* GetTag(IN TiXmlDocument &aXmlDoc, IN int8* aTagName);
	static TiXmlNode* GetTag(IN TiXmlNode* aNode, IN int8* aTagName);
	static bool IsTagPresent(IN TiXmlNode* aNode, IN int8* aTagName);
	static ReturnStatus ExtractTagValue(IN TiXmlNode* aNode, IN int8* aTagValue);
	static int8* ExtractTagValue(IN TiXmlNode* aNode);
	static int32 GetNumOfChilds(IN TiXmlNode* aNode);
	static TiXmlNode* GetSubTagAtIndex(IN TiXmlNode* aNode, int32 index);
	static TiXmlNode* GetRepetetiveTagAtIndex(IN TiXmlNode* aNode, IN int8* aTagName,int32 index);
	static int32 GetNumOfAttributes(IN TiXmlNode* aNode);
	static TiXmlAttribute* GetAttributeAtIndex(IN TiXmlNode* aNode, int32 index);
};
#endif
