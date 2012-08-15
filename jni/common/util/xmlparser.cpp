#include "xmlparser.h"

cXmlParser::cXmlParser()
{
	cvXMLVersionStr = NULL;
	cvFilePtr = NULL;
	os_memset(&cvXmlBaseTag,0,sizeof(cvXmlBaseTag));
}

cXmlParser::~cXmlParser()
{
	if(cvFilePtr){
	   fclose(cvFilePtr);
	}
	ResetParser();
}

void
cXmlParser::DeleteParserNodeMemAllocated(IN XmlParserTag &aXmlParserTag)
{
	/* First Delete the Tag Name And Tag Value Params */
	if(aXmlParserTag.TagName){
		os_free(aXmlParserTag.TagName);
		aXmlParserTag.TagName = NULL;
	}
	if(aXmlParserTag.TagValue){
		os_free(aXmlParserTag.TagValue);
		aXmlParserTag.TagValue = NULL;
	}
	if(aXmlParserTag.AttribList){
		/* Now start parsing AttribList and remove all the memroy allocated for it. */
		while(aXmlParserTag.AttribList->GetNumberOfElements()){
			XmlParserAttribute attrib;
			os_memset(&attrib,0,sizeof(attrib));
			if(SUCCESS == aXmlParserTag.AttribList->RemoveAtStart(attrib)){
				if(attrib.AttribName){
					os_free(attrib.AttribName);
					attrib.AttribName = NULL;
				}
				if(attrib.AttribValue){
					os_free(attrib.AttribValue);
					attrib.AttribValue = NULL;
				}
			}
		}
		os_delete(aXmlParserTag.AttribList);
		aXmlParserTag.AttribList = NULL;
	}
	if(aXmlParserTag.ChildTagList){
		/* Now start parsing Child TagList and remove all the memroy allocated for it. */
		while(aXmlParserTag.ChildTagList->GetNumberOfElements()){
			XmlParserTag childTag;
			os_memset(&childTag,0,sizeof(childTag));
			if(SUCCESS == aXmlParserTag.ChildTagList->RemoveAtStart(childTag)){
				DeleteParserNodeMemAllocated(childTag);
			}
		}
		os_delete(aXmlParserTag.ChildTagList);
		aXmlParserTag.ChildTagList = NULL;
	}
}

void
cXmlParser::ResetParser()
{
	/* Deallocate the memory properly. Other wise no escape from Mem Leaks. */
	/* Deallocate the XML Verstion String Memory if it is allocated */
	if(cvXMLVersionStr){
		os_free(cvXMLVersionStr);
		cvXMLVersionStr = NULL;
	}
	/* Now Start Deleting the mem allocated for Tags Starging from Base Tag. */
	DeleteParserNodeMemAllocated(cvXmlBaseTag);
}

void
cXmlParser::ScanXmlString(IN int8 *aXmlString)
{
	/* In this function scan the complete xml string and replace &lt; with < and &gt; with > inteligently. Because in some scenarios we may get
	   the xml sting in that format. Be prepare to parse that too.. */
	int8 *tempPtr = aXmlString;
	
	if(NULL == aXmlString){
		return;
	}

	do{
		tempPtr = os_strcasestr(tempPtr,"&lt;");
		if(NULL != tempPtr){
			tempPtr[0] = ' ';
			tempPtr[1] = ' ';
			tempPtr[2] = ' ';
			tempPtr[3] = '<';
		}
	}while(NULL != tempPtr);

	tempPtr = aXmlString;
	do{
		tempPtr = os_strcasestr(tempPtr,"&gt;");
		if(NULL != tempPtr){
			tempPtr[0] = '>';
			tempPtr[1] = ' ';
			tempPtr[2] = ' ';
			tempPtr[3] = ' ';
		}
	}while(NULL != tempPtr);

	tempPtr = aXmlString;
	do{
		tempPtr = os_strcasestr(tempPtr,"&quot;");
		if(NULL != tempPtr){
			tempPtr[0] = ' ';
			tempPtr[1] = ' ';
			tempPtr[2] = ' ';
			tempPtr[3] = ' ';
			tempPtr[4] = ' ';
			tempPtr[5] = '"';
		}
	}while(NULL != tempPtr);
}

int8*
cXmlParser::ExtractXmlVersion(IN int8 *aXmlString)
{
	if(NULL == aXmlString){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractXmlVersion Incomming XML string is NULL\n"); fflush(cvFilePtr);}
		return NULL;
	}
	int8 *xmlPtr = aXmlString, *tempPtr = NULL;
	xmlPtr = RemoveStartingSpaces(xmlPtr);
	/* Check for the start of the Xml Verstion String. Generally it will be of the form "<?xml version="1.0"?>" */
	if(*xmlPtr != '<' || *(xmlPtr+1) != '?'){
		/* Even if the xmlVersion string is not present don't treat it as the error case. Just ignore. */
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractXmlVersion doen't contain the XML Version String\n");fflush(cvFilePtr);}
		return xmlPtr;
	}
	/* Reached here menas we got the start of the xml string. so just parse and store that value. */
	xmlPtr += 2; /* Excluding "<?". */
	tempPtr = os_strcasestr(xmlPtr,"?>");
	if(NULL != tempPtr){
		int32 xmlVerStrLen = (tempPtr - xmlPtr) + 1;
		if(NULL == cvXMLVersionStr){
			cvXMLVersionStr = (int8*)os_malloc(xmlVerStrLen);
			if(NULL != cvXMLVersionStr){
				tempPtr = cvXMLVersionStr;
				while((xmlVerStrLen - 1) != 0){
					*tempPtr++ = *xmlPtr++;
					xmlVerStrLen--;
				}
			}
		}
		xmlPtr += 2; /* Skipping "?>" which will be end of XML Version String. */
	}else{
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractXmlVersion Unable to find the end of the XML String\n");fflush(cvFilePtr);}
		return NULL;
	}
	return xmlPtr;
}

