/*
 * hypercall.S
 */

#include "asm.h"
#include <xen/xen.h>

    .macro hypercall, name
    mov     x16, \name
    hvc     #0xEA1
    ret
    nop
    .endm

    .text
ENTRY(HYPERVISOR_memory_op)
    hypercall __HYPERVISOR_memory_op
ENDPROC(HYPERVISOR_memory_op)

ENTRY(HYPERVISOR_domctl)
    hypercall __HYPERVISOR_domctl
ENDPROC(HYPERVISOR_domctl)

ENTRY(HYPERVISOR_sched_op)
    hypercall __HYPERVISOR_sched_op
ENDPROC(HYPERVISOR_sched_op)

ENTRY(HYPERVISOR_console_io)
    hypercall __HYPERVISOR_console_io
ENDPROC(HYPERVISOR_console_io)

ENTRY(HYPERVISOR_xen_version)
    hypercall __HYPERVISOR_xen_version
ENDPROC(HYPERVISOR_xen_version)

ENTRY(HYPERVISOR_event_channel_op)
    hypercall __HYPERVISOR_event_channel_op
ENDPROC(HYPERVISOR_event_channel_op)

ENTRY(HYPERVISOR_physdev_op)
    hypercall __HYPERVISOR_physdev_op
ENDPROC(HYPERVISOR_physdev_op)

ENTRY(HYPERVISOR_sysctl)
    hypercall __HYPERVISOR_sysctl
ENDPROC(HYPERVISOR_sysctl)

ENTRY(HYPERVISOR_hvm_op)
    hypercall __HYPERVISOR_hvm_op
ENDPROC(HYPERVISOR_hvm_op)

ENTRY(HYPERVISOR_grant_table_op)
    hypercall __HYPERVISOR_grant_table_op
ENDPROC(HYPERVISOR_grant_table_op)

ENTRY(HYPERVISOR_vcpu_op)
    hypercall __HYPERVISOR_vcpu_op
ENDPROC(HYPERVISOR_vcpu_op)

ENTRY(HYPERVISOR_xsm_op)
    hypercall __HYPERVISOR_xsm_op
ENDPROC(HYPERVISOR_xsm_op)

