
#ifndef SOCKETIMPL_H_INCLUDED
#define SOCKETIMPL_H_INCLUDED

#include "../include/basic_datatypes.h"
#include "../../os/os.h"
#include "../include/return_codes.h"
#include "../log/log.h"
#include "../util/linkedlist.h"

/* Structures used to pass the information to the observer of the socket */
typedef enum{
	TCP_CLIENT_CONNECTED = 1,
	UDP_MESSAGE_RECEIVED
}SocketActivity;

typedef struct{
	SOCKET SockID;
	int8 SelfIPPtr[MAXIMUM_IP_SIZE];
	uint32 SelfPort;
	int8 PeerIPPtr[MAXIMUM_IP_SIZE];
	uint32 PeerPort;
}TCPSocketInfo;

typedef struct{
	SOCKET SockID;
	int8 SelfIPPtr[MAXIMUM_IP_SIZE];
	uint32 SelfPort;
	int8 PeerIPPtr[MAXIMUM_IP_SIZE];
	uint32 PeerPort;
	int8 *RecvBufPtr;
	uint32 RecvBufLen;
}UDPSocketInfo;

typedef struct {
	SocketActivity SockActivity;
	TCPSocketInfo *TCPSockInfoPtr;
	UDPSocketInfo *UDPSockInfoPtr;
}SocketObserverParams;

/* cSocketObserver class is the interface class. All the classes who want to use "cSocket" class needs to be derived from
   this "cSocketObserver" class and implement all the pure virtual functions present inside this class. Mostly the functions
   present inside this class are the callback functions which "cSocket" Class will call upon the connection of the client
   incase of TCP connections and upon receiving the data incase of UDP Sockets.
*/
class cSocketObserver
{
  public:
	cSocketObserver(){};
	virtual ~cSocketObserver(){};
	/* HandleSocketActivity - HandleSocketActivity is the pure virutal function which needs to be implemented
	   in the derived class. cSoket Class will call this function as a callback when any activity is happened
	   on the socket. Basically this function will be called in two scenarios.
	   1. If the observer creates TCP Server socket, and when any client is connected to this sever socket then
	      cSocket class will call this function as callback. In this case SockActivity will be equal to
	      TCP_CLINET_CONNECTED and TCPSockInfoPtr will contain the ptr to TCPSocketInfo which inturn contains
	      the detailed information about the connected client.
	   2. If the observer creates UDP Listen socket, and when any message is received on that socket then cSocket
	      class will call this function as callback. In this case SockActivity will be equal to UDP_MESSAGE_RECEIVED
	      and UDPSockInfoPtr will contain the ptr to UDPSocketInfo structure which inturn contains the detailed info
	      about the message and from which client this message is received.

	  Input Parameters:
	  param1: const SocketObserverParams : Structure which contins related info as per the scenarios explained above.

	  Return Value:
	  Void - This is the void function. Will return nothing.
	*/
	virtual void HandleSocketActivity(IN SocketObserverParams &aSockObsParms) = 0;
};

/* CSocket: This class provides all the necessary basic oprations related to Socket. Application can simply instantiate this
   class and use it for the socket operations. This class provides apis to operate on UDP & TCP Sockets. Detailed description
   of each api can be found below near it's declaration.
*/
class cSocket
{
public:
	/* Default Constructor */
	cSocket();

	/* cSocket - Constructor along with setting the socket params. If this constructor is used then after that it is not
	   required to call InitializeParams.

	   Input Parameters:
	   param1: const int8* const : Pointer to the null terminated string which contains the Address with which
				      socket needs to be created in the subcequent calls.
	   param2: const uint32 : Port number which will be used with the socket system call.
	   param3: const cSocketObserver* : pointer to the cSocketObserver class. This is used during classbacks. Application
					    which will use this cSocket class should be derived from cSocketObserver class
					    and pass its own pointer to this class. In case of creating the TCP Server socket
					    or UDP Listen socket then cSocket will use the call back functions (pure virutal
					    functions) defined in cSocketObserver class to intimate the application about the
					    activity.
	   Return Value:
	   Void - This is the void function. Will return nothing.
	*/
	cSocket(IN const int8* const aAddr,IN const uint32 aPort,IN const cSocketObserver * aObserver);