int8*
cXmlParser::GetTagNameStartPosition(IN int8 *aXmlString)
{
	//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTagNameStartPosition IN aXmlString = %s\n",aXmlString);fflush(cvFilePtr);}
	if(NULL == aXmlString){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTagNameStartPosition Incomming XML string is NULL\n");fflush(cvFilePtr);}
		return NULL;
	}
	int8 *tagStart = aXmlString,*temp = aXmlString, *strTemp = NULL;
	tagStart = RemoveStartingSpaces(tagStart);
	while(tagStart != NULL && *tagStart != '\0'){
		tagStart = os_strchr(tagStart,'<');
		if(temp != NULL){
			temp = os_strstr(temp,"&lt;"); // Tag might start with "&lt;" also as per UPnP Standards....
			if(temp != NULL){
				if(temp < tagStart){
					tagStart = temp+3;
				}
			}
		}
		if(0 == os_strncmp(tagStart,"<!--",4)){
			strTemp = os_strstr(tagStart,"-->");
			if(NULL != strTemp){
				strTemp += 3;
			}
			tagStart = strTemp;
		}else if(0 == os_strncasecmp(tagStart,"<![CDATA[",9)){
			strTemp = os_strstr(tagStart,"]]>");
			if(NULL != strTemp){
				strTemp += 3;
			}
			tagStart = strTemp;
		}else if('?' == *(tagStart+1)){
			tagStart++;
		}else{
			/* We came to the start of the Tag excluding '<' */
			//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTagNameStartPosition returning TagNameStart Position = %s\n",tagStart+1);fflush(cvFilePtr);}
			return tagStart+1;
		}
	}
	/* Should not reach here at any point. If reached then something is wrong. */
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTagNameStartPosition Reached to place where to not reach. Check once\n");fflush(cvFilePtr);}
	return NULL;
}

int8*
cXmlParser::GetTagNameEndPosition(IN int8 *aXmlString)
{
	//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTagNameEndPosition IN aXmlString = %s\n",aXmlString);fflush(cvFilePtr);}
	if(NULL == aXmlString){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTagNameEndPosition Incomming XML string is NULL\n");fflush(cvFilePtr);}
		return NULL;
	}
	int8 *tagStrEnd = NULL,*strComment = NULL,*strComment1 = NULL, *temp = NULL;

	tagStrEnd = temp = aXmlString;
		      tagStrEnd = RemoveStartingSpaces(tagStrEnd);
	strComment1 = strComment = tagStrEnd;
	while(*tagStrEnd != '\0'){
		tagStrEnd = os_strchr(tagStrEnd,'>');
		temp = os_strcasestr(temp,"&gt;"); // Tag might start with "&lt;" also as per UPnP Standards....
		if(temp != NULL){
			if(temp < tagStrEnd){
				tagStrEnd = temp;
			}
		}
		if(0 == os_strncmp(tagStrEnd-2,"-->",3)){
			tagStrEnd++;
		}else if(0 == os_strncmp(tagStrEnd-2,"]]>",3)){
			tagStrEnd++;
		}else{
			strComment = os_strstr(strComment,"<!--");
			strComment1 = os_strcasestr(strComment1,"<![CDATA[");
			if((NULL == strComment && NULL == strComment1) ||
			   (NULL != strComment && NULL == strComment1 && strComment > tagStrEnd) ||
			   (NULL != strComment1 && NULL == strComment && strComment1 > tagStrEnd) ||
			   (NULL != strComment1 && NULL != strComment && strComment1 > tagStrEnd && strComment1 > tagStrEnd)){
				/* Comming here means we got the end of the tag location. Hence break the loop here itself */
				//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTagNameEndPosition Function returning TagNameEndPosition = %s\n",tagStrEnd);fflush(cvFilePtr);}
				return tagStrEnd;
			}else if(NULL != strComment && NULL == strComment1){
				strComment = os_strstr(strComment,"-->");
				if(NULL != strComment){
					strComment += 3;
				}
				strComment1 = tagStrEnd = strComment;
			}else if(NULL != strComment1 && NULL == strComment){
				strComment1 = os_strstr(strComment1,"-->");
				if(NULL != strComment1){
					strComment1 += 3;
				}
				strComment = tagStrEnd = strComment1;
			}else if(NULL != strComment1 && NULL != strComment){
				if(strComment1 < strComment){
					strComment1 = os_strstr(strComment1,"]]>");
					if(NULL != strComment1){
						strComment1 += 3;
					}
					strComment = tagStrEnd = strComment1;
				}else{
					strComment = os_strstr(strComment,"-->");
					if(NULL != strComment){
						strComment += 3;
					}
					strComment1 = tagStrEnd = strComment;
				}
			}else{
				/* we should not reach here at any point. Reached here means something is wrong. */
				if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTagNameEndPosition Reached to place where to not reach. Check once\n");fflush(cvFilePtr);}
				return NULL;
			}
		}
	}
	/* Should not reach here at any point. Reached here means something is wrong. */
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTagNameEndPosition Reached to place1 where to not reach. Check once\n");fflush(cvFilePtr);}
	return NULL;
}

int8*
cXmlParser::GetCharPos(IN int8 *aXmlString, int8 aMatchingChar)
{
	if(NULL == aXmlString){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetCharPos Incomming XML string is NULL");fflush(cvFilePtr);}
		return NULL;
	}
	int8 *charPos = NULL,*strComment = NULL,*strComment1 = NULL;

	charPos = aXmlString;
	charPos = RemoveStartingSpaces(charPos);
	strComment1 = strComment = charPos;
	while(*charPos != '\0'){
		charPos = os_strchr(charPos,aMatchingChar);
		strComment = os_strstr(strComment,"<!--");
		strComment1 = os_strcasestr(strComment1,"<![CDATA[");
		if((NULL == strComment && NULL == strComment1) ||
		    (NULL != strComment && NULL == strComment1 && strComment > charPos) ||
		    (NULL != strComment1 && NULL == strComment && strComment1 > charPos) ||
		    (NULL != strComment1 && NULL != strComment && strComment1 > charPos && strComment1 > charPos)){
			/* Comming here means we got the end of the tag location. Hence break the loop here itself */
			//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetCharPos Function returning as expeted");fflush(cvFilePtr);}
			return charPos;
		}else if(NULL != strComment && NULL == strComment1){
			strComment = os_strstr(strComment,"-->");
			if(NULL != strComment){
				strComment += 3;
			}
			strComment1 = charPos = strComment;
		}else if(NULL != strComment1 && NULL == strComment){
			strComment1 = os_strstr(strComment1,"-->");
			if(NULL != strComment1){
				strComment1 += 3;
			}
			strComment = charPos = strComment1;
		}else if(NULL != strComment1 && NULL != strComment){
			if(strComment1 < strComment){
				strComment1 = os_strstr(strComment1,"]]>");
				if(NULL != strComment1){
					strComment1 += 3;
				}
				strComment = charPos = strComment1;
			}else{
				strComment = os_strstr(strComment,"-->");
				if(NULL != strComment){
					strComment += 3;
				}
				strComment1 = charPos = strComment;
			}
		}else{
			/* we should not reach here at any point. Reached here means something is wrong. */
			if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetCharPos Reached to place where to not reach. Check once\n");fflush(cvFilePtr);}
			return NULL;
		}
	}
	/* Should not reach here at any point. Reached here means something is wrong. */
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetCharPos Reached to place1 where to not reach. Check once\n");fflush(cvFilePtr);}
	return NULL;
}

