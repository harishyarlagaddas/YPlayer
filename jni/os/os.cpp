#include "os.h"

/* Global Variables. */
MallocPtrLinkedList *gMallocPtrLinkedListStartPtr = NULL;
MallocPtrLinkedList *gMallocPtrLinkedListLastPtr = NULL;

/********************************************************************************
*********************** Memory related functions ********************************
*********************************************************************************/

void* os_memset(IN void *aSrcPtr,IN int32 aVal,IN size_t aNumBytes)
{
  return memset(aSrcPtr,aVal,aNumBytes);
}

void* _os_malloc(const int8 *aFileName, int32 aLineNo, IN int32 aSize)
{
	void *ptr = malloc(aSize);
#ifdef  MEM_TRACE
	MallocPtrLinkedList *mallocPtrLinkedList = NULL;
	MallocStruct *mallocStruct = (MallocStruct*)malloc(sizeof(MallocStruct));
	if(NULL == mallocStruct){
		goto _os_malloc;
	}
	if(os_strlen((int8*)aFileName)){
		os_strncpy(mallocStruct->FileName,aFileName,sizeof(mallocStruct->FileName));
	}
	mallocStruct->LineNo = aLineNo;
	mallocStruct->Size = aSize;
	mallocStruct->Ptr = ptr;

	mallocPtrLinkedList = (MallocPtrLinkedList*)malloc(sizeof(MallocPtrLinkedList));
	if(NULL == mallocPtrLinkedList){
		free(mallocStruct);
		goto _os_malloc;
	}
	mallocPtrLinkedList->mallocStruct = mallocStruct;
	mallocPtrLinkedList->NextPtr = NULL;
	if(gMallocPtrLinkedListStartPtr == NULL){
		gMallocPtrLinkedListStartPtr = gMallocPtrLinkedListLastPtr = mallocPtrLinkedList;
	}else{
		gMallocPtrLinkedListLastPtr->NextPtr = mallocPtrLinkedList;
		gMallocPtrLinkedListLastPtr = mallocPtrLinkedList;
	}
_os_malloc:
#endif
	return ptr;
}

void os_free(IN void *aPtr)
{
	free(aPtr);
#ifdef MEM_TRACE
	MallocPtrLinkedList *mallocPtrLinkedList,*mallocPtrLinkedListPrev = NULL;
	if(NULL == gMallocPtrLinkedListStartPtr){
		return;
	}
	mallocPtrLinkedList = gMallocPtrLinkedListStartPtr;
	mallocPtrLinkedListPrev = gMallocPtrLinkedListStartPtr;
	while(NULL != mallocPtrLinkedList){
		if(aPtr == mallocPtrLinkedList->mallocStruct->Ptr){
			if(gMallocPtrLinkedListStartPtr == mallocPtrLinkedList){
				gMallocPtrLinkedListStartPtr = mallocPtrLinkedList->NextPtr;
				mallocPtrLinkedListPrev = NULL;
			}else{
				mallocPtrLinkedListPrev->NextPtr = mallocPtrLinkedList->NextPtr;
			}

			if(gMallocPtrLinkedListLastPtr == mallocPtrLinkedList){
				gMallocPtrLinkedListLastPtr = mallocPtrLinkedListPrev;
				if(gMallocPtrLinkedListLastPtr){
					gMallocPtrLinkedListLastPtr->NextPtr = NULL;
				}
			}
			free(mallocPtrLinkedList->mallocStruct);
			free(mallocPtrLinkedList);
			break;
		}
		mallocPtrLinkedListPrev = mallocPtrLinkedList;
		mallocPtrLinkedList = mallocPtrLinkedList->NextPtr;
	}
#endif
}

void os_mem_trace_result()
{
#ifdef MEM_TRACE
	if(NULL == gMallocPtrLinkedListStartPtr){
		//LOG_CONSOLE("All the memory is deallocated properly.. No Memory Leaks.. Good Programming.. :)");
		LOGD("All the memory is deallocated properly.. No Memory Leaks.. Good Programming.. :)\n");
	}else{
		//LOG_CONSOLE("Memory Leaks Detected!");
		LOGE("Memory Leaks Detected!\n");
		MallocPtrLinkedList *mallocPtrLinkedList = gMallocPtrLinkedListStartPtr, *mallocPtrLinkedListTemp = NULL;
		while(NULL != mallocPtrLinkedList){
			/*
			LOG_CONSOLE("FileName[%s],LineNo[%d],Size[%d]",mallocPtrLinkedList->mallocStruct->FileName,
								       mallocPtrLinkedList->mallocStruct->LineNo,
								       mallocPtrLinkedList->mallocStruct->Size);
			*/
			LOGE("FileName[%s],LineNo[%d],Size[%d]\n",mallocPtrLinkedList->mallocStruct->FileName,
								       mallocPtrLinkedList->mallocStruct->LineNo,
								       mallocPtrLinkedList->mallocStruct->Size);

			if(gMallocPtrLinkedListLastPtr == mallocPtrLinkedList){
				 gMallocPtrLinkedListLastPtr = NULL;
			}
			mallocPtrLinkedListTemp = mallocPtrLinkedList->NextPtr;
			free(mallocPtrLinkedList->mallocStruct);
			free(mallocPtrLinkedList);
			mallocPtrLinkedList = mallocPtrLinkedListTemp;
			gMallocPtrLinkedListStartPtr = mallocPtrLinkedList;
		}
	}
#endif
}

void* os_memcpy(OUT void *aDestPtr,IN const void *aSrcPtr, size_t aNumBytes)
{
	return memcpy(aDestPtr,aSrcPtr,aNumBytes);
}

/********************************************************************************
******************* Memory related functions END ********************************
*********************************************************************************/


/********************************************************************************
*********************** String related functions ********************************
*********************************************************************************/
int8* os_strncpy(OUT int8 *aDestPtr,IN const int8 *aSrcPtr, IN size_t aSize)
{
  return strncpy(aDestPtr,aSrcPtr,aSize);
}

int32 os_strlen(int8 *aInputPtr)
{
	if(NULL == aInputPtr){
		return 0;
	}else{
		return strlen(aInputPtr);
	}
}

int32 os_strcmp(IN const int8 *aStr1,IN const int8 *aStr2)
{
	return strcmp(aStr1,aStr2);
}

int8* os_strrstr(IN int8 *aStr1, IN int8 *aStr2)
{
	if(NULL == aStr1 || NULL == aStr2){
		return NULL;
	}

	int8 *tmp = aStr1;
	int32 len = os_strlen(aStr1);
	while(len > 0){
		if(os_strstr(tmp+len-1,aStr2)){
			return tmp+len-1;
		}
		len--;
	}
	/* Reached here means we didn't find the string. */
	return NULL;
}

int8* os_strrcasestr(IN int8 *aStr1, IN int8 *aStr2)
{
	if(NULL == aStr1 || NULL == aStr2){
		return NULL;
	}

	int8 *tmp = aStr1;
	int32 len = os_strlen(aStr1);
	while(len > 0){
		if(os_strcasestr(tmp+len-1,aStr2)){
			return tmp+len-1;
		}
		len--;
	}
	/* Reached here means we didn't find the string. */
	return NULL;
}

/********************************************************************************
******************* String related functions END ********************************
*********************************************************************************/
