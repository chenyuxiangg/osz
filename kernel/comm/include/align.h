#ifndef __ALIGN_H__
#define __ALIGN_H__

#define ALIGN_UP(x, boundary)       (((x) + (boundary-1))&(~(boundary-1)))
#define ALIGN_DOWN(x, boundary)     ((x) & ~(boundary - 1))

#endif