int8*
cXmlParser::GetAttributePos(IN int8 *aXmlString)
{
	if(NULL == aXmlString){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributePos Incomming XML string is NULL\n");fflush(cvFilePtr);}
		return NULL;
	}
	int8* spacePos = aXmlString;
	spacePos = RemoveStartingSpaces(spacePos);
	if(NULL != spacePos){
		if('>' == *spacePos || '?' == *spacePos || ('/' == *spacePos && '>' == *(spacePos+1))){
			//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributePos No Attribute. Hence returning NULL \n");
			return NULL;
		}else{
			//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributePos Attribute found as %s\n",spacePos);
			return spacePos;
		}
 	}
 	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributePos Reached to place where to not reach. Check once\n");fflush(cvFilePtr);}
	return NULL;
}

int8*
cXmlParser::FillTagName(IN int8 *aXmlString,OUT XmlParserTag &aTag)
{
	if(NULL == aXmlString){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::FillTagName Incomming XML string is NULL\n");fflush(cvFilePtr);}
		return NULL;
	}
	int32 tagNameLen = 0;
	int8 *tempPtr = NULL,*tagNameEnd = aXmlString, *attribStart = NULL,*tagNameEnd1 = NULL;

	tagNameEnd = RemoveStartingSpaces(tagNameEnd);

	attribStart = GetCharPos(tagNameEnd, ' '); /* First Get the Attribute start position. After that check whether its value is less than Tag end position or not. */
	/* Check for ' ' incase Attributes present otherwise check for '>' for tagname end. */
	tagNameEnd1 = os_strstr(tagNameEnd, "/>"); /* This check to handle the situation where the tag will be ended by "/>" */
	tagNameEnd = GetCharPos(tagNameEnd, '>');
	if(NULL != tagNameEnd1 && tagNameEnd1 < tagNameEnd){
		tagNameEnd = tagNameEnd1;
	}

	if(NULL != attribStart && attribStart < tagNameEnd){
		tagNameEnd = attribStart;
	}

	/* Get the length of the Tag Name and allocate the memory accordingly. */
	tagNameLen = (tagNameEnd - aXmlString) + 1;
	if(NULL == aTag.TagName){
		aTag.TagName = (int8*)os_malloc(tagNameLen);
		if(NULL == aTag.TagName){
			if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::FillTagName Failed to allocate memroy for aTag.TagName\n");fflush(cvFilePtr);}
			return NULL;
		}
		/* Clear the allocated memory first. */
		os_memset(aTag.TagName,'\0',tagNameLen);
		tempPtr = aTag.TagName;
		tagNameEnd = aXmlString;
		while((tagNameLen - 1) != 0){
			*tempPtr++ = *tagNameEnd++;
			tagNameLen--;
		}
		/* Remove the ending spaces which we may got because of replacing &lt; with < and &gt; with > */
		while(' ' == *--tempPtr){
			*tempPtr = '\0';
		}

		//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::FillTagName Filled the Tag Name: [%s]",aTag.TagName);
	}else{
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::FillTagName Already aTag.TagName is pointing to some memory\n");fflush(cvFilePtr);}
		return NULL;
	}
	//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::FillTagName returning TagNameEnd as %s\n",tagNameEnd);
	return tagNameEnd;
}

int8*
cXmlParser::RemoveStartingSpaces(IN int8 *aXmlString)
{
	if(NULL == aXmlString){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::RemoveStartingSpaces Incomming XML string is NULL");fflush(cvFilePtr);}
		return NULL;
	}
	int8 *outputPtr = aXmlString;
	while(' ' == *outputPtr || '\t' == *outputPtr || '\n' == *outputPtr || '\r' == *outputPtr){
		outputPtr++;
	}
	return outputPtr;
}

int8*
cXmlParser::ExtractAttributes(IN int8 *aXmlString,OUT XmlParserTag &aTag)
{
	//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractAttributes IN aTag = %s\n",aTag.TagName);fflush(cvFilePtr);}
	//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractAttributes IN aTag = %s and \n aXmlString = %s\n",aTag.TagName,aXmlString);fflush(cvFilePtr);}
	if(NULL == aXmlString){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractAttributes Incomming XML string is NULL");fflush(cvFilePtr);}
		return NULL;
	}
	int8 *attribName = aXmlString, *attribVal = NULL,*tempPtr = NULL;
	int32 len = 0;
	while('\0' != *attribName){
		/* First remove all the starting spaces. */
		XmlParserAttribute attrb;
		os_memset(&attrb,0,sizeof(attrb));
		attribName = RemoveStartingSpaces(attribName);
		/* Check wheter any attribute exists or not by checking for the Tag end character. */
		if('>' == *attribName || '?' == *attribName || ('/' == *attribName && '>' == *(attribName+1))){
			//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractAttributes Function returning as expeted. Came to the end of the Tag");fflush(cvFilePtr);}
			return attribName; /* Came to the end of Tag character. Hence return from here it self. */
		}
		attribVal = GetCharPos(attribName,'=');
		if(NULL != attribVal){
			/* IF we get '=' then we can copy the attribute name first. */
			len = (attribVal - attribName) +1;
			attrb.AttribName = (int8*)os_malloc(len);
			if(NULL == attrb.AttribName){
				if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractAttributes Failed to allocate memroy for aTag.AttribName\n");fflush(cvFilePtr);}
				return NULL;
			}
			os_memset(attrb.AttribName,'\0',len);
			tempPtr = attrb.AttribName;
			while((len-1) != 0){
				if(' ' != *attribName){ /* Do not copy the white spaces in case of attrib name. */
					*tempPtr++ = *attribName++;
				}
				len--;
			}

			/* Remove the ending spaces which we may got because of replacing &lt; with < and &gt; with > */
			while(' ' == *--tempPtr){
				*tempPtr = '\0';
			}

			/* Now start processing the Attribute Value. */
			attribVal++; /* need to skip '=' */
			attribVal = RemoveStartingSpaces(attribVal);
			/* If the attribute value starts with '"' then we need to copy the string until we get ending '"'. */
			if('"' == *attribVal){
				attribVal++;
				tempPtr = os_strchr(attribVal,'"');
			}else{
				tempPtr = GetCharPos(attribName,' ');
			}
			if(NULL != tempPtr){
				len = (tempPtr - attribVal) +1;
				attrb.AttribValue = (int8*)os_malloc(len);
				if(NULL == attrb.AttribValue){
					if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractAttributes Failed to allocate memroy for aTag.AttribValue\n");fflush(cvFilePtr);}
					return NULL;
				}
				os_memset(attrb.AttribValue,0,len);
				tempPtr = attrb.AttribValue;
				while((len-1) != 0){
					*tempPtr++ = *attribVal++;
					len--;
				}

				/* Remove the ending spaces which we may got because of replacing &lt; with < and &gt; with > */
				while(' ' == *--tempPtr){
					*tempPtr = '\0';
				}
				/* Create the Attribute list for the first time if it doesn't exist. */
				if(NULL == aTag.AttribList){
					aTag.AttribList = os_new(cLinkedList<XmlParserAttribute>,());
					if(NULL == aTag.AttribList){
						if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractAttributes Failed to create aTag.AttribList\n");fflush(cvFilePtr);}
						return NULL;
					}
				}
				/* Add the attribute the the attribute list of this tag. */
				if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractAttributes Adding the Attribute [%s] with Value [%s]\n",attrb.AttribName,attrb.AttribValue);fflush(cvFilePtr);}
				aTag.AttribList->Add(attrb);
				/* Re-initialize the Variables. to continue the while loop correctly. */
				attribName = attribVal+1;
			}else{
				/* If we didn't get the Tag Value End Character then no need of this Tag itself. Delete the memory allocate for it. */
				if(attrb.AttribName){
					os_free(attrb.AttribName);
					//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractAttributes Unable to find the End of Attribute\n");fflush(cvFilePtr);}
					attrb.AttribName = NULL;
				}
				//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractAttributes returning Attribute name = %s\n",attribName);fflush(cvFilePtr);}
				return attribName;
			}
		}else{
			/* Ideally we should not reach here. Came here means something wrong might have happened. Even then return until where we parsed to be more robust. */
			if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractAttributes Should not reache here. Reached means please check once\n");fflush(cvFilePtr);}
			return attribName;
		}
	}
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractAttributes reached to the end of xml string. We don't have any thing much to parse\n");fflush(cvFilePtr);}
	return NULL;
}

