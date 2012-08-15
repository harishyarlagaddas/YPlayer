#include "basic_datatypes.h"
#include "upnp_datatypes.h"
#include "os.h"
#include "return_codes.h"
#include "log.h"
#include "linkedlist.h"
#include "test_upnp_control_point.h"

cUPnPControlPoint *testUPNPControlPoint::cvUPnPCPPtr = NULL;
cLinkedList<CPObject> testUPNPControlPoint::cvServerList;
cLinkedList<CPObject> testUPNPControlPoint::cvRendererList;

int8 testUPNPControlPoint::cvLastActions[20][20] = {{'\0'}};
int32 testUPNPControlPoint::cvLastActionIndex = 0;

testUPNPControlPoint::testUPNPControlPoint()
{
	cvUPnPCPPtr = NULL;
	cvLastActionIndex = 0;
}

testUPNPControlPoint::~testUPNPControlPoint()
{
	if(cvUPnPCPPtr){
		os_delete(cvUPnPCPPtr);
	}
}

void testUPNPControlPoint::startTestUPNPControlPoint()
{
	{
		cvUPnPCPPtr = os_new(cUPnPControlPoint,());
		if(NULL == cvUPnPCPPtr){
			  LOG_CONSOLE("test_UPnPControlPoint cUPnPControlPoint creation Failed.Hence exiting from here itself.. ");
			  return;
		}
		SSDPInfoStruct sSSDPDisInfoStruct = {"239.255.255.250",1900,60,"Linux-Ubuntu",9,04,"UPnPTestStack",1,0,48600};
		LOG_INITIALIZE();
		//LOG_CONSOLE("Setting SSDP Info");
		cvUPnPCPPtr->SetSSDPInfo(sSSDPDisInfoStruct);
		cvUPnPCPPtr->RegisterControlPointObserver((UPnPControlPointObserver*)this);
		/* Now Create the New Thread which will listen for the user input and perfroms user selected operations.. */
		if(-1 == Create_New_Thread(testUPNPControlPoint::userInteraction,(void*)cvUPnPCPPtr,THREAD_PRIORITY_IS_NORMAL)){
			LOG_CONSOLE("test_UPnPControlPoint Failed to Create the userInteration Thread.. Hence Exiting from here itself..");
			return ;
		}
		//LOG_CONSOLE("Starting UPnP Control Point");
		cvUPnPCPPtr->StartControlPoint();
	}
}

void* testUPNPControlPoint::userInteraction(void *aInPtr)
{
	int8 userInput[256] = {'\0'};
	cUPnPControlPoint *upnpCPPtr = NULL;
	int32 count =0,getCount =0;

	if(NULL == aInPtr){
		LOG_CONSOLE("userInteration: Unable to get the pointer to cUPnPControlPoint. So no point in continueing. Hence Exiting.. ");
		return NULL;
	}

	upnpCPPtr = (cUPnPControlPoint*)aInPtr;
	printf("Choose From the Following Choices... \n\n LS -> To List all the Available Servers\n LR -> To List all the Available Renderers\n BACK -> At any point to go back to the previous Action (works only after selecting the server)\n\n");
	while(1){
		os_memset(userInput,0,sizeof(userInput));
		scanf("%s",userInput);
		if(0 == os_strcasecmp(userInput,"LS")){
			ListAvailableServers();
		}else if(0 == os_strcasecmp(userInput,"LR")){
			ListAvailableRenderers();
		}else if('S' == userInput[0] && 'S' == userInput[1]){
				/* When the server is selected then start the LastActions recording.. Ignore all the previouse Actions..*/
				while(cvLastActionIndex >= 0){
					os_memset(cvLastActions[cvLastActionIndex],0,sizeof(cvLastActions[cvLastActionIndex]));
					cvLastActionIndex--;
				}
				cvLastActionIndex = 0;

				cLinkedList<MetaDataObj> objList;
				if(SUCCESS != upnpCPPtr->SelectServer(os_atoi(&userInput[2]),false)){
					printf("Some issue with Selection of the Server..\n");
				}
				count = upnpCPPtr->GetNumOfChilds(0);
				getCount = 0;
				while(count > 0){
					os_memset(&objList,0,sizeof(objList));
					if(SUCCESS != upnpCPPtr->GetObjects(0,objList,getCount,50,MUSIC)){
						printf("Some issue with Getting Root Objects from the Server..\n");
					}
					/* Now we got the objects from the server.. print them... */
					printObjects(objList);
					count -= 50;
					getCount += 50;
				}
				os_strncpy(cvLastActions[cvLastActionIndex],userInput,sizeof(cvLastActions[cvLastActionIndex]));
				cvLastActionIndex++;
		}else if('S' == userInput[0] && 'R' == userInput[1]){
			cLinkedList<CPObject> objList;
			//if(SUCCESS == upnpCPPtr->BrowseObjectsInRenderer(0,userInput[2],objList)){
			//	printObjects(objList);
			//}
		}else if(os_strcasestr(userInput,"BC")){
			int32 /*deviceID = 0,*/ObjectID = 0;
			int8 *tempPtr = NULL;
			cLinkedList<MetaDataObj> objList;

			tempPtr = os_strcasestr(userInput,"BC");
			tempPtr += 2; /* Forwarding by 3 so that we will bypass -BC */
			//deviceID = os_atoi(userInput);
			ObjectID = os_atoi(tempPtr);
			count = upnpCPPtr->GetNumOfChilds(ObjectID);
			getCount = 0;
			while(count > 0){
				os_memset(&objList,0,sizeof(objList));
				if(SUCCESS != upnpCPPtr->GetObjects(ObjectID,objList,getCount,50,MUSIC)){
					printf("Some issue with Getting Root Objects from the Server..\n");
				}
				/* Now we got the objects from the server.. print them... */
				printObjects(objList);
				count -= 50;
				getCount += 50;
			}
			/* Now we got the objects from the server.. print them... */
			//printObjects(objList);
			os_strncpy(cvLastActions[cvLastActionIndex],userInput,sizeof(cvLastActions[cvLastActionIndex]));
			cvLastActionIndex++;
		}else if(os_strcasestr(userInput,"PSO")){
			int32 deviceID = 0,ObjectID = 0;
			int8 *tempPtr = NULL;
			MetaDataObj metadataObj;

			tempPtr = os_strcasestr(userInput,"PSO");
			*tempPtr = '\0';
			tempPtr += 3; /* Forwarding by 3 so that we will bypass -BC */
			deviceID = os_atoi(userInput);
			ObjectID = os_atoi(tempPtr);

			os_memset(&metadataObj,0,sizeof(metadataObj));
			if(SUCCESS != upnpCPPtr->GetMetaData(deviceID,ObjectID,metadataObj)){
				printf("Some issue with Getting the Metadata of the selected Object from the Server..\n");
			}
			/* Now we got the objects from the server.. print them... */
			printMetaData(metadataObj);
		}else{
			/* Presently we don't hadle this case.. Just ignore.. */
			printf("Invalid Option. Please Choose the correct option.\n");
		}
	}
}

