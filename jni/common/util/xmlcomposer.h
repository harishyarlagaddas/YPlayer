#ifndef XML_COMPOSER_H_INCLUDED
#define XML_COMPOSER_H_INCLUDED

#include "../include/basic_datatypes.h"
#include "../../os/os.h"
#include "../include/return_codes.h"
#include "../log/log.h"

typedef enum paramType{
	XML_INTEGER = 1,
	XML_STRING = 2,
	XML_NOVALUE = 3
}ParamType;

typedef struct{
	ParamType paramType;
	uint32 IntegerVal;
	struct{
		int32 Len;
		int8 *Val;
	}StringVal;
}XmlParam;

typedef struct xmlComposerTag{
	XmlParam Name;
	XmlParam Value;
	XmlParam AttributeName;
	XmlParam AttributeValue;
	XmlParam ParentName;
	XmlParam Depth;
	XmlParam ParentDepth;
}XmlComposerTag;

typedef struct xmlMatrix{
	XmlComposerTag *NodePtr;
	xmlMatrix *DownPtr;
	xmlMatrix *SidePtr;
	xmlMatrix *BackPtr;
}XmlMatrix;

class cXmlComposer
{
public:
	cXmlComposer();
	~cXmlComposer();
	ReturnStatus AddNode(XmlComposerTag aInputNode);
	uint32 GetXmlLength();
	ReturnStatus ComposeXml(int8 *aInputBuf, uint32 aInputBufLen);
private:
	XmlComposerTag* CreateXmlComposerTag(XmlComposerTag aInputNode);
	void DeleteXmlComposerTag(XmlComposerTag *aInputNodePtr);

	XmlMatrix *iXmlMatrix;
};
#endif /* XML_COMPOSER_H_INCLUDED */
