# select defconfig based on actual architecture
ifeq ($(ARCH),x86)
        KBUILD_DEFCONFIG := x86_defconfig
else ifeq ($(ARCH),x86_64)
        KBUILD_DEFCONFIG := x86_defconfig
else ifeq ($(ARCH),amd64)
        KBUILD_DEFCONFIG := x86_defconfig
else
        KBUILD_DEFCONFIG := $(ARCH)_defconfig
endif
