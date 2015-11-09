#ifndef __ARCH_LIMITS_H__
#define __ARCH_LIMITS_H__

#include <arch_mm.h>

#if defined(__arm__)
#define __STACK_SIZE_PAGE_ORDER	2
#elif defined(__aarch64__)
#define __STACK_SIZE_PAGE_ORDER	4
#endif

#define __STACK_SIZE (4 * PAGE_SIZE)

#endif
