#ifndef TEST_MAINL_H_INCLUDED
#define TEST_MAINL_H_INCLUDED

#include "basic_datatypes.h"
#include "os.h"
#include "return_codes.h"
#include "log.h"
#include "linkedlist.h"
#include "upnp_control_point.h"


#ifdef ANDROID
#define LOG_INI_FILE_NAME "/sdcard/MyThought.ini"
#else
#define LOG_INI_FILE_NAME "MyThought.ini"
#endif

class testUPNPControlPoint : public UPnPControlPointObserver
{
  public:
	testUPNPControlPoint();
	~testUPNPControlPoint();

	void startTestUPNPControlPoint();
	static void* userInteraction(void*);

	/* Pure Virual Function from UPnPControlPointObserver */
	void CPEventReceived(CPObject event);
  private:
	static void ListAvailableServers();
	static void ListAvailableRenderers();
	static void printObjects(cLinkedList<MetaDataObj> &aObjList);
	static void printMetaData(MetaDataObj &aMeatadataObj);

	static cUPnPControlPoint *cvUPnPCPPtr;
	static cLinkedList<CPObject> cvServerList;
	static cLinkedList<CPObject> cvRendererList;

	static int8 cvLastActions[20][20];
	static int32 cvLastActionIndex;
};

#endif /* TEST_MAINL_H_INCLUDED */
