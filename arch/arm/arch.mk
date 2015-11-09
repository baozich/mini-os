ifeq ($(MINIOS_TARGET_ARCH),arm32)
DEF_ASFLAGS += -march=armv7-a -mfpu=vfpv3
ARCH_CFLAGS  := -march=armv7-a -marm -fms-extensions -D__arm__ -DXEN_HAVE_PV_GUEST_ENTRY #-DCPU_EXCLUSIVE_LDST
EXTRA_INC += $(TARGET_ARCH_FAM)/$(MINIOS_TARGET_ARCH)
EXTRA_SRC += arch/$(EXTRA_INC)
endif

ifeq ($(MINIOS_TARGET_ARCH),arm64)
ARCH_CFLAGS  := -march=armv8-a -D__aarch64__ -DXEN_HAVE_PV_GUEST_ENTRY -mgeneral-regs-only -fno-stack-protector
ARCH_LDFLAGS :=
ARCH_ASFLAGS :=
EXTRA_INC += $(TARGET_ARCH_FAM)/$(MINIOS_TARGET_ARCH)
EXTRA_SRC += arch/$(EXTRA_INC)
endif