int8*
cXmlParser::ExtractTagNameAndAttributes(IN int8 *aXmlString,OUT XmlParserTag &aTag)
{
	if(NULL == aXmlString){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractTagNameAndAttributes Incomming XML string is NULL\n");fflush(cvFilePtr);}
		return NULL;
	}
	int8 *tagNameStartPos = NULL, *tagNameEndPos = NULL, *attribPos = NULL;

	tagNameStartPos = GetTagNameStartPosition(aXmlString);
	if(NULL == tagNameStartPos){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractTagNameAndAttributes Unable to find the Tag Name start position\n");fflush(cvFilePtr);}
		return NULL;
	}

	tagNameEndPos = FillTagName(tagNameStartPos,aTag);
	if(NULL == tagNameEndPos){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractTagNameAndAttributes Unable to fill the Tag Name\n");fflush(cvFilePtr);}
		return NULL;
	}
	/* Check whether any attributes are present or not. If present then extract them accordingly. */
	attribPos = GetAttributePos(tagNameEndPos);
	if(NULL != attribPos){
		tagNameEndPos = ExtractAttributes(tagNameEndPos,aTag);
	}
	return tagNameEndPos;
}

bool
cXmlParser::CheckForTagCompleteWithOutTagEnd(IN int8 *aXmlString)
{
	/* Here don't have much things to do with the tag end value. Just check wheter tag end is present of not. If present then just skip and return the pointer
	  correctly. */
	int8 *tagNameEndPos = RemoveStartingSpaces(aXmlString);
	if('/' == *tagNameEndPos && '>' == *(tagNameEndPos+1)){
		//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::CheckForTagCompleteWithOutTagEnd found TagComplete. Hence returning TRUE\n");fflush(cvFilePtr);}
		return true;
	}
	//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::CheckForTagCompleteWithOutTagEnd Not Found. Hence returning FALSE\n");fflush(cvFilePtr);}
	return false;
}

bool
cXmlParser::CheckForTagEnd(IN int8 *aXmlString,OUT XmlParserTag &aTag)
{
	if(NULL == aXmlString){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::CheckForTagEnd Incomming XML string is NULL\n");fflush(cvFilePtr);}
		return NULL;
	}

	int8 *tagEndPtr = aXmlString;
	tagEndPtr = RemoveStartingSpaces(tagEndPtr);
	if(0 == os_strncmp(tagEndPtr,"</",2)){
		if(0 == os_strncmp(aTag.TagName,tagEndPtr+2,os_strlen(aTag.TagName))){
			//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::CheckForTagEnd found Tag End. Hence returning TRUE\n");fflush(cvFilePtr);}
			return true;
		}
	}
	//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::CheckForTagEnd Tag End not found. Hence returning FALSE\n");fflush(cvFilePtr);}
	return false;
}

int8*
cXmlParser::ExtractTagEnd(IN int8 *aXmlString,OUT XmlParserTag &aTag)
{
	if(NULL == aXmlString){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractTagEnd Incomming XML string is NULL\n");fflush(cvFilePtr);}
		return NULL;
	}

	int8 *tagEndPtr = aXmlString;
	tagEndPtr = RemoveStartingSpaces(tagEndPtr);
	if(0 == os_strncmp(tagEndPtr,"</",2)){
		if(0 == os_strncmp(aTag.TagName,tagEndPtr+2,os_strlen(aTag.TagName))){
			tagEndPtr += os_strlen(aTag.TagName) + 2;
			tagEndPtr = RemoveStartingSpaces(tagEndPtr);
			if('>' == *tagEndPtr){
				tagEndPtr++;
				//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractTagEnd found Tag End = %s\n",tagEndPtr);fflush(cvFilePtr);}
				return tagEndPtr;
			}
		}
	}
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractTagEnd Tag End Not Found. Hence returning NULL");fflush(cvFilePtr);}
	return NULL;
}

bool
cXmlParser::CheckForTagValue(IN int8 *aXmlString)
{
	if(NULL == aXmlString){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::CheckForTagValue Incomming XML string is NULL\n");fflush(cvFilePtr);}
		return NULL;
	}
	int8 *tagValPtr = aXmlString;
	tagValPtr = RemoveStartingSpaces(tagValPtr);
	if('<' == *tagValPtr){
		//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::CheckForTagValue Tag Value Not Found. Hence returning FALSE\n");fflush(cvFilePtr);}
		return false;
	}
	//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::CheckForTagValue Tag Found. Hence returning TRUE\n");fflush(cvFilePtr);}
	return true;
}

