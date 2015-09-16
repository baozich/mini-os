ifeq ($(MINIOS_TARGET_ARCH),arm32)
DEF_ASFLAGS += -march=armv7-a -mfpu=vfpv3
ARCH_CFLAGS  := -march=armv7-a -marm -fms-extensions -D__arm__ -DXEN_HAVE_PV_GUEST_ENTRY #-DCPU_EXCLUSIVE_LDST
EXTRA_INC += $(TARGET_ARCH_FAM)/$(MINIOS_TARGET_ARCH)
EXTRA_SRC += arch/$(EXTRA_INC)
endif
