#include "xmlcomposer.h"

cXmlComposer::cXmlComposer()
{
	iXmlMatrix = NULL;
}

cXmlComposer::~cXmlComposer()
{
	XmlMatrix *xmlPtr = iXmlMatrix,*xmlPtr1 = NULL;
	while(NULL != xmlPtr){
		if(xmlPtr->SidePtr){
			xmlPtr = xmlPtr->SidePtr;
			continue;
		}
		else if(xmlPtr->DownPtr){
			xmlPtr1 = xmlPtr;
			xmlPtr = xmlPtr->DownPtr;
			/* Free the memory before going to down pointer. */
			DeleteXmlComposerTag(xmlPtr1->NodePtr);
			os_free(xmlPtr1);
			continue;
		}
		else{
			if(NULL == xmlPtr->BackPtr){
				/* Free the memory before leaving from here. */
				DeleteXmlComposerTag(xmlPtr->NodePtr);
				os_free(xmlPtr);
				return;
			}
			else{
				xmlPtr1 = xmlPtr;
				xmlPtr = xmlPtr->BackPtr;
				/* Free the memory before going further. */
				DeleteXmlComposerTag(xmlPtr1->NodePtr);
				os_free(xmlPtr1);
				while(xmlPtr){
					if(xmlPtr->DownPtr){
						xmlPtr1 = xmlPtr;
						xmlPtr = xmlPtr->DownPtr;
						/* Free the memory before going further. */
						DeleteXmlComposerTag(xmlPtr1->NodePtr);
						os_free(xmlPtr1);
						break;
					}else if(xmlPtr->BackPtr){
						xmlPtr1 = xmlPtr;
						xmlPtr = xmlPtr->BackPtr;
						/* Free the memory before going further. */
						DeleteXmlComposerTag(xmlPtr1->NodePtr);
						os_free(xmlPtr1);
						continue;
					}
					else{
						/* Free the memory before leaving from here. */
						DeleteXmlComposerTag(xmlPtr->NodePtr);
						os_free(xmlPtr);
						return;
					}
				}
			}

		}
	}
}
void cXmlComposer::DeleteXmlComposerTag(XmlComposerTag *aInputNodePtr)
{
	/* Free the memory allocated above before exiting. Otherwise Memory leaks will happen. :)*/
	if(aInputNodePtr->ParentName.StringVal.Val){
		os_free(aInputNodePtr->ParentName.StringVal.Val);
		aInputNodePtr->ParentName.StringVal.Val = NULL;
	}
	if(aInputNodePtr->AttributeValue.StringVal.Val){
		os_free(aInputNodePtr->AttributeValue.StringVal.Val);
		aInputNodePtr->AttributeValue.StringVal.Val = NULL;
	}
	if(aInputNodePtr->AttributeName.StringVal.Val){
		os_free(aInputNodePtr->AttributeName.StringVal.Val);
		aInputNodePtr->AttributeName.StringVal.Val = NULL;
	}
	if(aInputNodePtr->Value.StringVal.Val){
		os_free(aInputNodePtr->Value.StringVal.Val);
		aInputNodePtr->Value.StringVal.Val = NULL;
	}
	if(aInputNodePtr->Name.StringVal.Val){
		os_free(aInputNodePtr->Name.StringVal.Val);
		aInputNodePtr->Name.StringVal.Val = NULL;
	}
	if(aInputNodePtr){
		os_free(aInputNodePtr);
		aInputNodePtr = NULL;
	}
}

