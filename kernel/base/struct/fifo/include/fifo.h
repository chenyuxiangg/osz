#ifndef __FIFO_H__
#define __FIFO_H__
#include "comm.h"

typedef enum {
    FIFO_WRITE_OVER = 0,
    FIFO_WRITE_STOP
} FIFO_W_MODE;

typedef struct {
    UINTPTR start_addr;
    CHAR    *read_ptr;
    CHAR    *write_ptr;
    union {
        struct {
            UINT32  size : 16;
            UINT32  full : 1;
            UINT32  empty : 1;
            UINT32  reserv : 14;
        };
        UINT32 attr;
    };
} FIFO;

FIFO    *fifo_create(UINT16 size);
VOID    fifo_delete(FIFO *fifo);
BOOL    fifo_is_empty(FIFO *fifo);
BOOL    fifo_is_full(FIFO *fifo);
UINT16  fifo_read(FIFO *fifo, CHAR *out, UINT16 size);
UINT16  fifo_write(FIFO *fifo, VOID *val, UINT16 size, FIFO_W_MODE mode);

#endif