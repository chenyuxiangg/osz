#ifndef __STRING_H__
#define __STRING_H__

VOID *memset(VOID *dst, size_t value, size_t len);
VOID *memcpy(VOID *dst, VOID * src, size_t len);
UINT32 strlen(VOID *str);
UINT32 strncmp(VOID *str1, VOID *str2, UINT32 len);
VOID *strncpy(VOID *dest, VOID *src, UINT32 len);

#endif