XmlComposerTag* cXmlComposer::CreateXmlComposerTag(XmlComposerTag aInputNode)
{
	/* Allocate the memory for the xmlnode and add it to the iXmlMatrix. This memory deallocation will be taken care in destructor. */
	XmlComposerTag *xmlNodePtr = (XmlComposerTag*)os_malloc(sizeof(XmlComposerTag));
	if(NULL == xmlNodePtr){
		LOGD("cXmlComposer::CreateXmlComposerTag Memory Allocation Failed. Hence returning NULL");
		return NULL;
	}
	os_memset(xmlNodePtr,0,sizeof(XmlComposerTag));

	/* Allocate the memory as per it's enum type. */
	xmlNodePtr->Name.paramType = aInputNode.Name.paramType;
	if(XML_STRING == aInputNode.Name.paramType){
		xmlNodePtr->Name.StringVal.Len = aInputNode.Name.StringVal.Len;
		xmlNodePtr->Name.StringVal.Val = (int8*)os_malloc(aInputNode.Name.StringVal.Len+1);
		if(NULL == xmlNodePtr->Name.StringVal.Val){
			LOGD("cXmlComposer::CreateXmlComposerTag Memory Allocation Failed.");
			goto ErrorExit;
		}
		os_memset(xmlNodePtr->Name.StringVal.Val,0,aInputNode.Name.StringVal.Len+1);
		os_strncpy(xmlNodePtr->Name.StringVal.Val,aInputNode.Name.StringVal.Val,aInputNode.Name.StringVal.Len+1);
	}

	xmlNodePtr->Value.paramType = aInputNode.Value.paramType;
	if(XML_STRING == aInputNode.Value.paramType){
		xmlNodePtr->Value.StringVal.Len = aInputNode.Value.StringVal.Len;
		xmlNodePtr->Value.StringVal.Val = (int8*)os_malloc(aInputNode.Value.StringVal.Len+1);
		if(NULL == xmlNodePtr->Value.StringVal.Val){
			LOGD("cXmlComposer::CreateXmlComposerTag Memory Allocation Failed.");
			goto ErrorExit;
		}
		os_memset(xmlNodePtr->Value.StringVal.Val,0,aInputNode.Value.StringVal.Len+1);
		os_strncpy(xmlNodePtr->Value.StringVal.Val,aInputNode.Value.StringVal.Val,aInputNode.Value.StringVal.Len+1);
	}else{
		xmlNodePtr->Value.IntegerVal = aInputNode.Value.IntegerVal;
	}

	xmlNodePtr->AttributeName.paramType = aInputNode.AttributeName.paramType;
	if(XML_STRING == aInputNode.AttributeName.paramType){
		xmlNodePtr->AttributeName.StringVal.Len = aInputNode.AttributeName.StringVal.Len;
		xmlNodePtr->AttributeName.StringVal.Val = (int8*)os_malloc(aInputNode.AttributeName.StringVal.Len+1);
		if(NULL == xmlNodePtr->AttributeName.StringVal.Val){
			LOGD("cXmlComposer::CreateXmlComposerTag Memory Allocation Failed.");
			goto ErrorExit;
		}
		os_memset(xmlNodePtr->AttributeName.StringVal.Val,0,aInputNode.AttributeName.StringVal.Len+1);
		os_strncpy(xmlNodePtr->AttributeName.StringVal.Val,aInputNode.AttributeName.StringVal.Val,aInputNode.AttributeName.StringVal.Len+1);
	}

	xmlNodePtr->AttributeValue.paramType = aInputNode.AttributeValue.paramType;
	if(XML_STRING == aInputNode.AttributeValue.paramType){
		xmlNodePtr->AttributeValue.StringVal.Len = aInputNode.AttributeValue.StringVal.Len;
		xmlNodePtr->AttributeValue.StringVal.Val = (int8*)os_malloc(aInputNode.AttributeValue.StringVal.Len+1);
		if(NULL == xmlNodePtr->AttributeValue.StringVal.Val){
			LOGD("cXmlComposer::CreateXmlComposerTag Memory Allocation Failed.");
			goto ErrorExit;
		}
		os_memset(xmlNodePtr->AttributeValue.StringVal.Val,0,aInputNode.AttributeValue.StringVal.Len+1);
		os_strncpy(xmlNodePtr->AttributeValue.StringVal.Val,aInputNode.AttributeValue.StringVal.Val,aInputNode.AttributeValue.StringVal.Len+1);
	}else{
		xmlNodePtr->AttributeValue.IntegerVal = aInputNode.AttributeValue.IntegerVal;
	}

	xmlNodePtr->ParentName.paramType = aInputNode.ParentName.paramType;
	if(XML_STRING == aInputNode.ParentName.paramType){
		xmlNodePtr->ParentName.StringVal.Len = aInputNode.ParentName.StringVal.Len;
		xmlNodePtr->ParentName.StringVal.Val = (int8*)os_malloc(aInputNode.ParentName.StringVal.Len+1);
		if(NULL == xmlNodePtr->ParentName.StringVal.Val){
			LOGD("cXmlComposer::CreateXmlComposerTag Memory Allocation Failed.");
			goto ErrorExit;
		}
		os_memset(xmlNodePtr->ParentName.StringVal.Val,0,aInputNode.ParentName.StringVal.Len+1);
		os_strncpy(xmlNodePtr->ParentName.StringVal.Val,aInputNode.ParentName.StringVal.Val,aInputNode.ParentName.StringVal.Len+1);
	}

	xmlNodePtr->Depth.paramType = aInputNode.Depth.paramType;
	xmlNodePtr->Depth.IntegerVal = aInputNode.Depth.IntegerVal;
	xmlNodePtr->ParentDepth.paramType = aInputNode.ParentDepth.paramType;
	xmlNodePtr->ParentDepth.IntegerVal = aInputNode.ParentDepth.IntegerVal;
	return xmlNodePtr;
ErrorExit:
	LOGD("cXmlComposer::CreateXmlComposerTag Entered into ErrorExit section.");
	/* Free the memory allocated above before exiting. Otherwise Memory leaks will happen. :)*/
	if(xmlNodePtr->ParentName.StringVal.Val){
		os_free(xmlNodePtr->ParentName.StringVal.Val);
		xmlNodePtr->ParentName.StringVal.Val = NULL;
	}
	if(xmlNodePtr->AttributeValue.StringVal.Val){
		os_free(xmlNodePtr->AttributeValue.StringVal.Val);
		xmlNodePtr->AttributeValue.StringVal.Val = NULL;
	}
	if(xmlNodePtr->AttributeName.StringVal.Val){
		os_free(xmlNodePtr->AttributeName.StringVal.Val);
		xmlNodePtr->AttributeName.StringVal.Val = NULL;
	}
	if(xmlNodePtr->Value.StringVal.Val){
		os_free(xmlNodePtr->Value.StringVal.Val);
		xmlNodePtr->Value.StringVal.Val = NULL;
	}
	if(xmlNodePtr->Name.StringVal.Val){
		os_free(xmlNodePtr->Name.StringVal.Val);
		xmlNodePtr->Name.StringVal.Val = NULL;
	}
	if(xmlNodePtr){
		os_free(xmlNodePtr);
		xmlNodePtr = NULL;
	}
	return NULL;
}