int8*
cXmlParser::ExtractTagValue(IN int8 *aXmlString,OUT XmlParserTag &aTag)
{
	if(NULL == aXmlString){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractTagValue Incomming XML string is NULL\n");fflush(cvFilePtr);}
		return NULL;
	}
	int8 *tagValPtr = aXmlString,*tempPtr = NULL;
	int32 len = 0;

	tagValPtr = RemoveStartingSpaces(tagValPtr);
	if('<' != *tagValPtr){
		tempPtr = GetCharPos(tagValPtr,'<');
		if(NULL != tempPtr){
			/* Came here means we got the tag value end. So just allocate the memory and copy the value to the incomming Tag structure. */
			len = (tempPtr - tagValPtr) +1;
			aTag.TagValue = (int8*)os_malloc(len);
			if(NULL == aTag.TagValue){
				if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractTagValue Unable to allocate the memory for aTag.TagValue\n");fflush(cvFilePtr);}
				return NULL;
			}
			os_memset(aTag.TagValue,'\0',len);
			tempPtr = aTag.TagValue;
			while((len-1) != 0){
				*tempPtr++ = *tagValPtr++;
				len--;
			}

			/* Remove the ending spaces which we may got because of replacing &lt; with < and &gt; with > */
			while(' ' == *--tempPtr){
				*tempPtr = '\0';
			}

			//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractTagValue Extracted the Tag Value [%s]\n",aTag.TagValue);fflush(cvFilePtr);}
			return tagValPtr;
		}
	}
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ExtractTagValue Unable find TagValue\n");fflush(cvFilePtr);}
	return NULL;
}

bool
cXmlParser::CheckForNewTab(IN int8 *aXmlString)
{
	if(NULL == aXmlString){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::CheckForNewTab Incomming XML string is NULL\n");fflush(cvFilePtr);}
		return NULL;
	}
	int8 *subTagPtr = aXmlString;

	subTagPtr = RemoveStartingSpaces(subTagPtr);
	if('<' == *subTagPtr){
		if('/' != *(subTagPtr+1)){
			//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::CheckForNewTab found new tab\n");fflush(cvFilePtr);}
			return true;
		}
	}
	//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::CheckForNewTab new tab not found\n");fflush(cvFilePtr);}
	return false;
}

ReturnStatus
cXmlParser::SearchForRepeatedTag(IN int8 *aTagName, XmlParserTag &aXmlTag, XmlParserTag &aSearchResultTab, int32 count)
{
	int32 lcount = 0;
	//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::SearchForTag IN Searching for Tag [%s] inside root Object [%s]\n",aTagName,aXmlTag.TagName);fflush(cvFilePtr);}
	if(aXmlTag.ChildTagList){
		int32 childTagSize = aXmlTag.ChildTagList->GetNumberOfElements();
		for(int32 i=1; i<=childTagSize; i++){
			XmlParserTag childTag;
			os_memset(&childTag,0,sizeof(childTag));
			if(SUCCESS == aXmlTag.ChildTagList->GetElementAtIndex(childTag,i)){
				if(0 == os_strncasecmp(childTag.TagName,aTagName,os_strlen(aTagName))){
					lcount++;
					if(lcount != count){
						continue;
					}
					os_memcpy(&aSearchResultTab,&childTag,sizeof(aSearchResultTab));
					return SUCCESS;
				}
			}else{
				return FAILURE;
			}
		}
	}else{
		//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::SearchForTag Search Tag Not Found\n");fflush(cvFilePtr);}
		return FAILURE;
	}
	/* Should not reach here. Reached means something is wrong.. Find and break the puzzele... */
	//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::SearchForTag Reached to place where to not reach. Check once\n");fflush(cvFilePtr);}
	return FAILURE;
}

ReturnStatus
cXmlParser::SearchForTag(IN int8 *aTagName, XmlParserTag &aXmlTag, XmlParserTag &aSearchResultTab, bool aParentTag)
{
	//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::SearchForTag IN Searching for Tag [%s] inside root Object [%s]\n",aTagName,aXmlTag.TagName);fflush(cvFilePtr);}
	if(0 == os_strcasecmp(aXmlTag.TagName,aTagName)){
		if(aParentTag){
			return FAILURE;
		}else{
			os_memcpy(&aSearchResultTab,&aXmlTag,sizeof(aSearchResultTab));
			return SUCCESS;
		}
	}else{
		if(aXmlTag.ChildTagList){
			int32 childTagSize = aXmlTag.ChildTagList->GetNumberOfElements();
			for(int32 i=1; i<=childTagSize; i++){
				XmlParserTag childTag;
				os_memset(&childTag,0,sizeof(childTag));
				if(SUCCESS == aXmlTag.ChildTagList->GetElementAtIndex(childTag,i)){
					if(0 == os_strcasecmp(childTag.TagName,aTagName)){
						if(aParentTag){
							os_memcpy(&aSearchResultTab,&aXmlTag,sizeof(aSearchResultTab));
						}else{
							os_memcpy(&aSearchResultTab,&childTag,sizeof(aSearchResultTab));
						}
						return SUCCESS;
					}else{
						os_memset(&aSearchResultTab,0,sizeof(aSearchResultTab));
						if(SUCCESS == SearchForTag(aTagName,childTag,aSearchResultTab,aParentTag)){
							if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::SearchForTag found the Tag with Name [%s] and Value [%s]\n",aSearchResultTab.TagName,aSearchResultTab.TagValue);fflush(cvFilePtr);}
							return SUCCESS;
						}
					}
				}else{
					//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::SearchForTag Search Tag Not Found\n");fflush(cvFilePtr);}
					return FAILURE;
				}
			}
		}else{
			//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::SearchForTag Search Tag Not Found\n");fflush(cvFilePtr);}
			return FAILURE;
		}
	}
	/* Should not reach here. Reached means something is wrong.. Find and break the puzzele... */
	//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::SearchForTag Reached to place where to not reach. Check once\n");fflush(cvFilePtr);}
	return FAILURE;
}

