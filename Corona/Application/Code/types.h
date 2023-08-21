#ifndef _TYPES_
#define _TYPES_

#define Align16 __declspec(align(16))
#define Align64 __declspec(align(64))

typedef unsigned char    uchar;
typedef signed   char    schar;

typedef signed   short   sshort;
typedef unsigned short   ushort;

typedef signed   __int8  sint8;
typedef signed   __int16 sint16;
typedef signed   __int32 sint32;
typedef signed   __int64 sint64;

typedef unsigned __int8  uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;

typedef __w64 unsigned int nuint;
typedef __w64 signed   int nsint;

#endif //_TYPES_