ReturnStatus cXmlComposer::AddNode(XmlComposerTag aInputNode)
{
	uint32 parentDepth = aInputNode.ParentDepth.IntegerVal;
	/* If Depth is zero means then this entry should go in to the first row. */
	if(1 == aInputNode.Depth.IntegerVal){
		/* If iXmlMatrix is NULL means this will be first entry. */
		if(NULL == iXmlMatrix){
			/* Allocate the memory for iXmlMatrix and intialize with zeros. */
			iXmlMatrix = (XmlMatrix*)os_malloc(sizeof(XmlMatrix));
			if(NULL == iXmlMatrix){
				LOGD("cXmlComposer::AddNode Memory Allocation Failed. ");
				return FAILURE;
			}
			os_memset(iXmlMatrix,0,sizeof(XmlMatrix));

			/* Create the xmlnode locally and add it to the iXmlMatrix. */
			XmlComposerTag *xmlNodePtr = CreateXmlComposerTag(aInputNode);
			if(NULL == xmlNodePtr){
				LOGD("cXmlComposer::AddNode CreateXmlComposerTag Failed.");
				os_free(iXmlMatrix);
				iXmlMatrix = NULL;
				return FAILURE;
			}
			/* Store the pointer inside iXmlMatrix. */
			iXmlMatrix->NodePtr = xmlNodePtr;
			LOGD("cXmlComposer::AddNode Adding the Node with has Name [%s]",xmlNodePtr->Name.StringVal.Val);
			LOGD("cXmlComposer::AddNode Added %x to the iXmlMatrix. Staring entry",iXmlMatrix);
			return SUCCESS;
		}
		else{
			XmlMatrix *xmlPtr = iXmlMatrix,*xmlPtr1 = NULL;
			while(xmlPtr->DownPtr != NULL){
				xmlPtr = xmlPtr->DownPtr;
			}

			xmlPtr1 = (XmlMatrix*)os_malloc(sizeof(XmlMatrix));
			if(NULL == xmlPtr1){
				LOGD("cXmlComposer::AddNode Memory Allocation Failed. ");
				return FAILURE;
			}
			os_memset(xmlPtr1,0,sizeof(XmlMatrix));

			/* Create the xmlnode locally and add it to the iXmlMatrix. */
			XmlComposerTag *xmlNodePtr = CreateXmlComposerTag(aInputNode);
			if(NULL == xmlNodePtr){
				LOGD("cXmlComposer::AddNode CreateXmlComposerTag Failed.");
				os_free(xmlPtr1);
				xmlPtr1 = NULL;
				return FAILURE;
			}
			/* Store the pointer inside iXmlMatrix. */
			xmlPtr1->NodePtr = xmlNodePtr;
			LOGD("cXmlComposer::AddNode Adding the Node with has Name [%s]",xmlNodePtr->Name.StringVal.Val);
			/* Add the xmlMatrix ptr to iXmlMatrix. */
			xmlPtr->DownPtr = xmlPtr1;
			LOGD("cXmlComposer::AddNode Added %x to the iXmlMatrix extending downside to the entry [%s]",xmlPtr1,xmlPtr->NodePtr->Name.StringVal.Val);
			return SUCCESS;
		}
	}
	else{
		XmlMatrix *xmlPtr = iXmlMatrix;
		while(NULL != xmlPtr){
			if(xmlPtr->NodePtr->Depth.IntegerVal == parentDepth){
				if(0 == os_strcmp(xmlPtr->NodePtr->Name.StringVal.Val,aInputNode.ParentName.StringVal.Val)){
					/* Here the below piece of while loop code is needed because if we have two tags
					   of same name and same parent name then we need to add the entry to the last added
					   tag. If below 3 lines of code is not present means then in this case of having same
					   name and same parent then all the entries will be added to the first tag only which
					   is wrong. */
					XmlMatrix *xmlPtr3 = xmlPtr;
					while(xmlPtr3->DownPtr){
						xmlPtr3 = xmlPtr3->DownPtr;
						xmlPtr = xmlPtr3;
					}
					xmlPtr3 = xmlPtr->SidePtr;
					if(NULL == xmlPtr3){
						XmlMatrix *xmlPtr4 = (XmlMatrix*)os_malloc(sizeof(XmlMatrix));
						if(NULL == xmlPtr4){
							LOGD("cXmlComposer::AddNode Memory Allocation Failed. ");
							return FAILURE;
						}
						os_memset(xmlPtr4,0,sizeof(XmlMatrix));

						/* Create the xmlnode locally and add it to the iXmlMatrix. */
						XmlComposerTag *xmlNodePtr4 = CreateXmlComposerTag(aInputNode);
						if(NULL == xmlNodePtr4){
							LOGD("cXmlComposer::AddNode CreateXmlComposerTag Failed.");
							os_free(xmlPtr4);
							xmlPtr4 = NULL;
							return FAILURE;
						}
						/* Store the pointer inside iXmlMatrix. */
						xmlPtr4->NodePtr = xmlNodePtr4;
						xmlPtr4->BackPtr = xmlPtr;
						LOGD("cXmlComposer::AddNode Adding the Node with has Name [%s]",xmlNodePtr4->Name.StringVal.Val);
						/* Add the xmlMatrix ptr to iXmlMatrix. */
						xmlPtr->SidePtr = xmlPtr4;
						LOGD("cXmlComposer::AddNode Added %x to the iXmlMatrix extending left side to the entry[%s]",xmlPtr4,xmlPtr->NodePtr->Name.StringVal.Val);
						return SUCCESS;
					}else{
						while(xmlPtr3->DownPtr != NULL){
							xmlPtr3 = xmlPtr3->DownPtr;
						}

						XmlMatrix *xmlPtr5 = (XmlMatrix*)os_malloc(sizeof(XmlMatrix));
						if(NULL == xmlPtr5){
							LOGE("cXmlComposer::AddNode Memory Allocation Failed. ");
							return FAILURE;
						}
						os_memset(xmlPtr5,0,sizeof(XmlMatrix));

						/* Create the xmlnode locally and add it to the iXmlMatrix. */
						XmlComposerTag *xmlNodePtr5 = CreateXmlComposerTag(aInputNode);
						if(NULL == xmlNodePtr5){
							LOGE("cXmlComposer::AddNode CreateXmlComposerTag Failed.");
							os_free(xmlPtr5);
							xmlPtr5 = NULL;
							return FAILURE;
						}
						/* Store the pointer inside iXmlMatrix. */
						xmlPtr5->NodePtr = xmlNodePtr5;
						xmlPtr5->BackPtr = xmlPtr;
						LOGD("cXmlComposer::AddNode Adding the Node with has Name [%s]",xmlNodePtr5->Name.StringVal.Val);
						/* Add the xmlMatrix ptr to iXmlMatrix. */
						xmlPtr3->DownPtr = xmlPtr5;
						LOGD("cXmlComposer::AddNode Added %x to the iXmlMatrix extending Downside to the entry[%s]",xmlPtr5,xmlPtr3->NodePtr->Name.StringVal.Val);
						return SUCCESS;
					}
				}
			}
			if(xmlPtr->SidePtr){
				xmlPtr = xmlPtr->SidePtr;
				continue;
			}
			else if(xmlPtr->DownPtr){
				xmlPtr = xmlPtr->DownPtr;
				continue;
			}
			else{
				if(NULL == xmlPtr->BackPtr){
					LOGE("cXmlComposer::AddNode OUT returning Failure ");
					return FAILURE;
				}
				else{
					xmlPtr = xmlPtr->BackPtr;
					while(xmlPtr){
						if(xmlPtr->DownPtr){
							xmlPtr = xmlPtr->DownPtr;
							break;
						}else if(xmlPtr->BackPtr){
							xmlPtr = xmlPtr->BackPtr;
							continue;
						}
						else{
							LOGE("cXmlComposer::AddNode OUT returning Failure ");
							return FAILURE;
						}
					}
				}
			}
		}
		return FAILURE;
	}
	return FAILURE;
}
uint32 cXmlComposer::GetXmlLength()
{
	uint32 length = 0;
	XmlMatrix *xmlPtr = iXmlMatrix;
	while(NULL != xmlPtr){
		length += (((xmlPtr->NodePtr->Name.StringVal.Len * 2) + 8) + (xmlPtr->NodePtr->Name.StringVal.Len +1)
			  +((xmlPtr->NodePtr->AttributeName.StringVal.Len + xmlPtr->NodePtr->AttributeValue.StringVal.Len) + 8)
			  + 50); // Safe side adding some extra bits.
		if(xmlPtr->SidePtr){
			xmlPtr = xmlPtr->SidePtr;
			continue;
		}
		else if(xmlPtr->DownPtr){
			xmlPtr = xmlPtr->DownPtr;
			continue;
		}
		else{
			if(NULL == xmlPtr->BackPtr){
				return length;
			}
			else{
				xmlPtr = xmlPtr->BackPtr;
				while(xmlPtr){
					if(xmlPtr->DownPtr){
						xmlPtr = xmlPtr->DownPtr;
						break;
					}else if(xmlPtr->BackPtr){
						xmlPtr = xmlPtr->BackPtr;
						continue;
					}
					else{
						return length;
					}
				}
			}

		}
	}
	return length;
}

