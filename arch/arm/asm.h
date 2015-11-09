/*
 * asm.h
 */

#ifndef __ASM_H__
#define __ASM_H__

#ifdef __aarch64__
#define ALIGN   .align 4
#else
#define ALIGN   .align 0
#endif

#define ENTRY(name) \
    .globl name; \
    ALIGN; \
    name:

#define END(name) \
    .size name, .-name

#define ENDPROC(name) \
    .type name, @function; \
    END(name)

#endif /* __ASM_H__ */
