#ifndef __INNER_EVENT_ERR_H__
#define __INNER_EVENT_ERR_H__

#define EVENT_BASE_ERR                      (0x30)
#define EVENT_OP_COMFLICT_ERR               (EVENT_BASE_ERR | 0x1)
#define EVENT_OP_INVALID_ERR                (EVENT_BASE_ERR | 0x2)
#define EVENT_WAIT_INVALID_ERR              (EVENT_BASE_ERR | 0x3)

#endif