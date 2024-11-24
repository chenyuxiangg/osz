#ifndef __ASSERT_H__
#define __ASSERT_H__

#define ASSERT(x)      ({if ((x) == NULL) assert();})

void assert();

#endif