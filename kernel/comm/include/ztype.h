#ifndef __ZTYPE_H__
#define __ZTYPE_H__

#ifndef NULL
#define NULL        (0)
#endif

#ifndef TRUE
#define TRUE        (1)
#endif

#ifndef FALSE
#define FALSE       (0)
#endif

#define SUCCESS     (0)
#define FAILUER     (-1)
#define OS_OK       (0)
#define OS_NOK      (-1)

typedef int                 INT32;
typedef unsigned int        UINT32;
typedef unsigned long long  UINT64;
typedef long long  INT64;
typedef char                INT8;
typedef char                CHAR;
typedef unsigned char       UCHAR;
typedef unsigned char       UINT8;
typedef short               INT16;
typedef unsigned short      UINT16;
typedef UINT32              UINTPTR;
typedef void                VOID;
typedef unsigned char       BOOL;
typedef UINT32              reg_t;
typedef UINT32              size_t;
typedef __builtin_va_list   va_list;

typedef UINT64              uint64_t;
typedef UINT32              uint32_t;
typedef INT32               ptrdiff_t;
typedef INT64               intmax_t;
typedef UINT32              uintptr_t;

#define READ_BYTE(addr)             ({unsigned char val = *(volatile unsigned char *)(addr);val;})
#define WRITE_BYTE(addr, val)       (*(volatile unsigned char *)(addr)=val)

#endif