	/* InitializeParams - InitializeParams is used to set the inital parameters like Address and port number of socket

	   Input Parameters:
	   param1: const int8* const : Pointer to the null terminated string which contains the Address with which
				      socket needs to be created in the subcequent calls.
	   param2: const uint32 : Port number which will be used with the socket system call.
	   param3: const cSocketObserver* : pointer to the cSocketObserver class. This is used during classbacks. Application
					    which will use this cSocket class should be derived from cSocketObserver class
					    and pass its own pointer to this class. In case of creating the TCP Server socket
					    or UDP Listen socket then cSocket will use the call back functions (pure virutal
					    functions) defined in cSocketObserver class to intimate the application about the
					    activity.
	   Return Value:
	   Void - This is the void function. Will return nothing.
	*/
	void InitializeParams(IN const int8* const aAddr,IN const uint32 aPort, IN const cSocketObserver * aObserver);

	/* SetListenCount - SetListenCount will be used to configure the no. of clients to allow in the listen queue.

	   Note: Application can configure this Value. If application didn't set this one then we use the default value of 10.

	   Input Parameters:
	   param1: const uint32: Number which specifies the number of clients to keep in the listen queue.

	   Return Value:
	   Void - This is the void function. Will return nothing.
	*/
	void SetListenCount(IN const uint32 aListenCount);


	/* SetRecvBufferSize - SetRecvBufferSize will be used to configure the size of the receive buffer which will be
	  used in case of receivefrom system call and which is passed to the application along with the callback function
	  of UDPMessageReceived.

	  Note: Application can configure this Value. If application didn't set this one then we use the default value of 1024.

	   Input Parameters:
	   param1: const uint32: Number which specifies size of the received buffer which should be used in receivefrom system call.

	   Return Value:
	   Void - This is the void function. Will return nothing.
	*/
	void SetRecvBufferSize(IN const uint32	aRevBufSize);

	/* SetMembershipAddr - SetMembershipAddr will be used to specify the IP address to which we need to request for the
	  membership as soon as socket is created. This is useful in case of the UDP Linsten sockets. Actually in case of
	  CreateBlockingUDPListenSocket or CreateUnBlockingUDPListenSocket apis, UDP socket is created and immediatly it
	  waits on the receivefrom system call to listen for the messages. But setting of this membership is needed inbetween
	  socket creation and waiting on receivefrom system call. So application has no way to set this membership inbeween
	  above mentioned two states. Hence application can call this SetMembershipAddr to specify to which address we need
	  to request membership before the actual socket creation system call and inside the CreateBlockingUDPListenSocket
	  & CreateUnBlockingUDPListenSocket apis we set this membership after creating the socket and before waiting on
	  receivefrom system call.

	  Note: If application doen't specify this addr then we dont request membership. Everything else continues as usual.

	   Input Parameters:
	   param1: const int8* const: pointer to the null terminate string which contains the address to which membeship needs
				      to be requested.

	   Return Value:
	   Void - This is the void function. Will return nothing.
	*/
	void SetMembershipAddr(IN const int8* const aMemshipAddr);

	/* SetFilteringUDPMessageString - This function will be used to set the filtering string which can be used when
	   any message is received from udp socket. If this string is present in the UDP message received then only this
	   class will intimate to the observer using callback function. If this string is not present then the message will
	   be discarded and no intimation to the observer. By this way we can control the UDP trafic analyzation at the
	   observer class..

	  Note: If application doen't specify this addr then for all the UDP messages observer will be intimated and it will
		become the responsibility of the observer to parse each and every response.

	   Input Parameters:
	   param1: const int8* const: pointer to the null terminate string which will be used to match to see whether it is
		   present in UDP message received.

	   Return Value:
	   Void - This is the void function. Will return nothing.
	*/
	void SetFilteringUDPMessageString(IN const int8* const aUDPFilterString);

	/* CreateBlockingTCPServerSocket - CreateBlockingTCPServerSocket will create the TCP socket with the address and port
	  specified in InitializeParams api or in constructor, binds the address to it,create the listen queue and wait on
	  the accept system call. When any client is connected then it will create the new thread and will intimate to the
	  user by calling its TCPClientConnected callback function. User class needed to be derived from cSocketObserver
	  class and should implement this to perform further operation when and client is connected. On the main thread it
	  will keep on listening for further clients.

	   Note: By calling this function application will be in the blocking state as long as any client is connected.
	   once client is connected then application will receive the callback.

	   Input Parameters:
	   void: This function takes No parameters.

	   Return Value:
	   SOCKET: If everything goes well then this api is the blocking call. If something goes wrong then it will return FAILURE.
	*/
	SOCKET CreateBlockingTCPServerSocket(void);

