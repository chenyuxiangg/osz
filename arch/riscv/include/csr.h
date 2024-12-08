#ifndef __CSR_H__
#define __CSR_H__

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

#endif