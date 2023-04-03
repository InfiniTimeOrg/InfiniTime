//useful things to include in code

#ifndef TYPES_H
#define TYPES_H

#ifndef WIN32
	// true/false defines
	#define FALSE	0
	#define TRUE	-1
#endif

// datatype definitions macros
typedef unsigned char  u08;
typedef   signed char  s08;
typedef unsigned short u16;
typedef   signed short s16;
typedef unsigned long  u32;
typedef   signed long  s32;
typedef unsigned long long u64;
typedef   signed long long s64;

// #ifndef __AVR__
#ifdef __MBED__
	// use inttypes.h instead
	// C99 standard integer type definitions
	typedef unsigned char	uint8_t;
	typedef   signed char	int8_t;
	typedef unsigned short	uint16_t;
	typedef   signed short	int16_t;
	/*typedef unsigned long	uint32_t;
	typedef   signed long	int32_t;
	typedef unsigned long	uint64_t;
	typedef   signed long	int64_t;
	*/
#endif

// maximum value that can be held
// by unsigned data types (8,16,32bits)
#define MAX_U08	255
#define MAX_U16	65535
#define MAX_U32	4294967295

// maximum values that can be held
// by signed data types (8,16,32bits)
#define MIN_S08	-128
#define MAX_S08	127
#define MIN_S16	-32768
#define MAX_S16	32767
#define MIN_S32	-2147483648
#define MAX_S32	2147483647

#ifndef WIN32
	// more type redefinitions
	typedef unsigned char   BOOL;
	typedef unsigned char	BYTE;
	typedef unsigned int	WORD;
	typedef unsigned long	DWORD;

	typedef unsigned char	UCHAR;
	typedef unsigned int	UINT;
	typedef unsigned short  USHORT;
	typedef unsigned long	ULONG;

	typedef char			CHAR;
	typedef int				INT;
	typedef long			LONG;
#endif

#endif