int8*
cXmlParser::ParseXmlTag(IN int8 *aXmlString,OUT XmlParserTag &aTag)
{
	if(NULL == aXmlString){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ParseXmlTag input XML string is NULL\n");fflush(cvFilePtr);}
		return NULL;
	}

	int8 *offsetAddr = aXmlString;

	/* First Extract the Tag Name along with the attributes if present any */
	offsetAddr = ExtractTagNameAndAttributes(offsetAddr,aTag);
	if(NULL == offsetAddr){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ParseXmlTag ExtractTagNameAndAttributes failed\n");fflush(cvFilePtr);}
		return NULL;
	}
	/* After Extracting the Tag Name And Attributes check whether the Tag is completing with out Tag end by "/>" or not. If so
	  Handle it accordingly. */
	if(true == CheckForTagCompleteWithOutTagEnd(offsetAddr)){
		offsetAddr = RemoveStartingSpaces(offsetAddr);
		if('/' == *offsetAddr && '>' == *(offsetAddr+1)){
			//if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ParseXmlTag Found the Tag End\n");fflush(cvFilePtr);}
			return offsetAddr+2;
		}
	}else{
		offsetAddr = RemoveStartingSpaces(offsetAddr);
		if('>' == *offsetAddr){
			offsetAddr++;
		}
	}

	/* Check for the Tag End. If not then there might be sub tags handle them accordingly. */
	if(false == CheckForTagEnd(offsetAddr,aTag)){
		/* First check wheter any Tab value is present or not. If present then extract it. */
		if(true == CheckForTagValue(offsetAddr)){
			offsetAddr = ExtractTagValue(offsetAddr,aTag);
			if(NULL == offsetAddr){
				if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ParseXmlTag ExtractTagValue failed\n");fflush(cvFilePtr);}
				return NULL;
			}
		}
		/* Now Check here wheter there any sub tag or not. If present then handle it recursively. */
		else if(true == CheckForNewTab(offsetAddr)){
			if(NULL == aTag.ChildTagList){
				aTag.ChildTagList = os_new(cLinkedList<XmlParserTag>,());
				if(NULL == aTag.ChildTagList){
					if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ParseXmlTag failed to allocate the memory for aTag.ChildTagList\n");fflush(cvFilePtr);}
					return NULL;
				}
			}
			while(CheckForNewTab(offsetAddr)){
				XmlParserTag subTag;
	// 			/* Initialize the pointers with NULL. Otherwise while deleting it may create disturbance.. */
				os_memset(&subTag,0,sizeof(subTag));
				/* Call this function recursively to extract the sub tags until we reach the end. */
				offsetAddr = ParseXmlTag(offsetAddr,subTag);
				if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ParseXmlTag Adding Child tag [%s] with value [%s] to Parent Tab [%s]\n",subTag.TagName,subTag.TagValue,aTag.TagName);fflush(cvFilePtr);}
				aTag.ChildTagList->Add(subTag);
			}
		}else{
			if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ParseXmlTag No tag value or tag end is found. Need to check once again\n");fflush(cvFilePtr);}
			return NULL;
		}
	}
	/* Here we have nothing to do with the Tag end. Just cross check and leave it.*/
	offsetAddr = ExtractTagEnd(offsetAddr,aTag);
	if(NULL == offsetAddr){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ParseXmlTag ExtractTagEnd failed\n");fflush(cvFilePtr);}
		return NULL;
	}
	return offsetAddr;
}

ReturnStatus
cXmlParser::ParseXml(IN int8 *aXmlString,IN int32 logCount)
{
	int8 fileName[64] = {'\0'};
#ifdef ANDROID
	os_snprintf(fileName,sizeof(fileName),"/sdcard/.EminentMediaPLayer/xml%d.log",logCount);
#else
	os_snprintf(fileName,sizeof(fileName),"xml%d.log",logCount);
#endif
	if(cvFilePtr){
		fclose(cvFilePtr);
		cvFilePtr = NULL;
	}
	//LOG_CONSOLE("cXmlParser::ParseXml Opening the xml log file [%s]",fileName);
	//cvFilePtr = fopen(fileName,"w+");
	if(NULL == aXmlString){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ParseXml input XML string is NULL\n");fflush(cvFilePtr);}
		return FAILURE;
	}
	if(cvFilePtr){fprintf(cvFilePtr,"cXmlParser::ParseXml aXmlString = %s\n",aXmlString);fflush(cvFilePtr);}
	int8 *xmlStringPtr = aXmlString;

	/* First scan the xmlstring.. */
	ScanXmlString(xmlStringPtr);
	/* Extract the XML Verstion tag and store in class variable. */
	xmlStringPtr = ExtractXmlVersion(aXmlString);
	if(NULL == xmlStringPtr){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ParseXml ExtractXmlVersion Failed\n");fflush(cvFilePtr);}
		return FAILURE;
	}
	/* Now start processing the Xml Tags recursively starting from the top most root tag. */
	if(NULL == ParseXmlTag(xmlStringPtr,cvXmlBaseTag)){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ParseXml ParseXmlTag Failed\n");fflush(cvFilePtr);}
		return FAILURE;
	}
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::ParseXml aXmlString returning SUCCESS\n");fflush(cvFilePtr);}
	/* Completed the parsing. Hence return success.. */
	return SUCCESS;
}

bool
cXmlParser::IsTagPresent(IN int8 *aTagName)
{
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::IsTagPresent aTagName = %s\n",aTagName);fflush(cvFilePtr);}
	if(NULL == aTagName){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::IsTagPresent input tagName is NULL\n");fflush(cvFilePtr);}
		return false;
	}
	XmlParserTag searchTag;
	if(SUCCESS != SearchForTag(aTagName,cvXmlBaseTag,searchTag)){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::IsTagPresent No Tag found with this [%s] Name\n",aTagName);fflush(cvFilePtr);}
		return false;
	}else{
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::IsTagPresent Found the Tag found with this [%s] Name\n",aTagName);fflush(cvFilePtr);}
		return true;
	}
}

bool
cXmlParser::IsTagPresent(IN int8 *aTagName, IN XmlParserTag aBaseTag)
{
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::IsTagPresent In tagName = %s\n",aTagName);fflush(cvFilePtr);}
	if(NULL == aTagName){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::IsTagPresent input tagName is NULL\n");fflush(cvFilePtr);}
		return false;
	}
	XmlParserTag searchTag;
	if(SUCCESS != SearchForTag(aTagName,aBaseTag,searchTag)){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::IsTagPresent No Tag found with this [%s] Name\n",aTagName);fflush(cvFilePtr);}
		return false;
	}else{
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::IsTagPresent Found the Tag found with this [%s] Name\n",aTagName);fflush(cvFilePtr);}
		return true;
	}
}

ReturnStatus
cXmlParser::GetTagValue(IN int8 *aTagName, OUT int8 *aTagValue)
{
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTagValue IN aTagName = %s\n",aTagName);fflush(cvFilePtr);}
	if(NULL == aTagName){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTagValue input tagName is NULL\n");fflush(cvFilePtr);}
		return FAILURE;
	}
	XmlParserTag searchTag;
	if(SUCCESS != SearchForTag(aTagName,cvXmlBaseTag,searchTag)){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTagValue No Tag found with the Name [%s]\n",aTagName);fflush(cvFilePtr);}
		return FAILURE;
	}else{
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTagValue Found the Tag found with the Name [%s] and Value [%s]\n",aTagName,searchTag.TagValue);fflush(cvFilePtr);}
		os_strncpy(aTagValue,searchTag.TagValue,os_strlen(searchTag.TagValue));
		return SUCCESS;
	}
	/* Should not reach here. Reached means something went wrong. Find the culprit. */
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTagValue reached the unexpected portion. Check once more\n");fflush(cvFilePtr);}
	return FAILURE;
}

