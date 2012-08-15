
#ifndef BASIC_DATATYPES_H_INCLUDED
#define BASIC_DATATYPES_H_INCLUDED

/************************** Basic Data Types. ******************************/
typedef unsigned char uint8;
typedef char int8;
typedef unsigned short uint16;
typedef signed short int16;
typedef unsigned int uint32;
typedef signed int int32;
typedef int flag;
typedef double floatvar;
/*********************** Basic Data Types END. *****************************/
//#define size_t uint8
#ifndef NULL
#define NULL 0
#endif /* NULL */

/********************* Function Argument Types *****************************/
#ifndef IN
#define IN
#endif /* IN */

#ifndef OUT
#define OUT
#endif /* OUT */

#ifndef INOUT
#define INOUT
#endif /* INOUT */
/********************* Function Argument Types END. *************************/

/********************** Socket Related Parameters ***************************/
#define MAXIMUM_IP_SIZE 17
/******************* Socket Related Parameters END. *************************/


/********************* Thread Related Parameters ****************************/
typedef enum{
	THREAD_PRIORITY_IS_LOW = 25,
	THREAD_PRIORITY_IS_NORMAL =50,
	THREAD_PRIORITY_IS_HIGH = 75
}uThreadPriority;
/********************* Thread Related Parameters END. ***********************/

#endif /* BASIC_DATATYPES_H_INCLUDED */
