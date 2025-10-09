#ifndef __IPC_H__
#define __IPC_H__
#include "comm.h"
#include "object.h"

#define IPC_USED                0x1
#define IPC_NOT_USED            0x0

typedef enum __osz_ipc_obj_type_t {
    IPC_EVENTS = 0,
    IPC_SEM,
    IPC_MUTEX,
    IPC_MAIL_BOX,
    IPC_MSG_QUEUE,
} osz_ipc_obj_type_t;

typedef enum __osz_ipc_obj_create_type_t {
    IPC_STATIC_CREATE = 0,
    IPC_DYNAMIC_CREATE,
} osz_ipc_obj_create_type_t;

typedef struct __osz_ipc_t {
    osz_obj_t supper;
    struct {
        uint32_t   ipc_type : 16;
        uint32_t   ipc_create_type : 1;
        uint32_t   ipc_obj_used : 1;
        uint32_t   ipc_reserv : 14;
    } attr;
    union {
        uint32_t events;
        uint8_t  mutex;
        uint32_t sem;
        uint32_t *msg;
    } field;
    osz_list_t pend_list;
} osz_ipc_t;

#endif