	/* CreateUnBlockingTCPServerSocket-CreateUnBlockingTCPServerSocket will create the TCP socket with the address and
	  port specified in InitializeParams api or in constructor, binds the address to it,create the listen queue and it
	  willcreate the new thread. On main thread it will return the socket id of the created socket to the application
	  and on the child theread it will wait on the accept system call. When any client is connected then it will again
	  create the new thread and will intimate to the user by calling its TCPClientConnected callback function.
	  Application of this class needs to be derived from cSocketObserver class and should implement the pure virtual
	  functions of that class. On one thread it will keep on listening for further clients.

	  Note: By calling this function application will be returned with Socket id immediately. At the same time when any
	  client is connected then application will receive the callback. So application needs to be careful to remain
	  as long as it's created cSocket object is active. Other wise there may be the chance of crash if cSocket
	  calls TCPClientConnected callback and by that time appliation instance doesn't exist. If the application
	  really want to exit then it can call CloseSocket api and after that it can exist.

	   Input Parameters:
	   void: This function takes No parameters.

	   Return Value:
	   SOCKET: This api will return the Socket id of the created socket. In case of failures it will receive FAILURE.
	*/
	SOCKET CreateUnBlockingTCPServerSocket(void);

	/* CreateTCPClientSocket - CreateTCPClientSocket will create the TCP socket with the address and port specified in
	   InitializeParams api or in constructor, binds the address to it and return the socket id of the created socket
	   to the application. Application can do the further operations on this socket using this socket id.

	   Input/Output Parameters:
	   void: This function takes No parameters.

	   Return Value:
	   SOCKET: This api will return the Socket id of the created socket. In case of failures it will receive FAILURE.
	*/
	SOCKET CreateTCPClientSocket(void);

	/* CreateUDPSocket - CreateUDPSocket will create the UDP socket with the address and port specified in
	   InitializeParams api or in constructor, binds the address to it and return the socket id of the created socket
	   to the application. Application can do the further operations on this socket using this socket id.

	   Input/Output Parameters:
	   void: This function takes No parameters.

	   Return Value:
	   SOCKET: This api will return the Socket id of the created socket. In case of failures it will receive FAILURE.
	*/
	SOCKET CreateUDPSocket(void);

	/* CreateBlockingUDPListenSocket-CreateBlockingUDPListenSocket will create the UDP socket with the address and port
	  specified in InitializeParams api or in constructor, binds the address to it, if any membership address is set by
	  using SetMembershipAddr, then it will request for the membership and after that it will wait on the receivefrom
	  system call to listen for the messages. If any message is recieved then it will create new thread and on the child
	  thread it will intimate to the application by using UDPMessageReceived callback function. Application of this class
	  needs to be derived from cSocketObserver class and should implement the pure virtual functions of that class.

	  Input/Output Parameters:
	  void: This function takes No parameters.

	  Return Value:
	  SOCKET: If everything goes well then this api is the blocking call. If something goes wrong then it will return FAILURE.
	*/
	SOCKET UDPBlockingListenCall(void);

	/* CreateUnBlockingUDPListenSocket-CreateUnBlockingUDPListenSocket will create the UDP socket with the address
	  and port specified in InitializeParams api or in constructor, binds the address to it, if any membership address
	  is set by using SetMembershipAddr, then it will request for the membership and after that it will create new
	  thread and on the main thread it will return the socket id the created socket to the application and on the child
	  thread it will wait on the receivefrom system call to listen for the messages. If any message is recieved then it
	  will again create new thread and on the child thread it will intimate the application by using UDPMessageReceived
	  callback function. Application of this class needs to be derived from cSocketObserver class and should implement
	  the pure virtual functions of that class.

	  Note: By calling this function application will be returned with Socket id immediately. At the same time when any
	  message is received then application will receive the callback. So application needs to be careful to remain
	  as long as it's created cSocket object is active. Other wise there may be the chance of crash if cSocket
	  calls UDPMessageReceived callback and by that time appliation instance doesn't exist. If the application
	  really want to exit then it can call CloseSocket api and after that it can exist.

	   Input/Output Parameters:
	   void: This function takes No parameters.

	   Return Value:
	   SOCKET: This api will return the Socket id of the created socket. In case of failures it will receive FAILURE.
	*/
	SOCKET UDPUnBlockingListenCall(void);