ReturnStatus
cXmlParser::GetTagValue(IN XmlParserTag aInTag,OUT int8 *aTagValue)
{
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTagValue IN \n");fflush(cvFilePtr);}

	if(os_strlen(aInTag.TagValue) > 0){
		os_strncpy(aTagValue,aInTag.TagValue,os_strlen(aInTag.TagValue));
		return SUCCESS;
	}else{
		return FAILURE;
	}
	/* Should not reach here. Reached means something went wrong. Find the culprit. */
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTagValue reached the unexpected portion. Check once more\n");fflush(cvFilePtr);}
	return FAILURE;
}

ReturnStatus
cXmlParser::GetTagValue(IN int8 *aTagName, IN XmlParserTag aBaseTag,OUT int8 *aTagValue)
{
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTagValue IN tagName = %s\n",aTagName);fflush(cvFilePtr);}
	if(NULL == aTagName){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTagValue input tagName is NULL\n");fflush(cvFilePtr);}
		return FAILURE;
	}
	XmlParserTag searchTag;
	if(SUCCESS != SearchForTag(aTagName,aBaseTag,searchTag)){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTagValue No Tag found with this [%s] Name\n",aTagName);fflush(cvFilePtr);}
		return FAILURE;
	}else{
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTagValue Found the Tag found with the Name [%s] and Value [%s]\n",aTagName,searchTag.TagValue);fflush(cvFilePtr);}
		os_strncpy(aTagValue,searchTag.TagValue,os_strlen(searchTag.TagValue));
		return SUCCESS;
	}
	/* Should not reach here. Reached means something went wrong. Find the culprit. */
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTagValue reached the unexpected portion. Check once more\n");fflush(cvFilePtr);}
	return FAILURE;
}

ReturnStatus
cXmlParser::GetTab(IN int8 *aTagName, OUT XmlParserTag &aParserTag)
{
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTab IN tagName = %s\n",aTagName);fflush(cvFilePtr);}
	if(NULL == aTagName){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetTab input tagName is NULL\n");fflush(cvFilePtr);}
		return FAILURE;
	}
	return SearchForTag(aTagName,cvXmlBaseTag,aParserTag);
}

ReturnStatus
cXmlParser::GetParentTab(IN int8 *aTagName, IN XmlParserTag &aInTag,OUT XmlParserTag &aParserTag)
{
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetParentTab IN tagName = %s\n",aTagName);fflush(cvFilePtr);}
	if(NULL == aTagName){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetParentTab input tagName is NULL\n");fflush(cvFilePtr);}
		return FAILURE;
	}
	return SearchForTag(aTagName,aInTag,aParserTag,true);
}

ReturnStatus
cXmlParser::GetRepeatedTagAtIndex(IN int8 *aTagName, IN XmlParserTag aInTag, OUT XmlParserTag &aOutTag, int32 count)
{
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetRepeatedTagAtIndex IN tagName = %s\n",aTagName);fflush(cvFilePtr);}
	if(NULL == aTagName){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetRepeatedTagAtIndex input tagName is NULL\n");fflush(cvFilePtr);}
		return FAILURE;
	}
	return SearchForRepeatedTag(aTagName,aInTag,aOutTag,count);
}

int32
cXmlParser::GetNumberOfSubTabs(IN int8 *aTagName)
{	
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfSubTabs IN tagName = %s\n",aTagName);fflush(cvFilePtr);}
	if(NULL == aTagName){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfSubTabs input tagName is NULL\n");fflush(cvFilePtr);}
		return FAILURE;
	}
	XmlParserTag searchTag;
	if(SUCCESS != SearchForTag(aTagName,cvXmlBaseTag,searchTag)){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfSubTabs Tag Not Found with this [%s] Tag Name\n",aTagName);fflush(cvFilePtr);}
		return FAILURE;
	}else{
		if(searchTag.ChildTagList){
			if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfSubTabs returning Number of sub Tabs - %d\n",searchTag.ChildTagList->GetNumberOfElements());fflush(cvFilePtr);}
			return searchTag.ChildTagList->GetNumberOfElements();
		}else{
			if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfSubTabs ChildTagList is NULL. Hence returning number of sub tabs as 0\n");fflush(cvFilePtr);}
			return 0;
		}
	}
	/* Should not reach here. Reached means something went wrong. Find the culprit. */
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfSubTabs reached the unexpected portion. Check once more\n");fflush(cvFilePtr);}
	return FAILURE;
}

ReturnStatus
cXmlParser::GetSubTagAtIndex(IN int8 *aTagName,XmlParserTag &aSubTag,IN int32 aIndex)
{
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetSubTagAtIndex IN tagName = %s\n",aTagName);fflush(cvFilePtr);}
	if(NULL == aTagName){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetSubTagAtIndex input tagName is NULL\n");fflush(cvFilePtr);}
		return FAILURE;
	}
	XmlParserTag searchTag;
	if(SUCCESS != SearchForTag(aTagName,cvXmlBaseTag,searchTag)){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetSubTagAtIndex Tag Not Found with this [%s] Tag Name\n",aTagName);fflush(cvFilePtr);}
		return FAILURE;
	}else{
		if(searchTag.ChildTagList){
			if(SUCCESS == searchTag.ChildTagList->GetElementAtIndex(aSubTag,aIndex)){
				if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetSubTagAtIndex got the element from ChildTagList. Hence returning TRUE\n");fflush(cvFilePtr);}
				return SUCCESS;
			}else{
				if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetSubTagAtIndex Failed to get element from ChildTagList. Hence returning FALSE\n");fflush(cvFilePtr);}
				return FAILURE;
			}
		}else{
			if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetSubTagAtIndex ChildTagList is NULL\n");fflush(cvFilePtr);}
			return FAILURE;
		}
	}
	/* Should not reach here. Reached means something went wrong. Find the culprit. */
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetSubTagAtIndex reached the unexpected portion. Check once more\n");fflush(cvFilePtr);}
	return FAILURE;
}

int32
cXmlParser::GetNumberOfAttributesOfTab(IN XmlParserTag aBaseTag)
{
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfAttributesOfTab \n");fflush(cvFilePtr);}

	if(aBaseTag.AttribList){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfAttributesOfTab returning number of attributes = %d\n",aBaseTag.AttribList->GetNumberOfElements());fflush(cvFilePtr);}
		return aBaseTag.AttribList->GetNumberOfElements();
	}else{
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfAttributesOfTab AttribList is NULL. Hence returning 0\n");fflush(cvFilePtr);}
		return 0;
	}

	/* Should not reach here. Reached means something went wrong. Find the culprit. */
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfAttributes reached the unexpected portion. Check once more\n");fflush(cvFilePtr);}
	return FAILURE;
}

