
#ifndef XML_PARSER_H_INCLUDED
#define XML_PARSER_H_INCLUDED

#include "../include/basic_datatypes.h"
#include "../../os/os.h"
#include "../include/return_codes.h"
#include "../log/log.h"
#include "linkedlist.h"

typedef struct {
	int8 *AttribName;
	int8 *AttribValue;
}XmlParserAttribute;

typedef struct xmlParserTag{
	int8 *TagName;
	int8 *TagValue;
	cLinkedList<XmlParserAttribute> *AttribList;
	cLinkedList<struct xmlParserTag> *ChildTagList;
}XmlParserTag;


class cXmlParser
{
public:
	cXmlParser();
	~cXmlParser();
	ReturnStatus ParseXml(IN int8 *aXmlString,IN int32 logCount);
	ReturnStatus GetTagValue(IN int8 *aTagName, OUT int8 *aTagValue);
	ReturnStatus GetTagValue(IN int8 *aTagName, IN XmlParserTag aBaseTag,OUT int8 *aTagValue);
	ReturnStatus GetTagValue(IN XmlParserTag aInTag,OUT int8 *aTagValue);
	ReturnStatus GetTab(IN int8 *aTagName, OUT XmlParserTag &aParserTag);
	ReturnStatus GetParentTab(IN int8 *aTagName, IN XmlParserTag &aInTag,OUT XmlParserTag &aParserTag);
	int32 GetNumberOfAttributes(IN int8 *aTagName);
	int32 GetNumberOfAttributes(IN int8 *aTagName, XmlParserTag aBaseTag);
	int32 GetNumberOfAttributesOfTab(IN XmlParserTag aBaseTag);
	ReturnStatus GetAttributeAtIndex(IN int8 *aTagName, OUT XmlParserAttribute &aAttrib, IN int32 aIndex);
	ReturnStatus GetAttributeAtIndex(IN int8 *aTagName, IN XmlParserTag aBaseTag,OUT XmlParserAttribute &aAttrib, IN int32 aIndex);
	ReturnStatus GetAttributeOfTabAtIndex(IN XmlParserTag aBaseTag,OUT XmlParserAttribute &aAttrib, IN int32 aIndex);
	ReturnStatus GetSubTagAtIndex(IN int8 *aTagName,XmlParserTag &aSubTag,IN int32 aIndex);
	ReturnStatus GetRepeatedTagAtIndex(IN int8 *aTagName, IN XmlParserTag aInTag, OUT XmlParserTag &aOutTag, int32 count);
	int32 GetNumberOfSubTabs(IN int8 *aTagName);
	bool IsTagPresent(IN int8 *aTagName);
	bool IsTagPresent(IN int8 *aTagName, IN XmlParserTag aBaseTag);
	void ScanXmlString(IN int8 *aXmlString);
	void ResetParser();
private:
	/* Function Declarations. */
	int8* ExtractXmlVersion(IN int8 *aXmlString);
	int8* GetTagNameStartPosition(IN int8 *aXmlString);
	int8* GetTagNameEndPosition(IN int8 *aXmlString);
	int8* GetCharPos(IN int8 *aXmlString, int8 aMatchingChar);
	int8* RemoveStartingSpaces(IN int8 *aXmlString);
	int8* GetAttributePos(IN int8 *aXmlString);
	int8* FillTagName(IN int8 *aXmlString,OUT XmlParserTag &aTag);
	int8* ExtractAttributes(IN int8 *aXmlString,OUT XmlParserTag &aTag);
	int8* ExtractTagNameAndAttributes(IN int8 *aXmlString,OUT XmlParserTag &aTag);
	bool CheckForTagCompleteWithOutTagEnd(IN int8 *aXmlString);
	bool CheckForTagEnd(IN int8 *aXmlString,OUT XmlParserTag &aTag);
	int8* ExtractTagEnd(IN int8 *aXmlString,OUT XmlParserTag &aTag);
	bool CheckForTagValue(IN int8 *aXmlString);
	int8* ExtractTagValue(IN int8 *aXmlString,OUT XmlParserTag &aTag);
	bool CheckForNewTab(IN int8 *aXmlString);
	int8* ParseXmlTag(IN int8 *aXmlString,OUT XmlParserTag &aTag);
	void DeleteParserNodeMemAllocated(IN XmlParserTag &aXmlParserTag);
	ReturnStatus SearchForTag(IN int8 *aTagName, XmlParserTag &aXmlTag, XmlParserTag &aSearchResultTab,bool aParentTag=false);
	ReturnStatus SearchForRepeatedTag(IN int8 *aTagName, XmlParserTag &aXmlTag, XmlParserTag &aSearchResultTab, int32 count);

	/* Class Variables. */
	XmlParserTag cvXmlBaseTag;
	int8 *cvXMLVersionStr;
	FILE *cvFilePtr;
};

#endif /* LINKEDLIST_H_INCLUDED */