ReturnStatus cXmlComposer::ComposeXml(int8 *aInputBuf, uint32 aInputBufLen)
{
	int8 *buf = aInputBuf;
	int32 remainSize = aInputBufLen, bytesWritten = 0;
	XmlMatrix *xmlPtr = iXmlMatrix;
	if(aInputBufLen < GetXmlLength()){
		LOGD("cXmlComposer::ComposeXml Input buffer is too small. Hence returning Failure.");
		return FAILURE;
	}

	/* First here write the xml version. */
	bytesWritten = os_snprintf(buf,aInputBufLen,"<?xml version=\"1.0\"?>\n");
	buf += bytesWritten;
	remainSize -= bytesWritten;

	while(NULL != xmlPtr){
		LOGD("cXmlComposer::ComposeXml Adding the tag [%s] to xml",xmlPtr->NodePtr->Name.StringVal.Val);
		bytesWritten = os_snprintf(buf,aInputBufLen,"<%s%s%s%s%s>",xmlPtr->NodePtr->Name.StringVal.Val,
				(xmlPtr->NodePtr->AttributeName.StringVal.Val)?" ":"",
				(xmlPtr->NodePtr->AttributeName.StringVal.Val)?xmlPtr->NodePtr->AttributeName.StringVal.Val:"",
				(xmlPtr->NodePtr->AttributeValue.StringVal.Val)?"=":"",
				(xmlPtr->NodePtr->AttributeValue.StringVal.Val)?xmlPtr->NodePtr->AttributeValue.StringVal.Val:"");
		buf += bytesWritten;
		remainSize -= bytesWritten;

		if(XML_INTEGER == xmlPtr->NodePtr->Value.paramType){
			bytesWritten = os_snprintf(buf,aInputBufLen,"%d",xmlPtr->NodePtr->Value.IntegerVal);
			buf += bytesWritten;
			remainSize -= bytesWritten;
		}
		else if(XML_STRING == xmlPtr->NodePtr->Value.paramType){
			bytesWritten = os_snprintf(buf,aInputBufLen,"%s",xmlPtr->NodePtr->Value.StringVal.Val);
			buf += bytesWritten;
			remainSize -= bytesWritten;
		}
		else{
			bytesWritten = os_snprintf(buf,aInputBufLen,"\n");
			buf += bytesWritten;
			remainSize -= bytesWritten;
		}

		if(xmlPtr->SidePtr){
			xmlPtr = xmlPtr->SidePtr;
			continue;
		}
		else if(xmlPtr->DownPtr){
			/* Comming here means we came to end of corresponding XML tag. So keep ending xml tag. */
			bytesWritten = os_snprintf(buf,aInputBufLen,"</%s>\n",xmlPtr->NodePtr->Name.StringVal.Val);
			buf += bytesWritten;
			remainSize -= bytesWritten;

			xmlPtr = xmlPtr->DownPtr;
			continue;
		}
		else{
			/* Comming here means we came to end of corresponding XML tag. So keep ending xml tag. */
			bytesWritten = os_snprintf(buf,aInputBufLen,"</%s>\n",xmlPtr->NodePtr->Name.StringVal.Val);
			buf += bytesWritten;
			remainSize -= bytesWritten;

			if(NULL == xmlPtr->BackPtr){
				return SUCCESS;
			}
			else{
				xmlPtr = xmlPtr->BackPtr;
				while(xmlPtr){
					if(xmlPtr->DownPtr){
						/* Comming here means we came to end of corresponding XML tag. So keep ending xml tag. */
						bytesWritten = os_snprintf(buf,aInputBufLen,"</%s>\n",xmlPtr->NodePtr->Name.StringVal.Val);
						buf += bytesWritten;
						remainSize -= bytesWritten;

						xmlPtr = xmlPtr->DownPtr;
						break;
					}else if(xmlPtr->BackPtr){
						/* Comming here means we came to end of corresponding XML tag. So keep ending xml tag. */
						bytesWritten = os_snprintf(buf,aInputBufLen,"</%s>\n",xmlPtr->NodePtr->Name.StringVal.Val);
						buf += bytesWritten;
						remainSize -= bytesWritten;

						xmlPtr = xmlPtr->BackPtr;
						continue;
					}
					else{
						/* Comming here means we came to end of corresponding XML tag. So keep ending xml tag. */
						bytesWritten = os_snprintf(buf,aInputBufLen,"</%s>\n",xmlPtr->NodePtr->Name.StringVal.Val);
						buf += bytesWritten;
						remainSize -= bytesWritten;
						return SUCCESS;
					}
				}
			}

		}
	}
	return SUCCESS;
}