void testUPNPControlPoint::ListAvailableServers()
{
	CPObject obj;
	int32 count = 0,noServers = cvServerList.GetNumberOfElements();
	for(count = 1; count <= noServers; count++){
		os_memset(&obj,0,sizeof(obj));
		if(SUCCESS == cvServerList.GetElementAtIndex(obj,count)){
			printf(" SS%d -> To select [%s] Server\n",obj.ObjID,obj.DeviceName);
		}
	}
}

void testUPNPControlPoint::ListAvailableRenderers()
{
	CPObject obj;
	int32 count = 0,noRederers = cvRendererList.GetNumberOfElements();
	for(count = 1; count <= noRederers; count++){
		os_memset(&obj,0,sizeof(obj));
		if(SUCCESS == cvRendererList.GetElementAtIndex(obj,count)){
			printf(" SR%d -> To select [%s] Renderer\n",obj.ObjID,obj.DeviceName);
		}
	}
}

void testUPNPControlPoint::printObjects(cLinkedList<MetaDataObj> &aObjList)
{
	int32 count = 1;
	MetaDataObj obj;
	if(1 > aObjList.GetNumberOfElements()){
		printf("No Objects in the Selected Server.. :)\n");
	}
	for(count = 1; count <= aObjList.GetNumberOfElements(); count++){
		os_memset(&obj,0,sizeof(obj));
		if(SUCCESS == aObjList.GetElementAtIndex(obj,count)){
			if(true == obj.Container){
				printf("BC%d -> To Browse the Container [%s] of the Server\n",obj.ObjectID,obj.Title);
			}else{
				printf("PSO%d -> To Play the Selected the Object [%s] with ProtocolInfo [%s] and URL [%s]\n",obj.ObjectID,obj.Title,obj.ProtocolInfo,obj.URL);
			}
		}
	}
}

void testUPNPControlPoint::printMetaData(MetaDataObj &aMeatadataObj)
{
	printf(" Title = %s\n Artist = %s\n Alubm = %s\n Genre = %s\n ProtocolInfo = %s\n URL = %s\n Size = %d\n",aMeatadataObj.Title,
			       aMeatadataObj.Artist,
			       aMeatadataObj.Album,
			       aMeatadataObj.Genre,
			       aMeatadataObj.ProtocolInfo,
			       aMeatadataObj.URL,
			       aMeatadataObj.Size);
}

void testUPNPControlPoint::CPEventReceived(CPObject event)
{
	if(SERVER_ADDED == event.EventID){
		cvServerList.Add(event);
		printf(" SS%d -> To select [%s] Server\n",event.DeviceID,event.DeviceName);
	}else if(RENDERER_ADDED == event.EventID){
		cvRendererList.Add(event);
		printf(" SR%d -> To select [%s] Renderer\n",event.DeviceID,event.DeviceName);
	}else if(SERVER_REMOVED == event.EventID){
		CPObject obj;
		int32 count = 0,noServers = cvServerList.GetNumberOfElements();
		for(count = 1; count <= noServers; count++){
			os_memset(&obj,0,sizeof(obj));
			if(SUCCESS == cvServerList.GetElementAtIndex(obj,count)){
				if( event.ObjID == obj.ObjID){
					/* Object Found. Remove from the list...*/
					cvServerList.RemoveElementAtIndex(count);
					printf(" [%s] Server is removed. Hence SS%d is Invalid \n",obj.DeviceName,obj.DeviceID);
					return;
				}
			}
		}
	}else if(RENDERER_REMOVED == event.EventID){
		CPObject obj;
		int32 count = 0,noRenderers = cvRendererList.GetNumberOfElements();
		for(count = 1; count <= noRenderers; count++){
			os_memset(&obj,0,sizeof(obj));
			if(SUCCESS == cvRendererList.GetElementAtIndex(obj,count)){
				if( event.ObjID == obj.ObjID){
					/* Object Found. Remove from the list...*/
					cvRendererList.RemoveElementAtIndex(count);
					printf(" [%s] Renderer is removed. Hence SR%d is Invalid \n",obj.DeviceName,obj.DeviceID);
					return;
				}
			}
		}
	}else{
		/* NOthing to do here.. Just ignore.. */
	}
}
