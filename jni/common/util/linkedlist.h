#ifndef LINKEDLIST_H_INCLUDED
#define LINKEDLIST_H_INCLUDED

#include "../include/basic_datatypes.h"
#include "../../os/os.h"
#include "../include/return_codes.h"
#include "../log/log.h"

template <class aInputClass>
class cLinkedList
{
public:
	cLinkedList();
	~cLinkedList();
	int32 Add(aInputClass& aInputObj);
	int32 AddAtStart(aInputClass& aInputObj);
	int32 RemoveAtEnd(aInputClass& aInputObj);
	int32 RemoveAtStart(aInputClass& aInputObj);
	int32 GetElementAtIndex(aInputClass& aInputObj,const int32 aIndex);
	int32 GetElementAtStart(aInputClass& aInputObj);
	int32 GetElementAtEnd(aInputClass& aInputObj);
	int32 GetNumberOfElements();
	int32 RemoveElementAtIndex(const int32 aIndex);
	void DestroyList();
private:
	typedef struct sLinkedListStruct{
		aInputClass Data;
		struct sLinkedListStruct* spNext;
	}LinkedListStruct;
	LinkedListStruct* cvHeadPtr;
	LinkedListStruct* cvTailPtr;
	int32 cvCount;
};

#include "linkedlist.incl"

#endif /* LINKEDLIST_H_INCLUDED */
