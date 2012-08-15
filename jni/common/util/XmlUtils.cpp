#include "XmlUtils.h"

ReturnStatus XmlUtils::GetTagValue(IN TiXmlDocument &aXmlDoc, IN int8* aTagName, OUT int8* aTagValue){
	TiXmlNode* node = NULL;
	for(node = aXmlDoc.FirstChild(); node != NULL; node = node->NextSibling()){
		if(!node->ToElement()){
			continue;
		}
		if(os_strcasecmp(node->Value(),aTagName) == 0){
			return ExtractTagValue(node,aTagValue);
		}else{
			TiXmlNode* node1 = NULL;
			node1 = GetTag(node,aTagName);
			if(node1 != NULL){
				return ExtractTagValue(node1,aTagValue);
			}
		}
	}
	return FAILURE;
}

ReturnStatus XmlUtils::GetTagValue(IN TiXmlNode* aNode, IN int8* aTagName, OUT int8* aTagValue){
	TiXmlNode* node = NULL;
	if(NULL == aNode){
		return FAILURE;
	}
	for(node = aNode->FirstChild(); node != NULL; node = node->NextSibling()){
		if(!node->ToElement()){
			continue;
		}
		if(os_strcasecmp(node->Value(),aTagName) == 0){
			return ExtractTagValue(node,aTagValue);
		}else{
			TiXmlNode* node1 = NULL;
			node1 = GetTag(node,aTagName);
			if(node1 != NULL){
				return ExtractTagValue(node1,aTagValue);
			}
		}
	}
	return FAILURE;
}

int8* XmlUtils::GetTagValue(IN TiXmlNode* aNode, IN int8* aTagName){
	TiXmlNode* node = NULL;
	if(NULL == aNode){
		return NULL;
	}
	for(node = aNode->FirstChild(); node != NULL; node = node->NextSibling()){
		if(!node->ToElement()){
			continue;
		}
		if(os_strcasecmp(node->Value(),aTagName) == 0){
			return ExtractTagValue(node);
		}else{
			TiXmlNode* node1 = NULL;
			node1 = GetTag(node,aTagName);
			if(node1 != NULL){
				return ExtractTagValue(node1);
			}
		}
	}
	return NULL;
}

TiXmlNode* XmlUtils::GetTag(IN TiXmlDocument &aXmlDoc, IN int8* aTagName){
	TiXmlNode* node = NULL;
	for(node = aXmlDoc.FirstChild(); node != NULL; node = node->NextSibling()){
		if(!node->ToElement()){
			continue;
		}
		if(os_strcasecmp(node->Value(),aTagName) == 0){
			return node;
		}else{
			TiXmlNode* node1 = NULL;
			node1 = GetTag(node,aTagName);
			if(node1 != NULL){
				return node1;
			}
		}
	}
	return NULL;
}

bool XmlUtils::IsTagPresent(IN TiXmlNode* aNode, IN int8* aTagName){
	TiXmlNode* node = NULL;
	if(NULL == aNode){
		return false;
	}
	for(node = aNode->FirstChild(); node != NULL; node = node->NextSibling()){
		if(!node->ToElement()){
			continue;
		}
		if(os_strcasecmp(node->Value(),aTagName) == 0){
			return true;
		}else{
			TiXmlNode *node1 = GetTag(node,aTagName);
			if(node1 != NULL){
				return false;
			}
		}
	}
	return false;
}

TiXmlNode* XmlUtils::GetTag(IN TiXmlNode* aNode, IN int8* aTagName){
	TiXmlNode* node = NULL;
	if(NULL == aNode){
		return NULL;
	}
	for(node = aNode->FirstChild(); node != NULL; node = node->NextSibling()){
		if(!node->ToElement()){
			continue;
		}
		if(os_strcasecmp(node->Value(),aTagName) == 0){
			return node;
		}else{
			TiXmlNode *node1 = GetTag(node,aTagName);
			if(node1 != NULL){
				return node1;
			}
		}
	}
	return NULL;
}

ReturnStatus XmlUtils::ExtractTagValue(IN TiXmlNode* aNode, INOUT int8* aTagValue){
	TiXmlNode* node = NULL;
	if(NULL == aNode){
		return FAILURE;
	}
	for(node = aNode->FirstChild(); node != NULL; node = node->NextSibling()){
		if(node->ToText()){
			os_strncpy(aTagValue,node->Value(),os_strlen((int8*)node->Value()));
			return SUCCESS;
		}
	}
	return FAILURE;
}

int8* XmlUtils::ExtractTagValue(IN TiXmlNode* aNode){
	TiXmlNode* node = NULL;
	if(NULL == aNode){
		return NULL;
	}
	for(node = aNode->FirstChild(); node != NULL; node = node->NextSibling()){
		if(node->ToText()){
			return (int8*) node->Value();
		}
	}
	return NULL;
}

int32 XmlUtils::GetNumOfChilds(IN TiXmlNode* aNode){
	int count = 0;
	TiXmlNode* node = NULL;
	if(NULL == aNode){
		return 0;
	}
	for(node = aNode->FirstChild(); node != NULL; node = node->NextSibling()){
		if(node->ToElement()){
			count++;
		}
	}
	return count;
}

TiXmlNode* XmlUtils::GetSubTagAtIndex(IN TiXmlNode* aNode, int32 index){
	int count = 0;
	TiXmlNode* node = NULL;
	if(NULL == aNode){
		return NULL;
	}
	for(node = aNode->FirstChild(); node != NULL; node = node->NextSibling()){
		if(node->ToElement()){
			count++;
		}
		if(count == index){
			return node;
		}
	}
	return NULL;
}

TiXmlNode* XmlUtils::GetRepetetiveTagAtIndex(IN TiXmlNode* aNode, IN int8* aTagName,int32 index){
	int count = 0;
	TiXmlNode* node = NULL;
	if(NULL == aNode){
		return NULL;
	}
	for(node = aNode->FirstChild(); node != NULL; node = node->NextSibling()){
		if(node->ToElement() && os_strcasecmp(aTagName,node->Value()) == 0){
			count++;
		}
		if(count == index){
			return node;
		}
	}
	return NULL;
}

int32 XmlUtils::GetNumOfAttributes(IN TiXmlNode* aNode){
	int count = 0;
	TiXmlAttribute* attr = NULL;
	if(NULL == aNode){
		return 0;
	}
	TiXmlElement *elem = aNode->ToElement();
	if(elem == NULL){
		return count;
	}
	for(attr = elem->FirstAttribute(); attr != NULL; attr = attr->Next()){
			count++;
	}
	return count;
}

TiXmlAttribute* XmlUtils::GetAttributeAtIndex(IN TiXmlNode* aNode, int32 index){
	int count = 0;
	TiXmlAttribute* attr = NULL;
	if(NULL == aNode){
		return NULL;
	}
	TiXmlElement *elem = aNode->ToElement();
	if(elem == NULL){
		return NULL;
	}
	for(attr = elem->FirstAttribute(); attr != NULL; attr = attr->Next()){
			count++;
			if(count == index){
				return attr;
			}
	}
	return NULL;
}