	  /* Send - Send will be used to send the data on the socket.

	  Input/Output Parameters:
	  param1: const SOCKET : ID of the socket on which data needs to be sent.
	  param2: const int8* const: Pointer to the buffer whoose data to be sent on the socket.
	  param3: const uint32: Size of the above buffer which needs to be sent on socket.

	  Return Value:
	  int32: Number of bytes successfully sent on the socket. In case of failures -1 is returned.
	*/
	static int32 Send(IN const SOCKET aSockID, IN const int8* const aBuf, IN const uint32 aBufLen);

	/* Receive - Receive will be used to Receive the data from socket.

	  Input/Output Parameters:
	  param1: const SOCKET : ID of the socket on which data needs to be received.
	  param2: int8* const: Pointer to the buffer to where data to be received from the socket.
	  param3: const uint32: Size of the above buffer.

	  Return Value:
	  int32: Number of bytes successfully received from the socket. In case of failures -1 is returned.
	*/
	static int32 Receive(IN const SOCKET aSockID, OUT int8* const aBuf, IN const uint32 aBufLen);

	/* SendTo - SendTo will be used to send the data to particular client on the socket. Mostly useful incase of UDP sockets.

	  Input/Output Parameters:
	  param1: const SOCKET : ID of the socket on which data needs to be sent.
	  param2: const int8* const: Pointer to the buffer whoose data to be sent on the socket.
	  param3: const uint32: Size of the above buffer which needs to be sent on socket.
	  param4: const int8* const: pointer to the null terminated string which points to the ip address to where data to be sent.
	  param5: const uint32: port no of the destination to where data needs to be sent on socket.

	  Return Value:
	  int32: Number of bytes successfully sent on the socket. In case of failures -1 is returned.
	*/
	static int32 SendTo(IN const SOCKET aSockID, IN const int8* const aBuf, IN const uint32 aBufLen,
		     IN const int8* const aAddr, IN const uint32 aPort);

	/* ReceiveFrom - ReceiveFrom will be used to Receive the data from socket along with the ipaddress of the client
	  from where data is received..

	  Input/Output Parameters:
	  param1: const SOCKET : ID of the socket on which data needs to be received.
	  param2: int8* const: Pointer to the buffer to where data to be received from the socket.
	  param3: const uint32: Size of the above buffer.
	  param4: int8* const: Out paramter to which the ip address of the client where data is received is copied. It will
			       be the null terminated string.
	  param5: int32: port number of the destination port from where data is received.

	  Return Value:
	  int32: Number of bytes successfully received from the socket. In case of failures -1 is returned.
	*/
	static int32 ReceiveFrom(IN const SOCKET aSockID, OUT int8* const aBuf, IN const uint32 aBufLen,
			  OUT int8* const aAddr, OUT uint32& aPort);

	/* CloseSocket - CloseSocket will close the soket whose socket id is mentioned by its input parameter.

	  Input/Output Parameters:
	  param1: const SOCKET : ID of the socket which needs to be closed.

	  Return Value:
	  Void - This is the void function. Will return nothing.
	*/
	static void CloseSocket(const SOCKET aSockID);

	/* CloseConnection - CloseConnection will close the main soket created by its instance. The main difference between
	   this api and CloseSocket api is, CloseSocket api can be called with any socketid where are this api will close
	   the main socket opened by it's instance.

	  Input/Output Parameters:
	  void: This function takes nothing as input.

	  Return Value:
	  Void - This is the void function. Will return nothing.
	*/
	void CloseConnection(void);

	/* ReceiveBroadcastMessages - ReceiveBroadcastMessages is the function which is registerd to thread creation system call.
	  This function is registed inside the CreateUnBlockingUDPListenSocket api to return the SOCKET id to application
	  on one theread and continue receive the broadcast messages on the other thread. As the class variables are not
	  accessibe in the static functions we pass all the needed class variables as one structure named cvLocalCallbackStruct
	  as a void pointer when the new thread is created.

	  Input/Output Parameters:
	  param1: void*: This function takes void pointer as input. This is the requirement from pthread_create function to register
			 for its callback

	  Return Value:
	  Void* - This function will return void pointer.This is the requirement from pthread_create function to register
		  for its callback
	*/
	static void* ReceiveBroadcastMessages(void*);

