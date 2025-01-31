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

typedef int             INT32;
typedef unsigned int    UINT32;
typedef char            INT8;
typedef char            CHAR;
typedef unsigned char   UCHAR;
typedef unsigned char   UINT8;
typedef short           INT16;
typedef unsigned short  UINT16;
typedef UINT32          UINTPTR;
typedef void            VOID;
typedef unsigned char   BOOL;
typedef UINT32          reg_t;

#endif