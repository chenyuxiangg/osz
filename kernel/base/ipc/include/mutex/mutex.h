#ifndef __MUTEX_H__
#define __MUTEX_H__

#include "comm.h"
#include "ipc.h"

typedef osz_ipc_t osz_mutex_t;

#define MUTEX_BASE_ERR                                (0x00040000)
#define MUTEX_CREATE_OUTTER_NOT_NULL_ERR              (MUTEX_BASE_ERR | 0x1)
#define MUTEX_CREATE_OUTTER_MALLOC_FAIL_ERR           (MUTEX_BASE_ERR | 0x2)
#define MUTEX_DELETE_NULL_OBJ_ERR                     (MUTEX_BASE_ERR | 0x3)
#define MUTEX_CHECK_NULL_OBJ_ERR                      (MUTEX_BASE_ERR | 0x4)
#define MUTEX_CHECK_OBJ_NOT_MUTEX_ERR                 (MUTEX_BASE_ERR | 0x5)
#define MUTEX_CHECK_OBJ_NOT_USED_ERR                  (MUTEX_BASE_ERR | 0x6)
#define MUTEX_CHECK_NOT_MUTEX_TYPE_ERR                (MUTEX_BASE_ERR | 0x7)
#define MUTEX_PEND_NO_WAIT_ERR                        (MUTEX_BASE_ERR | 0x8)
#define MUTEX_POST_NUM_OVER_ERR                       (MUTEX_BASE_ERR | 0x9)
#define MUTEX_POST_WAIT_TASK_ERR                      (MUTEX_BASE_ERR | 0xA)
#define MUTEX_POST_REENTRANT_ERR                      (MUTEX_BASE_ERR | 0xB)

uint32_t osz_mutex_init(uint8_t *name, uint8_t name_size, uint32_t val, osz_mutex_t **outter_obj);
uint32_t osz_mutex_create(uint8_t *name, uint8_t name_size, osz_mutex_t **outter_obj);
uint32_t osz_mutex_pend(osz_mutex_t *mutex, uint32_t timeout);
uint32_t osz_mutex_post(osz_mutex_t *mutex);
uint32_t osz_mutex_delete(osz_mutex_t *mutex);
uint32_t osz_mutex_detach(osz_mutex_t *mutex);
#endif