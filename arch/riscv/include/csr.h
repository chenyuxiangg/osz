#ifndef __CSR_H__
#define __CSR_H__
#include "comm.h"

#define __ASM_STR(x)		#x

#define CSR_READ(csr)						            \
({								                        \
	register UINT32 __v;				                \
	__asm__ __volatile__ ("csrr %0, " __ASM_STR(csr)	\
			      : "=r" (__v) :			            \
			      : "memory");			                \
	__v;							                    \
})

#define CSR_WRITE(csr, val)                             \
({                                                      \
    register UINT32 __v = (UINT32)val;   	            \
	__asm__ __volatile__ ("csrw " __ASM_STR(csr) ", %0"	\
			      : : "r" (__v)			                \
			      : "memory");							\
})

#define CSR_READ_SET(csr, val)                                  \
({                                                              \
    register UINT32 __v = (UINT32)val;   	                    \
	__asm__ __volatile__ ("csrrs %0, " __ASM_STR(csr) ", %1"	\
			      : "=r" (__v)                                  \
                  : "r" (__v)			                        \
			      : "memory");			                        \
	__v;														\
})

#define CSR_READ_CLEAR(csr, val)                                \
({                                                              \
    register UINT32 __v = (UINT32)val;   	                    \
	__asm__ __volatile__ ("csrrc %0, " __ASM_STR(csr) ", %1"	\
			      : "=r" (__v)                                  \
                  : "r" (__v)			                        \
			      : "memory");			                        \
	__v;														\
})

#define CSR_CLEAR(csr, val)                             \
({                                                      \
    register UINT32 __v = (UINT32)val;   	            \
	__asm__ __volatile__ ("csrc " __ASM_STR(csr) ", %0"	\
			      : : "r" (__v)			                \
			      : "memory");			                \
})

#define CSR_SET(csr, val)                               \
({                                                      \
    register UINT32 __v = (UINT32)val;   	            \
	__asm__ __volatile__ ("csrs " __ASM_STR(csr) ", %0"	\
			      : : "r" (__v)			                \
			      : "memory");							\
})

/* Machine Status Register, mstatus */
#define MSTATUS_MPP (3 << 11)
#define MSTATUS_SPP (1 << 8)

#define MSTATUS_MPIE (1 << 7)
#define MSTATUS_SPIE (1 << 5)
#define MSTATUS_UPIE (1 << 4)

#define MSTATUS_MIE (1 << 3)
#define MSTATUS_SIE (1 << 1)
#define MSTATUS_UIE (1 << 0)

/* Machine-mode Interrupt Enable */
#define MIE_MEIE (1 << 11) // external
#define MIE_MTIE (1 << 7)  // timer
#define MIE_MSIE (1 << 3)  // software

/* Machine-mode Cause Masks */
#define MCAUSE_MASK_INTERRUPT	(reg_t)0x80000000
#define MCAUSE_MASK_ECODE	    (reg_t)0x7FFFFFFF

reg_t r_tp(VOID);
reg_t r_mhartid(VOID);
reg_t r_mstatus(VOID);
VOID w_mstatus(reg_t x);
VOID w_mepc(reg_t x);
reg_t r_mepc(VOID);
VOID w_mscratch(reg_t x);
VOID w_mtvec(reg_t x);
reg_t r_mie(VOID);
VOID w_mie(reg_t x);
reg_t r_mcause(VOID);

#endif