int32
cXmlParser::GetNumberOfAttributes(IN int8 *aTagName, XmlParserTag aBaseTag)
{
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfAttributes IN aTagName = %s\n",aTagName);fflush(cvFilePtr);}
	if(NULL == aTagName){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfAttributes input tagName is NULL\n");fflush(cvFilePtr);}
		return FAILURE;
	}
	XmlParserTag searchTag;
	if(SUCCESS != SearchForTag(aTagName,aBaseTag,searchTag)){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfAttributes Tag Not Found with this [%s] Tag Name\n",aTagName);fflush(cvFilePtr);}
		return FAILURE;
	}else{
		if(searchTag.AttribList){
			if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfAttributes returning number of attributes = %d\n",searchTag.AttribList->GetNumberOfElements());fflush(cvFilePtr);}
			return searchTag.AttribList->GetNumberOfElements();
		}else{
			if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfAttributes AttribList is NULL. Hence returning 0\n");fflush(cvFilePtr);}
			return 0;
		}
	}
	/* Should not reach here. Reached means something went wrong. Find the culprit. */
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfAttributes reached the unexpected portion. Check once more\n");fflush(cvFilePtr);}
	return FAILURE;
}

int32
cXmlParser::GetNumberOfAttributes(IN int8 *aTagName)
{
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfAttributes IN tagName = %s\n",aTagName);fflush(cvFilePtr);}
	if(NULL == aTagName){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfAttributes input tagName is NULL\n");fflush(cvFilePtr);}
		return FAILURE;
	}
	XmlParserTag searchTag;
	if(SUCCESS != SearchForTag(aTagName,cvXmlBaseTag,searchTag)){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfAttributes Tag Not Found with this [%s] Tag Name\n",aTagName);fflush(cvFilePtr);}
		return FAILURE;
	}else{
		if(searchTag.AttribList){
			if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfAttributes returning number of attributes = %d\n",searchTag.AttribList->GetNumberOfElements());fflush(cvFilePtr);}
			return searchTag.AttribList->GetNumberOfElements();
		}else{
			if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfAttributes AttribList is NULL. Hence returning 0\n");fflush(cvFilePtr);}
			return 0;
		}
	}
	/* Should not reach here. Reached means something went wrong. Find the culprit. */
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetNumberOfAttributes reached the unexpected portion. Check once more\n");fflush(cvFilePtr);}
	return FAILURE;
}

ReturnStatus
cXmlParser::GetAttributeAtIndex(IN int8 *aTagName, IN XmlParserTag aBaseTag,OUT XmlParserAttribute &aAttrib, IN int32 aIndex)
{
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributeAtIndex IN tagName = %s\n",aTagName);fflush(cvFilePtr);}
	if(NULL == aTagName){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributeAtIndex input tagName is NULL\n");fflush(cvFilePtr);}
		return FAILURE;
	}
	XmlParserTag searchTag;
	if(SUCCESS != SearchForTag(aTagName,aBaseTag,searchTag)){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributeAtIndex Tag Not Found with this [%s] Tag Name\n",aTagName);fflush(cvFilePtr);}
		return FAILURE;
	}else{
		if(searchTag.AttribList){
			if(SUCCESS == searchTag.AttribList->GetElementAtIndex(aAttrib,aIndex)){
				if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributeAtIndex got the element from AttribList. Hence returning SUCCESS\n");fflush(cvFilePtr);}
				return SUCCESS;
			}else{
				if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributeAtIndex unable to get element from AttribList. Hence returning FAILURE\n");fflush(cvFilePtr);}
				return FAILURE;
			}
		}else{
			if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributeAtIndex AttribList is NULL\n");fflush(cvFilePtr);}
			return FAILURE;
		}
	}
	/* Should not reach here. Reached means something went wrong. Find the culprit. */
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributeAtIndex reached the unexpected portion. Check once more\n");fflush(cvFilePtr);}
	return FAILURE;
}

ReturnStatus
cXmlParser::GetAttributeOfTabAtIndex(IN XmlParserTag aBaseTag,OUT XmlParserAttribute &aAttrib, IN int32 aIndex)
{
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributeOfTabAtIndex IN\n");fflush(cvFilePtr);}
	if(aBaseTag.AttribList){
		if(SUCCESS == aBaseTag.AttribList->GetElementAtIndex(aAttrib,aIndex)){
			if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributeOfTabAtIndex got the element from AttribList. Hence returning SUCCESS\n");fflush(cvFilePtr);}
			return SUCCESS;
		}else{
			if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributeOfTabAtIndex unable to get element from AttribList. Hence returning FAILURE\n");fflush(cvFilePtr);}
			return FAILURE;
		}
	}else{
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributeOfTabAtIndex AttribList is NULL\n");fflush(cvFilePtr);}
		return FAILURE;
	}
	/* Should not reach here. Reached means something went wrong. Find the culprit. */
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributeOfTabAtIndex reached the unexpected portion. Check once more\n");fflush(cvFilePtr);}
	return FAILURE;
}

ReturnStatus
cXmlParser::GetAttributeAtIndex(IN int8 *aTagName, OUT XmlParserAttribute &aAttrib, IN int32 aIndex)
{
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributeAtIndex IN tagName = %s\n",aTagName);fflush(cvFilePtr);}
	if(NULL == aTagName){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributeAtIndex input tagName is NULL\n");fflush(cvFilePtr);}
		return FAILURE;
	}
	XmlParserTag searchTag;
	if(SUCCESS != SearchForTag(aTagName,cvXmlBaseTag,searchTag)){
		if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributeAtIndex Tag Not Found with this [%s] Tag Name\n",aTagName);fflush(cvFilePtr);}
		return FAILURE;
	}else{
		if(searchTag.AttribList){
			if(SUCCESS == searchTag.AttribList->GetElementAtIndex(aAttrib,aIndex)){
				if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributeAtIndex got the element from AttribList. Hence returning SUCCESS\n");fflush(cvFilePtr);}
				return SUCCESS;
			}else{
				if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributeAtIndex unable to get element from AttribList. Hence returning FAILURE\n");fflush(cvFilePtr);}
				return FAILURE;
			}
		}else{
			if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributeAtIndex AttribList is NULL\n");fflush(cvFilePtr);}
			return FAILURE;
		}
	}
	/* Should not reach here. Reached means something went wrong. Find the culprit. */
	if(cvFilePtr){ fprintf(cvFilePtr,"cXmlParser::GetAttributeAtIndex reached the unexpected portion. Check once more\n");fflush(cvFilePtr);}
	return FAILURE;
}

