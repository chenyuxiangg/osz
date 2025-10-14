#ifndef __SEM_H__
#define __SEM_H__
#include "comm.h"
#include "ipc.h"

typedef osz_ipc_t osz_sem_t;

#define SEM_BASE_ERR                                (0x00030000)
#define SEM_CREATE_OUTTER_NOT_NULL_ERR              (SEM_BASE_ERR | 0x1)
#define SEM_CREATE_OUTTER_MALLOC_FAIL_ERR           (SEM_BASE_ERR | 0x2)
#define SEM_DELETE_NULL_OBJ_ERR                     (SEM_BASE_ERR | 0x3)
#define SEM_PEND_NULL_OBJ_ERR                       (SEM_BASE_ERR | 0x4)
#define SEM_PEND_OBJ_NOT_SEM_ERR                    (SEM_BASE_ERR | 0x5)
#define SEM_PEND_OBJ_NOT_USED_ERR                   (SEM_BASE_ERR | 0x6)
#define SEM_PEND_NOT_SEM_TYPE_ERR                   (SEM_BASE_ERR | 0x7)
#define SEM_PEND_NO_WAIT_ERR                        (SEM_BASE_ERR | 0x8)
#define SEM_POST_NUM_OVER_ERR                       (SEM_BASE_ERR | 0x9)
#define SEM_POST_WAIT_TASK_ERR                      (SEM_BASE_ERR | 0xA)

uint32_t osz_sem_init(uint8_t *name, uint8_t name_size, uint32_t val, osz_sem_t **outter_obj);
uint32_t osz_sem_pend(osz_sem_t *sem, uint32_t timeout);
uint32_t osz_sem_post(osz_sem_t *sem);
uint32_t osz_sem_create(uint8_t *name, uint8_t name_size, osz_sem_t **outter_obj);
uint32_t osz_sem_delete(osz_sem_t *sem);
uint32_t osz_sem_detach(osz_sem_t *sem);

#endif