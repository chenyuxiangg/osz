#ifndef __EVENT_NEW_H__
#define __EVENT_NEW_H__
#include "comm.h"
#include "ipc.h"

typedef osz_ipc_t osz_events_t;

typedef enum __osz_events_op_t{
    EVENT_FLAG_OR = 1,
    EVENT_FLAG_AND = 2,
    EVENT_FLAG_CLEAN = 4,
} osz_events_op_t;

#define EVENT_BASE_ERR                              (0x00020000)
#define EVENT_READ_EVENTS_NULL_ERR                  (EVENT_BASE_ERR | 0x1)
#define EVENT_READ_EVENTS_OBJ_NOT_IPC_ERR           (EVENT_BASE_ERR | 0x2)
#define EVENT_READ_EVENTS_OBJ_NOT_USED_ERR          (EVENT_BASE_ERR | 0x3)
#define EVENT_READ_NOT_EVENTS_TYPE_ERR              (EVENT_BASE_ERR | 0x4)
#define EVENT_READ_OP_CONFLICT_ERR                  (EVENT_BASE_ERR | 0x5)
#define EVENT_READ_OP_NOT_EXSIT_ERR                 (EVENT_BASE_ERR | 0x6)
#define EVENT_READ_NO_WAIT_ERR                      (EVENT_BASE_ERR | 0x7)
#define EVENT_WRITE_WAIT_TASK_ERR                   (EVENT_BASE_ERR | 0x8)
#define EVENT_CREATE_OUTTER_NOT_NULL_ERR            (EVENT_BASE_ERR | 0x9)
#define EVENT_CREATE_OUTTER_MALLOC_FAIL_ERR         (EVENT_BASE_ERR | 0xA)
#define EVENT_DELETE_NULL_OBJ_ERR                   (EVENT_BASE_ERR | 0xB)

uint32_t osz_events_init(uint8_t *name, uint8_t name_size, osz_events_t **outter_obj);
uint32_t osz_events_read(osz_events_t *events, uint32_t event_set, osz_events_op_t op_flag, uint32_t timeout, uint32_t *outter_events);
uint32_t osz_events_write(osz_events_t *events, uint32_t event_set);
uint32_t osz_events_create(uint8_t *name, uint8_t name_size, osz_events_t **outter_obj);
uint32_t osz_events_delete(osz_events_t *events);
uint32_t osz_events_detach(osz_events_t *events);

#endif