	/* LookForClients - LookForClients is the function which is registerd to thread creation system call.
	  This function is registed inside the CreateUnBlockingTCPServerSocket api to return the SOCKET id to application
	  on one theread and continue listening from the clients on the other thread. As the class variables are not
	  accessibe in the static functions we pass all the needed class variables as one structure named cvLocalCallbackStruct
	  as a void pointer when the new thread is created.

	  Input/Output Parameters:
	  param1: void*: This function takes void pointer as input. This is the requirement from pthread_create function to register
			 for its callback

	  Return Value:
	  Void* - This function will return void pointer.This is the requirement from pthread_create function to register
		  for its callback
	*/
	static void* LookForClients(void*);

	/* AcceptCallback - AcceptCallback is the function which is registerd to thread creation system call.
	  This function is registed when any TCP client is connected to TCP server socket or when any message is received on
	  the UDP listen socket to callback the applications function. All the needed information is passed as one structure
	  named cvThreadStruct as a void pointer when the new thread is created.

	  Input/Output Parameters:
	  param1: void*: This function takes void pointer as input. This is the requirement from pthread_create function to register
			 for its callback

	  Return Value:
	  Void* - This function will return void pointer.This is the requirement from pthread_create function to register
		  for its callback
	*/
	static void* AcceptCallback(void*);

	/* CloseAllSessions - CloseAllSessions will close all the sockets created by all the instances of this class. It
	  keeps track of all the sockes created by any instance using the staic variables cvConnectedSockIDs & cvNoOfConnectedSockIDs.
	  When this api is called then we will close all the socket ids registered in cvConnectedSockIDs satic array. With
	  this all sockets will be closed. Doen't matter which thread opened it. As this is the static function this function
	  can be called form any where with out even having any instance of this class. Most probably this function needs
	  to be called when the application is going to stop to close all the opened socket resources.

	  Input/Output Parameters:
	  void: This function takes nothing as input.

	  Return Value:
	  Void - This is the void function. Will return nothing.
	*/
	static void CloseAllSessions(void);

	void StopSocketServer();

	/* Static variables which keeps track of all the sockets opened by this class. No matter which instance opened it.
	   It will keep track of all the sockets opened with in this code. */
	static bool cvCloseAllSessions;
	static int32 cvConnectedSockIDs[100];
	static int32 cvNoOfConnectedSockIDs;
private:
	enum{
		IDLE = 0,
		CREATE,
		BIND,
		LISTEN,
		ACCEPT,
		CONNECT
	}cvState;

	/* Structure which is used to pass the information to the newly created thread. */
	typedef struct{
	  cSocketObserver *Observer;
	  int32 SockID;
	  //int32 ClientSockID;
	  int32 SelfPort;
	  int8 SelfAddr[MAXIMUM_IP_SIZE];
	  int32 ClientPort;
	  int8 ClientAddr[MAXIMUM_IP_SIZE];
	  void *RecvBufPtr;
	  int32 RecvBufLen;
	  SocketActivity SockActivity;
	}cvThreadStruct;

	/* Just a simple structure to hold the UDP Messages Filter Strings.. */
	typedef struct{
		int8 FilterString[64];
	}UDPFilterString;

	/* Structure which is used to pass the information to the newly created thread. */
	typedef struct{
	  int32 SockID;
	  int32 SelfPort;
	  int8 SelfAddr[MAXIMUM_IP_SIZE];
	  int32 *ServerActive;
	  cSocketObserver *Observer;
	  int32 RecvBufSize;
	  cLinkedList<UDPFilterString> *filterStringsListPtr;
	}cvLocalCallbackStruct;

	/* Address with which socket needs to be created. Doesn't mater whatever the socket is. */
	int8 cvAddr[MAXIMUM_IP_SIZE];
	/* port on which socket needs to be created. Doesn't mater whatever the socket is. */
	uint32 cvPort;
	/* socket id of the created socket. Doesn't mater whatever the socket is. */
	int32 cvSockID;
	/* contains the socket id of the client which is connected.*/
	int32 cvClientSockID;
	/* Holds the linsten count set by application. */
	int8  cvListenCount;
	/* Holds the receive buffer size set by application. */
	int32 cvRecvBufSize;
	/* Holds the membership address set by application. Useful incase of UDP Listen socket. */
	int8 cvMembershipAddr[MAXIMUM_IP_SIZE];
	/* Holds the UDPFilterString  which can be used to set the filter string by observer.. */
	cLinkedList<UDPFilterString> cvUDPFilterStringList;
	/* Keeps track wheather server is active or not. */
	int32  cvSocketServerActive;
	/* Holds the address of the observer. By using this pointer we can call the callback functions as and when needed.*/
	cSocketObserver *cvObserver;
};
#endif /* SOCKETIMPL_H_INCLUDED */
