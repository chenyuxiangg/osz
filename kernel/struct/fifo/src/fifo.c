#include "fifo.h"
#include "mem.h"
#include "atomic.h"

#define FIFO_EMPTY_BIT  (0x11)
#define FIFO_FULL_BIT   (0x10)
#define FIFO_EMPTY      (0x1)
#define FIFO_FULL       (0x1)

BOOL fifo_is_full(FIFO *fifo)
{
    UINT32 attr = atomic_read(UINT32, &(fifo->attr));
    CHAR c = ((attr >> FIFO_FULL_BIT) & FIFO_FULL);
    return (c == FIFO_FULL);
}

BOOL fifo_is_empty(FIFO *fifo)
{
    UINT32 attr = atomic_read(UINT32, &(fifo->attr));
    CHAR c = ((attr >> FIFO_EMPTY_BIT) & FIFO_EMPTY);
    return (c == FIFO_EMPTY);
}

FIFO *fifo_create(UINT16 size)
{
    if (size == 0) {
        return NULL;
    }
    FIFO *fifo = (FIFO *)osz_zalloc(sizeof(FIFO));
    fifo->start_addr = (UINTPTR)osz_zalloc(size);
    if (fifo->start_addr == NULL) {
        osz_free(fifo);
        return NULL;
    }
    fifo->size = size;
    fifo->read_ptr = fifo->write_ptr = (CHAR *)fifo->start_addr;
    fifo->empty = FIFO_EMPTY;
    return fifo;
}

VOID fifo_delete(FIFO *fifo)
{
    if (fifo != NULL) {
        if (fifo->start_addr != NULL) {
            osz_free((VOID *)fifo->start_addr);
            fifo->start_addr = NULL;
        }
        osz_free((VOID *)fifo);
        fifo = NULL;
    }
}

UINT16 fifo_read(FIFO *fifo, CHAR *out, UINT16 size)
{
    UINT32 real_cnt = 0;
    for (; real_cnt < size; ++real_cnt) {
        if (fifo_is_empty(fifo)) {
            break;
        }
        out[real_cnt] = atomic_read(CHAR, fifo->read_ptr);
        atomic_inc(CHAR, &(fifo->read_ptr));
        if ((UINTPTR)(fifo->read_ptr) >= (fifo->start_addr + fifo->size)) {
            atomic_write(UINT32, &(fifo->read_ptr), fifo->start_addr);
        }
        atomic_write(UINT32, &(fifo->attr), (fifo->attr & (~(FIFO_FULL << FIFO_FULL_BIT))));
        if (fifo->read_ptr == fifo->write_ptr) {
            atomic_write(UINT32, &(fifo->attr), (fifo->attr | ((FIFO_EMPTY << FIFO_EMPTY_BIT))));
        }
    }
    return real_cnt;
}

UINT16 fifo_write(FIFO *fifo, VOID *val, UINT16 size, FIFO_W_MODE mode)
{
    UINT32 real_cnt = 0;
    for (; real_cnt < size; ++real_cnt) {
        if (fifo_is_full(fifo) && (mode == FIFO_WRITE_STOP)) {
            break;
        }
        atomic_write(CHAR, fifo->write_ptr, *(CHAR *)val);
        atomic_inc(CHAR, val);
        atomic_inc(CHAR, &(fifo->write_ptr));
        if ((UINTPTR)(fifo->write_ptr) >= (fifo->start_addr + fifo->size)) {
            atomic_write(UINT32, &(fifo->write_ptr), fifo->start_addr);
        }
        atomic_write(UINT32, &(fifo->attr), (fifo->attr & (~(FIFO_EMPTY << FIFO_EMPTY_BIT))));
        if (fifo->write_ptr == fifo->read_ptr) {
            atomic_write(UINT32, &(fifo->attr), (fifo->attr | ((FIFO_FULL << FIFO_FULL_BIT))));
        }
    }
    return real_cnt;
}